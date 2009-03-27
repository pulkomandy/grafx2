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
#include "divers.h"
#include "pxwide.h" // for Display_transparent_line_on_screen_wide()

#define ZOOMX 2
#define ZOOMY 2

void Pixel_double (word x,word y,byte color)
/* Affiche un pixel de la color aux coords x;y à l'écran */
{
  *(Screen + x * 2 + y * 4 * Screen_width)=color;
  *(Screen + x * 2 + y * 4 * Screen_width + 1)=color;
  *(Screen + x * 2 + (y * 4 + 2 )* Screen_width)=color;
  *(Screen + x * 2 + (y * 4 + 2 )* Screen_width + 1)=color;
}

byte Read_pixel_double (word x,word y)
/* On retourne la couleur du pixel aux coords données */
{
  return *( Screen + y * 4 * Screen_width + x * 2);
}

void Block_double (word start_x,word start_y,word width,word height,byte color)
/* On affiche un rectangle de la couleur donnée */
{
  SDL_Rect rectangle;
  rectangle.x=start_x*2;
  rectangle.y=start_y*2;
  rectangle.w=width*2;
  rectangle.h=height*2;
  SDL_FillRect(Screen_SDL,&rectangle,color);
}

void Display_part_of_screen_double (word width,word height,word image_width)
/* Afficher une partie de l'image telle quelle sur l'écran */
{
  byte* dest=Screen; //On va se mettre en 0,0 dans l'écran (dest)
  byte* src=Main_offset_Y*image_width+Main_offset_X+Main_screen; //Coords de départ ds la source (src)
  int y;
  int dy;

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    for (dy=width;dy>0;dy--)
    {
      *(dest+1)=*dest=*src;
      src++;
      dest+=2;
    }
    // On double la ligne qu'on vient de copier
    memcpy(dest-width*ZOOMX+Screen_width*ZOOMX,dest-width*ZOOMX,width*ZOOMX);
    
    // On passe à la ligne suivante
    src+=image_width-width;
    dest+=Screen_width*4 - width*2;
  }
  //Update_rect(0,0,width,height);
}

void Pixel_preview_normal_double (word x,word y,byte color)
/* Affichage d'un pixel dans l'écran, par rapport au décalage de l'image 
 * dans l'écran, en mode normal (pas en mode loupe)
 * Note: si on modifie cette procédure, il faudra penser à faire également 
 * la modif dans la procédure Pixel_Preview_Loupe_SDL. */
{
//  if(x-Main_offset_X >= 0 && y - Main_offset_Y >= 0)
  Pixel_double(x-Main_offset_X,y-Main_offset_Y,color);
}

void Pixel_preview_magnifier_double  (word x,word y,byte color)
{
  // Affiche le pixel dans la partie non zoomée
  Pixel_double(x-Main_offset_X,y-Main_offset_Y,color);
  
  // Regarde si on doit aussi l'afficher dans la partie zoomée
  if (y >= Limit_top_zoom && y <= Limit_visible_bottom_zoom
          && x >= Limit_left_zoom && x <= Limit_visible_right_zoom)
  {
    // On est dedans
    int height;
    int y_zoom = Zoom_factor_table[y-Main_magnifier_offset_Y];

    if (Menu_Y - y_zoom < Main_magnifier_factor)
      // On ne doit dessiner qu'un morceau du pixel
      // sinon on dépasse sur le menu
      height = Menu_Y - y_zoom;
    else
      height = Main_magnifier_factor;

    Block_double(
      Zoom_factor_table[x-Main_magnifier_offset_X]+Main_X_zoom, 
      y_zoom, Main_magnifier_factor, height, color
      );
  }
}

void Horizontal_XOR_line_double(word x_pos,word y_pos,word width)
{
  //On calcule la valeur initiale de dest:
  byte* dest=y_pos*4*Screen_width+x_pos*2+Screen;

  int x;

  for (x=0;x<width*2;x+=2)
    *(dest+x+2*Screen_width+1)=*(dest+x+2*Screen_width)=*(dest+x+1)=*(dest+x)=~*(dest+x);
}

void Vertical_XOR_line_double(word x_pos,word y_pos,word height)
{
  int i;
  byte *dest=Screen+x_pos*2+y_pos*Screen_width*4;
  for (i=height;i>0;i--)
  {
    *dest=*(dest+1)=*(dest+Screen_width*2)=*(dest+Screen_width*2+1)=~*dest;
    dest+=Screen_width*4;
  }
}

