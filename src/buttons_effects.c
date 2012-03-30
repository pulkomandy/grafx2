/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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

////////////////////////////////////////////////////////////////////////////
///@file buttons_effects.c
/// Handles all the effects buttons and setup windows in the effects menu.
////////////////////////////////////////////////////////////////////////////

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "brush.h"
#include "buttons.h"
#include "engine.h"
#include "global.h"
#include "graph.h"
#include "help.h"
#include "input.h"
#include "misc.h"
#include "pages.h"
#include "readline.h"
#include "sdlscreen.h"
#include "struct.h"
#include "windows.h"
#include "tiles.h"

//---------- Menu dans lequel on tagge des couleurs (genre Stencil) ----------
void Menu_tag_colors(char * window_title, byte * table, byte * mode, byte can_cancel, const char *help_section, word close_shortcut)
{
  short clicked_button;
  byte backup_table[256];
  word index;
  word old_mouse_x;
  word old_mouse_y;
  byte old_mouse_k;
  byte tagged_color;
  byte color;
  byte click;


  Open_window(176,150,window_title);

  Window_set_palette_button(6,38);                            // 1
  Window_set_normal_button( 7, 19,78,14,"Clear" ,1,1,SDLK_c); // 2
  Window_set_normal_button(91, 19,78,14,"Invert",1,1,SDLK_i); // 3
  if (can_cancel)
  {
    Window_set_normal_button(91,129,78,14,"OK"    ,0,1,SDLK_RETURN); // 4
    Window_set_normal_button( 7,129,78,14,"Cancel",0,1,KEY_ESC);  // 5
    // On enregistre la table dans un backup au cas où on ferait Cancel
    memcpy(backup_table,table,256);
  }
  else
    Window_set_normal_button(49,129,78,14,"OK"    ,0,1,SDLK_RETURN); // 4

  // On affiche l'état actuel de la table
  for (index=0; index<=255; index++)
    Stencil_tag_color(index, (table[index])?MC_Black:MC_Light);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;

    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case  0 :
        break;
      case -1 :
      case  1 : // Palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          tagged_color=(clicked_button==1) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);
          table[tagged_color]=(Mouse_K==LEFT_SIDE);
          Stencil_tag_color(tagged_color,(Mouse_K==LEFT_SIDE)?MC_Black:MC_Light);
          Display_cursor();
          Stencil_update_color(tagged_color);
        }
        break;
      case  2 : // Clear
        memset(table,0,256);
        Hide_cursor();
        for (index=0; index<=255; index++)
          Stencil_tag_color(index,MC_Light);
        Display_cursor();
        Update_window_area(0,0,Window_width, Window_height);
        break;
      case  3 : // Invert
        Hide_cursor();
        for (index=0; index<=255; index++)
          Stencil_tag_color(index,(table[index]^=1)?MC_Black:MC_Light);
        Display_cursor();
        Update_window_area(0,0,Window_width, Window_height);
    }

    if (!Mouse_K)
    switch (Key)
    {
      case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case SDLK_COMMA :
        Get_color_behind_window(&color,&click);
        if (click)
        {
          Hide_cursor();
          tagged_color=color;
          table[tagged_color]=(click==LEFT_SIDE);
          Stencil_tag_color(tagged_color,(click==LEFT_SIDE)?MC_Black:MC_Light);
          Stencil_update_color(tagged_color);
          Display_cursor();
          Wait_end_of_click();
        }
        Key=0;
        break;
      default:
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
      {
        Window_help(BUTTON_EFFECTS, help_section);
        Key=0;
        break;
      }
      else if (Is_shortcut(Key,close_shortcut))
      {
        clicked_button=4;
      }
    }
  }
  while (clicked_button<4);

  Close_window();

  if (clicked_button==5) // Cancel
    memcpy(table,backup_table,256);
  else // OK
    *mode=1;

  Display_cursor();
}


// Constaint enforcer/checker ------------------------------------------------
void Button_Constraint_mode(void)
{
  if (Main_backups->Pages->Image_mode == IMAGE_MODE_LAYERED)
  {
    Main_backups->Pages->Image_mode = IMAGE_MODE_MODE5;
  	// TODO backup
  	// CPC Mode 5 - ensure there are at least 5 layers
  	while(Main_backups->Pages->Nb_layers < 5)
  	{
  		Add_layer(Main_backups, 0);
  			// Add layers below, so the picture is kept on the top one
  			// (the valuable pixel data)
  	}
  	
  	// TODO set the palette to a CPC one ?
  }
  else if (Main_backups->Pages->Image_mode == IMAGE_MODE_MODE5)
  {
    // Disable
    Main_backups->Pages->Image_mode = IMAGE_MODE_LAYERED;
  }
}


void Button_Constraint_menu(void)
{

}

// Tilemap mode
void Button_Tilemap_mode(void)
{
  Main_tilemap_mode=!Main_tilemap_mode;
  Tilemap_update();
}

