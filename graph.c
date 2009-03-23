/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

********************************************************************************

    Drawing functions and effects.

*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "struct.h"
#include "moteur.h"
#include "boutons.h"
#include "pages.h"
#include "erreurs.h"
#include "sdlscreen.h"
#include "graph.h"
#include "divers.h"
#include "pxsimple.h"
#include "pxtall.h"
#include "pxwide.h"
#include "pxdouble.h"
#include "windows.h"

// Fonction qui met à jour la zone de l'image donnée en paramètre sur l'écran.
// Tient compte du décalage X et Y et du zoom, et fait tous les controles nécessaires
void Mettre_Ecran_A_Jour(short x, short y, short width, short height)
{
  short L_effectif, H_effectif;
  short X_effectif;
  short Y_effectif;
  short Diff;

  // Première étape, si L ou H est négatif, on doit remettre la zone à l'endroit
  if (width < 0)
  {
    x += width;
    width = - width;
  }

  if (height < 0)
  {
    y += height;
    height = - height;
  }

  // D'abord on met à jour dans la zone écran normale
  Diff = x-Principal_Decalage_X;
  if (Diff<0)
  {
    L_effectif = width + Diff;
    X_effectif = 0;
  }
  else
  {
    L_effectif = width;
    X_effectif = Diff;
  }
  Diff = y-Principal_Decalage_Y;
  if (Diff<0)
  {
    H_effectif = height + Diff;
    Y_effectif = 0;
  }
  else
  {
    H_effectif = height;
    Y_effectif = Diff;
  }

  // Normalement il ne faudrait pas updater au delà du split quand on est en mode loupe,
  // mais personne ne devrait demander d'update en dehors de cette limite, même le fill est contraint
  // a rester dans la zone visible de l'image
  // ...Sauf l'affichage de brosse en preview - yr
  if(Loupe_Mode && X_effectif + L_effectif > Principal_Split)
    L_effectif = Principal_Split - X_effectif;
  else if(X_effectif + L_effectif > Largeur_ecran)
    L_effectif = Largeur_ecran - X_effectif;

  if(Y_effectif + H_effectif > Menu_Ordonnee)
    H_effectif = Menu_Ordonnee - Y_effectif;
  /*
  SDL_Rect r;
  r.x=X_effectif;
  r.y=Y_effectif;
  r.h=H_effectif;
  r.w=L_effectif;
  SDL_FillRect(Ecran_SDL,&r,3);
  */
  UpdateRect(X_effectif,Y_effectif,L_effectif,H_effectif);

  // Et ensuite dans la partie zoomée
  if(Loupe_Mode)
  {
    // Clipping en X
    X_effectif = (x-Loupe_Decalage_X)*Loupe_Facteur;
    Y_effectif = (y-Loupe_Decalage_Y)*Loupe_Facteur;
    L_effectif = width * Loupe_Facteur;
    H_effectif = height * Loupe_Facteur;

    if (X_effectif < 0)
    {
      L_effectif+=X_effectif;
      if (L_effectif<0)
        return;

      X_effectif = Principal_Split + LARGEUR_BARRE_SPLIT*Menu_Facteur_X;
    }
    else
      X_effectif += Principal_Split + LARGEUR_BARRE_SPLIT*Menu_Facteur_X;
    Diff = X_effectif+L_effectif-Largeur_ecran;
    if (Diff>0)
    {
      L_effectif -=Diff;
      if (L_effectif<0)
        return;
    }


    // Clipping en Y
    if (Y_effectif < 0)
    {
      H_effectif+=Y_effectif;
      if (H_effectif<0)
        return;
      Y_effectif = 0;
    }
    Diff = Y_effectif+H_effectif-Menu_Ordonnee;
    if (Diff>0)
    {
      H_effectif -=Diff;
      if (H_effectif<0)
        return;
    }

 // Très utile pour le debug :)
    /*SDL_Rect r;
    r.x=X_effectif;
    r.y=Y_effectif;
    r.h=H_effectif;
    r.w=L_effectif;
    SDL_FillRect(Ecran_SDL,&r,3);*/

    UpdateRect(X_effectif,Y_effectif,L_effectif,H_effectif);
  }
}



void Transformer_point(short x, short y, float cosA, float sinA,
                       short * Xr, short * Yr)
{
  *Xr=Round(((float)x*cosA)+((float)y*sinA));
  *Yr=Round(((float)y*cosA)-((float)x*sinA));
}



//--------------------- Initialisation d'un mode vidéo -----------------------

