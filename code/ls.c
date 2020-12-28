#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

void show_simple_header_infos(struct posix_header *, int *);
void get_header_size(struct posix_header *, int *);
void show_complete_header_infos(struct posix_header *, int *);
int print_dir(char *, char *);
int print_tar(char *, char *);
int print_inside_tar(char *, char *);
int get_indice(char *);
int get_indice_pere(char *);
int get_nb_dossier(int);
int is_tar(char *);
int contains_tar(char *);
int check_options(char *);
int is_options(char *);
int is_curr_or_parent_rep(char *);
int is_ext(char *, char *);
int nbdigit(int);
void convert_mode(mode_t, char*);
int get_profondeur(char *);
int get_filename(char *, char*);
int contains_filename(char *, char *);
void get_link(int );

int *tab_link;
char **tab_nom;

//FONCTION LS
/*TODO :
		 nombre de references dans un tar
*/

int ls(int argc, char *argv[]){
	if(argc == 0){
		errno = EINVAL;
		perror("erreur du programme");
		exit(EXIT_FAILURE);
	}
	else if(argc == 1){ //ls du dossier courant sans option
		print_dir(getenv("TWD"), "\0");
	}
	else if(argc == 2 && is_options(argv[1])){ //ls du dossier courant avec option
		check_options(argv[1]);
		print_dir(getenv("TWD"), argv[1]);
	}
	else{ //ls d'un dossier
		char option[2];
		int found = 0;
		for(int i = 1; i < argc; i++){ //On cherche l'option
			if(is_options(argv[i])){
				strcpy(option, argv[1]);
				found = 1;
			}
		}
		if (found == 0){
			strcpy(option, "\0");
		}
		for(int i = 1; i < argc; i++){
			if(!is_options(argv[i])){
				char format[strlen(argv[i]) + 4];
				sprintf(format, "%s : \n", argv[i]);
				if(argc != 2) write(STDOUT_FILENO, format, strlen(format));
				if(getenv("TWD") != NULL){
					if(contains_tar(getenv("TWD"))){
						if(argv[i][0] == '/'){ //Si l'appel ressort du tar (avec .. ou ~ par exemple), alors l'argument est transformé en chemin partant de la racine
							print_dir(argv[i], option);
						}else{
							char file[strlen(getenv("TWD")) + strlen(argv[i])];
							sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
							print_dir(file, option);
						}
					}else{
						char file[strlen(getenv("TWD")) + strlen(argv[i])];
						sprintf(file, "%s/%s", getenv("TWD"), argv[i]);
						print_dir(file, option);
					}
				}
				else{
					print_dir(argv[i], option);
				}
				if(i != argc-1){
					write(STDOUT_FILENO, "\n", 1);
				}
			}
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
	}else{//On se trouve dans un tar, mais on fait ls sur un chemin qui ne contient pas de tar
		char *format;
		switch(fork()){
			case -1 :
			format = "erreur de fork";
			if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
				perror("Erreur d'écriture dans le shell");
				exit(EXIT_FAILURE);
			}
			case 0 :
				execlp("ls", "ls", file, (options[0] == '\0')?NULL:options, NULL);
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
	return 0;
}

/*
* ls dans le cas ls arch.tar/file
*/
int print_inside_tar(char *file, char *options){
	char tarfile[strlen(file)]; //Contient le chemin jusqu'au tar pour l'ouvrir
	char namefile[strlen(file)]; //Contient la suite du chemin pour l'affichage
	int tarpos = strstr(file, ".tar") - file; //Existe car on sait qu'il y a un tar dans le chemin
	strncpy(tarfile, file, tarpos+4);
	strncpy(namefile, file+tarpos+5, strlen(file)-tarpos-4);
	tarfile[tarpos+4] = '\0';
	namefile[strlen(file)-tarpos-4] = '\0';
	if(namefile[strlen(namefile)-1] == '/') namefile[strlen(namefile)-1] = '\0';
	if (file[tarpos+4] != '/'){ //Si après le .tar il n'y a pas de / => Il y a une erreur dans le nom du fichier
		char format[72 + strlen(file)];
		sprintf(format, "ls : impossible d'accéder à '%s' : Aucun fichier ou dossier de ce type\n", file);
		if(write(STDERR_FILENO, format, strlen(format)) < strlen(format)){
			perror("Erreur d'écriture dans le shell");
			exit(EXIT_FAILURE);
		}
		return -1;
	}
	struct posix_header header;
	int fd = open(tarfile, O_RDONLY);
	if(fd == -1){
	  perror("erreur d'ouverture de l'archive");
	  return -1;
	}
	int n = 0;
	int found = 0;
	int read_size = 0;
	tab_link = malloc(sizeof(int)*get_nb_dossier(fd));
	tab_nom = malloc(sizeof(char *)*get_nb_dossier(fd));
	get_link(fd);
	int profondeur = get_profondeur(namefile);
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, "\0") == 0){
			break;
		}
		if (strstr(header.name, namefile) != NULL){ //Inutile de faire plus de tests si le fichier ne contient pas le nom recherché
			int namepos = strstr(header.name, namefile) - header.name;
			if (strncmp(header.name, namefile, strlen(namefile)) == 0){
				found = 1;
			}
			//Si le nom du fichier est exactement celui qu'on recherche (c'est un fichier) ou si on trouve un dossier qui porte se nom, on affiche le contenu a profondeur + 1
			if( (strcmp(header.name, namefile) == 0 && namefile[strlen(namefile)+1] != '/') ||
				(contains_filename(header.name, namefile) &&
				(header.name[namepos + strlen(namefile)] == '/' &&
				header.name[namepos + strlen(namefile)+1] != '\0' &&
				get_profondeur(header.name) == profondeur + 1))) {

				if(strcmp(options, "\0") == 0){ //pas d'option
					show_simple_header_infos(&header, &read_size);
				}
				else{ //ls -l
					show_complete_header_infos(&header, &read_size);
				}
				found = 1;
			}
			else{
				get_header_size(&header, &read_size);
			}
		}
		else{
			get_header_size(&header, &read_size);
		}

		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){ //On avance du nombre de blocks du fichier etudié pour passer au prochain
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	if(found && strcmp(options, "\0") == 0){ //On affiche un retour a la ligne, seulement si on avait pas d'option, car ls -l met un retour a la ligne à la fin de chaque ligne
		char *format = "\n";
		if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	if(!found){ //On a pas trouvé le fichier dans l'archive, message d'erreur
		char format[strlen(namefile) + 71];
		strcpy(format, "ls : impossible d'acceder a '");
		strcat(format, namefile);
		strcat(format, "': Aucun fichier ou dossier de ce type\n");
		if (write(STDERR_FILENO, format, strlen(format)) < strlen(format)) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	close(fd);
	return 0;
}

/*
* ls dans le cas ls arch.tar
*/
int print_tar(char *file, char *options){
	struct posix_header header;
	if(file[strlen(file)-1] == '/'){ //suppression des slashs en fin de noms
		file[strlen(file)-1] = '\0';
	}
	int fd = open(file, O_RDONLY);
	if(fd == -1){
	  perror("erreur d'ouverture de l'archive");
	  return -1;
	}
	int n = 0;
	int read_size = 0;
	tab_link = malloc(sizeof(int)*get_nb_dossier(fd));
	tab_nom = malloc(sizeof(char *)*get_nb_dossier(fd));
	get_link(fd);
	for(int i = 0; i < 2; i++){
		write(STDERR_FILENO, tab_nom[i], strlen(tab_nom[i]));
	}
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if(strcmp(header.name, "\0") == 0){
			break;
		}
		if ((get_profondeur(header.name) == 0)) {
			if(strcmp(options, "\0") == 0){ //Pas d'option, affichage simple
				show_simple_header_infos(&header, &read_size);
			}else{
				show_complete_header_infos(&header, &read_size);
			}
		}
		else { //ls -l
			get_header_size(&header, &read_size);
		}
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	if(strcmp(options, "\0") == 0){ //Si on a effectué un affichage simple, on doit rajouter un retour a la ligne, qui n'a pas lieu d'etre dans ls -l etant donné qu'on affiche ligne par ligne
		char *format = "\n";
		if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
	}
	close(fd);
	return 0;
}

int get_nb_dossier(int fd){
	lseek(fd, 0, SEEK_SET);
	struct posix_header header;
	int n = 0;
	int read_size = 0;
	int nb_dossier = 0;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if((header.name[strlen(header.name)-1] == '/') && header.name[strlen(header.name)] == '\0'){
			nb_dossier++;
		}
		else { //ls -l
			get_header_size(&header, &read_size);
		}
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return -1;
		}
	}
	char format[20];
	sprintf(format, "%d", nb_dossier);
	write(STDERR_FILENO, format, strlen(format));
	return nb_dossier;
}

