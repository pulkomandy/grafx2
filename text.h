/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Adrien Destugues
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
///@file text.h
/// Functions related to rendering text as a brush, using TrueType or SFont.
//////////////////////////////////////////////////////////////////////////////

/// Initialization of text settings, needs to be called once on program startup.
void Init_text(void);
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
/// Returns true on success.
byte *Render_text(const char *str, int font_number, int size, int antialias, int bold, int italic, int *width, int *height);

/// Finds a label to display for a font declared with ::Add_font().
char * Font_label(int index);
/// Finds the filename of a font declared with ::Add_font().
char * Font_name(int index);
/// Returns true if the font of this number is TrueType, false if it's a SFont bitmap.
char * TrueType_font(int index);
///
/// Number of fonts declared with a series of ::Add_font(). This is public for
/// convenience, but functionaly it is read-only.
extern int Nb_fonts;
