CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS =
all : tsh
tsh : tsh.o ls.o mycat.o
ls.o : ls.c ls.h tar.h
tsh.o : tsh.c mycat.h ls.h
mycat.o : mycat.c mycat.h
cleanall : clean
	rm -rf tsh *.o
clean :
	rm -rf *~

