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

    Brush manipulation functions
*/

#include <math.h>
#include <stdlib.h>
#include <string.h> // memset()

#include "global.h"
#include "graph.h"
#include "divers.h"
#include "erreurs.h"
#include "windows.h"
#include "sdlscreen.h"

// Calcul de redimensionnement du pinceau pour éviter les débordements de
// l'écran et de l'image
void Calculer_dimensions_clipees(short * X,short * Y,short * width,short * height)
{
  if ((*X)<Limite_Gauche)
  {
    (*width)-=(Limite_Gauche-(*X));
    (*X)=Limite_Gauche;
  }

  if (((*X)+(*width))>(Limite_Droite+1))
  {
    (*width)=(Limite_Droite-(*X))+1;
  }

  if ((*Y)<Limite_Haut)
  {
    (*height)-=(Limite_Haut-(*Y));
    (*Y)=Limite_Haut;
  }

  if (((*Y)+(*height))>(Limite_Bas+1))
  {
    (*height)=(Limite_Bas-(*Y))+1;
  }
}

  // -- Calcul de redimensionnement du pinceau pour éviter les débordements
  //    de l'écran zoomé et de l'image --

void Calculer_dimensions_clipees_zoom(short * X,short * Y,short * width,short * height)
{
  if ((*X)<Limite_Gauche_Zoom)
  {
    (*width)-=(Limite_Gauche_Zoom-(*X));
    (*X)=Limite_Gauche_Zoom;
  }

  if (((*X)+(*width))>(Limite_Droite_Zoom+1))
  {
    (*width)=(Limite_Droite_Zoom-(*X))+1;
  }

  if ((*Y)<Limite_Haut_Zoom)
  {
    (*height)-=(Limite_Haut_Zoom-(*Y));
    (*Y)=Limite_Haut_Zoom;
  }

  if (((*Y)+(*height))>(Limite_Bas_Zoom+1))
  {
    (*height)=(Limite_Bas_Zoom-(*Y))+1;
  }
}


  // -- Afficher le pinceau (de façon définitive ou non) --

