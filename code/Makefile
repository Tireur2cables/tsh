CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
ALL = test tsh
all : $(ALL)
test : test.o ls.o cd.o pwd.o
<<<<<<< HEAD
tsh : tsh.o ls.o cd.o pwd.o cat.o help.o exit.o cdIn.o mkdir.o tar.o
=======
tsh : tsh.o ls.o cd.o pwd.o cat.o help.o exit.o cdIn.o mkdir.o
>>>>>>> b5b4f253881bd62d1eae601ea15272b61d75c6d1
help.o : help.c help.h
cd.o : cd.c tar.h cd.h
cdIn.o : cdIn.c cdIn.h
cat.o : cat.c cat.h tar.h
test.o : test.c ls.h cd.h pwd.h
mkdir.o : mkdir.c mkdir.h tar.h
exit.o : exit.c exit.h
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c couple.h ls.h cd.h pwd.h cat.h help.h exit.h cdIn.h tar.h
pwd.o : pwd.c pwd.h
tar.o : tar.c tar.h
clean :
	rm -rf *~
cleanall : clean
	rm -rf test tsh *.o
