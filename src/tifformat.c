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

#ifndef __no_tifflib__

#ifdef _MSC_VER
#include <stdio.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#define fileno _fileno
#endif
#endif
#include <string.h>
#include <stdlib.h>
#include <tiffio.h>
#include "global.h"
#include "io.h"
#include "loadsave.h"
#include "gfx2log.h"

/**
 * @defgroup TIFF TIFF
 * @ingroup loadsaveformats
 * Tagged Image File Format
 *
 * Uses libtiff http://www.simplesystems.org/libtiff/
 *
 * @{
 */

/// GrafX2 private TIFF tag : 4 bytes
///
/// - bkg/transp color
/// - background transparent
/// - image mode
/// - reserved (0)
///
/// This TAG is read only if the Software tag begins with "GrafX2"
#define TIFFTAG_GRAFX2 65500

extern char Program_version[]; // generated in pversion.c
extern const char SVN_revision[]; // generated in version.c


static void TIFF_LogError(const char* module, const char* fmt, va_list ap)
{
  char format[256];
  snprintf(format, sizeof(format), "%s: %s\n", module, fmt);
  GFX2_LogV(GFX2_ERROR, format, ap);
}

static void TIFF_LogWarning(const char* module, const char* fmt, va_list ap)
{
  char format[256];
  snprintf(format, sizeof(format), "%s: %s\n", module, fmt);
  GFX2_LogV(GFX2_WARNING, format, ap);
}

/// old TIFF extender procedure to be recursively called
static TIFFExtendProc TIFFParentExtender = NULL;

/// Our TIFF Tag Extender procedure
///
/// Used to add ::TIFFTAG_GRAFX2 handling
static void GFX2_TIFFTagExtender(TIFF *tif)
{
  static const TIFFFieldInfo gfx2_fields[] = {
    // 4 bytes
    {TIFFTAG_GRAFX2, 4, 4, TIFF_BYTE, FIELD_CUSTOM, 1, 0, "GrafX2Private"}
  };
  TIFFMergeFieldInfo(tif, gfx2_fields, 1);
  if (TIFFParentExtender != NULL)
    (*TIFFParentExtender)(tif);
}

/// Initialisation for using the TIFF library
static void TIFF_Init(void)
{
  static int init_done = 0;

  if (init_done)
    return;

  /// use TIFFSetErrorHandler() and TIFFSetWarningHandler() to
  /// redirect warning/error output to our own functions
  TIFFSetErrorHandler(TIFF_LogError);
  TIFFSetWarningHandler(TIFF_LogWarning);
  TIFFParentExtender = TIFFSetTagExtender(GFX2_TIFFTagExtender);
  GFX2_Log(GFX2_DEBUG, "TIFF_Init() TIFFParentExtender=%p\n", TIFFParentExtender);

  init_done = 1;
}

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
  enum IMAGE_MODES mode = IMAGE_MODE_LAYERED;
  int layer = 0;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  dword width, height;
  word bps, spp;
  word photometric = PHOTOMETRIC_RGB;
  char * desc;
  char * software = NULL;
  float xresol, yresol;

#ifdef _DEBUG
  TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
#endif
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
  if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xresol) && TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yresol))
  {
    float ratiof = xresol / yresol;
    if (ratiof > 1.6f)
      ratio = PIXEL_TALL;
    else if (ratiof > 1.4f)
      ratio = PIXEL_TALL3;
    else if (ratiof < 0.7f)
      ratio = PIXEL_WIDE;
  }

  File_error = 0;
  Pre_load(context, width, height, file_size, FORMAT_TIFF, ratio, bps * spp);
  if (File_error != 0)
    return;

  if (TIFFGetField(tif, TIFFTAG_SOFTWARE, &software))
  {
    if (0 == memcmp(software, "GrafX2", 6)) // Check if the file was written by GrafX2
    {
      byte * grafx2_private; // bkg/transp color, background transparent, image mode, reserved (0)
      if (TIFFGetField(tif, TIFFTAG_GRAFX2, &grafx2_private))
      {
        GFX2_Log(GFX2_DEBUG, "bkg/transp color #%u, bkg transp %u, mode %u, reserved %u\n",
                 grafx2_private[0], grafx2_private[1], grafx2_private[2], grafx2_private[3]);
        context->Transparent_color = grafx2_private[0]; // need to be set after calling Pre_load()
        context->Background_transparent = grafx2_private[1];
        mode = grafx2_private[2];
      }
    }
  }
  Set_image_mode(context, (mode == IMAGE_MODE_ANIMATION) ? mode : IMAGE_MODE_LAYERED);
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
    word subifd_count;
