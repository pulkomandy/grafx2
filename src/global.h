/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2009 Franck Charlet
    Copyright 2009 Yves Rizoud
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
///@file global.h
/// This file contains all global variables.
/// They are prefixed by ::GFX2_GLOBAL so they are extern when needed.
//////////////////////////////////////////////////////////////////////////////
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <SDL.h>
#include "struct.h"

// MAIN declares the variables,
// other files only have an extern definition.
#ifdef GLOBAL_VARIABLES
  /// Magic prefix to make all declarations extern, except when included by main.c.
  #define GFX2_GLOBAL
#else
  #define GFX2_GLOBAL extern
#endif

// -- CONFIGURATION variables

/// Current configuration.
GFX2_GLOBAL T_Config Config;

/// Array of special keys.
GFX2_GLOBAL word Config_Key[NB_SPECIAL_SHORTCUTS][2];

/// A video mode (resolution) usable by Grafx2.
typedef struct
{
  short  Width;      ///< Screen width
  short  Height;     ///< Screen height
  byte   Mode;       ///< Unused (used to be Mode-X, SVGA, etc)
  word   Fullscreen; ///< 0 for window, 1 for fullscreen
  byte   State;      ///< How good is the mode supported. 0:Good (white) 1:OK (light) 2:So-so (dark) 4:User-disabled (black); +128 => System doesn't support it at all.
} T_Video_mode;

/// Array of all video modes supported by your platform. Actually filled up to ::Nb_video_modes, excluded.
GFX2_GLOBAL T_Video_mode Video_mode[MAX_VIDEO_MODES];

/// Actual number of video modes in ::Video_mode.
GFX2_GLOBAL int  Nb_video_modes;

// -- Menu colors

GFX2_GLOBAL byte MC_Black; ///< Index of color to use as "black" in the GUI menus.
GFX2_GLOBAL byte MC_Dark;  ///< Index of color to use as "dark grey" in the GUI menus.
GFX2_GLOBAL byte MC_Light; ///< Index of color to use as "light grey" in the GUI menus.
GFX2_GLOBAL byte MC_White; ///< Index of color to use as "white" in the GUI menus.
GFX2_GLOBAL byte MC_Trans; ///< Index of color to use as "transparent" while loading the GUI file.

// Input state
GFX2_GLOBAL word Mouse_X; ///< Current mouse cursor position.
GFX2_GLOBAL word Mouse_Y; ///< Current mouse cursor position.
GFX2_GLOBAL byte Mouse_K; ///< Current mouse buttons state. Bitfield: 1 for RMB, 2 for LMB.
GFX2_GLOBAL byte Keyboard_click_allowed; ///< Set to 0 when you edit a textfield so you can use space without exiting it

/// Helper macro to take only one button when both are pressed (LMB has priority)
#define Mouse_K_unique (Mouse_K==0?0:(Mouse_K&1?1:(Mouse_K&2?2:0)))

/// Last key pressed, 0 if none. Set by the latest call to ::Get_input()
GFX2_GLOBAL dword Key;

///
/// Last character typed, converted to ANSI character set (Windows-1252).
/// This is mostly used when the user enters text (filename, etc).
GFX2_GLOBAL dword Key_ANSI;

// Keyboard modifiers
// (Name conflict with windows.h)
#ifdef MOD_SHIFT
  #undef MOD_SHIFT
#endif
#ifdef MOD_CTRL
  #undef MOD_CTRL
#endif
#ifdef MOD_ALT
  #undef MOD_ALT
#endif

/// Key modifier for SHIFT key. Used as mask in ::Key, for example.
#define MOD_SHIFT 0x1000
/// Key modifier for CONTROL key. Used as mask in ::Key, for example.
#define MOD_CTRL  0x2000
/// Key modifier for ALT key. Used as mask in ::Key, for example.
#define MOD_ALT   0x4000
/// Key modifier for META key. Used as mask in ::Key, for example.
#define MOD_META  0x8000

/// Boolean set to true when the OS/window manager requests the application to close. ie: [X] button
GFX2_GLOBAL byte Quit_is_required;

///
/// This boolean is true when the current operation allows changing the
/// foreground or background color.
GFX2_GLOBAL byte Allow_color_change_during_operation;

// -- Mouse cursor data

/// Current mouse cursor. Value is in enum ::CURSOR_SHAPES
GFX2_GLOBAL byte Cursor_shape;
/// Backup of ::Cursor_shape, used while a window is open (and needs a different cursor)
GFX2_GLOBAL byte Cursor_shape_before_window;
/// Boolean, means the cursor should not be drawn. It's togglable by the user.
GFX2_GLOBAL byte Cursor_hidden;
/// Boolean, means the cursor is currently hovering over a menu GUI element.
GFX2_GLOBAL byte Cursor_in_menu;
/// Boolean, means the cursor was hovering over a menu GUI element.
GFX2_GLOBAL byte Cursor_in_menu_previous;
/// Storage for the graphics under the mouse cursor. Used by ::Hide_cursor and ::Display_cursor
GFX2_GLOBAL byte Cursor_background[CURSOR_SPRITE_HEIGHT][CURSOR_SPRITE_WIDTH];

// -- Paintbrush data

