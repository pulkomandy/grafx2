/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018-2019 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Petter Lindquist
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2007-2011 Adrien Destugues
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
///@file testop_c.c
/// Unit tests.
///
#include <stdio.h>
#include <string.h>
#include "tests.h"
#include "../op_c.h"
#include "../gfx2log.h"

int Test_Convert_24b_bitmap_to_256(char * msg)
{
  T_Palette palette;
  byte dest[256];
  T_Components source[256];
  int i;

  // first try if a 256 colors picture is converted OK
  for (i = 0; i < 256; i++)
  {
    source[i].R = i;
    source[i].G = i;
    source[i].B = i;
  }
  if (Convert_24b_bitmap_to_256(dest, source, 16, 16, palette) != 0)
  {
    return 0;
  }
  GFX2_LogHexDump(GFX2_DEBUG, "", dest, 0, 256);
  for (i = 0; i < 256; i++)
  {
    if (memcmp(&source[i], &palette[dest[i]], sizeof(T_Components)) != 0)
      return 0;
  }
  // TODO: test a real reduction
  return 1;
}
