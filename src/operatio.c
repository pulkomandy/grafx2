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
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "misc.h"
#include "osdep.h"
#include "engine.h"
#include "graph.h"
#include "operatio.h"
#include "buttons.h"
#include "pages.h"
#include "errors.h"
#include "screen.h"
#include "brush.h"
#include "windows.h"
#include "input.h"
#include "special.h"
#include "tiles.h"
#include "keyboard.h"

// PI is NOT part of math.h according to C standards...
#if defined(__GP2X__) || defined(__VBCC__)
    #define M_PI 3.14159265358979323846
#endif

/// Time (in SDL ticks) when the next airbrush drawing should be done. Also used
/// for discontinuous freehand drawing.
dword Airbrush_next_time;

// Panning needs its own operation storage because it may interrupt another
// interrupting operation
static enum OPERATIONS Operation_before_pan;

void Start_operation_stack(word new_operation)
{
  // This part handles things that must be done when exiting an operation.
  Brush_rotation_center_is_defined=0;
  switch(Current_operation)
  {
    case OPERATION_ROTATE_BRUSH:
      End_brush_rotation();
    break;

    default:
    break;
  }

  // On mémorise l'opération précédente si on démarre une interruption
  switch(new_operation)
  {
    case OPERATION_MAGNIFY:
    case OPERATION_COLORPICK:
    case OPERATION_RMB_COLORPICK:
    case OPERATION_GRAB_BRUSH:
    case OPERATION_POLYBRUSH:
    case OPERATION_STRETCH_BRUSH:
      Operation_before_interrupt=Current_operation;
      // On passe à l'operation demandée
      Current_operation=new_operation;
      break;
    case OPERATION_PAN_VIEW:
      // Use separate operation-before storage for panning
      Operation_before_pan=Current_operation;
      // On passe à l'operation demandée
      Current_operation=new_operation;
      break;
    case OPERATION_ROTATE_BRUSH:
      Begin_brush_rotation();
      Operation_before_interrupt=Current_operation;
      // On passe à l'operation demandée
      Current_operation=new_operation;
      break;
    default :
      // On passe à l'operation demandée
      Current_operation=new_operation;
      Operation_before_interrupt=Current_operation;
  }

  // On spécifie si l'opération autorise le changement de couleur au clavier
  switch(new_operation)
  {
    case OPERATION_CONTINUOUS_DRAW:
    case OPERATION_DISCONTINUOUS_DRAW:
    case OPERATION_AIRBRUSH:
    case OPERATION_CENTERED_LINES:
      Allow_color_change_during_operation=1;
      break;
    default :
      Allow_color_change_during_operation=0;
  }

  // Et on passe au curseur qui va avec
  Cursor_shape=CURSOR_FOR_OPERATION[new_operation];
  Operation_stack_size=0;
}


void Init_start_operation(void)
{
  Operation_in_magnifier=(Mouse_X>=Main.X_zoom);
  Smear_start=1;
}


void Operation_push(short value)
{
  Operation_stack[++Operation_stack_size]=value;
}


void Operation_pop(short * value)
{
  *value=Operation_stack[Operation_stack_size--];
}


byte Paintbrush_shape_before_operation;
byte Paintbrush_hidden_before_scroll;



short Distance(short x1, short y1, short x2, short y2)
{
  short x2_moins_x1=x2-x1;
  short y2_minus_y1=y2-y1;

  return Round( sqrt( (x2_moins_x1*x2_moins_x1) + (y2_minus_y1*y2_minus_y1) ) );
}


void Display_coords_rel_or_abs(short start_x, short start_y)
{
  char str[6];

  if (Config.Coords_rel)
  {
    if (Menu_is_visible)
    {
      if (Paintbrush_X>start_x)
      {
        Num2str(Paintbrush_X-start_x,str,5);
        str[0]='+';
      }
      else if (Paintbrush_X<start_x)
      {
        Num2str(start_x-Paintbrush_X,str,5);
        str[0]='-';
      }
      else
        strcpy(str,"\xb1   0");
      Print_in_menu(str,2);

      if (Paintbrush_Y>start_y)
      {
        Num2str(Paintbrush_Y-start_y,str,5);
        str[0]='+';
      }
      else if (Paintbrush_Y<start_y)
      {
        Num2str(start_y-Paintbrush_Y,str,5);
        str[0]='-';
      }
      else
        strcpy(str,"\xb1   0");
      Print_in_menu(str,12);
    }
  }
  else
    Print_coordinates();
}

//////////////////////////////////////////////////// OPERATION_CONTINUOUS_DRAW

void Freehand_mode1_1_0(void)
//  Opération   : OPERATION_CONTINUOUS_DRAW
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Init_start_operation();
  Backup();
  Shade_table=Shade_table_left;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Freehand_mode1_1_2(void)
//  Opération   : OPERATION_CONTINUOUS_DRAW
//  Click Souris: 1
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if ( (start_y!=Paintbrush_Y) || (start_x!=Paintbrush_X) )
  {
    Hide_cursor();
    Print_coordinates();
    Draw_line_permanent(start_x,start_y,Paintbrush_X,Paintbrush_Y,Fore_color);
    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Freehand_mode12_0_2(void)
//  Opération   : OPERATION_DESSIN_[DIS]CONTINU
//  Click Souris: 0
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  Operation_stack_size=0;
  End_of_modification();
}


void Freehand_mode1_2_0(void)
//  Opération   : OPERATION_CONTINUOUS_DRAW
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=Shade_table_right;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Back_color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Freehand_mode1_2_2(void)
//  Opération   : OPERATION_CONTINUOUS_DRAW
//  Click Souris: 2
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if ( (start_y!=Paintbrush_Y) || (start_x!=Paintbrush_X) )
  {
    Print_coordinates();
    Hide_cursor();
    Draw_line_permanent(start_x,start_y,Paintbrush_X,Paintbrush_Y,Back_color);
    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


///////////////////////////////////////////////// OPERATION_DISCONTINUOUS_DRAW

void Freehand_mode2_1_0(void)
//  Opération   : OPERATION_DISCONTINUOUS_DRAW
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Init_start_operation();
  Backup();
  Shade_table=Shade_table_left;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Print_coordinates();
  Airbrush_next_time = GFX2_GetTicks() + Airbrush_delay*10;
}


void Freehand_mode2_1_2(void)
//  Opération   : OPERATION_DISCONTINUOUS_DRAW
//  Click Souris: 1
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if ( (start_x!=Paintbrush_X) || (start_y!=Paintbrush_Y) )
  {
    Print_coordinates();
    if (GFX2_GetTicks()>Airbrush_next_time)
    {
      Airbrush_next_time+=Airbrush_delay*10;
      Hide_cursor();
      // On affiche définitivement le pinceau
      Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color);
      Display_cursor();
    }
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

// ----------

void Freehand_mode2_2_0(void)
//  Opération   : OPERATION_DISCONTINUOUS_DRAW
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=Shade_table_right;
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Print_coordinates();
  Airbrush_next_time = GFX2_GetTicks() + Airbrush_delay*10;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Back_color);
}


void Freehand_mode2_2_2(void)
//  Opération   : OPERATION_DISCONTINUOUS_DRAW
//  Click Souris: 2
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if ( (start_x!=Paintbrush_X) || (start_y!=Paintbrush_Y) )
  {
    Print_coordinates();
    if (GFX2_GetTicks()>Airbrush_next_time)
    {
      Airbrush_next_time+=Airbrush_delay*10;
      Hide_cursor();
      // On affiche définitivement le pinceau
      Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Back_color);
      Display_cursor();
    }
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


////////////////////////////////////////////////////// OPERATION_POINT_DRAW

void Freehand_mode3_1_0(void)
//  Opération   : OPERATION_POINT_DRAW
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Init_start_operation();
  Backup();
  Shade_table=Shade_table_left;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Fore_color);
  Operation_push(0);  // On change simplement l'état de la pile...
}


void Freehand_Mode3_2_0(void)
//  Opération   : OPERATION_POINT_DRAW
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=Shade_table_right;
  // On affiche définitivement le pinceau
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,Back_color);
  Operation_push(0);  // On change simplement l'état de la pile...
}


void Freehand_mode3_0_1(void)
//  Opération   : OPERATION_POINT_DRAW
//  Click Souris: 0
//  Taille_Pile : 1
//
//  Souris effacée: Non
{
  End_of_modification();
  Operation_stack_size--;
}


///////////////////////////////////////////////////////////// OPERATION_LINE

void Line_12_0(void)
// Opération   : OPERATION_LINE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Oui

//  Début du tracé d'une ligne (premier clic)
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Paintbrush_shape_before_operation=Paintbrush_shape;
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  if (Mouse_K==LEFT_SIDE)
  {
    Shade_table=Shade_table_left;
    Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,Fore_color);
    Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);
    Operation_push(Fore_color);
  }
  else
  {
    Shade_table=Shade_table_right;
    Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,Back_color);
    Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);
    Operation_push(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Line_12_5(void)
// Opération   : OPERATION_LINE
// Click Souris: 1
// Taille_Pile : 5
//
// Souris effacée: Non

// Poursuite du tracé d'une ligne (déplacement de la souris en gardant le
// curseur appuyé)
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;

  short cursor_x;
  short cursor_y;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);

  cursor_x = Paintbrush_X;
  cursor_y = Paintbrush_Y;

  // On corrige les coordonnées de la ligne si la touche shift est appuyée...
  if(Get_Key_modifiers() & GFX2_MOD_SHIFT)
    Clamp_coordinates_regular_angle(start_x,start_y,&cursor_x,&cursor_y);

  // On vient de bouger
  if ((cursor_x!=end_x) || (cursor_y!=end_y))
  {
      Hide_cursor();

    Display_coords_rel_or_abs(start_x,start_y);

    Hide_line_preview(start_x,start_y,end_x,end_y);
    if (Mouse_K==LEFT_SIDE)
    {
      Pixel_figure_preview (start_x,start_y,Fore_color);
      Draw_line_preview (start_x,start_y,cursor_x,cursor_y,Fore_color);
    }
    else
    {
      Pixel_figure_preview (start_x,start_y,Back_color);
      Draw_line_preview (start_x,start_y,cursor_x,cursor_y,Back_color);
    }

    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(cursor_x);
    Operation_push(cursor_y);

    Display_cursor();
  }
  else
  {
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(end_x);
    Operation_push(end_y);
  }
}


void Line_0_5(void)
// Opération   : OPERATION_LINE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui

// End du tracé d'une ligne (relachage du bouton)
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);

  Paintbrush_shape=Paintbrush_shape_before_operation;

  Pixel_figure_preview_auto  (start_x,start_y);
  Hide_line_preview (start_x,start_y,end_x,end_y);
  Draw_paintbrush            (start_x,start_y,color);
  Draw_line_permanent(start_x,start_y,end_x,end_y,color);

  End_of_modification();
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
}


