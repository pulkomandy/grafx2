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

///@file fileformats.c
/// Saving and loading different picture formats.

#include <string.h>
#ifndef __no_pnglib__
// just for png_sig_cmp()
#include <png.h>
#endif

#include <stdlib.h>
#if defined(_MSC_VER)
#include <stdio.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#if !defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2)
#if defined(__macosx__)
#include <machine/endian.h>
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#endif

#include "gfx2log.h"
#include "errors.h"
#include "global.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "misc.h"
#include "struct.h"
#include "io.h"
#include "pages.h"
#include "windows.h" // Best_color()
#include "unicode.h"
#include "fileformats.h"
#include "oldies.h"
#include "bitcount.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

void Draw_IFF_line(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes);

//////////////////////////////////// IMG ////////////////////////////////////

// -- Tester si un fichier est au format IMG --------------------------------
void Test_IMG(T_IO_Context * context, FILE * file)
{
  T_IMG_Header IMG_header;
  static const byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  (void)context;
  File_error=1;

    // Lecture et vérification de la signature
  if (Read_bytes(file,IMG_header.Filler1,6)
    && Read_word_le(file,&(IMG_header.Width))
    && Read_word_le(file,&(IMG_header.Height))
    && Read_bytes(file,IMG_header.Filler2,118)
    && Read_bytes(file,IMG_header.Palette,sizeof(T_Palette))
    )
  {
    if ( (!memcmp(IMG_header.Filler1,signature,6))
      && IMG_header.Width && IMG_header.Height)
      File_error=0;
  }
}


// -- Lire un fichier au format IMG -----------------------------------------
void Load_IMG(T_IO_Context * context)
{
  byte * buffer;
  FILE *file;
  word x_pos,y_pos;
  long file_size;
  T_IMG_Header IMG_header;

  File_error=0;

  if ((file=Open_file_read(context)))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,IMG_header.Filler1,6)
    && Read_word_le(file,&(IMG_header.Width))
    && Read_word_le(file,&(IMG_header.Height))
    && Read_bytes(file,IMG_header.Filler2,118)
    && Read_bytes(file,IMG_header.Palette,sizeof(T_Palette))
    )
    {

      buffer=(byte *)malloc(IMG_header.Width);

      Pre_load(context, IMG_header.Width,IMG_header.Height,file_size,FORMAT_IMG,PIXEL_SIMPLE,0);
      if (File_error==0)
      {
        memcpy(context->Palette,IMG_header.Palette,sizeof(T_Palette));

        for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
        {
          if (Read_bytes(file,buffer,context->Width))
          {
            for (x_pos=0; x_pos<context->Width;x_pos++)
              Set_pixel(context, x_pos,y_pos,buffer[x_pos]);
          }
          else
            File_error=2;
        }
      }

      free(buffer);
      buffer = NULL;
    }
    else
      File_error=1;

    fclose(file);
  }
  else
    File_error=1;
}

// -- Sauver un fichier au format IMG ---------------------------------------
void Save_IMG(T_IO_Context * context)
{
  FILE *file;
  short x_pos,y_pos;
  T_IMG_Header IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  File_error=0;

  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);
    
    memcpy(IMG_header.Filler1,signature,6);

    IMG_header.Width=context->Width;
    IMG_header.Height=context->Height;

    memset(IMG_header.Filler2,0,118);
    IMG_header.Filler2[4]=0xFF;
    IMG_header.Filler2[22]=64; // Lo(Longueur de la signature)
    IMG_header.Filler2[23]=0;  // Hi(Longueur de la signature)
    memcpy(IMG_header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

    memcpy(IMG_header.Palette,context->Palette,sizeof(T_Palette));

    if (Write_bytes(file,IMG_header.Filler1,6)
    && Write_word_le(file,IMG_header.Width)
    && Write_word_le(file,IMG_header.Height)
    && Write_bytes(file,IMG_header.Filler2,118)
    && Write_bytes(file,IMG_header.Palette,sizeof(T_Palette))
    )

    {
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
        for (x_pos=0; x_pos<context->Width; x_pos++)
          Write_one_byte(file,Get_pixel(context, x_pos,y_pos));

      fclose(file);

      if (File_error)
        Remove_file(context);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      Remove_file(context);
      File_error=1;
    }
  }
  else
  {
    File_error=1;
  }
}


/////////////////////////// .info (Amiga Icons) /////////////////////////////
typedef struct
{                     // offset
  word Magic;         // 0
  word Version;
  dword NextGadget;
  word LeftEdge;      // 8
  word TopEdge;
  word Width;
  word Height;
  word Flags;         // 16
  word Activation;
  word GadgetType;    // 20
  dword GadgetRender; // 22
  dword SelectRender;
  dword GadgetText;   // 30
  dword MutualExclude;
  dword SpecialInfo;  // 38
  word GadgetID;
  dword UserData;     // 44 icon revision : 0 = OS 1.x, 1 = OS 2.x/3.x
  byte Type;
  byte padding;
  dword DefaultTool;
  dword ToolTypes;
  dword CurrentX;
  dword CurrentY;
  dword DrawerData;
  dword ToolWindow;
  dword StackSize;
} T_INFO_Header;

static int Read_INFO_Header(FILE * file, T_INFO_Header * header)
{
  return (Read_word_be(file, &header->Magic)
       && Read_word_be(file, &header->Version)
       && Read_dword_be(file, &header->NextGadget)
       && Read_word_be(file, &header->LeftEdge)
       && Read_word_be(file, &header->TopEdge)
       && Read_word_be(file, &header->Width)
       && Read_word_be(file, &header->Height)
       && Read_word_be(file, &header->Flags)
       && Read_word_be(file, &header->Activation)
       && Read_word_be(file, &header->GadgetType)
       && Read_dword_be(file, &header->GadgetRender)
       && Read_dword_be(file, &header->SelectRender)
       && Read_dword_be(file, &header->GadgetText)
       && Read_dword_be(file, &header->MutualExclude)
       && Read_dword_be(file, &header->SpecialInfo)
       && Read_word_be(file, &header->GadgetID)
       && Read_dword_be(file, &header->UserData)
       && Read_byte(file, &header->Type)
       && Read_byte(file, &header->padding)
       && Read_dword_be(file, &header->DefaultTool)
       && Read_dword_be(file, &header->ToolTypes)
       && Read_dword_be(file, &header->CurrentX)
       && Read_dword_be(file, &header->CurrentY)
       && Read_dword_be(file, &header->DrawerData)
       && Read_dword_be(file, &header->ToolWindow)
       && Read_dword_be(file, &header->StackSize)
          );
}

typedef struct
{
  short LeftEdge;
  short TopEdge;
  word Width;
  word Height;
  word Depth;
  dword ImageData;
  byte PlanePick;
  byte PlaneOnOff;
  dword NextImage;
} T_INFO_ImageHeader;

static int Read_INFO_ImageHeader(FILE * file, T_INFO_ImageHeader * header)
{
  return (Read_word_be(file, (word *)&header->LeftEdge)
       && Read_word_be(file, (word *)&header->TopEdge)
       && Read_word_be(file, &header->Width)
       && Read_word_be(file, &header->Height)
       && Read_word_be(file, &header->Depth)
       && Read_dword_be(file, &header->ImageData)
       && Read_byte(file, &header->PlanePick)
       && Read_byte(file, &header->PlaneOnOff)
       && Read_dword_be(file, &header->NextImage)
          );
}

void Test_INFO(T_IO_Context * context, FILE * file)
{
  T_INFO_Header header;

  (void)context;
  File_error=1;

  if (Read_INFO_Header(file, &header))
  {
    if (header.Magic == 0xe310 && header.Version == 1)
      File_error = 0;
  }
}

static char * Read_INFO_String(FILE * file)
{
  dword size;
  char * p;
  if (!Read_dword_be(file, &size))
    return NULL;
  p = malloc(size);
  if (p == NULL)
    return NULL;
  if (!Read_bytes(file, p, size))
  {
    free(p);
    p = NULL;
  }
  return p;
}

static byte * Decode_NewIcons(const byte * p, int bits, unsigned int * len)
{
  int alloc_size;
  unsigned int i;
  byte * buffer;
  dword current_byte = 0;
  int current_bits = 0;

  alloc_size = 16*1024;
  buffer = malloc(alloc_size);
  if (buffer == NULL)
    return NULL;
  i = 0;
  while (*p)
  {
    byte value = *p++;
    if (0xd1 <= value)
    {
      value -= 0xd0; // RLE count
      while (value-- > 0)
      {
        current_byte = (current_byte << 7);
        current_bits += 7;
        while (current_bits >= bits)
        {
          buffer[i++] = (current_byte >> (current_bits - bits)) & ((1 << bits) - 1);
          current_bits -= bits;
        }
      }
      continue;
    }
    if (0x20 <= value && value <= 0x6f)
      value = value - 0x20;
    else if (0xa1 <= value && value <= 0xd0)
      value = value - 0x51;
    else
    {
      Warning("Invalid value");
      break;
    }
    current_byte = (current_byte << 7) | value;
    current_bits += 7;
    while (current_bits >= bits)
    {
      buffer[i++] = (current_byte >> (current_bits - bits)) & ((1 << bits) - 1);
      current_bits -= bits;
    }
  }
  //buffer[i++] = (current_byte << (bits - current_bits)) & ((1 << bits) - 1);
  *len = i;
  return buffer;
}


void Load_INFO(T_IO_Context * context)
{
  static const T_Components amigaOS1x_pal[] = {
//    {  85, 170, 255 },
    {   0, 85, 170  },
    { 255, 255, 255 },
    {   0,   0,   0 },
    { 255, 136,   0 },
  };
  static const T_Components amigaOS2x_pal[] = {
    { 149, 149, 149 },
    {   0,   0,   0 },
    { 255, 255, 255 },
    {  59, 103, 162 },
    { 123, 123, 123 },  // MagicWB extended colors
    { 175, 175, 175 },
    { 170, 144, 124 },
    { 255, 169, 151 },
    { 255,   0,   0 }, //
    { 160,   0,   0 }, //
    {   0, 246, 255 }, //
    {   0,   0, 255 }, //
    {   0, 160,   0 }, //
    {   0, 255,   0 }, //
    { 255, 255,   0 }, //
    { 255, 160,   0 }, //
  };
  T_INFO_Header header;
  T_INFO_ImageHeader imgheaders[2];
  FILE *file;
  long file_size;
  word plane_line_size = 0;
  word line_size = 0;
  int plane;
  short x_pos = 0, y_pos = 0;
  int has_NewIcons = 0;
  int img_count = 0; // 1 or 2
  byte * buffers[2];

  File_error = 0;

  file = Open_file_read(context);
  if (file == NULL)
  {
    File_error=1;
    return;
  }
  file_size = File_length_file(file);
  
  if (Read_INFO_Header(file, &header) && header.Magic == 0xe310)
  {
    if (header.GadgetRender == 0)
      File_error = 1;
    if (header.DrawerData)    // Skip DrawerData
      if (fseek(file, 56, SEEK_CUR) != 0)
        File_error = 1;

    // icons
    for (img_count = 0; File_error == 0 && img_count < 2; img_count++)
    {
      buffers[img_count] = NULL;
      if (img_count == 0 && header.GadgetRender == 0)
        continue;
      if (img_count == 1 && header.SelectRender == 0)
        break;
      if (!Read_INFO_ImageHeader(file, &imgheaders[img_count]))
        File_error = 1;
      else
      {
        plane_line_size = ((imgheaders[img_count].Width + 15) >> 4) * 2;
        line_size = plane_line_size * imgheaders[img_count].Depth;
        buffers[img_count] = malloc(line_size * imgheaders[img_count].Height);
        for (plane = 0; plane < imgheaders[img_count].Depth; plane++)
        {
          for (y_pos = 0; y_pos < imgheaders[img_count].Height; y_pos++)
          {
            if (!Read_bytes(file, buffers[img_count] + y_pos * line_size + plane * plane_line_size, plane_line_size))
            {
              File_error = 2;
              break;
            }
          }
        }
      }
    }
    if (File_error == 0 && header.DefaultTool)
    {
      char * DefaultTool = Read_INFO_String(file);
      if (DefaultTool == NULL)
        File_error = 2;
      else
      {
        free(DefaultTool);
      }
    }
    if (File_error == 0 && header.ToolTypes)
    {
      int i;
      int current_img = -1;
      int width = 0;
      int height = 0;
      int palette_continues = 0;
      unsigned int color_count = 0;
      unsigned int palette_color_count = 0;  // used colors in global palette
      int bpp = 0;
      byte palette_conv[256]; // to translate sub icon specific palette to global palette
      
      dword count;
      char * ToolType;
      if (Read_dword_be(file, &count))
      {
        int look_for_comments = 1;
        for (i = 0; i < 256; i++)
          palette_conv[i] = (byte)i;

        while(count > 0)
        {
          ToolType = Read_INFO_String(file);
          if (ToolType == NULL)
            break;
          else
          {
            // NewIcons
            if (strlen(ToolType) > 4 && ToolType[0] == 'I' && ToolType[1] == 'M' && ToolType[3] == '=')
            {
              const byte * p;
              int img_index = ToolType[2] - '0';
              p = (byte *)ToolType + 4;
              if (img_index != current_img)
              {
                // image info + palette
                T_Components * palette;
                unsigned int palette_len;

                current_img = img_index;
                if (current_img > 1 && (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE))
                  break;  // don't load 2nd image in preview mode
                if (*p++ == 'B')
                {
                  context->Transparent_color = 0;
                  context->Background_transparent = 1;
                }
                else
                  context->Background_transparent = 0;
                width = *p++ - 0x21;
                height = *p++ - 0x21;
                color_count = *p++ - 0x21;
                color_count = (color_count << 6) + *p++ - 0x21;
                for (bpp = 1; color_count > (unsigned)(1 << bpp); bpp++) { }
                palette = (T_Components *)Decode_NewIcons(p, 8, &palette_len);
                if (palette)
                {
                  if (img_index == 1)
                  { // Set palette
                    if (Config.Clear_palette)
                      memset(context->Palette,0,sizeof(T_Palette));
                    memcpy(context->Palette, palette, MIN(palette_len,sizeof(T_Palette)));
                    if (palette_len < color_count * 3)
                    {
                      palette_color_count = palette_len / 3;
                      palette_continues = 1;
                    }
                    else
                    {
                      palette_color_count = color_count;
                      palette_continues = 0;
                    }
                  }
                  else
                  {
                    // merge palette with the existing one
                    for (i = 0; (unsigned)i < MIN(color_count,palette_len/3); i++)
                    {
                      unsigned int j;
                      for (j = 0; j < palette_color_count; j++)
                      {
                        if (0 == memcmp(context->Palette + j, palette + i, sizeof(T_Components)))
                          break;
                      }
                      if (j == palette_color_count)
                      {
                        if (palette_color_count < 256)
                        { // Add color to palette
                          memcpy(context->Palette + palette_color_count, palette + i, sizeof(T_Components));
                          palette_color_count++;
                        }
                        else
                          Warning("Too much colors in new icons");
                      }
                      palette_conv[i] = (byte)j;
                    }
                    if (palette_len < color_count * 3)
                      palette_continues = palette_len / 3;
                    else
                      palette_continues = 0; 
                  }
                  free(palette);
                }
                if (img_index == 1)
                {
                  Pre_load(context, width, height,file_size,FORMAT_INFO,PIXEL_SIMPLE, bpp);
                  Set_image_mode(context, IMAGE_MODE_ANIMATION);
                  has_NewIcons = 1;
                }
                else
                  Set_loading_layer(context, img_index - 1);
                x_pos = 0; y_pos = 0;
              }
              else if (palette_continues)
              {
                T_Components * palette;
                unsigned int palette_len;
                palette = (T_Components *)Decode_NewIcons(p, 8, &palette_len);
                if (palette)
                {
                  if (img_index == 1)
                  {
                    memcpy(context->Palette + palette_color_count, palette, MIN(palette_len,sizeof(T_Palette) - 3*palette_color_count));
                    if (palette_color_count * 3 + palette_len < color_count * 3)
                      palette_color_count += palette_len / 3;
                    else
                    {
                      palette_color_count = color_count;
                      palette_continues = 0;
                    }
                  }
                  else
                  {
                    // merge palette with the existing one
                    for (i = 0; (unsigned)i < MIN(color_count-palette_continues,palette_len/3); i++)
                    {
                      unsigned int j;
                      for (j = 0; j < palette_color_count; j++)
                      {
                        if (0 == memcmp(context->Palette + j, palette + i, sizeof(T_Components)))
                          break;
                      }
                      if (j == palette_color_count)
                      {
                        if (palette_color_count < 256)
                        { // Add color to palette
                          memcpy(context->Palette + palette_color_count, palette + i, sizeof(T_Components));
                          palette_color_count++;
                        }
                        else
                          Warning("Too much colors in new icons");
                      }
                      palette_conv[i+palette_continues] = (byte)j;
                    }
                    if (palette_continues * 3 + palette_len < color_count * 3)
                      palette_continues += palette_len / 3;
                    else
                      palette_continues = 0; 
                  }
                  free(palette);
                }
              }
              else
              {
                byte * pixels;
                unsigned int pixel_count;

                pixels = Decode_NewIcons(p, bpp, &pixel_count);
                if (pixels)
                {
                  for (i = 0; (unsigned)i < pixel_count; i++)
                  {
                    Set_pixel(context, x_pos++, y_pos, palette_conv[pixels[i]]);
                    if (x_pos >= width)
                    {
                      x_pos = 0;  
                      y_pos++;
                    }
                  }
                  free(pixels);
                }
              }
            }
            else if (look_for_comments)
            {
              if (strlen(ToolType) > 1)
              {
                if (strcmp(ToolType, "*** DON'T EDIT THE FOLLOWING LINES!! ***") == 0)
                {
                  // The following lines are NewIcons data!
                  look_for_comments = 0;
                }
                else if (context->Comment[0] == '\0')
                {
                  strncpy(context->Comment, ToolType, COMMENT_SIZE);
                  context->Comment[COMMENT_SIZE] = '\0';
                }
              }
            }
            free(ToolType);
          }
          count -= 4;
        }
      }
    }
    if (File_error == 0 && header.ToolWindow != 0)
    {
      char * ToolWindow = Read_INFO_String(file);
      if (ToolWindow == NULL)
        File_error = 2;
      else
      {
        free(ToolWindow);
      }
    }
    if (File_error == 0 && header.UserData == 1)
    {
      // Skip extended Drawer Data for OS 2.x
      if (fseek(file, 8, SEEK_CUR) != 0)
        File_error = 1;
    }
    // OS 3.5 (Glow Icon) can follow :
    // it is IFF data

    if (!has_NewIcons && img_count > 0)
    {
      if (Config.Clear_palette)
        memset(context->Palette,0,sizeof(T_Palette));
      if (header.UserData == 0)
        memcpy(context->Palette, amigaOS1x_pal, sizeof(amigaOS1x_pal));
      else
        memcpy(context->Palette, amigaOS2x_pal, sizeof(amigaOS2x_pal));

      Pre_load(context, header.Width, header.Height,file_size,FORMAT_INFO,PIXEL_SIMPLE, imgheaders[0].Depth);
      Set_image_mode(context, IMAGE_MODE_ANIMATION);
      for (img_count = 0; img_count < 2 && buffers[img_count] != NULL; img_count++)
      {
        if (img_count > 0)
        {
          if (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE)
            break;  // don't load 2nd image in preview mode
          Set_loading_layer(context, img_count);
        }
        for (y_pos = 0; y_pos < imgheaders[img_count].Height; y_pos++)
          Draw_IFF_line(context, buffers[img_count] + y_pos * line_size, y_pos, plane_line_size << 3, imgheaders[img_count].Depth);
      }
    }
    for (img_count = 0; img_count < 2; img_count++)
      if (buffers[img_count] != NULL)
      {
        free(buffers[img_count]);
        buffers[img_count] = NULL;
      }
  }
  else
    File_error=1;
  fclose(file);
}



