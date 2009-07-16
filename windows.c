/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008      Franck Charlet
    Copyright 2007-2008 Adrien Destugues
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

********************************************************************************

    Graphical interface management functions (windows, menu, cursor)
*/

#include <math.h>
#include <string.h> // strncpy() strlen()

#include "windows.h"
#include "global.h"
#include "graph.h"
#include "engine.h"
#include "misc.h"
#include "sdlscreen.h"
#include "errors.h"
#include "input.h"

// L'encapsulation tente une percée...ou un dernier combat.

// Nombre de cellules réel dans la palette du menu
word Menu_cells_X;
word Palette_cells_X()
{
  return Menu_cells_X;
}
word Menu_cells_Y;
word Palette_cells_Y()
{
  return Menu_cells_Y;
}

// Affichage d'un pixel dans le menu (le menu doit être visible)
void Pixel_in_toolbar(word x,word y,byte color)
{
  Block(x*Menu_factor_X,(y*Menu_factor_Y)+Menu_Y,Menu_factor_X,Menu_factor_Y,color);
}

// Affichage d'un pixel dans la fenêtre (la fenêtre doit être visible)

void Pixel_in_window(word x,word y,byte color)
{
    Block((x*Menu_factor_X)+Window_pos_X,(y*Menu_factor_Y)+Window_pos_Y,Menu_factor_X,Menu_factor_Y,color);
}

// Affichage d'un rectangle dans la fenêtre (la fenêtre doit être visible)
void Window_rectangle(word x_pos,word y_pos,word width,word height,byte color)
{
  Block((x_pos*Menu_factor_X)+Window_pos_X,(y_pos*Menu_factor_Y)+Window_pos_Y,width*Menu_factor_X,height*Menu_factor_Y,color);
}


// -- Affichages de différents cadres dans une fenêtre -----------------------

  // -- Frame général avec couleurs paramètrables --

void Window_display_frame_generic(word x_pos,word y_pos,word width,word height,
                                    byte color_tl,byte color_br,byte color_s,byte color_tlc,byte color_brc)
// Paramètres de couleurs:
// color_tl =Bords Haut et Gauche
// color_br =Bords Bas et Droite
// color_s  =Coins Haut-Droite et Bas-Gauche
// color_tlc=Coin Haut-Gauche
// color_brc=Coin Bas-Droite
{
  // Bord haut (sans les extrémités)
  Block(Window_pos_X+((x_pos+1)*Menu_factor_X),
        Window_pos_Y+(y_pos*Menu_factor_Y),
        (width-2)*Menu_factor_X,Menu_factor_Y,color_tl);

  // Bord bas (sans les extrémités)
  Block(Window_pos_X+((x_pos+1)*Menu_factor_X),
        Window_pos_Y+((y_pos+height-1)*Menu_factor_Y),
        (width-2)*Menu_factor_X,Menu_factor_Y,color_br);

  // Bord gauche (sans les extrémités)
  Block(Window_pos_X+(x_pos*Menu_factor_X),
        Window_pos_Y+((y_pos+1)*Menu_factor_Y),
        Menu_factor_X,(height-2)*Menu_factor_Y,color_tl);

  // Bord droite (sans les extrémités)
  Block(Window_pos_X+((x_pos+width-1)*Menu_factor_X),
        Window_pos_Y+((y_pos+1)*Menu_factor_Y),
        Menu_factor_X,(height-2)*Menu_factor_Y,color_br);

  // Coin haut gauche
  Pixel_in_window(x_pos,y_pos,color_tlc);
  // Coin haut droite
  Pixel_in_window(x_pos+width-1,y_pos,color_s);
  // Coin bas droite
  Pixel_in_window(x_pos+width-1,y_pos+height-1,color_brc);
  // Coin bas gauche
  Pixel_in_window(x_pos,y_pos+height-1,color_s);
}

  // -- Frame dont tout le contour est d'une seule couleur --

void Window_display_frame_mono(word x_pos,word y_pos,word width,word height,byte color)
{
  Window_display_frame_generic(x_pos,y_pos,width,height,color,color,color,color,color);
}

  // -- Frame creux: foncé en haut-gauche et clair en bas-droite --

void Window_display_frame_in(word x_pos,word y_pos,word width,word height)
{
  Window_display_frame_generic(x_pos,y_pos,width,height,MC_Dark,MC_White,MC_Light,MC_Dark,MC_White);
}

  // -- Frame bombé: clair en haut-gauche et foncé en bas-droite --

void Window_display_frame_out(word x_pos,word y_pos,word width,word height)
{
  Window_display_frame_generic(x_pos,y_pos,width,height,MC_White,MC_Dark,MC_Light,MC_White,MC_Dark);
}

  // -- Frame de séparation: un cadre bombé dans un cadre creux (3D!!!) --

void Window_display_frame(word x_pos,word y_pos,word width,word height)
{
  Window_display_frame_in(x_pos,y_pos,width,height);
  Window_display_frame_out(x_pos+1,y_pos+1,width-2,height-2);
}


//-- Affichages relatifs à la palette dans le menu ---------------------------

  // -- Affichage des couleurs courante (fore/back) de pinceau dans le menu --

void Display_foreback(void)
{
  if (Menu_is_visible)
  {
    Block((MENU_WIDTH-17)*Menu_factor_X,Menu_Y+Menu_factor_Y,Menu_factor_X<<4,Menu_factor_Y*7,Back_color);
    Block((MENU_WIDTH-13)*Menu_factor_X,Menu_Y+(Menu_factor_Y<<1),Menu_factor_X<<3,Menu_factor_Y*5,Fore_color);

    Update_rect((MENU_WIDTH-17)*Menu_factor_X,Menu_Y+Menu_factor_Y,Menu_factor_X<<4,Menu_factor_Y*7);
  }
}

/*! Get the top left corner for the palette cell of a color
    @param index Index of the color, starting at 0 for the top left one. Limited to Menu_cells_X/Menu_cells_Y.
*/
word Palette_cell_X(byte index)
{
  if (Config.Palette_vertical)
  {
    return (MENU_WIDTH+1+((index-First_color_in_palette)%Menu_cells_X)*Menu_palette_cell_width)*Menu_factor_X;
  }
  else
  {
    return (MENU_WIDTH+1+((index-First_color_in_palette)/Menu_cells_Y)*Menu_palette_cell_width)*Menu_factor_X;
  }
}

/*! Get the top left corner for the palette cell of a color
    @param index Index of the color, starting at 0 for the top left one. Limited to Menu_cells_X/Menu_cells_Y.
*/
word Palette_cell_Y(byte index)
{
  if (Config.Palette_vertical)
  {
    return Menu_Y+((2+(((index-First_color_in_palette)/Menu_cells_X)*(32/Menu_cells_Y)))*Menu_factor_Y);
  }
  else
  {
    return Menu_Y+((2+(((index-First_color_in_palette)%Menu_cells_Y)*(32/Menu_cells_Y)))*Menu_factor_Y);
  }
}

void Set_fore_color(byte color)
{
  byte old_fore_color = Fore_color;
  
  Fore_color=color;
  Reposition_palette();
  Display_foreback();
  Frame_menu_color(old_fore_color);
  Frame_menu_color(Fore_color);
}

void Set_back_color(byte color)
{
  byte old_back_color = Back_color;
  
  Back_color=color;
  Display_foreback();
  Frame_menu_color(old_back_color);
  Frame_menu_color(Back_color);
}

///
/// Redraw the cell in the menu palette for ::Fore_color.
/// This function checks bounds, it won't draw anything if Fore_color is not visible.
/// @param id:Color number to frame
void Frame_menu_color(byte id)
{
  word start_x,start_y,end_x,end_y;
  word index;
  word cell_height=32/Menu_cells_Y;
  byte color;

  if (id==Fore_color)
    color = MC_White;
  else if (id==Back_color)
    color = MC_Dark;
  else
    color = MC_Black;

  if ((id>=First_color_in_palette) && (id<First_color_in_palette+Menu_cells_X*Menu_cells_Y) && (Menu_is_visible))
  {
    if (Config.Separate_colors)
    {
      start_x=Palette_cell_X(id)-1*Menu_factor_X;
      start_y=Palette_cell_Y(id)-1*Menu_factor_Y;

      Block(start_x,start_y,(Menu_palette_cell_width+1)*Menu_factor_X,Menu_factor_Y,color);
      Block(start_x,start_y+(Menu_factor_Y*cell_height),(Menu_palette_cell_width+1)*Menu_factor_X,Menu_factor_Y,color);

      Block(start_x,start_y+Menu_factor_Y,Menu_factor_X,Menu_factor_Y*(cell_height-1),color);
      Block(start_x+(Menu_palette_cell_width*Menu_factor_X),start_y+Menu_factor_Y,Menu_factor_X,Menu_factor_Y*(cell_height-1),color);

      Update_rect(start_x,start_y,(Menu_palette_cell_width+1)*Menu_factor_X,Menu_factor_Y*(cell_height+1));
    }
    else
    {
      start_x=Palette_cell_X(id);
      start_y=Palette_cell_Y(id);

      if (color==MC_Black)
      {
        Block(start_x,start_y,Menu_palette_cell_width*Menu_factor_X,
              cell_height*Menu_factor_Y,id);

        Update_rect(start_x,start_y,Menu_palette_cell_width*Menu_factor_X,cell_height*Menu_factor_Y);
      }
      else
      {
        end_x=Menu_palette_cell_width-1;
        end_y=cell_height-1;

        // Top line
        for (index=0; index<=end_x; index++)
          Block(start_x+index*Menu_factor_X,start_y,
                Menu_factor_X,Menu_factor_Y,
                ((index)&1)?color:MC_Black);
        // Left line
        for (index=1; index<end_y; index++)
          Block(start_x,start_y+index*Menu_factor_Y,
                Menu_factor_X,Menu_factor_Y,
                ((index)&1)?color:MC_Black);
        // Right line
        for (index=1; index<end_y; index++)
          Block(start_x+end_x*Menu_factor_X,start_y+index*Menu_factor_Y,
                Menu_factor_X,Menu_factor_Y,
                ((index+end_x)&1)?color:MC_Black);
        // Bottom line
        for (index=0; index<=end_x; index++)
          Block(start_x+index*Menu_factor_X,start_y+end_y*Menu_factor_Y,
                Menu_factor_X,Menu_factor_Y,
                ((index+end_y)&1)?color:MC_Black);

        Update_rect(start_x*Menu_factor_X,start_y*Menu_factor_Y,Menu_palette_cell_width*Menu_factor_X,Menu_Y+Menu_factor_Y*cell_height);
      }
    }
  }
}

  // -- Afficher la palette dans le menu --

void Display_menu_palette(void)
{
  int color;
  byte cell_height=32/Menu_cells_Y;
  // width: Menu_palette_cell_width
  
  if (Menu_is_visible)
  {
    Block(MENU_WIDTH*Menu_factor_X,Menu_Y,Screen_width-(MENU_WIDTH*Menu_factor_X),(MENU_HEIGHT-9)*Menu_factor_Y,MC_Black);

    if (Config.Separate_colors)
      for (color=First_color_in_palette;color<256&&(color-First_color_in_palette)<Menu_cells_X*Menu_cells_Y;color++)
        Block(Palette_cell_X(color),
              Palette_cell_Y(color),
              (Menu_palette_cell_width-1)*Menu_factor_X,
              (cell_height-1)*Menu_factor_Y,
              color);
    else
      for (color=First_color_in_palette;color<256&&color-First_color_in_palette<Menu_cells_X*Menu_cells_Y;color++)
        Block(Palette_cell_X(color),
              Palette_cell_Y(color),
              Menu_palette_cell_width*Menu_factor_X,
              cell_height*Menu_factor_Y,
              color);

    Frame_menu_color(Back_color);
    Frame_menu_color(Fore_color);
    Update_rect(MENU_WIDTH*Menu_factor_X,Menu_Y,Screen_width-(MENU_WIDTH*Menu_factor_X),(MENU_HEIGHT-9)*Menu_factor_Y);
  }
}

  // -- Recalculer l'origine de la palette dans le menu pour rendre la
  //    Fore_color visible --

