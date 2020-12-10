#define _DEFAULT_SOURCE // utile pour setenv
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cdIn.h"

int cdIn(int argc, char *argv[]) {
	if (argc > 2) {
		errno = E2BIG;
		perror("Trop d'arguments!");
		return 0;
	}

	char *actual = getcwd(NULL, 0);

	if (argv[1] == NULL || strlen(argv[1]) == 0) { //cd without arg = go back to $HOME
		char *home = getenv("HOME");
		if (home == NULL || strlen(home) == 0) {
			errno = EINVAL;
			perror("Home indisponible!");
			return 0;
		}
		if (chdir(home) == -1) {
			perror("Erreur de cd!");
			return 0;
		}
	}else if (strcmp(argv[1], "-") == 0) { //path is - as OLDPWD
		char *oldpwd = getenv("OLDPWD");
		if (oldpwd == NULL || strlen(oldpwd) == 0) {
			errno = EINVAL;
			perror("OLDPWD indisponible!");
			return 0;
		}
		if (chdir(oldpwd) == -1) {
			perror("Erreur de cd!");
			return 0;
		}
	}else { //normal case
		if (chdir(argv[1]) == -1) {
			perror("Erreur de cd!");
			return 0;
		}
	}

	if(setenv("OLDPWD", actual, 1) < 0) {
		perror("Changement de variable OLDPWD impossible");
		return 0;
	}

	return 0;
}
