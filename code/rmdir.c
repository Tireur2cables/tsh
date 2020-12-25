#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tar.h"
#include "rmdir.h"

int parcoursChemin(char *, char *);
int isAccessibleFrom(char *, char *);
int deleteDir(char *);
int isDirEmpty(char *);
int parcoursCheminTar(char *, char *, char *);
int is_ext(char *, char *);
int is_tar(char *);
int isSameDir(char *, char *);

//TODO : gérer suppression dossier dans tar (vérification si dossier dans tar déjà faite)

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
		
		char chemin[strlen(argv[i])+1];
		strcpy(chemin,argv[i]);                       
		
		if(chemin[0]!='/') {
			
			char * pwd = getcwd(NULL,0);
			parcoursChemin(chemin,pwd);
			
		} else {
		
		
		  parcoursChemin(chemin,"/"); 
		 
		}
		 
	}
	
	return 0;
	
	
}


int parcoursChemin(char * chemin, char * pwd) {
	
	char *saveptr;
	char *doss;
	char *currentpwd = malloc(strlen(pwd) + 1);
	strcpy(currentpwd, pwd);
	
	while((doss=strtok_r(chemin,"/",&saveptr))!=NULL) {
		
		if(isAccessibleFrom(doss,currentpwd) > 0) {
			
		if(strstr(doss,".tar")!=NULL && (is_tar(doss)>0) ) {  // tar dans chemin
			
			char newcurr[strlen(currentpwd) + 1];
				strcpy(newcurr, currentpwd);
				free(currentpwd);
				return parcoursCheminTar(newcurr,doss,saveptr);
		}
		
		//maj pwd wow
		int currentlen = strlen(currentpwd);
		if (currentlen != 1) currentlen++;
		char newcurr[currentlen + strlen(doss) + 1];
		strcpy(newcurr, currentpwd);
		if (currentlen != 1) strcat(newcurr, "/");
		strcat(newcurr, doss);

		currentpwd = realloc(currentpwd, strlen(newcurr)+1);
		strcpy(currentpwd, newcurr);
		
		chemin=saveptr;
		
		} else {
		free(currentpwd);
		return -1;
		}
	
	}
	
	char res[strlen(currentpwd+1)];
	strcpy(res,currentpwd);
	
	deleteDir(res);
	
	return 0;
	
	
}


int isAccessibleFrom(char * doss , char * dir) {
	
	DIR * courant = opendir(dir);
	
	if (courant == NULL) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + strlen(dir) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, dir);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	
	int found=0;
	struct dirent * d;
	struct stat st;
	
	while((d=readdir(courant))!=NULL) {
		
		if(strcmp(d->d_name,doss)==0) {
			found=1;
			if (strstr(doss, ".tar") == NULL) { //not tar so should be directory
				char absolutedoss[strlen(dir) + 1 + strlen(doss) + 1];
				strcpy(absolutedoss, dir);
				strcat(absolutedoss, "/");
				strcat(absolutedoss, doss);

				if (stat(absolutedoss, &st) < 0) {
					perror("Erreur de stat!");
					return -1;
				}

				if (!S_ISDIR(st.st_mode)) { //not directory
					char *deb  = "rmdir : ";
					char *end = " n'est pas un répertoire ou une archive !\n";
					char error[strlen(deb) + strlen(doss) + strlen(end) + 1];
					strcpy(error, deb);
					strcat(error, doss);
					strcat(error, end);
					int errorlen = strlen(error);
					if (write(STDERR_FILENO, error, errorlen) < errorlen)
						perror("Erreur d'écriture dans le shell!");
					return -1;
				}
			}
			break;
		}
	}
	
	closedir(courant);
	if (!found) {
		char *deb  = "rmdir : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(doss) + strlen(end) + 1];
		strcpy(error, deb);
		//strcat(error, dir);
		//strcat(error, "/");
		strcat(error, doss);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	
	return found;
}
	
	
int parcoursCheminTar(char * pwd , char * twd , char *rest) {
	
	char twd_copy[strlen(twd)+1];
	strcpy(twd_copy, twd);
	char *tar = strtok(twd_copy, "/");
	char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
	strcpy(absolutetar, pwd);
	strcat(absolutetar, "/");
	strcat(absolutetar, tar);

	char chemin[strlen(twd) - strlen(tar) + strlen(rest) + 1];
	if (strlen(twd) - strlen(tar) > 0) strcpy(chemin, &twd[strlen(tar)+1]);
	else strcpy(chemin, &twd[strlen(tar)]);
	if (strlen(chemin) != 0) strcat(chemin, "/");
	strcat(chemin, rest);
	
	int fd = open(absolutetar, O_RDONLY);
	
	if (fd == -1) {
		char *error_debut = "cd : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	
	int found = 0;
	
	if (chemin != NULL && strlen(chemin) != 0) {
		struct posix_header header;
		while (!found) {
			if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

			char nom[strlen(header.name)+1];
			strcpy(nom, header.name);
			if (nom[strlen(nom)-1] == '/' && isSameDir(chemin, nom)) found = 1;
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
	}else found = 1;

	close(fd);
	if (!found) {
		char *deb  = "rmdir : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, absolutetar);
		strcat(error, "/");
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}else if (found == 1) {
		char res[strlen(absolutetar) + 1 + strlen(chemin) + 1];
		strcpy(res, absolutetar);
		if (chemin != NULL && strlen(chemin) != 0) {
			strcat(res, "/");
			strcat(res, chemin);
		}
		if (res[strlen(res)-1] == '/') res[strlen(res)-1] = '\0';
		
		printf("%s\n",absolutetar);
		
	}else {
		char *deb  = "rmdir : ";
		char *end = " n'est pas un répertoire!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, absolutetar);
		strcat(error, "/");
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}
	
	return (found == 1)? 0 : -1;
	
	
	
}
	
int deleteDir(char * pwd) {
	
	
	if(isDirEmpty(pwd)<0) {  // pas vide
		
		errno = ENOTEMPTY;
		char * error1 = "rmdir : le répertoire ";
		char * error2 = " n'est pas vide !";
		char error3[strlen(error1) + 1 + strlen(pwd) + 1 + strlen(error2) + 1]; 
		strcpy(error3,error1);
		strcat(error3,pwd);
		strcat(error3,error2);
		perror(error3);
		return -1;
	
	} else { 

	if(rmdir(pwd)<0) {
			perror("erreur de suppression du répertoire");
			return -1;
		} 
	
	}		
	
	return 0;
	
}

int is_ext(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar(char *file){
	return is_ext(file, ".tar") || is_ext(file, ".tar/");
}


int isDirEmpty(char * pwd ) {
	
	DIR * dir = opendir(pwd);
	struct dirent * d;
	int nbFile=0;  // cb de fichiers dans le rep
	
	if (dir == NULL) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + strlen(pwd) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, pwd);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	
	while((d=readdir(dir))!=NULL) {
		nbFile++;
	}
	
	if(nbFile<3) return 0;
	return -1;
	
}

int isSameDir(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}





























