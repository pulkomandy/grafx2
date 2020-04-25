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
///@file keyboard.h
/// Functions to convert bewteen the SDL key formats and the keycode we use
/// in grafx2.
/// The keycode we're using is generalized to handle mouse and joystick shortcuts
/// as well. The format can be broken down as:
/// - 0x0000 + a number between 0 and SDLK_LAST (about 324) : the SDL "sym" key number.
/// - 0x0000 + SDLK_LAST+1: Mouse middle button.
/// - 0x0000 + SDLK_LAST+2: Mouse wheel up.
/// - 0x0000 + SDLK_LAST+3: Mouse wheel down.
/// - 0x0000 + SDLK_LAST+4+B : Joystick button number "B", starting at B=0.
/// - 0x0800 + a number between 0 and 0x7FF: The scancode key number, for keys which have no "sym", such as keys from multimedia keyboards, and "fn" and "Thinkpad" key for a laptop.
/// Add 0x1000 for the Shift modifier GFX2_MOD_SHIFT
/// Add 0x2000 for the Control modifier ::GFX2_MOD_CTRL
/// Add 0x4000 for the Alt modifier ::GFX2_MOD_ALT
/// Add 0x8000 for the "Meta" modifier ::GFX2_MOD_META (On MacOS X it's the CMD key)
//////////////////////////////////////////////////////////////////////////////
#if defined(USE_SDL)
#include <SDL.h>
#elif defined(USE_SDL2)
#include <SDL_keyboard.h>
#endif


#if defined(USE_SDL)
/*! Convert an SDL keysym to an ANSI/ASCII character.

  This is used to type text and numeric values in input boxes.

  @param[in] keysym SDL symbol to convert
*/
word Keysym_to_ANSI(SDL_keysym keysym);
#elif defined(USE_SDL2)
word Keysym_to_ANSI(SDL_Keysym keysym);
#endif

#if defined(USE_SDL)
/*! Convert an SDL keysym to an internal keycode number.

  This is needed because SDL tends to split the information across the unicode
  sym, the regular sym, and the raw keycode. We also need to differenciate
  1 (keypad) and 1 (regular keyboard), and some other things. See the notice
  at the beginning of keyboard.h for the format of a keycode.

  @param[in] keysym SDL symbol to convert
  @return internal keycode value
*/
word Keysym_to_keycode(SDL_keysym keysym);
#elif defined(USE_SDL2)
word Keysym_to_keycode(SDL_Keysym keysym);
#endif

/*! Helper function to convert between SDL system and the old coding for PC keycodes.
    This is only used to convert configuration files from the DOS version of
    Grafx2, where keyboard codes are in in the IBM PC AT form.
  @param scancode Scancode to convert
*/
word Key_for_scancode(word scancode);

/*! Returns key name in a string.
	
	Used to display them in the helpscreens and in the keymapper window.

    @param[in] key keycode of the key to translate, including modifiers
*/
const char * Key_name(word key);

/*!
  Gets the Key modifiers in our format
  Returns a combination of ::GFX2_MOD_SHIFT, ::GFX2_MOD_ALT, ::GFX2_MOD_CTRL
*/
word Get_Key_modifiers(void);
