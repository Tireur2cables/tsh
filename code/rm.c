#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tar.h"
#include "rm.h"

int delete_dir_tar(char * , char *);
int delete_file_tar(char * , char *);
int detectError_rm(int );
int isSameDir_rm(char *, char *);
int is_ext_rm(char *, char *);
int is_tar_rm(char *);
int contains_tar_rm(char *);
void which_rm(char *, int);
int delete_in_tar(char *, int);
void delete_tar(char *, int);
int get_header_size_rm(struct posix_header *, int *);


int rm_func(int argc , char ** argv) {

	if (detectError_rm(argc)) return -1;
	int withOption = 0;
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i],"-r")==0 || strcmp(argv[i],"-R")==0 || strcmp(argv[i],"--recursive")==0) withOption++;
	}
	if (withOption==argc-1) { // si que des options
		errno=EINVAL;
		perror("missing operand");
		return -1;
	}
	for(int i=1;i<argc;i++) {
		if(strcmp(argv[i],"-r")!=0 && strcmp(argv[i],"-R")!=0 && strcmp(argv[i],"--recursive")!=0) {
			if(getenv("TWD") != NULL){
				if(contains_tar_rm(getenv("TWD"))){
					if(argv[i][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
						which_rm(argv[i], withOption);
					}else{
						char file[strlen(getenv("TWD")) + strlen(argv[i])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
						which_rm(file, withOption);
					}
				}else{
					char file[strlen(getenv("TWD")) + strlen(argv[i])];
					sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
					which_rm(file, withOption);
				}
			}else{
				which_rm(argv[i], withOption);
			}
		}
	}
	return 0;
}

/*
* Choix de quelle fonction rm à employée
*/
void which_rm(char *chemin, int withOption){
	if(is_tar_rm(chemin)){
		delete_tar(chemin, withOption); //Si option -r, on peut supprimer un tar complet
	}
	else if(contains_tar_rm(chemin)){  //On supprimer un fichier ou dossier dans un tar
		delete_in_tar(chemin, withOption);
	}else{
		char *format;
		switch(fork()){  // sans tar dans le chemin
			case -1 :
			format = "erreur de fork";
			if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			case 0 :
				execlp("rm", "rm", chemin, (withOption<=0)?NULL:"-r", NULL);
				format = "erreur de exec";
				if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
					perror("Erreur d'écriture dans le shell");
					exit(EXIT_FAILURE);
				}
			default:
				wait(NULL);
				break;
		}
	}
}

