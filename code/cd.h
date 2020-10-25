#ifndef CD_H
#define CD_H


extern int cd(int, char *[]);

int parcoursChemin(char *, char *, char *);
int parcoursTar(char *, char *, char *);
int isAccessibleFrom(char *, char *);
int setPath(char *, char *);
int errorDetect(int);
int isTar(char *);
int isSameDir(char *, char *);
int findTarIn(char const *, int);
int appelSurTar(char *, char *);
int getLenLast(char const *);
char *getRealChemin(char *, char *);

#endif