void Afficher_pinceau(short X,short Y,byte Couleur,byte is_preview)
  // X,Y: position du centre du pinceau
  // Couleur: couleur à appliquer au pinceau
  // is_preview: "Il ne faut l'afficher qu'à l'écran"
{
  short Debut_X; // Position X (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short Debut_Y; // Position Y (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short width; // width dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short height; // height dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short Debut_Compteur_X; // Position X (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short Debut_Compteur_Y; // Position Y (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short x_pos; // Position X (dans l'image) en cours d'affichage
  short y_pos; // Position Y (dans l'image) en cours d'affichage
  short Compteur_X; // Position X (dans la brosse/pinceau) en cours
        // d'affichage
  short Compteur_Y; // Position Y (dans la brosse/pinceau) en cours
        // d'affichage
  short Fin_Compteur_X; // Position X ou s'arrête l'affichade de la
        // brosse/pinceau
  short Fin_Compteur_Y; // Position Y ou s'arrête l'affichade de la
        // brosse/pinceau
  byte  Couleur_temporaire; // Couleur de la brosse en cours d'affichage
  int position;
  byte * Temp;

  if (!(is_preview && Mouse_K)) // Si bouton enfoncé & preview > pas de dessin
  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_POINT : // !!! TOUJOURS EN PREVIEW !!!
      if ( (Pinceau_X>=Limite_Gauche)
        && (Pinceau_X<=Limite_Droite)
        && (Pinceau_Y>=Limite_Haut)
        && (Pinceau_Y<=Limite_Bas) )
        {
                Pixel_Preview(Pinceau_X,Pinceau_Y,Couleur);
                Mettre_Ecran_A_Jour(X,Y,1,1);
        }
      break;

    case FORME_PINCEAU_BROSSE_COULEUR : // Brosse en couleur

      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      width=Brosse_Largeur;
      height=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&width,&height);
      if (width<=0 || height<=0)
        break;
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+width;
      Fin_Compteur_Y=Debut_Compteur_Y+height;

      if (is_preview)
      {
        if ( (width>0) && (height>0) )
          Display_brush_Color(
                Debut_X-Principal_Decalage_X,
                Debut_Y-Principal_Decalage_Y,
                Debut_Compteur_X,
                Debut_Compteur_Y,
                width,
                height,
                Back_color,
                Brosse_Largeur
          );

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&width,
                &height
          );

          Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            height=Debut_Y+(height*Loupe_Facteur);
            if (height>Menu_Ordonnee)
              height=Menu_Ordonnee;

            Display_brush_Color_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                     Debut_Compteur_X,Debut_Compteur_Y,
                                     width,height,Back_color,
                                     Brosse_Largeur,
                                     Buffer_de_ligne_horizontale);
          }
        }

        Mettre_Ecran_A_Jour(X-Brosse_Decalage_X,Y-Brosse_Decalage_Y,Brosse_Largeur,Brosse_Hauteur);

      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((width>0) && (height>0))
            {
              Copier_une_partie_d_image_dans_une_autre(
                Principal_Ecran, Debut_X, Debut_Y, width, height,
                Principal_Largeur_image, Smear_Brosse,
                Debut_Compteur_X, Debut_Compteur_Y,
                Smear_Brosse_Largeur
              );

              Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
            }
            Smear_Debut=0;
          }
          else
          {
            for (y_pos = Debut_Y, Compteur_Y = Debut_Compteur_Y;
                Compteur_Y < Fin_Compteur_Y;
                y_pos++, Compteur_Y++
            )
              for (x_pos = Debut_X, Compteur_X = Debut_Compteur_X;
                Compteur_X < Fin_Compteur_X;
                x_pos++, Compteur_X++
              )
              {
                Couleur_temporaire = Lit_pixel_dans_ecran_courant(
                        x_pos,y_pos
                );
                position = (Compteur_Y * Smear_Brosse_Largeur)+ Compteur_X;
                if ( (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y) != Back_color)
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                        Afficher_pixel(x_pos,y_pos,Smear_Brosse[position]);
                Smear_Brosse[position]=Couleur_temporaire;
              }

              Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
          }

          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          if (Shade_Table==Shade_Table_gauche)
            for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
              for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_brosse(Compteur_X,Compteur_Y);
                if (Couleur_temporaire!=Back_color)
                  Afficher_pixel(x_pos,y_pos,Couleur_temporaire);
              }
          else
            for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
              for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
              {
                if (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                  Afficher_pixel(x_pos,y_pos,Couleur);
              }
        }
        Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);

      }
      break;
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      width=Brosse_Largeur;
      height=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&width,&height);
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+width;
      Fin_Compteur_Y=Debut_Compteur_Y+height;
      if (is_preview)
      {
        if ( (width>0) && (height>0) )
          Display_brush_Mono(Debut_X-Principal_Decalage_X,
                             Debut_Y-Principal_Decalage_Y,
                             Debut_Compteur_X,Debut_Compteur_Y,
                             width,height,
                             Back_color,Fore_color,
                             Brosse_Largeur);

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&width,&height);
          Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            height=Debut_Y+(height*Loupe_Facteur);
            if (height>Menu_Ordonnee)
              height=Menu_Ordonnee;

            Display_brush_Mono_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                    Debut_Compteur_X,Debut_Compteur_Y,
                                    width,height,
                                    Back_color,Fore_color,
                                    Brosse_Largeur,
                                    Buffer_de_ligne_horizontale);

          }
        }

        Mettre_Ecran_A_Jour(X-Brosse_Decalage_X,Y-Brosse_Decalage_Y,Brosse_Largeur,Brosse_Hauteur);
      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((width>0) && (height>0))
            {
              Copier_une_partie_d_image_dans_une_autre(Principal_Ecran,
                                                       Debut_X,Debut_Y,
                                                       width,height,
                                                       Principal_Largeur_image,
                                                       Smear_Brosse,
                                                       Debut_Compteur_X,
                                                       Debut_Compteur_Y,
                                                       Smear_Brosse_Largeur);
              Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
            }
            Smear_Debut=0;
          }
          else
          {
            for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
              for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_ecran_courant(x_pos,y_pos);
                position=(Compteur_Y*Smear_Brosse_Largeur)+Compteur_X;
                if ( (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                  Afficher_pixel(x_pos,y_pos,Smear_Brosse[position]);
                Smear_Brosse[position]=Couleur_temporaire;
              }

            Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);

          }

          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
            for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
            {
              if (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                Afficher_pixel(x_pos,y_pos,Couleur);
            }
          Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
        }
      }
      break;
    default : // Pinceau
      Debut_X=X-Pinceau_Decalage_X;
      Debut_Y=Y-Pinceau_Decalage_Y;
      width=Pinceau_Largeur;
      height=Pinceau_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&width,&height);
      Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+width;
      Fin_Compteur_Y=Debut_Compteur_Y+height;
      if (is_preview)
      {
        Temp=Brosse;
        Brosse=Pinceau_Sprite;

        if ( (width>0) && (height>0) )
          Display_brush_Mono(Debut_X-Principal_Decalage_X,
                             Debut_Y-Principal_Decalage_Y,
                             Debut_Compteur_X,Debut_Compteur_Y,
                             width,height,
                             0,Fore_color,
                             TAILLE_MAXI_PINCEAU);

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&width,&height);
          Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            height=Debut_Y+(height*Loupe_Facteur);
            if (height>Menu_Ordonnee)
              height=Menu_Ordonnee;

            Display_brush_Mono_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                    Debut_Compteur_X,Debut_Compteur_Y,
                                    width,height,
                                    0,Fore_color,
                                    TAILLE_MAXI_PINCEAU,
                                    Buffer_de_ligne_horizontale);

          }
        }

        Brosse=Temp;
      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((width>0) && (height>0))
            {
              Copier_une_partie_d_image_dans_une_autre(Principal_Ecran,
                                                       Debut_X,Debut_Y,
                                                       width,height,
                                                       Principal_Largeur_image,
                                                       Smear_Brosse,
                                                       Debut_Compteur_X,
                                                       Debut_Compteur_Y,
                                                       Smear_Brosse_Largeur);
              Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
            }
            Smear_Debut=0;
          }
          else
          {
            for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
              for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_ecran_courant(x_pos,y_pos);
                position=(Compteur_Y*Smear_Brosse_Largeur)+Compteur_X;
                if ( (Pinceau_Sprite[(TAILLE_MAXI_PINCEAU*Compteur_Y)+Compteur_X]) // Le pinceau sert de masque pour dire quels pixels on doit traiter dans le rectangle
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)          // On clippe l'effet smear entre Smear_Min et Smear_Max
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                  Afficher_pixel(x_pos,y_pos,Smear_Brosse[position]);
                Smear_Brosse[position]=Couleur_temporaire;
              }
              Mettre_Ecran_A_Jour(Debut_X, Debut_Y, width, height);
          }


          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          for (y_pos=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;y_pos++,Compteur_Y++)
            for (x_pos=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;x_pos++,Compteur_X++)
            {
              if (Pinceau_Sprite[(TAILLE_MAXI_PINCEAU*Compteur_Y)+Compteur_X])
                Afficher_pixel(x_pos,y_pos,Couleur);
            }
          Mettre_Ecran_A_Jour(Debut_X,Debut_Y,width,height);
        }
      }
  }
}

