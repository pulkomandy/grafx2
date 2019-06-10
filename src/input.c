/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2009 Franck Charlet
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

#include <string.h>
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#if (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11)) || defined(__WIN32__)
#include <SDL_syswm.h>
#endif
#endif

#ifdef WIN32
  #include <windows.h>
  #include <shellapi.h>
#endif
#if defined(_MSC_VER)
#define strdup _strdup
#endif

#ifdef USE_X11
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

#if defined(_MSC_VER)
#define strdup _strdup
#endif

#include "gfx2log.h"
#include "global.h"
#include "keyboard.h"
#include "screen.h"
#include "windows.h"
#include "errors.h"
#include "misc.h"
#include "buttons.h"
#include "input.h"
#include "loadsave.h"

#ifdef USE_X11
extern Display * X11_display;
extern Window X11_window;
#endif

#if defined(USE_SDL)
#define RSUPER_EMULATES_META_MOD
#endif
// Keyboards with a Super key never seem to have a Meta key at the same time.
// This setting allows the right 'Super' key (the one with a 'Windows' or
// 'Amiga' label to be used as a modifier instead of a normal key.
// This feature is especially useful for AROS where applications should use
// generic defaults like "Right Amiga+Q = Quit".
// In case this is annoying for some platforms, disable it.

static int Color_cycling(void);

// public Globals (available as extern)

int Input_sticky_control = 0;
int Snap_axis = 0;
int Snap_axis_origin_X;
int Snap_axis_origin_Y;

char * Drop_file_name = NULL;
word * Drop_file_name_unicode = NULL;

#if defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
char * X11_clipboard = NULL;
unsigned long X11_clipboard_size = 0;
enum X11_CLIPBOARD_TYPES X11_clipboard_type = X11_CLIPBOARD_NONE;
#endif

// --

// Digital joystick state
#define PAD_MAX_SPEED 3 /// Max speed per pixel when using the gamepad or keyboard directional keys
static byte Directional_click = 0;
static byte Digital_joystick_state = 0;
#define D_JOYSTICK_UP         (1 << 0)
#define D_JOYSTICK_UP_RIGHT   (1 << 1)
#define D_JOYSTICK_RIGHT      (1 << 2)
#define D_JOYSTICK_DOWN_RIGHT (1 << 3)
#define D_JOYSTICK_DOWN       (1 << 4)
#define D_JOYSTICK_DOWN_LEFT  (1 << 5)
#define D_JOYSTICK_LEFT       (1 << 6)
#define D_JOYSTICK_UP_LEFT    (1 << 7)

// Emulated directional controller.
// This has a distinct state from Directional_, because some joysticks send
// "I'm not moving" SDL events when idle, thus stopping the emulated one.
static byte Directional_emulated;

// Analog joystick state (mouse emulation)
#if defined(USE_JOYSTICK)
#define STICK_MAX_SPEED 3 // Max speed per pixel using the analog stick
long Joystick_vertical = 0; // 16.16 fixed point value
long Joystick_horizontal = 0; // 16.16 fixed point value
#endif

word Input_new_mouse_X;
word Input_new_mouse_Y;
word Input_new_mouse_X_frac;  // fractional part of Input_new_mouse_X
word Input_new_mouse_Y_frac;  // fractional part of Input_new_mouse_Y
byte Input_new_mouse_K;
byte Button_inverter=0; // State of the key that swaps mouse buttons.

long Directional_first_move;
long Directional_last_move;
int  Mouse_moved; ///< Boolean, Set to true if any cursor movement occurs.

byte Pan_shortcut_pressed;

// Joystick/pad configurations for the various console ports.
// See the #else for the documentation of fields.
// TODO: Make these user-settable somehow.
#if defined(__GP2X__)

  #define JOYSTICK_THRESHOLD  (4096)
  short Joybutton_shift=      JOY_BUTTON_L;
  short Joybutton_control=    JOY_BUTTON_R;
  short Joybutton_alt=        JOY_BUTTON_CLICK;
  short Joybutton_left_click= JOY_BUTTON_B;
  short Joybutton_right_click=JOY_BUTTON_Y;

#elif defined(__WIZ__)

  #define JOYSTICK_THRESHOLD  (4096)
  short Joybutton_shift=      JOY_BUTTON_X;
  short Joybutton_control=    JOY_BUTTON_SELECT;
  short Joybutton_alt=        JOY_BUTTON_Y;
  short Joybutton_left_click= JOY_BUTTON_A;
  short Joybutton_right_click=JOY_BUTTON_B;

#elif defined(__CAANOO__)

  #define JOYSTICK_THRESHOLD  (4096)
  short Joybutton_shift=      JOY_BUTTON_L;
  short Joybutton_control=    JOY_BUTTON_R;
  short Joybutton_alt=        JOY_BUTTON_Y;
  short Joybutton_left_click= JOY_BUTTON_A;
  short Joybutton_right_click=JOY_BUTTON_B;

#elif defined(__SWITCH__)

  #define JOYSTICK_THRESHOLD  (1024)
  short Joybutton_shift=      JOY_BUTTON_L;
  short Joybutton_control=    JOY_BUTTON_R;
  short Joybutton_alt=        JOY_BUTTON_Y;
  short Joybutton_left_click= JOY_BUTTON_A;
  short Joybutton_right_click=JOY_BUTTON_B;

#else // Default : Any joystick on a computer platform
  ///
  /// This is the sensitivity threshold for the directional
  /// pad of a cheap digital joypad on the PC. It has been set through
  /// trial and error : If value is too large then the movement is
  /// randomly interrupted; if the value is too low the cursor will
  /// move by itself, controlled by parasits.
  /// YR 04/11/2010: I just observed a -8700 when joystick is idle.
  #define JOYSTICK_THRESHOLD  (10000)
  
  /// A button that is marked as "modifier" will 
  short Joybutton_shift=-1; ///< Button number that serves as a "shift" modifier; -1 for none
  short Joybutton_control=-1; ///< Button number that serves as a "ctrl" modifier; -1 for none
  short Joybutton_alt=-1; ///< Button number that serves as a "alt" modifier; -1 for none
  
  short Joybutton_left_click=0; ///< Button number that serves as left click; -1 for none
  short Joybutton_right_click=1; ///< Button number that serves as right-click; -1 for none

#endif

int Has_shortcut(word function)
{
  if (function == 0xFFFF)
    return 0;
    
  if (function & 0x100)
  {
    if (Buttons_Pool[function&0xFF].Left_shortcut[0]!=KEY_NONE)
      return 1;
    if (Buttons_Pool[function&0xFF].Left_shortcut[1]!=KEY_NONE)
      return 1;
    return 0;
  }
  if (function & 0x200)
  {
    if (Buttons_Pool[function&0xFF].Right_shortcut[0]!=KEY_NONE)
      return 1;
    if (Buttons_Pool[function&0xFF].Right_shortcut[1]!=KEY_NONE)
      return 1;
    return 0;
  }
  if(Config_Key[function][0]!=KEY_NONE)
    return 1;
  if(Config_Key[function][1]!=KEY_NONE)
    return 1;
  return 0; 
}

int Is_shortcut(word key, word function)
{
  if (key == 0 || function == 0xFFFF)
    return 0;

  if (function & 0x100)
  {
    if (Buttons_Pool[function&0xFF].Left_shortcut[0]==key)
      return 1;
    if (Buttons_Pool[function&0xFF].Left_shortcut[1]==key)
      return 1;
    return 0;
  }
  if (function & 0x200)
  {
    if (Buttons_Pool[function&0xFF].Right_shortcut[0]==key)
      return 1;
    if (Buttons_Pool[function&0xFF].Right_shortcut[1]==key)
      return 1;
    return 0;
  }
  if(key == Config_Key[function][0])
    return 1;
  if(key == Config_Key[function][1])
    return 1;
  return 0; 
}

