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
//
//  Ce fichier contient la gestion du moteur
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include "special.h"
#include "boutons.h"
#include "operatio.h"
#include "shade.h"
#include "erreurs.h"
#include "sdlscreen.h"
#include "windows.h"
#include "brush.h"
#include "input.h"
#include "moteur.h"


// we need this as global
short Old_MX = -1;
short Old_MY = -1;

//---------- Annuler les effets des modes de dessin (sauf la grille) ---------

// Variables mémorisants les anciens effets

byte Shade_Mode_avant_annulation;
byte Quick_shade_Mode_avant_annulation;
byte Stencil_Mode_avant_annulation;
byte Trame_Mode_avant_annulation;
byte Colorize_Mode_avant_annulation;
byte Smooth_Mode_avant_annulation;
byte Tiling_Mode_avant_annulation;
fonction_effet Fonction_effet_avant_annulation;

byte* Fond_fenetre[8];

void Annuler_les_effets(void)
{
  Shade_Mode_avant_annulation=Shade_Mode;
  Shade_Mode=0;

  Quick_shade_Mode_avant_annulation=Quick_shade_Mode;
  Quick_shade_Mode=0;

  Stencil_Mode_avant_annulation=Stencil_Mode;
  Stencil_Mode=0;

  Trame_Mode_avant_annulation=Trame_Mode;
  Trame_Mode=0;

  Colorize_Mode_avant_annulation=Colorize_Mode;
  Colorize_Mode=0;

  Smooth_Mode_avant_annulation=Smooth_Mode;
  Smooth_Mode=0;

  Tiling_Mode_avant_annulation=Tiling_Mode;
  Tiling_Mode=0;

  Fonction_effet_avant_annulation=Fonction_effet;
  Fonction_effet=Aucun_effet;
}


//----------------- Restaurer les effets des modes de dessin -----------------
void Restaurer_les_effets(void)
{
  Shade_Mode      =Shade_Mode_avant_annulation;
  Quick_shade_Mode=Quick_shade_Mode_avant_annulation;
  Stencil_Mode    =Stencil_Mode_avant_annulation;
  Trame_Mode      =Trame_Mode_avant_annulation;
  Colorize_Mode   =Colorize_Mode_avant_annulation;
  Smooth_Mode     =Smooth_Mode_avant_annulation;
  Tiling_Mode     =Tiling_Mode_avant_annulation;
  Fonction_effet  =Fonction_effet_avant_annulation;
}


char * TITRE_BOUTON[NB_BOUTONS]=
{
  "Paintbrush choice       ",
  "Adjust picture / Effects",
  "Freehand draw. / Toggle ",
  "Splines / Toggle        ",
  "Lines / Toggle          ",
  "Spray / Menu            ",
  "Floodfill / Replace col.",
  "Polylines / Polyforms   ",
  "Polyfill / Filled Pforms",
  "Empty rectangles        ",
  "Filled rectangles       ",
  "Empty circles / ellipses",
  "Filled circles / ellips.",
  "Grad. rectangles        ",
  "Gradation menu          ",
  "Grad. spheres / ellipses",
  "Brush grab. / Restore   ",
  "Lasso / Restore brush   ",
  "Brush effects           ",
  "Drawing modes (effects) ",
  "Text                    ",
  "Magnify mode / Menu     ",
  "Pipette / Invert colors ",
  "Screen size / Safe. res.",
  "Go / Copy to other page ",
  "Save as / Save          ",
  "Load / Re-load          ",
  "Settings                ",
  "Clear / with backcolor  ",
  "Help / Statistics       ",
  "Undo / Redo             ",
  "Kill current page       ",
  "Quit                    ",
  "Palette editor          ",
  "Scroll pal. left / Fast ",
  "Scroll pal. right / Fast",
  "Color #"                 ,
  "Hide tool bar           "
};
// Sauvegarde un bloc (généralement l'arrière-plan d'une fenêtre)
void Sauve_fond(byte **Buffer, int Pos_X, int Pos_Y, int width, int height)
{
  int Indice;
  if(*Buffer != NULL) DEBUG("WARNING : Buffer already allocated !!!",0);
  *Buffer=(byte *) malloc(width*Menu_Facteur_X*height*Menu_Facteur_Y*Pixel_width);
  for (Indice=0; Indice<(height*Menu_Facteur_Y); Indice++)
    Lire_ligne(Pos_X,Pos_Y+Indice,width*Menu_Facteur_X,(*Buffer)+((int)Indice*width*Menu_Facteur_X*Pixel_width));
}
// Restaure de ce que la fenêtre cachait
void Restaure_fond(byte *Buffer, int Pos_X, int Pos_Y, int width, int height)
{
  int Indice;
  for (Indice=0; Indice<height*Menu_Facteur_Y; Indice++)
    Afficher_ligne_fast(Pos_X,Pos_Y+Indice,width*Menu_Facteur_X,Buffer+((int)Indice*width*Menu_Facteur_X*Pixel_width));
  free(Buffer);
}
// Ecrit un pixel dans un fond de fenêtre
void Pixel_fond(int Pos_X, int Pos_Y, byte Couleur)
{
  int Repetitions_X=Pixel_width;
  while (Repetitions_X--)
    (Fond_fenetre[0][Pos_X*Pixel_width+Repetitions_X+Pos_Y*Fenetre_Largeur*Pixel_width*Menu_Facteur_X])=Couleur;
}



int Numero_bouton_sous_souris(void)
{
  int Numero;
  short Pos_X;
  short Pos_Y;

  Pos_X=(Mouse_X              )/Menu_Facteur_X;
  Pos_Y=(Mouse_Y-Menu_Ordonnee)/Menu_Facteur_Y;

  for (Numero=0;Numero<NB_BOUTONS;Numero++)
  {
    switch(Bouton[Numero].Shape)
    {
      case FORME_BOUTON_SANS_CADRE :
      case FORME_BOUTON_RECTANGLE  :

        if ((Pos_X>=Bouton[Numero].Decalage_X) &&
            (Pos_Y>=Bouton[Numero].Decalage_Y) &&
            (Pos_X<=Bouton[Numero].Decalage_X+Bouton[Numero].Width) &&
            (Pos_Y<=Bouton[Numero].Decalage_Y+Bouton[Numero].Height))
          return Numero;
        break;

      case FORME_BOUTON_TRIANGLE_HAUT_GAUCHE:
        if ((Pos_X>=Bouton[Numero].Decalage_X) &&
            (Pos_Y>=Bouton[Numero].Decalage_Y) &&
            (Pos_X+Pos_Y-(short)Bouton[Numero].Decalage_Y-(short)Bouton[Numero].Decalage_X<=Bouton[Numero].Width))
          return Numero;
        break;

      case FORME_BOUTON_TRIANGLE_BAS_DROITE:
        if ((Pos_X<=Bouton[Numero].Decalage_X+Bouton[Numero].Width) &&
            (Pos_Y<=Bouton[Numero].Decalage_Y+Bouton[Numero].Height) &&
            (Pos_X+Pos_Y-(short)Bouton[Numero].Decalage_Y-(short)Bouton[Numero].Decalage_X>=Bouton[Numero].Width))
          return Numero;
        break;
    }
  }
  return -1;
}


void Tracer_cadre_de_bouton_du_menu(byte Numero,byte pressed)
{
  byte Couleur_Haut_gauche;
  byte Couleur_Bas_droite;
  byte Couleur_Diagonale;
  word Debut_X;
  word Debut_Y;
  word Fin_X;
  word Fin_Y;
  word Pos_X;
  word Pos_Y;

  Debut_X=Bouton[Numero].Decalage_X;
  Debut_Y=Bouton[Numero].Decalage_Y;
  Fin_X  =Debut_X+Bouton[Numero].Width;
  Fin_Y  =Debut_Y+Bouton[Numero].Height;

  if (!pressed)
  {
    Couleur_Haut_gauche=CM_Blanc;
    Couleur_Bas_droite=CM_Fonce;
    Couleur_Diagonale=CM_Clair;
  }
  else
  {
    Couleur_Haut_gauche=CM_Fonce;
    Couleur_Bas_droite=CM_Noir;
    Couleur_Diagonale=CM_Fonce;
  }

  switch(Bouton[Numero].Shape)
  {
    case FORME_BOUTON_SANS_CADRE :
      break;
    case FORME_BOUTON_RECTANGLE  :
      // On colorie le point haut droit
      Pixel_dans_menu(Fin_X,Debut_Y,Couleur_Diagonale);
      BLOCK_MENU[Debut_Y][Fin_X]=Couleur_Diagonale;
      // On colorie le point bas gauche
      Pixel_dans_menu(Debut_X,Fin_Y,Couleur_Diagonale);
      BLOCK_MENU[Fin_Y][Debut_X]=Couleur_Diagonale;
      // On colorie la partie haute
      for (Pos_X=Debut_X;Pos_X<=Fin_X-1;Pos_X++)
      {
        Pixel_dans_menu(Pos_X,Debut_Y,Couleur_Haut_gauche);
        BLOCK_MENU[Debut_Y][Pos_X]=Couleur_Haut_gauche;
      }
      for (Pos_Y=Debut_Y+1;Pos_Y<=Fin_Y-1;Pos_Y++)
      {
        // On colorie la partie gauche
        Pixel_dans_menu(Debut_X,Pos_Y,Couleur_Haut_gauche);
        BLOCK_MENU[Pos_Y][Debut_X]=Couleur_Haut_gauche;
        // On colorie la partie droite
        Pixel_dans_menu(Fin_X,Pos_Y,Couleur_Bas_droite);
        BLOCK_MENU[Pos_Y][Fin_X]=Couleur_Bas_droite;
      }
      // On colorie la partie basse
      for (Pos_X=Debut_X+1;Pos_X<=Fin_X;Pos_X++)
      {
        Pixel_dans_menu(Pos_X,Fin_Y,Couleur_Bas_droite);
        BLOCK_MENU[Fin_Y][Pos_X]=Couleur_Bas_droite;
      }
      break;
    case FORME_BOUTON_TRIANGLE_HAUT_GAUCHE:
      // On colorie le point haut droit
      Pixel_dans_menu(Fin_X,Debut_Y,Couleur_Diagonale);
      BLOCK_MENU[Debut_Y][Fin_X]=Couleur_Diagonale;
      // On colorie le point bas gauche
      Pixel_dans_menu(Debut_X,Fin_Y,Couleur_Diagonale);
      BLOCK_MENU[Fin_Y][Debut_X]=Couleur_Diagonale;
      // On colorie le coin haut gauche
      for (Pos_X=0;Pos_X<Bouton[Numero].Width;Pos_X++)
      {
        Pixel_dans_menu(Debut_X+Pos_X,Debut_Y,Couleur_Haut_gauche);
        BLOCK_MENU[Debut_Y][Debut_X+Pos_X]=Couleur_Haut_gauche;
        Pixel_dans_menu(Debut_X,Debut_Y+Pos_X,Couleur_Haut_gauche);
        BLOCK_MENU[Debut_Y+Pos_X][Debut_X]=Couleur_Haut_gauche;
      }
      // On colorie la diagonale
      for (Pos_X=1;Pos_X<Bouton[Numero].Width;Pos_X++)
      {
        Pixel_dans_menu(Debut_X+Pos_X,Fin_Y-Pos_X,Couleur_Bas_droite);
        BLOCK_MENU[Fin_Y-Pos_X][Debut_X+Pos_X]=Couleur_Bas_droite;
      }
      break;
    case FORME_BOUTON_TRIANGLE_BAS_DROITE:
      // On colorie le point haut droit
      Pixel_dans_menu(Fin_X,Debut_Y,Couleur_Diagonale);
      BLOCK_MENU[Debut_Y][Fin_X]=Couleur_Diagonale;
      // On colorie le point bas gauche
      Pixel_dans_menu(Debut_X,Fin_Y,Couleur_Diagonale);
      BLOCK_MENU[Fin_Y][Debut_X]=Couleur_Diagonale;
      // On colorie la diagonale
      for (Pos_X=1;Pos_X<Bouton[Numero].Width;Pos_X++)
      {
        Pixel_dans_menu(Debut_X+Pos_X,Fin_Y-Pos_X,Couleur_Haut_gauche);
        BLOCK_MENU[Fin_Y-Pos_X][Debut_X+Pos_X]=Couleur_Haut_gauche;
      }
      // On colorie le coin bas droite
      for (Pos_X=0;Pos_X<Bouton[Numero].Width;Pos_X++)
      {
        Pixel_dans_menu(Fin_X-Pos_X,Fin_Y,Couleur_Bas_droite);
        BLOCK_MENU[Fin_Y][Fin_X-Pos_X]=Couleur_Bas_droite;
        Pixel_dans_menu(Fin_X,Fin_Y-Pos_X,Couleur_Bas_droite);
        BLOCK_MENU[Fin_Y-Pos_X][Fin_X]=Couleur_Bas_droite;
      }
  }
  if (Menu_visible)
  {
    UpdateRect(
      Debut_X*Menu_Facteur_X,
      Debut_Y*Menu_Facteur_Y + Menu_Ordonnee,
      (Fin_X+1-Debut_X)*Menu_Facteur_X,
      (Fin_Y+1-Debut_Y)*Menu_Facteur_Y);
  }
}


