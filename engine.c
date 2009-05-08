/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
/// @file Window engine and interface management
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "misc.h"
#include "special.h"
#include "buttons.h"
#include "operatio.h"
#include "shade.h"
#include "errors.h"
#include "sdlscreen.h"
#include "windows.h"
#include "brush.h"
#include "input.h"
#include "engine.h"


// we need this as global
short Old_MX = -1;
short Old_MY = -1;

//---------- Annuler les effets des modes de dessin (sauf la grille) ---------

// Variables mémorisants les anciens effets

byte Shade_mode_before_cancel;
byte Quick_shade_mode_before_cancel;
byte Stencil_mode_before_cancel;
byte Sieve_mode_before_cancel;
byte Colorize_mode_before_cancel;
byte Smooth_mode_before_cancel;
byte Tiling_mode_before_cancel;
Func_effect Effect_function_before_cancel;

///This table holds pointers to the saved window backgrounds. We can have up to 8 windows open at a time.
byte* Window_background[8];

///Disables all the effects
void Cancel_effects(void)
{
  Shade_mode_before_cancel=Shade_mode;
  Shade_mode=0;

  Quick_shade_mode_before_cancel=Quick_shade_mode;
  Quick_shade_mode=0;

  Stencil_mode_before_cancel=Stencil_mode;
  Stencil_mode=0;

  Sieve_mode_before_cancel=Sieve_mode;
  Sieve_mode=0;

  Colorize_mode_before_cancel=Colorize_mode;
  Colorize_mode=0;

  Smooth_mode_before_cancel=Smooth_mode;
  Smooth_mode=0;

  Tiling_mode_before_cancel=Tiling_mode;
  Tiling_mode=0;

  Effect_function_before_cancel=Effect_function;
  Effect_function=No_effect;
}


///Get the effects and drawing modes back
void Restore_effects(void)
{
  Shade_mode      =Shade_mode_before_cancel;
  Quick_shade_mode=Quick_shade_mode_before_cancel;
  Stencil_mode    =Stencil_mode_before_cancel;
  Sieve_mode      =Sieve_mode_before_cancel;
  Colorize_mode   =Colorize_mode_before_cancel;
  Smooth_mode     =Smooth_mode_before_cancel;
  Tiling_mode     =Tiling_mode_before_cancel;
  Effect_function  =Effect_function_before_cancel;
}


///Table of tooltip texts for menu buttons
char * Menu_tooltip[NB_BUTTONS]=
{
  "Paintbrush choice       ",
  "Adjust / Transform menu ",
  "Freehand draw. / Toggle ",
  "Splines / Toggle        ",
  "Lines / Toggle          ",
  "Spray / Menu            ",
  "Floodfill / Replace col.",
  "Polylines / Polyforms   ",
  "Polyfill / Filled Pforms",
  "Empty rectangles        ",
  "Filled rectangles       ",
  "Empty circles / ellipses",
  "Filled circles / ellips.",
  "Grad. rectangles        ",
  "Gradation menu          ",
  "Grad. spheres / ellipses",
  "Brush grab. / Restore   ",
  "Lasso / Restore brush   ",
  "Brush effects           ",
  "Drawing modes (effects) ",
  "Text                    ",
  "Magnify mode / Menu     ",
  "Pipette / Invert colors ",
  "Screen size / Safe. res.",
  "Go / Copy to other page ",
  "Save as / Save          ",
  "Load / Re-load          ",
  "Settings                ",
  "Clear / with backcolor  ",
  "Help / Statistics       ",
  "Undo / Redo             ",
  "Kill current page       ",
  "Quit                    ",
  "Palette editor          ",
  "Scroll pal. bkwd / Fast ",
  "Scroll pal. fwd / Fast  ",
  "Color #"                 ,
  "Hide tool bar           "
};

///Save a screen block (usually before erasing it with a new window or a dropdown menu)
void Save_background(byte **buffer, int x_pos, int y_pos, int width, int height)
{
  int index;
  if(*buffer != NULL) DEBUG("WARNING : buffer already allocated !!!",0);
  *buffer=(byte *) malloc(width*Menu_factor_X*height*Menu_factor_Y*Pixel_width);
  for (index=0; index<(height*Menu_factor_Y); index++)
    Read_line(x_pos,y_pos+index,width*Menu_factor_X,(*buffer)+((int)index*width*Menu_factor_X*Pixel_width));
}

///Restores a screen block
void Restore_background(byte *buffer, int x_pos, int y_pos, int width, int height)
{
  int index;
  for (index=0; index<height*Menu_factor_Y; index++)
    Display_line_fast(x_pos,y_pos+index,width*Menu_factor_X,buffer+((int)index*width*Menu_factor_X*Pixel_width));
  free(buffer);
}

///Draw a pixel in a saved screen block (when you sort colors in the palette, for example)
void Pixel_background(int x_pos, int y_pos, byte color)
{
  int x_repetition=Pixel_width;
  while (x_repetition--)
    (Window_background[0][x_pos*Pixel_width+x_repetition+y_pos*Window_width*Pixel_width*Menu_factor_X])=color;
}


///Guess the number of the button that was just clicked
int Button_under_mouse(void)
{
  int btn_number;
  short x_pos;
  short y_pos;

  x_pos=(Mouse_X              )/Menu_factor_X;
  y_pos=(Mouse_Y-Menu_Y)/Menu_factor_Y;

  for (btn_number=0;btn_number<NB_BUTTONS;btn_number++)
  {
    switch(Buttons_Pool[btn_number].Shape)
    {
      case BUTTON_SHAPE_NO_FRAME :
      case BUTTON_SHAPE_RECTANGLE  :

        if ((x_pos>=Buttons_Pool[btn_number].X_offset) &&
            (y_pos>=Buttons_Pool[btn_number].Y_offset) &&
            (x_pos<=Buttons_Pool[btn_number].X_offset+Buttons_Pool[btn_number].Width) &&
            (y_pos<=Buttons_Pool[btn_number].Y_offset+Buttons_Pool[btn_number].Height))
          return btn_number;
        break;

      case BUTTON_SHAPE_TRIANGLE_TOP_LEFT:
        if ((x_pos>=Buttons_Pool[btn_number].X_offset) &&
            (y_pos>=Buttons_Pool[btn_number].Y_offset) &&
            (x_pos+y_pos-(short)Buttons_Pool[btn_number].Y_offset-(short)Buttons_Pool[btn_number].X_offset<=Buttons_Pool[btn_number].Width))
          return btn_number;
        break;

      case BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT:
        if ((x_pos<=Buttons_Pool[btn_number].X_offset+Buttons_Pool[btn_number].Width) &&
            (y_pos<=Buttons_Pool[btn_number].Y_offset+Buttons_Pool[btn_number].Height) &&
            (x_pos+y_pos-(short)Buttons_Pool[btn_number].Y_offset-(short)Buttons_Pool[btn_number].X_offset>=Buttons_Pool[btn_number].Width))
          return btn_number;
        break;
    }
  }
  return -1;
}


///Draw the frame for a menu button
void Draw_menu_button_frame(byte btn_number,byte pressed)
{
  byte color_top_left;
  byte color_bottom_right;
  byte color_diagonal;
  word start_x;
  word start_y;
  word end_x;
  word end_y;
  word x_pos;
  word y_pos;

  start_x=Buttons_Pool[btn_number].X_offset;
  start_y=Buttons_Pool[btn_number].Y_offset;
  end_x  =start_x+Buttons_Pool[btn_number].Width;
  end_y  =start_y+Buttons_Pool[btn_number].Height;

  if (!pressed)
  {
    color_top_left=MC_White;
    color_bottom_right=MC_Dark;
    color_diagonal=MC_Light;
  }
  else
  {
    color_top_left=MC_Dark;
    color_bottom_right=MC_Black;
    color_diagonal=MC_Dark;
  }

  switch(Buttons_Pool[btn_number].Shape)
  {
    case BUTTON_SHAPE_NO_FRAME :
      break;
    case BUTTON_SHAPE_RECTANGLE  :
      // On colorie le point haut droit
      Pixel_in_menu(end_x,start_y,color_diagonal);
      GFX_menu_block[start_y][end_x]=color_diagonal;
      // On colorie le point bas gauche
      Pixel_in_menu(start_x,end_y,color_diagonal);
      GFX_menu_block[end_y][start_x]=color_diagonal;
      // On colorie la partie haute
      for (x_pos=start_x;x_pos<=end_x-1;x_pos++)
      {
        Pixel_in_menu(x_pos,start_y,color_top_left);
        GFX_menu_block[start_y][x_pos]=color_top_left;
      }
      for (y_pos=start_y+1;y_pos<=end_y-1;y_pos++)
      {
        // On colorie la partie gauche
        Pixel_in_menu(start_x,y_pos,color_top_left);
        GFX_menu_block[y_pos][start_x]=color_top_left;
        // On colorie la partie droite
        Pixel_in_menu(end_x,y_pos,color_bottom_right);
        GFX_menu_block[y_pos][end_x]=color_bottom_right;
      }
      // On colorie la partie basse
      for (x_pos=start_x+1;x_pos<=end_x;x_pos++)
      {
        Pixel_in_menu(x_pos,end_y,color_bottom_right);
        GFX_menu_block[end_y][x_pos]=color_bottom_right;
      }
      break;
    case BUTTON_SHAPE_TRIANGLE_TOP_LEFT:
      // On colorie le point haut droit
      Pixel_in_menu(end_x,start_y,color_diagonal);
      GFX_menu_block[start_y][end_x]=color_diagonal;
      // On colorie le point bas gauche
      Pixel_in_menu(start_x,end_y,color_diagonal);
      GFX_menu_block[end_y][start_x]=color_diagonal;
      // On colorie le coin haut gauche
      for (x_pos=0;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu(start_x+x_pos,start_y,color_top_left);
        GFX_menu_block[start_y][start_x+x_pos]=color_top_left;
        Pixel_in_menu(start_x,start_y+x_pos,color_top_left);
        GFX_menu_block[start_y+x_pos][start_x]=color_top_left;
      }
      // On colorie la diagonale
      for (x_pos=1;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu(start_x+x_pos,end_y-x_pos,color_bottom_right);
        GFX_menu_block[end_y-x_pos][start_x+x_pos]=color_bottom_right;
      }
      break;
    case BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT:
      // On colorie le point haut droit
      Pixel_in_menu(end_x,start_y,color_diagonal);
      GFX_menu_block[start_y][end_x]=color_diagonal;
      // On colorie le point bas gauche
      Pixel_in_menu(start_x,end_y,color_diagonal);
      GFX_menu_block[end_y][start_x]=color_diagonal;
      // On colorie la diagonale
      for (x_pos=1;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu(start_x+x_pos,end_y-x_pos,color_top_left);
        GFX_menu_block[end_y-x_pos][start_x+x_pos]=color_top_left;
      }
      // On colorie le coin bas droite
      for (x_pos=0;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu(end_x-x_pos,end_y,color_bottom_right);
        GFX_menu_block[end_y][end_x-x_pos]=color_bottom_right;
        Pixel_in_menu(end_x,end_y-x_pos,color_bottom_right);
        GFX_menu_block[end_y-x_pos][end_x]=color_bottom_right;
      }
  }
  if (Menu_is_visible)
  {
    Update_rect(
      start_x*Menu_factor_X,
      start_y*Menu_factor_Y + Menu_Y,
      (end_x+1-start_x)*Menu_factor_X,
      (end_y+1-start_y)*Menu_factor_Y);
  }
}