void Reposition_palette(void)
{
  byte old_color=First_color_in_palette;
  short cells;
  if (Config.Palette_vertical)
    cells=Menu_cells_X;
  else
    cells=Menu_cells_Y;
    

  if (Fore_color<First_color_in_palette)
  {
    while (Fore_color<First_color_in_palette)
      First_color_in_palette-=cells;
  }
  else
  {
    while (Fore_color>=First_color_in_palette+Menu_cells_X*Menu_cells_Y)
      First_color_in_palette+=cells;
  }
  if (old_color!=First_color_in_palette)
    Display_menu_palette();
}

void Change_palette_cells()
{
  // On initialise avec la configuration de l'utilisateur
  Menu_cells_X=Config.Palette_cells_X;
  Menu_cells_Y=Config.Palette_cells_Y;
  // Mais on sait jamais
  if (Menu_cells_X<1)
    Menu_cells_X=1;
  if (Menu_cells_Y<1)
    Menu_cells_Y=1;
  
  while (1)
  {
    Menu_palette_cell_width = ((Screen_width/Menu_factor_X)-(MENU_WIDTH+2)) / Menu_cells_X;

    // Si ça tient, c'est bon. Sinon, on retente avec une colonne de moins
    if (Menu_palette_cell_width>2)
      break;
    Menu_cells_X--;
  }
  
  // Cale First_color_in_palette sur un multiple du nombre de cellules (arrondi inférieur)
  if (Config.Palette_vertical)
    First_color_in_palette=First_color_in_palette/Menu_cells_X*Menu_cells_X;
  else
    First_color_in_palette=First_color_in_palette/Menu_cells_Y*Menu_cells_Y;

  // Si le nombre de cellules a beaucoup augmenté et qu'on était près de
  // la fin, il faut reculer First_color_in_palette pour montrer plein
  // de couleurs.
  if ((int)First_color_in_palette+(Menu_cells_Y)*Menu_cells_X*2>=256)
    First_color_in_palette=255/Menu_cells_Y*Menu_cells_Y-(Menu_cells_X-1)*Menu_cells_Y;

  // Mise à jour de la taille du bouton dans le menu. C'est pour pas que
  // la bordure noire soit active.
  Buttons_Pool[BUTTON_CHOOSE_COL].Width=(Menu_palette_cell_width*Menu_cells_X)-1;
  Buttons_Pool[BUTTON_CHOOSE_COL].Height=32/Menu_cells_Y*Menu_cells_Y-1;
}

// Retrouve la couleur sur laquelle pointe le curseur souris.
// Cette fonction suppose qu'on a déja vérifié que le curseur est dans
// la zone rectangulaire du BUTTON_CHOOSE_COL
// La fonction renvoie -1 si on est "trop à gauche" (pas possible)
// ou après la couleur 255 (Ce qui peut arriver si la palette est affichée
// avec un nombre de lignes qui n'est pas une puissance de deux.)
int Pick_color_in_palette()
{
  int color;
  int line;
  int column;

  line=(((Mouse_Y-Menu_Y)/Menu_factor_Y)-2)/(32/Menu_cells_Y);
  column=(((Mouse_X/Menu_factor_X)-(MENU_WIDTH+1))/Menu_palette_cell_width);
  if (Config.Palette_vertical)
  {
    color=First_color_in_palette+line*Menu_cells_X+column;
  }
  else
  {
    color=First_color_in_palette+line+column*Menu_cells_Y;
  }
  if (color<0 || color>255)
    return -1;
  return color;
}

  // -- Afficher tout le menu --

void Display_menu(void)
{
  word x_pos;
  word y_pos;
  char str[4];


  if (Menu_is_visible)
  {
    // Affichage du sprite du menu
    for (y_pos=0;y_pos<MENU_HEIGHT;y_pos++)
      for (x_pos=0;x_pos<MENU_WIDTH;x_pos++)
        Pixel_in_menu(x_pos,y_pos,Gfx->Menu_block[y_pos][x_pos]);
    // Affichage de la bande grise sous la palette
    Block(MENU_WIDTH*Menu_factor_X,Menu_status_Y-Menu_factor_Y,Screen_width-(MENU_WIDTH*Menu_factor_X),9*Menu_factor_Y,MC_Light);

    // Affichage de la palette
    Display_menu_palette();

    // Affichage des couleurs de travail
    Display_foreback();


    if (!Windows_open)
    {
      if ((Mouse_Y<Menu_Y) &&                                                    // Souris dans l'image
          ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ))
      {
        // Dans ces deux cas, on met dans la barre les XY courant, même s'il y a des chances que ça soit recouvert si la souris est sur un bouton (trop chiant à vérifier)
        if ( (Current_operation!=OPERATION_COLORPICK)
          && (Current_operation!=OPERATION_REPLACE) )
          Print_in_menu("X:       Y:             ",0);
        else
        {
          Print_in_menu("X:       Y:       (    )",0);
          Num2str(Colorpicker_color,str,3);
          Print_in_menu(str,20);
          Print_general(170*Menu_factor_X,Menu_status_Y," ",0,Colorpicker_color);
        }
        Print_coordinates();
      }
      Print_filename();
    }
    Update_rect(0,Menu_Y,Screen_width,MENU_HEIGHT*Menu_factor_Y); // on met toute la largur à jour, ça inclut la palette et la zone d'état avec le nom du fichier
  }
}

// -- Affichage de texte -----------------------------------------------------

  // -- Afficher une chaîne n'importe où à l'écran --

void Print_general(short x,short y,const char * str,byte text_color,byte background_color)
{
  word  index;
  int x_pos;
  int y_pos;
  byte *font_pixel;
  short real_x;
  short real_y;
  byte repeat_menu_x_factor;
  byte repeat_menu_y_factor;

  real_y=y;
  for (y_pos=0;y_pos<8<<3;y_pos+=1<<3)
  {
    real_x=0; // Position dans le buffer
    for (index=0;str[index]!='\0';index++)
    {
      // Pointeur sur le premier pixel du caractère
      font_pixel=Menu_font+(((unsigned char)str[index])<<6);
      for (x_pos=0;x_pos<8;x_pos+=1)
        for (repeat_menu_x_factor=0;repeat_menu_x_factor<Menu_factor_X*Pixel_width;repeat_menu_x_factor++)
          Horizontal_line_buffer[real_x++]=*(font_pixel+x_pos+y_pos)?text_color:background_color;
    }
    for (repeat_menu_y_factor=0;repeat_menu_y_factor<Menu_factor_Y;repeat_menu_y_factor++)
      Display_line_fast(x,real_y++,index*Menu_factor_X*8,Horizontal_line_buffer);
  }
}

/// Draws a char in a window
void Print_char_in_window(short x_pos,short y_pos,const unsigned char c,byte text_color,byte background_color)
{
  short x,y;
  byte *pixel;
  x_pos=(x_pos*Menu_factor_X)+Window_pos_X;
  y_pos=(y_pos*Menu_factor_Y)+Window_pos_Y;
  // Premier pixel du caractère
  pixel=Menu_font + (c<<6);
  
  for (y=0;y<8;y++)
    for (x=0;x<8;x++)
      Block(x_pos+(x*Menu_factor_X), y_pos+(y*Menu_factor_Y),
            Menu_factor_X, Menu_factor_Y,
            (*(pixel++)?text_color:background_color));
}

///Draws a char in a window, checking for bounds
void Print_in_window_limited(short x,short y,const char * str,byte size,byte text_color,byte background_color)
{
  char display_string[256];
  strncpy(display_string, str, size);
  display_string[size]='\0';

  if (strlen(str) > size)
  {
    display_string[size-1]=ELLIPSIS_CHARACTER;
  }
  Print_in_window(x, y, display_string, text_color, background_color);
}

/// Draws a string in a window
void Print_in_window(short x,short y,const char * str,byte text_color,byte background_color)
{
  Print_general((x*Menu_factor_X)+Window_pos_X,
                (y*Menu_factor_Y)+Window_pos_Y,
                str,text_color,background_color);
  Update_rect(x*Menu_factor_X+Window_pos_X,y*Menu_factor_Y+Window_pos_Y,8*Menu_factor_X*strlen(str),8*Menu_factor_Y);
}

// Draws a string in the menu's status bar
void Print_in_menu(const char * str, short position)
{
  Print_general((18+(position<<3))*Menu_factor_X,Menu_status_Y,str,MC_Black,MC_Light);
  Update_rect((18+(position<<3))*Menu_factor_X,Menu_status_Y,strlen(str)*8*Menu_factor_X,8*Menu_factor_Y);
}

/// Draws the mouse coordinates on the menu
/// Only update the digits and doesn't refresh the "X: Y:" labels. This function needs to be fast as it is called each time the mouse moves.
void Print_coordinates(void)
{
  char temp[5];

  if (Menu_is_visible && !Cursor_in_menu)
  {
    if ( (Current_operation==OPERATION_COLORPICK)
      || (Current_operation==OPERATION_REPLACE) )
    {
      if ( (Paintbrush_X>=0) && (Paintbrush_Y>=0)
        && (Paintbrush_X<Main_image_width)
        && (Paintbrush_Y<Main_image_height) )
        Colorpicker_color=Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y);
      else
        Colorpicker_color=0;
      Colorpicker_X=Paintbrush_X;
      Colorpicker_Y=Paintbrush_Y;

      Num2str(Colorpicker_color,temp,3);
      Print_in_menu(temp,20);
      Print_general(170*Menu_factor_X,Menu_status_Y," ",0,Colorpicker_color);
      Update_rect(170*Menu_factor_X,Menu_status_Y,8*Menu_factor_X,8*Menu_factor_Y);
    }

    Num2str((dword)Paintbrush_X,temp,4);
    Print_in_menu(temp,2);
    Num2str((dword)Paintbrush_Y,temp,4);
    Print_in_menu(temp,11);
  }
}

  // -- Afficher le nom du fichier dans le menu --

void Print_filename(void)
{
  char display_name[12+1];
  int name_size;
  if (Menu_is_visible)
  {
    // Si le nom de fichier fait plus de 12 caractères, on n'affiche que les 12 derniers
    strncpy(display_name,Main_filename,12);
    name_size=strlen(Main_filename);
    display_name[12]='\0';
    if (name_size>12)
    {
      display_name[11]=ELLIPSIS_CHARACTER;
      name_size = 12;
    }
    
    Block(Screen_width-96*Menu_factor_X,
          Menu_status_Y,Menu_factor_X*96,Menu_factor_Y<<3,MC_Light);

    Print_general(Screen_width-name_size*8*Menu_factor_X,Menu_status_Y,display_name,MC_Black,MC_Light);
  }
}