/// Called each time there is a cursor move, either triggered by mouse
/// or keyboard shortcuts
/// @param x new cursor X coordinate
/// @param y new cursor Y coordinate
/// @return feedback
int Move_cursor_with_constraints(int x, int y)
{
  int feedback = 0;
  int mouse_blocked = 0;

  // Clip mouse to the editing area. There can be a border when using big 
  // pixels, if the SDL screen dimensions are not factors of the pixel size.
  if (y >= Screen_height)
  {
    Input_new_mouse_Y = Screen_height - 1;
    mouse_blocked = 1;
  }
  else if (y < 0)
  {
    Input_new_mouse_Y = 0;
    mouse_blocked = 1;
  }
  else
    Input_new_mouse_Y = y;

  if (x >= Screen_width)
  {
    Input_new_mouse_X = Screen_width - 1;
    mouse_blocked = 1;
  }
  else if (x < 0)
  {
    Input_new_mouse_X = 0;
    mouse_blocked = 1;
  }
  else
    Input_new_mouse_X = x;

  if (Operation_stack_size != 0)
  {
    // Forbid to go to the menu area when an operation is
    // on-going
    if(Menu_Y <= Input_new_mouse_Y)
    {
      // Cursor not in image area anymore
      mouse_blocked = 1;
      Input_new_mouse_Y = Menu_Y - 1; // just above the menu
    }

    if(Main.magnifier_mode)
    {
      // Do not cross the unzoomed/zoomed area separator
      if(!Operation_in_magnifier)
      {
        if(Input_new_mouse_X >= Main.separator_position)
        {
          mouse_blocked = 1;
          Input_new_mouse_X = Main.separator_position - 1;
        }
      }
      else
      {
        if(Input_new_mouse_X < Main.X_zoom)
        {
          mouse_blocked = 1;
          Input_new_mouse_X = Main.X_zoom;
        }
      }
    }
  }

  if ((Input_new_mouse_X != Mouse_X) ||
    (Input_new_mouse_Y != Mouse_Y))
  {
    // Hide cursor, because even just a click change needs it
    if (!Mouse_moved)
    {
      // Hide cursor (erasing icon and brush on screen
      // before changing the coordinates.
      Hide_cursor();
    }
    Mouse_moved++;
    Mouse_X = Input_new_mouse_X;
    Mouse_Y = Input_new_mouse_Y;

    if (Mouse_moved > Config.Mouse_merge_movement
      && !Operation[Current_operation][Mouse_K_unique]
          [Operation_stack_size].Fast_mouse)
        feedback=1;
  }
  if (mouse_blocked)
    Set_mouse_position();
  return feedback;
}


int Handle_mouse_btn_change(void)
{
  int feedback = 0;

  if (Input_new_mouse_K != Mouse_K)
  {
    if (Input_new_mouse_K == 0)
    {
      Input_sticky_control = 0;
    }
    // Hide cursor, because even just a click change needs it
    if (!Mouse_moved)
    {
      // Hide cursor (erasing icon and brush on screen
      // before changing the coordinates.
      Hide_cursor();
    }
    Mouse_moved++;
    Mouse_K = Input_new_mouse_K;

    feedback = 1;
  }
  return feedback;
}

// WM events management

#if defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
/**
 * Drag'n'Drop Protocol for X11 :
 * https://freedesktop.org/wiki/Specifications/XDND/
 * https://www.acc.umu.se/~vatten/XDND.html
 */
static int xdnd_version = 5;
static Window xdnd_source = None;

/**
 * Handle ClientMessage X11 event used by Drag-and-drop protocol
 */
static void Handle_ClientMessage(const XClientMessageEvent * xclient)
{
#if defined(SDL_VIDEO_DRIVER_X11)
  Display * X11_display;
  Window X11_window;

  if (!GFX2_Get_X11_Display_Window(&X11_display, &X11_window))
  {
    GFX2_Log(GFX2_ERROR, "Failed to get X11 display and window\n");
    return;
  }
#endif

  if (xclient->message_type == XInternAtom(X11_display, "XdndEnter", False))
  {
    // The ClientMessage only has space for three data types,
    // so if the source supports more than this, the target must
    // retrieve the property XdndTypeList from the source window
    // in order to get the list of available types.
    int i;
    //int list = xclient->data.l[1] & 1;
    xdnd_version = xclient->data.l[1] >> 24;
    xdnd_source = xclient->data.l[0];
    GFX2_Log(GFX2_DEBUG, "XdndEnter version=%d source=%lu %s\n",
             xdnd_version, xdnd_source, (xclient->data.l[1] & 1) ? "XdndTypeList" : "");
    if (xclient->data.l[1] & 1)
    {
      int r;
      unsigned long count = 0, bytesAfter = 0;
      unsigned char * value = NULL;
      Atom type = None;
      int format = 0;

      r = XGetWindowProperty(X11_display, xdnd_source,
                             XInternAtom(X11_display, "XdndTypeList", False),
                             0, LONG_MAX,
                             False, XA_ATOM, &type, &format, &count,
                             &bytesAfter, &value);
      if (r == Success && value != NULL)
      {
        Atom * atoms = (Atom *)value;
        for (i = 0; i < (int)count; i++)
        {
          char * atom_string = "None";
          if (atoms[i] != None)
            atom_string = XGetAtomName(X11_display, atoms[i]);
          GFX2_Log(GFX2_DEBUG, "  %lu %s\n", atoms[i], atom_string);
          if (atoms[i] != None)
            XFree(atom_string);
        }
        XFree(value);
      }
    }
    else
    {
      for (i = 2; i < 5; i++)
      {
        char * atom_string = "None";
        if (xclient->data.l[i] != None)
          atom_string = XGetAtomName(X11_display, xclient->data.l[i]);
        GFX2_Log(GFX2_DEBUG, "  %lu %s\n", xclient->data.l[i], atom_string);
        if (xclient->data.l[i] != None)
          XFree(atom_string);
      }
    }
  }
  else if (xclient->message_type == XInternAtom(X11_display, "XdndLeave", False))
  {
    GFX2_Log(GFX2_DEBUG, "XdndLeave\n");
  }
  else if (xclient->message_type == XInternAtom(X11_display, "XdndPosition", False))
  {
    XEvent reply;
    int x_abs, y_abs;
    int x_pos, y_pos;
    Window root_window, child;
    unsigned int width, height;
    unsigned int border_width, depth;

    x_abs = (xclient->data.l[2] >> 16) & 0xffff;
    y_abs = xclient->data.l[2] & 0xffff;
    // reply with XdndStatus
    // see https://github.com/glfw/glfw/blob/a9a5a0b016215b4e40a19acb69577d91cf21a563/src/x11_window.c

    memset(&reply, 0, sizeof(reply));

    reply.type = ClientMessage;
    reply.xclient.window = xclient->data.l[0]; // drag & drop source window
    reply.xclient.message_type = XInternAtom(X11_display, "XdndStatus", False);
    reply.xclient.format = 32;
    reply.xclient.data.l[0] = xclient->window;
    if (XGetGeometry(X11_display, X11_window, &root_window, &x_pos, &y_pos, &width, &height, &border_width, &depth)
        && XTranslateCoordinates(X11_display, X11_window, root_window, 0, 0, &x_abs, &y_abs, &child))
    {
      reply.xclient.data.l[2] = (x_abs & 0xffff) << 16 | (y_abs & 0xffff);
      reply.xclient.data.l[3] = (width & 0xffff) << 16 | (height & 0xffff);
    }

    // Reply that we are ready to copy the dragged data
    reply.xclient.data.l[1] = 1; // Accept with no rectangle
    if (xdnd_version >= 2)
      reply.xclient.data.l[4] = XInternAtom(X11_display, "XdndActionCopy", False);
    XSendEvent(X11_display, xclient->data.l[0], False, NoEventMask, &reply);
  }
  else if (xclient->message_type == XInternAtom(X11_display, "XdndDrop", False))
  {
    Atom selection = XInternAtom(X11_display, "XdndSelection", False);
    Time time = CurrentTime;
    if (xdnd_version >= 1)
      time = xclient->data.l[2];
    XConvertSelection(X11_display,
        selection,
        XInternAtom(X11_display, "text/uri-list", False),
        selection,
        xclient->window,
        time);
  }
  else
  {
    char * message_type_name = XGetAtomName(X11_display, xclient->message_type);
    GFX2_Log(GFX2_INFO, "Unhandled ClientMessage message_type=\"%s\"\n", message_type_name);
    XFree(message_type_name);
  }
}

/**
 * Handle SelectionNotify X11 event used for Clipboard Pasting and Drag-and-drop protocol
 */
