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

/// Create or update a tilemap based on current screen pixels.
void Tilemap_update(void);

///
/// Draw a pixel while Tilemap mode is active : This will paint on all
/// similar tiles of the layer, visible on the screen or not.
void Tilemap_draw(word x, word y, byte color);

///
/// This exchanges the tilemap settings of the main and spare, it should
/// be called when swapping pages.
void Swap_tilemap(void);

///
/// Clears all tilemap data and settings for the main page.
/// Safe to call again.
void Disable_main_tilemap(void);

///
/// Clears all tilemap data and settings for the spare.
/// Safe to call again.
void Disable_spare_tilemap(void);


/// Tilemap for the main screen
extern T_Tile * Main_tilemap;
/// Number of tiles (horizontally) for the main page's tilemap
extern short Main_tilemap_width;
/// Number of tiles (vertically) for the main page's tilemap
extern short Main_tilemap_height;

/// Tilemap for the spare
extern T_Tile * Spare_tilemap;
/// Number of tiles (horizontally) for the spare page's tilemap
extern short Spare_tilemap_width;
/// Number of tiles (vertically) for the spare page's tilemap
extern short Spare_tilemap_height;
