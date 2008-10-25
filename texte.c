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


// TrueType
#ifndef NOTTF
#include <SDL/SDL_ttf.h>
#endif
// SFont
#include <SDL/SDL_image.h>
#include "SFont.h"

#include "sdlscreen.h"
#include "struct.h"
#include "global.h"

// Initialisation à faire une fois au début du programme
void Initialisation_Texte(void)
{
  #ifndef NOTTF
  TTF_Init();
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
byte *Rendu_Texte_TTF(const char *Chaine, int Taille, int AntiAlias, int *Largeur, int *Hauteur)
{
 TTF_Font *Police;
  SDL_Surface * TexteColore;
  SDL_Surface * Texte8Bit;
  byte * BrosseRetour;
  int Indice;
  SDL_Color PaletteSDL[256];
  
  SDL_Color Couleur_Avant; // FIXME couleur avant
  SDL_Color Couleur_Arriere; // FIXME couleur arriere

  // Chargement de la police
  Police=TTF_OpenFont("fonts/Tuffy.ttf", Taille); // FIXME police en dur
  if (!Police)
  {
    return NULL;
  }
  // Couleurs
  Couleur_Avant = Conversion_couleur_SDL(Fore_color);
  Couleur_Arriere = Conversion_couleur_SDL(Back_color);
  
  // Rendu du texte: crée une surface SDL RGB 24bits
  if (AntiAlias)
    TexteColore=TTF_RenderText_Shaded(Police, Chaine, Couleur_Avant, Couleur_Arriere );
  else
    TexteColore=TTF_RenderText_Solid(Police, Chaine, Couleur_Avant);
  if (!TexteColore)
  {
    TTF_CloseFont(Police);
    return NULL;
  }
  
  Texte8Bit=SDL_DisplayFormat(TexteColore);
  /*
  
  Texte8Bit=SDL_CreateRGBSurface(SDL_SWSURFACE, TexteColore->w, TexteColore->h, 8, 0, 0, 0, 0);
  if (!Texte8Bit)
  {
    SDL_FreeSurface(TexteColore);
    TTF_CloseFont(Police);
    return NULL;
  }

  for(Indice=0;Indice<256;Indice++)
  {
    PaletteSDL[Indice].r=(Principal_Palette[Indice].R<<2) + (Principal_Palette[Indice].R>>4); //Les couleurs VGA ne vont que de 0 à 63
    PaletteSDL[Indice].g=(Principal_Palette[Indice].V<<2) + (Principal_Palette[Indice].V>>4);
    PaletteSDL[Indice].b=(Principal_Palette[Indice].B<<2) + (Principal_Palette[Indice].B>>4);
  }
  SDL_SetPalette(Texte8Bit,SDL_PHYSPAL|SDL_LOGPAL,PaletteSDL,0,256);
  SDL_BlitSurface(TexteColore, NULL, Texte8Bit, NULL);
  */
  SDL_FreeSurface(TexteColore);
    
  BrosseRetour=Surface_en_bytefield(Texte8Bit, NULL);
  if (!BrosseRetour)
  {
    SDL_FreeSurface(TexteColore);
    SDL_FreeSurface(Texte8Bit);
    TTF_CloseFont(Police);
    return NULL;
  }
  if (!AntiAlias)
  {
  // Mappage des couleurs
  /*for (Indice=0; Indice < Texte8Bit->w * Texte8Bit->h; Indice++)
  {
    if ()
    *(BrosseRetour+Indice)=*(BrosseRetour+Indice) ? (*(BrosseRetour+Indice)+96-15) : 0;
  }*/
  }
  
  *Largeur=Texte8Bit->w;
  *Hauteur=Texte8Bit->h;
  SDL_FreeSurface(Texte8Bit);

  return BrosseRetour;
}
#endif

// Crée une brosse à partir des paramètres de texte demandés.
// Si cela réussit, la fonction place les dimensions dans Largeur et Hauteur, 
// et retourne l'adresse du bloc d'octets.
byte *Rendu_Texte(const char *Chaine, int Taille, int AntiAlias, int *Largeur, int *Hauteur)
{
  #ifndef NOTTF 
    return Rendu_Texte_TTF(Chaine, Taille, AntiAlias, Largeur, Hauteur);
  #else
    return NULL;
  #endif
}


