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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

//////////////////////////////////////////////////////////////////////////////
///@file const.h
/// Constants (preprocessor defines) and enumerations used anywhere.
//////////////////////////////////////////////////////////////////////////////

#ifndef _CONST_H_
#define _CONST_H_

#define M_2PI 6.28318530717958647692528676656 ///< Hmm, pie...

#define VERSION1                  2     ///< Version number for gfx2.cfg (1/4)
#define VERSION2                  0     ///< Version number for gfx2.cfg (2/4)
#define BETA1                     98    ///< Version number for gfx2.cfg (3/4)
#define BETA2                     0     ///< Version number for gfx2.cfg (4/4)
#define MAX_VIDEO_MODES           100   ///< Maximum number of video modes Grafx2 can propose.
#define NB_SHORTCUTS              181   ///< Number of actions that can have a key combination associated to it.
#define NB_ZOOM_FACTORS           12    ///< Number of zoom levels available in the magnifier.
#define MENU_WIDTH                254   ///< Width of the menu (not counting the palette)
#define MENU_HEIGHT               44    ///< Height of the menu.
#define NB_CURSOR_SPRITES         8     ///< Number of available mouse cursor sprites.
#define CURSOR_SPRITE_WIDTH       15    ///< Width of a mouse cursor sprite.
#define CURSOR_SPRITE_HEIGHT      15    ///< Height of a mouse cursor sprite.
#define NB_EFFECTS_SPRITES        9     ///< Number of effect sprites.
#define NB_MENU_SPRITES           20    ///< Number of menu sprites.
#define MENU_SPRITE_WIDTH         14    ///< Width of a menu sprite in pixels
#define MENU_SPRITE_HEIGHT        14    ///< Height of a menu sprite in pixels
#define PAINTBRUSH_WIDTH          16    ///< Width of a preset paintbrush sprite
#define PAINTBRUSH_HEIGHT         16    ///< Height of a preset paintbrush sprite
#define MAX_PAINTBRUSH_SIZE       127   ///< Max size for a resizable paintbrush
#define ICON_SPRITE_WIDTH         8     ///< Width of an icon in pixels
#define ICON_SPRITE_HEIGHT        8     ///< Height of an icon in pixels
#define NB_PAINTBRUSH_SPRITES     48    ///< Number of preset paintbrushes
#define NB_PRESET_SIEVE           12    ///< Number of preset sieve patterns
#define OPERATION_STACK_SIZE      16    ///< Max number of parameters in the operation stack.
#define MAX_DISPLAYABLE_PATH      37    ///< Max number of characters to display directory name, in Save/Load screens.
#define COMMENT_SIZE              32    ///< Max number of characters for a comment in PKM or PNG file.
#define NB_MAX_PAGES_UNDO         99    ///< Max number of undo pages
#define DEFAULT_ZOOM_FACTOR        4    ///< Initial zoom factor for the magnifier.
#define MAX_PATH_CHARACTERS      260    ///< Number of characters for a file+complete path. Adapt to your OS...
#define NB_BOOKMARKS               4    ///< Number of bookmark buttons in Save/Load screen.
// Character to show a right arrow, used when editing long strings. It's present in ::Gfx->System_font 
#define RIGHT_TRIANGLE_CHARACTER  16
// Character to show a left arrow, used when editing long strings. It's present in ::Gfx->System_font 
#define LEFT_TRIANGLE_CHARACTER   17
/// Character to display in menus for an ellipsis.
#define ELLIPSIS_CHARACTER       '…'

#define BRUSH_CONTAINER_PREVIEW_WIDTH    16  ///< Size for preview of a brush in Brush container
#define BRUSH_CONTAINER_PREVIEW_HEIGHT   16  ///< Size for preview of a brush in Brush container
#define BRUSH_CONTAINER_COLUMNS          4  ///< Number of columns in the Brush container
#define BRUSH_CONTAINER_ROWS             3  ///< Number of rows in the Brush container

