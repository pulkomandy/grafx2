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

#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "screen.h"
#include "misc.h"
#include "graph.h"
#include "pxtall3.h"

#define ZOOMX 3
#define ZOOMY 4


void Pixel_tall3 (word x,word y,byte color)
/* Affiche un pixel de la color aux coords x;y à l'écran */
{
  Set_Screen_pixel(x * ZOOMX,     y * ZOOMY,     color);
  Set_Screen_pixel(x * ZOOMX + 1, y * ZOOMY,     color);
  Set_Screen_pixel(x * ZOOMX + 2, y * ZOOMY,     color);
  Set_Screen_pixel(x * ZOOMX,     y * ZOOMY + 1, color);
  Set_Screen_pixel(x * ZOOMX + 1, y * ZOOMY + 1, color);
  Set_Screen_pixel(x * ZOOMX + 2, y * ZOOMY + 1, color);
  Set_Screen_pixel(x * ZOOMX,     y * ZOOMY + 2, color);
  Set_Screen_pixel(x * ZOOMX + 1, y * ZOOMY + 2, color);
  Set_Screen_pixel(x * ZOOMX + 2, y * ZOOMY + 2, color);
  Set_Screen_pixel(x * ZOOMX,     y * ZOOMY + 3, color);
  Set_Screen_pixel(x * ZOOMX + 1, y * ZOOMY + 3, color);
  Set_Screen_pixel(x * ZOOMX + 2, y * ZOOMY + 3, color);
}

byte Read_pixel_tall3 (word x,word y)
/* On retourne la couleur du pixel aux coords données */
{
  return Get_Screen_pixel(x * ZOOMX, y * ZOOMY);
}

void Block_tall3 (word start_x,word start_y,word width,word height,byte color)
/* On affiche un rectangle de la couleur donnée */
{
  Screen_FillRect(start_x * ZOOMX, start_y * ZOOMY, width * ZOOMX, height * ZOOMY, color);
}

void Display_part_of_screen_tall3 (word width,word height,word image_width)
/* Afficher une partie de l'image telle quelle sur l'écran */
{
  byte* src=Main.offset_Y*image_width+Main.offset_X+Main_screen; //Coords de départ ds la source (src)
  int y;
  int dy;

  for(y = 0; y < height; y++)
  // Pour chaque ligne
  {
    byte* dest = Get_Screen_pixel_ptr(0, y * ZOOMY);
    // On fait une copie de la ligne
    for (dy=width;dy>0;dy--)
    {
      *(dest+2)=*(dest+1)=*dest=*src;
      src++;
      dest+=ZOOMX;
    }
    // On double la ligne qu'on vient de copier
    memcpy(Get_Screen_pixel_ptr(0, y * ZOOMY + 1), Get_Screen_pixel_ptr(0, y * ZOOMY), width * ZOOMX);
    // On la triple
    memcpy(Get_Screen_pixel_ptr(0, y * ZOOMY + 2), Get_Screen_pixel_ptr(0, y * ZOOMY), width * ZOOMX);
    // On la quadruple
    memcpy(Get_Screen_pixel_ptr(0, y * ZOOMY + 3), Get_Screen_pixel_ptr(0, y * ZOOMY), width * ZOOMX);

    // On passe à la ligne suivante
    src+=image_width-width;
  }
  //Update_rect(0,0,width,height);
}

void Pixel_preview_normal_tall3 (word x,word y,byte color)
/* Affichage d'un pixel dans l'écran, par rapport au décalage de l'image 
 * dans l'écran, en mode normal (pas en mode loupe)
 * Note: si on modifie cette procédure, il faudra penser à faire également 
 * la modif dans la procédure Pixel_Preview_Loupe_SDL. */
{
//  if(x-Main.offset_X >= 0 && y - Main.offset_Y >= 0)
  Pixel_tall3(x-Main.offset_X,y-Main.offset_Y,color);
}

void Pixel_preview_magnifier_tall3  (word x,word y,byte color)
{
  // Affiche le pixel dans la partie non zoomée
  Pixel_tall3(x-Main.offset_X,y-Main.offset_Y,color);
  
  // Regarde si on doit aussi l'afficher dans la partie zoomée
  if (y >= Limit_top_zoom && y <= Limit_visible_bottom_zoom
          && x >= Limit_left_zoom && x <= Limit_visible_right_zoom)
  {
    // On est dedans
    int height;
    int y_zoom = Main.magnifier_factor * (y-Main.magnifier_offset_Y);

    if (Menu_Y - y_zoom < Main.magnifier_factor)
      // On ne doit dessiner qu'un morceau du pixel
      // sinon on dépasse sur le menu
      height = Menu_Y - y_zoom;
    else
      height = Main.magnifier_factor;

    Block_tall3(
      Main.magnifier_factor * (x-Main.magnifier_offset_X) + Main.X_zoom,
      y_zoom, Main.magnifier_factor, height, color
      );
  }
}

