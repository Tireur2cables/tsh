#define _DEFAULT_SOURCE // utile pour setenv et strtok_r

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tar.h"
#include "cd.h"

int setWd(char *);
int parcoursChemin(char *, char *);
int isAccessibleFrom(char *, char *);
int parcoursCheminTar(char *, char *, char *);
int isSameDir(char *, char *);
int errorDetect(int);

int cd(int argc, char *argv[]) {

	if (errorDetect(argc)) return -1;

	if (argc == 1) { // cd sans arguments
		char *home = getenv("HOME");
		if (home == NULL) {
			char *error = "La variables $HOME n'est pas définie!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell");
			return -1;
		}
		return setWd(home);
	}

	char chemin[strlen(argv[1]) + 1];
	strcpy(chemin, argv[1]);

	if (strcmp(chemin, "-") == 0) { // cd -
		char *oldpwd = getenv("OLDPWD");
		if (oldpwd == NULL || strlen(oldpwd) == 0) {
			char *error = "La variables OLDPWD n'est pas définie!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell");
			return -1;
		}

		char *oldtwd = getenv("OLDTWD");
		int oldtwd_len = 0;
		if (oldtwd != NULL && strlen(oldtwd) != 0)
			oldtwd_len = 1 + strlen(oldtwd);

		char copy[strlen(oldpwd) + oldtwd_len + 1];
		strcpy(copy, oldpwd);
		if (oldtwd_len > 0) {
			strcat(copy, "/");
			strcat(copy, oldtwd);
		}
		return setWd(copy);
	}

	if (chemin[0] != '/') {
		char *pwd = getcwd(NULL, 0);
		char *twd = getenv("TWD");
		if (twd != NULL && strlen(twd) != 0)
			return parcoursCheminTar(pwd, twd, chemin);
		return parcoursChemin(pwd, chemin);
	}else return parcoursChemin("/", chemin);
}

int setWd(char *newwd) {

	char *oldpwd = getcwd(NULL, 0);
	if(setenv("OLDPWD", oldpwd, 1) < 0) {
		perror("Erreur changement de la variable OLDPWD impossible!");
		return -1;
	}

	char *oldtwd = getenv("TWD");
	if (oldtwd != NULL && strlen(oldtwd) != 0) {
		if(setenv("OLDTWD", oldtwd, 1) < 0) {
			perror("Erreur changement de la variable OLDTWD impossible!");
			return -1;
		}
	}else {
		if (unsetenv("OLDTWD") < 0) {
			perror("Erreur de suppression de la variable OLDTWD!");
			return -1;
		}
	}

	char newwd_copy[strlen(newwd) + 1];
	strcpy(newwd_copy, newwd);

	char *token;
	char *saveptr;
	int pwdlen = 0;
	while ((token = strtok_r(newwd, "/", &saveptr)) != NULL) {
		if (strstr(token, ".tar") != NULL) break;
		pwdlen += 1 + strlen(token);
		newwd = saveptr;
	}
	char pwd[pwdlen+1];
	strncpy(pwd, newwd_copy, pwdlen);
	pwd[pwdlen] = '\0';

	if (chdir(pwd) < 0) {
		perror("Erreur impossible de changer le dossier courant!");
		return -1;
	}

	int twdlen = strlen(newwd_copy) - pwdlen - 1;
	if (twdlen > 0) {
		char twd[twdlen+1];
		strcpy(twd, &newwd_copy[pwdlen+1]);

		if (setenv("TWD", twd, 1) < 0) {
			perror("Erreur changement de la variable TWD impossible!");
			return -1;
		}
	}else {
		if (unsetenv("TWD") < 0) {
			perror("Erreur de suppression de la variable TWD!");
			return -1;
		}
	}

	return 0;
}


int parcoursChemin(char *pwd, char *chemin) {
	char *saveptr;
	char *doss;
	char *current = malloc(strlen(pwd) + 1);
	strcpy(current, pwd);

	while ((doss = strtok_r(chemin, "/", &saveptr)) != NULL) {

		if (isAccessibleFrom(doss, current) > 0) { // is doss accessible from current

			if (strstr(doss, ".tar") != NULL) {
				char newcurr[strlen(current) + 1];
				strcpy(newcurr, current);
				free(current);
				return parcoursCheminTar(newcurr, doss, saveptr);
			}

			int currentlen = strlen(current);
			if (currentlen != 1) currentlen++;
			char newcurr[currentlen + strlen(doss) + 1];
			strcpy(newcurr, current);
			if (currentlen != 1) strcat(newcurr, "/");
			strcat(newcurr, doss);

			current = realloc(current, strlen(newcurr)+1);
			strcpy(current, newcurr);
			chemin = saveptr;

		}else {
			free(current);
			return -1;
		}
	}
	char res[strlen(current) + 1];
	strcpy(res, current);
	free(current);
	return setWd(res);
}