// -- Effacer le pinceau -- //
//
void Effacer_pinceau(short X,short Y)
  // X,Y: position du centre du pinceau
{
  short Debut_X; // Position X (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short Debut_Y; // Position Y (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short width; // width dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short height; // height dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short Debut_Compteur_X; // Position X (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short Debut_Compteur_Y; // Position Y (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  //short x_pos; // Position X (dans l'image) en cours d'affichage
  //short y_pos; // Position Y (dans l'image) en cours d'affichage
  //short Compteur_X; // Position X (dans la brosse/pinceau) en cours
        //d'affichage
  //short Compteur_Y; // Position Y (dans la brosse/pinceau) en cours d'affichage
  short Fin_Compteur_X; // Position X ou s'arrête l'affichade de la brosse/pinceau
  short Fin_Compteur_Y; // Position Y ou s'arrête l'affichade de la brosse/pinceau
  byte * Temp;

  if (!Mouse_K)
  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_POINT :
      if ( (Pinceau_X>=Limite_Gauche)
        && (Pinceau_X<=Limite_Droite)
        && (Pinceau_Y>=Limite_Haut)
        && (Pinceau_Y<=Limite_Bas) )
      {
        Pixel_Preview(Pinceau_X,Pinceau_Y,Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y));
        Mettre_Ecran_A_Jour(Pinceau_X,Pinceau_Y,1,1);
      }
      break;
    case FORME_PINCEAU_BROSSE_COULEUR :    // Brosse en couleur
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      width=Brosse_Largeur;
      height=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&width,&height);
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+width;
      Fin_Compteur_Y=Debut_Compteur_Y+height;

      if ( (width>0) && (height>0) )
        Clear_brush(Debut_X-Principal_Decalage_X,
                    Debut_Y-Principal_Decalage_Y,
                    Debut_Compteur_X,Debut_Compteur_Y,
                    width,height,Back_color,
                    Principal_Largeur_image);

      if (Loupe_Mode)
      {
        Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&width,&height);
        Debut_Compteur_X=Debut_X;
        Debut_Compteur_Y=Debut_Y;

        if ( (width>0) && (height>0) )
        {
          // Corrections dues au Zoom:
          Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
          Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
          height=Debut_Y+(height*Loupe_Facteur);
          if (height>Menu_Ordonnee)
            height=Menu_Ordonnee;

          Clear_brush_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                           Debut_Compteur_X,Debut_Compteur_Y,
                           width,height,Back_color,
                           Principal_Largeur_image,
                           Buffer_de_ligne_horizontale);
        }
      }
      break;
    default: // Pinceau
      Debut_X=X-Pinceau_Decalage_X;
      Debut_Y=Y-Pinceau_Decalage_Y;
      width=Pinceau_Largeur;
      height=Pinceau_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&width,&height);
      Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+width;
      Fin_Compteur_Y=Debut_Compteur_Y+height;

      Temp=Brosse;
      Brosse=Pinceau_Sprite;

      if ( (width>0) && (height>0) )
      {
        Clear_brush(Debut_X-Principal_Decalage_X,
                    Debut_Y-Principal_Decalage_Y,
                    Debut_Compteur_X,Debut_Compteur_Y,
                    width,height,0,
                    Principal_Largeur_image);
      }

      if (Loupe_Mode)
      {
        Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&width,&height);
        Debut_Compteur_X=Debut_X;
        Debut_Compteur_Y=Debut_Y;

        if ( (width>0) && (height>0) )
        {
          // Corrections dues au Zoom:
          Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
          Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
          height=Debut_Y+(height*Loupe_Facteur);
          if (height>Menu_Ordonnee)
            height=Menu_Ordonnee;

          Clear_brush_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                           Debut_Compteur_X,Debut_Compteur_Y,
                           width,height,0,
                           Principal_Largeur_image,
                           Buffer_de_ligne_horizontale);
        }
      }

      Brosse=Temp;
      break;
  }
}