///
/// We force the dynamic backup page allocation to leave a minimum of 
/// 256Kb of free memory, to allow the rest of the program to work safely.
/// Note: This is a remainder of the DOS version. This system might not work
/// so well on other OSes, where the "available memory" changes due to external
/// factors.
#define MINIMAL_MEMORY_TO_RESERVE (256*1024)

#define LEFT_SIDE                    1     ///< Indicates a left side or left-click
#define RIGHT_SIDE                   2     ///< Indicates a right side or right-click

#define SEPARATOR_WIDTH              6     ///< Width of the separator between the normal and the zoomed view
#define INITIAL_SEPARATOR_PROPORTION 0.3   ///< Proportion of the normal view width, relative to the whole screen width.
#define NB_ZOOMED_PIXELS_MIN         4     ///< Minimal number of pixel shown (in width) by the zoomed view. (Note: below 4, you can't scroll!)

#if defined(__MORPHOS__) || defined(__amigaos4__) || defined(__amigaos__)
   #define PARENT_DIR "/"
#else
   /// Filename that means "parent directory" for your operating system.
   #define PARENT_DIR ".."
#endif

/// List of file formats recognized by grafx2
enum FILE_FORMATS
{
  FORMAT_ALL_IMAGES=0, ///< This is not really a file format, it's reserverd for a compilation of all file extensions
  FORMAT_ALL_FILES=1, ///< This is not really a file format, it's reserverd for the "*.*" filter option.
  FORMAT_PNG,
  FORMAT_GIF,
  FORMAT_BMP,
  FORMAT_PCX,
  FORMAT_PKM,
  FORMAT_LBM,
  FORMAT_IMG,
  FORMAT_SCx,
  FORMAT_PI1,
  FORMAT_PC1,
  FORMAT_CEL,
  FORMAT_NEO,
  FORMAT_C64,
  FORMAT_KCF,
  FORMAT_PAL,
  FORMAT_MISC, ///< Must be last of enum: others formats recognized by SDL_image
};

/// Default format for 'save as'
#define DEFAULT_FILEFORMAT    FORMAT_GIF

/// Error codes for ::Error()
enum ERROR_CODES
{
  ERROR_WARNING=0,         ///< Red flash on screen, non-fatal error
  ERROR_GUI_MISSING,       ///< The graphics file is missing
  ERROR_GUI_CORRUPTED,     ///< The graphics file cannot be parsed for GUI elements
  ERROR_INI_MISSING,       ///< File gfx2def.ini is missing
  ERROR_CFG_MISSING,       ///< File gfx2.cfg is missing (non-fatal)
  ERROR_CFG_CORRUPTED,     ///< File gfx2.cfg couldn't be parsed (non-fatal)
  ERROR_CFG_OLD,           ///< Unknown version of gfx2.cfg : either VERY old or wrong file (non-fatal)
  ERROR_MEMORY,            ///< Out of memory
  ERROR_COMMAND_LINE,      ///< Error in command-line arguments (syntax, or couldn't find the file to open)
  ERROR_FORBIDDEN_MODE,    ///< Graphics mode requested is not supported
  ERROR_SAVING_CFG,        ///< Error while writing gfx2.cfg
  ERROR_MISSING_DIRECTORY, ///< Unable to return to the original "current directory" on program exit
  ERROR_INI_CORRUPTED,     ///< File gfx2.ini couldn't be parsed
  ERROR_SAVING_INI,        ///< Error while writing gfx2.ini
  ERROR_SORRY_SORRY_SORRY  ///< (Page allocation error that shouldn't ever happen, now)
};

/// Available pixel scalers
enum PIXEL_RATIO
{
    PIXEL_SIMPLE=0, ///< Use real pixels
    PIXEL_WIDE,		///< Use wide pixels (2x1) like on Amstrad CPC mode 0
    PIXEL_TALL,		///< Use tall pixels (1x2) like on Amstrad CPC mode 2
    PIXEL_DOUBLE,	///< Use big pixels (2x2) if your LCD screen can't do lowres by itself
    PIXEL_TRIPLE,	///< Use really big pixels (3x3)
    PIXEL_WIDE2,	///< Use big wide pixels (4x2)
    PIXEL_TALL2,	///< Use big tall pixels (2x4)
    PIXEL_QUAD,		///< Use really giant pixels (4x4). You need to have a screen resolution at least 1280x800 to use this one
    PIXEL_MAX ///< Number of elements in enum
};

