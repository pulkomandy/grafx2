/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "struct.h"
#include "oldies.h"
#include "global.h"
#include "errors.h"
#include "io.h"
#include "misc.h"
#include "palette.h"
#include "pages.h"
#include "windows.h"
#include "layers.h"

static void Set_Pixel_in_layer(word x,word y, byte layer, byte color)
{
  *((y)*Main.image_width+(x)+Main.backups->Pages->Image[layer].Pixels)=color;
}

int C64_FLI(byte *bitmap, byte *screen_ram, byte *color_ram, byte *background)
{
  word used_colors[200][40];
  word block_used_colors[25][40];
  word line_used_colors[200];
  byte used_colors_count[200][40];
  dword usage[16];
  word x,y,row,col;
  int i;
  byte line_color[200];
  byte block_color[25][40];
  word best_color_count;
  byte best_color;
  const byte no_color=16;

  // Prerequisites
  if (Main.backups->Pages->Nb_layers < 3)
    return 1;
  if (Main.image_width != 160 || Main.image_height != 200)
    return 2;
 
  memset(used_colors,0,200*40*sizeof(word));
  memset(block_used_colors,0,25*40*sizeof(word));
  memset(line_used_colors,0,200*sizeof(word));
  memset(used_colors_count,0,200*40*sizeof(byte));

  // Initialize these as "unset"
  memset(line_color,no_color,200*sizeof(byte));
  memset(block_color,no_color,25*40*sizeof(byte));
 
  // Examine all 4-pixel blocks to fill used_colors[][]
  for (row=0;row<200;row++)
  {
    for (col=0;col<40;col++)
    {
      for (x=0;x<4;x++)
      {
        byte c=*((row)*Main.image_width+(col*4+x)+Main.backups->Pages->Image[2].Pixels);
        used_colors[row][col] |= 1<<c;
      }
    }
  }
  // Get "mandatory colors" from layer 1
  for (row=0;row<200;row++)
  {
    byte c=*((row)*Main.image_width+0+Main.backups->Pages->Image[0].Pixels);
    if (c<16)
    {
      line_color[row]=c;
      for (col=0;col<40;col++)
      {
        // Remove that color from the sets
        used_colors[row][col] &= ~(1<<c);
      }
    }
  }
  // Get "mandatory colors" from layer 2
  for (row=0;row<200;row+=8)
  {
    for (col=0;col<40;col++)
    {
      byte c=*((row)*Main.image_width+(col*4)+Main.backups->Pages->Image[1].Pixels);
      if (c<16)
      {
        block_color[row/8][col]=c;
        // Remove that color from the sets
        for (y=0; y<8;y++)
          used_colors[row+y][col] &= ~(1<<c);
      }
    }
  }
  // Now count the "remaining colors".
  for (row=0;row<200;row++)
  {
    memset(usage,0,16*sizeof(dword));
    for (col=0;col<40;col++)
    {
      used_colors_count[row][col]=Popcount_word(used_colors[row][col]);
      // Count which colors are used 3 times
      if (used_colors_count[row][col]==3)
      {
        for (i=0; i<16; i++)
          if (used_colors[row][col] & (1<<i))
            usage[i]+=1;
      }
      // Count which colors are used 4 times (important)
      else if (used_colors_count[row][col]==4)
      {
        for (i=0; i<16; i++)
          if (used_colors[row][col] & (1<<i))
            usage[i]+=2;
      }
    }
    // A complete line has been examined. Pick the color which has been tagged most, and set it as
    // the line color (unless it already was set.)
    if (line_color[row]==no_color)
    {
      best_color_count=0;
      best_color=no_color;
      for (i=0; i<16; i++)
      {
        if (usage[i]>best_color_count)
        {
          best_color_count=usage[i];
          best_color=i;
        }
      }
      line_color[row]=best_color;

      // Remove that color from the sets
      for (col=0;col<40;col++)
      {
        if (used_colors[row][col] & (1<<best_color))
        {
          used_colors[row][col] &= ~(1<<best_color);
          // Update count
          used_colors_count[row][col]--;
        }
      }
    }
  }


  // Now check all 4*8 blocks
  for (col=0;col<40;col++)
  {
    for (row=0;row<200;row+=8)
    {
      // If there wasn't a preset color for this block
      if (block_color[row/8][col]==no_color)
      {
        word filter=0xFFFF;
        
        // Count used colors
        memset(usage,0,16*sizeof(dword));
        for (y=0;y<8;y++)
        {
          if (used_colors_count[row+y][col]>2)
          {
            filter &= used_colors[row+y][col];
            
            for (i=0; i<16; i++)
            {
              if (used_colors[row+y][col] & (1<<i))
                usage[i]+=1;
            }
          }
        }
        if (filter != 0 && Popcount_word(filter) == 1)
        {
          // Only one color matched all needs: Use it.
          i=1;
          best_color=0;
          while (! (filter & i))
          {
            best_color++;
            i= i << 1;
          }
        }
        else
        {
          if (filter==0)
          {
            // No color in common from multiple lines with 3+ colors...
            // Keep them all for the usage sort.
            filter=0xFFFF;
          }

          // Pick the color which has been tagged most, and set it as
          // the block color
          best_color_count=0;
          best_color=no_color;
          for (i=0; i<16; i++)
          {
            if (filter & (1<<i))
            {
              if (usage[i]>best_color_count)
              {
                best_color_count=usage[i];
                best_color=i;
              }
            }
          }
        }
        block_color[row/8][col]=best_color;

        // Remove that color from the sets
        for (y=0;y<8;y++)
        {
          if (used_colors[row+y][col] & (1<<best_color))
          {
            used_colors[row+y][col] &= ~(1<<best_color);
            // Update count
            used_colors_count[row+y][col]--;
          }
        }
      }
    }
  }
  // At this point, the following arrays are filled:
  // - block_color[][]
  // - line_color[]
  // They contain either a color 0-16, or no_color if no color is mandatory. 
  // It's now possible to scan the whole image and choose how to encode it.
  // TODO: Draw problematic places on layer 4, with one of the available colors
  /*
  if (bitmap!=NULL)
  {
    for (row=0;row<200;row++)
    {
      for (col=0;col<40;col++)
      {

      }
    }
  }
  */
  
  // Output background
  if (background!=NULL)
  {
    for (row=0;row<200;row++)
    {
      if (line_color[row]==no_color)
        background[row]=0; // Unneeded line is black
      else
        background[row]=line_color[row];
    }
  }
  
  // Output Color RAM
  if (color_ram!=NULL)
  {
    for (col=0;col<40;col++)
    {
      for (row=0;row<25;row++)
      {
        if (block_color[row][col]==no_color)
          color_ram[row*40+col]=0; // Unneeded block is black
        else
          color_ram[row*40+col]=block_color[row][col];
      }
    }
  }
  
  for(row=0; row<25; row++)
  {
    for(col=0; col<40; col++)
    {
      for(y=0; y<8; y++)
      {
        byte c1, c2;
        
        // Find 2 colors in used_colors[row*8+y][col]
        for (c1=0; c1<16 && !(used_colors[row*8+y][col] & (1<<c1)); c1++)
          ;
        for (c2=c1+1; c2<16 && !(used_colors[row*8+y][col] & (1<<c2)); c2++)
          ;
        if (c1>15)
          c1=16;
        if (c2>15)
          c2=16;
        
        // Output Screen RAMs
        if (screen_ram!=NULL)
          screen_ram[y*1024+row*40+col] = (c1&15) | ((c2&15)<<4);
          
        // Output bitmap
        if (bitmap!=NULL)
        {
          for(x=0; x<4; x++)
          {
            byte bits;
            byte c=*((row*8+y)*Main.image_width+(col*4+x)+Main.backups->Pages->Image[2].Pixels);
           
            if (c==line_color[row*8+y])
              // BG color
              bits=0;
            else if (c==block_color[row][col])
              // block color
              bits=3;
            else if (c==c1)
              // Color 1
              bits=2;
            else if (c==c2)
              // Color 2
              bits=1;
            else // problem
              bits=0;
            // clear target bits
            //bitmap[row*320+col*8+y] &= ~(3<<((3-x)*2));
            // set them
            bitmap[row*320+col*8+y] |= bits<<((3-x)*2);
          }
        }
      }
    }
  }
  //memset(background,3,200);
  //memset(color_ram,5,8000);
  //memset(screen_ram,(9<<4) | 7,8192);
  
  return 0;

}

