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

///@file stformats.c
/// Formats for the Atari ST line of machines

#include <stdlib.h>
#include <string.h>
#include "fileformats.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "misc.h"
#include "gfx2log.h"
#include "gfx2mem.h"
#include "packbits.h"

/**
 * @defgroup atarist Atari ST picture formats
 * @ingroup loadsaveformats
 *
 * Support for Atari ST picture formats. The Atari ST has
 * 3 video modes :
 * - low res : 320x200 16 colors
 * - med res : 640x200 4 colors
 * - high res : 640x400 monochrome
 *
 * Supported formats :
 * - PI1 : Degas
 * - PC1 : Degas elite compressed
 * - NEO : Neochrome
 * - TNY : Tiny Stuff / Tiny View
 *
 * @{
 */
//////////////////////////////////// PI1 ////////////////////////////////////

//// DECODAGE d'une partie d'IMAGE ////

/**
 * 4bpp Planar to chunky conversion.
 *
 * Reads 16 pixels from 4 words with Atari ST low resolution layout.
 * @param src 8 bytes Atari ST screen memory
 * @param dest 16 bytes buffer
 */
static void PI1_8b_to_16p(const byte * src, byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de decodage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la source

  byte_mask=0x8000;
  w0=(((word)src[0])<<8) | src[1];
  w1=(((word)src[2])<<8) | src[3];
  w2=(((word)src[4])<<8) | src[5];
  w3=(((word)src[6])<<8) | src[7];
  for (i=0;i<16;i++)
  {
    // Pour décoder le pixel n°i, il faut traiter les 4 words sur leur bit
    // correspondant à celui du masque

    dest[i]=((w0 & byte_mask)?0x01:0x00) |
           ((w1 & byte_mask)?0x02:0x00) |
           ((w2 & byte_mask)?0x04:0x00) |
           ((w3 & byte_mask)?0x08:0x00);
    byte_mask>>=1;
  }
}

/**
 * 2bpp Planar to chunky conversion.
 *
 * Reads 16 pixels from 2 words with Atari ST medium resolution layout.
 * @param src 4 bytes Atari ST screen memory
 * @param dest 16 bytes buffer
 */
static void PI2_4b_to_16p(const byte * src, byte * dest)
{
  int  i;           // index du pixel à calculer
  word mask;      // Masque de decodage
  word w0,w1;

  w0=(((word)src[0])<<8) | src[1];
  w1=(((word)src[2])<<8) | src[3];
  mask=0x8000;
  for (i = 0; i < 16; i++)
  {
    dest[i] = ((w0 & mask) ? 0x01 : 0) | ((w1 & mask) ? 0x02 : 0);
    mask >>= 1;
  }
}

/**
 * Chunky to 4bpp planar conversion.
 *
 * For ST Low resolution.
 * @param src 16 bytes buffer
 * @param dest 8 bytes (4 words in Atari ST memory screen layout)
 */
static void PI1_16p_to_8b(const byte * src, byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de codage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la destination

  byte_mask=0x8000;
  w0=w1=w2=w3=0;
  for (i=0;i<16;i++)
  {
    // Pour coder le pixel n°i, il faut modifier les 4 words sur leur bit
    // correspondant à celui du masque

    w0|=(src[i] & 0x01)?byte_mask:0x00;
    w1|=(src[i] & 0x02)?byte_mask:0x00;
    w2|=(src[i] & 0x04)?byte_mask:0x00;
    w3|=(src[i] & 0x08)?byte_mask:0x00;
    byte_mask>>=1;
  }
  dest[0]=w0 >> 8;
  dest[1]=w0 & 0x00FF;
  dest[2]=w1 >> 8;
  dest[3]=w1 & 0x00FF;
  dest[4]=w2 >> 8;
  dest[5]=w2 & 0x00FF;
  dest[6]=w3 >> 8;
  dest[7]=w3 & 0x00FF;
}

/**
 * Decode the 16 colors Atari ST(E) palette
 *
 * @param src 32 bytes Atari ST(E) palette
 * @param palette GrafX2 palette structure
 */
static void PI1_decode_palette(const byte * src, T_Components * palette)
{
  int i;  // Numéro de la couleur traitée
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  for (i=0;i<16;i++)
  {
    w = (word)src[0] << 8 | (word)src[1];
    src += 2;

    palette[i].R = (((w & 0x0700)>>7) | ((w & 0x0800) >> 11)) * 0x11 ;
    palette[i].G = (((w & 0x0070)>>3) | ((w & 0x0080) >> 7)) * 0x11 ;
    palette[i].B = (((w & 0x0007)<<1) | ((w & 0x0008) >> 3)) * 0x11 ;
  }
}

/**
 * Code a 16 colors Atari ST(E) palette.
 *
 * @param palette GrafX2 palette structure
 * @param dest 32 bytes buffer
 */
void PI1_code_palette(const T_Components * palette, byte * dest)
{
  int i;  // Numéro de la couleur traitée
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  // Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  for (i=0;i<16;i++)
  {
    w  = ((word)(palette[i].R & 0xe0) << 3) | ((word)(palette[i].R & 0x10) << 7);
    w |= ((word)(palette[i].G & 0xe0) >> 1) | ((word)(palette[i].G & 0x10) << 3);
    w |= ((word)(palette[i].B & 0xe0) >> 5) | ((word)(palette[i].B & 0x10) >> 1);

    *dest++ = (w >> 8);
    *dest++ = (w & 0xff);
  }
}

/// Load color cycling data from a PI1 or PC1 image (Degas Elite format)
static void PI1_load_ranges(T_IO_Context * context, const byte * buffer, int size)
{
  int range;

  if (buffer==NULL || size<32)
    return;

  for (range=0; range < 4; range ++)
  {
    word min_col, max_col, direction, delay;

    min_col   = (buffer[size - 32 + range*2 +  0] << 8) | buffer[size - 32 + range*2 +  1];
    max_col   = (buffer[size - 32 + range*2 +  8] << 8) | buffer[size - 32 + range*2 +  9];
    direction = (buffer[size - 32 + range*2 + 16] << 8) | buffer[size - 32 + range*2 + 17];
    delay     = (buffer[size - 32 + range*2 + 24] << 8) | buffer[size - 32 + range*2 + 25];

    if (max_col < min_col)
      SWAP_WORDS(min_col,max_col)

    GFX2_Log(GFX2_DEBUG, "Degas Color cycling : [#%d:#%d] direction=%d delay=%d\n", min_col, max_col, direction, delay);
    // Sanity checks
    if (min_col < 256 && max_col < 256 && direction < 3 && (direction == 1 || delay < 128))
    {
      int speed = 1;
      if (delay < 128)
        speed = 210/(128-delay);
      // Grafx2's slider has a limit of COLOR_CYCLING_SPEED_MAX
      if (speed > COLOR_CYCLING_SPEED_MAX)
        speed = COLOR_CYCLING_SPEED_MAX;
      context->Cycle_range[context->Color_cycles].Start=min_col;
      context->Cycle_range[context->Color_cycles].End=max_col;
      context->Cycle_range[context->Color_cycles].Inverse= (direction==0);
      context->Cycle_range[context->Color_cycles].Speed=direction == 1 ? 0 : speed;
      context->Color_cycles++;
    }
  }
}

