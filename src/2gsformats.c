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

///@file 2gsformats.c
/// Formats for the Apple II GS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "fileformats.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "gfx2mem.h"
#include "gfx2log.h"

/**
 * Test for an Apple 2 GS picture file
 */
void Test_2GS(T_IO_Context * context, FILE * file)
{
  unsigned long file_size;
  dword chunksize;
  byte header[5];

  (void)context;
  File_error = 1;
  file_size = File_length_file(file);
  if (!Read_dword_le(file, &chunksize))
    return;
  if (!Read_bytes(file, header, sizeof(header)))
    return;
  if (0 != memcmp(header, "\x04MAIN", 5))
    return;
  if (file_size < chunksize)
    return;
  File_error = 0;
}

/**
 * Load a 16 entry Apple II GS palette from file
 */
static int Load_2GS_Palette(T_Components * palette, FILE * file)
{
  unsigned int i;
  for (i = 0; i < 16; i++)
  {
    word palentry;
    if (!Read_word_le(file, &palentry))
      return 0;
    palette[i].R = ((palentry >> 8) & 0x0f) * 0x11;
    palette[i].G = ((palentry >> 4) & 0x0f) * 0x11;
    palette[i].B = (palentry & 0x0f) * 0x11;
  }
  return 1;
}

#define COMPONENTS(x) linepal[x].R, linepal[x].G, linepal[x].B

/**
 * set the (2 or 4) pixels for the byte.
 */
#define WRITE2GS_PIXELS \
if (multipalcount > 0)  \
{                       \
  if (linemode & 0x80)  \
  {                     \
    Set_pixel_24b(context, x++, y, COMPONENTS(8 + (pixel >> 6)));        \
    Set_pixel_24b(context, x++, y, COMPONENTS(12 + ((pixel >> 4) & 3))); \
    Set_pixel_24b(context, x++, y, COMPONENTS((pixel >> 2) & 3));        \
    Set_pixel_24b(context, x++, y, COMPONENTS(4 + (pixel & 3)));         \
  }                     \
  else                  \
  {                     \
    Set_pixel_24b(context, x++, y, COMPONENTS(pixel >> 4));       \
    Set_pixel_24b(context, x++, y, COMPONENTS(pixel & 15));       \
  }                     \
}                       \
else                    \
{                       \
  if (linemode & 0x80)  \
  {                     \
    Set_pixel(context, x++, y, (linemode << 4) | 8 | (pixel >> 6));          \
    Set_pixel(context, x++, y, (linemode << 4) | 12 | ((pixel >> 4) & 3));   \
    Set_pixel(context, x++, y, (linemode << 4) | ((pixel >> 2) & 3));        \
    Set_pixel(context, x++, y, (linemode << 4) | 4 | (pixel & 3));           \
  }                     \
  else                  \
  {                     \
    Set_pixel(context, x++, y, (linemode << 4) | (pixel >> 4));          \
    Set_pixel(context, x++, y, (linemode << 4) | (pixel & 15));          \
  }                     \
}