int C64_FLI_enforcer(void)
{
  byte background[200];
  byte bitmap[8000];
  byte screen_ram[8192];
  byte color_ram[1000];
  
  int row, col, x, y;
  byte c[4];
  
  // Checks
  if (Main.image_width != 160 || Main.image_height != 200)
  {
    GFX2_Log(GFX2_WARNING, "C64_FLI_enforcer() requires 160x200 image resolution\n");
    return 1;
  }
  if (Main.backups->Pages->Nb_layers != 4) {
    GFX2_Log(GFX2_WARNING, "C64_FLI_enforcer() requires 4 layers\n");
    return 2;
  }
  
  Backup_layers(3);
  
  memset(bitmap,0,8000);
  memset(background,0,200);
  memset(color_ram,0,1000);
  memset(screen_ram,0,8192);
  C64_FLI(bitmap, screen_ram, color_ram, background);

  for(row=0; row<25; row++)
  {
    for(col=0; col<40; col++)
    {
      c[3]=color_ram[row*40+col]&15;
      for(y=0; y<8; y++)
      {
        int pixel=bitmap[row*320+col*8+y];
        
        c[0]=background[row*8+y]&15;
        c[1]=screen_ram[y*1024+row*40+col]>>4;
        c[2]=screen_ram[y*1024+row*40+col]&15;
        for(x=0; x<4; x++)
        {
          int color=c[(pixel&3)];
          pixel>>=2;
          Set_Pixel_in_layer(col*4+(3-x),row*8+y,3,color);
        }
      }
    }
  }
  End_of_modification();
  
  // Visible feedback:
  
  // If the "check" layer was visible, manually update the whole thing
  if (Main.layers_visible & (1<<3))
  {
    Hide_cursor();
    Redraw_layered_image();
    Display_all_screen();
    Display_layerbar();
    Display_cursor();
  }  
  else
  // Otherwise, simply toggle the layer visiblity
    Layer_activate(3,RIGHT_SIDE);
    
    
  return 0;
}

