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
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "struct.h"
#include "global.h"
#include "errors.h"
#include "buttons.h"
#include "engine.h"
#include "misc.h"
#include "osdep.h"
#include "keyboard.h"
#include "screen.h"
#include "windows.h"
#include "palette.h"
#include "input.h"
#include "graph.h"
#include "pages.h"

///Count used palette indexes in the whole picture
///Return the total number of different colors
///Fill in "usage" with the count for each color
word Count_used_colors(dword* usage)
{
  int nb_pixels = 0;
  byte* current_pixel;
  byte color;
  word nb_colors = 0;
  int i;
  int layer;

  for (i = 0; i < 256; i++) usage[i]=0;

  // Compute total number of pixels in the picture
  nb_pixels = Main.image_height * Main.image_width;

  // For each layer
  for (layer = 0; layer < Main.backups->Pages->Nb_layers; layer++)
  {
    current_pixel = Main.backups->Pages->Image[layer].Pixels;
    // For each pixel in picture
    for (i = 0; i < nb_pixels; i++)
    {
      color=*current_pixel; // get color in picture for this pixel
  
      usage[color]++; // add it to the counter
  
      // go to next pixel
      current_pixel++;
    }
  }

  // count the total number of unique used colors
  for (i = 0; i < 256; i++)
  {
    if (usage[i]!=0)
      nb_colors++;
  }

  return nb_colors;
}

/// Same as ::Count_used_colors, but use a block screen memory instead of
/// picture data. Used to count colors in the loading screen.
word Count_used_colors_screen_area(dword* usage, word start_x, word start_y,
  word width, word height)
{
  byte color;
  word x, y;
  word nb_colors = 0;
  int i;

  // Init usage table
  for (i = 0; i < 256; i++) usage[i]=0;

  // For each pixel in screen area
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      // Get color in screen memory
      //color=*(Screen_pixels+((start_x + x)+(start_y + y) * Screen_width
      //  * Pixel_height) * Pixel_width);
      color = Get_Screen_pixel(start_x + x, start_y + y);
      usage[color]++; //Un point de plus pour cette couleur
    }
  }
  //On va maintenant compter dans la table les couleurs utilisées:
  for (i = 0; i < 256; i++)
  {
    if (usage[i]!=0)
      nb_colors++;
  }
  return nb_colors;
}


/// Same as ::Count_used_colors, but for a given rectangle in the picture only.
/// Used bu the C64 block constraint checker.
word Count_used_colors_area(dword* usage, word start_x, word start_y,
  word width, word height)
{
  byte color;
  word x, y;
  word nb_colors = 0;
  int i;

  // Init usage table
  for (i = 0; i < 256; i++) usage[i]=0;

  // On parcourt l'écran courant pour compter les utilisations des couleurs
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      // Get color from picture
      color=*(Main_screen+((start_x + x)+(start_y + y)*Main.image_width));
      usage[color]++; //Un point de plus pour cette couleur
    }
  }

  //On va maintenant compter dans la table les couleurs utilisées:
  for (i = 0; i < 256; i++)
  {
    if (usage[i]!=0)
      nb_colors++;
  }
  return nb_colors;
}


// Backup of the currently displayed palette.
// It is not always Main_palette ! (for example during a preview)
// external code must not modify this array but use Set_palette() / Set_color()
// Get_current_palette() offers a READ-ONLY access.
static T_Palette Current_palette;

const T_Components * Get_current_palette(void)
{
  return Current_palette;
}

void Set_palette(T_Palette palette)
{
  int i;

  memcpy(Current_palette, palette, sizeof(T_Palette));
  for(i=0;i<256;i++)
  {
    palette[i].R = Round_palette_component(palette[i].R);
    palette[i].G = Round_palette_component(palette[i].G);
    palette[i].B = Round_palette_component(palette[i].B);
  }
  GFX2_SetPalette(palette, 0, 256);
}

void Set_color(byte color, byte red, byte green, byte blue)
{
  Current_palette[color].R = red;
  Current_palette[color].G = green;
  Current_palette[color].B = blue;
  GFX2_SetPalette(Current_palette + color, color, 1);
}

