/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

#include <SDL.h>

#include "global.h"
#include "clavier.h"
#include "sdlscreen.h"
#include "windows.h"
#include "erreurs.h"
#include "divers.h"
#include "input.h"

void Handle_window_resize(SDL_ResizeEvent event);
void Handle_window_exit(SDL_QuitEvent event);

byte Directional_up;
byte Directional_up_right;
byte Directional_right;
byte Directional_down_right;
byte Directional_down;
byte Directional_down_left;
byte Directional_left;
byte Directional_up_left;
long Directional_delay;
long Directional_last_move;
long Directional_step;
short Mouse_count; // Number of mouse movements received in the current Get_input()

word Input_new_mouse_X;
word Input_new_mouse_Y;
byte Input_new_mouse_K;

// TODO: move to config
short Joybutton_shift=-1; // Button number that serves as a "shift" modifier
short Joybutton_control=-1; // Button number that serves as a "ctrl" modifier
short Joybutton_alt=-1; // Button number that serves as a "alt" modifier
short Joybutton_left_click=0; // Button number that serves as left click
short Joybutton_right_click=0; // Button number that serves as right-click

int Is_shortcut(word Key, word function)
{
  if (Key == 0)
    return 0;
    
  if (function & 0x100)
  {
    if (Button[function&0xFF].Left_shortcut[0]==Key)
      return 1;
    if (Button[function&0xFF].Left_shortcut[1]==Key)
      return 1;
    return 0;
  }
  if (function & 0x200)
  {
    if (Button[function&0xFF].Right_shortcut[0]==Key)
      return 1;
    if (Button[function&0xFF].Right_shortcut[1]==Key)
      return 1;
    return 0;
  }
  if(Key == Config_Key[function][0])
    return 1;
  if(Key == Config_Key[function][1])
    return 1;
  return 0; 
}

// Called each time there is a cursor move, either triggered by mouse or keyboard shortcuts
int Move_cursor_with_constraints()
{
  int feedback=0;
  
  //Gestion "avancée" du curseur: interdire la descente du curseur dans le
  //menu lorsqu'on est en train de travailler dans l'image
  if (Operation_stack_size != 0)
  {
        byte bl=0;//BL va indiquer si on doit corriger la position du curseur

        //Si le curseur ne se trouve plus dans l'image
        if(Menu_Y<=Input_new_mouse_Y)
        {
            //On bloque le curseur en fin d'image
            bl++;
            Input_new_mouse_Y=Menu_Y-1; //La ligne !!au-dessus!! du menu
        }

        if(Main_magnifier_mode)
        {
            if(Operation_in_magnifier==0)
            {
                if(Input_new_mouse_X>=Main_separator_position)
                {
                    bl++;
                    Input_new_mouse_X=Main_separator_position-1;
                }
            }
            else
            {
                if(Input_new_mouse_X<Main_X_zoom)
                {
                    bl++;
                    Input_new_mouse_X=Main_X_zoom;
                }
            }
        }

        if (bl)
        {
            SDL_WarpMouse(
                    Input_new_mouse_X*Pixel_width,
                    Input_new_mouse_Y*Pixel_height
                    );
        }
  }
  if ((Input_new_mouse_X != Mouse_X) ||
    (Input_new_mouse_Y != Mouse_Y) ||
    (Input_new_mouse_K != Mouse_K))
  {
    if ((Input_new_mouse_K != Mouse_K))
      feedback=1;        
    Hide_cursor(); // On efface le curseur AVANT de le déplacer...
    Mouse_X=Input_new_mouse_X;
    Mouse_Y=Input_new_mouse_Y;
    Mouse_K=Input_new_mouse_K;
    Compute_paintbrush_coordinates();
    Display_cursor();
    
    Mouse_count++;
    if (Mouse_count>Config.Mouse_merge_movement)
      feedback=1;
  }

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
    Input_new_mouse_X = event.x/Pixel_width;
    Input_new_mouse_Y = event.y/Pixel_height;

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
            // TODO: systeme de répétition
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
    else if(Is_shortcut(Key,SPECIAL_CLICK_LEFT))
    {
        Input_new_mouse_K=1;
        return Move_cursor_with_constraints();
    }
    else if(Is_shortcut(Key,SPECIAL_CLICK_RIGHT))
    {
        Input_new_mouse_K=2;
        return Move_cursor_with_constraints();
    }

    if (Operation_stack_size!=0 && Key != 0)
    {
        //Enfin, on inhibe les touches (sauf si c'est un changement de couleur
        //ou de taille de pinceau lors d'une des operations suivantes:
        //OPERATION_CONTINUOUS_DRAW, OPERATION_DISCONTINUOUS_DRAW, OPERATION_AIRBRUSH)
        if(Allow_color_change_during_operation)
        {
            //A ce stade là, on sait qu'on est dans une des 3 opérations
            //supportant le changement de couleur ou de taille de pinceau.

            if(
                    (!Is_shortcut(Key,SPECIAL_NEXT_FORECOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_PREVIOUS_FORECOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_NEXT_BACKCOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_PREVIOUS_BACKCOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_SMALLER_PAINTBRUSH)) &&
                    (!Is_shortcut(Key,SPECIAL_BIGGER_PAINTBRUSH)) &&
                    (!Is_shortcut(Key,SPECIAL_NEXT_USER_FORECOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_PREVIOUS_USER_FORECOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_NEXT_USER_BACKCOLOR)) &&
                    (!Is_shortcut(Key,SPECIAL_PREVIOUS_USER_BACKCOLOR))
              )
            {
                Key=0;
            }
        }
        else Key = 0;
    }

    return 0;
}