//////////////////////////////////// BMP ////////////////////////////////////
/**
 * @defgroup BMP Bitmap and icon files
 * @ingroup loadsaveformats
 * .BMP/.ICO/.CUR files from OS/2 or Windows
 *
 * We support OS/2 files and windows BITMAPINFOHEADER, BITMAPV4HEADER,
 * BITMAPV5HEADER files.
 *
 * .ICO with PNG content are also supported
 *
 * @{
 */

/// BMP file header
typedef struct
{
    byte  Signature[2]; ///< ='BM' = 0x4D42
    dword Size_1;       ///< file size
    word  Reserved_1;   ///< 0
    word  Reserved_2;   ///< 0
    dword Offset;       ///< Offset of bitmap data start
    dword Size_2;       ///< BITMAPINFOHEADER size
    dword Width;        ///< Image Width
    int32_t Height;     ///< Image Height. signed: negative means a top-down bitmap (rare)
    word  Planes;       ///< Should be 1
    word  Nb_bits;      ///< Bits per pixel : 1,2,4,8,16,24 or 32
    dword Compression;  ///< Known values : 0=BI_RGB, 1=BI_RLE8, 2=BI_RLE4, 3=BI_BITFIELDS, 4=BI_JPEG, 5=BI_PNG
    dword Size_3;       ///< (optional) byte size of bitmap data
    dword XPM;          ///< (optional) horizontal pixels-per-meter
    dword YPM;          ///< (optional) vertical pixels-per-meter
    dword Nb_Clr;       ///< number of color indexes used in the table. 0 for default (1 << Nb_bits)
    dword Clr_Imprt;    ///< number of color indexes that are required for displaying the bitmap. 0 : all colors are required.
} T_BMP_Header;

/// Test for BMP format
void Test_BMP(T_IO_Context * context, FILE * file)
{
  T_BMP_Header header;

  (void)context;
  File_error=1;

  if (Read_bytes(file,&(header.Signature),2) // "BM"
      && Read_dword_le(file,&(header.Size_1))
      && Read_word_le(file,&(header.Reserved_1))
      && Read_word_le(file,&(header.Reserved_2))
      && Read_dword_le(file,&(header.Offset))
      && Read_dword_le(file,&(header.Size_2))
     )
  {
    if (header.Signature[0]=='B' && header.Signature[1]=='M' &&
        ((header.Size_1 == (header.Size_2 + 14)) || // Size_1 is fixed to 14 + header Size in some OS/2 BMPs
         (header.Offset < header.Size_1 && header.Offset >= (14 + header.Size_2))))
    {
      GFX2_Log(GFX2_DEBUG, "BMP : Size_1=%u Offset=%u Size_2=%u\n",
               header.Size_1, header.Offset, header.Size_2);
      if ( header.Size_2==40 /* WINDOWS BITMAPINFOHEADER */
           || header.Size_2==12 /* OS/2 */
           || header.Size_2==64 /* OS/2 v2 */
           || header.Size_2==16 /* OS/2 v2 - short - */
           || header.Size_2==108 /* Windows BITMAPV4HEADER */
           || header.Size_2==124 /* Windows BITMAPV5HEADER */ )
      {
        File_error=0;
      }
    }
  }
}

/// extract component value and properly shift it.
static byte Bitmap_mask(dword pixel, dword mask, int bits, int shift)
{
  dword value = (pixel & mask) >> shift;
  if (bits < 8)
    value = (value << (8 - bits)) | (value >> (2 * bits - 8));
  else if (bits > 8)
    value >>= (bits - 8);
  return (byte)value;
}

/// Load the Palette for 1 to 8bpp BMP's
static void Load_BMP_Palette(T_IO_Context * context, FILE * file, unsigned int nb_colors, int is_rgb24)
{
  byte local_palette[256*4]; // R,G,B,0 or RGB
  unsigned int i, j;

  if (Read_bytes(file, local_palette, MIN(nb_colors, 256) * (is_rgb24?3:4)))
  {
    if (Config.Clear_palette)
      memset(context->Palette,0,sizeof(T_Palette));

    // We can now load the new palette
    for (i = 0, j = 0; i < nb_colors && i < 256; i++)
    {
      context->Palette[i].B = local_palette[j++];
      context->Palette[i].G = local_palette[j++];
      context->Palette[i].R = local_palette[j++];
      if (!is_rgb24) j++;
    }
    if (nb_colors > 256)  // skip additional entries
      fseek(file, (nb_colors - 256) * (is_rgb24?3:4), SEEK_CUR);
  }
  else
  {
    File_error=1;
  }
}

/// rows are stored from the top to the bottom (standard for BMP is from bottom to the top)
#define LOAD_BMP_PIXEL_FLAG_TOP_DOWN     0x01
/// We are decoding the AND-mask plane (transparency) of a .ICO file
#define LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE 0x02

static void Load_BMP_Pixels(T_IO_Context * context, FILE * file, unsigned int compression, unsigned int nbbits, int flags, const dword * mask)
{
  unsigned int index;
  short x_pos;
  short y_pos;
  byte value;
  byte a,b;
  int bits[4];
  int shift[4];
  int i;

  // compute bit count and shift for masks
  for (i = 0; i < 4; i++)
  {
    if (mask[i] == 0)
    {
      bits[i] = 0;
      shift[i] = 0;
    }
    else
    {
      bits[i] = count_set_bits(mask[i]);
      shift[i] = count_trailing_zeros(mask[i]);
    }
  }

  switch (compression)
  {
    case 0 :  // BI_RGB : No compression
    case 3 :  // BI_BITFIELDS
      for (y_pos=0; (y_pos < context->Height && !File_error); y_pos++)
      {
        short target_y;
        target_y = (flags & LOAD_BMP_PIXEL_FLAG_TOP_DOWN) ? y_pos : context->Height-1-y_pos;

        switch (nbbits)
        {
          case 8 :
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              if (!Read_byte(file, &value))
                File_error = 2;
              Set_pixel(context, x_pos, target_y, value);
            }
            break;
          case 4 :
            for (x_pos = 0; x_pos < context->Width; )
            {
              if (!Read_byte(file, &value))
                File_error = 2;
              Set_pixel(context, x_pos++, target_y, (value >> 4) & 0x0F);
              Set_pixel(context, x_pos++, target_y, value & 0x0F);
            }
            break;
          case 2:
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              if ((x_pos & 3) == 0)
              {
                if (!Read_byte(file, &value))
                  File_error = 2;
              }
              Set_pixel(context, x_pos, target_y, (value >> 6) & 3);
              value <<= 2;
            }
            break;
          case 1 :
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              if ((x_pos & 7) == 0)
              {
                if (!Read_byte(file, &value))
                  File_error = 2;
              }
              if (flags & LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE)
              {
                if (value & 0x80) // transparent pixel !
                  Set_pixel(context, x_pos, target_y, context->Transparent_color);
              }
              else
                Set_pixel(context, x_pos, target_y, (value >> 7) & 1);
              value <<= 1;
            }
            break;
          case 24:
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              byte bgr[3];
              if (!Read_bytes(file, bgr, 3))
                File_error = 2;
              Set_pixel_24b(context, x_pos, target_y, bgr[2], bgr[1], bgr[0]);
            }
            break;
          case 32:
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              dword pixel;
              if (!Read_dword_le(file, &pixel))
                File_error = 2;
              Set_pixel_24b(context, x_pos, target_y,
                            Bitmap_mask(pixel,mask[0],bits[0],shift[0]),
                            Bitmap_mask(pixel,mask[1],bits[1],shift[1]),
                            Bitmap_mask(pixel,mask[2],bits[2],shift[2]));
            }
            break;
          case 16:
            for (x_pos = 0; x_pos < context->Width; x_pos++)
            {
              word pixel;
              if (!Read_word_le(file, &pixel))
                File_error = 2;
              Set_pixel_24b(context, x_pos, target_y,
                            Bitmap_mask(pixel,mask[0],bits[0],shift[0]),
                            Bitmap_mask(pixel,mask[1],bits[1],shift[1]),
                            Bitmap_mask(pixel,mask[2],bits[2],shift[2]));
            }
            break;
        }
        // lines are padded to dword sizes
        if (((context->Width * nbbits + 7) >> 3) & 3)
          fseek(file, 4 - (((context->Width * nbbits + 7) >> 3) & 3), SEEK_CUR);
      }
      break;

    case 1 : // BI_RLE8 Compression
      x_pos=0;

      y_pos=context->Height-1;

      /*Init_lecture();*/
      if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1)
        File_error=2;
      while (!File_error)
      {
        if (a) // Encoded mode
          for (index=1; index<=a; index++)
            Set_pixel(context, x_pos++,y_pos,b);
        else   // Absolute mode
          switch (b)
          {
            case 0 : // End of line
              x_pos=0;
              y_pos--;
              break;
            case 1 : // End of bitmap
              break;
            case 2 : // Delta
              if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1)
                File_error=2;
              x_pos+=a;
              y_pos-=b;
              break;
            default: // Nouvelle série
              while (b)
              {
                if(Read_byte(file, &a)!=1)
                  File_error=2;
                //Read_one_byte(file, &c);
                Set_pixel(context, x_pos++,y_pos,a);
                //if (--c)
                //{
                //  Set_pixel(context, x_pos++,y_pos,c);
                //  b--;
                //}
                b--;
              }
              if (ftell(file) & 1) fseek(file, 1, SEEK_CUR);
          }
        if (a==0 && b==1)
          break;
        if(Read_byte(file, &a) !=1 || Read_byte(file, &b)!=1)
        {
          File_error=2;
        }
      }
      break;

    case 2 : // BI_RLE4 Compression
      x_pos = 0;
      y_pos = context->Height-1;

      while (!File_error)
      {
        if(!(Read_byte(file, &a) && Read_byte(file, &b)))
        {
          File_error = 2;
          break;
        }
        if (a > 0) // Encoded mode : pixel count = a
        {
          //GFX2_Log(GFX2_DEBUG, "BI_RLE4: %d &%02X\n", a, b);
          for (index = 0; index < a; index++)
            Set_pixel(context, x_pos++, y_pos, ((index & 1) ? b : (b >> 4)) & 0x0f);
        }
        else
        {
          // a == 0 : Escape code
          byte c = 0;

          //GFX2_Log(GFX2_DEBUG, "BI_RLE4: %d %d\n", a, b);
          if (b == 1) // end of bitmap
            break;
          switch (b)
          {
            case 0 : //End of line
              x_pos = 0;
              y_pos--;
              break;
            case 2 : // Delta
              if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1)
                File_error=2;
              x_pos += a;
              y_pos -= b;
              break;
            default: // Absolute mode : pixel count = b
              for (index = 0; index < b && !File_error; index++, x_pos++)
              {
                if (index&1)
                  Set_pixel(context, x_pos,y_pos,c&0xF);
                else
                {
                  if (!Read_byte(file, &c))
                    File_error=2;
                  else
                    Set_pixel(context, x_pos,y_pos,c>>4);
                }
              }
              if ((b + 1) & 2)
              {
                // read a pad byte to enforce word alignment
                if (!Read_byte(file, &c))
                  File_error = 2;
              }
          }
        }
      }
      break;
    case 5: // BI_PNG
      {
        byte png_header[8];
        // Load header (8 first bytes)
        if (!Read_bytes(file,png_header,8))
          File_error = 1;
        else
        {
          // Do we recognize a png file signature ?
#ifndef __no_pnglib__
          if (png_sig_cmp(png_header, 0, 8) == 0)
          {
            Load_PNG_Sub(context, file, NULL, 0);
          }
#else
          if (0 == memcmp(png_header, "\x89PNG", 4))
          {
            // NO PNG Support
            Warning("PNG Signature : Compiled without libpng support");
            File_error = 2;
          }
#endif
          else
          {
            GFX2_Log(GFX2_WARNING, "No PNG signature in BI_PNG BMP\n");
            File_error = 1;
          }
        }
      }
      break;
    default:
      Warning("Unknown compression type");
  }
}

