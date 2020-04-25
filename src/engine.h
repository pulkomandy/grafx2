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
///@file engine.h
/// Utility functions for the menu and all windows.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "struct.h"

void Main_handler          (void);
void Draw_menu_button      (byte btn_number,byte pressed);
void Unselect_button       (int btn_number);
void Select_button         (int btn_number,byte click);
void Open_window           (word width,word height, const char * title);
void Close_window          (void);

void Open_popup            (word x_pos, word y_pos, word width, word height);
void Close_popup           (void);

void Window_draw_normal_bouton(word x_pos,word y_pos,word width,word height,
  const char * title,byte undersc_letter,byte clickable);
void Window_select_normal_button(word x_pos,word y_pos,word width,word height);
void Window_unselect_normal_button(word x_pos,word y_pos,word width,word height);
void Window_draw_palette_bouton(word x_pos,word y_pos);

void Compute_slider_cursor_length(T_Scroller_button * button);
void Window_draw_slider(T_Scroller_button * button);
void Window_draw_scroller_button(T_Scroller_button * button);

void Window_input_content(T_Special_button * button, const char * content);
void Window_clear_input_button(T_Special_button * button);
void Window_draw_input_bouton(word x_pos, word y_pos, word width_in_characters);

T_Normal_button * Window_set_normal_button(word x_pos, word y_pos,
  word width, word height, const char * title, byte undersc_letter,
  byte clickable, word shortcut);

T_Normal_button * Window_set_repeatable_button(word x_pos, word y_pos,
  word width, word height, const char * title, byte undersc_letter,
  byte clickable, word shortcut);

T_Palette_button * Window_set_palette_button(word x_pos, word y_pos);
void Window_clear_tags(void);
void Tag_color_range(byte start, byte end);

T_Scroller_button * Window_set_scroller_button(word x_pos, word y_pos,
  word height, word nb_elements, word nb_elements_visible,
  word initial_position);

T_Scroller_button * Window_set_horizontal_scroller_button(word x_pos, word y_pos,
  word width, word nb_elements, word nb_elements_visible,
  word initial_position);

T_Special_button * Window_set_special_button(word x_pos, word y_pos,
  word width, word height, word shortcut);

T_Special_button * Window_set_input_button_s(word x_pos,word y_pos,
  word width_in_characters, word shortcut);

T_Special_button * Window_set_input_button(word x_pos, word y_pos,
  word width_in_characters);

T_Dropdown_button * Window_set_dropdown_button(word x_pos, word y_pos,
  word width, word height, word dropdown_width, const char *label,
  byte display_choice, byte display_centered, byte display_arrow,
  byte active_button,byte bottom_up);

/// Adds an item to a dropdown menu
void Window_dropdown_add_item(T_Dropdown_button * dropdown, word btn_number,
  const char *label);

void Window_dropdown_clear_items(T_Dropdown_button * dropdown);

///
/// Displays a dropped-down menu and handles the UI logic until the user
/// releases a mouse button.
/// This function then clears the dropdown and returns the selected item,
/// or NULL if the user wasn't highlighting an item when he closed.
T_Dropdown_choice * Dropdown_activate(T_Dropdown_button *button, short off_x, short off_y);

T_List_button * Window_set_list_button(T_Special_button * entry_button,
  T_Scroller_button * scroller, Func_draw_list_item draw_list_item, byte color_index);
void Window_redraw_list(T_List_button * list);
byte Window_click_in_rectangle(short start_x, short start_y, short end_x,
  short end_y);
short Wait_click_in_palette(T_Palette_button * button);
short Window_normal_button_onclick(word x_pos, word y_pos, word width, word height, short btn_number);
void Get_color_behind_window(byte * color, byte * click);

short Window_clicked_button(void);
int Button_under_mouse(void);
short Window_get_button_shortcut(void);
short Window_get_clicked_button(void);

/// Remaps all the colors from the window background buffers from row Min_Y to row Max_Y
void Remap_window_backgrounds(const byte * conversion_table, int Min_Y, int Max_Y);
/// Remaps only UI elements (Windows + Menu) in all window background buffers
void Remap_UI_in_window_backgrounds(const byte * conversion_table);

void Pixel_background(int x_pos, int y_pos, byte color);
///
/// Updates the status bar line with a color number.
/// Used when hovering the menu palette.
void Status_print_palette_color(byte color);

/// Puts the user in wait mode for the specified time ( in 1/100s),
/// though the mouse still works.
void Delay_with_active_mouse(int delay);

///
/// Based on which toolbars are visible, updates their offsets and
/// computes ::Menu_height and ::Menu_Y
void Compute_menu_offsets(void);

///
/// Shows or hides a tolbar from the menu.
/// If with_redraw is set to zero, the caller should
/// redraw itself using Display_menu() and Display_all_screen().
void Set_bar_visibility(word bar, int visible, int with_redraw);

///
/// Checks if the current menu toolbars suit the current image type :
/// layered vs anim. If they don't fit, swap the toolbars and return 1:
/// The caller is then responsible for refreshing the screen by calling
/// Display_menu() and Display_all_screen()
int Check_menu_mode(void);

#endif
