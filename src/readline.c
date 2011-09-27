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
/************************************************************************
*                                                                       *
* READLINE (procédure permettant de saisir une chaîne de caractères) *
*                                                                       *
************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "misc.h"
#include "errors.h"
#include "const.h"
#include "sdlscreen.h"
#include "readline.h"
#include "windows.h"
#include "input.h"
#include "engine.h"

#ifdef __WIN32__
#include <Windows.h>
#include <SDL_syswm.h>
#endif

// Virtual keyboard is mandatory on these platforms:
#if defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
  #ifndef VIRT_KEY
    #define VIRT_KEY 1
  #endif
#endif

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

int Prepend_string(char* dest, char* src, int max)
// Insert a string at the start of another. Up to MAX characters only
// Returns actual number of chars inserted
{
	// Insert src before dest
	int sized = strlen(dest);
	int sizes = strlen(src);

	if (sized + sizes >= max)
	{
		sizes = max - sized;
	}

	memmove(dest+sizes, dest, sized);
	memcpy(dest, src, sizes);

	return sizes;
}

int Valid_character(word c, int input_type)
	// returns 0 = Not allowed
	// returns 1 = Allowed
	// returns 2 = Allowed only once at start of string (for - sign in numbers)
{
	// On va regarder si l'utilisateur le droit de se servir de cette touche
	switch(input_type)
	{
		case INPUT_TYPE_STRING :
			if ((c>=' ' && c<= 255) || c=='\n')
				return 1;
			break;
		case INPUT_TYPE_INTEGER :
			if ( (c>='0') && (c<='9') )
				return 1;
			break;
		case INPUT_TYPE_DECIMAL:
			if ( (c>='0') && (c<='9') )
				return 1;
			else if (c=='-')
				return 2;
			else if (c=='.')
				return 1;
			break;
		case INPUT_TYPE_FILENAME:
		{
			// On regarde si la touche est autorisée
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
		case INPUT_TYPE_HEXA:
			if ( (c>='0') && (c<='9') )
				return 1;
			else if ( (c>='A') && (c<='F') )
				return 1;
			else if ( (c>='a') && (c<='f') )
				return 1;
			break;
	} // End du "switch(input_type)"
	return 0;
}

void Cleanup_string(char* str, int input_type)
{
	int i,j=0;
	
	for(i=0; str[i]!='\0'; i++)
	{
	  if (Valid_character((unsigned char)(str[i]), input_type))
	  {
	    str[j]=str[i];
	    j++;
	  }
	}
	str[j] = '\0';
}

void Display_whole_string(word x_pos,word y_pos,char * str,byte position)
{
	char cursor[2];
	Print_general(x_pos,y_pos,str,TEXT_COLOR,BACKGROUND_COLOR);

	cursor[0]=str[position] ? str[position] : ' ';
	cursor[1]='\0';
	Print_general(x_pos+(position<<3)*Menu_factor_X,y_pos,cursor,CURSOR_COLOR,CURSOR_BACKGROUND_COLOR);
}

void Init_virtual_keyboard(word y_pos, word keyboard_width, word keyboard_height)
{
	int h_pos;
	int v_pos;
	int parent_window_x=Window_pos_X+2;

	h_pos= Window_pos_X+(keyboard_width-Window_width)*Menu_factor_X/-2;
	if (h_pos<0)
		h_pos=0;
	else if (h_pos+keyboard_width*Menu_factor_X>Screen_width)
		h_pos=Screen_width-keyboard_width*Menu_factor_X;
	v_pos=Window_pos_Y+(y_pos+9)*Menu_factor_Y;
	if (v_pos+(keyboard_height*Menu_factor_Y)>Screen_height)
		v_pos=Window_pos_Y+(y_pos-keyboard_height-4)*Menu_factor_Y;

	Hide_cursor();
	Open_popup(h_pos,v_pos,keyboard_width,keyboard_height);
	Window_rectangle(1,0,Window_width-1, Window_height-1, MC_Light);
	Window_rectangle(0,0,1,Window_height-2, MC_White);
	// white border on top left angle, when it exceeds border.
	if (parent_window_x>Window_pos_X)
		Window_rectangle(0,0,(parent_window_x-Window_pos_X)/Menu_factor_X, 1, MC_White);
	Window_rectangle(2,Window_height-2,Window_width-2, 2, MC_Black);
	if(keyboard_width<320)
	{
		Window_rectangle(Window_width-2,2,2,Window_height-2, MC_Black);
	}
}


// Inspired from http://www.libsdl.org/projects/scrap/
// TODO X11 and others
char* getClipboard()
{
	char* dst = NULL;
#ifdef __WIN32__
    SDL_SysWMinfo info;
    HWND SDL_Window;

    SDL_VERSION(&info.version);

    if ( SDL_GetWMInfo(&info) )
    {
      SDL_Window = info.window;

      if ( IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(SDL_Window) )
      {
        HANDLE hMem;
        char *src;

        hMem = GetClipboardData(CF_TEXT);
        if ( hMem != NULL )
        {
          src = (char *)GlobalLock(hMem);
          dst = strdup(src);
          GlobalUnlock(hMem);
        }
        CloseClipboard();
      }    
    }
  #elif defined __HAIKU__
	return haiku_get_clipboard();
  #else
    #warning "Missing platform-specific code in getClipboard function"
  #endif
  return dst;
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
  return Readline_ex(x_pos,y_pos,str,visible_size,max_size,input_type,0);
}

/****************************************************************************
*           Enhanced super scanf deluxe pro plus giga mieux :-)             *
****************************************************************************/
byte Readline_ex(word x_pos,word y_pos,char * str,byte visible_size,byte max_size, byte input_type, byte decimal_places)
// Paramètres:
//   x_pos, y_pos : Coordonnées de la saisie dans la fenêtre
//   str       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   max_size  : Nombre de caractères logeant dans la zone de saisie
//   input_type  : 0=String, 1=Unsigned int, 2=Filename 3=Signed Double
//   decimal_places: Number of decimal places for a double
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  char initial_string[256];
  char display_string[256];
  byte position;
  byte size;
  word input_key=0;
  word window_x=Window_pos_X;
  word window_y=Window_pos_Y;
  byte offset=0; // index du premier caractère affiché
  
