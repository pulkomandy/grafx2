/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2007 Adrien Destugues
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

//////////////////////////////////////////////////////////////////////////////
///@file keyboard.h
/// Functions to convert bewteen the SDL key formats and the keycode we use
/// in grafx2.
/// The keycode we're using is generalized to handle mouse and joystick shortcuts
/// as well. The format can be broken down as:
/// - 0x0000 + 00-FF : The SDL "sym" key number, for SDLK_ values less than 256
/// - 0x0800 + 000-1FF: The SDL key number based on scancode
/// - 0x0100 + N : Joystick button N
/// - 0x0200 : Mouse wheel up
/// - 0x0201 : Mouse wheel down
/// - 0x0202 : Mouse wheel left
/// - 0x0203 : Mouse wheel right
/// - 0x0210 : Mouse middle
/// - 0x0211 : Mouse x1
/// - 0x0212 : Mouse x2
/// - 0x0400 + 00 to FF : Joystick button N
/// Add 0x1000 for the Shift modifier MOD_SHIFT
/// Add 0x2000 for the Control modifier ::MOD_CONTROL
/// Add 0x4000 for the Alt modifier ::MOD_ALT
/// Add 0x8000 for the "Meta" modifier ::MOD_META (On MacOS X it's the CMD key)
//////////////////////////////////////////////////////////////////////////////

/*!
  Convert an SDL keysym to an internal keycode number.
  This is needed because SDL tends to split the information across the unicode sym, the regular sym, and the raw keycode.
  We also need to differenciate 1 (keypad) and 1 (regular keyboard), and some other things.
  See the notice at the beginning of keyboard.h for the format of a keycode.
  @param keysym SDL symbol to convert
*/
word Keysym_to_keycode(SDL_Keysym keysym);

/*!
    Helper function to convert between SDL system and the old coding for PC keycodes.
    This is only used to convert configuration files from the DOS version of 
    Grafx2, where keyboard codes are in in the IBM PC AT form.
  @param scancode Scancode to convert
*/
word Key_for_scancode(word scancode);

/*!
    Returns key name in a string. Used to display them in the helpscreens and in the keymapper window.
  @param Key keycode of the key to translate, including modifiers
*/
const char * Key_name(word key);

/*!
  Gets the modifiers in our format from the SDL_Mod information.
  Returns a combination of ::MOD_SHIFT, ::MOD_ALT, ::MOD_CONTROL
  @param mod SDL modifiers state
*/
word Key_modifiers(SDL_Keymod mod);

///
/// Get the first unicode character at the begininng of an UTF-8 string.
/// The character is written to 'character', and the function returns a
/// pointer to the next character. If an invalid utf-8 sequence is found,
/// the function returns NULL - it's unsafe to keep parsing from this point.
const char * Parse_utf8_string(const char * str, word *character);

