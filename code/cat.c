#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "cat.h"
#include "tar.h"

int is_tar_cat(char *);
int contains_tar_cat(char *);
int is_ext_cat(char *, char *);
int cat_tar(char *, char *);
int cat_file(char *, char *);
ssize_t read_line(int, char *, size_t);
int get_header_size_cat(struct posix_header *, int *);
void write_content(int, int);


/*
* Appel de la fonction, gestion des options et du nom du fichier a cat
*/
int cat(int argc, char *argv[]) {
	if(argc == 1){
		cat_file(NULL, "\0");
	}
	char *twd = getenv("TWD");
	if (twd != NULL) {
		if (contains_tar_cat(twd)) {
			if (argv[1][0] == '/') // Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
				cat_file(argv[1], "");
			else {
				char file[strlen(twd) + 1 + strlen(argv[1]) + 1];
				sprintf(file, "%s/%s", twd, argv[1]);
				cat_file(file, "\0");
			}
		}else {
			char file[strlen(twd) + 1 + strlen(argv[1]) + 1];
			sprintf(file, "%s/%s", twd, argv[1]);
			cat_file(file, "");
		}
	}else
		cat_file(argv[1], "");

	return 0;
}

/*
*
Fonction générale qui gére les différents cas dans lesquels on peut se trouver : - Afficher un tar -> erreur, c'est un dossier
																				 - Pas de Tar dans le nom -> On sort du tar avec des .., on appelle donc la fonction externe
																				 - Un fichier dans un tar -> Appel de cat_tar

*/
int cat_file(char *file, char *options) {
	if(file == NULL)
		execlp("cat", "cat", NULL);

	char cp[strlen(file)+1];
	strcpy(cp, file);
	if (contains_tar_cat(cp)) {
		if (is_tar_cat(cp)) {
			char format[5 + strlen(cp) + 17];
			sprintf(format, "cat : %s : est un dossier\n", cp);
			int formatlen = strlen(format);
			if(write(STDERR_FILENO, format, formatlen) < formatlen)  {
				perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}else
			cat_tar(file, options);

	}else {
		execlp("cat", "cat", file, NULL);
		char *format = "Erreur du programme, la fonction cat externe aurait du etre appelée";
		int formatlen = strlen(format);
		if(write(STDERR_FILENO, format, formatlen) < formatlen)  {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
		return 0;
	}
	return 0;
}

/*
* Affiche sur stdout read_size BLOCK (512 octets) du fichier fd
*/
void write_content(int fd, int read_size) {
	int readen;
	for (int i = 0; i < read_size; i+=readen) {
		char format[BLOCKSIZE];
		int taille = (read_size - i >= BLOCKSIZE)? BLOCKSIZE : read_size - i;
		if ((readen = read(fd, format, taille)) < 0) {
			char *error = "cat : Erreur impossible de lire le contenu du fichier sur l'entree!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Impossible d'ecrire dans le shell!");
			return;
		}
		if (write(STDOUT_FILENO, format, readen) < readen) {
			char *error = "cat : Erreur impossible d'ecrire le contenu du fichier sur la sortie!\n";
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen)
				perror("Impossible d'ecrire dans le shell!");
			return;
		}
	}

}

/*
* Parcours le tar et appelle write_content sur les parties du fichier qui correspondent au nom de fichier demandé
*
*/
int cat_tar(char *file, char *options) {
		int tarpos = strstr(file, ".tar") - file; // Existe car on sait qu'il y a un tar dans le chemin, arithmétique des pointers pour retrouver la position du .tar dans le nom de fichier
		char tarfile[tarpos+4+1]; // Contient le chemin jusqu'au tar pour l'ouvrir
		strncpy(tarfile, file, tarpos+4);
		tarfile[tarpos+4] = '\0';
		char namefile[strlen(file)-tarpos-4+1]; // Contient la suite du chemin pour l'affichage
		strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
		namefile[strlen(file)-tarpos-4] = '\0';

		struct posix_header header;
		int fd = open(tarfile, O_RDONLY);
		if (fd == -1) {
		  perror("erreur d'ouverture de l'archive");
		  return -1;
		}

		int n = 0;
		int read_size = 0;
		int found = 0;
		while ((n=read(fd, &header, BLOCKSIZE)) > 0) {
			if (strcmp(header.name, "\0") == 0) break;

			if (strstr(header.name, namefile) != NULL && (strncmp(header.name, namefile, strlen(header.name)-1) == 0)) {
				if (header.typeflag == '5') { //On ne peut pas afficher de dossier
					char format[25 + strlen(file)];
					sprintf(format, "cat : %s: est un dossier\n", file);
					int formatlen = strlen(format);
					if (write(STDERR_FILENO, format, formatlen) < formatlen) {
						perror("Erreur d'écriture dans le shell!");
						exit(EXIT_FAILURE);
					}
					return 0;
				}else {
					get_header_size_cat(&header, &read_size);
					found = 1;
					unsigned int size;
					sscanf(header.size, "%o", &size);
					write_content(fd, size);
				}
			}
			else get_header_size_cat(&header, &read_size);


			if (lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1) {
				perror("erreur de lecture de l'archive");
				return -1;
			}
		}
		if (!found) {
			char format[60 + strlen(file)];
			sprintf(format, "cat : %s: Aucun fichier ou dossier de ce type\n", file);
			int formatlen = strlen(format);
			if (write(STDERR_FILENO, format, formatlen) < formatlen) {
				perror("Erreur d'écriture dans le shell!");
				exit(EXIT_FAILURE);
			}
		}
		close(fd);
	return 0;
}

/*
* Renvoit le nombre de block, decrivant le contenu du fichier, suivant le header
*/
int get_header_size_cat(struct posix_header *header, int *read_size) {
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
	return 0;
}

int contains_tar_cat(char *file) {
	return (strstr(file,".tar") != NULL);
}

int is_ext_cat(char *file, char *ext) {
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_cat(char *file) {
	return is_ext_cat(file, ".tar");
}
