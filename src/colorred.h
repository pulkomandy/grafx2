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

#include "struct.h"

/**
 * @file colorred.h
 * 24bit RGB to 8bit indexed functions
 *
 * Octree for mapping RGB to color. A bit slower than a plain conversion table in theory,
but :
 * Faster than running a search in the palette
 * Takes less memory than the huge conversion table
 * No loss of precision
 */

#ifndef __COLORRED_H
#define __COLORRED_H

/**
 * Color Tree node
 */
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
	
	// possible optimization: a cluster has either two childs or a color index.
	// if the first child is NULL, then the other can be used to store the index
	// when the tree is being built, a node may have child0 set and not child1, but not the reverse)
	
	// possible optimization: there can't be more than 511 clusters in the tree
	// for a 256 color picture, so use int16 as pointers and store everything in a table :
	// * makes them smaller
	// * helps with cache locality
	
	// Child nodes :
	// Either two indices in the colorTree array, or
	// 0 and a palette index
	// 0 is not a valid array index, because no node points to the root !
	word children[2];
} CT_Node;

/**
 * Color Tree
 */
typedef struct ColorTree_S {
	short nodecount;
	CT_Node nodes[511];
} CT_Tree;

CT_Tree* CT_new();
void CT_delete(CT_Tree* t);
byte CT_get(CT_Tree* t,byte r,byte g,byte b);
void CT_set(CT_Tree* colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index);

#endif
