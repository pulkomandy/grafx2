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

///@file ifformat.c
/// Saving and loading IFF picture formats.

#include <stdlib.h>
#include <string.h>
#include "fileformats.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "misc.h"
#include "packbits.h"
#include "gfx2mem.h"
#include "gfx2log.h"

#if defined(WIN32)
#if defined(_MSC_VER)
#define WIN32_BSWAP32 _byteswap_ulong
#else
#define WIN32_BSWAP32 __builtin_bswap32
#endif
#endif

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#ifndef be32toh
#define be32toh(x) OSSwapBigToHostInt32(x)
#endif
#endif

//////////////////////////////////// IFF ////////////////////////////////////
/** @defgroup IFF Interchange File Format
 * @ingroup loadsaveformats
 * Interchange File Format (Electronic Arts)
 *
 * This is the "native" format of Amiga.
 * We support ILBM/PBM/ACBM
 * @{
 */
typedef struct
{
  word  Width;
  word  Height;
  word  X_org;       // Inutile
  word  Y_org;       // Inutile
  byte  BitPlanes;
  byte  Mask;        // 0=none, 1=mask, 2=transp color, 3=Lasso
  byte  Compression; // 0=none, 1=packbits, 2=vertical RLE
  byte  Pad1;        // Inutile
  word  Transp_col;  // transparent color for masking mode 2
  byte  X_aspect;    // Inutile
  byte  Y_aspect;    // Inutile
  word  X_screen;
  word  Y_screen;
} T_IFF_Header;

typedef struct
{
  byte operation; // 0=normal body, 1=XOR, 2=Long Delta, 3=Short Delta,
                  // 4=Generalized Long/Short Delta, 5=Byte Vertical Delta,
                  // 7=short/long vertical delta, 74=Eric Graham's compression
  byte mask;  // for XOR mode only
  word w,h; // for XOR mode only
  word x,y; // for XOR mode only
  dword abstime;
  dword reltime;
  byte interleave;
  byte pad0;
  dword bits;
} T_IFF_AnimHeader;

/// Test if a file is in IFF format
void Test_IFF(FILE * IFF_file, const char *sub_type)
{
  char  format[4];
  char  section[4];
  dword dummy;

  File_error=1;

  if (! Read_bytes(IFF_file,section,4))
    return;
  if (memcmp(section,"FORM",4))
        return;

  if (! Read_dword_be(IFF_file, &dummy))
    return;
  //   On aurait pu vérifier que ce long est égal à la taille
  // du fichier - 8, mais ça aurait interdit de charger des
  // fichiers tronqués (et déjà que c'est chiant de perdre
  // une partie du fichier il faut quand même pouvoir en
  // garder un peu... Sinon, moi je pleure :'( !!! )
  if (! Read_bytes(IFF_file,format,4))
    return;

  if (!memcmp(format,"ANIM",4))
  {
    dword size;

    // An ANIM header: need to check that it encloses an image
    do
    {
      if (! Read_bytes(IFF_file,section,4))
        return;
      if (memcmp(section,"FORM",4))
        return;
      if (! Read_dword_be(IFF_file, &size))
        return;
      if (! Read_bytes(IFF_file,format,4))
        return;
      if (fseek(IFF_file, size - 4, SEEK_CUR) < 0)
        return;
    }
    while(0 == memcmp(format, "8SVX", 4));  // skip sound frames
  }
  else if(memcmp(format,"DPST",4) == 0)
  {
    if (! Read_bytes(IFF_file,section,4))
      return;
    if (memcmp(section, "DPAH", 4) != 0)
      return;
    if (! Read_dword_be(IFF_file, &dummy))
      return;
    fseek(IFF_file, dummy, SEEK_CUR);
    if (! Read_bytes(IFF_file,section,4))
      return;
    if (memcmp(section,"FORM",4))
      return;
    if (! Read_dword_be(IFF_file, &dummy))
      return;
    if (! Read_bytes(IFF_file,format,4))
      return;
  }
  if ( memcmp(format,sub_type,4))
    return;

  // If we reach this part, file is indeed ILBM/PBM or ANIM
  File_error=0;
}

void Test_PBM(T_IO_Context * context, FILE * f)
{
  (void)context;
  Test_IFF(f, "PBM ");
}
void Test_LBM(T_IO_Context * context, FILE * f)
{
  (void)context;
  Test_IFF(f, "ILBM");
}
void Test_ACBM(T_IO_Context * context, FILE * f)
{
  (void)context;
  Test_IFF(f, "ACBM");
}


// -- Lire un fichier au format IFF -----------------------------------------

typedef struct T_IFF_PCHG_Palette {
  struct T_IFF_PCHG_Palette * Next;
  short StartLine;
  T_Components Palette[1];
} T_IFF_PCHG_Palette;

/// Skips the current section in an IFF file.
/// This function should be called while the file pointer is right
/// after the 4-character code that identifies the section.
static int IFF_Skip_section(FILE * file)
{
  dword size;
  
  if (!Read_dword_be(file,&size))
    return 0;
  if (size&1)
    size++;
  if (fseek(file,size,SEEK_CUR))
    return 0;
  return 1;
}

/// Wait for a specific IFF chunk
static byte IFF_Wait_for(FILE * file, const char * expected_section)
{
  // Valeur retournée: 1=Section trouvée, 0=Section non trouvée (erreur)
  byte section_read[4];

  if (! Read_bytes(file,section_read,4))
    return 0;
  while (memcmp(section_read,expected_section,4)) // Sect. pas encore trouvée
  {
    if (!IFF_Skip_section(file))
      return 0;
    if (! Read_bytes(file,section_read,4))
      return 0;
  }
  return 1;
}

// Les images ILBM sont stockés en bitplanes donc on doit trifouiller les bits pour
// en faire du chunky

///
/// Decodes the color of one pixel from the ILBM line buffer.
/// Planar to chunky conversion
/// @param buffer          Planar buffer
/// @param x_pos           Position of the pixel in graphic line
/// @param real_line_size  Width of one bitplane in memory, in bytes
/// @param bitplanes       Number of bitplanes
static dword Get_IFF_color(const byte * buffer, word x_pos, word real_line_size, byte bitplanes)
{
  byte shift = 7 - (x_pos & 7);
  int address,masked_bit,plane;
  dword color=0;

  for(plane=bitplanes-1;plane>=0;plane--)
  {
    address = (real_line_size * plane + x_pos) >> 3;
    masked_bit = (buffer[address] >> shift) & 1;

    color = (color << 1) + masked_bit;
  }

  return color;
}

/// chunky to planar
static void Set_IFF_color(byte * buffer, word x_pos, byte color, word real_line_size, byte bitplanes)
{
  byte shift = 7 - (x_pos & 7);
  int address, plane;

  for(plane=0;plane<bitplanes;plane++)
  {
    address = (real_line_size * plane + x_pos) >> 3;
    buffer[address] |= (color&1) << shift;
    color = color >> 1;
  }
}

// ----------------------- Afficher une ligne ILBM ------------------------
/// Planar to chunky conversion of a line
/// @param context         the IO context
/// @param buffer          Planar buffer
/// @param y_pos           Current line
/// @param real_line_size  Width of one bitplane in memory, in bytes
/// @param bitplanes       Number of bitplanes
void Draw_IFF_line(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes)
{
  short x_pos;

  if (bitplanes > 8)
  {
    for (x_pos=0; x_pos<context->Width; x_pos++)
    {
      // Default standard deep ILBM bit ordering:
      // saved first -----------------------------------------------> saved last
      // R0 R1 R2 R3 R4 R5 R6 R7 G0 G1 G2 G3 G4 G5 G6 G7 B0 B1 B2 B3 B4 B5 B6 B7
      dword rgb = Get_IFF_color(buffer, x_pos,real_line_size, bitplanes);
      Set_pixel_24b(context, x_pos,y_pos, rgb, rgb >> 8, rgb >> 16);  // R is 8 LSB, etc.
    }
  }
  else for (x_pos=0; x_pos<context->Width; x_pos++)
  {
    Set_pixel(context, x_pos, y_pos, Get_IFF_color(buffer, x_pos,real_line_size, bitplanes));
  }
}

/// decode pixels with palette changes per line (copper list:)
static void Draw_IFF_line_PCHG(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes, const T_IFF_PCHG_Palette * PCHG_palettes)
{
  const T_IFF_PCHG_Palette * palette;
  short x_pos;

  palette = PCHG_palettes;  // find the palette to use for the line
  if (palette == NULL)
    return;
  while (palette->Next != NULL && palette->Next->StartLine <= y_pos)
    palette = palette->Next;

  for (x_pos=0; x_pos<context->Width; x_pos++)
  {
    dword c = Get_IFF_color(buffer, x_pos,real_line_size, bitplanes);
    Set_pixel_24b(context, x_pos,y_pos, palette->Palette[c].R, palette->Palette[c].G, palette->Palette[c].B);
  }
}

