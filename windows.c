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

// Affichage d'un pixel dans le menu (le menu doit être visible)
void Pixel_dans_barre_d_outil(word X,word Y,byte Couleur)
{
  Block(X*Menu_Facteur_X,(Y*Menu_Facteur_Y)+Menu_Ordonnee,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}

  // Affichage d'un pixel dans la fenêtre (la fenêtre doit être visible)

void Pixel_dans_fenetre(word X,word Y,byte Couleur)
{
    Block((X*Menu_Facteur_X)+Fenetre_Pos_X,(Y*Menu_Facteur_Y)+Fenetre_Pos_Y,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}


// -- Affichages de différents cadres dans une fenêtre -----------------------

  // -- Cadre général avec couleurs paramètrables --

void Fenetre_Afficher_cadre_general(word Pos_X,word Pos_Y,word Largeur,word Hauteur,
                                    byte Couleur_HG,byte Couleur_BD,byte Couleur_S,byte Couleur_CHG,byte Couleur_CBD)
// Paramètres de couleurs:
// Couleur_HG =Bords Haut et Gauche
// Couleur_BD =Bords Bas et Droite
// Couleur_S  =Coins Haut-Droite et Bas-Gauche
// Couleur_CHG=Coin Haut-Gauche
// Couleur_CBD=Coin Bas-Droite
{
  // Bord haut (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
        (Largeur-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_HG);

  // Bord bas (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+Hauteur-1)*Menu_Facteur_Y),
        (Largeur-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_BD);

  // Bord gauche (sans les extrémités)
  Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(Hauteur-2)*Menu_Facteur_Y,Couleur_HG);

  // Bord droite (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+Largeur-1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(Hauteur-2)*Menu_Facteur_Y,Couleur_BD);

  // Coin haut gauche
  Pixel_dans_fenetre(Pos_X,Pos_Y,Couleur_CHG);
  // Coin haut droite
  Pixel_dans_fenetre(Pos_X+Largeur-1,Pos_Y,Couleur_S);
  // Coin bas droite
  Pixel_dans_fenetre(Pos_X+Largeur-1,Pos_Y+Hauteur-1,Couleur_CBD);
  // Coin bas gauche
  Pixel_dans_fenetre(Pos_X,Pos_Y+Hauteur-1,Couleur_S);
}

  // -- Cadre dont tout le contour est d'une seule couleur --

void Fenetre_Afficher_cadre_mono(word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte Couleur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,Couleur,Couleur,Couleur,Couleur,Couleur);
}

  // -- Cadre creux: foncé en haut-gauche et clair en bas-droite --

void Fenetre_Afficher_cadre_creux(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,CM_Fonce,CM_Blanc,CM_Clair,CM_Fonce,CM_Blanc);
}

  // -- Cadre bombé: clair en haut-gauche et foncé en bas-droite --

void Fenetre_Afficher_cadre_bombe(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,CM_Blanc,CM_Fonce,CM_Clair,CM_Blanc,CM_Fonce);
}

  // -- Cadre de séparation: un cadre bombé dans un cadre creux (3D!!!) --

void Fenetre_Afficher_cadre(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_creux(Pos_X,Pos_Y,Largeur,Hauteur);
  Fenetre_Afficher_cadre_bombe(Pos_X+1,Pos_Y+1,Largeur-2,Hauteur-2);
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

  if ((Fore_color>=Couleur_debut_palette) && (Fore_color<Couleur_debut_palette+64) && (Menu_visible))
  {
    if (Config.Couleurs_separees)
    {
      Debut_X=(LARGEUR_MENU+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee+((1+(((Fore_color-Couleur_debut_palette)&7)<<2))*Menu_Facteur_Y);

      Block(Debut_X,Debut_Y,(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);
      Block(Debut_X,Debut_Y+(Menu_Facteur_Y<<2),(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);

      Block(Debut_X,Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*3,Couleur);
      Block(Debut_X+(Menu_Taille_couleur*Menu_Facteur_X),Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*3,Couleur);
    }
    else
    {
      if (Couleur==CM_Noir)
      {
        Debut_X=(LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
        Debut_Y=Menu_Ordonnee+((2+(((Fore_color-Couleur_debut_palette)&7)<<2))*Menu_Facteur_Y);

        Block(Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y<<2,Fore_color);

        UpdateRect(Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,Menu_Facteur_Y*4); // TODO On met à jour toute la palette... peut mieux faire
      }
      else
      {
        Debut_X=LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur;
        Debut_Y=2+(((Fore_color-Couleur_debut_palette)&7)<<2);

        Fin_X=Debut_X+Menu_Taille_couleur-1;
        Fin_Y=Debut_Y+3;

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

        UpdateRect(Debut_X*Menu_Facteur_X,Debut_Y*Menu_Facteur_Y,Menu_Taille_couleur*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y*4);
      }
    }
  }
}

  // -- Afficher la palette dans le menu --

void Afficher_palette_du_menu(void)
{
  byte Couleur;

  if (Menu_visible)
  {
    Block(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y,CM_Noir);

    if (Config.Couleurs_separees)
      for (Couleur=0;Couleur<64;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur&7)<<2))*Menu_Facteur_Y),
              (Menu_Taille_couleur-1)*Menu_Facteur_X,
              Menu_Facteur_Y*3,
              Couleur_debut_palette+Couleur);
    else
      for (Couleur=0;Couleur<64;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur&7)<<2))*Menu_Facteur_Y),
              Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y<<2,
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
      Couleur_debut_palette-=8;
  }
  else
  {
    while (Fore_color>=Couleur_debut_palette+64)
      Couleur_debut_palette+=8;
  }
  if (Ancienne_couleur!=Couleur_debut_palette)
    Afficher_palette_du_menu();
}



  // -- Afficher tout le menu --

