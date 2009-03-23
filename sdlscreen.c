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
#include <SDL.h>
#include "global.h"
#include "sdlscreen.h"
#include "erreurs.h"
#include "divers.h"

// Mise à jour minimaliste en nombre de pixels
#define METHODE_UPDATE_MULTI_RECTANGLE 1
// Mise à jour intermédiaire, par rectangle inclusif.
#define METHODE_UPDATE_PAR_CUMUL       2
// Mise à jour totale, pour les plate-formes qui imposent un Vsync à chaque mise à jour écran.
#define METHODE_UPDATE_PLEINE_PAGE     3

// METHODE_UPDATE peut être fixé depuis le makefile, sinon c'est ici:
#ifndef METHODE_UPDATE
  #if defined(__macosx__) || defined(__FreeBSD__)
    #define METHODE_UPDATE     METHODE_UPDATE_PLEINE_PAGE
  #else
    #define METHODE_UPDATE     METHODE_UPDATE_PAR_CUMUL
  #endif
#endif

void Set_Mode_SDL(int *width, int *height, int fullscreen)
/* On règle la résolution de l'écran */
{
  Ecran_SDL=SDL_SetVideoMode(*width,*height,8,(fullscreen?SDL_FULLSCREEN:0)|SDL_RESIZABLE);
  if(Ecran_SDL != NULL)
  {
    // Vérification du mode obtenu (ce n'est pas toujours celui demandé)
    if (Ecran_SDL->w != *width || Ecran_SDL->h != *height)
    {
      DEBUG("Erreur mode video obtenu différent de celui demandé !!",0);
      *width = Ecran_SDL->w;
      *height = Ecran_SDL->h;
    }
    Ecran=Ecran_SDL->pixels;
  }
  else
    DEBUG("Erreur changement de mode video !!",0);

  SDL_ShowCursor(0); // Cache le curseur SDL, on le gère en soft
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

void UpdateRect(short x, short y, unsigned short width, unsigned short height)
{
  #if (METHODE_UPDATE == METHODE_UPDATE_MULTI_RECTANGLE)
    SDL_UpdateRect(Ecran_SDL, x*Pixel_width, y*Pixel_height, width*Pixel_width, height*Pixel_height);
  #endif

  #if (METHODE_UPDATE == METHODE_UPDATE_PAR_CUMUL)
  if (width==0 || height==0)
  {
    Min_X=Min_Y=0;
    Max_X=Max_Y=10000;
  }
  else
  {
    if (x < Min_X)
      Min_X = x;
    if (y < Min_Y)
      Min_Y = y;
    if (x+width>Max_X)
      Max_X=x+width;
    if (y+height>Max_Y)
      Max_Y=y+height;
  }
  #endif

  #if (METHODE_UPDATE == METHODE_UPDATE_PLEINE_PAGE)
  Update_necessaire=1;
  #endif

}

// Convertit une SDL_Surface (couleurs indexées ou RGB) en tableau de bytes (couleurs indexées)
// Si on passe NULL comme destination, elle est allouée par malloc(). Sinon,
// attention aux dimensions!
byte * Surface_en_bytefield(SDL_Surface *Source, byte * dest)
{
  byte *Src;
  byte *dest_ptr;
  int y;
  int remainder;

  // Support seulement des images 256 couleurs
  if (Source->format->BytesPerPixel != 1)
    return NULL;

  if (Source->w & 3)
    remainder=4-(Source->w&3);
  else
    remainder=0;

  if (dest==NULL)
    dest=(byte *)malloc(Source->w*Source->h);

  dest_ptr=dest;
  Src=(byte *)(Source->pixels);
  for(y=0; y < Source->h; y++)
  {
    memcpy(dest_ptr, Src,Source->w);
    dest_ptr += Source->w;
    Src += Source->w + remainder;
  }
  return dest;

}

// Convertit un index de palette en couleur RGB 24 bits
SDL_Color Conversion_couleur_SDL(byte index)
{
  SDL_Color Couleur;
  Couleur.r = Principal_Palette[index].R;
  Couleur.g = Principal_Palette[index].G;
  Couleur.b = Principal_Palette[index].B;
  Couleur.unused = 255;
  return Couleur;
}

// Lecture d'un pixel pour une surface SDL.
// Attention, ne fonctionne que pour les surfaces 8-bit
byte Sdl_Get_pixel_8(SDL_Surface *Bmp, int x, int y)
{
  return ((byte *)(Bmp->pixels))[(y*Bmp->pitch+x)];
}