/////////////////////////////////////////////////////////// OPERATION_K_LINE


void K_line_12_0(void)
// Opération   : OPERATION_K_LINE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Oui
{
  byte color;

  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  Paintbrush_shape_before_operation=Paintbrush_shape;
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Mouse_K | 0x80);
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  // Taille de pile 6 : phase d'appui, non interruptible
}


void K_line_12_6(void)
// Opération   : OPERATION_K_LINE
// Click Souris: 1 ou 2 | 0
// Taille_Pile : 6      | 7
//
// Souris effacée: Non
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;

  Operation_pop(&end_y);
  Operation_pop(&end_x);

  if ((Paintbrush_X!=end_x) || (Paintbrush_Y!=end_y))
  {
    Hide_cursor();
    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Operation_pop(&color);

    Display_coords_rel_or_abs(start_x,start_y);

    Hide_line_preview(start_x,start_y,end_x,end_y);
    Pixel_figure_preview (start_x,start_y,color);
    Draw_line_preview (start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

    Operation_push(color);
    Operation_push(start_x);
    Operation_push(start_y);
    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void K_line_0_6(void)
// Opération   : OPERATION_K_LINE
// Click Souris: 0
// Taille_Pile : 6
//
// Souris effacée: Oui
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;
  short direction;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);
  Operation_pop(&direction);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Pixel_figure_preview_auto  (start_x,start_y);
  Hide_line_preview (start_x,start_y,end_x,end_y);
  /* Doesn't work if fast moving
  Pixel_figure_preview_xor (start_x,start_y, 0);
  Draw_line_preview_xor (start_x,start_y,end_x,end_y,0);
  */
  Paintbrush_shape=Paintbrush_shape_before_operation;
  if (direction & 0x80)
  {
    Draw_paintbrush(start_x,start_y,color);
    direction=(direction & 0x7F);
  }
  Draw_line_permanent(start_x,start_y,Paintbrush_X,Paintbrush_Y,color);
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  Operation_push(direction);
  Operation_push(direction); // Valeur bidon servant de nouvel état de pile
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  // Taille de pile 7 : phase de "repos", interruptible (comme Elliot Ness :))
}


void K_line_12_7(void)
// Opération   : OPERATION_K_LINE
// Click Souris: 1 ou 2
// Taille_Pile : 7
//
// Souris effacée: Oui
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;
  short direction;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);
  Operation_pop(&direction);
  Operation_pop(&direction);

  if (direction==Mouse_K)
  {
    Operation_push(direction);
    Operation_push(color);
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(end_x);
    Operation_push(end_y);
    // Taille de pile 6 : phase d'appui, non interruptible
  }
  else
  {
    // La série de ligne est terminée, il faut donc effacer la dernière
    // preview de ligne
    Pixel_figure_preview_auto  (start_x,start_y);
    Hide_line_preview (start_x,start_y,end_x,end_y);

    Display_cursor();
    Wait_end_of_click();
    Hide_cursor();
    Paintbrush_shape=Paintbrush_shape_before_operation;

    End_of_modification();
    if ( (Config.Coords_rel) && (Menu_is_visible) )
    {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
    }
  }
}

/////////////////////////////////////////////////// OPERATION_RECTANGLE_?????

void Rectangle_12_0(void)
// Opération   : OPERATION_EMPTY_RECTANGLE / OPERATION_FILLED_RECTANGLE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("\035:   1   \022:   1",0);
  // On laisse une trace du curseur à l'écran
  Display_cursor();

  if (Mouse_K==LEFT_SIDE)
  {
    Shade_table=Shade_table_left;
    Operation_push(Fore_color);
  }
  else
  {
    Shade_table=Shade_table_right;
    Operation_push(Back_color);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Rectangle_12_5(void)
// Opération   : OPERATION_EMPTY_RECTANGLE / OPERATION_FILLED_RECTANGLE
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non
{
  short start_x;
  short start_y;
  short old_x;
  short old_y;
  char  str[5];

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y))
  {
    Operation_pop(&start_y);
    Operation_pop(&start_x);

    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Num2str(((start_x<Paintbrush_X)?Paintbrush_X-start_x:start_x-Paintbrush_X)+1,str,4);
      Print_in_menu(str,2);
      Num2str(((start_y<Paintbrush_Y)?Paintbrush_Y-start_y:start_y-Paintbrush_Y)+1,str,4);
      Print_in_menu(str,11);
    }
    else
      Print_coordinates();

    Operation_push(start_x);
    Operation_push(start_y);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Empty_rectangle_0_5(void)
// Opération   : OPERATION_EMPTY_RECTANGLE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
{
  short old_paintbrush_x;
  short old_paintbrush_y;
  short color;

  // On mémorise la position courante du pinceau:

  old_paintbrush_x=Paintbrush_X;
  old_paintbrush_y=Paintbrush_Y;

  // On lit l'ancienne position du pinceau:

  Operation_pop(&Paintbrush_Y);
  Operation_pop(&Paintbrush_X);
  Operation_pop(&Paintbrush_Y);
  Operation_pop(&Paintbrush_X);

  // On va devoir effacer l'ancien curseur qu'on a laissé trainer:
  Hide_cursor();

  // On lit la couleur du rectangle:
  Operation_pop(&color);

  // On fait un petit backup de l'image:
  Backup();

  // Et on trace le rectangle:
  Draw_empty_rectangle(Paintbrush_X,Paintbrush_Y,old_paintbrush_x,old_paintbrush_y,color);

  // Enfin, on rétablit la position du pinceau:
  Paintbrush_X=old_paintbrush_x;
  Paintbrush_Y=old_paintbrush_y;

  End_of_modification();

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:       Y:",0);
  Print_coordinates();
}


void Filled_rectangle_0_5(void)
// Opération   : OPERATION_FILLED_RECTANGLE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
{
  short old_paintbrush_x;
  short old_paintbrush_y;
  short color;

  // On mémorise la position courante du pinceau:

  old_paintbrush_x=Paintbrush_X;
  old_paintbrush_y=Paintbrush_Y;

  // On lit l'ancienne position du pinceau:

  Operation_pop(&Paintbrush_Y);
  Operation_pop(&Paintbrush_X);
  Operation_pop(&Paintbrush_Y);
  Operation_pop(&Paintbrush_X);

  // On va devoir effacer l'ancien curseur qu'on a laissé trainer:
  Hide_cursor();

  // On lit la couleur du rectangle:
  Operation_pop(&color);

  // On fait un petit backup de l'image:
  Backup();

  // Et on trace le rectangle:
  Draw_filled_rectangle(Paintbrush_X,Paintbrush_Y,old_paintbrush_x,old_paintbrush_y,color);

  // Enfin, on rétablit la position du pinceau:
  Paintbrush_X=old_paintbrush_x;
  Paintbrush_Y=old_paintbrush_y;

  End_of_modification();
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:",0);
    Print_coordinates();
  }
}


////////////////////////////////////////////////////// OPERATION_CERCLE_?????


void Circle_12_0(void)
//
// Opération   : OPERATION_EMPTY_CIRCLE / OPERATION_FILLED_CIRCLE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();

  Paintbrush_shape_before_operation=Paintbrush_shape;
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  if (Mouse_K==LEFT_SIDE)
  {
    Shade_table=Shade_table_left;
    Operation_push(Fore_color);
  }
  else
  {
    Shade_table=Shade_table_right;
    Operation_push(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("Radius:   0    ",0);

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Circle_12_5(void)
//
// Opération   : OPERATION_EMPTY_CIRCLE / OPERATION_FILLED_CIRCLE
// Click Souris: 1 ou 2
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  long limit;
  char  str[5];

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  if ( (tangent_x!=Paintbrush_X) || (tangent_y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_TARGET;
    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Num2str(Distance(center_x,center_y,Paintbrush_X,Paintbrush_Y),str,4);
      Print_in_menu(str,7);
    }
    else
      Print_coordinates();

    limit=((tangent_x-center_x)*(tangent_x-center_x))+
           ((tangent_y-center_y)*(tangent_y-center_y));
    Hide_empty_circle_preview(center_x,center_y,limit);

    limit=((Paintbrush_X-center_x)*(Paintbrush_X-center_x))+
           ((Paintbrush_Y-center_y)*(Paintbrush_Y-center_y));
    Draw_empty_circle_preview(center_x,center_y,limit,color);

    Display_cursor();
  }

  Operation_push(color);
  Operation_push(center_x);
  Operation_push(center_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Empty_circle_0_5(void)
//
// Opération   : OPERATION_EMPTY_CIRCLE
// Click Souris: 0
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  long limit;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  limit=((tangent_x-center_x)*(tangent_x-center_x))+
         ((tangent_y-center_y)*(tangent_y-center_y));
  Hide_empty_circle_preview(center_x,center_y,limit);

  Paintbrush_shape=Paintbrush_shape_before_operation;

  Draw_empty_circle_permanent(center_x,center_y,limit,color);

  End_of_modification();

  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:",0);
    Print_coordinates();
  }
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;
}


void Filled_circle_0_5(void)
//
// Opération   : OPERATION_FILLED_CIRCLE
// Click Souris: 0
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  long limit;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  limit=((tangent_x-center_x)*(tangent_x-center_x))+
         ((tangent_y-center_y)*(tangent_y-center_y));
  Hide_empty_circle_preview(center_x,center_y,limit);

  Paintbrush_shape=Paintbrush_shape_before_operation;

  Draw_filled_circle(center_x,center_y,limit,color);

  End_of_modification();
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:",0);
    Print_coordinates();
  }
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;
}


///////////////////////////////////////////////////// OPERATION_ELLIPSE_?????


