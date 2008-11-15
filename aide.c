/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
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
#include <stdio.h>
#include <string.h>

#if defined(__WIN32__)
    #include <windows.h>
#elif defined(__macosx__)
    #include <sys/param.h>
    #include <sys/mount.h>
#elif defined (__linux__)
    #include <sys/vfs.h>
#endif

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "graph.h"
#include "moteur.h"
#include "tables_aide.h"
#include "aide.h"
#include "sdlscreen.h"
#include "texte.h"

extern char SVNRevision[];

// -- Menu d'aide -----------------------------------------------------------

void Afficher_aide(void)
{
  short  X;                   // Indices d'affichage d'un caractère
  short  Y;
  short  Position_X;          // Parcours de remplissage du buffer de ligne
  short  Indice_de_ligne;     // 0-15 (16 lignes de textes)
  short  Indice_de_caractere; // Parcours des caractères d'une ligne
  short  Ligne_de_depart=Position_d_aide_en_cours;
  short  Repeat_Menu_Facteur_X;
  short  Repeat_Menu_Facteur_Y;
  short  Pos_Reel_X;
  short  Pos_Reel_Y;
  int    Curseur;
  short  Largeur;             // Largeur physique d'une ligne de texte
  char   TypeLigne;           // N: Normale, T: Titre, S: Sous-titre
                              // -: Ligne inférieur de sous-titre
  const char * Ligne;

  Pos_Reel_X=Fenetre_Pos_X+(13*Menu_Facteur_X);
  Pos_Reel_Y=Fenetre_Pos_Y+(19*Menu_Facteur_Y);

  for (Indice_de_ligne=0;Indice_de_ligne<16;Indice_de_ligne++)
  {
    // Raccourci au cas ou la section fait moins de 16 lignes
    if (Indice_de_ligne >= Table_d_aide[Section_d_aide_en_cours].Nombre_de_lignes)
    {
      Block (Pos_Reel_X,
           Pos_Reel_Y,
           44*6*Menu_Facteur_X,
           // 44 = Nb max de char (+1 pour éviter les plantages en mode X
           // causés par une largeur = 0)
           (Menu_Facteur_Y<<3) * (16 - Indice_de_ligne),
           CM_Noir);
      break;
    }
    // On affiche la ligne
    Ligne = Table_d_aide[Section_d_aide_en_cours].Table_aide[Ligne_de_depart + Indice_de_ligne];
    TypeLigne = Ligne[0];
    // Si c'est une sous-ligne de titre, on utilise le texte de la ligne précédente
    if (TypeLigne == '-' && (Ligne_de_depart + Indice_de_ligne > 0))
      Ligne = Table_d_aide[Section_d_aide_en_cours].Table_aide[Ligne_de_depart + Indice_de_ligne - 1];
    // On ignore le premier caractère
    Ligne++;
    // Calcul de la taille
    Largeur=strlen(Ligne);
    // Les lignes de titres prennent plus de place
    if (TypeLigne == 'T' || TypeLigne == '-')
      Largeur = Largeur*2;

    // Pour chaque ligne dans la fenêtre:
    for (Y=0;Y<8;Y++)
    {
      Position_X=0;
      // On crée une nouvelle ligne à splotcher
      for (Indice_de_caractere=0;Indice_de_caractere<Largeur;Indice_de_caractere++)
      {
        // Recherche du caractère dans la fonte de 315 symboles.
        // Ligne titre : Si l'indice est impair on dessine le quart de caractère
        // qui va a gauche, sinon celui qui va a droite.
        
        if (TypeLigne=='T')
          Curseur=Caracteres_Aide_Titre_haut[Ligne[Indice_de_caractere/2]-' '] + (Indice_de_caractere & 1);
        else if (TypeLigne=='-')
          Curseur=Caracteres_Aide_Titre_bas[Ligne[Indice_de_caractere/2]-' '] + (Indice_de_caractere & 1);
        else if (TypeLigne=='S')
          Curseur=Caracteres_Aide_S[Ligne[Indice_de_caractere]-' '];
        else if (TypeLigne=='N')
          Curseur=Caracteres_Aide_N[Ligne[Indice_de_caractere]-' '];
        else
          Curseur=1; // Un garde-fou en cas de probleme
          
        // Un deuxième garde-fou
        if (Curseur < 0 || Curseur > 315)
          Curseur = 1; // '!' affiché pour les caractères non prévus
        
        for (X=0;X<6;X++)
          for (Repeat_Menu_Facteur_X=0;Repeat_Menu_Facteur_X<Menu_Facteur_X;Repeat_Menu_Facteur_X++)
            Buffer_de_ligne_horizontale[Position_X++]=Fonte_help[Curseur][X][Y];
      }

      // On la splotche
      for (Repeat_Menu_Facteur_Y=0;Repeat_Menu_Facteur_Y<Menu_Facteur_Y;Repeat_Menu_Facteur_Y++)
        Afficher_ligne(Pos_Reel_X,Pos_Reel_Y++,Largeur*Menu_Facteur_X*6,Buffer_de_ligne_horizontale);
    }

    // On efface la fin de la ligne:
    Block (Pos_Reel_X+Largeur*Menu_Facteur_X*6,
           Pos_Reel_Y-(8*Menu_Facteur_Y),
           ((44*6*Menu_Facteur_X)-Largeur*Menu_Facteur_X*6)+1,
           // 44 = Nb max de char (+1 pour éviter les plantages en mode X
           // causés par une largeur = 0)
           Menu_Facteur_Y<<3,
           CM_Noir);
  }
  UpdateRect(Fenetre_Pos_X+13*Menu_Facteur_X,Fenetre_Pos_Y+19*Menu_Facteur_Y,44*6*Menu_Facteur_X,16*8*Menu_Facteur_Y);
}


