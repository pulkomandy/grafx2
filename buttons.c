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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <SDL.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "misc.h"
#include "graph.h"
#include "engine.h"
#include "readline.h"
#include "filesel.h"
#include "loadsave.h"
#include "init.h"
#include "buttons.h"
#include "operatio.h"
#include "pages.h"
#include "palette.h"
#include "errors.h"
#include "readini.h"
#include "saveini.h"
#include "shade.h"
#include "io.h"
#include "help.h"
#include "text.h"
#include "sdlscreen.h"
#include "windows.h"
#include "brush.h"
#include "input.h"
#include "special.h"

#ifdef __VBCC__
    #define __attribute__(x)
#endif

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    #include <proto/dos.h>
    #include <dirent.h>
    #define isHidden(x) (0)
#elif defined(__WIN32__)
    #include <dirent.h>
    #include <windows.h>
    #define isHidden(x) (GetFileAttributesA((x)->d_name)&FILE_ATTRIBUTE_HIDDEN)
#else
    #include <dirent.h>
    #define isHidden(x) ((x)->d_name[0]=='.')
#endif

//extern char Program_version[]; // generated in pversion.c

extern short Old_MX;
extern short Old_MY;


//-- MODELE DE BOUTON DE MENU ------------------------------------------------
/*
void Bouton_***(void)
{
  short clicked_button;

  Open_window(310,190,"***");

  Window_set_normal_button(103,137,80,14,"OK",0,1,SDLK_RETURN); // 1
  Window_set_scroller_button(18,44,88,16,4,0);             // 2

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
  }
  while (clicked_button!=1);

  Close_window();
  Unselect_button(BOUTON_***);
  Display_cursor();
}
*/

void Stencil_update_color(byte color);
void Stencil_tag_color(byte color, byte tag_color);

void Message_out_of_memory(void)
{
  short clicked_button;

  Open_window(216,76,"Not enough memory!");

  Print_in_window(8,20,"Please consult the manual",MC_Black,MC_Light);
  Print_in_window(24,28,"to know how to obtain",MC_Black,MC_Light);
  Print_in_window(36,36,"more memory space.",MC_Black,MC_Light);
  Window_set_normal_button(60,53,40,14,"OK",1,1,SDLK_RETURN); // 1
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
    clicked_button=Window_clicked_button();
  while ((clicked_button<=0) && (Key!=KEY_ESC) && (Key!=SDLK_o));

  if(clicked_button<=0) Key=0;
  Close_window();
  Display_cursor();
}


void Button_Message_initial(void)
{
  char  str[30];
  int   x_pos,offs_y,x,y;

  strcpy(str,"GrafX2 version ");
  strcat(str,Program_version);
  Open_window(260,172,str);

  Window_display_frame_in(10,20,239,62);
  Block(Window_pos_X+(Menu_factor_X*11),
        Window_pos_Y+(Menu_factor_Y*21),
        Menu_factor_X*237,Menu_factor_Y*60,MC_Black);
  for (y=23,offs_y=0; y<79; offs_y+=231,y++)
    for (x=14,x_pos=0; x_pos<231; x_pos++,x++)
      Pixel_in_window(x,y,Gfx->Logo_grafx2[offs_y+x_pos]);

  Print_in_window(130-4*26,88,"Copyright (c) 2007-2009 by",MC_Dark,MC_Light);
  Print_in_window(130-4*23,96,"the Grafx2 project team",MC_Black,MC_Light);
  Print_in_window(130-4*26,112,"Copyright (c) 1996-2001 by",MC_Dark,MC_Light);
  Print_in_window(130-4*13,120,"Sunset Design",MC_Black,MC_Light);
  //Print_in_window( 120-4*13,128,"(placeholder)",MC_Dark,MC_Light);
  Print_in_window(130-4*28,136,"http://grafx2.googlecode.com",MC_Dark,MC_Light);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  while(!Mouse_K && !Key) if(!Get_input()) SDL_Delay(20);
  if (Mouse_K)
    Wait_end_of_click();

  Close_window();
  Display_cursor();
}



void Change_paintbrush_shape(byte shape)
{
  Paintbrush_shape=shape;
  Display_paintbrush_in_menu();

  switch (Current_operation)
  {
    case OPERATION_FILL :
      Paintbrush_shape_before_fill=shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
      break;
    case OPERATION_COLORPICK :
      Paintbrush_shape_before_colorpicker=shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
      break;
    // Note: Il existe un Paintbrush_shape_before_lasso, mais comme le lasso aura
    // été automatiquement désactivé avant d'arriver ici, y'a pas de problème.
  }
}


//-------------------------------- UNDO/REDO ---------------------------------
void Button_Undo(void)
{
  Hide_cursor();
  Undo();

  Set_palette(Main_palette);
  Compute_optimal_menu_colors(Main_palette);

  Display_all_screen();
  Unselect_button(BUTTON_UNDO);
  Draw_menu_button_frame(BUTTON_MAGNIFIER,Main_magnifier_mode);
  Display_menu();
  Display_cursor();
}

void Button_Redo(void)
{
  Hide_cursor();
  Redo();

  Set_palette(Main_palette);
  Compute_optimal_menu_colors(Main_palette);

  Display_all_screen();
  Unselect_button(BUTTON_UNDO);
  Draw_menu_button_frame(BUTTON_MAGNIFIER,Main_magnifier_mode);
  Display_menu();
  Display_cursor();
}


//---------------------------- SCROLL PALETTE LEFT ---------------------------
void Button_Pal_left(void)
{
  short cells;
  cells = (Config.Palette_vertical)?Palette_cells_X():Palette_cells_Y();

  Hide_cursor();
  if (First_color_in_palette)
  {
    if (First_color_in_palette>=cells)
      First_color_in_palette-=cells;
    else
      First_color_in_palette=0;
    Display_menu_palette();
  }
  Unselect_button(BUTTON_PAL_LEFT);
  Display_cursor();
}

void Button_Pal_left_fast(void)
{
  short cells_x = Palette_cells_X();
  short cells_y = Palette_cells_Y();

  Hide_cursor();
  if (First_color_in_palette)
  {
    if (First_color_in_palette>=cells_y*cells_x)
      First_color_in_palette-=cells_y*cells_x;
    else
      First_color_in_palette=0;
    Display_menu_palette();
  }
  Unselect_button(BUTTON_PAL_LEFT);
  Display_cursor();
}


//--------------------------- SCROLL PALETTE RIGHT ---------------------------
void Button_Pal_right(void)
{
  short cells;
  cells = (Config.Palette_vertical)?Palette_cells_X():Palette_cells_Y();

  Hide_cursor();
  if ((int)First_color_in_palette+Palette_cells_X()*Palette_cells_Y()<256)
  {
    First_color_in_palette+=cells;
    Display_menu_palette();
  }
  
  Unselect_button(BUTTON_PAL_RIGHT);
  Display_cursor();
}

void Button_Pal_right_fast(void)
{
  short cells_x = Palette_cells_X();
  short cells_y = Palette_cells_Y();

  Hide_cursor();
  if ((int)First_color_in_palette+cells_y*cells_x<256)
  {
    if ((int)First_color_in_palette+(cells_y)*cells_x*2<256)
      First_color_in_palette+=cells_x*cells_y;
    else
      First_color_in_palette=255/cells_y*cells_y-(cells_x-1)*cells_y;
    Display_menu_palette();
  }
  Unselect_button(BUTTON_PAL_RIGHT);
  Display_cursor();
}

//-------------------- item de la forecolor dans le menu --------------------
void Button_Select_forecolor(void)
{
  static long time_click = 0;
  long time_previous;

  int color=Pick_color_in_palette();

  time_previous = time_click;
  time_click = SDL_GetTicks();

  if (color == Fore_color)
  {
    // Check if it's a double-click    
    if (time_click - time_previous < Config.Double_click_speed)
    {
    	// Open palette window
    	Button_Palette();
    }
  }	  
  else if (color!=-1)
  {
    Hide_cursor();
    Set_fore_color(color);
    Display_cursor();
  }
}

//-------------------- item de la backcolor dans le menu --------------------
void Button_Select_backcolor(void)
{
  int color=Pick_color_in_palette();

  if (color!=-1)
  {
    Hide_cursor();
    Set_back_color(color);
    Display_cursor();
  }
}


//---------------------- Cacher ou réafficher le menu ------------------------
void Pixel_in_hidden_toolbar(__attribute__((unused)) word x,__attribute__((unused)) word y,__attribute__((unused)) byte color)
{
  // C'est fait exprès que ce soit vide...
  // C'est parce que y'a rien du tout à afficher vu que la barre d'outil est
  // cachée... C'est simple non?
}


void Button_Hide_menu(void)
{
  Hide_cursor();
  if (Menu_is_visible)
  {
    Menu_is_visible=0;
    Pixel_in_menu=Pixel_in_hidden_toolbar;
    Menu_Y=Screen_height;

    if (Main_magnifier_mode)
    {
      Compute_magnifier_data();
      if (Main_magnifier_offset_Y+Main_magnifier_height>Main_image_height)
      {
        if (Main_magnifier_height>Main_image_height)
          Main_magnifier_offset_Y=0;
        else
          Main_magnifier_offset_Y=Main_image_height-Main_magnifier_height;
      }
    }

    //   On repositionne le décalage de l'image pour qu'il n'y ait pas d'in-
    // -cohérences lorsqu'on sortira du mode Loupe.
    if (Main_offset_Y+Screen_height>Main_image_height)
    {
      if (Screen_height>Main_image_height)
        Main_offset_Y=0;
      else
        Main_offset_Y=Main_image_height-Screen_height;
    }
    // On fait pareil pour le brouillon
    if (Spare_offset_Y+Screen_height>Spare_image_height)
    {
      if (Screen_height>Spare_image_height)
        Spare_offset_Y=0;
      else
        Spare_offset_Y=Spare_image_height-Screen_height;
    }

    Compute_magnifier_data();
    if (Main_magnifier_mode)
      Position_screen_according_to_zoom();
    Compute_limits();
    Compute_paintbrush_coordinates();
    Display_all_screen();
  }
  else
  {
    Menu_is_visible=1;
    Pixel_in_menu=Pixel_in_toolbar;
    Menu_Y=Screen_height-(Menu_height*Menu_factor_Y);

    Compute_magnifier_data();
    if (Main_magnifier_mode)
      Position_screen_according_to_zoom();
    Compute_limits();
    Compute_paintbrush_coordinates();
    Display_menu();
    if (Main_magnifier_mode)
      Display_all_screen();
  }
  Unselect_button(BUTTON_HIDE);
  Display_cursor();
}


//--------------------------- Quitter le programme ---------------------------
byte Button_Quit_local_function(void)
{
  short clicked_button;
  static char  filename[MAX_PATH_CHARACTERS];
  byte  old_cursor_shape;

  if (!Main_image_is_modified)
    return 1;

  // On commence par afficher la fenêtre de QUIT
  Open_window(160,84,"Quit ?");
  Window_set_normal_button(20,20,120,14,"Stay",0,1,KEY_ESC);          // 1
  Window_set_normal_button(20,40,120,14,"Save & quit",1,1,SDLK_s);   // 2
  Window_set_normal_button(20,60,120,14,"Discard (Quit)",1,1,SDLK_d);// 3
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_QUIT, NULL);
    else if (Is_shortcut(Key,0x100+BUTTON_QUIT))
      clicked_button=1;
  }
  while (clicked_button<=0);

  Close_window();
  Display_cursor();

  switch(clicked_button)
  {
    case 1 : return 0; // Rester
    case 2 : // Sauver et enregistrer
             Get_full_filename(filename,0);
             if ( (!File_exists(filename)) || Confirmation_box("Erase old file ?") )
             {
               Hide_cursor();
               old_cursor_shape=Cursor_shape;
               Cursor_shape=CURSOR_SHAPE_HOURGLASS;
               Display_cursor();

               Save_image(1);

               Hide_cursor();
               Cursor_shape=old_cursor_shape;
               Display_cursor();

               if (!File_error)
                 // L'ayant sauvée avec succès,
                 return 1; // On peut quitter
               else
                 // Il y a eu une erreur lors de la sauvegarde,
                 return 0; // On ne peut donc pas quitter
             }
             else
               // L'utilisateur ne veut pas écraser l'ancien fichier,
               return 0; // On doit donc rester
    case 3 : return 1; // Quitter
  }
  return 0;
}


void Button_Quit(void)
{
  //short clicked_button;

  if (Button_Quit_local_function())
  {
    if (Spare_image_is_modified)
    {
      Button_Page(); // On passe sur le brouillon
      // Si l'utilisateur présente les derniers symptomes de l'abandon
      if (Button_Quit_local_function())
        Quitting=1;
    }
    else
      Quitting=1;
  }

  if ( (Menu_is_visible) && (Mouse_Y+8>Menu_Y) )
    Hide_cursor();

  Unselect_button(BUTTON_QUIT);

  if ( (Menu_is_visible) && (Mouse_Y+8>Menu_Y) )
    Display_cursor();
}


//---------------------------- Effacer l'écran -------------------------------
void Button_Clear(void)
{
  Hide_cursor();
  Backup();
  if (Stencil_mode && Config.Clear_with_stencil)
    Hide_current_image_with_stencil(0,Stencil);
  else
    Hide_current_image(0);
  Redraw_layered_image();
  Display_all_screen();
  End_of_modification();
  Unselect_button(BUTTON_CLEAR);
  Display_cursor();
}

void Button_Clear_with_backcolor(void)
{
  Hide_cursor();
  Backup();
  if (Stencil_mode && Config.Clear_with_stencil)
    Hide_current_image_with_stencil(Back_color,Stencil);
  else
    Hide_current_image(Back_color);
  Redraw_layered_image();
  Display_all_screen();
  End_of_modification();
  Unselect_button(BUTTON_CLEAR);
  Display_cursor();
}
 
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


//------------------------------- Paramètres ---------------------------------

void Settings_display_config(T_Config * conf)
#define YES "YES"
#define NO  " NO"
{
  T_Scroller_button * slider=Window_scroller_button_list;
  char str[4];

  Hide_cursor();

  // slider = sensitivity slider for Y
  slider->Position=conf->Mouse_sensitivity_index_y-1;
  Window_draw_slider(slider);

  slider=slider->Next;
  // slider = sensitivity slider for X
  slider->Position=conf->Mouse_sensitivity_index_x-1;
  Window_draw_slider(slider);

  Print_in_window(273, 31,(conf->Show_hidden_files)?YES:NO,MC_Black,MC_Light);
  Print_in_window(273, 46,(conf->Show_hidden_directories)?YES:NO,MC_Black,MC_Light);
//  Print_in_window(273, 61,(conf->Show_system_directories)?YES:NO,MC_Black,MC_Light);

  Print_in_window(223, 84,(conf->Safety_colors)?YES:NO,MC_Black,MC_Light);
  Print_in_window(223, 99,(conf->Adjust_brush_pick)?YES:NO,MC_Black,MC_Light);
  Print_in_window(223,114,(conf->Separate_colors)?YES:NO,MC_Black,MC_Light);
  Print_in_window(223,129,(conf->Auto_set_res)?YES:NO,MC_Black,MC_Light);
  Print_in_window(183,144,(conf->Coords_rel)?"Relative":"Absolute",MC_Black,MC_Light);

  Print_in_window( 91, 84,(conf->Display_image_limits)?YES:NO,MC_Black,MC_Light);
  Print_in_window( 91, 99,(conf->Clear_palette)?YES:NO,MC_Black,MC_Light);
  Print_in_window( 91,114,(conf->Maximize_preview)?YES:NO,MC_Black,MC_Light);
  Print_in_window( 91,129,(conf->Backup)?YES:NO,MC_Black,MC_Light);
  /*
  switch (conf->Cursor)
  {
    case 0 : Print_in_window(67,144," Solid",MC_Black,MC_Light); break;
    case 1 : Print_in_window(67,144,"Transp",MC_Black,MC_Light); break;
    default: Print_in_window(67,144,"  Thin",MC_Black,MC_Light);
  }
  */

  /*
  if (conf->Font)
  { // Fun
    Print_in_window(  8,31," ",MC_Black,MC_Light);
    Print_in_window( 78,31," ",MC_Black,MC_Light);
    Print_in_window( 82,31,"\020",MC_Black,MC_Light);
    Print_in_window(152,31,"\021",MC_Black,MC_Light);
  }
  else
  { // Classic
    Print_in_window( 82,31," ",MC_Black,MC_Light);
    Print_in_window(152,31," ",MC_Black,MC_Light);
    Print_in_window(  8,31,"\020",MC_Black,MC_Light);
    Print_in_window( 78,31,"\021",MC_Black,MC_Light);
  }
  */

  Print_in_window(155,166,(conf->Auto_save)?YES:NO,MC_Black,MC_Light);

  Num2str(conf->Max_undo_pages,str,2);
  Window_input_content(Window_special_button_list,str);

  Display_cursor();
}

void Settings_save_config(T_Config * conf)
{
  if (Save_CFG())
    Error(0);
  else
    if (Save_INI(conf))
      Error(0);
}

void Settings_load_config(T_Config * conf)
{
  if (Load_CFG(0))
    Error(0);
  else
    if (Load_INI(conf))
      Error(0);
}

