/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018-2019 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Petter Lindquist
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2007-2011 Adrien Destugues
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

///@file c64formats.c
/// Formats for the Commodore 64

#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "engine.h"
#include "screen.h"
#include "windows.h"
#include "input.h"
#include "help.h"
#include "fileformats.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "misc.h"
#include "oldies.h"
#include "c64load.h"
#include "keycodes.h"
#include "packbits.h"
#include "gfx2mem.h"
#include "gfx2log.h"

//////////////////////////////////// C64 ////////////////////////////////////

/** C64 file formats
 */
enum c64_format
{
  F_invalid = -1,
  F_hires = 0,    ///< 320x200
  F_multi = 1,    ///< 160x200
  F_bitmap = 2,   ///< 320x200 monochrome
  F_fli = 3       ///< FLI (Flexible Line Interpretation)
};

/** C64 file formats names
 */
static const char *c64_format_names[] = {
  "Hires",
  "Multicolor",
  "Bitmap",
  "FLI"
};

static long C64_unpack_doodle(byte ** file_buffer, long file_size);

/**
 * Test for a C64 picture file
 *
 * Checks the file size and the load address
 *
 * References :
 * - http://unusedino.de/ec64/technical/formats/bitmap.html
 * - http://codebase64.org/doku.php?id=base:c64_grafix_files_specs_list_v0.03
 * - https://sourceforge.net/p/view64/code/HEAD/tree/trunk/libview64.c#l3737
 */
void Test_C64(T_IO_Context * context, FILE * file)
{
  unsigned long file_size;
  word load_addr;
  byte header[14];

  (void)context;
  File_error = 1;
  file_size = File_length_file(file);
  if (file_size < 16 || file_size > 48*1024)
    return; // File too short or too long, exit now
  // First test for formats without load address
  switch (file_size)
  {
    // case 1000: // screen or color
    case 8000: // raw bitmap
    case 9000: // bitmap + ScreenRAM
    case 10001: // multicolor
    case 17472: // FLI (BlackMail)
      File_error = 0;
      return;
    default: // then we don't know for now.
      if (!Read_word_le(file, &load_addr))
        return;
  }
  if (load_addr < 0x0400) // No PRG could load to Zeropage or processor stack
    return;
  GFX2_Log(GFX2_DEBUG, "Test_C64() file_size=%ld LoadAddr=$%04X\n", file_size, load_addr);
  if (!Read_bytes(file, header, sizeof(header)))
    return;
  if (memcmp(header, "DRAZPAINT", 9) == 0)
  {
    GFX2_Log(GFX2_DEBUG, "Test_C64() header=%.13s RLE code = $%02X\n", header, header[13]);
    File_error = 0;
    return;
  }
  // check last 2 bytes
  if (fseek(file, -2, SEEK_END) < 0)
    return;
  if (!Read_bytes(file, header, 2))
    return;
  if (load_addr == 0x4000 && header[0] == 0xC2 && header[1] == 0x00) // Amica Paint EOF mark
  {
    File_error = 0;
    return;
  }
  switch (file_size)
  {
    // case 1002: // (screen or color) + loadaddr
    case 8002: // raw bitmap with loadaddr
    case 9002: // bitmap + ScreenRAM + loadaddr
      // $4000 => InterPaint Hi-Res (.iph)
    case 9003: // bitmap + ScreenRAM + loadaddr (+ border ?)
    case 9009: // bitmap + ScreenRAM + loadaddr
      // $2000 => Art Studio
    case 9218:
      // $5C00 => Doodle
    case 9332:
      // $3F8E => Paint Magic (.pmg) 'JEDI' at offset $0010 and $2010
    case 10003: // multicolor + loadaddr
      // $4000 => InterPaint multicolor
      // $6000 => Koala Painter
    case 10004:
      // $4000 => Face Paint (.fpt)
    case 10006:
      // $6000 => Run Paint (.rpm)
    case 10018:
      // $2000 => Advanced Art Studio
    case 10022:
      // $18DC => Micro Illustrator (uncompressed)
    case 10050:
      // $1800 => Picasso64
    case 10218:
      // $3C00 => Image System (.ism)
    case 10219:
      // $7800 => Saracen Paint (.sar)
      File_error = 0;
      break;
    case 10242:
      // $4000 => Artist 64 (.a64)
      // $A000 => Blazing paddles (.pi)
      // $5C00 => Rainbow Painter (.rp)
      if (load_addr != 0x4000 && load_addr != 0xa000 && load_addr != 0x5c00)
      {
        File_error = 1;
        return;
      }
      File_error = 0;
      break;
    case 10608:
      // $0801 = BASIC programs loading address
      File_error = 0;
      break;
    case 17218:
    case 17409:
      // $3c00 => FLI-designer v1.1
      // ? $3ff0 => FLI designer 2 ?
    case 17410:
     // $3c00 => FLI MATIC
    case 17474: // FLI (BlackMail) + loadaddr
      // $3b00 => FLI Graph 2
    case 17665:
      // $3b00 => FLI editor
    case 17666:
      // $3b00 => FLI Graph
    case 10277: // multicolor CDU-Paint + loadaddr
      // $7EEF
      File_error = 0;
      break;
    default: // then we don't know for now.
      if (load_addr == 0x6000 || load_addr == 0x5c00)
      {
        long unpacked_size;
        byte * buffer = GFX2_malloc(file_size);
        if (buffer == NULL)
          return;
        fseek(file, SEEK_SET, 0);
        if (!Read_bytes(file, buffer, file_size))
          return;
        unpacked_size = C64_unpack_doodle(&buffer, file_size);
        free(buffer);
        switch (unpacked_size)
        {
          case 9024:  // Doodle hi color
          case 9216:
          case 10001: // Koala painter 2
          case 10070:
            File_error = 0;
        }
      }
  }
}

/**
 * Test for a C64 auto-load machine language program
 * which could be a picture
 */
void Test_PRG(T_IO_Context * context, FILE * file)
{
  unsigned long file_size;
  word load_addr;
  (void)context;

  file_size = File_length_file(file);
  if (file_size > (38911 + 2))  // maximum length of PRG loaded at $0801
    return;
  if (!Read_word_le(file, &load_addr))
    return;
  if (load_addr != 0x0801)
    return;
  // 6502 emulators :
  // https://github.com/redcode/6502
  // http://rubbermallet.org/fake6502.c
  // https://github.com/jamestn/cpu6502
  // https://github.com/dennis-chen/6502-Emu
  // https://github.com/DavidBuchanan314/6502-emu
  // basic program
  if (C64_isBinaryProgram(file) != 0)
    File_error = 0;
}

/**
 * Load C64 hires (320x200)
 *
 * @param context	the IO context
 * @param bitmap the bitmap RAM (8000 bytes)
 * @param screen_ram the screen RAM (1000 bytes)
 */