void Scroller_aide(void)
{
  Effacer_curseur();
  Fenetre_Liste_boutons_scroller->Position=Position_d_aide_en_cours;
  Calculer_hauteur_curseur_jauge(Fenetre_Liste_boutons_scroller);
  Fenetre_Dessiner_jauge(Fenetre_Liste_boutons_scroller);
  Afficher_aide();
  Afficher_curseur();
}


void Bouton_Aide(void)
{
  short Numero_bouton;
  
  // Aide contextuelle
  if (Touche!=0)
  {
    Numero_bouton = Numero_bouton_sous_souris();
    if (Numero_bouton != -1)
    {
      Fenetre_aide(Numero_bouton, NULL);
      return;
    }
  }
  Fenetre_aide(-1, NULL);
}
// Ouvre l'ecran d'aide. Passer -1 pour la section par défaut (ou derniere,)
// Ou un nombre de l'enumération NUMEROS_DE_BOUTONS pour l'aide contextuelle.
void Fenetre_aide(int Section, const char *Sous_section)
{
  short Bouton_clicke;
  short Nb_lignes;

  if (Section!=-1)
  {
    Section_d_aide_en_cours = 4 + Section;
    Position_d_aide_en_cours = 0;
  }
  Nb_lignes=Table_d_aide[Section_d_aide_en_cours].Nombre_de_lignes;
  if (Section!=-1 && Sous_section!=NULL)
  {
    int Indice=0;
    for (Indice=0; Indice<Nb_lignes; Indice++)
      if (Table_d_aide[Section_d_aide_en_cours].Table_aide[Indice][0] == 'T' &&
        !strcmp(Table_d_aide[Section_d_aide_en_cours].Table_aide[Indice]+1, Sous_section))
      {
        Position_d_aide_en_cours = Indice;
        break;
      }
  }


  Ouvrir_fenetre(310,175,"Help / About...");

  // dessiner de la fenêtre où va défiler le texte
  Fenetre_Afficher_cadre_creux(8,17,274,132);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*9),
        Fenetre_Pos_Y+(Menu_Facteur_Y*18),
        Menu_Facteur_X*272,Menu_Facteur_Y*130,CM_Noir);

  Fenetre_Definir_bouton_normal(266,153,35,14,"Exit",0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_scroller(290,18,130,Nb_lignes,
                                  16,Position_d_aide_en_cours);   // 2

  Fenetre_Definir_bouton_normal(  9,154, 6*8,14,"About"  ,1,1,SDLK_a); // 3

  Fenetre_Definir_bouton_normal( 9+6*8+4,154, 8*8,14,"License",1,1,SDLK_l); // 4
  Fenetre_Definir_bouton_normal( 9+6*8+4+8*8+4,154, 5*8,14,"Help",1,1,SDLK_h); // 5
  Fenetre_Definir_bouton_normal(9+6*8+4+8*8+4+5*8+4,154, 8*8,14,"Credits",1,1,SDLK_c); // 6

  Afficher_aide();

  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,310*Menu_Facteur_X,175*Menu_Facteur_Y);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case -1:
      case  0:
      case  1:
        break;
      default:
        Effacer_curseur();
        if (Bouton_clicke>2)
        {
          Section_d_aide_en_cours=Bouton_clicke-3;
          Position_d_aide_en_cours=0;
          Nb_lignes=Table_d_aide[Section_d_aide_en_cours].Nombre_de_lignes;
          Fenetre_Liste_boutons_scroller->Position=0;
          Fenetre_Liste_boutons_scroller->Nb_elements=Nb_lignes;
          Calculer_hauteur_curseur_jauge(Fenetre_Liste_boutons_scroller);
          Fenetre_Dessiner_jauge(Fenetre_Liste_boutons_scroller);
        }
        else
          Position_d_aide_en_cours=Fenetre_Attribut2;

        Afficher_aide();
        Afficher_curseur();
    }


    // Gestion des touches de déplacement dans la liste
    switch (Touche)
    {
      case SDLK_UP : // Haut
        if (Position_d_aide_en_cours>0)
          Position_d_aide_en_cours--;
        Scroller_aide();
        break;
      case SDLK_DOWN : // Bas
        if (Position_d_aide_en_cours<Nb_lignes-16)
          Position_d_aide_en_cours++;
        Scroller_aide();
        break;
      case SDLK_PAGEUP : // PageUp
        if (Position_d_aide_en_cours>15)
          Position_d_aide_en_cours-=15;
        else
          Position_d_aide_en_cours=0;
        Scroller_aide();
        break;
      case SDLK_PAGEDOWN : // PageDown
        if (Position_d_aide_en_cours<Nb_lignes-31)
          Position_d_aide_en_cours+=15;
        else
          Position_d_aide_en_cours=Nb_lignes-16;
        Scroller_aide();
        break;
      case SDLK_HOME : // Home
        Position_d_aide_en_cours=0;
        Scroller_aide();
        break;
      case SDLK_END : // End
        Position_d_aide_en_cours=Nb_lignes-16;
        Scroller_aide();
        break;
    }

  }
  while ((Bouton_clicke!=1) && (Touche!=SDLK_RETURN));

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_AIDE);
  Afficher_curseur();
}