/// Saves color ranges from a PI1 or PC1 image (Degas Elite format)
void PI1_save_ranges(T_IO_Context * context, byte * buffer, int size)
{
  // empty by default
  memset(buffer+size - 32, 0, 32);
  if (context->Color_cycles)
  {
    int i; // index in context->Cycle_range[] : < context->Color_cycles
    int saved_range; // index in resulting buffer : < 4

    for (i=0, saved_range=0; i<context->Color_cycles && saved_range<4; i++)
    {
      if (context->Cycle_range[i].Start < 16 && context->Cycle_range[i].End < 16)
      {
        int speed;
        if (context->Cycle_range[i].Speed == 0)
          speed = 0;
        else if (context->Cycle_range[i].Speed == 1)
          // has to "round" manually to closest valid number for this format
          speed = 1;
        else
          speed = 128 - 210 / context->Cycle_range[i].Speed;

        buffer[size - 32 + saved_range*2 +  1] = context->Cycle_range[i].Start;
        buffer[size - 32 + saved_range*2 +  9] = context->Cycle_range[i].End;
        buffer[size - 32 + saved_range*2 + 17] = (context->Cycle_range[i].Speed == 0) ? 1 : (context->Cycle_range[i].Inverse ? 0 : 2);
        buffer[size - 32 + saved_range*2 + 25] = speed;

        saved_range ++;
      }
    }
  }
}

/// Test for Degas file format
void Test_PI1(T_IO_Context * context, FILE * file)
{
  unsigned long size;              // Taille du fichier
  word resolution;                 // Résolution de l'image

  (void)context;
  File_error=1;

  if (!Read_word_be(file,&resolution))
    return;

  size = File_length_file(file);
  if ((size==32034) || (size==32066)) // size check
  {
    if (resolution < 3)
      File_error=0;
  }
}


/// Load Degas file format
void Load_PI1(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  word resolution;
  word width, height;
  FILE *file;
  word x_pos,y_pos;
  byte buffer[160];
  byte * ptr;
  byte pixels[320];
  byte bpp;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;

  if (!Read_word_be(file, &resolution))
    return;
  GFX2_Log(GFX2_DEBUG, "Degas UnCompressed. Resolution = %04x\n", resolution);
  // Read palette
  if (!Read_bytes(file, buffer, 32))
  {
    fclose(file);
    return;
  }
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  switch (resolution)
  {
    case 0:  // Low Res
      width = 320;
      height = 200;
      bpp = 4;
      break;
    case 1:  // Med Res
      width = 640;
      height = 200;
      bpp = 2;
      ratio = PIXEL_TALL;
      break;
    case 2:  // High Res
      width = 640;
      height = 400;
      bpp = 1;
      break;
    default:
      fclose(file);
      return;
  }
  Pre_load(context, width, height, File_length_file(file),FORMAT_PI1,ratio, bpp);

  for (y_pos=0;y_pos<height;y_pos++)
  {
    if (!Read_bytes(file, buffer, (resolution == 2) ? 80 : 160))
    {
      fclose(file);
      return;
    }
    ptr = buffer;
    for (x_pos=0; x_pos < width;)
    {
      int i;
      switch (resolution)
      {
        case 0:
          PI1_8b_to_16p(ptr, pixels);
          ptr += 8;
          break;
        case 1:
          PI2_4b_to_16p(ptr, pixels);
          ptr += 4;
          break;
        case 2:
          for (i = 0; i < 8; i++)
            pixels[i] = (ptr[0] & (0x80 >> i)) ? 1 : 0;
          for (; i < 16; i++)
            pixels[i] = (ptr[1] & (0x80 >> (i - 8))) ? 1 : 0;
          ptr += 2;
      }
      for (i = 0; i < 16; i++)
        Set_pixel(context, x_pos++, y_pos, pixels[i]);
    }
  }
  // load color cycling information
  if (Read_bytes(file, buffer, 32))
  {
    PI1_load_ranges(context, buffer, 32);
  }
  fclose(file);
  File_error = 0;
}


/**
 * Save a PI1 Degas uncompressed picture
 *
 * @todo Support medium and high resolution
 */
void Save_PI1(T_IO_Context * context)
{
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  File_error=0;
  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    // allocation d'un buffer mémoire
    buffer = GFX2_malloc(32034);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_code_palette(context->Palette, buffer+2);
    // Codage de l'image
    ptr=buffer+34;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<context->Height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<context->Width);x_pos++)
          pixels[x_pos]=Get_pixel(context, x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    if (Write_bytes(file,buffer,32034))
    {
      if (context->Color_cycles)
      {
        PI1_save_ranges(context, buffer, 32);
        if (!Write_bytes(file,buffer,32))
          File_error=1;
      }
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      Remove_file(context);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
    buffer = NULL;
  }
  else
  {
    fclose(file);
    Remove_file(context);
    File_error=1;
  }
}


//////////////////////////////////// PC1 ////////////////////////////////////

/**
 * 4bpp planar to chunky conversion.
 * Converts 1 line (320 pixels) at a time.
 *
 * @param src0 40 bytes for plane 0
 * @param src1 40 bytes for plane 1
 * @param src2 40 bytes for plane 2
 * @param src3 40 bytes for plane 3
 * @param dest 320 bytes destination buffer
 */