static void Load_C64_hires(T_IO_Context *context, byte *bitmap, byte *screen_ram)
{
  int cx,cy,x,y,c[4],pixel,color;

  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      if(screen_ram != NULL)
      {
        c[0]=screen_ram[cy*40+cx]&15;
        c[1]=screen_ram[cy*40+cx]>>4;
      }
      else
      { /// If screen_ram is NULL, uses default C64 basic colors
        c[0] = 6;
        c[1] = 14;
      }
      for(y=0; y<8; y++)
      {
        pixel=bitmap[cy*320+cx*8+y];
        for(x=0; x<8; x++)
        {
          color=c[pixel&(1<<(7-x))?1:0];
          Set_pixel(context, cx*8+x,cy*8+y,color);
        }
      }
    }
  }
}

/**
 * Load C64 multicolor (160x200)
 *
 * @param context	the IO context
 * @param bitmap the bitmap RAM (8000 bytes)
 * @param screen_ram the screen RAM (1000 bytes)
 * @param color_ram the color RAM (1000 bytes)
 * @param background the background color
 */
static void Load_C64_multi(T_IO_Context *context, byte *bitmap, byte *screen_ram, byte *color_ram, byte background)
{
    int cx,cy,x,y,c[4],pixel,color;
    c[0]=background&15;
    for(cy=0; cy<25; cy++)
    {
        for(cx=0; cx<40; cx++)
        {
            c[1]=screen_ram[cy*40+cx]>>4;
            c[2]=screen_ram[cy*40+cx]&15;
            c[3]=color_ram[cy*40+cx]&15;

            for(y=0; y<8; y++)
            {
                pixel=bitmap[cy*320+cx*8+y];
                for(x=0; x<4; x++)
                {
                    color=c[(pixel&3)];
                    pixel>>=2;
                    Set_pixel(context, cx*4+(3-x),cy*8+y,color);
                }
            }
        }
    }
}

/**
 * Loads a C64 FLI (Flexible Line Interpretation) picture.
 * Sets 4 layers :
 *  - Layer 0 : filled with background colors (1 per line)
 *  - Layer 1 : "Color RAM" 4x8 blocks
 *  - Layer 2 : pixels (From Screen RAMs + Bitmap)
 *  - Layer 3 : Transparency layer filled with color 16
 *
 * @param context the IO context
 * @param bitmap 8000 bytes buffer
 * @param screen_ram 8 x 1024 bytes buffers
 * @param color_ram 1000 byte buffer
 * @param background 200 byte buffer
 */
void Load_C64_fli(T_IO_Context *context, byte *bitmap, byte *screen_ram, byte *color_ram, byte *background)
{
  // Thanks to MagerValp for complement of specifications.
  //
  // background : length: 200 (+ padding 56)
  //    These are the BG colors for lines 0-199 (top to bottom)
  //        Low nybble: the color.
  //        High nybble: garbage. ignore it.
  // color_ram  : length: 1000 (+ padding 24)
  //    Color RAM. Contains one color per 4x8 block.
  //    There are 40x25 such blocks, arranged from top left to bottom
  //    right, starting in right direction. For each block there is one byte.
  //        Low nybble: the color.
  //        High nybble: garbage. ignore it.
  // screen_ram : length: 8192
  //    Screen RAMs. The s is important.
  //    This is actually 8 blocks of 1000 bytes, each separated by a filler of
  //    24 bytes. Each byte contains data for a 4x1 pixel group, and a complete
  //    block will contain 40x25 of them. 40 is from left to right, and 25 is from
  //    top to bottom, spacing them 8 lines apart.
  //    The second block start at y=1, the third block starts at y=2, etc...
  //    Each byte contains 2 colors that *can* be used by the 4x1 pixel group:
  //        Low nybble: Color 1
  //        High nybble: Color 2
  //
  // bitmap     : length: 8000
  //    This is the final structure that refers to all others. It describes
  //    160x200 pixels linearly, from top left to bottom right, starting in
  //    right direction. For each pixel, two bits say which color is displayed
  //    (So 4 pixels are described by the same byte)
  //        00 Use the BG color of the current line (background[y])
  //        01 Use the Color 2 from the current 4x8 block of Screen RAM
  //           ((screen_ram[y/8][x/4] & 0xF0) >> 8)
  //        10 Use the Color 1 from the current 4x8 block of Screen RAM
  //           (screen_ram[y/8][x/4] & 0x0F)
  //        11 Use the color from Color RAM
  //           (color_ram[y/8][x/4] & 0x0F)
  //

  int cx,cy,x,y,c[4];

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // Fill layer 0 with background colors
    for(y=0; y<200; y++)
    {
      byte bg_color = 0;
      if (background != NULL)
        bg_color = background[y];
      for(x=0; x<160; x++)
        Set_pixel(context, x,y, bg_color);
    }

    // Fill layer 1 with color ram (1 color per 4x8 block)
    Set_loading_layer(context, 1);
    for(cy=0; cy<25; cy++)
    {
      for(cx=0; cx<40; cx++)
      {
        c[3]=color_ram[cy*40+cx]&15;
        for(y=0; y<8; y++)
        {
          for(x=0; x<4; x++)
          {
            Set_pixel(context, cx*4+x,cy*8+y,c[3]);
          }
        }
      }
    }
  }

  // Layer 2 are actual pixels
  Set_loading_layer(context, 2);
  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      c[3]=color_ram[cy*40+cx]&15;
      for(y=0; y<8; y++)
      {
        int pixel=bitmap[cy*320+cx*8+y];

        c[0] = 0;
        if(background != NULL)
          c[0] = background[cy*8+y]&15;
        c[1]=screen_ram[y*1024+cy*40+cx]>>4;
        c[2]=screen_ram[y*1024+cy*40+cx]&15;
        for(x=0; x<4; x++)
        {
          int color=c[(pixel&3)];
          pixel>>=2;
          Set_pixel(context, cx*4+(3-x),cy*8+y,color);
        }
      }
    }
  }
  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // Fill layer 3 with color 16
    Set_loading_layer(context, 3);
    for(y=0; y<200; y++)
    {
      for(x=0; x<160; x++)
        Set_pixel(context, x,y,16);
    }
  }
}

/**
 * Count the length of the unpacked data
 *
 * RLE encoding is either ESCAPE CODE, COUNT, VALUE
 * or ESCAPE CODE, VALUE, COUNT
 *
 * @param buffer the packed data
 * @param input_size the packed data byte count
 * @param RLE_code the escape code
 * @param order 0 for ESCAPE, COUNT, VALUE, 1 for ESCAPE, VALUE, COUNT
 * @return the unpacked data byte count
 */
static long C64_unpack_get_length(const byte * buffer, long input_size, byte RLE_code, int order)
{
  const byte * end;
  long unpacked_size = 0;

  end = buffer + input_size;
  while(buffer < end)
  {
    if (*buffer == RLE_code)
    {
      if (order)
      { // ESCAPE, VALUE, COUNT
        buffer += 2;  // skip value
        unpacked_size += *buffer;
      }
      else
      { // ESCAPE, COUNT, VALUE
        buffer++;
        if (*buffer == 0)
          break;
        unpacked_size += *buffer++;
      }
    }
    else
      unpacked_size++;
    buffer++;
  }
  return unpacked_size;
}

