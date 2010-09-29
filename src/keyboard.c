/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2010 A Filyanov
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
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "keyboard.h"

// Table de correspondance des scancode de clavier IBM PC AT vers
// les symboles de touches SDL (sym).
// La correspondance est bonne si le clavier est QWERTY US, ou si
// l'utilisateur est sous Windows.
// Dans l'ordre des colonnes: Normal, +Shift, +Control, +Alt
const word Scancode_to_sym[256][4] =
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

// Convertit des modificateurs de touches SDL en modificateurs GrafX2
word Key_modifiers(SDLMod mod)
{
  word modifiers=0;
  
    if (mod & KMOD_CTRL )
      modifiers|=MOD_CTRL;
    if (mod & KMOD_SHIFT )
      modifiers|=MOD_SHIFT;
    if (mod & (KMOD_ALT|KMOD_MODE))
      modifiers|=MOD_ALT;
    if (mod & (KMOD_META))
      modifiers|=MOD_META;

  return modifiers;
}

word Keysym_to_keycode(SDL_keysym keysym)
{
  word key_code = 0;
  word mod;

  // On ignore shift, alt et control isolés.
  if (keysym.sym == SDLK_RSHIFT || keysym.sym == SDLK_LSHIFT ||
      keysym.sym == SDLK_RCTRL  || keysym.sym == SDLK_LCTRL ||
      keysym.sym == SDLK_RALT   || keysym.sym == SDLK_LALT ||
      keysym.sym == SDLK_RMETA  || keysym.sym == SDLK_LMETA ||
      keysym.sym == SDLK_MODE) // AltGr
  return 0;
  
  // Les touches qui n'ont qu'une valeur unicode (très rares)
  // seront codées sur 11 bits, le 12e bit est mis à 1 (0x0800)
  if (keysym.sym != 0)
    key_code = keysym.sym;
  else if (keysym.scancode != 0)
  {
    key_code = (keysym.scancode & 0x07FF) | 0x0800;
  }
  
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
    { SDLK_BACKSPACE   , "Backspace" },
    { SDLK_TAB         , "Tab" },
    { SDLK_CLEAR       , "Clear" },
    { SDLK_RETURN      , "Return" },
    { SDLK_PAUSE       , "Pause" },
    { SDLK_ESCAPE      , "Esc" },
    { SDLK_DELETE      , "Del" },
    { SDLK_KP0         , "KP 0" },
    { SDLK_KP1         , "KP 1" },
    { SDLK_KP2         , "KP 2" },
    { SDLK_KP3         , "KP 3" },
    { SDLK_KP4         , "KP 4" },
    { SDLK_KP5         , "KP 5" },
    { SDLK_KP6         , "KP 6" },
    { SDLK_KP7         , "KP 7" },
    { SDLK_KP8         , "KP 8" },
    { SDLK_KP9         , "KP 9" },
    { SDLK_KP_PERIOD   , "KP ." },
    { SDLK_KP_DIVIDE   , "KP /" },
    { SDLK_KP_MULTIPLY,  "KP *" },
    { SDLK_KP_MINUS    , "KP -" },
    { SDLK_KP_PLUS     , "KP +" },
    { SDLK_KP_ENTER    , "KP Enter" },
    { SDLK_KP_EQUALS   , "KP =" },
    { SDLK_UP          , "Up" },
    { SDLK_DOWN        , "Down" },
    { SDLK_RIGHT       , "Right" },
    { SDLK_LEFT        , "Left" },
    { SDLK_INSERT      , "Ins" },
    { SDLK_HOME        , "Home" },
    { SDLK_END         , "End" },
    { SDLK_PAGEUP      , "PgUp" },
    { SDLK_PAGEDOWN    , "PgDn" },
    { SDLK_F1          , "F1" },
    { SDLK_F2          , "F2" },
    { SDLK_F3          , "F3" },
    { SDLK_F4          , "F4" },
    { SDLK_F5          , "F5" },
    { SDLK_F6          , "F6" },
    { SDLK_F7          , "F7" },
    { SDLK_F8          , "F8" },
    { SDLK_F9          , "F9" },
    { SDLK_F10         , "F10" },
    { SDLK_F11         , "F11" },
    { SDLK_F12         , "F12" },
    { SDLK_F13         , "F13" },
    { SDLK_F14         , "F14" },
    { SDLK_F15         , "F15" },
    { SDLK_NUMLOCK     , "NumLock" },
    { SDLK_CAPSLOCK    , "CapsLck" },
    { SDLK_SCROLLOCK   , "ScrlLock" },
    { SDLK_RSHIFT      , "RShift" },
    { SDLK_LSHIFT      , "LShift" },
    { SDLK_RCTRL       , "RCtrl" },
    { SDLK_LCTRL       , "LCtrl" },
    { SDLK_RALT        , "RAlt" },
    { SDLK_LALT        , "LAlt" },
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
    { SDLK_MENU        , "Menu" },
    { SDLK_POWER       , "Power" },
    { SDLK_EURO        , "Euro" },
    { SDLK_UNDO        , "Undo" },
    { KEY_MOUSEMIDDLE, "Mouse3" },
    { KEY_MOUSEWHEELUP, "WheelUp" },
    { KEY_MOUSEWHEELDOWN, "WheelDown" }
  };

  int index;
  static char buffer[41];
  buffer[0] = '\0';

  if (key == SDLK_UNKNOWN)
    return "None";
  
  if (key & MOD_CTRL)
    strcat(buffer, "Ctrl+");
  if (key & MOD_ALT)
    strcat(buffer, "Alt+");
  if (key & MOD_SHIFT)
    strcat(buffer, "Shift+");
  if (key & MOD_META)
    strcat(buffer, "\201");
  // Note: Apple's "command" character is not present in the ANSI table, so we
  // recycled an ANSI value that doesn't have any displayable character
  // associated.
  
  
  key=key & ~(MOD_CTRL|MOD_ALT|MOD_SHIFT);
  
