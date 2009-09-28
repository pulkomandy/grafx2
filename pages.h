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
///@file pages.h
/// Handler for the Undo/Redo system.
//////////////////////////////////////////////////////////////////////////////

#ifndef _PAGES_H_
#define _PAGES_H_


//////////////////////////////////////////////////////////////////////////
/////////////////////////// BACKUP ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


///
/// INDIVIDUAL PAGES
///

void Download_infos_page_main(T_Page * page);
void Upload_infos_page_main(T_Page * page);

// private
T_Page * New_page(byte nb_layers);
void Download_infos_page_spare(T_Page * page);
void Upload_infos_page_spare(T_Page * page);
void Download_infos_backup(T_List_of_pages * list);
void Clear_page(T_Page * page);
void Copy_S_page(T_Page * dest,T_Page * source);



///
/// LISTS OF PAGES
///

void Init_list_of_pages(T_List_of_pages * list);
// private
int Allocate_list_of_pages(T_List_of_pages * list);
void Backward_in_list_of_pages(T_List_of_pages * list);
void Advance_in_list_of_pages(T_List_of_pages * list);
void Free_last_page_of_list(T_List_of_pages * list);
int Create_new_page(T_Page * new_page,T_List_of_pages * current_list, byte layer_mask);
void Change_page_number_of_list(T_List_of_pages * list,int number);
void Free_page_of_a_list(T_List_of_pages * list);



///
/// BACKUP HIGH-LEVEL FUNCTIONS
///

int Init_all_backup_lists(int width,int height);
void Set_number_of_backups(int nb_backups);
int Backup_with_new_dimensions(int upload,int width,int height);
int Backup_and_resize_the_spare(int width,int height);
/// Backup with a new copy for the working layer, and references for all others.
void Backup(void);
/// Backup with a new copy of some layers (the others are references).
void Backup_layers(byte layer_mask);
void Undo(void);
void Redo(void);
void Free_current_page(void); // 'Kill' button
void Exchange_main_and_spare(void);
void End_of_modification(void);

#endif
