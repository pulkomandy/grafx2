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
#include "graph.h"
#include "sdlscreen.h"
#include "windows.h"
#include "erreurs.h"
#include "divers.h"
#include "input.h"

void Handle_Window_Resize(SDL_ResizeEvent event);
void Handle_Window_Exit(SDL_QuitEvent event);

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

// TODO: move to config
short Button_shift=-1; // Button number that serves as a "shift" modifier
short Button_control=-1; // Button number that serves as a "ctrl" modifier
short Button_alt=-1; // Button number that serves as a "alt" modifier
short Button_clic_gauche=0; // Button number that serves as left click
short Button_clic_droit=0; // Button number that serves as right-click

int Est_Raccourci(word Touche, word Fonction)
{
  if (Touche == 0)
    return 0;
    
  if (Fonction & 0x100)
  {
    if (Bouton[Fonction&0xFF].Raccourci_gauche[0]==Touche)
      return 1;
    if (Bouton[Fonction&0xFF].Raccourci_gauche[1]==Touche)
      return 1;
    return 0;
  }
  if (Fonction & 0x200)
  {
    if (Bouton[Fonction&0xFF].Raccourci_droite[0]==Touche)
      return 1;
    if (Bouton[Fonction&0xFF].Raccourci_droite[1]==Touche)
      return 1;
    return 0;
  }
  if(Touche == Config_Touche[Fonction][0])
    return 1;
  if(Touche == Config_Touche[Fonction][1])
    return 1;
  return 0; 
}

// Called each time there is a cursor move, either triggered by mouse or keyboard shortcuts
int Move_cursor_with_constraints()
{
  //Gestion "avancée" du curseur: interdire la descente du curseur dans le
  //menu lorsqu'on est en train de travailler dans l'image
  if (Operation_Taille_pile != 0)
  {
        byte bl=0;//BL va indiquer si on doit corriger la position du curseur

        //Si le curseur ne se trouve plus dans l'image
        if(Menu_Ordonnee<=INPUT_Nouveau_Mouse_Y)
        {
            //On bloque le curseur en fin d'image
            bl++;
            INPUT_Nouveau_Mouse_Y=Menu_Ordonnee-1; //La ligne !!au-dessus!! du menu
        }

        if(Loupe_Mode)
        {
            if(Operation_dans_loupe==0)
            {
                if(INPUT_Nouveau_Mouse_X>=Principal_Split)
                {
                    bl++;
                    INPUT_Nouveau_Mouse_X=Principal_Split-1;
                }
            }
            else
            {
                if(INPUT_Nouveau_Mouse_X<Principal_X_Zoom)
                {
                    bl++;
                    INPUT_Nouveau_Mouse_X=Principal_X_Zoom;
                }
            }
        }

        if (bl)
        {
            SDL_WarpMouse(
                    INPUT_Nouveau_Mouse_X*Pixel_width,
                    INPUT_Nouveau_Mouse_Y*Pixel_height
                    );
        }
  }

  if (
          (INPUT_Nouveau_Mouse_X != Mouse_X) ||
          (INPUT_Nouveau_Mouse_Y != Mouse_Y) ||
          (INPUT_Nouveau_Mouse_K != Mouse_K)
     )
  {
      Forcer_affichage_curseur=0;
      Effacer_curseur(); // On efface le curseur AVANT de le déplacer...
      Mouse_X=INPUT_Nouveau_Mouse_X;
      Mouse_Y=INPUT_Nouveau_Mouse_Y;
      Mouse_K=INPUT_Nouveau_Mouse_K;
      Calculer_coordonnees_pinceau();
      Afficher_curseur();
      return 1;
  }
  else
    return 0;
}

// Fonction qui filtre les evenements génériques.
void Gere_Evenement_SDL(SDL_Event * event)
{
    // Redimensionnement fenetre
    if (event->type == SDL_VIDEORESIZE )
    {
        Handle_Window_Resize(event->resize);
    }
    // Fermeture
    if (event->type == SDL_QUIT )
    {
        Handle_Window_Exit(event->quit);
    }
}


// WM events management

void Handle_Window_Resize(SDL_ResizeEvent event)
{
    Resize_Largeur = event.w;
    Resize_Hauteur = event.h;
}

void Handle_Window_Exit(__attribute__((unused)) SDL_QuitEvent event)
{
    Quit_demande = 1;
}