void Ellipse_12_0(void)
//
// Opération   : OPERATION_EMPTY_ELLIPSE / OPERATION_FILLED_ELLIPSE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();

  Paintbrush_shape_before_operation=Paintbrush_shape;
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  if (Mouse_K==LEFT_SIDE)
  {
    Shade_table=Shade_table_left;
    Operation_push(Fore_color);
  }
  else
  {
    Shade_table=Shade_table_right;
    Operation_push(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Ellipse_12_5(void)
//
// Opération   : OPERATION_EMPTY_ELLIPSE / OPERATION_FILLED_ELLIPSE
// Click Souris: 1 ou 2
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short horizontal_radius;
  short vertical_radius;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);


  if ( (tangent_x!=Paintbrush_X) || (tangent_y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Display_coords_rel_or_abs(center_x,center_y);

    if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
    {
      Cursor_shape=CURSOR_SHAPE_TARGET;
      horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                             :center_x-tangent_x;
      vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                             :center_y-tangent_y;
      Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);

      horizontal_radius=(Paintbrush_X>center_x)?Paintbrush_X-center_x
                                           :center_x-Paintbrush_X;
      vertical_radius  =(Paintbrush_Y>center_y)?Paintbrush_Y-center_y
                                           :center_y-Paintbrush_Y;
      Draw_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius,color);
    }
    else
    {
      Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
      Draw_empty_inscribed_ellipse_preview(center_x,center_y,Paintbrush_X,Paintbrush_Y,color);
    }

    Display_cursor();
  }

  Operation_push(color);
  Operation_push(center_x);
  Operation_push(center_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Empty_ellipse_0_5(void)
//
// Opération   : OPERATION_EMPTY_ELLIPSE
// Click Souris: 0
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short horizontal_radius;
  short vertical_radius;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
  {
    horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                           :center_x-tangent_x;
    vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                           :center_y-tangent_y;
    Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);
    Paintbrush_shape=Paintbrush_shape_before_operation;
    Draw_empty_ellipse_permanent(center_x,center_y,horizontal_radius,vertical_radius,color);
  }
  else
  {
    Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
    Paintbrush_shape=Paintbrush_shape_before_operation;
    Draw_empty_inscribed_ellipse_permanent(center_x,center_y,tangent_x,tangent_y,color);
  }

  End_of_modification();

  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;
}


void Filled_ellipse_0_5(void)
//
// Opération   : OPERATION_FILLED_ELLIPSE
// Click Souris: 0
// Taille_Pile : 5 (color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short horizontal_radius;
  short vertical_radius;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
  {
    horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                           :center_x-tangent_x;
    vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                           :center_y-tangent_y;
    Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);
    Paintbrush_shape=Paintbrush_shape_before_operation;
    Draw_filled_ellipse(center_x,center_y,horizontal_radius,vertical_radius,color);
  }
  else
  {
    Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
    Paintbrush_shape=Paintbrush_shape_before_operation;
    Draw_filled_inscribed_ellipse(center_x,center_y,tangent_x,tangent_y,color);
  }

  End_of_modification();
  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;
}


////////////////////////////////////////////////////////////// OPERATION_FILL


void Fill_1_0(void)
//
// Opération   : OPERATION_FILL
// Click Souris: 1
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Hide_cursor();
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Fill, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  // On ne s'occupe pas de faire un Backup: c'est "Fill_general" qui s'en charge.
  Shade_table=Shade_table_left;
  Fill_general(Fore_color);
  Display_cursor();
  End_of_modification();
  Wait_end_of_click();
}


void Fill_2_0(void)
//
// Opération   : OPERATION_FILL
// Click Souris: 2
// Taille_Pile : 0
//
// Souris effacée: Non
//
{
  if (Rightclick_colorpick(1))
    return;

  Hide_cursor();
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Fill, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  // On ne s'occupe pas de faire un Backup: c'est "Fill_general" qui s'en charge.
  Shade_table=Shade_table_right;
  Fill_general(Back_color);
  Display_cursor();
  End_of_modification();
  Wait_end_of_click();
}


///////////////////////////////////////////////////////// OPERATION_REPLACE


void Replace_1_0(void)
//
// Opération   : OPERATION_REPLACE
// Click Souris: 1
// Taille_Pile : 0
//
// Souris effacée: Non
//
{
  Hide_cursor();
  Init_start_operation();
  Backup();
//  Shade_table=Shade_table_left;
  Replace(Fore_color);
  End_of_modification();
  Display_all_screen();
  Display_cursor();
  Wait_end_of_click();
}


void Replace_2_0(void)
//
// Opération   : OPERATION_REPLACE
// Click Souris: 2
// Taille_Pile : 0
//
// Souris effacée: Non
//
{
  if (Rightclick_colorpick(1))
    return;

  Hide_cursor();
  Init_start_operation();
  Backup();
//  Shade_table=Shade_table_right;
  Replace(Back_color);
  End_of_modification();
  Display_all_screen();
  Display_cursor();
  Wait_end_of_click();
}


/////////////////////////////////////////////////// OPERATION_4_POINTS_CURVE


void Draw_curve_cross(short x_pos, short y_pos)
{
  short start_x,end_x;
  short start_y,end_y;
  short i,temp;
  //byte  temp2;

  if (x_pos>=Limit_left+3)
    start_x=0;
  else
    start_x=3-(x_pos-Limit_left);

  if (y_pos>=Limit_top+3)
    start_y=0;
  else
    start_y=3-(y_pos-Limit_top);

  if (x_pos<=Limit_visible_right-3)
    end_x=6;
  else
    end_x=3+(Limit_visible_right-x_pos);

  if (y_pos<=Limit_visible_bottom-3)
    end_y=6;
  else
    end_y=3+(Limit_visible_bottom-y_pos);

  if (start_x<=end_x && start_y<=end_y)
  {
    for (i=start_x; i<=end_x; i++)
    {
      temp=x_pos+i-3;
      Pixel_preview(temp,y_pos,xor_lut[Read_pixel(temp -Main.offset_X,
                                          y_pos-Main.offset_Y)]);
    }
    for (i=start_y; i<=end_y; i++)
    {
      temp=y_pos+i-3;
      Pixel_preview(x_pos,temp,xor_lut[Read_pixel(x_pos-Main.offset_X,
                                          temp -Main.offset_Y)]);
    }
    Update_part_of_screen(x_pos+start_x-3,y_pos+start_y-3,end_x-start_x+1,end_y-start_y+1);
  }
}


void Curve_34_points_1_0(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  Init_start_operation();
  Backup();
  Shade_table=Shade_table_left;

  Paintbrush_hidden=1;

  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,Fore_color);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Fore_color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Curve_34_points_2_0(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=Shade_table_right;

  Paintbrush_hidden=1;

  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,Back_color);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Back_color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Curve_34_points_1_5(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 1
//  Taille_Pile : 5
//
//  Souris effacée: Non
//
{
  short x1,x2,y1,y2;

  Operation_pop(&y2);
  Operation_pop(&x2);
  Operation_pop(&y1);
  Operation_pop(&x1);

  if ( (y2!=Paintbrush_Y) || (x2!=Paintbrush_X) )
  {
    Hide_cursor();
    Display_coords_rel_or_abs(x1,y1);

    Hide_line_preview(x1,y1,x2,y2);
    Pixel_figure_preview (x1,y1,Fore_color);
    Draw_line_preview (x1,y1,Paintbrush_X,Paintbrush_Y,Fore_color);

    Display_cursor();
  }

  Operation_push(x1);
  Operation_push(y1);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Curve_34_points_2_5(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 2
//  Taille_Pile : 5
//
//  Souris effacée: Non
//
{
  short x1,x2,y1,y2;

  Operation_pop(&y2);
  Operation_pop(&x2);
  Operation_pop(&y1);
  Operation_pop(&x1);

  if ( (y2!=Paintbrush_Y) || (x2!=Paintbrush_X) )
  {
    Hide_cursor();
    Display_coords_rel_or_abs(x1,y1);

    Hide_line_preview(x1,y1,x2,y2);
    Pixel_figure_preview (x1,y1,Back_color);
    Draw_line_preview (x1,y1,Paintbrush_X,Paintbrush_Y,Back_color);

    Display_cursor();
  }

  Operation_push(x1);
  Operation_push(y1);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


byte Cursor_hidden_before_curve;

void Curve_4_points_0_5(void)
//
//  Opération   : OPERATION_4_POINTS_CURVE
//  Click Souris: 0
//  Taille_Pile : 5
//
//  Souris effacée: Oui
//
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short third_x,third_y;
  short color;

  Operation_pop(&y4);
  Operation_pop(&x4);
  Operation_pop(&y1);
  Operation_pop(&x1);
  Operation_pop(&color);

  third_x=Round_div(abs(x4-x1),3);
  third_y=Round_div(abs(y4-y1),3);

  if (x1<x4)
  {
    x2=x1+third_x;
    x3=x4-third_x;
  }
  else
  {
    x3=x4+third_x;
    x2=x1-third_x;
  }

  if (y1<y4)
  {
    y2=y1+third_y;
    y3=y4-third_y;
  }
  else
  {
    y3=y4+third_y;
    y2=y1-third_y;
  }

  Hide_line_preview(x1,y1,x4,y4);
  Draw_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);

  // On trace les petites croix montrant les 2 points intermédiares
  Draw_curve_cross(x2,y2);
  Draw_curve_cross(x3,y3);

  Cursor_hidden_before_curve=Cursor_hidden;
  Cursor_hidden=0;

  Operation_push(color);
  Operation_push(x1);
  Operation_push(y1);
  Operation_push(x2);
  Operation_push(y2);
  Operation_push(x3);
  Operation_push(y3);
  Operation_push(x4);
  Operation_push(y4);

  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
}


void Curve_4_points_1_9(void)
//
//  Opération   : OPERATION_4_POINTS_CURVE
//  Click Souris: 1
//  Taille_Pile : 9
//
//  Souris effacée: Non
//
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short color;
  byte  it_is_x2;

  Operation_pop(&y4);
  Operation_pop(&x4);
  Operation_pop(&y3);
  Operation_pop(&x3);
  Operation_pop(&y2);
  Operation_pop(&x2);
  Operation_pop(&y1);
  Operation_pop(&x1);
  Operation_pop(&color);

  it_is_x2=(Distance(Paintbrush_X,Paintbrush_Y,x2,y2) < Distance(Paintbrush_X,Paintbrush_Y,x3,y3));

  if ( (   it_is_x2  && ( (Paintbrush_X!=x2) || (Paintbrush_Y!=y2) ) )
    || ( (!it_is_x2) && ( (Paintbrush_X!=x3) || (Paintbrush_Y!=y3) ) ) )
  {
    Hide_cursor();
    Print_coordinates();

    Draw_curve_cross(x2,y2);
    Draw_curve_cross(x3,y3);

    Hide_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);

    if (it_is_x2)
    {
      x2=Paintbrush_X;
      y2=Paintbrush_Y;
    }
    else
    {
      x3=Paintbrush_X;
      y3=Paintbrush_Y;
    }

    Draw_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);

    Draw_curve_cross(x2,y2);
    Draw_curve_cross(x3,y3);

    Display_cursor();
  }

  Operation_push(color);
  Operation_push(x1);
  Operation_push(y1);
  Operation_push(x2);
  Operation_push(y2);
  Operation_push(x3);
  Operation_push(y3);
  Operation_push(x4);
  Operation_push(y4);
}


