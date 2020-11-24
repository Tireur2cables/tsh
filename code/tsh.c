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
#include "couple.h"
#include "ls.h"
#include "cd.h"
#include "pwd.h"
#include "help.h"
#include "exit.h"
#include "cdIn.h"

//todo list
// redirection < > >> 2>>
// tube |

int iscmd(char *, char *);
int isOnlySpace(char *, int);
void selectCommand(int, char *);
void selectCustomCommand(int, char *);
int getNbArgs(char const *, int);
int launchFunc(int (*)(int, char *[]), char *, int);
int launchBuiltInFunc(int (*)(int, char *[]), char *, int);
int exec(int, char *[]);
int cdIn(int, char *[]);
int hasTarIn(char const *, int);

//tableau (et sa taille) des commandes implémentées (non built-in) pour les tar
int len_custom = 3; //8
couple custom[3] = {{"ls", ls}, {"pwd", pwd}};
//, {"cat", cat}, {"cp", cp}, {"rm", rm}, {"mv", mv}, {"rmdir", rmdir}, {"mkdir", mkdir};


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
			len_twd = 1 + strlen(twd);

		char new_prompt[strlen(pwd) + len_twd + strlen(prompt) + 1];
		strcpy(new_prompt, pwd);
		if (strcmp(pwd, "/") != 0) //seulement si on est pas à la racine
			strcat(new_prompt, "/");
		if (twd != NULL && strlen(twd) != 0) //seulement si besoin
			strcat(new_prompt, twd);
		strcat(new_prompt, prompt);

		char *mycat_buf;
		if ((mycat_buf = readline(new_prompt)) != NULL) {
			int readen = strlen(mycat_buf);
			if (readen > 0 && !isOnlySpace(mycat_buf, readen)) //if not empty line
				selectCommand(readen, mycat_buf);
		}else { //EOF detected
			char *newline = "\n";
			int newline_len = strlen(newline);
			if (write(STDOUT_FILENO, newline, newline_len) < newline_len) {
				perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}
		free(mycat_buf);
	}

	return 0;
}

void selectCommand(int readen, char *mycat_buf) { //lance la bonne commande ou lance avec exec
//les commandes spéciales pour les tar
	if (hasTarIn(mycat_buf, readen)) //custom commands if implies to use tarball
		selectCustomCommand(readen, mycat_buf);

//les commandes spéciales à ce shell
	else if (iscmd(mycat_buf, "help")) //cmd = help
		launchFunc(help, mycat_buf, readen);

//les commandes built-in
	else if (iscmd(mycat_buf, "cd")) //cmd = cd must be built-in func
		launchBuiltInFunc(cdIn, mycat_buf, readen);

	else if (iscmd(mycat_buf, "exit")) //cmd = exit must be built-in func
		launchBuiltInFunc(exit_tsh, mycat_buf, readen);

//execution normale de la command
	else //lancer la commande avec exec
		launchFunc(exec, mycat_buf, readen);
}

void selectCustomCommand(int readen, char *mycat_buf) { //lance la bonne custom commande ou lance avec exec
//les commandes built-in
	if (iscmd(mycat_buf, "cd")) //cmd = cd must be built-in func
		launchBuiltInFunc(cd, mycat_buf, readen);

//lance la commande si elle a été implémentée
	else if (isIn(mycat_buf, custom, len_custom))
		launchFunc(getFun(mycat_buf, custom, len_custom), mycat_buf, readen);

//essaie de lancer la commande avec exec sinon
	else
		launchFunc(exec, mycat_buf, readen);
}

int launchBuiltInFunc(int (*func)(int, char *[]), char *mycat_buf, int readen) { //lance la fonction demandée directement en processus principal
	int argc = getNbArgs(mycat_buf, readen);
	char *argv[argc+1];
	argv[0] = strtok(mycat_buf, " ");
	for (int i = 1; i <= argc; i++) {
		argv[i] = strtok(NULL, " ");
	}
	return func(argc, argv);
}

int launchFunc(int (*func)(int, char *[]), char *mycat_buf, int readen) { //lance dans un nouveau processus la fonction demandée et attend qu'elle finisse
	int argc = getNbArgs(mycat_buf, readen);
	char *argv[argc+1];
	argv[0] = strtok(mycat_buf, " ");
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
// utiliser bash -c comme commande pour lancer les commandes ?
	if (execvp(argv[0], argv) < 0) {
		perror("Erreur d'execution de la commande!");
		exit(EXIT_FAILURE);
	}
	return 0;
}

int getNbArgs(char const *mycat_buf, int len) { //compte le nombre de mots dans une phrase
	char mycat_buf_copy[len+1];
	strncpy(mycat_buf_copy, mycat_buf, len);
	mycat_buf_copy[len] = '\0';

	int res = 0;
	char *args;
	if ((args = strtok(mycat_buf_copy, " ")) != NULL) {
		res++;
		while ((args = strtok(NULL, " ")) != NULL) {
			res++;
		}
	}
	return res;
}

int isOnlySpace(char *mycat_buf, int readen) { //verifie si la phrase donnée est uniquement composée d'espaces (\n, ' ', etc...)
	for(int i = 0; i < readen; i++) {
		if (!isspace(mycat_buf[i])) return 0;
	}
	return 1;
}

int iscmd(char *mycat_buf, char *cmd) { //verifie qu'une phrase commence bien par la commande demandée suivie d'un espace (\n, ' ', etc...)
	return (strncmp(mycat_buf, cmd, strlen(cmd)) == 0) &&
	((isspace(mycat_buf[strlen(cmd)])) || (mycat_buf[strlen(cmd)] == '\0'));
}

int hasTarIn(char const *mycat_buf, int readen) { //vérifie si la commande utilise un tar dans ces arguments
	char const *env = getenv("TWD");
	if (env != NULL && strlen(env) != 0) return 1; //test si on est déjà dans un tar

	int argc = getNbArgs(mycat_buf, readen);
	char mycat_buf_copy[readen+1];
	strcpy(mycat_buf_copy, mycat_buf);
	char *argv[argc];
	argv[0] = strtok(mycat_buf_copy, " "); //nom de la commande inutile de verifier
	for (int i = 1; i < argc; i++) {
		argv[i] = strtok(NULL, " ");
		if (strstr(argv[i], ".tar") != NULL) return 1; //test si un tar est explicite dans les arguments
		if (strlen(argv[i]) != 0) { //vérifie les chemins spéciaux ~ et -
			if (argv[i][0] == '~' && strstr(getenv("HOME"), ".tar") != NULL) return 1;
			if (strcmp(argv[i], "-") == 0 && strstr(getenv("OLDPWD"), ".tar") != NULL) return 1;
		}
	}
	return 0;
}