/// Active paintbrush. It's an index in enum ::PAINTBRUSH_SHAPES
GFX2_GLOBAL byte  Paintbrush_shape;
/// Backup of ::Paintbrush_shape, before fill operation
GFX2_GLOBAL byte  Paintbrush_shape_before_fill;
/// Backup of ::Paintbrush_shape, before color picker operation
GFX2_GLOBAL byte  Paintbrush_shape_before_colorpicker;
/// Backup of ::Paintbrush_shaper, before lasso operation
GFX2_GLOBAL byte  Paintbrush_shape_before_lasso;
/// Boolean, true when the preview paintbrush shouldn't be drawn.
GFX2_GLOBAL byte  Paintbrush_hidden;
/// Cordinate of the preview paintbrush in image space.
GFX2_GLOBAL short Paintbrush_X;
/// Cordinate of the preview paintbrush in image space.
GFX2_GLOBAL short Paintbrush_Y;
/// Pixel data of the current brush
GFX2_GLOBAL byte * Paintbrush_sprite;
/// Current paintbrush's width
GFX2_GLOBAL short Paintbrush_width;
/// Current paintbrush's height
GFX2_GLOBAL short Paintbrush_height;
/// Position of current paintbrush's handle
GFX2_GLOBAL short Paintbrush_offset_X;
/// Position of current paintbrush's handle
GFX2_GLOBAL short Paintbrush_offset_Y;

// -- Graphic commands

/// On the screen, draw a point.
GFX2_GLOBAL Func_pixel Pixel;
/// Test a pixel color from screen.
GFX2_GLOBAL Func_read Read_pixel;
/// Redraw all screen, without overwriting the menu.
GFX2_GLOBAL Func_display Display_screen;
/// Draw a rectangle on screen.
GFX2_GLOBAL Func_block Block;
/// Draw a point from the image to screen (no zoom).
GFX2_GLOBAL Func_pixel Pixel_preview_normal;
/// Draw a point from the image to screen (magnified part).
GFX2_GLOBAL Func_pixel Pixel_preview_magnifier;
/// Draw a point from the image to screen (zoomed if needed).
GFX2_GLOBAL Func_pixel Pixel_preview;
/// Draw a horizontal XOR line on screen.
GFX2_GLOBAL Func_line_XOR Horizontal_XOR_line;
/// Draw a vertical XOR line on screen.
GFX2_GLOBAL Func_line_XOR Vertical_XOR_line;
/// Display part of the brush on screen, color mode.
GFX2_GLOBAL Func_display_brush_color Display_brush_color;
/// Display part of the brush on screen, monochrome mode.
GFX2_GLOBAL Func_display_brush_mono  Display_brush_mono;
/// Clear the brush currently displayed on screen, redrawing the image instead.
GFX2_GLOBAL Func_display_brush_color Clear_brush;
/// Remap part of the screen after the menu colors have changed.
GFX2_GLOBAL Func_remap     Remap_screen;
/// Draw a line on screen.
GFX2_GLOBAL Func_procsline Display_line;
/// Draw a line on screen, without doubling it if using wide pixels. (to be used when the line is already doubled in the input buffer)
GFX2_GLOBAL Func_procsline Display_line_fast;
/// Read a line of pixels from screen.
GFX2_GLOBAL Func_procsline Read_line;
/// Redraw all magnified part on screen, without overwriting the menu.
GFX2_GLOBAL Func_display_zoom Display_zoomed_screen;
/// Display part of the brush on the magnified part of screen, color mode.
GFX2_GLOBAL Func_display_brush_color_zoom Display_brush_color_zoom;
/// Display part of the brush on the magnified part of screen, monochrome mode.
GFX2_GLOBAL Func_display_brush_mono_zoom  Display_brush_mono_zoom;
/// Clear the brush currently displayed on the magnified part of screen, redrawing the image instead.
GFX2_GLOBAL Func_display_brush_color_zoom Clear_brush_scaled;
/// Draw an arbitrary brush on screen (not the current brush)
GFX2_GLOBAL Func_draw_brush Display_brush;

// -- Screen data

/// Requested window width. This is set when the user resizes the window.
GFX2_GLOBAL int   Resize_width;
/// Requested window height. This is set when the user resizes the window.
GFX2_GLOBAL int   Resize_height;
/// Current video mode. Index in ::Video_mode
GFX2_GLOBAL int   Current_resolution;
/// After loading an image, this holds the "original screen width", if the file format supported it.
GFX2_GLOBAL short Original_screen_X;
/// After loading an image, this holds the "original screen height", if the file format supported it.
GFX2_GLOBAL short Original_screen_Y;
///
/// Current screen (or window) width, in pixels.
/// Note that this takes ::Pixel_ratio into account.
GFX2_GLOBAL short Screen_width;
///
/// Current screen (or window) height, in pixels.
/// Note that this takes ::Pixel_ratio into account.
GFX2_GLOBAL short Screen_height;
/// Coordinate (in image space) of the topmost visible pixel.
GFX2_GLOBAL short Limit_top;
///
/// Coordinate (in image space) of the lowest visible pixel.
/// This can be larger than the image height, if the screen is bigger than image.
GFX2_GLOBAL short Limit_bottom;
/// Coordinate (in image space) of the leftmost visible pixel.
GFX2_GLOBAL short Limit_left;
///
/// Coordinate (in image space) of the rightmost visible pixel.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_right;
///
/// Coordinate (in image space) of the lowest visible pixel, limited by the
/// image height. Compare with ::Limit_bottom, which is not clipped.
GFX2_GLOBAL short Limit_visible_bottom;
///
/// Coordinate (in image space) of the rightmost visible pixel, limited by the
/// image width. Compare with ::Limit_right, which is not clipped.
GFX2_GLOBAL short Limit_visible_right;

/// Coordinate (in image space) of the pixel at the top of the magnified view.
GFX2_GLOBAL short Limit_top_zoom;
///
/// Coordinate (in image space) of the pixel at the bottom of the magnified view.
/// This can be larger than the image height, if the screen is bigger than image.
GFX2_GLOBAL short Limit_bottom_zoom;
/// Coordinate (in image space) of the pixel at the left of the magnified view.
GFX2_GLOBAL short Limit_left_zoom;
///
/// Coordinate (in image space) of the pixel at the right of the magnified view.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_right_zoom;
///
/// Coordinate (in image space) of the lowest visible pixel, limited by the
/// image height. Compare with ::Limit_bottom, which is not clipped.
GFX2_GLOBAL short Limit_visible_bottom_zoom;
/// Coordinate (in image space) of the rightmost visible pixel.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_visible_right_zoom;

