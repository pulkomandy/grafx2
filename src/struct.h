/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

//////////////////////////////////////////////////////////////////////////////
///@file struct.h
/// Structures that can be used in the whole program.
//////////////////////////////////////////////////////////////////////////////
#ifndef _STRUCT_H_
#define _STRUCT_H_

#if defined(__BEOS__) || defined(__TRU64__)
    #include <inttypes.h>
#else
    #include <stdint.h>
#endif

#include "const.h"


// POSIX calls it strcasecmp, Windows uses stricmp... no ANSI standard.
#ifdef WIN32
    #define strcasecmp stricmp
#endif

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
// GrafX2 use a lot of function pointer to do the drawing depending in the "fake hardware zoom" and the magnifier status.
typedef void (* Func_action)    (void); ///< An action. Used when you click a menu button or trigger a keyboard shortcut.
typedef void (* Func_pixel) (word,word,byte); ///< Set pixel at position (x,y) to color c. Used in load screen to write the data to brush, picture, or preview area.
typedef byte (* Func_read)   (word,word); ///< Read a pixel at position (x,y) on something. Used for example in save to tell if the data is a brush or a picture
typedef void (* Func_clear)  (byte);
typedef void (* Func_display)   (word,word,word);
typedef byte (* Func_effect)     (word,word,byte); ///< Called by all drawing tools to draw with a special effect (smooth, transparency, shade, ...)
typedef void (* Func_block)     (word,word,word,word,byte);
typedef void (* Func_line_XOR) (word,word,word); ///< Draw an XOR line on the picture view of the screen. Use a different function when in magnify mode.
typedef void (* Func_display_brush_color) (word,word,word,word,word,word,byte,word);
typedef void (* Func_display_brush_mono)  (word,word,word,word,word,word,byte,byte,word);
typedef void (* Func_gradient)   (long,short,short);
typedef void (* Func_remap)     (word,word,word,word,byte *);
typedef void (* Func_procsline) (word,word,word,byte *);
typedef void (* Func_display_zoom) (word,word,word,byte *);
typedef void (* Func_display_brush_color_zoom) (word,word,word,word,word,word,byte,word,byte *);
typedef void (* Func_display_brush_mono_zoom)  (word,word,word,word,word,word,byte,byte,word,byte *);
typedef void (* Func_draw_brush) (byte *,word,word,word,word,word,word,byte,word);
typedef void (* Func_draw_list_item) (word,word,word,byte); ///< Draw an item inside a list button. This is done with a callback so it is possible to draw anything, as the list itself doesn't handle the content

/// A set of RGB values.
#ifdef __GNUC__
typedef struct
{
  byte R; ///< Red
  byte G; ///< Green
  byte B; ///< Blue
} __attribute__((__packed__)) T_Components, T_Palette[256] ; ///< A complete 256-entry RGB palette (768 bytes).
#else
#pragma pack(1)
typedef struct
{
  byte R; ///< Red
  byte G; ///< Green
  byte B; ///< Blue
} T_Components, T_Palette[256] ; ///< A complete 256-entry RGB palette (768 bytes).
#pragma pack()
#endif

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

/// A window control that represents a scrollbar, with a slider, and two arrow buttons.
typedef struct T_Scroller_button
{
  short Number;                   ///< Unique identifier for all controls
  byte Is_horizontal;             ///< Boolean: True if slider is horizontal instead of vertical.
  word Pos_X;                     ///< Coordinate for top of button, relative to the window, before scaling.
  word Pos_Y;                     ///< Coordinate for left of button, relative to the window, before scaling.
  word Length;                    ///< Length before scaling.
  word Nb_elements;               ///< Number of distinct values it can take.
  word Nb_visibles;               ///< If this slider is meant to show several elements of a collection, this is their number (otherwise, it's 1).
  word Position;                  ///< Current position of the slider: which item it's pointing.
  word Cursor_length;             ///< Dimension of the slider, in pixels before scaling.
  struct T_Scroller_button * Next;///< Pointer to the next scroller of current window.
} T_Scroller_button;

