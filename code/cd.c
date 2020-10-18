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
#include <linux/limits.h>
#include "tar.h"
#include "cd.h"

char *home = NULL;
char *pwd = NULL;

int cd(int argc,char **argv) {

	path_initialisation();

	DIR*courant=opendir(pwd);
	assert(courant && pwd);

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

	char * newpath=malloc(sizeof(char)*strlen(pwd)+strlen(new)+1);
	char * to_add=malloc(sizeof(char)*strlen(new));

	strcpy(to_add,new);
	strcpy(newpath,pwd);
	strcat(newpath,"/");
	strcat(newpath,to_add);

	pwd=newpath;
}

char * isTAR(char * dirTAR) {
	return strstr(dirTAR,".tar");
}

void path_initialisation() {
	home=malloc(sizeof(char)*PATH_MAX);
	pwd=getcwd(home,PATH_MAX);
}