/// Buffer of pixels, used when drawing something to screen.
GFX2_GLOBAL byte * Horizontal_line_buffer;

/// Current pixel ratio. Index in enum ::PIXEL_RATIO
GFX2_GLOBAL int Pixel_ratio;
/// Current width of pixels, according to ::Pixel_ratio
GFX2_GLOBAL int Pixel_width;
/// Current height of pixels, according to ::Pixel_ratio
GFX2_GLOBAL int Pixel_height;


// -- Current image data

/// Pointer to the pixel data of the main image
GFX2_GLOBAL byte *    Main_screen;
/// Palette of the main image
GFX2_GLOBAL T_Palette Main_palette;
/// Boolean, means the image has been modified since last save.
GFX2_GLOBAL byte  Main_image_is_modified;
/// Width in pixels of the main image.
GFX2_GLOBAL short Main_image_width;
/// Height in pixels of the main image.
GFX2_GLOBAL short Main_image_height;
/// X position (in image space) of the pixel to display in the top left corner of screen.
GFX2_GLOBAL short Main_offset_X;
/// Y position (in image space) of the pixel to display in the top left corner of screen.
GFX2_GLOBAL short Main_offset_Y;
/// Backup of ::Main_offset_X, used to store it while the magnifier is open.
GFX2_GLOBAL short Old_main_offset_X;
/// Backup of ::Main_offset_Y, used to store it while the magnifier is open.
GFX2_GLOBAL short Old_main_offset_Y;
/// Name of the directory that holds the image currently edited.
GFX2_GLOBAL char  Main_file_directory[1024];
/// Filename (without directory) of the image currently edited.
GFX2_GLOBAL char  Main_filename[256];
/// File format of the image currently edited. It's a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Main_fileformat;
///
/// Fileselector "filter" format, for the current image.
/// (The spare page has its own separate settings)
/// It's 0 for "*.*", or a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Main_format;
/// Index of the first file/entry to display in the fileselector.
GFX2_GLOBAL short Main_fileselector_position;
///
/// Position of the "highlight" bar in the fileselector. 10 Files can be visible,
/// So it's a number in the [0-9] range.
GFX2_GLOBAL short Main_fileselector_offset;
/// Current directory for the fileselector.
GFX2_GLOBAL char  Main_current_directory[1024];
/// Main image's file comment (some image formats support text strings).
GFX2_GLOBAL char  Main_comment[COMMENT_SIZE+1];
/// X position (in screen coordinates) of the separator between normal and magnified views.
GFX2_GLOBAL short Main_separator_position;
/// X position (in screen coordinates) of the first pixel of the magnified view.
GFX2_GLOBAL short Main_X_zoom;
/// Proportion of the non-magnified part of the screen.
GFX2_GLOBAL float Main_separator_proportion;
/// Boolean, true if the main image has the magnifier active.
GFX2_GLOBAL byte  Main_magnifier_mode;
/// Zoom factor used in the magnifier (main image).
GFX2_GLOBAL word  Main_magnifier_factor;
/// Height of the magnified view for the main image.
GFX2_GLOBAL word  Main_magnifier_height;
/// Width of the magnified view for the main image.
GFX2_GLOBAL word  Main_magnifier_width;
/// X position (in image space) of the pixel to display in the top left corner of the magnified view.
GFX2_GLOBAL short Main_magnifier_offset_X;
/// Y position (in image space) of the pixel to display in the top left corner of the magnified view.
GFX2_GLOBAL short Main_magnifier_offset_Y;
/// Index of layer currently being edited
GFX2_GLOBAL byte Main_current_layer;
/// Bitfield that records which layers are visible. 2^0 for 0, 2^1 for 1, 2^2 for 2, etc.
GFX2_GLOBAL dword Main_layers_visible;
/// Index to use next time, when creating incremental backups, to make unique filename.
GFX2_GLOBAL long Main_safety_number;
/// Number of edit actions since the last safety backup
GFX2_GLOBAL long Main_edits_since_safety_backup;
/// SDL Time of the previous safety backup
GFX2_GLOBAL Uint32 Main_time_of_safety_backup;
/// Letter prefix for the filenames of safety backups. a or b
GFX2_GLOBAL byte Main_safety_backup_prefix;

// -- Spare page data