/// Special invisible button
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
  byte Bottom_up;                 ///< Boolean, true to make the dropdown panel go above its button instead of below it
  byte Active_button;             ///< Determines which mouse button(s) cause the dropdown panel to open: LEFT_SIDE || RIGHT_SIDE || (LEFT_SIDE|RIGHT_SIDE)
  word Dropdown_width;            ///< Width of the dropdown panel when it's open. Use 0 for "same as the dropdown button"
  T_Dropdown_choice * First_item; ///< Linked list with the choices available for this dropdown.
  struct T_Dropdown_button * Next;///< Pointer to the next dropdown button of current window.
} T_Dropdown_button;

/// Data for one item (file, directory) in a fileselector.
typedef struct T_Fileselector_item
{
  char Full_name[256]; ///< Filesystem value.
  byte Type;           ///< Type of item: 0 = File, 1 = Directory, 2 = Drive
  byte Icon;           ///< One of ::ICON_TYPES, ICON_NONE for none.

  struct T_Fileselector_item * Next;    ///< Pointer to next item of the current fileselector.
  struct T_Fileselector_item * Previous;///< Pointer to previous item of the current fileselector.
  
  word  Length_short_name; ///< Number of bytes allocated for :Short_name
  #if __GNUC__ < 3
  char Short_name[0]; ///< Name to display.
#else
  char Short_name[]; ///< Name to display.
#endif
  // No field after Short_name[] ! Dynamic allocation according to name length.
} T_Fileselector_item;

/// Data for a fileselector
typedef struct T_Fileselector
{
  /// Number of elements in the current fileselector's ::Filelist
  short Nb_elements;
  /// Number of files in the current fileselector's ::Filelist
  short Nb_files;
  /// Number of directories in the current fileselector's ::Filelist
  short Nb_directories;
  /// Head of the linked list for the fileselector.
  T_Fileselector_item * First;
  /// Index for direct access to element number N
  T_Fileselector_item ** Index;
} T_Fileselector;

/// "List" button as used in the font selection, skin selection, and brush factory screens. It's like a limited filelist.
/// The screenmode selection and load/save screen were written before this existed so they use their own code. It would be nice if they were updated to use this one.
typedef struct T_List_button
{
  short Number;                     ///< Unique identifier for all controls
  short List_start;                 ///< Index of the font to appear as first line
  short Cursor_position;            ///< Index of the selected line (0=top)

  T_Special_button  * Entry_button; ///< Pointer to the associated selection control.
  T_Scroller_button * Scroller;     ///< Pointer to the associated scroller
  
  Func_draw_list_item   Draw_list_item; ///< Function to call for each item to draw its line
  byte                  Color_index;    ///< Background color: From 0->MC_Black to 3->MC_White

  struct T_List_button * Next;    ///< Pointer to the next list button of current window.
} T_List_button;

/// A stackable window (editor screen)
typedef struct
{
  word Pos_X;
  word Pos_Y;
  word Width;
  word Height;
  word Nb_buttons;
  T_Normal_button   *Normal_button_list;
  T_Palette_button  *Palette_button_list;
  T_Scroller_button *Scroller_button_list;
  T_Special_button  *Special_button_list;
  T_Dropdown_button *Dropdown_button_list;
  T_List_button     *List_button_list;
  int Attribute1;
  int Attribute2;
  byte Draggable;
} T_Window;

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
  byte  Speed;    ///< Speed of cycling. 0 for disabled, 1-64 otherwise.
} T_Gradient_range;

/// Data for a full set of gradients.
typedef struct
{
  int Used; ///< Reference count
  T_Gradient_range Range[16];
}  T_Gradient_array;

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
} T_Config_video_mode;

