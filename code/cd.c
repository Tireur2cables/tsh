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

int cd(int argc,char **argv) {

	if (errorDetect(argc)) return -1;

	char const *oldtwd = getenv("TWD");
	if (oldtwd == NULL) {
		char *error = "Erreur de lecture de TWD!\n";
		if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
			perror("Erreur d'écriture dans le shell");
		}
		return -1;
	}
	char twd[strlen(oldtwd) + 1];
	strcpy(twd, oldtwd);
	char newtwd[strlen(twd) + 1];
	strcpy(newtwd, twd);

	char chemin[strlen(argv[1]) + 1];
	strcpy(chemin, argv[1]);

	if (strstr(twd, ".tar") != NULL) {
		int indice = findTarIn(twd, 1);
		char *tar = strtok(&newtwd[indice], "/");
		int len = strlen(tar);
		char newchemin[strlen(&twd[indice+len]) + 1 + strlen(chemin) + 1];
		strcpy(newchemin, &twd[indice+len]);
		newchemin[strlen(&twd[indice+len])] = '/';
		newchemin[strlen(&twd[indice+len])+1] = '\0';
		strcat(newchemin, chemin);

		char t[strlen(twd) -(len+strlen(newchemin)) + 1];
		//strncpy(tar, twd, strlen(twd) - strlen(&twd[indice+len]) + 1); //FIXME:bug
		printf("%s %s\n", &newchemin[1], tar);
		return 0;
		//return parcoursTar(&newchemin[1], twd, tar);
	}
	return parcoursChemin(chemin, twd, newtwd);
}

int parcoursChemin(char *chemin, char *twd, char *res) {
	if (chemin == NULL)	return setPath(res, twd);
	int lenchemin = strlen(chemin);
	char chemin_cpy[lenchemin + 1];
	strcpy(chemin_cpy, chemin);

	char *doss = strtok(chemin_cpy, "/");
	int lendoss = strlen(doss);
// FIXME : check les cas spéciaux . .. ~ -
	if (isAccessibleFrom(doss, res)) {
		int lenres = strlen(res);
		char newtwd[lenres + 1 + lendoss + 1];
		strcpy(newtwd, res);
		newtwd[lenres] = '/';
		newtwd[lenres+1] = '\0';
		strcat(newtwd, doss);

		if (lendoss >= lenchemin) return parcoursChemin(NULL, twd, newtwd);

		if (isTar(doss)) return parcoursTar(&chemin[lendoss+1], twd, newtwd);
		return parcoursChemin(&chemin[lendoss+1], twd, newtwd);
	}else return -1;
}

int parcoursTar(char *chemin, char *twd, char *tar) {
	int fd = open(tar, O_RDONLY);
	if (fd == -1) {
		perror("Erreur d'ouverture du tar!");
		return -1;
	}
	struct posix_header header;
	int found = 0;
	do {
		if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;
		char nom[strlen(header.name)+1];
		strcpy(nom, header.name);
		if (isSameDir(chemin, nom)) found = 1;
		else {
			if (strcmp(nom, "") == 0) break;
			unsigned int taille;
			sscanf(header.size, "%o", &taille);
			taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
			taille *= BLOCKSIZE;
			if (lseek(fd, (off_t) taille, SEEK_CUR) == -1) break;
		}
	} while(!found);
	close(fd);
	if (!found) {
		errno = ENOENT;
		perror("impossible de trouver le dossier dans le tar!");
	}else {
		int lenres = strlen(tar) + 1 + strlen(chemin) + 1;
		char res[lenres];
		strcpy(res, tar);
		res[strlen(tar)] = '/';
		res[strlen(tar)+1] = '\0';
		strcat(res, chemin);
		if (chemin[strlen(chemin)-1] == '/') res[lenres-2] = '\0';
		setPath(res, twd);
	}
	return (found)? 0 : -1;
}

int isAccessibleFrom(char *doss, char *dir) {
	DIR *courant = opendir(dir);
	if (courant == NULL) {
		errno = ENOENT;
		perror("Erreur d'ouverture du répertoire!");
		return -1;
	}
	int found = 0;
	struct stat st;
	struct dirent *d;
	while ((d = readdir(courant)) != NULL) {
		if (strcmp(d->d_name, doss) == 0) {
			found = 1;
			if (!isTar(doss)) { //not tar so should be directory
				if (stat(doss, &st) < 0) {
					perror("Erreur de stat!");
					return -1;
				}
				if (!S_ISDIR(st.st_mode)) { //not directory
					char *tmp = " n'est pas un répertoire!\n";
					char error[strlen(doss) + strlen(tmp) + 1];
					strcpy(error, doss);
					strcat(error, tmp);
					if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
						perror("Erreur d'écriture dans le shell!");
					}
					return -1;
				}
			}
			break;
		}
	}
	closedir(courant);
	if (!found) {
		errno = ENOENT;
		perror("Erreur dossier ou archive introuvable!");
	}
	return found;
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

int errorDetect(int argc) {
	if(argc <= 1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		return -1;
	}

	if(argc > 2) {
		errno = E2BIG;
		perror("trop d'arguments !");
		return -1;
	}

	return 0;
}

int isTar(char *dirTAR) {
	char *ext = ".tar";
	return (strcmp(&dirTAR[strlen(dirTAR)-strlen(ext)], ext) == 0);
}

int isSameDir(char *dir1, char *dir2) {
	return (strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}

int findTarIn(char *chemin, int res) {
	if (chemin == NULL) return -1;
	char copy[strlen(chemin)+1];
	strcpy(copy, chemin);
	char *doss = strtok(copy, "/");
	if (isTar(doss)) return res;
	int lendoss = strlen(doss);
	return findTarIn(&chemin[lendoss], res+lendoss);
}

/*

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


		char dir_argument[strlen(tmp) + 1];
		strcpy(dir_argument, tmp);
		int found = 0;




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
*/