/**
 * unpack RLE packed data
 *
 * RLE encoding is either ESCAPE CODE, COUNT, VALUE
 * or ESCAPE CODE, VALUE, COUNT
 *
 * @param unpacked buffer to received unpacked data
 * @param buffer the packed data
 * @param input_size the packed data byte count
 * @param RLE_code the escape code
 * @param order 0 for ESCAPE, COUNT, VALUE, 1 for ESCAPE, VALUE, COUNT
 */
static void C64_unpack(byte * unpacked, const byte * buffer, long input_size, byte RLE_code, int order)
{
  const byte * end;

  end = buffer + input_size;
  while(buffer < end)
  {
    if (*buffer == RLE_code)
    {
      byte count;
      byte value;
      buffer++;
      if (order)
      { // ESCAPE, VALUE, COUNT
        value = *buffer++;
        count = *buffer;
      }
      else
      { // ESCAPE, COUNT, VALUE
        count = *buffer++;
        value = *buffer;
      }
      if (count == 0)
        break;
      while (count-- > 0)
        *unpacked++ = value;
    }
    else
      *unpacked++ = *buffer;
    buffer++;
  }
}

/**
 * Unpack the Amica Paint RLE packing
 *
 * @param[in,out] file_buffer will contain the unpacked buffer on return
 * @param[in] file_size packed buffer size
 * @return the unpacked data size or -1 in case of error
 *
 * Ref:
 * - http://codebase64.org/doku.php?id=base:c64_grafix_files_specs_list_v0.03
 */