// Mouse events management

int Handle_Mouse_Move(SDL_MouseMotionEvent event)
{
    INPUT_Nouveau_Mouse_X = event.x/Pixel_width;
    INPUT_Nouveau_Mouse_Y = event.y/Pixel_height;

    return Move_cursor_with_constraints();
}

int Handle_Mouse_Click(SDL_MouseButtonEvent event)
{
    switch(event.button)
    {
        case SDL_BUTTON_LEFT:
            INPUT_Nouveau_Mouse_K |= 1;
            break;

        case SDL_BUTTON_RIGHT:
            INPUT_Nouveau_Mouse_K |= 2;
            break;

        case SDL_BUTTON_MIDDLE:
            Touche = TOUCHE_MOUSEMIDDLE;
            // TODO: systeme de répétition
            return 0;

        case SDL_BUTTON_WHEELUP:
            Touche = TOUCHE_MOUSEWHEELUP;
            return 0;

        case SDL_BUTTON_WHEELDOWN:
            Touche = TOUCHE_MOUSEWHEELDOWN;
            return 0;
        default:
        return 0;
    }
    return Move_cursor_with_constraints();
}

int Handle_Mouse_Release(SDL_MouseButtonEvent event)
{
    switch(event.button)
    {
        case SDL_BUTTON_LEFT:
            INPUT_Nouveau_Mouse_K &= ~1;
            break;

        case SDL_BUTTON_RIGHT:
            INPUT_Nouveau_Mouse_K &= ~2;
            break;
    }
    return Move_cursor_with_constraints();
}

// Keyboard management

int Handle_Key_Press(SDL_KeyboardEvent event)
{
    //Appui sur une touche du clavier
    Touche = Conversion_Touche(event.keysym);
    Touche_ANSI = Conversion_ANSI(event.keysym);

    if(Est_Raccourci(Touche,SPECIAL_MOUSE_UP))
    {
      Directional_up=1;
      return 0;
    }
    else if(Est_Raccourci(Touche,SPECIAL_MOUSE_DOWN))
    {
      Directional_down=1;
      return 0;
    }
    else if(Est_Raccourci(Touche,SPECIAL_MOUSE_LEFT))
    {
      Directional_left=1;
      return 0;
    }
    else if(Est_Raccourci(Touche,SPECIAL_MOUSE_RIGHT))
    {
      Directional_right=1;
      return 0;
    }
    else if(Est_Raccourci(Touche,SPECIAL_CLICK_LEFT))
    {
        INPUT_Nouveau_Mouse_K=1;
        return Move_cursor_with_constraints();
    }
    else if(Est_Raccourci(Touche,SPECIAL_CLICK_RIGHT))
    {
        INPUT_Nouveau_Mouse_K=2;
        return Move_cursor_with_constraints();
    }

    if (Operation_Taille_pile!=0 && Touche != 0)
    {
        //Enfin, on inhibe les touches (sauf si c'est un changement de couleur
        //ou de taille de pinceau lors d'une des operations suivantes:
        //OPERATION_DESSIN_CONTINU, OPERATION_DESSIN_DISCONTINU, OPERATION_SPRAY)
        if(Autoriser_changement_de_couleur_pendant_operation)
        {
            //A ce stade là, on sait qu'on est dans une des 3 opérations
            //supportant le changement de couleur ou de taille de pinceau.

            if(
                    (!Est_Raccourci(Touche,SPECIAL_NEXT_FORECOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_PREVIOUS_FORECOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_NEXT_BACKCOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_PREVIOUS_BACKCOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_RETRECIR_PINCEAU)) &&
                    (!Est_Raccourci(Touche,SPECIAL_GROSSIR_PINCEAU)) &&
                    (!Est_Raccourci(Touche,SPECIAL_NEXT_USER_FORECOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_PREVIOUS_USER_FORECOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_NEXT_USER_BACKCOLOR)) &&
                    (!Est_Raccourci(Touche,SPECIAL_PREVIOUS_USER_BACKCOLOR))
              )
            {
                Touche=0;
            }
        }
        else Touche = 0;
    }

    return 0;
}

