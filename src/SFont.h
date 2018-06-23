/* vim:expandtab:ts=2 sw=2:
*/
/*  SFont: a simple font-library that uses special bitmaps as fonts
    Copyright (C) 2003 Karl Bartel

    GrafX2 Modification
    Copyright (c) 2018 Thomas Bernard

    License: GPL or LGPL (at your choice)
    WWW: http://www.linux-games.com/sfont/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <http://www.gnu.org/licenses/>.

    Karl Bartel
    Cecilienstr. 14
    12307 Berlin
    GERMANY
    karlb@gmx.net
*/

/************************************************************************
*    SFONT - SDL Font Library by Karl Bartel <karlb@gmx.net>            *
*                                                                       *
*  All functions are explained below. For further information, take a   *
*  look at the example files, the links at the SFont web site, or       *
*  contact me, if you problem isn' addressed anywhere.                  *
*                                                                       *
************************************************************************/
//////////////////////////////////////////////////////////////////////////////
///@file SFont.h
/// Text rendering system, that uses bitmaps as fonts.
/// Not specific to Grafx2, it writes to SDL_Surface.
//////////////////////////////////////////////////////////////////////////////


#ifndef _SFONT_H_
#define _SFONT_H_

#include "gfx2surface.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// Declare one variable of this type for each font you are using.
/// To load the fonts, load the font image into YourFont->Surface
/// and call InitFont( YourFont );
typedef struct {
        T_GFX2_Surface *Surface;
        int CharBegin[256];
        int CharWidth[256];
        int Space;
        unsigned char Transparent;
} SFont_Font;

///
/// Initializes the font.
/// @param Font this contains the suface with the font.
///        The Surface must be loaded before calling this function
SFont_Font* SFont_InitFont (T_GFX2_Surface *Font);

///
/// Frees the font.
/// @param Font The font to free
///        The font must be loaded before using this function.
void SFont_FreeFont(SFont_Font* Font);

///
/// Blits a string to a surface.
/// @param Surface The surface you want to blit to.
/// @param Font    The font to use.
/// @param text    A string containing the text you want to blit.
/// @param x       Coordinates to start drawing.
/// @param y       Coordinates to start drawing.
void SFont_Write(T_GFX2_Surface *Surface, const SFont_Font *Font, int x, int y,
                                 const char *text);

/// Returns the width of "text" in pixels
int SFont_TextWidth(const SFont_Font* Font, const char *text);
/// Returns the height of "text" in pixels (which is always equal to Font->Surface->h)
int SFont_TextHeight(const SFont_Font* Font, const char *text);

/// Blits a string to Surface with centered x position
//void SFont_WriteCenter(SDL_Surface *Surface, const SFont_Font* Font, int y,
//                                           const char *text);

#ifdef __cplusplus
}
#endif

#endif /* SFONT_H */
