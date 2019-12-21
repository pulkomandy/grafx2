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
///@file testformats.c
/// Unit tests for picture format loaders/savers
///
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../global.h"
#include "../fileformats.h"
#include "../gfx2log.h"
#include "../gfx2mem.h"

// Load_IFF/Save_IFF does for both LBM and PBM (and Load_IFF also loads ACBM format)
#define Load_ACBM Load_IFF
#define Load_LBM Load_IFF
#define Load_PBM Load_IFF
#define Save_LBM Save_IFF
#define Save_PBM Save_IFF

// 16 colors 320x200 format. For testing Atari ST formats.
#define FLAG_16C 1
// 16 color 192x272 format. CPC overscan
#define FLAG_CPCO 2

// Load/Save
#define TESTFMTF(fmt, sample, flags) { FORMAT_ ## fmt, # fmt, Test_ ## fmt, Load_ ## fmt, Save_ ## fmt, flags, sample },
#define TESTFMT(fmt, sample) TESTFMTF(fmt, sample, 0)
// Load only
#define TESTFMTL(fmt, sample) { FORMAT_ ## fmt, # fmt, Test_ ## fmt, Load_ ## fmt, NULL, 0, sample },
static const struct {
  enum FILE_FORMATS format;
  const char * name;
  Func_IO_Test Test;
  Func_IO Load;
  Func_IO Save;
  int flags;
  const char * sample;
} formats[] = {
  TESTFMT(PKM, "pkm/EVILNUN.PKM")
  TESTFMT(GIF, "gif/2b_horse.gif")
  TESTFMT(PCX, "pcx/lena2.pcx")
  TESTFMTF(NEO, "atari_st/ATARIART.NEO", FLAG_16C)
  TESTFMTF(PC1, "atari_st/eunmiisa.pc1", FLAG_16C)
  TESTFMTF(PI1, "atari_st/evolutn.pi1", FLAG_16C)
  TESTFMTF(TNY, "atari_st/rose.tny", FLAG_16C)
  TESTFMTL(FLI, "autodesk_FLI_FLC/2noppaa.fli")
  TESTFMT(BMP, "bmp/test8.bmp")
  TESTFMTL(ICO, "ico/punzip.ico")               // Format with limitations
  TESTFMTL(C64, "c64/multicolor/ARKANOID.KOA")  // Format with limitations
  TESTFMTL(PRG, "c64/multicolor/speedball2_loading_jonegg.prg")
  TESTFMTL(GPX, "c64/pixcen/Cyberbird.gpx")
  TESTFMTF(SCR, "cpc/scr/DANCEOFF.SCR", FLAG_CPCO)
  TESTFMTL(CM5, "cpc/mode5/spidey.cm5")         // Format with limitations
  TESTFMTL(PPH, "cpc/pph/BF.PPH")               // Format with limitations
  TESTFMTF(GOS, "cpc/iMPdraw_GFX/SONIC.GO1", FLAG_CPCO)
  TESTFMTL(MOTO,"thomson/exocet-alientis.map")  // Format with limitations
  TESTFMTL(HGR, "apple2/hgr/pop-swordfight.hgr")  // Format with limitations
  TESTFMTL(ACBM, "iff/ACBM/Jupiter_alt.pic")
  TESTFMT(LBM, "iff/Danny_SkyTravellers_ANNO.iff")
  TESTFMT(PBM, "iff/pbm/FC.LBM")
  TESTFMTL(INFO,"amiga_icons/4colors/Utilities/Calculator.info")
#ifndef __no_pnglib__
  TESTFMT(PNG, "png/happy-birthday-guys.png")
#endif
#ifndef __no_tifflib__
  TESTFMT(TIFF,"tiff/grafx2_banner.tif")
#endif
  TESTFMTL(GPL, "palette-mariage_115.gpl") //PALETTE
  TESTFMTL(PAL, "pal/dp4_256.pal") // PALETTE
  { FORMAT_ALL_IMAGES, NULL, NULL, NULL, NULL, 0, NULL}
};

/**
 * Set the context File_directory and File_name
 */
static void context_set_file_path(T_IO_Context * context, const char * filepath)
{
  char * p = strrchr(filepath, '/');

  if (context->File_name)
    free(context->File_name);
  if (context->File_directory)
    free(context->File_directory);

  if (p != NULL)
  {
    size_t dirlen = p - filepath;
    context->File_name = strdup(p + 1);
    context->File_directory = GFX2_malloc(dirlen + 1);
    memcpy(context->File_directory, filepath, dirlen);
    context->File_directory[dirlen] = '\0';
  }
  else
  {
    context->File_name = strdup(filepath);
    context->File_directory = strdup(".");
  }
}

