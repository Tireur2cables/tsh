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

char * courant =NULL;

int rmdir_func(int argc,char **argv) {

	if (errorDetect(argc)) return -1;
	
	//////////////////////////////////////////////////
	char *oldpwd = getcwd(NULL, 0);
	if (setenv("TWD", oldpwd, 1) < 0) {
		perror("Erreur de création de TWD!");
		exit(EXIT_FAILURE);
	}
	free(oldpwd);
	///////////////////////////////////////////////////
	
	 courant = getcwd(NULL,0);
	
	char dir_argument[strlen(argv[1])+1];
	strcpy(dir_argument,argv[1]);
	
	char * chemin = strtok(dir_argument,"/");
	
	struct dirent *d;
	struct stat st;
	DIR * dir_courant;
	
	
	for(int i=0;i<argc;i++) {

	while(chemin!=NULL) {       // while 2
	
	printf("%s\n",courant);
	printf("%s\n",chemin);
	
	dir_courant = opendir(courant);
	if(errorOPEN(dir_courant,chemin)!=0) return -1;
	
	int found = 1;
	
	while((d=readdir(dir_courant))!=NULL) {		// while 1
		
		if(strcmp(chemin,d->d_name)==0) {
			
			found = 0;
			
		if (stat(chemin, &st) < 0) {
			perror("Erreur de stat!\n");
			return -1;
		}
		
		if (S_ISDIR(st.st_mode) != 0) { // dossier
			printf("REPERTOIRE\n");
			chdir(chemin);
			break;
		} else {
			char *tmp = " n'est pas un répertoire!\n";
			char error[strlen(chemin)+strlen(tmp)+1];
			strcpy(error, dir_argument);
			strcat(error, tmp);
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell!");
						}
			return -1;
					}
				}
				
		
			} // fin while1
		
			if (found) {
			char *error = "suppression impossible ! Fichier ou repertoire non existant !\n";
			if (write(STDERR_FILENO, error, strlen(error)) < strlen(error)) {
				perror("Erreur d'écriture dans le shell!");
				}
				return -1;
			}
			
			chemin=strtok(NULL,"/");
			courant=getcwd(NULL,0);
			
	
		} // fin while2
	
	} 
	
	closedir(dir_courant);

	return 0;
	
	
}



int errorOPEN(DIR * c,char * chemin) {
	if (c == NULL) {
			char *error1 = "erreur d'ouverture du repertoire ";
			if (write(STDERR_FILENO, error1, strlen(error1)) < strlen(error1)) {
				perror("Erreur d'écriture dans le shell");
			} 
			char error2[strlen(chemin)+7];
			strcpy(error2,"pour ");
			strcat(error2,chemin);
			write(STDERR_FILENO,error2,strlen(error2));
			write(STDERR_FILENO,"\n",2);
			return -1;
		}
		
		return 0;
}

int errorDetect(int argc) {
	
	if(argc <= 1) {
		errno = EINVAL;
		perror("Arguments manquants");
		return -1;
	}
	


	return 0;
}