void Horizontal_XOR_line_tall3(word x_pos,word y_pos,word width)
{
  //On calcule la valeur initiale de dest:
  //byte* dest=y_pos*ZOOMY*VIDEO_LINE_WIDTH+x_pos*ZOOMX+Screen_pixels;

  int x;

  for (x=0;x<width*ZOOMX;x+=ZOOMX)
  {
    byte color = xor_lut[Get_Screen_pixel(x_pos * ZOOMX + x, y_pos * ZOOMY)];
    Set_Screen_pixel(x_pos * ZOOMX + x,     y_pos * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 1, y_pos * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 2, y_pos * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX + x,     y_pos * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 1, y_pos * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 2, y_pos * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX + x,     y_pos * ZOOMY + 2, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 1, y_pos * ZOOMY + 2, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 2, y_pos * ZOOMY + 2, color);
    Set_Screen_pixel(x_pos * ZOOMX + x,     y_pos * ZOOMY + 3, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 1, y_pos * ZOOMY + 3, color);
    Set_Screen_pixel(x_pos * ZOOMX + x + 2, y_pos * ZOOMY + 3, color);
  }
}

void Vertical_XOR_line_tall3(word x_pos,word y_pos,word height)
{
  int i;
  for (i=0; i<height; i++)
  {
    byte color = xor_lut[Get_Screen_pixel(x_pos * ZOOMX, (y_pos + i) * ZOOMY)];
    Set_Screen_pixel(x_pos * ZOOMX,     (y_pos + i) * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX + 1, (y_pos + i) * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX + 2, (y_pos + i) * ZOOMY, color);
    Set_Screen_pixel(x_pos * ZOOMX,     (y_pos + i) * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX + 1, (y_pos + i) * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX + 2, (y_pos + i) * ZOOMY + 1, color);
    Set_Screen_pixel(x_pos * ZOOMX,     (y_pos + i) * ZOOMY + 2, color);
    Set_Screen_pixel(x_pos * ZOOMX + 1, (y_pos + i) * ZOOMY + 2, color);
    Set_Screen_pixel(x_pos * ZOOMX + 2, (y_pos + i) * ZOOMY + 2, color);
  }
}

void Display_brush_color_tall3(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width)
{
  // dest = Position à l'écran
  // src = Position dans la brosse
  byte* src = Brush + y_offset * brush_width + x_offset;

  word x,y;

  // Pour chaque ligne
  for(y = 0; y < height; y++)
  {
    byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY);
    byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 1);
    byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 2);
    byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 3);
    // Pour chaque pixel
    for(x = width;x > 0; x--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*src != transp_color)
      {
        *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest = *src;
      }

      // Pixel suivant
      src++;
      dest+=ZOOMX;
      dest1+=ZOOMX;
      dest2+=ZOOMX;
      dest3+=ZOOMX;
    }

    // On passe à la ligne suivante
    src = src + brush_width - width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Display_brush_mono_tall3(word x_pos, word y_pos,
        word x_offset, word y_offset, word width, word height,
        byte transp_color, byte color, word brush_width)
/* On affiche la brosse en monochrome */
{
  // dest = adr destination à l'écran
  byte* src=brush_width*y_offset+x_offset+Brush; // src = adr ds 
      // la brosse
  int x,y;

  // Pour chaque ligne
  for(y = 0; y < height; y++)
  {
    byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY);
    byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 1);
    byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 2);
    byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 3);
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      if (*src!=transp_color)
        *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest = color;

      // On passe au pixel suivant
      src++;
      dest+=ZOOMX;
      dest1+=ZOOMX;
      dest2+=ZOOMX;
      dest3+=ZOOMX;
    }

    // On passe à la ligne suivante
    src+=brush_width-width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Clear_brush_tall3(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word image_width)
{
  byte* src = ( y_pos + Main.offset_Y ) * image_width + x_pos + Main.offset_X + Main_screen; //Coords de départ ds la source (src)
  int y;
  int x;
  (void)x_offset; // unused
  (void)y_offset; // unused
  (void)transp_color; // unused

  // Pour chaque ligne
  for(y = 0; y < height; y++)
  {
    byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY);
    byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 1);
    byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 2);
    byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 3);
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest = *src;

      // On passe au pixel suivant
      src++;
      dest+=ZOOMX;
      dest1+=ZOOMX;
      dest2+=ZOOMX;
      dest3+=ZOOMX;
    }

    // On passe à la ligne suivante
    src+=image_width-width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