/// Decode a HAM line to 24bits pixels
static void Draw_IFF_line_HAM(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes, const T_IFF_PCHG_Palette * PCHG_palettes)
{
  short x_pos;
  byte red, green, blue, temp;
  const T_Components * palette;

  if (PCHG_palettes == NULL)
    palette = context->Palette;
  else
  {
    // find the palette to use for the line
    while (PCHG_palettes->Next != NULL && PCHG_palettes->Next->StartLine <= y_pos)
      PCHG_palettes = PCHG_palettes->Next;
    palette = PCHG_palettes->Palette;
  }
  red = palette[0].R;
  green = palette[0].G;
  blue = palette[0].B;
  if (bitplanes == 6)
  {
    for (x_pos=0; x_pos<context->Width; x_pos++)         // HAM6
    {
      temp=Get_IFF_color(buffer, x_pos,real_line_size, bitplanes);
      switch (temp & 0x30)
      {
        case 0x10: // blue
          blue=(temp&0x0F)*0x11;
          break;
        case 0x20: // red
          red=(temp&0x0F)*0x11;
          break;
        case 0x30: // green
          green=(temp&0x0F)*0x11;
          break;
        default:   // Nouvelle couleur
          red=palette[temp].R;
          green =palette[temp].G;
          blue =palette[temp].B;
      }
      Set_pixel_24b(context, x_pos,y_pos,red,green,blue);
    }
  }
  else
  {
    for (x_pos=0; x_pos<context->Width; x_pos++)         // HAM8
    {
      temp=Get_IFF_color(buffer,x_pos,real_line_size, bitplanes);
      switch (temp >> 6)
      {
        case 0x01: // blue
          blue= (temp << 2) | ((temp & 0x30) >> 4);
          break;
        case 0x02: // red
          red= (temp << 2) | ((temp & 0x30) >> 4);
          break;
        case 0x03: // green
          green= (temp << 2) | ((temp & 0x30) >> 4);
          break;
        default:   // Nouvelle couleur
          red=palette[temp].R;
          green =palette[temp].G;
          blue =palette[temp].B;
      }
      Set_pixel_24b(context, x_pos,y_pos,red,green,blue);
    }
  }
}

/// Decode PBM data
///
/// Supports compressions :
/// - 0 uncompressed
/// - 1 compressed
static void PBM_Decode(T_IO_Context * context, FILE * file, byte compression, word width, word height)
{
  byte * line_buffer;
  word x_pos, y_pos;
  word real_line_size = (width+1)&~1;

  switch (compression)
  {
    case 0: // uncompressed
      line_buffer=(byte *)malloc(real_line_size);
      for (y_pos=0; ((y_pos<height) && (!File_error)); y_pos++)
      {
        if (Read_bytes(file,line_buffer,real_line_size))
          for (x_pos=0; x_pos<width; x_pos++)
            Set_pixel(context, x_pos,y_pos,line_buffer[x_pos]);
        else
          File_error=26;
      }
      free(line_buffer);
      break;
    case 1: // Compressed
      for (y_pos=0; ((y_pos<height) && (!File_error)); y_pos++)
      {
        for (x_pos=0; ((x_pos<real_line_size) && (!File_error)); )
        {
          byte temp_byte, color;
          if(Read_byte(file, &temp_byte)!=1)
          {
            File_error=27;
            break;
          }
          if (temp_byte>127)
          {
            if(Read_byte(file, &color)!=1)
            {
              File_error=28;
              break;
            }
            do {
              Set_pixel(context, x_pos++,y_pos,color);
            }
            while(temp_byte++ != 0);
          }
          else
            do
            {
              if(Read_byte(file, &color)!=1)
              {
                File_error=29;
                break;
              }
              Set_pixel(context, x_pos++,y_pos,color);
            }
            while(temp_byte-- > 0);
        }
      }
      break;
    default:
      GFX2_Log(GFX2_ERROR, "PBM only supports compression type 0 and 1 (not %d)\n", compression);
      File_error = 50;
  }
}

