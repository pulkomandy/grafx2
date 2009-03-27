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
/************************************************************************
*                                                                       *
* READLINE (procédure permettant de saisir une chaîne de caractères) *
*                                                                       *
************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "erreurs.h"
#include "const.h"
#include "sdlscreen.h"
#include "readline.h"
#include "windows.h"
#include "input.h"

#define TEXT_COLOR         MC_Black
#define BACKGROUND_COLOR          MC_Light
#define CURSOR_COLOR MC_Black
#define CURSOR_BACKGROUND_COLOR  MC_Dark

// Suppresion d'un caractère à une certaine POSITION dans une CHAINE.
void Remove_character(char * str, byte position)
{
  for (;str[position]!='\0';position++)
    str[position]=str[position+1];
}


void Insert_character(char * str, char letter, byte position)
//  Insertion d'une LETTRE à une certaine POSITION
//  dans une CHAINE d'une certaine TAILLE.
{
  char temp_char;

  for (;letter!='\0';position++)
  {
    // On mémorise le caractère qui se trouve en "position"
    temp_char=str[position];
    // On splotch la lettre à insérer
    str[position]=letter;
    // On place le caractère mémorisé dans "letter" comme nouvelle lettre à insérer
    letter=temp_char;
  }
  // On termine la chaine
  str[position]='\0';
}

int Valid_character(int c)
{
  // Sous Linux: Seul le / est strictement interdit, mais beaucoup
  // d'autres poseront des problèmes au shell, alors on évite.
  // Sous Windows : c'est moins grave car le fopen() échouerait de toutes façons.
  // AmigaOS4: Pas de ':' car utilisé pour les volumes.
  #if defined(__WIN32__)
  char forbidden_char[] = {'/', '|', '?', '*', '<', '>', ':', '\\'};
  #elif defined (__amigaos4__)
  char forbidden_char[] = {'/', '|', '?', '*', '<', '>', ':'};
  #else
  char forbidden_char[] = {'/', '|', '?', '*', '<', '>'};
  #endif
  int position;
  
  if (c < ' ' || c > 255)
    return 0;
  
  for (position=0; position<(long)sizeof(forbidden_char); position++)
    if (c == forbidden_char[position])
      return 0;
  return 1;
}

void Display_whole_string(word x_pos,word y_pos,char * str,byte position)
{
  Print_in_window(x_pos,y_pos,str,TEXT_COLOR,BACKGROUND_COLOR);
  Print_char_in_window(x_pos+(position<<3),y_pos,str[position],CURSOR_COLOR,CURSOR_BACKGROUND_COLOR);
}

/****************************************************************************
*           Enhanced super scanf deluxe pro plus giga mieux :-)             *
****************************************************************************/
byte Readline(word x_pos,word y_pos,char * str,byte visible_size,byte input_type)
// Paramètres:
//   x_pos, y_pos : Coordonnées de la saisie dans la fenêtre
//   str       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   max_size  : Nombre de caractères logeant dans la zone de saisie
//   input_type  : 0=Chaîne, 1=Nombre, 2=Nom de fichier
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  byte max_size;
  // Grosse astuce pour les noms de fichiers: La taille affichée est différente
  // de la taille maximum gérée.
  if (input_type == 2)
    max_size = 255;
  else
    max_size = visible_size;
  return Readline_ex(x_pos,y_pos,str,visible_size,max_size,input_type);
}

