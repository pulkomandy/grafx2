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
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "moteur.h"
#include "graph.h"
#include "operatio.h"
#include "boutons.h"
#include "pages.h"
#include "erreurs.h"
#include "sdlscreen.h"
#include "brush.h"
#include "windows.h"

#ifdef __WATCOMC__
    #define M_PI 3.14159265358979323846 
#endif

void Demarrer_pile_operation(word Operation_demandee)
{
  Brosse_Centre_rotation_defini=0;

  // On mémorise l'opération précédente si on démarre une interruption
  switch(Operation_demandee)
  {
    case OPERATION_LOUPE         :
    case OPERATION_PIPETTE       :
    case OPERATION_PRISE_BROSSE  :
    case OPERATION_POLYBROSSE    :
    case OPERATION_ETIRER_BROSSE :
    case OPERATION_TOURNER_BROSSE:
      Operation_avant_interruption=Operation_en_cours;
      // On passe à l'operation demandée
      Operation_en_cours=Operation_demandee;
      break;
    default :
      // On passe à l'operation demandée
      Operation_en_cours=Operation_demandee;
      Operation_avant_interruption=Operation_en_cours;
  }

  // On spécifie si l'opération autorise le changement de couleur au clavier
  switch(Operation_demandee)
  {
    case OPERATION_DESSIN_CONTINU:
    case OPERATION_DESSIN_DISCONTINU:
    case OPERATION_SPRAY:
    case OPERATION_LIGNES_CENTREES:
      Autoriser_changement_de_couleur_pendant_operation=1;
      break;
    default :
      Autoriser_changement_de_couleur_pendant_operation=0;
  }

  // Et on passe au curseur qui va avec
  Forme_curseur=CURSEUR_D_OPERATION[Operation_demandee];
  Operation_Taille_pile=0;
}


void Initialiser_debut_operation(void)
{
  Operation_dans_loupe=(Mouse_X>=Principal_X_Zoom);
  Smear_Debut=1;
}


void Operation_PUSH(short Valeur)
{
  Operation_Pile[++Operation_Taille_pile]=Valeur;
}


void Operation_POP(short * Valeur)
{
  *Valeur=Operation_Pile[Operation_Taille_pile--];
}


byte Pinceau_Forme_avant_operation;
byte Cacher_pinceau_avant_operation;



short Distance(short X1, short Y1, short X2, short Y2)
{
  short X2_moins_X1=X2-X1;
  short Y2_moins_Y1=Y2-Y1;

  return Round( sqrt( (X2_moins_X1*X2_moins_X1) + (Y2_moins_Y1*Y2_moins_Y1) ) );
}


void Aff_coords_rel_ou_abs(short Debut_X, short Debut_Y)
{
  char Chaine[6];

  if (Config.Coords_rel)
  {
    if (Menu_visible)
    {
      if (Pinceau_X>Debut_X)
      {
        Num2str(Pinceau_X-Debut_X,Chaine,5);
        Chaine[0]='+';
      }
      else if (Pinceau_X<Debut_X)
      {
        Num2str(Debut_X-Pinceau_X,Chaine,5);
        Chaine[0]='-';
      }
      else
        strcpy(Chaine,"±   0");
      Print_dans_menu(Chaine,2);

      if (Pinceau_Y>Debut_Y)
      {
        Num2str(Pinceau_Y-Debut_Y,Chaine,5);
        Chaine[0]='+';
      }
      else if (Pinceau_Y<Debut_Y)
      {
        Num2str(Debut_Y-Pinceau_Y,Chaine,5);
        Chaine[0]='-';
      }
      else
        strcpy(Chaine,"±   0");
      Print_dans_menu(Chaine,12);
    }
  }
  else
    Print_coordonnees();
}

//////////////////////////////////////////////////// OPERATION_DESSIN_CONTINU

void Freehand_Mode1_1_0(void)
//  Opération   : OPERATION_DESSIN_CONTINU
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_gauche;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,0);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Freehand_Mode1_1_2(void)
//  Opération   : OPERATION_DESSIN_CONTINU
//  Click Souris: 1
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;

  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if ( (Debut_Y!=Pinceau_Y) || (Debut_X!=Pinceau_X) )
  {
    Effacer_curseur();
    Print_coordonnees();
    Tracer_ligne_Definitif(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Fore_color);
    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Freehand_Mode12_0_2(void)
//  Opération   : OPERATION_DESSIN_[DIS]CONTINU
//  Click Souris: 0
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  Operation_Taille_pile=0;
}


void Freehand_Mode1_2_0(void)
//  Opération   : OPERATION_DESSIN_CONTINU
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_droite;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Back_color,0);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Freehand_Mode1_2_2(void)
//  Opération   : OPERATION_DESSIN_CONTINU
//  Click Souris: 2
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;

  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if ( (Debut_Y!=Pinceau_Y) || (Debut_X!=Pinceau_X) )
  {
    Print_coordonnees();
    Effacer_curseur();
    Tracer_ligne_Definitif(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Back_color);
    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


///////////////////////////////////////////////// OPERATION_DESSIN_DISCONTINU

void Freehand_Mode2_1_0(void)
//  Opération   : OPERATION_DESSIN_DISCONTINU
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_gauche;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,0);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Print_coordonnees();
  Wait_VBL();
}


void Freehand_Mode2_1_2(void)
//  Opération   : OPERATION_DESSIN_DISCONTINU
//  Click Souris: 1
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;

  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if ( (Debut_X!=Pinceau_X) || (Debut_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    // On affiche définitivement le pinceau
    Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,0);
    Afficher_curseur();
    Print_coordonnees();
    Wait_VBL();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Freehand_Mode2_2_0(void)
//  Opération   : OPERATION_DESSIN_DISCONTINU
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_droite;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Back_color,0);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Print_coordonnees();
  Wait_VBL();
}


void Freehand_Mode2_2_2(void)
//  Opération   : OPERATION_DESSIN_DISCONTINU
//  Click Souris: 2
//  Taille_Pile : 2
//
//  Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;

  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if ( (Debut_X!=Pinceau_X) || (Debut_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    // On affiche définitivement le pinceau
    Afficher_pinceau(Pinceau_X,Pinceau_Y,Back_color,0);
    Afficher_curseur();
    Print_coordonnees();
    Wait_VBL();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


////////////////////////////////////////////////////// OPERATION_DESSIN_POINT

void Freehand_Mode3_1_0(void)
//  Opération   : OPERATION_DESSIN_POINT
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_gauche;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,0);
  Operation_PUSH(0);  // On change simplement l'état de la pile...
}


void Freehand_Mode3_2_0(void)
//  Opération   : OPERATION_DESSIN_POINT
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_droite;
  // On affiche définitivement le pinceau
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Back_color,0);
  Operation_PUSH(0);  // On change simplement l'état de la pile...
}


void Freehand_Mode3_0_1(void)
//  Opération   : OPERATION_DESSIN_POINT
//  Click Souris: 0
//  Taille_Pile : 1
//
//  Souris effacée: Non
{
  Operation_Taille_pile--;
}


///////////////////////////////////////////////////////////// OPERATION_LIGNE

void Ligne_12_0(void)
// Opération   : OPERATION_LIGNE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Oui

//  Début du tracé d'une ligne (premier clic)
{
  Initialiser_debut_operation();
  Backup();
  Pinceau_Forme_avant_operation=Pinceau_Forme;
  Pinceau_Forme=FORME_PINCEAU_POINT;

  if (Mouse_K==A_GAUCHE)
  {
    Shade_Table=Shade_Table_gauche;
    Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Fore_color);
    Operation_PUSH(Fore_color);
  }
  else
  {
    Shade_Table=Shade_Table_droite;
    Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Back_color);
    Operation_PUSH(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Ligne_12_5(void)
// Opération   : OPERATION_LIGNE
// Click Souris: 1
// Taille_Pile : 5
//
// Souris effacée: Non

// Poursuite du tracé d'une ligne (déplacement de la souris en gardant le curseur appuyé)
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  // On corrige les coordonnées de la ligne si la touche shift est appuyée...
  if(SDL_GetModState() & KMOD_SHIFT)
  {
    Rectifier_coordonnees_a_45_degres(Debut_X,Debut_Y,&Pinceau_X,&Pinceau_Y);
  }

  // On vient de bouger
  if ((Pinceau_X!=Fin_X) || (Pinceau_Y!=Fin_Y))
  {
      Effacer_curseur();

    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);

    Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);
    if (Mouse_K==A_GAUCHE)
    {
      Pixel_figure_Preview (Debut_X,Debut_Y,Fore_color);
      Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Fore_color);
    }
    else
    {
      Pixel_figure_Preview (Debut_X,Debut_Y,Back_color);
      Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Back_color);
    }

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);

    Afficher_curseur();
  }
  else
  {
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Fin_X);
    Operation_PUSH(Fin_Y);
  }
}


void Ligne_0_5(void)
// Opération   : OPERATION_LIGNE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui

// Fin du tracé d'une ligne (relachage du bouton)
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);

  // On corrige les coordonnées de la ligne si la touche shift est appuyée...
  if(SDL_GetModState() & KMOD_SHIFT)
      Rectifier_coordonnees_a_45_degres(Debut_X,Debut_Y,&Fin_X,&Fin_Y);

  Pinceau_Forme=Pinceau_Forme_avant_operation;

  Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
  Effacer_ligne_Preview (Debut_X,Debut_Y,Fin_X,Fin_Y);
  Afficher_pinceau      (Debut_X,Debut_Y,Couleur,0);
  Tracer_ligne_Definitif(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


/////////////////////////////////////////////////////////// OPERATION_K_LIGNE


void K_Ligne_12_0(void)
// Opération   : OPERATION_K_LIGNE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Oui
{
  byte Couleur;

  Initialiser_debut_operation();
  Backup();
  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Pinceau_Forme_avant_operation=Pinceau_Forme;
  Pinceau_Forme=FORME_PINCEAU_POINT;

  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Mouse_K | 0x80);
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  // Taille de pile 6 : phase d'appui, non interruptible
}


void K_Ligne_12_6(void)
// Opération   : OPERATION_K_LIGNE
// Click Souris: 1 ou 2 | 0
// Taille_Pile : 6      | 7
//
// Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Pinceau_X!=Fin_X) || (Pinceau_Y!=Fin_Y))
  {
    Effacer_curseur();
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Operation_POP(&Couleur);

    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);

    Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);
    Pixel_figure_Preview (Debut_X,Debut_Y,Couleur);
    Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

    Operation_PUSH(Couleur);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void K_Ligne_0_6(void)
// Opération   : OPERATION_K_LIGNE
// Click Souris: 0
// Taille_Pile : 6
//
// Souris effacée: Oui
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;
  short Direction;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);
  Operation_POP(&Direction);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
  Effacer_ligne_Preview (Debut_X,Debut_Y,Fin_X,Fin_Y);

  Pinceau_Forme=Pinceau_Forme_avant_operation;
  if (Direction & 0x80)
  {
    Afficher_pinceau(Debut_X,Debut_Y,Couleur,0);
    Direction=(Direction & 0x7F);
  }
  Tracer_ligne_Definitif(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);
  Pinceau_Forme=FORME_PINCEAU_POINT;

  Operation_PUSH(Direction);
  Operation_PUSH(Direction); // Valeur bidon servant de nouvel état de pile
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  // Taille de pile 7 : phase de "repos", interruptible (comme Elliot Ness :))
}


void K_Ligne_12_7(void)
// Opération   : OPERATION_K_LIGNE
// Click Souris: 1 ou 2
// Taille_Pile : 7
//
// Souris effacée: Oui
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;
  short Direction;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);
  Operation_POP(&Direction);
  Operation_POP(&Direction);

  if (Direction==Mouse_K)
  {
    Operation_PUSH(Direction);
    Operation_PUSH(Couleur);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Fin_X);
    Operation_PUSH(Fin_Y);
    // Taille de pile 6 : phase d'appui, non interruptible
  }
  else
  {
    // La série de ligne est terminée, il faut donc effacer la dernière
    // preview de ligne
    Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
    Effacer_ligne_Preview (Debut_X,Debut_Y,Fin_X,Fin_Y);

    Afficher_curseur();
    Attendre_fin_de_click();
    Effacer_curseur();
    Pinceau_Forme=Pinceau_Forme_avant_operation;

    if ( (Config.Coords_rel) && (Menu_visible) )
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }
  }
}


// ---------------------------------------------------------- OPERATION_LOUPE


void Loupe_12_0(void)

// Opération   : 4      (Choix d'une Loupe)
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui

{
  Attendre_fin_de_click();

  // On passe en mode loupe
  Loupe_Mode=1;

  // La fonction d'affichage dans la partie image est désormais un affichage
  // spécial loupe.
  Pixel_Preview=Pixel_Preview_Loupe;

  // On calcule l'origine de la loupe
  Loupe_Decalage_X=Mouse_X-(Loupe_Largeur>>1);
  Loupe_Decalage_Y=Mouse_Y-(Loupe_Hauteur>>1);

  // Calcul du coin haut_gauche de la fenêtre devant être zoomée DANS L'ECRAN
  if (Loupe_Decalage_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
    Loupe_Decalage_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
  if (Loupe_Decalage_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
    Loupe_Decalage_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;

  // Calcul des coordonnées absolues de ce coin DANS L'IMAGE
  Loupe_Decalage_X+=Principal_Decalage_X;
  Loupe_Decalage_Y+=Principal_Decalage_Y;

  if (Loupe_Decalage_X<0)
    Loupe_Decalage_X=0;
  if (Loupe_Decalage_Y<0)
    Loupe_Decalage_Y=0;

  // On calcule les bornes visibles dans l'écran
  Recadrer_ecran_par_rapport_au_zoom();
  Calculer_limites();
  Afficher_ecran();

  // Repositionner le curseur en fonction des coordonnées visibles
  Calculer_coordonnees_pinceau();

  // On fait de notre mieux pour restaurer l'ancienne opération:
  Demarrer_pile_operation(Operation_avant_interruption);
}

/////////////////////////////////////////////////// OPERATION_RECTANGLE_?????

void Rectangle_12_0(void)
// Opération   : OPERATION_RECTANGLE_VIDE / OPERATION_RECTANGLE_PLEIN
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  Initialiser_debut_operation();

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("\035:   1   \022:   1",0);
  // On laisse une trace du curseur à l'écran
  Afficher_curseur();

  if (Mouse_K==A_GAUCHE)
  {
    Shade_Table=Shade_Table_gauche;
    Operation_PUSH(Fore_color);
  }
  else
  {
    Shade_Table=Shade_Table_droite;
    Operation_PUSH(Back_color);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Rectangle_12_5(void)
// Opération   : OPERATION_RECTANGLE_VIDE / OPERATION_RECTANGLE_PLEIN
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;
  short Ancien_X;
  short Ancien_Y;
  char  Chaine[5];

  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y))
  {
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);

    if ((Config.Coords_rel) && (Menu_visible))
    {
      Num2str(((Debut_X<Pinceau_X)?Pinceau_X-Debut_X:Debut_X-Pinceau_X)+1,Chaine,4);
      Print_dans_menu(Chaine,2);
      Num2str(((Debut_Y<Pinceau_Y)?Pinceau_Y-Debut_Y:Debut_Y-Pinceau_Y)+1,Chaine,4);
      Print_dans_menu(Chaine,11);
    }
    else
      Print_coordonnees();

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Rectangle_vide_0_5(void)
// Opération   : OPERATION_RECTANGLE_VIDE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
{
  short Ancien_Pinceau_X;
  short Ancien_Pinceau_Y;
  short Couleur;

  // On mémorise la position courante du pinceau:

  Ancien_Pinceau_X=Pinceau_X;
  Ancien_Pinceau_Y=Pinceau_Y;

  // On lit l'ancienne position du pinceau:

  Operation_POP(&Pinceau_Y);
  Operation_POP(&Pinceau_X);
  Operation_POP(&Pinceau_Y);
  Operation_POP(&Pinceau_X);

  // On va devoir effacer l'ancien curseur qu'on a laissé trainer:
  Effacer_curseur();

  // On lit la couleur du rectangle:
  Operation_POP(&Couleur);

  // On fait un petit backup de l'image:
  Backup();

  // Et on trace le rectangle:
  Tracer_rectangle_vide(Pinceau_X,Pinceau_Y,Ancien_Pinceau_X,Ancien_Pinceau_Y,Couleur);

  // Enfin, on rétablit la position du pinceau:
  Pinceau_X=Ancien_Pinceau_X;
  Pinceau_Y=Ancien_Pinceau_Y;

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:       Y:",0);
  Print_coordonnees();
}


void Rectangle_plein_0_5(void)
// Opération   : OPERATION_RECTANGLE_PLEIN
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
{
  short Ancien_Pinceau_X;
  short Ancien_Pinceau_Y;
  short Couleur;

  // On mémorise la position courante du pinceau:

  Ancien_Pinceau_X=Pinceau_X;
  Ancien_Pinceau_Y=Pinceau_Y;

  // On lit l'ancienne position du pinceau:

  Operation_POP(&Pinceau_Y);
  Operation_POP(&Pinceau_X);
  Operation_POP(&Pinceau_Y);
  Operation_POP(&Pinceau_X);

  // On va devoir effacer l'ancien curseur qu'on a laissé trainer:
  Effacer_curseur();

  // On lit la couleur du rectangle:
  Operation_POP(&Couleur);

  // On fait un petit backup de l'image:
  Backup();

  // Et on trace le rectangle:
  Tracer_rectangle_plein(Pinceau_X,Pinceau_Y,Ancien_Pinceau_X,Ancien_Pinceau_Y,Couleur);

  // Enfin, on rétablit la position du pinceau:
  Pinceau_X=Ancien_Pinceau_X;
  Pinceau_Y=Ancien_Pinceau_Y;

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:",0);
    Print_coordonnees();
  }
}


////////////////////////////////////////////////////// OPERATION_CERCLE_?????