int Initialiser_mode_video(int width, int height, int fullscreen)
{
  int Sensibilite_X;
  int Sensibilite_Y;
  int Indice;
  int factor;

  // Pour la première entrée dans cette fonction
  if (Pixel_width<1)
    Pixel_width=1;
  if (Pixel_height<1)
    Pixel_height=1;
  
  if (Largeur_ecran!=width/Pixel_width ||
      Hauteur_ecran!=height/Pixel_height ||
      Mode_video[Resolution_actuelle].Fullscreen != fullscreen)
  {
    switch (Pixel_ratio)
    {
        case PIXEL_SIMPLE:
            Pixel_width=1;
            Pixel_height=1;
            Pixel = Pixel_Simple ;
            Lit_pixel= Lit_Pixel_Simple ;
            Display_screen = Afficher_partie_de_l_ecran_Simple ;
            Block = Block_Simple ;
            Pixel_Preview_Normal = Pixel_Preview_Normal_Simple ;
            Pixel_Preview_Loupe = Pixel_Preview_Loupe_Simple ;
            Ligne_horizontale_XOR = Ligne_horizontale_XOR_Simple ;
            Ligne_verticale_XOR = Ligne_verticale_XOR_Simple ;
            Display_brush_Color = Display_brush_Color_Simple ;
            Display_brush_Mono = Display_brush_Mono_Simple ;
            Clear_brush = Clear_brush_Simple ;
            Remap_screen = Remap_screen_Simple ;
            Afficher_ligne = Afficher_une_ligne_ecran_Simple ;
            Afficher_ligne_fast = Afficher_une_ligne_ecran_Simple ;
            Lire_ligne = Lire_une_ligne_ecran_Simple ;
            Display_zoomed_screen = Afficher_partie_de_l_ecran_zoomee_Simple ;
            Display_brush_Color_zoom = Display_brush_Color_zoom_Simple ;
            Display_brush_Mono_zoom = Display_brush_Mono_zoom_Simple ;
            Clear_brush_zoom = Clear_brush_zoom_Simple ;
            Affiche_brosse = Affiche_brosse_Simple ;
        break;
        case PIXEL_TALL:
            Pixel_width=1;
            Pixel_height=2;
            Pixel = Pixel_Tall;
            Lit_pixel= Lit_Pixel_Tall;
            Display_screen = Afficher_partie_de_l_ecran_Tall;
            Block = Block_Tall;
            Pixel_Preview_Normal = Pixel_Preview_Normal_Tall;
            Pixel_Preview_Loupe = Pixel_Preview_Loupe_Tall;
            Ligne_horizontale_XOR = Ligne_horizontale_XOR_Tall;
            Ligne_verticale_XOR = Ligne_verticale_XOR_Tall;
            Display_brush_Color = Display_brush_Color_Tall;
            Display_brush_Mono = Display_brush_Mono_Tall;
            Clear_brush = Clear_brush_Tall;
            Remap_screen = Remap_screen_Tall;
            Afficher_ligne = Afficher_une_ligne_ecran_Tall;
            Afficher_ligne_fast = Afficher_une_ligne_ecran_Tall;
            Lire_ligne = Lire_une_ligne_ecran_Tall;
            Display_zoomed_screen = Afficher_partie_de_l_ecran_zoomee_Tall;
            Display_brush_Color_zoom = Display_brush_Color_zoom_Tall;
            Display_brush_Mono_zoom = Display_brush_Mono_zoom_Tall;
            Clear_brush_zoom = Clear_brush_zoom_Tall;
            Affiche_brosse = Affiche_brosse_Tall;
        break;
        case PIXEL_WIDE:
            Pixel_width=2;
            Pixel_height=1;
            Pixel = Pixel_Wide ;
            Lit_pixel= Lit_Pixel_Wide ;
            Display_screen = Afficher_partie_de_l_ecran_Wide ;
            Block = Block_Wide ;
            Pixel_Preview_Normal = Pixel_Preview_Normal_Wide ;
            Pixel_Preview_Loupe = Pixel_Preview_Loupe_Wide ;
            Ligne_horizontale_XOR = Ligne_horizontale_XOR_Wide ;
            Ligne_verticale_XOR = Ligne_verticale_XOR_Wide ;
            Display_brush_Color = Display_brush_Color_Wide ;
            Display_brush_Mono = Display_brush_Mono_Wide ;
            Clear_brush = Clear_brush_Wide ;
            Remap_screen = Remap_screen_Wide ;
            Afficher_ligne = Afficher_une_ligne_ecran_Wide ;
            Afficher_ligne_fast = Afficher_une_ligne_ecran_fast_Wide ;
            Lire_ligne = Lire_une_ligne_ecran_Wide ;
            Display_zoomed_screen = Afficher_partie_de_l_ecran_zoomee_Wide ;
            Display_brush_Color_zoom = Display_brush_Color_zoom_Wide ;
            Display_brush_Mono_zoom = Display_brush_Mono_zoom_Wide ;
            Clear_brush_zoom = Clear_brush_zoom_Wide ;
            Affiche_brosse = Affiche_brosse_Wide ;
        break;
        case PIXEL_DOUBLE:
            Pixel_width=2;
            Pixel_height=2;
            Pixel = Pixel_Double ;
            Lit_pixel= Lit_Pixel_Double ;
            Display_screen = Afficher_partie_de_l_ecran_Double ;
            Block = Block_Double ;
            Pixel_Preview_Normal = Pixel_Preview_Normal_Double ;
            Pixel_Preview_Loupe = Pixel_Preview_Loupe_Double ;
            Ligne_horizontale_XOR = Ligne_horizontale_XOR_Double ;
            Ligne_verticale_XOR = Ligne_verticale_XOR_Double ;
            Display_brush_Color = Display_brush_Color_Double ;
            Display_brush_Mono = Display_brush_Mono_Double ;
            Clear_brush = Clear_brush_Double ;
            Remap_screen = Remap_screen_Double ;
            Afficher_ligne = Afficher_une_ligne_ecran_Double ;
            Afficher_ligne_fast = Afficher_une_ligne_ecran_fast_Double ;
            Lire_ligne = Lire_une_ligne_ecran_Double ;
            Display_zoomed_screen = Afficher_partie_de_l_ecran_zoomee_Double ;
            Display_brush_Color_zoom = Display_brush_Color_zoom_Double ;
            Display_brush_Mono_zoom = Display_brush_Mono_zoom_Double ;
            Clear_brush_zoom = Clear_brush_zoom_Double ;
            Affiche_brosse = Affiche_brosse_Double ;
        break;
    }
    // Valeurs raisonnables: minimum 320x200
    if (!fullscreen)
    {
      if (width < 320*Pixel_width)
          width = 320*Pixel_width;
      if (height < 200*Pixel_height)
          height = 200*Pixel_height;
    }
    else
    {
      if (width < 320*Pixel_width || height < 200*Pixel_height)
        return 1;
    }
    // La largeur doit être un multiple de 4
#ifdef __amigaos4__
    // On AmigaOS the systems adds some more constraints on that ...
    width = (width + 15) & 0xFFFFFFF0;
#else
    width = (width + 3 ) & 0xFFFFFFFC;
#endif
    Set_Mode_SDL(&width, &height,fullscreen);
    Largeur_ecran = width/Pixel_width;
    Hauteur_ecran = height/Pixel_height;

    // Taille des menus
    if (Largeur_ecran/320 > Hauteur_ecran/200)
      factor=Hauteur_ecran/200;
    else
      factor=Largeur_ecran/320;

    switch (Config.Ratio)
    {
      case 1: // adapter tout
        Menu_Facteur_X=factor;
        Menu_Facteur_Y=factor;
        break;
      case 2: // adapter légèrement
        Menu_Facteur_X=factor-1;
        if (Menu_Facteur_X<1) Menu_Facteur_X=1;
        Menu_Facteur_Y=factor-1;
        if (Menu_Facteur_Y<1) Menu_Facteur_Y=1;
        break;
      default: // ne pas adapter
        Menu_Facteur_X=1;
        Menu_Facteur_Y=1;
    }
    if (Pixel_height>Pixel_width && Largeur_ecran>=Menu_Facteur_X*2*320)
      Menu_Facteur_X*=2;
    else if (Pixel_width>Pixel_height && Hauteur_ecran>=Menu_Facteur_Y*2*200)
      Menu_Facteur_Y*=2;
      
    if (Buffer_de_ligne_horizontale)
      free(Buffer_de_ligne_horizontale);
    Buffer_de_ligne_horizontale=(byte *)malloc(Pixel_width*((Largeur_ecran>Principal_Largeur_image)?Largeur_ecran:Principal_Largeur_image));

    Set_palette(Principal_Palette);

    Resolution_actuelle=0;
    if (fullscreen)
    {
      for (Indice=1; Indice<Nb_modes_video; Indice++)
      {
        if (Mode_video[Indice].Width/Pixel_width==Largeur_ecran &&
            Mode_video[Indice].Height/Pixel_height==Hauteur_ecran)
        {
          Resolution_actuelle=Indice;
          break;
        }
      }
    }

    Changer_cellules_palette();
    
    Menu_Ordonnee = Hauteur_ecran;
    if (Menu_visible)
      Menu_Ordonnee -= HAUTEUR_MENU * Menu_Facteur_Y;
    Menu_Ordonnee_Texte = Hauteur_ecran-(Menu_Facteur_Y<<3);

    Sensibilite_X = Config.Indice_Sensibilite_souris_X;
    Sensibilite_Y = Config.Indice_Sensibilite_souris_Y;
    Sensibilite_X>>=Mouse_Facteur_de_correction_X;
    Sensibilite_Y>>=Mouse_Facteur_de_correction_Y;
    Sensibilite_souris(Sensibilite_X?Sensibilite_X:1,Sensibilite_Y?Sensibilite_Y:1);

    Brouillon_Decalage_X=0; // |  Il faut penser à éviter les incohérences
    Brouillon_Decalage_Y=0; // |- de décalage du brouillon par rapport à
    Brouillon_Loupe_Mode=0; // |  la résolution.
  }
  if (Loupe_Mode)
  {
    Pixel_Preview=Pixel_Preview_Loupe;
  }
  else
  {
    Pixel_Preview=Pixel_Preview_Normal;
    // Recaler la vue (meme clipping que dans Scroller_ecran())
    if (Principal_Decalage_X+Largeur_ecran>Principal_Largeur_image)
      Principal_Decalage_X=Principal_Largeur_image-Largeur_ecran;
    if (Principal_Decalage_X<0)
      Principal_Decalage_X=0;
    if (Principal_Decalage_Y+Menu_Ordonnee>Principal_Hauteur_image)
      Principal_Decalage_Y=Principal_Hauteur_image-Menu_Ordonnee;
    if (Principal_Decalage_Y<0)
      Principal_Decalage_Y=0;
  }

  Calculer_donnees_loupe();
  if (Loupe_Mode)
    Recadrer_ecran_par_rapport_au_zoom();
  Calculer_limites();
  Calculer_coordonnees_pinceau();
  Afficher_ecran();
  
  Resize_Largeur=0;
  Resize_Hauteur=0;
  return 0;
}



  // -- Redimentionner l'image (nettoie l'écran virtuel) --

void Redimentionner_image(word Largeur_choisie,word Hauteur_choisie)
{
  word Ancienne_largeur=Principal_Largeur_image;
  word Ancienne_hauteur=Principal_Hauteur_image;

  // +-+-+
  // |C| |  A+B+C = Ancienne image
  // +-+A|
  // |B| |    C   = Nouvelle image
  // +-+-+

  if (Backup_avec_nouvelles_dimensions(1,Largeur_choisie,Hauteur_choisie))
  {
    // La nouvelle page a pu être allouée, elle est pour l'instant pleine de
    // 0s. Elle fait Principal_Largeur_image de large.

    // On copie donc maintenant la partie C dans la nouvelle image.
    Copier_une_partie_d_image_dans_une_autre(
      Ecran_backup,0,0,Min(Ancienne_largeur,Principal_Largeur_image),
      Min(Ancienne_hauteur,Principal_Hauteur_image),Ancienne_largeur,
      Principal_Ecran,0,0,Principal_Largeur_image);
  }
  else
  {
    // Afficher un message d'erreur
    Afficher_curseur();
    Message_Memoire_insuffisante();
    Effacer_curseur();
  }
}



void Remap_picture(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;

  // On commence par initialiser le tableau de booléens à faux
  for (Couleur=0;Couleur<=255;Couleur++)
    Utilisee[Couleur]=0;

  // On calcule la table d'utilisation des couleurs
  for (y_pos=0;y_pos<Brouillon_Hauteur_image;y_pos++)
    for (x_pos=0;x_pos<Brouillon_Largeur_image;x_pos++)
      Utilisee[Lit_pixel_dans_ecran_brouillon(x_pos,y_pos)]=1;

  //   On va maintenant se servir de la table "Utilisee" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans l'image, donc elles
  //       ne seront pas utilisées par Remap_general_LOWLEVEL.
  for (Couleur=0;Couleur<=255;Couleur++)
    if (Utilisee[Couleur])
      Utilisee[Couleur]=Meilleure_couleur(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].G,Brouillon_Palette[Couleur].B);

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_LOWLEVEL(Utilisee,Brouillon_Ecran,Brouillon_Largeur_image,Brouillon_Hauteur_image,Brouillon_Largeur_image);
}



