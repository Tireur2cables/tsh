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

char *pwd; // à mettre dans cd .h

int iscmd(char *, char *);
int isOnlySpace(char *, int);
void selectCommand(int, char *);
int getNbArgs(const char *, int);

int main(int argc, char const *argv[]) {
	if (argc > 1) {
		errno = E2BIG;
		perror("Arguments non valides!");
		exit(EXIT_FAILURE);
	}

	char *prompt = "$ ";
	int prompt_len = strlen(prompt);
	while (1) {
		pwd = getcwd(NULL, 0);
		char new_prompt[strlen(pwd) + 1 + prompt_len + 1];
		strcpy(new_prompt, pwd);
		new_prompt[strlen(pwd)] = ' ';
		new_prompt[strlen(pwd)+1] = '\0';
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
		free(pwd);
	}

	return 0;
}

void selectCommand(int readen, char *mycat_buf) {
	if (iscmd(mycat_buf, "exit")) { //cmd = exit
		char *fini = "Arrivederci mio signore!\n\n";
		int fini_len = strlen(fini);
		if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}else if (iscmd(mycat_buf, "ls")) { //cmd = ls
		int status;
		int pid = fork();
	    switch(pid) {
	    	case -1: {//erreur
				perror("Erreur de fork!");
	     		exit(EXIT_FAILURE);
			}
	    	case 0: {//fils FIXME: c'est ls qui devrait gérer les différents cas? ou alors dans une fonction différente ?
				char mycat_buf_copy[readen+1];
				strncpy(mycat_buf_copy, mycat_buf, readen);
				mycat_buf_copy[readen] = '\0';
				strtok(mycat_buf, " ");
				if (strtok(NULL, " ") == NULL) {
					char *argv2[2];
					argv2[0]="ls";
					argv2[1]=pwd;
					ls(1, argv2);
				}else {
					char *argv2[2];
					argv2[0]="ls";
					argv2[1]=&mycat_buf_copy[3]; //cmd without "ls "
					ls(2, argv2);
				}
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
					if (write(STDOUT_FILENO, erreur, erreur_len) < erreur_len) {
						perror("Erreur d'écriture dans le shell!");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}else if (iscmd(mycat_buf, "pwd")) { //cmd = pwd
		int status;
		int pid = fork();
	    switch(pid) {
	    	case -1: {//erreur
				perror("Erreur de fork!");
	     		exit(EXIT_FAILURE);
			}
	    	case 0: {
				char *tab[1];
				tab[0] = "yolo"; //FIXME temporaire
				pwd_func(1,tab);
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
					if (write(STDOUT_FILENO, erreur, erreur_len) < erreur_len) {
						perror("Erreur d'écriture dans le shell!");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}else if (iscmd(mycat_buf, "cd")) { //cmd = cd
		int status;
		int pid = fork();
	    switch(pid) {
	    	case -1: {//erreur
				perror("Erreur de fork!");
	     		exit(EXIT_FAILURE);
			}
	    	case 0: {//fils FIXME: c'est cd qui devrait gérer les différents cas? ou alors dans une fonction différente ? + ça marhce pas ?
				char mycat_buf_copy[readen+1];
				strncpy(mycat_buf_copy, mycat_buf, readen);
				mycat_buf_copy[readen] = '\0';
				strtok(mycat_buf, " ");
				if (strtok(NULL, " ") == NULL) {
					char *argv2[2];
					argv2[0]="cd";
					argv2[1]=pwd;
					cd(1, argv2);
				}else {
					char *argv2[2];
					argv2[0]="cd";
					argv2[1]=&mycat_buf_copy[3]; //cmd without "cd "
					cd(2, argv2);
				}
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
					if (write(STDOUT_FILENO, erreur, erreur_len) < erreur_len) {
						perror("Erreur d'écriture dans le shell!");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}else if (iscmd(mycat_buf, "help")) { // cmd = help
		char *help = "Voici une liste non exhaustive des commandes implémentées:ǹ\nexit : quitte le tsh\nls : wip\nhelp : obtenir la liste des commandes\n\n";
		int help_len = strlen(help);
		if (write(STDOUT_FILENO, help, help_len) < help_len) {
			perror("Erreur d'écriture dans les shell!");
			exit(EXIT_FAILURE);
		}
	}else { // lancer la commande avec exec
		int status;
		int pid = fork();
	    switch(pid) {
	    	case -1: {//erreur
				perror("Erreur de fork!");
	     		exit(EXIT_FAILURE);
			}
	    	case 0: {//fils
				char mycat_buf_copy[readen+1];
				strncpy(mycat_buf_copy, mycat_buf, readen);
				mycat_buf_copy[readen] = '\0';
				char *cmd;
				cmd = strtok(mycat_buf, " ");
				int args_len = getNbArgs(mycat_buf_copy, readen) + 1;
				char *args[args_len];

				args[0] = strtok(mycat_buf_copy, " ");
				for (int i = 1; i < args_len; i++) {
					args[i] = strtok(NULL, " ");
				}
				if (execvp(cmd, args) < 0) { // probleme pour cd et surement d'autres
					perror("Erreur d'execution de la commande!");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_SUCCESS);
	    	}
			default: {//pere
				if (waitpid(pid, &status, 0) < 0) {
					perror("Erreur de wait!");
					exit(EXIT_FAILURE);
				}
				if (!WIFEXITED(status)) { // jamais atteint
					char *erreur = "Erreur lors l'execution de la commande!\n";
					int erreur_len = strlen(erreur);
					if (write(STDOUT_FILENO, erreur, erreur_len) < erreur_len) {
						perror("Erreur d'écriture dans le shell!");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}
}

int getNbArgs(const char *mycat_buf, int len) {
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

int isOnlySpace(char *mycat_buf, int readen) {
	for(int i = 0; i < readen; i++) {
		if (!isspace(mycat_buf[i])) return 0;
	}
	return 1;
}


int iscmd(char *mycat_buf, char *cmd) {
	return (strncmp(mycat_buf, cmd, strlen(cmd)) == 0) && ((isspace(mycat_buf[strlen(cmd)])) || (mycat_buf[strlen(cmd)] == '\0'));
}