/// Palette of the spare page
GFX2_GLOBAL T_Palette Spare_palette;
/// Boolean, means the spare page has been modified since last save.
GFX2_GLOBAL byte  Spare_image_is_modified;
/// Width in pixels of the spare image.
GFX2_GLOBAL short Spare_image_width;
/// Height in pixels of the spare image.
GFX2_GLOBAL short Spare_image_height;
/// X position (in image space) of the pixel to display in the top left corner of screen.
GFX2_GLOBAL short Spare_offset_X;
/// Y position (in image space) of the pixel to display in the top left corner of screen.
GFX2_GLOBAL short Spare_offset_Y;
/// Backup of ::Main_offset_X, used to store it while the magnifier is open.
GFX2_GLOBAL short Old_spare_offset_X;
/// Backup of ::Main_offset_Y, used to store it while the magnifier is open.
GFX2_GLOBAL short Old_spare_offset_Y;
/// Name of the directory that holds the image currently edited as spare page.
GFX2_GLOBAL char  Spare_file_directory[MAX_PATH_CHARACTERS];
/// Filename (without directory) of the image currently edited as spare page.
GFX2_GLOBAL char  Spare_filename[MAX_PATH_CHARACTERS];
/// File format of the image currently edited as spare page. It's a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Spare_fileformat;
///
/// Fileselector "filter" format, for the spare page.
/// (The main image has its own separate settings)
/// It's 0 for "*.*", or a value of enum ::FILE_FORMAT
GFX2_GLOBAL byte  Spare_format;
/// Index of the first file/entry to display in the fileselector.
GFX2_GLOBAL short Spare_fileselector_position;
///
/// Position of the "highlight" bar in the fileselector. 10 Files can be visible,
/// So it's a number in the [0-9] range.
GFX2_GLOBAL short Spare_fileselector_offset;
/// Current directory for the fileselector.
GFX2_GLOBAL char  Spare_current_directory[MAX_PATH_CHARACTERS];
/// Spare page's file comment  (some image formats support text strings).
GFX2_GLOBAL char  Spare_comment[COMMENT_SIZE+1];
/// X position (in screen coordinates) of the separator between normal and magnified views.
GFX2_GLOBAL short Spare_separator_position;
/// X position (in screen coordinates) of the first pixel of the magnified view.
GFX2_GLOBAL short Spare_X_zoom;
/// Proportion of the non-magnified part of the screen.
GFX2_GLOBAL float Spare_separator_proportion;
/// Boolean, true if the main image has the magnifier active.
GFX2_GLOBAL byte  Spare_magnifier_mode;
/// Zoom factor used in the magnifier (spare page).
GFX2_GLOBAL word  Spare_magnifier_factor;
/// Width of the magnified view for the spare page.
GFX2_GLOBAL word  Spare_magnifier_height;
/// Height of the magnified view for the spare page.
GFX2_GLOBAL word  Spare_magnifier_width;
/// X position (in image space) of the pixel to display in the top left corner of the magnified view.
GFX2_GLOBAL short Spare_magnifier_offset_X;
/// Y position (in image space) of the pixel to display in the top left corner of the magnified view.
GFX2_GLOBAL short Spare_magnifier_offset_Y;
/// Index of layer currently being edited
GFX2_GLOBAL byte Spare_current_layer;
/// Bitfield that records which layers are visible. 2^0 for 0, 2^1 for 1, 2^2 for 2, etc.
GFX2_GLOBAL dword Spare_layers_visible;
/// Index to use next time, when creating incremental backups, to make unique filename.
GFX2_GLOBAL long Spare_safety_number;
/// Number of edit actions since the last safety backup
GFX2_GLOBAL long Spare_edits_since_safety_backup;
/// SDL Time of the previous safety backup
GFX2_GLOBAL Uint32 Spare_time_of_safety_backup;
/// Letter prefix for the filenames of safety backups. a or b
GFX2_GLOBAL byte Spare_safety_backup_prefix;

// -- Image backups

/// Backup of the current screen, used during drawing when FX feedback is OFF.
GFX2_GLOBAL byte * Screen_backup;
/// List of backup pages for the main image.
GFX2_GLOBAL T_List_of_pages * Main_backups;
/// List of backup pages for the spare page.
GFX2_GLOBAL T_List_of_pages * Spare_backups;


// -- Brush data

/// Pixel data of the current brush.
GFX2_GLOBAL byte * Brush;
/// X coordinate of the brush's "hot spot". It is < ::Brush_width
GFX2_GLOBAL word Brush_offset_X;
/// Y coordinate of the brush's "hot spot". It is < ::Brush_height
GFX2_GLOBAL word Brush_offset_Y;
/// Width of the current brush.
GFX2_GLOBAL word Brush_width;
/// Height of the current brush.
GFX2_GLOBAL word Brush_height;
/// Name of the directory that holds the brush fil (after loading or saving it).
GFX2_GLOBAL char  Brush_file_directory[MAX_PATH_CHARACTERS];
/// Filename (without directory) of the brush (after loading or saving it).
GFX2_GLOBAL char  Brush_filename[MAX_PATH_CHARACTERS];
/// File format of the brush. It's a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Brush_fileformat;
///
/// Fileselector "filter" format, for the brush.
/// It's 0 for "*.*", or a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Brush_format;
/// Index of the first file/entry to display in the brush's fileselector.
GFX2_GLOBAL short Brush_fileselector_position;
///
/// Position of the "highlight" bar in the brush's fileselector. 10 Files can
/// be visible, so it's a number in the [0-9] range.
GFX2_GLOBAL short Brush_fileselector_offset;
/// Current directory for the brush's fileselector.
GFX2_GLOBAL char  Brush_current_directory[256];
/// File comment in the brush's fileselector (some image formats support text strings).
GFX2_GLOBAL char  Brush_comment[COMMENT_SIZE+1];
/// Indicator used for the "Rotate brush" operation.
GFX2_GLOBAL byte  Brush_rotation_center_is_defined;
/// Position of the brush's rotation center, in screen coordinates.
GFX2_GLOBAL short Brush_rotation_center_X;
/// Position of the brush's rotation center, in screen coordinates.
GFX2_GLOBAL short Brush_rotation_center_Y;

// -- Menu data (toolbox)

/// Boolean, true if the menu has to be displayed.
GFX2_GLOBAL byte  Menu_is_visible;
/// Height of the menu, when it's displayed
GFX2_GLOBAL word  Menu_height;
///
/// Y position (in screen coordinates) where the menu begins.
/// This is always either ::Screen_height (when menu is hidden) or (::Screen_height - ::Menu_height)
/// As a result, the drawing algoritm always draws the image from 0 to ::Menu_Y-1
GFX2_GLOBAL word  Menu_Y;
/// Y position of the status bar (in screen coordinates)
GFX2_GLOBAL word  Menu_status_Y;
/// Scaling factor for the menu and all GUI elements
GFX2_GLOBAL byte  Menu_factor_X;
/// Scaling factor for the menu and all GUI elements
GFX2_GLOBAL byte  Menu_factor_Y;
/// Size of a color cell in the menu's palette.
GFX2_GLOBAL word  Menu_palette_cell_width;

