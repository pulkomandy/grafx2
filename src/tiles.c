/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Yves Rizoud
    Copyright 2010-2011 Adrien Destugues

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

#include "struct.h"
#include "global.h"
#include "graph.h"
#include "sdlscreen.h"
#include "engine.h"
#include "windows.h"
#include "input.h"
#include "misc.h"
#include "tiles.h"

// These helpers are only needed internally at the moment
#define TILE_FOR_COORDS(x,y) (((y)-Snap_offset_Y)/Snap_height*Main_tilemap_width+((x)-Snap_offset_X)/Snap_width)
#define TILE_AT(x,y) (y)*Main_tilemap_width+(x)
#define TILE_X(t) (((t)%Main_tilemap_width)*Snap_width+Snap_offset_X)
#define TILE_Y(t) (((t)/Main_tilemap_width)*Snap_height+Snap_offset_Y)

enum TILE_FLIPPED
{
  TILE_FLIPPED_NONE = 0,
  TILE_FLIPPED_X = 1,
  TILE_FLIPPED_Y = 2,
  TILE_FLIPPED_XY = 3, // needs be TILE_FLIPPED_X|TILE_FLIPPED_Y
};

// globals

/// Tilemap for the main screen
T_Tile * Main_tilemap;
/// Number of tiles (horizontally) for the main page's tilemap
short Main_tilemap_width;
/// Number of tiles (vertically) for the main page's tilemap
short Main_tilemap_height;

/// Tilemap for the spare
T_Tile * Spare_tilemap;
/// Number of tiles (horizontally) for the spare page's tilemap
short Spare_tilemap_width;
/// Number of tiles (vertically) for the spare page's tilemap
short Spare_tilemap_height;


///
/// Draw a pixel while Tilemap mode is active : This will paint on all
/// similar tiles of the layer, visible on the screen or not.
void Tilemap_draw(word x, word y, byte color)
{
  int tile, first_tile;
  int rel_x, rel_y;
  
  if (x < Snap_offset_X
   || y < Snap_offset_Y
   || x >= Snap_offset_X + Main_tilemap_width*Snap_width
   || y >= Snap_offset_Y + Main_tilemap_height*Snap_height)
    return;
  
  tile = first_tile = TILE_FOR_COORDS(x,y);
  
  rel_x = (x - Snap_offset_X + Snap_width) % Snap_width;
  rel_y = (y - Snap_offset_Y + Snap_height) % Snap_height;
  do
  {
    int xx,yy;
    switch(Main_tilemap[tile].Flipped ^ Main_tilemap[first_tile].Flipped)
    {
      case 0: // no
      default:
        xx = (tile % Main_tilemap_width)*Snap_width+Snap_offset_X + rel_x;
        yy = (tile / Main_tilemap_width)*Snap_height+Snap_offset_Y + rel_y;
        break;
      case 1: // horizontal
        xx = (tile % Main_tilemap_width)*Snap_width+Snap_offset_X + Snap_width-rel_x-1;
        yy = (tile / Main_tilemap_width)*Snap_height+Snap_offset_Y + rel_y;
        break;
      case 2: // vertical
        xx = (tile % Main_tilemap_width)*Snap_width+Snap_offset_X + rel_x;
        yy = (tile / Main_tilemap_width)*Snap_height+Snap_offset_Y + Snap_height - rel_y - 1;
        break;
      case 3: // both
        xx = (tile % Main_tilemap_width)*Snap_width+Snap_offset_X + Snap_width-rel_x-1;
        yy = (tile / Main_tilemap_width)*Snap_height+Snap_offset_Y + Snap_height - rel_y - 1;
        break;
    }
    if (yy>=Limit_top&&yy<=Limit_bottom&&xx>=Limit_left&&xx<=Limit_right)
      Pixel_in_current_screen_with_preview(xx,yy,color);
    else
      Pixel_in_current_screen(xx,yy,color);
      
    tile = Main_tilemap[tile].Next;
  } while (tile != first_tile);

  Update_rect(0,0,0,0);
}

