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

////////////////////////////////////////////////////////////////////////////
///@file buttons_effects.c
/// Handles all the effects buttons and setup windows in the effects menu.
////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include "gfx2log.h"
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
#include "screen.h"
#include "struct.h"
#include "windows.h"
#include "tiles.h"
#include "oldies.h"
#include "palette.h"
#include "layers.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

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
  Window_set_normal_button( 7, 19,78,14,"Clear" ,1,1,KEY_c); // 2
  Window_set_normal_button(91, 19,78,14,"Invert",1,1,KEY_i); // 3
  if (can_cancel)
  {
    Window_set_normal_button(91,129,78,14,"OK"    ,0,1,KEY_RETURN); // 4
    Window_set_normal_button( 7,129,78,14,"Cancel",0,1,KEY_ESC);  // 5
    // On enregistre la table dans un backup au cas où on ferait Cancel
    memcpy(backup_table,table,256);
  }
  else
    Window_set_normal_button(49,129,78,14,"OK"    ,0,1,KEY_RETURN); // 4

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
      case KEY_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case KEY_COMMA :
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


/// Block Constraint check
///
/// This function count the number of colors in a block and reports errors
/// by marking the pixel in color 17 in layer 2.
/// @param mode
/// @return -1 if the mode is invalid
/// @return the number of constraint errors detected
static int Check_block_constraints(enum IMAGE_MODES mode)
{
  int x, y, x2, y2;
  int block_width = 8, block_height = 8, max_colors = 2;  // default values
  int error_count = 0;
  byte errcol = 17;
  byte pal_mask;

  switch (mode)
  {
    case IMAGE_MODE_TMS9918G2:
    case IMAGE_MODE_THOMSON:
      block_height = 1;
      break;
    case IMAGE_MODE_ZX:
    case IMAGE_MODE_C64HIRES:
      break;
    case IMAGE_MODE_C64MULTI:
      block_width = 4;
      max_colors = 3; // 3 + background color
      break;
    case IMAGE_MODE_GBC:
      max_colors = 4;
      errcol = 33;
      break;
    case IMAGE_MODE_MEGADRIVE:
      max_colors = 16;
      errcol = 65;
      break;
    default:
      return -1;  // unsupported mode
  }
  pal_mask = ~(max_colors - 1);

  for (y = 0; y <= Main.image_height - block_height; y += block_height)
  {
    for (x = 0; x <= Main.image_width - block_width; x += block_width)
    {
      int count = 0;
      byte c[16];   // colors already used in block

      for (y2 = 0; y2 < block_height; y2++)
      {
        for (x2 = 0; x2 < block_width; x2++)
        {
          int i;
          byte col = Read_pixel_from_layer(0, x+x2, y+y2);
          if (mode == IMAGE_MODE_GBC || mode == IMAGE_MODE_MEGADRIVE)
          {
            if (count == 0)
              c[count++] = col;
            else if ((col & pal_mask) != (c[0] & pal_mask))  // compare palettes
            {
              if (Main.backups->Pages->Nb_layers < 2)
                Add_layer(Main.backups, 1);
              Pixel_in_layer(1, x+x2, y+y2, errcol);
              error_count++;
            }
            continue;
          }
          if (col > 15) // forbidden color !
          {
            if (Main.backups->Pages->Nb_layers < 2)
              Add_layer(Main.backups, 1);
            Pixel_in_layer(1, x+x2, y+y2, errcol);
            error_count++;
            continue;
          }
          if (mode == IMAGE_MODE_C64MULTI && col == 0)  // Background color
            continue;
          // search color in already used ones
          for (i = 0; i < count; i++)
          {
            if (col == c[i])
              break;
            // ZX Spectrum : consider both blacks as the same color
            if (mode == IMAGE_MODE_ZX && (col & 7) == 0 && (c[i] & 7) == 0)
              break;
          }
          if (i >= count) // not found
          {
            // ZX Spectrum : check that both colors have same intensity (except black)
            if (mode == IMAGE_MODE_ZX && count > 0)
            {
              if ((col & 8) != (c[0] & 8) && (col & 7) != 0 && (c[0] & 7) != 0)
              {
                GFX2_Log(GFX2_INFO, "Check_block_constraints() intensity error at (%d,%d) color=%d (other color=%d)\n", x+x2, y+y2, col, c[0]);
                if (Main.backups->Pages->Nb_layers < 2)
                  Add_layer(Main.backups, 1);
                Pixel_in_layer(1, x+x2, y+y2, errcol);
                error_count++;
                continue;
              }
            }
            if (count >= max_colors)
            {
              // constraint error : add color 17 pixel in layer 2
              GFX2_Log(GFX2_INFO, "Check_block_constraints() constraint error at (%d,%d)\n", x+x2, y+y2);
              if (Main.backups->Pages->Nb_layers < 2)
                Add_layer(Main.backups, 1);
              if (mode == IMAGE_MODE_TMS9918G2)
              {
                Pixel_in_layer(1, x+x2, y+y2, col); // put color in sprite layer
                Pixel_in_layer(0, x+x2, y+y2, c[0]); // put other color in picture layer
              }
              else
                Pixel_in_layer(1, x+x2, y+y2, errcol);
              error_count++;
            }
            else
              c[count++] = col;
          }
        }
      }
    }
  }
  if (error_count > 0)
    Main.current_layer = 0; // activate layer 1 again
  return error_count;
}

