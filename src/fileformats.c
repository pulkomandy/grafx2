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

#ifndef __no_pnglib__
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
#endif

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif


#include <stdlib.h>

#include "errors.h"
#include "global.h"
#include "loadsave.h"
#include "misc.h"
#include "struct.h"
#include "io.h"
#include "pages.h"
#include "windows.h" // Best_color()

#ifndef __no_pnglib__
static void Load_PNG_Sub(T_IO_Context * context, FILE * file);
#endif

//////////////////////////////////// IMG ////////////////////////////////////

// -- Tester si un fichier est au format IMG --------------------------------
void Test_IMG(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_IMG_Header IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
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
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format IMG -----------------------------------------
void Load_IMG(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte * buffer;
  FILE *file;
  word x_pos,y_pos;
  long file_size;
  T_IMG_Header IMG_header;

  Get_full_filename(filename, context->File_name, context->File_directory);
  File_error=0;

  if ((file=fopen(filename, "rb")))
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

        context->Width=IMG_header.Width;
        context->Height=IMG_header.Height;

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
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_IMG_Header IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
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
        remove(filename);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}


//////////////////////////////////// IFF ////////////////////////////////////
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

// -- Tester si un fichier est au format IFF --------------------------------

void Test_IFF(T_IO_Context * context, const char *sub_type)
{
  FILE * IFF_file;
  char filename[MAX_PATH_CHARACTERS];
  char  format[4];
  char  section[4];
  dword dummy;

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  if ((IFF_file=fopen(filename, "rb")))
  {
    do // Dummy loop, so that all breaks jump to end.
    {
      if (! Read_bytes(IFF_file,section,4))
        break;
      if (memcmp(section,"FORM",4))
        break;
      
      if (! Read_dword_be(IFF_file, &dummy))
        break;
      //   On aurait pu vérifier que ce long est égal à la taille
      // du fichier - 8, mais ça aurait interdit de charger des
      // fichiers tronqués (et déjà que c'est chiant de perdre
      // une partie du fichier il faut quand même pouvoir en
      // garder un peu... Sinon, moi je pleure :'( !!! )
      if (! Read_bytes(IFF_file,format,4))
        break;
        
      if (!memcmp(format,"ANIM",4))
      {
        // An ANIM header: need to check that it encloses an image
        if (! Read_bytes(IFF_file,section,4))
          break;
        if (memcmp(section,"FORM",4))
          break;
        if (! Read_dword_be(IFF_file, &dummy))
          break;
        if (! Read_bytes(IFF_file,format,4))
          break;
      }
      else if(memcmp(format,"DPST",4) == 0)
      {
        if (! Read_bytes(IFF_file,section,4))
          break;
        if (memcmp(section, "DPAH", 4) != 0)
          break;
        if (! Read_dword_be(IFF_file, &dummy))
          break;
        fseek(IFF_file, dummy, SEEK_CUR);
        if (! Read_bytes(IFF_file,section,4))
          break;
        if (memcmp(section,"FORM",4))
          break;
        if (! Read_dword_be(IFF_file, &dummy))
          break;
        if (! Read_bytes(IFF_file,format,4))
          break;
      }
      if ( memcmp(format,sub_type,4))
        break;
      
      // If we reach this part, file is indeed ILBM/PBM or ANIM
      File_error=0;
      
    } while (0);
    
    fclose(IFF_file);
  }
}

void Test_PBM(T_IO_Context * context)
{
  Test_IFF(context, "PBM ");
}
void Test_LBM(T_IO_Context * context)
{
  Test_IFF(context, "ILBM");
}
void Test_ACBM(T_IO_Context * context)
{
  Test_IFF(context, "ACBM");
}


// -- Lire un fichier au format IFF -----------------------------------------

typedef struct T_IFF_PCHG_Palette {
  struct T_IFF_PCHG_Palette * Next;
  short StartLine;
  T_Components Palette[1];
} T_IFF_PCHG_Palette;

// Inspired by Allegro: storing a 4-character identifier as a 32bit litteral
#define ID4(a,b,c,d) ((((a)&255)<<24) | (((b)&255)<<16) | (((c)&255)<<8) | (((d)&255)))

/// Skips the current section in an IFF file.
/// This function should be called while the file pointer is right
/// after the 4-character code that identifies the section.
int IFF_Skip_section(FILE * file)
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

// ------------------------- Attendre une section -------------------------
byte IFF_Wait_for(FILE * file, const char * expected_section)
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
dword Get_IFF_color(const byte * buffer, word x_pos, word real_line_size, byte bitplanes)
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

void Set_IFF_color(byte * buffer, word x_pos, byte color, word real_line_size, byte bitplanes)
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
static void Draw_IFF_line(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes)
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
    Set_pixel(context, x_pos,y_pos,Get_IFF_color(buffer, x_pos,real_line_size, bitplanes));
  }
}

static void Draw_IFF_line_PCHG(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes, const T_IFF_PCHG_Palette * PCHG_palettes)
{
  const T_IFF_PCHG_Palette * palette;
  short x_pos;

  palette = PCHG_palettes;  // find the palette to use for the line
  if (palette == NULL)
    return;
  while (palette->Next != NULL && palette->Next->StartLine <= y_pos)
    palette = palette->Next;

//printf("%d %p %d\n", y_pos, palette, palette->StartLine);
  for (x_pos=0; x_pos<context->Width; x_pos++)
  {
    dword c = Get_IFF_color(buffer, x_pos,real_line_size, bitplanes);
    Set_pixel_24b(context, x_pos,y_pos, palette->Palette[c].R, palette->Palette[c].G, palette->Palette[c].B);
  }
}

static void Draw_IFF_line_HAM(T_IO_Context *context, const byte * buffer, short y_pos, short real_line_size, byte bitplanes, const T_Components * SHAM_palettes, int SHAM_palette_count)
{
  short x_pos;
  byte red, green, blue, temp;
  const T_Components * palette;

  if (SHAM_palettes == NULL)
    palette = context->Palette;
  else
  {
    if (SHAM_palette_count >= context->Height)
      palette = SHAM_palettes + 16*y_pos;
    else
      palette = SHAM_palettes + 16*(y_pos >> 1);
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
      Warning("PBM only supports compression type 0 and 1");
      File_error = 50;
  }
}

static void IFF_Set_EHB_Palette(T_IO_Context * context)
{
  int i, j;            // 32 colors in the palette.
  for (i=0; i<32; i++) // The next 32 colors are the same with values divided by 2
  {
    j=i+32;
    context->Palette[j].R=context->Palette[i].R>>1;
    context->Palette[j].G=context->Palette[i].G>>1;
    context->Palette[j].B=context->Palette[i].B>>1;
  }
}

