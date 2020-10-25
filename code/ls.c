#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "tar.h"
#include "ls.h"

//FONCTION LS
/*TODO :
		 nombre de references dans un tar
		 ls -l sur un fichier classique
		 affichage d'un dossier dans un tar par parfait
		 fichier dans un tar qui n'existe pas : Pour le moment on affiche rien -> Ecrire une erreur : ce fichier n'existe pas
*/

int ls(int argc, char *argv[]){
	if(argc == 0){
		errno = EINVAL;
		perror("erreur du programme");
		exit(EXIT_FAILURE);
	}
	else{
		if(argc == 1){
			print_dir(".", "\0");
		}else if(argc == 2){ // 2 cas - Options et repertoire =  . ou repertoire et pas d'options
			if(is_options(argv[1])){
				check_options(argv[1]);
				print_dir(".", argv[1]);
			}else{
				print_dir(argv[1], "\0");
			}
		}else if(argc == 3){
			check_options(argv[1]);
			print_dir(argv[2], argv[1]);
		}

	}
	return 0;
}

int print_dir(char *file, char *options){ //Fonction générale qui gère dans quel cas on se trouve
	char cp[strlen(file)+1];
	strcpy(cp, file);
	if(is_tar(cp)){
		print_tar(file, options);
	}else if(contains_tar(cp)){
		print_inside_tar(file, options);
	}else{
		print_rep(file, options);
	}
	return 0;
}

int print_inside_tar(char *file, char *options){
	char tarfile[strlen(file)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(file)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(file, ".tar") - file; //Existe car on sait qu'il y a un tar dans le chemin
	strncpy(tarfile, file, tarpos+4);
	strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(file)-tarpos-4] = '\0';
	if(strcmp(options, "\0") == 0){ //pas d'option
		struct posix_header header;
		int fd = open(tarfile, O_RDONLY);
		if(fd == -1){
		  perror("erreur d'ouverture de l'archive");
		  return -1;
		}
		int n = 0;
		int read_size = 0;
		while((n=read(fd, &header, BLOCKSIZE))>0){ //FIXME : Si on ne trouve pas de fichier : erreur
			if(strcmp(header.name, "\0") == 0){
				break;
			}if(strstr(header.name, namefile) != NULL && (strncmp(header.name, namefile, strlen(header.name)-1) != 0)) {
				show_simple_header_infos(&header, &read_size);
			}else{
				get_header_size(&header, &read_size);
			}

			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				return -1;
			}
		}
		printf("\n");
		close(fd);
	}else{
		struct posix_header header;
		int fd = open(tarfile, O_RDONLY);
		if(fd == -1){
		  perror("erreur d'ouverture de l'archive");
		  return -1;
		}
		int n = 0;
		int read_size = 0;
		while((n=read(fd, &header, BLOCKSIZE))>0){ //FIXME : Si on ne trouve pas de fichier : erreur
			if(strcmp(header.name, "\0") == 0){
				break;
			}
			if(strstr(header.name, namefile) != NULL && (strncmp(header.name, namefile, strlen(header.name)-1) != 0)) {
				show_complete_header_infos(&header, &read_size);
			}else{
				get_header_size(&header, &read_size);
			}
			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				exit(EXIT_FAILURE);
			}
		}
		close(fd);
	}
	return 0;
}

int print_tar(char *file, char *options){
	if(strcmp(options, "\0") == 0){ //pas d'option
		struct posix_header header;
		int fd = open(file, O_RDONLY);
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
			show_simple_header_infos(&header, &read_size);
			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				return -1;
			}
		}
		printf("\n");
		close(fd);
	}else{ //ls -l
		struct posix_header header;
		int fd = open(file, O_RDONLY);
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
			show_complete_header_infos(&header, &read_size);
			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				exit(EXIT_FAILURE);
			}
		}
		close(fd);
	}
	return 0;
}

