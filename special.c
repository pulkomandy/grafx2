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
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "moteur.h"
#include "windows.h"



//---------------------- Modifier le pinceau spécial -------------------------

void Modifier_pinceau(int Largeur, int Hauteur)
{
  int Pos_X,Pos_Y;
  int X,Y;
  float Rayon2;

  if (Largeur<1) Largeur=1;
  if (Hauteur<1) Hauteur=1;
  if (Largeur>TAILLE_MAXI_PINCEAU) Largeur=TAILLE_MAXI_PINCEAU;
  if (Hauteur>TAILLE_MAXI_PINCEAU) Hauteur=TAILLE_MAXI_PINCEAU;
  Pinceau_Largeur=Largeur;
  Pinceau_Hauteur=Hauteur;
  Pinceau_Decalage_X=Pinceau_Largeur>>1;
  Pinceau_Decalage_Y=Pinceau_Hauteur>>1;
  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_ROND :
      Rayon2=Pinceau_Decalage_X+0.414213562; // [0.410..0.415[
      Rayon2*=Rayon2;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
        {
          X=Pos_X-Pinceau_Decalage_X;
          Y=Pos_Y-Pinceau_Decalage_Y;
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=( ((X*X)+(Y*Y)) < Rayon2 );
        }
      break;
    case FORME_PINCEAU_CARRE :
      for (Pos_X=0,Pos_Y=0; Pos_X<Pinceau_Hauteur; Pos_X++,Pos_Y+=TAILLE_MAXI_PINCEAU)
        memset(Pinceau_Sprite+Pos_Y,1,Pinceau_Largeur);
      break;
    case FORME_PINCEAU_ROND_TRAME :
      Rayon2=Pinceau_Decalage_X+0.414213562; // [0.410..0.415[
      Rayon2*=Rayon2;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
        {
          X=Pos_X-Pinceau_Decalage_X;
          Y=Pos_Y-Pinceau_Decalage_Y;
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=( (!((Pos_X+Pos_Y)&1)) && (((X*X)+(Y*Y)) < Rayon2));
        }
      break;
    case FORME_PINCEAU_CARRE_TRAME:
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=!((Pos_X+Pos_Y)&1);
      break;
    case FORME_PINCEAU_PLUS:
      X=Pinceau_Largeur>>1;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=((Pos_X==X) || (Pos_Y==X));
      break;
    case FORME_PINCEAU_SLASH:
      X=Pinceau_Largeur>>1;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=(Pos_X==(Pinceau_Largeur-(Pos_Y+1)));
      break;
    case FORME_PINCEAU_ANTISLASH:
      X=Pinceau_Largeur>>1;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=(Pos_X==Pos_Y);
      break;
    case FORME_PINCEAU_BARRE_HORIZONTALE:
      memset(Pinceau_Sprite,1,Pinceau_Largeur);
      break;
    case FORME_PINCEAU_BARRE_VERTICALE:
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)]=1;
      break;
    case FORME_PINCEAU_X:
      X=Pinceau_Largeur>>1;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=( (Pos_X==Pos_Y) || (Pos_X==(Pinceau_Hauteur-(Pos_Y+1))) );
      break;
    case FORME_PINCEAU_LOSANGE:
      X=Pinceau_Largeur>>1;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
        {
          if (Pos_X<=X)
            Y=X-Pos_X;
          else
            Y=Pos_X-X;
          if (Pos_Y<=X)
            Y+=X-Pos_Y;
          else
            Y+=Pos_Y-X;
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=(Y<=X);
        }
      break;
    case FORME_PINCEAU_ALEATOIRE:
      Rayon2=Pinceau_Decalage_X+0.414213562; // [0.410..0.415[
      Rayon2*=Rayon2;
      for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
        for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
        {
          X=Pos_X-Pinceau_Decalage_X;
          Y=Pos_Y-Pinceau_Decalage_Y;
          Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=( (((X*X)+(Y*Y)) < Rayon2) && (!(rand()&7)) );
        }
  }
}

