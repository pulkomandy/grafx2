/* vim:expandtab:ts=2 sw=2:
*/
/*  SFont: a simple font-library that uses special .pngs as fonts
    Copyright (C) 2003 Karl Bartel

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
#include <SDL.h>
#include <SDL_video.h>

#include <assert.h>
#include <stdlib.h>
#include "SFont.h"

static Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{
   Uint8  *bits;
   Uint32 Bpp;

   assert(X>=0);
   assert(X<Surface->w);
   
   Bpp = Surface->format->BytesPerPixel;
   bits = ((Uint8 *)Surface->pixels)+Y*Surface->pitch+X*Bpp;

   // Get the pixel
   switch(Bpp) {
      case 1:
         return *((Uint8 *)Surface->pixels + Y * Surface->pitch + X);
         break;
      case 2:
         return *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X);
         break;
      case 3: { // Format/endian independent 
         Uint8 r, g, b;
         r = *((bits)+Surface->format->Rshift/8);
         g = *((bits)+Surface->format->Gshift/8);
         b = *((bits)+Surface->format->Bshift/8);
         return SDL_MapRGB(Surface->format, r, g, b);
         }
         break;
      case 4:
         return *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X);
         break;
   }

   return -1;
}

SFont_Font* SFont_InitFont(SDL_Surface* Surface)
{
    int x = 0, i = 33;
    Uint32 pixel;
    SFont_Font* Font;
    Uint32 pink;

    if (Surface == NULL)
        return NULL;

    Font = (SFont_Font *) malloc(sizeof(SFont_Font));
    memset(Font, 0, sizeof(SFont_Font));
    
    Font->Surface = Surface;

    SDL_LockSurface(Surface);

    pink = GetPixel(Surface, 0, 0);
    while (x < Surface->w) {
        if (GetPixel(Surface, x, 0) != pink) { 
            Font->CharBegin[i]=x;
            while((x < Surface->w) && (GetPixel(Surface, x, 0)!= pink))
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
    
    pixel = GetPixel(Surface, 0, Surface->h-1);
    SDL_UnlockSurface(Surface);
    // No longer use SDL color keying
    //SDL_SetColorKey(Surface, SDL_SRCCOLORKEY, pixel);
    Font->Transparent=pixel;

    return Font;
}

void SFont_FreeFont(SFont_Font* FontInfo)
{
    SDL_FreeSurface(FontInfo->Surface);
    free(FontInfo);
}

void SFont_Write(SDL_Surface *Surface, const SFont_Font *Font,
                 int x, int y, const char *text)
{
    const char* c;
    SDL_Rect srcrect, dstrect;

    if(text == NULL)
        return;

    // these values won't change in the loop
    srcrect.y = 1;
    dstrect.y = y;
    srcrect.h = dstrect.h = Font->Surface->h - 1;

    for(c = text; *c != '\0' && x <= Surface->w ; c++) {
        // skip spaces and nonprintable characters
        if (*c == ' ' || Font->CharWidth[(int)*c]==0) {
            x += Font->Space;
            continue;
        }

        srcrect.w = Font->CharWidth[(int)*c];
        dstrect.w = srcrect.w;
        srcrect.x = Font->CharBegin[(int)*c];
        dstrect.x = x;

        SDL_BlitSurface(Font->Surface, &srcrect, Surface, &dstrect); 

        x += Font->CharWidth[(int)*c];
    }
}

int SFont_TextWidth(const SFont_Font *Font, const char *text)
{
    const char* c;
    int width = 0;

    if(text == NULL)
        return 0;

    for(c = text; *c != '\0'; c++) {
        // skip spaces and nonprintable characters
        if (*c == ' ' || Font->CharWidth[(int)*c]==0) {
            width += Font->Space;
            continue;
        }
        
        width += Font->CharWidth[(int)*c];
    }

    return width;
}

int SFont_TextHeight(const SFont_Font* Font)
{
    return Font->Surface->h - 1;
}

void SFont_WriteCenter(SDL_Surface *Surface, const SFont_Font *Font,
                       int y, const char *text)
{
    SFont_Write(Surface, Font, Surface->w/2 - SFont_TextWidth(Font, text)/2,
                y, text);
}

