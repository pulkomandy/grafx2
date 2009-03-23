/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008      Franck Charlet
    Copyright 2007-2008 Adrien Destugues
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

********************************************************************************

    Graphical interface management functions (windows, menu, cursor)
*/

#include <math.h>
#include <string.h> // strncpy() strlen()

#include "windows.h"
#include "global.h"
#include "graph.h"
#include "moteur.h"
#include "divers.h"
#include "sdlscreen.h"
#include "erreurs.h"

// L'encapsulation tente une percée...ou un dernier combat.

// Nombre de cellules réel dans la palette du menu
word Menu_Cellules_X;
word Palette_Cells_X()
{
  return Menu_Cellules_X;
}
word Menu_Cellules_Y;
word Palette_Cells_Y()
{
  return Menu_Cellules_Y;
}

// Affichage d'un pixel dans le menu (le menu doit être visible)
void Pixel_dans_barre_d_outil(word x,word y,byte Couleur)
{
  Block(x*Menu_Facteur_X,(y*Menu_Facteur_Y)+Menu_Ordonnee,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}

  // Affichage d'un pixel dans la fenêtre (la fenêtre doit être visible)

void Pixel_dans_fenetre(word x,word y,byte Couleur)
{
    Block((x*Menu_Facteur_X)+Fenetre_Pos_X,(y*Menu_Facteur_Y)+Fenetre_Pos_Y,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}


// -- Affichages de différents cadres dans une fenêtre -----------------------

  // -- Cadre général avec couleurs paramètrables --

void Fenetre_Afficher_cadre_general(word x_pos,word y_pos,word width,word height,
                                    byte Couleur_HG,byte Couleur_BD,byte Couleur_S,byte Couleur_CHG,byte Couleur_CBD)
// Paramètres de couleurs:
// Couleur_HG =Bords Haut et Gauche
// Couleur_BD =Bords Bas et Droite
// Couleur_S  =Coins Haut-Droite et Bas-Gauche
// Couleur_CHG=Coin Haut-Gauche
// Couleur_CBD=Coin Bas-Droite
{
  // Bord haut (sans les extrémités)
  Block(Fenetre_Pos_X+((x_pos+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+(y_pos*Menu_Facteur_Y),
        (width-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_HG);

  // Bord bas (sans les extrémités)
  Block(Fenetre_Pos_X+((x_pos+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((y_pos+height-1)*Menu_Facteur_Y),
        (width-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_BD);

  // Bord gauche (sans les extrémités)
  Block(Fenetre_Pos_X+(x_pos*Menu_Facteur_X),
        Fenetre_Pos_Y+((y_pos+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(height-2)*Menu_Facteur_Y,Couleur_HG);

  // Bord droite (sans les extrémités)
  Block(Fenetre_Pos_X+((x_pos+width-1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((y_pos+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(height-2)*Menu_Facteur_Y,Couleur_BD);

  // Coin haut gauche
  Pixel_dans_fenetre(x_pos,y_pos,Couleur_CHG);
  // Coin haut droite
  Pixel_dans_fenetre(x_pos+width-1,y_pos,Couleur_S);
  // Coin bas droite
  Pixel_dans_fenetre(x_pos+width-1,y_pos+height-1,Couleur_CBD);
  // Coin bas gauche
  Pixel_dans_fenetre(x_pos,y_pos+height-1,Couleur_S);
}

  // -- Cadre dont tout le contour est d'une seule couleur --

void Fenetre_Afficher_cadre_mono(word x_pos,word y_pos,word width,word height,byte Couleur)
{
  Fenetre_Afficher_cadre_general(x_pos,y_pos,width,height,Couleur,Couleur,Couleur,Couleur,Couleur);
}

  // -- Cadre creux: foncé en haut-gauche et clair en bas-droite --

void Fenetre_Afficher_cadre_creux(word x_pos,word y_pos,word width,word height)
{
  Fenetre_Afficher_cadre_general(x_pos,y_pos,width,height,CM_Fonce,CM_Blanc,CM_Clair,CM_Fonce,CM_Blanc);
}

  // -- Cadre bombé: clair en haut-gauche et foncé en bas-droite --

void Fenetre_Afficher_cadre_bombe(word x_pos,word y_pos,word width,word height)
{
  Fenetre_Afficher_cadre_general(x_pos,y_pos,width,height,CM_Blanc,CM_Fonce,CM_Clair,CM_Blanc,CM_Fonce);
}

  // -- Cadre de séparation: un cadre bombé dans un cadre creux (3D!!!) --

void Fenetre_Afficher_cadre(word x_pos,word y_pos,word width,word height)
{
  Fenetre_Afficher_cadre_creux(x_pos,y_pos,width,height);
  Fenetre_Afficher_cadre_bombe(x_pos+1,y_pos+1,width-2,height-2);
}


//-- Affichages relatifs à la palette dans le menu ---------------------------

  // -- Affichage des couleurs courante (fore/back) de pinceau dans le menu --

void Afficher_foreback(void)
{
  if (Menu_visible)
  {
    Block((LARGEUR_MENU-17)*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y,Menu_Facteur_X<<4,Menu_Facteur_Y*7,Back_color);
    Block((LARGEUR_MENU-13)*Menu_Facteur_X,Menu_Ordonnee+(Menu_Facteur_Y<<1),Menu_Facteur_X<<3,Menu_Facteur_Y*5,Fore_color);

    UpdateRect((LARGEUR_MENU-17)*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y,Menu_Facteur_X<<4,Menu_Facteur_Y*7);
  }
}

  // -- Tracer un cadre de couleur autour de la Fore_color dans le menu --

void Encadrer_couleur_menu(byte Couleur)
{
  word Debut_X,Debut_Y,Fin_X,Fin_Y;
  word Indice;
  word Hauteur_cellule=32/Menu_Cellules_Y;

  if ((Fore_color>=Couleur_debut_palette) && (Fore_color<Couleur_debut_palette+Menu_Cellules_X*Menu_Cellules_Y) && (Menu_visible))
  {
    if (Config.Couleurs_separees)
    {
      Debut_X=(LARGEUR_MENU+((Fore_color-Couleur_debut_palette)/Menu_Cellules_Y)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee+((1+(((Fore_color-Couleur_debut_palette)%Menu_Cellules_Y)*Hauteur_cellule))*Menu_Facteur_Y);

      Block(Debut_X,Debut_Y,(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);
      Block(Debut_X,Debut_Y+(Menu_Facteur_Y*Hauteur_cellule),(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);

      Block(Debut_X,Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*(Hauteur_cellule-1),Couleur);
      Block(Debut_X+(Menu_Taille_couleur*Menu_Facteur_X),Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*(Hauteur_cellule-1),Couleur);

      UpdateRect(Debut_X,Debut_Y,(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y*(Hauteur_cellule+1));
    }
    else
    {
      if (Couleur==CM_Noir)
      {
        Debut_X=(LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)/Menu_Cellules_Y)*Menu_Taille_couleur)*Menu_Facteur_X;
        Debut_Y=Menu_Ordonnee+((2+(((Fore_color-Couleur_debut_palette)%Menu_Cellules_Y)*Hauteur_cellule))*Menu_Facteur_Y);

        Block(Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y*Hauteur_cellule,Fore_color);

        UpdateRect(Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,Menu_Facteur_Y*Hauteur_cellule);
      }
      else
      {
        Debut_X=LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)/Menu_Cellules_Y)*Menu_Taille_couleur;
        Debut_Y=2+(((Fore_color-Couleur_debut_palette)%Menu_Cellules_Y)*Hauteur_cellule);

        Fin_X=Debut_X+Menu_Taille_couleur-1;
        Fin_Y=Debut_Y+Hauteur_cellule-1;

        for (Indice=Debut_X; Indice<=Fin_X; Indice++)
          Block(Indice*Menu_Facteur_X,Menu_Ordonnee+(Debut_Y*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Debut_Y)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_Y+1; Indice<Fin_Y; Indice++)
          Block(Debut_X*Menu_Facteur_X,Menu_Ordonnee+(Indice*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Debut_X)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_Y+1; Indice<Fin_Y; Indice++)
          Block(Fin_X*Menu_Facteur_X,Menu_Ordonnee+(Indice*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Fin_X)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_X; Indice<=Fin_X; Indice++)
          Block(Indice*Menu_Facteur_X,Menu_Ordonnee+(Fin_Y*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Fin_Y)&1)?CM_Blanc:CM_Noir);

        UpdateRect(Debut_X*Menu_Facteur_X,Debut_Y*Menu_Facteur_Y,Menu_Taille_couleur*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y*Hauteur_cellule);
      }
    }
  }
}

  // -- Afficher la palette dans le menu --

void Afficher_palette_du_menu(void)
{
  int Couleur;
  byte Hauteur_cellule=32/Menu_Cellules_Y;
  // width: Menu_Taille_couleur
  
  if (Menu_visible)
  {
    Block(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y,CM_Noir);

    if (Config.Couleurs_separees)
      for (Couleur=0;Couleur_debut_palette+Couleur<256&&Couleur<Menu_Cellules_X*Menu_Cellules_Y;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur/Menu_Cellules_Y)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur%Menu_Cellules_Y)*Hauteur_cellule))*Menu_Facteur_Y),
              (Menu_Taille_couleur-1)*Menu_Facteur_X,
              Menu_Facteur_Y*(Hauteur_cellule-1),
              Couleur_debut_palette+Couleur);
    else
      for (Couleur=0;Couleur_debut_palette+Couleur<256&&Couleur<Menu_Cellules_X*Menu_Cellules_Y;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur/Menu_Cellules_Y)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur%Menu_Cellules_Y)*Hauteur_cellule))*Menu_Facteur_Y),
              Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y*Hauteur_cellule,
              Couleur_debut_palette+Couleur);

    Encadrer_couleur_menu(CM_Blanc);
    UpdateRect(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y);
  }
}

  // -- Recalculer l'origine de la palette dans le menu pour rendre la
  //    Fore_color visible --

void Recadrer_palette(void)
{
  byte Ancienne_couleur=Couleur_debut_palette;

  if (Fore_color<Couleur_debut_palette)
  {
    while (Fore_color<Couleur_debut_palette)
      Couleur_debut_palette-=Menu_Cellules_Y;
  }
  else
  {
    while (Fore_color>=Couleur_debut_palette+Menu_Cellules_X*Menu_Cellules_Y)
      Couleur_debut_palette+=Menu_Cellules_Y;
  }
  if (Ancienne_couleur!=Couleur_debut_palette)
    Afficher_palette_du_menu();
}

void Changer_cellules_palette()
{
  // On initialise avec la configuration de l'utilisateur
  Menu_Cellules_X=Config.Palette_Cells_X;
  Menu_Cellules_Y=Config.Palette_Cells_Y;
  // Mais on sait jamais
  if (Menu_Cellules_X<1)
    Menu_Cellules_X=1;
  if (Menu_Cellules_Y<1)
    Menu_Cellules_Y=1;
  
  while (1)
  {
    Menu_Taille_couleur = ((Largeur_ecran/Menu_Facteur_X)-(LARGEUR_MENU+2)) / Menu_Cellules_X;

    // Si ça tient, c'est bon. Sinon, on retente avec une colonne de moins
    if (Menu_Taille_couleur>2)
      break;
    Menu_Cellules_X--;
  }
  
  // Cale Couleur_debut_palette sur un multiple de Cells_Y (arrondi inférieur)
  Couleur_debut_palette=Couleur_debut_palette/Menu_Cellules_Y*Menu_Cellules_Y;

  // Si le nombre de cellules a beaucoup augmenté et qu'on était près de
  // la fin, il faut reculer Couleur_debut_palette pour montrer plein
  // de couleurs.
  if ((int)Couleur_debut_palette+(Menu_Cellules_Y)*Menu_Cellules_X*2>=256)
    Couleur_debut_palette=255/Menu_Cellules_Y*Menu_Cellules_Y-(Menu_Cellules_X-1)*Menu_Cellules_Y;

  // Mise à jour de la taille du bouton dans le menu. C'est pour pas que
  // la bordure noire soit active.
  Bouton[BOUTON_CHOIX_COL].Width=(Menu_Taille_couleur*Menu_Cellules_X)-1;
  Bouton[BOUTON_CHOIX_COL].Height=32/Menu_Cellules_Y*Menu_Cellules_Y-1;
}

// Retrouve la couleur sur laquelle pointe le curseur souris.
// Cette fonction suppose qu'on a déja vérifié que le curseur est dans
// la zone rectangulaire du BOUTON_CHOIX_COL
// La fonction renvoie -1 si on est "trop à gauche" (pas possible)
// ou après la couleur 255 (Ce qui peut arriver si la palette est affichée
// avec un nombre de lignes qui n'est pas une puissance de deux.)
int Couleur_palette()
{
  int Col;
  int line;
  line=(((Mouse_Y-Menu_Ordonnee)/Menu_Facteur_Y)-2)/(32/Menu_Cellules_Y);
  
  Col=Couleur_debut_palette+line+
    ((((Mouse_X/Menu_Facteur_X)-(LARGEUR_MENU+1))/Menu_Taille_couleur)*Menu_Cellules_Y);
  if (Col<0 || Col>255)
    return -1;
  return Col;
}

  // -- Afficher tout le menu --

void Afficher_menu(void)
{
  word x_pos;
  word y_pos;
  char Chaine[4];


  if (Menu_visible)
  {
    // Affichage du sprite du menu
    for (y_pos=0;y_pos<HAUTEUR_MENU;y_pos++)
      for (x_pos=0;x_pos<LARGEUR_MENU;x_pos++)
        Pixel_dans_menu(x_pos,y_pos,BLOCK_MENU[y_pos][x_pos]);
    // Affichage de la bande grise sous la palette
    Block(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee_Texte-Menu_Facteur_Y,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),9*Menu_Facteur_Y,CM_Clair);

    // Affichage de la palette
    Afficher_palette_du_menu();

    // Affichage des couleurs de travail
    Afficher_foreback();


    if (!Fenetre)
    {
      if ((Mouse_Y<Menu_Ordonnee) &&                                                    // Souris dans l'image
          ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ))
      {
        // Dans ces deux cas, on met dans la barre les XY courant, même s'il y a des chances que ça soit recouvert si la souris est sur un bouton (trop chiant à vérifier)
        if ( (Operation_en_cours!=OPERATION_PIPETTE)
          && (Operation_en_cours!=OPERATION_REMPLACER) )
          Print_dans_menu("X:       Y:             ",0);
        else
        {
          Print_dans_menu("X:       Y:       (    )",0);
          Num2str(Pipette_Couleur,Chaine,3);
          Print_dans_menu(Chaine,20);
          Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
        }
        Print_coordonnees();
      }
      Print_nom_fichier();
    }
    UpdateRect(0,Menu_Ordonnee,Largeur_ecran,HAUTEUR_MENU*Menu_Facteur_Y); // on met toute la largur à jour, ça inclut la palette et la zone d'état avec le nom du fichier
  }
}

// -- Affichage de texte -----------------------------------------------------

  // -- Afficher une chaîne n'importe où à l'écran --

void Print_general(short x,short y,const char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  word  Indice;
  int x_pos;
  int y_pos;
  byte *font_pixel;
  short Reel_X;
  short Reel_Y;
  byte Repeat_Menu_Facteur_X;
  byte Repeat_Menu_Facteur_Y;

  Reel_Y=y;
  for (y_pos=0;y_pos<8<<3;y_pos+=1<<3)
  {
    Reel_X=0; // Position dans le buffer
    for (Indice=0;Chaine[Indice]!='\0';Indice++)
    {
      // Pointeur sur le premier pixel du caractère
      font_pixel=Fonte+(((unsigned char)Chaine[Indice])<<6);
      for (x_pos=0;x_pos<8;x_pos+=1)
        for (Repeat_Menu_Facteur_X=0;Repeat_Menu_Facteur_X<Menu_Facteur_X*Pixel_width;Repeat_Menu_Facteur_X++)
          Buffer_de_ligne_horizontale[Reel_X++]=*(font_pixel+x_pos+y_pos)?Couleur_texte:Couleur_fond;
    }
    for (Repeat_Menu_Facteur_Y=0;Repeat_Menu_Facteur_Y<Menu_Facteur_Y;Repeat_Menu_Facteur_Y++)
      Afficher_ligne_fast(x,Reel_Y++,Indice*Menu_Facteur_X*8,Buffer_de_ligne_horizontale);
  }
}

  // -- Afficher un caractère dans une fenêtre --

void Print_char_dans_fenetre(short x_pos,short y_pos,const unsigned char c,byte Couleur_texte,byte Couleur_fond)
{
  short x,y;
  byte *pixel;
  x_pos=(x_pos*Menu_Facteur_X)+Fenetre_Pos_X;
  y_pos=(y_pos*Menu_Facteur_Y)+Fenetre_Pos_Y;
  // Premier pixel du caractère
  pixel=Fonte + (c<<6);
  
  for (y=0;y<8;y++)
    for (x=0;x<8;x++)
      Block(x_pos+(x*Menu_Facteur_X), y_pos+(y*Menu_Facteur_Y),
            Menu_Facteur_X, Menu_Facteur_Y,
            (*(pixel++)?Couleur_texte:Couleur_fond));
}

  // -- Afficher un caractère sans fond dans une fenêtre --

void Print_char_transparent_dans_fenetre(short x_pos,short y_pos,const unsigned char c,byte Couleur)
{
  short x,y;
  byte *pixel;
  x_pos=(x_pos*Menu_Facteur_X)+Fenetre_Pos_X;
  y_pos=(y_pos*Menu_Facteur_Y)+Fenetre_Pos_Y;
  // Premier pixel du caractère
  pixel=Fonte + (c<<6);
  
  for (y=0;y<8;y++)
    for (x=0;x<8;x++)
    {
      if (*(pixel++))
        Block(x_pos+(x*Menu_Facteur_X), y_pos+(y*Menu_Facteur_Y),
              Menu_Facteur_X, Menu_Facteur_Y, Couleur);
    }
}

  // -- Afficher une chaîne dans une fenêtre, avec taille maxi --

void Print_dans_fenetre_limite(short x,short y,const char * Chaine,byte size,byte Couleur_texte,byte Couleur_fond)
{
  char Chaine_affichee[256];
  strncpy(Chaine_affichee, Chaine, size);
  Chaine_affichee[size]='\0';

  if (strlen(Chaine) > size)
  {
    Chaine_affichee[size-1]=CARACTERE_SUSPENSION;
  }
  Print_dans_fenetre(x, y, Chaine_affichee, Couleur_texte, Couleur_fond);
}

  // -- Afficher une chaîne dans une fenêtre --

void Print_dans_fenetre(short x,short y,const char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  Print_general((x*Menu_Facteur_X)+Fenetre_Pos_X,
                (y*Menu_Facteur_Y)+Fenetre_Pos_Y,
                Chaine,Couleur_texte,Couleur_fond);
  UpdateRect(x*Menu_Facteur_X+Fenetre_Pos_X,y*Menu_Facteur_Y+Fenetre_Pos_Y,8*Menu_Facteur_X*strlen(Chaine),8*Menu_Facteur_Y);
}

  // -- Afficher une chaîne dans le menu --

void Print_dans_menu(const char * Chaine, short position)
{
  Print_general((18+(position<<3))*Menu_Facteur_X,Menu_Ordonnee_Texte,Chaine,CM_Noir,CM_Clair);
  UpdateRect((18+(position<<3))*Menu_Facteur_X,Menu_Ordonnee_Texte,strlen(Chaine)*8*Menu_Facteur_X,8*Menu_Facteur_Y);
}

  // -- Afficher les coordonnées du pinceau dans le menu --

// Note : cette fonction n'affiche que les chiffres, pas les X: Y: qui sont dans la gestion principale, car elle est apellée très souvent.
void Print_coordonnees(void)
{
  char Tempo[5];

  if (Menu_visible && !Curseur_dans_menu)
  {
    if ( (Operation_en_cours==OPERATION_PIPETTE)
      || (Operation_en_cours==OPERATION_REMPLACER) )
    {
      if ( (Pinceau_X>=0) && (Pinceau_Y>=0)
        && (Pinceau_X<Principal_Largeur_image)
        && (Pinceau_Y<Principal_Hauteur_image) )
        Pipette_Couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
      else
        Pipette_Couleur=0;
      Pipette_X=Pinceau_X;
      Pipette_Y=Pinceau_Y;

      Num2str(Pipette_Couleur,Tempo,3);
      Print_dans_menu(Tempo,20);
      Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
      UpdateRect(170*Menu_Facteur_X,Menu_Ordonnee_Texte,8*Menu_Facteur_X,8*Menu_Facteur_Y);
    }

    Num2str((dword)Pinceau_X,Tempo,4);
    Print_dans_menu(Tempo,2);
    Num2str((dword)Pinceau_Y,Tempo,4);
    Print_dans_menu(Tempo,11);
  }
}

  // -- Afficher le nom du fichier dans le menu --

void Print_nom_fichier(void)
{
  char Nom_affiche[12+1];
  int Taille_nom;
  if (Menu_visible)
  {
    // Si le nom de fichier fait plus de 12 caractères, on n'affiche que les 12 derniers
    strncpy(Nom_affiche,Principal_Nom_fichier,12);
    Taille_nom=strlen(Principal_Nom_fichier);
    Nom_affiche[12]='\0';
    if (Taille_nom>12)
    {
      Nom_affiche[11]=CARACTERE_SUSPENSION;
      Taille_nom = 12;
    }
    
    Block(Largeur_ecran-96*Menu_Facteur_X,
          Menu_Ordonnee_Texte,Menu_Facteur_X*96,Menu_Facteur_Y<<3,CM_Clair);

    Print_general(Largeur_ecran-Taille_nom*8*Menu_Facteur_X,Menu_Ordonnee_Texte,Nom_affiche,CM_Noir,CM_Clair);
  }
}

// Fonction d'affichage d'une chaine numérique avec une fonte très fine
// Spécialisée pour les compteurs RGB
void Print_compteur(short x,short y,const char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  // Macros pour écrire des litteraux binaires.
  // Ex: Ob(11110000) == 0xF0
  #define Ob(x)  ((unsigned)Ob_(0 ## x ## uL))
  #define Ob_(x) ((x & 1) | (x >> 2 & 2) | (x >> 4 & 4) | (x >> 6 & 8) |                \
          (x >> 8 & 16) | (x >> 10 & 32) | (x >> 12 & 64) | (x >> 14 & 128))

  byte thin_font[14][8] = {
   { // 0
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 1
    Ob(00001100),
    Ob(00011100),
    Ob(00111100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100),
    Ob(00001100)
   },
   { // 2
    Ob(00011100),
    Ob(00110110),
    Ob(00000110),
    Ob(00000110),
    Ob(00000110),
    Ob(00001100),
    Ob(00011000),
    Ob(00111110)
   },
   { // 3
    Ob(00011100),
    Ob(00110110),
    Ob(00000110),
    Ob(00001100),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 4
    Ob(00001100),
    Ob(00001100),
    Ob(00011000),
    Ob(00011000),
    Ob(00110000),
    Ob(00110100),
    Ob(00111110),
    Ob(00000100)
   },
   { // 5
    Ob(00111110),
    Ob(00110000),
    Ob(00110000),
    Ob(00111100),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 6
    Ob(00011100),
    Ob(00110110),
    Ob(00110000),
    Ob(00111100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 7
    Ob(00111110),
    Ob(00000110),
    Ob(00000110),
    Ob(00001100),
    Ob(00011000),
    Ob(00011000),
    Ob(00011000),
    Ob(00011000)
   },
   { // 8
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00110110),
    Ob(00011100)
   },
   { // 9
    Ob(00011100),
    Ob(00110110),
    Ob(00110110),
    Ob(00011110),
    Ob(00000110),
    Ob(00000110),
    Ob(00110110),
    Ob(00011100)
   },
   { // (espace)
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000)
   },
   { // +
    Ob(00000000),
    Ob(00001000),
    Ob(00001000),
    Ob(00111110),
    Ob(00001000),
    Ob(00001000),
    Ob(00000000),
    Ob(00000000)
   },
   { // -
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00111110),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000),
    Ob(00000000)
   },
   { // +-
    Ob(00001000),
    Ob(00001000),
    Ob(00111110),
    Ob(00001000),
    Ob(00001000),
    Ob(00000000),
    Ob(00111110),
    Ob(00000000)
   } };

  word  Indice;
  short x_pos;
  short y_pos;
  for (Indice=0;Chaine[Indice]!='\0';Indice++)
  {
    int Numero_car;
    switch(Chaine[Indice])
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        Numero_car=Chaine[Indice]-'0';
        break;
      case ' ':
      default:
        Numero_car=10;
        break;
      case '+':
        Numero_car=11;
        break;
      case '-':
        Numero_car=12;
        break;
      case '±':
        Numero_car=13;
        break;
    }
    for (y_pos=0;y_pos<8;y_pos++)
    {
      for (x_pos=0;x_pos<6;x_pos++)
      {
        byte Couleur = (thin_font[Numero_car][y_pos] & (1 << (6-x_pos))) ? Couleur_texte:Couleur_fond;
        Pixel_dans_fenetre(x+(Indice*6+x_pos),y+y_pos,Couleur);
      }
    }
  }
  UpdateRect(Fenetre_Pos_X+x*Menu_Facteur_X,Fenetre_Pos_Y+y*Menu_Facteur_Y,strlen(Chaine)*Menu_Facteur_X*6,8*Menu_Facteur_Y);
}



