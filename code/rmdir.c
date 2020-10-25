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

char * InitialRep = NULL;

//TODO : inclure tar dans chemin (isTAR..)

int main(int argc,char **argv) {
	
	InitialRep = getcwd(NULL,0);
	
	char *oldpwd = getcwd(NULL, 0);
	if (setenv("TWD", oldpwd, 1) < 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);

	
	if(argc <= 1 ) {
		errno = EINVAL;
		perror("manque d'arguments");
		return -1;
	}
	
	
	char * chemin;
	struct stat s;
	struct dirent * d;
	DIR * dir_courant;
	
	for(int i=1;i<argc;i++) {
		
		int error_or_not = 1;
		chemin=argv[i];
		char CheminTMP[strlen(chemin)+1];
		strcpy(CheminTMP,chemin);
		const char * separator = "/";
		char * part=strtok(CheminTMP,separator);
		char * lastRep = NULL;
		int found;
		int passage=1;  // pour passer a l'itération suivante dans for si erreur dans 1 chemin
		if (setenv("TWD", InitialRep, 1) < 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
		}
		
		while(part!=NULL) {
		
		if(passage<1) break;
		
		found=0;

		dir_courant=opendir(getenv("TWD"));
		if(openDetectError(dir_courant)!=0) return -1;
		
			while((d=readdir(dir_courant))!=NULL) {
				
				if(strcmp(part,d->d_name)==0) {
		
				found=1;
				
					if (stat(part, &s) < 0) {
						perror("erreur de stat");
						exit(EXIT_FAILURE);
					} else {
						if(S_ISDIR(s.st_mode)==0) {
						
							NotDirectory(chemin);
							error_or_not=0;
							break;
							}
						
						}
													
					
					} 
		
		
			} // fin while2
		
		if(fileFound(found)!=0)  { passage=0; error_or_not = 0; };
		if(part!=NULL)  { lastRep=part; actuDir(part); }
		part=strtok(NULL,separator);
		
		
		
		} // fin while1
		
		if(lastRep!=NULL && error_or_not!=0) {
			
			if(DeletingDirectory(lastRep,chemin)<0) {
				
			}
			
		}
		
		error_or_not = 1 ;
		
	} // fin for
	

	
	closedir(dir_courant);
	
	return 0;
	
}

int DeletingDirectory(char * rep , char * fullchemin) {
	
	DIR * c=opendir(getenv("TWD")); // normalement twd se finit par rep içi
	assert(c);
	struct dirent * d;
	int nbFile=0;
	
	while((d=readdir(c))!=NULL) {
		
		if(nbFile>2) break;
		nbFile++;
		
	}

	if(nbFile<3) {
		
		if(rmdir(rep)<0) {
			perror("erreur de suppression du répertoire");
			return -1;
		} 
		
	} else {
		
		char * error1 = "suppresion impossible , le répertoire ";
		char * error2 = " n'est pas vide";
		char * error3 = malloc(sizeof(char)*(strlen(error1)+strlen(error2)+strlen(fullchemin)));
		strcpy(error3,error1);
		strcat(error3,fullchemin);
		strcat(error3,error2);
		if (write(STDERR_FILENO, error3, strlen(error3)) < strlen(error3)) {
				perror("Erreur d'écriture dans le shell!");
						}
		write(STDERR_FILENO,"\n",2);
		return -1;
	}
	
	return 0;
	
}


void actuDir(char * part) {
	
	char * newpath = malloc(sizeof(char)*(strlen(getenv("TWD"))+1+strlen(part)));
	strcpy(newpath,getenv("TWD"));
	strcat(newpath,"/");
	strcat(newpath,part);
	
	if (setenv("TWD", newpath , 1) < 0) {
		perror("Erreur de modification de TWD!");
		exit(EXIT_FAILURE);
	}
	
	
	
}

void NotDirectory(char * p ) {
	
	char * error1 = "suppresion impossible : ";
	char * error2 = " n'est pas un répertoire!\n";
	char error3[strlen(p)+strlen(error1)+strlen(error2)+1];
	strcpy(error3 , error1);
	strcat(error3 , p);
	strcat(error3 , error2);
	
	if (write(STDERR_FILENO, error3, strlen(error3)) < strlen(error3)) {
				perror("Erreur d'écriture dans le shell!");
						}
			
}


int fileFound(int f) {
	if(f==0) {
		char *error = "suppression impossible , fichier ou repertoire non existant !\n";
			if(write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell!");
				}
				write(STDERR_FILENO,"\n",2);
		return -1;
	}
	
	return 0;
}

int openDetectError(DIR * d) {
	
	if(d==NULL) {
		char *error = "erreur d'ouverture du repertoire";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			} 	
			write(STDERR_FILENO,"\n",2);
			return -1;
	}
	
	return 0;
	
}