// Fonction d'affichage d'une chaine numérique avec une fonte très fine
// Spécialisée pour les compteurs RGB
void Print_counter(short x,short y,const char * str,byte text_color,byte background_color)
{
  // Macros pour écrire des litteraux binaires.
  // Ex: Ob(11110000) == 0xF0
  #define Ob(x)  ((unsigned)Ob_(0 ## x ## uL))
  #define Ob_(x) ((x & 1) | (x >> 2 & 2) | (x >> 4 & 4) | (x >> 6 & 8) |                \
          (x >> 8 & 16) | (x >> 10 & 32) | (x >> 12 & 64) | (x >> 14 & 128))

  byte thin_font[14][8] = {
   { // 0
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 1
    Ob(00001100),
    Ob(00011100),
    Ob(00111100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100)
   },
   { // 2
    Ob(00011100),
    Ob(00110110),
    Ob(00000110),
    Ob(00000110),
    Ob(00000110),
    Ob(00001100),
    Ob(00011000),
    Ob(00111110)
   },
   { // 3
    Ob(00011100),
    Ob(00110110),
    Ob(00000110),
    Ob(00001100),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 4
    Ob(00001100),
    Ob(00001100),
    Ob(00011000),
    Ob(00011000),
    Ob(00110000),
    Ob(00110100),
    Ob(00111110),
    Ob(00000100)
   },
   { // 5
    Ob(00111110),
    Ob(00110000),
    Ob(00110000),
    Ob(00111100),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 6
    Ob(00011100),
    Ob(00110110),
    Ob(00110000),
    Ob(00111100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 7
    Ob(00111110),
    Ob(00000110),
    Ob(00000110),
    Ob(00001100),
    Ob(00011000),
    Ob(00011000),
    Ob(00011000),
    Ob(00011000)
   },
   { // 8
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 9
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00011110),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // (espace)
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000)
   },
   { // +
    Ob(00000000),
    Ob(00001000),
    Ob(00001000),
    Ob(00111110),
    Ob(00001000),
    Ob(00001000),
    Ob(00000000),
    Ob(00000000)
   },
   { // -
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00111110),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000)
   },
   { // +-
    Ob(00001000),
    Ob(00001000),
    Ob(00111110),
    Ob(00001000),
    Ob(00001000),
    Ob(00000000),
    Ob(00111110),
    Ob(00000000)
   } };

  word  index;
  short x_pos;
  short y_pos;
  for (index=0;str[index]!='\0';index++)
  {
    int char_number;
    switch(str[index])
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        char_number=str[index]-'0';
        break;
      case ' ':
      default:
        char_number=10;
        break;
      case '+':
        char_number=11;
        break;
      case '-':
        char_number=12;
        break;
      case '±':
        char_number=13;
        break;
    }
    for (y_pos=0;y_pos<8;y_pos++)
    {
      for (x_pos=0;x_pos<6;x_pos++)
      {
        byte color = (thin_font[char_number][y_pos] & (1 << (6-x_pos))) ? text_color:background_color;
        Pixel_in_window(x+(index*6+x_pos),y+y_pos,color);
      }
    }
  }
  Update_rect(Window_pos_X+x*Menu_factor_X,Window_pos_Y+y*Menu_factor_Y,strlen(str)*Menu_factor_X*6,8*Menu_factor_Y);
}



//---- Fenêtre demandant de confirmer une action et renvoyant la réponse -----
byte Confirmation_box(char * message)
{
  short clicked_button;
  word  window_width;

  window_width=(strlen(message)<<3)+20;

  if (window_width<120)
    window_width=120;

  Open_window(window_width,60,"Confirmation");

  Print_in_window((window_width>>1)-(strlen(message)<<2),20,message,MC_Black,MC_Light);

  Window_set_normal_button((window_width/3)-20     ,37,40,14,"Yes",1,1,SDLK_y); // 1
  Window_set_normal_button(((window_width<<1)/3)-20,37,40,14,"No" ,1,1,SDLK_n); // 2

  Update_rect(Window_pos_X,Window_pos_Y,Menu_factor_X*window_width,Menu_factor_Y*60);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Key==SDLK_RETURN) clicked_button=1;
    if (Key==KEY_ESC) clicked_button=2;
  }
  while (clicked_button<=0);
  Key=0;

  Close_window();
  Display_cursor();

  return (clicked_button==1)? 1 : 0;
}



//---- Fenêtre avertissant de quelque chose et attendant un click sur OK -----
void Warning_message(char * message)
{
  short clicked_button;
  word  window_width;

  window_width=(strlen(message)<<3)+20;
  if (window_width<120)
    window_width=120;

  Open_window(window_width,60,"Warning!");

  Print_in_window((window_width>>1)-(strlen(message)<<2),20,message,MC_Black,MC_Light);
  Window_set_normal_button((window_width>>1)-20     ,37,40,14,"OK",1,1,SDLK_RETURN); // 1
  Update_rect(Window_pos_X,Window_pos_Y,Menu_factor_X*window_width,Menu_factor_Y*60);
  Display_cursor();

  do
    clicked_button=Window_clicked_button();
  while ((clicked_button<=0) && (Key!=KEY_ESC) && (Key!=SDLK_o));
  Key=0;

  Close_window();
  Display_cursor();
}

//---- Window that shows a big message, and waits for a click on OK -----
void Verbose_error_message(char * message)
{
  short clicked_button;
  int line;
  int i;
  char buffer[36]; // 35 characters + \0

  Open_window(300,160,"Error!");
  
  // Word-wrap the message
  for (line=0; line < 10; line++)
  {
    for (i=0;i<35 && *message!='\0';i++)
    {
      if (*message == '\n')
      {
        message++;
        break;
      }
      buffer[i]=*message;
      message++;
    }
    buffer[i]='\0';
    Print_in_window(10,20+line*8,buffer,MC_Black,MC_Light);
    if (*message=='\0')
      break;
  }

  Window_set_normal_button(300/2-20,160-23,40,14,"OK",1,1,SDLK_RETURN); // 1
  Update_window_area(0,0,Window_width,Window_height);
  Display_cursor();

  do
    clicked_button=Window_clicked_button();
  while ((clicked_button<=0) && (Key!=KEY_ESC) && (Key!=SDLK_o));
  Key=0;

  Close_window();
  Display_cursor();
}


  // -- Redessiner le sprite d'un bouton dans le menu --

void Display_sprite_in_menu(int btn_number,int sprite_number)
{
  word x_pos;
  word y_pos;
  word menu_x_pos;
  word menu_y_pos;
  byte color;

  menu_y_pos=Buttons_Pool[btn_number].Y_offset;
  menu_x_pos=Buttons_Pool[btn_number].X_offset;
  if (Buttons_Pool[btn_number].Shape != BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT)
  {
    menu_y_pos+=1;
    menu_x_pos+=1;
  }
  
  for (y_pos=0;y_pos<MENU_SPRITE_HEIGHT;y_pos++)
    for (x_pos=0;x_pos<MENU_SPRITE_WIDTH;x_pos++)
    {
      color=Gfx->Menu_sprite[sprite_number][y_pos][x_pos];
      Pixel_in_menu(menu_x_pos+x_pos,menu_y_pos+y_pos,color);
      Gfx->Menu_block[menu_y_pos+y_pos][menu_x_pos+x_pos]=color;
    }
  Update_rect(Menu_factor_X*(Buttons_Pool[btn_number].X_offset+1),
    (Buttons_Pool[btn_number].Y_offset+1)*Menu_factor_Y+Menu_Y,
    MENU_SPRITE_WIDTH*Menu_factor_X,MENU_SPRITE_HEIGHT*Menu_factor_Y);
}

  // -- Redessiner la forme du pinceau dans le menu --

void Display_paintbrush_in_menu(void)
{
  short x_pos,y_pos;
  short start_x;
  short menu_x_pos,menu_y_pos;
  short menu_start_x;
  byte color;

  switch (Paintbrush_shape)
  {
    case PAINTBRUSH_SHAPE_COLOR_BRUSH    : // Brush en couleur
    case PAINTBRUSH_SHAPE_MONO_BRUSH : // Brush monochrome
      for (menu_y_pos=2,y_pos=0;y_pos<MENU_SPRITE_HEIGHT;menu_y_pos++,y_pos++)
        for (menu_x_pos=1,x_pos=0;x_pos<MENU_SPRITE_WIDTH;menu_x_pos++,x_pos++)
        {
          color=Gfx->Menu_sprite[4][y_pos][x_pos];
          Pixel_in_menu(menu_x_pos,menu_y_pos,color);
          Gfx->Menu_block[menu_y_pos][menu_x_pos]=color;
        }
      break;
    default : // Pinceau
      // On efface le pinceau précédent
      for (menu_y_pos=2,y_pos=0;y_pos<MENU_SPRITE_HEIGHT;menu_y_pos++,y_pos++)
        for (menu_x_pos=1,x_pos=0;x_pos<MENU_SPRITE_WIDTH;menu_x_pos++,x_pos++)
        {
          Pixel_in_menu(menu_x_pos,menu_y_pos,MC_Light);
          Gfx->Menu_block[menu_y_pos][menu_x_pos]=MC_Light;
        }
      // On affiche le nouveau
      menu_start_x=8-Paintbrush_offset_X;
      if (menu_start_x<1)
      {
        start_x=Paintbrush_offset_X-7;
        menu_start_x=1;
      }
      else
        start_x=0;

      menu_y_pos=9-Paintbrush_offset_Y;
      if (menu_y_pos<2)
      {
        y_pos=Paintbrush_offset_Y-7;
        menu_y_pos=2;
      }
      else
        y_pos=0;

      for (;((y_pos<Paintbrush_height) && (menu_y_pos<16));menu_y_pos++,y_pos++)
        for (menu_x_pos=menu_start_x,x_pos=start_x;((x_pos<Paintbrush_width) && (menu_x_pos<15));menu_x_pos++,x_pos++)
        {
          color=(Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos])?MC_Black:MC_Light;
          Pixel_in_menu(menu_x_pos,menu_y_pos,color);
          Gfx->Menu_block[menu_y_pos][menu_x_pos]=color;
        }
  }
  Update_rect(0,Menu_Y,MENU_SPRITE_WIDTH*Menu_factor_X+3,MENU_SPRITE_HEIGHT*Menu_factor_Y+3);
}

  // -- Dessiner un pinceau prédéfini dans la fenêtre --

void Display_paintbrush_in_window(word x,word y,int number)
  // Pinceau = 0..NB_PAINTBRUSH_SPRITES-1 : Pinceau prédéfini
{
  word x_pos;
  word y_pos;
  word window_x_pos;
  word window_y_pos;
  int x_size;
  int y_size;
  word origin_x;
  word origin_y;

  x_size=Menu_factor_X/Pixel_height;
  if (x_size<1)
    x_size=1;
  y_size=Menu_factor_Y/Pixel_width;
  if (y_size<1)
    y_size=1;

  origin_x = (x + 8)*Menu_factor_X - (Gfx->Preset_paintbrush_offset_X[number])*x_size+Window_pos_X;
  origin_y = (y + 8)*Menu_factor_Y - (Gfx->Preset_paintbrush_offset_Y[number])*y_size+Window_pos_Y;

  for (window_y_pos=0,y_pos=0; y_pos<Gfx->Preset_paintbrush_height[number]; window_y_pos++,y_pos++)
    for (window_x_pos=0,x_pos=0; x_pos<Gfx->Preset_paintbrush_width[number]; window_x_pos++,x_pos++)
      Block(origin_x+window_x_pos*x_size,origin_y+window_y_pos*y_size,x_size,y_size,(Gfx->Paintbrush_sprite[number][y_pos][x_pos])?MC_Black:MC_Light);
  // On n'utilise pas Pixel_in_window() car on ne dessine pas
  // forcément avec la même taille de pixel.

  Update_rect( ToWinX(origin_x), ToWinY(origin_y),
        ToWinL(Gfx->Preset_paintbrush_width[number]),
        ToWinH(Gfx->Preset_paintbrush_height[number])
  );
}

  // -- Dessiner des zigouigouis --

void Draw_thingumajig(word x,word y, byte color, short direction)
{
  word i;

  for (i=0; i<11; i++) Pixel_in_window(x,y+i,color);
  x+=direction;
  for (i=1; i<10; i++) Pixel_in_window(x,y+i,color);
  x+=direction+direction;
  for (i=3; i<8; i++) Pixel_in_window(x,y+i,color);
  x+=direction+direction;
  Pixel_in_window(x,y+5,color);
}

  // -- Dessiner un bloc de couleurs dégradé verticalement

