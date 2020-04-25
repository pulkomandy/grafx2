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
///@file text.h
/// Functions related to rendering text as a brush, using TrueType or SFont.
//////////////////////////////////////////////////////////////////////////////

/// Initialization of text settings, needs to be called once on program startup.
void Init_text(void);
/// text settings cleanup. called on program shutdown
void Uninit_text(void);
/// Returns true if text.c was compiled with TrueType support.
int TrueType_is_supported(void);
/// Add a new font to the list to propose to the user.
void Add_font(const char *name);
///
/// Creates a brush, from the parameters given:
/// @param str         The text to render
/// @param font_number The index of the font to use. Pass 0 for the first font you declared with ::Add_font(), 1 for the second etc.
/// @param size        The size in points (unused for bitmap fonts)
/// @param antialias   Boolean, true to use antialiasing in TrueType
/// @param bold        Boolean, true to use bold rendering in TrueType
/// @param italic      Boolean, true to use italic rendering in TrueType
/// @param width       Returns the width of the created brush, in pixels.
/// @param height      Returns the height of the created brush, in pixels.
/// @param palette     Returns the custom palette for the brush.
/// Returns true on success.
byte *Render_text(const char *str, int font_number, int size, int antialias, int bold, int italic, int *width, int *height, T_Palette palette);

/// Finds a label to display for a font declared with ::Add_font().
const char * Font_label(int index);
/// Finds the filename of a font declared with ::Add_font().
const char * Font_name(int index);
/// Returns true if the font of this number is TrueType, false if it's a SFont bitmap.
int TrueType_font(int index);
///
/// Number of fonts declared with a series of ::Add_font()
int Font_count(void);