/// Load BMP file
void Load_BMP(T_IO_Context * context)
{
  FILE *file;
  T_BMP_Header header;
  word  nb_colors =  0;
  long  file_size;
  byte  negative_height; // top_down
  byte  true_color = 0;
  dword mask[4];  // R G B A

  file = Open_file_read(context);
  if (file == NULL)
  {
    File_error = 1;
    return;
  }

  File_error = 0;

  file_size = File_length_file(file);

  /* Read header */
  if (!(Read_bytes(file,header.Signature,2)
        && Read_dword_le(file,&(header.Size_1))
        && Read_word_le(file,&(header.Reserved_1))
        && Read_word_le(file,&(header.Reserved_2))
        && Read_dword_le(file,&(header.Offset))
        && Read_dword_le(file,&(header.Size_2))
       ))
  {
    File_error = 1;
  }
  else
  {
    if (header.Size_2 == 40 /* WINDOWS BITMAPINFOHEADER*/
        || header.Size_2 == 64 /* OS/2 v2 */
        || header.Size_2 == 108 /* Windows BITMAPV4HEADER */
        || header.Size_2 == 124 /* Windows BITMAPV5HEADER */)
    {
      if (!(Read_dword_le(file,&(header.Width))
            && Read_dword_le(file,(dword *)&(header.Height))
            && Read_word_le(file,&(header.Planes))
            && Read_word_le(file,&(header.Nb_bits))
            && Read_dword_le(file,&(header.Compression))
            && Read_dword_le(file,&(header.Size_3))
            && Read_dword_le(file,&(header.XPM))
            && Read_dword_le(file,&(header.YPM))
            && Read_dword_le(file,&(header.Nb_Clr))
            && Read_dword_le(file,&(header.Clr_Imprt))
           ))
        File_error = 1;
      else
        GFX2_Log(GFX2_DEBUG, "%s BMP %ux%d planes=%u bpp=%u compression=%u %u/%u\n",
            header.Size_2 == 64 ? "OS/2 v2" : "Windows",
            header.Width, header.Height, header.Planes, header.Nb_bits, header.Compression,
            header.XPM, header.YPM);
    }
    else if (header.Size_2 == 16) /* OS/2 v2 *short* */
    {
      if (!(Read_dword_le(file,&(header.Width))
            && Read_dword_le(file,(dword *)&(header.Height))
            && Read_word_le(file,&(header.Planes))
            && Read_word_le(file,&(header.Nb_bits))
           ))
        File_error = 1;
      else
      {
        GFX2_Log(GFX2_DEBUG, "OS/2 v2 BMP %ux%d planes=%u bpp=%u *short header*\n",
            header.Width, header.Height, header.Planes, header.Nb_bits);
        header.Compression = 0;
        header.Size_3 = 0;
        header.XPM = 0;
        header.YPM = 0;
        header.Nb_Clr = 0;
        header.Clr_Imprt = 0;
      }
    }
    else if (header.Size_2 == 12 /* OS/2 */)
    {
      word tmp_width = 0, tmp_height = 0;
      if (Read_word_le(file,&tmp_width)
          && Read_word_le(file,&tmp_height)
          && Read_word_le(file,&(header.Planes))
          && Read_word_le(file,&(header.Nb_bits)))
      {
        GFX2_Log(GFX2_DEBUG, "OS/2 BMP %ux%u planes=%u bpp=%u\n", tmp_width, tmp_height, header.Planes, header.Nb_bits);
        header.Width = tmp_width;
        header.Height = tmp_height;
        header.Compression = 0;
        header.Size_3 = 0;
        header.XPM = 0;
        header.YPM = 0;
        header.Nb_Clr = 0;
        header.Clr_Imprt = 0;
      }
      else
        File_error = 1;
    }
    else
    {
      Warning("Unknown BMP type");
      File_error = 1;
    }
  }

  if (File_error == 0)
  {
    /* header was read */
    switch (header.Nb_bits)
    {
      case 1 :
      case 2 :
      case 4 :
      case 8 :
        if (header.Nb_Clr)
          nb_colors = header.Nb_Clr;
        else
          nb_colors = 1 << header.Nb_bits;
        break;
      case 16:
      case 24:
      case 32:
        true_color = 1;
        break;
      case 0:
        if (header.Compression == 5)  // Nb_bits is 0 with BI_PNG
          break;
#if defined(__GNUC__) && (__GNUC__ >= 7)
        __attribute__ ((fallthrough));
#endif
      default:
        GFX2_Log(GFX2_WARNING, "BMP: Unsupported bit per pixel %u\n", header.Nb_bits);
        File_error = 1;
    }

    if (header.Height < 0)
    {
      negative_height = 1;
      header.Height = -header.Height;
    }
    else
    {
      negative_height = 0;
    }

    // Image 16/24/32 bits
    if (header.Nb_bits == 16)
    {
      mask[0] = 0x00007C00;
      mask[1] = 0x000003E0;
      mask[2] = 0x0000001F;
    }
    else
    {
      mask[0] = 0x00FF0000;
      mask[1] = 0x0000FF00;
      mask[2] = 0x000000FF;
    }
    mask[3] = 0;
    if (File_error == 0)
    {
      enum PIXEL_RATIO ratio = PIXEL_SIMPLE;

      if (header.XPM > 0 && header.YPM > 0)
      {
        if (header.XPM * 10 > header.YPM * 15)  // XPM/YPM > 1.5
          ratio = PIXEL_TALL;
        else if (header.XPM * 15 < header.YPM * 10) // XPM/YPM < 1/1.5
          ratio = PIXEL_WIDE;
      }

      Pre_load(context, header.Width, header.Height, file_size, FORMAT_BMP, ratio, header.Nb_bits);
      if (File_error==0)
      {
        if (header.Size_2 == 64)
          fseek(file, header.Size_2 - 40, SEEK_CUR);
        if (header.Size_2 >= 108 || (true_color && header.Compression == 3)) // BI_BITFIELDS
        {
          if (!Read_dword_le(file,&mask[0]) ||
              !Read_dword_le(file,&mask[1]) ||
              !Read_dword_le(file,&mask[2]))
            File_error=2;
          if (header.Size_2 >= 108)
          {
            Read_dword_le(file,&mask[3]); // Alpha mask
            fseek(file, header.Size_2 - 40 - 16, SEEK_CUR);  // skip extended v4/v5 header fields
          }
          GFX2_Log(GFX2_DEBUG, "BMP masks : R=%08x G=%08x B=%08x A=%08x\n", mask[0], mask[1], mask[2], mask[3]);
        }
        if (nb_colors > 0)
          Load_BMP_Palette(context, file, nb_colors, header.Size_2 == 12);

        if (File_error==0)
        {
          if (fseek(file, header.Offset, SEEK_SET))
            File_error=2;
          else
            Load_BMP_Pixels(context, file, header.Compression, header.Nb_bits, negative_height ? LOAD_BMP_PIXEL_FLAG_TOP_DOWN : 0, mask);
        }
      }
    }
  }
  fclose(file);
}


/// Save BMP file
void Save_BMP(T_IO_Context * context)
{
  FILE *file;
  T_BMP_Header header;
  short x_pos;
  short y_pos;
  long line_size;
  word index;
  byte local_palette[256][4]; // R,G,B,0


  File_error=0;

  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);
    
    // Image width must be a multiple of 4 bytes
    line_size = context->Width;
    if (line_size & 3)
      line_size += (4 - (line_size & 3));
    
    header.Signature[0]  = 'B';
    header.Signature[1]  = 'M';
    header.Size_1   =(line_size*context->Height)+1078;
    header.Reserved_1   =0;
    header.Reserved_2   =0;
    header.Offset   =1078; // Size of header data (including palette)
    header.Size_2   =40; // Size of header
    header.Width    =context->Width;
    header.Height    =context->Height;
    header.Planes      =1;
    header.Nb_bits    =8;
    header.Compression=0;
    header.Size_3   =0;
    header.XPM        =0;
    header.YPM        =0;
    header.Nb_Clr     =0;
    header.Clr_Imprt  =0;

    if (Write_bytes(file,header.Signature,2)
     && Write_dword_le(file,header.Size_1)
     && Write_word_le(file,header.Reserved_1)
     && Write_word_le(file,header.Reserved_2)
     && Write_dword_le(file,header.Offset)
     && Write_dword_le(file,header.Size_2)
     && Write_dword_le(file,header.Width)
     && Write_dword_le(file,header.Height)
     && Write_word_le(file,header.Planes)
     && Write_word_le(file,header.Nb_bits)
     && Write_dword_le(file,header.Compression)
     && Write_dword_le(file,header.Size_3)
     && Write_dword_le(file,header.XPM)
     && Write_dword_le(file,header.YPM)
     && Write_dword_le(file,header.Nb_Clr)
     && Write_dword_le(file,header.Clr_Imprt))
    {
      //   Chez Bill, ils ont dit: "On va mettre les couleur dans l'ordre
      // inverse, et pour faire chier, on va les mettre sur une échelle de
      // 0 à 255 parce que le standard VGA c'est de 0 à 63 (logique!). Et
      // puis comme c'est pas assez débile, on va aussi y rajouter un octet
      // toujours à 0 pour forcer les gens à s'acheter des gros disques
      // durs... Comme ça, ça fera passer la pillule lorsqu'on sortira
      // Windows 95." ...
      for (index=0; index<256; index++)
      {
        local_palette[index][0]=context->Palette[index].B;
        local_palette[index][1]=context->Palette[index].G;
        local_palette[index][2]=context->Palette[index].R;
        local_palette[index][3]=0;
      }

      if (Write_bytes(file,local_palette,1024))
      {
        // ... Et Bill, il a dit: "OK les gars! Mais seulement si vous rangez
        // les pixels dans l'ordre inverse, mais que sur les Y quand-même
        // parce que faut pas pousser."
        for (y_pos=context->Height-1; ((y_pos>=0) && (!File_error)); y_pos--)
          for (x_pos=0; x_pos<line_size; x_pos++)
                Write_one_byte(file,Get_pixel(context, x_pos,y_pos));

        fclose(file);

        if (File_error)
          Remove_file(context);
      }
      else
      {
        fclose(file);
        Remove_file(context);
        File_error=1;
      }

    }
    else
    {
      fclose(file);
      Remove_file(context);
      File_error=1;
    }
  }
  else
    File_error=1;
}


//////////////////////////////////// ICO ////////////////////////////////////
typedef struct {
  byte width;   //Specifies image width in pixels. Value 0 means image width is 256 pixels.
  byte height;  //Specifies image height in pixels. Value 0 means image height is 256 pixels.
  byte ncolors; //0 for image without palette
  byte reserved;
  word planes;  // (hotspot X for CUR files)
  word bpp;     // (hotspot Y for CUR files)
  dword bytecount;
  dword offset;
} T_ICO_ImageEntry;

void Test_ICO(T_IO_Context * context, FILE * file)
{
  struct {
    word Reserved;
    word Type;  // Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
    word Count; // Specifies number of images in the file.
  } header;

  (void)context;
  File_error=1;

  if (Read_word_le(file,&(header.Reserved))
      && Read_word_le(file,&(header.Type))
      && Read_word_le(file,&(header.Count)))
  {
    if (header.Reserved == 0 && (header.Type == 1 || header.Type == 2) && header.Count > 0)
      File_error=0;
  }
}

void Load_ICO(T_IO_Context * context)
{
  FILE *file;
  struct {
    word Reserved;
    word Type;  // Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
    word Count; // Specifies number of images in the file.
  } header;
  T_ICO_ImageEntry * images;
  T_ICO_ImageEntry * entry;
  unsigned int i;
  word width, max_width = 0;
  word max_bpp = 0;
  word min_bpp = 0xffff;
  dword mask[4];  // R G B A

  File_error=0;

  if ((file=Open_file_read(context)))
  {
    if (Read_word_le(file,&(header.Reserved))
     && Read_word_le(file,&(header.Type))
     && Read_word_le(file,&(header.Count)))
    {
      images = malloc(sizeof(T_ICO_ImageEntry) * header.Count);
      if (images == NULL)
      {
        fclose(file);
        File_error=1;
        return;
      }
      for (i = 0; i < header.Count; i++) {
        entry = images + i;
        if (!Read_byte(file,&entry->width)
         || !Read_byte(file,&entry->height)
         || !Read_byte(file,&entry->ncolors)
         || !Read_byte(file,&entry->reserved)
         || !Read_word_le(file,&entry->planes)
         || !Read_word_le(file,&entry->bpp)
         || !Read_dword_le(file,&entry->bytecount)
         || !Read_dword_le(file,&entry->offset))
        {
          free(images);
          fclose(file);
          File_error=1;
          return;
        }
        width = entry->width;
        if (width == 0) width = 256;
        if (width > max_width) max_width = width;
        // For various reasons, 256x256 icons are all in PNG format,
        // and Microsoft decided PNG inside ICO should be in 32bit ARGB format...
        // https://blogs.msdn.microsoft.com/oldnewthing/20101022-00/?p=12473
        GFX2_Log(GFX2_DEBUG, "%s #%02u %3ux%3u %ucols %ux%ubpp  %u bytes at 0x%06x\n",
                 (header.Type == 2) ? "CUR" : "ICO",
                 i, width, entry->height, entry->ncolors, entry->planes, entry->bpp,
                 entry->bytecount, entry->offset);
      }
      // select the picture with the maximum width and 256 colors or less
      for (i = 0; i < header.Count; i++)
      {
        if (images[i].width == (max_width & 0xff))
        {
          if (header.Type == 2) // .CUR files have hotspot instead of planes/bpp in header
            break;
          if (images[i].bpp == 8)
            break;
          if (images[i].bpp < 8 && images[i].bpp > max_bpp)
            max_bpp = images[i].bpp;
          if (images[i].bpp < min_bpp)
            min_bpp = images[i].bpp;
        }
      }
      if (i >= header.Count && header.Type == 1)
      {
        // 256 color not found, select another one
        for (i = 0; i < header.Count; i++)
        {
          if (images[i].width == (max_width & 0xff))
          {
            if ((max_bpp != 0 && images[i].bpp == max_bpp)
             || (images[i].bpp == min_bpp))
            {
              break;
            }
          }
        }
      }
      if (i >= header.Count)
      {
        File_error=2;
      }
      else
      {
        byte png_header[8];

        entry = images + i;
        GFX2_Log(GFX2_DEBUG, "Selected icon #%u at offset 0x%06x\n", i, entry->offset);
        fseek(file, entry->offset, SEEK_SET);

        // detect PNG icons
        // Load header (8 first bytes)
        if (!Read_bytes(file,png_header,8))
        {
          File_error = 1;
        }
        else
        {
          // Do we recognize a png file signature ?
#ifndef __no_pnglib__
          if (png_sig_cmp(png_header, 0, 8) == 0)
          {
            Load_PNG_Sub(context, file, NULL, 0);
          }
#else
          if (0 == memcmp(png_header, "\x89PNG", 4))
          {
            // NO PNG Support
            Warning("PNG Signature : Compiled without libpng support");
            File_error = 2;
          }
#endif
          else
          {
            T_BMP_Header bmpheader;

            fseek(file, -8, SEEK_CUR);  // back
            // BMP
            if (Read_dword_le(file,&(bmpheader.Size_2)) // 40
                && Read_dword_le(file,&(bmpheader.Width))
                && Read_dword_le(file,(dword *)&(bmpheader.Height))
                && Read_word_le(file,&(bmpheader.Planes))
                && Read_word_le(file,&(bmpheader.Nb_bits))
                && Read_dword_le(file,&(bmpheader.Compression))
                && Read_dword_le(file,&(bmpheader.Size_3))
                && Read_dword_le(file,&(bmpheader.XPM))
                && Read_dword_le(file,&(bmpheader.YPM))
                && Read_dword_le(file,&(bmpheader.Nb_Clr))
                && Read_dword_le(file,&(bmpheader.Clr_Imprt)) )
            {
              short real_height;
              word nb_colors = 0;

              GFX2_Log(GFX2_DEBUG, "  BITMAPINFOHEADER %u %dx%d %ux%ubpp comp=%u\n",
                       bmpheader.Size_2, bmpheader.Width, bmpheader.Height, bmpheader.Planes,
                       bmpheader.Nb_bits, bmpheader.Compression);
              if (bmpheader.Nb_Clr != 0)
                nb_colors=bmpheader.Nb_Clr;
              else
                nb_colors=1<<bmpheader.Nb_bits;

              real_height = bmpheader.Height / 2;
              if (bmpheader.Height < 0) real_height = - real_height;
              // check that real_height == entry->height
              if (real_height != entry->height)
              {
                Warning("Load_ICO() : real_height != entry->height");
              }

              // Image 16/24/32 bits
              if (bmpheader.Nb_bits == 16)
              {
                mask[0] = 0x00007C00;
                mask[1] = 0x000003E0;
                mask[2] = 0x0000001F;
              }
              else
              {
                mask[0] = 0x00FF0000;
                mask[1] = 0x0000FF00;
                mask[2] = 0x000000FF;
              }
              mask[3] = 0;
              Pre_load(context, bmpheader.Width,real_height,File_length_file(file),FORMAT_ICO,PIXEL_SIMPLE,bmpheader.Nb_bits);
              if (bmpheader.Nb_bits <= 8)
                Load_BMP_Palette(context, file, nb_colors, 0);
              else
              {
                if (bmpheader.Compression == 3) // BI_BITFIELDS
                {
                  if (!Read_dword_le(file,&mask[0]) ||
                      !Read_dword_le(file,&mask[1]) ||
                      !Read_dword_le(file,&mask[2]))
                    File_error=2;
                }
              }
              if (File_error == 0)
              {
                Load_BMP_Pixels(context, file, bmpheader.Compression, bmpheader.Nb_bits, (bmpheader.Height < 0) ? LOAD_BMP_PIXEL_FLAG_TOP_DOWN : 0, mask);
                // load transparency
                // TODO : load transparency for True color images too
                if (bmpheader.Nb_bits <= 8)
                {
                  context->Transparent_color = 0xff;  // TODO : pick an unused color if possible
                  context->Background_transparent = 1;
                  Load_BMP_Pixels(context, file, bmpheader.Compression, 1, (bmpheader.Height < 0) ? (LOAD_BMP_PIXEL_FLAG_TOP_DOWN|LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE) : LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE, mask);
                }
              }
            }
          }
        }
      }
      free(images);
    }
    fclose(file);
  } else {
    File_error=1;
  }
}