void Load_2GS(T_IO_Context * context)
{
  FILE * file;
  unsigned long file_size;
  dword chunksize;
  byte header[5];
  word mode;
  word width;
  word height;
  word palcount;
  word palindex;
  long lineoffset;
  long dataoffset;
  word y;
  byte bpp = 2;
  word multipalcount = 0;
  long multipaloffset = -1;
  long offset;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  if (!Read_dword_le(file, &chunksize))
    return;
  if (!Read_bytes(file, header, sizeof(header)))
    goto error;
  if (0 != memcmp(header, "\x04MAIN", 5))
    goto error;
  if (!Read_word_le(file, &mode) || !Read_word_le(file, &width) || !Read_word_le(file, &palcount))
    goto error;

  if (Config.Clear_palette)
    memset(context->Palette, 0, sizeof(T_Palette));
  for (palindex = 0; palindex < palcount; palindex++)
  {
    if (!Load_2GS_Palette(context->Palette + (palindex * 16), file))
      goto error;
  }
  if (!Read_word_le(file, &height))
    goto error;
  lineoffset = ftell(file);
  dataoffset = lineoffset + 4 * (long)height;
  GFX2_Log(GFX2_DEBUG, " mode %02x  %ux%u %u palettes\n", mode, (unsigned)width, (unsigned)height, (unsigned)palcount);

  // read other chunks before decoding the picture
  GFX2_Log(GFX2_DEBUG, "file_size : %06lx, chunksize : %06lx, current offset : %06lx\n", file_size, chunksize, dataoffset);
  offset = chunksize;
  while (offset < (long)file_size)
  {
    char * p;
    byte c;
    fseek(file, offset, SEEK_SET);
    if (!Read_dword_le(file, &chunksize) || !Read_byte(file, &c))
      goto error;
    p = GFX2_malloc(c + 1);
    if (p)
    {
      fread(p, 1, c, file);
      p[c] = '\0';
      GFX2_Log(GFX2_DEBUG, "%06lx: %04x %s\n", offset, chunksize, p);
      if (c == 4 && 0 == memcmp(p, "NOTE", 4))
      {
        word len;
        if (Read_word_le(file, &len))
        {
          free(p);
          p = GFX2_malloc(len + 1);
          if (p)
          {
            fread(p, 1, len, file);
            p[len] = '\0';
            GFX2_Log(GFX2_DEBUG, "%s\n", p);
          }
        }
      }
      else if (c == 12 && 0 == memcmp(p, "SuperConvert", 12))
      {
        dword len = chunksize - 4 - 13;
        free(p);
        p = GFX2_malloc(len);
        if (p)
        {
          fread(p, 1, len, file);
          GFX2_LogHexDump(GFX2_DEBUG, "", p, 0 /*offset + 4 + 13*/, (long)len);
        }
      }
      else if (c == 8 && 0 == memcmp(p, "MULTIPAL", 8))
      {
        if (Read_word_le(file, &multipalcount))
        {
          // all palettes are there...
          multipaloffset = ftell(file);
        }
      }
      free(p);
    }
    offset += chunksize;
  }

  if (multipalcount > 0)
  {
    bpp = 12;
  }
  else
  {
    while ((1 << bpp) < ((mode & 0x80) ? 4 : 16) * palcount)
      bpp++;
  }
  File_error = 0;
  Pre_load(context, width, height, file_size, FORMAT_2GS, (mode & 0x80) ? PIXEL_TALL : PIXEL_SIMPLE, bpp);
  if (File_error)
    goto error;
  for (y = 0; y < height; y++)
  {
    word linebytes;
    byte linemode;
    word x;
    T_Components linepal[16];
    if (multipalcount > y)
    {
      fseek(file, multipaloffset + 32 * y, SEEK_SET);
      if (!Load_2GS_Palette(linepal, file))
        goto error;
    }
    fseek(file, lineoffset, SEEK_SET);
    if (!Read_word_le(file, &linebytes) || !Read_byte(file, &linemode))
      goto error;
    //GFX2_Log(GFX2_DEBUG, "Line #%03u mode/pal=%02x %u bytes\n", (unsigned)y, linemode, (unsigned)linebytes);
    lineoffset += 4;
    fseek(file, dataoffset, SEEK_SET);
    if (multipalcount > 0)
    {
      linemode = 0;
    }
    for (x = 0; x < width; )
    {
      byte code, pixel;
      unsigned count;
      if (!Read_byte(file, &code))
        goto error;
      count = (code & 0x3f) + 1;
      switch (code >> 6)
      {
      case 0: // unpacked
        while (count-- > 0)
        {
          if (!Read_byte(file, &pixel))
            goto error;
          WRITE2GS_PIXELS
        }
        break;
      case 3: // byte packed x 4
        count <<= 2;
      case 1: // byte packed
        if (!Read_byte(file, &pixel))
          goto error;
        while (count-- > 0)
        {
          WRITE2GS_PIXELS
        }
        break;
      case 2: // dword packed
        {
          byte pixels[4];
          if (!Read_bytes(file, pixels, 4))
            goto error;
          while (count-- > 0)
          {
            int i;
            for (i = 0; i < 4; i++)
            {
              pixel = pixels[i];
              WRITE2GS_PIXELS
            }
          }
        }
        break;
      }
    }
    dataoffset += linebytes;
  }

  fclose(file);
  return;

error:
  File_error = 1;
  fclose(file);
}