GFX2_GLOBAL T_Menu_Bar Menu_bars[MENUBAR_COUNT] 
#ifdef GLOBAL_VARIABLES
  = 
{{MENU_WIDTH,  9, 1, 45, {NULL,NULL,NULL},  20, BUTTON_HIDE }, // Status
 {MENU_WIDTH, 10, 1, 35, {NULL,NULL,NULL}, 144, BUTTON_LAYER_SELECT }, // Layers
 {MENU_WIDTH, 35, 1,  0, {NULL,NULL,NULL}, 254, BUTTON_CHOOSE_COL }} // Main
#endif
 ;


// -- Window data

/// Number of stacked windows currently displayed. 0 when no window is present.
GFX2_GLOBAL byte Windows_open;
/// Backup of ::Menu_is_visible, used to store it while a window is open.
GFX2_GLOBAL byte Menu_is_visible_before_window;
/// Backup of ::Menu_Y, used to store it while a window is open.
GFX2_GLOBAL word Menu_Y_before_window;
/// Backup of ::Paintbrush_hidden, used to store it while a window is open.
GFX2_GLOBAL byte Paintbrush_hidden_before_window;

GFX2_GLOBAL word Window_stack_pos_X[8];
/// Position of the left border of the topmost window (in screen coordinates)
#define Window_pos_X Window_stack_pos_X[Windows_open-1]

GFX2_GLOBAL word Window_stack_pos_Y[8];
/// Position of the top border of the topmost window (in screen coordinates)
#define Window_pos_Y Window_stack_pos_Y[Windows_open-1]

GFX2_GLOBAL word Window_stack_width[8];
///
/// Width of the topmost window, in "window pixels"
/// (multiply by ::Menu_factor_X to get screen pixels)
#define Window_width Window_stack_width[Windows_open-1]

GFX2_GLOBAL word Window_stack_height[8];
///
/// Height of the topmost window, in "window pixels"
/// (multiply by ::Menu_factor_Y to get screen pixels)
#define Window_height Window_stack_height[Windows_open-1]

GFX2_GLOBAL word Window_stack_nb_buttons[8];
/// Total number of buttons/controls in the topmost window.
#define Window_nb_buttons Window_stack_nb_buttons[Windows_open-1]

GFX2_GLOBAL T_Normal_button   * Window_stack_normal_button_list[8];
/// List of normal buttons in the topmost window.
#define Window_normal_button_list Window_stack_normal_button_list[Windows_open-1]

GFX2_GLOBAL T_Palette_button  * Window_stack_palette_button_list[8];
/// List of "palette" buttons in the topmost window.
#define Window_palette_button_list Window_stack_palette_button_list[Windows_open-1]

GFX2_GLOBAL T_Scroller_button * Window_stack_scroller_button_list[8];
/// List of sliders (scrollers) in the topmost window.
#define Window_scroller_button_list Window_stack_scroller_button_list[Windows_open-1]

GFX2_GLOBAL T_Special_button  * Window_stack_special_button_list[8];
/// List of special buttons in the topmost window.
#define Window_special_button_list Window_stack_special_button_list[Windows_open-1]

GFX2_GLOBAL T_Dropdown_button  * Window_stack_dropdown_button_list[8];
/// List of dropdown buttons in the topmost window.
#define Window_dropdown_button_list Window_stack_dropdown_button_list[Windows_open-1]

GFX2_GLOBAL T_List_button  * Window_stack_list_button_list[8];
/// List of list buttons in the topmost window.
#define Window_list_button_list Window_stack_list_button_list[Windows_open-1]



GFX2_GLOBAL int Window_stack_attribute1[8];
///
/// The function ::Window_clicked_button() set this to ::LEFT_SIDE or ::RIGHT_SIDE
/// after a button is activated through left or right mouse click.
#define Window_attribute1 Window_stack_attribute1[Windows_open-1]

GFX2_GLOBAL int Window_stack_attribute2[8];
///
/// The function ::Window_clicked_button() set this to return extra information:
/// - When a scroller was clicked: the scroller position (0-n)
/// - When a palette was clicked: the color index (0-255)
/// - When a dropdown was used: the selected item's number T_Dropdown_choice::Number
#define Window_attribute2 Window_stack_attribute2[Windows_open-1]




/// Definition of the menu (toolbox)
GFX2_GLOBAL struct
{
  // Button aspect
  word            X_offset;         ///< Position relative to menu's left
  word            Y_offset;         ///< Position relative to menu's top
  word            Width;            ///< Button's active width
  word            Height;           ///< Button's active heigth
  byte            Pressed;          ///< Button is currently pressed
  byte            Shape;            ///< Shape, listed in enum ::BUTTON_SHAPES
  char            Icon;             ///< Which icon to display: Either the one from the toolbar (-1) or one of ::MENU_SPRITE

  // Triggers on mouse/keyboard
  Func_action     Left_action;      ///< Action triggered by a left mouseclick on the button
  Func_action     Right_action;     ///< Action triggered by a right mouseclick on the button
  word            Left_shortcut[2]; ///< Keyboard shortcut for a left mouseclick
  word            Right_shortcut[2];///< Keyboard shortcut for a right mouseclick
  byte            Left_instant;     ///< Will not wait for mouse release before triggering action
  byte            Right_instant;    ///< Will not wait for mouse release before triggering action

  // Data used when the button is unselected
  Func_action     Unselect_action;  ///< Action triggered by unselecting the button
  byte            Family;           ///< enum ::FAMILY_OF_BUTTONS.

} Buttons_Pool[NB_BUTTONS];