void Button_Tilemap_menu(void)
{
  short clicked_button;

  byte flip_x=Config.Tilemap_allow_flipped_x;
  byte flip_y=Config.Tilemap_allow_flipped_y;
  byte count=Config.Tilemap_show_count;

  Open_window(166,120,"Tilemap options");

  Window_set_normal_button(6,102,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(110,102,51,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Print_in_window(24,21, "Detect mirrored",MC_Dark,MC_Light);
  Window_display_frame(5,17,155,56);
  
  Print_in_window(37,37, "Horizontally",MC_Black,MC_Light);
  Window_set_normal_button(18,34,13,13,flip_x?"X":"",0,1,0);  // 3

  Print_in_window(37,55, "Vertically",MC_Black,MC_Light);
  Window_set_normal_button(18,52,13,13,flip_y?"X":"",0,1,0);  // 4

  Print_in_window(27,81, "Show count",MC_Black,MC_Light);
  Window_set_normal_button(7,78,13,13,count?"X":"",0,1,0);  // 5

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case 3 : // Horizontal flip
        flip_x=!flip_x;
        Hide_cursor();
        Print_in_window(21,37,flip_x?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;
      case 4 : // Vertical flip
        flip_y=!flip_y;
        Hide_cursor();
        Print_in_window(21,55,flip_y?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;
      case 5 : // Count
        count=!count;
        Hide_cursor();
        Print_in_window(10,81,count?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;      
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "TILEMAP");
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  if (clicked_button==2) // OK
  {
    byte changed =
      Config.Tilemap_allow_flipped_x!=flip_x ||
      Config.Tilemap_allow_flipped_y!=flip_y ||
      !Main_tilemap_mode;
    
    Config.Tilemap_allow_flipped_x=flip_x;
    Config.Tilemap_allow_flipped_y=flip_y;
    Config.Tilemap_show_count=count;
    
    if (changed)
    {
      Main_tilemap_mode=1;
      Tilemap_update();
    }
  }
  Close_window();
  Display_cursor();
}

//--------------------------------- Stencil ----------------------------------
void Button_Stencil_mode(void)
{
  Stencil_mode=!Stencil_mode;
}


void Stencil_tag_color(byte color, byte tag_color)
{
  Block(Window_pos_X+(Menu_factor_X*(Window_palette_button_list->Pos_X+4+(color >> 4)*10)),
        Window_pos_Y+(Menu_factor_Y*(Window_palette_button_list->Pos_Y+3+(color & 15)* 5)),
        Menu_factor_X<<1,Menu_factor_Y*5,tag_color);
}

void Stencil_update_color(byte color)
{
  Update_rect(Window_pos_X+(Menu_factor_X*(Window_palette_button_list->Pos_X+4+(color >> 4)*10)),
      Window_pos_Y+(Menu_factor_Y*(Window_palette_button_list->Pos_Y+3+(color & 15)* 5)),
      Menu_factor_X<<1,Menu_factor_Y*5);
}

void Button_Stencil_menu(void)
{
  Menu_tag_colors("Stencil",Stencil,&Stencil_mode,1, "STENCIL", SPECIAL_STENCIL_MENU);
}


//--------------------------------- Masque -----------------------------------
void Button_Mask_mode(void)
{
  Mask_mode=!Mask_mode;
}


void Button_Mask_menu(void)
{
  Menu_tag_colors("Mask",Mask_table,&Mask_mode,1, "MASK", SPECIAL_MASK_MENU);
}


// -------------------------------- Grille -----------------------------------

void Button_Snap_mode(void)
{
  Hide_cursor();
  Snap_mode=!Snap_mode;
  Compute_paintbrush_coordinates();
  Display_cursor();
}


void Button_Grid_menu(void)
{
  short clicked_button;
  word  chosen_X =Snap_width;
  word  chosen_Y =Snap_height;
  short dx_selected=Snap_offset_X;
  short dy_selected=Snap_offset_Y;

  char showgrid = Show_grid;
  // if grid is shown check if we snap
  // if not snap by default (so the window work like before we introduced the "show" option)
  char snapgrid = Show_grid?Snap_mode:1;

  T_Special_button * input_x_button;
  T_Special_button * input_y_button;
  T_Special_button * input_dx_button;
  T_Special_button * input_dy_button;

  char str[4];


  Open_window(149,118,"Grid");

  Window_set_normal_button(12,92,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(86,92,51,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Print_in_window(11,26, "X:",MC_Dark,MC_Light);
  input_x_button = Window_set_input_button(29,24,3); // 3
  Num2str(chosen_X,str,3);
  Window_input_content(input_x_button,str);

  Print_in_window(11,47, "Y:",MC_Dark,MC_Light);
  input_y_button = Window_set_input_button(29,45,3); // 4
  Num2str(chosen_Y,str,3);
  Window_input_content(input_y_button,str);

  Print_in_window(77,26,"dX:",MC_Dark,MC_Light);
  input_dx_button = Window_set_input_button(103,24,3); // 5
  Num2str(dx_selected,str,3);
  Window_input_content(input_dx_button,str);

  Print_in_window(77,47,"dY:",MC_Dark,MC_Light);
  input_dy_button = Window_set_input_button(103,45,3); // 6
  Num2str(dy_selected,str,3);

  Window_set_normal_button(12, 62, 14, 14, " ", 0, 1, 0);  // 7
  Window_set_normal_button(78, 62, 14, 14, " ", 0, 1, 0); // 8
  if (snapgrid)
    Print_in_window(16, 65, "X", MC_Black, MC_Light);
  if (Show_grid)
    Print_in_window(82, 65, "X", MC_Black, MC_Light);
  Print_in_window(32, 65,"Snap",MC_Dark,MC_Light);
  Print_in_window(98, 65,"Show",MC_Dark,MC_Light);

  Window_input_content(input_dy_button,str);
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case 3 :
        Num2str(chosen_X,str,3);
        Readline(31,26,str,3,INPUT_TYPE_INTEGER);
        chosen_X=atoi(str);
        // On corrige les dimensions
        if ((!chosen_X) || (chosen_X>999))
        {
          if (!chosen_X)
            chosen_X=1;
          else
            chosen_X=999;
          Num2str(chosen_X,str,3);
          Window_input_content(input_x_button,str);
        }
        if (dx_selected>=chosen_X)
        {
          dx_selected=chosen_X-1;
          Num2str(dx_selected,str,3);
          Window_input_content(input_dx_button,str);
        }
        Display_cursor();
        break;
      case 4 :
        Num2str(chosen_Y,str,3);
        Readline(31,47,str,3,INPUT_TYPE_INTEGER);
        chosen_Y=atoi(str);
        // On corrige les dimensions
        if ((!chosen_Y) || (chosen_Y>999))
        {
          if (!chosen_Y)
            chosen_Y=1;
          else
            chosen_Y=999;
          Num2str(chosen_Y,str,3);
          Window_input_content(input_y_button,str);
        }
        if (dy_selected>=chosen_Y)
        {
          dy_selected=chosen_Y-1;
          Num2str(dy_selected,str,3);
          Window_input_content(input_dy_button,str);
        }
        Display_cursor();
        break;
      case 5 :
        Num2str(dx_selected,str,3);
        Readline(105,26,str,3,INPUT_TYPE_INTEGER);
        dx_selected=atoi(str);
        // On corrige les dimensions
        if (dx_selected>=chosen_X)
          dx_selected=chosen_X-1;

        Num2str(dx_selected,str,3);
        Window_input_content(input_dx_button,str);

        Display_cursor();
        break;
      case 6 :
        Num2str(dy_selected,str,3);
        Readline(105,47,str,3,INPUT_TYPE_INTEGER);
        dy_selected=atoi(str);
        // On corrige les dimensions
        if (dy_selected>=chosen_Y)
          dy_selected=chosen_Y-1;

        Num2str(dy_selected,str,3);
        Window_input_content(input_dy_button,str);

        Display_cursor();

      case 7:
        snapgrid = !snapgrid;
        Hide_cursor();
        Print_in_window(16, 65, snapgrid?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;
      case 8:
        showgrid = !showgrid;
        Hide_cursor();
        Print_in_window(82, 65, showgrid?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;

    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "GRID");
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  if (clicked_button==2) // OK
  {
    byte modified;
    
    modified = Snap_width!=chosen_X
    || Snap_height!=chosen_Y
    || Snap_offset_X!=dx_selected
    || Snap_offset_Y!=dy_selected;
    
    Snap_width=chosen_X;
    Snap_height=chosen_Y;
    Snap_offset_X=dx_selected;
    Snap_offset_Y=dy_selected;
    Snap_mode=snapgrid;
    Show_grid=showgrid;
    
    if (modified)
    {
      Tilemap_update();
      Disable_spare_tilemap();
    }
  }

  Close_window();

  Display_cursor();
}

void Button_Show_grid(void)
{
  Show_grid = !Show_grid;
  Hide_cursor();
  Display_all_screen();
  Display_cursor();
} 


// -- Mode Smooth -----------------------------------------------------------
void Button_Smooth_mode(void)
{
  if (Smooth_mode)
    Effect_function=No_effect;
  else
  {
    Effect_function=Effect_smooth;
    Shade_mode=0;
    Quick_shade_mode=0;
    Colorize_mode=0;
    Tiling_mode=0;
    Smear_mode=0;
  }
  Smooth_mode=!Smooth_mode;
}


byte Smooth_default_matrices[4][3][3]=
{
 { {1,2,1}, {2,4,2}, {1,2,1} },
 { {1,3,1}, {3,9,3}, {1,3,1} },
 { {0,1,0}, {1,2,1}, {0,1,0} },
 { {2,3,2}, {3,1,3}, {2,3,2} }
};

void Button_Smooth_menu(void)
{
  short clicked_button;
  word x,y,i,j;
  byte  chosen_matrix[3][3];
  T_Special_button * matrix_input[3][3];
  char  str[3];

  Open_window(142,109,"Smooth");

  Window_set_normal_button(82,59,53,14,"Cancel",0,1,KEY_ESC); // 1
  Window_set_normal_button(82,88,53,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Window_display_frame(6,17,130,37);
  for (x=11,y=0; y<4; x+=31,y++)
  {
    Window_set_normal_button(x,22,27,27,"",0,1,SDLK_LAST);      // 3,4,5,6
    for (j=0; j<3; j++)
      for (i=0; i<3; i++)
        Print_char_in_window(x+2+(i<<3),24+(j<<3),'0'+Smooth_default_matrices[y][i][j],MC_Black,MC_Light);
  }

  Window_display_frame(6,58, 69,45);
  for (j=0; j<3; j++)
    for (i=0; i<3; i++)
    {
      matrix_input[i][j]=Window_set_input_button(10+(i*21),62+(j*13),2); // 7..15
      chosen_matrix[i][j] = Smooth_matrix[i][j] ;
      Num2str(chosen_matrix[i][j], str, 2);
      Window_input_content(matrix_input[i][j],str);
    }
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    if (clicked_button>2)
    {
      if (clicked_button<=6)
      {
        memcpy(chosen_matrix,Smooth_default_matrices[clicked_button-3],sizeof(chosen_matrix));
        Hide_cursor();
        for (j=0; j<3; j++)
          for (i=0; i<3; i++)
          {
            Num2str(chosen_matrix[i][j],str,2);
            Window_input_content(matrix_input[i][j],str);
          }
        Display_cursor();
      }
      else
      {
        i=clicked_button-7; x=i%3; y=i/3;
        Num2str(chosen_matrix[x][y],str,2);
        Readline(matrix_input[x][y]->Pos_X+2,
                 matrix_input[x][y]->Pos_Y+2,
                 str,2,INPUT_TYPE_INTEGER);
        chosen_matrix[x][y]=atoi(str);
        Display_cursor();
      }
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "SMOOTH");
    else if (Is_shortcut(Key,SPECIAL_SMOOTH_MENU))
      clicked_button=2;
  }
  while ((clicked_button!=1) && (clicked_button!=2));

  Close_window();

  if (clicked_button==2) // OK
  {
    memcpy(Smooth_matrix,chosen_matrix,sizeof(Smooth_matrix));
    Smooth_mode=0; // On le met à 0 car la fonct° suivante va le passer à 1
    Button_Smooth_mode();
  }

  Display_cursor();
}


// -- Mode Smear ------------------------------------------------------------
void Button_Smear_mode(void)
{
  if (!Smear_mode)
  {
    if (!Colorize_mode)
      Effect_function=No_effect;
    Shade_mode=0;
    Quick_shade_mode=0;
    Smooth_mode=0;
    Tiling_mode=0;
  }
  Smear_mode=!Smear_mode;
}

// -- Mode Colorize ---------------------------------------------------------
void Compute_colorize_table(void)
{
  word  index;
  word  factor_a;
  word  factor_b;

  factor_a=256*(100-Colorize_opacity)/100;
  factor_b=256*(    Colorize_opacity)/100;

  for (index=0;index<256;index++)
  {
    Factors_table[index]=index*factor_a;
    Factors_inv_table[index]=index*factor_b;
  }
}


void Button_Colorize_mode(void)
{
  if (Colorize_mode)
    Effect_function=No_effect;
  else
  {
    switch(Colorize_current_mode)
    {
      case 0 :
        Effect_function=Effect_interpolated_colorize;
        break;
      case 1 :
        Effect_function=Effect_additive_colorize;
        break;
      case 2 :
        Effect_function=Effect_substractive_colorize;
        break;
      case 3 :
        Effect_function=Effect_alpha_colorize;
    }
    Shade_mode=0;
    Quick_shade_mode=0;
    Smooth_mode=0;
    Tiling_mode=0;
  }
  Colorize_mode=!Colorize_mode;
}


void Button_Colorize_display_selection(int mode)
{
  short y_pos=0; // Ligne où afficher les flèches de sélection

  // On commence par effacer les anciennes sélections:
    // Partie gauche
  Print_in_window(4,37," ",MC_Black,MC_Light);
  Print_in_window(4,57," ",MC_Black,MC_Light);
  Print_in_window(4,74," ",MC_Black,MC_Light);
  Print_in_window(4,91," ",MC_Black,MC_Light);
    // Partie droite
  Print_in_window(129,37," ",MC_Black,MC_Light);
  Print_in_window(129,57," ",MC_Black,MC_Light);
  Print_in_window(129,74," ",MC_Black,MC_Light);
  Print_in_window(129,91," ",MC_Black,MC_Light);

  // Ensuite, on affiche la flèche là où il le faut:
  switch(mode)
  {
    case 0 : // Méthode interpolée
      y_pos=37;
      break;
    case 1 : // Méthode additive
      y_pos=57;
      break;
    case 2 : // Méthode soustractive
      y_pos=74;
      break;
    case 3 : // Méthode alpha
      y_pos=91;
  }
  Print_in_window(4,y_pos,"\020",MC_Black,MC_Light);
  Print_in_window(129,y_pos,"\021",MC_Black,MC_Light);
}

void Button_Colorize_menu(void)
{
  short chosen_opacity;
  short selected_mode;
  short clicked_button;
  char  str[4];

  Open_window(140,135,"Transparency");

  Print_in_window(16,23,"Opacity:",MC_Dark,MC_Light);
  Window_set_input_button(87,21,3);                               // 1
  Print_in_window(117,23,"%",MC_Dark,MC_Light);
  Window_set_normal_button(16,34,108,14,"Interpolate",1,1,SDLK_i); // 2
  Window_display_frame(12,18,116,34);

  Window_set_normal_button(16,54,108,14,"Additive"   ,2,1,SDLK_d); // 3
  Window_set_normal_button(16,71,108,14,"Subtractive",1,1,SDLK_s); // 4
  Window_set_normal_button(16,88,108,14,"Alpha",1,1,SDLK_a); // 4

  Window_set_normal_button(16,111, 51,14,"Cancel"     ,0,1,KEY_ESC); // 5
  Window_set_normal_button(73,111, 51,14,"OK"         ,0,1,SDLK_RETURN); // 6

  Num2str(Colorize_opacity,str,3);
  Window_input_content(Window_special_button_list,str);
  Button_Colorize_display_selection(Colorize_current_mode);

  chosen_opacity=Colorize_opacity;
  selected_mode    =Colorize_current_mode;

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch(clicked_button)
    {
      case 1: // Zone de saisie de l'opacité
        Num2str(chosen_opacity,str,3);
        Readline(89,23,str,3,INPUT_TYPE_INTEGER);
        chosen_opacity=atoi(str);
        // On corrige le pourcentage
        if (chosen_opacity>100)
        {
          chosen_opacity=100;
          Num2str(chosen_opacity,str,3);
          Window_input_content(Window_special_button_list,str);
        }
        Display_cursor();
        break;
      case 2: // Interpolated method
      case 3: // Additive method
      case 4: // Substractive method
      case 5: // Alpha method
        selected_mode=clicked_button-2;
        Hide_cursor();
        Button_Colorize_display_selection(selected_mode);
        Display_cursor();
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "TRANSPARENCY");
      else if (Is_shortcut(Key,SPECIAL_COLORIZE_MENU))
        clicked_button=7;
  }
  while (clicked_button<6);

  Close_window();

  if (clicked_button==7) // OK
  {
    Colorize_opacity      =chosen_opacity;
    Colorize_current_mode=selected_mode;
    Compute_colorize_table();
    Colorize_mode=0; // On le met à 0 car la fonct° suivante va le passer à 1
    Button_Colorize_mode();
  }

  Display_cursor();
}


// -- Mode Tiling -----------------------------------------------------------
void Button_Tiling_mode(void)
{
  if (Tiling_mode)
    Effect_function=No_effect;
  else
  {
    Effect_function=Effect_tiling;
    Shade_mode=0;
    Quick_shade_mode=0;
    Colorize_mode=0;
    Smooth_mode=0;
    Smear_mode=0;
  }
  Tiling_mode=!Tiling_mode;
}


void Button_Tiling_menu(void)
{
  short clicked_button;
  short chosen_offset_x=Tiling_offset_X;
  short chosen_offset_y=Tiling_offset_Y;
  char  str[5];
  T_Special_button * input_offset_x_button;
  T_Special_button * input_offset_y_button;

  Open_window(138,79,"Tiling");

  Window_set_normal_button(13,55,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(74,55,51,14,"OK"    ,0,1,SDLK_RETURN); // 2
  input_offset_x_button = Window_set_input_button(91,21,4);   // 3
  input_offset_y_button = Window_set_input_button(91,35,4);   // 4
  Print_in_window(12,23,"Offset X:",MC_Dark,MC_Light);
  Print_in_window(12,37,"Offset Y:",MC_Dark,MC_Light);

  Num2str(Tiling_offset_X,str,4);
  Window_input_content(input_offset_x_button,str);
  Num2str(Tiling_offset_Y,str,4);
  Window_input_content(input_offset_y_button,str);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    if (clicked_button==3)  // Zone de saisie du décalage X
    {
      Num2str(chosen_offset_x,str,4);
      Readline(93,23,str,4,INPUT_TYPE_INTEGER);
      chosen_offset_x=atoi(str);
      // On corrige le décalage en X
      if (chosen_offset_x>=Brush_width)
      {
        chosen_offset_x=Brush_width-1;
        Num2str(chosen_offset_x,str,4);
        Window_input_content(input_offset_x_button,str);
      }
      Display_cursor();
    }
    else
    if (clicked_button==4)  // Zone de saisie du décalage Y
    {
      Num2str(chosen_offset_y,str,4);
      Readline(93,37,str,4,INPUT_TYPE_INTEGER);
      chosen_offset_y=atoi(str);
      // On corrige le décalage en Y
      if (chosen_offset_y>=Brush_height)
      {
        chosen_offset_y=Brush_height-1;
        Num2str(chosen_offset_y,str,4);
        Window_input_content(input_offset_y_button,str);
      }
      Display_cursor();
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "TILING");
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  Close_window();

  if (clicked_button==2) // OK
  {
    Tiling_offset_X=chosen_offset_x;
    Tiling_offset_Y=chosen_offset_y;
    if (!Tiling_mode)
      Button_Tiling_mode();
  }

  Display_cursor();
}

// -- All modes off ---------------------------------------------------------
void Effects_off(void)
{
  Effect_function=No_effect;
  Shade_mode=0;
  Quick_shade_mode=0;
  Colorize_mode=0;
  Smooth_mode=0;
  Tiling_mode=0;
  Smear_mode=0;
  Stencil_mode=0;
  Mask_mode=0;
  Sieve_mode=0;
  Snap_mode=0;
  Main_tilemap_mode=0;
}


// -- Mode Sieve (Sieve) ----------------------------------------------------

void Button_Sieve_mode(void)
{
  Sieve_mode=!Sieve_mode;
}


void Draw_sieve_scaled(short origin_x, short origin_y)
{
  short x_pos;
  short y_pos;
  short x_size;
  short y_size;
  short start_x=Window_pos_X+(Menu_factor_X*230);
  short start_y=Window_pos_Y+(Menu_factor_Y*78);

  x_size=Menu_factor_X*5; // |_ Taille d'une case
  y_size=Menu_factor_Y*5; // |  de la trame zoomée

  // On efface de contenu précédent
  Block(origin_x,origin_y,
        Menu_factor_X*Window_special_button_list->Width,
        Menu_factor_Y*Window_special_button_list->Height,MC_Light);

  for (y_pos=0; y_pos<Sieve_height; y_pos++)
    for (x_pos=0; x_pos<Sieve_width; x_pos++)
    {
      // Bordures de la case
      Block(origin_x+(x_pos*x_size),
            origin_y+((y_pos+1)*y_size)-Menu_factor_Y,
            x_size, Menu_factor_Y,MC_Dark);
      Block(origin_x+((x_pos+1)*x_size)-Menu_factor_X,
            origin_y+(y_pos*y_size),
            Menu_factor_X, y_size-1,MC_Dark);
      // Contenu de la case
      Block(origin_x+(x_pos*x_size), origin_y+(y_pos*y_size),
            x_size-Menu_factor_X, y_size-Menu_factor_Y,
            (Sieve[x_pos][y_pos])?MC_White:MC_Black);
    }

  // Dessiner la preview de la trame
  x_size=Menu_factor_X*51; // |_ Taille de la fenêtre
  y_size=Menu_factor_Y*71; // |  de la preview
  for (y_pos=0; y_pos<y_size; y_pos++)
    for (x_pos=0; x_pos<x_size; x_pos++)
      Pixel(start_x+x_pos,start_y+y_pos,(Sieve[x_pos%Sieve_width][y_pos%Sieve_height])?MC_White:MC_Black);
  Update_rect(start_x,start_y,x_size,y_size);
}


void Draw_preset_sieve_patterns(void)
{
  short index;
  short i,j;
  //short x_size,y_size;
  short Zoom;
  
  Zoom=Min(Menu_factor_X,Menu_factor_Y);
  
  //x_size=1;//Menu_factor_X/Pixel_height;
  //y_size=1;//Menu_factor_Y/Pixel_width;

  for (index=0; index<12; index++)
    for (j=0; j<16*Menu_factor_Y/Zoom; j++)
      for (i=0; i<16*Menu_factor_X/Zoom; i++)
        Block(((index*23+10)*Menu_factor_X)+i*Zoom+Window_pos_X,
          (22*Menu_factor_Y)+j*Zoom+Window_pos_Y,Zoom,Zoom,
          ((Gfx->Sieve_pattern[index][j&0xF]>>(15-(i&0xF)))&1)?MC_White:MC_Black);

  Update_rect(ToWinX(10),ToWinY(22),ToWinL(12*23+16),ToWinH(16));
}


void Copy_preset_sieve(byte index)
{
  short i,j;

  for (j=0; j<16; j++)
    for (i=0; i<16; i++)
      Sieve[i][j]=(Gfx->Sieve_pattern[index][j]>>(15-i))&1;
  Sieve_width=16;
  Sieve_height=16;
}


void Invert_trame(void)
{
  byte x_pos,y_pos;

  for (y_pos=0; y_pos<Sieve_height; y_pos++)
    for (x_pos=0; x_pos<Sieve_width; x_pos++)
      Sieve[x_pos][y_pos]=!(Sieve[x_pos][y_pos]);
}

// Rafraichit toute la zone correspondant à la trame zoomee.
void Update_sieve_area(short x, short y)
{
  Update_rect(x,y,80*Menu_factor_X,80*Menu_factor_Y);
}


void Button_Sieve_menu(void)
{
  short clicked_button;
  short index;
  short x_pos;
  short y_pos;
  short old_x_pos=0;
  short old_y_pos=0;
  short origin_x;
  short origin_y;
  static byte default_bg_color=0;
  T_Normal_button * button_bg_color;
  char  str[3];
  byte  temp; // Octet temporaire servant à n'importe quoi
  short old_sieve_width=Sieve_width;
  short old_sieve_height=Sieve_height;
  byte  old_sieve[16][16];
  short preview_x_start; // |  Données précalculées
  short preview_y_start; // |_ pour la preview
  short preview_x_end;   // |  => plus grande
  short preview_y_end;   // |  rapidité.


  memcpy(old_sieve,Sieve,256);

  Open_window(290,179,"Sieve");

  preview_x_start=Window_pos_X+(Menu_factor_X*230);
  preview_y_start=Window_pos_Y+(Menu_factor_Y*78);
  preview_x_end=preview_x_start+(Menu_factor_X*51);
  preview_y_end=preview_y_start+(Menu_factor_Y*71);

  Window_display_frame      (  7, 65,130,43);
  Window_display_frame      (  7,110,130,43);
  Window_display_frame_in(142, 68, 82,82);
  Window_display_frame_in(229, 77, 53,73);

  Print_in_window(228, 68,"Preview",MC_Dark,MC_Light);
  Print_in_window( 27, 83,"Scroll" ,MC_Dark,MC_Light);
  Print_in_window( 23,120,"Width:" ,MC_Dark,MC_Light);
  Print_in_window( 15,136,"Height:",MC_Dark,MC_Light);

  Window_set_special_button(143,69,80,80);                     // 1

  Window_set_normal_button(175,157,51,14,"Cancel",0,1,KEY_ESC); // 2
  Window_set_normal_button(230,157,51,14,"OK"    ,0,1,SDLK_RETURN); // 3

  Window_set_normal_button(  8,157,51,14,"Clear" ,1,1,SDLK_c); // 4
  Window_set_normal_button( 63,157,51,14,"Invert",1,1,SDLK_i); // 5

  Window_set_normal_button(  8,46,131,14,"Get from brush"   ,1,1,SDLK_g); // 6
  Window_set_normal_button(142,46,139,14,"Transfer to brush",1,1,SDLK_t); // 7

  Window_set_normal_button(109,114,11,11,"\030",0,1,SDLK_UP|MOD_SHIFT); // 8
  Window_set_normal_button(109,138,11,11,"\031",0,1,SDLK_DOWN|MOD_SHIFT); // 9
  Window_set_normal_button( 97,126,11,11,"\033",0,1,SDLK_LEFT|MOD_SHIFT); // 10
  Window_set_normal_button(121,126,11,11,"\032",0,1,SDLK_RIGHT|MOD_SHIFT); // 11
  button_bg_color = Window_set_normal_button(109,126,11,11,""    ,0,1,SDLK_INSERT); // 12
  Block(Window_pos_X+(Menu_factor_X*(button_bg_color->Pos_X+2)),
        Window_pos_Y+(Menu_factor_Y*(button_bg_color->Pos_Y+2)),
        Menu_factor_X*7, Menu_factor_Y*7, (default_bg_color)?MC_White:MC_Black);

  Window_set_repeatable_button(109, 69,11,11,"\030",0,1,SDLK_UP); // 13
  Window_set_repeatable_button(109, 93,11,11,"\031",0,1,SDLK_DOWN); // 14
  Window_set_repeatable_button( 97, 81,11,11,"\033",0,1,SDLK_LEFT); // 15
  Window_set_repeatable_button(121, 81,11,11,"\032",0,1,SDLK_RIGHT); // 16

  for (index=0; index<12; index++)
    Window_set_normal_button((index*23)+8,20,20,20,"",0,1,SDLK_F1+index); // 17 -> 28
  Draw_preset_sieve_patterns();

  origin_x=Window_pos_X+(Menu_factor_X*Window_special_button_list->Pos_X);
  origin_y=Window_pos_Y+(Menu_factor_Y*Window_special_button_list->Pos_Y);

  Num2str(Sieve_width,str,2);
  Print_in_window(71,120,str,MC_Black,MC_Light);
  Num2str(Sieve_height,str,2);
  Print_in_window(71,136,str,MC_Black,MC_Light);
  Draw_sieve_scaled(origin_x,origin_y);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    origin_x=Window_pos_X+(Menu_factor_X*Window_special_button_list->Pos_X);
    origin_y=Window_pos_Y+(Menu_factor_Y*Window_special_button_list->Pos_Y);


    switch (clicked_button)
    {
      case -1 :
      case  0 :
        break;

      case  1 : // Zone de dessin de la trame
        /* // Version qui n'accepte pas les clicks sur la grille
        x_pos=(Mouse_X-origin_x)/Menu_factor_X;
        y_pos=(Mouse_Y-origin_y)/Menu_factor_Y;
        if ( (x_pos%5<4) && (y_pos%5<4) )
        {
          x_pos/=5;
          y_pos/=5;
          if ( (x_pos<Sieve_width) && (y_pos<Sieve_height) )
        }
        */
        x_pos=(Mouse_X-origin_x)/(Menu_factor_X*5);
        y_pos=(Mouse_Y-origin_y)/(Menu_factor_Y*5);
        if ( (x_pos<Sieve_width) && (y_pos<Sieve_height) )
        {
          temp=(Mouse_K==LEFT_SIDE);
          if ( (x_pos!=old_x_pos) || (y_pos!=old_y_pos)
            || (Sieve[x_pos][y_pos]!=temp) )
          {
            old_x_pos=x_pos;
            old_y_pos=y_pos;
            Sieve[x_pos][y_pos]=temp;
            x_pos=Menu_factor_X*5;
            y_pos=Menu_factor_Y*5;
            Hide_cursor();
            if (temp)
              temp=MC_White;
            else
              temp=MC_Black;
            // Affichage du pixel dans la fenêtre zoomée
            Block(origin_x+(old_x_pos*x_pos), origin_y+(old_y_pos*y_pos),
                  x_pos-Menu_factor_X, y_pos-Menu_factor_Y, temp);
            // Mise à jour de la preview
            Draw_sieve_scaled(origin_x,origin_y);
            Display_cursor();
            // Maj de la case seule
            Update_rect(origin_x+(old_x_pos*x_pos), origin_y+(old_y_pos*y_pos),Menu_factor_X*5,Menu_factor_Y*5);
          }
        }
        break;

      case  2 : // Cancel
      case  3 : // OK
        break;

      case  4 : // Clear
        Hide_cursor();
        memset(Sieve,default_bg_color,256);
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case  5 : // Invert
        Hide_cursor();
        Invert_trame();
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case  6 : // Get from brush
        Hide_cursor();
        Sieve_width=(Brush_width>16)?16:Brush_width;
        Sieve_height=(Brush_height>16)?16:Brush_height;
        for (y_pos=0; y_pos<Sieve_height; y_pos++)
          for (x_pos=0; x_pos<Sieve_width; x_pos++)
            Sieve[x_pos][y_pos]=(Read_pixel_from_brush(x_pos,y_pos)!=Back_color);
        Draw_sieve_scaled(origin_x,origin_y);
        Num2str(Sieve_height,str,2);
        Print_in_window(71,136,str,MC_Black,MC_Light);
        Num2str(Sieve_width,str,2);
        Print_in_window(71,120,str,MC_Black,MC_Light);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case  7 : // Transfer to brush
      
        if (Realloc_brush(Sieve_width, Sieve_height, NULL, NULL))
          break;
        
        for (y_pos=0; y_pos<Sieve_height; y_pos++)
          for (x_pos=0; x_pos<Sieve_width; x_pos++)
            *(Brush_original_pixels + y_pos * Brush_width + x_pos) = (Sieve[x_pos][y_pos])?Fore_color:Back_color;
        
        // Grab palette
        memcpy(Brush_original_palette, Main_palette,sizeof(T_Palette));
        // Remap (no change)
        Remap_brush();

        Brush_offset_X=(Brush_width>>1);
        Brush_offset_Y=(Brush_height>>1);
        
        Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
        break;

      case  8 : // Réduire hauteur
        if (Sieve_height>1)
        {
          Hide_cursor();
          Sieve_height--;
          Num2str(Sieve_height,str,2);
          Print_in_window(71,136,str,MC_Black,MC_Light);
          Draw_sieve_scaled(origin_x,origin_y);
          Display_cursor();
          Update_sieve_area(origin_x, origin_y);
        }
        break;

      case  9 : // Agrandir hauteur
        if (Sieve_height<16)
        {
          Hide_cursor();
          for (index=0; index<Sieve_width; index++)
            Sieve[index][Sieve_height]=default_bg_color;
          Sieve_height++;
          Num2str(Sieve_height,str,2);
          Print_in_window(71,136,str,MC_Black,MC_Light);
          Draw_sieve_scaled(origin_x,origin_y);
          Display_cursor();
          Update_sieve_area(origin_x, origin_y);
        }
        break;

      case 10 : // Réduire largeur
        if (Sieve_width>1)
        {
          Hide_cursor();
          Sieve_width--;
          Num2str(Sieve_width,str,2);
          Print_in_window(71,120,str,MC_Black,MC_Light);
          Draw_sieve_scaled(origin_x,origin_y);
          Display_cursor();
          Update_sieve_area(origin_x, origin_y);
        }
        break;

      case 11 : // Agrandir largeur
        if (Sieve_width<16)
        {
          Hide_cursor();
          for (index=0; index<Sieve_height; index++)
            Sieve[Sieve_width][index]=default_bg_color;
          Sieve_width++;
          Num2str(Sieve_width,str,2);
          Print_in_window(71,120,str,MC_Black,MC_Light);
          Draw_sieve_scaled(origin_x,origin_y);
          Display_cursor();
          Update_sieve_area(origin_x, origin_y);
        }
        break;

      case 12 : // Toggle octets insérés
        Hide_cursor();
        default_bg_color=!default_bg_color;
        Block(Window_pos_X+(Menu_factor_X*(button_bg_color->Pos_X+2)),
              Window_pos_Y+(Menu_factor_Y*(button_bg_color->Pos_Y+2)),
              Menu_factor_X*7, Menu_factor_Y*7, (default_bg_color)?MC_White:MC_Black);
        Display_cursor();
        Update_rect(
          Window_pos_X+(Menu_factor_X*(button_bg_color->Pos_X+2)),
          Window_pos_Y+(Menu_factor_Y*(button_bg_color->Pos_Y+2)),
          Menu_factor_X*7,
          Menu_factor_Y*7);

        break;

      case 13 : // Scroll vers le haut
        Hide_cursor();
        for (x_pos=0; x_pos<Sieve_width; x_pos++)
        {
          temp=Sieve[x_pos][0]; // Octet temporaire
          for (y_pos=1; y_pos<Sieve_height; y_pos++)
            Sieve[x_pos][y_pos-1]=Sieve[x_pos][y_pos];
          Sieve[x_pos][Sieve_height-1]=temp;
        }
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case 14 : // Scroll vers le bas
        Hide_cursor();
        for (x_pos=0; x_pos<Sieve_width; x_pos++)
        {
          temp=Sieve[x_pos][Sieve_height-1]; // Octet temporaire
          for (y_pos=Sieve_height-1; y_pos>0; y_pos--)
            Sieve[x_pos][y_pos]=Sieve[x_pos][y_pos-1];
          Sieve[x_pos][0]=temp;
        }
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case 15 : // Scroll vers la gauche
        Hide_cursor();
        for (y_pos=0; y_pos<Sieve_height; y_pos++)
        {
          temp=Sieve[0][y_pos]; // Octet temporaire
          for (x_pos=1; x_pos<Sieve_width; x_pos++)
            Sieve[x_pos-1][y_pos]=Sieve[x_pos][y_pos];
          Sieve[Sieve_width-1][y_pos]=temp;
        }
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      case 16 : // Scroll vers la droite
        Hide_cursor();
        for (y_pos=0; y_pos<Sieve_height; y_pos++)
        {
          temp=Sieve[Sieve_width-1][y_pos]; // Octet temporaire
          for (x_pos=Sieve_width-1; x_pos>0; x_pos--)
            Sieve[x_pos][y_pos]=Sieve[x_pos-1][y_pos];
          Sieve[0][y_pos]=temp;
        }
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
        break;

      default : // Boutons de trames prédéfinies
        Hide_cursor();
        Copy_preset_sieve(clicked_button-17);
        Draw_sieve_scaled(origin_x,origin_y);
        Num2str(Sieve_width,str,2);
        Print_in_window(71,120,str,MC_Black,MC_Light);
        Num2str(Sieve_height,str,2);
        Print_in_window(71,136,str,MC_Black,MC_Light);
        Draw_sieve_scaled(origin_x,origin_y);
        Display_cursor();
        Update_sieve_area(origin_x, origin_y);
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      Key=0;
      Window_help(BUTTON_EFFECTS, "SIEVE");
    }
  }
  while ( (clicked_button!=2) && (clicked_button!=3) );


  Close_window();

  if (clicked_button==2) // Cancel
  {
    Sieve_width=old_sieve_width;
    Sieve_height=old_sieve_height;
    memcpy(Sieve,old_sieve,256);
  }

  if ( (clicked_button==3) && (!Sieve_mode) ) // OK
    Button_Sieve_mode();

  Display_cursor();
}