void Load_IFF(T_IO_Context * context)
{
  FILE * IFF_file;
  char filename[MAX_PATH_CHARACTERS];
  T_IFF_Header header;
  char  format[4];
  char  section[4];
  byte  temp_byte;
  short b256;
  dword nb_colors = 0; // number of colors in the CMAP (color map)
  dword section_size;
  short x_pos;
  short y_pos;
  short counter;
  short line_size = 0;        // Taille d'une ligne en octets
  short plane_line_size = 0;  // Size of line in bytes for 1 plane
  short real_line_size = 0;   // Taille d'une ligne en pixels
  byte  color;
  long  file_size;
  dword dummy;
  int iff_format = 0;
  int plane;
  dword AmigaViewModes = 0;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  byte * buffer;
  T_Components * SHAM_palettes = NULL;
  unsigned SHAM_palette_count = 0;
  byte bpp = 0;
  byte Image_HAM = 0;
  T_IFF_PCHG_Palette * PCHG_palettes = NULL;
  int current_frame = 0;
  byte * previous_frame = NULL; // For animations

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((IFF_file=fopen(filename, "rb")))
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
      Read_dword_be(IFF_file,&dummy);
      Read_bytes(IFF_file,format,4);
    }
    else if(memcmp(format,"DPST",4)==0)
    {
      // TODO : read DPAH
      if (!IFF_Wait_for(IFF_file, "FORM"))
        File_error=1;
      Read_dword_be(IFF_file,&dummy);
      Read_bytes(IFF_file,format,4);
    }
    if (memcmp(format,"ILBM",4) == 0)
      iff_format = FORMAT_LBM;
    else if(memcmp(format,"PBM ",4) == 0)
      iff_format = FORMAT_PBM;
    else if(memcmp(format,"ACBM",4) == 0)
      iff_format = FORMAT_ACBM;
    else
    {
      char tmp_msg[60];
      snprintf(tmp_msg, sizeof(tmp_msg), "Unknown IFF format '%.4s'", format);
      Warning(tmp_msg);
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
          continue;
        }
        if (memcmp(section, "BMHD", 4) == 0)
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
            && (Read_word_be(IFF_file,&header.Y_screen))
            && header.Width && header.Height))
          {
            File_error = 1;
            break;
          }
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
        }
        else if (memcmp(section, "ANHD", 4) == 0) // ANimation  HeaDer
        {
          // http://www.textfiles.com/programming/FORMATS/anim7.txt
          byte operation;
          byte mask;
          word w,h;
          word x,y;
          dword abstime;
          dword reltime;
          byte interleave;
          byte pad0;
          dword bits;

          Read_byte(IFF_file, &operation);
          Read_byte(IFF_file, &mask);
          Read_word_be(IFF_file, &w);
          Read_word_be(IFF_file, &h);
          Read_word_be(IFF_file, &x);
          Read_word_be(IFF_file, &y);
          Read_dword_be(IFF_file, &abstime);
          Read_dword_be(IFF_file, &reltime);
          Read_byte(IFF_file, &interleave);
          Read_byte(IFF_file, &pad0);
          Read_dword_be(IFF_file, &bits);
          section_size -= 24;

          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);  // Skip remaining bytes
        }
        else if (memcmp(section, "DLTA", 4) == 0) // Animation DeLTA
        {
          int i, plane;
          dword offsets[16];
          dword current_offset = 0;

          if (previous_frame == NULL)
          {
            real_line_size = (context->Width+15) & ~15;
            plane_line_size = real_line_size >> 3;  // 8bits per byte
            line_size = plane_line_size * real_bit_planes;

            previous_frame = calloc(line_size * context->Height,1);
            for (y_pos=0; y_pos<context->Height; y_pos++)
            {
              const byte * pix_p = Main.backups->Pages->Image[Main.current_layer].Pixels + y_pos * Main.backups->Pages->Width;
              // Dispatch the pixel into planes
              for (x_pos=0; x_pos<context->Width; x_pos++)
              {
                Set_IFF_color(previous_frame+y_pos*line_size, x_pos, *pix_p++, real_line_size, real_bit_planes);
              }
            }
          }

          Set_loading_layer(context, ++current_frame);
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
              Warning("too much bitplanes in DLTA data");
              break;
            }
            while (current_offset < offsets[plane])
            {
              Read_byte(IFF_file, &op_count);
              current_offset++;
            }
            if (current_offset > offsets[plane])
            {
              Warning("Loading ERROR in DLTA data");
              File_error = 2;
              break;
            }
            for (x_pos = 0; x_pos < (context->Width+7) >> 3; x_pos++)
            {
              byte * p = previous_frame + x_pos + plane * plane_line_size;
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
              if (y_pos > 0 && y_pos != context->Height)
              {
              }
            }
          }
          if (File_error == 0)
          {
            for (y_pos=0; y_pos<context->Height; y_pos++)
            {
              Draw_IFF_line(context, previous_frame+line_size*y_pos,y_pos,real_line_size,real_bit_planes);
            }
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
        else if (memcmp(section, "CMAP", 4) == 0)
        {
          nb_colors = section_size/3;

          if ((header.BitPlanes==6 && nb_colors==16) || (header.BitPlanes==8 && nb_colors==64))
          {
            Image_HAM=header.BitPlanes;
            bpp = 3 * (header.BitPlanes - 2); // HAM6 = 12bpp, HAM8 = 18bpp
          }

          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
          if (Read_bytes(IFF_file,context->Palette,3*nb_colors))
          {
            if (((nb_colors==32) || (AmigaViewModes & 0x80)) && (header.BitPlanes==6))
              IFF_Set_EHB_Palette(context); // This is a Extra Half-Brite (EHB) 64 color image.

            section_size -= 3*nb_colors;
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
            if (section_size == 8 && min_col != max_col)
            {
              // Valid cycling range
              if (max_col<min_col)
                SWAP_BYTES(min_col,max_col)

              if (context->Color_cycles >= 16)
              {
                Warning("Maximum CRNG number is 16");
              }
              else
              {
                context->Cycle_range[context->Color_cycles].Start=min_col;
                context->Cycle_range[context->Color_cycles].End=max_col;
                context->Cycle_range[context->Color_cycles].Inverse=(flags&2)?1:0;
                context->Cycle_range[context->Color_cycles].Speed=(flags&1) ? rate/78 : 0;

                context->Color_cycles++;
              }
            }
          }
          else
            File_error=47;
        }
        else if (memcmp(section, "CAMG", 4) == 0) //  	Amiga Viewport Modes
        {
          Read_dword_be(IFF_file, &AmigaViewModes); // HIRES=0x8000 LACE=0x4  HAM=0x800  HALFBRITE=0x80
          section_size -= 4;
          if (AmigaViewModes & 0x800)
          {
            Image_HAM = header.BitPlanes;
            bpp = 3 * (header.BitPlanes - 2);
          }
          if ((AmigaViewModes & 0x80) && (header.BitPlanes == 6)) // This is a Extra Half-Brite (EHB) 64 color image.
            IFF_Set_EHB_Palette(context); // Set the palette in case CAMG is after CMAP
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
            Warning("Extra bytes at the end of CLUT chunk");
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          }
        }
        else if (memcmp(section, "SHAM", 4) == 0) // Sliced HAM
        {
          word version;

          Image_HAM = header.BitPlanes;
          bpp = 3 * (header.BitPlanes - 2);
          Read_word_be(IFF_file, &version); // always 0
          section_size -= 2;
          SHAM_palette_count = section_size >> 5;  // 32 bytes per palette (16 colors * 2 bytes)
          // SHAM_palette_count should be the image height, or height/2 for "interlaced" images
          SHAM_palettes = malloc(sizeof(T_Components)*16*SHAM_palette_count);
          if (SHAM_palettes == NULL)
          {
            Warning("Memory allocation error");
            File_error = 1;
            break;
          }
          for (counter = 0; (unsigned)counter < 16*SHAM_palette_count; counter++)
          {
            Read_byte(IFF_file, &temp_byte);  // 0R
            SHAM_palettes[counter].R = (temp_byte & 0x0f) * 0x11; // 4 bits to 8 bits
            Read_byte(IFF_file, &temp_byte);  // GB
            SHAM_palettes[counter].G = (temp_byte & 0xf0) | (temp_byte >> 4);
            SHAM_palettes[counter].B = (temp_byte & 0x0f) * 0x11; // 4 bits to 8 bits
            section_size -= 2;
          }
          if (section_size > 0)
          {
            Warning("Extra bytes at the end of SHAM chunk");
            fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          }
        }
        else if (memcmp(section, "PCHG", 4) == 0)
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

            lineBitMask = PCHGData;
            #if SDL_BYTEORDER != SDL_BIG_ENDIAN
            for (i = 0 ; i < ((LineCount + 31) >> 5); i++)
              lineBitMask[i] = SDL_Swap32(lineBitMask[i]);
            #endif
            data = PCHGData + ((LineCount + 31) >> 5) * 4;
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
                prev_pal = curr_pal;
              }
            }
            free(PCHGData);
          }
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
          if (PCHG_palettes != NULL)
            bpp = 12;
        }
        else if (memcmp(section, "TINY", 4) == 0)
        {
          word tiny_width, tiny_height;
          Read_word_be(IFF_file,&tiny_width);
          Read_word_be(IFF_file,&tiny_height);
          section_size -= 4;
printf("%d x %d = %d   %d\n", tiny_width, tiny_height, tiny_width*tiny_height, section_size);

          // Load thumbnail if in preview mode
          if ((context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE) && iff_format == FORMAT_PBM)
          {
            Pre_load(context, tiny_width, tiny_height,file_size,iff_format,ratio,bpp);
            PBM_Decode(context, IFF_file, header.Compression, tiny_width, tiny_height);
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
                Warning("Unhandled compression for ACBM ABIT chunk");
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
                Draw_IFF_line_HAM(context, buffer+y_pos*line_size, y_pos,real_line_size, real_bit_planes, SHAM_palettes, SHAM_palette_count);
            }
          }
          free(buffer);
        }
        else if (memcmp(section, "BODY", 4) == 0)
        {
          Original_screen_X = header.X_screen;
          Original_screen_Y = header.Y_screen;

          Pre_load(context, header.Width, header.Height, file_size, iff_format, ratio, bpp);
          context->Background_transparent = header.Mask == 2;
          context->Transparent_color = context->Background_transparent ? header.Transp_col : 0;

          if (context->Type == CONTEXT_MAIN_IMAGE)
          {
            Main.backups->Pages->Image_mode = IMAGE_MODE_ANIMATION;
            Update_screen_targets();
          }

          if (!memcmp(format,"ILBM",4))    // "ILBM": InterLeaved BitMap
          {
            // compute row size
            real_line_size = (context->Width+15) & ~15;
            plane_line_size = real_line_size >> 3;  // 8bits per byte
            line_size = plane_line_size * stored_bit_planes;

            switch(header.Compression)
            {
              case 0:            // uncompressed
                buffer=(byte *)malloc(line_size);
                for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
                {
                  if (Read_bytes(IFF_file,buffer,line_size))
                  {
                    if (PCHG_palettes)
                      Draw_IFF_line_PCHG(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
                    else if (Image_HAM <= 1)
                      Draw_IFF_line(context, buffer, y_pos,real_line_size, real_bit_planes);
                    else
                      Draw_IFF_line_HAM(context, buffer, y_pos,real_line_size, real_bit_planes, SHAM_palettes, SHAM_palette_count);
                  }
                  else
                    File_error=21;
                }
                free(buffer);
                break;
              case 1:          // packbits compression (Amiga)
                buffer=(byte *)malloc(line_size);
                for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
                {
                  for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
                  {
                    if(Read_byte(IFF_file, &temp_byte)!=1)
                    {
                      File_error=22;
                      break;
                    }
                    // temp_byte > 127  => repeat (256-temp_byte) the next byte
                    // temp_byte <= 127 => copy (temp_byte + 1) bytes
                    if(temp_byte == 128) // 128 = NOP !
                    {
                      Warning("NOP in packbits stream");
                    }
                    else if (temp_byte>127)
                    {
                      if(Read_byte(IFF_file, &color)!=1)
                      {
                        File_error=23;
                        break;
                      }
                      b256=(short)(256-temp_byte);
                      for (counter=0; counter<=b256; counter++)
                      {
                        if (x_pos<line_size)
                          buffer[x_pos++]=color;
                        else
                        {
                          File_error=24;
                          break;
                        }
                      }
                    }
                    else
                      for (counter=0; counter<=(short)(temp_byte); counter++)
                      {
                        if (x_pos>=line_size || Read_byte(IFF_file, &(buffer[x_pos++]))!=1)
                        {
                          File_error=25;
                          break;
                        }
                      }
                  }
                  if (!File_error)
                  {
                    if (PCHG_palettes)
                      Draw_IFF_line_PCHG(context, buffer, y_pos,real_line_size, real_bit_planes, PCHG_palettes);
                    else if (Image_HAM <= 1)
                      Draw_IFF_line(context, buffer, y_pos,real_line_size,real_bit_planes);
                    else
                      Draw_IFF_line_HAM(context, buffer, y_pos,real_line_size, real_bit_planes, SHAM_palettes, SHAM_palette_count);
                  }
                }
                free(buffer);
                break;
              case 2:     // vertical RLE compression (Atari ST)
                buffer=(byte *)calloc(line_size*context->Height, 1);
                if (buffer == NULL)
                {
                  File_error=1;
                  Warning("Failed to allocate memory for IFF decoding");
                }
                plane_line_size = real_line_size >> 3;
                for (plane = 0; plane < stored_bit_planes && !File_error; plane++)
                {
                  word cmd_count;
                  word cmd;
                  signed char * commands;
                  word count;

                  y_pos = 0; x_pos = 0;
                  if (!IFF_Wait_for(IFF_file, "VDAT"))
                  {
                    if (plane == 0) // don't cancel loading if at least 1 bitplane has been loaded
                      File_error = 30;
                    break;
                  }
                  Read_dword_be(IFF_file,&section_size);
                  Read_word_be(IFF_file,&cmd_count);
                  cmd_count -= 2;
                  commands = (signed char *)malloc(cmd_count);
                  if (!Read_bytes(IFF_file,commands,cmd_count))
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
                        Read_word_be(IFF_file,&count);
                        section_size -= 2;
                      }
                      else
                        count = -commands[cmd];
                      while (count-- > 0 && x_pos < plane_line_size && section_size > 0)
                      {
                        Read_bytes(IFF_file,buffer+x_pos+y_pos*line_size+plane*plane_line_size,2);
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
                        Read_word_be(IFF_file,&count);
                        section_size -= 2;
                      }
                      else
                        count = (word)commands[cmd];
                      if (section_size == 0)
                        break;
                      Read_bytes(IFF_file,data,2);
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
                    Warning("Early end in VDAT chunk");
                  if (section_size > 0)
                    fseek(IFF_file, (section_size+1)&~1, SEEK_CUR); // skip bytes
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
                Warning("Unknown IFF compression");
                File_error = 32;
            }
          }
          else                               // "PBM ": Packed BitMap
          {
            PBM_Decode(context, IFF_file, header.Compression, context->Width, context->Height);
          }
          if (section_size & 1)
            fseek(IFF_file, 1, SEEK_CUR); // SKIP one byte
        }
        else
        {
          char tmp_msg[60];
          // skip section
          snprintf(tmp_msg, sizeof(tmp_msg), "Skip unknown section '%.4s' of %u bytes", section, section_size);
          Warning(tmp_msg);
          fseek(IFF_file, (section_size+1)&~1, SEEK_CUR);
        }
      }

                /*
                while (!File_error && is_anim)
                {
                  dword delta_size;
                  
                  // Just loaded the first image successfully : now keep reading
                  
                  // FORM + size(4)
                  if (!Read_bytes(IFF_file,section,4))
                    break;
                  Read_dword_be(IFF_file,&dummy);

                  // ILBM, hopefully
                  Read_bytes(IFF_file,format,4);
                  if (!IFF_Wait_for(IFF_file, "DLTA"))
                  {
                    File_error=1;
                    break;
                  }
                  Set_loading_layer(context, context->Current_layer+1);

                  Read_dword_be(IFF_file,&delta_size);
                  fseek(IFF_file, delta_size + (delta_size & 1), SEEK_CUR);
                }
                */
    }

    fclose(IFF_file);
    IFF_file = NULL;
  }
  else
    File_error=1;
  if (SHAM_palettes)
    free(SHAM_palettes);
  if (previous_frame)
    free(previous_frame);
  while (PCHG_palettes != NULL)
  {
    T_IFF_PCHG_Palette * next = PCHG_palettes->Next;
    free(PCHG_palettes);
    PCHG_palettes = next;
  }
}


