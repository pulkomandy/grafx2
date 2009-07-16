/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007 Adrien Destugues
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

//////////////////////////////////////////////////////////////////////////////
///@file filesel.h
/// Fileselector window, used for loading and saving images and brushes.
//////////////////////////////////////////////////////////////////////////////
#ifndef __FILESEL_H__
#define __FILESEL_H__

#include "struct.h"

byte Button_Load_or_Save(byte load, byte image);

void Add_element_to_list(T_Fileselector *list, const char * fname, int type);
///
/// Formats a display name for a file, directory, or similar name (drive, volume).
/// The returned value is a pointer to a single static buffer of 19 characters
/// including the '\0'.
char * Format_filename(const char * fname, int type);

void Free_fileselector_list(T_Fileselector *list);

void Sort_list_of_files(T_Fileselector *list);

void Recount_files(T_Fileselector *list);

T_Fileselector_item * Get_item_by_index(T_Fileselector *list, short index);

short Find_file_in_fileselector(T_Fileselector *list, char * fname);

#endif
