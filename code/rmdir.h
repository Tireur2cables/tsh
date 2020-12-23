#ifndef RMDIR_H
#define RMDIR_H

extern int rmdir_func(int, char *[]); 

int parcoursChemin(char *, char *);
int isAccessibleFrom(char *, char *);
int deleteDir(char *);
int isDirEmpty(char *);
int parcoursCheminTar(char *, char *, char *);

#endif