void Save_ICO(T_IO_Context * context)
{
  FILE *file;
  short x_pos;
  short y_pos;
  long row_size;
  long row_size_mask;

  if (context->Width > 256 || context->Height > 256)
  {
    File_error=1;
    Warning(".ICO files can handle images up to 256x256");
    return;
  }

  File_error=0;

  if ((file=Open_file_write(context)) == NULL)
    File_error=1;
  else
  {
    row_size = (context->Width + 3) & ~3;                   // 8bpp (=1Byte) rounded up to dword
    row_size_mask = (((context->Width + 7) >> 3) + 3) & ~3; // 1bpp rounded up to dword
    // ICO Header
    if (!(Write_word_le(file,0) &&  // 0
        Write_word_le(file,1) &&  // TYPE 1 = .ICO (2=.CUR)
        Write_word_le(file,1)))    // Image count
      File_error=1;
    if (File_error == 0)
    {
      T_ICO_ImageEntry entry;
      // ICO image entry
      entry.width = context->Width & 0xff;  //Specifies image width in pixels. Value 0 means image width is 256 pixels.
      entry.height = context->Height & 0xff;//Specifies image height in pixels. Value 0 means image height is 256 pixels.
      entry.ncolors = 0;
      entry.reserved = 0;
      entry.planes = 1;
      entry.bpp = 8;
      entry.bytecount = (row_size + row_size_mask) * context->Height + 40 + 1024;
      entry.offset = 6 + 16;
      if  (!(Write_byte(file,entry.width) &&
           Write_byte(file,entry.height) &&
           Write_byte(file,entry.ncolors) &&
           Write_byte(file,entry.reserved) &&
           Write_word_le(file,entry.planes) &&
           Write_word_le(file,entry.bpp) &&
           Write_dword_le(file,entry.bytecount) &&
           Write_dword_le(file,entry.offset)))
        File_error=1;
    }
    if (File_error == 0)
    {
      T_BMP_Header bmpheader;
      // BMP Header
      bmpheader.Size_2 = 40;
      bmpheader.Width = context->Width;
      bmpheader.Height = context->Height * 2; // *2 because of mask (transparency) data added after the pixel data
      bmpheader.Planes = 1;
      bmpheader.Nb_bits = 8;
      bmpheader.Compression = 0;
      bmpheader.Size_3 = 0;
      bmpheader.XPM = 0;
      bmpheader.YPM = 0;
      bmpheader.Nb_Clr = 0;
      bmpheader.Clr_Imprt = 0;
      if (!(Write_dword_le(file,bmpheader.Size_2) // 40
          && Write_dword_le(file,bmpheader.Width)
          && Write_dword_le(file,bmpheader.Height)
          && Write_word_le(file,bmpheader.Planes)
          && Write_word_le(file,bmpheader.Nb_bits)
          && Write_dword_le(file,bmpheader.Compression)
          && Write_dword_le(file,bmpheader.Size_3)
          && Write_dword_le(file,bmpheader.XPM)
          && Write_dword_le(file,bmpheader.YPM)
          && Write_dword_le(file,bmpheader.Nb_Clr)
          && Write_dword_le(file,bmpheader.Clr_Imprt)) )
        File_error=1;
    }
    if (File_error == 0)
    {
      int i;
      // palette
      for (i = 0; i < 256; i++)
      {
        if (!Write_dword_le(file, context->Palette[i].R << 16 | context->Palette[i].G << 8 | context->Palette[i].B))
        {
          File_error=1;
          break;
        }
      }
    }
    if (File_error == 0)
    {
      // Image Data
      for (y_pos=context->Height-1; ((y_pos>=0) && (!File_error)); y_pos--)
        for (x_pos=0; x_pos<row_size; x_pos++)
          Write_one_byte(file,Get_pixel(context, x_pos,y_pos));
    }
    if (File_error == 0)
    {
      byte value = 0;
      // Mask (Transparency) Data
      for (y_pos=context->Height-1; ((y_pos>=0) && (!File_error)); y_pos--)
        for (x_pos=0; x_pos<row_size_mask*8; x_pos++)
        {
          value = value << 1;
          if (context->Background_transparent && Get_pixel(context, x_pos,y_pos) == context->Transparent_color)
            value |= 1; // 1 = Transparent pixel
          if ((x_pos & 7) == 7)
          {
            Write_one_byte(file,value);
          }
        }
    }
    fclose(file);
    if (File_error != 0)
      Remove_file(context);
  }
}
/** @} */


//////////////////////////////////// GIF ////////////////////////////////////
/**
 * @defgroup GIF GIF format
 * @ingroup loadsaveformats
 * Graphics Interchange Format
 *
 * The GIF format uses LZW compression and stores indexed color pictures
 * up to 256 colors. It has the ability to store several pictures in the same
 * file : GrafX2 takes advantage of this feature for storing layered images
 * and animations.
 *
 * GrafX2 implements GIF89a :
 * https://www.w3.org/Graphics/GIF/spec-gif89a.txt
 *
 * @{
 */

/// Logical Screen Descriptor Block
typedef struct
{
  word Width;   ///< Width of the complete image area
  word Height;  ///< Height of the complete image area
  byte Resol;   ///< Informations about the resolution (and other)
  byte Backcol; ///< Proposed background color
  byte Aspect;  ///< Informations about aspect ratio. Ratio = (Aspect + 15) / 64
} T_GIF_LSDB;

/// Image Descriptor Block
typedef struct
{
  word Pos_X;         ///< X offset where the image should be pasted
  word Pos_Y;         ///< Y offset where the image should be pasted
  word Image_width;   ///< Width of image
  word Image_height;  ///< Height of image
  byte Indicator;     ///< Misc image information
  byte Nb_bits_pixel; ///< Nb de bits par pixel
} T_GIF_IDB;

/// Graphic Control Extension
typedef struct
{
  byte Block_identifier;  ///< 0x21
  byte Function;          ///< 0xF9
  byte Block_size;        ///< 4
  byte Packed_fields;     ///< 11100000 : Reserved
                          ///< 00011100 : Disposal method
                          ///< 00000010 : User input flag
                          ///< 00000001 : Transparent flag
  word Delay_time;        ///< Time for this frame to stay displayed
  byte Transparent_color; ///< Which color index acts as transparent
  word Block_terminator;  ///< 0x00
} T_GIF_GCE;

enum DISPOSAL_METHOD
{
  DISPOSAL_METHOD_UNDEFINED = 0,
  DISPOSAL_METHOD_DO_NOT_DISPOSE = 1,
  DISPOSAL_METHOD_RESTORE_BGCOLOR = 2,
  DISPOSAL_METHOD_RESTORE_PREVIOUS = 3,
};


/// Test if a file is GIF format
void Test_GIF(T_IO_Context * context, FILE * file)
{
  char signature[6];

  (void)context;
  File_error=1;

  if (Read_bytes(file,signature,6))
  {
    /// checks if the signature (6 first bytes) is either GIF87a or GIF89a
    if ((!memcmp(signature,"GIF87a",6))||(!memcmp(signature,"GIF89a",6)))
      File_error=0;
  }
}


// -- Lire un fichier au format GIF -----------------------------------------

typedef struct {
  word nb_bits;        ///< bits for a code
  word remainder_bits; ///< available bits in @ref last_byte field
  byte remainder_byte; ///< Remaining bytes in current block
  word current_code;   ///< current code (generally the one just read)
  byte last_byte;      ///< buffer byte for reading bits for codes
  word pos_X;          ///< Current coordinates
  word pos_Y;
  word interlaced;     ///< interlaced flag
  word pass;           ///< current pass in interlaced decoding
  word stop;           ///< Stop flag (end of picture)
} T_GIF_context;


/// Reads the next code (GIF.nb_bits bits)
static word GIF_get_next_code(FILE * GIF_file, T_GIF_context * gif)
{
  word nb_bits_to_process = gif->nb_bits;
  word nb_bits_processed = 0;
  word current_nb_bits;

  gif->current_code = 0;

  while (nb_bits_to_process)
  {
    if (gif->remainder_bits == 0) // Il ne reste plus de bits...
    {
      // Lire l'octet suivant:

      // Si on a atteint la fin du bloc de Raster Data
      if (gif->remainder_byte == 0)
      {
        // Lire l'octet nous donnant la taille du bloc de Raster Data suivant
        if(Read_byte(GIF_file, &gif->remainder_byte)!=1)
        {
          File_error=2;
          return 0;
        }
        if (gif->remainder_byte == 0) // still nothing ? That is the end data block
        {
          File_error = 2;
          GFX2_Log(GFX2_WARNING, "GIF 0 sized data block\n");
          return gif->current_code;
        }
      }
      if(Read_byte(GIF_file,&gif->last_byte)!=1)
      {
        File_error = 2;
        GFX2_Log(GFX2_ERROR, "GIF failed to load data byte\n");
        return 0;
      }
      gif->remainder_byte--;
      gif->remainder_bits=8;
    }

    current_nb_bits=(nb_bits_to_process<=gif->remainder_bits)?nb_bits_to_process:gif->remainder_bits;

    gif->current_code |= (gif->last_byte & ((1<<current_nb_bits)-1))<<nb_bits_processed;
    gif->last_byte >>= current_nb_bits;
    nb_bits_processed += current_nb_bits;
    nb_bits_to_process -= current_nb_bits;
    gif->remainder_bits -= current_nb_bits;
  }

  return gif->current_code;
}

/// Put a new pixel
static void GIF_new_pixel(T_IO_Context * context, T_GIF_context * gif, T_GIF_IDB *idb, int is_transparent, byte color)
{
  if (!is_transparent || color!=context->Transparent_color)
    Set_pixel(context, idb->Pos_X+gif->pos_X, idb->Pos_Y+gif->pos_Y,color);

  gif->pos_X++;

  if (gif->pos_X >= idb->Image_width)
  {
    gif->pos_X=0;

    if (!gif->interlaced)
    {
      gif->pos_Y++;
      if (gif->pos_Y >= idb->Image_height)
        gif->stop = 1;
    }
    else
    {
      switch (gif->pass)
      {
        case 0 :
        case 1 : gif->pos_Y+=8;
                 break;
        case 2 : gif->pos_Y+=4;
                 break;
        default: gif->pos_Y+=2;
      }

      if (gif->pos_Y >= idb->Image_height)
      {
        switch(++(gif->pass))
        {
        case 1 : gif->pos_Y=4;
                 break;
        case 2 : gif->pos_Y=2;
                 break;
        case 3 : gif->pos_Y=1;
                 break;
        case 4 : gif->stop = 1;
        }
      }
    }
  }
}