int Relache_controle(int CodeTouche, int Modifieur)
{

    if(CodeTouche == (Config_Touche[SPECIAL_MOUSE_UP][0]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_UP][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_MOUSE_UP][1]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_UP][1]&Modifieur))
    {
      Directional_up=0;
    }
    if(CodeTouche == (Config_Touche[SPECIAL_MOUSE_DOWN][0]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_DOWN][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_MOUSE_DOWN][1]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_DOWN][1]&Modifieur))
    {
      Directional_down=0;
    }
    if(CodeTouche == (Config_Touche[SPECIAL_MOUSE_LEFT][0]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_LEFT][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_MOUSE_LEFT][1]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_LEFT][1]&Modifieur))
    {
      Directional_left=0;
    }
    if(CodeTouche == (Config_Touche[SPECIAL_MOUSE_RIGHT][0]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_RIGHT][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_MOUSE_RIGHT][1]&0x0FFF) || (Config_Touche[SPECIAL_MOUSE_RIGHT][1]&Modifieur))
    {
      Directional_right=0;
    }
    if(CodeTouche == (Config_Touche[SPECIAL_CLICK_LEFT][0]&0x0FFF) || (Config_Touche[SPECIAL_CLICK_LEFT][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_CLICK_LEFT][1]&0x0FFF) || (Config_Touche[SPECIAL_CLICK_LEFT][1]&Modifieur))
    {
        INPUT_Nouveau_Mouse_K &= ~1;
        return Move_cursor_with_constraints();
    }
    if(CodeTouche == (Config_Touche[SPECIAL_CLICK_RIGHT][0]&0x0FFF) || (Config_Touche[SPECIAL_CLICK_RIGHT][0]&Modifieur) ||
      CodeTouche == (Config_Touche[SPECIAL_CLICK_RIGHT][1]&0x0FFF) || (Config_Touche[SPECIAL_CLICK_RIGHT][1]&Modifieur))
    {
        INPUT_Nouveau_Mouse_K &= ~2;
        return Move_cursor_with_constraints();
    }
  
    // Other keys don't need to be released : they are handled as "events" and procesed only once.
    // These clicks are apart because they need to be continuous (ie move while key pressed)
    // We are relying on "hardware" keyrepeat to achieve that.
    return 0;
}


int Handle_Key_Release(SDL_KeyboardEvent event)
{
    int Modifieur;
    int ToucheR = Conversion_Touche(event.keysym) & 0x0FFF;
  
    switch(event.keysym.sym)
    {
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
        Modifieur=MOD_SHIFT;
        break;
      case SDLK_RCTRL:
      case SDLK_LCTRL:
        Modifieur=MOD_CTRL;
        break;
      case SDLK_RALT:
      case SDLK_LALT:
      case SDLK_MODE:
        Modifieur=MOD_ALT;
        break;
      default:
        Modifieur=0;
    }
    return Relache_controle(ToucheR, Modifieur);
}


// Joystick management

int Handle_Joystick_Press(SDL_JoyButtonEvent event)
{
  if (event.which==0) // joystick number 0
  {
    if (event.button == Button_shift)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_SHIFT);
      return 0;
    }
    if (event.button == Button_control)
    {
      SDL_SetModState(SDL_GetModState() | KMOD_CTRL);
      return 0;
    }
    if (event.button == Button_alt)
    {
      SDL_SetModState(SDL_GetModState() | (KMOD_ALT|KMOD_META));
      return 0;
    }
    if (event.button == Button_clic_gauche)
    {
      INPUT_Nouveau_Mouse_K=1;
      return Move_cursor_with_constraints();
    }
    if (event.button == Button_clic_droit)
    {
      INPUT_Nouveau_Mouse_K=2;
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
    Touche = TOUCHE_BUTTON+event.button;
    // TODO: systeme de répétition
    
    return Move_cursor_with_constraints();
  }
  return 0;
}

int Handle_Joystick_Release(SDL_JoyButtonEvent event)
{
  if (event.which==0) // joystick number 0
  {
    if (event.button == Button_shift)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_SHIFT);
      return Relache_controle(0,MOD_SHIFT);
    }
    if (event.button == Button_control)
    {
      SDL_SetModState(SDL_GetModState() & ~KMOD_CTRL);
      return Relache_controle(0,MOD_CTRL);
    }
    if (event.button == Button_alt)
    {
      SDL_SetModState(SDL_GetModState() & ~(KMOD_ALT|KMOD_META));
      return Relache_controle(0,MOD_ALT);
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
        INPUT_Nouveau_Mouse_K &= ~1;
        break;
      case GP2X_BUTTON_B: // B
        INPUT_Nouveau_Mouse_K &= ~2;
        break;
    }
    #else
    switch(event.button)
    {
      case 0: // A
        INPUT_Nouveau_Mouse_K &= ~1;
        break;
      case 1: // B
        INPUT_Nouveau_Mouse_K &= ~2;
        break;
    }
    #endif
  }
  return Move_cursor_with_constraints();
}

