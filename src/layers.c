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
#include "sdlscreen.h"
#include "input.h"
#include "help.h"
#include "misc.h"
#include "readline.h"
#include "graph.h"

void Layer_activate(int layer, short side)
{
  word old_layers;

  if (layer >= Main_backups->Pages->Nb_layers)
    return;
  
  // Keep a copy of which layers were visible
  old_layers = Main_layers_visible;
  
  if (Main_backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    if (side == RIGHT_SIDE)
    {
      // Right-click on current layer
      if (Main_current_layer == layer)
      {
        if (Main_layers_visible == (dword)(1<<layer))
        {
          // Set all layers visible
          Main_layers_visible = 0xFFFFFFFF;
        }
        else
        {
          // Set only this one visible
          Main_layers_visible = 1<<layer;
        }
      }
      else
      {
        // Right-click on an other layer : toggle its visibility
        Main_layers_visible ^= 1<<layer;
      }
    }
    else
    {
      // Left-click on any layer
      Main_current_layer = layer;
      Main_layers_visible |= 1<<layer;
    }
  }
  else
  {
    // Only allow one visible at a time
    if (side == LEFT_SIDE)
    {
      Main_current_layer = layer;
      Main_layers_visible = 1<<layer;
      
      Update_screen_targets();
    }
  }

  Hide_cursor();
  if (Main_layers_visible != old_layers)
    Redraw_layered_image();
  else
    Update_depth_buffer(); // Only need the depth buffer
  //Download_infos_page_main(Main_backups->Pages);
  //Update_FX_feedback(Config.FX_Feedback);
  Update_pixel_renderer();
  Display_all_screen();
  Display_layerbar();
  Display_cursor();
}

