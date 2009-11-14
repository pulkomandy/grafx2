/* vim:expandtab:ts=2 sw=2:
*/
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
///@file loadsave.h
/// Saving and loading different picture formats.
/// Also handles showing the preview in fileselectors.
//////////////////////////////////////////////////////////////////////////////

void Pixel_load_in_current_screen (word x_pos, word y_pos, byte color);
void Pixel_load_in_preview (word x_pos, word y_pos, byte color);
void Pixel_load_in_brush (word x_pos, word y_pos, byte color);

void Get_full_filename(char * filename, byte is_colorix_format);

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
  byte Identifier;         ///< Identifier for this format in enum :FILE_FORMATS
  char *Label;             ///< Five-letter label
  Func_action Test;        ///< Function which tests if the file is of this format
  Func_action Load;        ///< Function which loads an image of this format
  Func_action Save;        ///< Function which saves an image of this format
  byte Palette_only;       ///< Boolean, true if this format saves/loads only the palette.
  byte Comment;            ///< This file format allows a text comment
  byte Supports_layers;    ///< Boolean, true if this format preserves layers on saving
  char *Default_extension; ///< Default file extension
  char *Extensions;        ///< List of semicolon-separated file extensions
} T_Format;

/// Array of the known file formats
extern T_Format File_formats[];

///
/// Function which attempts to save backups of the images (main and spare),
/// called in case of SIGSEGV.
void Image_emergency_backup(void);

/// Pixel ratio of last loaded image: one of :PIXEL_SIMPLE, :PIXEL_WIDE or :PIXEL_TALL
extern enum PIXEL_RATIO Ratio_of_loaded_image;

T_Format * Get_fileformat(byte format);

// -- File formats

#ifndef __no_pnglib__
#define NB_KNOWN_FORMATS 19 ///< Total number of known file formats.
#else
// Without pnglib
#define NB_KNOWN_FORMATS 18 ///< Total number of known file formats.
#endif

// =================================================================
// What follows here are the definitions of functions and data
// useful for fileformats.c, miscfileformats.c etc.
// =================================================================

// This is here and not in fileformats.c because the emergency save uses it...
#pragma pack(1)
typedef struct
{
  byte Filler1[6];
  word Width;
  word Height;
  byte Filler2[118];
  T_Palette Palette;
} T_IMG_Header;
#pragma pack()

// Data for 24bit loading

typedef void (* Func_24b_display) (short,short,byte,byte,byte);

extern int Image_24b;
extern T_Components * Buffer_image_24b;
extern Func_24b_display Pixel_load_24b;

void Init_preview_24b(short width,short height,long size,int format);
void Pixel_load_in_24b_preview(short x_pos,short y_pos,byte r,byte g,byte b);

//

extern enum PIXEL_RATIO Ratio_of_loaded_image;

void Set_file_error(int value);
void Init_preview(short width,short height,long size,int format,enum PIXEL_RATIO ratio);

void Init_write_buffer(void);
void Write_one_byte(FILE *file, byte b);
void End_write(FILE *file);

void Remap_fileselector(void);
