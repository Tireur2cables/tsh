#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mycat.h"
//#include "ls.h"

int is_ls(char *);


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
			perror("Erreur de lecture dans le prompt!");
			exit(EXIT_FAILURE);
		}
		if ((readen > 1) && (strncmp(mycat_buf, "exit", readen-1) == 0)) {
			char *fini = "Arrivederci mio signore!\n\n";
			int fini_len = strlen(fini);
			if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
				perror("Erreur d'écriture du prompt!");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}else if (is_ls(mycat_buf)) {
			char mycat_buf_copy[BUFSIZE];
			strncpy(mycat_buf_copy, mycat_buf, readen);
			strtok(mycat_buf, " ");
			char *fic;
			if (strtok(NULL, " ") == NULL) {
				printf("ls sans arguments\n"); // FIXME: temporaire
			}else {
				strtok(mycat_buf_copy, " ");
				fic = strtok(NULL, "\n");
				printf("ls avec arguments : %s\n", fic); // FIXME: temporaire
			}
		}else {
			char *fini = "Commande inconnue!\nEssayez \'help\' pour connaître les commandes disponibles.\n\n";
			int fini_len = strlen(fini);
			if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
				perror("Erreur d'écriture du prompt!");
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}

int is_ls(char *mycat_buf) {
	return (strncmp(mycat_buf, "ls", 2) == 0) && (isspace(mycat_buf[2]));
}
