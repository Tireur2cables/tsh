#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "help.h"

int help(int argc, char const *argv[]) {
	if (argc != 1) {
		errno = E2BIG;
		perror("Trop d'arguments!");
		exit(EXIT_FAILURE);
	}

	char *help = "Voici une liste non exhaustive des commandes implémentées:\nexit : quitte le tsh\nls : wip\ncd : wip\npwd : wip\nhelp : obtenir la liste des commandes\n\n";
	int help_len = strlen(help);
	if (write(STDOUT_FILENO, help, help_len) < help_len) {
		perror("Erreur d'écriture dans les shell!");
		exit(EXIT_FAILURE);
	}
	return 0;
}