/// Load GIF file
void Load_GIF(T_IO_Context * context)
{
  FILE *GIF_file;
  int image_mode = -1;
  char signature[6];

  word * alphabet_stack;     // Pile de décodage d'une chaîne
  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   alphabet_stack_pos; // Position dans la pile de décodage d'un chaîne

  T_GIF_context GIF;
  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;
  T_GIF_GCE GCE;

  word nb_colors;       // Nombre de couleurs dans l'image
  word color_index; // index de traitement d'une couleur
  byte size_to_read; // Nombre de données à lire      (divers)
  byte block_identifier;  // Code indicateur du type de bloc en cours
  byte initial_nb_bits;   // Nb de bits au début du traitement LZW
  word special_case=0;       // Mémoire pour le cas spécial
  word old_code=0;       // Code précédent
  word byte_read;         // Sauvegarde du code en cours de lecture
  word value_clr;        // Valeur <=> Clear tables
  word value_eof;        // Valeur <=> End d'image
  long file_size;
  int number_LID; // Nombre d'images trouvées dans le fichier
  int current_layer = 0;
  int last_delay = 0;
  byte is_transparent = 0;
  byte is_looping=0;
  enum PIXEL_RATIO ratio;
  byte disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;

  byte previous_disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;
  word previous_width=0;
  word previous_height=0;
  word previous_pos_x=0;
  word previous_pos_y=0;

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  number_LID=0;
  
  if ((GIF_file=Open_file_read(context)))
  {
    file_size=File_length_file(GIF_file);
    if ( (Read_bytes(GIF_file,signature,6)) &&
         ( (memcmp(signature,"GIF87a",6)==0) ||
           (memcmp(signature,"GIF89a",6)==0) ) )
    {

      // Allocation de mémoire pour les tables & piles de traitement:
      alphabet_stack   =(word *)malloc(4096*sizeof(word));
      alphabet_prefix=(word *)malloc(4096*sizeof(word));
      alphabet_suffix=(word *)malloc(4096*sizeof(word));

      if (Read_word_le(GIF_file,&(LSDB.Width))
      && Read_word_le(GIF_file,&(LSDB.Height))
      && Read_byte(GIF_file,&(LSDB.Resol))
      && Read_byte(GIF_file,&(LSDB.Backcol))
      && Read_byte(GIF_file,&(LSDB.Aspect))
        )
      {
        // Lecture du Logical Screen Descriptor Block réussie:

        Original_screen_X=LSDB.Width;
        Original_screen_Y=LSDB.Height;

        ratio=PIXEL_SIMPLE;          //  (49 + 15) / 64 = 1:1
        if (LSDB.Aspect != 0) {
          if (LSDB.Aspect < 25)      //  (17 + 15) / 64 = 1:2
            ratio=PIXEL_TALL;
          else if (LSDB.Aspect < 41) //  (33 + 15) / 64 = 3:4
            ratio=PIXEL_TALL3;
          else if (LSDB.Aspect > 82) // (113 + 15) / 64 = 2:1
            ratio=PIXEL_WIDE;
        }

        Pre_load(context, LSDB.Width,LSDB.Height,file_size,FORMAT_GIF,ratio,(LSDB.Resol&7)+1);

        // Palette globale dispo = (LSDB.Resol  and $80)
        // Profondeur de couleur =((LSDB.Resol  and $70) shr 4)+1
        // Nombre de bits/pixel  = (LSDB.Resol  and $07)+1
        // Ordre de Classement   = (LSDB.Aspect and $80)

        nb_colors=(1 << ((LSDB.Resol & 0x07)+1));
        if (LSDB.Resol & 0x80)
        {
          // Palette globale dispo:

          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));

          // Load the palette
          for(color_index=0;color_index<nb_colors;color_index++)
          {
            Read_byte(GIF_file,&(context->Palette[color_index].R));
            Read_byte(GIF_file,&(context->Palette[color_index].G));
            Read_byte(GIF_file,&(context->Palette[color_index].B));
          }
        }

        // On lit un indicateur de block
        Read_byte(GIF_file,&block_identifier);
        while (block_identifier!=0x3B && !File_error)
        {
          switch (block_identifier)
          {
            case 0x21: // Bloc d'extension
            {
              byte function_code;
              // Lecture du code de fonction:
              Read_byte(GIF_file,&function_code);   
              // Lecture de la taille du bloc:
              Read_byte(GIF_file,&size_to_read);
              while (size_to_read!=0 && !File_error)
              {
                switch(function_code)
                {
                  case 0xFE: // Comment Block Extension
                    // On récupère le premier commentaire non-vide, 
                    // on jette les autres.
                    if (context->Comment[0]=='\0')
                    {
                      int nb_char_to_keep = MIN(size_to_read, COMMENT_SIZE);
                      
                      Read_bytes(GIF_file,context->Comment,nb_char_to_keep);
                      context->Comment[nb_char_to_keep+1]='\0';
                      // Si le commentaire etait trop long, on fait avance-rapide
                      // sur la suite.
                      if (size_to_read>nb_char_to_keep)
                        fseek(GIF_file,size_to_read-nb_char_to_keep,SEEK_CUR);
                    }
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;
                  case 0xF9: // Graphics Control Extension
                    // Prévu pour la transparence
                    if ( Read_byte(GIF_file,&(GCE.Packed_fields))
                      && Read_word_le(GIF_file,&(GCE.Delay_time))
                      && Read_byte(GIF_file,&(GCE.Transparent_color)))
                    {
                      previous_disposal_method = disposal_method;
                      disposal_method = (GCE.Packed_fields >> 2) & 7;
                      last_delay = GCE.Delay_time;
                      context->Transparent_color= GCE.Transparent_color;
                      is_transparent = GCE.Packed_fields & 1;
                      GFX2_Log(GFX2_DEBUG, "GIF Graphics Control Extension : transp=%d (color #%u) delay=%ums disposal_method=%d\n", is_transparent, GCE.Transparent_color, 10*GCE.Delay_time, disposal_method);
                      if (number_LID == 0)
                        context->Background_transparent = is_transparent;
                      is_transparent &= is_looping;
                    }
                    else
                      File_error=2;
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;

                  case 0xFF: // Application Extension
                    // Normally, always a 11-byte block
                    if (size_to_read == 0x0B)
                    {
                      char aeb[0x0B];
                      Read_bytes(GIF_file,aeb, 0x0B);
                      GFX2_Log(GFX2_DEBUG, "GIF extension \"%.11s\"\n", aeb);
                      if (File_error)
                        ;
                      else if (!memcmp(aeb,"NETSCAPE2.0",0x0B))
                      {
                        is_looping=1;
                        // The well-known Netscape extension.
                        // Load as an animation
                        Set_image_mode(context, IMAGE_MODE_ANIMATION);
                        // Skip sub-block
                        do
                        {
                          if (! Read_byte(GIF_file,&size_to_read))
                            File_error=1;
                          fseek(GIF_file,size_to_read,SEEK_CUR);
                        } while (!File_error && size_to_read!=0);
                      }
                      else if (!memcmp(aeb,"GFX2PATH\x00\x00\x00",0x0B))
                      {
                        // Original file path
                        Read_byte(GIF_file,&size_to_read);
                        if (!File_error && size_to_read > 0)
                        {
                          free(context->Original_file_directory);
                          context->Original_file_directory = malloc(size_to_read);
                          Read_bytes(GIF_file, context->Original_file_directory, size_to_read);
                          Read_byte(GIF_file, &size_to_read);
                          if (!File_error && size_to_read > 0)
                          {
                            free(context->Original_file_name);
                            context->Original_file_name = malloc(size_to_read);
                            Read_bytes(GIF_file, context->Original_file_name, size_to_read);
                            Read_byte(GIF_file, &size_to_read); // Normally 0
                          }
                        }
                      }
                      else if (!memcmp(aeb,"CRNG\0\0\0\0" "1.0",0x0B))
                      {            
                        // Color animation. Similar to a CRNG chunk in IFF file format.
                        word rate;
                        word flags;
                        byte min_col;
                        byte max_col;
                        //
                        Read_byte(GIF_file,&size_to_read);
                        for(;size_to_read>0 && !File_error;size_to_read-=6)
                        {
                          if ( (Read_word_be(GIF_file,&rate))
                            && (Read_word_be(GIF_file,&flags))
                            && (Read_byte(GIF_file,&min_col))
                            && (Read_byte(GIF_file,&max_col)))
                          {
                            if (min_col != max_col)
                            {
                              // Valid cycling range
                              if (max_col<min_col)
                              SWAP_BYTES(min_col,max_col)
                              
                              context->Cycle_range[context->Color_cycles].Start=min_col;
                              context->Cycle_range[context->Color_cycles].End=max_col;
                              context->Cycle_range[context->Color_cycles].Inverse=(flags&2)?1:0;
                              context->Cycle_range[context->Color_cycles].Speed=(flags&1)?rate/78:0;
                                                  
                              context->Color_cycles++;
                            }
                          }
                          else
                          {
                            File_error=1;
                          }
                        }
                        // Read end-of-block delimiter
                        if (!File_error)
                          Read_byte(GIF_file,&size_to_read);
                        if (size_to_read!=0)
                          File_error=1;
                      }
                      else if (0 == memcmp(aeb, "GFX2MODE", 8))
                      {
                        Read_byte(GIF_file,&size_to_read);
                        if (size_to_read > 0)
                        { // read the image mode. We'll set it after having loaded all layers.
                          char * label = malloc((size_t)size_to_read + 1);
                          Read_bytes(GIF_file, label, size_to_read);
                          label[size_to_read] = '\0';
                          image_mode = Constraint_mode_from_label(label);
                          GFX2_Log(GFX2_DEBUG, "    mode = %s (%d)\n", label, image_mode);
                          free(label);
                          Read_byte(GIF_file,&size_to_read);
                          // be future proof, skip following sub-blocks :
                          while (size_to_read!=0 && !File_error)
                          {
                            if (fseek(GIF_file,size_to_read,SEEK_CUR) < 0)
                              File_error = 1;
                            if (!Read_byte(GIF_file,&size_to_read))
                              File_error = 1;
                          }
                        }
                      }
                      else
                      {
                        // Unknown extension, skip.
                        Read_byte(GIF_file,&size_to_read);
                        while (size_to_read!=0 && !File_error)
                        {
                          if (fseek(GIF_file,size_to_read,SEEK_CUR) < 0)
                            File_error = 1;
                          if (!Read_byte(GIF_file,&size_to_read))
                            File_error = 1;
                        }
                      }
                    }
                    else
                    {
                      fseek(GIF_file,size_to_read,SEEK_CUR);
                      // Lecture de la taille du bloc suivant:
                      Read_byte(GIF_file,&size_to_read);
                    }
                    break;
                    
                  default:
                    // On saute le bloc:
                    fseek(GIF_file,size_to_read,SEEK_CUR);
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;
                }
              }
            }
            break;
            case 0x2C: // Local Image Descriptor
            {
              if (number_LID!=0)
              {
                // This a second layer/frame, or more.
                // Attempt to add a layer to current image
                current_layer++;
                Set_loading_layer(context, current_layer);
                if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
                {
                  // Copy the content of previous layer.
                  memcpy(
                    Main.backups->Pages->Image[Main.current_layer].Pixels,
                    Main.backups->Pages->Image[Main.current_layer-1].Pixels,
                    Main.backups->Pages->Width*Main.backups->Pages->Height);
                }
                else
                {
                  Fill_canvas(context, is_transparent ? context->Transparent_color : LSDB.Backcol);
                }
              }
              else
              {
                // First frame/layer, fill canvas with backcolor
                Fill_canvas(context, is_transparent ? context->Transparent_color : LSDB.Backcol);
              }
              // Duration was set in the previously loaded GCE
              Set_frame_duration(context, last_delay*10);
              number_LID++;
              
              // lecture de 10 derniers octets
              if ( Read_word_le(GIF_file,&(IDB.Pos_X))
                && Read_word_le(GIF_file,&(IDB.Pos_Y))
                && Read_word_le(GIF_file,&(IDB.Image_width))
                && Read_word_le(GIF_file,&(IDB.Image_height))
                && Read_byte(GIF_file,&(IDB.Indicator))
                && IDB.Image_width && IDB.Image_height)
              {
                GFX2_Log(GFX2_DEBUG, "GIF Image descriptor %u Pos (%u,%u) %ux%u %s%slocal palette(%ubpp)\n",
                         number_LID, IDB.Pos_X, IDB.Pos_Y, IDB.Image_width, IDB.Image_height,
                         (IDB.Indicator & 0x40) ? "interlaced " : "", (IDB.Indicator & 0x80) ? "" : "no ",
                         (IDB.Indicator & 7) + 1);
                // Palette locale dispo = (IDB.Indicator and $80)
                // Image entrelacée     = (IDB.Indicator and $40)
                // Ordre de classement  = (IDB.Indicator and $20)
                // Nombre de bits/pixel = (IDB.Indicator and $07)+1 (si palette locale dispo)
    
                if (IDB.Indicator & 0x80)
                {
                  // Palette locale dispo
    
                  if (Config.Clear_palette)
                    memset(context->Palette,0,sizeof(T_Palette));

                  nb_colors=(1 << ((IDB.Indicator & 0x07)+1));
                  // Load the palette
                  for(color_index=0;color_index<nb_colors;color_index++)
                  {   
                    Read_byte(GIF_file,&(context->Palette[color_index].R));
                    Read_byte(GIF_file,&(context->Palette[color_index].G));
                    Read_byte(GIF_file,&(context->Palette[color_index].B));
                  }
                  
                }
                if (number_LID!=1)
                {
                  // This a second layer/frame, or more.
                  if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
                  {
                    // Need to clear previous image to back-color.
                    if (previous_disposal_method==DISPOSAL_METHOD_RESTORE_BGCOLOR)
                    {
                      int y;
                      for (y=0; y<previous_height; y++)
                        memset(
                          Main.backups->Pages->Image[Main.current_layer].Pixels
                           + (previous_pos_y+y)* Main.backups->Pages->Width+previous_pos_x,
                          is_transparent ? context->Transparent_color : LSDB.Backcol,
                          previous_width);
                    }
                  }
                }
                previous_height=IDB.Image_height;
                previous_width=IDB.Image_width;
                previous_pos_x=IDB.Pos_X;
                previous_pos_y=IDB.Pos_Y;
                
                File_error=0;
                if (!Read_byte(GIF_file,&(initial_nb_bits)))
                  File_error=1;
    
                value_clr    =(1<<initial_nb_bits)+0;
                value_eof    =(1<<initial_nb_bits)+1;
                alphabet_free=(1<<initial_nb_bits)+2;

                GIF.nb_bits  =initial_nb_bits + 1;
                alphabet_max      =((1 <<  GIF.nb_bits)-1);
                GIF.interlaced    =(IDB.Indicator & 0x40);
                GIF.pass         =0;
    
                /*Init_lecture();*/
    

                GIF.stop = 0;
    
                //////////////////////////////////////////// DECOMPRESSION LZW //
    
                GIF.pos_X=0;
                GIF.pos_Y=0;
                alphabet_stack_pos=0;
                GIF.last_byte    =0;
                GIF.remainder_bits    =0;
                GIF.remainder_byte    =0;

                while ( (GIF_get_next_code(GIF_file, &GIF)!=value_eof) && (!File_error) )
                {
                  if (GIF.current_code > alphabet_free)
                  {
                    GFX2_Log(GFX2_INFO, "Load_GIF() Invalid code %u (should be <=%u)\n", GIF.current_code, alphabet_free);
                    File_error=2;
                    break;
                  }
                  else if (GIF.current_code != value_clr)
                  {
                    byte_read = GIF.current_code;
                    if (alphabet_free == GIF.current_code)
                    {
                      GIF.current_code=old_code;
                      alphabet_stack[alphabet_stack_pos++]=special_case;
                    }

                    while (GIF.current_code > value_clr)
                    {
                      if (GIF.current_code >= 4096)
                      {
                        GFX2_Log(GFX2_ERROR, "Load_GIF() GIF.current_code = %u >= 4096\n", GIF.current_code);
                        File_error = 2;
                        break;
                      }
                      alphabet_stack[alphabet_stack_pos++] = alphabet_suffix[GIF.current_code];
                      GIF.current_code = alphabet_prefix[GIF.current_code];
                    }

                    special_case = alphabet_stack[alphabet_stack_pos++] = GIF.current_code;

                    do
                      GIF_new_pixel(context, &GIF, &IDB, is_transparent, alphabet_stack[--alphabet_stack_pos]);
                    while (alphabet_stack_pos!=0);

                    alphabet_prefix[alphabet_free  ]=old_code;
                    alphabet_suffix[alphabet_free++]=GIF.current_code;
                    old_code=byte_read;

                    if (alphabet_free>alphabet_max)
                    {
                      if (GIF.nb_bits<12)
                        alphabet_max      =((1 << (++GIF.nb_bits))-1);
                    }
                  }
                  else // Clear code
                  {
                    GIF.nb_bits   = initial_nb_bits + 1;
                    alphabet_max  = ((1 <<  GIF.nb_bits)-1);
                    alphabet_free = (1<<initial_nb_bits)+2;
                    special_case  = GIF_get_next_code(GIF_file, &GIF);
                    if (GIF.current_code >= value_clr)
                    {
                      GFX2_Log(GFX2_INFO, "Load_GIF() Invalid code %u just after clear (=%u)!\n",
                               GIF.current_code, value_clr);
                      File_error = 2;
                      break;
                    }
                    old_code      = GIF.current_code;
                    GIF_new_pixel(context, &GIF, &IDB, is_transparent, GIF.current_code);
                  }
                }

                if (File_error == 2 && GIF.pos_X == 0 && GIF.pos_Y == IDB.Image_height)
                  File_error=0;
    
                if (File_error >= 0 && !GIF.stop)
                  File_error=2;
                
                // No need to read more than one frame in animation preview mode
                if (context->Type == CONTEXT_PREVIEW && is_looping)
                {
                  goto early_exit;
                }
                // Same with brush
                if (context->Type == CONTEXT_BRUSH && is_looping)
                {
                  goto early_exit;
                }
                
              } // Le fichier contenait un IDB
              else
                File_error=2;
            }
            default:
            break;
          }
          // Lecture du code de fonction suivant:
          if (!Read_byte(GIF_file,&block_identifier))
            File_error=2;
        }

        // set the mode that have been read previously.
        if (image_mode > 0)
          Set_image_mode(context, image_mode);
      } // Le fichier contenait un LSDB
      else
        File_error=1;

      early_exit:
      
      // Libération de la mémoire utilisée par les tables & piles de traitement:
      free(alphabet_suffix);
      free(alphabet_prefix);
      free(alphabet_stack);
      alphabet_suffix = alphabet_prefix = alphabet_stack = NULL;
    } // Le fichier contenait au moins la signature GIF87a ou GIF89a
    else
      File_error=1;

    fclose(GIF_file);

  } // Le fichier était ouvrable
  else
    File_error=1;
}


// -- Sauver un fichier au format GIF ---------------------------------------

/// Flush the buffer
static void GIF_empty_buffer(FILE * file, T_GIF_context *gif, byte * GIF_buffer)
{
  word index;

  if (gif->remainder_byte)
  {
    GIF_buffer[0] = gif->remainder_byte;

    for (index = 0; index <= gif->remainder_byte; index++)
      Write_one_byte(file, GIF_buffer[index]);

    gif->remainder_byte=0;
  }
}

/// Write a code (GIF_nb_bits bits)
static void GIF_set_code(FILE * GIF_file, T_GIF_context * gif, byte * GIF_buffer, word Code)
{
  word nb_bits_to_process = gif->nb_bits;
  word nb_bits_processed  =0;
  word current_nb_bits;

  while (nb_bits_to_process)
  {
    current_nb_bits = (nb_bits_to_process <= (8-gif->remainder_bits)) ?
                          nb_bits_to_process: (8-gif->remainder_bits);

    gif->last_byte |= (Code & ((1<<current_nb_bits)-1))<<gif->remainder_bits;
    Code>>=current_nb_bits;
    gif->remainder_bits    +=current_nb_bits;
    nb_bits_processed  +=current_nb_bits;
    nb_bits_to_process-=current_nb_bits;

    if (gif->remainder_bits==8) // Il ne reste plus de bits à coder sur l'octet courant
    {
      // Ecrire l'octet à balancer:
      GIF_buffer[++(gif->remainder_byte)] = gif->last_byte;

      // Si on a atteint la fin du bloc de Raster Data
      if (gif->remainder_byte==255)
        // On doit vider le buffer qui est maintenant plein
        GIF_empty_buffer(GIF_file, gif, GIF_buffer);

      gif->last_byte=0;
      gif->remainder_bits=0;
    }
  }
}