void Button_Settings(void)
{
  short clicked_button;
  T_Config Config_choisie;
  char str[3];
  byte config_is_reloaded=0;

  Config_choisie=Config;

  Open_window(307,182,"Settings");

  // On commence par dessiner tous les Cadres
  Window_display_frame(  5, 47,157,17); // Nb UNDO
  Window_display_frame(163, 16,139,48); // Show in filelist
  Window_display_frame(253, 77, 49,82); // Mouse sens.
  Window_display_frame(  5, 65,247,96); // |_ Misc.
/*  Window_display_frame(  5, 65,157,14); // |
  // On découpe le Frame bizarre des "Miscellaneous"
  Pixel_in_window(6,77,MC_White);
  Pixel_in_window(5,78,MC_Dark);
  Block(Window_pos_X+(7*Menu_factor_X),Window_pos_Y+(77*Menu_factor_Y),
        Menu_factor_X*154,Menu_factor_Y<<1,MC_Light);
  Pixel_in_window(161,77,MC_Light);
  Pixel_in_window(160,77,MC_Dark);
*/
  // On affiche maintenant tout le blabla
  Print_in_window(169, 19,"Show in filelist",MC_Dark,MC_Light);
  Print_in_window(  9, 52,"Nb of UNDO pages",MC_Dark,MC_Light);
  Print_in_window( 80, 70,"Miscellaneous"   ,MC_Dark,MC_Light);
  Print_in_window(258, 80,"Mouse"           ,MC_Dark,MC_Light);
  Print_in_window(258, 88,"Sens."           ,MC_Dark,MC_Light);
  Print_in_window(256,123,"X"               ,MC_Dark,MC_Light);
  Print_in_window(292,123,"Y"               ,MC_Dark,MC_Light);


  // Button Show/Hide dans le fileselect
  Window_set_normal_button(167, 28,131,14,"Hidden files:   ",0,1,SDLK_LAST); // 1
  Window_set_normal_button(167, 43,131,14,"Hidden dir. :   ",0,1,SDLK_LAST); // 2
//  Window_set_normal_button(167, 58,131,14,"System dir. :   ",0,1,SDLK_LAST);

  // Button Show/Hide Picture limits
  Window_set_normal_button(  9, 81,107,14,"Limits   :   ",0,1,SDLK_LAST); // 3
  // Button Show/Hide Picture limits
  Window_set_normal_button(  9, 96,107,14,"Clear pal:   ",0,1,SDLK_LAST); // 4
  // Button Show/Hide Picture limits
  Window_set_normal_button(  9,111,107,14,"Max prev.:   ",0,1,SDLK_LAST); // 5
  // Button Effectuer des backups à chaque sauvegarde
  Window_set_normal_button(  9,126,107,14,"Backup   :   ",0,1,SDLK_LAST); // 6

  // Button Safety colors
  Window_set_normal_button(117, 81,131,14,"Safe. colors:   ",0,1,SDLK_LAST); // 7
  // Button Adjust Brush Pick
  Window_set_normal_button(117, 96,131,14,"AdjBrushPick:   ",0,1,SDLK_LAST); // 8
  // Button Separate colors
  Window_set_normal_button(117,111,131,14,"Separate col:   ",0,1,SDLK_LAST); // 9
  // Button Passer dans la résolution appropriée après un chargement
  Window_set_normal_button(117,126,131,14,"Auto-set res:   ",0,1,SDLK_LAST); // 10
  // Button Adapter la palette après un chargement (<=> Shift+BkSpc)
  Window_set_normal_button(117,141,131,14,"Coords:         ",0,1,SDLK_LAST); // 11

    // Button Reload
  Window_set_normal_button(  6,163, 51,14,"Reload"       ,0,1,SDLK_LAST); // 12
    // Button Auto-save
  Window_set_normal_button( 73,163,107,14,"Auto-save:   ",0,1,SDLK_LAST); // 13
    // Button Save
  Window_set_normal_button(183,163, 51,14,"Save"         ,0,1,SDLK_LAST); // 14
    // Button Close
  Window_set_normal_button(250,163, 51,14,"Close"        ,0,1,KEY_ESC); // 15

  // Jauges de sensibilité de la souris (X puis Y)
  Window_set_scroller_button(265,99,56,4,1,0); // 16
  Window_set_scroller_button(279,99,56,4,1,0); // 17

  // Zone de saisie du nb de pages de Undo
  Window_set_input_button(140,50,2);           // 18

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  Settings_display_config(&Config_choisie);


  do
  {
    clicked_button=Window_clicked_button();

    switch(clicked_button)
    {
      case  1 : // Hidden files
        Config_choisie.Show_hidden_files=(Config_choisie.Show_hidden_files)?0:-1;
        break;
      case  2 : // Hidden dir.
        Config_choisie.Show_hidden_directories=(Config_choisie.Show_hidden_directories)?0:-1;
        break;
//      case  5 : // System dir.
//        Config_choisie.Show_system_directories=(Config_choisie.Show_system_directories)?0:-1;
//        break;
      case  3 : // Draw limits
        Config_choisie.Display_image_limits=!Config_choisie.Display_image_limits;
        break;
      case  4 : // Clear palette
        Config_choisie.Clear_palette=!Config_choisie.Clear_palette;
        break;
      case  5 : // Maximize preview
        Config_choisie.Maximize_preview=!Config_choisie.Maximize_preview;
        break;
      case  6 : // Backup
        Config_choisie.Backup=!Config_choisie.Backup;
        break;
      case 7 : // Safety colors
        Config_choisie.Safety_colors=!Config_choisie.Safety_colors;
        break;
      case 8 : // Adjust brush pick
        Config_choisie.Adjust_brush_pick=!Config_choisie.Adjust_brush_pick;
        break;
      case 9 : // Separate colors
        Config_choisie.Separate_colors=!Config_choisie.Separate_colors;
        break;
      case 10 : // Auto-set resolution
        Config_choisie.Auto_set_res=!Config_choisie.Auto_set_res;
        break;
      case 11 : // Coordonnées
        Config_choisie.Coords_rel=!Config_choisie.Coords_rel;
        break;
      case 12 : // Reload
        Settings_load_config(&Config_choisie);
        config_is_reloaded=1;
        break;
      case 13 : // Auto-save
        Config_choisie.Auto_save=!Config_choisie.Auto_save;
        break;
      case 14 : // Save
        Settings_save_config(&Config_choisie);
        break;
      // 15 : OK
      case 16 : // X Sensib.
        Config_choisie.Mouse_sensitivity_index_x=Window_attribute2+1;
        break;
      case 17 : // Y Sensib.
        Config_choisie.Mouse_sensitivity_index_y=Window_attribute2+1;
        break;
      case 18 : // Nb pages Undo
        Num2str(Config_choisie.Max_undo_pages,str,2);
        Readline(142,52,str,2,1);
        Config_choisie.Max_undo_pages=atoi(str);
        // On corrige la valeur
        if (Config_choisie.Max_undo_pages>NB_MAX_PAGES_UNDO)
        {
          Config_choisie.Max_undo_pages=NB_MAX_PAGES_UNDO;
          Num2str(Config_choisie.Max_undo_pages,str,2);
          Window_input_content(Window_special_button_list,str);
        }
        else if (!Config_choisie.Max_undo_pages)
        {
          Config_choisie.Max_undo_pages=1;
          Num2str(Config_choisie.Max_undo_pages,str,2);
          Window_input_content(Window_special_button_list,str);
        }
        Display_cursor();
        Key=0;
        Wait_end_of_click();
    }

    if ((clicked_button>=3) && (clicked_button<=4))
    {
      Main_fileselector_position=0;
      Main_fileselector_offset=0;
      Spare_fileselector_position=0;
      Spare_fileselector_offset=0;
    }

    if ((clicked_button>=1) && (clicked_button<15))
      Settings_display_config(&Config_choisie);
      
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_SETTINGS, NULL);
    else if (Is_shortcut(Key,0x100+BUTTON_SETTINGS))
      clicked_button=15;
  }
  while ( (clicked_button!=15) && (Key!=SDLK_RETURN) );

  Config=Config_choisie;

  if (config_is_reloaded)
    Compute_optimal_menu_colors(Main_palette);

  Close_window();
  Unselect_button(BUTTON_SETTINGS);
  // Raffichage du menu pour que les inscriptions qui y figurent soient 
  // retracées avec la nouvelle fonte
  Display_menu();
  Display_cursor();

  // On vérifie qu'on peut bien allouer le nombre de pages Undo.
  Set_number_of_backups(Config.Max_undo_pages);
}

// Data for skin selector
T_Fileselector Skin_files_list;


// Data for font selector
T_Fileselector Font_files_list;

//
char * Format_font_filename(const char * fname)
{
  static char result[12];
  int c;
  int length;
  
  fname+=5; // Assume "font_" prefix
  length=strlen(fname) - 4; // assume .png extension
  
  for (c=0;c<11 && c<length ;c++)
  {
    result[c]=fname[c];
  }
  result[c]='\0';
  if (length>11)
    result[10] = ELLIPSIS_CHARACTER;

  return result;
}

// Add a skin to the list
void Add_font_or_skin(const char *name)
{
  const char * fname;
  int namelength;
  
  // Cut the long name to keep only filename (no directory)
  fname = Find_last_slash(name);
  if (fname)
    fname++;
  else
    fname=name;
  namelength = strlen(fname);
  if (namelength>=10 && fname[0]!='_' && !strncasecmp(fname, "skin_", 5)
	&& (!strcasecmp(fname+namelength-4,".png") 
		|| !strcasecmp(fname+namelength-4,".gif")))
  {
    Add_element_to_list(&Skin_files_list, name, 0);  
    
    if (fname[0]=='\0')
      return;

    // Remove directory from full name
    strcpy(Skin_files_list.First->Full_name, fname);
    // Reformat the short name differently
    strcpy(Skin_files_list.First->Short_name,
		Format_filename(Skin_files_list.First->Full_name, 0)
	);
  }
  else if (namelength>=10 && !strncasecmp(fname, "font_", 5) && (!strcasecmp(fname+namelength-4,".png")))
  {
    Add_element_to_list(&Font_files_list, name, 0);  
    
    if (fname[0]=='\0')
      return;

    // Remove directory from full name
    strcpy(Font_files_list.First->Full_name, fname);
    // Reformat the short name differently
    strcpy(Font_files_list.First->Short_name,Format_font_filename(Font_files_list.First->Full_name));
  }
   
}

// Callback to display a skin name in the list
void Draw_one_skin_name(word x, word y, word index, byte highlighted)
{
  T_Fileselector_item * current_item;

  if (Skin_files_list.Nb_elements)
  {
    current_item = Get_item_by_index(&Skin_files_list, index);    
    Print_in_window(x,y,current_item->Short_name, MC_Black, (highlighted)?MC_Dark:MC_Light);
  }
}

/// Skin selector window
void Button_Skins(void)
{
  short clicked_button;
  short temp;
  char skinsdir[MAX_PATH_CHARACTERS];
  T_Dropdown_button * font_dropdown;
  T_Dropdown_button * cursor_dropdown;
  T_List_button * skin_list;
  T_Scroller_button * file_scroller;
  int selected_font=0;
  int selected_cursor=Config.Cursor;
  
  char * cursors[] = { "Solid", "Transparent", "Thin" };

  #define FILESEL_Y 52

  // --- Read the contents of skins/ directory ------------------
  
  // Here we use the same data container as the fileselectors.
  // Reinitialize the list
  Free_fileselector_list(&Skin_files_list);
  Free_fileselector_list(&Font_files_list);
  // Browse the "skins" directory
  strcpy(skinsdir,Data_directory);
  strcat(skinsdir,"skins");
  // Add each found file to the list
  For_each_file(skinsdir, Add_font_or_skin);
  // Sort it
  Sort_list_of_files(&Skin_files_list);
  Sort_list_of_files(&Font_files_list);
  
  selected_font = Find_file_in_fileselector(&Font_files_list, Config.Font_file);
  
  // --------------------------------------------------------------

  Open_window(178,155,"Skins");

  // Frames
  Window_display_frame_in(6,FILESEL_Y-2,148,84); // File selector

  // Texts
  Print_in_window( 6, 21,"Font"            ,MC_Black,MC_Light);
  Print_in_window( 6, 36,"Cursor"          ,MC_Black,MC_Light);

  // Ok button
  Window_set_normal_button(6,136, 51,14,"OK"         ,0,1,SDLK_RETURN); // 1

  // List of skins
  skin_list = Window_set_list_button(
    // Fileselector
    Window_set_special_button(8,FILESEL_Y+1,144,80), // 2
    // Scroller for the fileselector
    (file_scroller = Window_set_scroller_button(160,FILESEL_Y+1,82,
		Skin_files_list.Nb_elements,10,0)), // 3
    Draw_one_skin_name); // 4
  
  skin_list->Cursor_position = Find_file_in_fileselector(&Skin_files_list, Config.Skin_file);

  // Buttons to choose a font
  font_dropdown = Window_set_dropdown_button(60,19,104,11,0, Get_item_by_index(&Font_files_list,selected_font)->Short_name,1,0,1,RIGHT_SIDE|LEFT_SIDE); // 5
  for (temp=0; temp<Font_files_list.Nb_files; temp++)
    Window_dropdown_add_item(font_dropdown,temp,Get_item_by_index(&Font_files_list,temp)->Short_name);

  // Cancel
  Window_set_normal_button(62,136, 51,14,"Cancel",0,1,SDLK_ESCAPE); // 6

  // Dropdown list to choose cursor type
  cursor_dropdown = Window_set_dropdown_button(60,34,104,11,0,cursors[selected_cursor],1,0,1,RIGHT_SIDE|LEFT_SIDE); // 7
  for (temp=0; temp<3; temp++)
    Window_dropdown_add_item(cursor_dropdown,temp,cursors[temp]);
  
  Window_redraw_list(skin_list);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch(clicked_button)
    {
	    case 1 : // OK
	      break;
      case 2 : // doesn't happen
        break;
      case 3 : // doesn't happen
        break;
      case 4 : // a file is selected
        break;
      case  5 : // Font dropdown
        selected_font=Window_attribute2; // Get the index of the chosen font.
        break;
	  // 5: Cancel
       case 7 : // Cursor
       selected_cursor = Window_attribute2;
        break;
    }

    switch (Key)
    {
      case SDLK_UNKNOWN : break;
						  /*
      case SDLK_DOWN : // Bas
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_scroll_down(&Main_fileselector_position,&Main_fileselector_offset);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      case SDLK_UP : // Haut
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_scroll_up(&Main_fileselector_position,&Main_fileselector_offset);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      case SDLK_PAGEDOWN : // PageDown
      case KEY_MOUSEWHEELDOWN :
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_page_down(&Main_fileselector_position,&Main_fileselector_offset,9);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      case SDLK_PAGEUP : // PageUp
      case KEY_MOUSEWHEELUP :
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_page_up(&Main_fileselector_position,&Main_fileselector_offset,9);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      case SDLK_END : // End
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_end(&Main_fileselector_position,&Main_fileselector_offset);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      case SDLK_HOME : // Home
        *quicksearch_filename=0;
        Hide_cursor();
        Selector_home(&Main_fileselector_position,&Main_fileselector_offset);
  		if (file_scroller->Position!=Main_fileselector_position)
  		{
    	  // Si c'est le cas, il faut mettre à jour la jauge
    	  file_scroller->Position=Main_fileselector_position;
    	  Window_draw_slider(file_scroller);
  		}
        Key=0;
        break;
      default: // Autre => On se place sur le nom de fichier qui correspond
        if (clicked_button<=0)
        {
          if (Is_shortcut(Key,0x100+BUTTON_HELP))
          {
            Window_help(BUTTON_SETTINGS, NULL);
            break;
          }
          temp=strlen(quicksearch_filename);
          if (Key_ANSI>= ' ' && Key_ANSI < 255 && temp<50)
          {
            quicksearch_filename[temp]=Key_ANSI;
            quicksearch_filename[temp+1]='\0';
            most_matching_filename=Find_filename_match(Skin_files_list, quicksearch_filename);
            if ( (most_matching_filename) )
            {
              temp=Main_fileselector_position+Main_fileselector_offset;
              Hide_cursor();
              Highlight_file(most_matching_filename);
              Prepare_and_display_filelist(Main_fileselector_position,Main_fileselector_offset,file_scroller);
              Display_cursor();
              if (temp!=Main_fileselector_position+Main_fileselector_offset)
                New_preview_is_needed=1;
            }
            else
              *quicksearch_filename=0;
            Key=0;
          }
        }
        else
          *quicksearch_filename=0;
    		*/
    }
  }
  while ( (clicked_button!=1) && (clicked_button !=6) && (Key != SDLK_ESCAPE));

  if(clicked_button == 1)
  {
    T_Gui_skin * gfx;
    byte * new_font;

    // (Re-)load GUI graphics from selected skins
    strcpy(skinsdir, Get_item_by_index(&Skin_files_list, skin_list->List_start+skin_list->Cursor_position)->Full_name);
	  gfx=Load_graphics(skinsdir);
	  if (gfx == NULL) // Error
    {
      Verbose_error_message(Gui_loading_error_message);
	  }
	  else
	  {
      free(Gfx);
      Gfx = gfx;
      
      free(Config.Skin_file);
	    Config.Skin_file = strdup(skinsdir);
    }
	  // (Re-)load the selected font
	  new_font = Load_font(Get_item_by_index(&Font_files_list,selected_font)->Full_name);
	  if (new_font)
	  {
	    const char * fname;
	    
	    free(Menu_font);
	    Menu_font = new_font;
	    fname = Get_item_by_index(&Font_files_list,selected_font)->Full_name;
	    free(Config.Font_file);
	    Config.Font_file = strdup(fname);
	  }
    // Confirm the change of cursor shape
	  Config.Cursor = selected_cursor;
  }

  Close_window();
  Unselect_button(BUTTON_SETTINGS);
  // Raffichage du menu pour que les inscriptions qui y figurent soient retracées avec la nouvelle fonte
  Display_menu();
  Display_cursor();
}


//---------------------------- Changement de page ----------------------------
void Button_Page(void)
{
	byte   factor_index;
  char   Temp_buffer[256];
  
  #define SWAP_BYTES(a,b) { byte c=a; a=b; b=c;}
  #define SWAP_WORDS(a,b) { word c=a; a=b; b=c;}
  #define SWAP_SHORTS(a,b) { short c=a; a=b; b=c;}
  #define SWAP_FLOATS(a,b) { float c=a; a=b; b=c;}
  
  Hide_cursor();

  // On dégrossit le travail avec les infos des listes de pages
  Exchange_main_and_spare();

  // On fait le reste du travail "à la main":
  SWAP_SHORTS(Main_offset_X,Spare_offset_X)
  SWAP_SHORTS(Main_offset_Y,Spare_offset_Y)
  SWAP_SHORTS(Old_main_offset_X,Old_spare_offset_X)
  SWAP_SHORTS(Old_main_offset_Y,Old_spare_offset_Y)
  SWAP_SHORTS(Main_separator_position,Spare_separator_position)
  SWAP_SHORTS(Main_X_zoom,Spare_X_zoom)
  SWAP_FLOATS(Main_separator_proportion,Spare_separator_proportion)
  SWAP_BYTES (Main_magnifier_mode,Spare_magnifier_mode)

  Pixel_preview=(Main_magnifier_mode)?Pixel_preview_magnifier:Pixel_preview_normal;

  SWAP_WORDS (Main_magnifier_factor,Spare_magnifier_factor)
  SWAP_WORDS (Main_magnifier_height,Spare_magnifier_height)
  SWAP_WORDS (Main_magnifier_width,Spare_magnifier_width)
  SWAP_SHORTS(Main_magnifier_offset_X,Spare_magnifier_offset_X)
  SWAP_SHORTS(Main_magnifier_offset_Y,Spare_magnifier_offset_Y)
  // Swap du booléen "Image modifiée"
  SWAP_BYTES (Main_image_is_modified,Spare_image_is_modified)

  // Swap des infos sur les fileselects
  strcpy(Temp_buffer            ,Spare_current_directory);
  strcpy(Spare_current_directory,Main_current_directory);
  strcpy(Main_current_directory,Temp_buffer             );

  SWAP_BYTES (Main_format,Spare_format)
  SWAP_WORDS (Main_fileselector_position,Spare_fileselector_position)
  SWAP_WORDS (Main_fileselector_offset,Spare_fileselector_offset)
  SWAP_SHORTS(Main_current_layer,Spare_current_layer)
  SWAP_WORDS (Main_layers_visible,Spare_layers_visible)

  // A la fin, on affiche l'écran
  for (factor_index=0; ZOOM_FACTOR[factor_index]!=Main_magnifier_factor; factor_index++);
  Change_magnifier_factor(factor_index);

  Set_palette(Main_palette);
  Compute_optimal_menu_colors(Main_palette);
  Display_all_screen();
  Unselect_button(BUTTON_PAGE);
  Draw_menu_button_frame(BUTTON_MAGNIFIER,Main_magnifier_mode);
  Display_menu();

  Display_cursor();
}