void Afficher_menu(void)
{
  word Pos_X;
  word Pos_Y;
  char Chaine[4];


  if (Menu_visible)
  {
    // Affichage du sprite du menu
    for (Pos_Y=0;Pos_Y<HAUTEUR_MENU;Pos_Y++)
      for (Pos_X=0;Pos_X<LARGEUR_MENU;Pos_X++)
        Pixel_dans_menu(Pos_X,Pos_Y,BLOCK_MENU[Pos_Y][Pos_X]);
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

void Print_general(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  word  Indice;
  int Pos_X;
  int Pos_Y;
  byte *Caractere;
  short Reel_X;
  short Reel_Y;
  byte Repeat_Menu_Facteur_X;
  byte Repeat_Menu_Facteur_Y;

  Reel_Y=Y;
  for (Pos_Y=0;Pos_Y<8<<3;Pos_Y+=1<<3)
  {
    Reel_X=0; // Position dans le buffer
    for (Indice=0;Chaine[Indice]!='\0';Indice++)
    {
      // Pointeur sur le premier pixel du caractère
      Caractere=Fonte+(((unsigned char)Chaine[Indice])<<6);
      for (Pos_X=0;Pos_X<8;Pos_X+=1)
        for (Repeat_Menu_Facteur_X=0;Repeat_Menu_Facteur_X<Menu_Facteur_X*Pixel_width;Repeat_Menu_Facteur_X++)
          Buffer_de_ligne_horizontale[Reel_X++]=*(Caractere+Pos_X+Pos_Y)?Couleur_texte:Couleur_fond;
    }
    for (Repeat_Menu_Facteur_Y=0;Repeat_Menu_Facteur_Y<Menu_Facteur_Y;Repeat_Menu_Facteur_Y++)
      Afficher_ligne_fast(X,Reel_Y++,Indice*Menu_Facteur_X*8,Buffer_de_ligne_horizontale);
  }
}

  // -- Afficher un caractère dans une fenêtre --

void Print_char_dans_fenetre(short Pos_X,short Pos_Y,unsigned char Caractere,byte Couleur_texte,byte Couleur_fond)
{
  short X,Y;
  byte *Carac;
  Pos_X=(Pos_X*Menu_Facteur_X)+Fenetre_Pos_X;
  Pos_Y=(Pos_Y*Menu_Facteur_Y)+Fenetre_Pos_Y;
  // Premier pixel du caractère
  Carac=Fonte + (Caractere<<6);
  
  for (Y=0;Y<8;Y++)
    for (X=0;X<8;X++)
      Block(Pos_X+(X*Menu_Facteur_X), Pos_Y+(Y*Menu_Facteur_Y),
            Menu_Facteur_X, Menu_Facteur_Y,
            (*(Carac++)?Couleur_texte:Couleur_fond));
}

  // -- Afficher un caractère sans fond dans une fenêtre --

void Print_char_transparent_dans_fenetre(short Pos_X,short Pos_Y,unsigned char Caractere,byte Couleur)
{
  short X,Y;
  byte *Carac;
  Pos_X=(Pos_X*Menu_Facteur_X)+Fenetre_Pos_X;
  Pos_Y=(Pos_Y*Menu_Facteur_Y)+Fenetre_Pos_Y;
  // Premier pixel du caractère
  Carac=Fonte + (Caractere<<6);
  
  for (Y=0;Y<8;Y++)
    for (X=0;X<8;X++)
    {
      if (*(Carac++))
        Block(Pos_X+(X*Menu_Facteur_X), Pos_Y+(Y*Menu_Facteur_Y),
              Menu_Facteur_X, Menu_Facteur_Y, Couleur);
    }
}

  // -- Afficher une chaîne dans une fenêtre, avec taille maxi --

void Print_dans_fenetre_limite(short X,short Y,char * Chaine,byte Taille,byte Couleur_texte,byte Couleur_fond)
{
  char Chaine_affichee[256];
  strncpy(Chaine_affichee, Chaine, Taille);
  Chaine_affichee[Taille]='\0';

  if (strlen(Chaine) > Taille)
  {
    Chaine_affichee[Taille-1]=CARACTERE_SUSPENSION;
  }
  Print_dans_fenetre(X, Y, Chaine_affichee, Couleur_texte, Couleur_fond);
}

  // -- Afficher une chaîne dans une fenêtre --

void Print_dans_fenetre(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  Print_general((X*Menu_Facteur_X)+Fenetre_Pos_X,
                (Y*Menu_Facteur_Y)+Fenetre_Pos_Y,
                Chaine,Couleur_texte,Couleur_fond);
  UpdateRect(X*Menu_Facteur_X+Fenetre_Pos_X,Y*Menu_Facteur_Y+Fenetre_Pos_Y,8*Menu_Facteur_X*strlen(Chaine),8*Menu_Facteur_Y);
}

  // -- Afficher une chaîne dans le menu --

void Print_dans_menu(char * Chaine, short Position)
{
  Print_general((18+(Position<<3))*Menu_Facteur_X,Menu_Ordonnee_Texte,Chaine,CM_Noir,CM_Clair);
  UpdateRect((18+(Position<<3))*Menu_Facteur_X,Menu_Ordonnee_Texte,strlen(Chaine)*8*Menu_Facteur_X,8*Menu_Facteur_Y);
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
  short Debut_X;
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

    Block((LARGEUR_MENU+2+((Menu_Taille_couleur-12)<<3))*Menu_Facteur_X,
          Menu_Ordonnee_Texte,Menu_Facteur_X*96,Menu_Facteur_Y<<3,CM_Clair);

    Debut_X=LARGEUR_MENU+2+((Menu_Taille_couleur-Taille_nom)<<3);

    Print_general(Debut_X*Menu_Facteur_X,Menu_Ordonnee_Texte,Nom_affiche,CM_Noir,CM_Clair);
  }
}

// Fonction d'affichage d'une chaine numérique avec une fonte très fine
// Spécialisée pour les compteurs RGB
void Print_compteur(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  // Macros pour écrire des litteraux binaires.
  // Ex: Ob(11110000) == 0xF0
  #define Ob(x)  ((unsigned)Ob_(0 ## x ## uL))
  #define Ob_(x) ((x & 1) | (x >> 2 & 2) | (x >> 4 & 4) | (x >> 6 & 8) |                \
          (x >> 8 & 16) | (x >> 10 & 32) | (x >> 12 & 64) | (x >> 14 & 128))

  byte Caractere[14][8] = {
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
  short Pos_X;
  short Pos_Y;
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
    for (Pos_Y=0;Pos_Y<8;Pos_Y++)
    {
      for (Pos_X=0;Pos_X<6;Pos_X++)
      {
        byte Couleur = (Caractere[Numero_car][Pos_Y] & (1 << (6-Pos_X))) ? Couleur_texte:Couleur_fond;
        Pixel_dans_fenetre(X+(Indice*6+Pos_X),Y+Pos_Y,Couleur);
      }
    }
  }
  UpdateRect(Fenetre_Pos_X+X*Menu_Facteur_X,Fenetre_Pos_Y+Y*Menu_Facteur_Y,strlen(Chaine)*Menu_Facteur_X*6,8*Menu_Facteur_Y);
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
    if (Touche==SDLK_ESCAPE) Bouton_clicke=2;
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
  while ((Bouton_clicke<=0) && (Touche!=SDLK_ESCAPE) && (Touche!=SDLK_o));
  Touche=0;

  Fermer_fenetre();
  Afficher_curseur();
}



  // -- Redessiner le sprite d'un bouton dans le menu --

