#!/bin/bash

#Lancement du programme tsh

# Vérification de l'existence de l'executable issu de la compilation par make
if [ -f code/tsh ]
then # Lance le programme
	./code/tsh
else # Compile le programme avant de le lancer
	make -C code/
	./code/tsh
fi
