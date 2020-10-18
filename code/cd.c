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
#include <limits.h>
#include "tar.h"
#include "cd.h"


char * home=NULL;
char * buff=NULL; //sera conservé (répertoire a partir duquel on lance la commande)


int cd(int argc,char **argv) {
	
	path_initialisation();     
	
	DIR*courant=opendir(home); 
	assert(courant && home);
	
	if(argc==0 || argc==1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		exit(EXIT_FAILURE);
	}	else {
			char * dir_argument;
			if(argc==2) {
				dir_argument=argv[1];
				while(1) {
					struct dirent *d=readdir(courant);  
					if(d==NULL) break;
					if(strcmp(d->d_name,dir_argument)==0) {  
						if(isTAR(d->d_name)!=NULL) {      
							
							actuPath(dir_argument);
							chdir(dir_argument);
						
						
							break;
					} else { // autres cas : repertoire normal ; .. ; . 
						
					} 
				}
				
			} // fin while
				
				
		}
			
	}
	
	closedir(courant);
	return 0;
}

void actuPath(char * new) {
	
	char * newpath=malloc(sizeof(char)*strlen(home)+strlen(new)+1);
	char * to_add=malloc(sizeof(char)*strlen(new));

	strcpy(to_add,new);
	strcpy(newpath,home);
	strcat(newpath,"/");
	strcat(newpath,to_add);
	
	home=newpath;
}

char * isTAR(char * dirTAR) {
	return strstr(dirTAR,".tar");
}

void path_initialisation() {
	buff=malloc(sizeof(char)*PATH_MAX);
	home=getcwd(buff,PATH_MAX);
}
