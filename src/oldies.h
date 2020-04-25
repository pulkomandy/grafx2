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

#ifndef OLDIES_H_DEFINED
#define OLDIES_H_DEFINED

///@file oldies.h
/// functions relative to old computers (Commodore 64, Thomsons MO/TO, Amstrad CPC, ZX Spectrum, etc.)

#include "loadsave.h"

/**
 * identifier for each Image mode
 *
 * @return an ASCII label for the mode
 */
const char * Constraint_mode_label(enum IMAGE_MODES mode);

/**
 * Search constraint mode for a label
 *
 * @return -1 for unknown mode or one of ::IMAGE_MODES value
 */
int Constraint_mode_from_label(const char * label);

/** @defgroup c64 Commodore 64
 * Some C64 video mode related functions
 * @{
 */
/**
 * Save a 3 layer picture to C64 FLI format
 *
 * @param context the Save context
 * @param bitmap a 8000 byte buffer to store bitmap data
 * @param screen_ram a 8192 byte buffer to store the 8 screen RAMs
 * @param color_ram a 1000 byte buffer to store the color RAM
 * @param background a 200 byte buffer to store the background colors
 * @return 0 for success, 1 if the picture is less than 3 layers, 2 if the picture dimensions are not 160x200
 */
int C64_FLI(T_IO_Context * context, byte *bitmap, byte *screen_ram, byte *color_ram, byte *background);

/**
 * Convert a (1 layer) picture to C64 FLI format
 *
 * Some "hints" can be put in background and color_ram.
 * (a color value >= 16 means no hint).
 *
 * Errors can be either outputed to the user with Warning messages,
 * or put in layer 4. The layer 4 has to be created before.
 *
 * @param bitmap a 8000 byte buffer to store bitmap data
 * @param screen_ram a 8192 byte buffer to store the 8 screen RAMs
 * @param color_ram a 1000 byte buffer to store the color RAM
 * @param background a 200 byte buffer to store the background colors
 * @param pixels source pixel buffer (at least 160x200)
 * @param pitch bytes per line of the pixel buffer
 * @param errmode error reporting mode 0 = report, 1 = mark in layer 4
 * @return 0 the number of constraint errors
 */
int C64_pixels_to_FLI(byte *bitmap, byte *screen_ram, byte *color_ram, byte *background, const byte * pixels, long pitch, int errmode);

/**
 * Set the 16 colors Commodore 64 palette
 */
void C64_set_palette(T_Components * palette);

/** @}*/

/** @defgroup zx_spectrum Sinclair ZX Spectrum
 *
 * @{
 */

/**
 * Set the 16 colors ZX Spectrum palette.
 *
 * The ZX Spectrum palette is a 16 color RGBI palette.
 * The color components are in IGRB order.
 */
void ZX_Spectrum_set_palette(T_Components * palette);

/** @}*/

/** @defgroup cpc Amstrad CPC
 *
 * @{
 */

/**
 * Set 32 color HW Amstrad CPC palette
 *
 * The palette is in fact 27 colors, with some duplicates.
 * http://www.cpcwiki.eu/index.php/CPC_Palette
 */
void CPC_set_HW_palette(T_Components * palette);

/**
 * Check if the color is likely to be a CPC RGB tri level color
 */
int CPC_is_CPC_old_color(T_Components * col);

/**
 * Set 16 color default Amstrad CPC BASIC palette
 *
 * @note INK 14 and 15 are blinking on the real hardware.
 */
void CPC_set_default_BASIC_palette(T_Components * palette);

/**
 * compare two RGB colors
 *
 * The RGB colors are compared in order as if they were
 * converted from CPC (27 color palette), as there are several
 * mapping of the 3 level CPC RGB signals.
 *
 * @return true if the RGB colors map to the same CPC color
 */
int CPC_compare_colors(T_Components * col1, T_Components * col2);

/**
 * Convert CPC firmware color to hardware color (Gate Array)
 * @param fw_color a CPC Firmware color index (from 0 to 26)
 * @return a CPC Hardware color (from 0x40 to 0x5f)
 */
byte CPC_Firmware_to_Hardware_color(byte fw_color);

/**
 * Check AMSDOS header
 *
 * see http://www.cpcwiki.eu/index.php/AMSDOS_Header
 *
 * @param[in] file an open file
 * @param[out] loading_address the loading address from the header
 * @param[out] exec_address the execution address from the header
 * @param[out] file_length the file length written in the header
 * @return 0 if the file does not contain a valid AMSDOS header
 * @return 1 if it does.
 */
int CPC_check_AMSDOS(FILE * file, word * loading_address, word * exec_address, unsigned long * file_length);

