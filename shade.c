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
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "graph.h"
#include "moteur.h"
#include "divers.h"
#include "readline.h"
#include "aide.h"
#include "sdlscreen.h"

void Bouton_Shade_Mode(void)
{
  if (Shade_Mode)
    Fonction_effet=Aucun_effet;
  else
  {
    Fonction_effet=Effet_Shade;
    Quick_shade_Mode=0;
    Colorize_Mode=0;
    Smooth_Mode=0;
    Tiling_Mode=0;
    Smear_Mode=0;
  }
  Shade_Mode=!Shade_Mode;
}


void Bouton_Quick_shade_Mode(void)
{
  if (Quick_shade_Mode)
    Fonction_effet=Aucun_effet;
  else
  {
    Fonction_effet=Effet_Quick_shade;
    Shade_Mode=0;
    Colorize_Mode=0;
    Smooth_Mode=0;
    Tiling_Mode=0;
    Smear_Mode=0;
  }
  Quick_shade_Mode=!Quick_shade_Mode;
}


void Shade_Blocs_degrades(void)
{
  word  Curseur=0;
  word  Nb_shades=0;
  short Shade_traite,Ancien_shade_traite;
  word  Taille_shade=0;
  word  Debut_shade=0;
  short Pos_X,Pos_Y;
  short Taille_X,Taille_Y;
  short Debut_X,Debut_Y,Fin_X,Fin_Y;

  // On commence par compter le nombre de shades
  while (Curseur<512)
  {
    while ((Curseur<512) && (Shade_Liste[Shade_Actuel].Liste[Curseur]&0xFF00))
      Curseur++;

    if (Curseur<512)
    {
      Nb_shades++;
      while ( (Curseur<512)
         && (!(Shade_Liste[Shade_Actuel].Liste[Curseur]&0xFF00)) )
        Curseur++;
    }
  }

  // Maintenant qu'on sait combien il y en a, on les affiche:
  if (Nb_shades)
  {
    Taille_X=Menu_Facteur_X<<6;
    Taille_Y=Menu_Facteur_Y*48;
    Debut_X=Fenetre_Pos_X+(Menu_Facteur_X*224);
    Debut_Y=Fenetre_Pos_Y+(Menu_Facteur_Y*35);
    Fin_X=Debut_X+Taille_X;
    Fin_Y=Debut_Y+Taille_Y;

    Curseur=0;
    Ancien_shade_traite=-1;

    for (Pos_Y=Debut_Y;Pos_Y<Fin_Y;Pos_Y++)
    {
      // On regarde quel shade on va afficher en preview
      Shade_traite=((Pos_Y-Debut_Y)*Nb_shades)/Taille_Y;
      // Si ce n'est pas le shade précédemment traité on calcule ses infos
      if (Shade_traite>Ancien_shade_traite)
      {
        // On commence par sauter tous les vides jusqu'au prochain shade
        while ((Curseur<512) && (Shade_Liste[Shade_Actuel].Liste[Curseur]&0xFF00))
          Curseur++;
        Debut_shade=Curseur;
        // puis regarde sa taille
        while ((Curseur<512) && (!(Shade_Liste[Shade_Actuel].Liste[Curseur]&0xFF00)))
          Curseur++;
        Taille_shade=Curseur-Debut_shade;
        Ancien_shade_traite=Shade_traite;
      }

      for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
      {
        Pixel(Pos_X,Pos_Y,Shade_Liste[Shade_Actuel].Liste
              [(((Pos_X-Debut_X)*Taille_shade)/Taille_X)+Debut_shade]);
      }
    }
  }
  else
  {
    Fenetre_Afficher_cadre_bombe(224,35,64,48);
    Block(Fenetre_Pos_X+(Menu_Facteur_X*225),Fenetre_Pos_Y+(Menu_Facteur_Y*36),
          Menu_Facteur_X*62,Menu_Facteur_Y*46,CM_Clair);
  }
  UpdateRect(Fenetre_Pos_X+(Menu_Facteur_X*224),Fenetre_Pos_Y+(Menu_Facteur_Y*35),Menu_Facteur_X*64,Menu_Facteur_Y*48);
}


