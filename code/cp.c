#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cp.h"
#include "tar.h"

void detectError(int);
int optionDetection(int, char *[]);
void copyto(char *, char *, int);
int exist(char *);
int existAbsolut(char *);
int existInTar(char *, char *);
int isSamedir(char *, char *);

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
					if (exist(source)) //source must exist
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

int exist(char *path) { //verify if path exist even inside tar
	if (path[0] == '/') {
		char *pos = strstr(path, ".tar");
		if (pos == NULL) return existAbsolut(path);
		else {
			int tarlen = strlen(path) - strlen(pos) + 4;
			char absolutetar[tarlen + 1];
			strncpy(absolutetar, path, tarlen);
			absolutetar[tarlen] = '\0';
			if (existAbsolut(absolutetar)) {
				char *rest = (strlen(absolutetar) == strlen(path))? "" : &path[strlen(absolutetar)+1];
				return existInTar(absolutetar, rest);
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
			if (existAbsolut(absolutetar)) {
				char *tmp = (strlen(twd) == strlen(tar))? "" : &twd[strlen(tar)+1];
				int tmplen = strlen(tmp);
				if (tmplen != 0) tmplen++;
				char rest[tmplen + strlen(path) + 1];
				strcpy(rest, tmp);
				if (tmplen != 0) strcat(rest, "/");
				strcat(rest, path);
				return existInTar(absolutetar, rest);
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
				if (existAbsolut(absolutetar)) {
					char *rest = (strlen(tar) == strlen(path))? "" : &path[strlen(tar)+1];
					return existInTar(absolutetar, rest);
				}
			}else {
				char absolutepath[strlen(pwd) + 1 + strlen(path) + 1];
				strcpy(absolutepath, pwd);
				strcat(absolutepath, "/");
				strcat(absolutepath, path);
				return existAbsolut(absolutepath);
			}
		}
	}
	return 0;
}

int existAbsolut(char *absolutepath) { // verify if absolutepath exist outside a tar
	struct stat st;
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
	return 1;
}

int existInTar(char *tar, char *chemin) {
	int fd = open(tar, O_RDONLY);
	if (fd == -1) {
		char *error_debut = "cp : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(tar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, tar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return 0;
	}

	int found = 0;

	if (chemin != NULL && strlen(chemin) != 0) {
		struct posix_header header;
		while (!found) {
			if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

			char nom[strlen(header.name)+1];
			strcpy(nom, header.name);
			if ((nom[strlen(nom)-1] == '/' && isSamedir(chemin, nom)) || (nom[strlen(nom)-1] != '/' && strcmp(nom, chemin) == 0))
				found = 1;
			else {
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
		char *deb  = "cp :";
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

int isSamedir(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}

void copyto(char *source, char *dest, int option) { // copy source to dest with or without option


	if (option) write(STDOUT_FILENO, "-r\n", 3);
	write(STDOUT_FILENO, source, strlen(source));
	write(STDOUT_FILENO, "\n", 1);
	write(STDOUT_FILENO, dest, strlen(dest));
	write(STDOUT_FILENO, "\n", 1);


	/*
	outisde tar
		sans -r
			file to file : copy contenu de 1 vers 2 en remplaçant le contenu existant en entier et ne change pas le nom
			file to doss : create file in the doss if not exist puis fait file to file
			doss to file : erreur
			doss to doss : erreur
		avec -r
			file to file : pareil que sans -r
			file to doss ; pareil que sans -r
			doss to file : erreur
			doss to doss : create a doss1 directory in doss2 with all the content if already exist ecrase ce qui a le meme nom avec le nouveau contenu
	*/

}