//---- Fenêtre demandant de confirmer une action et renvoyant la réponse -----
byte Demande_de_confirmation(char * Message)
{
  short Bouton_clicke;
  word  Largeur_de_la_fenetre;

  Largeur_de_la_fenetre=(strlen(Message)<<3)+20;

  if (Largeur_de_la_fenetre<120)
    Largeur_de_la_fenetre=120;

  Ouvrir_fenetre(Largeur_de_la_fenetre,60,"Confirmation");

  Print_dans_fenetre((Largeur_de_la_fenetre>>1)-(strlen(Message)<<2),20,Message,CM_Noir,CM_Clair);

  Fenetre_Definir_bouton_normal((Largeur_de_la_fenetre/3)-20     ,37,40,14,"Yes",1,1,SDLK_y); // 1
  Fenetre_Definir_bouton_normal(((Largeur_de_la_fenetre<<1)/3)-20,37,40,14,"No" ,1,1,SDLK_n); // 2

  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*Largeur_de_la_fenetre,Menu_Facteur_Y*60);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
    if (Touche==SDLK_RETURN) Bouton_clicke=1;
    if (Touche==TOUCHE_ESC) Bouton_clicke=2;
  }
  while (Bouton_clicke<=0);
  Touche=0;

  Fermer_fenetre();
  Afficher_curseur();

  return (Bouton_clicke==1)? 1 : 0;
}