/**
 * test the Test_* functions
 */
int Test_Formats(void)
{
  T_IO_Context context;
  char path[256];
  FILE * f;
  int i, j;

  memset(&context, 0, sizeof(context));
  for (i = 0; formats[i].name != NULL; i++)
  {
    GFX2_Log(GFX2_DEBUG, "Testing format %s (Test)\n", formats[i].name);
    // first check that the right sample is recognized
    snprintf(path, sizeof(path), "../tests/pic-samples/%s", formats[i].sample);
    f = fopen(path, "rb");
    if (f == NULL)
    {
      GFX2_Log(GFX2_ERROR, "error opening %s\n", path);
      return 0;
    }
    context_set_file_path(&context, path);
    File_error = 1;
    formats[i].Test(&context, f);
    fclose(f);
    if (File_error != 0)
    {
      GFX2_Log(GFX2_ERROR, "Test_%s failed for file %s\n", formats[i].name, formats[i].sample);
      return 0;
    }
    // now check that all other samples are not recognized
    for (j = 0; formats[j].name != NULL; j++)
    {
      if (j == i)
        continue;
      // skip Test_HGR(*.SCR) because Test_HGR() only tests for file size
      if (strcmp(formats[i].name, "HGR") == 0 && strcmp(formats[j].name, "SCR") == 0)
        continue;
      snprintf(path, sizeof(path), "../tests/pic-samples/%s", formats[j].sample);
      f = fopen(path, "rb");
      if (f == NULL)
      {
        GFX2_Log(GFX2_ERROR, "error opening %s\n", path);
        return 0;
      }
      context_set_file_path(&context, path);
      File_error = 1;
      formats[i].Test(&context, f);
      fclose(f);
      if (File_error == 0)
      {
        GFX2_Log(GFX2_ERROR, "Test_%s failed for file %s\n", formats[i].name, formats[j].sample);
        return 0;
      }
    }
  }
  //Destroy_context(&context);
  free(context.File_name);
  free(context.File_directory);
  return 1;   // OK
}

/**
 * test the Load_* functions
 */
int Test_Load(void)
{
  T_IO_Context context;
  char path[256];
  int i;

  memset(&context, 0, sizeof(context));
  context.Type = CONTEXT_SURFACE;
  for (i = 0; formats[i].name != NULL; i++)
  {
    GFX2_Log(GFX2_DEBUG, "Testing format %s (Load)\n", formats[i].name);
    // first check that the right sample is recognized
    snprintf(path, sizeof(path), "../tests/pic-samples/%s", formats[i].sample);
    context_set_file_path(&context, path);
    File_error = 0;
    formats[i].Load(&context);
    if (File_error != 0)
    {
      GFX2_Log(GFX2_ERROR, "Load_%s failed for file %s\n", formats[i].name, formats[i].sample);
      return 0;
    }
    if (context.Surface)
    {
      printf(" %hux%hu\n", context.Surface->w, context.Surface->h);
      Free_GFX2_Surface(context.Surface);
      context.Surface = NULL;
    }
  }
  //Destroy_context(&context);
  free(context.File_name);
  free(context.File_directory);
  return 1; // OK
}

/**
 * Test the Save_* functions
 */
