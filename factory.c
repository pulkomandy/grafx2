/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2009 Adrien Destugues

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

/*! \file factory.c
 *	\brief Brush factory - generates brush from lua scripts
 *
 * The brush factory allows you to generate brushes with Lua code.
 */

#include "brush.h"
#include "buttons.h"
#include "engine.h"
#include "errors.h"
#include "filesel.h" // Get_item_by_index
#include "global.h"
#include "graph.h"
#include "io.h"     // find_last_slash
#include "misc.h"
#include "pages.h"  // Backup()
#include "readline.h"
#include "sdlscreen.h"
#include "windows.h"
#include "palette.h"

#ifdef __ENABLE_LUA__

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

///
/// Number of characters for name in fileselector.
/// Window is adjusted according to it.
#define NAME_WIDTH 34
/// Position of fileselector top, in window space
#define FILESEL_Y 18

// Work data that can be used during a script
static byte * Brush_backup = NULL;
static word Brush_backup_width;
static word Brush_backup_height;
static byte Palette_has_changed;
static byte Brush_was_altered;

/// Helper function to clamp a double to 0-255 range
static inline byte clamp_byte(double value)
{
  if (value<0.0)
    return 0;
  else if (value>255.0)
    return 255;
  else return (byte)value;
}

// Wrapper functions to call C from Lua

int L_SetBrushSize(lua_State* L)
{
  int w = lua_tonumber(L,1);
  int h = lua_tonumber(L,2);
  
  if (w<1 || h<1)
  {
    return luaL_error(L, "SetBrushSize: Unreasonable arguments");
  }
	if (Realloc_brush(w, h))
	{
	  return luaL_error(L, "SetBrushSize: Resize failed");
	}
	Brush_was_altered=1;
  // Fill with Back_color
  memset(Brush,Back_color,(long)Brush_width*Brush_height);
  // Center the handle
  Brush_offset_X=(Brush_width>>1);
  Brush_offset_Y=(Brush_height>>1);
	return 0;
}

int L_GetBrushSize(lua_State* L)
{
	lua_pushinteger(L, Brush_width);	
	lua_pushinteger(L, Brush_height);	
	return 2;
}

int L_GetBrushTransparentColor(lua_State* L)
{
	lua_pushinteger(L, Back_color);
	return 1;
}

int L_PutBrushPixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
	uint8_t c = lua_tonumber(L,3);
	
	Brush_was_altered=1;
	
  if (x<0 || y<0 || x>=Brush_width || y>=Brush_height)
  ;
  else
  {
    Pixel_in_brush(x, y, c);
  }
	return 0; // no values returned for lua
}

int L_GetBrushPixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
	uint8_t c;
  if (x<0 || y<0 || x>=Brush_width || y>=Brush_height)
  {
    c = Back_color; // Return 'transparent'
  }
  else
  {
    c = Read_pixel_from_brush(x, y);
  }
	lua_pushinteger(L, c);
	return 1;
}

int L_GetBrushBackupPixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
	uint8_t c;
  if (x<0 || y<0 || x>=Brush_backup_width || y>=Brush_backup_height)
  {
    c = Back_color; // Return 'transparent'
  }
  else
  {
    c = *(Brush_backup + y * Brush_backup_width + x);
  }
	lua_pushinteger(L, c);
	return 1;
}

int L_SetPictureSize(lua_State* L)
{
  int w = lua_tonumber(L,1);
  int h = lua_tonumber(L,2);
  
  if (w<1 || h<1 || w>9999 || h>9999)
    return luaL_error(L, "SetPictureSize: Unreasonable dimensions");
    
	Resize_image(w, h); // TODO: a return value to catch runtime errors
	return 0;
}

int L_GetPictureSize(lua_State* L)
{
	lua_pushinteger(L, Main_image_width);	
	lua_pushinteger(L, Main_image_height);	
	return 2;
}

int L_PutPicturePixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
  int c = lua_tonumber(L,3);
  
  // Bound check
  if (x<0 || y<0 || x>=Main_image_width || y>=Main_image_height)
  {
    // Silently ignored
    return 0;
  }
	Pixel_in_current_screen(x, y, c, 0);
	return 0; // no values returned for lua
}

int L_GetPicturePixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
  // Bound check
  if (x<0 || y<0 || x>=Main_image_width || y>=Main_image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main_backups->Pages->Transparent_color);
	  return 1;
  }
	lua_pushinteger(L, Read_pixel_from_current_screen(x,y));
	return 1;
}

