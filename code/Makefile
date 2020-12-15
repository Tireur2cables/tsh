CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
ALL = test tsh
all : $(ALL)
test : test.o ls.o cd.o pwd.o
tsh : tsh.o ls.o cd.o pwd.o cat.o
cd.o : cd.c tar.h cd.h
cat.o : cat.c cat.h tar.h
test.o : test.c ls.h cd.h pwd.h
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c ls.h cd.h pwd.h cat.h
pwd.o : pwd.c pwd.h
clean :
	rm -rf *~
cleanall : clean
	rm -rf test tsh *.o