#ifdef VIRT_KEY
  // Virtual keyboard
  byte use_virtual_keyboard=0;
  static byte caps_lock=0;
  word keymapping[] =
  {
    SDLK_CLEAR,SDLK_BACKSPACE,SDLK_RETURN,KEY_ESC,
    '0','1','2','3','4','5','6','7','8','9','.',',',
    'Q','W','E','R','T','Y','U','I','O','P',
    'A','S','D','F','G','H','J','K','L',
    SDLK_CAPSLOCK,'Z','X','C','V','B','N','M',' ',
    '-','+','*','/','|','\\',
    '(',')','{','}','[',']',
    '_','=','<','>','%','@',
    ':',';','`','\'','"','~',
    '!','?','^','&','#','$'
  };
#endif

  // Si on a commencé à editer par un clic-droit, on vide la chaine.
  if (Mouse_K==RIGHT_SIDE)
    str[0]='\0';
  else if (input_type==INPUT_TYPE_INTEGER && str[0]!='\0')
    snprintf(str,10,"%d",atoi(str)); // On tasse la chaine à gauche
  else if (input_type==INPUT_TYPE_DECIMAL)
  {
    //  Nothing. The caller should have used Sprint_double, with min_positions
    //  at zero, so there's no spaces on the left and no useless 0s on the right.
  }
  else if (input_type==INPUT_TYPE_HEXA)
  {
    //  Nothing. The caller should have initialized a valid hexa number.
  }
  
  // Virtual keyboards
