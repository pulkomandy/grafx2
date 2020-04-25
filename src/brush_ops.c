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
//////////////////////////////////////////////////////////////////////////////
///@file brush_ops.c
/// Code for operations about the brush (grabbing, rotating, ...) and magnifier
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>

#include "brush.h"
#include "buttons.h"
#include "engine.h"
#include "global.h"
#include "graph.h"
#include "misc.h"
#include "operatio.h"
#include "pages.h"
#include "screen.h"
#include "windows.h"
#include "keyboard.h"

#ifndef M_PI
    #define M_PI 3.141592653589793238462643
#endif

/// Simulates clicking the "Draw" button.
void Return_to_draw_mode(void)
{

  // Comme l'enclenchement du bouton efface le curseur, il faut l'afficher au
  // préalable:
  Display_cursor();
  if (Mouse_K)
    Wait_end_of_click();
  // !!! Efface la croix puis affiche le viseur !!!
  Select_button(BUTTON_DRAW,LEFT_SIDE); // Désenclenche au passage le bouton brosse
  if (Config.Auto_discontinuous)
  {
    // On se place en mode Dessin discontinu à la main
    while (Current_operation!=OPERATION_DISCONTINUOUS_DRAW)
      Select_button(BUTTON_DRAW,RIGHT_SIDE);
  }
  // Maintenant, il faut réeffacer le curseur parce qu'il sera raffiché en fin
  // d'appel à cette action:
  Hide_cursor();

  // On passe en brosse couleur:
  Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:       Y:",0);
  Print_coordinates();
}

// ---------------------------------------------------------- OPERATION_MAGNIFY


void Magnifier_12_0(void)

// Opération   : 4      (item d'une Loupe)
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui

{

  // On passe en mode loupe
  Main.magnifier_mode=1;

  // La fonction d'affichage dans la partie image est désormais un affichage
  // spécial loupe.
  Pixel_preview=Pixel_preview_magnifier;

  // On calcule l'origine de la loupe
  Main.magnifier_offset_X=Mouse_X-(Main.magnifier_width>>1);
  Main.magnifier_offset_Y=Mouse_Y-(Main.magnifier_height>>1);

  // Calcul des coordonnées absolues de ce coin DANS L'IMAGE
  Main.magnifier_offset_X+=Main.offset_X;
  Main.magnifier_offset_Y+=Main.offset_Y;

  Clip_magnifier_offsets(&Main.magnifier_offset_X, &Main.magnifier_offset_Y);

  // On calcule les bornes visibles dans l'écran
  Position_screen_according_to_zoom();
  Compute_limits();
  Display_all_screen();

  // Repositionner le curseur en fonction des coordonnées visibles
  Compute_paintbrush_coordinates();

  // On fait de notre mieux pour restaurer l'ancienne opération:
  Start_operation_stack(Operation_before_interrupt);
  Display_cursor();
  Wait_end_of_click();
}


/////////////////////////////////////////////////////////// OPERATION_COLORPICK


void Colorpicker_12_0(void)
//
// Opération   : OPERATION_COLORPICK
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Init_start_operation();

  if (Mouse_K==LEFT_SIDE)
  {
    Set_fore_color(Colorpicker_color);
  }
  else
  {
    Set_back_color(Colorpicker_color);
  }
  Operation_push(Mouse_K);
}


void Colorpicker_1_1(void)
//
// Opération   : OPERATION_COLORPICK
// Click Souris: 1
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  char str[4];

  if ( (Paintbrush_X>=0) && (Paintbrush_Y>=0)
    && (Paintbrush_X<Main.image_width)
    && (Paintbrush_Y<Main.image_height) )
    Colorpicker_color=Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y);
  else
    Colorpicker_color=0;

  if ( (Colorpicker_X!=Paintbrush_X)
    || (Colorpicker_Y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Colorpicker_X=Paintbrush_X;
    Colorpicker_Y=Paintbrush_Y;

    if (Colorpicker_color!=Fore_color)
    {
      Set_fore_color(Colorpicker_color);
    }

    if (Menu_is_visible)
    {
      Print_coordinates();
      Num2str(Colorpicker_color,str,3);
      Print_in_menu(str,20);
      Print_general(170*Menu_factor_X,Menu_status_Y," ",0,Colorpicker_color);
    }
    Display_cursor();
  }
}