//---- Fenêtre avertissant de quelque chose et attendant un click sur OK -----
void Warning_message(char * Message)
{
  short Bouton_clicke;
  word  Largeur_de_la_fenetre;

  Largeur_de_la_fenetre=(strlen(Message)<<3)+20;
  if (Largeur_de_la_fenetre<120)
    Largeur_de_la_fenetre=120;

  Ouvrir_fenetre(Largeur_de_la_fenetre,60,"Warning!");

  Print_dans_fenetre((Largeur_de_la_fenetre>>1)-(strlen(Message)<<2),20,Message,CM_Noir,CM_Clair);
  Fenetre_Definir_bouton_normal((Largeur_de_la_fenetre>>1)-20     ,37,40,14,"OK",1,1,SDLK_RETURN); // 1
  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*Largeur_de_la_fenetre,Menu_Facteur_Y*60);
  Afficher_curseur();

  do
    Bouton_clicke=Fenetre_Bouton_clicke();
  while ((Bouton_clicke<=0) && (Touche!=TOUCHE_ESC) && (Touche!=SDLK_o));
  Touche=0;

  Fermer_fenetre();
  Afficher_curseur();
}



  // -- Redessiner le sprite d'un bouton dans le menu --

void Afficher_sprite_dans_menu(int Numero_bouton,int Numero_sprite)
{
  word x_pos;
  word y_pos;
  word Pos_menu_X;
  word Pos_menu_Y;
  byte Couleur;

  Pos_menu_Y=Bouton[Numero_bouton].Decalage_Y;
  Pos_menu_X=Bouton[Numero_bouton].Decalage_X;
  if (Bouton[Numero_bouton].Shape != FORME_BOUTON_TRIANGLE_BAS_DROITE)
  {
    Pos_menu_Y+=1;
    Pos_menu_X+=1;
  }
  
  for (y_pos=0;y_pos<HAUTEUR_SPRITE_MENU;y_pos++)
    for (x_pos=0;x_pos<LARGEUR_SPRITE_MENU;x_pos++)
    {
      Couleur=SPRITE_MENU[Numero_sprite][y_pos][x_pos];
      Pixel_dans_menu(Pos_menu_X+x_pos,Pos_menu_Y+y_pos,Couleur);
      BLOCK_MENU[Pos_menu_Y+y_pos][Pos_menu_X+x_pos]=Couleur;
    }
  UpdateRect(Menu_Facteur_X*(Bouton[Numero_bouton].Decalage_X+1),
    (Bouton[Numero_bouton].Decalage_Y+1)*Menu_Facteur_Y+Menu_Ordonnee,
    LARGEUR_SPRITE_MENU*Menu_Facteur_X,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y);
}

  // -- Redessiner la forme du pinceau dans le menu --

void Afficher_pinceau_dans_menu(void)
{
  short x_pos,y_pos;
  short Debut_X;
  short Pos_menu_X,Pos_menu_Y;
  short Debut_menu_X;
  byte Couleur;

  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_BROSSE_COULEUR    : // Brosse en couleur
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      for (Pos_menu_Y=2,y_pos=0;y_pos<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,y_pos++)
        for (Pos_menu_X=1,x_pos=0;x_pos<LARGEUR_SPRITE_MENU;Pos_menu_X++,x_pos++)
        {
          Couleur=SPRITE_MENU[4][y_pos][x_pos];
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
      break;
    default : // Pinceau
      // On efface le pinceau précédent
      for (Pos_menu_Y=2,y_pos=0;y_pos<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,y_pos++)
        for (Pos_menu_X=1,x_pos=0;x_pos<LARGEUR_SPRITE_MENU;Pos_menu_X++,x_pos++)
        {
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,CM_Clair);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=CM_Clair;
        }
      // On affiche le nouveau
      Debut_menu_X=8-Pinceau_Decalage_X;
      if (Debut_menu_X<1)
      {
        Debut_X=Pinceau_Decalage_X-7;
        Debut_menu_X=1;
      }
      else
        Debut_X=0;

      Pos_menu_Y=9-Pinceau_Decalage_Y;
      if (Pos_menu_Y<2)
      {
        y_pos=Pinceau_Decalage_Y-7;
        Pos_menu_Y=2;
      }
      else
        y_pos=0;

      for (;((y_pos<Pinceau_Hauteur) && (Pos_menu_Y<16));Pos_menu_Y++,y_pos++)
        for (Pos_menu_X=Debut_menu_X,x_pos=Debut_X;((x_pos<Pinceau_Largeur) && (Pos_menu_X<15));Pos_menu_X++,x_pos++)
        {
          Couleur=(Pinceau_Sprite[(y_pos*TAILLE_MAXI_PINCEAU)+x_pos])?CM_Noir:CM_Clair;
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
  }
  UpdateRect(0,Menu_Ordonnee,LARGEUR_SPRITE_MENU*Menu_Facteur_X+3,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y+3);
}

  // -- Dessiner un pinceau prédéfini dans la fenêtre --

void Afficher_pinceau_dans_fenetre(word x,word y,int number)
  // Pinceau = 0..NB_SPRITES_PINCEAU-1 : Pinceau prédéfini
{
  word x_pos;
  word y_pos;
  word Pos_fenetre_X;
  word Pos_fenetre_Y;
  int Taille_X;
  int Taille_Y;
  word Orig_X;
  word Orig_Y;

  Taille_X=Menu_Facteur_X/Pixel_height;
  if (Taille_X<1)
    Taille_X=1;
  Taille_Y=Menu_Facteur_Y/Pixel_width;
  if (Taille_Y<1)
    Taille_Y=1;

  Orig_X = (x + 8)*Menu_Facteur_X - (Pinceau_predefini_Decalage_X[number])*Taille_X+Fenetre_Pos_X;
  Orig_Y = (y + 8)*Menu_Facteur_Y - (Pinceau_predefini_Decalage_Y[number])*Taille_Y+Fenetre_Pos_Y;

  for (Pos_fenetre_Y=0,y_pos=0; y_pos<Pinceau_predefini_Hauteur[number]; Pos_fenetre_Y++,y_pos++)
    for (Pos_fenetre_X=0,x_pos=0; x_pos<Pinceau_predefini_Largeur[number]; Pos_fenetre_X++,x_pos++)
      Block(Orig_X+Pos_fenetre_X*Taille_X,Orig_Y+Pos_fenetre_Y*Taille_Y,Taille_X,Taille_Y,(SPRITE_PINCEAU[number][y_pos][x_pos])?CM_Noir:CM_Clair);
  // On n'utilise pas Pixel_dans_fenetre() car on ne dessine pas
  // forcément avec la même taille de pixel.

  UpdateRect( ToWinX(Orig_X), ToWinY(Orig_Y),
        ToWinL(Pinceau_predefini_Largeur[number]),
        ToWinH(Pinceau_predefini_Hauteur[number])
  );
}

  // -- Dessiner des zigouigouis --

