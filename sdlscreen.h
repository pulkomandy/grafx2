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

//////////////////////////////////////////////////////////////////////////////
///@file sdlscreen.h
/// Screen update (refresh) system, and some SDL-specific graphic functions.
//////////////////////////////////////////////////////////////////////////////

#ifndef SDLSCREEN_H_INCLUDED
#define SDLSCREEN_H_INCLUDED

#include <SDL.h>
#include "struct.h"

  void Set_mode_SDL(int *,int *,int);

  SDL_Rect ** List_SDL_video_modes;
  byte* Screen_pixels;

  void Update_rect(short x, short y, unsigned short width, unsigned short height);
  void Flush_update(void);
  byte * Surface_to_bytefield(SDL_Surface *source, byte * dest);
  SDL_Color Color_to_SDL_color(byte);
  byte Get_SDL_pixel_8(SDL_Surface *bmp, int x, int y);
  
#endif // SDLSCREEN_H_INCLUDED