#if defined(__GP2X__)
  if (key>=KEY_JOYBUTTON && key<=KEY_JOYBUTTON+18)
  {
    
    char *button_name;
    switch(key-KEY_JOYBUTTON)
    {    
      case GP2X_BUTTON_UP: button_name="[UP]"; break;
      case GP2X_BUTTON_DOWN: button_name="[DOWN]"; break;
      case GP2X_BUTTON_LEFT: button_name="[LEFT]"; break;
      case GP2X_BUTTON_RIGHT: button_name="[RIGHT]"; break;
      case GP2X_BUTTON_UPLEFT: button_name="[UP-LEFT]"; break;
      case GP2X_BUTTON_UPRIGHT: button_name="[UP-RIGHT]"; break;
      case GP2X_BUTTON_DOWNLEFT: button_name="[DOWN-LEFT]"; break;
      case GP2X_BUTTON_DOWNRIGHT: button_name="[DOWN-RIGHT]"; break;
      case GP2X_BUTTON_CLICK: button_name="[CLICK]"; break;
      case GP2X_BUTTON_A: button_name="[A]"; break;
      case GP2X_BUTTON_B: button_name="[B]"; break;
      case GP2X_BUTTON_X: button_name="[X]"; break;
      case GP2X_BUTTON_Y: button_name="[Y]"; break;
      case GP2X_BUTTON_L: button_name="[L]"; break;
      case GP2X_BUTTON_R: button_name="[R]"; break;
      case GP2X_BUTTON_START: button_name="[START]"; break;
      case GP2X_BUTTON_SELECT: button_name="[SELECT]"; break;
      case GP2X_BUTTON_VOLUP: button_name="[VOL UP]"; break;
      case GP2X_BUTTON_VOLDOWN: button_name="[VOL DOWN]"; break;
      default: sprintf(buffer+strlen(buffer), "[B%d]", key);return buffer;
    }
    strcat(buffer,button_name);
#elif defined(__WIZ__)
  if (Key>=KEY_JOYBUTTON && Key<=KEY_JOYBUTTON+17)
  {
    char *button_name;
    switch(Key-KEY_JOYBUTTON)
    {
      case WIZ_BUTTON_A: button_name="[A]"; break;
      case WIZ_BUTTON_X: button_name="[X]"; break;
      case WIZ_BUTTON_B: button_name="[B]"; break;
      case WIZ_BUTTON_Y: button_name="[Y]"; break;
      case WIZ_BUTTON_L: button_name="[L]"; break;
      case WIZ_BUTTON_R: button_name="[R]"; break;
      case WIZ_BUTTON_MENU: button_name="[MENU]"; break;
      case WIZ_BUTTON_SELECT: button_name="[SELECT]"; break;
      case WIZ_BUTTON_VOLUP: button_name="[VOL UP]"; break;
      case WIZ_BUTTON_VOLDOWN: button_name="[VOL DOWN]"; break;
      default: sprintf(buffer+strlen(buffer), "[B%d]", Key);return buffer;
    }
    strcat(buffer,button_name);
#elif defined(__CAANOO__)
  if (Key>=KEY_JOYBUTTON && Key<=KEY_JOYBUTTON+10)
  {
    char *button_name;
    switch(Key-KEY_JOYBUTTON)
    {
      case CAANOO_BUTTON_A: button_name="[A]"; break;
      case CAANOO_BUTTON_X: button_name="[X]"; break;
      case CAANOO_BUTTON_B: button_name="[B]"; break;
      case CAANOO_BUTTON_Y: button_name="[Y]"; break;
      case CAANOO_BUTTON_L: button_name="[L]"; break;
      case CAANOO_BUTTON_R: button_name="[R]"; break;
      case CAANOO_BUTTON_HOME: button_name="[HOME]"; break;
      case CAANOO_BUTTON_HOLD: button_name="[HOLD]"; break;
      case CAANOO_BUTTON_I: button_name="[BUTTON I]"; break;
      case CAANOO_BUTTON_II: button_name="[BUTTON II]"; break;
      case CAANOO_BUTTON_JOY: button_name="[THUMB JOY]"; break;
      default: sprintf(buffer+strlen(buffer), "[B%d]", Key);return buffer;
    }
    strcat(buffer,button_name);
#else
  {
    sprintf(buffer+strlen(buffer), "[B%d]", key-KEY_JOYBUTTON);
#endif
    return buffer;
  }
  
  if (key & 0x800)
  {
    sprintf(buffer+strlen(buffer), "[%d]", key & 0x7FF);
    return buffer;
  }
  key = key & 0x7FF;
  // Touches ASCII
  if (key>=' ' && key < 127)
  {
    sprintf(buffer+strlen(buffer), "'%c'", toupper(key));
    return buffer;
  }
  // Touches 'World'
  if (key>=SDLK_WORLD_0 && key <= SDLK_WORLD_95)
  {
    sprintf(buffer+strlen(buffer), "w%d", key - SDLK_WORLD_0);
    return buffer;
  }
                             
  // Touches au libellé connu
  for (index=0; index < (long)sizeof(key_labels)/(long)sizeof(T_key_label);index++)
  {
    if (key == key_labels[index].keysym)
    {
      sprintf(buffer+strlen(buffer), "%s", key_labels[index].Key_name);
      return buffer;
    }
  }
  // Autres touches inconnues
  sprintf(buffer+strlen(buffer), "0x%X", key & 0x7FF);
  return buffer;

}

// Obtient le caractère ANSI tapé, à partir d'un keysym.
// (Valeur 32 à 255)
// Renvoie 0 s'il n'y a pas de caractère associé (shift, backspace, etc)
word Keysym_to_ANSI(SDL_keysym keysym)
{
  // This part was removed from the MacOSX port, but I put it back for others
  // as on Linux and Windows, it's what allows editing a text line with the keys
  // SDLK_LEFT, SDLK_RIGHT, SDLK_HOME, SDLK_END etc.
  #if !(defined(__macosx__) || defined(__FreeBSD__))
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
      case SDLK_RETURN:
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