// -- Sauver un fichier au format IFF ---------------------------------------

  byte IFF_color_list[129];
  word IFF_list_size;
  byte IFF_repetition_mode;

  // ------------- Ecrire les couleurs que l'on vient de traiter ------------
  void Transfer_colors(FILE * file)
  {
    byte index;

    if (IFF_list_size>0)
    {
      if (IFF_repetition_mode)
      {
        Write_one_byte(file,257-IFF_list_size);
        Write_one_byte(file,IFF_color_list[0]);
      }
      else
      {
        Write_one_byte(file,IFF_list_size-1);
        for (index=0; index<IFF_list_size; index++)
          Write_one_byte(file,IFF_color_list[index]);
      }
    }
    IFF_list_size=0;
    IFF_repetition_mode=0;
  }

  // - Compresion des couleurs encore plus performante que DP2e et que VPIC -
  void New_color(FILE * file, byte color)
  {
    byte last_color;
    byte second_last_color;

    switch (IFF_list_size)
    {
      case 0 : // Première couleur
        IFF_color_list[0]=color;
        IFF_list_size=1;
        break;
      case 1 : // Deuxième couleur
        last_color=IFF_color_list[0];
        IFF_repetition_mode=(last_color==color);
        IFF_color_list[1]=color;
        IFF_list_size=2;
        break;
      default: // Couleurs suivantes
        last_color      =IFF_color_list[IFF_list_size-1];
        second_last_color=IFF_color_list[IFF_list_size-2];
        if (last_color==color)  // On a une répétition de couleur
        {
          if ( (IFF_repetition_mode) || (second_last_color!=color) )
          // On conserve le mode...
          {
            IFF_color_list[IFF_list_size]=color;
            IFF_list_size++;
            if (IFF_list_size==128)
              Transfer_colors(file);
          }
          else // On est en mode <> et on a 3 couleurs qui se suivent
          {
            IFF_list_size-=2;
            Transfer_colors(file);
            IFF_color_list[0]=color;
            IFF_color_list[1]=color;
            IFF_color_list[2]=color;
            IFF_list_size=3;
            IFF_repetition_mode=1;
          }
        }
        else // La couleur n'est pas la même que la précédente
        {
          if (!IFF_repetition_mode)                 // On conserve le mode...
          {
            IFF_color_list[IFF_list_size++]=color;
            if (IFF_list_size==128)
              Transfer_colors(file);
          }
          else                                        // On change de mode...
          {
            Transfer_colors(file);
            IFF_color_list[IFF_list_size]=color;
            IFF_list_size++;
          }
        }
    }
  }


void Save_IFF(T_IO_Context * context)
{
  FILE * IFF_file;
  char filename[MAX_PATH_CHARACTERS];
  T_IFF_Header header;
  word x_pos;
  word y_pos;
  byte temp_byte;
  int i;
  int palette_entries;
  byte bit_depth;
  long body_offset = -1;

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
  }
  else // FORMAT_PBM
  {
    bit_depth=8;
  }
  palette_entries = 1<<bit_depth;

  File_error=0;
  
  Get_full_filename(filename, context->File_name, context->File_directory);

  // Ouverture du fichier
  if ((IFF_file=fopen(filename,"wb")))
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
      
      // Calcul de la taille d'une ligne ILBM (pour les images ayant des dimensions exotiques)
      real_line_size = (context->Width+15) & ~15;
      plane_line_size = real_line_size >> 3;  // 8bits per byte
      line_size = plane_line_size * header.BitPlanes;
      buffer=(byte *)malloc(line_size);
      
      // Start encoding
      IFF_list_size=0;
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
              New_color(IFF_file, buffer[x_pos+plane*plane_width]);

            if (!File_error)
              Transfer_colors(IFF_file);
          }
        }
        else
        {
          Write_bytes(IFF_file,buffer,line_size);
        }
      }
      free(buffer);
    }
    else // PBM = chunky 8bpp
    {
      IFF_list_size=0;
  
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
      {
        for (x_pos=0; ((x_pos<context->Width) && (!File_error)); x_pos++)
          New_color(IFF_file, Get_pixel(context, x_pos,y_pos));
  
        if (context->Width&1) // odd width fix
          New_color(IFF_file, 0);
          
        if (!File_error)
          Transfer_colors(IFF_file);
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
    IFF_file = NULL;
    if (File_error != 0)  // delete file if any error during saving
      remove(filename);
  }
  else
    File_error=1;
}



