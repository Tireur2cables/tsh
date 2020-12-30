#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include "tar.h"
#include "rmdir.h"

int parcoursChemin_rmdir(char *, char *);
int isAccessibleFrom_rmdir(char *, char *);
int deleteDir(char *);
int isDirTarEmpty(char * , char *);
int DeleteDirTar(char * , char *);
int delete_whole_tar(char *);
int delete_dir_in_tar(char *);
int isDirEmpty(char *);
int parcoursCheminTar_rmdir(char *, char *, char *);
int is_ext_rmdir(char *, char *);
int is_tar_rmdir(char *);
int isSameDir_rmdir(char *, char *);
int contains_tar_rmdir(char *);
void which_rmdir(char *);


int rmdir_func(int argc,char **argv) {
	if(argc==0) {
		errno=EINVAL;
		perror("program error");
		exit(EXIT_FAILURE);
	}

	if(argc==1) {
		errno=EINVAL;
		perror("missing operand");
		return -1;
	}

	for(int i=1;i<argc;i++) {
		if(getenv("TWD") != NULL){
			if(is_tar_rmdir(getenv("TWD"))){
				if(argv[1][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
					which_rmdir(argv[i]);
				}else{
					char file[strlen(getenv("TWD")) + strlen(argv[i])];
					sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
					which_rmdir(file);
				}
			}else{
				char file[strlen(getenv("TWD")) + strlen(argv[i])];
				sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
				which_rmdir(file);
			}
		}else{
			which_rmdir(argv[i]);
		}
	}

	return 0;
}

void which_rmdir(char *chemin){
	if(is_tar_rmdir(chemin)){
		delete_whole_tar(chemin);
	}
	else if(contains_tar_rmdir(chemin)){
		delete_dir_in_tar(chemin);
	}else{
		char *format;
		switch(fork()){  // sans tar dans chemin
			case -1 :
			format = "erreur de fork";
			if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			case 0 :
				execlp("rmdir", "rmdir", chemin, NULL);
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

int delete_whole_tar(char *chemin){
	int fd;
	fd = open(chemin, O_RDONLY);
	struct posix_header header;
	read(fd, &header, BLOCKSIZE);
	if(header.name[0] == '\0'){
		if(remove(chemin)!=0) {
			perror("erreur de supression du fichier");
			return 0;
		}
	}else{
		char format[60 + strlen(chemin)];
		sprintf(format, "rmdir: impossible de supprimer '%s': Le dossier n'est pas vide", chemin);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
	}
	return 1;
}

int delete_dir_in_tar(char *chemin) {
	char tarfile[strlen(chemin)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(chemin)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(chemin, ".tar") - chemin; //Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
	strncpy(tarfile, chemin, tarpos+4);
	strncpy(namefile, chemin+tarpos+5, strlen(chemin)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(chemin)-tarpos-4] = '\0';

	int fd = open(tarfile, O_RDONLY);
	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
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

	int found = 0;
	while (!found) {
		if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

		char nom[strlen(header.name)+1];
		strcpy(nom, header.name);
		if (nom[strlen(nom)-1] == '/' && isSameDir_rmdir(namefile, nom)) found = 1;
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
	close(fd);
	if (!found) {
		char *deb  = "rmdir : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}else if (found == 1) {
		isDirTarEmpty(tarfile, namefile); /// suppression
	}else {
		char *deb  = "rmdir : ";
		char *end = " n'est pas un répertoire!\n";
		char error[strlen(deb) + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	return 0;
}

int isDirTarEmpty(char *tarfile, char *namefile) {

	int fd = open (tarfile,O_RDONLY);
	struct posix_header header;

	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(tarfile) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, tarfile);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;
	int nbfound=-1;

	while(!found) {

		if(nbfound>0) {
			char * deb = "rmdir : le répertoire ";
			char * end = " n'est pas vide";
			char error[strlen(deb)+strlen(namefile)+strlen(end)+1];
			strcpy(error,deb);
			strcat(error,namefile);
			strcat(error,end);
			strcat(error,"\n");
			if(write(STDERR_FILENO,error,strlen(error))<strlen(error))
				perror("Erreur d'écriture dans le shell!");
			return -1;
		}

		if(read(fd,&header,BLOCKSIZE)<BLOCKSIZE) break;
		//if(strcmp(header->name, "\0") == 0) printf("backslash\n");

		char nom[strlen(header.name)+1];
		strcpy(nom,header.name);
		if(strncmp(nom, namefile, strlen(namefile)) == 0) nbfound++;

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header.size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);

		read(fd, &header, BLOCKSIZE*filesize);

		}

	close(fd);
	return DeleteDirTar(tarfile, namefile);
}

int DeleteDirTar(char *tarfile, char *namefile) {

	int fd = open (tarfile,O_RDWR);
	struct posix_header header;

	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(tarfile) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, tarfile);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;
	while(!found) {

		if(read(fd,&header,BLOCKSIZE)<BLOCKSIZE) break;

		char nom[strlen(header.name)+1];
		strcpy(nom,header.name);

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header.size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);
		if(isSameDir_rmdir(namefile, nom)) {

			off_t position;
			off_t endposition;

			if((position=lseek(fd, -BLOCKSIZE , SEEK_CUR))==-1) break;
			if((endposition=lseek(fd, 0 , SEEK_END))==-1) break;

			unsigned int size= endposition - (position+BLOCKSIZE);
			char cpy[size];

			if(lseek(fd, position+BLOCKSIZE , SEEK_SET)==-1) break;
			read(fd , cpy , size);

			if(lseek(fd, position , SEEK_SET)==-1) break;
			write(fd , cpy , size);

			found=1;
			break;

		}

		read(fd, &header, BLOCKSIZE*filesize);

	}

	close(fd);


	return 0;


}

int contains_tar_rmdir(char *file){
	return (strstr(file,".tar") != NULL);
}

int is_ext_rmdir(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_rmdir(char *file){
	return is_ext_rmdir(file, ".tar") || is_ext_rmdir(file, ".tar/");
}

int isSameDir_rmdir(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}
