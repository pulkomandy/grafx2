/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

#include <SDL.h>
#include <SDL_syswm.h>

#ifdef __WIN32__
  #include <windows.h>
  #include <ShellApi.h>
#endif

#include "global.h"
#include "keyboard.h"
#include "sdlscreen.h"
#include "windows.h"
#include "errors.h"
#include "misc.h"
#include "input.h"
#include "loadsave.h"

#ifdef __VBCC__
  #define __attribute__(x)
#endif

void Handle_window_resize(SDL_ResizeEvent event);
void Handle_window_exit(SDL_QuitEvent event);
int Color_cycling(__attribute__((unused)) void* useless);

// public Globals (available as extern)

int Input_sticky_control = 0;
int Snap_axis = 0;
int Snap_axis_origin_X;
int Snap_axis_origin_Y;

char * Drop_file_name = NULL;

// --

byte Directional_up;
byte Directional_up_right;
byte Directional_right;
byte Directional_down_right;
byte Directional_down;
byte Directional_down_left;
byte Directional_left;
byte Directional_up_left;
byte Directional_click;

long Directional_delay;
long Directional_last_move;
long Directional_step;
int  Mouse_moved; ///< Boolean, Set to true if any cursor movement occurs.

word Input_new_mouse_X;
word Input_new_mouse_Y;
byte Input_new_mouse_K;

byte Mouse_mode = 0; ///< Mouse mode = 0:normal, 1:emulated with custom sensitivity.
short Mouse_virtual_x_position;
short Mouse_virtual_y_position;
short Mouse_virtual_width;
short Mouse_virtual_height;

// TODO: move to config
#ifdef __GP2X__
short Joybutton_shift=GP2X_BUTTON_L; // Button number that serves as a "shift" modifier
short Joybutton_control=GP2X_BUTTON_R; // Button number that serves as a "ctrl" modifier
short Joybutton_alt=GP2X_BUTTON_CLICK; // Button number that serves as a "alt" modifier
short Joybutton_left_click=GP2X_BUTTON_B; // Button number that serves as left click
short Joybutton_right_click=GP2X_BUTTON_Y; // Button number that serves as right-click
#else
short Joybutton_shift=-1; // Button number that serves as a "shift" modifier
short Joybutton_control=-1; // Button number that serves as a "ctrl" modifier
short Joybutton_alt=-1; // Button number that serves as a "alt" modifier
short Joybutton_left_click=0; // Button number that serves as left click
short Joybutton_right_click=0; // Button number that serves as right-click
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

        if(Main_magnifier_mode)
        {
            if(Operation_in_magnifier==0)
            {
                if(Input_new_mouse_X>=Main_separator_position)
                {
                    mouse_blocked=1;
                    Input_new_mouse_X=Main_separator_position-1;
                }
            }
            else
            {
                if(Input_new_mouse_X<Main_X_zoom)
                {
                    mouse_blocked=1;
                    Input_new_mouse_X=Main_X_zoom;
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

void Handle_window_resize(SDL_ResizeEvent event)
{
    Resize_width = event.w;
    Resize_height = event.h;
}

void Handle_window_exit(__attribute__((unused)) SDL_QuitEvent event)
{
    Quit_is_required = 1;
}

// Mouse events management

int Handle_mouse_move(SDL_MouseMotionEvent event)
{
    if (Mouse_mode == 0)
    {
      Input_new_mouse_X = event.x/Pixel_width;
      Input_new_mouse_Y = event.y/Pixel_height;
    }
    else
    {
      Mouse_virtual_x_position += event.xrel * 12 / Config.Mouse_sensitivity_index_x;
      // Clip
      if (Mouse_virtual_x_position > Mouse_virtual_width)
        Mouse_virtual_x_position = Mouse_virtual_width;
      else if (Mouse_virtual_x_position < 0)
        Mouse_virtual_x_position = 0;
        
      Mouse_virtual_y_position += event.yrel * 12 / Config.Mouse_sensitivity_index_y;
      // Clip
      if (Mouse_virtual_y_position > Mouse_virtual_height)
        Mouse_virtual_y_position = Mouse_virtual_height;
      else if (Mouse_virtual_y_position < 0)
        Mouse_virtual_y_position = 0;
        
      Input_new_mouse_X = Mouse_virtual_x_position / 12 / Pixel_width;
      Input_new_mouse_Y = Mouse_virtual_y_position / 12 / Pixel_height;
    }

    return Move_cursor_with_constraints();
}

int Handle_mouse_click(SDL_MouseButtonEvent event)
{
    switch(event.button)
    {
        case SDL_BUTTON_LEFT:
            Input_new_mouse_K |= 1;
            break;

        case SDL_BUTTON_RIGHT:
            Input_new_mouse_K |= 2;
            break;

        case SDL_BUTTON_MIDDLE:
            Key = KEY_MOUSEMIDDLE|Key_modifiers(SDL_GetModState());
            // TODO: repeat system maybe?
            return 0;

        case SDL_BUTTON_WHEELUP:
            Key = KEY_MOUSEWHEELUP|Key_modifiers(SDL_GetModState());
            return 0;

        case SDL_BUTTON_WHEELDOWN:
            Key = KEY_MOUSEWHEELDOWN|Key_modifiers(SDL_GetModState());
            return 0;
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
            Input_new_mouse_K &= ~1;
            break;

        case SDL_BUTTON_RIGHT:
            Input_new_mouse_K &= ~2;
            break;
    }
    
    return Move_cursor_with_constraints();
}

// Keyboard management

int Handle_key_press(SDL_KeyboardEvent event)
{
    //Appui sur une touche du clavier
    Key = Keysym_to_keycode(event.keysym);
    Key_ANSI = Keysym_to_ANSI(event.keysym);

    if(Is_shortcut(Key,SPECIAL_MOUSE_UP))
    {
      Directional_up=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_DOWN))
    {
      Directional_down=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_LEFT))
    {
      Directional_left=1;
      return 0;
    }
    else if(Is_shortcut(Key,SPECIAL_MOUSE_RIGHT))
    {
      Directional_right=1;
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

    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_UP][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_UP][1]&modifier))
    {
      Directional_up=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_DOWN][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_DOWN][1]&modifier))
    {
      Directional_down=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_LEFT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_LEFT][1]&modifier))
    {
      Directional_left=0;
    }
    if((key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][0]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][0]&modifier) ||
      (key_code && key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][1]&0x0FFF)) || (Config_Key[SPECIAL_MOUSE_RIGHT][1]&modifier))
    {
      Directional_right=0;
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

      case SDLK_RMETA:
      case SDLK_LMETA:
        modifier=MOD_META;
        break;

      default:
        modifier=0;
    }
    return Release_control(released_key, modifier);
}