///
int Tile_is_same(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y;
  
  bmp1 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t1))*Main_image_width+(TILE_X(t1));
  bmp2 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t2))*Main_image_width+(TILE_X(t2));
  
  for (y=0; y < Snap_height; y++)
  {
    if (memcmp(bmp1+y*Main_image_width, bmp2+y*Main_image_width, Snap_width))
      return 0;
  }
  return 1;
}

///
int Tile_is_same_flipped_x(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y, x;
  
  bmp1 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t1))*Main_image_width+(TILE_X(t1));
  bmp2 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t2))*Main_image_width+(TILE_X(t2)+Snap_width-1);
  
  for (y=0; y < Snap_height; y++)
  {
    for (x=0; x < Snap_width; x++)
      if (*(bmp1+y*Main_image_width+x) != *(bmp2+y*Main_image_width-x))
        return 0;
  }
  return 1;
}

///
int Tile_is_same_flipped_y(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y;
  
  bmp1 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t1))*Main_image_width+(TILE_X(t1));
  bmp2 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t2)+Snap_height-1)*Main_image_width+(TILE_X(t2));
  
  for (y=0; y < Snap_height; y++)
  {
    if (memcmp(bmp1+y*Main_image_width, bmp2-y*Main_image_width, Snap_width))
      return 0;
  }
  return 1;
}