void Get_colors_from_brush(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;

  if (Demande_de_confirmation("Modify current palette ?"))
  {
    Backup();

    // On commence par initialiser le tableau de booléen à faux
    for (Couleur=0;Couleur<=255;Couleur++)
      Utilisee[Couleur]=0;

    // On calcule la table d'utilisation des couleurs
    for (y_pos=0;y_pos<Brosse_Hauteur;y_pos++)
      for (x_pos=0;x_pos<Brosse_Largeur;x_pos++)
        Utilisee[Lit_pixel_dans_brosse(x_pos,y_pos)]=1;

    // On recopie dans la palette principale les teintes des couleurs utilisées
    // dans la palette du brouillon
    for (Couleur=0;Couleur<=255;Couleur++)
      if (Utilisee[Couleur])
      {
        Principal_Palette[Couleur].R=Brouillon_Palette[Couleur].R;
        Principal_Palette[Couleur].G=Brouillon_Palette[Couleur].G;
        Principal_Palette[Couleur].B=Brouillon_Palette[Couleur].B;
      }

    Set_palette(Principal_Palette);
    Calculer_couleurs_menu_optimales(Principal_Palette);
    Effacer_curseur();
    Afficher_ecran();
    Afficher_menu();
    Afficher_curseur();

    Principal_Image_modifiee=1;
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////////////////// GESTION DU FILLER /////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void Fill(short * Limite_atteinte_Haut  , short * Limite_atteinte_Bas,
          short * Limite_atteinte_Gauche, short * Limite_atteinte_Droite)
//
//   Cette fonction fait un remplissage classique d'une zone délimitée de
// l'image. Les limites employées sont Limite_Haut, Limite_Bas, Limite_Gauche
// et Limite_Droite. Le point de départ du remplissage est Pinceau_X,Pinceau_Y
// et s'effectue en théorie sur la couleur 1 et emploie la couleur 2 pour le
// remplissage. Ces restrictions sont dûes à l'utilisation qu'on en fait dans
// la fonction principale "Remplir", qui se charge de faire une gestion de
// tous les effets.
//   Cette fonction ne doit pas être directement appelée.
//
{
  short x_pos;   // Abscisse de balayage du segment, utilisée lors de l'"affichage"
  short line;   // Ordonnée de la ligne en cours de traitement
  short Debut_X; // Abscisse de départ du segment traité
  short Fin_X;   // Abscisse de fin du segment traité
  int   Modifs_effectuees;    // Booléen "On a fait une modif dans le dernier passage"
  int   Propagation_possible; // Booléen "On peut propager la couleur dans le segment"
  short Limite_courante_Bas;  // Intervalle vertical restreint
  short Limite_courante_Haut;
  int   Ligne_modifiee;       // Booléen "On a fait une modif dans la ligne"

  Modifs_effectuees=1;
  Limite_courante_Haut=Pinceau_Y;
  Limite_courante_Bas =Min(Pinceau_Y+1,Limite_Bas);
  *Limite_atteinte_Gauche=Pinceau_X;
  *Limite_atteinte_Droite=Pinceau_X+1;
  Pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y,2);

  while (Modifs_effectuees)
  {
    Modifs_effectuees=0;

    for (line=Limite_courante_Haut;line<=Limite_courante_Bas;line++)
    {
      Ligne_modifiee=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      Debut_X=Limite_Gauche;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (Debut_X<=Limite_Droite)
      {
        // On cherche son début
        while((Debut_X<=Limite_Droite) &&
                (Lit_pixel_dans_ecran_courant(Debut_X,line)!=1))
             Debut_X++;

        if (Debut_X<=Limite_Droite)
        {
          // Un segment de couleur 1 existe et commence à la position Debut_X.
          // On va donc en chercher la fin.
          for (Fin_X=Debut_X+1;(Fin_X<=Limite_Droite) &&
               (Lit_pixel_dans_ecran_courant(Fin_X,line)==1);Fin_X++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // Debut_X et qui se termine en Fin_X-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          Propagation_possible=(
            // Test de la présence d'un point à gauche du segment
            ((Debut_X>Limite_Gauche) &&
             (Lit_pixel_dans_ecran_courant(Debut_X-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((Fin_X-1<Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Fin_X    ,line)==2))
                               );

          // Test de la présence d'un point en haut du segment
          if (!Propagation_possible && (line>Limite_Haut))
            for (x_pos=Debut_X;x_pos<Fin_X;x_pos++)
              if (Lit_pixel_dans_ecran_courant(x_pos,line-1)==2)
              {
                Propagation_possible=1;
                break;
              }

          if (Propagation_possible)
          {
            if (Debut_X<*Limite_atteinte_Gauche)
              *Limite_atteinte_Gauche=Debut_X;
            if (Fin_X>*Limite_atteinte_Droite)
              *Limite_atteinte_Droite=Fin_X;
            // On remplit le segment de Debut_X à Fin_X-1.
            for (x_pos=Debut_X;x_pos<Fin_X;x_pos++)
              Pixel_dans_ecran_courant(x_pos,line,2);
            // On vient d'effectuer des modifications.
            Modifs_effectuees=1;
            Ligne_modifiee=1;
          }

          Debut_X=Fin_X+1;
        }
      }

      // Si on est en bas, et qu'on peut se propager vers le bas...
      if ( (line==Limite_courante_Bas) &&
           (Ligne_modifiee) &&
           (Limite_courante_Bas<Limite_Bas) )
        Limite_courante_Bas++; // On descend cette limite vers le bas
    }

    // Pour le prochain balayage vers le haut, on va se permettre d'aller
    // voir une ligne plus haut.
    // Si on ne le fait pas, et que la première ligne (Limite_courante_Haut)
    // n'était pas modifiée, alors cette limite ne serait pas remontée, donc
    // le filler ne progresserait pas vers le haut.
    if (Limite_courante_Haut>Limite_Haut)
      Limite_courante_Haut--;

    for (line=Limite_courante_Bas;line>=Limite_courante_Haut;line--)
    {
      Ligne_modifiee=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      Debut_X=Limite_Gauche;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (Debut_X<=Limite_Droite)
      {
        // On cherche son début
        for (;(Debut_X<=Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Debut_X,line)!=1);Debut_X++);

        if (Debut_X<=Limite_Droite)
        {
          // Un segment de couleur 1 existe et commence à la position Debut_X.
          // On va donc en chercher la fin.
          for (Fin_X=Debut_X+1;(Fin_X<=Limite_Droite) &&
               (Lit_pixel_dans_ecran_courant(Fin_X,line)==1);Fin_X++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // Debut_X et qui se termine en Fin_X-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          Propagation_possible=(
            // Test de la présence d'un point à gauche du segment
            ((Debut_X>Limite_Gauche) &&
             (Lit_pixel_dans_ecran_courant(Debut_X-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((Fin_X-1<Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Fin_X    ,line)==2))
                               );

          // Test de la présence d'un point en bas du segment
          if (!Propagation_possible && (line<Limite_Bas))
            for (x_pos=Debut_X;x_pos<Fin_X;x_pos++)
              if (Lit_pixel_dans_ecran_courant(x_pos,line+1)==2)
              {
                Propagation_possible=1;
                break;
              }

          if (Propagation_possible)
          {
            if (Debut_X<*Limite_atteinte_Gauche)
              *Limite_atteinte_Gauche=Debut_X;
            if (Fin_X>*Limite_atteinte_Droite)
              *Limite_atteinte_Droite=Fin_X;
            // On remplit le segment de Debut_X à Fin_X-1.
            for (x_pos=Debut_X;x_pos<Fin_X;x_pos++)
              Pixel_dans_ecran_courant(x_pos,line,2);
            // On vient d'effectuer des modifications.
            Modifs_effectuees=1;
            Ligne_modifiee=1;
          }

          Debut_X=Fin_X+1;
        }
      }

      // Si on est en haut, et qu'on peut se propager vers le haut...
      if ( (line==Limite_courante_Haut) &&
           (Ligne_modifiee) &&
           (Limite_courante_Haut>Limite_Haut) )
        Limite_courante_Haut--; // On monte cette limite vers le haut
    }
  }

  *Limite_atteinte_Haut=Limite_courante_Haut;
  *Limite_atteinte_Bas =Limite_courante_Bas;
  (*Limite_atteinte_Droite)--;
} // end de la routine de remplissage "Fill"


void Remplir(byte Couleur_de_remplissage)
//
//  Cette fonction fait un remplissage qui gère tous les effets. Elle fait
// appel à "Fill()".
//
{
  byte   Forme_curseur_avant_remplissage;
  byte * FX_Feedback_Ecran_avant_remplissage;
  short  x_pos,y_pos;
  short  Limite_atteinte_Haut  ,Limite_atteinte_Bas;
  short  Limite_atteinte_Gauche,Limite_atteinte_Droite;
  byte   Table_de_remplacement[256];


  // Avant toute chose, on vérifie que l'on n'est pas en train de remplir
  // en dehors de l'image:

  if ( (Pinceau_X>=Limite_Gauche) &&
       (Pinceau_X<=Limite_Droite) &&
       (Pinceau_Y>=Limite_Haut)   &&
       (Pinceau_Y<=Limite_Bas) )
  {
    // On suppose que le curseur est déjà caché.
    // Effacer_curseur();

    //   On va faire patienter l'utilisateur en lui affichant un joli petit
    // sablier:
    Forme_curseur_avant_remplissage=Forme_curseur;
    Forme_curseur=FORME_CURSEUR_SABLIER;
    Afficher_curseur();

    // On commence par effectuer un backup de l'image.
    Backup();

    // On fait attention au Feedback qui DOIT se faire avec le backup.
    FX_Feedback_Ecran_avant_remplissage=FX_Feedback_Ecran;
    FX_Feedback_Ecran=Ecran_backup;

    // On va maintenant "épurer" la zone visible de l'image:
    memset(Table_de_remplacement,0,256);
    Table_de_remplacement[Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y)]=1;
    Remplacer_toutes_les_couleurs_dans_limites(Table_de_remplacement);

    // On fait maintenant un remplissage classique de la couleur 1 avec la 2
   Fill(&Limite_atteinte_Haut  ,&Limite_atteinte_Bas,
         &Limite_atteinte_Gauche,&Limite_atteinte_Droite);

    //  On s'apprête à faire des opérations qui nécessitent un affichage. Il
    // faut donc retirer de l'écran le curseur:
    Effacer_curseur();
    Forme_curseur=Forme_curseur_avant_remplissage;

    //  Il va maintenant falloir qu'on "turn" ce gros caca "into" un truc qui
    // ressemble un peu plus à ce à quoi l'utilisateur peut s'attendre.
    if (Limite_atteinte_Haut>Limite_Haut)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,                    // Source
                                               Limite_Gauche,Limite_Haut,       // Pos X et Y dans source
                                               (Limite_Droite-Limite_Gauche)+1, // width copie
                                               Limite_atteinte_Haut-Limite_Haut,// height copie
                                               Principal_Largeur_image,         // width de la source
                                               Principal_Ecran,                 // Destination
                                               Limite_Gauche,Limite_Haut,       // Pos X et Y destination
                                               Principal_Largeur_image);        // width destination
    if (Limite_atteinte_Bas<Limite_Bas)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_Gauche,Limite_atteinte_Bas+1,
                                               (Limite_Droite-Limite_Gauche)+1,
                                               Limite_Bas-Limite_atteinte_Bas,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_Gauche,Limite_atteinte_Bas+1,Principal_Largeur_image);
    if (Limite_atteinte_Gauche>Limite_Gauche)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_Gauche,Limite_atteinte_Haut,
                                               Limite_atteinte_Gauche-Limite_Gauche,
                                               (Limite_atteinte_Bas-Limite_atteinte_Haut)+1,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_Gauche,Limite_atteinte_Haut,Principal_Largeur_image);
    if (Limite_atteinte_Droite<Limite_Droite)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_atteinte_Droite+1,Limite_atteinte_Haut,
                                               Limite_Droite-Limite_atteinte_Droite,
                                               (Limite_atteinte_Bas-Limite_atteinte_Haut)+1,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_atteinte_Droite+1,Limite_atteinte_Haut,Principal_Largeur_image);

    for (y_pos=Limite_atteinte_Haut;y_pos<=Limite_atteinte_Bas;y_pos++)
      for (x_pos=Limite_atteinte_Gauche;x_pos<=Limite_atteinte_Droite;x_pos++)
        if (Lit_pixel_dans_ecran_courant(x_pos,y_pos)==2)
        {
          //   Si le pixel en cours de traitement a été touché par le Fill()
          // on se doit d'afficher le pixel modifié par la couleur de
          // remplissage:

          //  Ceci se fait en commençant par restaurer la couleur qu'il y avait
          // précédemment (c'est important pour que les effets ne s'emmèlent
          // pas le pinceaux)
          Pixel_dans_ecran_courant(x_pos,y_pos,Lit_pixel_dans_ecran_backup(x_pos,y_pos));

          //  Enfin, on peut afficher le pixel, en le soumettant aux effets en
          // cours:
          Afficher_pixel(x_pos,y_pos,Couleur_de_remplissage);
        }
        else
          Pixel_dans_ecran_courant(x_pos,y_pos,Lit_pixel_dans_ecran_backup(x_pos,y_pos));

    FX_Feedback_Ecran=FX_Feedback_Ecran_avant_remplissage;

    //   A la fin, on n'a pas besoin de réafficher le curseur puisque c'est
    // l'appelant qui s'en charge, et on n'a pas besoin de rafficher l'image
    // puisque les seuls points qui ont changé dans l'image ont été raffichés
    // par l'utilisation de "Afficher_pixel()", et que les autres... eh bein
    // on n'y a jamais touché à l'écran les autres: ils sont donc corrects.

    UpdateRect(0,0,0,0);
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////// TRACéS DE FIGURES GéOMéTRIQUES STANDARDS //////////////////
////////////////////////// avec gestion de previews //////////////////////////
//////////////////////////////////////////////////////////////////////////////


  // Affichage d'un point de façon définitive (utilisation du pinceau)
  void Pixel_figure_Definitif(word x_pos,word y_pos,byte Couleur)
  {
    Afficher_pinceau(x_pos,y_pos,Couleur,0);
  }

  // Affichage d'un point de façon définitive
  void Pixel_clippe(word x_pos,word y_pos,byte Couleur)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
    Afficher_pixel(x_pos,y_pos,Couleur);
  }
  
  // Affichage d'un point pour une preview
  void Pixel_figure_Preview(word x_pos,word y_pos,byte Couleur)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
      Pixel_Preview(x_pos,y_pos,Couleur);
  }
  // Affichage d'un point pour une preview, avec sa propre couleur
  void Pixel_figure_Preview_auto(word x_pos,word y_pos)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
      Pixel_Preview(x_pos,y_pos,Lit_pixel_dans_ecran_courant(x_pos,y_pos));
  }

  // Affichage d'un point pour une preview en xor
  void Pixel_figure_Preview_xor(word x_pos,word y_pos,__attribute__((unused)) byte Couleur)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
      Pixel_Preview(x_pos,y_pos,~Lit_pixel(x_pos-Principal_Decalage_X,
                                           y_pos-Principal_Decalage_Y));
  }
  
  // Affichage d'un point pour une preview en xor additif
  // (Il lit la couleur depuis la page backup)
  void Pixel_figure_Preview_xorback(word x_pos,word y_pos,__attribute__((unused)) byte Couleur)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
      Pixel_Preview(x_pos,y_pos,~Ecran_backup[x_pos+y_pos*Principal_Largeur_image]);
  }
  

  // Effacement d'un point de preview
  void Pixel_figure_Effacer_preview(word x_pos,word y_pos,__attribute__((unused)) byte Couleur)
  {
    if ( (x_pos>=Limite_Gauche) &&
         (x_pos<=Limite_Droite) &&
         (y_pos>=Limite_Haut)   &&
         (y_pos<=Limite_Bas) )
      Pixel_Preview(x_pos,y_pos,Lit_pixel_dans_ecran_courant(x_pos,y_pos));
  }

  // Affichage d'un point dans la brosse
  void Pixel_figure_Dans_brosse(word x_pos,word y_pos,byte Couleur)
  {
    x_pos-=Brosse_Decalage_X;
    y_pos-=Brosse_Decalage_Y;
    if ( (x_pos<Brosse_Largeur) && // Les pos sont des word donc jamais < 0 ...
         (y_pos<Brosse_Hauteur) )
      Pixel_dans_brosse(x_pos,y_pos,Couleur);
  }


  // -- Tracer général d'un cercle vide -------------------------------------

