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
///@file fileseltools.h
/// Utility functions for the Fileselector window
//////////////////////////////////////////////////////////////////////////////
#ifndef FILESELTOOLS_H_INCLUDED
#define FILESELTOOLS_H_INCLUDED

T_Fileselector_item * Add_element_to_list(T_Fileselector *list, const char * full_name, const char *short_name, enum FSOBJECT_TYPE type, enum ICON_TYPES icon);

///
/// Formats a display name for a file, directory, or similar name (drive, volume).
/// @param fname full file name from the file system
/// @param max_length numbers of characters to display
/// @param type 0 for file, 1 for directory, 2 for drive
/// @return a pointer to a single static buffer of maximum 40 characters
///         including the '\\0'.
char * Format_filename(const char * fname, word max_length, int type);

///
/// Formats a display name for a file, directory, or similar name (drive, volume).
/// @param fname full file name from the file system
/// @param max_length numbers of characters to display
/// @param type 0 for file, 1 for directory, 2 for drive
/// @return a pointer to a single static buffer of maximum 40 wide characters
///         including the '\\0'.
word * Format_filename_unicode(const word * fname, word max_length, int type);

#endif