//---------------------- Désenclenchement d'un bouton ------------------------
void Desenclencher_bouton(int Numero)
{
  if (Bouton[Numero].Enfonce)
  {
    // On affiche le cadre autour du bouton de façon à ce qu'il paraisse relâché
    Tracer_cadre_de_bouton_du_menu(Numero,BOUTON_RELACHE);
    // On considère que le bouton est relâché
    Bouton[Numero].Enfonce=BOUTON_RELACHE;
    // On appelle le désenclenchement particulier au bouton:
    Bouton[Numero].Desenclencher();
  }
}


//-Enclenchement d'un bouton (et désenclenchement de ceux de la même famille)-
void Enclencher_bouton(int Numero,byte click)
{
  int family;
  int b;
  int Icone;

  Effacer_curseur();

  // Certains boutons ont deux icones
  Icone=-1;
  switch(Numero)
  {
    case BOUTON_POLYGONES:
    case BOUTON_POLYFILL:
      Icone=12;break;
    case BOUTON_FLOODFILL:
      Icone=14;break;
    case BOUTON_CERCLES:
    case BOUTON_FILLCERC:
      Icone=10;break;
    case BOUTON_SPHERES:
      Icone=16;break;
  }
  if (Icone!=-1)
    Afficher_sprite_dans_menu(Numero,Icone+(click==A_DROITE));

  // On note déjà la famille du bouton (La "Famiglia" c'est sacré)
  family=Bouton[Numero].Famille;

  switch (family)
  {
    case FAMILLE_TOOLBAR: // On ne fait rien (on préserve les interruptions)
      break;

    case FAMILLE_INTERRUPTION: // Petit cas spécial dans la famille "Interruption":
      if ((Numero!=BOUTON_LOUPE) || (!Loupe_Mode))
      // Pour chaque bouton:
      for (b=0; b<NB_BOUTONS; b++)
        // S'il est de la même famille
        if (
             (b!=Numero) &&
             (Bouton[b].Famille==FAMILLE_INTERRUPTION) &&
             (  (b!=BOUTON_LOUPE) ||
               ((b==BOUTON_LOUPE) && (!Loupe_Mode)) )
           )
          // Alors on désenclenche le bouton
          Desenclencher_bouton(b);
      break;

    default:
      // On désenclenche D'ABORD les interruptions
      // Pour chaque bouton:
      for (b=0; b<NB_BOUTONS; b++)
        // S'il est de la famille interruption
        if ( (b!=Numero)
          && (Bouton[b].Famille==FAMILLE_INTERRUPTION)
          // Et que ce n'est pas la loupe, ou alors qu'on n'est pas en mode loupe
          && (!(Loupe_Mode && (b==BOUTON_LOUPE))) )
          // Alors on désenclenche le bouton
          Desenclencher_bouton(b);
      // Pour chaque bouton:
      for (b=0; b<NB_BOUTONS; b++)
        // S'il est de la même famille
        if ( (b!=Numero)
          && (Bouton[b].Famille==family) )
          // Alors on désenclenche le bouton
          Desenclencher_bouton(b);
  }

  // On affiche le cadre autour du bouton de façon à ce qu'il paraisse enfoncé
  Tracer_cadre_de_bouton_du_menu(Numero,BOUTON_ENFONCE);

  Afficher_curseur();

  // On attend ensuite que l'utilisateur lâche son bouton:
  Attendre_fin_de_click();

  // On considère que le bouton est enfoncé
  Bouton[Numero].Enfonce=BOUTON_ENFONCE;

  // Puis on se contente d'appeler l'action correspondant au bouton:
  if (click==1)
    Bouton[Numero].Gauche();
  else
    Bouton[Numero].Droite();
}



//--- Déplacer la barre de séparation entre la partie zoomée et la partie ----
//------------------ non-zoomée lorsqu'on est en mode loupe ------------------
void Deplacer_Split(void)
{
  short Ancien_Split=Principal_Split;
  short Ancien_X_Zoom=Principal_X_Zoom;
  short offset=Principal_X_Zoom-Mouse_X;
  byte  Ancienne_forme_curseur=Forme_curseur;
  short Ancien_Mouse_X=-1;

  // Afficher la barre en XOR
  Effacer_curseur();
  Fenetre=1;
  Forme_curseur=FORME_CURSEUR_HORIZONTAL;
  Ligne_verticale_XOR(Principal_Split,0,Menu_Ordonnee);
  Ligne_verticale_XOR(Principal_X_Zoom-1,0,Menu_Ordonnee);
  Afficher_curseur();
  UpdateRect(Principal_Split,0,abs(Principal_Split-Principal_X_Zoom)+1,Menu_Ordonnee);

  while (Mouse_K)
  {
    if (Mouse_X!=Ancien_Mouse_X)
    {
      Ancien_Mouse_X=Mouse_X;
      Principal_Proportion_split=(float)(Mouse_X+offset)/Largeur_ecran;
      Calculer_split();
  
      if (Principal_X_Zoom!=Ancien_X_Zoom)
      {
        Effacer_curseur();
  
        // Effacer la barre en XOR
        Ligne_verticale_XOR(Ancien_Split,0,Menu_Ordonnee);
        Ligne_verticale_XOR(Ancien_X_Zoom-1,0,Menu_Ordonnee);
  
        UpdateRect(Ancien_Split,0,abs(Ancien_Split-Ancien_X_Zoom)+1,Menu_Ordonnee);
  
        Ancien_Split=Principal_Split;
        Ancien_X_Zoom=Principal_X_Zoom;
  
        // Rafficher la barre en XOR
        Ligne_verticale_XOR(Principal_Split,0,Menu_Ordonnee);
        Ligne_verticale_XOR(Principal_X_Zoom-1,0,Menu_Ordonnee);
  
        UpdateRect(Principal_Split,0,abs(Principal_Split-Principal_X_Zoom)+1,Menu_Ordonnee);
  
        Afficher_curseur();
      }
    }
    if(!Get_input())Wait_VBL();
  }

  // Effacer la barre en XOR
  Effacer_curseur();
  Ligne_verticale_XOR(Principal_Split,0,Menu_Ordonnee);
  Ligne_verticale_XOR(Principal_X_Zoom-1,0,Menu_Ordonnee);
  Fenetre=0;
  Forme_curseur=Ancienne_forme_curseur;
  Calculer_donnees_loupe();
  Recadrer_ecran_par_rapport_au_zoom();
  Calculer_limites();
  Afficher_ecran();
  Afficher_curseur();
}


