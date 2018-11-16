/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Adrien Destugues

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

///@file oldies.h
/// functions relative to old computers (Commodore 64, Thomsons MO/TO, Amstrad CPC, ZX Spectrum, etc.)

/**
 * Save a 3 layer picture to C64 FLI format
 *
 * @param bitmap a 8000 byte buffer to store bitmap data
 * @param screen_ram a 8192 byte buffer to store the 8 screen RAMs
 * @param color_ram a 1000 byte buffer to store the color RAM
 * @param background a 200 byte buffer to store the background colors
 * @return 0 for success, 1 if the picture is less than 3 layers, 2 if the picture dimensions are not 160x200
 */
int C64_FLI(byte *bitmap, byte *screen_ram, byte *color_ram, byte *background);

int C64_FLI_enforcer(void);

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


int DECB_Check_binary_file(FILE * f);
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
