# Projet_Systeme

## Description du Projet  

Dans le cadre du projet de Système (SY5) du 5ème semestre, nous avons implémenté le shell `tsh`, qui gère chaque fichier tarball comme un dossier.  
`tsh` est capable d’exécuter les commandes suivantes :  
`help` -> donne des informations sur les commandes spéciales que nous avons implémentées (listées ci-dessous).  
`ls [-l] name` -> si `name` est un tarball ou un répertoire alors affiche son contenu direct (à profondeur +1 des sous dossiers comme `ls` classique) sinon si `name` est un fichier affiche son nom. L’option -l donne des informations supplémentaires (comme la taille, les droits, etc...) comme pour `ls` classique. On peut donner plusieurs `name` à `ls` sur la même ligne de commande.  
`cd répertoire` -> permet de se déplacer dans le répertoire demandé ou dans le tarball si `répertoire` en est un.  
`pwd` -> affiche notre position actuelle dans l'arborescence de fichier (avec éventuellement un chemin contenant des tarballs).  
`exit` -> quitte le `tsh`  
`cat ` -> sans argument, recopie STDIN sur STDOUT. Avec argument, affiche le contenu dans d'un fichier.  
`mv [repertoire1] [repertoire2]` -> déplace repertoire1 vers repertoire2.   
`rm [-r] [repertoire]` -> permet la suppression d'un fichier quelconque et également d'un répertoire avec l'option -r  
`rmdir [repertoire]` -> permet la suppression d'un répertoire vide  
`cp [-r] [repertoire] [repertoire]`  -> copie un fichier sous un nom différent et également d'un répertoire si utilisée avec l'option -r  
`mkdir [repertoire]` -> crée un nouveau répertoire vide  
Ainsi que toutes les commandes habituelles lorsqu'aucun tarball n'est en jeu.  
Également `tsh` gère les redirections d'entrée (` < `), de sortie standard (` > ` ou ` >> `) et de sortie erreur (` 2> ` ou ` 2>> `) ainsi que les tubes (`|`).  

## Difficultés rencontrées  

Il existe encore certains problèmes connus, notamment avec la commande `ls` (`ls .` par exemple, affiche actuellement tout le temps le contenu du répertoire a partir duquel
on a lancé le tsh, ou `ls ..` dans un tar ne fonctionne pas).  
Les commandes `pwd`, `cd` et `exit` fonctionnent sans problème. Nous avons aussi débuté la rédaction de tests, afin de vérifier le bon fonctionnement de toutes les commandes.
Le fichier test.c utilise pour le moment la fonction system() pour appeler un script construisant une architecture de fichier, mais sera rapidement remplacé par
un autre système de test.  

De façon générale , quelques difficultés ont été rencontrées dont les plus grandes implique le déplacement et le traitement d'informations au sein d'une   
archive qui aura nécessité beaucoup de temps et de réflexions !

Dans le dossier tests, le script `test.sh` permet de construire une arborescence de fichier pour tester les commandes, et le script `testrm.sh` de la supprimer.  


## Tests unitaires

Avec la commande suivante vous pouvez lancer une vague de tests unitaires afin de vérifier le bon fonctionnement de `tsh` :  
`./test/sh`  

## Test avec un conteneur Docker

### Construire l'image

Vous pouvez télécharger l'image Docker directement depuis le dépôt Docker en utilisant cette commande :  
`docker pull tireur2cables/tsh_img`  
Ensuite vous pouvez soit utiliser la commande suivante pour créer un nouveau tag de cette image :  
`docker tag tireur2cables/tsh_img tsh_img`  
Ou alors vous pouvez remplacer `tsh_img` par `tireur2cables/tsh_img` dans les parties suivantes.  

Vous pouvez également construire l'image Docker vous même en utilisant cette commande dans la racine du dépôt Git :  
`docker build -t tsh_img .`  

### Lancer le conteneur

Vous pouvez lancer le conteneur en mode interactif en utilisant cette commande :  
`docker run -tiw /home/tsh-testing tsh_img`  

### Exécution du projet

Vous pouvez exécuter le `tsh` dans le conteneur que vous venez de lancer en utilisant cette commande :  
`./tsh.sh`  
Cette commande compilera le projet s'il faut puis lance l'exécutable `code/tsh`.  
Maintenant vous pouvez faire tout ce que vous voulez dans `tsh` en lançant vos commandes.  
Dans le répertoire `test` vous trouverez de quoi faire déjà quelques essais avec des tarballs, des dossiers et des fichiers. Cependant vous ne devriez pas aller dans le répertoire `code` ou ses sous répertoires afin de préserver le fonctionnement de `tsh` et de ces tests unitaires.  

### Si vous n'utilisez pas Docker

Si vous n'utilisez pas Docker, vous devrez installer la librairie `readline` que nous utilisons pour la lecture des commandes.  
Vous pouvez les faire sur `Debian` avec la commande :  
`sudo apt-get install libreadline-dev`  
