/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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

///@file tifformat.c
/// Support of TIFF
///
/// @todo use TIFFSetErrorHandler() and TIFFSetWarningHandler() to
///       redirect warning/error output to our own functions

#ifndef __no_tifflib__

#include <tiffio.h>
#include "global.h"
#include "io.h"
#include "loadsave.h"
#include "gfx2log.h"

extern char Program_version[]; // generated in pversion.c
extern const char SVN_revision[]; // generated in version.c

/// test for a valid TIFF
void Test_TIFF(T_IO_Context * context, FILE * file)
{
  char buffer[4];

  File_error = 1;
  if (!Read_bytes(file, buffer, 4))
    return;
  if (0 == memcmp(buffer, "MM\0*", 4) || 0 == memcmp(buffer, "II*\0", 4))
    File_error = 0;
}

/// Load current image in TIFF
static void Load_TIFF_image(T_IO_Context * context, TIFF * tif, word spp, word bps)
{
  tsize_t size;
  int x, y;
  unsigned int i, j;

  if (TIFFIsTiled(tif))
  {
    // Tiled image
    dword tile_width, tile_height;
    if (!TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width) || !TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height))
    {
      File_error = 2;
      return;
    }
    if (spp > 1 || bps > 8)
    {
      dword * buffer;
      dword x2, y2;

      buffer = malloc(sizeof(dword) * tile_width * tile_height);
      for (y = 0; y < context->Height; y += tile_height)
      {
        for (x = 0; x < context->Width; x += tile_width)
        {
          if (!TIFFReadRGBATile(tif, x, y, buffer))
          {
            free(buffer);
            File_error = 2;
            return;
          }
          j = 0;
          // Note that the raster is assume to be organized such that the pixel at
          // location (x,y) is raster[y*width+x]; with the raster origin in the
          // lower-left hand corner of the tile. That is bottom to top organization.
          // Edge tiles which partly fall off the image will be filled out with
          // appropriate zeroed areas.
          for (y2 = 0; y2 < tile_height; y2++)
          {
            int y_pos = y + tile_height - 1 - y2;
            for (x2 = 0; x2 < tile_width ; x2++)
            {
              Set_pixel_24b(context, x + x2, y_pos, TIFFGetR(buffer[j]), TIFFGetG(buffer[j]), TIFFGetB(buffer[j]));
              j++;
            }
          }
        }
      }
      free(buffer);
    }
    else
    {
      byte * buffer;
      //ttile_t tile;

      size = TIFFTileSize(tif);
      buffer = malloc(size);
      for (y = 0; y < context->Height; y += tile_height)
      {
        for (x = 0; x < context->Width; x += tile_width)
        {
          dword x2, y2;
          if (TIFFReadTile(tif, buffer, x, y, 0, 0) == -1)
          {
            free(buffer);
            File_error = 2;
            return;
          }
          j = 0;
          for (y2 = 0; y2 < tile_height; y2++)
          {
            for (x2 = 0; x2 < tile_width ; x2++)
            {
              Set_pixel(context, x + x2, y + y2, buffer[j]);
              j++;
            }
          }
        }
      }
      free(buffer);
    }
  }
  else
  {
    dword rows_per_strip = 0;
    tstrip_t strip, strip_count;
    // "Striped" image
    TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip);
    if (rows_per_strip == 0)
    {
      File_error = 2;
      return;
    }
    if (spp > 1 || bps > 8)
    {
      // if not 8bit with colormap, use TIFFReadRGBAStrip
      dword * buffer;

      strip_count = (context->Height + rows_per_strip - 1) / rows_per_strip;
      buffer = malloc(sizeof(dword) * rows_per_strip * context->Width);
      for (strip = 0, y = 0; strip < strip_count; strip++)
      {
        if (!TIFFReadRGBAStrip(tif, strip * rows_per_strip, buffer))
        {
          free(buffer);
          File_error = 2;
          return;
        }
        // 	Note that the raster is assume to be organized such that the
        // pixel at location (x,y) is raster[y*width+x]; with the raster
        // origin in the lower-left hand corner of the strip. That is bottom
        // to top organization. When reading a partial last strip in the
        // file the last line of the image will begin at the beginning of
        // the buffer.
        y = (strip + 1) * rows_per_strip - 1;
        if (y >= context->Height)
          y = context->Height - 1;
        for (i = 0, j = 0;
             i < rows_per_strip && y >= (int)(strip * rows_per_strip);
             i++, y--)
        {
          for (x = 0; x < context->Width; x++)
          {
            Set_pixel_24b(context, x, y, TIFFGetR(buffer[j]), TIFFGetG(buffer[j]), TIFFGetB(buffer[j]));
            j++;
          }
        }
      }
      free(buffer);
      return; // ignore next image for now...
    }
    else
    {
      byte * buffer = NULL;

      strip_count = TIFFNumberOfStrips(tif);
      size = TIFFStripSize(tif);
      GFX2_Log(GFX2_DEBUG, "TIFF %u strips of %u bytes\n", strip_count, size);
      buffer = malloc(size);
      for (strip = 0, y = 0; strip < strip_count; strip++)
      {
        tsize_t r = TIFFReadEncodedStrip(tif, strip, buffer, size);
        if (r == -1)
        {
          free(buffer);
          File_error = 2;
          return;
        }
        for (i = 0, j = 0; i < rows_per_strip && y < context->Height; i++, y++)
        {
          for (x = 0; x < context->Width; x++)
          {
            switch (bps)
            {
              case 8:
                Set_pixel(context, x, y, buffer[j++]);
                break;
              case 6: // 3 bytes => 4 pixels
                Set_pixel(context, x++, y, buffer[j] >> 2);
                if (x < context->Width)
                {
                  Set_pixel(context, x++, y, (buffer[j] & 3) << 4 | (buffer[j+1] & 0xf0) >> 4);
                  j++;
                  if (x < context->Width)
                  {
                    Set_pixel(context, x++, y, (buffer[j] & 0x0f) << 2 | (buffer[j+1] & 0xc0) >> 6);
                    j++;
                    Set_pixel(context, x, y, buffer[j] & 0x3f);
                  }
                }
                j++;
                break;
              case 4:
                Set_pixel(context, x++, y, buffer[j] >> 4);
                Set_pixel(context, x, y, buffer[j++] & 0x0f);
                break;
              case 2:
                Set_pixel(context, x++, y, buffer[j] >> 6);
                Set_pixel(context, x++, y, (buffer[j] >> 4) & 3);
                Set_pixel(context, x++, y, (buffer[j] >> 2) & 3);
                Set_pixel(context, x, y, buffer[j++] & 3);
                break;
              case 1:
                Set_pixel(context, x++, y, (buffer[j] >> 7) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 6) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 5) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 4) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 3) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 2) & 1);
                Set_pixel(context, x++, y, (buffer[j] >> 1) & 1);
                Set_pixel(context, x, y, buffer[j++] & 1);
                break;
              default:
                File_error = 2;
                GFX2_Log(GFX2_ERROR, "TIFF : %u bps unsupported\n", bps);
                free(buffer);
                return;
            }
          }
        }
      }
      free(buffer);
    }
  }
}