/// Different kinds of menu button behavior.
enum FAMILY_OF_BUTTONS
{
  FAMILY_TOOL=1,       ///< Drawing tools (example : Freehand draw)
  FAMILY_INTERRUPTION, ///< Temporary operation (example : choosing paintbrush) > Interrupts the current operation to do something, then come back.
  FAMILY_INSTANT,      ///< Single-click action (example : choose a color in palette) > It will be over as soon as we exit the called function.
  FAMILY_TOOLBAR,      ///< Hide/show the menu
  FAMILY_EFFECTS       ///< Effects
};

/// The different kinds of buttons in menus or windows.
enum BUTTON_SHAPES
{
  BUTTON_SHAPE_NO_FRAME,             ///< Ex: the palette
  BUTTON_SHAPE_RECTANGLE,            ///< Ex: Most buttons.
  BUTTON_SHAPE_TRIANGLE_TOP_LEFT,    ///< Ex: Empty rectangles.
  BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT ///< Ex: Filled rectangles.
};

/// The different "mouse cursor" shapes
enum CURSOR_SHAPES
{
  CURSOR_SHAPE_ARROW,
  CURSOR_SHAPE_TARGET,           ///< This one uses the paintbrush
  CURSOR_SHAPE_COLORPICKER,      ///< This one uses the paintbrush
  CURSOR_SHAPE_HOURGLASS,
  CURSOR_SHAPE_MULTIDIRECTIONAL,
  CURSOR_SHAPE_HORIZONTAL,
  CURSOR_SHAPE_THIN_TARGET,      ///< This one uses the paintbrush
  CURSOR_SHAPE_THIN_COLORPICKER, ///< This one uses the paintbrush
  CURSOR_SHAPE_XOR_TARGET,
  CURSOR_SHAPE_XOR_RECTANGLE,
  CURSOR_SHAPE_XOR_ROTATION
};

/// The different shapes that can be used as a paintbrush (paintbrush types go in the beginning)
enum PAINTBRUSH_SHAPES
{
  PAINTBRUSH_SHAPE_ROUND,
  PAINTBRUSH_SHAPE_SQUARE,
  PAINTBRUSH_SHAPE_HORIZONTAL_BAR,
  PAINTBRUSH_SHAPE_VERTICAL_BAR,
  PAINTBRUSH_SHAPE_SLASH,
  PAINTBRUSH_SHAPE_ANTISLASH,
  PAINTBRUSH_SHAPE_RANDOM,     ///< Random pixels in a circle shape, like an airbrush.
  PAINTBRUSH_SHAPE_CROSS,
  PAINTBRUSH_SHAPE_PLUS,
  PAINTBRUSH_SHAPE_DIAMOND,
  PAINTBRUSH_SHAPE_SIEVE_ROUND,
  PAINTBRUSH_SHAPE_SIEVE_SQUARE,
  PAINTBRUSH_SHAPE_MISC,        ///< A raw monochrome bitmap, can't be resized. This must be the last of the preset paintbrush types.
  PAINTBRUSH_SHAPE_POINT,       ///< Used to reduce the paintbrush to a single pixel, during operations like colorpicker.
  PAINTBRUSH_SHAPE_COLOR_BRUSH, ///< User's brush, in color mode
  PAINTBRUSH_SHAPE_MONO_BRUSH,  ///< User's brush, in mono mode
  PAINTBRUSH_SHAPE_MAX          ///< Upper limit.
};

/// Normal resting state for a menu button.
#define BUTTON_RELEASED 0
/// State of a menu button that is being pressed.
#define BUTTON_PRESSED  1

/// The different modes of the Shade
enum SHADE_MODES
{
  SHADE_MODE_NORMAL,
  SHADE_MODE_LOOP,
  SHADE_MODE_NOSAT
};

