CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
ALL = test tsh
all : $(ALL)
test : test.o ls.o cd.o pwd.o
tsh : tsh.o ls.o cd.o pwd.o help.o exit.o cdIn.o
help.o : help.c help.h
cd.o : cd.c tar.h cd.h
cdIn.o : cdIn.c cdIn.h
test.o : test.c ls.h cd.h pwd.h
exit.o : exit.c exit.h
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c couple.h ls.h cd.h pwd.h help.h exit.h cdIn.h
pwd.o : pwd.c pwd.h
clean :
	rm -rf *~
cleanall : clean
	rm -rf test tsh *.o

