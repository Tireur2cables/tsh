#define _DEFAULT_SOURCE // utile pour strtok_r

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#include "mkdir.h"
#include "tar.h"

int exist_dir(char *, char *);
int exist_file(char *);
int exist_pere(char *);
int exist_pere_in_tar(char *, char *);
int create_dir(char *);
int create_tar(char *);
int try_create_dir(char *);
int contains_tar_mk(char *);
int create_header_mkdir(char *, struct posix_header *);
int is_tar_mk(char *);
int write_block_mkdir(int fd, struct posix_header *);
int is_ext_mk(char *, char *);
void get_header_size_mk(struct posix_header *, int *);

int mkdir_tar(int argc, char *argv[]) {
	if(argc == 0) {
		errno = EINVAL;
		perror("erreur du programme");
		exit(EXIT_FAILURE);
	}
	else if (argc == 1) {
		char *format = "rmdir: opérande manquant\n";
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}
	else {
		char *twd = getenv("TWD");
		for (int i = 1; i < argc; i++) {
			if (twd != NULL) {
				if (contains_tar_mk(twd)) {
					if(argv[i][0] == '/') // Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
						try_create_dir(argv[i]);
					else {
						char file[strlen(twd) + 1 + strlen(argv[i]) + 1];
						sprintf(file, "%s/%s", twd, argv[i]);
						try_create_dir(file);
					}
				}else {
					char file[strlen(twd) + 1 + strlen(argv[i]) + 1];
					sprintf(file, "%s/%s", twd, argv[i]);
					try_create_dir(file);
				}
			}
			else
				try_create_dir(argv[i]);
		}
	}
	return 0;
}

int try_create_dir(char *name) {
	if(is_tar_mk(name)) // On veut fabriquer un tar
		create_tar(name);
	else if(contains_tar_mk(name)) // On veut ajouter un dossier dans un tar
		create_dir(name);
	else // On se trouve dans un tar, mais on fait mkdir sur un chemin qui ne contient pas de tar
		execlp("mkdir", "mkdir", name, NULL);

	return 0;
}

int create_tar(char *name) { // Créer un tar
	if (exist_file(name)) {
		char format[60 + strlen(name)];
		sprintf(format, "mkdir: impossible de créer le repertoire « %s »: le fichier existe\n", name);
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		return -1;
	}
	if (!exist_pere(name)) {
		char *deb = "mkdir: Erreur le dossier parent de ";
		char *fin = " n'existe pas!\n";
		char format[strlen(deb)+strlen(fin)+1+strlen(name)];
		sprintf(format, "%s%s%s", deb, name, fin);
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		return -1;
	}
	int fd = open(name, O_WRONLY + O_CREAT, S_IRUSR + S_IWUSR + S_IRGRP + S_IROTH);
	if (fd == -1) {
		perror("Impossible d'ouvrir le fichier!");
		return -1;
	}
	// ecriture d'un block de BLOCKSIZE \0 pour indiquer la fin de l'archive
	write_block_mkdir(fd, NULL);
	close(fd);
	return 1;

}