void Display_grad_block_in_window(word x_pos,word y_pos,word block_start,word block_end)
{
  word total_lines  =Menu_factor_Y<<6; // <=> à 64 lignes fct(Menu_Facteur)
  word nb_colors   =(block_start<=block_end)?block_end-block_start+1:block_start-block_end+1;
  word Selected_line_mode=(block_start<=block_end)?0:total_lines-1;

  word start_x       =Window_pos_X+(Menu_factor_X*x_pos);
  word line_width =Menu_factor_X<<4; // <=> à 16 pixels fct(Menu_Facteur)

  word start_y       =Window_pos_Y+(Menu_factor_Y*y_pos);
  word end_y         =start_y+total_lines;
  word index;

  if (block_start>block_end)
  {
    index=block_start;
    block_start=block_end;
    block_end=index;
  }

  for (index=start_y;index<end_y;index++,Selected_line_mode++)
    Block(start_x,index,line_width,1,block_start+(nb_colors*Selected_line_mode)/total_lines);

  Update_rect(ToWinX(x_pos),ToWinY(y_pos),ToWinL(16),ToWinH(64));
}



  // -- Dessiner un petit sprite représentant le type d'un drive --

void Window_display_icon_sprite(word x_pos,word y_pos,byte type)
{
  word i,j;

  for (j=0; j<ICON_SPRITE_HEIGHT; j++)
    for (i=0; i<ICON_SPRITE_WIDTH; i++)
      Pixel_in_window(x_pos+i,y_pos+j,Gfx->Icon_sprite[type][j][i]);
  Update_rect(ToWinX(x_pos),ToWinY(y_pos),ToWinL(ICON_SPRITE_WIDTH),ToWinH(ICON_SPRITE_HEIGHT));
}



void Display_menu_palette_avoiding_window(byte * table)
{
  // On part du principe qu'il n'y a que le bas d'une fenêtre qui puisse
  // empiéter sur la palette... Et c'est déjà pas mal!
  word color,real_color;
  word start_x,start_y;
  word end_x,end_y;
  word width;
  word height;
  word corner_x=Window_pos_X+Window_width*Menu_factor_X; // |_ Coin bas-droit
  word corner_y=Window_pos_Y+Window_height*Menu_factor_Y; // |  de la fenêtre +1


  if (Config.Separate_colors)
  {
    width=(Menu_palette_cell_width-1)*Menu_factor_X;
    height=Menu_factor_Y*(32/Menu_cells_Y-1);
  }
  else
  {
    width=Menu_palette_cell_width*Menu_factor_X;
    height=Menu_factor_Y*(32/Menu_cells_Y);
  }

  for (color=0,real_color=First_color_in_palette;color<Menu_cells_X*Menu_cells_Y;color++,real_color++)
  {
    if (table[real_color]!=real_color)
    {
      start_x=Palette_cell_X(real_color);
      start_y=Palette_cell_Y(real_color); //Menu_Y_before_window ??!
      end_x=start_x+width;
      end_y=start_y+height;

      //   On affiche le bloc en entier si on peut, sinon on le découpe autour
      // de la fenêtre.
      if ( (start_y>=corner_y) || (end_x<=Window_pos_X) || (start_x>=corner_x) )
        Block(start_x,start_y,width,height,real_color);
      else
      {

        if (start_x>=Window_pos_X)
        {
          if ( (end_x>corner_x) || (end_y>corner_y) )
          {
            if ( (end_x>corner_x) && (end_y>corner_y) )
            {
              Block(corner_x,start_y,end_x-corner_x,corner_y-start_y,real_color);
              Block(start_x,corner_y,width,end_y-corner_y,real_color);
            }
            else
            {
              if (end_y>corner_y)
                Block(start_x,corner_y,width,end_y-corner_y,real_color);
              else
                Block(corner_x,start_y,end_x-corner_x,height,real_color);
            }
          }
        }
        else
        {
          if (end_x<corner_x)
          {
            if (end_y>corner_y)
            {
              Block(start_x,start_y,Window_pos_X-start_x,corner_y-start_y,real_color);
              Block(start_x,corner_y,width,end_y-corner_y,real_color);
            }
            else
              Block(start_x,start_y,Window_pos_X-start_x,height,real_color);
          }
          else
          {
            if (end_y>corner_y)
            {
              Block(start_x,start_y,Window_pos_X-start_x,corner_y-start_y,real_color);
              Block(corner_x,start_y,end_x-corner_x,corner_y-start_y,real_color);
              Block(start_x,corner_y,width,end_y-corner_y,real_color);
            }
            else
            {
              Block(start_x,start_y,Window_pos_X-start_x,height,real_color);
              Block(corner_x,start_y,end_x-corner_x,height,real_color);
            }
          }
        }
      }
      {
        // Affichage du bloc directement dans le "buffer de fond" de la fenetre.
        // Cela permet au bloc de couleur d'apparaitre si on déplace la fenetre.
        short x_pos;
        short y_pos;
        short relative_x; // besoin d'une variable signée
        short relative_y; // besoin d'une variable signée
        // Attention aux unités
        relative_x = ((short)start_x - (short)Window_pos_X);
        relative_y = ((short)start_y - (short)Window_pos_Y);

        for (y_pos=relative_y;y_pos<(relative_y+height)&&y_pos<Window_height*Menu_factor_Y;y_pos++)
          for (x_pos=relative_x;x_pos<(relative_x+width)&&x_pos<Window_width*Menu_factor_X;x_pos++)
            if (x_pos>=0&&y_pos>=0)
              Pixel_background(x_pos,y_pos,real_color);
      }
    }
  }
  Update_rect(MENU_WIDTH*Menu_factor_X,Menu_Y_before_window,Screen_width-(MENU_WIDTH*Menu_factor_X),(MENU_HEIGHT-9)*Menu_factor_Y);
}

// -------- Calcul des bornes de la partie d'image visible à l'écran ---------
void Compute_limits(void)
/*
  Avant l'appel à cette fonction, les données de la loupe doivent être à jour.
*/
{
  if (Main_magnifier_mode)
  {
    // -- Calcul des limites de la partie non zoomée de l'image --
    Limit_top  =Main_offset_Y;
    Limit_left=Main_offset_X;
    Limit_visible_bottom   =Limit_top+Menu_Y-1;
    Limit_visible_right=Limit_left+Main_separator_position-1;

    if (Limit_visible_bottom>=Main_image_height)
      Limit_bottom=Main_image_height-1;
    else
      Limit_bottom=Limit_visible_bottom;

    if (Limit_visible_right>=Main_image_width)
      Limit_right=Main_image_width-1;
    else
      Limit_right=Limit_visible_right;

    // -- Calcul des limites de la partie zoomée de l'image --
    Limit_top_zoom  =Main_magnifier_offset_Y;
    Limit_left_zoom=Main_magnifier_offset_X;
    Limit_visible_bottom_zoom   =Limit_top_zoom+Main_magnifier_height-1;
    Limit_visible_right_zoom=Limit_left_zoom+Main_magnifier_width-1;

    if (Limit_visible_bottom_zoom>=Main_image_height)
      Limit_bottom_zoom=Main_image_height-1;
    else
      Limit_bottom_zoom=Limit_visible_bottom_zoom;

    if (Limit_visible_right_zoom>=Main_image_width)
      Limit_right_zoom=Main_image_width-1;
    else
      Limit_right_zoom=Limit_visible_right_zoom;
  }
  else
  {
    // -- Calcul des limites de la partie visible de l'image --
    Limit_top  =Main_offset_Y;
    Limit_left=Main_offset_X;
    Limit_visible_bottom   =Limit_top+(Menu_is_visible?Menu_Y:Screen_height)-1; // A REVOIR POUR SIMPLIFICATION
    Limit_visible_right=Limit_left+Screen_width-1;

    if (Limit_visible_bottom>=Main_image_height)
      Limit_bottom=Main_image_height-1;
    else
      Limit_bottom=Limit_visible_bottom;

    if (Limit_visible_right>=Main_image_width)
      Limit_right=Main_image_width-1;
    else
      Limit_right=Limit_visible_right;
  }
}


// -- Calculer les coordonnées du pinceau en fonction du snap et de la loupe -
void Compute_paintbrush_coordinates(void)
{
  if ((Main_magnifier_mode) && (Mouse_X>=Main_X_zoom))
  {
    Paintbrush_X=((Mouse_X-Main_X_zoom)/Main_magnifier_factor)+Main_magnifier_offset_X;
    Paintbrush_Y=(Mouse_Y/Main_magnifier_factor)+Main_magnifier_offset_Y;
  }
  else
  {
    Paintbrush_X=Mouse_X+Main_offset_X;
    Paintbrush_Y=Mouse_Y+Main_offset_Y;
  }

  if (Snap_mode)
  {
    Paintbrush_X=(((Paintbrush_X+(Snap_width>>1)-Snap_offset_X)/Snap_width)*Snap_width)+Snap_offset_X;
    Paintbrush_Y=(((Paintbrush_Y+(Snap_height>>1)-Snap_offset_Y)/Snap_height)*Snap_height)+Snap_offset_Y;
  }
  
  // Handling the snap axis mode, when shift is pressed.
  switch (Current_operation)
  {
    // Operations that don't implement it
    case OPERATION_LINE:
      Snap_axis=0;
      break;
    // Operations that implement it
    default:
      if (Snap_axis==0 && (SDL_GetModState() & KMOD_SHIFT))
      {
        // Start "Snap axis" mode
        Snap_axis=1;
        Snap_axis_origin_X=Paintbrush_X;
        Snap_axis_origin_Y=Paintbrush_Y;
      } 
  }

  if (Snap_axis==1)
  {
    // Cursor moved
    if (Paintbrush_X != Snap_axis_origin_X || Paintbrush_Y != Snap_axis_origin_Y)
    {
      if ((Paintbrush_X-Snap_axis_origin_X)*(Paintbrush_X-Snap_axis_origin_X) >
          (Paintbrush_Y-Snap_axis_origin_Y)*(Paintbrush_Y-Snap_axis_origin_Y))
      // Displacement was bigger on X axis: lock Y
        Snap_axis=2;
      else
        Snap_axis=3;
    }
  }
  if (Snap_axis==2)
  {
    Paintbrush_Y = Snap_axis_origin_Y;
  }
  else if (Snap_axis==3)
  {
    Paintbrush_X = Snap_axis_origin_X;
  }
}



// -- Affichage de la limite de l'image -------------------------------------
void Display_image_limits(void)
{
  short start;
  short pos;
  short end;
  byte right_is_visible;
  byte bottom_is_visible;
  short old_zoom_limit;

  right_is_visible=Main_image_width<((Main_magnifier_mode)?Main_separator_position:Screen_width);
  bottom_is_visible   =Main_image_height<Menu_Y;


  // On vérifie que la limite à droite est visible:
  if (right_is_visible)
  {
    start=Limit_top;
    end=(Limit_bottom<Main_image_height)?
        Limit_bottom:Main_image_height;

    if (bottom_is_visible)
      end++;

    // Juste le temps d'afficher les limites, on étend les limites de la loupe
    // aux limites visibles, car sinon Pixel_preview ne voudra pas afficher.
    old_zoom_limit=Limit_right_zoom;
    Limit_right_zoom=Limit_visible_right_zoom;

    for (pos=start;pos<=end;pos++)
      Pixel_preview(Main_image_width,pos,((pos+Main_image_height)&1)?MC_White:MC_Black);

    Update_rect(Main_image_width,start,1,end-start + 1);
    // On restaure la bonne valeur des limites
    Limit_right_zoom=old_zoom_limit;
  }

  // On vérifie que la limite en bas est visible:
  if (bottom_is_visible)
  {
    start=Limit_left;
    end=(Limit_right<Main_image_width)?
        Limit_right:Main_image_width;

    // On étend également les limites en bas (comme pour la limite droit)
    old_zoom_limit=Limit_bottom_zoom;
    Limit_bottom_zoom=Limit_visible_bottom_zoom;

    for (pos=start;pos<=end;pos++)
      Pixel_preview(pos,Main_image_height,((pos+Main_image_height)&1)?MC_White:MC_Black);

    Update_rect(start,Main_image_height,end-start + 1,1);

    // On restaure la bonne valeur des limites
    Limit_bottom_zoom=old_zoom_limit;
  }
}



