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

//////////////////////////////////////////////////////////////////////////////
///@file screen.h
/// Screen update (refresh) system
//////////////////////////////////////////////////////////////////////////////

#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED

#ifdef WIN32
#include <windows.h>  // for HWND
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#if defined(SDL_VIDEO_DRIVER_X11)
#include <SDL_syswm.h> // for Display, Window
#endif
#endif
#if defined(USE_X11)
#include <X11/Xlib.h> // for Display, Window
#endif
#include "struct.h"
#include "global.h"

void GFX2_Set_mode(int *width, int *height, int fullscreen);

byte Get_Screen_pixel(int x, int y);

void Set_Screen_pixel(int x, int y, byte value);

byte* Get_Screen_pixel_ptr(int x, int y);

void Screen_FillRect(int x, int y, int w, int h, byte color);

void Update_rect(short x, short y, unsigned short width, unsigned short height);
void Flush_update(void);
void Update_status_line(short char_pos, short width);

int SetPalette(const T_Components * colors, int firstcolor, int ncolors);

///
/// Clears the parts of screen that are outside of the editing area.
/// There is such area only if the screen mode is not a multiple of the pixel
/// size, eg: 3x3 pixels in 1024x768 leaves 1 column on the right, 0 rows on bottom.
void Clear_border(byte color);
  
extern volatile int Allow_colorcycling;

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag);

#if defined(USE_SDL2)
void GFX2_UpdateScreen(void);
#endif

#if defined(WIN32)
HWND GFX2_Get_Window_Handle(void);
#endif

#if defined(USE_X11) || defined(SDL_VIDEO_DRIVER_X11)
int GFX2_Get_X11_Display_Window(Display * * display, Window * window);
#endif

/// Set application icon(s)
void Define_icon(void);

/// set (system) mouse cursor position
void Set_mouse_position(void);

#endif // SCREEN_H_INCLUDED