/// Identifiers for the chunks (data blocks) of gfx2.cfg
enum CHUNKS_CFG
{
  CHUNK_KEYS            = 0, ///< Shortcut keys definitions
  CHUNK_VIDEO_MODES     = 1, ///< List of video modes
  CHUNK_SHADE           = 2, ///< Shade settings
  CHUNK_MASK            = 3, ///< Mask settings
  CHUNK_STENCIL         = 4, ///< Stencil settings
  CHUNK_GRADIENTS       = 5, ///< Gradients
  CHUNK_SMOOTH          = 6, ///< Smooth effect settings
  CHUNK_EXCLUDE_COLORS  = 7, ///< List of excluded colors
  CHUNK_QUICK_SHADE     = 8, ///< QShade effect settings
  CHUNK_GRID            = 9,
  CHUNK_MAX
};

/// Identifiers for the 8x8 icons of ::Gfx->Icon_sprite (most are unused now)
enum ICON_TYPES
{
  ICON_FLOPPY_3_5=0, ///< 3.5 Floppy disk
  ICON_FLOPPY_5_25,  ///< 5.25 Floppy disk
  ICON_HDD,          ///< Hard disk drive
  ICON_CDROM,        ///< CD-ROM
  ICON_NETWORK,      ///< "Network" drive
  ICON_STAR,         ///< Star (favorite)
  ICON_DROPDOWN,     ///< Dropdown arrow
  NB_ICON_SPRITES    ///< Number of 8x8 icons
};

/// Identifiers for the buttons in the menu.
enum BUTTON_NUMBERS
{
  BUTTON_PAINTBRUSHES=0,
  BUTTON_ADJUST,
  BUTTON_DRAW,
  BUTTON_CURVES,
  BUTTON_LINES,
  BUTTON_AIRBRUSH,
  BUTTON_FLOODFILL,
  BUTTON_POLYGONS,
  BUTTON_POLYFILL,
  BUTTON_RECTANGLES,
  BUTTON_FILLRECT,
  BUTTON_CIRCLES,
  BUTTON_FILLCIRC,
  BUTTON_GRADRECT,
  BUTTON_SPHERES,
  BUTTON_BRUSH,
  BUTTON_POLYBRUSH,
  BUTTON_BRUSH_EFFECTS,
  BUTTON_EFFECTS,
  BUTTON_TEXT,
  BUTTON_MAGNIFIER,
  BUTTON_COLORPICKER,
  BUTTON_RESOL,
  BUTTON_PAGE,
  BUTTON_SAVE,
  BUTTON_LOAD,
  BUTTON_SETTINGS,
  BUTTON_CLEAR,
  BUTTON_HELP,
  BUTTON_UNDO,
  BUTTON_KILL,
  BUTTON_QUIT,
  BUTTON_PALETTE,
  BUTTON_PAL_LEFT,
  BUTTON_PAL_RIGHT,
  BUTTON_CHOOSE_COL,
  BUTTON_HIDE,
  NB_BUTTONS            ///< Number of buttons in the menu bar.
};

