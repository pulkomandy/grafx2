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
#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "gfx2log.h"
#include "windows.h"
#include "engine.h"
#include "pages.h"
#include "screen.h"
#include "input.h"
#include "help.h"
#include "misc.h"
#include "osdep.h"
#include "readline.h"
#include "graph.h"
#include "keycodes.h"
#include "layers.h"

void Layer_activate(int layer, short side)
{
  dword old_layers;

  if (layer >= Main.backups->Pages->Nb_layers)
    return;
  
  // Keep a copy of which layers were visible
  old_layers = Main.layers_visible;
  
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    if (side == RIGHT_SIDE)
    {
      // Right-click on current layer
      if (Main.current_layer == layer)
      {
        if (Main.layers_visible == (dword)(1<<layer))
        {
          // return to previous state (layers that were on before showing
          // only this one)
          Main.layers_visible = Main.layers_visible_backup;
        }
        else
        {
          // Set only this one visible
          Main.layers_visible_backup = Main.layers_visible;
          Main.layers_visible = 1<<layer;
        }
      }
      else
      {
        // Right-click on an other layer : toggle its visibility
        Main.layers_visible ^= 1<<layer;
      }
    }
    else
    {
      // Left-click on any layer
      Main.current_layer = layer;
      Main.layers_visible |= 1<<layer;
    }
  }
  else
  {
    // Only allow one visible at a time
    if (side == LEFT_SIDE)
    {
      Main.current_layer = layer;
      Main.layers_visible = 1<<layer;
      
      Update_screen_targets();
    }
  }

  Hide_cursor();
  if (Main.layers_visible != old_layers)
    Redraw_layered_image();
  else
    Update_depth_buffer(); // Only need the depth buffer
  //Download_infos_page_main(Main.backups->Pages);
  //Update_FX_feedback(Config.FX_Feedback);
  Update_pixel_renderer();
  Display_all_screen();
  Display_layerbar();
  Display_cursor();
}

int Layers_max(enum IMAGE_MODES mode)
{
  switch (mode)
  {
    case IMAGE_MODE_LAYERED:
      return MAX_NB_LAYERS;
      break;
    case IMAGE_MODE_ANIMATION:
      return MAX_NB_FRAMES;
      break;
    default:
      return 5;
  }
}

void Button_Layer_add(int btn)
{
  Hide_cursor();

  if (Main.backups->Pages->Nb_layers < Layers_max(Main.backups->Pages->Image_mode))
  {
    // Backup with unchanged layers
    Backup_layers(LAYER_NONE);
    if (!Add_layer(Main.backups,Main.current_layer+1))
    {
      Update_depth_buffer();
      // I just noticed this might be unneeded, since the new layer
      // is transparent, it shouldn't have any visible effect.
      Display_all_screen();
      Display_layerbar();
      End_of_modification();
    }
  }

  Unselect_button(btn);
  Display_cursor();
}