void Retrecir_pinceau(void)
{
  if ( (Pinceau_Forme<FORME_PINCEAU_DIVERS)
    && ( (Pinceau_Largeur>1)
      || (Pinceau_Hauteur>1) ) )
  {
    Effacer_curseur();
    switch (Pinceau_Forme)
    {
      case FORME_PINCEAU_ROND:
      case FORME_PINCEAU_ROND_TRAME:
      case FORME_PINCEAU_X:
      case FORME_PINCEAU_PLUS:
      case FORME_PINCEAU_LOSANGE:
      case FORME_PINCEAU_ALEATOIRE:
        if (Pinceau_Largeur&1)
          Modifier_pinceau(Pinceau_Largeur-2,Pinceau_Hauteur-2);
        else
          Modifier_pinceau(Pinceau_Largeur-1,Pinceau_Hauteur-1);
        break;
      case FORME_PINCEAU_CARRE:
      case FORME_PINCEAU_SLASH:
      case FORME_PINCEAU_ANTISLASH:
      case FORME_PINCEAU_CARRE_TRAME:
        Modifier_pinceau(Pinceau_Largeur-1,Pinceau_Hauteur-1);
        break;
      case FORME_PINCEAU_BARRE_HORIZONTALE:
        Modifier_pinceau(Pinceau_Largeur-1,1);
        break;
      case FORME_PINCEAU_BARRE_VERTICALE:
        Modifier_pinceau(1,Pinceau_Hauteur-1);
    }
    Afficher_pinceau_dans_menu();
    Afficher_curseur();
  }
}

void Grossir_pinceau(void)
{
  if ( (Pinceau_Forme<FORME_PINCEAU_DIVERS)
    && ( (Pinceau_Largeur<TAILLE_MAXI_PINCEAU)
      || (Pinceau_Hauteur<TAILLE_MAXI_PINCEAU) ) )
  {
    Effacer_curseur();
    switch (Pinceau_Forme)
    {
      case FORME_PINCEAU_ROND:
      case FORME_PINCEAU_ROND_TRAME:
      case FORME_PINCEAU_ALEATOIRE:
      case FORME_PINCEAU_X:
      case FORME_PINCEAU_PLUS:
      case FORME_PINCEAU_LOSANGE:
        if (Pinceau_Largeur&1)
          Modifier_pinceau(Pinceau_Largeur+2,Pinceau_Hauteur+2);
        else
          Modifier_pinceau(Pinceau_Largeur+1,Pinceau_Hauteur+1);
        break;
      case FORME_PINCEAU_CARRE:
      case FORME_PINCEAU_SLASH:
      case FORME_PINCEAU_ANTISLASH:
      case FORME_PINCEAU_CARRE_TRAME:
        Modifier_pinceau(Pinceau_Largeur+1,Pinceau_Hauteur+1);
        break;
      case FORME_PINCEAU_BARRE_HORIZONTALE:
        Modifier_pinceau(Pinceau_Largeur+1,1);
        break;
      case FORME_PINCEAU_BARRE_VERTICALE:
        Modifier_pinceau(1,Pinceau_Hauteur+1);
    }
    Afficher_pinceau_dans_menu();
    Afficher_curseur();
  }
}


//--------------------- Passer à la ForeColor suivante -----------------------
void Special_Next_forecolor(void)
{
  Effacer_curseur();
  Encadrer_couleur_menu(CM_Noir);

  Fore_color++;

  Recadrer_palette();
  Afficher_foreback();

  Encadrer_couleur_menu(CM_Blanc);
  Afficher_curseur();
}

//-------------------- Passer à la ForeColor précédente ----------------------
void Special_Previous_forecolor(void)
{
  Effacer_curseur();
  Encadrer_couleur_menu(CM_Noir);

  Fore_color--;

  Recadrer_palette();
  Afficher_foreback();

  Encadrer_couleur_menu(CM_Blanc);
  Afficher_curseur();
}