///
/// Identifiers of special actions that can have a keyboard shortcut.
/// They are special in the sense that there's no button in the menu for them,
/// so it requires a specific handling.
enum SPECIAL_ACTIONS
{
  SPECIAL_MOUSE_UP=0,
  SPECIAL_MOUSE_DOWN,
  SPECIAL_MOUSE_LEFT,
  SPECIAL_MOUSE_RIGHT,
  SPECIAL_CLICK_LEFT,
  SPECIAL_CLICK_RIGHT,
  SPECIAL_NEXT_FORECOLOR,
  SPECIAL_PREVIOUS_FORECOLOR,
  SPECIAL_NEXT_BACKCOLOR,
  SPECIAL_PREVIOUS_BACKCOLOR,
  SPECIAL_SMALLER_PAINTBRUSH,
  SPECIAL_BIGGER_PAINTBRUSH,
  SPECIAL_NEXT_USER_FORECOLOR,
  SPECIAL_PREVIOUS_USER_FORECOLOR,
  SPECIAL_NEXT_USER_BACKCOLOR,
  SPECIAL_PREVIOUS_USER_BACKCOLOR,
  SPECIAL_SCROLL_UP,
  SPECIAL_SCROLL_DOWN,
  SPECIAL_SCROLL_LEFT,
  SPECIAL_SCROLL_RIGHT,
  SPECIAL_SCROLL_UP_FAST,
  SPECIAL_SCROLL_DOWN_FAST,
  SPECIAL_SCROLL_LEFT_FAST,
  SPECIAL_SCROLL_RIGHT_FAST,
  SPECIAL_SCROLL_UP_SLOW,
  SPECIAL_SCROLL_DOWN_SLOW,
  SPECIAL_SCROLL_LEFT_SLOW,
  SPECIAL_SCROLL_RIGHT_SLOW,
  SPECIAL_SHOW_HIDE_CURSOR,
  SPECIAL_DOT_PAINTBRUSH,
  SPECIAL_CONTINUOUS_DRAW,
  SPECIAL_FLIP_X,
  SPECIAL_FLIP_Y,
  SPECIAL_ROTATE_90,
  SPECIAL_ROTATE_180,
  SPECIAL_STRETCH,
  SPECIAL_DISTORT,
  SPECIAL_OUTLINE,
  SPECIAL_NIBBLE,
  SPECIAL_GET_BRUSH_COLORS,
  SPECIAL_RECOLORIZE_BRUSH,
  SPECIAL_ROTATE_ANY_ANGLE,
  SPECIAL_LOAD_BRUSH,
  SPECIAL_SAVE_BRUSH,
  SPECIAL_INVERT_SIEVE,
  SPECIAL_ZOOM_IN,
  SPECIAL_ZOOM_OUT,
  SPECIAL_CENTER_ATTACHMENT,
  SPECIAL_TOP_LEFT_ATTACHMENT,
  SPECIAL_TOP_RIGHT_ATTACHMENT,
  SPECIAL_BOTTOM_LEFT_ATTACHMENT,
  SPECIAL_BOTTOM_RIGHT_ATTACHMENT,
  SPECIAL_EXCLUDE_COLORS_MENU,
  SPECIAL_SHADE_MODE,
  SPECIAL_SHADE_MENU,
  SPECIAL_QUICK_SHADE_MODE,       ///< This must be the first of the "effects" family
  SPECIAL_QUICK_SHADE_MENU,
  SPECIAL_STENCIL_MODE,
  SPECIAL_STENCIL_MENU,
  SPECIAL_MASK_MODE,
  SPECIAL_MASK_MENU,
  SPECIAL_GRID_MODE,
  SPECIAL_GRID_MENU,
  SPECIAL_SIEVE_MODE,
  SPECIAL_SIEVE_MENU,
  SPECIAL_COLORIZE_MODE,
  SPECIAL_COLORIZE_MENU,
  SPECIAL_SMOOTH_MODE,
  SPECIAL_SMOOTH_MENU,
  SPECIAL_SMEAR_MODE,
  SPECIAL_EFFECTS_OFF,
  SPECIAL_TILING_MODE,
  SPECIAL_TRANSPARENCY_1,
  SPECIAL_TRANSPARENCY_2,
  SPECIAL_TRANSPARENCY_3,
  SPECIAL_TRANSPARENCY_4,
  SPECIAL_TRANSPARENCY_5,
  SPECIAL_TRANSPARENCY_6,
  SPECIAL_TRANSPARENCY_7,
  SPECIAL_TRANSPARENCY_8,
  SPECIAL_TRANSPARENCY_9,
  SPECIAL_TRANSPARENCY_0,
  SPECIAL_TILING_MENU,            ///< This must be the last of the "effects" family
  SPECIAL_ZOOM_1,
  SPECIAL_ZOOM_2,
  SPECIAL_ZOOM_3,
  SPECIAL_ZOOM_4,
  SPECIAL_ZOOM_5,
  SPECIAL_ZOOM_6,
  SPECIAL_ZOOM_8,
  SPECIAL_ZOOM_10,
  SPECIAL_ZOOM_12,
  SPECIAL_ZOOM_14,
  SPECIAL_ZOOM_16,
  SPECIAL_ZOOM_18,
  SPECIAL_ZOOM_20,
  SPECIAL_SHOW_GRID,
  SPECIAL_LAYER1_SELECT,
  SPECIAL_LAYER1_TOGGLE,
  SPECIAL_LAYER2_SELECT,
  SPECIAL_LAYER2_TOGGLE,
  SPECIAL_LAYER3_SELECT,
  SPECIAL_LAYER3_TOGGLE,
  SPECIAL_LAYER4_SELECT,
  SPECIAL_LAYER4_TOGGLE,
  SPECIAL_LAYER5_SELECT,
  SPECIAL_LAYER5_TOGGLE,
  SPECIAL_LAYER6_SELECT,
  SPECIAL_LAYER6_TOGGLE,
  SPECIAL_LAYER7_SELECT,
  SPECIAL_LAYER7_TOGGLE,
  SPECIAL_LAYER8_SELECT,
  SPECIAL_LAYER8_TOGGLE,
  SPECIAL_LAYER_ADD,
  SPECIAL_LAYER_DELETE,
  SPECIAL_LAYER_MERGE,
  SPECIAL_LAYER_SWAP_UP,
  SPECIAL_LAYER_SWAP_DOWN,
  SPECIAL_LAYER_MENU,
  NB_SPECIAL_SHORTCUTS            ///< Number of special shortcuts
};