void Dessiner_zigouigoui(word x,word y, byte Couleur, short direction)
{
  word i;

  for (i=0; i<11; i++) Pixel_dans_fenetre(x,y+i,Couleur);
  x+=direction;
  for (i=1; i<10; i++) Pixel_dans_fenetre(x,y+i,Couleur);
  x+=direction+direction;
  for (i=3; i<8; i++) Pixel_dans_fenetre(x,y+i,Couleur);
  x+=direction+direction;
  Pixel_dans_fenetre(x,y+5,Couleur);
}

  // -- Dessiner un bloc de couleurs dégradé verticalement

void Bloc_degrade_dans_fenetre(word x_pos,word y_pos,word Debut_block,word Fin_block)
{
  word Total_lignes  =Menu_Facteur_Y<<6; // <=> à 64 lignes fct(Menu_Facteur)
  word Nb_couleurs   =(Debut_block<=Fin_block)?Fin_block-Debut_block+1:Debut_block-Fin_block+1;
  word Ligne_en_cours=(Debut_block<=Fin_block)?0:Total_lignes-1;

  word Debut_X       =Fenetre_Pos_X+(Menu_Facteur_X*x_pos);
  word Largeur_ligne =Menu_Facteur_X<<4; // <=> à 16 pixels fct(Menu_Facteur)

  word Debut_Y       =Fenetre_Pos_Y+(Menu_Facteur_Y*y_pos);
  word Fin_Y         =Debut_Y+Total_lignes;
  word Indice;

  if (Debut_block>Fin_block)
  {
    Indice=Debut_block;
    Debut_block=Fin_block;
    Fin_block=Indice;
  }

  for (Indice=Debut_Y;Indice<Fin_Y;Indice++,Ligne_en_cours++)
    Block(Debut_X,Indice,Largeur_ligne,1,Debut_block+(Nb_couleurs*Ligne_en_cours)/Total_lignes);

  UpdateRect(ToWinX(x_pos),ToWinY(y_pos),ToWinL(16),ToWinH(64));
}



  // -- Dessiner un petit sprite représentant le type d'un drive --

void Fenetre_Afficher_sprite_drive(word x_pos,word y_pos,byte type)
{
  word i,j;

  for (j=0; j<HAUTEUR_SPRITE_DRIVE; j++)
    for (i=0; i<LARGEUR_SPRITE_DRIVE; i++)
      Pixel_dans_fenetre(x_pos+i,y_pos+j,SPRITE_DRIVE[type][j][i]);
  UpdateRect(ToWinX(x_pos),ToWinY(y_pos),ToWinL(LARGEUR_SPRITE_DRIVE),ToWinH(HAUTEUR_SPRITE_DRIVE));
}



void Afficher_palette_du_menu_en_evitant_la_fenetre(byte * table)
{
  // On part du principe qu'il n'y a que le bas d'une fenêtre qui puisse
  // empiéter sur la palette... Et c'est déjà pas mal!
  word Couleur,Vraie_couleur;
  word Debut_X,Debut_Y;
  word Fin_X,Fin_Y;
  word width;
  word height;
  word Coin_X=Fenetre_Pos_X+Fenetre_Largeur*Menu_Facteur_X; // |_ Coin bas-droit
  word Coin_Y=Fenetre_Pos_Y+Fenetre_Hauteur*Menu_Facteur_Y; // |  de la fenêtre +1


  if (Config.Couleurs_separees)
  {
    width=(Menu_Taille_couleur-1)*Menu_Facteur_X;
    height=Menu_Facteur_Y*(32/Menu_Cellules_Y-1);
  }
  else
  {
    width=Menu_Taille_couleur*Menu_Facteur_X;
    height=Menu_Facteur_Y*(32/Menu_Cellules_Y);
  }

  for (Couleur=0,Vraie_couleur=Couleur_debut_palette;Couleur<Menu_Cellules_X*Menu_Cellules_Y;Couleur++,Vraie_couleur++)
  {
    if (table[Vraie_couleur]!=Vraie_couleur)
    {
      Debut_X=(LARGEUR_MENU+1+(Couleur/Menu_Cellules_Y)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee_avant_fenetre+((2+((Couleur%Menu_Cellules_Y)*(32/Menu_Cellules_Y)))*Menu_Facteur_Y);
      Fin_X=Debut_X+width;
      Fin_Y=Debut_Y+height;

      //   On affiche le bloc en entier si on peut, sinon on le découpe autour
      // de la fenêtre.
      if ( (Debut_Y>=Coin_Y) || (Fin_X<=Fenetre_Pos_X) || (Debut_X>=Coin_X) )
        Block(Debut_X,Debut_Y,width,height,Vraie_couleur);
      else
      {

        if (Debut_X>=Fenetre_Pos_X)
        {
          if ( (Fin_X>Coin_X) || (Fin_Y>Coin_Y) )
          {
            if ( (Fin_X>Coin_X) && (Fin_Y>Coin_Y) )
            {
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,width,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              if (Fin_Y>Coin_Y)
                Block(Debut_X,Coin_Y,width,Fin_Y-Coin_Y,Vraie_couleur);
              else
                Block(Coin_X,Debut_Y,Fin_X-Coin_X,height,Vraie_couleur);
            }
          }
        }
        else
        {
          if (Fin_X<Coin_X)
          {
            if (Fin_Y>Coin_Y)
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,width,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,height,Vraie_couleur);
          }
          else
          {
            if (Fin_Y>Coin_Y)
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,width,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,height,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,height,Vraie_couleur);
            }
          }
        }
      }
      {
        // Affichage du bloc directement dans le "buffer de fond" de la fenetre.
        // Cela permet au bloc de couleur d'apparaitre si on déplace la fenetre.
        short x_pos;
        short y_pos;
        short relative_x; // besoin d'une variable signée
        short relative_y; // besoin d'une variable signée
        // Attention aux unités
        relative_x = ((short)Debut_X - (short)Fenetre_Pos_X);
        relative_y = ((short)Debut_Y - (short)Fenetre_Pos_Y);

        for (y_pos=relative_y;y_pos<(relative_y+height)&&y_pos<Fenetre_Hauteur*Menu_Facteur_Y;y_pos++)
          for (x_pos=relative_x;x_pos<(relative_x+width)&&x_pos<Fenetre_Largeur*Menu_Facteur_X;x_pos++)
            if (x_pos>=0&&y_pos>=0)
              Pixel_fond(x_pos,y_pos,Vraie_couleur);
      }
    }
  }
  UpdateRect(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee_avant_fenetre,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y);
}

// -------- Calcul des bornes de la partie d'image visible à l'écran ---------
void Calculer_limites(void)
/*
  Avant l'appel à cette fonction, les données de la loupe doivent être à jour.
*/
{
  if (Loupe_Mode)
  {
    // -- Calcul des limites de la partie non zoomée de l'image --
    Limite_Haut  =Principal_Decalage_Y;
    Limite_Gauche=Principal_Decalage_X;
    Limite_visible_Bas   =Limite_Haut+Menu_Ordonnee-1;
    Limite_visible_Droite=Limite_Gauche+Principal_Split-1;

    if (Limite_visible_Bas>=Principal_Hauteur_image)
      Limite_Bas=Principal_Hauteur_image-1;
    else
      Limite_Bas=Limite_visible_Bas;

    if (Limite_visible_Droite>=Principal_Largeur_image)
      Limite_Droite=Principal_Largeur_image-1;
    else
      Limite_Droite=Limite_visible_Droite;

    // -- Calcul des limites de la partie zoomée de l'image --
    Limite_Haut_Zoom  =Loupe_Decalage_Y;
    Limite_Gauche_Zoom=Loupe_Decalage_X;
    Limite_visible_Bas_Zoom   =Limite_Haut_Zoom+Loupe_Hauteur-1;
    Limite_visible_Droite_Zoom=Limite_Gauche_Zoom+Loupe_Largeur-1;

    if (Limite_visible_Bas_Zoom>=Principal_Hauteur_image)
      Limite_Bas_Zoom=Principal_Hauteur_image-1;
    else
      Limite_Bas_Zoom=Limite_visible_Bas_Zoom;

    if (Limite_visible_Droite_Zoom>=Principal_Largeur_image)
      Limite_Droite_Zoom=Principal_Largeur_image-1;
    else
      Limite_Droite_Zoom=Limite_visible_Droite_Zoom;
  }
  else
  {
    // -- Calcul des limites de la partie visible de l'image --
    Limite_Haut  =Principal_Decalage_Y;
    Limite_Gauche=Principal_Decalage_X;
    Limite_visible_Bas   =Limite_Haut+(Menu_visible?Menu_Ordonnee:Hauteur_ecran)-1; // A REVOIR POUR SIMPLIFICATION
    Limite_visible_Droite=Limite_Gauche+Largeur_ecran-1;

    if (Limite_visible_Bas>=Principal_Hauteur_image)
      Limite_Bas=Principal_Hauteur_image-1;
    else
      Limite_Bas=Limite_visible_Bas;

    if (Limite_visible_Droite>=Principal_Largeur_image)
      Limite_Droite=Principal_Largeur_image-1;
    else
      Limite_Droite=Limite_visible_Droite;
  }
}


// -- Calculer les coordonnées du pinceau en fonction du snap et de la loupe -
void Calculer_coordonnees_pinceau(void)
{
  if ((Loupe_Mode) && (Mouse_X>=Principal_X_Zoom))
  {
    Pinceau_X=((Mouse_X-Principal_X_Zoom)/Loupe_Facteur)+Loupe_Decalage_X;
    Pinceau_Y=(Mouse_Y/Loupe_Facteur)+Loupe_Decalage_Y;
  }
  else
  {
    Pinceau_X=Mouse_X+Principal_Decalage_X;
    Pinceau_Y=Mouse_Y+Principal_Decalage_Y;
  }

  if (Snap_Mode)
  {
    Pinceau_X=(((Pinceau_X+(Snap_Largeur>>1)-Snap_Decalage_X)/Snap_Largeur)*Snap_Largeur)+Snap_Decalage_X;
    Pinceau_Y=(((Pinceau_Y+(Snap_Hauteur>>1)-Snap_Decalage_Y)/Snap_Hauteur)*Snap_Hauteur)+Snap_Decalage_Y;
  }
}



