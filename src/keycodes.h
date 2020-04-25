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

#ifndef KEYCODES_H_INCLUDED
#define KEYCODES_H_INCLUDED

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#elif defined(WIN32)
#include <windows.h>
#elif defined(USE_X11)
#include <X11/keysym.h>
#endif

#if defined(USE_SDL)
#define K2K(x) (x)
#elif defined(USE_SDL2)
#define K2K(x) ((((x) & 0x40000000) >> 19) | ((x) & 0x1FF))
#elif defined(USE_X11)
#define K2K(x) ((x) & 0x0FFF)
#endif

/* generated lists */
#if defined(USE_SDL) || defined(USE_SDL2)
// KEY definitions for SDL and SDL2
#define KEY_UNKNOWN      K2K(SDLK_UNKNOWN)
#define KEY_ESCAPE       K2K(SDLK_ESCAPE)
#define KEY_RETURN       K2K(SDLK_RETURN)
#define KEY_BACKSPACE    K2K(SDLK_BACKSPACE)
#define KEY_TAB          K2K(SDLK_TAB)
#define KEY_UP           K2K(SDLK_UP)
#define KEY_DOWN         K2K(SDLK_DOWN)
#define KEY_LEFT         K2K(SDLK_LEFT)
#define KEY_RIGHT        K2K(SDLK_RIGHT)
#define KEY_LEFTBRACKET  K2K(SDLK_LEFTBRACKET)
#define KEY_RIGHTBRACKET K2K(SDLK_RIGHTBRACKET)
#define KEY_INSERT       K2K(SDLK_INSERT)
#define KEY_DELETE       K2K(SDLK_DELETE)
#define KEY_COMMA        K2K(SDLK_COMMA)
#define KEY_BACKQUOTE    K2K(SDLK_BACKQUOTE)
#define KEY_PAGEUP       K2K(SDLK_PAGEUP)
#define KEY_PAGEDOWN     K2K(SDLK_PAGEDOWN)
#define KEY_HOME         K2K(SDLK_HOME)
#define KEY_END          K2K(SDLK_END)
#define KEY_KP_PLUS      K2K(SDLK_KP_PLUS)
#define KEY_KP_MINUS     K2K(SDLK_KP_MINUS)
#define KEY_KP_MULTIPLY  K2K(SDLK_KP_MULTIPLY)
#define KEY_KP_ENTER     K2K(SDLK_KP_ENTER)
#define KEY_KP_DIVIDE    K2K(SDLK_KP_DIVIDE)
#define KEY_KP_PERIOD    K2K(SDLK_KP_PERIOD)
#define KEY_KP_EQUALS    K2K(SDLK_KP_EQUALS)
#define KEY_EQUALS       K2K(SDLK_EQUALS)
#define KEY_MINUS        K2K(SDLK_MINUS)
#define KEY_PERIOD       K2K(SDLK_PERIOD)
#define KEY_CAPSLOCK     K2K(SDLK_CAPSLOCK)
#define KEY_CLEAR        K2K(SDLK_CLEAR)
#define KEY_SPACE        K2K(SDLK_SPACE)
#define KEY_PAUSE        K2K(SDLK_PAUSE)
#define KEY_LSHIFT       K2K(SDLK_LSHIFT)
#define KEY_RSHIFT       K2K(SDLK_RSHIFT)
#define KEY_LCTRL        K2K(SDLK_LCTRL)
#define KEY_RCTRL        K2K(SDLK_RCTRL)
#define KEY_LALT         K2K(SDLK_LALT)
#define KEY_RALT         K2K(SDLK_RALT)
#define KEY_MENU         K2K(SDLK_MENU)
#define KEY_0            K2K(SDLK_0)
#define KEY_1            K2K(SDLK_1)
#define KEY_2            K2K(SDLK_2)
#define KEY_3            K2K(SDLK_3)
#define KEY_4            K2K(SDLK_4)
#define KEY_5            K2K(SDLK_5)
#define KEY_6            K2K(SDLK_6)
#define KEY_7            K2K(SDLK_7)
#define KEY_8            K2K(SDLK_8)
#define KEY_9            K2K(SDLK_9)
#define KEY_a            K2K(SDLK_a)
#define KEY_b            K2K(SDLK_b)
#define KEY_c            K2K(SDLK_c)
#define KEY_d            K2K(SDLK_d)
#define KEY_e            K2K(SDLK_e)
#define KEY_f            K2K(SDLK_f)
#define KEY_g            K2K(SDLK_g)
#define KEY_h            K2K(SDLK_h)
#define KEY_i            K2K(SDLK_i)
#define KEY_j            K2K(SDLK_j)
#define KEY_k            K2K(SDLK_k)
#define KEY_l            K2K(SDLK_l)
#define KEY_m            K2K(SDLK_m)
#define KEY_n            K2K(SDLK_n)
#define KEY_o            K2K(SDLK_o)
#define KEY_p            K2K(SDLK_p)
#define KEY_q            K2K(SDLK_q)
#define KEY_r            K2K(SDLK_r)
#define KEY_s            K2K(SDLK_s)
#define KEY_t            K2K(SDLK_t)
#define KEY_u            K2K(SDLK_u)
#define KEY_v            K2K(SDLK_v)
#define KEY_w            K2K(SDLK_w)
#define KEY_x            K2K(SDLK_x)
#define KEY_y            K2K(SDLK_y)
#define KEY_z            K2K(SDLK_z)
#if defined(USE_SDL)
#define KEY_KP0          K2K(SDLK_KP0)
#define KEY_KP1          K2K(SDLK_KP1)
#define KEY_KP2          K2K(SDLK_KP2)
#define KEY_KP3          K2K(SDLK_KP3)
#define KEY_KP4          K2K(SDLK_KP4)
#define KEY_KP5          K2K(SDLK_KP5)
#define KEY_KP6          K2K(SDLK_KP6)
#define KEY_KP7          K2K(SDLK_KP7)
#define KEY_KP8          K2K(SDLK_KP8)
#define KEY_KP9          K2K(SDLK_KP9)
#define KEY_SCROLLOCK    K2K(SDLK_SCROLLOCK)
#else
#define KEY_KP0          K2K(SDLK_KP_0)
#define KEY_KP1          K2K(SDLK_KP_1)
#define KEY_KP2          K2K(SDLK_KP_2)
#define KEY_KP3          K2K(SDLK_KP_3)
#define KEY_KP4          K2K(SDLK_KP_4)
#define KEY_KP5          K2K(SDLK_KP_5)
#define KEY_KP6          K2K(SDLK_KP_6)
#define KEY_KP7          K2K(SDLK_KP_7)
#define KEY_KP8          K2K(SDLK_KP_8)
#define KEY_KP9          K2K(SDLK_KP_9)
#define KEY_SCROLLOCK    K2K(SDLK_SCROLLLOCK)
#endif
#define KEY_F1           K2K(SDLK_F1)
#define KEY_F2           K2K(SDLK_F2)
#define KEY_F3           K2K(SDLK_F3)
#define KEY_F4           K2K(SDLK_F4)
#define KEY_F5           K2K(SDLK_F5)
#define KEY_F6           K2K(SDLK_F6)
#define KEY_F7           K2K(SDLK_F7)
#define KEY_F8           K2K(SDLK_F8)
#define KEY_F9           K2K(SDLK_F9)
#define KEY_F10          K2K(SDLK_F10)
#define KEY_F11          K2K(SDLK_F11)
#define KEY_F12          K2K(SDLK_F12)
// end of KEY definitions for SDL and SDL2
#elif defined(USE_X11)
// KEY definitions for x11
#define KEY_UNKNOWN      0
#define KEY_ESCAPE       K2K(XK_Escape)
#define KEY_RETURN       K2K(XK_Return)
#define KEY_BACKSPACE    K2K(XK_BackSpace)
#define KEY_TAB          K2K(XK_Tab)
#define KEY_UP           K2K(XK_Up)
#define KEY_DOWN         K2K(XK_Down)
#define KEY_LEFT         K2K(XK_Left)
#define KEY_RIGHT        K2K(XK_Right)
#define KEY_LEFTBRACKET  K2K(XK_bracketleft)
#define KEY_RIGHTBRACKET K2K(XK_bracketright)
#define KEY_INSERT       K2K(XK_Insert)
#define KEY_DELETE       K2K(XK_Delete)
#define KEY_COMMA        K2K(XK_comma)
#define KEY_BACKQUOTE    K2K(XK_grave)
#define KEY_PAGEUP       K2K(XK_Page_Up)
#define KEY_PAGEDOWN     K2K(XK_Page_Down)
#define KEY_HOME         K2K(XK_Home)
#define KEY_END          K2K(XK_End)
#define KEY_KP_PLUS      K2K(XK_KP_Add)
#define KEY_KP_MINUS     K2K(XK_KP_Subtract)
#define KEY_KP_MULTIPLY  K2K(XK_KP_Multiply)
#define KEY_KP_ENTER     K2K(XK_KP_Enter)
#define KEY_KP_DIVIDE    K2K(XK_KP_Divide)
#define KEY_KP_PERIOD    K2K(XK_KP_Decimal)
#define KEY_KP_EQUALS    K2K(XK_KP_Equal)
#define KEY_EQUALS       K2K(XK_equal)
#define KEY_MINUS        K2K(XK_minus)
#define KEY_PERIOD       K2K(XK_period)
#define KEY_CAPSLOCK     K2K(XK_Caps_Lock)
#define KEY_CLEAR        K2K(XK_Clear)
#define KEY_SPACE        K2K(XK_space)
#define KEY_PAUSE        K2K(XK_Pause)
#define KEY_LSHIFT       K2K(XK_Shift_L)
#define KEY_RSHIFT       K2K(XK_Shift_R)
#define KEY_LCTRL        K2K(XK_Control_L)
#define KEY_RCTRL        K2K(XK_Control_R)
#define KEY_LALT         K2K(XK_Alt_L)
#define KEY_RALT         K2K(XK_Alt_R)
#define KEY_MENU         K2K(XK_Menu)
#define KEY_0            K2K(XK_0)
#define KEY_1            K2K(XK_1)
#define KEY_2            K2K(XK_2)
#define KEY_3            K2K(XK_3)
#define KEY_4            K2K(XK_4)
#define KEY_5            K2K(XK_5)
#define KEY_6            K2K(XK_6)
#define KEY_7            K2K(XK_7)
#define KEY_8            K2K(XK_8)
#define KEY_9            K2K(XK_9)
#define KEY_a            K2K(XK_a)
#define KEY_b            K2K(XK_b)
#define KEY_c            K2K(XK_c)
#define KEY_d            K2K(XK_d)
#define KEY_e            K2K(XK_e)
#define KEY_f            K2K(XK_f)
#define KEY_g            K2K(XK_g)
#define KEY_h            K2K(XK_h)
#define KEY_i            K2K(XK_i)
#define KEY_j            K2K(XK_j)
#define KEY_k            K2K(XK_k)
#define KEY_l            K2K(XK_l)
#define KEY_m            K2K(XK_m)
#define KEY_n            K2K(XK_n)
#define KEY_o            K2K(XK_o)
#define KEY_p            K2K(XK_p)
#define KEY_q            K2K(XK_q)
#define KEY_r            K2K(XK_r)
#define KEY_s            K2K(XK_s)
#define KEY_t            K2K(XK_t)
#define KEY_u            K2K(XK_u)
#define KEY_v            K2K(XK_v)
#define KEY_w            K2K(XK_w)
#define KEY_x            K2K(XK_x)
#define KEY_y            K2K(XK_y)
#define KEY_z            K2K(XK_z)
#define KEY_KP0          K2K(XK_KP_0)
#define KEY_KP1          K2K(XK_KP_1)
#define KEY_KP2          K2K(XK_KP_2)
#define KEY_KP3          K2K(XK_KP_3)
#define KEY_KP4          K2K(XK_KP_4)
#define KEY_KP5          K2K(XK_KP_5)
#define KEY_KP6          K2K(XK_KP_6)
#define KEY_KP7          K2K(XK_KP_7)
#define KEY_KP8          K2K(XK_KP_8)
#define KEY_KP9          K2K(XK_KP_9)
#define KEY_SCROLLOCK    K2K(XK_Scroll_Lock)
#define KEY_F1           K2K(XK_F1)
#define KEY_F2           K2K(XK_F2)
#define KEY_F3           K2K(XK_F3)
#define KEY_F4           K2K(XK_F4)
#define KEY_F5           K2K(XK_F5)
#define KEY_F6           K2K(XK_F6)
#define KEY_F7           K2K(XK_F7)
#define KEY_F8           K2K(XK_F8)
#define KEY_F9           K2K(XK_F9)
#define KEY_F10          K2K(XK_F10)
#define KEY_F11          K2K(XK_F11)
#define KEY_F12          K2K(XK_F12)
// end of KEY definitions for x11
#elif defined(WIN32)
// KEY definitions for win32
#define KEY_UNKNOWN      0
#define KEY_ESCAPE       VK_ESCAPE
#define KEY_RETURN       VK_RETURN
#define KEY_BACKSPACE    VK_BACK
#define KEY_TAB          VK_TAB
#define KEY_UP           VK_UP
#define KEY_DOWN         VK_DOWN
#define KEY_LEFT         VK_LEFT
#define KEY_RIGHT        VK_RIGHT
#define KEY_LEFTBRACKET  VK_OEM_4
#define KEY_RIGHTBRACKET VK_OEM_6
#define KEY_INSERT       VK_INSERT
#define KEY_DELETE       VK_DELETE
#define KEY_COMMA        VK_OEM_COMMA
#define KEY_BACKQUOTE    VK_OEM_3
#define KEY_PAGEUP       VK_PRIOR
#define KEY_PAGEDOWN     VK_NEXT
#define KEY_HOME         VK_HOME
#define KEY_END          VK_END
#define KEY_KP_PLUS      VK_ADD
#define KEY_KP_MINUS     VK_SUBTRACT
#define KEY_KP_MULTIPLY  VK_MULTIPLY
#define KEY_KP_ENTER     VK_RETURN
#define KEY_KP_DIVIDE    VK_DIVIDE
#define KEY_KP_PERIOD    VK_DECIMAL
#define KEY_KP_EQUALS    0
#define KEY_EQUALS       VK_OEM_PLUS
#define KEY_MINUS        VK_OEM_MINUS
#define KEY_PERIOD       VK_OEM_PERIOD
#define KEY_CAPSLOCK     VK_CAPITAL
#define KEY_CLEAR        VK_CLEAR
#define KEY_SPACE        VK_SPACE
#define KEY_PAUSE        VK_PAUSE
#define KEY_LSHIFT       VK_LSHIFT
#define KEY_RSHIFT       VK_RSHIFT
#define KEY_LCTRL        VK_LCONTROL
#define KEY_RCTRL        VK_RCONTROL
#define KEY_LALT         VK_LMENU
#define KEY_RALT         VK_RMENU
#define KEY_MENU         VK_APPS
#define KEY_0            0x30
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_9            0x39
#define KEY_a            0x41
#define KEY_b            0x42
#define KEY_c            0x43
#define KEY_d            0x44
#define KEY_e            0x45
#define KEY_f            0x46
#define KEY_g            0x47
#define KEY_h            0x48
#define KEY_i            0x49
#define KEY_j            0x4a
#define KEY_k            0x4b
#define KEY_l            0x4c
#define KEY_m            0x4d
#define KEY_n            0x4e
#define KEY_o            0x4f
#define KEY_p            0x50
#define KEY_q            0x51
#define KEY_r            0x52
#define KEY_s            0x53
#define KEY_t            0x54
#define KEY_u            0x55
#define KEY_v            0x56
#define KEY_w            0x57
#define KEY_x            0x58
#define KEY_y            0x59
#define KEY_z            0x5a
#define KEY_KP0          VK_NUMPAD0
#define KEY_KP1          VK_NUMPAD1
#define KEY_KP2          VK_NUMPAD2
#define KEY_KP3          VK_NUMPAD3
#define KEY_KP4          VK_NUMPAD4
#define KEY_KP5          VK_NUMPAD5
#define KEY_KP6          VK_NUMPAD6
#define KEY_KP7          VK_NUMPAD7
#define KEY_KP8          VK_NUMPAD8
#define KEY_KP9          VK_NUMPAD9
#define KEY_SCROLLOCK    VK_SCROLL
#define KEY_F1           VK_F1
#define KEY_F2           VK_F2
#define KEY_F3           VK_F3
#define KEY_F4           VK_F4
#define KEY_F5           VK_F5
#define KEY_F6           VK_F6
#define KEY_F7           VK_F7
#define KEY_F8           VK_F8
#define KEY_F9           VK_F9
#define KEY_F10          VK_F10
#define KEY_F11          VK_F11
#define KEY_F12          VK_F12
// end of KEY definitions for win32
#else
// KEY definitions for others
#define KEY_UNKNOWN      0
#define KEY_ESCAPE       1
#define KEY_RETURN       2
#define KEY_BACKSPACE    3
#define KEY_TAB          4
#define KEY_UP           5
#define KEY_DOWN         6
#define KEY_LEFT         7
#define KEY_RIGHT        8
#define KEY_LEFTBRACKET  9
#define KEY_RIGHTBRACKET 10
#define KEY_INSERT       11
#define KEY_DELETE       12
#define KEY_COMMA        13
#define KEY_BACKQUOTE    14
#define KEY_PAGEUP       15
#define KEY_PAGEDOWN     16
#define KEY_HOME         17
#define KEY_END          18
#define KEY_KP_PLUS      19
#define KEY_KP_MINUS     20
#define KEY_KP_MULTIPLY  21
#define KEY_KP_ENTER     22
#define KEY_KP_DIVIDE    23
#define KEY_KP_PERIOD    24
#define KEY_KP_EQUALS    25
#define KEY_EQUALS       26
#define KEY_MINUS        27
#define KEY_PERIOD       28
#define KEY_CAPSLOCK     29
#define KEY_CLEAR        30
#define KEY_SPACE        31
#define KEY_PAUSE        32
#define KEY_LSHIFT       33
#define KEY_RSHIFT       34
#define KEY_LCTRL        35
#define KEY_RCTRL        36
#define KEY_LALT         37
#define KEY_RALT         38
#define KEY_MENU         39
#define KEY_0            40
#define KEY_1            41
#define KEY_2            42
#define KEY_3            43
#define KEY_4            44
#define KEY_5            45
#define KEY_6            46
#define KEY_7            47
#define KEY_8            48
#define KEY_9            49
#define KEY_a            50
#define KEY_b            51
#define KEY_c            52
#define KEY_d            53
#define KEY_e            54
#define KEY_f            55
#define KEY_g            56
#define KEY_h            57
#define KEY_i            58
#define KEY_j            59
#define KEY_k            60
#define KEY_l            61
#define KEY_m            62
#define KEY_n            63
#define KEY_o            64
#define KEY_p            65
#define KEY_q            66
#define KEY_r            67
#define KEY_s            68
#define KEY_t            69
#define KEY_u            70
#define KEY_v            71
#define KEY_w            72
#define KEY_x            73
#define KEY_y            74
#define KEY_z            75
#define KEY_KP0          76
#define KEY_KP1          77
#define KEY_KP2          78
#define KEY_KP3          79
#define KEY_KP4          80
#define KEY_KP5          81
#define KEY_KP6          82
#define KEY_KP7          83
#define KEY_KP8          84
#define KEY_KP9          85
#define KEY_SCROLLOCK    86
#define KEY_F1           87
#define KEY_F2           88
#define KEY_F3           89
#define KEY_F4           90
#define KEY_F5           91
#define KEY_F6           92
#define KEY_F7           93
#define KEY_F8           94
#define KEY_F9           95
#define KEY_F10          96
#define KEY_F11          97
#define KEY_F12          98
// end of KEY definitions for others
#endif