// -- Recadrer la partie non-zoomée de l'image par rapport à la partie zoomée
//    lorsqu'on scrolle en mode Loupe --
void Position_screen_according_to_zoom(void)
{
  // Centrage en X
  if (Main_image_width>Main_separator_position)
  {
    Main_offset_X=Main_magnifier_offset_X+(Main_magnifier_width>>1)
                         -(Main_separator_position>>1);
    if (Main_offset_X<0)
      Main_offset_X=0;
    else
    if (Main_image_width<Main_offset_X+Main_separator_position)
      Main_offset_X=Main_image_width-Main_separator_position;
  }
  else
    Main_offset_X=0;

  // Centrage en Y
  if (Main_image_height>Menu_Y)
  {
    Main_offset_Y=Main_magnifier_offset_Y+(Main_magnifier_height>>1)
                         -(Menu_Y>>1);
    if (Main_offset_Y<0)
      Main_offset_Y=0;
    else
    if (Main_image_height<Main_offset_Y+Menu_Y)
      Main_offset_Y=Main_image_height-Menu_Y;
  }
  else
    Main_offset_Y=0;
}


// - Calcul des données du split en fonction de la proportion de chaque zone -
void Compute_separator_data(void)
{
  //short temp;
  short theoric_X=Round(Main_separator_proportion*Screen_width);

  Main_X_zoom=Screen_width-(((Screen_width+(Main_magnifier_factor>>1)-theoric_X)/Main_magnifier_factor)*Main_magnifier_factor);
  Main_separator_position=Main_X_zoom-(Menu_factor_X*SEPARATOR_WIDTH);

  // Correction en cas de débordement sur la gauche
  while (Main_separator_position*(Main_magnifier_factor+1)<Screen_width-(Menu_factor_X*SEPARATOR_WIDTH))
  {
    Main_separator_position+=Main_magnifier_factor;
    Main_X_zoom+=Main_magnifier_factor;
  }
  // Correction en cas de débordement sur la droite
  theoric_X=Screen_width-((NB_ZOOMED_PIXELS_MIN-1)*Main_magnifier_factor);
  while (Main_X_zoom>=theoric_X)
  {
    Main_separator_position-=Main_magnifier_factor;
    Main_X_zoom-=Main_magnifier_factor;
  }
}



// -------------------- Calcul des information de la loupe -------------------
void Compute_magnifier_data(void)
/*
  Après modification des données de la loupe, il faut recalculer les limites.
*/
{
  Compute_separator_data();

  Main_magnifier_width=(Screen_width-Main_X_zoom)/Main_magnifier_factor;

  Main_magnifier_height=Menu_Y/Main_magnifier_factor;
  if (Menu_Y%Main_magnifier_factor)
    Main_magnifier_height++;

  if (Main_magnifier_mode && Main_magnifier_offset_X)
  {
    if (Main_image_width<Main_magnifier_offset_X+Main_magnifier_width)
      Main_magnifier_offset_X=Main_image_width-Main_magnifier_width;
    if (Main_magnifier_offset_X<0) Main_magnifier_offset_X=0;
  }
  if (Main_magnifier_mode && Main_magnifier_offset_Y)
  {
    if (Main_image_height<Main_magnifier_offset_Y+Main_magnifier_height)
      Main_magnifier_offset_Y=Main_image_height-Main_magnifier_height;
    if (Main_magnifier_offset_Y<0) Main_magnifier_offset_Y=0;
  }
  
}



// ------------ Changer le facteur de zoom et tout mettre à jour -------------
void Change_magnifier_factor(byte factor_index)
{
  short center_x;
  short center_y;

  center_x=Main_magnifier_offset_X+(Main_magnifier_width>>1);
  center_y=Main_magnifier_offset_Y+(Main_magnifier_height>>1);

  Main_magnifier_factor=ZOOM_FACTOR[factor_index];
  Compute_magnifier_data();

  if (Main_magnifier_mode)
  {
    // Recalculer le décalage de la loupe
    // Centrage "brut" de lécran par rapport à la loupe
    Main_magnifier_offset_X=center_x-(Main_magnifier_width>>1);
    Main_magnifier_offset_Y=center_y-(Main_magnifier_height>>1);
    // Correction en cas de débordement de l'image
    if (Main_magnifier_offset_X+Main_magnifier_width>Main_image_width)
      Main_magnifier_offset_X=Main_image_width-Main_magnifier_width;
    if (Main_magnifier_offset_Y+Main_magnifier_height>Main_image_height)
      Main_magnifier_offset_Y=Main_image_height-Main_magnifier_height;
    if (Main_magnifier_offset_X<0)
      Main_magnifier_offset_X=0;
    if (Main_magnifier_offset_Y<0)
      Main_magnifier_offset_Y=0;

    Position_screen_according_to_zoom();

    Pixel_preview=Pixel_preview_magnifier;

  }
  else
    Pixel_preview=Pixel_preview_normal;

  Compute_limits();
  Compute_paintbrush_coordinates();
}



  // -- Afficher la barre de séparation entre les parties zoomées ou non en
  //    mode Loupe --

void Display_separator(void)
{
  // Partie grise du milieu
  Block(Main_separator_position+(Menu_factor_X<<1),Menu_factor_Y,
        (SEPARATOR_WIDTH-4)*Menu_factor_X,
        Menu_Y-(Menu_factor_Y<<1),MC_Light);

  // Barre noire de gauche
  Block(Main_separator_position,0,Menu_factor_X,Menu_Y,MC_Black);

  // Barre noire de droite
  Block(Main_X_zoom-Menu_factor_X,0,Menu_factor_X,Menu_Y,MC_Black);

  // Bord haut (blanc)
  Block(Main_separator_position+Menu_factor_X,0,
        (SEPARATOR_WIDTH-3)*Menu_factor_X,Menu_factor_Y,MC_White);

  // Bord gauche (blanc)
  Block(Main_separator_position+Menu_factor_X,Menu_factor_Y,
        Menu_factor_X,(Menu_Y-(Menu_factor_Y<<1)),MC_White);

  // Bord droite (gris foncé)
  Block(Main_X_zoom-(Menu_factor_X<<1),Menu_factor_Y,
        Menu_factor_X,(Menu_Y-(Menu_factor_Y<<1)),MC_Dark);

  // Bord bas (gris foncé)
  Block(Main_separator_position+(Menu_factor_X<<1),Menu_Y-Menu_factor_Y,
        (SEPARATOR_WIDTH-3)*Menu_factor_X,Menu_factor_Y,MC_Dark);

  // Coin bas gauche
  Block(Main_separator_position+Menu_factor_X,Menu_Y-Menu_factor_Y,
        Menu_factor_X,Menu_factor_Y,MC_Light);
  // Coin haut droite
  Block(Main_X_zoom-(Menu_factor_X<<1),0,
        Menu_factor_X,Menu_factor_Y,MC_Light);

  Update_rect(Main_separator_position,0,SEPARATOR_WIDTH*Menu_factor_X,Menu_Y); // On réaffiche toute la partie à gauche du split, ce qui permet d'effacer son ancienne position
}



// -- Fonctions de manipulation du curseur -----------------------------------


  // -- Afficher une barre horizontale XOR zoomée

void Horizontal_XOR_line_zoom(short x_pos, short y_pos, short width)
{
  short real_x_pos=Main_X_zoom+(x_pos-Main_magnifier_offset_X)*Main_magnifier_factor;
  short real_y_pos=(y_pos-Main_magnifier_offset_Y)*Main_magnifier_factor;
  short real_width=width*Main_magnifier_factor;
  short end_y_pos=(real_y_pos+Main_magnifier_factor<Menu_Y)?real_y_pos+Main_magnifier_factor:Menu_Y;
  short index;

  for (index=real_y_pos; index<end_y_pos; index++)
    Horizontal_XOR_line(real_x_pos,index,real_width);

  Update_rect(real_x_pos,real_y_pos,real_width,end_y_pos-real_y_pos);
}


  // -- Afficher une barre verticale XOR zoomée

void Vertical_XOR_line_zoom(short x_pos, short y_pos, short height)
{
  short real_x_pos=Main_X_zoom+(x_pos-Main_magnifier_offset_X)*Main_magnifier_factor;
  short real_y_pos=(y_pos-Main_magnifier_offset_Y)*Main_magnifier_factor;
  short end_y_pos=(real_y_pos+(height*Main_magnifier_factor<Menu_Y))?real_y_pos+(height*Main_magnifier_factor):Menu_Y;
  short index;

  for (index=real_y_pos; index<end_y_pos; index++)
    Horizontal_XOR_line(real_x_pos,index,Main_magnifier_factor);

  Update_rect(real_x_pos,real_y_pos,Main_magnifier_factor,end_y_pos-real_y_pos);
}


  // -- Afficher le curseur --

