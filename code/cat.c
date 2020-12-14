#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cat.h"
#include <string.h>
#include "tar.h"
#include <stdlib.h>

char read_line_buf[BUFFSIZE];
int read_line_pos;
int read_line_siz = 0;

int cat(int argc, char *argv[]) {
	char *file = argv[1];
	cat_file(file, "\0");
	return 0;
}

int write_tar(struct posix_header *header){
	write(STDOUT_FILENO, "yes", 3);
	int taille;
	sscanf(header->size, "%o", &taille);
	char format[taille+1];
	memcpy(format, header, taille);
	write(STDOUT_FILENO, format, strlen(format));
	return 0;
}
int get_header_size_cat(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	return 0;
}

int cat_file(char *file, char *options){ //Fonction générale qui gère dans quel cas on se trouve
	char cp[strlen(file)+1];
	strcpy(cp, file);
	if(contains_tar_cat(cp)){
		if(is_tar_cat(cp)){
			char format[5 + strlen(cp) + 17];
			sprintf(format, "cat : %s : est un dossier\n", cp);
			if(write(STDOUT_FILENO, format, strlen(format)) < strlen(format))  {
				perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}else{
			cat_tar(file, options);
		}

	}else{
		char *format = "WIP";
		if(write(STDOUT_FILENO, format, strlen(format)) < strlen(format))  {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		return 0;
	}
	return 0;
}

int cat_tar(char *file, char *options){ //Fonction générale qui gère dans quel cas on se trouve
		char tarfile[strlen(file)]; //Contient le chemin jusqu'au tar pour l'ouvrir
		char namefile[strlen(file)]; //Contient la suite du chemin pour l'affichage
		int tarpos = strstr(file, ".tar") - file; //Existe car on sait qu'il y a un tar dans le chemin
		strncpy(tarfile, file, tarpos+4);
		strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
		tarfile[tarpos+4] = '\0';
		namefile[strlen(file)-tarpos-4] = '\0';
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
			if(strstr(header.name, namefile) != NULL && (strncmp(header.name, namefile, strlen(header.name)-1) == 0)) {
				write_tar(&header);
			}else{
				get_header_size_cat(&header, &read_size);
			}

			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				return -1;
			}
		}
		close(fd);
	return 0;
}

ssize_t read_line(int fd, char *buf, size_t count){
	if(read_line_siz == 0){
		if((read_line_siz = read(fd, read_line_buf, BUFFSIZE)) < 0){
			return -1;
		}
	}
	int i;
	for(i = 0;read_line_pos < read_line_siz && i < count; i++, read_line_pos++){
		buf[i] = read_line_buf[read_line_pos];
		if(buf[i] == '\n'){
			i++;
			break;
		}
	}
	read_line_pos++;
	if(read_line_siz == read_line_pos){
		read_line_pos = 0;
		read_line_siz = 0;
	}
	if(i == (count-1) && buf[i-1] != '\n') return -1;
	return i;
}

int contains_tar_cat(char *file){
	return (strstr(file,".tar") != NULL);
}

int is_ext_cat(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_cat(char *file){
	return is_ext_cat(file, ".tar");
}
