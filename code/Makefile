CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
all : tsh test
tsh : tsh.o cd.o ls.o pwd.o help.o
test : test.c ls.o pwd.o cd.o
cd.o : cd.c tar.h cd.h
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c ls.h cd.h pwd.h
pwd.o : pwd.c pwd.h
help.o : help.c help.h
cleanall : clean
	rm -rf tsh *.o
clean :
	rm -rf *~