/// Identifiers of the operations, ie tools you use on the image.
enum OPERATIONS
{
  OPERATION_CONTINUOUS_DRAW=0, ///< Freehand continuous draw
  OPERATION_DISCONTINUOUS_DRAW,///< Freehand discontinuous draw
  OPERATION_POINT_DRAW,        ///< Freehand point-by-point draw
  OPERATION_FILLED_CONTOUR,    ///< Filled contour
  OPERATION_LINE,              ///< Lines
  OPERATION_K_LIGNE,           ///< Linked lines
  OPERATION_CENTERED_LINES,    ///< Centered lines
  OPERATION_EMPTY_RECTANGLE,   ///< Empty rectangle
  OPERATION_FILLED_RECTANGLE,  ///< Filled rectangle
  OPERATION_EMPTY_CIRCLE,      ///< Empty circle
  OPERATION_FILLED_CIRCLE,     ///< Filled circle
  OPERATION_EMPTY_ELLIPSE,     ///< Empty ellipse
  OPERATION_FILLED_ELLIPSE,    ///< Filled ellipse
  OPERATION_FILL,              ///< Fill
  OPERATION_REPLACE,           ///< Color replacer
  OPERATION_GRAB_BRUSH,        ///< Rectangular brush grabbing
  OPERATION_POLYBRUSH,         ///< Polygonal brush grabbing
  OPERATION_COLORPICK,         ///< Colorpicker
  OPERATION_MAGNIFY,           ///< Position the magnify window
  OPERATION_3_POINTS_CURVE,    ///< Curve with 3 control points
  OPERATION_4_POINTS_CURVE,    ///< Curve with 4 control points
  OPERATION_AIRBRUSH,          ///< Airbrush
  OPERATION_POLYGON,           ///< Polygon
  OPERATION_POLYFORM,          ///< Polyform
  OPERATION_POLYFILL,          ///< Filled polygon
  OPERATION_FILLED_POLYFORM,   ///< Filled polyform
  OPERATION_SCROLL,            ///< Scroll (pan)
  OPERATION_GRAD_CIRCLE,       ///< Gradient-filled circle
  OPERATION_GRAD_ELLIPSE,      ///< Gradient-filled ellipse
  OPERATION_ROTATE_BRUSH,      ///< Rotate brush
  OPERATION_STRETCH_BRUSH,     ///< Stretch brush
  OPERATION_DISTORT_BRUSH,     ///< Distort brush
  OPERATION_GRAD_RECTANGLE,    ///< Gradient-filled rectangle
  NB_OPERATIONS                ///< Number of operations handled by the engine
};

#endif
