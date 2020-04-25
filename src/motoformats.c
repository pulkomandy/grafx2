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

///@file motoformats.c
/// Formats for the MO/TO Thomson machines

#include <stdlib.h>
#include <string.h>
#if defined(_MSC_VER)
#define strdup _strdup
#endif
#include "struct.h"
#include "io.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "fileformats.h"
#include "oldies.h"
#include "input.h"
#include "engine.h"
#include "screen.h"
#include "windows.h"
#include "help.h"
#include "gfx2mem.h"
#include "gfx2log.h"

extern char Program_version[]; // generated in pversion.c
extern const char SVN_revision[]; // generated in version.c

/////////////////////////////// Thomson Files ///////////////////////////////

/**
 * Test for Thomson file
 */
void Test_MOTO(T_IO_Context * context, FILE * file)
{
  long file_size;

  file_size = File_length_file(file);

  File_error = 1;
  if (file_size <= 10)
    return;
  switch (MOTO_Check_binary_file(file))
  {
    case 0: // Not Thomson binary format
      switch (file_size)
      {
        // Files in RAW formats (from TGA2teo)
        case 8004:  // 2 colors palette
        case 8008:  // 4 colors palette
        case 8032:  // 16 colors palette
          {
            char * filename;
            char * path;
            char * ext;

            // Check there are both FORME and COULEUR files
            filename = strdup(context->File_name);
            ext = strrchr(filename, '.');
            if (ext == NULL || ext == filename)
            {
              free(filename);
              return;
            }
            if ((ext[-1] | 32) == 'c')
              ext[-1] = (ext[-1] & 32) | 'P';
            else if ((ext[-1] | 32) == 'p')
              ext[-1] = (ext[-1] & 32) | 'C';
            else
            {
              free(filename);
              return;
            }
            path = Filepath_append_to_dir(context->File_directory, filename);
            if (File_exists(path))
              File_error = 0;
            free(path);
            free(filename);
          }
          return;
        default:
          break;
      }
      break;
    case 2: // MAP file (SAVEP/LOADP)
    case 3: // TO autoloading picture
    case 4: // MO autoloading picture
      File_error = 0;
      return;
  }
}

/**
 * Load a picture for Thomson TO8/TO8D/TO9/TO9+/MO6
 *
 * One of the supported format is the one produced by TGA2Teo :
 * - Picture data is splitted into 2 files, one for each VRAM bank :
 *   - The first VRAM bank is called "forme" (shape).
 *     In 40col mode it stores pixels.
 *   - The second VRAM bank is called "couleur" (color).
 *     In 40col mode it store color indexes for foreground and background.
 * - File extension is .BIN, character before extension is "P" for the first
 *   file, and "C" for the second.
 * - The color palette is stored in both files after the data.
 *
 * The mode is detected thanks to the number of color in the palette :
 * - 2 colors is 80col (640x200)
 * - 4 colors is bitmap4 (320x200 4 colors)
 * - 16 colors is either bitmap16 (160x200 16colors)
 *   or 40col (320x200 16 colors with 2 unique colors in each 8x1 pixels
 *   block).
 *
 * As it is not possible to disriminate bitmap16 and 40col, opening the "P"
 * file sets bitmap16, opening the "C" file sets 40col.
 *
 * This function also supports .MAP files (with optional TO-SNAP extension)
 * and our own "autoloading" BIN files.
 * See http://pulkomandy.tk/projects/GrafX2/wiki/Develop/FileFormats/MOTO for
 * a detailled description.
 */
