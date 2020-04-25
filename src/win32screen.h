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

//////////////////////////////////////////////////////////////////////////////
///@file win32screen.h
//////////////////////////////////////////////////////////////////////////////
#ifndef WIN32SCREEN_H_INCLUDED
#define WIN32SCREEN_H_INCLUDED

#if defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2)
#include <windows.h>

int Init_Win32(HINSTANCE hInstance, HINSTANCE hPrevInstance);
#endif

#endif