/// Indicates "no keyboard shortcut".
#define KEY_NONE            0

#if defined(USE_SDL)
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

#else
// Not SDL 1.2
#define KEY_MOUSEMIDDLE     0x0210
#define KEY_MOUSEX1         0x0211
#define KEY_MOUSEX2         0x0212
#define KEY_MOUSEWHEELUP    0x0200
#define KEY_MOUSEWHEELDOWN  0x0201
#define KEY_MOUSEWHEELLEFT  0x0202
#define KEY_MOUSEWHEELRIGHT 0x0203
#define KEY_JOYBUTTON       0x0100
#endif

/// The joystick axis are {X,Y} - on all platforms so far.
/// If there is ever a platform where they are reversed, put
/// these lines in each platform "case" below.
#define JOYSTICK_AXIS_X             (0)
#define JOYSTICK_AXIS_Y             (1)

#ifdef __GP2X__

    #define JOYSTICK_THRESHOLD  (4096)

    /// Button definitions for the gp2x
    #define JOY_BUTTON_UP              (0)
    #define JOY_BUTTON_DOWN            (4)
    #define JOY_BUTTON_LEFT            (2)
    #define JOY_BUTTON_RIGHT           (6)
    #define JOY_BUTTON_UPLEFT          (1)
    #define JOY_BUTTON_UPRIGHT         (7)
    #define JOY_BUTTON_DOWNLEFT        (3)
    #define JOY_BUTTON_DOWNRIGHT       (5)
    #define JOY_BUTTON_CLICK           (18)
    #define JOY_BUTTON_A               (12)
    #define JOY_BUTTON_B               (13)
    #define JOY_BUTTON_Y               (14)
    #define JOY_BUTTON_X               (15)
    #define JOY_BUTTON_L               (10)
    #define JOY_BUTTON_R               (11)
    #define JOY_BUTTON_START           (8)
    #define JOY_BUTTON_SELECT          (9)
    #define JOY_BUTTON_VOLUP           (16)
    #define JOY_BUTTON_VOLDOWN         (17)
    
    #define KEY_ESC (KEY_JOYBUTTON+JOY_BUTTON_X)
