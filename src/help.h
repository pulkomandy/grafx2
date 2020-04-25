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
///@file help.h
/// Functions related to the help browser. The help data is in helpfile.h
//////////////////////////////////////////////////////////////////////////////

#ifndef __HELP_H_
#define __HELP_H_

/*!
    Called to open the help window with the keyboard shortcut.
    If the mouse is over a button, its contextual help will be displayed.
    Else, the default helpscreen will be shown.
*/
void Button_Help(int);

/*!
    Displays and runs the "Statistics" window
*/
void Button_Stats(int);

/*!
    Displays and runs the "Help / About..." window
    @param section Number of the help section page to display (equals the button number the mouse was hovering for the contextual help), -1 for the main help page.
    @param sub_section Help sub-section title (the page will be scrolled so this title is at the top).
*/
void Window_help(int section, const char * sub_section);

/// Opens a window where you can change a shortcut key(s).
void Window_set_shortcut(int action_id);

///
/// Print a line with the 'help' (6x8) font.
short Print_help(short x_pos, short y_pos, const char *line, char line_type, short link_position, short link_size);

// Nom de la touche actuallement assignÃ©e Ã  un raccourci d'aprÃ¨s son numÃ©ro
// de type 0x100+BOUTON_* ou SPECIAL_*
const char * Keyboard_shortcut_value(word shortcut_number);

///
/// Browse the complete list of shortcuts and ensure that a key only triggers
/// one of them.
void Remove_duplicate_shortcuts(void);

#endif