void Display_cursor(void)
{
  byte  shape;
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short x_pos;
  short y_pos;
  short counter_x = 0;
  short counter_y;
  int   temp;
  byte  color;
  float cos_a,sin_a;
  short x1,y1,x2,y2,x3,y3,x4,y4;

  // Si le curseur est dans le menu ou sur la barre de split, on affiche toujours une flèche.
  if ( ( (Mouse_Y<Menu_Y)
      && ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
    || (Windows_open) || (Cursor_shape==CURSOR_SHAPE_HOURGLASS) )
    shape=Cursor_shape;
  else
    shape=CURSOR_SHAPE_ARROW;

  switch(shape)
  {
    case CURSOR_SHAPE_TARGET :
      if (!Paintbrush_hidden)
        Display_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color,1);
      if (!Cursor_hidden)
      {
        if (Config.Cursor==1)
        {
          start_y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (start_y<4)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+start_y-6,4-start_y);

          start_x=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (start_x<4)
            Horizontal_XOR_line(Mouse_X+start_x-6,Mouse_Y,4-start_x);

          end_x=(Mouse_X+7>Screen_width)?Mouse_X+7-Screen_width:0;
          if (end_x<4)
            Horizontal_XOR_line(Mouse_X+3,Mouse_Y,4-end_x);

          end_y=(Mouse_Y+7>Screen_height)?Mouse_Y+7-Screen_height:0;
          if (end_y<4)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+3,4-end_y);

          Update_rect(Mouse_X+start_x-6,Mouse_Y+start_y-6,13-end_x,13-end_y);
        }
        else
        {
          temp=(Config.Cursor)?CURSOR_SHAPE_THIN_TARGET:CURSOR_SHAPE_TARGET;
          start_x=Mouse_X-Gfx->Cursor_offset_X[temp];
          start_y=Mouse_Y-Gfx->Cursor_offset_Y[temp];

          for (y_pos=start_y,counter_y=0; counter_y<15 && y_pos < Screen_height;
			y_pos++,counter_y++)
          {
        	if( y_pos < 0 ) continue;
          	for (x_pos=start_x,counter_x=0;
				counter_x<15 && x_pos < Screen_width; x_pos++,counter_x++)
          	{
            	if( x_pos < 0 ) continue;
              	color=Gfx->Cursor_sprite[temp][counter_y][counter_x];
              	CURSOR_BACKGROUND[counter_y][counter_x]=Read_pixel(x_pos,y_pos);
              	if (color!=MC_Trans)
                	Pixel(x_pos,y_pos,color);
            }
          }

          Update_rect(Max(start_x,0),Max(start_y,0),counter_x,counter_y);
        }
      }
      break;

    case CURSOR_SHAPE_COLORPICKER:
      if (!Paintbrush_hidden)
        Display_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color,1);
      if (!Cursor_hidden)
      {
        if (Config.Cursor==1)
        {
          // Barres formant la croix principale

          start_y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (start_y<3)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+start_y-5,3-start_y);

          start_x=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (start_x<3)
            Horizontal_XOR_line(Mouse_X+start_x-5,Mouse_Y,3-start_x);

          end_x=(Mouse_X+6>Screen_width)?Mouse_X+6-Screen_width:0;
          if (end_x<3)
            Horizontal_XOR_line(Mouse_X+3,Mouse_Y,3-end_x);

          end_y=(Mouse_Y+6>Menu_Y/*Screen_height*/)?Mouse_Y+6-Menu_Y/*Screen_height*/:0;
          if (end_y<3)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+3,3-end_y);

          // Petites barres aux extrémités

          start_x=(!Mouse_X);
          start_y=(!Mouse_Y);
          end_x=(Mouse_X>=Screen_width-1);
          end_y=(Mouse_Y>=Menu_Y-1);

          if (Mouse_Y>5)
            Horizontal_XOR_line(start_x+Mouse_X-1,Mouse_Y-6,3-(start_x+end_x));

          if (Mouse_X>5)
            Vertical_XOR_line  (Mouse_X-6,start_y+Mouse_Y-1,3-(start_y+end_y));

          if (Mouse_X<Screen_width-6)
            Vertical_XOR_line  (Mouse_X+6,start_y+Mouse_Y-1,3-(start_y+end_y));

          if (Mouse_Y<Menu_Y-6)
            Horizontal_XOR_line(start_x+Mouse_X-1,Mouse_Y+6,3-(start_x+end_x));
        }
        else
        {
          DEBUG("B",0);
          temp=(Config.Cursor)?CURSOR_SHAPE_THIN_COLORPICKER:CURSOR_SHAPE_COLORPICKER;
          start_x=Mouse_X-Gfx->Cursor_offset_X[temp];
          start_y=Mouse_Y-Gfx->Cursor_offset_Y[temp];

          for (y_pos=start_y,counter_y=0;counter_y<15;y_pos++,counter_y++)
          {
        	if(y_pos<0) continue;
        	if(y_pos>=Screen_height) break;
          	for (x_pos=start_x,counter_x=0;counter_x<15;x_pos++,counter_x++)
          	{
              if(x_pos<0) continue;
              if(x_pos>=Screen_width) break;
              color=Gfx->Cursor_sprite[temp][counter_y][counter_x];
              // On sauvegarde dans CURSOR_BACKGROUND pour restaurer plus tard
              CURSOR_BACKGROUND[counter_y][counter_x]=Read_pixel(x_pos,y_pos);
              if (color!=MC_Trans)
            	Pixel(x_pos,y_pos,color);
            }
          }
          Update_rect(Max(start_x,0),Max(start_y,0),counter_x,counter_y);
        }
      }
      break;

    case CURSOR_SHAPE_MULTIDIRECTIONAL :
    case CURSOR_SHAPE_HORIZONTAL :
      if (Cursor_hidden)
        break;

    case CURSOR_SHAPE_ARROW :
    case CURSOR_SHAPE_HOURGLASS :
      start_x=Mouse_X-Gfx->Cursor_offset_X[shape];
      start_y=Mouse_Y-Gfx->Cursor_offset_Y[shape];
      for (y_pos=start_y,counter_y=0;counter_y<15;y_pos++,counter_y++)
      {
    	if(y_pos<0) continue;
        if(y_pos>=Screen_height) break;
    	for (x_pos=start_x,counter_x=0;counter_x<15;x_pos++,counter_x++)
		{
        	if(x_pos<0) continue;
        	if(x_pos>=Screen_width) break;
          	color=Gfx->Cursor_sprite[shape][counter_y][counter_x];
          	// On sauvegarde dans CURSOR_BACKGROUND pour restaurer plus tard
          	CURSOR_BACKGROUND[counter_y][counter_x]=Read_pixel(x_pos,y_pos);
          	if (color!=MC_Trans)
              Pixel(x_pos,y_pos,color);
        }
      }
      Update_rect(Max(start_x,0),Max(start_y,0),counter_x,counter_y);
      break;

    case CURSOR_SHAPE_XOR_TARGET :
      x_pos=Paintbrush_X-Main_offset_X;
      y_pos=Paintbrush_Y-Main_offset_Y;

      counter_x=(Main_magnifier_mode)?Main_separator_position:Screen_width; // width de la barre XOR
      if ((y_pos<Menu_Y) && (Paintbrush_Y>=Limit_top))
      {
        Horizontal_XOR_line(0,Paintbrush_Y-Main_offset_Y,counter_x);
        Update_rect(0,Paintbrush_Y-Main_offset_Y,counter_x,1);
      }

      if ((x_pos<counter_x) && (Paintbrush_X>=Limit_left))
      {
        Vertical_XOR_line(Paintbrush_X-Main_offset_X,0,Menu_Y);
        Update_rect(Paintbrush_X-Main_offset_X,0,1,Menu_Y);
      }

      if (Main_magnifier_mode)
      {
        // UPDATERECT
        if ((Paintbrush_Y>=Limit_top_zoom) && (Paintbrush_Y<=Limit_visible_bottom_zoom))
          Horizontal_XOR_line_zoom(Limit_left_zoom,Paintbrush_Y,Main_magnifier_width);
        if ((Paintbrush_X>=Limit_left_zoom) && (Paintbrush_X<=Limit_visible_right_zoom))
          Vertical_XOR_line_zoom(Paintbrush_X,Limit_top_zoom,Main_magnifier_height);
      }
      break;
    case CURSOR_SHAPE_XOR_RECTANGLE :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      start_x=(Mouse_X-3);
      start_y=(Mouse_Y-3);
      end_x  =(Mouse_X+4);
      end_y  =(Mouse_Y+4);
      if (start_x<0)
        start_x=0;
      if (start_y<0)
        start_y=0;
      if (end_x>Screen_width)
        end_x=Screen_width;
      if (end_y>Menu_Y)
        end_y=Menu_Y;

      Horizontal_XOR_line(start_x,Mouse_Y,end_x-start_x);
      Vertical_XOR_line  (Mouse_X,start_y,end_y-start_y);

      // Grand rectangle autour
      start_x=Mouse_X-(Main_magnifier_width>>1);
      start_y=Mouse_Y-(Main_magnifier_height>>1);
      if (start_x+Main_magnifier_width>=Limit_right-Main_offset_X)
        start_x=Limit_right-Main_magnifier_width-Main_offset_X+1;
      if (start_y+Main_magnifier_height>=Limit_bottom-Main_offset_Y)
        start_y=Limit_bottom-Main_magnifier_height-Main_offset_Y+1;
      if (start_x<0)
        start_x=0;
      if (start_y<0)
        start_y=0;
      end_x=start_x+Main_magnifier_width-1;
      end_y=start_y+Main_magnifier_height-1;

      Horizontal_XOR_line(start_x,start_y,Main_magnifier_width);
      Vertical_XOR_line(start_x,start_y+1,Main_magnifier_height-2);
      Vertical_XOR_line(  end_x,start_y+1,Main_magnifier_height-2);
      Horizontal_XOR_line(start_x,  end_y,Main_magnifier_width);

      Update_rect(start_x,start_y,end_x+1-start_x,end_y+1-start_y);

      break;
    default: //case CURSOR_SHAPE_XOR_ROTATION :
      start_x=1-(Brush_width>>1);
      start_y=1-(Brush_height>>1);
      end_x=start_x+Brush_width-1;
      end_y=start_y+Brush_height-1;

      if (Brush_rotation_center_is_defined)
      {
        if ( (Brush_rotation_center_X==Paintbrush_X)
          && (Brush_rotation_center_Y==Paintbrush_Y) )
        {
          cos_a=1.0;
          sin_a=0.0;
        }
        else
        {
          x_pos=Paintbrush_X-Brush_rotation_center_X;
          y_pos=Paintbrush_Y-Brush_rotation_center_Y;
          cos_a=(float)x_pos/sqrt((x_pos*x_pos)+(y_pos*y_pos));
          sin_a=sin(acos(cos_a));
          if (y_pos>0) sin_a=-sin_a;
        }

        Transform_point(start_x,start_y, cos_a,sin_a, &x1,&y1);
        Transform_point(end_x  ,start_y, cos_a,sin_a, &x2,&y2);
        Transform_point(start_x,end_y  , cos_a,sin_a, &x3,&y3);
        Transform_point(end_x  ,end_y  , cos_a,sin_a, &x4,&y4);

        x1+=Brush_rotation_center_X;
        y1+=Brush_rotation_center_Y;
        x2+=Brush_rotation_center_X;
        y2+=Brush_rotation_center_Y;
        x3+=Brush_rotation_center_X;
        y3+=Brush_rotation_center_Y;
        x4+=Brush_rotation_center_X;
        y4+=Brush_rotation_center_Y;
        Pixel_figure_preview_xor(Brush_rotation_center_X,Brush_rotation_center_Y,0);
        Draw_line_preview_xor(Brush_rotation_center_X,Brush_rotation_center_Y,Paintbrush_X,Paintbrush_Y,0);
      }
      else
      {
        x1=x3=1-Brush_width;
        y1=y2=start_y;
        x2=x4=Paintbrush_X;
        y3=y4=end_y;

        x1+=Paintbrush_X;
        y1+=Paintbrush_Y;
        y2+=Paintbrush_Y;
        x3+=Paintbrush_X;
        y3+=Paintbrush_Y;
        y4+=Paintbrush_Y;
        Pixel_figure_preview_xor(Paintbrush_X-end_x,Paintbrush_Y,0);
        Draw_line_preview_xor(Paintbrush_X-end_x,Paintbrush_Y,Paintbrush_X,Paintbrush_Y,0);
      }

      Draw_line_preview_xor(x1,y1,x2,y2,0);
      Draw_line_preview_xor(x2,y2,x4,y4,0);
      Draw_line_preview_xor(x4,y4,x3,y3,0);
      Draw_line_preview_xor(x3,y3,x1,y1,0);
  }
}

  // -- Effacer le curseur --

