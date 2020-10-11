#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mycat.h"
#include "ls.h"

int iscmd(char *, char *);
void selectCommand(int, char *);


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
		if (readen > 1) {
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
	}else if (iscmd(mycat_buf, "ls")) { //lancer en tant que processus fils
		char mycat_buf_copy[readen+1];
		strncpy(mycat_buf_copy, mycat_buf, readen);
		mycat_buf_copy[readen] = '\0';
		strtok(mycat_buf, "\n");
		strtok(mycat_buf, " ");
		if (strtok(NULL, " ") == NULL) {
			char *argv2[2];
			argv2[0]="ls";
			argv2[1]=".";
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
	}else if (iscmd(mycat_buf, "help")) {//lancer en tant que processus fils?
		char *help = "Voici une liste non exhaustive des commandes implémentées:ǹ\nexit : quitte le tsh\nls : wip\nhelp : obtenir la liste des commandes\n\n";
		int help_len = strlen(help);
		if (write(STDOUT_FILENO, help, help_len) < help_len) {
			perror("Erreur d'écriture dans les shell!");
			exit(EXIT_FAILURE);
		}
	}else {
		char *fini = "Commande inconnue!\nEssayez \'help\' pour connaître les commandes disponibles.\n\n";
		int fini_len = strlen(fini);
		if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
}


int iscmd(char *mycat_buf, char *cmd) {
	return (strncmp(mycat_buf, cmd, strlen(cmd)) == 0) && (isspace(mycat_buf[strlen(cmd)]));
}
