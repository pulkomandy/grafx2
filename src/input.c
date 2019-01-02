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
#include <SDL_syswm.h>
#endif

#ifdef WIN32
  #include <windows.h>
  #include <shellapi.h>
#endif

#ifdef USE_X11
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
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

#if defined(USE_SDL)
void Handle_window_resize(SDL_ResizeEvent event);
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
void Handle_window_exit(SDL_QuitEvent event);
#endif
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
#endif

// --

// Digital joystick state
byte Directional_up;
byte Directional_up_right;
byte Directional_right;
byte Directional_down_right;
byte Directional_down;
byte Directional_down_left;
byte Directional_left;
byte Directional_up_left;
byte Directional_click;

// Emulated directional controller.
// This has a distinct state from Directional_, because some joysticks send
// "I'm not moving" SDL events when idle, thus stopping the emulated one.
byte Directional_emulated_up;
byte Directional_emulated_right;
byte Directional_emulated_down;
byte Directional_emulated_left;

long Directional_first_move;
long Directional_last_move;
int  Mouse_moved; ///< Boolean, Set to true if any cursor movement occurs.

word Input_new_mouse_X;
word Input_new_mouse_Y;
byte Input_new_mouse_K;
byte Button_inverter=0; // State of the key that swaps mouse buttons.

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

// Called each time there is a cursor move, either triggered by mouse or keyboard shortcuts
int Move_cursor_with_constraints()
{
  int feedback=0;
  int  mouse_blocked=0; ///< Boolean, Set to true if mouse movement was clipped.

  
  // Clip mouse to the editing area. There can be a border when using big 
  // pixels, if the SDL screen dimensions are not factors of the pixel size.
  if (Input_new_mouse_Y>=Screen_height)
  {
      Input_new_mouse_Y=Screen_height-1;
      mouse_blocked=1;
  }
  if (Input_new_mouse_X>=Screen_width)
  {
      Input_new_mouse_X=Screen_width-1;
      mouse_blocked=1;
  }
  //Gestion "avancée" du curseur: interdire la descente du curseur dans le
  //menu lorsqu'on est en train de travailler dans l'image
  if (Operation_stack_size != 0)
  {
        

        //Si le curseur ne se trouve plus dans l'image
        if(Menu_Y<=Input_new_mouse_Y)
        {
            //On bloque le curseur en fin d'image
            mouse_blocked=1;
            Input_new_mouse_Y=Menu_Y-1; //La ligne !!au-dessus!! du menu
        }

        if(Main.magnifier_mode)
        {
            if(Operation_in_magnifier==0)
            {
                if(Input_new_mouse_X>=Main.separator_position)
                {
                    mouse_blocked=1;
                    Input_new_mouse_X=Main.separator_position-1;
                }
            }
            else
            {
                if(Input_new_mouse_X<Main.X_zoom)
                {
                    mouse_blocked=1;
                    Input_new_mouse_X=Main.X_zoom;
                }
            }
        }
  }
  if ((Input_new_mouse_X != Mouse_X) ||
    (Input_new_mouse_Y != Mouse_Y) ||
    (Input_new_mouse_K != Mouse_K))
  {
    // On every change of mouse state
    if ((Input_new_mouse_K != Mouse_K))
    {
      feedback=1;
      
      if (Input_new_mouse_K == 0)
      {
        Input_sticky_control = 0;
      }
    }
    // Hide cursor, because even just a click change needs it
    if (!Mouse_moved)
    {
      // Hide cursor (erasing icon and brush on screen
      // before changing the coordinates.
      Hide_cursor();
    }
    Mouse_moved++;
    if (Input_new_mouse_X != Mouse_X || Input_new_mouse_Y != Mouse_Y)
    {
      Mouse_X=Input_new_mouse_X;
      Mouse_Y=Input_new_mouse_Y;
    }
    Mouse_K=Input_new_mouse_K;
    
    if (Mouse_moved > Config.Mouse_merge_movement
      && !Operation[Current_operation][Mouse_K_unique]
          [Operation_stack_size].Fast_mouse)
        feedback=1;
  }
  if (mouse_blocked)
    Set_mouse_position();
  return feedback;
}

// WM events management