// Affiche une brosse (arbitraire) à l'écran
void Display_brush_tall3(byte * brush, word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width)
{
  // dest = Position à l'écran
  // src = Position dans la brosse
  byte* src = brush + y_offset * brush_width + x_offset;
  
  word x,y;
  
  // Pour chaque ligne
  for(y = 0; y < height; y++)
  {
    byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY);
    byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 1);
    byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 2);
    byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 3);
    // Pour chaque pixel
    for(x = width;x > 0; x--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*src != transp_color)
      {
        *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest = *src;
      }

      // Pixel suivant
      src++; dest+=ZOOMX;
      dest1+=ZOOMX;
      dest2+=ZOOMX;
      dest3+=ZOOMX;
    }

    // On passe à la ligne suivante
    src = src + brush_width - width;
  }
}

void Remap_screen_tall3(word x_pos,word y_pos,word width,word height,byte * conversion_table)
{
  // dest = coords a l'écran
  int x,y;

  // Pour chaque ligne
  for(y = 0; y < height; y++)
  {
    byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY);
    byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 1);
    byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 2);
    byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, (y_pos + y) * ZOOMY + 3);
    // Pour chaque pixel
    for(x=width;x>0;x--)
    {
      *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest =
        conversion_table[*dest];
      dest +=ZOOMX;
      dest1+=ZOOMX;
      dest2+=ZOOMX;
      dest3+=ZOOMX;
    }
  }

  Update_rect(x_pos,y_pos,width,height);
}

void Display_line_on_screen_fast_tall3(word x_pos,word y_pos,word width,byte * line)
/* On affiche toute une ligne de pixels telle quelle. */
/* Utilisée si le buffer contient déja des pixel doublés. */
{
  memcpy(Get_Screen_pixel_ptr(+x_pos*ZOOMX, y_pos*ZOOMY), line, width*ZOOMX);
  memcpy(Get_Screen_pixel_ptr(+x_pos*ZOOMX, y_pos*ZOOMY+1), line, width*ZOOMX);
  memcpy(Get_Screen_pixel_ptr(+x_pos*ZOOMX, y_pos*ZOOMY+2), line, width*ZOOMX);
  memcpy(Get_Screen_pixel_ptr(+x_pos*ZOOMX, y_pos*ZOOMY+3), line, width*ZOOMX);
}

void Display_line_on_screen_tall3(word x_pos,word y_pos,word width,byte * line)
/* On affiche une ligne de pixels en les doublant. */
{
  int x;
  byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos * ZOOMY);
  byte* dest1 = Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos * ZOOMY + 1);
  byte* dest2 = Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos * ZOOMY + 2);
  byte* dest3 = Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos * ZOOMY + 3);
  for(x=width;x>0;x--)
  {
    *(dest3+2) =*(dest3+1) = *(dest3) = *(dest2+2) =*(dest2+1) = *(dest2) = *(dest1+2) =*(dest1+1) = *(dest1) = *(dest+2) =*(dest+1) = *dest = *line;
    dest+=ZOOMX;
    line++;
  }
}
void Display_transparent_mono_line_on_screen_tall3(
        word x_pos, word y_pos, word width, byte* line, 
        byte transp_color, byte color)
// Affiche une ligne à l'écran avec une couleur + transparence.
// Utilisé par les brosses en mode zoom
{
  byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos);
  int x;
  // Pour chaque pixel
  for(x=0;x<width;x++)
  {
    if (transp_color!=*line)
    {
      *(dest+2)=*(dest+1)=*dest=color;
    }
    line ++; // Pixel suivant
    dest+=ZOOMX;
  }
}

void Read_line_screen_tall3(word x_pos,word y_pos,word width,byte * line)
{
  memcpy(line, Get_Screen_pixel_ptr(x_pos * ZOOMX, y_pos * ZOOMY), width*ZOOMX);
}

void Display_part_of_screen_scaled_tall3(
        word width, // width non zoomée
        word height, // height zoomée
        word image_width,byte * buffer)
{
  byte* src = Main_screen + Main.magnifier_offset_Y * image_width
                      + Main.magnifier_offset_X;
  int y = 0; // Ligne en cours de traitement

  // Pour chaque ligne à zoomer
  while(1)
  {
    int x;
    
    // On éclate la ligne
    Zoom_a_line(src,buffer,Main.magnifier_factor*ZOOMX,width);
    // On l'affiche Facteur fois, sur des lignes consécutives
    x = Main.magnifier_factor/**ZOOMY*/;
    // Pour chaque ligne
    do{
      // On affiche la ligne zoomée
      Display_line_on_screen_fast_tall3(
        Main.X_zoom, y, width*Main.magnifier_factor,
        buffer
      );
      // On passe à la suivante
      y++;
      if(y==height/**ZOOMY*/)
      {
        Redraw_grid(Main.X_zoom,0,
          width*Main.magnifier_factor,height);
        Update_rect(Main.X_zoom,0,
          width*Main.magnifier_factor,height);
        return;
      }
      x--;
    }while (x > 0);
    src += image_width;
  }
// ATTENTION on n'arrive jamais ici !
}