/// Header for gfx2.cfg
typedef struct
{
  char Signature[3]; ///< Signature for the file format. "CFG".
  byte Version1;     ///< Major version number (ex: 2)
  byte Version2;     ///< Minor version number (ex: 0)
  byte Beta1;        ///< Major beta version number (ex: 96)
  byte Beta2;        ///< Major beta version number (ex: 5)
} T_Config_header;

/// Header for a config chunk in for gfx2.cfg
typedef struct
{
  byte Number; ///< Section identfier. Possible values are in enum ::CHUNKS_CFG
  word Size;   ///< Size of the configuration block that follows, in bytes.
} T_Config_chunk;


/// Configuration for one keyboard shortcut in gfx2.cfg
typedef struct
{
  word Number; ///< Indicates the shortcut action. This is a number starting from 0, which matches ::T_Key_config.Number
  word Key;    ///< Keyboard shortcut: SDLK_something, or -1 for none
  word Key2;   ///< Alternate keyboard shortcut: SDLK_something, or -1 for none
} T_Config_shortcut_info;


/// This structure holds all the settings saved and loaded as gfx2.ini.
typedef struct
{
  char *Font_file;                       ///< Name of the font used in the menus. Matches file skins/font_*.png (Case-sensitive on some filesystems)
  char *Skin_file;                       ///< String, name of the file where all the graphic data is stored
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
  byte Mouse_merge_movement;             ///< Number of SDL mouse events that are merged into a single change of mouse coordinates.
  byte Delay_left_click_on_slider;       ///< Delay (in 1/100s) between two activations of a repeatable button when you hold left-click.
  byte Delay_right_click_on_slider;      ///< Delay (in 1/100s) between two activations of a repeatable button when you hold left-click.
  long Timer_delay;                      ///< Delay (in 1/55s) before showing a preview in a fileselector.
  T_Components Fav_menu_colors[4];       ///< Favorite colors to use for the menu.
  int  Nb_max_vertices_per_polygon;      ///< Limit for the number of vertices in polygon tools.
  byte Clear_palette;                    ///< Boolean, true to reset the palette (to black) before loading an image.
  byte Set_resolution_according_to;      ///< When Auto_set_res is on, this determines if the mode should be chosen according to the "original screen" information in the file (1) or the picture dimensons (2)
  int8_t Ratio;                          ///< Determines the scaling of menu and windows: 0 no scaling, 1 scaling, 2 slight scaling, negative= opposite of max scaling
  byte Fast_zoom;                        ///< Boolean, true if the magnifier shortcut should automatically view the mouse area.
  byte Find_file_fast;                   ///< In fileselectors, this determines which entries should be sought when typing letters: 0 all, 1 files only, 2 directories only.
  byte Separate_colors;                  ///< Boolean, true if the menu palette should separate color cells with a black outline.
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
  char *Bookmark_directory[NB_BOOKMARKS];///< Bookmarked directories in fileselectors: This is the full directory name.
  char Bookmark_label[NB_BOOKMARKS][8+1];///< Bookmarked directories in fileselectors: This is the displayed name.
  int  Window_pos_x;                     ///< Last window x position (9999 if unsupportd/irrelevant for the platform)
  int  Window_pos_y;                     ///< Last window y position (9999 if unsupportd/irrelevant for the platform)
  word Double_click_speed;               ///< Maximum delay for double-click, in ms.
  word Double_key_speed;                 ///< Maximum delay for double-keypress, in ms.
  byte Grid_XOR_color;                   ///< XOR value to apply for grid color.
  byte Right_click_colorpick;            ///< Boolean, true to enable a "tablet" mode, where RMB acts as instant colorpicker
  byte Sync_views;                       ///< Boolean, true when the Main and Spare should share their viewport settings.
  byte Stylus_mode;                      ///< Boolean, true to tweak some tools (eg:Curve) for single-button stylus.
  word Swap_buttons;                     ///< Sets which key swaps mouse buttons : 0=none, or MOD_CTRL, or MOD_ALT.
  char Scripts_directory[MAX_PATH_CHARACTERS];///< Full pathname of directory for Lua scripts
  byte Allow_multi_shortcuts;            ///< Boolean, true if the same key combination can trigger multiple shortcuts.
} T_Config;