void Colorpicker_2_1(void)
//
// Opération   : OPERATION_COLORPICK
// Click Souris: 2
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  char str[4];

  if ( (Paintbrush_X>=0) && (Paintbrush_Y>=0)
    && (Paintbrush_X<Main.image_width)
    && (Paintbrush_Y<Main.image_height) )
    Colorpicker_color=Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y);
  else
    Colorpicker_color=0;

  if ( (Colorpicker_X!=Paintbrush_X)
    || (Colorpicker_Y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Colorpicker_X=Paintbrush_X;
    Colorpicker_Y=Paintbrush_Y;

    if (Colorpicker_color!=Back_color)
    {
      Set_back_color(Colorpicker_color);
    }

    if (Menu_is_visible)
    {
      Print_coordinates();
      Num2str(Colorpicker_color,str,3);
      Print_in_menu(str,20);
      Print_general(170*Menu_factor_X,Menu_status_Y," ",0,Colorpicker_color);
    }
    Display_cursor();
  }
}



void Colorpicker_0_1(void)
//
// Opération   : OPERATION_COLORPICK
// Click Souris: 0
// Taille_Pile : 1
//
// Souris effacée: Oui
//
{
  short click;

  Operation_pop(&click);
  if (click==LEFT_SIDE)
  {
    Set_fore_color(Colorpicker_color);
  }
  else
  {
    Set_back_color(Colorpicker_color);
  }
  Unselect_button(BUTTON_COLORPICKER);
}

/////////////////////////////////////////////////////////// OPERATION_RMB_COLORPICK


byte Rightclick_colorpick(byte cursor_visible)
{
  // Check if the rightclick colorpick should take over:
  if (!Config.Right_click_colorpick)
    return 0;
  if (Mouse_K!=RIGHT_SIDE)
    return 0;
  // In these modes, the Foreground color is ignored,
  // so the RMB should act as normal.
  if (Shade_mode||Quick_shade_mode||Tiling_mode)
    return 0;

  Colorpicker_color=-1;
  Colorpicker_X=-1;
  Colorpicker_Y=-1;

  if (cursor_visible)
    Hide_cursor();
  Start_operation_stack(OPERATION_RMB_COLORPICK);

  Init_start_operation();

  // Just an indicator to go to next step
  Operation_push(1);
  Rightclick_colorpick_2_1();

  if (cursor_visible)
    Display_cursor();

  return 1;
}

void Rightclick_colorpick_2_1(void)
//
// Opération   : OPERATION_RMB_COLORPICK
// Click Souris: 2
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  char str[4];

  if ( (Colorpicker_X!=Paintbrush_X)
    || (Colorpicker_Y!=Paintbrush_Y) )
  {
    if ( (Paintbrush_X>=0) && (Paintbrush_Y>=0)
      && (Paintbrush_X<Main.image_width)
      && (Paintbrush_Y<Main.image_height) )
      Colorpicker_color=Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y);
    else
      Colorpicker_color=0;

    Colorpicker_X=Paintbrush_X;
    Colorpicker_Y=Paintbrush_Y;

    if (Menu_is_visible)
    {
      Print_coordinates();
      Num2str(Colorpicker_color,str,3);
      Print_in_menu(str,20);
      Print_general(170*Menu_factor_X,Menu_status_Y," ",0,Colorpicker_color);
    }
  }
}



void Rightclick_colorpick_0_1(void)
//
// Opération   : OPERATION_RMB_COLORPICK
// Click Souris: 0
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  short dummy;

  Hide_cursor();

  Operation_pop(&dummy);
  Set_fore_color(Colorpicker_color);

  // Restore previous operation
  Start_operation_stack(Operation_before_interrupt);

  // Erase the color block which shows the picked color
  if (Operation_before_interrupt!=OPERATION_REPLACE)
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
      Print_in_menu("X:       Y:             ",0);

  Print_coordinates();

  Display_cursor();
}

////////////////////////////////////////////////////// OPERATION_GRAB_BRUSH


void Brush_12_0(void)
//
// Opération   : OPERATION_GRAB_BRUSH
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Init_start_operation();
  if (Mouse_K==RIGHT_SIDE) // Besoin d'effacer la brosse après ?
  {
    Operation_push(1);
    // Puisque la zone où on prend la brosse sera effacée, on fait un backup
    Backup();
  }
  else
    Operation_push(0);

  // On laisse une trace du curseur pour que l'utilisateur puisse visualiser
  // où demarre sa brosse:
  Display_cursor();

  Operation_push(Paintbrush_X); // Début X
  Operation_push(Paintbrush_Y); // Début Y
  Operation_push(Paintbrush_X); // Dernière position X
  Operation_push(Paintbrush_Y); // Dernière position Y

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("\035:   1   \022:   1",0);
}


