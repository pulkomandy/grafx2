/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
///@file struct.h
/// Structures that can be used in the whole program.
//////////////////////////////////////////////////////////////////////////////
#ifndef _STRUCT_H_
#define _STRUCT_H_

#if defined(__BEOS__)
    #include <inttypes.h>
#else
    #include <stdint.h>
#endif

#include "const.h"

// Definition of the base data types
///  8bit unsigned integer
#define byte  uint8_t  
/// 16bit unsigned integer
#define word  uint16_t 
/// 32bit unsigned integer
#define dword uint32_t
/// 64bit unsigned integer
#define qword uint64_t

// Named function prototypes
typedef void (* Func_action)    (void);
typedef void (* Func_pixel) (word,word,byte);
typedef byte (* Func_read)   (word,word);
typedef void (* Func_clear)  (byte);
typedef void (* Func_display)   (word,word,word);
typedef byte (* Func_effect)     (word,word,byte);
typedef void (* Func_block)     (word,word,word,word,byte);
typedef void (* Func_line_XOR) (word,word,word);
typedef void (* Func_display_brush_color) (word,word,word,word,word,word,byte,word);
typedef void (* Func_display_brush_mono)  (word,word,word,word,word,word,byte,byte,word);
typedef void (* Func_gradient)   (long,short,short);
typedef void (* Func_remap)     (word,word,word,word,byte *);
typedef void (* Func_procsline) (word,word,word,byte *);
typedef void (* Func_display_zoom) (word,word,word,byte *);
typedef void (* Func_display_brush_color_zoom) (word,word,word,word,word,word,byte,word,byte *);
typedef void (* Func_display_brush_mono_zoom)  (word,word,word,word,word,word,byte,byte,word,byte *);
typedef void (* Func_draw_brush) (byte *,word,word,word,word,word,word,byte,word);

/// A set of RGB values.
typedef struct
{
  byte R; ///< Red
  byte G; ///< Green
  byte B; ///< Blue
}__attribute__ ((__packed__)) T_Components, T_Palette[256]; ///< A complete 256-entry RGB palette (768 bytes).

/// A normal rectangular button in windows and menus.
typedef struct T_Normal_button
{
  short Number;                 ///< Unique identifier for all controls
  word Pos_X;                   ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                   ///< Coordinate for left of button, relative to the window, before scaling.
  word Width;                   ///< Width before scaling
  word Height;                  ///< Height before scaling
  byte Clickable;               ///< Boolean, unused.
  byte Repeatable;              ///< Boolean, true if the button activates repeatedly until you release the mouse button. Used for "+" buttons, for example.
  word Shortcut;                ///< Keyboard shortcut that will emulate a click on this button.
  struct T_Normal_button * Next;///< Pointer to the next normal button of current window.
} T_Normal_button;

/// A window control that shows a complete 256-color palette
typedef struct T_Palette_button
{
  short Number;                 ///< Unique identifier for all controls
  word Pos_X;                   ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                   ///< Coordinate for left of button, relative to the window, before scaling.
  struct T_Palette_button * Next;///< Pointer to the next palette of current window.
} T_Palette_button;

/// A window control that represents a vertical scrollbar, with a slider, and two arrow buttons.
typedef struct T_Scroller_button
{
  short Number;                   ///< Unique identifier for all controls
  word Pos_X;                     ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                     ///< Coordinate for left of button, relative to the window, before scaling.
  word Height;                    ///< Height before scaling.
  word Nb_elements;               ///< Number of distinct values it can take.
  word Nb_visibles;               ///< If this slider is meant to show several elements of a collection, this is their number (otherwise, it's 1).
  word Position;                  ///< Current position of the slider: which item it's pointing.
  word Cursor_height;             ///< Vertical dimension of the slider, in pixels before scaling.
  struct T_Scroller_button * Next;///< Pointer to the next scroller of current window.
} T_Scroller_button;