// -- Affichage de la limite de l'image -------------------------------------
void Afficher_limites_de_l_image(void)
{
  short start;
  short pos;
  short end;
  byte Droite_visible;
  byte Bas_visible;
  short Ancienne_Limite_Zoom;

  Droite_visible=Principal_Largeur_image<((Loupe_Mode)?Principal_Split:Largeur_ecran);
  Bas_visible   =Principal_Hauteur_image<Menu_Ordonnee;


  // On vérifie que la limite à droite est visible:
  if (Droite_visible)
  {
    start=Limite_Haut;
    end=(Limite_Bas<Principal_Hauteur_image)?
        Limite_Bas:Principal_Hauteur_image;

    if (Bas_visible)
      end++;

    // Juste le temps d'afficher les limites, on étend les limites de la loupe
    // aux limites visibles, car sinon Pixel_Preview ne voudra pas afficher.
    Ancienne_Limite_Zoom=Limite_Droite_Zoom;
    Limite_Droite_Zoom=Limite_visible_Droite_Zoom;

    for (pos=start;pos<=end;pos++)
      Pixel_Preview(Principal_Largeur_image,pos,((pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    UpdateRect(Principal_Largeur_image,start,1,end-start + 1);
    // On restaure la bonne valeur des limites
    Limite_Droite_Zoom=Ancienne_Limite_Zoom;
  }

  // On vérifie que la limite en bas est visible:
  if (Bas_visible)
  {
    start=Limite_Gauche;
    end=(Limite_Droite<Principal_Largeur_image)?
        Limite_Droite:Principal_Largeur_image;

    // On étend également les limites en bas (comme pour la limite droit)
    Ancienne_Limite_Zoom=Limite_Bas_Zoom;
    Limite_Bas_Zoom=Limite_visible_Bas_Zoom;

    for (pos=start;pos<=end;pos++)
      Pixel_Preview(pos,Principal_Hauteur_image,((pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    UpdateRect(start,Principal_Hauteur_image,end-start + 1,1);

    // On restaure la bonne valeur des limites
    Limite_Bas_Zoom=Ancienne_Limite_Zoom;
  }
}



// -- Recadrer la partie non-zoomée de l'image par rapport à la partie zoomée
//    lorsqu'on scrolle en mode Loupe --
void Recadrer_ecran_par_rapport_au_zoom(void)
{
  // Centrage en X
  if (Principal_Largeur_image>Principal_Split)
  {
    Principal_Decalage_X=Loupe_Decalage_X+(Loupe_Largeur>>1)
                         -(Principal_Split>>1);
    if (Principal_Decalage_X<0)
      Principal_Decalage_X=0;
    else
    if (Principal_Largeur_image<Principal_Decalage_X+Principal_Split)
      Principal_Decalage_X=Principal_Largeur_image-Principal_Split;
  }
  else
    Principal_Decalage_X=0;

  // Centrage en Y
  if (Principal_Hauteur_image>Menu_Ordonnee)
  {
    Principal_Decalage_Y=Loupe_Decalage_Y+(Loupe_Hauteur>>1)
                         -(Menu_Ordonnee>>1);
    if (Principal_Decalage_Y<0)
      Principal_Decalage_Y=0;
    else
    if (Principal_Hauteur_image<Principal_Decalage_Y+Menu_Ordonnee)
      Principal_Decalage_Y=Principal_Hauteur_image-Menu_Ordonnee;
  }
  else
    Principal_Decalage_Y=0;
}


// - Calcul des données du split en fonction de la proportion de chaque zone -
void Calculer_split(void)
{
  //short Temp;
  short X_theorique=Round(Principal_Proportion_split*Largeur_ecran);

  Principal_X_Zoom=Largeur_ecran-(((Largeur_ecran+(Loupe_Facteur>>1)-X_theorique)/Loupe_Facteur)*Loupe_Facteur);
  Principal_Split=Principal_X_Zoom-(Menu_Facteur_X*LARGEUR_BARRE_SPLIT);

  // Correction en cas de débordement sur la gauche
  while (Principal_Split*(Loupe_Facteur+1)<Largeur_ecran-(Menu_Facteur_X*LARGEUR_BARRE_SPLIT))
  {
    Principal_Split+=Loupe_Facteur;
    Principal_X_Zoom+=Loupe_Facteur;
  }
  // Correction en cas de débordement sur la droite
  X_theorique=Largeur_ecran-((NB_PIXELS_ZOOMES_MIN-1)*Loupe_Facteur);
  while (Principal_X_Zoom>=X_theorique)
  {
    Principal_Split-=Loupe_Facteur;
    Principal_X_Zoom-=Loupe_Facteur;
  }
}



// -------------------- Calcul des information de la loupe -------------------
void Calculer_donnees_loupe(void)
/*
  Après modification des données de la loupe, il faut recalculer les limites.
*/
{
  Calculer_split();

  Loupe_Largeur=(Largeur_ecran-Principal_X_Zoom)/Loupe_Facteur;

  Loupe_Hauteur=Menu_Ordonnee/Loupe_Facteur;
  if (Menu_Ordonnee%Loupe_Facteur)
    Loupe_Hauteur++;

  if (Loupe_Mode && Loupe_Decalage_X)
  {
    if (Principal_Largeur_image<Loupe_Decalage_X+Loupe_Largeur)
      Loupe_Decalage_X=Principal_Largeur_image-Loupe_Largeur;
    if (Loupe_Decalage_X<0) Loupe_Decalage_X=0;
  }
  if (Loupe_Mode && Loupe_Decalage_Y)
  {
    if (Principal_Hauteur_image<Loupe_Decalage_Y+Loupe_Hauteur)
      Loupe_Decalage_Y=Principal_Hauteur_image-Loupe_Hauteur;
    if (Loupe_Decalage_Y<0) Loupe_Decalage_Y=0;
  }
  
}



// ------------ Changer le facteur de zoom et tout mettre à jour -------------
void Changer_facteur_loupe(byte Indice_facteur)
{
  short Centre_X;
  short Centre_Y;

  Centre_X=Loupe_Decalage_X+(Loupe_Largeur>>1);
  Centre_Y=Loupe_Decalage_Y+(Loupe_Hauteur>>1);

  Loupe_Facteur=FACTEUR_ZOOM[Indice_facteur];
  Table_mul_facteur_zoom=TABLE_ZOOM[Indice_facteur];
  Calculer_donnees_loupe();

  if (Loupe_Mode)
  {
    // Recalculer le décalage de la loupe
    // Centrage "brut" de lécran par rapport à la loupe
    Loupe_Decalage_X=Centre_X-(Loupe_Largeur>>1);
    Loupe_Decalage_Y=Centre_Y-(Loupe_Hauteur>>1);
    // Correction en cas de débordement de l'image
    if (Loupe_Decalage_X+Loupe_Largeur>Principal_Largeur_image)
      Loupe_Decalage_X=Principal_Largeur_image-Loupe_Largeur;
    if (Loupe_Decalage_Y+Loupe_Hauteur>Principal_Hauteur_image)
      Loupe_Decalage_Y=Principal_Hauteur_image-Loupe_Hauteur;
    if (Loupe_Decalage_X<0)
      Loupe_Decalage_X=0;
    if (Loupe_Decalage_Y<0)
      Loupe_Decalage_Y=0;

    Recadrer_ecran_par_rapport_au_zoom();

    Pixel_Preview=Pixel_Preview_Loupe;

  }
  else
    Pixel_Preview=Pixel_Preview_Normal;

  Calculer_limites();
  Calculer_coordonnees_pinceau();
}



  // -- Afficher la barre de séparation entre les parties zoomées ou non en
  //    mode Loupe --

void Afficher_barre_de_split(void)
{
  // Partie grise du milieu
  Block(Principal_Split+(Menu_Facteur_X<<1),Menu_Facteur_Y,
        (LARGEUR_BARRE_SPLIT-4)*Menu_Facteur_X,
        Menu_Ordonnee-(Menu_Facteur_Y<<1),CM_Clair);

  // Barre noire de gauche
  Block(Principal_Split,0,Menu_Facteur_X,Menu_Ordonnee,CM_Noir);

  // Barre noire de droite
  Block(Principal_X_Zoom-Menu_Facteur_X,0,Menu_Facteur_X,Menu_Ordonnee,CM_Noir);

  // Bord haut (blanc)
  Block(Principal_Split+Menu_Facteur_X,0,
        (LARGEUR_BARRE_SPLIT-3)*Menu_Facteur_X,Menu_Facteur_Y,CM_Blanc);

  // Bord gauche (blanc)
  Block(Principal_Split+Menu_Facteur_X,Menu_Facteur_Y,
        Menu_Facteur_X,(Menu_Ordonnee-(Menu_Facteur_Y<<1)),CM_Blanc);

  // Bord droite (gris foncé)
  Block(Principal_X_Zoom-(Menu_Facteur_X<<1),Menu_Facteur_Y,
        Menu_Facteur_X,(Menu_Ordonnee-(Menu_Facteur_Y<<1)),CM_Fonce);

  // Bord bas (gris foncé)
  Block(Principal_Split+(Menu_Facteur_X<<1),Menu_Ordonnee-Menu_Facteur_Y,
        (LARGEUR_BARRE_SPLIT-3)*Menu_Facteur_X,Menu_Facteur_Y,CM_Fonce);

  // Coin bas gauche
  Block(Principal_Split+Menu_Facteur_X,Menu_Ordonnee-Menu_Facteur_Y,
        Menu_Facteur_X,Menu_Facteur_Y,CM_Clair);
  // Coin haut droite
  Block(Principal_X_Zoom-(Menu_Facteur_X<<1),0,
        Menu_Facteur_X,Menu_Facteur_Y,CM_Clair);

  UpdateRect(Principal_Split,0,LARGEUR_BARRE_SPLIT*Menu_Facteur_X,Menu_Ordonnee); // On réaffiche toute la partie à gauche du split, ce qui permet d'effacer son ancienne position
}



// -- Fonctions de manipulation du curseur -----------------------------------


  // -- Afficher une barre horizontale XOR zoomée

void Ligne_horizontale_XOR_Zoom(short x_pos, short y_pos, short width)
{
  short Pos_X_reelle=Principal_X_Zoom+(x_pos-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(y_pos-Loupe_Decalage_Y)*Loupe_Facteur;
  short Largeur_reelle=width*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+Loupe_Facteur<Menu_Ordonnee)?Pos_Y_reelle+Loupe_Facteur:Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Largeur_reelle);

  UpdateRect(Pos_X_reelle,Pos_Y_reelle,Largeur_reelle,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher une barre verticale XOR zoomée

void Ligne_verticale_XOR_Zoom(short x_pos, short y_pos, short height)
{
  short Pos_X_reelle=Principal_X_Zoom+(x_pos-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(y_pos-Loupe_Decalage_Y)*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+(height*Loupe_Facteur<Menu_Ordonnee))?Pos_Y_reelle+(height*Loupe_Facteur):Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Loupe_Facteur);

  UpdateRect(Pos_X_reelle,Pos_Y_reelle,Loupe_Facteur,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher le curseur --

void Afficher_curseur(void)
{
  byte  shape;
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short x_pos;
  short y_pos;
  short Compteur_X;
  short Compteur_Y;
  int   Temp;
  byte  Couleur;
  float cosA,sinA;
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;

  // Si le curseur est dans le menu ou sur la barre de split, on affiche toujours une flèche.
  if ( ( (Mouse_Y<Menu_Ordonnee)
      && ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
    || (Fenetre) || (Forme_curseur==FORME_CURSEUR_SABLIER) )
    shape=Forme_curseur;
  else
    shape=FORME_CURSEUR_FLECHE;

  switch(shape)
  {
    case FORME_CURSEUR_CIBLE :
      if (!Cacher_pinceau)
        Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,1);
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          Debut_Y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (Debut_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-6,4-Debut_Y);

          Debut_X=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (Debut_X<4)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-6,Mouse_Y,4-Debut_X);

          Fin_X=(Mouse_X+7>Largeur_ecran)?Mouse_X+7-Largeur_ecran:0;
          if (Fin_X<4)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,4-Fin_X);

          Fin_Y=(Mouse_Y+7>Hauteur_ecran)?Mouse_Y+7-Hauteur_ecran:0;
          if (Fin_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,4-Fin_Y);

          UpdateRect(Mouse_X+Debut_X-6,Mouse_Y+Debut_Y-6,13-Fin_X,13-Fin_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_FINE:FORME_CURSEUR_CIBLE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15 && x_pos < Largeur_ecran;x_pos++,Compteur_X++)
                {
                  if( x_pos < 0 ) continue;
            for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15 && y_pos < Hauteur_ecran;y_pos++,Compteur_Y++)
            {
                    if( y_pos < 0 || y_pos >= Hauteur_ecran) continue;
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(x_pos,y_pos);
              if (Couleur!=CM_Trans)
                Pixel(x_pos,y_pos,Couleur);
            }
                }

          UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Compteur_X,Compteur_Y);
        }
      }
      break;

    case FORME_CURSEUR_CIBLE_PIPETTE:
      if (!Cacher_pinceau)
        Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,1);
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          // Barres formant la croix principale

          Debut_Y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (Debut_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-5,3-Debut_Y);

          Debut_X=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (Debut_X<3)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-5,Mouse_Y,3-Debut_X);

          Fin_X=(Mouse_X+6>Largeur_ecran)?Mouse_X+6-Largeur_ecran:0;
          if (Fin_X<3)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,3-Fin_X);

          Fin_Y=(Mouse_Y+6>Menu_Ordonnee/*Hauteur_ecran*/)?Mouse_Y+6-Menu_Ordonnee/*Hauteur_ecran*/:0;
          if (Fin_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,3-Fin_Y);

          // Petites barres aux extrémités

          Debut_X=(!Mouse_X);
          Debut_Y=(!Mouse_Y);
          Fin_X=(Mouse_X>=Largeur_ecran-1);
          Fin_Y=(Mouse_Y>=Menu_Ordonnee-1);

          if (Mouse_Y>5)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y-6,3-(Debut_X+Fin_X));

          if (Mouse_X>5)
            Ligne_verticale_XOR  (Mouse_X-6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_X<Largeur_ecran-6)
            Ligne_verticale_XOR  (Mouse_X+6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_Y<Menu_Ordonnee-6)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y+6,3-(Debut_X+Fin_X));
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_PIPETTE_FINE:FORME_CURSEUR_CIBLE_PIPETTE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15;x_pos++,Compteur_X++)
            for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15;y_pos++,Compteur_Y++)
            {
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              if ( (x_pos>=0) && (x_pos<Largeur_ecran)
                && (y_pos>=0) && (y_pos<Hauteur_ecran) )
              {
                FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(x_pos,y_pos);
                if (Couleur!=CM_Trans)
                  Pixel(x_pos,y_pos,Couleur);
              }
            }
          UpdateRect(Debut_X,Debut_Y,16,16);
        }
      }
      break;

    case FORME_CURSEUR_MULTIDIRECTIONNEL :
    case FORME_CURSEUR_HORIZONTAL :
      if (Cacher_curseur)
        break;

    case FORME_CURSEUR_FLECHE :
    case FORME_CURSEUR_SABLIER :
      Debut_X=Mouse_X-Curseur_Decalage_X[shape];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[shape];
      for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15;x_pos++,Compteur_X++)
      {
        if(x_pos<0) continue;
        if(x_pos>=Largeur_ecran) break;
        for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15;y_pos++,Compteur_Y++)
        {
          if(y_pos<0) continue;
          if(y_pos>=Hauteur_ecran) break;
          Couleur=SPRITE_CURSEUR[shape][Compteur_Y][Compteur_X];
          // On sauvegarde dans FOND_CURSEUR pour restaurer plus tard
          FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(x_pos,y_pos);
          if (Couleur!=CM_Trans)
              Pixel(x_pos,y_pos,Couleur);
        }
      }
      UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Compteur_X,Compteur_Y);
      break;

    case FORME_CURSEUR_CIBLE_XOR :
      x_pos=Pinceau_X-Principal_Decalage_X;
      y_pos=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // width de la barre XOR
      if ((y_pos<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        UpdateRect(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((x_pos<Compteur_X) && (Pinceau_X>=Limite_Gauche))
      {
        Ligne_verticale_XOR(Pinceau_X-Principal_Decalage_X,0,Menu_Ordonnee);
        UpdateRect(Pinceau_X-Principal_Decalage_X,0,1,Menu_Ordonnee);
      }

      if (Loupe_Mode)
      {
        // UPDATERECT
        if ((Pinceau_Y>=Limite_Haut_Zoom) && (Pinceau_Y<=Limite_visible_Bas_Zoom))
          Ligne_horizontale_XOR_Zoom(Limite_Gauche_Zoom,Pinceau_Y,Loupe_Largeur);
        if ((Pinceau_X>=Limite_Gauche_Zoom) && (Pinceau_X<=Limite_visible_Droite_Zoom))
          Ligne_verticale_XOR_Zoom(Pinceau_X,Limite_Haut_Zoom,Loupe_Hauteur);
      }
      break;
    case FORME_CURSEUR_RECTANGLE_XOR :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      Debut_X=(Mouse_X-3);
      Debut_Y=(Mouse_Y-3);
      Fin_X  =(Mouse_X+4);
      Fin_Y  =(Mouse_Y+4);
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      if (Fin_X>Largeur_ecran)
        Fin_X=Largeur_ecran;
      if (Fin_Y>Menu_Ordonnee)
        Fin_Y=Menu_Ordonnee;

      Ligne_horizontale_XOR(Debut_X,Mouse_Y,Fin_X-Debut_X);
      Ligne_verticale_XOR  (Mouse_X,Debut_Y,Fin_Y-Debut_Y);

      // Grand rectangle autour
      Debut_X=Mouse_X-(Loupe_Largeur>>1);
      Debut_Y=Mouse_Y-(Loupe_Hauteur>>1);
      if (Debut_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
        Debut_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
      if (Debut_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
        Debut_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      Fin_X=Debut_X+Loupe_Largeur-1;
      Fin_Y=Debut_Y+Loupe_Hauteur-1;

      Ligne_horizontale_XOR(Debut_X,Debut_Y,Loupe_Largeur);
      Ligne_verticale_XOR(Debut_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_verticale_XOR(  Fin_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_horizontale_XOR(Debut_X,  Fin_Y,Loupe_Largeur);

      UpdateRect(Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);

      break;
    default: //case FORME_CURSEUR_ROTATE_XOR :
      Debut_X=1-(Brosse_Largeur>>1);
      Debut_Y=1-(Brosse_Hauteur>>1);
      Fin_X=Debut_X+Brosse_Largeur-1;
      Fin_Y=Debut_Y+Brosse_Hauteur-1;

      if (Brosse_Centre_rotation_defini)
      {
        if ( (Brosse_Centre_rotation_X==Pinceau_X)
          && (Brosse_Centre_rotation_Y==Pinceau_Y) )
        {
          cosA=1.0;
          sinA=0.0;
        }
        else
        {
          x_pos=Pinceau_X-Brosse_Centre_rotation_X;
          y_pos=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)x_pos/sqrt((x_pos*x_pos)+(y_pos*y_pos));
          sinA=sin(acos(cosA));
          if (y_pos>0) sinA=-sinA;
        }

        Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
        Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
        Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
        Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

        X1+=Brosse_Centre_rotation_X;
        Y1+=Brosse_Centre_rotation_Y;
        X2+=Brosse_Centre_rotation_X;
        Y2+=Brosse_Centre_rotation_Y;
        X3+=Brosse_Centre_rotation_X;
        Y3+=Brosse_Centre_rotation_Y;
        X4+=Brosse_Centre_rotation_X;
        Y4+=Brosse_Centre_rotation_Y;
        Pixel_figure_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,0);
        Tracer_ligne_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,Pinceau_X,Pinceau_Y,0);
      }
      else
      {
        X1=X3=1-Brosse_Largeur;
        Y1=Y2=Debut_Y;
        X2=X4=Pinceau_X;
        Y3=Y4=Fin_Y;

        X1+=Pinceau_X;
        Y1+=Pinceau_Y;
        Y2+=Pinceau_Y;
        X3+=Pinceau_X;
        Y3+=Pinceau_Y;
        Y4+=Pinceau_Y;
        Pixel_figure_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,0);
        Tracer_ligne_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,Pinceau_X,Pinceau_Y,0);
      }

      Tracer_ligne_Preview_xor(X1,Y1,X2,Y2,0);
      Tracer_ligne_Preview_xor(X2,Y2,X4,Y4,0);
      Tracer_ligne_Preview_xor(X4,Y4,X3,Y3,0);
      Tracer_ligne_Preview_xor(X3,Y3,X1,Y1,0);
  }
}

  // -- Effacer le curseur --