static void PC1_4bp_to_1line(const byte * src0, const byte * src1,
                             const byte * src2, const byte * src3, byte * dest)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    b0=src0[i];
    b1=src1[i];
    b2=src2[i];
    b3=src3[i];
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    for (j=0;j<8;j++)
    {
      dest[ip++]=((b0 & byte_mask)?0x01:0x00) |
                ((b1 & byte_mask)?0x02:0x00) |
                ((b2 & byte_mask)?0x04:0x00) |
                ((b3 & byte_mask)?0x08:0x00);
      byte_mask>>=1;
    }
  }
}

// Transformation d'1 ligne de pixels en 4 plans de bits

/**
 * Convert 1 line of 320 pixel from chunky to 4bpp planar.
 *
 * @param src 320 bytes buffer
 * @param dst0 40 bytes buffer for plane 0
 * @param dst1 40 bytes buffer for plane 1
 * @param dst2 40 bytes buffer for plane 2
 * @param dst3 40 bytes buffer for plane 3
 */
static void PC1_1line_to_4bp(const byte * src,
                             byte * dst0, byte * dst1,
                             byte * dst2, byte * dst3)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    b0=b1=b2=b3=0;
    for (j=0;j<8;j++)
    {
      b0|=(src[ip] & 0x01)?byte_mask:0x00;
      b1|=(src[ip] & 0x02)?byte_mask:0x00;
      b2|=(src[ip] & 0x04)?byte_mask:0x00;
      b3|=(src[ip] & 0x08)?byte_mask:0x00;
      ip++;
      byte_mask>>=1;
    }
    dst0[i]=b0;
    dst1[i]=b1;
    dst2[i]=b2;
    dst3[i]=b3;
  }
}


/// Test for Degas Elite Compressed format
void Test_PC1(T_IO_Context * context, FILE * file)
{
  int  size;              // Taille du fichier
  word resolution;        // Résolution de l'image

  (void)context;
  File_error=1;

  size = File_length_file(file);
  if (!Read_word_be(file,&resolution))
    return;

  if ((size <= 32066) && (resolution & 0x8000))
  {
    if ((resolution & 0x7fff) < 3)
      File_error=0;
  }
}


/// Load Degas Elite compressed files
void Load_PC1(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  unsigned long size;
  word width, height;
  byte bpp;
  FILE *file;
  word x_pos,y_pos;
  byte buffer[32];
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[640];
  word resolution;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  size = File_length_file(file);

  if (!Read_word_be(file, &resolution))
    return;
  GFX2_Log(GFX2_DEBUG, "Degas Elite Compressed. Resolution = %04x\n", resolution);
  // Read palette
  if (!Read_bytes(file, buffer, 32))
  {
    fclose(file);
    return;
  }
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  switch (resolution)
  {
    case 0x8000:  // Low Res
      width = 320;
      height = 200;
      bpp = 4;
      break;
    case 0x8001:  // Med Res
      width = 640;
      height = 200;
      bpp = 2;
      ratio = PIXEL_TALL;
      break;
    case 0x8002:  // High Res
      width = 640;
      height = 400;
      bpp = 1;
      break;
    default:
      fclose(file);
      return;
  }

  bufferdecomp = GFX2_malloc(32000);
  if (bufferdecomp == NULL)
  {
    fclose(file);
    return;
  }

  // Initialisation de la preview
  Pre_load(context, width, height, size, FORMAT_PC1, ratio, bpp);

  if (PackBits_unpack_from_file(file, bufferdecomp, 32000) != PACKBITS_UNPACK_OK)
  {
    GFX2_Log(GFX2_INFO, "PackBits_unpack_from_file() failed\n");
    free(bufferdecomp);
    fclose(file);
    return;
  }

  // Décodage de l'image
  ptr=bufferdecomp;
  for (y_pos = 0; y_pos < height; y_pos++)
  {
    // Décodage de la scanline
    switch (resolution)
    {
      case 0x8000:  // Low Res
        PC1_4bp_to_1line(ptr,ptr+40,ptr+80,ptr+120,pixels);
        ptr+=160;
        break;
      case 0x8001:  // Med Res
        x_pos = 0;
        while (x_pos < width)
        {
          int i;
          for (i = 7; i >= 0; i--, x_pos++)
            pixels[x_pos] =  ((ptr[0] >> i) & 1)
                            | (((ptr[80] >> i) << 1) & 2);
          ptr++;
        }
        ptr += 80;
        break;
      case 0x8002:  // High Res
        x_pos = 0;
        while (x_pos < width)
        {
          int i;
          for (i = 7; i >= 0; i--)
            pixels[x_pos++] = (*ptr >> i) & 1;
          ptr++;
        }
    }
    for (x_pos=0;x_pos<width;x_pos++)
      Set_pixel(context, x_pos, y_pos, pixels[x_pos]);
  }
  // Try to load color cycling information
  GFX2_Log(GFX2_DEBUG, "remaining bytes = %ld\n", size - ftell(file));
  if (Read_bytes(file, buffer, 32))
  {
    PI1_load_ranges(context, buffer, 32);
  }
  free(bufferdecomp);
  fclose(file);
  File_error = 0;
}


/**
 * Save a Degas Elite compressed picture
 *
 * @todo support medium and high resolution
  */
void Save_PC1(T_IO_Context * context)
{
  FILE *file;
  short x_pos,y_pos;
  byte buffer[32];
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  File_error=0;
  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    // Allocation des buffers mémoire
    bufferdecomp = GFX2_malloc(32000);
    // Codage de la résolution
    if (!Write_word_be(file, 0x8000))
      File_error = 1;
    // Codage de la palette
    PI1_code_palette(context->Palette, buffer);
    if (!Write_bytes(file, buffer, 32))
      File_error = 1;

    // Codage de l'image
    ptr=bufferdecomp;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<context->Height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<context->Width);x_pos++)
          pixels[x_pos]=Get_pixel(context, x_pos,y_pos);
      }

      // Encodage de la scanline
      PC1_1line_to_4bp(pixels,ptr,ptr+40,ptr+80,ptr+120);
      ptr+=160;
    }

    // Compression du buffer
    if (PackBits_pack_buffer(file, bufferdecomp, 32000) < 0)
      File_error = 1;

    PI1_save_ranges(context, buffer, 32);

    if (Write_bytes(file, buffer, 32))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      Remove_file(context);
      File_error=1;
    }
    // Libération des buffers mémoire
    free(bufferdecomp);
    bufferdecomp = NULL;
  }
  else
  {
    fclose(file);
    Remove_file(context);
    File_error=1;
  }
}