///Deselect a button
void Unselect_button(int btn_number)
{
  if (Buttons_Pool[btn_number].Pressed)
  {
    // On affiche le cadre autour du bouton de façon à ce qu'il paraisse relâché
    Draw_menu_button_frame(btn_number,BUTTON_RELEASED);
    // On considère que le bouton est relâché
    Buttons_Pool[btn_number].Pressed=BUTTON_RELEASED;
    // On appelle le désenclenchement particulier au bouton:
    Buttons_Pool[btn_number].Unselect_action();
  }
}


///Select a button and disable all his family (for example, selecting "freehand" unselect "curves", "lines", ...)
void Select_button(int btn_number,byte click)
{
  int family;
  int b;
  int icon;

  Hide_cursor();

  // Certains boutons ont deux icones
  icon=-1;
  switch(btn_number)
  {
    case BUTTON_POLYGONS:
    case BUTTON_POLYFILL:
      icon=12;break;
    case BUTTON_FLOODFILL:
      icon=14;break;
    case BUTTON_CIRCLES:
    case BUTTON_FILLCIRC:
      icon=10;break;
    case BUTTON_SPHERES:
      icon=16;break;
  }
  if (icon!=-1)
    Display_sprite_in_menu(btn_number,icon+(click==RIGHT_SIDE));

  // On note déjà la famille du bouton (La "Famiglia" c'est sacré)
  family=Buttons_Pool[btn_number].Family;

  switch (family)
  {
    case FAMILY_TOOLBAR: // On ne fait rien (on préserve les interruptions)
      break;

    case FAMILY_INTERRUPTION: // Petit cas spécial dans la famille "Interruption":
      if ((btn_number!=BUTTON_MAGNIFIER) || (!Main_magnifier_mode))
      // Pour chaque bouton:
      for (b=0; b<NB_BUTTONS; b++)
        // S'il est de la même famille
        if (
             (b!=btn_number) &&
             (Buttons_Pool[b].Family==FAMILY_INTERRUPTION) &&
             (  (b!=BUTTON_MAGNIFIER) ||
               ((b==BUTTON_MAGNIFIER) && (!Main_magnifier_mode)) )
           )
          // Alors on désenclenche le bouton
          Unselect_button(b);
      break;

    default:
      // On désenclenche D'ABORD les interruptions
      // Pour chaque bouton:
      for (b=0; b<NB_BUTTONS; b++)
        // S'il est de la famille interruption
        if ( (b!=btn_number)
          && (Buttons_Pool[b].Family==FAMILY_INTERRUPTION)
          // Et que ce n'est pas la loupe, ou alors qu'on n'est pas en mode loupe
          && (!(Main_magnifier_mode && (b==BUTTON_MAGNIFIER))) )
          // Alors on désenclenche le bouton
          Unselect_button(b);
      // Right-clicking on Adjust opens a menu, so in this case we skip
      // the unselection of all "Tool" buttons.
      if (btn_number==BUTTON_ADJUST && click==RIGHT_SIDE)
        break;
      // Pour chaque bouton:
      for (b=0; b<NB_BUTTONS; b++)
        // S'il est de la même famille
        if ( (b!=btn_number)
          && (Buttons_Pool[b].Family==family) )
          // Alors on désenclenche le bouton
          Unselect_button(b);
  }

  // On affiche le cadre autour du bouton de façon à ce qu'il paraisse enfoncé
  Draw_menu_button_frame(btn_number,BUTTON_PRESSED);

  Display_cursor();

  // On attend ensuite que l'utilisateur lâche son bouton:
  Wait_end_of_click();

  // On considère que le bouton est enfoncé
  Buttons_Pool[btn_number].Pressed=BUTTON_PRESSED;

  // Puis on se contente d'appeler l'action correspondant au bouton:
  if (click==1)
    Buttons_Pool[btn_number].Left_action();
  else
    Buttons_Pool[btn_number].Right_action();
}


///Moves the splitbar between zoom and standard views
void Move_separator(void)
{
  short old_main_separator_position=Main_separator_position;
  short old_x_zoom=Main_X_zoom;
  short offset=Main_X_zoom-Mouse_X;
  byte  old_cursor_shape=Cursor_shape;
  short old_mouse_x=-1;

  // Afficher la barre en XOR
  Hide_cursor();
  Windows_open=1;
  Cursor_shape=CURSOR_SHAPE_HORIZONTAL;
  Vertical_XOR_line(Main_separator_position,0,Menu_Y);
  Vertical_XOR_line(Main_X_zoom-1,0,Menu_Y);
  Display_cursor();
  Update_rect(Main_separator_position,0,abs(Main_separator_position-Main_X_zoom)+1,Menu_Y);

  while (Mouse_K)
  {
    if (Mouse_X!=old_mouse_x)
    {
      old_mouse_x=Mouse_X;
      Main_separator_proportion=(float)(Mouse_X+offset)/Screen_width;
      Compute_separator_data();
  
      if (Main_X_zoom!=old_x_zoom)
      {
        Hide_cursor();
  
        // Effacer la barre en XOR
        Vertical_XOR_line(old_main_separator_position,0,Menu_Y);
        Vertical_XOR_line(old_x_zoom-1,0,Menu_Y);
  
        Update_rect(old_main_separator_position,0,abs(old_main_separator_position-old_x_zoom)+1,Menu_Y);
  
        old_main_separator_position=Main_separator_position;
        old_x_zoom=Main_X_zoom;
  
        // Rafficher la barre en XOR
        Vertical_XOR_line(Main_separator_position,0,Menu_Y);
        Vertical_XOR_line(Main_X_zoom-1,0,Menu_Y);
  
        Update_rect(Main_separator_position,0,abs(Main_separator_position-Main_X_zoom)+1,Menu_Y);
  
        Display_cursor();
      }
    }
    if(!Get_input())Wait_VBL();
  }

  // Effacer la barre en XOR
  Hide_cursor();
  Vertical_XOR_line(Main_separator_position,0,Menu_Y);
  Vertical_XOR_line(Main_X_zoom-1,0,Menu_Y);
  Windows_open=0;
  Cursor_shape=old_cursor_shape;
  Compute_magnifier_data();
  Position_screen_according_to_zoom();
  Compute_limits();
  Display_all_screen();
  Display_cursor();
}


