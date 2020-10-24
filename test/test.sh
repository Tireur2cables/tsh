#! /bin/bash

mkdir tests
mkdir tests/test_rep
echo "ô le joli fichier" > tests/test_fic
ln -s tests/test_fic tests/test_lien
mkfifo tests/test_fifo
touch tests/test_exe1
touch tests/test_exe2
chmod 020 tests/test_exe1
chmod 022 tests/test_exe2