/****************************************************************************
*           Enhanced super scanf deluxe pro plus giga mieux :-)             *
****************************************************************************/
byte Readline_ex(word x_pos,word y_pos,char * str,byte visible_size,byte max_size, byte input_type)
// Paramètres:
//   x_pos, y_pos : Coordonnées de la saisie dans la fenêtre
//   str       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   max_size  : Nombre de caractères logeant dans la zone de saisie
//   input_type  : 0=Chaîne, 1=Nombre, 2=Nom de fichier
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  char initial_string[256];
  char display_string[256];
  byte position;
  byte size;
  word input_key=0;
  byte is_authorized;

  byte offset=0; // index du premier caractère affiché

  Hide_cursor();
  // Effacement de la chaîne
  Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
  Update_rect(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));

  // Mise à jour des variables se rapportant à la chaîne en fonction de la chaîne initiale
  strcpy(initial_string,str);

  // Si on a commencé à editer par un clic-droit, on vide la chaine.
  if (Mouse_K==RIGHT_SIDE)
    str[0]='\0';
  else if (input_type==1)
    snprintf(str,10,"%d",atoi(str)); // On tasse la chaine à gauche


  size=strlen(str);
  position=(size<max_size)? size:size-1;
  if (position-offset>visible_size)
    offset=position-visible_size+1;
  // Formatage d'une partie de la chaine (si trop longue pour tenir)
  strncpy(display_string, str + offset, visible_size);
  display_string[visible_size]='\0';
  if (offset>0)
    display_string[0]=LEFT_TRIANGLE_CHARACTER;
  if (visible_size + offset + 1 < size )
    display_string[visible_size-1]=RIGHT_TRIANGLE_CHARACTER;
  
  Display_whole_string(x_pos,y_pos,display_string,position - offset);
  Update_rect(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));
  Flush_update();

  while ((input_key!=SDLK_RETURN) && (input_key!=KEY_ESC))
  {
    Display_cursor();
    do
    {
      if(!Get_input()) Wait_VBL();
      input_key=Key_ANSI;
    } while(input_key==0);
    Hide_cursor();
    switch (input_key)
    {
      case SDLK_DELETE : // Suppr.
            if (position<size)
            {
              Remove_character(str,position);
              size--;
              
              // Effacement de la chaîne
              Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
                    visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
              goto affichage;
            }
      break;
      case SDLK_LEFT : // Gauche
            if (position>0)
            {
              // Effacement de la chaîne
              if (position==size)
                Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
                      visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
              position--;
              if (offset > 0 && (position == 0 || position < (offset + 1)))
                offset--;
              goto affichage;
            }
      break;
      case SDLK_RIGHT : // Droite
            if ((position<size) && (position<max_size-1))
            {
              position++;
              //if (position > visible_size + offset - 2)
              //if (offset + visible_size < max_size && (position == size || (position > visible_size + offset - 2)))
              if (display_string[position-offset]==RIGHT_TRIANGLE_CHARACTER || position-offset>=visible_size)
                offset++;
              goto affichage;
            }
      break;
      case SDLK_HOME : // Home
            if (position)
            {
              // Effacement de la chaîne
              if (position==size)
                Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
                      visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
              position = 0;
              offset = 0;
              goto affichage;
            }
      break;
      case SDLK_END : // End
            if ((position<size) && (position<max_size-1))
            {
              position=(size<max_size)?size:size-1;
              if (position-offset>visible_size)
                offset=position-visible_size+1;
              goto affichage;
            }
      break;
      case  SDLK_BACKSPACE : // Backspace : combinaison de gauche + suppr

        if (position)
        {       
          position--;
          if (offset > 0 && (position == 0 || position < (offset + 1)))
            offset--;
          Remove_character(str,position);
          size--;
          // Effacement de la chaîne
          Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
                visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
          goto affichage;
        }
        break;
      case SDLK_RETURN :
        break;
        
      case KEY_ESC :
        // On restaure la chaine initiale
        strcpy(str,initial_string);
        size=strlen(str);
        break;
      default :
        if (size<max_size)
        {
          // On va regarder si l'utilisateur le droit de se servir de cette touche
          is_authorized=0; // On commence par supposer qu'elle est interdite
          switch(input_type)
          {
            case 0 : // N'importe quelle chaîne:
              if (input_key>=' ' && input_key<= 255)
                is_authorized=1;
              break;
            case 1 : // Nombre
              if ( (input_key>='0') && (input_key<='9') )
                is_authorized=1;
              break;
            default : // Nom de fichier
              // On regarde si la touche est autorisée
              if ( Valid_character(input_key))
                is_authorized=1;
          } // End du "switch(input_type)"

          // Si la touche était autorisée...
          if (is_authorized)
          {
            // ... alors on l'insère ...
            Insert_character(str,input_key,position/*,size*/);
            // ce qui augmente la taille de la chaine
            size++;
            // et qui risque de déplacer le curseur vers la droite
            if (size<max_size)
            {
              position++;
              if (display_string[position-offset]==RIGHT_TRIANGLE_CHARACTER || position-offset>=visible_size)
                offset++;
            }
            // Enfin, on raffiche la chaine
            goto affichage;
          } // End du test d'autorisation de touche
        } // End du test de place libre
        break;
      
affichage:
        size=strlen(str);
        // Formatage d'une partie de la chaine (si trop longue pour tenir)
        strncpy(display_string, str + offset, visible_size);
        display_string[visible_size]='\0';
        if (offset>0)
          display_string[0]=LEFT_TRIANGLE_CHARACTER;
        if (visible_size + offset + 0 < size )
          display_string[visible_size-1]=RIGHT_TRIANGLE_CHARACTER;
        
        Display_whole_string(x_pos,y_pos,display_string,position - offset);
        Update_rect(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));
    } // End du "switch(input_key)"
    Flush_update();

  } // End du "while"

  // Effacement de la chaîne
  Block(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
  // On raffiche la chaine correctement
  if (input_type==1)
  {
    if (str[0]=='\0')
    {
      strcpy(str,"0");
      size=1;
    }
    Print_in_window(x_pos+((max_size-size)<<3),y_pos,str,TEXT_COLOR,BACKGROUND_COLOR);
  }
  else
  {
    Print_in_window_limited(x_pos,y_pos,str,visible_size,TEXT_COLOR,BACKGROUND_COLOR);
  }
  Update_rect(Window_pos_X+(x_pos*Menu_factor_X),Window_pos_Y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));

  return (input_key==SDLK_RETURN);
}
