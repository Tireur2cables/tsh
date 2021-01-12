#define _DEFAULT_SOURCE // utile pour setenv

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <assert.h>
#include "couple.h"
#include "ls.h"
#include "cd.h"
#include "pwd.h"
#include "cat.h"
#include "help.h"
#include "exit.h"
#include "cdIn.h"
#include "tar.h"
#include "mkdir.h"
#include "rm.h"
#include "cp.h"
#include "rmdir.h"
#include "mv.h"

int iscmd(char *, char *);
int isOnlySpace(char *, int);
void selectCommand(char *, int);
void selectCustomCommand(char *, int);
void get_header_size_tsh(struct posix_header *, int *);
int getNbArgs(char const *, int);
int launchFunc(int (*)(int, char *[]), char *, int);
int launchBuiltInFunc(int (*)(int, char *[]), char *, int);
int exec(int, char *[]);
int cdIn(int, char *[]);
int hasTarIn(char const *, int);
int is_tar_tsh(char *);
char *traiterArguements(char *, int *);
void parse_tube(char *, int *);
void parse_redirection(char *, int *);
int traite_redirection(char *, int, int *, int *, int *, int *);
void close_redirections(int, int, int, int, int, int, char *, int *, int *, char *, int *, int *);
char *traiterHome(char *, int *);
int exist_path_in_tar(int, char *);
int exist_file_in_tar(int, char *);
int in_same_tar(char *, char *);
int redirection_tar(char *, int, int *, int *, int *, int *);
int redirection_classique(char *, int, int *, int *);
int create_header(char *, struct posix_header *, int);
int write_block(int fd, struct posix_header *);

//tableau (et sa taille) des commandes implémentées (non built-in) pour les tar
int len_custom = 8;
couple custom[8] = {{"ls", ls}, {"pwd", pwd}, {"cat", cat}, {"mkdir", mkdir_tar}, {"cp", cp}, {"rmdir", rmdir_func}, {"rm", rm_func}, {"mv", mv}};