void Brush_12_5(void)
//
// Opération   : OPERATION_GRAB_BRUSH
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  str[5];
  short start_x;
  short start_y;
  short old_x;
  short old_y;
  short width;
  short height;

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ( (Menu_is_visible) && ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y)) )
  {
    if (Config.Coords_rel)
    {
      Operation_pop(&start_y);
      Operation_pop(&start_x);
      Operation_push(start_x);
      Operation_push(start_y);

      width=((start_x<Paintbrush_X)?Paintbrush_X-start_x:start_x-Paintbrush_X)+1;
      height=((start_y<Paintbrush_Y)?Paintbrush_Y-start_y:start_y-Paintbrush_Y)+1;

      Num2str(width,str,4);
      Print_in_menu(str,2);
      Num2str(height,str,4);
      Print_in_menu(str,11);
    }
    else
      Print_coordinates();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Brush_0_5(void)
//
// Opération   : OPERATION_GRAB_BRUSH
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
//
{
  short start_x;
  short start_y;
  short old_paintbrush_x;
  short old_paintbrush_y;
  short clear;

  // Comme on a demandé l'effacement du curseur, il n'y a plus de croix en
  // (Paintbrush_X,Paintbrush_Y). C'est une bonne chose.

  Operation_stack_size-=2;
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&clear);

  // On efface l'ancienne croix:
  old_paintbrush_x=Paintbrush_X;
  old_paintbrush_y=Paintbrush_Y;
  Paintbrush_X=start_x;
  Paintbrush_Y=start_y;
  Hide_cursor(); // Maintenant, il n'y a plus de croix à l'écran.

  Paintbrush_X=old_paintbrush_x;
  Paintbrush_Y=old_paintbrush_y;

  // Prise de la brosse
  if ((Snap_mode) && (Config.Adjust_brush_pick))
  {
    if (Paintbrush_X<start_x)
    {
      old_paintbrush_x=start_x;
      start_x=Paintbrush_X;
    }
    if (Paintbrush_Y<start_y)
    {
      old_paintbrush_y=start_y;
      start_y=Paintbrush_Y;
    }
    if (old_paintbrush_x!=start_x)
      old_paintbrush_x--;
    if (old_paintbrush_y!=start_y)
      old_paintbrush_y--;
  }
  Capture_brush(start_x,start_y,old_paintbrush_x,old_paintbrush_y,clear);
  if ((Snap_mode) && (Config.Adjust_brush_pick))
  {
    Brush_offset_X=(Brush_offset_X/Snap_width)*Snap_width;
    Brush_offset_Y=(Brush_offset_Y/Snap_height)*Snap_height;
  }

  End_of_modification();
  Return_to_draw_mode();
}


//////////////////////////////////////////////////////// OPERATION_POLYBRUSH