void Cercle_12_0(void)
//
// Opération   : OPERATION_CERCLE_VIDE / OPERATION_CERCLE_PLEIN
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();

  Pinceau_Forme_avant_operation=Pinceau_Forme;
  Pinceau_Forme=FORME_PINCEAU_POINT;

  if (Mouse_K==A_GAUCHE)
  {
    Shade_Table=Shade_Table_gauche;
    Operation_PUSH(Fore_color);
  }
  else
  {
    Shade_Table=Shade_Table_droite;
    Operation_PUSH(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("Radius:   0    ",0);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Cercle_12_5(void)
//
// Opération   : OPERATION_CERCLE_VIDE / OPERATION_CERCLE_PLEIN
// Click Souris: 1 ou 2
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon;
  char  Chaine[5];

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  if ( (Tangente_X!=Pinceau_X) || (Tangente_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    if ((Config.Coords_rel) && (Menu_visible))
    {
      Num2str(Distance(Centre_X,Centre_Y,Pinceau_X,Pinceau_Y),Chaine,4);
      Print_dans_menu(Chaine,7);
    }
    else
      Print_coordonnees();

    Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                  ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
    Rayon=sqrt(Cercle_Limite);
    Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

    Cercle_Limite=((Pinceau_X-Centre_X)*(Pinceau_X-Centre_X))+
                  ((Pinceau_Y-Centre_Y)*(Pinceau_Y-Centre_Y));
    Rayon=sqrt(Cercle_Limite);
    Tracer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon,Couleur);

    Afficher_curseur();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(Centre_X);
  Operation_PUSH(Centre_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Cercle_vide_0_5(void)
//
// Opération   : OPERATION_CERCLE_VIDE
// Click Souris: 0
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
  Rayon=sqrt(Cercle_Limite);
  Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

  Pinceau_Forme=Pinceau_Forme_avant_operation;

  Tracer_cercle_vide_Definitif(Centre_X,Centre_Y,Rayon,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:",0);
    Print_coordonnees();
  }
}


void Cercle_plein_0_5(void)
//
// Opération   : OPERATION_CERCLE_PLEIN
// Click Souris: 0
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
  Rayon=sqrt(Cercle_Limite);
  Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

  Pinceau_Forme=Pinceau_Forme_avant_operation;

  Tracer_cercle_plein(Centre_X,Centre_Y,Rayon,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:",0);
    Print_coordonnees();
  }
}


///////////////////////////////////////////////////// OPERATION_ELLIPSE_?????


void Ellipse_12_0(void)
//
// Opération   : OPERATION_ELLIPSE_VIDE / OPERATION_ELLIPSE_PLEINE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  Initialiser_debut_operation();
  Backup();

  Pinceau_Forme_avant_operation=Pinceau_Forme;
  Pinceau_Forme=FORME_PINCEAU_POINT;

  if (Mouse_K==A_GAUCHE)
  {
    Shade_Table=Shade_Table_gauche;
    Operation_PUSH(Fore_color);
  }
  else
  {
    Shade_Table=Shade_Table_droite;
    Operation_PUSH(Back_color);
  }

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Ellipse_12_5(void)
//
// Opération   : OPERATION_ELLIPSE_VIDE / OPERATION_ELLIPSE_PLEINE
// Click Souris: 1 ou 2
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon_horizontal;
  short Rayon_vertical;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  if ( (Tangente_X!=Pinceau_X) || (Tangente_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    Aff_coords_rel_ou_abs(Centre_X,Centre_Y);

    Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                           :Centre_X-Tangente_X;
    Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                           :Centre_Y-Tangente_Y;
    Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

    Rayon_horizontal=(Pinceau_X>Centre_X)?Pinceau_X-Centre_X
                                         :Centre_X-Pinceau_X;
    Rayon_vertical  =(Pinceau_Y>Centre_Y)?Pinceau_Y-Centre_Y
                                         :Centre_Y-Pinceau_Y;
    Tracer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);

    Afficher_curseur();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(Centre_X);
  Operation_PUSH(Centre_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Ellipse_vide_0_5(void)
//
// Opération   : OPERATION_ELLIPSE_VIDE
// Click Souris: 0
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon_horizontal;
  short Rayon_vertical;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                         :Centre_X-Tangente_X;
  Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                         :Centre_Y-Tangente_Y;
  Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

  Pinceau_Forme=Pinceau_Forme_avant_operation;

  Tracer_ellipse_vide_Definitif(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


void Ellipse_pleine_0_5(void)
//
// Opération   : OPERATION_ELLIPSE_PLEINE
// Click Souris: 0
// Taille_Pile : 5 (Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon_horizontal;
  short Rayon_vertical;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                         :Centre_X-Tangente_X;
  Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                         :Centre_Y-Tangente_Y;
  Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

  Pinceau_Forme=Pinceau_Forme_avant_operation;

  Tracer_ellipse_pleine(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


////////////////////////////////////////////////////////////// OPERATION_FILL


void Fill_1_0(void)
//
// Opération   : OPERATION_FILL
// Click Souris: 1
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Fill, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  // On ne s'occupe pas de faire un Backup: c'est "Remplir" qui s'en charge.
  Shade_Table=Shade_Table_gauche;
  Remplir(Fore_color);
  Attendre_fin_de_click();
}


void Fill_2_0(void)
//
// Opération   : OPERATION_FILL
// Click Souris: 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Fill, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  // On ne s'occupe pas de faire un Backup: c'est "Remplir" qui s'en charge.
  Shade_Table=Shade_Table_droite;
  Remplir(Back_color);
  Attendre_fin_de_click();
}


///////////////////////////////////////////////////////// OPERATION_REMPLACER


void Remplacer_1_0(void)
//
// Opération   : OPERATION_REMPLACER
// Click Souris: 1
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Replace, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  Backup();
//  Shade_Table=Shade_Table_gauche;
  Remplacer(Fore_color);
  Attendre_fin_de_click();
}


void Remplacer_2_0(void)
//
// Opération   : OPERATION_REMPLACER
// Click Souris: 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  // Pas besoin d'initialiser le début d'opération car le Smear n'affecte pas
  // le Replace, et on se fout de savoir si on est dans la partie gauche ou
  // droite de la loupe.
  Backup();
//  Shade_Table=Shade_Table_droite;
  Remplacer(Back_color);
  Attendre_fin_de_click();
}


/////////////////////////////////////////////////////////// OPERATION_PIPETTE


void Pipette_12_0(void)
//
// Opération   : OPERATION_PIPETTE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Initialiser_debut_operation();

  if (Mouse_K==A_GAUCHE)
  {
    Encadrer_couleur_menu(CM_Noir);
    Fore_color=Pipette_Couleur;
    Recadrer_palette();
    Afficher_foreback();
    Encadrer_couleur_menu(CM_Blanc);
  }
  else
  {
    Back_color=Pipette_Couleur;
    Afficher_foreback();
  }
  Operation_PUSH(Mouse_K);
}


void Pipette_1_1(void)
//
// Opération   : OPERATION_PIPETTE
// Click Souris: 1
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  char Chaine[4];

  if ( (Pinceau_X>=0) && (Pinceau_Y>=0)
    && (Pinceau_X<Principal_Largeur_image)
    && (Pinceau_Y<Principal_Hauteur_image) )
    Pipette_Couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
  else
    Pipette_Couleur=0;

  if ( (Pipette_X!=Pinceau_X)
    || (Pipette_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    Pipette_X=Pinceau_X;
    Pipette_Y=Pinceau_Y;

    if (Pipette_Couleur!=Fore_color)
    {
      Encadrer_couleur_menu(CM_Noir);
      Fore_color=Pipette_Couleur;
      Recadrer_palette();
      Afficher_foreback();
      Encadrer_couleur_menu(CM_Blanc);
    }

    if (Menu_visible)
    {
      Print_coordonnees();
      Num2str(Pipette_Couleur,Chaine,3);
      Print_dans_menu(Chaine,20);
      Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
    }
    Afficher_curseur();
  }
}


void Pipette_2_1(void)
//
// Opération   : OPERATION_PIPETTE
// Click Souris: 2
// Taille_Pile : 1
//
// Souris effacée: Non
//
{
  char Chaine[4];

  if ( (Pinceau_X>=0) && (Pinceau_Y>=0)
    && (Pinceau_X<Principal_Largeur_image)
    && (Pinceau_Y<Principal_Hauteur_image) )
    Pipette_Couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
  else
    Pipette_Couleur=0;

  if ( (Pipette_X!=Pinceau_X)
    || (Pipette_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    Pipette_X=Pinceau_X;
    Pipette_Y=Pinceau_Y;

    if (Pipette_Couleur!=Back_color)
    {
      Back_color=Pipette_Couleur;
      Afficher_foreback();
    }

    if (Menu_visible)
    {
      Print_coordonnees();
      Num2str(Pipette_Couleur,Chaine,3);
      Print_dans_menu(Chaine,20);
      Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
    }
    Afficher_curseur();
  }
}



void Pipette_0_1(void)
//
// Opération   : OPERATION_PIPETTE
// Click Souris: 0
// Taille_Pile : 1
//
// Souris effacée: Oui
//
{
  short Clic;

  Operation_POP(&Clic);
  if (Clic==A_GAUCHE)
  {
    Encadrer_couleur_menu(CM_Noir);
    Fore_color=Pipette_Couleur;
    Recadrer_palette();
    Afficher_foreback();
    Encadrer_couleur_menu(CM_Blanc);
  }
  else
  {
    Back_color=Pipette_Couleur;
    Afficher_foreback();
  }
  Desenclencher_bouton(BOUTON_PIPETTE);
}


/////////////////////////////////////////////////// OPERATION_COURBE_4_POINTS


void Courbe_Tracer_croix(short Pos_X, short Pos_Y)
{
  short Debut_X,Fin_X;
  short Debut_Y,Fin_Y;
  short i,Temp;
  //byte  Temp2;

  if (Pos_X>=Limite_Gauche+3)
    Debut_X=0;
  else
    Debut_X=3-(Pos_X-Limite_Gauche);

  if (Pos_Y>=Limite_Haut+3)
    Debut_Y=0;
  else
    Debut_Y=3-(Pos_Y-Limite_Haut);

  if (Pos_X<=Limite_visible_Droite-3)
    Fin_X=6;
  else
    Fin_X=3+(Limite_visible_Droite-Pos_X);

  if (Pos_Y<=Limite_visible_Bas-3)
    Fin_Y=6;
  else
    Fin_Y=3+(Limite_visible_Bas-Pos_Y);

  for (i=Debut_X; i<=Fin_X; i++)
  {
    Temp=Pos_X+i-3;
    Pixel_Preview(Temp,Pos_Y,~Lit_pixel(Temp -Principal_Decalage_X,
                                        Pos_Y-Principal_Decalage_Y));
  }
  for (i=Debut_Y; i<=Fin_Y; i++)
  {
    Temp=Pos_Y+i-3;
    Pixel_Preview(Pos_X,Temp,~Lit_pixel(Pos_X-Principal_Decalage_X,
                                        Temp -Principal_Decalage_Y));
  }
}


void Courbe_34_points_1_0(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_gauche;

  Cacher_pinceau=1;

  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Fore_color);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Fore_color);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Courbe_34_points_2_0(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_droite;

  Cacher_pinceau=1;

  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Back_color);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Back_color);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Courbe_34_points_1_5(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 1
//  Taille_Pile : 5
//
//  Souris effacée: Non
//
{
  short X1,X2,Y1,Y2;

  Operation_POP(&Y2);
  Operation_POP(&X2);
  Operation_POP(&Y1);
  Operation_POP(&X1);

  if ( (Y2!=Pinceau_Y) || (X2!=Pinceau_X) )
  {
    Effacer_curseur();
    Aff_coords_rel_ou_abs(X1,Y1);

    Effacer_ligne_Preview(X1,Y1,X2,Y2);
    Pixel_figure_Preview (X1,Y1,Fore_color);
    Tracer_ligne_Preview (X1,Y1,Pinceau_X,Pinceau_Y,Fore_color);

    Afficher_curseur();
  }

  Operation_PUSH(X1);
  Operation_PUSH(Y1);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Courbe_34_points_2_5(void)
//
//  Opération   : OPERATION_COURBE_?_POINTS
//  Click Souris: 2
//  Taille_Pile : 5
//
//  Souris effacée: Non
//
{
  short X1,X2,Y1,Y2;

  Operation_POP(&Y2);
  Operation_POP(&X2);
  Operation_POP(&Y1);
  Operation_POP(&X1);

  if ( (Y2!=Pinceau_Y) || (X2!=Pinceau_X) )
  {
    Effacer_curseur();
    Aff_coords_rel_ou_abs(X1,Y1);

    Effacer_ligne_Preview(X1,Y1,X2,Y2);
    Pixel_figure_Preview (X1,Y1,Back_color);
    Tracer_ligne_Preview (X1,Y1,Pinceau_X,Pinceau_Y,Back_color);

    Afficher_curseur();
  }

  Operation_PUSH(X1);
  Operation_PUSH(Y1);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


byte Cacher_curseur_avant_courbe;

void Courbe_4_points_0_5(void)
//
//  Opération   : OPERATION_COURBE_4_POINTS
//  Click Souris: 0
//  Taille_Pile : 5
//
//  Souris effacée: Oui
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Tiers_X,Tiers_Y;
  short Couleur;

  Operation_POP(&Y4);
  Operation_POP(&X4);
  Operation_POP(&Y1);
  Operation_POP(&X1);
  Operation_POP(&Couleur);

  Tiers_X=Round_div(abs(X4-X1),3);
  Tiers_Y=Round_div(abs(Y4-Y1),3);

  if (X1<X4)
  {
    X2=X1+Tiers_X;
    X3=X4-Tiers_X;
  }
  else
  {
    X3=X4+Tiers_X;
    X2=X1-Tiers_X;
  }

  if (Y1<Y4)
  {
    Y2=Y1+Tiers_Y;
    Y3=Y4-Tiers_Y;
  }
  else
  {
    Y3=Y4+Tiers_Y;
    Y2=Y1-Tiers_Y;
  }

  Effacer_ligne_Preview(X1,Y1,X4,Y4);
  Tracer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

  // On trace les petites croix montrant les 2 points intermédiares
  Courbe_Tracer_croix(X2,Y2);
  Courbe_Tracer_croix(X3,Y3);

  Cacher_curseur_avant_courbe=Cacher_curseur;
  Cacher_curseur=0;

  Operation_PUSH(Couleur);
  Operation_PUSH(X1);
  Operation_PUSH(Y1);
  Operation_PUSH(X2);
  Operation_PUSH(Y2);
  Operation_PUSH(X3);
  Operation_PUSH(Y3);
  Operation_PUSH(X4);
  Operation_PUSH(Y4);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


void Courbe_4_points_1_9(void)
//
//  Opération   : OPERATION_COURBE_4_POINTS
//  Click Souris: 1
//  Taille_Pile : 9
//
//  Souris effacée: Non
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Couleur;
  byte  C_est_X2;

  Operation_POP(&Y4);
  Operation_POP(&X4);
  Operation_POP(&Y3);
  Operation_POP(&X3);
  Operation_POP(&Y2);
  Operation_POP(&X2);
  Operation_POP(&Y1);
  Operation_POP(&X1);
  Operation_POP(&Couleur);

  C_est_X2=(Distance(Pinceau_X,Pinceau_Y,X2,Y2) < Distance(Pinceau_X,Pinceau_Y,X3,Y3));

  if ( (   C_est_X2  && ( (Pinceau_X!=X2) || (Pinceau_Y!=Y2) ) )
    || ( (!C_est_X2) && ( (Pinceau_X!=X3) || (Pinceau_Y!=Y3) ) ) )
  {
    Effacer_curseur();
    Print_coordonnees();

    Courbe_Tracer_croix(X2,Y2);
    Courbe_Tracer_croix(X3,Y3);

    Effacer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

    if (C_est_X2)
    {
      X2=Pinceau_X;
      Y2=Pinceau_Y;
    }
    else
    {
      X3=Pinceau_X;
      Y3=Pinceau_Y;
    }

    Tracer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

    Courbe_Tracer_croix(X2,Y2);
    Courbe_Tracer_croix(X3,Y3);

    Afficher_curseur();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(X1);
  Operation_PUSH(Y1);
  Operation_PUSH(X2);
  Operation_PUSH(Y2);
  Operation_PUSH(X3);
  Operation_PUSH(Y3);
  Operation_PUSH(X4);
  Operation_PUSH(Y4);
}


void Courbe_4_points_2_9(void)
//
//  Opération   : OPERATION_COURBE_4_POINTS
//  Click Souris: 2
//  Taille_Pile : 9
//
//  Souris effacée: Oui
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Couleur;

  Operation_POP(&Y4);
  Operation_POP(&X4);
  Operation_POP(&Y3);
  Operation_POP(&X3);
  Operation_POP(&Y2);
  Operation_POP(&X2);
  Operation_POP(&Y1);
  Operation_POP(&X1);
  Operation_POP(&Couleur);

  Courbe_Tracer_croix(X2,Y2);
  Courbe_Tracer_croix(X3,Y3);

  Cacher_pinceau=0;
  Cacher_curseur=Cacher_curseur_avant_courbe;

  Effacer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
  Tracer_courbe_Definitif(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

  Attendre_fin_de_click();
}


/////////////////////////////////////////////////// OPERATION_COURBE_3_POINTS


void Calculer_courbe_3_points(short X1, short Y1, short X4, short Y4,
                              short * X2, short * Y2, short * X3, short * Y3)
{
  float CX,CY; // Centre de (X1,Y1) et (X4,Y4)
  float BX,BY; // Intersect. des dtes ((X1,Y1),(X2,Y2)) et ((X3,Y3),(X4,Y4))

  CX=(float)(X1+X4)/2.0;           // P1*--_               Légende:
  CY=(float)(Y1+Y4)/2.0;           //   ·   \·· P2         -_\|/ : courbe
                                   //   ·    \ ·*·         * : point important
  BX=CX+((8.0/3.0)*(Pinceau_X-CX));//   ·     |   ··       · : pointillÚ
  BY=CY+((8.0/3.0)*(Pinceau_Y-CY));//   ·     |P    ··  B
                                   // C *·····*·········*  P=Pos. du pinceau
  *X2=Round((BX+X1)/2.0);          //   ·     |     ··     C=milieu de [P1,P4]
  *Y2=Round((BY+Y1)/2.0);          //   ·     |   ··       B=point tel que
                                   //   ·    / ·*·         C->B=(8/3) * C->P
  *X3=Round((BX+X4)/2.0);          //   ·  _/·· P3         P2=milieu de [P1,B]
  *Y3=Round((BY+Y4)/2.0);          // P4*--                P3=milieu de [P4,B]
}


void Courbe_3_points_0_5(void)
//
//  Opération   : OPERATION_COURBE_3_POINTS
//  Click Souris: 0
//  Taille_Pile : 5
//
//  Souris effacée: Oui
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Couleur;

  Operation_POP(&Y4);
  Operation_POP(&X4);
  Operation_POP(&Y1);
  Operation_POP(&X1);
  Operation_POP(&Couleur);

  Calculer_courbe_3_points(X1,Y1,X4,Y4,&X2,&Y2,&X3,&Y3);

  Effacer_ligne_Preview(X1,Y1,X4,Y4);
  Tracer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

  if ( (Config.Coords_rel) && (Menu_visible) )
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(X1);
  Operation_PUSH(Y1);
  Operation_PUSH(X2);
  Operation_PUSH(Y2);
  Operation_PUSH(X3);
  Operation_PUSH(Y3);
  Operation_PUSH(X4);
  Operation_PUSH(Y4);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Courbe_3_points_0_11(void)
//
//  Opération   : OPERATION_COURBE_3_POINTS
//  Click Souris: 0
//  Taille_Pile : 11
//
//  Souris effacée: Non
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Old_X,Old_Y;
  short Couleur;

  Operation_POP(&Old_Y);
  Operation_POP(&Old_X);

  if ( (Pinceau_X!=Old_X) || (Pinceau_Y!=Old_Y) )
  {
    Operation_POP(&Y4);
    Operation_POP(&X4);
    Operation_POP(&Y3);
    Operation_POP(&X3);
    Operation_POP(&Y2);
    Operation_POP(&X2);
    Operation_POP(&Y1);
    Operation_POP(&X1);
    Operation_POP(&Couleur);

    Effacer_curseur();
    Print_coordonnees();

    Effacer_courbe_Preview(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
    Calculer_courbe_3_points(X1,Y1,X4,Y4,&X2,&Y2,&X3,&Y3);
    Tracer_courbe_Preview (X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
    Afficher_curseur();

    Operation_PUSH(Couleur);
    Operation_PUSH(X1);
    Operation_PUSH(Y1);
    Operation_PUSH(X2);
    Operation_PUSH(Y2);
    Operation_PUSH(X3);
    Operation_PUSH(Y3);
    Operation_PUSH(X4);
    Operation_PUSH(Y4);
  }
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Courbe_3_points_12_11(void)
//
//  Opération   : OPERATION_COURBE_3_POINTS
//  Click Souris: 1 ou 2
//  Taille_Pile : 11
//
//  Souris effacée: Oui
//
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  short Old_X,Old_Y;
  short Couleur;

  Operation_POP(&Old_Y);
  Operation_POP(&Old_X);
  Operation_POP(&Y4);
  Operation_POP(&X4);
  Operation_POP(&Y3);
  Operation_POP(&X3);
  Operation_POP(&Y2);
  Operation_POP(&X2);
  Operation_POP(&Y1);
  Operation_POP(&X1);
  Operation_POP(&Couleur);

  Cacher_pinceau=0;

  Effacer_courbe_Preview (X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
  Calculer_courbe_3_points(X1,Y1,X4,Y4,&X2,&Y2,&X3,&Y3);
  Tracer_courbe_Definitif(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);

  Attendre_fin_de_click();
}


///////////////////////////////////////////////////////////// OPERATION_SPRAY


void Spray_1_0(void)
//
//  Opération   : OPERATION_SPRAY
//  Click Souris: 1
//  Taille_Pile : 0
//
//  Souris effacée: Non
//
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_gauche;
  Aerographe(A_GAUCHE);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(A_GAUCHE);
}

void Spray_2_0(void)
//
//  Opération   : OPERATION_SPRAY
//  Click Souris: 2
//  Taille_Pile : 0
//
//  Souris effacée: Non
//
{
  Initialiser_debut_operation();
  Backup();
  Shade_Table=Shade_Table_droite;
  Aerographe(A_DROITE);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(A_DROITE);
}

void Spray_12_3(void)
//
//  Opération   : OPERATION_SPRAY
//  Click Souris: 1 ou 2
//  Taille_Pile : 1
//
//  Souris effacée: Non
//
{
  short Bouton_clicke;
  short Ancien_X,Ancien_Y;

  Operation_POP(&Bouton_clicke);
  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ( (Menu_visible) && ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y)) )
  {
    Effacer_curseur();
    Print_coordonnees();
    Afficher_curseur();
  }

  Aerographe(Bouton_clicke);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Bouton_clicke);
}

void Spray_0_3(void)
//
//  Opération   : OPERATION_SPRAY
//  Click Souris: 0
//  Taille_Pile : 3
//
//  Souris effacée: Non
//
{
  Operation_Taille_pile-=3;
}


////////////////////////////////////////////////////////// OPERATION_POLYGONE


void Polygone_12_0(void)
// Opération   : OPERATION_POLYGONE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte Couleur;

  Initialiser_debut_operation();
  Backup();
  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Pinceau_Forme_avant_operation=Pinceau_Forme;
  Pinceau_Forme=FORME_PINCEAU_POINT;

  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Mouse_K | 0x80);
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  // Taille de pile 8 : phase d'appui, non interruptible
}



