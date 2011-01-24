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
  *((y)*Main_image_width+(x)+Main_backups->Pages->Image[layer])=color;
}

// - Fallback - for incomplete platforms
  byte a(short x, short y, byte color){return 0;}
  int b(){return -1;}
  int c(word x, word y){return -1;}
  byte d(word x, word y, byte color, int with_preview) {return 0;}
  void e(){}

const T_Constraint_Enforcer ConstraintsNone = {
  a,b,c,d, e
};


byte Null_enforcer(void)
{
  return 0;
}

// C64 FLI

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
        byte c=*((row)*Main_image_width+(col*4+x)+Main_backups->Pages->Image[2]);
        used_colors[row][col] |= 1<<c;
      }
    }
  }
  // Get "mandatory colors" from layer 1
  for (row=0;row<200;row++)
  {
    byte c=*((row)*Main_image_width+0+Main_backups->Pages->Image[0]);
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
      byte c=*((row)*Main_image_width+(col*4)+Main_backups->Pages->Image[1]);
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
            byte c=*((row*8+y)*Main_image_width+(col*4+x)+Main_backups->Pages->Image[2]);
           
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

// - Amstrad CPC "Mode 5" rasters

void CPC_Mode5_InitMode()
{
  // Set picture size
  // 5 layers
  Backup_with_new_dimensions(1,5,320,200);
  // Palette ?
}

byte CPC_Mode5_DisplayPaintbrush(short x, short y, byte color)
{
  byte old_color;
  if (Main_backups->Pages->Layermode_flags == 2 && Main_current_layer < 4)
  {
    // Flood-fill the enclosing area
    if (x<Main_image_width && y<Main_image_height && x>= 0 && y >= 0
        && (color=Effect_function(x,y,color)) != (old_color=Read_pixel_from_current_layer(x,y))
        && (!((Stencil_mode) && (Stencil[old_color])))
        && (!((Mask_mode)    && (Mask_table[Read_pixel_from_spare_screen(x,y)])))
       )
    {
      short min_x,width,min_y,height;
      short xx,yy;

      // determine area
      switch(Main_current_layer)
      {
        case 0:
        default:
          // Full layer
          min_x=0;
          min_y=0;
          width=Main_image_width;
          height=Main_image_height;
          break;
        case 1:
        case 2:
          // Line
          min_x=0;
          min_y=y;
          width=Main_image_width;
          height=1;
          break;
        case 3:
          // Segment
          min_x=x / 48 * 48;
          min_y=y;
          width=48;
          height=1;
          break;
          //case 4:
          //  // 8x8
          //  min_x=x / 8 * 8;
          //  min_y=y / 8 * 8;
          //  width=8;
          //  height=8;
          //  break;
      }
      // Clip the bottom edge.
      // (Necessary if image height is not a multiple)
      if (min_y+height>=Main_image_height)
        height=Main_image_height-min_y;
      // Clip the right edge.
      // (Necessary if image width is not a multiple)
      if (min_x+width>=Main_image_width)
        width=Main_image_width-min_x;

      for (yy=min_y; yy<min_y+height; yy++)
        for (xx=min_x; xx<min_x+width; xx++)
        {
          Pixel_in_current_screen(xx,yy,color,0);
        }
      // Feedback
      // This part is greatly taken from Hide_paintbrush()
      Compute_clipped_dimensions(&min_x,&min_y,&width,&height);

      if ( (width>0) && (height>0) )
        Clear_brush(min_x-Main_offset_X,
            min_y-Main_offset_Y,
            0,0,
            width,height,0,
            Main_image_width);

      if (Main_magnifier_mode != 0)
      {
        Compute_clipped_dimensions_zoom(&min_x,&min_y,&width,&height);
        xx=min_x;
        yy=min_y;

        if ( (width>0) && (height>0) )
        {
          // Corrections dues au Zoom:
          min_x=(min_x-Main_magnifier_offset_X)*Main_magnifier_factor;
          min_y=(min_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
          height=min_y+(height*Main_magnifier_factor);
          if (height>Menu_Y)
            height=Menu_Y;

          Clear_brush_scaled(Main_X_zoom+min_x,min_y,
              xx,yy,
              width,height,0,
              Main_image_width,
              Horizontal_line_buffer);
        }
      }
      // End of graphic feedback
    }
    return 1;
  }
  return 0;
}

extern T_Bitmap Main_visible_image;
extern T_Bitmap Main_visible_image_depth_buffer;
int CPC_Mode5_RedrawLayeredImage()
{
  byte layer = 0;
  if (Main_backups->Pages->Layermode_flags == 2 && Main_layers_visible & (1<<4))
  {
    // The raster result layer is visible: start there
    // Copy it in Main_visible_image
    int i;
    for (i=0; i< Main_image_width*Main_image_height; i++)
    {
      layer = *(Main_backups->Pages->Image[4]+i);
      Main_visible_image.Image[i]=*(Main_backups->Pages->Image[layer]+i);
    }
      
    // Copy it to the depth buffer
    memcpy(Main_visible_image_depth_buffer.Image,
      Main_backups->Pages->Image[4],
      Main_image_width*Main_image_height);
      
    // Next
    layer= (1<<4)+1;
    return layer;
  }
  return -1;
}

int CPC_Mode5_ReadPixel(word x, word y)
{
  if (Main_backups->Pages->Layermode_flags == 2 && Main_current_layer==4)
    return *(Main_backups->Pages->Image[Main_current_layer] + x+y*Main_image_width);
  else return -1;
}

byte CPC_Mode5_PutPixel(word x, word y, byte color, int with_preview)
{
    if (Main_current_layer == 4)
    {
      if (color<4)
      {
        // Paste in layer
        *(Main_backups->Pages->Image[Main_current_layer] + x+y*Main_image_width)=color;
        // Paste in depth buffer
        *(Main_visible_image_depth_buffer.Image+x+y*Main_image_width)=color;
        // Fetch pixel color from the target raster layer
        color=*(Main_backups->Pages->Image[color] + x+y*Main_image_width);
        // Draw that color on the visible image buffer
        *(x+y*Main_image_width+Main_screen)=color;

        if (with_preview)
          Pixel_preview(x,y,color);
      }
      return 1;
    }
    else if (Main_current_layer<4 && (Main_layers_visible & (1<<4)))
    {
      byte depth;

      // Paste in layer
      *(Main_backups->Pages->Image[Main_current_layer] + x+y*Main_image_width)=color;
      // Search depth
      depth = *(Main_backups->Pages->Image[4] + x+y*Main_image_width);

      if ( depth == Main_current_layer)
      {
        // Draw that color on the visible image buffer
        *(x+y*Main_image_width+Main_screen)=color;

        if (with_preview)
          Pixel_preview(x,y,color);
      }
      return 1;
    }
    return 0;
}

const T_Constraint_Enforcer ConstraintsCPC = {
  CPC_Mode5_DisplayPaintbrush,
  CPC_Mode5_RedrawLayeredImage,
  CPC_Mode5_ReadPixel,
  CPC_Mode5_PutPixel,
  CPC_Mode5_InitMode
};


