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
#include "errors.h"
#include "misc.h"

// Mise à jour minimaliste en nombre de pixels
#define UPDATE_METHOD_MULTI_RECTANGLE 1
// Mise à jour intermédiaire, par rectangle inclusif.
#define UPDATE_METHOD_CUMULATED       2
// Mise à jour totale, pour les plate-formes qui imposent un Vsync à chaque mise à jour écran.
#define UPDATE_METHOD_FULL_PAGE     3

// UPDATE_METHOD peut être fixé depuis le makefile, sinon c'est ici:
#ifndef UPDATE_METHOD
  #if defined(__macosx__) || defined(__FreeBSD__)
    #define UPDATE_METHOD     UPDATE_METHOD_FULL_PAGE
  #else
    #define UPDATE_METHOD     UPDATE_METHOD_CUMULATED
  #endif
#endif

void Set_mode_SDL(int *width, int *height, int fullscreen)
/* On règle la résolution de l'écran */
{
  Screen_SDL=SDL_SetVideoMode(*width,*height,8,(fullscreen?SDL_FULLSCREEN:0)|SDL_RESIZABLE);
  if(Screen_SDL != NULL)
  {
    // Vérification du mode obtenu (ce n'est pas toujours celui demandé)
    if (Screen_SDL->w != *width || Screen_SDL->h != *height)
    {
      DEBUG("Error mode video obtenu différent de celui demandé !!",0);
      *width = Screen_SDL->w;
      *height = Screen_SDL->h;
    }
    Screen_pixels=Screen_SDL->pixels;
  }
  else
    DEBUG("Error changement de mode video !!",0);

  SDL_ShowCursor(0); // Cache le curseur SDL, on le gère en soft
}

#if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
short Min_X=0;
short Min_Y=0;
short Max_X=10000;
short Max_Y=10000;
#endif

#if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  int update_is_required=0;
#endif

void Flush_update(void)
{
#if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  // Mise à jour de la totalité de l'écran
  if (update_is_required)
  {
    SDL_UpdateRect(Screen_SDL, 0, 0, 0, 0);
    update_is_required=0;
  }
#endif
  #if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
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
    SDL_UpdateRect(Screen_SDL, Min_X*Pixel_width, Min_Y*Pixel_height, Min(Screen_width-Min_X, Max_X-Min_X)*Pixel_width, Min(Screen_height-Min_Y, Max_Y-Min_Y)*Pixel_height);

    Min_X=Min_Y=10000;
    Max_X=Max_Y=0;
  }
    #endif

}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  #if (UPDATE_METHOD == UPDATE_METHOD_MULTI_RECTANGLE)
    SDL_UpdateRect(Screen_SDL, x*Pixel_width, y*Pixel_height, width*Pixel_width, height*Pixel_height);
  #endif

  #if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
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

  #if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  update_is_required=1;
  #endif

}

// Convertit une SDL_Surface (couleurs indexées ou RGB) en tableau de bytes (couleurs indexées)
// Si on passe NULL comme destination, elle est allouée par malloc(). Sinon,
// attention aux dimensions!
byte * Surface_to_bytefield(SDL_Surface *source, byte * dest)
{
  byte *src;
  byte *dest_ptr;
  int y;
  int remainder;

  // Support seulement des images 256 couleurs
  if (source->format->BytesPerPixel != 1)
    return NULL;

  if (source->w & 3)
    remainder=4-(source->w&3);
  else
    remainder=0;

  if (dest==NULL)
    dest=(byte *)malloc(source->w*source->h);

  dest_ptr=dest;
  src=(byte *)(source->pixels);
  for(y=0; y < source->h; y++)
  {
    memcpy(dest_ptr, src,source->w);
    dest_ptr += source->w;
    src += source->w + remainder;
  }
  return dest;

}

// Convertit un index de palette en couleur RGB 24 bits
SDL_Color Color_to_SDL_color(byte index)
{
  SDL_Color color;
  color.r = Main_palette[index].R;
  color.g = Main_palette[index].G;
  color.b = Main_palette[index].B;
  color.unused = 255;
  return color;
}

// Lecture d'un pixel pour une surface SDL.
// Attention, ne fonctionne que pour les surfaces 8-bit
byte Get_SDL_pixel_8(SDL_Surface *bmp, int x, int y)
{
  return ((byte *)(bmp->pixels))[(y*bmp->pitch+x)];
}