// -- Copie de page ---------------------------------------------------------

void Copy_image_only(void)
{
  word old_width=Spare_image_width;
  word old_height=Spare_image_height;
  
  if (Backup_and_resize_the_spare(Main_image_width,Main_image_height))
  {
    byte i;
    for (i=0; i<Spare_backups->Pages->Nb_layers; i++)
    {
      if (i == Spare_current_layer)
      {
        // Copy the current layer
        memcpy(Spare_backups->Pages->Image[i],Main_backups->Pages->Image[Main_current_layer],Main_image_width*Main_image_height);
      }
      else
      {
        // Resize the original layer
        Copy_part_of_image_to_another(
        Spare_backups->Pages->Next->Image[i],0,0,Min(old_width,Spare_image_width),
        Min(old_height,Spare_image_height),old_width,
        Spare_backups->Pages->Image[i],0,0,Spare_image_width);
      }
    }

    // Copie des dimensions de l'image
    /*
      C'est inutile, le "Backuper et redimensionner brouillon" a déjà modifié
      ces valeurs pour qu'elles soient correctes.
    */
    /*
    Spare_image_width=Main_image_width;
    Spare_image_height=Main_image_height;
    */

    // Copie des décalages de la fenêtre principale (non zoomée) de l'image
    Spare_offset_X=Main_offset_X;
    Spare_offset_Y=Main_offset_Y;

    // Copie du booléen "Mode loupe" de l'image
    Spare_magnifier_mode=Main_magnifier_mode;

    // Copie du facteur de zoom du brouillon
    Spare_magnifier_factor=Main_magnifier_factor;

    // Copie des dimensions de la fenêtre de zoom
    Spare_magnifier_width=Main_magnifier_width;
    Spare_magnifier_height=Main_magnifier_height;

    // Copie des décalages de la fenêtre de zoom
    Spare_magnifier_offset_X=Main_magnifier_offset_X;
    Spare_magnifier_offset_Y=Main_magnifier_offset_Y;

    // Copie des données du split du zoom
    Spare_separator_position=Main_separator_position;
    Spare_X_zoom=Main_X_zoom;
    Spare_separator_proportion=Main_separator_proportion;
  }
  else
    Message_out_of_memory();
}


void Copy_some_colors(void)
{
  short index;
  byte confirmation=0;
  static byte mask_color_to_copy[256]; // static to use less stack

  memset(mask_color_to_copy,1,256);
  Menu_tag_colors("Tag colors to copy",mask_color_to_copy,&confirmation,0, NULL, 0xFFFF);

  if (confirmation &&
    (!Spare_image_is_modified || Confirmation_box("Spare page was modified. Proceed?")))
  {
    for (index=0; index<256; index++)
    {
      if (mask_color_to_copy[index])
        memcpy(Spare_palette+index,Main_palette+index,
               sizeof(T_Components));
    }
  }
}


void Button_Copy_page(void)
{
  short clicked_button;


  Open_window(168,137,"Copy to spare page");

  Window_set_normal_button(10, 20,148,14,"Pixels + palette" , 0,1,SDLK_RETURN); // 1
  Window_set_normal_button(10, 37,148,14,"Pixels only"      , 3,1,SDLK_x); // 2
  Window_set_normal_button(10, 54,148,14,"Palette only"     , 1,1,SDLK_p); // 3
  Window_set_normal_button(10, 71,148,14,"Some colors only" , 6,1,SDLK_c); // 4
  Window_set_normal_button(10, 88,148,14,"Palette and remap",13,1,SDLK_r); // 5
  Window_set_normal_button(44,114, 80,14,"Cancel"           , 0,1,KEY_ESC); // 6
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_PAGE, NULL);
    else if (Is_shortcut(Key,0x200+BUTTON_PAGE))
      clicked_button=6;
  }
  while (clicked_button<=0);

  Close_window();
  Display_cursor();

  if (clicked_button!=6)
  {
    if (clicked_button==4)
      Copy_some_colors();
    else
    {
      if ( (!Spare_image_is_modified)
        || (Confirmation_box("Spare page was modified. Proceed?")) )
      {
        if (clicked_button<=2)
          Copy_image_only();

        if (clicked_button==5)
          Remap_spare();

        if (clicked_button!=2) // copie de la palette
          memcpy(Spare_palette,Main_palette,sizeof(T_Palette));

        Spare_image_is_modified=1;
      }
    }
  }

  Hide_cursor();
  Unselect_button(BUTTON_PAGE);
  Display_cursor();
}


// -- Suppression d'une page -------------------------------------------------
void Button_Kill(void)
{
  if ( (Main_backups->List_size==1)
    || (!Confirmation_box("Delete the current page?")) )
  {
    if (Main_backups->List_size==1)
      Warning_message("You can't delete the last page.");
    Hide_cursor();
    Unselect_button(BUTTON_KILL);
    Display_cursor();
  }
  else
  {
    Hide_cursor();
    Free_current_page();

    Set_palette(Main_palette);
    Compute_optimal_menu_colors(Main_palette);

    Display_all_screen();
    Unselect_button(BUTTON_KILL);
    Draw_menu_button_frame(BUTTON_MAGNIFIER,Main_magnifier_mode);
    Display_menu();
    Display_cursor();
  }
}


//------------------------- Dimensions Image/Screen ---------------------------

void Check_mode_button(short x_pos, short y_pos, byte state)
{
  byte color;

  switch (state & 0x7F)
  {
    case 0 : color=MC_White; break;
    case 1 : color=MC_Light; break;
    case 2 : color=MC_Dark; break;
    default: color=MC_Black;
  }
  Block(Window_pos_X+Menu_factor_X*x_pos,Window_pos_Y+Menu_factor_Y*y_pos,
        Menu_factor_X*9,Menu_factor_Y*3,color);
        
  Update_rect(Window_pos_X+Menu_factor_X*x_pos,Window_pos_Y+Menu_factor_Y*y_pos,
        Menu_factor_X*9,Menu_factor_Y*3);
}

/// Number of video modes to display in the resolution menu
#define MODELIST_LINES 10

void Display_modes_list(short list_start, short cursor_position)
{
  short index,current_mode;
  short y_pos;
  byte  text_color,background_color;
  char str[29];
  char *ratio;

  for (current_mode=list_start,index=0; index<MODELIST_LINES && current_mode < Nb_video_modes ; index++,current_mode++)
  {
    y_pos=86+(index<<3);
    Check_mode_button(19,y_pos+2,Video_mode[current_mode].State);

    if (cursor_position!=index)
    {
      background_color =MC_Black;
      if ((Video_mode[current_mode].State & 3) == 3)
        text_color=MC_Dark;
      else
        text_color=MC_Light;
    }
    else
    {
      background_color =MC_Dark;
      if ((Video_mode[current_mode].State & 3) == 3)
        text_color=MC_Light;
      else
        text_color=MC_White;
    }
    Num2str(Video_mode[current_mode].Width,str,4);
    str[4]=' ';
    Num2str(Video_mode[current_mode].Height,str+5,4);

    if(Video_mode[current_mode].Fullscreen == 0)
      memcpy(str+9,"   Window          ",20);
    else
	{
      memcpy(str+9," Fullscreen ",13);

	  if (Video_mode[current_mode].Width*3 == Video_mode[current_mode].Height*4)
		  ratio="    4:3";
	  else if (Video_mode[current_mode].Width*9 == Video_mode[current_mode].Height*16)
		  ratio="   16:9";
	  else if (Video_mode[current_mode].Width*10 == Video_mode[current_mode].Height*16)
		  ratio="  16:10";
	  else if (Video_mode[current_mode].Width*145 == Video_mode[current_mode].Height*192)
		  ratio="192:145";
	  else if (Video_mode[current_mode].Width*2 == Video_mode[current_mode].Height*3)
		  ratio="    3:2";
	  else if (Video_mode[current_mode].Width*3 == Video_mode[current_mode].Height*5)
		  ratio="    5:3";
	  else if (Video_mode[current_mode].Width*4 == Video_mode[current_mode].Height*5)
		  ratio="    5:4";
	  else if (Video_mode[current_mode].Width*16 == Video_mode[current_mode].Height*25)
		  ratio="  25:16";
	  else
		  ratio="       ";

	  strcpy(str+21,ratio);
	}

    Print_in_window(38,y_pos,str,text_color,background_color);
  }
}


void Scroll_list_of_modes(short list_start, short cursor_position, int * selected_mode)
{
  Hide_cursor();
  *selected_mode=list_start+cursor_position;
  if (Window_scroller_button_list->Position!=list_start)
  {
    Window_scroller_button_list->Position=list_start;
    Window_draw_slider(Window_scroller_button_list);
  }
  Display_modes_list(list_start,cursor_position);
  Display_cursor();
}

void Button_Resolution(void)
{
  short clicked_button;
  int   selected_mode;
  word  chosen_width;
  word  chosen_height;
  byte  chosen_pixel;
  short list_start;
  short cursor_position;
  short temp;
  char  str[5];
  T_Special_button * input_width_button, * input_button_height;
  T_Dropdown_button * pixel_button;
  static const char *pixel_ratio_labels[PIXEL_MAX] ={
    "Normal    (1x1)",
    "Wide      (2x1)",
    "Tall      (1x2)",
    "Double    (2x2)",
    "Triple    (3x3)",
    "Wide2     (4x2)",
    "Tall2     (2x4)",
    "Quadruple (4x4)"};

  Open_window(299,190,"Picture & screen sizes");

  Print_in_window( 12, 21,"Picture size:"   ,MC_Dark,MC_Light);
  Window_display_frame      ( 8,17,195, 33);

  Window_set_normal_button(223, 18,67,14,"OK"      ,0,1,SDLK_RETURN); // 1
  Window_set_normal_button(223, 35,67,14,"Cancel"  ,0,1,KEY_ESC);  // 2

  Print_in_window( 12, 37,"Width:"          ,MC_Dark,MC_Light);
  input_width_button=Window_set_input_button( 60, 35,4);            // 3
  
  Print_in_window(108, 37,"Height:"         ,MC_Dark,MC_Light);
  input_button_height=Window_set_input_button(164, 35,4);           // 4

  Window_display_frame      ( 8,72,283,110);
  Window_display_frame_in   (37,84,228,84);
  Window_rectangle          (38,85,226,82,MC_Black);  
  Print_in_window( 16, 76,"OK"              ,MC_Dark,MC_Light);
  Print_in_window( 55, 76,"X    Y"          ,MC_Dark,MC_Light);
  Print_in_window(120, 76,"Win / Full"      ,MC_Dark,MC_Light);
  Print_in_window(219, 76,"Ratio"           ,MC_Dark,MC_Light);
  Print_in_window( 30,170,"\03"             ,MC_Dark,MC_Light);
  Print_in_window( 62,170,"OK"              ,MC_Dark,MC_Light);
  Print_in_window(102,170,"Imperfect"       ,MC_Dark,MC_Light);
  Print_in_window(196,170,"Unsupported"     ,MC_Dark,MC_Light);
  Window_set_special_button(38,86,225,80);                       // 5

  selected_mode=Current_resolution;
  if (selected_mode>=MODELIST_LINES/2 && Nb_video_modes > MODELIST_LINES)
  {
    if (selected_mode<Nb_video_modes-MODELIST_LINES/2)
    {
      list_start=selected_mode-(MODELIST_LINES/2-1);
      cursor_position=(MODELIST_LINES/2-1);
    }
    else
    {
      list_start=Nb_video_modes-MODELIST_LINES;
      cursor_position=selected_mode-list_start;
    }
  }
  else
  {
    list_start=0;
    cursor_position=selected_mode;
  }
  Window_set_scroller_button(271,85,81,Nb_video_modes,MODELIST_LINES,list_start); // 6

  chosen_pixel=Pixel_ratio;
  Print_in_window( 12, 57,"Pixel size:"    ,MC_Dark,MC_Light);
  pixel_button=Window_set_dropdown_button(108,55,17*8,11,17*8,pixel_ratio_labels[Pixel_ratio],1,0,1,LEFT_SIDE|RIGHT_SIDE);    // 7
  for (temp=0;temp<PIXEL_MAX;temp++)
    Window_dropdown_add_item(pixel_button,temp,pixel_ratio_labels[temp]);

  // 10 little buttons for the state of each visible mode
  for (temp=0; temp<MODELIST_LINES && temp < Nb_video_modes; temp++)
    Window_set_normal_button(17,86+(temp<<3),13,7,"",0,1,SDLK_LAST);// 8..17

  // Dummy buttons as explainations of colors
  Window_draw_normal_bouton( 16,170,13,7,"",0,0);
  Check_mode_button( 18,172,0);
  Window_draw_normal_bouton( 48,170,13,7,"",0,0);
  Check_mode_button( 50,172,1);
  Window_draw_normal_bouton( 88,170,13,7,"",0,0);
  Check_mode_button( 90,172,2);
  Window_draw_normal_bouton(182,170,13,7,"",0,0);
  Check_mode_button(184,172,3);
 

  chosen_width=Main_image_width;
  Num2str(chosen_width,str,4);
  Window_input_content(input_width_button,str);

  chosen_height=Main_image_height;
  Num2str(chosen_height,str,4);
  Window_input_content(input_button_height,str);

  Display_modes_list(list_start,cursor_position);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    { case -1: case 0: case 1: case 2:
        break;

      case 3 : // Largeur
        Num2str(chosen_width,str,4);
        Readline(62,37,str,4,1);
        chosen_width=atoi(str);
        // On corrige les dimensions
        if (chosen_width==0)
        {
          chosen_width=1;
          Num2str(chosen_width,str,4);
          Window_input_content(input_width_button,str);
        }
        Display_cursor();
        break;

      case 4 : // Height
        Num2str(chosen_height,str,4);
        Readline(166,37,str,4,1);
        chosen_height=atoi(str);
        // On corrige les dimensions
        if (chosen_height==0)
        {
          chosen_height=1;
          Num2str(chosen_height,str,4);
          Window_input_content(input_button_height,str);
        }
        Display_cursor();
        break;

      case 5: // Liste des modes
        temp=(((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-86)>>3;
        if (temp<Nb_video_modes && ((Mouse_K==2) || (temp!=cursor_position)))
        {
          Hide_cursor();
          if (temp!=cursor_position)
          {
            cursor_position=temp;
            Display_modes_list(list_start,cursor_position);
          }
          selected_mode=list_start+cursor_position;
          // Si l'utilisateur s'est servi du bouton droit de la souris:
          if (Mouse_K==2)
          {
            // On affecte également les dimensions de l'image:
            chosen_width=Video_mode[selected_mode].Width/Pixel_width;
            Num2str(chosen_width,str,4);
            Window_input_content(input_width_button,str);

            chosen_height=Video_mode[selected_mode].Height/Pixel_height;
            Num2str(chosen_height,str,4);
            Window_input_content(input_button_height,str);
          }
          Display_cursor();
        }
        Wait_end_of_click();
        break;

      case 6: // Scroller
        list_start=Window_attribute2;
        selected_mode=list_start+cursor_position;
        Display_modes_list(list_start,cursor_position);
        break;

      case 7: // Pixel size
        chosen_pixel=Window_attribute2;
        break;
        
      default: // Boutons de tag des états des modes
        temp=list_start+clicked_button-8;
        if (Video_mode[temp].Fullscreen==1 && // On n'a pas le droit de cocher le mode fenêtré
            !(Video_mode[temp].State & 128)) // Ni ceux non détectés par SDL
        {
          if (Window_attribute1==LEFT_SIDE)
            Video_mode[temp].State=((Video_mode[temp].State&0x7F)+1)&3;
          else
            Video_mode[temp].State=((Video_mode[temp].State&0x7F)+3)&3;
            
          Hide_cursor();
          //Check_mode_button(19,16+(clicked_button<<3),Video_mode[temp].State);
          Display_modes_list(list_start,cursor_position);
          Display_cursor();
        }
    }

    // Gestion des touches de déplacement dans la liste
    switch (Key)
    {
      case SDLK_UP : // Haut
        if (cursor_position>0)
          cursor_position--;
        else
          if (list_start>0)
            list_start--;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case SDLK_DOWN : // Bas
        if (cursor_position<(MODELIST_LINES-1) && cursor_position<(Nb_video_modes-1))
          cursor_position++;
        else
          if (list_start<Nb_video_modes-MODELIST_LINES)
            list_start++;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case SDLK_PAGEUP : // PageUp
        if (cursor_position>0)
          cursor_position=0;
        else
        {
          if (list_start>(MODELIST_LINES-1))
            list_start-=(MODELIST_LINES-1);
          else
            list_start=0;
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case SDLK_PAGEDOWN : // PageDown
        if (Nb_video_modes<MODELIST_LINES)
          cursor_position=Nb_video_modes-1;
        else if (cursor_position<(MODELIST_LINES-1))
          cursor_position=(MODELIST_LINES-1);
        else
        {
          if (list_start<Nb_video_modes-(MODELIST_LINES*2-1))
            list_start+=(MODELIST_LINES-1);
          else
            list_start=Nb_video_modes-MODELIST_LINES;
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case SDLK_HOME : // Home
        list_start=0;
        cursor_position=0;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case SDLK_END : // End
        if (Nb_video_modes<MODELIST_LINES)
          cursor_position=Nb_video_modes-1;
        else
        {        
          list_start=Nb_video_modes-MODELIST_LINES;
          cursor_position=(MODELIST_LINES-1);
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      default:
        if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Window_help(BUTTON_RESOL, NULL);
          Key=0;
          break;
        }
    }

  }
  while ((clicked_button!=1) && (clicked_button!=2));

  Close_window();

  if (clicked_button==1) // OK
  {
    if (Main_magnifier_mode)
      Unselect_button(BUTTON_MAGNIFIER);

    if ( (chosen_width!=Main_image_width)
      || (chosen_height!=Main_image_height) )
      Resize_image(chosen_width,chosen_height);
    
    if ((Video_mode[selected_mode].State & 3) == 3 ||
      Init_mode_video(
        Video_mode[selected_mode].Width,
        Video_mode[selected_mode].Height,
        Video_mode[selected_mode].Fullscreen,
        chosen_pixel))
    {
      Error(0); // On signale à l'utilisateur que c'est un mode invalide
      Pixel_ratio=PIXEL_SIMPLE;
      Init_mode_video(
        Video_mode[Current_resolution].Width,
        Video_mode[Current_resolution].Height,
        Video_mode[Current_resolution].Fullscreen,
        Pixel_ratio);
    }

    Display_menu();
    Reposition_palette();
    Display_all_screen();
  }
  Paintbrush_X = Mouse_X;
  Paintbrush_Y = Mouse_Y;


  Unselect_button(BUTTON_RESOL);
  Display_cursor();
}


void Button_Safety_resolution(void)
{
  // In windowed mode, do nothing
  if (Current_resolution==0)
  {
    Hide_cursor();
    Unselect_button(BUTTON_RESOL);
    Display_cursor();
    return;
  }
  
  Hide_cursor();

  Unselect_button(BUTTON_MAGNIFIER);
  
  Init_mode_video(
    Video_mode[0].Width, 
    Video_mode[0].Height, 
    Video_mode[0].Fullscreen,
    PIXEL_SIMPLE);
  Current_resolution=0;
  Display_menu();
  Reposition_palette();
  Display_all_screen();

  Unselect_button(BUTTON_RESOL);
  // Le pinceau est affiché à la position du clic et pas 
  Display_cursor();
}


//------------------ Gestion des boutons de dessin à la main -----------------

void Button_Draw(void)
{
  Hide_cursor();
  Start_operation_stack(Selected_freehand_mode);
  Display_cursor();
}


void Button_Draw_switch_mode(void)
{
/* ANCIEN CODE SANS POPUPS */
  Selected_freehand_mode++;
  if (Selected_freehand_mode>OPERATION_FILLED_CONTOUR)
    Selected_freehand_mode=OPERATION_CONTINUOUS_DRAW;

  Hide_cursor();
  Display_sprite_in_menu(BUTTON_DRAW,Selected_freehand_mode);
  Start_operation_stack(Selected_freehand_mode);
  Display_cursor();
/* NOUVEAU CODE AVEC POPUP (EN COURS DE TEST) ***
    short clicked_button;
    Open_popup(16,Menu_Y/Menu_factor_X-32,18,50);
    Window_set_normal_button(1,1,16,16,"A",0,1,KEY_ESC); // 1
    Display_cursor();

    Update_rect(16*Menu_factor_X,Menu_Y-32*Menu_factor_X,18*Menu_factor_X,50*Menu_factor_X);

    do
    {
        while(!Get_input())Wait_VBL();
        clicked_button = Window_get_clicked_button();

        switch(clicked_button)
        {
            case 1:
                Selected_freehand_mode++;
                if (Selected_freehand_mode>OPERATION_FILLED_CONTOUR)
                    Selected_freehand_mode=OPERATION_CONTINUOUS_DRAW;
                break;
        }
    }
    while (Mouse_K);

    Close_popup();
    Display_sprite_in_menu(BUTTON_DRAW,Selected_freehand_mode);
    Start_operation_stack(Selected_freehand_mode);
    Display_cursor();
*/
}


// -- Gestion des boutons de rectangle vide et plein ------------------------

void Button_Empty_rectangle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_EMPTY_RECTANGLE);
  Display_cursor();
}


void Button_Filled_rectangle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_RECTANGLE);
  Display_cursor();
}


