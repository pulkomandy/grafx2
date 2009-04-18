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
///@file loadsave.h
/// Saving and loading different picture formats.
/// Also handles showing the preview in fileselectors.
//////////////////////////////////////////////////////////////////////////////

void Pixel_load_in_current_screen(word x_pos,word y_pos,byte color);
void Pixel_load_in_preview      (word x_pos,word y_pos,byte color);
void Pixel_load_in_brush       (word x_pos,word y_pos,byte color);

void filename_complet(char * filename, byte is_colorix_format);

///
/// High-level picture loading function.
/// Handles loading an image or a brush, or previewing only.
/// @param image true if the fileselector is the one for loading images (not brush)
void Load_image(byte image);
///
/// High-level picture saving function.
/// @param image true if the image should be saved (instead of the brush)
void Save_image(byte image);

/// Data for an image file format.
typedef struct {
  char *Extension;  ///< Three-letter file extension
  Func_action Test; ///< Function which tests if the file is of this format
  Func_action Load; ///< Function which loads an image of this format
  Func_action Save; ///< Function which saves an image of this format
  byte Backup_done; ///< Boolean, true if this format saves all the image, and considers it backed up. Set false for formats which only save the palette.
  byte Comment;     ///< This file format allows a text comment
} T_Format;

/// Array of the known file formats
extern T_Format File_formats[NB_KNOWN_FORMATS];

///
/// Function which attempts to save backups of the images (main and spare),
/// called in case of SIGSEGV. 
void Image_emergency_backup(void);