void Hide_cursor(void)
{
  byte  shape;
  int start_x; // int car sont parfois négatifs ! (quand on dessine sur un bord)
  int start_y;
  short end_x;
  short end_y;
  int x_pos = 0;
  int y_pos;
  short counter_x = 0;
  short counter_y;
  int   temp;
  float cos_a,sin_a;
  short x1,y1,x2,y2,x3,y3,x4,y4;

  if ( ( (Mouse_Y<Menu_Y)
      && ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position)
                         || (Mouse_X>=Main_X_zoom) ) )
    || (Windows_open) || (Cursor_shape==CURSOR_SHAPE_HOURGLASS) )
    shape=Cursor_shape;
  else
    shape=CURSOR_SHAPE_ARROW;

  switch(shape)
  {
    case CURSOR_SHAPE_TARGET :
      if (!Cursor_hidden)
      {
        if (Config.Cursor==1)
        {
          start_y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (start_y<4)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+start_y-6,4-start_y);

          start_x=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (start_x<4)
            Horizontal_XOR_line(Mouse_X+start_x-6,Mouse_Y,4-start_x);

          end_x=(Mouse_X+7>Screen_width)?Mouse_X+7-Screen_width:0;
          if (end_x<4)
            Horizontal_XOR_line(Mouse_X+3,Mouse_Y,4-end_x);

          end_y=(Mouse_Y+7>Screen_height)?Mouse_Y+7-Screen_height:0;
          if (end_y<4)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+3,4-end_y);

          Update_rect(Mouse_X+start_x-6,Mouse_Y+start_y-6,13-end_x,13-end_y);
        }
        else
        {
          temp=(Config.Cursor)?CURSOR_SHAPE_THIN_TARGET:CURSOR_SHAPE_TARGET;
          start_x=Mouse_X-Gfx->Cursor_offset_X[temp];
          start_y=Mouse_Y-Gfx->Cursor_offset_Y[temp];

          for (y_pos=start_y,counter_y=0;counter_y<15;y_pos++,counter_y++)
          {
			if(y_pos < 0) continue;
            if(y_pos>=Screen_height) break;
            for (x_pos=start_x,counter_x=0;counter_x<15;x_pos++,counter_x++)
            {
              if(x_pos < 0) continue;
              else if (x_pos>=Screen_width) break;
              Pixel(x_pos,y_pos,CURSOR_BACKGROUND[counter_y][counter_x]);
            }
          }

          Update_rect(Max(start_x,0),Max(start_y,0),x_pos-start_x,y_pos-start_y);
        }
      }
      if (!Paintbrush_hidden)
      {
        Hide_paintbrush(Paintbrush_X,Paintbrush_Y);
      }
      break;

    case CURSOR_SHAPE_COLORPICKER:
      if (!Cursor_hidden)
      {
        if (Config.Cursor==1)
        {
          // Barres formant la croix principale

          start_y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (start_y<3)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+start_y-5,3-start_y);

          start_x=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (start_x<3)
            Horizontal_XOR_line(Mouse_X+start_x-5,Mouse_Y,3-start_x);

          end_x=(Mouse_X+6>Screen_width)?Mouse_X+6-Screen_width:0;
          if (end_x<3)
            Horizontal_XOR_line(Mouse_X+3,Mouse_Y,3-end_x);

          end_y=(Mouse_Y+6>Screen_height)?Mouse_Y+6-Screen_height:0;
          if (end_y<3)
            Vertical_XOR_line  (Mouse_X,Mouse_Y+3,3-end_y);

          start_x=(!Mouse_X);
          start_y=(!Mouse_Y);
          end_x=(Mouse_X>=Screen_width-1);
          end_y=(Mouse_Y>=Menu_Y-1);

          if (Mouse_Y>5)
            Horizontal_XOR_line(start_x+Mouse_X-1,Mouse_Y-6,3-(start_x+end_x));

          if (Mouse_X>5)
            Vertical_XOR_line  (Mouse_X-6,start_y+Mouse_Y-1,3-(start_y+end_y));

          if (Mouse_X<Screen_width-6)
            Vertical_XOR_line  (Mouse_X+6,start_y+Mouse_Y-1,3-(start_y+end_y));

          if (Mouse_Y<Menu_Y-6)
            Horizontal_XOR_line(start_x+Mouse_X-1,Mouse_Y+6,3-(start_x+end_x));

          Update_rect(start_x,start_y,end_x-start_x,end_y-start_y);
        }
        else
        {
          temp=(Config.Cursor)?CURSOR_SHAPE_THIN_COLORPICKER:CURSOR_SHAPE_COLORPICKER;
          start_x=Mouse_X-Gfx->Cursor_offset_X[temp];
          start_y=Mouse_Y-Gfx->Cursor_offset_Y[temp];

          for (y_pos=start_y,counter_y=0;counter_y<15;y_pos++,counter_y++)
          {
        	if(y_pos<0) continue;
            if(y_pos>=Screen_height) break;
          	for (x_pos=start_x,counter_x=0;counter_x<15;x_pos++,counter_x++)
          	{
              if(x_pos<0) continue;
              if(x_pos>=Screen_width) break;
                  Pixel(x_pos,y_pos,CURSOR_BACKGROUND[counter_y][counter_x]);
            }
          }
          Update_rect(Max(start_x,0),Max(start_y,0),counter_x,counter_y);
        }
      }
      if (!Paintbrush_hidden)
        Hide_paintbrush(Paintbrush_X,Paintbrush_Y);
      break;

    case CURSOR_SHAPE_MULTIDIRECTIONAL :
    case CURSOR_SHAPE_HORIZONTAL :
      if (Cursor_hidden)
        break;

    case CURSOR_SHAPE_ARROW :
    case CURSOR_SHAPE_HOURGLASS :
      start_x=Mouse_X-Gfx->Cursor_offset_X[shape];
      start_y=Mouse_Y-Gfx->Cursor_offset_Y[shape];

      for (y_pos=start_y,counter_y=0;counter_y<15;y_pos++,counter_y++)
      {
        if(y_pos<0) continue;
        if(y_pos>=Screen_height) break;
      	for (x_pos=start_x,counter_x=0;counter_x<15;x_pos++,counter_x++)
      	{
        	if(x_pos<0) continue;
        	if(x_pos>=Screen_width) break;
            Pixel(x_pos,y_pos,CURSOR_BACKGROUND[counter_y][counter_x]);
        }
      }
      Update_rect(Max(start_x,0),Max(start_y,0),counter_x,counter_y);
      break;

    case CURSOR_SHAPE_XOR_TARGET :
      x_pos=Paintbrush_X-Main_offset_X;
      y_pos=Paintbrush_Y-Main_offset_Y;

      counter_x=(Main_magnifier_mode)?Main_separator_position:Screen_width; // width de la barre XOR
      if ((y_pos<Menu_Y) && (Paintbrush_Y>=Limit_top))
      {
        Horizontal_XOR_line(0,Paintbrush_Y-Main_offset_Y,counter_x);
        Update_rect(0,Paintbrush_Y-Main_offset_Y,counter_x,1);
      }

      if ((x_pos<counter_x) && (Paintbrush_X>=Limit_left))
      {
        Vertical_XOR_line(Paintbrush_X-Main_offset_X,0,Menu_Y);
        Update_rect(Paintbrush_X-Main_offset_X,0,1,Menu_Y);
      }

      if (Main_magnifier_mode)
      {
        // UPDATERECT
        if ((Paintbrush_Y>=Limit_top_zoom) && (Paintbrush_Y<=Limit_visible_bottom_zoom))
          Horizontal_XOR_line_zoom(Limit_left_zoom,Paintbrush_Y,Main_magnifier_width);
        if ((Paintbrush_X>=Limit_left_zoom) && (Paintbrush_X<=Limit_visible_right_zoom))
          Vertical_XOR_line_zoom(Paintbrush_X,Limit_top_zoom,Main_magnifier_height);
      }


      break;
    case CURSOR_SHAPE_XOR_RECTANGLE :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      start_x=(Mouse_X-3);
      start_y=(Mouse_Y-3);
      end_x  =(Mouse_X+4);
      end_y  =(Mouse_Y+4);
      if (start_x<0)
        start_x=0;
      if (start_y<0)
        start_y=0;
      if (end_x>Screen_width)
        end_x=Screen_width;
      if (end_y>Menu_Y)
        end_y=Menu_Y;

      Horizontal_XOR_line(start_x,Mouse_Y,end_x-start_x);
      Vertical_XOR_line  (Mouse_X,start_y,end_y-start_y);

      // Grand rectangle autour

      start_x=Mouse_X-(Main_magnifier_width>>1);
      start_y=Mouse_Y-(Main_magnifier_height>>1);
      if (start_x+Main_magnifier_width>=Limit_right-Main_offset_X)
        start_x=Limit_right-Main_magnifier_width-Main_offset_X+1;
      if (start_y+Main_magnifier_height>=Limit_bottom-Main_offset_Y)
        start_y=Limit_bottom-Main_magnifier_height-Main_offset_Y+1;
      if (start_x<0)
        start_x=0;
      if (start_y<0)
        start_y=0;
      end_x=start_x+Main_magnifier_width-1;
      end_y=start_y+Main_magnifier_height-1;

      Horizontal_XOR_line(start_x,start_y,Main_magnifier_width);
      Vertical_XOR_line(start_x,start_y+1,Main_magnifier_height-2);
      Vertical_XOR_line(  end_x,start_y+1,Main_magnifier_height-2);
      Horizontal_XOR_line(start_x,  end_y,Main_magnifier_width);

      Update_rect(start_x,start_y,end_x+1-start_x,end_y+1-start_y);

      break;
    default: //case CURSOR_SHAPE_XOR_ROTATION :
      start_x=1-(Brush_width>>1);
      start_y=1-(Brush_height>>1);
      end_x=start_x+Brush_width-1;
      end_y=start_y+Brush_height-1;

      if (Brush_rotation_center_is_defined)
      {
        if ( (Brush_rotation_center_X==Paintbrush_X)
          && (Brush_rotation_center_Y==Paintbrush_Y) )
        {
          cos_a=1.0;
          sin_a=0.0;
        }
        else
        {
          x_pos=Paintbrush_X-Brush_rotation_center_X;
          y_pos=Paintbrush_Y-Brush_rotation_center_Y;
          cos_a=(float)x_pos/sqrt((x_pos*x_pos)+(y_pos*y_pos));
          sin_a=sin(acos(cos_a));
          if (y_pos>0) sin_a=-sin_a;
        }

        Transform_point(start_x,start_y, cos_a,sin_a, &x1,&y1);
        Transform_point(end_x  ,start_y, cos_a,sin_a, &x2,&y2);
        Transform_point(start_x,end_y  , cos_a,sin_a, &x3,&y3);
        Transform_point(end_x  ,end_y  , cos_a,sin_a, &x4,&y4);

        x1+=Brush_rotation_center_X;
        y1+=Brush_rotation_center_Y;
        x2+=Brush_rotation_center_X;
        y2+=Brush_rotation_center_Y;
        x3+=Brush_rotation_center_X;
        y3+=Brush_rotation_center_Y;
        x4+=Brush_rotation_center_X;
        y4+=Brush_rotation_center_Y;
        Pixel_figure_preview_xor(Brush_rotation_center_X,Brush_rotation_center_Y,0);
        Draw_line_preview_xor(Brush_rotation_center_X,Brush_rotation_center_Y,Paintbrush_X,Paintbrush_Y,0);
      }
      else
      {
        x1=x3=1-Brush_width;
        y1=y2=start_y;
        x2=x4=Paintbrush_X;
        y3=y4=end_y;

        x1+=Paintbrush_X;
        y1+=Paintbrush_Y;
        y2+=Paintbrush_Y;
        x3+=Paintbrush_X;
        y3+=Paintbrush_Y;
        y4+=Paintbrush_Y;
        Pixel_figure_preview_xor(Paintbrush_X-end_x,Paintbrush_Y,0);
        Draw_line_preview_xor(Paintbrush_X-end_x,Paintbrush_Y,Paintbrush_X,Paintbrush_Y,0);
      }

      Draw_line_preview_xor(x1,y1,x2,y2,0);
      Draw_line_preview_xor(x2,y2,x4,y4,0);
      Draw_line_preview_xor(x4,y4,x3,y3,0);
      Draw_line_preview_xor(x3,y3,x1,y1,0);
  }
}



// -- Fonction diverses d'affichage ------------------------------------------

  // -- Reafficher toute l'image (en prenant en compte le facteur de zoom) --

void Display_all_screen(void)
{
  word width;
  word height;

  // ---/\/\/\  Partie non zoomée: /\/\/\---
  if (Main_magnifier_mode)
  {
    if (Main_image_width<Main_separator_position)
      width=Main_image_width;
    else
      width=Main_separator_position;
  }
  else
  {
    if (Main_image_width<Screen_width)
      width=Main_image_width;
    else
      width=Screen_width;
  }
  if (Main_image_height<Menu_Y)
    height=Main_image_height;
  else
    height=Menu_Y;
  Display_screen(width,height,Main_image_width);

  // Effacement de la partie non-image dans la partie non zoomée:
  if (Main_magnifier_mode)
  {
    if (Main_image_width<Main_separator_position && Main_image_width < Screen_width)
      Block(Main_image_width,0,(Main_separator_position-Main_image_width),Menu_Y,0);
  }
  else
  {
    if (Main_image_width<Screen_width)
      Block(Main_image_width,0,(Screen_width-Main_image_width),Menu_Y,0);
  }
  if (Main_image_height<Menu_Y)
    Block(0,Main_image_height,width,(Menu_Y-height),0);

  // ---/\/\/\  Partie zoomée: /\/\/\---
  if (Main_magnifier_mode)
  {
    // Affichage de la barre de split
    Display_separator();

    // Calcul de la largeur visible
    if (Main_image_width<Main_magnifier_width)
      width=Main_image_width;
    else
      width=Main_magnifier_width;

    // Calcul du nombre de lignes visibles de l'image zoomée
    if (Main_image_height<Main_magnifier_height)
      height=Main_image_height*Main_magnifier_factor;
    else
      height=Menu_Y;

    Display_zoomed_screen(width,height,Main_image_width,Horizontal_line_buffer);

    // Effacement de la partie non-image dans la partie zoomée:
    if (Main_image_width<Main_magnifier_width)
      Block(Main_X_zoom+(Main_image_width*Main_magnifier_factor),0,
            (Main_magnifier_width-Main_image_width)*Main_magnifier_factor,
            Menu_Y,0);
    if (Main_image_height<Main_magnifier_height)
      Block(Main_X_zoom,height,width*Main_magnifier_factor,(Menu_Y-height),0);
  }

  // ---/\/\/\ Affichage des limites /\/\/\---
  if (Config.Display_image_limits)
    Display_image_limits();
  Update_rect(0,0,Screen_width,Menu_Y); // TODO On peut faire plus fin, en évitant de mettre à jour la partie à droite du split quand on est en mode loupe. Mais c'est pas vraiment intéressant ?
}