void Tracer_cercle_vide_General(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short x_pos;
  short y_pos;

  // Ensuite, on va parcourire le quart haut gauche du cercle
  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;

  // Affichage des extremitées du cercle sur chaque quart du cercle:
  for (y_pos=Debut_Y,Cercle_Curseur_Y=-Rayon;y_pos<Centre_Y;y_pos++,Cercle_Curseur_Y++)
    for (x_pos=Debut_X,Cercle_Curseur_X=-Rayon;x_pos<Centre_X;x_pos++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
      {
        // On vient de tomber sur le premier point sur la ligne horizontale
        // qui fait partie du cercle.
        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,Couleur);
         // Quart Haut-droite
        Pixel_figure((Centre_X<<1)-x_pos,y_pos,Couleur);
         // Quart Bas-droite
        Pixel_figure((Centre_X<<1)-x_pos,(Centre_Y<<1)-y_pos,Couleur);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(Centre_Y<<1)-y_pos,Couleur);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas au cercle:
        for (Cercle_Curseur_Y--,x_pos++,Cercle_Curseur_X++;x_pos<Centre_X;x_pos++,Cercle_Curseur_X++)
          if (!Pixel_dans_cercle())
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,Couleur);
             // Quart Haut-droite
            Pixel_figure((Centre_X<<1)-x_pos,y_pos,Couleur);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(Centre_Y<<1)-y_pos,Couleur);
             // Quart Bas-droite
            Pixel_figure((Centre_X<<1)-x_pos,(Centre_Y<<1)-y_pos,Couleur);
          }
          else
            break;

        Cercle_Curseur_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:
  Pixel_figure(Centre_X,Centre_Y-Rayon,Couleur); // Haut
  Pixel_figure(Centre_X-Rayon,Centre_Y,Couleur); // Gauche
  Pixel_figure(Centre_X+Rayon,Centre_Y,Couleur); // Droite
  Pixel_figure(Centre_X,Centre_Y+Rayon,Couleur); // Bas

  if(Loupe_Mode) Mettre_Ecran_A_Jour(Centre_X-Rayon,Centre_Y-Rayon,2*Rayon+1,2*Rayon+1);
}

  // -- Tracé définitif d'un cercle vide --

void Tracer_cercle_vide_Definitif(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Tracer la preview d'un cercle vide --

void Tracer_cercle_vide_Preview(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Effacer la preview d'un cercle vide --

void Effacer_cercle_vide_Preview(short Centre_X,short Centre_Y,short Rayon)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,0);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Tracer un cercle plein --

void Tracer_cercle_plein(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short x_pos;
  short y_pos;
  short Fin_X;
  short Fin_Y;

  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;
  Fin_X=Centre_X+Rayon;
  Fin_Y=Centre_Y+Rayon;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage du cercle
  for (y_pos=Debut_Y,Cercle_Curseur_Y=(long)Debut_Y-Centre_Y;y_pos<=Fin_Y;y_pos++,Cercle_Curseur_Y++)
    for (x_pos=Debut_X,Cercle_Curseur_X=(long)Debut_X-Centre_X;x_pos<=Fin_X;x_pos++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
        Afficher_pixel(x_pos,y_pos,Couleur);

  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);
}


  // -- Tracer général d'une ellipse vide -----------------------------------

void Tracer_ellipse_vide_General(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short x_pos;
  short y_pos;

  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // Affichage des extremitées de l'ellipse sur chaque quart de l'ellipse:
  for (y_pos=Debut_Y,Ellipse_Curseur_Y=-Rayon_vertical;y_pos<Centre_Y;y_pos++,Ellipse_Curseur_Y++)
    for (x_pos=Debut_X,Ellipse_Curseur_X=-Rayon_horizontal;x_pos<Centre_X;x_pos++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
      {
        // On vient de tomber sur le premier point qui sur la ligne
        // horizontale fait partie de l'ellipse.

        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,Couleur);
         // Quart Haut-droite
        Pixel_figure((Centre_X<<1)-x_pos,y_pos,Couleur);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(Centre_Y<<1)-y_pos,Couleur);
         // Quart Bas-droite
        Pixel_figure((Centre_X<<1)-x_pos,(Centre_Y<<1)-y_pos,Couleur);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas à l'ellipse:
        for (Ellipse_Curseur_Y--,x_pos++,Ellipse_Curseur_X++;x_pos<Centre_X;x_pos++,Ellipse_Curseur_X++)
          if (!Pixel_dans_ellipse())
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,Couleur);
             // Quart Haut-droite
            Pixel_figure((Centre_X<<1)-x_pos,y_pos,Couleur);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(Centre_Y<<1)-y_pos,Couleur);
             // Quart Bas-droite
            Pixel_figure((Centre_X<<1)-x_pos,(Centre_Y<<1)-y_pos,Couleur);
          }
          else
            break;

        Ellipse_Curseur_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:

  // Points verticaux:
  x_pos=Centre_X;
  Ellipse_Curseur_X=-1;
  for (y_pos=Centre_Y+1-Rayon_vertical,Ellipse_Curseur_Y=-Rayon_vertical+1;y_pos<Centre_Y+Rayon_vertical;y_pos++,Ellipse_Curseur_Y++)
    if (!Pixel_dans_ellipse())
      Pixel_figure(x_pos,y_pos,Couleur);

  // Points horizontaux:
  y_pos=Centre_Y;
  Ellipse_Curseur_Y=-1;
  for (x_pos=Centre_X+1-Rayon_horizontal,Ellipse_Curseur_X=-Rayon_horizontal+1;x_pos<Centre_X+Rayon_horizontal;x_pos++,Ellipse_Curseur_X++)
    if (!Pixel_dans_ellipse())
      Pixel_figure(x_pos,y_pos,Couleur);

  Pixel_figure(Centre_X,Centre_Y-Rayon_vertical,Couleur);   // Haut
  Pixel_figure(Centre_X-Rayon_horizontal,Centre_Y,Couleur); // Gauche
  Pixel_figure(Centre_X+Rayon_horizontal,Centre_Y,Couleur); // Droite
  Pixel_figure(Centre_X,Centre_Y+Rayon_vertical,Couleur);   // Bas

  Mettre_Ecran_A_Jour(Centre_X-Rayon_horizontal,Centre_Y-Rayon_vertical,2*Rayon_horizontal+1,2*Rayon_vertical+1);
}

  // -- Tracé définitif d'une ellipse vide --

