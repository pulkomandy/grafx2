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
#include "../global.h"
#include "../fileformats.h"
#include "../gfx2log.h"

#define TESTFMT(fmt, sample) { # fmt, Test_ ## fmt, sample },
static const struct {
  const char * name;
  Func_IO_Test Test;
  const char * sample;
} formats[] = {
  TESTFMT(PKM, "EVILNUN.PKM")
  TESTFMT(GIF, "gif/2b_horse.gif")
  TESTFMT(PCX, "pcx/lena.pcx")
  TESTFMT(NEO, "atari_st/ATARIART.NEO")
  TESTFMT(PC1, "atari_st/eunmiisa.pc1")
  TESTFMT(PI1, "atari_st/evolutn.pi1")
  TESTFMT(FLI, "autodesk_FLI_FLC/2noppaa.fli")
  TESTFMT(BMP, "bmp/test16bf565.bmp")
  TESTFMT(ICO, "ico/gitlab_favicon.ico")
  TESTFMT(C64, "c64/multicolor/ARKANOID.KOA")
  TESTFMT(GPX, "c64/pixcen/Cyberbird.gpx")
  //TESTFMT(SCR, "CPC_SCR/DANCEOFF.SCR")
  TESTFMT(MOTO,"thomson/exocet-alientis.map")
  TESTFMT(HGR, "apple2/hgr/pop-swordfight.hgr")
  TESTFMT(ACBM,"iff/ACBM/Jupiter_alt.pic")
  TESTFMT(LBM, "iff/Danny_SkyTravellers_ANNO.iff")
  TESTFMT(PBM, "iff/pbm/FC.LBM")
  TESTFMT(INFO,"amiga_icons/4colors/Utilities/Calculator.info")
#ifndef __no_pnglib__
  TESTFMT(PNG, "png/happy-birthday-guys.png")
#endif
#ifndef __no_tifflib__
  TESTFMT(TIFF,"tiff/grafx2_banner.tif")
#endif
  TESTFMT(GPL, "palette-mariage_115.gpl")
  TESTFMT(PAL, "pal/dp4_256.pal")
  { NULL, NULL, NULL}
};

int Test_Formats(void)
{
  char path[256];
  FILE * f;
  int i, j;

  for (i = 0; formats[i].name != NULL; i++)
  {
    GFX2_Log(GFX2_DEBUG, "Testing format %s\n", formats[i].name);
    // first check that the right sample is recognized
    snprintf(path, sizeof(path), "../tests/pic-samples/%s", formats[i].sample);
    f = fopen(path, "rb");
    if (f == NULL)
    {
      GFX2_Log(GFX2_ERROR, "error opening %s\n", path);
      return 0;
    }
    File_error = 1;
    formats[i].Test(NULL, f);
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
      snprintf(path, sizeof(path), "../tests/pic-samples/%s", formats[j].sample);
      f = fopen(path, "rb");
      if (f == NULL)
      {
        GFX2_Log(GFX2_ERROR, "error opening %s\n", path);
        return 0;
      }
      File_error = 1;
      formats[i].Test(NULL, f);
      fclose(f);
      if (File_error == 0)
      {
        GFX2_Log(GFX2_ERROR, "Test_%s failed for file %s\n", formats[i].name, formats[j].sample);
        return 0;
      }
    }
  }
  return 1;   // OK
}