void Load_MOTO(T_IO_Context * context)
{
  // FORME / COULEUR
  FILE * file;
  byte * vram_forme = NULL;
  byte * vram_couleur = NULL;
  long file_size;
  int file_type;
  int bx, x, y, i;
  byte bpp = 4;
  byte code;
  word length, address;
  int transpose = 1;  // transpose the upper bits of the color plane bytes
                      // FFFFBBBB becomes bfFFFBBB (for TO7 compatibility)
  enum MOTO_Graphic_Mode mode = MOTO_MODE_40col;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  int width = 320, height = 200, columns = 40;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  // Load default palette
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  MOTO_set_TO7_palette(context->Palette);

  file_type = MOTO_Check_binary_file(file);
  if (fseek(file, 0, SEEK_SET) < 0)
  {
    fclose(file);
    return;
  }

  if (file_type == 2) // MAP file
  {
    // http://collection.thomson.free.fr/code/articles/prehisto_bulletin/page.php?XI=0&XJ=13
    byte map_mode, col_count, line_count;
    byte * vram_current;
    int end_marks;

    if (!(Read_byte(file,&code) && Read_word_be(file,&length) && Read_word_be(file,&address)))
    {
      fclose(file);
      return;
    }
    if (length < 5 || !(Read_byte(file,&map_mode) && Read_byte(file,&col_count) && Read_byte(file,&line_count)))
    {
      fclose(file);
      return;
    }
    length -= 3;
    columns = col_count + 1;
    height = 8 * (line_count + 1);
    switch(map_mode)
    {
      default:
      case 0: // bitmap4 or 40col
        width = 8 * columns;
        mode = MOTO_MODE_40col; // default to 40col
        bpp = 4;
        break;
      case 0x40:  // bitmap16
        columns >>= 1;
        width = 4 * columns;
        mode = MOTO_MODE_bm16;
        bpp = 4;
        ratio = PIXEL_WIDE;
        break;
      case 0x80:  // 80col
        columns >>= 1;
        width = 16 * columns;
        mode = MOTO_MODE_80col;
        bpp = 1;
        ratio = PIXEL_TALL;
        break;
    }
    GFX2_Log(GFX2_DEBUG, "Map mode &H%02X row=%u line=%u (%dx%d) %d\n", map_mode, col_count, line_count, width, height, columns * height);
    vram_forme = GFX2_malloc(columns * height);
    vram_couleur = GFX2_malloc(columns * height);
    // Check extension (TO-SNAP / PPM / ???)
    if (length > 36)
    {
      long pos_backup;
      word data;

      pos_backup = ftell(file);
      fseek(file, length-2, SEEK_CUR);  // go to last word of chunk
      Read_word_be(file, &data);
      GFX2_Log(GFX2_DEBUG, "%04X\n", data);
      switch (data)
      {
      case 0xA55A:  // TO-SNAP
        fseek(file, -40, SEEK_CUR); // go to begin of extension
        Read_word_be(file, &data);  // SCRMOD. 0=>40col, 1=>bm4, $40=>bm16, $80=>80col
        GFX2_Log(GFX2_DEBUG, "SCRMOD=&H%04X ", data);
        Read_word_be(file, &data);  // Border color
        GFX2_Log(GFX2_DEBUG, "BORDER=%u ", data);
        Read_word_be(file, &data);  // Mode BASIC (CONSOLE,,,,X) 0=40col, 1=80col, 2=bm4, 3=bm16, etc.
        GFX2_Log(GFX2_DEBUG, "CONSOLE,,,,%u\n", data);
        if(data == 2)
        {
          mode = MOTO_MODE_bm4;
          bpp = 2;
        }
        for (i = 0; i < 16; i++)
        {
          Read_word_be(file, &data);  // Palette entry
          if (data & 0x8000) data = ~data;
          MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[i], data);
        }
        snprintf(context->Comment, sizeof(context->Comment), "TO-SNAP .MAP file");
        break;
      case 0x484C:  // 'HL' PPM
        fseek(file, -36, SEEK_CUR); // go to begin of extension
        for (i = 0; i < 16; i++)
        {
          Read_word_be(file, &data);  // Palette entry
          if (data & 0x8000) data = ~data;
          MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[i], data);
        }
        Read_word_be(file, &data);  // Mode BASIC (CONSOLE,,,,X) 0=40col, 1=80col, 2=bm4, 3=bm16, etc.
        GFX2_Log(GFX2_DEBUG, "CONSOLE,,,,%u\n", data);
        if(data == 2)
        {
          mode = MOTO_MODE_bm4;
          bpp = 2;
        }
        snprintf(context->Comment, sizeof(context->Comment), "PPM .MAP file");
        break;
      default:
        snprintf(context->Comment, sizeof(context->Comment), "standard .MAP file");
      }
      fseek(file, pos_backup, SEEK_SET);  // RESET Position
    }
    i = 0;
    vram_current = vram_forme;
    end_marks = 0;
    while (length > 1)
    {
      byte byte1, byte2;
      Read_byte(file,&byte1);
      Read_byte(file,&byte2);
      length-=2;
      if(byte1 == 0)
      {
        if (byte2 == 0)
        {
          // end of vram stream
          GFX2_Log(GFX2_DEBUG, "0000 i=%d length=%ld\n", i, length);
          if (end_marks == 1)
            break;
          i = 0;
          vram_current = vram_couleur;
          end_marks++;
        }
        else while(byte2-- > 0 && length > 0) // copy
        {
          Read_byte(file,vram_current + i);
          length--;
          i += columns; // to the next line
          if (i >= columns * height)
          {
            if (mode == MOTO_MODE_bm4 || mode == MOTO_MODE_40col)
              i -= (columns * height - 1);  // to the 1st line of the next column
            else
            {
              i -= columns * height;  // back to the 1st line of the current column
              if (vram_current == vram_forme)   // other VRAM
                vram_current = vram_couleur;
              else
              {
                vram_current = vram_forme;
                i++;  // next column
              }
            }
          }
        }
      }
      else while(byte1-- > 0) // run length
      {
        vram_current[i] = byte2;
        i += columns; // to the next line
        if (i >= columns * height)
        {
          if (mode == MOTO_MODE_bm4 || mode == MOTO_MODE_40col)
            i -= (columns * height - 1);  // to the 1st line of the next column
          else
          {
            i -= columns * height;  // back to the 1st line of the current column
            if (vram_current == vram_forme)   // other VRAM
              vram_current = vram_couleur;
            else
            {
              vram_current = vram_forme;
              i++;  // next column
            }
          }
        }
      }
    }
    fclose(file);
  }
  else if(file_type == 3 || file_type == 4)
  {
    if (file_type == 4) // MO file
    {
      transpose = 0;
      MOTO_set_MO5_palette(context->Palette);
    }

    do
    {
      if (!(Read_byte(file,&code) && Read_word_be(file,&length) && Read_word_be(file,&address)))
      {
        if (vram_forme)
          break;
        fclose(file);
        return;
      }
      // MO5/MO6 VRAM address is &H0000
      // TO7/TO8/TO9 VRAM addres is &H4000
      if (length >= 8000 && length <= 8192 && (address == 0x4000 || address == 0))
      {
        if (vram_forme == NULL)
        {
          vram_forme = calloc(8192, 1);
          Read_bytes(file, vram_forme, length);
          length = 0;
        }
        else if (vram_couleur == NULL)
        {
          vram_couleur = calloc(8192, 1);
          Read_bytes(file, vram_couleur, length);
          if (length >= 8032)
          {
            for (x = 0; x < 16; x++)
            {
              // 1 byte Blue (4 lower bits)
              // 1 byte Green (4 upper bits) / Red (4 lower bits)
              MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[x],
                                             vram_couleur[8000+x*2]<<8 | vram_couleur[8000+x*2+1]);
            }
            if (length >= 8064)
            {
              memcpy(context->Comment, vram_couleur + 8032, 32);
              if (vram_couleur[8063] >= '0' && vram_couleur[8063] <= '3')
                mode = vram_couleur[8063] - '0';
            }
            context->Comment[COMMENT_SIZE] = '\0';
          }
          length = 0;
        }
      }
      if (length > 0)
        fseek(file, length, SEEK_CUR);
    } while(code == 0);
    fclose(file);
    switch (mode)
    {
      case MOTO_MODE_40col: // default
        break;
      case MOTO_MODE_bm4:
        bpp = 2;
        break;
      case MOTO_MODE_80col:
        bpp = 1;
        width = 640;
        ratio = PIXEL_TALL;
        break;
      case MOTO_MODE_bm16:
        width = 160;
        ratio = PIXEL_WIDE;
        break;
    }
  }
  else
  {
    char * filename;
    char * path;
    char * ext;
    int n_colors;

    vram_forme = GFX2_malloc(file_size);
    if (vram_forme == NULL)
    {
      fclose(file);
      return;
    }
    if (!Read_bytes(file, vram_forme, file_size))
    {
      free(vram_forme);
      fclose(file);
      return;
    }
    n_colors = (file_size - 8000) / 2;
    switch(n_colors)
    {
      case 16:
        bpp = 4;
        // 16 colors : either 40col or bm16 mode !
        // select later
        break;
      case 4:
        bpp = 2;
        mode = MOTO_MODE_bm4;
        break;
      default:
        bpp = 1;
        mode = MOTO_MODE_80col;
        width = 640;
        ratio = PIXEL_TALL;
    }
    filename = strdup(context->File_name);
    ext = strrchr(filename, '.');
    if (ext == NULL || ext == filename)
    {
      free(vram_forme);
      free(filename);
      return;
    }
    if ((ext[-1] | 32) == 'c')
    {
      vram_couleur = vram_forme;
      vram_forme = NULL;
      ext[-1] = (ext[-1] & 32) | 'P';
    }
    else if ((ext[-1] | 32) == 'p')
    {
      ext[-1] = (ext[-1] & 32) | 'C';
      if (n_colors == 16)
      {
        mode = MOTO_MODE_bm16;
        width = 160;
        ratio = PIXEL_WIDE;
      }
    }
    else
    {
      free(vram_forme);
      free(filename);
      return;
    }
    path = Filepath_append_to_dir(context->File_directory, filename);
    file = fopen(path, "rb");
    if (file == NULL)
      GFX2_Log(GFX2_ERROR, "Failed to open %s\n", path);
    free(path);
    free(filename);
    if (vram_forme == NULL)
    {
      vram_forme = GFX2_malloc(file_size);
      if (vram_forme == NULL)
      {
        free(vram_couleur);
        fclose(file);
        return;
      }
      Read_bytes(file,vram_forme,file_size);
    }
    else
    {
      vram_couleur = GFX2_malloc(file_size);
      if (vram_couleur == NULL)
      {
        free(vram_forme);
        fclose(file);
        return;
      }
      Read_bytes(file,vram_couleur,file_size);
    }
    fclose(file);
    GFX2_Log(GFX2_DEBUG, "MO/TO: %s,%s file_size=%ld n_colors=%d\n", context->File_name, filename, file_size, n_colors);
    for (x = 0; x < n_colors; x++)
    {
      // 1 byte Blue (4 lower bits)
      // 1 byte Green (4 upper bits) / Red (4 lower bits)
      MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[x],
                                     vram_couleur[8000+x*2]<<8 | vram_couleur[8000+x*2+1]);
    }
  }
  Pre_load(context, width, height, file_size, FORMAT_MOTO, ratio, bpp);
  if (mode == MOTO_MODE_40col)
    Set_image_mode(context, IMAGE_MODE_THOMSON);
  File_error = 0;
  i = 0;
  for (y = 0; y < height; y++)
  {
    for (bx = 0; bx < columns; bx++)
    {
      byte couleur_forme;
      byte couleur_fond;
      byte forme, couleurs;

      forme = vram_forme[i];
      if (vram_couleur)
        couleurs = vram_couleur[i];
      else
        couleurs = (mode == MOTO_MODE_40col) ? 0x01 : 0x00;
      i++;
      switch(mode)
      {
        case MOTO_MODE_bm4:
          for (x = bx*8; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, ((forme & 0x80) >> 6) | ((couleurs & 0x80) >> 7));
            forme <<= 1;
            couleurs <<= 1;
          }
#if 0     // the following would be for the alternate bm4 mode
          for (x = bx*8; x < bx*8+4; x++)
          {
            Set_pixel(context, x, y, couleurs >> 6);
            couleurs <<= 2;
          }
          for (x = bx*8 + 4; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, forme >> 6);
            forme <<= 2;
          }
#endif
          break;
        case MOTO_MODE_bm16:
          Set_pixel(context, bx*4, y, forme >> 4);
          Set_pixel(context, bx*4+1, y, forme & 0x0F);
          Set_pixel(context, bx*4+2, y, couleurs >> 4);
          Set_pixel(context, bx*4+3, y, couleurs & 0x0F);
          break;
        case MOTO_MODE_80col:
          for (x = bx*16; x < bx*16+8; x++)
          {
            Set_pixel(context, x, y, (forme & 0x80) >> 7);
            Set_pixel(context, x+8, y, (couleurs & 0x80) >> 7);
            forme <<= 1;
            couleurs <<= 1;
          }
          break;
        case MOTO_MODE_40col:
        default:
          if (transpose)
          {
            // the color plane byte is bfFFFBBB (for TO7 compatibility)
            // with the upper bits of both foreground (forme) and
            // background (fond) inverted.
            couleur_forme = ((couleurs & 0x78) >> 3) ^ 0x08;
            couleur_fond = ((couleurs & 7) | ((couleurs & 0x80) >> 4)) ^ 0x08;
          }
          else
          {
            // MO5 : the color plane byte is FFFFBBBB
            couleur_forme = couleurs >> 4;
            couleur_fond = couleurs & 0x0F;
          }
          for (x = bx*8; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, (forme & 0x80)?couleur_forme:couleur_fond);
            forme <<= 1;
          }
      }
    }
  }
  free(vram_forme);
  free(vram_couleur);
}

