/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2007 Adrien Destugues
    Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Fonctions de lecture/ecriture file, gèrent les systèmes big-endian et
// little-endian.

#include <SDL_endian.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#if defined(__amigaos4__)
    #include <proto/dos.h>
    #include <dirent.h>
#elif defined(__WIN32__)
    #include <dirent.h>
    #include <windows.h>
#else
    #include <dirent.h>
#endif

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
int read_byte(FILE *file, byte *Dest)
{
  return fread(Dest, 1, 1, file) == 1;
}
// Ecrit un octet
// Renvoie -1 si OK, 0 en cas d'erreur
int write_byte(FILE *file, byte b)
{
  return fwrite(&b, 1, 1, file) == 1;
}
// Lit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int read_bytes(FILE *file, void *Dest, size_t size)
{
  return fread(Dest, 1, size, file) == size;
}
// Ecrit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int write_bytes(FILE *file, void *Src, size_t size)
{
  return fwrite(Src, 1, size, file) == size;
}

// Lit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_word_le(FILE *file, word *Dest)
{
  if (fread(Dest, 1, sizeof(word), file) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *Dest = SDL_Swap16(*Dest);
  #endif
  return -1;
}
// Ecrit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_word_le(FILE *file, word w)
{
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    w = SDL_Swap16(w);
  #endif
  return fwrite(&w, 1, sizeof(word), file) == sizeof(word);
}
// Lit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_word_be(FILE *file, word *Dest)
{
  if (fread(Dest, 1, sizeof(word), file) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *Dest = SDL_Swap16(*Dest);
  #endif
  return -1;
}
// Ecrit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_word_be(FILE *file, word w)
{
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    w = SDL_Swap16(w);
  #endif
  return fwrite(&w, 1, sizeof(word), file) == sizeof(word);
}
// Lit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_dword_le(FILE *file, dword *Dest)
{
  if (fread(Dest, 1, sizeof(dword), file) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *Dest = SDL_Swap32(*Dest);
  #endif
  return -1;
}
// Ecrit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_dword_le(FILE *file, dword dw)
{
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    dw = SDL_Swap32(dw);
  #endif
  return fwrite(&dw, 1, sizeof(dword), file) == sizeof(dword);
}

// Lit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int read_dword_be(FILE *file, dword *Dest)
{
  if (fread(Dest, 1, sizeof(dword), file) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *Dest = SDL_Swap32(*Dest);
  #endif
  return -1;
}
// Ecrit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int write_dword_be(FILE *file, dword dw)
{
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    dw = SDL_Swap32(dw);
  #endif
  return fwrite(&dw, 1, sizeof(dword), file) == sizeof(dword);
}

// Détermine la position du dernier '/' ou '\\' dans une chaine,
// typiquement pour séparer le nom de file d'un chemin.
// Attention, sous Windows, il faut s'attendre aux deux car 
// par exemple un programme lancé sous GDB aura comme argv[0]:
// d:\Data\C\GFX2\grafx2/grafx2.exe
char * Position_dernier_slash(const char * Chaine)
{
  const char * position = NULL;
  for (; *Chaine != '\0'; Chaine++)
    if (*Chaine == SEPARATEUR_CHEMIN[0]
#ifdef __WIN32__    
     || *Chaine == '/'
#endif
     )
      position = Chaine;
  return (char *)position;
}
// Récupère la partie "nom de file seul" d'un chemin
void Extraire_nom_fichier(char *dest, const char *Source)
{
  const char * position = Position_dernier_slash(Source);

  if (position)
    strcpy(dest,position+1);
  else
    strcpy(dest,Source);
}
// Récupère la partie "répertoire+/" d'un chemin.
void Extraire_chemin(char *dest, const char *Source)
{
  char * position;

  strcpy(dest,Source);
  position = Position_dernier_slash(dest);
  if (position)
    *(position+1) = '\0';
  else
    strcat(dest, SEPARATEUR_CHEMIN);
}

int Fichier_existe(char * fname)
//   Détermine si un file passé en paramètre existe ou non dans le
// répertoire courant.
{
    struct stat buf;
    int Resultat;

    Resultat=stat(fname,&buf);
    if (Resultat!=0)
        return(errno!=ENOENT);
    else
        return 1;

}
int Repertoire_existe(char * Repertoire)
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
{
  DIR* entry;    // Structure de lecture des éléments

  if (strcmp(Repertoire,PARENT_DIR)==0)
    return 1;
  else
  {
    //  On va chercher si le répertoire existe à l'aide d'un Opendir. S'il
    //  renvoie NULL c'est que le répertoire n'est pas accessible...

    entry=opendir(Repertoire);
    if (entry==NULL)
        return 0;
    else
    {
        closedir(entry);
        return 1;
    }
  }
}

// Taille de fichier, en octets
int FileLength(const char * fname)
{
    struct stat infos_fichier;
    if (stat(fname,&infos_fichier))
        return 0;
    return infos_fichier.st_size;
}
int File_length_file(FILE * file)
{
    struct stat infos_fichier;
    if (fstat(fileno(file),&infos_fichier))
        return 0;
    return infos_fichier.st_size;
}

void for_each_file(const char * Nom_repertoire, void Callback(const char *))
{
  // Pour scan de répertoire
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* entry; // Structure de lecture des éléments
  char Nom_fichier_complet[TAILLE_CHEMIN_FICHIER];
  int Position_nom_fichier;
  strcpy(Nom_fichier_complet, Nom_repertoire);
  Repertoire_Courant=opendir(Nom_repertoire);
  if(Repertoire_Courant == NULL) return;        // Répertoire invalide ...
  strcat(Nom_fichier_complet, SEPARATEUR_CHEMIN);
  Position_nom_fichier = strlen(Nom_fichier_complet);
  while ((entry=readdir(Repertoire_Courant)))
  {
    struct stat Infos_enreg;
    strcpy(&Nom_fichier_complet[Position_nom_fichier], entry->d_name);
    stat(Nom_fichier_complet,&Infos_enreg);
    if (S_ISREG(Infos_enreg.st_mode))
    {
      Callback(Nom_fichier_complet);
    }
  }
  closedir(Repertoire_Courant);
}

