CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
ALL = test tsh
all : $(ALL)
test : test.o ls.o cd.o pwd.o
tsh : tsh.o ls.o cd.o pwd.o cat.o help.o exit.o cdIn.o tar.o mkdir.o rm.o cp.o rmdir.o mv.o
help.o : help.c help.h
cd.o : cd.c tar.h cd.h
cdIn.o : cdIn.c cdIn.h
cat.o : cat.c cat.h tar.h
mkdir.o : mkdir.c mkdir.h tar.h
rmdir.o : rmdir.c tar.h rmdir.h
test.o : test.c ls.h cd.h pwd.h
mv.o : mv.c cp.h rm.h
exit.o : exit.c exit.h
ls.o : ls.c tar.h ls.h
tsh.o : tsh.c couple.h ls.h cd.h pwd.h cat.h help.h exit.h cdIn.h tar.h mkdir.h rm.h cp.h rmdir.h mv.h
rm.o : rm.c tar.h rm.h
pwd.o : pwd.c pwd.h
tar.o : tar.c tar.h
cp.o : cp.c cp.h cat.h tar.h mkdir.h
clean :
	rm -rf *~
cleanall : clean
	rm -rf test tsh *.o
