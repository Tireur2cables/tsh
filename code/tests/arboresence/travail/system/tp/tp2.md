TP nº2 : filtres UNIX
=====================

**L3 Informatique - Système**


Filtres
-------

On rappelle qu'un **filtre** est un programme permettant d'effectuer des
transformations sur les flux de données. Dans le contexte de systèmes
d'exploitation de type *UNIX*, les filtres sont typiquement des
programmes qui utilisent principalement l'entrée standard (`stdin`) comme
source de données en entrée et la sortie standard (`stdout`) comme
destination des données en sortie.

Exemples de filtres UNIX classiques :

  - `cat` : appelé sans argument, copie `stdin` sur `stdout`
  - `wc` : affiche sur `stdout` le nombre de caractères, mots et lignes
    lus dans `stdin`
  - `grep PATTERN` : `cat` sélectif, *i.e.* seules les lignes de `stdin`
    qui correspondent au motif `PATTERN` (une expression régulière) sont
    copiées sur `stdout`

L'objectif de ce TP est d'implémenter (des versions simplifiées de) ces
filtres. Vous êtes invités à systématiquement consulter leurs pages de
manuel pour vérifier leur sémantique, et à les tester dans un shell.

L'implémentation des filtres doit être faite en utilisant comme fonctionnalités
d'entrée/sortie les appels systèmes `open`, `read`, `write`, etc. Il est
interdit d'utiliser les fonctions de plus haut niveau *bufferisées* de la
bibliothèque standard (`fopen`, `fread`, `fwrite`, etc.)


Exercice 1 : `cat`
-----------------

Écrire un programme `mycat` qui, exécuté sans argument, lit en boucle
depuis `stdin` et recopie ce qu'il a lu sur `stdout`. Une fois la lecture
sur `stdin` terminée (end-of-file détecté par `read`), `cat` doit
terminer.

Prenez garde à :

  - ne pas lire `stdin` caractère par caractère, mais en utilisant un
    *buffer*;
  - ne pas *bufferiser* en mémoire la totalité de `stdin` avant de
    commencer à écrire sur `stdout`; l'occupation mémoire de `mycat` doit
    rester constante, quelle que soit la taille de l'entrée;
  - gérer systématiquement les erreurs : dès qu'un appel système retourne
    une erreur, votre programme doit terminer avec une valeur de retour
    non nulle (et au contraire, si aucune erreur ne se produit pendant
    toute l'exécution de `mycat`, votre programme doit terminer avec la
    valeur de retour zéro).


Exercice 2 : taille optimale de `buf`
------------------------------------

Pour utiliser les appels systèmes `read()` et `write()`, vous devez utiliser une
portion de mémoire pour échanger les données à lire (ou à écrire) entre *user
space* et *kernel space*; il s'agit du tampon `buf` dans les prototypes des
deux fonctions correspondantes :

```C
        #include <unistd.h>

        ssize_t write(int fd, const void *buf, size_t count);
        ssize_t read(int fd, void *buf, size_t count);
```

La *taille* de `buf` a un impact sur les performances de lecture/écriture.
Essayez de copier avec votre implémentation de `cat` des fichiers de taille
considérable (plusieurs dizaines de MB, au minimum) et comparer les temps
d'exécution (avec l'outil `time`) pour différentes tailles de `buf`.  Essayez
par exemple avec les tailles suivantes : 10 KB, 1 B, 1 MB, 10 MB. Quelle taille
rend la copie la plus efficace?

Si vous n'avez pas de gros fichier à disposition, vous pouvez par exemple
en créer un à l'aide de `dd if=/dev/zero of=/tmp/toto bs=1M count=30`.

Pour copier des fichiers avec `mycat`, vous pouvez utiliser des
redirections dans le shell comme par exemple :

```bash
        mycat < BIG_FILE > BIG_FILE_NEW
```

Exercice 3 : `read_line()`
-----------------------

Implémenter une fonction `read_line()` avec le prototype suivant :

```
        ssize_t read_line(int fd, void *buf, size_t count)
```

Cette fonction se comporte comme `read` (et a le même prototype), mais
garantit que la chaîne de caractères retournée dans `buf` termine au
premier `'\n'` lu (inclus). Quand cela s'avère impossible (p.ex., si
aucun `'\n'` n'est lu dans les `count` premiers octets du descripteur de
fichier `fd`), `read_line()` doit retourner -1 pour indiquer une erreur.

Par exemple, si la lecture est effectuée par blocs de 1024 octets, que la
première ligne lue contient un `'\n'` en position 71 et la deuxième ligne
un `'\n'` en position 14, le premier appel à `read_line()` retournera un
*buffer* de taille 72 (avec les octets 0..71 lus depuis `fd`), et le
deuxième un *buffer* de taille 14 (avec les octets 72..85 lus depuis
`fd`).

La particularité de `read_line()` est qu'elle doit *bufferiser* la
lecture depuis `fd` : `read_line()` doit lire `fd` avec `read()`
**uniquement** si nécessaire, et donc pas forcément à chaque appel. Dans
l'exemple précédent, si `read_line()` utilise  un *buffer* interne
suffisamment grand (p.ex., 1 KB), un seul appel à `read()` doit être
effectué (lors du premier appel à `read_line()`); le deuxième appel à
`read_line()` doit retourner la deuxième ligne sans effectuer d'autre
`read()`.

Il faudra donc veiller à bien gérer les caractères lus par `read()`
au-delà du premier caractère `'\n'`. Pour cela, vous pouvez utiliser des
variables globales comme `read_line_buf`, `read_line_pos`, et
`read_line_siz` pour gérer un *buffer* persistant à travers différents
appels à `read_line()`.

Exemple 4 : `wc -l`
-----

À l'aide de la fonction `read_line()` de l'exercice précédent,
implémenter une commande équivalente à `wc -l` (qui affiche le nombre de
lignes reçues dans `stdin`).

Ajouter la possibilité de transmettre un nom de fichier en paramètre à
votre commande (qui doit alors afficher le nombre de lignes du fichier
dont le nom est passé en paramètre).

Exercice 5 : `grep`
------------------

Écrire un programme `mygrep` qui implémente une version simplifiée du filtre
UNIX `grep`. Quand on exécute `mygrep SUBSTRING`, votre programme devra lire
`stdin` et recopier sur `stdout` les *lignes* qui contiennent la
chaîne de caractères `SUBSTRING` quelque part entre le début et la fin de la
ligne, et elles seules.

Conseils :

- utilisez la fonction `read_line()` implémentée dans l'exercice 3;

- pour vérifier si une chaîne de caractères en contient une autre, vous pouvez
  utiliser la fonction `memmem` de la bibliothèque standard.
  
 Ajouter la possibilité de transmettre un nom de fichier en paramètre à votre commande.