void Curve_4_points_2_9(void)
//
//  Opération   : OPERATION_4_POINTS_CURVE
//  Click Souris: 2
//  Taille_Pile : 9
//
//  Souris effacée: Oui
//
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short color;

  Operation_pop(&y4);
  Operation_pop(&x4);
  Operation_pop(&y3);
  Operation_pop(&x3);
  Operation_pop(&y2);
  Operation_pop(&x2);
  Operation_pop(&y1);
  Operation_pop(&x1);
  Operation_pop(&color);

  Hide_cursor();

  Draw_curve_cross(x2,y2);
  Draw_curve_cross(x3,y3);

  Paintbrush_hidden=0;
  Cursor_hidden=Cursor_hidden_before_curve;

  Hide_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);
  Draw_curve_permanent(x1,y1,x2,y2,x3,y3,x4,y4,color);

  Display_cursor();
  End_of_modification();
  Wait_end_of_click();
}


/////////////////////////////////////////////////// OPERATION_3_POINTS_CURVE


void Compute_3_point_curve(short x1, short y1, short x4, short y4,
                              short * x2, short * y2, short * x3, short * y3)
{
  float cx,cy; // Centre de (x1,y1) et (x4,y4)
  float bx,by; // Intersect. des dtes ((x1,y1),(x2,y2)) et ((x3,y3),(x4,y4))

  cx=(float)(x1+x4)/2.0;              // P1*--_               Legend:
  cy=(float)(y1+y4)/2.0;              //   ·   \·· P2         -_\|/ : curve
                                      //   ·    \ ·*·         * : important point
  bx=cx+((8.0/3.0)*(Paintbrush_X-cx));//   ·     |   ··       · : dotted line
  by=cy+((8.0/3.0)*(Paintbrush_Y-cy));//   ·     |P    ··  B
                                      // C *·····*·········*  P=Pencil position
  *x2=Round((bx+x1)/2.0);             //   ·     |     ··     C=middle of [P1,P4]
  *y2=Round((by+y1)/2.0);             //   ·     |   ··       B=point computed as
                                      //   ·    / ·*·         C->B=(8/3) * C->P
  *x3=Round((bx+x4)/2.0);             //   ·  _/·· P3         P2=middle of [P1,B]
  *y3=Round((by+y4)/2.0);             // P4*--                P3=middle of [P4,B]
}


void Curve_3_points_0_5(void)
//
//  Opération   : OPERATION_3_POINTS_CURVE
//  Click Souris: 0
//  Taille_Pile : 5
//
//  Souris effacée: Oui
//
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short color;

  Operation_pop(&y4);
  Operation_pop(&x4);
  Operation_pop(&y1);
  Operation_pop(&x1);
  Operation_pop(&color);

  Compute_3_point_curve(x1,y1,x4,y4,&x2,&y2,&x3,&y3);

  if (!Config.Stylus_mode)
  {
    Hide_line_preview(x1,y1,x4,y4);
    Draw_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);
  }

  if ( (Config.Coords_rel) && (Menu_is_visible) )
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }

  Operation_push(color);
  Operation_push(x1);
  Operation_push(y1);
  Operation_push(x2);
  Operation_push(y2);
  Operation_push(x3);
  Operation_push(y3);
  Operation_push(x4);
  Operation_push(y4);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);

  if (Config.Stylus_mode)
  {
    Display_cursor();
    while(!Mouse_K)
      Get_input(20);
    Hide_cursor();

    Hide_line_preview(x1,y1,x4,y4);
  }
}

void Curve_drag(void)
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short old_x,old_y;
  short color;

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ( (Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y) )
  {
    Operation_pop(&y4);
    Operation_pop(&x4);
    Operation_pop(&y3);
    Operation_pop(&x3);
    Operation_pop(&y2);
    Operation_pop(&x2);
    Operation_pop(&y1);
    Operation_pop(&x1);
    Operation_pop(&color);

    Hide_cursor();
    Print_coordinates();

    Hide_curve_preview(x1,y1,x2,y2,x3,y3,x4,y4,color);
    Compute_3_point_curve(x1,y1,x4,y4,&x2,&y2,&x3,&y3);
    Draw_curve_preview (x1,y1,x2,y2,x3,y3,x4,y4,color);
    Display_cursor();

    Operation_push(color);
    Operation_push(x1);
    Operation_push(y1);
    Operation_push(x2);
    Operation_push(y2);
    Operation_push(x3);
    Operation_push(y3);
    Operation_push(x4);
    Operation_push(y4);
  }
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}
void Curve_finalize(void)
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  short old_x,old_y;
  short color;

  Operation_pop(&old_y);
  Operation_pop(&old_x);
  Operation_pop(&y4);
  Operation_pop(&x4);
  Operation_pop(&y3);
  Operation_pop(&x3);
  Operation_pop(&y2);
  Operation_pop(&x2);
  Operation_pop(&y1);
  Operation_pop(&x1);
  Operation_pop(&color);

  Paintbrush_hidden=0;

  Hide_cursor();

  Hide_curve_preview (x1,y1,x2,y2,x3,y3,x4,y4,color);
  Compute_3_point_curve(x1,y1,x4,y4,&x2,&y2,&x3,&y3);
  Draw_curve_permanent(x1,y1,x2,y2,x3,y3,x4,y4,color);

  End_of_modification();
  Display_cursor();
  Wait_end_of_click();
}

void Curve_3_points_0_11(void)
//
//  Opération   : OPERATION_3_POINTS_CURVE
//  Click Souris: 0
//  Taille_Pile : 11
//
//  Souris effacée: Non
//
{
  if (!Config.Stylus_mode)
    Curve_drag();
  else
    Curve_finalize();
}


void Curve_3_points_12_11(void)
//
//  Opération   : OPERATION_3_POINTS_CURVE
//  Click Souris: 1 ou 2
//  Taille_Pile : 11
//
//  Souris effacée: Oui
//
{
  if (!Config.Stylus_mode)
    Curve_finalize();
  else
    Curve_drag();
}


///////////////////////////////////////////////////////////// OPERATION_AIRBRUSH

void Airbrush_1_0(void)
//
//  Opération   : OPERATION_AIRBRUSH
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Non
//
{
  Init_start_operation();
  Backup();
  Shade_table=Shade_table_left;

  if (GFX2_GetTicks()>Airbrush_next_time)
  {
    Airbrush(LEFT_SIDE);
    Airbrush_next_time = GFX2_GetTicks()+Airbrush_delay*10;
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Airbrush_2_0(void)
//
//  Opération   : OPERATION_AIRBRUSH
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Non
//
{
  if (Rightclick_colorpick(1))
    return;

  Init_start_operation();
  Backup();
  Shade_table=Shade_table_right;
  if (GFX2_GetTicks()>Airbrush_next_time)
  {
    Airbrush(RIGHT_SIDE);
    Airbrush_next_time = GFX2_GetTicks()+Airbrush_delay*10;
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Airbrush_12_2(void)
//
//  Opération   : OPERATION_AIRBRUSH
//  Click Souris: 1 ou 2
//  Taille_Pile : 2
//
//  Souris effacée: Non
//
{
  short old_x,old_y;
  dword now;

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ( (Menu_is_visible) && ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y)) )
  {
    Hide_cursor();
    Print_coordinates();
    Display_cursor();
  }

  now=GFX2_GetTicks();
  if (now>Airbrush_next_time)
  {
    //Airbrush_next_time+=Airbrush_delay*10;
    // Time is now reset, because the += was death spiral
    // if drawing took more time than the frequency.
    Airbrush_next_time=now+Airbrush_delay*10;
    Airbrush(Mouse_K_unique);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Airbrush_0_2(void)
//
//  Opération   : OPERATION_AIRBRUSH
//  Click Souris: 0
//  Taille_Pile : 2
//
//  Souris effacée: Non
//
{
  Operation_stack_size-=2;
  End_of_modification();
}


////////////////////////////////////////////////////////// OPERATION_POLYGON


void Polygon_12_0(void)
// Opération   : OPERATION_POLYGON
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte color;

  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  Paintbrush_shape_before_operation=Paintbrush_shape;
  Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Mouse_K | 0x80);
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  // Taille de pile 8 : phase d'appui, non interruptible
}



void Polygon_12_9(void)
// Opération   : OPERATION_POLYGON
// Click Souris: 1 ou 2
// Taille_Pile : 9
//
// Souris effacée: Oui
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;
  short direction;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);
  Operation_pop(&direction);
  Operation_pop(&direction);

  if (direction==Mouse_K)
  {
    Operation_push(direction);
    Operation_push(color);
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(end_x);
    Operation_push(end_y);
    // Taille de pile 8 : phase d'appui, non interruptible
  }
  else
  {
    //   La série de ligne est terminée, il faut donc effacer la dernière
    // preview de ligne et relier le dernier point avec le premier
    Pixel_figure_preview_auto  (start_x,start_y);
    Hide_line_preview (start_x,start_y,end_x,end_y);
    Operation_pop(&end_y);
    Operation_pop(&end_x);
    Paintbrush_shape=Paintbrush_shape_before_operation;
    // Le pied aurait été de ne pas repasser sur le 1er point de la 1ère ligne
    // mais c'est pas possible :(
    Draw_line_permanent(start_x,start_y,end_x,end_y,color);
    Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

    Display_cursor();
    End_of_modification();
    Wait_end_of_click();
    Hide_cursor();

    if ( (Config.Coords_rel) && (Menu_is_visible) )
    {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
    }

    Paintbrush_shape=Paintbrush_shape_before_operation;
  }
}


////////////////////////////////////////////////////////// OPERATION_POLYFILL

void Polyfill_12_0(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte color;

  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  Paintbrush_hidden=1;

  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  Polyfill_table_of_points=(short *) malloc((Config.Nb_max_vertices_per_polygon<<1)*sizeof(short));
  Polyfill_table_of_points[0]=Paintbrush_X;
  Polyfill_table_of_points[1]=Paintbrush_Y;
  Polyfill_number_of_points=1;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_preview_xor(Paintbrush_X,Paintbrush_Y,0);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Mouse_K | 0x80);
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  // Taille de pile 8 : phase d'appui, non interruptible
}


void Polyfill_0_8(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 0
// Taille_Pile : 8
//
// Souris effacée: Oui
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;
  short direction;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);
  Operation_pop(&direction);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);

  if (direction & 0x80)
    direction=(direction & 0x7F);

  Operation_push(direction); // Valeur bidon servant de nouvel état de pile
  Operation_push(direction);
  Operation_push(color);

  Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);

  if (Polyfill_number_of_points<Config.Nb_max_vertices_per_polygon)
  {
    Polyfill_table_of_points[Polyfill_number_of_points<<1]    =Paintbrush_X;
    Polyfill_table_of_points[(Polyfill_number_of_points<<1)+1]=Paintbrush_Y;
    Polyfill_number_of_points++;

    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
  }
  else
  {
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
  }
  // Taille de pile 9 : phase de "repos", interruptible (comme Elliot Ness :))
}