static int Handle_SelectionNotify(const XSelectionEvent* xselection)
{
  int user_feedback_required = 0;
  Atom type = 0;
  int format = 0;
#if defined(SDL_VIDEO_DRIVER_X11)
  Display * X11_display;
  Window X11_window;

  if (!GFX2_Get_X11_Display_Window(&X11_display, &X11_window))
  {
    GFX2_Log(GFX2_ERROR, "Failed to get X11 display and window\n");
    return 0;
  }
#endif

  if (xselection->property == XInternAtom(X11_display, "XdndSelection", False))
  {
    int r;
    unsigned long count = 0, bytesAfter = 0;
    unsigned char * value = NULL;

    r = XGetWindowProperty(X11_display, xselection->requestor, xselection->property, 0, LONG_MAX,
                           False, xselection->target /* type */, &type, &format,
                           &count, &bytesAfter, &value);
    if (r == Success && value != NULL)
    {
      if (format == 8)
      {
        int i, j;
        Drop_file_name = malloc(count + 1);
        i = 0; j = 0;
        if (count > 7 && 0 == memcmp(value, "file://", 7))
          i = 7;
        while (i < (int)count && value[i] != 0 && value[i] != '\n' && value[i] != '\r')
        {
          if (i < ((int)count + 2) && value[i] == '%')
          {
            // URI-Decode : "%NN" to char of value 0xNN
            i++;
            Drop_file_name[j] = (value[i] - ((value[i] >= 'A') ? 'A' - 10 : '0')) << 4;
            i++;
            Drop_file_name[j++] |= (value[i] - ((value[i] >= 'A') ? 'A' - 10 : '0'));
            i++;
          }
          else
          {
            Drop_file_name[j++] = (char)value[i++];
          }
        }
        Drop_file_name[j++] = '\0';
      }
      XFree(value);
    }
    if (xdnd_version >= 2)
    {
      XEvent reply;
      memset(&reply, 0, sizeof(reply));

      reply.type = ClientMessage;
      reply.xclient.window = xdnd_source;
      reply.xclient.message_type = XInternAtom(X11_display, "XdndFinished", False);
      reply.xclient.format = 32;
      reply.xclient.data.l[0] = X11_window;
      reply.xclient.data.l[1] = 1;  // success
      reply.xclient.data.l[2] = XInternAtom(X11_display, "XdndActionCopy", False);

      XSendEvent(X11_display, xdnd_source, False, NoEventMask, &reply);
    }
  }
  else if (xselection->selection == XInternAtom(X11_display, "CLIPBOARD", False)
      || xselection->selection == XInternAtom(X11_display, "PRIMARY", False))
  {
    int r;
    unsigned long count = 0, bytesAfter = 0;
    unsigned char * value = NULL;

    if (xselection->property != None)
    {
      char * selection_name = XGetAtomName(X11_display, xselection->selection);
      char * property_name = XGetAtomName(X11_display, xselection->property);
      char * target_name = XGetAtomName(X11_display, xselection->target);

      GFX2_Log(GFX2_DEBUG, "xselection: selection=%s property=%s target=%s\n",
               selection_name, property_name, target_name);
      XFree(selection_name);
      XFree(property_name);
      XFree(target_name);

      r = XGetWindowProperty(X11_display, X11_window, xselection->property, 0, LONG_MAX,
          False, AnyPropertyType/*xselection->target*/ /* type */, &type, &format,
          &count, &bytesAfter, &value);
      if (r == Success && value != NULL)
      {
#ifndef __no_pnglib__
        Atom png = XInternAtom(X11_display, "image/png", False);
#endif
#ifndef __no_tifflib__
        Atom tiff = XInternAtom(X11_display, "image/tiff", False);
#endif
        Atom urilist = XInternAtom(X11_display, "text/uri-list", False);
        Atom utf8string = XInternAtom(X11_display, "UTF8_STRING", False);
        // by order of preference
        const struct { Atom a; enum X11_CLIPBOARD_TYPES t; } supported[] = {
#ifndef __no_pnglib__
          { png, X11_CLIPBOARD_PNG },
#endif
#ifndef __no_tifflib__
          { tiff, X11_CLIPBOARD_TIFF },
#endif
          { urilist, X11_CLIPBOARD_URILIST },
          { utf8string, X11_CLIPBOARD_UTF8STRING },
          { None, X11_CLIPBOARD_NONE }
        };
        char * type_name = XGetAtomName(X11_display, type);
        GFX2_Log(GFX2_DEBUG, "Clipboard value=%p %lu bytes format=%d type=%s\n",
                 value, count, format, type_name);
        XFree(type_name);
        if (xselection->target == XInternAtom(X11_display, "TARGETS", False))
        {
          unsigned long i;
          Atom * atoms = (Atom *)value;
          Atom prefered = None;
          for (i = 0; i < count; i++)
          {
            int j;
            char * atom_name = XGetAtomName(X11_display, atoms[i]);
            GFX2_Log(GFX2_DEBUG, "  %d %s\n", atoms[i], atom_name);
            XFree(atom_name);
            if (prefered == None)
            {
              for (j = 0; supported[j].a != None; j++)
              {
                if (atoms[i] == supported[j].a)
                {
                  prefered = atoms[i];
                  break;
                }
              }
            }
          }
          if (prefered != None)
          {
            XConvertSelection(X11_display, xselection->selection, prefered,
                    xselection->property, X11_window, CurrentTime);
          }
        }
        else if (count > 0)
        {
          X11_clipboard = malloc(count+1);
          if (X11_clipboard != NULL)
          {
            int i;
            X11_clipboard_size = count;
            X11_clipboard_type = X11_CLIPBOARD_UNKNOWN;
            for (i = 0; supported[i].a != None; i++)
            {
              if (supported[i].a == xselection->target)
              {
                X11_clipboard_type = supported[i].t;
                break;
              }
            }
            memcpy(X11_clipboard, value, count);
            X11_clipboard[count] = '\0';
          }
          else
          {
            X11_clipboard_type = X11_CLIPBOARD_NONE;
            X11_clipboard_size = 0;
          }
        }
        XFree(value);
      }
      else
        GFX2_Log(GFX2_INFO, "XGetWindowProperty failed. r=%d value=%p\n", r, value);
      user_feedback_required = 1;
    }
    else
    {
      GFX2_Log(GFX2_INFO, "X11 Selection conversion failed\n");
    }
  }
  else
  {
    char * selection_name = XGetAtomName(X11_display, xselection->selection);
    char * property_name = "None";
    if (xselection->property != None)
      XGetAtomName(X11_display, xselection->property);
    GFX2_Log(GFX2_INFO, "Unhandled SelectNotify selection=%s property=%s\n", selection_name, property_name);
    XFree(selection_name);
    if (xselection->property != None)
      XFree(property_name);
  }
  return user_feedback_required;
}

/**
 * Handle SelectionRequest X11 event used for Clipboard copying
 */
static void Handle_SelectionRequest(const XSelectionRequestEvent* xselectionrequest)
{
  XSelectionEvent xselection; 
  char * target_name;
  char * property_name;
  Atom png;
#if defined(SDL_VIDEO_DRIVER_X11)
  Display * X11_display;
  Window X11_window;

  if (!GFX2_Get_X11_Display_Window(&X11_display, &X11_window))
  {
    GFX2_Log(GFX2_ERROR, "Failed to get X11 display and window\n");
    return;
  }
#endif

  png = XInternAtom(X11_display, "image/png", False);

  target_name = XGetAtomName(X11_display, xselectionrequest->target);
  property_name = XGetAtomName(X11_display, xselectionrequest->property);
  GFX2_Log(GFX2_DEBUG, "Handle_SelectionRequest target=%s property=%s\n", target_name, property_name);
  XFree(target_name);
  XFree(property_name);

  xselection.type = SelectionNotify;
  xselection.requestor = xselectionrequest->requestor;
  xselection.selection = xselectionrequest->selection;
  xselection.target = xselectionrequest->target;
  xselection.property = xselectionrequest->property;
  xselection.time = xselectionrequest->time;

  if (xselectionrequest->target == XInternAtom(X11_display, "TARGETS", False))
  {
    Atom targets[1];
    targets[0] = png;   // Advertise image/png as the only supported format
    XChangeProperty(X11_display, xselectionrequest->requestor, xselectionrequest->property,
                    XA_ATOM, 32, PropModeReplace,
                    (unsigned char *)targets, 1);
  }
  else if (xselectionrequest->target == png)
  {
    XChangeProperty(X11_display, xselectionrequest->requestor, xselectionrequest->property,
                    png, 8, PropModeReplace,
                    (unsigned char *)X11_clipboard, X11_clipboard_size);
  }
  else
  {
    xselection.property = None; // refuse
  }

  XSendEvent(X11_display, xselectionrequest->requestor, True, NoEventMask, (XEvent *)&xselection);
}
#endif

#if defined(USE_SDL)
static void Handle_window_resize(SDL_ResizeEvent * event)
{
#if defined(WIN32)
  WINDOWPLACEMENT windowplacement;
  windowplacement.length = sizeof(WINDOWPLACEMENT);
  if (GetWindowPlacement(GFX2_Get_Window_Handle(), &windowplacement))
  {
    switch (windowplacement.showCmd)
    {
      case SW_SHOWMAXIMIZED:
        Window_state = GFX2_WINDOW_MAXIMIZED;
        GFX2_Log(GFX2_DEBUG, "window is MAXIMIZED\n");
        break;
      case SW_SHOWMINIMIZED:
        Window_state = GFX2_WINDOW_MINIMIZED;
        GFX2_Log(GFX2_DEBUG, "window is MINIMIZED\n");
        break;
      default:
        Window_state = GFX2_WINDOW_STANDARD;
        GFX2_Log(GFX2_DEBUG, "window is NORMAL\n");
    }
  }
#endif
  Resize_width = event->w;
  Resize_height = event->h;
}
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
static void Handle_window_exit(SDL_QuitEvent * event)
{
  (void)event; // unused
    
  Quit_is_required = 1;
}

// Mouse events management

static int Handle_mouse_move(SDL_MouseMotionEvent * event)
{
  //GFX2_Log(GFX2_DEBUG, "mouse motion (%+d,%+d)\n", event->xrel, event->yrel);

  return Move_cursor_with_constraints(event->x / Pixel_width, event->y / Pixel_height);
}

