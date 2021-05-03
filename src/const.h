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
///@file const.h
/// Constants (preprocessor defines) and enumerations used anywhere.
//////////////////////////////////////////////////////////////////////////////

#ifndef _CONST_H_
#define _CONST_H_

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676656 ///< Hmm, pie...
#endif

#define VERSION1                  2     ///< Version number for gfx2.cfg (1/4)
#define VERSION2                  0     ///< Version number for gfx2.cfg (2/4)
#define BETA1                     98    ///< Version number for gfx2.cfg (3/4)
#define BETA2                     0     ///< Version number for gfx2.cfg (4/4)
#define MAX_VIDEO_MODES           200   ///< Maximum number of video modes Grafx2 can propose.
#define NB_ZOOM_FACTORS           15    ///< Number of zoom levels available in the magnifier.
#define MENU_WIDTH                254   ///< Width of the menu (not counting the palette)
#define MENU_HEIGHT               44    ///< Height of the menu.
#define NB_CURSOR_SPRITES         9     ///< Number of available mouse cursor sprites.
#define CURSOR_SPRITE_WIDTH       16    ///< Width of a mouse cursor sprite.
#define CURSOR_SPRITE_HEIGHT      16    ///< Height of a mouse cursor sprite.
#define MENU_SPRITE_WIDTH         16    ///< Width of a menu sprite in pixels
#define MENU_SPRITE_HEIGHT        16    ///< Height of a menu sprite in pixels
#define EFFECT_SPRITE_WIDTH       14    ///< Width of an effect sprite in pixels
#define EFFECT_SPRITE_HEIGHT      14    ///< Height of an effect sprite in pixels
#define LAYER_SPRITE_WIDTH        14    ///< Width of a layer button in pixels
#define LAYER_SPRITE_HEIGHT       10    ///< Height of a layer button in pixels
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
#define MAX_PATH_CHARACTERS PATH_MAX    ///< Number of characters for a file+complete path. Adapt to your OS...
#define NB_BOOKMARKS               4    ///< Number of bookmark buttons in Save/Load screen.
// Character to show a right arrow, used when editing long strings. It's present in ::Gfx->System_font 
#define RIGHT_TRIANGLE_CHARACTER  16
// Character to show a left arrow, used when editing long strings. It's present in ::Gfx->System_font 
#define LEFT_TRIANGLE_CHARACTER   17
/// Character to display in menus for an ellipsis.
#define ELLIPSIS_CHARACTER       '\x85' // 3 dots : …  / HTML entity : &hellip;
#define NB_LAYERS                  1    ///< Initial number of layers for a new image
#define MAX_NB_FRAMES            999    ///< Maximum number of frames that can be used in a grafx2 animation.
#define MAX_NB_LAYERS             16    ///< Maximum number of layers that can be used in grafx2. Note that 32 is upper limit because of a few bit fields.
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
#define INITIAL_SEPARATOR_PROPORTION 0.3f  ///< Proportion of the normal view width, relative to the whole screen width.
#define NB_ZOOMED_PIXELS_MIN         4     ///< Minimal number of pixel shown (in width) by the zoomed view. (Note: below 4, you can't scroll!)

#if defined(__MORPHOS__) || defined(__amigaos4__) || defined(__amigaos__) || defined(__AROS__)
   #define PARENT_DIR "/"
#else
   /// Filename that means "parent directory" for your operating system.
   #define PARENT_DIR ".."
#endif