void Polyfill_12_8(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2 | 0
// Taille_Pile : 8      | 9
//
// Souris effacée: Non
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;

  Operation_pop(&end_y);
  Operation_pop(&end_x);

  if ((Paintbrush_X!=end_x) || (Paintbrush_Y!=end_y))
  {
    Hide_cursor();
    Operation_pop(&start_y);
    Operation_pop(&start_x);

    Display_coords_rel_or_abs(start_x,start_y);

    Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
    Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);

    Operation_push(start_x);
    Operation_push(start_y);
    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Polyfill_12_9(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2
// Taille_Pile : 9
//
// Souris effacée: Oui
{
  short start_x;
  short start_y;
  short end_x;
  short end_y;
  short color;
  short direction;

  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&color);
  Operation_pop(&direction);
  Operation_pop(&direction);

  if (direction==Mouse_K)
  {
    Operation_push(direction);
    Operation_push(color);
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(end_x);
    Operation_push(end_y);
    // Taille de pile 8 : phase d'appui, non interruptible
  }
  else
  {
    //   La série de lignes est terminée, il faut donc effacer la dernière
    // preview de ligne et relier le dernier point avec le premier
    Hide_cursor();
    Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
    Operation_pop(&end_y);
    Operation_pop(&end_x);
    Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);

    Display_all_screen();
    Polyfill(Polyfill_number_of_points,Polyfill_table_of_points,color);
    free(Polyfill_table_of_points);
    Polyfill_table_of_points = NULL;

    End_of_modification();
    if ( (Config.Coords_rel) && (Menu_is_visible) )
    {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
    }

    Paintbrush_hidden=0;

    Display_cursor();
    Wait_end_of_click();
  }
}


////////////////////////////////////////////////////////// OPERATION_POLYFORM


void Polyform_12_0(void)
//  Opération   : OPERATION_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  short color;

  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();
  Backup();
  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;

  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  // On place un premier pinceau en (Paintbrush_X,Paintbrush_Y):
  Draw_paintbrush(Paintbrush_X,Paintbrush_Y,color);
  // Et on affiche un pixel de preview en (Paintbrush_X,Paintbrush_Y):
  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);

  Operation_push(Paintbrush_X); // X Initial
  Operation_push(Paintbrush_Y); // X Initial
  Operation_push(color);   // color de remplissage
  Operation_push(Paintbrush_X); // Start X
  Operation_push(Paintbrush_Y); // Start Y
  Operation_push(Paintbrush_X); // End X
  Operation_push(Paintbrush_Y); // End Y
  Operation_push(Mouse_K);   // click
}


void Polyform_12_8(void)
//  Opération   : OPERATION_POLYFORM
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
    // L'utilisateur clic toujours avec le bon bouton de souris

    if ((start_x!=Paintbrush_X) || (start_y!=Paintbrush_Y))
    {
      // Il existe un segment définit par (start_x,start_y)-(Paintbrush_X,Paintbrush_Y)

      Hide_cursor();
      Print_coordinates();

      Operation_pop(&color);

      // On efface la preview du segment validé:
      Pixel_figure_preview_auto  (start_x,start_y);
      Hide_line_preview(start_x,start_y,end_x,end_y);

      // On l'affiche de façon définitive:
      Draw_line_permanent(start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

      // Et on affiche un pixel de preview en (Paintbrush_X,Paintbrush_Y):
      Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);

      Operation_push(color);

      Display_cursor();
    }

    Operation_push(Paintbrush_X); // Nouveau start_x
    Operation_push(Paintbrush_Y); // Nouveau start_y
    Operation_push(Paintbrush_X); // Nouveau end_x
    Operation_push(Paintbrush_Y); // Nouveau end_y
    Operation_push(click);
  }
  else
  {
    // L'utilisateur souhaite arrêter l'opération et refermer le polygone

    Operation_pop(&color);
    Operation_pop(&initial_y);
    Operation_pop(&initial_x);

    Hide_cursor();
    Print_coordinates();

    // On efface la preview du segment annulé:
    Hide_line_preview(start_x,start_y,end_x,end_y);

    // On affiche de façon définitive le bouclage du polygone:
    Draw_line_permanent(start_x,start_y,initial_x,initial_y,color);

    Display_cursor();
    End_of_modification();
    Wait_end_of_click();
  }
}


void Polyform_0_8(void)
//  Opération   : OPERATION_POLYFORM
//  Click Souris: 0
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

  Operation_pop(&click);
  Operation_pop(&end_y);
  Operation_pop(&end_x);

  if ((end_x!=Paintbrush_X) || (end_y!=Paintbrush_Y))
  {
    Hide_cursor();
    Print_coordinates();

    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Operation_pop(&color);

    // On met à jour l'affichage de la preview du prochain segment:
    Hide_line_preview(start_x,start_y,end_x,end_y);
    Draw_line_preview (start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

    Operation_push(color);
    Operation_push(start_x);
    Operation_push(start_y);

    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(click);
}


/////////////////////////////////////////////////// OPERATION_FILLED_POLYFORM


void Filled_polyform_12_0(void)
//  Opération   : OPERATION_FILLED_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  short color;

  if (Rightclick_colorpick(0))
    return;

  Init_start_operation();

  // Cette opération étant également utilisée pour le lasso, on ne fait pas de
  // backup si on prend une brosse au lasso avec le bouton gauche.
  if ((Current_operation==OPERATION_FILLED_POLYFORM) || (Current_operation==OPERATION_FILLED_CONTOUR) || (Mouse_K==RIGHT_SIDE))
    Backup();

  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  Paintbrush_hidden=1;

  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  Polyfill_table_of_points=(short *) malloc((Config.Nb_max_vertices_per_polygon<<1)*sizeof(short));
  Polyfill_table_of_points[0]=Paintbrush_X;
  Polyfill_table_of_points[1]=Paintbrush_Y;
  Polyfill_number_of_points=1;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_preview_xor(Paintbrush_X,Paintbrush_Y,0);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  Operation_push(Paintbrush_X); // X Initial
  Operation_push(Paintbrush_Y); // X Initial
  Operation_push(color);   // color de remplissage
  Operation_push(Paintbrush_X); // Start X
  Operation_push(Paintbrush_Y); // Start Y
  Operation_push(Paintbrush_X); // End X
  Operation_push(Paintbrush_Y); // End Y
  Operation_push(Mouse_K);   // click
}


void Filled_polyform_12_8(void)
//  Opération   : OPERATION_FILLED_POLYFORM
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
      if (Current_operation==OPERATION_FILLED_CONTOUR)
      {
        Draw_line_preview_xorback(start_x,start_y,end_x,end_y,0);
        Draw_line_preview_xorback(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);
      }
      else
      {
        Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
        Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);
      }

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
    // puisque on les effaces toutes d'un coup.

    Display_all_screen();
    Polyfill(Polyfill_number_of_points,Polyfill_table_of_points,color);
    free(Polyfill_table_of_points);
    Polyfill_table_of_points = NULL;

    Paintbrush_hidden=0;

    Display_cursor();
    End_of_modification();
    Wait_end_of_click();
  }
}


void Filled_polyform_0_8(void)
//  Opération   : OPERATION_FILLED_POLYFORM
//  Click Souris: 0
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short click;
  short end_y;
  short end_x;
  short start_y;
  short start_x;

  Operation_pop(&click);
  Operation_pop(&end_y);
  Operation_pop(&end_x);

  if ((end_x!=Paintbrush_X) || (end_y!=Paintbrush_Y))
  {
    Hide_cursor();
    Print_coordinates();

    Operation_pop(&start_y);
    Operation_pop(&start_x);

    // On met à jour l'affichage de la preview du prochain segment:
    Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
    Draw_line_preview_xor(start_x,start_y,Paintbrush_X,Paintbrush_Y,0);

    Operation_push(start_x);
    Operation_push(start_y);

    Display_cursor();
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(click);
}

void Filled_contour_0_8(void)
//  Opération   : OPERATION_FILLED_CONTOUR
//  Click Souris: 0
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
  Operation_pop(&color);
  Operation_pop(&initial_y);
  Operation_pop(&initial_x);

  Hide_cursor();
  Print_coordinates();

  // Pas besoin d'effacer la ligne (start_x,start_y)-(end_x,end_y)
  // puisque on les effaces toutes d'un coup.

  Display_all_screen();
  Polyfill(Polyfill_number_of_points,Polyfill_table_of_points,color);
  free(Polyfill_table_of_points);
  Polyfill_table_of_points = NULL;

  Paintbrush_hidden=0;

  Display_cursor();
  End_of_modification();
}


//////////////////////////////////////////////////////////// OPERATION_SCROLL


byte Cursor_hidden_before_scroll;

void Scroll_12_0(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  Init_start_operation();
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Mouse_K); // LEFT_SIDE or RIGHT_SIDE
  if (Mouse_K == LEFT_SIDE)
    Backup();
  else
  {
    Backup_layers(LAYER_ALL); // Main.layers_visible
    if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
    {
      // Ensure the backup visible image is up-to-date
      // (after swapping some layers on/off, it gets outdated)
      memcpy(Main_visible_image_backup.Image,
             Main.visible_image.Image,
             Main.image_width*Main.image_height);
    }
  }
  Update_screen_targets();

  Cursor_hidden_before_scroll=Cursor_hidden;
  Cursor_hidden=1;
  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);
}