///
/// A window control that only has a rectangular "active" area which catches mouse clicks,
// but no visible shape. It's used for custom controls where the drawing is done on
// a case by case basis.
typedef struct T_Special_button
{
  short Number;                  ///< Unique identifier for all controls
  word Pos_X;                    ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                    ///< Coordinate for left of button, relative to the window, before scaling.
  word Width;                    ///< Width before scaling
  word Height;                   ///< Height before scaling
  struct T_Special_button * Next;///< Pointer to the next special button of current window.
} T_Special_button;

/// Data for a dropdown item, ie. one proposed choice.
typedef struct T_Dropdown_choice
{
  short Number;                   ///< Value that identifies the choice (for this dropdown only)
  const char * Label;             ///< String to display in the dropdown panel
  struct T_Dropdown_choice * Next;///< Pointer to the next choice for this dropdown.
} T_Dropdown_choice;

/// A window control that behaves like a dropdown button.
typedef struct T_Dropdown_button
{
  short Number;                   ///< Unique identifier for all controls
  word Pos_X;                     ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                     ///< Coordinate for left of button, relative to the window, before scaling.
  word Width;                     ///< Width before scaling
  word Height;                    ///< Height before scaling
  byte Display_choice;            ///< Boolean, true if the engine should print the selected item's label in the dropdown area when the user chooses it.
  byte Display_centered;          ///< Boolean, true to center the labels (otherwise, align left)
  byte Display_arrow;             ///< Boolean, true to display a "down" arrow box in top right
  byte Active_button;             ///< Determines which mouse button(s) cause the dropdown panel to open: LEFT_SIDE || RIGHT_SIDE || (LEFT_SIDE|RIGHT_SIDE)
  word Dropdown_width;            ///< Width of the dropdown panel when it's open. Use 0 for "same as the dropdown button"
  T_Dropdown_choice * First_item; ///< Linked list with the choices available for this dropdown.
  struct T_Dropdown_button * Next;///< Pointer to the next dropdown button of current window.
} T_Dropdown_button;

/// Data for one item (file, directory) in a fileselector.
typedef struct T_Fileselector_item
{
  char Short_name[19]; ///< Name to display.
  char Full_name[256]; ///< Filesystem value.
  byte Type;           ///< Type of item: 0 = File, 1 = Directory, 2 = Drive

  struct T_Fileselector_item * Next;    ///< Pointer to next item of the current fileselector.
  struct T_Fileselector_item * Previous;///< Pointer to previous item of the current fileselector.
} T_Fileselector_item;

/// Data for one line of the "Help" screens.
typedef struct {
  char Line_type;     ///< Kind of line: 'N' for normal line, 'S' for a bold line, 'K' for a line with keyboard shortcut, 'T' and '-' for upper and lower titles.
  char * Text;        ///< Displayed string.
  int Line_parameter; ///< Generic parameter depending on line type. For 'K' lines: a shortcut identifier. For others: unused.
} T_Help_table;

/// Data for one section of the "Help" screens, ie a page.
typedef struct
{
  const T_Help_table* Help_table; ///< Pointer to the array of ::T_Help_table that contains the lines
  word Length;                    ///< Size of the array of lines
} T_Help_section;

/// Data for one setting of gradients. Warning, this one is saved/loaded as binary.
typedef struct
{
  byte Start;     ///< First color
  byte End;       ///< Last color
  dword Inverse;  ///< Boolean, true if the gradient goes in descending order
  dword Mix;      ///< Amount of randomness to add to the mix (0-255)
  dword Technique;///< Gradient technique: 0 (no pattern) 1 (dithering), or 2 (big dithering)
} __attribute__((__packed__)) T_Gradient_array;