int main(int argc, char const *argv[]) {
	if (argc > 1) {
		errno = E2BIG;
		perror("Arguments non valides!");
		exit(EXIT_FAILURE);
	}

	char *pwd;
	char *twd;
	char *prompt = " $ ";
	while (1) {
		pwd = getcwd(NULL, 0);
		twd = getenv("TWD");
		int len_twd = 0;
		if (twd != NULL && strlen(twd) != 0) //s'ajoute seulement si besoin
			len_twd = strlen(twd) + 1;

		char new_prompt[strlen(pwd) + 1 + len_twd + strlen(prompt) + 1];
		strcpy(new_prompt, pwd);
		if (strcmp(pwd, "/") != 0) //seulement si on est pas à la racine
			strcat(new_prompt, "/");
		if (twd != NULL && strlen(twd) != 0) { //seulement si on est dans un .tar
			strcat(new_prompt, twd);
			strcat(new_prompt, "/");
		}
		strcat(new_prompt, prompt);

		char *line;
		if ((line = readline(new_prompt)) != NULL) {
			int readen = strlen(line);
			if (readen > 0 && !isOnlySpace(line, readen)) {//seulement si la ligne lu n'est pas vide (retour a la ligne etc ...)
				line = traiterHome(line, &readen); //traite le ~ au début des arguments
				line = traiterArguements(line, &readen); //traite les arguments si presence de . ..
				parse_tube(line, &readen); //traite les tubes puis les redirections
			}
		}else { //EOF, fin de fichier détéctée
			char *newline = "\n";
			int newline_len = strlen(newline);
			if (write(STDERR_FILENO, newline, newline_len) < newline_len) {
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
		}
		free(line);
	}
	return 0;
}

/*
* Traitement de la présence de tube dans la saisie de l'utilisateur
*/

void parse_tube(char *line, int *readen){
	if(!strstr(line, "|")) { // pas de tubes, on passe directement aux redirections
		parse_redirection(line, readen);
	}else {
		char copy[strlen(line)+1];
		strcpy(copy, line);
		char *newline;
		char *tmp = copy;
		char *saveptr;
		char *tok;
		char *old = NULL;
		int fdin[2];
		int fdout[2];
		int parcouru = 0;
		while ((tok = strtok_r(tmp, "|", &saveptr)) != NULL) {
			parcouru += strlen(tok);
			if (old != NULL) parcouru++;
			int savein;
			pipe(fdout);
			if (old != NULL) {
				char buf[BLOCKSIZE];
				int readen = 0;
				while ((readen = read(fdin[0], buf, BLOCKSIZE)) > 0) {
					if (write(fdout[1], buf, readen) < readen) {
						perror("Impossible de trnafere le contenu du tube!");
						return;
					}
				}
				close(fdin[0]);
				close(fdout[1]);
				savein = dup(STDIN_FILENO);
				if((dup2(fdout[0], STDIN_FILENO) < 0)){
					perror("Erreur de redirection");
					return;
				}
			}
			pipe(fdin);
			newline = malloc(strlen(tok)+1);
			assert(newline);
			strcpy(newline, tok);
			*readen = strlen(newline);

			int saveout = dup(STDOUT_FILENO);
			if (parcouru != strlen(line)) {
				if((dup2(fdin[1], STDOUT_FILENO) < 0)){
					perror("Erreur de redirection");
					return;
				}
			}

			parse_redirection(newline, readen);

			if (old != NULL) {
				if((dup2(savein, STDIN_FILENO) < 0)){
					perror("Erreur de redirection");
					return;
				}
			}
			if((dup2(saveout, STDOUT_FILENO) < 0)){
				perror("Erreur de redirection");
				return;
			}
			close(fdin[1]);
			close(fdout[0]);
			tmp = saveptr;
			old = tok;
		}
		close(fdin[0]);
		close(fdout[1]);
	}
}

/*
 	Implémentation : les redirections ne doivent pas etre dans les memes fichiers ou dans le meme tar et doivent etre de la forme ' > ' etc...
			impossible de verifier si une redirection est faite dans le meme tar que le input d'une commande qui affiche des choses (comme cat)
 	faire le tube, donc comportement imprévisible dans ce cas
*/
void parse_redirection(char *line, int *readen){
	char *pwd = getcwd(NULL, 0);
	char *twd = getenv("TWD");
	int twdlen = 0;
	int pwdlen = strlen(pwd) + 1;
	if (twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;
	int fd_entree = -1;
	int fd_sortie = -1;
	int fd_erreur = -1;
	int save_entree = -1;
	int save_sortie = -1;
	int save_erreur = -1;
	int enderreur = -1;
	int endsortie = -1;
	int taillesortie = -1;
	int tailleerreur = -1;
	char *file_erreur = NULL;
	char *file_sortie = NULL;
	char *file_entree = NULL;
	char *pos;
	if((pos = strstr(line, ">")) != NULL){ //On doit faire une redirection de la sortie standard ou de la sortie erreur
		if((pos = strstr(line, " 2>"))){ //redirection de la sortie erreur
			char command[pos-line + 1];
			strncpy(command, line, pos-line);
			command[pos-line] = '\0';

			int type;
			char *rest;
			if(pos[3] == '>'){ //2>>
				if (pos[4] != ' ') { // pas d'espace apres redirection = erreur
					char *error = "Erreur! Il faut un espace apres le '>' de la redirection!\n";
					int errrorlen = strlen(error);
					if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
						perror("Erreur d'écriture dans le shell!");
					return;
				}
				rest = pos+5; // après le ' '
				type = 5;
			}else{ //2>
				if (pos[3] != ' ') { // pas d'espace apres redirection = erreur
					char *error = "Erreur! Il faut un espace apres le '>' de la redirection!\n";
					int errrorlen = strlen(error);
					if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
						perror("Erreur d'écriture dans le shell!");
					return;
				}
				rest = pos+4; // après le ' '
				type = 4;
			}
			int restelen = strlen(rest);
			char *tok;
			int toklen = 0;
			if ((tok = strstr(rest, " ")) != NULL) {// retire les potentielles autres redirections ou suite de commandes
				restelen -= strlen(tok);
				toklen = strlen(tok);
			}
			char file[restelen + 1];
			strncpy(file, rest, restelen);
			file[restelen] = '\0';
			if (file[0] != '/') {
				file_erreur = malloc(pwdlen + twdlen + strlen(file) + 1);
				assert(file_erreur);
				strcpy(file_erreur, pwd);
				strcat(file_erreur, "/");
				if (twdlen != 0) {
					strcat(file_erreur, twd);
					strcat(file_erreur, "/");
				}
				strcat(file_erreur, file);
			}else {
				file_erreur = malloc(strlen(file) + 1);
				assert(file_erreur);
				strcpy(file_erreur, file);
			}
			if ((file_entree != NULL && in_same_tar(file_entree, file_erreur)) || (file_sortie != NULL && in_same_tar(file_sortie, file_erreur))) {
				close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
				if (file_erreur != NULL) free(file_erreur); //On a du rediriger l'erreur
				if (file_sortie != NULL) free(file_sortie); //On a du rediriger la sortie
				if (file_entree != NULL) free(file_entree); //On a du rediriger l'entrée
				char *error = "Erreur! Impossible de faire deux redirections dans le meme tar!\n";
				int errrorlen = strlen(error);
				if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
			if (traite_redirection(file, type, &fd_erreur, &save_erreur, &enderreur, &tailleerreur) < 0) {
				close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
				if (file_erreur != NULL) free(file_erreur);
				if (file_sortie != NULL) free(file_sortie);
				if (file_entree != NULL) free(file_entree);
				return;
			}

			if (toklen != 0) toklen++;
			int newlen = strlen(command) + toklen;

			line = realloc(line, newlen+1);
			assert(line);
			strcpy(line, command);
			if (toklen != 0) {
				strcat(line, " ");
				strcat(line, tok);
			}
			*readen = strlen(line);
			if (strstr(line, " 2>") != NULL) { // il y a une autre redirection erreur = erreur
				char *error = "Erreur! Une seule redirection de sortie erreur est autorisée!\n";
				int errrorlen = strlen(error);
				if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
		}
		if ((pos = strstr(line, " >"))) { //redirection de la sortie standard
			char command[pos-line + 1];
			strncpy(command, line, pos-line);
			command[pos-line] = '\0';

			int type;
			char *rest;
			if(pos[2] == '>'){ // >>
				if (pos[3] != ' ') { // pas d'espace apres redirection = erreur
					char *error = "Erreur! Il faut un espace apres le '>' de la redirection!\n";
					int errrorlen = strlen(error);
					if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
						perror("Erreur d'écriture dans le shell!");
					return;
				}
				rest = pos+4; // après le ' '
				type = 3;
			}else{ // >
				if (pos[2] != ' ') { // pas d'espace apres redirection = erreur
					char *error = "Erreur! Il faut un espace apres le '>' de la redirection!\n";
					int errrorlen = strlen(error);
					if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
						perror("Erreur d'écriture dans le shell!");
					return;
				}
				rest = pos+3; // après le ' '
				type = 2;
			}
			int restelen = strlen(rest);
			char *tok;
			int toklen = 0;
			if ((tok = strstr(rest, " ")) != NULL) { // retire les potentielles autres redirections ou suite de commandes
				restelen -= strlen(tok);
				toklen = strlen(tok);
			}
			char file[restelen + 1];
			strncpy(file, rest, restelen);
			file[restelen] = '\0';
			if (file[0] != '/') {
				file_sortie = malloc(pwdlen + twdlen + strlen(file) + 1);
				assert(file_sortie);
				strcpy(file_sortie, pwd);
				strcat(file_sortie, "/");
				if (twdlen != 0) {
					strcat(file_sortie, twd);
					strcat(file_sortie, "/");
				}
				strcat(file_sortie, file);
			}else {
				file_sortie = malloc(strlen(file) + 1);
				assert(file_sortie);
				strcpy(file_sortie, file);
			}
			if ((file_entree != NULL && in_same_tar(file_entree, file_sortie)) || (file_erreur != NULL && in_same_tar(file_sortie, file_erreur))) {
				close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
				if (file_erreur != NULL) free(file_erreur);
				if (file_sortie != NULL) free(file_sortie);
				if (file_entree != NULL) free(file_entree);
				char *error = "Erreur! Impossible de faire deux redirections dans le meme tar!\n";
				int errrorlen = strlen(error);
				if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
			if (traite_redirection(file, type, &fd_sortie, &save_sortie, &endsortie, &taillesortie) < 0) {
				close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
				if (file_erreur != NULL) free(file_erreur);
				if (file_sortie != NULL) free(file_sortie);
				if (file_entree != NULL) free(file_entree);
				return;
			}

			if (toklen != 0) toklen++;
			int newlen = strlen(command) + toklen;
			line = realloc(line, newlen+1);
			assert(line);
			strcpy(line, command);
			if (toklen != 0) {
				strcat(line, " ");
				strcat(line, tok);
			}
			*readen = strlen(line);
			if (strstr(line, " >") != NULL) { // il y a une autre redirection standard = erreur
				char *error = "Erreur! Une seule redirection de sortie standard est autorisée!\n";
				int errrorlen = strlen(error);
				if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
		}
	}
	if((pos = strstr(line, " <")) != NULL){ //On doit faire une redirection de l'entrée
		char command[pos-line + 1];
		strncpy(command, line, pos-line);
		command[pos-line] = '\0';

		int type;
		char *rest;
		if (pos[2] != ' ') { // pas d'espace apres redirection = erreur
			char *error = "Erreur! Il faut un espace apres le '<' de la redirection!\n";
			int errrorlen = strlen(error);
			if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
				perror("Erreur d'écriture dans le shell!");
			return;
		}
		rest = pos+3; // après le ' '
		type = 1;

		int restelen = strlen(rest);
		char *tok;
		int toklen = 0;
		if ((tok = strstr(rest, " ")) != NULL) {// retire les potentielles autres redirections ou suite de commandes
			restelen -= strlen(tok);
			toklen = strlen(tok);
		}
		char file[restelen + 1];
		strncpy(file, rest, restelen);
		file[restelen] = '\0';
		if (file[0] != '/') {
			file_entree = malloc(pwdlen + twdlen + strlen(file)+1);
			assert(file_entree);
			strcpy(file_entree, pwd);
			strcat(file_entree, "/");
			if (twdlen != 0) {
				strcat(file_entree, twd);
				strcat(file_entree, "/");
			}
			strcat(file_entree, file);
		}else {
			file_entree = malloc(strlen(file)+1);
			assert(file_entree);
			strcpy(file_entree, file);
		}
		if ((file_erreur != NULL && in_same_tar(file_entree, file_erreur)) || (file_sortie != NULL && in_same_tar(file_sortie, file_entree))) {
			close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
			if (file_erreur != NULL) free(file_erreur);
			if (file_sortie != NULL) free(file_sortie);
			if (file_entree != NULL) free(file_entree);
			char *error = "Erreur! Impossible de faire deux redirections dans le meme tar!\n";
			int errrorlen = strlen(error);
			if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
				perror("Erreur d'écriture dans le shell!");
			return;
		}
		if (traite_redirection(file, type, &fd_entree, &save_entree, NULL, NULL) < 0) {
			close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
			if (file_erreur != NULL) free(file_erreur);
			if (file_sortie != NULL) free(file_sortie);
			if (file_entree != NULL) free(file_entree);
			return;
		}

		if (toklen != 0) toklen++;
		int newlen = strlen(command) + toklen;
		line = realloc(line, newlen+1);
		assert(line);
		strcpy(line, command);
		if (toklen != 0) {
			strcat(line, " ");
			strcat(line, tok);
		}
		*readen = strlen(line);
		if (strstr(line, " >") != NULL) { // il y a une autre redirection standard = erreur
			char *error = "Erreur! Une seule redirection de sortie standard est autorisée!\n";
			int errrorlen = strlen(error);
			if (write(STDERR_FILENO, error, errrorlen) < errrorlen)
				perror("Erreur d'écriture dans le shell!");
			return;
		}
	}
	selectCommand(line, *readen);
	close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file_sortie, &endsortie, &taillesortie, file_erreur, &enderreur, &tailleerreur);
	if (file_erreur != NULL) free(file_erreur);
	if (file_sortie != NULL) free(file_sortie);
	if (file_entree != NULL) free(file_entree);
}

/*
* Appelée après la fin de l'execution de la commande, repositione les descripteurs en place, fermes ceux qui doivent l'etre, et écrit le header dans le tar si nécéssaires, en
* calculant la taille écrite.
*/
void close_redirections(int fd_entree, int fd_sortie, int fd_erreur, int save_entree, int save_sortie, int save_erreur, char *file_sortie, int *endsortie, int *taillesortie, char *file_erreur, int *enderreur, int *tailleerreur){
	if(save_entree != -1){
		close(fd_entree);
		if(dup2(save_entree, STDIN_FILENO) < 0){
			perror("erreur de redirection");
			return;
		}
		close(save_entree);
	}
	if (save_sortie != -1 && fd_sortie != fd_entree) {
		if (file_sortie != NULL && strstr(file_sortie, ".tar") != NULL) { // On se trouve dans un tar, on doit donc écrire le header
			off_t new_end_of_tar = lseek(fd_sortie, 0, SEEK_CUR); // On écrit le nouveau fichier a la fin du tar, on se trouve donc à la fin après écriture
			int size = new_end_of_tar - (*endsortie);
			int complement = 0;
			if (new_end_of_tar % BLOCKSIZE != 0) { // Complément pour avoir un bloc complet de 512 octets
				complement = BLOCKSIZE - (new_end_of_tar % BLOCKSIZE);
				char block[complement];
				memset(block, '\0', complement);
				if (write(fd_sortie, block, complement) < complement) {
					perror("Impossible de completer le block à la fin du tar!");
					return;
				}
			}
			lseek(fd_sortie, -(size+complement+BLOCKSIZE+(*taillesortie)), SEEK_CUR); // retour à l'emplacement du header
			struct posix_header header;
			char *namepos = strstr(file_sortie, ".tar") + 5; // existe car file_sortie n'est pas juste un tar
			char namefile[strlen(namepos)+1];
			strcpy(namefile, namepos);
			create_header(namefile, &header, size+(*taillesortie));
			if (write(fd_sortie, &header, BLOCKSIZE) < BLOCKSIZE) {
				perror("Impossible d'écirre le header de la sortie standard!");
				return;
			}
			lseek(fd_sortie, 0, SEEK_END); // on se met à la fin
			write_block(fd_sortie, NULL);
			write_block(fd_sortie, NULL); // on ecrit deux blocks vides à la fin au cas ou
		}
		close(fd_sortie);
		if(dup2(save_sortie, STDOUT_FILENO) < 0){
			perror("erreur de redirection");
			return;
		}
		close(save_sortie);
	}

	if (save_erreur != -1 && fd_erreur != fd_entree && fd_erreur != fd_sortie) {
		if (file_erreur != NULL && strstr(file_erreur, ".tar") != NULL) { // On se trouve dans un tar, on doit donc écrire le header
			off_t new_end_of_tar = lseek(fd_erreur, 0, SEEK_CUR);
			int size = new_end_of_tar - (*enderreur);
			int complement;
			if (new_end_of_tar%BLOCKSIZE != 0) {
				complement = BLOCKSIZE-(new_end_of_tar%BLOCKSIZE);
				char block[complement];
				memset(block, '\0', complement);
				if (write(fd_erreur, block, complement) < complement) {
					perror("Impossible de completer le block à la fin du tar!");
					return;
				}
			}else {
				complement = 0;
			}
			lseek(fd_erreur, -(size+complement+BLOCKSIZE+(*tailleerreur)), SEEK_CUR); // retour à l'emplacement du header
			struct posix_header header;
			unsigned int oldsize = 0;
			if (read(fd_erreur, &header, BLOCKSIZE) < BLOCKSIZE) {
				perror("Impossible de lire le header du la sortie erreur!");
				return;
			}
			if (strcmp(header.name, "") != 0) sscanf(header.size, "%o", &oldsize);
			lseek(fd_erreur, -BLOCKSIZE, SEEK_CUR); // retour à l'emplacement du header
			char *namepos = strstr(file_erreur, ".tar") + 5; // existe car file_sortie n'est pas juste un tar
			char namefile[strlen(namepos)+1];
			strcpy(namefile, namepos);

			create_header(namefile, &header, size+oldsize); //On fabrique le header avec le nom du fichier créer et la taille écrite
			if (write(fd_erreur, &header, BLOCKSIZE) < BLOCKSIZE) {
				perror("Impossible d'écirre le header du la sortie erreur!");
				return;
			}
			lseek(fd_erreur, 0, SEEK_END); // on se met à la fin
			write_block(fd_erreur, NULL); // on ecrit un block vide à la fin au cas ou
		}
		close(fd_erreur);
		if(dup2(save_erreur, STDERR_FILENO) < 0){
			perror("erreur de redirection");
			return;
		}
		close(save_erreur);
	}
}

/*
* Calcul dans quel cas on se trouve pour faire la redirection
*/
int traite_redirection(char *file, int type, int *fd, int *save, int *end, int *oldtaille) {
	char *pwd = getcwd(NULL, 0);
	char *twd = getenv("TWD");
	int twdlen = 0;
	int pwdlen = 0;
	if (file[0] != '/') pwdlen = strlen(pwd) + 1;
	if (file[0] != '/' && twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;
	char absolutefile[pwdlen + twdlen + strlen(file) + 1];
	strcpy(absolutefile, "");
	if (pwdlen != 0) {
		strcat(absolutefile, pwd);
		strcat(absolutefile, "/");
	}
	if (twdlen != 0) {
		strcat(absolutefile, twd);
		strcat(absolutefile, "/");
	}
	strcat(absolutefile, file);

	if (strstr(absolutefile, ".tar") != NULL) {
		return redirection_tar(absolutefile, type, fd, save, end, oldtaille);
	}
	else {
		return redirection_classique(absolutefile, type, fd, save);
	}
}
/*
* Redirection mettant en jeu l'écriture ou la lecture d'un fichier dans un tar
*/
int redirection_tar(char *file, int type, int *fd, int *save, int *end, int *oldtaille) {
	if (is_tar_tsh(file)) { // file est un nom d'archive, on ne peut pas écrire dedans
		char format[strlen(file) + 25];
		sprintf(format, "tsh: %s: est une archive\n", file);
		if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			return -1;
		}
		return -1;
	}
	int tarpos = strstr(file, ".tar") - file; // Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
	char tarfile[tarpos+4+1]; // Contient le chemin jusqu'au tar pour l'ouvrir
	strncpy(tarfile, file, tarpos+4);
	tarfile[tarpos+4] = '\0';
	char namefile[strlen(file)-tarpos-4+1]; // Contient la suite du chemin pour l'affichage
	strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
	namefile[strlen(file)-tarpos-4] = '\0';

	struct posix_header header;
	*fd = open(tarfile, O_RDWR);
	if (*fd == -1) {
	  perror("erreur d'ouverture de l'archive");
	  return -1;
	}
	if (type >= 2) { // redirection stout ou stderr
		*oldtaille = 0;
		if (exist_file_in_tar(*fd, namefile)) { // existe donc on le suprrime et remet à la fin
			if (type == 3 || type == 5) { // >> ou 2>> (Concatenation)
				int read_size;
				while(read(*fd, &header, BLOCKSIZE) > 0){
					if(strcmp(header.name, namefile) == 0){
						unsigned int sizefile;
						sscanf(header.size, "%o", &sizefile);
						unsigned int size = (sizefile + BLOCKSIZE - 1) >> BLOCKBITS;
						size *= BLOCKSIZE; // Multiple de 512, pour conserver le bon formatage du tar
						size += BLOCKSIZE; // on ajoute le header
						off_t position = lseek(*fd, -BLOCKSIZE, SEEK_CUR); // avant le header

						char container[size];
						if (read(*fd, container, size) < size) {
							perror("Impossible de lire le contenu du fichier de redirection!");
							return -1;
						}
						off_t endfile = lseek(*fd, 0, SEEK_CUR); // a la fin du block

						unsigned int endsize = lseek(*fd, 0, SEEK_END) - endfile;
						char endcontainer[endsize];
						lseek(*fd, endfile, SEEK_SET); // retour à la fin du block contenu
						if (read(*fd, endcontainer, endsize) < endsize) {
							perror("Impossible de lire la fin de l'archive de redirection!");
							return -1;
						}

						lseek(*fd, position, SEEK_SET); // retour au debut du header du fichier;
						if (write(*fd, endcontainer, endsize) < endsize) {
							perror("Impossible d'écrire la fin de l'archive de redicrection!");
							return -1;
						}
						lseek(*fd, position, SEEK_SET); // retour au debut du header du fichier;
						while (read(*fd, &header, BLOCKSIZE) > 0) { // on recherche la fin du tar
							if (strcmp(header.name, "") == 0) { // fin du tar
								lseek(*fd, -BLOCKSIZE, SEEK_CUR); // retour au niveau du header
								if (write(*fd, container, sizefile+BLOCKSIZE) < sizefile+BLOCKSIZE) { // réécrit le fichier et son contenu
									perror("Impossible d'écrire la fin de l'archive de redirection!");
									return -1;
								}
								*oldtaille = sizefile;
								*end = lseek(*fd, 0, SEEK_CUR); // end not null because not stdin
								*save = dup((type < 4)? STDOUT_FILENO : STDERR_FILENO);
								if ((dup2(*fd, (type < 4)? STDOUT_FILENO : STDERR_FILENO) < 0)) {
									perror("Erreur de redirection");
									return -1;
								}
								return 0;
							}
							get_header_size_tsh(&header, &read_size);
							if (lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1) {
								perror("erreur de lecture de l'archive");
								return -1;
							}
						}
						return -1; // erreur si on arrive ici
					}else {
						get_header_size_tsh(&header, &read_size);
						if (lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1) {
							perror("erreur de lecture de l'archive");
							return -1;
						}
					}
				}
				return 0;
			}else { // > ou 2> (Truncation)
				char *argvbis[3] = {"rm", file, NULL};
				rm_func(2, argvbis);
			} // pas de return pour rentrer dans le if suivant
		}
		if (exist_path_in_tar(*fd, namefile)) { // vérifie que l'arborescence de fichier existe dans le tar
			int n = 0;
			int read_size = 0;
			while ((n=read(*fd, &header, BLOCKSIZE)) > 0) {
				if(strcmp(header.name, "") == 0) { // a la fin du tar
					lseek(*fd, -BLOCKSIZE, SEEK_CUR); // On remonte d'un block pour écrire au bon endroit
					write_block(*fd, NULL); // Place pour le header
					*end = lseek(*fd, 0, SEEK_CUR); // end n'est pas nul car on à écrit du contenu
					*save = dup((type < 4)? STDOUT_FILENO : STDERR_FILENO);
					if ((dup2(*fd, (type < 4)? STDOUT_FILENO : STDERR_FILENO) < 0)) {
						perror("Erreur de redirection");
						return -1;
					}
					return 0;
				}else {
					get_header_size_tsh(&header, &read_size);
					if(lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
						perror("erreur de lecture de l'archive");
						return -1;
					}
				}
			}
		}else {
			char format[strlen(file) + 50];
			sprintf(format, "tsh: %s: le dossier parent n'existe pas\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)) {
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}else { //redirection de stdin
		if (exist_file_in_tar(*fd, namefile)) {
			int n = 0;
			int read_size = 0;
			int readen = 0;
			char read_block[BLOCKSIZE];
			while ((n=read(*fd, &header, BLOCKSIZE)) > 0) {
				if (strcmp(header.name, "") == 0) {
					int tube[2];
					pipe(tube);
					get_header_size_tsh(&header, &read_size);
					switch (fork()) {
						case -1:
							perror("fork");
							return -1;
						case 0:
							close(tube[0]);
							while((readen = read(*fd, read_block, BLOCKSIZE)) > 0){
								write(tube[1], read_block, readen);
							}
						default:
							close(tube[1]);
							dup2(tube[0], STDIN_FILENO);
					}
				}
				else{
					get_header_size_tsh(&header, &read_size);
					if(lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
						perror("erreur de lecture de l'archive");
						return -1;
					}
				}
			}
		}
		else{
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	return 0;
}


int redirection_classique(char *file, int type, int *fd, int *save){
	if(type == 5){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_APPEND, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	else if(type == 4){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	else if(type == 3){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_APPEND, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	else if(type == 2){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	else if(type == 1){
		if((*fd = open(file, O_RDONLY)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				return -1;
			}
			return -1;
		}
	}
	*save = dup(type/2);
	if((dup2(*fd, (type < 4)?((type < 2)?STDIN_FILENO:STDOUT_FILENO):STDERR_FILENO) < 0)){
		perror("Erreur de redirection");
		return -1;
	}
	return 0;
}

void selectCommand(char *line, int readen) { //lance la bonne commande ou lance avec exec
//les commandes spéciales à ce shell
	if (iscmd(line, "help")) //cmd = help
		launchFunc(help, line, readen);

	else if (iscmd(line, "exit")) //cmd = exit must be built-in func
		launchBuiltInFunc(exit_tsh, line, readen);

//les commandes spéciales pour les tar
	else if (hasTarIn(line, readen)) //custom commands if implies to use tarball
		selectCustomCommand(line, readen);

//les commandes built-in n'agissant pas dans les tar
	else if (iscmd(line, "cd")) //cmd = cd must be built-in func
		launchBuiltInFunc(cdIn, line, readen);

//execution normale de la command
	else //lancer la commande avec exec
		launchFunc(exec, line, readen);
}

void selectCustomCommand(char *line, int readen) { //lance la bonne custom commande ou lance avec exec
//les commandes built-in
	if (iscmd(line, "cd")) //cmd = cd must be built-in func
		launchBuiltInFunc(cd, line, readen);

//lance la commande si elle a été implémentée
	else if (isIn(line, custom, len_custom))
		launchFunc(getFun(line, custom, len_custom), line, readen);

//essaie de lancer la commande avec exec sinon
	else
		launchFunc(exec, line, readen);
}

int launchBuiltInFunc(int (*func)(int, char *[]), char *line, int readen) { //lance la fonction demandée directement en processus principal
	int argc = getNbArgs(line, readen);
	char *argv[argc+1];
	argv[0] = strtok(line, " ");
	for (int i = 1; i <= argc; i++) {
		argv[i] = strtok(NULL, " ");
	}
	return func(argc, argv);
}

int launchFunc(int (*func)(int, char *[]), char *line, int readen) { //lance dans un nouveau processus la fonction demandée et attend qu'elle finisse
	int argc = getNbArgs(line, readen);
	char *argv[argc+1];
	argv[0] = strtok(line, " ");
	for (int i = 1; i <= argc; i++) {
		argv[i] = strtok(NULL, " ");
	}

	int status;
	int pid = fork();
	switch(pid) {
		case -1: {//erreur
			perror("Erreur de fork!");
			return -1;
		}
		case 0: {//fils
			func(argc, argv);
			exit(EXIT_SUCCESS);
		}
		default: {//pere
			if (waitpid(pid, &status, 0) < 0) {
				perror("Erreur de wait!");
				exit(EXIT_FAILURE);
			}
			if (!WIFEXITED(status)) {
				char *erreur = "Erreur lors l'execution de la commande!\n";
				int erreur_len = strlen(erreur);
				if (write(STDERR_FILENO, erreur, erreur_len) < erreur_len) {
					perror("Erreur d'écriture dans le shell!");
					return -1;
				}
			}
		}
	}
	return 0;
}

int exec(int argc, char *argv[]) { //lance une commande
	if (execvp(argv[0], &argv[0]) < 0) {
		perror("Erreur d'execution de la commande!");
		exit(EXIT_FAILURE);
	}
	return 0;
}

int getNbArgs(char const *line, int len) { //compte le nombre de mots dans une ligne
	char line_copy[len+1];
	strncpy(line_copy, line, len);
	line_copy[len] = '\0';

	int res = 0;
	char *args;
	if ((args = strtok(line_copy, " ")) != NULL) {
		res++;
		while ((args = strtok(NULL, " ")) != NULL) {
			res++;
		}
	}
	return res;
}

/*
*Vérifie que le chemin path existe bien dans le tar fd (sans le fichier cible)
*/
int exist_path_in_tar(int fd, char *path){
	if(strstr(path, "/") == NULL) return 1;
	char *pos = strrchr(path, '/');
	char pathbis[strlen(path)];
	memset(pathbis, '\0', strlen(path));
	strncpy(pathbis, path, pos-path);
	strcat(pathbis, "/");
	struct posix_header header;
	int n = 0;
	int read_size = 0;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, pathbis) == 0){
			lseek(fd, 0, SEEK_SET);
			return 1;
		}
		get_header_size_tsh(&header, &read_size);
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	lseek(fd, 0, SEEK_SET);
	return 0;
}

/*
*Vérifie que le fichier path existe bien dans le tar fd
*/
int exist_file_in_tar(int fd, char *path){
	struct posix_header header;
	int n = 0;
	int read_size = 0;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, path) == 0){
			lseek(fd, 0, SEEK_SET);
			return 1;
		}
		get_header_size_tsh(&header, &read_size);
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	lseek(fd, 0, SEEK_SET);
	return 0;
}

/*
* verifie si la ligne donnée est uniquement composée d'espaces (\n, ' ', etc...)
*/
int isOnlySpace(char *line, int readen) {
	for(int i = 0; i < readen; i++) {
		if (!isspace(line[i])) return 0;
	}
	return 1;
}

/*
* verifie qu'une ligne commence bien par la commande demandée suivie d'un espace (\n, ' ', etc...)
*/
int iscmd(char *line, char *cmd) {
	return (strncmp(line, cmd, strlen(cmd)) == 0) &&
	((isspace(line[strlen(cmd)])) || (line[strlen(cmd)] == '\0'));
}


/*
* vérifie si la commande utilise un tar dans ses arguments
*/
int hasTarIn(char const *line, int readen) {
	char *env = getenv("TWD");
	if (env != NULL && strlen(env) != 0) return 1; //test si on est déjà dans un tar

	int argc = getNbArgs(line, readen);
	char line_copy[readen+1];
	strcpy(line_copy, line);
	char *argv[argc];
	argv[0] = strtok(line_copy, " "); //nom de la commande
	if (argc == 1) { //no arguments
		if (strcmp(argv[0], "cd") == 0 && strstr(getenv("HOME"), ".tar") != NULL) return 1;
	}
	for (int i = 1; i < argc; i++) {
		argv[i] = strtok(NULL, " ");
		if (strstr(argv[i], ".tar") != NULL) return 1; //test si un tar est explicite dans les arguments
		if (strlen(argv[i]) != 0) { //vérifie les chemins spéciaux ~ et -
			if (argv[i][0] == '~' && strstr(getenv("HOME"), ".tar") != NULL) return 1;
			if (strcmp(argv[i], "-") == 0) {
				char *oldtwd = getenv("OLDTWD");
				if (oldtwd != NULL && strlen(oldtwd) != 0) return 1;
			}
		}
	}
	return 0;
}

/*
*	transforme ~ en HOME dans les arguments de la commande
*/
char *traiterHome(char *line, int *len) {
	int argc = getNbArgs(line, *len);
	char *argv[argc];
	argv[0] = strtok(line, " ");
	int newlen = strlen(argv[0]);

	for (int i = 1; i < argc; i++) {
		char *token = strtok(NULL, " ");

		if (strlen(token) != 0 && token[0] == '~') { //detection de ~
			char *home = getenv("HOME");
			char tmp[strlen(home) + strlen(token)];
			strcpy(tmp, home);
			strcat(tmp, &token[1]);

			argv[i] = malloc(strlen(tmp)+1);
			strcpy(argv[i], tmp);

		}else {
			argv[i] = malloc(strlen(token) + 1);
			strcpy(argv[i], token);
		}

		newlen += strlen(argv[i]);
	}

	*len = newlen + argc - 1;
	char *newline = malloc(newlen + argc + 1);
	strcpy(newline, argv[0]);
	for (int i = 1; i < argc; i++) {
		strcat(newline, " ");
		strcat(newline, argv[i]);
		free(argv[i]);
	}
	return newline;
}

/*
*	modifie les chemins contenant . et .. pour les simplifés
*/
char *traiterArguements(char *line, int *len) {
	int argc = getNbArgs(line, *len);
	char *argv[argc];
	argv[0] = strtok(line, " ");
	int newlen = strlen(argv[0]);

	for (int i = 1; i < argc; i++) {
		char *tok = strtok(NULL, " ");

		if (strstr(tok, ".") != NULL) { //verifie si . ou .. sont contenus dans les arguments de la commande
			int pwdlen = 0;
			int twdlen = 0;
			char *pwd;
			char *twd;
			if (tok[0] != '/') {
				pwd = getcwd(NULL, 0);
				pwdlen = strlen(pwd);
				twd = getenv("TWD");
				if (twd != NULL && strlen(twd) != 0)
					twdlen = strlen(twd) + 1;
			}
			char argvcopy[pwdlen+1+twdlen+strlen(tok)+1];
			if (tok[0] != '/') {
				strcpy(argvcopy, pwd);
				strcat(argvcopy, "/");
				if (twdlen != 0) {
					strcat(argvcopy, twd);
					strcat(argvcopy, "/");
				}
				strcat(argvcopy, tok);
			}else strcpy(argvcopy, tok);

			char argvcopycount[strlen(argvcopy)+1];
			strcpy(argvcopycount, argvcopy);

			char *saveptr;
			char *token;
			int argcount = 0;
			char *tmp = argvcopycount;
			while ((token = strtok_r(tmp, "/", &saveptr)) != NULL) {
			 	argcount += 1;
				tmp = saveptr;
			}

			char *tab[argcount];
			int indice = 0;
			int tab_len = 0;
			tmp = argvcopy;
			for (int i = 0; i < argcount; i++, tmp = saveptr) {
				token = strtok_r(tmp, "/", &saveptr);
				if (strcmp(token, "..") == 0) {
					if (indice != 0) {
						indice--;
						tab_len -= strlen(tab[indice]);
					}
				}else if (strcmp(token, ".") == 0) continue;
				else {
					tab_len += strlen(token);
					tab[indice++] = token;
				}
			}

			char res[1 + tab_len + indice];
			strcpy(res, "");
			for (int j = 0; j < indice; j++) {
				strcat(res, "/");
				strcat(res, tab[j]);
			}

			argv[i] = malloc(strlen(res) + 1);
			strcpy(argv[i], res);
		}else {
			argv[i] = malloc(strlen(tok) + 1);
			strcpy(argv[i], tok);
		}

		newlen += strlen(argv[i]);
	}

	*len = newlen + argc - 1;
	char *newline = malloc(newlen + argc + 1);
	strcpy(newline, argv[0]);
	for (int i = 1; i < argc; i++) {
		strcat(newline, " ");
		strcat(newline, argv[i]);
		free(argv[i]);
	}
	return newline;
}

void get_header_size_tsh(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + BLOCKSIZE-1)/BLOCKSIZE);
}

int create_header(char *name, struct posix_header *header, int size){
	memset(header, '\0', 512);
	for(int i = 0; i < 100; i++){
		if(i < strlen(name)){
			header->name[i] = name[i];
		}else{
			header->name[i] = '\0';
		}
	}
	sprintf(header->mode,"0000755");
	char uid[8];
	char gid[8];
	struct passwd *p = getpwuid(getuid());
	struct group *g = getgrgid(p->pw_gid);
	if(p == NULL){
		for (int i = 0; i < 8; i++) uid[i] = '\0';
	}else{
		sprintf(uid, "%07o", p->pw_uid);
	}
	if(p == NULL){
		for (int i = 0; i < 8; i++) gid[i] = '\0';
	}else{
		sprintf(gid, "%07o", p->pw_gid);
	}
	sprintf(header->uid, "%s", uid);
	sprintf(header->gid, "%s", gid);
	char uname[32];
	char gname[32];
	if(p->pw_name == NULL){
		for (int i = 0; i < 32; i++) uname[i] = '\0';
	}else{
		sprintf(uname, "%s", p->pw_name);
	}
	//write(STDOUT_FILENO, g->gr_name, strlen(g->gr_name));
	if(g->gr_name == NULL){
		for (int i = 0; i < 32; i++) gname[i] = '\0';
	}else{
		sprintf(gname, "%s", g->gr_name);
	}
	sprintf(header->uname, "%s", uname);
	sprintf(header->gname, "%s", gname);
	sprintf(header->size, "%011o", size);

	time_t mtime = time(NULL);
	sprintf(header->mtime, "%11lo", mtime);
	for (int i = 0; i < 8; i++) header->chksum[i] = '\0';
	header->typeflag = '0';
	for (int i = 0; i < 100; i++) header->linkname[i] = '\0';
	header->magic[0] = 'u';
	header->magic[1] = 's';
	header->magic[2] = 't';
	header->magic[3] = 'a';
	header->magic[4] = 'r';
	header->magic[5] = '\0';

	header->version[0] = '0';
	header->version[1] = '0';
	for (int i = 0; i < 8; i++) header->devmajor[i] = '\0';
	for (int i = 0; i < 8; i++) header->devminor[i] = '\0';
	for (int i = 0; i < 155; i++) header->prefix[i] = '\0';
	for (int i = 0; i < 12; i++) header->junk[i] = '\0';

	set_checksum(header);
	if (!check_checksum(header)) {
		char *format = "mkdir : Erreur le header n'est pas conforme!\n";
		if (write(STDERR_FILENO, format, strlen(format)) < strlen(format))
			perror("Erreur d'écriture dans le shell");
		return -1;
	}
	return 0;
}

int write_block(int fd, struct posix_header* header){
	if (header == NULL){
		char block[BLOCKSIZE];
		memset(block, '\0', 512);
		if(write(fd, block, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'archive");
			return -1;
		}
	}else{
		if(write(fd, &header, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'aarchive");
			return -1;
		}
	}
	return 0;
}

int is_tar_tsh(char *file){
	char *pos = strstr(file, ".tar");
	if(pos == NULL) return 0;
	return (strlen(pos) == 4 || strlen(pos) == 5);
}

int in_same_tar(char *file1, char *file2) {
	char *pwd = getcwd(NULL, 0);
	char *twd = getenv("TWD");
	int twdlen = 0;
	int pwdlen = 0;
	if (file1[0] != '/') pwdlen = strlen(pwd) + 1;
	if (file1[0] != '/' && twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;
	char absolutefile1[pwdlen + twdlen + strlen(file1) + 1];
	strcpy(absolutefile1, "");
	if (pwdlen != 0) {
		strcat(absolutefile1, pwd);
		strcat(absolutefile1, "/");
	}
	if (twdlen != 0) {
		strcat(absolutefile1, twd);
		strcat(absolutefile1, "/");
	}
	strcat(absolutefile1, file1);

	twdlen = 0;
	pwdlen = 0;
	if (file2[0] != '/') pwdlen = strlen(pwd) + 1;
	if (file2[0] != '/' && twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;
	char absolutefile2[pwdlen + twdlen + strlen(file2) + 1];
	strcpy(absolutefile2, "");
	if (pwdlen != 0) {
		strcat(absolutefile2, pwd);
		strcat(absolutefile2, "/");
	}
	if (twdlen != 0) {
		strcat(absolutefile2, twd);
		strcat(absolutefile2, "/");
	}
	strcat(absolutefile2, file2);

	char *tar1 = strstr(absolutefile1, ".tar");
	if (tar1 == NULL) return 0;
	char *tar2 = strstr(absolutefile2, ".tar");
	if (tar2 == NULL) return 0;

	return (strncmp(absolutefile1, absolutefile2, strlen(absolutefile1) - strlen(tar1)) == 0);
}