#ifdef VIRT_KEY
  if (input_type == INPUT_TYPE_STRING || input_type == INPUT_TYPE_FILENAME )
  {
    int x,y;

    Init_virtual_keyboard(y_pos, 320, 87);
    
    use_virtual_keyboard=1;
    
    // The order is important, see the array
    
    Window_set_normal_button(  7,67,43,15,"Clr", 0,1,KEY_NONE);
    Window_set_normal_button( 51,67,43,15,"Del", 0,1,KEY_NONE);
    Window_set_normal_button( 95,67,43,15,"OK",  0,1,KEY_NONE);
    Window_set_normal_button(139,67,43,15,"Esc", 0,1,KEY_NONE);
    Window_display_frame_in(5,65,179,19);

    Window_set_normal_button(193,63,17,19,"0", 0,1,KEY_NONE);
    Window_set_normal_button(193,43,17,19,"1", 0,1,KEY_NONE);
    Window_set_normal_button(211,43,17,19,"2", 0,1,KEY_NONE);
    Window_set_normal_button(229,43,17,19,"3", 0,1,KEY_NONE);
    Window_set_normal_button(193,23,17,19,"4", 0,1,KEY_NONE);
    Window_set_normal_button(211,23,17,19,"5", 0,1,KEY_NONE);
    Window_set_normal_button(229,23,17,19,"6", 0,1,KEY_NONE);
    Window_set_normal_button(193, 3,17,19,"7", 0,1,KEY_NONE);
    Window_set_normal_button(211, 3,17,19,"8", 0,1,KEY_NONE);
    Window_set_normal_button(229, 3,17,19,"9", 0,1,KEY_NONE);
    Window_set_normal_button(211,63,17,19,".", 0,1,KEY_NONE);
    Window_set_normal_button(229,63,17,19,",", 0,1,KEY_NONE);
 
    Window_set_normal_button(  3, 3,18,19,"Q", 0,1,KEY_NONE);
    Window_set_normal_button( 22, 3,18,19,"W", 0,1,KEY_NONE);
    Window_set_normal_button( 41, 3,18,19,"E", 0,1,KEY_NONE);
    Window_set_normal_button( 60, 3,18,19,"R", 0,1,KEY_NONE);
    Window_set_normal_button( 79, 3,18,19,"T", 0,1,KEY_NONE);
    Window_set_normal_button( 98, 3,18,19,"Y", 0,1,KEY_NONE);
    Window_set_normal_button(117, 3,18,19,"U", 0,1,KEY_NONE);
    Window_set_normal_button(136, 3,18,19,"I", 0,1,KEY_NONE);
    Window_set_normal_button(155, 3,18,19,"O", 0,1,KEY_NONE);
    Window_set_normal_button(174, 3,18,19,"P", 0,1,KEY_NONE);

    Window_set_normal_button( 12,23,18,19,"A", 0,1,KEY_NONE);
    Window_set_normal_button( 31,23,18,19,"S", 0,1,KEY_NONE);
    Window_set_normal_button( 50,23,18,19,"D", 0,1,KEY_NONE);
    Window_set_normal_button( 69,23,18,19,"F", 0,1,KEY_NONE);
    Window_set_normal_button( 88,23,18,19,"G", 0,1,KEY_NONE);
    Window_set_normal_button(107,23,18,19,"H", 0,1,KEY_NONE);
    Window_set_normal_button(126,23,18,19,"J", 0,1,KEY_NONE);
    Window_set_normal_button(145,23,18,19,"K", 0,1,KEY_NONE);
    Window_set_normal_button(164,23,18,19,"L", 0,1,KEY_NONE);
    
    Window_set_normal_button(  3,43,18,19,caps_lock?"\036":"\037", 0,1,KEY_NONE);
    Window_set_normal_button( 22,43,18,19,"Z", 0,1,KEY_NONE);
    Window_set_normal_button( 41,43,18,19,"X", 0,1,KEY_NONE);
    Window_set_normal_button( 60,43,18,19,"C", 0,1,KEY_NONE);
    Window_set_normal_button( 79,43,18,19,"V", 0,1,KEY_NONE);
    Window_set_normal_button( 98,43,18,19,"B", 0,1,KEY_NONE);
    Window_set_normal_button(117,43,18,19,"N", 0,1,KEY_NONE);
    Window_set_normal_button(136,43,18,19,"M", 0,1,KEY_NONE);
    Window_set_normal_button(155,43,18,19," ", 0,1,KEY_NONE);

    for (y=0; y<5; y++)
    {
      for (x=0; x<6; x++)
      {
        char label[2]=" ";
        label[0]=keymapping[x+y*6+44];        
        Window_set_normal_button(247+x*12, 3+y*16,11,15,label, 0,1,KEY_NONE);
      }
    }

    Update_window_area(0,0,Window_width, Window_height);
    Display_cursor();
  }
  else if (input_type == INPUT_TYPE_INTEGER || input_type == INPUT_TYPE_DECIMAL )
  {
    Init_virtual_keyboard(y_pos, 215, 47);
    
    use_virtual_keyboard=1;
    
    // The order is important, see the array
    
    Window_set_normal_button(  7,27,43,15,"Clr", 0,1,KEY_NONE);
    Window_set_normal_button( 51,27,43,15,"Del", 0,1,KEY_NONE);
    Window_set_normal_button( 95,27,43,15,"OK",  0,1,KEY_NONE);
    Window_set_normal_button(139,27,43,15,"Esc", 0,1,KEY_NONE);
    Window_display_frame_in(5,25,179,19);

    Window_set_normal_button(174, 3,18,19,"0", 0,1,KEY_NONE);
    Window_set_normal_button(  3, 3,18,19,"1", 0,1,KEY_NONE);
    Window_set_normal_button( 22, 3,18,19,"2", 0,1,KEY_NONE);
    Window_set_normal_button( 41, 3,18,19,"3", 0,1,KEY_NONE);
    Window_set_normal_button( 60, 3,18,19,"4", 0,1,KEY_NONE);
    Window_set_normal_button( 79, 3,18,19,"5", 0,1,KEY_NONE);
    Window_set_normal_button( 98, 3,18,19,"6", 0,1,KEY_NONE);
    Window_set_normal_button(117, 3,18,19,"7", 0,1,KEY_NONE);
    Window_set_normal_button(136, 3,18,19,"8", 0,1,KEY_NONE);
    Window_set_normal_button(155, 3,18,19,"9", 0,1,KEY_NONE);
    Window_set_normal_button(193, 3,18,19,".", 0,1,KEY_NONE);

    Update_window_area(0,0,Window_width, Window_height);
    Display_cursor();
  }
