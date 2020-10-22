#define _XOPEN_SOURCE

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
- Modifier pwd.c pour qu'il passe aussi par TWD
- Modifier aussi la variable OLDTWD pour la commande 'cd -' qui revient en arriève
- Prendre en compte les dossiers meme avec '/' à la fin
- Prendre en compte les chemins
- Factorisation
*/

int cd(int argc,char **argv) {

	if (errorDetect(argc)) return -1;

	char const *twd = getenv("TWD");
	if (twd == NULL) {
		char *error = "Erreur de lecture de TWD!\n";
		if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
			perror("Erreur d'écriture dans le shell");
			return -1;
		}
		return -1;
	}
	char pwd[strlen(twd)];
	strcpy(pwd, twd);

	DIR *courant = opendir(pwd);
	if (courant == NULL) {
		char *error1 = "erreur d'ouverture du repertoire\n";
		if (write(STDERR_FILENO, error1, strlen(error1)) < strlen(error1)) {
			perror("Erreur d'écriture dans le shell");
			return -1;
		}
		return -1;
	}

	int found = 0;
	struct stat st;

	if (argc == 2) { //test inutile par rapport au test de errorDetect ?
		char dir_argument[strlen(argv[1])];
		strcpy(dir_argument, argv[1]);
		struct dirent *d;

		while ((d = readdir(courant)) != NULL) {

				if (strcmp(d->d_name, dir_argument) == 0) {
					found = 1;
					if (isTAR(dir_argument) == 0) {
						printf("OK TAR\n");
						char newpwd[strlen(pwd) + 1 + strlen(dir_argument) + 1];
						strcpy(newpwd, pwd);
						strcat(newpwd, "/");
						strcat(newpwd, dir_argument);

						char newENV[strlen(newpwd)+4+1];
						strcpy(newENV, "TWD=");
						strcat(newENV, newpwd);

						if(putenv(newENV) != 0) {
							perror("Changement de variable impossible");
							return -1;
						}
						break;
					}

					if (stat(dir_argument, &st) < 0) {
						perror("Erreur de stat!");
						return -1;
					}
					if (S_ISDIR(st.st_mode) != 0) {
						printf("OK REP\n");
						char newpwd[strlen(pwd) + 1 + strlen(dir_argument) + 1];
						strcpy(newpwd, pwd);
						strcat(newpwd, "/");
						strcat(newpwd, dir_argument);

						char newENV[strlen(newpwd)+4+1];
						strcpy(newENV, "TWD=");
						strcat(newENV, newpwd);

						if(putenv(newENV) != 0) {
							perror("Changement de variable impossible");
							return -1;
						}
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

			} // fin while

			if (!found) {
				char *error4 = "fichier ou répertoire non existant !\n";
				if (write(STDERR_FILENO, error4, strlen(error4)) < strlen(error4)) {
					perror("Erreur d'écriture dans le shell!");
				}
				return -1;
			}
		}

	closedir(courant);
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