void Display_brush_color_double(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width)
{
  // dest = Position à l'écran
  byte* dest = Screen + y_pos * 4 * Screen_width + x_pos * 2;
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
        *(dest+Screen_width*2+1) = *(dest+Screen_width*2) = *(dest+1) = *dest = *src;
      }

      // Pixel suivant
      src++;
      dest+=2;
    }

    // On passe à la ligne suivante
    dest = dest + Screen_width*4 - width*2;
    src = src + brush_width - width;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Display_brush_mono_double(word x_pos, word y_pos,
        word x_offset, word y_offset, word width, word height,
        byte transp_color, byte color, word brush_width)
/* On affiche la brosse en monochrome */
{
  byte* dest=y_pos*4*Screen_width+x_pos*2+Screen; // dest = adr destination à 
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
        *(dest+Screen_width*2+1)=*(dest+Screen_width*2)=*(dest+1)=*dest=color;

      // On passe au pixel suivant
      src++;
      dest+=2;
    }

    // On passe à la ligne suivante
    src+=brush_width-width;
    dest+=Screen_width*4-width*2;
  }
  Update_rect(x_pos,y_pos,width,height);
}

void Clear_brush_double(word x_pos,word y_pos,__attribute__((unused)) word x_offset,__attribute__((unused)) word y_offset,word width,word height,__attribute__((unused))byte transp_color,word image_width)
{
  byte* dest=Screen+x_pos*2+y_pos*4*Screen_width; //On va se mettre en 0,0 dans l'écran (dest)
  byte* src = ( y_pos + Main_offset_Y ) * image_width + x_pos + Main_offset_X + Main_screen; //Coords de départ ds la source (src)
  int y;
  int x;

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      *(dest+Screen_width*2+1)=*(dest+Screen_width*2)=*(dest+1)=*dest=*src;

      // On passe au pixel suivant
      src++;
      dest+=2;
    }

    // On passe à la ligne suivante
    src+=image_width-width;
    dest+=Screen_width*4-width*2;
  }
  Update_rect(x_pos,y_pos,width,height);
}

// Affiche une brosse (arbitraire) à l'écran
void Display_brush_double(byte * brush, word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width)
{
  // dest = Position à l'écran
  byte* dest = Screen + y_pos * 4 * Screen_width + x_pos * 2;
  // src = Position dans la brosse
  byte* src = brush + y_offset * brush_width + x_offset;
  
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
        *(dest+Screen_width*2+1)=*(dest+Screen_width*2)=*(dest+1)=*dest=*src;
      }

      // Pixel suivant
      src++; dest+=2;
    }

    // On passe à la ligne suivante
    dest = dest + Screen_width*4 - width*2;
    src = src + brush_width - width;
  }
}

void Remap_screen_double(word x_pos,word y_pos,word width,word height,byte * conversion_table)
{
  // dest = coords a l'écran
  byte* dest = Screen + y_pos * 4 * Screen_width + x_pos * 2;
  int x,y;

  // Pour chaque ligne
  for(y=height;y>0;y--)
  {
    // Pour chaque pixel
    for(x=width;x>0;x--)
    {
      *(dest+Screen_width*2+1)=*(dest+Screen_width*2)=*(dest+1)=*dest=
        conversion_table[*dest];
      dest +=2;
    }

    dest = dest + Screen_width*4 - width*2;
  }

  Update_rect(x_pos,y_pos,width,height);
}

void Display_line_on_screen_fast_double(word x_pos,word y_pos,word width,byte * line)
/* On affiche toute une ligne de pixels telle quelle. */
/* Utilisée si le buffer contient déja des pixel doublés. */
{
  memcpy(Screen+x_pos*2+y_pos*4*Screen_width,line,width*2);
  memcpy(Screen+x_pos*2+(y_pos*4+2)*Screen_width,line,width*2);
}

void Display_line_on_screen_double(word x_pos,word y_pos,word width,byte * line)
/* On affiche une ligne de pixels en les doublant. */
{
  int x;
  byte *dest;
  dest=Screen+x_pos*2+y_pos*4*Screen_width;
  for(x=width;x>0;x--)
  {
    *(dest+Screen_width*2+1)=*(dest+Screen_width*2)=*(dest+1)=*dest=*line;
    dest+=2;
    line++;
  }
}
void Display_transparent_mono_line_on_screen_double(
        word x_pos, word y_pos, word width, byte* line, 
        byte transp_color, byte color)
