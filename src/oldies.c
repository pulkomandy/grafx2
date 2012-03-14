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
#include <SDL.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>
#include "struct.h"
#include "global.h"
#include "errors.h"
#include "misc.h"
#include "palette.h"

void Pixel_in_layer(word x,word y, byte layer, byte color)
{
  *((y)*Main_image_width+(x)+Main_backups->Pages->Image[layer].Pixels)=color;
}

byte C64_FLI(byte *bitmap, byte *screen_ram, byte *color_ram, byte *background)
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
  if (Main_backups->Pages->Nb_layers < 3)
    return 1;
  if (Main_image_width != 160 || Main_image_height != 200)
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
        byte c=*((row)*Main_image_width+(col*4+x)+Main_backups->Pages->Image[2].Pixels);
        used_colors[row][col] |= 1<<c;
      }
    }
  }
  // Get "mandatory colors" from layer 1
  for (row=0;row<200;row++)
  {
    byte c=*((row)*Main_image_width+0+Main_backups->Pages->Image[0].Pixels);
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
      byte c=*((row)*Main_image_width+(col*4)+Main_backups->Pages->Image[1].Pixels);
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
            byte c=*((row*8+y)*Main_image_width+(col*4+x)+Main_backups->Pages->Image[2].Pixels);
           
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

byte C64_FLI_enforcer(void)
{
  byte background[200];
  byte bitmap[8000];
  byte screen_ram[8192];
  byte color_ram[1000];
  
  int row, col, x, y;
  byte c[4];
  
  // Checks
  if (Main_image_width != 160)
    return 1;
  if (Main_image_height != 200)
    return 1;
  if (Main_backups->Pages->Nb_layers != 4)
    return 2;
  
  Backup_layers(1<<3);
  
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
          Pixel_in_layer(col*4+(3-x),row*8+y,3,color);
        }
      }
    }
  }
  End_of_modification();
  
  // Visible feedback:
  
  // If the "check" layer was visible, manually update the whole thing
  if (Main_layers_visible & (1<<3))
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