//////////////////////////////////// BMP ////////////////////////////////////
typedef struct
{
    byte  Signature[2];   // ='BM' = 0x4D42
    dword Size_1;    // file size
    word  Reserved_1; // 0
    word  Reserved_2; // 0
    dword Offset; // Offset of bitmap data start

    dword Size_2; // 40
    dword Width;
    int32_t Height; // signed: negative means a top-down bitmap (rare)
    word  Planes; // 1
    word  Nb_bits; // 1,4,8 ou 24
    dword Compression;
    dword Size_3;
    dword XPM;
    dword YPM;
    dword Nb_Clr;
    dword Clr_Imprt;
} T_BMP_Header;

// -- Tester si un fichier est au format BMP --------------------------------
void Test_BMP(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;

  File_error=1;
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename, "rb")))
  {
    if (Read_bytes(file,&(header.Signature),2) // "BM"
     && Read_dword_le(file,&(header.Size_1))
     && Read_word_le(file,&(header.Reserved_1))
     && Read_word_le(file,&(header.Reserved_2))
     && Read_dword_le(file,&(header.Offset))
     && Read_dword_le(file,&(header.Size_2))
     && Read_dword_le(file,&(header.Width))
     && Read_dword_le(file,(dword *)&(header.Height))
     && Read_word_le(file,&(header.Planes))
     && Read_word_le(file,&(header.Nb_bits))
     && Read_dword_le(file,&(header.Compression))
     && Read_dword_le(file,&(header.Size_3))
     && Read_dword_le(file,&(header.XPM))
     && Read_dword_le(file,&(header.YPM))
     && Read_dword_le(file,&(header.Nb_Clr))
     && Read_dword_le(file,&(header.Clr_Imprt))
        )
     {

      if ( header.Signature[0]=='B' && header.Signature[1]=='M'
        && (header.Size_2==40 /* WINDOWS */ || header.Size_2==12 /* OS/2 */)
        && header.Width && header.Height )
      {
        File_error=0;
      }
     }
    fclose(file);
  }
}

// Find the 8 important bits in a dword
static byte Bitmap_mask(dword pixel, dword mask)
{
  byte result;
  int i;
  int bits_found;

  switch(mask)
  {
    // Shortcuts to quickly handle the common 24/32bit cases
    case 0x000000FF:
      return (pixel & 0x000000FF);
    case 0x0000FF00:
      return (pixel & 0x0000FF00)>>8;
    case 0x00FF0000:
      return (pixel & 0x00FF0000)>>16;
    case 0xFF000000:
      return (pixel & 0xFF000000)>>24;
  }
  // Uncommon : do it bit by bit.
  bits_found=0;
  result=0;
  // Process the mask from low to high bit
  for (i=0;i<32;i++)
  {
    // Found a bit in the mask
    if (mask & (1<<i))
    {
      if (pixel & 1<<i)
        result |= 1<<bits_found;
        
      bits_found++;
      
      if (bits_found>=8)
        return result;
    }
  }
  // Less than 8 bits in the mask: scale the result to 8 bits
  return result << (8-bits_found);
}

static void Load_BMP_Palette(T_IO_Context * context, FILE * file, unsigned int nb_colors, int is_rgb24)
{
  byte  local_palette[256*4]; // R,G,B,0 or RGB
  unsigned int i, j;

  if (Read_bytes(file,local_palette,nb_colors*(is_rgb24?3:4)))
  {
    if (Config.Clear_palette)
      memset(context->Palette,0,sizeof(T_Palette));

    // We can now load the new palette
    for (i=0, j=0; i<nb_colors; i++)
    {
      context->Palette[i].B=local_palette[j++];
      context->Palette[i].G=local_palette[j++];
      context->Palette[i].R=local_palette[j++];
      if (!is_rgb24) j++;
    }
  }
  else
  {
    File_error=1;
  }
}

// rows are stored from the top to the bottom (standard for BMP is from bottom to the top)
#define LOAD_BMP_PIXEL_FLAG_TOP_DOWN     0x01
// We are decoding the AND-mask plane (transparency) of a .ICO file
#define LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE 0x02

static void Load_BMP_Pixels(T_IO_Context * context, FILE * file, unsigned int compression, unsigned int nbbits, int flags, const dword * mask)
{
  unsigned int row_size;
  unsigned int index;
  short x_pos;
  short y_pos;
  byte * buffer;
  byte value;
  byte a,b,c=0;

  switch (compression)
  {
    case 0 : // Pas de compression
    case 3 :
      row_size = ((nbbits*context->Width + 31) >> 3) & ~3;
      buffer = malloc(row_size);
      for (y_pos=0; (y_pos < context->Height && !File_error); y_pos++)
      {
        short target_y;
        target_y = (flags & LOAD_BMP_PIXEL_FLAG_TOP_DOWN) ? y_pos : context->Height-1-y_pos;

        if (Read_bytes(file,buffer,row_size))
        {
          for (x_pos=0; x_pos<context->Width; x_pos++)
          {
            switch (nbbits)
            {
              case 8 :
                value = buffer[x_pos];
                Set_pixel(context, x_pos, target_y, value);
                break;
              case 4 :
                value = (x_pos & 1) ? (buffer[x_pos>>1] & 0xF) : (buffer[x_pos>>1] >> 4);
                Set_pixel(context, x_pos, target_y, value);
                break;
              case 2:
                value = (buffer[x_pos>>2] >> (((x_pos&3)^3)*2)) & 3;
                Set_pixel(context, x_pos, target_y, value);
                break;
              case 1 :
                value = ( buffer[x_pos>>3] & (0x80>>(x_pos&7)) ) ? 1 : 0;
                if (flags & LOAD_BMP_PIXEL_FLAG_TRANSP_PLANE)
                {
                  if (value) // transparent pixel !
                    Set_pixel(context, x_pos, target_y, context->Transparent_color);
                }
                else
                  Set_pixel(context, x_pos, target_y, value);
                break;
              case 24:
                Set_pixel_24b(context, x_pos,target_y,buffer[x_pos*3+2],buffer[x_pos*3+1],buffer[x_pos*3+0]);
                break;
              case 32:
                {
                #if SDL_BYTEORDER != SDL_LIL_ENDIAN
                  dword pixel = SDL_Swap32(((dword *)buffer)[x_pos]);
                #else
                  dword pixel = ((dword *)buffer)[x_pos];
                #endif
                  Set_pixel_24b(context, x_pos,target_y,Bitmap_mask(pixel,mask[0]),Bitmap_mask(pixel,mask[1]),Bitmap_mask(pixel,mask[2]));
                }
                break;
              case 16:
                {
                #if SDL_BYTEORDER != SDL_LIL_ENDIAN
                  word pixel = SDL_Swap16(((word *)buffer)[x_pos]);
                #else
                  word pixel = ((word *)buffer)[x_pos];
                #endif
                  Set_pixel_24b(context, x_pos,target_y,Bitmap_mask(pixel,mask[0]),Bitmap_mask(pixel,mask[1]),Bitmap_mask(pixel,mask[2]));
                }
                break;
              default:
                value = 0;
            }
          }
        }
        else
        {
          File_error=2;
        }
      }
      free(buffer);
      buffer = NULL;
      break;

    case 1 : // Compression RLE 8 bits
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

    case 2 : // Compression RLE 4 bits
      x_pos=0;
      y_pos=context->Height-1;

      if(Read_byte(file, &a)!=1 ||  Read_byte(file, &b) != 1)
        File_error =2;
      while ( (!File_error) && ((a)||(b!=1)) )
      {
        if (a) // Encoded mode (A fois les 1/2 pixels de B)
          for (index=1; index<=a; index++)
          {
            if (index & 1)
              Set_pixel(context, x_pos,y_pos,b>>4);
            else
              Set_pixel(context, x_pos,y_pos,b&0xF);
            x_pos++;
          }
        else   // Absolute mode
          switch (b)
          {
            case 0 : //End of line
              x_pos=0;
              y_pos--;
              break;
            case 1 : // End of bitmap
              break;
            case 2 : // Delta
              if(Read_byte(file, &a)!=1 ||  Read_byte(file, &b)!=1)
                File_error=2;
              x_pos+=a;
              y_pos-=b;
              break;
            default: // Nouvelle série (B 1/2 pixels bruts)
              for (index=1; ((index<=b) && (!File_error)); index++,x_pos++)
              {
                if (index&1)
                {
                  if(Read_byte(file, &c)!=1) File_error=2;
                  Set_pixel(context, x_pos,y_pos,c>>4);
                }
                else
                  Set_pixel(context, x_pos,y_pos,c&0xF);
              }
              //   On lit l'octet rendant le nombre d'octets pair, si
              // nécessaire. Encore un truc de crétin "made in MS".
              if ( ((b&3)==1) || ((b&3)==2) )
              {
                byte dummy;
                if(Read_byte(file, &dummy)!=1) File_error=2;
              }
          }
        if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1) File_error=2;
      }
      break;
    default:
      Warning("Unknown compression type");
  }
}