int L_GetBackupPixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
  // Bound check
  if (x<0 || y<0 || x>=Main_image_width || y>=Main_image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main_backups->Pages->Transparent_color);
	  return 1;
  }
	lua_pushinteger(L, Read_pixel_from_backup_screen(x,y));
	return 1;
}

int L_GetLayerPixel(lua_State* L)
{
  int x = lua_tonumber(L,1);
  int y = lua_tonumber(L,2);
  // Bound check
  if (x<0 || y<0 || x>=Main_image_width || y>=Main_image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main_backups->Pages->Transparent_color);
	  return 1;
  }
	lua_pushinteger(L, Read_pixel_from_current_layer(x,y));
	return 1;
}

int L_SetColor(lua_State* L)
{
  byte c=lua_tonumber(L,1);

  byte r=clamp_byte((double)lua_tonumber(L,2));
  byte g=clamp_byte((double)lua_tonumber(L,3));
  byte b=clamp_byte((double)lua_tonumber(L,4));
    
  Main_palette[c].R=Round_palette_component(r);
  Main_palette[c].G=Round_palette_component(g);
  Main_palette[c].B=Round_palette_component(b);
	// Set_color(c, r, g, b); Not needed. Update screen when script is finished
	Palette_has_changed=1;
	return 0;
}

int L_GetColor(lua_State* L)
{
	byte c=lua_tonumber(L,1);

	lua_pushinteger(L, Main_palette[c].R);
	lua_pushinteger(L, Main_palette[c].G);
	lua_pushinteger(L, Main_palette[c].B);
	return 3;
}

int L_GetBackupColor(lua_State* L)
{
	byte c=lua_tonumber(L,1);

	lua_pushinteger(L, Main_backups->Pages->Next->Palette[c].R);
	lua_pushinteger(L, Main_backups->Pages->Next->Palette[c].G);
	lua_pushinteger(L, Main_backups->Pages->Next->Palette[c].B);
	return 3;
}

int L_MatchColor(lua_State* L)
{
  byte r=clamp_byte(lua_tonumber(L,1));
  byte g=clamp_byte(lua_tonumber(L,2));
  byte b=clamp_byte(lua_tonumber(L,3));

  int c = Best_color_nonexcluded(r,g,b);
  lua_pushinteger(L, c);
  return 1;
}