byte Best_color(byte r,byte g,byte b)
{
  short col;
  int   delta_r,delta_g,delta_b;
  int   dist;
  int   best_dist=0x7FFFFFFF;
  int   rmean;
  byte  best_color=0;

  for (col=0; col<256; col++)
  {
    if (!Exclude_color[col])
    {
      delta_r=(int)Main_palette[col].R-r;
      delta_g=(int)Main_palette[col].G-g;
      delta_b=(int)Main_palette[col].B-b;

      rmean = ( Main_palette[col].R + r ) / 2;

      if (!(dist= ( ( (512+rmean) *delta_r*delta_r) >>8) + 4*delta_g*delta_g + (((767-rmean)*delta_b*delta_b)>>8)))
      //if (!(dist=(delta_r*delta_r*30)+(delta_g*delta_g*59)+(delta_b*delta_b*11)))
        return col;

      if (dist<best_dist)
      {
        best_dist=dist;
        best_color=col;
      }
    }
  }

  return best_color;
}

byte Best_color_nonexcluded(byte red,byte green,byte blue)
{
  short col;
  int   delta_r,delta_g,delta_b;
  int   dist;
  int   best_dist=0x7FFFFFFF;
  int   rmean;
  byte  best_color=0;

  for (col=0; col<256; col++)
  {
    delta_r=(int)Main_palette[col].R-red;
    delta_g=(int)Main_palette[col].G-green;
    delta_b=(int)Main_palette[col].B-blue;
    rmean = ( Main_palette[col].R + red ) / 2;

    if (!(dist= ( ( (512+rmean) *delta_r*delta_r) >>8) + 4*delta_g*delta_g + (((767-rmean)*delta_b*delta_b)>>8)))
    //if (!(dist=(delta_r*delta_r*30)+(delta_g*delta_g*59)+(delta_b*delta_b*11)))
      return col;

    if (dist<best_dist)
    {
      best_dist=dist;
      best_color=col;
    }
  }
  return best_color;
}

void Compute_4_best_colors_for_1_menu_color
     (byte red, byte green, byte blue, T_Components * palette, byte * table)
{
  short col;
  int   delta_r,delta_g,delta_b;
  int   dist;
  int   best_dist[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};


  for (col=0; col<256; col++)
  {
    delta_r=(int)palette[col].R-red;
    delta_g=(int)palette[col].G-green;
    delta_b=(int)palette[col].B-blue;

    dist=(delta_r*delta_r*30)+(delta_g*delta_g*59)+(delta_b*delta_b*11);

    if (dist<best_dist[0])
    {
      best_dist[3]=best_dist[2];
      best_dist[2]=best_dist[1];
      best_dist[1]=best_dist[0];
      best_dist[0]=dist;
      table[3]=table[2];
      table[2]=table[1];
      table[1]=table[0];
      table[0]=col;
    }
    else
    {
      if (dist<best_dist[1])
      {
        best_dist[3]=best_dist[2];
        best_dist[2]=best_dist[1];
        best_dist[1]=dist;
        table[3]=table[2];
        table[2]=table[1];
        table[1]=col;
      }
      else
      {
        if (dist<best_dist[2])
        {
          best_dist[3]=best_dist[2];
          best_dist[2]=dist;
          table[3]=table[2];
          table[2]=col;
        }
        else
        if (dist<best_dist[3])
        {
          best_dist[3]=dist;
          table[3]=col;
        }
      }
    }
  }
}



byte Old_black;
byte Old_dark;
byte Old_light;
byte Old_white;
byte Old_trans;

void Remap_pixel(byte * pixel)
{
  if (*pixel==Old_light)         // On commence par tester le Gris clair
    *pixel=MC_Light;             // qui est pas mal utilisé.
  else
  {
    if (*pixel==Old_black)        // Puis le Noir...
      *pixel=MC_Black;
    else
    {
      if (*pixel==Old_dark)     // etc...
        *pixel=MC_Dark;
      else
      {
        if (*pixel==Old_white)
          *pixel=MC_White;
        else
        {
          if (*pixel==Old_trans)
            *pixel=MC_Trans;
        }
      }
    }
  }
}



void Remap_screen_after_menu_colors_change(void)
{
  short index;
  byte  conversion_table[256];
  short temp/*,temp2*/;

  if ( (MC_Light!=Old_light) || (MC_Dark!=Old_dark) || (MC_White!=Old_white) || (MC_Black !=Old_black )
    || (MC_Trans!=Old_trans) )
  {
    // Création de la table de conversion
    for (index=0; index<256; index++)
      conversion_table[index]=index;

    conversion_table[Old_black ]=MC_Black;
    conversion_table[Old_dark]=MC_Dark;
    conversion_table[Old_light]=MC_Light;
    conversion_table[Old_white]=MC_White;

    // Remappage de l'écran

    temp=Window_height*Menu_factor_Y;

    Remap_screen(Window_pos_X, Window_pos_Y,
                 Window_width*Menu_factor_X,
                 (Window_pos_Y+temp<Menu_Y_before_window)?temp:Menu_Y_before_window-Window_pos_Y,
                 conversion_table);

    if (Menu_is_visible_before_window)
    {
      Remap_screen(0, Menu_Y_before_window,
                   Screen_width, Screen_height-Menu_Y_before_window,
                   conversion_table);
      // Remappage de la partie du fond de la fenetre qui cacherait le menu...
      Remap_window_backgrounds(conversion_table, Menu_Y_before_window, Screen_height);
      /*
         Il faudrait peut-être remapper les pointillés délimitant l'image.
         Mais ça va être chiant parce qu'ils peuvent être affichés en mode Loupe.
         Mais de toutes façons, c'est franchement facultatif...
      */
      // On passe la table juste pour ne rafficher que les couleurs modifiées
      Display_menu_palette_avoiding_window(conversion_table);
    }
    Clear_border(MC_Black);
  }

}




void Compute_optimal_menu_colors(T_Components * palette)
{
  byte table[4];
  short i,j,k;


  Old_black =MC_Black;
  Old_dark=MC_Dark;
  Old_light=MC_Light;
  Old_white=MC_White;
  Old_trans=MC_Trans;

  // Recherche du noir
  Compute_4_best_colors_for_1_menu_color
    (Fav_menu_colors[0].R, Fav_menu_colors[0].G, Fav_menu_colors[0].B,palette,table);
  MC_Black=table[0];

  // Recherche du blanc
  Compute_4_best_colors_for_1_menu_color
    (Fav_menu_colors[3].R, Fav_menu_colors[3].G, Fav_menu_colors[3].B,palette,table);
  if (MC_Black!=table[0])
    MC_White=table[0];
  else
    MC_White=table[1];

  // Recherche du gris clair
  Compute_4_best_colors_for_1_menu_color
    (Fav_menu_colors[2].R, Fav_menu_colors[2].G, Fav_menu_colors[2].B,palette,table);
  if ( (MC_Black!=table[0]) && (MC_White!=table[0]) )
    MC_Light=table[0];
  else
  {
    if ( (MC_Black!=table[1]) && (MC_White!=table[1]) )
      MC_Light=table[1];
    else
      MC_Light=table[2];
  }

  // Recherche du gris foncé
  Compute_4_best_colors_for_1_menu_color
    (Fav_menu_colors[1].R, Fav_menu_colors[1].G, Fav_menu_colors[1].B,palette,table);
  if ( (MC_Black!=table[0]) && (MC_White!=table[0]) && (MC_Light!=table[0]) )
    MC_Dark=table[0];
  else
  {
    if ( (MC_Black!=table[1]) && (MC_White!=table[1]) && (MC_Light!=table[1]) )
      MC_Dark=table[1];
    else
    {
      if ( (MC_Black!=table[2]) && (MC_White!=table[2]) && (MC_Light!=table[2]) )
        MC_Dark=table[2];
      else
        MC_Dark=table[3];
    }
  }

  // C'est peu probable mais il est possible que MC_Light soit plus foncée que
  // MC_Dark. Dans ce cas, on les inverse.
  if ( ((palette[MC_Light].R*30)+(palette[MC_Light].G*59)+(palette[MC_Light].B*11)) <
       ((palette[MC_Dark].R*30)+(palette[MC_Dark].G*59)+(palette[MC_Dark].B*11)) )
  {
    i=MC_Light;
    MC_Light=MC_Dark;
    MC_Dark=i;
  }

  // On cherche une couleur de transparence différente des 4 autres.
  for (MC_Trans=0; ((MC_Trans==MC_Black) || (MC_Trans==MC_Dark) ||
                   (MC_Trans==MC_Light) || (MC_Trans==MC_White)); MC_Trans++);

  // Et maintenant, on "remappe" tous les sprites, etc...
  if ( (MC_Light!=Old_light)
    || (MC_Dark!=Old_dark)
    || (MC_White!=Old_white)
    || (MC_Black !=Old_black )
    || (MC_Trans!=Old_trans) )
  {
    // Sprites du curseur
    for (k=0; k<NB_CURSOR_SPRITES; k++)
      for (j=0; j<CURSOR_SPRITE_HEIGHT; j++)
        for (i=0; i<CURSOR_SPRITE_WIDTH; i++)
          Remap_pixel(&Gfx->Cursor_sprite[k][j][i]);
    // Le menu
    for (j=0; j<MENU_HEIGHT; j++)
      for (i=0; i<MENU_WIDTH; i++)
        Remap_pixel(&Gfx->Menu_block[j][i]);
    // Sprites du menu
    for (k=0; k<NB_MENU_SPRITES; k++)
      for (j=0; j<MENU_SPRITE_HEIGHT; j++)
        for (i=0; i<MENU_SPRITE_WIDTH; i++)
          Remap_pixel(&Gfx->Menu_sprite[k][j][i]);
    // Sprites d'effets
    for (k=0; k<NB_EFFECTS_SPRITES; k++)
      for (j=0; j<MENU_SPRITE_HEIGHT; j++)
        for (i=0; i<MENU_SPRITE_WIDTH; i++)
          Remap_pixel(&Gfx->Effect_sprite[k][j][i]);
    // Fontes de l'aide
    for (k=0; k<256; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Help_font_norm[k][i][j]);
    for (k=0; k<256; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Bold_font[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Help_font_t1[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Help_font_t2[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Help_font_t3[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Gfx->Help_font_t4[k][i][j]);
        
    // Sprites de lecteurs (drives)
    for (k=0; k<NB_ICON_SPRITES; k++)
      for (j=0; j<ICON_SPRITE_HEIGHT; j++)
        for (i=0; i<ICON_SPRITE_WIDTH; i++)
          Remap_pixel(&Gfx->Icon_sprite[k][j][i]);
  }
  Clear_border(MC_Black);
}
