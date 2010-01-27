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

#ifdef __ENABLE_LUA__

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Wrapper functions to call C from Lua

int L_SetBrushSize(lua_State* L)
{
	Realloc_brush(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

int L_GetBrushSize(lua_State* L)
{
	DEBUG("GBS",Brush_width);
	lua_pushinteger(L, Brush_width);	
	lua_pushinteger(L, Brush_height);	
	return 2;
}

int L_PutBrushPixel(lua_State* L)
{
	Pixel_in_brush(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0; // no values returned for lua
}

int L_GetBrushPixel(lua_State* L)
{
	uint8_t c = Read_pixel_from_brush(lua_tonumber(L, 1), lua_tonumber(L, 2));
	lua_pushinteger(L, c);
	return 1;
}

int L_SetPictureSize(lua_State* L)
{
	Resize_image(lua_tonumber(L, 1), lua_tonumber(L, 2));
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
	Pixel_in_current_layer(x, y, c);
	return 0; // no values returned for lua
}

int L_GetPicturePixel(lua_State* L)
{
	uint8_t c = Read_pixel_from_current_layer(lua_tonumber(L, 1),
		lua_tonumber(L, 2));
	lua_pushinteger(L, c);
	return 1;
}

int L_GetBackupPixel(lua_State* L)
{
	uint8_t c = Read_pixel_from_backup_screen(lua_tonumber(L, 1),
		lua_tonumber(L, 2));
	lua_pushinteger(L, c);
	return 1;
}

int L_SetColor(lua_State* L)
{
	Set_color(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3),
		lua_tonumber(L, 4));
	return 0;
}

int L_GetColor(lua_State* L)
{
	T_Components couleur;
	couleur = Main_palette[(int)(lua_tonumber(L,1))];

	lua_pushinteger(L, couleur.R);
	lua_pushinteger(L, couleur.G);
	lua_pushinteger(L, couleur.B);
	return 3;
}

int L_MatchColor(lua_State* L)
{
  int c = Best_color_nonexcluded(lua_tonumber(L,1), lua_tonumber(L, 2), lua_tonumber(L, 3));
  lua_pushinteger(L, c);
  return 1;
}

int L_BrushEnable(__attribute__((unused)) lua_State* L)
{
	Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
	return 0;
}

// Handlers for window internals
T_Fileselector Scripts_list;

// Callback to display a skin name in the list
void Draw_script_name(word x, word y, word index, byte highlighted)
{
	T_Fileselector_item * current_item;

	if (Scripts_list.Nb_elements)
	{
		current_item = Get_item_by_index(&Scripts_list, index);
		Print_in_window(x, y, current_item->Short_name, MC_Black,
			(highlighted)?MC_Dark:MC_Light);
	}
}

// Add a skin to the list
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

	Open_window(175, 162, "Brush Factory");

	// Here we use the same data container as the fileselectors.
	// Reinitialize the list
	Free_fileselector_list(&Scripts_list);
	strcpy(scriptdir, Data_directory);
	strcat(scriptdir, "scripts/");
	// Add each found file to the list
	For_each_file(scriptdir, Add_script);
	// Sort it
	Sort_list_of_files(&Scripts_list);

	Window_set_normal_button(77, 141, 67, 14, "Cancel", 0, 1, KEY_ESC); // 1
	Window_set_normal_button(10, 141, 67, 14, "Run", 0, 1, 0); // 2

	#define FILESEL_Y 18
	Window_display_frame_in(6, FILESEL_Y - 2, 148, 84); // File selector
	scriptlist = Window_set_list_button(
		// Fileselector
		Window_set_special_button(8, FILESEL_Y + 1, 144, 80), // 3
		// Scroller for the fileselector
		(scriptscroll = Window_set_scroller_button(154, FILESEL_Y - 1, 82,
			Scripts_list.Nb_elements, 10, 0)), // 4
		Draw_script_name); // 5
	Window_redraw_list(scriptlist);

	Update_window_area(0, 0, Window_width, Window_height);
	Display_cursor();

	do {
		clicked_button = Window_clicked_button();

		switch (clicked_button)
		{
			default:
				break;
		}
		
	} while (clicked_button <= 0 || clicked_button >= 3);

	if (clicked_button == 2) // Run the script
	{
		lua_State* L = lua_open();
    const char* message;

		lua_register(L,"putbrushpixel",L_PutBrushPixel);
		lua_register(L,"getbrushpixel",L_GetBrushPixel);
		lua_register(L,"putpicturepixel",L_PutPicturePixel);
		lua_register(L,"getpicturepixel",L_GetPicturePixel);
		lua_register(L,"getbackuppixel",L_GetBackupPixel);
		lua_register(L,"setbrushsize",L_SetBrushSize);
		lua_register(L,"setpicturesize",L_SetPictureSize);
		lua_register(L,"getbrushsize",L_GetBrushSize);
		lua_register(L,"getpicturesize",L_GetPictureSize);
		lua_register(L,"setcolor",L_SetColor);
		lua_register(L,"getcolor",L_GetColor);
		lua_register(L,"matchcolor",L_MatchColor);
		lua_register(L,"brushenable",L_BrushEnable);

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

    End_of_modification();

		lua_close(L);
	}

	Close_window();
	Unselect_button(BUTTON_BRUSH_EFFECTS);
	Display_cursor();
}

#else // NOLUA
void Button_Brush_Factory(void)
{
    Verbose_error_message("The brush factory is not available in this build of GrafX2.");
}

#endif