int Release_control(int key_code, int modifier)
{

    if(key_code == (Config_Key[SPECIAL_MOUSE_UP][0]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_UP][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_MOUSE_UP][1]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_UP][1]&modifier))
    {
      Directional_up=0;
    }
    if(key_code == (Config_Key[SPECIAL_MOUSE_DOWN][0]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_DOWN][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_MOUSE_DOWN][1]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_DOWN][1]&modifier))
    {
      Directional_down=0;
    }
    if(key_code == (Config_Key[SPECIAL_MOUSE_LEFT][0]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_LEFT][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_MOUSE_LEFT][1]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_LEFT][1]&modifier))
    {
      Directional_left=0;
    }
    if(key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][0]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_RIGHT][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_MOUSE_RIGHT][1]&0x0FFF) || (Config_Key[SPECIAL_MOUSE_RIGHT][1]&modifier))
    {
      Directional_right=0;
    }
    if(key_code == (Config_Key[SPECIAL_CLICK_LEFT][0]&0x0FFF) || (Config_Key[SPECIAL_CLICK_LEFT][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_CLICK_LEFT][1]&0x0FFF) || (Config_Key[SPECIAL_CLICK_LEFT][1]&modifier))
    {
        Input_new_mouse_K &= ~1;
        return Move_cursor_with_constraints();
    }
    if(key_code == (Config_Key[SPECIAL_CLICK_RIGHT][0]&0x0FFF) || (Config_Key[SPECIAL_CLICK_RIGHT][0]&modifier) ||
      key_code == (Config_Key[SPECIAL_CLICK_RIGHT][1]&0x0FFF) || (Config_Key[SPECIAL_CLICK_RIGHT][1]&modifier))
    {
        Input_new_mouse_K &= ~2;
        return Move_cursor_with_constraints();
    }
  
    // Other keys don't need to be released : they are handled as "events" and procesed only once.
    // These clicks are apart because they need to be continuous (ie move while key pressed)
    // We are relying on "hardware" keyrepeat to achieve that.
    return 0;
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
      default:
        modifier=0;
    }
    return Release_control(released_key, modifier);
}


// Joystick management

int Handle_joystick_press(SDL_JoyButtonEvent event)
{
  if (event.which==0) // Joystick number 0
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
    #ifdef __gp2x__
    switch(event.button)
    {
      #ifndef NO_JOYCURSOR
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
      #endif
      default:
    }
    #endif
    Key = (KEY_JOYBUTTON+event.button)|Key_modifiers(SDL_GetModState());
    // TODO: systeme de répétition
    
    return Move_cursor_with_constraints();
  }
  return 0;
}

int Handle_joystick_release(SDL_JoyButtonEvent event)
{
  if (event.which==0) // Joystick number 0
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
  
    #ifdef __gp2x__
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
      case GP2X_BUTTON_A: // A
        Input_new_mouse_K &= ~1;
        break;
      case GP2X_BUTTON_B: // B
        Input_new_mouse_K &= ~2;
        break;
    }
    #else
    switch(event.button)
    {
      case 0: // A
        Input_new_mouse_K &= ~1;
        break;
      case 1: // B
        Input_new_mouse_K &= ~2;
        break;
    }
    #endif
  }
  return Move_cursor_with_constraints();
}

void Handle_joystick_movement(SDL_JoyAxisEvent event)
{
  if (event.which==0) // Joystick number 0
  {
    #ifndef NO_JOYCURSOR
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
    #endif
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

int Get_input(void)
{
    SDL_Event event;
    int user_feedback_required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer

    Key_ANSI = 0;
    Key = 0;
    Mouse_count=0;

    // Process as much events as possible without redrawing the screen.
    // This mostly allows us to merge mouse events for people with an high
    // resolution mouse
    while( (!user_feedback_required) && SDL_PollEvent(&event)) // Try to cumulate for a full VBL except if there is a required feedback
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
                
            default:
            //    DEBUG("Unhandled SDL event number : ",event.type);
                break;
        }
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
    // Vidage de toute mise à jour de l'affichage à l'écran qui serait encore en attente.
    // (c'est fait ici car on est sur que cette function est apellée partout ou on a besoin d'interragir avec l'utilisateur)
    Flush_update();

    return user_feedback_required;
}
