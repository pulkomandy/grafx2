/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2009 Yves Rizoud
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
#include "const.h"
#include "struct.h"
#include "global.h"
#include "windows.h"
#include "engine.h"
#include "pages.h"


void Button_Layer_add(void)
{
  // Backup with unchanged layers
  Backup_layers(0);
  if (!Add_layer(Main_backups,Main_current_layer+1))
  {
    Update_depth_buffer();
    Hide_cursor();
    Display_all_screen();
    Display_cursor();
    End_of_modification();
  }

  Hide_cursor();
  Unselect_button(BUTTON_LAYER_ADD);
  Display_cursor();
}

void Button_Layer_remove(void)
{
  // Backup with unchanged layers
  Backup_layers(0);
  if (!Delete_layer(Main_backups,Main_current_layer))
  {
    Update_screen_targets();
    Redraw_layered_image();
    Hide_cursor();
    Display_all_screen();
    Display_cursor();
    End_of_modification();
  }

  Hide_cursor();
  Unselect_button(BUTTON_LAYER_REMOVE);
  Display_cursor();
}
void Button_Layer_select(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_SELECT);
  Display_cursor();
}

/*
void Button_Layer_1(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_1);
  Display_cursor();
}

void Button_Layer_2(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_2);
  Display_cursor();
}

void Button_Layer_3(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_3);
  Display_cursor();
}

void Button_Layer_4(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_4);
  Display_cursor();
}

void Button_Layer_5(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_5);
  Display_cursor();
}

void Button_Layer_6(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_6);
  Display_cursor();
}

void Button_Layer_7(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_7);
  Display_cursor();
}

void Button_Layer_8(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_8);
  Display_cursor();
}
*/
void Button_Layer_menu(void)
{
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_MENU);
  Display_cursor();
}

void Button_Layer_color(void)
{
  if (Main_backups->Pages->Transparent_color != Back_color)
  {
    Backup_layers(-1);
    Main_backups->Pages->Transparent_color = Back_color;
    
    Redraw_layered_image();
    Hide_cursor();
    Display_all_screen();
    Display_cursor();
    End_of_modification();
  }

  Hide_cursor();
  Unselect_button(BUTTON_LAYER_COLOR);
  Display_cursor();
}

void Button_Layer_merge(void)
{
  if (Main_current_layer>0)
  {
    // Backup layer below the current
    Backup_layers(1<<(Main_current_layer-1));
  
    Merge_layer();
    
    Update_screen_targets();
    Redraw_layered_image();
    Hide_cursor();
    Display_all_screen();
    Display_cursor();
    End_of_modification();
  }
  
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_MERGE);
  Display_cursor();
}

void Button_Layer_up(void)
{
  if (Main_current_layer < (Main_backups->Pages->Nb_layers-1))
  {
  byte * tmp;
  dword layer_flags;
  
  // Backup with unchanged layers
  Backup_layers(0);
  
  // swap
  tmp = Main_backups->Pages->Image[Main_current_layer];
  Main_backups->Pages->Image[Main_current_layer] = Main_backups->Pages->Image[Main_current_layer+1];
  Main_backups->Pages->Image[Main_current_layer+1] = tmp;
  
  // Swap visibility indicators
  layer_flags = (Main_layers_visible >> Main_current_layer) & 3;
  // Only needed if they are different.
  if (layer_flags == 1 || layer_flags == 2)
  {
    // One is on, the other is off. Negating them will
    // perform the swap.
    Main_layers_visible ^= (3 << Main_current_layer);
  }
  Main_current_layer++;
  
  Update_screen_targets();
  Redraw_layered_image();
  Hide_cursor();
  Display_all_screen();
  Display_cursor();
  End_of_modification();
  }
  
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_UP);
  Display_cursor();
}

void Button_Layer_down(void)
{
  if (Main_current_layer > 0)
  {
    byte * tmp;
    dword layer_flags;
  
    // Backup with unchanged layers
    Backup_layers(0);
    
    // swap
    tmp = Main_backups->Pages->Image[Main_current_layer];
    Main_backups->Pages->Image[Main_current_layer] = Main_backups->Pages->Image[Main_current_layer-1];
    Main_backups->Pages->Image[Main_current_layer-1] = tmp;
    
    // Swap visibility indicators
    layer_flags = (Main_layers_visible >> (Main_current_layer-1)) & 3;
    // Only needed if they are different.
    if (layer_flags == 1 || layer_flags == 2)
    {
      // Only needed if they are different.
      // One is on, the other is off. Negating them will
      // perform the swap.
      Main_layers_visible ^= (3 << (Main_current_layer-1));
    }
    Main_current_layer--;
    Update_screen_targets();
    Redraw_layered_image();
    Hide_cursor();
    Display_all_screen();
    Display_cursor();
    End_of_modification();
  }
  Hide_cursor();
  Unselect_button(BUTTON_LAYER_DOWN);
  Display_cursor();
}
