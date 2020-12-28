# ARCHITECTURE DU PROJET  

### Lancement du projet

Le script `tsh.sh` permet de lancer l’exécutable `tsh` du dossier `code/`.  


### tsh.c

Le programme principal se situe dans le fichier `tsh.c`.  
Dans ce programme on trouvera une boucle lisant les commandes entrées par l'utilisateur. La librairie `readline` a été utilisée afin de pouvoir gérer toutes les tailles de commandes (utilisation possible indiquée dans README) .
La commande entrée par l'utilisateur est ensuite comparée d'abord aux commandes dites `built-in` ([cf. Commandes built-in](#-Commandes-built-in)) (qui ne seront pas lancées dans un processus fils, par exemple `cd` ou `exit`). Si elle ne correspond pas à l'une de ces commandes elle est ensuite comparée aux autres commandes qui ont été réimplémentées (ces commandes seront lancées dans un processus fils le cas échéant).  
Si l'utilisateur n'a rentré aucune de ces commandes alors sa commande sera exécutée dans un processus fils avec la fonction `exec`.  
Si l'utilisateur rentre une commande et que celle-ci met en jeu l'utilisation d'un fichier `.tar` de quelques façon que ce soit, les arguments de cette commande sont 'traités'. Ce traitement transforme le caractère spécial `~` en la valeur de `$HOME` et gère les caractères `.` et `..` en transformant le chemin en chemin absolu et le modifiant ou non (suivant si le caractère rencontré et `..` ou `.`). Ce choix permet de simplifier au maximum le chemin entré par l'utilisateur, les commandes s'exécuteront alors plus rapidement que si l'utilisateur avait emprunté des 'détours' pour désigner un chemin plus simple. En outre il est tout à fait possible d'entrer un chemin comprenant des dossiers inexistants s'il y a assez de caractères `..` pour en sortir.  


### Implémentations des commandes spéciales

Chaque commande nécessitant d'être implémentée pour pouvoir gérer de façon spécifique les fichier `.tar` sont dans un fichier à part du main (`tsh.c`).  
Chacune de ces commandes est implémentée comme si elle était une fonction main (renvoit un entier et prend en arguments `argc`, nombre d'éléments dans `argv`, et `argv`, tableau de string finissant par un pointeur `NULL`).  
Le fichier `tar.h` contient les structures nécéssaires à la gestion des fichiers `.tar`.  


##### Implémentations de cd & ls :  
Ces fonctions différencient le travail dans un repertoire classique, du travail dans une archive tar. Ainsi on effectue un parcours récursif du chemin passé en   
paramètre en vérifiant si on est dans une archive ou non. Ce parcours permet de séparer les différents cas que l'on peut rencontrer, c'est à dire :
 - Archive simple : cd archive.tar  
 - Repertoire simple : cd dossier  
 - Dossier dans une archive : cd archive.tar/dossier  

Plus simplement parlant : trouver le dossier suivant dans le chemin, vérifier qu'il est accessible, puis l'ouvrir / l'afficher.


### Gestion du working directory

Comme les fonctions `chdir` ou `getcwd` ne prennent pas en compte un chemin contenant un fichier `.tar` nous utilisons une variable différente de `PWD`, à savoir `TWD`. Cette variable est récupérée à l'aide de `getenv` et mise à jour à l'aide de `setenv`.  


### Commandes built-in

Certaines commandes nécessitent, au moins par soucis de simplicité d'implémentation, d'être appelées dans le processus principal (celui contenant le main) et non dans un processus fils.  
En effet voici les commandes qui sont `built-in` dans `tsh` :  
- exit : Le processus principal est à priori le mieux placé pour mettre fin à son execution.  
- cd : Afin de mettre à jour son working directory ([cf. Gestion du working directory](#-Gestion-du-working-directory)), cd doit pouvoir modifier le working directory du processus principal (de `tsh`) , or un processus fils ne peut pas modifier lui-même le working directory de son père.  