#endif
  Keyboard_click_allowed = 0;
  Hide_cursor();

  // Effacement de la chaîne
  Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
  Update_rect(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));

  // Mise à jour des variables se rapportant à la chaîne en fonction de la chaîne initiale
  strcpy(initial_string,str);

  size=strlen(str);
  position=(size<max_size)? size:size-1;
  if (position-offset>=visible_size)
    offset=position-visible_size+1;
  // Formatage d'une partie de la chaine (si trop longue pour tenir)
  strncpy(display_string, str + offset, visible_size);
  display_string[visible_size]='\0';
  if (offset>0)
    display_string[0]=LEFT_TRIANGLE_CHARACTER;
  if (visible_size + offset + 1 < size )
    display_string[visible_size-1]=RIGHT_TRIANGLE_CHARACTER;
  
  Display_whole_string(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),display_string,position - offset);
  Update_rect(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));
  Flush_update();
  if (Mouse_K)
  {
    Display_cursor();
    Wait_end_of_click();
    Hide_cursor();
  }

  while ((input_key!=SDLK_RETURN) && (input_key!=KEY_ESC))
  {
    Display_cursor();
#ifdef VIRT_KEY
    if (use_virtual_keyboard)
    {
      int clicked_button;

      clicked_button=Window_clicked_button();
      input_key=Key_ANSI;

      if (clicked_button==-1)
        input_key=SDLK_RETURN;
      else if (clicked_button>0)
      {
        input_key=keymapping[clicked_button-1];
        if (input_key==SDLK_CAPSLOCK)
        {
          // toggle uppercase
          caps_lock=!caps_lock;
          Hide_cursor();
          Print_in_window(8, 49,caps_lock?"\036":"\037", MC_Black,MC_Light);
          Display_cursor();
        }
        else if (input_key==SDLK_BACKSPACE)
        {
          // A little hack: the button for backspace will:
          // - backspace if the cursor is at end of string
          // - delete otherwise
          // It's needed for those input boxes that are completely full.
          if (position<size)
            input_key = SDLK_DELETE;
        }
        else if (input_key>='A' && input_key<='Z' && !caps_lock)
        {
          input_key+='a'-'A';
        }
      }
    }
    else
#endif
    {
      do
      {
        Get_input(20);
        input_key=Key_ANSI;
        if (Mouse_K)
          input_key=SDLK_RETURN;

        // Handle paste request on CTRL+v
#if defined __HAIKU__
	#define SHORTCUTKEY MOD_ALT
#else
	#define SHORTCUTKEY MOD_CTRL
#endif
        if ((Key & SHORTCUTKEY) && ((Key & 0xFFF) == 'v'))
        {
          char* data = getClipboard();
          if (data == NULL) continue; // No clipboard data
		  Cleanup_string(data, input_type);
          // Insert it at the cursor position
		  position += Prepend_string(str + position, data, max_size - position);
          free(data);
          goto affichage;
        }
      } while(input_key==0);
#undef SHORTCUTKEY
    }
    Hide_cursor();

    switch (input_key)
    {
      case SDLK_DELETE : // Suppr.
            if (position<size)
            {
              Remove_character(str,position);
              size--;
              
              // Effacement de la chaîne
              Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
                    visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
              goto affichage;
            }
      break;
      case SDLK_LEFT : // Gauche
            if (position>0)
            {
              // Effacement de la chaîne
              if (position==size)
                Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
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
                Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
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
              if (position-offset>=visible_size)
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
          Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
                visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
          goto affichage;
        }
        break;
      case  SDLK_CLEAR : // Clear
        str[0]='\0';
        position=offset=0;
        // Effacement de la chaîne
        Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
              visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
        goto affichage;
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
          // Si la touche était autorisée...
		  byte is_authorized = Valid_character(input_key, input_type);
          if (is_authorized == 1 || (is_authorized == 2 && position == 0 && str[position] != '-'))
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
        
        Display_whole_string(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),display_string,position - offset);
        Update_rect(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));
    } // End du "switch(input_key)"
    Flush_update();

  } // End du "while"
  Keyboard_click_allowed = 1;
  #ifdef VIRT_KEY
  if (use_virtual_keyboard)
  {
    byte old_mouse_k = Mouse_K;
    Close_popup();
    Mouse_K=old_mouse_k;
    Input_sticky_control=0;
  }
  #endif
  
  // Effacement de la chaîne
  Block(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3),BACKGROUND_COLOR);
  // On raffiche la chaine correctement
  if (input_type==INPUT_TYPE_INTEGER)
  {
    if (str[0]=='\0')
    {
      strcpy(str,"0");
      size=1;
    }
    Print_in_window(x_pos+((max_size-size)<<3),y_pos,str,TEXT_COLOR,BACKGROUND_COLOR);
  }
  else if (input_type==INPUT_TYPE_DECIMAL)
  {
    double value;
    // Discard extra digits
    value = Fround(atof(str), decimal_places);
    Sprint_double(str,value,decimal_places,visible_size);
    // Recompute updated size
    size = strlen(str);
    
    if (size<=visible_size)
      Print_in_window(x_pos+((visible_size-size)<<3),y_pos,str,TEXT_COLOR,BACKGROUND_COLOR);
    else
      Print_in_window_limited(x_pos,y_pos,str,visible_size,TEXT_COLOR,BACKGROUND_COLOR);
  }
  else
  {
    Print_in_window_limited(x_pos,y_pos,str,visible_size,TEXT_COLOR,BACKGROUND_COLOR);
  }
  Update_rect(window_x+(x_pos*Menu_factor_X),window_y+(y_pos*Menu_factor_Y),
        visible_size*(Menu_factor_X<<3),(Menu_factor_Y<<3));

  return (input_key==SDLK_RETURN);
}

void Sprint_double(char *str, double value, byte decimal_places, byte min_positions)
{
  int i;
  int length;
  
  sprintf(str,"%.*f",decimal_places, value);
  length=strlen(str);

  for (i=0; i<length; i++)
  {
    if (str[i]=='.')
    {
      // Remove extraneous zeroes
      char * decimals = str+i+1;
      int j;
      
      for (j=strlen(decimals)-1; j >= 0 && decimals[j]=='0'; j--)
      {
          decimals[j] = '\0';
      }
      // If all decimals were removed, remove the dot too
      if (str[i+1]=='\0')
        str[i]='\0';
      
      // Update string length
      length=strlen(str);
      
      // Ends the parent loop
      break; 
    }
  }
  
  // Now try add spaces at beginning
  if (length<min_positions)
  {
    int offset = min_positions - length;
    
    // Move the string to the right
    for (i=0; i<=length; i++)
    {
      str[length+offset-i] = str[length-i];
    }
    // Replace the N first characters by spaces
    for (i=0; i<offset; i++)
    {
      str[i] = ' ';
    }
  }
}