// -- Information about the different drawing modes (effects)

/// Current effecting function. When no effect is selected this is ::No_effect()
GFX2_GLOBAL Func_effect Effect_function;

///
/// Array of booleans, indicates which colors should never be picked by
/// ::Best_color()
GFX2_GLOBAL byte Exclude_color[256];

// -- Smear mode

/// Smear mode is activated
GFX2_GLOBAL byte  Smear_mode;
/// Boolean, indicates that a smear is in progress.
GFX2_GLOBAL byte  Smear_start;
/// Pointer to the sprite to use for smear; it contains pixels from the image.
GFX2_GLOBAL byte * Smear_brush;
/// Width of the ::Smear_brush
GFX2_GLOBAL word  Smear_brush_width;
/// Height of the ::Smear_brush
GFX2_GLOBAL word  Smear_brush_height;
/// Limits of the smear.
GFX2_GLOBAL short Smear_min_X;
/// Limits of the smear.
GFX2_GLOBAL short Smear_max_X;
/// Limits of the smear.
GFX2_GLOBAL short Smear_min_Y;
/// Limits of the smear.
GFX2_GLOBAL short Smear_max_Y;

// -- Shade mode
/// List of the shade tables
GFX2_GLOBAL T_Shade        Shade_list[8];
/// Shade currently selected (index in ::Shade_list)
GFX2_GLOBAL byte           Shade_current;
/// Conversion table in use
GFX2_GLOBAL byte *         Shade_table;
/// Conversion table for a left click
GFX2_GLOBAL byte           Shade_table_left[256];
/// Conversion table for a right click
GFX2_GLOBAL byte           Shade_table_right[256];
/// Boolean, true when the shade mode is active.
GFX2_GLOBAL byte           Shade_mode;

/// Boolean, true when the quick-shade mode is active.
GFX2_GLOBAL byte           Quick_shade_mode;
/// Size of the step, in Quick-shade mode. It's the number of colors to "jump".
GFX2_GLOBAL byte           Quick_shade_step;
/// Determines how colors should loop in Quick-shade more. Value in enum ::SHADE_MODES
GFX2_GLOBAL byte           Quick_shade_loop;

// -- Stencil mode

/// Boolean, true when stencil mode is active.
GFX2_GLOBAL byte Stencil_mode;
/// Array of the protected colors by Stencil mode.
GFX2_GLOBAL byte Stencil[256];

// -- Grid mode

/// Boolean, true when the Grid mode is active.
GFX2_GLOBAL byte  Snap_mode;
/// Boolean, true when the Grid is displayed in zoomed view.
GFX2_GLOBAL byte  Show_grid;
/// Width of the grid in Grid mode.
GFX2_GLOBAL word Snap_width;
/// Height of the grid in Grid mode.
GFX2_GLOBAL word Snap_height;
/// Position of the starting pixel, in Grid mode.
GFX2_GLOBAL word Snap_offset_X;
/// Position of the starting pixel, in Grid mode.
GFX2_GLOBAL word Snap_offset_Y;

// -- Sieve mode

/// Boolean, true when the Sieve mode is active
GFX2_GLOBAL byte  Sieve_mode;
/// Sprite of the sieve pattern. It's actually an array of booleans.
GFX2_GLOBAL byte  Sieve[16][16];
/// Width of the sieve pattern, in Sieve mode.
GFX2_GLOBAL short Sieve_width;
/// Height of the sieve pattern, in Sieve mode.
GFX2_GLOBAL short Sieve_height;

// -- Colorize mode

/// Boolean, true when the Colorize mode is active.
GFX2_GLOBAL byte Colorize_mode;
/// % of opacity of Colorize mode (for translucency)
GFX2_GLOBAL byte Colorize_opacity;
/// Sets the colorization mode: 0 transparency, 1 additive, 2 substractive
GFX2_GLOBAL byte Colorize_current_mode;
///
/// Table of precomputed factors used by Colorize mode. It hold 0 to 255 when
/// opacity is 100%, 0 to 128 when opacity is 50%, etc.
// FIXME: This only caches a multiplication and a division. Maybe we should scrap it
GFX2_GLOBAL word Factors_table[256];
///
/// Table of precomputed factors used by Colorize mode. It hold 255 to 0 when
/// opacity is 100%, 128 to 0 when opacity is 50%, etc.
// FIXME: This only caches a multiplication, a division, a substraction. Maybe we should scrap it
GFX2_GLOBAL word Factors_inv_table[256];

// -- Smooth mode

/// Boolean, true when the Smooth mode is active
GFX2_GLOBAL byte Smooth_mode;
/// Matrix of "weights" used by the Smooth mode.
GFX2_GLOBAL byte Smooth_matrix[3][3];

// -- Tiling mode

/// Boolean, true when the Tiling mode is active
GFX2_GLOBAL byte  Tiling_mode;
/// Position of the starting pixel in Tiling mode.
GFX2_GLOBAL short Tiling_offset_X;
/// Position of the starting pixel in Tiling mode.
GFX2_GLOBAL short Tiling_offset_Y;

// -- Mask mode

/// Boolean, true when the Tiling mode is active
GFX2_GLOBAL byte Mask_mode;
/// Array of booleans. True if the indexed color is protected by the mask.
GFX2_GLOBAL byte Mask_table[256];

// -- Magnifier data

#ifdef GLOBAL_VARIABLES
  word ZOOM_FACTOR[NB_ZOOM_FACTORS]={2,3,4,5,6,8,10,12,14,16,18,20, 24, 28, 32};
#else
/// Successive zoom factors, used by the Magnifier.
  extern word ZOOM_FACTOR[NB_ZOOM_FACTORS];
#endif