void Wait_end_of_click(void)
{
  // On désactive tous les raccourcis clavier

  while(Mouse_K)
    Get_input(20);
}

void Clear_current_image_with_stencil(byte color, byte * stencil)
  //Effacer l'image courante avec une certaine couleur en mode Stencil
{
  int nb_pixels=0; //ECX
  //al=color
  //edi=Screen_pixels
  byte* pixel=Main.backups->Pages->Image[Main.current_layer].Pixels;
  int i;

  nb_pixels=Main.image_height*Main.image_width;

  for(i=0;i<nb_pixels;i++)
  {
    if (stencil[*pixel]==0)
      *pixel=color;
    pixel++;
  }
}

void Clear_current_image(byte color)
  // Effacer l'image courante avec une certaine couleur
{
  memset(
    Main.backups->Pages->Image[Main.current_layer].Pixels,
    color ,
    Main.image_width * Main.image_height
    );
}

void Init_chrono(dword delay)
  // Démarrer le chrono
{
  Timer_delay = delay;
  Timer_start = GFX2_GetTicks()/55;
  return;
}

void Pixel_in_brush (word x, word y, byte color)
{
  *(Brush + y * Brush_width + x)=color;
}

byte Read_pixel_from_brush (word x, word y)
{
  return *(Brush + y * Brush_width + x);
}

void Copy_part_of_image_to_another(byte * source,word source_x,word source_y,word width,word height,word source_width,byte * dest,word dest_x,word dest_y,word destination_width)
{
  // ESI = adresse de la source en (S_Pox_X,source_y)
  byte* esi = source + source_y * source_width + source_x;

  // EDI = adresse de la destination (dest_x,dest_y)
  byte* edi = dest + dest_y * destination_width + dest_x;

  int line;

  // Pour chaque ligne
  for (line=0;line < height; line++)
  {
    memcpy(edi,esi,width);

    // Passe à la ligne suivante
    esi+=source_width;
    edi+=destination_width;
  }
}

