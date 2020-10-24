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
int findTarIn(char *, int);
int appelSurTar(char *, char *, char *);
int getLenLast(char const *);

char *findpere(char *);
int isOnlySpaceString(char *);

#endif
