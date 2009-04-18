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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
void Button_Help(void);

/*!
    Displays and runs the "Statistics" window
*/
void Button_Stats(void);

/*!
    Displays and runs the "Help / About..." window
    @param section Number of the help section page to display (equals the button number the mouse was hovering for the contextual help), -1 for the main help page.
    @param sub_section Help sub-section title (the page will be scrolled so this title is at the top).
*/
void Window_help(int section, const char * sub_section);

#endif