// Structures utilisées pour les descriptions de pages et de liste de pages.
// Lorsqu'on gèrera les animations, il faudra aussi des listes de listes de
// pages.

// Ces structures sont manipulées à travers des fonctions de gestion du
// backup dans "graph.c".

typedef struct T_Image
{
  byte * Pixels;
  int Duration;
} T_Image;

/// This is the data for one step of Undo/Redo, for one image.
/// This structure is resized dynamically to hold pointers to all of the layers in the picture.
/// The pointed layers are just byte* holding the raw pixel data. But at Image[0]-1 you will find a short that is used as a reference counter for each layer.
/// This way we can use the same pixel data in many undo pages when the user edit only one of the layers (which is what they usually do).
typedef struct T_Page
{
  int       Width;   ///< Image width in pixels.
  int       Height;  ///< Image height in pixels.
  T_Palette Palette; ///< Image palette.

  char      Comment[COMMENT_SIZE+1]; ///< Comment to store in the image file.

  char      File_directory[MAX_PATH_CHARACTERS];///< Directory that contains the file.
  char      Filename[MAX_PATH_CHARACTERS];      ///< Filename without directory.
  byte      File_format;                        ///< File format, in enum ::FILE_FORMATS
  struct T_Page *Next; ///< Pointer to the next backup
  struct T_Page *Prev; ///< Pointer to the previous backup
  T_Gradient_array *Gradients; ///< Pointer to the gradients used by the image.
  byte      Background_transparent; ///< Boolean, true if Layer 0 should have transparent pixels
  byte      Transparent_color; ///< Index of transparent color. 0 to 255.
  int       Nb_layers; ///< Number of layers
#if __GNUC__ < 3
  // gcc2 doesn't suport [], but supports [0] which does the same thing.
  T_Image    Image[0];  ///< Pixel data for the (first layer of) image.
#else
  T_Image    Image[];  ///< Pixel data for the (first layer of) image.
#endif
  // No field after Image[] ! Dynamic layer allocation for Image[1], [2] etc.
} T_Page;

/// Collection of undo/redo steps.
typedef struct
{
  int      List_size;         ///< Number of ::T_Page in the vector "Pages".
  T_Page * Pages;             ///< Head of a linked list of pages, each one being a undo/redo step.
} T_List_of_pages;

/// A single image bitmap
/// This struct is used to store a flattened view of the current picture.
typedef struct
{
  int       Width;   ///< Image width in pixels.
  int       Height;  ///< Image height in pixels.
  byte *    Image;   ///< Pixel data for the image.
} T_Bitmap;

/// A single memorized brush from the Brush Container
typedef struct
{
  byte Paintbrush_shape; ///< Kind of brush
  byte Thumbnail[BRUSH_CONTAINER_PREVIEW_WIDTH][BRUSH_CONTAINER_PREVIEW_HEIGHT]; 
  // Data for color brush
  word Width;
  word Height;
  byte * Brush; /// < Color brush (if any)
  T_Palette Palette;
  byte Colormap[256];
  byte Transp_color;
} T_Brush_template;