// -- Gestion des boutons de cercle (ellipse) vide et plein(e) --------------

void Button_Empty_circle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_EMPTY_CIRCLE);
  Display_cursor();
}


void Button_Empty_ellipse(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_EMPTY_ELLIPSE);
  Display_cursor();
}


void Button_Filled_circle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_CIRCLE);
  Display_cursor();
}


void Button_Filled_ellipse(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_ELLIPSE);
  Display_cursor();
}


// -- Gestion du menu des dégradés ------------------------------------------
void Draw_button_gradient_style(short x_pos,short y_pos,int technique)
{
  short line;

  // On commence par afficher les 2 côtés qui constituent le dégradé de base:
    // Côté gauche (noir)
  Block(Window_pos_X+((x_pos+2)*Menu_factor_X),
        Window_pos_Y+((y_pos+2)*Menu_factor_Y),
        Menu_factor_X*6,
        Menu_factor_Y*10,MC_Black);
    // Côté droit (blanc)
  Block(Window_pos_X+((x_pos+8)*Menu_factor_X),
        Window_pos_Y+((y_pos+2)*Menu_factor_Y),
        Menu_factor_X*5,
        Menu_factor_Y*10,MC_White);

  switch(technique)
  {
    case 1 : // Dégradé de trames simples
      // Au centre, on place 10 lignes tramées simplement
      for (line=2;line<2+10;line++)
        if (line&1)
        {
          // Lignes impaires
          Pixel_in_window(x_pos+ 5,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 7,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 8,y_pos+line,MC_Black);
        }
        else
        {
          // Lignes paires
          Pixel_in_window(x_pos+ 6,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 9,y_pos+line,MC_Black);
        }
      break;
    case 2 : // Dégradé de trames étendues
      // Au centre, on place 10 lignes tramées de façon compliquée
      for (line=2;line<2+10;line++)
        if (line&1)
        {
          // Lignes impaires
          Pixel_in_window(x_pos+ 7,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 8,y_pos+line,MC_Black);
          Pixel_in_window(x_pos+10,y_pos+line,MC_Black);
        }
        else
        {
          // Lignes paires
          Pixel_in_window(x_pos+ 4,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 6,y_pos+line,MC_White);
        }
  }
  
  Update_rect(Window_pos_X+((x_pos+2)*Menu_factor_X),Window_pos_Y+((y_pos+2)*Menu_factor_Y),
                    Menu_factor_X*10,Menu_factor_Y*10);  
}

void Load_gradient_data(int index)
{
  Gradient_lower_bound =Gradient_array[index].Start;
  Gradient_upper_bound =Gradient_array[index].End;
  Gradient_is_inverted          =Gradient_array[index].Inverse;
  Gradient_random_factor=Gradient_array[index].Mix+1;

  Gradient_bounds_range=(Gradient_lower_bound<Gradient_upper_bound)?
                            Gradient_upper_bound-Gradient_lower_bound:
                            Gradient_lower_bound-Gradient_upper_bound;
  Gradient_bounds_range++;

  switch(Gradient_array[index].Technique)
  {
    case 0 : // Degradé de base
      Gradient_function=Gradient_basic;
      break;
    case 1 : // Dégradé de trames simples
      Gradient_function=Gradient_dithered;
      break;
    case 2 : // Dégradé de trames étendues
      Gradient_function=Gradient_extra_dithered;
  }
}

void Draw_gradient_preview(short start_x,short start_y,short width,short height,int index)
{
  short x_pos; // Variables de balayage du block en bas de l'écran.
  short y_pos;
  short end_x;
  short end_y;

  Load_gradient_data(index);

  start_x=Window_pos_X+(start_x*Menu_factor_X);
  start_y=Window_pos_Y+(start_y*Menu_factor_Y);

  Gradient_total_range=width*Menu_factor_X;

  end_x=start_x+Gradient_total_range;
  end_y=start_y+(height*Menu_factor_Y);

  for (y_pos=start_y;y_pos<end_y;y_pos++)
    for (x_pos=start_x;x_pos<end_x;x_pos++)
      Gradient_function(x_pos-start_x,x_pos,y_pos);
  Update_rect(start_x,start_y,width*Menu_factor_X,height*Menu_factor_Y);
}

void Button_Gradients(void)
{
  short clicked_button;
  char  str[3];
  T_Gradient_array backup_gradients[16];
  int   old_current_gradient;
  T_Scroller_button * mix_scroller;
  short old_mouse_x;
  short old_mouse_y;
  byte  old_mouse_k;
  byte  temp_color;
  byte  first_color;
  byte  last_color;
  byte  color;
  byte  click;


  Gradient_pixel=Pixel;
  old_current_gradient=Current_gradient;
  memcpy(backup_gradients,Gradient_array,sizeof(T_Gradient_array)*16);

  Open_window(237,133,"Gradation menu");

  Window_set_palette_button(48,21);                            // 1
    // Définition du scrolleur <=> indice du dégradé dans le tableau
  Window_set_scroller_button(218,22,75,16,1,Current_gradient);  // 2
    // Définition du scrolleur de mélange du dégradé
  mix_scroller = Window_set_scroller_button(31,22,84,256,1,
    Gradient_array[Current_gradient].Mix);                      // 3
    // Définition du bouton de sens
  Window_set_normal_button(8,22,15,14,
    (Gradient_array[Current_gradient].Inverse)?"\033":"\032",0,1,SDLK_TAB); // 4
    // Définition du bouton de technique
  Window_set_normal_button(8,92,15,14,"",0,1,SDLK_TAB|MOD_SHIFT); // 5
  Draw_button_gradient_style(8,92,Gradient_array[Current_gradient].Technique);

  Window_set_normal_button(178,112,51,14,"OK",0,1,SDLK_RETURN);     // 6
  Window_set_normal_button(123,112,51,14,"Cancel",0,1,KEY_ESC);  // 7

  Print_in_window(5,60,"MIX",MC_Dark,MC_Light);

  // On tagge les couleurs qui vont avec
  Tag_color_range(Gradient_array[Current_gradient].Start,Gradient_array[Current_gradient].End);

  Num2str(Current_gradient+1,str,2);
  Print_in_window(215,100,str,MC_Black,MC_Light);

    // On affiche le cadre autour de la préview
  Window_display_frame_in(7,111,110,16);
    // On affiche la preview
  Draw_gradient_preview(8,112,108,14,Current_gradient);

  first_color=last_color=(Gradient_array[Current_gradient].Inverse)?Gradient_array[Current_gradient].End:Gradient_array[Current_gradient].Start;
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;

    clicked_button=Window_clicked_button();

    switch(clicked_button)
    {
      case -1 :
      case  1 : // Palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          temp_color=(clicked_button==1) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);

          if (!old_mouse_k)
          {
            // On vient de clicker

            // On met à jour l'intervalle du dégradé
            first_color=last_color=Gradient_array[Current_gradient].Start=Gradient_array[Current_gradient].End=temp_color;
            // On tagge le bloc
            Tag_color_range(Gradient_array[Current_gradient].Start,Gradient_array[Current_gradient].End);
            // Tracé de la preview:
            Draw_gradient_preview(8,112,108,14,Current_gradient);
          }
          else
          {
            // On maintient le click, on va donc tester si le curseur bouge
            if (temp_color!=last_color)
            {
              // On commence par ordonner la 1ère et dernière couleur du bloc
              if (first_color<temp_color)
              {
                Gradient_array[Current_gradient].Start=first_color;
                Gradient_array[Current_gradient].End  =temp_color;
              }
              else if (first_color>temp_color)
              {
                Gradient_array[Current_gradient].Start=temp_color;
                Gradient_array[Current_gradient].End  =first_color;
              }
              else
                Gradient_array[Current_gradient].Start=Gradient_array[Current_gradient].End=first_color;
              // On tagge le bloc
              Tag_color_range(Gradient_array[Current_gradient].Start,Gradient_array[Current_gradient].End);
              // Tracé de la preview:
              Draw_gradient_preview(8,112,108,14,Current_gradient);
              last_color=temp_color;
            }
          }
          Display_cursor();
        }
        break;
      case  2 : // Nouvel indice de dégradé
        Hide_cursor();
        // Nouvel indice dans Window_attribute2
        Current_gradient=Window_attribute2;

        // On affiche la valeur sous la jauge
        Num2str(Current_gradient+1,str,2);
        Print_in_window(215,100,str,MC_Black,MC_Light);

        // On tagge les couleurs qui vont avec
        Tag_color_range(Gradient_array[Current_gradient].Start,Gradient_array[Current_gradient].End);

        // On affiche le sens qui va avec
        Print_in_window(12,25,(Gradient_array[Current_gradient].Inverse)?"\033":"\032",MC_Black,MC_Light);

        // On raffiche le mélange (jauge) qui va avec
        mix_scroller->Position=Gradient_array[Current_gradient].Mix;
        Window_draw_slider(mix_scroller);

        // On raffiche la technique qui va avec
        Draw_button_gradient_style(8,92,Gradient_array[Current_gradient].Technique);

        // On affiche la nouvelle preview
        Draw_gradient_preview(8,112,108,14,Current_gradient);

        Display_cursor();
        break;
      case  3 : // Nouveau mélange de dégradé
        Hide_cursor();
        // Nouvel mélange dans Window_attribute2
        Gradient_array[Current_gradient].Mix=Window_attribute2;
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,112,108,14,Current_gradient);
        Display_cursor();
        break;
      case  4 : // Changement de sens
        Hide_cursor();
        // On inverse le sens (par un XOR de 1)
        Gradient_array[Current_gradient].Inverse^=1;
        Print_in_window(12,25,(Gradient_array[Current_gradient].Inverse)?"\033":"\032",MC_Black,MC_Light);
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,112,108,14,Current_gradient);
        Display_cursor();
        break;
      case  5 : // Changement de technique
        Hide_cursor();
        // On change la technique par (+1)%3
        Gradient_array[Current_gradient].Technique=(Gradient_array[Current_gradient].Technique+1)%3;
        Draw_button_gradient_style(8,92,Gradient_array[Current_gradient].Technique);
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,112,108,14,Current_gradient);
        Display_cursor();
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
          temp_color=color;

          // On met à jour l'intervalle du dégradé
          first_color=last_color=Gradient_array[Current_gradient].Start=Gradient_array[Current_gradient].End=temp_color;
          // On tagge le bloc
          Tag_color_range(Gradient_array[Current_gradient].Start,Gradient_array[Current_gradient].End);
          // Tracé de la preview:
          Draw_gradient_preview(8,112,108,14,Current_gradient);
          Display_cursor();
          Wait_end_of_click();
        }
        Key=0;
        break;
      default:
        if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Window_help(BUTTON_GRADRECT, NULL);
          Key=0;
          break;
        }
        if (Is_shortcut(Key,0x200+BUTTON_GRADRECT))
          clicked_button=6;
    }
  }
  while (clicked_button<6);

  Close_window();
  // The Grad rect operation uses the same button as Grad menu.
  if (Current_operation != OPERATION_GRAD_RECTANGLE)
    Unselect_button(BUTTON_GRADRECT);
    
  Display_cursor();

  Gradient_pixel=Display_pixel;
  if (clicked_button==7) // Cancel
  {
    Current_gradient=old_current_gradient;
    memcpy(Gradient_array,backup_gradients,sizeof(T_Gradient_array)*16);
    Load_gradient_data(Current_gradient);
  }
}


// -- Gestion des boutons de cercle / ellipse / rectangle dégradés --------------------

void Button_Grad_circle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_GRAD_CIRCLE);
  Display_cursor();
}


void Button_Grad_ellipse(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_GRAD_ELLIPSE);
  Display_cursor();
}


void Button_Grad_rectangle(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_GRAD_RECTANGLE);
  Display_cursor();
}


// -- Gestion du bouton de remplissage ---------------------------------------

void Button_Fill(void)
{
  if (Current_operation!=OPERATION_FILL)
  {
    Hide_cursor();

    if (Current_operation!=OPERATION_REPLACE)
    {
      Paintbrush_shape_before_fill=Paintbrush_shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    }
    else
      if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
           ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
        Print_in_menu("X:       Y:             ",0);
    Start_operation_stack(OPERATION_FILL);
    Display_cursor();
  }
}


void Button_Replace(void)
{
  if (Current_operation!=OPERATION_REPLACE)
  {
    Hide_cursor();
    if (Current_operation!=OPERATION_FILL)
    {
      Paintbrush_shape_before_fill=Paintbrush_shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    }
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
      Print_in_menu("X:       Y:       (    )",0);
    Start_operation_stack(OPERATION_REPLACE);
    Display_cursor();
  }
}


void Button_Unselect_fill(void)
{
  Paintbrush_shape=Paintbrush_shape_before_fill;

  if (Current_operation==OPERATION_REPLACE)
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
      Print_in_menu("X:       Y:             ",0);
}


//---------------------------- Menu des pinceaux -----------------------------

void Button_Paintbrush_menu(void)
{
  short clicked_button;
  short x_pos,y_pos;
  byte index;

  Open_window(310,180,"Paintbrush menu");

  Window_display_frame(8,21,294,132);

  Window_set_normal_button(122,158,67,14,"Cancel",0,1,KEY_ESC); // 1

  for (index=0; index<NB_PAINTBRUSH_SPRITES; index++)
  {
    x_pos=13+(index%12)*24;
    y_pos=27+(index/12)*25;
    Window_set_normal_button(x_pos  ,y_pos  ,20,20,"",0,1,SDLK_LAST);
    Display_paintbrush_in_window(x_pos+2,y_pos+2,index);
  }
  for (index=0; index<BRUSH_CONTAINER_COLUMNS*BRUSH_CONTAINER_ROWS; index++)
  {
    x_pos=13+((index+NB_PAINTBRUSH_SPRITES)%12)*24;
    y_pos=27+((index+NB_PAINTBRUSH_SPRITES)/12)*25;
    Window_set_normal_button(x_pos  ,y_pos  ,20,20,"",0,1,SDLK_LAST);
    Display_stored_brush_in_window(x_pos+2, y_pos+2, index);
  }
  
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_PAINTBRUSHES, NULL);
    // Brush container
    if (clicked_button>(NB_PAINTBRUSH_SPRITES+1))
    {
      index = clicked_button-NB_PAINTBRUSH_SPRITES-2;
      
      if (Window_attribute1==RIGHT_SIDE)
      {
        // Store
        
        x_pos=13+((index+NB_PAINTBRUSH_SPRITES)%12)*24;
        y_pos=27+((index+NB_PAINTBRUSH_SPRITES)/12)*25;
      
        Store_brush(index);
        Hide_cursor();
        Display_stored_brush_in_window(x_pos+2, y_pos+2, index);
        Display_cursor();
  }
      else
      {
        // Restore and exit

        if (Restore_brush(index))
        {
  Close_window();
          break;
        }
      }

    }
    else if (clicked_button>1 && Window_attribute1==LEFT_SIDE)
    // Standard paintbrushes
  {
      Close_window();
    index=clicked_button-2;
    Paintbrush_shape=Gfx->Paintbrush_type[index];
    Paintbrush_width=Gfx->Preset_paintbrush_width[index];
    Paintbrush_height=Gfx->Preset_paintbrush_height[index];
    Paintbrush_offset_X=Gfx->Preset_paintbrush_offset_X[index];
    Paintbrush_offset_Y=Gfx->Preset_paintbrush_offset_Y[index];
    for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
      for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
        Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=Gfx->Paintbrush_sprite[index][y_pos][x_pos];
    Change_paintbrush_shape(Gfx->Paintbrush_type[index]);
      
      break;
    }
    else if (clicked_button==1 || Is_shortcut(Key,0x100+BUTTON_PAINTBRUSHES))
    {
      Close_window();
      break;
    }
  }
  while (1);

  Unselect_button(BUTTON_PAINTBRUSHES);
  Display_cursor();
}


void Button_Brush_monochrome(void)
{
  Hide_cursor();
  // On passe en brosse monochrome:
  Change_paintbrush_shape(PAINTBRUSH_SHAPE_MONO_BRUSH);

  Unselect_button(BUTTON_PAINTBRUSHES);

  Display_cursor();
}

