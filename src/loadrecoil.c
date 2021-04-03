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
#ifdef _MSC_VER
#include <stdio.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#include "struct.h"
#include "global.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "errors.h"
#include "unicode.h"
#include "gfx2mem.h"
#include "gfx2log.h"

#include "recoil.h"

// RECOIL Vtable with our private data added
typedef struct {
	int (*readFile)(RECOIL *, const char *filename, unsigned char *content, int content_length);
  T_IO_Context *context;
}
RECOILVtbl;

static int RECOIL_ReadFile(RECOIL * recoil, const char * filename, unsigned char * content, int content_length)
{
  FILE * f;
  const char * ext;

  ext = strrchr(filename, '.');   // extract back extension from filename
  if (ext == NULL)
    return -1;
  ext++;
  f = Open_file_read_with_alternate_ext((*((const RECOILVtbl **)recoil))->context, ext);
  if (f == NULL)
    return -1;
  content_length = fread(content, 1, content_length, f);
  fclose(f);
  return content_length;
}

void Load_Recoil_Image(T_IO_Context *context)
{
  RECOILVtbl vtbl = { RECOIL_ReadFile, context };
  RECOIL *recoil;
  byte * file_content;
  unsigned long file_length;
  FILE * f;

  f = Open_file_read(context);
  if (f == NULL)
  {
    File_error = 1;
    return;
  }
  file_length = File_length_file(f);
  if (file_length == 0)
  {
    fclose(f);
    File_error = 1;
    return;
  }
  file_content = GFX2_malloc(file_length);
  if (file_content == NULL)
  {
    fclose(f);
    File_error = 1;
    return;
  }
  if (!Read_bytes(f, file_content, file_length))
  {
    GFX2_Log(GFX2_WARNING, "Load_Recoil_Image(): Read error on file\n");
    fclose(f);
    free(file_content);
    File_error = 1;
    return;
  }
  fclose(f);

  recoil = RECOIL_New();
  if (recoil)
  {
#ifdef WIN32
    char * tempfilename = NULL;
#endif
    const char * filename = context->File_name;
    *(const RECOILVtbl **)recoil = &vtbl; // set Vtable
#ifdef WIN32
    if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
    {
      // get the full file extension from the long filename (unicode)
      // RECOIL doesn't recognize file if the "short" extension is passed,
      // ie .DEE instead of .deep
      size_t i;
      size_t len = Unicode_strlen(context->File_name_unicode);
      tempfilename = (char *)GFX2_malloc(len + 1);
      for (i = 0; i < len; i++)
        tempfilename[i] = (context->File_name_unicode[i] < 256) ? (char)context->File_name_unicode[i] : '_';
      tempfilename[i] = '\0';
      filename = tempfilename;
    }
#endif
    if (RECOIL_Decode(recoil, filename, file_content, file_length))
    {
      int width, height;
      int original_width, original_height;
      int x, y;
      int x_ratio, y_ratio;
      const int *palette;
      enum PIXEL_RATIO ratio = PIXEL_SIMPLE;

      width = RECOIL_GetWidth(recoil);
      height = RECOIL_GetHeight(recoil);
      original_width = RECOIL_GetOriginalWidth(recoil);
      original_height = RECOIL_GetOriginalHeight(recoil);
      x_ratio = width / original_width;
      y_ratio = height / original_height;
      if (x_ratio > 1 && y_ratio == 1)
        ratio = PIXEL_WIDE;
      else if(x_ratio == 1 && y_ratio > 1)
        ratio = PIXEL_TALL;
      // try to convert to 8bpp image
      File_error = 0;
      palette = RECOIL_ToPalette(recoil);
      if (palette == NULL)
      {
        // 24bits
        const int * tc_pixels;

        Pre_load(context, original_width, original_height, file_length, FORMAT_MISC, ratio, 24);
        tc_pixels = RECOIL_GetPixels(recoil);
        for (y = 0; y < original_height; y++)
        {
          for (x = 0; x < original_width; x++)
          {
            Set_pixel_24b(context, x, y, *tc_pixels >> 16, *tc_pixels >> 8, *tc_pixels);
            tc_pixels += x_ratio;
          }
          tc_pixels += width * (y_ratio - 1);
        }
      }
      else
      {
        // 8bits
        int i;
        int bpp;
        int ncolors = RECOIL_GetColors(recoil);
        const uint8_t * pixels = RECOIL_GetIndexes(recoil);
        const byte * p = pixels;

        bpp = 8;
        while (ncolors <= (1 << (bpp - 1)))
          bpp--;
        if (Config.Clear_palette)
          memset(context->Palette,0,sizeof(T_Palette));
        for (i = 0; i < ncolors; i++)
        {
          context->Palette[i].R = (byte)(palette[i] >> 16);
          context->Palette[i].G = (byte)(palette[i] >> 8);
          context->Palette[i].B = (byte)(palette[i]);
        }
        Pre_load(context, original_width, original_height, file_length, FORMAT_MISC, ratio, bpp);
        for (y = 0; y < original_height; y++)
        {
          for (x = 0; x < original_width; x++)
          {
            Set_pixel(context, x, y, *p);
            p += x_ratio;
          }
          p += width * (y_ratio - 1);
        }
      }
      if (!File_error)
      {
        snprintf(context->Comment, COMMENT_SIZE + 1, "RECOIL: %s %d colors", RECOIL_GetPlatform(recoil), RECOIL_GetColors(recoil));
      }
    }
    RECOIL_Delete(recoil);
#ifdef WIN32
    free(tempfilename);
#endif
  }
  free(file_content);
}
