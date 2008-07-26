// Affcihenom fichier, n° ligne, nom fonction, une chaine et un entier.
#define DEBUG(y,z) printf("%s %d %s | %s : %d###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

// DEBUG en hexadécimal
#define DEBUGX(y,z) printf("%s %d %s | %s : %X###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

#define UNIMPLEMENTED printf("%s %d %s non implémenté !\n",__FILE__,__LINE__,__func__);

#define UNTESTED printf("%s %d %s à tester !\n",__FILE__,__LINE__,__func__);

void Erreur(int);

