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

********************************************************************************

    24bit RGB to 8bit indexed functions
*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "colorred.h"

/* Octree for mapping RGB to color. A bit slower than a plain conversion table in theory,
but :
  * Faster than running a search in the palette
  * Takes less memory than the huge conversion table
  * No loss of precision
*/

CT_Tree* CT_new() {return calloc(1, sizeof(CT_Tree));}

// debug helper
/*
void CT_Print(CT_Node* node)
{
	printf("R %d %d\tG %d %d\tB %d %d\tc %d/%d\n",
		node->Rmin, node->Rmax, node->Gmin, node->Gmax,
		node->Bmin, node->Bmax, node->children[0], node->children[1]);
}
*/

/**
 * insert a node in the color tree
 */
void CT_set(CT_Tree* colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index)
{
	CT_Node* parent;
	// Create and setup node
	CT_Node* node = &colorTree->nodes[colorTree->nodecount];
	
	node->Rmin = Rmin;
	node->Gmin = Gmin;
	node->Bmin = Bmin;
	node->Rmax = Rmax;
	node->Gmax = Gmax;
	node->Bmax = Bmax;
	node->children[1] = index;
	
	// Now insert it in tree (if we're not the root node)
	parent = &colorTree->nodes[0];
	if (colorTree->nodecount != 0) for(;;) {
		// Find where to insert ourselves
		
		// pre-condition: the parent we're looking at is a superset of the node we're inserting
		// it may have 0, 1, or 2 child
		
		// 0 child: insert as child 0
		// 1 child: either we're included in the child, and recurse, or we''re not, and insert at child 1
		// 2 child: one of them has to be a superset of the node.
    
		if (parent->children[0] == 0)
		{
			parent->children[0] = colorTree->nodecount;	
      // We KNOW children[1] was set to 0, because the parent was a split cluster.
			break;
		} else {
			CT_Node* child0 = &colorTree->nodes[parent->children[0]];
			if (child0->Rmin <= node->Rmin
				&& child0->Gmin <= node->Gmin
				&& child0->Bmin <= node->Bmin
				&& child0->Rmax >= node->Rmax
				&& child0->Gmax >= node->Gmax
				&& child0->Bmax >= node->Bmax
			) {
				parent = child0;
			} else if(parent->children[1] == 0)
			{
				parent->children[1] = colorTree->nodecount;	
				break;
			} else {
				parent = &colorTree->nodes[parent->children[1]];
			}
		}
	}

  ++colorTree->nodecount;
}

/**
 * find the leaf that also contains (rgb)
 *
 * pre condition: node contains (rgb)
 */
byte CT_get(CT_Tree* tree, byte r, byte g, byte b)
{	
	
	CT_Node* node = &tree->nodes[0];
	
	for(;;) {
		if(node->children[0] == 0)
			// return the palette index
			return node->children[1];
		else {
			// Left or right ?
			CT_Node* child0 = &tree->nodes[node->children[0]];
			if (child0->Rmin <= r
				&& child0->Gmin <= g
				&& child0->Bmin <= b
				&& child0->Rmax >= r
				&& child0->Gmax >= g
				&& child0->Bmax >= b
			) {
				// left
				node = child0;
			} else {
				// right
				node = &tree->nodes[node->children[1]];
			}
		}
	}
}

void CT_delete(CT_Tree* tree)
{
	free(tree);
}