#if defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
/**
 * Drag'n'Drop Protocol for X11 :
 * https://freedesktop.org/wiki/Specifications/XDND/
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
    //int list = xclient->data.l[1] & 1;
    xdnd_version = xclient->data.l[1] >> 24;
    xdnd_source = xclient->data.l[0];
    GFX2_Log(GFX2_DEBUG, "XdndEnter version=%d source=%lu\n", xdnd_version, xdnd_source);
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
          False, xselection->target /* type */, &type, &format,
          &count, &bytesAfter, &value);
      if (r == Success && value != NULL)
      {
        char * type_name = XGetAtomName(X11_display, type);
        GFX2_Log(GFX2_DEBUG, "Clipboard value=%p %lu bytes format=%d type=%s\n",
                 value, count, format, type_name);
        XFree(type_name);
        X11_clipboard_size = count;
        if (xselection->target == XInternAtom(X11_display, "UTF8_STRING", False))
          X11_clipboard = strdup((char *)value); // Text Clipboard
        else if (xselection->target == XInternAtom(X11_display, "image/png", False))
        { // Picture clipboard (PNG)
          X11_clipboard = malloc(count);
          if (X11_clipboard != NULL)
            memcpy(X11_clipboard, value, count);
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
    GFX2_Log(GFX2_INFO, "Unhandled SelectNotify selection=%s\n", selection_name);
    XFree(selection_name);
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
void Handle_window_resize(SDL_ResizeEvent event)
{
    Resize_width = event.w;
    Resize_height = event.h;
}
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
void Handle_window_exit(SDL_QuitEvent event)
{
    (void)event, // unused
    
    Quit_is_required = 1;
}

// Mouse events management

int Handle_mouse_move(SDL_MouseMotionEvent event)
{
    Input_new_mouse_X = event.x/Pixel_width;
    Input_new_mouse_Y = event.y/Pixel_height;

    return Move_cursor_with_constraints();
}

int Handle_mouse_click(SDL_MouseButtonEvent event)
{
    switch(event.button)
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
        return 0;
    }
    return Move_cursor_with_constraints();
}

int Handle_mouse_release(SDL_MouseButtonEvent event)
{
    switch(event.button)
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
    
    return Move_cursor_with_constraints();
}
#endif

// Keyboard management

#if defined(USE_SDL) || defined(USE_SDL2)
int Handle_key_press(SDL_KeyboardEvent event)
{
    //Appui sur une touche du clavier
    int modifier;
  
    Key = Keysym_to_keycode(event.keysym);
    Key_ANSI = Keysym_to_ANSI(event.keysym);
#if defined(USE_SDL)
    Key_UNICODE = event.keysym.unicode;
    if (Key_UNICODE == 0)
#endif
      Key_UNICODE = Key_ANSI;
    switch(event.keysym.sym)
    {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
        modifier=MOD_SHIFT;
        break;

      case SDLK_RCTRL:
      case SDLK_LCTRL:
        modifier=MOD_CTRL;
        break;

      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_MODE:
        modifier=MOD_ALT;
        break;

#if defined(USE_SDL2)
      case SDLK_RGUI:
      case SDLK_LGUI:
#else
      case SDLK_RMETA:
      case SDLK_LMETA:
#endif
        modifier=MOD_META;
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
        return Move_cursor_with_constraints();
      }
    }
    #ifdef RSUPER_EMULATES_META_MOD
    if (Key==SDLK_RSUPER)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_META);
      Key=0;
    }
    #endif

    if(Is_shortcut(Key,SPECIAL_MOUSE_UP))
    {
      Directional_emulated_up=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_DOWN))
    {
      Directional_emulated_down=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_LEFT))
    {
      Directional_emulated_left=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_RIGHT))
    {
      Directional_emulated_right=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_CLICK_LEFT) && Keyboard_click_allowed > 0)
    {
        Input_new_mouse_K=1;
        Directional_click=1;
        return Move_cursor_with_constraints();
    }
    else if(Is_shortcut(Key,SPECIAL_CLICK_RIGHT) && Keyboard_click_allowed > 0)
    {
        Input_new_mouse_K=2;
        Directional_click=2;
        return Move_cursor_with_constraints();
    }
    else if(Is_shortcut(Key,SPECIAL_HOLD_PAN))
    {
      Pan_shortcut_pressed=1;
      return 0;
    }
    return 0;
}