void Afficher_sprite_dans_menu(int Numero_bouton,int Numero_sprite)
{
  word Pos_X;
  word Pos_Y;
  word Pos_menu_X;
  word Pos_menu_Y;
  byte Couleur;

  for (Pos_Y=0,Pos_menu_Y=Bouton[Numero_bouton].Decalage_Y+1;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_Y++,Pos_menu_Y++)
    for (Pos_X=0,Pos_menu_X=Bouton[Numero_bouton].Decalage_X+1;Pos_X<LARGEUR_SPRITE_MENU;Pos_X++,Pos_menu_X++)
    {
      Couleur=SPRITE_MENU[Numero_sprite][Pos_Y][Pos_X];
      Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
      BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
    }
  UpdateRect(Menu_Facteur_X*(Bouton[Numero_bouton].Decalage_X+1),
    (Bouton[Numero_bouton].Decalage_Y+1)*Menu_Facteur_Y+Menu_Ordonnee,
    LARGEUR_SPRITE_MENU*Menu_Facteur_X,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y);
}

  // -- Redessiner la forme du pinceau dans le menu --

void Afficher_pinceau_dans_menu(void)
{
  short Pos_X,Pos_Y;
  short Debut_X;
  short Pos_menu_X,Pos_menu_Y;
  short Debut_menu_X;
  byte Couleur;

  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_BROSSE_COULEUR    : // Brosse en couleur
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      for (Pos_menu_Y=2,Pos_Y=0;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=1,Pos_X=0;Pos_X<LARGEUR_SPRITE_MENU;Pos_menu_X++,Pos_X++)
        {
          Couleur=SPRITE_MENU[4][Pos_Y][Pos_X];
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
      break;
    default : // Pinceau
      // On efface le pinceau précédent
      for (Pos_menu_Y=2,Pos_Y=0;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=1,Pos_X=0;Pos_X<LARGEUR_SPRITE_MENU;Pos_menu_X++,Pos_X++)
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
        Pos_Y=Pinceau_Decalage_Y-7;
        Pos_menu_Y=2;
      }
      else
        Pos_Y=0;

      for (;((Pos_Y<Pinceau_Hauteur) && (Pos_menu_Y<16));Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=Debut_menu_X,Pos_X=Debut_X;((Pos_X<Pinceau_Largeur) && (Pos_menu_X<15));Pos_menu_X++,Pos_X++)
        {
          Couleur=(Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X])?CM_Noir:CM_Clair;
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
  }
  UpdateRect(0,Menu_Ordonnee,LARGEUR_SPRITE_MENU*Menu_Facteur_X+3,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y+3);
}

  // -- Dessiner un pinceau prédéfini dans la fenêtre --

