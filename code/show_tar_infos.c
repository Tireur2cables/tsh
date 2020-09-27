#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "tar.h"

void show_header_infos(struct posix_header *, int *);

int main(int argc, char *argv[]){
	if(argc == 0 || argc == 1) printf("Aucun fichier passé en paramètre !\n");
	else{
		char *file = argv[1];
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
				printf("---- FIN DE L'AFFICHAGE ----\n");
				return 0;
			}
			show_header_infos(header, &read_size);
			read(fd, header, BLOCKSIZE*read_size);
		}
		printf("\n");
		close(fd);
	}
	return 0;
}

void show_header_infos(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	printf("Nom du premier fichier de l'archive : %s\n", header->name);
	printf("Le premier fichier de l'archive fait %d octet(s).\n", taille);
	printf("Le premier fichier de l'archive est %s.\n", (header->typeflag=='0')?"un fichier standard":(header->typeflag=='5')?" un repertoire":"un type inconnu");
}
