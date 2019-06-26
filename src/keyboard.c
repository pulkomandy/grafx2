/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2010 Alexander Filyanov
    Copyright 2009 Franck Charlet
    Copyright 2008 Yves Rizoud
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#endif
#include "global.h"
#include "keyboard.h"
#include "keycodes.h"
#include "gfx2log.h"

#if defined(USE_X11)
/// State of modifier keys (shift, CTRL, ALT, etc.)
extern word X11_key_mod;
#endif

#if defined(__macosx__)
  // Apple's "command" character is not present in the ANSI table, so we
  // recycled an ANSI value that doesn't have any displayable character
  // associated.
  #define META_KEY_PREFIX "\201"
#elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  // 'Amiga' key: an outlined uppercase A. Drawn on 2 unused characters.
  #define META_KEY_PREFIX "\215\216"
#elif defined(WIN32)
  // Windows Key
  #define META_KEY_PREFIX "Win+"
#else
  // All other platforms
  #define META_KEY_PREFIX "Super+"
#endif

#if defined(USE_SDL)
// Table de correspondance des scancode de clavier IBM PC AT vers
// les symboles de touches SDL (sym).
// La correspondance est bonne si le clavier est QWERTY US, ou si
// l'utilisateur est sous Windows.
// Dans l'ordre des colonnes: Normal, +Shift, +Control, +Alt
static const word Scancode_to_sym[256][4] =
{
/* 00  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 01  Esc   */ { SDLK_ESCAPE      ,SDLK_ESCAPE      ,SDLK_ESCAPE      ,SDLK_ESCAPE      },
/* 02  1 !   */ { SDLK_1           ,SDLK_1           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 03  2 @   */ { SDLK_2           ,SDLK_2           ,SDLK_2           ,SDLK_UNKNOWN     },
/* 04  3 #   */ { SDLK_3           ,SDLK_3           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 05  4 $   */ { SDLK_4           ,SDLK_4           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 06  5 %   */ { SDLK_5           ,SDLK_5           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 07  6 ^   */ { SDLK_6           ,SDLK_6           ,SDLK_6           ,SDLK_UNKNOWN     },
/* 08  7 &   */ { SDLK_7           ,SDLK_7           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 09  8 *   */ { SDLK_8           ,SDLK_8           ,SDLK_8           ,SDLK_UNKNOWN     },
/* 0A  9 (   */ { SDLK_9           ,SDLK_9           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 0B  0 )   */ { SDLK_0           ,SDLK_0           ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 0C  - _   */ { SDLK_MINUS       ,SDLK_MINUS       ,SDLK_MINUS       ,SDLK_UNKNOWN     },
/* 0D  = +   */ { SDLK_EQUALS      ,SDLK_EQUALS      ,SDLK_EQUALS      ,SDLK_UNKNOWN     },
/* 0E  BkSpc */ { SDLK_BACKSPACE   ,SDLK_BACKSPACE   ,SDLK_BACKSPACE   ,SDLK_BACKSPACE   },
/* 0F  Tab   */ { SDLK_TAB         ,SDLK_TAB         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 10  Q     */ { SDLK_q           ,SDLK_q           ,SDLK_q           ,SDLK_q           },
/* 11  W     */ { SDLK_w           ,SDLK_w           ,SDLK_w           ,SDLK_w           },
/* 12  E     */ { SDLK_e           ,SDLK_e           ,SDLK_e           ,SDLK_e           },
/* 13  R     */ { SDLK_r           ,SDLK_r           ,SDLK_r           ,SDLK_r           },
/* 14  T     */ { SDLK_t           ,SDLK_t           ,SDLK_t           ,SDLK_t           },
/* 15  Y     */ { SDLK_y           ,SDLK_y           ,SDLK_y           ,SDLK_y           },
/* 16  U     */ { SDLK_u           ,SDLK_u           ,SDLK_u           ,SDLK_u           },
/* 17  I     */ { SDLK_i           ,SDLK_i           ,SDLK_i           ,SDLK_i           },
/* 18  O     */ { SDLK_o           ,SDLK_o           ,SDLK_o           ,SDLK_o           },
/* 19  P     */ { SDLK_p           ,SDLK_p           ,SDLK_p           ,SDLK_p           },
/* 1A  [     */ { SDLK_LEFTBRACKET ,SDLK_LEFTBRACKET ,SDLK_LEFTBRACKET ,SDLK_LEFTBRACKET },
/* 1B  ]     */ { SDLK_RIGHTBRACKET,SDLK_RIGHTBRACKET,SDLK_RIGHTBRACKET,SDLK_RIGHTBRACKET},
/* 1C  Retrn */ { SDLK_RETURN      ,SDLK_RETURN      ,SDLK_RETURN      ,SDLK_RETURN      },
/* 1D  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 1E  A     */ { SDLK_a           ,SDLK_a           ,SDLK_a           ,SDLK_a           },
/* 1F  S     */ { SDLK_s           ,SDLK_s           ,SDLK_s           ,SDLK_s           },
/* 20  D     */ { SDLK_d           ,SDLK_d           ,SDLK_d           ,SDLK_d           },
/* 21  F     */ { SDLK_f           ,SDLK_f           ,SDLK_f           ,SDLK_f           },
/* 22  G     */ { SDLK_g           ,SDLK_g           ,SDLK_g           ,SDLK_g           },
/* 23  H     */ { SDLK_h           ,SDLK_h           ,SDLK_h           ,SDLK_h           },
/* 24  J     */ { SDLK_j           ,SDLK_j           ,SDLK_j           ,SDLK_j           },
/* 25  K     */ { SDLK_k           ,SDLK_k           ,SDLK_k           ,SDLK_k           },
/* 26  L     */ { SDLK_l           ,SDLK_l           ,SDLK_l           ,SDLK_l           },
/* 27  ; :   */ { SDLK_SEMICOLON   ,SDLK_SEMICOLON   ,SDLK_SEMICOLON   ,SDLK_SEMICOLON   },
/* 28  '     */ { SDLK_QUOTE       ,SDLK_QUOTE       ,SDLK_UNKNOWN     ,SDLK_QUOTE       },
/* 29  ` ~   */ { SDLK_BACKQUOTE   ,SDLK_BACKQUOTE   ,SDLK_UNKNOWN     ,SDLK_BACKQUOTE   },
/* 2A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 2B  \\    */ { SDLK_BACKSLASH   ,SDLK_BACKSLASH   ,SDLK_BACKSLASH   ,SDLK_BACKSLASH   },
/* 2C  Z     */ { SDLK_z           ,SDLK_z           ,SDLK_z           ,SDLK_z           },
/* 2D  X     */ { SDLK_x           ,SDLK_x           ,SDLK_x           ,SDLK_x           },
/* 2E  C     */ { SDLK_c           ,SDLK_c           ,SDLK_c           ,SDLK_c           },
/* 2F  V     */ { SDLK_v           ,SDLK_v           ,SDLK_v           ,SDLK_v           },
/* 30  B     */ { SDLK_b           ,SDLK_b           ,SDLK_b           ,SDLK_b           },
/* 31  N     */ { SDLK_n           ,SDLK_n           ,SDLK_n           ,SDLK_n           },
/* 32  M     */ { SDLK_m           ,SDLK_m           ,SDLK_m           ,SDLK_m           },
/* 33  , <   */ { SDLK_COMMA       ,SDLK_COMMA       ,SDLK_UNKNOWN     ,SDLK_COMMA       },
/* 34  . >   */ { SDLK_PERIOD      ,SDLK_PERIOD      ,SDLK_UNKNOWN     ,SDLK_PERIOD      },
/* 35  / ?   */ { SDLK_SLASH       ,SDLK_SLASH       ,SDLK_UNKNOWN     ,SDLK_SLASH       },
/* 36  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 37  Grey* */ { SDLK_KP_MULTIPLY ,SDLK_KP_MULTIPLY ,SDLK_UNKNOWN     ,SDLK_KP_MULTIPLY },
/* 38  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 39  Space */ { SDLK_SPACE       ,SDLK_SPACE       ,SDLK_SPACE       ,SDLK_SPACE       },
/* 3A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 3B  F1    */ { SDLK_F1          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 3C  F2    */ { SDLK_F2          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 3D  F3    */ { SDLK_F3          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 3E  F4    */ { SDLK_F4          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 3F  F5    */ { SDLK_F5          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 40  F6    */ { SDLK_F6          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 41  F7    */ { SDLK_F7          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 42  F8    */ { SDLK_F8          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 43  F9    */ { SDLK_F9          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 44  F10   */ { SDLK_F10         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 45  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 46  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 47  Home  */ { SDLK_HOME        ,SDLK_HOME        ,SDLK_UNKNOWN     ,SDLK_HOME        },
/* 48  Up    */ { SDLK_UP          ,SDLK_UP          ,SDLK_UNKNOWN     ,SDLK_UP          },
/* 49  PgUp  */ { SDLK_PAGEUP      ,SDLK_PAGEUP      ,SDLK_UNKNOWN     ,SDLK_PAGEUP      },
/* 4A  Grey- */ { SDLK_KP_MINUS    ,SDLK_KP_MINUS    ,SDLK_UNKNOWN     ,SDLK_KP_MINUS    },
/* 4B  Left  */ { SDLK_LEFT        ,SDLK_LEFT        ,SDLK_UNKNOWN     ,SDLK_LEFT        },
/* 4C  Kpad5 */ { SDLK_KP5         ,SDLK_KP5         ,SDLK_UNKNOWN     ,SDLK_KP5         },
/* 4D  Right */ { SDLK_RIGHT       ,SDLK_RIGHT       ,SDLK_UNKNOWN     ,SDLK_RIGHT       },
/* 4E  Grey+ */ { SDLK_KP_PLUS     ,SDLK_KP_PLUS     ,SDLK_UNKNOWN     ,SDLK_KP_PLUS     },
/* 4F  End   */ { SDLK_END         ,SDLK_END         ,SDLK_UNKNOWN     ,SDLK_END         },
/* 50  Down  */ { SDLK_DOWN        ,SDLK_DOWN        ,SDLK_UNKNOWN     ,SDLK_DOWN        },
/* 51  PgDn  */ { SDLK_PAGEDOWN    ,SDLK_PAGEDOWN    ,SDLK_UNKNOWN     ,SDLK_PAGEDOWN    },
/* 52  Ins   */ { SDLK_INSERT      ,SDLK_INSERT      ,SDLK_UNKNOWN     ,SDLK_INSERT      },
/* 53  Del   */ { SDLK_DELETE      ,SDLK_DELETE      ,SDLK_UNKNOWN     ,SDLK_DELETE      },
/* 54  ???   */ { SDLK_UNKNOWN     ,SDLK_F1          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 55  ???   */ { SDLK_UNKNOWN     ,SDLK_F2          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 56  Lft|  */ { SDLK_UNKNOWN     ,SDLK_F3          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 57  ???   */ { SDLK_UNKNOWN     ,SDLK_F4          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 58  ???   */ { SDLK_UNKNOWN     ,SDLK_F5          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 59  ???   */ { SDLK_UNKNOWN     ,SDLK_F6          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 5A  ???   */ { SDLK_UNKNOWN     ,SDLK_F7          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 5B  ???   */ { SDLK_UNKNOWN     ,SDLK_F8          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 5C  ???   */ { SDLK_UNKNOWN     ,SDLK_F9          ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 5D  ???   */ { SDLK_UNKNOWN     ,SDLK_F10         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 5E  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F1          ,SDLK_UNKNOWN     },
/* 5F  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F2          ,SDLK_UNKNOWN     },
/* 60  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F3          ,SDLK_UNKNOWN     },
/* 61  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F4          ,SDLK_UNKNOWN     },
/* 62  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F5          ,SDLK_UNKNOWN     },
/* 63  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F6          ,SDLK_UNKNOWN     },
/* 64  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F7          ,SDLK_UNKNOWN     },
/* 65  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F8          ,SDLK_UNKNOWN     },
/* 66  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F9          ,SDLK_UNKNOWN     },
/* 67  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F10         ,SDLK_UNKNOWN     },
/* 68  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F1          },
/* 69  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F2          },
/* 6A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F3          },
/* 6B  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F4          },
/* 6C  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F5          },
/* 6D  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F6          },
/* 6E  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F7          },
/* 6F  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F8          },
/* 70  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F9          },
/* 71  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F10         },
/* 72  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 73  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_LEFT        ,SDLK_UNKNOWN     },
/* 74  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_RIGHT       ,SDLK_UNKNOWN     },
/* 75  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_END         ,SDLK_UNKNOWN     },
/* 76  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_PAGEDOWN    ,SDLK_UNKNOWN     },
/* 77  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_HOME        ,SDLK_UNKNOWN     },
/* 78  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_1           },
/* 79  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_2           },
/* 7A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_3           },
/* 7B  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_4           },
/* 7C  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_5           },
/* 7D  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_6           },
/* 7E  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_7           },
/* 7F  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_8           },
/* 80  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_9           },
/* 81  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_0           },
/* 82  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_MINUS       },
/* 83  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_EQUALS      },
/* 84  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_PAGEUP      ,SDLK_UNKNOWN     },
/* 85  F11   */ { SDLK_F11         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 86  F12   */ { SDLK_F12         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 87  ???   */ { SDLK_UNKNOWN     ,SDLK_F11         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 88  ???   */ { SDLK_UNKNOWN     ,SDLK_F12         ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 89  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F11         ,SDLK_UNKNOWN     },
/* 8A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F12         ,SDLK_UNKNOWN     },
/* 8B  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F11         },
/* 8C  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_F12         },
/* 8D  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UP          ,SDLK_UNKNOWN     },
/* 8E  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_MINUS    ,SDLK_UNKNOWN     },
/* 8F  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP5         ,SDLK_UNKNOWN     },
/* 90  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_PLUS     ,SDLK_UNKNOWN     },
/* 91  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_DOWN        ,SDLK_UNKNOWN     },
/* 92  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_INSERT      ,SDLK_UNKNOWN     },
/* 93  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_DELETE      ,SDLK_UNKNOWN     },
/* 94  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_TAB         ,SDLK_UNKNOWN     },
/* 95  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_DIVIDE   ,SDLK_UNKNOWN     },
/* 96  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_MULTIPLY ,SDLK_UNKNOWN     },
/* 97  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_HOME        },
/* 98  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UP          },
/* 99  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_PAGEUP      },
/* 9A  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 9B  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_LEFT        },
/* 9C  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 9D  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_RIGHT       },
/* 9E  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* 9F  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_END         },
/* A0  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_DOWN        },
/* A1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_PAGEUP      },
/* A2  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_INSERT      },
/* A3  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_DELETE      },
/* A4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_DIVIDE   },
/* A5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_TAB         },
/* A6  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_KP_ENTER    },
/* A7  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* A8  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* A9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AD  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* AF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B0  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B2  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B3  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B6  Win L */ { SDLK_LSUPER      ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B7  Win R */ { SDLK_RSUPER      ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B8  Win M */ { SDLK_MENU        ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* B9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BD  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* BF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C0  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C2  ???   */ { SDLK_UNKNOWN     ,SDLK_LSUPER      ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C3  ???   */ { SDLK_UNKNOWN     ,SDLK_RSUPER      ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C6  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C7  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C8  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* C9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* CA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* CB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* CC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* CD  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* CE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_LSUPER      ,SDLK_UNKNOWN     },
/* CF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_RSUPER      ,SDLK_UNKNOWN     },
/* D0  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_MENU        ,SDLK_UNKNOWN     },
/* D1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D2  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D3  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D6  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D7  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D8  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* D9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* DA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_LSUPER      },
/* DB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_RSUPER      },
/* DC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_MENU        },
/* DD  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* DE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* DF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E0  Enter */ { SDLK_KP_ENTER    ,SDLK_KP_ENTER    ,SDLK_KP_ENTER    ,SDLK_UNKNOWN     },
/* E1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E2  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E3  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E6  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E7  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E8  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* E9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* EA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* EB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* EC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* ED  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* EE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* EF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F0  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F1  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F2  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F3  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F4  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F5  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F6  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F7  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F8  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* F9  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FA  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FB  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FC  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FD  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FE  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
/* FF  ???   */ { SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     ,SDLK_UNKNOWN     },
};

// Conversion de l'ancien codage des touches:
// 0x00FF le scancode    (maintenant code sym sur 0x0FFF)
// 0x0100 shift          (maintenant 0x1000)
// 0x0200 control        (maintenant 0x2000)
// 0x0400 alt            (maintenant 0x4000)
word Key_for_scancode(word scancode)
{
  if (scancode & 0x0400)
    return Scancode_to_sym[scancode & 0xFF][3] |
     (scancode & 0x0700) << 4;
  else if (scancode & 0x0200)
    return Scancode_to_sym[scancode & 0xFF][2] |
     (scancode & 0x0700) << 4;
  else if (scancode & 0x0100)
    return Scancode_to_sym[scancode & 0xFF][1] |
     (scancode & 0x0700) << 4;
  else
    return Scancode_to_sym[scancode & 0xFF][0];
}
#endif

#if defined(USE_SDL) || defined(USE_SDL2)

// Convertit des modificateurs de touches SDL en modificateurs GrafX2
word Get_Key_modifiers(void)
{
#if defined(USE_SDL)
  SDLMod mod;
#else
  SDL_Keymod mod;
#endif
  word modifiers = 0;
  mod = SDL_GetModState();

    if (mod & KMOD_CTRL)    // either one of the CTRL keys
      modifiers |= GFX2_MOD_CTRL;
    if (mod & KMOD_SHIFT )  // either one of the SHIFT keys
      modifiers |= GFX2_MOD_SHIFT;
    if (mod & (KMOD_ALT|KMOD_MODE)) // either ALT or ALTGR key
      modifiers |= GFX2_MOD_ALT;
#if defined(USE_SDL)
    if (mod & (KMOD_META))
#else
    if (mod & (KMOD_GUI))   // right or left Window key
#endif
      modifiers |= GFX2_MOD_META;

  return modifiers;
}

#if defined(USE_SDL)
word Keysym_to_keycode(SDL_keysym keysym)
#elif defined(USE_SDL2)
word Keysym_to_keycode(SDL_Keysym keysym)
#endif
{
  word key_code = 0;
  word mod;

  // On ignore shift, alt et control isolés.
  if (keysym.sym == SDLK_RSHIFT || keysym.sym == SDLK_LSHIFT ||
      keysym.sym == SDLK_RCTRL  || keysym.sym == SDLK_LCTRL ||
      keysym.sym == SDLK_RALT   || keysym.sym == SDLK_LALT ||
#if defined(USE_SDL2)
      keysym.sym == SDLK_RGUI  || keysym.sym == SDLK_LGUI ||
#else
      keysym.sym == SDLK_RMETA  || keysym.sym == SDLK_LMETA ||
#endif
      keysym.sym == SDLK_MODE) // AltGr
  return 0;

  // Les touches qui n'ont qu'une valeur unicode (très rares)
  // seront codées sur 11 bits, le 12e bit est mis à 1 (0x0800)
  if (keysym.sym != 0)
    key_code = K2K(keysym.sym);
  else if (keysym.scancode != 0)
  {
    key_code = (keysym.scancode & 0x07FF) | 0x0800;
  }

#if defined(USE_SDL)
  // Normally I should test keysym.mod here, but on windows the implementation
  // is buggy: if you release a modifier key, the following keys (when they repeat)
  // still name the original modifiers.
  mod = Get_Key_modifiers();
#else
  mod = 0;
  if (keysym.mod & KMOD_CTRL)
    mod |= GFX2_MOD_CTRL;
  if (keysym.mod & KMOD_SHIFT )
    mod |= GFX2_MOD_SHIFT;
  if (keysym.mod & (KMOD_ALT|KMOD_MODE))
    mod |= GFX2_MOD_ALT;
  if (keysym.mod & (KMOD_GUI))
    mod |= GFX2_MOD_META;
#endif

  // SDL_GetModState() seems to get the right up-to-date info.
  key_code |= mod;
  return key_code;
}
#endif

const char * Key_name(word key)
{
  typedef struct
  {
    word keysym;
    const char *Key_name;
  } T_key_label;
  static const T_key_label key_labels[] =
  {
#ifdef GCWZERO
    { SDLK_BACKSPACE   , "Right-SP" },
    { SDLK_TAB         , "Left-SP" },
    { SDLK_CLEAR       , "Clear" },
    { SDLK_RETURN      , "START" },
    { SDLK_PAUSE       , "Power-Down" },
    { SDLK_ESCAPE      , "SELECT" },
#else
    { KEY_BACKSPACE   , "Backspace" },
    { KEY_TAB         , "Tab" },
    { KEY_CLEAR       , "Clear" },
    { KEY_RETURN      , "Return" },
    { KEY_PAUSE       , "Pause" },
    { KEY_ESCAPE      , "Esc" },
#endif
    { KEY_DELETE      , "Del" },
    { KEY_KP0         , "KP 0" },
    { KEY_KP1         , "KP 1" },
    { KEY_KP2         , "KP 2" },
    { KEY_KP3         , "KP 3" },
    { KEY_KP4         , "KP 4" },
    { KEY_KP5         , "KP 5" },
    { KEY_KP6         , "KP 6" },
    { KEY_KP7         , "KP 7" },
    { KEY_KP8         , "KP 8" },
    { KEY_KP9         , "KP 9" },
    { KEY_KP_PERIOD   , "KP ." },
    { KEY_KP_DIVIDE   , "KP /" },
    { KEY_KP_MULTIPLY,  "KP *" },
    { KEY_KP_MINUS    , "KP -" },
    { KEY_KP_PLUS     , "KP +" },
    { KEY_KP_ENTER    , "KP Enter" },
    { KEY_KP_EQUALS   , "KP =" },
    { KEY_UP          , "Up" },
    { KEY_DOWN        , "Down" },
    { KEY_RIGHT       , "Right" },
    { KEY_LEFT        , "Left" },
    { KEY_INSERT      , "Ins" },
    { KEY_HOME        , "Home" },
    { KEY_END         , "End" },
    { KEY_PAGEUP      , "PgUp" },
    { KEY_PAGEDOWN    , "PgDn" },
    { KEY_F1          , "F1" },
    { KEY_F2          , "F2" },
    { KEY_F3          , "F3" },
    { KEY_F4          , "F4" },
    { KEY_F5          , "F5" },
    { KEY_F6          , "F6" },
    { KEY_F7          , "F7" },
    { KEY_F8          , "F8" },
    { KEY_F9          , "F9" },
    { KEY_F10         , "F10" },
    { KEY_F11         , "F11" },
    { KEY_F12         , "F12" },
#if defined(USE_SDL)
    { SDLK_F13         , "F13" },
    { SDLK_F14         , "F14" },
    { SDLK_F15         , "F15" },
    { SDLK_NUMLOCK     , "NumLock" },
#endif
    { KEY_CAPSLOCK    , "CapsLck" },
    { KEY_SCROLLOCK   , "ScrlLock" },
    { KEY_RSHIFT      , "RShift" },
#ifdef GCWZERO
    { SDLK_LSHIFT      , "X" },
    { SDLK_RCTRL       , "RCtrl" },
    { SDLK_LCTRL       , "A" },
    { SDLK_RALT        , "RAlt" },
    { SDLK_LALT        , "B" },
#else
    { KEY_LSHIFT      , "LShift" },
    { KEY_RCTRL       , "RCtrl" },
    { KEY_LCTRL       , "LCtrl" },
    { KEY_RALT        , "RAlt" },
    { KEY_LALT        , "LAlt" },
#endif
    { KEY_MENU        , "Menu" },
#if defined(USE_SDL)
    { SDLK_RMETA       , "RMeta" },
    { SDLK_LMETA       , "LMeta" },
    { SDLK_LSUPER      , "LWin" },
    { SDLK_RSUPER      , "RWin" },
    { SDLK_MODE        , "AltGr" },
    { SDLK_COMPOSE     , "Comp" },
    { SDLK_HELP        , "Help" },
    { SDLK_PRINT       , "Print" },
    { SDLK_SYSREQ      , "SysReq" },
    { SDLK_BREAK       , "Break" },
    { SDLK_POWER       , "Power" },
    { SDLK_EURO        , "Euro" },
    { SDLK_UNDO        , "Undo" },
#endif
    { KEY_MOUSEMIDDLE, "Mouse3" },
    { KEY_MOUSEWHEELUP, "WheelUp" },
    { KEY_MOUSEWHEELDOWN, "WheelDown" },
#if !defined(USE_SDL) && !defined(USE_SDL2) && defined(WIN32)
    /* windows VK_* codes which translate to ASCII characters are
     * not using the ASCII code, except for digit and letters */
    { KEY_COMMA,       "','" },
    { KEY_PERIOD,      "'.'" },
    { KEY_EQUALS,      "'='" },
    { KEY_MINUS,       "'-'" },
    { VK_OEM_1,        "';'" },
    { VK_OEM_2,        "'/'" },
    { KEY_BACKQUOTE,   "'`'" }, // VK_OEM_3
    { KEY_LEFTBRACKET, "'['" }, // VK_OEM_4
    { VK_OEM_5,        "'\\'"},
    { KEY_RIGHTBRACKET,"']'" }, // VK_OEM_6
    { VK_OEM_7,        "'''" },
    { VK_OEM_8,        "'!'" },
    { VK_OEM_102,      "<>" },
    /* special keys, such as browser navigation keys,
     * play/pause, sound volume, etc. */
#if defined(VK_BROWSER_BACK)
    { VK_BROWSER_BACK,    "Browser Back" },
    { VK_BROWSER_FORWARD, "Browser Forward" },
    { VK_BROWSER_REFRESH, "Browser Refresh" },
    { VK_BROWSER_STOP,    "Browser Stop" },
    { VK_BROWSER_SEARCH,  "Browser Search" },
    { VK_BROWSER_FAVORITES, "Browser Favorites" },
    { VK_BROWSER_HOME,    "Browser Home" },
#endif
#if defined(VK_VOLUME_MUTE)
    { VK_VOLUME_MUTE,      "Volume Mute" },
    { VK_VOLUME_DOWN,      "Volume Down" },
    { VK_VOLUME_UP,        "Volume Up" },
#endif
#if defined(VK_MEDIA_NEXT_TRACK)
    { VK_MEDIA_NEXT_TRACK, "Next Track" },
    { VK_MEDIA_PREV_TRACK, "Previous Track" },
    { VK_MEDIA_STOP,       "Stop" },
    { VK_MEDIA_PLAY_PAUSE, "Play/Pause" },
#endif
#if defined(VK_LAUNCH_MAIL)
    { VK_LAUNCH_MAIL,      "Email" },
    { VK_LAUNCH_MEDIA_SELECT, "Select Media" },
    { VK_LAUNCH_APP1,      "Start App1" },
    { VK_LAUNCH_APP2,      "Start App2" },
#endif
#endif
  };

  int index;
  static char buffer[41];
  buffer[0] = '\0';

  if (key == KEY_UNKNOWN)
    return "None";

#ifdef GCWZERO
  if (key & GFX2_MOD_CTRL)
    strcat(buffer, "A+");
  if (key & GFX2_MOD_ALT)
    strcat(buffer, "B+");
  if (key & GFX2_MOD_SHIFT)
    strcat(buffer, "X+");
#else
  if (key & GFX2_MOD_CTRL)
    strcat(buffer, "Ctl+");
  if (key & GFX2_MOD_ALT)
    strcat(buffer, "Alt+");
  if (key & GFX2_MOD_SHIFT)
    strcat(buffer, "Shift+");
#endif
  if (key & GFX2_MOD_META)
    strcat(buffer, META_KEY_PREFIX);

  key=key & ~(GFX2_MOD_CTRL|GFX2_MOD_ALT|GFX2_MOD_SHIFT|GFX2_MOD_META);

  // 99 is only a sanity check
  if (key>=KEY_JOYBUTTON && key<=KEY_JOYBUTTON+99)
  {

    const char *button_name;
    switch(key-KEY_JOYBUTTON)
    {
      #ifdef JOY_BUTTON_UP
      case JOY_BUTTON_UP: button_name="[UP]"; break;
      #endif
      #ifdef JOY_BUTTON_DOWN
      case JOY_BUTTON_DOWN: button_name="[DOWN]"; break;
      #endif
      #ifdef JOY_BUTTON_LEFT
      case JOY_BUTTON_LEFT: button_name="[LEFT]"; break;
      #endif
      #ifdef JOY_BUTTON_RIGHT
      case JOY_BUTTON_RIGHT: button_name="[RIGHT]"; break;
      #endif
      #ifdef JOY_BUTTON_UPLEFT
      case JOY_BUTTON_UPLEFT: button_name="[UP-LEFT]"; break;
      #endif
      #ifdef JOY_BUTTON_UPRIGHT
      case JOY_BUTTON_UPRIGHT: button_name="[UP-RIGHT]"; break;
      #endif
      #ifdef JOY_BUTTON_DOWNLEFT
      case JOY_BUTTON_DOWNLEFT: button_name="[DOWN-LEFT]"; break;
      #endif
      #ifdef JOY_BUTTON_DOWNRIGHT
      case JOY_BUTTON_DOWNRIGHT: button_name="[DOWN-RIGHT]"; break;
      #endif
      #ifdef JOY_BUTTON_CLICK
      case JOY_BUTTON_CLICK: button_name="[CLICK]"; break;
      #endif
      #ifdef JOY_BUTTON_A
      case JOY_BUTTON_A: button_name="[A]"; break;
      #endif
      #ifdef JOY_BUTTON_B
      case JOY_BUTTON_B: button_name="[B]"; break;
      #endif
      #ifdef JOY_BUTTON_X
      case JOY_BUTTON_X: button_name="[X]"; break;
      #endif
      #ifdef JOY_BUTTON_Y
      case JOY_BUTTON_Y: button_name="[Y]"; break;
      #endif
      #ifdef JOY_BUTTON_L
      case JOY_BUTTON_L: button_name="[L]"; break;
      #endif
      #ifdef JOY_BUTTON_R
      case JOY_BUTTON_R: button_name="[R]"; break;
      #endif
      #ifdef JOY_BUTTON_START
      case JOY_BUTTON_START: button_name="[START]"; break;
      #endif
      #ifdef JOY_BUTTON_SELECT
      case JOY_BUTTON_SELECT: button_name="[SELECT]"; break;
      #endif
      #ifdef JOY_BUTTON_VOLUP
      case JOY_BUTTON_VOLUP: button_name="[VOL UP]"; break;
      #endif
      #ifdef JOY_BUTTON_VOLDOWN
      case JOY_BUTTON_VOLDOWN: button_name="[VOL DOWN]"; break;
      #endif
      #ifdef JOY_BUTTON_MENU
      case JOY_BUTTON_MENU: button_name="[MENU]"; break;
      #endif
      #ifdef JOY_BUTTON_HOME
      case JOY_BUTTON_HOME: button_name="[HOME]"; break;
      #endif
      #ifdef JOY_BUTTON_HOLD
      case JOY_BUTTON_HOLD: button_name="[HOLD]"; break;
      #endif
      #ifdef JOY_BUTTON_I
      case JOY_BUTTON_I: button_name="[BUTTON I]"; break;
      #endif
      #ifdef JOY_BUTTON_II
      case JOY_BUTTON_II: button_name="[BUTTON II]"; break;
      #endif
      #ifdef JOY_BUTTON_JOY
      case JOY_BUTTON_JOY: button_name="[THUMB JOY]"; break;
      #endif

      default: sprintf(buffer+strlen(buffer), "[B%d]", key-KEY_JOYBUTTON);return buffer;
    }
    strcat(buffer,button_name);

    return buffer;
  }

  // Keys with a known label
  for (index=0; index < (long)sizeof(key_labels)/(long)sizeof(T_key_label);index++)
  {
    if (key == key_labels[index].keysym)
    {
      sprintf(buffer+strlen(buffer), "%s", key_labels[index].Key_name);
      return buffer;
    }
  }

  // ASCII characters keys
  if (key>=' ' && key < 127)
  {
    sprintf(buffer+strlen(buffer), "'%c'", toupper(key));
    return buffer;
  }

#if defined(USE_SDL)
  // 'World' keys
  if (key>=SDLK_WORLD_0 && key <= SDLK_WORLD_95)
  {
    sprintf(buffer+strlen(buffer), "w%d", key - SDLK_WORLD_0);
    return buffer;
  }
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
  if (key & 0x800)
  {
    // print Scancode
    sprintf(buffer+strlen(buffer), "[%d]", key & 0x7FF);
    return buffer;
  }
#endif

  // Unknown keys
  sprintf(buffer+strlen(buffer), "0x%X", key);
  return buffer;
}

#if defined(USE_SDL)
// Obtient le caractère ANSI tapé, à partir d'un keysym.
// (Valeur 32 à 255)
// Renvoie 0 s'il n'y a pas de caractère associé (shift, backspace, etc)
word Keysym_to_ANSI(SDL_keysym keysym)
{
  // This part was removed from the MacOSX port, but I put it back for others
  // as on Linux and Windows, it's what allows editing a text line with the keys
  // SDLK_LEFT, SDLK_RIGHT, SDLK_HOME, SDLK_END etc.
  #if !defined(__macosx__)
  if ( keysym.unicode == 0)
  {

    switch(keysym.sym)
    {
      case SDLK_DELETE:
      case SDLK_LEFT:
      case SDLK_RIGHT:
      case SDLK_HOME:
      case SDLK_END:
      case SDLK_BACKSPACE:
      case KEY_ESC:
        return keysym.sym;
      case SDLK_RETURN:
        // Case alt-enter
        if (SDL_GetModState() & (KMOD_ALT|KMOD_META))
          return '\n';
        return keysym.sym;
      default:
        return 0;
    }
  }
  #endif
  //
  if ( keysym.unicode > 32 && keysym.unicode < 127)
  {
    return keysym.unicode; // Pas de souci, on est en ASCII standard
  }

  // Quelques conversions Unicode-ANSI
  switch(keysym.unicode)
  {
    case 0x8100:
      return '\xfc'; // ü
    case 0x1A20:
      return '\xe9'; // é
    case 0x201A:
      return '\xe8'; // è
    case 0x9201:
      return '\xe2'; // â
    case 0x1E20:
      return '\xe4'; // ä
    case 0x2620:
      return '\xe0'; // à
    case 0x2020:
      return '\xe5'; // å
    case 0x2120:
      return '\xe7'; // ç
    case 0xC602:
      return '\xea'; // ê
    case 0x3020:
      return '\xeb'; // ë
    case 0x6001:
      return '\xe8'; // è
    case 0x3920:
      return '\xef'; // ï
    case 0x5201:
      return '\xee'; // î
    case 0x8D00:
      return '\xec'; // ì
    case 0x1C20:
      return '\xf4'; // ô
    case 0x1D20:
      return '\xf6'; // ö
    case 0x2220:
      return '\xf2'; // ò
    case 0x1320:
      return '\xfb'; // û
    case 0x1420:
      return '\xf9'; // ù
    case 0xDC02:
      return '\xff'; // ÿ
    case 0x5301:
      return '\xa3'; // £
    case 0xA000:
      return '\xe1'; // á
    case 0xA100:
      return '\xed'; // í
    case 0xA200:
      return '\xf3'; // ó
    case 0xA300:
      return '\xfa'; // ú
    case 0xA400:
      return '\xf1'; // ñ
    case 0xA700:
      return '\xba'; // º
    case 0xC600:
      return '\xe3'; // ã
    case 0x20AC:
      return '\x80';  // Euro sign is 20AC in unicode, 80 in CP1252
  }

  // Key entre 127 et 255
  if (keysym.unicode<256)
  {
#if defined(__macosx__) || defined(__FreeBSD__)
    // fc: Looks like there's a mismatch with delete & backspace
    //     i don't why SDLK_DELETE was returned instead of SDLK_BACKSPACE
    if(keysym.unicode == 127)
    {
        return(SDLK_BACKSPACE);
    }
    // We don't make any difference between return & enter in the app context.
    if(keysym.unicode == 3)
    {
        return(SDLK_RETURN);
    }
#endif
    return keysym.unicode;
  }

 // Sinon c'est une touche spéciale, on retourne son scancode
  return keysym.sym;
}
#elif defined(USE_SDL2)
word Keysym_to_ANSI(SDL_Keysym keysym)
{
  if (keysym.sym < 128)
    return (word)keysym.sym;
  return K2K(keysym.sym);
}

word Key_for_scancode(word scancode)
{
  return scancode;
}
#else
word Key_for_scancode(word scancode)
{
  return scancode;
}
word Get_Key_modifiers(void)
{
  word mod = 0;
#if defined(WIN32)
  if (GetKeyState(VK_SHIFT) & 0x8000)
    mod |= GFX2_MOD_SHIFT;
  // Pressing AltGr is reported by windows as the right Alt together with the Left control keys
  if ((GetKeyState(VK_RMENU) & 0x8000) && (GetKeyState(VK_LCONTROL) & 0x8000))
  {
    mod |= GFX2_MOD_ALT; // ALT GR
    if (GetKeyState(VK_RCONTROL) & 0x8000)
      mod |= GFX2_MOD_CTRL;
  }
  else
  {
    if (GetKeyState(VK_CONTROL) & 0x8000)
      mod |= GFX2_MOD_CTRL;
    if (GetKeyState(VK_MENU) & 0x8000)
      mod |= GFX2_MOD_ALT;
  }
  if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
    mod |= GFX2_MOD_META;
#elif defined(USE_X11)
  mod = X11_key_mod;
#else
  GFX2_Log(GFX2_WARNING, "Get_Key_modifiers() not implemented !\n");
#endif
  return mod;
}
#endif