// -- Fonction renvoyant le mode vidéo le plus adapté à l'image chargée -----
#define TOLERANCE_X 8
#define TOLERANCE_Y 4
int Best_video_mode(void)
{
  short best_width,best_height;
  int best_mode;
  short temp_x,temp_y;
  int mode;

  // Si mode fenêtre, on reste dans ce mode.
  if (Current_resolution == 0)
    return 0;
    
  // On commence par borner les dimensions, ou du moins les rendre cohérentes
  if ((Original_screen_X<=0) || (Config.Set_resolution_according_to==2))
    Original_screen_X=Main_image_width;
  else
    if (Original_screen_X<320)
      Original_screen_X=320;

  if ((Original_screen_Y<=0) || (Config.Set_resolution_according_to==2))
    Original_screen_Y=Main_image_height;
  else
    if (Original_screen_Y<200)
      Original_screen_Y=200;

  if ((Original_screen_X>1024) || (Original_screen_Y>768))
  {
    Original_screen_X=1024;
    Original_screen_Y=768;
  }

  // Maintenant on peut chercher le mode qui correspond le mieux
  best_mode=Current_resolution;
  best_width=0;
  best_height=0;


  for (mode=1; mode<Nb_video_modes; mode++)
  {
    if (Video_mode[mode].Fullscreen && (Video_mode[mode].State&3)<2)
    {
      temp_x=Video_mode[mode].Width;
      temp_y=Video_mode[mode].Height;

      if ( (Original_screen_X-TOLERANCE_X<=temp_x)
        && (Original_screen_Y-TOLERANCE_Y<=temp_y) )
        return mode;
      else
      {
        if ( (best_width<=temp_x)
          && (best_height<=temp_y)
          && (temp_x-TOLERANCE_X<=Original_screen_X)
          && (temp_y-TOLERANCE_Y<=Original_screen_Y) )
        {
          best_width=temp_x;
          best_height=temp_y;
          best_mode=mode;
        }
      }
    }
  }

  return best_mode;
}


void Swap_data_of_image_and_brush(void)
{
  char  temp_string[MAX_PATH_CHARACTERS];
  byte  temp_byte;
  short temp_int;


  strcpy(temp_string           ,Brush_file_directory);
  strcpy(Brush_file_directory   ,Main_file_directory);
  strcpy(Main_file_directory,temp_string);

  strcpy(temp_string    ,Brush_filename);
  strcpy(Brush_filename   ,Main_filename);
  strcpy(Main_filename,temp_string);

  temp_byte        =Brush_fileformat;
  Brush_fileformat   =Main_fileformat;
  Main_fileformat=temp_byte;

  temp_byte=Brush_format;
  Brush_format   =Main_format;
  Main_format=temp_byte;

  temp_int            =Brush_fileselector_position;
  Brush_fileselector_position    =Main_fileselector_position;
  Main_fileselector_position=temp_int;

  temp_int            =Brush_fileselector_offset;
  Brush_fileselector_offset    =Main_fileselector_offset;
  Main_fileselector_offset=temp_int;

  strcpy(temp_string           ,Brush_current_directory);
  strcpy(Brush_current_directory   ,Main_current_directory);
  strcpy(Main_current_directory,temp_string);

  strcpy(temp_string    ,Brush_comment);
  strcpy(Brush_comment   ,Main_comment);
  strcpy(Main_comment,temp_string);
}


void Load_picture(byte image)
  // Image=1 => On charge/sauve une image
  // Image=0 => On charge/sauve une brosse
{
  // Données initiales du fichier (au cas où on voudrait annuler)
  char  initial_file_directory[MAX_PATH_CHARACTERS];
  char  initial_filename[MAX_PATH_CHARACTERS];
  byte  initial_file_format;
  byte  do_not_restore;
  byte  use_brush_palette = 0;
  T_Components * initial_palette=NULL;
  byte  old_cursor_shape;
  short initial_main_image_width=Main_image_width;
  short initial_main_image_height=Main_image_height;
  //char  initial_comment[COMMENT_SIZE+1];
  int   new_mode;


  if (!image)
    Swap_data_of_image_and_brush();

  strcpy(initial_file_directory,Main_file_directory);
  strcpy(initial_filename       ,Main_filename);
  initial_file_format=Main_fileformat;

  if (!image)
  {
    initial_palette=(T_Components *)malloc(sizeof(T_Palette));
    memcpy(initial_palette,Main_palette,sizeof(T_Palette));
  }

  do_not_restore=Button_Load_or_Save(1,image);

  if (do_not_restore)
  {
    if (image)
    {
      if (Main_image_is_modified)
        do_not_restore=Confirmation_box("Discard unsaved changes?");
    }
    else
      use_brush_palette=Confirmation_box("Use the palette of the brush?");
  }

  // do_not_restore is modified inside the first if, that's why we check it
  // again here
  if (do_not_restore)
  {
    old_cursor_shape=Cursor_shape;
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    if (image)
    {
      // Si c'est une image qu'on charge, on efface l'ancien commentaire
      // C'est loin d'être indispensable, m'enfin bon...
      if (File_formats[Main_fileformat-1].Backup_done)
        Main_comment[0]='\0';

      Original_screen_X=0;
      Original_screen_Y=0;
    }
    else
      Pixel_load_function=Pixel_load_in_brush;

    Load_image(image);

    if (!image)
    {
      if (!use_brush_palette)
        memcpy(Main_palette,initial_palette,sizeof(T_Palette));

      if (File_error==3) // On ne peut pas allouer la brosse
      {
        free(Brush);
        Brush=(byte *)malloc(1*1);
        Brush_height=1;
        Brush_width=1;
        *Brush=Fore_color;

    	free(Smear_brush);
        Smear_brush=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
        Smear_brush_height=MAX_PAINTBRUSH_SIZE;
        Smear_brush_width=MAX_PAINTBRUSH_SIZE;
      }
      else
      {
        Brush_width=Main_image_width;
        Brush_height=Main_image_height;
        Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
        Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
      }

      Tiling_offset_X=0;
      Tiling_offset_Y=0;

      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);

      Main_image_width=initial_main_image_width;
      Main_image_height=initial_main_image_height;
      Pixel_load_function=Pixel_load_in_current_screen;

      Select_button(BUTTON_DRAW,LEFT_SIDE);
      if (Config.Auto_discontinuous)
      {
        // On se place en mode Dessin discontinu à la main
        while (Current_operation!=OPERATION_DISCONTINUOUS_DRAW)
          Select_button(BUTTON_DRAW,RIGHT_SIDE);
      }
      Hide_cursor();
      // On passe en brosse couleur:
      Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
    }
    else
    {
      Hide_cursor();
      Cursor_shape=old_cursor_shape;
    }

    if ( (File_error==1) || (!File_formats[Main_fileformat-1].Backup_done) )
    {
      do_not_restore=0;
      if (File_error!=1)
        Compute_optimal_menu_colors(Main_palette);
    }
    else
    {
      if (image)
      {
        if (Main_magnifier_mode)
        {
          Draw_menu_button_frame(BUTTON_MAGNIFIER,0);
          Pixel_preview=Pixel_preview_normal;
          Main_magnifier_mode=0;
        }

        new_mode=Best_video_mode();
        if ((Config.Auto_set_res) && (new_mode!=Current_resolution))
        {
          Init_mode_video(
            Video_mode[new_mode].Width,
            Video_mode[new_mode].Height,
            Video_mode[new_mode].Fullscreen,
            Pixel_ratio);
          Display_menu();
        }
        // In window mode, activate wide or tall pixels if the image says so.
        else if (!Video_mode[Current_resolution].Fullscreen &&
          ((Ratio_of_loaded_image == PIXEL_WIDE &&
            Pixel_ratio != PIXEL_WIDE && Pixel_ratio != PIXEL_WIDE2) ||
            (Ratio_of_loaded_image == PIXEL_TALL &&
            Pixel_ratio != PIXEL_TALL && Pixel_ratio != PIXEL_TALL2)))
        {
          Init_mode_video(
            Video_mode[Current_resolution].Width,
            Video_mode[Current_resolution].Height,
            Video_mode[Current_resolution].Fullscreen,
            Ratio_of_loaded_image);
            Display_menu();
        }
        else
        {
          Main_offset_X=0;
          Main_offset_Y=0;
          Compute_limits();
          Compute_paintbrush_coordinates();
        }
      }

      Compute_optimal_menu_colors(Main_palette);
      Redraw_layered_image();
      Display_all_screen();

      if (image)
        Main_image_is_modified=0;
    }
    Display_menu();
    Display_cursor();
  }

  free(initial_palette);

  if (!do_not_restore)
  {
    strcpy(Main_filename       ,initial_filename);
    strcpy(Main_file_directory,initial_file_directory);
    Main_fileformat    =initial_file_format;
  }

  if (!image)
    Swap_data_of_image_and_brush();
  Hide_cursor();
  Print_filename();
  Display_cursor();
  Set_palette(Main_palette);
}


void Button_Load(void)
{
  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page_main(Main_backups->Pages);

  Load_picture(1);
}


void Button_Reload(void)
{
  byte old_cursor_shape;
  int  new_mode;

  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page_main(Main_backups->Pages);

  if ( (!Main_image_is_modified) || Confirmation_box("Discard unsaved changes ?") )
  {
    Hide_cursor();
    old_cursor_shape=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    Original_screen_X=0;
    Original_screen_Y=0;
    Load_image(1);

    Hide_cursor();
    Cursor_shape=old_cursor_shape;

    if (File_error!=1)
    {
      if (Main_magnifier_mode)
      {
        Draw_menu_button_frame(BUTTON_MAGNIFIER,0);
        Pixel_preview=Pixel_preview_normal;
        Main_magnifier_mode=0;
      }

      new_mode=Best_video_mode();     
      if ( ((Config.Auto_set_res) && (new_mode!=Current_resolution)) &&
           (!Resolution_in_command_line) )
      {
        Init_mode_video(
        Video_mode[new_mode].Width,
        Video_mode[new_mode].Height,
        Video_mode[new_mode].Fullscreen,
        Pixel_ratio);
        Display_menu();
      }
      // In window mode, activate wide or tall pixels if the image says so.
      else if (!Video_mode[Current_resolution].Fullscreen &&
        ((Ratio_of_loaded_image == PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE && Pixel_ratio != PIXEL_WIDE2) ||
          (Ratio_of_loaded_image == PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL && Pixel_ratio != PIXEL_TALL2)))
      {
        Init_mode_video(
          Video_mode[Current_resolution].Width,
          Video_mode[Current_resolution].Height,
          Video_mode[Current_resolution].Fullscreen,
          Ratio_of_loaded_image);
          Display_menu();
      }
      else
      {
        Main_offset_X=0;
        Main_offset_Y=0;
        Compute_limits();
        Compute_paintbrush_coordinates();
      }
      Redraw_layered_image();
      Display_all_screen();

      Main_image_is_modified=0;
    }
  }
  else
    Hide_cursor();

  Compute_optimal_menu_colors(Main_palette);
  Display_menu();
  if (Config.Display_image_limits)
    Display_image_limits();

  Unselect_button(BUTTON_LOAD);

  Display_cursor();
}


void Backup_filename(char * fname, char * backup_name)
{
  short i;

  strcpy(backup_name,fname);
  for (i=strlen(fname)-strlen(Main_filename); backup_name[i]!='.'; i++);
  backup_name[i+1]='\0';
  strcat(backup_name,"BAK");
}


void Backup_existing_file(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  char new_filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier backup

  Get_full_filename(filename,0);
  // Calcul du nom complet du fichier backup
  Backup_filename(filename,new_filename);

  File_error=0;

  // On fait un backup si le nom du fichier n'est pas celui qu'on a choisi
  // pour nommer les backups (c'est évident!).
  if (strcmp(new_filename,filename))
  {
    // S'il y avait déjà un fichier Backup, on l'efface
    if ((File_exists(new_filename))
     && (remove(new_filename)!=0))
      File_error=1;

    if ((!File_error)
     && (rename(filename,new_filename)!=0))
      File_error=1;
  }
}


void Save_picture(byte image)
  // image=1 => On charge/sauve une image
  // image=0 => On charge/sauve une brosse
{
  // Données initiales du fichier (au cas où on voudrait annuler)
  char  initial_file_directory[MAX_PATH_CHARACTERS];
  char  initial_filename[MAX_PATH_CHARACTERS];
  byte  initial_file_format;
  byte  do_not_restore;
  byte  old_cursor_shape;
  short initial_main_image_width=Main_image_width;
  short initial_main_image_height=Main_image_height;
  //char  initial_comment[COMMENT_SIZE+1];


  if (!image)
    Swap_data_of_image_and_brush();

  strcpy(initial_file_directory,Main_file_directory);
  strcpy(initial_filename       ,Main_filename);
  initial_file_format=Main_fileformat;

  do_not_restore=Button_Load_or_Save(0,image);

  if (do_not_restore && File_exists(Main_filename))
  {
    do_not_restore=Confirmation_box("Erase old file ?");
    if ((do_not_restore) && (Config.Backup))
    {
      Backup_existing_file();
      if (File_error)
      {
        do_not_restore=0;
        Error(0);
      }
    }
  }

  if (do_not_restore)
  {
    old_cursor_shape=Cursor_shape;
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    if (image)
      Save_image(image);
    else
    {
      Main_image_width=Brush_width;
      Main_image_height=Brush_height;
      Save_image(image);
      Main_image_width=initial_main_image_width;
      Main_image_height=initial_main_image_height;
    }

    Hide_cursor();
    Cursor_shape=old_cursor_shape;

    if ((File_error==1) || (!File_formats[Main_fileformat-1].Backup_done))
      do_not_restore=0;

    Display_cursor();
  }

  if (!do_not_restore)
  {
    strcpy(Main_filename       ,initial_filename);
    strcpy(Main_file_directory,initial_file_directory);
           Main_fileformat    =initial_file_format;
  }

  if (!image)
    Swap_data_of_image_and_brush();

  Print_filename();
  Set_palette(Main_palette);
}


void Button_Save(void)
{
  Save_picture(1);
}


void Button_Autosave(void)
{
  byte old_cursor_shape;
  static char filename[MAX_PATH_CHARACTERS];
  byte file_already_exists;


  Get_full_filename(filename,0);
  file_already_exists=File_exists(filename);

  if ( (!file_already_exists) || Confirmation_box("Erase old file ?") )
  {
    if ((file_already_exists) && (Config.Backup))
      Backup_existing_file();
    else
      File_error=0;

    Hide_cursor();

    if (!File_error)
    {
      old_cursor_shape=Cursor_shape;
      Cursor_shape=CURSOR_SHAPE_HOURGLASS;
      Display_cursor();

      Save_image(1);

      Hide_cursor();
      Cursor_shape=old_cursor_shape;
    }
    else
      Error(0);
  }
  else
    Hide_cursor();

  Unselect_button(BUTTON_SAVE);

  Display_cursor();
}


// -- Gestion des boutons de ligne ------------------------------------------

void Button_Lines(void)
{
  Hide_cursor();
  Start_operation_stack(Selected_line_mode);
  Display_cursor();
}


void Button_Lines_switch_mode(void)
{
  if (Selected_line_mode==OPERATION_LINE)
    Selected_line_mode=OPERATION_K_LIGNE;
  else
  {
    if (Selected_line_mode==OPERATION_K_LIGNE)
      Selected_line_mode=OPERATION_CENTERED_LINES;
    else
      Selected_line_mode=OPERATION_LINE;
  }

  Hide_cursor();
  Display_sprite_in_menu(BUTTON_LINES,Selected_line_mode-OPERATION_LINE+7);
  Start_operation_stack(Selected_line_mode);
  Display_cursor();
}


// -- Button de brosse ------------------------------------------------------

void Button_Brush(void)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_GRAB_BRUSH)
    Start_operation_stack(OPERATION_GRAB_BRUSH);
  else
    Unselect_button(BUTTON_BRUSH);

  Display_cursor();
}


void Button_Unselect_brush(void)
{
  // On fait de notre mieux pour restaurer l'ancienne opération:
  Start_operation_stack(Operation_before_interrupt);
}


void Button_Restore_brush(void)
{
  Hide_cursor();
  // On passe en brosse couleur:
  Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);

  Unselect_button(BUTTON_BRUSH);
  Unselect_button(BUTTON_POLYBRUSH);

  Display_cursor();
}


// -- Button de prise de brosse au lasso ------------------------------------

void Button_Lasso(void)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_POLYBRUSH)
  {
    Paintbrush_shape_before_lasso=Paintbrush_shape;
    Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    Start_operation_stack(OPERATION_POLYBRUSH);
  }
  else
    Unselect_button(BUTTON_POLYBRUSH);

  Display_cursor();
}


void Button_Unselect_lasso(void)
{
  // On fait de notre mieux pour restaurer l'ancienne opération:
  Start_operation_stack(Operation_before_interrupt);
  Paintbrush_shape=Paintbrush_shape_before_lasso;
}


// -- Button de pipette -----------------------------------------------------

void Button_Colorpicker(void)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_COLORPICK)
  {
    Colorpicker_color=-1;
    Start_operation_stack(OPERATION_COLORPICK);
    Paintbrush_shape_before_colorpicker=Paintbrush_shape;
    Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    if (Operation_before_interrupt!=OPERATION_REPLACE)
      if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
           ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
        Print_in_menu("X:       Y:       (    )",0);
  }
  else
    Unselect_button(BUTTON_COLORPICKER);

  Display_cursor();
}


void Button_Unselect_colorpicker(void)
{
  if (Operation_before_interrupt!=OPERATION_REPLACE)
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
      Print_in_menu("X:       Y:             ",0);

  // On fait de notre mieux pour restaurer l'ancienne opération:
  if (Current_operation==OPERATION_COLORPICK)
  {
    Start_operation_stack(Operation_before_interrupt);
    Paintbrush_shape=Paintbrush_shape_before_colorpicker;
  }
}


  // -- Inversion de la couleur Fore et de la couleur Back --
void Button_Invert_foreback(void)
{
  byte temp_color;

  temp_color=Fore_color;
  Fore_color        =Back_color;
  Back_color        =temp_color;

  Hide_cursor();
  Frame_menu_color(Back_color);
  Frame_menu_color(Fore_color);
  Reposition_palette();
  Display_foreback();
  Unselect_button(BUTTON_COLORPICKER);
  Display_cursor();  
}


// -- Gestion du bouton Loupe -----------------------------------------------

byte Coming_from_zoom_factor_menu=0;

void Button_Magnify(void)
{
  Hide_cursor();
  if ( (Current_operation==OPERATION_MAGNIFY) || (Main_magnifier_mode) )
  {
    Unselect_button(BUTTON_MAGNIFIER);
  }
  else
  {
    Old_main_offset_X=Main_offset_X;
    Old_main_offset_Y=Main_offset_Y;
    Compute_magnifier_data();
    if ((!Config.Fast_zoom) || (Mouse_Y>=Menu_Y) || Coming_from_zoom_factor_menu)
    {
      Coming_from_zoom_factor_menu=0;
      Start_operation_stack(OPERATION_MAGNIFY);
    }
    else
    { /* Ceci est de la duplication de code de presque toute l'opération de */
      /* la loupe... Il serait peut-être plus propre de faire une procédure */
      /* qui s'en charge... */
      // On passe en mode loupe
      Main_magnifier_mode=1;

      // La fonction d'affichage dans la partie image est désormais un affichage
      // spécial loupe.
      Pixel_preview=Pixel_preview_magnifier;

      // On calcule l'origine de la loupe
      Main_magnifier_offset_X=Mouse_X-(Main_magnifier_width>>1);
      Main_magnifier_offset_Y=Mouse_Y-(Main_magnifier_height>>1);

      // Calcul du coin haut_gauche de la fenêtre devant être zoomée DANS L'ECRAN
      if (Main_magnifier_offset_X+Main_magnifier_width>=Limit_right-Main_offset_X)
        Main_magnifier_offset_X=Limit_right-Main_magnifier_width-Main_offset_X+1;
      if (Main_magnifier_offset_Y+Main_magnifier_height>=Limit_bottom-Main_offset_Y)
        Main_magnifier_offset_Y=Limit_bottom-Main_magnifier_height-Main_offset_Y+1;

      // Calcul des coordonnées absolues de ce coin DANS L'IMAGE
      Main_magnifier_offset_X+=Main_offset_X;
      Main_magnifier_offset_Y+=Main_offset_Y;

      if (Main_magnifier_offset_X<0)
        Main_magnifier_offset_X=0;
      if (Main_magnifier_offset_Y<0)
        Main_magnifier_offset_Y=0;

      // On calcule les bornes visibles dans l'écran
      Position_screen_according_to_zoom();
      Compute_limits();
      Display_all_screen();

      // Repositionner le curseur en fonction des coordonnées visibles
      Compute_paintbrush_coordinates();
    }
  }
  Display_cursor();
  Update_rect(0,0,0,0);
}