int L_InputBox(lua_State* L)
{
  const int max_settings = 9;
  int min_value[max_settings];
  int max_value[max_settings];
  int current_value[max_settings];
  const char * label[max_settings];
  unsigned short control[max_settings*3+1]; // Each value has at most 3 widgets.
  enum CONTROL_TYPE {
    CONTROL_OK          = 0x0100,
    CONTROL_CANCEL      = 0x0200,
    CONTROL_INPUT       = 0x0300,
    CONTROL_INPUT_MINUS = 0x0400,
    CONTROL_INPUT_PLUS  = 0x0500,
    CONTROL_CHECKBOX    = 0x0600,
    CONTROL_VALUE_MASK  = 0x00FF,
    CONTROL_TYPE_MASK   = 0xFF00
  };
  const char * window_caption;
  int caption_length;
  int nb_settings;
  int nb_args;
  unsigned int max_label_length;
  int setting;
  short clicked_button;
  char  str[5];
  short close_window = 0;
  
  nb_args = lua_gettop (L);
  
  if (nb_args < 5)
  {
    return luaL_error(L, "InputBox: Less than 5 arguments");
  }
  if ((nb_args - 1) % 4)
  {
    return luaL_error(L, "InputBox: Wrong number of arguments");
  }
  nb_settings = (nb_args-1)/4;
  if (nb_settings > max_settings)
  {
    return luaL_error(L, "InputBox: Too many settings, limit reached");
  }
  
  max_label_length=4; // Minimum size to account for OK / Cancel buttons
  
  // First argument is window caption
  window_caption = lua_tostring(L,1);
  caption_length = strlen(window_caption);
  if ( caption_length > 14)
    max_label_length = caption_length - 10;
  
  for (setting=0; setting<nb_settings; setting++)
  {
    label[setting] = lua_tostring(L,setting*4+2);
    if (strlen(label[setting]) > max_label_length)
      max_label_length = strlen(label[setting]);
      
    current_value[setting] = lua_tonumber(L,setting*4+3);
    min_value[setting] = lua_tonumber(L,setting*4+4);
    max_value[setting] = lua_tonumber(L,setting*4+5);
    if (max_value[setting] > 9999)
      max_value[setting] = 9999;
      
    // Keep current value in range
    if (current_value[setting] < min_value[setting])
      current_value[setting] = min_value[setting];
    else if (current_value[setting] > max_value[setting])
      current_value[setting] = max_value[setting];
  }
  // Max is 25 to keep window under 320 pixel wide
  if (max_label_length>25)
    max_label_length=25;

  Hide_cursor();
  Open_window(91+max_label_length*8,44+nb_settings*17,window_caption);

  // OK
  Window_set_normal_button( 7, 25 + 17 * nb_settings, 51,14,"OK" , 0,1,SDLK_RETURN);
  control[Window_nb_buttons] = CONTROL_OK;

  // Cancel
  Window_set_normal_button( 64, 25 + 17 * nb_settings, 51,14,"Cancel" , 0,1,KEY_ESC);
  control[Window_nb_buttons] = CONTROL_CANCEL;
  
  for (setting=0; setting<nb_settings; setting++)
  {
    Print_in_window_limited(12,22+setting*17,label[setting],max_label_length,MC_Black,MC_Light);
    if (min_value[setting]==0 && max_value[setting]==1)
    {
      // Checkbox
      Window_set_normal_button(12+max_label_length*8+30, 19+setting*17, 15,13,current_value[setting]?"X":"", 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_CHECKBOX | setting;
    }
    else
    {
      // - Button
      Window_set_repeatable_button(12+max_label_length*8+5, 20+setting*17, 13,11,"-" , 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_INPUT_MINUS | setting;

      // Numeric input field
      Window_display_frame_in(12+max_label_length*8+21, 20+setting*17,35, 11);
      Window_set_input_button(12+max_label_length*8+21, 20+setting*17,4);
      // Print editable value
      Num2str(current_value[setting],str,4);
      Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 4,MC_Black,MC_Light);
      //
      control[Window_nb_buttons] = CONTROL_INPUT | setting;

      // + Button
      Window_set_repeatable_button(12+max_label_length*8+60, 20+setting*17, 13,11,"+" , 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_INPUT_PLUS | setting;
    }
  }
  
  Update_window_area(0,0,Window_width, Window_height);
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Display_cursor();

  while (!close_window)
  {
    clicked_button=Window_clicked_button();
    if (clicked_button>0)
    {
      setting = control[clicked_button] & (CONTROL_VALUE_MASK);
      
      switch (control[clicked_button] & CONTROL_TYPE_MASK)
      {
        case CONTROL_OK:
          close_window = CONTROL_OK;
          break;
          
        case CONTROL_CANCEL:
          close_window = CONTROL_CANCEL;
          break;
          
        case CONTROL_INPUT:
          Num2str(current_value[setting],str,4);
          Readline(12+max_label_length*8+23, 22+setting*17,str,4,1);
          current_value[setting]=atoi(str);

          if (current_value[setting] < min_value[setting])
            current_value[setting] = min_value[setting];
          else if (current_value[setting] > max_value[setting])
            current_value[setting] = max_value[setting];
          Hide_cursor();
          // Print editable value
          Num2str(current_value[setting],str,4);
          Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 4,MC_Black,MC_Light);
          //
          Display_cursor();
          
          break;
          
        case CONTROL_INPUT_MINUS:
          if (current_value[setting] > min_value[setting])
          {
            current_value[setting]--;
            Hide_cursor();
            // Print editable value
            Num2str(current_value[setting],str,4);
            Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 4,MC_Black,MC_Light);
            //
            Display_cursor();
          }
          break;
          
        case CONTROL_INPUT_PLUS:
          if (current_value[setting] < max_value[setting])
          {
            current_value[setting]++;
            Hide_cursor();
            // Print editable value
            Num2str(current_value[setting],str,4);
            Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 4,MC_Black,MC_Light);
            //
            Display_cursor();
          }
          break;
          
        case CONTROL_CHECKBOX:
          current_value[setting] = !current_value[setting];
          Hide_cursor();
          Print_in_window(12+max_label_length*8+34, 22+setting*17, current_value[setting]?"X":" ",MC_Black,MC_Light);
          Display_cursor();
          break;
      }
    }
  }
  
  Hide_cursor();
  Close_window();
  Cursor_shape=CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  
  // Return values:

  // One boolean to tell if user pressed ok or cancel
  lua_pushboolean(L, (close_window == CONTROL_OK));
  
  // One value per control
  for (setting=0; setting<nb_settings; setting++)
    lua_pushinteger(L, current_value[setting]);
    
  return 1 + nb_settings;
}