/**
 * Pack a stream of byte in the format used by Thomson MO/TO MAP files.
 *
 * - 00 cc xx yy .. : encodes a "copy run" (cc = bytes to copy)
 * - cc xx          : encodes a "repeat run" (cc > 0 : count)
 */
//#define MOTO_MAP_NOPACKING
unsigned int MOTO_MAP_pack(byte * packed, const byte * unpacked, unsigned int unpacked_len)
{
  unsigned int src;
  unsigned int dst = 0;
  unsigned int count;
#ifndef MOTO_MAP_NOPACKING
  unsigned int repeat;
  unsigned int i;
  word * counts;
#endif

  GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack(%p, %p, %u)\n", packed, unpacked, unpacked_len);
  if (unpacked_len == 0)
    return 0;
  if (unpacked_len == 1)
  {
    packed[0] = 1;
    packed[1] = unpacked[0];
    return 2;
  }
#ifdef MOTO_MAP_NOPACKING
  // compression disabled
  src = 0;
  while ((unpacked_len - src) > 255)
  {
    packed[dst++] = 0;
    packed[dst++] = 255;
    memcpy(packed+dst, unpacked+src, 255);
    dst += 255;
    src += 255;
  }
  count = unpacked_len - src;
  packed[dst++] = 0;
  packed[dst++] = count;
  memcpy(packed+dst, unpacked+src, count);
  dst += count;
  src += count;
  return dst;
#else
  counts = GFX2_malloc(sizeof(word) * (unpacked_len + 1));
  i = 0;
  repeat = (unpacked[0] == unpacked[1]);
  count = 2;
  src = 2;
  // 1st step : count lenght of the Copy runs and Repeat runs
  while (src < unpacked_len)
  {
    if (repeat)
    {
      if (unpacked[src-1] == unpacked[src])
        count++;
      else
      {
        // flush the repeat run
        counts[i++] = count | 0x8000; // 0x8000 is the marker for repeat runs
        count = 1;
        repeat = 0;
      }
    }
    else
    {
      if (unpacked[src-1] != unpacked[src])
        count++;
      else if (count == 1)
      {
        count++;
        repeat = 1;
      }
      else
      {
        // flush the copy run
        counts[i++] = (count-1) | (count == 2 ? 0x8000 : 0); // mark copy run of 1 as repeat of 1
        count = 2;
        repeat = 1;
      }
    }
    src++;
  }
  // flush the last run
  counts[i++] = ((repeat || count == 1) ? 0x8000 : 0) | count;
  counts[i++] = 0;  // end marker
  // check consistency of counts
  count = 0;
  for (i = 0; counts[i] != 0; i++)
    count += (counts[i] & ~0x8000);
  if (count != unpacked_len)
    GFX2_Log(GFX2_ERROR, "*** encoding error in MOTO_MAP_pack() *** count=%u unpacked_len=%u\n",
             count, unpacked_len);
  // output optimized packed stream
  // repeat run are encoded cc xx
  // copy run are encoded   00 cc xx xx xx xx
  i = 0;
  src = 0;
  while (counts[i] != 0)
  {
    while (counts[i] & 0x8000)  // repeat run
    {
      count = counts[i] & ~0x8000;
      GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() %4u %4u repeat %u times %02x\n", src, i, count, unpacked[src]);
      while(count > 255)
      {
        packed[dst++] = 255;
        packed[dst++] = unpacked[src];
        count -= 255;
        src += 255;
      }
      packed[dst++] = count;
      packed[dst++] = unpacked[src];
      src += count;
      i++;
    }
    while (counts[i] != 0 && !(counts[i] & 0x8000))  // copy run
    {
      // calculate the "savings" of repeat runs between 2 copy run
      int savings = 0;
      unsigned int j;
      GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() %4u %4u copy %u bytes\n", src, i, counts[i]);
      for (j = i + 1; counts[j] & 0x8000; j++) // check repeat runs until the next copy run
      {
        count = counts[j] & ~0x8000;
        if (savings < 0 && (savings + (int)count - 2) > 0)
          break;
        savings += count - 2; // a repeat run outputs 2 bytes for count bytes of input
      }
      count = counts[i];
GFX2_Log(GFX2_DEBUG, " savings=%d i=%u j=%u (counts[j]=0x%04x)\n", savings, i, j, counts[j]);
      if (savings < 2 && (j > i + 1))
      {
        unsigned int k;
        if (counts[j] == 0) // go to the end of stream
        {
          for (k = i + 1; k < j; k++)
            count += (counts[k] & ~0x8000);
          GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u extend copy from %u to %u\n", src, counts[i], count);
          i = j - 1;
        }
        else
        {
          for (k = i + 1; k < j; k++)
            count += (counts[k] & ~0x8000);
          if (!(counts[j] & 0x8000))
          { // merge with the next copy run (and the repeat runs between)
            GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u merge savings=%d\n", src, savings);
            i = j;
            counts[i] += count;
            continue;
          }
          else
          { // merge with the next few repeat runs
            GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u extends savings=%d\n", src, savings);
            i = j - 1;
          }
        }
      }
      while (count > 255)
      {
        packed[dst++] = 0;
        packed[dst++] = 255;
        memcpy(packed+dst, unpacked+src, 255);
        dst += 255;
        src += 255;
        count -= 255;
      }
      packed[dst++] = 0;
      packed[dst++] = count;
      memcpy(packed+dst, unpacked+src, count);
      dst += count;
      src += count;
      i++;
    }
  }
  free(counts);
  return dst;
#endif
}