void Effacer_curseur(void)
{
  byte  shape;
  int Debut_X; // int car sont parfois négatifs ! (quand on dessine sur un bord)
  int Debut_Y;
  short Fin_X;
  short Fin_Y;
  int x_pos;
  int y_pos;
  short Compteur_X;
  short Compteur_Y;
  //short Fin_Compteur_X; // Position X ou s'arrête l'affichage de la brosse/pinceau
  //short Fin_Compteur_Y; // Position Y ou s'arrête l'affichage de la brosse/pinceau
  int   Temp;
  //byte  Couleur;
  float cosA,sinA;
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;

  if ( ( (Mouse_Y<Menu_Ordonnee)
      && ( (!Loupe_Mode) || (Mouse_X<Principal_Split)
                         || (Mouse_X>=Principal_X_Zoom) ) )
    || (Fenetre) || (Forme_curseur==FORME_CURSEUR_SABLIER) )
    shape=Forme_curseur;
  else
    shape=FORME_CURSEUR_FLECHE;

  switch(shape)
  {
    case FORME_CURSEUR_CIBLE :
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          Debut_Y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (Debut_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-6,4-Debut_Y);

          Debut_X=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (Debut_X<4)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-6,Mouse_Y,4-Debut_X);

          Fin_X=(Mouse_X+7>Largeur_ecran)?Mouse_X+7-Largeur_ecran:0;
          if (Fin_X<4)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,4-Fin_X);

          Fin_Y=(Mouse_Y+7>Hauteur_ecran)?Mouse_Y+7-Hauteur_ecran:0;
          if (Fin_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,4-Fin_Y);

          UpdateRect(Mouse_X+Debut_X-6,Mouse_Y+Debut_Y-6,13-Fin_X,13-Fin_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_FINE:FORME_CURSEUR_CIBLE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15;y_pos++,Compteur_Y++)
          {
            if(y_pos>=Hauteur_ecran) break;
            for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15;x_pos++,Compteur_X++)
            {
              if ( (x_pos<0) || (y_pos < 0)) continue;
              else if (x_pos>=Largeur_ecran) break;
              Pixel(x_pos,y_pos,FOND_CURSEUR[Compteur_Y][Compteur_X]);
            }
          }

          UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),x_pos-Debut_X,y_pos-Debut_Y);
        }
      }
      if (!Cacher_pinceau)
      {
        Effacer_pinceau(Pinceau_X,Pinceau_Y);
      }
      break;

    case FORME_CURSEUR_CIBLE_PIPETTE:
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          // Barres formant la croix principale

          Debut_Y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (Debut_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-5,3-Debut_Y);

          Debut_X=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (Debut_X<3)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-5,Mouse_Y,3-Debut_X);

          Fin_X=(Mouse_X+6>Largeur_ecran)?Mouse_X+6-Largeur_ecran:0;
          if (Fin_X<3)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,3-Fin_X);

          Fin_Y=(Mouse_Y+6>Hauteur_ecran)?Mouse_Y+6-Hauteur_ecran:0;
          if (Fin_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,3-Fin_Y);

          Debut_X=(!Mouse_X);
          Debut_Y=(!Mouse_Y);
          Fin_X=(Mouse_X>=Largeur_ecran-1);
          Fin_Y=(Mouse_Y>=Menu_Ordonnee-1);

          if (Mouse_Y>5)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y-6,3-(Debut_X+Fin_X));

          if (Mouse_X>5)
            Ligne_verticale_XOR  (Mouse_X-6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_X<Largeur_ecran-6)
            Ligne_verticale_XOR  (Mouse_X+6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_Y<Menu_Ordonnee-6)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y+6,3-(Debut_X+Fin_X));

          UpdateRect(Debut_X,Debut_Y,Fin_X-Debut_X,Fin_Y-Debut_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_PIPETTE_FINE:FORME_CURSEUR_CIBLE_PIPETTE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15;x_pos++,Compteur_X++)
            for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15;y_pos++,Compteur_Y++)
              if ( (x_pos>=0) && (x_pos<Largeur_ecran) && (y_pos>=0) && (y_pos<Hauteur_ecran) )
                Pixel(x_pos,y_pos,FOND_CURSEUR[Compteur_Y][Compteur_X]);

          UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),16,16);
        }
      }
      if (!Cacher_pinceau)
        Effacer_pinceau(Pinceau_X,Pinceau_Y);
      break;

    case FORME_CURSEUR_MULTIDIRECTIONNEL :
    case FORME_CURSEUR_HORIZONTAL :
      if (Cacher_curseur)
        break;

    case FORME_CURSEUR_FLECHE :
    case FORME_CURSEUR_SABLIER :
      Debut_X=Mouse_X-Curseur_Decalage_X[shape];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[shape];

      for (x_pos=Debut_X,Compteur_X=0;Compteur_X<15;x_pos++,Compteur_X++)
      {
        if(x_pos<0) continue;
        if(x_pos>=Largeur_ecran) break;
        for (y_pos=Debut_Y,Compteur_Y=0;Compteur_Y<15;y_pos++,Compteur_Y++)
        {
            if(y_pos<0) continue;
            if(y_pos>=Hauteur_ecran) break;
            Pixel(x_pos,y_pos,FOND_CURSEUR[Compteur_Y][Compteur_X]);
        }
      }
      UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Compteur_X,Compteur_Y);
      break;

    case FORME_CURSEUR_CIBLE_XOR :
      x_pos=Pinceau_X-Principal_Decalage_X;
      y_pos=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // width de la barre XOR
      if ((y_pos<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        UpdateRect(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((x_pos<Compteur_X) && (Pinceau_X>=Limite_Gauche))
      {
        Ligne_verticale_XOR(Pinceau_X-Principal_Decalage_X,0,Menu_Ordonnee);
        UpdateRect(Pinceau_X-Principal_Decalage_X,0,1,Menu_Ordonnee);
      }

      if (Loupe_Mode)
      {
        // UPDATERECT
        if ((Pinceau_Y>=Limite_Haut_Zoom) && (Pinceau_Y<=Limite_visible_Bas_Zoom))
          Ligne_horizontale_XOR_Zoom(Limite_Gauche_Zoom,Pinceau_Y,Loupe_Largeur);
        if ((Pinceau_X>=Limite_Gauche_Zoom) && (Pinceau_X<=Limite_visible_Droite_Zoom))
          Ligne_verticale_XOR_Zoom(Pinceau_X,Limite_Haut_Zoom,Loupe_Hauteur);
      }


      break;
    case FORME_CURSEUR_RECTANGLE_XOR :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      Debut_X=(Mouse_X-3);
      Debut_Y=(Mouse_Y-3);
      Fin_X  =(Mouse_X+4);
      Fin_Y  =(Mouse_Y+4);
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      if (Fin_X>Largeur_ecran)
        Fin_X=Largeur_ecran;
      if (Fin_Y>Menu_Ordonnee)
        Fin_Y=Menu_Ordonnee;

      Ligne_horizontale_XOR(Debut_X,Mouse_Y,Fin_X-Debut_X);
      Ligne_verticale_XOR  (Mouse_X,Debut_Y,Fin_Y-Debut_Y);

      // Grand rectangle autour

      Debut_X=Mouse_X-(Loupe_Largeur>>1);
      Debut_Y=Mouse_Y-(Loupe_Hauteur>>1);
      if (Debut_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
        Debut_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
      if (Debut_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
        Debut_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      Fin_X=Debut_X+Loupe_Largeur-1;
      Fin_Y=Debut_Y+Loupe_Hauteur-1;

      Ligne_horizontale_XOR(Debut_X,Debut_Y,Loupe_Largeur);
      Ligne_verticale_XOR(Debut_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_verticale_XOR(  Fin_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_horizontale_XOR(Debut_X,  Fin_Y,Loupe_Largeur);

      UpdateRect(Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);

      break;
    default: //case FORME_CURSEUR_ROTATE_XOR :
      Debut_X=1-(Brosse_Largeur>>1);
      Debut_Y=1-(Brosse_Hauteur>>1);
      Fin_X=Debut_X+Brosse_Largeur-1;
      Fin_Y=Debut_Y+Brosse_Hauteur-1;

      if (Brosse_Centre_rotation_defini)
      {
        if ( (Brosse_Centre_rotation_X==Pinceau_X)
          && (Brosse_Centre_rotation_Y==Pinceau_Y) )
        {
          cosA=1.0;
          sinA=0.0;
        }
        else
        {
          x_pos=Pinceau_X-Brosse_Centre_rotation_X;
          y_pos=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)x_pos/sqrt((x_pos*x_pos)+(y_pos*y_pos));
          sinA=sin(acos(cosA));
          if (y_pos>0) sinA=-sinA;
        }

        Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
        Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
        Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
        Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

        X1+=Brosse_Centre_rotation_X;
        Y1+=Brosse_Centre_rotation_Y;
        X2+=Brosse_Centre_rotation_X;
        Y2+=Brosse_Centre_rotation_Y;
        X3+=Brosse_Centre_rotation_X;
        Y3+=Brosse_Centre_rotation_Y;
        X4+=Brosse_Centre_rotation_X;
        Y4+=Brosse_Centre_rotation_Y;
        Pixel_figure_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,0);
        Tracer_ligne_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,Pinceau_X,Pinceau_Y,0);
      }
      else
      {
        X1=X3=1-Brosse_Largeur;
        Y1=Y2=Debut_Y;
        X2=X4=Pinceau_X;
        Y3=Y4=Fin_Y;

        X1+=Pinceau_X;
        Y1+=Pinceau_Y;
        Y2+=Pinceau_Y;
        X3+=Pinceau_X;
        Y3+=Pinceau_Y;
        Y4+=Pinceau_Y;
        Pixel_figure_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,0);
        Tracer_ligne_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,Pinceau_X,Pinceau_Y,0);
      }

      Tracer_ligne_Preview_xor(X1,Y1,X2,Y2,0);
      Tracer_ligne_Preview_xor(X2,Y2,X4,Y4,0);
      Tracer_ligne_Preview_xor(X4,Y4,X3,Y3,0);
      Tracer_ligne_Preview_xor(X3,Y3,X1,Y1,0);
  }
}



