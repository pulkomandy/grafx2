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
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#if defined(WIN32)
#include <windows.h>
#endif
#include "../struct.h"
#include "../global.h"
#include "../io.h"
#include "../gfx2log.h"
#include "tests.h"

// random()/srandom() not available with mingw32
#if defined(WIN32)
#define srandom srand
#endif


// mkdtemp() not available with mingw32
#if defined(WIN32)
#define mkdtemp my_mkdtemp

char * my_mkdtemp(char *template)
{
  char * p = strstr(template, "XXXXXX");
  if (p == NULL)
    return NULL;
  snprintf(p, 7, "%06x", rand());
  if (!CreateDirectoryA(template, NULL))
    return NULL;
  return template;
}
#endif

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

char tmpdir[256];

static const struct {
  int (*test_func)(char * errmsg);
  const char * test_name;
} tests[] = {
#define TEST(func) { Test_ ## func, # func },
#include "testlist.h"
#undef TEST
};
#define TEST_COUNT (sizeof(tests) / sizeof(tests[0]))

/**
 * Initializations for test program
 */
int init(void)
{
#ifdef WIN32
  char temp[256];
  DWORD len;
#endif
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
#ifdef WIN32
  len = GetTempPathA(sizeof(temp), temp);
  snprintf(tmpdir, sizeof(tmpdir), "%s%sgrafx2-test.XXXXXX",
           temp, temp[len-1] == PATH_SEPARATOR[0] ? "" : PATH_SEPARATOR);
#else
  snprintf(tmpdir, sizeof(tmpdir), "%s%sgrafx2-test.XXXXXX", "/tmp", PATH_SEPARATOR);
#endif
  if (mkdtemp(tmpdir) == NULL)
  {
    perror("mkdtemp");
    return -1;
  }
  printf("temp dir : %s\n", tmpdir);
  return 0;
}

/**
 * Releases resources
 */
void finish(void)
{
#ifdef ENABLE_FILENAMES_ICONV
  iconv_close(cd);
  iconv_close(cd_inv);
  iconv_close(cd_utf16);
  iconv_close(cd_utf16_inv);
#endif /* ENABLE_FILENAMES_ICONV */
  if (rmdir(tmpdir) < 0)
    fprintf(stderr, "Failed to rmdir(\"%s\"): %s\n", tmpdir, strerror(errno));
}

#define ESC_GREEN "\033[32m"
#define ESC_RED   "\033[31m"
#define ESC_RESET "\033[0m"

/**
 * Test program entry point
 */
int main(int argc, char * * argv)
{
  int i;
  int fail = 0;
  int r[TEST_COUNT];
  int fail_early = 0;
  const char * xml_path = "test-report.xml";
  FILE * xml; // see https://llg.cubic.org/docs/junit/
  char errmsg[ERRMSG_LENGTH];

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--help") == 0)
    {
      printf("Usage:  %s [--fail-early] [--xml <report.xml>]\n", argv[0]);
      printf("default path for xml report is \"%s\"\n", xml_path);
      return 0;
    }
    else if (strcmp(argv[i], "--fail-early") == 0)
      fail_early = 1;
    else if ((i < (argc - 1)) && strcmp(argv[i], "--xml") == 0)
      xml_path = argv[++i];
    else
    {
      fprintf(stderr, "Unrecognized option \"%s\"\n", argv[i]);
      return 1;
    }
  }

  GFX2_verbosity_level = GFX2_DEBUG;
  if (init() < 0)
  {
    fprintf(stderr, "Failed to init.\n");
    return 1;
  }

  xml = fopen(xml_path, "w");
  if (xml == NULL)
  {
    fprintf(stderr, "Failed to open %s for writing\n", xml_path);
    return 1;
  }
  fprintf(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(xml, "<testsuite name=\"GrafX2\" tests=\"%lu\">\n", TEST_COUNT);
  for (i = 0; i < (int)TEST_COUNT; i++)
  {
    if (fail_early && fail > 0)
    {
      fprintf(xml, "  <testcase name=\"%s\" classname=\"%s\">\n", tests[i].test_name, "GrafX2");
      fprintf(xml, "    <skipped />\n");
    }
    else
    {
      long t;
      struct timeval t0, t1;
      printf("Testing %s :\n", tests[i].test_name);
      errmsg[0] = '\0';
      gettimeofday(&t0, NULL);
      r[i] = tests[i].test_func(errmsg);
      gettimeofday(&t1, NULL);
      t = t1.tv_sec - t0.tv_sec;
      if ((t1.tv_usec - t0.tv_usec) > 500000)
        t++;
      else if ((t1.tv_usec - t0.tv_usec) < -500000)
        t--;
      fprintf(xml, "  <testcase name=\"%s\" classname=\"%s\" time=\"%ld\"",
              tests[i].test_name, "GrafX2", t);
      if (r[i])
      {
        fprintf(xml, " />\n");
        printf(ESC_GREEN "OK" ESC_RESET "\n");
      }
      else
      {
        fprintf(stderr, ESC_RED "%s" ESC_RESET "\n", errmsg);
        printf(ESC_RED "FAILED" ESC_RESET "\n");
        fail++;
        fprintf(xml, ">\n    <failure message='%s'><!-- failure details --></failure>\n",
                errmsg);
        fprintf(xml, "  </testcase>\n");
      }
    }
  }
  fprintf(xml, "</testsuite>\n");
  fclose(xml);

  finish();

  if (fail == 0)
  {
    printf(ESC_GREEN "All tests succesfull" ESC_RESET "\n");
    return 0;
  }
  else if (!fail_early)
  {
    printf(ESC_RED "%d tests failed :\n  ", fail);
    for (i = 0; i < (int)TEST_COUNT; i++)
    {
      if (!r[i])
      {
        printf("%s ", tests[i].test_name);
      }
    }
    puts(ESC_RESET);  /* puts writes an additional newline character */
  }
  return 1;
}