/**
 * GUI window to choose Thomson MO/TO saving parameters
 *
 * @param[out] machine target machine
 * @param[out] format file format (0 = BIN, 1 = MAP)
 * @param[in,out] mode video mode @ref MOTO_Graphic_Mode
 */
static int Save_MOTO_window(enum MOTO_Machine_Type * machine, int * format, enum MOTO_Graphic_Mode * mode)
{
  int button;
  T_Dropdown_button * machine_dd;
  T_Dropdown_button * format_dd;
  T_Dropdown_button * mode_dd;
  static const char * mode_list[] = { "40col", "80col", "bm4", "bm16" };
  char text_info[24];

  Open_window(200, 125, "Thomson MO/TO Saving");
  Window_set_normal_button(110,100,80,15,"Save",1,1,KEY_RETURN); // 1
  Window_set_normal_button(10,100,80,15,"Cancel",1,1,KEY_ESCAPE); // 2

  Print_in_window(13,18,"Target Machine:",MC_Dark,MC_Light);
  machine_dd = Window_set_dropdown_button(10,28,110,15,100,
                                          (*mode == MOTO_MODE_40col) ? "TO7/TO7-70" : "TO9/TO8/TO9+",
                                          1, 0, 1, LEFT_SIDE,0); // 3
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(machine_dd, MACHINE_TO7, "TO7/TO7-70");
  Window_dropdown_add_item(machine_dd, MACHINE_TO8, "TO9/TO8/TO9+");
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(machine_dd, MACHINE_MO5, "MO5");
  Window_dropdown_add_item(machine_dd, MACHINE_MO6, "MO6");

  Print_in_window(13,46,"Format:",MC_Dark,MC_Light);
  format_dd = Window_set_dropdown_button(10,56,110,15,92,"BIN",1, 0, 1, LEFT_SIDE,0); // 4
  Window_dropdown_add_item(format_dd, 0, "BIN");
  Window_dropdown_add_item(format_dd, 1, "MAP/TO-SNAP");

  Print_in_window(136,46,"Mode:",MC_Dark,MC_Light);
  mode_dd = Window_set_dropdown_button(136,56,54,15,44,mode_list[*mode],1, 0, 1, LEFT_SIDE,0); // 5
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);
  if (*mode == MOTO_MODE_80col)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(mode_dd, MOTO_MODE_bm4, mode_list[MOTO_MODE_bm4]);
  if (*mode == MOTO_MODE_bm16)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);

  Update_window_area(0,0,Window_width,Window_height);
  Display_cursor();
  do
  {
    button = Window_clicked_button();
    if (Is_shortcut(Key, 0x100+BUTTON_HELP))
    {
      Key = 0;
      Window_help(BUTTON_SAVE, "THOMSON MO/TO FORMAT");
    }
    else switch (button)
    {
      case 3:
        *machine = (enum MOTO_Machine_Type)Window_attribute2;
        break;
      case 4:
        *format = Window_attribute2;
        break;
      case 5:
        *mode = (enum MOTO_Graphic_Mode)Window_attribute2;
        break;
    }
    Hide_cursor();
    //"ABCDEFGHIJKLMNOPQRSTUVW"
    memset(text_info, ' ', 23);
    text_info[23] = '\0';
    if (*machine == MACHINE_TO7 || *machine == MACHINE_TO770 || *machine == MACHINE_MO5)
    {
      if (*mode != MOTO_MODE_40col)
        snprintf(text_info, sizeof(text_info), "%s only supports 40col",
                 (*machine == MACHINE_MO5) ? "MO5" : "TO7");
      else if (*format == 1)
        strncpy(text_info, "No TO-SNAP extension.  ", sizeof(text_info));
      else
        strncpy(text_info, "No palette to save.    ", sizeof(text_info));
    }
    Print_in_window(9, 80, text_info, MC_Dark, MC_Light);
    Display_cursor();
  } while(button!=1 && button!=2);

  Close_window();
  Display_cursor();
  return button==1;
}

