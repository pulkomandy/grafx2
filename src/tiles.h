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
/// Clears all tilemap data and settings
/// Safe to call again.
void Disable_tilemap(T_Document * doc);

