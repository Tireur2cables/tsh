#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cp.h"

void detectError(int);
int optionDetection(int, char *[]);
void copyto(char *, char *, int);
int exist(char *);

int cp(int argc, char *argv[]) {
// detect error in number of args
	detectError(argc);

// get the index of the option if speciefied
	int indexOpt = optionDetection(argc, argv);

	int indexLastArg = (indexOpt == argc-1)? argc-2 : argc-1;
// dest is last arg
	char dest[strlen(argv[indexLastArg])+1];
	strcpy(dest, argv[indexLastArg]);

// first verify if dest exist
	if (exist(dest)) {
		// then for all sources copy to dest
			for (int i = 1; i < indexLastArg; i++) {
				if (i != indexOpt) { // skip option arg
					char source[strlen(argv[i])+1];
					strcpy(source, argv[i]);
					copyto(source, dest, (indexOpt != 0));
				}
			}
	}

	return 0;
}

void detectError(int argc) { // vérifie qu'un nombre correct d'éléments est donné
	if (argc < 3) {
		char *error = "cp : Il faut donner au moins 2 arguments à cp!";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
}

int optionDetection(int argc, char *argv[]) { //return the position of the option '-r' if specified or return 0
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0) return i;
	}
	return 0;
}

int exist(char *path) {
	struct stat st;
	if (path[0] == '/') {
		char *pos = strstr(path, ".tar");
		if (pos == NULL) {
			if (stat(path, &st) < 0) {
				char *error_fin = " n'existe pas!\n";
				char error[strlen(path)+strlen(error_fin)+1];
				strcpy(error, path);
				strcat(error, error_fin);
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return 0;
			}
		}
	}else {
		char *twd = getenv("TWD");
		char *pwd = getcwd(NULL, 0);
		if (twd != NULL && strlen(twd) != 0) {
			char absolutepath[strlen(pwd)+strlen(twd)+1+strlen(path)+1];
			strcpy(absolutepath, pwd);
			strcat(absolutepath, "/");
			strcat(absolutepath, twd);
			strcat(absolutepath, "/");
			strcat(absolutepath, path);
			if (stat(absolutepath, &st) < 0) {
				char *error_fin = " n'existe pas!\n";
				char error[strlen(absolutepath)+strlen(error_fin)+1];
				strcpy(error, absolutepath);
				strcat(error, error_fin);
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return 0;
			}
		}else {
			char absolutepath[strlen(pwd)+1+strlen(path)+1];
			strcpy(absolutepath, pwd);
			strcat(absolutepath, "/");
			strcat(absolutepath, path);
			if (stat(absolutepath, &st) < 0) {
				char *error_fin = " n'existe pas!\n";
				char error[strlen(absolutepath)+strlen(error_fin)+1];
				strcpy(error, absolutepath);
				strcat(error, error_fin);
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return 0;
			}
		}
	}
	return 1;
}

void copyto(char *source, char *dest, int option) { // copy source to dest with or without option

	if (exist(source)) {

	}

}