int Test_Save(void)
{
  T_IO_Context context;
  char path[256];
  char tmpdir[40];
  int i;
  int ok = 0;
  T_GFX2_Surface * testpic256 = NULL;
  T_GFX2_Surface * testpic16 = NULL;
  T_GFX2_Surface * testpiccpco = NULL;

  memset(&context, 0, sizeof(context));
  context.Type = CONTEXT_SURFACE;
  context.Nb_layers = 1;
  // Load EVILNUN.PKM
  context_set_file_path(&context, "../tests/pic-samples/pkm/EVILNUN.PKM");
  File_error = 0;
  Load_PKM(&context);
  if (File_error != 0)
  {
    fprintf(stderr, "Failed to load reference 256 colors picture\n");
    goto ret;
  }
  testpic256 = context.Surface;
  context.Surface = NULL;
  memcpy(testpic256->palette, context.Palette, sizeof(T_Palette));
  // Load borregas.gif
  context_set_file_path(&context, "../tests/pic-samples/gif/borregas.gif");
  Load_GIF(&context);
  if (File_error != 0)
  {
    fprintf(stderr, "Failed to load reference 16 colors picture\n");
    goto ret;
  }
  testpic16 = context.Surface;
  context.Surface = NULL;
  memcpy(testpic16->palette, context.Palette, sizeof(T_Palette));
  // Load POULPE.GO1/GO2 etc.
  context_set_file_path(&context, "../tests/pic-samples/cpc/iMPdraw_GFX/POULPE.GO1");
  Load_GOS(&context);
  if (File_error != 0)
  {
    fprintf(stderr, "Failed to load reference CPC overscan mode 0 picture\n");
    goto ret;
  }
  testpiccpco = context.Surface;
  context.Surface = NULL;
  memcpy(testpiccpco->palette, context.Palette, sizeof(T_Palette));

  snprintf(tmpdir, sizeof(tmpdir), "/tmp/grafx2-test.XXXXXX");
  if (mkdtemp(tmpdir) == NULL)
  {
    perror("mkdtemp");
    goto ret;
  }
  printf("temp dir : %s\n", tmpdir);
  ok = 1;
  for (i = 0; ok && formats[i].name != NULL; i++)
  {
    if (formats[i].Save == NULL)
      continue;
    GFX2_Log(GFX2_DEBUG, "Testing format %s (Save)\n", formats[i].name);
    snprintf(path, sizeof(path), "%s/%s.%s", tmpdir, "test", formats[i].name);
    context_set_file_path(&context, path);

    // save the reference picture
    context.Surface = (formats[i].flags & FLAG_16C) ? testpic16 : (formats[i].flags & FLAG_CPCO) ? testpiccpco : testpic256;
    context.Target_address = context.Surface->pixels;
    context.Pitch = context.Surface->w;
    context.Width = context.Surface->w;
    context.Height = context.Surface->h;
    context.Ratio = (formats[i].flags & FLAG_CPCO) ? PIXEL_WIDE : PIXEL_SIMPLE;
    memcpy(context.Palette, context.Surface->palette, sizeof(T_Palette));
    context.Format = formats[i].format;
    File_error = 0;
    formats[i].Save(&context);
    context.Surface = NULL;
    if (File_error != 0)
    {
      GFX2_Log(GFX2_ERROR, "Save_%s failed.\n", formats[i].name);
      ok = 0;
    }
    else
    {
      FILE * f;
      // Test the saved file
      f = fopen(path, "rb");
      if (f == NULL)
      {
        GFX2_Log(GFX2_ERROR, "error opening %s\n", path);
        ok = 0;
      }
      else
      {
        File_error = 1;
        formats[i].Test(&context, f);
        fclose(f);
        if (File_error != 0)
        {
          GFX2_Log(GFX2_ERROR, "Test_%s failed for file %s\n", formats[i].name, path);
          ok = 0;
        }
      }
      memset(context.Palette, -1, sizeof(T_Palette));
      // load the saved file
      formats[i].Load(&context);
      if (File_error != 0 || context.Surface == NULL)
      {
        GFX2_Log(GFX2_ERROR, "Load_%s failed for file %s\n", formats[i].name, path);
        ok = 0;
      }
      else
      {
        T_GFX2_Surface * ref = (formats[i].flags & FLAG_16C) ? testpic16 : (formats[i].flags & FLAG_CPCO) ? testpiccpco : testpic256;
        // compare with the reference picture
        if (context.Surface->w != ref->w || context.Surface->h != ref->h)
        {
          GFX2_Log(GFX2_ERROR, "Saved %hux%hu, reloaded %hux%hu from %s\n",
                   ref->w, ref->h, context.Surface->w, context.Surface->h, path);
          ok = 0;
        }
        else if (0 != memcmp(context.Surface->pixels, ref->pixels, ref->w * ref->h))
        {
          GFX2_Log(GFX2_ERROR, "Save_%s/Load_%s: Pixels mismatch\n", formats[i].name, formats[i].name);
          ok = 0;
        }
        else if (!(formats[i].flags & FLAG_CPCO) && 0 != memcmp(context.Palette, ref->palette, (formats[i].flags & FLAG_16C) ? 16 * sizeof(T_Components) : sizeof(T_Palette)))
        {
          GFX2_Log(GFX2_ERROR, "Save_%s/Load_%s: Palette mismatch\n", formats[i].name, formats[i].name);
          ok = 0;
        }
        else
        {
          if (unlink(path) < 0)
            perror("unlink");
        }
        Free_GFX2_Surface(context.Surface);
        context.Surface = NULL;
      }
    }
  }
  if (rmdir(tmpdir) < 0)
    perror("rmdir");
ret:
  if (testpic16)
    Free_GFX2_Surface(testpic16);
  if (testpic256)
    Free_GFX2_Surface(testpic256);
  if (testpiccpco)
    Free_GFX2_Surface(testpiccpco);
  free(context.File_name);
  free(context.File_directory);
  return ok;
}