//////////////////////////////////// NEO ////////////////////////////////////
/**
NeoChrome Format :
<pre>
1 word          flag word [always 0]
1 word          resolution [0 = low res, 1 = medium res, 2 = high res]
16 words        palette
12 bytes        filename [usually "        .   "]
1 word          color animation limits.  High bit (bit 15) set if color
                animation data is valid.  Low byte contains color animation
                limits (4 most significant bits are left/lower limit,
                4 least significant bits are right/upper limit).
1 word          color animation speed and direction.  High bit (bit 15) set
                if animation is on.  Low order byte is # vblanks per step.
                If negative, scroll is left (decreasing).  Number of vblanks
                between cycles is |x| - 1
1 word          # of color steps (as defined in previous word) to display
                picture before going to the next.  (For use in slide shows)
1 word          image X offset [unused, always 0]
1 word          image Y offset [unused, always 0]
1 word          image width [unused, always 320]
1 word          image height [unused, always 200]
33 words        reserved for future expansion
32000 bytes     pixel data
</pre>

Dali           *.SD0 (ST low resolution)
               *.SD1 (ST medium resolution)
               *.SD2 (ST high resolution)

Files do not seem to have any resolution or bit plane info stored in them. The file
extension seems to be the only way to determine the contents.

1 long         file id? [always 0]
16 words       palette
92 bytes       reserved? [usually 0]
*/
void Test_NEO(T_IO_Context * context, FILE * file)
{
  word flag;
  word resolution;   // Atari ST resolution

  (void)context;
  File_error=1;

  if (File_length_file(file) != 32128)
    return;

  if (!Read_word_be(file,&flag))
    return;
  // Flag word : always 0
  if (flag != 0)
    return;

  if (!Read_word_be(file,&resolution))
    return;
  // 0 = STlow, 1 = STmed, 2 = SThigh
  if (resolution==0 || resolution==1 || resolution==2)
    File_error = 0;
}


