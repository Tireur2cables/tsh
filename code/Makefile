CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
all : tsh
tsh : tsh.o ls.o
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c ls.h
cleanall : clean
	rm -rf tsh *.o
clean :
	rm -rf *~

