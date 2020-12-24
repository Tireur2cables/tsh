#define _DEFAULT_SOURCE // utile pour strtok_r

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "cp.h"
#include "cat.h"
#include "tar.h"

void detectError(int, char *[]);
int optionDetection(int, char *[]);
int isCorrectDest(char *);
void copyto(char *, char *, int);
int exist(char *, int);
int existAbsolut(char *, int);
int existInTar(char *, char *, int);
int isSamedir(char *, char *);
int isInTar(char *);
int isTarDir(char *);
int isTar(char *);
void copyTar(char *, char *, int);
void copyDoss(char *, char *, int);
void copyFile(char *, char *, int);
void copystandard(char *, char *);
void copyfiletartofile(char *, char *, mode_t);
void copyfiletofiletar(char *, char *);
mode_t getmode(char *, char *);
int getHeader(struct posix_header *, char *, mode_t, uid_t, gid_t, off_t, struct timespec);
int ecritInTar(struct posix_header *, int, char *, unsigned int);

int cp(int argc, char *argv[]) {
// detect error in number of args
	detectError(argc, argv);

// get the index of the option if speciefied
	int indexOpt = optionDetection(argc, argv);

	int indexLastArg = (indexOpt == argc-1)? argc-2 : argc-1;
// dest is last arg
	char dest[strlen(argv[indexLastArg])+1];
	strcpy(dest, argv[indexLastArg]);

// first verify if dest is correct
	if (isCorrectDest(dest)) {
		// then for all sources copy to dest
			for (int i = 1; i < indexLastArg; i++) {
				if (i != indexOpt) { // skip option arg
					char source[strlen(argv[i])+1];
					strcpy(source, argv[i]);
					if (exist(source, 1)) //source must exist
						copyto(source, dest, (indexOpt != 0));
				}
			}
	}

	return 0;
}