// -- Charger un fichier au format BMP --------------------------------------
void Load_BMP(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;
  word  nb_colors =  0;
  long  file_size;
  byte  negative_height; // top_down
  byte  true_color = 0;
  dword mask[3];  // R G B

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

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
      if (header.Size_2 == 40 /* WINDOWS */)
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
        )) File_error = 1;
      }
      else if (header.Size_2 == 12 /* OS/2 */)
      {
        word tmp_width = 0, tmp_height = 0;
        if (Read_word_le(file,&tmp_width)
         && Read_word_le(file,&tmp_height)
         && Read_word_le(file,&(header.Planes))
         && Read_word_le(file,&(header.Nb_bits)))
        {
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
        File_error = 2;
      }
    }
    if (File_error == 0)
    {
      switch (header.Nb_bits)
      {
        case 1 :
        case 2 :
        case 4 :
        case 8 :
          if (header.Nb_Clr)
            nb_colors=header.Nb_Clr;
          else
            nb_colors=1<<header.Nb_bits;
          break;
        case 16:
        case 24:
        case 32:
          true_color = 1;
          break;
        default:
          Warning("Unsupported bit per pixel");
          File_error = 1;
      }
      
      if (header.Height < 0)
      {
        negative_height=1;
        header.Height = -header.Height;
      }
      else
      {
        negative_height=0;
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
      if (File_error == 0)
      {
        Pre_load(context, header.Width,header.Height,file_size,FORMAT_BMP,PIXEL_SIMPLE,header.Nb_bits);
        if (File_error==0)
        {
          if (true_color)
          {
            if (header.Compression == 3) // BI_BITFIELDS
            {
              if (!Read_dword_le(file,&mask[0]) ||
                  !Read_dword_le(file,&mask[1]) ||
                  !Read_dword_le(file,&mask[2]))
                File_error=2;
            }
          }
          else
          {
            Load_BMP_Palette(context, file, nb_colors, header.Size_2 == 12);
          }

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
    else
    {
      File_error=1;
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Sauvegarder un fichier au format BMP ----------------------------------
void Save_BMP(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;
  short x_pos;
  short y_pos;
  long line_size;
  word index;
  byte local_palette[256][4]; // R,G,B,0


  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
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
          remove(filename);
      }
      else
      {
        fclose(file);
        remove(filename);
        File_error=1;
      }

    }
    else
    {
      fclose(file);
      remove(filename);
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

void Test_ICO(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  struct {
    word Reserved;
    word Type;  // Specifies image type: 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
    word Count; // Specifies number of images in the file.
  } header;

  File_error=1;
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename, "rb")))
  {
    if (Read_word_le(file,&(header.Reserved))
     && Read_word_le(file,&(header.Type))
     && Read_word_le(file,&(header.Count)))
    {
      if (header.Reserved == 0 && (header.Type == 1 || header.Type == 2) && header.Count > 0)
        File_error=0;
    }
    fclose(file);
  }
}

void Load_ICO(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
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
  dword mask[3];  // R G B

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename, "rb")))
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
      }
      // select the picture with the maximum width and 256 colors or less
      //printf("max width = %d\n", max_width);
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
            Load_PNG_Sub(context, file);
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
  char filename[MAX_PATH_CHARACTERS];
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
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename,"wb")) == NULL)
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
      remove(filename);
  }
}


//////////////////////////////////// GIF ////////////////////////////////////
typedef struct
{
  word Width;   // Width of the complete image area
  word Height;  // Height of the complete image area
  byte Resol;   // Informations about the resolution (and other)
  byte Backcol; // Proposed background color
  byte Aspect;  // Informations about aspect ratio (and other)
} T_GIF_LSDB;   // Logical Screen Descriptor Block

typedef struct
{
  word Pos_X;         // X offset where the image should be pasted
  word Pos_Y;         // Y offset where the image should be pasted
  word Image_width;   // Width of image
  word Image_height;  // Height of image
  byte Indicator;     // Misc image information
  byte Nb_bits_pixel; // Nb de bits par pixel
} T_GIF_IDB;          // Image Descriptor Block

typedef struct
{
  byte Block_identifier;  // 0x21
  byte Function;          // 0xF9
  byte Block_size;        // 4
  byte Packed_fields;     // 11100000 : Reserved
                          // 00011100 : Disposal method
                          // 00000010 : User input flag
                          // 00000001 : Transparent flag
  word Delay_time;        // Time for this frame to stay displayed
  byte Transparent_color; // Which color index acts as transparent
  word Block_terminator;  // 0x00
} T_GIF_GCE;              // Graphic Control Extension

enum DISPOSAL_METHOD
{
  DISPOSAL_METHOD_UNDEFINED = 0,
  DISPOSAL_METHOD_DO_NOT_DISPOSE = 1,
  DISPOSAL_METHOD_RESTORE_BGCOLOR = 2,
  DISPOSAL_METHOD_RESTORE_PREVIOUS = 3,
};

// -- Tester si un fichier est au format GIF --------------------------------

void Test_GIF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  char signature[6];
  FILE *file;


  File_error=1;
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename, "rb")))
  {
    if (
        (Read_bytes(file,signature,6)) &&
        ((!memcmp(signature,"GIF87a",6))||(!memcmp(signature,"GIF89a",6)))
       )
      File_error=0;

    fclose(file);
  }
}


// -- Lire un fichier au format GIF -----------------------------------------
// -- Lire un fichier au format GIF -----------------------------------------

// Définition de quelques variables globales au chargement du GIF87a
word GIF_nb_bits;        // Nb de bits composants un code complet
word GIF_remainder_bits;      // Nb de bits encore dispos dans GIF_last_byte
byte GIF_remainder_byte;      // Nb d'octets avant le prochain bloc de Raster Data
word GIF_current_code;    // Code traité (qui vient d'être lu en général)
byte GIF_last_byte;      // Octet de lecture des bits
word GIF_pos_X;          // Coordonnées d'affichage de l'image
word GIF_pos_Y;
word GIF_interlaced;     // L'image est entrelacée
word GIF_finished_interlaced_image; // L'image entrelacée est finie de charger
word GIF_pass;          // index de passe de l'image entrelacée
FILE *GIF_file;        // L'handle du fichier

// -- Lit le code à GIF_nb_bits suivant --

word GIF_get_next_code(void)
{
  word nb_bits_to_process=GIF_nb_bits;
  word nb_bits_processed  =0;
  word current_nb_bits;

  GIF_current_code=0;

  while (nb_bits_to_process)
  {
    if (GIF_remainder_bits==0) // Il ne reste plus de bits...
    {
      // Lire l'octet suivant:

      // Si on a atteint la fin du bloc de Raster Data
      if (GIF_remainder_byte==0)
        // Lire l'octet nous donnant la taille du bloc de Raster Data suivant
        if(Read_byte(GIF_file, &GIF_remainder_byte)!=1)
          File_error=2;

      if(Read_byte(GIF_file,&GIF_last_byte)!=1)
        File_error = 2;
      GIF_remainder_byte--;
      GIF_remainder_bits=8;
    }

    current_nb_bits=(nb_bits_to_process<=GIF_remainder_bits)?nb_bits_to_process:GIF_remainder_bits;

    GIF_current_code|=(GIF_last_byte & ((1<<current_nb_bits)-1))<<nb_bits_processed;
    GIF_last_byte>>=current_nb_bits;
    nb_bits_processed  +=current_nb_bits;
    nb_bits_to_process-=current_nb_bits;
    GIF_remainder_bits    -=current_nb_bits;
  }

  return GIF_current_code;
}

// -- Affiche un nouveau pixel --

void GIF_new_pixel(T_IO_Context * context, T_GIF_IDB *idb, int is_transparent, byte color)
{
  if (!is_transparent || color!=context->Transparent_color)
  Set_pixel(context, idb->Pos_X+GIF_pos_X, idb->Pos_Y+GIF_pos_Y,color);

  GIF_pos_X++;

  if (GIF_pos_X>=idb->Image_width)
  {
    GIF_pos_X=0;

    if (!GIF_interlaced)
      GIF_pos_Y++;
    else
    {
      switch (GIF_pass)
      {
        case 0 : GIF_pos_Y+=8;
                 break;
        case 1 : GIF_pos_Y+=8;
                 break;
        case 2 : GIF_pos_Y+=4;
                 break;
        default: GIF_pos_Y+=2;
      }

      if (GIF_pos_Y>=idb->Image_height)
      {
        switch(++GIF_pass)
        {
        case 1 : GIF_pos_Y=4;
                 break;
        case 2 : GIF_pos_Y=2;
                 break;
        case 3 : GIF_pos_Y=1;
                 break;
        case 4 : GIF_finished_interlaced_image=1;
        }
      }
    }
  }
}


