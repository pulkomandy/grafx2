/* vim:expandtab:ts=2 sw=2:
*/
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
/// @file engine.c: Window engine and interface management
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
#include "pages.h"
#include "layers.h"


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


///Table of tooltip texts for menu buttons
char * Menu_tooltip[NB_BUTTONS]=
{
  "Hide toolbars / Select  ",

  "Layers manager          ",
  "Get/Set transparent col.",
  "Merge layer             ",
  "Add layer               ",
  "Drop layer              ",
  "Raise layer             ",
  "Lower layer             ",
  "Layer select / toggle   ",
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
  "Grad. rect / Grad. menu ",
  "Grad. spheres / ellipses",
  "Brush grab. / Restore   ",
  "Lasso / Restore brush   ",
#ifdef __ENABLE_LUA__
  "Brush effects / factory ",
#else
  "Brush effects           ",
#endif
  "Drawing modes (effects) ",
  "Text                    ",
  "Magnify mode / Menu     ",
  "Pipette / Invert colors ",
  "Screen size / Safe. res.",
  "Go / Copy to other page ",
  "Save as / Save          ",
  "Load / Re-load          ",
  "Settings / Skins        ",
  "Clear / with backcolor  ",
  "Help / Statistics       ",
  "Undo / Redo             ",
  "Kill current page       ",
  "Quit                    ",
  "Palette editor / setup  ",
  "Scroll pal. bkwd / Fast ",
  "Scroll pal. fwd / Fast  ",
  "Color #"                 ,
};

///Save a screen block (usually before erasing it with a new window or a dropdown menu)
void Save_background(byte **buffer, int x_pos, int y_pos, int width, int height)
{
  int index;
  if(*buffer != NULL) DEBUG("WARNING : buffer already allocated !!!",0);
  *buffer=(byte *) malloc(width*Menu_factor_X*height*Menu_factor_Y*Pixel_width);
  if(*buffer==NULL) Error(0);
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
  buffer = NULL;
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
  byte current_menu;
  byte first_button;

  x_pos = Mouse_X / Menu_factor_X;

  // Find in which menubar we are
  for (current_menu = 0; current_menu < MENUBAR_COUNT; current_menu ++)
  {
    if (Menu_bars[current_menu].Visible)
    {
      if (Mouse_Y >= Menu_Y+Menu_factor_Y*(Menu_bars[current_menu].Top) && 
      Mouse_Y < Menu_Y+Menu_factor_Y*(Menu_bars[current_menu].Top + Menu_bars[current_menu].Height))
        break;
    }
  }
  if (current_menu==MENUBAR_COUNT)
    return -1;
    
  y_pos=(Mouse_Y - Menu_Y)/Menu_factor_Y - Menu_bars[current_menu].Top;

  if (current_menu == 0) first_button = 0;
  else first_button = Menu_bars[current_menu - 1].Last_button_index + 1;

  for (btn_number=first_button;btn_number<=Menu_bars[current_menu].Last_button_index;btn_number++)
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


///Draw a menu button, selected or not
void Draw_menu_button(byte btn_number,byte pressed)
{
  word start_x;
  word start_y;
  word width;
  word height;
  byte * bitmap;
  word bitmap_width;
  word x_pos;
  word y_pos;
  byte current_menu;
  byte color;
  char icon;

  // Find in which menu the button is
  for (current_menu = 0; current_menu < MENUBAR_COUNT; current_menu++)
  {
    // We found the right bar !
    if (Menu_bars[current_menu].Last_button_index >= btn_number && 
    (current_menu==0 || Menu_bars[current_menu -1].Last_button_index < btn_number))
    {
      break;
    }
  }

  start_x = Buttons_Pool[btn_number].X_offset;
  start_y = Buttons_Pool[btn_number].Y_offset;
  width = Buttons_Pool[btn_number].Width+1;
  height = Buttons_Pool[btn_number].Height+1;
  icon = Buttons_Pool[btn_number].Icon;

  if (icon==-1)
  {
    // Standard button
    bitmap_width = Menu_bars[current_menu].Skin_width;
    bitmap=&(Menu_bars[current_menu].Skin[pressed][start_y*Menu_bars[current_menu].Skin_width+start_x]);
  }
  else
  {
    // From Menu_buttons
    bitmap_width = MENU_SPRITE_WIDTH;
    bitmap=Gfx->Menu_sprite[pressed][(byte)icon][0];
    // For bottom right: offset +1,+1
    if (Buttons_Pool[btn_number].Shape==BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT)
      bitmap += MENU_SPRITE_WIDTH+1;
  } 
     
  switch(Buttons_Pool[btn_number].Shape)
  {
    case BUTTON_SHAPE_NO_FRAME :
      break;
    case BUTTON_SHAPE_RECTANGLE  :
    for (y_pos=0;y_pos<height;y_pos++)
      for (x_pos=0;x_pos<width;x_pos++)
      {
        color=bitmap[x_pos+y_pos*bitmap_width];
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, start_y+y_pos, color);
      }
    break;
    case BUTTON_SHAPE_TRIANGLE_TOP_LEFT:
    for (y_pos=0;y_pos<15;y_pos++)
      for (x_pos=0;x_pos<15-y_pos;x_pos++)
      {
        color=bitmap[x_pos+y_pos*bitmap_width];
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, start_y+y_pos, color);
      }
    break;
    case BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT:
    for (y_pos=0;y_pos<15;y_pos++)
      for (x_pos=14-y_pos;x_pos<15;x_pos++)
      {
        color=bitmap[(x_pos)+(y_pos)*bitmap_width];
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, start_y+y_pos, color);
      }
    break;
  }
  // Special: Show specific shape
  if (btn_number==BUTTON_PAINTBRUSHES
    && Paintbrush_shape!=PAINTBRUSH_SHAPE_COLOR_BRUSH
    && Paintbrush_shape!=PAINTBRUSH_SHAPE_MONO_BRUSH)
  {
    short x_pos,y_pos;
    short start_x;
    short menu_x_pos,menu_y_pos;
    short menu_start_x;
  
    // Print paintbrush shape
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
        if (Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos])
        Pixel_in_menu_and_skin(MENUBAR_TOOLS, menu_x_pos, menu_y_pos, MC_Black);
      }
  }
  
  if (Menu_is_visible && Menu_bars[current_menu].Visible)
    Update_rect(Menu_factor_X*(Buttons_Pool[btn_number].X_offset),
      (Buttons_Pool[btn_number].Y_offset+Menu_bars[MENUBAR_TOOLS].Top)*Menu_factor_Y+Menu_Y,
      width*Menu_factor_X,height*Menu_factor_Y);