//======================= Gestion principale du moteur =======================
void Gestion_principale(void)
{
  static byte Temp_color;
  int  Indice_bouton;           // Numéro de bouton de menu en cours
  int  Indice_bouton_precedent=0; // Numéro de bouton de menu sur lequel on était précédemment
  byte Blink;                   // L'opération demande un effacement du curseur
  int  Bouton_Touche;           // Bouton à enclencher d'après la touche de raccourci enfoncée
  byte Bouton_Cote = 0;         // Côté du bouton à enclencher d'après la touche de raccourci enfoncée
  int  Indice_Touche;           // Indice du tableau de touches spéciales correspondant à la touche enfoncée
  char Chaine[25];
  byte Temp;

  do
  {
    if(Get_input())
    {
    // Redimensionnement demandé
    if (Resize_Largeur || Resize_Hauteur)
    {
      Effacer_curseur();
      Initialiser_mode_video(Resize_Largeur, Resize_Hauteur, 0);
      Mode_video[0].Width = Largeur_ecran*Pixel_width;
      Mode_video[0].Height = Hauteur_ecran*Pixel_height;
      // Remise à zero des variables indiquant le Redimensionnement demandé
      Afficher_menu();
      Afficher_ecran();
      Afficher_curseur();
    }
    
    // Evenement de fermeture
    if (Quit_demande)
    {
      Quit_demande=0;
      Bouton_Quit();
    }
    
    // Gestion des touches
    if (Touche)
    {
      for (Indice_Touche=0;(Indice_Touche<NB_TOUCHES_SPECIALES) && !Est_Raccourci(Touche,Indice_Touche);Indice_Touche++);

      // Gestion des touches spéciales:
      if (Indice_Touche>SPECIAL_CLICK_RIGHT)
      switch(Indice_Touche)
      {
        case SPECIAL_SCROLL_UP : // Scroll up
          if (Loupe_Mode)
            Scroller_loupe(0,-(Loupe_Hauteur>>2));
          else
            Scroller_ecran(0,-(Hauteur_ecran>>3));
          Touche=0;
          break;
        case SPECIAL_SCROLL_DOWN : // Scroll down
          if (Loupe_Mode)
            Scroller_loupe(0,(Loupe_Hauteur>>2));
          else
            Scroller_ecran(0,(Hauteur_ecran>>3));
          Touche=0;
          break;
        case SPECIAL_SCROLL_LEFT : // Scroll left
          if (Loupe_Mode)
            Scroller_loupe(-(Loupe_Largeur>>2),0);
          else
            Scroller_ecran(-(Largeur_ecran>>3),0);
          Touche=0;
          break;
        case SPECIAL_SCROLL_RIGHT : // Scroll right
          if (Loupe_Mode)
            Scroller_loupe((Loupe_Largeur>>2),0);
          else
            Scroller_ecran((Largeur_ecran>>3),0);
          Touche=0;
          break;
        case SPECIAL_SCROLL_UP_FAST : // Scroll up faster
          if (Loupe_Mode)
            Scroller_loupe(0,-(Loupe_Hauteur>>1));
          else
            Scroller_ecran(0,-(Hauteur_ecran>>2));
          Touche=0;
          break;
        case SPECIAL_SCROLL_DOWN_FAST : // Scroll down faster
          if (Loupe_Mode)
            Scroller_loupe(0,(Loupe_Hauteur>>1));
          else
            Scroller_ecran(0,(Hauteur_ecran>>2));
          Touche=0;
          break;
        case SPECIAL_SCROLL_LEFT_FAST : // Scroll left faster
          if (Loupe_Mode)
            Scroller_loupe(-(Loupe_Largeur>>1),0);
          else
            Scroller_ecran(-(Largeur_ecran>>2),0);
          Touche=0;
          break;
        case SPECIAL_SCROLL_RIGHT_FAST : // Scroll right faster
          if (Loupe_Mode)
            Scroller_loupe((Loupe_Largeur>>1),0);
          else
            Scroller_ecran((Largeur_ecran>>2),0);
          Touche=0;
          break;
        case SPECIAL_SCROLL_UP_SLOW : // Scroll up slower
          if (Loupe_Mode)
            Scroller_loupe(0,-1);
          else
            Scroller_ecran(0,-1);
          Touche=0;
          break;
        case SPECIAL_SCROLL_DOWN_SLOW : // Scroll down slower
          if (Loupe_Mode)
            Scroller_loupe(0,1);
          else
            Scroller_ecran(0,1);
          Touche=0;
          break;
        case SPECIAL_SCROLL_LEFT_SLOW : // Scroll left slower
          if (Loupe_Mode)
            Scroller_loupe(-1,0);
          else
            Scroller_ecran(-1,0);
          Touche=0;
          break;
        case SPECIAL_SCROLL_RIGHT_SLOW : // Scroll right slower
          if (Loupe_Mode)
            Scroller_loupe(1,0);
          else
            Scroller_ecran(1,0);
          Touche=0;
          break;
        case SPECIAL_NEXT_FORECOLOR : // Next foreground color
          Special_Next_forecolor();
          Touche=0;
          break;
        case SPECIAL_PREVIOUS_FORECOLOR : // Previous foreground color
          Special_Previous_forecolor();
          Touche=0;
          break;
        case SPECIAL_NEXT_BACKCOLOR : // Next background color
          Special_Next_backcolor();
          Touche=0;
          break;
        case SPECIAL_PREVIOUS_BACKCOLOR : // Previous background color
          Special_Previous_backcolor();
          Touche=0;
          break;
        case SPECIAL_RETRECIR_PINCEAU: // Rétrécir le pinceau
          Retrecir_pinceau();
          Touche=0;
          break;
        case SPECIAL_GROSSIR_PINCEAU: // Grossir le pinceau
          Grossir_pinceau();
          Touche=0;
          break;
        case SPECIAL_NEXT_USER_FORECOLOR : // Next user-defined foreground color
          Message_Non_disponible(); // !!! TEMPORAIRE !!!
          //Special_Next_user_forecolor();
          Touche=0;
          break;
        case SPECIAL_PREVIOUS_USER_FORECOLOR : // Previous user-defined foreground color
          Message_Non_disponible(); // !!! TEMPORAIRE !!!
          //Special_Previous_user_forecolor();
          Touche=0;
          break;
        case SPECIAL_NEXT_USER_BACKCOLOR : // Next user-defined background color
          Message_Non_disponible(); // !!! TEMPORAIRE !!!
          //Special_Next_user_backcolor();
          Touche=0;
          break;
        case SPECIAL_PREVIOUS_USER_BACKCOLOR : // Previous user-defined background color
          Message_Non_disponible(); // !!! TEMPORAIRE !!!
          //Special_Previous_user_backcolor();
          Touche=0;
          break;
        case SPECIAL_SHOW_HIDE_CURSOR : // Show / Hide cursor
          Effacer_curseur();
          Cacher_curseur=!Cacher_curseur;
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_PINCEAU_POINT : // Paintbrush = "."
          Effacer_curseur();
          Pinceau_Forme=FORME_PINCEAU_ROND;
          Modifier_pinceau(1,1);
          Changer_la_forme_du_pinceau(FORME_PINCEAU_ROND);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_DESSIN_CONTINU : // Continuous freehand drawing
          Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE);
          // ATTENTION CE TRUC EST MOCHE ET VA MERDER SI ON SE MET A UTILISER DES BOUTONS POPUPS
          while (Operation_en_cours!=OPERATION_DESSIN_CONTINU)
            Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
          Touche=0;
          break;
        case SPECIAL_FLIP_X : // Flip X
          Effacer_curseur();
          Flip_X_LOWLEVEL();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_FLIP_Y : // Flip Y
          Effacer_curseur();
          Flip_Y_LOWLEVEL();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_ROTATE_90 : // 90° brush rotation
          Effacer_curseur();
          Rotate_90_deg();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_ROTATE_180 : // 180° brush rotation
          Effacer_curseur();
          if (Brosse_Hauteur&1)
          {
            // Brosse de hauteur impaire
            Flip_X_LOWLEVEL();
            Flip_Y_LOWLEVEL();
          }
          else
            Rotate_180_deg_LOWLEVEL();
          Brosse_Decalage_X=(Brosse_Largeur>>1);
          Brosse_Decalage_Y=(Brosse_Hauteur>>1);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_STRETCH : // Stretch brush
          Effacer_curseur();
          Demarrer_pile_operation(OPERATION_ETIRER_BROSSE);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_DISTORT : // Distort brush
          Message_Non_disponible(); // !!! TEMPORAIRE !!!
          Touche=0;
          break;
        case SPECIAL_ROTATE_ANY_ANGLE : // Rotate brush by any angle
          Effacer_curseur();
          Demarrer_pile_operation(OPERATION_TOURNER_BROSSE);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_OUTLINE : // Outline brush
          Effacer_curseur();
          Outline_brush();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_NIBBLE : // Nibble brush
          Effacer_curseur();
          Nibble_brush();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_GET_BRUSH_COLORS : // Get colors from brush
          Get_colors_from_brush();
          Touche=0;
          break;
        case SPECIAL_RECOLORIZE_BRUSH : // Recolorize brush
          Effacer_curseur();
          Remap_brosse();
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_LOAD_BRUSH :
          Load_picture(0);
          Touche=0;
          break;
        case SPECIAL_SAVE_BRUSH :
          Save_picture(0);
          Touche=0;
          break;
        case SPECIAL_ZOOM_IN : // Zoom in
          Zoom(+1);
          Touche=0;
          break;
        case SPECIAL_ZOOM_OUT : // Zoom out
          Zoom(-1);
          Touche=0;
          break;
        case SPECIAL_CENTER_ATTACHMENT : // Center brush attachment
          Effacer_curseur();
          Brosse_Decalage_X=(Brosse_Largeur>>1);
          Brosse_Decalage_Y=(Brosse_Hauteur>>1);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_TOP_LEFT_ATTACHMENT : // Top-left brush attachment
          Effacer_curseur();
          Brosse_Decalage_X=0;
          Brosse_Decalage_Y=0;
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_TOP_RIGHT_ATTACHMENT : // Top-right brush attachment
          Effacer_curseur();
          Brosse_Decalage_X=(Brosse_Largeur-1);
          Brosse_Decalage_Y=0;
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_BOTTOM_LEFT_ATTACHMENT : // Bottom-left brush attachment
          Effacer_curseur();
          Brosse_Decalage_X=0;
          Brosse_Decalage_Y=(Brosse_Hauteur-1);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_BOTTOM_RIGHT_ATTACHMENT : // Bottom right brush attachment
          Effacer_curseur();
          Brosse_Decalage_X=(Brosse_Largeur-1);
          Brosse_Decalage_Y=(Brosse_Hauteur-1);
          Afficher_curseur();
          Touche=0;
          break;
        case SPECIAL_EXCLUDE_COLORS_MENU : // Exclude colors menu
          Menu_Tag_couleurs("Tag colors to exclude",Exclude_color,&Temp,1, NULL);
          Touche=0;
          break;
        case SPECIAL_INVERT_SIEVE :
          Inverser_trame();
          Touche=0;
          break;
        case SPECIAL_SHADE_MODE :
          Bouton_Shade_Mode();
          Touche=0;
          break;
        case SPECIAL_SHADE_MENU :
          Bouton_Shade_Menu();
          Touche=0;
          break;
        case SPECIAL_QUICK_SHADE_MODE :
          Bouton_Quick_shade_Mode();
          Touche=0;
          break;
        case SPECIAL_QUICK_SHADE_MENU :
          Bouton_Quick_shade_Menu();
          Touche=0;
          break;
        case SPECIAL_STENCIL_MODE :
          Bouton_Stencil_Mode();
          Touche=0;
          break;
        case SPECIAL_STENCIL_MENU :
          Bouton_Menu_Stencil();
          Touche=0;
          break;
        case SPECIAL_MASK_MODE :
          Bouton_Mask_Mode();
          Touche=0;
          break;
        case SPECIAL_MASK_MENU :
          Bouton_Mask_Menu();
          Touche=0;
          break;
        case SPECIAL_GRID_MODE :
          Bouton_Snap_Mode();
          Touche=0;
          break;
        case SPECIAL_GRID_MENU :
          Bouton_Menu_Grille();
          Touche=0;
          break;
        case SPECIAL_SIEVE_MODE :
          Bouton_Trame_Mode();
          Touche=0;
          break;
        case SPECIAL_SIEVE_MENU :
          Bouton_Trame_Menu();
          Touche=0;
          break;
        case SPECIAL_COLORIZE_MODE :
          Bouton_Colorize_Mode();
          Touche=0;
          break;
        case SPECIAL_COLORIZE_MENU :
          Bouton_Colorize_Menu();
          Touche=0;
          break;
        case SPECIAL_SMOOTH_MODE :
          Bouton_Smooth_Mode();
          Touche=0;
          break;
        case SPECIAL_SMOOTH_MENU :
          Bouton_Smooth_Menu();
          Touche=0;
          break;
        case SPECIAL_SMEAR_MODE :
          Bouton_Smear_Mode();
          Touche=0;
          break;
        case SPECIAL_TILING_MODE :
          Bouton_Tiling_Mode();
          Touche=0;
          break;
        case SPECIAL_TILING_MENU :
          Bouton_Tiling_Menu();
          Touche=0;
          break;
        default   : // Gestion des touches de raccourci de bouton:
          // Pour chaque bouton
          Bouton_Touche=-1;
          for (Indice_bouton=0;Indice_bouton<NB_BOUTONS;Indice_bouton++)
          {
            if (Est_Raccourci(Touche,0x100+Indice_bouton))
            {
              Bouton_Touche=Indice_bouton;
              Bouton_Cote  =A_GAUCHE;
              Indice_bouton=NB_BOUTONS;
            }
            else if (Est_Raccourci(Touche,0x200+Indice_bouton))
            {
              Bouton_Touche=Indice_bouton;
              Bouton_Cote  =A_DROITE;
              Indice_bouton=NB_BOUTONS;
            }
          }
          // Après avoir scruté les boutons, si la recherche a été fructueuse,
          // on lance le bouton.
          if (Bouton_Touche!=-1)
          {
            Enclencher_bouton(Bouton_Touche,Bouton_Cote);
            Indice_bouton_precedent=-1;
          }
      }

      // Si on a modifié un effet, il faut rafficher le bouton des effets en
      // conséquence.
      if ((Indice_Touche>=SPECIAL_SHADE_MODE)
       && (Indice_Touche<=SPECIAL_TILING_MENU))
      {
        Effacer_curseur();
        Tracer_cadre_de_bouton_du_menu(BOUTON_EFFETS,
          (Shade_Mode||Quick_shade_Mode||Colorize_Mode||Smooth_Mode||Tiling_Mode||Smear_Mode||Stencil_Mode||Mask_Mode||Trame_Mode||Snap_Mode));
        Afficher_curseur();
      }
    }
    }
    else Wait_VBL(); // S'il n'y a pas d'évènement, on ne gère pas tout ça et on attend un peu. La partie en dessous doit être exécutée quand
                     // même pour les trucs asynchrones, par exemple le spray.

    // Gestion de la souris

    Curseur_dans_menu=(Mouse_Y>=Menu_Ordonnee) ||
                      ( (Loupe_Mode) && (Mouse_X>=Principal_Split) &&
                        (Mouse_X<Principal_X_Zoom) );

    if (Curseur_dans_menu)
    {
      // Le curseur se trouve dans le menu

      // On cherche sur quel bouton du menu se trouve la souris:
      Indice_bouton=Numero_bouton_sous_souris();

      // Si le curseur vient de changer de zone

      if ( (Indice_bouton!=Indice_bouton_precedent)
        || (!Curseur_dans_menu_precedent)
        || (Indice_bouton_precedent==BOUTON_CHOIX_COL) )
      {
        // Si le curseur n'est pas sur un bouton
        if (Indice_bouton==-1)
        {
          if (Menu_visible)
          {
            // On nettoie les coordonnées
            Effacer_curseur();
            Block(18*Menu_Facteur_X,Menu_Ordonnee_Texte,192*Menu_Facteur_X,Menu_Facteur_Y<<3,CM_Clair);
            UpdateRect(18*Menu_Facteur_X,Menu_Ordonnee_Texte,192*Menu_Facteur_X,Menu_Facteur_Y<<3);
            Afficher_curseur();
          }
        }
        else
        {
          if ( (Indice_bouton_precedent!=BOUTON_CHOIX_COL)
            || (Temp_color!=Couleur_debut_palette)
            || (Old_MX!=Mouse_X) || (Old_MY!=Mouse_Y) )
          {
            // Le curseur est sur un nouveau bouton
            if (Indice_bouton!=BOUTON_CHOIX_COL)
            {
              Effacer_curseur();
              Print_dans_menu(TITRE_BOUTON[Indice_bouton],0);
              Afficher_curseur();
            }
            else
            { // Le curseur est-il sur une couleur de la palette?
              int Couleur;
              if ((Couleur=Couleur_palette())!=-1)
              {
                Effacer_curseur();
                strcpy(Chaine,TITRE_BOUTON[Indice_bouton]);
                sprintf(Chaine+strlen(Chaine),"%d (%d,%d,%d)",Couleur,Principal_Palette[Couleur].R,Principal_Palette[Couleur].V,Principal_Palette[Couleur].B);
                for (Temp=strlen(Chaine); Temp<24; Temp++)
                  Chaine[Temp]=' ';
                Chaine[24]=0;
                Print_dans_menu(Chaine,0);
                Afficher_curseur();
              }
              else
              {
                if ( (Old_MX!=Mouse_X) || (Old_MY!=Mouse_Y) )
                {
                  Effacer_curseur();
                  Block(18*Menu_Facteur_X,Menu_Ordonnee_Texte,192*Menu_Facteur_X,Menu_Facteur_Y<<3,CM_Clair);
                  UpdateRect(18*Menu_Facteur_X,Menu_Ordonnee_Texte,192*Menu_Facteur_X,8*Menu_Facteur_Y);
                  Afficher_curseur();
                }
              }
            }
          }
        }
      }

      Indice_bouton_precedent=Indice_bouton;

      // Gestion des clicks
      if (Mouse_K)
      {
        if (Mouse_Y>=Menu_Ordonnee)
        {
          if (Indice_bouton>=0)
          {
            Enclencher_bouton(Indice_bouton,Mouse_K);
            Indice_bouton_precedent=-1;
          }
        }
        else
          if (Loupe_Mode) Deplacer_Split();
      }

    }

    // we need to refresh that one as we may come from a sub window
    Curseur_dans_menu=(Mouse_Y>=Menu_Ordonnee) ||
                      ( (Loupe_Mode) && (Mouse_X>=Principal_Split) &&
                        (Mouse_X<Principal_X_Zoom) );


    // Le curseur se trouve dans l'image
    if ( (!Curseur_dans_menu) && (Menu_visible) && (Old_MY != Mouse_Y || Old_MX != Mouse_X || Touche || Mouse_K)) // On ne met les coordonnées à jour que si l'utilisateur a fait un truc
    {
       if ( (Operation_en_cours!=OPERATION_PIPETTE) && (Operation_en_cours!=OPERATION_REMPLACER) )
       {
          if(Curseur_dans_menu_precedent)
          {
             Print_dans_menu("X:       Y:             ",0);
          }
       }
       else
       {
          if(Curseur_dans_menu_precedent)
          {
             Print_dans_menu("X:       Y:       (    )",0);
          }
       }
       Curseur_dans_menu_precedent = 0;
    }

    if(Curseur_dans_menu)
    {
        Curseur_dans_menu_precedent = 1;
    }
    else
    {
      Blink=Operation[Operation_en_cours][Mouse_K_Unique][Operation_Taille_pile].Effacer_curseur;
 
      if (Blink) Effacer_curseur();
 
      Operation[Operation_en_cours][Mouse_K_Unique][Operation_Taille_pile].Action();

      if (Blink) Afficher_curseur();
    }
    Old_MX=Mouse_X;
    Old_MY=Mouse_Y;
  }
  while (!Sortir_du_programme);
}