// Affiche une partie de la brosse couleur zoomée
void Display_brush_color_zoom_tall3(word x_pos,word y_pos,
        word x_offset,word y_offset,
        word width, // width non zoomée
        word end_y_pos,byte transp_color,
        word brush_width, // width réelle de la brosse
        byte * buffer)
{
  byte* src = Brush+y_offset*brush_width + x_offset;
  word y = y_pos;
  byte bx;

  // Pour chaque ligne
  while(1)
  {
    Zoom_a_line(src,buffer,Main.magnifier_factor,width);
    // On affiche facteur fois la ligne zoomée
    for(bx=Main.magnifier_factor;bx>0;bx--)
    {
      byte* line_src = buffer;
      //byte* dest = Screen_pixels + y*ZOOMY * VIDEO_LINE_WIDTH + x_pos * ZOOMX;
      byte* dest = Get_Screen_pixel_ptr(x_pos * ZOOMX, y*ZOOMY);
      word x;
      // Pour chaque pixel de la ligne
      for(x = width*Main.magnifier_factor;x > 0;x--)
      {
        if(*line_src!=transp_color)
        {
          *(dest+2)=*(dest+1)=*dest = *line_src;
        }
        line_src++;
        dest+=ZOOMX;
      }
      // Double the line
      memcpy(Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY + 1), Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY), width*ZOOMX*Main.magnifier_factor);
      // Triple the line
      memcpy(Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY + 2), Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY), width*ZOOMX*Main.magnifier_factor);
      // Quadruple it
      memcpy(Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY + 3), Get_Screen_pixel_ptr(x_pos * ZOOMX, y * ZOOMY), width*ZOOMX*Main.magnifier_factor);
      y++;
      if(y==end_y_pos)
      {
        return;
      }
    }
    src += brush_width;
  }
  // ATTENTION zone jamais atteinte
}

void Display_brush_mono_zoom_tall3(word x_pos, word y_pos,
        word x_offset, word y_offset, 
        word width, // width non zoomée 
        word end_y_pos,
        byte transp_color, byte color, 
        word brush_width, // width réelle de la brosse
        byte * buffer
)

{
  byte* src = Brush + y_offset * brush_width + x_offset;
  int y=y_pos*ZOOMY;

  //Pour chaque ligne à zoomer :
  while(1)
  {
    int bx;
    // src = Ligne originale
    // On éclate la ligne
    Zoom_a_line(src,buffer,Main.magnifier_factor,width);

    // On affiche la ligne Facteur fois à l'écran (sur des
    // lignes consécutives)
    bx = Main.magnifier_factor*ZOOMY;

    // Pour chaque ligne écran
    do
    {
      // On affiche la ligne zoomée
      Display_transparent_mono_line_on_screen_tall3(
        x_pos, y, width * Main.magnifier_factor,
        buffer, transp_color, color
      );
      // On passe à la ligne suivante
      y++;
      // On vérifie qu'on est pas à la ligne finale
      if(y == end_y_pos*ZOOMY)
      {
        Redraw_grid( x_pos, y_pos,
          width * Main.magnifier_factor, end_y_pos - y_pos );
        Update_rect( x_pos, y_pos,
          width * Main.magnifier_factor, end_y_pos - y_pos );
        return;
      }
      bx --;
    }
    while (bx > 0);
    
    // Passage à la ligne suivante dans la brosse aussi
    src+=brush_width;
  }
}

void Clear_brush_scaled_tall3(word x_pos,word y_pos,word x_offset,word y_offset,word width,word end_y_pos,byte transp_color,word image_width,byte * buffer)
{

  // En fait on va recopier l'image non zoomée dans la partie zoomée !
  byte* src = Main_screen + y_offset * image_width + x_offset;
  int y = y_pos;
  int bx;
  (void)transp_color; // unused

  // Pour chaque ligne à zoomer
  while(1){
    Zoom_a_line(src,buffer,Main.magnifier_factor*ZOOMX,width);

    bx=Main.magnifier_factor;

    // Pour chaque ligne
    do{
      // TODO a verifier
      Display_line_on_screen_fast_tall3(x_pos,y,
        width * Main.magnifier_factor,buffer);

      // Ligne suivante
      y++;
      if(y==end_y_pos)
      {
        Redraw_grid(x_pos,y_pos,
          width*Main.magnifier_factor,end_y_pos-y_pos);
        Update_rect(x_pos,y_pos,
          width*Main.magnifier_factor,end_y_pos-y_pos);
        return;
      }
      bx--;
    }while(bx!=0);

    src+= image_width;
  }
}