static long C64_unpack_amica(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  const byte RLE_code = 0xC2;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  unpacked_size = C64_unpack_get_length(*file_buffer + 2, file_size - 2, RLE_code, 0);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_amica() unpacked_size=%ld\n", unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = GFX2_malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 2, file_size - 2, RLE_code, 0);

  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Unpack the DRAZPAINT RLE packing
 *
 * @param[in,out] file_buffer will contain the unpacked buffer on return
 * @param[in] file_size packed buffer size
 * @return the unpacked data size or -1 in case of error
 *
 * Ref:
 * - https://www.godot64.de/german/l_draz.htm
 * - https://sourceforge.net/p/view64/code/HEAD/tree/trunk/libview64.c#l2805
 */
static long C64_unpack_draz(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  byte RLE_code;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  RLE_code = (*file_buffer)[15];
  // First pass to know unpacked size
  unpacked_size = C64_unpack_get_length(*file_buffer + 16, file_size - 16, RLE_code, 0);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_draz() \"%.13s\" RLE code=$%02X RLE data length=%ld unpacked_size=%ld\n",
           *file_buffer + 2, RLE_code, file_size - 16, unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = GFX2_malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 16, file_size - 16, RLE_code, 0);
  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Unpack doodle/koala painter 2 data
 *
 * @return the unpacked data size or -1 in case of error
 */
static long C64_unpack_doodle(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  const byte RLE_code = 0xFE;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  // First pass to know unpacked size
  unpacked_size = C64_unpack_get_length(*file_buffer + 2, file_size - 2, RLE_code, 1);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_doodle() unpacked_size=%ld\n", unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = GFX2_malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 2, file_size - 2, RLE_code, 1);
  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Load C64 pictures formats.
 *
 * Supports:
 * - Hires (with or without ScreenRAM)
 * - Multicolor (Koala or CDU-paint format)
 * - FLI
 *
 * see http://unusedino.de/ec64/technical/formats/bitmap.html
 *
 * @param context the IO context
 */
void Load_C64(T_IO_Context * context)
{
    FILE* file;
    long file_size;
    byte hasLoadAddr=0;
    word load_addr;
    enum c64_format loadFormat = F_invalid;

    byte *file_buffer;
    byte *bitmap, *screen_ram, *color_ram=NULL, *background=NULL; // Only pointers to existing data
    byte *temp_buffer = NULL;
    word width, height=200;

    file = Open_file_read(context);

    if (file)
    {
        File_error=0;
        file_size = File_length_file(file);

        // Load entire file in memory
        file_buffer = GFX2_malloc(file_size);
        if (!file_buffer)
        {
            File_error = 1;
            fclose(file);
            return;
        }
        if (!Read_bytes(file,file_buffer,file_size))
        {
            File_error = 1;
            free(file_buffer);
            fclose(file);
            return;
        }
        fclose(file);

        // get load address (valid only if hasLoadAddr = 1)
        load_addr = file_buffer[0] | (file_buffer[1] << 8);

        // Unpack if needed
        if (memcmp(file_buffer + 2, "DRAZPAINT", 9) == 0)
          file_size = C64_unpack_draz(&file_buffer, file_size);
        else if(load_addr == 0x4000 && file_buffer[file_size-2] == 0xC2 && file_buffer[file_size-1] == 0)
          file_size = C64_unpack_amica(&file_buffer, file_size);
        else if (file_size < 8000 && (load_addr == 0x6000 || load_addr == 0x5c00))
          file_size = C64_unpack_doodle(&file_buffer, file_size);

        switch (file_size)
        {
            case 8000: // raw bitmap
                hasLoadAddr=0;
                loadFormat=F_bitmap;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=NULL;
                break;

            case 8002: // raw bitmap with loadaddr
                hasLoadAddr=1;
                loadFormat=F_bitmap;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=NULL;
                break;

            case 9000: // bitmap + ScreenRAM
                hasLoadAddr=0;
                loadFormat=F_hires;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=file_buffer+8000; // length: 1000
                break;

            case 9003: // bitmap + ScreenRAM + loadaddr (+ border ?)
            case 9002: // bitmap + ScreenRAM + loadaddr
                hasLoadAddr=1;
                loadFormat=F_hires;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                break;

            case 9009: // Art Studio (.aas)
                hasLoadAddr=1;
                loadFormat=F_hires;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                break;

            case 9024:  // Doodle (unpacked from .jj)
            case 9216:
                hasLoadAddr=0;
                loadFormat=F_hires;
                screen_ram=file_buffer; // length: 1000 (+24 padding)
                bitmap=file_buffer+1024; // length: 8000
                break;

            case 9218: // Doodle (.dd)
                hasLoadAddr=1;
                loadFormat=F_hires;
                screen_ram=file_buffer+2; // length: 1000 (+24 padding)
                bitmap=file_buffer+1024+2; // length: 8000
                break;

            case 9332:  // Paint Magic .pmg
                hasLoadAddr=1;
                loadFormat=F_multi;
                // Display routine between offset $0002 and $0073 (114 bytes)
                // duplicated between offset      $2002 and $2073
                bitmap=file_buffer+114+2;         // $0074
                background=file_buffer+8000+114+2;// $1FB4
                temp_buffer = GFX2_malloc(1000);
                memset(temp_buffer, file_buffer[3+8000+114+2], 1000); // color RAM Byte
                color_ram=temp_buffer;
                //border  byte = file_buffer[4+8000+114+2];
                screen_ram=file_buffer+8192+114+2;  // $2074
                break;

            case 10001: // multicolor
            case 10070: // unpacked file.
                hasLoadAddr=0;
                loadFormat=F_multi;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=file_buffer+8000; // length: 1000
                color_ram=file_buffer+9000; // length: 1000
                background=file_buffer+10000; // only 1
                break;

            case 10003: // multicolor + loadaddr
            case 10004: // extra byte is border color
            case 10006: // Run Paint
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                color_ram=file_buffer+9002; // length: 1000
                background=file_buffer+10002; // only 1
                break;

            case 10018: // Advanced Art Studio (.ocp) + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8000+2; // length: 1000
                color_ram=file_buffer+9016+2; // length: 1000
                // filebuffer+9000+2 is border
                background=file_buffer+9001+2; // only 1
                break;

            case 10022: // Micro Illustrator (.mil)
                hasLoadAddr=1;
                loadFormat=F_multi;
                screen_ram=file_buffer+20+2;
                color_ram=file_buffer+1000+20+2;
                bitmap=file_buffer+2*1000+20+2;
                break;

            case 10049: // unpacked DrazPaint
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer; // length: 1000 + (padding 24)
                screen_ram=file_buffer+1024; // length: 1000 + (padding 24)
                bitmap=file_buffer+1024*2; // length: 8000
                background=file_buffer+8000+1024*2;
                break;

            case 10050: // Picasso64 multicolor + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer+2; // length: 1000 + (padding 24)
                screen_ram=file_buffer+1024+2; // length: 1000 + (padding 24)
                bitmap=file_buffer+1024*2+2; // length: 8000
                background=file_buffer+1024*2+2-1; // only 1
                break;

            case 10218: // Image System
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer+2; // Length: 1000 (+ padding 24)
                bitmap=file_buffer+1024+2; // Length: 8000 (+padding 192)
                screen_ram=file_buffer+8192+1024+2;  // Length: 1000 (no padding)
                background=file_buffer+8192+1024+2-1; // only 1
                break;

            case 10219: // Saracen Paint (.sar)
                hasLoadAddr=1;
                loadFormat=F_multi;
                screen_ram=file_buffer+2;  // Length: 1000 (+ padding24)
                background=file_buffer+1008+2; // offset 0x3F0 (only 1 byte)
                bitmap=file_buffer+1024+2; // Length: 8000 (+padding 192)
                color_ram=file_buffer+8192+1024+2; // Length: 1000 (+ padding 24)
                break;

            case 10242: // Artist 64/Blazing Paddles/Rainbow Painter multicolor + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                switch(load_addr)
                {
                  default:
                  case 0x4000:  // Artist 64
                    bitmap=file_buffer+2; // length: 8000 (+padding 192)
                    screen_ram=file_buffer+8192+2; // length: 1000 + (padding 24)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer+1024*2+8192+2-1; // only 1
                    break;
                  case 0xa000:  // Blazing Paddles
                    bitmap=file_buffer+2; // length: 8000 (+padding 192)
                    screen_ram=file_buffer+8192+2; // length: 1000 + (padding 24)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer+8064+2; // only 1
                    break;
                  case 0x5c00:  // Rainbow Painter
                    screen_ram=file_buffer+2; // length: 1000 + (padding 24)
                    bitmap=file_buffer+1024+2; // length: 8000 (+padding 192)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer; // only 1
                    break;
                }
                break;

            case 10257: // unpacked Amica Paint (.ami)
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer; // length 8000
                screen_ram=file_buffer+8000;  // length: 1000
                color_ram=file_buffer+1000+8000;// length:1000
                background=file_buffer+2*1000+8000;//1
                // remaining bytes (offset 10001, length 256) are a "Color Rotation Table"
                // we should decode if we learn its format...
                break;

            case 10277: // multicolor CDU-Paint + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                // 273 bytes of display routine
                bitmap=file_buffer+275; // length: 8000
                screen_ram=file_buffer+8275; // length: 1000
                color_ram=file_buffer+9275; // length: 1000
                background=file_buffer+10275; // only 1
                break;

            case 10608: // prg
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap = file_buffer + 0x239;
                // border = bitmap + 8000
                background = bitmap + 8000 + 1;
                screen_ram = bitmap + 8000 + 2;
                color_ram = screen_ram + 1000;
                break;

            case 17472: // FLI (BlackMail)
                hasLoadAddr=0;
                loadFormat=F_fli;
                background=file_buffer+0; // length: 200 (+ padding 56)
                color_ram=file_buffer+256; // length: 1000 (+ padding 24)
                screen_ram=file_buffer+1280; // length: 8192
                bitmap=file_buffer+9472; // length: 8000
                break;

            case 17474: // FLI (BlackMail) + loadaddr
                hasLoadAddr=1;
                loadFormat=F_fli;
                background=file_buffer+2; // length: 200 (+ padding 56)
                color_ram=file_buffer+258; // length: 1000 (+ padding 24)
                screen_ram=file_buffer+1282; // length: 8192
                bitmap=file_buffer+9474; // length: 8000
                break;

            case 17218:
            case 17409: // FLI-Designer v1.1 (+loadaddr)
            case 17410: // => FLI MATIC (background at 2+1024+8192+8000+65 ?)
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=NULL;
              color_ram=file_buffer+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+2; // length: 8192
              bitmap=file_buffer+8192+1024+2; // length: 8000
              break;

            case 17666: // FLI Graph
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=file_buffer+2;
              color_ram=file_buffer+256+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+256+2; // length: 8192
              bitmap=file_buffer+8192+1024+256+2; // length: 8000
              break;

            case 17665: // FLI Editor
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=file_buffer+8;
              color_ram=file_buffer+256+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+256+2; // length: 8192
              bitmap=file_buffer+8192+1024+256+2; // length: 8000
              break;

            default:
                File_error = 1;
                free(file_buffer);
                return;
        }

        if (loadFormat == F_invalid)
        {
          File_error = 1;
          free(file_buffer);
          return;
        }

        if (loadFormat == F_fli || loadFormat == F_multi)
        {
          context->Ratio = PIXEL_WIDE;
          width = 160;
        }
        else
        {
          context->Ratio = PIXEL_SIMPLE;
          width = 320;
        }

        // Write detailed format in comment
        if (hasLoadAddr)
          snprintf(context->Comment,COMMENT_SIZE+1,"%s, load at $%4.4X",c64_format_names[loadFormat],load_addr);
        else
          snprintf(context->Comment,COMMENT_SIZE+1,"%s, no addr",c64_format_names[loadFormat]);

        Pre_load(context, width, height, file_size, FORMAT_C64, context->Ratio, (loadFormat == F_bitmap) ? 1 : 4); // Do this as soon as you can

        if (Config.Clear_palette)
          memset(context->Palette,0, sizeof(T_Palette));
        C64_set_palette(context->Palette);
        context->Transparent_color=16;

        switch(loadFormat)
        {
          case F_fli:
            Load_C64_fli(context,bitmap,screen_ram,color_ram,background);
            Set_image_mode(context, IMAGE_MODE_C64FLI);
            break;
          case F_multi:
            Load_C64_multi(context,bitmap,screen_ram,color_ram,
                           (background==NULL) ? 0 : *background);
            Set_image_mode(context, IMAGE_MODE_C64MULTI);
            break;
          default:
            Load_C64_hires(context,bitmap,screen_ram);
            if (loadFormat == F_hires)
              Set_image_mode(context, IMAGE_MODE_C64HIRES);
        }

        free(file_buffer);
        if (temp_buffer)
          free(temp_buffer);
    }
    else
        File_error = 1;
}

/**
 * Load C64 autoload pictures
 *
 * @param context the IO context
 */
void Load_PRG(T_IO_Context * context)
{
  FILE* file;
  unsigned long file_size;
  struct c64state c64;
  enum c64_format loadFormat = F_invalid;
  word load_addr;
  word width, height = 200;

  memset(&c64, 0, sizeof(c64));

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  if (!Read_word_le(file, &load_addr))
    return;
  if (load_addr == 0x801)
  {
    word start_addr = C64_isBinaryProgram(file);
    if (start_addr == 0)
      return;
    if (fseek(file, 2, SEEK_SET) < 0)
      return;
    if (C64_LoadPrg(&c64, file, start_addr))
    {
      File_error = 0;
      if (c64.vicmode & C64_VICMODE_FLI)
        loadFormat = F_fli;
      else if (c64.vicmode & C64_VICMODE_MULTI)
        loadFormat = F_multi;
      else
        loadFormat = F_hires;

      if (loadFormat == F_fli || loadFormat == F_multi)
      {
        context->Ratio = PIXEL_WIDE;
        width = 160;
      }
      else
      {
        context->Ratio = PIXEL_SIMPLE;
        width = 320;
      }

      Pre_load(context, width, height, file_size, FORMAT_PRG, context->Ratio, 4); // Do this as soon as you can

      if (Config.Clear_palette)
        memset(context->Palette, 0, sizeof(T_Palette));
      C64_set_palette(context->Palette);
      context->Transparent_color = 16;

      switch(loadFormat)
      {
        case F_fli:
          Load_C64_fli(context, c64.ram + c64.bitmap, c64.ram + c64.screen, c64.ram + 0xd800, c64.backgrounds);
          Set_image_mode(context, IMAGE_MODE_C64FLI);
          break;
        case F_multi:
          Load_C64_multi(context, c64.ram + c64.bitmap, c64.ram + c64.screen, c64.ram + 0xd800, c64.ram[0xd021]);
          Set_image_mode(context, IMAGE_MODE_C64MULTI);
          break;
        default:
          Load_C64_hires(context, c64.ram + c64.bitmap, c64.ram + c64.screen);
          if (loadFormat == F_hires)
            Set_image_mode(context, IMAGE_MODE_C64HIRES);
      }
    }
    if (c64.ram != NULL)
      free(c64.ram);
  }
}

/**
 * Display the dialog for C64 save parameters
 *
 * @param[in,out] saveFormat one of the C64 mode from @ref c64_format
 * @param[in,out] saveWhat 0=All, 1=Only bitmap, 2=Only Screen RAM, 3=Only color RAM
 * @param[in,out] loadAddr actual load address or 0 for "None"
 * @return true to proceed, false to abort
 */
static int Save_C64_window(enum c64_format *saveFormat, byte *saveWhat, word *loadAddr)
{
  int button;
  unsigned int i;
  T_Dropdown_button *what, *addr;
  T_Dropdown_button *format;
  static const char * what_label[] = {
    "All",
    "Bitmap",
    "Screen",
    "Color"
  };
  static const char * address_label[] = {
    "None",
    "$2000",
    "$4000",
    "$6000",
    "$8000",
    "$A000",
    "$C000",
    "$E000"
  };
  // default addresses :
  //  - FLI Fli Graph 2 (BlackMail) => $3b00
  //  - multicolor (Koala Painter) => $6000
  //  - hires (InterPaint) => $4000

  Open_window(200,120,"C64 saving settings");
  Window_set_normal_button(110,100,80,15,"Save",1,1,KEY_RETURN); // 1
  Window_set_normal_button(10,100,80,15,"Cancel",1,1,KEY_ESCAPE); // 2

  Print_in_window(13,18,"Data:",MC_Dark,MC_Light);
  what = Window_set_dropdown_button(10,28,90,15,70,what_label[*saveWhat],1, 0, 1, LEFT_SIDE,0); // 3
  Window_dropdown_clear_items(what);
  for (i=0; i<sizeof(what_label)/sizeof(what_label[0]); i++)
    Window_dropdown_add_item(what,i,what_label[i]);

  Print_in_window(113,18,"Address:",MC_Dark,MC_Light);
  addr = Window_set_dropdown_button(110,28,70,15,70,address_label[*loadAddr/0x2000],1, 0, 1, LEFT_SIDE,0); // 4
  Window_dropdown_clear_items(addr);
  for (i=0; i<sizeof(address_label)/sizeof(address_label[0]); i++)
    Window_dropdown_add_item(addr,i,address_label[i]);

  Print_in_window(13,46,"Format:",MC_Dark,MC_Light);
  format = Window_set_dropdown_button(10,56,90,15,88,c64_format_names[*saveFormat],1, 0, 1, LEFT_SIDE,0); // 5
  if (*saveFormat == F_hires || *saveFormat == F_bitmap)
  {
    Window_dropdown_add_item(format, F_hires, c64_format_names[F_hires]);
    Window_dropdown_add_item(format, F_bitmap, c64_format_names[F_bitmap]);
  }
  else
  {
    Window_dropdown_add_item(format, F_multi, c64_format_names[F_multi]);
    Window_dropdown_add_item(format, F_fli, c64_format_names[F_fli]);
  }

  Update_window_area(0,0,Window_width,Window_height);
  Display_cursor();

  do
  {
    button = Window_clicked_button();
    if (Is_shortcut(Key, 0x100+BUTTON_HELP))
    {
      Key = 0;
      Window_help(BUTTON_SAVE, "COMMODORE 64 FORMATS");
    }
    else switch(button)
    {
      case 3: // Save what
        *saveWhat = Window_attribute2;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : what=%d (%s)\n", *saveWhat, what_label[*saveWhat]);
        break;

      case 4: // Load addr
        *loadAddr = Window_attribute2*0x2000;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : addr=$%04x (%d)\n",*loadAddr,Window_attribute2);
        break;

      case 5:
        *saveFormat = Window_attribute2;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : format=%d\n", Window_attribute2);
        break;

      case 0: break;
    }
  } while(button!=1 && button!=2);

  Close_window();
  Display_cursor();
  return button==1;
}


/**
 * Encode a C64 HiRes Bitmap picture.
 * 320x200 pixels, with only 2 different colors per 8x8 block.
 *
 * 8000 bytes bitmap, 1000 bytes screen RAM
 */
static int Encode_C64_hires(T_IO_Context * context, byte * bitmap, byte * screen_ram)
{
  int i, pos = 0;
  word cx, cy, x, y;

  for(cy=0; cy<25; cy++) // Character line, 25 lines
  {
    for(cx=0; cx<40; cx++) // Character column, 40 columns
    {
      byte fg, bg;  // foreground and background colors for the 8x8 block
      byte c[2];
      int count = 0;
      // first pass : find colors used
      for(y=0; y<8; y++)
      {
        for(x=0; x<8; x++)
        {
          byte pixel = Get_pixel(context, x+cx*8,y+cy*8);
          if(pixel>15)
          {
            Warning_message("Color above 15 used");
            // TODO hilite offending block here too?
            // or make it smarter with color allocation?
            // However, the palette is fixed to the 16 first colors
            return 1;
          }
          for (i = 0; i < count; i++)
          {
            if (c[i] == pixel)
              break;
          }
          if (i >= 2)
          {
            Warning_with_format("More than 2 colors\nin 8x8 pixel cell: (%d, %d)\nRect: (%d, %d, %d, %d)", cx, cy, cx * 8, cy * 8, cx * 8 + 7, cy * 8 + 7);
            // TODO here we should hilite the offending block
            return 1;
          }
          if (i >= count)
            c[count++] = pixel;
        }
      }

      if (count == 1)
      {
        if (c[0] == 0)  // only black
          fg = 1; // white
        else
          fg = c[0];
        bg = 0; // black
      }
      else
      {
        // set lower color index as background
        if (c[0] < c[1])
        {
          fg = c[1];
          bg = c[0];
        }
        else
        {
          fg = c[0];
          bg = c[1];
        }
      }
      screen_ram[cx+cy*40] = (fg<<4) | bg;

      // 2nd pass : store bitmap (0 = background, 1 = foreground)
      for(y=0; y<8; y++)
      {
        byte bits = 0;
        for(x=0; x<8; x++)
        {
          bits <<= 1;
          if (Get_pixel(context, x+cx*8, y+cy*8) == fg)
            bits |= 1;
        }
        bitmap[pos++] = bits;
      }
    }
  }
  return 0;
}

/// Save a C64 hires picture
///
/// c64 hires is 320x200 with only 2 colors per 8x8 block.
static int Save_C64_hires(T_IO_Context *context, byte saveWhat, word loadAddr)
{
  byte screen_ram[1000],bitmap[8000];
  FILE *file;
  int ret;

  ret = Encode_C64_hires(context, bitmap, screen_ram);
  if (ret != 0)
    return ret;
  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file,loadAddr);

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,bitmap,8000);
  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,screen_ram,1000);

  fclose(file);
  return 0;
}