int print_rep(char *file, char *options){
	struct stat statbuf;
	if(stat(file, &statbuf) == -1){ //Cas d'un nom de fichier absent
		perror("Impossible d'ouvrir le fichier");
		return -1;
	}else if(S_ISREG(statbuf.st_mode)){ //Cas d'un fichier simple
		char format[strlen(file) + 2];
		strcpy(format, file);
		strcat(format, "\n");
		if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		return 0;
	}
	if(strcmp(options, "\0") == 0){ //pas d'options
		print_normal_dir(file);
	}else{ //ls -l
		print_complete_normal_dir(file);
	}
	return 0;
}

void show_complete_header_infos(struct posix_header *header, int *read_size){
	int taille, mode, uid, gid;
	char name[strlen(header->name)];
	char mode_str[10];
	char typeformat[2];
	long int mtime;
	sscanf(header->name, "%s", name);
	sscanf(header->size, "%o", &taille);
	char taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1):6]; //Les tailles ne sont plus alignés au dessus de 6 chiffres
	sprintf(taille_str, "%d", taille);
	for(int i = nbdigit(taille); i < 6; i++){ //On complète la string avec des espaces afin d'avoir un alignement
		taille_str[i] = ' ';
	}
	taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1)-1:5] = '\0';

	sscanf(header->mode, "%o", &mode);
	convert_mode(mode, mode_str);
	sscanf(header->uid, "%o", &uid);
	sscanf(header->gid, "%o", &gid);
	sscanf(header->mtime, "%lo", &mtime);
	char *pw_name = getpwuid(uid)->pw_name;
	char *gr_name = getgrgid(gid)->gr_name;
	char *date= ctime(&mtime);
	typeformat[0] = ((header->typeflag=='0')?'-':(header->typeflag=='5')?'d':'-');
	typeformat[1] = '\0';
	date[strlen(date) - 1] = '\0'; // ctime renvoit une string se terminant par \n ...

	*read_size = ((taille + 512-1)/512);
	char format[2*sizeof(int) + 1 + strlen(name) + strlen(date) + strlen(pw_name) + strlen(gr_name)+ 1]; //calcul de taille faux
	strcpy(format, typeformat);
	strcat(format, mode_str);
	strcat(format, " ");
	strcat(format, pw_name);
	strcat(format, " ");
	strcat(format, gr_name);
	strcat(format, " ");
	strcat(format, taille_str);
	strcat(format, " ");
	strcat(format, date);
	strcat(format, " ");
	strcat(format, name);
	strcat(format, "\n");
	//fixme nombre de references
	if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
}

void show_simple_header_infos(struct posix_header *header, int *read_size){
	int taille = 0;
	int mode = 0;
	sscanf(header->size, "%o", &taille);
	sscanf(header->mode, "%o", &mode);
	*read_size = ((taille + 512-1)/512);
	int filename_len = strlen(header->name) + 3;
	char filename[filename_len];
	strcpy(filename, header->name);
	strcat(filename, "  ");
	if (write(STDOUT_FILENO, filename, filename_len) < filename_len) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
}

void get_header_size(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
}

int print_normal_dir(char* file){
	DIR * dirp;
	if((dirp = opendir(file)) == NULL){
		perror("erreur");
		exit(EXIT_FAILURE);
	}
	int taille_totale = 2;
	struct dirent *entry;
	while((entry = readdir(dirp)) != NULL){
		if(entry->d_name[0] != '.'){
			taille_totale += strlen(entry->d_name) + 2;
		}
	}
	//printf("%d", taille_totale);
	closedir(dirp);
	char format[taille_totale];
	format[0] = '\0';
	if((dirp = opendir(file)) == NULL){
		perror("erreur");
		exit(EXIT_FAILURE);
	}
	while((entry = readdir(dirp)) != NULL){
		if(entry->d_name[0] != '.'){
			strcat(format, entry->d_name);
			strcat(format, "  ");
		}
	}
	strcat(format, "\n");
	if (write(STDOUT_FILENO, format, taille_totale) < taille_totale) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
	closedir(dirp);
	return 0;
}