void Polygone_12_9(void)
// Opération   : OPERATION_POLYGONE
// Click Souris: 1 ou 2
// Taille_Pile : 9
//
// Souris effacée: Oui
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;
  short Direction;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);
  Operation_POP(&Direction);
  Operation_POP(&Direction);

  if (Direction==Mouse_K)
  {
    Operation_PUSH(Direction);
    Operation_PUSH(Couleur);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Fin_X);
    Operation_PUSH(Fin_Y);
    // Taille de pile 8 : phase d'appui, non interruptible
  }
  else
  {
    //   La série de ligne est terminée, il faut donc effacer la dernière
    // preview de ligne et relier le dernier point avec le premier
    Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
    Effacer_ligne_Preview (Debut_X,Debut_Y,Fin_X,Fin_Y);
    Operation_POP(&Fin_Y);
    Operation_POP(&Fin_X);
    Pinceau_Forme=Pinceau_Forme_avant_operation;
    // Le pied aurait été de ne pas repasser sur le 1er point de la 1ère ligne
    // mais c'est pas possible :(
    Tracer_ligne_Definitif(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
    Pinceau_Forme=FORME_PINCEAU_POINT;

    Afficher_curseur();
    Attendre_fin_de_click();
    Effacer_curseur();

    if ( (Config.Coords_rel) && (Menu_visible) )
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }

    Pinceau_Forme=Pinceau_Forme_avant_operation;
  }
}


////////////////////////////////////////////////////////// OPERATION_POLYFILL

short * Polyfill_Table_de_points;
int Polyfill_Nombre_de_points;

void Polyfill_12_0(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte Couleur;

  Initialiser_debut_operation();
  Backup();
  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Cacher_pinceau=1;

  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  Polyfill_Table_de_points=(short *) malloc((Config.Nb_max_de_vertex_par_polygon<<1)*sizeof(short));
  Polyfill_Table_de_points[0]=Pinceau_X;
  Polyfill_Table_de_points[1]=Pinceau_Y;
  Polyfill_Nombre_de_points=1;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_Preview_xor(Pinceau_X,Pinceau_Y,0);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Mouse_K | 0x80);
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  // Taille de pile 8 : phase d'appui, non interruptible
}


void Polyfill_0_8(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 0
// Taille_Pile : 8
//
// Souris effacée: Oui
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;
  short Direction;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);
  Operation_POP(&Direction);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);

  if (Direction & 0x80)
    Direction=(Direction & 0x7F);

  Operation_PUSH(Direction); // Valeur bidon servant de nouvel état de pile
  Operation_PUSH(Direction);
  Operation_PUSH(Couleur);

  Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);

  if (Polyfill_Nombre_de_points<Config.Nb_max_de_vertex_par_polygon)
  {
    Polyfill_Table_de_points[Polyfill_Nombre_de_points<<1]    =Pinceau_X;
    Polyfill_Table_de_points[(Polyfill_Nombre_de_points<<1)+1]=Pinceau_Y;
    Polyfill_Nombre_de_points++;

    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
  }
  else
  {
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
  }
  // Taille de pile 9 : phase de "repos", interruptible (comme Elliot Ness :))
}


void Polyfill_12_8(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2 | 0
// Taille_Pile : 8      | 9
//
// Souris effacée: Non
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Pinceau_X!=Fin_X) || (Pinceau_Y!=Fin_Y))
  {
    Effacer_curseur();
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);

    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);

    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Polyfill_12_9(void)
// Opération   : OPERATION_POLYFILL
// Click Souris: 1 ou 2
// Taille_Pile : 9
//
// Souris effacée: Oui
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Couleur;
  short Direction;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Couleur);
  Operation_POP(&Direction);
  Operation_POP(&Direction);

  if (Direction==Mouse_K)
  {
    Operation_PUSH(Direction);
    Operation_PUSH(Couleur);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Fin_X);
    Operation_PUSH(Fin_Y);
    // Taille de pile 8 : phase d'appui, non interruptible
  }
  else
  {
    //   La série de lignes est terminée, il faut donc effacer la dernière
    // preview de ligne et relier le dernier point avec le premier
    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
    Operation_POP(&Fin_Y);
    Operation_POP(&Fin_X);
    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);

    Afficher_curseur();
    Attendre_fin_de_click();
    Effacer_curseur();

    Afficher_ecran();
    Polyfill(Polyfill_Nombre_de_points,Polyfill_Table_de_points,Couleur);
    free(Polyfill_Table_de_points);

    if ( (Config.Coords_rel) && (Menu_visible) )
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }

    Cacher_pinceau=0;
  }
}


////////////////////////////////////////////////////////// OPERATION_POLYFORM


void Polyform_12_0(void)
//  Opération   : OPERATION_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  short Couleur;

  Initialiser_debut_operation();
  Backup();
  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;

  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  // On place un premier pinceau en (Pinceau_X,Pinceau_Y):
  Afficher_pinceau(Pinceau_X,Pinceau_Y,Couleur,0);
  // Et on affiche un pixel de preview en (Pinceau_X,Pinceau_Y):
  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

  Operation_PUSH(Pinceau_X); // X Initial
  Operation_PUSH(Pinceau_Y); // X Initial
  Operation_PUSH(Couleur);   // Couleur de remplissage
  Operation_PUSH(Pinceau_X); // Debut X
  Operation_PUSH(Pinceau_Y); // Debut Y
  Operation_PUSH(Pinceau_X); // Fin X
  Operation_PUSH(Pinceau_Y); // Fin Y
  Operation_PUSH(Mouse_K);   // Clic
}


void Polyform_12_8(void)
//  Opération   : OPERATION_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short Clic;
  short Fin_Y;
  short Fin_X;
  short Debut_Y;
  short Debut_X;
  short Couleur;
  short Initial_Y;
  short Initial_X;

  Operation_POP(&Clic);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if (Clic==Mouse_K)
  {
    // L'utilisateur clic toujours avec le bon bouton de souris

    if ((Debut_X!=Pinceau_X) || (Debut_Y!=Pinceau_Y))
    {
      // Il existe un segment définit par (Debut_X,Debut_Y)-(Pinceau_X,Pinceau_Y)

      Effacer_curseur();
      Print_coordonnees();

      Operation_POP(&Couleur);

      // On efface la preview du segment validé:
      Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);

      // On l'affiche de façon définitive:
      Tracer_ligne_Definitif(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

      // Et on affiche un pixel de preview en (Pinceau_X,Pinceau_Y):
      Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

      Operation_PUSH(Couleur);

      Afficher_curseur();
    }

    Operation_PUSH(Pinceau_X); // Nouveau Debut_X
    Operation_PUSH(Pinceau_Y); // Nouveau Debut_Y
    Operation_PUSH(Pinceau_X); // Nouveau Fin_X
    Operation_PUSH(Pinceau_Y); // Nouveau Fin_Y
    Operation_PUSH(Clic);
  }
  else
  {
    // L'utilisateur souhaite arrêter l'opération et refermer le polygone

    Operation_POP(&Couleur);
    Operation_POP(&Initial_Y);
    Operation_POP(&Initial_X);

    Attendre_fin_de_click();
    Effacer_curseur();
    Print_coordonnees();

    // On efface la preview du segment annulé:
    Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);

    // On affiche de façon définitive le bouclage du polygone:
    Tracer_ligne_Definitif(Debut_X,Debut_Y,Initial_X,Initial_Y,Couleur);

    Afficher_curseur();
  }
}


void Polyform_0_8(void)
//  Opération   : OPERATION_POLYFORM
//  Click Souris: 0
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short Clic;
  short Fin_Y;
  short Fin_X;
  short Debut_Y;
  short Debut_X;
  short Couleur;

  Operation_POP(&Clic);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Fin_X!=Pinceau_X) || (Fin_Y!=Pinceau_Y))
  {
    Effacer_curseur();
    Print_coordonnees();

    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Operation_POP(&Couleur);

    // On met à jour l'affichage de la preview du prochain segment:
    Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);
    Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

    Operation_PUSH(Couleur);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);

    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Clic);
}


/////////////////////////////////////////////////// OPERATION_FILLED_POLYFORM


void Filled_polyform_12_0(void)
//  Opération   : OPERATION_FILLED_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
{
  short Couleur;

  Initialiser_debut_operation();

  // Cette opération étant également utilisée pour le lasso, on ne fait pas de
  // backup si on prend une brosse au lasso avec le bouton gauche.
  if ((Operation_en_cours==OPERATION_FILLED_POLYFORM) || (Mouse_K==A_DROITE))
    Backup();

  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Cacher_pinceau=1;

  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  Polyfill_Table_de_points=(short *) malloc((Config.Nb_max_de_vertex_par_polygon<<1)*sizeof(short));
  Polyfill_Table_de_points[0]=Pinceau_X;
  Polyfill_Table_de_points[1]=Pinceau_Y;
  Polyfill_Nombre_de_points=1;

  // On place temporairement le début de la ligne qui ne s'afficherait pas sinon
  Pixel_figure_Preview_xor(Pinceau_X,Pinceau_Y,0);

  Operation_PUSH(Pinceau_X); // X Initial
  Operation_PUSH(Pinceau_Y); // X Initial
  Operation_PUSH(Couleur);   // Couleur de remplissage
  Operation_PUSH(Pinceau_X); // Debut X
  Operation_PUSH(Pinceau_Y); // Debut Y
  Operation_PUSH(Pinceau_X); // Fin X
  Operation_PUSH(Pinceau_Y); // Fin Y
  Operation_PUSH(Mouse_K);   // Clic
}


void Filled_polyform_12_8(void)
//  Opération   : OPERATION_FILLED_POLYFORM
//  Click Souris: 1 ou 2
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short Clic;
  short Fin_Y;
  short Fin_X;
  short Debut_Y;
  short Debut_X;
  short Couleur;
  short Initial_Y;
  short Initial_X;

  Operation_POP(&Clic);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if (Clic==Mouse_K)
  {
    // L'utilisateur clique toujours avec le bon bouton de souris

    if (((Debut_X!=Pinceau_X) || (Debut_Y!=Pinceau_Y)) &&
        (Polyfill_Nombre_de_points<Config.Nb_max_de_vertex_par_polygon))
    {
      // Il existe un nouveau segment défini par
      // (Debut_X,Debut_Y)-(Pinceau_X,Pinceau_Y)

      Effacer_curseur();
      Print_coordonnees();

      // On le place à l'écran
      Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
      Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);

      // On peut le rajouter au polygone

      Polyfill_Table_de_points[Polyfill_Nombre_de_points<<1]    =Pinceau_X;
      Polyfill_Table_de_points[(Polyfill_Nombre_de_points<<1)+1]=Pinceau_Y;
      Polyfill_Nombre_de_points++;

      Operation_PUSH(Pinceau_X); // Nouveau Debut_X
      Operation_PUSH(Pinceau_Y); // Nouveau Debut_Y
      Operation_PUSH(Pinceau_X); // Nouveau Fin_X
      Operation_PUSH(Pinceau_Y); // Nouveau Fin_Y
      Operation_PUSH(Clic);

      Afficher_curseur();
    }
    else
    {
      if (Polyfill_Nombre_de_points==Config.Nb_max_de_vertex_par_polygon)
      {
        // Le curseur bouge alors qu'on ne peut plus stocker de segments ?

        if ((Fin_X!=Pinceau_X) || (Fin_Y!=Pinceau_Y))
        {
          Effacer_curseur();
          Print_coordonnees();

          // On le place à l'écran
          Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
          Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);
          Afficher_curseur();
        }

        // On remet les mêmes valeurs (comme si on n'avait pas cliqué):
        Operation_PUSH(Debut_X);
        Operation_PUSH(Debut_Y);
        Operation_PUSH(Pinceau_X);
        Operation_PUSH(Pinceau_Y);
        Operation_PUSH(Clic);
      }
      else
      {
        Operation_PUSH(Pinceau_X); // Nouveau Debut_X
        Operation_PUSH(Pinceau_Y); // Nouveau Debut_Y
        Operation_PUSH(Pinceau_X); // Nouveau Fin_X
        Operation_PUSH(Pinceau_Y); // Nouveau Fin_Y
        Operation_PUSH(Clic);
      }
    }
  }
  else
  {
    // L'utilisateur souhaite arrêter l'opération et refermer le polygone

    Operation_POP(&Couleur);
    Operation_POP(&Initial_Y);
    Operation_POP(&Initial_X);

    Attendre_fin_de_click();
    Effacer_curseur();
    Print_coordonnees();

    // Pas besoin d'effacer la ligne (Debut_X,Debut_Y)-(Fin_X,Fin_Y)
    // puisque on les effaces toutes d'un coup.

    Afficher_ecran();
    Polyfill(Polyfill_Nombre_de_points,Polyfill_Table_de_points,Couleur);
    free(Polyfill_Table_de_points);

    Cacher_pinceau=0;

    Afficher_curseur();
  }
}