void detectError(int argc, char *argv[]) { // vérifie qu'un nombre correct d'éléments est donné
	if (argc < 3) {
		char *error = "cp : Il faut donner au moins 2 chemins à cp!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
	if (argc == 3) {
		if (argv[1][0] == '-' || argv[2][0] == '-') {
			char *error = "cp : Il faut donner au moins 2 chemins à cp!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
}

int optionDetection(int argc, char *argv[]) { //return the position of the option '-r' if specified or return 0
	int indice = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0) {
			if (indice == 0) indice = i;
			else {
				char *error = "cp : Erreur trop d'options -r !\n";
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}else if (strncmp(argv[i], "-", 1) == 0) {
			char *deb = "cp : Erreur option inconnue : ";
			char error[strlen(deb) + strlen(argv[i]) + 1 + 1];
			strcpy(error, deb);
			strcat(error, argv[i]);
			strcat(error, "\n");
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	return indice;
}

int isCorrectDest(char *dest) { // dest must respect certain convention
	if (exist(dest, 0) || strstr(dest, "/") == NULL) return 1;
	char dest_copy[strlen(dest)+1];
	strcpy(dest_copy, dest);
	char dest_copy_copy[strlen(dest)+1];
	strcpy(dest_copy_copy, dest);
	char *tmp = dest_copy;
	char *saveptr;
	char *tok;
	int last = 0;
	while ((tok = strtok_r(tmp, "/", &saveptr)) != NULL) {
		tmp = saveptr;
		if (strlen(tok) != 0) last = strlen(tok);
	}
	int indice = strlen(dest_copy_copy) - last - 1;
	dest_copy_copy[indice] = '\0';
	if (exist(dest_copy_copy, 1)) return 1;
	return 0;
}

int exist(char *path, int error) { //verify if path exist even inside tar
	if (path[0] == '/') {
		char *pos = strstr(path, ".tar");
		if (pos == NULL) return existAbsolut(path, error);
		else {
			int tarlen = strlen(path) - strlen(pos) + 4;
			char absolutetar[tarlen + 1];
			strncpy(absolutetar, path, tarlen);
			absolutetar[tarlen] = '\0';
			if (existAbsolut(absolutetar, error)) {
				char *rest = (strlen(absolutetar) == strlen(path))? "" : &path[strlen(absolutetar)+1];
				return existInTar(absolutetar, rest, error);
			}
		}
	}else {
		char *twd = getenv("TWD");
		char *pwd = getcwd(NULL, 0);
		if (twd != NULL && strlen(twd) != 0) {
			char *pos = strstr(twd, ".tar");
			int tarlen = strlen(twd) - strlen(pos) + 4;
			char tar[tarlen + 1];
			strncpy(tar, twd, tarlen);
			tar[tarlen] = '\0';
			char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
			strcpy(absolutetar, pwd);
			strcat(absolutetar, "/");
			strcat(absolutetar, tar);
			if (existAbsolut(absolutetar, error)) {
				char *tmp = (strlen(twd) == strlen(tar))? "" : &twd[strlen(tar)+1];
				int tmplen = strlen(tmp);
				if (tmplen != 0) tmplen++;
				char rest[tmplen + strlen(path) + 1];
				strcpy(rest, tmp);
				if (tmplen != 0) strcat(rest, "/");
				strcat(rest, path);
				return existInTar(absolutetar, rest, error);
			}
		}else {
			char *pos = strstr(path, ".tar");
			if (pos != NULL) {
				int tarlen = strlen(path) - strlen(pos) + 4;
				char tar[tarlen + 1];
				strncpy(tar, path, tarlen);
				tar[tarlen] = '\0';
				char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
				strcpy(absolutetar, pwd);
				strcat(absolutetar, "/");
				strcat(absolutetar, tar);
				if (existAbsolut(absolutetar, error)) {
					char *rest = (strlen(tar) == strlen(path))? "" : &path[strlen(tar)+1];
					return existInTar(absolutetar, rest, error);
				}
			}else {
				char absolutepath[strlen(pwd) + 1 + strlen(path) + 1];
				strcpy(absolutepath, pwd);
				strcat(absolutepath, "/");
				strcat(absolutepath, path);
				return existAbsolut(absolutepath, error);
			}
		}
	}
	return 0;
}

int existAbsolut(char *absolutepath, int error) { // verify if absolutepath exist outside a tar
	struct stat st;
	if (stat(absolutepath, &st) < 0) {
		if (error) {
			char *error_fin = " n'existe pas!\n";
			char error[strlen(absolutepath)+strlen(error_fin)+1];
			strcpy(error, absolutepath);
			strcat(error, error_fin);
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
		}
		return 0;
	}
	return 1;
}

int existInTar(char *tar, char *chemin, int error) { // verifie que chemin existe dans tar ou est tar
	int fd = open(tar, O_RDONLY);
	if (fd == -1) {
		if (error) {
			char *error_debut = "cp : Erreur! Impossible d'ouvrir l'archive ";
			char error[strlen(error_debut) + strlen(tar) + 1 + 1];
			strcpy(error, error_debut);
			strcat(error, tar);
			strcat(error, "\n");
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
		}
		return 0;
	}

	int found = 0;

	if (chemin != NULL && strlen(chemin) != 0) {
		struct posix_header header;
		while (!found) {
			if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

			char nom[strlen(header.name)+1];
			strcpy(nom, header.name);
			if ((nom[strlen(nom)-1] == '/' && isSamedir(chemin, nom)) || strcmp(nom, chemin) == 0)
				found = 1;
			else {
				if (strcmp(nom, "") == 0) break;
				unsigned int taille = atoi(header.size);
				taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
				taille *= BLOCKSIZE;
				if (lseek(fd, (off_t) taille, SEEK_CUR) == -1) break;
			}
		}
	}else found = 1;

	close(fd);
	if (!found && error) {
		char *deb  = "cp : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(tar) + 1 + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, tar);
		strcat(error, "/");
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	return found;
}

int isSamedir(char *dir1, char *dir2) { // verifie si dir1 à le même nom que dir2 avec dir2 dans un tar
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}

void copyto(char *source, char *dest, int option) { // copy source to dest with or without option
	struct stat stsource;
	stat(source, &stsource); // verifications déjà faites

	if (isInTar(source)) // source est dans un tar ou est un tar
		copyTar(source, dest, option);

	else if (S_ISDIR(stsource.st_mode)) // source est un dossier
		copyDoss(source, dest, option);

	else // source est un fichier
		copyFile(source, dest, option);
}

int isInTar(char *path) {
	return (path[0] == '/' && strstr(path, ".tar") != NULL) || (path[0] != '/' && getenv("TWD") != NULL);
}

void copyTar(char *source, char *dest, int option) {
	struct stat stdest;
	stat(dest, &stdest); // verifications déjà faites

	if (isTarDir(source)) { // source est un dossier dans un tar ou un tar
		if (!option) { // -r pas spécifié
			char *error = "cp : l'option -r doit être spécifiée pour les dossiers!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
			return;
		}

		if (isInTar(dest)) { // dest est dans un tar
			// TODO
			if (!exist(dest, 0)) {
				// creer le dossier dest et mettre le contenu de source dedans
			}else {
				if (isTarDir(dest)) { // dest est un dossier dans un tar ou un tar
					// creer le dossier dest/sourcefile et mettre le contenu de source dedans
				}else { // dest est un fichier dasn le tar
					char *deb  = "cp : ";
					char *end = " n'est pas un dossier!\n";
					char error[strlen(deb) + strlen(dest) + strlen(end) + 1];
					strcpy(error, deb);
					strcat(error, dest);
					strcat(error, end);
					int errorlen = strlen(error);
					if (write(STDERR_FILENO, error, errorlen) < errorlen)
						perror("Erreur d'écriture dans le shell!");
					return;
				}
			}
		}

		else if (exist(dest, 0) && !S_ISDIR(stdest.st_mode)) { // dest n'est pas un dossier
			char *deb  = "cp : ";
			char *end = " n'est pas un dossier!\n";
			char error[strlen(deb) + strlen(dest) + strlen(end) + 1];
			strcpy(error, deb);
			strcat(error, dest);
			strcat(error, end);
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
			return;
		}

		else { // dest est un dossier
			struct stat stsource;
			stat(source, &stsource); // verifications déjà faites

			char *sourcedoss = source;
			char *iter = source;
			char *tok;
			while ((tok = strstr(iter, "/")) != NULL) {
				iter = tok+1;
				if (strlen(tok+1) != 0) sourcedoss = tok+1;
			}

			int destlen = strlen(dest);
			if (dest[destlen-1] != '/') destlen++;
			char tmpdest[destlen + strlen(sourcedoss) + 1];
			strcpy(tmpdest, dest);
			if (destlen != strlen(dest)) strcat(tmpdest, "/");
			strcat(tmpdest, sourcedoss);

			if (!exist(dest, 0)) {
				if (mkdir(dest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH /*stsource.st_mode ?*/) < 0) {
					perror("Impossible de créer le dossier!");
					exit(EXIT_FAILURE);
				}
			}else {
				if (isTar(source)) { // source est un tar et doit etre copié directement en tant que tel
					copystandard(source, dest);
					return;
				}else {
					if (!exist(tmpdest, 0)) {
						if (mkdir(tmpdest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH /*stsource.st_mode ?*/) < 0) {
							perror("Impossible de créer le dossier!");
							exit(EXIT_FAILURE);
						}
					}
					dest = tmpdest; // dest deveint dest/sourcedoss
				}
			}

			int pwdlen = 0;
			int twdlen = 0;
			char *pwd = getcwd(NULL, 0);
			char *twd = getenv("TWD");
			if (source[0] != '/') pwdlen = strlen(pwd) + 1;
			if (source[0] != '/' && twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;
			char absolutesource[pwdlen + twdlen + strlen(source) + 1];
			strcpy(absolutesource, "");
			if (source[0] != '/') {
				strcat(absolutesource, pwd);
				strcat(absolutesource, "/");
				if (twdlen != 0) {
					strcat(absolutesource, twd);
					strcat(absolutesource, "/");
				}
			}
			strcat(absolutesource, source);

			tok = strstr(absolutesource, ".tar"); //not null
			int tarlen = strlen(absolutesource) - strlen(tok) + 4;
			char tar[tarlen + 1];
			strncpy(tar, absolutesource, tarlen);
			tar[tarlen] = '\0';

			char chemin[strlen(absolutesource) - strlen(tar) + 1];
			strcpy(chemin, &absolutesource[strlen(tar)+1]); // source n'est pas juste un tar

			int fdsource = open(tar, O_RDONLY);
			if (fdsource == -1) {
				char *error_debut = "cp : Erreur! Impossible d'ouvrir l'archive ";
				char error[strlen(error_debut) + strlen(tar) + 1 + 1];
				strcpy(error, error_debut);
				strcat(error, tar);
				strcat(error, "\n");
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
			struct posix_header header;
			while (1) {
				if (read(fdsource, &header, BLOCKSIZE) < BLOCKSIZE) break;

				char nom[strlen(header.name)+1];
				strcpy(nom, header.name);
				if (strcmp(nom, "") == 0) break;

				if (strstr(nom, chemin) != NULL && strcmp(chemin, nom) <= 0) { // nom must start with chemin
					mode_t mode;
					sscanf(header.mode, "%o", &mode);

					char *newnom = strstr(nom, chemin) + strlen(chemin);
					char *tmp = newnom;
					char *pos;
					while ((pos = strstr(tmp, "/")) != NULL) { // parcours (et créer si besoin) les dossiers nécéssaires pour copier le fichier
						char dossier[strlen(dest) + 1 + strlen(newnom) - strlen(pos) + 1];
						strcpy(dossier, dest);
						strcat(dossier, "/");
						strncat(dossier, newnom, strlen(newnom) - strlen(pos));
						if (!exist(dossier, 0) && header.typeflag == '5') {
							// on suppose raisonnablement que les dossiers arrivent avant les fichier de ces dossiers dans le parcours du tar
							if (mkdir(dossier, mode) < 0) {
								perror("Impossible de créer le dossier!");
								exit(EXIT_FAILURE);
							}
						}
						tmp = pos+1;
					}
					if (header.typeflag != '5') { // n'est pas un dossier
						char newDest[strlen(dest) + 1 + strlen(newnom) + 1];
						strcpy(newDest, dest);
						strcat(newDest, "/");
						strcat(newDest, newnom);

						char newSource[strlen(source) + 1 + strlen(newnom) + 1];
						strcpy(newSource, source);
						strcat(newSource, "/");
						strcat(newSource, newnom);
						copyfiletartofile(newSource, newDest, mode);
					}
				}

				unsigned int taille = atoi(header.size);
				taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
				taille *= BLOCKSIZE;
				if (lseek(fdsource, (off_t) taille, SEEK_CUR) == -1) break;
			}
			close(fdsource);
		}

	}

	else { // source est un fichier dans un tar
		struct stat stsource;
		stat(source, &stsource); // verifications déjà faites

		if (isInTar(dest)) { // dest est dans un tar ou est un tar
			// TODO
			if (!exist(dest, 0)) {
				// creer le fichier dest et mettre le contenu de source dedans
			}else {
				if (isTarDir(dest)) {
					// creer le fichier dest/sourcefile et mettre le contenu de source dedans
				}else {
					// remplacer le contenu de dest par celui de source
				}
			}
		}

		else if (!exist(dest, 0) || !S_ISDIR(stdest.st_mode)) // dest est un fichier
			copyfiletartofile(source, dest, stsource.st_mode);

		else { // dest un dossier
			if (exist(dest, 1)) {
				char source_copy[strlen(source)+1];
				strcpy(source_copy, source);
				char *sourcefile;
				char *tok;
				char *saveptr;
				char *tmp = source_copy;
				while ((tok = strtok_r(tmp, "/", &saveptr)) != NULL) {
					tmp = saveptr;
					sourcefile = tok;
				}

				int destlen = strlen(dest);
				if (dest[destlen-1] != '/') destlen++;
				char newDest[destlen + strlen(sourcefile) + 1];
				strcpy(newDest, dest);
				if (strlen(dest) != destlen) strcat(newDest, "/");
				strcat(newDest, sourcefile);
				copyfiletartofile(source, newDest, stsource.st_mode);
			}
		}
	}
}

int isTar(char *path) { // verifie si path correspond au chemin d'un .tar
	char *pos = strstr(path, ".tar");
	if (pos == NULL) return 0;
	return (strlen(pos) == 4 || strlen(pos) == 5);
}

int isTarDir(char *path) { // verifie que path est un chemin de dossier dans un tar ou un tar
	if (isTar(path)) return 1;
	if (path[0] == '/') {
		char *pos = strstr(path, ".tar");
		int tarlen = strlen(path) - strlen(pos) + 4;
		char absolutetar[tarlen + 1];
		strncpy(absolutetar, path, tarlen);
		absolutetar[tarlen] = '\0';

		int poslen = strlen(pos) - 5;
		if (pos[strlen(pos)-1] != '/') poslen++;
		char newpath[poslen + 1]; // exist because path is not just a tar
		strcpy(newpath, &pos[5]);
		if (pos[strlen(pos)-1] != '/') strcat(newpath, "/");

		return existInTar(absolutetar, newpath, 0);
	}
	else {
		char *pwd = getcwd(NULL, 0);
		char *twd = getenv("TWD");
		if (twd != NULL && strlen(twd) != 0) {
			char twd_copy[strlen(twd)+1];
			strcpy(twd_copy, twd);
			char *tar = strtok(twd_copy, "/");
			char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
			strcpy(absolutetar, pwd);
			strcat(absolutetar, "/");
			strcat(absolutetar, tar);

			int pathlen = strlen(path);
			if (path[pathlen-1] != '/') pathlen++;
			if (strlen(tar) == strlen(twd)) {
				char newpath[pathlen+1];
				strcpy(newpath, path);
				if (pathlen != strlen(path)) strcat(newpath, "/");
				return existInTar(absolutetar, newpath, 0);
			}else {
				char newpath[strlen(twd) - strlen(tar) + pathlen + 1];
				strcpy(newpath, &twd[strlen(tar)+1]);
				strcat(newpath, "/");
				strcat(newpath, path);
				if (pathlen != strlen(path)) strcat(newpath, "/");
				return existInTar(absolutetar, newpath, 0);
			}
		}else {
			char *pos = strstr(path, ".tar");
			int tarlen = strlen(path) - strlen(pos) + 4;
			char tar[tarlen + 1];
			strncpy(tar, path, tarlen);
			tar[tarlen] = '\0';
			char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
			strcpy(absolutetar, pwd);
			strcat(absolutetar, "/");
			strcat(absolutetar, tar);

			int poslen = strlen(pos) - 5;
			if (pos[strlen(pos)-1] != '/') poslen++;
			char newpath[poslen + 1]; // exist because path is not just a tar
			strcpy(newpath, &pos[5]);
			if (pos[strlen(pos)-1] != '/') strcat(newpath, "/");

			return existInTar(absolutetar, newpath, 0);
		}
	}
	return 1;
}

void copyDoss(char *source, char *dest, int option) {
	struct stat stdest;
	stat(dest, &stdest); // verifications déjà faites

	if (!option) { // -r pas spécifié
		char *error = "cp : l'option -r doit être spécifiée pour les dossiers!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return;
	}

	if (isInTar(dest)) { // dest est dans un tar
		// TODO
		if (!exist(dest, 0)) {
			// creer le dossier dest et mettre le contenu de source dedans
		}else {
			if (isTarDir(dest)) { // dest un dossier dans le tar ou un tar
				// creer le dossier dest/source et mettre le contenu de source dedans
			}else { // dest est un fichier dans le tar
				char *deb  = "cp : ";
				char *end = " n'est pas un dossier!\n";
				char error[strlen(deb) + strlen(dest) + strlen(end) + 1];
				strcpy(error, deb);
				strcat(error, dest);
				strcat(error, end);
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
		}
	}

	else if (exist(dest, 0) && !S_ISDIR(stdest.st_mode)) { // dest n'est pas un dossier
		char *deb  = "cp : ";
		char *end = " n'est pas un dossier!\n";
		char error[strlen(deb) + strlen(dest) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, dest);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return;
	}

	else // dest est un dossier
		copystandard(source, dest);
}

void copyFile(char *source, char *dest, int option) {
	struct stat stdest;
	stat(dest, &stdest); // verifications déjà faites
	struct stat stsource;
	stat(source, &stsource); // verifications déjà faites

	if (isInTar(dest)) { // dest est dans un tar
		if (isTarDir(dest)) { // dest est un dossier dans un tar ou un tar
			char source_copy[strlen(source)+1];
			strcpy(source_copy, source);
			char *sourcefile;
			char *tok;
			char *saveptr;
			char *tmp = source_copy;
			while ((tok = strtok_r(tmp, "/", &saveptr)) != NULL) {
				tmp = saveptr;
				sourcefile = tok;
			}
			char newdest[strlen(dest) + 1 + strlen(sourcefile) + 1];
			strcpy(newdest, dest);
			strcat(newdest, "/");
			strcat(newdest, sourcefile);
			copyfiletofiletar(source, newdest);
		}else // dest est un fichier dans un tar
			copyfiletofiletar(source, dest);
	}

	else if (!exist(dest, 0) || !S_ISDIR(stdest.st_mode)) // dest est un fichier
		copystandard(source, dest);

	else { // dest un dossier
		if (exist(dest, 1)) {
			char source_copy[strlen(source)+1];
			strcpy(source_copy, source);
			char *sourcefile;
			char *tok;
			char *saveptr;
			char *tmp = source_copy;
			while ((tok = strtok_r(tmp, "/", &saveptr)) != NULL) {
				tmp = saveptr;
				sourcefile = tok;
			}

			int destlen = strlen(dest);
			if (dest[destlen-1] != '/') destlen++;
			char newDest[destlen + strlen(sourcefile) + 1];
			strcpy(newDest, dest);
			if (strlen(dest) != destlen) strcat(newDest, "/");
			strcat(newDest, sourcefile);
			copystandard(source, newDest);
		}
	}
}

void copystandard(char *source, char *dest) { // cp -r standard
	int pid = fork();
	switch(pid) {
		case -1: {//erreur
			perror("Erreur de fork!");
			return;
		}
		case 0: {//fils
			execlp("cp", "cp", "-r", source, dest, NULL);
			exit(EXIT_SUCCESS);
		}
		default: {//pere
			if (waitpid(pid, NULL, 0) < 0) {
				perror("Erreur de wait!");
				return;
			}
		}
	}
}

void copyfiletartofile(char *source, char *dest, mode_t mode) { // ecrase contenu de dest avec contenu de source et ne change pas le nom de dest
	int pid = fork();
	switch(pid) {
		case -1: {//erreur
			perror("Erreur de fork!");
			return;
		}
		case 0: {//fils
			int fddest;
			if ((fddest = open(dest, O_WRONLY + O_TRUNC + O_CREAT, mode)) == -1) {
				char *deb  = "cp : Impossible d'ouvrir ";
				char error[strlen(deb) + strlen(dest) + 1 + 1];
				strcpy(error, deb);
				strcat(error, dest);
				strcat(error, "\n");
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return;
			}
			if (dup2(fddest, STDOUT_FILENO) < 0) {
				perror("Erreur lors de la redirection vers la destination!");
				return;
			}
			char *argv[3] = {"cat", source, NULL};
			cat(2, argv);
			close(fddest);
			exit(EXIT_SUCCESS);
		}
		default: {//pere
			if (waitpid(pid, NULL, 0) < 0) {
				perror("Erreur de wait!");
				return;
			}
		}
	}
}

void copyfiletofiletar(char *source, char *dest) { // ecrase contenu de dest avec contenu de source et ne change pas le nom de dest
	int pwdlen = 0;
	int twdlen = 0;
	char *pwd = getcwd(NULL, 0);
	char *twd = getenv("TWD");
	if (source[0] != '/') pwdlen = strlen(pwd) + 1;
	if (source[0] != '/' && twd != NULL && strlen(twd) != 0) twdlen = strlen(twd) + 1;

	char absolutedest[pwdlen + twdlen + strlen(dest) + 1];
	strcpy(absolutedest, "");
	if (dest[0] != '/') {
		strcat(absolutedest, pwd);
		strcat(absolutedest, "/");
		if (twdlen != 0) {
			strcat(absolutedest, twd);
			strcat(absolutedest, "/");
		}
	}
	strcat(absolutedest, dest);

	char *pos = strstr(absolutedest, ".tar");
	int tarlen = strlen(absolutedest) - strlen(pos) + 4;
	char tar[tarlen+1];
	strncpy(tar, absolutedest, tarlen);
	tar[tarlen] = '\0';

	char chemin[strlen(absolutedest) - strlen(tar) - 1 + 1];
	strcpy(chemin, &absolutedest[strlen(tar)+1]); // existe car dest n'est pas un tar

	char *cheminfile;
	char *tmp = chemin;
	while ((pos = strstr(tmp, "/")) != NULL) {
		tmp = pos+1;
		if (strlen(pos+1) != 0) cheminfile = pos+1;
	}

	int chemindosslen = 0;
	if (strlen(chemin) > strlen(cheminfile)) chemindosslen = strlen(chemin) - strlen(cheminfile) - 1;
	char chemindoss[chemindosslen + 1];
	if (chemindosslen != 0) {
		strncpy(chemindoss, chemin, chemindosslen);
		chemindoss[chemindosslen] = '\0';
	}

	struct stat stsource;
	stat(source, &stsource); // verifications déjà faites

	int fd = open(tar, O_RDWR);
	if (fd == -1) {
		char *deb  = "cp : Impossible d'ouvrir ";
		char error[strlen(deb) + strlen(tar) + 1 + 1];
		strcpy(error, deb);
		strcat(error, tar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return;
	}

	int existdest = exist(dest, 0);

	struct posix_header header;
	while (1) {
		if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;
		char nom[strlen(header.name)+1];
		strcpy(nom, header.name);

		if (existdest) {
			if (strcmp(nom, chemin) == 0) {
				char *deb = "Remplace le fichier ";
				char buf[strlen(deb) + strlen(nom) + 1];
				strcpy(buf, deb);
				strcat(buf, nom);
				write(STDOUT_FILENO, buf, strlen(buf));
				write(STDOUT_FILENO, "\n", 1);
				break;
			}
		}else {
			if (chemindoss != NULL && chemindosslen != 0) { // chemin est dans un dossier
				if (strstr(nom, chemindoss) != NULL && strcmp(chemindoss, nom) <= 0) { // nom commence par chemindoss
					char *deb = "Insere le fichier dans le dossier ";
					char buf[strlen(deb) + strlen(nom) + 1];
					strcpy(buf, deb);
					strcat(buf, nom);
					write(STDOUT_FILENO, buf, strlen(buf));
					write(STDOUT_FILENO, "\n", 1);
					break;
				}
			}

			if (strcmp(nom, "") == 0) { // block vide = fin du tar
				char *buf = "Met le fichier à la fin!";
				write(STDOUT_FILENO, buf, strlen(buf));
				write(STDOUT_FILENO, "\n", 1);
				if (getHeader(&header, chemin, stsource.st_mode, stsource.st_uid, stsource.st_gid, stsource.st_size, stsource.st_mtim) < 0)
					return;

				unsigned int taille = stsource.st_size;
				unsigned int taille_finale = BLOCKSIZE * ((taille + BLOCKSIZE - 1) >> BLOCKBITS);
				char contenu[taille_finale];

				int fd_tube[2];
				if (pipe(fd_tube) == -1) {
			    	perror("Erreur de création du tube!");
			    	return;
				}
				int pid = fork();
				switch(pid) {
			  		case -1: { // erreur
						perror("erreur de fork!");
				   		return;
					}
					case 0: { // fils
						close(fd_tube[0]);
					    dup2(fd_tube[1], STDOUT_FILENO);

						char *argv[3] = {"cat", source, NULL};
						cat(2, argv);

					    close(fd_tube[1]);
						exit(EXIT_SUCCESS);
					}
					default: { //pere
						close(fd_tube[1]);
						if (waitpid(pid, NULL, 0) < 0) {
							perror("Erreur de wait!");
							exit(EXIT_FAILURE);
						}
						char tmp[500];
						int readen;
						int indice = 0;
						while ((readen = read(fd_tube[0], tmp, 500)) != 0) {
							for (int i = 0; i < readen; i++) contenu[indice+i] = tmp[i];
							indice += readen;
						}
						close(fd_tube[0]);
					}
				}

				for (int i = taille; i < taille_finale; i++) contenu[i] = '\0';

				if (ecritInTar(&header, fd, contenu, taille_finale) < 0) return;

				break;
			}
		}

		unsigned int taille = atoi(header.size);
		taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
		taille *= BLOCKSIZE;
		off_t position;
		if ((position = lseek(fd, (off_t) taille, SEEK_CUR)) == -1) {
			perror("Erreur de déplacement dans le tar!");
			break;
		}

	}
	close(fd);
}

int getHeader(struct posix_header *header, char *chemin, mode_t mode, uid_t uid, gid_t gid, off_t taille, struct timespec mtime) {
	int decalage = 0;
	for (int i = 0; i < 155; i++) {
		if (strlen(chemin) > 99 && i < strlen(chemin) - 99) {
			header->prefix[i] = chemin[i];
			decalage++;
		}
		else header->prefix[i] = '\0';
	}
	for (int i = 0; i < 100; i++) {
		if (i+decalage < strlen(chemin)) header->name[i] = chemin[i+decalage];
		else header->name[i] = '\0';
	}

	sprintf(header->mode, "%o", mode);
	sprintf(header->uid, "%07d", uid);
	sprintf(header->gid, "%07d", gid);
	sprintf(header->size, "%011ld", taille);
	sprintf(header->mtime, "%11ld", mtime.tv_sec*1000);

	for (int i = 0; i < 8; i++) header->chksum[i] = '\0'; // on le set à la fin
	header->typeflag = '0'; // fichier
	for (int i = 0; i < 100; i++) header->linkname[i] = '\0'; //pas un lien
	sprintf(header->magic, "ustar"); // version courante

	header->version[0] = '0';
	header->version[1] = '0';

	// useless ?
	for (int i = 0; i < 32; i++) header->uname[i] = '\0';
	for (int i = 0; i < 32; i++) header->gname[i] = '\0';
	for (int i = 0; i < 8; i++) header->devmajor[i] = '\0';
	for (int i = 0; i < 8; i++) header->devminor[i] = '\0';
	for (int i = 0; i < 12; i++) header->junk[i] = '\0';

	set_checksum(header);
	if (!check_checksum(header)) {
		char *error = "cp : Erreur le header n'est pas conforme!\n";
		if (write(STDERR_FILENO, error, strlen(error)) < strlen(error))
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	return 1;
}

int ecritInTar(struct posix_header *header, int fd, char *contenu, unsigned int taille) {
	off_t zone;
	if ((zone = lseek(fd, -BLOCKSIZE, SEEK_CUR)) == -1) { // on se remet à l'emplacement du header
		perror("Erreur de déplacement dans le tar!");
		return -1;
	}

	unsigned int taillefin = lseek(fd, 0, SEEK_END) - zone; // taille de ce qu'il reste jusqua la fin du tar
	char buf[taillefin];
	printf("%ld\n", zone2);

	if (lseek(fd, zone, SEEK_SET) == -1) { // on se remet à l'emplacement du header
		perror("Erreur de déplacement dans le tar!");
		return -1;
	}

	if (read(fd, buf, taillefin) < taillefin) {
		perror("Erreur de lecture de la fin du tar!");
		return -1;
	}

	if (lseek(fd, zone, SEEK_SET) == -1) { // on se remet à l'emplacement du header
		perror("Erreur de déplacement dans le tar!");
		return -1;
	}

	if (write(fd, header, BLOCKSIZE) < BLOCKSIZE) { // on écrit le header
		char *error  = "cp : Impossible d'écrire le header!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	if (write(fd, contenu, taille) < taille) {
		char *error  = "cp : Impossible d'écrire le contenu du fichier!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	if (write(fd, buf, taillefin) < taillefin) {
		char *error  = "cp : Impossible de réécrire la fin du tar!\n";
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	return 1;
}
