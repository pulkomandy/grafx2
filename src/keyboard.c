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
/* 00  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 01  Esc   */ { SDL_SCANCODE_ESCAPE      ,SDL_SCANCODE_ESCAPE      ,SDL_SCANCODE_ESCAPE      ,SDL_SCANCODE_ESCAPE      },  
/* 02  1 !   */ { SDL_SCANCODE_1           ,SDL_SCANCODE_1           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 03  2 @   */ { SDL_SCANCODE_2           ,SDL_SCANCODE_2           ,SDL_SCANCODE_2           ,SDL_SCANCODE_UNKNOWN     },  
/* 04  3 #   */ { SDL_SCANCODE_3           ,SDL_SCANCODE_3           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 05  4 $   */ { SDL_SCANCODE_4           ,SDL_SCANCODE_4           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 06  5 %   */ { SDL_SCANCODE_5           ,SDL_SCANCODE_5           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 07  6 ^   */ { SDL_SCANCODE_6           ,SDL_SCANCODE_6           ,SDL_SCANCODE_6           ,SDL_SCANCODE_UNKNOWN     },  
/* 08  7 &   */ { SDL_SCANCODE_7           ,SDL_SCANCODE_7           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 09  8 *   */ { SDL_SCANCODE_8           ,SDL_SCANCODE_8           ,SDL_SCANCODE_8           ,SDL_SCANCODE_UNKNOWN     },  
/* 0A  9 (   */ { SDL_SCANCODE_9           ,SDL_SCANCODE_9           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 0B  0 )   */ { SDL_SCANCODE_0           ,SDL_SCANCODE_0           ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 0C  - _   */ { SDL_SCANCODE_MINUS       ,SDL_SCANCODE_MINUS       ,SDL_SCANCODE_MINUS       ,SDL_SCANCODE_UNKNOWN     },  
/* 0D  = +   */ { SDL_SCANCODE_EQUALS      ,SDL_SCANCODE_EQUALS      ,SDL_SCANCODE_EQUALS      ,SDL_SCANCODE_UNKNOWN     },  
/* 0E  BkSpc */ { SDL_SCANCODE_BACKSPACE   ,SDL_SCANCODE_BACKSPACE   ,SDL_SCANCODE_BACKSPACE   ,SDL_SCANCODE_BACKSPACE   },  
/* 0F  Tab   */ { SDL_SCANCODE_TAB         ,SDL_SCANCODE_TAB         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 10  Q     */ { SDL_SCANCODE_Q           ,SDL_SCANCODE_Q           ,SDL_SCANCODE_Q           ,SDL_SCANCODE_Q           },  
/* 11  W     */ { SDL_SCANCODE_W           ,SDL_SCANCODE_W           ,SDL_SCANCODE_W           ,SDL_SCANCODE_W           },  
/* 12  E     */ { SDL_SCANCODE_E           ,SDL_SCANCODE_E           ,SDL_SCANCODE_E           ,SDL_SCANCODE_E           },  
/* 13  R     */ { SDL_SCANCODE_R           ,SDL_SCANCODE_R           ,SDL_SCANCODE_R           ,SDL_SCANCODE_R           },  
/* 14  T     */ { SDL_SCANCODE_T           ,SDL_SCANCODE_T           ,SDL_SCANCODE_T           ,SDL_SCANCODE_T           },  
/* 15  Y     */ { SDL_SCANCODE_Y           ,SDL_SCANCODE_Y           ,SDL_SCANCODE_Y           ,SDL_SCANCODE_Y           },  
/* 16  U     */ { SDL_SCANCODE_U           ,SDL_SCANCODE_U           ,SDL_SCANCODE_U           ,SDL_SCANCODE_U           },  
/* 17  I     */ { SDL_SCANCODE_I           ,SDL_SCANCODE_I           ,SDL_SCANCODE_I           ,SDL_SCANCODE_I           },  
/* 18  O     */ { SDL_SCANCODE_O           ,SDL_SCANCODE_O           ,SDL_SCANCODE_O           ,SDL_SCANCODE_O           },  
/* 19  P     */ { SDL_SCANCODE_P           ,SDL_SCANCODE_P           ,SDL_SCANCODE_P           ,SDL_SCANCODE_P           },  
/* 1A  [     */ { SDL_SCANCODE_LEFTBRACKET ,SDL_SCANCODE_LEFTBRACKET ,SDL_SCANCODE_LEFTBRACKET ,SDL_SCANCODE_LEFTBRACKET },  
/* 1B  ]     */ { SDL_SCANCODE_RIGHTBRACKET,SDL_SCANCODE_RIGHTBRACKET,SDL_SCANCODE_RIGHTBRACKET,SDL_SCANCODE_RIGHTBRACKET},  
/* 1C  Retrn */ { SDL_SCANCODE_RETURN      ,SDL_SCANCODE_RETURN      ,SDL_SCANCODE_RETURN      ,SDL_SCANCODE_RETURN      },  
/* 1D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 1E  A     */ { SDL_SCANCODE_A           ,SDL_SCANCODE_A           ,SDL_SCANCODE_A           ,SDL_SCANCODE_A           },  
/* 1F  S     */ { SDL_SCANCODE_S           ,SDL_SCANCODE_S           ,SDL_SCANCODE_S           ,SDL_SCANCODE_S           },  
/* 20  D     */ { SDL_SCANCODE_D           ,SDL_SCANCODE_D           ,SDL_SCANCODE_D           ,SDL_SCANCODE_D           },  
/* 21  F     */ { SDL_SCANCODE_F           ,SDL_SCANCODE_F           ,SDL_SCANCODE_F           ,SDL_SCANCODE_F           },  
/* 22  G     */ { SDL_SCANCODE_G           ,SDL_SCANCODE_G           ,SDL_SCANCODE_G           ,SDL_SCANCODE_G           },  
/* 23  H     */ { SDL_SCANCODE_H           ,SDL_SCANCODE_H           ,SDL_SCANCODE_H           ,SDL_SCANCODE_H           },  
/* 24  J     */ { SDL_SCANCODE_J           ,SDL_SCANCODE_J           ,SDL_SCANCODE_J           ,SDL_SCANCODE_J           },  
/* 25  K     */ { SDL_SCANCODE_K           ,SDL_SCANCODE_K           ,SDL_SCANCODE_K           ,SDL_SCANCODE_K           },  
/* 26  L     */ { SDL_SCANCODE_L           ,SDL_SCANCODE_L           ,SDL_SCANCODE_L           ,SDL_SCANCODE_L           },  
/* 27  ; :   */ { SDL_SCANCODE_SEMICOLON   ,SDL_SCANCODE_SEMICOLON   ,SDL_SCANCODE_SEMICOLON   ,SDL_SCANCODE_SEMICOLON   },  
/* 28  '     */ { SDL_SCANCODE_APOSTROPHE       ,SDL_SCANCODE_APOSTROPHE       ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_APOSTROPHE       },  
/* 29  ` ~   */ { SDL_SCANCODE_GRAVE   ,SDL_SCANCODE_GRAVE   ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_GRAVE   },  
/* 2A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 2B  \\    */ { SDL_SCANCODE_BACKSLASH   ,SDL_SCANCODE_BACKSLASH   ,SDL_SCANCODE_BACKSLASH   ,SDL_SCANCODE_BACKSLASH   },  
/* 2C  Z     */ { SDL_SCANCODE_Z           ,SDL_SCANCODE_Z           ,SDL_SCANCODE_Z           ,SDL_SCANCODE_Z           },  
/* 2D  X     */ { SDL_SCANCODE_X           ,SDL_SCANCODE_X           ,SDL_SCANCODE_X           ,SDL_SCANCODE_X           },  
/* 2E  C     */ { SDL_SCANCODE_C           ,SDL_SCANCODE_C           ,SDL_SCANCODE_C           ,SDL_SCANCODE_C           },  
/* 2F  V     */ { SDL_SCANCODE_V           ,SDL_SCANCODE_V           ,SDL_SCANCODE_V           ,SDL_SCANCODE_V           },  
/* 30  B     */ { SDL_SCANCODE_B           ,SDL_SCANCODE_B           ,SDL_SCANCODE_B           ,SDL_SCANCODE_B           },  
/* 31  N     */ { SDL_SCANCODE_N           ,SDL_SCANCODE_N           ,SDL_SCANCODE_N           ,SDL_SCANCODE_N           },  
/* 32  M     */ { SDL_SCANCODE_M           ,SDL_SCANCODE_M           ,SDL_SCANCODE_M           ,SDL_SCANCODE_M           },  
/* 33  , <   */ { SDL_SCANCODE_COMMA       ,SDL_SCANCODE_COMMA       ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_COMMA       },  
/* 34  . >   */ { SDL_SCANCODE_PERIOD      ,SDL_SCANCODE_PERIOD      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PERIOD      },  
/* 35  / ?   */ { SDL_SCANCODE_SLASH       ,SDL_SCANCODE_SLASH       ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_SLASH       },  
/* 36  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 37  Grey* */ { SDL_SCANCODE_KP_MULTIPLY ,SDL_SCANCODE_KP_MULTIPLY ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_MULTIPLY },  
/* 38  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 39  Space */ { SDL_SCANCODE_SPACE       ,SDL_SCANCODE_SPACE       ,SDL_SCANCODE_SPACE       ,SDL_SCANCODE_SPACE       },  
/* 3A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 3B  F1    */ { SDL_SCANCODE_F1          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 3C  F2    */ { SDL_SCANCODE_F2          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 3D  F3    */ { SDL_SCANCODE_F3          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 3E  F4    */ { SDL_SCANCODE_F4          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 3F  F5    */ { SDL_SCANCODE_F5          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 40  F6    */ { SDL_SCANCODE_F6          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 41  F7    */ { SDL_SCANCODE_F7          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 42  F8    */ { SDL_SCANCODE_F8          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 43  F9    */ { SDL_SCANCODE_F9          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 44  F10   */ { SDL_SCANCODE_F10         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 45  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 46  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 47  Home  */ { SDL_SCANCODE_HOME        ,SDL_SCANCODE_HOME        ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_HOME        },  
/* 48  Up    */ { SDL_SCANCODE_UP          ,SDL_SCANCODE_UP          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UP          },  
/* 49  PgUp  */ { SDL_SCANCODE_PAGEUP      ,SDL_SCANCODE_PAGEUP      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEUP      },  
/* 4A  Grey- */ { SDL_SCANCODE_KP_MINUS    ,SDL_SCANCODE_KP_MINUS    ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_MINUS    },  
/* 4B  Left  */ { SDL_SCANCODE_LEFT        ,SDL_SCANCODE_LEFT        ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LEFT        },  
/* 4C  Kpad5 */ { SDL_SCANCODE_KP_5         ,SDL_SCANCODE_KP_5         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_5         },  
/* 4D  Right */ { SDL_SCANCODE_RIGHT       ,SDL_SCANCODE_RIGHT       ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RIGHT       },  
/* 4E  Grey+ */ { SDL_SCANCODE_KP_PLUS     ,SDL_SCANCODE_KP_PLUS     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_PLUS     },  
/* 4F  End   */ { SDL_SCANCODE_END         ,SDL_SCANCODE_END         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_END         },  
/* 50  Down  */ { SDL_SCANCODE_DOWN        ,SDL_SCANCODE_DOWN        ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DOWN        },  
/* 51  PgDn  */ { SDL_SCANCODE_PAGEDOWN    ,SDL_SCANCODE_PAGEDOWN    ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEDOWN    },  
/* 52  Ins   */ { SDL_SCANCODE_INSERT      ,SDL_SCANCODE_INSERT      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_INSERT      },  
/* 53  Del   */ { SDL_SCANCODE_DELETE      ,SDL_SCANCODE_DELETE      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DELETE      },  
/* 54  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F1          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 55  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F2          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 56  Lft|  */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F3          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 57  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F4          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 58  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F5          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 59  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F6          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 5A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F7          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 5B  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F8          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 5C  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F9          ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 5D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F10         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 5E  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F1          ,SDL_SCANCODE_UNKNOWN     },  
/* 5F  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F2          ,SDL_SCANCODE_UNKNOWN     },  
/* 60  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F3          ,SDL_SCANCODE_UNKNOWN     },  
/* 61  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F4          ,SDL_SCANCODE_UNKNOWN     },  
/* 62  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F5          ,SDL_SCANCODE_UNKNOWN     },  
/* 63  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F6          ,SDL_SCANCODE_UNKNOWN     },  
/* 64  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F7          ,SDL_SCANCODE_UNKNOWN     },  
/* 65  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F8          ,SDL_SCANCODE_UNKNOWN     },  
/* 66  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F9          ,SDL_SCANCODE_UNKNOWN     },  
/* 67  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F10         ,SDL_SCANCODE_UNKNOWN     },  
/* 68  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F1          },  
/* 69  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F2          },  
/* 6A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F3          },  
/* 6B  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F4          },  
/* 6C  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F5          },  
/* 6D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F6          },  
/* 6E  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F7          },  
/* 6F  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F8          },  
/* 70  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F9          },  
/* 71  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F10         },  
/* 72  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 73  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LEFT        ,SDL_SCANCODE_UNKNOWN     },  
/* 74  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RIGHT       ,SDL_SCANCODE_UNKNOWN     },  
/* 75  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_END         ,SDL_SCANCODE_UNKNOWN     },  
/* 76  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEDOWN    ,SDL_SCANCODE_UNKNOWN     },  
/* 77  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_HOME        ,SDL_SCANCODE_UNKNOWN     },  
/* 78  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_1           },  
/* 79  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_2           },  
/* 7A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_3           },  
/* 7B  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_4           },  
/* 7C  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_5           },  
/* 7D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_6           },  
/* 7E  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_7           },  
/* 7F  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_8           },  
/* 80  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_9           },  
/* 81  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_0           },  
/* 82  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_MINUS       },  
/* 83  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_EQUALS      },  
/* 84  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEUP      ,SDL_SCANCODE_UNKNOWN     },  
/* 85  F11   */ { SDL_SCANCODE_F11         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 86  F12   */ { SDL_SCANCODE_F12         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 87  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F11         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 88  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F12         ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 89  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F11         ,SDL_SCANCODE_UNKNOWN     },  
/* 8A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F12         ,SDL_SCANCODE_UNKNOWN     },  
/* 8B  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F11         },  
/* 8C  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_F12         },  
/* 8D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UP          ,SDL_SCANCODE_UNKNOWN     },  
/* 8E  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_MINUS    ,SDL_SCANCODE_UNKNOWN     },  
/* 8F  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_5         ,SDL_SCANCODE_UNKNOWN     },  
/* 90  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_PLUS     ,SDL_SCANCODE_UNKNOWN     },  
/* 91  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DOWN        ,SDL_SCANCODE_UNKNOWN     },  
/* 92  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_INSERT      ,SDL_SCANCODE_UNKNOWN     },  
/* 93  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DELETE      ,SDL_SCANCODE_UNKNOWN     },  
/* 94  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_TAB         ,SDL_SCANCODE_UNKNOWN     },  
/* 95  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_DIVIDE   ,SDL_SCANCODE_UNKNOWN     },  
/* 96  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_MULTIPLY ,SDL_SCANCODE_UNKNOWN     },  
/* 97  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_HOME        },  
/* 98  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UP          },  
/* 99  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEUP      },  
/* 9A  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 9B  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LEFT        },  
/* 9C  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 9D  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RIGHT       },  
/* 9E  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* 9F  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_END         },  
/* A0  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DOWN        },  
/* A1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_PAGEUP      },  
/* A2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_INSERT      },  
/* A3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_DELETE      },  
/* A4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_DIVIDE   },  
/* A5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_TAB         },  
/* A6  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_KP_ENTER    },  
/* A7  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* A8  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* A9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AD  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* AF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B0  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B6  Win L */ { SDL_SCANCODE_LGUI      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B7  Win R */ { SDL_SCANCODE_RGUI      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B8  Win M */ { SDL_SCANCODE_MENU        ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* B9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BD  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* BF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C0  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LGUI      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RGUI      ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C6  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C7  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C8  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* C9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* CA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* CB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* CC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* CD  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* CE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LGUI      ,SDL_SCANCODE_UNKNOWN     },  
/* CF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RGUI      ,SDL_SCANCODE_UNKNOWN     },  
/* D0  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_MENU        ,SDL_SCANCODE_UNKNOWN     },  
/* D1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D6  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D7  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D8  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* D9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* DA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_LGUI      },  
/* DB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_RGUI      },  
/* DC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_MENU        },  
/* DD  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* DE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* DF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E0  Enter */ { SDL_SCANCODE_KP_ENTER    ,SDL_SCANCODE_KP_ENTER    ,SDL_SCANCODE_KP_ENTER    ,SDL_SCANCODE_UNKNOWN     },  
/* E1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E6  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E7  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E8  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* E9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* EA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* EB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* EC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* ED  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* EE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* EF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F0  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F1  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F2  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F3  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F4  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F5  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F6  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F7  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F8  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* F9  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FA  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FB  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FC  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FD  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FE  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
/* FF  ???   */ { SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     ,SDL_SCANCODE_UNKNOWN     },  
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

  // On ignore shift, alt et control isolés.
  if (keysym.sym == SDL_SCANCODE_RSHIFT || keysym.sym == SDL_SCANCODE_LSHIFT ||
      keysym.sym == SDL_SCANCODE_RCTRL  || keysym.sym == SDL_SCANCODE_LCTRL ||
      keysym.sym == SDL_SCANCODE_RALT   || keysym.sym == SDL_SCANCODE_LALT ||
      keysym.sym == SDL_SCANCODE_MODE) // AltGr
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
    { SDL_SCANCODE_BACKSPACE   , "Backspace" },
    { SDL_SCANCODE_TAB         , "Tab" },
    { SDL_SCANCODE_CLEAR       , "Clear" },
    { SDL_SCANCODE_RETURN      , "Return" },
    { SDL_SCANCODE_PAUSE       , "Pause" },
    { SDL_SCANCODE_ESCAPE      , "Esc" },
    { SDL_SCANCODE_DELETE      , "Del" },
    { SDL_SCANCODE_KP_0        , "KP 0" },
    { SDL_SCANCODE_KP_1        , "KP 1" },
    { SDL_SCANCODE_KP_2        , "KP 2" },
    { SDL_SCANCODE_KP_3        , "KP 3" },
    { SDL_SCANCODE_KP_4        , "KP 4" },
    { SDL_SCANCODE_KP_5        , "KP 5" },
    { SDL_SCANCODE_KP_6        , "KP 6" },
    { SDL_SCANCODE_KP_7        , "KP 7" },
    { SDL_SCANCODE_KP_8        , "KP 8" },
    { SDL_SCANCODE_KP_9        , "KP 9" },
    { SDL_SCANCODE_KP_PERIOD   , "KP ." },
    { SDL_SCANCODE_KP_DIVIDE   , "KP /" },
    { SDL_SCANCODE_KP_MULTIPLY,  "KP *" },
    { SDL_SCANCODE_KP_MINUS    , "KP -" },
    { SDL_SCANCODE_KP_PLUS     , "KP +" },
    { SDL_SCANCODE_KP_ENTER    , "KP Enter" },
    { SDL_SCANCODE_KP_EQUALS   , "KP =" },
    { SDL_SCANCODE_UP          , "Up" },
    { SDL_SCANCODE_DOWN        , "Down" },
    { SDL_SCANCODE_RIGHT       , "Right" },
    { SDL_SCANCODE_LEFT        , "Left" },
    { SDL_SCANCODE_INSERT      , "Ins" },
    { SDL_SCANCODE_HOME        , "Home" },
    { SDL_SCANCODE_END         , "End" },
    { SDL_SCANCODE_PAGEUP      , "PgUp" },
    { SDL_SCANCODE_PAGEDOWN    , "PgDn" },
    { SDL_SCANCODE_F1          , "F1" },
    { SDL_SCANCODE_F2          , "F2" },
    { SDL_SCANCODE_F3          , "F3" },
    { SDL_SCANCODE_F4          , "F4" },
    { SDL_SCANCODE_F5          , "F5" },
    { SDL_SCANCODE_F6          , "F6" },
    { SDL_SCANCODE_F7          , "F7" },
    { SDL_SCANCODE_F8          , "F8" },
    { SDL_SCANCODE_F9          , "F9" },
    { SDL_SCANCODE_F10         , "F10" },
    { SDL_SCANCODE_F11         , "F11" },
    { SDL_SCANCODE_F12         , "F12" },
    { SDL_SCANCODE_F13         , "F13" },
    { SDL_SCANCODE_F14         , "F14" },
    { SDL_SCANCODE_F15         , "F15" },
    { SDL_SCANCODE_NUMLOCKCLEAR     , "NumLock" },
    { SDL_SCANCODE_CAPSLOCK    , "CapsLck" },
    { SDL_SCANCODE_SCROLLLOCK   , "ScrlLock" },
    { SDL_SCANCODE_RSHIFT      , "RShift" },
    { SDL_SCANCODE_LSHIFT      , "LShift" },
    { SDL_SCANCODE_RCTRL       , "RCtrl" },
    { SDL_SCANCODE_LCTRL       , "LCtrl" },
    { SDL_SCANCODE_RALT        , "RAlt" },
    { SDL_SCANCODE_LALT        , "LAlt" },
    { SDL_SCANCODE_LGUI      , "LWin" },
    { SDL_SCANCODE_RGUI      , "RWin" },
    { SDL_SCANCODE_MODE        , "AltGr" },
    { SDL_SCANCODE_APPLICATION     , "App" },
    { SDL_SCANCODE_HELP        , "Help" },
    { SDL_SCANCODE_PRINTSCREEN       , "Print" },
    { SDL_SCANCODE_SYSREQ      , "SysReq" },
    { SDL_SCANCODE_PAUSE       , "Pause" },
    { SDL_SCANCODE_MENU        , "Menu" },
    { SDL_SCANCODE_POWER       , "Power" },
    { SDL_SCANCODE_UNDO        , "Undo" },
    { KEY_MOUSEMIDDLE, "Mouse3" },
    { KEY_MOUSEWHEELUP, "WheelUp" },
    { KEY_MOUSEWHEELDOWN, "WheelDown" }
  };

  int index;
  static char buffer[41];
  buffer[0] = '\0';

  if (key == SDL_SCANCODE_UNKNOWN)
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
  
  // 99 is only a sanity check
  if (key>=KEY_JOYBUTTON && key<=KEY_JOYBUTTON+99)
  {
    
    char *button_name;
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
  /*
  if (key>=SDL_SCANCODE_WORLD_0 && key <= SDL_SCANCODE_WORLD_95)
  {
    sprintf(buffer+strlen(buffer), "w%d", key - SDL_SCANCODE_WORLD_0);
    return buffer;
  }*/
                             
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
word Keysym_to_ANSI(SDL_Keysym keysym)
{
/*
  // This part was removed from the MacOSX port, but I put it back for others
  // as on Linux and Windows, it's what allows editing a text line with the keys
  // SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_HOME, SDL_SCANCODE_END etc.
  #if !(defined(__macosx__) || defined(__FreeBSD__))
  if ( keysym.unicode == 0)
  {

    switch(keysym.sym)
    {
      case SDL_SCANCODE_DELETE:
      case SDL_SCANCODE_LEFT:
      case SDL_SCANCODE_RIGHT:
      case SDL_SCANCODE_HOME:
      case SDL_SCANCODE_END:
      case SDL_SCANCODE_BACKSPACE:
      case KEY_ESC:
        return keysym.sym;
      case SDL_SCANCODE_RETURN:
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
    //     i don't why SDL_SCANCODE_DELETE was returned instead of SDL_SCANCODE_BACKSPACE
    if(keysym.unicode == 127)
    {
        return(SDL_SCANCODE_BACKSPACE);
    }
    // We don't make any difference between return & enter in the app context.
    if(keysym.unicode == 3)
    {
        return(SDL_SCANCODE_RETURN);
    }
#endif
    return keysym.unicode;
  }

 // Sinon c'est une touche spéciale, on retourne son scancode
  return keysym.sym;
  */
  return 0;
}
