/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

********************************************************************************

    24bit RGB to 8bit indexed functions
*/

#include "struct.h"

/* Octree for mapping RGB to color. A bit slower than a plain conversion table in theory,
but :
  * Faster than running a search in the palette
  * Takes less memory than the huge conversion table
  * No loss of precision
*/

#ifndef __COLORRED_H
#define __COLORRED_H

typedef struct CT_Node_s
{
	// min
	byte Rmin;
	byte Gmin;
	byte Bmin;
	
	// max
	byte Rmax;
	byte Gmax;
	byte Bmax;
	
	// palette index (valid iff any child is NULL)
	byte index;
	
	// child nodes
	struct CT_Node_s* children[8];
	//rgb rgB rGb rGB Rgb RgB RGb RGB
} CT_Node;

CT_Node* CT_new();
void CT_delete(CT_Node* t);
byte CT_get(CT_Node* t,byte r,byte g,byte b);
void CT_set(CT_Node* colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index);

#endif