/// Data for one setting of shade. Warning, this one is saved/loaded as binary.
typedef struct
{
  word List[512]; ///< List of entries, each one is either a color (0-255) or -1 for empty.
  byte Step;      ///< Step to increment/decrement on left-clicks.
  byte Mode;      ///< Shade mode: Normal, Loop, or No-saturation see ::SHADE_MODES
} T_Shade;

/// Data for one fullscreen video mode in configuration file. Warning, this one is saved/loaded as binary.
typedef struct
{
  byte State; ///< How good is the mode supported. 0:Good (white) 1:OK (light) 2:So-so (dark) 4:User-disabled (black); +128 => System doesn't support it at all.
  word Width; ///< Videomode width in pixels.
  word Height;///< Videomode height in pixels. 
} __attribute__((__packed__)) T_Config_video_mode;

/// Header for gfx2.cfg. Warning, this one is saved/loaded as binary.
typedef struct
{
  char Signature[3]; ///< Signature for the file format. "CFG".
  byte Version1;     ///< Major version number (ex: 2)
  byte Version2;     ///< Minor version number (ex: 0)
  byte Beta1;        ///< Major beta version number (ex: 96)
  byte Beta2;        ///< Major beta version number (ex: 5)
} __attribute__((__packed__)) T_Config_header;

/// Header for a config chunk in for gfx2.cfg. Warning, this one is saved/loaded as binary.
typedef struct
{
  byte Number; ///< Section identfier. Possible values are in enum ::CHUNKS_CFG
  word Size;   ///< Size of the configuration block that follows, in bytes.
} __attribute__((__packed__)) T_Config_chunk;

/// Configuration for one keyboard shortcut in gfx2.cfg. Warning, this one is saved/loaded as binary.
typedef struct
{
  word Number; ///< Indicates the shortcut action. This is a number starting from 0, which matches ::T_Key_config.Number
  word Key;    ///< Keyboard shortcut: SDLK_something, or -1 for none
  word Key2;   ///< Alternate keyboard shortcut: SDLK_something, or -1 for none
} __attribute__((__packed__)) T_Config_shortcut_info;

