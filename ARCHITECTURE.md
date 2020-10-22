# ARCHITECTURE DU PROJET  

### Lancement du projet

Le script `tsh.sh` permet de lancer l'executable `tsh` du dossier `code/`.  

### tsh.c

Le programme principal se situe dans le ficier `tsh.c`.  
Dans ce programme on trouvera une boucle lisant les commandes entrées par l'utilisateur. La librairie `readline` a été utilisée afin de pouvoir gérer toutes les tailles de commandes.  
La commande entrée par l'utilisateur est ensuite comparée d'abord aux commandes dites `built-in` (qui ne seront pas lancées dans un processus fils, par exemple `cd` ou `exit`). Si elle ne correspond pas à l'une de ces commandes elle est ensuite comparée aux autres commandes qui ont été réimplémentées (ces commandes seront lancées dans un processus fils le cas échéant).  
Si l'utilisateur n'a rentré aucune de ces commandes alors sa commande sera éxécutée dans un processus fils avec la fonction `exec`.  

### Implémentations des commandes spéciales

Chaque commande nécéssitant d'être implémentée pour pouvoir gérer de façon spécifique les fichier `.tar` sont dans un fichier à part du main (`tsh.c`).  
Chacune de ces commandes est implémentée comme si elle était une fonction main (renvoit un entier et prend en arguments `argc`, nombre d'éléments dans `argv`, et `argv`, tableau de string finissant par un pointeur NULL).  
Le fichier `tar.h` contient les structures nécéssaires à la gestion des fichier `.tar`.  

### Gestion du working directory

Comme les fonction `chdir` ou `getcwd` ne prennent pas en compte un chemin contenant un fichier `.tar` nous utilisons une variable différente de `PWD`, à savoir `TWD`. Cette variable est récupérée à l'aide de `getenv` et mise à jour à l'aide de `putenv`.  


### Commandes built-in

Certaines commandes nécéssitent, au moins par soucis de simplicité d'implémentation, d'être appelées dans le processus principal (celui contenant le main) et non dans un processus fils.  
En effet voici les commandes qui sont `built-in` dans `tsh` :  
- exit : Le processus principal est en théorie le mieux placé pour mettre fin à son execution.  
- cd : Afin de mettre à jour son working directory ([cf. Gestion du working directory](#-Gestion-du-working-directory)), cd doit pouvoir modifier le working directory du processus principal (de `tsh`) or un processus fils ne peut pas modifier lui même le working directory de son père.  
