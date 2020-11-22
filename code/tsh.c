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
#include "ls.h"
#include "cd.h"
#include "pwd.h"
#include "help.h"
#include "exit.h"

//utiliser une struct pour faire une map avec clé string et valeur pointeur function et factoriser ?
int len_builtin = 2;
char const *builtin[2] = {"cd", "exit"};
int len_custom = 9;
char const *custom[9] = {"help", "ls", "pwd", "cat", "cp", "rm", "mv", "rmdir", "mkdir"};

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
int isIn(char *, char const *[], int);;

int main(int argc, char const *argv[]) { //main
	if (argc > 1) {
		errno = E2BIG;
		perror("Arguments non valides!");
		exit(EXIT_FAILURE);
	}
	char const *pwd;
	char const *twd;
	char *prompt = " $ ";
	int prompt_len = strlen(prompt);
	while (1) {
		pwd = getcwd(NULL, 0);
		twd = getenv("TWD");
		int len_twd = 0;
		if (twd != NULL && strlen(twd) != 0)
			len_twd = 1 + strlen(twd);
		char new_prompt[strlen(pwd) + len_twd + prompt_len + 1];
		strcpy(new_prompt, pwd);
		strcat(new_prompt, "/");
		if (twd != NULL && strlen(twd) != 0) strcat(new_prompt, twd);
		strcat(new_prompt, prompt);

		int readen;
		char *mycat_buf;
		if ((mycat_buf = readline(new_prompt)) != NULL) {
			readen = strlen(mycat_buf);
			if (readen > 0 && !isOnlySpace(mycat_buf, readen))
				selectCommand(readen, mycat_buf);
		}else { // EOF detected
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
//les commandes spéciales à ce shell
	if (iscmd(mycat_buf, "exit")) //cmd = exit must be built-in func
		launchBuiltInFunc(exit_tsh, mycat_buf, readen);

	else if (iscmd(mycat_buf, "help")) //cmd = help
		launchFunc(help, mycat_buf, readen);

//les commandes spéciales pour les tar
	else if (hasTarIn(mycat_buf, readen)) //custom commands if implies to use tarball
		selectCustomCommand(readen, mycat_buf);

//les commandes builtin qui ne doivent pas faire de EXIT
	if (iscmd(mycat_buf, "cd")) //cmd = cd must be built-in func
		launchBuiltInFunc(cdIn, mycat_buf, readen);

//execution normale de la commande
	else //lancer la commande avec exec
		launchFunc(exec, mycat_buf, readen);
}

void selectCustomCommand(int readen, char *mycat_buf) { //lance la bonne custom commande ou lance avec exec
	if (iscmd(mycat_buf, "ls")) //cmd = ls
		launchFunc(ls, mycat_buf, readen);

	else if (iscmd(mycat_buf, "pwd")) //cmd = pwd
		launchFunc(pwd, mycat_buf, readen);

	else if (iscmd(mycat_buf, "cat")) { //cmd = cat
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else if (iscmd(mycat_buf, "cp")) { //cmd = cp
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else if (iscmd(mycat_buf, "rm")) { //cmd = rm
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else if (iscmd(mycat_buf, "mv")) { //cmd = mv
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else if (iscmd(mycat_buf, "mkdir")) { //cmd = mkdir
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else if (iscmd(mycat_buf, "rmdir")) { //cmd = rmdir
		if (write(STDOUT_FILENO, "WIP\n", 4) < 4) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	else //lancer la commande avec exec
		launchFunc(exec, mycat_buf, readen);
}

int launchBuiltInFunc(int (*func)(int, char *[]), char *mycat_buf, int readen) { //lance la fonction demandée directement en processus principal
	int argc = getNbArgs(mycat_buf, readen);
	char *argv[argc+1];
	argv[0] = strtok(mycat_buf, " ");
	for (int i = 1; i <= argc; i++) {
		argv[i] = strtok(NULL, " ");
	}
	func(argc, argv);
	return 0;
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

int cdIn(int argc, char *argv[]) {
	return chdir(argv[1]);
}

int exec(int argc, char *argv[]) { //lance une commande
	char *argvbis[2+argc+1];
	argvbis[0] = "bash";
	argvbis[1] = "-c";
	for (int i = 0; i <= argc; i++)
		argvbis[i+2] = argv[i];
	if (execvp(argvbis[0], argvbis) < 0) {
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

int isIn(char *cmd, char const *tab[], int len) {
	for (int i = 0; i < len; i++) {
		if (strcmp(cmd, tab[i]) == 0) return 1;
	}
	return 0;
}

int hasTarIn(char const *mycat_buf, int readen) { //vérifie si la commande utilise un tar dans ces arguments
	char const *env = getenv("TWD");
	if (env != NULL && strlen(env) != 0) return 1; //test si on est déjà dans un tar

	int argc = getNbArgs(mycat_buf, readen);
	char mycat_buf_copy[readen+1];
	strcpy(mycat_buf_copy, mycat_buf);
	char *argv[argc];
	argv[0] = strtok(mycat_buf_copy, " ");
	for (int i = 1; i < argc; i++) {
		argv[i] = strtok(NULL, " ");
		if (strstr(argv[i], ".tar") != NULL) return 1; //test si un tar est explicite dans les arguments
	}
	return 0;
}