// Affiche une ligne à l'écran avec une couleur + transparence.
// Utilisé par les brosses en mode zoom
{
  byte* dest = Screen+ y_pos*ZOOMX*Screen_width + x_pos*ZOOMX;
  int x;
  // Pour chaque pixel
  for(x=0;x<width;x++)
  {
    if (transp_color!=*line)
    {
      *(dest+1)=*dest=color;
    }
    line ++; // Pixel suivant
    dest+=2;
  }
}

void Read_line_screen_double(word x_pos,word y_pos,word width,byte * line)
{
  memcpy(line,Screen_width * 4 * y_pos + x_pos * 2 + Screen,width*2);
}

void Display_part_of_screen_scaled_double(
        word width, // width non zoomée
        word height, // height zoomée
        word image_width,byte * buffer)
{
  byte* src = Main_screen + Main_magnifier_offset_Y * image_width 
                      + Main_magnifier_offset_X;
  int y = 0; // Ligne en cours de traitement

  // Pour chaque ligne à zoomer
  while(1)
  {
    int x;
    
    // On éclate la ligne
    Zoom_a_line(src,buffer,Main_magnifier_factor*ZOOMX,width);
    // On l'affiche Facteur fois, sur des lignes consécutives
    x = Main_magnifier_factor/**ZOOMY*/;
    // Pour chaque ligne
    do{
      // On affiche la ligne zoomée
      Display_line_on_screen_fast_double(
        Main_X_zoom, y, width*Main_magnifier_factor,
        buffer
      );
      // On passe à la suivante
      y++;
      if(y==height/**ZOOMY*/)
      {
        Update_rect(Main_X_zoom,0,
          width*Main_magnifier_factor,height);
        return;
      }
      x--;
    }while (x > 0);
    src += image_width;
  }
// ATTENTION on n'arrive jamais ici !
}

// Affiche une partie de la brosse couleur zoomée
void Display_brush_color_zoom_double(word x_pos,word y_pos,
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
    Zoom_a_line(src,buffer,Main_magnifier_factor,width);
    // On affiche facteur fois la ligne zoomée
    for(bx=Main_magnifier_factor;bx>0;bx--)
    {
      Display_transparent_line_on_screen_wide(x_pos,y*ZOOMX,width*Main_magnifier_factor,buffer,transp_color);
      // TODO: pas clair ici
      memcpy(Screen + (y*ZOOMY+1)*ZOOMX*Screen_width + x_pos*ZOOMX, Screen + y*ZOOMX*ZOOMY*Screen_width + x_pos*ZOOMX, width*ZOOMX*Main_magnifier_factor);
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

void Display_brush_mono_zoom_double(word x_pos, word y_pos,
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
    Zoom_a_line(src,buffer,Main_magnifier_factor,width);

    // On affiche la ligne Facteur fois à l'écran (sur des
    // lignes consécutives)
    bx = Main_magnifier_factor*ZOOMX;

    // Pour chaque ligne écran
    do
    {
      // On affiche la ligne zoomée
      Display_transparent_mono_line_on_screen_double(
        x_pos, y, width * Main_magnifier_factor, 
        buffer, transp_color, color
      );
      // On passe à la ligne suivante
      y++;
      // On vérifie qu'on est pas à la ligne finale
      if(y == end_y_pos*ZOOMX)
      {
        Update_rect( x_pos, y_pos,
          width * Main_magnifier_factor, end_y_pos - y_pos );
        return;
      }
      bx --;
    }
    while (bx > 0);
    
    // Passage à la ligne suivante dans la brosse aussi
    src+=brush_width;
  }
}

void Clear_brush_scaled_double(word x_pos,word y_pos,word x_offset,word y_offset,word width,word end_y_pos,__attribute__((unused)) byte transp_color,word image_width,byte * buffer)
{

  // En fait on va recopier l'image non zoomée dans la partie zoomée !
  byte* src = Main_screen + y_offset * image_width + x_offset;
  int y = y_pos;
  int bx;

  // Pour chaque ligne à zoomer
  while(1){
    Zoom_a_line(src,buffer,Main_magnifier_factor*2,width);

    bx=Main_magnifier_factor;

    // Pour chaque ligne
    do{
      // TODO a verifier
      Display_line_on_screen_fast_double(x_pos,y,
        width * Main_magnifier_factor,buffer);

      // Ligne suivante
      y++;
      if(y==end_y_pos)
      {
        Update_rect(x_pos,y_pos,
          width*Main_magnifier_factor,end_y_pos-y_pos);
        return;
      }
      bx--;
    }while(bx!=0);

    src+= image_width;
  }
}