void Tracer_ellipse_vide_Definitif(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical+1);
}

  // -- Tracer la preview d'une ellipse vide --

void Tracer_ellipse_vide_Preview(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical +1);
}

  // -- Effacer la preview d'une ellipse vide --

void Effacer_ellipse_vide_Preview(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,0);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical+1);
}

  // -- Tracer une ellipse pleine --

void Tracer_ellipse_pleine(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short x_pos;
  short y_pos;
  short Fin_X;
  short Fin_Y;

  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;
  Fin_X=Centre_X+Rayon_horizontal;
  Fin_Y=Centre_Y+Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage de l'ellipse
  for (y_pos=Debut_Y,Ellipse_Curseur_Y=Debut_Y-Centre_Y;y_pos<=Fin_Y;y_pos++,Ellipse_Curseur_Y++)
    for (x_pos=Debut_X,Ellipse_Curseur_X=Debut_X-Centre_X;x_pos<=Fin_X;x_pos++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
        Afficher_pixel(x_pos,y_pos,Couleur);
  Mettre_Ecran_A_Jour(Centre_X-Rayon_horizontal,Centre_Y-Rayon_vertical,2*Rayon_horizontal+1,2*Rayon_vertical+1);
}

/******************
* TRACÉ DE LIGNES *
******************/

void Rectifier_coordonnees_a_45_degres(short AX, short AY, short* BX, short* BY)
// Modifie BX et BY pour que la ligne AXAY - BXBY soit
//  - une droite horizontale
//  - une droite verticale
//  - une droite avec une pente de 45 degrés
{
    int dx, dy;
    float tan;

    dx = (*BX)-AX;
    dy = AY- *BY; // On prend l'opposée car à l'écran les Y sont positifs en bas, et en maths, positifs en haut

    if (dx==0) return; // On est en lockx et de toutes façons le X n'a pas bougé, on sort tout de suite pour éviter une méchante division par 0

    tan = (float)dy/(float)dx;

    if (tan <= 0.4142 && tan >= -0.4142)
    {
      // Cas 1 : Lock Y
      *BY = AY;
    }
    else if ( tan > 0.4142 && tan < 2.4142)
    {
      // Cas 2 : dy=dx
      *BY = (*BY + AY - dx)/2;
      *BX = AX  + AY - *BY;
    }
    else if (tan < -0.4142 && tan >= -2.4142)
    {
      // Cas 8 : dy = -dx
      *BY = (*BY + AY + dx)/2;
      *BX = AX  - AY + *BY;
    }
    else
    {
      // Cas 3 : Lock X
      *BX = AX;
    }

    return;
}

  // -- Tracer général d'une ligne ------------------------------------------

void Tracer_ligne_General(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y, byte Couleur)
{
  short x_pos,y_pos;
  short Incr_X,Incr_Y;
  short i,Cumul;
  short Delta_X,Delta_Y;


  x_pos=Debut_X;
  y_pos=Debut_Y;

  if (Debut_X<Fin_X)
  {
    Incr_X=+1;
    Delta_X=Fin_X-Debut_X;
  }
  else
  {
    Incr_X=-1;
    Delta_X=Debut_X-Fin_X;
  }

  if (Debut_Y<Fin_Y)
  {
    Incr_Y=+1;
    Delta_Y=Fin_Y-Debut_Y;
  }
  else
  {
    Incr_Y=-1;
    Delta_Y=Debut_Y-Fin_Y;
  }

  if (Delta_Y>Delta_X)
  {
    Cumul=Delta_Y>>1;
    for (i=1; i<Delta_Y; i++)
    {
      y_pos+=Incr_Y;
      Cumul+=Delta_X;
      if (Cumul>=Delta_Y)
      {
        Cumul-=Delta_Y;
        x_pos+=Incr_X;
      }
      Pixel_figure(x_pos,y_pos,Couleur);
    }
  }
  else
  {
    Cumul=Delta_X>>1;
    for (i=1; i<Delta_X; i++)
    {
      x_pos+=Incr_X;
      Cumul+=Delta_Y;
      if (Cumul>=Delta_X)
      {
        Cumul-=Delta_X;
        y_pos+=Incr_Y;
      }
      Pixel_figure(x_pos,y_pos,Couleur);
    }
  }

  if ( (Debut_X!=Fin_X) || (Debut_Y!=Fin_Y) )
    Pixel_figure(Fin_X,Fin_Y,Couleur);

}

  // -- Tracer définitif d'une ligne --

void Tracer_ligne_Definitif(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y, byte Couleur)
{

  int w = Fin_X-Debut_X, h = Fin_Y - Debut_Y;
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  Mettre_Ecran_A_Jour((Debut_X<Fin_X)?Debut_X:Fin_X,(Debut_Y<Fin_Y)?Debut_Y:Fin_Y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne --

void Tracer_ligne_Preview(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  int w = Fin_X-Debut_X, h = Fin_Y - Debut_Y;
  Pixel_figure=Pixel_figure_Preview;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  Mettre_Ecran_A_Jour((Debut_X<Fin_X)?Debut_X:Fin_X,(Debut_Y<Fin_Y)?Debut_Y:Fin_Y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor --

void Tracer_ligne_Preview_xor(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  int w, h;
  Pixel_figure=Pixel_figure_Preview_xor;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  if (Debut_X<0)
    Debut_X=0;
  if (Debut_Y<0)
    Debut_Y=0;
  if (Fin_X<0)
    Fin_X=0;
  if (Fin_Y<0)
    Fin_Y=0;
  w = Fin_X-Debut_X;
  h = Fin_Y-Debut_Y;
  Mettre_Ecran_A_Jour((Debut_X<Fin_X)?Debut_X:Fin_X,(Debut_Y<Fin_Y)?Debut_Y:Fin_Y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor additif --

void Tracer_ligne_Preview_xorback(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  int w = Fin_X-Debut_X, h = Fin_Y - Debut_Y;
  Pixel_figure=Pixel_figure_Preview_xorback;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  Mettre_Ecran_A_Jour((Debut_X<Fin_X)?Debut_X:Fin_X,(Debut_Y<Fin_Y)?Debut_Y:Fin_Y,abs(w)+1,abs(h)+1);
}

  // -- Effacer la preview d'une ligne --

void Effacer_ligne_Preview(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y)
{
  int w = Fin_X-Debut_X, h = Fin_Y - Debut_Y;
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
  Mettre_Ecran_A_Jour((Debut_X<Fin_X)?Debut_X:Fin_X,(Debut_Y<Fin_Y)?Debut_Y:Fin_Y,abs(w)+1,abs(h)+1);
}


  // -- Tracer un rectangle vide --

void Tracer_rectangle_vide(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  short Tempo;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes soient dans le bon sens:
  if (Debut_X>Fin_X)
  {
    Tempo=Debut_X;
    Debut_X=Fin_X;
    Fin_X=Tempo;
  }
  if (Debut_Y>Fin_Y)
  {
    Tempo=Debut_Y;
    Debut_Y=Fin_Y;
    Fin_Y=Tempo;
  }

  // On trace le rectangle:

  for (x_pos=Debut_X;x_pos<=Fin_X;x_pos++)
    Afficher_pinceau(x_pos,Debut_Y,Couleur,0);

  for (y_pos=Debut_Y+1;y_pos<Fin_Y;y_pos++)
  {
    Afficher_pinceau(Debut_X,y_pos,Couleur,0);
    Afficher_pinceau(  Fin_X,y_pos,Couleur,0);
  }

  for (x_pos=Debut_X;x_pos<=Fin_X;x_pos++)
    Afficher_pinceau(x_pos,  Fin_Y,Couleur,0);
#if defined(__macosx__) || defined(__FreeBSD__)
  Mettre_Ecran_A_Jour(Debut_X,Fin_X,Fin_X-Debut_X,Fin_Y-Debut_Y);
#endif
}

  // -- Tracer un rectangle plein --

void Tracer_rectangle_plein(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  short Tempo;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes sont dans le bon sens:
  if (Debut_X>Fin_X)
  {
    Tempo=Debut_X;
    Debut_X=Fin_X;
    Fin_X=Tempo;
  }
  if (Debut_Y>Fin_Y)
  {
    Tempo=Debut_Y;
    Debut_Y=Fin_Y;
    Fin_Y=Tempo;
  }

  // Correction en cas de dépassement des limites de l'image
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;

  // On trace le rectangle:
  for (y_pos=Debut_Y;y_pos<=Fin_Y;y_pos++)
    for (x_pos=Debut_X;x_pos<=Fin_X;x_pos++)
      // Afficher_pixel traite chaque pixel avec tous les effets ! (smear, ...)
      // Donc on ne peut pas otimiser en traçant ligne par ligne avec memset :(
      Afficher_pixel(x_pos,y_pos,Couleur);
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X,Fin_Y-Debut_Y);

}




  // -- Tracer une courbe de Bézier --

void Tracer_courbe_General(short X1, short Y1,
                           short X2, short Y2,
                           short X3, short Y3,
                           short X4, short Y4,
                           byte Couleur)
{
  float delta,t,t2,t3;
  short x,y,Old_X,Old_Y;
  word  i;
  int   CX[4];
  int   CY[4];

  // Calcul des vecteurs de coefficients
  CX[0]= -   X1 + 3*X2 - 3*X3 + X4;
  CX[1]= + 3*X1 - 6*X2 + 3*X3;
  CX[2]= - 3*X1 + 3*X2;
  CX[3]= +   X1;
  CY[0]= -   Y1 + 3*Y2 - 3*Y3 + Y4;
  CY[1]= + 3*Y1 - 6*Y2 + 3*Y3;
  CY[2]= - 3*Y1 + 3*Y2;
  CY[3]= +   Y1;

  // Traçage de la courbe
  Old_X=X1;
  Old_Y=Y1;
  Pixel_figure(Old_X,Old_Y,Couleur);
  delta=0.05; // 1.0/20
  t=0;
  for (i=1; i<=20; i++)
  {
    t=t+delta; t2=t*t; t3=t2*t;
    x=Round(t3*CX[0] + t2*CX[1] + t*CX[2] + CX[3]);
    y=Round(t3*CY[0] + t2*CY[1] + t*CY[2] + CY[3]);
    Tracer_ligne_General(Old_X,Old_Y,x,y,Couleur);
    Old_X=x;
    Old_Y=y;
  }

  x = Min(Min(X1,X2),Min(X3,X4));
  y = Min(Min(Y1,Y2),Min(Y3,Y4));
  Old_X = Max(Max(X1,X2),Max(X3,X4)) - x;
  Old_Y = Max(Max(Y1,Y2),Max(Y3,Y4)) - y;
  Mettre_Ecran_A_Jour(x,y,Old_X+1,Old_Y+1);
}

  // -- Tracer une courbe de Bézier définitivement --

void Tracer_courbe_Definitif(short X1, short Y1,
                             short X2, short Y2,
                             short X3, short Y3,
                             short X4, short Y4,
                             byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}

  // -- Tracer la preview d'une courbe de Bézier --

void Tracer_courbe_Preview(short X1, short Y1,
                           short X2, short Y2,
                           short X3, short Y3,
                           short X4, short Y4,
                           byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}

  // -- Effacer la preview d'une courbe de Bézier --

void Effacer_courbe_Preview(short X1, short Y1,
                            short X2, short Y2,
                            short X3, short Y3,
                            short X4, short Y4,
                            byte Couleur)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}




  // -- Spray : un petit coup de Pschiitt! --

void Aerographe(short Bouton_clicke)
{
  short x_pos,y_pos;
  short Rayon=Spray_Size>>1;
  long  Rayon_au_carre=(long)Rayon*Rayon;
  short Indice,Count;
  byte  Indice_couleur;
  byte  direction;


  Effacer_curseur();

  if (Spray_Mode)
  {
    for (Count=1; Count<=Spray_Mono_flow; Count++)
    {
      x_pos=(rand()%Spray_Size)-Rayon;
      y_pos=(rand()%Spray_Size)-Rayon;
      if ( (x_pos*x_pos)+(y_pos*y_pos) <= Rayon_au_carre )
      {
        x_pos+=Pinceau_X;
        y_pos+=Pinceau_Y;
        if (Bouton_clicke==1)
          Afficher_pinceau(x_pos,y_pos,Fore_color,0);
        else
          Afficher_pinceau(x_pos,y_pos,Back_color,0);
      }
    }
  }
  else
  {
    //   On essaye de se balader dans la table des flux de façon à ce que ce
    // ne soit pas toujours la dernière couleur qui soit affichée en dernier
    // Pour ça, on part d'une couleur au pif dans une direction aléatoire.
    direction=rand()&1;
    for (Indice=0,Indice_couleur=rand()/*%256*/; Indice<256; Indice++)
    {
      for (Count=1; Count<=Spray_Multi_flow[Indice_couleur]; Count++)
      {
        x_pos=(rand()%Spray_Size)-Rayon;
        y_pos=(rand()%Spray_Size)-Rayon;
        if ( (x_pos*x_pos)+(y_pos*y_pos) <= Rayon_au_carre )
        {
          x_pos+=Pinceau_X;
          y_pos+=Pinceau_Y;
          if (Bouton_clicke==A_GAUCHE)
            Afficher_pinceau(x_pos,y_pos,Indice_couleur,0);
          else
            Afficher_pinceau(x_pos,y_pos,Back_color,0);
        }
      }
      if (direction)
        Indice_couleur++;
      else
        Indice_couleur--;
    }
  }

  Afficher_curseur();
}



  //////////////////////////////////////////////////////////////////////////
  ////////////////////////// GESTION DES DEGRADES //////////////////////////
  //////////////////////////////////////////////////////////////////////////


  // -- Gestion d'un dégradé de base (le plus moche) --

void Degrade_de_base(long Indice,short x_pos,short y_pos)
{
  long position;

  // On fait un premier calcul partiel
  position=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard
  position+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  position-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  position/=Degrade_Intervalle_total;

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position<0)
    position=0;
  else if (position>=Degrade_Intervalle_bornes)
    position=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Traiter_pixel_de_degrade(x_pos,y_pos,Degrade_Borne_Superieure-position);
  else
    Traiter_pixel_de_degrade(x_pos,y_pos,Degrade_Borne_Inferieure+position);
}


  // -- Gestion d'un dégradé par trames simples --

void Degrade_de_trames_simples(long Indice,short x_pos,short y_pos)
{
  long Position_dans_degrade;
  long Position_dans_segment;

  //
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "Position_dans_degrade", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  Position_dans_degrade=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard...
  Position_dans_degrade+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  Position_dans_degrade-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;

  // ... qui nous permet de calculer la position dans le segment
  Position_dans_segment=((Position_dans_degrade<<2)/Degrade_Intervalle_total)&3;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  Position_dans_degrade/=Degrade_Intervalle_total;

  // On va pouvoir discuter de la valeur de Position_dans_degrade en fonction
  // de la position dans l'écran et de la Position_dans_segment.

  switch (Position_dans_segment)
  {
    case 0 : // On est sur la gauche du segment
      if (((x_pos+y_pos)&1)==0)
        Position_dans_degrade--;
      break;

      // On n'a pas à traiter les cas 1 et 2 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 3 : // On est sur la droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        Position_dans_degrade++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;
  else if (Position_dans_degrade>=Degrade_Intervalle_bornes)
    Position_dans_degrade=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Position_dans_degrade=Degrade_Borne_Superieure-Position_dans_degrade;
  else
    Position_dans_degrade=Degrade_Borne_Inferieure+Position_dans_degrade;

  Traiter_pixel_de_degrade(x_pos,y_pos,Position_dans_degrade);
}


  // -- Gestion d'un dégradé par trames étendues --

void Degrade_de_trames_etendues(long Indice,short x_pos,short y_pos)
{
  long Position_dans_degrade;
  long Position_dans_segment;

//
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "Position_dans_degrade", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  Position_dans_degrade=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard
  Position_dans_degrade+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  Position_dans_degrade-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;

  // Qui nous permet de calculer la position dans le segment
  Position_dans_segment=((Position_dans_degrade<<3)/Degrade_Intervalle_total)&7;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  Position_dans_degrade/=Degrade_Intervalle_total;

  // On va pouvoir discuter de la valeur de Position_dans_degrade en fonction
  // de la position dans l'écran et de la Position_dans_segment.

  switch (Position_dans_segment)
  {
    case 0 : // On est sur l'extrême gauche du segment
      if (((x_pos+y_pos)&1)==0)
        Position_dans_degrade--;
      break;

    case 1 : // On est sur la gauche du segment
    case 2 : // On est sur la gauche du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)==0))
        Position_dans_degrade--;
      break;

      // On n'a pas à traiter les cas 3 et 4 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 5 : // On est sur la droite du segment
    case 6 : // On est sur la droite du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)!=0))
        Position_dans_degrade++;
      break;

    case 7 : // On est sur l'extreme droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        Position_dans_degrade++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;
  else if (Position_dans_degrade>=Degrade_Intervalle_bornes)
    Position_dans_degrade=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Position_dans_degrade=Degrade_Borne_Superieure-Position_dans_degrade;
  else
    Position_dans_degrade=Degrade_Borne_Inferieure+Position_dans_degrade;

  Traiter_pixel_de_degrade(x_pos,y_pos,Position_dans_degrade);
}



  // -- Tracer un cercle degradé (une sphère) --

void Tracer_cercle_degrade(short Centre_X,short Centre_Y,short Rayon,short Eclairage_X,short Eclairage_Y)
{
  long Debut_X;
  long Debut_Y;
  long x_pos;
  long y_pos;
  long Fin_X;
  long Fin_Y;
  long Distance_X; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long Distance_Y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage

  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;
  Fin_X=Centre_X+Rayon;
  Fin_Y=Centre_Y+Rayon;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  Degrade_Intervalle_total=Cercle_Limite+
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))+
                           (2L*Rayon*sqrt(
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))));

  if (Degrade_Intervalle_total==0)
    Degrade_Intervalle_total=1;

  // Affichage du cercle
  for (y_pos=Debut_Y,Cercle_Curseur_Y=(long)Debut_Y-Centre_Y;y_pos<=Fin_Y;y_pos++,Cercle_Curseur_Y++)
  {
    Distance_Y =(y_pos-Eclairage_Y);
    Distance_Y*=Distance_Y;
    for (x_pos=Debut_X,Cercle_Curseur_X=(long)Debut_X-Centre_X;x_pos<=Fin_X;x_pos++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
      {
        Distance_X =(x_pos-Eclairage_X);
        Distance_X*=Distance_X;
        Traiter_degrade(Distance_X+Distance_Y,x_pos,y_pos);
      }
  }

  Mettre_Ecran_A_Jour(Centre_X-Rayon,Centre_Y-Rayon,2*Rayon+1,2*Rayon+1);
}


  // -- Tracer une ellipse degradée --

void Tracer_ellipse_degradee(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,short Eclairage_X,short Eclairage_Y)
{
  long Debut_X;
  long Debut_Y;
  long x_pos;
  long y_pos;
  long Fin_X;
  long Fin_Y;
  long Distance_X; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long Distance_Y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage


  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;
  Fin_X=Centre_X+Rayon_horizontal;
  Fin_Y=Centre_Y+Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // On calcule la distance maximale:
  Degrade_Intervalle_total=(Rayon_horizontal*Rayon_horizontal)+
                           (Rayon_vertical*Rayon_vertical)+
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))+
                           (2L
                           *sqrt(
                           (Rayon_horizontal*Rayon_horizontal)+
                           (Rayon_vertical  *Rayon_vertical  ))
                           *sqrt(
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))));

  if (Degrade_Intervalle_total==0)
    Degrade_Intervalle_total=1;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage de l'ellipse
  for (y_pos=Debut_Y,Ellipse_Curseur_Y=Debut_Y-Centre_Y;y_pos<=Fin_Y;y_pos++,Ellipse_Curseur_Y++)
  {
    Distance_Y =(y_pos-Eclairage_Y);
    Distance_Y*=Distance_Y;
    for (x_pos=Debut_X,Ellipse_Curseur_X=Debut_X-Centre_X;x_pos<=Fin_X;x_pos++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
      {
        Distance_X =(x_pos-Eclairage_X);
        Distance_X*=Distance_X;
        Traiter_degrade(Distance_X+Distance_Y,x_pos,y_pos);
      }
  }

  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}


