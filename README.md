# Projet_Systeme

`sudo apt-get install libreadline-dev` permet d'installer la librairie readline  
`-lreadline` dans le makefile pour le linkeur  



## Commands usable with tarballs as directory

You can see all these commands by using `help` command in tsh.  


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