// Handlers for window internals
T_Fileselector Scripts_list;

// Callback to display a skin name in the list
void Draw_script_name(word x, word y, word index, byte highlighted)
{
	T_Fileselector_item * current_item;

	if (Scripts_list.Nb_elements)
	{
	  short name_size;
	  
		current_item = Get_item_by_index(&Scripts_list, index);

		Print_in_window_limited(x, y, current_item->Full_name, NAME_WIDTH, MC_Black,
			(highlighted)?MC_Dark:MC_Light);
	  name_size=strlen(current_item->Full_name);
	  // Clear remaining area on the right
	  if (name_size<NAME_WIDTH)
  	  Window_rectangle(x+name_size*8,y,(NAME_WIDTH-name_size)*8,8,(highlighted)?MC_Dark:MC_Light);

  	Update_window_area(x,y,NAME_WIDTH*8,8);
	}
}

///
/// Displays first lines of comments from a lua script in the window.
void Draw_script_information(T_Fileselector_item * script_item)
{
  FILE *script_file;
  char full_name[MAX_PATH_CHARACTERS];
  char text_block[3][NAME_WIDTH+3];
  int x, y;
  
  // Blank the target area
	Window_rectangle(7, FILESEL_Y + 89, (NAME_WIDTH+2)*8+2, 3*8, MC_Black);

  if (script_item && script_item->Full_name && script_item->Full_name[0]!='\0')
  {
    strcpy(full_name, Data_directory);
  	strcat(full_name, "scripts/");
  	strcat(full_name, script_item->Full_name);
    
    
    x=0;
    y=0;
    text_block[0][0] = text_block[1][0] = text_block[2][0] = '\0';
    // Start reading
    script_file = fopen(full_name, "r");
    if (script_file != NULL)
    {
      int c;
      c = fgetc(script_file);
      while (c != EOF && y<3)
      {
        if (c == '\n')
        {
          if (x<2)
            break; // Carriage return without comment: Stopping
          y++;
          x=0;
        }
        else if (x==0 || x==1)
        {
          if (c != '-')
            break; // Non-comment line was encountered. Stopping.       
          x++;
        }
        else
        {
          if (x < NAME_WIDTH+4)
          {
            // Adding character
            text_block[y][x-2] = (c<32 || c>255) ? ' ' : c;
            text_block[y][x-1] = '\0';
          }
          x++;
        }
        // Read next
        c = fgetc(script_file);
      }
      fclose(script_file);
    }
    
    Print_in_window(7, FILESEL_Y + 89   , text_block[0], MC_Light, MC_Black);
    Print_in_window(7, FILESEL_Y + 89+ 8, text_block[1], MC_Light, MC_Black);
    Print_in_window(7, FILESEL_Y + 89+16, text_block[2], MC_Light, MC_Black);
  
  }
  Update_window_area(7, FILESEL_Y + 89, (NAME_WIDTH+2)*8+2, 3*8);
    
}

// Add a script to the list
void Add_script(const char *name)
{
	Add_element_to_list(&Scripts_list, Find_last_slash(name)+1, 0);
}

