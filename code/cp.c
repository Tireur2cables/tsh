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
