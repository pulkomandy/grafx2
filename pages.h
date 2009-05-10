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
/////////////////////////// GESTION DU BACKUP ////////////////////////////
//////////////////////////////////////////////////////////////////////////


  ///
  /// GESTION DES PAGES
  ///

void Init_page(T_Page * page);
void Download_infos_page_main(T_Page * page);
void Upload_infos_page_main(T_Page * page);
void Download_infos_page_spare(T_Page * page);
void Upload_infos_page_spare(T_Page * page);
void Download_infos_backup(T_List_of_pages * list);
void Free_a_page(T_Page * page);
void Copy_S_page(T_Page * dest,T_Page * source);
int Size_of_a_page(T_Page * page);



  ///
  /// GESTION DES LISTES DE PAGES
  ///

void Init_list_of_pages(T_List_of_pages * list);
int Allocate_list_of_pages(T_List_of_pages * list,int size);
void Free_a_list_of_pages(T_List_of_pages * list);
int Size_of_a_list_of_pages(T_List_of_pages * list);
void Backward_in_list_of_pages(T_List_of_pages * list);
void Advance_in_list_of_pages(T_List_of_pages * list);
int New_page_is_possible(T_Page * new_page,T_List_of_pages * current_list,T_List_of_pages * secondary_list);
void Free_last_page_of_list(T_List_of_pages * list);
void Create_new_page(T_Page * new_page,T_List_of_pages * current_list,T_List_of_pages * secondary_list);
void Change_page_number_of_list(T_List_of_pages * list,int number);
void Free_page_of_a_list(T_List_of_pages * list);



  ///
  /// GESTION DES BACKUPS
  ///

int Init_all_backup_lists(int size,int width,int height);
void Set_number_of_backups(int nb_backups);
int Backup_with_new_dimensions(int upload,int width,int height);
int Backup_and_resize_the_spare(int width,int height);
void Backup(void);
void Undo(void);
void Redo(void);
void Free_current_page(void);
void Exchange_main_and_spare(void);



  ///
  /// GESTION DES EMPRUNTS DE MEMOIRE DE PAGE
  ///

int Can_borrow_memory_from_page(int size);
void * Borrow_memory_from_page(int size);



#endif
