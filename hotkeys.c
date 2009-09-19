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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
#include "struct.h"
#include "global.h"
#include "hotkeys.h"

#ifdef __VBCC__
  #define false 0
  #define true 1
#endif

T_Key_config ConfigKey[NB_SHORTCUTS] = {
  {0,
  "Scroll up",
  "Scrolls the picture up, both in",
  "magnify and normal mode.",
  "",
  false,
  SDLK_UP, // HAUT
  0},
  {1,
  "Scroll down",
  "Scrolls the picture down, both in",
  "magnify and normal mode.",
  "",
  false,
  SDLK_DOWN, // BAS
  0},
  {2,
  "Scroll left",
  "Scrolls the picture to the left,",
  "both in magnify and normal mode.",
  "",
  false,
  SDLK_LEFT, // GAUCHE
  0},
  {3,
  "Scroll right",
  "Scrolls the picture to the right,",
  "both in magnify and normal mode.",
  "",
  false,
  SDLK_RIGHT, // DROITE
  0},
  {4,
  "Faster scroll up",
  "Used to scroll upwards in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  SDLK_UP|MOD_SHIFT, // Shift + Haut
  0},
  {5,
  "Faster scroll down",
  "Used to scroll downwards in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  SDLK_DOWN|MOD_SHIFT, // Shift + Bas
  0},
  {6,
  "Faster scroll left",
  "Used to scroll to the left in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  SDLK_LEFT|MOD_SHIFT, // Shift + Gauche
  0},
  {7,
  "Faster scroll right",
  "Used to scroll to the right in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  SDLK_RIGHT|MOD_SHIFT, // Shift + Droite
  0},
  {8,
  "Slower scroll up",
  "Used to scroll upwards in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  SDLK_UP|MOD_ALT, // Alt + Haut
  0},
  {9,
  "Slower scroll down",
  "Used to scroll downwards in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  SDLK_DOWN|MOD_ALT, // Alt + Bas
  0},
  {10,
  "Slower scroll left",
  "Used to scroll to the left in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  SDLK_LEFT|MOD_ALT, // Alt + Gauche
  0},
  {11,
  "Slower scroll right",
  "Used to scroll to the right in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  SDLK_RIGHT|MOD_ALT, // Alt + Droite
  0},
  {12,
  "Move mouse cursor 1 pixel up",
  "Used to simulate a very small mouse",
  "deplacement up.It""s very useful",
  "when you want ultra-high precision.",
  true,
  SDLK_UP|MOD_CTRL, // Ctrl + Haut
  0},
  {13,
  "Move mouse cursor 1 pixel down",
  "Used to simulate a very small mouse",
  "deplacement down.It""s very useful",
  "when you want ultra-high precision.",
  true,
  SDLK_DOWN|MOD_CTRL, // Ctrl + Bas
  0},
  {14,
  "Move mouse cursor 1 pixel left",
  "Used to simulate a very small mouse",
  "deplacement left.It""s very useful",
  "when you want ultra-high precision.",
  true,
  SDLK_LEFT|MOD_CTRL, // Ctrl + Gauche
  0},
  {15,
  "Move mouse cursor 1 pixel right",
  "Used to simulate a very small mouse",
  "deplacement right.It""s very useful",
  "when you want ultra-high precision.",
  true,
  SDLK_RIGHT|MOD_CTRL, // Ctrl + Droite
  0},
  {16,
  "Simulate left mouse click",
  "Used to simulate a click with the",
  "left mouse button. It""s useful",
  "when you want ultra-high precision.",
  true,
  SDLK_SPACE, // Space
  0},
  {17,
  "Simulate right mouse click",
  "Used to simulate a click with the",
  "right mouse button.. It""s useful",
  "when you want ultra-high precision.",
  true,
  SDLK_SPACE|MOD_SHIFT, // Shift + Space
  0},
  {18,
  "Show/hide option menu",
  "Switch the tool bar display on/off.",
  "This hot-key cannot be removed.",
  "",
  false,
  SDLK_F10, // F10
  0},
  {19,
  "Show/hide cursor",
  "Switch the cursor display on/off.",
  "This only works on the \"small cross\"",
  "and \"hand\" cursors.",
  true,
  SDLK_F9, // F9
  0},
  {20,
  "Set paintbrush to 1 pixel",
  "Useful when you want to use a",
  "\"single-pixel-brush\".",
  "",
  true,
  SDLK_DELETE, // Del
  0},
  {21,
  "Paintbrush choice",
  "Opens a menu where you can choose a",
  "paintbrush out of 24 predefined",
  "ones.",
  true,
  SDLK_F4, // F4
  0},
  {22,
  "Monochrome brush",
  "Turn your current user-defined brush",
  "into a single colored one. All non-",
  "transparent colors are set to FG.",
  true,
  SDLK_F4|MOD_SHIFT, // Shift + F4
  0},
  {23,
  "Freehand drawing",
  "Set the drawing mode to the",
  "classical freehand one.",
  "",
  true,
  SDLK_d, // D
  0},
  {24,
  "Switch freehand drawing mode",
  "Alternates between: continuous,",
  "discontinuous, point by point,",
  "and contour fill",
  true,
  SDLK_d|MOD_SHIFT, // Shift + D
  0},
  {25,
  "Continuous freehand drawing",
  "Switch directly to continuous",
  "freehand drawing mode.",
  "",
  true,
  SDLK_d|MOD_CTRL, // Ctrl + D
  0},
  {26,
  "Line",
  "Allows you to draw lines.",
  "",
  "",
  true,
  SDLK_l, // L
  0},
  {27,
  "Knotted lines",
  "Allows you to draw linked lines.",
  "This mode can also be called",
  "\"Polyline\".",
  true,
  SDLK_l|MOD_SHIFT, // Shift + L
  0},
  {28,
  "Spray",
  "Allows you to spray brushes",
  "randomly in the picture.",
  "",
  true,
  SDLK_a, // A (Q en AZERTY)
  0},
  {29,
  "Spray menu",
  "Opens a menu in which you can",
  "configure the spray flow and size.",
  "",
  true,
  SDLK_a|MOD_SHIFT, // Shift + A
  0},
  {30,
  "Flood-fill",
  "Allows you to fill an area of the",
  "picture made of pixels of the same",
  "color.",
  true,
  SDLK_f, // F
  0},
  {124,
  "Replace color",
  "This tool replaces all the pixels of",
  "the clicked color to the fore-color",
  "or the back-color.",
  true,
  SDLK_f|MOD_SHIFT, // Shift + F
  0},
  {31,
  "Bezier""s curves",
  "Allows you to draw Bezier""s curves.",
  "",
  "",
  true,
  SDLK_i, // I
  0},
  {32,
  "Bezier""s curve with 3 or 4 points",
  "Allows you to choose whether you",
  "want to draw Bezier""s curves with",
  "3 or 4 points.",
  true,
  SDLK_i|MOD_SHIFT, // Shift + I
  0},
  {33,
  "Empty rectangle",
  "Allows you to draw a rectangle using",
  "the brush.",
  "",
  true,
  SDLK_r, // R
  0},
  {34,
  "Filled rectangle",
  "Allows you to draw a filled",
  "rectangle.",
  "",
  true,
  SDLK_r|MOD_SHIFT, // Shift + R
  0},
  {35,
  "Empty circle",
  "Allows you to draw a circle using",
  "the brush.",
  "",
  true,
  SDLK_c, // C
  0},
  {36,
  "Empty ellipse",
  "Allows you to draw an ellipse using",
  "the brush.",
  "",
  true,
  SDLK_c|MOD_CTRL, // Ctrl + C
  0},
  {37,
  "Filled circle",
  "Allows you to draw a filled circle.",
  "",
  "",
  true,
  SDLK_c|MOD_SHIFT, // Shift + C
  0},
  {38,
  "Filled ellipse",
  "Allows you to draw a filled ellipse.",
  "",
  "",
  true,
  SDLK_c|MOD_SHIFT|MOD_CTRL, // Shift + Ctrl + C
  0},
  {39,
  "Empty polygon",
  "Allows you to draw a polygon using",
  "the brush.",
  "",
  true,
  SDLK_n, // N
  0},
  {40,
  "Empty \"polyform\"",
  "Allows you to draw a freehand",
  "polygon using the brush.",
  "",
  true,
  SDLK_n|MOD_CTRL, // Ctrl + N
  0},
  {41,
  "Filled polygon",
  "Allows you to draw a filled polygon.",
  "",
  "",
  true,
  SDLK_n|MOD_SHIFT, // Shift + N
  0},
  {42,
  "Filled \"polyform\"",
  "Allows you to draw a filled freehand",
  "polygon.",
  "",
  true,
  SDLK_n|MOD_SHIFT|MOD_CTRL, // Shift + Ctrl + N
  0},
  {43,
  "Rectangle with gradation",
  "Allows you to draw a rectangle with",
  "a color gradation.",
  "",
  true,
  SDLK_r|MOD_ALT, // Alt + R
  0},
  {44,
  "Gradation menu",
  "Allows you to configure the way",
  "color gradations are calculated.",
  "",
  true,
  SDLK_g|MOD_ALT, // Alt + G
  0},
  {45,
  "Sphere with gradation",
  "Allows you to draw a rectangle with",
  "a color gradation.",
  "",
  true,
  SDLK_c|MOD_ALT, // Alt + C
  0},
  {46,
  "Ellipse with gradation",
  "Allows you to draw an ellipse filled",
  "with a color gradation.",
  "",
  true,
  SDLK_c|MOD_SHIFT|MOD_ALT, // Shift + Alt + C
  0},
  {47,
  "Adjust picture",
  "Allows you to move the whole picture",
  "Around. What gets out from a side",
  "reappears on the other.",
  true,
  SDLK_KP5, // Kpad5
  0},
  {48,
  "Picture effects",
  "Opens the 'Picture effects' window.",
  "",
  "",
  true,
  SDLK_KP5|MOD_SHIFT, // Shift + Kpad5
  0},
  {49,
  "Drawing effects",
  "Opens a menu where you can enable/",
  "disable and configure the drawing",
  "effects.",
  true,
  SDLK_e, // E
  0},
  {50,
  "Shade mode",
  "Enables or disables Shade mode",
  "",
  "",
  true,
  SDLK_F5, // F5
  0},
  {51,
  "Shade menu",
  "Opens a the menu for Shade settings.",
  "",
  "",
  true,
  SDLK_F5|MOD_SHIFT, // Shift + F5
  0},
  {131,
  "Quick-shade mode",
  "Enables or disables Quick-shade",
  "mode.",
  "",
  true,
  SDLK_F5|MOD_CTRL, // Ctrl + F5
  0},
  {132,
  "Quick-shade menu",
  "Opens a the menu for Quick-shade",
  "settings.",
  "",
  true,
  SDLK_F5|MOD_SHIFT|MOD_CTRL, // Shift + Ctrl + F5
  0},
  {52,
  "Stencil mode",
  "Enables or disables Stencil mode.",
  "",
  "",
  true,
  SDLK_F6, // F6
  0},
  {53,
  "Stencil menu",
  "Opens a the menu for Stencil",
  "settings.",
  "",
  true,
  SDLK_F6|MOD_SHIFT, // Shift + F6
  0},
  {54,
  "Mask mode",
  "Enables or disables Mask mode.",
  "",
  "",
  true,
  SDLK_F6|MOD_ALT, // Alt + F6
  0},
  {55,
  "Mask menu",
  "Opens a the menu for Mask settings.",
  "",
  "",
  true,
  SDLK_F6|MOD_SHIFT|MOD_ALT, // Shift + Alt + F6
  0},
  {56,
  "Grid mode",
  "Enables or disables the Grid mode.",
  "",
  "",
  true,
  SDLK_g, // G
  0},
  {57,
  "Grid menu",
  "Open a menu where you can configure",
  "the grid used by Grid mode.",
  "",
  true,
  SDLK_g|MOD_SHIFT, // Shift + G
  0},
  {58,
  "Sieve mode",
  "Enables or disables the Sieve mode.",
  "",
  "",
  true,
  SDLK_g|MOD_CTRL, // Ctrl + G
  0},
  {59,
  "Sieve menu",
  "Opens a menu where you can configure",
  "the sieve.",
  "",
  true,
  SDLK_g|MOD_SHIFT|MOD_CTRL, // Shift + Ctrl + G
  0},
  {60,
  "Invert sieve",
  "Inverts the pattern defined in the",
  "Sieve menu.",
  "",
  true,
  SDLK_g|MOD_CTRL|MOD_ALT, // Ctrl + Alt + G
  0},
  {61,
  "Colorize mode",
  "Enables or disables the Colorize",
  "mode.",
  "",
  true,
  SDLK_F7, // F7
  0},
  {62,
  "Colorize menu",
  "Opens a menu where you can give the",
  "opacity percentage for Colorize",
  "mode.",
  true,
  SDLK_F7|MOD_SHIFT, // Shift + F7
  0},
  {63,
  "Smooth mode",
  "Enables or disables the Smooth",
  "mode.",
  "",
  true,
  SDLK_F8, // F8
  0},
  {123,
  "Smooth menu",
  "Opens a menu where you can define",
  "the Smooth matrix.",
  "",
  true,
  SDLK_F8|MOD_SHIFT, // Shift + F8
  0},
  {64,
  "Smear mode",
  "Enables or disables the Smear mode.",
  "",
  "",
  true,
  SDLK_F8|MOD_ALT, // Alt + F8
  0},
  {65,
  "Tiling mode",
  "Enables or disables the Tiling",
  "mode.",
  "",
  true,
  SDLK_b|MOD_ALT, // Alt + B
  0},
  {66,
  "Tiling menu",
  "Opens a menu where you can configure",
  "the origin of the tiling.",
  "",
  true,
  SDLK_b|MOD_SHIFT|MOD_ALT, // Shift + Alt + B
  0},
  {67,
  "Classical brush grabbing",
  "Allows you to pick a brush defined",
  "within a rectangle.",
  "",
  true,
  SDLK_b, // B
  0},
  {68,
  "\"Lasso\" brush grabbing",
  "Allows you to pick a brush defined",
  "within a freehand polygon.",
  "",
  true,
  SDLK_b|MOD_CTRL, // Ctrl + B
  0},
  {69,
  "Get previous brush back",
  "Restore the last user-defined brush.",
  "",
  "",
  true,
  SDLK_b|MOD_SHIFT, // Shift + B
  0},
  {70,
  "Horizontal brush flipping",
  "Reverse brush horizontally.",
  "",
  "",
  true,
  SDLK_x, // X
  0},
  {71,
  "Vertical brush flipping",
  "Reverse brush vertically.",
  "",
  "",
  true,
  SDLK_y, // Y
  0},
  {72,
  "90° brush rotation",
  "Rotate the user-defined brush by 90°",
  "(counter-clockwise).",
  "",
  true,
  SDLK_z, // Z (W en AZERTY)
  0},
  {73,
  "180° brush rotation",
  "Rotate the user-defined brush by",
  "180°.",
  "",
  true,
  SDLK_z|MOD_SHIFT, // Shift + Z
  0},
  {74,
  "Strech brush",
  "Allows you to resize the",
  "user-defined brush.",
  "",
  true,
  SDLK_s, // S
  0},
  {75,
  "Distort brush",
  "Allows you to distort the",
  "user-defined brush.",
  "",
  true,
  SDLK_s|MOD_SHIFT, // Shift + S
  0},
  {76,
  "Outline brush",
  "Outlines the user-defined brush",
  "with the fore color.",
  "",
  true,
  SDLK_o, // O
  0},
  {77,
  "Nibble brush",
  "Deletes the borders of the",
  "user-defined brush.This does the",
  "opposite of the Outline option.",
  true,
  SDLK_o|MOD_SHIFT, // Shift + O
  0},
  {78,
  "Get colors from brush",
  "Copy colors of the spare page that",
  "are used in the brush.",
  "",
  true,
  SDLK_F11, // F11
  0},
  {79,
  "Recolorize brush",
  "Recolorize the user-defined brush in",
  "order to get a brush which looks as",
  "if it was grabbed in the spare page.",
  true,
  SDLK_F12, // F12
  0},
  {80,
  "Rotate by any angle",
  "Rotate the brush by an angle that",
  "you can define.",
  "",
  true,
  SDLK_w, // W (Z en AZERTY)
  0},
  {81,
  "Pipette",
  "Allows you to copy the color of a",
  "pixel in the picture into the",
  "foreground or background color.",
  true,
  SDLK_BACKQUOTE, // `~ (Key sous le Esc - ² en AZERTY)
  0},
  {82,
  "Swap foreground/background colors",
  "Invert foreground and background",
  "colors.",
  "",
  true,
  SDLK_BACKQUOTE|MOD_SHIFT, // Shift + `~
  0},
  {83,
  "Magnifier mode",
  "Allows you to zoom into the picture.",
  "",
  "",
  true,
  SDLK_m, // M (, ? sur AZERTY)
  KEY_MOUSEMIDDLE},
  {84,
  "Zoom factor menu",
  "Opens a menu where you can choose a",
  "magnifying factor.",
  "",
  true,
  SDLK_m|MOD_SHIFT, // Shift + M
  0},
  {85,
  "Zoom in",
  "Increase magnifying factor.",
  "",
  "",
  true,
  SDLK_KP_PLUS, // Grey +
  KEY_MOUSEWHEELUP},
  {86,
  "Zoom out",
  "Decrease magnifying factor.",
  "",
  "",
  true,
  SDLK_KP_MINUS, // Grey -
  KEY_MOUSEWHEELDOWN},
  {87,
  "Brush effects menu",
  "Opens a menu which proposes",
  "different effects on the",
  "user-defined brush.",
  true,
  SDLK_b|MOD_CTRL|MOD_ALT, // Ctrl + Alt + B
  0},
  {88,
  "Text",
  "Opens a menu which permits you to",
  "type in a character string and",
  "render it as a brush.",
  true,
  SDLK_t, // T
  0},
  {89,
  "Screen resolution menu",
  "Opens a menu where you can choose",
  "the screen resolution and image",
  "dimensions.",
  true,
  SDLK_RETURN, // Enter
  0},
  {90,
  "\"Safety\" resolution",
  "Resets the resolution to a 'safe'",
  "mode that should work everywhere:",
  "usually a 640x400 window.",
  false,
  SDLK_RETURN|MOD_SHIFT, // Shift + Enter
  0},
  {91,
  "Help and credits",
  "Opens a window where you can get",
  "information about the program,",
  "or contextual help.",
  true,
  SDLK_F1, // F1
  0},
  {92,
  "Statistics",
  "Displays miscellaneous more or less",
  "useful information.",
  "",
  true,
  SDLK_F1|MOD_SHIFT, // Shift + F1
  0},
  {93,
  "Jump to spare page",
  "Swap current page and spare page.",
  "",
  "",
  true,
  SDLK_TAB, // Tab
  0},
  {94,
  "Copy current page to spare page",
  "Copy current page to spare page.",
  "",
  "",
  true,
  SDLK_TAB|MOD_SHIFT, // Shift + Tab
  0},
  {95,
  "Save picture as...",
  "Opens a file-selector that allows",
  "you to save your picture with a new",
  "path-name.",
  true,
  SDLK_F2, // F2
  0},
  {96,
  "Save picture",
  "Saves your picture with the last",
  "name you gave it.",
  "",
  true,
  SDLK_F2|MOD_SHIFT, // Shift + F2
  0},
  {97,
  "Load picture",
  "Opens a file-selector that allows",
  "you to load a new picture.",
  "",
  true,
  SDLK_F3, // F3
  0},
  {98,
  "Re-load picture",
  "Re-load the current picture. This",
  "allows you to cancel modifications",
  "made since last saving.",
  true,
  SDLK_F3|MOD_SHIFT, // Shift + F3
  0},
  {99,
  "Save brush",
  "Opens a file-selector that allows",
  "you to save your current",
  "user-defined brush.",
  true,
  SDLK_F2|MOD_CTRL, // Ctrl + F2
  0},
  {100,
  "Load brush",
  "Opens a file-selector that allows",
  "you to load a brush.",
  "",
  true,
  SDLK_F3|MOD_CTRL, // Ctrl + F3
  0},
  {101,
  "Settings",
  "Opens a menu which permits you to",
  "modify some parameters of the",
  "program.",
  true,
  SDLK_F10|MOD_SHIFT, // Shift + F10
  0},
  {102,
  "Undo (Oops!)",
  "Cancel the last action which",
  "modified the picture.",
  "",
  true,
  SDLK_u, // U
  0},
  {103,
  "Redo",
  "Redo the last undone action.",
  "",
  "",
  true,
  SDLK_u|MOD_SHIFT, // Shift + U
  0},
  {133,
  "Kill",
  "Kills the current page. It actually",
  "removes the current page from the",
  "list of \"Undo\" pages.",
  true,
  SDLK_DELETE|MOD_SHIFT, // Shift + Suppr
  0},
  {104,
  "Clear page",
  "Clears the picture with the first",
  "color of the palette (usually black)",
  "",
  true,
  SDLK_BACKSPACE, // BackSpace
  0},
  {105,
  "Clear page with backcolor",
  "Clears the picture with the",
  "current backcolor.",
  "",
  true,
  SDLK_BACKSPACE|MOD_SHIFT, // Shift + BackSpace
  0},
  {106,
  "Quit program",
  "Allows you to leave the program.",
  "If modifications were not saved,",
  "confirmation is asked.",
  false,
  SDLK_q, // Q (A en AZERTY)
  0},
  {107,
  "Palette menu",
  "Opens a menu which allows you to",
  "modify the current palette.",
  "",
  true,
  SDLK_p, // P
  0},
  {125,
  "Secondary palette menu",
  "Opens a menu which allows you to",
  "define color series and some tagged",
  "colors.",
  true,
  SDLK_p|MOD_SHIFT, // Shift + P
  0},
  {130,
  "Exclude colors menu",
  "Opens a menu which allows you to",
  "define the colors you don""t want to",
  "use in Smooth and Transparency",
  true,
  SDLK_p|MOD_CTRL, // Ctrl + P
  0},
  {108,
  "Scroll palette to the left",
  "Scroll palette in the tool bar to",
  "the left, column by column.",
  "",
  true,
  SDLK_PAGEUP, // PgUp
  0},
  {109,
  "Scroll palette to the right",
  "Scroll palette in the tool bar to",
  "the right, column by column.",
  "",
  true,
  SDLK_PAGEDOWN, // PgDn
  0},
  {110,
  "Scroll palette to the left faster",
  "Scroll palette in the tool bar to",
  "the left, 8 columns by 8 columns.",
  "",
  true,
  SDLK_PAGEUP|MOD_SHIFT, // Shift + PgUp
  0},
  {111,
  "Scroll palette to the right faster",
  "Scroll palette in the tool bar to",
  "the right, 8 columns by 8 columns.",
  "",
  true,
  SDLK_PAGEDOWN|MOD_SHIFT, // Shift + PgDn
  0},
  {112,
  "Center brush attachment point",
  "Set the attachement of the",
  "user-defined brush to its center.",
  "",
  true,
  SDLK_KP5|MOD_CTRL, // Ctrl + 5 (pavé numérique)
  0},
  {113,
  "Top-left brush attachment point",
  "Set the attachement of the",
  "user-defined brush to its top-left",
  "corner.",
  true,
  SDLK_HOME|MOD_CTRL, // Ctrl + 7
  0},
  {114,
  "Top-right brush attachment point",
  "Set the attachement of the",
  "user-defined brush to its top-right",
  "corner.",
  true,
  SDLK_PAGEUP|MOD_CTRL, // Ctrl + 9
  0},
  {115,
  "Bottom-left brush attachment point",
  "Set the attachement of the",
  "user-defined brush to its",
  "bottom-left corner.",
  true,
  SDLK_END|MOD_CTRL, // Ctrl + 1
  0},
  {116,
  "Bottom-right brush attachment point",
  "Set the attachement of the",
  "user-defined brush to its",
  "bottom-right corner.",
  true,
  SDLK_PAGEDOWN|MOD_CTRL, // Ctrl + 3
  0},
  {117,
  "Next foreground color",
  "Set the foreground color to the next",
  "in the palette.",
  "",
  true,
  SDLK_RIGHTBRACKET, // ] (0x en AZERTY)
  0},
  {118,
  "Previous foreground color",
  "Set the foreground color to the",
  "previous in the palette.",
  "",
  true,
  SDLK_LEFTBRACKET, // [ (^ en AZERTY)
  0},
  {119,
  "Next background color",
  "Set the background color to the next",
  "in the palette.",
  "",
  true,
  SDLK_RIGHTBRACKET|MOD_SHIFT, // Shift + ]
  0},
  {120,
  "Previous background color",
  "Set the background color to the",
  "previous in the palette.",
  "",
  true,
  SDLK_LEFTBRACKET|MOD_SHIFT, // Shift + [
  0},
  {126,
  "Next user-defined forecolor",
  "Set the foreground color to the next",
  "in the user-defined color series.",
  "",
  true,
  SDLK_EQUALS, // "=+"
  0},
  {127,
  "Previous user-defined forecolor",
  "Set the foreground color to the",
  "previous in the user-defined color",
  "series.",
  true,
  SDLK_MINUS, // "-_" (")°" en AZERTY
  0},
  {128,
  "Next user-defined backcolor",
  "Set the background color to the next",
  "in the user-defined color series.",
  "",
  true,
  SDLK_EQUALS|MOD_SHIFT, // Shift + "=+"
  0},
  {129,
  "Previous user-defined backcolor",
  "Set the background color to the",
  "previous in the user-defined color",
  "series.",
  true,
  SDLK_MINUS|MOD_SHIFT, // Shift + "-_" (")°" en AZERTY
  0},
  {121,
  "Shrink paintbrush",
  "Decrease the width of the paintbrush",
  "if it is special circle or square.",
  "",
  true,
  SDLK_COMMA, // , < (;. en AZERTY)
  0},
  {122,
  "Enlarge paintbrush",
  "Increase the width of the paintbrush",
  "if it is special circle or square.",
  "",
  true,
  SDLK_PERIOD, // .> (:/ en AZERTY)
  0},
  {134,
  "Effects off",
  "Turns off all drawing effects. This",
  "is the same as the 'All off' button",
  "in the Effects screen",
  true,
  SDLK_e|MOD_SHIFT, // Shift-E
  0},
  {135,
  "Transparency 10%",
  "Turns transparency on and sets its",
  "opacity at 10%.",
  "",
  true,
  SDLK_1, // 1
  0},
  {136,
  "Transparency 20%",
  "Turns transparency on and sets its",
  "opacity at 20%.",
  "",
  true,
  SDLK_2, // 2
  0},
  {137,
  "Transparency 30%",
  "Turns transparency on and sets its",
  "opacity at 30%.",
  "",
  true,
  SDLK_3, // 3
  0},
  {138,
  "Transparency 40%",
  "Turns transparency on and sets its",
  "opacity at 40%.",
  "",
  true,
  SDLK_4, // 4
  0},
  {139,
  "Transparency 50%",
  "Turns transparency on and sets its",
  "opacity at 50%.",
  "",
  true,
  SDLK_5, // 5
  0},
  {140,
  "Transparency 60%",
  "Turns transparency on and sets its",
  "opacity at 60%.",
  "",
  true,
  SDLK_6, // 6
  0},
  {141,
  "Transparency 70%",
  "Turns transparency on and sets its",
  "opacity at 70%.",
  "",
  true,
  SDLK_7, // 7
  0},
  {142,
  "Transparency 80%",
  "Turns transparency on and sets its",
  "opacity at 80%.",
  "",
  true,
  SDLK_8, // 8
  0},
  {143,
  "Transparency 90%",
  "Turns transparency on and sets its",
  "opacity at 90%.",
  "",
  true,
  SDLK_9, // 9
  0},
  {144,
  "Transparency 0%",
  "Turns transparency on and sets its",
  "opacity at 0%.",
  "",
  true,
  SDLK_0, // 0
  0},
  {145,
  "Zoom 1:1",
  "Turns magnifier mode off.",
  "",
  "",
  true,
  SDLK_1|MOD_CTRL, /* Ctrl + 1 */
  0},
  {146,
  "Zoom 2:1",
  "Turns magnifier mode on and set its",
  "factor to 2:1",
  "",
  true,
  SDLK_2|MOD_CTRL, /* Ctrl + 2 */
  0},
  {147,
  "Zoom 3:1",
  "Turns magnifier mode on and set its",
  "factor to 3:1",
  "",
  true,
  SDLK_3|MOD_CTRL, /* Ctrl + 3 */
  0},
  {148,
  "Zoom 4:1",
  "Turns magnifier mode on and set its",
  "factor to 4:1",
  "",
  true,
  SDLK_4|MOD_CTRL, /* Ctrl + 4 */
  0},
  {149,
  "Zoom 5:1",
  "Turns magnifier mode on and set its",
  "factor to 5:1",
  "",
  true,
  SDLK_5|MOD_CTRL, /* Ctrl + 5 */
  0},
  {150,
  "Zoom 6:1",
  "Turns magnifier mode on and set its",
  "factor to 6:1",
  "",
  true,
  SDLK_6|MOD_CTRL, /* Ctrl + 6 */
  0},
  {151,
  "Zoom 8:1",
  "Turns magnifier mode on and set its",
  "factor to 8:1",
  "",
  true,
  SDLK_7|MOD_CTRL, /* Ctrl + 7 */
  0},
  {152,
  "Zoom 10:1",
  "Turns magnifier mode on and set its",
  "factor to 10:1",
  "",
  true,
  SDLK_8|MOD_CTRL, /* Ctrl + 8 */
  0},
  {153,
  "Zoom 12:1",
  "Turns magnifier mode on and set its",
  "factor to 12:1",
  "",
  true,
  0,
  0},
  {154,
  "Zoom 14:1",
  "Turns magnifier mode on and set its",
  "factor to 14:1",
  "",
  true,
  0,
  0},
  {155,
  "Zoom 16:1",
  "Turns magnifier mode on and set its",
  "factor to 16:1",
  "",
  true,
  0,
  0},
  {156,
  "Zoom 18:1",
  "Turns magnifier mode on and set its",
  "factor to 18:1",
  "",
  true,
  0,
  0},
  {157,
  "Zoom 20:1",
  "Turns magnifier mode on and set its",
  "factor to 20:1",
  "",
  true,
  0,
  0},
  {158,
  "Show/Hide Grid",
  "Turns on or off the visible grid in ",
  "the magnified view. Grid cells match",
  "the size ",
  true,
  SDLK_g|MOD_SHIFT|MOD_ALT, // Shift + Alt + G,
  0},
};

