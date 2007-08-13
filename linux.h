void _splitpath(char* Buffer, char* Chemin, char* Nom_Fichier);
/* Sépare dans la chaîne passée dans Buffer le chemin d'accès du nom de fichier */

struct find_t {
  unsigned char attrib;
  char name[256];
};

int filelength(char *);

#define _A_SUBDIR 1