int Release_control(int key_code, int modifier)
{
    int need_feedback = 0;

    if (modifier == MOD_SHIFT)
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
        return Move_cursor_with_constraints();
      }
    }

    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][1]&modifier))
    {
      Directional_emulated_up=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][1]&modifier))
    {
      Directional_emulated_down=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][1]&modifier))
    {
      Directional_emulated_left=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][1]&modifier))
    {
      Directional_emulated_right=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_CLICK_LEFT][0]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_LEFT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_CLICK_LEFT][1]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_LEFT][1]&modifier))
    {
        if (Directional_click & 1)
        {
            Directional_click &= ~1;
            Input_new_mouse_K &= ~1;
            return Move_cursor_with_constraints() || need_feedback;
        }
    }
    if((key_code && key_code == (Config_Key[SPECIAL_CLICK_RIGHT][0]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_RIGHT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_CLICK_RIGHT][1]&0x0FFF)) || (Config_Key[SPECIAL_CLICK_RIGHT][1]&modifier))
    {
        if (Directional_click & 2)
        {
            Directional_click &= ~2;
            Input_new_mouse_K &= ~2;
            return Move_cursor_with_constraints() || need_feedback;
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


int Handle_key_release(SDL_KeyboardEvent event)
{
    int modifier;
    int released_key = Keysym_to_keycode(event.keysym) & 0x0FFF;
  
    switch(event.keysym.sym)
    {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
        modifier=MOD_SHIFT;
        break;

      case SDLK_RCTRL:
      case SDLK_LCTRL:
        modifier=MOD_CTRL;
        break;

      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_MODE:
        modifier=MOD_ALT;
        break;

      #ifdef RSUPER_EMULATES_META_MOD
      case SDLK_RSUPER:
        SDL_SetModState(SDL_GetModState() & ~KMOD_META);
        modifier=MOD_META;
        break;
      #endif
      
#if defined(USE_SDL2)
      case SDLK_RGUI:
      case SDLK_LGUI:
#else
      case SDLK_RMETA:
      case SDLK_LMETA:
#endif
        modifier=MOD_META;
        break;

      default:
        modifier=0;
    }
    return Release_control(released_key, modifier);
}
#endif


// Joystick management

#if defined(USE_JOYSTICK) && (defined(USE_SDL) || defined(USE_SDL2))
static int Handle_joystick_press(SDL_JoyButtonEvent event)
{
    if (event.button == Joybutton_shift)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_SHIFT);
      return 0;
    }
    if (event.button == Joybutton_control)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_CTRL);
      if (Config.Swap_buttons == MOD_CTRL && Button_inverter==0)
      {
        Button_inverter=1;
        if (Input_new_mouse_K)
        {
          Input_new_mouse_K ^= 3; // Flip bits 0 and 1
          return Move_cursor_with_constraints();
        }
      }
      return 0;
    }
    if (event.button == Joybutton_alt)
    {
#if defined(USE_SDL)
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_META));
#else
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_GUI));
#endif
      if (Config.Swap_buttons == MOD_ALT && Button_inverter==0)
      {
        Button_inverter=1;
        if (Input_new_mouse_K)
        {
          Input_new_mouse_K ^= 3; // Flip bits 0 and 1
          return Move_cursor_with_constraints();
        }
      }
      return 0;
    }
    if (event.button == Joybutton_left_click)
    {
      Input_new_mouse_K = Button_inverter ? 2 : 1;
      return Move_cursor_with_constraints();
    }
    if (event.button == Joybutton_right_click)
    {
      Input_new_mouse_K = Button_inverter ? 1 : 2;
      return Move_cursor_with_constraints();
    }
    switch(event.button)
    {
      #ifdef JOY_BUTTON_UP
      case JOY_BUTTON_UP:
        Directional_up=1;
        break;
      #endif
      #ifdef JOY_BUTTON_UPRIGHT
      case JOY_BUTTON_UPRIGHT:
        Directional_up_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_RIGHT
      case JOY_BUTTON_RIGHT:
        Directional_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNRIGHT
      case JOY_BUTTON_DOWNRIGHT:
        Directional_down_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWN
      case JOY_BUTTON_DOWN:
        Directional_down=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNLEFT
      case JOY_BUTTON_DOWNLEFT:
        Directional_down_left=1;
        break;
      #endif
      #ifdef JOY_BUTTON_LEFT
      case JOY_BUTTON_LEFT:
        Directional_left=1;
        break;
      #endif
      #ifdef JOY_BUTTON_UPLEFT
      case JOY_BUTTON_UPLEFT:
        Directional_up_left=1;
        break;
      #endif
      
      default:
        break;
    }
      
    Key = (KEY_JOYBUTTON+event.button)|Get_Key_modifiers();
    // TODO: systeme de répétition
    
    return Move_cursor_with_constraints();
}

