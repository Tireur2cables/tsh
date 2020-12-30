#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>

#include "ls.h"
#include "cd.h"
#include "pwd.h"


int generate_files();
int remove_files();

int main(int argc, char const *argv[]) {
	generate_files();
	DIR* dir = opendir("../test/tests_in");

	struct dirent * readdir(DIR *dirp);
	int output = open("../test/test_out",  O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU);
	int input = open("../test/test_in",  O_RDONLY);
	int save = dup(1);
	dup2(output, STDOUT_FILENO);

	char *arg[2];
	char *arg2[2];
	arg[0] = "ls";
	arg[1] = "tests";
	arg2[0] = "ls";
	arg2[1] = "tests";
	ls(2, arg);
	ls(2, arg2);
	close(output);
	dup2(save, STDOUT_FILENO);
	close(save);

	int output2 = open("../test/test_out",  O_RDONLY);
	switch(fork()){
		case -1:
			exit(1);
		case 0:
			execlp("diff", "diff", "../test/test_out", "../test/test_in", NULL);
		default:
			wait(NULL);
	}
	remove_files();
	return 0;
}


int generate_files(){
	//TEMPORAIRE POUR CREER L'ARBORESCENCE DE FICHIER POUR EFFECTUER LES TESTS
	system("../test/test.sh");
	return 0;
}

int remove_files(){
	system("../test/testrm.sh");
	return 0;
}
