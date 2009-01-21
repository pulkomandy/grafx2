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

// Fonction qui filtre les evenements génériques.
void Gere_Evenement_SDL(SDL_Event * event)
{
  // Redimensionnement fenetre
  if (event->type == SDL_VIDEORESIZE )
  {
    Handle_Window_Resize(&event);
  }
  // Fermeture
  if (event->type == SDL_QUIT )
  {
    Handle_Window_Exit(&event);
  }
}


// WM events management

void Handle_Window_Resize(&event)
{
  Resize_Largeur = event->resize.w;
  Resize_Hauteur = event->resize.h;
}

void Handle_Window_Exit(&event)
{
  Quit_demande = 1;
}

// Mouse events management

void Handle_Mouse_Move(&event)
{
  INPUT_Nouveau_Mouse_X = event.motion.x/Pixel_width;
  INPUT_Nouveau_Mouse_Y = event.motion.y/Pixel_height;

  // Il peut arriver (à cause de la division ci dessus) que les nouvelles
  // coordonnees soient égales aux anciennes...
  // Dans ce cas on ne traite pas l'évènement.
  if (INPUT_Nouveau_Mouse_X == Mouse_X && INPUT_Nouveau_Mouse_Y == Mouse_Y)
    return 0;
}

void Handle_Mouse_Click(&event)
{
  switch(event.button.button)
  {
    case SDL_BUTTON_LEFT:
      INPUT_Nouveau_Mouse_K |= 1;
    break;

    case SDL_BUTTON_MIDDLE:
      INPUT_Nouveau_Mouse_K |= 4;
    break;

    case SDL_BUTTON_RIGHT:
      INPUT_Nouveau_Mouse_K |= 2;
    break;
  }
}

void Handle_Mouse_Release(&event)
{
  switch(event.button.button)
  {
    case SDL_BUTTON_LEFT:
      INPUT_Nouveau_Mouse_K &= ~1;
    break;

    case SDL_BUTTON_MIDDLE:
      INPUT_Nouveau_Mouse_K &= ~4;
    break;

    case SDL_BUTTON_RIGHT:
      INPUT_Nouveau_Mouse_K &= ~2;
    break;
  }
}

// Keyboard management

void Handle_Key_Press(&event)
{
  //Appui sur une touche du clavier
  Touche = Conversion_Touche(event.key.keysym);
  Touche_ANSI = Conversion_ANSI(event.key.keysym);

  // Instead of all this mess, send back a mouse event !
  if(Touche == Config_Touche[0])
  {
    //[Touche] = Emulation de MOUSE UP
    //si on est déjà en haut on peut plus bouger
          if(INPUT_Nouveau_Mouse_Y!=0)
          {
            if(Loupe_Mode && INPUT_Nouveau_Mouse_Y < Menu_Ordonnee && INPUT_Nouveau_Mouse_X > Principal_Split)
                INPUT_Nouveau_Mouse_Y=INPUT_Nouveau_Mouse_Y<Loupe_Facteur?0:INPUT_Nouveau_Mouse_Y-Loupe_Facteur;
            else
                INPUT_Nouveau_Mouse_Y--;
            ok=1;
          }
        }
        else if(Touche == Config_Touche[1])
        {
          //[Touche] = Emulation de MOUSE DOWN
          if(INPUT_Nouveau_Mouse_Y<Hauteur_ecran-1)
          {
            if(Loupe_Mode && INPUT_Nouveau_Mouse_Y < Menu_Ordonnee && INPUT_Nouveau_Mouse_X > Principal_Split)
            {
                INPUT_Nouveau_Mouse_Y+=Loupe_Facteur;
                if (INPUT_Nouveau_Mouse_Y>=Hauteur_ecran)
                    INPUT_Nouveau_Mouse_Y=Hauteur_ecran-1;
            }
            else
                INPUT_Nouveau_Mouse_Y++;
            ok=1;
          }
        }
        else if(Touche == Config_Touche[2])
        {
          //[Touche] = Emulation de MOUSE LEFT
          if(INPUT_Nouveau_Mouse_X!=0)
          {
            if(Loupe_Mode && INPUT_Nouveau_Mouse_Y < Menu_Ordonnee && INPUT_Nouveau_Mouse_X > Principal_Split)
                INPUT_Nouveau_Mouse_X-=Loupe_Facteur;
            else
                INPUT_Nouveau_Mouse_X--;
            ok=1;
          }
        }
        else if(Touche == Config_Touche[3])
        {
          //[Touche] = Emulation de MOUSE RIGHT

          if(INPUT_Nouveau_Mouse_X<Largeur_ecran-1)
          {
            if(Loupe_Mode && INPUT_Nouveau_Mouse_Y < Menu_Ordonnee && INPUT_Nouveau_Mouse_X > Principal_Split)
            {
                INPUT_Nouveau_Mouse_X+=Loupe_Facteur;
                if (INPUT_Nouveau_Mouse_X>=Largeur_ecran)
                    INPUT_Nouveau_Mouse_X=Largeur_ecran-1;
            }
            else
                INPUT_Nouveau_Mouse_X++;
            ok=1;
          }
        }
        else if(Touche == Config_Touche[4])
        {
            //[Touche] = Emulation de MOUSE CLICK LEFT
            INPUT_Nouveau_Mouse_K=1;
            ok=1;
        }
        else if(Touche == Config_Touche[5])
        {
          //[Touche] = Emulation de MOUSE CLICK RIGHT
          INPUT_Nouveau_Mouse_K=2;
          ok=1;
        }

        if(ok)
        {
          SDL_WarpMouse(
            INPUT_Nouveau_Mouse_X*Pixel_width,
            INPUT_Nouveau_Mouse_Y*Pixel_height
          );
        }
      }
}

