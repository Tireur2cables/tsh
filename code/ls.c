#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include "tar.h"

void show_simple_header_infos(struct posix_header *, int *);
void show_complete_header_infos(struct posix_header *, int *);
int print_normal_dir(DIR*);
int print_inoeud_normal_dir(DIR*);
int print_dir(char *, char *);
//fixme implémenter
int check_options(char *);

//FONCTION LS
/*TODO : FONCTION CHECK_OPTIONS
		 FONCTION is_tar et supprimé options -t
		 REMPLACER TOUT LES PRINTF PAR DES WRITE
		 GERER LES OPTIONS (-a dans ., dans
		 TAR : - Afficher les droits correctement - nombre de references - createur - date
		 	   - Changer le read inutile en lseek

*/

int main(int argc, char *argv[]){
	if(argc == 0){
		errno = EINVAL;
		perror("erreur du programmee");
	}
	else{
		if(argc == 1){
			print_dir(".", "\0");
		}else if(argc == 2){
			print_dir(argv[1], "\0");
		}else if(argc == 3){
			//check_options(argv[2]);
			print_dir(argv[1], argv[2]);
		}

	}
	return 0;
}

int print_dir(char *file /*NULL si repetoire = .*/, char *options/*NULL si pas d'options*/){
	if(strcmp(file, ".") == 0){ //repertoire courant
		//fixme gerer les options
		/*
		if(option == x){
		}else{
		}*/
		DIR* dir = opendir(file);
		print_normal_dir(dir);
		closedir(dir);

	}else{ //repetoire en paramètre
		//fixme gerer les options
		/*if(is_tar(file)){
			if(option == x){
			}else{
			}
		}else{
			if(option == x){
			}else{
		}
		}*/
		if(strcmp(options, "\0") == 0){ //pas d'options
			char *arg = ".";
			DIR* dir = opendir(arg);
			print_normal_dir(dir);
			closedir(dir);
		}else{
			//fixme fonctions istar
			if(strcmp(options, "\t")){ //temporaire pour gérer les tars a remplacer par une fonction is_tar(char *file)
				struct posix_header * header = malloc(sizeof(struct posix_header));
				assert(header);

				int fd = open(file, O_RDONLY);
				if(fd == -1){
			      perror("erreur d'ouverture du fichier");
				  return -1;
			    }

				int n = 0;
				int read_size = 0;
				while((n=read(fd, header, BLOCKSIZE))>0){
					if(strcmp(header->name, "\0") == 0){
						return 0;
					}
					show_complete_header_infos(header, &read_size);
					/*if((strcmp(options, "\0") == 0)){
						show_simple_header_infos(header, &read_size);
					}
					else if((strcmp(options, "-l") == 0)){
						show_complete_header_infos(header, &read_size);
					}*/
					/*TODO : Changer en lseek*/
					read(fd, header, BLOCKSIZE*read_size);
				}
				printf("\n");
				close(fd);
			}
			else{
				char *arg = ".";
				DIR* dir = opendir(arg);
				print_inoeud_normal_dir(dir);
				closedir(dir);
			}
		}
	}
	printf("\n");
	return 0;
}

void show_complete_header_infos(struct posix_header *header, int *read_size){
	int taille = 0;
	int mode = 0;
	sscanf(header->size, "%o", &taille);
	sscanf(header->mode, "%o", &mode);
	*read_size = ((taille + 512-1)/512);
	//fixme Afficher les droits correctement - nombre de references - createur - date
	//fixme printf
	printf("%c%o x user user %d date %s\n", ((header->typeflag=='0')?'-':(header->typeflag=='5')?'d':'-'), mode, taille, header->name);
}
void show_simple_header_infos(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	//fixme printf
	printf("%s  ", header->name);
}

int print_normal_dir(DIR* dirp){
	struct dirent *entry;
	while((entry = readdir(dirp)) != NULL){
		//fixme printf
		printf("%s  ", entry->d_name);
	}
	return 0;
}

int print_inoeud_normal_dir(DIR* dirp){
	struct dirent *entry;
	while((entry = readdir(dirp)) != NULL){
		if(entry->d_name[0] != '.'){
			//fixme printf
			printf("%ld %s  ", entry->d_ino, entry->d_name);
		}
	}
	return 0;
}
