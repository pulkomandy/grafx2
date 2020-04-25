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

#include "struct.h"

//////////////////////////////////////////////////////////////////////////////
///@file special.h
/// Editor functions that can be hooked to a keyboard shortcut, but don't have
/// a menu button associated to them.
//////////////////////////////////////////////////////////////////////////////

void Set_paintbrush_size(int width, int height);
void Smaller_paintbrush(void);
void Bigger_paintbrush(void);

void Special_next_forecolor(void);
void Special_previous_forecolor(void);
void Special_next_backcolor(void);
void Special_previous_backcolor(void);

void Special_next_user_forecolor(void);
void Special_previous_user_forecolor(void);
void Special_next_user_backcolor(void);
void Special_previous_user_backcolor(void);

void Scroll_screen(short delta_x,short delta_y);
void Scroll_magnifier(short delta_x,short delta_y);

void Zoom(short delta);
void Zoom_set(int index);

void Display_stored_brush_in_window(word x,word y,int number);
void Store_brush(int index);
byte Restore_brush(int index);

/*!
    Command that sets the transparency level.
*/
void Transparency_set(byte amount);