void Load_GIF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  char signature[6];

  word * alphabet_stack;     // Pile de décodage d'une chaîne
  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   alphabet_stack_pos; // Position dans la pile de décodage d'un chaîne

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
  
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((GIF_file=fopen(filename, "rb")))
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

        if (LSDB.Aspect==17)
           ratio=PIXEL_TALL;
        else if (LSDB.Aspect==113)
           ratio=PIXEL_WIDE;
        else
           ratio=PIXEL_SIMPLE;

        Pre_load(context, LSDB.Width,LSDB.Height,file_size,FORMAT_GIF,ratio,(LSDB.Resol&7)+1);
        context->Width=LSDB.Width;
        context->Height=LSDB.Height;

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
                      int nb_char_to_keep=Min(size_to_read,COMMENT_SIZE);
                      
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
                      if (File_error)
                        ;
                      else if (!memcmp(aeb,"NETSCAPE2.0",0x0B))
                      {
                        is_looping=1;
                        // The well-known Netscape extension.
                        // Load as an animation
                        if (context->Type == CONTEXT_MAIN_IMAGE)
                        {
                          Main.backups->Pages->Image_mode = IMAGE_MODE_ANIMATION;
                          Update_screen_targets();
                        }
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
                        if (context->Original_file_name && context->Original_file_directory)
                        {
                          Read_byte(GIF_file,&size_to_read);
                          if (!File_error && size_to_read)
                          {
                            Read_bytes(GIF_file,context->Original_file_directory, size_to_read);
                            Read_byte(GIF_file,&size_to_read);
                            if (!File_error && size_to_read)
                            {
                              Read_bytes(GIF_file,context->Original_file_name, size_to_read);
                              Read_byte(GIF_file,&size_to_read); // Normally 0
                            }
                          }
                        }
                        else
                        {
                          // Nothing to do, just skip sub-block
                          Read_byte(GIF_file,&size_to_read);
                          while (size_to_read!=0 && !File_error)
                          {
                            fseek(GIF_file,size_to_read,SEEK_CUR);
                            Read_byte(GIF_file,&size_to_read);
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
                      else
                      {
                        // Unknown extension, skip.
                        Read_byte(GIF_file,&size_to_read);
                        while (size_to_read!=0 && !File_error)
                        {
                          fseek(GIF_file,size_to_read,SEEK_CUR);
                          Read_byte(GIF_file,&size_to_read);
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

                GIF_nb_bits  =initial_nb_bits + 1;
                alphabet_max      =((1 <<  GIF_nb_bits)-1);
                GIF_interlaced    =(IDB.Indicator & 0x40);
                GIF_pass         =0;
    
                /*Init_lecture();*/
    

                GIF_finished_interlaced_image=0;
    
                //////////////////////////////////////////// DECOMPRESSION LZW //
    
                GIF_pos_X=0;
                GIF_pos_Y=0;
                alphabet_stack_pos=0;
                GIF_last_byte    =0;
                GIF_remainder_bits    =0;
                GIF_remainder_byte    =0;

                while ( (GIF_get_next_code()!=value_eof) && (!File_error) )
                {
                  if (GIF_current_code<=alphabet_free)
                  {
                    if (GIF_current_code!=value_clr)
                    {
                      if (alphabet_free==(byte_read=GIF_current_code))
                      {
                        GIF_current_code=old_code;
                        alphabet_stack[alphabet_stack_pos++]=special_case;
                      }
    
                      while (GIF_current_code>value_clr)
                      {
                        alphabet_stack[alphabet_stack_pos++]=alphabet_suffix[GIF_current_code];
                        GIF_current_code=alphabet_prefix[GIF_current_code];
                      }
    
                      special_case=alphabet_stack[alphabet_stack_pos++]=GIF_current_code;
    
                      do
                        GIF_new_pixel(context, &IDB, is_transparent, alphabet_stack[--alphabet_stack_pos]);
                      while (alphabet_stack_pos!=0);
    
                      alphabet_prefix[alphabet_free  ]=old_code;
                      alphabet_suffix[alphabet_free++]=GIF_current_code;
                      old_code=byte_read;
    
                      if (alphabet_free>alphabet_max)
                      {
                        if (GIF_nb_bits<12)
                          alphabet_max      =((1 << (++GIF_nb_bits))-1);
                      }
                    }
                    else // Code Clear rencontré
                    {
                      GIF_nb_bits       =initial_nb_bits + 1;
                      alphabet_max      =((1 <<  GIF_nb_bits)-1);
                      alphabet_free     =(1<<initial_nb_bits)+2;
                      special_case       =GIF_get_next_code();
                      old_code       =GIF_current_code;
                      GIF_new_pixel(context, &IDB, is_transparent, GIF_current_code);
                    }
                  }
                  else
                  {
                    File_error=2;
                    break;
                  }
                } // Code End-Of-Information ou erreur de fichier rencontré
                if (File_error==2 && GIF_pos_X==0 && GIF_pos_Y==IDB.Image_height)
                  File_error=0;
                /*Close_lecture();*/
    
                if (File_error>=0)
                if ( /* (GIF_pos_X!=0) || */
                     ( ( (!GIF_interlaced) && (GIF_pos_Y!=IDB.Image_height) && (GIF_pos_X!=0)) ||
                       (  (GIF_interlaced) && (!GIF_finished_interlaced_image) )
                     ) )
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

  int  GIF_stop;         // "On peut arrêter la sauvegarde du fichier"
  byte GIF_buffer[256];   // buffer d'écriture de bloc de données compilées

  // -- Vider le buffer GIF dans le buffer KM --

  void GIF_empty_buffer(void)
  {
    word index;

    if (GIF_remainder_byte)
    {
      GIF_buffer[0]=GIF_remainder_byte;

      for (index=0;index<=GIF_remainder_byte;index++)
        Write_one_byte(GIF_file,GIF_buffer[index]);

      GIF_remainder_byte=0;
    }
  }

  // -- Ecrit un code à GIF_nb_bits --

  void GIF_set_code(word Code)
  {
    word nb_bits_to_process=GIF_nb_bits;
    word nb_bits_processed  =0;
    word current_nb_bits;

    while (nb_bits_to_process)
    {
      current_nb_bits=(nb_bits_to_process<=(8-GIF_remainder_bits))?nb_bits_to_process:(8-GIF_remainder_bits);

      GIF_last_byte|=(Code & ((1<<current_nb_bits)-1))<<GIF_remainder_bits;
      Code>>=current_nb_bits;
      GIF_remainder_bits    +=current_nb_bits;
      nb_bits_processed  +=current_nb_bits;
      nb_bits_to_process-=current_nb_bits;

      if (GIF_remainder_bits==8) // Il ne reste plus de bits à coder sur l'octet courant
      {
        // Ecrire l'octet à balancer:
        GIF_buffer[++GIF_remainder_byte]=GIF_last_byte;

        // Si on a atteint la fin du bloc de Raster Data
        if (GIF_remainder_byte==255)
          // On doit vider le buffer qui est maintenant plein
          GIF_empty_buffer();

        GIF_last_byte=0;
        GIF_remainder_bits=0;
      }
    }
  }


  // -- Lire le pixel suivant --

  byte GIF_next_pixel(T_IO_Context *context, T_GIF_IDB *idb)
  {
    byte temp;

    temp=Get_pixel(context, GIF_pos_X,GIF_pos_Y);

    if (++GIF_pos_X >= (idb->Image_width + idb->Pos_X))
    {
      GIF_pos_X = idb->Pos_X;
      if (++GIF_pos_Y >= (idb->Image_height + idb->Pos_Y))
        GIF_stop=1;
    }

    return temp;
  }



void Save_GIF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];

  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word * alphabet_daughter;    // Table des chaînes filles (plus longues)
  word * alphabet_sister;    // Table des chaînes soeurs (même longueur)
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   start;            // Code précédent (sert au linkage des chaînes)
  int    descend;          // Booléen "On vient de descendre"

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
  
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((GIF_file=fopen(filename,"wb")))
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
          LSDB.Aspect = 17; // 1:2 
          break;
        case PIXEL_WIDE:
          LSDB.Aspect = 113; // 2:1
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

          // Ecriture de la transparence
          //Write_bytes(GIF_file,"\x21\xF9\x04\x01\x00\x00\xNN\x00",8);

          // "Netscape" animation extension
          //  Write_bytes(GIF_file,"\x21\xFF\x0BNETSCAPE2.0\x03\xLL\xSS\xSS\x00",19);
          // LL : 01 to loop
          // SSSS : number of loops
          if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
            if (context->Nb_layers>1)
              Write_bytes(GIF_file,"\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00",19);
            
          // Ecriture du commentaire
          if (context->Comment[0])
          {
            Write_bytes(GIF_file,"\x21\xFE",2);
            Write_byte(GIF_file,strlen(context->Comment));
            Write_bytes(GIF_file,context->Comment,strlen(context->Comment)+1);
          }
          // Write cycling colors
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
                temp = LSDB.Backcol;//=context->Transparent_color;
                for(GIF_pos_Y = 0; GIF_pos_Y < context->Height; GIF_pos_Y++) {
                  for(GIF_pos_X = 0; GIF_pos_X < context->Width; GIF_pos_X++) {
                    // compare Pixel from previous layer or from background depending on disposal method
                    if(disposal_method == DISPOSAL_METHOD_DO_NOT_DISPOSE)
                      temp = Main.backups->Pages->Image[current_layer - 1].Pixels[GIF_pos_Y * context->Pitch + GIF_pos_X];
                    if(temp != Get_pixel(context, GIF_pos_X, GIF_pos_Y)) {
                      if(GIF_pos_X < min_X) min_X = GIF_pos_X;
                      if(GIF_pos_X > max_X) max_X = GIF_pos_X;
                      if(GIF_pos_Y < min_Y) min_Y = GIF_pos_Y;
                      if(GIF_pos_Y > max_Y) max_Y = GIF_pos_Y;
                    }
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
              for(GIF_pos_Y = IDB.Pos_Y; GIF_pos_Y < IDB.Image_height + IDB.Pos_Y; GIF_pos_Y++) {
                for(GIF_pos_X = IDB.Pos_X; GIF_pos_X < IDB.Image_width + IDB.Pos_X; GIF_pos_X++) {
                  temp=Get_pixel(context, GIF_pos_X, GIF_pos_Y);
                  if(temp > max) max = temp;
                }
              }
              IDB.Nb_bits_pixel=2;  // Find the minimum bpp value to fit all pixels
              while((int)max >= (1 << IDB.Nb_bits_pixel)) {
                IDB.Nb_bits_pixel++;
              }
            
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
    
                GIF_pos_X=IDB.Pos_X;
                GIF_pos_Y=IDB.Pos_Y;
                GIF_last_byte=0;
                GIF_remainder_bits=0;
                GIF_remainder_byte=0;
    
#define GIF_INVALID_CODE (65535)
                index=GIF_INVALID_CODE;
                File_error=0;
                GIF_stop=0;
    
                // Réintialisation de la table:
                alphabet_free=clear + 2;  // 258 for 8bpp
                GIF_nb_bits  =IDB.Nb_bits_pixel + 1; // 9 for 8 bpp
                alphabet_max =clear+clear-1;  // 511 for 8bpp
                GIF_set_code(clear);  //256 for 8bpp
                for (start=0;start<4096;start++)
                {
                  alphabet_daughter[start] = GIF_INVALID_CODE;
                  alphabet_sister[start] = GIF_INVALID_CODE;
                }
    
                ////////////////////////////////////////////// COMPRESSION LZW //
    
                start=current_string=GIF_next_pixel(context, &IDB);
                descend=1;
    
                while ((!GIF_stop) && (!File_error))
                {
                  current_char=GIF_next_pixel(context, &IDB);
    
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
                    GIF_set_code(current_string);

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
                      GIF_set_code(clear);    // 256 for 8bpp
                      alphabet_free=clear+2;  // 258 for 8bpp
                      GIF_nb_bits  =IDB.Nb_bits_pixel + 1;  // 9 for 8bpp
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
    
                      GIF_nb_bits++;
                      alphabet_max=(1<<GIF_nb_bits)-1;
                    }
    
                    // initialize current_string as the string "current_char"
                    index=alphabet_daughter[current_char];
                    start=current_string=current_char;
                    descend=1;
                  }
                }
    
                if (!File_error)
                {
                  // On écrit le code dans le fichier
                  GIF_set_code(current_string); // Dernière portion d'image
    
                  //   Cette dernière portion ne devrait pas poser de problèmes
                  // du côté GIF_nb_bits puisque pour que GIF_nb_bits change de
                  // valeur, il faudrait que la table de chaîne soit remplie or
                  // c'est impossible puisqu'on traite une chaîne qui se trouve
                  // déjà dans la table, et qu'elle n'a rien d'inédit. Donc on
                  // ne devrait pas avoir à changer de taille, mais je laisse
                  // quand même en remarque tout ça, au cas où il subsisterait
                  // des problèmes dans certains cas exceptionnels.
                  //
                  // Note: de toutes façons, ces lignes en commentaires ont étés
                  //      écrites par copier/coller du temps où la sauvegarde du
                  //      GIF déconnait. Il y a donc fort à parier qu'elles ne
                  //      sont pas correctes.
    
                  /*
                  if (current_string==alphabet_max)
                  {
                    if (alphabet_max==0xFFF)
                    {
                      // On balargue un Clear Code
                      GIF_set_code(256);
    
                      // On réinitialise les données LZW
                      alphabet_free=258;
                      GIF_nb_bits  =9;
                      alphabet_max =511;
                    }
                    else
                    {
                      GIF_nb_bits++;
                      alphabet_max=(1<<GIF_nb_bits)-1;
                    }
                  }
                  */
    
                  GIF_set_code(eof);  // 257 for 8bpp    // Code de End d'image
                  if (GIF_remainder_bits!=0)
                  {
                    // Write last byte (this is an incomplete byte)
                    GIF_buffer[++GIF_remainder_byte]=GIF_last_byte;
                    GIF_last_byte=0;
                    GIF_remainder_bits=0;
                  }
                  GIF_empty_buffer();         // On envoie les dernières données du buffer GIF dans le buffer KM
    
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
            // If requested, write a specific extension for storing
            // original file path.
            // This is used by the backup system.
            // The format is :
            //   21 FF 0B G  F  X  2  P  A  T  H  00 00 00
            //   <size of path (byte)> <null-terminated path>
            //   <size of filename (byte)> <null-terminated filename>
            //   00
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
      remove(filename);

  } // On a pu ouvrir le fichier en écriture
  else
    File_error=1;

}



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

void Test_PCX(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);

  if ((file=fopen(filename, "rb")))
  {
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

    fclose(file);
  }
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
  char filename[MAX_PATH_CHARACTERS];
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

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((file=fopen(filename, "rb")))
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
      
      context->Width=PCX_header.X_max-PCX_header.X_min+1;
      context->Height=PCX_header.Y_max-PCX_header.Y_min+1;

      Original_screen_X=PCX_header.Screen_X;
      Original_screen_Y=PCX_header.Screen_Y;

      Pre_load(context, context->Width, context->Height, file_size, FORMAT_PCX, PIXEL_SIMPLE, PCX_header.Plane * PCX_header.Depth);

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
                    DEBUG("ERROR READING PCX PALETTE !",index);
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
                        else
                          Set_file_error(2);
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
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;

  short line_size;
  short x_pos;
  short y_pos;
  byte  counter;
  byte  last_pixel;
  byte  pixel_read;



  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((file=fopen(filename,"wb")))
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
      remove(filename);
       
  }    
  else 
    File_error=1;
}      


//////////////////////////////////// SCx ////////////////////////////////////
typedef struct
{
  byte Filler1[4];
  word Width;
  word Height;
  byte Filler2;
  byte Planes;
} T_SCx_Header;

// -- Tester si un fichier est au format SCx --------------------------------
void Test_SCx(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  //byte Signature[3];
  T_SCx_Header SCx_header;


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture et vérification de la signature
    if (Read_bytes(file,SCx_header.Filler1,4)
    && Read_word_le(file, &(SCx_header.Width))
    && Read_word_le(file, &(SCx_header.Height))
    && Read_byte(file, &(SCx_header.Filler2))
    && Read_byte(file, &(SCx_header.Planes))
    )
    {
      if ( (!memcmp(SCx_header.Filler1,"RIX",3))
        && SCx_header.Width && SCx_header.Height)
      File_error=0;
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format SCx -----------------------------------------
void Load_SCx(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  long size,real_size;
  long file_size;
  T_SCx_Header SCx_header;
  T_Palette SCx_Palette;
  byte * buffer;

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,SCx_header.Filler1,4)
    && Read_word_le(file, &(SCx_header.Width))
    && Read_word_le(file, &(SCx_header.Height))
    && Read_byte(file, &(SCx_header.Filler2))
    && Read_byte(file, &(SCx_header.Planes))
    )
    {
      Pre_load(context, SCx_header.Width,SCx_header.Height,file_size,FORMAT_SCx,PIXEL_SIMPLE,0);
      if (File_error==0)
      {
        if (!SCx_header.Planes)
          size=sizeof(T_Palette);
        else
          size=sizeof(T_Components)*(1<<SCx_header.Planes);

        if (Read_bytes(file,SCx_Palette,size))
        {
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));

          Palette_64_to_256(SCx_Palette);
          memcpy(context->Palette,SCx_Palette,size);

          context->Width=SCx_header.Width;
          context->Height=SCx_header.Height;

          if (!SCx_header.Planes)
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
            size=((context->Width+7)>>3)*SCx_header.Planes;
            real_size=(size/SCx_header.Planes)<<3;
            buffer=(byte *)malloc(size);

            for (y_pos=0;(y_pos<context->Height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,buffer,size))
                Draw_IFF_line(context, buffer, y_pos,real_size,SCx_header.Planes);
              else
                File_error=2;
            }
          }
          free(buffer);
        }
        else
          File_error=1;
      }
    }
    else
      File_error=1;

    fclose(file);
  }
  else
    File_error=1;
}

