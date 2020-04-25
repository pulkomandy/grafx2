/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

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
  byte * pixels;      ///< pixel data (1 byte = 1 pixel)
  T_Palette palette;  ///< 256 color palette
  word w;             ///< Width
  word h;             ///< Height
} T_GFX2_Surface;

/**
 * Allocate a new surface.
 * @param width, height dimension of the new surface. Cannot be 0
 * @return the new surface
 * @return NULL in case of error
 */
T_GFX2_Surface * New_GFX2_Surface(word width, word height);

/**
 * Free the surface object and the associated pixel data.
 * @param surface The surface to free
 */
void Free_GFX2_Surface(T_GFX2_Surface * surface);

/**
 * Retrieve a pixel value.
 * @param surface The surface to access
 * @param x, y the coordinate of the pixel to read
 * @return the pixel 8 bits value
 * @return 0 for out of bound access
 */
byte Get_GFX2_Surface_pixel(const T_GFX2_Surface * surface, int x, int y);

/**
 * Set a pixel. Nothing is done if the coordinates are out of bound.
 * @param surface The surface to access
 * @param x, y the coordinate of the pixel to write
 * @param value the 8 bits pixel value
 */
void Set_GFX2_Surface_pixel(T_GFX2_Surface * surface, int x, int y, byte value);

#endif