void C64_set_palette(T_Components * palette)
{
  /// Set C64 Palette from http://www.pepto.de/projects/colorvic/
  static const byte pal[48]={
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x68, 0x37, 0x2B,
    0x70, 0xA4, 0xB2,
    0x6F, 0x3D, 0x86,
    0x58, 0x8D, 0x43,
    0x35, 0x28, 0x79,
    0xB8, 0xC7, 0x6F,
    0x6F, 0x4F, 0x25,
    0x43, 0x39, 0x00,
    0x9A, 0x67, 0x59,
    0x44, 0x44, 0x44,
    0x6C, 0x6C, 0x6C,
    0x9A, 0xD2, 0x84,
    0x6C, 0x5E, 0xB5,
    0x95, 0x95, 0x95};

  memcpy(palette, pal, 48);
  /// Also set transparent color "16" as a dark grey that is distinguishable
  /// from black, but darker than normal colors.
  palette[16].R=20;
  palette[16].G=20;
  palette[16].B=20;
  /// set color "17" as RED to see color clashes
  palette[17].R=255;
  palette[17].G=0;
  palette[17].B=0;
}

void ZX_Spectrum_set_palette(T_Components * palette)
{
  int i, intensity;

  for (i = 0; i < 16; i++)
  {
    intensity = (i & 8) ? 255 : 205;
    palette[i].G = ((i & 4) >> 2) * intensity;
    palette[i].R = ((i & 2) >> 1) * intensity;
    palette[i].B = (i & 1) * intensity;
  }
  // set color 17 for color clashes
  palette[17].R = 255;
  palette[17].G = 127;
  palette[17].B = 127;
}

static const T_Components CPC_Hw_Palette[] = {
  {0x6E, 0x7D, 0x6B}, // 0x40
  {0x6E, 0x7B, 0x6B}, // 0x41
  {0, 0xF3, 0x6B},
  {0xF3, 0xF3, 0x6D},
  {0, 2, 0x6B},
  {0xF0, 2, 0x68},
  {0, 0x78, 0x68},
  {0xF3, 0x7D, 0x6B},

  {0xF3, 0x02, 0x68}, // 0x48
  {0xF3, 0xF3, 0x6B},
  {0xF3, 0xF3, 0xD},
  {255, 0xF3, 0xF9},
  {0xF3, 5, 6},
  {0xF3, 2, 0xF4},
  {0xF3, 0x7D, 0xD},
  {0xFA, 0x80, 0xF9},

  {0x00, 0x02, 0x68}, // 0x50
  {0x02, 0xF3, 0x6B},
  {2, 0xF0, 1},
  {0xF, 0xF3, 0xF2},
  {0, 2, 1},
  {0x0C, 2, 0xF4},
  {2, 0x78, 1},
  {0xC, 0x7B, 0xF4},
  {0x69, 2, 0x68},  // 0x58
  {0x71, 0xF3, 0x6B},
  {0x71, 0xF5, 4},
  {0x71, 0xF3, 0xF4},
  {0x6C, 2, 1},
  {0x6C, 2, 0xF2},
  {0x6E, 0x7B, 1},
  {0x6E, 0x7B, 0xF6}
};

void CPC_set_HW_palette(T_Components * palette)
{
  memcpy(palette, CPC_Hw_Palette, sizeof(CPC_Hw_Palette));
}

