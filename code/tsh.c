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
#include "mycat.h"
#include "ls.h"

int iscmd(char *, char *);
int isOnlySpace(char *, int);
void selectCommand(int, char *);

char *pwd = ".";

int main(int argc, char const *argv[]) {
	if (argc > 1) {
		errno = E2BIG;
		perror("Arguments non valides!");
		exit(EXIT_FAILURE);
	}

	char *prompt = "$ ";
	int prompt_len = strlen(prompt);
	while (1) {
		if (write(STDOUT_FILENO, prompt, prompt_len) < prompt_len) {
			perror("Erreur d'écriture du prompt!");
			exit(EXIT_FAILURE);
		}
		int readen;
		if ((readen = read(STDIN_FILENO, mycat_buf, BUFSIZE)) < 0) {
			perror("Erreur de lecture dans le shell!");
			exit(EXIT_FAILURE);
		}
		if (readen > 1 && !isOnlySpace(mycat_buf, readen)) {
			selectCommand(readen, mycat_buf);
		}
	}

	return 0;
}

void selectCommand(int readen, char *mycat_buf) {
	if (iscmd(mycat_buf, "exit")) {
		char *fini = "Arrivederci mio signore!\n\n";
		int fini_len = strlen(fini);
		if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}else if (iscmd(mycat_buf, "ls")) {
		int status;
		char mycat_buf_copy[readen+1];
		int pid = fork();
	    switch(pid) {
	    	case -1:
				perror("Erreur de fork!");
	     		exit(EXIT_FAILURE);
	    	case 0: //fils
				strncpy(mycat_buf_copy, mycat_buf, readen);
				mycat_buf_copy[readen] = '\0';
				strtok(mycat_buf, "\n");
				strtok(mycat_buf, " ");
				if (strtok(NULL, " ") == NULL) {
					char *argv2[2];
					argv2[0]="ls";
					argv2[1]=pwd;
					ls(1, argv2);
				}else {
					char *args;
					strtok(mycat_buf_copy, " ");
					args = strtok(NULL, "\n");
					char *argv2[2];
					argv2[0]="ls";
					argv2[1]=args;
					ls(2, argv2);
				}
				exit(EXIT_SUCCESS);
	    	default: //pere
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
	}else if (iscmd(mycat_buf, "help")) {
		char *help = "Voici une liste non exhaustive des commandes implémentées:ǹ\nexit : quitte le tsh\nls : wip\nhelp : obtenir la liste des commandes\n\n";
		int help_len = strlen(help);
		if (write(STDOUT_FILENO, help, help_len) < help_len) {
			perror("Erreur d'écriture dans les shell!");
			exit(EXIT_FAILURE);
		}
	}else { // lancer la commande avec exec
		char *fini = "Commande inconnue!\nEssayez \'help\' pour connaître les commandes disponibles.\n\n";
		int fini_len = strlen(fini);
		if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
}

int isOnlySpace(char *mycat_buf, int readen) {
	for(int i = 0; i < readen; i++) {
		if (!isspace(mycat_buf[i])) return 0;
	}
	return 1;
}


int iscmd(char *mycat_buf, char *cmd) {
	return (strncmp(mycat_buf, cmd, strlen(cmd)) == 0) && (isspace(mycat_buf[strlen(cmd)]));
}
