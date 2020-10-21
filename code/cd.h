#ifndef CD_H
#define CD_H


extern int cd(int, char *[]);

void path_initialisation();
int isTAR(char *,unsigned char);
int actuPath(char *);
int errorDetect(int);

#endif
