#! /bin/bash

mkdir ../test/tests
mkdir ../test/tests/rep1
mkdir ../test/tests/rep1/rep2
mkdir ../test/tests/rep1/rep2/rep3
mkdir ../test/tests/rep1/rep2/rep3/rep4
mkdir ../test/tests/rep6
mkdir ../test/tests/rep7
echo "fichier 1" > ../test/tests/fic1
echo "Fichier 2" > ../test/tests/fic2
echo "Fichier 3" > ../test/tests/rep1/fic3
echo "Fichier 4" > ../test/tests/rep1/fic4
echo "Fichier avec extension 1" > ../test/tests/rep1/ext.txt
echo "Fichier avec extension 2" > ../test/tests/rep1/tar.txt
echo "Fichier avec extension 3" > ../test/tests/rep1/tar.g
echo "Fichier 5" > ../test/tests/rep1/fic5
echo "Fichier 6" > ../test/tests/rep1/rep2/fic6
echo "Fichier 7" > ../test/tests/rep1/rep2/rep3/rep4/fic5
#tar cf arch.tar tests/ tests/rep1 tests/rep1/rep2 tests/rep1/rep2/rep3 tests/rep1/rep2/fic6 tests/rep1/rep2/rep3/rep4/fic5 tests/fic1 tests/fic2 tests/rep1/fic3 tests/rep1/fic4 tests/rep1/ext.txt tests/rep1/tar.txt tests/rep1/tar.g
tar cf arch.tar ../test/tests/
mv arch.tar ../test/
ln -s ../test/tests/test_fic ../test/tests/test_lien
mkfifo ../test/tests/test_fifo
touch ../test/tests/test_exe1
touch ../test/tests/test_exe2
chmod 220 ../test/tests/test_exe1
chmod 222 ../test/tests/test_exe2