/// GUI skin data
typedef struct
{
  // Mouse
  
  /// X coordinate of the mouse cursor's "hot spot". It is < ::CURSOR_SPRITE_WIDTH
  word Cursor_offset_X[NB_CURSOR_SPRITES];
  /// Y coordinate of the mouse cursor's "hot spot". It is < ::CURSOR_SPRITE_HEIGHT
  word Cursor_offset_Y[NB_CURSOR_SPRITES];
  /// Graphic resources for the mouse cursor.
  byte Cursor_sprite[NB_CURSOR_SPRITES][CURSOR_SPRITE_HEIGHT][CURSOR_SPRITE_WIDTH];

  // Sieve patterns
  
  /// Preset sieve patterns, stored as binary (one word per line)
  word  Sieve_pattern[12][16];
  
  // Menu and other graphics
  
  /// Bitmap data for the menu, a single rectangle.
  byte Menu_block[3][35][MENU_WIDTH];
  byte Layerbar_block[3][10][144];
  byte Animbar_block[3][14][236];
  byte Statusbar_block[3][9][20];
  /// Bitmap data for the icons that are displayed over the menu.
  byte Menu_sprite[2][NB_MENU_SPRITES][MENU_SPRITE_HEIGHT][MENU_SPRITE_WIDTH];
  /// Bitmap data for the different "effects" icons.
  byte Effect_sprite[NB_EFFECTS_SPRITES][EFFECT_SPRITE_HEIGHT][EFFECT_SPRITE_WIDTH];
  /// Bitmap data for the different Layer icons.
  byte Layer_sprite[3][16][LAYER_SPRITE_HEIGHT][LAYER_SPRITE_WIDTH];
  /// Bitmap data for the Grafx2 logo that appears on splash screen. All 256 colors allowed.
  byte Logo_grafx2[231*56];
  /// Bitmap data for the 6x8 font used in help screens.
  byte Help_font_norm [256][6][8];
  /// Bitmap data for the 6x8 font used in help screens ("bold" verstion).
  byte Bold_font [256][6][8];
  // 12
  // 34
  /// Bitmap data for the title font used in help screens. Top-left quarter.
  byte Help_font_t1 [64][6][8];
  /// Bitmap data for the title font used in help screens. Top-right quarter.
  byte Help_font_t2 [64][6][8];
  /// Bitmap data for the title font used in help screens. Bottom-left quarter.
  byte Help_font_t3 [64][6][8];
  /// Bitmap data for the title font used in help screens. Bottom-right quarter.
  byte Help_font_t4 [64][6][8];
  /// Bitmap data for the small 8x8 icons.
  byte Icon_sprite[NB_ICON_SPRITES][ICON_SPRITE_HEIGHT][ICON_SPRITE_WIDTH];

  /// A default 256-color palette.
  T_Palette Default_palette;

  /// Preview for displaying in the skin dialog
  byte Preview[16][173];

  /// GUI color indices in skin palette: black, dark, light, white.
  byte Color[4];
  /// Transparent GUI color index in skin file
  byte Color_trans;

} T_Gui_skin;

typedef struct {
  // Preset paintbrushes
  
  /// Graphic resources for the preset paintbrushes.
  byte  Sprite[PAINTBRUSH_HEIGHT][PAINTBRUSH_WIDTH];
  /// Width of the preset paintbrushes.
  word  Width;
  /// Height of the preset paintbrushes.
  word  Height;
  /// Type of the preset paintbrush: index in enum PAINTBRUSH_SHAPES
  byte  Shape;
  /// Brush handle for the preset brushes. Generally ::Width[]/2
  word  Offset_X;
  /// Brush handle for the preset brushes. Generally ::Height[]/2
  word  Offset_Y;

} T_Paintbrush;

// A menubar.
typedef struct {
  word Width;
  word Height;
  byte Visible;
  word Top; ///< Relative to the top line of the menu, hidden bars don't count.
  byte* Skin[3]; ///< [0] has normal buttons, [1] has selected buttons, [2] is current.
  word Skin_width;
  byte Last_button_index;
} T_Menu_Bar;

typedef enum {
  MENUBAR_STATUS = 0, // MUST be 0
  MENUBAR_LAYERS,
  MENUBAR_TOOLS,
  MENUBAR_COUNT
} T_Menubars;

typedef struct
{
  int Previous;
  int Next;
} T_Tile;

#endif
