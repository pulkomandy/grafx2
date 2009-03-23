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

#define COULEUR_TEXTE         CM_Noir
#define COULEUR_FOND          CM_Clair
#define COULEUR_TEXTE_CURSEUR CM_Noir
#define COULEUR_FOND_CURSEUR  CM_Fonce

// Suppresion d'un caractère à une certaine POSITION dans une CHAINE.
void Supprimer_caractere(char * Chaine, byte position)
{
  for (;Chaine[position]!='\0';position++)
    Chaine[position]=Chaine[position+1];
}


void Inserer_caractere(char * Chaine, char letter, byte position)
//  Insertion d'une LETTRE à une certaine POSITION
//  dans une CHAINE d'une certaine TAILLE.
{
  char Char_tempo;

  for (;letter!='\0';position++)
  {
    // On mémorise le caractère qui se trouve en "position"
    Char_tempo=Chaine[position];
    // On splotch la lettre à insérer
    Chaine[position]=letter;
    // On place le caractère mémorisé dans "letter" comme nouvelle lettre à insérer
    letter=Char_tempo;
  }
  // On termine la chaine
  Chaine[position]='\0';
}

int CaractereValide(int c)
{
  // Sous Linux: Seul le / est strictement interdit, mais beaucoup
  // d'autres poseront des problèmes au shell, alors on évite.
  // Sous Windows : c'est moins grave car le fopen() échouerait de toutes façons.
  // AmigaOS4: Pas de ':' car utilisé pour les volumes.
  #if defined(__WIN32__)
  char CaracteresInterdits[] = {'/', '|', '?', '*', '<', '>', ':', '\\'};
  #elif defined (__amigaos4__)
  char CaracteresInterdits[] = {'/', '|', '?', '*', '<', '>', ':'};
  #else
  char CaracteresInterdits[] = {'/', '|', '?', '*', '<', '>'};
  #endif
  int position;
  
  if (c < ' ' || c > 255)
    return 0;
  
  for (position=0; position<(long)sizeof(CaracteresInterdits); position++)
    if (c == CaracteresInterdits[position])
      return 0;
  return 1;
}

void Rafficher_toute_la_chaine(word x_pos,word y_pos,char * Chaine,byte position)
{
  Print_dans_fenetre(x_pos,y_pos,Chaine,COULEUR_TEXTE,COULEUR_FOND);
  Print_char_dans_fenetre(x_pos+(position<<3),y_pos,Chaine[position],COULEUR_TEXTE_CURSEUR,COULEUR_FOND_CURSEUR);
}

/****************************************************************************
*           Enhanced super scanf deluxe pro plus giga mieux :-)             *
****************************************************************************/
byte Readline(word x_pos,word y_pos,char * Chaine,byte Taille_affichee,byte Type_saisie)
// Paramètres:
//   x_pos, y_pos : Coordonnées de la saisie dans la fenêtre
//   Chaine       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractères logeant dans la zone de saisie
//   Type_saisie  : 0=Chaîne, 1=Nombre, 2=Nom de fichier
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  byte Taille_maxi;
  // Grosse astuce pour les noms de fichiers: La taille affichée est différente
  // de la taille maximum gérée.
  if (Type_saisie == 2)
    Taille_maxi = 255;
  else
    Taille_maxi = Taille_affichee;
  return Readline_ex(x_pos,y_pos,Chaine,Taille_affichee,Taille_maxi,Type_saisie);
}

