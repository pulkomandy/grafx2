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

#include "colorred.h"

/* Octree for mapping RGB to color. A bit slower than a plain conversion table in theory,
but :
  * Faster than running a search in the palette
  * Takes less memory than the huge conversion table
  * No loss of precision
*/

CT_Node* CT_new() {return NULL;}

void CT_set(CT_Node* colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index)
{
	int i;
	CT_Node* parent = colorTree;
	// Create and setup node
	CT_Node* node = malloc(sizeof(CT_Node));
	
	node->Rmin = Rmin;
	node->Gmin = Gmin;
	node->Bmin = Bmin;
	node->Rmax = Rmax;
	node->Gmax = Gmax;
	node->Bmax = Bmax;
	node->index = index;
	
	for(i = 0; i < 7; i++)
		node->children[i] = NULL;
	
	// Now insert it in tree
	
	if (parent == NULL) {
		// This is our first node.
		colorTree = node;
	} else for(;;) {
		// Find where to insert ourselves
		// (clusters are non-intersecting, so there is no need to check for that)
		int where = 0;
		if (parent->Rmin < node->Rmin)
			where += 4;
		if (parent->Gmin < node->Gmin)
			where += 2;
		if (parent->Bmin < node->Bmin)
			where += 1;
			
		// TODO - we always insert downwards, maybe we should try to keep
		// the tree balanced...
			
		if (parent->children[where] == NULL)
		{
			// insert!	
			parent->children[where] = node;
			break;
		} else {
			// recurse
			parent = parent->children[where];	
		}
	}
}

byte CT_get(CT_Node* node, byte r, byte g, byte b)
{	
	int wheremin = 0;
	int wheremax = 0;
	
	if (node->Rmin < r)
		wheremin += 4;
	if (node->Gmin < g)
		wheremin += 2;
	if (node->Bmin < b)
		wheremin += 1;
		
	if (node->Rmax < r)
		wheremin += 4;
	if (node->Gmax < g)
		wheremin += 2;
	if (node->Bmax < b)
		wheremin += 1;
		
	if (wheremin == 7 && wheremax == 0)
	{
		// Found it!
		return node->index;	
	} else {
		return CT_get(node->children[wheremin], r, g, b);	
	}
}

void CT_delete(CT_Node* tree)
{
	int i;
	if (tree == NULL)
		return;
	for	(i = 0; i < 8; i++)
	{
		CT_delete(tree->children[i]);
	}
	free(tree);
}
