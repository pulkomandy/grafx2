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
///@file filesel.h
/// Fileselector window, used for loading and saving images and brushes.
//////////////////////////////////////////////////////////////////////////////
#ifndef __FILESEL_H__
#define __FILESEL_H__

#include "struct.h"
#include "loadsave.h"

///
/// Launch the Load/Save dialog (file selector)
/// @param settings the settings associated with this file selector
/// @param load 1 for a load dialog, 0 for a save
/// @param context the IO context
/// @return 0 if the user cancelled the load/save
/// @return 1 if the user want to load/save a file
byte Button_Load_or_Save(T_Selector_settings *settings, byte load, T_IO_Context *context);

void Free_fileselector_list(T_Fileselector *list);

void Sort_list_of_files(T_Fileselector *list);

///
/// Fast access to a list item.
/// @param list the linked list
/// @param index the item index to access
/// @return the item at the index position in the list
/// @return NULL in case of error
T_Fileselector_item * Get_item_by_index(T_Fileselector *list, unsigned short index);

void Read_list_of_drives(T_Fileselector *list, byte name_length);

/// Look for a file name in a file selector list
/// @param list  The file selector list
/// @param fname The file name to search for
/// @return the index of the file
/// @return -1 if the file name was not found
short Find_file_in_fileselector(T_Fileselector *list, const char * fname);

void Locate_list_item(T_List_button * list, short selected_item);

int Quicksearch_list(T_List_button * list, T_Fileselector * selector);

void Reset_quicksearch(void);

#endif