word Ordering[NB_SHORTCUTS]=
{
  SPECIAL_SCROLL_UP,                // Scroll up
  SPECIAL_SCROLL_DOWN,              // Scroll down
  SPECIAL_SCROLL_LEFT,              // Scroll left
  SPECIAL_SCROLL_RIGHT,             // Scroll right
  SPECIAL_SCROLL_UP_FAST,           // Scroll up faster
  SPECIAL_SCROLL_DOWN_FAST,         // Scroll down faster
  SPECIAL_SCROLL_LEFT_FAST,         // Scroll left faster
  SPECIAL_SCROLL_RIGHT_FAST,        // Scroll right faster
  SPECIAL_SCROLL_UP_SLOW,           // Scroll up slower
  SPECIAL_SCROLL_DOWN_SLOW,         // Scroll down slower
  SPECIAL_SCROLL_LEFT_SLOW,         // Scroll left slower
  SPECIAL_SCROLL_RIGHT_SLOW,        // Scroll right slower
  SPECIAL_MOUSE_UP,                 // Emulate mouse up
  SPECIAL_MOUSE_DOWN,               // Emulate mouse down
  SPECIAL_MOUSE_LEFT,               // Emulate mouse left
  SPECIAL_MOUSE_RIGHT,              // Emulate mouse right
  SPECIAL_CLICK_LEFT,               // Emulate mouse click left
  SPECIAL_CLICK_RIGHT,              // Emulate mouse click right
  0x100+BUTTON_HIDE,                // Show / Hide menu
  SPECIAL_SHOW_HIDE_CURSOR,         // Show / Hide cursor
  SPECIAL_DOT_PAINTBRUSH,           // Paintbrush = "."
  0x100+BUTTON_PAINTBRUSHES,        // Paintbrush choice
  0x200+BUTTON_PAINTBRUSHES,        // Monochrome brush
  0x100+BUTTON_DRAW,                // Freehand drawing
  0x200+BUTTON_DRAW,                // Switch freehand drawing mode
  SPECIAL_CONTINUOUS_DRAW,          // Continuous freehand drawing
  0x100+BUTTON_LINES,               // Line
  0x200+BUTTON_LINES,               // Knotted lines
  0x100+BUTTON_AIRBRUSH,            // Spray
  0x200+BUTTON_AIRBRUSH,            // Spray menu
  0x100+BUTTON_FLOODFILL,           // Floodfill
  0x200+BUTTON_FLOODFILL,           // Replace color
  0x100+BUTTON_CURVES,              // Bézier's curves
  0x200+BUTTON_CURVES,              // Bézier's curve with 3 or 4 points
  0x100+BUTTON_RECTANGLES,          // Empty rectangle
  0x100+BUTTON_FILLRECT,            // Filled rectangle
  0x100+BUTTON_CIRCLES,             // Empty circle
  0x200+BUTTON_CIRCLES,             // Empty ellipse
  0x100+BUTTON_FILLCIRC,            // Filled circle
  0x200+BUTTON_FILLCIRC,            // Filled ellipse
  0x100+BUTTON_POLYGONS,            // Empty polygon
  0x200+BUTTON_POLYGONS,            // Empty polyform
  0x100+BUTTON_POLYFILL,            // Polyfill
  0x200+BUTTON_POLYFILL,            // Filled polyform
  0x100+BUTTON_GRADRECT,            // Gradient rectangle
  0x200+BUTTON_GRADRECT,            // Gradation menu
  0x100+BUTTON_SPHERES,             // Spheres
  0x200+BUTTON_SPHERES,             // Gradient ellipses
  0x100+BUTTON_ADJUST,              // Adjust picture
  0x200+BUTTON_ADJUST,              // Flip picture menu
  0x100+BUTTON_EFFECTS,             // Menu des effets
  SPECIAL_SHADE_MODE,               // Shade mode
  SPECIAL_SHADE_MENU,               // Shade menu
  SPECIAL_QUICK_SHADE_MODE,         // Quick-shade mode
  SPECIAL_QUICK_SHADE_MENU,         // Quick-shade menu
  SPECIAL_STENCIL_MODE,             // Stencil mode
  SPECIAL_STENCIL_MENU,             // Stencil menu
  SPECIAL_MASK_MODE,                // Mask mode
  SPECIAL_MASK_MENU,                // Mask menu
  SPECIAL_GRID_MODE,                // Grid mode
  SPECIAL_GRID_MENU,                // Grid menu
  SPECIAL_SIEVE_MODE,               // Sieve mode
  SPECIAL_SIEVE_MENU,               // Sieve menu
  SPECIAL_INVERT_SIEVE,             // Inverser la trame du mode Sieve
  SPECIAL_COLORIZE_MODE,            // Colorize mode
  SPECIAL_COLORIZE_MENU,            // Colorize menu
  SPECIAL_SMOOTH_MODE,              // Smooth mode
  SPECIAL_SMOOTH_MENU,              // Smooth menu
  SPECIAL_SMEAR_MODE,               // Smear mode
  SPECIAL_TILING_MODE,              // Tiling mode
  SPECIAL_TILING_MENU,              // Tiling menu
  0x100+BUTTON_BRUSH,               // Pick brush
  0x100+BUTTON_POLYBRUSH,           // Pick polyform brush
  0x200+BUTTON_BRUSH,               // Restore brush
  SPECIAL_FLIP_X,                   // Flip X
  SPECIAL_FLIP_Y,                   // Flip Y
  SPECIAL_ROTATE_90,                // 90° brush rotation
  SPECIAL_ROTATE_180,               // 180° brush rotation
  SPECIAL_STRETCH,                  // Stretch brush
  SPECIAL_DISTORT,                  // Distort brush
  SPECIAL_OUTLINE,                  // Outline brush
  SPECIAL_NIBBLE,                   // Nibble brush
  SPECIAL_GET_BRUSH_COLORS,         // Get colors from brush
  SPECIAL_RECOLORIZE_BRUSH,         // Recolorize brush
  SPECIAL_ROTATE_ANY_ANGLE,         // Rotate brush by any angle
  0x100+BUTTON_COLORPICKER,         // Pipette
  0x200+BUTTON_COLORPICKER,         // Swap fore/back color
  0x100+BUTTON_MAGNIFIER,           // Magnifier mode
  0x200+BUTTON_MAGNIFIER,           // Zoom factor menu
  SPECIAL_ZOOM_IN,                  // Zoom in
  SPECIAL_ZOOM_OUT,                 // Zoom out
  0x100+BUTTON_BRUSH_EFFECTS,       // Brush effects menu
  0x100+BUTTON_TEXT,                // Text
  0x100+BUTTON_RESOL,               // Resolution menu
  0x200+BUTTON_RESOL,               // Safety resolution
  0x100+BUTTON_HELP,                // Help & credits
  0x200+BUTTON_HELP,                // Statistics
  0x100+BUTTON_PAGE,                // Go to spare page
  0x200+BUTTON_PAGE,                // Copy to spare page
  0x100+BUTTON_SAVE,                // Save as
  0x200+BUTTON_SAVE,                // Save
  0x100+BUTTON_LOAD,                // Load
  0x200+BUTTON_LOAD,                // Re-load
  SPECIAL_SAVE_BRUSH,               // Save brush
  SPECIAL_LOAD_BRUSH,               // Load brush
  0x100+BUTTON_SETTINGS,            // Settings
  0x100+BUTTON_UNDO,                // Undo
  0x200+BUTTON_UNDO,                // Redo
  0x100+BUTTON_KILL,                // Kill
  0x100+BUTTON_CLEAR,               // Clear
  0x200+BUTTON_CLEAR,               // Clear with backcolor
  0x100+BUTTON_QUIT,                // Quit
  0x100+BUTTON_PALETTE,             // Palette menu
  0x200+BUTTON_PALETTE,             // Palette menu secondaire
  SPECIAL_EXCLUDE_COLORS_MENU,      // Exclude colors menu
  0x100+BUTTON_PAL_LEFT,            // Scroll palette left
  0x100+BUTTON_PAL_RIGHT,           // Scroll palette right
  0x200+BUTTON_PAL_LEFT,            // Scroll palette left faster
  0x200+BUTTON_PAL_RIGHT,           // Scroll palette right faster
  SPECIAL_CENTER_ATTACHMENT,        // Center brush attachement
  SPECIAL_TOP_LEFT_ATTACHMENT,      // Top-left brush attachement
  SPECIAL_TOP_RIGHT_ATTACHMENT,     // Top-right brush attachement
  SPECIAL_BOTTOM_LEFT_ATTACHMENT,   // Bottom-left brush attachement
  SPECIAL_BOTTOM_RIGHT_ATTACHMENT,  // Bottom right brush attachement
  SPECIAL_NEXT_FORECOLOR,           // Next foreground color
  SPECIAL_PREVIOUS_FORECOLOR,       // Previous foreground color
  SPECIAL_NEXT_BACKCOLOR,           // Next background color
  SPECIAL_PREVIOUS_BACKCOLOR,       // Previous background color
  SPECIAL_NEXT_USER_FORECOLOR,      // Next user-defined foreground color
  SPECIAL_PREVIOUS_USER_FORECOLOR,  // Previous user-defined foreground color
  SPECIAL_NEXT_USER_BACKCOLOR,      // Next user-defined background color
  SPECIAL_PREVIOUS_USER_BACKCOLOR,  // Previous user-defined background color
  SPECIAL_SMALLER_PAINTBRUSH,       // Sets paintbrush size: smaller
  SPECIAL_BIGGER_PAINTBRUSH,        // Sets paintbrush size: bigger
  SPECIAL_EFFECTS_OFF,              // Turns off all effects
  SPECIAL_TRANSPARENCY_1,           // Sets transparency level 10%
  SPECIAL_TRANSPARENCY_2,           // Sets transparency level 20% 
  SPECIAL_TRANSPARENCY_3,           // Sets transparency level 30%
  SPECIAL_TRANSPARENCY_4,           // Sets transparency level 40%
  SPECIAL_TRANSPARENCY_5,           // Sets transparency level 50%
  SPECIAL_TRANSPARENCY_6,           // Sets transparency level 60%
  SPECIAL_TRANSPARENCY_7,           // Sets transparency level 70%
  SPECIAL_TRANSPARENCY_8,           // Sets transparency level 80%
  SPECIAL_TRANSPARENCY_9,           // Sets transparency level 90%
  SPECIAL_TRANSPARENCY_0,           // Sets transparency level 00%
  SPECIAL_ZOOM_1,                   /**< Sets zoom factor to 1:1 (no magnification) */
  SPECIAL_ZOOM_2,                   /**< Sets zoom factor to 2:1 */
  SPECIAL_ZOOM_3,                   /**< Sets zoom factor to 3:1 */
  SPECIAL_ZOOM_4,                   /**< Sets zoom factor to 4:1 */
  SPECIAL_ZOOM_5,                   /**< Sets zoom factor to 5:1 */
  SPECIAL_ZOOM_6,                   /**< Sets zoom factor to 6:1 */
  SPECIAL_ZOOM_8,                   /**< Sets zoom factor to 8:1 */
  SPECIAL_ZOOM_10,                  /**< Sets zoom factor to 10:1 */
  SPECIAL_ZOOM_12,                  /**< Sets zoom factor to 12:1 */
  SPECIAL_ZOOM_14,                  /**< Sets zoom factor to 14:1 */
  SPECIAL_ZOOM_16,                  /**< Sets zoom factor to 16:1 */
  SPECIAL_ZOOM_18,                  /**< Sets zoom factor to 18:1 */
  SPECIAL_ZOOM_20,                  /**< Sets zoom factor to 20:1 */
  SPECIAL_SHOW_GRID,
};
