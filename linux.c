// Diverses fonctions qui existaient sous dos mais pas sous linux...
#ifdef __linux__

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

int filelength(int fichier)
{
	struct stat infos_fichier;
	fstat(fichier,&infos_fichier);
	return infos_fichier.st_size;
}

void itoa(int source,char* dest, int longueur)
{
	snprintf(dest,longueur,"%d",source);
}

#endif