/// convert a picture to the HGR mode
///
/// Recognize monochrome pictures.
/// Color pictures should use the 8 first colors.
static void Convert_to_hgr(void)
{
  int i;
  word count, x, y;
  dword usage[256];

  count = Count_used_colors(usage);
  if (count <= 1) // blank picture, nothing to do :)
    return;
  if (count == 2) // monochrome !
  {
    byte bg, fg;
    i = 0;
    while (usage[i] == 0 && i < 256)
      i++;
    bg = (byte)i;
    i++;
    while (usage[i] == 0 && i < 256)
      i++;
    fg = (byte)i;
    GFX2_Log(GFX2_DEBUG, "Convert_to_hgr() monochrome bg=%u fg=%u\n", bg, fg);
    if (!(bg == 0 && fg == 3) && !(bg == 4 && fg == 7))
    {
      // convert to B&W
      for (y = 0; y < Main.image_height; y++)
      {
        for (x = 0; x < Main.image_width; x++)
        {
          byte c = Read_pixel_from_layer(0, x, y);
          Pixel_in_layer(0, x, y, (c == fg) ? 3 : 0);
        }
      }
    }
  }
  else
  {
    // "convert" color picture to B&W
    for (y = 0; y < Main.image_height; y++)
    {
      for (x = 0; x < Main.image_width; x++)
      {
        byte c = Read_pixel_from_layer(0, x, y);
        switch (c & 3)
        {
          case 0: // black
          case 3: // white
            Pixel_in_layer(0, x, y, c);
            break;
          case 1: // green/orange
          case 2: // purple/blue
            Pixel_in_layer(0, x, y, (c & 4) | (((c & 1) ^ (x & 1) ^ 1) * 3));
        }
      }
    }
  }
  // update color layer
  for (y = 0; y < Main.image_height; y++)
  {
    for (x = 0; x < Main.image_width; x++)
    {
      Update_color_hgr_pixel(x, y, 0);
    }
  }
}

/// convert a picture to the DHGR mode
///
/// Recognize monochrome pictures.
static void Convert_to_dhgr(void)
{
  int i;
  word count, x, y;
  dword usage[256];

  count = Count_used_colors(usage);
  if (count <= 1) // blank picture, nothing to do :)
    return;
  if (count == 2) // monochrome !
  {
    byte bg, fg;
    i = 0;
    while (usage[i] == 0 && i < 256)
      i++;
    bg = (byte)i;
    i++;
    while (usage[i] == 0 && i < 256)
      i++;
    fg = (byte)i;
    GFX2_Log(GFX2_DEBUG, "Convert_to_dhgr() monochrome bg=%u fg=%u\n", bg, fg);
    if (!(bg == 0 && fg == 3) && !(bg == 4 && fg == 7))
    {
      // convert to B&W
      for (y = 0; y < Main.image_height; y++)
      {
        for (x = 0; x < Main.image_width; x++)
        {
          byte c = Read_pixel_from_layer(0, x, y);
          Pixel_in_layer(0, x, y, (c == fg) ? 15 : 0);
        }
      }
    }
  }
  else
  {
    // "convert" color picture to B&W
    for (y = 0; y < Main.image_height; y++)
    {
      for (x = 0; x < Main.image_width; x += 4)
      {
        byte c = Read_pixel_from_layer(0, x, y);
        Pixel_in_layer(0, x, y, c & 0x18);
        Pixel_in_layer(0, x + 1, y, c & 0x14);
        Pixel_in_layer(0, x + 2, y, c & 0x12);
        Pixel_in_layer(0, x + 3, y, c & 0x11);
      }
    }
  }
  // update color layer
  for (y = 0; y < Main.image_height; y++)
  {
    for (x = 0; x < Main.image_width; x += 4)
    {
      Update_color_dhgr_pixel(x, y, 0);
    }
  }
}


