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
		if (strncmp(mycat_buf, "exit", readen-1) == 0) { //probleme quand on appui sur entré direct
			char *fini = "Arrivederci mio signore!\n\n";
			int fini_len = strlen(fini);
			if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
				perror("Erreur d'écriture du prompt!");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}else if (is_ls(mycat_buf)) { //ls marche pas
			strtok(mycat_buf, " ");
			char *fic;
			if ((fic = strtok(NULL, "\n")) == NULL) {
				char *t = "./ls";
				ls(1, &t);
			}else {
				printf("%s\n", fic);
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
