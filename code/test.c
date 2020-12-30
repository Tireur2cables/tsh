#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>

#include "couple.h"
#include "ls.h"
#include "cd.h"
#include "cat.h"
#include "pwd.h"


int generate_files();
int remove_files();

int nb_test = 4;
char *test_file[4] = {"../test/tests_in/ls1", "../test/tests_in/ls2", "../test/tests_in/ls3", "..test/tests_in/cat1"};
int (*fun[4])(int, char *[]) = {ls, ls, ls, cat};

int main(int argc, char const *argv[]) {
	remove_files();
	generate_files();
	char *arg1[2];
	char *arg2[3];
	char *arg3[2];
	char *arg4[2];
	arg1[0] = "ls";
	arg1[1] = "../test/tests";
	arg2[0] = "ls";
	arg2[1] = "-l";
	arg2[2] = "../test/static";
	arg3[0] = "ls";
	arg3[1] = "../test/testss";
	arg4[0] = "cat";
	arg4[1] = "../test/static/file";

	char **test_arg[4] = {arg1, arg2, arg3, arg4};
	int nb_arg[4] = {2,3, 2, 2};

	int w;
	for(int i = 0; i < nb_test; i++){
		int output = open("../test/test_out",  O_RDWR + O_CREAT + O_TRUNC, S_IRWXU);
		int save = dup(1);
		int save_err = dup(2);
		dup2(output, STDOUT_FILENO);
		dup2(output, STDERR_FILENO);
		fun[i](nb_arg[i], test_arg[i]);
		close(output);
		dup2(save, STDOUT_FILENO);
		dup2(save, STDERR_FILENO);
		close(save);
		close(save_err);
		switch(fork()){
			case -1:
				exit(1);
			case 0:
				execlp("diff", "diff", "../test/test_out", test_file[i], NULL);
			default:
				wait(&w);
				if(w == 0){
					char format[60];
					sprintf(format, "test %d passé avec succès\n", i);
					write(STDOUT_FILENO, format, strlen(format));
				}
		}
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
