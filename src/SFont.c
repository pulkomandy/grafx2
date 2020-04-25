/* vim:expandtab:ts=2 sw=2:
*/
/*  SFont: a simple font-library that uses special .pngs as fonts
    Copyright (C) 2003 Karl Bartel

    GrafX2 Modification
    Copyright (c) 2011 Yves Rizoud
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

/* This file is based on SFont 2.03 was modified for GrafX2:
  - SDL2 support
  - Automatic space width detection
  - Handling of missing lowercase characters
  - Multiline support
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "SFont.h"

SFont_Font* SFont_InitFont(T_GFX2_Surface * Surface)
{
    int x = 0, i = 33;
    byte pixel;
    SFont_Font* Font;
    byte pink;

    if (Surface == NULL)
        return NULL;

    Font = (SFont_Font *) malloc(sizeof(SFont_Font));
    memset(Font, 0, sizeof(SFont_Font));

    Font->Surface = Surface;

    pink = Get_GFX2_Surface_pixel(Surface, 0, 0);
    while (x < Surface->w) {
        if (Get_GFX2_Surface_pixel(Surface, x, 0) != pink) {
            Font->CharBegin[i]=x;
            while((x < Surface->w) && (Get_GFX2_Surface_pixel(Surface, x, 0)!= pink))
                x++;
            Font->CharWidth[i]=x-Font->CharBegin[i];
            i++;
        }
        x++;
    }

    // Create lowercase characters, if not present
    for (i=0; i <26; i++)
    {
      if (Font->CharWidth['a'+i]==0)
      {
        Font->CharBegin['a'+i]=Font->CharBegin['A'+i];
        Font->CharWidth['a'+i]=Font->CharWidth['A'+i];
      }
    }

    // Determine space width.
    // This strange format doesn't allow font designer to write explicit
    // space as a character.
    // Rule: A space should be as large as the character " if available,
    // or 'a' if it's not.
    Font->Space = Font->CharWidth[(int)'"'];
    if (Font->Space<2)
      Font->Space = Font->CharWidth[(int)'a'];

    pixel = Get_GFX2_Surface_pixel(Surface, 0, Surface->h-1);
    Font->Transparent=pixel;

    return Font;
}

void SFont_FreeFont(SFont_Font* FontInfo)
{
    Free_GFX2_Surface(FontInfo->Surface);
    free(FontInfo);
}

void SFont_Write(T_GFX2_Surface *Surface, const SFont_Font *Font,
                 int x, int y, const char *text)
{
    const char* c;
    int line;
    int height;

    if(text == NULL)
        return;

    height = Font->Surface->h - 1;

    for(c = text; *c != '\0' && x <= Surface->w ; c++) {
        if (*c == '\n') {
          y += height;
          x = 0;
          continue;
        }
        // skip spaces and nonprintable characters
        else if (*c == ' ' || Font->CharWidth[(int)*c]==0) {
            x += Font->Space;
            continue;
        }

        for (line = 0; line < height && (y + line) < Surface->h; line++) {
            memcpy(Surface->pixels + (y + line) * Surface->w + x,
                   Font->Surface->pixels + (line + 1) * Font->Surface->w + Font->CharBegin[(int)*c],
                   Font->CharWidth[(int)*c]);
        }

        x += Font->CharWidth[(int)*c];
    }
}

int SFont_TextWidth(const SFont_Font *Font, const char *text)
{
    const char* c;
    int width = 0;
    int previous_width = 0;

    if(text == NULL)
        return 0;

    for(c = text; *c != '\0'; c++)
    {
        if (*c == '\n')
        {
          if (previous_width<width)
            previous_width=width;
          width=0;
        }
        else
        // skip spaces and nonprintable characters
        if (*c == ' ' || Font->CharWidth[(int)*c]==0)
        {
            width += Font->Space;
            continue;
        }

        width += Font->CharWidth[(int)*c];
    }

    return previous_width<width ? width : previous_width;
}

int SFont_TextHeight(const SFont_Font* Font, const char *text)
{
    // Count occurrences of '\n'
    int nb_cr=0;
    while (*text!='\0')
    {
      if (*text=='\n')
        nb_cr++;
      text++;
    }

    return (Font->Surface->h - 1) * (nb_cr+1);
}

/*
// Do not use: Doesn't implement carriage returns

void SFont_WriteCenter(SDL_Surface *Surface, const SFont_Font *Font,
                       int y, const char *text)
{
    SFont_Write(Surface, Font, Surface->w/2 - SFont_TextWidth(Font, text)/2,
                y, text);
}
*/