//--------------------- Passer à la BackColor suivante -----------------------
void Special_Next_backcolor(void)
{
  Effacer_curseur();
  Back_color++;
  Afficher_foreback();
  Afficher_curseur();
}

//-------------------- Passer à la BackColor précédente ----------------------
void Special_Previous_backcolor(void)
{
  Effacer_curseur();
  Back_color--;
  Afficher_foreback();
  Afficher_curseur();
}


// ------------------- Scroller l'écran (pas en mode loupe) ------------------
void Scroller_ecran(short Decalage_en_X,short Decalage_en_Y)
{
  short Decalage_temporaire_X;
  short Decalage_temporaire_Y;

  Decalage_temporaire_X=Principal_Decalage_X+Decalage_en_X;
  Decalage_temporaire_Y=Principal_Decalage_Y+Decalage_en_Y;

  if (Decalage_temporaire_X+Largeur_ecran>Principal_Largeur_image)
    Decalage_temporaire_X=Principal_Largeur_image-Largeur_ecran;
  if (Decalage_temporaire_Y+Menu_Ordonnee>Principal_Hauteur_image)
    Decalage_temporaire_Y=Principal_Hauteur_image-Menu_Ordonnee;
  if (Decalage_temporaire_X<0)
    Decalage_temporaire_X=0;
  if (Decalage_temporaire_Y<0)
    Decalage_temporaire_Y=0;

  if ( (Principal_Decalage_X!=Decalage_temporaire_X) ||
       (Principal_Decalage_Y!=Decalage_temporaire_Y) )
  {
    Effacer_curseur();
    Principal_Decalage_X=Decalage_temporaire_X;
    Principal_Decalage_Y=Decalage_temporaire_Y;

    Calculer_limites();
    Calculer_coordonnees_pinceau();

    Afficher_ecran();  // <=> Display_screen + Afficher_limites_de_l_image
    Afficher_curseur();
  }
}


// ---------------------- Scroller la fenêtre de la loupe --------------------
void Scroller_loupe(short Decalage_en_X,short Decalage_en_Y)
{
  short Decalage_temporaire_X;
  short Decalage_temporaire_Y;

  Decalage_temporaire_X=Loupe_Decalage_X+Decalage_en_X;
  Decalage_temporaire_Y=Loupe_Decalage_Y+Decalage_en_Y;

  if (Decalage_temporaire_X+Loupe_Largeur>Principal_Largeur_image)
    Decalage_temporaire_X=Principal_Largeur_image-Loupe_Largeur;
  if (Decalage_temporaire_Y+Loupe_Hauteur>Principal_Hauteur_image)
    Decalage_temporaire_Y=Principal_Hauteur_image-Loupe_Hauteur;
  if (Decalage_temporaire_X<0)
    Decalage_temporaire_X=0;
  if (Decalage_temporaire_Y<0)
    Decalage_temporaire_Y=0;

  if ( (Loupe_Decalage_X!=Decalage_temporaire_X) ||
       (Loupe_Decalage_Y!=Decalage_temporaire_Y) )
  {
    Effacer_curseur();
    Loupe_Decalage_X=Decalage_temporaire_X;
    Loupe_Decalage_Y=Decalage_temporaire_Y;

    Recadrer_ecran_par_rapport_au_zoom();

    Calculer_limites();
    Calculer_coordonnees_pinceau();

    Afficher_ecran();
    Afficher_curseur();
  }
}


// -------------- Changer le Zoom (grâce aux touches [+] et [-]) -------------
void Zoom(short Sens)
{
  short Indice;
  for (Indice=0; FACTEUR_ZOOM[Indice]!=Loupe_Facteur; Indice++);
  Indice+=Sens;

  if ( (Indice>=0) && (Indice<NB_FACTEURS_DE_ZOOM) )
  {
    Effacer_curseur();
    Changer_facteur_loupe(Indice);
    if (Loupe_Mode)
      Afficher_ecran();
    Afficher_curseur();
  }
}