/// This structure holds all the settings which are saved and loaded as gfx2.ini.
typedef struct
{
  byte Font;                             ///< Boolean, true to use the "fun" font in menus, false to use the classic one.
  int  Show_hidden_files;                ///< Boolean, true to show hidden files in fileselectors.
  int  Show_hidden_directories;          ///< Boolean, true to show hidden directories in fileselectors.
//  int  Show_system_directories;        ///< (removed when converted from DOS)
  byte Display_image_limits;             ///< Boolean, true to display a dotted line at the borders of the image if it's smaller than screen.
  byte Cursor;                           ///< Mouse cursor aspect: 1 Solid, 2 Transparent, 3 Thin
  byte Maximize_preview;                 ///< Boolean, true to make previews in fileselector fit the whole rectangle.
  byte Auto_set_res;                     ///< Boolean, true to make grafx2 switch to a new resolution whenever you load an image.
  byte Coords_rel;                       ///< Boolean, true to display coordinates as relative (instead of absolute)
  byte Backup;                           ///< Boolean, true to backup the original file whenever you save an image.
  byte Adjust_brush_pick;                ///< Boolean, true to omit the right and bottom edges when grabbing a brush in Grid mode.
  byte Auto_save;                        ///< Boolean, true to save configuration when exiting program.
  byte Max_undo_pages;                   ///< Number of steps to memorize for Undo/Redo.
  byte Mouse_sensitivity_index_x;        ///< Mouse sensitivity in X axis
  byte Mouse_sensitivity_index_y;        ///< Mouse sensitivity in Y axis
  byte Mouse_fix_factor_X;               ///< Mouse correction factor in X axis.
  byte Mouse_fix_factor_Y;               ///< Mouse correction factor in Y axis.
  byte Mouse_merge_movement;             ///< Number of SDL mouse events that are merged into a single change of mouse coordinates.
  byte Delay_left_click_on_slider;       ///< Delay (in 1/100s) between two activations of a repeatable button when you hold left-click.
  byte Delay_right_click_on_slider;      ///< Delay (in 1/100s) between two activations of a repeatable button when you hold left-click.
  long Timer_delay;                      ///< Delay (in 1/55s) before showing a preview in a fileselector.
  T_Components Fav_menu_colors[4];       ///< Favorite colors to use for the menu.
  int  Nb_max_vertices_per_polygon;      ///< Limit for the number of vertices in polygon tools.
  byte Clear_palette;                    ///< Boolean, true to reset the palette (to black) before loading an image.
  byte Set_resolution_according_to;      ///< When Auto_set_res is on, this determines if the mode should be chosen according to the "original screen" information in the file (1) or the picture dimensons (2)
  byte Ratio;                            ///< Determines the scaling of menu and windows: 0 no scaling, 1 scaling, 2 slight scaling.
  byte Fast_zoom;                        ///< Boolean, true if the magnifier shortcut should automatically view the mouse area.
  byte Find_file_fast;                   ///< In fileselectors, this determines which entries should be sought when typing letters: 0 all, 1 files only, 2 directories only.
  byte Couleurs_separees;                ///< Boolean, true if the menu palette should separate color cells with a black outline.
  word Palette_cells_X;                  ///< Number of colors to show in a row of the menu palette.
  word Palette_cells_Y;                  ///< Number of colors to show in a column of the menu palette.
  byte Palette_vertical;                 ///< Boolean, true if the menu palette should go top to bottom instead of left to right
  byte FX_Feedback;                      ///< Boolean, true if drawing effects should read the image being modified (instead of the image before clicking)
  byte Safety_colors;                    ///< Boolean, true to make the palette automatically re-create menu colors if needed after a "Zap" or color reduction.
  byte Opening_message;                  ///< Boolean, true to display the splash screen on strtup.
  byte Clear_with_stencil;               ///< Boolean, true to take the stencil into effect (if active) when using the Clear function.
  byte Auto_discontinuous;               ///< Boolean, true to automatically switch to the discontinuous freehand draw after grabbing a brush.
  byte Screen_size_in_GIF;               ///< Boolean, true to store current resolution in GIF files.
  byte Auto_nb_used;                     ///< Boolean, true to count colors in Palette screen.
  byte Default_resolution;               ///< Default video mode to use on startup. Index in ::Video_mode.
  char *Bookmark_directory[NB_BOOKMARKS];///< Bookmarked directories in fileselectors: This is the full dierctory name.
  char Bookmark_label[NB_BOOKMARKS][8+1];///< Bookmarked directories in fileselectors: This is the displayed name.
} T_Config;

// Structures utilisées pour les descriptions de pages et de liste de pages.
// Lorsqu'on gèrera les animations, il faudra aussi des listes de listes de
// pages.

// Ces structures sont manipulées à travers des fonctions de gestion du
// backup dans "graph.c".

/// This is the data for one step of Undo/Redo, for one image.
typedef struct
{
  byte *    Image;   ///< Pixel data for the image.
  int       Width;   ///< Image width in pixels.
  int       Height;  ///< Image height in pixels.
  T_Palette Palette; ///< Image palette.

  char      Comment[COMMENT_SIZE+1]; ///< Comment to store in the image file.

  char      File_directory[MAX_PATH_CHARACTERS];///< Directory that contains the file.
  char      Filename[MAX_PATH_CHARACTERS];      ///< Filename without directory.
  byte      File_format;                        ///< File format, in enum ::FILE_FORMATS

} T_Page;

/// Collection of undo/redo steps.
typedef struct
{
  int      List_size;         /// Number of ::T_Page in the vector "Pages".
  int      Nb_pages_allocated;/// Number of ::T_Page used so far in the vector "Pages".
  T_Page * Pages;             /// Vector of Pages, each one being a undo/redo step.
} T_List_of_pages;



#endif