///
int Tile_is_same_flipped_xy(int t1, int t2)
{
  byte *bmp1,*bmp2;
  int y, x;
  
  bmp1 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t1))*Main_image_width+(TILE_X(t1));
  bmp2 = Main_backups->Pages->Image[Main_current_layer].Pixels+(TILE_Y(t2)+Snap_height-1)*Main_image_width+(TILE_X(t2)+Snap_width-1);
  
  for (y=0; y < Snap_height; y++)
  {
    for (x=0; x < Snap_width; x++)
      if (*(bmp1+y*Main_image_width+x) != *(bmp2-y*Main_image_width-x))
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

  if (!Main_tilemap_mode)
    return;
  
  width=(Main_image_width-Snap_offset_X)/Snap_width;
  height=(Main_image_height-Snap_offset_Y)/Snap_height;
  
  if (width<1 || height<1 || width*height>1000000l
   || (tile_ptr=(T_Tile *)malloc(width*height*sizeof(T_Tile))) == NULL)
  {
    // Cannot enable tilemap because either the image is too small
    // for the grid settings (and I don't want to implement partial tiles)
    // Or the number of tiles seems unreasonable (one million) : This can
    // happen if you set grid 1x1 for example.
  
    Disable_main_tilemap();
    return;
  }
  
  if (Main_tilemap)
  {
    // Recycle existing tilemap
    free(Main_tilemap);
    Main_tilemap=NULL;
  }
  Main_tilemap=tile_ptr;
  
  Main_tilemap_width=width;
  Main_tilemap_height=height;

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
    Main_tilemap[tile].Previous = tile;
    Main_tilemap[tile].Next = tile;
    Main_tilemap[tile].Flipped = 0;
  }
  
  // Now find similar tiles and link them in circular linked list
  //It will be used to modify all tiles whenever you draw on one.
  for (tile=1; tile<width*height; tile++)
  {
    int ref_tile;
    
    // Try normal comparison
    
    for (ref_tile=0; ref_tile<tile; ref_tile++)
    {
      if (Main_tilemap[ref_tile].Previous<=ref_tile && Tile_is_same(ref_tile, tile))
      {
        break; // stop loop on ref_tile
      }
    }
    if (ref_tile<tile)
    {
      // New occurence of a known tile
      // Insert at the end. classic doubly-linked-list.
      int last_tile=Main_tilemap[ref_tile].Previous;
      Main_tilemap[tile].Previous=last_tile;
      Main_tilemap[tile].Next=ref_tile;
      Main_tilemap[tile].Flipped=Main_tilemap[ref_tile].Flipped;
      Main_tilemap[ref_tile].Previous=tile;
      Main_tilemap[last_tile].Next=tile;
      continue; // next tile
    }
    
    // Try flipped-y comparison
    if (Config.Tilemap_allow_flipped_y)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main_tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_y(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main_tilemap[ref_tile].Previous;
        Main_tilemap[tile].Previous=last_tile;
        Main_tilemap[tile].Next=ref_tile;
        Main_tilemap[tile].Flipped=Main_tilemap[ref_tile].Flipped ^ TILE_FLIPPED_Y;
        Main_tilemap[ref_tile].Previous=tile;
        Main_tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // Try flipped-x comparison
    if (Config.Tilemap_allow_flipped_x)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main_tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_x(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main_tilemap[ref_tile].Previous;
        Main_tilemap[tile].Previous=last_tile;
        Main_tilemap[tile].Next=ref_tile;
        Main_tilemap[tile].Flipped=Main_tilemap[ref_tile].Flipped ^ TILE_FLIPPED_X;
        Main_tilemap[ref_tile].Previous=tile;
        Main_tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // Try flipped-xy comparison
    if (Config.Tilemap_allow_flipped_x && Config.Tilemap_allow_flipped_y)
    {
      for (ref_tile=0; ref_tile<tile; ref_tile++)
      {
        if (Main_tilemap[ref_tile].Previous<=ref_tile && Tile_is_same_flipped_xy(ref_tile, tile))
        {
          break; // stop loop on ref_tile
        }
      }
      if (ref_tile<tile)
      {
        // New occurence of a known tile
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main_tilemap[ref_tile].Previous;
        Main_tilemap[tile].Previous=last_tile;
        Main_tilemap[tile].Next=ref_tile;
        Main_tilemap[tile].Flipped=Main_tilemap[ref_tile].Flipped ^ TILE_FLIPPED_XY;
        Main_tilemap[ref_tile].Previous=tile;
        Main_tilemap[last_tile].Next=tile;
        continue; // next tile
      }
    }
    
    // This tile is really unique.
    // Nothing to do at this time: the initialization
    // has already set the right data for Main_tilemap[tile].
    count++;
  }
  
  if (wait_window)
  {
    char   str[8];
    Uint32 end;
    
    if (Config.Tilemap_show_count)
    {
      Num2str(count,str,7);
      Hide_cursor();
      Print_in_window(6, 20, "Unique tiles: ",MC_Black,MC_Light);
      Print_in_window(6+8*14,20,str,MC_Black,MC_Light);
      Display_cursor();
      
      // Wait a moment to display count
      end = SDL_GetTicks()+750;
      do
      {
        Get_input(20);
      } while (SDL_GetTicks()<end);
    }

    Close_window();
    Cursor_shape=old_cursor;
    Display_cursor();
  }
}

///
/// This exchanges the tilemap settings of the main and spare, it should
/// be called when swapping pages.
void Swap_tilemap(void)
{
  SWAP_BYTES(Main_tilemap_mode, Spare_tilemap_mode)
  {
    T_Tile * a;
    a=Main_tilemap;
    Main_tilemap=Spare_tilemap;
    Spare_tilemap=a;
  }
  SWAP_SHORTS(Main_tilemap_width, Spare_tilemap_width)
  SWAP_SHORTS(Main_tilemap_height, Spare_tilemap_height)
}

///
/// Clears all tilemap data and settings for the main page.
/// Safe to call again.
void Disable_main_tilemap(void)
{
  if (Main_tilemap)
  {
    // Recycle existing tilemap
    free(Main_tilemap);
    Main_tilemap=NULL;
  }
  Main_tilemap_width=0;
  Main_tilemap_height=0;
  Main_tilemap_mode=0;
}

///
/// Clears all tilemap data and settings for the spare.
/// Safe to call again.
void Disable_spare_tilemap(void)
{
    if (Spare_tilemap)
  {
    // Recycle existing tilemap
    free(Spare_tilemap);
    Spare_tilemap=NULL;
  }
  Spare_tilemap_width=0;
  Spare_tilemap_height=0;
  Spare_tilemap_mode=0;
}