int isAccessibleFrom(char *doss, char *dir) {
	DIR *courant = opendir(dir);
	if (courant == NULL) {
		char *error_debut = "cd : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + strlen(dir) + 1 + 1];
		sprintf(error, "%s%s\n", error_debut, dir);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	int found = 0;
	struct stat st;
	struct dirent *d;
	while ((d = readdir(courant)) != NULL) {
		if (strcmp(d->d_name, doss) == 0) {
			found = 1;
			if (strstr(doss, ".tar") == NULL) { // not tar so should be directory
				char absolutedoss[strlen(dir) + 1 + strlen(doss) + 1];
				sprintf(absolutedoss, "%s/%s", dir, doss);

				if (stat(absolutedoss, &st) < 0) {
					perror("Erreur de stat!");
					return -1;
				}

				if (!S_ISDIR(st.st_mode)) { // not directory
					char *deb  = "cd : ";
					char *end = " n'est pas un répertoire!\n";
					char error[strlen(deb) + strlen(absolutedoss) + strlen(end) + 1];
					sprintf(error, "%s%s%s", deb, absolutedoss, end);
					int errorlen = strlen(error);
					if (write(STDERR_FILENO, error, errorlen) < errorlen)
						perror("Erreur d'écriture dans le shell!");
					return -1;
				}
			}
			break;
		}
	}
	closedir(courant);
	if (!found) {
		char *deb  = "cd : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(dir) + 1 + strlen(doss) + strlen(end) + 1];
		sprintf(error, "%s%s/%s%s", deb, dir, doss, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	return found;
}

int parcoursCheminTar(char *pwd, char *twd, char *rest) {
	char twd_copy[strlen(twd)+1];
	strcpy(twd_copy, twd);
	char *tar = strtok(twd_copy, "/");
	char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
	sprintf(absolutetar, "%s/%s", pwd, tar);

	char chemin[strlen(twd) - strlen(tar) + strlen(rest) + 1];
	if (strlen(twd) - strlen(tar) > 0) strcpy(chemin, &twd[strlen(tar)+1]);
	else strcpy(chemin, &twd[strlen(tar)]);
	if (strlen(chemin) != 0) strcat(chemin, "/");
	strcat(chemin, rest);

	int fd = open(absolutetar, O_RDONLY);
	if (fd == -1) {
		char *error_debut = "cd : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		sprintf(error, "%s%s\n", error_debut, absolutetar);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	int found = 0;

	if (chemin != NULL && strlen(chemin) != 0) {
		struct posix_header header;
		while (!found) {
			if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

			char nom[strlen(header.name)+1];
			strcpy(nom, header.name);
			if (nom[strlen(nom)-1] == '/' && isSameDir(chemin, nom)) found = 1;
			else {
				if (strcmp(nom, chemin) == 0) found = -1;
				if (strcmp(nom, "") == 0) break;
				unsigned int taille;
				sscanf(header.size, "%o", &taille);
				taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
				taille *= BLOCKSIZE;
				if (lseek(fd, (off_t) taille, SEEK_CUR) == -1) break;
			}
		}
	}else found = 1;

	close(fd);
	if (!found) {
		char *deb  = "cd : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		sprintf(error, "%s%s/%s%s", deb, absolutetar, chemin, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}else if (found == 1) {
		char res[strlen(absolutetar) + 1 + strlen(chemin) + 1];
		strcpy(res, absolutetar);
		if (chemin != NULL && strlen(chemin) != 0) {
			strcat(res, "/");
			strcat(res, chemin);
		}
		if (res[strlen(res)-1] == '/') res[strlen(res)-1] = '\0';
		setWd(res);
	}else {
		char *deb  = "cd : ";
		char *end = " n'est pas un répertoire!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		sprintf(error, "%s%s/%s%s", deb, absolutetar, chemin, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	return (found == 1)? 0 : -1;
}

int isSameDir(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}

int errorDetect(int argc) {
	if(argc < 1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		return -1;
	}

	if(argc > 2) {
		errno = E2BIG;
		perror("Trop d'arguments !");
		return -1;
	}

	return 0;
}