// -- Sauver un fichier au format SCx ---------------------------------------
void Save_SCx(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_SCx_Header SCx_header;
  byte last_char;
 
  last_char=strlen(context->File_name)-1;
  if (context->File_name[last_char]=='?')
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
  }
  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    T_Palette palette_64;
    
    setvbuf(file, NULL, _IOFBF, 64*1024);
    memcpy(palette_64,context->Palette,sizeof(T_Palette));
    Palette_256_to_64(palette_64);
    
    memcpy(SCx_header.Filler1,"RIX3",4);
    SCx_header.Width=context->Width;
    SCx_header.Height=context->Height;
    SCx_header.Filler2=0xAF;
    SCx_header.Planes=0x00;

    if (Write_bytes(file,SCx_header.Filler1,4)
    && Write_word_le(file, SCx_header.Width)
    && Write_word_le(file, SCx_header.Height)
    && Write_byte(file, SCx_header.Filler2)
    && Write_byte(file, SCx_header.Planes)
    && Write_bytes(file,&palette_64,sizeof(T_Palette))
    )
    {
      for (y_pos=0; ((y_pos<context->Height) && (!File_error)); y_pos++)
        for (x_pos=0; x_pos<context->Width; x_pos++)
          Write_one_byte(file,Get_pixel(context, x_pos,y_pos));

      fclose(file);

      if (File_error)
        remove(filename);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}