///Main handler for everything. This is the main loop of the program
void Main_handler(void)
{
  static byte temp_color;
  int  button_index;           // Numéro de bouton de menu en cours
  int  prev_button_number=0; // Numéro de bouton de menu sur lequel on était précédemment
  byte blink;                   // L'opération demande un effacement du curseur
  int  shortcut_button;           // Button à enclencher d'après la touche de raccourci enfoncée
  byte clicked_button = 0;         // Côté du bouton à enclencher d'après la touche de raccourci enfoncée
  int  key_index;           // index du tableau de touches spéciales correspondant à la touche enfoncée
  char str[25];
  byte temp;

  do
  {
    if(Get_input())
    {
    // Redimensionnement demandé
    if (Resize_width || Resize_height)
    {
      Hide_cursor();
      Init_mode_video(Resize_width, Resize_height, 0, Pixel_ratio);
      // Remise à zero des variables indiquant le Redimensionnement demandé
      Display_menu();
      Reposition_palette();
      Display_all_screen();
      Display_cursor();
    }
    
    // Evenement de fermeture
    if (Quit_is_required)
    {
      Quit_is_required=0;
      Button_Quit();
    }
    
    // Gestion des touches
    if (Key)
    {
      for (key_index=0;(key_index<NB_SPECIAL_SHORTCUTS) && !Is_shortcut(Key,key_index);key_index++);

      // Gestion des touches spéciales:
      if (key_index>SPECIAL_CLICK_RIGHT)
      switch(key_index)
      {
        case SPECIAL_SCROLL_UP : // Scroll up
          if (Main_magnifier_mode)
            Scroll_magnifier(0,-(Main_magnifier_height>>2));
          else
            Scroll_screen(0,-(Screen_height>>3));
          Key=0;
          break;
        case SPECIAL_SCROLL_DOWN : // Scroll down
          if (Main_magnifier_mode)
            Scroll_magnifier(0,(Main_magnifier_height>>2));
          else
            Scroll_screen(0,(Screen_height>>3));
          Key=0;
          break;
        case SPECIAL_SCROLL_LEFT : // Scroll left
          if (Main_magnifier_mode)
            Scroll_magnifier(-(Main_magnifier_width>>2),0);
          else
            Scroll_screen(-(Screen_width>>3),0);
          Key=0;
          break;
        case SPECIAL_SCROLL_RIGHT : // Scroll right
          if (Main_magnifier_mode)
            Scroll_magnifier((Main_magnifier_width>>2),0);
          else
            Scroll_screen((Screen_width>>3),0);
          Key=0;
          break;
        case SPECIAL_SCROLL_UP_FAST : // Scroll up faster
          if (Main_magnifier_mode)
            Scroll_magnifier(0,-(Main_magnifier_height>>1));
          else
            Scroll_screen(0,-(Screen_height>>2));
          Key=0;
          break;
        case SPECIAL_SCROLL_DOWN_FAST : // Scroll down faster
          if (Main_magnifier_mode)
            Scroll_magnifier(0,(Main_magnifier_height>>1));
          else
            Scroll_screen(0,(Screen_height>>2));
          Key=0;
          break;
        case SPECIAL_SCROLL_LEFT_FAST : // Scroll left faster
          if (Main_magnifier_mode)
            Scroll_magnifier(-(Main_magnifier_width>>1),0);
          else
            Scroll_screen(-(Screen_width>>2),0);
          Key=0;
          break;
        case SPECIAL_SCROLL_RIGHT_FAST : // Scroll right faster
          if (Main_magnifier_mode)
            Scroll_magnifier((Main_magnifier_width>>1),0);
          else
            Scroll_screen((Screen_width>>2),0);
          Key=0;
          break;
        case SPECIAL_SCROLL_UP_SLOW : // Scroll up slower
          if (Main_magnifier_mode)
            Scroll_magnifier(0,-1);
          else
            Scroll_screen(0,-1);
          Key=0;
          break;
        case SPECIAL_SCROLL_DOWN_SLOW : // Scroll down slower
          if (Main_magnifier_mode)
            Scroll_magnifier(0,1);
          else
            Scroll_screen(0,1);
          Key=0;
          break;
        case SPECIAL_SCROLL_LEFT_SLOW : // Scroll left slower
          if (Main_magnifier_mode)
            Scroll_magnifier(-1,0);
          else
            Scroll_screen(-1,0);
          Key=0;
          break;
        case SPECIAL_SCROLL_RIGHT_SLOW : // Scroll right slower
          if (Main_magnifier_mode)
            Scroll_magnifier(1,0);
          else
            Scroll_screen(1,0);
          Key=0;
          break;
        case SPECIAL_NEXT_FORECOLOR : // Next foreground color
          Special_next_forecolor();
          Key=0;
          break;
        case SPECIAL_PREVIOUS_FORECOLOR : // Previous foreground color
          Special_previous_forecolor();
          Key=0;
          break;
        case SPECIAL_NEXT_BACKCOLOR : // Next background color
          Special_next_backcolor();
          Key=0;
          break;
        case SPECIAL_PREVIOUS_BACKCOLOR : // Previous background color
          Special_previous_backcolor();
          Key=0;
          break;
        case SPECIAL_SMALLER_PAINTBRUSH: // Rétrécir le pinceau
          Smaller_paintbrush();
          Key=0;
          break;
        case SPECIAL_BIGGER_PAINTBRUSH: // Grossir le pinceau
          Bigger_paintbrush();
          Key=0;
          break;
        case SPECIAL_NEXT_USER_FORECOLOR : // Next user-defined foreground color
          Special_next_user_forecolor();
          Key=0;
          break;
        case SPECIAL_PREVIOUS_USER_FORECOLOR : // Previous user-defined foreground color
          Special_previous_user_forecolor();
          Key=0;
          break;
        case SPECIAL_NEXT_USER_BACKCOLOR : // Next user-defined background color
          Special_next_user_backcolor();
          Key=0;
          break;
        case SPECIAL_PREVIOUS_USER_BACKCOLOR : // Previous user-defined background color
          Special_previous_user_backcolor();
          Key=0;
          break;
        case SPECIAL_SHOW_HIDE_CURSOR : // Show / Hide cursor
          Hide_cursor();
          Cursor_hidden=!Cursor_hidden;
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_DOT_PAINTBRUSH : // Paintbrush = "."
          Hide_cursor();
          Paintbrush_shape=PAINTBRUSH_SHAPE_ROUND;
          Set_paintbrush_size(1,1);
          Change_paintbrush_shape(PAINTBRUSH_SHAPE_ROUND);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_CONTINUOUS_DRAW : // Continuous freehand drawing
          Select_button(BUTTON_DRAW,LEFT_SIDE);
          // ATTENTION CE TRUC EST MOCHE ET VA MERDER SI ON SE MET A UTILISER DES BOUTONS POPUPS
          while (Current_operation!=OPERATION_CONTINUOUS_DRAW)
            Select_button(BUTTON_DRAW,RIGHT_SIDE);
          Key=0;
          break;
        case SPECIAL_FLIP_X : // Flip X
          Hide_cursor();
          Flip_X_lowlevel(Brush, Brush_width, Brush_height);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_FLIP_Y : // Flip Y
          Hide_cursor();
          Flip_Y_lowlevel(Brush, Brush_width, Brush_height);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_ROTATE_90 : // 90° brush rotation
          Hide_cursor();
          Rotate_90_deg();
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_ROTATE_180 : // 180° brush rotation
          Hide_cursor();
          Rotate_180_deg_lowlevel(Brush, Brush_width, Brush_height);
          Brush_offset_X=(Brush_width>>1);
          Brush_offset_Y=(Brush_height>>1);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_STRETCH : // Stretch brush
          Hide_cursor();
          Start_operation_stack(OPERATION_STRETCH_BRUSH);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_DISTORT : // Distort brush
          Hide_cursor();
          Start_operation_stack(OPERATION_DISTORT_BRUSH);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_ROTATE_ANY_ANGLE : // Rotate brush by any angle
          Hide_cursor();
          Start_operation_stack(OPERATION_ROTATE_BRUSH);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_OUTLINE : // Outline brush
          Hide_cursor();
          Outline_brush();
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_NIBBLE : // Nibble brush
          Hide_cursor();
          Nibble_brush();
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_GET_BRUSH_COLORS : // Get colors from brush
          Get_colors_from_brush();
          Key=0;
          break;
        case SPECIAL_RECOLORIZE_BRUSH : // Recolorize brush
          Hide_cursor();
          Remap_brush();
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_LOAD_BRUSH :
          Load_picture(0);
          Key=0;
          break;
        case SPECIAL_SAVE_BRUSH :
          Save_picture(0);
          Key=0;
          break;
        case SPECIAL_ZOOM_IN : // Zoom in
          Zoom(+1);
          Key=0;
          break;
        case SPECIAL_ZOOM_OUT : // Zoom out
          Zoom(-1);
          Key=0;
          break;
        case SPECIAL_CENTER_ATTACHMENT : // Center brush attachment
          Hide_cursor();
          Brush_offset_X=(Brush_width>>1);
          Brush_offset_Y=(Brush_height>>1);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_TOP_LEFT_ATTACHMENT : // Top-left brush attachment
          Hide_cursor();
          Brush_offset_X=0;
          Brush_offset_Y=0;
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_TOP_RIGHT_ATTACHMENT : // Top-right brush attachment
          Hide_cursor();
          Brush_offset_X=(Brush_width-1);
          Brush_offset_Y=0;
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_BOTTOM_LEFT_ATTACHMENT : // Bottom-left brush attachment
          Hide_cursor();
          Brush_offset_X=0;
          Brush_offset_Y=(Brush_height-1);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_BOTTOM_RIGHT_ATTACHMENT : // Bottom right brush attachment
          Hide_cursor();
          Brush_offset_X=(Brush_width-1);
          Brush_offset_Y=(Brush_height-1);
          Display_cursor();
          Key=0;
          break;
        case SPECIAL_EXCLUDE_COLORS_MENU : // Exclude colors menu
          Menu_tag_colors("Tag colors to exclude",Exclude_color,&temp,1, NULL);
          Key=0;
          break;
        case SPECIAL_INVERT_SIEVE :
          Invert_trame();
          Key=0;
          break;
        case SPECIAL_SHADE_MODE :
          Button_Shade_mode();
          Key=0;
          break;
        case SPECIAL_SHADE_MENU :
          Button_Shade_menu();
          Key=0;
          break;
        case SPECIAL_QUICK_SHADE_MODE :
          Button_Quick_shade_mode();
          Key=0;
          break;
        case SPECIAL_QUICK_SHADE_MENU :
          Button_Quick_shade_menu();
          Key=0;
          break;
        case SPECIAL_STENCIL_MODE :
          Button_Stencil_mode();
          Key=0;
          break;
        case SPECIAL_STENCIL_MENU :
          Button_Stencil_menu();
          Key=0;
          break;
        case SPECIAL_MASK_MODE :
          Button_Mask_mode();
          Key=0;
          break;
        case SPECIAL_MASK_MENU :
          Button_Mask_menu();
          Key=0;
          break;
        case SPECIAL_GRID_MODE :
          Button_Snap_mode();
          Key=0;
          break;
        case SPECIAL_GRID_MENU :
          Button_Grid_menu();
          Key=0;
          break;
        case SPECIAL_SIEVE_MODE :
          Button_Sieve_mode();
          Key=0;
          break;
        case SPECIAL_SIEVE_MENU :
          Button_Sieve_menu();
          Key=0;
          break;
        case SPECIAL_COLORIZE_MODE :
          Button_Colorize_mode();
          Key=0;
          break;
        case SPECIAL_COLORIZE_MENU :
          Button_Colorize_menu();
          Key=0;
          break;
        case SPECIAL_SMOOTH_MODE :
          Button_Smooth_mode();
          Key=0;
          break;
        case SPECIAL_SMOOTH_MENU :
          Button_Smooth_menu();
          Key=0;
          break;
        case SPECIAL_SMEAR_MODE :
          Button_Smear_mode();
          Key=0;
          break;
        case SPECIAL_TILING_MODE :
          Button_Tiling_mode();
          Key=0;
          break;
        case SPECIAL_TILING_MENU :
          Button_Tiling_menu();
          Key=0;
          break;
        default   : // Gestion des touches de raccourci de bouton:
          // Pour chaque bouton
          shortcut_button=-1;
          for (button_index=0;button_index<NB_BUTTONS;button_index++)
          {
            if (Is_shortcut(Key,0x100+button_index))
            {
              shortcut_button=button_index;
              clicked_button  =LEFT_SIDE;
              button_index=NB_BUTTONS;
            }
            else if (Is_shortcut(Key,0x200+button_index))
            {
              shortcut_button=button_index;
              clicked_button  =RIGHT_SIDE;
              button_index=NB_BUTTONS;
            }
          }
          // Après avoir scruté les boutons, si la recherche a été fructueuse,
          // on lance le bouton.
          if (shortcut_button!=-1)
          {
            Select_button(shortcut_button,clicked_button);
            prev_button_number=-1;
          }
      }

      // Si on a modifié un effet, il faut rafficher le bouton des effets en
      // conséquence.
      if ((key_index>=SPECIAL_SHADE_MODE)
       && (key_index<=SPECIAL_TILING_MENU))
      {
        Hide_cursor();
        Draw_menu_button_frame(BUTTON_EFFECTS,
          (Shade_mode||Quick_shade_mode||Colorize_mode||Smooth_mode||Tiling_mode||Smear_mode||Stencil_mode||Mask_mode||Sieve_mode||Snap_mode));
        Display_cursor();
      }
    }
    }
    else Wait_VBL(); // S'il n'y a pas d'évènement, on ne gère pas tout ça et on attend un peu. La partie en dessous doit être exécutée quand
                     // même pour les trucs asynchrones, par exemple le spray.

    // Gestion de la souris

    Cursor_in_menu=(Mouse_Y>=Menu_Y) ||
                      ( (Main_magnifier_mode) && (Mouse_X>=Main_separator_position) &&
                        (Mouse_X<Main_X_zoom) );

    if (Cursor_in_menu)
    {
      // Le curseur se trouve dans le menu

      // On cherche sur quel bouton du menu se trouve la souris:
      button_index=Button_under_mouse();

      // Si le curseur vient de changer de zone

      if ( (button_index!=prev_button_number)
        || (!Cursor_in_menu_previous)
        || (prev_button_number==BUTTON_CHOOSE_COL) )
      {
        // Si le curseur n'est pas sur un bouton
        if (button_index==-1)
        {
          if (Menu_is_visible)
          {
            // On nettoie les coordonnées
            Hide_cursor();
            Block(18*Menu_factor_X,Menu_status_Y,192*Menu_factor_X,Menu_factor_Y<<3,MC_Light);
            Update_rect(18*Menu_factor_X,Menu_status_Y,192*Menu_factor_X,Menu_factor_Y<<3);
            Display_cursor();
          }
        }
        else
        {
          if ( (prev_button_number!=BUTTON_CHOOSE_COL)
            || (temp_color!=First_color_in_palette)
            || (Old_MX!=Mouse_X) || (Old_MY!=Mouse_Y) )
          {
            // Le curseur est sur un nouveau bouton
            if (button_index!=BUTTON_CHOOSE_COL)
            {
              Hide_cursor();
              Print_in_menu(Menu_tooltip[button_index],0);
              Display_cursor();
            }
            else
            { // Le curseur est-il sur une couleur de la palette?
              int color;
              if ((color=Pick_color_in_palette())!=-1)
              {
                Hide_cursor();
                strcpy(str,Menu_tooltip[button_index]);
                sprintf(str+strlen(str),"%d (%d,%d,%d)",color,Main_palette[color].R,Main_palette[color].G,Main_palette[color].B);
                for (temp=strlen(str); temp<24; temp++)
                  str[temp]=' ';
                str[24]=0;
                Print_in_menu(str,0);
                Display_cursor();
              }
              else
              {
                if ( (Old_MX!=Mouse_X) || (Old_MY!=Mouse_Y) )
                {
                  Hide_cursor();
                  Block(18*Menu_factor_X,Menu_status_Y,192*Menu_factor_X,Menu_factor_Y<<3,MC_Light);
                  Update_rect(18*Menu_factor_X,Menu_status_Y,192*Menu_factor_X,8*Menu_factor_Y);
                  Display_cursor();
                }
              }
            }
          }
        }
      }

      prev_button_number=button_index;

      // Gestion des clicks
      if (Mouse_K)
      {
        if (Mouse_Y>=Menu_Y)
        {
          if (button_index>=0)
          {
            Select_button(button_index,Mouse_K);
            prev_button_number=-1;
          }
        }
        else
          if (Main_magnifier_mode) Move_separator();
      }

    }

    // we need to refresh that one as we may come from a sub window
    Cursor_in_menu=(Mouse_Y>=Menu_Y) ||
                      ( (Main_magnifier_mode) && (Mouse_X>=Main_separator_position) &&
                        (Mouse_X<Main_X_zoom) );


    // Le curseur se trouve dans l'image
    if ( (!Cursor_in_menu) && (Menu_is_visible) && (Old_MY != Mouse_Y || Old_MX != Mouse_X || Key || Mouse_K)) // On ne met les coordonnées à jour que si l'utilisateur a fait un truc
    {
       if ( (Current_operation!=OPERATION_COLORPICK) && (Current_operation!=OPERATION_REPLACE) )
       {
          if(Cursor_in_menu_previous)
          {
             Print_in_menu("X:       Y:             ",0);
          }
       }
       else
       {
          if(Cursor_in_menu_previous)
          {
             Print_in_menu("X:       Y:       (    )",0);
          }
       }
       Cursor_in_menu_previous = 0;
    }

    if(Cursor_in_menu)
    {
        Cursor_in_menu_previous = 1;
    }
    else
    {
      blink=Operation[Current_operation][Mouse_K_unique][Operation_stack_size].Hide_cursor;
 
      if (blink) Hide_cursor();
 
      Operation[Current_operation][Mouse_K_unique][Operation_stack_size].Action();

      if (blink) Display_cursor();
    }
    Old_MX=Mouse_X;
    Old_MY=Mouse_Y;
  }
  while (!Quitting);
}





