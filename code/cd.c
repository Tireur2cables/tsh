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

/*
TODO LIST :
- Modifier la variable TWD au lieu d'utiliser pwd
- Modifier aussi la variable OLDTWD pour la commande 'cd -' qui revient en arriève
- Prendre en compte les dossiers meme avec '/' à la fin
- Prendre en compte les chemins
*/

int cd(int argc,char **argv) {

	if (errorDetect(argc)) return -1;
	path_initialisation();

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


					if (isTAR(dir_argument)== 0) {
						printf("OK TAR\n");
						exist_or_not=1;
						if (actuPath(dir_argument) < 0) return -1;
						break;
					}

					if (stat(dir_argument, &st) == 0) {
						if (S_ISDIR(st.st_mode) != 0){
							exist_or_not=1;
							chdir(dir_argument);
							pwd=getcwd(NULL,0);
							printf("OK REP\n");
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

int isTAR(char * dirTAR) {
	char * ext = ".tar";
	return strcmp(&dirTAR[strlen(dirTAR)-strlen(ext)],ext);
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
		errno = E2BIG;
		perror("trop d'arguments !");
		return -1;
	}

	return 0;
}
