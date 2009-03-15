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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*!
  Convert an SDL keysym to an ANSI/ASCII character.
  @param Sym SDL symbol to convert
*/
word Conversion_ANSI(SDL_keysym Sym);

/*!
  Convert an SDL keysym to an internal keycode number.
  This is needed because SDL tends to split the information across the unicode sym, the regular sym, and the raw keycode.
  We also need to differenciate 1 (keypad) and 1 (regular keyboard), and some other things.
  @param Sym SDL symbol to convert
*/
word Conversion_Touche(SDL_keysym Sym);

/*!
    Helper function to convert between SDL system and old coding for keycodes. This is needed because some SDL keycode are actually unicode and won't fit in 8 bits.
  @param scancode Scancode to convert
*/
word Touche_pour_scancode(word scancode);

/*!
    Returns key name in a string. Used to display them in the helpscreens and in the keymapper window.
  @param Touche keycode of the key to translate, including modifiers
*/
const char * Nom_touche(word Touche);

/*!
  Gets the modifiers in our format from the SDL_Mod information.
  @param Mod SDL modifiers state
*/
word Modificateurs_Touche(SDLMod Mod);

