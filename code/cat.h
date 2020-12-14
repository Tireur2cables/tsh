#ifndef CAT_H
#define CAT_H

//define size of buffer for read_line
#define BUFFSIZE 1024

extern int cat(int, char *[]);

int is_tar_cat(char *);
int contains_tar_cat(char *);
int is_ext_cat(char *, char *);
int cat_tar(char *, char *);
int cat_file(char *, char *);
ssize_t read_line(int, char *, size_t);
int get_header_size_cat_t(struct posix_header *, int *);

#endif