void Polybrush_12_8(void)
//  Opération   : OPERATION_POLYBRUSH
//  Click Souris: 1 ou 2
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short click;
  short end_y;
  short end_x;
  short start_y;
  short start_x;
  short color;
  short initial_y;
  short initial_x;

  Operation_pop(&click);
  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if (click==Mouse_K)
  {
    // L'utilisateur clique toujours avec le bon bouton de souris

    if (((start_x!=Paintbrush_X) || (start_y!=Paintbrush_Y)) &&
        (Polyfill_number_of_points<Config.Nb_max_vertices_per_polygon))
    {
      // Il existe un nouveau segment défini par
      // (start_x,start_y)-(Paintbrush_X,Paintbrush_Y)

      Hide_cursor();
      Print_coordinates();

      // On le place à l'écran
      Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
      Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);

      // On peut le rajouter au polygone

      Polyfill_table_of_points[Polyfill_number_of_points<<1]    =Paintbrush_X;
      Polyfill_table_of_points[(Polyfill_number_of_points<<1)+1]=Paintbrush_Y;
      Polyfill_number_of_points++;

      Operation_push(Paintbrush_X); // Nouveau start_x
      Operation_push(Paintbrush_Y); // Nouveau start_y
      Operation_push(Paintbrush_X); // Nouveau end_x
      Operation_push(Paintbrush_Y); // Nouveau end_y
      Operation_push(click);

      Display_cursor();
    }
    else
    {
      if (Polyfill_number_of_points==Config.Nb_max_vertices_per_polygon)
      {
        // Le curseur bouge alors qu'on ne peut plus stocker de segments ?

        if ((end_x!=Paintbrush_X) || (end_y!=Paintbrush_Y))
        {
          Hide_cursor();
          Print_coordinates();

          // On le place à l'écran
          Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
          Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);
          Display_cursor();
        }

        // On remet les mêmes valeurs (comme si on n'avait pas cliqué):
        Operation_push(start_x);
        Operation_push(start_y);
        Operation_push(Paintbrush_X);
        Operation_push(Paintbrush_Y);
        Operation_push(click);
      }
      else
      {
        Operation_push(Paintbrush_X); // Nouveau start_x
        Operation_push(Paintbrush_Y); // Nouveau start_y
        Operation_push(Paintbrush_X); // Nouveau end_x
        Operation_push(Paintbrush_Y); // Nouveau end_y
        Operation_push(click);
      }
    }
  }
  else
  {
    // L'utilisateur souhaite arrêter l'opération et refermer le polygone

    Operation_pop(&color);
    Operation_pop(&initial_y);
    Operation_pop(&initial_x);

    Hide_cursor();
    Print_coordinates();

    // Pas besoin d'effacer la ligne (start_x,start_y)-(end_x,end_y)
    // puisqu'on les efface toutes d'un coup.

    Capture_brush_with_lasso(Polyfill_number_of_points,Polyfill_table_of_points,click==RIGHT_SIDE);
    free(Polyfill_table_of_points);
    Polyfill_table_of_points = NULL;

    if (click==RIGHT_SIDE)
      End_of_modification();

    // On raffiche l'écran pour effacer les traits en xor et pour raffraichir
    // l'écran si on a découpé une partie de l'image en prenant la brosse.
    Display_all_screen();

    Paintbrush_hidden=0;

    if ((Snap_mode) && (Config.Adjust_brush_pick))
    {
      Brush_offset_X=(Brush_offset_X/Snap_width)*Snap_width;
      Brush_offset_Y=(Brush_offset_Y/Snap_height)*Snap_height;
    }

    Return_to_draw_mode();
    Display_cursor();
  }
}


///////////////////////////////////////////////////// OPERATION_STRETCH_BRUSH


void Stretch_brush_12_0(void)
//
// Opération   : OPERATION_STRETCH_BRUSH
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Init_start_operation();
  if (Mouse_K==LEFT_SIDE)
  {
    // On laisse une trace du curseur pour que l'utilisateur puisse visualiser
    // où demarre sa brosse:
    Display_cursor();

    Operation_push(Paintbrush_X); // Dernier calcul X
    Operation_push(Paintbrush_Y); // Dernier calcul Y
    Operation_push(Paintbrush_X); // Début X
    Operation_push(Paintbrush_Y); // Début Y
    Operation_push(Paintbrush_X); // Dernière position X
    Operation_push(Paintbrush_Y); // Dernière position Y
    Operation_push(1); // State précédent

    if ((Config.Coords_rel) && (Menu_is_visible))
      Print_in_menu("\035:   1   \022:   1",0);
  }
  else
  {
    Wait_end_of_click();
    Start_operation_stack(Operation_before_interrupt);
  }
}



void Stretch_brush_1_7(void)
//
// Opération   : OPERATION_STRETCH_BRUSH
// Click Souris: 1
// Taille_Pile : 7
//
// Souris effacée: Non
//
{
  char  str[5];
  short start_x;
  short start_y;
  short old_x;
  short old_y;
  short width;
  short height;
  short prev_state;
  short dx,dy,x,y;

  Operation_pop(&prev_state);
  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ( (Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y) || (prev_state!=2) )
  {
    Hide_cursor();
    Operation_pop(&start_y);
    Operation_pop(&start_x);

    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
      {
        width=((start_x<Paintbrush_X)?Paintbrush_X-start_x:start_x-Paintbrush_X)+1;
        height=((start_y<Paintbrush_Y)?Paintbrush_Y-start_y:start_y-Paintbrush_Y)+1;

        if (Snap_mode && Config.Adjust_brush_pick)
        {
          if (width>1) width--;
          if (height>1) height--;
        }

        Num2str(width,str,4);
        Print_in_menu(str,2);
        Num2str(height,str,4);
        Print_in_menu(str,11);
      }
      else
        Print_coordinates();
    }

    Display_all_screen();

    x=Paintbrush_X;
    y=Paintbrush_Y;
    if (Snap_mode && Config.Adjust_brush_pick)
    {
      dx=Paintbrush_X-start_x;
      dy=Paintbrush_Y-start_y;
      if (dx<0) x++; else {if (dx>0) x--;}
      if (dy<0) y++; else {if (dy>0) y--;}
      Stretch_brush_preview(start_x,start_y,x,y);
    }
    else
      Stretch_brush_preview(start_x,start_y,Paintbrush_X,Paintbrush_Y);

    old_x=Paintbrush_X;
    old_y=Paintbrush_Y;
    Paintbrush_X=start_x;
    Paintbrush_Y=start_y;
    Display_cursor();
    Paintbrush_X=old_x;
    Paintbrush_Y=old_y;
    Display_cursor();

    Operation_stack_size-=2;
    Operation_push(x);
    Operation_push(y);

    Operation_push(start_x);
    Operation_push(start_y);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(2);
}



