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

#include <stdlib.h>
#include <string.h>

#include "struct.h"
#include "global.h"
#include "graph.h"
#include "screen.h"
#include "engine.h"
#include "windows.h"
#include "input.h"
#include "misc.h"
#include "osdep.h"
#include "tiles.h"

// These helpers are only needed internally at the moment
#define TILE_FOR_COORDS(x,y) (((y)-Snap_offset_Y)/Snap_height*Main.tilemap_width+((x)-Snap_offset_X)/Snap_width)
#define TILE_AT(x,y) (y)*Main.tilemap_width+(x)
#define TILE_X(t) (((t)%Main.tilemap_width)*Snap_width+Snap_offset_X)
#define TILE_Y(t) (((t)/Main.tilemap_width)*Snap_height+Snap_offset_Y)

enum TILE_FLIPPED
{
  TILE_FLIPPED_NONE = 0,
  TILE_FLIPPED_X = 1,
  TILE_FLIPPED_Y = 2,
  TILE_FLIPPED_XY = 3, // needs be TILE_FLIPPED_X|TILE_FLIPPED_Y
};

// globals

///
/// Draw a pixel while Tilemap mode is active : This will paint on all
/// similar tiles of the layer, visible on the screen or not.
void Tilemap_draw(word x, word y, byte color)
{
  int tile, first_tile;
  int rel_x, rel_y;
  
  if (x < Snap_offset_X
   || y < Snap_offset_Y
   || x >= Snap_offset_X + Main.tilemap_width*Snap_width
   || y >= Snap_offset_Y + Main.tilemap_height*Snap_height)
    return;
  
  tile = first_tile = TILE_FOR_COORDS(x,y);
  
  rel_x = (x - Snap_offset_X + Snap_width) % Snap_width;
  rel_y = (y - Snap_offset_Y + Snap_height) % Snap_height;
  do
  {
    int xx,yy;
    switch(Main.tilemap[tile].Flipped ^ Main.tilemap[first_tile].Flipped)
    {
      case 0: // no
      default:
        xx = (tile % Main.tilemap_width)*Snap_width+Snap_offset_X + rel_x;
        yy = (tile / Main.tilemap_width)*Snap_height+Snap_offset_Y + rel_y;
        break;
      case 1: // horizontal
        xx = (tile % Main.tilemap_width)*Snap_width+Snap_offset_X + Snap_width-rel_x-1;
        yy = (tile / Main.tilemap_width)*Snap_height+Snap_offset_Y + rel_y;
        break;
      case 2: // vertical
        xx = (tile % Main.tilemap_width)*Snap_width+Snap_offset_X + rel_x;
        yy = (tile / Main.tilemap_width)*Snap_height+Snap_offset_Y + Snap_height - rel_y - 1;
        break;
      case 3: // both
        xx = (tile % Main.tilemap_width)*Snap_width+Snap_offset_X + Snap_width-rel_x-1;
        yy = (tile / Main.tilemap_width)*Snap_height+Snap_offset_Y + Snap_height - rel_y - 1;
        break;
    }
    if (yy>=Limit_top&&yy<=Limit_bottom&&xx>=Limit_left&&xx<=Limit_right)
      Pixel_in_current_screen_with_preview(xx,yy,color);
    else
      Pixel_in_current_screen(xx,yy,color);
      
    tile = Main.tilemap[tile].Next;
  } while (tile != first_tile);

  Update_rect(0,0,0,0);
}

///
int Tile_is_same(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y;
  
  bmp1 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t1))*Main.image_width+(TILE_X(t1));
  bmp2 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t2))*Main.image_width+(TILE_X(t2));
  
  for (y=0; y < Snap_height; y++)
  {
    if (memcmp(bmp1+y*Main.image_width, bmp2+y*Main.image_width, Snap_width))
      return 0;
  }
  return 1;
}

///
int Tile_is_same_flipped_x(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y, x;
  
  bmp1 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t1))*Main.image_width+(TILE_X(t1));
  bmp2 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t2))*Main.image_width+(TILE_X(t2)+Snap_width-1);
  
  for (y=0; y < Snap_height; y++)
  {
    for (x=0; x < Snap_width; x++)
      if (*(bmp1+y*Main.image_width+x) != *(bmp2+y*Main.image_width-x))
        return 0;
  }
  return 1;
}

///
int Tile_is_same_flipped_y(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y;
  
  bmp1 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t1))*Main.image_width+(TILE_X(t1));
  bmp2 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t2)+Snap_height-1)*Main.image_width+(TILE_X(t2));
  
  for (y=0; y < Snap_height; y++)
  {
    if (memcmp(bmp1+y*Main.image_width, bmp2-y*Main.image_width, Snap_width))
      return 0;
  }
  return 1;
}


///
int Tile_is_same_flipped_xy(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y, x;
  
  bmp1 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t1))*Main.image_width+(TILE_X(t1));
  bmp2 = Main.backups->Pages->Image[Main.current_layer].Pixels+(TILE_Y(t2)+Snap_height-1)*Main.image_width+(TILE_X(t2)+Snap_width-1);
  
  for (y=0; y < Snap_height; y++)
  {
    for (x=0; x < Snap_width; x++)
      if (*(bmp1+y*Main.image_width+x) != *(bmp2-y*Main.image_width-x))
        return 0;
  }
  return 1;
}

