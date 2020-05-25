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
#include <stdlib.h>
#include "../loadsave.h"
#include "../global.h"
#include "../gfx2log.h"

void Pre_load(T_IO_Context *context, short width, short height, long file_size, int format, enum PIXEL_RATIO ratio, byte bpp)
{
  printf("Pre_load(%p, %hd, %hd, %ld, %d, %d, %hhu)\n",
         context, width, height, file_size, format, ratio, bpp);
  context->Width = width;
  context->Height = height;
  if (bpp > 8) {
    fprintf(stderr, "Truecolor not supported yet\n");
    File_error = 1;
  }
  if (context->Type == CONTEXT_SURFACE)
  {
    if (context->Surface)
      Free_GFX2_Surface(context->Surface);
    context->Surface = New_GFX2_Surface(width, height);
    if (context->Surface == NULL)
      File_error = 1;
    else
    {
      context->Target_address = context->Surface->pixels;
      context->Pitch = context->Surface->w;
    }
  }
}

byte Get_pixel(T_IO_Context *context, short x, short y)
{
  if (x < 0 || x >= context->Width || y < 0 || y >= context->Height)
    return 0;
  return context->Target_address[y*context->Pitch + x];
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
  if (context->Type == CONTEXT_SURFACE)
  {
    if (context->Surface == NULL)
    {
      GFX2_Log(GFX2_ERROR, "Set_pixel() : no Surface allocated\n");
      File_error = 1;
    }
    if ((x < 0) || (x >= context->Surface->w) || (y < 0) || (y >= context->Surface->h))
    {
      GFX2_Log(GFX2_WARNING, "Set_pixel() : position %(%hd,%hd) is outside of the image\n", x, y);
      return;
    }
    Set_GFX2_Surface_pixel(context->Surface, x, y, c);
  }
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
  (void)context;
  (void)layer;
  //printf("Set_loading_layer(%p, %d)\n", context, layer);
}

void Set_image_mode(T_IO_Context *context, enum IMAGE_MODES mode)
{
  printf("Set_image_mode(%p, %d)\n", context, mode);
}

enum IMAGE_MODES Get_image_mode(T_IO_Context *context)
{
  (void)context;
  return -1;
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

int Get_palette_RGB_scale(void)
{
  return 256;
}
