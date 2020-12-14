#ifndef COUPLE_H
#define COUPLE_H

#include <string.h>

typedef struct {
	char *name;
	int (*fun)(int, char *[]);
} couple;

int isIn(char const *line, couple tab[], int len) {
	char tmp[strlen(line)+1];
	strcpy(tmp, line);
	char *cmd = strtok(tmp, " ");
	for (int i = 0; i < len; i++) {
		if (strcmp(cmd, tab[i].name) == 0) return 1;
	}
	return 0;
}

int (*getFun(char *line, couple tab[], int len))(int, char *[]) {
	char tmp[strlen(line)+1];
	strcpy(tmp, line);
	char *name = strtok(tmp, " ");
	for (int i = 0; i < len; i++) {
		if (strcmp(name, tab[i].name) == 0) return tab[i].fun;
	}
	return NULL;
}

#endif
