/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2020 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
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
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#else
#if defined(WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#endif

#include "struct.h"

dword GFX2_GetTicks(void)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  return SDL_GetTicks();
#elif defined(WIN32)
  return GetTickCount();
#else
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0)
    return 0;
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