/// Load Neochrome file format
void Load_NEO(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  word flag;
  word resolution;   // Atari ST resolution
  word width, height;
  byte bpp;
  word color_cycling_range, color_cycling_delay;
  word display_time;
  word image_width, image_height, image_X_pos, image_Y_pos;
  FILE *file;
  word x_pos,y_pos;
  byte * ptr;
  byte buffer[160];
  byte pixels[16];

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;

  if (!Read_word_be(file,&flag))
    goto error;
  // Flag word : always 0
  if (flag != 0)
    goto error;

  if (!Read_word_be(file,&resolution))
    goto error;

  switch (resolution)
  {
  case 0:
    width = 320;
    height = 200;
    bpp = 4;
    break;
  case 1:
    width = 640;
    height = 200;
    bpp = 2;
    ratio = PIXEL_TALL;
    break;
  case 2:
    width = 640;
    height = 400;
    bpp = 1;
    break;
  default:
    goto error;
  }

  Pre_load(context, width, height, File_length_file(file), FORMAT_NEO, ratio, bpp);

  if (!Read_bytes(file,buffer,32))
    goto error;

  // Initialisation de la palette
  if (Config.Clear_palette)
    memset(context->Palette, 0, sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  if (!Read_bytes(file, buffer, 12))
    goto error;
  buffer[12] = '\0';
  GFX2_Log(GFX2_DEBUG, "NEO resolution %u name=\"%s\"\n", resolution, (char *)buffer);
  if (!Read_word_be(file, &color_cycling_range)
     || !Read_word_be(file, &color_cycling_delay)
     || !Read_word_be(file, &display_time))
    goto error;
  GFX2_Log(GFX2_DEBUG, "  Color cycling : %04x %04x. Time to show %u\n", color_cycling_range, color_cycling_delay, display_time);
  if (color_cycling_range & 0x8000)
  {
    context->Cycle_range[context->Color_cycles].Start = (color_cycling_range & 0x00f0) >> 4;
    context->Cycle_range[context->Color_cycles].End = (color_cycling_range & 0x000f);
    if (color_cycling_delay & 0x8000)
    {
      // color cycling on
      color_cycling_delay &= 0xff;
      if (color_cycling_delay & 0x0080)
      {
        context->Cycle_range[context->Color_cycles].Inverse = 1;
        color_cycling_delay = 256 - color_cycling_delay;
      }
      else
        context->Cycle_range[context->Color_cycles].Inverse = 0;
      // Speed resolution is 0.2856Hz
      // NEO color_cycling_delay is in 50Hz VBL
      // Speed = (50/delay) / 0.2856 = 175 / delay
      if (color_cycling_delay != 0)
        context->Cycle_range[context->Color_cycles].Speed = 175 / color_cycling_delay;
      else
        context->Cycle_range[context->Color_cycles].Speed = COLOR_CYCLING_SPEED_MAX; // fastest
      if (context->Cycle_range[context->Color_cycles].Speed > COLOR_CYCLING_SPEED_MAX)
        context->Cycle_range[context->Color_cycles].Speed = COLOR_CYCLING_SPEED_MAX;
    }
    else
      context->Cycle_range[context->Color_cycles].Speed = 0;  // cycling off
    context->Color_cycles++;
  }

  if (!Read_word_be(file, &image_X_pos) || !Read_word_be(file, &image_Y_pos)
      || !Read_word_be(file, &image_width) || !Read_word_be(file, &image_height))
    goto error;
  GFX2_Log(GFX2_DEBUG, "  pos (%u,%u) size %ux%u\n", image_X_pos, image_Y_pos, image_width, image_height);
  if (!Read_bytes(file, buffer, 128-4-32-12-6-8))
    goto error;
  GFX2_LogHexDump(GFX2_DEBUG, "NEO ", buffer, 0, 128-4-32-12-6-8);

  // Chargement/décompression de l'image
  for (y_pos=0;y_pos<height;y_pos++)
  {
    if (!Read_bytes(file, buffer, (resolution==2) ? 80 : 160))
      goto error;

    ptr = buffer;
    for (x_pos = 0; x_pos < width; )
    {
      int i;
      switch (resolution)
      {
        case 0:
          PI1_8b_to_16p(ptr, pixels);
          ptr += 8;
          break;
        case 1:
          PI2_4b_to_16p(ptr, pixels);
          ptr += 4;
          break;
        case 2:
          for (i = 0; i < 8; i++)
            pixels[i] = (ptr[0] & (0x80 >> i)) ? 1 : 0;
          for (; i < 16; i++)
            pixels[i] = (ptr[1] & (0x80 >> (i - 8))) ? 1 : 0;
          ptr += 2;
          break;
        default:
          goto error;
      }
      for (i = 0; i < 16; i++)
        Set_pixel(context, x_pos++, y_pos, pixels[i]);
    }
  }
  File_error = 0; // everything was ok

error:
  fclose(file);
}

/**
 * Save in NeoChrome format
 *
 * @todo support medium and high resolution
 */
void Save_NEO(T_IO_Context * context)
{
  word resolution = 0;
  FILE *file = NULL;
  short x_pos,y_pos;
  word color_cycling_range = 0, color_cycling_delay = 0;
  word display_time = 0;
  word image_width = 320, image_height = 200;
  byte buffer[32];
  byte pixels[320];
  char * ext;
  int i, j;

  File_error = 1;
  file = Open_file_write(context);
  if (file == NULL)
    return;

  // flags and resolution
  if (!Write_word_be(file, 0) || !Write_word_be(file, resolution))
    goto error;

  // palette
  PI1_code_palette(context->Palette, buffer);
  if (!Write_bytes(file, buffer, 16*2))
    goto error;

  // file name
  i = 0;
  j = 0;
  ext = strrchr(context->File_name, '.');
  while (j < 8 && ext != (context->File_name + i))
  {
    byte c = context->File_name[i++];
    if (c == 0)
      break;
    if (c >= 'a' && c <= 'z')
      c -= 32;
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
      buffer[j++] = c;
  }
  while (j < 8)
    buffer[j++] = ' ';
  buffer[j++] = '.';
  if (ext != NULL)
  {
    i = 0;
    while (j < 12)
    {
      byte c = ext[i++];
      if (c == 0)
        break;
      if (c >= 'a' && c <= 'z')
        c -= 32;
      if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
        buffer[j++] = c;
    }
  }
  while (j < 12)
    buffer[j++] = ' ';

  if (!Write_bytes(file, buffer, 12))
    goto error;

  // Save the 1st valid Color cycling range
  for (i = 0; i < context->Color_cycles; i++)
  {
    if (context->Cycle_range[i].Start < 16 && context->Cycle_range[i].End < 16)
    {
      color_cycling_range = 0x8000 | (context->Cycle_range[i].Start << 4) | context->Cycle_range[i].End;
      if (context->Cycle_range[i].Speed > 0)
      {
        color_cycling_delay = 175 / context->Cycle_range[i].Speed;
        if (color_cycling_delay > 0 && context->Cycle_range[i].Inverse)
          color_cycling_delay = 256 - color_cycling_delay;
        color_cycling_delay |= 0x8000;
      }
      break;
    }
  }
  if (!Write_word_be(file, color_cycling_range) || !Write_word_be(file, color_cycling_delay) || !Write_word_be(file, display_time))
    goto error;

  // Save image position and size
  if (!Write_word_be(file, 0) || !Write_word_be(file, 0)
      || !Write_word_be(file, image_width) || !Write_word_be(file, image_height))
    goto error;

  // Fill with 128 bytes header with 0's
  // a few files have the string "NEO!" at offset 124 (0x7C)
  for (i = ftell(file); i < 128; i++)
  {
    if (!Write_byte(file, 0))
      goto error;
  }

  // image coding
  for (y_pos=0;y_pos<200;y_pos++)
  {
    // Codage de la ligne
    memset(pixels, 0, 320);
    if (y_pos < context->Height)
    {
      for (x_pos = 0; (x_pos < 320) && (x_pos < context->Width); x_pos++)
        pixels[x_pos] = Get_pixel(context, x_pos, y_pos);
    }

    for (x_pos=0; x_pos < 320; x_pos += 16)
    {
      PI1_16p_to_8b(pixels + x_pos, buffer);
      if (!Write_bytes(file, buffer, 8))
        goto error;
    }
  }

  fclose(file);
  File_error = 0;
  return;

error:
  if (file != NULL)
    fclose(file);
  Remove_file(context);
}

void Test_TNY(T_IO_Context * context, FILE * file)
{
  unsigned long file_size;
  unsigned long theorical_size;
  byte res;  // 0 = Low, 1 = midres, 2 = hires. +3 for color cycling
  word control_bytes;
  word data_words;
  (void)context;

  File_error = 1;
  file_size = File_length_file(file);
  if (file_size > 32044)
    return;
  if (!Read_byte(file, &res))
    return;
  if (res >= 6)
    return;
  if (res >= 3)
  {
    if (fseek(file, 4, SEEK_CUR) < 0)  // skip color cycling info
      return;
    theorical_size = 41;
  }
  else
    theorical_size = 37;
  if (fseek(file, 16*2, SEEK_CUR) < 0) // skip palette
    return;
  if (!Read_word_be(file, &control_bytes) || !Read_word_be(file, &data_words))
    return;
  theorical_size += control_bytes + data_words * 2;
  if (theorical_size <= file_size && file_size < theorical_size + 512)
  {
    GFX2_Log(GFX2_DEBUG, "TNY res=%d %hu control bytes, %hu data words\n",
             (int)res, control_bytes, data_words);
    File_error = 0;
  }
}

void Load_TNY(T_IO_Context * context)
{
  FILE * file;
  byte res;  // 0 = Low, 1 = midres, 2 = hires. +3 for color cycling
  word control_bytes;
  byte * control;
  word data_words;
  byte * data;
  byte buffer[32000 + 2];

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  if (Read_byte(file, &res) && (res < 6))
  {
    enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
    word width = 640, height = 200;
    byte bpp = 4;
    byte cycling_range = 0;
    byte cycling_speed = 0;
    word duration = 0;

    switch (res)
    {
      case 0:
      case 3:
        width = 320;
        break;
      case 1:
      case 4:
        bpp = 2;
        ratio = PIXEL_TALL;
        break;
      case 2:
      case 5:
        bpp = 1;
        height = 400;
        break;
    }
    if (res >= 3)
    {
      if(   !Read_byte(file, &cycling_range)
         || !Read_byte(file, &cycling_speed)
         || !Read_word_be(file, &duration))
      {
        fclose(file);
        return;
      }
      GFX2_Log(GFX2_DEBUG, "TNY Color Cycling : %02x speed=%02x duration=%d\n",
               cycling_range, cycling_speed, duration);
    }
    // Read palette
    if (   !Read_bytes(file, buffer, 32)
        || !Read_word_be(file, &control_bytes)
        || !Read_word_be(file, &data_words))
    {
      fclose(file);
      return;
    }
    control = GFX2_malloc(control_bytes);
    data = GFX2_malloc(data_words * 2);
    if (control == NULL || data == NULL)
    {
      fclose(file);
      return;
    }
    if (   Read_bytes(file, control, control_bytes)
        && Read_bytes(file, data, data_words * 2))
    {
      byte cb;
      word cc, count;
      int src, dst;
      int line;

      File_error = 0;
      Pre_load(context, width, height, File_length_file(file), FORMAT_TNY, ratio, bpp);
      // Set palette
      if (Config.Clear_palette)
        memset(context->Palette, 0, sizeof(T_Palette));
      PI1_decode_palette(buffer, context->Palette);
      if (res >= 3)
      {
        context->Cycle_range[context->Color_cycles].Start = (cycling_range & 0xf0) >> 4;
        context->Cycle_range[context->Color_cycles].End = (cycling_range & 0x0f);
        if (cycling_speed & 0x80)
        {
          context->Cycle_range[context->Color_cycles].Inverse = 1;
          cycling_speed = 256 - cycling_speed;
        }
        else
          context->Cycle_range[context->Color_cycles].Inverse = 0;
        context->Cycle_range[context->Color_cycles].Speed = 175 / cycling_speed;
        context->Color_cycles++;
      }
      for (cc = 0, src = 0, dst = 0; cc < control_bytes && dst < 32000; )
      {
        cb = control[cc++];
        if (cb & 0x80)
        {
          // copy
          do {
            buffer[dst++] = data[src++];
            buffer[dst++] = data[src++];
          } while(++cb != 0);
        }
        else switch(cb)
        {
          case 0: // repeat (word count)
            count = control[cc++] << 8;
            count += control[cc++];
            while (count-- > 0)
            {
              buffer[dst++] = data[src];
              buffer[dst++] = data[src+1];
            }
            src += 2;
            break;
          case 1: // copy (word count)
            count = control[cc++] << 8;
            count += control[cc++];
            memcpy(buffer+dst, data+src, count * 2);
            dst += count * 2;
            src += count * 2;
            break;
          default: // repeat next data word
            while(cb-- > 0)
            {
              buffer[dst++] = data[src];
              buffer[dst++] = data[src+1];
            }
            src += 2;
        }
      }
      GFX2_Log(GFX2_DEBUG, "    %d %d ; %d %d ; %d\n", cc, control_bytes, src, data_words * 2, dst);
      /**
       * Tiny stuff packs the ST RAM in 4 planes of 8000 bytes
       * Data is organized by column in each plane
       * Warning : this is the same organization for all 3 ST Video modes
       */
      for (line = 0; line < 200; line++)
      {
        int col;
        for (col = 0; col < 20; col++)
        {
          byte planar[8];
          byte pixels[16];
          src = (line + col * 200) * 2;
          for (dst = 0; dst < 8;)
          {
            planar[dst++] = buffer[src];
            planar[dst++] = buffer[src+1];
            src += 8000;
          }
          switch(res)
          {
            case 0:
            case 3:
              PI1_8b_to_16p(planar, pixels);
              for (dst = 0 ; dst < 16; dst++)
                Set_pixel(context, col * 16 + dst, line, pixels[dst]);
              break;
            case 1:
            case 4:
              PI2_4b_to_16p(planar, pixels);
              for (dst = 0 ; dst < 16; dst++)
                Set_pixel(context, col * 32 + dst, line, pixels[dst]);
              PI2_4b_to_16p(planar + 4, pixels);
              for (dst = 0 ; dst < 16; dst++)
                Set_pixel(context, col * 32 + 16 + dst, line, pixels[dst]);
              break;
            case 2:
            case 5:
              for (dst = 0 ; dst < 64; dst++)
                Set_pixel(context, (col % 10) * 64 + dst, line * 2 + (col / 10),
                          (planar[(dst >> 3)] >> (7 - (dst & 7))) & 1);
          }
        }
      }
    }
    free(control);
    free(data);
  }
  fclose(file);
}

/**
 * Save in Tiny compressed format.
 *
 * @todo support medium and high resolution
 */
void Save_TNY(T_IO_Context * context)
{
  byte res = 0;
  FILE * file;
  byte buffer[32000];
  byte control[16000];
  byte data[32000];
  word cc = 0, dc = 0;  // control count, data count
  int line, dst, src;

  File_error = 1;
  file = Open_file_write(context);
  if (file == NULL)
    return;

  // TODO : detect color cycling
  if (!Write_byte(file, res)) // resolution +3 with color cycling
    goto error;

  // palette
  PI1_code_palette(context->Palette, buffer);
  if (!Write_bytes(file, buffer, 16*2))
    goto error;

  /**
   * fill the buffer with the special Tiny Stuff organization
   * @see Load_TNY
   */
  for (line = 0; line < 200; line++)
  {
    int col;
    for (col = 0; col < 20; col++)
    {
      byte planar[8];

      // Low res
      PI1_16p_to_8b(context->Target_address + line * context->Pitch + col*16, planar);
      dst = (line + col * 200) * 2;
      for (src = 0; src < 8;)
      {
        buffer[dst] = planar[src++];
        buffer[dst+1] = planar[src++];
        dst += 8000;
      }
    }
  }

#define WORDS_EQU(p1, p2) (((p1)[0] == (p2)[0]) && ((p1)[1] == (p2)[1]))

  // now the compression
  for (src = 0; src < 32000; )
  {
    word count;

    // count repeat
    count = 0;
    while ((src + count * 2) < 32000 && WORDS_EQU(buffer + src, buffer + src + count * 2))
      count++;
    if (count > 127)
    {
      //GFX2_Log(GFX2_DEBUG, "%5d REPEAT %d %02x%02x\n", src, count, buffer[src+1], buffer[src]);
      control[cc++] = 0;  // repeat word
      control[cc++] = count >> 8;
      control[cc++] = count & 0xff;
      data[dc * 2] = buffer[src];
      data[dc * 2 + 1] = buffer[src+1];
      dc++;
      src += count * 2;
      count = 1;
    }
    else if (count > 1)
    {
      // TODO: merge a repeat count of 2 between 2 copy count ?
      //GFX2_Log(GFX2_DEBUG, "%5d REPEAT %d %02x%02x\n", src, count, buffer[src+1], buffer[src]);
      control[cc++] = (byte)count;
      data[dc * 2] = buffer[src];
      data[dc * 2 + 1] = buffer[src+1];
      dc++;
      src += count * 2;
      count = 1;
    }
    if (src >= 32000)
      break;
    // count copy
    while ((src + count * 2) < 32000 && !WORDS_EQU(buffer + src + (count - 1) * 2, buffer + src + count * 2))
      count++;

    if ((src + count * 2) < 32000)
      count--;
    if (count > 128)
    {
      //GFX2_Log(GFX2_DEBUG, "%5d COPY  %d %02x%02x %02x%02x...\n",
      //         src, count, buffer[src+1], buffer[src], buffer[src+3], buffer[src+2]);
      control[cc++] = 1;  // copy word
      control[cc++] = count >> 8;
      control[cc++] = count & 0xff;
      memcpy(data + dc * 2, buffer + src, count * 2);
      dc += count;
      src += count * 2;
    }
    else if (count > 0)
    {
      //GFX2_Log(GFX2_DEBUG, "%5d COPY  %d %02x%02x ...\n",
      //         src, count, buffer[src+1], buffer[src]);
      control[cc++] = (byte)(256 - count);  // copy byte
      memcpy(data + dc * 2, buffer + src, count * 2);
      dc += count;
      src += count * 2;
    }
  }

  if (!Write_word_be(file, cc) || !Write_word_be(file, dc))
    goto error;
  if (!Write_bytes(file, control, cc) || !Write_bytes(file, data, dc * 2))
    goto error;
  fclose(file);
  File_error = 0;
  return;
error:
  if (file != NULL)
    fclose(file);
  Remove_file(context);
}

/**
 * test for CrackArt format.
 *
 * Test that the files starts with the "CA" signature,
 * then 1 byte for the compressed flag (0 or 1),
 * then 1 byte for the resolution (0=low, 1=med, 2=high)
 */
void Test_CA1(T_IO_Context * context, FILE * file)
{
  byte header[4];

  (void)context;
  File_error = 1;
  if (!Read_bytes(file, header, 4))
    return;
  if (header[0] == 'C' && header[1] == 'A')
  {
    if ((header[2] & 0xfe) == 0 && (header[3] < 3))
      File_error = 0;
  }
}

void Load_CA1(T_IO_Context * context)
{
  FILE * file;
  byte sig[2];
  byte compressed;
  byte res;
  byte * buffer;

  File_error = 1;
  buffer = GFX2_malloc(32000);
  if (buffer == NULL)
    return;
  file = Open_file_read(context);
  if (file == NULL)
  {
    free(buffer);
    return;
  }
  if (Read_bytes(file, sig, 2) && Read_byte(file, &compressed)
      && Read_byte(file, &res))
  {
    unsigned long file_size;
    short width = 640, height = 200;
    enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
    byte bpp = 4;

    file_size = File_length_file(file);
    GFX2_Log(GFX2_DEBUG, "Signature : '%c%c' %s res=%d\n",
             sig[0], sig[1], compressed ? "compressed" : "", res);
    switch (res)
    {
      case 0:
        width = 320;
        break;
      case 1:
        ratio = PIXEL_TALL;
        bpp = 2;
        break;
      case 2:
        height = 400;
        bpp = 1;
    }
    File_error = 0;

    Pre_load(context, width, height, file_size, FORMAT_CA1, ratio, bpp);
    if (File_error == 0)
    {
      if (Config.Clear_palette)
        memset(context->Palette,0,sizeof(T_Palette));
      memset(buffer, 0, 32);
      if (res == 2)
      {
        // black & white
        buffer[0] = 0xff;
        buffer[1] = 0xff;
      }
      else
      {
        if (!Read_bytes(file, buffer, 1 << (bpp + 1)))
          File_error = 1;
      }
      PI1_decode_palette(buffer, context->Palette);
      if (compressed)
      {
        byte escape, delta;
        word offset;

        if (!(Read_byte(file, &escape) && Read_byte(file, &delta) && Read_word_be(file, &offset)))
          File_error = 1;
        else if(offset != 0)
        {
          int i = 0, c = 0;

          GFX2_Log(GFX2_DEBUG, "  escape=%02X delta=%02X offset=%hu\n", escape, delta, offset);
          memset(buffer, delta, 32000);
          while (c < 32000 && File_error == 0)
          {
            byte cmd, data;
            word repeat;

            repeat = 0;
            data = delta;
            if (!Read_byte(file, &cmd))
              File_error = 1;
            if (cmd == escape)
            {
              if (!Read_byte(file, &cmd))
                File_error = 1;
              if (cmd == 0)
              {
                // byte count repeat
                if (!Read_byte(file, &cmd) || !Read_byte(file, &data))
                  File_error = 1;
                repeat = cmd;
                GFX2_Log(GFX2_DEBUG, "byte count repeat : 0x%02x,0x%02x,%hu,0x%02x\n", escape, 0, repeat, data);
              }
              else if (cmd == 1)
              {
                // word count repeat
                if (!Read_word_be(file, &repeat) || !Read_byte(file, &data))
                  File_error = 1;
                GFX2_Log(GFX2_DEBUG, "word count repeat : 0x%02x,0x%02x,%hu,0x%02x\n", escape, cmd, repeat, data);
              }
              else if (cmd == 2)
              {
                if (!Read_byte(file, &cmd))
                  File_error = 1;
                else if (cmd == 0)
                {
                  // ESC,02,00 => STOP code
                  GFX2_Log(GFX2_DEBUG, "STOP : 0x%02x,0x02,%02x\n", escape, cmd);
                  break;
                }
                else
                {
                  // ESC,02,a,b => repeat (a << 8 + b + 1) x byte "delta"
                  repeat = cmd << 8;
                  if (!Read_byte(file, &cmd))
                    File_error = 1;
                  repeat += cmd;
                  GFX2_Log(GFX2_DEBUG, "delta repeat : 0x%02x,%hu\n", escape, repeat);
                }
              }
              else if (cmd == escape)
              {
                // ESC,ESC => 1 x byte "ESC"
                data = cmd;
              }
              else
              {
                // ESC,a,b => repeat (a + 1) x byte b
                repeat = cmd;
                if (!Read_byte(file, &data))
                  File_error = 1;
              }
            }
            else
            {
              data = cmd;
            }
            // output bytes
            do
            {
              buffer[i] = data;
              i += offset;
              c++;
              if (i >= 32000)
                i -= (32000 - 1);
            }
            while (repeat-- > 0);
          }
          GFX2_Log(GFX2_DEBUG, "finished : i=%d c=%d\n", i, c);
        }
      }
      else
      {
        // not compressed
        if (!Read_bytes(file, buffer, 32000))
          File_error = 1;
      }
      if (File_error == 0)
      {
        int line;
        const byte * ptr = buffer;
        int ncols;

        ncols = (res == 0) ? 20 : 40;

        for (line = 0; line < height; line++)
        {
          byte pixels[16];
          int col, x;

          for (col = 0; col < ncols; col++)
          {
            switch (res)
            {
              case 0:
                PI1_8b_to_16p(ptr, pixels);
                ptr += 8;
                for (x = 0; x < 16; x++)
                  Set_pixel(context, col * 16 + x, line, pixels[x]);
                break;
              case 1:
                PI2_4b_to_16p(ptr, pixels);
                ptr += 4;
                for (x = 0; x < 16; x++)
                  Set_pixel(context, col * 16 + x, line, pixels[x]);
                break;
              case 2:
                for (x = 0 ; x < 16; x++)
                  Set_pixel(context, col * 16 + x, line, (ptr[(x >> 3)] >> (7 - (x & 7))) & 1);
                ptr += 2;
            }
          }
        }
      }
    }
  }
  fclose(file);
  free(buffer);
}

/**
 * Save a 320x200 16c picture in CrackArt format
 *
 * @see http://www.atari-wiki.com/index.php/CrackArt_file_format
 * @todo support medium and high resolution
 */
void Save_CA1(T_IO_Context * context)
{
  FILE * file;
  byte * buffer;
  byte res = 0;   // 0 = low, 1 = med, 2 = high
  byte compressed = 1;  // 0 or 1
  int height = 200;

  File_error = 1;
  buffer = GFX2_malloc(32000);
  if (buffer == NULL)
    return;
  file = Open_file_write(context);
  if (file == NULL)
  {
    free(buffer);
    return;
  }
  if (Write_bytes(file, "CA", 2) && Write_byte(file, compressed) && Write_byte(file, res))
  {
    PI1_code_palette(context->Palette, buffer);
    if (Write_bytes(file, buffer, 32))
    {
      int line;
      byte * ptr = buffer;

      for (line = 0; line < height; line++)
      {
        byte pixels[16];
        int col, x;

        for (col = 0; col < 20; col++)
        {
          for (x = 0; x < 16; x++)
            pixels[x] = Get_pixel(context, col * 16 + x, line);
          PI1_16p_to_8b(pixels, ptr);
          ptr += 8;
        }
      }

      if (compressed)
      {
        word freq[256];
        word max, min;
        byte max_index, min_index;
        byte escape;
        word offset;
        int i;

        memset(freq, 0, sizeof(freq));
        for (i = 0; i < 32000; i++)
          freq[buffer[i]]++;
        min = 65535;
        min_index = 0;
        max = 0;
        max_index = 0;
        for (i = 0; i < 256; i++)
        {
          if (freq[i] <= min && i > 2)
          {
            min = freq[i];
            min_index = (byte)i;
          }
          if (freq[i] > max)
          {
            max = freq[i];
            max_index = (byte)i;
          }
        }
        GFX2_Log(GFX2_DEBUG, " 0x%02X (%hu times)   0x%02X (%hu times)\n", min_index, min, max_index, max);
        escape = min_index;
        offset = 160; // 80 in high res
        if (Write_byte(file, escape) && Write_byte(file, max_index) && Write_word_be(file, offset))
        {
          int c = 1;
          byte current;
          word count = 0;
          File_error = 0;
          i = offset;
          current = buffer[0];
          while (c < 32000 && File_error == 0)
          {
            if (buffer[i] == current)
              count++;
            else
            {
              if (count < 3)
              {
                // Litteral
                do
                {
                  if (!Write_byte(file, current))
                    File_error = 1;
                  if (current == escape)
                    Write_byte(file, current);
                }
                while (count-- && File_error == 0);
              }
              else
              {
                //GFX2_Log(GFX2_DEBUG, "byte %02X x %hu\n", current, count);
                if (count < 256)
                {
                  // ESC,a,b => repeat (a + 1) x byte b
                  // with a > 2
                  if (!(Write_byte(file, escape)
                     && Write_byte(file, count)
                     && Write_byte(file, current)))
                    File_error = 1;
                }
                else if (current == max_index)
                {
                  // ESC,02,word count => repeat (count + 1) x byte "delta"
                  if (!(Write_byte(file, escape)
                     && Write_byte(file, 2)
                     && Write_word_be(file, count)))
                    File_error = 1;
                }
                else
                {
                  // ESC,01,word count,data
                  if (!(Write_byte(file, escape)
                     && Write_byte(file, 1)
                     && Write_word_be(file, count)
                     && Write_byte(file, current)))
                    File_error = 1;
                }
              }
              current = buffer[i];
              count = 0;
            }
            i += offset;
            c++;
            if (i >= 32000)
              i -= (32000 - 1);
          }
          GFX2_Log(GFX2_DEBUG, "end: byte %02X x %hu\n", current, count);
          if (count < 3)
          {
            do
            {
              Write_byte(file, current);
              if (current == escape)
                Write_byte(file, current);
            }
            while (count--);
          }
          else if (current == max_index)
          {
            // STOP code
            if (!(Write_byte(file, escape)
               && Write_byte(file, 2)
               && Write_byte(file, 0)))
              File_error = 1;
          }
          else if (count < 256)
          {
            // ESC,a,b => repeat (a + 1) x byte b
            // with a > 2
            if (!(Write_byte(file, escape)
               && Write_byte(file, count)
               && Write_byte(file, current)))
              File_error = 1;
          }
          else
          {
            // ESC,01,word count,data
            if (!(Write_byte(file, escape)
               && Write_byte(file, 1)
               && Write_word_be(file, count)
               && Write_byte(file, current)))
              File_error = 1;
          }
        }
      }
      else
      {
        // uncompressed
        if (Write_bytes(file, buffer, 32000))
          File_error = 0;
      }
    }
  }
  fclose(file);
  free(buffer);
}

/* @} */