/// List of file formats recognized by grafx2
enum FILE_FORMATS
{
  FORMAT_ALL_IMAGES=0, ///< This is not really a file format, it's reserverd for a compilation of all image file extensions
  FORMAT_ALL_PALETTES=1, ///< This is not really a file format, it's reserverd for a compilation of all palette file extensions
  FORMAT_ALL_FILES=2, ///< This is not really a file format, it's reserverd for the "*.*" filter option.
  FORMAT_PNG,  ///< Portable Network Graphics
  FORMAT_GIF,  ///< Graphics Interchange Format
  FORMAT_BMP,  ///< Windows or OS/2 bitmap
  FORMAT_PCX,  ///< Paintbrush
  FORMAT_PKM,  ///< GrafX2 own format (Karl Maritaud)
  FORMAT_LBM,  ///< IFF ILBM : InterLeaved BitMap
  FORMAT_PBM,  ///< IFF PBM : Packet BitMap
  FORMAT_ACBM, ///< IFF ACBM : Amiga Contiguous BitMap
  FORMAT_IMG,  ///< from PV (c)W.Wiedmann
  FORMAT_SCx,  ///< ColoRIX
  FORMAT_PI1,  ///< Atari ST Degas
  FORMAT_PC1,  ///< Atari ST Degas Elite
  FORMAT_CA1,  ///< Atari ST CrackArt
  FORMAT_CEL,  ///< Atari ST Cyber Paint Cell
  FORMAT_NEO,  ///< Atari ST NeoChrome
  FORMAT_TNY,  ///< Atari ST Tiny Stuff
  FORMAT_C64,  ///< Several C64 formats : Koala, FLI, BML, etc.
  FORMAT_PRG,  ///< C64 autoload picture
  FORMAT_GPX,  ///< pixcen C64 format : .gpx
  FORMAT_KCF,  ///< KiSS Color File
  FORMAT_PAL,  ///< raw 6bit palette or Jasc Paint Shop Pro palette
  FORMAT_GPL,  ///< Gimp palette
  FORMAT_SCR,  ///< Amstrad CPC
  FORMAT_CM5,  ///< Amstrad CPC Mode 5
  FORMAT_PPH,  ///< Amstrad CPC Perfect Pix
  FORMAT_GOS,  ///< Amstrad Plus Graphos
  FORMAT_SGX,  ///< SymbOS graphic files
  FORMAT_XPM,  ///< X PixMap
  FORMAT_ICO,  ///< Windows icons
  FORMAT_INFO, ///< Amiga OS icons
  FORMAT_FLI,  ///< Autodesk Animator FLI/FLC
  FORMAT_MOTO, ///< Thomson MO/TO computers pictures
  FORMAT_HGR,  ///< Apple II HGR and DHGR
  FORMAT_2GS,  ///< Apple II GS format
  FORMAT_TIFF, ///< Tagged Image File Format
  FORMAT_GRB,  ///< HP-48 Grob
  FORMAT_MSX,  ///< MSX formats
  FORMAT_MISC, ///< Must be last of enum: others formats recognized by SDL_image (or recoil)
  FORMAT_CLIPBOARD  ///< To load/save from/to Clipboard
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
  ERROR_FORBIDDEN_SIZE,    ///< Image size requested is not supported
  ERROR_SAVING_CFG,        ///< Error while writing gfx2.cfg
  ERROR_MISSING_DIRECTORY, ///< Unable to return to the original "current directory" on program exit
  ERROR_INI_CORRUPTED,     ///< File gfx2.ini couldn't be parsed
  ERROR_SAVING_INI,        ///< Error while writing gfx2.ini
  ERROR_INIT,              ///< Program initialization error
  ERROR_SORRY_SORRY_SORRY  ///< (Page allocation error that shouldn't ever happen, now)
};

/// Available pixel scalers
enum PIXEL_RATIO
{
    PIXEL_SIMPLE=0, ///< Use real pixels
    PIXEL_WIDE,     ///< Use wide pixels (2x1) like on Amstrad CPC mode 0
    PIXEL_TALL,     ///< Use tall pixels (1x2) like on Amstrad CPC mode 2
    PIXEL_DOUBLE,   ///< Use big pixels (2x2) if your LCD screen can't do lowres by itself
    PIXEL_TRIPLE,   ///< Use really big pixels (3x3)
    PIXEL_WIDE2,    ///< Use big wide pixels (4x2)
    PIXEL_TALL2,    ///< Use big tall pixels (2x4)
    PIXEL_TALL3,    ///< Use big tall pixels (3x4)
    PIXEL_QUAD,     ///< Use really giant pixels (4x4). You need to have a screen resolution at least 1280x800 to use this one
    PIXEL_MAX       ///< Number of elements in enum
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
  // Sprite based cursors first (also used as index in cursor sprite array)
  CURSOR_SHAPE_ARROW,
  CURSOR_SHAPE_TARGET,           ///< This one uses the paintbrush
  CURSOR_SHAPE_COLORPICKER,
  CURSOR_SHAPE_HOURGLASS,
  CURSOR_SHAPE_MULTIDIRECTIONAL,
  CURSOR_SHAPE_HORIZONTAL,
  CURSOR_SHAPE_THIN_TARGET,      ///< This one uses the paintbrush
  CURSOR_SHAPE_THIN_COLORPICKER, ///< This one uses the paintbrush
  CURSOR_SHAPE_BUCKET,

  // XOR/runtime-generated cursors last
  CURSOR_SHAPE_XOR_TARGET,
  CURSOR_SHAPE_XOR_RECTANGLE,
  CURSOR_SHAPE_XOR_ROTATION,
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
  PAINTBRUSH_SHAPE_RESERVED1,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED2,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED3,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED4,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED5,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED6,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED7,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_RESERVED8,   ///< Reserved for future use
  PAINTBRUSH_SHAPE_MISC,        ///< A raw monochrome bitmap, can't be resized. This must be the last of the preset paintbrush types.
  PAINTBRUSH_SHAPE_POINT,       ///< Used to reduce the paintbrush to a single pixel, during operations like floodfill.
  PAINTBRUSH_SHAPE_NONE,        ///< Used to display no cursor at all (colorpicker)
  PAINTBRUSH_SHAPE_COLOR_BRUSH, ///< User's brush, in color mode
  PAINTBRUSH_SHAPE_MONO_BRUSH,  ///< User's brush, in mono mode
  PAINTBRUSH_SHAPE_MAX          ///< Upper limit.
};