// -- Fonction diverses d'affichage ------------------------------------------

  // -- Reafficher toute l'image (en prenant en compte le facteur de zoom) --

void Afficher_ecran(void)
{
  word width;
  word height;

  // ---/\/\/\  Partie non zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    if (Principal_Largeur_image<Principal_Split)
      width=Principal_Largeur_image;
    else
      width=Principal_Split;
  }
  else
  {
    if (Principal_Largeur_image<Largeur_ecran)
      width=Principal_Largeur_image;
    else
      width=Largeur_ecran;
  }
  if (Principal_Hauteur_image<Menu_Ordonnee)
    height=Principal_Hauteur_image;
  else
    height=Menu_Ordonnee;
  Display_screen(width,height,Principal_Largeur_image);

  // Effacement de la partie non-image dans la partie non zoomée:
  if (Loupe_Mode)
  {
    if (Principal_Largeur_image<Principal_Split && Principal_Largeur_image < Largeur_ecran)
      Block(Principal_Largeur_image,0,(Principal_Split-Principal_Largeur_image),Menu_Ordonnee,0);
  }
  else
  {
    if (Principal_Largeur_image<Largeur_ecran)
      Block(Principal_Largeur_image,0,(Largeur_ecran-Principal_Largeur_image),Menu_Ordonnee,0);
  }
  if (Principal_Hauteur_image<Menu_Ordonnee)
    Block(0,Principal_Hauteur_image,width,(Menu_Ordonnee-height),0);

  // ---/\/\/\  Partie zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    // Affichage de la barre de split
    Afficher_barre_de_split();

    // Calcul de la largeur visible
    if (Principal_Largeur_image<Loupe_Largeur)
      width=Principal_Largeur_image;
    else
      width=Loupe_Largeur;

    // Calcul du nombre de lignes visibles de l'image zoomée
    if (Principal_Hauteur_image<Loupe_Hauteur)
      height=Principal_Hauteur_image*Loupe_Facteur;
    else
      height=Menu_Ordonnee;

    Display_zoomed_screen(width,height,Principal_Largeur_image,Buffer_de_ligne_horizontale);

    // Effacement de la partie non-image dans la partie zoomée:
    if (Principal_Largeur_image<Loupe_Largeur)
      Block(Principal_X_Zoom+(Principal_Largeur_image*Loupe_Facteur),0,
            (Loupe_Largeur-Principal_Largeur_image)*Loupe_Facteur,
            Menu_Ordonnee,0);
    if (Principal_Hauteur_image<Loupe_Hauteur)
      Block(Principal_X_Zoom,height,width*Loupe_Facteur,(Menu_Ordonnee-height),0);
  }

  // ---/\/\/\ Affichage des limites /\/\/\---
  if (Config.Afficher_limites_image)
    Afficher_limites_de_l_image();
  UpdateRect(0,0,Largeur_ecran,Menu_Ordonnee); // TODO On peut faire plus fin, en évitant de mettre à jour la partie à droite du split quand on est en mode loupe. Mais c'est pas vraiment intéressant ?
}



