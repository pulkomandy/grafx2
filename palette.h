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
///@file palette.h
/// Palette screen, and some palette-related high-level functions.
//////////////////////////////////////////////////////////////////////////////

/// Open the palette menu and handles everything inside it.
void Button_Palette(void);
/// Open the secondary palette menu and handles it.
void Button_Secondary_palette(void);

/// Choose the number of graduations for RGB components, from 2 to 256.
void Set_palette_RGB_scale(int);

///
/// Scale a component (R, G or B) according to the current RGB graduations.
/// Returns the resulting value, in the [0-255] range.
byte Round_palette_component(byte comp);

/*!
  Adds 4 menu colors in the current palette.
  @param color_usage An up-to-date color usage table (byte[256]) (read only)
  @param not_picture 0 if the caller is the palette screen, 1 if it's a preview in the file selector.
*/
void Set_nice_menu_colors(dword * color_usage,int not_picture);