void Button_Layer_add(void)
{
  int max[] = {MAX_NB_LAYERS, MAX_NB_FRAMES, 5};
  
  Hide_cursor();

  if (Main_backups->Pages->Nb_layers < max[Main_backups->Pages->Image_mode])
  {
    // Backup with unchanged layers
    Backup_layers(0);
    if (!Add_layer(Main_backups,Main_current_layer+1))
    {
      if (Main_backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
      {
        // Make a copy of current image, so the display is unchanged
        memcpy(
          Main_backups->Pages->Image[Main_current_layer].Pixels,
          Main_backups->Pages->Image[Main_current_layer-1].Pixels,
          Main_backups->Pages->Width*Main_backups->Pages->Height);
      }
      else
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

  Unselect_button(BUTTON_LAYER_ADD);
  Unselect_button(BUTTON_ANIM_ADD_FRAME);
  Display_cursor();
}

void Button_Layer_remove(void)
{
  Hide_cursor();

  if (Main_backups->Pages->Nb_layers > 1)
  {
    // Backup with unchanged layers
    Backup_layers(0);
    if (!Delete_layer(Main_backups,Main_current_layer))
    {
      Update_screen_targets();
      Redraw_layered_image();
      
      Display_all_screen();
      Display_layerbar();
      End_of_modification();
    }
  }
  Unselect_button(BUTTON_LAYER_REMOVE);
  Unselect_button(BUTTON_ANIM_REMOVE_FRAME);
  Display_cursor();
}

short Layer_under_mouse(void)
{
  short layer;
  // Determine which button is clicked according to mouse position
  layer = (Mouse_X/Menu_factor_X - Menu_bars[MENUBAR_LAYERS].Skin_width)
    / Layer_button_width;

  // Safety required because the mouse cursor can have slided outside button.
  if (layer < 0)
    layer=0;
  else if (layer > Main_backups->Pages->Nb_layers-1)
    layer=Main_backups->Pages->Nb_layers-1;

  return layer;
}

void Button_Layer_select(void)
{
  short layer = Layer_under_mouse();
  Layer_activate(layer, LEFT_SIDE);
  Mouse_K=0;
}

void Button_Layer_toggle(void)
{
  int layer;
  // Determine which button is clicked according to mouse position
  layer = (Mouse_X/Menu_factor_X - Menu_bars[MENUBAR_LAYERS].Skin_width)
    / Layer_button_width;

  // Safety required because the mouse cursor can have slided outside button.
  if (layer < 0)
    layer=0;
  else if (layer > Main_backups->Pages->Nb_layers-1)
    layer=Main_backups->Pages->Nb_layers-1;
  
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


void Button_Layer_menu(void)
{
  byte transparent_color = Main_backups->Pages->Transparent_color;
  byte transparent_background = Main_backups->Pages->Background_transparent;
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
  
  Window_set_normal_button( 7, 78, 51,14,"OK" , 0,1,SDLK_RETURN); // 3
  Window_set_normal_button(63, 78, 51,14,"Cancel", 0,1,KEY_ESC); // 4
  
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();
  
  do
  {
    
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_LAYER_MENU, NULL);
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
    if (Main_backups->Pages->Transparent_color != transparent_color ||
        Main_backups->Pages->Background_transparent != transparent_background)
    {
      Backup_layers(-1);
      Main_backups->Pages->Transparent_color = transparent_color;
      Main_backups->Pages->Background_transparent = transparent_background;
      Redraw_layered_image();
      Display_all_screen();
      End_of_modification();
    }
  }
  Unselect_button(BUTTON_LAYER_MENU);
  Unselect_button(BUTTON_LAYER_MENU2);
  Display_cursor();
}

void Button_Layer_set_transparent(void)
{
  Hide_cursor();

  if (Main_backups->Pages->Transparent_color != Back_color)
  {
    Backup_layers(-1);
    Main_backups->Pages->Transparent_color = Back_color;
    
    Redraw_layered_image();
    Display_all_screen();
    End_of_modification();
  }

  Unselect_button(BUTTON_LAYER_COLOR);
  Display_cursor();
}

void Button_Layer_get_transparent(void)
{
  Hide_cursor();

  if (Main_backups->Pages->Transparent_color != Back_color)
  {
    Set_back_color(Main_backups->Pages->Transparent_color);
  }

  Unselect_button(BUTTON_LAYER_COLOR);
  Display_cursor();
}

void Button_Layer_merge(void)
{
  Hide_cursor();

  if (Main_current_layer>0)
  {
    // Backup layer below the current
    Backup_layers(1<<(Main_current_layer-1));
  
    Merge_layer();
    
    Update_screen_targets();
    Redraw_layered_image();
    Display_all_screen();
    Display_layerbar();
    End_of_modification();
  }
  
  Unselect_button(BUTTON_LAYER_MERGE);
  Display_cursor();
}

void Button_Layer_up(void)
{
  Hide_cursor();

  if (Main_current_layer < (Main_backups->Pages->Nb_layers-1))
  {
    T_Image tmp;
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
    Display_all_screen();
    Display_layerbar();
    End_of_modification();
  }
  
  Unselect_button(BUTTON_LAYER_UP);
  Unselect_button(BUTTON_ANIM_UP_FRAME);
  Display_cursor();
}

void Button_Layer_down(void)
{
  Hide_cursor();
  
  if (Main_current_layer > 0)
  {
    T_Image tmp;
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
    Display_layerbar();
    Display_all_screen();
    End_of_modification();
  }
  
  Unselect_button(BUTTON_LAYER_DOWN);
  Unselect_button(BUTTON_ANIM_DOWN_FRAME);
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
void Button_Anim_time(void)
{
  short clicked_button;
  int mode=0;
  int frame;
  char buffer[6+1];
  T_Special_button * input_duration_button;
  int duration=Main_backups->Pages->Image[Main_current_layer].Duration;
  
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

  Window_set_normal_button( 7, 92, 51,14,"OK" , 0,1,SDLK_RETURN); // 5
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
    Backup_layers(0);
    switch(mode)
    {
      case 0:
        if (duration<0)
          duration=0;
        else if (duration>655350)
          duration=655350;
        Main_backups->Pages->Image[Main_current_layer].Duration = duration;
        break;
      case 1:
        if (duration<0)
          duration=0;
        else if (duration>655350)
          duration=655350;
        for (frame=0; frame<Main_backups->Pages->Nb_layers; frame++)
        {
          Main_backups->Pages->Image[frame].Duration = duration;
        }
        break;
      case 2:
        for (frame=0; frame<Main_backups->Pages->Nb_layers; frame++)
        {
          int cur_duration = Main_backups->Pages->Image[frame].Duration+duration;
          if (cur_duration<0)
            cur_duration=0;
          else if (cur_duration>655350)
            cur_duration=655350;
          Main_backups->Pages->Image[frame].Duration = cur_duration;
        }
        break;
      break;
    }
    End_of_modification();
  }

  Unselect_button(BUTTON_ANIM_TIME);
  Display_cursor();
}

void Button_Anim_first_frame(void)
{
  if (Main_current_layer>0)
    Layer_activate(0,LEFT_SIDE);

  Hide_cursor();
  Unselect_button(BUTTON_ANIM_FIRST_FRAME);
  Display_cursor();
}

void Button_Anim_prev_frame(void)
{
  if (Main_backups->Pages->Nb_layers>1)
  {
    if (Main_current_layer==0)
      Layer_activate(Main_backups->Pages->Nb_layers-1,LEFT_SIDE);
    else
      Layer_activate(Main_current_layer-1,LEFT_SIDE);
  }
  Hide_cursor();
  Unselect_button(BUTTON_ANIM_PREV_FRAME);
  Display_cursor();
}

void Button_Anim_next_frame(void)
{
  if (Main_backups->Pages->Nb_layers>1)
  {
    if (Main_current_layer==Main_backups->Pages->Nb_layers-1)
      Layer_activate(0,LEFT_SIDE);
    else
      Layer_activate(Main_current_layer+1,LEFT_SIDE);
  }

  Hide_cursor();
  Unselect_button(BUTTON_ANIM_NEXT_FRAME);
  Display_cursor();
}

void Button_Anim_last_frame(void)
{
  if (Main_current_layer < (Main_backups->Pages->Nb_layers-1))
    Layer_activate((Main_backups->Pages->Nb_layers-1),LEFT_SIDE);
    
  Hide_cursor();
  Unselect_button(BUTTON_ANIM_LAST_FRAME);
  Display_cursor();
}

void Button_Anim_continuous_next(void)
{
  Uint32 time_start;
  int time_in_current_frame=0;

  time_start = SDL_GetTicks();
  
  do
  {
    int target_frame;
    Uint32 time_now;
  
    Get_input(20);
    
    time_now=SDL_GetTicks();
    time_in_current_frame += time_now-time_start;
    time_start=time_now;
    target_frame = Main_current_layer;
    while (time_in_current_frame > Main_backups->Pages->Image[target_frame].Duration)
    {
      time_in_current_frame -= Interpret_delay(Main_backups->Pages->Image[target_frame].Duration);
      target_frame = (target_frame+1) % Main_backups->Pages->Nb_layers;
    }
    if (target_frame != Main_current_layer)
    {
      Layer_activate(target_frame,LEFT_SIDE);
    }
    
  } while (Mouse_K);

  Hide_cursor();
  Unselect_button(BUTTON_ANIM_NEXT_FRAME);
  Display_cursor();
}

void Button_Anim_continuous_prev(void)
{
  Uint32 time_start;
  int time_in_current_frame=0;

  time_start = SDL_GetTicks();
  
  do
  {
    int target_frame;
    Uint32 time_now;
  
    Get_input(20);
    
    time_now=SDL_GetTicks();
    time_in_current_frame += time_now-time_start;
    time_start=time_now;
    target_frame = Main_current_layer;
    while (time_in_current_frame > Main_backups->Pages->Image[target_frame].Duration)
    {
      time_in_current_frame -= Interpret_delay(Main_backups->Pages->Image[target_frame].Duration);
      target_frame = (target_frame+Main_backups->Pages->Nb_layers-1) % Main_backups->Pages->Nb_layers;
    }
    if (target_frame != Main_current_layer)
    {
      Layer_activate(target_frame,LEFT_SIDE);
    }
    
  } while (Mouse_K);

  Hide_cursor();
  Unselect_button(BUTTON_ANIM_PREV_FRAME);
  Display_cursor();
}

void Switch_layer_mode(enum IMAGE_MODES new_mode)
{
  if (new_mode == Main_backups->Pages->Image_mode)
    return;

  switch (new_mode)
  {
    case IMAGE_MODE_LAYERED:
      Update_buffers(Main_image_width, Main_image_height);
      // Refresh the buffer, special shortcut because only one layer exists.
      memset(Main_visible_image_depth_buffer.Image, 0, Main_image_width*Main_image_height);
      memcpy(Main_visible_image.Image,
        Main_backups->Pages->Image[0].Pixels,
        Main_image_width*Main_image_height);
      break;
    case IMAGE_MODE_ANIMATION:
      // nothing to do
      break;
    case IMAGE_MODE_MODE5:
      break;
  }
  
  Main_backups->Pages->Image_mode = new_mode;
  Update_pixel_renderer();      
}