void Stretch_brush_0_7(void)
//
// Opération   : OPERATION_STRETCH_BRUSH
// Click Souris: 0
// Taille_Pile : 7
//
// Souris effacée: Non
//
{
  char  str[5];
  short start_x;
  short start_y;
  short old_x;
  short old_y;
  short width=0;
  short height=0;
  byte  size_change;
  short prev_state;

  Operation_pop(&prev_state);
  Operation_pop(&old_y);
  Operation_pop(&old_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y) || (prev_state!=3))
  {
    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
      {
        width=((start_x<Paintbrush_X)?Paintbrush_X-start_x:start_x-Paintbrush_X)+1;
        height=((start_y<Paintbrush_Y)?Paintbrush_Y-start_y:start_y-Paintbrush_Y)+1;

        Num2str(width,str,4);
        Print_in_menu(str,2);
        Num2str(height,str,4);
        Print_in_menu(str,11);
      }
      else
        Print_coordinates();
    }
  }

  // Utilise Key_ANSI au lieu de Key, car Get_input() met ce dernier
  // à zero si une operation est en cours (Operation_stack_size!=0)
  if (Key_ANSI)
  {
    size_change=1;
    switch (Key_ANSI)
    {
      case 'd': // Double
        width=start_x+(Brush_width<<1)-1;
        height=start_y+(Brush_height<<1)-1;
        break;
      case 'x': // Double X
        width=start_x+(Brush_width<<1)-1;
        height=start_y+Brush_height-1;
        break;
      case 'y': // Double Y
        width=start_x+Brush_width-1;
        height=start_y+(Brush_height<<1)-1;
        break;
      case 'h': // Moitié
        width=(Brush_width>1)?start_x+(Brush_width>>1)-1:1;
        height=(Brush_height>1)?start_y+(Brush_height>>1)-1:1;
        break;
      case 'X': // Moitié X
        width=(Brush_width>1)?start_x+(Brush_width>>1)-1:1;
        height=start_y+Brush_height-1;
        break;
      case 'Y': // Moitié Y
        width=start_x+Brush_width-1;
        height=(Brush_height>1)?start_y+(Brush_height>>1)-1:1;
        break;
      case 'n': // Normal
        width=start_x+Brush_width-1;
        height=start_y+Brush_height-1;
        break;
      default :
        size_change=0;
    }
    Key_ANSI=0;
  }
  else
    size_change=0;

  if (size_change)
  {
    // On efface la preview de la brosse (et la croix)
    Display_all_screen();

    old_x=Paintbrush_X;
    old_y=Paintbrush_Y;
    Paintbrush_X=start_x;
    Paintbrush_Y=start_y;
    Display_cursor();
    Paintbrush_X=old_x;
    Paintbrush_Y=old_y;

    Stretch_brush_preview(start_x,start_y,width,height);
    Display_cursor();

    Operation_stack_size-=2;
    Operation_push(width);
    Operation_push(height);
  }

  Operation_push(start_x);
  Operation_push(start_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(3);
}


void Stretch_brush_2_7(void)
//
// Opération   : OPERATION_STRETCH_BRUSH
// Click Souris: 2
// Taille_Pile : 7
//
// Souris effacée: Oui
//
{
  short computed_x;
  short computed_y;
  short start_x;
  short start_y;


  Operation_stack_size-=3;
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&computed_y);
  Operation_pop(&computed_x);

  // On efface la preview de la brosse (et la croix)
  Display_all_screen();

  // Et enfin on stocke pour de bon la nouvelle brosse étirée
  Stretch_brush(start_x,start_y,computed_x,computed_y);

  Return_to_draw_mode();
}


