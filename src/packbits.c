/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018-2019 Thomas Bernard
    Copyright 2008 Yves Rizoud
    Copyright 2007 Adrien Destugues
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

///@file packbits.c
/// Packbits compression as used in IFF etc.
/// see http://fileformats.archiveteam.org/wiki/PackBits

#include <stdio.h>
#include <string.h>
#include "struct.h"
#include "gfx2log.h"
#include "packbits.h"

int PackBits_unpack_from_file(FILE * f, byte * dest, unsigned int count)
{
  unsigned int i = 0;
  while (i < count)
  {
    byte cmd;
    if (fread(&cmd, 1, 1, f) != 1)
      return PACKBITS_UNPACK_READ_ERROR;
    if (cmd > 128)
    {
      // cmd > 128 => repeat (257 - cmd) the next byte
      byte v;
      if (fread(&v, 1, 1, f) != 1)
        return PACKBITS_UNPACK_READ_ERROR;
      if (count < (i + 257 - cmd))
        return PACKBITS_UNPACK_OVERFLOW_ERROR;
      memset(dest + i, v, (257 - cmd));
      i += (257 - cmd);
    }
    else if (cmd < 128)
    {
      // cmd < 128 => copy (cmd + 1) bytes
      if (count < (i + cmd + 1))
        return PACKBITS_UNPACK_OVERFLOW_ERROR;
      if (fread(dest + i, 1, (cmd + 1), f) != (unsigned)(cmd + 1))
        return PACKBITS_UNPACK_READ_ERROR;
      i += (cmd + 1);
    }
    else
    {
      // 128 = NOP
      GFX2_Log(GFX2_WARNING, "NOP in packbits stream\n");
    }
  }
  return PACKBITS_UNPACK_OK;
}