/// Normal resting state for a menu button.
#define BUTTON_RELEASED 0
/// State of a menu button that is being pressed.
#define BUTTON_PRESSED  1
/// State of a button temporarily highligted
#define BUTTON_HIGHLIGHTED 2

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
  CHUNK_GRID            = 9, ///< Grid settings
  CHUNK_BRUSH           =10, ///< Paintbrushes
  CHUNK_SCRIPTS         =11, ///< Callable scripts
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
  NB_ICON_SPRITES,   ///< Number of 8x8 icons
  ICON_NONE          ///< None of the above
};

enum EFFECT_SPRITES
{
  EFFECTS_SPRITE_SHADE,
  EFFECTS_SPRITE_TRANSP,
  EFFECTS_SPRITE_SMOOTH,
  EFFECTS_SPRITE_TILING,
  EFFECTS_SPRITE_STENCIL,
  EFFECTS_SPRITE_SIEVE,
  EFFECTS_SPRITE_GRID,
  EFFECTS_SPRITE_MASK,
  EFFECTS_SPRITE_SMEAR,
  EFFECTS_SPRITE_8BIT,
  NB_EFFECTS_SPRITES ///< Number of effect sprites.
};

/// Identifiers for the buttons in the menu.
enum BUTTON_NUMBERS
{
  // Status bar
  BUTTON_HIDE = 0,

  // Anim bar
  BUTTON_LAYER_MENU2,
  BUTTON_ANIM_TIME,
  BUTTON_ANIM_FIRST_FRAME,
  BUTTON_ANIM_PREV_FRAME,
  BUTTON_ANIM_NEXT_FRAME,
  BUTTON_ANIM_LAST_FRAME,
  BUTTON_ANIM_ADD_FRAME,
  BUTTON_ANIM_REMOVE_FRAME,
  BUTTON_ANIM_UP_FRAME,
  BUTTON_ANIM_DOWN_FRAME,
  
  BUTTON_ANIM_PLAY, // unused at this time

  // Layer bar
  BUTTON_LAYER_MENU,
  BUTTON_LAYER_COLOR,
  BUTTON_LAYER_MERGE,
  BUTTON_LAYER_ADD,
  BUTTON_LAYER_REMOVE,
  BUTTON_LAYER_UP,
  BUTTON_LAYER_DOWN,
  BUTTON_LAYER_SELECT,

  // Main menu
  BUTTON_PAINTBRUSHES,
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
  NB_BUTTONS            ///< Number of buttons in the menu bar.
};

enum MENU_SPRITE
{
  MENU_SPRITE_COLOR_BRUSH=0,
  MENU_SPRITE_MONO_BRUSH,
  MENU_SPRITE_DISCONTINUOUS_DRAW,
  MENU_SPRITE_POINT_DRAW,
  MENU_SPRITE_CONTOUR_DRAW,
  MENU_SPRITE_4_POINTS_CURVE,
  MENU_SPRITE_K_LINE,
  MENU_SPRITE_CENTERED_LINES,
  MENU_SPRITE_ELLIPSES,
  MENU_SPRITE_POLYFORM,
  MENU_SPRITE_REPLACE,
  MENU_SPRITE_GRAD_ELLIPSE,
  MENU_SPRITE_VERTICAL_PALETTE_SCROLL,
  NB_MENU_SPRITES ///< Number of menu sprites.
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
  SPECIAL_BRUSH_DOUBLE,
  SPECIAL_BRUSH_DOUBLE_WIDTH,
  SPECIAL_BRUSH_DOUBLE_HEIGHT,
  SPECIAL_BRUSH_HALVE,
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
  SPECIAL_TILEMAP_MODE,
  SPECIAL_TILEMAP_MENU,
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
  SPECIAL_REPEAT_SCRIPT,
  SPECIAL_RUN_SCRIPT_1,
  SPECIAL_RUN_SCRIPT_2,
  SPECIAL_RUN_SCRIPT_3,
  SPECIAL_RUN_SCRIPT_4,
  SPECIAL_RUN_SCRIPT_5,
  SPECIAL_RUN_SCRIPT_6,
  SPECIAL_RUN_SCRIPT_7,
  SPECIAL_RUN_SCRIPT_8,
  SPECIAL_RUN_SCRIPT_9,
  SPECIAL_RUN_SCRIPT_10,
  SPECIAL_CYCLE_MODE,

  SPECIAL_FORMAT_CHECKER,
  SPECIAL_FORMAT_CHECKER_MENU,

