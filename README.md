# Projet_Systeme

`sudo apt-get install libreadline-dev` permet d'installer la librairie readline  
`-lreadline` dans le makefile pour le linkeur  



## Commands usable with tarballs as directory

You can see all these commands by using `help` command in tsh.  


## Test with Docker container

### Build image

You can build the docker image by using this command in the root folder of the repo :  
`docker build -t tsh:testing .`  

### Launch container

You can launch tsh project on a container by using this command :  
`docker run -tiw /home/tsh-testing tsh:testing`  

### Compile Project

You can compile tsh projet by using this command in the container :  
`make`

### Run Project

You can run tsh project in the container by using this command :  
`./tsh`  
Then feel free to test all the commands you want.  