/****************************************************************************
*           Enhanced super scanf deluxe pro plus giga mieux :-)             *
****************************************************************************/
byte Readline_ex(word x_pos,word y_pos,char * Chaine,byte Taille_affichee,byte Taille_maxi, byte Type_saisie)
// Paramètres:
//   x_pos, y_pos : Coordonnées de la saisie dans la fenêtre
//   Chaine       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractères logeant dans la zone de saisie
//   Type_saisie  : 0=Chaîne, 1=Nombre, 2=Nom de fichier
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  char Chaine_initiale[256];
  char Chaine_affichee[256];
  byte position;
  byte Taille;
  word Touche_lue=0;
  byte Touche_autorisee;

  byte offset=0; // Indice du premier caractère affiché

  Effacer_curseur();
  // Effacement de la chaîne
  Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
  UpdateRect(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3));

  // Mise à jour des variables se rapportant à la chaîne en fonction de la chaîne initiale
  strcpy(Chaine_initiale,Chaine);

  // Si on a commencé à editer par un clic-droit, on vide la chaine.
  if (Mouse_K==A_DROITE)
    Chaine[0]='\0';
  else if (Type_saisie==1)
    snprintf(Chaine,10,"%d",atoi(Chaine)); // On tasse la chaine à gauche


  Taille=strlen(Chaine);
  position=(Taille<Taille_maxi)? Taille:Taille-1;
  if (position-offset>Taille_affichee)
    offset=position-Taille_affichee+1;
  // Formatage d'une partie de la chaine (si trop longue pour tenir)
  strncpy(Chaine_affichee, Chaine + offset, Taille_affichee);
  Chaine_affichee[Taille_affichee]='\0';
  if (offset>0)
    Chaine_affichee[0]=CARACTERE_TRIANGLE_GAUCHE;
  if (Taille_affichee + offset + 1 < Taille )
    Chaine_affichee[Taille_affichee-1]=CARACTERE_TRIANGLE_DROIT;
  
  Rafficher_toute_la_chaine(x_pos,y_pos,Chaine_affichee,position - offset);
  UpdateRect(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3));
  Flush_update();

  while ((Touche_lue!=SDLK_RETURN) && (Touche_lue!=TOUCHE_ESC))
  {
    Afficher_curseur();
    do
    {
      if(!Get_input()) Wait_VBL();
      Touche_lue=Touche_ANSI;
    } while(Touche_lue==0);
    Effacer_curseur();
    switch (Touche_lue)
    {
      case SDLK_DELETE : // Suppr.
            if (position<Taille)
            {
              Supprimer_caractere(Chaine,position);
              Taille--;
              
              // Effacement de la chaîne
              Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
                    Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              goto affichage;
            }
      break;
      case SDLK_LEFT : // Gauche
            if (position>0)
            {
              // Effacement de la chaîne
              if (position==Taille)
                Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
                      Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              position--;
              if (offset > 0 && (position == 0 || position < (offset + 1)))
                offset--;
              goto affichage;
            }
      break;
      case SDLK_RIGHT : // Droite
            if ((position<Taille) && (position<Taille_maxi-1))
            {
              position++;
              //if (position > Taille_affichee + offset - 2)
              //if (offset + Taille_affichee < Taille_maxi && (position == Taille || (position > Taille_affichee + offset - 2)))
              if (Chaine_affichee[position-offset]==CARACTERE_TRIANGLE_DROIT || position-offset>=Taille_affichee)
                offset++;
              goto affichage;
            }
      break;
      case SDLK_HOME : // Home
            if (position)
            {
              // Effacement de la chaîne
              if (position==Taille)
                Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
                      Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              position = 0;
              offset = 0;
              goto affichage;
            }
      break;
      case SDLK_END : // End
            if ((position<Taille) && (position<Taille_maxi-1))
            {
              position=(Taille<Taille_maxi)?Taille:Taille-1;
              if (position-offset>Taille_affichee)
                offset=position-Taille_affichee+1;
              goto affichage;
            }
      break;
      case  SDLK_BACKSPACE : // Backspace : combinaison de gauche + suppr

        if (position)
        {       
          position--;
          if (offset > 0 && (position == 0 || position < (offset + 1)))
            offset--;
          Supprimer_caractere(Chaine,position);
          Taille--;
          // Effacement de la chaîne
          Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
                Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
          goto affichage;
        }
        break;
      case SDLK_RETURN :
        break;
        
      case TOUCHE_ESC :
        // On restaure la chaine initiale
        strcpy(Chaine,Chaine_initiale);
        Taille=strlen(Chaine);
        break;
      default :
        if (Taille<Taille_maxi)
        {
          // On va regarder si l'utilisateur le droit de se servir de cette touche
          Touche_autorisee=0; // On commence par supposer qu'elle est interdite
          switch(Type_saisie)
          {
            case 0 : // N'importe quelle chaîne:
              if (Touche_lue>=' ' && Touche_lue<= 255)
                Touche_autorisee=1;
              break;
            case 1 : // Nombre
              if ( (Touche_lue>='0') && (Touche_lue<='9') )
                Touche_autorisee=1;
              break;
            default : // Nom de fichier
              // On regarde si la touche est autorisée
              if ( CaractereValide(Touche_lue))
                Touche_autorisee=1;
          } // Fin du "switch(Type_saisie)"

          // Si la touche était autorisée...
          if (Touche_autorisee)
          {
            // ... alors on l'insère ...
            Inserer_caractere(Chaine,Touche_lue,position/*,Taille*/);
            // ce qui augmente la taille de la chaine
            Taille++;
            // et qui risque de déplacer le curseur vers la droite
            if (Taille<Taille_maxi)
            {
              position++;
              if (Chaine_affichee[position-offset]==CARACTERE_TRIANGLE_DROIT || position-offset>=Taille_affichee)
                offset++;
            }
            // Enfin, on raffiche la chaine
            goto affichage;
          } // Fin du test d'autorisation de touche
        } // Fin du test de place libre
        break;
      
affichage:
        Taille=strlen(Chaine);
        // Formatage d'une partie de la chaine (si trop longue pour tenir)
        strncpy(Chaine_affichee, Chaine + offset, Taille_affichee);
        Chaine_affichee[Taille_affichee]='\0';
        if (offset>0)
          Chaine_affichee[0]=CARACTERE_TRIANGLE_GAUCHE;
        if (Taille_affichee + offset + 0 < Taille )
          Chaine_affichee[Taille_affichee-1]=CARACTERE_TRIANGLE_DROIT;
        
        Rafficher_toute_la_chaine(x_pos,y_pos,Chaine_affichee,position - offset);
        UpdateRect(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3));
    } // Fin du "switch(Touche_lue)"
    Flush_update();

  } // Fin du "while"

  // Effacement de la chaîne
  Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
  // On raffiche la chaine correctement
  if (Type_saisie==1)
  {
    if (Chaine[0]=='\0')
    {
      strcpy(Chaine,"0");
      Taille=1;
    }
    Print_dans_fenetre(x_pos+((Taille_maxi-Taille)<<3),y_pos,Chaine,COULEUR_TEXTE,COULEUR_FOND);
  }
  else
  {
    Print_dans_fenetre_limite(x_pos,y_pos,Chaine,Taille_affichee,COULEUR_TEXTE,COULEUR_FOND);
  }
  UpdateRect(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3));

  return (Touche_lue==SDLK_RETURN);
}
