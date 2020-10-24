#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ls.h"
#include "cd.h"
#include "pwd.h"


int generate_files();
int remove_files();

int main(int argc, char const *argv[]) {
	generate_files();
	int output = open("../test/test_out",  O_WRONLY + O_CREAT + O_TRUNC, S_IRWXU);
	int input = open("../test/test_in",  O_RDONLY);
	char in_str[71];
<<<<<<< HEAD
	char out_str[71];
=======
	char out_str[4];
>>>>>>> début de la redaction des testsé
	//write(output, "test_rep  ..  test_fic  test_exe2  .  test_exe1  test_lien  test_fifo  \n", strlen("test_rep  ..  test_fic  test_exe2  .  test_exe1  test_lien  test_fifo  \n"));
	//printf("o : %d, i : %d ", output, input);
	int save = dup(1);
	//printf("%d", save_out);
<<<<<<< HEAD
	dup2(output, STDOUT_FILENO);
	char *arg[2];
	arg[0] = "ls";
	arg[1] = "tests";
	ls(2,arg);
	close(output);
	dup2(save, STDOUT_FILENO);
	close(save);

	int output2 = open("../test/test_out",  O_RDONLY);
	read(output2, out_str, 70);
	read(input, in_str, 70);
	printf(out_str);
	printf(in_str);
=======
	printf("%d", dup2(output, STDOUT_FILENO));
	printf("aled");
	/*
	char *arg[2];
	arg[0] = "ls";
	arg[1] = "tests";
	ls(2,arg);*/
	close(output);
	printf("%d", dup2(STDOUT_FILENO, save));
	//close(save_out);

	int output2 = open("../test/test_out",  O_RDONLY);
	read(output2, out_str, 4);
	//read(input, in_str, 70);
	printf(out_str);
	//printf(in_str);
>>>>>>> début de la redaction des testsé
	if(strcmp(out_str, in_str) == 0){
		printf("Les fichiers sont les mêmes");
	}else{
		printf("Les fichiers sont différents");
	}
	close(input);
	close(output2);
	remove_files();
	return 0;
}


int generate_files(){
	system("../test/test.sh");
	return 0;
}

int remove_files(){
	system("../test/testrm.sh");
	return 0;
}