void Tagger_shades(word Select_Debut,word Select_Fin)
{
  word Ligne, Colonne;
  word Position;
  word Pos_X, Pos_Y;


  if (Select_Fin<Select_Debut)
  {
    Position    =Select_Fin;
    Select_Fin  =Select_Debut;
    Select_Debut=Position;
  }

  for (Ligne=0; Ligne<8; Ligne++)
    for (Colonne=0; Colonne<64; Colonne++)
    {
      Position=(Ligne<<6)+Colonne;
      Pos_X=Fenetre_Pos_X+(Menu_Facteur_X*((Colonne<<2)+8));
      Pos_Y=Fenetre_Pos_Y+(Menu_Facteur_Y*((Ligne*7)+131));

      // On regarde si la case est "disablée"
      if (Shade_Liste[Shade_Actuel].Liste[Position]&0x8000)
      {
        if ((Position>=Select_Debut) && (Position<=Select_Fin))
        {
          Block(Pos_X,Pos_Y,Menu_Facteur_X<<2,Menu_Facteur_Y,CM_Blanc);
          Block(Pos_X,Pos_Y+Menu_Facteur_Y,Menu_Facteur_X<<2,Menu_Facteur_Y,CM_Noir);
        }
        else
          Block(Pos_X,Pos_Y,Menu_Facteur_X<<2,Menu_Facteur_Y<<1,CM_Blanc);
      }
      else // "enablée"
      {
        if ((Position>=Select_Debut) && (Position<=Select_Fin))
          Block(Pos_X,Pos_Y,Menu_Facteur_X<<2,Menu_Facteur_Y<<1,CM_Noir);
        else
          Block(Pos_X,Pos_Y,Menu_Facteur_X<<2,Menu_Facteur_Y<<1,CM_Clair);
      }
    }
    UpdateRect(Fenetre_Pos_X+8*Menu_Facteur_X,Fenetre_Pos_Y+131*Menu_Facteur_Y,Menu_Facteur_X*64<<2,Menu_Facteur_Y*8<<3);
}


void Afficher_couleur_case_selectionnee(word Select_Debut,word Select_Fin)
{
  char Chaine[4];

  if ((Select_Debut!=Select_Fin)
   || (Shade_Liste[Shade_Actuel].Liste[Select_Debut]&0x0100))
    strcpy(Chaine,"   ");
  else
    Num2str(Shade_Liste[Shade_Actuel].Liste[Select_Debut]&0xFF,Chaine,3);

  Print_dans_fenetre(213,115,Chaine,CM_Noir,CM_Clair);
}


void Afficher_couleur_selectionnee(word Select_Debut,word Select_Fin)
{
  char Chaine[4];

  if (Select_Debut!=Select_Fin)
    strcpy(Chaine,"   ");
  else
    Num2str(Select_Debut,Chaine,3);

  Print_dans_fenetre(213,106,Chaine,CM_Noir,CM_Clair);
}


void Afficher_mode_du_shade(short X,short Y,byte Mode)
{
  char Chaine[7];

  switch (Mode)
  {
    case MODE_SHADE_NORMAL :
      strcpy(Chaine,"Normal");
      break;
    case MODE_SHADE_BOUCLE :
      strcpy(Chaine," Loop ");
      break;
    default : // MODE_SHADE_NOSAT
      strcpy(Chaine,"No sat");
  }
  Print_dans_fenetre(X,Y,Chaine,CM_Noir,CM_Clair);
}


void Afficher_tout_le_shade(word Select_Debut1,word Select_Fin1,
                            word Select_Debut2,word Select_Fin2)
{
  word Ligne, Colonne;
  word Position;

  for (Ligne=0; Ligne<8; Ligne++)
    for (Colonne=0; Colonne<64; Colonne++)
    {
      Position=(Ligne<<6)+Colonne;
      // On regarde si c'est une couleur ou un bloc vide
      if (Shade_Liste[Shade_Actuel].Liste[Position]&0x0100) // Vide
      {
        Fenetre_Afficher_cadre_bombe((Colonne<<2)+8,(Ligne*7)+127,4,4);
        Block(Fenetre_Pos_X+(Menu_Facteur_X*((Colonne<<2)+9)),
              Fenetre_Pos_Y+(Menu_Facteur_Y*((Ligne*7)+128)),
              Menu_Facteur_X<<1,Menu_Facteur_Y<<1,CM_Clair);
      }
      else // Couleur
        Block(Fenetre_Pos_X+(Menu_Facteur_X*((Colonne<<2)+8)),
              Fenetre_Pos_Y+(Menu_Facteur_Y*((Ligne*7)+127)),
              Menu_Facteur_X<<2,Menu_Facteur_Y<<2,
              Shade_Liste[Shade_Actuel].Liste[Position]&0xFF);
    }
  UpdateRect(Fenetre_Pos_X+7*Menu_Facteur_X,Fenetre_Pos_Y+126*Menu_Facteur_Y,Menu_Facteur_X*((64<<2)+2),Menu_Facteur_Y*((8<<2)+2));
  Tagger_shades(Select_Debut2,Select_Fin2);
  Shade_Blocs_degrades();
  Afficher_couleur_case_selectionnee(Select_Debut2,Select_Fin2);
  Afficher_couleur_selectionnee(Select_Debut1,Select_Fin1);
  Afficher_mode_du_shade(250,110,Shade_Liste[Shade_Actuel].Mode);
}


void Supprimer_shade(word Select_Debut,word Select_Fin)
{
  word Temp;

  if (Select_Fin<Select_Debut)
  {
    Temp        =Select_Fin;
    Select_Fin  =Select_Debut;
    Select_Debut=Temp;
  }

  for (Select_Fin++;Select_Fin<512;Select_Debut++,Select_Fin++)
    Shade_Liste[Shade_Actuel].Liste[Select_Debut]=Shade_Liste[Shade_Actuel].Liste[Select_Fin];

  for (;Select_Debut<512;Select_Debut++)
    Shade_Liste[Shade_Actuel].Liste[Select_Debut]=0x0100;
}


