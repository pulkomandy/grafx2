/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

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

///@file fileformats.h
/// Saving and loading different picture formats.

#ifndef FILEFORMATS_H_INCLUDED
#define FILEFORMATS_H_INCLUDED

#include "loadsave.h"

/// @defgroup loadsaveformats Test, Load and Save functions
///
/// The Test, Load and Save function for each supported
/// file formats are referenced in @ref File_formats
///
/// @{

// -- PKM -------------------------------------------------------------------
void Test_PKM(T_IO_Context *, FILE *);
void Load_PKM(T_IO_Context *);
void Save_PKM(T_IO_Context *);

// -- IFF -------------------------------------------------------------------
void Test_LBM(T_IO_Context *, FILE *);
void Test_PBM(T_IO_Context *, FILE *);
void Test_ACBM(T_IO_Context *, FILE *);
void Load_IFF(T_IO_Context *);
void Save_IFF(T_IO_Context *);

// -- GIF -------------------------------------------------------------------
void Test_GIF(T_IO_Context *, FILE *);
void Load_GIF(T_IO_Context *);
void Save_GIF(T_IO_Context *);

// -- PCX -------------------------------------------------------------------
void Test_PCX(T_IO_Context *, FILE *);
void Load_PCX(T_IO_Context *);
void Save_PCX(T_IO_Context *);

// -- BMP -------------------------------------------------------------------
void Test_BMP(T_IO_Context *, FILE *);
void Load_BMP(T_IO_Context *);
void Save_BMP(T_IO_Context *);

// -- IMG -------------------------------------------------------------------
void Test_IMG(T_IO_Context *, FILE *);
void Load_IMG(T_IO_Context *);
void Save_IMG(T_IO_Context *);

// -- SCx -------------------------------------------------------------------
void Test_SCx(T_IO_Context *, FILE *);
void Load_SCx(T_IO_Context *);
void Save_SCx(T_IO_Context *);

// -- CEL -------------------------------------------------------------------
void Test_CEL(T_IO_Context *, FILE *);
void Load_CEL(T_IO_Context *);
void Save_CEL(T_IO_Context *);

// -- KCF -------------------------------------------------------------------
void Test_KCF(T_IO_Context *, FILE *);
void Load_KCF(T_IO_Context *);
void Save_KCF(T_IO_Context *);

// -- PAL -------------------------------------------------------------------
void Test_PAL(T_IO_Context *, FILE *);
void Load_PAL(T_IO_Context *);
void Save_PAL(T_IO_Context *);

// -- GPL -------------------------------------------------------------------
void Test_GPL(T_IO_Context *, FILE *);
void Load_GPL(T_IO_Context *);
void Save_GPL(T_IO_Context *);

// -- PI1 -------------------------------------------------------------------
void Test_PI1(T_IO_Context *, FILE *);
void Load_PI1(T_IO_Context *);
void Save_PI1(T_IO_Context *);

// -- PC1 -------------------------------------------------------------------
void Test_PC1(T_IO_Context *, FILE *);
void Load_PC1(T_IO_Context *);
void Save_PC1(T_IO_Context *);

// -- CA1 -------------------------------------------------------------------
void Test_CA1(T_IO_Context *, FILE *);
void Load_CA1(T_IO_Context *);
void Save_CA1(T_IO_Context *);

// -- Tiny Stuff ------------------------------------------------------------
void Test_TNY(T_IO_Context *, FILE *);
void Load_TNY(T_IO_Context *);
void Save_TNY(T_IO_Context *);

// -- NEO -------------------------------------------------------------------
void Test_NEO(T_IO_Context *, FILE *);
void Load_NEO(T_IO_Context *);
void Save_NEO(T_IO_Context *);

// -- C64 -------------------------------------------------------------------
void Test_C64(T_IO_Context *, FILE *);
void Load_C64(T_IO_Context *);
void Save_C64(T_IO_Context *);

void Test_PRG(T_IO_Context *, FILE *);
void Load_PRG(T_IO_Context *);
void Save_PRG(T_IO_Context *);

// -- GPX (pixcen C64)
void Test_GPX(T_IO_Context *, FILE *);
void Load_GPX(T_IO_Context *);

// -- SCR (Amstrad CPC)
void Test_SCR(T_IO_Context *, FILE *);
void Load_SCR(T_IO_Context *);
void Save_SCR(T_IO_Context *);

// -- CM5 (Amstrad CPC)
void Test_CM5(T_IO_Context *, FILE *);
void Load_CM5(T_IO_Context *);
void Save_CM5(T_IO_Context *);

// -- PPH (Amstrad CPC)
void Test_PPH(T_IO_Context *, FILE *);
void Load_PPH(T_IO_Context *);
void Save_PPH(T_IO_Context *);

// -- Graphos (Amstrad CPC)
void Test_GOS(T_IO_Context *, FILE *);
void Load_GOS(T_IO_Context *);
void Save_GOS(T_IO_Context *);

// -- SGX (SymbOS)
void Test_SGX(T_IO_Context *, FILE *);
void Load_SGX(T_IO_Context *);
void Save_SGX(T_IO_Context *);

// -- XPM (X PixMap)
// Loading is done through SDL_Image
void Save_XPM(T_IO_Context*);

// -- ICO (Windows ICO)
void Test_ICO(T_IO_Context *, FILE *);
void Load_ICO(T_IO_Context *);
void Save_ICO(T_IO_Context *);

// -- PNG -------------------------------------------------------------------
#ifndef __no_pnglib__
void Test_PNG(T_IO_Context *, FILE *);
void Load_PNG(T_IO_Context *);
void Save_PNG(T_IO_Context *);
void Load_PNG_Sub(T_IO_Context * context, FILE * file, const char * memory_buffer, unsigned long memory_buffer_size);
void Save_PNG_Sub(T_IO_Context * context, FILE * file, char * * buffer, unsigned long * buffer_size);
#endif

// -- INFO (Amiga ICONS) ----------------------------------------------------
void Test_INFO(T_IO_Context *, FILE *);
void Load_INFO(T_IO_Context *);

// -- FLI/FLC (Autodesk animator) -------------------------------------------
void Test_FLI(T_IO_Context *, FILE *);
void Load_FLI(T_IO_Context *);

// -- Thomson MO/TO computer series pictures --------------------------------
void Test_MOTO(T_IO_Context *, FILE *);
void Load_MOTO(T_IO_Context *);
void Save_MOTO(T_IO_Context *);

// -- Apple II HGR and DHGR pictures ----------------------------------------
void Test_HGR(T_IO_Context *, FILE *);
void Load_HGR(T_IO_Context *);
void Save_HGR(T_IO_Context *);

// -- TIFF ------------------------------------------------------------------
#ifndef __no_tifflib__
void Test_TIFF(T_IO_Context *, FILE *);
void Load_TIFF(T_IO_Context *);
void Save_TIFF(T_IO_Context *);
void Load_TIFF_from_memory(T_IO_Context *, const void *, unsigned long);
void Save_TIFF_to_memory(T_IO_Context *, void * *, unsigned long *);
#endif

// -- HP-48 Grob ------------------------------------------------------------
void Test_GRB(T_IO_Context *, FILE *);
void Load_GRB(T_IO_Context *);

// -- MSX -------------------------------------------------------------------
void Test_MSX(T_IO_Context *, FILE *);
void Load_MSX(T_IO_Context *);
void Save_MSX(T_IO_Context *);

// -- Apple II GS -----------------------------------------------------------
void Test_2GS(T_IO_Context *, FILE *);
void Load_2GS(T_IO_Context *);
void Save_2GS(T_IO_Context *);

/// @}
#endif