//////////////////////////////////////////////////////////////////////////////
//      différentes fonctions d'affichage utilisées dans les fenêtres       //
//////////////////////////////////////////////////////////////////////////////

//----------------------- Tracer une fenêtre d'options -----------------------

void Open_window(word width,word height, char * title)
// Lors de l'appel à cette procédure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  //word i,j;

  Hide_cursor();

  Windows_open++;

  Window_width=width;
  Window_height=height;

  // Positionnement de la fenêtre
  Window_pos_X=(Screen_width-(width*Menu_factor_X))>>1;

  Window_pos_Y=(Screen_height-(height*Menu_factor_Y))>>1;

  // Sauvegarde de ce que la fenêtre remplace
  Save_background(&(Window_background[Windows_open-1]), Window_pos_X, Window_pos_Y, width, height);

  // Fenêtre grise
  Block(Window_pos_X+(Menu_factor_X<<1),Window_pos_Y+(Menu_factor_Y<<1),(width-4)*Menu_factor_X,(height-4)*Menu_factor_Y,MC_Light);

  // -- Frame de la fenêtre ----- --- -- -  -

  // Frame noir puis en relief
  Window_display_frame_mono(0,0,width,height,MC_Black);
  Window_display_frame_out(1,1,width-2,height-2);

  // Barre sous le titre
  Block(Window_pos_X+(Menu_factor_X<<3),Window_pos_Y+(11*Menu_factor_Y),(width-16)*Menu_factor_X,Menu_factor_Y,MC_Dark);
  Block(Window_pos_X+(Menu_factor_X<<3),Window_pos_Y+(12*Menu_factor_Y),(width-16)*Menu_factor_X,Menu_factor_Y,MC_White);

  Print_in_window((width-(strlen(title)<<3))>>1,3,title,MC_Black,MC_Light);

  if (Windows_open == 1)
  {
    Menu_is_visible_before_window=Menu_is_visible;
    Menu_is_visible=0;
    Menu_Y_before_window=Menu_Y;
    Menu_Y=Screen_height;
    Cursor_shape_before_window=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_ARROW;
    Paintbrush_hidden_before_window=Paintbrush_hidden;
    Paintbrush_hidden=1;
  }

  // Initialisation des listes de boutons de la fenêtre
  Window_normal_button_list  =NULL;
  Window_palette_button_list =NULL;
  Window_scroller_button_list=NULL;
  Window_special_button_list =NULL;
  Window_dropdown_button_list=NULL;
  Window_nb_buttons            =0;

}

//----------------------- Fermer une fenêtre d'options -----------------------

void Close_window(void)
// Lors de l'appel à cette procedure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  T_Normal_button   * temp1;
  T_Palette_button  * temp2;
  T_Scroller_button * temp3;
  T_Special_button  * temp4;
  T_Dropdown_button * temp5;

  Hide_cursor();

  while (Window_normal_button_list)
  {
    temp1=Window_normal_button_list->Next;
    free(Window_normal_button_list);
    Window_normal_button_list=temp1;
  }
  while (Window_palette_button_list)
  {
    temp2=Window_palette_button_list->Next;
    free(Window_palette_button_list);
    Window_palette_button_list=temp2;
  }
  while (Window_scroller_button_list)
  {
    temp3=Window_scroller_button_list->Next;
    free(Window_scroller_button_list);
    Window_scroller_button_list=temp3;
  }
  while (Window_special_button_list)
  {
    temp4=Window_special_button_list->Next;
    free(Window_special_button_list);
    Window_special_button_list=temp4;
  }
  while (Window_dropdown_button_list)
  {
    temp5=Window_dropdown_button_list->Next;
    Window_dropdown_clear_items(Window_dropdown_button_list);
    free(Window_dropdown_button_list);
    Window_dropdown_button_list=temp5;
  }

  if (Windows_open != 1)
  {
    // Restore de ce que la fenêtre cachait
    Restore_background(Window_background[Windows_open-1], Window_pos_X, Window_pos_Y, Window_width, Window_height);
    Window_background[Windows_open-1]=NULL;
    Update_rect(Window_pos_X,Window_pos_Y,Window_width*Menu_factor_X,Window_height*Menu_factor_Y);
    Windows_open--;
  }
  else
  {
    free(Window_background[Windows_open-1]);
    Window_background[Windows_open-1]=NULL;
    Windows_open--;
  
    Paintbrush_hidden=Paintbrush_hidden_before_window;
  
    Compute_paintbrush_coordinates();
  
    Menu_Y=Menu_Y_before_window;
    Menu_is_visible=Menu_is_visible_before_window;
    Cursor_shape=Cursor_shape_before_window;
    
    Display_all_screen();
    Display_menu();
  }

  Key=0;
  Mouse_K=0;
  
  Old_MX = -1;
  Old_MY = -1;

}


//---------------- Dessiner un bouton normal dans une fenêtre ----------------

void Window_draw_normal_bouton(word x_pos,word y_pos,word width,word height,
                                    char * title,byte undersc_letter,byte clickable)
{
  byte title_color;
  word text_x_pos,text_y_pos;

  if (clickable)
  {
    Window_display_frame_out(x_pos,y_pos,width,height);
    Window_display_frame_generic(x_pos-1,y_pos-1,width+2,height+2,MC_Black,MC_Black,MC_Dark,MC_Dark,MC_Dark);
    title_color=MC_Black;
  }
  else
  {
    Window_display_frame_out(x_pos,y_pos,width,height);
    Window_display_frame_mono(x_pos-1,y_pos-1,width+2,height+2,MC_Light);
    title_color=MC_Dark;
  }

  text_x_pos=x_pos+( (width-(strlen(title)<<3)+1) >>1 );
  text_y_pos=y_pos+((height-7)>>1);
  Print_in_window(text_x_pos,text_y_pos,title,title_color,MC_Light);

  if (undersc_letter)
    Block(Window_pos_X+((text_x_pos+((undersc_letter-1)<<3))*Menu_factor_X),
          Window_pos_Y+((text_y_pos+8)*Menu_factor_Y),
          Menu_factor_X<<3,Menu_factor_Y,MC_Dark);
}


// -- Button normal enfoncé dans la fenêtre --
void Window_select_normal_button(word x_pos,word y_pos,word width,word height)
{
  Window_display_frame_generic(x_pos,y_pos,width,height,MC_Dark,MC_Black,MC_Dark,MC_Dark,MC_Black);
  Update_rect(Window_pos_X+x_pos*Menu_factor_X, Window_pos_Y+y_pos*Menu_factor_Y, width*Menu_factor_X, height*Menu_factor_Y);
}

// -- Button normal désenfoncé dans la fenêtre --
void Window_unselect_normal_button(word x_pos,word y_pos,word width,word height)
{
  Window_display_frame_out(x_pos,y_pos,width,height);
  Update_rect(Window_pos_X+x_pos*Menu_factor_X, Window_pos_Y+y_pos*Menu_factor_Y, width*Menu_factor_X, height*Menu_factor_Y);
}


//--------------- Dessiner un bouton palette dans une fenêtre ----------------
void Window_draw_palette_bouton(word x_pos,word y_pos)
{
  word color;

  for (color=0; color<=255; color++)
    Block( Window_pos_X+((((color >> 4)*10)+x_pos+6)*Menu_factor_X),Window_pos_Y+((((color & 15)*5)+y_pos+3)*Menu_factor_Y),Menu_factor_X*5,Menu_factor_Y*5,color);

  Window_display_frame(x_pos,y_pos,164,86);
}


// -------------------- Effacer les TAGs sur les palette ---------------------
// Cette fonct° ne sert plus que lorsqu'on efface les tags dans le menu Spray.
void Window_clear_tags(void)
{
  word origin_x;
  word origin_y;
  word x_pos;
  word window_x_pos;
  //word window_y_pos;

  origin_x=Window_pos_X+(Window_palette_button_list->Pos_X+3)*Menu_factor_X;
  origin_y=Window_pos_Y+(Window_palette_button_list->Pos_Y+3)*Menu_factor_Y;
  for (x_pos=0,window_x_pos=origin_x;x_pos<16;x_pos++,window_x_pos+=(Menu_factor_X*10))
    Block(window_x_pos,origin_y,Menu_factor_X*3,Menu_factor_Y*80,MC_Light);
  Update_rect(origin_x,origin_y,ToWinL(160),ToWinH(80));
}