void Button_Brush_Factory(void)
{
	short clicked_button;
	T_List_button* scriptlist;
	T_Scroller_button* scriptscroll;
	char scriptdir[MAX_PATH_CHARACTERS];

  Open_window(33+8*NAME_WIDTH, 162, "Brush Factory");

	// Here we use the same data container as the fileselectors.
	// Reinitialize the list
	Free_fileselector_list(&Scripts_list);
	strcpy(scriptdir, Data_directory);
	strcat(scriptdir, "scripts/");
	// Add each found file to the list
	For_each_file(scriptdir, Add_script);
	// Sort it
	Sort_list_of_files(&Scripts_list);

	Window_set_normal_button(85, 141, 67, 14, "Cancel", 0, 1, KEY_ESC); // 1
	Window_set_normal_button(10, 141, 67, 14, "Run", 0, 1, 0); // 2

	Window_display_frame_in(6, FILESEL_Y - 2, NAME_WIDTH*8+4, 84); // File selector
	scriptlist = Window_set_list_button(
		// Fileselector
		Window_set_special_button(8, FILESEL_Y + 1, NAME_WIDTH*8, 80), // 3
		// Scroller for the fileselector
		(scriptscroll = Window_set_scroller_button(NAME_WIDTH*8+14, FILESEL_Y - 1, 82,
			Scripts_list.Nb_elements, 10, 0)), // 4
		Draw_script_name); // 5
		
	Window_display_frame_in(6, FILESEL_Y + 88, (NAME_WIDTH+2)*8+4, 3*8+2); // Descr.
	
	Window_redraw_list(scriptlist);
  Draw_script_information(Get_item_by_index(&Scripts_list,
    scriptlist->List_start + scriptlist->Cursor_position));
  
	Update_window_area(0, 0, Window_width, Window_height);
	Display_cursor();

	do {
		clicked_button = Window_clicked_button();

		switch (clicked_button)
		{
		  case 5:
        Hide_cursor();
        Draw_script_information(Get_item_by_index(&Scripts_list,
          scriptlist->List_start + scriptlist->Cursor_position));
        Display_cursor();
		    break;
		    
		    
			default:
				break;
		}
		
	} while (clicked_button <= 0 || clicked_button >= 3);

	if (clicked_button == 2) // Run the script
	{
		lua_State* L;
    const char* message;

    // Some scripts are slow
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();
    Flush_update();
    

    L = lua_open();

		lua_register(L,"putbrushpixel",L_PutBrushPixel);
		lua_register(L,"getbrushpixel",L_GetBrushPixel);
		lua_register(L,"getbrushbackuppixel",L_GetBrushBackupPixel);
		lua_register(L,"putpicturepixel",L_PutPicturePixel);
		lua_register(L,"getpicturepixel",L_GetPicturePixel);
		lua_register(L,"getlayerpixel",L_GetLayerPixel);
		lua_register(L,"getbackuppixel",L_GetBackupPixel);
		lua_register(L,"setbrushsize",L_SetBrushSize);
		lua_register(L,"setpicturesize",L_SetPictureSize);
		lua_register(L,"getbrushsize",L_GetBrushSize);
		lua_register(L,"getpicturesize",L_GetPictureSize);
		lua_register(L,"setcolor",L_SetColor);
		lua_register(L,"getcolor",L_GetColor);
		lua_register(L,"getbackupcolor",L_GetBackupColor);
		lua_register(L,"matchcolor",L_MatchColor);
		lua_register(L,"getbrushtransparentcolor",L_GetBrushTransparentColor);
		lua_register(L,"inputbox",L_InputBox);


		// For debug only
		// luaL_openlibs(L);

    luaopen_math(L);

		strcat(scriptdir, Get_item_by_index(&Scripts_list,
					scriptlist->List_start + scriptlist->Cursor_position)
						-> Full_name);

    // TODO The script may modify the picture, so we do a backup here.
    // If the script is only touching the brush, this isn't needed...
    // The backup also allows the script to read from it to make something
    // like a feedback off effect (convolution matrix comes to mind).
    Backup();

    Palette_has_changed=0;
    Brush_was_altered=0;

    // Backup the brush
    Brush_backup=(byte *)malloc(((long)Brush_height)*Brush_width);
    Brush_backup_width = Brush_width;
    Brush_backup_height = Brush_height;
    
    if (Brush_backup == NULL)
    {
      Verbose_error_message("Out of memory!");
    }
		else 
		{
		  memcpy(Brush_backup, Brush, ((long)Brush_height)*Brush_width);
		
  		if (luaL_loadfile(L,scriptdir) != 0)
      {
        message = lua_tostring(L, 1);
        if(message)
  			  Verbose_error_message(message);
        else
          Warning_message("Unknown error loading script!");
      }
  		else if (lua_pcall(L, 0, 0, 0) != 0)
      {
        message = lua_tostring(L, 1);
        if(message)
  			  Verbose_error_message(message);
        else
          Warning_message("Unknown error running script!");
      }
    }
    // Cleanup
    free(Brush_backup);
    Brush_backup=NULL;
    if (Palette_has_changed)
    {
      Set_palette(Main_palette);
      Compute_optimal_menu_colors(Main_palette);
    }
    End_of_modification();

		lua_close(L);
	}

	Close_window();
	if (Brush_was_altered)
	  Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
	Unselect_button(BUTTON_BRUSH_EFFECTS);
  Display_all_screen();
	Display_cursor();
}

#else // NOLUA
void Button_Brush_Factory(void)
{
    Verbose_error_message("The brush factory is not available in this build of GrafX2.");
}

#endif
