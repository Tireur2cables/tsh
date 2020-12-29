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
#include <assert.h>
#include "tar.h"
#include "rmdir.h"

int parcoursChemin_rmdir(char *, char *);
int isAccessibleFrom_rmdir(char *, char *);
int deleteDir(char *);
int isDirTarEmpty(char * , char * , char *);
int DeleteDirTar(char * , char * , char * );
int isDirEmpty(char *);
int parcoursCheminTar_rmdir(char *, char *, char *);
int is_ext_rmdir(char *, char *);
int is_tar_rmdir(char *);
int isSameDir_rmdir(char *, char *);


int rmdir_func(int argc,char **argv) {
	if(argc==0) {
		errno=EINVAL;
		perror("program error");
		exit(EXIT_FAILURE);
	}

	if(argc==1) {
		errno=EINVAL;
		perror("missing operand");
		return -1;
	}

	for(int i=1;i<argc;i++) {

		char chemin[strlen(argv[i])+1];
		strcpy(chemin,argv[i]);
		char * pwd = getcwd(NULL,0);
		char * twd = getenv("TWD");

		if(chemin[0]=='/') parcoursChemin_rmdir(chemin,"/");
		else if (twd == NULL || strlen(twd) == 0) parcoursChemin_rmdir(chemin,pwd);
		else parcoursCheminTar_rmdir(pwd, twd, chemin);



	}

	return 0;


}


int parcoursChemin_rmdir(char * chemin, char * pwd) {
	char *saveptr;
	char *doss;
	char *currentpwd = malloc(strlen(pwd) + 1);
	assert(currentpwd);
	strcpy(currentpwd, pwd);

	while((doss=strtok_r(chemin,"/",&saveptr))!=NULL) {

		if(isAccessibleFrom_rmdir(doss,currentpwd) > 0) {
			if(strstr(doss,".tar")!=NULL && (is_tar_rmdir(doss)>0) ) {  // tar dans chemin

				char newcurr[strlen(currentpwd) + 1];
				strcpy(newcurr, currentpwd);
				free(currentpwd);
				return parcoursCheminTar_rmdir(newcurr,doss,saveptr);
			}

			int currentlen = strlen(currentpwd);
			if (currentlen != 1) currentlen++; // pas la racine
			char newcurr[currentlen + strlen(doss) + 1];
			strcpy(newcurr, currentpwd);
			if (currentlen != 1) strcat(newcurr, "/");
			strcat(newcurr, doss);

			currentpwd = realloc(currentpwd, strlen(newcurr)+1);
			assert(currentpwd);
			strcpy(currentpwd, newcurr);

			chemin=saveptr;

		} else {
			free(currentpwd);
			return -1;
		}

	}
	write(STDOUT_FILENO, "yes\n", 4);
	char res[strlen(currentpwd)+1];
	strcpy(res,currentpwd);
	free(currentpwd);
	deleteDir(res);

	return 0;


}


int isAccessibleFrom_rmdir(char * doss , char * dir) {

	DIR * courant = opendir(dir);

	if (courant == NULL) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir le répertoire ";
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

				if (!S_ISDIR(st.st_mode)) { //not directory
					char *deb  = "rmdir : ";
					char *end = " n'est pas un répertoire ou une archive !\n";
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
		char *deb  = "rmdir : ";
		char *end = " n'existe pas!\n";
		char error[strlen(deb) + strlen(doss) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, doss);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	return found;
}


int parcoursCheminTar_rmdir(char * pwd , char * twd , char *rest) {

	char twd_copy[strlen(twd)+1];
	strcpy(twd_copy, twd);
	char *tar = strtok(twd_copy, "/");
	char absolutetar[strlen(pwd) + 1 + strlen(tar) + 1];
	strcpy(absolutetar, pwd);
	strcat(absolutetar, "/");
	strcat(absolutetar, tar);

	char chemin[strlen(twd) - strlen(tar) + strlen(rest) + 1];
	if (strlen(twd) - strlen(tar) > 0) strcpy(chemin, &twd[strlen(tar)+1]);
	else strcpy(chemin, &twd[strlen(tar)]);
	if (strlen(chemin) != 0) strcat(chemin, "/");
	strcat(chemin, rest);

	int fd = open(absolutetar, O_RDONLY);

	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
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
			if (nom[strlen(nom)-1] == '/' && isSameDir_rmdir(chemin, nom)) found = 1;
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
		char *deb  = "rmdir : ";
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
	}else if (found == 1) {
		char res[strlen(absolutetar) + 1 + strlen(chemin) + 1];
		strcpy(res, absolutetar);
		if (chemin != NULL && strlen(chemin) != 0) {
			strcat(res, "/");
			strcat(res, chemin);
		}
		if (res[strlen(res)-1] == '/') res[strlen(res)-1] = '\0';

		if(strcmp(absolutetar,res)==0) {
			char *deb  = "rmdir : ";
			char *end = " n'est pas un répertoire!\n";
			char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(end) + 1];
			strcpy(error, deb);
			strcat(error, absolutetar);
			strcat(error, end);
			int errorlen = strlen(error);
			if (write(STDERR_FILENO, error, errorlen) < errorlen) {
				perror("Erreur d'écriture dans le shell!");
			} return -1;
		}

		isDirTarEmpty(absolutetar,chemin,res); /// suppression


	}else {
		char *deb  = "rmdir : ";
		char *end = " n'est pas un répertoire!\n";
		char error[strlen(deb) + strlen(absolutetar) + 1 + strlen(chemin) + strlen(end) + 1];
		strcpy(error, deb);
		strcat(error, absolutetar);
		strcat(error, "/");
		strcat(error, chemin);
		strcat(error, end);
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	return (found == 1)? 0 : -1;



}