int CPC_compare_colors(T_Components * col1, T_Components * col2)
{
  byte v1, v2;
  /** The mapping used in this function is :
   * - RGB value   [0: 85] => CPC level 0
   * - RGB value  [86:171] => CPC level 1
   * - RGB value [172:255] => CPC level 2 */
  v1 = (col1->R / 86) << 4 | (col1->G / 86) << 2 | (col1->B / 86);
  v2 = (col2->R / 86) << 4 | (col2->G / 86) << 2 | (col2->B / 86);
  return v1 == v2;
}

void CPC_set_default_BASIC_palette(T_Components * palette)
{
  static const byte basic_colors[] = {
    0x44/*0x50*/, 0x4a, 0x53, 0x4c,
    0x4b, 0x54, 0x55, 0x4d,
    0x46, 0x5e, 0x5f, 0x47,
    0x52, 0x59, 0x4a, 0x47
  };
  unsigned int i;
  /** in the default CPC Basic palette, INKs 14 and 15 are blicking,
   * so that would be great to include theses in the colorcycling,
   * but I don't see any way to make theses blinking colors
   * with the way GrafX2 handles color cycling */

  for (i = 0; i < sizeof(basic_colors) / sizeof(byte); i++)
    memcpy(palette + i,
           CPC_Hw_Palette + basic_colors[i] - 0x40,
           sizeof(T_Components));
}

int CPC_check_AMSDOS(FILE * file, word * loading_address, unsigned long * file_length)
{
  int i;
  byte data[128];
  word checksum = 0;

  fseek(file, 0, SEEK_SET);
  if (!Read_bytes(file, data, 128))
    return 0;
  for (i = 1; i <= 11; i++) // check filename and extension
  {
    if (data[i] < ' ' || data[i] >= 0x7F)
      return 0;
  }
  for (i = 0; i < 67; i++)
    checksum += (word)data[i];
  if (checksum != (data[67] | (data[68] << 8)))
  {
    GFX2_Log(GFX2_INFO, "AMSDOS header checksum mismatch %04X != %04X\n",
             checksum, data[67] | (data[68] << 8));
    return 0;
  }
  GFX2_Log(GFX2_DEBUG, "AMSDOS : user=%02X %.8s.%.3s %d %u(%u) bytes, load at $%04X exec $%04X checksum $%04X\n",
           data[0],
           (char *)(data + 1), (char *)(data + 9), data[18], /* Type*/
           data[24] | (data[25] << 8), data[64] | (data[65] << 8) | (data[66] << 16),
           data[21] | (data[22] << 8),
           data[26] | (data[27] << 8), checksum);
  if (loading_address)
    *loading_address = data[21] | (data[22] << 8);
  //  *exec_address = data[26] | (data[27] << 8);
  if (file_length)
    *file_length = data[64] | (data[65] << 8) | (data[66] << 16); // 24bit size
  //  *file_length = data[24] | (data[25] << 8);  // 16bit size
  return 1;
}

int DECB_Check_binary_file(FILE * f)
{
  byte code;
  word length, address;
  long end;

  // Check end of file
  if (fseek(f, -5, SEEK_END) < 0)
    return 0;
  if (!(Read_byte(f, &code) && Read_word_be(f, &length) && Read_word_be(f, &address)))
    return 0;
  if (code != 0xff || length != 0)
    return 0;
  end = ftell(f);
  if (end < 0)
    return 0;
  if (fseek(f, 0, SEEK_SET) < 0)
    return 0;
  // Read Binary structure
  do
  {
    if (!(Read_byte(f, &code) && Read_word_be(f, &length) && Read_word_be(f, &address)))
      return 0;
    // skip chunk content
    if (fseek(f, length, SEEK_CUR) < 0)
      return 0;
  }
  while(code == 0);
  if (code != 0xff)
    return 0;
  if (ftell(f) != end)
    return 0;
  return 1;
}