/// Read the next pixel
static byte GIF_next_pixel(T_IO_Context *context, T_GIF_context *gif, T_GIF_IDB *idb)
{
  byte temp;

  temp = Get_pixel(context, gif->pos_X, gif->pos_Y);

  if (++gif->pos_X >= (idb->Image_width + idb->Pos_X))
  {
    gif->pos_X = idb->Pos_X;
    if (++gif->pos_Y >= (idb->Image_height + idb->Pos_Y))
      gif->stop = 1;
  }

  return temp;
}


/// Save a GIF file
void Save_GIF(T_IO_Context * context)
{
  FILE * GIF_file;
  byte GIF_buffer[256];   // buffer d'écriture de bloc de données compilées

  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word * alphabet_daughter;    // Table des chaînes filles (plus longues)
  word * alphabet_sister;    // Table des chaînes soeurs (même longueur)
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   start;            // Code précédent (sert au linkage des chaînes)
  int    descend;          // Booléen "On vient de descendre"

  T_GIF_context GIF;
  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;


  byte block_identifier;  // Code indicateur du type de bloc en cours
  word current_string;   // Code de la chaîne en cours de traitement
  byte current_char;         // Caractère à coder
  word index;            // index de recherche de chaîne
  int current_layer;

  word clear;   // LZW clear code
  word eof;     // End of image code

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //
  
  File_error=0;
  
  if ((GIF_file=Open_file_write(context)))
  {
    setvbuf(GIF_file, NULL, _IOFBF, 64*1024);
    
    // On écrit la signature du fichier
    if (Write_bytes(GIF_file,"GIF89a",6))
    {
      // La signature du fichier a été correctement écrite.

      // Allocation de mémoire pour les tables
      alphabet_prefix=(word *)malloc(4096*sizeof(word));
      alphabet_suffix=(word *)malloc(4096*sizeof(word));
      alphabet_daughter  =(word *)malloc(4096*sizeof(word));
      alphabet_sister  =(word *)malloc(4096*sizeof(word));

      // On initialise le LSDB du fichier
      if (Config.Screen_size_in_GIF)
      {
        LSDB.Width=Screen_width;
        LSDB.Height=Screen_height;
      }
      else
      {
        LSDB.Width=context->Width;
        LSDB.Height=context->Height;
      }
      LSDB.Resol  = 0xF7;  // Global palette of 256 entries, 256 color image
      // 0xF7 = 1111 0111
      // <Packed Fields>  =      Global Color Table Flag       1 Bit
      //                         Color Resolution              3 Bits
      //                         Sort Flag                     1 Bit
      //                         Size of Global Color Table    3 Bits
      LSDB.Backcol=context->Transparent_color;
      switch(context->Ratio)
      {
        case PIXEL_TALL:
        case PIXEL_TALL2:
          LSDB.Aspect = 17; // 1:2 = 2:4
          break;
        case PIXEL_TALL3:
          LSDB.Aspect = 33; // 3:4
          break;
        case PIXEL_WIDE:
        case PIXEL_WIDE2:
          LSDB.Aspect = 113; // 2:1 = 4:2
          break;
        default:
          LSDB.Aspect = 0; // undefined, which is most frequent.
          // 49 would be 1:1 ratio
      }

      // On sauve le LSDB dans le fichier

      if (Write_word_le(GIF_file,LSDB.Width) &&
          Write_word_le(GIF_file,LSDB.Height) &&
          Write_byte(GIF_file,LSDB.Resol) &&
          Write_byte(GIF_file,LSDB.Backcol) &&
          Write_byte(GIF_file,LSDB.Aspect) )
      {
        // Le LSDB a été correctement écrit.
        int i;
        // On sauve la palette
        for(i=0;i<256 && !File_error;i++)
        {
          if (!Write_byte(GIF_file,context->Palette[i].R)
            ||!Write_byte(GIF_file,context->Palette[i].G)
            ||!Write_byte(GIF_file,context->Palette[i].B))
            File_error=1;
        }
        if (!File_error)
        {
          // La palette a été correctement écrite.

          /// - "Netscape" animation extension :
          /// <pre>
          ///   0x21       Extension Label
          ///   0xFF       Application Extension Label
          ///   0x0B       Block Size
          ///   "NETSCAPE" Application Identifier (8 bytes)
          ///   "2.0"      Application Authentication Code (3 bytes)
          ///   0x03       Sub-block Data Size
          ///   0xLL       01 to loop
          ///   0xSSSS     (little endian) number of loops, 0 means infinite loop
          ///   0x00 Block terminator </pre>
          /// see http://www.vurdalakov.net/misc/gif/netscape-looping-application-extension
          if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
          {
            if (context->Nb_layers>1)
              Write_bytes(GIF_file,"\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00",19);
          }
          else if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION)
          {
            /// - GrafX2 extension to store ::IMAGE_MODES :
            /// <pre>
            ///   0x21       Extension Label
            ///   0xFF       Application Extension Label
            ///   0x0B       Block Size
            ///   "GFX2MODE" Application Identifier (8 bytes)
            ///   "2.6"      Application Authentication Code (3 bytes)
            ///   0xll       Sub-block Data Size
            ///   string     label
            ///   0x00 Block terminator </pre>
            /// @see Constraint_mode_label()
            const char * label = Constraint_mode_label(Main.backups->Pages->Image_mode);
            if (label != NULL)
            {
              size_t len = strlen(label);
              // Write extension for storing IMAGE_MODE
              Write_byte(GIF_file,0x21);  // Extension Introducer
              Write_byte(GIF_file,0xff);  // Extension Label
              Write_byte(GIF_file,  11);  // Block size
              Write_bytes(GIF_file, "GFX2MODE2.6", 11); // Application Identifier + Appl. Authentication Code
              Write_byte(GIF_file, (byte)len);    // Block size
              Write_bytes(GIF_file, label, len);  // Data
              Write_byte(GIF_file, 0);    // Block terminator
            }
          }

          // Ecriture du commentaire
          if (context->Comment[0])
          {
            Write_bytes(GIF_file,"\x21\xFE",2);
            Write_byte(GIF_file, (byte)strlen(context->Comment));
            Write_bytes(GIF_file,context->Comment,strlen(context->Comment)+1);
          }
          /// - "CRNG" Color cycing extension :
          /// <pre>
          ///   0x21       Extension Label
          ///   0xFF       Application Extension Label
          ///   0x0B       Block Size
          ///   "CRNG\0\0\0\0" "CRNG" Application Identifier (8 bytes)
          ///   "1.0"      Application Authentication Code (3 bytes)
          ///   0xll       Sub-block Data Size (6 bytes per color cycle)
          ///   For each color cycle :
          ///     0xRRRR   (big endian) Rate
          ///     0xFFFF   (big endian) Flags
          ///     0xSS     start (lower color index)
          ///     0xEE     end (higher color index)
          ///   0x00       Block terminator </pre>
          if (context->Color_cycles)
          {
            int i;
            
            Write_bytes(GIF_file,"\x21\xff\x0B" "CRNG\0\0\0\0" "1.0",14);
            Write_byte(GIF_file,context->Color_cycles*6);
            for (i=0; i<context->Color_cycles; i++)
            {
              word flags=0;
              flags|= context->Cycle_range[i].Speed?1:0; // Cycling or not
              flags|= context->Cycle_range[i].Inverse?2:0; // Inverted
              
              Write_word_be(GIF_file,context->Cycle_range[i].Speed*78); // Rate
              Write_word_be(GIF_file,flags); // Flags
              Write_byte(GIF_file,context->Cycle_range[i].Start); // Min color
              Write_byte(GIF_file,context->Cycle_range[i].End); // Max color
            }
            Write_byte(GIF_file,0);
          }
          
          // Loop on all layers
          for (current_layer=0; 
            current_layer < context->Nb_layers && !File_error;
            current_layer++)
          {
            // Write a Graphic Control Extension
            T_GIF_GCE GCE;
            byte disposal_method;
            
            Set_saving_layer(context, current_layer);
            
            GCE.Block_identifier = 0x21;
            GCE.Function = 0xF9;
            GCE.Block_size=4;
            
            if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
            {
              // Animation frame
              int duration;
              if(context->Background_transparent)
                disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;
              else
                disposal_method = DISPOSAL_METHOD_DO_NOT_DISPOSE;
              GCE.Packed_fields=(disposal_method<<2)|(context->Background_transparent);
              duration=Get_frame_duration(context)/10;
              GCE.Delay_time=duration<0xFFFF?duration:0xFFFF;
            }
            else
            {
              // Layered image or brush
              disposal_method = DISPOSAL_METHOD_DO_NOT_DISPOSE;
              if (current_layer==0)
                GCE.Packed_fields=(disposal_method<<2)|(context->Background_transparent);
              else
                GCE.Packed_fields=(disposal_method<<2)|(1);
              GCE.Delay_time=5; // Duration 5/100s (minimum viable value for current web browsers)
              if (current_layer == context->Nb_layers -1)
                GCE.Delay_time=0xFFFF; // Infinity (10 minutes)
            }
            GCE.Transparent_color=context->Transparent_color;
            GCE.Block_terminator=0x00;
            
            if (Write_byte(GIF_file,GCE.Block_identifier)
             && Write_byte(GIF_file,GCE.Function)
             && Write_byte(GIF_file,GCE.Block_size)
             && Write_byte(GIF_file,GCE.Packed_fields)
             && Write_word_le(GIF_file,GCE.Delay_time)
             && Write_byte(GIF_file,GCE.Transparent_color)
             && Write_byte(GIF_file,GCE.Block_terminator)
             )
            {
              byte temp, max = 0;

              IDB.Pos_X=0;
              IDB.Pos_Y=0;
              IDB.Image_width=context->Width;
              IDB.Image_height=context->Height;
              if(current_layer > 0)
              {
                word min_X, max_X, min_Y, max_Y;
                // find bounding box of changes for Animated GIFs
                min_X = min_Y = 0xffff;
                max_X = max_Y = 0;
                for(GIF.pos_Y = 0; GIF.pos_Y < context->Height; GIF.pos_Y++) {
                  for(GIF.pos_X = 0; GIF.pos_X < context->Width; GIF.pos_X++) {
                    if (GIF.pos_X >= min_X && GIF.pos_X <= max_X && GIF.pos_Y >= min_Y && GIF.pos_Y <= max_Y)
                      continue; // already in the box
                    if(disposal_method == DISPOSAL_METHOD_DO_NOT_DISPOSE)
                    {
                      // if that pixel has same value in previous layer, no need to save it
                      Set_saving_layer(context, current_layer - 1);
                      temp = Get_pixel(context, GIF.pos_X, GIF.pos_Y);
                      Set_saving_layer(context, current_layer);
                      if(temp == Get_pixel(context, GIF.pos_X, GIF.pos_Y))
                        continue;
                    }
                    if (disposal_method == DISPOSAL_METHOD_RESTORE_BGCOLOR
                      || context->Background_transparent
                      || Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
                    {
                      // if that pixel is Backcol, no need to save it
                      if (LSDB.Backcol == Get_pixel(context, GIF.pos_X, GIF.pos_Y))
                        continue;
                    }
                    if(GIF.pos_X < min_X) min_X = GIF.pos_X;
                    if(GIF.pos_X > max_X) max_X = GIF.pos_X;
                    if(GIF.pos_Y < min_Y) min_Y = GIF.pos_Y;
                    if(GIF.pos_Y > max_Y) max_Y = GIF.pos_Y;
                  }
                }
                if((min_X <= max_X) && (min_Y <= max_Y))
                {
                  IDB.Pos_X = min_X;
                  IDB.Pos_Y = min_Y;
                  IDB.Image_width = max_X + 1 - min_X;
                  IDB.Image_height = max_Y + 1 - min_Y;
                }
                else
                {
                  // if no pixel changes, store a 1 pixel image
                  IDB.Image_width = 1;
                  IDB.Image_height = 1;
                }
              }

              // look for the maximum pixel value
              // to decide how many bit per pixel are needed.
              for(GIF.pos_Y = IDB.Pos_Y; GIF.pos_Y < IDB.Image_height + IDB.Pos_Y; GIF.pos_Y++) {
                for(GIF.pos_X = IDB.Pos_X; GIF.pos_X < IDB.Image_width + IDB.Pos_X; GIF.pos_X++) {
                  temp=Get_pixel(context, GIF.pos_X, GIF.pos_Y);
                  if(temp > max) max = temp;
                }
              }
              IDB.Nb_bits_pixel=2;  // Find the minimum bpp value to fit all pixels
              while((int)max >= (1 << IDB.Nb_bits_pixel)) {
                IDB.Nb_bits_pixel++;
              }
              GFX2_Log(GFX2_DEBUG, "GIF image #%d %ubits (%u,%u) %ux%u\n",
                       current_layer, IDB.Nb_bits_pixel, IDB.Pos_X, IDB.Pos_Y,
                       IDB.Image_width, IDB.Image_height);
            
              // On va écrire un block indicateur d'IDB et l'IDB du fichier
              block_identifier=0x2C;
              IDB.Indicator=0x07;    // Image non entrelacée, pas de palette locale.
              clear = 1 << IDB.Nb_bits_pixel; // Clear Code
              eof = clear + 1;                // End of Picture Code
    
              if ( Write_byte(GIF_file,block_identifier) &&
                   Write_word_le(GIF_file,IDB.Pos_X) &&
                   Write_word_le(GIF_file,IDB.Pos_Y) &&
                   Write_word_le(GIF_file,IDB.Image_width) &&
                   Write_word_le(GIF_file,IDB.Image_height) &&
                   Write_byte(GIF_file,IDB.Indicator) &&
                   Write_byte(GIF_file,IDB.Nb_bits_pixel))
              {
                //   Le block indicateur d'IDB et l'IDB ont étés correctements
                // écrits.
    
                GIF.pos_X=IDB.Pos_X;
                GIF.pos_Y=IDB.Pos_Y;
                GIF.last_byte=0;
                GIF.remainder_bits=0;
                GIF.remainder_byte=0;
    
#define GIF_INVALID_CODE (65535)
                index=GIF_INVALID_CODE;
                File_error=0;
                GIF.stop=0;
    
                // Réintialisation de la table:
                alphabet_free=clear + 2;  // 258 for 8bpp
                GIF.nb_bits  =IDB.Nb_bits_pixel + 1; // 9 for 8 bpp
                alphabet_max =clear+clear-1;  // 511 for 8bpp
                GIF_set_code(GIF_file, &GIF, GIF_buffer, clear);  //256 for 8bpp
                for (start=0;start<4096;start++)
                {
                  alphabet_daughter[start] = GIF_INVALID_CODE;
                  alphabet_sister[start] = GIF_INVALID_CODE;
                }
    
                ////////////////////////////////////////////// COMPRESSION LZW //
    
                start=current_string=GIF_next_pixel(context, &GIF, &IDB);
                descend=1;
    
                while ((!GIF.stop) && (!File_error))
                {
                  current_char=GIF_next_pixel(context, &GIF, &IDB);
    
                  // look for (current_string,current_char) in the alphabet
                  while ( (index != GIF_INVALID_CODE) &&
                          ( (current_string!=alphabet_prefix[index]) ||
                            (current_char      !=alphabet_suffix[index]) ) )
                  {
                    descend=0;
                    start=index;
                    index=alphabet_sister[index];
                  }
    
                  if (index != GIF_INVALID_CODE)
                  {
                    // (current_string,current_char) == (alphabet_prefix,alphabet_suffix)[index]
                    // We have found (current_string,current_char) in the alphabet
                    // at the index position. So go on and prepare for then next character
    
                    descend=1;
                    start=current_string=index;
                    index=alphabet_daughter[index];
                  }
                  else
                  {
                    // (current_string,current_char) was not found in the alphabet
                    // so write current_string to the Gif stream
                    GIF_set_code(GIF_file, &GIF, GIF_buffer, current_string);

                    if(alphabet_free < 4096) {
                      // link current_string and the new one
                      if (descend)
                        alphabet_daughter[start]=alphabet_free;
                      else
                        alphabet_sister[start]=alphabet_free;

                      // add (current_string,current_char) to the alphabet
                      alphabet_prefix[alphabet_free]=current_string;
                      alphabet_suffix[alphabet_free]=current_char;
                      alphabet_free++;
                    }
    
                    if (alphabet_free >= 4096)
                    {
                      // clear alphabet
                      GIF_set_code(GIF_file, &GIF, GIF_buffer, clear);    // 256 for 8bpp
                      alphabet_free=clear+2;  // 258 for 8bpp
                      GIF.nb_bits  =IDB.Nb_bits_pixel + 1;  // 9 for 8bpp
                      alphabet_max =clear+clear-1;    // 511 for 8bpp
                      for (start=0;start<4096;start++)
                      {
                        alphabet_daughter[start] = GIF_INVALID_CODE;
                        alphabet_sister[start] = GIF_INVALID_CODE;
                      }
                    }
                    else if (alphabet_free>alphabet_max+1)
                    {
                      // On augmente le nb de bits
    
                      GIF.nb_bits++;
                      alphabet_max = (1<<GIF.nb_bits)-1;
                    }
    
                    // initialize current_string as the string "current_char"
                    index=alphabet_daughter[current_char];
                    start=current_string=current_char;
                    descend=1;
                  }
                }
    
                if (!File_error)
                {
                  // Write the last code (before EOF)
                  GIF_set_code(GIF_file, &GIF, GIF_buffer, current_string);
    
                  // we need to update alphabet_free / GIF.nb_bits here because
                  // the decoder will update them after each code,
                  // so in very rare cases there might be a problem if we
                  // don't do it.
                  // see http://pulkomandy.tk/projects/GrafX2/ticket/125
                  if(alphabet_free < 4096)
                  {
                    alphabet_free++;
                    if ((alphabet_free > alphabet_max+1) && (GIF.nb_bits < 12))
                    {
                      GIF.nb_bits++;
                      alphabet_max = (1 << GIF.nb_bits) - 1;
                    }
                  }

                  GIF_set_code(GIF_file, &GIF, GIF_buffer, eof);  // 257 for 8bpp    // Code de End d'image
                  if (GIF.remainder_bits!=0)
                  {
                    // Write last byte (this is an incomplete byte)
                    GIF_buffer[++GIF.remainder_byte]=GIF.last_byte;
                    GIF.last_byte=0;
                    GIF.remainder_bits=0;
                  }
                  GIF_empty_buffer(GIF_file, &GIF, GIF_buffer); // On envoie les dernières données du buffer GIF dans le buffer KM
    
                  // On écrit un \0
                  if (! Write_byte(GIF_file,'\x00'))
                    File_error=1;
                }
      
              } // On a pu écrire l'IDB
              else
                File_error=1;
            }
            else
              File_error=1;
          }
          
          // After writing all layers
          if (!File_error)
          {
            /// - If requested, write a specific extension for storing
            /// original file path.
            /// This is used by the backup system.
            /// The format is :
            /// <pre>
            ///   0x21       Extension Label
            ///   0xFF       Application Extension Label
            ///   0x0B       Block Size
            ///   "GFX2PATH" "GFX2PATH" Application Identifier (8 bytes)
            ///   "\0\0\0"   Application Authentication Code (3 bytes)
            ///   0xll       Sub-block Data Size : path size (including null)
            ///   "..path.." path (null-terminated)
            ///   0xll       Sub-block Data Size : filename size (including null)
            ///   "..file.." file name (null-terminated)
            ///   0x00       Block terminator </pre>
            if (context->Original_file_name != NULL
             && context->Original_file_directory != NULL)
            {
              long name_size = 1+strlen(context->Original_file_name);
              long dir_size = 1+strlen(context->Original_file_directory);
              if (name_size<256 && dir_size<256)
              {
                if (! Write_bytes(GIF_file,"\x21\xFF\x0BGFX2PATH\x00\x00\x00", 14)
                || ! Write_byte(GIF_file,dir_size)
                || ! Write_bytes(GIF_file, context->Original_file_directory, dir_size)
                || ! Write_byte(GIF_file,name_size)
                || ! Write_bytes(GIF_file, context->Original_file_name, name_size)
                || ! Write_byte(GIF_file,0))
                  File_error=1;
              }
            }
          
            // On écrit un GIF TERMINATOR, exigé par SVGA et SEA.
            if (! Write_byte(GIF_file,'\x3B'))
              File_error=1;
          }

        } // On a pu écrire la palette
        else
          File_error=1;

      } // On a pu écrire le LSDB
      else
        File_error=1;

      // Libération de la mémoire utilisée par les tables
      free(alphabet_sister);
      free(alphabet_daughter);
      free(alphabet_suffix);
      free(alphabet_prefix);

    } // On a pu écrire la signature du fichier
    else
      File_error=1;

    fclose(GIF_file);
    if (File_error)
      Remove_file(context);

  } // On a pu ouvrir le fichier en écriture
  else
    File_error=1;

}

