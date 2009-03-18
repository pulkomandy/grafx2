/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007-2008 Adrien Destugues
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __BRUSH_H_
#define __BRUSH_H_

/*!
    Gets the brush from the picture.
    @param Debut_X left edge coordinate in the picture
    @param Debut_Y upper edge coordinate in the picture
    @param Fin_X right edge coordinate in the picture
    @param Fin_Y bottom edge coordinate in the picture
    @param effacement If 1, the area is also cleared from the picture.
*/
void Capturer_brosse(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,short clear);

/*!
  Rotates the brush to the right.
*/
void Rotate_90_deg(void);

/*!
    Stretch the brush to fit the given rectangle.
*/
void Etirer_brosse(short X1, short Y1, short X2, short Y2);

/*!
    Stretch the brush to fit the given rectangle.
    Uses fast approximation for the preview while drawing the rectangle on screen.
*/
void Etirer_brosse_preview(short X1, short Y1, short X2, short Y2);

/*!
    Rotates the brush to the right from the given angle.
*/
void Tourner_brosse(float angle);

/*!
    Stretch the brush to fit the given rectangle.
    Uses fast approximation for the preview while changing the angle.
*/
void Tourner_brosse_preview(float angle);

/*!
    Remap the brush palette to the nearest color in the picture one.
    Used when switching to the spare page.
*/
void Remap_brosse(void);

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
    @param Vertices number of points in the freehand form
    @param Points array of points coordinates
    @param clear If set to 1, the captured area is also cleared from the picture.
*/
void Capturer_brosse_au_lasso(int Vertices, short * Points,short clear);

#endif
