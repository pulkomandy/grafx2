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
#include "global.h"
#include "graph.h"
#include "misc.h"
#include "readline.h"
#include "sdlscreen.h"
#include "windows.h"

#ifdef __ENABLE_LUA__

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>

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
	Pixel_in_current_screen(lua_tonumber(L, 1), lua_tonumber(L, 2),
		lua_tonumber(L, 3));
	return 0; // no values returned for lua
}

int L_GetPicturePixel(lua_State* L)
{
	uint8_t c = Read_pixel_from_current_screen(lua_tonumber(L, 1),
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

int L_BrushEnable(lua_State* L)
{
	Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
	return 0;
}

void Button_Brush_Factory(void)
{
	short clicked_button;

	Open_window(154, 162, "Brush Factory");

	Window_set_normal_button(77, 141, 67, 14, "Cancel", 0, 1, KEY_ESC); // 1
	Window_set_normal_button(10, 141, 67, 14, "Run", 0, 1, 0); // 2

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

		lua_register(L,"putbrushpixel",L_PutBrushPixel);
		lua_register(L,"getbrushpixel",L_GetBrushPixel);
		lua_register(L,"putpicturepixel",L_PutPicturePixel);
		lua_register(L,"getpicturepixel",L_GetPicturePixel);
		lua_register(L,"setbrushsize",L_SetBrushSize);
		lua_register(L,"setpicturesize",L_SetPictureSize);
		lua_register(L,"getbrushsize",L_GetBrushSize);
		lua_register(L,"getpicturesize",L_GetPictureSize);
		lua_register(L,"setcolor",L_SetColor);
		lua_register(L,"brushenable",L_BrushEnable);

		// For debug only
		// luaL_openlibs(L);

		if (luaL_loadfile(L,"./test.lua") != 0)
			Verbose_error_message(lua_tostring(L, 1));
		else if (lua_pcall(L, 0, 0, 0) != 0)
			Verbose_error_message(lua_tostring(L, 1));

		lua_close(L);
	}

	Close_window();
	Unselect_button(BUTTON_BRUSH_EFFECTS);
	Display_cursor();
}

#endif
