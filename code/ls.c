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
int print_complete_normal_dir(DIR*);
int print_dir(char *, char *);
int print_tar(char *, char *);
int print_inside_tar(char *, char *);
int print_rep(char *, char *);
int is_tar(char *);
int contains_tar(char *);
int check_options(char *);
int is_options(char *);

//FONCTION LS
/*TODO : REMPLACER TOUT LES PRINTF PAR DES WRITE
		 Afficher les droits correctement - nombre de references - createur - date
		 traiter le cas d'un fichier dans un tar
		 gerer la cas ou on donne repertoire/fichier
*/

int main(int argc, char *argv[]){
	print_tar("tonton.tar/somedir", "\0");
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

int print_dir(char *file, char *options){
	char *cp = malloc(sizeof(file)+1);
	assert(cp);
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
	//RECUPERER LE TAR
	//RECUPERER LE FICHIER A AFFICHER
	//ENUMERER LE HEADER DU TAR
	//SI FICHIER EST UN FICHIER -> AFFICHER LE CHEMIN
	//SI FICHIER EST UN REPERTOIRE -> AFFICHER LES FICHIERS DE CE REPERTOIRE
	return 0;
}

int print_tar(char *file, char *options){
	if(strcmp(options, "\0") == 0){ //pas d'option
		struct posix_header * header = malloc(sizeof(struct posix_header));
		assert(header);
		int fd = open(file, O_RDONLY);
		if(fd == -1){
		  perror("erreur d'ouverture du fichier");
		  exit(EXIT_FAILURE);
		}

		int n = 0;
		int read_size = 0;
		while((n=read(fd, header, BLOCKSIZE))>0){
			if(strcmp(header->name, "\0") == 0){
				break;
			}
			show_simple_header_infos(header, &read_size);
			if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
				perror("erreur de lecture de l'archive");
				exit(EXIT_FAILURE);
			}
		}
		printf("\n");
		close(fd);
	}else{
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
				break;
			}
			show_complete_header_infos(header, &read_size);
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
	DIR* dir;
	if(strcmp(options, "\0") == 0){ //pas d'options
		if((dir = opendir(file)) == NULL){
			perror("erreur");
			exit(EXIT_FAILURE); //fixme mettre la valeur d'erreur
		}
		print_normal_dir(dir);
		closedir(dir);
		printf("\n");
	}else{
		if((dir = opendir(file)) == NULL){
			perror("erreur");
			exit(EXIT_FAILURE); //fixme mettre la valeur d'erreur
		}
		print_complete_normal_dir(dir);
		closedir(dir);
	}
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
	int mode = 0;
	sscanf(header->size, "%o", &taille);
	sscanf(header->mode, "%o", &mode);
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

int print_complete_normal_dir(DIR* dirp){
	struct dirent *entry;
	while((entry = readdir(dirp)) != NULL){
		if(entry->d_name[0] != '.'){
			//fixme printf
			printf(" x user user date %s\n",entry->d_name);
		}
	}
	return 0;
}

int contains_tar(char *file){
	char *token;
	token = strtok(file, "/");
	while(token != NULL){
		if(is_tar(token)){
			return 1;
		}
		token = strtok(NULL, ".");
	}
	return 0;
}

int is_tar(char *file){
	char *token, *last;
	last = token = strtok(file, ".");
	while(token != NULL){
		last = token;
		token = strtok(NULL, ".");
	}
	if(last == NULL) return -1;
	return (strcmp(last, "tar") == 0);
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
