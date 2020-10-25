#ifndef RMDIR_H
#define RMDIR_H

extern int rmdir_func(int, char *[]); 

int openDetectError(DIR *);
int fileFound(int);
void NotDirectory(char *);
int actuDir(char *);

#endif

