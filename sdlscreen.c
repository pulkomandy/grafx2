/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
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
#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "global.h"
#include "sdlscreen.h"
#include "erreurs.h"
#include "graph.h"
#include "divers.h"

// Mise à jour minimaliste en nombre de pixels
#define METHODE_UPDATE_MULTI_RECTANGLE 1
// Mise à jour intermédiaire, par rectangle inclusif.
#define METHODE_UPDATE_PAR_CUMUL       2
// Mise à jour totale, pour les plate-formes qui imposent un Vsync à chaque mise à jour écran.
#define METHODE_UPDATE_PLEINE_PAGE     3

// METHODE_UPDATE peut être fixé depuis le makefile, sinon c'est ici:
#ifndef METHODE_UPDATE
  #ifdef __macosx__
    #define METHODE_UPDATE     METHODE_UPDATE_PLEINE_PAGE
  #else
    #define METHODE_UPDATE     METHODE_UPDATE_PAR_CUMUL
  #endif
#endif

void Set_Mode_SDL(int *Largeur, int *Hauteur, int Fullscreen)
/* On règle la résolution de l'écran */
{
  Ecran_SDL=SDL_SetVideoMode(*Largeur,*Hauteur,8,(Fullscreen?SDL_FULLSCREEN:0)|SDL_RESIZABLE);
  if(Ecran_SDL != NULL)
  {
    // Vérification du mode obtenu (ce n'est pas toujours celui demandé)
    if (Ecran_SDL->w != *Largeur || Ecran_SDL->h != *Hauteur)
    {
      DEBUG("Erreur mode video obtenu différent de celui demandé !!",0);
      *Largeur = Ecran_SDL->w;
      *Hauteur = Ecran_SDL->h;
    }
    Ecran=Ecran_SDL->pixels;
  }
  else
    DEBUG("Erreur changement de mode video !!",0);

  SDL_ShowCursor(0); // Cache le curseur SDL, on le gère en soft
}

// Fonction qui filtre les evenements génériques.
void Gere_Evenement_SDL(SDL_Event * event)
{
  // Redimensionnement fenetre
  if (event->type == SDL_VIDEORESIZE )
  {
    Resize_Largeur = event->resize.w;
    Resize_Hauteur = event->resize.h;
  }
  // Fermeture
  if (event->type == SDL_QUIT )
  {
    Quit_demande=1;
  }
}

#if (METHODE_UPDATE == METHODE_UPDATE_PAR_CUMUL)
short Min_X=0;
short Min_Y=0;
short Max_X=10000;
short Max_Y=10000;
#endif

#if (METHODE_UPDATE == METHODE_UPDATE_PLEINE_PAGE)
  int Update_necessaire=0;
#endif

void Flush_update(void)
{
#if (METHODE_UPDATE == METHODE_UPDATE_PLEINE_PAGE)
  // Mise à jour de la totalité de l'écran
  if (Update_necessaire)
  {
    SDL_UpdateRect(Ecran_SDL, 0, 0, 0, 0);
    Update_necessaire=0;
  }
#endif
  #if (METHODE_UPDATE == METHODE_UPDATE_PAR_CUMUL)
  if (Min_X>=Max_X || Min_Y>=Max_Y)
  {
    ; // Rien a faire
  }
  else
  {
    if (Min_X<0)
      Min_X=0;
    if (Min_Y<0)
      Min_Y=0;
    SDL_UpdateRect(Ecran_SDL, Min_X*Pixel_width, Min_Y*Pixel_height, Min(Largeur_ecran-Min_X, Max_X-Min_X)*Pixel_width, Min(Hauteur_ecran-Min_Y, Max_Y-Min_Y)*Pixel_height);

    Min_X=Min_Y=10000;
    Max_X=Max_Y=0;
  }
    #endif
  
}

void UpdateRect(short X, short Y, unsigned short Largeur, unsigned short Hauteur)
{
  #if (METHODE_UPDATE == METHODE_UPDATE_MULTI_RECTANGLE)
    SDL_UpdateRect(Ecran_SDL, X*Pixel_width, Y*Pixel_height, Largeur*Pixel_width, Hauteur*Pixel_height);
  #endif
  
  #if (METHODE_UPDATE == METHODE_UPDATE_PAR_CUMUL)
  if (Largeur==0 || Hauteur==0)
  {
    Min_X=Min_Y=0;
    Max_X=Max_Y=10000;
  }
  else
  {
    if (X < Min_X)
      Min_X = X;
    if (Y < Min_Y)
      Min_Y = Y;
    if (X+Largeur>Max_X)
      Max_X=X+Largeur;
    if (Y+Hauteur>Max_Y)
      Max_Y=Y+Hauteur;
  }
  #endif
  
  #if (METHODE_UPDATE == METHODE_UPDATE_PLEINE_PAGE)
  Update_necessaire=1;
  #endif

}

// Convertit une SDL_Surface (couleurs indexées ou RGB) en tableau de bytes (couleurs indexées)
// Si on passe NULL comme destination, elle est allouée par malloc(). Sinon,
// attention aux dimensions!
byte * Surface_en_bytefield(SDL_Surface *Source, byte * Destination)
{
  byte *Src;
  byte *Dst;
  int Y;
  int Reste;

  // Support seulement des images 256 couleurs
  if (Source->format->BytesPerPixel != 1)
    return NULL;

  if (Source->w & 3)
    Reste=4-(Source->w&3);
  else
    Reste=0;
  
  if (Destination==NULL)
    Destination=(byte *)malloc(Source->w*Source->h);

  Dst=Destination;
  Src=(byte *)(Source->pixels);
  for(Y=0; Y < Source->h; Y++)
  {
    memcpy(Dst, Src,Source->w);
    Dst += Source->w;
    Src += Source->w + Reste;
  }
  return Destination;

}

// Convertit un index de palette en couleur RGB 24 bits
SDL_Color Conversion_couleur_SDL(byte Index)
{
  SDL_Color Couleur;
  Couleur.r = Principal_Palette[Index].R;
  Couleur.g = Principal_Palette[Index].V;
  Couleur.b = Principal_Palette[Index].B;
  Couleur.unused = 255;
  return Couleur;
}
