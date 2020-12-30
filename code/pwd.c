#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "pwd.h"


int pwd(int argc, char *argv[]) {
	if (argc < 1) {
		errno = EINVAL;
		perror("Pas assez d'arguments!");
		exit(EXIT_FAILURE);
	}

	char *pwd = getcwd(NULL, 0);
	int pwdlen = strlen(pwd);
	char *twd = getenv("TWD");
	int twdlen = 0;
	if (twd != NULL && strlen(twd) != 0) twdlen = 1 + strlen(twd);

	char chemin[pwdlen + twdlen + 1 + 1];
	strcpy(chemin, pwd);
	if (twdlen != 0) {
		strcat(chemin, "/");
		strcat(chemin, twd);
	}
	strcat(chemin, "\n");

	int len = strlen(chemin);
	if(write(STDOUT_FILENO, chemin, len) < len) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}

	return 0;
}