int create_dir(char *name) { // Créer un dossier dans un tar si possible
	int tarpos = strstr(name, ".tar") - name; // Existe car on sait qu'il y a un tar dans le chemin
	char tarfile[tarpos+4+1]; // Contient le chemin jusqu'au tar pour l'ouvrir
	strncpy(tarfile, name, tarpos+4);
	tarfile[tarpos+4] = '\0';
	char namefile[strlen(name)-tarpos-4+1]; // Contient le nom du dossier a créer
	strncpy(namefile, name+tarpos+5, strlen(name)-tarpos-4);
	namefile[strlen(name)-tarpos-4] = '\0';
	if (namefile[strlen(namefile)-1] == '/') {
		namefile[strlen(namefile)-1] = '\0';
	}

	if (name[tarpos+4] != '/') { // Si après le .tar il n'y a pas de / => Il y a une erreur dans le nom du fichier
		char format[72 + strlen(name)];
		sprintf(format, "mkdir : impossible d'accéder à '%s' : Aucun fichier ou dossier de ce type\n", name);
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		return -1;
	}

	if (exist_dir(namefile, tarfile)) {
		char format[60 + strlen(name)];
		sprintf(format, "mkdir: impossible de créer le répertoire « %s »: Le fichier existe\n", name);
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}else if (!exist_pere_in_tar(namefile, tarfile)) {
		char *deb = "mkdir: Erreur le dossier parent de ";
		char *fin = " n'existe pas!\n";
		char format[strlen(deb)+strlen(fin)+1+strlen(namefile)];
		sprintf(format, "%s%s%s", deb, namefile, fin);
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}else {
		struct posix_header header;
		int fd = open(tarfile, O_RDWR);
		if (fd == -1) {
		  perror("erreur d'ouverture de l'archive");
		  return -1;
		}
		int n = 0;
		int read_size = 0;
		while ((n=read(fd, &header, BLOCKSIZE)) > 0) {
			if (strcmp(header.name, "\0") == 0) {
				// On est a la fin du tar, on écrit un nouveau header de dossier, puis un nouveau block de 512 vide
				struct posix_header header2;
				if (lseek(fd, -BLOCKSIZE, SEEK_CUR) == -1) { // On remonte d'un block pour écrire au bon endroit
					perror("Impossible de se déplacer dans le tar!");
					return -1;
				}
				create_header_mkdir(namefile, &header2);
				if (write(fd, &header2, BLOCKSIZE) < BLOCKSIZE) {
					perror("Impossible d'écrire le header dans le tar!");
					return -1;
				}
				write_block_mkdir(fd, NULL);
				write_block_mkdir(fd, NULL);
				break;
			}else {
				get_header_size_mk(&header, &read_size);
				if (lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1) {
					perror("erreur de lecture de l'archive");
					return -1;
				}
			}

		}
		close(fd);
	}
	return 0;
}

int exist_file(char *name) { // file exist ouside tar
	struct stat buffer;
	return (stat (name, &buffer) == 0);
}

int exist_dir(char *namefile, char *tarfile) { // exist namefile in tarfile
	struct posix_header header;
	int fd = open(tarfile, O_RDONLY);
	if (fd == -1) {
	  perror("erreur d'ouverture de l'archive");
	  return 0;
	}
	int n = 0;
	int read_size = 0;
	while ((n=read(fd, &header, BLOCKSIZE)) > 0) {
		if (strcmp(header.name, "\0") == 0) break;

		if(strcmp(namefile, header.name) == 0) return 1;

		get_header_size_mk(&header, &read_size);
		if (lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1) {
			perror("erreur de lecture de l'archive");
			return 0;
		}
	}
	close(fd);
	return 0;
}