/**
 * Save a C64 FLI (Flexible Line Interpretation) picture.
 *
 * This function is able to save a one layer picture, by finding
 * itself the background colors and color RAM value to be used.
 *
 * The algorithm is :
 * - first choose the lowest value for all possible background colors for each line
 * - first the lowest value from the possible colors for color RAM
 * - encode bitmap and screen RAMs
 *
 * The algorithm can fail by picking a "wrong" background color for a line,
 * that make the choice for the color RAM value of one of the 40 blocks impossible.
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_fli_monolayer(T_IO_Context *context, byte saveWhat, word loadAddr)
{
  FILE * file;
  byte bitmap[8000],screen_ram[1024*8],color_ram[1024];
  byte background[256];

  memset(bitmap, 0, sizeof(bitmap));
  memset(screen_ram, 0, sizeof(screen_ram));
  memset(color_ram, 0, sizeof(color_ram));
  memset(background, 0, sizeof(background));

  memset(color_ram, 0xff, 40*25); // no hint
  memset(background, 0xff, 200);

  if (C64_pixels_to_FLI(bitmap, screen_ram, color_ram, background, context->Target_address, context->Pitch, 0) > 0)
    return 1;

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file, loadAddr);

  if (saveWhat==0)
    Write_bytes(file,background,256);    // Background colors for lines 0-199 (+ 56bytes padding)

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,color_ram,1024); // Color RAM (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,screen_ram,8192);  // Screen RAMs 8 x (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,bitmap,8000);  // BitMap

  fclose(file);

  return 0;
}

/**
 * Encode a picture in the C64 Multicolor bitmap format.
 *
 * 8000 bytes bitmap, 1000 bytes screen RAM and 1000 bytes color RAM :
 *
 * BITS   COLOR INFORMATION COMES FROM
 * 00     Background color #0 (screen color)
 * 01     Upper 4 bits of Screen RAM
 * 10     Lower 4 bits of Screen RAM
 * 11     Color RAM nybble (nybble = 1/2 byte = 4 bits)
 */