static int Handle_mouse_click(SDL_MouseButtonEvent * event)
{
  switch(event->button)
  {
    case SDL_BUTTON_LEFT:
      if (Button_inverter)
        Input_new_mouse_K |= 2;
      else
        Input_new_mouse_K |= 1;
      break;

    case SDL_BUTTON_RIGHT:
      if (Button_inverter)
        Input_new_mouse_K |= 1;
      else
        Input_new_mouse_K |= 2;
      break;

    case SDL_BUTTON_MIDDLE:
      Key = KEY_MOUSEMIDDLE|Get_Key_modifiers();
      // TODO: repeat system maybe?
      return 0;

      // In SDL 2.0 the mousewheel is no longer a button.
      // Look for SDL_MOUSEWHEEL events.
#if defined(USE_SDL)
    case SDL_BUTTON_WHEELUP:
      Key = KEY_MOUSEWHEELUP|Get_Key_modifiers();
      return 0;

    case SDL_BUTTON_WHEELDOWN:
      Key = KEY_MOUSEWHEELDOWN|Get_Key_modifiers();
      return 0;
#endif

    default:
      GFX2_Log(GFX2_DEBUG, "Unknown mouse button %d\n", event->button);
      return 0;
  }
  return Handle_mouse_btn_change();
}

static int Handle_mouse_release(SDL_MouseButtonEvent * event)
{
  switch(event->button)
  {
  case SDL_BUTTON_LEFT:
    if (Button_inverter)
      Input_new_mouse_K &= ~2;
    else
      Input_new_mouse_K &= ~1;
    break;

  case SDL_BUTTON_RIGHT:
    if (Button_inverter)
      Input_new_mouse_K &= ~1;
    else
      Input_new_mouse_K &= ~2;
    break;
  }

  return Handle_mouse_btn_change();
}
#endif

// Keyboard management

/**
 * check Keys that emulate mouse moves, etc.
 */
int Handle_special_key_press(void)
{
    if(Is_shortcut(Key,SPECIAL_MOUSE_UP))
    {
      Directional_emulated |= D_JOYSTICK_UP;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_DOWN))
    {
      Directional_emulated |= D_JOYSTICK_DOWN;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_LEFT))
    {
      Directional_emulated |= D_JOYSTICK_LEFT;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_RIGHT))
    {
      Directional_emulated |= D_JOYSTICK_RIGHT;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_CLICK_LEFT) && Keyboard_click_allowed > 0)
    {
        Input_new_mouse_K=1;
        Directional_click=1;
        return Handle_mouse_btn_change();
    }
    else if(Is_shortcut(Key,SPECIAL_CLICK_RIGHT) && Keyboard_click_allowed > 0)
    {
        Input_new_mouse_K=2;
        Directional_click=2;
        return Handle_mouse_btn_change();
    }
    else if(Is_shortcut(Key,SPECIAL_HOLD_PAN))
    {
      Pan_shortcut_pressed=1;
      return 0;
    }
    return 0;
}

#if defined(USE_SDL) || defined(USE_SDL2)
static int Handle_key_press(SDL_KeyboardEvent * event)
{
    //Appui sur une touche du clavier
    int modifier;
  
    Key = Keysym_to_keycode(event->keysym);
    Key_ANSI = Keysym_to_ANSI(event->keysym);
#if defined(USE_SDL)
    Key_UNICODE = event->keysym.unicode;
    if (Key_UNICODE == 0)
#endif
      Key_UNICODE = Key_ANSI;
    switch(event->keysym.sym)
    {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
        modifier=GFX2_MOD_SHIFT;
        break;

      case SDLK_RCTRL:
      case SDLK_LCTRL:
        modifier=GFX2_MOD_CTRL;
        break;

      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_MODE:
        modifier=GFX2_MOD_ALT;
        break;

#if defined(USE_SDL2)
      case SDLK_RGUI:
      case SDLK_LGUI:
#else
      case SDLK_RMETA:
      case SDLK_LMETA:
#endif
        modifier=GFX2_MOD_META;
        break;

      default:
        modifier=0;
    }
    if (Config.Swap_buttons && modifier == Config.Swap_buttons && Button_inverter==0)
    {
      Button_inverter=1;
      if (Input_new_mouse_K)
      {
        Input_new_mouse_K ^= 3; // Flip bits 0 and 1
        return Handle_mouse_btn_change();
      }
    }
    #ifdef RSUPER_EMULATES_META_MOD
    if (Key==SDLK_RSUPER)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_META);
      Key=0;
    }
    #endif

    return Handle_special_key_press();
}
#endif

int Release_control(int key_code, int modifier)
{
    int need_feedback = 0;

    if (modifier == GFX2_MOD_SHIFT)
    {
      // Disable "snap axis" mode
      Snap_axis = 0;
      need_feedback = 1;
    }
    if (Config.Swap_buttons && modifier == Config.Swap_buttons && Button_inverter==1)
    {
      Button_inverter=0;
      if (Input_new_mouse_K)
      {      
        Input_new_mouse_K ^= 3; // Flip bits 0 and 1
        return Handle_mouse_btn_change();
      }
    }

    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][1]&modifier))
    {
      Directional_emulated &= ~D_JOYSTICK_UP;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][1]&modifier))
    {
      Directional_emulated &= ~D_JOYSTICK_DOWN;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][1]&modifier))
    {
      Directional_emulated &= ~D_JOYSTICK_LEFT;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][1]&modifier))
    {
      Directional_emulated &= ~D_JOYSTICK_RIGHT;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_CLICK_LEFT][0]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_LEFT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_CLICK_LEFT][1]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_LEFT][1]&modifier))
    {
        if (Directional_click & 1)
        {
            Directional_click &= ~1;
            Input_new_mouse_K &= ~1;
            return Handle_mouse_btn_change() || need_feedback;
        }
    }
    if((key_code && key_code == (Config_Key[SPECIAL_CLICK_RIGHT][0]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_RIGHT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_CLICK_RIGHT][1]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_RIGHT][1]&modifier))
    {
        if (Directional_click & 2)
        {
            Directional_click &= ~2;
            Input_new_mouse_K &= ~2;
            return Handle_mouse_btn_change() || need_feedback;
        }
    }
    if((key_code && key_code == (Config_Key[SPECIAL_HOLD_PAN][0]&0x0FFF)) || (Config_Key[SPECIAL_HOLD_PAN][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_HOLD_PAN][1]&0x0FFF)) || (Config_Key[SPECIAL_HOLD_PAN][1]&modifier))
    {
      Pan_shortcut_pressed=0;
      need_feedback = 1;
    }
    
    // Other keys don't need to be released : they are handled as "events" and procesed only once.
    // These clicks are apart because they need to be continuous (ie move while key pressed)
    // We are relying on "hardware" keyrepeat to achieve that.
    return need_feedback;
}


#if defined(USE_SDL) || defined(USE_SDL2)
static int Handle_key_release(SDL_KeyboardEvent * event)
{
    int modifier;
    int released_key = Keysym_to_keycode(event->keysym) & 0x0FFF;
  
    switch(event->keysym.sym)
    {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
        modifier=GFX2_MOD_SHIFT;
        break;

      case SDLK_RCTRL:
      case SDLK_LCTRL:
        modifier=GFX2_MOD_CTRL;
        break;

      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_MODE:
        modifier=GFX2_MOD_ALT;
        break;

      #ifdef RSUPER_EMULATES_META_MOD
      case SDLK_RSUPER:
        SDL_SetModState(SDL_GetModState() & ~KMOD_META);
        modifier=GFX2_MOD_META;
        break;
      #endif
      
#if defined(USE_SDL2)
      case SDLK_RGUI:
      case SDLK_LGUI:
#else
      case SDLK_RMETA:
      case SDLK_LMETA:
#endif
        modifier=GFX2_MOD_META;
        break;

      default:
        modifier=0;
    }
    return Release_control(released_key, modifier);
}
#endif


// Joystick management

