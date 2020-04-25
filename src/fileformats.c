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
      GFX2_Log(GFX2_WARNING, "Decode_NewIcons(): Invalid value 0x%02x\n", value);
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
                          GFX2_Log(GFX2_WARNING, "Too much colors in new icons\n");
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
                          GFX2_Log(GFX2_WARNING, "Too much colors in new icons\n");
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
            GFX2_Log(GFX2_WARNING, "PNG Signature : Compiled without libpng support\n");
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
      GFX2_Log(GFX2_WARNING, "BMP: Unknown compression type %d\n", compression);
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
      GFX2_Log(GFX2_WARNING, "Unknown BMP type Size_2=%u\n", header.Size_2);
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
            GFX2_Log(GFX2_WARNING, "PNG Signature : Compiled without libpng support\n");
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
                GFX2_Log(GFX2_WARNING, "Load_ICO() : real_height(%hd) != entry->height(%hd)\n", real_height, entry->height);
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
    GFX2_Log(GFX2_WARNING, ".ICO files can handle images up to 256x256\n");
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
          GFX2_Log(GFX2_WARNING, "Compressed SCx files are not supported\n");
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
