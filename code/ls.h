#ifndef LS_H
#define LS_H

extern int ls(int, char *[]);

void show_simple_header_infos(struct posix_header *, int *);
void show_complete_header_infos(struct posix_header *, int *);

#endif
