/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2008 Yves Rizoud
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
#include "screen.h"

byte Get_Screen_pixel(int x, int y)
{
  return 0;
}

void Set_Screen_pixel(int x, int y, byte value)
{
}

byte* Get_Screen_pixel_ptr(int x, int y)
{
  return NULL;
}

void Screen_FillRect(int x, int y, int w, int h, byte color)
{
}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
}

void Flush_update(void)
{
}

void Update_status_line(short char_pos, short width)
{
}

int SetPalette(const T_Components * colors, int firstcolor, int ncolors)
{
  return 0;
}

void Clear_border(byte color)
{
}
  
volatile int Allow_colorcycling = 0;

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
}