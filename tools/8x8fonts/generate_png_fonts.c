/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2019 Thomas Bernard

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
#include <string.h>

#include <png.h>
#include <zlib.h>

#include "font8x8/font8x8_greek.h"
#include "font8x8/font8x8_hiragana.h"

#define GENERATE_PNG(font, start) generate_png_font(directory, (const char *)font, start, sizeof(font) / 8)

int generate_png_font(const char * directory, const char * font, unsigned int start, size_t count)
{
  char filename[256];
  FILE * fp;
  png_structp png;
  png_infop info;
  unsigned int width = 256;  // 32 chars
  unsigned int height;
  unsigned int end = start + count - 1;
  unsigned char * pixels;
  unsigned int c, l, b, x, y;
  png_color palette[2] = { { 0, 0, 192}, {255, 255, 255} };

  snprintf(filename, sizeof(filename), "%sunicode_%04X-%04X.png",
           directory, start, end);
  printf("writing %s\n", filename);
  height = 8 * ((count + 31) >> 5);
  printf("  size : %dx%d\n", width, height);
  pixels = malloc(width * height);
  if (pixels == NULL)
  {
    fprintf(stderr, "Failed to allocate %d bytes\n", width * height);
    return -1;
  }
  memset(pixels, 0, width * height);

  // generate the bitmap
  for (c = 0; c < count; c++)
  {
    x = (c & 31) << 3;
    y = (c >> 5) << 3;
    for (l = 0; l < 8; l++)
    {
      int data = font[8*c + l];
      for (b = 0; b < 8; b++)
      {
        if (data & 1)
          pixels[((y + l) << 8) + x + b] = 1;
        data >>= 1;
      }
    }
  }

  // write to file
  fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    free(pixels);
    return -1;
  }
  png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                (png_voidp)NULL/*user_error_ptr*/,
                                /*user_error_fn*/NULL, /*user_warning_fn*/NULL);
  if (png == NULL)
  {
    return -1;
  }
  info = png_create_info_struct(png);
  if (info == NULL)
  {
    png_destroy_write_struct(&png, NULL);
    return -1;
  }
  if (setjmp(png_jmpbuf(png)))
  {
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(pixels);
    return -1;
  }
  png_init_io(png, fp);
  /* Set the zlib compression level */
  png_set_compression_level(png, Z_BEST_COMPRESSION);
  /* Set PNG header */
  png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_PALETTE,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  /* Set palette */
  png_set_PLTE(png, info, palette, 2);

  png_write_info(png, info);
  for (y = 0; y < height; y++)
    png_write_row(png, pixels + (y << 8));
  png_write_end(png, info);
  //png_write_flush(png);

  png_destroy_write_struct(&png, &info);
  fclose(fp);
  free(pixels);
  return 0;
}

int main(int argc, char * * argv)
{
  const char * directory = "./";
  int r;

  printf("GrafX2 unicode font generator\n");
  if (argc > 1)
    directory = argv[1];

  r = GENERATE_PNG(font8x8_greek, 0x0390);
  if (r != 0)
    return r;
  r = GENERATE_PNG(font8x8_hiragana, 0x3040);
  if (r != 0)
    return r;

  return 0;
}