int isDirTarEmpty(char * absolutetar , char * chemin , char * res) {

	char neochemin[strlen(chemin)+1+1];

	if(chemin[strlen(chemin)-1]!='/') {
		strcpy(neochemin,chemin);
		strcat(neochemin,"/");
	} else {
		strcpy(neochemin,chemin);
	}

	int fd = open (absolutetar,O_RDONLY);
	struct posix_header * header = malloc(sizeof(struct posix_header));

	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;
	int nbfound=-1;

	while(!found) {

		if(nbfound>0) {
			char * deb = "rmdir : le répertoire ";
			char * end = " n'est pas vide";
			char error[strlen(deb)+strlen(res)+strlen(end)+1];
			strcpy(error,deb);
			strcat(error,res);
			strcat(error,end);
			strcat(error,"\n");
			if(write(STDERR_FILENO,error,strlen(error))<strlen(error))
				perror("Erreur d'écriture dans le shell!");
			return -1;
		}

		if(read(fd,header,BLOCKSIZE)<BLOCKSIZE) break;
		//if(strcmp(header->name, "\0") == 0) printf("backslash\n");

		char nom[strlen(header->name)+1];
		strcpy(nom,header->name);
		if(strstr(nom,neochemin)!=NULL) nbfound++;

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header->size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);

		read(fd, header, BLOCKSIZE*filesize);

		}

	close(fd);

	DeleteDirTar(absolutetar , chemin , res);


	return 0;

}

int DeleteDirTar(char * absolutetar , char * chemin , char * res) {
	char neochemin[strlen(chemin)+1+1];

	if(chemin[strlen(chemin)-1]!='/') {
		strcpy(neochemin,chemin);
		strcat(neochemin,"/");
	} else {
		strcpy(neochemin,chemin);
	}


	int fd = open (absolutetar,O_RDWR);
	struct posix_header * header = malloc(sizeof(struct posix_header));


	if (fd == -1) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir l'archive ";
		char error[strlen(error_debut) + strlen(absolutetar) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, absolutetar);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
	}

	int found=0;

	while(!found) {

		if(read(fd,header,BLOCKSIZE)<BLOCKSIZE) break;

		char nom[strlen(header->name)+1];
		strcpy(nom,header->name);

		int taille = 0;
		int *ptaille = &taille;
		sscanf(header->size, "%o", ptaille);
		int filesize = ((*ptaille + 512-1)/512);

		if(strcmp(nom,neochemin)==0) {

			off_t position;
			off_t endposition;

			if((position=lseek(fd, -BLOCKSIZE , SEEK_CUR))==-1) break;
			if((endposition=lseek(fd, 0 , SEEK_END))==-1) break;

			unsigned int size= endposition - (position+BLOCKSIZE);
			char cpy[size];

			if(lseek(fd, position+BLOCKSIZE , SEEK_SET)==-1) break;
			read(fd , cpy , size);

			if(lseek(fd, position , SEEK_SET)==-1) break;
			write(fd , cpy , size);

			found=1;
			break;

		}

		read(fd, header, BLOCKSIZE*filesize);

	}

	close(fd);


	return 0;


}

int deleteDir(char * pwd) {


	if(isDirEmpty(pwd)<0) {  // pas vide

		errno = ENOTEMPTY;
		char * error1 = "rmdir : le répertoire ";
		char * error2 = " n'est pas vide !";
		char error3[strlen(error1) + 1 + strlen(pwd) + 1 + strlen(error2) + 1];
		strcpy(error3,error1);
		strcat(error3,pwd);
		strcat(error3,error2);
		perror(error3);
		return -1;

	} else {

	if(rmdir(pwd)<0) {
			perror("erreur de suppression du répertoire");
			return -1;
		}

	}

	return 0;

}

int is_ext_rmdir(char *file, char *ext){
	return (strcmp(&file[strlen(file)-strlen(ext)], ext) == 0);
}

int is_tar_rmdir(char *file){
	return is_ext_rmdir(file, ".tar") || is_ext_rmdir(file, ".tar/");
}


int isDirEmpty(char * pwd ) {

	DIR * dir = opendir(pwd);
	struct dirent * d;
	int nbFile=0;  // cb de fichiers dans le rep

	if (dir == NULL) {
		char *error_debut = "rmdir : Erreur! Impossible d'ouvrir le répertoire ";
		char error[strlen(error_debut) + strlen(pwd) + 1 + 1];
		strcpy(error, error_debut);
		strcat(error, pwd);
		strcat(error, "\n");
		int errorlen = strlen(error);
		if (write(STDERR_FILENO, error, errorlen) < errorlen)
			perror("Erreur d'écriture dans le shell!");
		return -1;
	}

	while((d=readdir(dir))!=NULL) {
		nbFile++;
	}

	if(nbFile<3) return 0;
	return -1;

}

int isSameDir_rmdir(char *dir1, char *dir2) {
	return
	(strcmp(dir1, dir2) == 0) ||
	((strncmp(dir1, dir2, strlen(dir1)) == 0)
		&& (strlen(dir2) == strlen(dir1)+1)
		&& (dir1[strlen(dir1)-1] != '/')
		&& (dir2[strlen(dir2)-1] == '/'));
}
