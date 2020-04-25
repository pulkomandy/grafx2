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
///@file hotkeys.h
/// Definition of the tables used by the keyboard shortcuts.
/// The actual data is in hotkeys.c
//////////////////////////////////////////////////////////////////////////////

#if !defined(__VBCC__) && !defined(_MSC_VER)
    #include <stdbool.h>
#else
    #define bool char
#endif

#define NB_SHORTCUTS 212   ///< Number of actions that can have a key combination associated to it.

/*** Types definitions and structs ***/

typedef struct
{
    word Number;           ///< Identifier for shortcut. This is a number starting from 0, which matches ::T_Config_shortcut_info.Number
    char Label[36];        ///< Text to show in the screen where you can edit the shortcut.
    char Explanation1[37]; ///< Explanation text (1/3) to show in the screen where you can edit the shortcut.
    char Explanation2[37]; ///< Explanation text (2/3) to show in the screen where you can edit the shortcut.
    char Explanation3[37]; ///< Explanation text (3/3) to show in the screen where you can edit the shortcut.
    bool Suppr;            ///< Boolean, true if the shortcut can be removed.
    word Key;              ///< Primary shortcut. Value is a keycode, see keyboard.h
    word Key2;             ///< Secondary shortcut. Value is a keycode, see keyboard.h
} T_Key_config;

/// Table with all the configurable shortcuts, whether they are for a menu button or a special action.
extern T_Key_config ConfigKey[NB_SHORTCUTS];
///
/// Translation table from a shortcut index to a shortcut identifier.
/// The value is either:
/// - 0x000 + special shortcut number
/// - 0x100 + button number (left click)
/// - 0x200 + button number (right click)
extern word Ordering[NB_SHORTCUTS];
