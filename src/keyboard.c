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
#include "global.h"
#include "keyboard.h"

#if defined(__macosx__)
  // Apple's "command" character is not present in the ANSI table, so we
  // recycled an ANSI value that doesn't have any displayable character
  // associated.
  #define GUI_KEY_PREFIX "\201"
#elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  // 'Amiga' key: an outlined uppercase A. Drawn on 2 unused characters.
  #define GUI_KEY_PREFIX "\215\216"
#else
  // All other platforms
  #define GUI_KEY_PREFIX "Gui+"
#endif

// Table de correspondance des scancode de clavier IBM PC AT vers
// les symboles de touches SDL (sym).
// La correspondance est bonne si le clavier est QWERTY US, ou si
// l'utilisateur est sous Windows.
// Dans l'ordre des colonnes: Normal, +Shift, +Control, +Alt
const word Scancode_to_sym[256][4] =
{
/* 00  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 01  Esc   */ { K2K(SDLK_ESCAPE)      ,K2K(SDLK_ESCAPE)      ,K2K(SDLK_ESCAPE)      ,K2K(SDLK_ESCAPE)      },  
/* 02  1 !   */ { K2K(SDLK_1)           ,K2K(SDLK_1)           ,KEY_NONE     ,KEY_NONE     },  
/* 03  2 @   */ { K2K(SDLK_2)           ,K2K(SDLK_2)           ,K2K(SDLK_2)           ,KEY_NONE     },  
/* 04  3 #   */ { K2K(SDLK_3)           ,K2K(SDLK_3)           ,KEY_NONE     ,KEY_NONE     },  
/* 05  4 $   */ { K2K(SDLK_4)           ,K2K(SDLK_4)           ,KEY_NONE     ,KEY_NONE     },  
/* 06  5 %   */ { K2K(SDLK_5)           ,K2K(SDLK_5)           ,KEY_NONE     ,KEY_NONE     },  
/* 07  6 ^   */ { K2K(SDLK_6)           ,K2K(SDLK_6)           ,K2K(SDLK_6)           ,KEY_NONE     },  
/* 08  7 &   */ { K2K(SDLK_7)           ,K2K(SDLK_7)           ,KEY_NONE     ,KEY_NONE     },  
/* 09  8 *   */ { K2K(SDLK_8)           ,K2K(SDLK_8)           ,K2K(SDLK_8)           ,KEY_NONE     },  
/* 0A  9 (   */ { K2K(SDLK_9)           ,K2K(SDLK_9)           ,KEY_NONE     ,KEY_NONE     },  
/* 0B  0 )   */ { K2K(SDLK_0)           ,K2K(SDLK_0)           ,KEY_NONE     ,KEY_NONE     },  
/* 0C  - _   */ { K2K(SDLK_MINUS)       ,K2K(SDLK_MINUS)       ,K2K(SDLK_MINUS)       ,KEY_NONE     },  
/* 0D  = +   */ { K2K(SDLK_EQUALS)      ,K2K(SDLK_EQUALS)      ,K2K(SDLK_EQUALS)      ,KEY_NONE     },  
/* 0E  BkSpc */ { K2K(SDLK_BACKSPACE)   ,K2K(SDLK_BACKSPACE)   ,K2K(SDLK_BACKSPACE)   ,K2K(SDLK_BACKSPACE)   },  
/* 0F  Tab   */ { K2K(SDLK_TAB)         ,K2K(SDLK_TAB)         ,KEY_NONE     ,KEY_NONE     },  
/* 10  Q     */ { K2K(SDLK_q)           ,K2K(SDLK_q)           ,K2K(SDLK_q)           ,K2K(SDLK_q)           },  
/* 11  W     */ { K2K(SDLK_w)           ,K2K(SDLK_w)           ,K2K(SDLK_w)           ,K2K(SDLK_w)           },  
/* 12  E     */ { K2K(SDLK_e)           ,K2K(SDLK_e)           ,K2K(SDLK_e)           ,K2K(SDLK_e)           },  
/* 13  R     */ { K2K(SDLK_r)           ,K2K(SDLK_r)           ,K2K(SDLK_r)           ,K2K(SDLK_r)           },  
/* 14  T     */ { K2K(SDLK_t)           ,K2K(SDLK_t)           ,K2K(SDLK_t)           ,K2K(SDLK_t)           },  
/* 15  Y     */ { K2K(SDLK_y)           ,K2K(SDLK_y)           ,K2K(SDLK_y)           ,K2K(SDLK_y)           },  
/* 16  U     */ { K2K(SDLK_u)           ,K2K(SDLK_u)           ,K2K(SDLK_u)           ,K2K(SDLK_u)           },  
/* 17  I     */ { K2K(SDLK_i)           ,K2K(SDLK_i)           ,K2K(SDLK_i)           ,K2K(SDLK_i)           },  
/* 18  O     */ { K2K(SDLK_o)           ,K2K(SDLK_o)           ,K2K(SDLK_o)           ,K2K(SDLK_o)           },  
/* 19  P     */ { K2K(SDLK_p)           ,K2K(SDLK_p)           ,K2K(SDLK_p)           ,K2K(SDLK_p)           },  
/* 1A  [     */ { K2K(SDLK_LEFTBRACKET) ,K2K(SDLK_LEFTBRACKET) ,K2K(SDLK_LEFTBRACKET) ,K2K(SDLK_LEFTBRACKET) },  
/* 1B  ]     */ { K2K(SDLK_RIGHTBRACKET),K2K(SDLK_RIGHTBRACKET),K2K(SDLK_RIGHTBRACKET),K2K(SDLK_RIGHTBRACKET)},  
/* 1C  Retrn */ { K2K(SDLK_RETURN)      ,K2K(SDLK_RETURN)      ,K2K(SDLK_RETURN)      ,K2K(SDLK_RETURN)      },  
/* 1D  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 1E  A     */ { K2K(SDLK_a)           ,K2K(SDLK_a)           ,K2K(SDLK_a)           ,K2K(SDLK_a)           },  
/* 1F  S     */ { K2K(SDLK_s)           ,K2K(SDLK_s)           ,K2K(SDLK_s)           ,K2K(SDLK_s)           },  
/* 20  D     */ { K2K(SDLK_d)           ,K2K(SDLK_d)           ,K2K(SDLK_d)           ,K2K(SDLK_d)           },  
/* 21  F     */ { K2K(SDLK_f)           ,K2K(SDLK_f)           ,K2K(SDLK_f)           ,K2K(SDLK_f)           },  
/* 22  G     */ { K2K(SDLK_g)           ,K2K(SDLK_g)           ,K2K(SDLK_g)           ,K2K(SDLK_g)           },  
/* 23  H     */ { K2K(SDLK_h)           ,K2K(SDLK_h)           ,K2K(SDLK_h)           ,K2K(SDLK_h)           },  
/* 24  J     */ { K2K(SDLK_j)           ,K2K(SDLK_j)           ,K2K(SDLK_j)           ,K2K(SDLK_j)           },  
/* 25  K     */ { K2K(SDLK_k)           ,K2K(SDLK_k)           ,K2K(SDLK_k)           ,K2K(SDLK_k)           },  
/* 26  L     */ { K2K(SDLK_l)           ,K2K(SDLK_l)           ,K2K(SDLK_l)           ,K2K(SDLK_l)           },  
/* 27  ; :   */ { K2K(SDLK_SEMICOLON)   ,K2K(SDLK_SEMICOLON)   ,K2K(SDLK_SEMICOLON)   ,K2K(SDLK_SEMICOLON)   },  
/* 28  '     */ { K2K(SDLK_QUOTE)       ,K2K(SDLK_QUOTE)       ,KEY_NONE     ,K2K(SDLK_QUOTE)       },  
/* 29  ` ~   */ { K2K(SDLK_BACKQUOTE)   ,K2K(SDLK_BACKQUOTE)   ,KEY_NONE     ,K2K(SDLK_BACKQUOTE)   },  
/* 2A  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 2B  \\    */ { K2K(SDLK_BACKSLASH)   ,K2K(SDLK_BACKSLASH)   ,K2K(SDLK_BACKSLASH)   ,K2K(SDLK_BACKSLASH)   },  
/* 2C  Z     */ { K2K(SDLK_z)           ,K2K(SDLK_z)           ,K2K(SDLK_z)           ,K2K(SDLK_z)           },  
/* 2D  X     */ { K2K(SDLK_x)           ,K2K(SDLK_x)           ,K2K(SDLK_x)           ,K2K(SDLK_x)           },  
/* 2E  C     */ { K2K(SDLK_c)           ,K2K(SDLK_c)           ,K2K(SDLK_c)           ,K2K(SDLK_c)           },  
/* 2F  V     */ { K2K(SDLK_v)           ,K2K(SDLK_v)           ,K2K(SDLK_v)           ,K2K(SDLK_v)           },  
/* 30  B     */ { K2K(SDLK_b)           ,K2K(SDLK_b)           ,K2K(SDLK_b)           ,K2K(SDLK_b)           },  
/* 31  N     */ { K2K(SDLK_n)           ,K2K(SDLK_n)           ,K2K(SDLK_n)           ,K2K(SDLK_n)           },  
/* 32  M     */ { K2K(SDLK_m)           ,K2K(SDLK_m)           ,K2K(SDLK_m)           ,K2K(SDLK_m)           },  
/* 33  , <   */ { K2K(SDLK_COMMA)       ,K2K(SDLK_COMMA)       ,KEY_NONE     ,K2K(SDLK_COMMA)       },  
/* 34  . >   */ { K2K(SDLK_PERIOD)      ,K2K(SDLK_PERIOD)      ,KEY_NONE     ,K2K(SDLK_PERIOD)      },  
/* 35  / ?   */ { K2K(SDLK_SLASH)       ,K2K(SDLK_SLASH)       ,KEY_NONE     ,K2K(SDLK_SLASH)       },  
/* 36  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 37  Grey* */ { K2K(SDLK_KP_MULTIPLY) ,K2K(SDLK_KP_MULTIPLY) ,KEY_NONE     ,K2K(SDLK_KP_MULTIPLY) },  
/* 38  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 39  Space */ { K2K(SDLK_SPACE)       ,K2K(SDLK_SPACE)       ,K2K(SDLK_SPACE)       ,K2K(SDLK_SPACE)       },  
/* 3A  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 3B  F1    */ { K2K(SDLK_F1)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 3C  F2    */ { K2K(SDLK_F2)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 3D  F3    */ { K2K(SDLK_F3)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 3E  F4    */ { K2K(SDLK_F4)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 3F  F5    */ { K2K(SDLK_F5)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 40  F6    */ { K2K(SDLK_F6)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 41  F7    */ { K2K(SDLK_F7)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 42  F8    */ { K2K(SDLK_F8)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 43  F9    */ { K2K(SDLK_F9)          ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 44  F10   */ { K2K(SDLK_F10)         ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 45  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 46  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 47  Home  */ { K2K(SDLK_HOME)        ,K2K(SDLK_HOME)        ,KEY_NONE     ,K2K(SDLK_HOME)        },  
/* 48  Up    */ { K2K(SDLK_UP)          ,K2K(SDLK_UP)          ,KEY_NONE     ,K2K(SDLK_UP)          },  
/* 49  PgUp  */ { K2K(SDLK_PAGEUP)      ,K2K(SDLK_PAGEUP)      ,KEY_NONE     ,K2K(SDLK_PAGEUP)      },  
/* 4A  Grey- */ { K2K(SDLK_KP_MINUS)    ,K2K(SDLK_KP_MINUS)    ,KEY_NONE     ,K2K(SDLK_KP_MINUS)    },  
/* 4B  Left  */ { K2K(SDLK_LEFT)        ,K2K(SDLK_LEFT)        ,KEY_NONE     ,K2K(SDLK_LEFT)        },  
/* 4C  Kpad5 */ { K2K(SDLK_KP_5)         ,K2K(SDLK_KP_5)         ,KEY_NONE     ,K2K(SDLK_KP_5)         },  
/* 4D  Right */ { K2K(SDLK_RIGHT)       ,K2K(SDLK_RIGHT)       ,KEY_NONE     ,K2K(SDLK_RIGHT)       },  
/* 4E  Grey+ */ { K2K(SDLK_KP_PLUS)     ,K2K(SDLK_KP_PLUS)     ,KEY_NONE     ,K2K(SDLK_KP_PLUS)     },  
/* 4F  End   */ { K2K(SDLK_END)         ,K2K(SDLK_END)         ,KEY_NONE     ,K2K(SDLK_END)         },  
/* 50  Down  */ { K2K(SDLK_DOWN)        ,K2K(SDLK_DOWN)        ,KEY_NONE     ,K2K(SDLK_DOWN)        },  
/* 51  PgDn  */ { K2K(SDLK_PAGEDOWN)    ,K2K(SDLK_PAGEDOWN)    ,KEY_NONE     ,K2K(SDLK_PAGEDOWN)    },  
/* 52  Ins   */ { K2K(SDLK_INSERT)      ,K2K(SDLK_INSERT)      ,KEY_NONE     ,K2K(SDLK_INSERT)      },  
/* 53  Del   */ { K2K(SDLK_DELETE)      ,K2K(SDLK_DELETE)      ,KEY_NONE     ,K2K(SDLK_DELETE)      },  
/* 54  ???   */ { KEY_NONE     ,K2K(SDLK_F1)          ,KEY_NONE     ,KEY_NONE     },  
/* 55  ???   */ { KEY_NONE     ,K2K(SDLK_F2)          ,KEY_NONE     ,KEY_NONE     },  
/* 56  Lft|  */ { KEY_NONE     ,K2K(SDLK_F3)          ,KEY_NONE     ,KEY_NONE     },  
/* 57  ???   */ { KEY_NONE     ,K2K(SDLK_F4)          ,KEY_NONE     ,KEY_NONE     },  
/* 58  ???   */ { KEY_NONE     ,K2K(SDLK_F5)          ,KEY_NONE     ,KEY_NONE     },  
/* 59  ???   */ { KEY_NONE     ,K2K(SDLK_F6)          ,KEY_NONE     ,KEY_NONE     },  
/* 5A  ???   */ { KEY_NONE     ,K2K(SDLK_F7)          ,KEY_NONE     ,KEY_NONE     },  
/* 5B  ???   */ { KEY_NONE     ,K2K(SDLK_F8)          ,KEY_NONE     ,KEY_NONE     },  
/* 5C  ???   */ { KEY_NONE     ,K2K(SDLK_F9)          ,KEY_NONE     ,KEY_NONE     },  
/* 5D  ???   */ { KEY_NONE     ,K2K(SDLK_F10)         ,KEY_NONE     ,KEY_NONE     },  
/* 5E  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F1)          ,KEY_NONE     },  
/* 5F  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F2)          ,KEY_NONE     },  
/* 60  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F3)          ,KEY_NONE     },  
/* 61  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F4)          ,KEY_NONE     },  
/* 62  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F5)          ,KEY_NONE     },  
/* 63  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F6)          ,KEY_NONE     },  
/* 64  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F7)          ,KEY_NONE     },  
/* 65  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F8)          ,KEY_NONE     },  
/* 66  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F9)          ,KEY_NONE     },  
/* 67  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F10)         ,KEY_NONE     },  
/* 68  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F1)          },  
/* 69  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F2)          },  
/* 6A  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F3)          },  
/* 6B  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F4)          },  
/* 6C  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F5)          },  
/* 6D  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F6)          },  
/* 6E  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F7)          },  
/* 6F  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F8)          },  
/* 70  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F9)          },  
/* 71  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F10)         },  
/* 72  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 73  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_LEFT)        ,KEY_NONE     },  
/* 74  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_RIGHT)       ,KEY_NONE     },  
/* 75  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_END)         ,KEY_NONE     },  
/* 76  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_PAGEDOWN)    ,KEY_NONE     },  
/* 77  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_HOME)        ,KEY_NONE     },  
/* 78  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_1)           },  
/* 79  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_2)           },  
/* 7A  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_3)           },  
/* 7B  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_4)           },  
/* 7C  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_5)           },  
/* 7D  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_6)           },  
/* 7E  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_7)           },  
/* 7F  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_8)           },  
/* 80  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_9)           },  
/* 81  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_0)           },  
/* 82  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_MINUS)       },  
/* 83  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_EQUALS)      },  
/* 84  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_PAGEUP)      ,KEY_NONE     },  
/* 85  F11   */ { K2K(SDLK_F11)         ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 86  F12   */ { K2K(SDLK_F12)         ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 87  ???   */ { KEY_NONE     ,K2K(SDLK_F11)         ,KEY_NONE     ,KEY_NONE     },  
/* 88  ???   */ { KEY_NONE     ,K2K(SDLK_F12)         ,KEY_NONE     ,KEY_NONE     },  
/* 89  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F11)         ,KEY_NONE     },  
/* 8A  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F12)         ,KEY_NONE     },  
/* 8B  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F11)         },  
/* 8C  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_F12)         },  
/* 8D  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_UP)          ,KEY_NONE     },  
/* 8E  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_MINUS)    ,KEY_NONE     },  
/* 8F  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_5)         ,KEY_NONE     },  
/* 90  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_PLUS)     ,KEY_NONE     },  
/* 91  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_DOWN)        ,KEY_NONE     },  
/* 92  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_INSERT)      ,KEY_NONE     },  
/* 93  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_DELETE)      ,KEY_NONE     },  
/* 94  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_TAB)         ,KEY_NONE     },  
/* 95  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_DIVIDE)   ,KEY_NONE     },  
/* 96  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_MULTIPLY) ,KEY_NONE     },  
/* 97  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_HOME)        },  
/* 98  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_UP)          },  
/* 99  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_PAGEUP)      },  
/* 9A  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 9B  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_LEFT)        },  
/* 9C  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 9D  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_RIGHT)       },  
/* 9E  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* 9F  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_END)         },  
/* A0  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_DOWN)        },  
/* A1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_PAGEUP)      },  
/* A2  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_INSERT)      },  
/* A3  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_DELETE)      },  
/* A4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_DIVIDE)   },  
/* A5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_TAB)         },  
/* A6  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_KP_ENTER)    },  
/* A7  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* A8  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* A9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AD  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AE  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* AF  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B0  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B2  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B3  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B6  Win L */ { K2K(SDLK_LGUI)      ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B7  Win R */ { K2K(SDLK_RGUI)      ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B8  Win M */ { K2K(SDLK_MENU)        ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* B9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BD  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BE  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* BF  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C0  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C2  ???   */ { KEY_NONE     ,K2K(SDLK_LGUI)      ,KEY_NONE     ,KEY_NONE     },  
/* C3  ???   */ { KEY_NONE     ,K2K(SDLK_RGUI)      ,KEY_NONE     ,KEY_NONE     },  
/* C4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C6  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C7  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C8  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* C9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* CA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* CB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* CC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* CD  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* CE  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_LGUI)      ,KEY_NONE     },  
/* CF  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_RGUI)      ,KEY_NONE     },  
/* D0  ???   */ { KEY_NONE     ,KEY_NONE     ,K2K(SDLK_MENU)        ,KEY_NONE     },  
/* D1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D2  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D3  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D6  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D7  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D8  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* D9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* DA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_LGUI)      },  
/* DB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_RGUI)      },  
/* DC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,K2K(SDLK_MENU)        },  
/* DD  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* DE  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* DF  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E0  Enter */ { K2K(SDLK_KP_ENTER)    ,K2K(SDLK_KP_ENTER)    ,K2K(SDLK_KP_ENTER)    ,KEY_NONE     },  
/* E1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E2  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E3  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E6  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E7  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E8  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* E9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* EA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* EB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* EC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* ED  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* EE  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* EF  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F0  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F1  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F2  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F3  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F4  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F5  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F6  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F7  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F8  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* F9  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FA  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FB  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FC  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FD  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FE  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
/* FF  ???   */ { KEY_NONE     ,KEY_NONE     ,KEY_NONE     ,KEY_NONE     },  
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

// Convertit des modificateurs de touches SDL en modificateurs GrafX2
word Key_modifiers(SDL_Keymod mod)
{
  word modifiers=0;
  
    if (mod & KMOD_CTRL )
      modifiers|=MOD_CTRL;
    if (mod & KMOD_SHIFT )
      modifiers|=MOD_SHIFT;
    if (mod & (KMOD_ALT|KMOD_MODE))
      modifiers|=MOD_ALT;
    if (mod & (KMOD_GUI))
      modifiers|=MOD_META;

  return modifiers;
}

word Keysym_to_keycode(SDL_Keysym keysym)
{
  word key_code = 0;
  word mod;

  // Ignore an isolate pressing of shift, alt and control
  if (keysym.sym == SDLK_RSHIFT || keysym.sym == SDLK_LSHIFT ||
      keysym.sym == SDLK_RCTRL  || keysym.sym == SDLK_LCTRL ||
      keysym.sym == SDLK_RALT   || keysym.sym == SDLK_LALT ||
      keysym.sym == SDLK_MODE) // AltGr
  return 0;
  
  key_code = K2K(keysym.sym);
  
  // Normally I should test keysym.mod here, but on windows the implementation
  // is buggy: if you release a modifier key, the following keys (when they repeat)
  // still name the original modifiers.
  mod=Key_modifiers(SDL_GetModState());

  // SDL_GetModState() seems to get the right up-to-date info.
  key_code |= mod;
  return key_code;
}

const char * Key_name(word key)
{
  typedef struct
  {
  word keysym;
  char *Key_name;
  } T_key_label;
  T_key_label key_labels[] =
  {
    { K2K(SDLK_BACKSPACE)   , "Backspace" },
    { K2K(SDLK_TAB)         , "Tab" },
    { K2K(SDLK_CLEAR)       , "Clear" },
    { K2K(SDLK_RETURN)      , "Return" },
    { K2K(SDLK_PAUSE)       , "Pause" },
    { K2K(SDLK_ESCAPE)      , "Esc" },
    { K2K(SDLK_DELETE)      , "Del" },
    { K2K(SDLK_KP_0)        , "KP 0" },
    { K2K(SDLK_KP_1)        , "KP 1" },
    { K2K(SDLK_KP_2)        , "KP 2" },
    { K2K(SDLK_KP_3)        , "KP 3" },
    { K2K(SDLK_KP_4)        , "KP 4" },
    { K2K(SDLK_KP_5)        , "KP 5" },
    { K2K(SDLK_KP_6)        , "KP 6" },
    { K2K(SDLK_KP_7)        , "KP 7" },
    { K2K(SDLK_KP_8)        , "KP 8" },
    { K2K(SDLK_KP_9)        , "KP 9" },
    { K2K(SDLK_KP_PERIOD)   , "KP ." },
    { K2K(SDLK_KP_DIVIDE)   , "KP /" },
    { K2K(SDLK_KP_MULTIPLY) , "KP *" },
    { K2K(SDLK_KP_MINUS)    , "KP -" },
    { K2K(SDLK_KP_PLUS)     , "KP +" },
    { K2K(SDLK_KP_ENTER)    , "KP Enter" },
    { K2K(SDLK_KP_EQUALS)   , "KP =" },
    { K2K(SDLK_UP)          , "Up" },
    { K2K(SDLK_DOWN)        , "Down" },
    { K2K(SDLK_RIGHT)       , "Right" },
    { K2K(SDLK_LEFT)        , "Left" },
    { K2K(SDLK_INSERT)      , "Ins" },
    { K2K(SDLK_HOME)        , "Home" },
    { K2K(SDLK_END)         , "End" },
    { K2K(SDLK_PAGEUP)      , "PgUp" },
    { K2K(SDLK_PAGEDOWN)    , "PgDn" },
    { K2K(SDLK_F1)          , "F1" },
    { K2K(SDLK_F2)          , "F2" },
    { K2K(SDLK_F3)          , "F3" },
    { K2K(SDLK_F4)          , "F4" },
    { K2K(SDLK_F5)          , "F5" },
    { K2K(SDLK_F6)          , "F6" },
    { K2K(SDLK_F7)          , "F7" },
    { K2K(SDLK_F8)          , "F8" },
    { K2K(SDLK_F9)          , "F9" },
    { K2K(SDLK_F10)         , "F10" },
    { K2K(SDLK_F11)         , "F11" },
    { K2K(SDLK_F12)         , "F12" },
    { K2K(SDLK_F13)         , "F13" },
    { K2K(SDLK_F14)         , "F14" },
    { K2K(SDLK_F15)         , "F15" },
    { K2K(SDLK_NUMLOCKCLEAR), "NumLock" },
    { K2K(SDLK_CAPSLOCK)    , "CapsLck" },
    { K2K(SDLK_SCROLLLOCK)  , "ScrlLock" },
    { K2K(SDLK_RSHIFT)      , "RShift" },
    { K2K(SDLK_LSHIFT)      , "LShift" },
    { K2K(SDLK_RCTRL)       , "RCtrl" },
    { K2K(SDLK_LCTRL)       , "LCtrl" },
    { K2K(SDLK_RALT)        , "RAlt" },
    { K2K(SDLK_LALT)        , "LAlt" },
    { K2K(SDLK_LGUI)        , "LWin" },
    { K2K(SDLK_RGUI)        , "RWin" },
    { K2K(SDLK_MODE)        , "AltGr" },
    { K2K(SDLK_APPLICATION) , "App" },
    { K2K(SDLK_HELP)        , "Help" },
    { K2K(SDLK_PRINTSCREEN) , "Print" },
    { K2K(SDLK_SYSREQ)      , "SysReq" },
    { K2K(SDLK_PAUSE)       , "Pause" },
    { K2K(SDLK_MENU)        , "Menu" },
    { K2K(SDLK_POWER)       , "Power" },
    { K2K(SDLK_UNDO)        , "Undo" },
    { KEY_MOUSEMIDDLE       , "Mouse3" },
    { KEY_MOUSEX1           , "Mouse4" },
    { KEY_MOUSEX2           , "Mouse5" },
    { KEY_MOUSEWHEELUP      , "WheelUp" },
    { KEY_MOUSEWHEELDOWN    , "WheelDown" },
    { KEY_MOUSEWHEELLEFT    , "WheelLeft" },
    { KEY_MOUSEWHEELRIGHT   , "WheelRight" },
  };

  int index;
  static char buffer[41];
  buffer[0] = '\0';

  if (key == KEY_NONE)
    return "None";
  
  if (key & MOD_CTRL)
    strcat(buffer, "Ctrl+");
  if (key & MOD_ALT)
    strcat(buffer, "Alt+");
  if (key & MOD_SHIFT)
    strcat(buffer, "Shift+");
  if (key & MOD_META)
    strcat(buffer, GUI_KEY_PREFIX);
  
  key=key & ~(MOD_CTRL|MOD_ALT|MOD_SHIFT);
  
  // Joystick button
  if (key & 0x0100)
  {
    char *button_name;
    switch(key & 0xFF)
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
      
      default: sprintf(buffer+strlen(buffer), "[B%d]", key&0xFF);return buffer;
    }
    strcat(buffer,button_name);

    return buffer;
  }
  // ASCII Key
  if ((key & 0xFF00) == 0 && (key & 0xFF) >= ' ' && (key & 0xFF) <= 0x7F)  
  {
    sprintf(buffer+strlen(buffer), "'%c'", toupper(key));
    return buffer;
  }
  /*
  // 'World' keys
  if (key>=K2K(SDLK_WORLD_0) && key <= K2K(SDLK_WORLD_95))
  {
    sprintf(buffer+strlen(buffer), "w%d", key - K2K(SDLK_WORLD_0);
    return buffer;
  }*/
  // Keys with a known label
  for (index=0; index < (long)sizeof(key_labels)/(long)sizeof(T_key_label);index++)
  {
    if (key == key_labels[index].keysym)
    {
      sprintf(buffer+strlen(buffer), "%s", key_labels[index].Key_name);
      return buffer;
    }
  }
  // Other unknown keys
  sprintf(buffer+strlen(buffer), "0x%X", key & 0x7FF);
  return buffer;

}


///
/// Get the first unicode character at the begininng of an UTF-8 string.
/// The character is written to 'character', and the function returns a
/// pointer to the next character. If an invalid utf-8 sequence is found,
/// the function returns NULL - it's unsafe to keep parsing from this point.
const char * Parse_utf8_string(const char * str, word *character)
{
  int number_bytes = 0;
  
  if ((str[0] & 0x80) == 0)
  {
   // 0xxxxxxx
    *character=str[0];
    number_bytes=1;
  }
  // 110xxxxx 10xxxxxx
  else if ((str[0] & 0xE0) == 0xC0)
  {
    *character=str[0] & 31;
    number_bytes=2;    
  }
  // 1110xxxx 10xxxxxx 10xxxxxx
  else if ((str[0] & 0xF0) == 0xE0)
  {
    *character=str[0] & 15;
    number_bytes=3;
  }
  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  else if ((str[0] & 0xF8) == 0xF0)
  {
    *character=str[0] & 7;
    number_bytes=4;
  }
  else
    return NULL;
  while(--number_bytes)
  {
    str++;
    if ((str[0] & 0xC0) != 0x80)
      return NULL;
    *character = (*character << 6) | (str[0] & 0x3F);    
  };
  // DEBUG
  printf("[%X]\n",*character);
  return str+1;
}

// Obtient le caractère ANSI tapé, à partir d'un keysym.
// (Valeur 32 à 255)
// Renvoie 0 s'il n'y a pas de caractère associé (shift, backspace, etc)
word Keysym_to_ANSI(SDL_Keysym keysym)
{
/*
  // This part was removed from the MacOSX port, but I put it back for others
  // as on Linux and Windows, it's what allows editing a text line with the keys
  // K2K(SDLK_LEFT), K2K(SDLK_RIGHT), K2K(SDLK_HOME), K2K(SDLK_END) etc.
  #if !(defined(__macosx__) || defined(__FreeBSD__))
  if ( keysym.unicode == 0)
  {

    switch(keysym.sym)
    {
      case K2K(SDLK_DELETE):
      case K2K(SDLK_LEFT):
      case K2K(SDLK_RIGHT):
      case K2K(SDLK_HOME):
      case K2K(SDLK_END):
      case K2K(SDLK_BACKSPACE):
      case KEY_ESC:
        return keysym.sym;
      case K2K(SDLK_RETURN):
        // Case alt-enter
        if (SDL_GetModState() & (KMOD_ALT|KMOD_GUI))
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
      return 'ü'; // ü
    case 0x1A20:
      return 'é'; // é
    case 0x201A:
      return 'è'; // è
    case 0x9201:
      return 'â'; // â
    case 0x1E20:
      return 'ä'; // ä
    case 0x2620:
      return 'à'; // à
    case 0x2020: 
      return 'å'; // å
    case 0x2120: 
      return 'ç'; // ç
    case 0xC602: 
      return 'ê'; // ê
    case 0x3020: 
      return 'ë'; // ë
    case 0x6001: 
      return 'è'; // è
    case 0x3920: 
      return 'ï'; // ï
    case 0x5201: 
      return 'î'; // î
    case 0x8D00: 
      return 'ì'; // ì
    case 0x1C20: 
      return 'ô'; // ô
    case 0x1D20: 
      return 'ö'; // ö
    case 0x2220: 
      return 'ò'; // ò
    case 0x1320: 
      return 'û'; // û
    case 0x1420: 
      return 'ù'; // ù
    case 0xDC02: 
      return 'ÿ'; // ÿ
    case 0x5301: 
      return '£'; // £
    case 0xA000: 
      return 'á'; // á
    case 0xA100: 
      return 'í'; // í
    case 0xA200: 
      return 'ó'; // ó
    case 0xA300: 
      return 'ú'; // ú
    case 0xA400: 
      return 'ñ'; // ñ
    case 0xA700: 
      return 'º'; // º
    case 0xC600: 
      return 'ã'; // ã
  }
  
  // Key entre 127 et 255
  if (keysym.unicode<256)
  {
#if defined(__macosx__) || defined(__FreeBSD__)
    // fc: Looks like there's a mismatch with delete & backspace
    //     i don't why SDLK_DELETE was returned instead of SDLK_BACKSPACE
    if(keysym.unicode == 127)
    {
        return(K2K(SDLK_BACKSPACE));
    }
    // We don't make any difference between return & enter in the app context.
    if(keysym.unicode == 3)
    {
        return(K2K(SDLK_RETURN));
    }
#endif
    return keysym.unicode;
  }

 // Sinon c'est une touche spéciale, on retourne son scancode
  return keysym.sym;
  */
  return 0;
}