// ---- Tracer les TAGs sur les palettes du menu Palette ou du menu Shade ----
void Tag_color_range(byte start,byte end)
{
  word origin_x;
  word origin_y;
  //word x_pos;
  word y_pos;
  //word window_x_pos;
  word window_y_pos;
  word index;

  // On efface les anciens TAGs
  for (index=0;index<=start;index++)
    Block(Window_pos_X+(Window_palette_button_list->Pos_X+3+((index>>4)*10))*Menu_factor_X,
          Window_pos_Y+(Window_palette_button_list->Pos_Y+3+((index&15)* 5))*Menu_factor_Y,
          Menu_factor_X*3,Menu_factor_Y*5,MC_Light);

  for (index=end;index<256;index++)
    Block(Window_pos_X+(Window_palette_button_list->Pos_X+3+((index>>4)*10))*Menu_factor_X,
          Window_pos_Y+(Window_palette_button_list->Pos_Y+3+((index&15)* 5))*Menu_factor_Y,
          Menu_factor_X*3,Menu_factor_Y*5,MC_Light);

  // On affiche le 1er TAG
  origin_x=(Window_palette_button_list->Pos_X+3)+(start>>4)*10;
  origin_y=(Window_palette_button_list->Pos_Y+3)+(start&15)* 5;
  for (y_pos=0,window_y_pos=origin_y  ;y_pos<5;y_pos++,window_y_pos++)
    Pixel_in_window(origin_x  ,window_y_pos,MC_Black);
  for (y_pos=0,window_y_pos=origin_y+1;y_pos<3;y_pos++,window_y_pos++)
    Pixel_in_window(origin_x+1,window_y_pos,MC_Black);
  Pixel_in_window(origin_x+2,origin_y+2,MC_Black);

  if (start!=end)
  {
    // On complète le 1er TAG
    Pixel_in_window(origin_x+1,origin_y+4,MC_Black);

    // On affiche le 2ème TAG
    origin_x=(Window_palette_button_list->Pos_X+3)+(end>>4)*10;
    origin_y=(Window_palette_button_list->Pos_Y+3)+(end&15)* 5;
    for (y_pos=0,window_y_pos=origin_y; y_pos<5; y_pos++,window_y_pos++)
      Pixel_in_window(origin_x  ,window_y_pos,MC_Black);
    for (y_pos=0,window_y_pos=origin_y; y_pos<4; y_pos++,window_y_pos++)
      Pixel_in_window(origin_x+1,window_y_pos,MC_Black);
    Pixel_in_window(origin_x+2,origin_y+2,MC_Black);

    // On TAG toutes les couleurs intermédiaires
    for (index=start+1;index<end;index++)
    {
      Block(Window_pos_X+(Window_palette_button_list->Pos_X+3+((index>>4)*10))*Menu_factor_X,
            Window_pos_Y+(Window_palette_button_list->Pos_Y+3+((index&15)* 5))*Menu_factor_Y,
            Menu_factor_X*2,Menu_factor_Y*5,MC_Black);
      // On efface l'éventuelle pointe d'une ancienne extrémité de l'intervalle
      Pixel_in_window(Window_palette_button_list->Pos_X+5+((index>>4)*10),
                         Window_palette_button_list->Pos_Y+5+((index&15)* 5),
                         MC_Light);
    }


  }

  Update_rect(ToWinX(Window_palette_button_list->Pos_X+3),ToWinY(Window_palette_button_list->Pos_Y+3),ToWinL(12*16),ToWinH(5*16));

}


//------------------ Dessiner un scroller dans une fenêtre -------------------

void Compute_slider_cursor_height(T_Scroller_button * button)
{
  if (button->Nb_elements>button->Nb_visibles)
  {
    button->Cursor_height=(button->Nb_visibles*(button->Height-24))/button->Nb_elements;
    if (!(button->Cursor_height))
      button->Cursor_height=1;
  }
  else
  {
    button->Cursor_height=button->Height-24;
  }
}

void Window_draw_slider(T_Scroller_button * button)
{
  word slider_position;

  slider_position=button->Pos_Y+12;

  Block(Window_pos_X+(button->Pos_X*Menu_factor_X),
        Window_pos_Y+(slider_position*Menu_factor_Y),
        11*Menu_factor_X,(button->Height-24)*Menu_factor_Y,MC_Black/*MC_Dark*/);

  if (button->Nb_elements>button->Nb_visibles)
    slider_position+=Round_div(button->Position*(button->Height-24-button->Cursor_height),button->Nb_elements-button->Nb_visibles);

  Block(Window_pos_X+(button->Pos_X*Menu_factor_X),
        Window_pos_Y+(slider_position*Menu_factor_Y),
        11*Menu_factor_X,button->Cursor_height*Menu_factor_Y,MC_Dark/*MC_White*/);

  Update_rect(Window_pos_X+(button->Pos_X*Menu_factor_X),
        Window_pos_Y+button->Pos_Y*Menu_factor_Y,
        11*Menu_factor_X,(button->Height)*Menu_factor_Y);
}

void Window_draw_scroller_bouton(T_Scroller_button * button)
{
  Window_display_frame_generic(button->Pos_X-1,button->Pos_Y-1,13,button->Height+2,MC_Black,MC_Black,MC_Dark,MC_Dark,MC_Dark);
  Window_display_frame_mono(button->Pos_X-1,button->Pos_Y+11,13,button->Height-22,MC_Black);
  Window_display_frame_out(button->Pos_X,button->Pos_Y,11,11);
  Window_display_frame_out(button->Pos_X,button->Pos_Y+button->Height-11,11,11);
  Print_in_window(button->Pos_X+2,button->Pos_Y+2,"\030",MC_Black,MC_Light);
  Print_in_window(button->Pos_X+2,button->Pos_Y+button->Height-9,"\031",MC_Black,MC_Light);
  Window_draw_slider(button);
}


//--------------- Dessiner une zone de saisie dans une fenêtre ---------------

void Window_draw_input_bouton(word x_pos,word y_pos,word width_in_characters)
{
  Window_display_frame_in(x_pos,y_pos,(width_in_characters<<3)+3,11);
}


//------------ Modifier le contenu (caption) d'une zone de saisie ------------

void Window_input_content(T_Special_button * button, char * content)
{
  Print_in_window_limited(button->Pos_X+2,button->Pos_Y+2,content,button->Width/8,MC_Black,MC_Light);
}

//------------ Effacer le contenu (caption) d'une zone de saisie ------------

void Window_clear_input_button(T_Special_button * button)
{
  Block((button->Pos_X+2)*Menu_factor_X+Window_pos_X,(button->Pos_Y+2)*Menu_factor_Y+Window_pos_Y,(button->Width/8)*8*Menu_factor_X,8*Menu_factor_Y,MC_Light);
  Update_rect((button->Pos_X+2)*Menu_factor_X+Window_pos_X,(button->Pos_Y+2)*Menu_factor_Y+Window_pos_Y,button->Width/8*8*Menu_factor_X,8*Menu_factor_Y);
}


//------ Rajout d'un bouton à la liste de ceux présents dans la fenêtre ------

T_Normal_button * Window_set_normal_button(word x_pos, word y_pos,
                                   word width, word height,
                                   char * title, byte undersc_letter,
                                   byte clickable, word shortcut)
{
  T_Normal_button * temp=NULL;

  Window_nb_buttons++;

  if (clickable)
  {
    temp=(T_Normal_button *)malloc(sizeof(T_Normal_button));
    temp->Number   =Window_nb_buttons;
    temp->Pos_X    =x_pos;
    temp->Pos_Y    =y_pos;
    temp->Width    =width;
    temp->Height   =height;
    temp->Clickable=clickable;
    temp->Shortcut =shortcut;
    temp->Repeatable=0;

    temp->Next=Window_normal_button_list;
    Window_normal_button_list=temp;
  }

  Window_draw_normal_bouton(x_pos,y_pos,width,height,title,undersc_letter,clickable);
  return temp;
}
//------ Rajout d'un bouton à la liste de ceux présents dans la fenêtre ------

T_Normal_button * Window_set_repeatable_button(word x_pos, word y_pos,
                                   word width, word height,
                                   char * title, byte undersc_letter,
                                   byte clickable, word shortcut)
{
  T_Normal_button * temp=NULL;

  Window_nb_buttons++;

  if (clickable)
  {
    temp=(T_Normal_button *)malloc(sizeof(T_Normal_button));
    temp->Number   =Window_nb_buttons;
    temp->Pos_X    =x_pos;
    temp->Pos_Y    =y_pos;
    temp->Width  =width;
    temp->Height  =height;
    temp->Shortcut=shortcut;
    temp->Repeatable=1;

    temp->Next=Window_normal_button_list;
    Window_normal_button_list=temp;
  }

  Window_draw_normal_bouton(x_pos,y_pos,width,height,title,undersc_letter,clickable);
  return temp;
}

T_Palette_button * Window_set_palette_button(word x_pos, word y_pos)
{
  T_Palette_button * temp;

  temp=(T_Palette_button *)malloc(sizeof(T_Palette_button));
  temp->Number   =++Window_nb_buttons;
  temp->Pos_X    =x_pos;
  temp->Pos_Y    =y_pos;

  temp->Next=Window_palette_button_list;
  Window_palette_button_list=temp;

  Window_draw_palette_bouton(x_pos,y_pos);
  return temp;
}


T_Scroller_button * Window_set_scroller_button(word x_pos, word y_pos,
                                     word height,
                                     word nb_elements,
                                     word nb_elements_visible,
                                     word initial_position)
{
  T_Scroller_button * temp;

  temp=(T_Scroller_button *)malloc(sizeof(T_Scroller_button));
  temp->Number        =++Window_nb_buttons;
  temp->Pos_X         =x_pos;
  temp->Pos_Y         =y_pos;
  temp->Height       =height;
  temp->Nb_elements   =nb_elements;
  temp->Nb_visibles   =nb_elements_visible;
  temp->Position      =initial_position;
  Compute_slider_cursor_height(temp);

  temp->Next=Window_scroller_button_list;
  Window_scroller_button_list=temp;

  Window_draw_scroller_bouton(temp);
  return temp;
}


T_Special_button * Window_set_special_button(word x_pos,word y_pos,word width,word height)
{
  T_Special_button * temp;

  temp=(T_Special_button *)malloc(sizeof(T_Special_button));
  temp->Number   =++Window_nb_buttons;
  temp->Pos_X    =x_pos;
  temp->Pos_Y    =y_pos;
  temp->Width  =width;
  temp->Height  =height;

  temp->Next=Window_special_button_list;
  Window_special_button_list=temp;
  return temp;
}


T_Special_button * Window_set_input_button(word x_pos,word y_pos,word width_in_characters)
{
  T_Special_button *temp;
  temp=Window_set_special_button(x_pos,y_pos,(width_in_characters<<3)+3,11);
  Window_draw_input_bouton(x_pos,y_pos,width_in_characters);
  return temp;
}

