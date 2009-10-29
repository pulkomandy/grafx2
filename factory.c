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
#include "global.h"
#include "misc.h"
#include "readline.h"
#include "sdlscreen.h"
#include "windows.h"

#ifdef __ENABLE_LUA__

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>

// Wrapper functions to call C from Lua
int L_PutPixel(lua_State* L)
{
	Pixel_in_brush(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0; // no values returned for lua
}

int L_GetPixel(lua_State* L)
{
	uint8_t c = Read_pixel_from_brush(lua_tonumber(L, 1), lua_tonumber(L, 2));
	lua_pushinteger(L, c);
	return 1;
}

int L_SetColor(lua_State* L)
{
	Set_color(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3),
		lua_tonumber(L, 4));
	return 0;
}

void Button_Brush_Factory(void)
{
	short clicked_button;
	word height;
	word width;
	char str[5];

	T_Special_button* width_button;
	T_Special_button* height_button;

	Open_window(154, 162, "Brush Factory");

	Window_set_normal_button(77, 141, 67, 14, "Cancel", 0, 1, KEY_ESC); // 1
	Window_set_normal_button(10, 141, 67, 14, "Run", 0, 1, 0); // 2
	Print_in_window(10, 17, "Width:", MC_Black, MC_Light);
	width_button = Window_set_input_button(64, 15, 4); // 3
	Print_in_window(10, 30, "Height:", MC_Black, MC_Light);
	height_button = Window_set_input_button(64, 28, 4); // 4

	width = Paintbrush_width;
	Num2str(width, str, 4);
	Window_input_content(width_button, str);

	height = Paintbrush_height;
	Num2str(height, str, 4);
	Window_input_content(height_button, str);

	Update_window_area(0, 0, Window_width, Window_height);
	Display_cursor();

	do {
		clicked_button = Window_clicked_button();

		switch (clicked_button)
		{
			case 3 : // Largeur
				Num2str(width, str, 4);
				Readline(65, 16, str, 4, 1);
				width = atoi(str);
				// On corrige les dimensions
				if (width == 0)
				{
					width = 1;
					Num2str(width, str, 4);
					Window_input_content(width_button, str);
				}
				Display_cursor();
				break;

			case 4 : // Height
				Num2str(height, str, 4);
				Readline(65, 29, str, 4, 1);
				height = atoi(str);
				// On corrige les dimensions
				if (height == 0)
				{
					height = 1;
					Num2str(height, str, 4);
					Window_input_content(height_button, str);
				}
				Display_cursor();
				break;

			default:
				break;
		}
		
	} while (clicked_button <= 0 || clicked_button >= 3);

	if (clicked_button == 2) // Run the script
	{
		lua_State* L = lua_open();

		Realloc_brush(width, height);

		lua_register(L,"putpixel",L_PutPixel);
		lua_register(L,"getpixel",L_GetPixel);
		lua_register(L,"setcolor",L_SetColor);

		if (luaL_loadfile(L,"./test.lua") != 0)
			Verbose_error_message(lua_tostring(L, 1));

		lua_pushinteger(L, width);
		lua_pushinteger(L, height);
		if (lua_pcall(L, 2, 0, 0) != 0)
			Verbose_error_message(lua_tostring(L, 1));

		lua_close(L);

		Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
	}

	Close_window();
	Display_cursor();
}

#endif