void get_link(int fd){
	struct posix_header header;
	lseek(fd, 0, SEEK_SET);
	int n = 0;
	int read_size = 0;
	int i = 0;
	int j;
	while((n=read(fd, &header, BLOCKSIZE))>0){
		if((header.name[strlen(header.name)-1] == '/') && header.name[strlen(header.name)] == '\0'){
			tab_nom[i] = malloc(strlen(header.name)+1);
			strcpy(tab_nom[i], header.name);
			tab_link[i] = malloc(sizeof(int));
			tab_link[i] = 2;
			j = get_indice_pere(header.name);
			if(j == -1){
				perror("erreur dans le comptage du nombre de lien");
				return;
			}
			tab_link[j] = tab_link[j]+1;
			i++;
		}
		else { //ls -l
			get_header_size(&header, &read_size);
		}
		if(lseek(fd, BLOCKSIZE*read_size, SEEK_CUR) == -1){
			perror("erreur de lecture de l'archive");
			return;
		}
	}
	lseek(fd, 0, SEEK_SET);
}

int get_indice(char *nom){
	for(int i = 0; i < sizeof(tab_nom); i++){
		if(strcmp(nom, tab_nom[i]) == 0) return i;
	}
	return -1;
}
int get_indice_pere(char *nom){
	char *pos = strrchr(nom, '/');
	int spos = pos - nom;
	char pere[strlen(nom)];
	pere[spos] = '\0';
	for(int i = 0; i < sizeof(tab_nom); i++){
		if(strcmp(pere, tab_nom[i]) == 0) return i;
	}
	return -1;
}