#if defined(USE_JOYSTICK) && (defined(USE_SDL) || defined(USE_SDL2))
static int Handle_joystick_press(SDL_JoyButtonEvent * event)
{
    if (event->button == Joybutton_shift)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_SHIFT);
      return 0;
    }
    if (event->button == Joybutton_control)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_CTRL);
      if (Config.Swap_buttons == GFX2_MOD_CTRL && Button_inverter==0)
      {
        Button_inverter=1;
        if (Input_new_mouse_K)
        {
          Input_new_mouse_K ^= 3; // Flip bits 0 and 1
          return Handle_mouse_btn_change();
        }
      }
      return 0;
    }
    if (event->button == Joybutton_alt)
    {
#if defined(USE_SDL)
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_META));
#else
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_GUI));
#endif
      if (Config.Swap_buttons == GFX2_MOD_ALT && Button_inverter==0)
      {
        Button_inverter=1;
        if (Input_new_mouse_K)
        {
          Input_new_mouse_K ^= 3; // Flip bits 0 and 1
          return Handle_mouse_btn_change();
        }
      }
      return 0;
    }
    if (event->button == Joybutton_left_click)
    {
      Input_new_mouse_K = Button_inverter ? 2 : 1;
      return Handle_mouse_btn_change();
    }
    if (event->button == Joybutton_right_click)
    {
      Input_new_mouse_K = Button_inverter ? 1 : 2;
      return Handle_mouse_btn_change();
    }
    switch(event->button)
    {
      #ifdef JOY_BUTTON_UP
      case JOY_BUTTON_UP:
        Digital_joystick_state |= D_JOYSTICK_UP;
        break;
      #endif
      #ifdef JOY_BUTTON_UPRIGHT
      case JOY_BUTTON_UPRIGHT:
        Digital_joystick_state |= D_JOYSTICK_UP_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_RIGHT
      case JOY_BUTTON_RIGHT:
        Digital_joystick_state |= D_JOYSTICK_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNRIGHT
      case JOY_BUTTON_DOWNRIGHT:
        Digital_joystick_state |= D_JOYSTICK_DOWN_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWN
      case JOY_BUTTON_DOWN:
        Digital_joystick_state |= D_JOYSTICK_DOWN;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNLEFT
      case JOY_BUTTON_DOWNLEFT:
        Digital_joystick_state |= D_JOYSTICK_DOWN_LEFT;
        break;
      #endif
      #ifdef JOY_BUTTON_LEFT
      case JOY_BUTTON_LEFT:
        Digital_joystick_state |= D_JOYSTICK_LEFT;
        break;
      #endif
      #ifdef JOY_BUTTON_UPLEFT
      case JOY_BUTTON_UPLEFT:
        Digital_joystick_state |= D_JOYSTICK_UP_LEFT;
        break;
      #endif
      
      default:
        break;
    }
      
    Key = (KEY_JOYBUTTON + event->button) | Get_Key_modifiers();
    // TODO: systeme de répétition
    
    return 1;
}

static int Handle_joystick_release(SDL_JoyButtonEvent * event)
{
    if (event->button == Joybutton_shift)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_SHIFT);
      return Release_control(0,GFX2_MOD_SHIFT);
    }
    if (event->button == Joybutton_control)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_CTRL);
      return Release_control(0,GFX2_MOD_CTRL);
    }
    if (event->button == Joybutton_alt)
    {
#if defined(USE_SDL)
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_META));
#else
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_GUI));
#endif
      return Release_control(0,GFX2_MOD_ALT);
    }
    if (event->button == Joybutton_left_click)
    {
      Input_new_mouse_K &= ~1;
      return Handle_mouse_btn_change();
    }
    if (event->button == Joybutton_right_click)
    {
      Input_new_mouse_K &= ~2;
      return Handle_mouse_btn_change();
    }
  
    switch(event->button)
    {
      #ifdef JOY_BUTTON_UP
      case JOY_BUTTON_UP:
        Digital_joystick_state &= ~D_JOYSTICK_UP;
        break;
      #endif
      #ifdef JOY_BUTTON_UPRIGHT
      case JOY_BUTTON_UPRIGHT:
        Digital_joystick_state &= ~D_JOYSTICK_UP_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_RIGHT
      case JOY_BUTTON_RIGHT:
        Digital_joystick_state &= ~D_JOYSTICK_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNRIGHT
      case JOY_BUTTON_DOWNRIGHT:
        Digital_joystick_state &= ~D_JOYSTICK_DOWN_RIGHT;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWN
      case JOY_BUTTON_DOWN:
        Digital_joystick_state &= ~D_JOYSTICK_DOWN;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNLEFT
      case JOY_BUTTON_DOWNLEFT:
        Digital_joystick_state &= ~D_JOYSTICK_DOWN_LEFT;
        break;
      #endif
      #ifdef JOY_BUTTON_LEFT
      case JOY_BUTTON_LEFT:
        Digital_joystick_state &= ~D_JOYSTICK_LEFT;
        break;
      #endif
      #ifdef JOY_BUTTON_UPLEFT
      case JOY_BUTTON_UPLEFT:
        Digital_joystick_state &= ~D_JOYSTICK_UP_LEFT;
        break;
      #endif
      
      default:
        break;
    }
  return 1;
}

static void Handle_joystick_movement(SDL_JoyAxisEvent * event)
{
  if (event->axis == JOYSTICK_AXIS_X)
  {
    if (event->value < -JOYSTICK_THRESHOLD || event->value > JOYSTICK_THRESHOLD)
      Joystick_horizontal = event->value << 1;
    else
      Joystick_horizontal = 0;
  }
  else if (event->axis == JOYSTICK_AXIS_Y)
  {
    if (event->value < -JOYSTICK_THRESHOLD || event->value > JOYSTICK_THRESHOLD)
      Joystick_vertical = event->value << 1;
    else
      Joystick_vertical = 0;
  }
}
#endif

/** Attempts to move the mouse cursor by the given deltas
 * @param step move multiply factor
 * @param delta_x 16.16 fixed point value
 * @param delta_y 16.16 fixed point value
 * @return feedback
 */
static int Cursor_displace(int step, long delta_x, long delta_y)
{
  long x, y;

  x = ((long)Input_new_mouse_X << 16) + Input_new_mouse_X_frac;
  y = ((long)Input_new_mouse_Y << 16) + Input_new_mouse_Y_frac;

  if(Main.magnifier_mode && Input_new_mouse_Y < Menu_Y && Input_new_mouse_X > Main.separator_position)
  {
    // Cursor in zoomed area
    x += Main.magnifier_factor * delta_x;
    if (x < (Main.separator_position << 16))
      x = Main.separator_position << 16;
    else if (x > ((Screen_width-1) << 16))
      x = (Screen_width-1) << 16;
    y += Main.magnifier_factor * delta_y;
  }
  else
  {
    x += delta_x * step;
    if (x < 0)
      x = 0;
    else if (x > ((Screen_width-1) << 16))
      x = (Screen_width-1) << 16;
    y += delta_y * step;
  }
  if (y < 0)
    y = 0;
  else if (y > ((Screen_height-1) << 16))
    y = (Screen_height-1) << 16;

  Input_new_mouse_X_frac = x & 0xffff;
  Input_new_mouse_Y_frac = y & 0xffff;

  return Move_cursor_with_constraints(x >> 16, y >> 16);
}

// This function is the acceleration profile for directional (digital) cursor
// controllers.
int Directional_acceleration(int msec)
{
  const int initial_delay = 250;
  const int linear_factor = 200;
  const int accel_factor = 10000;
  // At beginning there is 1 pixel move, then nothing for N milliseconds
  if (msec<initial_delay)
    return 1;
    
  // After that, position over time is generally y = ax²+bx+c
  // a = 1/accel_factor
  // b = 1/linear_factor
  // c = 1
  return 1+(msec-initial_delay+linear_factor)/linear_factor+(msec-initial_delay)*(msec-initial_delay)/accel_factor;
}

#if defined(USE_X11)
word X11_key_mod;

static word X11_to_GFX2_Modifiers(unsigned int state)
{
  word mod = 0;
  // right/left window 40 Mod4Mask
  // left alt = 8         Mod1Mask
  // right alt = 80       Mod5Mask
  // NumLock = 10         Mod2Mask
  // see "modmap"
  if (state & ShiftMask)
    mod |= GFX2_MOD_SHIFT;
  if (state & ControlMask)
    mod |= GFX2_MOD_CTRL;
  if (state & (Mod1Mask | Mod5Mask))
    mod |= GFX2_MOD_ALT;
  if (state & Mod4Mask)
    mod |= GFX2_MOD_META;
  X11_key_mod = mod;
  return mod;
}
#endif

#if defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2)
int user_feedback_required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer
#endif

// Main input handling function

int Get_input(int sleep_time)
{
#if defined(USE_SDL) || defined(USE_SDL2)
    SDL_Event event;
    int user_feedback_required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer
                
    Color_cycling();
    // Commit any pending screen update.
    // This is done in this function because it's called after reading 
    // some user input.
    Flush_update();

    if (Quit_is_required)
      return 1;

    Key_ANSI = 0;
    Key_UNICODE = 0;
    Key = 0;
#if defined(USE_SDL2)
    memset(Key_Text, 0, sizeof(Key_Text));
#endif
    Mouse_moved = 0;
    Input_new_mouse_X = Mouse_X;
    Input_new_mouse_Y = Mouse_Y;
    Input_new_mouse_K = Mouse_K;

    // Not using SDL_PollEvent() because every call polls the input
    // device. In some cases such as high-sensitivity mouse or cheap
    // digital joypad, every call will see something subtly different in
    // the state of the device, and thus it will enqueue a new event.
    // The result is that the queue will never empty !!!

    // Get new events from input devices.
    SDL_PumpEvents();

    // Process as much events as possible without redrawing the screen.
    // This mostly allows us to merge mouse events for people with an high
    // resolution mouse
#if defined(USE_SDL)
    while(!user_feedback_required && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS)==1)
#elif defined(USE_SDL2)
    while(!user_feedback_required && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)==1)