void Button_Layer_duplicate(int btn)
{
  Hide_cursor();

  if (Main.backups->Pages->Nb_layers < Layers_max(Main.backups->Pages->Image_mode))
  {
    // Backup with unchanged layers
    Backup_layers(LAYER_NONE);
    if (!Add_layer(Main.backups,Main.current_layer+1))
    {
      // Make a copy of current image
      memcpy(
        Main.backups->Pages->Image[Main.current_layer].Pixels,
        Main.backups->Pages->Image[Main.current_layer-1].Pixels,
        Main.backups->Pages->Width*Main.backups->Pages->Height);

      if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
      {
        Update_depth_buffer();
        // I just noticed this might be unneeded, since the new layer
        // is transparent, it shouldn't have any visible effect.
        Display_all_screen();
      }
      Display_layerbar();
      End_of_modification();
    }
  }

  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_remove(int btn)
{

  Hide_cursor();

  /// enforce 5 layers for IMAGE_MODE_MODE5 and IMAGE_MODE_RASTER
  /// and 2 layers for IMAGE_MODE_HGR and IMAGE_MODE_DHGR.
  if (!(  (Main.backups->Pages->Image_mode == IMAGE_MODE_MODE5
        || Main.backups->Pages->Image_mode == IMAGE_MODE_RASTER)
       && (Main.backups->Pages->Nb_layers <= 5))
   && !(  (Main.backups->Pages->Image_mode == IMAGE_MODE_HGR
        || Main.backups->Pages->Image_mode == IMAGE_MODE_DHGR)
       && (Main.backups->Pages->Nb_layers <= 2))
   &&  (Main.backups->Pages->Nb_layers > 1) )
  {
    // Backup with unchanged layers
    Backup_layers(LAYER_NONE);
    if (!Delete_layer(Main.backups,Main.current_layer))
    {
      Update_screen_targets();
      Redraw_layered_image();
      
      Display_all_screen();
      Display_layerbar();
      End_of_modification();
    }
  }
  Unselect_button(btn);
  Display_cursor();
}

static int Layer_under_mouse(void)
{
  int layer;
  // Determine which button is clicked according to mouse position
  layer = (Mouse_X/Menu_factor_X - Menu_bars[MENUBAR_LAYERS].Skin_width)
    / Layer_button_width;

  // Safety required because the mouse cursor can have slided outside button.
  if (layer < 0)
    layer=0;
  else if (layer > Main.backups->Pages->Nb_layers-1)
    layer=Main.backups->Pages->Nb_layers-1;

  return layer;
}

void Button_Layer_select(int btn)
{
  int layer = Layer_under_mouse();
  (void)btn;
  Layer_activate(layer, LEFT_SIDE);
  Mouse_K=0;
}

void Button_Layer_toggle(int btn)
{
  int layer = Layer_under_mouse();
  (void)btn;
  Layer_activate(layer, RIGHT_SIDE);
  Mouse_K=0;
}

static void Draw_transparent_color(byte color)
{
  char buf[4];
  Num2str(color, buf, 3);
  Print_in_window(63,39,buf,MC_Black,MC_Light);
  Window_rectangle(90,39,13,7,color);
}

static void Draw_transparent_background(byte background)
{
  Print_in_window(99,57,background?"X":" ",MC_Black,MC_Light);
}


void Button_Layer_menu(int btn)
{
  byte transparent_color = Main.backups->Pages->Transparent_color;
  byte transparent_background = Main.backups->Pages->Background_transparent;
  short clicked_button;
  byte color;
  byte click;

  Open_window(122,100,"Layers");

  Window_display_frame_in( 6, 21,110, 52);
  Print_in_window(14,18,"Transparency",MC_Dark,MC_Light);

  Print_in_window(11,38,"Color",MC_Black,MC_Light);
  Window_set_normal_button(54, 36, 56,13,"" , 0,1,KEY_NONE); // 1
  Draw_transparent_color(transparent_color);
  
  Print_in_window(11,57,"Background",MC_Black,MC_Light);
  Window_set_normal_button(95, 54, 15,13,"" , 0,1,KEY_NONE); // 2
  Draw_transparent_background(transparent_background);
  
  Window_set_normal_button( 7, 78, 51,14,"OK" , 0,1,KEY_RETURN); // 3
  Window_set_normal_button(63, 78, 51,14,"Cancel", 0,1,KEY_ESC); // 4
  
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();
  
  do
  {
    
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(btn, NULL);
    switch(clicked_button)
    {
      case 1: // color
        Get_color_behind_window(&color,&click);
        if (click && transparent_color!=color)
        {
          transparent_color=color;
          Hide_cursor();
          Draw_transparent_color(transparent_color);
          Display_cursor();
          Wait_end_of_click();
        }
        break;
        
      case 2: // background
        transparent_background = !transparent_background;
        Hide_cursor();
        Draw_transparent_background(transparent_background);
        Display_cursor();
        break;
    }
  }
  while (clicked_button<3);

  // On exit
  Hide_cursor();
  Close_window();
  if (clicked_button==3)
  {
    // Accept changes
    if (Main.backups->Pages->Transparent_color != transparent_color ||
        Main.backups->Pages->Background_transparent != transparent_background)
    {
      Backup_layers(LAYER_NONE);
      Main.backups->Pages->Transparent_color = transparent_color;
      Main.backups->Pages->Background_transparent = transparent_background;
      Redraw_layered_image();
      Display_all_screen();
      End_of_modification();
    }
  }
  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_set_transparent(int btn)
{
  Hide_cursor();

  if (Main.backups->Pages->Transparent_color != Back_color)
  {
    Backup_layers(LAYER_ALL);
    Main.backups->Pages->Transparent_color = Back_color;
    
    Redraw_layered_image();
    Display_all_screen();
    End_of_modification();
  }

  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_get_transparent(int btn)
{
  Hide_cursor();

  if (Main.backups->Pages->Transparent_color != Back_color)
  {
    Set_back_color(Main.backups->Pages->Transparent_color);
  }

  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_merge(int btn)
{
  Hide_cursor();

  if (Main.current_layer>0)
  {
    // Backup layer below the current
    Backup_layers(Main.current_layer-1);
  
    Merge_layer();
    
    Update_screen_targets();
    Redraw_layered_image();
    Display_all_screen();
    Display_layerbar();
    End_of_modification();
  }
  
  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_up(int btn)
{
  Hide_cursor();

  if (Main.current_layer < (Main.backups->Pages->Nb_layers-1))
  {
    T_Image tmp;
    dword layer_flags;
    
    // Backup with unchanged layers
    Backup_layers(LAYER_NONE);
    
    // swap
    tmp = Main.backups->Pages->Image[Main.current_layer];
    Main.backups->Pages->Image[Main.current_layer] = Main.backups->Pages->Image[Main.current_layer+1];
    Main.backups->Pages->Image[Main.current_layer+1] = tmp;
    
    // Swap visibility indicators
    layer_flags = (Main.layers_visible >> Main.current_layer) & 3;
    // Only needed if they are different.
    if (layer_flags == 1 || layer_flags == 2)
    {
      // One is on, the other is off. Negating them will
      // perform the swap.
      Main.layers_visible ^= (3 << Main.current_layer);
    }
    Main.current_layer++;
    
    Update_screen_targets();
    Redraw_layered_image();
    Display_all_screen();
    Display_layerbar();
    End_of_modification();
  }
  
  Unselect_button(btn);
  Display_cursor();
}

void Button_Layer_down(int btn)
{
  Hide_cursor();
  
  if (Main.current_layer > 0)
  {
    T_Image tmp;
    dword layer_flags;
  
    // Backup with unchanged layers
    Backup_layers(LAYER_NONE);
    
    // swap
    tmp = Main.backups->Pages->Image[Main.current_layer];
    Main.backups->Pages->Image[Main.current_layer] = Main.backups->Pages->Image[Main.current_layer-1];
    Main.backups->Pages->Image[Main.current_layer-1] = tmp;
    
    // Swap visibility indicators
    layer_flags = (Main.layers_visible >> (Main.current_layer-1)) & 3;
    // Only needed if they are different.
    if (layer_flags == 1 || layer_flags == 2)
    {
      // Only needed if they are different.
      // One is on, the other is off. Negating them will
      // perform the swap.
      Main.layers_visible ^= (3 << (Main.current_layer-1));
    }
    Main.current_layer--;
    Update_screen_targets();
    Redraw_layered_image();
    Display_layerbar();
    Display_all_screen();
    End_of_modification();
  }
  
  Unselect_button(btn);
  Display_cursor();
}

int Interpret_delay(int delay)
{
  // Firefox behavior
  if (delay>30)
    return delay;
  if (delay==0)
    return 100;
  return 30;
}
void Button_Anim_time(int btn)
{
  short clicked_button;
  int mode=0;
  int frame;
  char buffer[6+1];
  T_Special_button * input_duration_button;
  int duration=Main.backups->Pages->Image[Main.current_layer].Duration;
  
  Open_window(166,110,"Animation speed");

  Print_in_window(88,20,"ms",MC_Black,MC_Light);
  input_duration_button = Window_set_input_button(33,18,6); // 1

  Num2str(duration,buffer,6);
  Print_in_window_limited(input_duration_button->Pos_X+2,input_duration_button->Pos_Y+2,buffer,input_duration_button->Width/8,MC_Black,MC_Light);
  
  Print_in_window(24,37,"Set this frame",MC_Black,MC_Light);
  Window_set_normal_button(7, 34, 13,13,"X" , 0,1,KEY_NONE); // 2

  Print_in_window(24,55,"Set all frames",MC_Black,MC_Light);
  Window_set_normal_button(7, 52, 13,13,"" , 0,1,KEY_NONE); // 3

  Print_in_window(24,73,"Add to all frames",MC_Black,MC_Light);
  Window_set_normal_button(7, 70, 13,13,"" , 0,1,KEY_NONE); // 4

  Window_set_normal_button( 7, 92, 51,14,"OK" , 0,1,KEY_RETURN); // 5
  Window_set_normal_button(63, 92, 51,14,"Cancel", 0,1,KEY_ESC); // 6
  
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();
  
  do
  {
    
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_ANIM_TIME, NULL);
    switch(clicked_button)
    {
      case 1: // duration
        // safety
        if (duration <= -10000)
          sprintf(buffer,"-99999");
        else if (duration >= 1000000)
          sprintf(buffer,"999999");
        else
          sprintf(buffer,"%d", duration);
        Hide_cursor();
        if (Readline(input_duration_button->Pos_X+2, 
          input_duration_button->Pos_Y+2,
          buffer,
          6,
          INPUT_TYPE_DECIMAL))
        {
          duration=atoi(buffer);
        }
        Print_in_window_limited(input_duration_button->Pos_X+2,input_duration_button->Pos_Y+2,buffer,input_duration_button->Width/8,MC_Black,MC_Light);
        Display_cursor();
      break;
      case 2: // Radio: set 1
      case 3: // Radio: set all
      case 4: // Radio: add
        mode=clicked_button-2;
        Hide_cursor();
        Print_in_window(10,37,mode==0?"X":" ",MC_Black,MC_Light);
        Print_in_window(10,55,mode==1?"X":" ",MC_Black,MC_Light);
        Print_in_window(10,73,mode==2?"X":" ",MC_Black,MC_Light);
        Display_cursor();
        break;
    }
  }
  while (clicked_button<5);

  // On exit
  Hide_cursor();
  Close_window();
  if (clicked_button==5)
  {
    // Accept changes
    Backup_layers(LAYER_NONE);
    switch(mode)
    {
      case 0:
        if (duration<0)
          duration=0;
        else if (duration>655350)
          duration=655350;
        Main.backups->Pages->Image[Main.current_layer].Duration = duration;
        break;
      case 1:
        if (duration<0)
          duration=0;
        else if (duration>655350)
          duration=655350;
        for (frame=0; frame<Main.backups->Pages->Nb_layers; frame++)
        {
          Main.backups->Pages->Image[frame].Duration = duration;
        }
        break;
      case 2:
        for (frame=0; frame<Main.backups->Pages->Nb_layers; frame++)
        {
          int cur_duration = Main.backups->Pages->Image[frame].Duration+duration;
          if (cur_duration<0)
            cur_duration=0;
          else if (cur_duration>655350)
            cur_duration=655350;
          Main.backups->Pages->Image[frame].Duration = cur_duration;
        }
        break;
      break;
    }
    End_of_modification();
  }

  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_first_frame(int btn)
{
  if (Main.current_layer>0)
    Layer_activate(0,LEFT_SIDE);

  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_prev_frame(int btn)
{
  if (Main.backups->Pages->Nb_layers>1)
  {
    if (Main.current_layer==0)
      Layer_activate(Main.backups->Pages->Nb_layers-1,LEFT_SIDE);
    else
      Layer_activate(Main.current_layer-1,LEFT_SIDE);
  }
  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_next_frame(int btn)
{
  if (Main.backups->Pages->Nb_layers>1)
  {
    if (Main.current_layer==Main.backups->Pages->Nb_layers-1)
      Layer_activate(0,LEFT_SIDE);
    else
      Layer_activate(Main.current_layer+1,LEFT_SIDE);
  }

  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_last_frame(int btn)
{
  if (Main.current_layer < (Main.backups->Pages->Nb_layers-1))
    Layer_activate((Main.backups->Pages->Nb_layers-1),LEFT_SIDE);
    
  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_continuous_next(int btn)
{
  dword time_start;
  int time_in_current_frame=0;

  time_start = GFX2_GetTicks();
  
  do
  {
    int target_frame;
    dword time_now;
  
    Get_input(20);
    
    time_now=GFX2_GetTicks();
    time_in_current_frame += time_now-time_start;
    time_start=time_now;
    target_frame = Main.current_layer;
    while (time_in_current_frame > Main.backups->Pages->Image[target_frame].Duration)
    {
      time_in_current_frame -= Interpret_delay(Main.backups->Pages->Image[target_frame].Duration);
      target_frame = (target_frame+1) % Main.backups->Pages->Nb_layers;
    }
    if (target_frame != Main.current_layer)
    {
      Layer_activate(target_frame,LEFT_SIDE);
    }
    
  } while (Mouse_K);

  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Anim_continuous_prev(int btn)
{
  dword time_start;
  int time_in_current_frame=0;

  time_start = GFX2_GetTicks();
  
  do
  {
    int target_frame;
    dword time_now;
  
    Get_input(20);
    
    time_now=GFX2_GetTicks();
    time_in_current_frame += time_now-time_start;
    time_start=time_now;
    target_frame = Main.current_layer;
    while (time_in_current_frame > Main.backups->Pages->Image[target_frame].Duration)
    {
      time_in_current_frame -= Interpret_delay(Main.backups->Pages->Image[target_frame].Duration);
      target_frame = (target_frame+Main.backups->Pages->Nb_layers-1) % Main.backups->Pages->Nb_layers;
    }
    if (target_frame != Main.current_layer)
    {
      Layer_activate(target_frame,LEFT_SIDE);
    }
    
  } while (Mouse_K);

  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}