T_Dropdown_button * Window_set_dropdown_button(word x_pos,word y_pos,word width,word height,word dropdown_width,const char *label,byte display_choice,byte display_centered,byte display_arrow,byte active_button)
{
  T_Dropdown_button *temp;
  
  temp=(T_Dropdown_button *)malloc(sizeof(T_Dropdown_button));
  temp->Number       =++Window_nb_buttons;
  temp->Pos_X        =x_pos;
  temp->Pos_Y        =y_pos;
  temp->Width      =width;
  temp->Height      =height;
  temp->Display_choice =display_choice;
  temp->First_item=NULL;
  temp->Dropdown_width=dropdown_width?dropdown_width:width;
  temp->Display_centered=display_centered;
  temp->Display_arrow=display_arrow;
  temp->Active_button=active_button;

  temp->Next=Window_dropdown_button_list;
  Window_dropdown_button_list=temp;
  Window_draw_normal_bouton(x_pos,y_pos,width,height,"",-1,1);
  if (label && label[0])
    Print_in_window(temp->Pos_X+2,temp->Pos_Y+(temp->Height-7)/2,label,MC_Black,MC_Light);
  if (display_arrow)
    Window_display_icon_sprite(temp->Pos_X+temp->Width-10,temp->Pos_Y+(temp->Height-7)/2,ICON_DROPDOWN);
  
  return temp;
}

// Ajoute un choix à une dropdown. Le libellé est seulement référencé,
// il doit pointer sur une zone qui doit être encore valide à la fermeture 
// de la fenêtre (comprise).
void Window_dropdown_add_item(T_Dropdown_button * dropdown, word btn_number, const char *label)
{
  T_Dropdown_choice *temp;
  T_Dropdown_choice *last;
  
  temp=(T_Dropdown_choice *)malloc(sizeof(T_Dropdown_choice));
  temp->Number =btn_number;
  temp->Label=label;
  temp->Next=NULL;

  last=dropdown->First_item;
  if (last)
  {
    // On cherche le dernier élément
    for (;last->Next;last=last->Next)
      ;
    last->Next=temp;
  }
  else
  {
    dropdown->First_item=temp;
  }
}

// ------------- Suppression de tous les choix d'une dropdown ---------
void Window_dropdown_clear_items(T_Dropdown_button * dropdown)
{
  T_Dropdown_choice * next_choice;
    while (dropdown->First_item)
    {
      next_choice=dropdown->First_item->Next;
      free(dropdown->First_item);
      dropdown->First_item=next_choice;
    }
}

//----------------------- Ouverture d'un pop-up -----------------------

void Open_popup(word x_pos, word y_pos, word width,word height)
// Lors de l'appel à cette procédure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.

// Note : les pop-ups sont gérés comme s'ils étaient des sous-fenêtres, ils ont donc leur propre boucle d'évènements et tout, on peut ajouter des widgets dedans, ...
// Les différences sont surtout graphiques :
    // -Possibilité de préciser la position XY
    // -Pas de titre
    // -Pas de cadre en relief mais seulement un plat, et il est blanc au lieu de noir.
{
  Windows_open++;

  Window_width=width;
  Window_height=height;
  Window_pos_X=x_pos;
  Window_pos_Y=y_pos;

  // Sauvegarde de ce que la fenêtre remplace
  Save_background(&(Window_background[Windows_open-1]), Window_pos_X, Window_pos_Y, width, height);

/*
  // Fenêtre grise
  Block(Window_pos_X+1*Menu_factor_X,
        Window_pos_Y+1*Menu_factor_Y,
        (width-2)*Menu_factor_X,(height-2)*Menu_factor_Y,MC_Light);

  // Frame noir puis en relief
  Window_display_frame_mono(0,0,width,height,MC_White);
*/
  if (Windows_open == 1)
  {
    Menu_is_visible_before_window=Menu_is_visible;
    Menu_is_visible=0;
    Menu_Y_before_window=Menu_Y;
    Menu_Y=Screen_height;
    Cursor_shape_before_window=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_ARROW;
    Paintbrush_hidden_before_window=Paintbrush_hidden;
    Paintbrush_hidden=1;
  }

  // Initialisation des listes de boutons de la fenêtre
  Window_normal_button_list  =NULL;
  Window_palette_button_list =NULL;
  Window_scroller_button_list=NULL;
  Window_special_button_list =NULL;
  Window_dropdown_button_list =NULL;
  Window_nb_buttons            =0;

}

//----------------------- Fermer une fenêtre d'options -----------------------

void Close_popup(void)
// Lors de l'appel à cette procedure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  T_Normal_button   * temp1;
  T_Palette_button  * temp2;
  T_Scroller_button * temp3;
  T_Special_button  * temp4;
  T_Dropdown_button  * temp5;

  Hide_cursor();

  while (Window_normal_button_list)
  {
    temp1=Window_normal_button_list->Next;
    free(Window_normal_button_list);
    Window_normal_button_list=temp1;
  }
  while (Window_palette_button_list)
  {
    temp2=Window_palette_button_list->Next;
    free(Window_palette_button_list);
    Window_palette_button_list=temp2;
  }
  while (Window_scroller_button_list)
  {
    temp3=Window_scroller_button_list->Next;
    free(Window_scroller_button_list);
    Window_scroller_button_list=temp3;
  }
  while (Window_special_button_list)
  {
    temp4=Window_special_button_list->Next;
    free(Window_special_button_list);
    Window_special_button_list=temp4;
  }
  while (Window_dropdown_button_list)
  {
    Window_dropdown_clear_items(Window_dropdown_button_list);
    temp5=Window_dropdown_button_list->Next;
    free(Window_dropdown_button_list);
    Window_dropdown_button_list=temp5;
  }

  if (Windows_open != 1)
  {
    // Restore de ce que la fenêtre cachait
    Restore_background(Window_background[Windows_open-1], Window_pos_X, Window_pos_Y, Window_width, Window_height);
    Window_background[Windows_open-1]=NULL;
    Update_rect(Window_pos_X,Window_pos_Y,Window_width*Menu_factor_X,Window_height*Menu_factor_Y);
    Windows_open--;
  }
  else
  {
    free(Window_background[Windows_open-1]);
    Windows_open--;
  
    Paintbrush_hidden=Paintbrush_hidden_before_window;
  
    Compute_paintbrush_coordinates();
  
    Menu_Y=Menu_Y_before_window;
    Menu_is_visible=Menu_is_visible_before_window;
    Cursor_shape=Cursor_shape_before_window;
    
    Display_all_screen();
    Display_menu();
  }

  Key=0;
  Mouse_K=0;
  
  Old_MX = -1;
  Old_MY = -1;


}
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//       Mini-MOTEUR utilisé dans les fenêtres (menus des boutons...)       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


// -- Indique si on a cliqué dans une zone définie par deux points extremes --
byte Window_click_in_rectangle(short start_x,short start_y,short end_x,short end_y)
{
  short x_pos,y_pos;

  x_pos=((short)Mouse_X-Window_pos_X)/Menu_factor_X;
  y_pos=((short)Mouse_Y-Window_pos_Y)/Menu_factor_Y;

  return ((x_pos>=start_x) &&
          (y_pos>=start_y) &&
          (x_pos<=end_x)   &&
          (y_pos<=end_y));
}


