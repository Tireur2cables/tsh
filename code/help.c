#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "help.h"

int help(int argc, char *argv[]) {
	if (argc != 1) {
		errno = E2BIG;
		perror("Trop d'arguments!");
		exit(EXIT_FAILURE);
	}

	char *help = "Voici la liste des commandes implémentées :\n'cat' : écrit sur sa sortie le contenu de son entrée.\n'cd' : se déplace dans un dossier ou dans un tarball.\n'exit' : quitte le tsh.\n'help' : vous venez de faire cette commande. :)\n'ls' : affiche le nom du fichier, le contenu du dossier ou le contenu du tarball demandé (option gérée : -l -> affiche plus d'informations).\n'mkdir' : créer un dossier (ou tarball) au chemin spécifié.\n'mv' : déplace le fichier ou dossier (ou tarball) à l'endroit demandé.\n'pwd' : affiche le répertoire courant (même avec tarball).\n'rm' : supprime le fichier demandé, (option gérée : -r -> suprrime récursivement fichiers et sous dossiers).\n'rmdir' : supprime le dossier (ou tarball) demandé s'il est vide.\nVous pouvez également utiliser toutes les commandes habituelles quand ça n'implique pas un tarball (voir 'man').\nIl est également possible de faire des redirections d'entrée (' < '), redirections de sortie (' > ' ou ' >> ') et redirections de sortie erreur (' 2> ' ou ' 2>> ') ainsi que des tubes ('|').\n\n";
	int help_len = strlen(help);
	if (write(STDOUT_FILENO, help, help_len) < help_len) {
		perror("Erreur d'écriture dans les shell!");
		exit(EXIT_FAILURE);
	}
	return 0;
}