static int Handle_joystick_release(SDL_JoyButtonEvent event)
{
    if (event.button == Joybutton_shift)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_SHIFT);
      return Release_control(0,MOD_SHIFT);
    }
    if (event.button == Joybutton_control)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_CTRL);
      return Release_control(0,MOD_CTRL);
    }
    if (event.button == Joybutton_alt)
    {
#if defined(USE_SDL)
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_META));
#else
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_GUI));
#endif
      return Release_control(0,MOD_ALT);
    }
    if (event.button == Joybutton_left_click)
    {
      Input_new_mouse_K &= ~1;
      return Move_cursor_with_constraints();
    }
    if (event.button == Joybutton_right_click)
    {
      Input_new_mouse_K &= ~2;
      return Move_cursor_with_constraints();
    }
  
    switch(event.button)
    {
      #ifdef JOY_BUTTON_UP
      case JOY_BUTTON_UP:
        Directional_up=1;
        break;
      #endif
      #ifdef JOY_BUTTON_UPRIGHT
      case JOY_BUTTON_UPRIGHT:
        Directional_up_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_RIGHT
      case JOY_BUTTON_RIGHT:
        Directional_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNRIGHT
      case JOY_BUTTON_DOWNRIGHT:
        Directional_down_right=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWN
      case JOY_BUTTON_DOWN:
        Directional_down=1;
        break;
      #endif
      #ifdef JOY_BUTTON_DOWNLEFT
      case JOY_BUTTON_DOWNLEFT:
        Directional_down_left=1;
        break;
      #endif
      #ifdef JOY_BUTTON_LEFT
      case JOY_BUTTON_LEFT:
        Directional_left=1;
        break;
      #endif
      #ifdef JOY_BUTTON_UPLEFT
      case JOY_BUTTON_UPLEFT:
        Directional_up_left=1;
        break;
      #endif
      
      default:
        break;
    }
  return Move_cursor_with_constraints();
}

static void Handle_joystick_movement(SDL_JoyAxisEvent event)
{
    if (event.axis==JOYSTICK_AXIS_X)
    {
      Directional_right=Directional_left=0;
      if (event.value<-JOYSTICK_THRESHOLD)
      {
        Directional_left=1;
      }
      else if (event.value>JOYSTICK_THRESHOLD)
        Directional_right=1;
    }
    else if (event.axis==JOYSTICK_AXIS_Y)
    {
      Directional_up=Directional_down=0;
      if (event.value<-JOYSTICK_THRESHOLD)
      {
        Directional_up=1;
      }
      else if (event.value>JOYSTICK_THRESHOLD)
        Directional_down=1;
    }
}
#endif