/**
 * Write AMSDOS header
 */
int CPC_write_AMSDOS_header(FILE * file, const char * filename, const char * ext, byte type, word load_address, word save_address, word file_length);

/** @}*/

/** @defgroup decb DECB binary format
 *
 * The DECB binary format was used on 6809 based machines :
 * Tandy CoCo, Dragon 32/64 and Thomson MO/TO.
 *
 * DECB stand for Disk Extended Color Basic and was a version
 * of the Microsoft BASIC-69 (their basic for Motorola 6809)
 * @{
 */
/**
 * Add a chunk to a DECB binary file
 *
 * @param f open file
 * @param size size of the memory chunk
 * @param address load address of the memory chunk
 * @param data data to add in memory chunk
 * @return true for success
 */
int DECB_BIN_Add_Chunk(FILE * f, word size, word address, const byte * data);

/**
 * Add a chunk to a DECB binary file
 *
 * @param f open file
 * @param address run address of the binary file (LOADM,,R)
 * @return true for success
 */
int DECB_BIN_Add_End(FILE * f, word address);


/**
 * Check if the file is in the DECB Binary format
 *
 * @param f open file
 * @return 1 if the file is in DECB Binary format, or else 0
 */
int DECB_Check_binary_file(FILE * f);
/** @}*/

/** @defgroup moto Thomson MO/TO
 * Thomson MO/TO computer range
 *
 * The Thomson MO/TO computer range was based on a Motorola 6809 CPU
 * and equiped with Microsoft Basic called Basic 1.0 for the first
 * versions and then Basic 128 or Basic 512 which included a DOS
 * and more features.
 *
 * The range includes :
 * - TO7
 * - MO5/MO5E
 * - TO7/70
 * - TO9
 * - MO6/MO5NR
 * - TO8/TO8D
 * - TO9
 * - Olivetti Prodest PC128 (a Thomson MO6 clone)
 * @{
 */

/**
 * to define a specific machine in the Thomson MO/TO range of machines
 */
enum MOTO_Machine_Type {
  MACHINE_TO7,    ///< original TO7 with 8 colors
  MACHINE_TO770,  ///< the TO7-70 had 16 colors
  MACHINE_MO5,
  MACHINE_MO6,    ///<
  MACHINE_TO9,
  MACHINE_TO8     ///< TO8, TO8D and TO9+ are equivalent
};

/**
 * Graphic modes available in BASIC 128/512 with CONSOLE,,,,X instruction
 */
enum MOTO_Graphic_Mode {
  MOTO_MODE_40col = 0,  ///< 320x200 16 colors with constraints
  MOTO_MODE_80col = 1,  ///< 640x200 2 colors
  MOTO_MODE_bm4 = 2,    ///< 320x200 4 colors without constraint
  MOTO_MODE_bm16 = 3,   ///< 160x200 16 colors without constraint
};

/**
 * Checks if the file is a Thomson binary file (SAVEM/LOADM format)
 *
 * @param f a file open for reading
 * @return 0 if this is not a binary file
 * @return >0 if this is a binary file :
 * @return 1 no further details found
 * @return 2 This is likely a MAP file (SAVEP/LOADP format)
 * @return 3 This is likely a TO autoloading picture
 * @return 4 This is likely a MO5/MO6 autoloading picture
 */
int MOTO_Check_binary_file(FILE * f);

/**
 * Convert a RGB value to Thomson BGR value with gamma correction.
 */
word MOTO_gamma_correct_RGB_to_MOTO(const T_Components * color);

/**
 * Convert a Thomson BGR value to RGB values with gamma correction.
 */
void MOTO_gamma_correct_MOTO_to_RGB(T_Components * color, word bgr);

/**
 * Set MO5 Palette
 *
 * http://pulkomandy.tk/wiki/doku.php?id=documentations:devices:gate.arrays#video_generation
 * https://16couleurs.wordpress.com/2013/03/31/archeologie-infographique-le-pixel-art-pour-thomson/
 */
void MOTO_set_MO5_palette(T_Components * palette);

/**
 * Set TO7/70 Palette.
 *
 * The 8 first colors are the TO7 palette
 */
void MOTO_set_TO7_palette(T_Components * palette);

/** @}*/

/** @defgroup apple2 Apple II
 *
 * HGR and DHGR modes
 * @{
 */

/**
 * Set the 6 color Apple II HGR palette
 */
void HGR_set_palette(T_Components * palette);

/**
 * Set the 16 color Apple II DHGR palette
 */
void DHGR_set_palette(T_Components * palette);

/** @}*/

/**
 * Set the 15 color MSX palette
 */
void MSX_set_palette(T_Components * palette);

#endif
