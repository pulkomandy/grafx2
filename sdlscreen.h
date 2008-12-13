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
#ifndef SDLSCREEN_H_INCLUDED
#define SDLSCREEN_H_INCLUDED

#include <SDL/SDL.h>
#include "struct.h"

  void Set_Mode_SDL(int *,int *,int);
  void Gere_Evenement_SDL(SDL_Event * event);

  SDL_Rect ** Liste_Modes_Videos_SDL;
  byte* Ecran;

  void UpdateRect(short X, short Y, unsigned short Largeur, unsigned short Hauteur);
  void Flush_update(void);
  byte * Surface_en_bytefield(SDL_Surface *Source, byte * Destination);
  SDL_Color Conversion_couleur_SDL(byte);

#endif // SDLSCREEN_H_INCLUDED