int print_complete_normal_dir(char* file){
	DIR *dirp;
	if((dirp = opendir(file)) == NULL){
		perror("erreur");
		exit(EXIT_FAILURE);
	}
	struct dirent *entry;
	struct stat statbuf;
	while((entry = readdir(dirp)) != NULL){
		if(entry->d_name[0] != '.'){
			char name[strlen(entry->d_name)];
			sscanf(entry->d_name, "%s", name);
			char savefile[strlen(file) + strlen(name)+ 2];
			strcpy(savefile, file);
			strcat(savefile, "/");
			strcat(savefile, name);
			stat(savefile, &statbuf);
			char typeformat[2];
			int uid, gid, mode, taille, nlink;
			uid = statbuf.st_uid;
			gid = statbuf.st_gid;
			mode = statbuf.st_mode;
			taille = statbuf.st_size;
			nlink = statbuf.st_nlink;
			char nlink_str[nbdigit(nlink)];
			sprintf(nlink_str, "%d", nlink);
			char *pw_name = getpwuid(uid)->pw_name;
			char *gr_name = getgrgid(gid)->gr_name;
			char mode_str[10]; //taille prédéfinie (drwxrwxrwx)
			convert_mode(mode, mode_str);
			long int time = statbuf.st_mtime;
			char *date= ctime(&time);
			date[strlen(date) - 1] = '\0';
			char taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1):6]; //Les tailles ne sont plus alignés au dessus de 6 chiffres
			sprintf(taille_str, "%d", taille);
			for(int i = nbdigit(taille); i < 6; i++){ //On complète la string avec des espaces afin d'avoir un alignement
				taille_str[i] = ' ';
			}
			taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1)-1:5] = '\0';
			typeformat[0] = ((S_ISDIR(mode))?'d':'-');
			typeformat[1] = '\0';
			char format[2*sizeof(int) + 1 + strlen(name) + strlen(date) + strlen(nlink_str) + strlen(pw_name) + strlen(gr_name)+ 1]; //calcul de taille faux
			strcpy(format, typeformat);
			strcat(format, mode_str);
			strcat(format, " ");
			strcat(format, nlink_str);
			strcat(format, " ");
			strcat(format, pw_name);
			strcat(format, " ");
			strcat(format, gr_name);
			strcat(format, " ");
			strcat(format, taille_str);
			strcat(format, " ");
			strcat(format, date);
			strcat(format, " ");
			strcat(format, name);
			strcat(format, "\n");
			if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
				perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}
	}
	closedir(dirp);
	return 0;
}

int contains_tar(char *file){
	return (strstr(file,".tar") != NULL);
}

int is_ext(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar(char *file){
	return is_ext(file, ".tar");
}

int is_options(char *options){
	return (options[0] == '-');
}

int check_options(char *options){
	if((strcmp(options, "-l") == 0)|| (strcmp(options, "\0") == 0)){
		return 0;
	}else{
		printf("ls : option invalide -- '%s'\n", options);
		exit(EXIT_FAILURE);
	}
}

int nbdigit(int n){
	int count = 1;
	while(n > 9){
		count++;
		n/=10;
	}
	return count;
}

void convert_mode(mode_t mode, char* res){
	//UTILISATEUR
	if (mode & S_IRUSR)
		*res++ = 'r';
	else
		*res++ = '-';
	if (mode & S_IWUSR)
		*res++ = 'w';
	else
		*res++ = '-';
	if (mode & S_IXUSR)
		*res++ = 'x';
	else
		*res++ = '-';
	//GROUPE
	if (mode & S_IRGRP)
		*res++ = 'r';
	else
		*res++ = '-';
	if (mode & S_IWGRP)
		*res++ = 'w';
	else
		*res++ = '-';
	if (mode & S_IXGRP)
		*res++ = 'x';
	else
		*res++ = '-';
	//OTHER
	if (mode & S_IROTH)
		*res++ = 'r';
	else
		*res++ = '-';
	if (mode & S_IWOTH)
		*res++ = 'w';
	else
		*res++ = '-';
	if (mode & S_IXOTH)
		*res++ = 'x';
	else
		*res++ = '-';
	*res = '\0';
}
