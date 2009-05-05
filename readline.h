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
///@file readline.h
/// Text input functions.
//////////////////////////////////////////////////////////////////////////////

///
/// Lets the user input a line of text, exit by Esc or Return.
/// @param x_pos        Coordinates of input, in window coordinates before scaling.
/// @param y_pos        Coordinates of input, in window coordinates before scaling.
/// @param str          The original string value (will be modified, unless user cancels.
/// @param visible_size Number of characters visible and editable.
/// @param input_type   0=string, 1=number, 2=filename (255 editable characters)
/// @return             0 if user cancelled (esc), 1 if accepted (return)
byte Readline(word x_pos,word y_pos,char * str,byte visible_size,byte input_type);

///
/// Lets the user input a line of text, exit by Esc or Return.
/// @param x_pos        Coordinates of input, in window coordinates before scaling.
/// @param y_pos        Coordinates of input, in window coordinates before scaling.
/// @param str          The original string value (will be modified, unless user cancels.
/// @param visible_size Number of characters visible.
/// @param max_size     Number of characters editable.
/// @param input_type   0=string, 1=number, 2=filename (255 editable characters)
/// @return             0 if user cancelled (esc), 1 if accepted (return)
byte Readline_ex(word x_pos,word y_pos,char * str,byte visible_size,byte max_size, byte input_type);