void delete_tar(char *chemin, int withOption){
	if( withOption<=0) { //On ne peut pas supprimer un dossier avec rm sans option
		char *deb  = "rm : ";
		char *end = " est un répertoire !\n";
		char error[strlen(deb) + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen){
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		return;
	}
	else {
		if(remove(chemin)!=0) {
			perror("erreur de supression du fichier");
			return;
		}
	}
}

int delete_in_tar(char *chemin, int option) {
	char tarfile[strlen(chemin)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(chemin)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(chemin, ".tar") - chemin; //Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
	strncpy(tarfile, chemin, tarpos+4);
	strncpy(namefile, chemin+tarpos+5, strlen(chemin)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(chemin)-tarpos-4] = '\0';

	int fd = open(tarfile, O_RDONLY);
	if (fd == -1) {
		char *error_debut = "rm : erreur, impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(tarfile) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, tarfile);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	struct posix_header header;
	int n = 0;
	int read_size = 0;
	int found = 0;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, "\0") == 0){
			break;
		}
		if(strcmp(header.name, namefile) == 0 || isSameDir_rm(namefile, header.name)) { //Si on trouve un fichier ou un dossier du bon nom
			if(header.typeflag == '5'){
				found = 1;
			}else{
				found = 2;
			}
		}else{
			get_header_size_rm(&header, &read_size);
		}
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	if(!found){
		char format[60 + strlen(chemin)];
		sprintf(format, "rm : %s: Aucun fichier ou dossier de ce type\n", chemin);
		write(STDOUT_FILENO, format, strlen(format));
	}
	close(fd);
	if (found == 1) {
		if(option<=0) {
			char *deb  = "rm : ";
			char *end = " est un répertoire !\n";
			char error[strlen(deb) + strlen(chemin) + strlen(end) + 1];
			strcpy(error, deb);
			strcat(error, chemin);
			strcat(error, end);
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Erreur d'écriture dans le shell!");
			return -1;
		} else {
			delete_dir_tar(tarfile, namefile);
		}
	}
	else { //delete file in tar
		delete_file_tar(tarfile,namefile);
	}
	return 0;
}

int delete_dir_tar(char *absolutetar, char *chemin) {

	int fd = open(absolutetar,O_RDWR);
	struct posix_header * header = malloc(sizeof(struct posix_header));

	if (fd == -1) {
		char *error_debut = "rm : erreur, impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;
	while(!found) {
		if(read(fd,header,BLOCKSIZE)<BLOCKSIZE) break;

		char nom[strlen(header->name)+1];
		strcpy(nom,header->name);

		if(strcmp(nom,"\0")==0) break;

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header->size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);

		if(strncmp(nom, chemin, strlen(chemin)) == 0 && ((chemin[strlen(chemin)-1] == '/') || (chemin[strlen(chemin)-1] != '/' && nom[strlen(chemin)] == '/')) ) {

			off_t position;
			off_t endposition;

			if((position=lseek(fd, -BLOCKSIZE , SEEK_CUR))==-1) break;
			if((endposition=lseek(fd, 0 , SEEK_END))==-1) break;

			unsigned int size= endposition - (position+BLOCKSIZE+filesize*BLOCKSIZE);
			char cpy[size];

			if(lseek(fd, position+BLOCKSIZE+filesize*BLOCKSIZE , SEEK_SET)==-1) break;
			read(fd , cpy , size);

			if(lseek(fd, position , SEEK_SET)==-1) break;
			write(fd , cpy , size);

			if(lseek(fd,position,SEEK_SET)==-1) break;

		} else {
			read(fd,header,BLOCKSIZE*filesize);
		}
	}
	close(fd);
	return 0;
}

int delete_file_tar(char * absolutetar , char * chemin) {
	int fd = open(absolutetar,O_RDWR);
	struct posix_header * header = malloc(sizeof(struct posix_header));

	if (fd == -1) {
		char *error_debut = "rm : erreur, impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;
	while(!found) {

		if(read(fd,header,BLOCKSIZE)<BLOCKSIZE) break;

		char nom[strlen(header->name)+1];
		strcpy(nom,header->name);

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header->size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);


		if(strcmp(nom, chemin) == 0) {

			off_t position;
			off_t endposition;

			if((position=lseek(fd, -BLOCKSIZE , SEEK_CUR))==-1) break;
			if((endposition=lseek(fd, 0 , SEEK_END))==-1) break;

			unsigned int size= endposition - (position+BLOCKSIZE+filesize*BLOCKSIZE);
			char cpy[size];

			if(lseek(fd, position+BLOCKSIZE+filesize*BLOCKSIZE , SEEK_SET)==-1) break;
			read(fd , cpy , size);

			if(lseek(fd, position , SEEK_SET)==-1) break;
			write(fd , cpy , size);

			found=1;
			break;
		}
		read(fd,header,BLOCKSIZE*filesize);
	}
	close(fd);
	return 0;
}
int detectError_rm(int argc) {

	if(argc==0) {
		errno=EINVAL;
		perror("program error");
		exit(EXIT_FAILURE);
		return -1;
	}
	if(argc==1) {
		errno=EINVAL;
		perror("missing operand");
		return -1;
	}
	return 0;
}

int contains_tar_rm(char *file){
	return (strstr(file,".tar") != NULL);
}

int is_ext_rm(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_rm(char *file){
	return is_ext_rm(file, ".tar") || is_ext_rm(file, ".tar/");
}
int get_header_size_rm(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	return 0;
}

int isSameDir_rm(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}
