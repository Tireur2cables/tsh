#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cat.h"

char read_line_buf[BUFFSIZE];
int read_line_pos;
int read_line_siz = 0;

int cat(int argc, char *argv[]) {
	int input = -1;
	if (argc < 2){
		input = STDIN_FILENO;
	}else{
		char *file = argv[1];
		input = open(file, O_RDONLY);
	}
	int eof = 0;
	char buffer[BUFFSIZE];
	int readsize = 0;
	while(!eof){
		if((readsize = read_line(input, buffer, BUFFSIZE)) == 0){
			eof = 1;
		}
		if(readsize < 0){
			printf("erreur");
			return -1;
		}
		if((write(STDOUT_FILENO, buffer, readsize)) < readsize){
			return -1;
		}
	}
	return 0;
}

ssize_t read_line(int fd, char *buf, size_t count){
	if(read_line_siz == 0){
		if((read_line_siz = read(fd, read_line_buf, BUFFSIZE)) < 0){
			return -1;
		}
	}
	int i;
	for(i = 0;read_line_pos < read_line_siz && i < count; i++, read_line_pos++){
		buf[i] = read_line_buf[read_line_pos];
		if(buf[i] == '\n'){
			i++;
			break;
		}
	}
	read_line_pos++;
	if(read_line_siz == read_line_pos){
		read_line_pos = 0;
		read_line_siz = 0;
	}
	if(i == (count-1) && buf[i-1] != '\n') return -1;
	return i;
}
