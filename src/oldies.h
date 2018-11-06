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
/// C64 FLI functions

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