#if TIFFLIB_VERSION < 20120101
    uint32 * subifd_array;
#else
    uint64 * subifd_array;
#endif
    if (TIFFGetField(tif, TIFFTAG_SUBIFD, &subifd_count, &subifd_array))
    {
      GFX2_Log(GFX2_DEBUG, "TIFFTAG_SUBIFD : count = %u\n", subifd_count);
    }
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
#ifdef _DEBUG
    TIFFPrintDirectory(tif, stdout, TIFFPRINT_NONE);
#endif
  }
  if (mode > IMAGE_MODE_ANIMATION)
    Set_image_mode(context, mode);
}

struct memory_buffer
{
  char * buffer;
  unsigned long offset;
  unsigned long size;
  unsigned long alloc_size;
};

tsize_t lTIFF_read(thandle_t p, void * data, tsize_t size)
{
  struct memory_buffer * mbuffer = (struct memory_buffer *)p;
  GFX2_Log(GFX2_DEBUG, "lTIFF_read(%p, %p, %u)\n", p, data, size);
  memcpy(data, mbuffer->buffer + mbuffer->offset, size);
  mbuffer->offset += size;
  return size;
}

tsize_t lTIFF_write(thandle_t p, void * data, tsize_t size)
{
  struct memory_buffer * mbuffer = (struct memory_buffer *)p;
  GFX2_Log(GFX2_DEBUG, "lTIFF_write(%p, %p, %u)\n", p, data, size);
  if (mbuffer->offset + size > mbuffer->alloc_size)
  {
    char * tmp = realloc(mbuffer->buffer, mbuffer->offset + size + 1024);
    if (tmp == NULL)
    {
      GFX2_Log(GFX2_ERROR, "lTIFF_write() failed to allocate %u bytes of memory\n", mbuffer->offset + size + 1024);
      return -1;
    }
    mbuffer->buffer = tmp;
    mbuffer->alloc_size = mbuffer->offset + size + 1024;
  }
  memcpy(mbuffer->buffer + mbuffer->offset, data, size);
  mbuffer->offset += size;
  if (mbuffer->offset > mbuffer->size)
    mbuffer->size = mbuffer->offset;
  return size;
}

toff_t lTIFF_seek(thandle_t p, toff_t offset, int whence)
{
  struct memory_buffer * mbuffer = (struct memory_buffer *)p;
  switch (whence)
  {
    case SEEK_SET:
      mbuffer->offset = offset;
      break;
    case SEEK_CUR:
      mbuffer->offset += offset;
      break;
    case SEEK_END:
      mbuffer->offset = mbuffer->size - offset;
      break;
    default:
      return -1;
  }
  GFX2_Log(GFX2_DEBUG, "lTIFF_seek(%p, %u, %d) new offset=%u (size=%u)\n",
           p, offset, whence, mbuffer->offset, mbuffer->size);
  if (mbuffer->offset > mbuffer->alloc_size)
  {
    char * tmp = realloc(mbuffer->buffer, mbuffer->offset + 1024);
    if (tmp == NULL)
    {
      GFX2_Log(GFX2_ERROR, "lTIFF_seek() failed to allocate %u bytes of memory\n", mbuffer->offset + 1024);
      return -1;
    }
    mbuffer->buffer = tmp;
    mbuffer->alloc_size = mbuffer->offset + 1024;
  }
  if (mbuffer->offset > mbuffer->size)
  {
    memset(mbuffer->buffer + mbuffer->size, 0, mbuffer->offset - mbuffer->size);
    GFX2_Log(GFX2_ERROR, "  seeking %d bytes after end of buffer, filling with 0s\n", mbuffer->offset - mbuffer->size);
    mbuffer->size = mbuffer->offset;
  }
  return mbuffer->offset;
}


toff_t lTIFF_size(thandle_t p)
{
  struct memory_buffer * mbuffer = (struct memory_buffer *)p;
  GFX2_Log(GFX2_DEBUG, "lTIFF_size(%p) = %u\n", p, mbuffer->size);
  return mbuffer->size;
}

int lTIFF_close(thandle_t p)
{
  (void)p;
  return 0;
}

int lTIFF_map(thandle_t p, void ** base, toff_t * size)
{
  struct memory_buffer * mbuffer = (struct memory_buffer *)p;
  GFX2_Log(GFX2_DEBUG, "lTIFF_map(%p, %p, %p)\n", p, base, size);
  *base = mbuffer->buffer;
  *size = mbuffer->size;
  return 1;
}

