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
///@file pages.h
/// Handler for the Undo/Redo system.
//////////////////////////////////////////////////////////////////////////////

#ifndef _PAGES_H_
#define _PAGES_H_

///
/// Pointer to the image to read, while drawing. It's either the last history
/// layer page when FX feedback is on, or the history page before it
/// when FX feedback is off.
extern byte * FX_feedback_screen;



//////////////////////////////////////////////////////////////////////////
/////////////////////////// BACKUP ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// The pixels of visible layers, flattened copy, used for no-feedback effects.
extern T_Bitmap Main_visible_image_backup;
/** The index of visible pixels Main page ::T_Document.visible_image.
 *
 * Points to the right layer.
 */
extern T_Bitmap Main_visible_image_depth_buffer;

///
/// INDIVIDUAL PAGES
///

void Download_infos_page_main(T_Page * page);
/// Add a new layer to latest page of a list. Returns 0 on success.
byte Add_layer(T_List_of_pages *list, int layer);
/// Delete a layer from the latest page of a list. Returns 0 on success.
byte Delete_layer(T_List_of_pages *list, int layer);
/// Merges the current layer onto the one below it.
byte Merge_layer(void);
/// Backs up a layer, unless it's already different from previous history step.
int Dup_layer_if_shared(T_Page * page, int layer);

void Upload_infos_page(T_Document * doc);


///
/// LISTS OF PAGES
///

void Init_list_of_pages(T_List_of_pages * list);
// private
int Allocate_list_of_pages(T_List_of_pages * list);
void Backward_in_list_of_pages(T_List_of_pages * list);
void Advance_in_list_of_pages(T_List_of_pages * list);
void Free_last_page_of_list(T_List_of_pages * list);
int Create_new_page(T_Page * new_page,T_List_of_pages * current_list, int layer);
void Change_page_number_of_list(T_List_of_pages * list,int number);
void Free_page_of_a_list(T_List_of_pages * list);



///
/// BACKUP HIGH-LEVEL FUNCTIONS
///

static const int LAYER_NONE = -1;
static const int LAYER_ALL = -2;

int Init_all_backup_lists(enum IMAGE_MODES image_mode,int width,int height);
void Set_number_of_backups(int nb_backups);
int Backup_new_image(int layers,int width,int height);
int Backup_with_new_dimensions(int width,int height);
///
/// Resizes a backup step in-place (doesn't add a Undo/Redo step).
/// Should only be called after an actual backup, because it loses the current.
/// pixels. This function is meant to be used from within Lua scripts.
int Backup_in_place(int width,int height);
/// Backup the spare image, the one you don't see.
void Backup_the_spare(int layer);
int Backup_and_resize_the_spare(int width,int height);
/// Backup with a new copy for the working layer, and references for all others.
void Backup(void);
/// Backup with a new copy of some layers (the others are references).
void Backup_layers(int layer);
void Undo(void);
void Redo(void);
void Free_current_page(void); // 'Kill' button
void End_of_modification(void);

void Update_depth_buffer(void);
void Redraw_layered_image(void);
void Redraw_current_layer(void);

void Update_screen_targets(void);
/// Update all the special image buffers, if necessary.
int Update_buffers(int width, int height);
int Update_spare_buffers(int width, int height);
void Redraw_spare_image(void);
///
/// Must be called after changing the head of Main_backups list, or
/// Main_current_layer
void Update_FX_feedback(byte with_feedback);

void Switch_layer_mode(enum IMAGE_MODES new_mode);

///
/// STATISTICS
///

/// Total number of unique bitmaps (layers, animation frames, backups)
extern long  Stats_pages_number;
/// Total memory used by bitmaps (layers, animation frames, backups)
extern long long  Stats_pages_memory;

#endif