/// Constaint enforcer/checker
///
/// A call toggles between constraint mode and Layered mode.
void Button_Constraint_mode(void)
{
  int pixel;
  
  if (Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION)
  {
    // Disable
    Switch_layer_mode(IMAGE_MODE_LAYERED);
    return;
  }

  if (Selected_Constraint_Mode <= IMAGE_MODE_ANIMATION)
    Selected_Constraint_Mode = IMAGE_MODE_EGX;  ///@todo load prefered/last used contrained mode from config ?

  if (Selected_Constraint_Mode == IMAGE_MODE_MODE5 && (Main.image_width%48))
  {
    Verbose_message("Error!", "Emulation of Amstrad CPC's Mode5 can only be used on an image whose width is a multiple of 48.");
    return;
  }
  if (Selected_Constraint_Mode == IMAGE_MODE_C64FLI && ((Main.image_width < 160) || (Main.image_height < 200)))
  {
    Verbose_message("Error!", "Emulation of Commodore 64 FLI Mode needs a 160x200 sized image.");
    return;
  }

  // now check the constraints on existing pixels
  switch (Selected_Constraint_Mode)
  {
  case IMAGE_MODE_MODE5:
  case IMAGE_MODE_RASTER:
    // switch to layer mode if needed
    if (Main.backups->Pages->Image_mode != IMAGE_MODE_LAYERED)
      Switch_layer_mode(IMAGE_MODE_LAYERED);
    // auto-create extra layers
    while (Main.backups->Pages->Nb_layers < 5)
      if (Add_layer(Main.backups, Main.backups->Pages->Nb_layers))
      {
        Verbose_message("Error!", "Failed to create the 5 layers needed by Emulation of Amstrad CPC's rasters.");
        return;
      }
    for (pixel=0; pixel < Main.image_width*Main.image_height; pixel++)
    {
      if (Main.backups->Pages->Image[4].Pixels[pixel]>3)
      {
        GFX2_Log(GFX2_INFO, "pixel[%u]=0x%02x\n", pixel, Main.backups->Pages->Image[4].Pixels[pixel]);
        Verbose_message("Error!", "Emulation of Amstrad CPC's rasters needs all pixels of layer 5 to use colors 0-3.");
        return;
      }
    }
    break;
  case IMAGE_MODE_C64FLI:
    // switch to layer mode if needed
    if (Main.backups->Pages->Image_mode != IMAGE_MODE_LAYERED)
      Switch_layer_mode(IMAGE_MODE_LAYERED);
    Main.backups->Pages->Transparent_color = 16;
    // auto-create extra layers
    while (Main.backups->Pages->Nb_layers < 3)
      if (Add_layer(Main.backups, 0))
      {
        Verbose_message("Error!", "Failed to create the 3 layers needed by C64 Flexible Line Interpretation mode.");
        return;
      }
    {
      word x, y;
      byte bitmap[8000],screen_ram[1024*8],color_ram[1024];
      byte background[200];

      memset(bitmap, 0, sizeof(bitmap));
      memset(screen_ram, 0, sizeof(screen_ram));
      memset(color_ram, 0, sizeof(color_ram));
      memset(background, 0, sizeof(background));

      // give "hints" to the converter
      for (y = 0; y < 200; y++)
        background[y] = Read_pixel_from_layer(0, 0, y);
      for (y = 0; y < 25; y++)
      {
        for (x = 0; x < 40; x++)
          color_ram[x + y*40] = Read_pixel_from_layer(1, x*4, y*8);
      }

      if (C64_pixels_to_FLI(bitmap, screen_ram, color_ram, background, Main.backups->Pages->Image[2].Pixels, Main.image_width, 1) > 0)
      {
        // put errors in layer 4 if not already done
        if (Main.backups->Pages->Nb_layers < 4)
        {
          Add_layer(Main.backups, Main.backups->Pages->Nb_layers);
          C64_pixels_to_FLI(bitmap, screen_ram, color_ram, background, Main.backups->Pages->Image[2].Pixels, Main.image_width, 1);
        }
      }

      // copy background to layer 1
      // and color RAM to layer 2
      for (y = 0; y < 200; y++)
      {
        for (x = 0; x < 160; x++)
        {
          Pixel_in_layer(0, x, y, background[y]);
          Pixel_in_layer(1, x, y, color_ram[(x >> 2) + (y >> 3)*40]);
        }
      }
    }
    break;
  case IMAGE_MODE_HGR:
  case IMAGE_MODE_DHGR:
    // switch to layer mode if needed
    if (Main.backups->Pages->Image_mode != IMAGE_MODE_LAYERED)
      Switch_layer_mode(IMAGE_MODE_LAYERED);
    // auto-create extra layers
    while (Main.backups->Pages->Nb_layers < 2)
      if (Add_layer(Main.backups, Main.backups->Pages->Nb_layers))
      {
        Verbose_message("Error!", "Failed to create the 2 layers needed by Emulation of Apple II HGR or DHGR.");
        return;
      }
    if (Selected_Constraint_Mode == IMAGE_MODE_HGR)
      Convert_to_hgr();
    else
      Convert_to_dhgr();
    break;
  case IMAGE_MODE_TMS9918G2:
    // switch to layer mode if needed
    if (Main.backups->Pages->Image_mode != IMAGE_MODE_LAYERED)
      Switch_layer_mode(IMAGE_MODE_LAYERED);
    Main.backups->Pages->Transparent_color = 0;
    Check_block_constraints(Selected_Constraint_Mode);
    break;
  default:
    Check_block_constraints(Selected_Constraint_Mode);
  }
  /// Setting the palette is done in @ref Button_Constraint_menu (8-bit constraint window)

  /// @todo backup
  Switch_layer_mode(Selected_Constraint_Mode);
}