static int Encode_C64_multicolor(T_IO_Context * context, byte * bitmap, byte * screen_ram, byte * color_ram, byte * background)
{
  int cx, cy, x, y;
  int c[4] = {0,0,0,0};
  int color, lut[16], bits, pixel, pos=0;
  int cand,n,used;
  word cols, candidates = 0, invalids = 0;

  // Detect the background color the image should be using. It's the one that's
  // used on all tiles having 4 colors.
  for(y = 0; y < 200; y += 8)
  {
    for (x = 0; x < 160; x += 4)
    {
      cols = 0;

      // Compute the usage count of each color in the tile
      for (cy = 0; cy < 8; cy++)
        for (cx = 0; cx < 4; cx++)
        {
          pixel = Get_pixel(context, x+cx,y+cy);
          if(pixel > 15)
          {
            Warning_message("Color above 15 used");
            // TODO hilite as in hires, you should stay to
            // the fixed 16 color palette
            return 1;
          }
          cols |= (1 << pixel);
        }

      cand = 0;
      used = 0;
      // Count the number of used colors in the tile
      for (n = 0; n<16; n++)
      {
        if (cols & (1 << n))
          used++;
      }

      if (used > 3)
      {
        GFX2_Log(GFX2_DEBUG, "(%3d,%3d) used=%d cols=%04x\n", x, y, used,(unsigned)cols);
        // This is a tile that uses the background color (and 3 others)

        // Try to guess which color is most likely the background one
        for (n = 0; n<16; n++)
        {
          if ((cols & (1 << n)) && !((candidates | invalids) & (1 << n))) {
            // This color is used in this tile but
            // was not used in any other tile yet,
            // so it could be the background one.
            candidates |= 1 << n;
          }

          if ((cols & (1 << n)) == 0 ) {
            // This color isn't used at all in this tile:
            // Can't be the global background
            invalids |= 1 << n;
            candidates &= ~(1 << n);
          }

          if (candidates & (1 << n)) {
            // We have a candidate, mark it as such
            cand++;
          }
        }

        // After checking the constraints for this tile, do we have
        // candidate background colors left ?
        if (cand==0)
        {
          Warning_message("No possible global background color");
          return 1;
        }
      }
    }
  }

	// Now just pick the first valid candidate
	for (n = 0; n<16; n++)
	{
		if (candidates & (1 << n)) {
			*background = n;
			break;
		}
	}
  GFX2_Log(GFX2_DEBUG, "Save_C64_multi() background=%d ($%x) candidates=%x invalid=%x\n",
           (int)*background, (int)*background, (unsigned)candidates, (unsigned)invalids);


  // Now that we know which color is the background, we can encode the cells
  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      memset(lut, 0xff, sizeof(lut));
      c[0] = *background;
      lut[*background] = 0;
      color = 1;

      for(y=0;y<8;y++)
      {
        bits=0;
        for(x=0;x<4;x++)
        {
          pixel = Get_pixel(context, cx*4+x, cy*8+y);
          if (lut[pixel] < 0)
          {
            if (color < 4)
            {
              c[color] = pixel;
              lut[pixel] = color;
              color++;
            }
            else
            {
              Warning_with_format("More than 4 colors\nin 4x8 pixel cell: (%d, %d)\nRect: (%d, %d, %d, %d)", cx, cy, cx * 4, cy * 8, cx * 4 + 3, cy * 8 + 7);
              // TODO hilite offending block
              return 1;
            }
          }
          bits = (bits << 2) | lut[pixel];
        }
        bitmap[pos++]=bits;
      }

      // add to screen_ram and color_ram
      screen_ram[cx+cy*40] = (c[1] << 4) | c[2];
      color_ram[cx+cy*40] = c[3];
    }
  }
  return 0;
}