// Attempts to move the mouse cursor by the given deltas (may be more than 1 pixel at a time)
int Cursor_displace(short delta_x, short delta_y)
{
  short x=Input_new_mouse_X;
  short y=Input_new_mouse_Y;
  
  if(Main.magnifier_mode && Input_new_mouse_Y < Menu_Y && Input_new_mouse_X > Main.separator_position)
  {
    // Cursor in zoomed area
    
    if (delta_x<0)
      Input_new_mouse_X = Max(Main.separator_position, x-Main.magnifier_factor);
    else if (delta_x>0)
      Input_new_mouse_X = Min(Screen_width-1, x+Main.magnifier_factor);
    if (delta_y<0)
      Input_new_mouse_Y = Max(0, y-Main.magnifier_factor);
    else if (delta_y>0)
      Input_new_mouse_Y = Min(Screen_height-1, y+Main.magnifier_factor);
  }
  else
  {
    if (delta_x<0)
      Input_new_mouse_X = Max(0, x+delta_x);
    else if (delta_x>0)
      Input_new_mouse_X = Min(Screen_width-1, x+delta_x);
    if (delta_y<0)
      Input_new_mouse_Y = Max(0, y+delta_y);
    else if (delta_y>0)
      Input_new_mouse_Y = Min(Screen_height-1, y+delta_y);
  }
  return Move_cursor_with_constraints();
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
    Key_ANSI = 0;
    Key_UNICODE = 0;
    Key = 0;
#if defined(USE_SDL2)
    memset(Key_Text, 0, sizeof(Key_Text));
#endif
    Mouse_moved=0;
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
              Handle_window_resize(event.resize);
              user_feedback_required = 1;
              break;
#endif

#if defined(USE_SDL2)
          case SDL_WINDOWEVENT:
              switch(event.window.event)
              {
                  case SDL_WINDOWEVENT_RESIZED: // change by external event (user or window manager)
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
                      Quit_is_required = 1;
                      user_feedback_required = 1;
                      break;
              }
              break;
#endif

          case SDL_QUIT:
              Handle_window_exit(event.quit);
              user_feedback_required = 1;
              break;

          case SDL_MOUSEMOTION:
              user_feedback_required = Handle_mouse_move(event.motion);
              break;

          case SDL_MOUSEBUTTONDOWN:
              Handle_mouse_click(event.button);
              user_feedback_required = 1;
              break;

          case SDL_MOUSEBUTTONUP:
              Handle_mouse_release(event.button);
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
              Handle_key_press(event.key);
              user_feedback_required = 1;
              break;

          case SDL_KEYUP:
              Handle_key_release(event.key);
              break;

#if defined(USE_SDL2)
          case SDL_TEXTINPUT:
              memcpy(Key_Text, event.text.text, sizeof(Key_Text));
              user_feedback_required = 1;
              break;
#endif

          // Start of Joystik handling
          #ifdef USE_JOYSTICK

          case SDL_JOYBUTTONUP:
              Handle_joystick_release(event.jbutton);
              user_feedback_required = 1;
              break;

          case SDL_JOYBUTTONDOWN:
              Handle_joystick_press(event.jbutton);
              user_feedback_required = 1;
              break;

          case SDL_JOYAXISMOTION:
              Handle_joystick_movement(event.jaxis);
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
#endif

          case SDL_SYSWMEVENT:
#ifdef __WIN32__
              if(event.syswm.msg->msg  == WM_DROPFILES)
              {
                int file_count;
                HDROP hdrop = (HDROP)(event.syswm.msg->wParam);
                if((file_count = DragQueryFile(hdrop,(UINT)-1,(LPTSTR) NULL ,(UINT) 0)) > 0)
                {
                  long len;
                  // Query filename length
                  len = DragQueryFile(hdrop,0 ,NULL ,0);
                  if (len)
                  {
                    Drop_file_name=calloc(len+1,1);
                    if (Drop_file_name)
                    {
#ifdef UNICODE
                      TCHAR LongDropFileName[MAX_PATH];
                      TCHAR ShortDropFileName[MAX_PATH];
                      if (DragQueryFile(hdrop, 0 , LongDropFileName ,(UINT) MAX_PATH)
                        && GetShortPathName(LongDropFileName, ShortDropFileName, MAX_PATH))
                      {
                        int i;
                        for (i = 0; ShortDropFileName[i] != 0; i++)
                          Drop_file_name[i] = (char)ShortDropFileName[i];
                        Drop_file_name[i] = 0;
                      }
#else
                      if (DragQueryFile(hdrop,0 ,(LPTSTR) Drop_file_name ,(UINT) MAX_PATH))
                      {
                        // Success
                      }
#endif
                      else
                      {
                        free(Drop_file_name);
                        // Don't report name copy error
                      }
                    }
                    else
                    {
                      // Don't report alloc error (for a file name? :/ )
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
                    X11_clipboard_size = 0;
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
                  GFX2_Log(GFX2_DEBUG, "SDL_SYSWMEVENT x11 PropertyNotify\n");
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
    // Directional controller
    if (!(Directional_up||Directional_up_right||Directional_right||
      Directional_down_right||Directional_down||Directional_down_left||
      Directional_left||Directional_up_left||Directional_emulated_up||
      Directional_emulated_right||Directional_emulated_down||
      Directional_emulated_left))
    {
       Directional_first_move=0;
    }
    else
    {
      long time_now;
      int step=0;
      
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
        if (step > 3)
          step=3;
        
      }
      Directional_last_move = time_now;
      if (step)
      {
        // Directional controller UP
        if ((Directional_up||Directional_emulated_up||Directional_up_left||Directional_up_right) &&
           !(Directional_down_right||Directional_down||Directional_emulated_down||Directional_down_left))
        {
          Cursor_displace(0, -step);
        }
        // Directional controller RIGHT
        if ((Directional_up_right||Directional_right||Directional_emulated_right||Directional_down_right) &&
           !(Directional_down_left||Directional_left||Directional_emulated_left||Directional_up_left))
        {
          Cursor_displace(step,0);
        }    
        // Directional controller DOWN
        if ((Directional_down_right||Directional_down||Directional_emulated_down||Directional_down_left) &&
           !(Directional_up_left||Directional_up||Directional_emulated_up||Directional_up_right))
        {
          Cursor_displace(0, step);
        }
        // Directional controller LEFT
        if ((Directional_down_left||Directional_left||Directional_emulated_left||Directional_up_left) &&
           !(Directional_up_right||Directional_right||Directional_emulated_right||Directional_down_right))
        {
          Cursor_displace(-step,0);
        }
      }
    }
    // If the cursor was moved since last update,
    // it was erased, so we need to redraw it (with the preview brush)
    if (Mouse_moved)
    {
      Compute_paintbrush_coordinates();
      Display_cursor();
#if defined(USE_SDL2)
      //GFX2_UpdateScreen();
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

	  while (!user_feedback_required && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
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
      if(PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE)) {
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
            // right/left window 40 Mod4Mask
            // left alt = 8         Mod1Mask
            // right alt = 80       Mod5Mask
            // NumLock = 10         Mod2Mask
            // see "modmap"
            if (event.xkey.state & ShiftMask)
              mod |= MOD_SHIFT;
            if (event.xkey.state & ControlMask)
              mod |= MOD_CTRL;
            if (event.xkey.state & (Mod1Mask | Mod5Mask))
              mod |= MOD_ALT;
            if (event.xkey.state & Mod4Mask)
              mod |= MOD_META;
            //sym = XKeycodeToKeysym(X11_display, event.xkey.keycode, 0);
            sym = XkbKeycodeToKeysym(X11_display, event.xkey.keycode, 0, 0);
            GFX2_Log(GFX2_DEBUG, "key code = %3d state=0x%08x sym = 0x%04lx %s\tmod=%04x\n",
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
              if (count == 1) {
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
            user_feedback_required = 1;
          }
          break;
        case ButtonPress: // left = 1, middle = 2, right = 3, wheelup = 4, wheeldown = 5
          //printf("Press button = %d state = 0x%08x\n", event.xbutton.button, event.xbutton.state);
          if (event.xkey.state & ShiftMask)
            mod |= MOD_SHIFT;
          if (event.xkey.state & ControlMask)
            mod |= MOD_CTRL;
          if (event.xkey.state & (Mod1Mask | Mod5Mask))
            mod |= MOD_ALT;
          if (event.xkey.state & Mod3Mask)
            mod |= MOD_META;
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
                user_feedback_required = Move_cursor_with_constraints();
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
          //printf("Release button = %d\n", event.xbutton.button);
          if(event.xbutton.button == 1 || event.xbutton.button == 3)
          {
            byte mask = 1;
            if(event.xbutton.button == 3)
              mask ^= 3;
            if (Button_inverter)
              mask ^= 3;
            Input_new_mouse_K &= ~mask;
            user_feedback_required = Move_cursor_with_constraints();
          }
          break;
        case MotionNotify:
          //printf("mouse %dx%d\n", event.xmotion.x, event.xmotion.y);
          Input_new_mouse_X = (event.xmotion.x < 0) ? 0 : event.xmotion.x/Pixel_width;
          Input_new_mouse_Y = (event.xmotion.y < 0) ? 0 : event.xmotion.y/Pixel_height;
          user_feedback_required = Move_cursor_with_constraints();
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
            X11_clipboard_size = 0;
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

void Set_mouse_position(void)
{
#if defined(USE_SDL)
    SDL_WarpMouse(Mouse_X*Pixel_width, Mouse_Y*Pixel_height);
#elif defined(USE_SDL2)
    //SDL_WarpMouseInWindow(Window_SDL, Mouse_X*Pixel_width, Mouse_Y*Pixel_height);
#endif
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
