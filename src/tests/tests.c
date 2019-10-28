/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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
///@file tests.c
/// Unit tests.
///
/// TODO : make a test binary and include the tests to the constant-integration

#include <stdio.h>
#include <string.h>
#include "../struct.h"
#include "../gfx2log.h"

/**
 * Tests for MOTO_MAP_pack()
 */
int Test_MOTO_MAP_pack(void)
{
  unsigned int i, j;
  byte buffer[1024];
  byte buffer2[1024];
  unsigned int packed_len, unpacked_len, original_len;
  static const char * tests[] = {
    "12345AAAAAAA@",    // best : 00 05 "12345" 07 'A' 01 '@' : 11
    "123AABAA123@@@@@", // best : 00 0B "123AABAA123" 05 '@' : 15
    "123AAAAA123AABAA", // best : 00 03 "123" 05 'A' 00 06 "123AAB" 02 'A' : 17
    "123AAAAA123AAB",   // best : 00 03 "123" 05 'A' 00 06 "123AAB" : 15
    "abcAABAAdddddddd", // best : 00 08 "abcAABAA" 08 'd' : 12
    NULL
  };

  GFX2_Log(GFX2_DEBUG, "Test_MOTO_MAP_pack\n");
  for (i = 0; tests[i]; i++)
  {
    original_len = strlen(tests[i]);
    packed_len = MOTO_MAP_pack(buffer, (const byte *)tests[i], original_len);
    GFX2_Log(GFX2_DEBUG, "    %s (%u) packed to %u\n", tests[i], original_len, packed_len);
    unpacked_len = 0;
    j = 0;
    // unpack to test
    while (j < packed_len)
    {
      if (buffer[j] == 0)
      { // copy
        memcpy(buffer2 + unpacked_len, buffer + j + 2, buffer[j+1]);
        unpacked_len += buffer[j+1];
        j += 2 + buffer[j+1];
      }
      else
      { // repeat
        memset(buffer2 + unpacked_len, buffer[j+1], buffer[j]);
        unpacked_len += buffer[j];
        j += 2;
      }
    }
    if (unpacked_len != original_len || 0 != memcmp(tests[i], buffer2, original_len))
    {
      GFX2_Log(GFX2_ERROR, "*** %u %s != %u %s ***\n", original_len, tests[i], unpacked_len, buffer2);
      return 0;  // test failed
    }
  }
  return 1;  // test OK
}
