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
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "struct.h"
#include "transform.h"
#include "engine.h"
#include "screen.h"
#include "windows.h"
#include "input.h"
#include "help.h"
#include "misc.h" // Num2str
#include "readline.h"
#include "buttons.h" // Message_out_of_memory()
#include "pages.h" // Backup_with_new_dimensions()
#include "tiles.h"


/// Reduces a fraction A/B to its smallest representation. ie (40,60) becomes (2/3)
void Factorize(short *a, short *b)
{
  // Method: brute-force.
  short factor;

  factor=2;
  while (factor<=*a && factor<=*b)
  {
    if (((*a % factor) == 0) && ((*b % factor) == 0))
    {
      // common factor is found
      *a/=factor;
      *b/=factor;
      // restart
      factor=2;
    }
    else
      factor++;
  }
}

/// Multiplies original_size by new_ratio/old_ratio, but keeps result in 1-9999 range.
short Compute_dimension(short original_size, short new_ratio, short old_ratio)
{
  long amount;
  amount = (long)original_size*new_ratio/old_ratio;
  if (amount>9999)
    return 9999;
  else if (amount<1)
    return 1;
  else
    return amount;
}

void Button_Transform_menu(int btn)
{
  enum RESIZE_UNIT {
    UNIT_PIXELS  = 1,
    UNIT_PERCENT = 2,
    UNIT_RATIO   = 3
  };

  char buffer[5];
  short clicked_button;
  const char * unit_label[] = {
    "",
    "Pixels ",
    "Percent",
    "Ratio  "};
  short last_unit_index = -1;
  short old_ratio_width;
  short old_ratio_height;
  short new_ratio_width;
  short new_ratio_height;
  short new_width=Main.image_width;
  short new_height=Main.image_height;
  byte need_display_size = 0;

  // Persistent data
  static short unit_index = 1; // 1= Pixels, 2= Percent, 3=Ratio
  static short ratio_is_locked = 1; // True if X and Y resize should go together

  T_Dropdown_button * unit_button;
  T_Special_button * input_button[4];
  short *input_value[4];

  // Set initial ratio
  if (unit_index == UNIT_PERCENT)
    new_ratio_width=old_ratio_width=new_ratio_height=old_ratio_height=100;
  else
    new_ratio_width=old_ratio_width=new_ratio_height=old_ratio_height=1;

  Open_window(215,165,"Picture transform");

  Window_display_frame( 5, 15,205,91);
  Window_display_frame( 5,110, 55,49);
  Window_display_frame(64,110, 85,49);

  Window_set_normal_button(154,140, 54,14,"Cancel",0,1,KEY_ESC); // 1

  Print_in_window(  9,114,"Mirror",MC_Dark,MC_Light);
  Window_set_normal_button( 17,125, 27,14,"X\035" ,1,1,KEY_x);  // 2
  Window_set_normal_button( 17,140, 27,14,"Y\022" ,1,1,KEY_y);  // 3

  Print_in_window( 84,114,"Rotate",MC_Dark,MC_Light);
  Window_set_normal_button( 69,125, 37,14,"-90\xb0"  ,0,1,KEY_NONE); // 4
  Window_set_normal_button(107,125, 37,14,"+90\xb0"  ,0,1,KEY_NONE); // 5
  Window_set_normal_button( 69,140, 75,14,"180\xb0"  ,0,1,KEY_NONE); // 6

  Print_in_window( 87, 19,"Resize",MC_Dark,MC_Light);
  Window_set_normal_button( 80, 86, 60,14,"RESIZE",1,1,KEY_r);    // 7
  Print_in_window( 51, 34,"New",MC_Dark,MC_Light);
  Print_in_window( 96, 34,"Old",MC_Dark,MC_Light);
  Print_in_window_underscore( 30, 44,"W:",MC_Dark,MC_Light,1);
  Print_in_window_underscore( 30, 59,"H:",MC_Dark,MC_Light,1);
  Print_in_window( 80, 44,":",MC_Dark,MC_Light);
  Print_in_window( 80, 59,":",MC_Dark,MC_Light);
  Print_in_window_underscore( 44, 75,"Lock proportions",MC_Dark,MC_Light,1);

  Window_set_normal_button( 28, 72, 13,13,ratio_is_locked?"X":" ",0,1,KEY_l);// 8
  unit_button = Window_set_dropdown_button(128,50,69,11,69,unit_label[unit_index],1,0,1,LEFT_SIDE|RIGHT_SIDE,0);// 9
  Window_dropdown_add_item(unit_button,UNIT_PIXELS,unit_label[UNIT_PIXELS]);
  Window_dropdown_add_item(unit_button,UNIT_PERCENT,unit_label[UNIT_PERCENT]);
  Window_dropdown_add_item(unit_button,UNIT_RATIO,unit_label[UNIT_RATIO]);

  input_button[0] = Window_set_input_button_s(45,43,4,KEY_w); // 10
  input_button[1] = Window_set_input_button(89,43,4); // 11
  input_button[2] = Window_set_input_button_s(45,58,4,KEY_h); // 12
  input_button[3] = Window_set_input_button(89,58,4); // 13

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    // Display the coordinates with the right unit
    if (last_unit_index != unit_index)
    {
      switch(unit_index)
      {
        case UNIT_PIXELS:
        default:
          input_value[0]=&new_width;
          input_value[1]=&Main.image_width; // Don't worry, it's read-only
          input_value[2]=&new_height;
          input_value[3]=&Main.image_height; // Don't worry, it's read-only
        break;
        case UNIT_PERCENT:
        case UNIT_RATIO:
          input_value[0]=&new_ratio_width;
          input_value[1]=&old_ratio_width;
          input_value[2]=&new_ratio_height;
          input_value[3]=&old_ratio_height;
        break;
      }
      need_display_size=1;
      last_unit_index=unit_index;
    }
    if (need_display_size)
    {
      short i;
      Hide_cursor();
      for (i=0;i<4;i++)
      {
        // "Old" values are not editable, unless the unit is "ratio"
        byte color = ((unit_index!=UNIT_RATIO) && (i==1 || i==3)) ? MC_Dark : MC_Black;
        Num2str(*(input_value[i]),buffer,4);
        Print_in_window_limited(input_button[i]->Pos_X+2,input_button[i]->Pos_Y+2,buffer,input_button[i]->Width/8,color,MC_Light);
      }
      Display_cursor();
      need_display_size=0;
    }

    clicked_button=Window_clicked_button();

    // Contextual help
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      Key=0;
      Window_help(btn, "PICTURE TRANSFORM");
    }
    else if (Is_shortcut(Key,0x200+BUTTON_ADJUST))
      clicked_button=1;

    else switch(clicked_button)
    {
      case 9: // Unit
        switch(Window_attribute2)
        {
          case UNIT_PIXELS:
           // Do nothing, pixel size was already computed.
          break;
          case UNIT_PERCENT:
            if (unit_index == UNIT_RATIO)
            {
              // Approximate from current ratio
              new_ratio_width = Compute_dimension(new_ratio_width,100,old_ratio_width);
              new_ratio_height = Compute_dimension(new_ratio_height,100,old_ratio_height);
              old_ratio_width = 100;
              old_ratio_height = 100;
              // Update pixel dimensions, to match percentage exactly
              new_width=Compute_dimension(Main.image_width, new_ratio_width, old_ratio_width);
              new_height=Compute_dimension(Main.image_height, new_ratio_height, old_ratio_height);
            }
            else // unit_index == UNIT_PIXELS
            {
              // Approximate from current pixel size
              new_ratio_width = new_width*100/Main.image_width;
              new_ratio_height = new_height*100/Main.image_height;
              old_ratio_width = 100;
              old_ratio_height = 100;
            }
          break;
          case UNIT_RATIO:
            if (unit_index == UNIT_PERCENT)
            {
              // Compute simplest ratio from current %
              Factorize(&new_ratio_width, &old_ratio_width);
              Factorize(&new_ratio_height, &old_ratio_height);
            }
            else // unit_index == UNIT_PIXELS
            {
              // Compute simplest ratio from current pixel size
              new_ratio_width = new_width;
              new_ratio_height = new_height;
              old_ratio_width = Main.image_width;
              old_ratio_height = Main.image_height;
              Factorize(&new_ratio_width, &old_ratio_width);
              Factorize(&new_ratio_height, &old_ratio_height);
            }
          break;
        }

        unit_index = Window_attribute2;
        break;

      case 8: // Lock proportions
        ratio_is_locked = ! ratio_is_locked;
        Hide_cursor();
        Print_in_window(31,75,(ratio_is_locked)?"X":" ",MC_Black,MC_Light);
        Display_cursor();
        break;

      case 11: // input old width
      case 13: // input old height
        // "Old" values are not editable, unless the unit is "ratio"
        if (unit_index!=UNIT_RATIO)
          break;
#if defined(__GNUC__) && (__GNUC__ >= 7)
          __attribute__ ((fallthrough));
#endif
      case 10: // input new width
      case 12: // input new height
        Num2str(*( input_value[clicked_button-10]),buffer,4);
        Hide_cursor();
        if (Readline(input_button[clicked_button-10]->Pos_X+2,
          input_button[clicked_button-10]->Pos_Y+2,
          buffer,
          4,
          INPUT_TYPE_INTEGER))
        {
          // Accept entered value
          *(input_value[clicked_button-10])=atoi(buffer);
          // 0 is not acceptable size
          if (*(input_value[clicked_button-10])==0)
          {
            *(input_value[clicked_button-10])=1;
          }
          // Adapt the other coordinate if X and Y are locked
          if (ratio_is_locked)
          {
            if (clicked_button == 10 || clicked_button == 11 )
            {
              // Get Y value because X changed
              if (unit_index == UNIT_PIXELS)
              {
                new_height=Compute_dimension(Main.image_height, new_width, Main.image_width);
              }
              else
              {
                // Copy the whole ratio
                new_ratio_height=new_ratio_width;
                old_ratio_height=old_ratio_width;
              }
            }
            else //  (clicked_button == 12 || clicked_button == 13)
            {
              // Get X value because Y changed
              if (unit_index == UNIT_PIXELS)
              {
                new_width=Compute_dimension(Main.image_width, new_height, Main.image_height);
              }
              else
              {
                // Copy the whole ratio
                new_ratio_width=new_ratio_height;
                old_ratio_width=old_ratio_height;
              }
            }
          }

          // Re-compute ratio from size in pixels
          if (unit_index == UNIT_PIXELS)
          {
            //new_width=(long)Main.image_width*new_ratio_width/old_ratio_width;
            //new_height=(long)Main.image_height*new_ratio_height/old_ratio_height;
          }
          else // Re-compute size in pixels from ratio
          {
            new_width=Compute_dimension(Main.image_width,new_ratio_width,old_ratio_width);
            new_height=Compute_dimension(Main.image_height,new_ratio_height,old_ratio_height);
          }
          need_display_size=1;
        }
        Display_cursor();
      break;
    }
  }
  while ((clicked_button<=0 || clicked_button>=8) && !Quit_is_required);

  Close_window();

  // The Scroll operation uses the same button as transformation menu.
  if (Current_operation != OPERATION_SCROLL)
    Unselect_button(btn);

  if (clicked_button != 1 && !Quit_is_required) // 1 is Cancel
  {
    short old_width;
    short old_height;

    // Determine new image dimensions
    switch (clicked_button)
    {
      case  7 : // Resize
        // Keep new_width and new_height as entered.
        break;
      case  2 : // Flip X
      case  3 : // Flip Y
      case  6 : // 180° Rotation
        new_width=Main.image_width;
        new_height=Main.image_height;
        break;

      case  4 : // -90° Rotation
      case  5 : // +90° Rotation

        new_width=Main.image_height;
        new_height=Main.image_width;
        break;
    }
    // Memorize the current dimensions
    old_width=Main.image_width;
    old_height=Main.image_height;

    Upload_infos_page(&Main);
    // Allocate a new page
    if (Backup_with_new_dimensions(new_width,new_height))
    {
      // The new image is allocated, the new dimensions are already updated.

      Main.image_is_modified=1;

      // Process the transformation:
      switch(clicked_button)
      {
        int i;

        case  2 : // Flip X
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            memcpy(Main.backups->Pages->Image[i].Pixels,Main.backups->Pages->Next->Image[i].Pixels,Main.image_width*Main.image_height);
            Flip_X_lowlevel(Main.backups->Pages->Image[i].Pixels, Main.image_width, Main.image_height);
          }
          break;
        case  3 : // Flip Y
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            memcpy(Main.backups->Pages->Image[i].Pixels,Main.backups->Pages->Next->Image[i].Pixels,Main.image_width*Main.image_height);
            Flip_Y_lowlevel(Main.backups->Pages->Image[i].Pixels, Main.image_width, Main.image_height);
          }
          break;
        case  4 : // -90° Rotation
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            Rotate_270_deg_lowlevel(Main.backups->Pages->Next->Image[i].Pixels, Main.backups->Pages->Image[i].Pixels, old_width, old_height);
          }
          break;
        case  5 : // +90° Rotation
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            Rotate_90_deg_lowlevel(Main.backups->Pages->Next->Image[i].Pixels, Main.backups->Pages->Image[i].Pixels, old_width, old_height);
          }
          break;
        case  6 : // 180° Rotation
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            memcpy(Main.backups->Pages->Image[i].Pixels,Main.backups->Pages->Next->Image[i].Pixels,Main.image_width*Main.image_height);
            Rotate_180_deg_lowlevel(Main.backups->Pages->Image[i].Pixels, Main.image_width, Main.image_height);
          }
          break;
        case  7 : // Resize
          for (i=0; i<Main.backups->Pages->Nb_layers; i++)
          {
            Rescale(Main.backups->Pages->Next->Image[i].Pixels, old_width, old_height, Main.backups->Pages->Image[i].Pixels, Main.image_width, Main.image_height, 0, 0);
          }
          break;
      }
      /*
      for (i=0; i<NB_LAYERS; i++)
      {
        Copy_part_of_image_to_another(
          Main.backups->Pages->Next->Image[i].Pixels,0,0,Min(old_width,Main.image_width),
          Min(old_height,Main.image_height),old_width,
          Main.backups->Pages->Image[i].Pixels,0,0,Main.image_width);
      }
      */
      Redraw_layered_image();
      Display_all_screen();
      End_of_modification();
      Tilemap_update();
    }
    else
    {
      Display_cursor();
      Message_out_of_memory();
      Hide_cursor();
    }
  }
  Display_cursor();
}