/// Load TIFF
void Load_TIFF_Sub(T_IO_Context * context, TIFF * tif, unsigned long file_size)
{
  int layer = 0;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  dword width, height;
  word bps, spp;
  word photometric = PHOTOMETRIC_RGB;
  char * desc;

  TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
  if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width) || !TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))
    return;
  if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps) || !TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp))
    return;
  GFX2_Log(GFX2_DEBUG, "TIFF #0 : %ux%u %ux%ubps\n", width, height, spp, bps);
  if (TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &desc))
  {
    size_t len = strlen(desc);
    if (len <= COMMENT_SIZE)
      memcpy(context->Comment, desc, len + 1);
    else
    {
      memcpy(context->Comment, desc, COMMENT_SIZE);
      context->Comment[COMMENT_SIZE] = '\0';
    }
  }
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  File_error = 0;
  Pre_load(context, width, height, file_size, FORMAT_TIFF, ratio, bps * spp);
  if (File_error != 0)
    return;
  if (spp == 1)
  {
    struct {
      word * r;
      word * g;
      word * b;
    } colormap;
    unsigned i, count;

    count = (bps <= 8) ? 1 << bps : 256;
    if (TIFFGetField(tif, TIFFTAG_COLORMAP, &colormap.r, &colormap.g, &colormap.b))
    {
      for (i = 0; i < count; i++)
      {
        context->Palette[i].R = colormap.r[i] >> 8;
        context->Palette[i].G = colormap.g[i] >> 8;
        context->Palette[i].B = colormap.b[i] >> 8;
      }
    }
    else
    {
      // Grayscale palette
      for (i = 0; i < count; i++)
      {
        unsigned int value = 255
          * (photometric == PHOTOMETRIC_MINISWHITE ? (count - 1 - i) : i)
          / (count - 1);
        context->Palette[i].R = value;
        context->Palette[i].G = value;
        context->Palette[i].B = value;
      }
    }
  }

  for (;;)
  {
    Load_TIFF_image(context, tif, spp, bps);
    if (File_error != 0)
      return;

    if (context->Type != CONTEXT_MAIN_IMAGE)
      return;
    if (!TIFFReadDirectory(tif))
      return;
    if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width) || !TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))
      return;
    if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps) || !TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp))
      return;
    layer++;
    GFX2_Log(GFX2_DEBUG, "TIFF #%d : %ux%u %ux%ubps\n", layer, width, height, spp, bps);
    if ((int)width != context->Width || (int)height != context->Height)
      return;
    if (context->bpp != (spp * bps))
      return;
    Set_loading_layer(context, layer);
    TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
  }
}

