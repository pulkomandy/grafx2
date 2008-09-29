
// Fonctions de lecture/ecriture fichier, gèrent les systèmes big-endian et
// little-endian.

#include <SDL/SDL_endian.h>
#include "struct.h"

word endian_magic16(word x)
{
  #if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return x;
  #else
    return SDL_Swap16(x);
  #endif
}
word endian_magic32(word x)
{
  #if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return x;
  #else
    return SDL_Swap32(x);
  #endif
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
