#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tar.h"
#include "rm.h"

int parcoursCheminTar_rm(char * , char * , char * , int );
int parcoursChemin_rm(char * , char * , int );
int detectError(int );
int isSameDir_rm(char *, char *);
int isAccessibleFrom_rm(char * , char * , int);
int is_ext_rm(char *, char *);
int is_tar_rm(char *);
int dir_or_file(char * , char * , char * , int );
int delete_dir(char *);
int delete_tar(char * , char * , char * , int);


int main(int argc , char ** argv) {
	
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
		
		if(chemin[0]!='/') parcoursChemin_rm(chemin,pwd,withOption);
		else parcoursChemin_rm(chemin,"/",withOption);
		
		}
		
	}
	
	
	return 0;
}


int parcoursChemin_rm(char * chemin, char * pwd , int option) {
	
	char *saveptr;
	char *doss;
	char * cpydoss=NULL;
	char * oldpwd=NULL;
	char *currentpwd = malloc(strlen(pwd) + 1);
	strcpy(currentpwd, pwd);
	
	while((doss=strtok_r(chemin,"/",&saveptr))!=NULL) {
		
		if(doss!=NULL) {
			cpydoss = realloc(cpydoss,strlen(doss)+1);
			assert(cpydoss);
			strcpy(cpydoss,doss);
		}
		
		oldpwd = realloc(oldpwd,strlen(currentpwd)+1);
		assert(oldpwd);
		strcpy(oldpwd,currentpwd);
		
		
		if(isAccessibleFrom_rm(doss,currentpwd,option) > 0) {
			
		if(strstr(doss,".tar")!=NULL && (is_tar_rm(doss)>0) ) {  // tar dans chemin
			
			return parcoursCheminTar_rm(doss,oldpwd,saveptr,option);
				
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

	dir_or_file(resdoss,oldpwd,res,option);
	
	free(cpydoss);
	free(oldpwd);
	
	return 0;
	
	
}

int isAccessibleFrom_rm(char * doss , char * dir , int option) {
	
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
		
		if(strcmp(d->d_name,doss)==0) found=1;

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
	
	
int parcoursCheminTar_rm(char * doss , char * oldpwd , char * rest , int option) {
	
	char cpydoss[strlen(doss)+1];
	strcpy(cpydoss, doss);
	char *tar = strtok(cpydoss, "/");
	char absolutetar[strlen(oldpwd) + 1 + strlen(tar) + 1];
	strcpy(absolutetar, oldpwd);
	strcat(absolutetar, "/");
	strcat(absolutetar, tar);

	char chemin[strlen(doss) - strlen(tar) + strlen(rest) + 1];
	if (strlen(doss) - strlen(tar) > 0) strcpy(chemin, &doss[strlen(tar)+1]);
	else strcpy(chemin, &doss[strlen(tar)]);
	if (strlen(chemin) != 0) strcat(chemin, "/");
	strcat(chemin, rest);
	
	int fd = open(absolutetar, O_RDONLY);

	if (fd == -1) {
		char *error_debut = "rm : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	int found = 0;

	if (chemin != NULL && strlen(chemin) != 0) {
		struct posix_header header;
		while (!found) {
			if (read(fd, &header, BLOCKSIZE) < BLOCKSIZE) break;

			char nom[strlen(header.name)+1];
			strcpy(nom, header.name);
			if (nom[strlen(nom)-1] == '/' && isSameDir_rm(chemin, nom)) found = 1;
			else {
				if (strcmp(nom, chemin) == 0) found = -1;
				if (strcmp(nom, "") == 0) break;
				unsigned int taille;
				sscanf(header.size, "%o", &taille);
				taille = (taille + BLOCKSIZE - 1) >> BLOCKBITS;
				taille *= BLOCKSIZE;
				if (lseek(fd, (off_t) taille, SEEK_CUR) == -1) break;
			}
		}
	}else found = 1;

	close(fd);
	if (!found) {
		char *deb  = "rm : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, absolutetar);
		strcat(error, "/");
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	} else {
		char res[strlen(absolutetar) + 1 + strlen(chemin) + 1];
		strcpy(res, absolutetar);
		if (chemin != NULL && strlen(chemin) != 0) {
			strcat(res, "/");
			strcat(res, chemin);
		}
		if (res[strlen(res)-1] == '/') res[strlen(res)-1] = '\0';
		
		//printf("%s\n",absolutetar); // chemin avec tar
		//printf("%s\n",chemin); // après le tar
		//printf("%s\n",res);  // concat
	
		if((int)strlen(chemin)==0 ) {  // si rien après tar
			
			if(doss[strlen(doss)-1]=='/') {} //  cmt savoir avec strtok ?
		
			printf("suppression du tar vide...\n");
			/*if(remove(res)!=0) {
				perror("erreur de supression du fichier");
				return -1;
				}*/
			} else {
				
				delete_tar(absolutetar,chemin,res,option);
				
			}
	
	}
	
	return (found == 1)? 0 : -1;

}


int delete_tar(char * absolutetar , char * chemin , char * res , int option) {
	
	
	
	
	
	
	return 0;
}

int dir_or_file(char * doss , char * oldpwd , char * concat , int option ) {
	
	//printf("%s\n",doss);
	//printf("%s\n",oldpwd);
	//printf("%s\n",concat);
	
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
			
		if (stat(concat, &st) < 0) {
			perror("Erreur de stat!");
			return -1;
		}
		
		if(S_ISDIR(st.st_mode) && option>0) {
			
			printf("suppression du répertoire...\n");
			//delete_dir(concat);
			
		} else {
			if(S_ISDIR(st.st_mode) && option<=0) {
				
				char *deb  = "rm : ";
				char *end = " est un répertoire !\n";
				char error[strlen(deb) + strlen(doss) + strlen(end) + 1];
				strcpy(error, deb);
				strcat(error, doss);
				strcat(error, end);
				int errorlen = strlen(error);
				if (write(STDERR_FILENO, error, errorlen) < errorlen)
					perror("Erreur d'écriture dans le shell!");
				return -1;
				
			} else {
			
				printf("suppression du fichier...\n");
				/* if(remove(concat)!=0) {
					perror("erreur de suppression du fichier");
					return -1;
					} */
			
				}
		
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
           if(remove(newpwd)!=0) {
			perror("erreur de supression du fichier");
			return -1;
		   }
		}
		
	}
	
	closedir(dir);
		
		if(rmdir(pwd)!=0) {
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

int is_ext_rm(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_rm(char *file){
	return is_ext_rm(file, ".tar") || is_ext_rm(file, ".tar/");
}


int isSameDir_rm(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}