#define STATS_COULEUR_TITRES  CM_Blanc
#define STATS_COULEUR_DONNEES CM_Clair
void Bouton_Stats(void)
{
  short Bouton_clicke;
  char  Buffer[37];
  dword Utilisation_couleur[256];
  unsigned long long freeRam;
  uint64_t Taille = 0;

  Ouvrir_fenetre(310,174,"Statistics");

  // Dessin de la fenetre ou va s'afficher le texte
  Fenetre_Afficher_cadre_creux(8,17,294,132);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*9),
        Fenetre_Pos_Y+(Menu_Facteur_Y*18),
        Menu_Facteur_X*292,Menu_Facteur_Y*130,CM_Noir);

  Fenetre_Definir_bouton_normal(120,153,70,14,"OK",0,1,SDLK_ESCAPE); // 1

  // Affichage du numéro de version
  Print_dans_fenetre(10,19,"Program version:",STATS_COULEUR_TITRES,CM_Noir);
  sprintf(Buffer,"GrafX 2.00 %s%s",ALPHA_BETA,POURCENTAGE_VERSION);
  Print_dans_fenetre(146,19,Buffer,STATS_COULEUR_DONNEES,CM_Noir);
  Print_dans_fenetre(10,27,"SVN revision #:",STATS_COULEUR_TITRES,CM_Noir);
  Print_dans_fenetre(146,27,SVNRevision,STATS_COULEUR_DONNEES,CM_Noir);
  Print_dans_fenetre(10,35,"Build options:",STATS_COULEUR_TITRES,CM_Noir);
  Print_dans_fenetre(146,35,Support_TrueType()?"TTF fonts":"no TTF fonts",STATS_COULEUR_DONNEES,CM_Noir);


  // Affichage de la mémoire restante
  Print_dans_fenetre(10,51,"Free memory: ",STATS_COULEUR_TITRES,CM_Noir);

  freeRam = Memoire_libre();
  
  if(freeRam > (100ULL*1024*1024*1024))
        sprintf(Buffer,"%d Gigabytes",(unsigned int)(freeRam/(1024*1024*1024)));
  else if(freeRam > (100*1024*1024))
        sprintf(Buffer,"%d Megabytes",(unsigned int)(freeRam/(1024*1024)));
  else if(freeRam > 100*1024)
        sprintf(Buffer,"%d Kilobytes",(unsigned int)(freeRam/1024));
  else
        sprintf(Buffer,"%d bytes",(unsigned int)freeRam);
  Print_dans_fenetre(114,51,Buffer,STATS_COULEUR_DONNEES,CM_Noir);

  // Affichage de l'espace disque libre
  sprintf(Buffer,"Free space on %c:",Principal_Repertoire_courant[0]);
  Print_dans_fenetre(10,67,Buffer,STATS_COULEUR_TITRES,CM_Noir);

