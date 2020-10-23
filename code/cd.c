#define _DEFAULT_SOURCE // utile pour setenv

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <linux/limits.h>
#include "tar.h"
#include "cd.h"

/*
TODO LIST :
- Ne pas setenv à chaque itération mais seulement à a fin
- reconnaitre un chemin comprenant des .. (voir FIXME plus bas)
- Prendre en compte des chemins une fois dans un tar
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

	char chemin[strlen(argv[1]) + 1];
	strcpy(chemin, argv[1]);

	return parcoursChemin(chemin, pwd);
}

int parcoursChemin(char *chemin, char *pwd) {
	char *tmp = strtok(chemin, "/");
	if (tmp != NULL && !isOnlySpaceString(tmp)) {
		if (selectNewPath(tmp, pwd) < 0) return -1;
	}
	if (strlen(tmp)+1 >= strlen(chemin)) { //FIXME boucle une fois de trop ??????????
		while ((tmp = strtok(NULL, "/")) != NULL && !isOnlySpaceString(tmp)) {
			if (selectNewPath(tmp, pwd) < 0) return -1;
		}
	}
	return 0;
}

int selectNewPath(char *tmp, char *pwd) {
	if (strcmp(tmp, "-") == 0) {
		char const *oldtwd = getenv("OLDTWD");
		if (oldtwd == NULL) {
			char *error = "La variables OLDTWD n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char dir_argument[strlen(oldtwd) + 1];
		strcpy(dir_argument, oldtwd);
		setPath(dir_argument, pwd);
	}else if (strcmp(tmp, "~") == 0) {
		char const *home = getenv("HOME");
		if (home == NULL) {
			char *error = "La variables HOME n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char dir_argument[strlen(home) + 1];
		strcpy(dir_argument, home);
		setPath(dir_argument, pwd);
	}else {
		DIR *courant = opendir(pwd);
		if (courant == NULL) {
			char *error1 = "erreur d'ouverture du repertoire\n";
			if (write(STDERR_FILENO, error1, strlen(error1)) < strlen(error1)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}

		char dir_argument[strlen(tmp) + 1];
		strcpy(dir_argument, tmp);
		int found = 0;
		struct stat st;
		struct dirent *d;

		while ((d = readdir(courant)) != NULL) {
			if (strcmp(d->d_name, dir_argument) == 0) {
				found = 1;
				if (strcmp(dir_argument, ".") == 0) {
					break;
				}else if (strcmp(dir_argument, "..") == 0) {
					char *pere = findpere(pwd);
					setPath(pere, pwd);
					free(pere);
					break;
				}else {
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
		}
		if (!found) {
			char *error4 = "fichier ou répertoire non existant !\n";
			if (write(STDERR_FILENO, error4, strlen(error4)) < strlen(error4)) {
				perror("Erreur d'écriture dans le shell!");
			}
			return -1;
		}
		closedir(courant);
	}
	return 0;
}

char *findpere(char *pwd) {
	char pwd_copy[strlen(pwd)+1];
	strcpy(pwd_copy, pwd);

	int len;
	char *tmp = strtok(pwd_copy, "/");
	if (tmp != NULL) {
		len = strlen(tmp);
	}
	while ((tmp = strtok(NULL, "/")) != NULL) {
		len = strlen(tmp);
	}
	char *pere = malloc(strlen(pwd) - len - 1 + 1);
	strncpy(pere, pwd, strlen(pwd) - len - 1);
	pere[strlen(pwd) - len - 1] = '\0';
	return pere;
}

int actuPath(char *new, char *pwd) {
	char newpwd[strlen(pwd) + 1 + strlen(new) + 1];
	strcpy(newpwd, pwd);
	strcat(newpwd, "/");
	strcat(newpwd, new);
	return setPath(newpwd, pwd);
}

int setPath(char *new, char *old) {
	if(setenv("OLDTWD", old, 1) < 0) {
		perror("Changement de variable impossible");
		return -1;
	}
	if(setenv("TWD", new, 1) < 0) {
		perror("Changement de variable impossible");
		return -1;
	}
	return 0;
}


int isTAR(char * dirTAR) {
	char *ext = ".tar";
	return strcmp(&dirTAR[strlen(dirTAR)-strlen(ext)], ext);
}

int isOnlySpaceString(char *mycat_buf) { //verifie si la phrase donnée est uniquement composée d'espaces (\n, ' ', etc...)
	for(int i = 0; i < strlen(mycat_buf); i++) {
		if (!isspace(mycat_buf[i])) return 0;
	}
	return 1;
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
