#include <string.h>
#include <stdio.h>

void _splitpath(char* Buffer, char* Chemin, char* Nom_Fichier)
{
    int i=0, Position_Du_Dernier_Slash=0;
    do
    {
        Chemin[i]=Buffer[i];
        if (Buffer[i]=='/')
            Position_Du_Dernier_Slash=i;
        i++;
    }while (Buffer[i]!=0);

    i=Position_Du_Dernier_Slash+1;
    Chemin[i]=0;
    strcpy(Nom_Fichier,Buffer+i);
}

int filelength(int fichier)
{
	printf("filelenght non implémenté!\n");
	return 0;
}

void itoa(int source,char* dest, int longueur)
{
	snprintf(dest,longueur,"%d",source);
}
