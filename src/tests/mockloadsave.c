/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2019      Thomas Bernard
    Copyright 2007-2018 Adrien Destugues
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

#include <stdio.h>
#include "../loadsave.h"

void Pre_load(T_IO_Context *context, short width, short height, long file_size, int format, enum PIXEL_RATIO ratio, byte bpp)
{
  printf("Pre_load(%p, %hd, %hd, %ld, %d, %d, %hhu)\n",
         context, width, height, file_size, format, ratio, bpp);
}

byte Get_pixel(T_IO_Context *context, short x, short y)
{
  (void)context;
  (void)x;
  (void)y;
  return 0;
}

void Pixel_in_layer(int layer, word x, word y, byte color)
{
  (void)layer;
  (void)x;
  (void)y;
  (void)color;
}

void Set_pixel(T_IO_Context *context, short x, short y, byte c)
{
  (void)context;
  (void)x;
  (void)y;
  (void)c;
}

void Set_pixel_24b(T_IO_Context *context, short x, short y, byte r, byte g, byte b)
{
  (void)context;
  (void)x;
  (void)y;
  (void)r;
  (void)g;
  (void)b;
}

void Fill_canvas(T_IO_Context *context, byte color)
{
  printf("Fill_canvas(%p, %hhu)\n", context, color);
}

void Set_saving_layer(T_IO_Context *context, int layer)
{
  printf("Set_saving_layer(%p, %d)\n", context, layer);
}

void Set_loading_layer(T_IO_Context *context, int layer)
{
  printf("Set_loading_layer(%p, %d)\n", context, layer);
}

void Set_image_mode(T_IO_Context *context, enum IMAGE_MODES mode)
{
  printf("Set_image_mode(%p, %d)\n", context, mode);
}

void Set_frame_duration(T_IO_Context *context, int duration)
{
  printf("Set_frame_duration(%p, %d)\n", context, duration);
}

int Get_frame_duration(T_IO_Context *context)
{
  (void)context;
  return 0;
}
