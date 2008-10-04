
// Fonctions de lecture/ecriture fichier, gèrent les systèmes big-endian et
// little-endian.

#include <SDL/SDL_endian.h>
#include <string.h>
#include "struct.h"
#include "io.h"

word endian_magic16(word x)
{
  #if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return x;
  #else
    return SDL_Swap16(x);
  #endif
}
dword endian_magic32(dword x)
{
  #if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return x;
  #else
    return SDL_Swap32(x);
  #endif
}

// Lit un octet
// Renvoie -1 si OK, 0 en cas d'erreur
int read_byte(FILE *Fichier, byte *Dest)
{
  return fread(Dest, 1, 1, Fichier) == 1;
}
// Ecrit un octet
// Renvoie -1 si OK, 0 en cas d'erreur
int write_byte(FILE *Fichier, byte Byte)
{
  return fwrite(&Byte, 1, 1, Fichier) == 1;
}
// Lit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int read_bytes(FILE *Fichier, void *Dest, size_t Taille)
{
  return fread(Dest, 1, Taille, Fichier) == Taille;
}
// Ecrit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int write_bytes(FILE *Fichier, void *Src, size_t Taille)
{
  return fwrite(Src, 1, Taille, Fichier) == Taille;
}

// Lit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_word_le(FILE *Fichier, word *Dest)
{
  if (fread(Dest, 1, sizeof(word), Fichier) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *Dest = SDL_Swap16(*Dest);
  #endif
  return -1;
}
// Ecrit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_word_le(FILE *Fichier, word Mot)
{
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    Mot = SDL_Swap16(Mot);
  #endif
  return fwrite(&Mot, 1, sizeof(word), Fichier) == sizeof(word);
}
// Lit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_word_be(FILE *Fichier, word *Dest)
{
  if (fread(Dest, 1, sizeof(word), Fichier) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *Dest = SDL_Swap16(*Dest);
  #endif
  return -1;
}
// Ecrit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_word_be(FILE *Fichier, word Mot)
{
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    Mot = SDL_Swap16(Mot);
  #endif
  return fwrite(&Mot, 1, sizeof(word), Fichier) == sizeof(word);
}
// Lit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_dword_le(FILE *Fichier, dword *Dest)
{
  if (fread(Dest, 1, sizeof(dword), Fichier) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *Dest = SDL_Swap32(*Dest);
  #endif
  return -1;
}
// Ecrit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_dword_le(FILE *Fichier, dword Mot)
{
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    Mot = SDL_Swap32(Mot);
  #endif
  return fwrite(&Mot, 1, sizeof(dword), Fichier) == sizeof(dword);
}

// Lit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_dword_be(FILE *Fichier, dword *Dest)
{
  if (fread(Dest, 1, sizeof(dword), Fichier) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *Dest = SDL_Swap32(*Dest);
  #endif
  return -1;
}
// Ecrit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_dword_be(FILE *Fichier, dword Mot)
{
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    Mot = SDL_Swap32(Mot);
  #endif
  return fwrite(&Mot, 1, sizeof(dword), Fichier) == sizeof(dword);
}

// Détermine la position du dernier '/' ou '\\' dans une chaine,
// typiquement pour séparer le nom de fichier d'un chemin.
// Attention, sous Windows, il faut s'attendre aux deux car 
// par exemple un programme lancé sous GDB aura comme argv[0]:
// d:\Data\C\GFX2\grafx2/grafx2.exe
char * Position_dernier_slash(const char * Chaine)
{
  const char * Position = NULL;
  for (; *Chaine != '\0'; Chaine++)
    if (*Chaine == SEPARATEUR_CHEMIN[0]
#ifdef __WIN32__    
     || *Chaine == '/'
#endif
     )
      Position = Chaine;
  return (char *)Position;
}
// Récupère la partie "nom de fichier seul" d'un chemin
void Extraire_nom_fichier(char *Destination, const char *Source)
{
  const char * Position = Position_dernier_slash(Source);

  if (Position)
    strcpy(Destination,Position+1);
  else
    strcpy(Destination,Source);
}
// Récupère la partie "répertoire+/" d'un chemin.
void Extraire_chemin(char *Destination, const char *Source)
{
  char * Position;

  strcpy(Destination,Source);
  Position = Position_dernier_slash(Destination);
  if (Position)
    *(Position+1) = '\0';
  else
    strcat(Destination, SEPARATEUR_CHEMIN);
}

