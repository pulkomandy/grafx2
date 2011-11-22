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

#include <stdlib.h>
#include <stdio.h>

#include "colorred.h"

/* Octree for mapping RGB to color. A bit slower than a plain conversion table in theory,
but :
  * Faster than running a search in the palette
  * Takes less memory than the huge conversion table
  * No loss of precision
*/

CT_Node* CT_new() {return NULL;}

// debug helper
void CT_Print(CT_Node* node)
{
	printf("R %d %d\tG %d %d\tB %d %d\ti %d\n",
		node->Rmin, node->Rmax, node->Gmin, node->Gmax,
		node->Bmin, node->Bmax, node->index);
}

void CT_set(CT_Node** colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index)
{
	int i;
	CT_Node* parent;
	// Create and setup node
	CT_Node* node = malloc(sizeof(CT_Node));
	
	node->Rmin = Rmin;
	node->Gmin = Gmin;
	node->Bmin = Bmin;
	node->Rmax = Rmax;
	node->Gmax = Gmax;
	node->Bmax = Bmax;
	node->index = index;
	
	printf("Add node:");
	CT_Print(node);
	
	for(i = 0; i < 2; i++)
		node->children[i] = NULL;
	
	// Now insert it in tree
	parent = *colorTree;
	if (parent == NULL) {
		// This is our first node.
		*colorTree = node;
	} else for(;;) {
		// Find where to insert ourselves
		
		// pre-condition: the parent we're looking at is a superset of the node we're inserting
		// it may have 0, 1, or 2 child
		
		// 0 child: insert as child 0
		// 1 child: either we're included in the child, and recurse, or we''re not, and insert at child 1
		// 2 child: one of them has to be a superset of the node.
		
		if (parent->children[0] == NULL)
		{
			parent->children[0] = node;	
			break;
		} else {
			CT_Node* child0 = parent->children[0];
			if (child0->Rmin <= node->Rmin
				&& child0->Gmin <= node->Gmin
				&& child0->Bmin <= node->Bmin
				&& child0->Rmax >= node->Rmax
				&& child0->Gmax >= node->Gmax
				&& child0->Bmax >= node->Bmax
			) {
				parent = child0;
			} else if(parent->children[1] == NULL)
			{
				parent->children[1] = node;	
				break;
			} else {
				parent = parent->children[1];
			}
		}
	}
}

byte CT_get(CT_Node* node, byte r, byte g, byte b)
{	
	// pre condition: node contains (rgb)
	// find the leaf that also contains (rgb)
	
	for(;;) {
		if(node->children[0] == NULL)
			return node->index;
		else {
			// Left or right ?
			CT_Node* child0 = node->children[0];
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
				node = node->children[1];
			}
		}
	}
}

void CT_delete(CT_Node* tree)
{
	int i;
	if (tree == NULL)
		return;
	for	(i = 0; i < 2; i++)
	{
		CT_delete(tree->children[i]);
	}
	free(tree);
}