// Joystick management

int Handle_joystick_press(SDL_JoyButtonEvent event)
{
    if (event.button == Joybutton_shift)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_SHIFT);
      return 0;
    }
    if (event.button == Joybutton_control)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_CTRL);
      return 0;
    }
    if (event.button == Joybutton_alt)
    {
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_META));
      return 0;
    }
    if (event.button == Joybutton_left_click)
    {
      Input_new_mouse_K=1;
      return Move_cursor_with_constraints();
    }
    if (event.button == Joybutton_right_click)
    {
      Input_new_mouse_K=2;
      return Move_cursor_with_constraints();
    }
    #ifdef __GP2X__
    switch(event.button)
    {
      case GP2X_BUTTON_UP:
        Directional_up=1;
        break;
      case GP2X_BUTTON_UPRIGHT:
        Directional_up_right=1;
        break;
      case GP2X_BUTTON_RIGHT:
        Directional_right=1;
        break;
      case GP2X_BUTTON_DOWNRIGHT:
        Directional_down_right=1;
        break;
      case GP2X_BUTTON_DOWN:
        Directional_down=1;
        break;
      case GP2X_BUTTON_DOWNLEFT:
        Directional_down_left=1;
        break;
      case GP2X_BUTTON_LEFT:
        Directional_left=1;
        break;
      case GP2X_BUTTON_UPLEFT:
        Directional_up_left=1;
        break;
      default:
        break;
    }
    #endif
    Key = (KEY_JOYBUTTON+event.button)|Key_modifiers(SDL_GetModState());
    // TODO: systeme de répétition
    
    return Move_cursor_with_constraints();
}

