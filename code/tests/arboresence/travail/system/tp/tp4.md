TP nº4 : le système de gestion des fichiers (SGF)
==========================

**L3 Informatique - Système**


Ce TP porte, d'une part, sur la consultation du contenu des inœuds, et
d'autre part sur le parcours de répertoire.


#### Rappels :

* les fonctions `stat()` et `lstat()` retournent une `struct stat`
  contenant (à peu près) toutes les informations concernant un inœud
  identifié par une quelconque de ses références, entre autres :

```C
struct stat {
  dev_t     st_dev;         /* périphérique */
  ino_t     st_ino;         /* numéro d’inœud */
  mode_t    st_mode;        /* protection */
  nlink_t   st_nlink;       /* nombre de liens physiques */
  uid_t     st_uid;         /* UID du propriétaire */
  gid_t     st_gid;         /* GID du propriétaire */
  off_t     st_size;        /* taille totale en octets */
  ...
}
```

Pour plus de détails : `man 2 stat`.

* sous UNIX, un répertoire est un type particulier de fichier, constitué
  d'une liste d'_entrées de répertoire_, chacune établissant une
  correspondance entre un _nom_ et un _numéro d'inoeud_.

Un tel fichier est manipulable via le type opaque `DIR` et les fonctions
suivantes (et quelques autres, voir le `man`) :

```C
#include <dirent.h>

DIR * opendir(const char *filename);
int closedir(DIR *dirp);
struct dirent * readdir(DIR *dirp);
```

où `struct dirent` contient au moins (d'après la norme POSIX) :

```C
struct dirent {
  ino_t     d_ino;          /* numéro d'inœud */
  char      d_name[256];    /* nom du fichier */
}
```

#### Exercice 1 : consulter les caractéristiques d'un fichier à l'aide de `stat()`ou `lstat()`

* Écrire un programme qui teste à l'aide de (la valeur de retour de)
  `stat()` si une chaîne de caractères `ref` (passée en paramètre) est
  une référence valide.

* Modifier le programme pour qu'il affiche le numéro d'inœud
  correspondant à `ref` (si `ref` est valide).

* Modifier à nouveau le programme pour qu'il affiche le type de fichier
  sur lequel pointe `ref` (fichier ordinaire, répertoire, lien
  symbolique, tube nommé...). 

  [Indications](Indications/indic_1_1.md)

  Vérifier que votre programme se comporte correctement pour les fichiers
  créés (dans le répertoire courant) par le script
  [tests_tp7.sh](tests_tp7.sh). Que pensez-vous en particulier du cas de
  `test_lien`? Changer `stat()` en `lstat()` et comparer.

* Modifier le programme pour qu'il indique si le fichier est exécutable
  par au moins une catégorie d'utilisateurs. 

  [Indications](Indications/indic_1_2.md)

  Vérifier que `tests_tp7.sh`, `test_rep`, `test_exe1` et `test_exe2` sont
  bien reconnus comme exécutables, contrairement à `test_fic`. Qu'en est-il
  de `test_lien` ?


#### Exercice 2 : lire les entrées d'un répertoire avec `readdir()` (ainsi qu'`opendir()` et `closedir()`)

* Écrire un programme ayant le même comportement que `ls -a` (ou `ls -f`,
  plutôt; consulter `man ls` si nécessaire).

  [Indications](Indications/indic_2_1.md)

* Modifier votre programme pour obtenir (à peu près) le comportement de
  `ls -i` (à peu près au sens où on se contentera d'un `ls -i` non trié,
  mais pas d'un `ls -ai`).

  [Indications](Indications/indic_2_2.md)


#### Exercice 3 : chercher un fichier

Le but de cet exercice est d'écrire un programme effectuant une recherche 
analogue à `find . -name toto`, où `toto` est passé en paramètre. 

Cela nécessite de parcourir récursivement toute l'arborescence dont le
répertoire courant est la racine, pour chercher dans chaque répertoire si
une entrée porte le nom (de base) `toto`.

Cette récursion peut être gérée de plusieurs manières, par exemple :

  * soit, sans jamais changer de répertoire courant, en construisant les
    références complètes des répertoires et autres fichiers manipulés par
    concaténation des noms de base des répertoires sur le chemin;

  * soit en changeant de répertoire courant à chaque appel récursif, pour
    que les noms de base des entrées de répertoire soient des références
    (relatives au répertoire courant) correctes. Attention alors à
    rétablir ensuite l'ancien répertoire courant...


Dans tous les cas, attention à ne pas partir en récursion infinie!