void Filled_polyform_0_8(void)
//  Opération   : OPERATION_FILLED_POLYFORM
//  Click Souris: 0
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short Clic;
  short Fin_Y;
  short Fin_X;
  short Debut_Y;
  short Debut_X;

  Operation_POP(&Clic);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Fin_X!=Pinceau_X) || (Fin_Y!=Pinceau_Y))
  {
    Effacer_curseur();
    Print_coordonnees();

    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);

    // On met à jour l'affichage de la preview du prochain segment:
    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
    Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);

    Afficher_curseur();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Clic);
}


////////////////////////////////////////////////////// OPERATION_PRISE_BROSSE


void Brosse_12_0(void)
//
// Opération   : OPERATION_PRISE_BROSSE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  if (Mouse_K==A_DROITE) // Besoin d'effacer la brosse après ?
  {
    Operation_PUSH(1);
    // Puisque la zone où on prend la brosse sera effacée, on fait un backup
    Backup();
  }
  else
    Operation_PUSH(0);

  // On laisse une trace du curseur pour que l'utilisateur puisse visualiser
  // où demarre sa brosse:
  Afficher_curseur();

  Operation_PUSH(Pinceau_X); // Début X
  Operation_PUSH(Pinceau_Y); // Début Y
  Operation_PUSH(Pinceau_X); // Dernière position X
  Operation_PUSH(Pinceau_Y); // Dernière position Y

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("\035:   1   \022:   1",0);
}


void Brosse_12_5(void)
//
// Opération   : OPERATION_PRISE_BROSSE
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  Chaine[5];
  short Debut_X;
  short Debut_Y;
  short Ancien_X;
  short Ancien_Y;
  short Largeur;
  short Hauteur;

  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ( (Menu_visible) && ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y)) )
  {
    if (Config.Coords_rel)
    {
      Operation_POP(&Debut_Y);
      Operation_POP(&Debut_X);
      Operation_PUSH(Debut_X);
      Operation_PUSH(Debut_Y);

      Largeur=((Debut_X<Pinceau_X)?Pinceau_X-Debut_X:Debut_X-Pinceau_X)+1;
      Hauteur=((Debut_Y<Pinceau_Y)?Pinceau_Y-Debut_Y:Debut_Y-Pinceau_Y)+1;

      Num2str(Largeur,Chaine,4);
      Print_dans_menu(Chaine,2);
      Num2str(Hauteur,Chaine,4);
      Print_dans_menu(Chaine,11);
    }
    else
      Print_coordonnees();
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Brosse_0_5(void)
//
// Opération   : OPERATION_PRISE_BROSSE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Oui
//
{
  short Debut_X;
  short Debut_Y;
  short Ancien_Pinceau_X;
  short Ancien_Pinceau_Y;
  short Effacement;

  // Comme on a demandé l'effacement du curseur, il n'y a plus de croix en
  // (Pinceau_X,Pinceau_Y). C'est une bonne chose.

  Operation_Taille_pile-=2;
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Effacement);

  // On efface l'ancienne croix:
  Ancien_Pinceau_X=Pinceau_X;
  Ancien_Pinceau_Y=Pinceau_Y;
  Pinceau_X=Debut_X;
  Pinceau_Y=Debut_Y;
  Effacer_curseur(); // Maintenant, il n'y a plus de croix à l'écran.

  Pinceau_X=Ancien_Pinceau_X;
  Pinceau_Y=Ancien_Pinceau_Y;

  // Prise de la brosse
  if ((Snap_Mode) && (Config.Adjust_brush_pick))
  {
    if (Pinceau_X<Debut_X)
    {
      Ancien_Pinceau_X=Debut_X;
      Debut_X=Pinceau_X;
    }
    if (Pinceau_Y<Debut_Y)
    {
      Ancien_Pinceau_Y=Debut_Y;
      Debut_Y=Pinceau_Y;
    }
    if (Ancien_Pinceau_X!=Debut_X)
      Ancien_Pinceau_X--;
    if (Ancien_Pinceau_Y!=Debut_Y)
      Ancien_Pinceau_Y--;
  }
  Capturer_brosse(Debut_X,Debut_Y,Ancien_Pinceau_X,Ancien_Pinceau_Y,Effacement);
  if ((Snap_Mode) && (Config.Adjust_brush_pick))
  {
    Brosse_Decalage_X=(Brosse_Decalage_X/Snap_Largeur)*Snap_Largeur;
    Brosse_Decalage_Y=(Brosse_Decalage_Y/Snap_Hauteur)*Snap_Hauteur;
  }

  // Simuler l'appui du bouton "Dessin"

  // Comme l'enclenchement du bouton efface le curseur, il faut l'afficher au
  // préalable:
  Afficher_curseur();
  // !!! Efface la croix puis affiche le viseur !!!
  Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE); // Désenclenche au passage le bouton brosse
  if (Config.Auto_discontinuous)
  {
    // On se place en mode Dessin discontinu à la main
    while (Operation_en_cours!=OPERATION_DESSIN_DISCONTINU)
      Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
  }
  // Maintenant, il faut réeffacer le curseur parce qu'il sera raffiché en fin
  // d'appel à cette action:
  Effacer_curseur();

  // On passe en brosse couleur:
  Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:       Y:",0);
  Print_coordonnees();
}


//////////////////////////////////////////////////////// OPERATION_POLYBROSSE


void Polybrosse_12_8(void)
//  Opération   : OPERATION_POLYBROSSE
//  Click Souris: 1 ou 2
//  Taille_Pile : 8
//
//  Souris effacée: Non
{
  short Clic;
  short Fin_Y;
  short Fin_X;
  short Debut_Y;
  short Debut_X;
  short Couleur;
  short Initial_Y;
  short Initial_X;

  Operation_POP(&Clic);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if (Clic==Mouse_K)
  {
    // L'utilisateur clique toujours avec le bon bouton de souris

    if (((Debut_X!=Pinceau_X) || (Debut_Y!=Pinceau_Y)) &&
        (Polyfill_Nombre_de_points<Config.Nb_max_de_vertex_par_polygon))
    {
      // Il existe un nouveau segment défini par
      // (Debut_X,Debut_Y)-(Pinceau_X,Pinceau_Y)

      Effacer_curseur();
      Print_coordonnees();

      // On le place à l'écran
      Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
      Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);

      // On peut le rajouter au polygone

      Polyfill_Table_de_points[Polyfill_Nombre_de_points<<1]    =Pinceau_X;
      Polyfill_Table_de_points[(Polyfill_Nombre_de_points<<1)+1]=Pinceau_Y;
      Polyfill_Nombre_de_points++;

      Operation_PUSH(Pinceau_X); // Nouveau Debut_X
      Operation_PUSH(Pinceau_Y); // Nouveau Debut_Y
      Operation_PUSH(Pinceau_X); // Nouveau Fin_X
      Operation_PUSH(Pinceau_Y); // Nouveau Fin_Y
      Operation_PUSH(Clic);

      Afficher_curseur();
    }
    else
    {
      if (Polyfill_Nombre_de_points==Config.Nb_max_de_vertex_par_polygon)
      {
        // Le curseur bouge alors qu'on ne peut plus stocker de segments ?

        if ((Fin_X!=Pinceau_X) || (Fin_Y!=Pinceau_Y))
        {
          Effacer_curseur();
          Print_coordonnees();

          // On le place à l'écran
          Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
          Tracer_ligne_Preview_xor(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,0);
          Afficher_curseur();
        }

        // On remet les mêmes valeurs (comme si on n'avait pas cliqué):
        Operation_PUSH(Debut_X);
        Operation_PUSH(Debut_Y);
        Operation_PUSH(Pinceau_X);
        Operation_PUSH(Pinceau_Y);
        Operation_PUSH(Clic);
      }
      else
      {
        Operation_PUSH(Pinceau_X); // Nouveau Debut_X
        Operation_PUSH(Pinceau_Y); // Nouveau Debut_Y
        Operation_PUSH(Pinceau_X); // Nouveau Fin_X
        Operation_PUSH(Pinceau_Y); // Nouveau Fin_Y
        Operation_PUSH(Clic);
      }
    }
  }
  else
  {
    // L'utilisateur souhaite arrêter l'opération et refermer le polygone

    Operation_POP(&Couleur);
    Operation_POP(&Initial_Y);
    Operation_POP(&Initial_X);

    Attendre_fin_de_click();
    Effacer_curseur();
    Print_coordonnees();

    // Pas besoin d'effacer la ligne (Debut_X,Debut_Y)-(Fin_X,Fin_Y)
    // puisqu'on les efface toutes d'un coup.

    Capturer_brosse_au_lasso(Polyfill_Nombre_de_points,Polyfill_Table_de_points,Clic==A_DROITE);
    free(Polyfill_Table_de_points);

    // On raffiche l'écran pour effacer les traits en xor et pour raffraichir
    // l'écran si on a découpé une partie de l'image en prenant la brosse.
    Afficher_ecran();

    Cacher_pinceau=0;

    if ((Snap_Mode) && (Config.Adjust_brush_pick))
    {
      Brosse_Decalage_X=(Brosse_Decalage_X/Snap_Largeur)*Snap_Largeur;
      Brosse_Decalage_Y=(Brosse_Decalage_Y/Snap_Hauteur)*Snap_Hauteur;
    }

    // Simuler l'appui du bouton "Dessin"

    // Comme l'enclenchement du bouton efface le curseur, il faut l'afficher au
    // préalable:
    Afficher_curseur();
    // !!! Efface la croix puis affiche le viseur !!!
    Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE); // Désenclenche au passage le bouton brosse
    if (Config.Auto_discontinuous)
    {
      // On se place en mode Dessin discontinu à la main
      while (Operation_en_cours!=OPERATION_DESSIN_DISCONTINU)
        Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
    }
    // Maintenant, il faut réeffacer le curseur parce qu'il sera raffiché en fin
    // d'appel à cette action:
    Effacer_curseur();

    // On passe en brosse couleur:
    Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);

    Afficher_curseur();
  }
}


///////////////////////////////////////////////////// OPERATION_ETIRER_BROSSE


void Etirer_brosse_12_0(void)
//
// Opération   : OPERATION_ETIRER_BROSSE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  if (Mouse_K==A_GAUCHE)
  {
    // On laisse une trace du curseur pour que l'utilisateur puisse visualiser
    // où demarre sa brosse:
    Afficher_curseur();

    Operation_PUSH(Pinceau_X); // Dernier calcul X
    Operation_PUSH(Pinceau_Y); // Dernier calcul Y
    Operation_PUSH(Pinceau_X); // Début X
    Operation_PUSH(Pinceau_Y); // Début Y
    Operation_PUSH(Pinceau_X); // Dernière position X
    Operation_PUSH(Pinceau_Y); // Dernière position Y
    Operation_PUSH(1); // Etat précédent

    if ((Config.Coords_rel) && (Menu_visible))
      Print_dans_menu("\035:   1   \022:   1",0);
  }
  else
  {
    Attendre_fin_de_click();
    Demarrer_pile_operation(Operation_avant_interruption);
  }
}



void Etirer_brosse_1_7(void)
//
// Opération   : OPERATION_ETIRER_BROSSE
// Click Souris: 1
// Taille_Pile : 7
//
// Souris effacée: Non
//
{
  char  Chaine[5];
  short Debut_X;
  short Debut_Y;
  short Ancien_X;
  short Ancien_Y;
  short Largeur;
  short Hauteur;
  short Etat_prec;
  short dX,dY,X,Y;

  Operation_POP(&Etat_prec);
  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ( (Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y) || (Etat_prec!=2) )
  {
    Effacer_curseur();
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);

    if (Menu_visible)
    {
      if (Config.Coords_rel)
      {
        Largeur=((Debut_X<Pinceau_X)?Pinceau_X-Debut_X:Debut_X-Pinceau_X)+1;
        Hauteur=((Debut_Y<Pinceau_Y)?Pinceau_Y-Debut_Y:Debut_Y-Pinceau_Y)+1;

        if (Snap_Mode && Config.Adjust_brush_pick)
        {
          if (Largeur>1) Largeur--;
          if (Hauteur>1) Hauteur--;
        }

        Num2str(Largeur,Chaine,4);
        Print_dans_menu(Chaine,2);
        Num2str(Hauteur,Chaine,4);
        Print_dans_menu(Chaine,11);
      }
      else
        Print_coordonnees();
    }

    Afficher_ecran();

    X=Pinceau_X;
    Y=Pinceau_Y;
    if (Snap_Mode && Config.Adjust_brush_pick)
    {
      dX=Pinceau_X-Debut_X;
      dY=Pinceau_Y-Debut_Y;
      if (dX<0) X++; else {if (dX>0) X--;}
      if (dY<0) Y++; else {if (dY>0) Y--;}
      Etirer_brosse_preview(Debut_X,Debut_Y,X,Y);
    }
    else
      Etirer_brosse_preview(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y);

    Ancien_X=Pinceau_X;
    Ancien_Y=Pinceau_Y;
    Pinceau_X=Debut_X;
    Pinceau_Y=Debut_Y;
    Afficher_curseur();
    Pinceau_X=Ancien_X;
    Pinceau_Y=Ancien_Y;
    Afficher_curseur();

    Operation_Taille_pile-=2;
    Operation_PUSH(X);
    Operation_PUSH(Y);

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(2);
}