int Handle_joystick_release(SDL_JoyButtonEvent event)
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
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_META));
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
  
    #ifdef __GP2X__
    switch(event.button)
    {
      case GP2X_BUTTON_UP:
        Directional_up=0;
        break;
      case GP2X_BUTTON_UPRIGHT:
        Directional_up_right=0;
        break;
      case GP2X_BUTTON_RIGHT:
        Directional_right=0;
        break;
      case GP2X_BUTTON_DOWNRIGHT:
        Directional_down_right=0;
        break;
      case GP2X_BUTTON_DOWN:
        Directional_down=0;
        break;
      case GP2X_BUTTON_DOWNLEFT:
        Directional_down_left=0;
        break;
      case GP2X_BUTTON_LEFT:
        Directional_left=0;
        break;
      case GP2X_BUTTON_UPLEFT:
        Directional_up_left=0;
        break;
    }
    #endif
  return Move_cursor_with_constraints();
}

void Handle_joystick_movement(SDL_JoyAxisEvent event)
{
    if (event.axis==0) // X
    {
      Directional_right=Directional_left=0;
      if (event.value<-1000)
      {
        Directional_left=1;
      }
      else if (event.value>1000)
        Directional_right=1;
    }
    else if (event.axis==1) // Y
    {
      Directional_up=Directional_down=0;
      if (event.value<-1000)
      {
        Directional_up=1;
      }
      else if (event.value>1000)
        Directional_down=1;
    }
}

