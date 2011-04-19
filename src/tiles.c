/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2010 Adrien Destugues

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

/// \file Handle tiles.

/// Build the tile-area from the current picture 
void build_tile_area()
{
	word tileAreaWidth = Main_image_width / Snap_width + 1;
	word tileAreaHeight = Main_image_height / Snap_height + 1;

	int* tileArea = malloc(tileAreaWidth*tileAreaHeight*sizeof(int));

	word tile_x, tile_y, pixel_x, pixel_y;

	// For each tile, we have to crawl up to the top of the picture and
	// find the first identical tile
	for (tile_y = 0; tile_y < tileAreaWidth; tile_y++)
		for(tile_x = 0; tile_x < tileAreaHeight; tile_x++)
		{
			// So, this is the "for each tile"
			word ctx, cty;
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
		}
}


// Compare tiles
// The parameters are in pixel-space.
void compare_tiles(word x1, word y1, word x2, word y2)
{
	word pixel_x, pixel_y;
	byte c1, c2;

	for (pixel_y = 0; pixel_y < Snap_width; pixel_y++)
	for (pixel_x = 0; pixel_x < Snap_height; pixel_x++)
	{
		c1 = Main_screen + (y1+pixel_y) * Main_image_width + (x1+pixel_x);
		c2 = Main_screen + (y2+pixel_y) * Main_image_width + (x2+pixel_x);
		if (c1 != c2) return 0;
	}

	return 1;
}

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