#endif
    {
      switch(event.type)
      {
#if defined(USE_SDL)
          case SDL_ACTIVEEVENT:
              GFX2_Log(GFX2_DEBUG, "SDL_ACTIVEEVENT gain=%d state=%d (%s%s%s)\n",
                       event.active.gain, event.active.state,
                       (event.active.state & SDL_APPMOUSEFOCUS)?"Mouse ":"",
                       (event.active.state & SDL_APPINPUTFOCUS)?"Keyboard ":"",
                       (event.active.state & SDL_APPACTIVE)?"Iconification":"");
#ifdef WIN32
              // Work around a bug in SDL1.2 with win32
              // when doing ALT-TAB to loose focus, and then gaining focus back
              // by clicking on the GrafX2 window, the "ALT" key appears as still pressed
              // So "depress" ALT
              if (event.active.gain && (event.active.state & SDL_APPINPUTFOCUS) != 0)
                SDL_SetModState(SDL_GetModState() & ~KMOD_ALT);
#endif
              break;

          case SDL_VIDEORESIZE:
              Handle_window_resize(&event.resize);
              user_feedback_required = 1;
              break;

          case SDL_VIDEOEXPOSE:
              GFX2_Log(GFX2_DEBUG, "SDL_VIDEOEXPOSE\n");
              break;
#endif

#if defined(USE_SDL2)
          case SDL_WINDOWEVENT:
              switch(event.window.event)
              {
                case SDL_WINDOWEVENT_RESIZED: // change by external event (user or window manager)
                  GFX2_Log(GFX2_DEBUG, "SDL_WINDOWEVENT_RESIZED %d %dx%d\n", event.window.windowID, event.window.data1, event.window.data2);
                  Resize_width = event.window.data1;
                  Resize_height = event.window.data2;
                  // forbid window size < 320x200
                  if (Resize_width < 320)
                    Resize_width = 320;
                  if (Resize_height < 200)
                    Resize_height = 200;
                  if (Resize_width != event.window.data1 || Resize_height != event.window.data2)
                    SDL_SetWindowSize(SDL_GetWindowFromID(event.window.windowID), Resize_width, Resize_height);
                  break;
                case SDL_WINDOWEVENT_CLOSE:
                  GFX2_Log(GFX2_DEBUG, "SDL_WINDOWEVENT_CLOSE %d\n", event.window.windowID);
                  Quit_is_required = 1;
                  user_feedback_required = 1;
                  break;
                case SDL_WINDOWEVENT_MINIMIZED:
                  GFX2_Log(GFX2_DEBUG, "SDL_WINDOWEVENT_MINIMIZED %d\n", event.window.windowID);
                  Window_state = GFX2_WINDOW_MINIMIZED;
                  break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                  GFX2_Log(GFX2_DEBUG, "SDL_WINDOWEVENT_MAXIMIZED %d\n", event.window.windowID);
                  Window_state = GFX2_WINDOW_MAXIMIZED;
                  break;
                case SDL_WINDOWEVENT_RESTORED:
                  GFX2_Log(GFX2_DEBUG, "SDL_WINDOWEVENT_RESTORED %d\n", event.window.windowID);
                  Window_state = GFX2_WINDOW_STANDARD;
                  break;
                default:
                  GFX2_Log(GFX2_DEBUG, "Unhandled SDL_WINDOWEVENT : %d\n", event.window.event);
              }
              break;
#endif

          case SDL_QUIT:
              GFX2_Log(GFX2_DEBUG, "SDL_QUIT\n");
              Handle_window_exit(&event.quit);
              user_feedback_required = 1;
              break;

          case SDL_MOUSEMOTION:
              user_feedback_required = Handle_mouse_move(&event.motion);
              break;

          case SDL_MOUSEBUTTONDOWN:
#if defined(USE_SDL2) && SDL_VERSION_ATLEAST(2, 0, 4)
              // SDL_CaptureMouse() is available since SDL 2.0.4.
              SDL_CaptureMouse(SDL_TRUE);
#elif defined(USE_SDL)
              SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
              Handle_mouse_click(&event.button);
              user_feedback_required = 1;
              break;

          case SDL_MOUSEBUTTONUP:
#if defined(USE_SDL2) && SDL_VERSION_ATLEAST(2, 0, 4)
              // SDL_CaptureMouse() is available since SDL 2.0.4.
              SDL_CaptureMouse(SDL_FALSE);
#elif defined(USE_SDL)
              SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
              Handle_mouse_release(&event.button);
              user_feedback_required = 1;
              break;

#if defined(USE_SDL2)
          case SDL_MOUSEWHEEL:
              if (event.wheel.y > 0)
                Key = KEY_MOUSEWHEELUP|Get_Key_modifiers();
              else if (event.wheel.y < 0)
                Key = KEY_MOUSEWHEELDOWN|Get_Key_modifiers();
              user_feedback_required = 1;
              break;
#endif

          case SDL_KEYDOWN:
              Handle_key_press(&event.key);
              user_feedback_required = 1;
              break;

          case SDL_KEYUP:
              Handle_key_release(&event.key);
              break;

#if defined(USE_SDL2)
          case SDL_TEXTINPUT:
              memcpy(Key_Text, event.text.text, sizeof(Key_Text));
              user_feedback_required = 1;
              break;
          case SDL_TEXTEDITING:
              GFX2_Log(GFX2_DEBUG, "SDL_TEXTEDITING event : start=%d length=%d text='%s'\n",
                event.edit.start, event.edit.length, event.edit.text);
              break;
#endif

          // Start of Joystik handling
          #ifdef USE_JOYSTICK

          case SDL_JOYBUTTONUP:
              Handle_joystick_release(&event.jbutton);
              user_feedback_required = 1;
              break;

          case SDL_JOYBUTTONDOWN:
              Handle_joystick_press(&event.jbutton);
              user_feedback_required = 1;
              break;

          case SDL_JOYAXISMOTION:
              Handle_joystick_movement(&event.jaxis);
              break;

          #endif
          // End of Joystick handling
          
#if defined(USE_SDL2)
          case SDL_DROPFILE:
              GFX2_Log(GFX2_DEBUG, "SDL_DROPFILE: %s\n", event.drop.file);
              Drop_file_name = strdup(event.drop.file);
              SDL_free(event.drop.file);
              break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
          // SDL_DROPTEXT, SDL_DROPBEGIN, and SDL_DROPCOMPLETE
          // are available since SDL 2.0.5.
          case SDL_DROPTEXT:
              GFX2_Log(GFX2_DEBUG, "SDL_DROPTEXT: \"%s\"\n", event.drop.file);
              SDL_free(event.drop.file);
              break;
          case SDL_DROPBEGIN:
              GFX2_Log(GFX2_DEBUG, "SDL_DROPBEGIN\n");
              break;
          case SDL_DROPCOMPLETE:
              GFX2_Log(GFX2_DEBUG, "SDL_DROPCOMPLETE\n");
              break;
#endif
          /// @todo We could do something with finger touch events
          case SDL_FINGERDOWN:
          case SDL_FINGERUP:
          case SDL_FINGERMOTION:
              break;
#endif

          case SDL_SYSWMEVENT:
#if defined(USE_SDL) && defined(__WIN32__)
              if(event.syswm.msg->msg  == WM_DROPFILES)
              {
                int file_count;
                HDROP hdrop = (HDROP)(event.syswm.msg->wParam);
                if((file_count = DragQueryFile(hdrop,(UINT)-1,(LPTSTR) NULL ,(UINT) 0)) > 0)
                {
                  long len;
                  // Query filename length
                  len = DragQueryFile(hdrop, 0, NULL, 0);
                  if (len)
                  {
                    Drop_file_name = calloc(len+1, 1);
                    if (Drop_file_name)
                    {
#ifdef UNICODE
                      TCHAR LongDropFileName[MAX_PATH];
                      TCHAR ShortDropFileName[MAX_PATH];
                      if (DragQueryFile(hdrop, 0, LongDropFileName, (UINT)MAX_PATH)
                        && GetShortPathName(LongDropFileName, ShortDropFileName, MAX_PATH))
                      {
                        int i;
                        for (i = 0; ShortDropFileName[i] != 0; i++)
                          Drop_file_name[i] = (char)ShortDropFileName[i];
                        Drop_file_name[i] = 0;
                      }
#else
                      if (DragQueryFile(hdrop, 0, (LPTSTR)Drop_file_name, (UINT)MAX_PATH))
                      {
                        // Success
                      }
#endif
                      else
                      {
                        free(Drop_file_name);
                        GFX2_Log(GFX2_ERROR, "Failed to get Drag filename\n");
                      }
                    }
                    else
                    {
                      GFX2_Log(GFX2_ERROR, "Failed to allocate %ld bytes\n", len + 1);
                    }
                  }
                  else
                  {
                    // Don't report weird Windows error
                  }
                }
                else
                {
                  // Drop of zero files. Thanks for the information, Bill.
                }
              }
#elif defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11)
#if defined(USE_SDL)
#define xevent event.syswm.msg->event.xevent
#else
#define xevent event.syswm.msg->msg.x11.event
#endif
              switch (xevent.type)
              {
                case ClientMessage:
                  Handle_ClientMessage(&(xevent.xclient));
                  break;
                case SelectionNotify:
                  if (Handle_SelectionNotify(&(xevent.xselection)))
                    user_feedback_required = 1;
                  break;
                case SelectionRequest:
                  Handle_SelectionRequest(&(xevent.xselectionrequest));
                  break;
                case SelectionClear:
                  GFX2_Log(GFX2_DEBUG, "X11 SelectionClear\n");
                  if (X11_clipboard)
                  {
                    free(X11_clipboard);
                    X11_clipboard = NULL;
                    X11_clipboard_size = 0;
                    X11_clipboard_type = X11_CLIPBOARD_NONE;
                  }
                  SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);
                  break;
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
                  // ignore
                  break;
#ifdef GenericEvent
                case GenericEvent:
                  GFX2_Log(GFX2_DEBUG, "SDL_SYSWMEVENT x11 GenericEvent extension=%d evtype=%d\n",
                           xevent.xgeneric.extension,
                           xevent.xgeneric.evtype);
                  break;
#endif
                case PropertyNotify:
                  {
                    char * property_name = XGetAtomName(xevent.xproperty.display, xevent.xproperty.atom);
                    GFX2_Log(GFX2_DEBUG, "SDL_SYSWMEVENT x11 PropertyNotify : %s %s\n",
                             (xevent.xproperty.state == PropertyNewValue) ? "PropertyNewValue" : "PropertyDelete",
                             property_name);
                    XFree(property_name);
                    if (xevent.xproperty.atom == XInternAtom(xevent.xproperty.display, "_NET_WM_STATE", False))
                    {
                      Atom prop_type = None;
                      int format = 0;
                      unsigned long n_items, bytes_after_return;
                      unsigned char * data = NULL;
                      if (XGetWindowProperty(xevent.xproperty.display, xevent.xproperty.window,
                                             xevent.xproperty.atom, 0, 0x10000, False, AnyPropertyType,
                                             &prop_type, &format, &n_items, &bytes_after_return,
                                             &data) == Success)
                      {
                        GFX2_Log(GFX2_DEBUG, "%d format=%d n_items=%lu bytes_after_return=%lu\n",
                                 prop_type, format, n_items, bytes_after_return);
                        if (prop_type == XA_ATOM)
                        {
                          unsigned long i;
                          Atom * array = (Atom *)data;
                          Atom hidden = XInternAtom(xevent.xproperty.display, "_NET_WM_STATE_HIDDEN", False);
                          Atom max_horz = XInternAtom(xevent.xproperty.display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
                          Atom max_vert = XInternAtom(xevent.xproperty.display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
                          Window_state = GFX2_WINDOW_STANDARD;
                          for (i = 0; i < n_items; i++)
                          {
                            char * atom_name = XGetAtomName(xevent.xproperty.display, array[i]);
                            GFX2_Log(GFX2_DEBUG, "  %s\n", atom_name);
                            XFree(atom_name);
                            if (array[i] == hidden)
                              Window_state = GFX2_WINDOW_MINIMIZED;
                            else if (array[i] == max_horz || array[i] == max_vert)
                              Window_state = GFX2_WINDOW_MAXIMIZED;
                          }
                        }
                        XFree(data);
                      }
                      else
                      {
                        GFX2_Log(GFX2_WARNING, "XGetWindowProperty() failed\n");
                      }
                    }
                  }
                  break;
                default:
                  GFX2_Log(GFX2_DEBUG, "Unhandled SDL_SYSWMEVENT x11 event type=%d\n", xevent.type);
              }
#undef xevent
#endif
              break;
          
          default:
              GFX2_Log(GFX2_DEBUG, "Unhandled SDL event number : %d\n",event.type);
              break;
      }
    }
    // Directional and analog controller
    if (!(Digital_joystick_state||Directional_emulated)
      #if defined(USE_JOYSTICK)
      && !(Joystick_vertical != 0 || Joystick_horizontal != 0)
      #endif
    )
    {
       Directional_first_move=0;
    }
    else
    {
      long time_now;
      int step=0;
      #if defined(USE_JOYSTICK)
      int joy_step = 0;
      #endif
      time_now=GFX2_GetTicks();
      
      if (Directional_first_move==0)
      {
        Directional_first_move=time_now;
        step=1;
      }
      else
      {
        // Compute how much the cursor has moved since last call.
        // This tries to make smooth cursor movement
        // no matter the frequency of calls to Get_input()
        step =
          Directional_acceleration(time_now - Directional_first_move) -
          Directional_acceleration(Directional_last_move - Directional_first_move);
        
        // Clip speed at 3 pixel per visible frame.
        if (step > PAD_MAX_SPEED)
          step = PAD_MAX_SPEED;

        #if defined(USE_JOYSTICK)
        joy_step =
          Directional_acceleration(time_now) -
          Directional_acceleration(Directional_last_move);

        if (joy_step > STICK_MAX_SPEED)
          joy_step = STICK_MAX_SPEED;
        #endif
      }
      Directional_last_move = time_now;
      #if defined(USE_JOYSTICK)
      // Analog joystick
      if (Joystick_vertical != 0 || Joystick_horizontal != 0)
      {
        Cursor_displace(joy_step, Joystick_horizontal, Joystick_vertical);
      }
      #endif

      if (step)
      {
        long delta_x = 0;
        long delta_y = 0;
        // Directional controller UP
        if ((Digital_joystick_state & (D_JOYSTICK_UP | D_JOYSTICK_UP_RIGHT | D_JOYSTICK_UP_LEFT))
            || (Directional_emulated & D_JOYSTICK_UP))
          delta_y -= 1 << 16;
        // Directional controller DOWN
        if ((Digital_joystick_state & (D_JOYSTICK_DOWN | D_JOYSTICK_DOWN_RIGHT | D_JOYSTICK_DOWN_LEFT))
            || (Directional_emulated & D_JOYSTICK_DOWN))
          delta_y += 1 << 16;

        // Directional controller RIGHT
        if ((Digital_joystick_state & (D_JOYSTICK_RIGHT | D_JOYSTICK_UP_RIGHT | D_JOYSTICK_DOWN_RIGHT))
            || (Directional_emulated & D_JOYSTICK_RIGHT))
          delta_x += 1 << 16;
        // Directional controller LEFT
        if ((Digital_joystick_state & (D_JOYSTICK_LEFT | D_JOYSTICK_UP_LEFT | D_JOYSTICK_DOWN_LEFT))
            || (Directional_emulated & D_JOYSTICK_LEFT))
          delta_x -= 1 << 16;

        if (delta_x != 0 || delta_y != 0)
        {
          Cursor_displace(step, delta_x, delta_y);
        }
      }
    }
    // If the cursor was moved since last update,
    // it was erased, so we need to redraw it (with the preview brush)
    if (Mouse_moved)
    {
      Compute_paintbrush_coordinates();
      Display_cursor();
#if defined(USE_SDL2) && defined(USE_JOYSTICK)
      // To achieve mouse smooth movement using a joystick, we update the screen now and return
      // this is because for mouse coordinate the joystick use floats internally and Mouse_moved become always
      // true and stop updating the screen until the axis are released.
      GFX2_UpdateScreen();
#endif
      return 1;
    }
    if (user_feedback_required)
      return 1;

#if defined(USE_SDL2)
    GFX2_UpdateScreen();
#endif
    // Nothing significant happened
    if (sleep_time)
      SDL_Delay(sleep_time);
#elif defined(WIN32)
    MSG msg;

    user_feedback_required = 0;
    Key_ANSI = 0;
    Key_UNICODE = 0;
    Key = 0;
    Mouse_moved=0;
    Input_new_mouse_X = Mouse_X;
    Input_new_mouse_Y = Mouse_Y;
    Input_new_mouse_K = Mouse_K;

    Color_cycling();
    // Commit any pending screen update.
    // This is done in this function because it's called after reading
    // some user input.
    Flush_update();

    if (Quit_is_required)
      return 1;

    while (!user_feedback_required && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // If the cursor was moved since last update,
    // it was erased, so we need to redraw it (with the preview brush)
    if (Mouse_moved)
    {
      Compute_paintbrush_coordinates();
      Display_cursor();
      return 1;
    }
    if (user_feedback_required)
    {
      // Process the WM_CHAR event that follow WM_KEYDOWN
      if(PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      return 1;
    }
    if (sleep_time == 0)
      sleep_time = 20;  // default of 20 ms
    // TODO : we should check where Get_input(0) is called
    {
      UINT_PTR timerId = SetTimer(NULL, 0, sleep_time, NULL);
      WaitMessage();
      KillTimer(NULL, timerId);
    }
#elif defined(USE_X11)
    int user_feedback_required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer

    Color_cycling();
    // Commit any pending screen update.
    // This is done in this function because it's called after reading 
    // some user input.
    Flush_update();

    Key_ANSI = 0;
    Key_UNICODE = 0;
    Key = 0;
    Mouse_moved=0;
    Input_new_mouse_X = Mouse_X;
    Input_new_mouse_Y = Mouse_Y;
    Input_new_mouse_K = Mouse_K;

    if (X11_display == NULL)
      return 0;
    XFlush(X11_display);
    while(!user_feedback_required && XPending(X11_display) > 0)
    {
      word mod = 0;
      XEvent event;
      XNextEvent(X11_display, &event);
      switch(event.type)
      {
        case KeyPress:
          {
            KeySym sym;
            mod = X11_to_GFX2_Modifiers(event.xkey.state);
            //sym = XKeycodeToKeysym(X11_display, event.xkey.keycode, 0);
            sym = XkbKeycodeToKeysym(X11_display, event.xkey.keycode, 0, 0);
            GFX2_Log(GFX2_DEBUG, "key press code = %3d state=0x%08x sym = 0x%04lx %s\tmod=%04x\n",
                     event.xkey.keycode, event.xkey.state, sym, XKeysymToString(sym), mod);
            if (sym == XK_Shift_L || sym == XK_Shift_R ||
                sym == XK_Control_L || sym == XK_Control_R ||
                sym == XK_Alt_L || sym == XK_Alt_R || sym == XK_ISO_Level3_Shift || // ALT GR
                sym == XK_Super_L || sym == XK_Super_R)
              break;    // ignore shift/ctrl/alt/windows alone
            Key = mod | (sym & 0x0fff);
            //sym = XkbKeycodeToKeysym(X11_display, event.xkey.keycode, 0, event.xkey.state);
            if (((sym & 0xf000) != 0xf000) || IsKeypadKey(sym)) // test for standard key or KeyPad
            {
              int count;
              char buffer[16];
              static XComposeStatus status;
              count = XLookupString(&event.xkey, buffer, sizeof(buffer),
                                    &sym, &status);
              if (count == 1)
              {
                Key_ANSI = Key_UNICODE = (word)buffer[0] & 0x00ff;
              }
              else if((sym & 0xf000) != 0xf000)
              {
                Key_UNICODE = sym;
                if (sym < 0x100)
                  Key_ANSI = sym;
              }
            }
            else
            {
              Key_UNICODE = Key_ANSI = 0;
            }
            Handle_special_key_press();
            user_feedback_required = 1;
          }
          break;
        case KeyRelease:
          {
            KeySym sym;

            mod = X11_to_GFX2_Modifiers(event.xkey.state);
            sym = XkbKeycodeToKeysym(X11_display, event.xkey.keycode, 0, 0);
            GFX2_Log(GFX2_DEBUG, "keyrelease code= %3d state=0x%08x sym = 0x%04lx %s\tmod=%04x\n",
                     event.xkey.keycode, event.xkey.state, sym, XKeysymToString(sym), mod);
            Release_control(sym & 0x0fff, mod);
          }
          break;
        case ButtonPress: // left = 1, middle = 2, right = 3, wheelup = 4, wheeldown = 5
          XGrabPointer(X11_display, X11_window, True,
                       PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                       GrabModeAsync, GrabModeAsync,
                       X11_window, None, CurrentTime);
          //printf("Press button = %d state = 0x%08x\n", event.xbutton.button, event.xbutton.state);
          mod = X11_to_GFX2_Modifiers(event.xbutton.state);
          switch(event.xbutton.button)
          {
            case 1:
            case 3:
              {
                byte mask = 1;
                if(event.xbutton.button == 3)
                  mask ^= 3;
                if (Button_inverter)
                  mask ^= 3;
                Input_new_mouse_K |= mask;
                user_feedback_required = Handle_mouse_btn_change();
              }
              break;
            case 2:
              Key = KEY_MOUSEMIDDLE | mod;
              user_feedback_required = 1;
              break;
            case 4:
              Key = KEY_MOUSEWHEELUP | mod;
              user_feedback_required = 1;
              break;
            case 5:
              Key = KEY_MOUSEWHEELDOWN | mod;
              user_feedback_required = 1;
              break;
          }
          break;
        case ButtonRelease:
          XUngrabPointer(X11_display, CurrentTime);
          mod = X11_to_GFX2_Modifiers(event.xbutton.state);
          //printf("Release button = %d\n", event.xbutton.button);
          if(event.xbutton.button == 1 || event.xbutton.button == 3)
          {
            byte mask = 1;
            if(event.xbutton.button == 3)
              mask ^= 3;
            if (Button_inverter)
              mask ^= 3;
            Input_new_mouse_K &= ~mask;
            user_feedback_required = Handle_mouse_btn_change();
          }
          break;
        case MotionNotify:
          //printf("mouse %dx%d\n", event.xmotion.x, event.xmotion.y);
          mod = X11_to_GFX2_Modifiers(event.xmotion.state);
          user_feedback_required = Move_cursor_with_constraints(event.xmotion.x / Pixel_width, event.xmotion.y / Pixel_height);
          break;
        case Expose:
          GFX2_Log(GFX2_DEBUG, "Expose (%d,%d) (%d,%d)\n", event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height);
          Update_rect(event.xexpose.x, event.xexpose.y,
                      event.xexpose.width, event.xexpose.height);
          break;
        case ConfigureNotify:
          if (event.xconfigure.above == 0)
          {
            int x_pos, y_pos;
            unsigned int width, height, border_width, depth;
            Window root_window;

            Resize_width = event.xconfigure.width;
            Resize_height = event.xconfigure.height;
            if (XGetGeometry(X11_display, X11_window, &root_window, &x_pos, &y_pos, &width, &height, &border_width, &depth))
            {
              Config.Window_pos_x = event.xconfigure.x - x_pos;
              Config.Window_pos_y = event.xconfigure.y - y_pos;
            }
          }
          break;
        case ClientMessage:
          if (event.xclient.message_type == XInternAtom(X11_display,"WM_PROTOCOLS", False))
          {
            if ((Atom)event.xclient.data.l[0] == XInternAtom(X11_display, "WM_DELETE_WINDOW", False))
            {
              Quit_is_required = 1;
              user_feedback_required = 1;
            }
            else
            {
              char * atom_name = XGetAtomName(X11_display, (Atom)event.xclient.data.l[0]);
              GFX2_Log(GFX2_INFO, "unrecognized WM event : %s\n", atom_name);
              XFree(atom_name);
            }
          }
          else
            Handle_ClientMessage(&event.xclient);
          break;
        case SelectionNotify:
          if (Handle_SelectionNotify(&event.xselection))
            user_feedback_required = 1;
          break;
        case SelectionClear:
          GFX2_Log(GFX2_DEBUG, "X11 SelectionClear\n");
          if (X11_clipboard)
          {
            free(X11_clipboard);
            X11_clipboard = NULL;
            X11_clipboard_size = 0;
            X11_clipboard_type = X11_CLIPBOARD_NONE;
          }
          break;
        case SelectionRequest:
          Handle_SelectionRequest(&event.xselectionrequest);
          break;
        case ReparentNotify:
          GFX2_Log(GFX2_DEBUG, "X11 ReparentNotify\n");
          break;
        case MapNotify:
          GFX2_Log(GFX2_DEBUG, "X11 MapNotify\n");
          break;
        default:
          GFX2_Log(GFX2_INFO, "X11 event.type = %d not handled\n", event.type);
      }
    }
    // If the cursor was moved since last update,
    // it was erased, so we need to redraw it (with the preview brush)
    if (Mouse_moved)
    {
      Compute_paintbrush_coordinates();
      Display_cursor();
      return 1;
    }
    if (user_feedback_required)
      return 1;
    // Nothing significant happened
    if (sleep_time)
      usleep(1000 * sleep_time);
#endif
    return 0;
}

void Adjust_mouse_sensitivity(word fullscreen)
{
  // Deprecated
  (void)fullscreen;
}

static int Color_cycling(void)
{
  static byte offset[16];
  int i, color;
  int changed; // boolean : true if the palette needs a change in this tick.
  const T_Gradient_range * range;
  int len;
  
  long now;
  static long start=0;
  
  if (start==0)
  {
    // First run
    start = GFX2_GetTicks();
    return 1;
  }
  if (!Allow_colorcycling || !Cycling_mode)
    return 1;
    

  now = GFX2_GetTicks();
  changed=0;
  
  // Check all cycles for a change at this tick
  for (i=0; i<16; i++)
  {
    range = &Main.backups->Pages->Gradients->Range[i];
    len = range->End-range->Start+1;
    if (len>1 && range->Speed)
    {
      int new_offset;
      
      new_offset=(now-start)/(int)(1000.0/(range->Speed*0.2856)) % len;
      if (!range->Inverse)
        new_offset=len - new_offset;
      
      if (new_offset!=offset[i])
        changed=1;
      offset[i]=new_offset;
    }
  }
  if (changed)
  {
    T_Palette palette;
    // Initialize the palette
    memcpy(palette, Main.palette, sizeof(T_Palette));
    for (i=0; i<16; i++)
    {
      range = &Main.backups->Pages->Gradients->Range[i];
      len = range->End-range->Start+1;
      if (len>1 && range->Speed)
      {
        for(color=range->Start;color<=range->End;color++)
        {
          int new_color = range->Start+((color-range->Start+offset[i])%len);
          palette[color] = Main.palette[new_color];
        }
      }
    }
    SetPalette(palette, 0, 256);
  }
  return 0;
}