void Button_Magnify_menu(void)
{
  short clicked_button;

  Open_window(141,114,"Zoom factors");

  Window_set_normal_button(45,88,51,14,"Cancel",0,1,KEY_ESC); // 1

  Window_set_normal_button(  9,25,27,14, "x2",0,Main_magnifier_factor!= 2,SDLK_F1);    // 2
  Window_set_normal_button( 41,25,27,14, "x3",0,Main_magnifier_factor!= 3,SDLK_F2);    // 3
  Window_set_normal_button( 73,25,27,14, "x4",0,Main_magnifier_factor!= 4,SDLK_F3);    // 4
  Window_set_normal_button(105,25,27,14, "x5",0,Main_magnifier_factor!= 5,SDLK_F4);    // 5
  Window_set_normal_button(  9,45,27,14, "x6",0,Main_magnifier_factor!= 6,SDLK_F5);    // 6
  Window_set_normal_button( 41,45,27,14, "x8",0,Main_magnifier_factor!= 8,SDLK_F6);    // 7
  Window_set_normal_button( 73,45,27,14,"x10",0,Main_magnifier_factor!=10,SDLK_F7);    // 8
  Window_set_normal_button(105,45,27,14,"x12",0,Main_magnifier_factor!=12,SDLK_F8);    // 9
  Window_set_normal_button(  9,65,27,14,"x14",0,Main_magnifier_factor!=14,SDLK_F9);    // 10
  Window_set_normal_button( 41,65,27,14,"x16",0,Main_magnifier_factor!=16,SDLK_F10);    // 11
  Window_set_normal_button( 73,65,27,14,"x18",0,Main_magnifier_factor!=18,SDLK_F11);    // 12
  Window_set_normal_button(105,65,27,14,"x20",0,Main_magnifier_factor!=20,SDLK_F12);    // 13
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_MAGNIFIER, NULL);
    else if (Is_shortcut(Key,0x200+BUTTON_MAGNIFIER))
      clicked_button=1;
  }
  while (clicked_button<=0);

  Close_window();

  if (clicked_button>1)
  {
    Menu_Y=Menu_Y_before_window;
    Change_magnifier_factor(clicked_button-2);
  }

  if ( (clicked_button==1) && (!Main_magnifier_mode) && (Current_operation!=OPERATION_MAGNIFY) ) // Cancel
    Unselect_button(BUTTON_MAGNIFIER);

  Display_all_screen();
  Display_cursor();
  Update_rect(Main_separator_position,0,Screen_width-Main_separator_position,Menu_Y);

  if ( (clicked_button>1) && (!Main_magnifier_mode) && (Current_operation!=OPERATION_MAGNIFY) ) // Passage en mode zoom
  {
    Coming_from_zoom_factor_menu=1;
    Select_button(BUTTON_MAGNIFIER,LEFT_SIDE);
  }

}


void Button_Unselect_magnifier(void)
{
  if (Main_magnifier_mode)
  {
    // On sort du mode loupe
    Main_magnifier_mode=0;

    /*
    // --> Recalculer le décalage de l'écran lorsqu'on sort de la loupe <--
    // Centrage "brut" de lécran par rapport à la loupe
    Main_offset_X=Main_magnifier_offset_X-((Screen_width-Main_magnifier_width)>>1);
    Main_offset_Y=Main_magnifier_offset_Y-((Menu_Y-Main_magnifier_height)>>1);
    */
    // Correction en cas de débordement de l'image
    if (Old_main_offset_X+Screen_width>Main_image_width)
      Main_offset_X=Main_image_width-Screen_width;
    else
      Main_offset_X=Old_main_offset_X;
    if (Main_offset_X<0)
      Main_offset_X=0;

    if (Old_main_offset_Y+Menu_Y>Main_image_height)
      Main_offset_Y=Main_image_height-Menu_Y;
    else
      Main_offset_Y=Old_main_offset_Y;
    if (Main_offset_Y<0)
      Main_offset_Y=0;

    // La fonction d'affichage dans l'image est désormais un affichage normal.
    Pixel_preview=Pixel_preview_normal;

    // Calculer les bornes visibles dans l'écran
    Compute_limits();
    Display_all_screen();  // <=> Display_screen();
    // Repositionner le curseur en fonction des coordonnées visibles
    Compute_paintbrush_coordinates();
    
    Old_MX = -1;
    Old_MY = -1;
  }
  else // On fait de notre mieux pour restaurer l'ancienne opération:
    Start_operation_stack(Operation_before_interrupt);
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

  T_Special_button * input_x_button;
  T_Special_button * input_y_button;
  T_Special_button * input_dx_button;
  T_Special_button * input_dy_button;

  char str[3];


  Open_window(133,98,"Grid");

  Window_set_normal_button(12,72,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(70,72,51,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Print_in_window(19,26, "X:",MC_Dark,MC_Light);
  input_x_button = Window_set_input_button(37,24,2); // 3
  Num2str(chosen_X,str,2);
  Window_input_content(input_x_button,str);

  Print_in_window(19,47, "Y:",MC_Dark,MC_Light);
  input_y_button = Window_set_input_button(37,45,2); // 4
  Num2str(chosen_Y,str,2);
  Window_input_content(input_y_button,str);

  Print_in_window(69,26,"dX:",MC_Dark,MC_Light);
  input_dx_button = Window_set_input_button(95,24,2); // 5
  Num2str(dx_selected,str,2);
  Window_input_content(input_dx_button,str);

  Print_in_window(69,47,"dY:",MC_Dark,MC_Light);
  input_dy_button = Window_set_input_button(95,45,2); // 6
  Num2str(dy_selected,str,2);
  Window_input_content(input_dy_button,str);
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case 3 :
        Num2str(chosen_X,str,2);
        Readline(39,26,str,2,1);
        chosen_X=atoi(str);
        // On corrige les dimensions
        if ((!chosen_X) || (chosen_X>80))
        {
          if (!chosen_X)
            chosen_X=1;
          else
            chosen_X=80;
          Num2str(chosen_X,str,2);
          Window_input_content(input_x_button,str);
        }
        if (dx_selected>=chosen_X)
        {
          dx_selected=chosen_X-1;
          Num2str(dx_selected,str,2);
          Window_input_content(input_dx_button,str);
        }
        Display_cursor();
        break;
      case 4 :
        Num2str(chosen_Y,str,2);
        Readline(39,47,str,2,1);
        chosen_Y=atoi(str);
        // On corrige les dimensions
        if ((!chosen_Y) || (chosen_Y>80))
        {
          if (!chosen_Y)
            chosen_Y=1;
          else
            chosen_Y=80;
          Num2str(chosen_Y,str,2);
          Window_input_content(input_y_button,str);
        }
        if (dy_selected>=chosen_Y)
        {
          dy_selected=chosen_Y-1;
          Num2str(dy_selected,str,2);
          Window_input_content(input_dy_button,str);
        }
        Display_cursor();
        break;
      case 5 :
        Num2str(dx_selected,str,2);
        Readline(97,26,str,2,1);
        dx_selected=atoi(str);
        // On corrige les dimensions
        if (dx_selected>79)
          dx_selected=79;
        if (dx_selected>=chosen_X)
          dx_selected=chosen_X-1;

        Num2str(dx_selected,str,2);
        Window_input_content(input_dx_button,str);

        Display_cursor();
        break;
      case 6 :
        Num2str(dy_selected,str,2);
        Readline(97,47,str,2,1);
        dy_selected=atoi(str);
        // On corrige les dimensions
        if (dy_selected>79)
          dy_selected=79;
        if (dy_selected>=chosen_Y)
          dy_selected=chosen_Y-1;

        Num2str(dy_selected,str,2);
        Window_input_content(input_dy_button,str);

        Display_cursor();
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "GRID");
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  if (clicked_button==2) // OK
  {
    Snap_width=chosen_X;
    Snap_height=chosen_Y;
    Snap_offset_X=dx_selected;
    Snap_offset_Y=dy_selected;
    Snap_mode=1;
  }

  Close_window();

  if ( (clicked_button==2) && (!Snap_mode) )
    Button_Snap_mode();

  Display_cursor();
}

void Button_Show_grid(void)
{
  Show_grid = !Show_grid;
  Hide_cursor();
  Display_all_screen();
  Display_cursor();
} 

// ----------------------- Modifications de brosse ---------------------------

void Button_Brush_FX(void)
{
  short clicked_button;
  short index;

  Open_window(310,162,"Brush effects");

  Window_display_frame(  6,19,298,61);
  Window_display_frame(  6,83,122,53);
  Window_display_frame(137,83,167,53);

  Window_set_normal_button(236,141, 67,14,"Cancel"          ,0,1,KEY_ESC); // 1
  Window_set_normal_button( 19, 46, 27,14,"X\035"           ,0,1,Config_Key[SPECIAL_FLIP_X][0]); // 2
  Window_set_normal_button( 19, 61, 27,14,"Y\022"           ,0,1,Config_Key[SPECIAL_FLIP_Y][0]); // 3
  Window_set_normal_button( 58, 46, 37,14,"90°"             ,0,1,Config_Key[SPECIAL_ROTATE_90][0]); // 4
  Window_set_normal_button( 96, 46, 37,14,"180°"            ,0,1,Config_Key[SPECIAL_ROTATE_180][0]); // 5
  Window_set_normal_button( 58, 61, 75,14,"any angle"       ,0,1,Config_Key[SPECIAL_ROTATE_ANY_ANGLE][0]); // 6
  Window_set_normal_button(145, 46, 67,14,"Stretch"         ,0,1,Config_Key[SPECIAL_STRETCH][0]); // 7
  Window_set_normal_button(145, 61, 67,14,"Distort"         ,0,1,Config_Key[SPECIAL_DISTORT][0]); // 8
  Window_set_normal_button(155, 99,131,14,"Recolorize"      ,0,1,Config_Key[SPECIAL_RECOLORIZE_BRUSH][0]); // 9
  Window_set_normal_button(155,117,131,14,"Get brush colors",0,1,Config_Key[SPECIAL_GET_BRUSH_COLORS][0]); // 10

  // Boutons représentant les coins du brush handle: (HG,HD,C,BG,BD)
  Window_set_normal_button( 75, 90,11,11,"",0,1,Config_Key[SPECIAL_TOP_LEFT_ATTACHMENT][0]); // 11
  Window_set_normal_button(103, 90,11,11,"",0,1,Config_Key[SPECIAL_TOP_RIGHT_ATTACHMENT][0]); // 12
  Window_set_normal_button( 89,104,11,11,"",0,1,Config_Key[SPECIAL_CENTER_ATTACHMENT][0]); // 13
  Window_set_normal_button( 75,118,11,11,"",0,1,Config_Key[SPECIAL_BOTTOM_LEFT_ATTACHMENT][0]); // 14
  Window_set_normal_button(103,118,11,11,"",0,1,Config_Key[SPECIAL_BOTTOM_RIGHT_ATTACHMENT][0]); // 15

  Window_set_normal_button(224,46,67,14,"Outline",0,1,Config_Key[SPECIAL_OUTLINE][0]); // 16
  Window_set_normal_button(224,61,67,14,"Nibble" ,0,1,Config_Key[SPECIAL_NIBBLE][0]); // 17

  Window_set_normal_button(  7,141, 60,14,"Load",0,1,Config_Key[SPECIAL_LOAD_BRUSH][0]); // 18
  Window_set_normal_button( 70,141, 60,14,"Save",0,1,Config_Key[SPECIAL_SAVE_BRUSH][0]); // 19

  Print_in_window( 80, 24,"Shape modifications",MC_Dark,MC_Light);
  Print_in_window( 10, 36,"Mirror",MC_Dark,MC_Light);
  Print_in_window( 72, 36,"Rotate",MC_Dark,MC_Light);
  Print_in_window(155, 36,"Deform",MC_Dark,MC_Light);
  Print_in_window(230, 36,"Borders",MC_Dark,MC_Light);
  Print_in_window(141, 88,"Colors modifications",MC_Dark,MC_Light);
  Print_in_window( 20,102,"Brush",MC_Dark,MC_Light);
  Print_in_window( 16,110,"handle",MC_Dark,MC_Light);

  // Dessin des pointillés pour le "brush handle"
  for (index=0; index<13; index+=2)
  {
    Pixel_in_window( 88+index, 92,MC_Dark);
    Pixel_in_window( 88+index,126,MC_Dark);
    Pixel_in_window( 77,103+index,MC_Dark);
    Pixel_in_window(111,103+index,MC_Dark);
  }
  // Dessin des coins et du centre pour les boutons du "brush handle"
    // Coin HG
  Block(Window_pos_X+(Menu_factor_X* 77),Window_pos_Y+(Menu_factor_Y* 92),Menu_factor_X*7,Menu_factor_Y,MC_Black);
  Block(Window_pos_X+(Menu_factor_X* 77),Window_pos_Y+(Menu_factor_Y* 92),Menu_factor_X,Menu_factor_Y*7,MC_Black);
    // Coin HD
  Block(Window_pos_X+(Menu_factor_X*105),Window_pos_Y+(Menu_factor_Y* 92),Menu_factor_X*7,Menu_factor_Y,MC_Black);
  Block(Window_pos_X+(Menu_factor_X*111),Window_pos_Y+(Menu_factor_Y* 92),Menu_factor_X,Menu_factor_Y*7,MC_Black);
    // Centre
  Block(Window_pos_X+(Menu_factor_X* 91),Window_pos_Y+(Menu_factor_Y*109),Menu_factor_X*7,Menu_factor_Y,MC_Black);
  Block(Window_pos_X+(Menu_factor_X* 94),Window_pos_Y+(Menu_factor_Y*106),Menu_factor_X,Menu_factor_Y*7,MC_Black);
    // Coin BG
  Block(Window_pos_X+(Menu_factor_X* 77),Window_pos_Y+(Menu_factor_Y*126),Menu_factor_X*7,Menu_factor_Y,MC_Black);
  Block(Window_pos_X+(Menu_factor_X* 77),Window_pos_Y+(Menu_factor_Y*120),Menu_factor_X,Menu_factor_Y*7,MC_Black);
    // Coin BD
  Block(Window_pos_X+(Menu_factor_X*105),Window_pos_Y+(Menu_factor_Y*126),Menu_factor_X*7,Menu_factor_Y,MC_Black);
  Block(Window_pos_X+(Menu_factor_X*111),Window_pos_Y+(Menu_factor_Y*120),Menu_factor_X,Menu_factor_Y*7,MC_Black);
  
  Update_window_area(0,0,Window_width, Window_height);
  
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      Key=0;
      Window_help(BUTTON_BRUSH_EFFECTS, NULL);
    }
    else if (Is_shortcut(Key,0x100+BUTTON_BRUSH_EFFECTS))
    {
      clicked_button=1;
    }
  }
  while (clicked_button<=0);

  Close_window();
  Unselect_button(BUTTON_BRUSH_EFFECTS);

  // Gestion du bouton clické
  switch (clicked_button)
  {
    case  2 : // Flip X
      Flip_X_lowlevel(Brush, Brush_width, Brush_height);
      break;
    case  3 : // Flip Y
      Flip_Y_lowlevel(Brush, Brush_width, Brush_height);
      break;
    case  4 : // 90° Rotation
      Rotate_90_deg();
      break;
    case  5 : // 180° Rotation
      if (Brush_height&1)
      Rotate_180_deg_lowlevel(Brush, Brush_width, Brush_height);
      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);
      break;
    case  6 : // Any angle rotation
      Start_operation_stack(OPERATION_ROTATE_BRUSH);
      break;
    case  7 : // Stretch
      Start_operation_stack(OPERATION_STRETCH_BRUSH);
      break;
    case  8 : // Distort
      Start_operation_stack(OPERATION_DISTORT_BRUSH);
      break;
    case  9 : // Recolorize
      Remap_brush();
      break;
    case 10 : // Get brush colors
      Display_cursor();
      Get_colors_from_brush();
      Hide_cursor();
      break;
    case 11 : // Brush Attachment: Top-Left
      Brush_offset_X=0;
      Brush_offset_Y=0;
      break;
    case 12 : // Brush Attachment: Top-Right
      Brush_offset_X=(Brush_width-1);
      Brush_offset_Y=0;
      break;
    case 13 : // Brush Attachment: Center
      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);
      break;
    case 14 : // Brush Attachment: Bottom-Left
      Brush_offset_X=0;
      Brush_offset_Y=(Brush_height-1);
      break;
    case 15 : // Brush Attachment: Bottom-Right
      Brush_offset_X=(Brush_width-1);
      Brush_offset_Y=(Brush_height-1);
      break;
    case 16 : // Outline
      Outline_brush();
      break;
    case 17 : // Nibble
      Nibble_brush();
      break;
    case 18 : // Load
      Display_cursor();
      Load_picture(0);
      Hide_cursor();
      break;
    case 19 : // Save
      Display_cursor();
      Save_picture(0);
      Hide_cursor();
      break;
  }

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
  short x,y,i,j;
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
      Num2str(chosen_matrix[i][j]=Smooth_matrix[i][j],str,2);
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
                 str,2,1);
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
    // Partie droite
  Print_in_window(129,37," ",MC_Black,MC_Light);
  Print_in_window(129,57," ",MC_Black,MC_Light);
  Print_in_window(129,74," ",MC_Black,MC_Light);

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

  Open_window(140,118,"Transparency");

  Print_in_window(16,23,"Opacity:",MC_Dark,MC_Light);
  Window_set_input_button(87,21,3);                               // 1
  Print_in_window(117,23,"%",MC_Dark,MC_Light);
  Window_set_normal_button(16,34,108,14,"Interpolate",1,1,SDLK_i); // 2
  Window_display_frame(12,18,116,34);

  Window_set_normal_button(16,54,108,14,"Additive"   ,2,1,SDLK_d); // 3
  Window_set_normal_button(16,71,108,14,"Subtractive",1,1,SDLK_s); // 4

  Window_set_normal_button(16,94, 51,14,"Cancel"     ,0,1,KEY_ESC); // 5
  Window_set_normal_button(73,94, 51,14,"OK"         ,0,1,SDLK_RETURN); // 6

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
        Readline(89,23,str,3,1);
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
      case 2: // Méthode interpolée
      case 3: // Méthode additive
      case 4: // Méthode soustractive
        selected_mode=clicked_button-2;
        Hide_cursor();
        Button_Colorize_display_selection(selected_mode);
        Display_cursor();
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "TRANSPARENCY");
      else if (Is_shortcut(Key,SPECIAL_COLORIZE_MENU))
        clicked_button=6;
  }
  while (clicked_button<5);

  Close_window();

  if (clicked_button==6) // OK
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
      Readline(93,23,str,4,1);
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
      Readline(93,37,str,4,1);
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
}