void Inserer_shade(byte Premiere_couleur, byte Derniere_couleur, word Select_Debut)
{
  word Curseur,Limite;
  word Temp;

  if (Derniere_couleur<Premiere_couleur)
  {
    Temp            =Derniere_couleur;
    Derniere_couleur=Premiere_couleur;
    Premiere_couleur=Temp;
  }

  // Avant d'insérer quoi que ce soit, on efface les éventuelles couleurs que
  // l'on va réinsérer:
  Limite=512-Select_Debut;
  for (Curseur=0; Curseur<512; Curseur++)
  {
    if (!(Shade_Liste[Shade_Actuel].Liste[Curseur]&0x0100))
      for (Temp=Premiere_couleur; Temp<=Derniere_couleur; Temp++)
        if ( (Temp-Premiere_couleur<Limite)
          && ((Shade_Liste[Shade_Actuel].Liste[Curseur]&0xFF)==Temp) )
          Shade_Liste[Shade_Actuel].Liste[Curseur]=(Shade_Liste[Shade_Actuel].Liste[Curseur]&0x8000)|0x0100;
  }
  // Voilà... Maintenant on peut y aller peinard.

  Temp=1+Derniere_couleur-Premiere_couleur;
  Limite=Select_Debut+Temp;
  if (Limite>=512)
    Temp=512-Select_Debut;

  for (Curseur=511;Curseur>=Limite;Curseur--)
    Shade_Liste[Shade_Actuel].Liste[Curseur]=Shade_Liste[Shade_Actuel].Liste[Curseur-Temp];

  for (Curseur=Select_Debut+Temp;Select_Debut<Curseur;Select_Debut++,Premiere_couleur++)
    Shade_Liste[Shade_Actuel].Liste[Select_Debut]=Premiere_couleur;
}


void Inserer_case_vide_dans_shade(word Position)
{
  word Curseur;

  if (Position>=512) return;

  for (Curseur=511;Curseur>Position;Curseur--)
    Shade_Liste[Shade_Actuel].Liste[Curseur]=Shade_Liste[Shade_Actuel].Liste[Curseur-1];

  Shade_Liste[Shade_Actuel].Liste[Position]=0x0100;
}