// Tracé d'un rectangle (RAX RAY - RBX RBY) dégradé selon le vecteur (VAX VAY - VBX - VBY)
void Tracer_rectangle_degrade(short RAX,short RAY,short RBX,short RBY,short VAX,short VAY, short VBX, short VBY)
{
    short y_pos, x_pos;

    // On commence par s'assurer que le rectangle est à l'endroit
    if(RBX < RAX)
    {
      x_pos = RBX;
      RBX = RAX;
      RAX = x_pos;
    }

    if(RBY < RAY)
    {
      y_pos = RBY;
      RBY = RAY;
      RAY = y_pos;
    }

    // Correction des bornes d'après les limites
    if (RAY<Limite_Haut)
      RAY=Limite_Haut;
    if (RBY>Limite_Bas)
      RBY=Limite_Bas;
    if (RAX<Limite_Gauche)
      RAX=Limite_Gauche;
    if (RBX>Limite_Droite)
      RBX=Limite_Droite;

    if(VBX == VAX)
    {
      // Le vecteur est vertical, donc on évite la partie en dessous qui foirerait avec une division par 0...
      if (VBY == VAY) return;  // L'utilisateur fait n'importe quoi
      Degrade_Intervalle_total = abs(VBY - VAY);
      for(y_pos=RAY;y_pos<=RBY;y_pos++)
        for(x_pos=RAX;x_pos<=RBX;x_pos++)
          Traiter_degrade(abs(VBY - y_pos),x_pos,y_pos);

    }
    else
    {
      float a;
      float b;
      float Distance_X, Distance_Y;

      Degrade_Intervalle_total = sqrt(pow(VBY - VAY,2)+pow(VBX - VAX,2));
      a = (float)(VBY - VAY)/(float)(VBX - VAX);
      b = VAY - a*VAX;
      
      for (y_pos=RAY;y_pos<=RBY;y_pos++)
        for (x_pos = RAX;x_pos<=RBX;x_pos++)
        {
          // On calcule ou on en est dans le dégradé
          Distance_X = pow((y_pos - VAY),2)+pow((x_pos - VAX),2);
          Distance_Y = pow((-a * x_pos + y_pos - b),2)/(a*a+1);
      
          Traiter_degrade((int)sqrt(Distance_X - Distance_Y),x_pos,y_pos);
        }
    }
    Mettre_Ecran_A_Jour(RAX,RAY,RBX,RBY);
}