void Button_Constraint_menu(void)
{
  unsigned int i;
  int set_palette = 1;
  int set_pic_size = 0;
  int set_grid = 1;
  short clicked_button;
  T_Dropdown_button* dropdown;
  const char * label;
  const char * summary;
  static const struct {
    enum IMAGE_MODES mode;
    const char * label;
    const char * summary;
    int grid;
  } modes[] = {
    {IMAGE_MODE_ZX,      "ZX Spectrum",   "2 colors per 8x8 block", 1}, // 256x192
    {IMAGE_MODE_GBC,     "Game Boy Color","4 colors per 8x8 block", 1}, // 160x144 to 256x256
    {IMAGE_MODE_MEGADRIVE,"Sega MegaDrive","16colors per 8x8 block",1}, // 256x224 to 1024x256
    {IMAGE_MODE_THOMSON, "40col (MO/TO)", "2 colors per 8x1 block", 1}, // 320x200
    {IMAGE_MODE_EGX,     "EGX (CPC)",     "Alternate Mode0/Mode1 ", 0}, // 320x200
    {IMAGE_MODE_EGX2,    "EGX2 (CPC)",    "Alternate Mode1/Mode2 ", 0}, // 640x200
    {IMAGE_MODE_MODE5,   "Mode 5 (CPC)",  "Mode5                 ", 0}, // 288x256
    {IMAGE_MODE_RASTER,  "Rasters (CPC)", "CPC Rasters           ", 1},
    {IMAGE_MODE_C64HIRES,"C64 HiRes",     "2 colors per 8x8 block", 1}, // 320x200
    {IMAGE_MODE_C64MULTI,"C64 Multicolor","4 colors per 4x1 block", 1}, // 160x200
    {IMAGE_MODE_C64FLI,  "C64 FLI",       "improved multicolor   ", 1}, // 160x200
    {IMAGE_MODE_HGR,     "Apple II HGR",  "6 colors              ", 1},  // 280x192
    {IMAGE_MODE_DHGR,    "Apple II DHGR", "\"Le Chat Mauve\" mode3 ", 1},  // 560x192
    {IMAGE_MODE_TMS9918G2,"TMS9918 Mode 2","MSX Screen2, etc.    ", 1},  // 256x192
  };

  Open_window(194,95+36,"8-bit constraints");

  Window_set_normal_button(31,71+36,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(112,71+36,51,14,"OK"    ,0,1,KEY_RETURN); // 2

  label = "Constraints";
  summary = "";
  for (i = 0; i < sizeof(modes)/sizeof(modes[0]) ; i++)
    if (Selected_Constraint_Mode == modes[i].mode)
    {
      label = modes[i].label;
      summary = modes[i].summary;
      set_grid = modes[i].grid;
      break;
    }
  dropdown = Window_set_dropdown_button(32, 21, 130, 14, 120, label, 1, 0, 1, RIGHT_SIDE|LEFT_SIDE, 0); // 3
  for (i = 0; i < sizeof(modes)/sizeof(modes[0]) ; i++)
    Window_dropdown_add_item(dropdown, modes[i].mode, modes[i].label);
  Print_in_window(10, 21+18, summary, MC_Dark, MC_Light);

  Window_set_normal_button(10, 51, 14, 14, set_palette?"X":" ", 0, 1, KEY_p);  // 4
  Print_in_window_underscore(10+18, 51+3, "Set palette", MC_Dark, MC_Light, 5);

  Window_set_normal_button(10, 69, 14, 14, set_pic_size?"X":" ", 0, 1, KEY_s);  // 5
  Print_in_window_underscore(10+18, 69+3, "Set picture size", MC_Dark, MC_Light, 13);

  Window_set_normal_button(10, 87, 14, 14, set_grid?"X":" ", 0, 1, KEY_g);  // 6
  Print_in_window_underscore(10+18, 87+3, "Enable grid", MC_Dark, MC_Light, 8);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key, 0x100+BUTTON_HELP))
    {
      Key = 0;
      Window_help(BUTTON_EFFECTS, "8 BIT");
    }
    else if (clicked_button == 3)
    {
      if (Selected_Constraint_Mode == IMAGE_MODE_GBC || Selected_Constraint_Mode == IMAGE_MODE_MEGADRIVE)
        set_palette = 1;  // activate palette back when switching from GBC
      Selected_Constraint_Mode = Window_attribute2;
      for (i = 0; i < sizeof(modes)/sizeof(modes[0]) ; i++)
        if (Selected_Constraint_Mode == modes[i].mode)
        {
          set_grid = modes[i].grid;
          Hide_cursor();
          Print_in_window(10, 21+18, modes[i].summary, MC_Dark, MC_Light);
          Display_cursor();
          break;
        }
      if (Selected_Constraint_Mode == IMAGE_MODE_GBC || Selected_Constraint_Mode == IMAGE_MODE_MEGADRIVE)
        set_palette = 0;
    }
    else if (clicked_button == 4) // palette
      set_palette = !set_palette;
    else if (clicked_button == 5) // picture size
      set_pic_size = !set_pic_size;
    else if (clicked_button == 6) // enable grid
      set_grid = !set_grid;

    if (clicked_button > 0) // refresh buttons
    {
      Hide_cursor();
      Print_in_window(10+3, 51+3, set_palette?"X":" ", MC_Black, MC_Light);
      Print_in_window(10+3, 69+3, set_pic_size?"X":" ", MC_Black, MC_Light);
      Print_in_window(10+3, 87+3, set_grid?"X":" ", MC_Black, MC_Light);
      Display_cursor();
    }
  }
  while ( (clicked_button!=1) && (clicked_button!=2) );

  Close_window();

  if (clicked_button==2) // OK
  {
    if (Selected_Constraint_Mode > IMAGE_MODE_ANIMATION)
    {
      if (set_pic_size)
      {
        switch (Selected_Constraint_Mode)
        {
          case IMAGE_MODE_ZX:
            Resize_image(256, 192);
            End_of_modification();
            break;
          case IMAGE_MODE_GBC:
            Resize_image(160, 144);
            End_of_modification();
            break;
          case IMAGE_MODE_MODE5:
            Resize_image(288, 256);
            End_of_modification();
            break;
          case IMAGE_MODE_EGX:
          case IMAGE_MODE_THOMSON:
          case IMAGE_MODE_C64HIRES:
            Resize_image(320, 200);
            End_of_modification();
            break;
          case IMAGE_MODE_EGX2:
            Resize_image(640, 200);
            End_of_modification();
            break;
          case IMAGE_MODE_C64MULTI:
          case IMAGE_MODE_C64FLI:
            Resize_image(160, 200);
            End_of_modification();
            /// @todo enable WIDE pixels when switching to 160x200
            break;
          case IMAGE_MODE_HGR:
            Resize_image(280, 192);
            End_of_modification();
            break;
          case IMAGE_MODE_DHGR:
            Resize_image(560, 192);
            End_of_modification();
            /// @todo enable TALL pixels when switching to 560x192
            break;
          case IMAGE_MODE_TMS9918G2:
            Resize_image(256, 192);
            End_of_modification();
            break;
          default:
            break;
        }
      }
      if (Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION)
        Button_Constraint_mode();  // unactivate current mode
      Button_Constraint_mode();  // activate selected Mode
      if (set_grid)
      {
        switch (Selected_Constraint_Mode)
        {
          case IMAGE_MODE_ZX:
          case IMAGE_MODE_GBC:
          case IMAGE_MODE_C64HIRES:
          case IMAGE_MODE_MEGADRIVE:
            Snap_width = 8;
            Snap_height = 8;
            break;
          case IMAGE_MODE_C64MULTI:
          case IMAGE_MODE_C64FLI:
            Snap_width = 4;
            Snap_height = 8;
            break;
          case IMAGE_MODE_RASTER:
          case IMAGE_MODE_THOMSON:
          case IMAGE_MODE_TMS9918G2:
            Snap_width = 8;
            Snap_height = 999;  // maximum value (3 digits)
            break;
          case IMAGE_MODE_HGR:
            Snap_width = 7;
            Snap_height = 999;  // maximum value (3 digits)
            break;
          case IMAGE_MODE_DHGR:
            Snap_width = 4;
            Snap_height = 999;  // maximum value (3 digits)
            break;
          default:
            set_grid = 0;
        }
        if (set_grid)
        {
          Show_grid = 1;
          Snap_offset_X = 0;
          Snap_offset_Y = 0;
          Snap_mode = 0;
          //Tilemap_update();
          Display_all_screen();
        }
      }
      if (set_palette)
      {
        switch (Selected_Constraint_Mode)
        {
          case IMAGE_MODE_ZX:
            memset(Main.palette, 0, sizeof(T_Palette));
            ZX_Spectrum_set_palette(Main.palette);
            First_color_in_palette = 0;
            Fore_color = 7;
            Back_color = 0;
            break;
          case IMAGE_MODE_MEGADRIVE:  // 64 colors among 512
            memset(Main.palette + 64, 0, sizeof(T_Components) * (256 - 64));
            Main.palette[65].R = 255; // for color clashes
            First_color_in_palette = 0;
            Fore_color = 15;
            Back_color = 0;
            break;
          case IMAGE_MODE_GBC:  // 32 colors among 32768
            memset(Main.palette + 32, 0, sizeof(T_Components) * (256 - 32));
            Main.palette[33].R = 255; // for color clashes
            First_color_in_palette = 0;
            Fore_color = 3;
            Back_color = 0;
            break;
          case IMAGE_MODE_THOMSON:
            {
              static const T_MultipleChoice moto_choices[] = {
                { MACHINE_TO7, "TO7 / TO7/70", "16 colors" },
                { MACHINE_MO5, "MO5", "16 colors" },
                { MACHINE_TO8, "TO9/TO8/TO9+", "4096 colors" },
                { MACHINE_MO6, "MO6", "4096 colors" },
                { -1, NULL, NULL }
              };
              int machine = Dialog_multiple_choice("Select machine", moto_choices, -1);
              if (machine >= 0)
              {
                memset(Main.palette, 0, sizeof(T_Palette));
                if (machine == MACHINE_MO5 || machine == MACHINE_MO6)
                  MOTO_set_MO5_palette(Main.palette);
                else
                  MOTO_set_TO7_palette(Main.palette);
                if (machine == MACHINE_MO6 || machine == MACHINE_TO8)
                {
                  Set_palette_Gamma(Config.MOTO_gamma);
                  Set_palette_RGB_scale(16);
                }
                First_color_in_palette = 0;
                Fore_color = 7;
                Back_color = 0;
              }
            }
            break;
          case IMAGE_MODE_EGX:
          case IMAGE_MODE_EGX2:
            {
              static const T_MultipleChoice cpc_choices[] = {
                { 1, "CPC", "27 colors" },
                { 2, "CPC+", "4096 colors" },
                { -1, NULL, NULL }
              };
              int machine = Dialog_multiple_choice("Select machine", cpc_choices, -1);
              if (machine >= 0)
              {
                memset(Main.palette, 0, sizeof(T_Palette));
                CPC_set_default_BASIC_palette(Main.palette);
                First_color_in_palette = 0;
                Fore_color = 1;
                Back_color = 0;
                if (machine == 2)
                  Set_palette_RGB_scale(16);
                else
                  Set_palette_RGB_scale(3);
              }
            }
            break;
          case IMAGE_MODE_MODE5:
          case IMAGE_MODE_RASTER:
            memset(Main.palette, 0, sizeof(T_Palette));
            // setup colors 0,1,2,3 to see something in the thumbnail preview of layer 5
            Main.palette[1].R = 60;
            Main.palette[2].B = 60;
            Main.palette[3].G = 60;
            CPC_set_HW_palette(Main.palette + 0x40);
            First_color_in_palette = 64;
            Fore_color = 0x4b;
            Back_color = 0x54;
            break;
          case IMAGE_MODE_C64HIRES:
          case IMAGE_MODE_C64MULTI:
          case IMAGE_MODE_C64FLI:
            memset(Main.palette, 0, sizeof(T_Palette));
            C64_set_palette(Main.palette);
            First_color_in_palette = 0;
            Fore_color = 1;
            Back_color = 0;
            break;
          case IMAGE_MODE_HGR:
            memset(Main.palette, 0, sizeof(T_Palette));
            HGR_set_palette(Main.palette);
            First_color_in_palette = 0;
            Fore_color = 3;
            Back_color = 0;
            break;
          case IMAGE_MODE_DHGR:
            memset(Main.palette, 0, sizeof(T_Palette));
            DHGR_set_palette(Main.palette);
            First_color_in_palette = 0;
            Fore_color = 15;
            Back_color = 0;
            break;
          case IMAGE_MODE_TMS9918G2:
            memset(Main.palette, 0, sizeof(T_Palette));
            MSX_set_palette(Main.palette);
            First_color_in_palette = 0;
            Fore_color = 15;  // White
            Back_color = 1;   // Black
            break;
          default:
            break;
        }
        for (i = 0; i < 4; i++)
          memcpy(Main.palette + 252 + i, Favorite_GUI_color(i), sizeof(T_Components));
        // Refresh palette
        Set_palette(Main.palette);
        Compute_optimal_menu_colors(Main.palette);
        Remap_screen_after_menu_colors_change();
        Redraw_layered_image();
        Check_menu_mode();
        Display_all_screen();
        //Display_menu_palette();
        Display_menu();
      }
    }
  }

  Display_cursor();
}

