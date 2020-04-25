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
#include <stdlib.h>
#include <string.h>
#include "gfx2surface.h"
#include "gfx2mem.h"
#include "errors.h"

T_GFX2_Surface * New_GFX2_Surface(word width, word height)
{
  T_GFX2_Surface * surface;
  size_t size;

  size = width * height;
  if (size == 0)  // surfaces with no pixels not allowed
    return NULL;

  surface = GFX2_malloc(sizeof(T_GFX2_Surface));
  if (surface == NULL)
    return NULL;
  surface->pixels = GFX2_malloc(size);
  if(surface->pixels == NULL)
  {
    free(surface);
    return NULL;
  }
  memset(surface->palette, 0, sizeof(surface->palette));
  surface->w = width;
  surface->h = height;
  return surface;
}

void Free_GFX2_Surface(T_GFX2_Surface * surface)
{
  free(surface->pixels);
  free(surface);
}

byte Get_GFX2_Surface_pixel(const T_GFX2_Surface * surface, int x, int y)
{
  if (surface == NULL) return 0;
  if (x<0 || x>=surface->w || y<0 || y>=surface->h)
  {
    GFX2_Log(GFX2_WARNING, "Get_GFX2_Surface_pixel() out of bound access (%d,%d)\n", x, y);
    return 0;
  }
  return surface->pixels[x + surface->w * y];
}

void Set_GFX2_Surface_pixel(T_GFX2_Surface * surface, int x, int y, byte value)
{
  if (surface == NULL) return;
  if (x<0 || x>=surface->w || y<0 || y>=surface->h)
  {
    GFX2_Log(GFX2_WARNING, "Set_GFX2_Surface_pixel() out of bound access (%d,%d)\n", x, y);
    return;
  }
  surface->pixels[x + surface->w * y] = value;
}
