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
///@file brush.h
/// Actions on the brush.
//////////////////////////////////////////////////////////////////////////////

#ifndef __BRUSH_H_
#define __BRUSH_H_

#include "struct.h"

/*!
    Gets the brush from the picture.
    @param start_x left edge coordinate in the picture
    @param start_y upper edge coordinate in the picture
    @param end_x right edge coordinate in the picture
    @param end_y bottom edge coordinate in the picture
    @param clear If 1, the area is also cleared from the picture.
*/
void Capture_brush(short start_x,short start_y,short end_x,short end_y,short clear);

/*!
  Rotates the brush to the right.
*/
void Rotate_90_deg(void);

/*!
    Stretch the brush to fit the given rectangle.
*/
void Stretch_brush(short x1, short y1, short x2, short y2);

/*!
    Stretch the brush to fit the given rectangle.
    Uses fast approximation for the preview while drawing the rectangle on screen.
*/
void Stretch_brush_preview(short x1, short y1, short x2, short y2);

/*!
    Rotates the brush to the right from the given angle.
*/
void Rotate_brush(float angle);

/*!
    Stretch the brush to fit the given rectangle.
    Uses fast approximation for the preview while changing the angle.
*/
void Rotate_brush_preview(float angle);

/*!
    Remap the brush palette to the nearest color in the picture one.
    Used when switching to the spare page.
*/

/*!
    Distort the brush on the screen.
*/
void Distort_brush_preview(short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4);

/*!
    Replace the brush by a distorted version of itself.
*/
void Distort_brush(short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4);


void Remap_brush(void);

/*!
    Get color indexes used by the brush.
*/
void Get_colors_from_brush(void);

/*!
    Outline the brush, add 1 foreground-colored pixel on the edges.
    Edges are detected considering the backcolor as transparent.
*/
void Outline_brush(void);

/*!
    Nibble the brush, remove 1 pixel on the edges and make it transparent (ie filled with back color).
    Edges are detected considering the backcolor as transparent.
*/
void Nibble_brush(void);

/*!
    Get brush from picture according to a freehand form.
    @param vertices number of points in the freehand form
    @param points array of points coordinates
    @param clear If set to 1, the captured area is also cleared from the picture.
*/
void Capture_brush_with_lasso(int vertices, short * points,short clear);


/*!
 \brief Allocates memory for a brush size change.

 This function can return the old brush pixels which can then be used by the
 caller to fill the new brush. Data is not automatically copied or converted
 from the old brush to the new one here.

 @param new_brush_width: new width of the brush
 @param new_brush_height: new height of the brush
 @param new_brush: Optionally, you can provide an already allocated new
        brush - otherwise, this function performs the allocation.
 @param old_brush: If the caller passes NULL, this function will free the old
        pixel data. If the caller provides the address of a (free) byte
        pointer, the function will make it point to the original pixel data,
        in this case it will be the caller's responsibility to free() it
        (after transferring pixels to Brush, usually).

 @return 0 on success, non-zero when running out of memory.
*/
byte Realloc_brush(word new_brush_width, word new_brush_height, byte *new_brush, byte **old_brush);

/// Sets brush's original palette and color mapping.
void Brush_set_palette(T_Palette *palette);

void Begin_brush_rotation(void);
void End_brush_rotation(void);

#endif