// Attempts to move the mouse cursor by the given deltas (may be more than 1 pixel at a time)
int Cursor_displace(short delta_x, short delta_y)
{
  short x=Input_new_mouse_X;
  short y=Input_new_mouse_Y;
  
  if(Main_magnifier_mode && Input_new_mouse_Y < Menu_Y && Input_new_mouse_X > Main_separator_position)
  {
    // Cursor in zoomed area
    
    if (delta_x<0)
      Input_new_mouse_X = Max(Main_separator_position, x-Main_magnifier_factor);
    else if (delta_x>0)
      Input_new_mouse_X = Min(Screen_width-1, x+Main_magnifier_factor);
    if (delta_y<0)
      Input_new_mouse_Y = Max(0, y-Main_magnifier_factor);
    else if (delta_y>0)
      Input_new_mouse_Y = Min(Screen_height-1, y+Main_magnifier_factor);
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


// Main input handling function

int Get_input(int sleep_time)
{
    SDL_Event event;
    int user_feedback_required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer
                
    // Commit any pending screen update.
    // This is done in this function because it's called after reading 
    // some user input.
    Flush_update();
    Color_cycling(NULL);

    Key_ANSI = 0;
    Key = 0;
    Mouse_moved=0;
    Input_new_mouse_X = Mouse_X;
    Input_new_mouse_Y = Mouse_Y;
    if (!SDL_PollEvent(&event))
    {
      SDL_Delay(sleep_time);
      return 0;
    }
    // Process as much events as possible without redrawing the screen.
    // This mostly allows us to merge mouse events for people with an high
    // resolution mouse
    while(1) 
    {
        switch(event.type)
        {
            case SDL_VIDEORESIZE:
                Handle_window_resize(event.resize);
                user_feedback_required = 1;
                break;

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

            case SDL_KEYDOWN:
                Handle_key_press(event.key);
                user_feedback_required = 1;
                break;

            case SDL_KEYUP:
                Handle_key_release(event.key);
                break;

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
                        if (DragQueryFile(hdrop,0 ,(LPTSTR) Drop_file_name ,(UINT) MAX_PATH))
                        {
                          // Success
                        }
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
#endif
                break;
            
            default:
                //DEBUG("Unhandled SDL event number : ",event.type);
                break;
        }
        if (user_feedback_required)
          break;
        // Fetch another event from the queue,
        // stopping when it's empty.
        if (!SDL_PollEvent(&event))
          break;
    }
    // Directional controller
    if (!(Directional_up||Directional_up_right||Directional_right||
    Directional_down_right||Directional_down||Directional_down_left||
      Directional_left||Directional_up_left))
    {
      Directional_delay=-1;
      Directional_last_move=SDL_GetTicks();
    }
    else
    {
      long time_now;
      
      time_now=SDL_GetTicks();
      
      if (time_now>Directional_last_move+Directional_delay)
      {
        // Speed parameters, acceleration etc. are here
        if (Directional_delay==-1)
        {
          Directional_delay=150;
          Directional_step=16;
        }
        else if (Directional_delay==150)
          Directional_delay=40;
        else if (Directional_delay!=0)
          Directional_delay=Directional_delay*8/10;
        else if (Directional_step<16*4)
          Directional_step++;
        Directional_last_move = time_now;
      
        // Directional controller UP
        if ((Directional_up||Directional_up_left||Directional_up_right) &&
           !(Directional_down_right||Directional_down||Directional_down_left))
        {
          Cursor_displace(0, -Directional_step/16);
        }
        // Directional controller RIGHT
        if ((Directional_up_right||Directional_right||Directional_down_right) &&
           !(Directional_down_left||Directional_left||Directional_up_left))
        {
          Cursor_displace(Directional_step/16,0);
        }    
        // Directional controller DOWN
        if ((Directional_down_right||Directional_down||Directional_down_left) &&
           !(Directional_up_left||Directional_up||Directional_up_right))
        {
          Cursor_displace(0, Directional_step/16);
        }
        // Directional controller LEFT
        if ((Directional_down_left||Directional_left||Directional_up_left) &&
           !(Directional_up_right||Directional_right||Directional_down_right))
        {
          Cursor_displace(-Directional_step/16,0);
        }
      }
    }
    // If the cursor was moved since last update,
    // it was erased, so we need to redraw it (with the preview brush)
    if (Mouse_moved)
    {
      Compute_paintbrush_coordinates();
      Display_cursor();
    }
    
    return (Mouse_moved!=0) || user_feedback_required;
}

void Adjust_mouse_sensitivity(word fullscreen)
{
  if (fullscreen == 0)
  {
    Mouse_mode = 0;
    return;
  }
  Mouse_mode = 1;
  Mouse_virtual_x_position = 12*Mouse_X*Pixel_width;
  Mouse_virtual_y_position = 12*Mouse_Y*Pixel_height;
  Mouse_virtual_width = 12*(Screen_width-1)*Pixel_width;
  Mouse_virtual_height = 12*(Screen_height-1)*Pixel_height;
}

void Set_mouse_position(void)
{
    if (Mouse_mode == 0)
    {
      SDL_WarpMouse(
          Mouse_X*Pixel_width,
          Mouse_Y*Pixel_height
      );
    }
    else
    {
      Mouse_virtual_x_position = 12*Mouse_X*Pixel_width;
      Mouse_virtual_y_position = 12*Mouse_Y*Pixel_height;
    }
}

int Color_cycling(__attribute__((unused)) void* useless)
{
  static byte offset[16];
  int i, color;
  static SDL_Color PaletteSDL[256];
  int changed; // boolean : true if the palette needs a change in this tick.
  
  long now;
  static long start=0;
  
  if (start==0)
  {
    // First run
    start = SDL_GetTicks();
    return 1;
  }
  if (!Allow_colorcycling || !Cycling_mode)
    return 1;
    

  now = SDL_GetTicks();
  changed=0;
  
  // Check all cycles for a change at this tick
  for (i=0; i<16; i++)
  {
    int len;
    
    len=Gradient_array[i].End-Gradient_array[i].Start+1;
    if (len>1 && Gradient_array[i].Speed)
    {
      int new_offset;
      
      new_offset=(now-start)/(int)(1000.0/(Gradient_array[i].Speed*0.2856)) % len;
      if (!Gradient_array[i].Inverse)
        new_offset=len - new_offset;
      
      if (new_offset!=offset[i])
        changed=1;
      offset[i]=new_offset;
    }
  }
  if (changed)
  {
    // Initialize the palette
    for(color=0;color<256;color++)
    {
      PaletteSDL[color].r=Main_palette[color].R;
      PaletteSDL[color].g=Main_palette[color].G;
      PaletteSDL[color].b=Main_palette[color].B;
    }
    for (i=0; i<16; i++)
    {
      int len;
    
      len=Gradient_array[i].End-Gradient_array[i].Start+1;
      if (len>1 && Gradient_array[i].Speed)
      {
        for(color=Gradient_array[i].Start;color<=Gradient_array[i].End;color++)
        {
          PaletteSDL[color].r=Main_palette[Gradient_array[i].Start+((color-Gradient_array[i].Start+offset[i])%len)].R;
          PaletteSDL[color].g=Main_palette[Gradient_array[i].Start+((color-Gradient_array[i].Start+offset[i])%len)].G;
          PaletteSDL[color].b=Main_palette[Gradient_array[i].Start+((color-Gradient_array[i].Start+offset[i])%len)].B;
        }
      }
    }
    SDL_SetPalette(Screen_SDL, SDL_PHYSPAL | SDL_LOGPAL, PaletteSDL,0,256);
  }
  return 0;
}
