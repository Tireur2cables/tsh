#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "pwd.h"


int pwd(int argc, char *argv[]) {
	if (argc == 0) {
		errno = EINVAL;
		perror("Pas assez d'arguments!");
		exit(EXIT_FAILURE);
	}

	char const *twd = getenv("TWD");
	if (twd == NULL) {
		char *error = "Erreur de lecture de TWD!\n";
		if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
	}
	int len = strlen(twd);
	char wd[len + 1 + 1];
	strcpy(wd, twd);
	wd[len] = '\n';
	wd[++len] = '\0';

	if(write(STDOUT_FILENO, wd, len) < len) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}

	return 0;
}
