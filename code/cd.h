#ifndef CD_H
#define CD_H


extern int cd(int, char *[]);

int isTAR(char *);
int parcoursChemin(char *, char *);
int selectNewPath(char *, char *);
int actuPath(char *, char *);
int setPath(char *, char *);
int errorDetect(int);
char *findpere(char *);
int isOnlySpaceString(char *);

#endif