void Scroll_12_5(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 1 ou 2
//  Taille_Pile : 5
//
//  Souris effacée: Non
//
{
  short center_x;
  short center_y;
  short x_pos;
  short y_pos;
  short x_offset;
  short y_offset;
  short side;
  //char  str[5];

  Operation_pop(&side);
  Operation_pop(&y_pos);
  Operation_pop(&x_pos);
  Operation_pop(&center_y);
  Operation_pop(&center_x);

  if ( (Paintbrush_X!=x_pos) || (Paintbrush_Y!=y_pos) )
  {
    // L'utilisateur a bougé, il faut scroller l'image

    if (Paintbrush_X>=center_x)
      x_offset=(Paintbrush_X-center_x)%Main.image_width;
    else
      x_offset=Main.image_width-((center_x-Paintbrush_X)%Main.image_width);

    if (Paintbrush_Y>=center_y)
      y_offset=(Paintbrush_Y-center_y)%Main.image_height;
    else
      y_offset=Main.image_height-((center_y-Paintbrush_Y)%Main.image_height);

    Display_coords_rel_or_abs(center_x,center_y);

    if (side == RIGHT_SIDE)
    {
      // All layers at once
      Scroll_picture(Screen_backup, Main_screen, x_offset,y_offset);
    }
    else
    {
      // One layer at once
      Scroll_picture(Main.backups->Pages->Next->Image[Main.current_layer].Pixels, Main.backups->Pages->Image[Main.current_layer].Pixels, x_offset, y_offset);
      Redraw_current_layer();
    }

    Display_all_screen();
  }

  Operation_push(center_x);
  Operation_push(center_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(side);
}

void Scroll_0_5(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 0
//  Taille_Pile : 5
//
//  Souris effacée: Oui
//
{
  // All layers at once
  short center_x;
  short center_y;
  short x_pos;
  short y_pos;
  short x_offset;
  short y_offset;
  short side;
  int i;


  Operation_pop(&side);
  Operation_pop(&y_pos);
  Operation_pop(&x_pos);
  Operation_pop(&center_y);
  Operation_pop(&center_x);

  if (side == RIGHT_SIDE)
  {
    // All layers at once
    if (x_pos>=center_x)
      x_offset=(x_pos-center_x)%Main.image_width;
    else
      x_offset=Main.image_width-((center_x-x_pos)%Main.image_width);

    if (y_pos>=center_y)
      y_offset=(y_pos-center_y)%Main.image_height;
    else
      y_offset=Main.image_height-((center_y-y_pos)%Main.image_height);

    // Do the actual scroll operation on all layers.
    for (i=0; i<Main.backups->Pages->Nb_layers; i++)
      //if ((1<<i) & Main.layers_visible)
      Scroll_picture(Main.backups->Pages->Next->Image[i].Pixels, Main.backups->Pages->Image[i].Pixels, x_offset, y_offset);
    // Update the depth buffer too ...
    // It would be faster to scroll it, but we don't have method
    // for in-place scrolling.
    Update_depth_buffer();
  }
  else
  {
    // One layer : everything was done while dragging the mouse
  }
  if (Main.tilemap_mode)
    Tilemap_update();

  Cursor_hidden=Cursor_hidden_before_scroll;

  End_of_modification();
  if ((Config.Coords_rel) && (Menu_is_visible))
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
}


//////////////////////////////////////////////////// OPERATION_GRAD_CIRCLE


void Grad_circle_12_0(void)
//
// Opération   : OPERATION_GRAD_CIRCLE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte color;

  Init_start_operation();
  Backup();
  Load_gradient_data(Current_gradient);

  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  Paintbrush_hidden_before_scroll=Paintbrush_hidden;
  Paintbrush_hidden=1;

  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("Radius:   0    ",0);

  Operation_push(Mouse_K);
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Grad_circle_12_6(void)
//
// Opération   : OPERATION_GRAD_CIRCLE
// Click Souris: 1 ou 2
// Taille_Pile : 6 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  long limit;
  char  str[5];

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);


  if ( (tangent_x!=Paintbrush_X) || (tangent_y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_TARGET;
    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Num2str(Distance(center_x,center_y,Paintbrush_X,Paintbrush_Y),str,4);
      Print_in_menu(str,7);
    }
    else
      Print_coordinates();

    limit=((tangent_x-center_x)*(tangent_x-center_x))+
           ((tangent_y-center_y)*(tangent_y-center_y));
    Hide_empty_circle_preview(center_x,center_y,limit);

    limit=((Paintbrush_X-center_x)*(Paintbrush_X-center_x))+
                  ((Paintbrush_Y-center_y)*(Paintbrush_Y-center_y));
    Draw_empty_circle_preview(center_x,center_y,limit,color);

    Display_cursor();
  }

  Operation_push(color);
  Operation_push(center_x);
  Operation_push(center_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Grad_circle_0_6(void)
//
// Opération   : OPERATION_GRAD_CIRCLE
// Click Souris: 0
// Taille_Pile : 6 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short click;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);

  Operation_pop(&color);
  Operation_pop(&click);

  if (click==LEFT_SIDE)
  {
    Operation_push(click);
    Operation_push(color);

    Operation_push(center_x);
    Operation_push(center_y);
    Operation_push(tangent_x);
    Operation_push(tangent_y);

    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);

    // On change la forme du curseur
    Cursor_shape=CURSOR_SHAPE_XOR_TARGET;

    // On affiche une croix XOR au centre du cercle
    Draw_curve_cross(center_x,center_y);

    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
        Print_in_menu("X:        Y:",0);
      else
        Print_in_menu("X:       Y:             ",0);
      Display_coords_rel_or_abs(center_x,center_y);
    }
  }
  else
  {
    long limit;
    limit=((tangent_x-center_x)*(tangent_x-center_x))+
           ((tangent_y-center_y)*(tangent_y-center_y));
    Hide_empty_circle_preview(center_x,center_y,limit);

    Paintbrush_hidden=Paintbrush_hidden_before_scroll;
    Cursor_shape=CURSOR_SHAPE_TARGET;

    Draw_filled_circle(center_x,center_y,limit,Back_color);

    End_of_modification();
    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
    }
  }
}


void Grad_circle_12_8(void)
//
// Opération   : OPERATION_GRAD_CIRCLE
// Click Souris: 0
// Taille_Pile : 8 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente, old_x, old_y)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short old_mouse_k;
  long limit;

  Operation_stack_size-=2;   // On fait sauter les 2 derniers élts de la pile
  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);
  Operation_pop(&old_mouse_k);

  Hide_cursor();
  // On efface la croix XOR au centre du cercle
  Draw_curve_cross(center_x,center_y);

  limit=((tangent_x-center_x)*(tangent_x-center_x))+
         ((tangent_y-center_y)*(tangent_y-center_y));
  Hide_empty_circle_preview(center_x,center_y,limit);

  Paintbrush_hidden=Paintbrush_hidden_before_scroll;
  Cursor_shape=CURSOR_SHAPE_TARGET;

  if (Mouse_K==old_mouse_k)
    Draw_grad_circle(center_x,center_y,limit,Paintbrush_X,Paintbrush_Y);
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;

  Display_cursor();
  End_of_modification();
  Wait_end_of_click();

  if ((Config.Coords_rel) && (Menu_is_visible))
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
}


void Grad_circle_or_ellipse_0_8(void)
//
// Opération   : OPERATION_{CERCLE|ELLIPSE}_DEGRADE
// Click Souris: 0
// Taille_Pile : 8
//
// Souris effacée: Non
//
{
  short start_x;
  short start_y;
  short tangent_x;
  short tangent_y;
  short old_x;
  short old_y;

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y))
  {
    Operation_pop(&tangent_y);
    Operation_pop(&tangent_x);
    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Display_coords_rel_or_abs(start_x,start_y);
    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(tangent_x);
    Operation_push(tangent_y);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


////////////////////////////////////////////////// OPERATION_GRAD_ELLIPSE


void Grad_ellipse_12_0(void)
//
// Opération   : OPERATION_GRAD_ELLIPSE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte color;

  Init_start_operation();
  Backup();
  Load_gradient_data(Current_gradient);


  Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;
  color=(Mouse_K==LEFT_SIDE)?Fore_color:Back_color;

  Paintbrush_hidden_before_scroll=Paintbrush_hidden;
  Paintbrush_hidden=1;

  Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);
  Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

  Operation_push(Mouse_K);
  Operation_push(color);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Grad_ellipse_12_6(void)
//
// Opération   : OPERATION_GRAD_ELLIPSE
// Click Souris: 1 ou 2
// Taille_Pile : 6 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short horizontal_radius;
  short vertical_radius;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);

  if ( (tangent_x!=Paintbrush_X) || (tangent_y!=Paintbrush_Y) )
  {
    Hide_cursor();
    Display_coords_rel_or_abs(center_x,center_y);

    if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
    {
      Cursor_shape=CURSOR_SHAPE_TARGET;
      horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                             :center_x-tangent_x;
      vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                             :center_y-tangent_y;
      Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);

      horizontal_radius=(Paintbrush_X>center_x)?Paintbrush_X-center_x
                                           :center_x-Paintbrush_X;
      vertical_radius  =(Paintbrush_Y>center_y)?Paintbrush_Y-center_y
                                           :center_y-Paintbrush_Y;
      Draw_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius,color);
    }
    else
    {
      Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
      Draw_empty_inscribed_ellipse_preview(center_x,center_y,Paintbrush_X,Paintbrush_Y,color);
    }

    Display_cursor();
  }

  Operation_push(color);
  Operation_push(center_x);
  Operation_push(center_y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Grad_ellipse_0_6(void)
//
// Opération   : OPERATION_GRAD_ELLIPSE
// Click Souris: 0
// Taille_Pile : 6 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short click;
  //short radius;
  short horizontal_radius;
  short vertical_radius;

  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);

  Operation_pop(&color);
  Operation_pop(&click);

  if (click==LEFT_SIDE)
  {
    Operation_push(click);
    Operation_push(color);

    Operation_push(center_x);
    Operation_push(center_y);
    Operation_push(tangent_x);
    Operation_push(tangent_y);

    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);

    // On change la forme du curseur
    Cursor_shape=CURSOR_SHAPE_XOR_TARGET;

    // On affiche une croix XOR au centre du cercle
    if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
      Draw_curve_cross(center_x,center_y);
    else
      Draw_curve_cross((center_x+tangent_x)/2, (center_y+tangent_y)/2);

    if (Menu_is_visible)
    {
      if (Config.Coords_rel)
        Print_in_menu("X:        Y:",0);
      else
        Print_in_menu("X:       Y:             ",0);
      Display_coords_rel_or_abs(center_x,center_y);
    }
  }
  else
  {
    Paintbrush_hidden=Paintbrush_hidden_before_scroll;
    Cursor_shape=CURSOR_SHAPE_TARGET;

    if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
    {
      horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                             :center_x-tangent_x;
      vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                             :center_y-tangent_y;
      Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);
      Draw_filled_ellipse(center_x,center_y,horizontal_radius,vertical_radius,Back_color);
    }
    else
    {
      Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
      Draw_filled_inscribed_ellipse(center_x,center_y,tangent_x,tangent_y,Back_color);
    }

    End_of_modification();
    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Print_in_menu("X:       Y:             ",0);
      Print_coordinates();
    }
  }
}