/// Decode LBM data
///
/// Supports packings :
/// - 0 uncompressed
/// - 1 packbits (Amiga)
/// - 2 Vertical RLE (Atari ST)
static void LBM_Decode(T_IO_Context * context, FILE * file, byte compression, byte Image_HAM,
                       byte stored_bit_planes, byte real_bit_planes, const T_IFF_PCHG_Palette * PCHG_palettes)
{
  int plane;
  byte * buffer;
  short x_pos, y_pos;
  // compute row size
  int real_line_size = (context->Width+15) & ~15; // size in bit for one bit plane
  int plane_line_size = real_line_size >> 3;      // size in byte for one bit plane
  int line_size = plane_line_size * stored_bit_planes; // size in byte for all bitplanes

  switch(compression)
  {
    case 0:            // uncompressed
      buffer = (byte *)GFX2_malloc(line_size);
      if (buffer == NULL)
      {
        File_error=1;
        return;
      }
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        if (Read_bytes(file,buffer,line_size))
        {
          if (Image_HAM > 1)
            Draw_IFF_line_HAM(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
          else if (PCHG_palettes)
            Draw_IFF_line_PCHG(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
          else
            Draw_IFF_line(context, buffer, y_pos,real_line_size, real_bit_planes);
        }
        else
          File_error=21;
      }
      free(buffer);
      break;
    case 1:          // packbits compression (Amiga)
      buffer = (byte *)GFX2_malloc(line_size);
      if (buffer == NULL)
      {
        File_error=1;
        return;
      }
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
        {
          byte color;
          byte temp_byte;
          if(Read_byte(file, &temp_byte)!=1)
          {
            File_error=22;
            break;
          }
          // temp_byte > 127  => repeat (257-temp_byte) the next byte
          // temp_byte <= 127 => copy (temp_byte + 1) bytes
          if(temp_byte == 128) // 128 = NOP !
          {
            GFX2_Log(GFX2_WARNING, "NOP in packbits stream\n");
          }
          else if (temp_byte>127)
          {
            if(Read_byte(file, &color)!=1)
            {
              File_error=23;
              break;
            }
            do
            {
              if (x_pos<line_size)
                buffer[x_pos++]=color;
              else
              {
                File_error=24;
                break;
              }
            }
            while (temp_byte++ != 0);
          }
          else
            do
            {
              if (x_pos>=line_size || !Read_byte(file, &(buffer[x_pos++])))
              {
                File_error=25;
                break;
              }
            }
            while (temp_byte-- > 0);
        }
        if (!File_error)
        {
          if (Image_HAM > 1)
            Draw_IFF_line_HAM(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
          else if (PCHG_palettes)
            Draw_IFF_line_PCHG(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
          else
            Draw_IFF_line(context, buffer, y_pos,real_line_size,real_bit_planes);
        }
      }
      free(buffer);
      break;
    case 2:     // vertical RLE compression (Atari ST)
      buffer = (byte *)GFX2_malloc(line_size*context->Height);
      if (buffer == NULL)
      {
        File_error=1;
        return;
      }
	  memset(buffer, 0, line_size*context->Height);
      for (plane = 0; plane < stored_bit_planes && !File_error; plane++)
      {
        dword section_size;
        word cmd_count;
        word cmd;
        signed char * commands;
        word count;

        y_pos = 0; x_pos = 0;
        if (!IFF_Wait_for(file, "VDAT"))
        {
          if (plane == 0) // don't cancel loading if at least 1 bitplane has been loaded
            File_error = 30;
          break;
        }
        Read_dword_be(file,&section_size);
        Read_word_be(file,&cmd_count);
        cmd_count -= 2;
        commands = (signed char *)malloc(cmd_count);
        if (!Read_bytes(file,commands,cmd_count))
        {
          File_error = 31;
          break;
        }
        section_size -= (cmd_count + 2);
        for (cmd = 0; cmd < cmd_count && x_pos < plane_line_size && section_size > 0; cmd++)
        {
          if (commands[cmd] <= 0)
          { // cmd=0 : load count from data, COPY
            // cmd < 0 : count = -cmd, COPY
            if (commands[cmd] == 0)
            {
              Read_word_be(file,&count);
              section_size -= 2;
            }
            else
              count = -commands[cmd];
            while (count-- > 0 && x_pos < plane_line_size && section_size > 0)
            {
              Read_bytes(file,buffer+x_pos+y_pos*line_size+plane*plane_line_size,2);
              section_size -= 2;
              if(++y_pos >= context->Height)
              {
                y_pos = 0;
                x_pos += 2;
              }
            }
          }
          else if (commands[cmd] >= 1)
          { // cmd=1 : load count from data, RLE
            // cmd >1 : count = cmd, RLE
            byte data[2];
            if (commands[cmd] == 1)
            {
              Read_word_be(file,&count);
              section_size -= 2;
            }
            else
              count = (word)commands[cmd];
            if (section_size == 0)
              break;
            Read_bytes(file,data,2);
            section_size -= 2;
            while (count-- > 0 && x_pos < plane_line_size)
            {
              memcpy(buffer+x_pos+y_pos*line_size+plane*plane_line_size,data,2);
              if(++y_pos >= context->Height)
              {
                y_pos = 0;
                x_pos += 2;
              }
            }
          }
        }
        if(cmd < (cmd_count-1) || section_size > 0)
          GFX2_Log(GFX2_WARNING, "Early end in VDAT chunk\n");
        if (section_size > 0)
          fseek(file, (section_size+1)&~1, SEEK_CUR); // skip bytes
        free(commands);
      }
      if (!File_error)
      {
        for (y_pos = 0; y_pos < context->Height; y_pos++)
        {
          Draw_IFF_line(context,buffer+line_size*y_pos,y_pos,real_line_size,real_bit_planes);
        }
      }
      free(buffer);
      break;
    default:
      GFX2_Log(GFX2_WARNING, "Unknown IFF compression %d\n", compression);
      File_error = 32;
  }
}

/// Decode RAST chunk (from Atari ST pictures)
static void RAST_chunk_decode(T_IO_Context * context, FILE * file, dword section_size, T_IFF_PCHG_Palette ** PCHG_palettes)
{
  int i;
  T_Components palette[16];
  T_IFF_PCHG_Palette * prev_pal = NULL;
  T_IFF_PCHG_Palette * new_pal = NULL;

  // 17 words per palette : 1 for line, 16 for the colors
  while (section_size >= 34)
  {
    word line, value;

    Read_word_be(file, &line);
    for (i = 0; i < 16; i++)
    {
      Read_word_be(file, &value);  // Decode STE Palette
      palette[i].R = ((value & 0x0700) >> 7 | (value & 0x0800) >> 11) * 0x11;
      palette[i].G = ((value & 0x0070) >> 3 | (value & 0x0080) >> 7) * 0x11;
      palette[i].B = ((value & 0x0007) << 1 | (value & 0x0008) >> 3) * 0x11;
    }
    section_size -= 34;

    if (prev_pal == NULL || (line > prev_pal->StartLine && (0 != memcmp(palette, prev_pal->Palette, sizeof(T_Components)*3))))
    {
      new_pal = malloc(sizeof(T_IFF_PCHG_Palette) + sizeof(T_Components) * 16);
      if (new_pal == NULL)
      {
        File_error = 2;
        return;
      }
      memcpy(new_pal->Palette, palette, sizeof(T_Components) * 16);
      new_pal->StartLine = line;
      new_pal->Next = NULL;
      if (prev_pal != NULL)
      {
        prev_pal->Next = new_pal;
        prev_pal = new_pal;
      }
      else if (line == 0)
      {
        prev_pal = new_pal;
        *PCHG_palettes = prev_pal;
      }
      else  // line > 0 && prev_pal == NULL
      {
        // create a palette for line 0
        prev_pal = malloc(sizeof(T_IFF_PCHG_Palette) + sizeof(T_Components) * 16);
        if (prev_pal == NULL)
        {
          File_error = 2;
          return;
        }
        memcpy(prev_pal->Palette, context->Palette, sizeof(T_Components) * 16);
        prev_pal->StartLine = 0;
        prev_pal->Next = new_pal;
        *PCHG_palettes = prev_pal;
        prev_pal = new_pal;
      }
    }
  }
}

/// Sets 32 upper colors of EHB palette
static void IFF_Set_EHB_Palette(T_Components * palette)
{
  int i, j;            /// 32 colors in the palette.
  for (i=0; i<32; i++) /// The next 32 colors are the same with values divided by 2
  {
    j=i+32;
    palette[j].R = palette[i].R>>1;
    palette[j].G = palette[i].G>>1;
    palette[j].B = palette[i].B>>1;
  }
}

/// Load IFF picture (PBM/ILBM/ACBM) or animation
void Load_IFF(T_IO_Context * context)
{
  FILE * IFF_file;
  T_IFF_Header header;
  T_IFF_AnimHeader aheader;
  char  format[4];
  char  section[4];
  byte  temp_byte;
  dword nb_colors = 0; // number of colors in the CMAP (color map)
  dword section_size;
  short x_pos;
  short y_pos;
  short line_size = 0;        // Taille d'une ligne en octets
  short plane_line_size = 0;  // Size of line in bytes for 1 plane
  short real_line_size = 0;   // Taille d'une ligne en pixels
  unsigned long file_size;
  dword dummy;
  int iff_format = 0;
  int plane;
  dword AmigaViewModes = 0;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  byte * buffer;
  byte bpp = 0;
  byte Image_HAM = 0;
  T_IFF_PCHG_Palette * PCHG_palettes = NULL;
  int current_frame = 0;
  byte * previous_frame = NULL; // For animations
  byte * anteprevious_frame = NULL;
  word frame_count = 0;
  word frame_duration = 0;
  word vdlt_plane = 0; // current plane during Atari ST animation decoding

  memset(&aheader, 0, sizeof(aheader));

  File_error=0;

  if ((IFF_file=Open_file_read(context)))
  {
    file_size=File_length_file(IFF_file);

    // FORM + size(4)
    Read_bytes(IFF_file,section,4);
    Read_dword_be(IFF_file,&dummy);
    
    Read_bytes(IFF_file,format,4);
    if (!memcmp(format,"ANIM",4))
    {
      // Skip a bit, brother
      Read_bytes(IFF_file,section,4);
      Read_dword_be(IFF_file,&section_size);
      Read_bytes(IFF_file,format,4);
      while (0 == memcmp(format, "8SVX", 4))
      {
        GFX2_Log(GFX2_DEBUG, "IFF : skip sound %.4s %u bytes\n", format, section_size);
        if (fseek(IFF_file, section_size - 4, SEEK_CUR) < 0)
          break;
        Read_bytes(IFF_file,section,4);
        Read_dword_be(IFF_file,&section_size);
        Read_bytes(IFF_file,format,4);
      }
    }
    else if(memcmp(format,"DPST",4)==0)
    {
      // read DPAH
      while (File_error == 0)
      {
        if (!(Read_bytes(IFF_file,section,4)
           && Read_dword_be(IFF_file,&section_size)))
          File_error = 1;
        if (memcmp(section, "FORM", 4) == 0)
        {
          Read_bytes(IFF_file,format,4);
          break;
        }
        else if (memcmp(section, "DPAH", 4) == 0) // Deluxe Paint Animation Header
        {
          word version;
          Read_word_be(IFF_file, &frame_duration);
          Read_word_be(IFF_file, &frame_count);
          Read_word_be(IFF_file, &version);
          section_size -= 6;
          Set_frame_duration(context, (frame_duration * 50) / 3);  // convert 1/60th sec to msec
        }
        fseek(IFF_file, (section_size+1)&~1, SEEK_CUR); // skip unread bytes
      }
    }
    if (memcmp(format,"ILBM",4) == 0)
      iff_format = FORMAT_LBM;
    else if(memcmp(format,"PBM ",4) == 0)
      iff_format = FORMAT_PBM;
    else if(memcmp(format,"ACBM",4) == 0)
      iff_format = FORMAT_ACBM;
    else
    {
      GFX2_Log(GFX2_WARNING, "Unknown IFF format '%.4s'\n", format);
      File_error=1;
    }
    

    {
      byte real_bit_planes = 0;
      byte stored_bit_planes = 0;
      while (File_error == 0
          && Read_bytes(IFF_file,section,4)
          && Read_dword_be(IFF_file,&section_size))
      {
        if (memcmp(section, "FORM", 4) == 0)
        {
          // special
          Read_bytes(IFF_file, format, 4);
          if (memcmp(format, "VDLT", 4) == 0) // Vertical DeLTa
          { // found in animation produced by DeluxePaint for Atari ST
            if (frame_count != 0 && current_frame >= (frame_count - 1))
              break;  // file contains the Delta from the last frame to the 1st one to allow looping, skip it

            real_line_size = (context->Width+15) & ~15;
            plane_line_size = real_line_size >> 3;  // 8bits per byte
            line_size = plane_line_size * real_bit_planes;

            if (previous_frame == NULL)
            {
              previous_frame = calloc(line_size * context->Height,1);
              for (y_pos=0; y_pos<context->Height; y_pos++)
              {
                const byte * pix_p = context->Target_address + y_pos * context->Pitch;
                // Dispatch the pixel into planes
                for (x_pos=0; x_pos<context->Width; x_pos++)
                {
                  Set_IFF_color(previous_frame+y_pos*line_size, x_pos, *pix_p++, real_line_size, real_bit_planes);
                }
              }
            }

            Set_loading_layer(context, ++current_frame);
            Set_frame_duration(context, (frame_duration * 50) / 3);  // convert 1/60th sec to msec
            vdlt_plane = 0;
          }
          continue;
        }
        if (memcmp(section, "BMHD", 4) == 0)  // BitMap HeaDer
        {
          if (!((Read_word_be(IFF_file,&header.Width))
            && (Read_word_be(IFF_file,&header.Height))
            && (Read_word_be(IFF_file,&header.X_org))
            && (Read_word_be(IFF_file,&header.Y_org))
            && (Read_byte(IFF_file,&header.BitPlanes))
            && (Read_byte(IFF_file,&header.Mask))
            && (Read_byte(IFF_file,&header.Compression))
            && (Read_byte(IFF_file,&header.Pad1))
            && (Read_word_be(IFF_file,&header.Transp_col))
            && (Read_byte(IFF_file,&header.X_aspect))
            && (Read_byte(IFF_file,&header.Y_aspect))
            && (Read_word_be(IFF_file,&header.X_screen))
            && (Read_word_be(IFF_file,&header.Y_screen))) )
          {
            File_error = 1;
            break;
          }
          GFX2_Log(GFX2_DEBUG, "IFF BMHD : origin (%u,%u) size %ux%u BitPlanes=%u Mask=%u Compression=%u\n",
                   header.X_org, header.Y_org, header.Width, header.Height,
                   header.BitPlanes, header.Mask, header.Compression);
          GFX2_Log(GFX2_DEBUG, "          Transp_col=#%u aspect ratio %u/%u screen size %ux%u\n",
                   header.Transp_col, header.X_aspect, header.Y_aspect,
                   header.X_screen, header.Y_screen);
          real_bit_planes = header.BitPlanes;
          stored_bit_planes = header.BitPlanes;
          if (header.Mask==1)
            stored_bit_planes++;
          Image_HAM=0;
          if (header.X_aspect != 0 && header.Y_aspect != 0)
          {
            if ((10 * header.X_aspect) <= (6 * header.Y_aspect))
              ratio = PIXEL_TALL;   // ratio <= 0.6
            else if ((10 * header.X_aspect) <= (8 * header.Y_aspect))
              ratio = PIXEL_TALL3;  // 0.6 < ratio <= 0.8
            else if ((10 * header.X_aspect) < (15 * header.Y_aspect))
              ratio = PIXEL_SIMPLE; // 0.8 < ratio < 1.5
            else
              ratio = PIXEL_WIDE; // 1.5 <= ratio
          }
          bpp = header.BitPlanes;
          if (bpp <= 8 && bpp > 0)
          {
            unsigned int i;
            // Set a default grayscale palette : if the
            // ILBM/PBM file has no palette, it should be assumed to be a
            // Gray scale picture.
            if (Config.Clear_palette)
              memset(context->Palette,0,sizeof(T_Palette));
            nb_colors = 1 << bpp;
            for (i = 0; i < nb_colors; i++)
              context->Palette[i].R =
              context->Palette[i].G =
              context->Palette[i].B = (i * 255) / (nb_colors - 1);
          }
        }
        else if (memcmp(section, "ANHD", 4) == 0) // ANimation  HeaDer
        {
          // http://www.textfiles.com/programming/FORMATS/anim7.txt
          Read_byte(IFF_file, &aheader.operation);
          Read_byte(IFF_file, &aheader.mask);
          Read_word_be(IFF_file, &aheader.w);
          Read_word_be(IFF_file, &aheader.h);
          Read_word_be(IFF_file, &aheader.x);
          Read_word_be(IFF_file, &aheader.y);
          Read_dword_be(IFF_file, &aheader.abstime);
          Read_dword_be(IFF_file, &aheader.reltime);
          Read_byte(IFF_file, &aheader.interleave);
          Read_byte(IFF_file, &aheader.pad0);
          Read_dword_be(IFF_file, &aheader.bits);
          section_size -= 24;

          if ((aheader.bits & 0xffffffc0) != 0) // invalid ? => clearing
            aheader.bits = 0;
          if (aheader.operation == 0) // ANHD for 1st frame (BODY)
            Set_frame_duration(context, (aheader.reltime * 50) / 3);  // convert 1/60th sec to msec
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip remaining bytes
        }
        else if (memcmp(section, "DPAN", 4) == 0) // Deluxe Paint ANimation
        {
          word version;
          dword flags;
          if (section_size >= 8)
          {
            Read_word_be(IFF_file, &version);
            Read_word_be(IFF_file, &frame_count);
            Read_dword_be(IFF_file, &flags);
            section_size -= 8;
          }
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip remaining bytes
        }
        else if (memcmp(section, "DLTA", 4) == 0) // Animation DeLTA
        {
          int i, plane;
          dword offsets[16];
          dword current_offset = 0;
          byte * frame;

          if (Image_HAM > 1)
          {
            GFX2_Log(GFX2_WARNING, "HAM animations are not supported\n");
            //Verbose_message("Notice", "HAM animations are not supported, loading only first frame"); // TODO: causes an issue with colors
            break;
          }
          if (frame_count != 0 && current_frame >= (frame_count - 1))
            break;  // some animations have delta from last to first frame
          if (previous_frame == NULL)
          {
            real_line_size = (context->Width+15) & ~15;
            plane_line_size = real_line_size >> 3;  // 8bits per byte
            line_size = plane_line_size * real_bit_planes;

            previous_frame = calloc(line_size * context->Height,1);
            for (y_pos=0; y_pos<context->Height; y_pos++)
            {
              const byte * pix_p = context->Target_address + y_pos * context->Pitch;
              // Dispatch the pixel into planes
              for (x_pos=0; x_pos<context->Width; x_pos++)
              {
                Set_IFF_color(previous_frame+y_pos*line_size, x_pos, *pix_p++, real_line_size, real_bit_planes);
              }
            }
            // many animations are designed for double buffering
            // and delta is against frame n-2
            anteprevious_frame = malloc(line_size * context->Height);
            memcpy(anteprevious_frame, previous_frame, line_size * context->Height);
          }

          Set_loading_layer(context, ++current_frame);
          Set_frame_duration(context, (aheader.reltime * 50) / 3 ); // convert 1/60th sec in msec
          frame = previous_frame;

          if(aheader.operation == 5)  // Byte Vertical Delta mode
          {
            if (aheader.interleave != 1)
              frame = anteprevious_frame;
            for (i = 0; i < 16; i++)
            {
              if (!Read_dword_be(IFF_file, offsets+i))
              {
                File_error = 2;
                break;
              }
              current_offset += 4;
            }
            for (plane = 0; plane < 16; plane++)
            {
              byte op_count = 0;
              if (offsets[plane] == 0)
                continue;
              if (plane >= real_bit_planes)
              {
                GFX2_Log(GFX2_WARNING, "IFF : too much bitplanes in DLTA data %u >= %u (frame #%u/%u)\n", plane, real_bit_planes, current_frame + 1, frame_count);
                break;
              }
              while (current_offset < offsets[plane])
              {
                Read_byte(IFF_file, &op_count);
                current_offset++;
              }
              if (current_offset > offsets[plane])
              {
                GFX2_Log(GFX2_ERROR, "Loading ERROR in DLTA data\n");
                File_error = 2;
                break;
              }
              for (x_pos = 0; x_pos < (context->Width+7) >> 3; x_pos++)
              {
                byte * p = frame + x_pos + plane * plane_line_size;
                y_pos = 0;
                Read_byte(IFF_file, &op_count);
                current_offset++;
                for (i = 0; i < op_count; i++)
                {
                  byte op;

                  if (y_pos >= context->Height)
                  {
                  }
                  Read_byte(IFF_file, &op);
                  current_offset++;
                  if (op == 0)
                  { // Same ops
                    byte countb, datab;
                    Read_byte(IFF_file, &countb);
                    Read_byte(IFF_file, &datab);
                    current_offset += 2;
                    while(countb > 0 && y_pos < context->Height)
                    {
                      if(aheader.bits & 2)  // XOR
                        *p ^= datab;
                      else                  // set
                        *p = datab;
                      p += line_size;
                      y_pos++;
                      countb--;
                    }
                  }
                  else if (op & 0x80)
                  { // Uniq Ops
                    op &= 0x7f;
                    while (op > 0)
                    {
                      byte datab;
                      Read_byte(IFF_file, &datab);
                      current_offset++;
                      if (y_pos < context->Height)
                      {
                        if(aheader.bits & 2)  // XOR
                          *p ^= datab;
                        else                  // set
                          *p = datab;
                        p += line_size;
                        y_pos++;
                      }
                      op--;
                    }
                  }
                  else
                  { // skip ops
                    p += op * line_size;
                    y_pos += op;
                  }
                }
                if (y_pos > context->Height)
                {
                }
              }
            }
          }
          else if(aheader.operation==74)
          {
            // from sources found on aminet :
            // http://aminet.net/package/gfx/conv/unmovie
            while (current_offset < section_size)
            {
              word change_type;
              word uni_flag;
              word y_size;
              word x_size;
              word num_blocks;
              word offset;
              word x_start, y_start;

              Read_word_be(IFF_file, &change_type);
              current_offset += 2;
              if (change_type == 0)
                break;
              else if (change_type == 1)
              { // "Wall"
                Read_word_be(IFF_file, &uni_flag);
                Read_word_be(IFF_file, &y_size);
                Read_word_be(IFF_file, &num_blocks);
                current_offset += 6;
                while (num_blocks-- > 0)
                {
                  Read_word_be(IFF_file, &offset);
                  current_offset += 2;
                  x_start = offset % plane_line_size;
                  y_start = offset / plane_line_size;
                  for (plane = 0; plane < real_bit_planes; plane++)
                  {
                    byte * p = frame + plane * plane_line_size;
                    p += x_start + y_start*line_size;
                    for (y_pos=0; y_pos < y_size; y_pos++)
                    {
                      byte value;
                      Read_byte(IFF_file, &value);
                      current_offset++;
                      if (uni_flag)
                        *p ^= value;
                      else
                        *p = value;
                      }
                      p += line_size;
                    }
                }
              }
              else if (change_type == 2)
              { // "Pile"
                Read_word_be(IFF_file, &uni_flag);
                Read_word_be(IFF_file, &y_size);
                Read_word_be(IFF_file, &x_size);
                Read_word_be(IFF_file, &num_blocks);
                current_offset += 8;
                while (num_blocks-- > 0)
                {
                  Read_word_be(IFF_file, &offset);
                  current_offset += 2;
                  x_start = offset % plane_line_size;
                  y_start = offset / plane_line_size;
                  for (plane = 0; plane < real_bit_planes; plane++)
                  {
                    byte * p = frame + plane * plane_line_size;
                    p += x_start + y_start*line_size;
                    for (y_pos=0; y_pos < y_size; y_pos++)
                    {
                      for (x_pos=0; x_pos < x_size; x_pos++)
                      {
                        byte value;
                        Read_byte(IFF_file, &value);
                        current_offset++;
                        if (uni_flag)
                          p[x_pos] ^= value;
                        else
                          p[x_pos] = value;
                      }
                      p += line_size;
                    }
                  }
                }
              }
              else
              {
                GFX2_Log(GFX2_ERROR, "Unknown change type in type 74 DLTA\n");
                File_error = 2;
                break;
              }
              if (current_offset & 1) // align to WORD boundary
              {
                byte junk;
                Read_byte(IFF_file, &junk);
                current_offset++;
              }
            }
          }
          else
          {
            GFX2_Log(GFX2_INFO, "IFF DLTA : Unsupported compression type %u\n", aheader.operation);
          }

          if (File_error == 0)
          {
            for (y_pos=0; y_pos<context->Height; y_pos++)
            {
              Draw_IFF_line(context, frame+line_size*y_pos,y_pos,real_line_size,real_bit_planes);
            }
          }
          if (aheader.operation == 5 && aheader.interleave != 1)
          {
            anteprevious_frame = previous_frame;
            previous_frame = frame;
          }
          if (current_offset&1)
          {
            byte dummy_byte;
            Read_byte(IFF_file, &dummy_byte);
            current_offset++;
          }
          section_size -= current_offset;
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip it
        }
        else if (memcmp(section, "ADAT", 4) == 0)
        { // Animation made with Deluxe Paint for Atari ST
          buffer = previous_frame;
          if (section_size > 0)
          {
            byte * p;
            byte data[2];
            word cmd_count;
            word cmd;
            signed char * commands;
            word count;

            y_pos = 0; x_pos = 0;
            Read_word_be(IFF_file,&cmd_count);
            cmd_count -= 2;
            commands = (signed char *)malloc(cmd_count);
            if (!Read_bytes(IFF_file,commands,cmd_count))
            {
              File_error = 31;
              break;
            }
            section_size -= (cmd_count + 2);
            for (cmd = 0; cmd < cmd_count && section_size > 0; cmd++)
            {
              if (commands[cmd] == 0)
              { // move pointer
                word offset;
                short x_ofs, y_ofs;
                Read_word_be(IFF_file,&offset);
                section_size -= 2;
                if((short)offset < 0) {
                  y_ofs = ((short)offset - plane_line_size + 1) / plane_line_size;
                  x_ofs = (short)offset - y_ofs * plane_line_size;
                } else {
                  x_ofs = offset % plane_line_size;
                  y_ofs = offset / plane_line_size;
                }
                y_pos += y_ofs;
                x_pos += x_ofs;
              }
              else if(commands[cmd] < 0)
              { // XOR with a string of Words
                if (commands[cmd] == -1 && section_size >= 2)
                {
                  Read_word_be(IFF_file,&count);
                  section_size -= 2;
                }
                else
                  count = -commands[cmd] - 1;
                while (count-- > 0 && y_pos < context->Height && section_size > 0)
                {
                  p = buffer+x_pos+y_pos*line_size+vdlt_plane*plane_line_size;
                  Read_bytes(IFF_file,data,2);
                  section_size -= 2;
                  p[0] ^= data[0];
                  p[1] ^= data[1];
                  y_pos++;
                }
              }
              else // commands[cmd] > 0
              { // XOR a word several times
                if (commands[cmd] == 1)
                {
                  Read_word_be(IFF_file,&count);
                  section_size -= 2;
                  if (section_size < 2)
                    break;
                }
                else
                  count = commands[cmd] - 1;
                Read_bytes(IFF_file,data,2);
                section_size -= 2;
                do
                {
                  p = buffer+x_pos+y_pos*line_size+vdlt_plane*plane_line_size;
                  p[0] ^= data[0];
                  p[1] ^= data[1];
                  y_pos++;
                }
                while (count-- > 0 && y_pos < context->Height);
              }
            }
            free(commands);
            if(cmd < (cmd_count-1) || section_size > 0)
            {
              GFX2_Log(GFX2_WARNING, "Early end in ADAT chunk\n");
            }
          }

          vdlt_plane++;
          if (vdlt_plane == real_bit_planes)
          {
            for (y_pos=0; y_pos<context->Height; y_pos++)
            {
              Draw_IFF_line(context, buffer+line_size*y_pos,y_pos,real_line_size,real_bit_planes);
            }
          }
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip remaining bytes
        }
        else if (memcmp(section, "CMAP", 4) == 0) // Colour MAP
        {
          nb_colors = section_size/3;

          if (nb_colors > 256)
          {
            File_error = 1;
            break;
          }
          GFX2_Log(GFX2_DEBUG, "IFF CMAP : %u colors (header.BitPlanes=%u)\n", nb_colors, header.BitPlanes);

          if (current_frame != 0)
            GFX2_Log(GFX2_WARNING, "IFF : One CMAP per frame is not supported (frame #%u/%u)\n", current_frame + 1, frame_count);
          if ((header.BitPlanes==6 && nb_colors==16) || (header.BitPlanes==8 && nb_colors==64))
          {
            Image_HAM=header.BitPlanes;
            bpp = 3 * (header.BitPlanes - 2); // HAM6 = 12bpp, HAM8 = 18bpp
          }

          if (Read_bytes(IFF_file,context->Palette,3*nb_colors))
          {
            section_size -= 3*nb_colors;
            if (nb_colors <= 64)
            {
              unsigned int i;
              int is_4bits_colors = 1;  // recoil also checks that header.Pad1 is not 0x80
              for(i = 0; i < nb_colors && is_4bits_colors; i++)
              {
                is_4bits_colors = is_4bits_colors && ((context->Palette[i].R & 0x0f) == 0)
                                                  && ((context->Palette[i].G & 0x0f) == 0)
                                                  && ((context->Palette[i].B & 0x0f) == 0);
              }
              if (is_4bits_colors)    // extend 4bits per component color palettes to 8bits per component
                for(i = 0; i < nb_colors; i++)
                {
                  context->Palette[i].R |= (context->Palette[i].R >> 4);
                  context->Palette[i].G |= (context->Palette[i].G >> 4);
                  context->Palette[i].B |= (context->Palette[i].B >> 4);
                }
            }
            if (((nb_colors==32) || (AmigaViewModes & 0x80)) && (header.BitPlanes==6))
            {
              IFF_Set_EHB_Palette(context->Palette); // This is a Extra Half-Brite (EHB) 64 color image.
              nb_colors = 64;
            }

            while(section_size > 0) // Read padding bytes
            {
              if (Read_byte(IFF_file,&temp_byte))
                File_error=20;
              section_size--;
            }
          }
          else
            File_error=1;
          if (context->Type == CONTEXT_PALETTE || context->Type == CONTEXT_PREVIEW_PALETTE)
            break;  // stop once the palette is loaded
        }
        else if (memcmp(section,"CRNG",4) == 0)
        {
          // The content of a CRNG is as follows:
          word padding;
          word rate;
          word flags;
          byte min_col;
          byte max_col;
          if ( (Read_word_be(IFF_file,&padding))
            && (Read_word_be(IFF_file,&rate))
            && (Read_word_be(IFF_file,&flags))
            && (Read_byte(IFF_file,&min_col))
            && (Read_byte(IFF_file,&max_col)))
          {
            GFX2_Log(GFX2_DEBUG, "IFF CRNG : [#%u #%u] rate=%u flags=%04x\n", min_col, max_col, rate, flags);
            if (section_size == 8 && min_col != max_col)
            {
              // Valid cycling range
              if (max_col<min_col)
                SWAP_BYTES(min_col,max_col)

              if (context->Color_cycles >= 16)
              {
                GFX2_Log(GFX2_INFO, "IFF CRNG : Maximum CRNG number is 16\n");
              }
              else
              {
                word speed;

                context->Cycle_range[context->Color_cycles].Start=min_col;
                context->Cycle_range[context->Color_cycles].End=max_col;
                context->Cycle_range[context->Color_cycles].Inverse=(flags&2)?1:0;
                speed = (flags&1) ? rate/78 : 0;
                context->Cycle_range[context->Color_cycles].Speed = (speed > COLOR_CYCLING_SPEED_MAX) ? COLOR_CYCLING_SPEED_MAX : speed;
                context->Color_cycles++;
              }
            }
          }
          else
            File_error=47;
        }
        else if (memcmp(section, "DRNG", 4) == 0) // DPaint IV enhanced color cycle chunk
        {
          /// @todo DRNG IFF chunk is read, but complex color cycling are not implemented.
          // see http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node0282.html
          byte min_col;
          byte max_col;
          word rate;
          word flags;
          byte ntruecolor;
          byte nregs;
          Read_byte(IFF_file, &min_col);
          Read_byte(IFF_file, &max_col);
          Read_word_be(IFF_file, &rate);
          Read_word_be(IFF_file, &flags);
          Read_byte(IFF_file, &ntruecolor);
          Read_byte(IFF_file, &nregs);
          section_size -= 8;
          GFX2_Log(GFX2_DEBUG, "IFF DRNG : [#%u #%u] rate=%u flags=%04x, %u tc cells, %u reg cells\n", min_col, max_col, rate, flags, ntruecolor, nregs);
          while (ntruecolor-- > 0 && section_size > 0)
          {
            byte cell, r, g, b;
            Read_byte(IFF_file, &cell);
            Read_byte(IFF_file, &r);
            Read_byte(IFF_file, &g);
            Read_byte(IFF_file, &b);
            section_size -= 4;
            GFX2_Log(GFX2_DEBUG, "          #%u #%02X%02X%02X\n", cell, r, g, b);
          }
          while (nregs-- > 0 && section_size > 0)
          {
            byte cell, index;
            Read_byte(IFF_file, &cell);
            Read_byte(IFF_file, &index);
            section_size -= 2;
            GFX2_Log(GFX2_DEBUG, "          #%u index %u\n", cell, index);
          }
          if (section_size > 0)
          {
            GFX2_Log(GFX2_DEBUG, "IFF DRNG : skipping %u bytes\n", section_size);
            fseek(IFF_file, section_size, SEEK_CUR);
          }
        }
        else if (memcmp(section, "CCRT", 4) == 0) // Color Cycling Range and Timing
        {
          // see http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node01BA.html
          word direction;
          byte start, end;
          dword seconds, microseconds;
          word pad;

          Read_word_be(IFF_file, &direction);
          Read_byte(IFF_file, &start);
          Read_byte(IFF_file, &end);
          Read_dword_be(IFF_file, &seconds);
          Read_dword_be(IFF_file, &microseconds);
          Read_word_be(IFF_file, &pad);
          section_size -= 14;
          GFX2_Log(GFX2_DEBUG, "IFF CCRT : dir %04x [#%u #%u] delay %u.%06u\n",
                   direction, start, end, seconds, microseconds);
          if (context->Color_cycles >= 16)
          {
            GFX2_Log(GFX2_INFO, "IFF CCRT : Maximum CRNG number is 16\n");
          }
          else if (start != end)
          {
            // Speed resolution is 0.2856Hz
            // Speed = (1000000/delay) / 0.2856 = 3501400 / delay

            context->Cycle_range[context->Color_cycles].Start = start;
            context->Cycle_range[context->Color_cycles].End = end;
            if (direction != 0)
            {
              dword speed;
              context->Cycle_range[context->Color_cycles].Inverse = (~direction >> 15) & 1;
              speed = 3501400 / (seconds * 1000000 + microseconds);
              context->Cycle_range[context->Color_cycles].Speed = (speed > COLOR_CYCLING_SPEED_MAX) ? COLOR_CYCLING_SPEED_MAX : speed;
            }
            else
            {
              context->Cycle_range[context->Color_cycles].Speed = 0;
            }

            context->Color_cycles++;
          }
        }
        else if (memcmp(section, "DPI ", 4) == 0)
        {
          word dpi_x, dpi_y;
          Read_word_be(IFF_file, &dpi_x);
          Read_word_be(IFF_file, &dpi_y);
          section_size -= 4;
          GFX2_Log(GFX2_DEBUG, "IFF DPI %hu x %hu\n", dpi_x, dpi_y);
        }
        else if (memcmp(section, "CAMG", 4) == 0) //  	Amiga Viewport Modes
        {
          Read_dword_be(IFF_file, &AmigaViewModes); // HIRES=0x8000 LACE=0x4  HAM=0x800  HALFBRITE=0x80
          section_size -= 4;
          GFX2_Log(GFX2_DEBUG, "CAMG = $%08x\n", AmigaViewModes);
          if (AmigaViewModes & 0x800 && (header.BitPlanes == 6 || header.BitPlanes == 8))
          {
            Image_HAM = header.BitPlanes;
            bpp = 3 * (header.BitPlanes - 2);
          }
          if ((AmigaViewModes & 0x80) && (header.BitPlanes == 6)) // This is a Extra Half-Brite (EHB) 64 color image.
          {
            IFF_Set_EHB_Palette(context->Palette); // Set the palette in case CAMG is after CMAP
            nb_colors = 64;
          }
        }
        else if (memcmp(section, "DPPV", 4) == 0) // DPaint II ILBM perspective chunk
        {
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip it
        }
        else if (memcmp(section, "CLUT", 4) == 0) // lookup table
        {
          dword lut_type; // 0 = A Monochrome, contrast or intensity LUT
          byte lut[256];  // 1 = RED, 2 = GREEN, 3 = BLUE, 4 = HUE, 5 = SATURATION

          Read_dword_be(IFF_file, &lut_type);
          Read_dword_be(IFF_file, &dummy);
          Read_bytes(IFF_file, lut, 256);
          section_size -= (4+4+256);
          if (section_size > 0)
          {
            GFX2_Log(GFX2_WARNING, "Extra bytes at the end of CLUT chunk\n");
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          }
        }
        else if (memcmp(section, "DYCP", 4) == 0) // DYnamic Color Palette
        {
          // All files I've seen have 4 words (8bytes) :
          // { 0, 1, 16, 0}   16 is probably the number of colors in each palette
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip it
        }
        else if (memcmp(section, "SHAM", 4) == 0) // Sliced HAM
        {
          int i;
          word version;
          dword SHAM_palette_count;
          T_IFF_PCHG_Palette * prev_pal = NULL;
          T_IFF_PCHG_Palette * new_pal = NULL;

          Image_HAM = header.BitPlanes;
          bpp = 3 * (header.BitPlanes - 2);
          Read_word_be(IFF_file, &version); // always 0
          section_size -= 2;
          SHAM_palette_count = section_size >> 5;  // 32 bytes per palette (16 colors * 2 bytes)
          // SHAM_palette_count should be the image height, or height/2 for "interlaced" images
          for (y_pos = 0; y_pos < header.Height && section_size >= 32; y_pos += (SHAM_palette_count < header.Height ? 2 : 1))
          {
            new_pal = GFX2_malloc(sizeof(T_IFF_PCHG_Palette) + nb_colors*sizeof(T_Components));
            if (new_pal == NULL)
            {
              File_error = 1;
              break;
            }
            new_pal->Next = NULL;
            new_pal->StartLine = y_pos;
            for (i = 0; i < 16; i++)
            {
              Read_byte(IFF_file, &temp_byte);  // 0R
              new_pal->Palette[i].R = (temp_byte & 0x0f) * 0x11; // 4 bits to 8 bits
              Read_byte(IFF_file, &temp_byte);  // GB
              new_pal->Palette[i].G = (temp_byte & 0xf0) | (temp_byte >> 4);
              new_pal->Palette[i].B = (temp_byte & 0x0f) * 0x11; // 4 bits to 8 bits
              section_size -= 2;
            }
            if (prev_pal != NULL)
              prev_pal->Next = new_pal;
            else
              PCHG_palettes = new_pal;
            prev_pal = new_pal;
          }
          if (section_size > 0)
          {
            GFX2_Log(GFX2_WARNING, "Extra bytes at the end of SHAM chunk\n");
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          }
        }
        else if (memcmp(section, "BEAM", 4) == 0 || memcmp(section, "CTBL", 4) == 0)
        {
          // One palette per line is stored
          if (section_size >= header.Height * nb_colors * 2)
          {
            T_Palette palette;
            T_IFF_PCHG_Palette * prev_pal = NULL;
            T_IFF_PCHG_Palette * new_pal = NULL;

            for (y_pos = 0; y_pos < header.Height; y_pos++)
            {
              unsigned int i;
              for (i = 0; i < nb_colors; i++)
              {
                word value;
                Read_word_be(IFF_file, &value);
                section_size -= 2;
                palette[i].R = ((value & 0x0f00) >> 8) * 0x11;
                palette[i].G = ((value & 0x00f0) >> 4) * 0x11;
                palette[i].B = (value & 0x000f) * 0x11;
              }
              if (y_pos == 0)
              {
                prev_pal = GFX2_malloc(sizeof(T_IFF_PCHG_Palette) + nb_colors*sizeof(T_Components));
                if (prev_pal == NULL)
                {
                  File_error = 1;
                  break;
                }
                prev_pal->Next = NULL;
                prev_pal->StartLine = 0;
                memcpy(prev_pal->Palette, palette, nb_colors*sizeof(T_Components));
                PCHG_palettes = prev_pal;
              }
              else if (memcmp(palette, prev_pal->Palette, nb_colors*sizeof(T_Components)) != 0)
              {
                new_pal = GFX2_malloc(sizeof(T_IFF_PCHG_Palette) + nb_colors*sizeof(T_Components));
                if (new_pal == NULL)
                {
                  File_error = 1;
                  break;
                }
                new_pal->Next = NULL;
                new_pal->StartLine = y_pos;
                memcpy(new_pal->Palette, palette, nb_colors*sizeof(T_Components));
                prev_pal->Next = new_pal;
                prev_pal = new_pal;
              }
            }
            if (PCHG_palettes != NULL)
              bpp = 12;
          }
          else
            GFX2_Log(GFX2_WARNING, "inconsistent size of BEAM/CTLB chunk, ignoring\n");
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
        }
        else if (memcmp(section, "PCHG", 4) == 0) // Palette CHanGes
        {
          dword * lineBitMask;
          int i;
          T_IFF_PCHG_Palette * prev_pal = NULL;
          T_IFF_PCHG_Palette * curr_pal = NULL;
          void * PCHGData = NULL;

          // http://wiki.amigaos.net/wiki/ILBM_IFF_Interleaved_Bitmap#ILBM.PCHG
          // http://vigna.di.unimi.it/software.php
          // Header
          word Compression; // 0 = None, 1 = Huffman
          word Flags;       // 0x1 = SmallLineChanges, 0x2 = BigLineChanges, 0x4 = use alpha
          short StartLine; // possibly negative
          word LineCount;
          word ChangedLines;
          word MinReg;
          word MaxReg;
          word MaxChanges;
          dword TotalChanges;

          if (!(Read_word_be(IFF_file, &Compression)
            && Read_word_be(IFF_file, &Flags)
            && Read_word_be(IFF_file, (word *)&StartLine)
            && Read_word_be(IFF_file, &LineCount)
            && Read_word_be(IFF_file, &ChangedLines)
            && Read_word_be(IFF_file, &MinReg)
            && Read_word_be(IFF_file, &MaxReg)
            && Read_word_be(IFF_file, &MaxChanges)
            && Read_dword_be(IFF_file, &TotalChanges)))
            File_error = 1;
          section_size -= 20;
          if (Compression)
          {
            short * TreeCode;
            const short * TreeP;    //a3
            int remaining_bits = 0; //d1
            dword src_dword = 0;    //d2
            byte * dst_data;        //a1

            dword CompInfoSize;
            dword OriginalDataSize; //d0

            Read_dword_be(IFF_file, &CompInfoSize);
            Read_dword_be(IFF_file, &OriginalDataSize);
            section_size -= 8;
            // read HuffMan tree
            TreeCode = malloc(CompInfoSize);
            for (i = 0; i < (int)(CompInfoSize / 2); i++)
              Read_word_be(IFF_file, (word *)(TreeCode + i));
            section_size -= CompInfoSize;

            PCHGData = malloc(OriginalDataSize);
            dst_data = PCHGData;
            // HuffMan depacking
            TreeP = TreeCode+(CompInfoSize/2-1); // pointer to the last element
            while (OriginalDataSize > 0)
            {
              if (--remaining_bits < 0)
              {
                Read_dword_be(IFF_file, &src_dword);
                section_size -= 4;
                remaining_bits = 31;
              }
              if (src_dword & (1 << remaining_bits))
              {
                if (*TreeP < 0)
                {
                  // OffsetPointer
                  TreeP += (*TreeP / 2);
                  continue; // pick another bit
                }
              }
              else
              {
                // Case0
                --TreeP;
                if ((*TreeP < 0) || !(*TreeP & 0x100))
                  continue; // pick another bit
              }
              // StoreValue
              *dst_data = (byte)(*TreeP & 0xff);
              dst_data++;
              TreeP = TreeCode+(CompInfoSize/2-1); // pointer to the last element
              OriginalDataSize--;
            }
            free(TreeCode);
          }
          else
          {
            PCHGData = malloc(section_size);
            Read_bytes(IFF_file, PCHGData, section_size);
            section_size = 0;
          }
          if (PCHGData != NULL)
          {
            const byte * data;
            // initialize first palette from CMAP
            prev_pal = malloc(sizeof(T_IFF_PCHG_Palette) + nb_colors*sizeof(T_Components));
            prev_pal->Next = NULL;
            prev_pal->StartLine = 0;
            memcpy(prev_pal->Palette, context->Palette, nb_colors*sizeof(T_Components));
            PCHG_palettes = prev_pal;

            lineBitMask = (dword *)PCHGData;
#if defined(SDL_BYTEORDER) && (SDL_BYTEORDER != SDL_BIG_ENDIAN)
            for (i = 0 ; i < ((LineCount + 31) >> 5); i++)
              lineBitMask[i] = SDL_Swap32(lineBitMask[i]);
#elif defined(BYTE_ORDER) && (BYTE_ORDER != BIG_ENDIAN)
            for (i = 0 ; i < ((LineCount + 31) >> 5); i++)
              lineBitMask[i] = be32toh(lineBitMask[i]);
#elif defined(WIN32)
            // assume WIN32 is little endian
            for (i = 0 ; i < ((LineCount + 31) >> 5); i++)
              lineBitMask[i] = WIN32_BSWAP32(lineBitMask[i]);
#endif
            data = (const byte *)PCHGData + ((LineCount + 31) >> 5) * 4;
            for (y_pos = 0 ; y_pos < LineCount; y_pos++)
            {
              if (lineBitMask[y_pos >> 5] & (1 << (31-(y_pos & 31))))
              {
                byte ChangeCount16, ChangeCount32;
                word PaletteChange;

                if ((y_pos + StartLine) < 0)
                  curr_pal = PCHG_palettes;
                else
                {
                  curr_pal = malloc(sizeof(T_IFF_PCHG_Palette) + nb_colors*sizeof(T_Components));
                  curr_pal->Next = NULL;
                  curr_pal->StartLine = StartLine + y_pos;
                  memcpy(curr_pal->Palette, prev_pal->Palette, nb_colors*sizeof(T_Components));
                  prev_pal->Next = curr_pal;
                }

                ChangeCount16 = *data++;
                ChangeCount32 = *data++;
                for (i = 0; i < ChangeCount16; i++)
                {
                  PaletteChange = data[0] << 8 | data[1]; // Big endian
                  data += 2;
                  curr_pal->Palette[(PaletteChange >> 12)].R = ((PaletteChange & 0x0f00) >> 8) * 0x11;
                  curr_pal->Palette[(PaletteChange >> 12)].G = ((PaletteChange & 0x00f0) >> 4) * 0x11;
                  curr_pal->Palette[(PaletteChange >> 12)].B = ((PaletteChange & 0x000f) >> 0) * 0x11;
                }
                for (i = 0; i < ChangeCount32; i++)
                {
                  PaletteChange = data[0] << 8 | data[1]; // Big endian
                  data += 2;
                  curr_pal->Palette[16+(PaletteChange >> 12)].R = ((PaletteChange & 0x0f00) >> 8) * 0x11;
                  curr_pal->Palette[16+(PaletteChange >> 12)].G = ((PaletteChange & 0x00f0) >> 4) * 0x11;
                  curr_pal->Palette[16+(PaletteChange >> 12)].B = ((PaletteChange & 0x000f) >> 0) * 0x11;
                }
                if (nb_colors == 64)  // Extend the 32 colors decoded to 64
                  IFF_Set_EHB_Palette(curr_pal->Palette);
                prev_pal = curr_pal;
              }
            }
            free(PCHGData);
          }
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          if (PCHG_palettes != NULL)
            bpp = 12;
        }
        else if (memcmp(section, "RAST", 4) == 0) // Atari ST
        {
          if (PCHG_palettes == NULL)
          {
            RAST_chunk_decode(context, IFF_file, section_size, &PCHG_palettes);
            if (PCHG_palettes != NULL)
              bpp = 12;
          }
          else
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR); // Skip
        }
        else if (memcmp(section, "TINY", 4) == 0)
        {
          word tiny_width, tiny_height;
          Read_word_be(IFF_file,&tiny_width);
          Read_word_be(IFF_file,&tiny_height);
          section_size -= 4;

          // Load thumbnail if in preview mode
          if ((context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE)
            && tiny_width > 0 && tiny_height > 0)
          {
            context->Original_width = header.Width;
            context->Original_height = header.Height;
            Pre_load(context, tiny_width, tiny_height,file_size,iff_format,ratio,bpp);
            context->Background_transparent = header.Mask == 2;
            context->Transparent_color = context->Background_transparent ? header.Transp_col : 0;
            if (iff_format == FORMAT_PBM)
              PBM_Decode(context, IFF_file, header.Compression, tiny_width, tiny_height);
            else
              LBM_Decode(context, IFF_file, header.Compression, Image_HAM, stored_bit_planes, real_bit_planes, PCHG_palettes);
            fclose(IFF_file);
            IFF_file = NULL;
            return;
          }
          else
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
        }
        else if (memcmp(section, "ANNO", 4) == 0)
        {
          dword length;
          section_size = (section_size + 1) & ~1;
          length = section_size;
          if (length > COMMENT_SIZE)
            length = COMMENT_SIZE;
          Read_bytes(IFF_file,context->Comment,length);
          context->Comment[length]='\0';
          section_size -= length;
          fseek(IFF_file, section_size, SEEK_CUR);
        }
        else if (memcmp(section, "ABIT", 4) == 0)
        {
          // ACBM format : ABIT = Amiga BITplanes
          // The ABIT chunk contains contiguous bitplane data.
          //  The chunk contains sequential data for bitplane 0 through bitplane n.
          if (header.Width == 0 || header.Height == 0)
            break;
          Pre_load(context, header.Width, header.Height, file_size, iff_format, ratio, bpp);
          // compute row size
          real_line_size = (context->Width+15) & ~15;
          plane_line_size = real_line_size >> 3;  // 8bits per byte
          line_size = plane_line_size * stored_bit_planes;
          buffer = malloc(line_size * context->Height);
          if ((dword)(line_size * context->Height) == section_size)
            header.Compression = 0;   // size is of uncompressed data. Forcing.
          for (plane = 0; plane < stored_bit_planes; plane++)
          {
            for (y_pos = 0; y_pos < context->Height; y_pos++)
            {
              if (header.Compression == 0)
              {
                if (!Read_bytes(IFF_file,buffer+line_size*y_pos+plane_line_size*plane,plane_line_size))
                {
                  File_error = 21;
                  break;
                }
              }
              else
              {
                GFX2_Log(GFX2_WARNING, "Unhandled compression %d for ACBM ABIT chunk\n", header.Compression);
                File_error = 32;
                break;
              }
            }
          }
          if (File_error == 0)
          {
            for (y_pos = 0; y_pos < context->Height; y_pos++)
            {
              if (Image_HAM <= 1)
                Draw_IFF_line(context, buffer+y_pos*line_size, y_pos,real_line_size, real_bit_planes);
              else
                Draw_IFF_line_HAM(context, buffer+y_pos*line_size, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
            }
          }
          free(buffer);
          buffer = NULL;
        }
        else if (memcmp(section, "BODY", 4) == 0)
        {
          long offset = ftell(IFF_file);
          if (file_size > (unsigned long)(offset + section_size + 8))
          {
            // Chunk RAST is placed AFTER the BODY, but we need the palette now to decode the image
            // In addition, some files break the IFF standard by not aligning
            // the chunk on word boundaries.
            fseek(IFF_file, section_size, SEEK_CUR);
            Read_bytes(IFF_file, section, 1);
            if (section[0] == 'R')                  // we are good
              Read_bytes(IFF_file, section + 1, 3); // read the remaining 3 bytes
            else                                    // skip 1 byte
              Read_bytes(IFF_file, section, 4);     // read 4 bytes
            if (memcmp(section, "RAST", 4) == 0)
            {
              dword rast_size;
              Read_dword_be(IFF_file, &rast_size);
              RAST_chunk_decode(context, IFF_file, rast_size, &PCHG_palettes);
              if (PCHG_palettes != NULL)
                bpp = 12;
            }
            fseek(IFF_file, offset, SEEK_SET);  // rewind
          }
          if (header.Width == 0 || header.Height == 0)
            break;
          Original_screen_X = header.X_screen;
          Original_screen_Y = header.Y_screen;

          Pre_load(context, header.Width, header.Height, file_size, iff_format, ratio, bpp);
          context->Background_transparent = header.Mask == 2;
          context->Transparent_color = context->Background_transparent ? header.Transp_col : 0;

          Set_image_mode(context, IMAGE_MODE_ANIMATION);

          if (iff_format == FORMAT_LBM)    // "ILBM": InterLeaved BitMap
          {
            LBM_Decode(context, IFF_file, header.Compression, Image_HAM, stored_bit_planes, real_bit_planes, PCHG_palettes);
          }
          else                               // "PBM ": Packed BitMap
          {
            PBM_Decode(context, IFF_file, header.Compression, context->Width, context->Height);
          }
          if (ftell(IFF_file) & 1)
            fseek(IFF_file, 1, SEEK_CUR); // SKIP one byte
          if (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE)
          {
            break; // dont load animations in Preview mode
          }
        }
        else
        {
          // skip section
          GFX2_Log(GFX2_INFO, "IFF : Skip unknown section '%.4s' of %u bytes at offset %06X\n", section, section_size, ftell(IFF_file));
          if (section_size < 256)
          {
            byte buffer[256];

            Read_bytes(IFF_file, buffer, (section_size+1)&~1);
            GFX2_LogHexDump(GFX2_DEBUG, "", buffer, 0, (section_size+1)&~1);
          }
          else
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
        }
      }
    }

    fclose(IFF_file);
    IFF_file = NULL;
  }
  else
    File_error=1;
  if (previous_frame)
    free(previous_frame);
  if (anteprevious_frame)
    free(anteprevious_frame);
  while (PCHG_palettes != NULL)
  {
    T_IFF_PCHG_Palette * next = PCHG_palettes->Next;
    free(PCHG_palettes);
    PCHG_palettes = next;
  }
}


// -- Sauver un fichier au format IFF ---------------------------------------

/// Save IFF file (LBM or PBM)
void Save_IFF(T_IO_Context * context)
{
  FILE * IFF_file;
  T_IFF_Header header;
  word x_pos;
  word y_pos;
  byte temp_byte;
  int i;
  int palette_entries;
  byte bit_depth;
  long body_offset = -1;
  int is_ehb = 0; // Extra half-bright

  if (context->Format == FORMAT_LBM)
  {
    // Check how many bits are used by pixel colors
    temp_byte = 0;
    for (y_pos=0; y_pos<context->Height; y_pos++)
      for (x_pos=0; x_pos<context->Width; x_pos++)
        temp_byte |= Get_pixel(context, x_pos,y_pos);
    bit_depth=0;
    do
    {
      bit_depth++;
      temp_byte>>=1;
    } while (temp_byte);
    if (bit_depth == 6)
    {
      for (i = 0; i < 32; i++)
      {
        if (   (context->Palette[i].R >> 5) != (context->Palette[i+32].R >> 4)
            || (context->Palette[i].G >> 5) != (context->Palette[i+32].G >> 4)
            || (context->Palette[i].B >> 5) != (context->Palette[i+32].B >> 4))
          break;
      }
      if (i == 32) is_ehb = 1;
    }
    GFX2_Log(GFX2_DEBUG, "Saving ILBM with bit_depth = %d %s\n", bit_depth,
             is_ehb ? "(Extra Half-Bright)" : "");
    palette_entries = 1 << (bit_depth - is_ehb);
  }
  else // FORMAT_PBM
  {
    bit_depth = 8;
    palette_entries = 256;
  }

  File_error=0;
  
  // Ouverture du fichier
  if ((IFF_file=Open_file_write(context)))
  {
    setvbuf(IFF_file, NULL, _IOFBF, 64*1024);
    
    Write_bytes(IFF_file,"FORM",4);
    Write_dword_be(IFF_file,0); // On mettra la taille à jour à la fin

    if (context->Format == FORMAT_LBM)
      Write_bytes(IFF_file,"ILBM",4);
    else
      Write_bytes(IFF_file,"PBM ",4);
      
    Write_bytes(IFF_file,"BMHD",4);
    Write_dword_be(IFF_file,20);

    header.Width=context->Width;
    header.Height=context->Height;
    header.X_org=0;
    header.Y_org=0;
    header.BitPlanes=bit_depth;
    header.Mask=context->Background_transparent ? 2 : 0;
    header.Compression=1;
    header.Pad1=0;
    header.Transp_col=context->Background_transparent ? context->Transparent_color : 0;
    header.X_aspect=10; // Amiga files are usually 10:11
    header.Y_aspect=10;
    switch (context->Ratio)
    {
      case PIXEL_SIMPLE:
      case PIXEL_DOUBLE:
      case PIXEL_TRIPLE:
      case PIXEL_QUAD:
      default:
        break;
      case PIXEL_WIDE:
      case PIXEL_WIDE2:
        header.X_aspect *= 2; // 2:1
        break;
      case PIXEL_TALL3:       // 3:4
        header.X_aspect = (header.X_aspect * 15) / 10; // *1.5
#if defined(__GNUC__) && (__GNUC__ >= 7)
        __attribute__ ((fallthrough));
#endif
      case PIXEL_TALL:
      case PIXEL_TALL2:
        header.Y_aspect *= 2; // 1:2
        break;
    }
    header.X_screen = context->Width;// Screen_width?;
    header.Y_screen = context->Height;// Screen_height?;

    Write_word_be(IFF_file,header.Width);
    Write_word_be(IFF_file,header.Height);
    Write_word_be(IFF_file,header.X_org);
    Write_word_be(IFF_file,header.Y_org);
    Write_bytes(IFF_file,&header.BitPlanes,1);
    Write_bytes(IFF_file,&header.Mask,1);
    Write_bytes(IFF_file,&header.Compression,1);
    Write_bytes(IFF_file,&header.Pad1,1);
    Write_word_be(IFF_file,header.Transp_col);
    Write_bytes(IFF_file,&header.X_aspect,1);
    Write_bytes(IFF_file,&header.Y_aspect,1);
    Write_word_be(IFF_file,header.X_screen);
    Write_word_be(IFF_file,header.Y_screen);

    if (context->Format == FORMAT_LBM)
    {
      dword ViewMode = 0; // HIRES=0x8000 LACE=0x4  HAM=0x800  HALFBRITE=0x80
      if (is_ehb) ViewMode |= 0x80;
      if (context->Width > 400)
      {
        ViewMode |= 0x8000;
        if (context->Width > 800)
          ViewMode |= 0x20; // Super High-Res
      }
      if (context->Height > 300) ViewMode |= 0x4;
      Write_bytes(IFF_file, "CAMG", 4);
      Write_dword_be(IFF_file, 4); // Section size
      Write_dword_be(IFF_file, ViewMode);
    }

    Write_bytes(IFF_file,"CMAP",4);
    Write_dword_be(IFF_file,palette_entries*3);

    Write_bytes(IFF_file,context->Palette,palette_entries*3);

    if (context->Comment[0]) // write ANNO
    {
      dword comment_size;

      Write_bytes(IFF_file,"ANNO",4); // Chunk name
      comment_size = strlen(context->Comment);  // NULL termination is not needed
      Write_dword_be(IFF_file, comment_size); // Section size
      Write_bytes(IFF_file, context->Comment, comment_size);
      if (comment_size&1)
        Write_byte(IFF_file, 0);  // align to WORD boundaries
    }
    
    for (i=0; i<context->Color_cycles; i++)
    {
      word flags=0;
      flags|= context->Cycle_range[i].Speed?1:0; // Cycling or not
      flags|= context->Cycle_range[i].Inverse?2:0; // Inverted
              
      Write_bytes(IFF_file,"CRNG",4);
      Write_dword_be(IFF_file,8); // Section size
      Write_word_be(IFF_file,0); // Padding
      Write_word_be(IFF_file,context->Cycle_range[i].Speed*78); // Rate
      Write_word_be(IFF_file,flags); // Flags
      Write_byte(IFF_file,context->Cycle_range[i].Start); // Min color
      Write_byte(IFF_file,context->Cycle_range[i].End); // Max color
      // No padding, size is multiple of 2
    }

    body_offset = ftell(IFF_file);
    Write_bytes(IFF_file,"BODY",4);
    Write_dword_be(IFF_file,0); // On mettra la taille à jour à la fin

    if (context->Format == FORMAT_LBM)
    {
      byte * buffer;
      short line_size; // Size of line in bytes
      short plane_line_size;  // Size of line in bytes for 1 plane
      short real_line_size; // Size of line in pixels
      T_PackBits_data pb_data;
      
      // Calcul de la taille d'une ligne ILBM (pour les images ayant des dimensions exotiques)
      real_line_size = (context->Width+15) & ~15;
      plane_line_size = real_line_size >> 3;  // 8bits per byte
      line_size = plane_line_size * header.BitPlanes;
      buffer=(byte *)malloc(line_size);
      
      // Start encoding
      PackBits_pack_init(&pb_data, IFF_file);
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        // Dispatch the pixel into planes
        memset(buffer,0,line_size);
        for (x_pos=0; x_pos<context->Width; x_pos++)
          Set_IFF_color(buffer, x_pos, Get_pixel(context, x_pos,y_pos), real_line_size, header.BitPlanes);
          
        if (context->Width&1) // odd width fix
          Set_IFF_color(buffer, x_pos, 0, real_line_size, header.BitPlanes);
        
        // encode the resulting sequence of bytes
        if (header.Compression)
        {
          int plane_width=line_size/header.BitPlanes;
          int plane;
          
          for (plane=0; plane<header.BitPlanes; plane++)
          {
            for (x_pos=0; x_pos<plane_width && !File_error; x_pos++)
            {
              if (PackBits_pack_add(&pb_data, buffer[x_pos+plane*plane_width]) < 0)
                File_error = 1;
            }

            if (!File_error)
            {
              if (PackBits_pack_flush(&pb_data) < 0)
                File_error = 1;
            }
          }
        }
        else
        {
          // No compression
          if (!Write_bytes(IFF_file, buffer, line_size))
            File_error = 1;
        }
      }
      free(buffer);
    }
    else // PBM = chunky 8bpp
    {
      T_PackBits_data pb_data;

      PackBits_pack_init(&pb_data, IFF_file);
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        for (x_pos=0; ((x_pos<context->Width) && (!File_error)); x_pos++)
        {
          if (PackBits_pack_add(&pb_data, Get_pixel(context, x_pos, y_pos)) < 0)
            File_error = 1;
        }
  
        if (context->Width & 1) // odd width fix
        {
          if (PackBits_pack_add(&pb_data, Get_pixel(context, context->Width - 1, y_pos)) < 0)
            File_error = 1;
        }
          
        if (!File_error)
        {
          if (PackBits_pack_flush(&pb_data) < 0)
            File_error = 1;
        }
      }
    }
    // Now update FORM and BODY size
    if (!File_error)
    {
      long file_size = ftell(IFF_file);
      if (file_size&1)
      {
        // PAD to even file size
        if (! Write_byte(IFF_file,0))
          File_error=1;
      }
      // Write BODY size
      fseek(IFF_file, body_offset + 4, SEEK_SET);
      Write_dword_be(IFF_file, file_size-body_offset-8);
      // Write FORM size
      file_size = (file_size+1)&~1;
      fseek(IFF_file,4,SEEK_SET);
      Write_dword_be(IFF_file,file_size-8);
    }
    fclose(IFF_file);
    if (File_error != 0) // remove the file if there have been an error
      Remove_file(context);
  }
  else
    File_error=1;
}

/** @} */