void Capturer_brosse(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,short clear)
{
  short Temporaire;
  short x_pos;
  short y_pos;
  word  Nouvelle_Brosse_Largeur;
  word  Nouvelle_Brosse_Hauteur;


  // On commence par "redresser" les bornes:
  if (Debut_X>Fin_X)
  {
    Temporaire=Debut_X;
    Debut_X   =Fin_X;
    Fin_X     =Temporaire;
  }
  if (Debut_Y>Fin_Y)
  {
    Temporaire=Debut_Y;
    Debut_Y   =Fin_Y;
    Fin_Y     =Temporaire;
  }

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((Debut_X<Principal_Largeur_image) && (Debut_Y<Principal_Hauteur_image))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_Decalage_X=0;
    Tiling_Decalage_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    Nouvelle_Brosse_Largeur=(Fin_X-Debut_X)+1;
    Nouvelle_Brosse_Hauteur=(Fin_Y-Debut_Y)+1;

    if (Debut_X+Nouvelle_Brosse_Largeur>Principal_Largeur_image)
      Nouvelle_Brosse_Largeur=Principal_Largeur_image-Debut_X;
    if (Debut_Y+Nouvelle_Brosse_Hauteur>Principal_Hauteur_image)
      Nouvelle_Brosse_Hauteur=Principal_Hauteur_image-Debut_Y;

    if ( (((long)Brosse_Hauteur)*Brosse_Largeur) !=
         (((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur) )
    {
      free(Brosse);
      Brosse=(byte *)malloc(((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur);
      if (!Brosse)
      {
        Erreur(0);

        Brosse=(byte *)malloc(1*1);
        Nouvelle_Brosse_Hauteur=Nouvelle_Brosse_Largeur=1;
        *Brosse=Fore_color;
      }
    }
    Brosse_Largeur=Nouvelle_Brosse_Largeur;
    Brosse_Hauteur=Nouvelle_Brosse_Hauteur;

    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Copier_image_dans_brosse(Debut_X,Debut_Y,Brosse_Largeur,Brosse_Hauteur,Principal_Largeur_image);

    // On regarde s'il faut effacer quelque chose:
    if (clear)
    {
      for (y_pos=Debut_Y;y_pos<Debut_Y+Brosse_Hauteur;y_pos++)
        for (x_pos=Debut_X;x_pos<Debut_X+Brosse_Largeur;x_pos++)
        {
          Pixel_dans_ecran_courant(x_pos,y_pos,Back_color);
          Pixel_Preview           (x_pos,y_pos,Back_color);
        }
      Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Brosse_Largeur,Brosse_Hauteur);
    }

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
}


void Rotate_90_deg()
{
  short Temporaire;
  byte * Nouvelle_Brosse;

  Nouvelle_Brosse=(byte *)malloc(((long)Brosse_Hauteur)*Brosse_Largeur);
  if (Nouvelle_Brosse)
  {
    Rotate_90_deg_LOWLEVEL(Brosse,Nouvelle_Brosse);
    free(Brosse);
    Brosse=Nouvelle_Brosse;

    Temporaire=Brosse_Largeur;
    Brosse_Largeur=Brosse_Hauteur;
    Brosse_Hauteur=Temporaire;

    Temporaire=Smear_Brosse_Largeur;
    Smear_Brosse_Largeur=Smear_Brosse_Hauteur;
    Smear_Brosse_Hauteur=Temporaire;

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
    Erreur(0);
}


void Remap_brosse(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;


  // On commence par initialiser le tableau de booléens à faux
  for (Couleur=0;Couleur<=255;Couleur++)
    Utilisee[Couleur]=0;

  // On calcule la table d'utilisation des couleurs
  for (y_pos=0;y_pos<Brosse_Hauteur;y_pos++)
    for (x_pos=0;x_pos<Brosse_Largeur;x_pos++)
      Utilisee[Lit_pixel_dans_brosse(x_pos,y_pos)]=1;

  //  On n'est pas censé remapper la couleur de transparence, sinon la brosse
  // changera de forme, donc on dit pour l'instant qu'elle n'est pas utilisée
  // ainsi on ne s'embêtera pas à la recalculer
  Utilisee[Back_color]=0;

  //   On va maintenant se servir de la table "Utilisee" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans la brosse, donc elles
  //       ne seront pas utilisées par Remap_brush_LOWLEVEL.
  for (Couleur=0;Couleur<=255;Couleur++)
    if (Utilisee[Couleur])
      Utilisee[Couleur]=Meilleure_couleur(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].G,Brouillon_Palette[Couleur].B);

  //   Il reste une couleur non calculée dans la table qu'il faut mettre à
  // jour: c'est la couleur de fond. On l'avait inhibée pour éviter son
  // calcul inutile, mais comme il est possible qu'elle soit quand même dans
  // la brosse, on va mettre dans la table une relation d'équivalence entre
  // les deux palettes: comme on ne veut pas que la couleur soit remplacée,
  // on va dire qu'on veut qu'elle soit remplacée par la couleur en question.
  Utilisee[Back_color]=Back_color;

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_LOWLEVEL(Utilisee,Brosse,Brosse_Largeur,Brosse_Hauteur,Brosse_Largeur);
  //Remap_brush_LOWLEVEL(Utilisee);
}



void Outline_brush(void)
{
  long /*Pos,*/x_pos,y_pos;
  byte state;
  byte * Nouvelle_brosse;
  byte * Temporaire;
  word width;
  word height;


  width=Brosse_Largeur+2;
  height=Brosse_Hauteur+2;
  Nouvelle_brosse=(byte *)malloc(((long)width)*height);

  if (Nouvelle_brosse)
  {
    // On remplit la bordure ajoutée par la Backcolor
    memset(Nouvelle_brosse,Back_color,((long)width)*height);

    // On copie la brosse courante dans la nouvelle
    Copier_une_partie_d_image_dans_une_autre(Brosse, // Source
                                             0, 0,
                                             Brosse_Largeur,
                                             Brosse_Hauteur,
                                             Brosse_Largeur,
                                             Nouvelle_brosse, // Destination
                                             1, 1,
                                             width);

    // On intervertit la nouvelle et l'ancienne brosse:
    Temporaire=Brosse;
    Brosse=Nouvelle_brosse;
    Brosse_Largeur+=2;
    Brosse_Hauteur+=2;
    width-=2;
    height-=2;

    // Si on "outline" avec une couleur différente de la Back_color on y va!
    if (Fore_color!=Back_color)
    {
      // 1er balayage (horizontal)
      for (y_pos=1; y_pos<Brosse_Hauteur-1; y_pos++)
      {
        state=0;
        for (x_pos=1; x_pos<Brosse_Largeur-1; x_pos++)
        {
          if (Temporaire[((y_pos-1)*width)+x_pos-1]==Back_color)
          {
            if (state)
            {
              Pixel_dans_brosse(x_pos,y_pos,Fore_color);
              state=0;
            }
          }
          else
          {
            if (!state)
            {
              Pixel_dans_brosse(x_pos-1,y_pos,Fore_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (state)
          Pixel_dans_brosse(x_pos,y_pos,Fore_color);
      }

      // 2ème balayage (vertical)
      for (x_pos=1; x_pos<Brosse_Largeur-1; x_pos++)
      {
        state=0;
        for (y_pos=1; y_pos<Brosse_Hauteur-1; y_pos++)
        {
          if (Temporaire[((y_pos-1)*width)+x_pos-1]==Back_color)
          {
            if (state)
            {
              Pixel_dans_brosse(x_pos,y_pos,Fore_color);
              state=0;
            }
          }
          else
          {
            if (!state)
            {
              Pixel_dans_brosse(x_pos,y_pos-1,Fore_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (state)
          Pixel_dans_brosse(x_pos,y_pos,Fore_color);
      }
    }

    // On recentre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);

    free(Temporaire); // Libération de l'ancienne brosse

    // Réallocation d'un buffer de Smear
    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Largeur)*Smear_Brosse_Hauteur);
  }
  else
    Erreur(0); // Pas assez de mémoire!
}


void Nibble_brush(void)
{
  long /*Pos,*/x_pos,y_pos;
  byte state;
  byte * Nouvelle_brosse;
  byte * Temporaire;
  word width;
  word height;

  if ( (Brosse_Largeur>2) && (Brosse_Hauteur>2) )
  {
    width=Brosse_Largeur-2;
    height=Brosse_Hauteur-2;
    Nouvelle_brosse=(byte *)malloc(((long)width)*height);

    if (Nouvelle_brosse)
    {
      // On copie la brosse courante dans la nouvelle
      Copier_une_partie_d_image_dans_une_autre(Brosse, // Source
                                               1,
                                               1,
                                               width,
                                               height,
                                               Brosse_Largeur,
                                               Nouvelle_brosse, // Destination
                                               0,
                                               0,
                                               width);

      // On intervertit la nouvelle et l'ancienne brosse:
      Temporaire=Brosse;
      Brosse=Nouvelle_brosse;
      Brosse_Largeur-=2;
      Brosse_Hauteur-=2;
      width+=2;
      height+=2;

      // 1er balayage (horizontal)
      for (y_pos=0; y_pos<Brosse_Hauteur; y_pos++)
      {
        state=(Temporaire[(y_pos+1)*width]!=Back_color);
        for (x_pos=0; x_pos<Brosse_Largeur; x_pos++)
        {
          if (Temporaire[((y_pos+1)*width)+x_pos+1]==Back_color)
          {
            if (state)
            {
              if (x_pos>0)
                Pixel_dans_brosse(x_pos-1,y_pos,Back_color);
              state=0;
            }
          }
          else
          {
            if (!state)
            {
              Pixel_dans_brosse(x_pos,y_pos,Back_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (Temporaire[((y_pos+1)*width)+x_pos+1]==Back_color)
          Pixel_dans_brosse(x_pos-1,y_pos,Back_color);
      }

      // 2ème balayage (vertical)
      for (x_pos=0; x_pos<Brosse_Largeur; x_pos++)
      {
        state=(Temporaire[width+x_pos+1]!=Back_color);;
        for (y_pos=0; y_pos<Brosse_Hauteur; y_pos++)
        {
          if (Temporaire[((y_pos+1)*width)+x_pos+1]==Back_color)
          {
            if (state)
            {
              if (y_pos>0)
                Pixel_dans_brosse(x_pos,y_pos-1,Back_color);
              state=0;
            }
          }
          else
          {
            if (!state)
            {
              Pixel_dans_brosse(x_pos,y_pos,Back_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (Temporaire[((y_pos+1)*width)+x_pos+1]==Back_color)
          Pixel_dans_brosse(x_pos,y_pos-1,Back_color);
      }

      // On recentre la prise sur la brosse
      Brosse_Decalage_X=(Brosse_Largeur>>1);
      Brosse_Decalage_Y=(Brosse_Hauteur>>1);

      free(Temporaire); // Libération de l'ancienne brosse

      // Réallocation d'un buffer de Smear
      free(Smear_Brosse);
      Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
      Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Largeur)*Smear_Brosse_Hauteur);
    }
    else
      Erreur(0);  // Pas assez de mémoire!
  }
}



void Capturer_brosse_au_lasso(int Vertices, short * Points,short clear)
{
  short Debut_X=Limite_Droite+1;
  short Debut_Y=Limite_Bas+1;
  short Fin_X=Limite_Gauche-1;
  short Fin_Y=Limite_Haut-1;
  short Temporaire;
  short x_pos;
  short y_pos;
  word  Nouvelle_Brosse_Largeur;
  word  Nouvelle_Brosse_Hauteur;


  // On recherche les bornes de la brosse:
  for (Temporaire=0; Temporaire<Vertices; Temporaire++)
  {
    x_pos=Points[Temporaire<<1];
    y_pos=Points[(Temporaire<<1)+1];
    if (x_pos<Debut_X)
      Debut_X=x_pos;
    if (x_pos>Fin_X)
      Fin_X=x_pos;
    if (y_pos<Debut_Y)
      Debut_Y=y_pos;
    if (y_pos>Fin_Y)
      Fin_Y=y_pos;
  }

  // On clippe ces bornes à l'écran:
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((Debut_X<Principal_Largeur_image) && (Debut_Y<Principal_Hauteur_image))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_Decalage_X=0;
    Tiling_Decalage_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    Nouvelle_Brosse_Largeur=(Fin_X-Debut_X)+1;
    Nouvelle_Brosse_Hauteur=(Fin_Y-Debut_Y)+1;

    if ( (((long)Brosse_Hauteur)*Brosse_Largeur) !=
         (((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur) )
    {
      free(Brosse);
      Brosse=(byte *)malloc(((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur);
      if (!Brosse)
      {
        Erreur(0);

        Brosse=(byte *)malloc(1*1);
        Nouvelle_Brosse_Hauteur=Nouvelle_Brosse_Largeur=1;
        *Brosse=Fore_color;
      }
    }
    Brosse_Largeur=Nouvelle_Brosse_Largeur;
    Brosse_Hauteur=Nouvelle_Brosse_Hauteur;

    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=Debut_X;
    Brosse_Decalage_Y=Debut_Y;
    Pixel_figure=Pixel_figure_Dans_brosse;

    memset(Brosse,Back_color,(long)Brosse_Largeur*Brosse_Hauteur);
    Polyfill_General(Vertices,Points,~Back_color);

    // On retrace les bordures du lasso:
    for (Temporaire=1; Temporaire<Vertices; Temporaire++)
    {
      Tracer_ligne_General(Points[(Temporaire-1)<<1],Points[((Temporaire-1)<<1)+1],
                           Points[Temporaire<<1],Points[(Temporaire<<1)+1],
                           ~Back_color);
    }
    Tracer_ligne_General(Points[(Vertices-1)<<1],Points[((Vertices-1)<<1)+1],
                         Points[0],Points[1],
                         ~Back_color);

    // On scanne la brosse pour remplacer tous les pixels affectés par le
    // polyfill par ceux de l'image:
    for (y_pos=Debut_Y;y_pos<=Fin_Y;y_pos++)
      for (x_pos=Debut_X;x_pos<=Fin_X;x_pos++)
        if (Lit_pixel_dans_brosse(x_pos-Debut_X,y_pos-Debut_Y)!=Back_color)
        {
          Pixel_dans_brosse(x_pos-Debut_X,y_pos-Debut_Y,Lit_pixel_dans_ecran_courant(x_pos,y_pos));
          // On regarde s'il faut effacer quelque chose:
          if (clear)
            Pixel_dans_ecran_courant(x_pos,y_pos,Back_color);
        }

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
}



//------------------------- Etirement de la brosse ---------------------------

void Etirer_brosse(short X1, short Y1, short X2, short Y2)
{
  int    offset,line,Colonne;
  byte * New_Brosse;

  int    New_Brosse_Largeur;  // Width de la nouvelle brosse
  int    New_Brosse_Hauteur;  // Height de la nouvelle brosse

  int    Pos_X_dans_brosse;   // Position courante dans l'ancienne brosse
  int    Pos_Y_dans_brosse;
  int    Delta_X_dans_brosse; // "Vecteur incrémental" du point précédent
  int    Delta_Y_dans_brosse;
  int    Pos_X_initial;       // Position X de début de parcours de ligne
  int    Dx,Dy;

  Dx=(X1<X2)?1:-1;
  Dy=(Y1<Y2)?1:-1;

  // Calcul des nouvelles dimensions de la brosse:
  if ((New_Brosse_Largeur=X1-X2)<0)
    New_Brosse_Largeur=-New_Brosse_Largeur;
  New_Brosse_Largeur++;

  if ((New_Brosse_Hauteur=Y1-Y2)<0)
    New_Brosse_Hauteur=-New_Brosse_Hauteur;
  New_Brosse_Hauteur++;

  // Calcul des anciennes dimensions de la brosse:

  // Calcul du "vecteur incrémental":
  Delta_X_dans_brosse=(Brosse_Largeur<<16)/(X2-X1+Dx);
  Delta_Y_dans_brosse=(Brosse_Hauteur<<16)/(Y2-Y1+Dy);

  // Calcul de la valeur initiale de x_pos pour chaque ligne:
  if (Dx>=0)
    Pos_X_initial = 0;                // Pas d'inversion en X de la brosse
  else
    Pos_X_initial = (Brosse_Largeur<<16)-1; // Inversion en X de la brosse

  free(Smear_Brosse); // On libère un peu de mémoire

  if ((New_Brosse=((byte *)malloc(New_Brosse_Largeur*New_Brosse_Hauteur))))
  {
    offset=0;

    // Calcul de la valeur initiale de y_pos:
    if (Dy>=0)
      Pos_Y_dans_brosse=0;                // Pas d'inversion en Y de la brosse
    else
      Pos_Y_dans_brosse=(Brosse_Hauteur<<16)-1; // Inversion en Y de la brosse

    // Pour chaque ligne
    for (line=0;line<New_Brosse_Hauteur;line++)
    {
      // On repart du début de la ligne:
      Pos_X_dans_brosse=Pos_X_initial;

      // Pour chaque colonne:
      for (Colonne=0;Colonne<New_Brosse_Largeur;Colonne++)
      {
        // On copie le pixel:
        New_Brosse[offset]=Lit_pixel_dans_brosse(Pos_X_dans_brosse>>16,Pos_Y_dans_brosse>>16);
        // On passe à la colonne de brosse suivante:
        Pos_X_dans_brosse+=Delta_X_dans_brosse;
        // On passe au pixel suivant de la nouvelle brosse:
        offset++;
      }

      // On passe à la ligne de brosse suivante:
      Pos_Y_dans_brosse+=Delta_Y_dans_brosse;
    }

    free(Brosse);
    Brosse=New_Brosse;

    Brosse_Largeur=New_Brosse_Largeur;
    Brosse_Hauteur=New_Brosse_Hauteur;

    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffit donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_Brosse car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);
    Erreur(0);
  }
}



void Etirer_brosse_preview(short X1, short Y1, short X2, short Y2)
{
  int    Pos_X_src,Pos_Y_src;
  int    Pos_X_src_Initiale,Pos_Y_src_Initiale;
  int    Delta_X,Delta_Y;
  int    Pos_X_dest,Pos_Y_dest;
  int    Pos_X_dest_Initiale,Pos_Y_dest_Initiale;
  int    Pos_X_dest_Finale,Pos_Y_dest_Finale;
  int    Largeur_dest,Hauteur_dest;
  byte   Couleur;


  // 1er calcul des positions destination extremes:
  Pos_X_dest_Initiale=Min(X1,X2);
  Pos_Y_dest_Initiale=Min(Y1,Y2);
  Pos_X_dest_Finale  =Max(X1,X2);
  Pos_Y_dest_Finale  =Max(Y1,Y2);

  // Calcul des dimensions de la destination:
  Largeur_dest=Pos_X_dest_Finale-Pos_X_dest_Initiale+1;
  Hauteur_dest=Pos_Y_dest_Finale-Pos_Y_dest_Initiale+1;

  // Calcul des vecteurs d'incrémentation :
  Delta_X=(Brosse_Largeur<<16)/Largeur_dest;
  Delta_Y=(Brosse_Hauteur<<16)/Hauteur_dest;

  // 1er calcul de la position X initiale dans la source:
  Pos_X_src_Initiale=(Brosse_Largeur<<16)*
                     (Max(Pos_X_dest_Initiale,Limite_Gauche)-
                      Pos_X_dest_Initiale)/Largeur_dest;
  // Calcul du clip de la destination:
  Pos_X_dest_Initiale=Max(Pos_X_dest_Initiale,Limite_Gauche);
  Pos_X_dest_Finale  =Min(Pos_X_dest_Finale  ,Limite_visible_Droite);
  // On discute selon l'inversion en X
  if (X1>X2)
  {
    // Inversion -> Inversion du signe de Delta_X
    Delta_X=-Delta_X;
    Pos_X_src_Initiale=(Brosse_Largeur<<16)-1-Pos_X_src_Initiale;
  }

  // 1er calcul de la position Y initiale dans la source:
  Pos_Y_src_Initiale=(Brosse_Hauteur<<16)*
                     (Max(Pos_Y_dest_Initiale,Limite_Haut)-
                      Pos_Y_dest_Initiale)/Hauteur_dest;
  // Calcul du clip de la destination:
  Pos_Y_dest_Initiale=Max(Pos_Y_dest_Initiale,Limite_Haut);
  Pos_Y_dest_Finale  =Min(Pos_Y_dest_Finale  ,Limite_visible_Bas);
  // On discute selon l'inversion en Y
  if (Y1>Y2)
  {
    // Inversion -> Inversion du signe de Delta_Y
    Delta_Y=-Delta_Y;
    Pos_Y_src_Initiale=(Brosse_Hauteur<<16)-1-Pos_Y_src_Initiale;
  }

  // Pour chaque ligne :
  Pos_Y_src=Pos_Y_src_Initiale;
  for (Pos_Y_dest=Pos_Y_dest_Initiale;Pos_Y_dest<=Pos_Y_dest_Finale;Pos_Y_dest++)
  {
    // Pour chaque colonne:
    Pos_X_src=Pos_X_src_Initiale;
    for (Pos_X_dest=Pos_X_dest_Initiale;Pos_X_dest<=Pos_X_dest_Finale;Pos_X_dest++)
    {
      Couleur=Lit_pixel_dans_brosse(Pos_X_src>>16,Pos_Y_src>>16);
      if (Couleur!=Back_color)
        Pixel_Preview(Pos_X_dest,Pos_Y_dest,Couleur);

      Pos_X_src+=Delta_X;
    }

    Pos_Y_src+=Delta_Y;
  }
  Mettre_Ecran_A_Jour(Pos_X_dest_Initiale,Pos_Y_dest_Initiale,Largeur_dest,Hauteur_dest);
}



//------------------------- Rotation de la brosse ---------------------------

#define INDEFINI (-1.0e20F)
float * ScanY_Xt[2];
float * ScanY_Yt[2];
float * ScanY_X[2];


void Interpoler_texture(int Debut_X,int Debut_Y,int Xt1,int Yt1,
                        int Fin_X  ,int Fin_Y  ,int Xt2,int Yt2,int height)
{
  int x_pos,y_pos;
  int Incr_X,Incr_Y;
  int i,Cumul;
  int Delta_X,Delta_Y;
  int Delta_Xt=Xt2-Xt1;
  int Delta_Yt=Yt2-Yt1;
  int Delta_X2=Fin_X-Debut_X;
  int Delta_Y2=Fin_Y-Debut_Y;
  float Xt,Yt;


  x_pos=Debut_X;
  y_pos=Debut_Y;

  if (Debut_X<Fin_X)
  {
    Incr_X=+1;
    Delta_X=Delta_X2;
  }
  else
  {
    Incr_X=-1;
    Delta_X=-Delta_X2;
  }

  if (Debut_Y<Fin_Y)
  {
    Incr_Y=+1;
    Delta_Y=Delta_Y2;
  }
  else
  {
    Incr_Y=-1;
    Delta_Y=-Delta_Y2;
  }

  if (Delta_X>Delta_Y)
  {
    Cumul=Delta_X>>1;
    for (i=0; i<=Delta_X; i++)
    {
      if (Cumul>=Delta_X)
      {
        Cumul-=Delta_X;
        y_pos+=Incr_Y;
      }

      if ((y_pos>=0) && (y_pos<height))
      {
        Xt=(((float)((x_pos-Debut_X)*Delta_Xt))/(float)Delta_X2) + (float)Xt1;
        Yt=(((float)((x_pos-Debut_X)*Delta_Yt))/(float)Delta_X2) + (float)Yt1;
        if (ScanY_X[0][y_pos]==INDEFINI) // Gauche non défini
        {
          ScanY_X[0][y_pos]=x_pos;
          ScanY_Xt[0][y_pos]=Xt;
          ScanY_Yt[0][y_pos]=Yt;
        }
        else
        {
          if (x_pos>=ScanY_X[0][y_pos])
          {
            if ((ScanY_X[1][y_pos]==INDEFINI) // Droit non défini
             || (x_pos>ScanY_X[1][y_pos]))
            {
              ScanY_X[1][y_pos]=x_pos;
              ScanY_Xt[1][y_pos]=Xt;
              ScanY_Yt[1][y_pos]=Yt;
            }
          }
          else
          {
            if (ScanY_X[1][y_pos]==INDEFINI) // Droit non défini
            {
              ScanY_X[1][y_pos]=ScanY_X[0][y_pos];
              ScanY_Xt[1][y_pos]=ScanY_Xt[0][y_pos];
              ScanY_Yt[1][y_pos]=ScanY_Yt[0][y_pos];
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=Xt;
              ScanY_Yt[0][y_pos]=Yt;
            }
            else
            {
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=Xt;
              ScanY_Yt[0][y_pos]=Yt;
            }
          }
        }
      }
      x_pos+=Incr_X;
      Cumul+=Delta_Y;
    }
  }
  else
  {
    Cumul=Delta_Y>>1;
    for (i=0; i<=Delta_Y; i++)
    {
      if (Cumul>=Delta_Y)
      {
        Cumul-=Delta_Y;
        x_pos+=Incr_X;
      }

      if ((y_pos>=0) && (y_pos<height))
      {
        Xt=(((float)((y_pos-Debut_Y)*Delta_Xt))/(float)Delta_Y2) + (float)Xt1;
        Yt=(((float)((y_pos-Debut_Y)*Delta_Yt))/(float)Delta_Y2) + (float)Yt1;
        if (ScanY_X[0][y_pos]==INDEFINI) // Gauche non défini
        {
          ScanY_X[0][y_pos]=x_pos;
          ScanY_Xt[0][y_pos]=Xt;
          ScanY_Yt[0][y_pos]=Yt;
        }
        else
        {
          if (x_pos>=ScanY_X[0][y_pos])
          {
            if ((ScanY_X[1][y_pos]==INDEFINI) // Droit non défini
             || (x_pos>ScanY_X[1][y_pos]))
            {
              ScanY_X[1][y_pos]=x_pos;
              ScanY_Xt[1][y_pos]=Xt;
              ScanY_Yt[1][y_pos]=Yt;
            }
          }
          else
          {
            if (ScanY_X[1][y_pos]==INDEFINI) // Droit non défini
            {
              ScanY_X[1][y_pos]=ScanY_X[0][y_pos];
              ScanY_Xt[1][y_pos]=ScanY_Xt[0][y_pos];
              ScanY_Yt[1][y_pos]=ScanY_Yt[0][y_pos];
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=Xt;
              ScanY_Yt[0][y_pos]=Yt;
            }
            else
            {
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=Xt;
              ScanY_Yt[0][y_pos]=Yt;
            }
          }
        }
      }
      y_pos+=Incr_Y;
      Cumul+=Delta_X;
    }
  }
}



void Calculer_quad_texture(int X1,int Y1,int Xt1,int Yt1,
                           int X2,int Y2,int Xt2,int Yt2,
                           int X3,int Y3,int Xt3,int Yt3,
                           int X4,int Y4,int Xt4,int Yt4,
                           byte * Buffer, int width, int height)
{
  int Xmin,/*Xmax,*/Ymin/*,Ymax*/;
  int X,Y,Xt,Yt;
  int Debut_X,Fin_X,Largeur_ligne;
  float Temp;
  //byte Couleur;

  Xmin=Min(Min(X1,X2),Min(X3,X4));
  Ymin=Min(Min(Y1,Y2),Min(Y3,Y4));

  ScanY_Xt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(height*sizeof(float));
  ScanY_X[0] =(float *)malloc(height*sizeof(float));
  ScanY_X[1] =(float *)malloc(height*sizeof(float));

  // Remplir avec des valeurs égales à INDEFINI.
  for (Y=0; Y<height; Y++)
  {
    ScanY_X[0][Y]=INDEFINI;
    ScanY_X[1][Y]=INDEFINI;
  }

  Interpoler_texture(X1-Xmin,Y1-Ymin,Xt1,Yt1,X3-Xmin,Y3-Ymin,Xt3,Yt3,height);
  Interpoler_texture(X3-Xmin,Y3-Ymin,Xt3,Yt3,X4-Xmin,Y4-Ymin,Xt4,Yt4,height);
  Interpoler_texture(X4-Xmin,Y4-Ymin,Xt4,Yt4,X2-Xmin,Y2-Ymin,Xt2,Yt2,height);
  Interpoler_texture(X2-Xmin,Y2-Ymin,Xt2,Yt2,X1-Xmin,Y1-Ymin,Xt1,Yt1,height);

  for (Y=0; Y<height; Y++)
  {
    Debut_X=Round(ScanY_X[0][Y]);
    Fin_X  =Round(ScanY_X[1][Y]);

    Largeur_ligne=1+Fin_X-Debut_X;

    for (X=0; X<Debut_X; X++)
      Buffer[X+(Y*width)]=Back_color;
    for (; X<=Fin_X; X++)
    {
      Temp=(float)(0.5+(float)X-ScanY_X[0][Y])/(float)Largeur_ligne;
      Xt=Round((float)(ScanY_Xt[0][Y])+(Temp*(ScanY_Xt[1][Y]-ScanY_Xt[0][Y])));
      Yt=Round((float)(ScanY_Yt[0][Y])+(Temp*(ScanY_Yt[1][Y]-ScanY_Yt[0][Y])));

      Buffer[X+(Y*width)]=Lit_pixel_dans_brosse(Xt,Yt);
    }
    for (; X<width; X++)
      Buffer[X+(Y*width)]=Back_color;
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}



void Tourner_brosse(float angle)
{
  byte * New_Brosse;
  int    New_Brosse_Largeur;  // Width de la nouvelle brosse
  int    New_Brosse_Hauteur;  // Height de la nouvelle brosse

  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  int Debut_X,Fin_X,Debut_Y,Fin_Y;
  int Xmin,Xmax,Ymin,Ymax;
  float cosA=cos(angle);
  float sinA=sin(angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  Debut_X=1-(Brosse_Largeur>>1);
  Debut_Y=1-(Brosse_Hauteur>>1);
  Fin_X=Debut_X+Brosse_Largeur-1;
  Fin_Y=Debut_Y+Brosse_Hauteur-1;

  Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
  Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
  Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
  Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

  // Calcul des nouvelles dimensions de la brosse:
  Xmin=Min(Min((int)X1,(int)X2),Min((int)X3,(int)X4));
  Xmax=Max(Max((int)X1,(int)X2),Max((int)X3,(int)X4));
  Ymin=Min(Min((int)Y1,(int)Y2),Min((int)Y3,(int)Y4));
  Ymax=Max(Max((int)Y1,(int)Y2),Max((int)Y3,(int)Y4));

  New_Brosse_Largeur=Xmax+1-Xmin;
  New_Brosse_Hauteur=Ymax+1-Ymin;

  free(Smear_Brosse); // On libère un peu de mémoire

  if ((New_Brosse=((byte *)malloc(New_Brosse_Largeur*New_Brosse_Hauteur))))
  {
    // Et maintenant on calcule la nouvelle brosse tournée.
    Calculer_quad_texture(X1,Y1,               0,               0,
                          X2,Y2,Brosse_Largeur-1,               0,
                          X3,Y3,               0,Brosse_Hauteur-1,
                          X4,Y4,Brosse_Largeur-1,Brosse_Hauteur-1,
                          New_Brosse,New_Brosse_Largeur,New_Brosse_Hauteur);

    free(Brosse);
    Brosse=New_Brosse;

    Brosse_Largeur=New_Brosse_Largeur;
    Brosse_Hauteur=New_Brosse_Hauteur;

    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffit donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_Brosse car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);
    Erreur(0);
  }
}



void Dessiner_quad_texture_preview(int X1,int Y1,int Xt1,int Yt1,
                                   int X2,int Y2,int Xt2,int Yt2,
                                   int X3,int Y3,int Xt3,int Yt3,
                                   int X4,int Y4,int Xt4,int Yt4)
{
  int Xmin,Xmax,Ymin,Ymax;
  int X,Y,Xt,Yt;
  int Y_,Ymin_;
  int Debut_X,Fin_X,width,height;
  float Temp;
  byte Couleur;

  Xmin=Min(Min(X1,X2),Min(X3,X4));
  Xmax=Max(Max(X1,X2),Max(X3,X4));
  Ymin=Min(Min(Y1,Y2),Min(Y3,Y4));
  Ymax=Max(Max(Y1,Y2),Max(Y3,Y4));
  height=1+Ymax-Ymin;

  ScanY_Xt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(height*sizeof(float));
  ScanY_X[0] =(float *)malloc(height*sizeof(float));
  ScanY_X[1] =(float *)malloc(height*sizeof(float));

  // Remplir avec des valeurs égales à INDEFINI.
  for (Y=0; Y<height; Y++)
  {
    ScanY_X[0][Y]=INDEFINI;
    ScanY_X[1][Y]=INDEFINI;
  }

  Interpoler_texture(X1,Y1-Ymin,Xt1,Yt1,X3,Y3-Ymin,Xt3,Yt3,height);
  Interpoler_texture(X3,Y3-Ymin,Xt3,Yt3,X4,Y4-Ymin,Xt4,Yt4,height);
  Interpoler_texture(X4,Y4-Ymin,Xt4,Yt4,X2,Y2-Ymin,Xt2,Yt2,height);
  Interpoler_texture(X2,Y2-Ymin,Xt2,Yt2,X1,Y1-Ymin,Xt1,Yt1,height);

  Ymin_=Ymin;
  if (Ymin<Limite_Haut) Ymin=Limite_Haut;
  if (Ymax>Limite_Bas)  Ymax=Limite_Bas;

  for (Y_=Ymin; Y_<=Ymax; Y_++)
  {
    Y=Y_-Ymin_;
    Debut_X=Round(ScanY_X[0][Y]);
    Fin_X  =Round(ScanY_X[1][Y]);

    width=1+Fin_X-Debut_X;

    if (Debut_X<Limite_Gauche) Debut_X=Limite_Gauche;
    if (  Fin_X>Limite_Droite)   Fin_X=Limite_Droite;

    for (X=Debut_X; X<=Fin_X; X++)
    {
      Temp=(float)(0.5+(float)X-ScanY_X[0][Y])/(float)width;
      Xt=Round((float)(ScanY_Xt[0][Y])+(Temp*(ScanY_Xt[1][Y]-ScanY_Xt[0][Y])));
      Yt=Round((float)(ScanY_Yt[0][Y])+(Temp*(ScanY_Yt[1][Y]-ScanY_Yt[0][Y])));

      Couleur=Lit_pixel_dans_brosse(Xt,Yt);
      if (Couleur!=Back_color)
        Pixel_Preview(X,Y_,Couleur);
    }
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}


void Tourner_brosse_preview(float angle)
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  int Debut_X,Fin_X,Debut_Y,Fin_Y;
  float cosA=cos(angle);
  float sinA=sin(angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  Debut_X=1-(Brosse_Largeur>>1);
  Debut_Y=1-(Brosse_Hauteur>>1);
  Fin_X=Debut_X+Brosse_Largeur-1;
  Fin_Y=Debut_Y+Brosse_Hauteur-1;

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

  // Et maintenant on dessine la brosse tournée.
  Dessiner_quad_texture_preview(X1,Y1,               0,               0,
                                X2,Y2,Brosse_Largeur-1,               0,
                                X3,Y3,               0,Brosse_Hauteur-1,
                                X4,Y4,Brosse_Largeur-1,Brosse_Hauteur-1);
  Debut_X=Min(Min(X1,X2),Min(X3,X4));
  Fin_X=Max(Max(X1,X2),Max(X3,X4));
  Debut_Y=Min(Min(Y1,Y2),Min(Y3,Y4));
  Fin_Y=Max(Max(Y1,Y2),Max(Y3,Y4));
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}
