/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Petter Lindquist
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

///@file pngformat.c
/// Saving and loading of PNG file format

#ifndef __no_pnglib__
#ifdef __MINT__
#undef _GNU_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <png.h>
#if !defined(PNG_HAVE_PLTE)
#define PNG_HAVE_PLTE 0x02
#endif
#if (PNG_LIBPNG_VER_MAJOR <= 1) && (PNG_LIBPNG_VER_MINOR < 4)
  // Compatibility layer to allow us to use libng 1.4 or any older one.
  
  // This function is renamed in 1.4
  #define png_set_expand_gray_1_2_4_to_8(x) png_set_gray_1_2_4_to_8(x)
  
  // Wrappers that are mandatory in 1.4. Older version allowed direct access.
  #define png_get_rowbytes(png_ptr,info_ptr) ((info_ptr)->rowbytes)
  #define png_get_image_width(png_ptr,info_ptr) ((info_ptr)->width)
  #define png_get_image_height(png_ptr,info_ptr) ((info_ptr)->height)
  #define png_get_bit_depth(png_ptr,info_ptr) ((info_ptr)->bit_depth)
  #define png_get_color_type(png_ptr,info_ptr) ((info_ptr)->color_type)
#endif

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#include "loadsave.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "misc.h"
#include "gfx2log.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

//////////////////////////////////// PNG ////////////////////////////////////
/**
 * @defgroup PNG PNG format
 * @ingroup loadsaveformats
 * Portable Network Graphics
 *
 * We make use of libpng : http://www.libpng.org/pub/png/libpng.html
 * @{
 */

/// Test for PNG format
///
/// The 8 byte signature at the start of file is tested
void Test_PNG(T_IO_Context * context, FILE * file)
{
  byte png_header[8];
  
  (void)context;
  File_error=1;

  // Lecture du header du fichier
  if (Read_bytes(file,png_header,8))
  {
    if ( !png_sig_cmp(png_header, 0, 8))
      File_error=0;
  }
}

/// Callback to handle our private chunks
///
/// We have one private chunk at the moment :
/// - "crNg" which is similar to a CRNG chunk in an IFF file
static int PNG_read_unknown_chunk(png_structp ptr, png_unknown_chunkp chunk)
{
  T_IO_Context * context;
  // png_unknown_chunkp members:
  //    png_byte name[5];
  //    png_byte *data;
  //    png_size_t size;

  context = (T_IO_Context *)png_get_user_chunk_ptr(ptr);

  GFX2_Log(GFX2_DEBUG, "PNG private chunk '%s' :\n", chunk->name);
  GFX2_LogHexDump(GFX2_DEBUG, "", chunk->data, 0, chunk->size);
  
  if (!strcmp((const char *)chunk->name, "crNg"))
  {
    unsigned int i;
    const byte *chunk_ptr = chunk->data;
    
    // Should be a multiple of 6
    if (chunk->size % 6)
      return (-1);
    
    
    for(i=0;i<chunk->size/6 && i<16; i++)
    {
      word rate;
      word flags;
      byte min_col;
      byte max_col;
      
      // Rate (big-endian word)
      rate = *(chunk_ptr++) << 8;
      rate |= *(chunk_ptr++);
      
      // Flags (big-endian)
      flags = *(chunk_ptr++) << 8;
      flags |= *(chunk_ptr++);

      // Min color
      min_col = *(chunk_ptr++);
      // Max color
      max_col = *(chunk_ptr++);

      // Check validity
      if (min_col != max_col)
      {
        // Valid cycling range
        if (max_col<min_col)
          SWAP_BYTES(min_col,max_col)
        
          context->Cycle_range[i].Start=min_col;
          context->Cycle_range[i].End=max_col;
          context->Cycle_range[i].Inverse=(flags&2)?1:0;
          context->Cycle_range[i].Speed=(flags&1) ? rate/78 : 0;
                              
          context->Color_cycles=i+1;
      }
    }
  
    return (1); // >0 = success
  }
  return (0); /* did not recognize */
  
}


/// Private structure used in PNG_memory_read() and PNG_memory_write()
struct PNG_memory_buffer {
  char * buffer;
  unsigned long offset;
  unsigned long size;
};

