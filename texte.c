/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2008 Adrien Destugues
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

// Pour désactiver le support TrueType, définir NOTTF
// To disable TrueType support, define NOTTF

#include <string.h>
#include <stdlib.h>
#include <ctype.h> // tolower()

// TrueType
#ifndef NOTTF
#ifdef __macosx__
  #include <SDL_ttf/SDL_ttf.h>
#else
  #include <SDL_ttf.h>
#endif

#ifdef __linux__
#ifdef __macosx__
  #include <Carbon/Carbon.h>
  #import <corefoundation/corefoundation.h>
  #import <sys/param.h>
#else
  #include <X11/Xlib.h>
#endif
#endif
#endif

#include <SDL_image.h>
// SFont
#include "SFont.h"

#include "struct.h"
#include "global.h"
#include "sdlscreen.h"
#include "io.h"

typedef struct T_FONTE
{
  char * Name;
  int    EstTrueType;
  int    EstImage;
  char   Label[22];
  
  // Liste chainée simple
  struct T_FONTE * Suivante;
  struct T_FONTE * Precedente;
} T_FONTE;
// Liste chainée des polices de texte
T_FONTE * Liste_fontes_debut;
int Fonte_nombre;

// Inspiré par Allegro
#define EXTID(a,b,c) ((((a)&255)<<16) | (((b)&255)<<8) | (((c)&255)))
#define EXTID4(a,b,c,d) ((((a)&255)<<24) | (((b)&255)<<16) | (((c)&255)<<8) | (((d)&255)))

int Compare_fontes(T_FONTE * Fonte1, T_FONTE * Fonte2)
{
  if (Fonte1->EstImage && !Fonte2->EstImage)
    return -1;
  if (Fonte2->EstImage && !Fonte1->EstImage)
    return 1;
  return strcmp(Fonte1->Label, Fonte2->Label);
}