void Etirer_brosse_0_7(void)
//
// Opération   : OPERATION_ETIRER_BROSSE
// Click Souris: 0
// Taille_Pile : 7
//
// Souris effacée: Non
//
{
  char  Chaine[5];
  short Debut_X;
  short Debut_Y;
  short Ancien_X;
  short Ancien_Y;
  short Largeur=0;
  short Hauteur=0;
  byte  Changement_de_taille;
  short Etat_prec;

  Operation_POP(&Etat_prec);
  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);

  if ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y) || (Etat_prec!=3))
  {
    if (Menu_visible)
    {
      if (Config.Coords_rel)
      {
        Largeur=((Debut_X<Pinceau_X)?Pinceau_X-Debut_X:Debut_X-Pinceau_X)+1;
        Hauteur=((Debut_Y<Pinceau_Y)?Pinceau_Y-Debut_Y:Debut_Y-Pinceau_Y)+1;

        Num2str(Largeur,Chaine,4);
        Print_dans_menu(Chaine,2);
        Num2str(Hauteur,Chaine,4);
        Print_dans_menu(Chaine,11);
      }
      else
        Print_coordonnees();
    }
  }

  // Utilise Touche_ANSI au lieu de Touche, car Get_input() met ce dernier
  // à zero si une operation est en cours (Operation_Taille_pile!=0)
  if (Touche_ANSI)
  {
    Changement_de_taille=1;
    switch (Touche_ANSI)
    {
      case 'd': // Double
        Largeur=Debut_X+(Brosse_Largeur<<1)-1;
        Hauteur=Debut_Y+(Brosse_Hauteur<<1)-1;
        break;
      case 'x': // Double X
        Largeur=Debut_X+(Brosse_Largeur<<1)-1;
        Hauteur=Debut_Y+Brosse_Hauteur-1;
        break;
      case 'y': // Double Y
        Largeur=Debut_X+Brosse_Largeur-1;
        Hauteur=Debut_Y+(Brosse_Hauteur<<1)-1;
        break;
      case 'h': // Moitié
        Largeur=(Brosse_Largeur>1)?Debut_X+(Brosse_Largeur>>1)-1:1;
        Hauteur=(Brosse_Hauteur>1)?Debut_Y+(Brosse_Hauteur>>1)-1:1;
        break;
      case 'X': // Moitié X
        Largeur=(Brosse_Largeur>1)?Debut_X+(Brosse_Largeur>>1)-1:1;
        Hauteur=Debut_Y+Brosse_Hauteur-1;
        break;
      case 'Y': // Moitié Y
        Largeur=Debut_X+Brosse_Largeur-1;
        Hauteur=(Brosse_Hauteur>1)?Debut_Y+(Brosse_Hauteur>>1)-1:1;
        break;
      case 'n': // Normal
        Largeur=Debut_X+Brosse_Largeur-1;
        Hauteur=Debut_Y+Brosse_Hauteur-1;
        break;
      default :
        Changement_de_taille=0;
    }
  }
  else
    Changement_de_taille=0;

  if (Changement_de_taille)
  {
    // On efface la preview de la brosse (et la croix)
    Afficher_ecran();

    Ancien_X=Pinceau_X;
    Ancien_Y=Pinceau_Y;
    Pinceau_X=Debut_X;
    Pinceau_Y=Debut_Y;
    Afficher_curseur();
    Pinceau_X=Ancien_X;
    Pinceau_Y=Ancien_Y;

    Etirer_brosse_preview(Debut_X,Debut_Y,Largeur,Hauteur);
    Afficher_curseur();

    Operation_Taille_pile-=2;
    Operation_PUSH(Largeur);
    Operation_PUSH(Hauteur);
  }

  Operation_PUSH(Debut_X);
  Operation_PUSH(Debut_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(3);
}


void Etirer_brosse_2_7(void)
//
// Opération   : OPERATION_ETIRER_BROSSE
// Click Souris: 2
// Taille_Pile : 7
//
// Souris effacée: Oui
//
{
  short Calcul_X;
  short Calcul_Y;
  short Debut_X;
  short Debut_Y;


  Operation_Taille_pile-=3;
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Calcul_Y);
  Operation_POP(&Calcul_X);

  // On efface la preview de la brosse (et la croix)
  Afficher_ecran();

  // Et enfin on stocke pour de bon la nouvelle brosse étirée
  Etirer_brosse(Debut_X,Debut_Y,Calcul_X,Calcul_Y);

  // Simuler l'appui du bouton "Dessin"

  // Comme l'enclenchement du bouton efface le curseur, il faut l'afficher au
  // préalable:
  Afficher_curseur();
  // !!! Efface la croix puis affiche le viseur !!!
  Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE); // Désenclenche au passage le bouton brosse
  if (Config.Auto_discontinuous)
  {
    // On se place en mode Dessin discontinu à la main
    while (Operation_en_cours!=OPERATION_DESSIN_DISCONTINU)
      Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
  }
  // Maintenant, il faut réeffacer le curseur parce qu'il sera raffiché en fin
  // d'appel à cette action:
  Effacer_curseur();

  // On passe en brosse couleur:
  Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:       Y:",0);
  Print_coordonnees();

  // Inutile de de faire un Attendre_fin_de_click car c'est fait dans Enclencher_bouton
}


//////////////////////////////////////////////////// OPERATION_TOURNER_BROSSE


void Tourner_brosse_12_0(void)
//
// Opération   : OPERATION_TOURNER_BROSSE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  if (Mouse_K==A_GAUCHE)
  {
    Brosse_Centre_rotation_X=Pinceau_X+(Brosse_Largeur>>1)-Brosse_Largeur;
    Brosse_Centre_rotation_Y=Pinceau_Y;
    Brosse_Centre_rotation_defini=1;
    Operation_PUSH(Pinceau_X); // Dernière position calculée X
    Operation_PUSH(Pinceau_Y); // Dernière position calculée Y
    Operation_PUSH(Pinceau_X); // Dernière position X
    Operation_PUSH(Pinceau_Y); // Dernière position Y
    Operation_PUSH(1); // Etat précédent

    if ((Config.Coords_rel) && (Menu_visible))
      Print_dans_menu("Angle:   0°    ",0);
  }
  else
  {
    Attendre_fin_de_click();
    Demarrer_pile_operation(Operation_avant_interruption);
  }
}



void Tourner_brosse_1_5(void)
//
// Opération   : OPERATION_TOURNER_BROSSE
// Click Souris: 1
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  Chaine[4];
  short Ancien_X;
  short Ancien_Y;
  short Etat_prec;
  float Angle;
  int dX,dY;

  Operation_POP(&Etat_prec);
  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ( (Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y) || (Etat_prec!=2) )
  {
    if ( (Brosse_Centre_rotation_X==Pinceau_X)
      && (Brosse_Centre_rotation_Y==Pinceau_Y) )
      Angle=0.0;
    else
    {
      dX=Pinceau_X-Brosse_Centre_rotation_X;
      dY=Pinceau_Y-Brosse_Centre_rotation_Y;
      Angle=acos(((float)dX)/sqrt((dX*dX)+(dY*dY)));
      if (dY>0) Angle=M_2PI-Angle;
    }

    if (Menu_visible)
    {
      if (Config.Coords_rel)
      {
        Num2str((int)(Angle*180.0/M_PI),Chaine,3);
        Print_dans_menu(Chaine,7);
      }
      else
        Print_coordonnees();
    }

    Afficher_ecran();
    Tourner_brosse_preview(Angle);
    Afficher_curseur();

    Operation_Taille_pile-=2;
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(2);
}



void Tourner_brosse_0_5(void)
//
// Opération   : OPERATION_TOURNER_BROSSE
// Click Souris: 0
// Taille_Pile : 5
//
// Souris effacée: Non
//
{
  char  Chaine[4];
  short Ancien_X;
  short Ancien_Y;
  short Calcul_X=0;
  short Calcul_Y=0;
  byte  Changement_angle;
  short Etat_prec;
  float Angle=0.0;
  int dX,dY;

  Operation_POP(&Etat_prec);
  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y) || (Etat_prec!=3))
  {
    if ( (Brosse_Centre_rotation_X==Pinceau_X)
      && (Brosse_Centre_rotation_Y==Pinceau_Y) )
      Angle=0.0;
    else
    {
      dX=Pinceau_X-Brosse_Centre_rotation_X;
      dY=Pinceau_Y-Brosse_Centre_rotation_Y;
      Angle=acos(((float)dX)/sqrt((dX*dX)+(dY*dY)));
      if (dY>0) Angle=M_2PI-Angle;
    }

    if (Menu_visible)
    {
      if (Config.Coords_rel)
      {
        Num2str(Round(Angle*180.0/M_PI),Chaine,3);
        Print_dans_menu(Chaine,7);
      }
      else
        Print_coordonnees();
    }
  }

  // Utilise Touche_ANSI au lieu de Touche, car Get_input() met ce dernier
  // à zero si une operation est en cours (Operation_Taille_pile!=0)
  if (Touche_ANSI)
  {
    Changement_angle=1;
    Calcul_X=Brosse_Centre_rotation_X;
    Calcul_Y=Brosse_Centre_rotation_Y;
    switch (Touche_ANSI)
    {
      case '6': Angle=     0.0 ; Calcul_X++;             break;
      case '9': Angle=M_PI*0.25; Calcul_X++; Calcul_Y--; break;
      case '8': Angle=M_PI*0.5 ;             Calcul_Y--; break;
      case '7': Angle=M_PI*0.75; Calcul_X--; Calcul_Y--; break;
      case '4': Angle=M_PI     ; Calcul_X--;             break;
      case '1': Angle=M_PI*1.25; Calcul_X--; Calcul_Y++; break;
      case '2': Angle=M_PI*1.5 ;             Calcul_Y++; break;
      case '3': Angle=M_PI*1.75; Calcul_X++; Calcul_Y++; break;
      default :
        Changement_angle=0;
    }
  }
  else
    Changement_angle=0;

  if (Changement_angle)
  {
    // On efface la preview de la brosse
    Afficher_ecran();
    Tourner_brosse_preview(Angle);
    Afficher_curseur();

    Operation_Taille_pile-=2;
    Operation_PUSH(Calcul_X);
    Operation_PUSH(Calcul_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(3);
}


void Tourner_brosse_2_5(void)
//
// Opération   : OPERATION_TOURNER_BROSSE
// Click Souris: 2
// Taille_Pile : 5
//
// Souris effacée: Oui
//
{
  short Calcul_X;
  short Calcul_Y;
  int dX,dY;
  float Angle;


  // On efface la preview de la brosse
  Afficher_ecran();

  Operation_Taille_pile-=3;
  Operation_POP(&Calcul_Y);
  Operation_POP(&Calcul_X);

  // Calcul de l'angle par rapport à la dernière position calculée
  if ( (Brosse_Centre_rotation_X==Calcul_X)
    && (Brosse_Centre_rotation_Y==Calcul_Y) )
    Angle=0.0;
  else
  {
    dX=Calcul_X-Brosse_Centre_rotation_X;
    dY=Calcul_Y-Brosse_Centre_rotation_Y;
    Angle=acos(((float)dX)/sqrt((dX*dX)+(dY*dY)));
    if (dY>0) Angle=M_2PI-Angle;
  }

  // Et enfin on stocke pour de bon la nouvelle brosse étirée
  Tourner_brosse(Angle);

  // Simuler l'appui du bouton "Dessin"

  // Comme l'enclenchement du bouton efface le curseur, il faut l'afficher au
  // préalable:
  Afficher_curseur();
  // !!! Efface le curseur de l'opération puis affiche le viseur !!!
  Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE); // Désenclenche au passage le bouton brosse
  if (Config.Auto_discontinuous)
  {
    // On se place en mode Dessin discontinu à la main
    while (Operation_en_cours!=OPERATION_DESSIN_DISCONTINU)
      Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
  }
  // Maintenant, il faut réeffacer le curseur parce qu'il sera raffiché en fin
  // d'appel à cette action:
  Effacer_curseur();

  // On passe en brosse couleur:
  Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:       Y:",0);
  Print_coordonnees();

  // Inutile de de faire un Attendre_fin_de_click car c'est fait dans Enclencher_bouton
}


//////////////////////////////////////////////////////////// OPERATION_SCROLL


byte Cacher_curseur_avant_scroll;

void Scroll_12_0(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 1 ou 2
//  Taille_Pile : 0
//
//  Souris effacée: Oui
//
{
  Initialiser_debut_operation();
  Backup();
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Cacher_curseur_avant_scroll=Cacher_curseur;
  Cacher_curseur=1;
  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);
}