/**
 * Save a C64 multicolor picture
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_multi(T_IO_Context *context, byte saveWhat, word loadAddr)
{
  FILE *file;
  byte background = 0;
  // FIXME allocating this on the stack is not a good idea. On some platforms
  // the stack has a rather small size...
  byte bitmap[8000], screen_ram[1000], color_ram[1000];

  File_error = Encode_C64_multicolor(context, bitmap, screen_ram, color_ram, &background);
  if (File_error != 0)
    return File_error;

  file = Open_file_write(context);
  if (file == NULL)
  {
    Warning_message("File open failed");
    File_error = 2;
    return 2;
  }

  if (loadAddr)
    Write_word_le(file,loadAddr);

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,bitmap,8000);

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,screen_ram,1000);

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,color_ram,1000);

  if (saveWhat==0)
    Write_byte(file,background);

  fclose(file);
  return 0;
}

/**
 * Save a C64 FLI (Flexible Line Interpretation) picture.
 *
 * This function need a 3 layer image :
 * - layer 0 is background colors
 * - layer 1 is color RAM values (4x8 blocks)
 * - layer 2 is the actual picture
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_fli(T_IO_Context * context, byte saveWhat, word loadAddr)
{
  FILE *file;
  byte file_buffer[17474];

  memset(file_buffer,0,sizeof(file_buffer));

  switch(C64_FLI(context, file_buffer+9474, file_buffer+1282, file_buffer+258, file_buffer+2))
  {
    case 0: // OK
      break;
    case 1:
      Warning_message("Less than 3 layers");
      File_error=1;
      return 1;
    case 2:
      Warning_message("Picture must be 160x200");
      File_error=1;
      return 1;
    default:
      File_error=1;
      return 1;
  }

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file, loadAddr);

  if (saveWhat==0)
    Write_bytes(file,file_buffer+2,256);    // Background colors for lines 0-199 (+ 56bytes padding)

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,file_buffer+258,1024); // Color RAM (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,file_buffer+1282,8192);  // Screen RAMs 8 x (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,file_buffer+9474,8000);  // BitMap

  fclose(file);
  return 0;
}

/**
 * Save C64 picture.
 *
 * Supports :
 * - HiRes (320x200)
 * - Multicolor
 * - FLI
 *
 * @param context the IO context
 */
void Save_C64(T_IO_Context * context)
{
  enum c64_format saveFormat = F_invalid;
  static byte saveWhat=0;
  static word loadAddr=0;

  if (((context->Width!=320) && (context->Width!=160)) || context->Height!=200)
  {
    Warning_message("must be 320x200 or 160x200");
    File_error = 1;
    return;
  }

  saveFormat = (context->Width == 320) ? F_hires : F_multi;

  GFX2_Log(GFX2_DEBUG, "Save_C64() extension : %s\n", context->File_name + strlen(context->File_name) - 4);
  if (strcasecmp(context->File_name + strlen(context->File_name) - 4, ".fli") == 0)
    saveFormat = F_fli;

  if(!Save_C64_window(&saveFormat, &saveWhat,&loadAddr))
  {
    File_error = 1;
    return;
  }

  Set_saving_layer(context, 0);
  switch (saveFormat)
  {
    case F_fli:
      if (context->Nb_layers < 3)
        File_error = Save_C64_fli_monolayer(context, saveWhat, loadAddr);
      else
        File_error = Save_C64_fli(context, saveWhat, loadAddr);
      break;
    case F_multi:
      File_error = Save_C64_multi(context, saveWhat, loadAddr);
      break;
    case F_bitmap:
      saveWhat = 1; // force save bitmap
#if defined(__GNUC__) && (__GNUC__ >= 7)
      __attribute__ ((fallthrough));
#endif
    case F_hires:
    default:
      File_error = Save_C64_hires(context, saveWhat, loadAddr);
  }
}

#include "c64picview_inc.h"

/**
 * Pack a stream of nibbles (ignore the high 4 bits) to a file.
 * This is designed to pack the color RAM data for VIC-II, as
 * the color RAM is only 4 bits.
 *
 * The output format is a stream of bytes of the following format :
 * CD  C = (16 - count), D = DATA (4bits)
 *
 * @return the output stream size, -1 for error
 */
static int C64_color_ram_pack(FILE * f, const byte * data, int count)
{
  byte previous = 0;
  int repeat_count = 0;
  int output_count = 0;
  while (count-- > 0)
  {
    if (repeat_count == 0)
    {
      previous = *data & 0x0f;
      repeat_count = 1;
    }
    else if ((*data & 0x0f) == previous)
    {
      repeat_count++;
      if (repeat_count >= 16)
      {
        if (!Write_byte(f, previous))
          return 0;
        output_count++;
        repeat_count = 0;
      }
    }
    else
    {
      if (!Write_byte(f, ((16 - repeat_count) << 4) | previous))
        return 0;
      output_count++;
      previous = *data & 0x0f;
      repeat_count = 1;
    }
    data++;
  }
  if (repeat_count > 0)
  {
    if (!Write_byte(f, ((16 - repeat_count) << 4) | previous))
      return -1;
    output_count++;
  }
  return output_count;
}

/**
 * Save autoloading C64 picture
 *
 * @todo handle more modes than multicolor
 */