// Ajout d'une fonte à la liste.
void Ajout_fonte(const char *name)
{
  char * Nom_fonte;
  T_FONTE * Fonte;
  int size=strlen(name)+1;
  int Indice;
  
  // Détermination du type:

#ifdef __macosx__

  if (size < 6) return;
  
  char strFontName[512];
  CFStringRef CFSFontName;// = CFSTR(name);

  CFSFontName = CFStringCreateWithBytes(NULL, (UInt8 *) name, size - 1, kCFStringEncodingASCII, false);
  // Fix some funny names
  CFStringGetCString(CFSFontName, strFontName, 512, kCFStringEncodingASCII);

  // Now we have a printable font name, use it
  name = strFontName;

#else
  if (size<5 ||
      name[size-5]!='.')
    return;
#endif

  Fonte = (T_FONTE *)malloc(sizeof(T_FONTE));

  switch (EXTID(tolower(name[size-4]), tolower(name[size-3]), tolower(name[size-2])))
  {
    case EXTID('t','t','f'):
    case EXTID('f','o','n'):
    case EXTID('o','t','f'):
    case EXTID('p','f','b'):
      Fonte->EstTrueType = 1;
      Fonte->EstImage = 0;
      break;
    case EXTID('b','m','p'):
    case EXTID('g','i','f'):
    case EXTID('j','p','g'):
    case EXTID('l','b','m'):
    case EXTID('p','c','x'):
    case EXTID('p','n','g'):
    case EXTID('t','g','a'):
    case EXTID('t','i','f'):
    case EXTID('x','c','f'):
    case EXTID('x','p','m'):
    case EXTID('.','x','v'):
      Fonte->EstTrueType = 0;
      Fonte->EstImage = 1;
      break;
    default:
      #ifdef __macosx__
         if(strcasecmp(&name[size-6], "dfont") == 0)
         {
           Fonte->EstTrueType = 1;
           Fonte->EstImage = 0;
         }
         else
         {
            free(Fonte);
            return;
         }
      #else
         free(Fonte);
         return;
      #endif
  }

  Fonte->Name = (char *)malloc(size);
  strcpy(Fonte->Name, name);
  // Label
  strcpy(Fonte->Label, "                   ");
  if (Fonte->EstTrueType)
    Fonte->Label[17]=Fonte->Label[18]='T'; // Logo TT
  Nom_fonte=Position_dernier_slash(Fonte->Name);
  if (Nom_fonte==NULL)
    Nom_fonte=Fonte->Name;
  else
    Nom_fonte++;
  for (Indice=0; Indice < 17 && Nom_fonte[Indice]!='\0' && Nom_fonte[Indice]!='.'; Indice++)
    Fonte->Label[Indice]=Nom_fonte[Indice];

  // Gestion Liste
  Fonte->Suivante = NULL;
  Fonte->Precedente = NULL;
  if (Liste_fontes_debut==NULL)
  {
    // Premiere (liste vide)
    Liste_fontes_debut = Fonte;
    Fonte_nombre++;
  }
  else
  {
    int Compare;
    Compare = Compare_fontes(Fonte, Liste_fontes_debut);
    if (Compare<=0)
    {
      if (Compare==0 && !strcmp(Fonte->Name, Liste_fontes_debut->Name))
      {
        // Doublon
        free(Fonte->Name);
        free(Fonte);
        return;
      }
      // Avant la premiere
      Fonte->Suivante=Liste_fontes_debut;
      Liste_fontes_debut=Fonte;
      Fonte_nombre++;
    }
    else
    {
      T_FONTE *Fonte_cherchee;
      Fonte_cherchee=Liste_fontes_debut;
      while (Fonte_cherchee->Suivante && (Compare=Compare_fontes(Fonte, Fonte_cherchee->Suivante))>0)
        Fonte_cherchee=Fonte_cherchee->Suivante;
      // Après Fonte_cherchee
      if (Compare==0 && strcmp(Fonte->Name, Fonte_cherchee->Suivante->Name)==0)
      {
        // Doublon
        free(Fonte->Name);
        free(Fonte);
        return;
      }
      Fonte->Suivante=Fonte_cherchee->Suivante;
      Fonte_cherchee->Suivante=Fonte;
      Fonte_nombre++;
    }
  }
}


// Trouve le nom d'une fonte par son numéro
char * Nom_fonte(int Indice)
{
  T_FONTE *Fonte = Liste_fontes_debut;
  if (Indice<0 ||Indice>=Fonte_nombre)
    return "";
  while (Indice--)
    Fonte = Fonte->Suivante;
  return Fonte->Name;
}


// Trouve le libellé d'affichage d'une fonte par son numéro
// Renvoie un pointeur sur un buffer statique de 20 caracteres.
char * Libelle_fonte(int Indice)
{
  T_FONTE *Fonte;
  static char label[20];
  
  strcpy(label, "                   ");
  
  // Recherche de la fonte
  Fonte = Liste_fontes_debut;
  if (Indice<0 ||Indice>=Fonte_nombre)
    return label;
  while (Indice--)
    Fonte = Fonte->Suivante;
  
  // Libellé
  strcpy(label, Fonte->Label);
  return label;
}


// Vérifie si une fonte donnée est TrueType
int TrueType_fonte(int Indice)
{
  T_FONTE *Fonte = Liste_fontes_debut;
  if (Indice<0 ||Indice>=Fonte_nombre)
    return 0;
  while (Indice--)
    Fonte = Fonte->Suivante;
  return Fonte->EstTrueType;
}