//////////////////////////////////////////////////////////////////////////////
//      Différentes fonctions d'affichage utilisées dans les fenêtres       //
//////////////////////////////////////////////////////////////////////////////

//----------------------- Tracer une fenêtre d'options -----------------------

void Ouvrir_fenetre(word width,word height, char * Titre)
// Lors de l'appel à cette procédure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  //word i,j;

  Effacer_curseur();

  Fenetre++;

  Fenetre_Largeur=width;
  Fenetre_Hauteur=height;

  // Positionnement de la fenêtre
  Fenetre_Pos_X=(Largeur_ecran-(width*Menu_Facteur_X))>>1;

  Fenetre_Pos_Y=(Hauteur_ecran-(height*Menu_Facteur_Y))>>1;

  // Sauvegarde de ce que la fenêtre remplace
  Sauve_fond(&(Fond_fenetre[Fenetre-1]), Fenetre_Pos_X, Fenetre_Pos_Y, width, height);

  // Fenêtre grise
  Block(Fenetre_Pos_X+(Menu_Facteur_X<<1),Fenetre_Pos_Y+(Menu_Facteur_Y<<1),(width-4)*Menu_Facteur_X,(height-4)*Menu_Facteur_Y,CM_Clair);

  // -- Cadre de la fenêtre ----- --- -- -  -

  // Cadre noir puis en relief
  Fenetre_Afficher_cadre_mono(0,0,width,height,CM_Noir);
  Fenetre_Afficher_cadre_bombe(1,1,width-2,height-2);

  // Barre sous le titre
  Block(Fenetre_Pos_X+(Menu_Facteur_X<<3),Fenetre_Pos_Y+(11*Menu_Facteur_Y),(width-16)*Menu_Facteur_X,Menu_Facteur_Y,CM_Fonce);
  Block(Fenetre_Pos_X+(Menu_Facteur_X<<3),Fenetre_Pos_Y+(12*Menu_Facteur_Y),(width-16)*Menu_Facteur_X,Menu_Facteur_Y,CM_Blanc);

  Print_dans_fenetre((width-(strlen(Titre)<<3))>>1,3,Titre,CM_Noir,CM_Clair);

  if (Fenetre == 1)
  {
    Menu_visible_avant_fenetre=Menu_visible;
    Menu_visible=0;
    Menu_Ordonnee_avant_fenetre=Menu_Ordonnee;
    Menu_Ordonnee=Hauteur_ecran;
    Forme_curseur_avant_fenetre=Forme_curseur;
    Forme_curseur=FORME_CURSEUR_FLECHE;
    Cacher_pinceau_avant_fenetre=Cacher_pinceau;
    Cacher_pinceau=1;
  }

  // Initialisation des listes de boutons de la fenêtre
  Fenetre_Liste_boutons_normal  =NULL;
  Fenetre_Liste_boutons_palette =NULL;
  Fenetre_Liste_boutons_scroller=NULL;
  Fenetre_Liste_boutons_special =NULL;
  Fenetre_Liste_boutons_dropdown=NULL;
  Nb_boutons_fenetre            =0;

}

//----------------------- Fermer une fenêtre d'options -----------------------

void Fermer_fenetre(void)
// Lors de l'appel à cette procedure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  T_Bouton_normal   * Temp1;
  T_Bouton_palette  * Temp2;
  T_Bouton_scroller * Temp3;
  T_Bouton_special  * Temp4;
  T_Bouton_dropdown * Temp5;

  Effacer_curseur();

  while (Fenetre_Liste_boutons_normal)
  {
    Temp1=Fenetre_Liste_boutons_normal->Next;
    free(Fenetre_Liste_boutons_normal);
    Fenetre_Liste_boutons_normal=Temp1;
  }
  while (Fenetre_Liste_boutons_palette)
  {
    Temp2=Fenetre_Liste_boutons_palette->Next;
    free(Fenetre_Liste_boutons_palette);
    Fenetre_Liste_boutons_palette=Temp2;
  }
  while (Fenetre_Liste_boutons_scroller)
  {
    Temp3=Fenetre_Liste_boutons_scroller->Next;
    free(Fenetre_Liste_boutons_scroller);
    Fenetre_Liste_boutons_scroller=Temp3;
  }
  while (Fenetre_Liste_boutons_special)
  {
    Temp4=Fenetre_Liste_boutons_special->Next;
    free(Fenetre_Liste_boutons_special);
    Fenetre_Liste_boutons_special=Temp4;
  }
  while (Fenetre_Liste_boutons_dropdown)
  {
    Temp5=Fenetre_Liste_boutons_dropdown->Next;
    Fenetre_Dropdown_vider_choix(Fenetre_Liste_boutons_dropdown);
    free(Fenetre_Liste_boutons_dropdown);
    Fenetre_Liste_boutons_dropdown=Temp5;
  }

  if (Fenetre != 1)
  {
    // Restore de ce que la fenêtre cachait
    Restaure_fond(Fond_fenetre[Fenetre-1], Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur, Fenetre_Hauteur);
    Fond_fenetre[Fenetre-1]=NULL;
    UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Fenetre_Largeur*Menu_Facteur_X,Fenetre_Hauteur*Menu_Facteur_Y);
    Fenetre--;
  }
  else
  {
    free(Fond_fenetre[Fenetre-1]);
    Fond_fenetre[Fenetre-1]=NULL;
    Fenetre--;
  
    Cacher_pinceau=Cacher_pinceau_avant_fenetre;
  
    Calculer_coordonnees_pinceau();
  
    Menu_Ordonnee=Menu_Ordonnee_avant_fenetre;
    Menu_visible=Menu_visible_avant_fenetre;
    Forme_curseur=Forme_curseur_avant_fenetre;
    
    Afficher_ecran();
    Afficher_menu();
  }

  Touche=0;
  Mouse_K=0;
  
  Old_MX = -1;
  Old_MY = -1;

}


//---------------- Dessiner un bouton normal dans une fenêtre ----------------

void Fenetre_Dessiner_bouton_normal(word Pos_X,word Pos_Y,word width,word height,
                                    char * Titre,byte Lettre_soulignee,byte clickable)
{
  byte Couleur_titre;
  word Pos_texte_X,Pos_texte_Y;

  if (clickable)
  {
    Fenetre_Afficher_cadre_bombe(Pos_X,Pos_Y,width,height);
    Fenetre_Afficher_cadre_general(Pos_X-1,Pos_Y-1,width+2,height+2,CM_Noir,CM_Noir,CM_Fonce,CM_Fonce,CM_Fonce);
    Couleur_titre=CM_Noir;
  }
  else
  {
    Fenetre_Afficher_cadre_bombe(Pos_X,Pos_Y,width,height);
    Fenetre_Afficher_cadre_mono(Pos_X-1,Pos_Y-1,width+2,height+2,CM_Clair);
    Couleur_titre=CM_Fonce;
  }

  Pos_texte_X=Pos_X+( (width-(strlen(Titre)<<3)+1) >>1 );
  Pos_texte_Y=Pos_Y+((height-7)>>1);
  Print_dans_fenetre(Pos_texte_X,Pos_texte_Y,Titre,Couleur_titre,CM_Clair);

  if (Lettre_soulignee)
    Block(Fenetre_Pos_X+((Pos_texte_X+((Lettre_soulignee-1)<<3))*Menu_Facteur_X),
          Fenetre_Pos_Y+((Pos_texte_Y+8)*Menu_Facteur_Y),
          Menu_Facteur_X<<3,Menu_Facteur_Y,CM_Fonce);
}


// -- Bouton normal enfoncé dans la fenêtre --
void Fenetre_Enfoncer_bouton_normal(word Pos_X,word Pos_Y,word width,word height)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,width,height,CM_Fonce,CM_Noir,CM_Fonce,CM_Fonce,CM_Noir);
  UpdateRect(Fenetre_Pos_X+Pos_X*Menu_Facteur_X, Fenetre_Pos_Y+Pos_Y*Menu_Facteur_Y, width*Menu_Facteur_X, height*Menu_Facteur_Y);
}

// -- Bouton normal désenfoncé dans la fenêtre --
void Fenetre_Desenfoncer_bouton_normal(word Pos_X,word Pos_Y,word width,word height)
{
  Fenetre_Afficher_cadre_bombe(Pos_X,Pos_Y,width,height);
  UpdateRect(Fenetre_Pos_X+Pos_X*Menu_Facteur_X, Fenetre_Pos_Y+Pos_Y*Menu_Facteur_Y, width*Menu_Facteur_X, height*Menu_Facteur_Y);
}


//--------------- Dessiner un bouton palette dans une fenêtre ----------------
void Fenetre_Dessiner_bouton_palette(word Pos_X,word Pos_Y)
{
  word Couleur;

  for (Couleur=0; Couleur<=255; Couleur++)
    Block( Fenetre_Pos_X+((((Couleur >> 4)*10)+Pos_X+6)*Menu_Facteur_X),Fenetre_Pos_Y+((((Couleur & 15)*5)+Pos_Y+3)*Menu_Facteur_Y),Menu_Facteur_X*5,Menu_Facteur_Y*5,Couleur);

  Fenetre_Afficher_cadre(Pos_X,Pos_Y,164,86);
}


// -------------------- Effacer les TAGs sur les palette ---------------------
// Cette fonct° ne sert plus que lorsqu'on efface les tags dans le menu Spray.
void Fenetre_Effacer_tags(void)
{
  word Origine_X;
  word Origine_Y;
  word Pos_X;
  word Pos_fenetre_X;
  //word Pos_fenetre_Y;

  Origine_X=Fenetre_Pos_X+(Fenetre_Liste_boutons_palette->Pos_X+3)*Menu_Facteur_X;
  Origine_Y=Fenetre_Pos_Y+(Fenetre_Liste_boutons_palette->Pos_Y+3)*Menu_Facteur_Y;
  for (Pos_X=0,Pos_fenetre_X=Origine_X;Pos_X<16;Pos_X++,Pos_fenetre_X+=(Menu_Facteur_X*10))
    Block(Pos_fenetre_X,Origine_Y,Menu_Facteur_X*3,Menu_Facteur_Y*80,CM_Clair);
  UpdateRect(Origine_X,Origine_Y,ToWinL(160),ToWinH(80));
}