// -- Data for ellipses and circles
// FIXME: move most of these to graph.c
GFX2_GLOBAL long  Ellipse_cursor_X;
GFX2_GLOBAL long  Ellipse_cursor_Y;
GFX2_GLOBAL long  Ellipse_vertical_radius_squared;
GFX2_GLOBAL long  Ellipse_horizontal_radius_squared;
GFX2_GLOBAL qword Ellipse_limit;
GFX2_GLOBAL long  Circle_cursor_X;
GFX2_GLOBAL long  Circle_cursor_Y;
GFX2_GLOBAL long  Circle_limit;

// -- Data for gradients

/// First color of the gradient.
GFX2_GLOBAL short Gradient_lower_bound;
/// Last color of the gradient
GFX2_GLOBAL short Gradient_upper_bound;
/// Boolean, true if the gradient should use colors in descending order
GFX2_GLOBAL int   Gradient_is_inverted;
/// Number of colors in the range ::Gradient_lower_bound to ::Gradient_upper_bound (included)
GFX2_GLOBAL long  Gradient_bounds_range;
/// Maximum value passed to the gradient function. The pixels assigned this value should use last gradient color.
GFX2_GLOBAL long  Gradient_total_range;
/// Amount of randomness to use in gradient (1-256+)
GFX2_GLOBAL long  Gradient_random_factor;
/// Pointer to a gradient function, depending on the selected method.
GFX2_GLOBAL Func_gradient Gradient_function;
///
/// Pointer to the pixel-drawing function that gradients should use:
/// either ::Pixel (if the gradient must be drawn on menus only)
/// or ::Display_pixel (if the gradient must be drawn on the image)
GFX2_GLOBAL Func_pixel Gradient_pixel;
/// Settings for all gradients
GFX2_GLOBAL T_Gradient_array Gradient_array[16];
/// Index in ::Gradient_array of the currently selected gradient.
GFX2_GLOBAL byte Current_gradient;

// -- Airbrush data

/// Mode to use in airbrush: 0 for multicolor, 1 for mono.
GFX2_GLOBAL byte  Airbrush_mode;
/// Diameter of the airbrush, in pixels.
GFX2_GLOBAL short Airbrush_size;
/// Delay between two airbrush "shots", in 1/100s
GFX2_GLOBAL byte  Airbrush_delay;
/// Number of pixels that are emitted by the airbrush, in mono mode.
GFX2_GLOBAL byte  Airbrush_mono_flow;
/// Number of pixels that are emitted by the airbrush for each color (multi mode)
GFX2_GLOBAL byte  Airbrush_multi_flow[256];

/// -- Misc data about the program

/// Boolean, set to true to exit the program.
GFX2_GLOBAL byte Quitting;
/// Name of the directory that was current when the program was run.
GFX2_GLOBAL char Initial_directory[256];
/// Name of the directory that holds the program's (read-only) data: skins, icon, etc.
GFX2_GLOBAL char Data_directory[256];
/// Name of the directory where grafx2 reads and writes configuration (gfx2.ini, gfx2.cfg)
GFX2_GLOBAL char Config_directory[256];
/// Current foreground color for drawing.
GFX2_GLOBAL byte Fore_color;
/// Current background color for drawing.
GFX2_GLOBAL byte Back_color;
/// For the "Freehand draw" tool, this determines which variant is selected, from ::OPERATION_CONTINUOUS_DRAW to ::OPERATION_FILLED_CONTOUR
GFX2_GLOBAL byte Selected_freehand_mode;
/// For the Curve tool, this determines which variant is selected, either ::OPERATION_3_POINTS_CURVE or ::OPERATION_4_POINTS_CURVE
GFX2_GLOBAL byte Selected_curve_mode;
/// For the Line tool, this determines which variant is selected, either ::OPERATION_LINE, ::OPERATION_K_LIGNE or ::OPERATION_CENTERED_LINES
GFX2_GLOBAL byte Selected_line_mode;
/// Determines which color appears in the first cell of the menu palette. Change this value to "scroll" the palette.
GFX2_GLOBAL byte First_color_in_palette;
/// Boolean, true if Grafx2 was run with a command-line argument to set a resolution on startup (overrides config)
GFX2_GLOBAL byte Resolution_in_command_line;

// - Graphic

/// Pointer to the font selected for menus.
GFX2_GLOBAL byte * Menu_font;

/// Pointer to the current active skin.
GFX2_GLOBAL T_Gui_skin * Gfx;

/// Pointer to the current active skin.
GFX2_GLOBAL T_Paintbrush Paintbrush[NB_PAINTBRUSH_SPRITES];

// -- Help data

/// Index of the ::Help_section shown by the Help screen.
GFX2_GLOBAL byte Current_help_section;
/// Line number of the help viewer, in current ::Help_section. 0 for top, increase value to scroll down.
GFX2_GLOBAL word Help_position;

// -- Operation data

/// Index of the operation which was selected (ex: drawing rectangle) before the current interruption (ex: colorpicking).
GFX2_GLOBAL word Operation_before_interrupt;
/// Index of the current operation. This is the active "tool".
GFX2_GLOBAL word Current_operation;
///
/// This stack is used to memorize all parameters needed during the course of
/// an operation. For example when drawing a rectangle: color, starting
/// coordinates, ending coordinates.
GFX2_GLOBAL word Operation_stack[OPERATION_STACK_SIZE];
/// Number of parameters stored in ::Operation_stack (0=empty)
GFX2_GLOBAL byte Operation_stack_size;
/// Boolean, true if the operation (drawing) started in the magnified area.
GFX2_GLOBAL byte Operation_in_magnifier;
/// Last color hovered by the colorpicker. -1 if it wasn't over the image.
GFX2_GLOBAL short Colorpicker_color;
/// Position of the colorpicker tool, in image coordinates.
GFX2_GLOBAL short Colorpicker_X;
/// Position of the colorpicker tool, in image coordinates.
GFX2_GLOBAL short Colorpicker_Y;

