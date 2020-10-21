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
	errorDetect(argc);
	
	int exist_or_not=0;
	struct stat st;
	DIR*courant=opendir(pwd);
	
	if(courant==NULL) {
		char * error1= "erreur d'ouverture du repertoire\n";
		write(2,error1,strlen(error1));
		exit(EXIT_FAILURE);
	}
	
	
			
	if(argc==2) {
		char * dir_argument=argv[1];
		struct dirent *d;
				
		while((d=readdir(courant))!=NULL) {

				if(d==NULL) {
					char * error2="erreur de lecture du repertoire\n";
					write(2,error2,strlen(error2));
					exit(EXIT_FAILURE);
				}
					
					
				if(strcmp(d->d_name,dir_argument)==0) {  
				
					unsigned char type=d->d_type;

					if(isTAR(dir_argument,type)==2) { 
						printf("OK TAR\n");
						exist_or_not=1;
						actuPath(dir_argument);
						break;
					}  
						
					if(stat(dir_argument,&st)==0) {			
						if(S_ISDIR(st.st_mode)!=0){
							exist_or_not=1;
							chdir(dir_argument);
							printf("OK REP\n");
							//printf("%s\n",getcwd(NULL,0));
							break;
							
						} else {
							
							exist_or_not=1;
							char * tmp = " n'est pas un répertoire";
							char * error3=malloc(sizeof(char)*(strlen(dir_argument)+strlen(tmp)));
							strcpy(error3,dir_argument);
							strcat(error3,tmp);
							write(2,error3,strlen(error3));
							write(1,"\n",2);
							exit(EXIT_FAILURE);
						}
							
					}
					
				}
				
			} // fin while
			
				if(!exist_or_not) {
					char * error4= "fichier ou répertoire non existant !\n";
					write(2,error4,strlen(error4));
					write(1,"\n",2);
					exit(EXIT_FAILURE);
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

void errorDetect(int argc) {

	if(argc==0 || argc==1) {
		errno = EINVAL;
		perror("Aucun répertoire indiqué");
		exit(EXIT_FAILURE);
	}
	
	if(argc>2) {
		errno = EINVAL;
		perror("trop d'arguments !");
		exit(EXIT_FAILURE);	
	}
	
	
	
}