void Handle_Key_Release(&event)
{
  int ToucheR = Conversion_Touche(event.key.keysym);

  // Send back a mouse event instead. Or extract the code and put it in common.
        if(ToucheR == Config_Touche[4])
        {
          INPUT_Nouveau_Mouse_K=0;
        }
        else if(ToucheR == Config_Touche[5])
        {
          //[Touche] = Emulation de MOUSE CLICK RIGHT
          INPUT_Nouveau_Mouse_K=0;
        }
}


// Joystick management

void Handle_Joystick_Press(&event)
{

}

void Handle_Joystick_Release(&event)
{

}

int Get_input(void)
{
  SDL_Event event;

  // Process as much events as possible without redrawing the screen.
  // This mostly allows us to merge mouse events for people with an high
  // resolution mouse
  while( !User_Feedback_Required && SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_VIDEORESIZE:
	Handle_Window_Resize(&event);
      break;

      case SDL_QUIT:
        Handle_Window_Exit(&event);
      break;

      case SDL_MOUSEMOTION:
        Handle_Mouse_Move(&event);
      break;

      case SDL_MOUSEBUTTONDOWN:
        Handle_Mouse_Click(&event);
      break;

      case SDL_MOUSEBUTTONUP:
        Handle_Mouse_Release(&event);
      break;

      case SDL_KEYDOWN:
        Handle_Key_Press(&event);
      break;

      case SDL_KEYUP:
        Handle_Key_Release(&event);
      break;

      case SDL_JOYBUTTONUP:
        Handle_Joystick_Press(&event);
      break;

      case SDL_JOYBUTTONDOWN:
        Handle_Joystick_Release(&event);
      break;
    }
  }

  // Everything below should disapear

  //Gestion "avancée" du curseur: interdire la descente du curseur dans le
  //menu lorsqu'on est en train de travailler dans l'image

  if(Operation_Taille_pile!=0)
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


    if (Touche != 0)
    {
      //Enfin, on inhibe les touches (sauf si c'est un changement de couleur
      //ou de taille de pinceau lors d'une des operations suivantes:
      //OPERATION_DESSIN_CONTINU, OPERATION_DESSIN_DISCONTINU, OPERATION_SPRAY)
      if(Autoriser_changement_de_couleur_pendant_operation)
      {
        //A ce stade là, on sait qu'on est dans une des 3 opérations
        //supportant le changement de couleur ou de taille de pinceau.

        if(
          (Touche != Config_Touche[6]) &&
          (Touche != Config_Touche[7]) &&
          (Touche != Config_Touche[8]) &&
          (Touche != Config_Touche[9]) &&
          (Touche != Config_Touche[10]) &&
          (Touche != Config_Touche[11]) &&
          (Touche != Config_Touche[12]) &&
          (Touche != Config_Touche[13]) &&
          (Touche != Config_Touche[14]) &&
          (Touche != Config_Touche[15])
        )
        {
          Touche=0;
        }
      }
      else Touche = 0;
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
  }

  // Vidage de toute mise à jour de l'affichage à l'écran qui serait encore en attente.
  // (c'est fait ici car on est sur que cette fonction est apellée partout ou on a besoin d'interragir avec l'utilisateur)
  Flush_update();

}
