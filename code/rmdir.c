#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include "tar.h"
#include "rmdir.h"

int parcoursChemin(char *, char *);
int isAccessibleFrom(char *, char *);
int deleteDir(char *);
int isDirEmpty(char *);
int parcoursCheminTar(char *, char *, char *);

//TODO : gérer les tar dans les chemins et gérer le chemin commençant par "/"

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
	
	char *oldpwd = getcwd(NULL, 0);
	if (setenv("TWD", oldpwd, 1) < 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);
	
	for(int i=1;i<argc;i++) {
		
		char chemin[strlen(argv[i])+1];
		strcpy(chemin,argv[i]);                       
		
		if(chemin[0]!='/') {
			char * pwd = getcwd(NULL,0);
			parcoursChemin(chemin,pwd);
			
		} else {
			
			 // parcoursChemin(chemin,"/");
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
			
		if(strstr(doss,".tar")!=NULL) {  // tar dans chemin
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
					char *end = " n'est pas un répertoire!\n";
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
	
	
int parcoursCheminTar(char * pwd , char * dos , char *rest) {
	
	
	
	
	return 0;
}
	
int deleteDir(char * pwd) {
	
	
	if(isDirEmpty(pwd)<0) {  // pas vide
		
		errno = ENOTEMPTY;
		char * error1 = "le répertoire ";
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






