// ---- Tracer les TAGs sur les palettes du menu Palette ou du menu Shade ----
void Tagger_intervalle_palette(byte start,byte end)
{
  word Origine_X;
  word Origine_Y;
  //word Pos_X;
  word Pos_Y;
  //word Pos_fenetre_X;
  word Pos_fenetre_Y;
  word Indice;

  // On efface les anciens TAGs
  for (Indice=0;Indice<=start;Indice++)
    Block(Fenetre_Pos_X+(Fenetre_Liste_boutons_palette->Pos_X+3+((Indice>>4)*10))*Menu_Facteur_X,
          Fenetre_Pos_Y+(Fenetre_Liste_boutons_palette->Pos_Y+3+((Indice&15)* 5))*Menu_Facteur_Y,
          Menu_Facteur_X*3,Menu_Facteur_Y*5,CM_Clair);

  for (Indice=end;Indice<256;Indice++)
    Block(Fenetre_Pos_X+(Fenetre_Liste_boutons_palette->Pos_X+3+((Indice>>4)*10))*Menu_Facteur_X,
          Fenetre_Pos_Y+(Fenetre_Liste_boutons_palette->Pos_Y+3+((Indice&15)* 5))*Menu_Facteur_Y,
          Menu_Facteur_X*3,Menu_Facteur_Y*5,CM_Clair);

  // On affiche le 1er TAG
  Origine_X=(Fenetre_Liste_boutons_palette->Pos_X+3)+(start>>4)*10;
  Origine_Y=(Fenetre_Liste_boutons_palette->Pos_Y+3)+(start&15)* 5;
  for (Pos_Y=0,Pos_fenetre_Y=Origine_Y  ;Pos_Y<5;Pos_Y++,Pos_fenetre_Y++)
    Pixel_dans_fenetre(Origine_X  ,Pos_fenetre_Y,CM_Noir);
  for (Pos_Y=0,Pos_fenetre_Y=Origine_Y+1;Pos_Y<3;Pos_Y++,Pos_fenetre_Y++)
    Pixel_dans_fenetre(Origine_X+1,Pos_fenetre_Y,CM_Noir);
  Pixel_dans_fenetre(Origine_X+2,Origine_Y+2,CM_Noir);

  if (start!=end)
  {
    // On complète le 1er TAG
    Pixel_dans_fenetre(Origine_X+1,Origine_Y+4,CM_Noir);

    // On affiche le 2ème TAG
    Origine_X=(Fenetre_Liste_boutons_palette->Pos_X+3)+(end>>4)*10;
    Origine_Y=(Fenetre_Liste_boutons_palette->Pos_Y+3)+(end&15)* 5;
    for (Pos_Y=0,Pos_fenetre_Y=Origine_Y; Pos_Y<5; Pos_Y++,Pos_fenetre_Y++)
      Pixel_dans_fenetre(Origine_X  ,Pos_fenetre_Y,CM_Noir);
    for (Pos_Y=0,Pos_fenetre_Y=Origine_Y; Pos_Y<4; Pos_Y++,Pos_fenetre_Y++)
      Pixel_dans_fenetre(Origine_X+1,Pos_fenetre_Y,CM_Noir);
    Pixel_dans_fenetre(Origine_X+2,Origine_Y+2,CM_Noir);

    // On TAG toutes les couleurs intermédiaires
    for (Indice=start+1;Indice<end;Indice++)
    {
      Block(Fenetre_Pos_X+(Fenetre_Liste_boutons_palette->Pos_X+3+((Indice>>4)*10))*Menu_Facteur_X,
            Fenetre_Pos_Y+(Fenetre_Liste_boutons_palette->Pos_Y+3+((Indice&15)* 5))*Menu_Facteur_Y,
            Menu_Facteur_X*2,Menu_Facteur_Y*5,CM_Noir);
      // On efface l'éventuelle pointe d'une ancienne extrémité de l'intervalle
      Pixel_dans_fenetre(Fenetre_Liste_boutons_palette->Pos_X+5+((Indice>>4)*10),
                         Fenetre_Liste_boutons_palette->Pos_Y+5+((Indice&15)* 5),
                         CM_Clair);
    }


  }

  UpdateRect(ToWinX(Fenetre_Liste_boutons_palette->Pos_X+3),ToWinY(Fenetre_Liste_boutons_palette->Pos_Y+3),ToWinL(12*16),ToWinH(5*16));

}


//------------------ Dessiner un scroller dans une fenêtre -------------------

void Calculer_hauteur_curseur_jauge(T_Bouton_scroller * button)
{
  if (button->Nb_elements>button->Nb_visibles)
  {
    button->Hauteur_curseur=(button->Nb_visibles*(button->Height-24))/button->Nb_elements;
    if (!(button->Hauteur_curseur))
      button->Hauteur_curseur=1;
  }
  else
  {
    button->Hauteur_curseur=button->Height-24;
  }
}

void Fenetre_Dessiner_jauge(T_Bouton_scroller * button)
{
  word Position_curseur_jauge;

  Position_curseur_jauge=button->Pos_Y+12;

  Block(Fenetre_Pos_X+(button->Pos_X*Menu_Facteur_X),
        Fenetre_Pos_Y+(Position_curseur_jauge*Menu_Facteur_Y),
        11*Menu_Facteur_X,(button->Height-24)*Menu_Facteur_Y,CM_Noir/*CM_Fonce*/);

  if (button->Nb_elements>button->Nb_visibles)
    Position_curseur_jauge+=Round_div(button->Position*(button->Height-24-button->Hauteur_curseur),button->Nb_elements-button->Nb_visibles);

  Block(Fenetre_Pos_X+(button->Pos_X*Menu_Facteur_X),
        Fenetre_Pos_Y+(Position_curseur_jauge*Menu_Facteur_Y),
        11*Menu_Facteur_X,button->Hauteur_curseur*Menu_Facteur_Y,CM_Clair/*CM_Blanc*/);

  UpdateRect(Fenetre_Pos_X+(button->Pos_X*Menu_Facteur_X),
        Fenetre_Pos_Y+button->Pos_Y*Menu_Facteur_Y,
        11*Menu_Facteur_X,(button->Height)*Menu_Facteur_Y);
}

void Fenetre_Dessiner_bouton_scroller(T_Bouton_scroller * button)
{
  Fenetre_Afficher_cadre_general(button->Pos_X-1,button->Pos_Y-1,13,button->Height+2,CM_Noir,CM_Noir,CM_Fonce,CM_Fonce,CM_Fonce);
  Fenetre_Afficher_cadre_mono(button->Pos_X-1,button->Pos_Y+11,13,button->Height-22,CM_Noir);
  Fenetre_Afficher_cadre_bombe(button->Pos_X,button->Pos_Y,11,11);
  Fenetre_Afficher_cadre_bombe(button->Pos_X,button->Pos_Y+button->Height-11,11,11);
  Print_dans_fenetre(button->Pos_X+2,button->Pos_Y+2,"\030",CM_Noir,CM_Clair);
  Print_dans_fenetre(button->Pos_X+2,button->Pos_Y+button->Height-9,"\031",CM_Noir,CM_Clair);
  Fenetre_Dessiner_jauge(button);
}


//--------------- Dessiner une zone de saisie dans une fenêtre ---------------

void Fenetre_Dessiner_bouton_saisie(word Pos_X,word Pos_Y,word Largeur_en_caracteres)
{
  Fenetre_Afficher_cadre_creux(Pos_X,Pos_Y,(Largeur_en_caracteres<<3)+3,11);
}


//------------ Modifier le contenu (caption) d'une zone de saisie ------------

void Fenetre_Contenu_bouton_saisie(T_Bouton_special * button, char * content)
{
  Print_dans_fenetre_limite(button->Pos_X+2,button->Pos_Y+2,content,button->Width/8,CM_Noir,CM_Clair);
}

//------------ Effacer le contenu (caption) d'une zone de saisie ------------

void Fenetre_Effacer_bouton_saisie(T_Bouton_special * button)
{
  Block((button->Pos_X+2)*Menu_Facteur_X+Fenetre_Pos_X,(button->Pos_Y+2)*Menu_Facteur_Y+Fenetre_Pos_Y,(button->Width/8)*8*Menu_Facteur_X,8*Menu_Facteur_Y,CM_Clair);
  UpdateRect((button->Pos_X+2)*Menu_Facteur_X+Fenetre_Pos_X,(button->Pos_Y+2)*Menu_Facteur_Y+Fenetre_Pos_Y,button->Width/8*8*Menu_Facteur_X,8*Menu_Facteur_Y);
}


//------ Rajout d'un bouton à la liste de ceux présents dans la fenêtre ------

T_Bouton_normal * Fenetre_Definir_bouton_normal(word Pos_X, word Pos_Y,
                                   word width, word height,
                                   char * Titre, byte Lettre_soulignee,
                                   byte clickable, word Raccourci)
{
  T_Bouton_normal * Temp=NULL;

  Nb_boutons_fenetre++;

  if (clickable)
  {
    Temp=(T_Bouton_normal *)malloc(sizeof(T_Bouton_normal));
    Temp->Numero   =Nb_boutons_fenetre;
    Temp->Pos_X    =Pos_X;
    Temp->Pos_Y    =Pos_Y;
    Temp->Width  =width;
    Temp->Height  =height;
    Temp->Raccourci=Raccourci;
    Temp->Repetable=0;

    Temp->Next=Fenetre_Liste_boutons_normal;
    Fenetre_Liste_boutons_normal=Temp;
  }

  Fenetre_Dessiner_bouton_normal(Pos_X,Pos_Y,width,height,Titre,Lettre_soulignee,clickable);
  return Temp;
}
//------ Rajout d'un bouton à la liste de ceux présents dans la fenêtre ------

T_Bouton_normal * Fenetre_Definir_bouton_repetable(word Pos_X, word Pos_Y,
                                   word width, word height,
                                   char * Titre, byte Lettre_soulignee,
                                   byte clickable, word Raccourci)
{
  T_Bouton_normal * Temp=NULL;

  Nb_boutons_fenetre++;

  if (clickable)
  {
    Temp=(T_Bouton_normal *)malloc(sizeof(T_Bouton_normal));
    Temp->Numero   =Nb_boutons_fenetre;
    Temp->Pos_X    =Pos_X;
    Temp->Pos_Y    =Pos_Y;
    Temp->Width  =width;
    Temp->Height  =height;
    Temp->Raccourci=Raccourci;
    Temp->Repetable=1;

    Temp->Next=Fenetre_Liste_boutons_normal;
    Fenetre_Liste_boutons_normal=Temp;
  }

  Fenetre_Dessiner_bouton_normal(Pos_X,Pos_Y,width,height,Titre,Lettre_soulignee,clickable);
  return Temp;
}

T_Bouton_palette * Fenetre_Definir_bouton_palette(word Pos_X, word Pos_Y)
{
  T_Bouton_palette * Temp;

  Temp=(T_Bouton_palette *)malloc(sizeof(T_Bouton_palette));
  Temp->Numero   =++Nb_boutons_fenetre;
  Temp->Pos_X    =Pos_X;
  Temp->Pos_Y    =Pos_Y;

  Temp->Next=Fenetre_Liste_boutons_palette;
  Fenetre_Liste_boutons_palette=Temp;

  Fenetre_Dessiner_bouton_palette(Pos_X,Pos_Y);
  return Temp;
}


T_Bouton_scroller * Fenetre_Definir_bouton_scroller(word Pos_X, word Pos_Y,
                                     word height,
                                     word nb_elements,
                                     word nb_elements_visible,
                                     word Position_initiale)
{
  T_Bouton_scroller * Temp;

  Temp=(T_Bouton_scroller *)malloc(sizeof(T_Bouton_scroller));
  Temp->Numero        =++Nb_boutons_fenetre;
  Temp->Pos_X         =Pos_X;
  Temp->Pos_Y         =Pos_Y;
  Temp->Height       =height;
  Temp->Nb_elements   =nb_elements;
  Temp->Nb_visibles   =nb_elements_visible;
  Temp->Position      =Position_initiale;
  Calculer_hauteur_curseur_jauge(Temp);

  Temp->Next=Fenetre_Liste_boutons_scroller;
  Fenetre_Liste_boutons_scroller=Temp;

  Fenetre_Dessiner_bouton_scroller(Temp);
  return Temp;
}


T_Bouton_special * Fenetre_Definir_bouton_special(word Pos_X,word Pos_Y,word width,word height)
{
  T_Bouton_special * Temp;

  Temp=(T_Bouton_special *)malloc(sizeof(T_Bouton_special));
  Temp->Numero   =++Nb_boutons_fenetre;
  Temp->Pos_X    =Pos_X;
  Temp->Pos_Y    =Pos_Y;
  Temp->Width  =width;
  Temp->Height  =height;

  Temp->Next=Fenetre_Liste_boutons_special;
  Fenetre_Liste_boutons_special=Temp;
  return Temp;
}


T_Bouton_special * Fenetre_Definir_bouton_saisie(word Pos_X,word Pos_Y,word Largeur_en_caracteres)
{
  T_Bouton_special *Temp;
  Temp=Fenetre_Definir_bouton_special(Pos_X,Pos_Y,(Largeur_en_caracteres<<3)+3,11);
  Fenetre_Dessiner_bouton_saisie(Pos_X,Pos_Y,Largeur_en_caracteres);
  return Temp;
}

T_Bouton_dropdown * Fenetre_Definir_bouton_dropdown(word Pos_X,word Pos_Y,word width,word height,word dropdown_width,char *label,byte display_choice,byte display_centered,byte display_arrow,byte active_button)
{
  T_Bouton_dropdown *Temp;
  
  Temp=(T_Bouton_dropdown *)malloc(sizeof(T_Bouton_dropdown));
  Temp->Numero       =++Nb_boutons_fenetre;
  Temp->Pos_X        =Pos_X;
  Temp->Pos_Y        =Pos_Y;
  Temp->Width      =width;
  Temp->Height      =height;
  Temp->Affiche_choix =display_choice;
  Temp->Premier_choix=NULL;
  Temp->Largeur_choix=dropdown_width?dropdown_width:width;
  Temp->Affiche_centre=display_centered;
  Temp->Affiche_fleche=display_arrow;
  Temp->Bouton_actif=active_button;

  Temp->Next=Fenetre_Liste_boutons_dropdown;
  Fenetre_Liste_boutons_dropdown=Temp;
  Fenetre_Dessiner_bouton_normal(Pos_X,Pos_Y,width,height,"",-1,1);
  if (label && label[0])
    Print_dans_fenetre(Temp->Pos_X+2,Temp->Pos_Y+(Temp->Height-7)/2,label,CM_Noir,CM_Clair);
  if (display_arrow)
    Fenetre_Afficher_sprite_drive(Temp->Pos_X+Temp->Width-10,Temp->Pos_Y+(Temp->Height-7)/2,6);
  
  return Temp;
}

