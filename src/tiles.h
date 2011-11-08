/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Yves Rizoud
    Copyright 2011 Adrien Destugues

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

//////////////////////////////////////////////////////////////////////////////
///@file tiles.h
/// Functions for tilemap effect
//////////////////////////////////////////////////////////////////////////////

int compare_tiles(word x1, word y1, word x2, word y2);

/// Create or update a tilemap based on current screen pixels.
void Tilemap_create(void);

void Tilemap_draw(word x, word y, byte color);

#define TILE_FOR_COORDS(x,y) (((y)-Snap_offset_Y)/Snap_height*Main_tilemap_width+((x)-Snap_offset_X)/Snap_width)
#define TILE_AT(x,y) (y)*Main_tilemap_width+(x)
#define TILE_X(t) (((t)%Main_tilemap_width)*Snap_width+Snap_offset_X)
#define TILE_Y(t) (((t)/Main_tilemap_width)*Snap_height+Snap_offset_Y)