void Scroll_12_4(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 1 ou 2
//  Taille_Pile : 4
//
//  Souris effacée: Non
//
{
  short Centre_X;
  short Centre_Y;
  short Pos_X;
  short Pos_Y;
  short Decalage_X;
  short Decalage_Y;
  //char  Chaine[5];

  Operation_POP(&Pos_Y);
  Operation_POP(&Pos_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);

  if ( (Pinceau_X!=Pos_X) || (Pinceau_Y!=Pos_Y) )
  {
    // L'utilisateur a bougé, il faut scroller l'image

    if (Pinceau_X>=Centre_X)
      Decalage_X=(Pinceau_X-Centre_X)%Principal_Largeur_image;
    else
      Decalage_X=Principal_Largeur_image-((Centre_X-Pinceau_X)%Principal_Largeur_image);

    if (Pinceau_Y>=Centre_Y)
      Decalage_Y=(Pinceau_Y-Centre_Y)%Principal_Hauteur_image;
    else
      Decalage_Y=Principal_Hauteur_image-((Centre_Y-Pinceau_Y)%Principal_Hauteur_image);

    Aff_coords_rel_ou_abs(Centre_X,Centre_Y);

    Scroll_picture(Decalage_X,Decalage_Y);

    Afficher_ecran();
  }

  Operation_PUSH(Centre_X);
  Operation_PUSH(Centre_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Scroll_0_4(void)
//
//  Opération   : OPERATION_SCROLL
//  Click Souris: 0
//  Taille_Pile : 4
//
//  Souris effacée: Oui
//
{
  Operation_Taille_pile-=4;
  Cacher_curseur=Cacher_curseur_avant_scroll;
  if ((Config.Coords_rel) && (Menu_visible))
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


//////////////////////////////////////////////////// OPERATION_CERCLE_DEGRADE


void Cercle_degrade_12_0(void)
//
// Opération   : OPERATION_CERCLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte Couleur;

  Initialiser_debut_operation();
  Backup();

  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  Cacher_pinceau_avant_operation=Cacher_pinceau;
  Cacher_pinceau=1;

  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("Radius:   0    ",0);

  Operation_PUSH(Mouse_K);
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Cercle_degrade_12_6(void)
//
// Opération   : OPERATION_CERCLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 6 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon;
  char  Chaine[5];

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  if ( (Tangente_X!=Pinceau_X) || (Tangente_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    if ((Config.Coords_rel) && (Menu_visible))
    {
      Num2str(Distance(Centre_X,Centre_Y,Pinceau_X,Pinceau_Y),Chaine,4);
      Print_dans_menu(Chaine,7);
    }
    else
      Print_coordonnees();

    Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                  ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
    Rayon=sqrt(Cercle_Limite);
    Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

    Cercle_Limite=((Pinceau_X-Centre_X)*(Pinceau_X-Centre_X))+
                  ((Pinceau_Y-Centre_Y)*(Pinceau_Y-Centre_Y));
    Rayon=sqrt(Cercle_Limite);
    Tracer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon,Couleur);

    Afficher_curseur();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(Centre_X);
  Operation_PUSH(Centre_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Cercle_degrade_0_6(void)
//
// Opération   : OPERATION_CERCLE_DEGRADE
// Click Souris: 0
// Taille_Pile : 6 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Click;
  short Rayon;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);

  Operation_POP(&Couleur);
  Operation_POP(&Click);

  if (Click==A_GAUCHE)
  {
    Operation_PUSH(Click);
    Operation_PUSH(Couleur);

    Operation_PUSH(Centre_X);
    Operation_PUSH(Centre_Y);
    Operation_PUSH(Tangente_X);
    Operation_PUSH(Tangente_Y);

    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);

    // On change la forme du curseur
    Forme_curseur=FORME_CURSEUR_CIBLE_XOR;

    // On affiche une croix XOR au centre du cercle
    Courbe_Tracer_croix(Centre_X,Centre_Y);

    if (Menu_visible)
    {
      if (Config.Coords_rel)
        Print_dans_menu("X:        Y:",0);
      else
        Print_dans_menu("X:       Y:             ",0);
      Aff_coords_rel_ou_abs(Centre_X,Centre_Y);
    }
  }
  else
  {
    Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                  ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
    Rayon=sqrt(Cercle_Limite);
    Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

    Cacher_pinceau=Cacher_pinceau_avant_operation;
    Forme_curseur=FORME_CURSEUR_CIBLE;

    Tracer_cercle_plein(Centre_X,Centre_Y,Rayon,Back_color);

    if ((Config.Coords_rel) && (Menu_visible))
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }
  }
}


void Cercle_degrade_12_8(void)
//
// Opération   : OPERATION_CERCLE_DEGRADE
// Click Souris: 0
// Taille_Pile : 8 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente, Ancien_X, Ancien_Y)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Ancien_Mouse_K;

  short Rayon;

  Operation_Taille_pile-=2;   // On fait sauter les 2 derniers élts de la pile
  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);
  Operation_POP(&Ancien_Mouse_K);

  // On efface la croix XOR au centre du cercle
  Courbe_Tracer_croix(Centre_X,Centre_Y);

  Cercle_Limite=((Tangente_X-Centre_X)*(Tangente_X-Centre_X))+
                ((Tangente_Y-Centre_Y)*(Tangente_Y-Centre_Y));
  Rayon=sqrt(Cercle_Limite);
  Effacer_cercle_vide_Preview(Centre_X,Centre_Y,Rayon);

  Cacher_pinceau=Cacher_pinceau_avant_operation;
  Forme_curseur=FORME_CURSEUR_CIBLE;

  if (Mouse_K==Ancien_Mouse_K)
    Tracer_cercle_degrade(Centre_X,Centre_Y,Rayon,Pinceau_X,Pinceau_Y);

  Attendre_fin_de_click();

  if ((Config.Coords_rel) && (Menu_visible))
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}


void Cercle_ou_ellipse_degrade_0_8(void)
//
// Opération   : OPERATION_{CERCLE|ELLIPSE}_DEGRADE
// Click Souris: 0
// Taille_Pile : 8
//
// Souris effacée: Non
//
{
  short Debut_X;
  short Debut_Y;
  short Tangente_X;
  short Tangente_Y;
  short Ancien_X;
  short Ancien_Y;

  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y))
  {
    Operation_POP(&Tangente_Y);
    Operation_POP(&Tangente_X);
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Tangente_X);
    Operation_PUSH(Tangente_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


////////////////////////////////////////////////// OPERATION_ELLIPSE_DEGRADEE


void Ellipse_degradee_12_0(void)
//
// Opération   : OPERATION_ELLIPSE_DEGRADEE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui
{
  byte Couleur;

  Initialiser_debut_operation();
  Backup();

  Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;
  Couleur=(Mouse_K==A_GAUCHE)?Fore_color:Back_color;

  Cacher_pinceau_avant_operation=Cacher_pinceau;
  Cacher_pinceau=1;

  Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("X:±   0   Y:±   0",0);

  Operation_PUSH(Mouse_K);
  Operation_PUSH(Couleur);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Ellipse_degradee_12_6(void)
//
// Opération   : OPERATION_ELLIPSE_DEGRADEE
// Click Souris: 1 ou 2
// Taille_Pile : 6 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Non
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon_horizontal;
  short Rayon_vertical;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);

  if ( (Tangente_X!=Pinceau_X) || (Tangente_Y!=Pinceau_Y) )
  {
    Effacer_curseur();
    Aff_coords_rel_ou_abs(Centre_X,Centre_Y);

    Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                           :Centre_X-Tangente_X;
    Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                           :Centre_Y-Tangente_Y;
    Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

    Rayon_horizontal=(Pinceau_X>Centre_X)?Pinceau_X-Centre_X
                                         :Centre_X-Pinceau_X;
    Rayon_vertical  =(Pinceau_Y>Centre_Y)?Pinceau_Y-Centre_Y
                                         :Centre_Y-Pinceau_Y;
    Tracer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);

    Afficher_curseur();
  }

  Operation_PUSH(Couleur);
  Operation_PUSH(Centre_X);
  Operation_PUSH(Centre_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Ellipse_degradee_0_6(void)
//
// Opération   : OPERATION_ELLIPSE_DEGRADEE
// Click Souris: 0
// Taille_Pile : 6 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Click;
  //short Rayon;
  short Rayon_horizontal;
  short Rayon_vertical;

  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);

  Operation_POP(&Couleur);
  Operation_POP(&Click);

  if (Click==A_GAUCHE)
  {
    Operation_PUSH(Click);
    Operation_PUSH(Couleur);

    Operation_PUSH(Centre_X);
    Operation_PUSH(Centre_Y);
    Operation_PUSH(Tangente_X);
    Operation_PUSH(Tangente_Y);

    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);

    // On change la forme du curseur
    Forme_curseur=FORME_CURSEUR_CIBLE_XOR;

    // On affiche une croix XOR au centre du cercle
    Courbe_Tracer_croix(Centre_X,Centre_Y);

    if (Menu_visible)
    {
      if (Config.Coords_rel)
        Print_dans_menu("X:        Y:",0);
      else
        Print_dans_menu("X:       Y:             ",0);
      Aff_coords_rel_ou_abs(Centre_X,Centre_Y);
    }
  }
  else
  {
    Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                           :Centre_X-Tangente_X;
    Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                           :Centre_Y-Tangente_Y;
    Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

    Cacher_pinceau=Cacher_pinceau_avant_operation;
    Forme_curseur=FORME_CURSEUR_CIBLE;

    Tracer_ellipse_pleine(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Back_color);

    if ((Config.Coords_rel) && (Menu_visible))
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }
  }
}


void Ellipse_degradee_12_8(void)
//
// Opération   : OPERATION_ELLIPSE_DEGRADEE
// Click Souris: 0
// Taille_Pile : 8 (Mouse_K, Couleur, X_Centre, Y_Centre, X_Tangente, Y_Tangente, Ancien_X, Ancien_Y)
//
// Souris effacée: Oui
//
{
  short Tangente_X;
  short Tangente_Y;
  short Centre_X;
  short Centre_Y;
  short Couleur;
  short Rayon_horizontal;
  short Rayon_vertical;
  short Ancien_Mouse_K;

  Operation_Taille_pile-=2;   // On fait sauter les 2 derniers élts de la pile
  Operation_POP(&Tangente_Y);
  Operation_POP(&Tangente_X);
  Operation_POP(&Centre_Y);
  Operation_POP(&Centre_X);
  Operation_POP(&Couleur);
  Operation_POP(&Ancien_Mouse_K);

  // On efface la croix XOR au centre de l'ellipse
  Courbe_Tracer_croix(Centre_X,Centre_Y);

  Rayon_horizontal=(Tangente_X>Centre_X)?Tangente_X-Centre_X
                                         :Centre_X-Tangente_X;
  Rayon_vertical  =(Tangente_Y>Centre_Y)?Tangente_Y-Centre_Y
                                         :Centre_Y-Tangente_Y;
  Effacer_ellipse_vide_Preview(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical);

  Cacher_pinceau=Cacher_pinceau_avant_operation;
  Forme_curseur=FORME_CURSEUR_CIBLE;

  if (Mouse_K==Ancien_Mouse_K)
    Tracer_ellipse_degradee(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Pinceau_X,Pinceau_Y);

  Attendre_fin_de_click();

  if ((Config.Coords_rel) && (Menu_visible))
  {
    Print_dans_menu("X:       Y:             ",0);
    Print_coordonnees();
  }
}

/******************************
* Operation_Rectangle_Degrade *
******************************/

// 1) tracé d'un rectangle classique avec les lignes XOR
// 2) tracé d'une ligne vecteur de dégradé, comme une ligne normale
// 3) dessin du dégradé


void Rectangle_Degrade_12_0(void)
// Opération   : OPERATION_RECTANGLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
// Souris effacée: Oui