void Grad_ellipse_12_8(void)
//
// Opération   : OPERATION_GRAD_ELLIPSE
// Click Souris: 0
// Taille_Pile : 8 (Mouse_K, color, X_Centre, Y_Centre, X_Tangente, Y_Tangente, old_x, old_y)
//
// Souris effacée: Oui
//
{
  short tangent_x;
  short tangent_y;
  short center_x;
  short center_y;
  short color;
  short horizontal_radius=0;
  short vertical_radius=0;
  short old_mouse_k;

  Operation_stack_size-=2;   // On fait sauter les 2 derniers élts de la pile
  Operation_pop(&tangent_y);
  Operation_pop(&tangent_x);
  Operation_pop(&center_y);
  Operation_pop(&center_x);
  Operation_pop(&color);
  Operation_pop(&old_mouse_k);

  Hide_cursor();
  // On efface la croix XOR au centre de l'ellipse
  if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
  {
    Draw_curve_cross(center_x,center_y);
    horizontal_radius=(tangent_x>center_x)?tangent_x-center_x
                                           :center_x-tangent_x;
    vertical_radius  =(tangent_y>center_y)?tangent_y-center_y
                                           :center_y-tangent_y;
    Hide_empty_ellipse_preview(center_x,center_y,horizontal_radius,vertical_radius);
  }
  else
  {
    Draw_curve_cross((center_x+tangent_x)/2, (center_y+tangent_y)/2);
    Hide_empty_inscribed_ellipse_preview(center_x,center_y,tangent_x,tangent_y);
  }

  Paintbrush_hidden=Paintbrush_hidden_before_scroll;
  Cursor_shape=CURSOR_SHAPE_XOR_TARGET;

  if (Mouse_K==old_mouse_k)
  {
    if ((Selected_circle_ellipse_mode & MASK_CENTER_CORNERS) == MODE_CENTER)
      Draw_grad_ellipse(center_x,center_y,horizontal_radius,vertical_radius,Paintbrush_X,Paintbrush_Y);
    else
      Draw_grad_inscribed_ellipse(center_x,center_y,tangent_x,tangent_y,Paintbrush_X,Paintbrush_Y);
  }

  Display_cursor();
  End_of_modification();
  Wait_end_of_click();

  if ((Config.Coords_rel) && (Menu_is_visible))
  {
    Print_in_menu("X:       Y:             ",0);
    Print_coordinates();
  }
}

/******************************
* Operation_Rectangle_Degrade *
******************************/

// 1) tracé d'un rectangle classique avec les lignes XOR
// 2) tracé d'une ligne vecteur de dégradé, comme une ligne normale
// 3) dessin du dégradé


void Grad_rectangle_12_0(void)
// Opération   : OPERATION_GRAD_RECTANGLE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui

// Initialisation de l'étape 1, on commence à dessiner le rectangle
{
  Init_start_operation();
  Backup();
  Load_gradient_data(Current_gradient);

  if ((Config.Coords_rel) && (Menu_is_visible))
    Print_in_menu("\035:   1   \022:   1",0);
  // On laisse une trace du curseur à l'écran
  Display_cursor();

  if (Mouse_K==LEFT_SIDE)
  {
    Shade_table=Shade_table_left;
    Operation_push(Mouse_K);
  }
  else
  {
    Shade_table=Shade_table_right;
    Operation_push(Mouse_K);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}


void Grad_rectangle_12_5(void)
// Opération   : OPERATION_GRAD_RECTANGLE
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non

// Modification de la taille du rectangle
{
  short start_x;
  short start_y;
  short old_x;
  short old_y;
  char  str[5];

  Operation_pop(&old_y);
  Operation_pop(&old_x);

  if ((Paintbrush_X!=old_x) || (Paintbrush_Y!=old_y))
  {
    Operation_pop(&start_y);
    Operation_pop(&start_x);

    if ((Config.Coords_rel) && (Menu_is_visible))
    {
      Num2str(((start_x<Paintbrush_X)?Paintbrush_X-start_x:start_x-Paintbrush_X)+1,str,4);
      Print_in_menu(str,2);
      Num2str(((start_y<Paintbrush_Y)?Paintbrush_Y-start_y:start_y-Paintbrush_Y)+1,str,4);
      Print_in_menu(str,11);
    }
    else
      Print_coordinates();

    Operation_push(start_x);
    Operation_push(start_y);
  }

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

// Start and End coordinates must be sorted
void Draw_xor_rect(short start_x, short start_y, short end_x, short end_y)
{
  short offset_width = 0;
  short offset_height = 0;
  short offset_left = 0;
  short offset_top = 0;
  short width = end_x-start_x;
  short height = end_y-start_y;

  // Handle clipping
  if (end_x-Main.offset_X > Min(Main.image_width,
    Main.magnifier_mode?Main.separator_position:Screen_width))
  {
    offset_width = end_x - Min(Main.image_width,
      Main.magnifier_mode?Main.separator_position:Screen_width);
  }

  if (end_y-Main.offset_Y > Min(Main.image_height, Menu_Y))
    offset_height = end_y - Min(Main.image_height, Menu_Y);

  if (width == 0)
  {
    // Single line
    Vertical_XOR_line(start_x-Main.offset_X, start_y
      - Main.offset_Y, height - offset_height + 1);
  }
  else if (height == 0)
  {
    // Single line
    Horizontal_XOR_line(start_x-Main.offset_X,
      start_y - Main.offset_Y, width - offset_width + 1);
  }
  else
  {
    // Dessin dans la zone de dessin normale
    Horizontal_XOR_line(start_x-Main.offset_X,
      start_y - Main.offset_Y, width - offset_width + 1);

    // If not, this line is out of the picture so there is no need to draw it
    if (offset_height == 0 || end_y - 1 > Menu_Y + Main.offset_Y)
    {
      Horizontal_XOR_line(start_x - Main.offset_X, end_y
        - Main.offset_Y, width - offset_width + 1);
    }

    if (height > offset_height + 2)
    {
      Vertical_XOR_line(start_x-Main.offset_X, start_y
        - Main.offset_Y + 1, height - offset_height - 1);

      if (offset_width == 0)
      {
        Vertical_XOR_line(end_x - Main.offset_X, start_y
          - Main.offset_Y + 1, height - offset_height - 1);
      }
    }
  }

  Update_rect(start_x - Main.offset_X, start_y - Main.offset_Y,
    width + 1 - offset_width, height + 1 - offset_height);

  // Dessin dans la zone zoomée
  if (Main.magnifier_mode && start_x <= Limit_right_zoom
    && end_x > Limit_left_zoom
    && start_y <= Limit_bottom_zoom
    && end_y > Limit_top_zoom )
  {
    offset_width = 0;
    offset_height = 0;

    if (start_x<Limit_left_zoom) // On dépasse du zoom à gauche
    {
      offset_width += Limit_left_zoom - start_x;
      offset_left = Limit_left_zoom;
    }

    if(end_x>Limit_right_zoom) // On dépasse du zoom à droite
        offset_width += end_x - Limit_right_zoom;

    if(start_y<Limit_top_zoom) // On dépasse du zoom en haut
    {
        offset_height += Limit_top_zoom - start_y;
        offset_top = Limit_top_zoom;
    }

    if(end_y>Limit_bottom_zoom) // On dépasse du zoom en bas
        offset_height += end_y - Limit_bottom_zoom;

    if(width > offset_width)
    {
      if(offset_top==0) // La ligne du haut est visible
        Horizontal_XOR_line_zoom(offset_left>0?offset_left:start_x,start_y,width-offset_width+1);

      if(height!=0 && end_y<=Limit_bottom_zoom) // La  ligne du bas est visible
        Horizontal_XOR_line_zoom(offset_left>0?offset_left:start_x,end_y,width-offset_width+1);
    }
    if (width==0 && height!=0 && height > offset_height && offset_left==0)
    {
      // Single vertical line
      Vertical_XOR_line_zoom(start_x,offset_top!=0?offset_top:start_y,height-offset_height);
    }
    else
    {
      if(height > offset_height + 2)
      {
        if(offset_left==0) // La ligne de gauche est visible
          Vertical_XOR_line_zoom(start_x,offset_top!=0?offset_top:(start_y+1),height-offset_height-(offset_top==0)+(end_y>Limit_bottom_zoom));

        if(end_x<=Limit_right_zoom) // La ligne de droite est visible
          Vertical_XOR_line_zoom(end_x,offset_top!=0?offset_top:(start_y+1),height-offset_height-(offset_top==0)+(end_y>Limit_bottom_zoom));
      }
    }
  }
}
void Grad_rectangle_0_5(void)
// OPERATION_GRAD_RECTANGLE
// click souris 0
// Taile pile : 5
//
// Souris effacée : non

// Le rectangle est en place, maintenant il faut tracer le vecteur de dégradé,
// on doit donc attendre que l'utilisateur clique quelque part
// On stocke tout de suite les coordonnées du pinceau comme ça on change d'état et on passe à la suite
{
  // !!! Cette fonction remet start_x start_y end_x end_y dans la pile à la fin donc il ne faut pas les modifier ! (sauf éventuellement un tri)
  short start_x;
  short start_y;
  short end_x;
  short end_y;

  // Tracé propre du rectangle
  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);

  // This trick will erase the large crosshair at original position,
  // in normal and zoomed views.
  Paintbrush_X = start_x;
  Paintbrush_Y = start_y;

  if (start_x>end_x)
    SWAP_SHORTS(start_x, end_x)

  if (start_y>end_y)
    SWAP_SHORTS(start_y, end_y)

  Hide_cursor();

  // Check if the rectangle is not fully outside the picture
  if (start_x > Main.image_width                  // Rectangle at right of picture
      || start_y > Main.image_height              // Rectangle below picture
      || start_y - 1 - Main.offset_Y > Menu_Y )   // Rectangle below viewport
  {
    Operation_pop(&end_y); // reset the stack
    return; // cancel the operation
  }
  Draw_xor_rect(start_x, start_y, end_x, end_y);

  Operation_push(start_x);
  Operation_push(start_y);
  Operation_push(end_x);
  Operation_push(end_y);

  // On ajoute des trucs dans la pile pour forcer le passage à l'étape suivante
  Operation_push(end_x);
  Operation_push(end_y);
}

void Grad_rectangle_0_7(void)
// OPERATION_GRAD_RECTANGLE
// click souris 0
// Taile pile : 5
//
// Souris effacée : non

// On continue à attendre que l'utilisateur clique en gardant les coords à jour
{
    Operation_stack_size -= 2;
    Print_coordinates();
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
}

void Grad_rectangle_12_7(void)
// Opération   : OPERATION_GRAD_RECTANGLE
// Click Souris: 1 ou 2
// Taille_Pile : 7
//
//  Souris effacée: Oui

//  Début du tracé du vecteur (premier clic)
// On garde les anciennes coordonnées dans la pile, et on ajoute les nouvelles par dessus

// Si l'utilisateur utilise le mauvais bouton, on annule le tracé. Mais ça nous oblige à vider toute la pile pour vérifier :(
{
  short start_x,end_x,start_y,end_y,vax,vay,click;

  Operation_pop(&vay);
  Operation_pop(&vax);
  Operation_pop(&end_y);
  Operation_pop(&end_x);
  Operation_pop(&start_y);
  Operation_pop(&start_x);
  Operation_pop(&click);


  if(click==Mouse_K)
  {
      Operation_push(click);
      Operation_push(start_x);
      Operation_push(start_y);
      Operation_push(end_x);
      Operation_push(end_y);
      Operation_push(vax);
      Operation_push(vay);
      Operation_push(Paintbrush_X);
      Operation_push(Paintbrush_Y);

  }
  else
  {
      // Mauvais bouton > anulation de l'opération.
      // On a déjà vidé la pile, il reste à effacer le rectangle XOR
      Draw_xor_rect(start_x, start_y, end_x, end_y);
  }
}

void Grad_rectangle_12_9(void)
    // Opération   : OPERATION_GRAD_RECTANGLE
    // Click Souris: 1
    // Taille_Pile : 9
    //
    // Souris effacée: Oui

    // Poursuite du tracé du vecteur (déplacement de la souris en gardant le curseur appuyé)
{
    short start_x;
    short start_y;
    short end_x;
    short end_y;
    short cursor_x;
    short cursor_y;

    Operation_pop(&end_y);
    Operation_pop(&end_x);
    Operation_pop(&start_y);
    Operation_pop(&start_x);

    cursor_x = Paintbrush_X;
    cursor_y = Paintbrush_Y;
    // On corrige les coordonnées de la ligne si la touche shift est appuyée...
    if(Get_Key_modifiers() & GFX2_MOD_SHIFT)
        Clamp_coordinates_regular_angle(start_x,start_y,&cursor_x,&cursor_y);

    if ((cursor_x!=end_x) || (cursor_y!=end_y))
    {
        Display_coords_rel_or_abs(start_x,start_y);

        Draw_line_preview_xor(start_x,start_y,end_x,end_y,0);
        Draw_line_preview_xor(start_x,start_y,cursor_x,cursor_y,0);

    }


    Operation_push(start_x);
    Operation_push(start_y);
    Operation_push(cursor_x);
    Operation_push(cursor_y);
}

void Grad_rectangle_0_9(void)
    // Opération   : OPERATION_GRAD_RECTANGLE
    // Click Souris: 0
    // Taille_Pile : 9
    //
    //  Souris effacée: Oui

    // Ouf, fini ! on dessine enfin le rectangle avec son dégradé
{
    short rect_start_x;
    short rect_start_y;
    short rect_end_x;
    short rect_end_y;

    short vector_start_x;
    short vector_start_y;
    short vector_end_x;
    short vector_end_y;

    Operation_pop(&vector_end_y);
    Operation_pop(&vector_end_x);
    Operation_pop(&vector_start_y);
    Operation_pop(&vector_start_x);
    Operation_pop(&rect_end_y);
    Operation_pop(&rect_end_x);
    Operation_pop(&rect_start_y);
    Operation_pop(&rect_start_x);
    Operation_stack_size--;

    Hide_cursor();
    // Maintenant on efface tout le bazar temporaire : rectangle et ligne XOR
    Draw_xor_rect(rect_start_x, rect_start_y, rect_end_x, rect_end_y);
    Hide_line_preview(vector_start_x,vector_start_y,vector_end_x,vector_end_y);

    // Et enfin on trace le rectangle avec le dégradé dedans !
    if (vector_end_x==vector_start_x && vector_end_y==vector_start_y)
    {
        // Vecteur nul > pas de rectangle tracé
    }
    else
    {
        Draw_grad_rectangle(rect_start_x,rect_start_y,rect_end_x,rect_end_y,vector_start_x,vector_start_y,vector_end_x,vector_end_y);
    }
    Display_cursor();
    End_of_modification();
    Wait_end_of_click();

    if ((Config.Coords_rel) && (Menu_is_visible))
    {
        Print_in_menu("X:       Y:             ",0);
        Print_coordinates();
    }
}
/////////////////////////////////////////////////// OPERATION_CENTERED_LINES


void Centered_lines_12_0(void)
    // Opération   : OPERATION_CENTERED_LINES
    // Click Souris: 1 ou 2
    // Taille_Pile : 0
    //
    //  Souris effacée: Oui
{
    if (Rightclick_colorpick(0))
      return;

    Init_start_operation();
    Backup();
    Shade_table=(Mouse_K==LEFT_SIDE)?Shade_table_left:Shade_table_right;

    if ((Config.Coords_rel) && (Menu_is_visible))
        Print_in_menu("X:\xb1   0   Y:\xb1   0",0);

    Operation_push(Mouse_K);
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
}


void Centered_lines_12_3(void)
    // Opération   : OPERATION_CENTERED_LINES
    // Click Souris: 1 ou 2
    // Taille_Pile : 3
    //
    // Souris effacée: Non
{
    short start_x;
    short start_y;

    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
}


void Centered_lines_0_3(void)
    // Opération   : OPERATION_CENTERED_LINES
    // Click Souris: 0
    // Taille_Pile : 3
    //
    // Souris effacée: Oui
{
    short start_x;
    short start_y;
    short Button;
    short color;

    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Operation_pop(&Button);

    color=(Button==LEFT_SIDE)?Fore_color:Back_color;

    Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);
    Paintbrush_shape_before_operation=Paintbrush_shape;
    Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;

    Operation_push(Button);
    Operation_push(Paintbrush_X); // Nouveau début X
    Operation_push(Paintbrush_Y); // Nouveau début Y
    Operation_push(Paintbrush_X); // Nouvelle dernière fin X
    Operation_push(Paintbrush_Y); // Nouvelle dernière fin Y
    Operation_push(Paintbrush_X); // Nouvelle dernière position X
    Operation_push(Paintbrush_Y); // Nouvelle dernière position Y
}