void show_complete_header_infos(struct posix_header *header, int *read_size){
	int taille, mode, uid, gid, link;
	char name[strlen(header->name)+1];
	get_filename(header->name, name);
	char mode_str[10];
	char typeformat[2];
	long int mtime;
	sscanf(header->size, "%o", &taille);
	char taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1):6]; //Les tailles ne sont plus alignés au dessus de 6 chiffres
	sprintf(taille_str, "%o", taille);
	for(int i = nbdigit(taille); i < 6; i++){ //On complète la string avec des espaces afin d'avoir un alignement
		taille_str[i] = ' ';
	}
	taille_str[((nbdigit(taille)+1)>6)?(nbdigit(taille)+1)-1:5] = '\0';
	//int j = get_indice(header->name);
	//link = tab_link[j];
	//char link_str[nbdigit(link)+1];
	//sprintf(link_str, "%d", link);
	sscanf(header->mode, "%o", &mode);
	convert_mode(mode, mode_str);
	sscanf(header->uid, "%o", &uid);
	sscanf(header->gid, "%o", &gid);
	sscanf(header->mtime, "%lo", &mtime);
	char *pw_name = header->uname;
	char *gr_name = header->gname;
	char *date= ctime(&mtime);
	typeformat[0] = ((header->typeflag=='0')?'-':(header->typeflag=='5')?'d':'-');
	typeformat[1] = '\0';
	date[strlen(date) - 1] = '\0'; // ctime renvoit une string se terminant par \n ...

	*read_size = ((taille + 512-1)/512);
	char format[strlen(typeformat) + strlen(mode_str) + strlen(taille_str) + /*strlen(link_str) +*/ strlen(name) + strlen(date) + strlen(pw_name) + strlen(gr_name)+ 1]; //calcul de taille faux
	strcpy(format, typeformat);
	strcat(format, mode_str);
	strcat(format, " ");
	//strcat(format, link_str);
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
	char filename[strlen(header->name)+1];
	get_filename(header->name, filename);
	char filename_format[strlen(filename) + 2 + 1];
	strcpy(filename_format, filename);
	strcat(filename_format, "  ");
	if (write(STDOUT_FILENO, filename_format, strlen(filename_format)) < strlen(filename_format)) {
		perror("Erreur d'écriture dans le shell!");
		exit(EXIT_FAILURE);
	}
}

int contains_filename(char *haystack, char *needle){
	char cp[strlen(haystack) + 1];
	strcpy(cp, haystack);
	char *token = strtok(cp, "/");
	if(strcmp(needle, token) == 0){
		return 1;
	}
	while((token = strtok(NULL, "/")) != NULL){
		if(strcmp(needle, token) == 0){
			return 1;
		}
	}
	return 0;
}

int get_filename(char *name, char* namecp){
	int index = 0;
	int trailing_slash = ((name[strlen(name)-1] == '/')?1:0);
    for (int i = 0; i < strlen(name)-trailing_slash; i++) { //Cherche l'index du dernier /, ce qui se trouve après est le nom du fichier
        if (name[i] == '/') {
            index = i;
		}
	}
	if(index == 0){
		for (int i = 0; i < strlen(name)-index; i++) {
	        namecp[i] = name[index + i];
		}
		namecp[strlen(name)-index] = '\0';
	}else{
		for (int i = 0; i < strlen(name)-index - 1; i++) {
			namecp[i] = name[index + i + 1];
		}
		namecp[strlen(name)-index-1] = '\0';
	}
	return 0;
}

int get_profondeur(char *name){
	int profondeur = 0;
	int trailing_slash = ((name[strlen(name)-1] == '/')?1:0);
    for (int i = 0; i < strlen(name)-trailing_slash; i++) {
        if (name[i] == '/') {
			profondeur++;
		}
	}
	return profondeur;
}

void get_header_size(struct posix_header *header, int *read_size){
	int taille = 0;
	sscanf(header->size, "%o", &taille);
	*read_size = ((taille + 512-1)/512);
}

int contains_tar(char *file){
	return (strstr(file,".tar") != NULL);
}

int is_ext(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar(char *file){
	return is_ext(file, ".tar") || is_ext(file, ".tar/");
}

int is_options(char *options){
	return (options[0] == '-' && strlen(options) > 1);
}

int check_options(char *options){
	if((strcmp(options, "-l") == 0)|| (strcmp(options, "\0") == 0)){
		return 0;
	}else{
		char *message = "ls : option invalide ";
		char format[strlen(message) + strlen(options) + 1];
		strcpy(format, message);
		strcat(format, options);
		strcat(format, "\n");
		if (write(STDOUT_FILENO, format, strlen(format)) < strlen(format)) {
			perror("Erreur d'écriture dans le shell!");
			exit(EXIT_FAILURE);
		}
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
/*
*Convertis le mode (en octal) en une string rwxrwxrwx
*/
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