//---------------------------- Courbes de Bézier ----------------------------

void Button_Curves(void)
{
  Hide_cursor();
  Start_operation_stack(Selected_curve_mode);
  Display_cursor();
}


void Button_Curves_switch_mode(void)
{
  if (Selected_curve_mode==OPERATION_4_POINTS_CURVE)
    Selected_curve_mode=OPERATION_3_POINTS_CURVE;
  else
    Selected_curve_mode=OPERATION_4_POINTS_CURVE;

  Hide_cursor();
  Display_sprite_in_menu(BUTTON_CURVES,Selected_curve_mode-OPERATION_3_POINTS_CURVE+5);
  Start_operation_stack(Selected_curve_mode);
  Display_cursor();
}


//--------------------------------- Spray -----------------------------------

void Button_Airbrush(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_AIRBRUSH);
  Display_cursor();
}


void Refresh_airbrush_settings(byte selected_color, byte update_slider)
{
  char  str[3];

  if (update_slider)
  {
    Window_scroller_button_list->Position=49-Airbrush_multi_flow[selected_color];
    Window_draw_slider(Window_scroller_button_list);
  }
  Num2str(Airbrush_multi_flow[selected_color],str,2);
  Print_in_window(196,130,str,MC_Black,MC_Light);
  
  Update_rect(Window_pos_X+(Menu_factor_X*(Window_palette_button_list->Pos_X+4+(selected_color >> 4)*10)),
      Window_pos_Y+(Menu_factor_Y*(Window_palette_button_list->Pos_Y+3+(selected_color & 15)* 5)),
      Menu_factor_X<<1,Menu_factor_Y*5);
}


void Button_Airbrush_menu(void)
{
  static byte spray_init=0;
  short  clicked_button;
  char   str[4];
  word   index;
  byte   selected_color=Fore_color;
  byte   old_airbrush_mode     =Airbrush_mode;
  short  old_airbrush_size     =Airbrush_size;
  byte   old_airbrush_delay    =Airbrush_delay;
  byte   old_airbrush_mono_flow=Airbrush_mono_flow;
  byte   old_airbrush_multi_flow[256];
  T_Special_button * input_size_button;
  T_Special_button * input_delay_button;
  T_Special_button * input_flow_button;
  T_Special_button * input_init_button;
  word old_mouse_x;
  word old_mouse_y;
  byte old_mouse_k;
  byte color;
  byte click;


  memcpy(old_airbrush_multi_flow,Airbrush_multi_flow,256);


  Open_window(226,170,"Spray");

  Window_set_normal_button(110,148,51,14,"Cancel"    ,0,1,KEY_ESC); // 1
  Window_set_normal_button(166,148,51,14,"OK"        ,0,1,SDLK_RETURN); // 2

  Window_set_scroller_button(178,62,74,50,1,49-Airbrush_multi_flow[selected_color]); // 3

  Window_set_palette_button(7,56);                                 // 4

  Window_set_normal_button(  8,148,83,14,"Mode:     ",0,1,SDLK_TAB); // 5
  if (Airbrush_mode)
    Print_in_window(50,151," Mono",MC_Black,MC_Light);
  else
    Print_in_window(50,151,"Multi",MC_Black,MC_Light);

  Window_set_normal_button(194, 62,19,14,"+1"        ,0,1,SDLK_KP_PLUS); // 6
  Window_set_normal_button(194, 79,19,14,"-1"        ,0,1,SDLK_KP_MINUS); // 7
  Window_set_normal_button(194, 96,19,14,"x2"        ,0,1,SDLK_KP_MULTIPLY); // 8
  Window_set_normal_button(194,113,19,14,"÷2"        ,0,1,SDLK_KP_ENTER); // 9

  Window_set_normal_button(  8, 37,43,14,"Clear"     ,1,1,SDLK_c); // 10

  Print_in_window(142,25,"Size:"     ,MC_Dark,MC_Light);
  input_size_button = Window_set_input_button(186,23,3);                // 11
  Num2str(Airbrush_size,str,3);
  Window_input_content(input_size_button,str);

  Print_in_window(142,39,"Delay:"    ,MC_Dark,MC_Light);
  input_delay_button = Window_set_input_button(194,37,2);               // 12
  Num2str(Airbrush_delay,str,2);
  Window_input_content(input_delay_button,str);

  Print_in_window( 27,24,"Mono-Flow:",MC_Dark,MC_Light);
  input_flow_button = Window_set_input_button(111,22,2);           // 13
  Num2str(Airbrush_mono_flow,str,2);
  Window_input_content(input_flow_button,str);

  Print_in_window( 67,40,"Init:",MC_Dark,MC_Light);
  input_init_button = Window_set_input_button(111,38,2);                // 14
  Num2str(spray_init,str,2);
  Window_input_content(input_init_button,str);

  Window_display_frame(173,56,45,86);
  Window_display_frame(137,19,81,33);

  // On tagge toutes les couleurs utilisées
  for (index=0; index<256; index++)
    if (Airbrush_multi_flow[index])
      Stencil_tag_color(index,MC_Black);
  // Et enfin, on tagge la couleur sélectionnée
  Stencil_tag_color(selected_color,MC_White);
  Refresh_airbrush_settings(selected_color,0);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();
  Stencil_update_color(selected_color);


  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;

    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case  0 :
      case  2 : // OK
        break;

      case  1 : // Cancel
        Airbrush_mode     =old_airbrush_mode;
        Airbrush_size     =old_airbrush_size;
        Airbrush_delay    =old_airbrush_delay;
        Airbrush_mono_flow=old_airbrush_mono_flow;
        memcpy(Airbrush_multi_flow,old_airbrush_multi_flow,256);
        break;

      case  3 : // Scroller
        Hide_cursor();
        Airbrush_multi_flow[selected_color]=49-Window_attribute2;
        Refresh_airbrush_settings(selected_color,0);
        Display_cursor();
        break;

      case -1 :
      case  4 : // Palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          Stencil_tag_color(selected_color,(Airbrush_multi_flow[selected_color])?MC_Black:MC_Light);
          Stencil_update_color(selected_color);
          // Mettre la couleur sélectionnée à jour suivant le click
          selected_color=(clicked_button==4) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);
          if (Mouse_K==2)
            Airbrush_multi_flow[selected_color]=0;
          else
            if (Airbrush_multi_flow[selected_color]==0)
              Airbrush_multi_flow[selected_color]=spray_init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_tag_color(selected_color,MC_White);
          Refresh_airbrush_settings(selected_color,1);
          Display_cursor();
          Stencil_update_color(selected_color);
        }
        break;

      case  5 : // Toggle Mode
        Airbrush_mode=(Airbrush_mode+1)&1;
        Hide_cursor();
        if (Airbrush_mode)
          Print_in_window(50,151," Mono",MC_Black,MC_Light);
        else
          Print_in_window(50,151,"Multi",MC_Black,MC_Light);
        Update_rect(Window_pos_X+50*Menu_factor_X,Window_pos_Y+151*Menu_factor_Y,5*8*Menu_factor_X,8*Menu_factor_Y);
        Display_cursor();
        break;

      case  6 : // +1
        for (index=0; index<256; index++)
        {
          if ( (Airbrush_multi_flow[index]) && (Airbrush_multi_flow[index]<49) )
            Airbrush_multi_flow[index]++;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  7 : // -1
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index]>1)
            Airbrush_multi_flow[index]--;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  8 : // x2
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index])
          {
            Airbrush_multi_flow[index]<<=1;
            if (Airbrush_multi_flow[index]>49)
              Airbrush_multi_flow[index]=49;
          }
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  9 : // ÷2
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index]>1)
            Airbrush_multi_flow[index]>>=1;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case 10 : // Clear
        memset(Airbrush_multi_flow,0,256);
        // On raffiche les infos de la couleur sélectionnée
        Refresh_airbrush_settings(selected_color,1);
        // On efface les anciens TAGs
        Window_clear_tags();
        // Tagger la couleur sélectionnée en blanc
        Stencil_tag_color(selected_color,MC_White);
        Stencil_update_color(selected_color);
        break;

      case 11 : // Size
        Num2str(Airbrush_size,str,3);
        Readline(188,25,str,3,1);
        Airbrush_size=atoi(str);
        // On corrige les dimensions
        if (Airbrush_size>256)
        {
          Airbrush_size=256;
          Num2str(Airbrush_size,str,3);
          Window_input_content(input_size_button,str);
        }
        else if (!Airbrush_size)
        {
          Airbrush_size=1;
          Num2str(Airbrush_size,str,3);
          Window_input_content(input_size_button,str);
        }
        Display_cursor();
        break;

      case 12 : // Delay
        Num2str(Airbrush_delay,str,2);
        Readline(196,39,str,2,1);
        Airbrush_delay=atoi(str);
        // On corrige le delai
        if (Airbrush_delay>99)
        {
          Airbrush_delay=99;
          Num2str(Airbrush_delay,str,2);
          Window_input_content(input_delay_button,str);
        }
        Display_cursor();
        break;

      case 13 : // Mono-Flow
        Num2str(Airbrush_mono_flow,str,2);
        Readline(113,24,str,2,1);
        Airbrush_mono_flow=atoi(str);
        // On corrige le flux
        if (!Airbrush_mono_flow)
        {
          Airbrush_mono_flow=1;
          Num2str(Airbrush_mono_flow,str,2);
          Window_input_content(input_flow_button,str);
        }
        Display_cursor();
        break;

      case 14 : // Init
        Num2str(spray_init,str,2);
        Readline(113,40,str,2,1);
        spray_init=atoi(str);
        // On corrige la valeur
        if (spray_init>=50)
        {
          spray_init=49;
          Num2str(spray_init,str,2);
          Window_input_content(input_init_button,str);
        }
        Display_cursor();
        break;
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
          Stencil_tag_color(selected_color,(Airbrush_multi_flow[selected_color])?MC_Black:MC_Light);
          Stencil_update_color(selected_color);
          // Mettre la couleur sélectionnée à jour suivant le click
          selected_color=color;
          if (click==2)
            Airbrush_multi_flow[selected_color]=0;
          else
            if (Airbrush_multi_flow[selected_color]==0)
              Airbrush_multi_flow[selected_color]=spray_init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_tag_color(selected_color,MC_White);
          Refresh_airbrush_settings(selected_color,1);
          Display_cursor();
          Stencil_update_color(selected_color);
          Wait_end_of_click();
        }
        Key=0;
        break;
      default:
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
      {
        Window_help(BUTTON_AIRBRUSH, NULL);
        Key=0;
        break;
      }
      if (Is_shortcut(Key,0x200+BUTTON_AIRBRUSH))
      {
        clicked_button=2;
        break;
      }
    }
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  Close_window();

/*
  //   Tant que l'on aura pas résolu le problème du désenclenchement du mode
  // de dessin précedent, il faudra laisser ça en remarque et donc passer en
  // spray même si on a clické sur Cancel (idem pour OK (un peu plus bas)).
  if (clicked_button==1) // Cancel
  {
    if (Current_operation!=OPERATION_AIRBRUSH)
      Unselect_button(BUTTON_AIRBRUSH);
  }
*/

  Display_cursor();

/*
  if (clicked_button==2) // OK
*/
    if (Current_operation!=OPERATION_AIRBRUSH)
      Select_button(BUTTON_AIRBRUSH,LEFT_SIDE);
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
        Brush_width=Sieve_width;
        Brush_height=Sieve_height;
        free(Brush);
        Brush=(byte *)malloc(((long)Brush_height)*Brush_width);
        for (y_pos=0; y_pos<Sieve_height; y_pos++)
          for (x_pos=0; x_pos<Sieve_width; x_pos++)
            Pixel_in_brush(x_pos,y_pos,(Sieve[x_pos][y_pos])?Fore_color:Back_color);
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


// -- Gestion des boutons de polygone vide et plein -------------------------

void Button_polygon(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYGON);
  Display_cursor();
}


void Button_Polyform(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYFORM);
  Display_cursor();
}


void Button_Polyfill(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYFILL);
  Display_cursor();
}


void Button_Filled_polyform(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_POLYFORM);
  Display_cursor();
}


// -- Boutons d'ajustement de l'image ---------------------------------------

void Button_Adjust(void)
{
  Hide_cursor();
  Start_operation_stack(OPERATION_SCROLL);
  Display_cursor();
}



// -- Menu des effets (Shade, Stencil, etc...) ------------------------------

void Display_effect_sprite(short sprite_number, short start_x, short start_y)
{
  short x,y,x_pos,y_pos;

  for (y=0,y_pos=start_y;y<MENU_SPRITE_HEIGHT;y++,y_pos++)
    for (x=0,x_pos=start_x;x<MENU_SPRITE_WIDTH;x++,x_pos++)
      Pixel_in_window(x_pos,y_pos,Gfx->Effect_sprite[sprite_number][y][x]);

  Update_rect(ToWinX(start_x),ToWinY(start_y),MENU_SPRITE_WIDTH*Menu_factor_X,MENU_SPRITE_HEIGHT*Menu_factor_Y);
}


void Display_effect_state(short x, short y, char * label, byte state)
{
  Block(Window_pos_X+(x*Menu_factor_X),Window_pos_Y+(y*Menu_factor_Y),
        12*Menu_factor_X,Menu_factor_Y<<3,MC_Light);

  Print_in_window(x,y,label,(state)?MC_White:MC_Black,MC_Light);
  if (state)
    Print_in_window(x+56,y,":ON ",MC_White,MC_Light);
  else
    Print_in_window(x+56,y,":OFF",MC_Black,MC_Light);
}

void Display_effect_states(void)
{
  Display_effect_state( 30, 24,"Shade"  ,Shade_mode);
  Display_effect_state( 30, 43,"Q-shade",Quick_shade_mode);
  Display_effect_state( 30, 62,"Transp.",Colorize_mode);
  Display_effect_state( 30, 81,"Smooth" ,Smooth_mode);
  Display_effect_state( 30,100,"Smear"  ,Smear_mode);
  Display_effect_state(176, 24,"Stencil",Stencil_mode);
  Display_effect_state(176, 43,"Mask"   ,Mask_mode);
  Display_effect_state(176, 62,"Sieve"  ,Sieve_mode);
  Display_effect_state(176, 81,"Grid"   ,Snap_mode);
  Display_effect_state(176,100,"Tiling" ,Tiling_mode);
}


void Display_feedback_state(void)
{
  Print_in_window(159,134,(Config.FX_Feedback)?"YES":" NO",MC_Black,MC_Light);
}


void Button_Effects(void)
{
  short clicked_button;
  byte exit_by_close_button=0;

  Open_window(270,152,"Drawing modes (effects)");

  Window_set_normal_button(  7, 19, 16,16,"",0,1,Config_Key[SPECIAL_SHADE_MODE][0]); // 1
  Window_set_normal_button(  7, 38, 16,16,"",0,1,Config_Key[SPECIAL_QUICK_SHADE_MODE][0]); // 2
  Window_set_normal_button(  7, 57, 16,16,"",0,1,Config_Key[SPECIAL_COLORIZE_MODE][0]); // 3
  Window_set_normal_button(  7, 76, 16,16,"",0,1,Config_Key[SPECIAL_SMOOTH_MODE][0]); // 4
  Window_set_normal_button(  7, 95, 16,16,"",0,1,Config_Key[SPECIAL_SMEAR_MODE][0]); // 5
  Window_set_normal_button(153, 19, 16,16,"",0,1,Config_Key[SPECIAL_MASK_MODE][0]); // 6
  Window_set_normal_button(153, 38, 16,16,"",0,1,Config_Key[SPECIAL_STENCIL_MODE][0]); // 7
  Window_set_normal_button(153, 57, 16,16,"",0,1,Config_Key[SPECIAL_SIEVE_MODE][0]); // 8
  Window_set_normal_button(153, 76, 16,16,"",0,1,Config_Key[SPECIAL_GRID_MODE][0]); // 9
  Window_set_normal_button(153, 95, 16,16,"",0,1,Config_Key[SPECIAL_TILING_MODE][0]); // 10
  Window_set_normal_button(195,131, 68,14,"Close",0,1,SDLK_RETURN); // 11
  Window_set_normal_button(  7,131, 68,14,"All off",0,1,SDLK_DELETE); // 12
  Window_set_normal_button( 83,131,104,14,"Feedback:   ",1,1,SDLK_f); // 13
  Display_feedback_state();
  Display_effect_sprite(0,  8,20);
  Display_effect_sprite(0,  8,39);
  Display_effect_sprite(1,  8,58);
  Display_effect_sprite(2,  8,77);
  Display_effect_sprite(8,  8,96);
  Display_effect_sprite(4,154,20);
  Display_effect_sprite(7,154,39);
  Display_effect_sprite(5,154,58);
  Display_effect_sprite(6,154,77);
  Display_effect_sprite(3,154,96);
  Display_effect_states();

  Print_in_window(12,117,"click: Left:Switch / Right:Edit",MC_Dark,MC_Light);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    if (Key==KEY_ESC || Is_shortcut(Key,0x100+BUTTON_EFFECTS))
    {
      clicked_button=11;
      Key=0;
    }
    else if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      // Aide contextuelle
      switch(Window_get_clicked_button())
      {
        case 1:
          Window_help(BUTTON_EFFECTS, "SHADE");
          break;
        case 2:
          Window_help(BUTTON_EFFECTS, "QUICK SHADE");
          break;
        case 3:
          Window_help(BUTTON_EFFECTS, "TRANSPARENCY");
          break;
        case 4:
          Window_help(BUTTON_EFFECTS, "SMOOTH");
          break;
        case 5:
          Window_help(BUTTON_EFFECTS, "SMEAR");
          break;
        case 6:
          Window_help(BUTTON_EFFECTS, "STENCIL");
          break;
        case 7:
          Window_help(BUTTON_EFFECTS, "MASK");
          break;
        case 8:
          Window_help(BUTTON_EFFECTS, "SIEVE");
          break;
        case 9:
          Window_help(BUTTON_EFFECTS, "GRID");
          break;
        case 10:
          Window_help(BUTTON_EFFECTS, "TILING");
          break;
        default:
          Window_help(BUTTON_EFFECTS, NULL);
      }
      // Hack because we have used Window_get_clicked_button()
      Input_sticky_control=0;
      //
    }

    switch (clicked_button)
    {
      case 1 : // Shade
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Shade_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Shade_menu();
          clicked_button=11;
        }
        break;
      case 2 : // Quick-shade
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Quick_shade_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Quick_shade_menu();
          clicked_button=11;
        }
        break;
      case 3 : // Colorize / Transparency
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Colorize_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Colorize_menu();
          clicked_button=11;
        }
        break;
      case 4 : // Smooth
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Smooth_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Smooth_menu();
          clicked_button=11;
        }
        break;
      case 5 : // Smear
        Button_Smear_mode();
        Hide_cursor();
        Display_effect_states();
        Display_cursor();
        break;
      case 6 : // Stencil
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Stencil_mode();
          Hide_cursor();
          Display_effect_state(176,24,"Stencil",Stencil_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Stencil_menu();
          clicked_button=11;
        }
        break;
      case 7 : // Mask
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Mask_mode();
          Hide_cursor();
          Display_effect_state(176,43,"Mask",Mask_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Mask_menu();
          clicked_button=11;
        }
        break;
      case 8 : // Sieve
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Sieve_mode();
          Hide_cursor();
          Display_effect_state(176,62,"Sieve",Sieve_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Sieve_menu();
          clicked_button=11;
        }
        break;
      case 9 : // Grid
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Snap_mode();
          Hide_cursor();
          Display_effect_state(176,81,"Grid",Snap_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Grid_menu();
          clicked_button=11;
        }
        break;
      case 10 : // Tiling
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Tiling_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Tiling_menu();
          clicked_button=11;
        }
        break;
      case 11 : // Close
        exit_by_close_button=1;
        break;
      case 12 : // All off
        Effects_off();
        Hide_cursor();
        Display_effect_states();
        Display_cursor();
        break;
      case 13 : // Feedback (pour Colorize et Shade)
        if ((Config.FX_Feedback=!Config.FX_Feedback)) //!!!
          FX_feedback_screen=Main_backups->Pages->Image[Main_current_layer];
          // Main_screen
        else
          FX_feedback_screen=Main_backups->Pages->Next->Image[Main_current_layer];
          // Screen_backup
        Hide_cursor();
        Display_feedback_state();
        Display_cursor();
        break;
    }
  }
  while (clicked_button!=11);

  if (exit_by_close_button)
    Close_window();
  else
    Hide_cursor();

  if (!(Shade_mode||Quick_shade_mode||Colorize_mode||Smooth_mode||Tiling_mode||Smear_mode||Stencil_mode||Mask_mode||Sieve_mode||Snap_mode))
    Unselect_button(BUTTON_EFFECTS);

  Display_cursor();
}