#elif defined(__WIZ__)
    /// Button definitions for the Wiz
    #define JOY_BUTTON_UP               (0)
    #define JOY_BUTTON_DOWN             (4)
    #define JOY_BUTTON_LEFT             (2)
    #define JOY_BUTTON_RIGHT            (6)
    #define JOY_BUTTON_UPLEFT           (1)
    #define JOY_BUTTON_UPRIGHT          (7)
    #define JOY_BUTTON_DOWNLEFT         (3)
    #define JOY_BUTTON_DOWNRIGHT        (5)
    #define JOY_BUTTON_L                (10)
    #define JOY_BUTTON_R                (11)
    #define JOY_BUTTON_A                (12)
    #define JOY_BUTTON_B                (13)
    #define JOY_BUTTON_X                (14)
    #define JOY_BUTTON_Y                (15)
    #define JOY_BUTTON_MENU             (8)
    #define JOY_BUTTON_SELECT           (9)
    #define JOY_BUTTON_VOLUP            (16)
    #define JOY_BUTTON_VOLDOWN          (17)

    #define KEY_ESC (KEY_JOYBUTTON+JOY_BUTTON_X)
#elif defined (__CAANOO__)

    #define JOYSTICK_THRESHOLD  (4096)

    /// Button definitions for the Caanoo
    #define JOY_BUTTON_A             (0)
    #define JOY_BUTTON_X             (1)
    #define JOY_BUTTON_B             (2)
    #define JOY_BUTTON_Y             (3)
    #define JOY_BUTTON_L             (4)
    #define JOY_BUTTON_R             (5)
    #define JOY_BUTTON_HOME          (6)
    #define JOY_BUTTON_HOLD          (7)
    #define JOY_BUTTON_I             (8)
    #define JOY_BUTTON_II            (9)
    #define JOY_BUTTON_JOY           (10)

    #define KEY_ESC (KEY_JOYBUTTON+JOY_BUTTON_HOME)
#elif defined (__SWITCH__)
/// Button definitions for the Switch
    #define JOY_BUTTON_UP               (13)
    #define JOY_BUTTON_DOWN             (15)
    #define JOY_BUTTON_LEFT             (12)
    #define JOY_BUTTON_RIGHT            (14)
    #define JOY_BUTTON_A                (0)
    #define JOY_BUTTON_X                (2)
    #define JOY_BUTTON_B                (1)
    #define JOY_BUTTON_Y                (3)
    #define JOY_BUTTON_L                (6)
    #define JOY_BUTTON_R                (7)

    #define KEY_ESC (KEY_JOYBUTTON+JOY_BUTTON_X)
#else
  ///
  /// This is the key identifier for ESC. When hard-coding keyboard shortcuts
  /// for buttons, etc. we use this instead of SDLK_ESCAPE,
  /// so the console ports can get a joybutton equivalent of it.
  #define KEY_ESC KEY_ESCAPE
#endif

#endif
