/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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

#ifndef KEYCODES_H_INCLUDED
#define KEYCODES_H_INCLUDED

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#endif

#if defined(USE_SDL)
#define K2K(x) (x)
#elif defined(USE_SDL2)
#define K2K(x) ((((x) & 0x40000000) >> 19) | ((x) & 0x1FF))
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
#define KEY_EQUALS       K2K(SDLK_EQUALS)
#define KEY_MINUS        K2K(SDLK_MINUS)
#define KEY_PERIOD       K2K(SDLK_PERIOD)
#define KEY_CAPSLOCK     K2K(SDLK_CAPSLOCK)
#define KEY_CLEAR        K2K(SDLK_CLEAR)
#define KEY_SPACE        K2K(SDLK_SPACE)
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
#define KEY_EQUALS       23
#define KEY_MINUS        24
#define KEY_PERIOD       25
#define KEY_CAPSLOCK     26
#define KEY_CLEAR        27
#define KEY_SPACE        28
#define KEY_0            29
#define KEY_1            30
#define KEY_2            31
#define KEY_3            32
#define KEY_4            33
#define KEY_5            34
#define KEY_6            35
#define KEY_7            36
#define KEY_8            37
#define KEY_9            38
#define KEY_a            39
#define KEY_b            40
#define KEY_c            41
#define KEY_d            42
#define KEY_e            43
#define KEY_f            44
#define KEY_g            45
#define KEY_h            46
#define KEY_i            47
#define KEY_j            48
#define KEY_k            49
#define KEY_l            50
#define KEY_m            51
#define KEY_n            52
#define KEY_o            53
#define KEY_p            54
#define KEY_q            55
#define KEY_r            56
#define KEY_s            57
#define KEY_t            58
#define KEY_u            59
#define KEY_v            60
#define KEY_w            61
#define KEY_x            62
#define KEY_y            63
#define KEY_z            64
#define KEY_KP0          65
#define KEY_KP1          66
#define KEY_KP2          67
#define KEY_KP3          68
#define KEY_KP4          69
#define KEY_KP5          70
#define KEY_KP6          71
#define KEY_KP7          72
#define KEY_KP8          73
#define KEY_KP9          74
#define KEY_F1           75
#define KEY_F2           76
#define KEY_F3           77
#define KEY_F4           78
#define KEY_F5           79
#define KEY_F6           80
#define KEY_F7           81
#define KEY_F8           82
#define KEY_F9           83
#define KEY_F10          84
#define KEY_F11          85
#define KEY_F12          86
// end of KEY definitions for others
#endif

#endif
