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
///@file loadsave.h
/// Saving and loading different picture formats.
/// Also handles showing the preview in fileselectors.
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

#include <stdio.h>
#include "gfx2surface.h"

enum CONTEXT_TYPE {
  CONTEXT_MAIN_IMAGE,
  CONTEXT_BRUSH,
  CONTEXT_PREVIEW,
  CONTEXT_PREVIEW_PALETTE,
  CONTEXT_SURFACE,
  CONTEXT_PALETTE
};

/// Data for a cycling color series. Heavily cloned from T_Gradient_array.
typedef struct
{
  byte Start;    ///< First color
  byte End;      ///< Last color
  byte Inverse;  ///< Boolean, true if the gradient goes in descending order
  byte Speed;    ///< Frequency of cycling, from 1 (slow) to COLOR_CYCLING_SPEED_MAX (fast)
} T_Color_cycle;

typedef struct
{
  /// Kind of context. Internally used to differentiate the "sub-classes"
  enum CONTEXT_TYPE Type;
  
  // File properties
  
  char * File_name;         ///< File name in UTF-8 (or short ASCII file nmae under win32)
  word * File_name_unicode; ///< Wide character version of the filename
  char * File_directory;    ///< Directory. If NULL File_name should be the full path name
  byte Format;
  
  // Image properties
  
  T_Palette Palette;
  short Width;
  short Height;
  short Original_width;   /// Size of the whole image in case of PREVIEW of a thumbnail
  short Original_height;
  int  Nb_layers;
  char Comment[COMMENT_SIZE+1];
  byte Background_transparent;
  byte Transparent_color;
  byte bpp;
  /// Pixel ratio of the image
  enum PIXEL_RATIO Ratio;
  
  /// Load/save address of first pixel
  byte *Target_address;
  /// Pitch: Difference of addresses between one pixel and the one just "below" it
  long Pitch;
  
  /// Original file name, stored in GIF file
  char * Original_file_name;
  /// Original file directory, stored in GIF file
  char * Original_file_directory;

  byte Color_cycles;
  T_Color_cycle Cycle_range[16];

  /// Internal: during load, marks which layer is being loaded.
  int Current_layer;

  /// Internal: Used to mark truecolor images on loading. Only used by preview.
  //byte Is_truecolor;
  /// Internal: Temporary RGB buffer when loading 24bit images
  T_Components *Buffer_image_24b;
  
  /// Internal: Temporary buffer when saving the flattened copy of something
  byte *Buffer_image;
  
  // Internal: working data for preview case
  short Preview_factor_X;
  short Preview_factor_Y;
  short Preview_pos_X;
  short Preview_pos_Y;
  byte *Preview_bitmap;
  byte  Preview_usage[256];
  
  // Internal: returned surface for Surface case
  T_GFX2_Surface * Surface;

} T_IO_Context;

#define PREVIEW_WIDTH  120
#define PREVIEW_HEIGHT  80

/// Type of a function that can be called for a T_IO_Context. Kind of a method.
typedef void (* Func_IO_Test) (T_IO_Context *, FILE *);
typedef void (* Func_IO) (T_IO_Context *);

/*
void Pixel_load_in_current_screen (word x_pos, word y_pos, byte color);
void Pixel_load_in_preview (word x_pos, word y_pos, byte color);
void Pixel_load_in_brush (word x_pos, word y_pos, byte color);
*/

// Setup for loading a preview in fileselector
void Init_context_preview(T_IO_Context * context, const char *file_name, const char *file_directory);
// Setup for loading/saving the current main image
void Init_context_layered_image(T_IO_Context * context, const char *file_name, const char *file_directory);
// Setup for loading/saving an intermediate backup
void Init_context_backup_image(T_IO_Context * context, const char *file_name, const char *file_directory);
// Setup for loading/saving the flattened version of current main image
void Init_context_flat_image(T_IO_Context * context, const char *file_name, const char *file_directory);
// Setup for loading/saving the user's brush
void Init_context_brush(T_IO_Context * context, const char *file_name, const char *file_directory);
// Setup for saving an arbitrary undo/redo step, from either the main or spare page. 
void Init_context_history_step(T_IO_Context * context, T_Page *page);
// Setup for loading an image into a new GFX2 surface.
void Init_context_surface(T_IO_Context * context, const char *file_name, const char *file_directory);

// Cleans up resources (currently: the 24bit buffer) 
void Destroy_context(T_IO_Context *context);