/// read from memory buffer
static void PNG_memory_read(png_structp png_ptr, png_bytep p, png_size_t count)
{
  struct PNG_memory_buffer * buffer = (struct PNG_memory_buffer *)png_get_io_ptr(png_ptr);
  GFX2_Log(GFX2_DEBUG, "PNG_memory_read(%p, %p, %u) (io_ptr=%p)\n", png_ptr, p, count, buffer);
  if (buffer == NULL || p == NULL)
    return;
  if (buffer->offset + count <= buffer->size)
  {
    memcpy(p, buffer->buffer + buffer->offset, count);
    buffer->offset += count;
  }
  else
  {
    unsigned long available_count = buffer->size - buffer->offset;
    GFX2_Log(GFX2_DEBUG, "PNG_memory_read(): only %lu bytes available\n", available_count);
    if (available_count > 0)
    {
      memcpy(p, buffer->buffer + buffer->offset, available_count);
      buffer->offset += available_count;
    }
  }
}


/// Read PNG format file
void Load_PNG_Sub(T_IO_Context * context, FILE * file, const char * memory_buffer, unsigned long memory_buffer_size)
{
  png_structp png_ptr;
  png_infop info_ptr = NULL;

  // Prepare internal PNG loader
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr)
  {
    // Prepare internal PNG loader
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr)
    {
      png_byte color_type;
      png_byte bit_depth;
      byte bpp;
      struct PNG_memory_buffer buffer;

      // Setup a return point. If a pnglib loading error occurs
      // in this if(), the else will be executed.
      if (!setjmp(png_jmpbuf(png_ptr)))
      {
        // to read from memory, I need to use png_set_read_fn() instead of calling png_init_io()
        if (file != NULL)
          png_init_io(png_ptr, file);
        else
        {
          buffer.buffer = (char *)memory_buffer;
          buffer.offset = 8;  // skip header
          buffer.size = memory_buffer_size;
          png_set_read_fn(png_ptr, &buffer, PNG_memory_read);
        }
        // Inform pnglib we already loaded the header.
        png_set_sig_bytes(png_ptr, 8);

        // Hook the handler for unknown chunks
        png_set_read_user_chunk_fn(png_ptr, (png_voidp)context, &PNG_read_unknown_chunk);

        // Load file information
        png_read_info(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr,info_ptr);
        bit_depth = png_get_bit_depth(png_ptr,info_ptr);

        switch (color_type)
        {
          case PNG_COLOR_TYPE_GRAY_ALPHA:
            // no more than 8bpp or else we enable true color picture loading
            bpp = MIN(8, bit_depth * 2);
            break;
          case PNG_COLOR_TYPE_RGB:
            bpp = bit_depth * 3;
            break;
          case PNG_COLOR_TYPE_RGB_ALPHA:
            bpp = bit_depth * 4;
            break;
          case PNG_COLOR_TYPE_PALETTE:
          case PNG_COLOR_TYPE_GRAY:
          default:
            // no more than 8bpp or else we enable true color picture loading
            bpp = MIN(8, bit_depth);
        }
        GFX2_Log(GFX2_DEBUG, "PNG type=%u bit_depth=%u : %ubpp\n", color_type, bit_depth, bpp);

        // If it's any supported file
        // (Note: As of writing this, this test covers every possible
        // image format of libpng)
        if (color_type == PNG_COLOR_TYPE_PALETTE
            || color_type == PNG_COLOR_TYPE_GRAY
            || color_type == PNG_COLOR_TYPE_GRAY_ALPHA
            || color_type == PNG_COLOR_TYPE_RGB
            || color_type == PNG_COLOR_TYPE_RGB_ALPHA
           )
        {
          enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
          int num_text;
          png_text *text_ptr;

          int unit_type;
          png_uint_32 res_x;
          png_uint_32 res_y;

          // Comment (tEXt)
          context->Comment[0]='\0'; // Clear the previous comment
          if ((num_text=png_get_text(png_ptr, info_ptr, &text_ptr, NULL)))
          {
            while (num_text--)
            {
              int size = COMMENT_SIZE;
#ifdef PNG_iTXt_SUPPORTED
              size_t length = (text_ptr[num_text].compression >= 1) ? text_ptr[num_text].itxt_length : text_ptr[num_text].text_length;
#else
              size_t length = text_ptr[num_text].text_length;
              if (text_ptr[num_text].compression >= 1)
                continue; // skip iTXt
#endif
              if (length > 0 && length < COMMENT_SIZE)
                size = (int)length;
              GFX2_Log(GFX2_DEBUG, "PNG Text %d \"%s\" (%lu bytes): %.*s\n",
                       text_ptr[num_text].compression, text_ptr[num_text].key,
                       (unsigned long)length,
                       (int)MIN(length, 160), text_ptr[num_text].text);
              if (strcmp(text_ptr[num_text].key,"Title") == 0)
              {
                strncpy(context->Comment, text_ptr[num_text].text, size);
                context->Comment[size]='\0';
                break; // Skip all others tEXt chunks
              }
              else if(strcmp(text_ptr[num_text].key, "Comment") == 0)
              {
                strncpy(context->Comment, text_ptr[num_text].text, size);
                context->Comment[size]='\0';
                break; // Skip all others tEXt chunks
              }
            }
          }
          // Pixel Ratio (pHYs)
          if (png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type))
          {
            // Ignore unit, and use the X/Y ratio as a hint for
            // WIDE or TALL pixels
            if (res_x>0 && res_y>0)
            {
              GFX2_Log(GFX2_DEBUG, "PNG pHYs unit %d %dx%d\n", unit_type, res_x, res_y);
              if (unit_type == 1)
                GFX2_Log(GFX2_DEBUG, "    %dx%d DPI\n", (res_x * 254 + 5000) / 10000, (res_y * 254 + 5000) / 10000);
              if (res_y * 10 > res_x * 12)  // X/Y < 1/1.2
                ratio = PIXEL_WIDE;
              else if (res_x * 10 > res_y * 12) // X/Y > 1.2
                ratio = PIXEL_TALL;
            }
          }
          Pre_load(context,
                   png_get_image_width(png_ptr, info_ptr),
                   png_get_image_height(png_ptr, info_ptr),
                   file != NULL ? File_length_file(file) : memory_buffer_size,
                   FORMAT_PNG, ratio, bpp);

          if (File_error==0)
          {
            int x,y;
            png_colorp palette;
            int num_palette;
            png_bytep * Row_pointers = NULL;
            byte row_pointers_allocated = 0;
            int num_trans;
            png_bytep trans;
            png_color_16p trans_values;

            // 16-bit images
            if (bit_depth == 16)
            {
              // Reduce to 8-bit
              png_set_strip_16(png_ptr);
            }
            else if (bit_depth < 8)
            {
              // Inform libpng we want one byte per pixel,
              // even though the file was less than 8bpp
              png_set_packing(png_ptr);
            }

            // Images with alpha channel
            if (color_type & PNG_COLOR_MASK_ALPHA)
            {
              // Tell libpng to ignore it
              png_set_strip_alpha(png_ptr);
            }

            // Greyscale images :
            if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            {
              // Map low bpp greyscales to full 8bit (0-255 range)
              if (bit_depth < 8)
              {
#if (PNG_LIBPNG_VER_MAJOR <= 1) && (PNG_LIBPNG_VER_MINOR < 4)
                // Works well with png 1.2.8, but deprecated in 1.4 ...
                png_set_gray_1_2_4_to_8(png_ptr);
#else
                // ...where this seems to replace it:
                png_set_expand_gray_1_2_4_to_8(png_ptr);
#endif
              }

              // Create greyscale palette
              for (x=0;x<256;x++)
              {
                context->Palette[x].R=x;
                context->Palette[x].G=x;
                context->Palette[x].B=x;
              }
            }
            else if (color_type == PNG_COLOR_TYPE_PALETTE) // Palette images
            {
              if (bit_depth < 8)
              {
                // Clear unused colors
                if (Config.Clear_palette)
                  memset(context->Palette,0,sizeof(T_Palette));
              }
              // Get a pointer to the PNG palette
              png_get_PLTE(png_ptr, info_ptr, &palette,
                  &num_palette);
              // Copy all colors to the context
              for (x=0;x<num_palette;x++)
              {
                context->Palette[x].R=palette[x].red;
                context->Palette[x].G=palette[x].green;
                context->Palette[x].B=palette[x].blue;
              }
              // The palette must not be freed: it is owned by libpng.
              palette = NULL;
            }
            // Transparency (tRNS)
            if (png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values))
            {
              GFX2_Log(GFX2_DEBUG, "PNG transparency %d %p %p\n", num_trans, trans, trans_values);
              if (color_type == PNG_COLOR_TYPE_PALETTE && trans!=NULL)
              {
                int i;
                for (i=0; i<num_trans; i++)
                {
                  // Look for the first color with alpha = 0 (full transparency)
                  if (trans[i]==0)
                  {
                    context->Transparent_color = i;
                    context->Background_transparent = 1;
                    break;
                  }
                }
              }
              else if ((color_type == PNG_COLOR_TYPE_GRAY
                    || color_type == PNG_COLOR_TYPE_RGB) && trans_values!=NULL)
              {
                // In this case, num_trans is supposed to be "1",
                // and trans_values[0] contains the reference color
                // (RGB triplet) that counts as transparent.

                // Ideally, we should reserve this color in the palette,
                // (so it's not merged and averaged with a neighbor one)
                // and after creating the optimized palette, find its
                // index and mark it transparent.

                // Current implementation: ignore.
              }
            }

            png_set_interlace_handling(png_ptr); // return number of image passes (7 for interlaced images)
            png_read_update_info(png_ptr, info_ptr);

            // Allocate row pointers
            Row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * context->Height);
            row_pointers_allocated = 0;

            /* read file */
            if (!setjmp(png_jmpbuf(png_ptr)))
            {
              if (color_type == PNG_COLOR_TYPE_GRAY
                  ||  color_type == PNG_COLOR_TYPE_GRAY_ALPHA
                  ||  color_type == PNG_COLOR_TYPE_PALETTE
                 )
              {
                // 8bpp

                for (y=0; y<context->Height; y++)
                  Row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
                row_pointers_allocated = 1;

                png_read_image(png_ptr, Row_pointers);

                for (y=0; y<context->Height; y++)
                  for (x=0; x<context->Width; x++)
                    Set_pixel(context, x, y, Row_pointers[y][x]);
              }
              else
              {
                switch (context->Type)
                {
                  case CONTEXT_PREVIEW:
                    // 24bpp

                    // It's a preview
                    // Unfortunately we need to allocate loads of memory
                    for (y=0; y<context->Height; y++)
                      Row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
                    row_pointers_allocated = 1;

                    png_read_image(png_ptr, Row_pointers);

                    for (y=0; y<context->Height; y++)
                      for (x=0; x<context->Width; x++)
                        Set_pixel_24b(context, x, y, Row_pointers[y][x*3],Row_pointers[y][x*3+1],Row_pointers[y][x*3+2]);
                    break;
                  case CONTEXT_MAIN_IMAGE:
                  case CONTEXT_BRUSH:
                  case CONTEXT_SURFACE:
                    // It's loading an actual image
                    // We'll save memory and time by writing directly into
                    // our pre-allocated 24bit buffer
                    for (y=0; y<context->Height; y++)
                      Row_pointers[y] = (png_byte*) (&(context->Buffer_image_24b[y * context->Width]));
                    png_read_image(png_ptr, Row_pointers);
                    break;

                  case CONTEXT_PALETTE:
                  case CONTEXT_PREVIEW_PALETTE:
                    // No pixels to draw in a palette!
                    break;
                }
              }
            }
            else
              File_error=2;

            /* cleanup heap allocation */
            if (row_pointers_allocated)
            {
              for (y=0; y<context->Height; y++) {
                free(Row_pointers[y]);
                Row_pointers[y] = NULL;
              }

            }
            free(Row_pointers);
            Row_pointers = NULL;
          }
          else
            File_error=2;
        }
        else
          // Unsupported image type
          File_error=1;
      }
      else
        File_error=1;
    }
    else
      File_error=1;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  }
}


