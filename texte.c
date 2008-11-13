/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
  #include <SDL/SDL_ttf.h>
#endif

#ifdef __linux__
  #include <X11/Xlib.h>
#endif
#endif
#include <SDL/SDL_image.h>
// SFont
#include "SFont.h"

#include "struct.h"
#include "global.h"
#include "sdlscreen.h"
#include "io.h"
#include "files.h"

typedef struct T_FONTE
{
  char * Nom;
  int    EstTrueType;
  int    EstImage;
  
  // Liste chainée simple
  struct T_FONTE * Suivante;
} T_FONTE;
// Liste chainée des polices de texte
T_FONTE * Liste_fontes_debut;
T_FONTE * Liste_fontes_fin;
int Fonte_nombre;

// Inspiré par Allegro
#define EXTID(a,b,c) ((((a)&255)<<16) | (((b)&255)<<8) | (((c)&255)))

// Ajout d'une fonte à la liste.
void Ajout_fonte(const char *Nom)
{
  T_FONTE * Fonte = (T_FONTE *)malloc(sizeof(T_FONTE));
  int Taille=strlen(Nom)+1;
  // Détermination du type:
  if (Taille<5 ||
    Nom[Taille-5]!='.')
    return;
  switch (EXTID(tolower(Nom[Taille-4]), tolower(Nom[Taille-3]), tolower(Nom[Taille-2])))
  {
    case EXTID('t','t','f'):
    case EXTID('f','o','n'):
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
      return;
  }

  Fonte->Nom = (char *)malloc(Taille);
  strcpy(Fonte->Nom, Nom);

  // Gestion Liste
  Fonte->Suivante = NULL;
  if (Liste_fontes_debut==NULL)
    Liste_fontes_debut = Fonte;
  else
    Liste_fontes_fin->Suivante = Fonte;
  Liste_fontes_fin = Fonte;
  Fonte_nombre++;

}


// Trouve le nom d'une fonte par son numéro
char * Nom_fonte(int Indice)
{
  T_FONTE *Fonte = Liste_fontes_debut;
  if (Indice<0 ||Indice>=Fonte_nombre)
    return "";
  while (Indice--)
    Fonte = Fonte->Suivante;
  return Fonte->Nom;
}


// Trouve le libellé d'affichage d'une fonte par son numéro
char * Libelle_fonte(int Indice)
{
  T_FONTE *Fonte;
  static char Libelle[22];
  char * Nom_fonte;
  
  strcpy(Libelle, "                     ");
  
  // Recherche de la fonte
  Fonte = Liste_fontes_debut;
  if (Indice<0 ||Indice>=Fonte_nombre)
    return Libelle;
  while (Indice--)
    Fonte = Fonte->Suivante;
  
  // Libellé
  if (Fonte->EstTrueType)
    Libelle[19]=Libelle[20]='T'; // Logo TT
  Nom_fonte=Position_dernier_slash(Fonte->Nom);
  if (Nom_fonte==NULL)
    Nom_fonte=Fonte->Nom;
  else
    Nom_fonte++;
  for (Indice=0; Indice < 19 && Nom_fonte[Indice]!='\0' && Nom_fonte[Indice]!='.'; Indice++)
    Libelle[Indice]=Nom_fonte[Indice];
  return Libelle;
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
/*
  // Pour scan de répertoire
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* Enreg; // Structure de lecture des éléments
*/
  char Nom_repertoire[TAILLE_CHEMIN_FICHIER];
  #ifndef NOTTF
  // Initialisation de TTF
  TTF_Init();
  #endif
  
  // Initialisation des fontes
  Liste_fontes_debut = Liste_fontes_fin = NULL;
  Fonte_nombre=0;
  // Parcours du répertoire "fonts"
  strcpy(Nom_repertoire, Repertoire_du_programme);
  strcat(Nom_repertoire, "fonts");
  for_each_file(Nom_repertoire, Ajout_fonte);
  
  #ifdef __WIN32__
    // Parcours du répertoire systeme windows "fonts"
    #ifndef NOTTF
    for_each_file("c:\\windows\\fonts", Ajout_fonte);
    #endif
  #elif defined(__linux__)
    // Récupération de la liste des fonts avec fontconfig
    #define USE_XLIB

    #ifdef USE_XLIB
	int i,number;
	Display* dpy = XOpenDisplay(NULL);
	char** font_path_list = XGetFontPath(dpy,&number);

	for(i=0;i<number;i++)
	    for_each_file(*(font_path_list+i),Ajout_fonte);
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
byte *Rendu_Texte_TTF(const char *Chaine, int Numero_fonte, int Taille, int AntiAlias, int *Largeur, int *Hauteur)
{
 TTF_Font *Fonte;
  SDL_Surface * TexteColore;
  SDL_Surface * Texte8Bit;
  byte * BrosseRetour;
  int Indice;
  
  SDL_Color Couleur_Avant;
  SDL_Color Couleur_Arriere;

  // Chargement de la fonte
  Fonte=TTF_OpenFont(Nom_fonte(Numero_fonte), Taille);
  if (!Fonte)
  {
    return NULL;
  }
  // Couleurs
  if (AntiAlias)
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
  if (AntiAlias)
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
  if (!AntiAlias)
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
  *Largeur=Texte8Bit->w;
  *Hauteur=Texte8Bit->h;
  SDL_FreeSurface(Texte8Bit);
  TTF_CloseFont(Fonte);
  return BrosseRetour;
}
#endif


byte *Rendu_Texte_SFont(const char *Chaine, int Numero_fonte, int *Largeur, int *Hauteur)
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
  *Hauteur=SFont_TextHeight(Fonte);
  *Largeur=SFont_TextWidth(Fonte, Chaine);
  // Allocation d'une surface SDL
  TexteColore=SDL_CreateRGBSurface(SDL_SWSURFACE, *Largeur, *Hauteur, 24, 0, 0, 0, 0);
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
// Si cela réussit, la fonction place les dimensions dans Largeur et Hauteur, 
// et retourne l'adresse du bloc d'octets.
byte *Rendu_Texte(const char *Chaine, int Numero_fonte, int Taille, int AntiAlias, int *Largeur, int *Hauteur)
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
    return Rendu_Texte_TTF(Chaine, Numero_fonte, Taille, AntiAlias, Largeur, Hauteur);
  #else
    return NULL;
  #endif
  }
  else
  {
    return Rendu_Texte_SFont(Chaine, Numero_fonte, Largeur, Hauteur);
  }
}


