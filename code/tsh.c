#define _XOPEN_SOURCE

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
#include <linux/limits.h>
#include <readline/readline.h>
#include "ls.h"
#include "cd.h"
#include "pwd.h"

/* utiliser un tableau des commandes implémentées pour facotriser encore plus ? */

//char pwd[PATH_MAX];

int iscmd(char *, char *);
int isOnlySpace(char *, int);
void selectCommand(int, char *);
int getNbArgs(const char *, int);
int launchFunc(int (*)(int, char *[]), char *, int);
int launchBuiltInFunc(int (*)(int, char *[]), char *, int);
int exec(int, char *[]);
void setEnv();

int main(int argc, char const *argv[]) { //main
	if (argc > 1) {
		errno = E2BIG;
		perror("Arguments non valides!");
		exit(EXIT_FAILURE);
	}
	//setEnv();

	char *oldtwd = "TWD=";
	char *oldpwd = getcwd(NULL, 0);
	char setTWD[strlen(oldpwd) + strlen(oldtwd) + 1];
	strcpy(setTWD, oldtwd);
	strcat(setTWD, oldpwd);
	if (putenv(setTWD) != 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);

	char const *twd;
	char *prompt = "$ ";
	int prompt_len = strlen(prompt);
	while (1) {
		twd = getenv("TWD");
		char new_prompt[strlen(twd) + 1 + prompt_len + 1];
		strcpy(new_prompt, twd);
		new_prompt[strlen(twd)] = ' ';
		new_prompt[strlen(twd)+1] = '\0';
		strcat(new_prompt, prompt);

		int readen;
		char *mycat_buf;
		if ((mycat_buf = readline (new_prompt)) != NULL) {
			readen = strlen(mycat_buf);
			if (readen > 0 && !isOnlySpace(mycat_buf, readen)) {
				selectCommand(readen, mycat_buf);
			}
		}else {
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
	if (iscmd(mycat_buf, "exit")) { //cmd = exit
		char *fini = "Arrivederci mio signore!\n\n";
		int fini_len = strlen(fini);
		if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}else if (iscmd(mycat_buf, "help")) { //cmd = help //FIXME : A faire comme une fonction / commande a part
		char *help = "Voici une liste non exhaustive des commandes implémentées:\nexit : quitte le tsh\nls : wip\ncd : wip\npwd : wip\nhelp : obtenir la liste des commandes\n\n";
		int help_len = strlen(help);
		if (write(STDOUT_FILENO, help, help_len) < help_len) {
			perror("Erreur d'écriture dans les shell!");
			exit(EXIT_FAILURE);
		}
	}else if (iscmd(mycat_buf, "cd")) { //cmd = cd must be built-in func
		launchBuiltInFunc(cd, mycat_buf, readen);
	}else if (iscmd(mycat_buf, "ls")) { //cmd = ls
		launchFunc(ls, mycat_buf, readen);
	}else if (iscmd(mycat_buf, "pwd")) { //cmd = pwd
		launchFunc(pwd_func, mycat_buf, readen);
	}else { //lancer la commande avec exec
		launchFunc(exec, mycat_buf, readen);
	}
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

int exec(int argc, char *argv[]) { //lance une commande
	if (execvp(argv[0], argv) < 0) {
		perror("Erreur d'execution de la commande!");
		exit(EXIT_FAILURE);
	}
	return 0;
}

int getNbArgs(const char *mycat_buf, int len) { //compte le nombre de mots dans une phrase
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

void setEnv() {
	char *twd = "TWD=";
	char *oldpwd = getcwd(NULL, 0);
	char setTWD[strlen(oldpwd) + strlen(twd) + 1];
	strcpy(setTWD, twd);
	strcat(setTWD, oldpwd);
	if (putenv(setTWD) != 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);
}