///
/// High-level picture loading function.
void Load_image(T_IO_Context *context);

///
/// High-level picture saving function.
void Save_image(T_IO_Context *context);

///
/// Checks if there are any pending safety backups, and then opens them.
/// Returns 0 if there were none
/// Returns non-zero if some backups were loaded.
int Check_recovery(void);

/// Makes a safety backup periodically.
void Rotate_safety_backups(void);

/// Remove safety backups. Need to call on normal program exit.
void Delete_safety_backups(void);

/// Data for an image file format.
typedef struct {
  enum FILE_FORMATS Identifier; ///< Identifier for this format
  const char *Label;       ///< Five-letter label
  Func_IO_Test Test;       ///< Function which tests if the file is of this format
  Func_IO Load;            ///< Function which loads an image of this format
  Func_IO Save;            ///< Function which saves an image of this format
  byte Palette_only;       ///< Boolean, true if this format saves/loads only the palette.
  byte Comment;            ///< This file format allows a text comment
  byte Supports_layers;    ///< Boolean, true if this format preserves layers on saving
  const char *Default_extension; ///< Default file extension
  const char *Extensions;  ///< List of semicolon-separated file extensions
} T_Format;

/// Array of the known file formats
extern const T_Format File_formats[];

///
/// Function which attempts to save backups of the images (main and spare),
/// called in case of SIGSEGV.
/// It will save an image only if it has just one layer... otherwise,
/// the risk of flattening a layered image (or saving just one detail layer)
/// is too high.
void Image_emergency_backup(void);

///
/// Load an arbitrary Surface.
/// @param filename file to load.
/// @param directory path of the file to load. if NULL, filename have to be a full path name
/// @param gradients Pass the address of a target T_Gradient_array if you want the gradients, NULL otherwise
T_GFX2_Surface * Load_surface(const char *filename, const char *directory, T_Gradient_array *gradients);


/*
/// Pixel ratio of last loaded image: one of :PIXEL_SIMPLE, :PIXEL_WIDE or :PIXEL_TALL
extern enum PIXEL_RATIO Ratio_of_loaded_image;
*/

const T_Format * Get_fileformat(byte format);

// -- File formats

/// Total number of known file formats
unsigned int Nb_known_formats(void);

// Internal use

/// Generic allocation and similar stuff, done at beginning of image load, as soon as size is known.
void Pre_load(T_IO_Context *context, short width, short height, long file_size, int format, enum PIXEL_RATIO ratio, byte bpp);
/// Fill the entire current layer/frame of an image being loaded with a color.
void Fill_canvas(T_IO_Context *context, byte color);

/// Query the color of a pixel (to save)
byte Get_pixel(T_IO_Context *context, short x, short y);
/// Set the color of a pixel (on load)
void Set_pixel(T_IO_Context *context, short x, short y, byte c);
/// Set the color of a 24bit pixel (on load)
void Set_pixel_24b(T_IO_Context *context, short x, short y, byte r, byte g, byte b);
/// Function to call when need to switch layers.
void Set_loading_layer(T_IO_Context *context, int layer);
/// Function to call when need to switch layers.
void Set_saving_layer(T_IO_Context *context, int layer);
/// Function to call when loading an image's duration
void Set_frame_duration(T_IO_Context *context, int duration);
/// Function to call to get an image's duration for saving
int Get_frame_duration(T_IO_Context *context);
/// Function to set a specific image mode
void Set_image_mode(T_IO_Context *context, enum IMAGE_MODES mode);
/// get the current image mode
enum IMAGE_MODES Get_image_mode(T_IO_Context *context);

// =================================================================
// What follows here are the definitions of functions and data
// useful for fileformats.c, miscfileformats.c etc.
// =================================================================

// This is here and not in fileformats.c because the emergency save uses it...
typedef struct
{
  byte Filler1[6];
  word Width;
  word Height;
  byte Filler2[118];
  T_Palette Palette;
} T_IMG_Header;

// Data for 24bit loading

/*
typedef void (* Func_24b_display) (short,short,byte,byte,byte);

extern int Image_24b;
extern T_Components * Buffer_image_24b;
extern Func_24b_display Pixel_load_24b;

void Init_preview_24b(short width,short height,long size,int format);
void Pixel_load_in_24b_preview(short x_pos,short y_pos,byte r,byte g,byte b);
*/
//

/*
void Init_preview(short width,short height,long size,int format,enum PIXEL_RATIO ratio);
*/

#endif
