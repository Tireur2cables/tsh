#define _DEFAULT_SOURCE // utile pour setenv

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
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

	if (argc == 1) { //cd sans arguments
		char const *home = getenv("HOME");
		if (home == NULL) {
			char *error = "La variables HOME n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char copy[strlen(home) + 1];
		strcpy(copy, home);
		return setPath(copy, twd);
	}

	char chemin[strlen(argv[1]) + 1];
	strcpy(chemin, argv[1]);
	char *realchemin;
	int retour;
	char *root = "/";
/*
Détéction des chemins spéciaux
*/
	if (strcmp(chemin, "~") == 0) {
		char const *home = getenv("HOME");
		if (home == NULL) {
			char *error = "La variables HOME n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char copy[strlen(home) + 1];
		strcpy(copy, home);
		return setPath(copy, twd);
	}
	if (strcmp(chemin, "-") == 0) {
		char const *oldtwd = getenv("OLDTWD");
		if (oldtwd == NULL) {
			char *error = "La variables OLDTWD n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char copy[strlen(oldtwd) + 1];
		strcpy(copy, oldtwd);
		return setPath(copy, twd);
	}

	if (strncmp(chemin, "~/", 2) == 0) {
		char const *home = getenv("HOME");
		if (home == NULL) {
			char *error = "La variables HOME n'est pas définie!\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			}
			return -1;
		}
		char copy[strlen(home) + 1];
		strcpy(copy, home);
		char copytmp[strlen(copy) + 1];
		strcpy(copytmp, copy);
		realchemin = getRealChemin(&chemin[2], copytmp);
		if (realchemin == NULL) return -1;
		retour = parcoursChemin(realchemin, copy, root);
		free(realchemin);
		return retour;
	}
	if (strncmp(chemin, "/", 1) == 0) {
		char *rootcopy = "/";
		if (strlen(chemin) == 1) return setPath(root, twd);
		realchemin = getRealChemin(&chemin[1], rootcopy);
		if (realchemin == NULL) return -1;
		retour = parcoursChemin(realchemin, twd, root);
		free(realchemin);
		return retour;
	}

	if (strstr(twd, ".tar") != NULL) return appelSurTar(twd, chemin);

	char twdcopy[strlen(twd) + 1];
	strcpy(twdcopy, twd);
	realchemin = getRealChemin(chemin, twdcopy);
	if (realchemin == NULL) return -1;

	retour = parcoursChemin(realchemin, twd, root);
	free(realchemin);
	return retour;
}

int parcoursChemin(char *chemin, char *oldtwd, char *res) {
	if (chemin == NULL)	return setPath(res, oldtwd);
	if (strcmp(chemin, "/") == 0) return setPath(res, oldtwd);
	int lenchemin = strlen(chemin);
	char chemin_cpy[lenchemin + 1];
	strcpy(chemin_cpy, chemin);

	char *doss = strtok(chemin_cpy, "/");
	int lendoss = strlen(doss);

	if (isAccessibleFrom(doss, res) > 0) {
		int lenres = strlen(res);
		if (strcmp(res, "/") == 0) lenres--;
		char newtwd[lenres + 1 + lendoss + 1];
		if (strcmp(res, "/") != 0) strcpy(newtwd, res);
		newtwd[lenres] = '/';
		newtwd[lenres+1] = '\0';
		strcat(newtwd, doss);
		if (lendoss+1 >= lenchemin) return parcoursChemin(NULL, oldtwd, newtwd);

		if (isTar(doss)) return parcoursTar(&chemin[1+lendoss+1], oldtwd, newtwd);
		return parcoursChemin(&chemin[lendoss+1], oldtwd, newtwd);
	}else return -1;
}

int parcoursTar(char *chemin, char *oldtwd, char *tar) {
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
		if (nom[strlen(nom)-1] == '/'  && isSameDir(chemin, nom)) found = 1;
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
		setPath(res, oldtwd);
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
				char absolutedoss[strlen(dir) + 1 + strlen(doss) + 1];
				strcpy(absolutedoss, dir);
				absolutedoss[strlen(dir)] = '/';
				absolutedoss[strlen(dir)+1] = '\0';
				strcat(absolutedoss, doss);
				if (stat(absolutedoss, &st) < 0) {
					perror("Erreur de stat!");
					return -1;
				}
				if (!S_ISDIR(st.st_mode)) { //not directory
					char *tmp = " n'est pas un répertoire!\n";
					char error[strlen(absolutedoss) + strlen(tmp) + 1];
					strcpy(error, absolutedoss);
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
	if(argc < 1) {
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

int findTarIn(char const *chemin, int res) {
	if (chemin == NULL) return -1;
	char copy[strlen(chemin)+1];
	strcpy(copy, chemin);
	char *doss = strtok(copy, "/");
	if (isTar(doss)) return res;
	int lendoss = strlen(doss);
	return findTarIn(&chemin[lendoss], res+lendoss);
}

int appelSurTar(char *twd, char *chemin) {
	int indice = findTarIn(twd, 1);
	char twdcopy[strlen(twd)+1];
	strcpy(twdcopy, twd);

	char *tar = strtok(&twdcopy[indice], "/");
	int lentar = strlen(tar);

	int len = strlen(&twd[indice+lentar]);
	if(len != 0) len = strlen(&twd[indice+lentar+1]) + 1; //utile ? cas ou juste dans un tar
	char newchemin[len + strlen(chemin) + 1];
	if (len != 0) {
		strcpy(newchemin, &twd[indice+lentar+1]);
		newchemin[len-1] = '/';
		newchemin[len] = '\0';
		strcat(newchemin, chemin);
	}else {
		strcpy(newchemin, chemin);
	}

	char tarchemin[strlen(twd) - len + 1];
	strncpy(tarchemin, twd, strlen(twd) - len);
	tarchemin[strlen(twd) - len] = '\0';

	int retour;
	char tarchemincopy[strlen(tarchemin)+1];
	strcpy(tarchemincopy, tarchemin);
	char *realchemin = getRealChemin(newchemin, tarchemincopy);
	if (realchemin == NULL) return -1;
	if (strlen(realchemin) <= strlen(tarchemin)) retour = parcoursChemin(realchemin, twd, "/");
	else retour = parcoursTar(&realchemin[strlen(tarchemin)+1], twd, tarchemin);
	free(realchemin);
	return retour;
}

int getLenLast(char const *chemin) {
	if (chemin == NULL) {
		errno = ENOENT;
		perror("Erreur impossible de trouver le pere!");
		return -1;
	}
	if (strcmp(chemin, "/") == 0) return 1;
	char copy[strlen(chemin) + 1];
	strcpy(copy, chemin);

	char *arg = strtok(copy, "/");
	if (arg == NULL) {
		errno = ENOENT;
		perror("Erreur pour obtenir la taille du pere!");
		return -1;
	}
	int len = strlen(arg);
	while ((arg = strtok(NULL, "/")) != NULL) {
		len = strlen(arg);
	}
	return len;
}

char *getRealChemin(char *chemin, char *res) {
	if (chemin == NULL) {
		char *newres = malloc(strlen(res)+1);
		assert(newres);
		strcpy(newres, res);
		return newres;
	}
	char copy[strlen(chemin) + 1];
	strcpy(copy, chemin);

	char *doss = strtok(copy, "/");
	if (doss == NULL) return getRealChemin(NULL, res);

	if (strcmp(doss, ".") == 0) {
		if (strlen(doss)+1 >= strlen(chemin)) return getRealChemin(NULL, res);
		return getRealChemin(&chemin[strlen(doss)+1], res);
	}
	if (strcmp(doss, "..") == 0) {
		int lenlast = getLenLast(res);
		if (lenlast < 0) return NULL;
		if (lenlast == strlen(res)) {
			if (strlen(doss)+1 >= strlen(chemin)) return getRealChemin(NULL, res);
			return getRealChemin(&chemin[strlen(doss)+1], res);
		}
		int lennewres = strlen(res) - lenlast - 1;
		if (lennewres == 0) lennewres++;
		char newres[lennewres + 1];
		strncpy(newres, res, lennewres);
		newres[lennewres] = '\0';
		if (strlen(doss)+1 >= strlen(chemin)) return getRealChemin(NULL, newres);
		return getRealChemin(&chemin[strlen(doss)+1], newres);
	}

	int lenres = strlen(res);
	if (strcmp(res, "/") == 0) lenres--;
	char newres[lenres + 1 + strlen(doss) + 1];
	if (strcmp(res, "/") != 0) strcpy(newres, res);
	newres[lenres] = '/';
	newres[lenres+1] = '\0';
	strcat(newres, doss);
	if (strlen(doss)+1 >= strlen(chemin)) return getRealChemin(NULL, newres);
	return getRealChemin(&chemin[strlen(doss)+1], newres);

}
