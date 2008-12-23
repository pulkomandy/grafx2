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

void Capturer_brosse(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,short Effacement);
void Rotate_90_deg(void);
void Etirer_brosse(short X1, short Y1, short X2, short Y2);
void Etirer_brosse_preview(short X1, short Y1, short X2, short Y2);
void Tourner_brosse(float Angle);
void Tourner_brosse_preview(float Angle);

void Remap_brosse(void);
void Get_colors_from_brush(void);
void Outline_brush(void);
void Nibble_brush(void);
void Capturer_brosse_au_lasso(int Vertices, short * Points,short Effacement);

#endif