/*
  switch (pressed)
  {
    default:
    case BUTTON_RELEASED:
    color_top_left=MC_White;
    color_bottom_right=MC_Dark;
    color_diagonal=MC_Light;
    break;

    case BUTTON_PRESSED:
    color_top_left=MC_Dark;
    color_bottom_right=MC_Black;
    color_diagonal=MC_Black;
    break;    
  }

  switch(Buttons_Pool[btn_number].Shape)
  {
    case BUTTON_SHAPE_NO_FRAME :
      break;
    case BUTTON_SHAPE_RECTANGLE  :
      // On colorie le point haut droit
      Pixel_in_menu_and_skin(current_menu, end_x, start_y, color_diagonal);
      // On colorie le point bas gauche
      Pixel_in_menu_and_skin(current_menu, start_x, end_y, color_diagonal);
      // On colorie la partie haute
      for (x_pos=start_x;x_pos<=end_x-1;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, x_pos, start_y, color_top_left);
      }
      for (y_pos=start_y+1;y_pos<=end_y-1;y_pos++)
      {
        // On colorie la partie gauche
        Pixel_in_menu_and_skin(current_menu, start_x, y_pos, color_top_left);
        // On colorie la partie droite
        Pixel_in_menu_and_skin(current_menu, end_x, y_pos, color_bottom_right);
      }
      // On colorie la partie basse
      for (x_pos=start_x+1;x_pos<=end_x;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, x_pos, end_y, color_bottom_right);
      }
      break;
    case BUTTON_SHAPE_TRIANGLE_TOP_LEFT:
      // On colorie le point haut droit
      Pixel_in_menu_and_skin(current_menu, end_x, start_y, color_top_left);
      // On colorie le point bas gauche
      Pixel_in_menu_and_skin(current_menu, start_x, end_y, color_top_left);
      // On colorie le coin haut gauche
      for (x_pos=0;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, start_y, color_top_left);
        Pixel_in_menu_and_skin(current_menu, start_x, start_y+x_pos, color_top_left);
      }
      // On colorie la diagonale
      for (x_pos=1;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, end_y-x_pos, color_bottom_right);
      }
      break;
    case BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT:
      // On colorie le point haut droit
      Pixel_in_menu_and_skin(current_menu, end_x, start_y, color_diagonal);
      // On colorie le point bas gauche
      Pixel_in_menu_and_skin(current_menu, start_x, end_y, color_diagonal);
      // On colorie la diagonale
      for (x_pos=1;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, start_x+x_pos, end_y-x_pos, color_top_left);
      }
      // On colorie le coin bas droite
      for (x_pos=0;x_pos<Buttons_Pool[btn_number].Width;x_pos++)
      {
        Pixel_in_menu_and_skin(current_menu, end_x-x_pos, end_y, color_bottom_right);
        Pixel_in_menu_and_skin(current_menu, end_x, end_y-x_pos, color_bottom_right);
      }
  }
  if (Menu_is_visible && Menu_bars[current_menu].Visible)
  {
    Update_rect(
      start_x*Menu_factor_X,
      (start_y+Menu_bars[current_menu].Top)*Menu_factor_Y + Menu_Y,
      (end_x+1-start_x)*Menu_factor_X,
      (end_y+1-start_y)*Menu_factor_Y);
  }
  */
}


///Deselect a button
void Unselect_button(int btn_number)
{
  if (Buttons_Pool[btn_number].Pressed)
  {
    // On considère que le bouton est relâché
    Buttons_Pool[btn_number].Pressed=BUTTON_RELEASED;
    // On affiche le cadre autour du bouton de façon à ce qu'il paraisse relâché
    Draw_menu_button(btn_number,BUTTON_RELEASED);
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
      icon=MENU_SPRITE_POLYFORM;
      break;
    case BUTTON_FLOODFILL:
      icon=MENU_SPRITE_REPLACE;
      break;
    case BUTTON_CIRCLES:
    case BUTTON_FILLCIRC:
      icon=MENU_SPRITE_ELLIPSES;
      break;
    case BUTTON_SPHERES:
      icon=MENU_SPRITE_GRAD_ELLIPSE;
      break;
  }
  if (icon!=-1)
  {
    // This changes the sprite number of both halves of a split button
    Display_sprite_in_menu(btn_number,click==RIGHT_SIDE?icon:-1);

    // Redraw the other half if Unselect_button() won't do it.
    if (Buttons_Pool[btn_number].Shape==BUTTON_SHAPE_TRIANGLE_TOP_LEFT && 
      !Buttons_Pool[btn_number+1].Pressed)
      Draw_menu_button(btn_number+1, BUTTON_RELEASED);
    else if (Buttons_Pool[btn_number].Shape==BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT &&
      !Buttons_Pool[btn_number-1].Pressed)
      Draw_menu_button(btn_number-1, BUTTON_RELEASED);
  }

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
      // Same case with the Grad. Rectangle button.
      if (btn_number==BUTTON_GRADRECT && click==RIGHT_SIDE)
        break;
      // Pour chaque bouton:
      for (b=0; b<NB_BUTTONS; b++)
        // S'il est de la même famille
        if ( (b!=btn_number)
          && (Buttons_Pool[b].Family==family) )
          // Alors on désenclenche le bouton
          Unselect_button(b);
  }

  // On considère que le bouton est enfoncé
  Buttons_Pool[btn_number].Pressed=BUTTON_PRESSED;

  // On affiche le cadre autour du bouton de façon à ce qu'il paraisse enfoncé
  Draw_menu_button(btn_number, BUTTON_PRESSED);

  Display_cursor();

  if ((click==1 && !Buttons_Pool[btn_number].Left_instant)
    ||(click!=1 && !Buttons_Pool[btn_number].Right_instant))
  {
    // On attend ensuite que l'utilisateur lâche son bouton:
    Wait_end_of_click();
  }

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
    if(!Get_input())SDL_Delay(20);
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

///
/// Updates the status bar line with a color number.
/// Used when hovering the menu palette.
void Status_print_palette_color(byte color)
{
  char str[25];
  int i;
  
  strcpy(str,Menu_tooltip[BUTTON_CHOOSE_COL]);
  sprintf(str+strlen(str),"%d (%d,%d,%d)",color,Main_palette[color].R,Main_palette[color].G,Main_palette[color].B);
  // Pad spaces
  for (i=strlen(str); i<24; i++)
    str[i]=' ';
  str[24]='\0';
  
  Print_in_menu(str,0);
}