//////////////////////////////////// XPM ////////////////////////////////////
void Save_XPM(T_IO_Context* context)
{
  FILE* file;
  char filename[MAX_PATH_CHARACTERS];
  int i,j;

  Get_full_filename(filename, context->File_name, context->File_directory);
  File_error = 0;

  file = fopen(filename, "w");
  if (file == NULL)
  {
    File_error = 1;
    return;
  }
  setvbuf(file, NULL, _IOFBF, 64*1024);
  fprintf(file, "/* XPM */\nstatic char* pixmap[] = {\n");
  fprintf(file, "\"%d %d 256 2\",\n", context->Width, context->Height);

  for (i = 0; i < 256; i++)
  {
    fprintf(file,"\"%2.2X c #%2.2x%2.2x%2.2x\",\n", i, context->Palette[i].R, context->Palette[i].G,
      context->Palette[i].B);
  }

  for (j = 0; j < context->Height; j++)
  {
    fprintf(file, "\"");
    for (i = 0; i < context->Width; i++)
    {
      fprintf(file, "%2.2X", Get_pixel(context, i, j));
    }
    fprintf(file,"\"\n");
  }

  fclose(file);
}

//////////////////////////////////// PNG ////////////////////////////////////

#ifndef __no_pnglib__

// -- Tester si un fichier est au format PNG --------------------------------
void Test_PNG(T_IO_Context * context)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte png_header[8];
  
  Get_full_filename(filename, context->File_name, context->File_directory);
  
  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture du header du fichier
    if (Read_bytes(file,png_header,8))
    {
      if ( !png_sig_cmp(png_header, 0, 8))
        File_error=0;
    }
    fclose(file);
  }
}

/// Used by a callback in Load_PNG
T_IO_Context * PNG_current_context;

int PNG_read_unknown_chunk(png_structp ptr, png_unknown_chunkp chunk)
{
  (void)ptr; // unused
  // png_unknown_chunkp members:
  //    png_byte name[5];
  //    png_byte *data;
  //    png_size_t size;
  
  if (!strcmp((const char *)chunk->name, "crNg"))
  {
    // Color animation. Similar to a CRNG chunk in an IFF file.
    unsigned int i;
    byte *chunk_ptr = chunk->data;
    
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
        
          PNG_current_context->Cycle_range[i].Start=min_col;
          PNG_current_context->Cycle_range[i].End=max_col;
          PNG_current_context->Cycle_range[i].Inverse=(flags&2)?1:0;
          PNG_current_context->Cycle_range[i].Speed=(flags&1) ? rate/78 : 0;
                              
          PNG_current_context->Color_cycles=i+1;
      }
    }
  
    return (1); // >0 = success
  }
  return (0); /* did not recognize */
  
}


// -- Lire un fichier au format PNG -----------------------------------------
static void Load_PNG_Sub(T_IO_Context * context, FILE * file)
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
      png_voidp user_chunk_ptr;
      byte bpp;

      // Setup a return point. If a pnglib loading error occurs
      // in this if(), the else will be executed.
      if (!setjmp(png_jmpbuf(png_ptr)))
      {
        png_init_io(png_ptr, file);
        // Inform pnglib we already loaded the header.
        png_set_sig_bytes(png_ptr, 8);

        // Hook the handler for unknown chunks
        user_chunk_ptr = png_get_user_chunk_ptr(png_ptr);
        png_set_read_user_chunk_fn(png_ptr, user_chunk_ptr, &PNG_read_unknown_chunk);
        // This is a horrid way to pass parameters, but we don't get
        // much choice. PNG loader can't be reintrant.
        PNG_current_context=context;

        // Load file information
        png_read_info(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr,info_ptr);
        bit_depth = png_get_bit_depth(png_ptr,info_ptr);

        switch (color_type)
        {
          case PNG_COLOR_TYPE_GRAY_ALPHA:
            bpp = bit_depth * 2;
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
            bpp = bit_depth;
        }

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
              if (!strcmp(text_ptr[num_text].key,"Title"))
              {
                int size;
                size = Min(text_ptr[num_text].text_length, COMMENT_SIZE);
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
              if (res_y/res_x>1)
              {
                context->Ratio=PIXEL_WIDE;
              }
              else if (res_x/res_y>1)
              {
                context->Ratio=PIXEL_TALL;
              }
            }
          }
          if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            Pre_load(context,png_get_image_width(png_ptr,info_ptr),png_get_image_height(png_ptr,info_ptr),File_length_file(file),FORMAT_PNG,PIXEL_SIMPLE,bpp);
          else
            Pre_load(context,png_get_image_width(png_ptr,info_ptr),png_get_image_height(png_ptr,info_ptr),File_length_file(file),FORMAT_PNG,context->Ratio,bpp);

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
              if (color_type == PNG_COLOR_TYPE_PALETTE && trans!=NULL)
              {
                int i;
                for (i=0; i<num_trans; i++)
                {
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

            context->Width=png_get_image_width(png_ptr,info_ptr);
            context->Height=png_get_image_height(png_ptr,info_ptr);

            png_set_interlace_handling(png_ptr);
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

void Load_PNG(T_IO_Context * context)
{
  FILE *file;
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte png_header[8];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    // Load header (8 first bytes)
    if (Read_bytes(file,png_header,8))
    {
      // Do we recognize a png file signature ?
      if ( !png_sig_cmp(png_header, 0, 8))
        Load_PNG_Sub(context, file);
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

void Save_PNG(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  int y;
  byte * pixel_ptr;
  png_structp png_ptr;
  png_infop info_ptr;
  png_unknown_chunk crng_chunk;
  byte cycle_data[16*6]; // Storage for color-cycling data, referenced by crng_chunk
  static png_bytep * Row_pointers;
  
  Get_full_filename(filename, context->File_name, context->File_directory);
  File_error=0;
  Row_pointers = NULL;
  
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);
    
    /* initialisation */
    if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))
      && (info_ptr = png_create_info_struct(png_ptr)))
    {
  
      if (!setjmp(png_jmpbuf(png_ptr)))
      {    
        png_init_io(png_ptr, file);
      
        /* en-tete */
        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          png_set_IHDR(png_ptr, info_ptr, context->Width, context->Height,
            8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

          png_set_PLTE(png_ptr, info_ptr, (png_colorp)context->Palette, 256);
          {
            // Commentaires texte PNG
            // Cette partie est optionnelle
            #ifdef PNG_iTXt_SUPPORTED
              png_text text_ptr[2] = {
                {-1, "Software", "Grafx2", 6, 0, NULL, NULL},
                {-1, "Title", NULL, 0, 0, NULL, NULL}
            #else
            png_text text_ptr[2] = {
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
          switch(Pixel_ratio)
          {
            case PIXEL_WIDE:
            case PIXEL_WIDE2:
              png_set_pHYs(png_ptr, info_ptr, 3000, 6000, PNG_RESOLUTION_METER);
              break;
            case PIXEL_TALL:
            case PIXEL_TALL2:
            case PIXEL_TALL3:
              png_set_pHYs(png_ptr, info_ptr, 6000, 3000, PNG_RESOLUTION_METER);
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
    // fermeture du fichier
    fclose(file);
  }

  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri trainait... Ca fait pas propre.
  if (File_error)
    remove(filename);
  
  if (Row_pointers)
  {
    free(Row_pointers);
    Row_pointers=NULL;
  }
}
#endif  // __no_pnglib__

