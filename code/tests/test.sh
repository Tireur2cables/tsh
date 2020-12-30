#! /bin/bash

mkdir tests/tests
mkdir tests/tests/rep1
mkdir tests/tests/rep1/rep2
mkdir tests/tests/rep1/rep2/rep3
mkdir tests/tests/rep1/rep2/rep3/rep4
mkdir tests/tests/rep6
mkdir tests/tests/rep7
echo "fichier 1" > tests/tests/fic1
echo "Fichier 2" > tests/tests/fic2
echo "Fichier 3" > tests/tests/rep1/fic3
echo "Fichier 4" > tests/tests/rep1/fic4
echo "Fichier avec extension 1" > tests/tests/rep1/ext.txt
echo "Fichier avec extension 2" > tests/tests/rep1/tar.txt
echo "Fichier avec extension 3" > tests/tests/rep1/tar.g
echo "Fichier 5" > tests/tests/rep1/fic5
echo "Fichier 6" > tests/tests/rep1/rep2/fic6
echo "Fichier 7" > tests/tests/rep1/rep2/rep3/rep4/fic5
tar cf arch.tar tests/
mv arch.tar tests/
ln -s tests/tests/test_fic tests/tests/test_lien
mkfifo tests/tests/test_fifo
touch tests/tests/test_exe1
touch tests/tests/test_exe2