// --- Attend que l'on clique dans la palette pour renvoyer la couleur choisie
// ou bien renvoie -1 si on a annulé l'action pas click-droit ou Escape ------
short Wait_click_in_palette(T_Palette_button * button)
{
  short start_x=button->Pos_X+5;
  short start_y=button->Pos_Y+3;
  short end_x  =button->Pos_X+160;
  short end_y  =button->Pos_Y+82;
  byte  selected_color;
  byte  old_hide_cursor;
  byte  old_main_magnifier_mode;

  Hide_cursor();
  old_hide_cursor=Cursor_hidden;
  old_main_magnifier_mode=Main_magnifier_mode;
  Main_magnifier_mode=0;
  Cursor_hidden=0;
  Cursor_shape=CURSOR_SHAPE_TARGET;
  Display_cursor();

  for (;;)
  {
    while(!Get_input())Wait_VBL();

    if (Mouse_K==LEFT_SIDE)
    {
      if (Window_click_in_rectangle(start_x,start_y,end_x,end_y))
      {
        Hide_cursor();
        selected_color=(((Mouse_X-Window_pos_X)/Menu_factor_X)-(button->Pos_X+2)) / 10 * 16 +
        (((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-(button->Pos_Y+3)) / 5;
        Cursor_shape=CURSOR_SHAPE_ARROW;
        Cursor_hidden=old_hide_cursor;
        Main_magnifier_mode=old_main_magnifier_mode;
        Display_cursor();
        return selected_color;
      }
      if ((Mouse_X<Window_pos_X) || (Mouse_Y<Window_pos_Y) ||
          (Mouse_X>=Window_pos_X+(Window_width*Menu_factor_X)) ||
          (Mouse_Y>=Window_pos_Y+(Window_height*Menu_factor_Y)) )
      {
        Hide_cursor();
        selected_color=Read_pixel(Mouse_X,Mouse_Y);
        Cursor_shape=CURSOR_SHAPE_ARROW;
        Cursor_hidden=old_hide_cursor;
        Main_magnifier_mode=old_main_magnifier_mode;
        Display_cursor();
        return selected_color;
      }
    }

    if ((Mouse_K==RIGHT_SIDE) || (Key==KEY_ESC))
    {
      Hide_cursor();
      Cursor_shape=CURSOR_SHAPE_ARROW;
      Cursor_hidden=old_hide_cursor;
      Main_magnifier_mode=old_main_magnifier_mode;
      Display_cursor();
      return -1;
    }
  }
}



// -------------- Récupération d'une couleur derrière un menu ----------------
void Get_color_behind_window(byte * color, byte * click)
{
  short width=Window_width*Menu_factor_X;
  short height=Window_height*Menu_factor_Y;
  short old_x=-1;
  short old_y=-1;
  short index;
  short a,b,c,d; // Variables temporaires et multitâches...
  byte * buffer;
  char str[25];
  byte cursor_was_hidden;


  if ((buffer=(byte *) malloc(width*height)))
  {
    Hide_cursor();

    cursor_was_hidden=Cursor_hidden;
    Cursor_hidden=0;

    for (index=0; index<height; index++)
      Read_line(Window_pos_X,Window_pos_Y+index,width,buffer+((int)index*width*Pixel_width));
    a=Menu_Y;
    Menu_Y=Menu_Y_before_window;
    b=Menu_is_visible;
    Menu_is_visible=Menu_is_visible_before_window;
    Display_all_screen();
    Display_menu();
    Menu_Y=a;
    Menu_is_visible=b;

    Cursor_shape=CURSOR_SHAPE_COLORPICKER;
    b=Paintbrush_hidden;
    Paintbrush_hidden=1;
    c=-1; // color pointée: au début aucune, comme ça on initialise tout
    if (Menu_is_visible_before_window)
      Print_in_menu(Menu_tooltip[BUTTON_CHOOSE_COL],0);

    Display_cursor();

    do
    {
      if(!Get_input())Wait_VBL();

      if ((Mouse_X!=old_x) || (Mouse_Y!=old_y))
      {
        Hide_cursor();
        a=Read_pixel(Mouse_X,Mouse_Y);
        if (a!=c)
        {
          c=a; // Mise à jour de la couleur pointée
          if (Menu_is_visible_before_window)
          {
            sprintf(str,"%d",a);
            d=strlen(str);
            strcat(str,"   (");
            sprintf(str+strlen(str),"%d",Main_palette[a].R);
            strcat(str,",");
            sprintf(str+strlen(str),"%d",Main_palette[a].G);
            strcat(str,",");
            sprintf(str+strlen(str),"%d",Main_palette[a].B);
            strcat(str,")");
            a=24-d;
            for (index=strlen(str); index<a; index++)
              str[index]=' ';
            str[a]=0;
            Print_in_menu(str,strlen(Menu_tooltip[BUTTON_CHOOSE_COL]));

            Print_general((26+((d+strlen(Menu_tooltip[BUTTON_CHOOSE_COL]))<<3))*Menu_factor_X,
                          Menu_status_Y," ",0,c);
          }
        }
        Display_cursor();
      }

      old_x=Mouse_X;
      old_y=Mouse_Y;
    } while (!(Mouse_K || (Key==KEY_ESC)));

    if (Mouse_K)
    {
      Hide_cursor();
      *click=Mouse_K;
      *color=Read_pixel(Mouse_X,Mouse_Y);
    }
    else
    {
      *click=0;
      Hide_cursor();
    }

    for (index=0; index<height; index++)
      Display_line(Window_pos_X,Window_pos_Y+index,width,buffer+((int)index*width));
    Update_rect(Window_pos_X, Window_pos_Y, Window_width*Menu_factor_X, Window_height*Menu_factor_Y);
    Cursor_shape=CURSOR_SHAPE_ARROW;
    Paintbrush_hidden=b;
    Cursor_hidden=cursor_was_hidden;
    Display_cursor();

    free(buffer);
  }
  else
  {
    Error(0);
  }
}



// ------------ Opération de déplacement de la fenêtre à l'écran -------------
void Move_window(short dx, short dy)
{
  short new_x=Mouse_X-dx;
  short new_y=Mouse_Y-dy;
  short old_x;
  short old_y;
  short width=Window_width*Menu_factor_X;
  short height=Window_height*Menu_factor_Y;
  short a;
  byte  b;
  byte  *buffer=NULL;

  Hide_cursor();

  Horizontal_XOR_line(new_x,new_y,width);
  Vertical_XOR_line(new_x,new_y+1,height-2);
  Vertical_XOR_line(new_x+width-1,new_y+1,height-2);
  Horizontal_XOR_line(new_x,new_y+height-1,width);
  Update_rect(new_x,new_y,width,height);
  Cursor_shape=CURSOR_SHAPE_MULTIDIRECTIONAL;
  Display_cursor();

  while (Mouse_K)
  {
    old_x=new_x;
    old_y=new_y;

    while(!Get_input() && new_x==Mouse_X-dx && new_y==Mouse_Y-dy) Wait_VBL();

    new_x=Mouse_X-dx;

    if (new_x<0)
    {
      new_x=0;
      dx = Mouse_X;
    }
    if (new_x>Screen_width-width)
    {
      new_x=Screen_width-width;
      dx = Mouse_X - new_x;
    }

    new_y=Mouse_Y-dy;

    if (new_y<0)
    {
      new_y=0;
      dy = Mouse_Y;
    }
    if (new_y>Screen_height-height)
    {
      new_y=Screen_height-height;
      dy = Mouse_Y - new_y;
    }

    if ((new_x!=old_x) || (new_y!=old_y))
    {
      Hide_cursor();

      Horizontal_XOR_line(old_x,old_y,width);
      Vertical_XOR_line(old_x,old_y+1,height-2);
      Vertical_XOR_line(old_x+width-1,old_y+1,height-2);
      Horizontal_XOR_line(old_x,old_y+height-1,width);

      Horizontal_XOR_line(new_x,new_y,width);
      Vertical_XOR_line(new_x,new_y+1,height-2);
      Vertical_XOR_line(new_x+width-1,new_y+1,height-2);
      Horizontal_XOR_line(new_x,new_y+height-1,width);

      Display_cursor();
      Update_rect(old_x,old_y,width,height);
      Update_rect(new_x,new_y,width,height);
    }
  }

  Hide_cursor();
  Horizontal_XOR_line(new_x,new_y,width);
  Vertical_XOR_line(new_x,new_y+1,height-2);
  Vertical_XOR_line(new_x+width-1,new_y+1,height-2);
  Horizontal_XOR_line(new_x,new_y+height-1,width);

  if ((new_x!=Window_pos_X)
   || (new_y!=Window_pos_Y))
  {
    a=Menu_Y;
    Menu_Y=Menu_Y_before_window;
    b=Menu_is_visible;
    Menu_is_visible=Menu_is_visible_before_window;
    //Display_all_screen();
    //Display_menu();
    Menu_Y=a;
    Menu_is_visible=b;

    // Sauvegarde du contenu actuel de la fenêtre
    Save_background(&buffer, Window_pos_X, Window_pos_Y, Window_width, Window_height);
    
    // Restore de ce que la fenêtre cachait
    Restore_background(Window_background[Windows_open-1], Window_pos_X, Window_pos_Y, Window_width, Window_height);
    Window_background[Windows_open-1] = NULL;

    // Sauvegarde de ce que la fenêtre remplace
    Save_background(&(Window_background[Windows_open-1]), new_x, new_y, Window_width, Window_height);

    // Raffichage de la fenêtre
    Restore_background(buffer, new_x, new_y, Window_width, Window_height);
    buffer = NULL;

    // Mise à jour du rectangle englobant
    Update_rect(
      (new_x>Window_pos_X)?Window_pos_X:new_x,
      (new_y>Window_pos_Y)?Window_pos_Y:new_y,
      ((new_x>Window_pos_X)?(new_x-Window_pos_X):(Window_pos_X-new_x)) + Window_width*Menu_factor_X,
      ((new_y>Window_pos_Y)?(new_y-Window_pos_Y):(Window_pos_Y-new_y)) + Window_height*Menu_factor_Y);
    Window_pos_X=new_x;
    Window_pos_Y=new_y;

  }
  else
  {
    // Update pour effacer le rectangle XOR
    Update_rect(Window_pos_X, Window_pos_Y, Window_width*Menu_factor_X, Window_height*Menu_factor_Y);
  }    
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Display_cursor();

}

// Gestion des dropdown
short Window_dropdown_on_click(T_Dropdown_button *Button)
{
  short nb_choices;
  short choice_index;
  short selected_index;
  short old_selected_index;
  short box_height;
  T_Dropdown_choice *item;
  // Taille de l'ombre portée (en plus des dimensions normales)
  #define SHADOW_RIGHT 3
  #define SHADOW_BOTTOM 4

  
  // Comptage des items pour calculer la taille
  nb_choices=0;
  for (item=Button->First_item; item!=NULL; item=item->Next)
  {
    nb_choices++;
  }
  box_height=3+nb_choices*8+1;
  
  Hide_cursor();
  Window_select_normal_button(Button->Pos_X,Button->Pos_Y,Button->Width,Button->Height);
  Open_popup(
    Window_pos_X+(Button->Pos_X)*Menu_factor_X,
    Window_pos_Y+(Button->Pos_Y+Button->Height)*Menu_factor_Y,
    Button->Dropdown_width+SHADOW_RIGHT,
    box_height+SHADOW_BOTTOM);

  // Dessin de la boite

  // Bord gauche
  Block(Window_pos_X,Window_pos_Y,Menu_factor_X,box_height*Menu_factor_Y,MC_Black);
  // Frame fonce et blanc
  Window_display_frame_out(1,0,Button->Dropdown_width-1,box_height);
  // Ombre portée
  if (SHADOW_BOTTOM)
  {
    Block(Window_pos_X+SHADOW_RIGHT*Menu_factor_X,
        Window_pos_Y+box_height*Menu_factor_Y,
        Button->Dropdown_width*Menu_factor_X,
        SHADOW_BOTTOM*Menu_factor_Y,
        MC_Black);
    Block(Window_pos_X,
        Window_pos_Y+box_height*Menu_factor_Y,
        SHADOW_RIGHT*Menu_factor_X,
        Menu_factor_Y,
        MC_Black);
  }
  if (SHADOW_RIGHT)
  {
    Block(Window_pos_X+Button->Dropdown_width*Menu_factor_X,
        Window_pos_Y+SHADOW_BOTTOM*Menu_factor_Y,
        SHADOW_RIGHT*Menu_factor_X,
        (box_height-SHADOW_BOTTOM)*Menu_factor_Y,
        MC_Black);
    Block(Window_pos_X+Button->Dropdown_width*Menu_factor_X,
        Window_pos_Y,
        Menu_factor_X,
        SHADOW_BOTTOM*Menu_factor_Y,
        MC_Black);
  }

  selected_index=-1;
  while (1)
  {
    old_selected_index = selected_index;
    // Fenêtre grise
    Block(Window_pos_X+2*Menu_factor_X,
        Window_pos_Y+1*Menu_factor_Y,
        (Button->Dropdown_width-3)*Menu_factor_X,(box_height-2)*Menu_factor_Y,MC_Light);
    // Affichage des items
    for(item=Button->First_item,choice_index=0; item!=NULL; item=item->Next,choice_index++)
    {
      byte color_1;
      byte color_2;
      if (choice_index==selected_index)
      {
        color_1=MC_White;
        color_2=MC_Dark;
        Block(Window_pos_X+3*Menu_factor_X,
        Window_pos_Y+((2+choice_index*8)*Menu_factor_Y),
        (Button->Dropdown_width-5)*Menu_factor_X,(8)*Menu_factor_Y,MC_Dark);
      }
      else
      {
        color_1=MC_Black;
        color_2=MC_Light;
      }
      Print_in_window(3,2+choice_index*8,item->Label,color_1,color_2);
    }
    Update_rect(Window_pos_X,Window_pos_Y,Window_width*Menu_factor_X,Window_height*Menu_factor_Y);
    Display_cursor();

    do 
    {
      // Attente
      if(!Get_input())
        Wait_VBL();
      // Mise à jour du survol
      selected_index=Window_click_in_rectangle(2,2,Button->Dropdown_width-2,box_height-1)?
        (((Mouse_Y-Window_pos_Y)/Menu_factor_Y-2)>>3) : -1;

    } while (Mouse_K && selected_index==old_selected_index);
    
    if (!Mouse_K)
      break;
    Hide_cursor();
  }

  Close_popup();  


  Window_unselect_normal_button(Button->Pos_X,Button->Pos_Y,Button->Width,Button->Height);
  Display_cursor();

  if (selected_index>=0 && selected_index<nb_choices)
  {
    for(item=Button->First_item; selected_index; item=item->Next,selected_index--)
      ;
    Window_attribute2=item->Number;
    if (Button->Display_choice)
    {
      // Mettre à jour automatiquement le libellé de la dropdown
      Print_in_window(Button->Pos_X+2,Button->Pos_Y+(Button->Height-7)/2,item->Label,MC_Black,MC_Light);
    }
    return Button->Number;
  }
  Window_attribute2=-1;
  return 0;
}

// --- Fonction de clic sur un bouton a peu près ordinaire:
// Attend que l'on relache le bouton, et renvoie le numero du bouton si on
// est resté dessus, 0 si on a annulé en sortant du bouton.
short Window_normal_button_onclick(word x_pos, word y_pos, word width, word height, short btn_number)
{
  while(1)
  {
    Hide_cursor();
    Window_select_normal_button(x_pos,y_pos,width,height);
    Display_cursor();
    while (Window_click_in_rectangle(x_pos,y_pos,x_pos+width-1,y_pos+height-1))
    {
      if(!Get_input())
        Wait_VBL();
      if (!Mouse_K)
      {
        Hide_cursor();
        Window_unselect_normal_button(x_pos,y_pos,width,height);
        Display_cursor();
        return btn_number;
      }
    }
    Hide_cursor();
    Window_unselect_normal_button(x_pos,y_pos,width,height);
    Display_cursor();
    while (!(Window_click_in_rectangle(x_pos,y_pos,x_pos+width-1,y_pos+height-1)))
    {
      if(!Get_input())
        Wait_VBL();
      if (!Mouse_K)
        return 0;
    }
  }
}

// --- Renvoie le numéro du bouton clicke (-1:hors de la fenêtre, 0:aucun) ---
short Window_get_clicked_button(void)
{
  T_Normal_button   * temp1;
  T_Palette_button  * temp2;
  T_Scroller_button * temp3;
  T_Special_button  * temp4;
  T_Dropdown_button * temp5;

  long max_slider_height;

  Window_attribute1=Mouse_K;

  // Test du click sur les boutons normaux
  for (temp1=Window_normal_button_list; temp1; temp1=temp1->Next)
  {
    if (Window_click_in_rectangle(temp1->Pos_X,temp1->Pos_Y,temp1->Pos_X+temp1->Width-1,temp1->Pos_Y+temp1->Height-1))
    {
      if (temp1->Repeatable)
      {
        Hide_cursor();
        Window_select_normal_button(temp1->Pos_X,temp1->Pos_Y,temp1->Width,temp1->Height);
        Display_cursor();
        Slider_timer((Mouse_K==1)? Config.Delay_left_click_on_slider : Config.Delay_right_click_on_slider);
        Hide_cursor();
        Window_unselect_normal_button(temp1->Pos_X,temp1->Pos_Y,temp1->Width,temp1->Height);
        Display_cursor();        
        return temp1->Number;
      }
      return Window_normal_button_onclick(temp1->Pos_X,temp1->Pos_Y,temp1->Width,temp1->Height,temp1->Number);
    }
  }

  // Test du click sur les zones "palette"
  for (temp2=Window_palette_button_list; temp2; temp2=temp2->Next)
  {
    if (Window_click_in_rectangle(temp2->Pos_X+5,temp2->Pos_Y+3,temp2->Pos_X+160,temp2->Pos_Y+82))
    {
      // On stocke dans Attribut2 le numero de couleur cliqué
      Window_attribute2 = (((Mouse_X-Window_pos_X)/Menu_factor_X)-(temp2->Pos_X+2)) / 10 * 16 +
        (((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-(temp2->Pos_Y+3)) / 5;
        return temp2->Number;
    }
  }

  // Test du click sur les barres de défilement
  for (temp3=Window_scroller_button_list; temp3; temp3=temp3->Next)
  {
    if (Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y,temp3->Pos_X+10,temp3->Pos_Y+temp3->Height-1))
    {
      // Button flèche Haut
      if (Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y,temp3->Pos_X+10,temp3->Pos_Y+10))
      {
        Hide_cursor();
        Window_select_normal_button(temp3->Pos_X,temp3->Pos_Y,11,11);

        if (temp3->Position)
        {
          temp3->Position--;
          Window_attribute1=1;
          Window_attribute2=temp3->Position;
          Window_draw_slider(temp3);
        }
        else
          Window_attribute1=0;
        
        Display_cursor();

        Slider_timer((Mouse_K==1)? Config.Delay_left_click_on_slider : Config.Delay_right_click_on_slider);

        Hide_cursor();
        Window_unselect_normal_button(temp3->Pos_X,temp3->Pos_Y,11,11);
        Display_cursor();
      }
      else
      // Button flèche Bas
      if (Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y+temp3->Height-11,temp3->Pos_X+10,temp3->Pos_Y+temp3->Height-1))
      {
        Hide_cursor();
        Window_select_normal_button(temp3->Pos_X,temp3->Pos_Y+temp3->Height-11,11,11);

        if (temp3->Position+temp3->Nb_visibles<temp3->Nb_elements)
        {
          temp3->Position++;
          Window_attribute1=2;
          Window_attribute2=temp3->Position;
          Window_draw_slider(temp3);
        }
        else
          Window_attribute1=0;

        Display_cursor();

        Slider_timer((Mouse_K==1)? Config.Delay_left_click_on_slider : Config.Delay_right_click_on_slider);

        Hide_cursor();
        Window_unselect_normal_button(temp3->Pos_X,temp3->Pos_Y+temp3->Height-11,11,11);
        Display_cursor();
      }
      else
      // Jauge
      if (Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y+12,temp3->Pos_X+10,temp3->Pos_Y+temp3->Height-13))
      {
        if (temp3->Nb_elements>temp3->Nb_visibles)
        {
          // S'il y a la place de faire scroller le curseur:

          max_slider_height=(temp3->Height-24);

          // Window_attribute2 reçoit la position dans la jauge correspondant au click
          Window_attribute2 =(Mouse_Y-Window_pos_Y) / Menu_factor_Y;
          Window_attribute2-=(temp3->Pos_Y+12+((temp3->Cursor_height-1)>>1));
          Window_attribute2*=(temp3->Nb_elements-temp3->Nb_visibles);

          if (Window_attribute2<0)
            Window_attribute2=0;
          else
          {
            Window_attribute2 =Round_div(Window_attribute2,max_slider_height-temp3->Cursor_height);
            if (Window_attribute2+temp3->Nb_visibles>temp3->Nb_elements)
              Window_attribute2=temp3->Nb_elements-temp3->Nb_visibles;
          }

          // Si le curseur de la jauge bouge:

          if (temp3->Position!=Window_attribute2)
          {
            temp3->Position=Window_attribute2;
            Window_attribute1=3;
            Hide_cursor();
            Window_draw_slider(temp3);
            Display_cursor();
          }
          else
            // Si le curseur de la jauge ne bouge pas:
            Window_attribute1=0;
        }
        else
          // S'il n'y a pas la place de bouger le curseur de la jauge:
          Window_attribute1=0;
      }
      else
        // Le click se situe dans la zone de la jauge mais n'est sur aucune
        // des 3 parties importantes de la jauge
        Window_attribute1=0;

      return (Window_attribute1)? temp3->Number : 0;
    }
  }

  // Test du click sur une zone spéciale
  for (temp4=Window_special_button_list; temp4; temp4=temp4->Next)
  {
    if (Window_click_in_rectangle(temp4->Pos_X,temp4->Pos_Y,temp4->Pos_X+temp4->Width-1,temp4->Pos_Y+temp4->Height-1))
      return temp4->Number;
  }

  // Test du click sur une dropdown
  for (temp5=Window_dropdown_button_list; temp5; temp5=temp5->Next)
  {
    if (Window_click_in_rectangle(temp5->Pos_X,temp5->Pos_Y,temp5->Pos_X+temp5->Width-1,temp5->Pos_Y+temp5->Height-1))
    {
      if (Mouse_K & temp5->Active_button)
        return Window_dropdown_on_click(temp5);
      else
      {
        Window_attribute2=-1;
        return Window_normal_button_onclick(temp5->Pos_X,temp5->Pos_Y,temp5->Width,temp5->Height,temp5->Number);
      }
    }
  }

  return 0;
}


