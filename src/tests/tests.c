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
///@file tests.c
/// Unit tests.
///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tests.h"
#include "../struct.h"
#include "../oldies.h"
#include "../packbits.h"
#include "../io.h"
#include "../gfx2log.h"

// random()/srandom() not available with mingw32
#if defined(WIN32)
#define random (long)rand
#endif

unsigned int MOTO_MAP_pack(byte * packed, const byte * unpacked, unsigned int unpacked_len);

/**
 * Tests for MOTO_MAP_pack()
 */
int Test_MOTO_MAP_pack(char * errmsg)
{
  unsigned int i, j;
  byte buffer[256];
  byte buffer2[256];
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
      snprintf(errmsg, ERRMSG_LENGTH, "(un)pack error : %u %s != %u %s",
               original_len, tests[i], unpacked_len, buffer2);
      return 0;  // test failed
    }
  }
  return 1;  // test OK
}

/**
 * Test for Test_CPC_compare_colors()
 */
int Test_CPC_compare_colors(char * errmsg)
{
  unsigned int r, g, b;
  T_Components c1, c2;
  for (r = 0; r < 16; r++)
  {
    for (g = 0; g < 16; g++)
    {
      for (b = 0; b < 16; b++)
      {
        c1.R = r * 0x11;
        c1.G = g * 0x11;
        c1.B = b * 0x11;
        if (!CPC_compare_colors(&c1, &c1))
        {
          snprintf(errmsg, ERRMSG_LENGTH, "same colors should be recognized as identical");
          return 0;
        }
        c2.R = ((r + 6) & 15) * 0x11;
        c2.G = c1.G;
        c2.B = c1.B;
        if (CPC_compare_colors(&c1, &c2))
        {
          snprintf(errmsg, ERRMSG_LENGTH, "#%02x%02x%02x != #%02x%02x%02x",
                   c1.R, c1.G, c1.B, c2.R, c2.G, c2.B);
          return 0; // Should be differents !
        }
        c2.R = c1.R;
        c2.G = ((g + 6) & 15) * 0x11;
        if (CPC_compare_colors(&c1, &c2))
        {
          snprintf(errmsg, ERRMSG_LENGTH, "#%02x%02x%02x != #%02x%02x%02x",
                   c1.R, c1.G, c1.B, c2.R, c2.G, c2.B);
          return 0; // Should be differents !
        }
        c2.G = c1.G;
        c2.B = ((b + 6) & 15) * 0x11;
        if (CPC_compare_colors(&c1, &c2))
        {
          snprintf(errmsg, ERRMSG_LENGTH, "#%02x%02x%02x != #%02x%02x%02x",
                   c1.R, c1.G, c1.B, c2.R, c2.G, c2.B);
          return 0; // Should be differents !
        }
      }
    }
  }
  return 1; // test OK
}

/**
 * Tests for the packbits compression used in IFF ILBM, etc.
 * see http://fileformats.archiveteam.org/wiki/PackBits
 */
int Test_Packbits(char * errmsg)
{
  char tempfilename[256];
  FILE * f;
  int i, j;
  long unpacked;
  long packed;
  static const char * tests[] = {
    "1234AAAAAAAAAAAAAAAAAAAAA",  // best : 03 "1234" -20(ec) 'A' => 7 bytes
    "AABBCCDDDDDDD12345@@@54321", // best : -1(ff) 'A' -1(ff) 'B' -1(ff) 'C' -6(fa) 'D' 12(0c) "12345@@@54321" => 22 bytes
                                  //                                            or 04 "12345" -2(fe) '@' 04 "54321"
    "123AA123BBB123CCCC123DDDDD", // best : 07 "123AA123" -2 'B' 02 "123" -3(fd) 'C' 02 "123" -4(fc) 'D' => 23 bytes
    "12345678123456781234567812345678" // 32
    "12345678123456781234567812345678" // 64
    "12345678123456781234567812345678" // 96
    "12345678123456781234567812345678" // 128
    "1", // best : 127(7f) "123..." 01 "1" => 131
    "12345678123456781234567812345678" // 32
    "12345678123456781234567812345678" // 64
    "12345678123456781234567812345678" // 96
    "123456781234567812345678123456@@" // 128
    "@@12345678", // best : 125(7d) "123..." -3 '@' 08 "12345678" => 138
    "12345678123456781234567812345678" // 32
    "12345678123456781234567812345678" // 64
    "12345678123456781234567812345678" // 96
    "1234567812345678123456781234567@" // 128
    "@@12345678", // best : 126(7e) "123..." -2 '@' 08 "12345678" => 139
    NULL
  };
  const long best_packed = 7 + 22 + 23 + 131 + (138/*+2*/) + (139/*+1*/);
  byte buffer[1024];
  T_PackBits_data pb_data;

  snprintf(tempfilename, sizeof(tempfilename), "%s%sgfx2test-packbits-%lx", tmpdir, PATH_SEPARATOR, random());
  GFX2_Log(GFX2_DEBUG, "tempfile %s\n", tempfilename);
  f = fopen(tempfilename, "wb");
  if (f == NULL)
  {
    snprintf(errmsg, ERRMSG_LENGTH, "Failed to open %s for writing", tempfilename);
    return 0;
  }

  // Start encoding
  PackBits_pack_init(&pb_data, f);
  for (i = 0, unpacked = 0; tests[i]; i++)
  {
    for (j = 0; tests[i][j]; j++)
    {
      if (PackBits_pack_add(&pb_data, (byte)tests[i][j]) < 0)
      {
        snprintf(errmsg, ERRMSG_LENGTH, "PackBits_pack_add() failed");
        return 0;
      }
      unpacked++;
    }
    if (PackBits_pack_flush(&pb_data) < 0)
    {
      snprintf(errmsg, ERRMSG_LENGTH, "PackBits_pack_flush() failed");
      return 0;
    }
  }
  packed = ftell(f);
  fclose(f);
  GFX2_Log(GFX2_DEBUG, "Compressed %ld bytes to %ld\n", unpacked, packed);
  if (packed > best_packed) {
    snprintf(errmsg, ERRMSG_LENGTH, "Packbits less efficient than expected (%ld > %ld bytes)",
             packed, best_packed);
    return 0;
  }

  // test unpacking
  f = fopen(tempfilename, "rb");
  if (f == NULL)
  {
    snprintf(errmsg, ERRMSG_LENGTH, "Failed to open %s for reading", tempfilename);
    return 0;
  }
  for (i = 0; tests[i]; i++)
  {
    size_t len = strlen(tests[i]);
    memset(buffer, 0x80, len);
    if (PackBits_unpack_from_file(f, buffer, len) != PACKBITS_UNPACK_OK)
    {
      snprintf(errmsg, ERRMSG_LENGTH, "PackBits_unpack_from_file() failed");
      return 0;
    }
    if (memcmp(buffer, tests[i], len) != 0)
    {
      GFX2_Log(GFX2_ERROR, "uncompressed stream mismatch !\n");
      GFX2_LogHexDump(GFX2_ERROR, "original ", (const byte *)tests[i], 0, len);
      GFX2_LogHexDump(GFX2_ERROR, "unpacked ", buffer, 0, len);
      snprintf(errmsg, ERRMSG_LENGTH, "uncompressed stream mismatch");
      return 0;
    }
  }
  fclose(f);
  unlink(tempfilename);
  return 1; // test OK
}