// -- Tracer un polygône plein --

typedef struct POLYGON_EDGE      /* an active edge */
{
    short top;                     /* top y position */
    short bottom;                  /* bottom y position */
    float x, dx;                   /* floating point x position and gradient */
    float w;                       /* width of line segment */
    struct POLYGON_EDGE *prev;     /* doubly linked list */
    struct POLYGON_EDGE *next;
} POLYGON_EDGE;



/* fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
void fill_edge_structure(POLYGON_EDGE *edge, short *i1, short *i2)
{
  short *it;

  if (i2[1] < i1[1])
  {
    it = i1;
    i1 = i2;
    i2 = it;
  }

  edge->top = i1[1];
  edge->bottom = i2[1] - 1;
  edge->dx = ((float) i2[0] - (float) i1[0]) / ((float) i2[1] - (float) i1[1]);
  edge->x = i1[0] + 0.4999999;
  edge->prev = NULL;
  edge->next = NULL;

  if (edge->dx+1 < 0.0)
    edge->x += edge->dx+1;

  if (edge->dx >= 0.0)
    edge->w = edge->dx;
  else
    edge->w = -(edge->dx);

  if (edge->w-1.0<0.0)
    edge->w = 0.0;
  else
    edge->w = edge->w-1;
}



/* add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
POLYGON_EDGE * add_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge, int sort_by_x)
{
  POLYGON_EDGE *pos = list;
  POLYGON_EDGE *prev = NULL;

  if (sort_by_x)
  {
    while ( (pos) && ((pos->x+((pos->w+pos->dx)/2)) < (edge->x+((edge->w+edge->dx)/2))) )
    {
      prev = pos;
      pos = pos->next;
    }
  }
  else
  {
    while ((pos) && (pos->top < edge->top))
    {
      prev = pos;
      pos = pos->next;
    }
  }

  edge->next = pos;
  edge->prev = prev;

  if (pos)
    pos->prev = edge;

  if (prev)
  {
    prev->next = edge;
    return list;
  }
  else
    return edge;
}



/* remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
POLYGON_EDGE * remove_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge)
{
  if (edge->next)
    edge->next->prev = edge->prev;

  if (edge->prev)
  {
    edge->prev->next = edge->next;
    return list;
  }
  else
    return edge->next;
}



/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the
 *  number of vertices, then an array containing a series of x, y points
 *  (a total of vertices*2 values).
 */
void Polyfill_General(int Vertices, short * Points, int color)
{
  short c;
  short top = 0x7FFF;
  short bottom = 0;
  short *i1, *i2;
  short x_pos,Fin_X;
  POLYGON_EDGE *edge, *next_edge, *initial_edge;
  POLYGON_EDGE *active_edges = NULL;
  POLYGON_EDGE *inactive_edges = NULL;

  /* allocate some space and fill the edge table */
  initial_edge=edge=(POLYGON_EDGE *) malloc(sizeof(POLYGON_EDGE) * Vertices);

  i1 = Points;
  i2 = Points + ((Vertices-1)<<1);

  for (c=0; c<Vertices; c++)
  {
    if (i1[1] != i2[1])
    {
      fill_edge_structure(edge, i1, i2);

      if (edge->bottom >= edge->top)
      {
        if (edge->top < top)
          top = edge->top;

        if (edge->bottom > bottom)
          bottom = edge->bottom;

        inactive_edges = add_edge(inactive_edges, edge, 0);
        edge++;
      }
    }
    i2 = i1;
    i1 += 2;
  }

  /* for each scanline in the polygon... */
  for (c=top; c<=bottom; c++)
  {
    /* check for newly active edges */
    edge = inactive_edges;
    while ((edge) && (edge->top == c))
    {
      next_edge = edge->next;
      inactive_edges = remove_edge(inactive_edges, edge);
      active_edges = add_edge(active_edges, edge, 1);
      edge = next_edge;
    }

    /* draw horizontal line segments */
    if ((c>=Limite_Haut) && (c<=Limite_Bas))
    {
      edge = active_edges;
      while ((edge) && (edge->next))
      {
        x_pos=/*Round*/(edge->x);
        Fin_X=/*Round*/(edge->next->x+edge->next->w);
        if (x_pos<Limite_Gauche)
          x_pos=Limite_Gauche;
        if (Fin_X>Limite_Droite)
          Fin_X=Limite_Droite;
        for (; x_pos<=Fin_X; x_pos++)
          Pixel_figure(x_pos,c,color);
        edge = edge->next->next;
      }
    }

    /* update edges, sorting and removing dead ones */
    edge = active_edges;
    while (edge)
    {
      next_edge = edge->next;
      if (c >= edge->bottom)
        active_edges = remove_edge(active_edges, edge);
      else
      {
        edge->x += edge->dx;
        while ((edge->prev) && ( (edge->x+(edge->w/2)) < (edge->prev->x+(edge->prev->w/2))) )
        {
          if (edge->next)
            edge->next->prev = edge->prev;
          edge->prev->next = edge->next;
          edge->next = edge->prev;
          edge->prev = edge->prev->prev;
          edge->next->prev = edge;
          if (edge->prev)
            edge->prev->next = edge;
          else
            active_edges = edge;
        }
      }
      edge = next_edge;
    }
  }

  free(initial_edge);

  // On ne connait pas simplement les xmin et xmax ici, mais de toutes façon ce n'est pas utilisé en preview
  Mettre_Ecran_A_Jour(0,top,Principal_Largeur_image,bottom-top+1);
}


void Polyfill(int Vertices, short * Points, int color)
{
  int Indice;
  byte *FX_Feedback_Ecran_avant_remplissage;

  Pixel_clippe(Points[0],Points[1],color);
  if (Vertices==1)
  {
    Mettre_Ecran_A_Jour(Points[0],Points[1],1,1);
    return;
  }

  // Comme pour le Fill, cette operation fait un peu d'"overdraw"
  // (pixels dessinés plus d'une fois) alors on force le FX Feedback à OFF
  FX_Feedback_Ecran_avant_remplissage=FX_Feedback_Ecran;
  FX_Feedback_Ecran=Ecran_backup;

  Pixel_figure=Pixel_clippe;    
  Polyfill_General(Vertices,Points,color);

  // Remarque: pour dessiner la bordure avec la brosse en cours au lieu
  // d'un pixel de couleur premier-plan, il suffit de mettre ici:
  // Pixel_figure=Pixel_figure_Definitif;

  // Dessin du contour
  for (Indice=0; Indice<Vertices-1;Indice+=1)
    Tracer_ligne_General(Points[Indice*2],Points[Indice*2+1],Points[Indice*2+2],Points[Indice*2+3],color);
  Tracer_ligne_General(Points[0],Points[1],Points[Indice*2],Points[Indice*2+1],color);

  // restore de l'etat du FX Feedback  
  FX_Feedback_Ecran=FX_Feedback_Ecran_avant_remplissage;

}