  SPECIAL_HOLD_PAN,
  SPECIAL_ZOOM_IN_MORE,
  SPECIAL_ZOOM_OUT_MORE,
  
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
  OPERATION_K_LINE,            ///< Linked lines
  OPERATION_CENTERED_LINES,    ///< Centered lines
  OPERATION_EMPTY_RECTANGLE,   ///< Empty rectangle
  OPERATION_FILLED_RECTANGLE,  ///< Filled rectangle
  OPERATION_EMPTY_CIRCLE_CTR,  ///< Empty circle (center radius)
  OPERATION_EMPTY_CIRCLE_CRN,  ///< Empty circle (corners)
  OPERATION_EMPTY_ELLIPSE_CTR, ///< Empty ellipse (center radius)
  OPERATION_EMPTY_ELLIPSE_CRN, ///< Empty ellipse (corners)
  OPERATION_FILLED_CIRCLE_CTR, ///< Filled circle (center radius)
  OPERATION_FILLED_CIRCLE_CRN, ///< Filled circle (corners)
  OPERATION_FILLED_ELLIPSE_CTR,///< Filled ellipse (center radius)
  OPERATION_FILLED_ELLIPSE_CRN,///< Filled ellipse (corners)
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
  OPERATION_GRAD_CIRCLE_CTR,   ///< Gradient-filled circle (center radius)
  OPERATION_GRAD_CIRCLE_CRN,   ///< Gradient-filled circle (corners)
  OPERATION_GRAD_ELLIPSE_CTR,  ///< Gradient-filled ellipse (center radius)
  OPERATION_GRAD_ELLIPSE_CRN,  ///< Gradient-filled ellipse (corners)
  OPERATION_ROTATE_BRUSH,      ///< Rotate brush
  OPERATION_STRETCH_BRUSH,     ///< Stretch brush
  OPERATION_DISTORT_BRUSH,     ///< Distort brush
  OPERATION_GRAD_RECTANGLE,    ///< Gradient-filled rectangle
  OPERATION_RMB_COLORPICK,     ///< Colorpick on right mouse button
  OPERATION_PAN_VIEW,          ///< Pan view
  NB_OPERATIONS                ///< Number of operations handled by the engine
};

/// Image drawing modes.
enum IMAGE_MODES
{
  IMAGE_MODE_LAYERED=0, ///< Layered image
  IMAGE_MODE_ANIMATION, ///< Animation
  IMAGE_MODE_ZX,        ///< ZX Spectrum (note "SPECTRUM" is kept for later... Spectrum 512 anyone?)
  IMAGE_MODE_GBC,       ///< Game Boy Color
  IMAGE_MODE_THOMSON,   ///< "40 columns" mode on Thomson machines
  IMAGE_MODE_EGX,       ///< CPC EGX
  IMAGE_MODE_EGX2,      ///< CPC EGX2
  IMAGE_MODE_MODE5,     ///< CPC mode 5
  IMAGE_MODE_RASTER,    ///< CPC generic rasters
  IMAGE_MODE_C64HIRES,  ///< C64 HiRes
  IMAGE_MODE_C64MULTI,  ///< C64 Multicolor
  IMAGE_MODE_C64FLI,    ///< C64 Flexible Line Interpretation
  IMAGE_MODE_HGR,       ///< Apple 2 HGR
  IMAGE_MODE_DHGR,      ///< Apple 2 DHGR
  IMAGE_MODE_MEGADRIVE, ///< Sega Mega Drive / Genesis
  IMAGE_MODE_TMS9918G2, ///< TMS9918 Mode 2 (MSX, ColecoVision, SC-3000, etc.)
};

/// Circle / Ellipse Modes
#define MASK_CIRCLE_ELLIPSE (2)
#define MASK_CENTER_CORNERS (1)
#define MODE_CIRCLE    (0)
#define MODE_ELLIPSE   (2)
#define MODE_CENTER    (0)
#define MODE_CORNERS   (1)
#define MODE_CIRCLE_CTR  (MODE_CIRCLE|MODE_CENTER)
#define MODE_CIRCLE_CRN  (MODE_CIRCLE|MODE_CORNERS)
#define MODE_ELLIPSE_CTR (MODE_ELLIPSE|MODE_CENTER)
#define MODE_ELLIPSE_CRN (MODE_ELLIPSE|MODE_CORNERS)

enum FSOBJECT_TYPE
{
  FSOBJECT_FILE = 0,
  FSOBJECT_DIR = 1,
  FSOBJECT_DRIVE = 2
};

enum GFX2_WINDOW_STATES
{
  GFX2_WINDOW_STANDARD = 0,
  GFX2_WINDOW_MAXIMIZED,
  GFX2_WINDOW_MINIMIZED
};

#endif
