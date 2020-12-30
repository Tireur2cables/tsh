#! /bin/bash

mkdir tests/rep1
mkdir tests/rep1/rep2
mkdir tests/rep1/rep2/rep3
mkdir tests/rep1/rep2/rep3/rep4
mkdir tests/rep6
mkdir tests/rep7
echo "fichier 1" > tests/fic1
echo "Fichier 2" > tests/fic2
echo "Fichier 3" > tests/rep1/fic3
echo "Fichier 4" > tests/rep1/fic4
echo "Fichier avec extension 1" > tests/rep1/ext.txt
echo "Fichier avec extension 2" > tests/rep1/tar.txt
echo "Fichier avec extension 3" > tests/rep1/tar.g
echo "Fichier 5" > tests/rep1/fic5
echo "Fichier 6" > tests/rep1/rep2/fic6
echo "Fichier 7" > tests/rep1/rep2/rep3/rep4/fic5
tar cf arch.tar tests/
mv arch.tar tests/
ln -s tests/test_fic tests/test_lien
mkfifo tests/test_fifo
touch tests/test_exe1
touch tests/test_exe2
chmod 220 tests/test_exe1
chmod 222 tests/test_exe2