/// Create or update a tilemap based on current screen (layer)'s pixels.
void Tilemap_update(void)
{
  int width;
  int height;
  int tile;
  int count=1;
  T_Tile * tile_ptr;
  
  int wait_window=0;
  byte old_cursor=0;

  if (!Main.tilemap_mode)
    return;
  
  width=(Main.image_width-Snap_offset_X)/Snap_width;
  height=(Main.image_height-Snap_offset_Y)/Snap_height;
  
  if (width<1 || height<1 || width*height>1000000l
   || (tile_ptr=(T_Tile *)malloc(width*height*sizeof(T_Tile))) == NULL)
  {
    // Cannot enable tilemap because either the image is too small
    // for the grid settings (and I don't want to implement partial tiles)
    // Or the number of tiles seems unreasonable (one million) : This can
    // happen if you set grid 1x1 for example.
  
    Disable_tilemap(&Main);
    return;
  }
  
  if (Main.tilemap)
  {
    // Recycle existing tilemap
    free(Main.tilemap);
    Main.tilemap=NULL;
  }
  Main.tilemap=tile_ptr;
  
  Main.tilemap_width=width;
  Main.tilemap_height=height;

  if (width*height > 1000 || Config.Tilemap_show_count)
  {
    wait_window=1;
    old_cursor=Cursor_shape;
    Open_window(180,36,"Creating tileset");
    Print_in_window(26, 20, "Please wait...",MC_Black,MC_Light);
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Update_window_area(0,0,Window_width, Window_height);
    Display_cursor();
    Get_input(0);
  }
  
  // Initialize array with all tiles unique by default
  for (tile=0; tile<width*height; tile++)
  {
    Main.tilemap[tile].Previous = tile;
    Main.tilemap[tile].Next = tile;
    Main.tilemap[tile].Flipped = 0;
  }
  
  // Now find similar tiles and link them in circular linked list
  //It will be used to modify all tiles whenever you draw on one.
  for (tile=1; tile<width*height; tile++)
  {
    int ref_tile;
    
    // Try normal comparison
    
    for (ref_tile=0; ref_tile<tile; ref_tile++)
    {
      if (Main.tilemap[ref_tile].Previous<=ref_tile && Tile_is_same(ref_tile, tile))
      {
        break; // stop loop on ref_tile
      }
    }
    if (ref_tile<tile)
    {
      // New occurrence of a known tile
      // Insert at the end. classic doubly-linked-list.
      int last_tile=Main.tilemap[ref_tile].Previous;
      Main.tilemap[tile].Previous=last_tile;
      Main.tilemap[tile].Next=ref_tile;
      Main.tilemap[tile].Flipped=Main.tilemap[ref_tile].Flipped;
      Main.tilemap[ref_tile].Previous=tile;
      Main.tilemap[last_tile].Next=tile;
      continue; // next tile
    }
    
    // Try flipped-y comparison
    if (Config.Tilemap_allow_flipped_y)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main.tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_y(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurrence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main.tilemap[ref_tile].Previous;
        Main.tilemap[tile].Previous=last_tile;
        Main.tilemap[tile].Next=ref_tile;
        Main.tilemap[tile].Flipped=Main.tilemap[ref_tile].Flipped ^ TILE_FLIPPED_Y;
        Main.tilemap[ref_tile].Previous=tile;
        Main.tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // Try flipped-x comparison
    if (Config.Tilemap_allow_flipped_x)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main.tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_x(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurrence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main.tilemap[ref_tile].Previous;
        Main.tilemap[tile].Previous=last_tile;
        Main.tilemap[tile].Next=ref_tile;
        Main.tilemap[tile].Flipped=Main.tilemap[ref_tile].Flipped ^ TILE_FLIPPED_X;
        Main.tilemap[ref_tile].Previous=tile;
        Main.tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // Try flipped-xy comparison
    if (Config.Tilemap_allow_flipped_x && Config.Tilemap_allow_flipped_y)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main.tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_xy(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurrence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main.tilemap[ref_tile].Previous;
        Main.tilemap[tile].Previous=last_tile;
        Main.tilemap[tile].Next=ref_tile;
        Main.tilemap[tile].Flipped=Main.tilemap[ref_tile].Flipped ^ TILE_FLIPPED_XY;
        Main.tilemap[ref_tile].Previous=tile;
        Main.tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // This tile is really unique.
    // Nothing to do at this time: the initialization
    // has already set the right data for Main.tilemap[tile].
    count++;
  }
  
  if (wait_window)
  {
    char   str[8];
    dword end;
    
    if (Config.Tilemap_show_count)
    {
      Num2str(count,str,7);
      Hide_cursor();
      Print_in_window(6, 20, "Unique tiles: ",MC_Black,MC_Light);
      Print_in_window(6+8*14,20,str,MC_Black,MC_Light);
      Display_cursor();
      
      // Wait a moment to display count
      end = GFX2_GetTicks()+750;
      do
      {
        Get_input(20);
      } while (GFX2_GetTicks()<end);
    }

    Close_window();
    Cursor_shape=old_cursor;
    Display_cursor();
  }
}

///
/// Clears all tilemap data and settings
/// Safe to call again.
void Disable_tilemap(T_Document * doc)
{
  if (doc->tilemap)
  {
    // Recycle existing tilemap
    free(doc->tilemap);
    doc->tilemap=NULL;
  }
  doc->tilemap_width=0;
  doc->tilemap_height=0;
  doc->tilemap_mode=0;
}
