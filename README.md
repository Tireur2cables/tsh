# Projet_Systeme

## Commands usable with tarballs as directory  
  
Nous avons implémenté le shell, `./tsh`, capable d'executer les commandes suivantes :  
`help` -> donne des informations sur les commandes disponibles  
`ls [-l] [repertoire]` -> affiche le contenu du repertoire, l'options -l donne des informations supplémentaires  
`cd [repertoire]` -> permet de se déplacer dans un repertoire particulier  
`exit` -> quitte le tsh  

Il existe encore certains problèmes connu, notamment avec la commande ls (ls . par exemple, affiche actuellement tout le temps le contenu du repertoire a partir duquel
on a lancé le tsh, ou ls .. dans un tar ne fonctionne pas)  
  

## Test with Docker container

### Build image

You can pull the image directly from the docker repo by using this command :  
`docker pull tireur2cables/tsh_img`  
Then you can either use the following command to create a new tag for this image :  
`docker tag tireur2cables/tsh_img tsh_img`  
Or on you can replace `tsh_img` by `tireur2cables/tsh_img` on the following parts.  

You can also build the docker image yourself by using this command in the root folder of the repo :  
`docker build -t tsh_img .`  

### Launch container

You can launch tsh project on a container by using this command :  
`docker run -tiw /home/tsh-testing tsh_img`  

### Compile Project

Note that you should ignore this step if you pull directly tsh_img from docker repo as the image comes with a compile project.  
You can compile tsh projet by using this command in the container (in `code` directory) :  
`make`

### Run Project

You can run tsh project in the container by using this command on the root folder of the repo :  
`./tsh.sh`  
Then feel free to test all the commands you want.  
On `test` directory you will find some tar and classic files and directories.  