// Tilemap mode
void Button_Tilemap_mode(void)
{
  Main.tilemap_mode=!Main.tilemap_mode;
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
  Window_set_normal_button(110,102,51,14,"OK"    ,0,1,KEY_RETURN); // 2

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
      !Main.tilemap_mode;
    
    Config.Tilemap_allow_flipped_x=flip_x;
    Config.Tilemap_allow_flipped_y=flip_y;
    Config.Tilemap_show_count=count;
    
    if (changed)
    {
      Main.tilemap_mode=1;
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
  Window_rectangle(Window_palette_button_list->Pos_X+4+(color >> 4)*10,
        Window_palette_button_list->Pos_Y+3+(color & 15)* 5,
        2,5,tag_color);
}

void Stencil_update_color(byte color)
{
  Update_window_area(Window_palette_button_list->Pos_X+4+(color >> 4)*10,
      Window_palette_button_list->Pos_Y+3+(color & 15)* 5,
      2,5);
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

  // Entering this window automatically enables "snap"
  char snapgrid = 1;

  T_Special_button * input_x_button;
  T_Special_button * input_y_button;
  T_Special_button * input_dx_button;
  T_Special_button * input_dy_button;

  char str[4];


  Open_window(149,118,"Grid");

  Window_set_normal_button(12,92,51,14,"Cancel",0,1,KEY_ESC);  // 1
  Window_set_normal_button(86,92,51,14,"OK"    ,0,1,KEY_RETURN); // 2

  Print_in_window(11,26, "X:",MC_Dark,MC_Light);
  input_x_button = Window_set_input_button(29,24,3); // 3
  Num2str(MIN(chosen_X, 999), str, 3);
  Window_input_content(input_x_button,str);

  Print_in_window(11,47, "Y:",MC_Dark,MC_Light);
  input_y_button = Window_set_input_button(29,45,3); // 4
  Num2str(MIN(chosen_Y, 999), str, 3);
  Window_input_content(input_y_button,str);

  Print_in_window(77,26,"dX:",MC_Dark,MC_Light);
  input_dx_button = Window_set_input_button(103,24,3); // 5
  Num2str(dx_selected,str,3);
  Window_input_content(input_dx_button,str);

  Print_in_window(77,47,"dY:",MC_Dark,MC_Light);
  input_dy_button = Window_set_input_button(103,45,3); // 6
  Num2str(dy_selected,str,3);

  Window_set_normal_button(12, 62, 14, 14, " ", 0, 1, 0);  // 7
  if (snapgrid)
    Print_in_window(16, 65, "X", MC_Black, MC_Light);
  Print_in_window(32, 65,"Snap",MC_Dark,MC_Light);

  Window_input_content(input_dy_button,str);
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case 3 :
        Num2str(MIN(chosen_X, 999), str, 3);
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
        Num2str(MIN(chosen_Y, 999), str, 3);
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
        break;
      case 7:
        snapgrid = !snapgrid;
        Hide_cursor();
        Print_in_window(16, 65, snapgrid?"X":" ", MC_Black, MC_Light);
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
    
    if (modified)
    {
      Tilemap_update();
      Disable_tilemap(&Spare);
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


static const byte Smooth_default_matrices[4][3][3]=
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
  char  str[4];

  Open_window(142,109,"Smooth");

  Window_set_normal_button(82,59,53,14,"Cancel",0,1,KEY_ESC); // 1
  Window_set_normal_button(82,88,53,14,"OK"    ,0,1,KEY_RETURN); // 2

  Window_display_frame(6,17,130,37);
  for (x=11,y=0; y<4; x+=31,y++)
  {
    Window_set_normal_button(x,22,27,27,"",0,1,KEY_NONE);      // 3,4,5,6
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
  Window_set_normal_button(16,34,108,14,"Interpolate",1,1,KEY_i); // 2
  Window_display_frame(12,18,116,34);

  Window_set_normal_button(16,54,108,14,"Additive"   ,2,1,KEY_d); // 3
  Window_set_normal_button(16,71,108,14,"Subtractive",1,1,KEY_s); // 4
  Window_set_normal_button(16,88,108,14,"Alpha",1,1,KEY_a); // 4

  Window_set_normal_button(16,111, 51,14,"Cancel"     ,0,1,KEY_ESC); // 5
  Window_set_normal_button(73,111, 51,14,"OK"         ,0,1,KEY_RETURN); // 6

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
  Window_set_normal_button(74,55,51,14,"OK"    ,0,1,KEY_RETURN); // 2
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
  Main.tilemap_mode=0;
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

  memcpy(old_sieve,Sieve,256);

  Open_window(290,179,"Sieve");

  Window_display_frame      (  7, 65,130,43);
  Window_display_frame      (  7,110,130,43);
  Window_display_frame_in(142, 68, 82,82);
  Window_display_frame_in(229, 77, 53,73);

  Print_in_window(228, 68,"Preview",MC_Dark,MC_Light);
  Print_in_window( 27, 83,"Scroll" ,MC_Dark,MC_Light);
  Print_in_window( 23,120,"Width:" ,MC_Dark,MC_Light);
  Print_in_window( 15,136,"Height:",MC_Dark,MC_Light);

  Window_set_special_button(143,69,80,80,0);                     // 1

  Window_set_normal_button(175,157,51,14,"Cancel",0,1,KEY_ESC); // 2
  Window_set_normal_button(230,157,51,14,"OK"    ,0,1,KEY_RETURN); // 3

  Window_set_normal_button(  8,157,51,14,"Clear" ,1,1,KEY_c); // 4
  Window_set_normal_button( 63,157,51,14,"Invert",1,1,KEY_i); // 5

  Window_set_normal_button(  8,46,131,14,"Get from brush"   ,1,1,KEY_g); // 6
  Window_set_normal_button(142,46,139,14,"Transfer to brush",1,1,KEY_t); // 7

  Window_set_normal_button(109,114,11,11,"\030",0,1,KEY_UP|GFX2_MOD_SHIFT); // 8
  Window_set_normal_button(109,138,11,11,"\031",0,1,KEY_DOWN|GFX2_MOD_SHIFT); // 9
  Window_set_normal_button( 97,126,11,11,"\033",0,1,KEY_LEFT|GFX2_MOD_SHIFT); // 10
  Window_set_normal_button(121,126,11,11,"\032",0,1,KEY_RIGHT|GFX2_MOD_SHIFT); // 11
  button_bg_color = Window_set_normal_button(109,126,11,11,""    ,0,1,KEY_INSERT); // 12
  Window_rectangle(button_bg_color->Pos_X+2,
        button_bg_color->Pos_Y+2,
        7, 7, (default_bg_color)?MC_White:MC_Black);

  Window_set_repeatable_button(109, 69,11,11,"\030",0,1,KEY_UP); // 13
  Window_set_repeatable_button(109, 93,11,11,"\031",0,1,KEY_DOWN); // 14
  Window_set_repeatable_button( 97, 81,11,11,"\033",0,1,KEY_LEFT); // 15
  Window_set_repeatable_button(121, 81,11,11,"\032",0,1,KEY_RIGHT); // 16

  for (index=0; index<12; index++)
    Window_set_normal_button((index*23)+8,20,20,20,"",0,1,KEY_F1+index); // 17 -> 28
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
        memcpy(Brush_original_palette, Main.palette,sizeof(T_Palette));
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
        Window_rectangle(button_bg_color->Pos_X+2,
              button_bg_color->Pos_Y+2,
              7, 7, (default_bg_color)?MC_White:MC_Black);
        Display_cursor();
        Update_window_area(
          button_bg_color->Pos_X+2,
          button_bg_color->Pos_Y+2,
          7,
          7);

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


