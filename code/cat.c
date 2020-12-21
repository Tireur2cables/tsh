#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cat.h"
#include <string.h>
#include "tar.h"
#include <stdlib.h>

int is_tar_cat(char *);
int contains_tar_cat(char *);
int is_ext_cat(char *, char *);
int cat_tar(char *, char *);
int cat_file(char *, char *);
ssize_t read_line(int, char *, size_t);
int get_header_size_cat(struct posix_header *, int *);
void write_content(int, int);


/*
* Affiche sur stdout read_size BLOCK (512 octets) du fichier fd
*/
void write_content(int fd, int read_size){
	for(int i = 0; i < read_size; i++){
		char format[BLOCKSIZE];
		read(fd, format, BLOCKSIZE);
		write(STDOUT_FILENO, format, (strlen(format)));
	}
}

/*
* Appel de la fonction, gestion des options et du nom du fichier a cat
*/
int cat(int argc, char *argv[]) {
	if(argc == 1){
		cat_file(NULL, "\0");
	}
	if(getenv("TWD") != NULL){
		if(is_tar_cat(getenv("TWD"))){
			if(argv[1][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
				cat_file(argv[1], "\0");
			}else{
				char file[strlen(getenv("TWD")) + strlen(argv[1])];
				sprintf(file, "%s/%s", getenv("TWD"), argv[1]);
				cat_file(file, "\0");
			}
		}else{
			char file[strlen(getenv("TWD")) + strlen(argv[1])];
			sprintf(file, "%s/%s", getenv("TWD"), argv[1]);
			cat_file(file, "\0");
		}
	}else{
		cat_file(argv[1], "\0");
	}
	return 0;
}

/*
*
Fonction générale qui gére les différents cas dans lesquels on peut se trouver : - Afficher un tar -> erreur
																				 - Pas de Tar dans le nom -> Ne dois pas arriver -> Erreur
																				 - Un fichier dans un tar -> Appel de cat_tar

*/
int cat_file(char *file, char *options){
	if(file == NULL){
		execlp("cat", "cat", NULL);
	}
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
		execlp("cat", "cat", file, NULL);
		char *format = "Erreur du programme, la fonction cat externe aurait du etre appelée";
		if(write(STDOUT_FILENO, format, strlen(format)) < strlen(format))  {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		return 0;
	}
	return 0;
}
/*
* Parcours le tar et appelle write_content sur les parties du fichier qui correspondent au nom de fichier demandé
*
*/
int cat_tar(char *file, char *options){
		char tarfile[strlen(file)]; //Contient le chemin jusqu'au tar pour l'ouvrir
		char namefile[strlen(file)]; //Contient la suite du chemin pour l'affichage
		int tarpos = strstr(file, ".tar") - file; //Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
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
		int found = 0;
		while((n=read(fd, &header, BLOCKSIZE))>0){
			if(strcmp(header.name, "\0") == 0){
				break;
			}
			if(strstr(header.name, namefile) != NULL && (strncmp(header.name, namefile, strlen(header.name)-1) == 0)) {
				get_header_size_cat(&header, &read_size);
				found = 1;
				write_content(fd, read_size);
			}else{
				get_header_size_cat(&header, &read_size);
			}

			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				return -1;
			}
		}
		if(!found){
			char format[60 + strlen(file)];
			sprintf(format, "cat : %s: Aucun fichier ou dossier de ce type\n", file);
			write(STDOUT_FILENO, format, strlen(format));
		}
		close(fd);
	return 0;
}

/*
* Renvoit le nombre de block, decrivant le contenu du fichier, suivant le header
*/
int get_header_size_cat(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	return 0;
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