//////////////////////////////////////////////////// OPERATION_ROTATE_BRUSH


void Rotate_brush_12_0(void)
//
// Opération   : OPERATION_ROTATE_BRUSH
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Init_start_operation();
  if (Mouse_K==LEFT_SIDE)
  {
    Brush_rotation_center_X=Paintbrush_X+(Brush_width>>1)-Brush_width;
    Brush_rotation_center_Y=Paintbrush_Y;
    Brush_rotation_center_is_defined=1;
    Operation_push(Paintbrush_X); // Dernière position calculée X
    Operation_push(Paintbrush_Y); // Dernière position calculée Y
    Operation_push(Paintbrush_X); // Dernière position X
    Operation_push(Paintbrush_Y); // Dernière position Y
    Operation_push(1); // State précédent

    if ((Config.Coords_rel) && (Menu_is_visible))
      Print_in_menu("Angle:   0\xb0    ",0);
  }
  else
  {
    Start_operation_stack(Operation_before_interrupt);
    Wait_end_of_click(); // FIXME: celui-la il donne un résultat pas très chouette en visuel
  }
}



void Rotate_brush_1_5(void)
//
// Opération   : OPERATION_ROTATE_BRUSH
// Click Souris: 1
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  str[8];
  short old_x;
  short old_y;
  short prev_state;
  double angle;
  short cursor_x,cursor_y;

  Operation_pop(&prev_state);
  Operation_pop(&old_y);
  Operation_pop(&old_x);

  // On corrige les coordonnées de la ligne si la touche shift est appuyée...
  cursor_x = Paintbrush_X;
  cursor_y = Paintbrush_Y;
  if(Get_Key_modifiers() & GFX2_MOD_SHIFT)
    Clamp_coordinates_regular_angle(Brush_rotation_center_X,Brush_rotation_center_Y,&cursor_x,&cursor_y);

  if ( (cursor_x!=old_x) || (cursor_y!=old_y) || (prev_state!=2) )
  {
    if ( (Brush_rotation_center_X==cursor_x)
      && (Brush_rotation_center_Y==cursor_y) )
      angle=0.0;
    else
    {
      // Angle from 0 to 2 PI
      int dx = cursor_x - Brush_rotation_center_X;
      int dy = -(cursor_y - Brush_rotation_center_Y);
      angle = atan2((double)dy, (double)dx);
    }

    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
      {
        snprintf(str, sizeof(str), "%4d", (int)(angle*180.0/M_PI));
        Print_in_menu(str, 6);
      }
      else
        Print_coordinates();
    }

    Display_all_screen();
    Rotate_brush_preview(angle);
    Display_cursor();

    Operation_stack_size-=2;
    Operation_push(cursor_x);
    Operation_push(cursor_y);
  }


  Operation_push(cursor_x);
  Operation_push(cursor_y);
  Operation_push(2);
}