short Window_get_button_shortcut(void)
{
  T_Normal_button * temp;

  if (Key & MOD_SHIFT)
    Window_attribute1=RIGHT_SIDE;
  else
    Window_attribute1=LEFT_SIDE;

  // On fait une première recherche
  temp=Window_normal_button_list;
  while (temp!=NULL)
  {
    if (temp->Shortcut==Key)
    {
      Hide_cursor();
      Window_select_normal_button(temp->Pos_X,temp->Pos_Y,temp->Width,temp->Height);
      Display_cursor();
      
      Slider_timer(Config.Delay_right_click_on_slider);
      
      Hide_cursor();
      Window_unselect_normal_button(temp->Pos_X,temp->Pos_Y,temp->Width,temp->Height);
      Display_cursor();

      return temp->Number;
    }
    temp=temp->Next;
  }

  // Si la recherche n'a pas été fructueuse ET que l'utilisateur appuyait sur
  // <Shift>, on regarde si un bouton ne pourrait pas réagir comme si <Shift>
  // n'était pas appuyé.
  if (Window_attribute1==RIGHT_SIDE)
  {
    temp=Window_normal_button_list;
    while (temp!=NULL)
    {
      if (temp->Shortcut==(Key&0x0FFF))
        return temp->Number;
      temp=temp->Next;
    }
  }

  return 0;
}

short Window_clicked_button(void)
{
  short Button;

  if(!Get_input())Wait_VBL();

  // Gestion des clicks
  if (Mouse_K)
  {
    if ((Mouse_X<Window_pos_X) || (Mouse_Y<Window_pos_Y)
     || (Mouse_X>=Window_pos_X+(Window_width*Menu_factor_X))
     || (Mouse_Y>=Window_pos_Y+(Window_height*Menu_factor_Y)))
      return -1;
    else
    {
      if (Mouse_Y < Window_pos_Y+(12*Menu_factor_Y))
        Move_window(Mouse_X-Window_pos_X,Mouse_Y-Window_pos_Y);
      else
        return Window_get_clicked_button();
    }
  }

  // Gestion des touches
  if (Key)
  {
    Button=Window_get_button_shortcut();
    if (Button)
    {
      Key=0;
      return Button;
    }
  }

  return 0;
}


// Fonction qui sert à remapper les parties sauvegardées derriere les
// fenetres ouvertes. C'est utilisé par exemple par la fenetre de palette
// Qui remappe des couleurs, afin de propager les changements.
void Remap_window_backgrounds(byte * conversion_table, int Min_Y, int Max_Y)
{
  int window_index; 
        byte* EDI;
        int dx,cx;

  for (window_index=0; window_index<Windows_open; window_index++)
  {
    EDI = Window_background[window_index];
  
        // Pour chaque ligne
        for(dx=0; dx<Window_stack_height[window_index]*Menu_factor_Y;dx++)
        {
          if (dx+Window_stack_pos_Y[window_index]>Max_Y)
            return;
          if (dx+Window_stack_pos_Y[window_index]<Min_Y)
          {
            EDI += Window_stack_width[window_index]*Menu_factor_X*Pixel_width;
          }
          else
                // Pour chaque pixel
                for(cx=Window_stack_width[window_index]*Menu_factor_X*Pixel_width;cx>0;cx--)
                {
                        *EDI = conversion_table[*EDI];
                        EDI ++;
                }
        }
  }
}
