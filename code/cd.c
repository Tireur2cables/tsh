#define _XOPEN_SOURCE
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

char *home = NULL;  // contient chemin du 1er rep
char *pwd = NULL;


int cd(int argc,char **argv) {

	path_initialisation();
	if (errorDetect(argc) < 0) return -1;

	int exist_or_not = 0;
	struct stat st;
	DIR *courant = opendir(pwd);

	if (courant == NULL) {
		char * error1= "erreur d'ouverture du repertoire\n";
		write(STDERR_FILENO,error1,strlen(error1));
		return -1;
	}



	if (argc == 2) {
		char *dir_argument = argv[1];
		struct dirent *d;

		while ((d = readdir(courant)) != NULL) {

				if (strcmp(d->d_name, dir_argument) == 0) {

					unsigned char type = d->d_type;

					if (isTAR(dir_argument, type) == 2) { // FIXME : isTar n'est pas bon
						printf("OK TAR\n");
						exist_or_not=1;
						if (actuPath(dir_argument) < 0) return -1;
						break;
					}

					if (stat(dir_argument, &st) == 0) {
						if (S_ISDIR(st.st_mode) != 0){
							exist_or_not=1;
							chdir(dir_argument);
							printf("OK REP\n");
							//printf("%s\n",getcwd(NULL,0));
							break;

						}else {

							exist_or_not=1;
							char * tmp = " n'est pas un répertoire";
							char * error3 = malloc(sizeof(char)*(strlen(dir_argument)+strlen(tmp)));
							strcpy(error3, dir_argument);
							strcat(error3, tmp);
							write(STDERR_FILENO, error3, strlen(error3));
							write(STDOUT_FILENO, "\n", 1);
							return -1;
						}

					}

				}

			} // fin while

			if (!exist_or_not) {
				char * error4 = "fichier ou répertoire non existant !\n";
				write(STDERR_FILENO, error4, strlen(error4));
				write(STDOUT_FILENO,"\n",1);
				return -1;
			}

		}

	closedir(courant);
	return 0;
}







int actuPath(char * new) {
	char * newpath=malloc(sizeof(char)*strlen(pwd)+strlen(new)+1);
	char * to_add=malloc(sizeof(char)*strlen(new));

	strcpy(to_add,new);
	strcpy(newpath,pwd);
	strcat(newpath,"/");
	strcat(newpath,to_add);

	pwd=newpath;

	char * newENV=malloc(sizeof(char)*(strlen(pwd)+4));
	strcpy(newENV,"PWD=");
	strcat(newENV,pwd);
	int c=putenv(newENV);

	if(c) {
		errno = ENOMEM;
		perror("changement de variable impossible");
		return -1;
	}
	return 0;
}

int isTAR(char * dirTAR,unsigned char type) {
	int c=0;
	if(strstr(dirTAR,".tar")) c++;
	if(type==0) c++;
	return c;
}

void path_initialisation() {
	home=malloc(sizeof(char)*PATH_MAX);
	pwd=getcwd(home,PATH_MAX);
}

int errorDetect(int argc) {
	if(argc==0 || argc==1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		return -1;
	}

	if(argc>2) {
		errno = EINVAL;
		perror("trop d'arguments !");
		return -1;
	}

	return 0;
}