/**
 * Save a picture in MAP or BIN Thomson MO/TO file format.
 *
 * File format details :
 * http://pulkomandy.tk/projects/GrafX2/wiki/Develop/FileFormats/MOTO
 */
void Save_MOTO(T_IO_Context * context)
{
  int transpose = 1;  // transpose upper bits in "couleur" vram
  enum MOTO_Machine_Type target_machine = MACHINE_TO7;
  int format = 0; // 0 = BIN, 1 = MAP
  enum MOTO_Graphic_Mode mode;
  FILE * file = NULL;
  byte * vram_forme;
  byte * vram_couleur;
  int i, x, y, bx;
  word reg_prc = 0xE7C3; // PRC : TO7/8/9 0xE7C3 ; MO5/MO6 0xA7C0
  byte prc_value = 0x65;// Value to write to PRC to select VRAM bank
  // MO5 : 0x51
  word vram_address = 0x4000; // 4000 on TO7/TO8/TO9, 0000 on MO5/MO6

  File_error = 1;

  /**
   * In the future we could support other resolution for .MAP
   * format.
   * And even in .BIN format, we could store less lines. */
  if (context->Height != 200)
  {
    Warning_message("must be 640x200, 320x200 or 160x200");
    return;
  }

  switch (context->Width)
  {
    case 160:
      mode = MOTO_MODE_bm16;
      target_machine = MACHINE_TO8;
      break;
    case 640:
      mode = MOTO_MODE_80col;
      target_machine = MACHINE_TO8;
      break;
    case 320:
      mode = MOTO_MODE_40col; // or bm4
      break;
    default:
      Warning_message("must be 640x200, 320x200 or 160x200");
      return;
  }

  if (!Save_MOTO_window(&target_machine, &format, &mode))
    return;

  if (target_machine == MACHINE_MO5 || target_machine == MACHINE_MO6)
  {
    reg_prc = 0xA7C0; // PRC : MO5/MO6 0xA7C0
    prc_value = 0x51;
    vram_address = 0;
    transpose = 0;
  }

  vram_forme = GFX2_malloc(8192);
  vram_couleur = GFX2_malloc(8192);
  switch (mode)
  {
  case MOTO_MODE_40col:
    {
      /**
       * The 40col encoding algorithm is optimized for further vertical
       * RLE packing. The "attibute" byte is kept as constant as possible
       * between adjacent blocks.
       */
      unsigned color_freq[16];
      unsigned max_freq = 0;
      byte previous_fond = 0, previous_forme = 0;
      byte most_used_color = 0;

      // search for most used color to prefer it as background color
      for (i = 0; i < 16; i++)
        color_freq[i] = 0;
      for (y = 0; y < context->Height; y++)
      {
        for (x = 0; x < context->Width; x++)
        {
          byte col = Get_pixel(context, x, y);
          if (col > 15)
          {
            Warning_with_format("color %u > 15 at pixel (%d,%d)", col, x, y);
            goto error;
          }
          color_freq[col]++;
        }
      }
      for (i = 0; i < 16; i++)
      {
        if (color_freq[i] > max_freq)
        {
          max_freq = color_freq[i];
          most_used_color = (byte)i;  // most used color
        }
      }
      previous_fond = most_used_color;
      max_freq = 0;
      for (i = 0; i < 16; i++)
      {
        if (i != most_used_color && color_freq[i] > max_freq)
        {
          max_freq = color_freq[i];
          previous_forme = (byte)i;  // second most used color
        }
      }
      GFX2_Log(GFX2_DEBUG, "Save_MOTO() most used color index %u, 2nd %u\n", previous_fond, previous_forme);

      if (target_machine == MACHINE_MO5)
      {
        /**
         * For MO5 we use a different 40col algorithm
         * to make sure the last pixel of a GPL and the first the next
         * are both FORME or both FOND, else we get an ugly glitch on the
         * EFGJ033 Gate Array MO5!
         */
        byte forme_byte = 0;
        byte couleur_byte = 0x10;
        GFX2_Log(GFX2_DEBUG, "Save_MOTO() 40col using MO5 algo\n");
        for (y = 0; y < context->Height; y++)
        {
          for (bx = 0; bx < 40; bx++)
          {
            byte fond = 0xff, forme = 0xff;
            forme_byte &= 1;  // Last bit of the previous FORME byte
            x = bx*8;
            if (forme_byte)
              forme = Get_pixel(context, x, y);
            else
              fond = Get_pixel(context, x, y);
            while (++x < bx * 8 + 8)
            {
              byte col = Get_pixel(context, x, y);
              forme_byte <<= 1;
              if (col == forme)
                forme_byte |= 1;
              else if (col != fond)
              {
                if (forme == 0xff)
                {
                  forme_byte |= 1;
                  forme = col;
                }
                else if (fond == 0xff)
                  fond = col;
                else
                {
                  Warning_with_format("Constraint error at pixel (%d,%d)", x, y);
                  goto error;
                }
              }
            }
            if (forme != 0xff)
              couleur_byte = (forme << 4) | (couleur_byte & 0x0f);
            if (fond != 0xff)
              couleur_byte = (couleur_byte & 0xf0) | fond;
            vram_forme[bx+y*40] = forme_byte;
            vram_couleur[bx+y*40] = couleur_byte;
          }
        }
      }
      else
      {
        GFX2_Log(GFX2_DEBUG, "Save_MOTO() 40col using optimized algo\n");
        // encoding of each 8x1 block
        for (bx = 0; bx < 40; bx++)
        {
          for (y = 0; y < context->Height; y++)
          {
            byte forme_byte = 1;
            byte col;
            byte c1, c1_count = 1;
            byte c2 = 0xff, c2_count = 0;
            byte fond, forme;
            x = bx * 8;
            c1 = Get_pixel(context, x, y);
            while (++x < bx * 8 + 8)
            {
              forme_byte <<= 1;
              col = Get_pixel(context, x, y);
              if (col > 15)
              {
                Warning_with_format("color %d > 15 at pixel (%d,%d)", col, x, y);
                goto error;
              }
              if (col == c1)
              {
                forme_byte |= 1;
                c1_count++;
              }
              else
              {
                c2_count++;
                if (c2 == 0xff)
                  c2 = col;
                else if (col != c2)
                {
                  Warning_with_format("constraint error at pixel (%d,%d)", x, y);
                  goto error;
                }
              }
            }
            if (c2 == 0xff)
            {
              // Only one color in the 8x1 block
              if (c1 == previous_fond)
                c2 = previous_forme;
              else
                c2 = previous_fond;
            }
            // select background color (fond)
            // and foreground color (forme)
            if (c1 == previous_fond)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else if (c2 == previous_fond)
            {
              fond = c2;
              forme = c1;
            }
            else if (c1 == most_used_color)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else if (c2 == most_used_color)
            {
              fond = c2;
              forme = c1;
            }
            else if (c1_count >= c2_count)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else
            {
              fond = c2;
              forme = c1;
            }
            // write to VRAM
            vram_forme[bx+y*40] = forme_byte;
            // transpose for TO7 compatibility
            if (transpose)
              vram_couleur[bx+y*40] = ((fond & 7) | ((fond & 8) << 4) | (forme << 3)) ^ 0xC0;
            else
              vram_couleur[bx+y*40] = fond | (forme << 4);
            previous_fond = fond;
            previous_forme = forme;
          }
          if (transpose)
          {
            previous_fond = (vram_couleur[bx] & 7) | (~vram_couleur[bx] & 0x80) >> 4;
            previous_forme = ((vram_couleur[bx] & 0x78) >> 3) ^ 8;
          }
          else
          {
            previous_fond = vram_couleur[bx] & 15;
            previous_forme = vram_couleur[bx] >> 4;
          }
        }
      }
    }
    break;
  case MOTO_MODE_80col:
    for (bx = 0; bx < context->Width / 16; bx++)
    {
      for (y = 0; y < context->Height; y++)
      {
        byte val = 0;
        for (x = bx * 16; x < bx*16 + 8; x++)
          val = (val << 1) | Get_pixel(context, x, y);
        vram_forme[y*(context->Width/16)+bx] = val;
        for (; x < bx*16 + 16; x++)
          val = (val << 1) | Get_pixel(context, x, y);
        vram_couleur[y*(context->Width/16)+bx] = val;
      }
    }
    break;
  case MOTO_MODE_bm4:
    for (y = 0; y < context->Height; y++)
    {
      for (bx = 0; bx < context->Width / 8; bx++)
      {
        byte val1 = 0, val2 = 0, pixel;
        for (x = bx * 8; x < bx*8 + 8; x++)
        {
          pixel = Get_pixel(context, x, y);
          if (pixel > 3)
          {
            Warning_with_format("color %d > 3 at pixel (%d,%d)", pixel, x, y);
            goto error;
          }
          val1 = (val1 << 1) | (pixel >> 1);
          val2 = (val2 << 1) | (pixel & 1);
        }
        vram_forme[y*(context->Width/8)+bx] = val1;
        vram_couleur[y*(context->Width/8)+bx] = val2;
      }
    }
    break;
  case MOTO_MODE_bm16:
    for (bx = 0; bx < context->Width / 4; bx++)
    {
      for (y = 0; y < context->Height; y++)
      {
         vram_forme[y*(context->Width/4)+bx] = (Get_pixel(context, bx*4, y) << 4) | Get_pixel(context, bx*4+1, y);
         vram_couleur[y*(context->Width/4)+bx] = (Get_pixel(context, bx*4+2, y) << 4) | Get_pixel(context, bx*4+3, y);
      }
    }
    break;
  }
  // palette
  for (i = 0; i < 16; i++)
  {
    word to8color = MOTO_gamma_correct_RGB_to_MOTO(context->Palette + i);
    vram_forme[8000+i*2] = to8color >> 8;
    vram_forme[8000+i*2+1] = to8color & 0xFF;
  }

  file = Open_file_write(context);
  if (file == NULL)
    goto error;

  if (format == 0)  // BIN
  {
    word chunk_length;

    if (target_machine == MACHINE_TO7 || target_machine == MACHINE_TO770 || target_machine == MACHINE_MO5)
      chunk_length = 8000;  // Do not save palette
    else
    {
      chunk_length = 8000 + 32 + 32;  // data + palette + comment
      // Commentaire
      if (context->Comment[0] != '\0')
        strncpy((char *)vram_forme + 8032, context->Comment, 32);
      else
        snprintf((char *)vram_forme + 8032, 32, "GrafX2 %s.%s", Program_version, SVN_revision);
      // also saves the video mode
      vram_forme[8063] = '0' + mode;
      memcpy(vram_couleur + 8000, vram_forme + 8000, 64);
    }

    // Format BIN
    // TO8/TO9 : set LGAMOD 0xE7DC  40col=0 bm4=0x21 80col=0x2a bm16=0x7b
    if (!DECB_BIN_Add_Chunk(file, 1, reg_prc, &prc_value))
      goto error;
    if (!DECB_BIN_Add_Chunk(file, chunk_length, vram_address, vram_forme))
      goto error;
    prc_value &= 0xFE; // select color data
    if (!DECB_BIN_Add_Chunk(file, 1, reg_prc, &prc_value))
      goto error;
    if (!DECB_BIN_Add_Chunk(file, chunk_length, vram_address, vram_couleur))
      goto error;
    if (!DECB_BIN_Add_End(file, 0x0000))
      goto error;
  }
  else
  {
    // format MAP with TO-SNAP extensions
    byte * unpacked_data;
    byte * packed_data;

    unpacked_data = GFX2_malloc(16*1024);
    packed_data = GFX2_malloc(16*1024);
    if (packed_data == NULL || unpacked_data == NULL)
    {
      GFX2_Log(GFX2_ERROR, "Failed to allocate 2x16kB of memory\n");
      free(packed_data);
      free(unpacked_data);
      goto error;
    }
    switch (mode)
    {
      case MOTO_MODE_40col:
      case MOTO_MODE_bm4:
        packed_data[0] = 0;  // mode
        packed_data[1] = (context->Width / 8) - 1;
        break;
      case MOTO_MODE_80col:
        packed_data[0] = 0x80; // mode
        packed_data[1] = (context->Width / 8) - 1;
        break;
      case MOTO_MODE_bm16:
        packed_data[0] = 0x40; // mode
        packed_data[1] = (context->Width / 2) - 1;
        break;
    }
    packed_data[2] = (context->Height / 8) - 1;
    // 1st step : put data to pack in a linear buffer
    // 2nd step : pack data
    i = 0;
    switch (mode)
    {
      case MOTO_MODE_40col:
      case MOTO_MODE_bm4:
        for (bx = 0; bx <= packed_data[1]; bx++)
        {
          for (y = 0; y < context->Height; y++)
          {
            unpacked_data[i] = vram_forme[bx + y*(packed_data[1]+1)];
            unpacked_data[i+8192] = vram_couleur[bx + y*(packed_data[1]+1)];
            i++;
          }
        }
        i = 3;
        i += MOTO_MAP_pack(packed_data+3, unpacked_data, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM forme packing
        packed_data[i++] = 0;
        i += MOTO_MAP_pack(packed_data+i, unpacked_data + 8192, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM couleur packing
        packed_data[i++] = 0;
        break;
      case MOTO_MODE_80col:
      case MOTO_MODE_bm16:
        for (bx = 0; bx < (packed_data[1] + 1) / 2; bx++)
        {
          for (y = 0; y < context->Height; y++)
            unpacked_data[i++] = vram_forme[bx + y*(packed_data[1]+1)/2];
          for (y = 0; y < context->Height; y++)
            unpacked_data[i++] = vram_couleur[bx + y*(packed_data[1]+1)/2];
        }
        i = 3;
        i += MOTO_MAP_pack(packed_data+3, unpacked_data, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM forme packing
        packed_data[i++] = 0;
        packed_data[i++] = 0; // ending of VRAM couleur packing
        packed_data[i++] = 0;
        break;
    }
    if (i&1)  // align
      packed_data[i++] = 0;
    if (target_machine != MACHINE_TO7 && target_machine != MACHINE_TO770 && target_machine != MACHINE_MO5)
    {
      // add TO-SNAP extension
      // see http://collection.thomson.free.fr/code/articles/prehisto_bulletin/page.php?XI=0&XJ=13
      // bytes 0-1 : Hardware video mode (value of SCRMOD 0x605F)
      packed_data[i++] = 0;
      switch (mode)
      {
        case MOTO_MODE_40col:
          packed_data[i++] = 0;
          break;
        case MOTO_MODE_bm4:
          packed_data[i++] = 0x01;
          break;
        case MOTO_MODE_80col:
          packed_data[i++] = 0x80;
          break;
        case MOTO_MODE_bm16:
          packed_data[i++] = 0x40;
          break;
      }
      // bytes 2-3 : Border color
      packed_data[i++] = 0;
      packed_data[i++] = 0;
      // bytes 4-5 : BASIC video mode (CONSOLE,,,,X)
      packed_data[i++] = 0;
      switch (mode)
      {
        case MOTO_MODE_40col:
          packed_data[i++] = 0;
          break;
        case MOTO_MODE_bm4:
          packed_data[i++] = 2;
          break;
        case MOTO_MODE_80col:
          packed_data[i++] = 1;
          break;
        case MOTO_MODE_bm16:
          packed_data[i++] = 3;
          break;
      }
      // bytes 6-37 : BGR palette
      for (x = 0; x < 16; x++)
      {
        word bgr = MOTO_gamma_correct_RGB_to_MOTO(context->Palette + x);
        packed_data[i++] = bgr >> 8;
        packed_data[i++] = bgr & 0xff;
      }
      // bytes 38-39 : TO-SNAP signature
      packed_data[i++] = 0xA5;
      packed_data[i++] = 0x5A;
    }

    free(unpacked_data);

    if (!DECB_BIN_Add_Chunk(file, i, 0, packed_data) ||
        !DECB_BIN_Add_End(file, 0x0000))
    {
      free(packed_data);
      goto error;
    }
    free(packed_data);
  }
  fclose(file);
  File_error = 0;
  return;

error:
  free(vram_forme);
  free(vram_couleur);
  if (file)
    fclose(file);
  File_error = 1;
}
