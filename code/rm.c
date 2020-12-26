#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tar.h"
#include "rm.h"

int parcoursChemin(char * , char * , int );
int detectError(int );
int isAccessibleFrom(char * , char * , int);
int is_ext(char *, char *);
int is_tar(char *);
int dir_or_file(char * , char * , char *);
int delete_dir(char *);


int rm_func(int argc , char ** argv) {
	
	if (detectError(argc)) return -1;
	
	int withOption = 0;
	
	for(int i=1;i<argc;i++) 
		if(strcmp(argv[i],"-r")==0 || strcmp(argv[i],"-R")==0 || strcmp(argv[i],"--recursive")==0) withOption++;
	
	if (withOption==argc-1) { // si que des options
		
		errno=EINVAL;
		perror("missing operand");
		return -1;
		
	}
	
	for(int i=1;i<argc;i++) {
		
		if(strcmp(argv[i],"-r")!=0 && strcmp(argv[i],"-R")!=0 && strcmp(argv[i],"--recursive")!=0) {
		
		char * pwd = getcwd(NULL,0);
		char chemin[strlen(argv[i]+1)];
		strcpy(chemin,argv[i]);
		
		parcoursChemin(chemin,pwd,withOption);
		
		}
		
	}
	
	
	return 0;
}


int parcoursChemin(char * chemin, char * pwd , int option) {
	
	char *saveptr;
	char *doss;
	char * cpydoss=NULL;
	char * oldpwd=NULL;
	char *currentpwd = malloc(strlen(pwd) + 1);
	strcpy(currentpwd, pwd);
	
	while((doss=strtok_r(chemin,"/",&saveptr))!=NULL) {
		
		if(doss!=NULL) {
			cpydoss = realloc(cpydoss,strlen(doss)+1);
			strcpy(cpydoss,doss);
		}
		
		oldpwd = realloc(oldpwd,strlen(currentpwd)+1);
		strcpy(oldpwd,currentpwd);
		
		
		if(isAccessibleFrom(doss,currentpwd,option) > 0) {
			
		if(strstr(doss,".tar")!=NULL && (is_tar(doss)>0) ) {  // tar dans chemin
			
			
				
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
	
	char resdoss[strlen(cpydoss+1)];
	strcpy(resdoss,cpydoss);

	dir_or_file(resdoss,oldpwd,res);
	
	free(cpydoss);
	free(oldpwd);
	
	return 0;
	
	
}

int isAccessibleFrom(char * doss , char * dir , int option) {
	
	DIR * courant = opendir(dir);
	
	if (courant == NULL) {
		char *error_debut = "rm : Erreur! Impossible d'ouvrir le répertoire ";
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

				if (S_ISDIR(st.st_mode) && option<=0) { //directory without option
					char *deb  = "rm : ";
					char *end = " is a directory !\n";
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
		char *deb  = "rm : ";
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
	
int dir_or_file(char * doss , char * oldpwd , char * pwd ) {
	
	DIR * dir = opendir(oldpwd);
	
	if (dir == NULL) {
		char *error_debut = "rm : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + 1];
		strcpy(error, error_debut);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	
	struct dirent * d;
	struct stat st;
	
	while((d=readdir(dir))!=NULL) {
		
		if(strcmp(d->d_name,doss)==0) {
			
		if (stat(pwd, &st) < 0) {
			perror("Erreur de stat!");
			return -1;
		}
		
		if(S_ISDIR(st.st_mode)) {
			
			delete_dir(pwd);
			
		} else {
			
			remove(pwd);
			
			}
		
		}
		
	}
	
	return 0;
	
}

int delete_dir(char * pwd) {
	
	DIR * dir = opendir(pwd);
	
	if (dir == NULL) {
		char *error_debut = "rm : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + 1];
		strcpy(error, error_debut);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}
	
	struct dirent * d;
	struct stat st;
	
	while((d=readdir(dir))!=NULL) {
	
	
	 if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
	 	 
	  char newpwd[strlen(pwd)+1+strlen(d->d_name)+1+1];
	  strcpy(newpwd,pwd);
	  strcat(newpwd,"/");
	  strcat(newpwd,d->d_name);
		
	 if (stat(newpwd, &st) < 0) {
			perror("Erreur de stat!");
			return -1;
		}
	
	 if(S_ISDIR(st.st_mode)) {
            delete_dir(newpwd);
        } else {
           remove(newpwd);
		}
		
	}
	
	closedir(dir);
		
		if(rmdir(pwd)<0) {
			perror("erreur de suppression du répertoire");
			return -1;
		}
	
	return 0;
}


int detectError(int argc) {
	
	if(argc==0) {
		errno=EINVAL;
		perror("program error");
		exit(EXIT_FAILURE);		
		return -1;
	}
	
	if(argc==1) {
		errno=EINVAL;
		perror("missing operand");
		return -1;
	}
	
	return 0;
	
}

int is_ext(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar(char *file){
	return is_ext(file, ".tar") || is_ext(file, ".tar/");
}

















