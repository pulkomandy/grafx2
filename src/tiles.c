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
#include "tiles.h"


/// Build the tile-area from the current picture 
void build_tile_area()
{
	word tileAreaWidth = Main_image_width / Snap_width + 1;
	word tileAreaHeight = Main_image_height / Snap_height + 1;

	int* tileArea = malloc(tileAreaWidth*tileAreaHeight*sizeof(int));

	word tile_x, tile_y;

	// For each tile, we have to crawl up to the top of the picture and
	// find the first identical tile
	for (tile_y = 0; tile_y < tileAreaWidth; tile_y++)
		for(tile_x = 0; tile_x < tileAreaHeight; tile_x++)
		{
			// So, this is the "for each tile"
			short ctx, cty;
			// First we compare the tile with the others one in the same line at the left
			for(ctx = tile_x - 1; ctx >= 0; ctx--)
			if(compare_tiles(tile_x*Snap_width, tile_y*Snap_height, ctx*Snap_width, tile_y*Snap_height))
			{
				// We found a match !
				tileArea[tile_y*tileAreaWidth+tile_x] = tile_y*tileAreaWidth+ctx;
				goto found;
			}

			// Then we look at all the lines above
			for(cty = tile_y - 1; cty >= 0; cty--)
			for(ctx = tileAreaWidth - 1; ctx >= 0; ctx--)
			if(compare_tiles(tile_x*Snap_width, tile_y*Snap_height, ctx*Snap_width, cty*Snap_height))
			{
				// We found a match !
				tileArea[tile_y*tileAreaWidth+tile_x] = cty*tileAreaWidth+ctx;
				goto found;
			}

			// Then we look at all the lines below
			for(cty = tileAreaHeight - 1; cty >= tile_y; cty--)
			for(ctx = tileAreaWidth - 1; ctx >= 0; ctx--)
			if(compare_tiles(tile_x*Snap_width, tile_y*Snap_height, ctx*Snap_width, cty*Snap_height))
			{
				// We found a match !
				tileArea[tile_y*tileAreaWidth+tile_x] = cty*tileAreaWidth+ctx;
				goto found;
			}
			
			// Then we look at the tiles at the right of this one
			// (including the tile itself, so we are sure we match something this time)
			for(ctx = tileAreaHeight; ctx >= tile_x; ctx--)
			if(compare_tiles(tile_x*Snap_width, tile_y*Snap_height, ctx*Snap_width, tile_y*Snap_height))
			{
				// We found a match !
				tileArea[tile_y*tileAreaWidth+tile_x] = tile_y*tileAreaWidth+ctx;
			}
			
      found:
      ;
		}
}


// Compare tiles
// The parameters are in pixel-space.
int compare_tiles(word x1, word y1, word x2, word y2)
{
	word pixel_x, pixel_y;
	byte c1, c2;

	for (pixel_y = 0; pixel_y < Snap_width; pixel_y++)
	for (pixel_x = 0; pixel_x < Snap_height; pixel_x++)
	{
		c1 = *(Main_screen + (y1+pixel_y) * Main_image_width + (x1+pixel_x));
		c2 = *(Main_screen + (y2+pixel_y) * Main_image_width + (x2+pixel_x));
		if (c1 != c2) return 0;
	}

	return 1;
}
/*
/// Copy a tile pixeldata to all the identical ones
// Call this after the end of an operation
void update_tile(word pixel_x, word pixel_y)
{
	int tileOffset = (pixel_y/Snap_height)*tileAreaHeight + pixel_x/Snap_width;
	int firstTileOffset = tileOffset + 1; // to make sure they are not equal

	while(firstTileOffset != tileOffset)
	{
		tileOffset = tileArea[tileOffset];
		
		//do the copy of a block starting at (pixel_x, pixel_y)
	}
}
*/

/* Basic repeat-all
void Tilemap_draw(word x, word y, byte color)
{
  int xx,yy;
  for (yy=(y+Snap_height-Snap_offset_Y)%Snap_height+Snap_offset_Y;yy<Main_image_height;yy+=Snap_height)
    for (xx=(x+Snap_width-Snap_offset_X)%Snap_width+Snap_offset_X;xx<Main_image_width;xx+=Snap_width)
      Pixel_in_current_screen(xx,yy,color,yy>=Limit_top&&yy<=Limit_bottom&&xx>=Limit_left&&xx<=Limit_right);
  Update_rect(0,0,0,0);
}
*/

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
    xx = (tile % Main_tilemap_width)*Snap_width+Snap_offset_X + rel_x;
    yy = (tile / Main_tilemap_width)*Snap_height+Snap_offset_Y + rel_y;
    Pixel_in_current_screen(xx,yy,color,yy>=Limit_top&&yy<=Limit_bottom&&xx>=Limit_left&&xx<=Limit_right);
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

/// Create or update a tilemap based on current screen pixels.
void Tilemap_create(void)
{
  int width;
  int height;
  int tile;
  T_Tile * tile_ptr;

  width=(Main_image_width-Snap_offset_X)/Snap_width;
  height=(Main_image_height-Snap_offset_Y)/Snap_height;
  
  if (width<1 || height<1 || width*height>1000000l
   || (tile_ptr=(T_Tile *)malloc(width*height*sizeof(T_Tile))) == NULL)
  {
    // Cannot enable tilemap because either the image is too small
    // for the grid settings (and I don't want to implement partial tiles)
    // Or the number of tiles seems unreasonable (one million) : This can
    // happen if you set grid 1x1 for example.
  
    if (Main_tilemap)
    {
      // Recycle existing tilemap
      free(Main_tilemap);
      Main_tilemap=NULL;
    }
    Main_tilemap_width=0;
    Main_tilemap_height=0;
    Tilemap_mode=0;
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

  // Init array with all tiles unique by default
  for (tile=0; tile<width*height; tile++)
  {
    Main_tilemap[tile].Previous = tile;
    Main_tilemap[tile].Next = tile;
  }
  
  // Now find similar tiles and link them in circular linked list
  //It will be used to modify all tiles whenever you draw on one.
  for (tile=1; tile<width*height; tile++)
  {
    int ref_tile=0;
    while(1)
    {
      if (Main_tilemap[ref_tile].Previous<=ref_tile && Tile_is_same(ref_tile, tile))
      {
        // Insert at the end. classic doubly-linked-list.
        int last_tile=Main_tilemap[ref_tile].Previous;
        Main_tilemap[tile].Previous=last_tile;
        Main_tilemap[tile].Next=ref_tile;
        Main_tilemap[ref_tile].Previous=tile;
        Main_tilemap[last_tile].Next=tile;
        
        break;
      }
      // next
      ref_tile++;

      // end of loop without finding a match
      if (ref_tile>=tile)
      {
        // This tile is really unique.
        // Nothing to do at the moment
        break;
      }
    }
    
  }
  
}