// Initialisation de l'étape 1, on commence à dessiner le rectangle
{
  Initialiser_debut_operation();
  Backup();

  if ((Config.Coords_rel) && (Menu_visible))
    Print_dans_menu("\035:   1   \022:   1",0);
  // On laisse une trace du curseur à l'écran
  Afficher_curseur();

  if (Mouse_K==A_GAUCHE)
  {
    Shade_Table=Shade_Table_gauche;
    Operation_PUSH(Fore_color);
  }
  else
  {
    Shade_Table=Shade_Table_droite;
    Operation_PUSH(Back_color);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


void Rectangle_Degrade_12_5(void)
// Opération   : OPERATION_RECTANGLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 5
//
// Souris effacée: Non

// Modification de la taille du rectangle
{
  short Debut_X;
  short Debut_Y;
  short Ancien_X;
  short Ancien_Y;
  char  Chaine[5];

  Operation_POP(&Ancien_Y);
  Operation_POP(&Ancien_X);

  if ((Pinceau_X!=Ancien_X) || (Pinceau_Y!=Ancien_Y))
  {
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);

    if ((Config.Coords_rel) && (Menu_visible))
    {
      Num2str(((Debut_X<Pinceau_X)?Pinceau_X-Debut_X:Debut_X-Pinceau_X)+1,Chaine,4);
      Print_dans_menu(Chaine,2);
      Num2str(((Debut_Y<Pinceau_Y)?Pinceau_Y-Debut_Y:Debut_Y-Pinceau_Y)+1,Chaine,4);
      Print_dans_menu(Chaine,11);
    }
    else
      Print_coordonnees();

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
  }

  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Rectangle_Degrade_0_5(void)
// OPERATION_RECTANGLE_DEGRADE
// Clic souris 0
// Taile pile : 5
//
// Souris effacée : non

// Le rectangle est en place, maintenant il faut tracer le vecteur de dégradé,
// on doit donc attendre que l'utilisateur clique quelque part
// On stocke tout de suite les coordonnées du pinceau comme ça on change d'état et on passe à la suite
{
  // !!! Cette fonction remet RAX RAY RBX RBY dans la pile à la fin donc il ne faut pas les modifier ! (sauf éventuellement un tri)
  short RAX;
  short RAY;
  short RBX;
  short RBY, largeur,hauteur;
  short decalage_largeur = 0;
  short decalage_hauteur = 0;
  short decalage_gauche = 0;
  short decalage_haut = 0;


  // Tracé propre du rectangle
  Operation_POP(&RBY);
  Operation_POP(&RBX);
  Operation_POP(&RAY);
  Operation_POP(&RAX);

  Pinceau_X = RAX;
  Pinceau_Y = RAY;
  Effacer_curseur();

  largeur = abs(RBX-RAX);
  hauteur = abs(RBY-RAY);

  if (Max(RAX,RBX)-Principal_Decalage_X > Min(Principal_Largeur_image,Loupe_Mode?Principal_Split:Largeur_ecran)) // Tous les clippings à gérer sont là
    decalage_largeur = Max(RAX,RBX) - Min(Principal_Largeur_image,Loupe_Mode?Principal_Split:Largeur_ecran);

  if (Max(RAY,RBY)-Principal_Decalage_Y > Min(Principal_Hauteur_image,Menu_Ordonnee))
      decalage_hauteur = Max(RAY,RBY) - Min(Principal_Hauteur_image,Menu_Ordonnee);

  // Dessin dans la zone de dessin normale
  Ligne_horizontale_XOR(Min(RAX,RBX)-Principal_Decalage_X,Min(RAY,RBY)-Principal_Decalage_Y,largeur - decalage_largeur);
  if(decalage_hauteur == 0)
    Ligne_horizontale_XOR(Min(RAX,RBX)-Principal_Decalage_X,Max(RAY,RBY)-1-Principal_Decalage_Y,largeur - decalage_largeur);

  Ligne_verticale_XOR(Min(RAX,RBX)-Principal_Decalage_X,Min(RAY,RBY)-Principal_Decalage_Y,hauteur-decalage_hauteur);
  if (decalage_largeur == 0) // Sinon cette ligne est en dehors de la zone image, inutile de la dessiner
    Ligne_verticale_XOR(Max(RAX,RBX)-1-Principal_Decalage_X,Min(RAY,RBY)-Principal_Decalage_Y,hauteur-decalage_hauteur);

  UpdateRect(Min(RAX,RBX)-Principal_Decalage_X,Min(RAY,RBY)-Principal_Decalage_Y,largeur+1-decalage_largeur,hauteur+1-decalage_hauteur);

  // Dessin dans la zone zoomée
  if(Loupe_Mode)
  {
    decalage_largeur = 0;
    if(Min(RAX,RBX)<Limite_Gauche_Zoom)
    {
	decalage_largeur += Limite_Gauche_Zoom - Min(RAX,RAY);
	decalage_gauche = Limite_Gauche_Zoom;
    }
    if(Max(RAX,RAY)>Limite_visible_Droite_Zoom)
	decalage_largeur += Max(RAX,RAY) - Limite_visible_Droite_Zoom;

    decalage_hauteur=0;
    if(Min(RAY,RBY)<Limite_Haut_Zoom)
    {
	decalage_hauteur += Limite_Haut_Zoom - Min(RAY,RBY);
	decalage_haut = Limite_Haut_Zoom;
    }
    if(Max(RAY,RBY)>Limite_visible_Bas_Zoom)
	decalage_hauteur += Max(RAX,RAY) + Limite_visible_Bas_Zoom;

    if(decalage_haut==0)
	Ligne_horizontale_XOR_Zoom(decalage_gauche>0?decalage_gauche:Min(RAX,RBX),Min(RAY,RBY),largeur-decalage_largeur);
    if(Max(RAY,RBY)<Limite_visible_Bas_Zoom)
	Ligne_horizontale_XOR_Zoom(decalage_gauche>0?decalage_gauche:Min(RAX,RBX),Max(RAY,RBY),largeur-decalage_largeur);
    if(decalage_gauche==0)
	Ligne_verticale_XOR_Zoom(Min(RAX,RBX),decalage_haut>0?decalage_haut:Min(RAY,RBY),hauteur-decalage_hauteur);
    if(Max(RAX,RBX)<Limite_visible_Droite_Zoom)
	Ligne_verticale_XOR_Zoom(Max(RAX,RBX),decalage_haut>0?decalage_haut:Min(RAY,RBY),hauteur-decalage_hauteur);
  }

  Operation_PUSH(RAX);
  Operation_PUSH(RAY);
  Operation_PUSH(RBX);
  Operation_PUSH(RBY);

//  Forme_curseur = FORME_CURSEUR_CIBLE; // On reste en mode cible XOR pour ne pas effacer l'empreinte du rectangle ...
  Operation_PUSH(RBX);
  Operation_PUSH(RBY);
}

void Rectangle_Degrade_0_7(void)
// OPERATION_RECTANGLE_DEGRADE
// Clic souris 0
// Taile pile : 5
//
// Souris effacée : non

// On continue à attendre que l'utilisateur clique en gardant les coords à jour
{
    Operation_Taille_pile -= 2;
    Print_coordonnees();
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
}

void Rectangle_Degrade_12_7(void)
// Opération   : OPERATION_RECTANGLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 7
//
//  Souris effacée: Oui

//  Début du tracé du vecteur (premier clic)
// On garde les anciennes coordonnées dans la pile, et on ajoute les nouvelles par dessus
{
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Rectangle_Degrade_12_9(void)
// Opération   : OPERATION_RECTANGLE_DEGRADE
// Click Souris: 1
// Taille_Pile : 5
//
// Souris effacée: Oui

// Poursuite du tracé du vecteur (déplacement de la souris en gardant le curseur appuyé)
{
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;

  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Pinceau_X!=Fin_X) || (Pinceau_Y!=Fin_Y))
  {
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
      // On corrige les coordonnées de la ligne si la touche shift est appuyée...
      if(SDL_GetModState() & KMOD_SHIFT)
	  Rectifier_coordonnees_a_45_degres(Debut_X,Debut_Y,&Pinceau_X,&Pinceau_Y);

    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);

    Effacer_ligne_Preview(Debut_X,Debut_Y,Fin_X,Fin_Y);
    if (Mouse_K==A_GAUCHE)
    {
      Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Fore_color);
    }
    else
    {
      Tracer_ligne_Preview (Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Back_color);
    }

    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
  }


  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}

void Rectangle_Degrade_0_9(void)
// Opération   : OPERATION_RECTANGLE_DEGRADE
// Click Souris: 1 ou 2
// Taille_Pile : 0
//
//  Souris effacée: Oui

// Ouf, fini ! on dessine enfin le rectangle avec son dégradé
{
    short Rect_Debut_X;
    short Rect_Debut_Y;
    short Rect_Fin_X;
    short Rect_Fin_Y;

    short Vecteur_Debut_X;
    short Vecteur_Debut_Y;
    short Vecteur_Fin_X;
    short Vecteur_Fin_Y;

    Operation_POP(&Vecteur_Fin_Y);
    Operation_POP(&Vecteur_Fin_X);
    Operation_POP(&Vecteur_Debut_Y);
    Operation_POP(&Vecteur_Debut_X);
    Operation_POP(&Rect_Fin_Y);
    Operation_POP(&Rect_Fin_X);
    Operation_POP(&Rect_Debut_Y);
    Operation_POP(&Rect_Debut_X);
    Operation_Taille_pile --;

    // Maintenant on efface tout le bazar temporaire : rectangle et ligne XOR
    Effacer_ligne_Preview(Vecteur_Debut_X,Vecteur_Debut_Y,Vecteur_Fin_X,Vecteur_Fin_Y);
   
    // Et enfin on trace le rectangle avec le dégradé dedans !
//    if (Mouse_K==Ancien_Mouse_K)				// TODO sauver l'ancien mouse K à la place de la couleur dans l'étape 1. Modifier aussi les autres étapes pour pouvoir annuler à tout moment.
	Tracer_rectangle_degrade(Rect_Debut_X,Rect_Debut_Y,Rect_Fin_X,Rect_Fin_Y,Vecteur_Debut_X,Vecteur_Debut_Y,Vecteur_Fin_X,Vecteur_Fin_Y);

    Attendre_fin_de_click();

    if ((Config.Coords_rel) && (Menu_visible))
    {
	Print_dans_menu("X:       Y:             ",0);
	Print_coordonnees();
    }
}
/////////////////////////////////////////////////// OPERATION_LIGNES_CENTREES


void Lignes_centrees_12_0(void)
    // Opération   : OPERATION_LIGNES_CENTREES
    // Click Souris: 1 ou 2
    // Taille_Pile : 0
    //
    //  Souris effacée: Oui
{
    Initialiser_debut_operation();
    Backup();
    Shade_Table=(Mouse_K==A_GAUCHE)?Shade_Table_gauche:Shade_Table_droite;

    if ((Config.Coords_rel) && (Menu_visible))
	Print_dans_menu("X:±   0   Y:±   0",0);

    Operation_PUSH(Mouse_K);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
}


void Lignes_centrees_12_3(void)
    // Opération   : OPERATION_LIGNES_CENTREES
    // Click Souris: 1 ou 2
    // Taille_Pile : 3
    //
    // Souris effacée: Non
{
    short Debut_X;
    short Debut_Y;

    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
}


void Lignes_centrees_0_3(void)
    // Opération   : OPERATION_LIGNES_CENTREES
    // Click Souris: 0
    // Taille_Pile : 3
    //
    // Souris effacée: Oui
{
    short Debut_X;
    short Debut_Y;
    short Bouton;
    short Couleur;

    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Operation_POP(&Bouton);

    Couleur=(Bouton==A_GAUCHE)?Fore_color:Back_color;

    Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);
    Pinceau_Forme_avant_operation=Pinceau_Forme;
    Pinceau_Forme=FORME_PINCEAU_POINT;

    Operation_PUSH(Bouton);
    Operation_PUSH(Pinceau_X); // Nouveau début X
    Operation_PUSH(Pinceau_Y); // Nouveau début Y
    Operation_PUSH(Pinceau_X); // Nouvelle dernière fin X
    Operation_PUSH(Pinceau_Y); // Nouvelle dernière fin Y
    Operation_PUSH(Pinceau_X); // Nouvelle dernière position X
    Operation_PUSH(Pinceau_Y); // Nouvelle dernière position Y
}


void Lignes_centrees_12_7(void)
    // Opération   : OPERATION_LIGNES_CENTREES
    // Click Souris: 1 ou 2
    // Taille_Pile : 7
    //
    // Souris effacée: Non
{
    short Bouton;
    short Debut_X;
    short Debut_Y;
    short Fin_X;
    short Fin_Y;
    short Dernier_X;
    short Dernier_Y;
    short Couleur;

    Operation_POP(&Dernier_Y);
  Operation_POP(&Dernier_X);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);
  Operation_POP(&Debut_Y);
  Operation_POP(&Debut_X);
  Operation_POP(&Bouton);

  if (Mouse_K==Bouton)
  {
    if ( (Fin_X!=Pinceau_X) || (Fin_Y!=Pinceau_Y) ||
         (Dernier_X!=Pinceau_X) || (Dernier_Y!=Pinceau_Y) )
    {
      Effacer_curseur();

      Couleur=(Bouton==A_GAUCHE)?Fore_color:Back_color;

      Pinceau_Forme=Pinceau_Forme_avant_operation;

      Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
      Effacer_ligne_Preview (Debut_X,Debut_Y,Dernier_X,Dernier_Y);

      Smear_Debut=1;
      Afficher_pinceau      (Debut_X,Debut_Y,Couleur,0);
      Tracer_ligne_Definitif(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

      Pinceau_Forme=FORME_PINCEAU_POINT;
      Pixel_figure_Preview(Pinceau_X,Pinceau_Y,Couleur);
      Tracer_ligne_Preview(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

      Afficher_curseur();
    }

    Operation_PUSH(Bouton);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
    Operation_PUSH(Pinceau_X);
    Operation_PUSH(Pinceau_Y);
  }
  else
  {
    Effacer_curseur();

    Pinceau_Forme=Pinceau_Forme_avant_operation;

    Pixel_figure_Preview_auto  (Debut_X,Debut_Y);
    Effacer_ligne_Preview (Debut_X,Debut_Y,Dernier_X,Dernier_Y);

    if ( (Config.Coords_rel) && (Menu_visible) )
    {
      Print_dans_menu("X:       Y:             ",0);
      Print_coordonnees();
    }

    Afficher_curseur();

    Attendre_fin_de_click();
  }
}


void Lignes_centrees_0_7(void)
// Opération   : OPERATION_LIGNES_CENTREES
// Click Souris: 0
// Taille_Pile : 7
//
// Souris effacée: Non
{
  short Bouton;
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Dernier_X;
  short Dernier_Y;
  short Couleur;

  Operation_POP(&Dernier_Y);
  Operation_POP(&Dernier_X);
  Operation_POP(&Fin_Y);
  Operation_POP(&Fin_X);

  if ((Pinceau_X!=Dernier_X) || (Pinceau_Y!=Dernier_Y))
  {
    Effacer_curseur();
    Operation_POP(&Debut_Y);
    Operation_POP(&Debut_X);
    Operation_POP(&Bouton);

    Couleur=(Bouton==A_GAUCHE)?Fore_color:Back_color;

    Aff_coords_rel_ou_abs(Debut_X,Debut_Y);

    Effacer_ligne_Preview(Debut_X,Debut_Y,Dernier_X,Dernier_Y);

    Pixel_figure_Preview(Debut_X,Debut_Y,Couleur);
    Tracer_ligne_Preview(Debut_X,Debut_Y,Pinceau_X,Pinceau_Y,Couleur);

    Operation_PUSH(Bouton);
    Operation_PUSH(Debut_X);
    Operation_PUSH(Debut_Y);
    Afficher_curseur();
  }

  Operation_PUSH(Fin_X);
  Operation_PUSH(Fin_Y);
  Operation_PUSH(Pinceau_X);
  Operation_PUSH(Pinceau_Y);
}