void Save_PRG(T_IO_Context * context)
{
  byte background = 0;
  byte bitmap[8000], screen_ram[1000], color_ram[1000];
  enum c64_format saveFormat = F_invalid;

  if (((context->Width != 320) && (context->Width != 160)) || context->Height != 200)
  {
    Warning_message("must be 320x200 or 160x200");
    File_error = 1;
    return;
  }

  saveFormat = (context->Width == 320) ? F_hires : F_multi;
  File_error = 1;
  Set_saving_layer(context, 0);
  switch (saveFormat)
  {
    case F_hires:
      File_error = Encode_C64_hires(context, bitmap, screen_ram);
      break;
    case F_multi:
      File_error = Encode_C64_multicolor(context, bitmap, screen_ram, color_ram, &background);
      break;
    default:
      GFX2_Log(GFX2_ERROR, "Save_PRG(): format %d not handled (yet?)\n", saveFormat);
  }
  if (File_error == 0)
  {
    FILE *file;
    int n;

    file = Open_file_write(context);
    if (file == NULL)
    {
      File_error = 2;
      return;
    }
    if (!Write_bytes(file, picview_prg, sizeof(picview_prg)))
      File_error = 2;
    Write_byte(file, saveFormat == F_multi ? 0x30 : 0x20); // Mode : 0x40 Extended bg, 0x20 Bitmap, 0x10 Multicolor
    n = PackBits_pack_buffer(NULL, bitmap, 8000);
    GFX2_Log(GFX2_DEBUG, "PackBits of bitmap : 8000 => %d bytes\n", n + 1);
    if (n >= 0 && n < 7999)
    {
      Write_byte(file, 0x11); // bitmap / packbits
      PackBits_pack_buffer(file, bitmap, 8000);
      Write_byte(file, 0x80); // end of packbits stream marker
    }
    else
    {
      // packing was not efficient
      Write_byte(file, 0x10); // bitmap / no packing
      Write_bytes(file, bitmap, 8000);
    }
    n = PackBits_pack_buffer(NULL, screen_ram, 1000);
    GFX2_Log(GFX2_DEBUG, "PackBits of screen RAM : 1000 => %d bytes\n", n + 1);
    if (n >= 0 && n < 999)
    {
      Write_byte(file, 0x21); // screen RAM / packbits
      PackBits_pack_buffer(file, screen_ram, 1000);
      Write_byte(file, 0x80); // end of packbits stream marker
    }
    else
    {
      Write_byte(file, 0x20); // screen RAM / no packing
      Write_bytes(file, screen_ram, 1000);
    }
    if (saveFormat == F_multi)
    {
      //Write_byte(file, 0x30); // color RAM / no packing
      //Write_bytes(file, color_ram, 1000);
      Write_byte(file, 0x32); // color RAM / special color RAM packing
      n = C64_color_ram_pack(file, color_ram, 1000);
      if (n < 0)
        File_error = 1;
      GFX2_Log(GFX2_DEBUG, "custom packing of color RAM : 1000 => %d bytes\n", n);

      Write_byte(file, 0x42); // border/background/etc. / color ram RLE packing
      Write_byte(file, background | 0x10);
    }
    Write_byte(file, 0); // end of file
    fclose(file);
  }
}


/////////////////////////// pixcen *.GPX ///////////////////////////
void Test_GPX(T_IO_Context * context, FILE * file)
{
  byte header[2];
  (void)context;

  // check for a Zlib compressed stream
  File_error = 1;
  if (!Read_bytes(file, header, 2))
    return;
  if ((header[0] & 0x0f) != 8)
    return;
  if (((header[0] << 8) + header[1]) % 31)
    return;
  File_error = 0;
}

void Load_GPX(T_IO_Context * context)
{
  FILE * file;
  unsigned long file_size;
  byte * buffer;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  buffer = GFX2_malloc(file_size);
  if (buffer == NULL)
  {
    fclose(file);
    return;
  }
  if (Read_bytes(file, buffer, file_size))
  {
    byte * gpx = NULL;
    unsigned long gpx_size = 0;
    int r = Z_MEM_ERROR;

    do
    {
      free(gpx);
      gpx_size += 65536;
      gpx = GFX2_malloc(gpx_size);
      if (gpx == NULL)
        break;
      r = uncompress(gpx, &gpx_size, buffer, file_size);
      if (r != Z_BUF_ERROR && r != Z_OK)
        GFX2_Log(GFX2_ERROR, "uncompress() failed with error %d: %s\n", r, zError(r));
    }
    while (r == Z_BUF_ERROR); // there was not enough room in the output buffer
    if (r == Z_OK)
    {
      byte * p;
      dword version, mode;
/*
 mode :
0		BITMAP,
1		MC_BITMAP,
2		SPRITE,
3		MC_SPRITE,
4		CHAR,
5   MC_CHAR,
6		UNUSED1,
7		UNUSED2,
8		UNRESTRICTED,
9		W_UNRESTRICTED
*/
      GFX2_Log(GFX2_DEBUG, "inflated %lu bytes to %lu\n", file_size, gpx_size);
#define READU32LE(p) ((p)[0] | (p)[1] << 8 | (p)[2] << 16 | (p)[3] << 24)
      version = READU32LE(gpx);
      mode = READU32LE(gpx+4);
      GFX2_Log(GFX2_DEBUG, "gpx version %u mode %u\n", version, mode);
      snprintf(context->Comment, COMMENT_SIZE, "pixcen file version %u mode %u", version, mode);
      if (version >= 4)
      {
        dword count;
        const char * key;
        word value[256];
        int xsize = -1;
        int ysize = -1;
        int mapsize = -1;
        int screensize = -1;
        int colorsize = -1;
        int backbuffers = -1;

        count = READU32LE(gpx+8);
        p = gpx + 12;
        while (count--)
        {
          int i = 0;
          int int_value = 0;

          key = (const char *)p;
          while (*p++);
          for (;;)
          {
            value[i] = p[0] + (p[1] << 8);
            p += 2;
            if (value[i] == 0)
              break;
            int_value = int_value * 10 + (value[i] - '0');
            i++;
          }
          GFX2_Log(GFX2_DEBUG, "%s=%d\n", key, int_value);
          if (0 == strcmp(key, "xsize"))
            xsize = int_value;
          else if (0 == strcmp(key, "ysize"))
            ysize = int_value;
          else if (0 == strcmp(key, "mapsize"))
            mapsize = int_value;
          else if (0 == strcmp(key, "screensize"))
            screensize = int_value;
          else if (0 == strcmp(key, "colorsize"))
            colorsize = int_value;
          else if (0 == strcmp(key, "backbuffers"))
            backbuffers = int_value;
        }
//buffersize = 64 + (64 + mapsize + screensize + colorsize) * backbuffers;
        p += 64;  // 64 empty bytes ?
        File_error = 0;
        if (mode & 1)
          context->Ratio = PIXEL_WIDE;
        else
          context->Ratio = PIXEL_SIMPLE;
        Pre_load(context, xsize, ysize, file_size, FORMAT_GPX, context->Ratio, 4); // Do this as soon as you can
        if (Config.Clear_palette)
          memset(context->Palette,0, sizeof(T_Palette));
        C64_set_palette(context->Palette);
        context->Transparent_color=16;

        //foreach backbuffer
        if (backbuffers >= 1)
        {
          byte border, background;
          //byte ext0, ext1, ext2;
          byte * bitmap, * color, * screen;

          //GFX2_LogHexDump(GFX2_DEBUG, "GPX ", p, 0, 64);
          p += 47;  // Extra bytes
          //crippled = p;
          p += 6;
          //lock = p;
          p += 6;
          border = *p++;
          background = *p++;
          /*ext0 = *p++;
          ext1 = *p++;
          ext2 = *p++;*/
          p += 3;
          bitmap = p;
          p += mapsize;
          color = p;
          p += colorsize;
          screen = p;
          p += screensize;

          GFX2_Log(GFX2_DEBUG, "background color #%d, border color #%d\n", (int)background, (int)border);
          Load_C64_multi(context, bitmap, screen, color, background);
          Set_image_mode(context, (mode & 1) ? IMAGE_MODE_C64MULTI : IMAGE_MODE_C64HIRES);
        }
      }
      else
      {
        GFX2_Log(GFX2_ERROR, "GPX file version %d unsupported\n", version);
      }
    }
    free(gpx);
  }
  free(buffer);
  fclose(file);
}