///Main handler for everything. This is the main loop of the program
void Main_handler(void)
{
  static byte temp_color;
  int  button_index;           // Numéro de bouton de menu en cours
  int  prev_button_number=0; // Numéro de bouton de menu sur lequel on était précédemment
  byte blink;                   // L'opération demande un effacement du curseur
  int  key_index;           // index du tableau de touches spéciales correspondant à la touche enfoncée
  byte temp;
  byte effect_modified;
  byte action;
  dword key_pressed;

  do
  {
    // Resize requested
    if (Resize_width || Resize_height)
    {
      Hide_cursor();
      Init_mode_video(Resize_width, Resize_height, 0, Pixel_ratio);
      // Reset the variables that indicate a resize was requested.
      Display_menu();
      Reposition_palette();
      Display_all_screen();
      Display_cursor();
    }
    
    if(Get_input())
    {
      action = 0;

      // Inhibit all keys if a drawing operation is in progress.
      // We make an exception for the freehand operations, but these will
      // only accept a very limited number of shortcuts.
      if (Operation_stack_size!=0 && !Allow_color_change_during_operation)
        Key=0;
      
      // Evenement de fermeture
      if (Quit_is_required)
      {
        Quit_is_required=0;
        Button_Quit();
      }
      
      if (Key)
      {
        effect_modified = 0;
        
        for (key_index=SPECIAL_CLICK_RIGHT+1;key_index<NB_SPECIAL_SHORTCUTS;key_index++)
        {
          if (Is_shortcut(Key,key_index))
          {
            // Special keys (functions not hooked to a UI button)
            switch(key_index)
            {
              case SPECIAL_NEXT_FORECOLOR : // Next foreground color
                Special_next_forecolor();
                action++;
                break;
              case SPECIAL_PREVIOUS_FORECOLOR : // Previous foreground color
                Special_previous_forecolor();
                action++;
                break;
              case SPECIAL_NEXT_BACKCOLOR : // Next background color
                Special_next_backcolor();
                action++;
                break;
              case SPECIAL_PREVIOUS_BACKCOLOR : // Previous background color
                Special_previous_backcolor();
                action++;
                break;
              case SPECIAL_SMALLER_PAINTBRUSH: // Rétrécir le pinceau
                Smaller_paintbrush();
                action++;
                break;
              case SPECIAL_BIGGER_PAINTBRUSH: // Grossir le pinceau
                Bigger_paintbrush();
                action++;
                break;
              case SPECIAL_NEXT_USER_FORECOLOR : // Next user-defined foreground color
                Special_next_user_forecolor();
                action++;
                break;
              case SPECIAL_PREVIOUS_USER_FORECOLOR : // Previous user-defined foreground color
                Special_previous_user_forecolor();
                action++;
                break;
              case SPECIAL_NEXT_USER_BACKCOLOR : // Next user-defined background color
                Special_next_user_backcolor();
                action++;
                break;
              case SPECIAL_PREVIOUS_USER_BACKCOLOR : // Previous user-defined background color
                Special_previous_user_backcolor();
                action++;
                break;
            }
            
            // Other shortcuts are forbidden while an operation is in progress
            if (Operation_stack_size!=0)
              continue;
            
            switch (key_index)
            {
              case SPECIAL_SCROLL_UP : // Scroll up
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,-(Main_magnifier_height>>2));
                else
                  Scroll_screen(0,-(Screen_height>>3));
                action++;
                break;
              case SPECIAL_SCROLL_DOWN : // Scroll down
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,(Main_magnifier_height>>2));
                else
                  Scroll_screen(0,(Screen_height>>3));
                action++;
                break;
              case SPECIAL_SCROLL_LEFT : // Scroll left
                if (Main_magnifier_mode)
                  Scroll_magnifier(-(Main_magnifier_width>>2),0);
                else
                  Scroll_screen(-(Screen_width>>3),0);
                action++;
                break;
              case SPECIAL_SCROLL_RIGHT : // Scroll right
                if (Main_magnifier_mode)
                  Scroll_magnifier((Main_magnifier_width>>2),0);
                else
                  Scroll_screen((Screen_width>>3),0);
                action++;
                break;
              case SPECIAL_SCROLL_UP_FAST : // Scroll up faster
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,-(Main_magnifier_height>>1));
                else
                  Scroll_screen(0,-(Screen_height>>2));
                action++;
                break;
              case SPECIAL_SCROLL_DOWN_FAST : // Scroll down faster
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,(Main_magnifier_height>>1));
                else
                  Scroll_screen(0,(Screen_height>>2));
                action++;
                break;
              case SPECIAL_SCROLL_LEFT_FAST : // Scroll left faster
                if (Main_magnifier_mode)
                  Scroll_magnifier(-(Main_magnifier_width>>1),0);
                else
                  Scroll_screen(-(Screen_width>>2),0);
                action++;
                break;
              case SPECIAL_SCROLL_RIGHT_FAST : // Scroll right faster
                if (Main_magnifier_mode)
                  Scroll_magnifier((Main_magnifier_width>>1),0);
                else
                  Scroll_screen((Screen_width>>2),0);
                action++;
                break;
              case SPECIAL_SCROLL_UP_SLOW : // Scroll up slower
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,-1);
                else
                  Scroll_screen(0,-1);
                action++;
                break;
              case SPECIAL_SCROLL_DOWN_SLOW : // Scroll down slower
                if (Main_magnifier_mode)
                  Scroll_magnifier(0,1);
                else
                  Scroll_screen(0,1);
                action++;
                break;
              case SPECIAL_SCROLL_LEFT_SLOW : // Scroll left slower
                if (Main_magnifier_mode)
                  Scroll_magnifier(-1,0);
                else
                  Scroll_screen(-1,0);
                action++;
                break;
              case SPECIAL_SCROLL_RIGHT_SLOW : // Scroll right slower
                if (Main_magnifier_mode)
                  Scroll_magnifier(1,0);
                else
                  Scroll_screen(1,0);
                action++;
                break;
              case SPECIAL_SHOW_HIDE_CURSOR : // Show / Hide cursor
                Hide_cursor();
                Cursor_hidden=!Cursor_hidden;
                Display_cursor();
                action++;
                break;
              case SPECIAL_DOT_PAINTBRUSH : // Paintbrush = "."
                Hide_cursor();
                Paintbrush_shape=PAINTBRUSH_SHAPE_ROUND;
                Set_paintbrush_size(1,1);
                Change_paintbrush_shape(PAINTBRUSH_SHAPE_ROUND);
                Display_cursor();
                action++;
                break;
              case SPECIAL_CONTINUOUS_DRAW : // Continuous freehand drawing
                Select_button(BUTTON_DRAW,LEFT_SIDE);
                // ATTENTION CE TRUC EST MOCHE ET VA MERDER SI ON SE MET A UTILISER DES BOUTONS POPUPS
                while (Current_operation!=OPERATION_CONTINUOUS_DRAW)
                  Select_button(BUTTON_DRAW,RIGHT_SIDE);
                action++;
                break;
              case SPECIAL_FLIP_X : // Flip X
                Hide_cursor();
                Flip_X_lowlevel(Brush, Brush_width, Brush_height);
                Display_cursor();
                action++;
                break;
              case SPECIAL_FLIP_Y : // Flip Y
                Hide_cursor();
                Flip_Y_lowlevel(Brush, Brush_width, Brush_height);
                Display_cursor();
                action++;
                break;
              case SPECIAL_ROTATE_90 : // 90° brush rotation
                Hide_cursor();
                Rotate_90_deg();
                Display_cursor();
                action++;
                break;
              case SPECIAL_ROTATE_180 : // 180° brush rotation
                Hide_cursor();
                Rotate_180_deg_lowlevel(Brush, Brush_width, Brush_height);
                Brush_offset_X=(Brush_width>>1);
                Brush_offset_Y=(Brush_height>>1);
                Display_cursor();
                action++;
                break;
              case SPECIAL_STRETCH : // Stretch brush
                Hide_cursor();
                Start_operation_stack(OPERATION_STRETCH_BRUSH);
                Display_cursor();
                action++;
                break;
              case SPECIAL_DISTORT : // Distort brush
                Hide_cursor();
                Start_operation_stack(OPERATION_DISTORT_BRUSH);
                Display_cursor();
                action++;
                break;
              case SPECIAL_ROTATE_ANY_ANGLE : // Rotate brush by any angle
                Hide_cursor();
                Start_operation_stack(OPERATION_ROTATE_BRUSH);
                Display_cursor();
                action++;
                break;
              case SPECIAL_OUTLINE : // Outline brush
                Hide_cursor();
                Outline_brush();
                Display_cursor();
                action++;
                break;
              case SPECIAL_NIBBLE : // Nibble brush
                Hide_cursor();
                Nibble_brush();
                Display_cursor();
                action++;
                break;
              case SPECIAL_GET_BRUSH_COLORS : // Get colors from brush
                Get_colors_from_brush();
                action++;
                break;
              case SPECIAL_RECOLORIZE_BRUSH : // Recolorize brush
                Hide_cursor();
                Remap_brush();
                Display_cursor();
                action++;
                break;
              case SPECIAL_LOAD_BRUSH :
                Load_picture(0);
                action++;
                break;
              case SPECIAL_SAVE_BRUSH :
                Save_picture(0);
                action++;
                break;
              case SPECIAL_ZOOM_IN : // Zoom in
                Zoom(+1);
                action++;
                break;
              case SPECIAL_ZOOM_OUT : // Zoom out
                Zoom(-1);
                action++;
                break;
              case SPECIAL_CENTER_ATTACHMENT : // Center brush attachment
                Hide_cursor();
                Brush_offset_X=(Brush_width>>1);
                Brush_offset_Y=(Brush_height>>1);
                Display_cursor();
                action++;
                break;
              case SPECIAL_TOP_LEFT_ATTACHMENT : // Top-left brush attachment
                Hide_cursor();
                Brush_offset_X=0;
                Brush_offset_Y=0;
                Display_cursor();
                action++;
                break;
              case SPECIAL_TOP_RIGHT_ATTACHMENT : // Top-right brush attachment
                Hide_cursor();
                Brush_offset_X=(Brush_width-1);
                Brush_offset_Y=0;
                Display_cursor();
                action++;
                break;
              case SPECIAL_BOTTOM_LEFT_ATTACHMENT : // Bottom-left brush attachment
                Hide_cursor();
                Brush_offset_X=0;
                Brush_offset_Y=(Brush_height-1);
                Display_cursor();
                action++;
                break;
              case SPECIAL_BOTTOM_RIGHT_ATTACHMENT : // Bottom right brush attachment
                Hide_cursor();
                Brush_offset_X=(Brush_width-1);
                Brush_offset_Y=(Brush_height-1);
                Display_cursor();
                action++;
                break;
              case SPECIAL_EXCLUDE_COLORS_MENU : // Exclude colors menu
                Menu_tag_colors("Tag colors to exclude",Exclude_color,&temp,1, NULL, SPECIAL_EXCLUDE_COLORS_MENU);
                action++;
                break;
              case SPECIAL_INVERT_SIEVE :
                Invert_trame();
                action++;
                break;
              case SPECIAL_SHADE_MODE :
                Button_Shade_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SHADE_MENU :
                Button_Shade_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_QUICK_SHADE_MODE :
                Button_Quick_shade_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_QUICK_SHADE_MENU :
                Button_Quick_shade_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_STENCIL_MODE :
                Button_Stencil_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_STENCIL_MENU :
                Button_Stencil_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_MASK_MODE :
                Button_Mask_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_MASK_MENU :
                Button_Mask_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_GRID_MODE :
                Button_Snap_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_GRID_MENU :
                Button_Grid_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SHOW_GRID :
                Button_Show_grid();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SIEVE_MODE :
                Button_Sieve_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SIEVE_MENU :
                Button_Sieve_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_COLORIZE_MODE :
                Button_Colorize_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_COLORIZE_MENU :
                Button_Colorize_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SMOOTH_MODE :
                Button_Smooth_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SMOOTH_MENU :
                Button_Smooth_menu();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_SMEAR_MODE :
                Button_Smear_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TILING_MODE :
                Button_Tiling_mode();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TILING_MENU :
                effect_modified = 1;
                Button_Tiling_menu();
                action++;
                break;
              case SPECIAL_EFFECTS_OFF :
                Effects_off();
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_1 :
                Transparency_set(1);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_2 :
                Transparency_set(2);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_3 :
                Transparency_set(3);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_4 :
                Transparency_set(4);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_5 :
                Transparency_set(5);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_6 :
                Transparency_set(6);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_7 :
                Transparency_set(7);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_8 :
                Transparency_set(8);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_9 :
                Transparency_set(9);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_TRANSPARENCY_0 :
                Transparency_set(0);
                effect_modified = 1;
                action++;
                break;
              case SPECIAL_ZOOM_1 :
                Zoom_set(-1);
                action++;
                break;
              case SPECIAL_ZOOM_2 :
                Zoom_set(0);
                action++;
                break;
              case SPECIAL_ZOOM_3 :
                Zoom_set(1);
                action++;
                break;
              case SPECIAL_ZOOM_4 :
                Zoom_set(2);
                action++;
                break;
              case SPECIAL_ZOOM_5 :
                Zoom_set(3);
                action++;
                break;
              case SPECIAL_ZOOM_6 :
                Zoom_set(4);
                action++;
                break;
              case SPECIAL_ZOOM_8 :
                Zoom_set(5);
                action++;
                break;
              case SPECIAL_ZOOM_10 :
                Zoom_set(6);
                action++;
                break;
              case SPECIAL_ZOOM_12 :
                Zoom_set(7);
                action++;
                break;
              case SPECIAL_ZOOM_14 :
                Zoom_set(8);
                action++;
                break;
              case SPECIAL_ZOOM_16 :
                Zoom_set(9);
                action++;
                break;
              case SPECIAL_ZOOM_18 :
                Zoom_set(10);
                action++;
                break;
              case SPECIAL_ZOOM_20 :
                Zoom_set(11);
                action++;
                break;
              case SPECIAL_LAYER1_SELECT:
              case SPECIAL_LAYER2_SELECT:
              case SPECIAL_LAYER3_SELECT:
              case SPECIAL_LAYER4_SELECT:
              case SPECIAL_LAYER5_SELECT:
              case SPECIAL_LAYER6_SELECT:
              case SPECIAL_LAYER7_SELECT:
              case SPECIAL_LAYER8_SELECT:
                Layer_activate((key_index-SPECIAL_LAYER1_SELECT)/2, LEFT_SIDE);
                action++;
                break;
              case SPECIAL_LAYER1_TOGGLE:
              case SPECIAL_LAYER2_TOGGLE:
              case SPECIAL_LAYER3_TOGGLE:
              case SPECIAL_LAYER4_TOGGLE:
              case SPECIAL_LAYER5_TOGGLE:
              case SPECIAL_LAYER6_TOGGLE:
              case SPECIAL_LAYER7_TOGGLE:
              case SPECIAL_LAYER8_TOGGLE:
                Layer_activate((key_index-SPECIAL_LAYER1_TOGGLE)/2, RIGHT_SIDE);
                action++;
                break;
            }
          }
        } // End of special keys
        
        
        // Shortcut for clicks of Menu buttons.
        // Disable all of them when an operation is in progress
        if (Operation_stack_size==0)
        {
          // Some functions open windows that clear the Key variable, 
          // so we need to use a temporary replacement.
          key_pressed = Key;
          for (button_index=0;button_index<NB_BUTTONS;button_index++)
          {
            if (Is_shortcut(key_pressed,0x100+button_index))
            {          
              Select_button(button_index,LEFT_SIDE);
              prev_button_number=-1;
              action++;
            }
            else if (Is_shortcut(key_pressed,0x200+button_index))
            {
              Select_button(button_index,RIGHT_SIDE);
              prev_button_number=-1;
              action++;
            }
          }
        }
  
        // Si on a modifié un effet, il faut rafficher le bouton des effets en
        // conséquence.
        if (effect_modified)
        {
          Hide_cursor();
          Draw_menu_button(BUTTON_EFFECTS,
            (Shade_mode||Quick_shade_mode||Colorize_mode||Smooth_mode||Tiling_mode||Smear_mode||Stencil_mode||Mask_mode||Sieve_mode||Snap_mode));
          Display_cursor();
        }
      }
      if (action)
        Key=0;
    }
    else
    {
      // No event : we go asleep for a while, but we try to get waked up at constant intervals of time
      // no matter the machine speed or system load. The time is fixed to 10ms (that should be about a cpu slice on most systems)
      // This allows nice smooth mouse movement.
        const int delay = 10;
  
      Uint32 debut;
      debut = SDL_GetTicks();
      // Première attente : le complément de "delay" millisecondes
      SDL_Delay(delay - (debut % delay));
      // Si ça ne suffit pas, on complète par des attentes successives de "1ms".
      // (Remarque, Windows arrondit généralement aux 10ms supérieures)
      while ( SDL_GetTicks()/delay <= debut/delay)
      {
        SDL_Delay(1);
      }
    }

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
            
            /*if (Gfx->Hover_effect && prev_button_number > -1 && !Buttons_Pool[prev_button_number].Pressed)
              Draw_menu_button(prev_button_number, BUTTON_RELEASED);
            */

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

              /*if (Gfx->Hover_effect && prev_button_number > -1 && !Buttons_Pool[prev_button_number].Pressed)
                Draw_menu_button(prev_button_number, BUTTON_RELEASED);
              */
              
              Print_in_menu(Menu_tooltip[button_index],0);

              /*if (Gfx->Hover_effect && !Buttons_Pool[button_index].Pressed)
                Draw_menu_button(button_index, BUTTON_HIGHLIGHTED);
              */
              
              Display_cursor();
            }
            else
            { // Le curseur est-il sur une couleur de la palette?
              int color;
              if ((color=Pick_color_in_palette())!=-1)
              {
                Hide_cursor();
                Status_print_palette_color(color);
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
      if(Cursor_in_menu_previous)
      {
        Hide_cursor();
        
        /*if (Gfx->Hover_effect && prev_button_number > -1 && !Buttons_Pool[prev_button_number].Pressed)
          Draw_menu_button(prev_button_number, BUTTON_RELEASED);
        */
                
        if ( (Current_operation!=OPERATION_COLORPICK) && (Current_operation!=OPERATION_REPLACE) )
        {
          Print_in_menu("X:       Y:             ",0);
        }
        else
        {
          Print_in_menu("X:       Y:       (    )",0);
        }
        
        Display_cursor();
        
        Cursor_in_menu_previous = 0;
      }
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

void Open_window(word width,word height, const char * title)
// Lors de l'appel à cette procédure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  //word i,j;
  size_t title_length;

  Hide_cursor();
  
  /*if (Windows_open == 0 && Gfx->Hover_effect)
  {
    if (Cursor_in_menu)
    {
      int button_index=Button_under_mouse();
      if (button_index > -1 && !Buttons_Pool[button_index].Pressed)
              Draw_menu_button(button_index, BUTTON_RELEASED);
    }
  }*/
    
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

  title_length = strlen(title);
  if (title_length+2 > width/8)
    title_length = width/8-2;
  Print_in_window_limited((width-(title_length<<3))>>1,3,title,title_length,MC_Black,MC_Light);

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
  T_List_button     * temp6;

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
  while (Window_list_button_list)
  {
    temp6=Window_list_button_list->Next;
    free(Window_list_button_list);
    Window_list_button_list=temp6;
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

T_Dropdown_button * Window_set_dropdown_button(word x_pos,word y_pos,word width,word height,word dropdown_width,const char *label,byte display_choice,byte display_centered,byte display_arrow,byte active_button, byte bottom_up)
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
  temp->Bottom_up=bottom_up;

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

//----------------------- Create a List control -----------------------
// These controls are special. They work over two controls previously created:
// - entry_button is the textual area where the list values will be printed.
// - scroller is a scroller button attached to it

T_List_button * Window_set_list_button(T_Special_button * entry_button, T_Scroller_button * scroller, Func_draw_list_item draw_list_item)
{
  T_List_button *temp;

  temp=(T_List_button *)malloc(sizeof(T_List_button));
  temp->Number          =++Window_nb_buttons;
  temp->List_start      = 0;
  temp->Cursor_position = 0;
  temp->Entry_button    = entry_button;
  temp->Scroller        = scroller;
  temp->Draw_list_item  = draw_list_item;

  temp->Next=Window_list_button_list;
  Window_list_button_list=temp;
  return temp;
}

void Window_redraw_list(T_List_button * list)
{
  int i;

  for (i=Min(list->Scroller->Nb_visibles-1, list->Scroller->Nb_elements-1); i>=0; i--)
  {
    list->Draw_list_item(
      list->Entry_button->Pos_X,
      list->Entry_button->Pos_Y + i * 8,
      list->List_start + i,
      i == list->Cursor_position);
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
  T_Dropdown_button * temp5;
  T_List_button     * temp6;

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
  while (Window_list_button_list)
  {
    temp6=Window_list_button_list->Next;
    free(Window_list_button_list);
    Window_list_button_list=temp6;
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
    Window_background[Windows_open-1] = NULL;
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
    while(!Get_input())SDL_Delay(20);

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
  short old_x=-1;
  short old_y=-1;
  short index;
  short a,b,c,d; // Variables temporaires et multitâches...
  byte * buffer = NULL;
  char str[25];
  byte cursor_was_hidden;


  Hide_cursor();

  cursor_was_hidden=Cursor_hidden;
  Cursor_hidden=0;

  Save_background(&buffer,Window_pos_X,Window_pos_Y,Window_width,Window_height);
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
    if(!Get_input())SDL_Delay(20);

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

  Restore_background(buffer,Window_pos_X,Window_pos_Y,Window_width,Window_height);
  Update_rect(Window_pos_X, Window_pos_Y, Window_width*Menu_factor_X, Window_height*Menu_factor_Y);
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Paintbrush_hidden=b;
  Cursor_hidden=cursor_was_hidden;
  Display_cursor();
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

    while(!Get_input() && new_x==Mouse_X-dx && new_y==Mouse_Y-dy) SDL_Delay(20);

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

///
/// Displays a dropped-down menu and handles the UI logic until the user
/// releases a mouse button.
/// This function then clears the dropdown and returns the selected item,
/// or NULL if the user wasn't highlighting an item when he closed.
T_Dropdown_choice * Dropdown_activate(T_Dropdown_button *button, short off_x, short off_y)
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
  for (item=button->First_item; item!=NULL; item=item->Next)
  {
    nb_choices++;
  }
  box_height=3+nb_choices*8+1;

  // Open a new stacked "window" to serve as drawing area.
  Open_popup(
    off_x+(button->Pos_X)*Menu_factor_X,
    off_y+(button->Pos_Y+(button->Bottom_up?-box_height:button->Height))*Menu_factor_Y,
    button->Dropdown_width+SHADOW_RIGHT,
    box_height+SHADOW_BOTTOM);

  // Dessin de la boite

  // Bord gauche
  Block(Window_pos_X,Window_pos_Y,Menu_factor_X,box_height*Menu_factor_Y,MC_Black);
  // Frame fonce et blanc
  Window_display_frame_out(1,0,button->Dropdown_width-1,box_height);
  // Ombre portée
  if (SHADOW_BOTTOM)
  {
    Block(Window_pos_X+SHADOW_RIGHT*Menu_factor_X,
        Window_pos_Y+box_height*Menu_factor_Y,
        button->Dropdown_width*Menu_factor_X,
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
    Block(Window_pos_X+button->Dropdown_width*Menu_factor_X,
        Window_pos_Y+SHADOW_BOTTOM*Menu_factor_Y,
        SHADOW_RIGHT*Menu_factor_X,
        (box_height-SHADOW_BOTTOM)*Menu_factor_Y,
        MC_Black);
    Block(Window_pos_X+button->Dropdown_width*Menu_factor_X,
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
        (button->Dropdown_width-3)*Menu_factor_X,(box_height-2)*Menu_factor_Y,MC_Light);
    // Affichage des items
    for(item=button->First_item,choice_index=0; item!=NULL; item=item->Next,choice_index++)
    {
      byte color_1;
      byte color_2;
      if (choice_index==selected_index)
      {
        color_1=MC_White;
        color_2=MC_Dark;
        Block(Window_pos_X+3*Menu_factor_X,
        Window_pos_Y+((2+choice_index*8)*Menu_factor_Y),
        (button->Dropdown_width-5)*Menu_factor_X,(8)*Menu_factor_Y,MC_Dark);
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
      if(!Get_input()) SDL_Delay(20);
      // Mise à jour du survol
      selected_index=Window_click_in_rectangle(2,2,button->Dropdown_width-2,box_height-1)?
        (((Mouse_Y-Window_pos_Y)/Menu_factor_Y-2)>>3) : -1;

    } while (Mouse_K && selected_index==old_selected_index);
    
    if (!Mouse_K)
      break;
    Hide_cursor();
  }

  Close_popup();  

  if (selected_index>=0 && selected_index<nb_choices)
  {
    for(item=button->First_item; selected_index; item=item->Next,selected_index--)
      ;
    return item;
  }
  return NULL;
}

// Gestion des dropdown
short Window_dropdown_on_click(T_Dropdown_button *button)
{
  T_Dropdown_choice * item;
  
  // Highlight the button
  Hide_cursor();
  Window_select_normal_button(button->Pos_X,button->Pos_Y,button->Width,button->Height);
        
  // Handle the dropdown's logic
  item = Dropdown_activate(button, Window_pos_X, Window_pos_Y);
  
  // Unhighlight the button
  Window_unselect_normal_button(button->Pos_X,button->Pos_Y,button->Width,button->Height);
  Display_cursor();

  if (item == NULL)
  {
    Window_attribute2=-1;
    return 0;
  }

  if (button->Display_choice)
  {
    // Automatically update the label of the dropdown list.
    int text_length = (button->Width-4-(button->Display_arrow?8:0))/8;
    // Clear original label area
    Window_rectangle(button->Pos_X+2,button->Pos_Y+(button->Height-7)/2,text_length*8,8,MC_Light);
    Print_in_window_limited(button->Pos_X+2,button->Pos_Y+(button->Height-7)/2,item->Label,text_length ,MC_Black,MC_Light);
  } 

  Window_attribute2=item->Number;
  return button->Number;
  
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
      if(!Get_input()) SDL_Delay(20);
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
      if(!Get_input()) SDL_Delay(20);
      if (!Mouse_K)
        return 0;
    }
  }
}

// --- Returns the number of the clicked button (-1:out of the window, 0:none) ---
short Window_get_clicked_button(void)
{
  T_Normal_button   * temp1;
  T_Palette_button  * temp2;
  T_Scroller_button * temp3;
  T_Special_button  * temp4;
  T_Dropdown_button * temp5;

  long max_slider_height;

  Window_attribute1=Mouse_K;

  // Test click on normal buttons
  for (temp1=Window_normal_button_list; temp1; temp1=temp1->Next)
  {
    if ((Input_sticky_control == 0 || Input_sticky_control == temp1->Number)
      && Window_click_in_rectangle(temp1->Pos_X,temp1->Pos_Y,temp1->Pos_X+temp1->Width-1,temp1->Pos_Y+temp1->Height-1))
    {
      Input_sticky_control = temp1->Number;
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

  // Test click on "Palette" buttons
  for (temp2=Window_palette_button_list; temp2; temp2=temp2->Next)
  {
    if ((Input_sticky_control == 0 || Input_sticky_control == temp2->Number)
      && Window_click_in_rectangle(temp2->Pos_X+5,temp2->Pos_Y+3,temp2->Pos_X+160,temp2->Pos_Y+82))
    {
      Input_sticky_control = temp2->Number;
      // We store the clicked color in Attribute2
      Window_attribute2 = (((Mouse_X-Window_pos_X)/Menu_factor_X)-(temp2->Pos_X+2)) / 10 * 16 +
        (((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-(temp2->Pos_Y+3)) / 5;
        return temp2->Number;
    }
  }

  // Test click oin slider/scroller bars
  for (temp3=Window_scroller_button_list; temp3; temp3=temp3->Next)
  {
    // Button Up arrow
    if ((Input_sticky_control == 0 || Input_sticky_control == (temp3->Number|1024))
      && Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y,temp3->Pos_X+10,temp3->Pos_Y+10))
    {
      Input_sticky_control = temp3->Number | 1024;
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
      
      return (Window_attribute1)? temp3->Number : 0;
    }
    
    // Button Down arrow
    if ((Input_sticky_control == 0 || Input_sticky_control == (temp3->Number|2048))
      && Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y+temp3->Height-11,temp3->Pos_X+10,temp3->Pos_Y+temp3->Height-1))
    {
      Input_sticky_control = temp3->Number | 2048;
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
      
      return (Window_attribute1)? temp3->Number : 0;
    }
    // Middle slider
    if ((Input_sticky_control == temp3->Number) || (Input_sticky_control==0 &&
        Window_click_in_rectangle(temp3->Pos_X,temp3->Pos_Y+12,temp3->Pos_X+10,temp3->Pos_Y+temp3->Height-13)))
    {
      Input_sticky_control = temp3->Number;
      if (temp3->Nb_elements>temp3->Nb_visibles)
      {
        // If there is enough room to make the cursor move:

        max_slider_height=(temp3->Height-24);

        // Window_attribute2 receives the position of the cursor.
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

        // If the cursor moved

        if (temp3->Position!=Window_attribute2)
        {
          temp3->Position=Window_attribute2;
          Window_attribute1=3;
          Hide_cursor();
          Window_draw_slider(temp3);
          Display_cursor();
        }
        else
          // If the cursor moved
          Window_attribute1=0;
      }
      else
        // If there's not enough room to make the cursor move:
        Window_attribute1=0;
        
      return (Window_attribute1)? temp3->Number : 0; 
    }
  }

  // Test click on a special button
  for (temp4=Window_special_button_list; temp4; temp4=temp4->Next)
  {
    if ((Input_sticky_control == 0 || Input_sticky_control == temp4->Number)
      && Window_click_in_rectangle(temp4->Pos_X,temp4->Pos_Y,temp4->Pos_X+temp4->Width-1,temp4->Pos_Y+temp4->Height-1))
    {
       Input_sticky_control = temp4->Number;
       return temp4->Number;
    }
  }

  // Test click on a dropdown box
  for (temp5=Window_dropdown_button_list; temp5; temp5=temp5->Next)
  {
    if ((Input_sticky_control == 0 || Input_sticky_control == temp5->Number)
     && Window_click_in_rectangle(temp5->Pos_X,temp5->Pos_Y,temp5->Pos_X+temp5->Width-1,temp5->Pos_Y+temp5->Height-1))
    {
      Input_sticky_control = temp5->Number;
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
  
  // Handle arrow keys, end/home, and mouse wheel that have
  // a certain behavior if a list control is present.
  if (Window_list_button_list)
  {
    T_List_button *list = Window_list_button_list;
    // If there's more than one of such control, only capture
    // events if the mouse cursor is over it.
    if (list->Next)
    {
      // to do
    }
  
  
  
  
  
  
  }
  return 0;
}

short Window_clicked_button(void)
{
  short Button;

  if(!Get_input())SDL_Delay(20);

  // Handle clicks
  if (Mouse_K)
  {
    if ((Mouse_X<Window_pos_X) || (Mouse_Y<Window_pos_Y)
     || (Mouse_X>=Window_pos_X+(Window_width*Menu_factor_X))
     || (Mouse_Y>=Window_pos_Y+(Window_height*Menu_factor_Y)))
    {
      if (Input_sticky_control == 0 || Input_sticky_control == -1)
      {
        Input_sticky_control = -1;
        return -1;
      }
      else
      {
        return 0;
      }
    }
     
    if (!Input_sticky_control && Mouse_Y < Window_pos_Y+(12*Menu_factor_Y))
    {
      Move_window(Mouse_X-Window_pos_X,Mouse_Y-Window_pos_Y);
    }
    else
    {
      short clicked_button;
      T_List_button * list;
      
      // Check which controls was clicked (by rectangular area)
      clicked_button = Window_get_clicked_button();
      
      // Check if it's part of a list control
      for (list=Window_list_button_list; list!=NULL; list=list->Next)
      {
        if (list->Entry_button->Number == clicked_button)
        {
          // Click in the textual part of a list.
          short clicked_line;            
          clicked_line = (((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-list->Entry_button->Pos_Y)>>3;
          if (clicked_line == list->Cursor_position ||   // Same as before
            clicked_line >= list->Scroller->Nb_elements) // Below last line              
            return 0;
          
          Hide_cursor();
          // Redraw one item as disabled
          if (list->Cursor_position>=0 && list->Cursor_position<list->Scroller->Nb_visibles)
            list->Draw_list_item(
              list->Entry_button->Pos_X,
              list->Entry_button->Pos_Y + list->Cursor_position * 8,
              list->List_start + list->Cursor_position,
              0);
          list->Cursor_position = clicked_line;
          // Redraw one item as enabled
          if (list->Cursor_position>=0 && list->Cursor_position<list->Scroller->Nb_visibles)
            list->Draw_list_item(
              list->Entry_button->Pos_X,
              list->Entry_button->Pos_Y + list->Cursor_position * 8,
              list->List_start + list->Cursor_position,
              1);
          Display_cursor();

          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        else if (list->Scroller->Number == clicked_button)
        {
          // Click in the scroller part of a list
          if (list->List_start == list->Scroller->Position)
            return 0; // Didn't actually move
          // Update scroller indices
          list->Cursor_position += list->List_start;
          list->List_start = list->Scroller->Position;
          list->Cursor_position -= list->List_start;
          // Need to redraw all
          Hide_cursor();
          Window_redraw_list(list);
          Display_cursor();
        }
      }
      return clicked_button;
    }
  }

  // Intercept keys
  if (Key)
  {
    T_List_button * list;

    Button=Window_get_button_shortcut();
    if (Button)
    {
      Key=0;
      return Button;
    }
    // Check if there's a list control and the keys can control it
    for (list=Window_list_button_list; list!=NULL; list=list->Next)
    {
      // FIXME: Make only one list have the keyboard focus.
      if (1)
      {
        if (Key==SDLK_UP && (list->Cursor_position+list->List_start)>0)
        {
          Key=0;
          Hide_cursor();
          list->Cursor_position--;
          if (list->Cursor_position<0)
          {
            list->List_start=list->List_start+list->Cursor_position;
            list->Cursor_position=0;
            // Mise à jour du scroller
            list->Scroller->Position=list->List_start;
            Window_draw_slider(list->Scroller);
          }
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key==SDLK_DOWN && (list->Cursor_position+list->List_start)<(list->Scroller->Nb_elements-1))
        {
          Key=0;
          Hide_cursor();
          list->Cursor_position++;
          if (list->Cursor_position>(list->Scroller->Nb_visibles-1))
          {
            list->List_start=list->List_start+list->Cursor_position-(list->Scroller->Nb_visibles-1);
            list->Cursor_position=(list->Scroller->Nb_visibles-1);
            // Mise à jour du scroller
            list->Scroller->Position=list->List_start;
            Window_draw_slider(list->Scroller);
          }
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key==SDLK_HOME && (list->Cursor_position!=0 || list->List_start!=0))
        {
          Key=0;
          Hide_cursor();
          list->Cursor_position=0;
          list->List_start=0;
          // Mise à jour du scroller
          list->Scroller->Position=list->List_start;
          Window_draw_slider(list->Scroller);
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key==SDLK_END && (list->Cursor_position+list->List_start)<(list->Scroller->Nb_elements-1))
        {
          Key=0;
          Hide_cursor();
          list->Cursor_position=(list->Scroller->Nb_elements-1)-list->List_start;
          if (list->Cursor_position>(list->Scroller->Nb_visibles-1))
          {
            list->List_start=list->List_start+list->Cursor_position-(list->Scroller->Nb_visibles-1);
            list->Cursor_position=(list->Scroller->Nb_visibles-1);
            // Mise à jour du scroller
            list->Scroller->Position=list->List_start;
            Window_draw_slider(list->Scroller);
          }
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key==SDLK_PAGEDOWN && (list->Cursor_position+list->List_start)<(list->Scroller->Nb_elements-1))
        {
          Key=0;
          Hide_cursor();
          if (list->Scroller->Nb_elements<list->Scroller->Nb_visibles)
          {
            list->Cursor_position=list->Scroller->Nb_elements-1;
          }
          else if(list->Cursor_position!=list->Scroller->Nb_visibles-1)
          {
            list->Cursor_position=list->Scroller->Nb_visibles-1;
          }
          else
          {
            list->List_start+=list->Scroller->Nb_visibles;
            if (list->List_start+list->Scroller->Nb_visibles>list->Scroller->Nb_elements)
            {
              list->List_start=list->Scroller->Nb_elements-list->Scroller->Nb_visibles;
            }
            // Mise à jour du scroller
            list->Scroller->Position=list->List_start;
            Window_draw_slider(list->Scroller);
          }
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key==SDLK_PAGEUP && (list->Cursor_position+list->List_start)>0)
        {
          Key=0;
          Hide_cursor();
          if(list->Cursor_position!=0)
          {
            list->Cursor_position=0;
          }
          else
          {
            list->List_start-=list->Scroller->Nb_visibles;
            if (list->List_start<0)
            {
              list->List_start=0;
            }
            // Mise à jour du scroller
            list->Scroller->Position=list->List_start;
            Window_draw_slider(list->Scroller);
          }
          Window_redraw_list(list);
          Display_cursor();
          // Store the selected value as attribute2
          Window_attribute2=list->List_start + list->Cursor_position;
          // Return the control ID of the list.
          return list->Number;
        }
        if (Key == KEY_MOUSEWHEELUP && list->List_start>0)
        {
          list->Cursor_position+=list->List_start;
          if (list->List_start>=3)
            list->List_start-=3;
          else
            list->List_start=0;
          list->Cursor_position-=list->List_start;
          // On affiche à nouveau la liste
          Hide_cursor();
          Window_redraw_list(list);
          // Mise à jour du scroller
          list->Scroller->Position=list->List_start;
          Window_draw_slider(list->Scroller);
          Display_cursor();
        }
        if (Key==KEY_MOUSEWHEELDOWN && list->List_start<list->Scroller->Nb_elements-list->Scroller->Nb_visibles)
        {
          list->Cursor_position+=list->List_start;
          list->List_start+=3;
          if (list->List_start+list->Scroller->Nb_visibles>list->Scroller->Nb_elements)
          {
            list->List_start=list->Scroller->Nb_elements-list->Scroller->Nb_visibles;
          }          
          list->Cursor_position-=list->List_start;
          // On affiche à nouveau la liste
          Hide_cursor();
          Window_redraw_list(list);
          // Mise à jour du scroller
          list->Scroller->Position=list->List_start;
          Window_draw_slider(list->Scroller);
          Display_cursor();
        }
      
      }
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