#if defined(__WIN32__)
    {
      ULARGE_INTEGER tailleU;
      GetDiskFreeSpaceEx(Principal_Repertoire_courant,&tailleU,NULL,NULL);
      Taille = tailleU.QuadPart;
    }
#elif defined(__linux__) || (__macosx__)
    // Note: under MacOSX, both macros are defined anyway.
    {
      struct statfs Informations_Disque;
      statfs(Principal_Repertoire_courant,&Informations_Disque);
      Taille=(uint64_t) Informations_Disque.f_bfree * (uint64_t) Informations_Disque.f_bsize;
    }
#else
    // Free disk space is only for shows. Other platforms can display 0.
#endif
  
    if(Taille > (100ULL*1024*1024*1024))
        sprintf(Buffer,"%d Gigabytes",(unsigned int)(Taille/(1024*1024*1024)));
    else if(Taille > (100*1024*1024))
        sprintf(Buffer,"%d Megabytes",(unsigned int)(Taille/(1024*1024)));
    else if(Taille > (100*1024))
        sprintf(Buffer,"%d Kilobytes",(unsigned int)(Taille/1024));
    else 
        sprintf(Buffer,"%d bytes",(unsigned int)Taille);
    Print_dans_fenetre(146,67,Buffer,STATS_COULEUR_DONNEES,CM_Noir);

  // Affichage des informations sur l'image
  Print_dans_fenetre(10,83,"Picture info.:",STATS_COULEUR_TITRES,CM_Noir);

  // Affichage des dimensions de l'image
  Print_dans_fenetre(18,91,"Dimensions :",STATS_COULEUR_TITRES,CM_Noir);
  sprintf(Buffer,"%dx%d",Principal_Largeur_image,Principal_Hauteur_image);
  Print_dans_fenetre(122,91,Buffer,STATS_COULEUR_DONNEES,CM_Noir);

  // Affichage du nombre de couleur utilisé
  Print_dans_fenetre(18,99,"Colors used:",STATS_COULEUR_TITRES,CM_Noir);
  memset(Utilisation_couleur,0,sizeof(Utilisation_couleur));
  sprintf(Buffer,"%d",Palette_Compter_nb_couleurs_utilisees(Utilisation_couleur));
  Print_dans_fenetre(122,99,Buffer,STATS_COULEUR_DONNEES,CM_Noir);

  // Affichage des dimensions de l'écran
  Print_dans_fenetre(10,115,"Resolution:",STATS_COULEUR_TITRES,CM_Noir);
  sprintf(Buffer,"%dx%d",Largeur_ecran,Hauteur_ecran);
  Print_dans_fenetre(106,115,Buffer,STATS_COULEUR_DONNEES,CM_Noir);

  UpdateRect(Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*310,Menu_Facteur_Y*174);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while ( (Bouton_clicke!=1) && (Touche!=SDLK_RETURN) );

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_AIDE);
  Afficher_curseur();
}