byte Meilleure_couleur(byte r,byte g,byte b)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist=0x7FFFFFFF;
  byte  Best_color=0;

  for (Coul=0; Coul<256; Coul++)
  {
    if (!Exclude_color[Coul])
    {
      Delta_R=(int)Principal_Palette[Coul].R-r;
      Delta_V=(int)Principal_Palette[Coul].G-g;
      Delta_B=(int)Principal_Palette[Coul].B-b;

      if (!(Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11)))
        return Coul;

      if (Dist<Best_dist)
      {
        Best_dist=Dist;
        Best_color=Coul;
      }
    }
  }

  return Best_color;
}

byte Meilleure_couleur_sans_exclusion(byte Rouge,byte Vert,byte Bleu)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist=0x7FFFFFFF;
  byte  Best_color=0;

  for (Coul=0; Coul<256; Coul++)
  {
    Delta_R=(int)Principal_Palette[Coul].R-Rouge;
    Delta_V=(int)Principal_Palette[Coul].G-Vert;
    Delta_B=(int)Principal_Palette[Coul].B-Bleu;

    if (!(Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11)))
      return Coul;

    if (Dist<Best_dist)
    {
      Best_dist=Dist;
      Best_color=Coul;
    }
  }
  return Best_color;
}

void Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
     (byte Rouge, byte Vert, byte Bleu, Composantes * palette, byte * table)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};


  for (Coul=0; Coul<256; Coul++)
  {
    Delta_R=(int)palette[Coul].R-Rouge;
    Delta_V=(int)palette[Coul].G-Vert;
    Delta_B=(int)palette[Coul].B-Bleu;

    Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11);

    if (Dist<Best_dist[0])
    {
      Best_dist[3]=Best_dist[2];
      Best_dist[2]=Best_dist[1];
      Best_dist[1]=Best_dist[0];
      Best_dist[0]=Dist;
      table[3]=table[2];
      table[2]=table[1];
      table[1]=table[0];
      table[0]=Coul;
    }
    else
    {
      if (Dist<Best_dist[1])
      {
        Best_dist[3]=Best_dist[2];
        Best_dist[2]=Best_dist[1];
        Best_dist[1]=Dist;
        table[3]=table[2];
        table[2]=table[1];
        table[1]=Coul;
      }
      else
      {
        if (Dist<Best_dist[2])
        {
          Best_dist[3]=Best_dist[2];
          Best_dist[2]=Dist;
          table[3]=table[2];
          table[2]=Coul;
        }
        else
        if (Dist<Best_dist[3])
        {
          Best_dist[3]=Dist;
          table[3]=Coul;
        }
      }
    }
  }
}



byte Old_Noir;
byte Old_Fonce;
byte Old_Clair;
byte Old_Blanc;
byte Old_Trans;

void Remap_pixel(byte * Pix)
{
  if (*Pix==Old_Clair)         // On commence par tester le Gris clair
    *Pix=CM_Clair;             // qui est pas mal utilisé.
  else
  {
    if (*Pix==Old_Noir)        // Puis le Noir...
      *Pix=CM_Noir;
    else
    {
      if (*Pix==Old_Fonce)     // etc...
        *Pix=CM_Fonce;
      else
      {
        if (*Pix==Old_Blanc)
          *Pix=CM_Blanc;
        else
        {
          if (*Pix==Old_Trans)
            *Pix=CM_Trans;
        }
      }
    }
  }
}



void Remapper_ecran_apres_changement_couleurs_menu(void)
{
  short Indice;
  byte  Table_de_conversion[256];
  short Temp/*,Temp2*/;

  if ( (CM_Clair!=Old_Clair) || (CM_Fonce!=Old_Fonce) || (CM_Blanc!=Old_Blanc) || (CM_Noir !=Old_Noir )
    || (CM_Trans!=Old_Trans) )
  {
    // Création de la table de conversion
    for (Indice=0; Indice<256; Indice++)
      Table_de_conversion[Indice]=Indice;

    Table_de_conversion[Old_Noir ]=CM_Noir;
    Table_de_conversion[Old_Fonce]=CM_Fonce;
    Table_de_conversion[Old_Clair]=CM_Clair;
    Table_de_conversion[Old_Blanc]=CM_Blanc;

    // Remappage de l'écran

    Temp=Fenetre_Hauteur*Menu_Facteur_Y;

    Remap_screen(Fenetre_Pos_X, Fenetre_Pos_Y,
                 Fenetre_Largeur*Menu_Facteur_X,
                 (Fenetre_Pos_Y+Temp<Menu_Ordonnee_avant_fenetre)?Temp:Menu_Ordonnee_avant_fenetre-Fenetre_Pos_Y,
                 Table_de_conversion);

    if (Menu_visible_avant_fenetre)
    {
      Remap_screen(0, Menu_Ordonnee_avant_fenetre,
                   Largeur_ecran, Hauteur_ecran-Menu_Ordonnee_avant_fenetre,
                   Table_de_conversion);
      // Remappage de la partie du fond de la fenetre qui cacherait le menu...
      Remappe_fond_fenetres(Table_de_conversion, Menu_Ordonnee_avant_fenetre, Hauteur_ecran);
      /*
         Il faudrait peut-être remapper les pointillés délimitant l'image.
         Mais ça va être chiant parce qu'ils peuvent être affichés en mode Loupe.
         Mais de toutes façons, c'est franchement facultatif...
      */
      // On passe la table juste pour ne rafficher que les couleurs modifiées
      Afficher_palette_du_menu_en_evitant_la_fenetre(Table_de_conversion);
    }
  }

}




void Calculer_couleurs_menu_optimales(Composantes * palette)
{
  byte table[4];
  short i,j,k;


  Old_Noir =CM_Noir;
  Old_Fonce=CM_Fonce;
  Old_Clair=CM_Clair;
  Old_Blanc=CM_Blanc;
  Old_Trans=CM_Trans;

  // Recherche du noir
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[0].R, Coul_menu_pref[0].G, Coul_menu_pref[0].B,palette,table);
  CM_Noir=table[0];

  // Recherche du blanc
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[3].R, Coul_menu_pref[3].G, Coul_menu_pref[3].B,palette,table);
  if (CM_Noir!=table[0])
    CM_Blanc=table[0];
  else
    CM_Blanc=table[1];

  // Recherche du gris clair
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[2].R, Coul_menu_pref[2].G, Coul_menu_pref[2].B,palette,table);
  if ( (CM_Noir!=table[0]) && (CM_Blanc!=table[0]) )
    CM_Clair=table[0];
  else
  {
    if ( (CM_Noir!=table[1]) && (CM_Blanc!=table[1]) )
      CM_Clair=table[1];
    else
      CM_Clair=table[2];
  }

  // Recherche du gris foncé
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[1].R, Coul_menu_pref[1].G, Coul_menu_pref[1].B,palette,table);
  if ( (CM_Noir!=table[0]) && (CM_Blanc!=table[0]) && (CM_Clair!=table[0]) )
    CM_Fonce=table[0];
  else
  {
    if ( (CM_Noir!=table[1]) && (CM_Blanc!=table[1]) && (CM_Clair!=table[1]) )
      CM_Fonce=table[1];
    else
    {
      if ( (CM_Noir!=table[2]) && (CM_Blanc!=table[2]) && (CM_Clair!=table[2]) )
        CM_Fonce=table[2];
      else
        CM_Fonce=table[3];
    }
  }

  // C'est peu probable mais il est possible que CM_Clair soit plus foncée que
  // CM_Fonce. Dans ce cas, on les inverse.
  if ( ((palette[CM_Clair].R*30)+(palette[CM_Clair].G*59)+(palette[CM_Clair].B*11)) <
       ((palette[CM_Fonce].R*30)+(palette[CM_Fonce].G*59)+(palette[CM_Fonce].B*11)) )
  {
    i=CM_Clair;
    CM_Clair=CM_Fonce;
    CM_Fonce=i;
  }

  // On cherche une couleur de transparence différente des 4 autres.
  for (CM_Trans=0; ((CM_Trans==CM_Noir) || (CM_Trans==CM_Fonce) ||
                   (CM_Trans==CM_Clair) || (CM_Trans==CM_Blanc)); CM_Trans++);

  // Et maintenant, on "remappe" tous les sprites, etc...
  if ( (CM_Clair!=Old_Clair)
    || (CM_Fonce!=Old_Fonce)
    || (CM_Blanc!=Old_Blanc)
    || (CM_Noir !=Old_Noir )
    || (CM_Trans!=Old_Trans) )
  {
    // Sprites du curseur
    for (k=0; k<NB_SPRITES_CURSEUR; k++)
      for (j=0; j<HAUTEUR_SPRITE_CURSEUR; j++)
        for (i=0; i<LARGEUR_SPRITE_CURSEUR; i++)
          Remap_pixel(&SPRITE_CURSEUR[k][j][i]);
    // Le menu
    for (j=0; j<HAUTEUR_MENU; j++)
      for (i=0; i<LARGEUR_MENU; i++)
        Remap_pixel(&BLOCK_MENU[j][i]);
    // Sprites du menu
    for (k=0; k<NB_SPRITES_MENU; k++)
      for (j=0; j<HAUTEUR_SPRITE_MENU; j++)
        for (i=0; i<LARGEUR_SPRITE_MENU; i++)
          Remap_pixel(&SPRITE_MENU[k][j][i]);
    // Sprites d'effets
    for (k=0; k<NB_SPRITES_EFFETS; k++)
      for (j=0; j<HAUTEUR_SPRITE_MENU; j++)
        for (i=0; i<LARGEUR_SPRITE_MENU; i++)
          Remap_pixel(&SPRITE_EFFET[k][j][i]);
    // Fontes de l'aide
    for (k=0; k<256; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_norm[k][i][j]);
    for (k=0; k<256; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_bold[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_t1[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_t2[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_t3[k][i][j]);
    for (k=0; k<64; k++)
      for (j=0; j<8; j++)
        for (i=0; i<6; i++)
          Remap_pixel(&Fonte_help_t4[k][i][j]);
        
    // Sprites de lecteurs (drives)
    for (k=0; k<NB_SPRITES_DRIVES; k++)
      for (j=0; j<HAUTEUR_SPRITE_DRIVE; j++)
        for (i=0; i<LARGEUR_SPRITE_DRIVE; i++)
          Remap_pixel(&SPRITE_DRIVE[k][j][i]);
  }
}
