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

char * courant = NULL;
int error_or_not = 1;

int main(int argc,char **argv) {
	
	char *oldpwd = getcwd(NULL, 0);
	if (setenv("TWD", oldpwd, 1) < 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);

	courant = getcwd(NULL,0);
	assert(courant);
	
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
		
		chemin=argv[i];
		char CheminTMP[strlen(chemin)+1];
		strcpy(CheminTMP,chemin);
		const char * separator = "/";
		char * part=strtok(CheminTMP,separator);
		char * lastRep = NULL;
		int found;
		int passage=1;  // pour passer a l'itération suivante dans for si erreur dans 1 chemin
		
		
		while(part!=NULL) {
		
		if(passage<1) break;
		
		found=0;
		
		dir_courant=opendir(courant);
		if(openDetectError(dir_courant)!=0) return -1;
		
			while((d=readdir(dir_courant))!=NULL) {
			
				if(strcmp(part,d->d_name)==0) {
				
				found=1;
				
					if (stat(part, &s) < 0) {
						perror("Erreur de stat!\n");
						return -1;
					} else {
				
						if(S_ISDIR(s.st_mode)==0) NotDirectory(chemin);
													
					}
				
				} 
		
		
			} // fin while2
		
		if(fileFound(found)!=0)  { passage=0; error_or_not = 0; };
		part=strtok(NULL,separator);
		if(part!=NULL) lastRep=part;
		actuDir(part);
		
		} // fin while1
		
		
		//if(lastRep!=NULL && error_or_not) DeletingDirectory(lastRep);
		
		error_or_not = 1 ;
		
	} // fin for
	

	
	closedir(dir_courant);
	
	return 0;
	
}

int actuDir(char * part) {
	
	printf("%s\n",getenv("TWD"));
	
	return 0;
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
			
	error_or_not = 0;
}


int fileFound(int f) {
	if(f==0) {
		char *error = "suppression impossible , fichier ou repertoire non existant !\n";
			if(write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell!");
				}
		return -1;
	}
	
	return 0;
}

int openDetectError(DIR * d) {
	
	if(d==NULL) {
		char *error = "erreur d'ouverture du repertoire ";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell");
			} 	
			return -1;
	}
	
	return 0;
	
}