void Afficher_pinceau_dans_fenetre(word X,word Y,int Numero)
  // Pinceau = 0..NB_SPRITES_PINCEAU-1 : Pinceau prédéfini
{
  word Pos_X;
  word Pos_Y;
  word Pos_fenetre_X;
  word Pos_fenetre_Y;
  int Taille_X;
  int Taille_Y;
  word Orig_X;
  word Orig_Y;

  Taille_X=Menu_Facteur_X/Pixel_height;
  Taille_Y=Menu_Facteur_Y/Pixel_width;

  Orig_X = (X + 8)*Menu_Facteur_X - (Pinceau_predefini_Decalage_X[Numero])*Taille_X+Fenetre_Pos_X;
  Orig_Y = (Y + 8)*Menu_Facteur_Y - (Pinceau_predefini_Decalage_Y[Numero])*Taille_Y+Fenetre_Pos_Y;

  for (Pos_fenetre_Y=0,Pos_Y=0; Pos_Y<Pinceau_predefini_Hauteur[Numero]; Pos_fenetre_Y++,Pos_Y++)
    for (Pos_fenetre_X=0,Pos_X=0; Pos_X<Pinceau_predefini_Largeur[Numero]; Pos_fenetre_X++,Pos_X++)
      Block(Orig_X+Pos_fenetre_X*Taille_X,Orig_Y+Pos_fenetre_Y*Taille_Y,Taille_X,Taille_Y,(SPRITE_PINCEAU[Numero][Pos_Y][Pos_X])?CM_Noir:CM_Clair);
  // On n'utilise pas Pixel_dans_fenetre() car on ne dessine pas
  // forcément avec la même taille de pixel.

  UpdateRect( ToWinX(Orig_X), ToWinY(Orig_Y),
        ToWinL(Pinceau_predefini_Largeur[Numero]),
        ToWinH(Pinceau_predefini_Hauteur[Numero])
  );
}

  // -- Dessiner des zigouigouis --

