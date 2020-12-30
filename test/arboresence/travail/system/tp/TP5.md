TP nº4 : duplication de processus
=================================

**L3 Informatique - Système**


Exercice 1 : cacophonie
-----------------------

1. Écrire un programme qui, lorsque compilé et exécuté, lance un processus qui fait les choses suivantes:
_ Le processus commence par créer un processus fils,
_ Il affiche affiche 100 fois "ouaf!" sur la sortie standard (avec à chaque fois un retour à la ligne), et son fils affiche 100 fois "miaou!". Vous réaliserez cela au moyen de deux simples boucles.
Qu'observez vous ? Pourquoi ?
2. Écrire un programme qui fait la même chose que le programme précédent, à ceci près qu'il n'a le droit d'afficher uniquement le contenu d'une seule variable `cri`. Vous stockerez donc tour à tour dans cette variable les chaînes de caratères "ouaf!" et "miaou". Qu'observez vous ? Pourquoi ?


Exercice 2 : généalogie imposée
-------------------------------

1. Écrire un programme qui crée la généalogie de processus suivante :

   ![](arbre_processus.png)

   Chaque processus doit afficher son pid et celui de son père, puis
   faire un appel à `pause()`. Vérifier (depuis un autre terminal) que
   la généalogie est bien celle attendue à l'aide de `ps` ou `pstree`
   avant de terminer les processus.

2. Modifier le programme pour que le processus "racine" initialise une
   variable `profondeur` à 0. Chaque processus doit ensuite utiliser
   `profondeur` pour préciser sa position dans l'arbre en affichant,
   en plus des pids, sa profondeur et celle de son père.


Exercice 3 : exécutions de programmes
--------------------------------------------

1. Écrire un programme `twice` tel que `./twice cmd arg1 .. argn` exécute
deux fois la commande `cmd` avec ses arguments `arg1 .. argn`.

2. Écrire un programme `repeat` tel que `./repeat k cmd arg1 .. argn` exécute k fois la commande `cmd` avec ses arguments `arg1 .. argn`.

Exercice 4 : recherches d'éléments
----------------------------------

1. Écrire un programme qui crée un tableau de n cases, où n est un entier passé en ligne de commande, contenant des entiers aléatoires entre 0 et 100, puis affiche tous les indices des cases de ce tableau dont la valeur est 33. Pour transformer un argument contenant la représentation textuelle d'un entier en `int`, vous pourrez utiliser la fonction `atoi`.


2. Modifier le programme pour qu'il utilise deux processus, le père qui parcourt la première moitié du tableau, et le fils qui parcourt la seconde.

Exercice 5 : logarithme discret
--------------------------------------

L'objectif de cet exercice est d'utiliser ce que vous avez appris sur les processus pour paralléliser une recherche naïve de logarithme discret. Plus précisément, étant donné un nombre premier p, un entier positif n inférieur à p, et un entier m inférieur à p, nous voulons trouver, si il existe, un entier l compris entre 0 et p-1 tel que n^l modulo p est égal à m. On rappelle que l'opérateur modulo est implémenté en c par l'opérateur `%`.

1. Ecrire un programme `logdiscret` qui fait la chose suivante :
`./logdiscret n m p` affiche sur la sortie standard au moins un entier l compris entre 0 et p-1 tel que n^l modulo p est égal à m, si un tel entier existe. Par ailleurs, `./logdiscret n m p` doit renvoyer une erreur lorsque p est un nombre premier supérieur à 2^32-1, lorsque n n'est pas compris entre 1 et p-1, et lorsque m n'est pas compris entre 1 et p-1. Vous utiliserez le type `long long int` pour récupérer les valeurs de n, m, et p. Pour transformer un argument contenant la représentation textuelle d'un entier en `long long int`, vous pourrez utiliser la fonction `atoll`. Pour tester votre fonction, vous pourrez utiliser p=2^31-1 et, dans un premier temps, m=1. Si cela est trop long, vous pourrez utiliser p=555555587.

2. Modifier le programme précédent pour qu'il utilise deux processus différents pour chercher l'entier l. Un des processus pourra par exemple chercher l parmi les entiers pairs uniquement, et l'autre pourra chercher l parmi les entiers impairs. Ce n'est pas grave si deux entiers l différents, chacun écrits par un des deux processus, sont écrits sur la sortie standard.

3. Modifier encore le programme précédent pour qu'il utilise k processus différents. Est-ce que le temps que met ce programme pour afficher le premier entier l qu'il trouve, varie en fonction de k (vous pourrez utiliser les fonctions `time` et `difftime` de <time.h>) ? Commentez.