/* @} */


//////////////////////////////////// PCX ////////////////////////////////////
typedef struct
  {
    byte Manufacturer;       // |_ Il font chier ces cons! Ils auraient pu
    byte Version;            // |  mettre une vraie signature!
    byte Compression;        // L'image est-elle compressée?
    byte Depth;              // Nombre de bits pour coder un pixel (inutile puisqu'on se sert de Plane)
    word X_min;              // |_ Coin haut-gauche   |
    word Y_min;              // |  de l'image         |_ (Crétin!)
    word X_max;              // |_ Coin bas-droit     |
    word Y_max;              // |  de l'image         |
    word X_dpi;              // |_ Densité de |_ (Presque inutile parce que
    word Y_dpi;              // |  l'image    |  aucun moniteur n'est pareil!)
    byte Palette_16c[48];    // Palette 16 coul (inutile pour 256c) (débile!)
    byte Reserved;           // Ca me plait ça aussi!
    byte Plane;              // 4 => 16c , 1 => 256c , ...
    word Bytes_per_plane_line;// Doit toujours être pair
    word Palette_info;       // 1 => color , 2 => Gris (ignoré à partir de la version 4)
    word Screen_X;           // |_ Dimensions de
    word Screen_Y;           // |  l'écran d'origine
    byte Filler[54];         // Ca... J'adore!
  } T_PCX_Header;

T_PCX_Header PCX_header;

// -- Tester si un fichier est au format PCX --------------------------------

void Test_PCX(T_IO_Context * context, FILE * file)
{
  (void)context;
  File_error=0;

  if (Read_byte(file,&(PCX_header.Manufacturer)) &&
      Read_byte(file,&(PCX_header.Version)) &&
      Read_byte(file,&(PCX_header.Compression)) &&
      Read_byte(file,&(PCX_header.Depth)) &&
      Read_word_le(file,&(PCX_header.X_min)) &&
      Read_word_le(file,&(PCX_header.Y_min)) &&
      Read_word_le(file,&(PCX_header.X_max)) &&
      Read_word_le(file,&(PCX_header.Y_max)) &&
      Read_word_le(file,&(PCX_header.X_dpi)) &&
      Read_word_le(file,&(PCX_header.Y_dpi)) &&
      Read_bytes(file,&(PCX_header.Palette_16c),48) &&
      Read_byte(file,&(PCX_header.Reserved)) &&
      Read_byte(file,&(PCX_header.Plane)) &&
      Read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
      Read_word_le(file,&(PCX_header.Palette_info)) &&
      Read_word_le(file,&(PCX_header.Screen_X)) &&
      Read_word_le(file,&(PCX_header.Screen_Y)) &&
      Read_bytes(file,&(PCX_header.Filler),54) )
  {
    //   Vu que ce header a une signature de merde et peu significative, il
    // va falloir que je teste différentes petites valeurs dont je connais
    // l'intervalle. Grrr!
    if ( (PCX_header.Manufacturer!=10)
      || (PCX_header.Compression>1)
      || ( (PCX_header.Depth!=1) && (PCX_header.Depth!=2) && (PCX_header.Depth!=4) && (PCX_header.Depth!=8) )
      || ( (PCX_header.Plane!=1) && (PCX_header.Plane!=2) && (PCX_header.Plane!=4) && (PCX_header.Plane!=8) && (PCX_header.Plane!=3) )
      || (PCX_header.X_max<PCX_header.X_min)
      || (PCX_header.Y_max<PCX_header.Y_min)
      || (PCX_header.Bytes_per_plane_line&1) )
        File_error=1;
  }
  else
    File_error=1;

}


// -- Lire un fichier au format PCX -----------------------------------------

  // -- Afficher une ligne PCX codée sur 1 seul plan avec moins de 256 c. --
  static void Draw_PCX_line(T_IO_Context *context, const byte * buffer, short y_pos, byte depth)
  {
    short x_pos;
    byte  color;
    byte  reduction=8/depth;
    byte  byte_mask=(1<<depth)-1;
    byte  reduction_minus_one=reduction-1;

    for (x_pos=0; x_pos<context->Width; x_pos++)
    {
      color=(buffer[x_pos/reduction]>>((reduction_minus_one-(x_pos%reduction))*depth)) & byte_mask;
      Set_pixel(context, x_pos,y_pos,color);
    }
  }

// generate CGA RGBI colors.
static void Set_CGA_Color(int i, T_Components * comp)
{
  int intensity = (i & 8) ? 85 : 0;
  comp->R = ((i & 4) ? 170 : 0) + intensity;
  if (i == 6)
    comp->G = 85; // color 6 is brown instead of yellow on IBM CGA display
  else
    comp->G = ((i & 2) ? 170 : 0) + intensity;
  comp->B = ((i & 1) ? 170 : 0) + intensity;
}

void Load_PCX(T_IO_Context * context)
{
  FILE *file;
  
  short line_size;
  short real_line_size; // width de l'image corrigée
  short width_read;
  short x_pos;
  short y_pos;
  byte  byte1;
  byte  byte2;
  byte  index;
  dword nb_colors;
  long  file_size;

  long  position;
  long  image_size;
  byte * buffer;

  File_error=0;

  if ((file=Open_file_read(context)))
  {
    file_size=File_length_file(file);   
    if (Read_byte(file,&(PCX_header.Manufacturer)) &&
        Read_byte(file,&(PCX_header.Version)) &&
        Read_byte(file,&(PCX_header.Compression)) &&
        Read_byte(file,&(PCX_header.Depth)) &&
        Read_word_le(file,&(PCX_header.X_min)) &&
        Read_word_le(file,&(PCX_header.Y_min)) &&
        Read_word_le(file,&(PCX_header.X_max)) &&
        Read_word_le(file,&(PCX_header.Y_max)) &&
        Read_word_le(file,&(PCX_header.X_dpi)) &&
        Read_word_le(file,&(PCX_header.Y_dpi)) &&
        Read_bytes(file,&(PCX_header.Palette_16c),48) &&        
        Read_byte(file,&(PCX_header.Reserved)) &&
        Read_byte(file,&(PCX_header.Plane)) &&
        Read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
        Read_word_le(file,&(PCX_header.Palette_info)) &&
        Read_word_le(file,&(PCX_header.Screen_X)) &&
        Read_word_le(file,&(PCX_header.Screen_Y)) &&
        Read_bytes(file,&(PCX_header.Filler),54) )
    {
      Pre_load(context,
               PCX_header.X_max - PCX_header.X_min + 1,
               PCX_header.Y_max - PCX_header.Y_min + 1,
               file_size, FORMAT_PCX, PIXEL_SIMPLE,
               PCX_header.Plane * PCX_header.Depth);

      Original_screen_X = PCX_header.Screen_X;
      Original_screen_Y = PCX_header.Screen_Y;

      if (!(PCX_header.Plane==3 && PCX_header.Depth==8))
      {
        if (File_error==0)
        {
          // On prépare la palette à accueillir les valeurs du fichier PCX
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
          nb_colors=(dword)(1<<PCX_header.Plane)<<(PCX_header.Depth-1);

          memcpy(context->Palette,PCX_header.Palette_16c,48);

          if (nb_colors<=4)
          {
            // CGA !
            int i;

            if (PCX_header.Version < 5           // Detect if the palette is usable
                || (nb_colors == 4
                 && (PCX_header.Palette_16c[6]&15) == 0
                 && (PCX_header.Palette_16c[7]&15) == 0
                 && (PCX_header.Palette_16c[8]&15) == 0
                 && (PCX_header.Palette_16c[9]&15) == 0
                 && (PCX_header.Palette_16c[10]&15) == 0
                 && (PCX_header.Palette_16c[11]&15) == 0)
                || (nb_colors == 2
                 && PCX_header.Palette_16c[1] == 0
                 && PCX_header.Palette_16c[2] == 0))
            {
              // special CGA palette meaning :
              if (nb_colors == 2)
              {
                // Background : BLACK
                context->Palette[0].R=0;
                context->Palette[0].G=0;
                context->Palette[0].B=0;
                // Foreground : 4 MSB of palette[0] is index of the CGA color to use.
                i = (PCX_header.Palette_16c[0] >> 4);
                if (i==0) i = 15;  // Bright White by default
                Set_CGA_Color(i, &context->Palette[1]);
              }
              else
              {
                // Color CGA
                // background color : 4 MSB of palette[0]
                Set_CGA_Color((PCX_header.Palette_16c[0] >> 4), &context->Palette[0]);
                // Palette_16c[3] : 8 bits CPIx xxxx
                // C bit : Color burst enabled  => disable it to set 3rd palette
                // P bit : palette : 0 = yellow/ 1 = white
                // I bit : intensity
                // CGA Palette 0 : 2 green, 4 red, 6 brown
                // CGA Palette 1 : 3 cyan, 5 magenta, 7 white
                // CGA 3rd palette : 3 cyan, 4 red, 7 white
                // After some tests in PC Paintbrush 3.11, it looks like
                // the Color burst bit is not taken into acount.
                i = 2;  // 2 - CGA Green
                if (PCX_header.Palette_16c[3] & 0x40)
                  i++; // Palette 1 (3 = cyan)
                if (PCX_header.Palette_16c[3] & 0x20)
                  i += 8; // High intensity
                Set_CGA_Color(i++, &context->Palette[1]);
                i++; // Skip 1 color
                Set_CGA_Color(i++, &context->Palette[2]);
                i++; // Skip 1 color
                Set_CGA_Color(i, &context->Palette[3]);
              }
            }
          }

          //   On se positionne à la fin du fichier - 769 octets pour voir s'il y
          // a une palette.
          if ( (PCX_header.Depth==8) && (PCX_header.Version>=5) && (file_size>(256*3+128)) )
          {
            fseek(file,file_size-((256*3)+1),SEEK_SET);
            // On regarde s'il y a une palette après les données de l'image
            if (Read_byte(file,&byte1))
              if (byte1==12) // Lire la palette si c'est une image en 256 couleurs
              {
                int index;
                // On lit la palette 256c que ces crétins ont foutue à la fin du fichier
                for(index=0;index<256;index++)
                  if ( ! Read_byte(file,&(context->Palette[index].R))
                   || ! Read_byte(file,&(context->Palette[index].G))
                   || ! Read_byte(file,&(context->Palette[index].B)) )
                  {
                    File_error=2;
                    GFX2_Log(GFX2_ERROR, "ERROR READING PCX PALETTE ! index=%d\n", index);
                    break;
                  }
              }
          }

          //   Maintenant qu'on a lu la palette que ces crétins sont allés foutre
          // à la fin, on retourne juste après le header pour lire l'image.
          fseek(file,128,SEEK_SET);
          if (!File_error)
          {
            line_size=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
            real_line_size=(short)PCX_header.Bytes_per_plane_line<<3;
            //   On se sert de données ILBM car le dessin de ligne en moins de 256
            // couleurs se fait comme avec la structure ILBM.
            buffer=(byte *)malloc(line_size);

            // Chargement de l'image
            if (PCX_header.Compression)  // Image compressée
            {
              /*Init_lecture();*/
  
              image_size=(long)PCX_header.Bytes_per_plane_line*context->Height;

              if (PCX_header.Depth==8) // 256 couleurs (1 plan)
              {
                for (position=0; ((position<image_size) && (!File_error));)
                {
                  // Lecture et décompression de la ligne
                  if(Read_byte(file,&byte1) !=1) File_error=2;
                  if (!File_error)
                  {
                    if ((byte1&0xC0)==0xC0)
                    {
                      byte1-=0xC0;               // facteur de répétition
                      if(Read_byte(file,&byte2)!=1) File_error = 2; // octet à répéter
                      if (!File_error)
                      {
                        for (index=0; index<byte1; index++,position++)
                          if (position<image_size)
                            Set_pixel(context, position%line_size,
                                                position/line_size,
                                                byte2);
                          else
                            File_error=2;
                      }
                    }
                    else
                    {
                      Set_pixel(context, position%line_size,
                                          position/line_size,
                                          byte1);
                      position++;
                    }
                  }
                }
              }
              else                 // couleurs rangées par plans
              {
                for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
                {
                  for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
                  {
                    if(Read_byte(file,&byte1)!=1) File_error = 2;
                    if (!File_error)
                    {
                      if ((byte1&0xC0)==0xC0)
                      {
                        byte1-=0xC0;               // facteur de répétition
                        if(Read_byte(file,&byte2)!=1) File_error=2; // octet à répéter
                        if (!File_error)
                        {
                          for (index=0; index<byte1; index++)
                            if (x_pos<line_size)
                              buffer[x_pos++]=byte2;
                            else
                              File_error=2;
                        }
                      }
                      else
                        buffer[x_pos++]=byte1;
                    }
                  }
                  // Affichage de la ligne par plan du buffer
                  if (PCX_header.Depth==1)
                    Draw_IFF_line(context, buffer, y_pos,real_line_size,PCX_header.Plane);
                  else
                    Draw_PCX_line(context, buffer, y_pos,PCX_header.Depth);
                }
              }

              /*Close_lecture();*/
            }
            else                     // Image non compressée
            {
              for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
              {
                if ((width_read=Read_bytes(file,buffer,line_size)))
                {
                  if (PCX_header.Plane==1)
                    for (x_pos=0; x_pos<context->Width;x_pos++)
                      Set_pixel(context, x_pos,y_pos,buffer[x_pos]);
                  else
                  {
                    if (PCX_header.Depth==1)
                      Draw_IFF_line(context, buffer, y_pos,real_line_size,PCX_header.Plane);
                    else
                      Draw_PCX_line(context, buffer, y_pos,PCX_header.Depth);
                  }
                }
                else
                  File_error=2;
              }
            }

            free(buffer);
          }
        }
      }
      else
      {
        // Image 24 bits!!!
        if (File_error==0)
        {
          line_size=PCX_header.Bytes_per_plane_line*3;
          buffer=(byte *)malloc(line_size);

          if (!PCX_header.Compression)
          {
            for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,buffer,line_size))
              {
                for (x_pos=0; x_pos<context->Width; x_pos++)
                  Set_pixel_24b(context, x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
              }
              else
                File_error=2;
            }
          }
          else
          {
            /*Init_lecture();*/

            for (y_pos=0,position=0;(y_pos<context->Height) && (!File_error);)
            {
              // Lecture et décompression de la ligne
              if(Read_byte(file,&byte1)!=1) File_error=2;
              if (!File_error)
              {
                if ((byte1 & 0xC0)==0xC0)
                {
                  byte1-=0xC0;               // facteur de répétition
                  if(Read_byte(file,&byte2)!=1) File_error=2; // octet à répéter
                  if (!File_error)
                  {
                    for (index=0; (index<byte1) && (!File_error); index++)
                    {
                      buffer[position++]=byte2;
                      if (position>=line_size)
                      {
                        for (x_pos=0; x_pos<context->Width; x_pos++)
                          Set_pixel_24b(context, x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                        y_pos++;
                        position=0;
                      }
                    }
                  }
                }
                else
                {
                  buffer[position++]=byte1;
                  if (position>=line_size)
                  {
                    for (x_pos=0; x_pos<context->Width; x_pos++)
                      Set_pixel_24b(context, x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                    y_pos++;
                    position=0;
                  }
                }
              }
            }
            if (position!=0)
              File_error=2;

            /*Close_lecture();*/
          }
          free(buffer);
          buffer = NULL;
        }
      }
    }
    else
    {
      File_error=1;
    }

    fclose(file);
  }
  else
    File_error=1;
}


