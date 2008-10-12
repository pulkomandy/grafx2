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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdbool.h>
#include <SDL.h>

/*** Types definitions and structs ***/

typedef struct{
    uint16_t Numero;
    char Libelle[36];
    char Explic1[77];
    char Explic2[77];
    bool Suppr;       // Raccourci facultatif
    uint16_t Touche;
} S_ConfigTouche;

S_ConfigTouche ConfigTouche[] = {
  {0,
  "Scroll up",
  "Scrolls the picture up, both in magnify and normal mode.",
  "This hotkey cannot be removed.",
  false,
  SDLK_UP}, // HAUT
  {1,
  "Scroll down",
  "Scrolls the picture down, both in magnify and normal mode.",
  "This hotkey cannot be removed.",
  false,
  SDLK_DOWN}, // BAS
  {2,
  "Scroll left",
  "Scrolls the picture to the left, both in magnify and normal mode.",
  "This hotkey cannot be removed.",
  false,
  SDLK_LEFT}, // GAUCHE
  {3,
  "Scroll right",
  "Scrolls the picture to the right, both in magnify and normal mode.",
  "This hotkey cannot be removed.",
  false,
  SDLK_RIGHT}, // DROITE
  {4,
  "Faster scroll up",
  "Used to scroll upwards in the picture fast, either in magnify and normal",
  "mode.",
  true,
  SDLK_UP|MOD_SHIFT}, // Shift + Haut
  {5,
  "Faster scroll down",
  "Used to scroll downwards in the picture fast, either in magnify and normal",
  "mode.",
  true,
  SDLK_DOWN|MOD_SHIFT}, // Shift + Bas
  {6,
  "Faster scroll left",
  "Used to scroll to the left in the picture fast, either in magnify and normal",
  "mode.",
  true,
  SDLK_LEFT|MOD_SHIFT}, // Shift + Gauche
  {7,
  "Faster scroll right",
  "Used to scroll to the right in the picture fast, either in magnify and",
  "normal mode.",
  true,
  SDLK_RIGHT|MOD_SHIFT}, // Shift + Droite
  {8,
  "Slower scroll up",
  "Used to scroll upwards in the picture pixel by pixel, either in magnify and",
  "normal mode.",
  true,
  SDLK_UP|MOD_ALT}, // Alt + Haut
  {9,
  "Slower scroll down",
  "Used to scroll downwards in the picture pixel by pixel, either in magnify and",
  "normal mode.",
  true,
  SDLK_DOWN|MOD_ALT}, // Alt + Bas
  {10,
  "Slower scroll left",
  "Used to scroll to the left in the picture pixel by pixel, either in magnify",
  "and normal mode.",
  true,
  SDLK_LEFT|MOD_ALT}, // Alt + Gauche
  {11,
  "Slower scroll right",
  "Used to scroll to the right in the picture pixel by pixel, either in magnify",
  "and normal mode.",
  true,
  SDLK_RIGHT|MOD_ALT}, // Alt + Droite
  {12,
  "Move mouse cursor 1 pixel up",
  "Used to simulate a very small mouse deplacement upwards.",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_UP|MOD_CTRL}, // Ctrl + Haut
  {13,
  "Move mouse cursor 1 pixel down",
  "Used to simulate a very small mouse deplacement downwards.",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_DOWN|MOD_CTRL}, // Ctrl + Bas
  {14,
  "Move mouse cursor 1 pixel left",
  "Used to simulate a very small mouse deplacement to the left.",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_LEFT|MOD_CTRL}, // Ctrl + Gauche
  {15,
  "Move mouse cursor 1 pixel right",
  "Used to simulate a very small mouse deplacement to the right.",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_RIGHT|MOD_CTRL}, // Ctrl + Droite
  {16,
  "Simulate left mouse click",
  "Used to simulate a click with the left mouse button..",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_SPACE}, // Space
  {17,
  "Simulate right mouse click",
  "Used to simulate a click with the right mouse button..",
  "It""s very useful when you want a ultra-high precision.",
  true,
  SDLK_SPACE|MOD_SHIFT}, // Shift + Space
  {18,
  "Show/hide option menu",
  "Switch the tool bar display on/off.",
  "This hot-key cannot be removed.",
  false,
  SDLK_F10}, // F10
  {19,
  "Show/hide cursor",
  "Switch the cursor display on/off.",
  "This only works on the \"small cross\" and \"hand\" cursors.",
  true,
  SDLK_F9}, // F9
  {20,
  "Set paintbrush to 1 pixel",
  "Useful when you want to use a \"single-pixel-brush\".",
  "",
  true,
  SDLK_DELETE}, // Del
  {21,
  "Paintbrush choice",
  "Opens a menu where you can choose a paintbrush out of 24 predefined ones.",
  "",
  true,
  SDLK_F4}, // F4
  {22,
  "Monochrome brush",
  "Turn your current user-defined brush into a single colored one.",
  "All non-transparent colors are set to current foreground color.",
  true,
  SDLK_F4|MOD_SHIFT}, // Shift + F4
  {23,
  "Freehand drawing",
  "Set the drawing mode to the classical freehand one.",
  "",
  true,
  SDLK_d}, // D
  {24,
  "Switch freehand drawing mode",
  "Switch between the 3 ways to use freehand drawing.",
  "These modes are: continuous, discontinuous and point by point.",
  true,
  SDLK_d|MOD_SHIFT}, // Shift + D
  {25,
  "Continuous freehand drawing",
  "Switch directly to continuous freehand drawing mode.",
  "",
  true,
  SDLK_d|MOD_CTRL}, // Ctrl + D
  {26,
  "Line",
  "Allows you to draw lines.",
  "",
  true,
  SDLK_l}, // L
  {27,
  "Knotted lines",
  "Allows you to draw linked lines.",
  "This mode can also be called \"Polyline\".",
  true,
  SDLK_l|MOD_SHIFT}, // Shift + L
  {28,
  "Spray",
  "Allows you to spray brushes randomly in the picture.",
  "",
  true,
  SDLK_a}, // A (Q en AZERTY)
  {29,
  "Spray menu",
  "Opens a menu in which you can configure the spray flow and size.",
  "",
  true,
  SDLK_a|MOD_SHIFT}, // Shift + A
  {30,
  "Flood-fill",
  "Allows you to fill an area of the picture made of pixels of the same color.",
  "",
  true,
  SDLK_f}, // F
  {124,
  "Replace color",
  "Allows you to replace all the pixels of the color pointed by the mouse with",
  "the fore-color or the back-color.",
  true,
  SDLK_f|MOD_SHIFT}, // Shift + F
  {31,
  "Bezier""s curves",
  "Allows you to draw Bezier""s curves.",
  "",
  true,
  SDLK_i}, // I
  {32,
  "Bezier""s curve with 3 or 4 points",
  "Allows you to choose whether you want to draw Bezier""s curves with 3 or 4",
  "points.",
  true,
  SDLK_i|MOD_SHIFT}, // Shift + I
  {33,
  "Empty rectangle",
  "Allows you to draw a rectangle using the brush.",
  "",
  true,
  SDLK_r}, // R
  {34,
  "Filled rectangle",
  "Allows you to draw a filled rectangle.",
  "",
  true,
  SDLK_r|MOD_SHIFT}, // Shift + R
  {35,
  "Empty circle",
  "Allows you to draw a circle using the brush.",
  "",
  true,
  SDLK_c}, // C
  {36,
  "Empty ellipse",
  "Allows you to draw an ellipse using the brush.",
  "",
  true,
  SDLK_c|MOD_CTRL}, // Ctrl + C
  {37,
  "Filled circle",
  "Allows you to draw a filled circle.",
  "",
  true,
  SDLK_c|MOD_SHIFT}, // Shift + C
  {38,
  "Filled ellipse",
  "Allows you to draw a filled ellipse.",
  "",
  true,
  SDLK_c|MOD_SHIFT|MOD_CTRL}, // Shift + Ctrl + C
  {39,
  "Empty polygon",
  "Allows you to draw a polygon using the brush.",
  "",
  true,
  SDLK_n}, // N
  {40,
  "Empty \"polyform\"",
  "Allows you to draw a freehand polygon using the brush.",
  "",
  true,
  SDLK_n|MOD_CTRL}, // Ctrl + N
  {41,
  "Filled polygon",
  "Allows you to draw a filled polygon.",
  "",
  true,
  SDLK_n|MOD_SHIFT}, // Shift + N
  {42,
  "Filled \"polyform\"",
  "Allows you to draw a filled freehand polygon.",
  "",
  true,
  SDLK_n|MOD_SHIFT|MOD_CTRL}, // Shift + Ctrl + N
  {43,
  "Rectangle with gradation",
  "Allows you to draw a rectangle with a color gradation.",
  "",
  true,
  SDLK_r|MOD_ALT}, // Alt + R
  {44,
  "Gradation menu",
  "Allows you to configure the way color gradations are calculated.",
  "",
  true,
  SDLK_g|MOD_ALT}, // Alt + G
  {45,
  "Sphere with gradation",
  "Allows you to draw a rectangle with a color gradation.",
  "",
  true,
  SDLK_c|MOD_ALT}, // Alt + C
  {46,
  "Ellipse with gradation",
  "Allows you to draw an ellipse filled with a color gradation.",
  "",
  true,
  SDLK_c|MOD_SHIFT|MOD_ALT}, // Shift + Alt + C
  {47,
  "Adjust picture",
  "Allows you to move the whole picture in order to re-center it.",
  "Notice that what gets out from a side reappears on the other.",
  true,
  SDLK_KP5}, // Kpad5
  {48,
  "Flip/shrink picture menu",
  "Opens a menu which allows you to flip the picture horizontally/vertically or",
  "to shrink it to half-scale horizontally and/or vertically.",
  true,
  SDLK_KP5|MOD_SHIFT}, // Shift + Kpad5
  {49,
  "Drawing effects",
  "Opens a menu where you can enable/disable and configure the drawing effects",
  "listed below.",
  true,
  SDLK_e}, // E
  {50,
  "Shade mode",
  "Allows you to shade or lighten some pixels of the picture belonging to a",
  "color range, in addition of any drawing tool.",
  true,
  SDLK_F5}, // F5
  {51,
  "Shade menu",
  "Opens a menu where you can choose color ranges to use with the Shade mode.",
  "This menu also contains parameters used both in Shade and Quick-shade modes.",
  true,
  SDLK_F5|MOD_SHIFT}, // Shift + F5
  {131,
  "Quick-shade mode",
  "Does the same thing as shade mode with a simpler method (faster to define",
  "but a bit less powerful).",
  true,
  SDLK_F5|MOD_CTRL}, // Ctrl + F5
  {132,
  "Quick-shade menu",
  "Opens a menu where you can define the parameters of the quick-shade mode.",
  "",
  true,
  SDLK_F5|MOD_SHIFT|MOD_CTRL}, // Shift + Ctrl + F5
  {52,
  "Stencil mode",
  "Allows you to mask colors that must not be affected when you are drawing.",
  "",
  true,
  SDLK_F6}, // F6
  {53,
  "Stencil menu",
  "Opens a menu where you can choose colors masked by the Stencil mode.",
  "",
  true,
  SDLK_F6|MOD_SHIFT}, // Shift + F6
  {54,
  "Mask mode",
  "Allows you to mask colors of the spare page that will keep you from ",
  "drawing. This mode should be called \"True stencil\".",
  true,
  SDLK_F6|MOD_ALT}, // Alt + F6
  {55,
  "Mask menu",
  "Opens a menu where you can choose colors for the Mask mode.",
  "",
  true,
  SDLK_F6|MOD_SHIFT|MOD_ALT}, // Shift + Alt + F6
  {56,
  "Grid mode",
  "Force the cursor to snap up grid points.",
  "",
  true,
  SDLK_g}, // G
  {57,
  "Grid menu",
  "Open a menu where you can configure the grid used by Grid mode.",
  "",
  true,
  SDLK_g|MOD_SHIFT}, // Shift + G
  {58,
  "Sieve mode",
  "Only draws pixels on certain positions matching with a sieve.",
  "",
  true,
  SDLK_g|MOD_CTRL}, // Ctrl + G
  {59,
  "Sieve menu",
  "Opens a menu where you can configure the sieve.",
  "",
  true,
  SDLK_g|MOD_SHIFT|MOD_CTRL}, // Shift + Ctrl + G
  {60,
  "Invert sieve",
  "Inverts the pattern defined in the Sieve menu.",
  "",
  true,
  SDLK_g|MOD_CTRL|MOD_ALT}, // Ctrl + Alt + G
  {61,
  "Colorize mode",
  "Allows you to colorize the pixels on which your brush is pasted.",
  "This permits you to make transparency effects.",
  true,
  SDLK_F7}, // F7
  {62,
  "Colorize menu",
  "Opens a menu where you can give the opacity percentage for Colorize mode.",
  "",
  true,
  SDLK_F7|MOD_SHIFT}, // Shift + F7
  {63,
  "Smooth mode",
  "Soften pixels on which your brush is pasted.",
  "",
  true,
  SDLK_F8}, // F8
  {123,
  "Smooth menu",
  "Opens a menu where you can define the Smooth matrix.",
  "",
  true,
  SDLK_F8|MOD_SHIFT}, // Shift + F8
  {64,
  "Smear mode",
  "Smears the pixels when you move your brush on the picture.",
  "",
  true,
  SDLK_F8|MOD_ALT}, // Alt + F8
  {65,
  "Tiling mode",
  "Puts parts of the brush where you draw.",
  "",
  true,
  SDLK_b|MOD_ALT}, // Alt + B
  {66,
  "Tiling menu",
  "Opens a menu where you can configure the origin of the tiling.",
  "",
  true,
  SDLK_b|MOD_SHIFT|MOD_ALT}, // Shift + Alt + B
  {67,
  "Classical brush grabbing",
  "Allows you to pick a brush defined within a rectangle.",
  "",
  true,
  SDLK_b}, // B
  {68,
  "\"Lasso\" brush grabbing",
  "Allows you to pick a brush defined within a freehand polygon.",
  "",
  true,
  SDLK_b|MOD_CTRL}, // Ctrl + B
  {69,
  "Get previous brush back",
  "Restore the last user-defined brush.",
  "",
  true,
  SDLK_b|MOD_SHIFT}, // Shift + B
  {70,
  "Horizontal brush flipping",
  "Reverse brush horizontally.",
  "",
  true,
  SDLK_x}, // X
  {71,
  "Vertical brush flipping",
  "Reverse brush vertically.",
  "",
  true,
  SDLK_y}, // Y
  {72,
  "90ø brush rotation",
  "Rotate the user-defined brush by 90ø (counter-clockwise).",
  "",
  true,
  SDLK_z}, // Z (W en AZERTY)
  {73,
  "180ø brush rotation",
  "Rotate the user-defined brush by 180ø.",
  "",
  true,
  SDLK_z|MOD_SHIFT}, // Shift + Z
  {74,
  "Strech brush",
  "Allows you to resize the user-defined brush.",
  "",
  true,
  SDLK_s}, // S
  {75,
  "Distort brush",
  "Allows you to distort the user-defined brush.",
  "",
  true,
  SDLK_s|MOD_SHIFT}, // Shift + S
  {76,
  "Outline brush",
  "Outlines the user-defined brush with the fore color.",
  "",
  true,
  SDLK_o}, // O
  {77,
  "Nibble brush",
  "Deletes the borders of the user-defined brush.",
  "This does the opposite of the Outline option.",
  true,
  SDLK_o|MOD_SHIFT}, // Shift + O
  {78,
  "Get colors from brush",
  "Copy colors of the spare page that are used in the brush.",
  "",
  true,
  SDLK_F11}, // F11
  {79,
  "Recolorize brush",
  "Recolorize pixels of the user-defined brush in order to get a brush which",
  "looks like the one grabbed in the spare page.",
  true,
  SDLK_F12}, // F12
  {80,
  "Rotate by any angle",
  "Rotate the brush by an angle that you can define.",
  "",
  true,
  SDLK_w}, // W (Z en AZERTY)
  {81,
  "Pipette",
  "Allows you to copy the color of a pixel in the picture into the foreground",
  "or background color.",
  true,
  SDLK_BACKQUOTE}, // `~ (Touche sous le Esc - ² en AZERTY)
  {82,
  "Swap foreground/background colors",
  "Invert foreground and background colors.",
  "",
  true,
  SDLK_BACKQUOTE|MOD_SHIFT}, // Shift + `~
  {83,
  "Magnifier mode",
  "Allows you to zoom into the picture.",
  "",
  true,
  SDLK_m}, // M (, ? sur AZERTY)
  {84,
  "Zoom factor menu",
  "Opens a menu where you can choose a magnifying factor.",
  "",
  true,
  SDLK_m|MOD_SHIFT}, // Shift + M
  {85,
  "Zoom in",
  "Increase magnifying factor.",
  "",
  true,
  SDLK_KP_PLUS}, // Grey +
  {86,
  "Zoom out",
  "Decrease magnifying factor.",
  "",
  true,
  SDLK_KP_MINUS}, // Grey -
  {87,
  "Brush effects menu",
  "Opens a menu which proposes different effects on the user-defined brush.",
  "",
  true,
  SDLK_b|MOD_CTRL|MOD_ALT}, // Ctrl + Alt + B
  {88,
  "Text",
  "Opens a menu which permits you to type in a character string and to choose a",
  "font, and then creates a new user-defined brush fitting to your choices.",
  true,
  SDLK_t}, // T
  {89,
  "Screen resolution menu",
  "Opens a menu where you can choose the dimensions of the screen in which you",
  "want to draw among the numerous X and SVGA proposed modes.",
  true,
  SDLK_RETURN}, // Enter
  {90,
  "\"Safety\" resolution",
  "Set resolution to 320x200. This can be useful if you choosed a resolution",
  "that is not supported by your monitor and video card. Cannot be removed.",
  false,
  SDLK_RETURN|MOD_SHIFT}, // Shift + Enter
  {91,
  "Help and credits",
  "Opens a window where you can get information about the program.",
  "",
  true,
  SDLK_F1}, // F1
  {92,
  "Statistics",
  "Displays miscellaneous more or less useful information.",
  "",
  true,
  SDLK_F1|MOD_SHIFT}, // Shift + F1
  {93,
  "Jump to spare page",
  "Swap current page and spare page.",
  "",
  true,
  SDLK_TAB}, // Tab
  {94,
  "Copy current page to spare page",
  "Copy current page to spare page.",
  "",
  true,
  SDLK_TAB|MOD_SHIFT}, // Shift + Tab
  {95,
  "Save picture as...",
  "Opens a file-selector that allows you to save your picture with a new",
  "path-name.",
  true,
  SDLK_F2}, // F2
  {96,
  "Save picture",
  "Saves your picture with the last name you gave it.",
  "",
  true,
  SDLK_F2|MOD_SHIFT}, // Shift + F2
  {97,
  "Load picture",
  "Opens a file-selector that allows you to load a new picture.",
  "",
  true,
  SDLK_F3}, // F3
  {98,
  "Re-load picture",
  "Re-load the current picture.",
  "This allows you to cancel modifications made since last saving.",
  true,
  SDLK_F3|MOD_SHIFT}, // Shift + F3
  {99,
  "Save brush",
  "Opens a file-selector that allows you to save your current user-defined",
  "brush.",
  true,
  SDLK_F2|MOD_CTRL}, // Ctrl + F2
  {100,
  "Load brush",
  "Opens a file-selector that allows you to load a brush.",
  "",
  true,
  SDLK_F3|MOD_CTRL}, // Ctrl + F3
  {101,
  "Settings",
  "Opens a menu which permits you to set the dimension of your picture, and to",
  "modify some parameters of the program.",
  true,
  SDLK_F10|MOD_SHIFT}, // Shift + F10
  {102,
  "Undo (Oops!)",
  "Cancel the last action which modified the picture. This has no effect after",
  "a jump to the spare page, loading a picture or modifying its size.",
  true,
  SDLK_u}, // U
  {103,
  "Redo",
  "Redo the last undone action. This has no effect after a jump to the spare",
  "page, loading a picture or modifying its size.",
  true,
  SDLK_u|MOD_SHIFT}, // Shift + U
  {133,
  "Kill",
  "Kills the current page. It actually removes the current page from the list",
  "of \"Undo\" pages.",
  true,
  SDLK_DELETE|MOD_SHIFT}, // Shift + Suppr
  {104,
  "Clear page",
  "Clears the picture with the first color of the palette (usually black).",
  "",
  true,
  SDLK_BACKSPACE}, // BackSpace
  {105,
  "Clear page with backcolor",
  "Clears the picture with the backcolor.",
  "",
  true,
  SDLK_BACKSPACE|MOD_SHIFT}, // Shift + BackSpace
  {106,
  "Quit program",
  "Allows you to leave the program.",
  "If modifications were not saved, confirmation is asked.",
  false,
  SDLK_q}, // Q (A en AZERTY)
  {107,
  "Palette menu",
  "Opens a menu which allows you to modify the current palette.",
  "",
  true,
  SDLK_p}, // P
  {125,
  "Secondary palette menu",
  "Opens a menu which allows you to define color series and some tagged colors.",
  "",
  true,
  SDLK_p|MOD_SHIFT}, // Shift + P
  {130,
  "Exclude colors menu",
  "Opens a menu which allows you to define the colors you don""t want to use in",
  "modes such as Smooth and Transparency, or when remapping a brush.",
  true,
  SDLK_p|MOD_CTRL}, // Ctrl + P
  {108,
  "Scroll palette to the left",
  "Scroll palette in the tool bar to the left, column by column.",
  "",
  true,
  SDLK_PAGEUP}, // PgUp
  {109,
  "Scroll palette to the right",
  "Scroll palette in the tool bar to the right, column by column.",
  "",
  true,
  SDLK_PAGEDOWN}, // PgDn
  {110,
  "Scroll palette to the left faster",
  "Scroll palette in the tool bar to the left, 8 columns by 8 columns.",
  "",
  true,
  SDLK_PAGEUP|MOD_SHIFT}, // Shift + PgUp
  {111,
  "Scroll palette to the right faster",
  "Scroll palette in the tool bar to the right, 8 columns by 8 columns.",
  "",
  true,
  SDLK_PAGEDOWN|MOD_SHIFT}, // Shift + PgDn
  {112,
  "Center brush attachment point",
  "Set the attachement of the user-defined brush to its center.",
  "",
  true,
  SDLK_KP5|MOD_CTRL}, // Ctrl + 5 (pavé numérique)
  {113,
  "Top-left brush attachment point",
  "Set the attachement of the user-defined brush to its top-left corner.",
  "",
  true,
  SDLK_HOME|MOD_CTRL}, // Ctrl + 7
  {114,
  "Top-right brush attachment point",
  "Set the attachement of the user-defined brush to its top-right corner.",
  "",
  true,
  SDLK_PAGEUP|MOD_CTRL}, // Ctrl + 9
  {115,
  "Bottom-left brush attachment point",
  "Set the attachement of the user-defined brush to its bottom-left corner.",
  "",
  true,
  SDLK_END|MOD_CTRL}, // Ctrl + 1
  {116,
  "Bottom-right brush attachment point",
  "Set the attachement of the user-defined brush to its bottom-right corner.",
  "",
  true,
  SDLK_PAGEDOWN|MOD_CTRL}, // Ctrl + 3
  {117,
  "Next foreground color",
  "Set the foreground color to the next in the palette.",
  "",
  true,
  SDLK_RIGHTBRACKET}, // ] (0x en AZERTY)
  {118,
  "Previous foreground color",
  "Set the foreground color to the previous in the palette.",
  "",
  true,
  SDLK_LEFTBRACKET}, // [ (^ en AZERTY)
  {119,
  "Next background color",
  "Set the background color to the next in the palette.",
  "",
  true,
  SDLK_RIGHTBRACKET|MOD_SHIFT}, // Shift + ]
  {120,
  "Previous background color",
  "Set the background color to the previous in the palette.",
  "",
  true,
  SDLK_LEFTBRACKET|MOD_SHIFT}, // Shift + [
  {126,
  "Next user-defined forecolor",
  "Set the foreground color to the next in the user-defined color series.",
  "",
  true,
  SDLK_EQUALS}, // "=+"
  {127,
  "Previous user-defined forecolor",
  "Set the foreground color to the previous in the user-defined color series.",
  "",
  true,
  SDLK_MINUS}, // "-_" (")ø" en AZERTY
  {128,
  "Next user-defined backcolor",
  "Set the background color to the next in the user-defined color series.",
  "",
  true,
  SDLK_EQUALS|MOD_SHIFT}, // Shift + "=+"
  {129,
  "Previous user-defined backcolor",
  "Set the background color to the previous in the user-defined color series.",
  "",
  true,
  SDLK_MINUS|MOD_SHIFT}, // Shift + "-_" (")ø" en AZERTY
  {121,
  "Shrink paintbrush",
  "Decrease the width of the paintbrush if it is special circle or square.",
  "",
  true,
  SDLK_COMMA}, // , < (;. en AZERTY)
  {122,
  "Enlarge paintbrush",
  "Increase the width of the paintbrush if it is special circle or square.",
  "",
  true,
  SDLK_PERIOD}, // .> (:/ en AZERTY)
};
