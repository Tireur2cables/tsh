#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "exit.h"

int exit_tsh(int argc, char *argv[]) {
	if (argc != 1) {
		errno = E2BIG;
		perror("Trop d'arguments!");
		exit(EXIT_FAILURE);
	}

	char *fini = "Arrivederci mio signore!\n\n";
	int fini_len = strlen(fini);
	if (write(STDOUT_FILENO, fini, fini_len) < fini_len) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
