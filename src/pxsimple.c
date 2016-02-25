/* vim:expandtab:ts=2 sw=2:
*/
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include "global.h"
#include "sdlscreen.h"
#include "misc.h"
#include "graph.h"
#include "pxsimple.h"


void Pixel (word x,word y,byte color)
/* Affiche un pixel de la color aux coords x;y à l'écran */
{
  *(Screen_pixels + x + y * VIDEO_LINE_WIDTH)=color;
}

byte Read_pixel (word x,word y)
/* On retourne la couleur du pixel aux coords données */
{
  return *( Screen_pixels + y * VIDEO_LINE_WIDTH + x );
}

void Block (word start_x,word start_y,word width,word height,byte color)
/* On affiche un rectangle de la couleur donnée */
{
  SDL_Rect rectangle;
  rectangle.x=start_x;
  rectangle.y=start_y;
  rectangle.w=width;
  rectangle.h=height;
  SDL_FillRect(Screen_SDL,&rectangle,color);
}

void Display_screen (word width,word height,word image_width)
/* Afficher une partie de l'image telle quelle sur l'écran */
{
  byte* dest=Screen_pixels; //On va se mettre en 0,0 dans l'écran (dest)
  byte* src=Main_offset_Y*image_width+Main_offset_X+Main_screen; //Coords de départ ds la source (src)
  int y;

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    memcpy(dest,src,width);

    // On passe à la ligne suivante
    src+=image_width;
    dest+=VIDEO_LINE_WIDTH;
  }
  //Update_rect(0,0,width,height);
}

void Pixel_preview_normal (word x,word y,byte color)
/* Affichage d'un pixel dans l'écran, par rapport au décalage de l'image 
 * dans l'écran, en mode normal (pas en mode loupe)
 * Note: si on modifie cette procédure, il faudra penser à faire également 
 * la modif dans la procédure Pixel_Preview_Loupe_SDL. */
{
//  if(x-Main_offset_X >= 0 && y - Main_offset_Y >= 0)
  Pixel(x-Main_offset_X,y-Main_offset_Y,color);
}

void Pixel_preview_magnifier (word x,word y,byte color)
{
  Pixel_preview_normal (x,y,color);
/*
  // Affiche le pixel dans la partie non zoomée
  Pixel(x-Main_offset_X,y-Main_offset_Y,color);
  
  // Regarde si on doit aussi l'afficher dans la partie zoomée
  if (y >= Limit_top_zoom && y <= Limit_visible_bottom_zoom
          && x >= Limit_left_zoom && x <= Limit_visible_right_zoom)
  {
    // On est dedans
    int height;
    int y_zoom = Main_magnifier_factor * (y-Main_magnifier_offset_Y);

    if (Menu_Y - y_zoom < Main_magnifier_factor)
      // On ne doit dessiner qu'un morceau du pixel
      // sinon on dépasse sur le menu
      height = Menu_Y - y_zoom;
    else
      height = Main_magnifier_factor;

    Block(
      Main_magnifier_factor * (x-Main_magnifier_offset_X) + Main_X_zoom, 
      y_zoom, Main_magnifier_factor, height, color
      );
  }
  */
}

void Horizontal_XOR_line(word x_pos,word y_pos,word width)
{
  //On calcule la valeur initiale de dest:
  byte* dest=y_pos*VIDEO_LINE_WIDTH+x_pos+Screen_pixels;

  int x;

  for (x=0;x<width;x++)
    *(dest+x)=xor_lut[*(dest+x)];
}

void Vertical_XOR_line(word x_pos,word y_pos,word height)
{
  int i;
  byte color;
  for (i=y_pos;i<y_pos+height;i++)
  {
    color=*(Screen_pixels+x_pos+i*VIDEO_LINE_WIDTH);
    *(Screen_pixels+x_pos+i*VIDEO_LINE_WIDTH)=xor_lut[color];
  }
}

void Display_brush_color(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width)
{
  // dest = Position à l'écran
  byte* dest = Screen_pixels + y_pos * VIDEO_LINE_WIDTH + x_pos;
  // src = Position dans la brosse
  byte* src = Brush + y_offset * brush_width + x_offset;

  word x,y;

  // Pour chaque ligne
  for(y = height;y > 0; y--)
  {
    // Pour chaque pixel
    for(x = width;x > 0; x--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*src != transp_color)
      {
        *dest = *src;
      }

      // Pixel suivant
      src++; dest++;
    }

    // On passe à la ligne suivante
    dest = dest + VIDEO_LINE_WIDTH - width;
    src = src + brush_width - width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Display_brush_mono(word x_pos, word y_pos,
        word x_offset, word y_offset, word width, word height,
        byte transp_color, byte color, word brush_width)
/* On affiche la brosse en monochrome */
{
  byte* dest=y_pos*VIDEO_LINE_WIDTH+x_pos+Screen_pixels; // dest = adr Destination à 
      // l'écran
  byte* src=brush_width*y_offset+x_offset+Brush; // src = adr ds 
      // la brosse
  int x,y;

  for(y=height;y!=0;y--)
  //Pour chaque ligne
  {
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      if (*src!=transp_color)
        *dest=color;

      // On passe au pixel suivant
      src++;
      dest++;
    }

    // On passe à la ligne suivante
    src+=brush_width-width;
    dest+=VIDEO_LINE_WIDTH-width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Clear_brush(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word image_width)
{
  byte* dest=Screen_pixels+x_pos+y_pos*VIDEO_LINE_WIDTH; //On va se mettre en 0,0 dans l'écran (dest)
  byte* src = ( y_pos + Main_offset_Y ) * image_width + x_pos + Main_offset_X + Main_screen; //Coords de départ ds la source (src)
  int y;
  (void)x_offset; // unused
  (void)y_offset; // unused
  (void)transp_color; // unused

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    memcpy(dest,src,width);

    // On passe à la ligne suivante
    src+=image_width;
    dest+=VIDEO_LINE_WIDTH;
  }
  Update_rect(x_pos,y_pos,width,height);
}
