void _splitpath(char* Buffer, char* Chemin, char* Nom_Fichier);
/* Sépare dans la chaîne passée dans Buffer le chemin d'accès du nom de fichier */

struct find_t {
  unsigned char attrib;
  char name[256];
};

int filelength(int);

void itoa(int source,char* dest, int longueur);
/* Integer TO Ascii */

#define _A_SUBDIR 1
