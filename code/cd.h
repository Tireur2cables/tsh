#ifndef CD_H
#define CD_H

extern int cd(int, char *[]);

char * home=NULL;
char * pwd=NULL; 

void path_initialisation();
char * isTAR(char *);
void actuPath(char *);






#endif