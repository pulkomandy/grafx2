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
///@file readline.h
/// Text input functions.
//////////////////////////////////////////////////////////////////////////////

enum INPUT_TYPE
{
  INPUT_TYPE_STRING=0,  ///< Any string
  INPUT_TYPE_INTEGER=1, ///< Decimal integer
  INPUT_TYPE_FILENAME=2,///< Filename
  INPUT_TYPE_DECIMAL=3, ///< Decimal value
  INPUT_TYPE_HEXA=4,    ///< Hexadecimal integer
};

///
/// Lets the user input a line of text, exit by Esc or Return.
/// @param x_pos        Coordinates of input, in window coordinates before scaling.
/// @param y_pos        Coordinates of input, in window coordinates before scaling.
/// @param str          The original string value (will be modified, unless user cancels.
/// @param visible_size Number of characters visible and editable.
/// @param input_type   one of enum ::INPUT_TYPE
/// @return             0 if user cancelled (esc), 1 if accepted (return)
byte Readline(word x_pos, word y_pos, char * str, byte visible_size,
              enum INPUT_TYPE input_type);

///
/// Lets the user input a line of text, exit by Esc or Return.
/// @param x_pos        Coordinates of input, in window coordinates before scaling.
/// @param y_pos        Coordinates of input, in window coordinates before scaling.
/// @param str          The original string value (will be modified, unless user cancels.
/// @param visible_size Number of characters visible.
/// @param max_size     Number of characters editable.
/// @param input_type   one of enum ::INPUT_TYPE
/// @param decimal_places Number of decimal places (used only with decimal type)
/// @return             0 if user cancelled (esc), 1 if accepted (return)
byte Readline_ex(word x_pos, word y_pos, char * str,
                 byte visible_size, byte max_size,
                 enum INPUT_TYPE input_type, byte decimal_places);

///
/// Lets the user input a line of text, exit by Esc or Return.
/// @param x_pos, y_pos Coordinates of input, in window coordinates before scaling.
/// @param str          The original string value. Will be modified,
///                     unless user cancels or @p unicode_str is not NULL.
/// @param unicode_str  The original unicode string value or NULL for ANSI editing.
///                     Will be modified if not NULL, unless user cancels.
/// @param visible_size Number of characters visible.
/// @param max_size     Number of characters editable.
/// @param input_type   one of enum ::INPUT_TYPE
/// @param decimal_places Number of decimal places (used only with decimal type)
/// @return             0 : user cancelled (esc)
/// @return             1 : user accepted (return)
byte Readline_ex_unicode(word x_pos, word y_pos, char * str, word * unicode_str,
                         byte visible_size, byte max_size,
                         enum INPUT_TYPE input_type, byte decimal_places);

///
/// Converts a double to string.
/// @param str            Target string, should be pre-allocated and at least 40 characters, to be safe.
/// @param value          The number to convert
/// @param decimal_places Number of decimal places to keep. 15 seems the maximum.
/// @param min_positions  Minimum number of characters: Will pad spaces on the left to meet this minimum.
void Sprint_double(char *str, double value, byte decimal_places, byte min_positions);
