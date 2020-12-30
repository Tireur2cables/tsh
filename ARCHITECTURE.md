# ARCHITECTURE DU PROJET  

### Lancement du projet

Le script `tsh.sh` permet de lancer l’exécutable `tsh` du dossier `code/`.  


### tsh.c

Le programme principal se situe dans le fichier `tsh.c`.  
Dans ce programme on trouvera une boucle lisant les commandes entrées par l'utilisateur. La librairie `readline` a été utilisée afin de pouvoir gérer toutes les tailles de commandes (utilisation possible indiquée dans README) .
La commande entrée par l'utilisateur est ensuite comparée d'abord aux commandes dites `built-in` ([cf. Commandes built-in](#-Commandes-built-in)) (qui ne seront pas lancées dans un processus fils, par exemple `cd` ou `exit`).
Si elle ne correspond pas à l'une de ces commandes elle est ensuite comparée aux autres commandes qui ont été réimplémentées (ces commandes seront lancées dans un processus fils le cas échéant).  
Si l'utilisateur n'a rentré aucune de ces commandes alors sa commande sera exécutée dans un processus fils avec la fonction `exec`.  
Si l'utilisateur rentre une commande et que celle-ci met en jeu l'utilisation d'un fichier `.tar` de quelques façon que ce soit, les arguments de cette commande sont 'traités'.
Ce traitement transforme le caractère spécial `~` en la valeur de `$HOME` et gère les caractères `.` et `..` en transformant le chemin en chemin absolu et le modifiant ou non (suivant si le caractère rencontré et `..` ou `.`).
Ce choix permet de simplifier au maximum le chemin entré par l'utilisateur, les commandes s'exécuteront alors plus rapidement que
si l'utilisateur avait emprunté des 'détours' pour désigner un chemin plus simple. En outre il est tout à fait possible d'entrer un chemin comprenant des dossiers inexistants
s'il y a assez de caractères `..` pour en sortir. C'est un choix de design que nous avons fait, qui diffère de comment fonctionne `bash` par exemple.
Nous avons fait le choix d'inclure directemnt toutes les commandes que nous avons ré-implémentés dans le tsh.

### Implémentations des commandes spéciales

Chaque commande nécessitant d'être implémentée pour pouvoir gérer de façon spécifique les fichier `.tar` sont dans un fichier à part du main (`tsh.c`).  
Chacune de ces commandes est implémentée comme si elle était une fonction main (renvoit un entier et prend en arguments `argc`, nombre d'éléments dans `argv`, et `argv`,
tableau de string finissant par un pointeur `NULL`).  
Le fichier `tar.h` contient les structures nécéssaires à la gestion des fichiers `.tar`.  
Lors de l'appelle d'une commande, le tsh appelle les fonctions externes s'il est évident qu'il n'y a pas de tar en jeu. Cependant, il existe des situations
ou l'on veut executer une commande externe, même si il y a un tar dans le chemin. exemple :  
`cd archive.tar`, puis `ls ..`. Dans ce cas, c'est la commande `ls` que nous avons implémentée qui se charge d'appeler la commande externe ls.   


##### Implémentations de cd :  
Ces fonctions différencient le travail dans un repertoire classique, du travail dans une archive tar. Ainsi on effectue un parcours récursif du chemin passé en   
paramètre en vérifiant si on est dans une archive ou non. Ce parcours permet de séparer les différents cas que l'on peut rencontrer, c'est à dire :
 - Archive simple : cd archive.tar  
 - Repertoire simple : cd dossier  
 - Dossier dans une archive : cd archive.tar/dossier  



##### Implémentations de rm & rmdir :  
A l'instar de ls , l'implémentation de rm et de rmdir implique premièrement l'analyse du chemin passé en argument dans la fonction principale . Soit le chemin  
ne contient pas d'archive auquel cas la commande externe est invoquée ou bien le chemin en possède et dans ce cas on procède à la suppression du chemin  
postérieur à l'archive selon la présence d'option(s). En fin d'analyse ou en début de suppression , les cas possibles sont donc :  
- tar indiqué en fin chemin : rmdir tar , rm -r archive.tar  
- fichier quelconque indiqué en fin de chemin : rm (-r) archive.tar/../fichier  
- dossier indiqué en fin de chemin : rmdir tar/../dossier , rm -r archive.tar/../dossier  

En cas d'utilisation de rm , la suppression d'un ou plusieurs répertoires ne se fera qu'en cas d'indication de l'option -r ou -R  
On vérifiera si l'option est bien indiqué en début de fonction principale. Pour rmdir , bien sûr , la suppression d'un répertoire se fera à  
condition que celui-ci soit vide. Cette condition est vérifiée dans rmdir lors de la tentative de suppression après vérification.  



##### Implémentation de mkdir :  
On procède d'abord à l'analyse du chemin passé en argument dans la fonction principale puis à la tentative de création d'un répertoire. Selon les cas  
rencontrés , la fonction `try_create_dir` appelée depuis la fonction principale appellera une fonction particulière :   
- création d'une archive --> appel de create_tar  
- création d'un dossier dans une archive --> appel de create_dir  
- création dans un tar dont le chemin postérieur ne contenant pas d'archive --> appel de la commande externe  

La création d'une archive ou d'un répertoire ne se fait que si celle/celui-ci n'existe pas déjà , dans le cas contraire , un nouveau bloc d'entête est  
créé dans une fonction à part selon le format classque du header d'une archive.  




##### Implémentation de mv :  
Cette fonction vérifie si l'on rentre bien au moins 2 arguments/chemins , puis implique l'utilisation de rm et cp pour les cas suivants :  
- déplacement d'une archive / dossier  
- changement du nom du dossier / header de l'archive si pas de nouveaux chemins indiqués après  

Avant chaque déplacement ou changement de header , le dossier / l'archive est copié suite à l'appel de la fonction cp puis supprimé avec rm




##### Implémentation de cat :  
La fonction permet l'affichage du contenu de fichiers dans une archive ou non. Si l'on tente d'afficher un fichier présent dans une archive , on vérifie  
sa présence dans le chemin indiqué , le cas échéant  puis on procède à son affichage (assez similaire à ls mais en rajoutant un parcours supplémentaire dans l'archive) dans  
la fonction `cat_tar` , 
  
  
  
##### Implémentation de exit :  
Implémentée en une fonction principale permettant simplement de quitter le tsh si aucun autre argument n'est indiqué.  

  
  
##### Implémentation de help :  
A l'instar de exit , cette commande est implémentée en une fonction principale qui ne prend qu'un argument et qui affiche toutes les autres commandes
implémentées suivi de leur description.



##### Implémentation de cp (-r) :  
On vérifie d'abord si un bon nombre d'arguments est donné puis on indique la présence d'une option éventuelle. Et de façon similaire à l'implémentation  
des autres commandes , on effectue un parcours pour vérifier l'existence des chemins indiqués en arguments contenant une archive ou non (`isCorrectDest`  
et `exist`). Avant la suppression de fichiers ou répertoire on procède donc a des vérifications auxiliaires :  
- si le chemin existe y compris dans une archive -> exist()  
- si le chemin absolu existe à l'extérieur d'une archive -> existAbsolut()  
- si le chemin existe dand une archive ou si le chemin est un tar -> existInTar()  

Enfin , si les chemins sont corrects , on effectue la copie des fichers ou des répertoires dans l'archive si présence d'option -r en parcourant de manière  
récursive si le chemin est un répertoire.  

#### Implémentation de ls
`ls` peut etre utilisé avec l'option `-l` pour afficher des informations détaillées sur les fichiers. Elle vérifie donc dans un premier temps la présence de
l'option `-l`, puis parcours la liste des fichiers qu'on lui à donné en argument. Si elle doit afficher un fichier en dehors d'un tar, elle appelle simplement la 
commande `ls` externe. Si on veut lister le contenu d'un tar, ou d'un dossier dans un tar, elle parcourt le tar, en calculant la `profondeur` de chaque fichier, 
via le nombre de `/` dans leur nom.  
Elle n'affiche donc que le contenu à profondeur `n+1` dans le tar.  
Pour l'option `-l`, on calcul avant d'afficher les fichiers, le nombre de liens de ceux-ci, car cette information n'est pas stockée dans le header. On parcourt donc
une première fois le tar, afin de chercher les liens des fichiers avec leurs père.  

#### Implémentation des redirections et des tubes  
Le tsh supporte aussi les redirections et les tubes entre plusieurs commandes, sous certaines limitations. On peut rediriger l'entrée, la sortie et la sortie erreur
des commandes avec `>`,`>>`,`2>`,`2>>` et `<`. Nous n'acceptons que le cas ou il y a des espaces entre les commandes et les chevrons.  
On effectue donc une redirection classique (avec simplement l'ouverture et `dup`) s'il n'y a pas de tar en jeu. Dans l'autre cas, on distingue 3 cas :  
- Si le fichier n'existe pas dans le tar : On se positionne à la fin de l'archive, on laisse un place pour écrire le header par la suite, puis on écrit de contenu de la commande.
- Si le fichier existe dans le tar et qu'on utilise `>` : On supprime le fichier dans le tar, puis on en créé un nouveau à la fin de l'archive.
- Si le fichier existe dans le tar et qu'on utilise `>>` : On trouve le fichier dans le tar, copie son contenu a la fin du tar, en déplaçant tout le contenu du tar vers le haut, puis on écrit la sortie de la commande.  
Finalement, il ne nous reste plus qu'a calculer la taille du contenu du fichier qu'on à créé, et ecrire le header à la bonne position (qu'on avais laissé blanche).  

Nous ne traitons pas le cas si deux redirections vont dans le même fichier, ou des fichiers différents, mais dans le même tar. De plus, on ne garantit pas le comportement 
de l'écriture, si une commande utilise en argument un fichier qui est aussi utilisé pour une redirection. Par exemple `cat archive.tar/fichier > archive.tar/autrefichier` 
a un comportement que nous ne pouvons pas prévoir.  
Dans le cas ou il y aurait plusieurs redirections du même descripteur, on affiche une erreur.  
Les tubes fonctionnent comme les tubes de `bash`, ils redirigent la sortie de chaque commande sur l'entrée de la suivante. Nous avons cependant pu remarquer un comportement différent
de celui de bash ou zsh, pour la suite de commande `cat | ls`, qui met `cat` en attente de lecture, puis affiche `ls` quand `cat` est fini.  

#### Tests unitaires
Le programme `test`, compilé en même temps que le `tsh`, permet de faire quelques tests sur certaines fonctions du programmes (principalement `ls` et `cd`). On passe par
un tableau de pointeurs de fonctions, qu'on peut appeler avec une boucle. On redirige l'affichage ou l'erreur des fonctions dans un fichier, qu'on peut comparer avec aisement
avec ce qu'on attend habituellement des fonctions, et donc détecter si elles agissent de façon anormale.  

-lreadline dans le makefile pour le linkeur


### Gestion du working directory

Comme les fonctions `chdir` ou `getcwd` ne prennent pas en compte un chemin contenant un fichier `.tar` nous utilisons une variable différente de `PWD`, à savoir `TWD`. Cette variable est récupérée à l'aide de `getenv` et mise à jour à l'aide de `setenv`.  


### Commandes built-in

Certaines commandes nécessitent, au moins par soucis de simplicité d'implémentation, d'être appelées dans le processus principal (celui contenant le main) et non dans un processus fils.  
En effet voici les commandes qui sont `built-in` dans `tsh` :  
- exit : Le processus principal est à priori le mieux placé pour mettre fin à son execution.  
- cd : Afin de mettre à jour son working directory ([cf. Gestion du working directory](#-Gestion-du-working-directory)), cd doit pouvoir modifier le working directory du processus principal (de `tsh`) , or un processus fils ne peut pas modifier lui-même le working directory de son père.  
