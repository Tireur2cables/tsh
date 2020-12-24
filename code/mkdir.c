#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "mkdir.h"
#include <stdlib.h>
#include "tar.h"

int exist_dir(char *, char *);
int exist_file(char *);
int create_dir(char *);
int create_tar(char *);
int try_create_dir(char *);
int contains_tar_mk(char *);
int create_header(char *, struct posix_header *);
int is_tar_mk(char *);
int write_block(int fd, struct posix_header *);
int is_ext_mk(char *, char *);
void get_header_size_mk(struct posix_header *, int *);

int mkdir_tar(int argc, char *argv[]) {
	if(argc == 0){
		errno = EINVAL;
		perror("erreur du programme");
		exit(EXIT_FAILURE);
	}
	else if(argc == 1){
		char *format = "rmdir: opérande manquant\n";
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}
	else{
		for(int i = 1; i < argc; i++){
			if(getenv("TWD") != NULL){
				if(is_tar_mk(getenv("TWD"))){
					if(argv[i][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
						try_create_dir(argv[i]);
					}else{
<<<<<<< HEAD
<<<<<<< HEAD
						char file[strlen(getenv("TWD")) + strlen(argv[i])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
=======
						char file[strlen(getenv("TWD")) + strlen(argv[1])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[1]);
						write(STDOUT_FILENO, file, strlen(file));
>>>>>>> avancement mkdiré
=======
						char file[strlen(getenv("TWD")) + strlen(argv[i])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
>>>>>>> Mkdir écrit bien le nouveau dossier dans le tar, mais créer un tar invalide pour le moment
						try_create_dir(file);
					}
				}else{
					char file[strlen(getenv("TWD")) + strlen(argv[i])];
					sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
					try_create_dir(file);
				}
			}
			else{
				try_create_dir(argv[i]);
			}
		}
	}
	return 0;
}

int try_create_dir(char *name){
	//write(STDOUT_FILENO, name, strlen(name));
	if(is_tar_mk(name)){ //On veut fabriquer un tar
		create_tar(name);
	}else if(contains_tar_mk(name)){ //On veut ajouter un dossier dans un tar
		create_dir(name);
	}else{//On se trouve dans un tar, mais on fait mkdir sur un chemin qui ne contient pas de tar
		execlp("mkdir", "mkdir", name, NULL);
	}
	return 0;
}

int create_tar(char *name){ //Créer un tar
	if(exist_file(name)){
		char format[60 + strlen(name)];
		sprintf(format, "mkdir: impossible de créer le repertoire « %s »: le fichier existe\n", name);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}
	int fd = open(name, O_WRONLY + O_CREAT, S_IRUSR + S_IWUSR + S_IRGRP + S_IROTH);
	//ecriture d'un block de BLOCKSIZE \0 pour indiquer la fin de l'archive
	write_block(fd, NULL);
	close(fd);
	return 1;

}

int create_dir(char *name){ //Créer un dossier dans un tar si possible
	char tarfile[strlen(name)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(name)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(name, ".tar") - name; //Existe car on sait qu'il y a un tar dans le chemin
	strncpy(tarfile, name, tarpos+4);
	strncpy(namefile, name+tarpos+5, strlen(name)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(name)-tarpos-4] = '\0';
	if (name[tarpos+4] != '/'){ //Si après le .tar il n'y a pas de / => Il y a une erreur dans le nom du fichier
		char format[72 + strlen(name)];
		sprintf(format, "mkdir : impossible d'accéder à '%s' : Aucun fichier ou dossier de ce type\n", name);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		return -1;
	}
	//write(STDOUT_FILENO, tarfile, strlen(tarfile));
	if(exist_dir(namefile, tarfile)){
		char format[60 + strlen(name)];
		sprintf(format, "mkdir: impossible de créer le répertoire « %s »: Le fichier existe\n", name);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}else{
		struct posix_header header;
		int fd = open(tarfile, O_RDWR + O_APPEND);
		if(fd == -1){
		  perror("erreur d'ouverture de l'archive");
		  return -1;
		}
		int n = 0;
		int read_size = 0;
		while((n=read(fd, &header, BLOCKSIZE))>0){
			//write(STDOUT_FILENO, "yes", 3);
			if(strcmp(header.name, "\0") == 0){
				write(STDOUT_FILENO, "yes", 3);
				//On est a la fin du tar, on écrit un nouveau header de dossier, puis un nouveau block de 512 vide
				struct posix_header header2;
				create_header(namefile, &header2);
				write(fd, &header2, BLOCKSIZE);
				write_block(fd, NULL);
				break;
			}else{
				get_header_size_mk(&header, &read_size);
				if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
					perror("erreur de lecture de l'archive");
					return -1;
				}
			}

		}
		close(fd);
	}
	return 0;
}

int exist_file(char *name){
	struct stat buffer;
	return (stat (name, &buffer) == 0);
}

int exist_dir(char *namefile, char *tarfile){
<<<<<<< HEAD
<<<<<<< HEAD
	//write(STDOUT_FILENO, "\n",1);
	//write(STDOUT_FILENO, tarfile, strlen(tarfile));
=======
	write(STDOUT_FILENO, "\n",1);
	write(STDOUT_FILENO, tarfile, strlen(tarfile));
>>>>>>> avancement mkdiré
=======
	//write(STDOUT_FILENO, "\n",1);
	//write(STDOUT_FILENO, tarfile, strlen(tarfile));
>>>>>>> Mkdir écrit bien le nouveau dossier dans le tar, mais créer un tar invalide pour le moment
	struct posix_header header;
	int fd = open(tarfile, O_RDONLY);
	if(fd == -1){
	  perror("erreur d'ouverture de l'archive");
	  return -1;
	}
	int n = 0;
	int read_size = 0;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, "\0") == 0){
			break;
		}
		if(strcmp(namefile, header.name) == 0){
			return 1;
		}
		get_header_size_mk(&header, &read_size);
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	close(fd);
	return 0;
}

int create_header(char *name, struct posix_header *header){ //Meilleur méthode pour le faire ?
	for(int i = 0; i < 100; i++){
		if(i < strlen(name)){
			header->name[i] = name[i];
		}else{
			header->name[i] = '\0';
		}
	}
	sprintf(header->mode,"0000755");
	for (int i = 0; i < 8; i++) header->uid[i] = '\0';
	for (int i = 0; i < 8; i++) header->gid[i] = '\0';
	unsigned int taille = 0;
	sprintf(header->size, "%011o", taille);

	for (int i = 0; i < 12; i++) header->mtime[i] = '\0';
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
	for (int i = 0; i < 32; i++) header->uname[i] = '\0';
	for (int i = 0; i < 32; i++) header->gname[i] = '\0';
	for (int i = 0; i < 8; i++) header->devmajor[i] = '\0';
	for (int i = 0; i < 8; i++) header->devminor[i] = '\0';
	for (int i = 0; i < 155; i++) header->prefix[i] = '\0';
	for (int i = 0; i < 12; i++) header->junk[i] = '\0';

	set_checksum(header);
	return 0;
}

int is_ext_mk(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_mk(char *file){
	return is_ext_mk(file, ".tar") || is_ext_mk(file, ".tar/");
}

int write_block(int fd, struct posix_header* header){
	if (header == NULL){
		char block[BLOCKSIZE];
		memset(block, '\0', 512);
		if(write(fd, block, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'archive");
			exit(EXIT_FAILURE);
		}
	}else{
		if(write(fd, &header, BLOCKSIZE) < BLOCKSIZE){
			perror("Erreur d'écriture dans l'aarchive");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

int contains_tar_mk(char *file){
	return (strstr(file,".tar") != NULL);
}

void get_header_size_mk(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
}