void Rotate_brush_0_5(void)
//
// Opération   : OPERATION_ROTATE_BRUSH
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  str[8];
  short old_x;
  short old_y;
  short computed_x=0;
  short computed_y=0;
  byte  angle_change;
  short prev_state;
  double angle=0.0;
  short cursor_x, cursor_y;


  Operation_pop(&prev_state);
  Operation_pop(&old_y);
  Operation_pop(&old_x);

  // On corrige les coordonnées de la ligne si la touche shift est appuyée...
  cursor_x = Paintbrush_X;
  cursor_y = Paintbrush_Y;
  if(Get_Key_modifiers() & GFX2_MOD_SHIFT)
    Clamp_coordinates_regular_angle(Brush_rotation_center_X,Brush_rotation_center_Y,&cursor_x,&cursor_y);

  if ((cursor_x!=old_x) || (cursor_y!=old_y) || (prev_state!=3))
  {
    Hide_cursor();
    if ( (Brush_rotation_center_X==cursor_x)
      && (Brush_rotation_center_Y==cursor_y) )
      angle=0.0;
    else
    {
      // Angle from 0 to 2 PI
      int dx = cursor_x - Brush_rotation_center_X;
      int dy = -(cursor_y - Brush_rotation_center_Y);
      angle = atan2((double)dy, (double)dx);
    }

    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
      {
        snprintf(str, sizeof(str), "%4d", (int)(angle*180.0/M_PI));
        Print_in_menu(str, 6);
      }
      else
        Print_coordinates();
    }
    Display_cursor();
  }

  // Utilise Key_ANSI au lieu de Key, car Get_input() met ce dernier
  // à zero si une operation est en cours (Operation_stack_size!=0)
  if (Key_ANSI)
  {
    angle_change=1;
    computed_x=Brush_rotation_center_X;
    computed_y=Brush_rotation_center_Y;
    switch (Key_ANSI)
    {
      case '6': angle =       0.0; computed_x++;               break;
      case '9': angle = M_PI*0.25; computed_x++; computed_y--; break;
      case '8': angle = M_PI*0.5 ;               computed_y--; break;
      case '7': angle = M_PI*0.75; computed_x--; computed_y--; break;
      case '4': angle = M_PI     ; computed_x--;               break;
      case '1': angle =-M_PI*0.75; computed_x--; computed_y++; break;
      case '2': angle =-M_PI*0.5 ;               computed_y++; break;
      case '3': angle =-M_PI*0.25; computed_x++; computed_y++; break;
      default :
        angle_change=0;
    }
    Key_ANSI=0;
  }
  else
    angle_change=0;

  if (angle_change)
  {
    // On efface la preview de la brosse
    Display_all_screen();
    Rotate_brush_preview(angle);
    Display_cursor();

    Operation_stack_size-=2;
    Operation_push(computed_x);
    Operation_push(computed_y);
  }

  Operation_push(cursor_x);
  Operation_push(cursor_y);
  Operation_push(3);
}


void Rotate_brush_2_5(void)
//
// Opération   : OPERATION_ROTATE_BRUSH
// Click Souris: 2
// Taille_Pile : 5
//
// Souris effacée: Oui
//
{
  short computed_x;
  short computed_y;
  int dx,dy;
  float angle;


  // On efface la preview de la brosse
  Display_all_screen();

  Operation_stack_size-=3;
  Operation_pop(&computed_y);
  Operation_pop(&computed_x);

  // Calcul de l'angle par rapport à la dernière position calculée
  if ( (Brush_rotation_center_X==computed_x)
    && (Brush_rotation_center_Y==computed_y) )
    angle=0.0;
  else
  {
    dx=computed_x-Brush_rotation_center_X;
    dy=computed_y-Brush_rotation_center_Y;
    angle=acos(((float)dx)/sqrt((dx*dx)+(dy*dy)));
    if (dy>0) angle=M_2PI-angle;
  }

  // Et enfin on stocke pour de bon la nouvelle brosse étirée
  Rotate_brush(angle);

  Return_to_draw_mode();
}

///////////////////////////////////////////////////// OPERATION_DISTORT_BRUSH

/// Draws a 2x2 XOR square at the specified picture coordinates, on the screen.
void Draw_stretch_spot(short x_pos, short y_pos)
{
  short x,y;

  for (y=y_pos-1;y<y_pos+1;y++)
    if (y>=Limit_top && y<=Limit_visible_bottom)
      for (x=x_pos-1;x<x_pos+1;x++)
        if (x>=Limit_left && x<=Limit_visible_right)
          Pixel_preview(x,y,xor_lut[Read_pixel(x-Main.offset_X,y-Main.offset_Y)]);
  Update_part_of_screen(x_pos-1, y_pos-1, 2, 2);
}

void Distort_brush_0_0(void)
//
// Opération   : OPERATION_DISTORT_BRUSH
// Click Souris: 0
// Taille_Pile : 0
//
// Souris effacée: Non
//
{
  if ( Menu_is_visible )
  {
      Print_in_menu("POSITION BRUSH TO START ",0);
  }
}