// Callback to display a font name in the list
void Draw_one_font_name(word x, word y, word index, byte highlighted)
{
  Print_in_window(x,y,Font_label(index), MC_Black, (highlighted)?MC_Dark:MC_Light);
}

void Button_Text()
{
  static char str[256]="";
  static int font_size=32;
  static int antialias=1;
  static short list_start=0; // index de le premiere fonte dans le selector
  static short cursor_position=0; // index de la ligne active dans le selector
  static short selected_font_index=0;
  static short is_bold=0;
  static short is_italic=0;

  byte * new_brush=NULL;
  int new_width;
  int new_height;
  int clicked_button;  
  const int NB_FONTS=8;
  char size_buffer[3];
  T_Special_button * input_size_button;
  T_Special_button * input_text_button;
  T_Special_button * preview_button;
  T_Special_button * font_list_button;
  T_Scroller_button * font_scroller;
  T_List_button * font_list;
  
  byte redraw_is_needed=1;
  byte preview_is_needed=1;
  
  Open_window(288,180,"Text");

  // Texte saisi
  Print_in_window(6,20,"Text:",MC_Dark,MC_Light);
  input_text_button = Window_set_input_button(48,18,29); // 1
  
  // TrueType options
  Window_display_frame_in(182,34,100,68);
  Print_in_window(199,31,"TrueType", MC_Dark, MC_Light);
  // AA
  Window_set_normal_button(188,58,13,11,antialias?"X":" ",0,1,SDLK_a); // 2
  Print_in_window(206,60,"AntiAlias", MC_Dark, MC_Light);
  // Bold
  Window_set_normal_button(188,72,13,11,is_bold?"X":" ",0,1,SDLK_b); // 3
  Print_in_window(206,75,"Bold", MC_Dark, MC_Light);
  // Italic
  Window_set_normal_button(188,86,13,11,is_italic?"X":" ",0,1,SDLK_i); // 4
  Print_in_window(206,89,"Italic", MC_Dark, MC_Light);
  
  // Scroller des fontes
  font_scroller = Window_set_scroller_button(165,35,NB_FONTS*8,Nb_fonts,NB_FONTS,list_start); // 5
  // Liste des fontes disponibles
  font_list_button = Window_set_special_button(8,35,152,NB_FONTS*8); // 6
  Window_display_frame_in(7, 33, 154, NB_FONTS*8+4);
  
  // Taille texte
  input_size_button = Window_set_input_button(220,43,3); // 7
  Window_set_repeatable_button(202,43,13,11,"-",0,1,SDLK_LAST); // 8
  Window_set_repeatable_button(251,43,13,11,"+",0,1,SDLK_LAST); // 9
  
  // Preview
  preview_button = Window_set_special_button(8,106,273,50); // 10
  Window_display_frame_in(7, 105, 275, 52);
  
  Window_set_normal_button(8,160,40,14,"OK",0,1,SDLK_RETURN); // 11
  Window_set_normal_button(54,160,60,14,"Cancel",0,1,KEY_ESC); // 12
  
  // List of fonts
  font_list = Window_set_list_button(font_list_button, font_scroller, Draw_one_font_name); // 13
  // Restore its settings from last passage in screen
  font_list->List_start = list_start;
  font_list->Cursor_position = cursor_position;
  
  Window_redraw_list(font_list);
  
  Update_window_area(0,0,Window_width, Window_height);
  
  // str texte
  Window_input_content(input_text_button,str);
  // Taille police
  redraw_is_needed=1;
  // --

  while (1)
  {
    if (redraw_is_needed)
    {
      // Taille
      Num2str(font_size,size_buffer,3);
      Window_input_content(input_size_button,size_buffer);
    }
    if (preview_is_needed)
    {
      const char * preview_string = "AaBbCcDdEeFf012345";
      if (str[0])
        preview_string=str;
      Window_rectangle(8, 106, 273, 50,Back_color);
      free(new_brush);
      new_brush = Render_text(preview_string, selected_font_index, font_size, antialias, is_bold, is_italic, &new_width, &new_height);
      if (new_brush)
      {
        Display_brush(
          new_brush,
          Window_pos_X+preview_button->Pos_X*Menu_factor_X,
          Window_pos_Y+preview_button->Pos_Y*Menu_factor_Y,
          0,
          0,
          Min(preview_button->Width*Menu_factor_X, new_width),
          Min(preview_button->Height*Menu_factor_Y, new_height),
          Back_color,
          new_width);
      }
      Update_rect(
        Window_pos_X+preview_button->Pos_X*Menu_factor_X,
        Window_pos_Y+preview_button->Pos_Y*Menu_factor_Y,
        preview_button->Width*Menu_factor_X,
        preview_button->Height*Menu_factor_Y);
    }
    if (redraw_is_needed || preview_is_needed)
    {    
      redraw_is_needed=0;
      preview_is_needed=0;
      Display_cursor();
    }
  
    clicked_button=Window_clicked_button();
    if (clicked_button==0)
    {
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
        Window_help(BUTTON_TEXT, NULL);
      else if (Is_shortcut(Key,0x100+BUTTON_TEXT))
        clicked_button=12;
    }
    switch(clicked_button)
    {
      case 1: // Texte saisi
      Readline_ex(50,20,str,29,250,0);
      preview_is_needed=1;
      break;

      case 2: // AA
      antialias = (antialias==0);
      Hide_cursor();
      Print_in_window(191,60,antialias?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;

      case 3: // Bold
      is_bold = (is_bold==0);
      Hide_cursor();
      Print_in_window(191,74,is_bold?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;

      case 4: // Italic
      is_italic = (is_italic==0);
      Hide_cursor();
      Print_in_window(191,88,is_italic?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;
      
      case 5: // Scroller des fontes
      /* Cannot happen, event is catched by the list control */
      break;
      
      case 6: // Selecteur de fonte
      /* Cannot happen, event is catched by the list control */
      break;
      
      case 13: // Font selection
        selected_font_index = Window_attribute2;
        Hide_cursor();
        preview_is_needed=1;
      break;
                  
      case 7: // Taille du texte (nombre)
      Readline(222,45,size_buffer,3,1);
      font_size=atoi(size_buffer);
      // On corrige les dimensions
      if (font_size < 1)
      {
        font_size = 1;
      }
      else if (font_size>500)
      {
        font_size = 500;
      }
      redraw_is_needed=1;
      preview_is_needed=1;
      break;
      
      case 8: // Taille -
      if (font_size > 1)
      {
        font_size--;
        Hide_cursor();
        redraw_is_needed=1;
        preview_is_needed=1;
      }
      break;
      
      case 9: // Taille +
      if (font_size < 255)
      {
        font_size++;
        Hide_cursor();
        redraw_is_needed=1;
        preview_is_needed=1;
      }
      break;
      
    
      case 11: // OK
      // Save the selector settings
      list_start = font_list->List_start;
      cursor_position = font_list->Cursor_position;
      
      if (!new_brush)
      {
        // Si echec de rendu
        Close_window();
        Unselect_button(BUTTON_TEXT);
        Display_cursor();
        Error(0);
        return;
      }
      free(Brush);
    
      Brush=new_brush;
      Brush_width=new_width;
      Brush_height=new_height;
      Brush_offset_X=Brush_width>>1;
      Brush_offset_Y=Brush_height>>1;
 
      // Fermeture
      Close_window();
      Unselect_button(BUTTON_TEXT);
      
      // On passe en brosse:
      Display_cursor();
      if (antialias || !TrueType_font(selected_font_index))
        Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
      else
        Change_paintbrush_shape(PAINTBRUSH_SHAPE_MONO_BRUSH);

      Select_button(BUTTON_DRAW,LEFT_SIDE);
      if (Config.Auto_discontinuous)
      {
        // On se place en mode Dessin discontinu à la main
        while (Current_operation!=OPERATION_DISCONTINUOUS_DRAW)
          Select_button(BUTTON_DRAW,RIGHT_SIDE);
      }
      //Display_cursor();
      return;
      
      case 12: // Cancel
      // Save the selector settings
      list_start = font_list->List_start;
      cursor_position = font_list->Cursor_position;
      
      free(new_brush);
	  new_brush = NULL;
      Close_window();
      Unselect_button(BUTTON_TEXT);
      Display_cursor();
      return;
    }
  }
}

void Display_stored_brush_in_window(word x_pos,word y_pos,int index)
{
  if (Brush_container[index].Paintbrush_shape < PAINTBRUSH_SHAPE_MAX)
  {
    int x,y;
    int offset_x=0, offset_y=0;
    //int brush_offset_x=0, brush_offset_y=0;
    
    // Determine draw offset (small brushes are stacked on corner of their preview)
    if (Brush_container[index].Width<BRUSH_CONTAINER_PREVIEW_WIDTH)
      offset_x = (BRUSH_CONTAINER_PREVIEW_WIDTH-Brush_container[index].Width)/2;
    if (Brush_container[index].Height<BRUSH_CONTAINER_PREVIEW_HEIGHT)
      offset_y = (BRUSH_CONTAINER_PREVIEW_HEIGHT-Brush_container[index].Height)/2;
    // Determine corner pixel of paintbrush to draw (if bigger than preview area) 
    //

    // Clear
    Window_rectangle(x_pos,y_pos,BRUSH_CONTAINER_PREVIEW_WIDTH,BRUSH_CONTAINER_PREVIEW_HEIGHT,MC_Light);

    // Draw up to 16x16
    for (y=0; y<Brush_container[index].Height && y<BRUSH_CONTAINER_PREVIEW_HEIGHT; y++)
    {
      for (x=0; x<Brush_container[index].Width && x<BRUSH_CONTAINER_PREVIEW_WIDTH; x++)
      {
        byte color;
        if (Brush_container[index].Paintbrush_shape <= PAINTBRUSH_SHAPE_MISC)
          color = Brush_container[index].Thumbnail[y][x]?MC_Black:MC_Light;
        else
          color = Brush_container[index].Thumbnail[y][x];
        Pixel_in_window(x_pos+x+offset_x,y_pos+y+offset_y,color);
      }
    }
    Update_window_area(x_pos,y_pos,BRUSH_CONTAINER_PREVIEW_WIDTH,BRUSH_CONTAINER_PREVIEW_HEIGHT);
    
  }
}

void Store_brush(int index)
{
  if (Brush_container[index].Paintbrush_shape < PAINTBRUSH_SHAPE_MAX)
  {
    // Free previous stored brush
    Brush_container[index].Paintbrush_shape = PAINTBRUSH_SHAPE_MAX;
    free(Brush_container[index].Brush);
    Brush_container[index].Brush = NULL;
  }

  // Store a mono brush
  if (Paintbrush_shape <= PAINTBRUSH_SHAPE_MISC)
  {
    int x,y;
    int brush_offset_x=0, brush_offset_y=0;
    
    Brush_container[index].Paintbrush_shape=Paintbrush_shape;
    Brush_container[index].Width=Paintbrush_width;
    Brush_container[index].Height=Paintbrush_height;
    //memcpy(Brush_container[index].Palette,Main_palette,sizeof(T_Palette));
    // Preview: pick center for big mono brush
    if (Paintbrush_width>BRUSH_CONTAINER_PREVIEW_WIDTH)
      brush_offset_x = (Paintbrush_width-BRUSH_CONTAINER_PREVIEW_WIDTH)/2;
    if (Paintbrush_height>BRUSH_CONTAINER_PREVIEW_HEIGHT)
      brush_offset_y = (Paintbrush_height-BRUSH_CONTAINER_PREVIEW_HEIGHT)/2;

    for (y=0; y<BRUSH_CONTAINER_PREVIEW_HEIGHT && y<Paintbrush_height; y++)
      for (x=0; x<BRUSH_CONTAINER_PREVIEW_WIDTH && x<Paintbrush_width; x++)
        Brush_container[index].Thumbnail[y][x]=Paintbrush_sprite[((y+brush_offset_y)*MAX_PAINTBRUSH_SIZE)+x+brush_offset_x];
    // Re-init the rest
    Brush_container[index].Transp_color=0;
  }
  if (Paintbrush_shape == PAINTBRUSH_SHAPE_COLOR_BRUSH ||
     Paintbrush_shape == PAINTBRUSH_SHAPE_MONO_BRUSH)
  {
    Brush_container[index].Brush=(byte *)malloc(Brush_width*Brush_height);
    if (Brush_container[index].Brush)
    {
      Brush_container[index].Paintbrush_shape=Paintbrush_shape;
      Brush_container[index].Width=Brush_width;
      Brush_container[index].Height=Brush_height;

      memcpy(Brush_container[index].Brush, Brush,Brush_height*Brush_width);

      // Scale for preview
      if (Brush_width>BRUSH_CONTAINER_PREVIEW_WIDTH ||
          Brush_height>BRUSH_CONTAINER_PREVIEW_HEIGHT)
      {
        // Scale
        Rescale(Brush, Brush_width, Brush_height, (byte *)(Brush_container[index].Thumbnail), BRUSH_CONTAINER_PREVIEW_WIDTH, BRUSH_CONTAINER_PREVIEW_HEIGHT, 0, 0);
      }
      else
      {
        // Direct copy
        Copy_part_of_image_to_another(Brush, 0,0,Brush_width, Brush_height,Brush_width,(byte *)(Brush_container[index].Thumbnail),0,0,BRUSH_CONTAINER_PREVIEW_WIDTH);
      }
    }
    else
    {
      Error(0);
    }
  }
}

byte Restore_brush(int index)
{
  byte shape;
  word x_pos;
  word y_pos;
  
  shape = Brush_container[index].Paintbrush_shape;
  
  if (shape == PAINTBRUSH_SHAPE_MAX)
    return 0;
  // Mono brushes
  if (shape <= PAINTBRUSH_SHAPE_MISC)
  {
    Paintbrush_shape=shape;
    Paintbrush_width=Brush_container[index].Width;
    Paintbrush_height=Brush_container[index].Height;
    if (shape == PAINTBRUSH_SHAPE_HORIZONTAL_BAR)
      Paintbrush_height=1;
    else if (shape == PAINTBRUSH_SHAPE_VERTICAL_BAR)
      Paintbrush_width=1;
    
    if (Paintbrush_width <= BRUSH_CONTAINER_PREVIEW_WIDTH &&
        Paintbrush_height <= BRUSH_CONTAINER_PREVIEW_HEIGHT)
    {
      // Manually copy the "pixels"
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=Brush_container[index].Thumbnail[y_pos][x_pos];
          
      Paintbrush_offset_X=Paintbrush_width>>1;
      Paintbrush_offset_Y=Paintbrush_height>>1;
    }
    else
    {
      // Recreate the brush pixels from its shape and dimensions
      Set_paintbrush_size(Paintbrush_width,Paintbrush_height);
    }
  }
  // Color brushes
  if (shape == PAINTBRUSH_SHAPE_COLOR_BRUSH ||
     shape == PAINTBRUSH_SHAPE_MONO_BRUSH)
  {
    Paintbrush_shape=shape;
    Realloc_brush(Brush_container[index].Width,Brush_container[index].Height);
    // Realloc sets Brush_width and Brush_height to new size.
    memcpy(Brush, Brush_container[index].Brush, Brush_height*Brush_width);
    
    Brush_offset_X=Brush_width>>1;
    Brush_offset_Y=Brush_height>>1;

  }
  Change_paintbrush_shape(shape);

  return 1;
}

void Button_Brush_container(void)
{
  short clicked_button;
  short x_pos,y_pos;
  byte index;

  Open_window(BRUSH_CONTAINER_COLUMNS*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+8,
    BRUSH_CONTAINER_ROWS*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+40,
    "Brushes");

  Window_set_normal_button(
    (BRUSH_CONTAINER_COLUMNS*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)-59)/2,
    (BRUSH_CONTAINER_ROWS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18,
    67,14,"Cancel",0,1,KEY_ESC); // 1

  index=0;
  for (index=0; index < BRUSH_CONTAINER_ROWS*BRUSH_CONTAINER_COLUMNS; index++)
  {
    x_pos = (index % BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+7;
    y_pos = (index / BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18;
    Window_set_normal_button(
      x_pos,
      y_pos,
      BRUSH_CONTAINER_PREVIEW_WIDTH+2,
      BRUSH_CONTAINER_PREVIEW_HEIGHT+2,
      "",0,1,SDLK_LAST);
    Display_stored_brush_in_window(x_pos+1, y_pos+1, index);
  }
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    //if (Is_shortcut(Key,0x100+BUTTON_HELP))
    //  Window_help(BUTTON_PAINTBRUSHES, NULL);
    
    if (clicked_button == 1)
      break;
      
    if (clicked_button>1)
    {
      index = clicked_button-2;
      
      if (Window_attribute1==RIGHT_SIDE)
      {
        // Store
        
        x_pos = (index % BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+7;
        y_pos = (index / BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18;
      
        Store_brush(index);
        Hide_cursor();
        Display_stored_brush_in_window(x_pos+1, y_pos+1, index);
        Display_cursor();
      }
      else
      {
        // Restore and exit
      
        if (Restore_brush(index))
          break;
      }
    }
  }
  while (1);
  Close_window();

  //Unselect_button(BUTTON_PAINTBRUSHES);
  Display_cursor();
}