int create_header_mkdir(char *name, struct posix_header *header) { // Meilleur méthode pour le faire ?
	int decalage = 0;
	for (int i = 0; i < 155; i++) {
		if (strlen(name) > 98 && i < strlen(name) - 98) {
			header->prefix[i] = name[i];
			decalage++;
		}
		else header->prefix[i] = '\0';
	}
	for (int i = 0; i < 100; i++) {
		if (i+decalage < strlen(name)) header->name[i] = name[i+decalage];
		else if (i+decalage == strlen(name)) header->name[i] = '/';
		else header->name[i] = '\0';
	}

	sprintf(header->mode,"0000755");
	char uid[8];
	char gid[8];
	int namelen = 32;
	char uname[namelen];
	char gname[namelen];
	struct passwd *p = getpwuid(getuid());
	if (p == NULL) {
		for (int i = 0; i < 8; i++) {
			uid[i] = '\0';
			gid[i] = '\0';
		}
		for (int i = 0; i < namelen; i++) uname[i] = '\0';
		for (int i = 0; i < namelen; i++) gname[i] = '\0';
	}else {
		sprintf(uid, "%07o", p->pw_uid);
		sprintf(gid, "%07o", p->pw_gid);
		sprintf(uname, "%s", p->pw_name);
		struct group *g = getgrgid(p->pw_gid);
		if (g == NULL)
			for (int i = 0; i < namelen; i++) gname[i] = '\0';
		else
			sprintf(gname, "%s", g->gr_name);

	}
	sprintf(header->uid, "%s", uid);
	sprintf(header->gid, "%s", gid);
	sprintf(header->uname, "%s", uname);
	sprintf(header->gname, "%s", gname);
	for (int i = strlen(uname); i < namelen; i++) header->uname[i] = '\0';
	for (int i = strlen(gname); i < namelen; i++) header->gname[i] = '\0';
	unsigned int taille = 0;
	sprintf(header->size, "%011o", taille);

	time_t mtime = time(NULL);
	sprintf(header->mtime, "%11lo", mtime);
	for (int i = 0; i < 8; i++) header->chksum[i] = '\0';
	header->typeflag = '5';
	for (int i = 0; i < 100; i++) header->linkname[i] = '\0';
	header->magic[0] = 'u';
	header->magic[1] = 's';
	header->magic[2] = 't';
	header->magic[3] = 'a';
	header->magic[4] = 'r';
	header->magic[5] = '\0';

	header->version[0] = '0';
	header->version[1] = '0';
	for (int i = 0; i < 8; i++) header->devmajor[i] = '\0';
	for (int i = 0; i < 8; i++) header->devminor[i] = '\0';
	for (int i = 0; i < 12; i++) header->junk[i] = '\0';

	set_checksum(header);
	if (!check_checksum(header)) {
		char *format = "mkdir : Erreur le header n'est pas conforme!\n";
		int formatlen = strlen(format);
		if (write(STDERR_FILENO, format, formatlen) < formatlen)
			perror("Erreur d'écriture dans le shell");
		return -1;
	}
	return 0;
}

int is_ext_mk(char *file, char *ext) {
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_mk(char *file) {
	return (is_ext_mk(file, ".tar") || is_ext_mk(file, ".tar/"));
}

int exist_pere(char *name) { // dossier parent exist outside tar
	char *pwd = getcwd(NULL, 0);
	int pwdlen = 0;
	if (name[0] != '/') pwdlen = strlen(pwd) + 1;
	char absolutename[pwdlen + strlen(name) + 1];
	strcpy(absolutename, "");
	if (pwdlen != 0) {
		strcat(absolutename, pwd);
		strcat(absolutename, "/");
	}
	strcat(absolutename, name);
	if (absolutename[strlen(absolutename)-1] == '/') absolutename[strlen(absolutename)-1] = '\0';
	char *pos = strrchr(absolutename, '/');
	if (pos == NULL) return 1; // seulement possible à la racine /
	int spos = pos - absolutename;
	char pere[strlen(absolutename)+1];
	strcpy(pere, absolutename);

	pere[spos] = '\0';
	// namedoss null seulement si name null

	return exist_file(pere);
}

int exist_pere_in_tar(char *namefile, char *tarfile) { // dossier parent exist inside tar
	char absolutename[strlen(namefile)+1];
	strcpy(absolutename, namefile);
	if (absolutename[strlen(absolutename)-1] == '/') absolutename[strlen(absolutename)-1] = '\0';
	char *pos = strrchr(absolutename, '/');
	if (pos == NULL) return exist_file(tarfile); // le pere est le tar
	int spos = pos - absolutename;
	char pere[strlen(absolutename)+1];
	strcpy(pere, absolutename);
	pere[spos+1] = '\0';

	return exist_dir(pere, tarfile);
}


int write_block_mkdir(int fd, struct posix_header *header) {
	if (header == NULL) {
		char block[BLOCKSIZE];
		memset(block, '\0', BLOCKSIZE);
		if (write(fd, block, BLOCKSIZE) < BLOCKSIZE) {
			perror("Erreur d'écriture dans l'archive");
			exit(EXIT_FAILURE);
		}
	}else {
		if(write(fd, header, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'aarchive");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

int contains_tar_mk(char *file) {
	return (strstr(file,".tar") != NULL);
}

void get_header_size_mk(struct posix_header *header, int *read_size) {
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + BLOCKSIZE - 1) / BLOCKSIZE);
}
