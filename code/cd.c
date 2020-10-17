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
#include "cd.h"
//#include <linux/limits.h>

//char * home=NULL;
//char * buff=NULL;



int cd(int argc,char **argv) {
	
	/*path_initialisation();     
	enlevez les coms quand vous compilez et enlevez le char home d'après du coup ça 
	marche juste pas pour moi pour l'instant */
	
	char*home=getcwd(NULL,0); //pour moi
	
	DIR*courant=opendir(home); 
	assert(courant && home);
	
	if(argc==0 || argc==1) printf("Aucun répertoire indiqué");
		else {
			char *dirTAR;
			if(argc==2) {
				dirTAR=argv[1];
				while(1) {
					struct dirent *d=readdir(courant);  
					if(d==NULL) break;
					if(strcmp(d->d_name,dirTAR)==0) {  
						if(isTAR(dirTAR)!=NULL) {
							
					} else {
						
					} // TODO :autres cas : repertoires non tar
				}
			}
				
				
		}
			
	}
	
	
}


char * isTAR(char * dirTAR) {
	return strstr(dirTAR,".tar");
}

void path_initialisation() {
	//buff=malloc(sizeof(char)*PATH_MAX);
	//home=getcwd(buff,PATH_MAX);
}
