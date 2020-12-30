#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cp.h"
#include "rm.h"

void verifArgs(int, char *[]);

int mv(int argc, char *argv[]) {
// detect error in number of args
	verifArgs(argc, argv);

// cp sources to dest
	int argcbis = argc + 1;
	char *argvbis[argcbis + 1];
	for (int i = 0; i < argc; i++) argvbis[i] = argv[i];
	argvbis[argc] = "-r";
	argvbis[argcbis] = NULL;
	cp(argcbis, argvbis);

// rm sources but not dest (last argument)
	rm_func(argc-1, argv);

	return 0;
}

void verifArgs(int argc, char *argv[]) { // vérifie qu'un nombre correct de chemins est donné
	if (argc < 3) {
		char *error = "mv : Il faut donner au moins 2 chemins à mv!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
}