// Ajoute un choix à une dropdown. Le libellé est seulement référencé,
// il doit pointer sur une zone qui doit être encore valide à la fermeture 
// de la fenêtre (comprise).
void Fenetre_Dropdown_choix(T_Bouton_dropdown * dropdown, word Numero, const char *label)
{
  T_Dropdown_choix *Temp;
  T_Dropdown_choix *last;
  
  Temp=(T_Dropdown_choix *)malloc(sizeof(T_Dropdown_choix));
  Temp->Numero =Numero;
  Temp->Label=label;
  Temp->Next=NULL;

  last=dropdown->Premier_choix;
  if (last)
  {
    // On cherche le dernier élément
    for (;last->Next;last=last->Next)
      ;
    last->Next=Temp;
  }
  else
  {
    dropdown->Premier_choix=Temp;
  }
}

// ------------- Suppression de tous les choix d'une dropdown ---------
void Fenetre_Dropdown_vider_choix(T_Bouton_dropdown * dropdown)
{
  T_Dropdown_choix * Choix_suivant;
    while (dropdown->Premier_choix)
    {
      Choix_suivant=dropdown->Premier_choix->Next;
      free(dropdown->Premier_choix);
      dropdown->Premier_choix=Choix_suivant;
    }
}

//----------------------- Ouverture d'un pop-up -----------------------

void Ouvrir_popup(word Pos_X, word Pos_Y, word width,word height)
// Lors de l'appel à cette procédure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.

// Note : les pop-ups sont gérés comme s'ils étaient des sous-fenêtres, ils ont donc leur propre boucle d'évènements et tout, on peut ajouter des widgets dedans, ...
// Les différences sont surtout graphiques :
    // -Possibilité de préciser la position XY
    // -Pas de titre
    // -Pas de cadre en relief mais seulement un plat, et il est blanc au lieu de noir.
{
  Fenetre++;

  Fenetre_Largeur=width;
  Fenetre_Hauteur=height;
  Fenetre_Pos_X=Pos_X;
  Fenetre_Pos_Y=Pos_Y;

  // Sauvegarde de ce que la fenêtre remplace
  Sauve_fond(&(Fond_fenetre[Fenetre-1]), Fenetre_Pos_X, Fenetre_Pos_Y, width, height);

/*
  // Fenêtre grise
  Block(Fenetre_Pos_X+1*Menu_Facteur_X,
        Fenetre_Pos_Y+1*Menu_Facteur_Y,
        (width-2)*Menu_Facteur_X,(height-2)*Menu_Facteur_Y,CM_Clair);

  // Cadre noir puis en relief
  Fenetre_Afficher_cadre_mono(0,0,width,height,CM_Blanc);
*/
  if (Fenetre == 1)
  {
    Menu_visible_avant_fenetre=Menu_visible;
    Menu_visible=0;
    Menu_Ordonnee_avant_fenetre=Menu_Ordonnee;
    Menu_Ordonnee=Hauteur_ecran;
    Forme_curseur_avant_fenetre=Forme_curseur;
    Forme_curseur=FORME_CURSEUR_FLECHE;
    Cacher_pinceau_avant_fenetre=Cacher_pinceau;
    Cacher_pinceau=1;
  }

  // Initialisation des listes de boutons de la fenêtre
  Fenetre_Liste_boutons_normal  =NULL;
  Fenetre_Liste_boutons_palette =NULL;
  Fenetre_Liste_boutons_scroller=NULL;
  Fenetre_Liste_boutons_special =NULL;
  Fenetre_Liste_boutons_dropdown =NULL;
  Nb_boutons_fenetre            =0;

}

//----------------------- Fermer une fenêtre d'options -----------------------

void Fermer_popup(void)
// Lors de l'appel à cette procedure, la souris doit être affichée.
// En sortie de cette procedure, la souris est effacée.
{
  T_Bouton_normal   * Temp1;
  T_Bouton_palette  * Temp2;
  T_Bouton_scroller * Temp3;
  T_Bouton_special  * Temp4;
  T_Bouton_dropdown  * Temp5;

  Effacer_curseur();

  while (Fenetre_Liste_boutons_normal)
  {
    Temp1=Fenetre_Liste_boutons_normal->Next;
    free(Fenetre_Liste_boutons_normal);
    Fenetre_Liste_boutons_normal=Temp1;
  }
  while (Fenetre_Liste_boutons_palette)
  {
    Temp2=Fenetre_Liste_boutons_palette->Next;
    free(Fenetre_Liste_boutons_palette);
    Fenetre_Liste_boutons_palette=Temp2;
  }
  while (Fenetre_Liste_boutons_scroller)
  {
    Temp3=Fenetre_Liste_boutons_scroller->Next;
    free(Fenetre_Liste_boutons_scroller);
    Fenetre_Liste_boutons_scroller=Temp3;
  }
  while (Fenetre_Liste_boutons_special)
  {
    Temp4=Fenetre_Liste_boutons_special->Next;
    free(Fenetre_Liste_boutons_special);
    Fenetre_Liste_boutons_special=Temp4;
  }
  while (Fenetre_Liste_boutons_dropdown)
  {
    Fenetre_Dropdown_vider_choix(Fenetre_Liste_boutons_dropdown);
    Temp5=Fenetre_Liste_boutons_dropdown->Next;
    free(Fenetre_Liste_boutons_dropdown);
    Fenetre_Liste_boutons_dropdown=Temp5;
  }

  if (Fenetre != 1)
  {
    // Restore de ce que la fenêtre cachait
    Restaure_fond(Fond_fenetre[Fenetre-1], Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur, Fenetre_Hauteur);
    Fond_fenetre[Fenetre-1]=NULL;
    UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Fenetre_Largeur*Menu_Facteur_X,Fenetre_Hauteur*Menu_Facteur_Y);
    Fenetre--;
  }
  else
  {
    free(Fond_fenetre[Fenetre-1]);
    Fenetre--;
  
    Cacher_pinceau=Cacher_pinceau_avant_fenetre;
  
    Calculer_coordonnees_pinceau();
  
    Menu_Ordonnee=Menu_Ordonnee_avant_fenetre;
    Menu_visible=Menu_visible_avant_fenetre;
    Forme_curseur=Forme_curseur_avant_fenetre;
    
    Afficher_ecran();
    Afficher_menu();
  }

  Touche=0;
  Mouse_K=0;
  
  Old_MX = -1;
  Old_MY = -1;


}
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//       Mini-MOTEUR utilisé dans les fenêtres (menus des boutons...)       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