//------------ Remplacement de la couleur pointée par une autre --------------

void Remplacer(byte Nouvelle_couleur)
{
  byte Ancienne_couleur;

  if ((Pinceau_X<Principal_Largeur_image)
   && (Pinceau_Y<Principal_Hauteur_image))
  {
    Ancienne_couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
    if ( (Ancienne_couleur!=Nouvelle_couleur)
      && ((!Stencil_Mode) || (!Stencil[Ancienne_couleur])) )
    {
      Remplacer_une_couleur(Ancienne_couleur,Nouvelle_couleur);
      Afficher_ecran();
    }
  }
}



/******************************************************************************/
/********************************** SHADES ************************************/

// Transformer une liste de shade en deux tables de conversion
void Liste2tables(word * list,short step,byte mode,byte * Table_inc,byte * Table_dec)
{
  int Indice;
  int first;
  int last;
  int Couleur;
  int Temp;


  // On initialise les deux tables de conversion en Identité
  for (Indice=0;Indice<256;Indice++)
  {
    Table_inc[Indice]=Indice;
    Table_dec[Indice]=Indice;
  }

  // On s'apprête à examiner l'ensemble de la liste
  for (Indice=0;Indice<512;Indice++)
  {
    // On recherche la première case de la liste non vide (et non inhibée)
    while ((Indice<512) && (list[Indice]>255))
      Indice++;

    // On note la position de la première case de la séquence
    first=Indice;

    // On recherche la position de la dernière case de la séquence
    for (last=first;list[last+1]<256;last++);

    // Pour toutes les cases non vides (et non inhibées) qui suivent
    switch (mode)
    {
      case MODE_SHADE_NORMAL :
        for (;(Indice<512) && (list[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=list[Indice];
          Table_inc[Couleur]=list[(Indice+step<=last)?Indice+step:last];
          Table_dec[Couleur]=list[(Indice-step>=first)?Indice-step:first];
        }
        break;
      case MODE_SHADE_BOUCLE :
        Temp=1+last-first;
        for (;(Indice<512) && (list[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=list[Indice];
          Table_inc[Couleur]=list[first+((step+Indice-first)%Temp)];
          Table_dec[Couleur]=list[first+(((Temp-step)+Indice-first)%Temp)];
        }
        break;
      default : // MODE_SHADE_NOSAT
        for (;(Indice<512) && (list[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=list[Indice];
          if (Indice+step<=last)
            Table_inc[Couleur]=list[Indice+step];
          if (Indice-step>=first)
            Table_dec[Couleur]=list[Indice-step];
        }
    }
  }
}



// -- Interface avec l'image, affectée par le facteur de grossissement -------

  // fonction d'affichage "Pixel" utilisée pour les opérations définitivement
  // Ne doit à aucune condition être appelée en dehors de la partie visible
  // de l'image dans l'écran (ça pourrait être grave)
void Afficher_pixel(word x,word y,byte Couleur)
  // x & y    sont la position d'un point dans l'IMAGE
  // Couleur  est la couleur du point
  // Le Stencil est géré.
  // Les effets sont gérés par appel à Fonction_effet().
  // La Loupe est gérée par appel à Pixel_Preview().
{
  if ( ( (!Trame_Mode)   || (Effet_Trame(x,y)) )
    && (!((Stencil_Mode) && (Stencil[Lit_pixel_dans_ecran_courant(x,y)])))
    && (!((Mask_Mode)    && (Mask_table[Lit_pixel_dans_ecran_brouillon(x,y)]))) )
  {
    Couleur=Fonction_effet(x,y,Couleur);
    Pixel_dans_ecran_courant(x,y,Couleur);
    Pixel_Preview(x,y,Couleur);
  }
}



// -- Calcul des différents effets -------------------------------------------

  // -- Aucun effet en cours --

byte Aucun_effet(__attribute__((unused)) word x,__attribute__((unused)) word y,byte Couleur)
{
  return Couleur;
}

  // -- Effet de Shading --

byte Effet_Shade(word x,word y,__attribute__((unused)) byte Couleur)
{
  return Shade_Table[Lit_pixel_dans_ecran_feedback(x,y)];
}

byte Effet_Quick_shade(word x,word y,byte Couleur)
{
  int c=Couleur=Lit_pixel_dans_ecran_feedback(x,y);
  int direction=(Fore_color<=Back_color);
  byte start,end;
  int width;

  if (direction)
  {
    start=Fore_color;
    end  =Back_color;
  }
  else
  {
    start=Back_color;
    end  =Fore_color;
  }

  if ((c>=start) && (c<=end) && (start!=end))
  {
    width=1+end-start;

    if ( ((Shade_Table==Shade_Table_gauche) && direction) || ((Shade_Table==Shade_Table_droite) && (!direction)) )
      c-=Quick_shade_Step%width;
    else
      c+=Quick_shade_Step%width;

    if (c<start)
      switch (Quick_shade_Loop)
      {
        case MODE_SHADE_NORMAL : return start;
        case MODE_SHADE_BOUCLE : return (width+c);
        default : return Couleur;
      }

    if (c>end)
      switch (Quick_shade_Loop)
      {
        case MODE_SHADE_NORMAL : return end;
        case MODE_SHADE_BOUCLE : return (c-width);
        default : return Couleur;
      }
  }

  return c;
}

  // -- Effet de Tiling --

byte Effet_Tiling(word x,word y,__attribute__((unused)) byte Couleur)
{
  return Lit_pixel_dans_brosse((x+Brosse_Largeur-Tiling_Decalage_X)%Brosse_Largeur,
                               (y+Brosse_Hauteur-Tiling_Decalage_Y)%Brosse_Hauteur);
}

  // -- Effet de Smooth --

byte Effet_Smooth(word x,word y,__attribute__((unused)) byte Couleur)
{
  int r,g,b;
  byte c;
  int Poids,Poids_total;
  byte X2=((x+1)<Principal_Largeur_image);
  byte Y2=((y+1)<Principal_Hauteur_image);

  // On commence par le pixel central
  c=Lit_pixel_dans_ecran_feedback(x,y);
  Poids_total=Smooth_Matrice[1][1];
  r=Poids_total*Principal_Palette[c].R;
  g=Poids_total*Principal_Palette[c].G;
  b=Poids_total*Principal_Palette[c].B;

  if (x)
  {
    c=Lit_pixel_dans_ecran_feedback(x-1,y);
    Poids_total+=(Poids=Smooth_Matrice[0][1]);
    r+=Poids*Principal_Palette[c].R;
    g+=Poids*Principal_Palette[c].G;
    b+=Poids*Principal_Palette[c].B;

    if (y)
    {
      c=Lit_pixel_dans_ecran_feedback(x-1,y-1);
      Poids_total+=(Poids=Smooth_Matrice[0][0]);
      r+=Poids*Principal_Palette[c].R;
      g+=Poids*Principal_Palette[c].G;
      b+=Poids*Principal_Palette[c].B;

      if (Y2)
      {
        c=Lit_pixel_dans_ecran_feedback(x-1,y+1);
        Poids_total+=(Poids=Smooth_Matrice[0][2]);
        r+=Poids*Principal_Palette[c].R;
        g+=Poids*Principal_Palette[c].G;
        b+=Poids*Principal_Palette[c].B;
      }
    }
  }

  if (X2)
  {
    c=Lit_pixel_dans_ecran_feedback(x+1,y);
    Poids_total+=(Poids=Smooth_Matrice[2][1]);
    r+=Poids*Principal_Palette[c].R;
    g+=Poids*Principal_Palette[c].G;
    b+=Poids*Principal_Palette[c].B;

    if (y)
    {
      c=Lit_pixel_dans_ecran_feedback(x+1,y-1);
      Poids_total+=(Poids=Smooth_Matrice[2][0]);
      r+=Poids*Principal_Palette[c].R;
      g+=Poids*Principal_Palette[c].G;
      b+=Poids*Principal_Palette[c].B;

      if (Y2)
      {
        c=Lit_pixel_dans_ecran_feedback(x+1,y+1);
        Poids_total+=(Poids=Smooth_Matrice[2][2]);
        r+=Poids*Principal_Palette[c].R;
        g+=Poids*Principal_Palette[c].G;
        b+=Poids*Principal_Palette[c].B;
      }
    }
  }

  if (y)
  {
    c=Lit_pixel_dans_ecran_feedback(x,y-1);
    Poids_total+=(Poids=Smooth_Matrice[1][0]);
    r+=Poids*Principal_Palette[c].R;
    g+=Poids*Principal_Palette[c].G;
    b+=Poids*Principal_Palette[c].B;
  }

  if (Y2)
  {
    c=Lit_pixel_dans_ecran_feedback(x,y+1);
    Poids_total+=(Poids=Smooth_Matrice[1][2]);
    r+=Poids*Principal_Palette[c].R;
    g+=Poids*Principal_Palette[c].G;
    b+=Poids*Principal_Palette[c].B;
  }

  return (Poids_total)? // On regarde s'il faut éviter le 0/0.
    Meilleure_couleur(Round_div(r,Poids_total),
                      Round_div(g,Poids_total),
                      Round_div(b,Poids_total)):
    Lit_pixel_dans_ecran_courant(x,y); // C'est bien l'écran courant et pas
                                       // l'écran feedback car il s'agit de ne
}                                      // pas modifier l'écran courant.
