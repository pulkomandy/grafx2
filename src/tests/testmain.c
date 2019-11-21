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
///@file testmain.c
/// Unit tests.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../struct.h"
#include "../global.h"
#include "../gfx2log.h"
#include "tests.h"

#ifdef ENABLE_FILENAMES_ICONV
iconv_t cd;             // FROMCODE => TOCODE
iconv_t cd_inv;         // TOCODE => FROMCODE
iconv_t cd_utf16;       // FROMCODE => UTF16
iconv_t cd_utf16_inv;   // UTF16 => FROMCODE
#endif
signed char File_error;

T_Config Config;

T_Document Main;
T_Document Spare;
byte * Screen_backup;
byte * Main_Screen;
short Screen_width;
short Screen_height;
short Original_screen_X;
short Original_screen_Y;

byte Menu_factor_X;
byte Menu_factor_Y;
int Pixel_ratio;

byte First_color_in_palette;
byte Back_color;
byte MC_Dark;
byte MC_Light;

T_Window Window_stack[8];
byte Windows_open;

dword Key;

static const struct {
  int (*test_func)(void);
  const char * test_name;
} tests[] = {
#define TEST(func) { Test_ ## func, # func },
#include "testlist.h"
#undef TEST
  { NULL, NULL}
};

/**
 * Initializations for test program
 */
void init(void)
{
  srandom(time(NULL));
#ifdef ENABLE_FILENAMES_ICONV
  // iconv is used to convert filenames
  cd = iconv_open(TOCODE, FROMCODE);  // From UTF8 to ANSI
  cd_inv = iconv_open(FROMCODE, TOCODE);  // From ANSI to UTF8
#if (defined(SDL_BYTEORDER) && (SDL_BYTEORDER == SDL_BIG_ENDIAN)) || (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN))
  cd_utf16 = iconv_open("UTF-16BE", FROMCODE); // From UTF8 to UTF16
  cd_utf16_inv = iconv_open(FROMCODE, "UTF-16BE"); // From UTF16 to UTF8
#else
  cd_utf16 = iconv_open("UTF-16LE", FROMCODE); // From UTF8 to UTF16
  cd_utf16_inv = iconv_open(FROMCODE, "UTF-16LE"); // From UTF16 to UTF8
#endif
#endif /* ENABLE_FILENAMES_ICONV */
}

/**
 * Test program entry point
 */
int main(int argc, char * * argv)
{
  int i, r;
  int fail = 0;

  GFX2_verbosity_level = GFX2_DEBUG;
  init();

  for (i = 0; tests[i].test_func != 0; i++)
  {
    printf("Testing %s :\n", tests[i].test_name);
    r = tests[i].test_func();
    if (r)
      printf("OK\n");
    else {
      printf("FAILED\n");
      fail++;
    }
  }

  if (fail == 0)
  {
    printf("All tests succesfull\n");
    return 0;
  }
  else
  {
    printf("%d tests failed\n", fail);
    return 1;
  }
}
