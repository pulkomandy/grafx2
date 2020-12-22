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
//////////////////////////////////////////////////////////////////////////////
///@file loadsavefuncs.h
/// helper functions for load/save
//////////////////////////////////////////////////////////////////////////////

#ifndef LOADSAVEFUNCS_H_INCLUDED
#define LOADSAVEFUNCS_H_INCLUDED

/// For use by Save_XXX() functions
FILE * Open_file_write(T_IO_Context *context);

/// For use by Load_XXX() and Test_XXX() functions
FILE * Open_file_read(T_IO_Context *context);

/// For use by Load_XXX() and Test_XXX() functions
FILE * Open_file_read_with_alternate_ext(T_IO_Context *context, const char * ext);

/// For use by Save_XXX() functions
void Remove_file(T_IO_Context *context);

/// For use by Save_XXX() functions
FILE * Open_file_write_with_alternate_ext(T_IO_Context *context, const char * ext);

#define Write_one_byte(file, b) do { if (!Write_byte(file,(b))) File_error=1; } while (0)

void Palette_256_to_64(T_Palette palette);
void Palette_64_to_256(T_Palette palette);

word Current_layer_count_used_colors(T_IO_Context *context, dword *usage);

#endif