// Initialisation à faire une fois au début du programme
void Initialisation_Texte(void)
{
  char Nom_repertoire[TAILLE_CHEMIN_FICHIER];
  #ifndef NOTTF
  // Initialisation de TTF
  TTF_Init();
  #endif
  
  // Initialisation des fontes
  Liste_fontes_debut = NULL;
  Fonte_nombre=0;
  // Parcours du répertoire "fonts"
  strcpy(Nom_repertoire, Repertoire_des_donnees);
  strcat(Nom_repertoire, "fonts");
  for_each_file(Nom_repertoire, Ajout_fonte);
  
  #ifdef __WIN32__
    // Parcours du répertoire systeme windows "fonts"
    #ifndef NOTTF
    {
      char * WindowsPath=getenv("windir");
      if (WindowsPath)
      {
        sprintf(Nom_repertoire, "%s\\FONTS", WindowsPath);
        for_each_file(Nom_repertoire, Ajout_fonte);
      }
    }
    #endif
  #elif defined(__macosx__)
    // Récupération de la liste des fonts avec fontconfig
    #ifndef NOTTF


      int i,number;
      char home_dir[MAXPATHLEN];
      char *font_path_list[3] = {
         "/System/Library/Fonts",
         "/Library/Fonts"
      };
      number = 3;
      // Make sure we also search into the user's fonts directory
      CFURLRef url = (CFURLRef) CFCopyHomeDirectoryURLForUser(NULL);
      CFURLGetFileSystemRepresentation(url, true, (UInt8 *) home_dir, MAXPATHLEN);
      strcat(home_dir, "/Library/Fonts");
      font_path_list[2] = home_dir;

      for(i=0;i<number;i++)
         for_each_file(*(font_path_list+i),Ajout_fonte);

      CFRelease(url);
    #endif

  #elif defined(__linux__)
    #ifndef NOTTF
       #define USE_XLIB
    
       #ifdef USE_XLIB
       {
        int i,number;
        Display* dpy = XOpenDisplay(NULL);
        char** font_path_list = XGetFontPath(dpy,&number);
        XCloseDisplay(dpy);

        for(i=0;i<number;i++)
            for_each_file(*(font_path_list+i),Ajout_fonte);

        XFreeFontPath(font_path_list);
       }
       #endif
    #endif
  #elif defined(__amigaos4__)
    #ifndef NOTTF
      for_each_file( "FONTS:_TrueType", Ajout_fonte );
    #endif
  #elif defined(__BEOS__) || defined(__HAIKU__)
    #ifndef NOTTF
      for_each_file("/etc/fonts/ttfonts", Ajout_fonte);
    #endif

  #elif defined(__SKYOS__)
    #ifndef NOTTF
      for_each_file("/boot/system/fonts", Ajout_fonte);
    #endif

  #endif
}

// Informe si texte.c a été compilé avec l'option de support TrueType ou pas.
int Support_TrueType()
{
  #ifdef NOTTF
  return 0;
  #else
  return 1;
  #endif
}

  
#ifndef NOTTF
byte *Rendu_Texte_TTF(const char *Chaine, int Numero_fonte, int size, int antialias, int bold, int italic, int *width, int *height)
{
 TTF_Font *Fonte;
  SDL_Surface * TexteColore;
  SDL_Surface * Texte8Bit;
  byte * BrosseRetour;
  int Indice;
  int style;
  
  SDL_Color Couleur_Avant;
  SDL_Color Couleur_Arriere;

  // Chargement de la fonte
  Fonte=TTF_OpenFont(Nom_fonte(Numero_fonte), size);
  if (!Fonte)
  {
    return NULL;
  }
  // Style
  style=0;
  if (italic)
    style|=TTF_STYLE_ITALIC;
  if (bold)
    style|=TTF_STYLE_BOLD;
  TTF_SetFontStyle(Fonte, style);
  // Couleurs
  if (antialias)
  {
    Couleur_Avant = Conversion_couleur_SDL(Fore_color);
    Couleur_Arriere = Conversion_couleur_SDL(Back_color);
  }
  else
  {
    Couleur_Avant = Conversion_couleur_SDL(CM_Blanc);
    Couleur_Arriere = Conversion_couleur_SDL(CM_Noir);
  }

  
  // Rendu du texte: crée une surface SDL RGB 24bits
  if (antialias)
    TexteColore=TTF_RenderText_Shaded(Fonte, Chaine, Couleur_Avant, Couleur_Arriere );
  else
    TexteColore=TTF_RenderText_Solid(Fonte, Chaine, Couleur_Avant);
  if (!TexteColore)
  {
    TTF_CloseFont(Fonte);
    return NULL;
  }
  
  Texte8Bit=SDL_DisplayFormat(TexteColore);

  SDL_FreeSurface(TexteColore);
    
  BrosseRetour=Surface_en_bytefield(Texte8Bit, NULL);
  if (!BrosseRetour)
  {
    SDL_FreeSurface(TexteColore);
    SDL_FreeSurface(Texte8Bit);
    TTF_CloseFont(Fonte);
    return NULL;
  }
  if (!antialias)
  {
    // Mappage des couleurs
    for (Indice=0; Indice < Texte8Bit->w * Texte8Bit->h; Indice++)
    {
      if (*(BrosseRetour+Indice) == CM_Noir)
      *(BrosseRetour+Indice)=Back_color;
      else if (*(BrosseRetour+Indice) == CM_Blanc)
      *(BrosseRetour+Indice)=Fore_color;
    }
  }
  *width=Texte8Bit->w;
  *height=Texte8Bit->h;
  SDL_FreeSurface(Texte8Bit);
  TTF_CloseFont(Fonte);
  return BrosseRetour;
}
#endif


