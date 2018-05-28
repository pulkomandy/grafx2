/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
//////////////////////////////////////////////////////////////////////////////
///@file gfx2surface.h
/// Memory representation of a 8bits picture
//////////////////////////////////////////////////////////////////////////////

#ifndef GFX2SURFACE_H__
#define GFX2SURFACE_H__

#include "struct.h"

/// to load 256 color pictures into memory
typedef struct
{
  byte * pixels;
  T_Palette palette;
  word w;           ///< Width
  word h;           ///< Height
} T_GFX2_Surface;

T_GFX2_Surface * New_GFX2_Surface(word width, word height);
void Free_GFX2_Surface(T_GFX2_Surface * surface);

byte Get_GFX2_Surface_pixel(const T_GFX2_Surface * surface, int x, int y);
void Set_GFX2_Surface_pixel(T_GFX2_Surface * surface, int x, int y, byte value);

#endif