void Dessiner_zigouigoui(word X,word Y, byte Couleur, short Sens)
{
  word i;

  for (i=0; i<11; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens;
  for (i=1; i<10; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens+Sens;
  for (i=3; i<8; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens+Sens;
  Pixel_dans_fenetre(X,Y+5,Couleur);
}

  // -- Dessiner un bloc de couleurs dégradé verticalement

void Bloc_degrade_dans_fenetre(word Pos_X,word Pos_Y,word Debut_block,word Fin_block)
{
  word Total_lignes  =Menu_Facteur_Y<<6; // <=> à 64 lignes fct(Menu_Facteur)
  word Nb_couleurs   =(Debut_block<=Fin_block)?Fin_block-Debut_block+1:Debut_block-Fin_block+1;
  word Ligne_en_cours=(Debut_block<=Fin_block)?0:Total_lignes-1;

  word Debut_X       =Fenetre_Pos_X+(Menu_Facteur_X*Pos_X);
  word Largeur_ligne =Menu_Facteur_X<<4; // <=> à 16 pixels fct(Menu_Facteur)

  word Debut_Y       =Fenetre_Pos_Y+(Menu_Facteur_Y*Pos_Y);
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

  UpdateRect(ToWinX(Pos_X),ToWinY(Pos_Y),ToWinL(16),ToWinH(64));
}



  // -- Dessiner un petit sprite représentant le type d'un drive --

void Fenetre_Afficher_sprite_drive(word Pos_X,word Pos_Y,byte Type)
{
  word i,j;

  for (j=0; j<HAUTEUR_SPRITE_DRIVE; j++)
    for (i=0; i<LARGEUR_SPRITE_DRIVE; i++)
      Pixel_dans_fenetre(Pos_X+i,Pos_Y+j,SPRITE_DRIVE[Type][j][i]);
  UpdateRect(ToWinX(Pos_X),ToWinY(Pos_Y),ToWinL(LARGEUR_SPRITE_DRIVE),ToWinH(HAUTEUR_SPRITE_DRIVE));
}



void Afficher_palette_du_menu_en_evitant_la_fenetre(byte * Table)
{
  // On part du principe qu'il n'y a que le bas d'une fenêtre qui puisse
  // empiéter sur la palette... Et c'est déjà pas mal!
  byte Couleur,Vraie_couleur;
  word Debut_X,Debut_Y;
  word Fin_X,Fin_Y;
  word Largeur;
  word Hauteur;
  word Coin_X=Fenetre_Pos_X+Fenetre_Largeur*Menu_Facteur_X; // |_ Coin bas-droit
  word Coin_Y=Fenetre_Pos_Y+Fenetre_Hauteur*Menu_Facteur_Y; // |  de la fenêtre +1


  if (Config.Couleurs_separees)
  {
    Largeur=(Menu_Taille_couleur-1)*Menu_Facteur_X;
    Hauteur=Menu_Facteur_Y*3;
  }
  else
  {
    Largeur=Menu_Taille_couleur*Menu_Facteur_X;
    Hauteur=Menu_Facteur_Y<<2;
  }

  for (Couleur=0,Vraie_couleur=Couleur_debut_palette;Couleur<64;Couleur++,Vraie_couleur++)
  {
    if (Table[Vraie_couleur]!=Vraie_couleur)
    {
      Debut_X=(LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee_avant_fenetre+((2+((Couleur&7)<<2))*Menu_Facteur_Y);
      Fin_X=Debut_X+Largeur;
      Fin_Y=Debut_Y+Hauteur;

      //   On affiche le bloc en entier si on peut, sinon on le découpe autour
      // de la fenêtre.
      if ( (Debut_Y>=Coin_Y) || (Fin_X<=Fenetre_Pos_X) || (Debut_X>=Coin_X) )
        Block(Debut_X,Debut_Y,Largeur,Hauteur,Vraie_couleur);
      else
      {

        if (Debut_X>=Fenetre_Pos_X)
        {
          if ( (Fin_X>Coin_X) || (Fin_Y>Coin_Y) )
          {
            if ( (Fin_X>Coin_X) && (Fin_Y>Coin_Y) )
            {
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              if (Fin_Y>Coin_Y)
                Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
              else
                Block(Coin_X,Debut_Y,Fin_X-Coin_X,Hauteur,Vraie_couleur);
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
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Hauteur,Vraie_couleur);
          }
          else
          {
            if (Fin_Y>Coin_Y)
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Hauteur,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Hauteur,Vraie_couleur);
            }
          }
        }
      }
      {
        // Affichage du bloc directement dans le "buffer de fond" de la fenetre.
        // Cela permet au bloc de couleur d'apparaitre si on déplace la fenetre.
        short Pos_X;
        short Pos_Y;
        short Deb_X; // besoin d'une variable signée
        short Deb_Y; // besoin d'une variable signée
        // Attention aux unités
        Deb_X = ((short)Debut_X - (short)Fenetre_Pos_X);
        Deb_Y = ((short)Debut_Y - (short)Fenetre_Pos_Y);

        for (Pos_Y=Deb_Y;Pos_Y<(Deb_Y+Hauteur)&&Pos_Y<Fenetre_Hauteur*Menu_Facteur_Y;Pos_Y++)
          for (Pos_X=Deb_X;Pos_X<(Deb_X+Largeur)&&Pos_X<Fenetre_Largeur*Menu_Facteur_X;Pos_X++)
            if (Pos_X>=0&&Pos_Y>=0)
              Pixel_fond(Pos_X,Pos_Y,Vraie_couleur);
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
  short Debut;
  short Pos;
  short Fin;
  byte Droite_visible;
  byte Bas_visible;
  short Ancienne_Limite_Zoom;

  Droite_visible=Principal_Largeur_image<((Loupe_Mode)?Principal_Split:Largeur_ecran);
  Bas_visible   =Principal_Hauteur_image<Menu_Ordonnee;


  // On vérifie que la limite à droite est visible:
  if (Droite_visible)
  {
    Debut=Limite_Haut;
    Fin=(Limite_Bas<Principal_Hauteur_image)?
        Limite_Bas:Principal_Hauteur_image;

    if (Bas_visible)
      Fin++;

    // Juste le temps d'afficher les limites, on étend les limites de la loupe
    // aux limites visibles, car sinon Pixel_Preview ne voudra pas afficher.
    Ancienne_Limite_Zoom=Limite_Droite_Zoom;
    Limite_Droite_Zoom=Limite_visible_Droite_Zoom;

    for (Pos=Debut;Pos<=Fin;Pos++)
      Pixel_Preview(Principal_Largeur_image,Pos,((Pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    UpdateRect(Principal_Largeur_image,Debut,1,Fin-Debut + 1);
    // On restaure la bonne valeur des limites
    Limite_Droite_Zoom=Ancienne_Limite_Zoom;
  }

  // On vérifie que la limite en bas est visible:
  if (Bas_visible)
  {
    Debut=Limite_Gauche;
    Fin=(Limite_Droite<Principal_Largeur_image)?
        Limite_Droite:Principal_Largeur_image;

    // On étend également les limites en bas (comme pour la limite droit)
    Ancienne_Limite_Zoom=Limite_Bas_Zoom;
    Limite_Bas_Zoom=Limite_visible_Bas_Zoom;

    for (Pos=Debut;Pos<=Fin;Pos++)
      Pixel_Preview(Pos,Principal_Hauteur_image,((Pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    UpdateRect(Debut,Principal_Hauteur_image,Fin-Debut + 1,1);

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

void Ligne_horizontale_XOR_Zoom(short Pos_X, short Pos_Y, short Largeur)
{
  short Pos_X_reelle=Principal_X_Zoom+(Pos_X-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(Pos_Y-Loupe_Decalage_Y)*Loupe_Facteur;
  short Largeur_reelle=Largeur*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+Loupe_Facteur<Menu_Ordonnee)?Pos_Y_reelle+Loupe_Facteur:Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Largeur_reelle);

  UpdateRect(Pos_X_reelle,Pos_Y_reelle,Largeur_reelle,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher une barre verticale XOR zoomée

void Ligne_verticale_XOR_Zoom(short Pos_X, short Pos_Y, short Hauteur)
{
  short Pos_X_reelle=Principal_X_Zoom+(Pos_X-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(Pos_Y-Loupe_Decalage_Y)*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+(Hauteur*Loupe_Facteur<Menu_Ordonnee))?Pos_Y_reelle+(Hauteur*Loupe_Facteur):Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Loupe_Facteur);

  UpdateRect(Pos_X_reelle,Pos_Y_reelle,Loupe_Facteur,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher le curseur --

void Afficher_curseur(void)
{
  byte  Forme;
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Pos_X;
  short Pos_Y;
  short Compteur_X;
  short Compteur_Y;
  //short Fin_Compteur_X; // Position X ou s'arrête l'affichage de la brosse/pinceau
  //short Fin_Compteur_Y; // Position Y ou s'arrête l'affichage de la brosse/pinceau
  int   Temp;
  byte  Couleur;
  float cosA,sinA;
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;

  if ( ( (Mouse_Y<Menu_Ordonnee)
      && ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
    || (Fenetre) || (Forme_curseur==FORME_CURSEUR_SABLIER) )
    Forme=Forme_curseur;
  else
    Forme=FORME_CURSEUR_FLECHE;

  switch(Forme)
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

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15 && Pos_X < Largeur_ecran;Pos_X++,Compteur_X++)
                {
                  if( Pos_X < 0 ) continue;
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15 && Pos_Y < Hauteur_ecran;Pos_Y++,Compteur_Y++)
            {
                    if( Pos_Y < 0 || Pos_Y >= Hauteur_ecran) continue;
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
              if (Couleur!=CM_Trans)
                Pixel(Pos_X,Pos_Y,Couleur);
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

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
            {
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran)
                && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
              {
                FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
                if (Couleur!=CM_Trans)
                  Pixel(Pos_X,Pos_Y,Couleur);
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
      Debut_X=Mouse_X-Curseur_Decalage_X[Forme];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[Forme];
      for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
      {
        if(Pos_X<0) continue;
        if(Pos_X>=Largeur_ecran) break;
        for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
        {
          if(Pos_Y<0) continue;
          if(Pos_Y>=Hauteur_ecran) break;
          Couleur=SPRITE_CURSEUR[Forme][Compteur_Y][Compteur_X];
          // On sauvegarde dans FOND_CURSEUR pour restaurer plus tard
          FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
          if (Couleur!=CM_Trans)
              Pixel(Pos_X,Pos_Y,Couleur);
        }
      }
      UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Compteur_X,Compteur_Y);
      break;

    case FORME_CURSEUR_CIBLE_XOR :
      Pos_X=Pinceau_X-Principal_Decalage_X;
      Pos_Y=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // Largeur de la barre XOR
      if ((Pos_Y<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        UpdateRect(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((Pos_X<Compteur_X) && (Pinceau_X>=Limite_Gauche))
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
          Pos_X=Pinceau_X-Brosse_Centre_rotation_X;
          Pos_Y=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)Pos_X/sqrt((Pos_X*Pos_X)+(Pos_Y*Pos_Y));
          sinA=sin(acos(cosA));
          if (Pos_Y>0) sinA=-sinA;
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
  byte  Forme;
  int Debut_X; // int car sont parfois négatifs ! (quand on dessine sur un bord)
  int Debut_Y;
  short Fin_X;
  short Fin_Y;
  int Pos_X;
  int Pos_Y;
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
    Forme=Forme_curseur;
  else
    Forme=FORME_CURSEUR_FLECHE;

  switch(Forme)
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

          for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
          {
            if(Pos_Y>=Hauteur_ecran) break;
            for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            {
              if ( (Pos_X<0) || (Pos_Y < 0)) continue;
              else if (Pos_X>=Largeur_ecran) break;
              Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);
            }
          }

          UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Pos_X-Debut_X,Pos_Y-Debut_Y);
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

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran) && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
                Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);

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
      Debut_X=Mouse_X-Curseur_Decalage_X[Forme];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[Forme];

      for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
      {
        if(Pos_X<0) continue;
        if(Pos_X>=Largeur_ecran) break;
        for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
        {
            if(Pos_Y<0) continue;
            if(Pos_Y>=Hauteur_ecran) break;
            Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);
        }
      }
      UpdateRect(Max(Debut_X,0),Max(Debut_Y,0),Compteur_X,Compteur_Y);
      break;

    case FORME_CURSEUR_CIBLE_XOR :
      Pos_X=Pinceau_X-Principal_Decalage_X;
      Pos_Y=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // Largeur de la barre XOR
      if ((Pos_Y<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        UpdateRect(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((Pos_X<Compteur_X) && (Pinceau_X>=Limite_Gauche))
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
          Pos_X=Pinceau_X-Brosse_Centre_rotation_X;
          Pos_Y=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)Pos_X/sqrt((Pos_X*Pos_X)+(Pos_Y*Pos_Y));
          sinA=sin(acos(cosA));
          if (Pos_Y>0) sinA=-sinA;
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
  word Largeur;
  word Hauteur;

  // ---/\/\/\  Partie non zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    if (Principal_Largeur_image<Principal_Split)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Principal_Split;
  }
  else
  {
    if (Principal_Largeur_image<Largeur_ecran)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Largeur_ecran;
  }
  if (Principal_Hauteur_image<Menu_Ordonnee)
    Hauteur=Principal_Hauteur_image;
  else
    Hauteur=Menu_Ordonnee;
  Display_screen(Largeur,Hauteur,Principal_Largeur_image);

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
    Block(0,Principal_Hauteur_image,Largeur,(Menu_Ordonnee-Hauteur),0);

  // ---/\/\/\  Partie zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    // Affichage de la barre de split
    Afficher_barre_de_split();

    // Calcul de la largeur visible
    if (Principal_Largeur_image<Loupe_Largeur)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Loupe_Largeur;

    // Calcul du nombre de lignes visibles de l'image zoomée
    if (Principal_Hauteur_image<Loupe_Hauteur)
      Hauteur=Principal_Hauteur_image*Loupe_Facteur;
    else
      Hauteur=Menu_Ordonnee;

    Display_zoomed_screen(Largeur,Hauteur,Principal_Largeur_image,Buffer_de_ligne_horizontale);

    // Effacement de la partie non-image dans la partie zoomée:
    if (Principal_Largeur_image<Loupe_Largeur)
      Block(Principal_X_Zoom+(Principal_Largeur_image*Loupe_Facteur),0,
            (Loupe_Largeur-Principal_Largeur_image)*Loupe_Facteur,
            Menu_Ordonnee,0);
    if (Principal_Hauteur_image<Loupe_Hauteur)
      Block(Principal_X_Zoom,Hauteur,Largeur*Loupe_Facteur,(Menu_Ordonnee-Hauteur),0);
  }

  // ---/\/\/\ Affichage des limites /\/\/\---
  if (Config.Afficher_limites_image)
    Afficher_limites_de_l_image();
  UpdateRect(0,0,Largeur_ecran,Menu_Ordonnee); // TODO On peut faire plus fin, en évitant de mettre à jour la partie à droite du split quand on est en mode loupe. Mais c'est pas vraiment intéressant ?
}



byte Meilleure_couleur(byte R,byte V,byte B)
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
      Delta_R=(int)Principal_Palette[Coul].R-R;
      Delta_V=(int)Principal_Palette[Coul].V-V;
      Delta_B=(int)Principal_Palette[Coul].B-B;

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
    Delta_V=(int)Principal_Palette[Coul].V-Vert;
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
     (byte Rouge, byte Vert, byte Bleu, struct Composantes * Palette, byte * Table)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};


  for (Coul=0; Coul<256; Coul++)
  {
    Delta_R=(int)Palette[Coul].R-Rouge;
    Delta_V=(int)Palette[Coul].V-Vert;
    Delta_B=(int)Palette[Coul].B-Bleu;

    Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11);

    if (Dist<Best_dist[0])
    {
      Best_dist[3]=Best_dist[2];
      Best_dist[2]=Best_dist[1];
      Best_dist[1]=Best_dist[0];
      Best_dist[0]=Dist;
      Table[3]=Table[2];
      Table[2]=Table[1];
      Table[1]=Table[0];
      Table[0]=Coul;
    }
    else
    {
      if (Dist<Best_dist[1])
      {
        Best_dist[3]=Best_dist[2];
        Best_dist[2]=Best_dist[1];
        Best_dist[1]=Dist;
        Table[3]=Table[2];
        Table[2]=Table[1];
        Table[1]=Coul;
      }
      else
      {
        if (Dist<Best_dist[2])
        {
          Best_dist[3]=Best_dist[2];
          Best_dist[2]=Dist;
          Table[3]=Table[2];
          Table[2]=Coul;
        }
        else
        if (Dist<Best_dist[3])
        {
          Best_dist[3]=Dist;
          Table[3]=Coul;
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




void Calculer_couleurs_menu_optimales(struct Composantes * Palette)
{
  byte Table[4];
  short I,J,K;


  Old_Noir =CM_Noir;
  Old_Fonce=CM_Fonce;
  Old_Clair=CM_Clair;
  Old_Blanc=CM_Blanc;
  Old_Trans=CM_Trans;

  // Recherche du noir
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[0].R, Coul_menu_pref[0].V, Coul_menu_pref[0].B,Palette,Table);
  CM_Noir=Table[0];

  // Recherche du blanc
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[3].R, Coul_menu_pref[3].V, Coul_menu_pref[3].B,Palette,Table);
  if (CM_Noir!=Table[0])
    CM_Blanc=Table[0];
  else
    CM_Blanc=Table[1];

  // Recherche du gris clair
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[2].R, Coul_menu_pref[2].V, Coul_menu_pref[2].B,Palette,Table);
  if ( (CM_Noir!=Table[0]) && (CM_Blanc!=Table[0]) )
    CM_Clair=Table[0];
  else
  {
    if ( (CM_Noir!=Table[1]) && (CM_Blanc!=Table[1]) )
      CM_Clair=Table[1];
    else
      CM_Clair=Table[2];
  }

  // Recherche du gris foncé
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[1].R, Coul_menu_pref[1].V, Coul_menu_pref[1].B,Palette,Table);
  if ( (CM_Noir!=Table[0]) && (CM_Blanc!=Table[0]) && (CM_Clair!=Table[0]) )
    CM_Fonce=Table[0];
  else
  {
    if ( (CM_Noir!=Table[1]) && (CM_Blanc!=Table[1]) && (CM_Clair!=Table[1]) )
      CM_Fonce=Table[1];
    else
    {
      if ( (CM_Noir!=Table[2]) && (CM_Blanc!=Table[2]) && (CM_Clair!=Table[2]) )
        CM_Fonce=Table[2];
      else
        CM_Fonce=Table[3];
    }
  }

  // C'est peu probable mais il est possible que CM_Clair soit plus foncée que
  // CM_Fonce. Dans ce cas, on les inverse.
  if ( ((Palette[CM_Clair].R*30)+(Palette[CM_Clair].V*59)+(Palette[CM_Clair].B*11)) <
       ((Palette[CM_Fonce].R*30)+(Palette[CM_Fonce].V*59)+(Palette[CM_Fonce].B*11)) )
  {
    I=CM_Clair;
    CM_Clair=CM_Fonce;
    CM_Fonce=I;
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
    for (K=0; K<NB_SPRITES_CURSEUR; K++)
      for (J=0; J<HAUTEUR_SPRITE_CURSEUR; J++)
        for (I=0; I<LARGEUR_SPRITE_CURSEUR; I++)
          Remap_pixel(&SPRITE_CURSEUR[K][J][I]);
    // Le menu
    for (J=0; J<HAUTEUR_MENU; J++)
      for (I=0; I<LARGEUR_MENU; I++)
        Remap_pixel(&BLOCK_MENU[J][I]);
    // Sprites du menu
    for (K=0; K<NB_SPRITES_MENU; K++)
      for (J=0; J<HAUTEUR_SPRITE_MENU; J++)
        for (I=0; I<LARGEUR_SPRITE_MENU; I++)
          Remap_pixel(&SPRITE_MENU[K][J][I]);
    // Sprites d'effets
    for (K=0; K<NB_SPRITES_EFFETS; K++)
      for (J=0; J<HAUTEUR_SPRITE_MENU; J++)
        for (I=0; I<LARGEUR_SPRITE_MENU; I++)
          Remap_pixel(&SPRITE_EFFET[K][J][I]);
    // Fontes de l'aide
    for (K=0; K<256; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_norm[K][I][J]);
    for (K=0; K<256; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_bold[K][I][J]);
    for (K=0; K<64; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_t1[K][I][J]);
    for (K=0; K<64; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_t2[K][I][J]);
    for (K=0; K<64; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_t3[K][I][J]);
    for (K=0; K<64; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help_t4[K][I][J]);
        
    // Sprites de lecteurs (drives)
    for (K=0; K<NB_SPRITES_DRIVES; K++)
      for (J=0; J<HAUTEUR_SPRITE_DRIVE; J++)
        for (I=0; I<LARGEUR_SPRITE_DRIVE; I++)
          Remap_pixel(&SPRITE_DRIVE[K][J][I]);
  }
}
