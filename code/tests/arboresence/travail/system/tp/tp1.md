TP nº1 : archives tar
=====================

**L3 Informatique - Système**

L'objectif de ce TP est de programmer un outil permettant d'obtenir certaines informations d'archives `tar`.

On rappelle que `tar` est un outil d'archivage, permettant par exemple les usages suivants :

```bash
tar cf test.tar fichier1 ... fichierN      # création d'une archive test.tar
tar tvf test.tar                           # liste des fichiers contenus dans test.tar
tar xf test.tar                            # extraction des fichiers présents dans test.tar
```

Note : par défaut, une archive `tar` n'est pas compressée, mais il suffit d'utiliser dessus un outil de compression tel que `gzip` pour obtenir des archives compressées `*.tar.gz`, souvent aussi nommées `*.tgz`. L'outil `tar` peut directement faire cela, voir par exemple `tar czf`. Dans ce TP nous ne travaillerons qu'avec des archives `tar` *non compressées*.

#### Le format des archives tar

Il existe plusieurs variantes de `tar`, elles-mêmes avec un certain nombre d'extensions possibles. Ce qui suit devrait suffire à pouvoir lire la plupart des fichiers `.tar`, et à écrire des `.tar` acceptés par la plupart des utilitaires `tar`.

Un fichier `tar` est une suite de blocs de 512 octets. S'il représente une archive des fichiers `f1`, ..., `fn`, alors ce fichier `tar` comporte, dans l'ordre :
 
  - un bloc entête pour `f1`
  - les blocs correspondant au contenu de `f1`
  - ...
  - un bloc entête pour `fn`
  - les blocs correspondant au contenu de `fn`
  - deux blocs finaux formés uniquement de zéros

Si la taille d'un des fichiers archivés `fi` n'est pas un multiple de 512, alors le dernier bloc concernant `fi` est complété avec des octets nuls `'\0'` à hauteur de 512 octets. 

Un bloc entête a une structure décrite par le type `struct posix_header` dans le fichier [tar.h](tar.h) fourni. Notez que cette structure fait exactement 512 octets de long (macro `BLOCKSIZE`), afin de correspondre exactement à la lecture (ou à l'écriture) d'un bloc. Voici quelques mots sur les principaux champs, les autres pouvant être ignorés ici :

  - `char name[100]` : nom long du fichier (_ie_ sa référence relative au point d'archivage). On supposera ici que 100 caractères suffisent pour stocker ce nom. Les caractères inutilisés seront mis à `'\0'`.
  
  - `char mode[8]` : permissions du fichier, converties en entier. Comme pour tous les autres champs numériques de l'entête, le dernier caractère est un `'\0'`, et les autres des caractères-chiffres entre `'0'` et `'7'` représentant un entier en octal. Comme vous ne savez pas encore manipuler les droits d'un fichier, ce champ sera ignoré à la lecture, et à l'écriture il sera rempli arbitrairement par `sprintf(hd.mode,"0000700")`.
  
  - `char size[12]` : taille du fichier. Même remarque que précédemment concernant le codage de ce nombre, mais cette fois sur 12 caractères au lieu de 8. La lecture pourra se faire par `sscanf(hd.size,"%o",...)` et l'écriture par `sprintf(hd.size,"%011o",...)`.
  
  - `char chksum[8]` : empreinte ("checksum") de ce bloc entête. À la lecture, vous pouvez l'ignorer. En revanche, pour fabriquer un `tar` acceptable par GNU `tar` ce champ doit être correct. Pour cela, utiliser la fonction fournie `set_checksum()` de `tar.h` une fois que votre entête est prête. Pour plus de détail, voir le commentaire devant `set_checksum()`.

  - `char typeflag` : il vaut `'0'` pour un fichier standard et `'5'`pour un répertoire. À l'extraction, il faudra donc ignorer les fichiers ayant un autre type.
 
 - `char magic[6]` : pour le format de `tar` que l'on utilise ici, ce champ devra être mis à `"ustar"` (vous pouvez utiliser la macro `TMAGIC` définie dans `tar.h` et valant `"ustar"`), et le champ suivant `version` être à `"00"` (sans terminateur).


#### Exercice 1 : lire un entête

Écrire un programme `premier_entete` tel que l'appel `premier_entete toto.tar` affiche le nom du fichier décrit par le premier entête de `toto.tar`, son type (fichier ordinaire, répertoire ou autre) et sa taille.  Plus précisément, ce programme doit :

* ouvrir en lecture le fichier `tar` dont le nom est passé en argument;
* lire, en une fois, les 512 octets du premier bloc entête;
* interpréter les informations contenues dans l'entête et réaliser l'affichage demandé.


#### Exercice 2 : type d'un fichier particulier

Écrire un programme `listar` tel que l'appel à `listar toto.tar chemin` affiche le chemin si le fichier de nom `chemin` existe dans l'archive `toto.tar`, ainsi que son type. Sinon, il affiche un message d'erreur. Pour cela, utiliser un tampon de 512 octets pour lire les blocs les uns après les autres.

Approche conseillée : modifier le programme de l'exercice précédent pour boucler sur les entêtes, jusqu'à tomber sur le bon entête ou un bloc rempli de '\0'.

Pour vous déplacer d'entête en entête dans le fichier `tar`, vous pouvez remarquer que le nombre de blocs occupés par un fichier de taille `filesize` est la partie entière _supérieure_ de la division de `filesize` par 512, c'est-à-dire `(filesize + 512 - 1) / 512`, ou encore `(filesize + 512 - 1) >> 9` puisque 512 = 2^9, ou `(filesize + BLOCKSIZE - 1) >> BLOCKBITS` avec les macros définies dans `tar.h`.


#### Exercice 3 : extraction d'un fichier particulier

Écrire un programme  `detar` tel que l'appel à  `detar toto.tar fic1 ... ficn` extraie de l'archive uniquement les fichiers dont les noms ont été passés en paramètre. On supposera qu'aucune création de répertoires n'est nécessaire.

Attention! L'extraction d'une archive peut écraser des fichiers déjà présents. De plus vos premiers essais d'extracteur de `tar` peuvent mal se comporter (par exemple créer des millions de fichiers aux noms cabalistiques). Il est donc *impératif* de faire vos essais dans un sous-répertoire de test ne contenant rien d'important, pas même vos fichiers C. Se placer dans `/tmp` est un bon choix.

#### Exercice 4 : création d'une archive simple

Écrire un programme `mktar` tel que l'appel à `mktar archive.tar fic1 ... ficn` crée l'archive `archive.tar` contenant les fichiers ordinaires `fic1` ... `ficn` (situés dans le répertoire courant). Ce fichier `archive.tar` devra pouvoir être lu avec l'utilitaire GNU `tar`.

Pour déterminer la taille d'un fichier, une solution consiste à utiliser les fonctions `fseek()` (pour se placer à la fin du fichier) et `ftell()`.


#### NB : lien avec le projet
Ce que vous avez écrit lors de ce TP pourra vous servir pour le projet de cette année. Toutefois, faites bien attention : le projet devra être écrit en utilisant les **fonctions d'entrée-sortie bas niveau**.
