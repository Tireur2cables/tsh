#!/bin/bash

#Lancement du programme tests unitaires

# Vérification de l'existence de l'executable issu de la compilation par make
if [ -f code/test ]
then # Lance le programme de tests unitaires
	./code/test
else # Compile le programme avant de le lancer
	make -C code/
	./code/test
fi
