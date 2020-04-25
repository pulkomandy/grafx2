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
///@file input.h
/// Functions for mouse, keyboard and joystick input.
/// Joystick input is used to emulate mouse on platforms that don't have a
/// pointing device, ie: the GP2X.
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "keycodes.h"

///
/// This is the keyboard/mouse/joystick input polling function.
/// Returns 1 if a significant changed occurred, such as a mouse button pressed
/// or depressed, or a new keypress was in the keyboard buffer.
/// The latest input variables are held in ::Key, ::Key_ANSI, ::Key_UNICODE, ::Mouse_X, ::Mouse_Y, ::Mouse_K.
/// Note that ::Key, ::Key_ANSI and ::Key_UNICODE are not persistent, they will be reset to 0
/// on subsequent calls to ::Get_input().
int  Get_input(int sleep_time);

/// Returns true if the keycode has been set as a keyboard shortcut for the function.
int Is_shortcut(word key, word function);

/// Returns true if the function has any shortcut key.
int Has_shortcut(word function);

/// Adjust mouse sensitivity (and actual mouse input mode)
void Adjust_mouse_sensitivity(word fullscreen);

int Move_cursor_with_constraints(int x, int y);
int Handle_mouse_btn_change(void);

///
/// This holds the ID of the GUI control that the mouse
/// is manipulating. The input system will reset it to zero 
/// when mouse button is released, but it's the engine
/// that will record and retrieve a real control ID.
extern int Input_sticky_control;

///
/// State of the shortcut for panning (SPECIAL_HOLD_PAN) : pressed or not.
extern byte Pan_shortcut_pressed;

/// Allows locking movement to X or Y axis: 0=normal, 1=lock on next move, 2=locked horizontally, 3=locked vertically.
extern int Snap_axis;
/// For the :Snap_axis mode, sets the origin's point (in image coordinates)
extern int Snap_axis_origin_X;
/// For the :Snap_axis mode, sets the origin's point (in image coordinates)
extern int Snap_axis_origin_Y;

///
/// This malloced string is set when a drag-and-drop event
/// brings a file to Grafx2's window.
extern char * Drop_file_name;
extern word * Drop_file_name_unicode;

#if defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
enum X11_CLIPBOARD_TYPES {
  X11_CLIPBOARD_NONE,
  X11_CLIPBOARD_UNKNOWN,
  X11_CLIPBOARD_PNG,
  X11_CLIPBOARD_TIFF,
  X11_CLIPBOARD_URILIST,
  X11_CLIPBOARD_UTF8STRING
};
///
/// malloc'ed copy of the X11 clipboard
extern char * X11_clipboard;
extern unsigned long X11_clipboard_size;
extern enum X11_CLIPBOARD_TYPES X11_clipboard_type;
#endif


#if defined __HAIKU__
	#define SHORTCUT_COPY (KEY_c|GFX2_MOD_ALT)
#elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__) || defined(__macosx__)
	#define SHORTCUT_COPY (KEY_c|GFX2_MOD_META)
#else
	#define SHORTCUT_COPY (KEY_c|GFX2_MOD_CTRL)
#endif

#if defined __HAIKU__
	#define SHORTCUT_PASTE (KEY_v|GFX2_MOD_ALT)
#elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__) || defined(__macosx__)
	#define SHORTCUT_PASTE (KEY_v|GFX2_MOD_META)
#else
	#define SHORTCUT_PASTE (KEY_v|GFX2_MOD_CTRL)
#endif

#endif
