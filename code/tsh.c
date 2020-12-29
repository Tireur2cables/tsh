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
#include "couple.h"
#include "ls.h"
#include "cd.h"
#include "pwd.h"
#include "cat.h"
#include "help.h"
#include "exit.h"
#include "cdIn.h"
#include "tar.h"

//todo list
// redirection < > >> 2>>
// tube |

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
char *traiterArguements(char *, int *);
void parse_command(char *, int);
void parse_tube(char *, int);
void parse_redirection(char *, int);
void traite_redirection(char *, char *, int);
char *traiterHome(char *, int *);
int exist_path_in_tar(int, char *);
int exist_file_in_tar(int, char *);
void redirection_tar(char *, char*, int);
void redirection_classique(char *, char *, int);
int create_header(char *, struct posix_header *, int);
int write_block(int fd, struct posix_header *);

//tableau (et sa taille) des commandes implémentées (non built-in) pour les tar
int len_custom = 3; //8
couple custom[3] = {{"ls", ls}, {"pwd", pwd}, {"cat", cat}};
//, {"cp", cp}, {"rm", rm}, {"mv", mv}, {"rmdir", rmdir}, {"mkdir", mkdir};


int main(int argc, char const *argv[]) { //main
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
			if (readen > 0 && !isOnlySpace(line, readen)) //if not empty line
				parse_command(line, readen);
				//selectCommand(line, readen);
		}else { //EOF detected
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

void parse_command(char *line, int readen){
	parse_redirection(line, readen);
	//parse_tube(line, readen);

}
/*
* POUR LE MOMENT ON NE GERE PAS COMMAND < INPUT > OUTPUT
*/
void parse_redirection(char *line, int readen){
	int fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur = -1;
	char *pos;
	if((pos = strstr(line, ">")) != NULL){ //On doit faire une redirection de la sortie standard ou de la sortie erreur
		if((pos = strstr(line, "2>"))){ //redirection de la sortie erreur
			char command[strlen(line)];
			char file[strlen(line)];
			memset(file, '\0', strlen(line));
			memset(command, '\0', strlen(line));
			strncpy(command, line, pos-line);
			if(pos[2] == '>'){ //2>>
				strcpy(file, (pos[2] == ' ')?pos+4:pos+3);
				traite_redirection(file, 5, &fd_erreur, &save_erreur);
			}else{ //2>
				strcpy(file, (pos[2] == ' ')?pos+3:pos+2);
				traite_redirection(file, 4, &fd_erreur, &save_erreur);
			}
		}
		if(strstr(line, "2>") == NULL && (pos = strstr(line, ">"))){ //redirection de la sortie standard
			char command[strlen(line)];
			char file[strlen(line)];
			memset(command, '\0', strlen(line));
			memset(file, '\0', strlen(line));
			strncpy(command, line, pos-line);
			if(pos[1] == '>'){ //>>
				strcpy(file, (pos[2] == ' ')?pos+3:pos+2);
				traite_redirection(file, 3, &fd_sortie, &save_sortie);
			}else{ //>
				strcpy(file, (pos[1] == ' ')?pos+2:pos+1);
				traite_redirection(file, 2, &fd_sortie, &save_sortie);
			}
		}
	}
	if((pos = strstr(line, "<")) != NULL){ //On doit faire une reditction de l'entrée
		char command[strlen(line)];
		char file[strlen(line)];
		memset(command, '\0', strlen(line));
		memset(file, '\0', strlen(line));
		strncpy(command, line, pos-line);
		strcpy(file, (pos[1] == ' ')?pos+2:pos+1);
		traite_redirection(file, 1, &fd_entree, &save_entree);
	}
	selectCommand(line, readen);
	close_redirections(fd_entree, fd_sortie, fd_erreur, save_entree, save_sortie, save_erreur, file);
}

void close_redirections(int fd_entree, int fd_sortie, int fd_erreur, int save_entree, int save_sortie, int save_erreur, char *file){
	if(save_entree != -1){
		close(fd_entree);
		if(dup2(save_entree, STDIN_FILENO) < 0){
			perror("erreur de redirection");
			exit(EXIT_FAILURE);
		}
		close(save_entree);
	}
	if(save_sortie != -1){
		if(strstr(file, ".tar") != NULL){
		 //On doit en plus faire le traitement pour le tar
		}
		close(fd_sortie);
		if(dup2(save_sortie, STDOUT_FILENO) < 0){
			perror("erreur de redirection");
			exit(EXIT_FAILURE);
		}
		close(save_sortie);
	}
	if(save_erreur != -1){
		if(strstr(file, ".tar") != NULL){
		//On doit en plus faire le traitement pour le tar
		}
		close(fd_erreur);
		if(dup2(save_erreur, STDERR_FILENO) < 0){
			perror("erreur de redirection");
			exit(EXIT_FAILURE);
		}
		close(save_erreur);
	}
}

void traite_redirection(char *file, int type, int *fd, int *save){
	if(strstr(file, ".tar") != NULL){
		redirection_tar(file, type, fd, save);
	}
	else{
		redirection_classique(file, type, fd, save);
	}
}
void redirection_tar(char *file, int type, int *fd, int *save){
	char tarfile[strlen(file)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(file)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(file, ".tar") - file; //Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
	strncpy(tarfile, file, tarpos+4);
	strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(file)-tarpos-4] = '\0';
	struct posix_header header;
	//write(STDERR_FILENO, namefile, strlen(namefile));
	*fd = open(tarfile, O_RDWR);
	if(*fd == -1){
	  perror("erreur d'ouverture de l'archive");
	  exit(EXIT_FAILURE);
	}
	if(type >= 2){//redirection stout ou stderr
		if(exist_file_in_tar(*fd, namefile)){
			write(STDERR_FILENO, "WIP", 3);
		}else{
			if(exist_path_in_tar(*fd, namefile)){ //vérifie que l'arborescence de fichier existe dans le tar
				//write(STDERR_FILENO, namefile, strlen(namefile));
				int n = 0;
				int read_size = 0;
				//write(STDERR_FILENO, "yes", 3);
				while((n=read(*fd, &header, BLOCKSIZE)) > 0){
					//write(STDERR_FILENO, "yes", 3);
					if(strcmp(header.name, "\0") == 0){
						off_t end_of_tar;
						off_t new_end_of_tar;
						struct posix_header header2;
						lseek(*fd, -BLOCKSIZE, SEEK_CUR); //On remonte d'un block pour écrire au bon endroit
						write_block(*fd, NULL); //Place pour le header
						end_of_tar = lseek(*fd, 0, SEEK_CUR);
						*save = dup((type < 4)?STDOUT_FILENO:STDERR_FILENO);
						if((dup2(*fd, (type < 4)?STDOUT_FILENO:STDERR_FILENO) < 0)){
							perror("Erreur de redirection");
							exit(EXIT_FAILURE);
						}
						//Ceci doit etre dans close();
						new_end_of_tar = lseek(*fd, 0, SEEK_CUR);
						int size = new_end_of_tar-end_of_tar;
						int complement;
						if(size%BLOCKSIZE != 0){
							complement = BLOCKSIZE-(size%BLOCKSIZE);
							char block[complement];
							memset(block, '\0', complement);
							write(*fd, block, complement);
						}else{
							complement = 0;
						}
						lseek(*fd, -(size+complement+BLOCKSIZE), SEEK_CUR);
						create_header(namefile, &header2, size);
						write(*fd, &header2, BLOCKSIZE);
						return;
					}else{
						get_header_size_tsh(&header, &read_size);
						if(lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
							perror("erreur de lecture de l'archive");
							return;
						}
					}
				}
			}else{
				char format[strlen(file) + 60];
				sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
				if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
					perror("Erreur d'écriture dans le shell");
					exit(EXIT_FAILURE);
				}
				return;
			}
		}
	}else{ //redirection de stdin
		if(exist_file_in_tar(*fd, namefile)){
			int n = 0;
			int read_size = 0;
			while((n=read(*fd, &header, BLOCKSIZE)) > 0){
				if(strcmp(header.name, "\0") == 0){
					//Faire le traitement de l'input ici'
				}
				else{
					get_header_size_tsh(&header, &read_size);
					if(lseek(*fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
						perror("erreur de lecture de l'archive");
						return;
					}
				}
			}
		}
		else{
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
		}
	}

}


void redirection_classique(char *file, int type, int *fd, int *save){
	if(type == 5){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_APPEND, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	else if(type == 4){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	else if(type == 3){
		if((*fd = open(file, O_WRONLY + O_CREAT + O_APPEND, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	else if(type == 2){
		//write(STDOUT_FILENO, command, strlen(command));
		if((*fd = open(file, O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	else if(type == 1){
		if((*fd = open(file, O_RDONLY)) < 0){
			char format[strlen(file) + 60];
			sprintf(format, "tsh: %s: Aucun dossier ou fichier de ce type\n", file);
			if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	*save = dup(type/2);
	if((dup2(*fd, (type < 4)?((type < 2)?STDIN_FILENO:STDOUT_FILENO):STDERR_FILENO) < 0)){
		perror("Erreur de redirection");
		exit(EXIT_FAILURE);
	}

}

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

void parse_tube(char *line, int readen){
	if(!strstr(line, "|")){
		selectCommand(line, readen);
	}else{
		char cp[strlen(line)];
		memset(cp, '\0', strlen(cp));
		int k = 0;
		for(int i = 0; i < strlen(line); i++){
			if(!isspace(line[i])){
				cp[k++] = line[i];
			}
			if (line[i] == '|'){
				cp[--k] = '\0';
				printf("%s\n", cp);
				selectCommand(cp, k);
				memset(cp, '\0', strlen(cp));
				k = 0;
			}
		}
		selectCommand(cp, k);
	}
}

void selectCommand(char *line, int readen) { //lance la bonne commande ou lance avec exec
//traite le ~ au début des arguments
	line = traiterHome(line, &readen);

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
//traite les arguments si presence de . .. ou ~
	line = traiterArguements(line, &readen);
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
			exit(EXIT_FAILURE);
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
					exit(EXIT_FAILURE);
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

int isOnlySpace(char *line, int readen) { //verifie si la ligne donnée est uniquement composée d'espaces (\n, ' ', etc...)
	for(int i = 0; i < readen; i++) {
		if (!isspace(line[i])) return 0;
	}
	return 1;
}

int iscmd(char *line, char *cmd) { //verifie qu'une ligne commence bien par la commande demandée suivie d'un espace (\n, ' ', etc...)
	return (strncmp(line, cmd, strlen(cmd)) == 0) &&
	((isspace(line[strlen(cmd)])) || (line[strlen(cmd)] == '\0'));
}

int hasTarIn(char const *line, int readen) { //vérifie si la commande utilise un tar dans ces arguments
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

char *traiterHome(char *line, int *len) { //transforme ~ en HOME dans les arguments passés
	int argc = getNbArgs(line, *len);
	char *argv[argc];
	argv[0] = strtok(line, " ");
	int newlen = strlen(argv[0]);

	for (int i = 1; i < argc; i++) {
		char *token = strtok(NULL, " ");

		if (strlen(token) != 0 && token[0] == '~') { //detection of ~
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
	char *newline = malloc(newlen + argc);
	strcpy(newline, argv[0]);
	for (int i = 1; i < argc; i++) {
		strcat(newline, " ");
		strcat(newline, argv[i]);
		free(argv[i]);
	}
	return newline;
}

char *traiterArguements(char *line, int *len) { //modifie les chemins contenant . et .. pour les simplifés
	int argc = getNbArgs(line, *len);
	char *argv[argc];
	argv[0] = strtok(line, " ");
	int newlen = strlen(argv[0]);

	for (int i = 1; i < argc; i++) {
		char *tok = strtok(NULL, " ");

		if (strstr(tok, ".") != NULL) { //verifie si . ou .. sont contenus
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
	char *newline = malloc(newlen + argc);
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
	*read_size = ((taille + 512-1)/512);
}

int create_header(char *name, struct posix_header *header, int size){ //Meilleur méthode pour le faire ?
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
			exit(EXIT_FAILURE);
		}
	}else{
		if(write(fd, &header, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'aarchive");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