// -- Indique si on a cliqué dans une zone définie par deux points extremes --
byte Fenetre_click_dans_zone(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y)
{
  short Pos_X,Pos_Y;

  Pos_X=((short)Mouse_X-Fenetre_Pos_X)/Menu_Facteur_X;
  Pos_Y=((short)Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y;

  return ((Pos_X>=Debut_X) &&
          (Pos_Y>=Debut_Y) &&
          (Pos_X<=Fin_X)   &&
          (Pos_Y<=Fin_Y));
}


// --- Attend que l'on clique dans la palette pour renvoyer la couleur choisie
// ou bien renvoie -1 si on a annulé l'action pas click-droit ou Escape ------
short Attendre_click_dans_palette(T_Bouton_palette * button)
{
  short Debut_X=button->Pos_X+5;
  short Debut_Y=button->Pos_Y+3;
  short Fin_X  =button->Pos_X+160;
  short Fin_Y  =button->Pos_Y+82;
  byte  Couleur_choisie;
  byte  Ancien_Cacher_curseur;
  byte  Ancien_Loupe_Mode;

  Effacer_curseur();
  Ancien_Cacher_curseur=Cacher_curseur;
  Ancien_Loupe_Mode=Loupe_Mode;
  Loupe_Mode=0;
  Cacher_curseur=0;
  Forme_curseur=FORME_CURSEUR_CIBLE;
  Afficher_curseur();

  for (;;)
  {
    while(!Get_input())Wait_VBL();

    if (Mouse_K==A_GAUCHE)
    {
      if (Fenetre_click_dans_zone(Debut_X,Debut_Y,Fin_X,Fin_Y))
      {
        Effacer_curseur();
        Couleur_choisie=(((Mouse_X-Fenetre_Pos_X)/Menu_Facteur_X)-(button->Pos_X+2)) / 10 * 16 +
        (((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-(button->Pos_Y+3)) / 5;
        Forme_curseur=FORME_CURSEUR_FLECHE;
        Cacher_curseur=Ancien_Cacher_curseur;
        Loupe_Mode=Ancien_Loupe_Mode;
        Afficher_curseur();
        return Couleur_choisie;
      }
      if ((Mouse_X<Fenetre_Pos_X) || (Mouse_Y<Fenetre_Pos_Y) ||
          (Mouse_X>=Fenetre_Pos_X+(Fenetre_Largeur*Menu_Facteur_X)) ||
          (Mouse_Y>=Fenetre_Pos_Y+(Fenetre_Hauteur*Menu_Facteur_Y)) )
      {
        Effacer_curseur();
        Couleur_choisie=Lit_pixel(Mouse_X,Mouse_Y);
        Forme_curseur=FORME_CURSEUR_FLECHE;
        Cacher_curseur=Ancien_Cacher_curseur;
        Loupe_Mode=Ancien_Loupe_Mode;
        Afficher_curseur();
        return Couleur_choisie;
      }
    }

    if ((Mouse_K==A_DROITE) || (Touche==TOUCHE_ESC))
    {
      Effacer_curseur();
      Forme_curseur=FORME_CURSEUR_FLECHE;
      Cacher_curseur=Ancien_Cacher_curseur;
      Loupe_Mode=Ancien_Loupe_Mode;
      Afficher_curseur();
      return -1;
    }
  }
}



// -------------- Récupération d'une couleur derrière un menu ----------------
void Recuperer_couleur_derriere_fenetre(byte * Couleur, byte * click)
{
  short width=Fenetre_Largeur*Menu_Facteur_X;
  short height=Fenetre_Hauteur*Menu_Facteur_Y;
  short Ancien_X=-1;
  short Ancien_Y=-1;
  short Indice;
  short a,b,c,d; // Variables temporaires et multitâches...
  byte * Buffer;
  char Chaine[25];
  byte Cacher_curseur_avant_recuperation;


  if ((Buffer=(byte *) malloc(width*height)))
  {
    Effacer_curseur();

    Cacher_curseur_avant_recuperation=Cacher_curseur;
    Cacher_curseur=0;

    for (Indice=0; Indice<height; Indice++)
      Lire_ligne(Fenetre_Pos_X,Fenetre_Pos_Y+Indice,width,Buffer+((int)Indice*width*Pixel_width));
    a=Menu_Ordonnee;
    Menu_Ordonnee=Menu_Ordonnee_avant_fenetre;
    b=Menu_visible;
    Menu_visible=Menu_visible_avant_fenetre;
    Afficher_ecran();
    Afficher_menu();
    Menu_Ordonnee=a;
    Menu_visible=b;

    Forme_curseur=FORME_CURSEUR_CIBLE_PIPETTE;
    b=Cacher_pinceau;
    Cacher_pinceau=1;
    c=-1; // Couleur pointée: au début aucune, comme ça on initialise tout
    if (Menu_visible_avant_fenetre)
      Print_dans_menu(TITRE_BOUTON[BOUTON_CHOIX_COL],0);

    Afficher_curseur();

    do
    {
      if(!Get_input())Wait_VBL();

      if ((Mouse_X!=Ancien_X) || (Mouse_Y!=Ancien_Y))
      {
        Effacer_curseur();
        a=Lit_pixel(Mouse_X,Mouse_Y);
        if (a!=c)
        {
          c=a; // Mise à jour de la couleur pointée
          if (Menu_visible_avant_fenetre)
          {
            sprintf(Chaine,"%d",a);
            d=strlen(Chaine);
            strcat(Chaine,"   (");
            sprintf(Chaine+strlen(Chaine),"%d",Principal_Palette[a].R);
            strcat(Chaine,",");
            sprintf(Chaine+strlen(Chaine),"%d",Principal_Palette[a].V);
            strcat(Chaine,",");
            sprintf(Chaine+strlen(Chaine),"%d",Principal_Palette[a].B);
            strcat(Chaine,")");
            a=24-d;
            for (Indice=strlen(Chaine); Indice<a; Indice++)
              Chaine[Indice]=' ';
            Chaine[a]=0;
            Print_dans_menu(Chaine,strlen(TITRE_BOUTON[BOUTON_CHOIX_COL]));

            Print_general((26+((d+strlen(TITRE_BOUTON[BOUTON_CHOIX_COL]))<<3))*Menu_Facteur_X,
                          Menu_Ordonnee_Texte," ",0,c);
          }
        }
        Afficher_curseur();
      }

      Ancien_X=Mouse_X;
      Ancien_Y=Mouse_Y;
    } while (!(Mouse_K || (Touche==TOUCHE_ESC)));

    if (Mouse_K)
    {
      Effacer_curseur();
      *click=Mouse_K;
      *Couleur=Lit_pixel(Mouse_X,Mouse_Y);
    }
    else
    {
      *click=0;
      Effacer_curseur();
    }

    for (Indice=0; Indice<height; Indice++)
      Afficher_ligne(Fenetre_Pos_X,Fenetre_Pos_Y+Indice,width,Buffer+((int)Indice*width));
    UpdateRect(Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur*Menu_Facteur_X, Fenetre_Hauteur*Menu_Facteur_Y);
    Forme_curseur=FORME_CURSEUR_FLECHE;
    Cacher_pinceau=b;
    Cacher_curseur=Cacher_curseur_avant_recuperation;
    Afficher_curseur();

    free(Buffer);
  }
  else
  {
    Erreur(0);
  }
}



// ------------ Opération de déplacement de la fenêtre à l'écran -------------
void Deplacer_fenetre(short Dx, short Dy)
{
  short Nouveau_X=Mouse_X-Dx;
  short Nouveau_Y=Mouse_Y-Dy;
  short Ancien_X;
  short Ancien_Y;
  short width=Fenetre_Largeur*Menu_Facteur_X;
  short height=Fenetre_Hauteur*Menu_Facteur_Y;
  short a;
  byte  b;
  byte  *Buffer=NULL;

  Effacer_curseur();

  Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y,width);
  Ligne_verticale_XOR(Nouveau_X,Nouveau_Y+1,height-2);
  Ligne_verticale_XOR(Nouveau_X+width-1,Nouveau_Y+1,height-2);
  Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y+height-1,width);
  UpdateRect(Nouveau_X,Nouveau_Y,width,height);
  Forme_curseur=FORME_CURSEUR_MULTIDIRECTIONNEL;
  Afficher_curseur();

  while (Mouse_K)
  {
    Ancien_X=Nouveau_X;
    Ancien_Y=Nouveau_Y;

    while(!Get_input() && Nouveau_X==Mouse_X-Dx && Nouveau_Y==Mouse_Y-Dy) Wait_VBL();

    Nouveau_X=Mouse_X-Dx;

    if (Nouveau_X<0)
    {
      Nouveau_X=0;
      Dx = Mouse_X;
    }
    if (Nouveau_X>Largeur_ecran-width)
    {
      Nouveau_X=Largeur_ecran-width;
      Dx = Mouse_X - Nouveau_X;
    }

    Nouveau_Y=Mouse_Y-Dy;

    if (Nouveau_Y<0)
    {
      Nouveau_Y=0;
      Dy = Mouse_Y;
    }
    if (Nouveau_Y>Hauteur_ecran-height)
    {
      Nouveau_Y=Hauteur_ecran-height;
      Dy = Mouse_Y - Nouveau_Y;
    }

    if ((Nouveau_X!=Ancien_X) || (Nouveau_Y!=Ancien_Y))
    {
      Effacer_curseur();

      Ligne_horizontale_XOR(Ancien_X,Ancien_Y,width);
      Ligne_verticale_XOR(Ancien_X,Ancien_Y+1,height-2);
      Ligne_verticale_XOR(Ancien_X+width-1,Ancien_Y+1,height-2);
      Ligne_horizontale_XOR(Ancien_X,Ancien_Y+height-1,width);

      Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y,width);
      Ligne_verticale_XOR(Nouveau_X,Nouveau_Y+1,height-2);
      Ligne_verticale_XOR(Nouveau_X+width-1,Nouveau_Y+1,height-2);
      Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y+height-1,width);

      Afficher_curseur();
      UpdateRect(Ancien_X,Ancien_Y,width,height);
      UpdateRect(Nouveau_X,Nouveau_Y,width,height);
    }
  }

  Effacer_curseur();
  Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y,width);
  Ligne_verticale_XOR(Nouveau_X,Nouveau_Y+1,height-2);
  Ligne_verticale_XOR(Nouveau_X+width-1,Nouveau_Y+1,height-2);
  Ligne_horizontale_XOR(Nouveau_X,Nouveau_Y+height-1,width);

  if ((Nouveau_X!=Fenetre_Pos_X)
   || (Nouveau_Y!=Fenetre_Pos_Y))
  {
    a=Menu_Ordonnee;
    Menu_Ordonnee=Menu_Ordonnee_avant_fenetre;
    b=Menu_visible;
    Menu_visible=Menu_visible_avant_fenetre;
    //Afficher_ecran();
    //Afficher_menu();
    Menu_Ordonnee=a;
    Menu_visible=b;

    // Sauvegarde du contenu actuel de la fenêtre
    Sauve_fond(&Buffer, Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur, Fenetre_Hauteur);
    
    // Restore de ce que la fenêtre cachait
    Restaure_fond(Fond_fenetre[Fenetre-1], Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur, Fenetre_Hauteur);
    Fond_fenetre[Fenetre-1] = NULL;

    // Sauvegarde de ce que la fenêtre remplace
    Sauve_fond(&(Fond_fenetre[Fenetre-1]), Nouveau_X, Nouveau_Y, Fenetre_Largeur, Fenetre_Hauteur);

    // Raffichage de la fenêtre
    Restaure_fond(Buffer, Nouveau_X, Nouveau_Y, Fenetre_Largeur, Fenetre_Hauteur);
    Buffer = NULL;

    // Mise à jour du rectangle englobant
    UpdateRect(
      (Nouveau_X>Fenetre_Pos_X)?Fenetre_Pos_X:Nouveau_X,
      (Nouveau_Y>Fenetre_Pos_Y)?Fenetre_Pos_Y:Nouveau_Y,
      ((Nouveau_X>Fenetre_Pos_X)?(Nouveau_X-Fenetre_Pos_X):(Fenetre_Pos_X-Nouveau_X)) + Fenetre_Largeur*Menu_Facteur_X,
      ((Nouveau_Y>Fenetre_Pos_Y)?(Nouveau_Y-Fenetre_Pos_Y):(Fenetre_Pos_Y-Nouveau_Y)) + Fenetre_Hauteur*Menu_Facteur_Y);
    Fenetre_Pos_X=Nouveau_X;
    Fenetre_Pos_Y=Nouveau_Y;

  }
  else
  {
    // Update pour effacer le rectangle XOR
    UpdateRect(Fenetre_Pos_X, Fenetre_Pos_Y, Fenetre_Largeur*Menu_Facteur_X, Fenetre_Hauteur*Menu_Facteur_Y);
  }    
  Forme_curseur=FORME_CURSEUR_FLECHE;
  Afficher_curseur();

}

// Gestion des dropdown
short Fenetre_Dropdown_click(T_Bouton_dropdown *Bouton)
{
  short Nb_choix;
  short Indice_choix;
  short Indice_selectionne;
  short Ancien_Indice_selectionne;
  short Hauteur_boite;
  T_Dropdown_choix *Choix;
  // Taille de l'ombre portée (en plus des dimensions normales)
  #define OMBRE_DROITE 3
  #define OMBRE_BAS 4

  
  // Comptage des items pour calculer la taille
  Nb_choix=0;
  for (Choix=Bouton->Premier_choix; Choix!=NULL; Choix=Choix->Next)
  {
    Nb_choix++;
  }
  Hauteur_boite=3+Nb_choix*8+1;
  
  Effacer_curseur();
  Fenetre_Enfoncer_bouton_normal(Bouton->Pos_X,Bouton->Pos_Y,Bouton->Width,Bouton->Height);
  Ouvrir_popup(
    Fenetre_Pos_X+(Bouton->Pos_X)*Menu_Facteur_X,
    Fenetre_Pos_Y+(Bouton->Pos_Y+Bouton->Height)*Menu_Facteur_Y,
    Bouton->Largeur_choix+OMBRE_DROITE,
    Hauteur_boite+OMBRE_BAS);

  // Dessin de la boite

  // Bord gauche
  Block(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X,Hauteur_boite*Menu_Facteur_Y,CM_Noir);
  // Cadre fonce et blanc
  Fenetre_Afficher_cadre_bombe(1,0,Bouton->Largeur_choix-1,Hauteur_boite);
  // Ombre portée
  if (OMBRE_BAS)
  {
    Block(Fenetre_Pos_X+OMBRE_DROITE*Menu_Facteur_X,
        Fenetre_Pos_Y+Hauteur_boite*Menu_Facteur_Y,
        Bouton->Largeur_choix*Menu_Facteur_X,
        OMBRE_BAS*Menu_Facteur_Y,
        CM_Noir);
    Block(Fenetre_Pos_X,
        Fenetre_Pos_Y+Hauteur_boite*Menu_Facteur_Y,
        OMBRE_DROITE*Menu_Facteur_X,
        Menu_Facteur_Y,
        CM_Noir);
  }
  if (OMBRE_DROITE)
  {
    Block(Fenetre_Pos_X+Bouton->Largeur_choix*Menu_Facteur_X,
        Fenetre_Pos_Y+OMBRE_BAS*Menu_Facteur_Y,
        OMBRE_DROITE*Menu_Facteur_X,
        (Hauteur_boite-OMBRE_BAS)*Menu_Facteur_Y,
        CM_Noir);
    Block(Fenetre_Pos_X+Bouton->Largeur_choix*Menu_Facteur_X,
        Fenetre_Pos_Y,
        Menu_Facteur_X,
        OMBRE_BAS*Menu_Facteur_Y,
        CM_Noir);
  }

  Indice_selectionne=-1;
  while (1)
  {
    Ancien_Indice_selectionne = Indice_selectionne;
    // Fenêtre grise
    Block(Fenetre_Pos_X+2*Menu_Facteur_X,
        Fenetre_Pos_Y+1*Menu_Facteur_Y,
        (Bouton->Largeur_choix-3)*Menu_Facteur_X,(Hauteur_boite-2)*Menu_Facteur_Y,CM_Clair);
    // Affichage des items
    for(Choix=Bouton->Premier_choix,Indice_choix=0; Choix!=NULL; Choix=Choix->Next,Indice_choix++)
    {
      byte color_1;
      byte color_2;
      if (Indice_choix==Indice_selectionne)
      {
        color_1=CM_Blanc;
        color_2=CM_Fonce;
        Block(Fenetre_Pos_X+3*Menu_Facteur_X,
        Fenetre_Pos_Y+((2+Indice_choix*8)*Menu_Facteur_Y),
        (Bouton->Largeur_choix-5)*Menu_Facteur_X,(8)*Menu_Facteur_Y,CM_Fonce);
      }
      else
      {
        color_1=CM_Noir;
        color_2=CM_Clair;
      }
      Print_dans_fenetre(3,2+Indice_choix*8,Choix->Label,color_1,color_2);
    }
    UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Fenetre_Largeur*Menu_Facteur_X,Fenetre_Hauteur*Menu_Facteur_Y);
    Afficher_curseur();

    do 
    {
      // Attente
      if(!Get_input())
        Wait_VBL();
      // Mise à jour du survol
      Indice_selectionne=Fenetre_click_dans_zone(2,2,Bouton->Largeur_choix-2,Hauteur_boite-1)?
        (((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y-2)>>3) : -1;

    } while (Mouse_K && Indice_selectionne==Ancien_Indice_selectionne);
    
    if (!Mouse_K)
      break;
    Effacer_curseur();
  }

  Fermer_popup();  


  Fenetre_Desenfoncer_bouton_normal(Bouton->Pos_X,Bouton->Pos_Y,Bouton->Width,Bouton->Height);
  Afficher_curseur();

  if (Indice_selectionne>=0 && Indice_selectionne<Nb_choix)
  {
    for(Choix=Bouton->Premier_choix; Indice_selectionne; Choix=Choix->Next,Indice_selectionne--)
      ;
    Fenetre_Attribut2=Choix->Numero;
    if (Bouton->Affiche_choix)
    {
      // Mettre à jour automatiquement le libellé de la dropdown
      Print_dans_fenetre(Bouton->Pos_X+2,Bouton->Pos_Y+(Bouton->Height-7)/2,Choix->Label,CM_Noir,CM_Clair);
    }
    return Bouton->Numero;
  }
  Fenetre_Attribut2=-1;
  return 0;
}