short Attendre_click_dans_shade()
{
  short Case_choisie=-1;
  byte  Ancien_Cacher_curseur;


  Effacer_curseur();
  Ancien_Cacher_curseur=Cacher_curseur;
  Cacher_curseur=0;
  Forme_curseur=FORME_CURSEUR_CIBLE;
  Afficher_curseur();

  while (Case_choisie<0)
  {
    Get_input();

    if ( (Mouse_K==A_GAUCHE)
      && ( ( (Fenetre_click_dans_zone(8,127,263,179)) && (((((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-127)%7)<4) )
           || ( (Mouse_X<Fenetre_Pos_X) || (Mouse_Y<Fenetre_Pos_Y)
             || (Mouse_X>=Fenetre_Pos_X+(Fenetre_Largeur*Menu_Facteur_X))
             || (Mouse_Y>=Fenetre_Pos_Y+(Fenetre_Hauteur*Menu_Facteur_Y)) ) )
       )
      Case_choisie=(((((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-127)/7)<<6)+
                    ((((Mouse_X-Fenetre_Pos_X)/Menu_Facteur_X)-8 )>>2);

    if ((Mouse_K==A_DROITE) || (Touche==SDLK_ESCAPE))
      Case_choisie=512; // valeur indiquant que l'on n'a rien choisi
  }

  Attendre_fin_de_click();
  Effacer_curseur();
  Forme_curseur=FORME_CURSEUR_FLECHE;
  Cacher_curseur=Ancien_Cacher_curseur;
  Afficher_curseur();
  return Case_choisie;
}


void Swap_shade(short Debut_Bloc_1,short Debut_Bloc_2,short Taille_du_bloc)
{
  short  Pos_1;
  short  Pos_2;
  short  Fin_1;
  short  Fin_2;
  word   Temp;
  word * Shade_temporaire;

  // On fait une copie de la liste
  Shade_temporaire=(word *)malloc(512*sizeof(word));
  memcpy(Shade_temporaire,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));

  // On calcul les dernières couleurs de chaque bloc.
  Fin_1=Debut_Bloc_1+Taille_du_bloc-1;
  Fin_2=Debut_Bloc_2+Taille_du_bloc-1;

  if ((Debut_Bloc_2>=Debut_Bloc_1) && (Debut_Bloc_2<=Fin_1))
  {
    // Le bloc destination commence dans le bloc source.
    for (Pos_1=Debut_Bloc_1,Pos_2=Fin_1+1;Pos_1<=Fin_2;Pos_1++)
    {
      // Il faut transformer la case Pos_1 en Pos_2:
      Shade_Liste[Shade_Actuel].Liste[Pos_1]=Shade_temporaire[Pos_2];
      // On gère la mise à jour de Pos_2
      if (Pos_2==Fin_2)
        Pos_2=Debut_Bloc_1;
      else
        Pos_2++;
    }
  }
  else
  if ((Debut_Bloc_2<Debut_Bloc_1) && (Fin_2>=Debut_Bloc_1))
  {
    // Le bloc destination déborde dans le bloc source.
    for (Pos_1=Debut_Bloc_2,Pos_2=Debut_Bloc_1;Pos_1<=Fin_1;Pos_1++)
    {
      // Il faut transformer la couleur Pos_1 en Pos_2:
      Shade_Liste[Shade_Actuel].Liste[Pos_1]=Shade_temporaire[Pos_2];
      // On gère la mise à jour de Pos_2
      if (Pos_2==Fin_1)
        Pos_2=Debut_Bloc_2;
      else
        Pos_2++;
    }
  }
  else
  {
    // Le bloc source et le bloc destination sont distincts.
    for (Pos_1=Debut_Bloc_1,Pos_2=Debut_Bloc_2;Pos_1<=Fin_1;Pos_1++,Pos_2++)
    {
      // On échange les cases
      Temp                                  =Shade_Liste[Shade_Actuel].Liste[Pos_1];
      Shade_Liste[Shade_Actuel].Liste[Pos_1]=Shade_Liste[Shade_Actuel].Liste[Pos_2];
      Shade_Liste[Shade_Actuel].Liste[Pos_2]=Temp;
    }
  }

  free(Shade_temporaire);
}


int Menu_Shade(void)
{
  short Bouton_clicke;  // Numéro du bouton sur lequel l'utilisateur a clické
  char Chaine[4];        // Chaine d'affichage du n° de shade actif et du Pas
  word Ancien_Mouse_X,Ancien_Mouse_X2; // Mémo. de l'ancienne pos. du curseur
  word Ancien_Mouse_Y,Ancien_Mouse_Y2;
  byte Ancien_Mouse_K,Ancien_Mouse_K2;
  byte Couleur_temporaire; // Variables de gestion des clicks dans la palette
  byte Premiere_couleur=0;
  byte Derniere_couleur=0;
  word Select_Debut=0;
  word Select_Fin=0;
  struct Fenetre_Bouton_special * Bouton_saisie;
  short Temp,Temp2;
  word Case_temporaire;
  word * Buffer;       // Buffer du Copy/Paste
  word * Buffer_Undo;  // Buffer du Undo
  word * Pointeur_temp;
  byte Couleur;
  byte Click;


  Buffer       =(word *)malloc(512*sizeof(word));
  Buffer_Undo  =(word *)malloc(512*sizeof(word));
  Pointeur_temp=(word *)malloc(512*sizeof(word));

  // Ouverture de la fenêtre du menu
  Ouvrir_fenetre(310,190,"Shade");

  // Déclaration & tracé du bouton de palette
  Fenetre_Definir_bouton_palette(5,16);                             // 1

  // Déclaration & tracé du scroller de sélection du n° de dégradé
  Fenetre_Definir_bouton_scroller(192,17,84,8,1,Shade_Actuel);      // 2

  // Déclaration & tracé de la zone de définition des dégradés
  Fenetre_Definir_bouton_special(8,127,256,53);                     // 3

  // Déclaration & tracé des boutons de sortie
  Fenetre_Definir_bouton_normal(207,17,51,14,"Cancel",0,1,SDLK_ESCAPE);  // 4
  Fenetre_Definir_bouton_normal(261,17,43,14,"OK"    ,0,1,SDLK_RETURN);  // 5

  // Déclaration & tracé des boutons de copie de shade
  Fenetre_Definir_bouton_normal(206,87,27,14,"Cpy"   ,1,1,SDLK_c);  // 6
  Fenetre_Definir_bouton_normal(234,87,43,14,"Paste" ,1,1,SDLK_p);  // 7

  // On tagge le bloc
  Tagger_intervalle_palette(0,0);

  // Tracé d'un cadre creux autour du bloc dégradé
  Fenetre_Afficher_cadre_creux(171,26,18,66);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*172),Fenetre_Pos_Y+(Menu_Facteur_Y*27),
        Menu_Facteur_X<<4,Menu_Facteur_Y<<6,CM_Noir);
  // Tracé d'un cadre creux autour de tous les dégradés
  Fenetre_Afficher_cadre_creux(223,34,66,50);
  Shade_Blocs_degrades();
  // Tracé d'un cadre autour de la zone de définition de dégradés
  Fenetre_Afficher_cadre(5,124,262,61);
  Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);

  // Déclaration & tracé des boutons d'édition de shade
  Fenetre_Definir_bouton_normal(  6,107,27,14,"Ins"  ,0,1,SDLK_INSERT);  // 8
  Fenetre_Definir_bouton_normal( 38,107,27,14,"Del"  ,0,1,SDLK_DELETE);  // 9
  Fenetre_Definir_bouton_normal( 66,107,43,14,"Blank",1,1,SDLK_b);  // 10
  Fenetre_Definir_bouton_normal(110,107,27,14,"Inv"  ,1,1,SDLK_i);  // 11
  Fenetre_Definir_bouton_normal(138,107,27,14,"Swp"  ,1,1,SDLK_s);  // 12

  // Déclaration & tracé des boutons de taggage
  Print_dans_fenetre(268,123,"Disbl"/*"Dsabl"*/,CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_normal(274,133,27,14,"Set"   ,0,1,SDLK_F1); // 13
  Fenetre_Definir_bouton_normal(274,148,27,14,"Clr"   ,0,1,SDLK_F2); // 14

  // Déclaration & tracé de la zone de saisie du pas
  Print_dans_fenetre(272,165,"Step",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(274,174,3);                         // 15
  Bouton_saisie=Fenetre_Liste_boutons_special;
  Num2str(Shade_Liste[Shade_Actuel].Pas,Chaine,3);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie,Chaine);

  // Bouton Undo
  Fenetre_Definir_bouton_normal(170,107,35,14,"Undo",1,1,SDLK_u);   // 16
  // Bouton Clear
  Fenetre_Definir_bouton_normal(278,87,27,14,"Clr",0,1,SDLK_BACKSPACE);     // 17

  // Bouton Mode
  Fenetre_Definir_bouton_normal(244,107,60,14,"",0,1,SDLK_TAB);       // 18

  // Affichage du n° de shade actif
  Num2str(Shade_Actuel+1,Chaine,1);
  Print_dans_fenetre(210,55,Chaine,CM_Noir,CM_Clair);

  memcpy(Buffer     ,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
  memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));

  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*310,Menu_Facteur_Y*190);

  Afficher_curseur();

  do
  {
    Ancien_Mouse_X=Ancien_Mouse_X2=Mouse_X;
    Ancien_Mouse_Y=Ancien_Mouse_Y2=Mouse_Y;
    Ancien_Mouse_K=Ancien_Mouse_K2=Mouse_K;

    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case  0 :
        break;
      case -1 :
      case  1 : // Gestion de la palette
        if ( (Mouse_X!=Ancien_Mouse_X) || (Mouse_Y!=Ancien_Mouse_Y) || (Mouse_K!=Ancien_Mouse_K) )
        {
          Effacer_curseur();
          Couleur_temporaire=(Bouton_clicke==1) ? Fenetre_Attribut2 : Lit_pixel(Mouse_X,Mouse_Y);

          if (!Ancien_Mouse_K)
          { // On vient de clicker

            // On met à jour l'intervalle du Shade
            Premiere_couleur=Derniere_couleur=Couleur_temporaire;
            // On tagge le bloc
            Tagger_intervalle_palette(Premiere_couleur,Derniere_couleur);
            // Tracé du bloc dégradé:
            Bloc_degrade_dans_fenetre(172,27,Premiere_couleur,Derniere_couleur);
          }
          else
          { // On maintient le click, on va donc tester si le curseur bouge
            if (Couleur_temporaire!=Derniere_couleur)
            {
              Derniere_couleur=Couleur_temporaire;

              // On tagge le bloc
              if (Premiere_couleur<=Couleur_temporaire)
              {
                Tagger_intervalle_palette(Premiere_couleur,Derniere_couleur);
                Bloc_degrade_dans_fenetre(172,27,Premiere_couleur,Derniere_couleur);
              }
              else
              {
                Tagger_intervalle_palette(Derniere_couleur,Premiere_couleur);
                Bloc_degrade_dans_fenetre(172,27,Derniere_couleur,Premiere_couleur);
              }
            }
          }

          // On affiche le numéro de la couleur sélectionnée
          Afficher_couleur_selectionnee(Premiere_couleur,Derniere_couleur);

          Afficher_curseur();
        }
        break;

      case  2 : // Gestion du changement de Shade (scroller)
        Effacer_curseur();
        Shade_Actuel=Fenetre_Attribut2;
        // Affichade du n° de shade actif
        Num2str(Shade_Actuel+1,Chaine,1);
        Print_dans_fenetre(210,55,Chaine,CM_Noir,CM_Clair);
        // Affichade du Pas
        Num2str(Shade_Liste[Shade_Actuel].Pas,Chaine,3);
        Print_dans_fenetre(276,176,Chaine,CM_Noir,CM_Clair);
        // Tracé du bloc dégradé:
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        // On place le nouveau shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        break;

      case  3 : // Gestion de la zone de définition de shades
        if (((((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-127)%7)<4)
        if ( (Mouse_X!=Ancien_Mouse_X2) || (Mouse_Y!=Ancien_Mouse_Y2) || (Mouse_K!=Ancien_Mouse_K2) )
        {
          Effacer_curseur();
          Select_Fin=(((((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-127)/7)<<6)+
                      ((((Mouse_X-Fenetre_Pos_X)/Menu_Facteur_X)-8 )>>2);
          if (!Ancien_Mouse_K2) // On vient de clicker
            Select_Debut=Select_Fin;
          Tagger_shades(Select_Debut,Select_Fin);
          Afficher_couleur_case_selectionnee(Select_Debut,Select_Fin);
          Afficher_curseur();
        }
        break;

      case  6 : // Copy
        memcpy(Buffer,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        break;

      case  7 : // Paste
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        memcpy(Shade_Liste[Shade_Actuel].Liste,Buffer,512*sizeof(word));
        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case  8 : // Insert
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        if (Premiere_couleur<=Derniere_couleur)
          Temp=Derniere_couleur-Premiere_couleur;
        else
          Temp=Premiere_couleur-Derniere_couleur;

        if (Select_Debut==Select_Fin) // Une couleur sélectionnée
        {
          if (Fenetre_Attribut1==2)
            Supprimer_shade(Select_Debut,Select_Debut+Temp);
        }
        else                          // Un bloc sélectionné
        {
          Supprimer_shade(Select_Debut,Select_Fin);

          if (Premiere_couleur<=Derniere_couleur)
            Temp=Derniere_couleur-Premiere_couleur;
          else
            Temp=Premiere_couleur-Derniere_couleur;

          if (Select_Debut<Select_Fin)
            Select_Fin=Select_Debut+Temp;
          else
          {
            Select_Debut=Select_Fin;
            Select_Fin+=Temp;
          }
        }

        if (Select_Debut<Select_Fin)
          Select_Fin=Select_Debut+Temp;
        else
        {
          Select_Debut=Select_Fin;
          Select_Fin+=Temp;
        }
        Inserer_shade(Premiere_couleur,Derniere_couleur,Select_Debut);

        // On sélectionne la position juste après ce qu'on vient d'insérer
        Select_Debut+=Temp+1;
        if (Select_Debut>=512)
          Select_Debut=511;
        Select_Fin=Select_Debut;

        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case  9 : // Delete
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        Supprimer_shade(Select_Debut,Select_Fin);
        if (Select_Debut<=Select_Fin)
          Select_Fin=Select_Debut;
        else
          Select_Debut=Select_Fin;
        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case 10 : // Blank
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        if (Fenetre_Attribut1==A_DROITE)  // Click droit
        {
          if (Select_Debut!=Select_Fin)
          {
            if (Select_Debut<=Select_Fin)
            {
              Inserer_case_vide_dans_shade(Select_Debut);
              Inserer_case_vide_dans_shade(Select_Fin+2);
            }
            else
            {
              Inserer_case_vide_dans_shade(Select_Fin);
              Inserer_case_vide_dans_shade(Select_Debut+2);
            }
          }
          else
            Inserer_case_vide_dans_shade(Select_Debut);

          if (Select_Debut<511) Select_Debut++;
          if (Select_Fin<511) Select_Fin++;
        }
        else                              // Click gauche
        {
          if (Select_Debut<=Select_Fin)
          {
            Temp=Select_Debut;
            Temp2=Select_Fin;
          }
          else
          {
            Temp=Select_Fin;
            Temp2=Select_Debut;
          }
          while (Temp<=Temp2)
            Shade_Liste[Shade_Actuel].Liste[Temp++]=0x0100;
        }

        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case 11 : // Invert
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        if (Select_Debut<=Select_Fin)
        {
          Temp=Select_Debut;
          Temp2=Select_Fin;
        }
        else
        {
          Temp=Select_Fin;
          Temp2=Select_Debut;
        }

        for (;Temp<Temp2;Temp++,Temp2--)
        {
          Case_temporaire=Shade_Liste[Shade_Actuel].Liste[Temp];
          Shade_Liste[Shade_Actuel].Liste[Temp]=Shade_Liste[Shade_Actuel].Liste[Temp2];
          Shade_Liste[Shade_Actuel].Liste[Temp2]=Case_temporaire;
        }

        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case 12 : // Swap
        Case_temporaire=Attendre_click_dans_shade();
        if (Case_temporaire<512)
        {
          // On place le shade dans le buffer du Undo
          memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
          // Et on le modifie
          // On échange le bloc avec sa destination
          if (Select_Debut<=Select_Fin)
          {
            Temp=(Case_temporaire+Select_Fin-Select_Debut<512)?Select_Fin+1-Select_Debut:512-Case_temporaire;
            Swap_shade(Select_Debut,Case_temporaire,Temp);
          }
          else
          {
            Temp=(Case_temporaire+Select_Debut-Select_Fin<512)?Select_Debut+1-Select_Fin:512-Case_temporaire;
            Swap_shade(Select_Fin,Case_temporaire,Temp);
          }
          // On place la sélection sur la nouvelle position du bloc
          Select_Debut=Case_temporaire;
          Select_Fin=Select_Debut+Temp-1;
          // Et on raffiche tout
          Effacer_curseur();
          Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
          Afficher_curseur();
        }
        break;

      case 13 : // Set (disable)
      case 14 : // Clear (enable)
        // On place le shade dans le buffer du Undo
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        // Et on le modifie
        if (Select_Debut<Select_Fin)
        {
          Temp=Select_Debut;
          Temp2=Select_Fin;
        }
        else
        {
          Temp=Select_Fin;
          Temp2=Select_Debut;
        }

        if (Bouton_clicke==13)
          for (;Temp<=Temp2;Temp++)
            Shade_Liste[Shade_Actuel].Liste[Temp]|=0x8000;
        else
          for (;Temp<=Temp2;Temp++)
            Shade_Liste[Shade_Actuel].Liste[Temp]&=0x7FFF;

        Effacer_curseur();
        Tagger_shades(Select_Debut,Select_Fin);
        Shade_Blocs_degrades();
        Afficher_curseur();
        break;

      case 15 : // Saisie du pas
        Effacer_curseur();
        Num2str(Shade_Liste[Shade_Actuel].Pas,Chaine,3);
        Readline(276,176,Chaine,3,1);
        Temp=atoi(Chaine);
        // On corrige le pas
        if (!Temp)
        {
          Temp=1;
          Num2str(Temp,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie,Chaine);
        }
        else if (Temp>255)
        {
          Temp=255;
          Num2str(Temp,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie,Chaine);
        }
        Shade_Liste[Shade_Actuel].Pas=Temp;
        Afficher_curseur();
        break;

      case 16 : // Undo
        memcpy(Pointeur_temp,Buffer_Undo,512*sizeof(word));
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        memcpy(Shade_Liste[Shade_Actuel].Liste,Pointeur_temp,512*sizeof(word));

        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case 17 : // Clear
        memcpy(Buffer_Undo,Shade_Liste[Shade_Actuel].Liste,512*sizeof(word));
        for (Temp=0;Temp<512;Temp++)
          Shade_Liste[Shade_Actuel].Liste[Temp]=0x0100;
        Effacer_curseur();
        Afficher_tout_le_shade(Premiere_couleur,Derniere_couleur,Select_Debut,Select_Fin);
        Afficher_curseur();
        break;

      case 18 : // Mode
        Shade_Liste[Shade_Actuel].Mode=(Shade_Liste[Shade_Actuel].Mode+1)%3;
        Effacer_curseur();
        Afficher_mode_du_shade(250,110,Shade_Liste[Shade_Actuel].Mode);
        Afficher_curseur();
    }

    if (!Mouse_K)
    switch (Touche)
    {
      case SDLK_LEFTBRACKET : // Décaler couleur dans palette vers la gauche
      case SDLK_RIGHTBRACKET : // Décaler couleur dans palette vers la droite
        if (Premiere_couleur==Derniere_couleur)
        {
          if (Touche==SDLK_LEFTBRACKET)
          {
            Premiere_couleur--;
            Derniere_couleur--;
          }
          else
          {
            Premiere_couleur++;
            Derniere_couleur++;
          }
          Effacer_curseur();
          Tagger_intervalle_palette(Premiere_couleur,Premiere_couleur);
          Block(Fenetre_Pos_X+(Menu_Facteur_X*172),
                Fenetre_Pos_Y+(Menu_Facteur_Y*27),
                Menu_Facteur_X<<4,Menu_Facteur_Y*64,Premiere_couleur);
          // On affiche le numéro de la couleur sélectionnée
          Afficher_couleur_selectionnee(Premiere_couleur,Derniere_couleur);
          Afficher_curseur();
        }
        break;

      case SDLK_UP    : // Select Haut
      case SDLK_DOWN  : // Select Bas
      case SDLK_LEFT  : // Select Gauche
      case SDLK_RIGHT : // Select Droite
        if (Select_Debut==Select_Fin)
        {
          switch (Touche)
          {
            case SDLK_UP : // Select Haut
              if (Select_Debut>=64)
              {
                Select_Debut-=64;
                Select_Fin-=64;
              }
              else
                Select_Debut=Select_Fin=0;
              break;
            case SDLK_DOWN : // Select Bas
              if (Select_Debut<448)
              {
                Select_Debut+=64;
                Select_Fin+=64;
              }
              else
                Select_Debut=Select_Fin=511;
              break;
            case SDLK_LEFT : // Select Gauche
              if (Select_Debut>0)
              {
                Select_Debut--;
                Select_Fin--;
              }
              break;
            default :     // Select Droite
              if (Select_Debut<511)
              {
                Select_Debut++;
                Select_Fin++;
              }
          }
          Effacer_curseur();
          Tagger_shades(Select_Debut,Select_Debut);
          Afficher_couleur_case_selectionnee(Select_Debut,Select_Debut);
          Afficher_curseur();
        }
        break;

      case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case SDLK_COMMA :
        Recuperer_couleur_derriere_fenetre(&Couleur,&Click);
        if (Click)
        {
          Effacer_curseur();
          Couleur_temporaire=Couleur;

          // On met à jour l'intervalle du Shade
          Premiere_couleur=Derniere_couleur=Couleur_temporaire;
          // On tagge le bloc
          Tagger_intervalle_palette(Premiere_couleur,Derniere_couleur);
          // Tracé du bloc dégradé:
          Bloc_degrade_dans_fenetre(172,27,Premiere_couleur,Derniere_couleur);

          // On affiche le numéro de la couleur sélectionnée
          Afficher_couleur_selectionnee(Premiere_couleur,Derniere_couleur);

          Afficher_curseur();
        }
        break;
      default:
        if (Touche==Bouton[BOUTON_AIDE].Raccourci_gauche)
          Fenetre_aide(BOUTON_EFFETS, "SHADE");
    }
  }
  while ((Bouton_clicke!=4) && (Bouton_clicke!=5));

  Fermer_fenetre();
  free(Buffer_Undo);
  free(Buffer);
  free(Pointeur_temp);

  return (Bouton_clicke==5);
}



void Bouton_Shade_Menu(void)
{
  struct T_Shade * Shade_Liste_Backup;        // Anciennes données des shades
  byte Ancien_shade;                              // Ancien n° de shade actif


  // Backup des anciennes données
  Shade_Liste_Backup=(struct T_Shade *)malloc(sizeof(Shade_Liste));
  memcpy(Shade_Liste_Backup,Shade_Liste,sizeof(Shade_Liste));
  Ancien_shade=Shade_Actuel;

  if (!Menu_Shade()) // Cancel
  {
    memcpy(Shade_Liste,Shade_Liste_Backup,sizeof(Shade_Liste));
    Shade_Actuel=Ancien_shade;
  }
  else // OK
  {
    Liste2tables(Shade_Liste[Shade_Actuel].Liste,
                 Shade_Liste[Shade_Actuel].Pas,
                 Shade_Liste[Shade_Actuel].Mode,
                 Shade_Table_gauche,Shade_Table_droite);

    // Si avant de rentrer dans le menu on n'était pas en mode Shade
    if (!Shade_Mode)
      Bouton_Shade_Mode(); // => On y passe (cool!)
  }

  free(Shade_Liste_Backup);

  Afficher_curseur();
}




void Bouton_Quick_shade_Menu(void)
{
  short Bouton_clicke;
  int Temp;
  char Chaine[4];
  byte Step_Backup=Quick_shade_Step; // Backup des
  byte Loop_Backup=Quick_shade_Loop; // anciennes données


  Ouvrir_fenetre(142,56,"Quick-shade");

  Fenetre_Definir_bouton_normal(76,36,60,14,"OK",0,1,SDLK_RETURN);     // 1
  Fenetre_Definir_bouton_normal( 6,36,60,14,"Cancel",0,1,SDLK_ESCAPE); // 2
  Fenetre_Definir_bouton_normal(76,18,60,14,"",0,1,SDLK_TAB);       // 3
  Afficher_mode_du_shade(83,21,Quick_shade_Loop);

  // Déclaration & tracé de la zone de saisie du pas
  Print_dans_fenetre(5,21,"Step",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(40,19,3);                         // 4
  Num2str(Quick_shade_Step,Chaine,3);
  Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);

  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*142,Menu_Facteur_Y*56);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case 3 : // Mode
        Quick_shade_Loop=(Quick_shade_Loop+1)%3;
        Effacer_curseur();
        Afficher_mode_du_shade(83,21,Quick_shade_Loop);
        Afficher_curseur();
        break;

      case 4 : // Saisie du pas
        Effacer_curseur();
        Num2str(Quick_shade_Step,Chaine,3);
        Readline(42,21,Chaine,3,1);
        Temp=atoi(Chaine);
        // On corrige le pas
        if (!Temp)
        {
          Temp=1;
          Num2str(Temp,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
        }
        else if (Temp>255)
        {
          Temp=255;
          Num2str(Temp,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
        }
        Quick_shade_Step=Temp;
        Afficher_curseur();
    }
    if (Touche==Bouton[BOUTON_AIDE].Raccourci_gauche)
      Fenetre_aide(BOUTON_EFFETS, "QUICK SHADE");
  }
  while ((Bouton_clicke!=1) && (Bouton_clicke!=2));

  Fermer_fenetre();

  if (Bouton_clicke==2) // Cancel
  {
    Quick_shade_Step=Step_Backup;
    Quick_shade_Loop=Loop_Backup;
  }
  else // OK
  {
    // Si avant de rentrer dans le menu on n'était pas en mode Quick-Shade
    if (!Quick_shade_Mode)
      Bouton_Quick_shade_Mode(); // => On y passe (cool!)
  }

  Afficher_curseur();
}
