#ifndef CD_H
#define CD_H


extern int cd(int, char *[]);

int isTAR(char *);
int actuPath(char *, char *);
int setPath(char *, char *);
int errorDetect(int);

#endif