void Handle_Joystick_Movement(SDL_JoyAxisEvent event)
{
  if (event.which==0) // joystick number 0
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
int Cursor_displace(short Delta_X, short Delta_Y)
{
  short X=INPUT_Nouveau_Mouse_X;
  short Y=INPUT_Nouveau_Mouse_Y;
  
  if(Loupe_Mode && INPUT_Nouveau_Mouse_Y < Menu_Ordonnee && INPUT_Nouveau_Mouse_X > Principal_Split)
  {
    // Cursor in zoomed area
    
    if (Delta_X<0)
      INPUT_Nouveau_Mouse_X = Max(Principal_Split, X-Loupe_Facteur);
    else if (Delta_X>0)
      INPUT_Nouveau_Mouse_X = Min(Largeur_ecran-1, X+Loupe_Facteur);
    if (Delta_Y<0)
      INPUT_Nouveau_Mouse_Y = Max(0, Y-Loupe_Facteur);
    else if (Delta_Y>0)
      INPUT_Nouveau_Mouse_Y = Min(Hauteur_ecran-1, Y+Loupe_Facteur);
  }
  else
  {
    if (Delta_X<0)
      INPUT_Nouveau_Mouse_X = Max(0, X+Delta_X);
    else if (Delta_X>0)
      INPUT_Nouveau_Mouse_X = Min(Largeur_ecran-1, X+Delta_X);
    if (Delta_Y<0)
      INPUT_Nouveau_Mouse_Y = Max(0, Y+Delta_Y);
    else if (Delta_Y>0)
      INPUT_Nouveau_Mouse_Y = Min(Hauteur_ecran-1, Y+Delta_Y);
  }
  return Move_cursor_with_constraints();
}


// Main input handling function

int Get_input(void)
{
    SDL_Event event;
    int User_Feedback_Required = 0; // Flag qui indique si on doit arrêter de traiter les évènements ou si on peut enchainer

    Touche_ANSI = 0;
    Touche = 0;

    // Process as much events as possible without redrawing the screen.
    // This mostly allows us to merge mouse events for people with an high
    // resolution mouse
    while( (!User_Feedback_Required) && SDL_PollEvent(&event)) // Try to cumulate for a full VBL except if there is a required feedback
    {
        switch(event.type)
        {
            case SDL_VIDEORESIZE:
                Handle_Window_Resize(event.resize);
                User_Feedback_Required = 1;
                break;

            case SDL_QUIT:
                Handle_Window_Exit(event.quit);
                User_Feedback_Required = 1;
                break;

            case SDL_MOUSEMOTION:
                User_Feedback_Required = Handle_Mouse_Move(event.motion);
                break;

            case SDL_MOUSEBUTTONDOWN:
                Handle_Mouse_Click(event.button);
                User_Feedback_Required = 1;
                break;

            case SDL_MOUSEBUTTONUP:
                Handle_Mouse_Release(event.button);
                User_Feedback_Required = 1;
                break;

            case SDL_KEYDOWN:
                Handle_Key_Press(event.key);
                User_Feedback_Required = 1;
                break;

            case SDL_KEYUP:
                Handle_Key_Release(event.key);
                break;

            case SDL_JOYBUTTONUP:
                Handle_Joystick_Release(event.jbutton);
                User_Feedback_Required = 1;
                break;

            case SDL_JOYBUTTONDOWN:
                Handle_Joystick_Press(event.jbutton);
                User_Feedback_Required = 1;
                break;

            case SDL_JOYAXISMOTION:
                Handle_Joystick_Movement(event.jaxis);
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
      long Now;
      
      Now=SDL_GetTicks();
      
      if (Now>Directional_last_move+Directional_delay)
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
        Directional_last_move = Now;
      
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
    // (c'est fait ici car on est sur que cette fonction est apellée partout ou on a besoin d'interragir avec l'utilisateur)
    Flush_update();

    return User_Feedback_Required;
}
