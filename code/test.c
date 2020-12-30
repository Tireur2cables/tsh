#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>

#include "tar.h"
#include "couple.h"
#include "ls.h"
#include "cd.h"
#include "cat.h"
#include "pwd.h"
#include "mkdir.h"
#include "rm.h"


int generate_files();
int remove_files();

#define NB_TESTS 15

int (*fun[NB_TESTS])(int, char *[]) = {ls, ls, ls, ls, cd, ls, ls, cd, ls, cat, cd, mkdir_tar, mkdir_tar, cd, rm_func};

int main(int argc, char const *argv[]) {
	//remove_files();
	char *home = getcwd(NULL, 0);
	generate_files();
	char *arg1[2];
	char *arg2[2];
	char *arg3[2];
	char *arg4[2];
	char *arg5[2];
	char *arg6[2];
	char *arg7[1];
	char *arg8[2];
	char *arg9[2];
	char *arg10[2];
	char *arg11[2];
	char *arg12[2];
	char *arg13[2];
	char *arg14[2];
	char *arg15[3];

	arg1[0] = "ls";
	arg1[1] = "tests/tests";
	arg2[0] = "ls";
	arg2[1] = "tests/arch.tar";
	arg3[0] = "ls";
	arg3[1] = "tests/testss";
	arg4[0] = "ls";
	arg4[1] = "tests/arch.tar";
	arg5[0] = "cd";
	arg5[1] = "tests/arch.tar";
	arg6[0] = "ls";
	arg6[1] = "ll";
	arg7[0] = "ls";
	arg8[0] = "cd";
	arg8[1] = home;
	arg9[0] = "ls";
	arg9[1] = "cp.c";
	arg10[0] = "cat";
	arg10[1] = "tests/arch.tar/tests/tests/rep1/fic5";
	arg11[0] = "cd";
	arg11[1] = "tests/arch.tar/tests";
	arg12[0] = "mkdir";
	arg12[1] = "tropbien";
	arg13[0] = "mkdir";
	arg13[1] = "tropbien2";
	arg14[0] = "cd";
	arg14[1] = home;
	arg15[0] = "rm";
	arg15[1] = "-r";
	arg15[2] = "tests/arch.tar/tests/tropbien";




	char **test_arg[NB_TESTS] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15};
	int nb_arg[NB_TESTS] = {2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 3};
						//  1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
	char home_open[strlen(home) + 60];
	sprintf(home_open, "%s/tests/test_out", home);

	char home_cd1[strlen(home) + 60];
	sprintf(home_cd1, "%s/tests/tests_in/cd1", home);
	char home_ls5[strlen(home) + 60];
	sprintf(home_ls5, "%s/tests/tests_in/ls5", home);
	char home_ls7[strlen(home) + 60];
	sprintf(home_ls7, "%s/tests/tests_in/ls7", home);
	char home_cat2[strlen(home) + 60];
	sprintf(home_cat2, "%s/tests/tests_in/cat2", home);
	char *test_file[NB_TESTS] = {"tests/tests_in/ls1", "tests/tests_in/ls2", "tests/tests_in/ls3", "tests/tests_in/ls4", home_cd1, home_ls5, home_ls7, home_cd1, "tests/tests_in/ls6", "tests/tests_in/cat1", home_cd1, home_cd1, home_cd1, home_cd1, home_cd1};
	int w;
	for(int i = 0; i < NB_TESTS; i++){
		int output = open(home_open,  O_RDWR + O_CREAT + O_TRUNC, S_IRWXU);
		int save = dup(STDOUT_FILENO);
		int save_err = dup(STDERR_FILENO);
		dup2(output, STDOUT_FILENO);
		dup2(output, STDERR_FILENO);
		fun[i](nb_arg[i], test_arg[i]);
		close(output);
		dup2(save, STDOUT_FILENO);
		dup2(save_err, STDERR_FILENO);
		close(save);
		close(save_err);
		switch(fork()){
			case -1:
				exit(EXIT_FAILURE);
			case 0:
				execlp("diff", "diff", home_open, test_file[i], NULL);
			default:
				wait(&w);
				if(w == 0){
					char format[60];
					sprintf(format, "test %d passé avec succès\n", i+1);
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
