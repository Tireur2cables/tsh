#define _DEFAULT_SOURCE // utile pour setenv

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <linux/limits.h>
#include "tar.h"
#include "cd.h"

/*
TODO LIST :
- Gérer le cas . et .. et - et ~
- Prendre en compte les dossiers meme avec '/' à la fin
- Prendre en compte les chemins
*/

int cd(int argc,char **argv) {

	if (errorDetect(argc)) return -1;

	char const *twd = getenv("TWD");
	if (twd == NULL) {
		char *error = "Erreur de lecture de TWD!\n";
		if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
			perror("Erreur d'écriture dans le shell");
		}
		return -1;
	}
	char pwd[strlen(twd) + 1];
	strcpy(pwd, twd);

	DIR *courant = opendir(pwd);
	if (courant == NULL) {
		char *error1 = "erreur d'ouverture du repertoire\n";
		if (write(STDERR_FILENO, error1, strlen(error1)) < strlen(error1)) {
			perror("Erreur d'écriture dans le shell");
		}
		return -1;
	}

	char *dir_argument;
	char tmp[strlen(argv[1]) + 1];
	strcpy(tmp, argv[1]);
	if (strcmp(tmp, "-") == 0) {
		char const *oldtwd = getenv("OLDTWD");
		if (oldtwd == NULL) {
			char *error = "La variables OLDTWD n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		dir_argument = malloc(strlen(oldtwd)+1);
		assert(dir_argument);
		strcpy(dir_argument, oldtwd);
	}else if ((strcmp(tmp, "~/") == 0) || (strcmp(tmp, "~") == 0)) {
		char const *home = getenv("HOME");
		if (oldtwd == NULL) {
			char *error = "La variables HOME n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		dir_argument = malloc(strlen(home)+1);
		assert(dir_argument);
		strcpy(dir_argument, home);
	}else if ((strcmp(tmp, "./") == 0) || (strcmp(tmp, ".") == 0)) {
		dir_argument = malloc(strlen(pwd) + 1);
		assert(dir_argument);
		strcpy(dir_argument, pwd);
	}else if ((strcmp(tmp, "../") == 0) || (strcmp(tmp, "..") == 0) {
		//aller vers le pere
	}else {
		dir_argument = malloc(strlen(tmp) + 1);
		assert(dir_argument);
		strcpy(dir_argument, tmp);
	}

	int found = 0;
	struct stat st;
	struct dirent *d;

	while ((d = readdir(courant)) != NULL) {
		if (strcmp(d->d_name, dir_argument) == 0) {
			found = 1;
			if (isTAR(dir_argument) == 0) { //tar
				if (actuPath(dir_argument, pwd) < 0) return -1;
				break;
			}
			if (stat(dir_argument, &st) < 0) {
				perror("Erreur de stat!");
				return -1;
			}
			if (S_ISDIR(st.st_mode) != 0) { // dossier
				if (actuPath(dir_argument, pwd) < 0) return -1;
				break;
			}else {
				char *tmp = " n'est pas un répertoire!\n";
				char error3[strlen(dir_argument)+strlen(tmp)+1];
				strcpy(error3, dir_argument);
				strcat(error3, tmp);
				if (write(STDERR_FILENO, error3, strlen(error3)) < strlen(error3)) {
					perror("Erreur d'écriture dans le shell!");
				}
				return -1;
			}
		}

	}

	if (!found) {
		char *error4 = "fichier ou répertoire non existant !\n";
		if (write(STDERR_FILENO, error4, strlen(error4)) < strlen(error4)) {
			perror("Erreur d'écriture dans le shell!");
		}
		return -1;
	}
	free(dir_argument);
	closedir(courant);
	return 0;
}

int actuPath(char *new, char *pwd) {
	char newpwd[strlen(pwd) + 1 + strlen(new) + 1];
	strcpy(newpwd, pwd);
	strcat(newpwd, "/");
	strcat(newpwd, new);
	if(setenv("OLDTWD", pwd, 1) < 0) {
		perror("Changement de variable impossible");
		return -1;
	}
	if(setenv("TWD", newpwd, 1) < 0) {
		perror("Changement de variable impossible");
		return -1;
	}
	return 0;
}


int isTAR(char * dirTAR) {
	char *ext = ".tar";
	return strcmp(&dirTAR[strlen(dirTAR)-strlen(ext)], ext);
}

int errorDetect(int argc) {
	if(argc <= 1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		return -1;
	}

	if(argc>2) {
		errno = E2BIG;
		perror("trop d'arguments !");
		return -1;
	}

	return 0;
}
