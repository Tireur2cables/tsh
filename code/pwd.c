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
#include "pwd.h"


int pwd_func(int argc,char **argv) {

	if(argc==0) {
		errno = EINVAL;
		perror("error 404");
		exit(EXIT_FAILURE);

	} else {

		char *home=getcwd(NULL,0); //FIXME ajouter retour à la ligne

		if(write(1,home,strlen(home))==-1) {
			errno = EINTR;
			perror("error 9999");
			exit(EXIT_FAILURE);
		} else {
			write(1,"\n",1);
		}

	}


	return 0;


}
