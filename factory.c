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
  
  Hide_cursor();
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
  Display_cursor();
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
        Draw_script_information(Get_item_by_index(&Scripts_list,
          scriptlist->List_start + scriptlist->Cursor_position));
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
