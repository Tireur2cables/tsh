#ifndef LS_H
#define LS_H

#include <dirent.h>

extern int ls(int, char *[]);

void show_simple_header_infos(struct posix_header *, int *);
void get_header_size(struct posix_header *, int *);
void show_complete_header_infos(struct posix_header *, int *);
int print_normal_dir(char *);
int print_complete_normal_dir(char *);
int print_dir(char *, char *);
int print_tar(char *, char *);
int print_inside_tar(char *, char *);
int print_rep(char *, char *);
int is_tar(char *);
int contains_tar(char *);
int check_options(char *);
int is_options(char *);
int is_curr_or_parent_rep(char *);
int is_ext(char *, char *);
int nbdigit(int);
void convert_mode(mode_t, char*);
int count_slash(char *);
int get_filename(char *, char*);

#endif