GFX2_GLOBAL short * Polyfill_table_of_points;
GFX2_GLOBAL int Polyfill_number_of_points;

/// Brush container
GFX2_GLOBAL T_Brush_template Brush_container[BRUSH_CONTAINER_COLUMNS*BRUSH_CONTAINER_ROWS];

#ifdef GLOBAL_VARIABLES
  byte CURSOR_FOR_OPERATION[NB_OPERATIONS]=
  {
    CURSOR_SHAPE_TARGET            , // Freehand continuous draw
    CURSOR_SHAPE_TARGET            , // Freehand discontinuous draw
    CURSOR_SHAPE_TARGET            , // Freehand point-by-point draw
    CURSOR_SHAPE_TARGET            , // Filled contour
    CURSOR_SHAPE_TARGET            , // Lines
    CURSOR_SHAPE_TARGET            , // Linked lines
    CURSOR_SHAPE_TARGET            , // Centered lines
    CURSOR_SHAPE_XOR_TARGET        , // Empty rectangle
    CURSOR_SHAPE_XOR_TARGET        , // Filled rectangle
    CURSOR_SHAPE_TARGET            , // Empty circle
    CURSOR_SHAPE_TARGET            , // Filled circle
    CURSOR_SHAPE_TARGET            , // Empty ellipse
    CURSOR_SHAPE_TARGET            , // Filled ellipse
    CURSOR_SHAPE_TARGET            , // Fill
    CURSOR_SHAPE_TARGET            , // Color replacer
    CURSOR_SHAPE_XOR_TARGET        , // Rectangular brush grabbing
    CURSOR_SHAPE_TARGET            , // Polygonal brush grabbing
    CURSOR_SHAPE_COLORPICKER       , // Colorpicker
    CURSOR_SHAPE_XOR_RECTANGLE     , // Position the magnify window
    CURSOR_SHAPE_TARGET            , // Curve with 3 control points
    CURSOR_SHAPE_TARGET            , // Curve with 4 control points
    CURSOR_SHAPE_TARGET            , // Airbrush
    CURSOR_SHAPE_TARGET            , // Polygon
    CURSOR_SHAPE_TARGET            , // Polyform
    CURSOR_SHAPE_TARGET            , // Filled polygon
    CURSOR_SHAPE_TARGET            , // Filled polyform
    CURSOR_SHAPE_MULTIDIRECTIONAL  , // Scroll (pan)
    CURSOR_SHAPE_TARGET            , // Gradient-filled circle
    CURSOR_SHAPE_TARGET            , // Gradient-filled ellipse
    CURSOR_SHAPE_XOR_ROTATION      , // Rotate brush
    CURSOR_SHAPE_XOR_TARGET        , // Stretch brush
    CURSOR_SHAPE_TARGET            , // Distort brush
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled rectangle
    CURSOR_SHAPE_COLORPICKER       , // Colorpick on right mouse button
  };
#else
  /// ::Cursor_shape to use for each operation.
  extern byte CURSOR_FOR_OPERATION[NB_OPERATIONS];
#endif

///
/// Procedures to call for each state (determined by ::Operation_stack_size) of
/// each operation, and for each mouse state (no button,left button,right button)
GFX2_GLOBAL struct
{
  Func_action Action; ///< Function to call
  byte Hide_cursor;   ///< Boolean: Need to hide/unhide cursor during this step
  byte Fast_mouse;    ///< Operation should take shortcuts with mouse movements
} Operation[NB_OPERATIONS][3][OPERATION_STACK_SIZE];

// -- misc

///
/// Indicator of error in previous file operations.
/// -  0: OK
/// -  1: Error when beginning operation. Existing data should be ok.
/// -  2: Error while operation was in progress. Data is modified.
/// - -1: Interruption of a preview.
GFX2_GLOBAL signed char File_error;
/// Current line number when reading/writing gfx2.ini
GFX2_GLOBAL int Line_number_in_INI_file;

// -- Specific to SDL

/// Pointer to the program's screen.
GFX2_GLOBAL SDL_Surface * Screen_SDL;
/// Pointer to the current joystick controller.
GFX2_GLOBAL SDL_Joystick* Joystick;

/// Indicates "no keyboard shortcut".
#define KEY_NONE            0
///
/// This is the "key identifier" for the mouse 3rd button.
/// It was chosen to not conflict with any SDL key number.
#define KEY_MOUSEMIDDLE     (SDLK_LAST+1)
///
/// This is the "key identifier" for the mouse wheelup.
/// It was chosen to not conflict with any SDL key number.
#define KEY_MOUSEWHEELUP    (SDLK_LAST+2)
///
/// This is the "key identifier" for the mouse wheeldown.
/// It was chosen to not conflict with any SDL key number.
#define KEY_MOUSEWHEELDOWN  (SDLK_LAST+3)
///
/// This is the "key identifier" for joystick button number 0.
/// All numbers starting with this one are reserved for joystick buttons
/// (since their is an unknown number of them, and for example 18 on GP2X)
/// It was chosen to not conflict with any SDL key number.
#define KEY_JOYBUTTON       (SDLK_LAST+4)

/// Button definitions for the gp2x
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_Y               (14)
#define GP2X_BUTTON_X               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

#ifdef __gp2x__
  #define KEY_ESC (KEY_JOYBUTTON+GP2X_BUTTON_X)
#else
  ///
  /// This is the key identifier for ESC. When hard-coding keyboard shortcuts
  /// for buttons, etc. we use this instead of SDLK_ESCAPE,
  /// so the GP2X port can get a joybutton equivalent of it.
  #define KEY_ESC SDLK_ESCAPE
#endif

#endif