byte Read_pixel_from_spare_screen(word x,word y)
{
//  return *(Spare_screen+y*Spare.image_width+x);

  // Clipping is required as this can be called with coordinates from main image
  // (can be a bigger or smaller image)
  if (x>=Spare.image_width || y>=Spare.image_height)
    return Spare.backups->Pages->Transparent_color;
  if (Spare.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
  {
    return *(Spare.backups->Pages->Image[Spare.current_layer].Pixels + y*Spare.image_width + x);
  }
  else
  {
    return *(Spare.visible_image.Image + y*Spare.image_width + x);
  }
}

void Rotate_90_deg_lowlevel(byte * source, byte * dest, short width, short height)
{
  word x,y;

  for(y=0;y<height;y++)
  {
    for(x=0;x<width;x++)
    {
      *(dest+height*(width-1-x)+y)=*source;
      source++;  
    }
  }
}

void Rotate_270_deg_lowlevel(byte * source, byte * dest, short width, short height)
{
  word x,y;

  for(y=0;y<height;y++)
  {
    for(x=0;x<width;x++)
    {
      *(dest+(height-1-y)+x*height)=*source;
      source++;  
    }
  }
}

// Replace une couleur par une autre dans un buffer

void Remap_general_lowlevel(byte * conversion_table,byte * in_buffer, byte *out_buffer,short width,short height,short buffer_width)
{
  int dx,cx;

  // Pour chaque ligne
  for(dx=height;dx>0;dx--)
  {
    // Pour chaque pixel
    for(cx=width;cx>0;cx--)
    {
      *out_buffer = conversion_table[*in_buffer];
      in_buffer++;
      out_buffer++;
    }
    in_buffer += buffer_width-width;
    out_buffer += buffer_width-width;
  }
}

void Copy_image_to_brush(short start_x,short start_y,short Brush_width,short Brush_height,word image_width)
{
  byte* src=start_y*image_width+start_x+Main.backups->Pages->Image[Main.current_layer].Pixels; //Adr départ image (ESI)
  byte* dest=Brush_original_pixels; //Adr dest brosse (EDI)
  int dx;

  for (dx=Brush_height;dx!=0;dx--)
    //Pour chaque ligne
  {

    // On fait une copie de la ligne
    memcpy(dest,src,Brush_width);

    // On passe à la ligne suivante
    src+=image_width;
    dest+=Brush_width;
  }

}

byte Read_pixel_from_feedback_screen (word x,word y)
{
  return *(FX_feedback_screen+y*Main.image_width+x);
}

dword Round_div(dword numerator,dword divisor)
{
  return numerator/divisor;
}

byte Effect_sieve(word x,word y)
{
  return Sieve[x % Sieve_width][y % Sieve_height];
}

void Replace_colors_within_limits(byte * replace_table)
{
  int y;
  int x;
  byte* pixel;

  // Pour chaque ligne :
  for(y = Limit_top;y <= Limit_bottom; y++)
  {
    // Pour chaque pixel sur la ligne :
    for (x = Limit_left;x <= Limit_right;x ++)
    {
      pixel = Main.backups->Pages->Image[Main.current_layer].Pixels+y*Main.image_width+x;
      *pixel = replace_table[*pixel];
    }
  }
}

byte Read_pixel_from_backup_screen (word x,word y)
{
  return *(Screen_backup + x + Main.image_width * y);
}

byte Effect_interpolated_colorize  (word x,word y,byte color)
{
  // factor_a = 256*(100-Colorize_opacity)/100
  // factor_b = 256*(    Colorize_opacity)/100
  //
  // (Couleur_dessous*factor_a+color*facteur_B)/256
  //

  // On place dans ESI 3*Couleur_dessous ( = position de cette couleur dans la
  // palette des teintes) et dans EDI, 3*color.
  byte color_under = Read_pixel_from_feedback_screen(x,y);
  byte blue_under=Main.palette[color_under].B;
  byte blue=Main.palette[color].B;
  byte green_under=Main.palette[color_under].G;
  byte green=Main.palette[color].G;
  byte red_under=Main.palette[color_under].R;
  byte red=Main.palette[color].R;

  // On récupère les 3 composantes RVB

  // blue
  blue = (Factors_inv_table[blue]
      + Factors_table[blue_under]) / 256;
  green = (Factors_inv_table[green]
      + Factors_table[green_under]) / 256;
  red = (Factors_inv_table[red]
      + Factors_table[red_under]) / 256;
  return Best_color(red,green,blue);

}

byte Effect_additive_colorize    (word x,word y,byte color)
{
  byte color_under = Read_pixel_from_feedback_screen(x,y);
  byte blue_under=Main.palette[color_under].B;
  byte green_under=Main.palette[color_under].G;
  byte red_under=Main.palette[color_under].R;
  byte blue=Main.palette[color].B;
  byte green=Main.palette[color].G;
  byte red=Main.palette[color].R;

  return Best_color(
    red>red_under?red:red_under,
    green>green_under?green:green_under,
    blue>blue_under?blue:blue_under);
}

byte Effect_substractive_colorize(word x,word y,byte color)
{
  byte color_under = Read_pixel_from_feedback_screen(x,y);
  byte blue_under=Main.palette[color_under].B;
  byte green_under=Main.palette[color_under].G;
  byte red_under=Main.palette[color_under].R;
  byte blue=Main.palette[color].B;
  byte green=Main.palette[color].G;
  byte red=Main.palette[color].R;

  return Best_color(
    red<red_under?red:red_under,
    green<green_under?green:green_under,
    blue<blue_under?blue:blue_under);
}

byte Effect_alpha_colorize    (word x,word y,byte color)
{
  byte color_under = Read_pixel_from_feedback_screen(x,y);
  byte blue_under=Main.palette[color_under].B;
  byte green_under=Main.palette[color_under].G;
  byte red_under=Main.palette[color_under].R;
  int factor=(Main.palette[color].R*76 +
    Main.palette[color].G*151 +
    Main.palette[color].B*28)/255;

  return Best_color(
    (Main.palette[Fore_color].R*factor + red_under*(255-factor))/255,
    (Main.palette[Fore_color].G*factor + green_under*(255-factor))/255,
    (Main.palette[Fore_color].B*factor + blue_under*(255-factor))/255);
}

void Check_timer(void)
{
  if((GFX2_GetTicks()/55)-Timer_delay>Timer_start) Timer_state=1;
}

void Flip_Y_lowlevel(byte *src, short width, short height)
{
  // ESI pointe sur la partie haute de la brosse
  // EDI sur la partie basse
  byte* ESI = src ;
  byte* EDI = src + (height - 1) *width;
  byte tmp;
  word cx;

  while(ESI < EDI)
  {
    // Il faut inverser les lignes pointées par ESI et
    // EDI ("Brush_width" octets en tout)

    for(cx = width;cx>0;cx--)
    {
      tmp = *ESI;
      *ESI = *EDI;
      *EDI = tmp;
      ESI++;
      EDI++;
    }

    // On change de ligne :
    // ESI pointe déjà sur le début de la ligne suivante
    // EDI pointe sur la fin de la ligne en cours, il
    // doit pointer sur le début de la précédente...
    EDI -= 2 * width; // On recule de 2 lignes
  }
}

void Flip_X_lowlevel(byte *src, short width, short height)
{
  // ESI pointe sur la partie gauche et EDI sur la partie
  // droite
  byte* ESI = src;
  byte* EDI = src + width - 1;

  byte* line_start;
  byte* line_end;
  byte tmp;
  word cx;

  while(ESI<EDI)
  {
    line_start = ESI;
    line_end = EDI;

    // On échange par colonnes
    for(cx=height;cx>0;cx--)
    {
      tmp=*ESI;
      *ESI=*EDI;
      *EDI=tmp;
      EDI+=width;
      ESI+=width;
    }

    // On change de colonne
    // ESI > colonne suivante
    // EDI > colonne précédente
    ESI = line_start + 1;
    EDI = line_end - 1;
  }
}

// Rotate a pixel buffer 180º on itself.
void Rotate_180_deg_lowlevel(byte *src, short width, short height)
{
  // ESI pointe sur la partie supérieure de la brosse
  // EDI pointe sur la partie basse
  byte* ESI = src;
  byte* EDI = src + height*width - 1;
  // EDI pointe sur le dernier pixel de la derniere ligne
  byte tmp;
  word cx;

  // In case of odd height, the algorithm in this function would
  // miss the middle line, so we do it this way:
  if (height & 1)
  {
    Flip_X_lowlevel(src, width, height);
    Flip_Y_lowlevel(src, width, height);
    return;
  }


  while(ESI < EDI)
  {
    // On échange les deux lignes pointées par EDI et
    // ESI (Brush_width octets)
    // En même temps, on échange les pixels, donc EDI
    // pointe sur la FIN de sa ligne

    for(cx=width;cx>0;cx--)
    {
      tmp = *ESI;
      *ESI = *EDI;
      *EDI = tmp;

      EDI--; // Attention ici on recule !
      ESI++;
    }
  }
}

void Rescale(byte *src_buffer, short src_width, short src_height, byte *dst_buffer, short dst_width, short dst_height, short x_flipped, short y_flipped)
{
  int    offset,line,column;

  int    x_pos_in_brush;   // Position courante dans l'ancienne brosse
  int    y_pos_in_brush;
  int    initial_x_pos;       // Position X de début de parcours de ligne
  int    initial_y_pos;       // Position Y de début de parcours de ligne

  int	delta_x, delta_y;

  offset=0;

  // Calcul de la valeur initiale de y_pos:
  if (y_flipped) {
    initial_y_pos=(src_height)-1; // Inversion en Y de la brosse
	delta_y = -1 * src_height;
  } else {
    initial_y_pos=0;                // Pas d'inversion en Y de la brosse
	delta_y = src_height;
  }

  // Calcul de la valeur initiale de x_pos pour chaque ligne:
  if (x_flipped) {
    initial_x_pos = (src_width)-1; // Inversion en X de la brosse
	delta_x = -1 * src_width;
  } else {
    initial_x_pos = 0;                // Pas d'inversion en X de la brosse
	delta_x = src_width;
  }

  // Pour chaque ligne
  for (line=0;line<dst_height;line++)
  {
    // On passe à la ligne de brosse suivante:
    y_pos_in_brush = initial_y_pos + line * delta_y / dst_height;

    // Pour chaque colonne:
    for (column=0;column<dst_width;column++)
    {
      // On passe à la colonne de brosse suivante:
      x_pos_in_brush = initial_x_pos + column * delta_x / dst_width;
      // On copie le pixel:
      dst_buffer[offset]=*(src_buffer + x_pos_in_brush + y_pos_in_brush * src_width);
      // On passe au pixel suivant de la nouvelle brosse:
      offset++;
    }
  }
}


void Scroll_picture(byte * main_src, byte * main_dest, short x_offset,short y_offset)
{
  byte* src = main_src; //source de la copie
  byte* dest = main_dest + y_offset * Main.image_width + x_offset;
  const word length = Main.image_width - x_offset; // Nombre de pixels à copier à droite
  word y;
  for(y = Main.image_height - y_offset;y>0;y--)
  {
    // Pour chaque ligne
    memcpy(dest,src,length);
    memcpy(dest - x_offset,src+length,x_offset);

    // On passe à la ligne suivante
    dest += Main.image_width;
    src += Main.image_width;
  }

  // On vient de faire le traitement pour otutes les lignes au-dessous de y_offset
  // Maintenant on traite celles au dessus
  dest = x_offset + main_dest;
  for(y = y_offset;y>0;y--)
  {
    memcpy(dest,src,length);
    memcpy(dest - x_offset,src+length,x_offset);

    dest += Main.image_width;
    src += Main.image_width;
  }

  Update_rect(0,0,0,0);
}

void Zoom_a_line(byte* original_line, byte* zoomed_line,
    word factor, word width
    )
{
  byte color;
  word x;

  // Pour chaque pixel
  for(x=0;x<width;x++){
    color = *original_line;

    memset(zoomed_line,color,factor);
    zoomed_line+=factor;

    original_line++;
  }
}

/*############################################################################*/

// Arrondir un nombre réel à la valeur entière la plus proche
// TODO : this should probably be replaced with round() from C99...
short Round(float value)
{
  short temp=value;

  if (value>=0)
  { if ((value-temp)>= 0.5) temp++; }
  else
  { if ((value-temp)<=-0.5) temp--; }

  return temp;
}


// Arrondir le résultat d'une division à la valeur entière supérieure
short Round_div_max(short numerator,short divisor)
{
  if (!(numerator % divisor))
    return (numerator/divisor);
  else
    return (numerator/divisor)+1;
}


// Retourne le minimum entre deux nombres
int Min(int a,int b)
{
  return (a<b)?a:b;
}


// Retourne le maximum entre deux nombres
int Max(int a,int b)
{
  return (a>b)?a:b;
}

/* Round number n to d decimal points */
double Fround(double n, unsigned d)
{
  double exp;
  exp = pow(10.0, d);
  return floor(n * exp + 0.5) / exp;
}


// Fonction retournant le libellé d'une mode (ex: " 320x200")
const char * Mode_label(int mode)
{
  static char str[24];
  if (! Video_mode[mode].Fullscreen)
    return "window";
  sprintf(str, "%dx%d", Video_mode[mode].Width, Video_mode[mode].Height);

  return str;
}


// Trouve un mode video à partir d'une chaine: soit "window",
// soit de la forme "320x200"
// Renvoie -1 si la chaine n'est pas convertible
int Convert_videomode_arg(const char *argument)
{
  // Je suis paresseux alors je vais juste tester les libellés
  int mode_index;
  for (mode_index=0; mode_index<Nb_video_modes; mode_index++)
    // Attention les vieilles fonctions de lecture .ini mettent tout en MAJUSCULE.
    if (!strcasecmp(Mode_label(mode_index), argument) && (Video_mode[mode_index].State &128) ==0)
      return mode_index;

  return -1;
}