// --- Fonction de clic sur un bouton a peu près ordinaire:
// Attend que l'on relache le bouton, et renvoie le numero du bouton si on
// est resté dessus, 0 si on a annulé en sortant du bouton.
short Fenetre_bouton_normal_click(word Pos_X, word Pos_Y, word width, word height, short Numero)
{
  while(1)
  {
    Effacer_curseur();
    Fenetre_Enfoncer_bouton_normal(Pos_X,Pos_Y,width,height);
    Afficher_curseur();
    while (Fenetre_click_dans_zone(Pos_X,Pos_Y,Pos_X+width-1,Pos_Y+height-1))
    {
      if(!Get_input())
        Wait_VBL();
      if (!Mouse_K)
      {
        Effacer_curseur();
        Fenetre_Desenfoncer_bouton_normal(Pos_X,Pos_Y,width,height);
        Afficher_curseur();
        return Numero;
      }
    }
    Effacer_curseur();
    Fenetre_Desenfoncer_bouton_normal(Pos_X,Pos_Y,width,height);
    Afficher_curseur();
    while (!(Fenetre_click_dans_zone(Pos_X,Pos_Y,Pos_X+width-1,Pos_Y+height-1)))
    {
      if(!Get_input())
        Wait_VBL();
      if (!Mouse_K)
        return 0;
    }
  }
}

// --- Renvoie le numéro du bouton clicke (-1:hors de la fenêtre, 0:aucun) ---
short Fenetre_Numero_bouton_clicke(void)
{
  T_Bouton_normal   * Temp1;
  T_Bouton_palette  * Temp2;
  T_Bouton_scroller * Temp3;
  T_Bouton_special  * Temp4;
  T_Bouton_dropdown * Temp5;

  long Hauteur_maxi_jauge;

  Fenetre_Attribut1=Mouse_K;

  // Test du click sur les boutons normaux
  for (Temp1=Fenetre_Liste_boutons_normal; Temp1; Temp1=Temp1->Next)
  {
    if (Fenetre_click_dans_zone(Temp1->Pos_X,Temp1->Pos_Y,Temp1->Pos_X+Temp1->Width-1,Temp1->Pos_Y+Temp1->Height-1))
    {
      if (Temp1->Repetable)
      {
        Effacer_curseur();
        Fenetre_Enfoncer_bouton_normal(Temp1->Pos_X,Temp1->Pos_Y,Temp1->Width,Temp1->Height);
        Afficher_curseur();
        Tempo_jauge((Mouse_K==1)? Config.Valeur_tempo_jauge_gauche : Config.Valeur_tempo_jauge_droite);
        Effacer_curseur();
        Fenetre_Desenfoncer_bouton_normal(Temp1->Pos_X,Temp1->Pos_Y,Temp1->Width,Temp1->Height);
        Afficher_curseur();        
        return Temp1->Numero;
      }
      return Fenetre_bouton_normal_click(Temp1->Pos_X,Temp1->Pos_Y,Temp1->Width,Temp1->Height,Temp1->Numero);
    }
  }

  // Test du click sur les zones "palette"
  for (Temp2=Fenetre_Liste_boutons_palette; Temp2; Temp2=Temp2->Next)
  {
    if (Fenetre_click_dans_zone(Temp2->Pos_X+5,Temp2->Pos_Y+3,Temp2->Pos_X+160,Temp2->Pos_Y+82))
    {
      // On stocke dans Attribut2 le numero de couleur cliqué
      Fenetre_Attribut2 = (((Mouse_X-Fenetre_Pos_X)/Menu_Facteur_X)-(Temp2->Pos_X+2)) / 10 * 16 +
        (((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-(Temp2->Pos_Y+3)) / 5;
        return Temp2->Numero;
    }
  }

  // Test du click sur les barres de défilement
  for (Temp3=Fenetre_Liste_boutons_scroller; Temp3; Temp3=Temp3->Next)
  {
    if (Fenetre_click_dans_zone(Temp3->Pos_X,Temp3->Pos_Y,Temp3->Pos_X+10,Temp3->Pos_Y+Temp3->Height-1))
    {
      // Bouton flèche Haut
      if (Fenetre_click_dans_zone(Temp3->Pos_X,Temp3->Pos_Y,Temp3->Pos_X+10,Temp3->Pos_Y+10))
      {
        Effacer_curseur();
        Fenetre_Enfoncer_bouton_normal(Temp3->Pos_X,Temp3->Pos_Y,11,11);

        if (Temp3->Position)
        {
          Temp3->Position--;
          Fenetre_Attribut1=1;
          Fenetre_Attribut2=Temp3->Position;
          Fenetre_Dessiner_jauge(Temp3);
        }
        else
          Fenetre_Attribut1=0;
        
        Afficher_curseur();

        Tempo_jauge((Mouse_K==1)? Config.Valeur_tempo_jauge_gauche : Config.Valeur_tempo_jauge_droite);

        Effacer_curseur();
        Fenetre_Desenfoncer_bouton_normal(Temp3->Pos_X,Temp3->Pos_Y,11,11);
        Afficher_curseur();
      }
      else
      // Bouton flèche Bas
      if (Fenetre_click_dans_zone(Temp3->Pos_X,Temp3->Pos_Y+Temp3->Height-11,Temp3->Pos_X+10,Temp3->Pos_Y+Temp3->Height-1))
      {
        Effacer_curseur();
        Fenetre_Enfoncer_bouton_normal(Temp3->Pos_X,Temp3->Pos_Y+Temp3->Height-11,11,11);

        if (Temp3->Position+Temp3->Nb_visibles<Temp3->Nb_elements)
        {
          Temp3->Position++;
          Fenetre_Attribut1=2;
          Fenetre_Attribut2=Temp3->Position;
          Fenetre_Dessiner_jauge(Temp3);
        }
        else
          Fenetre_Attribut1=0;

        Afficher_curseur();

        Tempo_jauge((Mouse_K==1)? Config.Valeur_tempo_jauge_gauche : Config.Valeur_tempo_jauge_droite);

        Effacer_curseur();
        Fenetre_Desenfoncer_bouton_normal(Temp3->Pos_X,Temp3->Pos_Y+Temp3->Height-11,11,11);
        Afficher_curseur();
      }
      else
      // Jauge
      if (Fenetre_click_dans_zone(Temp3->Pos_X,Temp3->Pos_Y+12,Temp3->Pos_X+10,Temp3->Pos_Y+Temp3->Height-13))
      {
        if (Temp3->Nb_elements>Temp3->Nb_visibles)
        {
          // S'il y a la place de faire scroller le curseur:

          Hauteur_maxi_jauge=(Temp3->Height-24);

          // Fenetre_Attribut2 reçoit la position dans la jauge correspondant au click
          Fenetre_Attribut2 =(Mouse_Y-Fenetre_Pos_Y) / Menu_Facteur_Y;
          Fenetre_Attribut2-=(Temp3->Pos_Y+12+((Temp3->Hauteur_curseur-1)>>1));
          Fenetre_Attribut2*=(Temp3->Nb_elements-Temp3->Nb_visibles);

          if (Fenetre_Attribut2<0)
            Fenetre_Attribut2=0;
          else
          {
            Fenetre_Attribut2 =Round_div(Fenetre_Attribut2,Hauteur_maxi_jauge-Temp3->Hauteur_curseur);
            if (Fenetre_Attribut2+Temp3->Nb_visibles>Temp3->Nb_elements)
              Fenetre_Attribut2=Temp3->Nb_elements-Temp3->Nb_visibles;
          }

          // Si le curseur de la jauge bouge:

          if (Temp3->Position!=Fenetre_Attribut2)
          {
            Temp3->Position=Fenetre_Attribut2;
            Fenetre_Attribut1=3;
            Effacer_curseur();
            Fenetre_Dessiner_jauge(Temp3);
            Afficher_curseur();
          }
          else
            // Si le curseur de la jauge ne bouge pas:
            Fenetre_Attribut1=0;
        }
        else
          // S'il n'y a pas la place de bouger le curseur de la jauge:
          Fenetre_Attribut1=0;
      }
      else
        // Le click se situe dans la zone de la jauge mais n'est sur aucune
        // des 3 parties importantes de la jauge
        Fenetre_Attribut1=0;

      return (Fenetre_Attribut1)? Temp3->Numero : 0;
    }
  }

  // Test du click sur une zone spéciale
  for (Temp4=Fenetre_Liste_boutons_special; Temp4; Temp4=Temp4->Next)
  {
    if (Fenetre_click_dans_zone(Temp4->Pos_X,Temp4->Pos_Y,Temp4->Pos_X+Temp4->Width-1,Temp4->Pos_Y+Temp4->Height-1))
      return Temp4->Numero;
  }

  // Test du click sur une dropdown
  for (Temp5=Fenetre_Liste_boutons_dropdown; Temp5; Temp5=Temp5->Next)
  {
    if (Fenetre_click_dans_zone(Temp5->Pos_X,Temp5->Pos_Y,Temp5->Pos_X+Temp5->Width-1,Temp5->Pos_Y+Temp5->Height-1))
    {
      if (Mouse_K & Temp5->Bouton_actif)
        return Fenetre_Dropdown_click(Temp5);
      else
      {
        Fenetre_Attribut2=-1;
        return Fenetre_bouton_normal_click(Temp5->Pos_X,Temp5->Pos_Y,Temp5->Width,Temp5->Height,Temp5->Numero);
      }
    }
  }

  return 0;
}


short Fenetre_Numero_bouton_touche(void)
{
  T_Bouton_normal * Temp;

  if (Touche & MOD_SHIFT)
    Fenetre_Attribut1=A_DROITE;
  else
    Fenetre_Attribut1=A_GAUCHE;

  // On fait une première recherche
  Temp=Fenetre_Liste_boutons_normal;
  while (Temp!=NULL)
  {
    if (Temp->Raccourci==Touche)
    {
      Effacer_curseur();
      Fenetre_Enfoncer_bouton_normal(Temp->Pos_X,Temp->Pos_Y,Temp->Width,Temp->Height);
      Afficher_curseur();
      
      Tempo_jauge(Config.Valeur_tempo_jauge_droite);
      
      Effacer_curseur();
      Fenetre_Desenfoncer_bouton_normal(Temp->Pos_X,Temp->Pos_Y,Temp->Width,Temp->Height);
      Afficher_curseur();

      return Temp->Numero;
    }
    Temp=Temp->Next;
  }

  // Si la recherche n'a pas été fructueuse ET que l'utilisateur appuyait sur
  // <Shift>, on regarde si un bouton ne pourrait pas réagir comme si <Shift>
  // n'était pas appuyé.
  if (Fenetre_Attribut1==A_DROITE)
  {
    Temp=Fenetre_Liste_boutons_normal;
    while (Temp!=NULL)
    {
      if (Temp->Raccourci==(Touche&0x0FFF))
        return Temp->Numero;
      Temp=Temp->Next;
    }
  }

  return 0;
}

short Fenetre_Bouton_clicke(void)
{
  short Bouton;

  if(!Get_input())Wait_VBL();

  // Gestion des clicks
  if (Mouse_K)
  {
    if ((Mouse_X<Fenetre_Pos_X) || (Mouse_Y<Fenetre_Pos_Y)
     || (Mouse_X>=Fenetre_Pos_X+(Fenetre_Largeur*Menu_Facteur_X))
     || (Mouse_Y>=Fenetre_Pos_Y+(Fenetre_Hauteur*Menu_Facteur_Y)))
      return -1;
    else
    {
      if (Mouse_Y < Fenetre_Pos_Y+(12*Menu_Facteur_Y))
        Deplacer_fenetre(Mouse_X-Fenetre_Pos_X,Mouse_Y-Fenetre_Pos_Y);
      else
        return Fenetre_Numero_bouton_clicke();
    }
  }

  // Gestion des touches
  if (Touche)
  {
    Bouton=Fenetre_Numero_bouton_touche();
    if (Bouton)
    {
      Touche=0;
      return Bouton;
    }
  }

  return 0;
}


// Fonction qui sert à remapper les parties sauvegardées derriere les
// fenetres ouvertes. C'est utilisé par exemple par la fenetre de palette
// Qui remappe des couleurs, afin de propager les changements.
void Remappe_fond_fenetres(byte * Table_de_conversion, int Min_Y, int Max_Y)
{
  int Indice_fenetre; 
        byte* EDI;
        int dx,cx;

  for (Indice_fenetre=0; Indice_fenetre<Fenetre; Indice_fenetre++)
  {
    EDI = Fond_fenetre[Indice_fenetre];
  
        // Pour chaque ligne
        for(dx=0; dx<Pile_Fenetre_Hauteur[Indice_fenetre]*Menu_Facteur_Y;dx++)
        {
          if (dx+Pile_Fenetre_Pos_Y[Indice_fenetre]>Max_Y)
            return;
          if (dx+Pile_Fenetre_Pos_Y[Indice_fenetre]<Min_Y)
          {
            EDI += Pile_Fenetre_Largeur[Indice_fenetre]*Menu_Facteur_X*Pixel_width;
          }
          else
                // Pour chaque pixel
                for(cx=Pile_Fenetre_Largeur[Indice_fenetre]*Menu_Facteur_X*Pixel_width;cx>0;cx--)
                {
                        *EDI = Table_de_conversion[*EDI];
                        EDI ++;
                }
        }
  }
}