void lTIFF_unmap(thandle_t p, void *base, toff_t size)
{
  GFX2_Log(GFX2_DEBUG, "lTIFF_unmap(%p, %p, %u)\n", p, base, size);
  return;
}

/// Load TIFF from memory
void Load_TIFF_from_memory(T_IO_Context * context, const void * buffer, unsigned long size)
{
  TIFF * tif;
  struct memory_buffer memory_buffer;

  memory_buffer.buffer = (char *)buffer;
  memory_buffer.offset = 0;
  memory_buffer.size = size;
  memory_buffer.alloc_size = 0; // unused for read

  TIFF_Init();
  tif = TIFFClientOpen("memory.tiff", "r", &memory_buffer,
                       lTIFF_read, lTIFF_write, lTIFF_seek, lTIFF_close,
                       lTIFF_size, lTIFF_map, lTIFF_unmap);
  if (tif != NULL)
  {
    Load_TIFF_Sub(context, tif, size);
    TIFFClose(tif);
  }
}

/// Load TIFF from file
void Load_TIFF(T_IO_Context * context)
{
  TIFF * tif;
#if !defined(WIN32)
  FILE * file;

  File_error = 1;

  file = Open_file_read(context);
  if (file != NULL)
  {
    TIFF_Init();
    tif = TIFFFdOpen(fileno(file), context->File_name, "r");
    if (tif != NULL)
    {
      Load_TIFF_Sub(context, tif, File_length_file(file));
      TIFFClose(tif);
    }
    fclose(file);
  }
#else
  char * filename; // filename with full path

  File_error = 1;
  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  TIFF_Init();
  tif = TIFFOpen(filename, "r");
  if (tif != NULL)
  {
    Load_TIFF_Sub(context, tif, File_length(filename));
    TIFFClose(tif);
  }
  free(filename);
#endif
}


/// Save (already open) TIFF
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
  const word photometric = PHOTOMETRIC_PALETTE;
  float xresol = 1.0f, yresol = 1.0f;
  // bkg/transp color, background transparent, image mode, reserved (0)
  byte grafx2_private[4] = { context->Transparent_color, context->Background_transparent, 0, 0 };

  if (context->Type == CONTEXT_MAIN_IMAGE)
    grafx2_private[2] = Main.backups->Pages->Image_mode;

  switch (context->Ratio)
  {
    case PIXEL_WIDE:
    case PIXEL_WIDE2:
      yresol = 2.0f;
      break;
    case PIXEL_TALL:
    case PIXEL_TALL2:
      xresol = 2.0f;
      break;
    case PIXEL_TALL3:
      xresol = 4.0f;
      yresol = 3.0f;
      break;
    default:
      break;
  }

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
//TIFFTAG_SUBFILETYPE
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);
    //TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); // not relevant if SAMPLESPERPIXEL == 1
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
    if (context->Comment[0] != '\0')
      TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, context->Comment);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, version);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
    // RESUNIT_NONE : No absolute unit of measurement.
    // Used for images that may have a non-square aspect ratio,
    // but no meaningful absolute dimensions.
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, xresol);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, yresol);
    TIFFSetField(tif, TIFFTAG_COLORMAP, colormap.r, colormap.g, colormap.b);

//    TIFFTAG_SUBIFD  // point to thumbnail, etc.
//    TIFFSetField(tif, TIFFTAG_PAGENUMBER, current_page, page_count);

    // extensions :
    TIFFSetField(tif, TIFFTAG_GRAFX2, grafx2_private);


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

/// Save TIFF to memory
void Save_TIFF_to_memory(T_IO_Context * context, void * * buffer, unsigned long * size)
{
  TIFF * tif;
  struct memory_buffer memory_buffer;

  memory_buffer.buffer = NULL;
  memory_buffer.offset = 0;
  memory_buffer.size = 0;
  memory_buffer.alloc_size = 0;

  TIFF_Init();
  tif = TIFFClientOpen("memory.tiff", "w", &memory_buffer,
                       lTIFF_read, lTIFF_write, lTIFF_seek, lTIFF_close,
                       lTIFF_size, lTIFF_map, lTIFF_unmap);
  if (tif != NULL)
  {
    Save_TIFF_Sub(context, tif);
    TIFFClose(tif);
    *buffer = memory_buffer.buffer;
    *size = memory_buffer.size;
  }
}

/// Save TIFF
void Save_TIFF(T_IO_Context * context)
{
  TIFF * tif;

  File_error = 1;

  TIFF_Init();
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

/** @} */

#endif