// -- Ecrire un fichier au format PCX ---------------------------------------

void Save_PCX(T_IO_Context * context)
{
  FILE *file;

  short line_size;
  short x_pos;
  short y_pos;
  byte  counter;
  byte  last_pixel;
  byte  pixel_read;



  File_error=0;

  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);
    
    PCX_header.Manufacturer=10;
    PCX_header.Version=5;
    PCX_header.Compression=1;
    PCX_header.Depth=8;
    PCX_header.X_min=0;
    PCX_header.Y_min=0;
    PCX_header.X_max=context->Width-1;
    PCX_header.Y_max=context->Height-1;
    PCX_header.X_dpi=0;
    PCX_header.Y_dpi=0;
    memcpy(PCX_header.Palette_16c,context->Palette,48);
    PCX_header.Reserved=0;
    PCX_header.Plane=1;
    PCX_header.Bytes_per_plane_line=(context->Width&1)?context->Width+1:context->Width;
    PCX_header.Palette_info=1;
    PCX_header.Screen_X=Screen_width;
    PCX_header.Screen_Y=Screen_height;
    memset(PCX_header.Filler,0,54);

    if (Write_bytes(file,&(PCX_header.Manufacturer),1) &&
        Write_bytes(file,&(PCX_header.Version),1) &&
        Write_bytes(file,&(PCX_header.Compression),1) &&
        Write_bytes(file,&(PCX_header.Depth),1) &&
        Write_word_le(file,PCX_header.X_min) &&
        Write_word_le(file,PCX_header.Y_min) &&
        Write_word_le(file,PCX_header.X_max) &&
        Write_word_le(file,PCX_header.Y_max) &&
        Write_word_le(file,PCX_header.X_dpi) &&
        Write_word_le(file,PCX_header.Y_dpi) &&
        Write_bytes(file,&(PCX_header.Palette_16c),48) &&
        Write_bytes(file,&(PCX_header.Reserved),1) &&
        Write_bytes(file,&(PCX_header.Plane),1) &&
        Write_word_le(file,PCX_header.Bytes_per_plane_line) &&
        Write_word_le(file,PCX_header.Palette_info) &&
        Write_word_le(file,PCX_header.Screen_X) &&
        Write_word_le(file,PCX_header.Screen_Y) &&
        Write_bytes(file,&(PCX_header.Filler),54) )
    {
      line_size=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
     
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        pixel_read=Get_pixel(context, 0,y_pos);
     
        // Compression et écriture de la ligne
        for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
        {
          x_pos++;
          last_pixel=pixel_read;
          pixel_read=Get_pixel(context, x_pos,y_pos);
          counter=1;
          while ( (counter<63) && (x_pos<line_size) && (pixel_read==last_pixel) )
          {
            counter++;
            x_pos++;
            pixel_read=Get_pixel(context, x_pos,y_pos);
          }
      
          if ( (counter>1) || (last_pixel>=0xC0) )
            Write_one_byte(file,counter|0xC0);
          Write_one_byte(file,last_pixel);
      
        }
      }
      
      // Ecriture de l'octet (12) indiquant que la palette arrive
      if (!File_error)
        Write_one_byte(file,12);
      
      // Ecriture de la palette
      if (!File_error)
      {
        if (! Write_bytes(file,context->Palette,sizeof(T_Palette)))
          File_error=1;
      }
    }
    else
      File_error=1;
       
    fclose(file);
       
    if (File_error)
      Remove_file(context);
       
  }    
  else 
    File_error=1;
}      


//////////////////////////////////// SCx ////////////////////////////////////
/**
 * @defgroup SCx SCx format
 * @ingroup loadsaveformats
 * ColoRix VGA Paint SCx File Format
 *
 * file extensions are sci, scq, scf, scn, sco
 * @{
 */

/// SCx header data
typedef struct
{
  byte Filler1[4];  ///< "RIX3"
  word Width;       ///< Image Width
  word Height;      ///< Image Height
  byte PaletteType; ///< M P RGB PIX  0xAF = VGA
  byte StorageType; ///< 00 = Linear (1 byte per pixel) 01,02 Planar 03 text 80 Compressed 40 extension block 20 encrypted
} T_SCx_Header;

/// Test if a file is SCx format
void Test_SCx(T_IO_Context * context, FILE * file)
{
  T_SCx_Header SCx_header;

  (void)context;
  File_error=1;

  // read and check header
  if (Read_bytes(file,SCx_header.Filler1,4)
      && Read_word_le(file, &(SCx_header.Width))
      && Read_word_le(file, &(SCx_header.Height))
      && Read_byte(file, &(SCx_header.PaletteType))
      && Read_byte(file, &(SCx_header.StorageType))
     )
  {
    if ( (!memcmp(SCx_header.Filler1,"RIX",3))
        && SCx_header.Width && SCx_header.Height)
      File_error=0;
  }
}


/// Read a SCx file
void Load_SCx(T_IO_Context * context)
{
  FILE *file;
  word x_pos,y_pos;
  long size,real_size;
  long file_size;
  T_SCx_Header SCx_header;
  T_Palette SCx_Palette;
  byte * buffer;
  byte bpp;

  File_error=0;

  if ((file=Open_file_read(context)))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,SCx_header.Filler1,4)
    && Read_word_le(file, &(SCx_header.Width))
    && Read_word_le(file, &(SCx_header.Height))
    && Read_byte(file, &(SCx_header.PaletteType))
    && Read_byte(file, &(SCx_header.StorageType))
    )
    {
      bpp = (SCx_header.PaletteType & 7) + 1;
      // Bit per RGB component in palette = ((SCx_header.PaletteType >> 3) & 7)
      Pre_load(context, SCx_header.Width,SCx_header.Height,file_size,FORMAT_SCx,PIXEL_SIMPLE,bpp);
      size=sizeof(T_Components)*(1 << bpp);

      if (SCx_header.PaletteType & 0x80)
      {
        if (!Read_bytes(file, SCx_Palette, size))
          File_error = 2;
        else
        {
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));

          Palette_64_to_256(SCx_Palette);
          memcpy(context->Palette,SCx_Palette,size);
        }
      }
      if (File_error == 0)
      {
        if (SCx_header.StorageType == 0x80)
        {
          Warning("Compressed SCx files are not supported");
          File_error = 2;
        }
        else
        {
          if (SCx_header.StorageType == 0)
          { // 256 couleurs (raw)
            buffer=(byte *)malloc(context->Width);

            for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,buffer,context->Width))
                for (x_pos=0; x_pos<context->Width;x_pos++)
                  Set_pixel(context, x_pos,y_pos,buffer[x_pos]);
              else
                File_error=2;
            }
          }
          else
          { // moins de 256 couleurs (planar)
            size=((context->Width+7)>>3)*bpp;
            real_size=(size/bpp)<<3;
            buffer=(byte *)malloc(size);

            for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,buffer,size))
                Draw_IFF_line(context, buffer, y_pos,real_size,bpp);
              else
                File_error=2;
            }
          }
          free(buffer);
        }
      }
    }
    else
      File_error=1;

    fclose(file);
  }
  else
    File_error=1;
}

/// Save a SCx file
void Save_SCx(T_IO_Context * context)
{
  FILE *file;
  short x_pos,y_pos;
  T_SCx_Header SCx_header;
  size_t last_char;

  // replace the '?' in file extension with the right letter
  last_char = strlen(context->File_name) - 1;
  if (context->File_name[last_char] == '?')
  {
    if (context->Width<=320)
      context->File_name[last_char]='I';
    else
    {
      if (context->Width<=360)
        context->File_name[last_char]='Q';
      else
      {
        if (context->Width<=640)
          context->File_name[last_char]='F';
        else
        {
          if (context->Width<=800)
            context->File_name[last_char]='N';
          else
            context->File_name[last_char]='O';
        }
      }
    }
    // makes it same case as the previous character
    if (last_char > 0)
      context->File_name[last_char] |= (context->File_name[last_char - 1] & 32);
    // also fix the unicode file name
    if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
    {
      size_t ulen = Unicode_strlen(context->File_name_unicode);
      if (ulen > 1)
        context->File_name_unicode[ulen - 1] = context->File_name[last_char];
    }
  }


  file = Open_file_write(context);

  if (file != NULL)
  {
    T_Palette palette_64;
    
    File_error = 0;
    memcpy(palette_64, context->Palette, sizeof(T_Palette));
    Palette_256_to_64(palette_64);
    
    memcpy(SCx_header.Filler1,"RIX3",4);
    SCx_header.Width=context->Width;
    SCx_header.Height=context->Height;
    SCx_header.PaletteType=0xAF;
    SCx_header.StorageType=0x00;

    if (Write_bytes(file,SCx_header.Filler1, 4)
    && Write_word_le(file, SCx_header.Width)
    && Write_word_le(file, SCx_header.Height)
    && Write_byte(file, SCx_header.PaletteType)
    && Write_byte(file, SCx_header.StorageType)
    && Write_bytes(file,&palette_64,sizeof(T_Palette))
    )
    {
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
        for (x_pos=0; x_pos<context->Width; x_pos++)
          Write_one_byte(file, Get_pixel(context, x_pos, y_pos));
    }
    else
    {
      File_error = 1;
    }
    fclose(file);
    if (File_error)
      Remove_file(context);
  }
  else
  {
    File_error=1;
  }
}

/** @} */

//////////////////////////////////// XPM ////////////////////////////////////
void Save_XPM(T_IO_Context* context)
{
  // XPM are unix files, so use LF '\n' line endings
  FILE* file;
  int i,j;
  byte max_color = 0;
  word color_count;

  File_error = 0;

  file = Open_file_write(context);
  if (file == NULL)
  {
    File_error = 1;
    return;
  }
  setvbuf(file, NULL, _IOFBF, 64*1024);
  // in case there are less colors than 256, we could
  // optimize, and use only 1 character per pixel if possible
  // printable characters are from 0x20 to 0x7e, minus " 0x22 and \ 0x5c
#define XPM_USABLE_CHARS (0x7f - 0x20 - 2)
  for (j = 0; j < context->Height; j++)
    for (i = 0; i < context->Width; i++)
    {
      byte value = Get_pixel(context, i, j);
      if (value > max_color)
        max_color = value;
    }
  color_count = (word)max_color + 1;

  fprintf(file, "/* XPM */\nstatic char* pixmap[] = {\n");
  fprintf(file, "\"%d %d %d %d\",\n", context->Width, context->Height, color_count, color_count > XPM_USABLE_CHARS ? 2 : 1);

  if (color_count > XPM_USABLE_CHARS)
  {
    for (i = 0; i < color_count; i++)
    {
      if (context->Background_transparent && (i == context->Transparent_color))
        fprintf(file, "\"%2.2X\tc None\",\n", i); // None is for transparent color
      else
        fprintf(file,"\"%2.2X\tc #%2.2x%2.2x%2.2x\",\n", i, context->Palette[i].R, context->Palette[i].G,
          context->Palette[i].B);
    }

    for (j = 0; j < context->Height; j++)
    {
      fprintf(file, "\"");
      for (i = 0; i < context->Width; i++)
        fprintf(file, "%2.2X", Get_pixel(context, i, j));
      if (j == (context->Height - 1))
        fprintf(file,"\"\n");
      else
        fprintf(file,"\",\n");
    }
  }
  else
  {
    int c;
    for (i = 0; i < color_count; i++)
    {
      c = (i < 2) ? i + 0x20 : i + 0x21;
      if (c >= 0x5c) c++;
      if (context->Background_transparent && (i == context->Transparent_color))
        fprintf(file, "\"%c\tc None\",\n", c); // None is for transparent color
      else
        fprintf(file,"\"%c\tc #%2.2x%2.2x%2.2x\",\n", c, context->Palette[i].R, context->Palette[i].G,
          context->Palette[i].B);
    }

    for (j = 0; j < context->Height; j++)
    {
      fprintf(file, "\"");
      for (i = 0; i < context->Width; i++)
      {
        c = Get_pixel(context, i, j);
        c = (c < 2) ? c + 0x20 : c + 0x21;
        if (c >= 0x5c) c++;
        fprintf(file, "%c", c);
      }
      if (j == (context->Height - 1))
        fprintf(file,"\"\n");
      else
        fprintf(file,"\",\n");
    }
  }
  fprintf(file, "};\n");

  fclose(file);
}