/// Read PNG format files
///
/// just read/test the header and call Load_PNG_Sub()
void Load_PNG(T_IO_Context * context)
{
  FILE *file;
  byte png_header[8];

  File_error=0;

  if ((file=Open_file_read(context)))
  {
    // Load header (8 first bytes)
    if (Read_bytes(file,png_header,8))
    {
      // Do we recognize a png file signature ?
      if ( !png_sig_cmp(png_header, 0, 8))
        Load_PNG_Sub(context, file, NULL, 0);
      else
        File_error=1;
    }
    else // Lecture header impossible: Error ne modifiant pas l'image
      File_error=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Error ne modifiant pas l'image
    File_error=1;
}


/// Write to memory buffer
static void PNG_memory_write(png_structp png_ptr, png_bytep p, png_size_t count)
{
  struct PNG_memory_buffer * buffer = (struct PNG_memory_buffer *)png_get_io_ptr(png_ptr);
  GFX2_Log(GFX2_DEBUG, "PNG_memory_write(%p, %p, %u) (io_ptr=%p)\n", png_ptr, p, count, buffer);
  if (buffer->size < buffer->offset + count)
  {
    char * tmp = realloc(buffer->buffer, buffer->offset + count + 1024);
    if (tmp == NULL)
    {
      GFX2_Log(GFX2_ERROR, "PNG_memory_write() Failed to allocate %u bytes of memory\n", buffer->offset + count + 1024);
      File_error = 1;
      return;
    }
    buffer->buffer = tmp;
    buffer->size = buffer->offset + count + 1024;
  }
  memcpy(buffer->buffer + buffer->offset, p, count);
  buffer->offset += count;
}

/// do nothing
static void PNG_memory_flush(png_structp png_ptr)
{
  struct PNG_memory_buffer * buffer = (struct PNG_memory_buffer *)png_get_io_ptr(png_ptr);
  GFX2_Log(GFX2_DEBUG, "PNG_memory_flush(%p) (io_ptr=%p)\n", png_ptr, buffer);
}


/// Save a PNG to file or memory
/// @param context the IO context
/// @param file the FILE to write to or NULL to write to memory
/// @param buffer will receive a malloc'ed buffer if writting to memory
/// @param buffer_size will receive the PNG size in memory
void Save_PNG_Sub(T_IO_Context * context, FILE * file, char * * buffer, unsigned long * buffer_size)
{
  static png_bytep * Row_pointers = NULL;
  int y;
  byte * pixel_ptr;
  png_structp png_ptr;
  png_infop info_ptr;
  png_unknown_chunk crng_chunk;
  byte cycle_data[16*6]; // Storage for color-cycling data, referenced by crng_chunk
  struct PNG_memory_buffer memory_buffer;

  assert((file != NULL) || ((buffer != NULL) && (buffer_size != NULL)));
  memset(&memory_buffer, 0, sizeof(memory_buffer));
  /* initialisation */
  if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))
      && (info_ptr = png_create_info_struct(png_ptr)))
  {
    if (!setjmp(png_jmpbuf(png_ptr)))
    {
      if (file != NULL)
        png_init_io(png_ptr, file);
      else // to write to memory, use png_set_write_fn() instead of calling png_init_io()
        png_set_write_fn(png_ptr, &memory_buffer, PNG_memory_write, PNG_memory_flush);

      /* read PNG header */
      if (!setjmp(png_jmpbuf(png_ptr)))
      {
        png_set_IHDR(png_ptr, info_ptr, context->Width, context->Height,
            8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_set_PLTE(png_ptr, info_ptr, (png_colorp)context->Palette, 256);
        {
          // text chunks in PNG (optional)
          png_text text_ptr[2] = {
#ifdef PNG_iTXt_SUPPORTED
            {-1, "Software", "Grafx2", 6, 0, NULL, NULL},
            {-1, "Title", NULL, 0, 0, NULL, NULL}
#else
            {-1, "Software", "Grafx2", 6},
            {-1, "Title", NULL, 0}
#endif
          };
          int nb_text_chunks=1;
          if (context->Comment[0])
          {
            text_ptr[1].text=context->Comment;
            text_ptr[1].text_length=strlen(context->Comment);
            nb_text_chunks=2;
          }
          png_set_text(png_ptr, info_ptr, text_ptr, nb_text_chunks);
        }
        if (context->Background_transparent)
        {
          // Transparency
          byte opacity[256];
          // Need to fill a segment with '255', up to the transparent color
          // which will have a 0. This piece of data (1 to 256 bytes)
          // will be stored in the file.
          memset(opacity, 255,context->Transparent_color);
          opacity[context->Transparent_color]=0;
          png_set_tRNS(png_ptr, info_ptr, opacity, (int)1 + context->Transparent_color,0);
        }
        // if using PNG_RESOLUTION_METER, unit is in dot per meter.
        // 72 DPI = 2835,  600 DPI = 23622
        // with PNG_RESOLUTION_UNKNOWN, it is arbitrary
        switch(Pixel_ratio)
        {
          case PIXEL_WIDE:
          case PIXEL_WIDE2:
            png_set_pHYs(png_ptr, info_ptr, 1, 2, PNG_RESOLUTION_UNKNOWN);
            break;
          case PIXEL_TALL:
          case PIXEL_TALL2:
            png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
            break;
          case PIXEL_TALL3:
            png_set_pHYs(png_ptr, info_ptr, 4, 3, PNG_RESOLUTION_UNKNOWN);
            break;
          default:
            break;
        }
        // Write cycling colors
        if (context->Color_cycles)
        {
          // Save a chunk called 'crNg'
          // The case is selected by the following rules from PNG standard:
          // char 1: non-mandatory = lowercase
          // char 2: private (not standard) = lowercase
          // char 3: reserved = always uppercase
          // char 4: can be copied by editors = lowercase

          // First, turn our nice structure into byte array
          // (just to avoid padding in structures)
            
          byte *chunk_ptr = cycle_data;
          int i;
            
          for (i=0; i<context->Color_cycles; i++)
          {
            word flags=0;
            flags|= context->Cycle_range[i].Speed?1:0; // Cycling or not
            flags|= context->Cycle_range[i].Inverse?2:0; // Inverted

            // Big end of Rate
            *(chunk_ptr++) = (context->Cycle_range[i].Speed*78) >> 8;
            // Low end of Rate
            *(chunk_ptr++) = (context->Cycle_range[i].Speed*78) & 0xFF;

            // Big end of Flags
            *(chunk_ptr++) = (flags) >> 8;
            // Low end of Flags
            *(chunk_ptr++) = (flags) & 0xFF;

            // Min color
            *(chunk_ptr++) = context->Cycle_range[i].Start;
            // Max color
            *(chunk_ptr++) = context->Cycle_range[i].End;
          }

          // Build one unknown_chuck structure
          memcpy(crng_chunk.name, "crNg",5);
          crng_chunk.data=cycle_data;
          crng_chunk.size=context->Color_cycles*6;
          crng_chunk.location=PNG_HAVE_PLTE;

          // Give it to libpng
          png_set_unknown_chunks(png_ptr, info_ptr, &crng_chunk, 1);
          // libpng seems to ignore the location I provided earlier.
          png_set_unknown_chunk_location(png_ptr, info_ptr, 0, PNG_HAVE_PLTE);
        }


        png_write_info(png_ptr, info_ptr);

        /* ecriture des pixels de l'image */
        Row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * context->Height);
        pixel_ptr = context->Target_address;
        for (y=0; y<context->Height; y++)
          Row_pointers[y] = (png_byte*)(pixel_ptr+y*context->Pitch);

        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          png_write_image(png_ptr, Row_pointers);

          /* cloture png */
          if (!setjmp(png_jmpbuf(png_ptr)))
          {
            png_write_end(png_ptr, NULL);
          }
          else
            File_error=1;
        }
        else
          File_error=1;
      }
      else
        File_error=1;
    }
    else
    {
      File_error=1;
    }
    png_destroy_write_struct(&png_ptr, &info_ptr);
  }
  else
    File_error=1;

  if (Row_pointers)
    free(Row_pointers);
  if (File_error == 0 && buffer != NULL)
  {
    *buffer = memory_buffer.buffer;
    if (buffer_size != NULL)
      *buffer_size = memory_buffer.offset;
  }
  else
    free(memory_buffer.buffer);
}


/// Save a PNG file
void Save_PNG(T_IO_Context * context)
{
  FILE *file;

  File_error = 0;

  file = Open_file_write(context);
  if (file != NULL)
  {
    Save_PNG_Sub(context, file, NULL, NULL);
    fclose(file);
    // remove the file if there was an error
    if (File_error)
      Remove_file(context);
  }
  else
    File_error = 1;
}
/** @} */

#endif  // __no_pnglib__