byte *Rendu_Texte_SFont(const char *Chaine, int Numero_fonte, int *width, int *height)
{
  SFont_Font *Fonte;
  SDL_Surface * TexteColore;
  SDL_Surface * Texte8Bit;
  SDL_Surface *Surface_fonte;
  byte * BrosseRetour;

  // Chargement de la fonte
  Surface_fonte=IMG_Load(Nom_fonte(Numero_fonte));
  if (!Surface_fonte)
    return NULL;
  Fonte=SFont_InitFont(Surface_fonte);
  if (!Fonte)
  {
    return NULL;
  }
  
  // Calcul des dimensions
  *height=SFont_TextHeight(Fonte);
  *width=SFont_TextWidth(Fonte, Chaine);
  // Allocation d'une surface SDL
  TexteColore=SDL_CreateRGBSurface(SDL_SWSURFACE, *width, *height, 24, 0, 0, 0, 0);
  // Rendu du texte
  SFont_Write(TexteColore, Fonte, 0, 0, Chaine);
  if (!TexteColore)
  {
    SFont_FreeFont(Fonte);
    return NULL;
  }
  
  Texte8Bit=SDL_DisplayFormat(TexteColore);
  SDL_FreeSurface(TexteColore);
    
  BrosseRetour=Surface_en_bytefield(Texte8Bit, NULL);
  if (!BrosseRetour)
  {
    SDL_FreeSurface(TexteColore);
    SDL_FreeSurface(Texte8Bit);
    SFont_FreeFont(Fonte);
    return NULL;
  }
  SDL_FreeSurface(Texte8Bit);
  SFont_FreeFont(Fonte);

  return BrosseRetour;
}


// Crée une brosse à partir des paramètres de texte demandés.
// Si cela réussit, la fonction place les dimensions dans width et height, 
// et retourne l'adresse du bloc d'octets.
byte *Rendu_Texte(const char *Chaine, int Numero_fonte, int size, int antialias, int bold, int italic, int *width, int *height)
{
  T_FONTE *Fonte = Liste_fontes_debut;
  int Indice=Numero_fonte;
  
  // Verification type de la fonte
  if (Numero_fonte<0 ||Numero_fonte>=Fonte_nombre)
    return NULL;
    
  while (Indice--)
    Fonte = Fonte->Suivante;
  if (Fonte->EstTrueType)
  {
  #ifndef NOTTF 
    return Rendu_Texte_TTF(Chaine, Numero_fonte, size, antialias, bold, italic, width, height);
  #else
    return NULL;
  #endif
  }
  else
  {
    return Rendu_Texte_SFont(Chaine, Numero_fonte, width, height);
  }
}


