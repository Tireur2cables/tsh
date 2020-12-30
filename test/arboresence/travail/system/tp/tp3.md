TP nº3 : duplication de descripteurs
=====================

**L3 Informatique - Système**


Ce (tout petit) TP consacré à la duplication de descripteurs n'a pas
vocation à vous occuper 2 heures. Quand vous aurez fini, retour vers le
[TP nº2](../TP2/tp2.md)!

**Dans la suite, toutes les lectures et écritures devront être effectuées
sur l'entrée et la sortie standard** -- après redirection éventuelle de
celles-ci. En particulier, les recopies de l'entrée sur la sortie se
feront exclusivement à l'aide de la fonction `mycat()` fournie dans
[mycat.c](mycat.c).


### Exercice 1 : discute avec ton voisin

Le but de cet exercice est de pouvoir tranquillement écrire des messages
sur le terminal d'un camarade (et réciproquement).

Pour cela, vous devez simplement écrire un programme `mytalk` tel que
`mytalk fic` recopie son entrée standard sur `fic` **par un appel à la
fonction `mycat()`**.  Le comportement doit donc être similaire à `cat >
fic`.

##### Exemple d'utilisation :

   - ouvrir deux terminaux,
   - déterminer la référence absolue du 1er terminal à l'aide de la
     commande `tty`,
   - depuis le 2ème terminal,  exécuter `mytalk` avec en paramètre la 
     référence du premier terminal.

##### Utilisation un poil plus poussée :

   - se connecter par `ssh` à la même machine qu'un autre étudiant,
   - déterminer un terminal de connexion de chacun,
   - s'assurer que les permissions nécessaires sont accordées,
   - discuter à l'aide de `mytalk` (pas trop longtemps tout de même).

[Un petit memento `ssh`](../../ssh.md) au cas où!


### Exercice 2 : copie de fichiers

Écrire un programme `mycp` tel que `mycp fic1 fic2` recopie le contenu de
`fic1` dans `fic2` -- toujours par un appel à `mycat()` : le comportement
doit donc être similaire à `cat < fic1 > fic2`.


### Exercice 3 : un shell particulièrement limité

Écrire un programme ayant le comportement suivant : comme un shell, il
affiche en boucle une invite de commande (par exemple `"Qu'y a-t-il pour
votre service, mon bon maître ?"`) puis attend que l'utilisateur saisisse
une commande. Il exécute ensuite cette commande, puis recommence. Mais ce
shell un peu minable est incapable d'exécuter la moindre commande
externe, et ne comprend que deux commandes internes :

   - `ecris_dans`, qui prend en paramètre une référence de fichier dans
     lequel immortaliser vos pensées profondes,
   - `tu_peux_disposer`, qui termine l'exécution (après une respectueuse
     révérence tout de même).

Pour toute autre demande, il répond simplement `"quèsaco ?"`.
La commande interne `ecris_dans` doit utiliser la fonction `mycat()` pour 
recopier l'entrée standard dans le fichier en paramètre. Mais attention,
le shell doit pouvoir ensuite rétablir la sortie standard initiale pour
réafficher son invite de commande!

