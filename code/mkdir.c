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

int exist_dir(char *);
int create_dir(char *);
int create_tar(char *);
int try_create_dir(char *);
int contains_tar_mk(char *);
int is_tar_mk(char *);
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
					if(argv[1][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
						try_create_dir(argv[1]);
					}else{
						char file[strlen(getenv("TWD")) + strlen(argv[1])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[1]);
						try_create_dir(file);
					}
				}else{
					char file[strlen(getenv("TWD")) + strlen(argv[1])];
					sprintf(file, "%s/%s", getenv("TWD"), argv[1]);
					try_create_dir(file);
				}
			}
			else{
				try_create_dir(argv[1]);
			}
		}
	}
	return 0;
}

int try_create_dir(char *name){
	write(STDOUT_FILENO, name, strlen(name));
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
	return 1;

}

int create_dir(char *name){ //Créer un dossier dans un tar si possible
	if(exist_dir(name)){
		char format[60 + strlen(name)];
		sprintf(format, "mkdir: impossible de créer le répertoire « %s »: Le fichier existe\n", name);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}else{
		create_dir(name);
	}
	return 0;
}

int exist_dir(char *name){
	struct posix_header header;
	int fd = open(name, O_RDONLY);
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
		if(strcmp(name, header.name) == 0){
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

int is_ext_mk(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_mk(char *file){
	return is_ext_mk(file, ".tar") || is_ext_mk(file, ".tar/");
}

int contains_tar_mk(char *file){
	return (strstr(file,".tar") != NULL);
}

void get_header_size_mk(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
}