int MOTO_Check_binary_file(FILE * f)
{
  int type = 1; // BIN
  byte code;
  word length, address;

  if (!DECB_Check_binary_file(f))
    return 0;
  if (fseek(f, 0, SEEK_SET) < 0)
    return 0;
  // Read Binary structure
  do
  {
    if (!(Read_byte(f, &code) && Read_word_be(f, &length) && Read_word_be(f, &address)))
      return 0;
    GFX2_Log(GFX2_DEBUG, "Thomson BIN &H%02X &H%04X &H%04X\n", code, length, address);

    // VRAM is &H4000 on TO7/TO8/TO9/TO9+
    if (length >= 8000 && length <= 8192 && address == 0x4000)
      type = 3; // TO autoloading picture
    else if (length > 16 && address == 0)  // address is 0 for MAP files
    {
      byte map_header[3];
      if (!Read_bytes(f, map_header, 3))
        return 0;
      length -= 3;
      if ((map_header[0] & 0x3F) == 0 && (map_header[0] & 0xC0) != 0xC0)  // 00, 40 or 80
      {
        GFX2_Log(GFX2_DEBUG, "Thomson MAP &H%02X %u %u\n", map_header[0], map_header[1], map_header[2]);
        if (((map_header[1] < 80 && map_header[0] != 0) // <= 80col in 80col and bm16
             || map_header[1] < 40) // <= 40col in 40col
            && map_header[2] < 25)  // <= 200 pixels high
          type = 2; // MAP file (SAVEP/LOADP format)
      }
      if (type == 1)
      {
        if (length == 8000 || length == 8032 || length == 8064)
          type = 4; // MO autoloading picture
      }
    }
    else if (length == 1)
    {
      if(address == 0xE7C3)  // TO7/TO8/TO9 6846 PRC
        type = 3; // TO autoloading picture
      else if(address == 0xA7C0)  // MO5/MO6 PRC
        type = 4; // MO autoloading picture
    }

    if (fseek(f, length, SEEK_CUR) < 0)
      return 0;
  }
  while(code == 0);
  return type;
}

int DECB_BIN_Add_Chunk(FILE * f, word size, word address, const byte * data)
{
  return Write_byte(f, 0)
      && Write_word_be(f, size)
      && Write_word_be(f, address)
      && Write_bytes(f, data, size);
}

int DECB_BIN_Add_End(FILE * f, word address)
{
  return Write_byte(f, 0xff)
      && Write_word_be(f, 0)
      && Write_word_be(f, address);
}

word MOTO_gamma_correct_RGB_to_MOTO(const T_Components * color)
{
  word r, g, b;
  double gamma = Config.MOTO_gamma / 10.0;
  r = (word)round(pow(color->R / 255.0, gamma) * 15.0);
  g = (word)round(pow(color->G / 255.0, gamma) * 15.0);
  b = (word)round(pow(color->B / 255.0, gamma) * 15.0);
  GFX2_Log(GFX2_DEBUG, "#%02x%02x%02x => &H%04X\n",
           color->R, color->G, color->B,
           b << 8 | g << 4 | r);
  return b << 8 | g << 4 | r;
}

void MOTO_gamma_correct_MOTO_to_RGB(T_Components * color, word bgr)
{
  double inv_gamma = 10.0 / Config.MOTO_gamma;
  color->B = (byte)round(pow(((bgr >> 8)& 0x0F)/15.0, inv_gamma) * 255.0);
  color->G = (byte)round(pow(((bgr >> 4)& 0x0F)/15.0, inv_gamma) * 255.0);
  color->R = (byte)round(pow((bgr & 0x0F)/15.0, inv_gamma) * 255.0);
}

void MOTO_set_MO5_palette(T_Components * palette)
{
  static const unsigned char mo5palette[48] = {
    // Taken from https://16couleurs.wordpress.com/2013/03/31/archeologie-infographique-le-pixel-art-pour-thomson/
    // http://pulkomandy.tk/wiki/doku.php?id=documentations:devices:gate.arrays#video_generation
    0, 0, 0, 255, 85, 85, 0, 255, 0, 255, 255, 0,
    85, 85, 255, 255, 0, 255, 0, 255, 255, 255, 255, 255,
    170, 170, 170, 255, 170, 255, 170, 255, 170, 255, 255, 170,
    85, 170, 255, 255, 170, 170, 170, 255, 255, 255, 170, 85
  };
  memcpy(palette, mo5palette, 48);
  // set color 17 for color clashes
  palette[17].R = 255;
  palette[17].G = 0;
  palette[17].B = 85;
}

void MOTO_set_TO7_palette(T_Components * palette)
{
  int i;
  static const word to8default_pal[16] = {
    // BGR values Dumped from a TO8D with a BASIC 512 program :
    // FOR I=0TO15:PRINT PALETTE(I):NEXT I
    0x000, 0x00F, 0x0F0, 0x0FF, 0xF00, 0xF0F, 0xFF0, 0xFFF,
    0x777, 0x33A, 0x3A3, 0x3AA, 0xA33, 0xA3A, 0xEE7, 0x07B
  };

  for (i = 0; i < 16; i++)
    MOTO_gamma_correct_MOTO_to_RGB(palette + i, to8default_pal[i]);
  // set color 17 for color clashes
  palette[17].R = 255;
  palette[17].G = 127;
  palette[17].B = 127;
}