void Centered_lines_12_7(void)
    // Opération   : OPERATION_CENTERED_LINES
    // Click Souris: 1 ou 2
    // Taille_Pile : 7
    //
    // Souris effacée: Non
{
    short Button;
    short start_x;
    short start_y;
    short end_x;
    short end_y;
    short last_x;
    short last_y;
    short color;

    Operation_pop(&last_y);
    Operation_pop(&last_x);
    Operation_pop(&end_y);
    Operation_pop(&end_x);
    Operation_pop(&start_y);
    Operation_pop(&start_x);
    Operation_pop(&Button);

    if (Mouse_K==Button)
    {
        if ( (end_x!=Paintbrush_X) || (end_y!=Paintbrush_Y) ||
                (last_x!=Paintbrush_X) || (last_y!=Paintbrush_Y) )
        {
            Hide_cursor();

            color=(Button==LEFT_SIDE)?Fore_color:Back_color;

            Paintbrush_shape=Paintbrush_shape_before_operation;

            Pixel_figure_preview_auto  (start_x,start_y);
            Hide_line_preview (start_x,start_y,last_x,last_y);

            Smear_start=1;
            Draw_paintbrush            (start_x,start_y,color);
            Draw_line_permanent(start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

            Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
            Pixel_figure_preview(Paintbrush_X,Paintbrush_Y,color);
            Draw_line_preview(start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

            Display_cursor();
        }

        Operation_push(Button);
        Operation_push(start_x);
        Operation_push(start_y);
        Operation_push(Paintbrush_X);
        Operation_push(Paintbrush_Y);
        Operation_push(Paintbrush_X);
        Operation_push(Paintbrush_Y);
    }
    else
    {
        Hide_cursor();

        Paintbrush_shape=Paintbrush_shape_before_operation;

        Pixel_figure_preview_auto  (start_x,start_y);
        Hide_line_preview (start_x,start_y,last_x,last_y);

        if ( (Config.Coords_rel) && (Menu_is_visible) )
        {
            Print_in_menu("X:       Y:             ",0);
            Print_coordinates();
        }

        Display_cursor();
        End_of_modification();
        Wait_end_of_click();
    }
}


void Centered_lines_0_7(void)
    // Opération   : OPERATION_CENTERED_LINES
    // Click Souris: 0
    // Taille_Pile : 7
    //
    // Souris effacée: Non
{
    short Button;
    short start_x;
    short start_y;
    short end_x;
    short end_y;
    short last_x;
    short last_y;
    short color;

    Operation_pop(&last_y);
    Operation_pop(&last_x);
    Operation_pop(&end_y);
    Operation_pop(&end_x);

    if ((Paintbrush_X!=last_x) || (Paintbrush_Y!=last_y))
    {
        Hide_cursor();
        Operation_pop(&start_y);
        Operation_pop(&start_x);
        Operation_pop(&Button);

        color=(Button==LEFT_SIDE)?Fore_color:Back_color;

        Display_coords_rel_or_abs(start_x,start_y);

        Hide_line_preview(start_x,start_y,last_x,last_y);

        Pixel_figure_preview(start_x,start_y,color);
        Draw_line_preview(start_x,start_y,Paintbrush_X,Paintbrush_Y,color);

        Operation_push(Button);
        Operation_push(start_x);
        Operation_push(start_y);
        Display_cursor();
    }

    Operation_push(end_x);
    Operation_push(end_y);
    Operation_push(Paintbrush_X);
    Operation_push(Paintbrush_Y);
}

///////////////////////////////////////////////////////////// OPERATION_PAN_VIEW

void Pan_view_0_0(void)
// Opération   : OPERATION_PAN_VIEW
// Click Souris: 0
// Taille_Pile : 0
//
//  Souris effacée: Non
{
  if (Pan_shortcut_pressed)
  {
    Print_coordinates();
  }
  else
  {
    // End of operation, return to previous
    Hide_cursor();
    Start_operation_stack(Operation_before_pan);
    Display_cursor();
  }
}

void Pan_view_12_0(void)
// Opération   : OPERATION_PAN_VIEW
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Non

//  First time the user clicks
{
  Init_start_operation();

  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Pan_view_12_2(void)
// Opération   : OPERATION_PAN_VIEW
// Click Souris: 1 ou 2
// Taille_Pile : 2
//
//  Souris effacée: Non

//  While dragging view
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if (Paintbrush_X!=start_x || Paintbrush_Y!=start_y)
  {
    // User moved
    if (Main.magnifier_mode)
      Scroll_magnifier(start_x-Paintbrush_X,start_y-Paintbrush_Y);
    else
      Scroll_screen(start_x-Paintbrush_X,start_y-Paintbrush_Y);
  }
  // The "scroll" functions have actualized the Paintbrush_X and Y
  if (Paintbrush_X!=start_x || Paintbrush_Y!=start_y)
  {
    Print_coordinates();
  }
  Operation_push(Paintbrush_X);
  Operation_push(Paintbrush_Y);
}

void Pan_view_0_2(void)
// Opération   : OPERATION_PAN_VIEW
// Click Souris: 0
// Taille_Pile : 2
//
//  Souris effacée: Non

//  When releasing after dragging
{
  short start_x;
  short start_y;

  Operation_pop(&start_y);
  Operation_pop(&start_x);

  if (!Pan_shortcut_pressed)
  {
    // End of operation, return to previous
    Hide_cursor();
    Start_operation_stack(Operation_before_pan);
    Display_cursor();
  }
}