/// Load TIFF from file
void Load_TIFF(T_IO_Context * context)
{
  FILE * file;
  TIFF * tif;

  File_error = 1;

  file = Open_file_read(context);
  if (file != NULL)
  {
    tif = TIFFFdOpen(fileno(file), context->File_name, "r");
    if (tif != NULL)
    {
      Load_TIFF_Sub(context, tif, File_length_file(file));
      TIFFClose(tif);
    }
    fclose(file);
  }
}


void Save_TIFF_Sub(T_IO_Context * context, TIFF * tif)
{
  char version[64];
  int i;
  int layer = 0;
  dword width, height;
  dword y;
  tstrip_t strip;
  struct {
    word r[256];
    word g[256];
    word b[256];
  } colormap;
  const word bps = 8;
  const word spp = 1;
  const dword rowsperstrip = 64;

  snprintf(version, sizeof(version), "GrafX2 %s.%s", Program_version, SVN_revision);
  width = context->Width;
  height = context->Height;
  for (i = 0; i < 256; i++)
  {
    colormap.r[i] = 0x101 * context->Palette[i].R;
    colormap.g[i] = 0x101 * context->Palette[i].G;
    colormap.b[i] = 0x101 * context->Palette[i].B;
  }

  for (;;)
  {
    GFX2_Log(GFX2_DEBUG, "TIFF save layer #%d\n", layer);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    //TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); // not relevant if SAMPLESPERPIXEL == 1
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
    if (context->Comment[0] != '\0')
      TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, context->Comment);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, version);
    TIFFSetField(tif, TIFFTAG_COLORMAP, colormap.r, colormap.g, colormap.b);

#if 0
    for (y = 0; y < height; y++)
    {
      if (TIFFWriteScanline(tif, context->Target_address + y*context->Pitch, y, 0) < 0)
        return;
    }
#else
    for (y = 0, strip = 0; y < height; y += rowsperstrip, strip++)
    {
      if (TIFFWriteEncodedStrip(tif, strip, context->Target_address + y*context->Pitch, rowsperstrip * context->Pitch) < 0)
        return;
    }
#endif
    layer++;

    if (layer >= context->Nb_layers)
    {
      TIFFFlushData(tif);
      File_error = 0; // everything was fine
      return;
    }
    Set_saving_layer(context, layer);
    if (!TIFFWriteDirectory(tif))
      return;
    TIFFFlushData(tif);
  }
}

/// Save TIFF
void Save_TIFF(T_IO_Context * context)
{
  TIFF * tif;

  File_error = 1;

#if defined(WIN32)
  if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
    tif = TIFFOpenW(context->File_name_unicode, "w");
  else
#endif
    tif = TIFFOpen(context->File_name, "w");
  if (tif != NULL)
  {
    Save_TIFF_Sub(context, tif);
    TIFFClose(tif);
  }
}

#endif