void Distort_brush_1_0(void)
//
// Opération   : OPERATION_DISTORT_BRUSH
// Click Souris: 1
// Taille_Pile : 0
//
// Souris effacée: Non
//
{
  short x_pos, y_pos;

  Init_start_operation();
  Paintbrush_hidden=1;
  Hide_cursor();

  // Top left angle
  x_pos=Paintbrush_X-Brush_offset_X;
  y_pos=Paintbrush_Y-Brush_offset_Y;
  Draw_stretch_spot(x_pos,y_pos);
  Operation_push(x_pos);
  Operation_push(y_pos);

  // Top right angle
  x_pos+=Brush_width;
  Draw_stretch_spot(x_pos,y_pos);
  Operation_push(x_pos);
  Operation_push(y_pos);

  // Bottom right angle
  y_pos+=Brush_height;
  Draw_stretch_spot(x_pos,y_pos);
  Operation_push(x_pos);
  Operation_push(y_pos);

  // Bottom left angle
  x_pos-=Brush_width;
  Draw_stretch_spot(x_pos,y_pos);
  Operation_push(x_pos);
  Operation_push(y_pos);

  Distort_brush_preview(
    Operation_stack[1],
    Operation_stack[2],
    Operation_stack[3],
    Operation_stack[4],
    Operation_stack[5],
    Operation_stack[6],
    Operation_stack[7],
    Operation_stack[8]);
  Display_cursor();
  Update_part_of_screen(Paintbrush_X-Brush_offset_X, Paintbrush_Y-Brush_offset_Y, Brush_width, Brush_height);
  Wait_end_of_click();
  // Erase the message in status bar
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
      Print_in_menu("X:       Y:             ",0);
  }
}

void Distort_brush_1_8(void)
//
//  Opération   : OPERATION_DISTORT_BRUSH
//  Click Souris: 1
//  Taille_Pile : 8
//
//  Souris effacée: No
//
{
  // How far (in pixels) you can catch a handle
  #define REACH_DISTANCE 100
  short i;
  short x[4];
  short y[4];
  long best_distance=REACH_DISTANCE;
  short best_spot=-1;

  for (i=3;i>=0;i--)
  {
    long distance;
    Operation_pop(&y[i]);
    Operation_pop(&x[i]);
    distance=Distance(Paintbrush_X,Paintbrush_Y,x[i],y[i]);
    if (distance<best_distance)
    {
      best_spot=i;
      best_distance=distance;
    }
  }

  for (i=0;i<4;i++)
  {
    Operation_push(x[i]);
    Operation_push(y[i]);
  }

  if (best_spot>-1)
  {
    Operation_push(best_spot);
  }
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
  }
}

void Distort_brush_1_9(void)
//
//  Opération   : OPERATION_DISTORT_BRUSH
//  Click Souris: 1
//  Taille_Pile : 9
//
//  Souris effacée: No
//
{
  short i;
  short x[4];
  short y[4];
  short selected_corner;

  // Pop all arguments
  Operation_pop(&selected_corner);
  for (i=3;i>=0;i--)
  {
    Operation_pop(&y[i]);
    Operation_pop(&x[i]);
  }

  if (Paintbrush_X!=x[selected_corner] || Paintbrush_Y!=y[selected_corner])
  {
    Hide_cursor();

    // Easiest refresh mode: make no assumptions on how the brush was
    // displayed before.
    Display_all_screen();

    x[selected_corner]=Paintbrush_X;
    y[selected_corner]=Paintbrush_Y;

    for (i=0;i<4;i++)
      Draw_stretch_spot(x[i],y[i]);

    Distort_brush_preview(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]);

    Display_cursor();

    if ( (Config.Coords_rel) && (Menu_is_visible) )
    {
        Print_in_menu("X:       Y:             ",0);
        Print_coordinates();
    }
    Update_rect(0,0,Screen_width,Menu_Y);
  }

  // Push back all arguments
  for (i=0;i<4;i++)
  {
    Operation_push(x[i]);
    Operation_push(y[i]);
  }
  Operation_push(selected_corner);

}
void Distort_brush_0_9(void)
//
//  Opération   : OPERATION_DISTORT_BRUSH
//  Click Souris: 0
//  Taille_Pile : 9
//
//  Souris effacée: No
//
{
  short selected_corner;
  Operation_pop(&selected_corner);

}

void Distort_brush_2_0(void)
//
// Opération   : OPERATION_DISTORT_BRUSH
// Click Souris: 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Paintbrush_hidden=0;
  Display_all_screen();
  // Erase the message in status bar
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
      Print_in_menu("X:       Y:             ",0);
  }
  Return_to_draw_mode();
}

void Distort_brush_2_8(void)
//
// Opération   : OPERATION_DISTORT_BRUSH
// Click Souris: 2
// Taille_Pile : 8
//
// Souris effacée: Oui
//
{
  short i;
  short x[4];
  short y[4];

  // Pop all arguments
  for (i=3;i>=0;i--)
  {
    Operation_pop(&y[i]);
    Operation_pop(&x[i]);
  }
  Distort_brush(x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]);

  Paintbrush_hidden=0;
  Display_all_screen();

  Return_to_draw_mode();
}

