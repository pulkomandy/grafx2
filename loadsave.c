/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
#define _XOPEN_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#ifndef __no_pnglib__
#include <png.h>
#endif

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "pages.h"
#include "op_c.h"
#include "boutons.h"
#include "erreurs.h"
#include "io.h"
#include "sdlscreen.h"
#include "windows.h"
#include "loadsave.h"

// -- PKM -------------------------------------------------------------------
void Test_PKM(void);
void Load_PKM(void);
void Save_PKM(void);

// -- LBM -------------------------------------------------------------------
void Test_LBM(void);
void Load_LBM(void);
void Save_LBM(void);

// -- GIF -------------------------------------------------------------------
void Test_GIF(void);
void Load_GIF(void);
void Save_GIF(void);

// -- PCX -------------------------------------------------------------------
void Test_PCX(void);
void Load_PCX(void);
void Save_PCX(void);

// -- BMP -------------------------------------------------------------------
void Test_BMP(void);
void Load_BMP(void);
void Save_BMP(void);

// -- IMG -------------------------------------------------------------------
void Test_IMG(void);
void Load_IMG(void);
void Save_IMG(void);

// -- SCx -------------------------------------------------------------------
void Test_SCx(void);
void Load_SCx(void);
void Save_SCx(void);

// -- CEL -------------------------------------------------------------------
void Test_CEL(void);
void Load_CEL(void);
void Save_CEL(void);

// -- KCF -------------------------------------------------------------------
void Test_KCF(void);
void Load_KCF(void);
void Save_KCF(void);

// -- PAL -------------------------------------------------------------------
void Test_PAL(void);
void Load_PAL(void);
void Save_PAL(void);

// -- PI1 -------------------------------------------------------------------
void Test_PI1(void);
void Load_PI1(void);
void Save_PI1(void);

// -- PC1 -------------------------------------------------------------------
void Test_PC1(void);
void Load_PC1(void);
void Save_PC1(void);

// -- PNG -------------------------------------------------------------------
#ifndef __no_pnglib__
void Test_PNG(void);
void Load_PNG(void);
void Save_PNG(void);
#endif

T_Format FormatFichier[NB_FORMATS_CONNUS] = {
  {"pkm", Test_PKM, Load_PKM, Save_PKM, 1, 1},
  {"lbm", Test_LBM, Load_LBM, Save_LBM, 1, 0},
  {"gif", Test_GIF, Load_GIF, Save_GIF, 1, 1},
  {"bmp", Test_BMP, Load_BMP, Save_BMP, 1, 0},
  {"pcx", Test_PCX, Load_PCX, Save_PCX, 1, 0},
  {"img", Test_IMG, Load_IMG, Save_IMG, 1, 0},
  {"sc?", Test_SCx, Load_SCx, Save_SCx, 1, 0},
  {"pi1", Test_PI1, Load_PI1, Save_PI1, 1, 0},
  {"pc1", Test_PC1, Load_PC1, Save_PC1, 1, 0},
  {"cel", Test_CEL, Load_CEL, Save_CEL, 1, 0},
  {"kcf", Test_KCF, Load_KCF, Save_KCF, 0, 0},
  {"pal", Test_PAL, Load_PAL, Save_PAL, 0, 0},
#ifndef __no_pnglib__
  {"png", Test_PNG, Load_PNG, Save_PNG, 1, 1}
#endif
};

// Cette variable est alimentée après chargement réussi d'une image.
// Actuellement seul le format PNG peut donner autre chose que PIXEL_SIMPLE.
enum PIXEL_RATIO Ratio_image_chargee=PIXEL_SIMPLE;

// Chargement des pixels dans l'écran principal
void Pixel_Chargement_dans_ecran_courant(word x_pos,word y_pos,byte Couleur)
{
  //if ((x_pos>=0) && (y_pos>=0)) //Toujours vrai ?
  if ((x_pos<Principal_Largeur_image) && (y_pos<Principal_Hauteur_image))
    Pixel_dans_ecran_courant(x_pos,y_pos,Couleur);
}


// Chargement des pixels dans la brosse
void Pixel_Chargement_dans_brosse(word x_pos,word y_pos,byte Couleur)
{
  //if ((x_pos>=0) && (y_pos>=0))
  if ((x_pos<Brosse_Largeur) && (y_pos<Brosse_Hauteur))
    Pixel_dans_brosse(x_pos,y_pos,Couleur);
}


short Preview_Facteur_X;
short Preview_Facteur_Y;
short Preview_Pos_X;
short Preview_Pos_Y;

byte HBPm1; // header.BitPlanes-1


// Chargement des pixels dans la preview
void Pixel_Chargement_dans_preview(word x_pos,word y_pos,byte Couleur)
{
  if (((x_pos % Preview_Facteur_X)==0) && ((y_pos % Preview_Facteur_Y)==0))
  if ((x_pos<Principal_Largeur_image) && (y_pos<Principal_Hauteur_image))
    Pixel(Preview_Pos_X+(x_pos/Preview_Facteur_X),
          Preview_Pos_Y+(y_pos/Preview_Facteur_Y),
          Couleur);
}


void Remapper_fileselect(void)
{
  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
  {
    Calculer_couleurs_menu_optimales(Principal_Palette);

    if(
            (
            Principal_Palette[CM_Noir].R==Principal_Palette[CM_Fonce].R &&
            Principal_Palette[CM_Noir].G==Principal_Palette[CM_Fonce].G &&
            Principal_Palette[CM_Noir].B==Principal_Palette[CM_Fonce].B
            ) ||
            (
            Principal_Palette[CM_Clair].R==Principal_Palette[CM_Fonce].R &&
            Principal_Palette[CM_Clair].G==Principal_Palette[CM_Fonce].G &&
            Principal_Palette[CM_Clair].B==Principal_Palette[CM_Fonce].B
            ) ||
            (
            Principal_Palette[CM_Blanc].R==Principal_Palette[CM_Clair].R &&
            Principal_Palette[CM_Blanc].G==Principal_Palette[CM_Clair].G &&
            Principal_Palette[CM_Blanc].B==Principal_Palette[CM_Clair].B
            )
      )
    {
        // Si on charge une image monochrome, le fileselect ne sera plus visible. Dans ce cas on force quelques couleurs à des valeurs sures

        int black =
            Principal_Palette[CM_Noir].R +
            Principal_Palette[CM_Noir].G +
            Principal_Palette[CM_Noir].B;
        int Blanc =
            Principal_Palette[CM_Blanc].R +
            Principal_Palette[CM_Blanc].G +
            Principal_Palette[CM_Blanc].B;

        //Set_color(CM_Clair,(2*Blanc+black)/9,(2*Blanc+black)/9,(2*Blanc+black)/9);
        //Set_color(CM_Fonce,(2*black+Blanc)/9,(2*black+Blanc)/9,(2*black+Blanc)/9);
        Principal_Palette[CM_Fonce].R=(2*black+Blanc)/9;
        Principal_Palette[CM_Fonce].G=(2*black+Blanc)/9;
        Principal_Palette[CM_Fonce].B=(2*black+Blanc)/9;
        Principal_Palette[CM_Clair].R=(2*Blanc+black)/9;
        Principal_Palette[CM_Clair].G=(2*Blanc+black)/9;
        Principal_Palette[CM_Clair].B=(2*Blanc+black)/9;

        Set_palette(Principal_Palette);
    }
    Remapper_ecran_apres_changement_couleurs_menu();
  }
}



// Données pour la gestion du chargement en 24b
#define FORMAT_24B 0x100
typedef void (* fonction_afficheur_24b) (short,short,byte,byte,byte);
int                    Image_24b;
Composantes *   Buffer_image_24b;
fonction_afficheur_24b Pixel_Chargement_24b;


// Chargement des pixels dans le buffer 24b
void Pixel_Chargement_dans_buffer_24b(short x_pos,short y_pos,byte r,byte g,byte b)
{
  int index;

  if ((x_pos>=0) && (y_pos>=0))
  if ((x_pos<Principal_Largeur_image) && (y_pos<Principal_Hauteur_image))
  {
    index=(y_pos*Principal_Largeur_image)+x_pos;
    Buffer_image_24b[index].R=r;
    Buffer_image_24b[index].G=g;
    Buffer_image_24b[index].B=b;
  }
}

// Chargement des pixels dans la preview en 24b
void Pixel_Chargement_dans_preview_24b(short x_pos,short y_pos,byte r,byte g,byte b)
{
  byte Couleur;

  if (((x_pos % Preview_Facteur_X)==0) && ((y_pos % Preview_Facteur_Y)==0))
  if ((x_pos<Principal_Largeur_image) && (y_pos<Principal_Hauteur_image))
  {
    Couleur=((r >> 5) << 5) |
            ((g >> 5) << 2) |
            ((b >> 6));
    Pixel(Preview_Pos_X+(x_pos/Preview_Facteur_X),
          Preview_Pos_Y+(y_pos/Preview_Facteur_Y),
          Couleur);
  }
}

// Création d'une palette fake
void Palette_fake_24b(T_Palette palette)
{
  int Couleur;

  // Génération de la palette
  for (Couleur=0;Couleur<256;Couleur++)
  {
    palette[Couleur].R=((Couleur & 0xE0)>>5)<<5;
    palette[Couleur].G=((Couleur & 0x1C)>>2)<<5;
    palette[Couleur].B=((Couleur & 0x03)>>0)<<6;
  }
}

// Supplément à faire lors de l'initialisation d'une preview dans le cas
// d'une image 24b
void Initialiser_preview_24b(int width,int height)
{
  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
  {
    // Aiguillage du chargement 24b
    Pixel_Chargement_24b=Pixel_Chargement_dans_preview_24b;

    // Changement de palette
    Palette_fake_24b(Principal_Palette);
    Set_palette(Principal_Palette);
    Remapper_fileselect();
  }
  else
  {
    // Aiguillage du chargement 24b
    Pixel_Chargement_24b=Pixel_Chargement_dans_buffer_24b;

    // Allocation du buffer 24b
    Buffer_image_24b=
      (Composantes *)Emprunter_memoire_de_page(width*height*sizeof(Composantes));
    if (!Buffer_image_24b)
    {
      // Afficher un message d'erreur

      // Pour être sûr que ce soit lisible.
      Calculer_couleurs_menu_optimales(Principal_Palette);
      Message_Memoire_insuffisante();
      if (Pixel_de_chargement==Pixel_Chargement_dans_ecran_courant)
        Erreur_fichier=1; // 1 => On n'a pas perdu l'image courante
      else
        Erreur_fichier=3; // 3 => Chargement de brosse échoué
    }
    else
      Image_24b=1;        // On a un buffer à traiter en fin de chargement
  }
}




void Initialiser_preview(short width,short height,long size,int format)
//
//   Cette procédure doit être appelée par les routines de chargement
// d'images.
//   Elle doit être appelée entre le moment où l'on connait la dimension de
// l'image (dimension réelle, pas dimension tronquée) et l'affichage du
// premier point.
//
{
  char  Chaine[10];
  int   Image_en_24b;

  Image_en_24b=format & FORMAT_24B;
  format      =format & (~FORMAT_24B);

  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
  {
    // Préparation du chargement d'une preview:

    // Affichage des données "Image size:"
    if ((width<10000) && (height<10000))
    {
      Num2str(width,Chaine,4);
      Num2str(height,Chaine+5,4);
      Chaine[4]='x';
      Print_dans_fenetre(143,59,Chaine,CM_Noir,CM_Clair);
    }
    else
    {
      Print_dans_fenetre(143,59,"VERY BIG!",CM_Noir,CM_Clair);
    }

    // Affichage de la taille du fichier
    if (size<1048576)
    {
      // Le fichier fait moins d'un Mega, on affiche sa taille direct
      Num2str(size,Chaine,7);
      Print_dans_fenetre(236,59,Chaine,CM_Noir,CM_Clair);
    }
    else if ((size/1024)<100000)
    {
      // Le fichier fait plus d'un Mega, on peut afficher sa taille en Ko
      Num2str(size/1024,Chaine,5);
      strcpy(Chaine+5,"Kb");
      Print_dans_fenetre(236,59,Chaine,CM_Noir,CM_Clair);
    }
    else
    {
      // Le fichier fait plus de 100 Mega octets (cas très rare :))
      Print_dans_fenetre(236,59,"LARGE!!",CM_Noir,CM_Clair);
    }

    // Affichage du vrai format
    if (format!=Principal_Format)
    {
      Print_dans_fenetre( 59,59,FormatFichier[format-1].Extension,CM_Noir,CM_Clair);
    }

    // On efface le commentaire précédent
    Window_rectangle(45,70,32*8,8,CM_Clair);
    // Affichage du commentaire
    if (FormatFichier[format-1].Commentaire)
      Print_dans_fenetre(45,70,Principal_Commentaire,CM_Noir,CM_Clair);

    // Calculs des données nécessaires à l'affichage de la preview:
    Preview_Facteur_X=Round_div_max(width,122*Menu_Facteur_X);
    Preview_Facteur_Y=Round_div_max(height, 82*Menu_Facteur_Y);

    if ( (!Config.Maximize_preview) && (Preview_Facteur_X!=Preview_Facteur_Y) )
    {
      if (Preview_Facteur_X>Preview_Facteur_Y)
        Preview_Facteur_Y=Preview_Facteur_X;
      else
        Preview_Facteur_X=Preview_Facteur_Y;
    }

    Preview_Pos_X=Fenetre_Pos_X+183*Menu_Facteur_X;
    Preview_Pos_Y=Fenetre_Pos_Y+ 95*Menu_Facteur_Y;

    // On nettoie la zone où va s'afficher la preview:
    Window_rectangle(183,95,120,80,CM_Clair);
    
    // Un update pour couvrir les 4 zones: 3 libellés plus le commentaire
    Display_Window(45,48,256,30);
    // Zone de preview
    Display_Window(183,95,120,80);
  }
  else
  {
    if (Pixel_de_chargement==Pixel_Chargement_dans_ecran_courant)
    {
      if (Backup_avec_nouvelles_dimensions(0,width,height))
      {
        // La nouvelle page a pu être allouée, elle est pour l'instant pleine
        // de 0s. Elle fait Principal_Largeur_image de large.
        // Normalement tout va bien, tout est sous contrôle...
      }
      else
      {
        // Afficher un message d'erreur

        // Pour être sûr que ce soit lisible.
        Calculer_couleurs_menu_optimales(Principal_Palette);
        Message_Memoire_insuffisante();
        Erreur_fichier=1; // 1 => On n'a pas perdu l'image courante
      }
    }
    else // chargement dans la brosse
    {
      free(Brosse);
      free(Smear_Brosse);
      Brosse=(byte *)malloc(width*height);
      Brosse_Largeur=width;
      Brosse_Hauteur=height;
      if (Brosse)
      {
        Smear_Brosse=(byte *)malloc(width*height);
        if (!Smear_Brosse)
          Erreur_fichier=3;
      }
      else
        Erreur_fichier=3;
    }
  }

  if (!Erreur_fichier)
    if (Image_en_24b)
      Initialiser_preview_24b(width,height);
}



void Dessiner_preview_palette(void)
{
  short Indice;

  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
    for (Indice=0; Indice<256; Indice++)
      Window_rectangle(183+(Indice/16)*7,95+(Indice&15)*5,5,5,Indice);

  Display_Window(183,95,120,80);
}



// Calcul du nom complet du fichier
void Nom_fichier_complet(char * Nom_du_fichier, byte Sauve_Colorix)
{
    byte last_char;

    strcpy(Nom_du_fichier,Principal_Repertoire_fichier);

    //On va ajouter un séparateur à la fin du chemin s'il n'y est pas encore
    if (Nom_du_fichier[strlen(Nom_du_fichier)-1]!=SEPARATEUR_CHEMIN[0])
        strcat(Nom_du_fichier,SEPARATEUR_CHEMIN);

  // Si on est en train de sauvegarder une image Colorix, on calcule son ext.
  if (Sauve_Colorix)
  {
    last_char=strlen(Principal_Nom_fichier)-1;
    if (Principal_Nom_fichier[last_char]=='?')
    {
      if (Principal_Largeur_image<=320)
        Principal_Nom_fichier[last_char]='I';
      else
      {
        if (Principal_Largeur_image<=360)
          Principal_Nom_fichier[last_char]='Q';
        else
        {
          if (Principal_Largeur_image<=640)
            Principal_Nom_fichier[last_char]='F';
          else
          {
            if (Principal_Largeur_image<=800)
              Principal_Nom_fichier[last_char]='N';
            else
              Principal_Nom_fichier[last_char]='O';
          }
        }
      }
    }
  }

  strcat(Nom_du_fichier,Principal_Nom_fichier);
}


/////////////////////////////////////////////////////////////////////////////
//                    Gestion des lectures et écritures                    //
/////////////////////////////////////////////////////////////////////////////

void Lire_octet(FILE * file, byte *b)
{
  // FIXME : Remplacer les appelants par read_bytes(), et gérer les retours d'erreur.
  read_byte(file, b);
}

// --------------------------------------------------------------------------

byte * Tampon_ecriture;
word   Index_ecriture;

void Init_ecriture(void)
{
  Tampon_ecriture=(byte *)malloc(64000);
  Index_ecriture=0;
}

void Ecrire_octet(FILE *file, byte b)
{
  Tampon_ecriture[Index_ecriture++]=b;
  if (Index_ecriture>=64000)
  {
    if (! write_bytes(file,Tampon_ecriture,64000))
      Erreur_fichier=1;
    Index_ecriture=0;
  }
}

void Close_ecriture(FILE *file)
{
  if (Index_ecriture)
    if (! write_bytes(file,Tampon_ecriture,Index_ecriture))
      Erreur_fichier=1;
  free(Tampon_ecriture);
}


/////////////////////////////////////////////////////////////////////////////

// -------- Modifier la valeur du code d'erreur d'accès à un fichier --------
//   On n'est pas obligé d'utiliser cette fonction à chaque fois mais il est
// important de l'utiliser dans les cas du type:
//   if (!Erreur_fichier) *** else Erreur_fichier=***;
// En fait, dans le cas où l'on modifie Erreur_fichier alors qu'elle contient
// dèjà un code d'erreur.
void Modif_Erreur_fichier(int Nouvelle_valeur)
{
  if (Erreur_fichier>=0)
    Erreur_fichier=Nouvelle_valeur;
}


// -- Charger n'importe connu quel type de fichier d'image (ou palette) -----
void Charger_image(byte image)
{
  int  Indice; // Indice de balayage des formats
  int  format=0; // Format du fichier à charger


  // On place par défaut Erreur_fichier à vrai au cas où on ne sache pas
  // charger le format du fichier:
  Erreur_fichier=1;

  if (Principal_Format!=0)
  {
    FormatFichier[Principal_Format-1].Test();
    if (!Erreur_fichier)
      // Si dans le sélecteur il y a un format valide on le prend tout de suite
      format=Principal_Format-1;
  }

  if (Erreur_fichier)
  {
    //  Sinon, on va devoir scanner les différents formats qu'on connait pour
    // savoir à quel format est le fichier:
    for (Indice=0;Indice<NB_FORMATS_LOAD;Indice++)
    {
      // On appelle le testeur du format:
      FormatFichier[Indice].Test();
      // On s'arrête si le fichier est au bon format:
      if (Erreur_fichier==0)
      {
        format=Indice;
        break;
      }
    }
  }

  // Si on a su déterminer avec succès le format du fichier:
  if (!Erreur_fichier)
  {
    // On peut charger le fichier:
    Image_24b=0;
    Ratio_image_chargee=PIXEL_SIMPLE;
    // Dans certains cas il est possible que le chargement plante
    // après avoir modifié la palette. TODO
    FormatFichier[format].Load();

    if (Erreur_fichier>0)
    {
      Erreur(0);
    }

    if (Image_24b)
    {
      // On vient de charger une image 24b
      if (!Erreur_fichier)
      {
        // Le chargement a réussi, on peut faire la conversion en 256 couleurs
        if (Pixel_de_chargement==Pixel_Chargement_dans_ecran_courant)
        {
          // Cas d'un chargement dans l'image
          if (Convert_bitmap_24B_to_256(Principal_Ecran,Buffer_image_24b,Principal_Largeur_image,Principal_Hauteur_image,Principal_Palette))
            Erreur_fichier=2;
        }
        else
        {
          // Cas d'un chargement dans la brosse
          if (Convert_bitmap_24B_to_256(Brosse,Buffer_image_24b,Brosse_Largeur,Brosse_Hauteur,Principal_Palette))
            Erreur_fichier=2;
        }
        //if (!Erreur_fichier)
        //  Palette_256_to_64(Principal_Palette);
        // Normalement plus besoin car 256 color natif, et c'etait probablement
        // un bug - yr
      }

      free(Buffer_image_24b);
    }

    if (image)
    {
      if ( (Erreur_fichier!=1) && (FormatFichier[format].Backup_done) )
      {
        // On considère que l'image chargée n'est plus modifiée
        Principal_Image_modifiee=0;
        // Et on documente la variable Principal_Format_fichier avec la valeur:
        Principal_Format_fichier=format+1;

        // Correction des dimensions
        if (Principal_Largeur_image<1)
          Principal_Largeur_image=1;
        if (Principal_Hauteur_image<1)
          Principal_Hauteur_image=1;
      }
      else if (Erreur_fichier!=1)
      {
        // On considère que l'image chargée est encore modifiée
        Principal_Image_modifiee=1;
        // Et on documente la variable Principal_Format_fichier avec la valeur:
        Principal_Format_fichier=format+1;
      }
      else
      {
        // Dans ce cas, on sait que l'image n'a pas changé, mais ses
        // paramètres (dimension, palette, ...) si. Donc on les restaures.
        Download_infos_page_principal(Principal_Backups->Pages);
      }
    }
  }
  else
    // Sinon, l'appelant sera au courant de l'échec grace à Erreur_fichier;
    // et si on s'apprêtait à faire un chargement définitif de l'image (pas
    // une preview), alors on flash l'utilisateur.
    if (Pixel_de_chargement!=Pixel_Chargement_dans_preview)
      Erreur(0);
}


// -- Sauver n'importe quel type connu de fichier d'image (ou palette) ------
void Sauver_image(byte image)
{
  // On place par défaut Erreur_fichier à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  Erreur_fichier=1;

  Lit_pixel_de_sauvegarde=(image)?Lit_pixel_dans_ecran_courant:Lit_pixel_dans_brosse;

  FormatFichier[Principal_Format_fichier-1].Save();

  if (Erreur_fichier)
    Erreur(0);
  else
  {
    if ((image) && (FormatFichier[Principal_Format_fichier-1].Backup_done))
      Principal_Image_modifiee=0;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PAL ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// -- Tester si un fichier est au format PAL --------------------------------
void Test_PAL(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  long Taille_du_fichier;   // Taille du fichier

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture de la taille du fichier
    Taille_du_fichier=File_length_file(file);
    fclose(file);
    // Le fichier ne peut être au format PAL que si sa taille vaut 768 octets
    if (Taille_du_fichier==sizeof(T_Palette))
      Erreur_fichier=0;
  }
}


// -- Lire un fichier au format PAL -----------------------------------------
void Load_PAL(void)
{
  FILE *file;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //long  Taille_du_fichier;   // Taille du fichier


  Nom_fichier_complet(Nom_du_fichier,0);
  Erreur_fichier=0;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    T_Palette Palette_64;
    // Initialiser_preview(???); // Pas possible... pas d'image...

    // Lecture du fichier dans Principal_Palette
    if (read_bytes(file,Palette_64,sizeof(T_Palette)))
    {
      Palette_64_to_256(Palette_64);
      memcpy(Principal_Palette,Palette_64,sizeof(T_Palette));
      Set_palette(Principal_Palette);
      Remapper_fileselect();

      // On dessine une preview de la palette (si chargement=preview)
      Dessiner_preview_palette();
    }
    else
      Erreur_fichier=2;

    // Fermeture du fichier
    fclose(file);
  }
  else
    // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PAL ---------------------------------------
void Save_PAL(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //long Taille_du_fichier;   // Taille du fichier

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    T_Palette Palette_64;
    memcpy(Palette_64,Principal_Palette,sizeof(T_Palette));
    Palette_256_to_64(Palette_64);
    // Enregistrement de Principal_Palette dans le fichier
    if (! write_bytes(file,Palette_64,sizeof(T_Palette)))
    {
      Erreur_fichier=1;
      fclose(file);
      remove(Nom_du_fichier);
    }
    else // Ecriture correcte => Fermeture normale du fichier
      fclose(file);
  }
  else // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
  {
    Erreur_fichier=1;
    fclose(file);
    remove(Nom_du_fichier);
                     //   On se fout du résultat de l'opération car si ça
                     // renvoie 0 c'est que le fichier avait été partiel-
                     // -lement écrit, sinon pas du tout. Or dans tous les
                     // cas ça revient au même pour nous: Sauvegarde ratée!
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// IMG ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  byte Filler1[6];
  word Width;
  word Height;
  byte Filler2[118];
  T_Palette Palette;
} __attribute__((__packed__)) T_Header_IMG;

// -- Tester si un fichier est au format IMG --------------------------------
void Test_IMG(void)
{
  FILE *file;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_IMG IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture et vérification de la signature
    if (read_bytes(file,&IMG_header,sizeof(T_Header_IMG)))
    {
      if ( (!memcmp(IMG_header.Filler1,signature,6))
        && IMG_header.Width && IMG_header.Height)
        Erreur_fichier=0;
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format IMG -----------------------------------------
void Load_IMG(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  byte * Buffer;
  FILE *file;
  word x_pos,y_pos;
  long Largeur_lue;
  long Taille_du_fichier;
  T_Header_IMG IMG_header;

  Nom_fichier_complet(Nom_du_fichier,0);
  Erreur_fichier=0;

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);

    if (read_bytes(file,&IMG_header,sizeof(T_Header_IMG)))
    {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    IMG_header.Width = SDL_Swap16(IMG_header.Width);
    IMG_header.Height = SDL_Swap16(IMG_header.Height);
#endif

      Buffer=(byte *)malloc(IMG_header.Width);

      Initialiser_preview(IMG_header.Width,IMG_header.Height,Taille_du_fichier,FORMAT_IMG);
      if (Erreur_fichier==0)
      {
        memcpy(Principal_Palette,IMG_header.Palette,sizeof(T_Palette));
        Set_palette(Principal_Palette);
        Remapper_fileselect();

        Principal_Largeur_image=IMG_header.Width;
        Principal_Hauteur_image=IMG_header.Height;
        Largeur_lue=IMG_header.Width;

        for (y_pos=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);y_pos++)
        {
          if (read_bytes(file,Buffer,Principal_Largeur_image))
          {
            for (x_pos=0; x_pos<Principal_Largeur_image;x_pos++)
              Pixel_de_chargement(x_pos,y_pos,Buffer[x_pos]);
          }
          else
            Erreur_fichier=2;
        }
      }

      free(Buffer);
    }
    else
      Erreur_fichier=1;

    fclose(file);
  }
  else
    Erreur_fichier=1;
}

// -- Sauver un fichier au format IMG ---------------------------------------
void Save_IMG(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_Header_IMG IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    memcpy(IMG_header.Filler1,signature,6);

    IMG_header.Width=Principal_Largeur_image;
    IMG_header.Height=Principal_Hauteur_image;

    memset(IMG_header.Filler2,0,118);
    IMG_header.Filler2[4]=0xFF;
    IMG_header.Filler2[22]=64; // Lo(Longueur de la signature)
    IMG_header.Filler2[23]=0;  // Hi(Longueur de la signature)
    memcpy(IMG_header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

    memcpy(IMG_header.Palette,Principal_Palette,sizeof(T_Palette));

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    IMG_header.Width = SDL_Swap16(IMG_header.Width);
    IMG_header.Height = SDL_Swap16(IMG_header.Height);
#endif

    if (write_bytes(file,&IMG_header,sizeof(T_Header_IMG)))
    {
      Init_ecriture();

      for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
        for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
          Ecrire_octet(file,Lit_pixel_de_sauvegarde(x_pos,y_pos));

      Close_ecriture(file);
      fclose(file);

      if (Erreur_fichier)
        remove(Nom_du_fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
  }
  else
  {
    fclose(file);
    remove(Nom_du_fichier);
    Erreur_fichier=1;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PKM ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  char Ident[3]; // Chaîne "PKM" }
  byte Method;  // Compression method
                 //   0 = compression en ligne (c)KM
                 //   autres = inconnues pour le moment
  byte Recon1;   // Octet de reconnaissance sur 1 octet  }
  byte Recon2;   // Octet de reconnaissance sur 2 octets }
  word Width;  // width de l'image
  word Height;  // height de l'image
  T_Palette Palette; // Palette RVB 256*3
  word Jump;     // Taille du saut entre le header et l'image:
                 //   On va s'en servir pour rajouter un commentaire
} __attribute__((__packed__)) T_Header_PKM;

// -- Tester si un fichier est au format PKM --------------------------------
void Test_PKM(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_PKM header;


  Nom_fichier_complet(Nom_du_fichier,0);
  
  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture du header du fichier
    if (read_bytes(file,&header,sizeof(T_Header_PKM)))
    {
      // On regarde s'il y a la signature PKM suivie de la méthode 0.
      // La constante "PKM" étant un chaîne, elle se termine toujours par 0.
      // Donc pas la peine de s'emm...er à regarder si la méthode est à 0.
      if ( (!memcmp(&header,"PKM",4)) && header.Width && header.Height)
        Erreur_fichier=0;
    }
    fclose(file);
  }
}


// -- Lire un fichier au format PKM -----------------------------------------
void Load_PKM(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_PKM header;
  byte  Couleur;
  byte  temp_byte;
  word  len;
  word  Indice;
  dword Compteur_de_pixels;
  dword Compteur_de_donnees_packees;
  dword Taille_image;
  dword Taille_pack;
  long  Taille_du_fichier;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);

    if (read_bytes(file,&header,sizeof(T_Header_PKM)))
    {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Width = SDL_Swap16(header.Width);
    header.Height = SDL_Swap16(header.Height);
    header.Jump = SDL_Swap16(header.Jump);
#endif

      Principal_Commentaire[0]='\0'; // On efface le commentaire
      if (header.Jump)
      {
        Indice=0;
        while ( (Indice<header.Jump) && (!Erreur_fichier) )
        {
          if (read_byte(file,&temp_byte))
          {
            Indice+=2; // On rajoute le "Field-id" et "le Field-size" pas encore lu
            switch (temp_byte)
            {
              case 0 : // Commentaire
                if (read_byte(file,&temp_byte))
                {
                  if (temp_byte>TAILLE_COMMENTAIRE)
                  {
                    Couleur=temp_byte;              // On se sert de Couleur comme
                    temp_byte=TAILLE_COMMENTAIRE;   // variable temporaire
                    Couleur-=TAILLE_COMMENTAIRE;
                  }
                  else
                    Couleur=0;

                  if (read_bytes(file,Principal_Commentaire,temp_byte))
                  {
                    Indice+=temp_byte;
                    Principal_Commentaire[temp_byte]='\0';
                    if (Couleur)
                      if (fseek(file,Couleur,SEEK_CUR))
                        Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              case 1 : // Dimensions de l'écran d'origine
                if (read_byte(file,&temp_byte))
                {
                  if (temp_byte==4)
                  {
                    Indice+=4;
                    if ( ! read_word_le(file,(word *) &Ecran_original_X)
                      || !read_word_le(file,(word *) &Ecran_original_Y) )
                      Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              case 2 : // Couleur de transparence
                if (read_byte(file,&temp_byte))
                {
                  if (temp_byte==1)
                  {
                    Indice++;
                    if (! read_byte(file,&Back_color))
                      Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              default:
                if (read_byte(file,&temp_byte))
                {
                  Indice+=temp_byte;
                  if (fseek(file,temp_byte,SEEK_CUR))
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
            }
          }
          else
            Erreur_fichier=2;
        }
        if ( (!Erreur_fichier) && (Indice!=header.Jump) )
          Erreur_fichier=2;
      }

      /*Init_lecture();*/

      if (!Erreur_fichier)
      {
        Initialiser_preview(header.Width,header.Height,Taille_du_fichier,FORMAT_PKM);
        if (Erreur_fichier==0)
        {
          
          Principal_Largeur_image=header.Width;
          Principal_Hauteur_image=header.Height;
          Taille_image=(dword)(Principal_Largeur_image*Principal_Hauteur_image);
          // Palette lue en 64
          memcpy(Principal_Palette,header.Palette,sizeof(T_Palette));
          Palette_64_to_256(Principal_Palette);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Compteur_de_donnees_packees=0;
          Compteur_de_pixels=0;
          Taille_pack=(Taille_du_fichier)-sizeof(T_Header_PKM)-header.Jump;

          // Boucle de décompression:
          while ( (Compteur_de_pixels<Taille_image) && (Compteur_de_donnees_packees<Taille_pack) && (!Erreur_fichier) )
          {
            Lire_octet(file, &temp_byte);

            // Si ce n'est pas un octet de reconnaissance, c'est un pixel brut
            if ( (temp_byte!=header.Recon1) && (temp_byte!=header.Recon2) )
            {
              Pixel_de_chargement(Compteur_de_pixels % Principal_Largeur_image,
                                  Compteur_de_pixels / Principal_Largeur_image,
                                  temp_byte);
              Compteur_de_donnees_packees++;
              Compteur_de_pixels++;
            }
            else // Sinon, On regarde si on va décompacter un...
            { // ... nombre de pixels tenant sur un byte
              if (temp_byte==header.Recon1)
              {
                Lire_octet(file, &Couleur);
                Lire_octet(file, &temp_byte);
                for (Indice=0; Indice<temp_byte; Indice++)
                  Pixel_de_chargement((Compteur_de_pixels+Indice) % Principal_Largeur_image,
                                      (Compteur_de_pixels+Indice) / Principal_Largeur_image,
                                      Couleur);
                Compteur_de_pixels+=temp_byte;
                Compteur_de_donnees_packees+=3;
              }
              else // ... nombre de pixels tenant sur un word
              {
                Lire_octet(file, &Couleur);
                read_word_be(file, &len);
                for (Indice=0; Indice<len; Indice++)
                  Pixel_de_chargement((Compteur_de_pixels+Indice) % Principal_Largeur_image,
                                      (Compteur_de_pixels+Indice) / Principal_Largeur_image,
                                      Couleur);
                Compteur_de_pixels+=len;
                Compteur_de_donnees_packees+=4;
              }
            }
          }
        }
      }
      /*Close_lecture();*/
    }
    else // Lecture header impossible: Erreur ne modifiant pas l'image
      Erreur_fichier=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Erreur ne modifiant pas l'image
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PKM ---------------------------------------

  // Trouver quels sont les octets de reconnaissance
  void Trouver_recon(byte * Recon1, byte * Recon2)
  {
    dword Find_recon[256]; // Table d'utilisation de couleurs
    byte  Best;   // Meilleure couleur pour recon (recon1 puis recon2)
    dword NBest;  // Nombre d'occurences de cette couleur
    word  Indice;


    // On commence par compter l'utilisation de chaque couleurs
    Palette_Compter_nb_couleurs_utilisees(Find_recon);

    // Ensuite Recon1 devient celle la moins utilisée de celles-ci
    *Recon1=0;
    Best=1;
    NBest=INT_MAX; // Une même couleur ne pourra jamais être utilisée 1M de fois.
    for (Indice=1;Indice<=255;Indice++)
      if (Find_recon[Indice]<NBest)
      {
        Best=Indice;
        NBest=Find_recon[Indice];
      }
    *Recon1=Best;

    // Enfin Recon2 devient la 2ème moins utilisée
    *Recon2=0;
    Best=0;
    NBest=INT_MAX;
    for (Indice=0;Indice<=255;Indice++)
      if ( (Find_recon[Indice]<NBest) && (Indice!=*Recon1) )
      {
        Best=Indice;
        NBest=Find_recon[Indice];
      }
    *Recon2=Best;
  }


void Save_PKM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_Header_PKM header;
  dword Compteur_de_pixels;
  dword Taille_image;
  word  Compteur_de_repetitions;
  byte  Derniere_couleur;
  byte  Valeur_pixel;
  byte  Taille_commentaire;



  // Construction du header
  memcpy(header.Ident,"PKM",3);
  header.Method=0;
  Trouver_recon(&header.Recon1,&header.Recon2);
  header.Width=Principal_Largeur_image;
  header.Height=Principal_Hauteur_image;
  memcpy(header.Palette,Principal_Palette,sizeof(T_Palette));
  Palette_256_to_64(header.Palette);

  // Calcul de la taille du Post-header
  header.Jump=9; // 6 pour les dimensions de l'ecran + 3 pour la back-color
  Taille_commentaire=strlen(Principal_Commentaire);
  if (Taille_commentaire)
    header.Jump+=Taille_commentaire+2;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Width = SDL_Swap16(header.Width);
    header.Height = SDL_Swap16(header.Height);
    header.Jump = SDL_Swap16(header.Jump);
#endif

    // Ecriture du header
    if (write_bytes(file,&header,sizeof(T_Header_PKM)))
    {
      Init_ecriture();

      // Ecriture du commentaire
      // (Compteur_de_pixels est utilisé ici comme simple index de comptage)
      if (Taille_commentaire)
      {
        Ecrire_octet(file,0);
        Ecrire_octet(file,Taille_commentaire);
        for (Compteur_de_pixels=0; Compteur_de_pixels<Taille_commentaire; Compteur_de_pixels++)
          Ecrire_octet(file,Principal_Commentaire[Compteur_de_pixels]);
      }
      // Ecriture des dimensions de l'écran
      Ecrire_octet(file,1);
      Ecrire_octet(file,4);
      Ecrire_octet(file,Largeur_ecran&0xFF);
      Ecrire_octet(file,Largeur_ecran>>8);
      Ecrire_octet(file,Hauteur_ecran&0xFF);
      Ecrire_octet(file,Hauteur_ecran>>8);
      // Ecriture de la back-color
      Ecrire_octet(file,2);
      Ecrire_octet(file,1);
      Ecrire_octet(file,Back_color);

      // Routine de compression PKM de l'image
      Taille_image=(dword)(Principal_Largeur_image*Principal_Hauteur_image);
      Compteur_de_pixels=0;
      Valeur_pixel=Lit_pixel_de_sauvegarde(0,0);

      while ( (Compteur_de_pixels<Taille_image) && (!Erreur_fichier) )
      {
        Compteur_de_pixels++;
        Compteur_de_repetitions=1;
        Derniere_couleur=Valeur_pixel;
        if(Compteur_de_pixels<Taille_image)
        {
          Valeur_pixel=Lit_pixel_de_sauvegarde(Compteur_de_pixels % Principal_Largeur_image,Compteur_de_pixels / Principal_Largeur_image);
        }
        while ( (Valeur_pixel==Derniere_couleur)
             && (Compteur_de_pixels<Taille_image)
             && (Compteur_de_repetitions<65535) )
        {
          Compteur_de_pixels++;
          Compteur_de_repetitions++;
          if(Compteur_de_pixels>=Taille_image) break;
          Valeur_pixel=Lit_pixel_de_sauvegarde(Compteur_de_pixels % Principal_Largeur_image,Compteur_de_pixels / Principal_Largeur_image);
        }

        if ( (Derniere_couleur!=header.Recon1) && (Derniere_couleur!=header.Recon2) )
        {
          if (Compteur_de_repetitions==1)
            Ecrire_octet(file,Derniere_couleur);
          else
          if (Compteur_de_repetitions==2)
          {
            Ecrire_octet(file,Derniere_couleur);
            Ecrire_octet(file,Derniere_couleur);
          }
          else
          if ( (Compteur_de_repetitions>2) && (Compteur_de_repetitions<256) )
          { // RECON1/couleur/nombre
            Ecrire_octet(file,header.Recon1);
            Ecrire_octet(file,Derniere_couleur);
            Ecrire_octet(file,Compteur_de_repetitions&0xFF);
          }
          else
          if (Compteur_de_repetitions>=256)
          { // RECON2/couleur/hi(nombre)/lo(nombre)
            Ecrire_octet(file,header.Recon2);
            Ecrire_octet(file,Derniere_couleur);
            Ecrire_octet(file,Compteur_de_repetitions>>8);
            Ecrire_octet(file,Compteur_de_repetitions&0xFF);
          }
        }
        else
        {
          if (Compteur_de_repetitions<256)
          {
            Ecrire_octet(file,header.Recon1);
            Ecrire_octet(file,Derniere_couleur);
            Ecrire_octet(file,Compteur_de_repetitions&0xFF);
          }
          else
          {
            Ecrire_octet(file,header.Recon2);
            Ecrire_octet(file,Derniere_couleur);
            Ecrire_octet(file,Compteur_de_repetitions>>8);
            Ecrire_octet(file,Compteur_de_repetitions&0xFF);
          }
        }
      }

      Close_ecriture(file);
    }
    else
      Erreur_fichier=1;
    fclose(file);
  }
  else
  {
    Erreur_fichier=1;
    fclose(file);
  }
  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri traîner... Ca fait pas propre.
  if (Erreur_fichier)
    remove(Nom_du_fichier);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LBM ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word  Width;
  word  Height;
  word  Xorg;       // Inutile
  word  Yorg;       // Inutile
  byte  BitPlanes;
  byte  Mask;
  byte  Compression;
  byte  Pad1;       // Inutile
  word  Transp_col;
  byte  Xaspect;    // Inutile
  byte  Yaspect;    // Inutile
  word  Xscreen;
  word  Yscreen;
} __attribute__((__packed__)) T_Header_LBM;

byte * LBM_Buffer;
FILE *LBM_Fichier;

// -- Tester si un fichier est au format LBM --------------------------------

void Test_LBM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  char  format[4];
  char  section[4];
  dword Dummy;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((LBM_Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (! read_bytes(LBM_Fichier,section,4))
      Erreur_fichier=1;
    else
    if (memcmp(section,"FORM",4))
      Erreur_fichier=1;
    else
    {
      read_dword_be(LBM_Fichier, &Dummy);
                   //   On aurait pu vérifier que ce long est égal à la taille
                   // du fichier - 8, mais ça aurait interdit de charger des
                   // fichiers tronqués (et déjà que c'est chiant de perdre
                   // une partie du fichier il faut quand même pouvoir en
                   // garder un peu... Sinon, moi je pleure :'( !!! )
      if (! read_bytes(LBM_Fichier,format,4))
        Erreur_fichier=1;
      else
      if ( (memcmp(format,"ILBM",4)) && (memcmp(format,"PBM ",4)) )
        Erreur_fichier=1;
    }
    fclose(LBM_Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Lire un fichier au format LBM -----------------------------------------

  byte Image_HAM;

  // ---------------- Adapter la palette pour les images HAM ----------------
  void Adapter_Palette_HAM(void)
  {
    short i,j,Temp;
    byte  Couleur;

    if (Image_HAM==6)
    {
      for (i=1; i<=14; i++)
      {
        // On recopie a palette de base
        memcpy(Principal_Palette+(i<<4),Principal_Palette,48);
        // On modifie les teintes de cette palette
        for (j=0; j<16; j++)
        {
          Couleur=(i<<4)+j;
          if (i<=7)
          {
            if (i&1)
            {
              Temp=Principal_Palette[j].R+16;
              Principal_Palette[Couleur].R=(Temp<63)?Temp:63;
            }
            if (i&2)
            {
              Temp=Principal_Palette[j].G+16;
              Principal_Palette[Couleur].G=(Temp<63)?Temp:63;
            }
            if (i&4)
            {
              Temp=Principal_Palette[j].B+16;
              Principal_Palette[Couleur].B=(Temp<63)?Temp:63;
            }
          }
          else
          {
            if ((i-7)&1)
            {
              Temp=Principal_Palette[j].R-16;
              Principal_Palette[Couleur].R=(Temp>=0)?Temp:0;
            }
            if ((i-7)&2)
            {
              Temp=Principal_Palette[j].G-16;
              Principal_Palette[Couleur].G=(Temp>=0)?Temp:0;
            }
            if ((i-7)&4)
            {
              Temp=Principal_Palette[j].B-16;
              Principal_Palette[Couleur].B=(Temp>=0)?Temp:0;
            }
          }
        }
      }
      // Ici, il reste les 16 dernières couleurs à modifier
      for (i=240,j=0; j<16; i++,j++)
      {
        Temp=Principal_Palette[j].R+8;
        Principal_Palette[i].R=(Temp<63)?Temp:63;
        Temp=Principal_Palette[j].G+8;
        Principal_Palette[i].G=(Temp<63)?Temp:63;
        Temp=Principal_Palette[j].B+8;
        Principal_Palette[i].B=(Temp<63)?Temp:63;
      }
    }
    else if (Image_HAM==8)
    {
      for (i=1; i<=3; i++)
      {
        // On recopie la palette de base
        memcpy(Principal_Palette+(i<<6),Principal_Palette,192);
        // On modifie les teintes de cette palette
        for (j=0; j<64; j++)
        {
          Couleur=(i<<6)+j;
          switch (i)
          {
            case 1 :
              Temp=Principal_Palette[j].R+16;
              Principal_Palette[Couleur].R=(Temp<63)?Temp:63;
              break;
            case 2 :
              Temp=Principal_Palette[j].G+16;
              Principal_Palette[Couleur].G=(Temp<63)?Temp:63;
              break;
            default:
              Temp=Principal_Palette[j].B+16;
              Principal_Palette[Couleur].B=(Temp<63)?Temp:63;
          }
        }
      }
    }
    else // Image 64 couleurs sauvée en 32.
    {
      for (i=0; i<32; i++)
      {
        j=i+32;
        Principal_Palette[j].R=Principal_Palette[i].R>>1;
        Principal_Palette[j].G=Principal_Palette[i].G>>1;
        Principal_Palette[j].B=Principal_Palette[i].B>>1;
      }
    }
  }

  // ------------------------- Attendre une section -------------------------
  byte Wait_for(byte * Section_attendue)
  {
    // Valeur retournée: 1=Section trouvée, 0=Section non trouvée (erreur)
    dword Taille_section;
    byte Section_lue[4];

    if (! read_bytes(LBM_Fichier,Section_lue,4))
      return 0;
    while (memcmp(Section_lue,Section_attendue,4)) // Sect. pas encore trouvée
    {
      if (!read_dword_be(LBM_Fichier,&Taille_section))
        return 0;
      if (Taille_section&1)
        Taille_section++;
      if (fseek(LBM_Fichier,Taille_section,SEEK_CUR))
        return 0;
      if (! read_bytes(LBM_Fichier,Section_lue,4))
        return 0;
    }
    return 1;
  }

// Les images ILBM sont stockés en bitplanes donc on doit trifouiller les bits pour
// en faire du chunky

byte Couleur_ILBM_line(word x_pos, word Vraie_taille_ligne, byte HBPm1)
{
  // Renvoie la couleur du pixel (ILBM) en x_pos.
  // CL sera le rang auquel on extrait les bits de la couleur
  byte cl = 7 - (x_pos & 7);
  int ax,bh,dx;
  byte bl=0;

  for(dx=HBPm1;dx>=0;dx--)
  {
  //CIL_Loop
    ax = (Vraie_taille_ligne * dx + x_pos) >> 3;
    bh = (LBM_Buffer[ax] >> cl) & 1;

    bl = (bl << 1) + bh;
  }

  return bl;
}

  // ----------------------- Afficher une ligne ILBM ------------------------
  void Draw_ILBM_line(short y_pos, short Vraie_taille_ligne)
  {
    byte  Couleur;
    byte  Rouge,Vert,Bleu;
    byte  Temp;
    short x_pos;

    if (Image_HAM<=1)                                               // ILBM
    {
      for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
      {
        Pixel_de_chargement(x_pos,y_pos,Couleur_ILBM_line(x_pos,Vraie_taille_ligne, HBPm1));
      }
    }
    else
    {
      Couleur=0;
      Rouge=Principal_Palette[0].R;
      Vert =Principal_Palette[0].G;
      Bleu =Principal_Palette[0].B;
      if (Image_HAM==6)
      for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)         // HAM6
      {
        Temp=Couleur_ILBM_line(x_pos,Vraie_taille_ligne, HBPm1);
        switch (Temp & 0xF0)
        {
          case 0x10: // Bleu
            Bleu=(Temp&0x0F)<<2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          case 0x20: // Rouge
            Rouge=(Temp&0x0F)<<2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          case 0x30: // Vert
            Vert=(Temp&0x0F)<<2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          default:   // Nouvelle couleur
            Couleur=Temp;
            Rouge=Principal_Palette[Couleur].R;
            Vert =Principal_Palette[Couleur].G;
            Bleu =Principal_Palette[Couleur].B;
        }
        Pixel_de_chargement(x_pos,y_pos,Couleur);
      }
      else
      for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)         // HAM8
      {
        Temp=Couleur_ILBM_line(x_pos,Vraie_taille_ligne, HBPm1);
        switch (Temp & 0x03)
        {
          case 0x01: // Bleu
            Bleu=Temp>>2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          case 0x02: // Rouge
            Rouge=Temp>>2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          case 0x03: // Vert
            Vert=Temp>>2;
            Couleur=Meilleure_couleur(Rouge,Vert,Bleu);
            break;
          default:   // Nouvelle couleur
            Couleur=Temp;
            Rouge=Principal_Palette[Couleur].R;
            Vert =Principal_Palette[Couleur].G;
            Bleu =Principal_Palette[Couleur].B;
        }
        Pixel_de_chargement(x_pos,y_pos,Couleur);
      }
    }
  }


void Load_LBM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  T_Header_LBM header;
  char  format[4];
  char  section[4];
  byte  temp_byte;
  short B256;
  dword Nb_couleurs;
  dword Taille_image;
  short x_pos;
  short y_pos;
  short counter;
  short Taille_ligne;       // Taille d'une ligne en octets
  short Vraie_taille_ligne; // Taille d'une ligne en pixels
  byte  Couleur;
  long  Taille_du_fichier;
  dword Dummy;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((LBM_Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(LBM_Fichier);

    // On avance dans le fichier (pas besoin de tester ce qui l'a déjà été)
    read_bytes(LBM_Fichier,section,4);
    read_dword_be(LBM_Fichier,&Dummy);
    read_bytes(LBM_Fichier,format,4);
    if (!Wait_for((byte *)"BMHD"))
      Erreur_fichier=1;
    read_dword_be(LBM_Fichier,&Dummy);

    // Maintenant on lit le header pour pouvoir commencer le chargement de l'image
    if ( (read_word_be(LBM_Fichier,&header.Width))
      && (read_word_be(LBM_Fichier,&header.Height))
      && (read_word_be(LBM_Fichier,&header.Xorg))
      && (read_word_be(LBM_Fichier,&header.Yorg))
      && (read_byte(LBM_Fichier,&header.BitPlanes))
      && (read_byte(LBM_Fichier,&header.Mask))
      && (read_byte(LBM_Fichier,&header.Compression))
      && (read_byte(LBM_Fichier,&header.Pad1))
      && (read_word_be(LBM_Fichier,&header.Transp_col))
      && (read_byte(LBM_Fichier,&header.Xaspect))
      && (read_byte(LBM_Fichier,&header.Yaspect))
      && (read_word_be(LBM_Fichier,&header.Xscreen))
      && (read_word_be(LBM_Fichier,&header.Yscreen))
      && header.Width && header.Height)
    {
      if ( (header.BitPlanes) && (Wait_for((byte *)"CMAP")) )
      {
        read_dword_be(LBM_Fichier,&Nb_couleurs);
        Nb_couleurs/=3;

        if (((dword)1<<header.BitPlanes)!=Nb_couleurs)
        {
          if ((Nb_couleurs==32) && (header.BitPlanes==6))
          {              // Ce n'est pas une image HAM mais une image 64 coul.
            Image_HAM=1; // Sauvée en 32 coul. => il faut copier les 32 coul.
          }              // sur les 32 suivantes et assombrir ces dernières.
          else
          {
            if ((header.BitPlanes==6) || (header.BitPlanes==8))
              Image_HAM=header.BitPlanes;
            else
              /* Erreur_fichier=1;*/  /* C'est censé être incorrect mais j'ai */
              Image_HAM=0;            /* trouvé un fichier comme ça, alors... */
          }
        }
        else
          Image_HAM=0;

        if ( (!Erreur_fichier) && (Nb_couleurs>=2) && (Nb_couleurs<=256) )
        {
          HBPm1=header.BitPlanes-1;
          if (header.Mask==1)
            header.BitPlanes++;

          // Deluxe paint le fait... alors on le fait...
          Back_color=header.Transp_col;

          // On commence par passer la palette en 256 comme ça, si la nouvelle
          // palette a moins de 256 coul, la précédente ne souffrira pas d'un
          // assombrissement préjudiciable.
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          else
            Palette_64_to_256(Principal_Palette);
          //   On peut maintenant charger la nouvelle palette
          if (read_bytes(LBM_Fichier,Principal_Palette,3*Nb_couleurs))
          {
            Palette_256_to_64(Principal_Palette);
            if (Image_HAM)
              Adapter_Palette_HAM();
            Palette_64_to_256(Principal_Palette);
            Set_palette(Principal_Palette);
            Remapper_fileselect();

            // On lit l'octet de padding du CMAP si la taille est impaire
            if (Nb_couleurs&1)
              if (read_byte(LBM_Fichier,&temp_byte))
                Erreur_fichier=2;

            if ( (Wait_for((byte *)"BODY")) && (!Erreur_fichier) )
            {
              read_dword_be(LBM_Fichier,&Taille_image);
              //swab((char *)&header.Width ,(char *)&Principal_Largeur_image,2);
              //swab((char *)&header.Height,(char *)&Principal_Hauteur_image,2);
              Principal_Largeur_image = header.Width;
              Principal_Hauteur_image = header.Height;

              //swab((char *)&header.Xscreen,(char *)&Ecran_original_X,2);
              //swab((char *)&header.Yscreen,(char *)&Ecran_original_Y,2);
              Ecran_original_X = header.Xscreen;
              Ecran_original_Y = header.Yscreen;

              Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_LBM);
              if (Erreur_fichier==0)
              {
                if (!memcmp(format,"ILBM",4))    // "ILBM": InterLeaved BitMap
                {
                  // Calcul de la taille d'une ligne ILBM (pour les images ayant des dimensions exotiques)
                  if (Principal_Largeur_image & 15)
                  {
                    Vraie_taille_ligne=( (Principal_Largeur_image+16) >> 4 ) << 4;
                    Taille_ligne=( (Principal_Largeur_image+16) >> 4 )*(header.BitPlanes<<1);
                  }
                  else
                  {
                    Vraie_taille_ligne=Principal_Largeur_image;
                    Taille_ligne=(Principal_Largeur_image>>3)*header.BitPlanes;
                  }

                  if (!header.Compression)
                  {                                           // non compressé
                    LBM_Buffer=(byte *)malloc(Taille_ligne);
                    for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
                    {
                      if (read_bytes(LBM_Fichier,LBM_Buffer,Taille_ligne))
                        Draw_ILBM_line(y_pos,Vraie_taille_ligne);
                      else
                        Erreur_fichier=2;
                    }
                    free(LBM_Buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/

                    LBM_Buffer=(byte *)malloc(Taille_ligne);

                    for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
                    {
                      for (x_pos=0; ((x_pos<Taille_ligne) && (!Erreur_fichier)); )
                      {
                        Lire_octet(LBM_Fichier, &temp_byte);
                        // Si temp_byte > 127 alors il faut répéter 256-'temp_byte' fois la couleur de l'octet suivant
                        // Si temp_byte <= 127 alors il faut afficher directement les 'temp_byte' octets suivants
                        if (temp_byte>127)
                        {
                          Lire_octet(LBM_Fichier, &Couleur);
                          B256=(short)(256-temp_byte);
                          for (counter=0; counter<=B256; counter++)
                            if (x_pos<Taille_ligne)
                              LBM_Buffer[x_pos++]=Couleur;
                            else
                              Erreur_fichier=2;
                        }
                        else
                          for (counter=0; counter<=(short)(temp_byte); counter++)
                            if (x_pos<Taille_ligne)
                              Lire_octet(LBM_Fichier, &(LBM_Buffer[x_pos++]));
                            else
                              Erreur_fichier=2;
                      }
                      if (!Erreur_fichier)
                        Draw_ILBM_line(y_pos,Vraie_taille_ligne);
                    }

                    free(LBM_Buffer);
                    /*Close_lecture();*/
                  }
                }
                else                               // "PBM ": Planar(?) BitMap
                {
                  Vraie_taille_ligne=Principal_Largeur_image+(Principal_Largeur_image&1);

                  if (!header.Compression)
                  {                                           // non compressé
                    LBM_Buffer=(byte *)malloc(Vraie_taille_ligne);
                    for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
                    {
                      if (read_bytes(LBM_Fichier,LBM_Buffer,Vraie_taille_ligne))
                        for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
                          Pixel_de_chargement(x_pos,y_pos,LBM_Buffer[x_pos]);
                      else
                        Erreur_fichier=2;
                    }
                    free(LBM_Buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/
                    for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
                    {
                      for (x_pos=0; ((x_pos<Vraie_taille_ligne) && (!Erreur_fichier)); )
                      {
                        Lire_octet(LBM_Fichier, &temp_byte);
                        if (temp_byte>127)
                        {
                          Lire_octet(LBM_Fichier, &Couleur);
                          B256=256-temp_byte;
                          for (counter=0; counter<=B256; counter++)
                            Pixel_de_chargement(x_pos++,y_pos,Couleur);
                        }
                        else
                          for (counter=0; counter<=temp_byte; counter++)
                          {
                            byte Lu=0;
                            Lire_octet(LBM_Fichier, &Lu);
                            Pixel_de_chargement(x_pos++,y_pos,Lu);
                          }
                      }
                    }
                    /*Close_lecture();*/
                  }
                }
              }
            }
            else
              Modif_Erreur_fichier(2);
          }
          else
          {
            Erreur_fichier=1;
          }
        }
        else
          Modif_Erreur_fichier(1);
      }
      else
        Erreur_fichier=1;
    }
    else
      Erreur_fichier=1;

    fclose(LBM_Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format LBM ---------------------------------------

  byte LBM_File_de_couleurs[129];
  word LBM_Taille_de_file;
  byte LBM_Mode_repetition;

  // ------------- Ecrire les couleurs que l'on vient de traiter ------------
  void Transferer_couleurs(void)
  {
    byte Indice;

    if (LBM_Taille_de_file>0)
    {
      if (LBM_Mode_repetition)
      {
        Ecrire_octet(LBM_Fichier,257-LBM_Taille_de_file);
        Ecrire_octet(LBM_Fichier,LBM_File_de_couleurs[0]);
      }
      else
      {
        Ecrire_octet(LBM_Fichier,LBM_Taille_de_file-1);
        for (Indice=0; Indice<LBM_Taille_de_file; Indice++)
          Ecrire_octet(LBM_Fichier,LBM_File_de_couleurs[Indice]);
      }
    }
    LBM_Taille_de_file=0;
    LBM_Mode_repetition=0;
  }

  // - Compresion des couleurs encore plus performante que DP2e et que VPIC -
  void Nouvelle_couleur(byte Couleur)
  {
    byte Derniere_couleur;
    byte Avant_derniere_couleur;

    switch (LBM_Taille_de_file)
    {
      case 0 : // Première couleur
        LBM_File_de_couleurs[0]=Couleur;
        LBM_Taille_de_file=1;
        break;
      case 1 : // Deuxième couleur
        Derniere_couleur=LBM_File_de_couleurs[0];
        LBM_Mode_repetition=(Derniere_couleur==Couleur);
        LBM_File_de_couleurs[1]=Couleur;
        LBM_Taille_de_file=2;
        break;
      default: // Couleurs suivantes
        Derniere_couleur      =LBM_File_de_couleurs[LBM_Taille_de_file-1];
        Avant_derniere_couleur=LBM_File_de_couleurs[LBM_Taille_de_file-2];
        if (Derniere_couleur==Couleur)  // On a une répétition de couleur
        {
          if ( (LBM_Mode_repetition) || (Avant_derniere_couleur!=Couleur) )
          // On conserve le mode...
          {
            LBM_File_de_couleurs[LBM_Taille_de_file]=Couleur;
            LBM_Taille_de_file++;
            if (LBM_Taille_de_file==128)
              Transferer_couleurs();
          }
          else // On est en mode <> et on a 3 couleurs qui se suivent
          {
            LBM_Taille_de_file-=2;
            Transferer_couleurs();
            LBM_File_de_couleurs[0]=Couleur;
            LBM_File_de_couleurs[1]=Couleur;
            LBM_File_de_couleurs[2]=Couleur;
            LBM_Taille_de_file=3;
            LBM_Mode_repetition=1;
          }
        }
        else // La couleur n'est pas la même que la précédente
        {
          if (!LBM_Mode_repetition)                 // On conserve le mode...
          {
            LBM_File_de_couleurs[LBM_Taille_de_file++]=Couleur;
            if (LBM_Taille_de_file==128)
              Transferer_couleurs();
          }
          else                                        // On change de mode...
          {
            Transferer_couleurs();
            LBM_File_de_couleurs[LBM_Taille_de_file]=Couleur;
            LBM_Taille_de_file++;
          }
        }
    }
  }


void Save_LBM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  T_Header_LBM header;
  word x_pos;
  word y_pos;
  byte temp_byte;
  word Vraie_largeur;
  int Taille_fichier;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);

  // Ouverture du fichier
  if ((LBM_Fichier=fopen(Nom_du_fichier,"wb")))
  {
    write_bytes(LBM_Fichier,"FORM",4);
    write_dword_be(LBM_Fichier,0); // On mettra la taille à jour à la fin

    write_bytes(LBM_Fichier,"PBM BMHD",8);
    write_dword_be(LBM_Fichier,20);

    // On corrige la largeur de l'image pour qu'elle soit multiple de 2
    Vraie_largeur=Principal_Largeur_image+(Principal_Largeur_image&1);

    //swab((byte *)&Vraie_largeur,(byte *)&header.Width,2);
    header.Width=Principal_Largeur_image;
    header.Height=Principal_Hauteur_image;
    header.Xorg=0;
    header.Yorg=0;
    header.BitPlanes=8;
    header.Mask=0;
    header.Compression=1;
    header.Pad1=0;
    header.Transp_col=Back_color;
    header.Xaspect=1;
    header.Yaspect=1;
    header.Xscreen = Largeur_ecran;
    header.Yscreen = Hauteur_ecran;

    write_word_be(LBM_Fichier,header.Width);
    write_word_be(LBM_Fichier,header.Height);
    write_word_be(LBM_Fichier,header.Xorg);
    write_word_be(LBM_Fichier,header.Yorg);
    write_bytes(LBM_Fichier,&header.BitPlanes,1);
    write_bytes(LBM_Fichier,&header.Mask,1);
    write_bytes(LBM_Fichier,&header.Compression,1);
    write_bytes(LBM_Fichier,&header.Pad1,1);
    write_word_be(LBM_Fichier,header.Transp_col);
    write_bytes(LBM_Fichier,&header.Xaspect,1);
    write_bytes(LBM_Fichier,&header.Yaspect,1);
    write_word_be(LBM_Fichier,header.Xscreen);
    write_word_be(LBM_Fichier,header.Yscreen);

    write_bytes(LBM_Fichier,"CMAP",4);
    write_dword_be(LBM_Fichier,sizeof(T_Palette));

    write_bytes(LBM_Fichier,Principal_Palette,sizeof(T_Palette));

    write_bytes(LBM_Fichier,"BODY",4);
    write_dword_be(LBM_Fichier,0); // On mettra la taille à jour à la fin

    Init_ecriture();

    LBM_Taille_de_file=0;

    for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
    {
      for (x_pos=0; ((x_pos<Vraie_largeur) && (!Erreur_fichier)); x_pos++)
        Nouvelle_couleur(Lit_pixel_de_sauvegarde(x_pos,y_pos));

      if (!Erreur_fichier)
        Transferer_couleurs();
    }

    Close_ecriture(LBM_Fichier);
    fclose(LBM_Fichier);

    if (!Erreur_fichier)
    {
      Taille_fichier=FileLength(Nom_du_fichier);
      
      LBM_Fichier=fopen(Nom_du_fichier,"rb+");
      fseek(LBM_Fichier,820,SEEK_SET);
      write_dword_be(LBM_Fichier,Taille_fichier-824);

      if (!Erreur_fichier)
      {
        fseek(LBM_Fichier,4,SEEK_SET);

        //   Si la taille de la section de l'image (taille fichier-8) est
        // impaire, on rajoute un 0 (Padding) à la fin.
        if ((Taille_fichier) & 1)
        {
          write_dword_be(LBM_Fichier,Taille_fichier-7);
          fseek(LBM_Fichier,0,SEEK_END);
          temp_byte=0;
          if (! write_bytes(LBM_Fichier,&temp_byte,1))
            Erreur_fichier=1;
        }
        else
          write_dword_be(LBM_Fichier,Taille_fichier-8);

        fclose(LBM_Fichier);

        if (Erreur_fichier)
          remove(Nom_du_fichier);
      }
      else
      {
        Erreur_fichier=1;
        fclose(LBM_Fichier);
        remove(Nom_du_fichier);
      }
    }
    else // Il y a eu une erreur lors du compactage => on efface le fichier
      remove(Nom_du_fichier);
  }
  else
    Erreur_fichier=1;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// BMP ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

typedef struct
{
    word  Signature;   // ='BM' = 0x4D42
    dword Taille_1;    // =Taille du fichier
    word  Reserv_1;    // =0
    word  Reserv_2;    // =0
    dword Decalage;    // Nb octets avant les données bitmap

    dword Taille_2;    // =40 
    dword Width;
    dword Height;
    word  Plans;       // =1
    word  Nb_bits;     // =1,4,8 ou 24
    dword Compression;
    dword Taille_3;
    dword XPM;
    dword YPM;
    dword Nb_Clr;
    dword Clr_Imprt;
} __attribute__((__packed__)) T_BMP_Header;

// -- Tester si un fichier est au format BMP --------------------------------
void Test_BMP(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_BMP_Header header;

  Erreur_fichier=1;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (read_bytes(file,&(header.Signature),2) // "BM"
     && read_dword_le(file,&(header.Taille_1))
     && read_word_le(file,&(header.Reserv_1))
     && read_word_le(file,&(header.Reserv_2))
     && read_dword_le(file,&(header.Decalage))
     && read_dword_le(file,&(header.Taille_2))
     && read_dword_le(file,&(header.Width))
     && read_dword_le(file,&(header.Height))
     && read_word_le(file,&(header.Plans))
     && read_word_le(file,&(header.Nb_bits))
     && read_dword_le(file,&(header.Compression))
     && read_dword_le(file,&(header.Taille_3))
     && read_dword_le(file,&(header.XPM))
     && read_dword_le(file,&(header.YPM))
     && read_dword_le(file,&(header.Nb_Clr))
     && read_dword_le(file,&(header.Clr_Imprt))
        )
     {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      header.Signature = SDL_Swap16(header.Signature);
#endif

      if ( (header.Signature==0x4D42) && (header.Taille_2==40)
        && header.Width && header.Height )
        Erreur_fichier=0;
     }
    fclose(file);
  }
}


// -- Charger un fichier au format BMP --------------------------------------
void Load_BMP(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_BMP_Header header;
  byte * Buffer;
  word  Indice;
  byte  Palette_locale[256][4]; // R,G,B,0
  word  Nb_Couleurs =  0;
  short x_pos;
  short y_pos;
  word  Taille_ligne;
  byte  a,b,c=0;
  long  Taille_du_fichier;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);

    if (read_word_le(file,&(header.Signature))
     && read_dword_le(file,&(header.Taille_1))
     && read_word_le(file,&(header.Reserv_1))
     && read_word_le(file,&(header.Reserv_2))
     && read_dword_le(file,&(header.Decalage))
     && read_dword_le(file,&(header.Taille_2))
     && read_dword_le(file,&(header.Width))
     && read_dword_le(file,&(header.Height))
     && read_word_le(file,&(header.Plans))
     && read_word_le(file,&(header.Nb_bits))
     && read_dword_le(file,&(header.Compression))
     && read_dword_le(file,&(header.Taille_3))
     && read_dword_le(file,&(header.XPM))
     && read_dword_le(file,&(header.YPM))
     && read_dword_le(file,&(header.Nb_Clr))
     && read_dword_le(file,&(header.Clr_Imprt))
    )
    {
      switch (header.Nb_bits)
      {
        case 1 :
        case 4 :
        case 8 :
          if (header.Nb_Clr)
            Nb_Couleurs=header.Nb_Clr;
          else
            Nb_Couleurs=1<<header.Nb_bits;
          break;
        default:
          Erreur_fichier=1;
      }

      if (!Erreur_fichier)
      {
        Initialiser_preview(header.Width,header.Height,Taille_du_fichier,FORMAT_BMP);
        if (Erreur_fichier==0)
        {
          if (read_bytes(file,Palette_locale,Nb_Couleurs<<2))
          {
            //   On commence par passer la palette en 256 comme ça, si la nouvelle
            // palette a moins de 256 coul, la précédente ne souffrira pas d'un
            // assombrissement préjudiciable.
            if (Config.Clear_palette)
              memset(Principal_Palette,0,sizeof(T_Palette));
            //   On peut maintenant transférer la nouvelle palette
            for (Indice=0; Indice<Nb_Couleurs; Indice++)
            {
              Principal_Palette[Indice].R=Palette_locale[Indice][2];
              Principal_Palette[Indice].G=Palette_locale[Indice][1];
              Principal_Palette[Indice].B=Palette_locale[Indice][0];
            }
            Set_palette(Principal_Palette);
            Remapper_fileselect();

            Principal_Largeur_image=header.Width;
            Principal_Hauteur_image=header.Height;

            switch (header.Compression)
            {
              case 0 : // Pas de compression
                Taille_ligne=Principal_Largeur_image;
                x_pos=(32/header.Nb_bits); // x_pos sert de variable temporaire
                if (Taille_ligne % x_pos)
                  Taille_ligne=((Taille_ligne/x_pos)*x_pos)+x_pos;
                Taille_ligne=(Taille_ligne*header.Nb_bits)>>3;

                Buffer=(byte *)malloc(Taille_ligne);
                for (y_pos=Principal_Hauteur_image-1; ((y_pos>=0) && (!Erreur_fichier)); y_pos--)
                {
                  if (read_bytes(file,Buffer,Taille_ligne))
                    for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
                      switch (header.Nb_bits)
                      {
                        case 8 :
                          Pixel_de_chargement(x_pos,y_pos,Buffer[x_pos]);
                          break;
                        case 4 :
                          if (x_pos & 1)
                            Pixel_de_chargement(x_pos,y_pos,Buffer[x_pos>>1] & 0xF);
                          else
                            Pixel_de_chargement(x_pos,y_pos,Buffer[x_pos>>1] >> 4 );
                          break;
                        case 1 :
                          if ( Buffer[x_pos>>3] & (0x80>>(x_pos&7)) )
                            Pixel_de_chargement(x_pos,y_pos,1);
                          else
                            Pixel_de_chargement(x_pos,y_pos,0);
                      }
                  else
                    Erreur_fichier=2;
                }
                free(Buffer);
                break;

              case 1 : // Compression RLE 8 bits
                x_pos=0;
                y_pos=Principal_Hauteur_image-1;

                /*Init_lecture();*/
                Lire_octet(file, &a);
                Lire_octet(file, &b);
                while ( (!Erreur_fichier) && ((a)||(b!=1)) )
                {
                  if (a) // Encoded mode
                    for (Indice=1; Indice<=a; Indice++)
                      Pixel_de_chargement(x_pos++,y_pos,b);
                  else   // Absolute mode
                    switch (b)
                    {
                      case 0 : // End of line
                        x_pos=0;
                        y_pos--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                        Lire_octet(file, &a);
                        Lire_octet(file, &b);
                        x_pos+=a;
                        y_pos-=b;
                        break;
                      default: // Nouvelle série
                        while (b)
                        {
                          Lire_octet(file, &a);
                          Lire_octet(file, &c);
                          Pixel_de_chargement(x_pos++,y_pos,a);
                          if (--c)
                          {
                            Pixel_de_chargement(x_pos++,y_pos,c);
                            b--;
                          }
                        }
                    }
                  Lire_octet(file, &a);
                  Lire_octet(file, &b);
                }
                /*Close_lecture();*/
                break;

              case 2 : // Compression RLE 4 bits
                x_pos=0;
                y_pos=Principal_Hauteur_image-1;

                /*Init_lecture();*/
                Lire_octet(file, &a);
                Lire_octet(file, &b);
                while ( (!Erreur_fichier) && ((a)||(b!=1)) )
                {
                  if (a) // Encoded mode (A fois les 1/2 pixels de B)
                    for (Indice=1; Indice<=a; Indice++)
                    {
                      if (Indice & 1)
                        Pixel_de_chargement(x_pos,y_pos,b>>4);
                      else
                        Pixel_de_chargement(x_pos,y_pos,b&0xF);
                      x_pos++;
                    }
                  else   // Absolute mode
                    switch (b)
                    {
                      case 0 : //End of line
                        x_pos=0;
                        y_pos--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                        Lire_octet(file, &a);
                        Lire_octet(file, &b);
                        x_pos+=a;
                        y_pos-=b;
                        break;
                      default: // Nouvelle série (B 1/2 pixels bruts)
                        for (Indice=1; ((Indice<=b) && (!Erreur_fichier)); Indice++,x_pos++)
                        {
                          if (Indice&1)
                          {
                            Lire_octet(file, &c);
                            Pixel_de_chargement(x_pos,y_pos,c>>4);
                          }
                          else
                            Pixel_de_chargement(x_pos,y_pos,c&0xF);
                        }
                        //   On lit l'octet rendant le nombre d'octets pair, si
                        // nécessaire. Encore un truc de crétin "made in MS".
                        if ( ((b&3)==1) || ((b&3)==2) )
                        {
                          byte Dummy;
                          Lire_octet(file, &Dummy);
                        }
                    }
                  Lire_octet(file, &a);
                  Lire_octet(file, &b);
                }
                /*Close_lecture();*/
            }
            fclose(file);
          }
          else
          {
            fclose(file);
            Erreur_fichier=1;
          }
        }
      }
      else
      {
        // Image 24 bits!!!
        Erreur_fichier=0;

        Principal_Largeur_image=header.Width;
        Principal_Hauteur_image=header.Height;
        Initialiser_preview(header.Width,header.Height,Taille_du_fichier,FORMAT_BMP | FORMAT_24B);

        if (Erreur_fichier==0)
        {
          Taille_ligne=Principal_Largeur_image*3;
          x_pos=(Taille_ligne % 4); // x_pos sert de variable temporaire
          if (x_pos>0)
            Taille_ligne+=(4-x_pos);

          Buffer=(byte *)malloc(Taille_ligne);
          for (y_pos=Principal_Hauteur_image-1; ((y_pos>=0) && (!Erreur_fichier)); y_pos--)
          {
            if (read_bytes(file,Buffer,Taille_ligne))
              for (x_pos=0,Indice=0; x_pos<Principal_Largeur_image; x_pos++,Indice+=3)
                Pixel_Chargement_24b(x_pos,y_pos,Buffer[Indice+2],Buffer[Indice+1],Buffer[Indice+0]);
            else
              Erreur_fichier=2;
          }
          free(Buffer);
          fclose(file);
        }
      }
    }
    else
    {
      fclose(file);
      Erreur_fichier=1;
    }
  }
  else
    Erreur_fichier=1;
}


// -- Sauvegarder un fichier au format BMP ----------------------------------
void Save_BMP(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_BMP_Header header;
  short x_pos;
  short y_pos;
  long Taille_ligne;
  word Indice;
  byte Palette_locale[256][4]; // R,G,B,0


  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    if (Principal_Largeur_image & 7)
      Taille_ligne=((Principal_Largeur_image >> 3)+1) << 3;
    else
      Taille_ligne=Principal_Largeur_image;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Signature  = 0x424D;
#else
    header.Signature  = 0x4D42;
#endif
    header.Taille_1   =(Taille_ligne*Principal_Hauteur_image)+1078;
    header.Reserv_1   =0;
    header.Reserv_2   =0;
    header.Decalage   =1078;
    header.Taille_2   =40;
    header.Width    =Principal_Largeur_image;
    header.Height    =Principal_Hauteur_image;
    header.Plans      =1;
    header.Nb_bits    =8;
    header.Compression=0;
    header.Taille_3   =0;
    header.XPM        =0;
    header.YPM        =0;
    header.Nb_Clr     =0;
    header.Clr_Imprt  =0;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Taille_1   = SDL_Swap32( header.Taille_1 );
    header.Decalage   = SDL_Swap32( header.Decalage );
    header.Taille_2   = SDL_Swap32( header.Taille_2 );
    header.Width    = SDL_Swap32( header.Width );
    header.Height    = SDL_Swap32( header.Height );
    header.Plans      = SDL_Swap16( header.Plans );
    header.Nb_bits    = SDL_Swap16( header.Nb_bits );
    // If you ever set any more fields to non-zero, please swap here!
#endif

    if (write_bytes(file,&header,sizeof(T_BMP_Header)))
    {
      //   Chez Bill, ils ont dit: "On va mettre les couleur dans l'ordre
      // inverse, et pour faire chier, on va les mettre sur une échelle de
      // 0 à 255 parce que le standard VGA c'est de 0 à 63 (logique!). Et
      // puis comme c'est pas assez débile, on va aussi y rajouter un octet
      // toujours à 0 pour forcer les gens à s'acheter des gros disques
      // durs... Comme ça, ça fera passer la pillule lorsqu'on sortira
      // Windows 95." ...
      for (Indice=0; Indice<256; Indice++)
      {
        Palette_locale[Indice][0]=Principal_Palette[Indice].B;
        Palette_locale[Indice][1]=Principal_Palette[Indice].G;
        Palette_locale[Indice][2]=Principal_Palette[Indice].R;
        Palette_locale[Indice][3]=0;
      }

      if (write_bytes(file,Palette_locale,1024))
      {
        Init_ecriture();

        // ... Et Bill, il a dit: "OK les gars! Mais seulement si vous rangez
        // les pixels dans l'ordre inverse, mais que sur les Y quand-même
        // parce que faut pas pousser."
        for (y_pos=Principal_Hauteur_image-1; ((y_pos>=0) && (!Erreur_fichier)); y_pos--)
          for (x_pos=0; x_pos<Taille_ligne; x_pos++)
                Ecrire_octet(file,Lit_pixel_de_sauvegarde(x_pos,y_pos));

        Close_ecriture(file);
        fclose(file);

        if (Erreur_fichier)
          remove(Nom_du_fichier);
      }
      else
      {
        fclose(file);
        remove(Nom_du_fichier);
        Erreur_fichier=1;
      }

    }
    else
    {
      fclose(file);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
  }
  else
    Erreur_fichier=1;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// GIF ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word Width; // width de l'écran virtuel
  word Height; // height de l'écran virtuel
  byte Resol;   // Informations sur la résolution (et autres)
  byte Backcol; // Couleur de fond
  byte Aspect;  // Informations sur l'aspect ratio (et autres)
} __attribute__((__packed__)) T_LSDB; // Logical Screen Descriptor Block

typedef struct
{
  word Pos_X;         // Abscisse où devrait être affichée l'image
  word Pos_Y;         // Ordonnée où devrait être affichée l'image
  word Largeur_image; // width de l'image
  word Hauteur_image; // height de l'image
  byte Indicateur;    // Informations diverses sur l'image
  byte Nb_bits_pixel; // Nb de bits par pixel
} __attribute__((__packed__)) T_IDB; // Image Descriptor Block

// -- Tester si un fichier est au format GIF --------------------------------

void Test_GIF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  char signature[6];
  FILE *file;


  Erreur_fichier=1;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (
        (read_bytes(file,signature,6)) &&
        ((!memcmp(signature,"GIF87a",6))||(!memcmp(signature,"GIF89a",6)))
       )
      Erreur_fichier=0;

    fclose(file);
  }
}


// -- Lire un fichier au format GIF -----------------------------------------

  // Définition de quelques variables globales au chargement du GIF87a
  word GIF_Nb_bits;        // Nb de bits composants un code complet
  word GIF_Rest_bits;      // Nb de bits encore dispos dans GIF_Last_byte
  byte GIF_Rest_byte;      // Nb d'octets avant le prochain bloc de Raster Data
  word GIF_Code_actuel;    // Code traité (qui vient d'être lu en général)
  byte GIF_Last_byte;      // Octet de lecture des bits
  word GIF_Pos_X;          // Coordonnées d'affichage de l'image
  word GIF_Pos_Y;
  word GIF_Entrelacee;     // L'image est entrelacée
  word GIF_Image_entrelacee_terminee; // L'image entrelacée est finie de charger
  word GIF_Passe;          // Indice de passe de l'image entrelacée
  FILE *GIF_Fichier;        // L'handle du fichier

  // -- Lit le code à GIF_Nb_bits suivant --

  word GIF_Get_next_code(void)
  {
    word Nb_bits_a_traiter=GIF_Nb_bits;
    word Nb_bits_traites  =0;
    word Nb_bits_en_cours;

    GIF_Code_actuel=0;

    while (Nb_bits_a_traiter)
    {
      if (GIF_Rest_bits==0) // Il ne reste plus de bits...
      {
        // Lire l'octet suivant:

        // Si on a atteint la fin du bloc de Raster Data
        if (GIF_Rest_byte==0)
          // Lire l'octet nous donnant la taille du bloc de Raster Data suivant
          Lire_octet(GIF_Fichier, &GIF_Rest_byte);
        Lire_octet(GIF_Fichier,&GIF_Last_byte);
        GIF_Rest_byte--;
        GIF_Rest_bits=8;
      }

      Nb_bits_en_cours=(Nb_bits_a_traiter<=GIF_Rest_bits)?Nb_bits_a_traiter:GIF_Rest_bits;

      GIF_Code_actuel|=(GIF_Last_byte & ((1<<Nb_bits_en_cours)-1))<<Nb_bits_traites;
      GIF_Last_byte>>=Nb_bits_en_cours;
      Nb_bits_traites  +=Nb_bits_en_cours;
      Nb_bits_a_traiter-=Nb_bits_en_cours;
      GIF_Rest_bits    -=Nb_bits_en_cours;
    }

    return GIF_Code_actuel;
  }

  // -- Affiche un nouveau pixel --

  void GIF_Nouveau_pixel(byte Couleur)
  {
    Pixel_de_chargement(GIF_Pos_X,GIF_Pos_Y,Couleur);

    GIF_Pos_X++;

    if (GIF_Pos_X>=Principal_Largeur_image)
    {
      GIF_Pos_X=0;

      if (!GIF_Entrelacee)
        GIF_Pos_Y++;
      else
      {
        switch (GIF_Passe)
        {
          case 0 : GIF_Pos_Y+=8;
                   break;
          case 1 : GIF_Pos_Y+=8;
                   break;
          case 2 : GIF_Pos_Y+=4;
                   break;
          default: GIF_Pos_Y+=2;
        }

        if (GIF_Pos_Y>=Principal_Hauteur_image)
        {
          switch(++GIF_Passe)
          {
          case 1 : GIF_Pos_Y=4;
                   break;
          case 2 : GIF_Pos_Y=2;
                   break;
          case 3 : GIF_Pos_Y=1;
                   break;
          case 4 : GIF_Image_entrelacee_terminee=1;
          }
        }
      }
    }
  }


void Load_GIF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  char signature[6];

  word * Alphabet_Pile;     // Pile de décodage d'une chaîne
  word * Alphabet_Prefixe;  // Table des préfixes des codes
  word * Alphabet_Suffixe;  // Table des suffixes des codes
  word   Alphabet_Free;     // Position libre dans l'alphabet
  word   Alphabet_Max;      // Nombre d'entrées possibles dans l'alphabet
  word   Alphabet_Pos_pile; // Position dans la pile de décodage d'un chaîne

  T_LSDB LSDB;
  T_IDB IDB;

  word Nb_couleurs;       // Nombre de couleurs dans l'image
  word Indice_de_couleur; // Indice de traitement d'une couleur
  byte Taille_de_lecture; // Nombre de données à lire      (divers)
  byte Block_indicateur;  // Code indicateur du type de bloc en cours
  word Nb_bits_initial;   // Nb de bits au début du traitement LZW
  word Cas_special=0;       // Mémoire pour le cas spécial
  word Code_ancien=0;       // Code précédent
  word Octet_lu;         // Sauvegarde du code en cours de lecture
  word Valeur_Clr;        // Valeur <=> Clear tables
  word Valeur_Eof;        // Valeur <=> Fin d'image
  long Taille_du_fichier;
  int Nombre_LID; // Nombre d'images trouvées dans le fichier

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  GIF_Pos_X=0;
  GIF_Pos_Y=0;
  GIF_Last_byte=0;
  GIF_Rest_bits=0;
  GIF_Rest_byte=0;
  Nombre_LID=0;
  
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((GIF_Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(GIF_Fichier);
    if ( (read_bytes(GIF_Fichier,signature,6)) &&
         ( (memcmp(signature,"GIF87a",6)==0) ||
           (memcmp(signature,"GIF89a",6)==0) ) )
    {

      // Allocation de mémoire pour les tables & piles de traitement:
      Alphabet_Pile   =(word *)malloc(4096*sizeof(word));
      Alphabet_Prefixe=(word *)malloc(4096*sizeof(word));
      Alphabet_Suffixe=(word *)malloc(4096*sizeof(word));

      if (read_word_le(GIF_Fichier,&(LSDB.Width))
      && read_word_le(GIF_Fichier,&(LSDB.Height))
      && read_byte(GIF_Fichier,&(LSDB.Resol))
      && read_byte(GIF_Fichier,&(LSDB.Backcol))
      && read_byte(GIF_Fichier,&(LSDB.Aspect))
        )
      {
        // Lecture du Logical Screen Descriptor Block réussie:

        Ecran_original_X=LSDB.Width;
        Ecran_original_Y=LSDB.Height;

        // Palette globale dispo = (LSDB.Resol  and $80)
        // Profondeur de couleur =((LSDB.Resol  and $70) shr 4)+1
        // Nombre de bits/pixel  = (LSDB.Resol  and $07)+1
        // Ordre de Classement   = (LSDB.Aspect and $80)

        Alphabet_Pos_pile=0;
        GIF_Last_byte    =0;
        GIF_Rest_bits    =0;
        GIF_Rest_byte    =0;

        Nb_couleurs=(1 << ((LSDB.Resol & 0x07)+1));
        Nb_bits_initial=(LSDB.Resol & 0x07)+2;

        if (LSDB.Resol & 0x80)
        {
          // Palette globale dispo:

          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));

          //   On peut maintenant charger la nouvelle palette:
          if (!(LSDB.Aspect & 0x80))
            // Palette dans l'ordre:
            for(Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
            {
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].G);
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
            }
          else
          {
            // Palette triée par composantes:
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].G);
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
          }
          Set_palette(Principal_Palette);
        }

        // On lit un indicateur de block
        read_byte(GIF_Fichier,&Block_indicateur);
        while (Block_indicateur!=0x3B && !Erreur_fichier)
        {
          switch (Block_indicateur)
          {
            case 0x21: // Bloc d'extension
            {
              byte Code_Fonction;
              // Lecture du code de fonction:
              read_byte(GIF_Fichier,&Code_Fonction);   
              // Lecture de la taille du bloc:
              read_byte(GIF_Fichier,&Taille_de_lecture);
              while (Taille_de_lecture!=0 && !Erreur_fichier)
              {
                switch(Code_Fonction)
                {
                  case 0xFE: // Comment Block Extension
                    // On récupère le premier commentaire non-vide, 
                    // on jette les autres.
                    if (Principal_Commentaire[0]=='\0')
                    {
                      int Caracteres_a_garder=Min(Taille_de_lecture,TAILLE_COMMENTAIRE);
                      
                      read_bytes(GIF_Fichier,Principal_Commentaire,Caracteres_a_garder);
                      Principal_Commentaire[Caracteres_a_garder+1]='\0';
                      // Si le commentaire etait trop long, on fait avance-rapide
                      // sur la suite.
                      if (Taille_de_lecture>Caracteres_a_garder)
                        fseek(GIF_Fichier,Taille_de_lecture-Caracteres_a_garder,SEEK_CUR);
                    }
                    break;
                  case 0xF9: // Graphics Control Extension
                    // Prévu pour la transparence
                    
                  default:
                    // On saute le bloc:
                    fseek(GIF_Fichier,Taille_de_lecture,SEEK_CUR);
                    break;
                }
                // Lecture de la taille du bloc suivant:
                read_byte(GIF_Fichier,&Taille_de_lecture);
              }
            }
            break;
            case 0x2C: // Local Image Descriptor
            {
              // Si on a deja lu une image, c'est une GIF animée ou bizarroide, on sort.
              if (Nombre_LID!=0)
              {
                Erreur_fichier=2;
                break;
              }
              Nombre_LID++;
              
              // lecture de 10 derniers octets
              if ( read_word_le(GIF_Fichier,&(IDB.Pos_X))
                && read_word_le(GIF_Fichier,&(IDB.Pos_Y))
                && read_word_le(GIF_Fichier,&(IDB.Largeur_image))
                && read_word_le(GIF_Fichier,&(IDB.Hauteur_image))
                && read_byte(GIF_Fichier,&(IDB.Indicateur))
                && read_byte(GIF_Fichier,&(IDB.Nb_bits_pixel))
                && IDB.Largeur_image && IDB.Hauteur_image)
              {
                Principal_Largeur_image=IDB.Largeur_image;
                Principal_Hauteur_image=IDB.Hauteur_image;
    
                Initialiser_preview(IDB.Largeur_image,IDB.Hauteur_image,Taille_du_fichier,FORMAT_GIF);
    
                // Palette locale dispo = (IDB.Indicateur and $80)
                // Image entrelacée     = (IDB.Indicateur and $40)
                // Ordre de classement  = (IDB.Indicateur and $20)
                // Nombre de bits/pixel = (IDB.Indicateur and $07)+1 (si palette locale dispo)
    
                if (IDB.Indicateur & 0x80)
                {
                  // Palette locale dispo
    
                  Nb_couleurs=(1 << ((IDB.Indicateur & 0x07)+1));
                  Nb_bits_initial=(IDB.Indicateur & 0x07)+2;
    
                  if (!(IDB.Indicateur & 0x40))
                    // Palette dans l'ordre:
                    for(Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                    {   
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].G);
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
                    }
                  else
                  {
                    // Palette triée par composantes:
                    for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
                    for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].G);
                    for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                      read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
                  }
                  Set_palette(Principal_Palette);
                }
    
                Remapper_fileselect();
    
                Valeur_Clr   =Nb_couleurs+0;
                Valeur_Eof   =Nb_couleurs+1;
                Alphabet_Free=Nb_couleurs+2;
                GIF_Nb_bits  =Nb_bits_initial;
                Alphabet_Max      =((1 <<  GIF_Nb_bits)-1);
                GIF_Entrelacee    =(IDB.Indicateur & 0x40);
                GIF_Passe         =0;
    
                /*Init_lecture();*/
    
                Erreur_fichier=0;
                GIF_Image_entrelacee_terminee=0;
    
                //////////////////////////////////////////// DECOMPRESSION LZW //
    
                while ( (GIF_Get_next_code()!=Valeur_Eof) && (!Erreur_fichier) )
                {
                  if (GIF_Code_actuel<=Alphabet_Free)
                  {
                    if (GIF_Code_actuel!=Valeur_Clr)
                    {
                      if (Alphabet_Free==(Octet_lu=GIF_Code_actuel))
                      {
                        GIF_Code_actuel=Code_ancien;
                        Alphabet_Pile[Alphabet_Pos_pile++]=Cas_special;
                      }
    
                      while (GIF_Code_actuel>Valeur_Clr)
                      {
                        Alphabet_Pile[Alphabet_Pos_pile++]=Alphabet_Suffixe[GIF_Code_actuel];
                        GIF_Code_actuel=Alphabet_Prefixe[GIF_Code_actuel];
                      }
    
                      Cas_special=Alphabet_Pile[Alphabet_Pos_pile++]=GIF_Code_actuel;
    
                      do
                        GIF_Nouveau_pixel(Alphabet_Pile[--Alphabet_Pos_pile]);
                      while (Alphabet_Pos_pile!=0);
    
                      Alphabet_Prefixe[Alphabet_Free  ]=Code_ancien;
                      Alphabet_Suffixe[Alphabet_Free++]=GIF_Code_actuel;
                      Code_ancien=Octet_lu;
    
                      if (Alphabet_Free>Alphabet_Max)
                      {
                        if (GIF_Nb_bits<12)
                          Alphabet_Max      =((1 << (++GIF_Nb_bits))-1);
                      }
                    }
                    else // Code Clear rencontré
                    {
                      GIF_Nb_bits       =Nb_bits_initial;
                      Alphabet_Max      =((1 <<  GIF_Nb_bits)-1);
                      Alphabet_Free     =Nb_couleurs+2;
                      Cas_special       =GIF_Get_next_code();
                      Code_ancien       =GIF_Code_actuel;
                      GIF_Nouveau_pixel(GIF_Code_actuel);
                    }
                  }
                  else
                    Erreur_fichier=2;
                } // Code End-Of-Information ou erreur de fichier rencontré
    
                /*Close_lecture();*/
    
                if (Erreur_fichier>=0)
                if ( /* (GIF_Pos_X!=0) || */
                     ( ( (!GIF_Entrelacee) && (GIF_Pos_Y!=Principal_Hauteur_image) ) ||
                       (  (GIF_Entrelacee) && (!GIF_Image_entrelacee_terminee) )
                     ) )
                  Erreur_fichier=2;
              } // Le fichier contenait un IDB
              else
                Erreur_fichier=2;
            }
            default:
            break;
          }
          // Lecture du code de fonction suivant:
          read_byte(GIF_Fichier,&Block_indicateur);
        }
      } // Le fichier contenait un LSDB
      else
        Erreur_fichier=1;

      // Libération de la mémoire utilisée par les tables & piles de traitement:
      free(Alphabet_Suffixe);
      free(Alphabet_Prefixe);
      free(Alphabet_Pile);
    } // Le fichier contenait au moins la signature GIF87a ou GIF89a
    else
      Erreur_fichier=1;

    fclose(GIF_Fichier);

  } // Le fichier était ouvrable
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format GIF ---------------------------------------

  int  GIF_Arret;         // "On peut arrêter la sauvegarde du fichier"
  byte GIF_Buffer[256];   // Buffer d'écriture de bloc de données compilées

  // -- Vider le buffer GIF dans le buffer KM --

  void GIF_Vider_le_buffer(void)
  {
    word Indice;

    if (GIF_Rest_byte)
    {
      GIF_Buffer[0]=GIF_Rest_byte;

      for (Indice=0;Indice<=GIF_Rest_byte;Indice++)
        Ecrire_octet(GIF_Fichier,GIF_Buffer[Indice]);

      GIF_Rest_byte=0;
    }
  }

  // -- Ecrit un code à GIF_Nb_bits --

  void GIF_Set_code(word Code)
  {
    word Nb_bits_a_traiter=GIF_Nb_bits;
    word Nb_bits_traites  =0;
    word Nb_bits_en_cours;

    while (Nb_bits_a_traiter)
    {
      Nb_bits_en_cours=(Nb_bits_a_traiter<=(8-GIF_Rest_bits))?Nb_bits_a_traiter:(8-GIF_Rest_bits);

      GIF_Last_byte|=(Code & ((1<<Nb_bits_en_cours)-1))<<GIF_Rest_bits;
      Code>>=Nb_bits_en_cours;
      GIF_Rest_bits    +=Nb_bits_en_cours;
      Nb_bits_traites  +=Nb_bits_en_cours;
      Nb_bits_a_traiter-=Nb_bits_en_cours;

      if (GIF_Rest_bits==8) // Il ne reste plus de bits à coder sur l'octet courant
      {
        // Ecrire l'octet à balancer:
        GIF_Buffer[++GIF_Rest_byte]=GIF_Last_byte;

        // Si on a atteint la fin du bloc de Raster Data
        if (GIF_Rest_byte==255)
          // On doit vider le buffer qui est maintenant plein
          GIF_Vider_le_buffer();

        GIF_Last_byte=0;
        GIF_Rest_bits=0;
      }
    }
  }


  // -- Lire le pixel suivant --

  byte GIF_Pixel_suivant(void)
  {
    byte Temp;

    Temp=Lit_pixel_de_sauvegarde(GIF_Pos_X,GIF_Pos_Y);

    if (++GIF_Pos_X>=Principal_Largeur_image)
    {
      GIF_Pos_X=0;
      if (++GIF_Pos_Y>=Principal_Hauteur_image)
        GIF_Arret=1;
    }

    return Temp;
  }



void Save_GIF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];

  word * Alphabet_Prefixe;  // Table des préfixes des codes
  word * Alphabet_Suffixe;  // Table des suffixes des codes
  word * Alphabet_Fille;    // Table des chaînes filles (plus longues)
  word * Alphabet_Soeur;    // Table des chaînes soeurs (même longueur)
  word   Alphabet_Free;     // Position libre dans l'alphabet
  word   Alphabet_Max;      // Nombre d'entrées possibles dans l'alphabet
  word   Depart;            // Code précédent (sert au linkage des chaînes)
  int    Descente;          // Booléen "On vient de descendre"

  T_LSDB LSDB;
  T_IDB IDB;


  byte Block_indicateur;  // Code indicateur du type de bloc en cours
  word Chaine_en_cours;   // Code de la chaîne en cours de traitement
  byte current_char;         // Caractère à coder
  word Indice;            // Indice de recherche de chaîne


  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  GIF_Pos_X=0;
  GIF_Pos_Y=0;
  GIF_Last_byte=0;
  GIF_Rest_bits=0;
  GIF_Rest_byte=0;

  Nom_fichier_complet(Nom_du_fichier,0);

  if ((GIF_Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // On écrit la signature du fichier
    if (write_bytes(GIF_Fichier,"GIF89a",6))
    {
      // La signature du fichier a été correctement écrite.

      // Allocation de mémoire pour les tables
      Alphabet_Prefixe=(word *)malloc(4096*sizeof(word));
      Alphabet_Suffixe=(word *)malloc(4096*sizeof(word));
      Alphabet_Fille  =(word *)malloc(4096*sizeof(word));
      Alphabet_Soeur  =(word *)malloc(4096*sizeof(word));

      // On initialise le LSDB du fichier
      if (Config.Taille_ecran_dans_GIF)
      {
        LSDB.Width=Largeur_ecran;
        LSDB.Height=Hauteur_ecran;
      }
      else
      {
        LSDB.Width=Principal_Largeur_image;
        LSDB.Height=Principal_Hauteur_image;
      }
      LSDB.Resol  =0x97;          // Image en 256 couleurs, avec une palette
      LSDB.Backcol=0;
      LSDB.Aspect =0;             // Palette normale

      // On sauve le LSDB dans le fichier

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      LSDB.Width = SDL_Swap16(LSDB.Width);
      LSDB.Height = SDL_Swap16(LSDB.Height);
#endif

      if (write_bytes(GIF_Fichier,&LSDB,sizeof(T_LSDB)))
      {
        // Le LSDB a été correctement écrit.

        // On sauve la palette
        if (write_bytes(GIF_Fichier,Principal_Palette,768))
        {
          // La palette a été correctement écrite.

          //   Le jour où on se servira des blocks d'extensions pour placer
          // des commentaires, on le fera ici.

          // Ecriture de la transparence
          //write_bytes(GIF_Fichier,"\x21\xF9\x04\x01\x00\x00\xNN\x00",8);

          // Ecriture du commentaire
          if (Principal_Commentaire[0])
          {
            write_bytes(GIF_Fichier,"\x21\xFE",2);
            write_byte(GIF_Fichier,strlen(Principal_Commentaire));
            write_bytes(GIF_Fichier,Principal_Commentaire,strlen(Principal_Commentaire)+1);
          }
                            

          
          // On va écrire un block indicateur d'IDB et l'IDB du fichier

          Block_indicateur=0x2C;
          IDB.Pos_X=0;
          IDB.Pos_Y=0;
          IDB.Largeur_image=Principal_Largeur_image;
          IDB.Hauteur_image=Principal_Hauteur_image;
          IDB.Indicateur=0x07;    // Image non entrelacée, pas de palette locale.
          IDB.Nb_bits_pixel=8; // Image 256 couleurs;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
          IDB.Largeur_image = SDL_Swap16(IDB.Largeur_image);
          IDB.Hauteur_image = SDL_Swap16(IDB.Hauteur_image);
#endif

          if ( write_bytes(GIF_Fichier,&Block_indicateur,1) &&
               write_bytes(GIF_Fichier,&IDB,sizeof(T_IDB)) )
          {
            //   Le block indicateur d'IDB et l'IDB ont étés correctements
            // écrits.

            Init_ecriture();

            Indice=4096;
            Erreur_fichier=0;
            GIF_Arret=0;

            // Réintialisation de la table:
            Alphabet_Free=258;
            GIF_Nb_bits  =9;
            Alphabet_Max =511;
            GIF_Set_code(256);
            for (Depart=0;Depart<4096;Depart++)
            {
              Alphabet_Fille[Depart]=4096;
              Alphabet_Soeur[Depart]=4096;
            }

            ////////////////////////////////////////////// COMPRESSION LZW //

            Depart=Chaine_en_cours=GIF_Pixel_suivant();
            Descente=1;

            do
            {
              current_char=GIF_Pixel_suivant();

              //   On regarde si dans la table on aurait pas une chaîne
              // équivalente à Chaine_en_cours+Caractere

              while ( (Indice<Alphabet_Free) &&
                      ( (Chaine_en_cours!=Alphabet_Prefixe[Indice]) ||
                        (current_char      !=Alphabet_Suffixe[Indice]) ) )
              {
                Descente=0;
                Depart=Indice;
                Indice=Alphabet_Soeur[Indice];
              }

              if (Indice<Alphabet_Free)
              {
                //   On sait ici que la Chaine_en_cours+Caractere se trouve
                // en position Indice dans les tables.

                Descente=1;
                Depart=Chaine_en_cours=Indice;
                Indice=Alphabet_Fille[Indice];
              }
              else
              {
                // On fait la jonction entre la Chaine_en_cours et l'actuelle
                if (Descente)
                  Alphabet_Fille[Depart]=Alphabet_Free;
                else
                  Alphabet_Soeur[Depart]=Alphabet_Free;

                // On rajoute la chaîne Chaine_en_cours+Caractere à la table
                Alphabet_Prefixe[Alphabet_Free  ]=Chaine_en_cours;
                Alphabet_Suffixe[Alphabet_Free++]=current_char;

                // On écrit le code dans le fichier
                GIF_Set_code(Chaine_en_cours);

                if (Alphabet_Free>0xFFF)
                {
                  // Réintialisation de la table:
                  GIF_Set_code(256);
                  Alphabet_Free=258;
                  GIF_Nb_bits  =9;
                  Alphabet_Max =511;
                  for (Depart=0;Depart<4096;Depart++)
                  {
                    Alphabet_Fille[Depart]=4096;
                    Alphabet_Soeur[Depart]=4096;
                  }
                }
                else if (Alphabet_Free>Alphabet_Max+1)
                {
                  // On augmente le nb de bits

                  GIF_Nb_bits++;
                  Alphabet_Max=(1<<GIF_Nb_bits)-1;
                }

                // On initialise la Chaine_en_cours et le reste pour la suite
                Indice=Alphabet_Fille[current_char];
                Depart=Chaine_en_cours=current_char;
                Descente=1;
              }
            }
            while ((!GIF_Arret) && (!Erreur_fichier));

            if (!Erreur_fichier)
            {
              // On écrit le code dans le fichier
              GIF_Set_code(Chaine_en_cours); // Dernière portion d'image

              //   Cette dernière portion ne devrait pas poser de problèmes
              // du côté GIF_Nb_bits puisque pour que GIF_Nb_bits change de
              // valeur, il faudrait que la table de chaîne soit remplie or
              // c'est impossible puisqu'on traite une chaîne qui se trouve
              // déjà dans la table, et qu'elle n'a rien d'inédit. Donc on
              // ne devrait pas avoir à changer de taille, mais je laisse
              // quand même en remarque tout ça, au cas où il subsisterait
              // des problèmes dans certains cas exceptionnels.
              //
              // Note: de toutes façons, ces lignes en commentaires ont étés
              //      écrites par copier/coller du temps où la sauvegarde du
              //      GIF déconnait. Il y a donc fort à parier qu'elles ne
              //      sont pas correctes.

              /*
              if (Chaine_en_cours==Alphabet_Max)
              {
                if (Alphabet_Max==0xFFF)
                {
                  // On balargue un Clear Code
                  GIF_Set_code(256);

                  // On réinitialise les données LZW
                  Alphabet_Free=258;
                  GIF_Nb_bits  =9;
                  Alphabet_Max =511;
                }
                else
                {
                  GIF_Nb_bits++;
                  Alphabet_Max=(1<<GIF_Nb_bits)-1;
                }
              }
              */

              GIF_Set_code(257);             // Code de Fin d'image
              if (GIF_Rest_bits!=0)
                GIF_Set_code(0);             // Code bidon permettant de s'assurer que tous les bits du dernier code aient bien étés inscris dans le buffer GIF
              GIF_Vider_le_buffer();         // On envoie les dernières données du buffer GIF dans le buffer KM
              Close_ecriture(GIF_Fichier);   // On envoie les dernières données du buffer KM  dans le fichier

              // On écrit un \0
              if (! write_byte(GIF_Fichier,'\x00'))
                Erreur_fichier=1;
              // On écrit un GIF TERMINATOR, exigé par SVGA et SEA.
              if (! write_byte(GIF_Fichier,'\x3B'))
                Erreur_fichier=1;

              
            }

          } // On a pu écrire l'IDB
          else
            Erreur_fichier=1;

        } // On a pu écrire la palette
        else
          Erreur_fichier=1;

      } // On a pu écrire le LSDB
      else
        Erreur_fichier=1;

      // Libération de la mémoire utilisée par les tables
      free(Alphabet_Soeur);
      free(Alphabet_Fille);
      free(Alphabet_Suffixe);
      free(Alphabet_Prefixe);

    } // On a pu écrire la signature du fichier
    else
      Erreur_fichier=1;

    fclose(GIF_Fichier);
    if (Erreur_fichier)
      remove(Nom_du_fichier);

  } // On a pu ouvrir le fichier en écriture
  else
    Erreur_fichier=1;
}






/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PCX ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
  {
    byte Manufacturer;       // |_ Il font chier ces cons! Ils auraient pu
    byte Version;            // |  mettre une vraie signature!
    byte Compression;        // L'image est-elle compressée?
    byte Depth;              // Nombre de bits pour coder un pixel (inutile puisqu'on se sert de Plane)
    word X_min;              // |_ Coin haut-gauche   |
    word Y_min;              // |  de l'image         |_ (Crétin!)
    word X_max;              // |_ Coin bas-droit     |
    word Y_max;              // |  de l'image         |
    word X_dpi;              // |_ Densité de |_ (Presque inutile parce que
    word Y_dpi;              // |  l'image    |  aucun moniteur n'est pareil!)
    byte Palette_16c[48];    // Palette 16 coul (inutile pour 256c) (débile!)
    byte Reserved;           // Ca me plait ça aussi!
    byte Plane;              // 4 => 16c , 1 => 256c , ...
    word Bytes_per_plane_line;// Doit toujours être pair
    word Palette_info;       // 1 => Couleur , 2 => Gris (ignoré à partir de la version 4)
    word Screen_X;           // |_ Dimensions de
    word Screen_Y;           // |  l'écran d'origine
    byte Filler[54];         // Ca... J'adore!
  } __attribute__((__packed__)) T_PCX_Header;

T_PCX_Header PCX_header;

// -- Tester si un fichier est au format PCX --------------------------------

void Test_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (read_byte(file,&(PCX_header.Manufacturer)) &&
        read_byte(file,&(PCX_header.Version)) &&
        read_byte(file,&(PCX_header.Compression)) &&
        read_byte(file,&(PCX_header.Depth)) &&
        read_word_le(file,&(PCX_header.X_min)) &&
        read_word_le(file,&(PCX_header.Y_min)) &&
        read_word_le(file,&(PCX_header.X_max)) &&
        read_word_le(file,&(PCX_header.Y_max)) &&
        read_word_le(file,&(PCX_header.X_dpi)) &&
        read_word_le(file,&(PCX_header.Y_dpi)) &&
        read_bytes(file,&(PCX_header.Palette_16c),48) &&        
        read_byte(file,&(PCX_header.Reserved)) &&
        read_byte(file,&(PCX_header.Plane)) &&
        read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
        read_word_le(file,&(PCX_header.Palette_info)) &&
        read_word_le(file,&(PCX_header.Screen_X)) &&
        read_word_le(file,&(PCX_header.Screen_Y)) &&
        read_bytes(file,&(PCX_header.Filler),54) )
    {
    
      //   Vu que ce header a une signature de merde et peu significative, il
      // va falloir que je teste différentes petites valeurs dont je connais
      // l'intervalle. Grrr!
      if ( (PCX_header.Manufacturer!=10)
        || (PCX_header.Compression>1)
        || ( (PCX_header.Depth!=1) && (PCX_header.Depth!=2) && (PCX_header.Depth!=4) && (PCX_header.Depth!=8) )
        || ( (PCX_header.Plane!=1) && (PCX_header.Plane!=2) && (PCX_header.Plane!=4) && (PCX_header.Plane!=8) && (PCX_header.Plane!=3) )
        || (PCX_header.X_max<PCX_header.X_min)
        || (PCX_header.Y_max<PCX_header.Y_min)
        || (PCX_header.Bytes_per_plane_line&1) )
        Erreur_fichier=1;
    }
    else
      Erreur_fichier=1;

    fclose(file);
  }
}


// -- Lire un fichier au format PCX -----------------------------------------

  // -- Afficher une ligne PCX codée sur 1 seul plan avec moins de 256 c. --
  void Draw_PCX_line(short y_pos, byte depth)
  {
    short x_pos;
    byte  Couleur;
    byte  Reduction=8/depth;
    byte  byte_mask=(1<<depth)-1;
    byte  Reduction_moins_1=Reduction-1;

    for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
    {
      Couleur=(LBM_Buffer[x_pos/Reduction]>>((Reduction_moins_1-(x_pos%Reduction))*depth)) & byte_mask;
      Pixel_de_chargement(x_pos,y_pos,Couleur);
    }
  }

void Load_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  
  short Taille_ligne;
  short Vraie_taille_ligne; // width de l'image corrigée
  short Largeur_lue;
  short x_pos;
  short y_pos;
  byte  byte1;
  byte  byte2;
  byte  Indice;
  dword Nb_couleurs;
  long  Taille_du_fichier;
  byte  Palette_CGA[9]={ 84,252,252,  252, 84,252,  252,252,252};

  long  position;
  long  Taille_image;
  byte * Buffer;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);
    /*
    if (read_bytes(file,&PCX_header,sizeof(T_PCX_Header)))
    {*/
    
    if (read_byte(file,&(PCX_header.Manufacturer)) &&
        read_byte(file,&(PCX_header.Version)) &&
        read_byte(file,&(PCX_header.Compression)) &&
        read_byte(file,&(PCX_header.Depth)) &&
        read_word_le(file,&(PCX_header.X_min)) &&
        read_word_le(file,&(PCX_header.Y_min)) &&
        read_word_le(file,&(PCX_header.X_max)) &&
        read_word_le(file,&(PCX_header.Y_max)) &&
        read_word_le(file,&(PCX_header.X_dpi)) &&
        read_word_le(file,&(PCX_header.Y_dpi)) &&
        read_bytes(file,&(PCX_header.Palette_16c),48) &&        
        read_byte(file,&(PCX_header.Reserved)) &&
        read_byte(file,&(PCX_header.Plane)) &&
        read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
        read_word_le(file,&(PCX_header.Palette_info)) &&
        read_word_le(file,&(PCX_header.Screen_X)) &&
        read_word_le(file,&(PCX_header.Screen_Y)) &&
        read_bytes(file,&(PCX_header.Filler),54) )
    {
      
      Principal_Largeur_image=PCX_header.X_max-PCX_header.X_min+1;
      Principal_Hauteur_image=PCX_header.Y_max-PCX_header.Y_min+1;

      Ecran_original_X=PCX_header.Screen_X;
      Ecran_original_Y=PCX_header.Screen_Y;

      if (PCX_header.Plane!=3)
      {
        Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_PCX);
        if (Erreur_fichier==0)
        {
          // On prépare la palette à accueillir les valeurs du fichier PCX
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          Nb_couleurs=(dword)(1<<PCX_header.Plane)<<(PCX_header.Depth-1);

          if (Nb_couleurs>4)
            memcpy(Principal_Palette,PCX_header.Palette_16c,48);
          else
          {
            Principal_Palette[1].R=0;
            Principal_Palette[1].G=0;
            Principal_Palette[1].B=0;
            byte1=PCX_header.Palette_16c[3]>>5;
            if (Nb_couleurs==4)
            { // Pal. CGA "alakon" (du Turc Allahkoum qui signifie "à la con" :))
              memcpy(Principal_Palette+1,Palette_CGA,9);
              if (!(byte1&2))
              {
                Principal_Palette[1].B=84;
                Principal_Palette[2].B=84;
                Principal_Palette[3].B=84;
              }
            } // Palette monochrome (on va dire que c'est du N&B)
            else
            {
              Principal_Palette[1].R=252;
              Principal_Palette[1].G=252;
              Principal_Palette[1].B=252;
            }
          }

          //   On se positionne à la fin du fichier - 769 octets pour voir s'il y
          // a une palette.
          if ( (PCX_header.Depth==8) && (PCX_header.Version>=5) && (Taille_du_fichier>(256*3)) )
          {
            fseek(file,Taille_du_fichier-((256*3)+1),SEEK_SET);
            // On regarde s'il y a une palette après les données de l'image
            if (read_byte(file,&byte1))
              if (byte1==12) // Lire la palette si c'est une image en 256 couleurs
              {
                int index;
                // On lit la palette 256c que ces crétins ont foutue à la fin du fichier
                for(index=0;index<256;index++)
                  if ( ! read_byte(file,&Principal_Palette[index].R)
                   || ! read_byte(file,&Principal_Palette[index].G)
                   || ! read_byte(file,&Principal_Palette[index].B) )
                  {
                    Erreur_fichier=2;
                    DEBUG("ERROR READING PCX PALETTE !",index);
                    break;
                  }
              }
          }
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          //   Maintenant qu'on a lu la palette que ces crétins sont allés foutre
          // à la fin, on retourne juste après le header pour lire l'image.
          fseek(file,128,SEEK_SET);
          if (!Erreur_fichier)
          {
            Taille_ligne=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
            Vraie_taille_ligne=(short)PCX_header.Bytes_per_plane_line<<3;
            //   On se sert de données LBM car le dessin de ligne en moins de 256
            // couleurs se fait comme avec la structure ILBM.
            Image_HAM=0;
            HBPm1=PCX_header.Plane-1;
            LBM_Buffer=(byte *)malloc(Taille_ligne);

            // Chargement de l'image
            if (PCX_header.Compression)  // Image compressée
            {
              /*Init_lecture();*/
  
              Taille_image=(long)PCX_header.Bytes_per_plane_line*Principal_Hauteur_image;

              if (PCX_header.Depth==8) // 256 couleurs (1 plan)
              {
                for (position=0; ((position<Taille_image) && (!Erreur_fichier));)
                {
                  // Lecture et décompression de la ligne
                  Lire_octet(file,&byte1);
                  if (!Erreur_fichier)
                  {
                    if ((byte1&0xC0)==0xC0)
                    {
                      byte1-=0xC0;               // facteur de répétition
                      Lire_octet(file,&byte2); // octet à répéter
                      if (!Erreur_fichier)
                      {
                        for (Indice=0; Indice<byte1; Indice++,position++)
                          if (position<Taille_image)
                            Pixel_de_chargement(position%Taille_ligne,
                                                position/Taille_ligne,
                                                byte2);
                          else
                            Erreur_fichier=2;
                      }
                    }
                    else
                    {
                      Pixel_de_chargement(position%Taille_ligne,
                                          position/Taille_ligne,
                                          byte1);
                      position++;
                    }
                  }
                }
              }
              else                 // couleurs rangées par plans
              {
                for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
                {
                  for (x_pos=0; ((x_pos<Taille_ligne) && (!Erreur_fichier)); )
                  {
                    Lire_octet(file,&byte1);
                    if (!Erreur_fichier)
                    {
                      if ((byte1&0xC0)==0xC0)
                      {
                        byte1-=0xC0;               // facteur de répétition
                        Lire_octet(file,&byte2); // octet à répéter
                        if (!Erreur_fichier)
                        {
                          for (Indice=0; Indice<byte1; Indice++)
                            if (x_pos<Taille_ligne)
                              LBM_Buffer[x_pos++]=byte2;
                            else
                              Erreur_fichier=2;
                        }
                        else
                          Modif_Erreur_fichier(2);
                      }
                      else
                        LBM_Buffer[x_pos++]=byte1;
                    }
                  }
                  // Affichage de la ligne par plan du buffer
                  if (PCX_header.Depth==1)
                    Draw_ILBM_line(y_pos,Vraie_taille_ligne);
                  else
                    Draw_PCX_line(y_pos,PCX_header.Depth);
                }
              }

              /*Close_lecture();*/
            }
            else                     // Image non compressée
            {
              for (y_pos=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);y_pos++)
              {
                if ((Largeur_lue=read_bytes(file,LBM_Buffer,Taille_ligne)))
                {
                  if (PCX_header.Plane==1)
                    for (x_pos=0; x_pos<Principal_Largeur_image;x_pos++)
                      Pixel_de_chargement(x_pos,y_pos,LBM_Buffer[x_pos]);
                  else
                  {
                    if (PCX_header.Depth==1)
                      Draw_ILBM_line(y_pos,Vraie_taille_ligne);
                    else
                      Draw_PCX_line(y_pos,PCX_header.Depth);
                  }
                }
                else
                  Erreur_fichier=2;
              }
            }

            free(LBM_Buffer);
          }
        }
      }
      else
      {
        // Image 24 bits!!!

        Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_PCX | FORMAT_24B);

        if (Erreur_fichier==0)
        {
          Taille_ligne=PCX_header.Bytes_per_plane_line*3;
          Buffer=(byte *)malloc(Taille_ligne);

          if (!PCX_header.Compression)
          {
            for (y_pos=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);y_pos++)
            {
              if (read_bytes(file,Buffer,Taille_ligne))
              {
                for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
                  Pixel_Chargement_24b(x_pos,y_pos,Buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
              }
              else
                Erreur_fichier=2;
            }
          }
          else
          {
            /*Init_lecture();*/

            for (y_pos=0,position=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);)
            {
              // Lecture et décompression de la ligne
              Lire_octet(file,&byte1);
              if (!Erreur_fichier)
              {
                if ((byte1 & 0xC0)==0xC0)
                {
                  byte1-=0xC0;               // facteur de répétition
                  Lire_octet(file,&byte2); // octet à répéter
                  if (!Erreur_fichier)
                  {
                    for (Indice=0; (Indice<byte1) && (!Erreur_fichier); Indice++)
                    {
                      Buffer[position++]=byte2;
                      if (position>=Taille_ligne)
                      {
                        for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
                          Pixel_Chargement_24b(x_pos,y_pos,Buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                        y_pos++;
                        position=0;
                      }
                    }
                  }
                }
                else
                {
                  Buffer[position++]=byte1;
                  if (position>=Taille_ligne)
                  {
                    for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
                      Pixel_Chargement_24b(x_pos,y_pos,Buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],Buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                    y_pos++;
                    position=0;
                  }
                }
              }
            }
            if (position!=0)
              Erreur_fichier=2;

            /*Close_lecture();*/
          }
          free(Buffer);
        }
      }
    }
    else
    {
      Erreur_fichier=1;
    }

    fclose(file);
  }
  else
    Erreur_fichier=1;
}


// -- Ecrire un fichier au format PCX ---------------------------------------

void Save_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;

  short Taille_ligne;
  short x_pos;
  short y_pos;
  byte  counter;
  byte  Last_pixel;
  byte  Pixel_lu;



  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((file=fopen(Nom_du_fichier,"wb")))
  {

    PCX_header.Manufacturer=10;
    PCX_header.Version=5;
    PCX_header.Compression=1;
    PCX_header.Depth=8;
    PCX_header.X_min=0;
    PCX_header.Y_min=0;
    PCX_header.X_max=Principal_Largeur_image-1;
    PCX_header.Y_max=Principal_Hauteur_image-1;
    PCX_header.X_dpi=0;
    PCX_header.Y_dpi=0;
    memcpy(PCX_header.Palette_16c,Principal_Palette,48);
    PCX_header.Reserved=0;
    PCX_header.Plane=1;
    PCX_header.Bytes_per_plane_line=(Principal_Largeur_image&1)?Principal_Largeur_image+1:Principal_Largeur_image;
    PCX_header.Palette_info=1;
    PCX_header.Screen_X=Largeur_ecran;
    PCX_header.Screen_Y=Hauteur_ecran;
    memset(PCX_header.Filler,0,54);

    if (write_bytes(file,&(PCX_header.Manufacturer),1) &&
        write_bytes(file,&(PCX_header.Version),1) &&
        write_bytes(file,&(PCX_header.Compression),1) &&
        write_bytes(file,&(PCX_header.Depth),1) &&
        write_word_le(file,PCX_header.X_min) &&
        write_word_le(file,PCX_header.Y_min) &&
        write_word_le(file,PCX_header.X_max) &&
        write_word_le(file,PCX_header.Y_max) &&
        write_word_le(file,PCX_header.X_dpi) &&
        write_word_le(file,PCX_header.Y_dpi) &&
        write_bytes(file,&(PCX_header.Palette_16c),sizeof(PCX_header.Palette_16c)) &&        
        write_bytes(file,&(PCX_header.Reserved),1) &&
        write_bytes(file,&(PCX_header.Plane),1) &&
        write_word_le(file,PCX_header.Bytes_per_plane_line) &&
        write_word_le(file,PCX_header.Palette_info) &&
        write_word_le(file,PCX_header.Screen_X) &&
        write_word_le(file,PCX_header.Screen_Y) &&
        write_bytes(file,&(PCX_header.Filler),sizeof(PCX_header.Filler)) )
    {
      Taille_ligne=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
     
      Init_ecriture();
     
      for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
      {
        Pixel_lu=Lit_pixel_de_sauvegarde(0,y_pos);
     
        // Compression et écriture de la ligne
        for (x_pos=0; ((x_pos<Taille_ligne) && (!Erreur_fichier)); )
        {
          x_pos++;
          Last_pixel=Pixel_lu;
          Pixel_lu=Lit_pixel_de_sauvegarde(x_pos,y_pos);
          counter=1;
          while ( (counter<63) && (x_pos<Taille_ligne) && (Pixel_lu==Last_pixel) )
          {
            counter++;
            x_pos++;
            Pixel_lu=Lit_pixel_de_sauvegarde(x_pos,y_pos);
          }
      
          if ( (counter>1) || (Last_pixel>=0xC0) )
            Ecrire_octet(file,counter|0xC0);
          Ecrire_octet(file,Last_pixel);
      
        }
      }
      
      // Ecriture de l'octet (12) indiquant que la palette arrive
      if (!Erreur_fichier)
        Ecrire_octet(file,12);
      
      Close_ecriture(file);
      
      // Ecriture de la palette
      if (!Erreur_fichier)
      {
        if (! write_bytes(file,Principal_Palette,sizeof(T_Palette)))
          Erreur_fichier=1;
      }
    }
    else
      Erreur_fichier=1;
       
    fclose(file);
       
    if (Erreur_fichier)
      remove(Nom_du_fichier);
       
  }    
  else 
    Erreur_fichier=1;
}      
       
       
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// CEL ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word Width;              // width de l'image
  word Height;             // height de l'image
} __attribute__((__packed__))  T_CEL_Header1;

typedef struct
{
  byte Signa[4];           // Signature du format
  byte Kind;               // Type de fichier ($10=PALette $20=BitMaP)
  byte Nbbits;             // Nombre de bits
  word Filler1;            // ???
  word Width;            // width de l'image
  word Height;            // height de l'image
  word Decalage_X;         // Decalage en X de l'image
  word Decalage_Y;         // Decalage en Y de l'image
  byte Filler2[16];        // ???
} __attribute__((__packed__))  T_CEL_Header2;

// -- Tester si un fichier est au format CEL --------------------------------

void Test_CEL(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  int  size;
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  int Taille_fichier;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  Taille_fichier=FileLength(Nom_du_fichier);
  if (Taille_fichier==0)
  {
    Erreur_fichier = 1; // Si on ne peut pas faire de stat il vaut mieux laisser tomber
    return;
  }
  
  if (! (file=fopen(Nom_du_fichier, "rb")))
  {
    Erreur_fichier = 1;
    return;
  }
  if (read_word_le(file,&header1.Width) &&
      read_word_le(file,&header1.Height) )
  {
      //   Vu que ce header n'a pas de signature, il va falloir tester la
      // cohérence de la dimension de l'image avec celle du fichier.
      
      size=Taille_fichier-sizeof(T_CEL_Header1);
      if ( (!size) || ( (((header1.Width+1)>>1)*header1.Height)!=size ) )
      {
        // Tentative de reconnaissance de la signature des nouveaux fichiers

        fseek(file,0,SEEK_SET);        
        if (read_bytes(file,&header2.Signa,4) &&
            !memcmp(header2.Signa,"KiSS",4) &&
            read_byte(file,&header2.Kind) &&
            (header2.Kind==0x20) &&
            read_byte(file,&header2.Nbbits) &&
            read_word_le(file,&header2.Filler1) &&
            read_word_le(file,&header2.Width) &&
            read_word_le(file,&header2.Height) &&
            read_word_le(file,&header2.Decalage_X) &&
            read_word_le(file,&header2.Decalage_Y) &&
            read_bytes(file,&header2.Filler2,16))
        {
          // ok
        }
        else
          Erreur_fichier=1;
      }
      else
        Erreur_fichier=1;
  }
  else
  {
    Erreur_fichier=1;
  }
  fclose(file);    
}


// -- Lire un fichier au format CEL -----------------------------------------

void Load_CEL(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  Dernier_octet=0;
  long  Taille_du_fichier;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (read_bytes(file,&header1,sizeof(T_CEL_Header1)))
    {
      Taille_du_fichier=File_length_file(file);
      if ( (Taille_du_fichier>(long int)sizeof(T_CEL_Header1))
        && ( (((header1.Width+1)>>1)*header1.Height)==(Taille_du_fichier-(long int)sizeof(T_CEL_Header1)) ) )
      {
        // Chargement d'un fichier CEL sans signature (vieux fichiers)
        Principal_Largeur_image=header1.Width;
        Principal_Hauteur_image=header1.Height;
        Ecran_original_X=Principal_Largeur_image;
        Ecran_original_Y=Principal_Hauteur_image;
        Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_CEL);
        if (Erreur_fichier==0)
        {
          // Chargement de l'image
          /*Init_lecture();*/
          for (y_pos=0;((y_pos<Principal_Hauteur_image) && (!Erreur_fichier));y_pos++)
            for (x_pos=0;((x_pos<Principal_Largeur_image) && (!Erreur_fichier));x_pos++)
              if ((x_pos & 1)==0)
              {
                Lire_octet(file,&Dernier_octet);
                Pixel_de_chargement(x_pos,y_pos,(Dernier_octet >> 4));
              }
              else
                Pixel_de_chargement(x_pos,y_pos,(Dernier_octet & 15));
          /*Close_lecture();*/
        }
      }
      else
      {
        // On réessaye avec le nouveau format

        fseek(file,0,SEEK_SET);
        if (read_bytes(file,&header2,sizeof(T_CEL_Header2)))
        {
          // Chargement d'un fichier CEL avec signature (nouveaux fichiers)

          Principal_Largeur_image=header2.Width+header2.Decalage_X;
          Principal_Hauteur_image=header2.Height+header2.Decalage_Y;
          Ecran_original_X=Principal_Largeur_image;
          Ecran_original_Y=Principal_Hauteur_image;
          Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_CEL);
          if (Erreur_fichier==0)
          {
            // Chargement de l'image
            /*Init_lecture();*/

            if (!Erreur_fichier)
            {
              // Effacement du décalage
              for (y_pos=0;y_pos<header2.Decalage_Y;y_pos++)
                for (x_pos=0;x_pos<Principal_Largeur_image;x_pos++)
                  Pixel_de_chargement(x_pos,y_pos,0);
              for (y_pos=header2.Decalage_Y;y_pos<Principal_Hauteur_image;y_pos++)
                for (x_pos=0;x_pos<header2.Decalage_X;x_pos++)
                  Pixel_de_chargement(x_pos,y_pos,0);

              switch(header2.Nbbits)
              {
                case 4:
                  for (y_pos=0;((y_pos<header2.Height) && (!Erreur_fichier));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!Erreur_fichier));x_pos++)
                      if ((x_pos & 1)==0)
                      {
                        Lire_octet(file,&Dernier_octet);
                        Pixel_de_chargement(x_pos+header2.Decalage_X,y_pos+header2.Decalage_Y,(Dernier_octet >> 4));
                      }
                      else
                        Pixel_de_chargement(x_pos+header2.Decalage_X,y_pos+header2.Decalage_Y,(Dernier_octet & 15));
                  break;

                case 8:
                  for (y_pos=0;((y_pos<header2.Height) && (!Erreur_fichier));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!Erreur_fichier));x_pos++)
                    {
                      byte Lu;
                      Lire_octet(file,&Lu);
                      Pixel_de_chargement(x_pos+header2.Decalage_X,y_pos+header2.Decalage_Y,Lu);
                      }
                  break;

                default:
                  Erreur_fichier=1;
              }
            }
            /*Close_lecture();*/
          }
        }
        else
          Erreur_fichier=1;
      }
      fclose(file);
    }
    else
      Erreur_fichier=1;
  }
  else
    Erreur_fichier=1;
}


// -- Ecrire un fichier au format CEL ---------------------------------------

void Save_CEL(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  Dernier_octet=0;
  dword Utilisation[256]; // Table d'utilisation de couleurs


  // On commence par compter l'utilisation de chaque couleurs
  Palette_Compter_nb_couleurs_utilisees(Utilisation);

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (x_pos=16;((x_pos<256) && (!Utilisation[x_pos]));x_pos++);

    if (x_pos==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      header1.Width =Principal_Largeur_image;
      header1.Height=Principal_Hauteur_image;

      if (write_bytes(file,&header1,sizeof(T_CEL_Header1)))
      {
        // Sauvegarde de l'image
        Init_ecriture();
        for (y_pos=0;((y_pos<Principal_Hauteur_image) && (!Erreur_fichier));y_pos++)
        {
          for (x_pos=0;((x_pos<Principal_Largeur_image) && (!Erreur_fichier));x_pos++)
            if ((x_pos & 1)==0)
              Dernier_octet=(Lit_pixel_de_sauvegarde(x_pos,y_pos) << 4);
            else
            {
              Dernier_octet=Dernier_octet | (Lit_pixel_de_sauvegarde(x_pos,y_pos) & 15);
              Ecrire_octet(file,Dernier_octet);
            }

          if ((x_pos & 1)==1)
            Ecrire_octet(file,Dernier_octet);
        }
        Close_ecriture(file);
      }
      else
        Erreur_fichier=1;
      fclose(file);
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      // Recherche du décalage
      for (y_pos=0;y_pos<Principal_Hauteur_image;y_pos++)
      {
        for (x_pos=0;x_pos<Principal_Largeur_image;x_pos++)
          if (Lit_pixel_de_sauvegarde(x_pos,y_pos)!=0)
            break;
        if (Lit_pixel_de_sauvegarde(x_pos,y_pos)!=0)
          break;
      }
      header2.Decalage_Y=y_pos;
      for (x_pos=0;x_pos<Principal_Largeur_image;x_pos++)
      {
        for (y_pos=0;y_pos<Principal_Hauteur_image;y_pos++)
          if (Lit_pixel_de_sauvegarde(x_pos,y_pos)!=0)
            break;
        if (Lit_pixel_de_sauvegarde(x_pos,y_pos)!=0)
          break;
      }
      header2.Decalage_X=x_pos;

      memcpy(header2.Signa,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x20;              // Initialisation du type (BitMaP)
      header2.Nbbits=8;               // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (???)
      header2.Width=Principal_Largeur_image-header2.Decalage_X; // Initialisation de la largeur
      header2.Height=Principal_Hauteur_image-header2.Decalage_Y; // Initialisation de la hauteur
      for (x_pos=0;x_pos<16;x_pos++)  // Initialisation du filler 2 (???)
        header2.Filler2[x_pos]=0;

      if (write_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        // Sauvegarde de l'image
        Init_ecriture();
        for (y_pos=0;((y_pos<header2.Height) && (!Erreur_fichier));y_pos++)
          for (x_pos=0;((x_pos<header2.Width) && (!Erreur_fichier));x_pos++)
            Ecrire_octet(file,Lit_pixel_de_sauvegarde(x_pos+header2.Decalage_X,y_pos+header2.Decalage_Y));
        Close_ecriture(file);
      }
      else
        Erreur_fichier=1;
      fclose(file);
    }

    if (Erreur_fichier)
      remove(Nom_du_fichier);
  }
  else
    Erreur_fichier=1;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// KCF ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct
  {
    struct
    {
      byte Byte1;
      byte Byte2;
    } Couleur[16];
  } Palette[10];
} __attribute__((__packed__)) T_KCF_Header;

// -- Tester si un fichier est au format KCF --------------------------------

void Test_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_KCF_Header Buffer;
  T_CEL_Header2 header2;
  int Indice_palette;
  int Indice_couleur;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (File_length_file(file)==sizeof(T_KCF_Header))
    {
      read_bytes(file,&Buffer,sizeof(T_KCF_Header));
      // On vérifie une propriété de la structure de palette:
      for (Indice_palette=0;Indice_palette<10;Indice_palette++)
        for (Indice_couleur=0;Indice_couleur<16;Indice_couleur++)
          if ((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte2>>4)!=0)
            Erreur_fichier=1;
    }
    else
    {
      if (read_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        if (memcmp(header2.Signa,"KiSS",4)==0)
        {
          if (header2.Kind!=0x10)
            Erreur_fichier=1;
        }
        else
          Erreur_fichier=1;
      }
      else
        Erreur_fichier=1;
    }
    fclose(file);
  }
  else
    Erreur_fichier=1;
}


// -- Lire un fichier au format KCF -----------------------------------------

void Load_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_KCF_Header Buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int Indice_palette;
  int Indice_couleur;
  int Indice;
  long  Taille_du_fichier;


  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);
    if (Taille_du_fichier==sizeof(T_KCF_Header))
    {
      // Fichier KCF à l'ancien format

      if (read_bytes(file,&Buffer,sizeof(T_KCF_Header)))
      {
        // Initialiser_preview(???); // Pas possible... pas d'image...

        if (Config.Clear_palette)
          memset(Principal_Palette,0,sizeof(T_Palette));

        // Chargement de la palette
        for (Indice_palette=0;Indice_palette<10;Indice_palette++)
          for (Indice_couleur=0;Indice_couleur<16;Indice_couleur++)
          {
            Indice=16+(Indice_palette*16)+Indice_couleur;
            Principal_Palette[Indice].R=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte1 >> 4) << 4);
            Principal_Palette[Indice].B=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte1 & 15) << 4);
            Principal_Palette[Indice].G=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte2 & 15) << 4);
          }

        for (Indice=0;Indice<16;Indice++)
        {
          Principal_Palette[Indice].R=Principal_Palette[Indice+16].R;
          Principal_Palette[Indice].G=Principal_Palette[Indice+16].G;
          Principal_Palette[Indice].B=Principal_Palette[Indice+16].B;
        }

        Set_palette(Principal_Palette);
        Remapper_fileselect();
      }
      else
        Erreur_fichier=1;
    }
    else
    {
      // Fichier KCF au nouveau format

      if (read_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        // Initialiser_preview(???); // Pas possible... pas d'image...

        Indice=(header2.Nbbits==12)?16:0;
        for (Indice_palette=0;Indice_palette<header2.Height;Indice_palette++)
        {
           // Pour chaque palette

           for (Indice_couleur=0;Indice_couleur<header2.Width;Indice_couleur++)
           {
             // Pour chaque couleur

             switch(header2.Nbbits)
             {
               case 12: // RRRR BBBB | 0000 VVVV
                 read_bytes(file,bytes,2);
                 Principal_Palette[Indice].R=(bytes[0] >> 4) << 4;
                 Principal_Palette[Indice].B=(bytes[0] & 15) << 4;
                 Principal_Palette[Indice].G=(bytes[1] & 15) << 4;
                 break;

               case 24: // RRRR RRRR | VVVV VVVV | BBBB BBBB
                 read_bytes(file,bytes,3);
                 Principal_Palette[Indice].R=bytes[0];
                 Principal_Palette[Indice].G=bytes[1];
                 Principal_Palette[Indice].B=bytes[2];
             }

             Indice++;
           }
        }

        if (header2.Nbbits==12)
          for (Indice=0;Indice<16;Indice++)
          {
            Principal_Palette[Indice].R=Principal_Palette[Indice+16].R;
            Principal_Palette[Indice].G=Principal_Palette[Indice+16].G;
            Principal_Palette[Indice].B=Principal_Palette[Indice+16].B;
          }

        Set_palette(Principal_Palette);
        Remapper_fileselect();
      }
      else
        Erreur_fichier=1;
    }
    fclose(file);
  }
  else
    Erreur_fichier=1;

  if (!Erreur_fichier) Dessiner_preview_palette();
}


// -- Ecrire un fichier au format KCF ---------------------------------------

void Save_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  T_KCF_Header Buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int Indice_palette;
  int Indice_couleur;
  int Indice;
  dword Utilisation[256]; // Table d'utilisation de couleurs

  // On commence par compter l'utilisation de chaque couleurs
  Palette_Compter_nb_couleurs_utilisees(Utilisation);

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    // Sauvegarde de la palette

    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (Indice=16;((Indice<256) && (!Utilisation[Indice]));Indice++);

    if (Indice==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      for (Indice_palette=0;Indice_palette<10;Indice_palette++)
        for (Indice_couleur=0;Indice_couleur<16;Indice_couleur++)
        {
          Indice=16+(Indice_palette*16)+Indice_couleur;
          Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte1=((Principal_Palette[Indice].R>>4)<<4) | (Principal_Palette[Indice].B>>4);
          Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Byte2=Principal_Palette[Indice].G>>4;
        }

      if (! write_bytes(file,&Buffer,sizeof(T_KCF_Header)))
        Erreur_fichier=1;
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      memcpy(header2.Signa,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x10;              // Initialisation du type (PALette)
      header2.Nbbits=24;              // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (???)
      header2.Width=256;            // Initialisation du nombre de couleurs
      header2.Height=1;              // Initialisation du nombre de palettes
      header2.Decalage_X=0;           // Initialisation du décalage X
      header2.Decalage_Y=0;           // Initialisation du décalage Y
      for (Indice=0;Indice<16;Indice++) // Initialisation du filler 2 (???)
        header2.Filler2[Indice]=0;

      if (! write_bytes(file,&header2,sizeof(T_CEL_Header2)))
        Erreur_fichier=1;

      for (Indice=0;(Indice<256) && (!Erreur_fichier);Indice++)
      {
        bytes[0]=Principal_Palette[Indice].R;
        bytes[1]=Principal_Palette[Indice].G;
        bytes[2]=Principal_Palette[Indice].B;
        if (! write_bytes(file,bytes,3))
          Erreur_fichier=1;
      }
    }

    fclose(file);

    if (Erreur_fichier)
      remove(Nom_du_fichier);
  }
  else
    Erreur_fichier=1;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SCx ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  byte Filler1[4];
  word Width;
  word Height;
  byte Filler2;
  byte Plans;
} __attribute__((__packed__)) T_SCx_Header;

// -- Tester si un fichier est au format SCx --------------------------------
void Test_SCx(void)
{
  FILE *file;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //byte Signature[3];
  T_SCx_Header SCx_header;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture et vérification de la signature
    if ((read_bytes(file,&SCx_header,sizeof(T_SCx_Header))))
    {
      if ( (!memcmp(SCx_header.Filler1,"RIX",3))
        && SCx_header.Width && SCx_header.Height)
      Erreur_fichier=0;
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format SCx -----------------------------------------
void Load_SCx(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  long size,Vraie_taille;
  long Taille_du_fichier;
  T_SCx_Header SCx_header;
  T_Palette SCx_Palette;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=File_length_file(file);

    if ((read_bytes(file,&SCx_header,sizeof(T_SCx_Header))))
    {
      Initialiser_preview(SCx_header.Width,SCx_header.Height,Taille_du_fichier,FORMAT_SCx);
      if (Erreur_fichier==0)
      {
        if (!SCx_header.Plans)
          size=sizeof(T_Palette);
        else
          size=sizeof(Composantes)*(1<<SCx_header.Plans);

        if (read_bytes(file,SCx_Palette,size))
        {
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));

          Palette_64_to_256(SCx_Palette);
          memcpy(Principal_Palette,SCx_Palette,size);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Principal_Largeur_image=SCx_header.Width;
          Principal_Hauteur_image=SCx_header.Height;

          if (!SCx_header.Plans)
          { // 256 couleurs (raw)
            LBM_Buffer=(byte *)malloc(Principal_Largeur_image);

            for (y_pos=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);y_pos++)
            {
              if (read_bytes(file,LBM_Buffer,Principal_Largeur_image))
                for (x_pos=0; x_pos<Principal_Largeur_image;x_pos++)
                  Pixel_de_chargement(x_pos,y_pos,LBM_Buffer[x_pos]);
              else
                Erreur_fichier=2;
            }
          }
          else
          { // moins de 256 couleurs (planar)
            size=((Principal_Largeur_image+7)>>3)*SCx_header.Plans;
            Vraie_taille=(size/SCx_header.Plans)<<3;
            LBM_Buffer=(byte *)malloc(size);
            HBPm1=SCx_header.Plans-1;
            Image_HAM=0;

            for (y_pos=0;(y_pos<Principal_Hauteur_image) && (!Erreur_fichier);y_pos++)
            {
              if (read_bytes(file,LBM_Buffer,size))
                Draw_ILBM_line(y_pos,Vraie_taille);
              else
                Erreur_fichier=2;
            }
          }
          free(LBM_Buffer);
        }
        else
          Erreur_fichier=1;
      }
    }
    else
      Erreur_fichier=1;

    fclose(file);
  }
  else
    Erreur_fichier=1;
}

// -- Sauver un fichier au format SCx ---------------------------------------
void Save_SCx(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_SCx_Header SCx_header;

  Nom_fichier_complet(Nom_du_fichier,1);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    T_Palette Palette_64;
    memcpy(Palette_64,Principal_Palette,sizeof(T_Palette));
    Palette_256_to_64(Palette_64);
    
    memcpy(SCx_header.Filler1,"RIX3",4);
    SCx_header.Width=Principal_Largeur_image;
    SCx_header.Height=Principal_Hauteur_image;
    SCx_header.Filler2=0xAF;
    SCx_header.Plans=0x00;

    if (write_bytes(file,&SCx_header,sizeof(T_SCx_Header)) &&
      write_bytes(file,&Palette_64,sizeof(T_Palette)))
    {
      Init_ecriture();

      for (y_pos=0; ((y_pos<Principal_Hauteur_image) && (!Erreur_fichier)); y_pos++)
        for (x_pos=0; x_pos<Principal_Largeur_image; x_pos++)
          Ecrire_octet(file,Lit_pixel_de_sauvegarde(x_pos,y_pos));

      Close_ecriture(file);
      fclose(file);

      if (Erreur_fichier)
        remove(Nom_du_fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
  }
  else
  {
    fclose(file);
    remove(Nom_du_fichier);
    Erreur_fichier=1;
  }
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PI1 ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//// DECODAGE d'une partie d'IMAGE ////

void PI1_8b_to_16p(byte * Src,byte * Dst)
{
  int  i;           // Indice du pixel à calculer
  word byte_mask;      // Masque de decodage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la source

  byte_mask=0x8000;
  w0=(((word)Src[0])<<8) | Src[1];
  w1=(((word)Src[2])<<8) | Src[3];
  w2=(((word)Src[4])<<8) | Src[5];
  w3=(((word)Src[6])<<8) | Src[7];
  for (i=0;i<16;i++)
  {
    // Pour décoder le pixel n°i, il faut traiter les 4 words sur leur bit
    // correspondant à celui du masque

    Dst[i]=((w0 & byte_mask)?0x01:0x00) |
           ((w1 & byte_mask)?0x02:0x00) |
           ((w2 & byte_mask)?0x04:0x00) |
           ((w3 & byte_mask)?0x08:0x00);
    byte_mask>>=1;
  }
}

//// CODAGE d'une partie d'IMAGE ////

void PI1_16p_to_8b(byte * Src,byte * Dst)
{
  int  i;           // Indice du pixel à calculer
  word byte_mask;      // Masque de codage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la destination

  byte_mask=0x8000;
  w0=w1=w2=w3=0;
  for (i=0;i<16;i++)
  {
    // Pour coder le pixel n°i, il faut modifier les 4 words sur leur bit
    // correspondant à celui du masque

    w0|=(Src[i] & 0x01)?byte_mask:0x00;
    w1|=(Src[i] & 0x02)?byte_mask:0x00;
    w2|=(Src[i] & 0x04)?byte_mask:0x00;
    w3|=(Src[i] & 0x08)?byte_mask:0x00;
    byte_mask>>=1;
  }
  Dst[0]=w0 >> 8;
  Dst[1]=w0 & 0x00FF;
  Dst[2]=w1 >> 8;
  Dst[3]=w1 & 0x00FF;
  Dst[4]=w2 >> 8;
  Dst[5]=w2 & 0x00FF;
  Dst[6]=w3 >> 8;
  Dst[7]=w3 & 0x00FF;
}

//// DECODAGE de la PALETTE ////

void PI1_Decoder_palette(byte * Src,byte * palette)
{
  int i;  // Numéro de la couleur traitée
  int ip; // Indice dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  ip=0;
  for (i=0;i<16;i++)
  {
    w=((word)Src[(i*2)+1]<<8) | Src[(i*2)+0];

    // Traitement des couleurs rouge, verte et bleue:
    palette[ip++]=(((w & 0x0007) <<  1) | ((w & 0x0008) >>  3)) << 4;
    palette[ip++]=(((w & 0x7000) >> 11) | ((w & 0x8000) >> 15)) << 4;
    palette[ip++]=(((w & 0x0700) >>  7) | ((w & 0x0800) >> 11)) << 4;
  }
}

//// CODAGE de la PALETTE ////

void PI1_Coder_palette(byte * palette,byte * Dst)
{
  int i;  // Numéro de la couleur traitée
  int ip; // Indice dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  ip=0;
  for (i=0;i<16;i++)
  {
    // Traitement des couleurs rouge, verte et bleue:
    w =(((word)(palette[ip]>>2) & 0x38) >> 3) | (((word)(palette[ip]>>2) & 0x04) <<  1); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 9) | (((word)(palette[ip]>>2) & 0x04) << 13); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 5) | (((word)(palette[ip]>>2) & 0x04) <<  9); ip++;

    Dst[(i*2)+0]=w & 0x00FF;
    Dst[(i*2)+1]=(w>>8);
  }
}


// -- Tester si un fichier est au format PI1 --------------------------------
void Test_PI1(void)
{
  FILE * file;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word Res;                 // Résolution de l'image


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size==32034) || (size==32066))
    {
      // Lecture et vérification de la résolution
      if (read_word_le(file,&Res))
      {
        if (Res==0x0000)
          Erreur_fichier=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PI1 -----------------------------------------
void Load_PI1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (read_bytes(file,buffer,32034))
      {
        // Initialisation de la preview
        Initialiser_preview(320,200,File_length_file(file),FORMAT_PI1);
        if (Erreur_fichier==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          PI1_Decoder_palette(buffer+2,(byte *)Principal_Palette);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Principal_Largeur_image=320;
          Principal_Hauteur_image=200;

          // Chargement/décompression de l'image
          ptr=buffer+34;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            for (x_pos=0;x_pos<(320>>4);x_pos++)
            {
              PI1_8b_to_16p(ptr,pixels+(x_pos<<4));
              ptr+=8;
            }
            for (x_pos=0;x_pos<320;x_pos++)
              Pixel_de_chargement(x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        Erreur_fichier=1;
      free(buffer);
    }
    else
      Erreur_fichier=1;
    fclose(file);
  }
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PI1 ---------------------------------------
void Save_PI1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32066);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_Coder_palette((byte *)Principal_Palette,buffer+2);
    // Codage de l'image
    ptr=buffer+34;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<Principal_Hauteur_image)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<Principal_Largeur_image);x_pos++)
          pixels[x_pos]=Lit_pixel_de_sauvegarde(x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    memset(buffer+32034,0,32); // 32 extra NULL bytes at the end of the file to make ST Deluxe Paint happy

    if (write_bytes(file,buffer,32066))
    {
      fclose(file);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
    // Libération du buffer mémoire
    free(buffer);
  }
  else
  {
    fclose(file);
    remove(Nom_du_fichier);
    Erreur_fichier=1;
  }
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PC1 ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//// DECOMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_Decompresser_PackBits(byte * Src,byte * Dst)
{
  int is,id; // Les indices de parcour des buffers
  int n;     // Octet de contrôle

  for (is=id=0;id<32000;)
  {
    n=Src[is++];

    if (n & 0x80)
    {
      // Recopier Src[is] -n+1 fois
      n=257-n;
      for (;(n>0) && (id<32000);n--)
        Dst[id++]=Src[is];
      is++;
    }
    else
    {
      // Recopier n+1 octets littéralement
      n=n+1;
      for (;(n>0) && (id<32000);n--)
        Dst[id++]=Src[is++];
    }

    // Contrôle des erreurs
    if (n>0)
      Erreur_fichier=1;
  }
}

//// COMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_Compresser_PackBits(byte * Src,byte * Dst,int TailleS,int * TailleD)
{
  int is; // Indice dans la source
  int id; // Indice dans la destination
  int ir; // Indice de   la répétition
  int n;  // Taille des séquences
  int repet; // "Il y a répétition"

  for (is=id=0;is<TailleS;)
  {
    // On recherche le 1er endroit où il y a répétition d'au moins 3 valeurs
    // identiques

    repet=0;
    for (ir=is;ir<TailleS-2;ir++)
    {
      if ((Src[ir]==Src[ir+1]) && (Src[ir+1]==Src[ir+2]))
      {
        repet=1;
        break;
      }
      if ((ir-is)+1==40)
        break;
    }

    // On code la partie sans répétitions
    if (ir!=is)
    {
      n=(ir-is)+1;
      Dst[id++]=n-1;
      for (;n>0;n--)
        Dst[id++]=Src[is++];
    }

    // On code la partie sans répétitions
    if (repet)
    {
      // On compte la quantité de fois qu'il faut répéter la valeur
      for (ir+=3;ir<TailleS;ir++)
      {
        if (Src[ir]!=Src[is])
          break;
        if ((ir-is)+1==40)
          break;
      }
      n=(ir-is);
      Dst[id++]=257-n;
      Dst[id++]=Src[is];
      is=ir;
    }
  }

  // On renseigne la taille du buffer compressé
  *TailleD=id;
}

//// DECODAGE d'une partie d'IMAGE ////

// Transformation de 4 plans de bits en 1 ligne de pixels

void PC1_4pb_to_1lp(byte * Src0,byte * Src1,byte * Src2,byte * Src3,byte * Dst)
{
  int  i,j;         // Compteurs
  int  ip;          // Indice du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    b0=Src0[i];
    b1=Src1[i];
    b2=Src2[i];
    b3=Src3[i];
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    for (j=0;j<8;j++)
    {
      Dst[ip++]=((b0 & byte_mask)?0x01:0x00) |
                ((b1 & byte_mask)?0x02:0x00) |
                ((b2 & byte_mask)?0x04:0x00) |
                ((b3 & byte_mask)?0x08:0x00);
      byte_mask>>=1;
    }
  }
}

//// CODAGE d'une partie d'IMAGE ////

// Transformation d'1 ligne de pixels en 4 plans de bits

void PC1_1lp_to_4pb(byte * Src,byte * Dst0,byte * Dst1,byte * Dst2,byte * Dst3)
{
  int  i,j;         // Compteurs
  int  ip;          // Indice du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    b0=b1=b2=b3=0;
    for (j=0;j<8;j++)
    {
      b0|=(Src[ip] & 0x01)?byte_mask:0x00;
      b1|=(Src[ip] & 0x02)?byte_mask:0x00;
      b2|=(Src[ip] & 0x04)?byte_mask:0x00;
      b3|=(Src[ip] & 0x08)?byte_mask:0x00;
      ip++;
      byte_mask>>=1;
    }
    Dst0[i]=b0;
    Dst1[i]=b1;
    Dst2[i]=b2;
    Dst3[i]=b3;
  }
}


// -- Tester si un fichier est au format PC1 --------------------------------
void Test_PC1(void)
{
  FILE *file;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word Res;                 // Résolution de l'image


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size<=32066))
    {
      // Lecture et vérification de la résolution
      if (read_word_le(file,&Res))
      {
        if (Res==0x0080)
          Erreur_fichier=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PC1 -----------------------------------------
void Load_PC1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  int  size;
  word x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    size=File_length_file(file);
    // allocation des buffers mémoire
    buffercomp=(byte *)malloc(size);
    bufferdecomp=(byte *)malloc(32000);
    if ( (buffercomp!=NULL) && (bufferdecomp!=NULL) )
    {
      // Lecture du fichier dans le buffer
      if (read_bytes(file,buffercomp,size))
      {
        // Initialisation de la preview
        Initialiser_preview(320,200,File_length_file(file),FORMAT_PC1);
        if (Erreur_fichier==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          PI1_Decoder_palette(buffercomp+2,(byte *)Principal_Palette);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Principal_Largeur_image=320;
          Principal_Hauteur_image=200;

          // Décompression du buffer
          PC1_Decompresser_PackBits(buffercomp+34,bufferdecomp);

          // Décodage de l'image
          ptr=bufferdecomp;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            // Décodage de la scanline
            PC1_4pb_to_1lp(ptr,ptr+40,ptr+80,ptr+120,pixels);
            ptr+=160;
            // Chargement de la ligne
            for (x_pos=0;x_pos<320;x_pos++)
              Pixel_de_chargement(x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        Erreur_fichier=1;
      free(bufferdecomp);
      free(buffercomp);
    }
    else
    {
      Erreur_fichier=1;
      if (bufferdecomp) free(bufferdecomp);
      if (buffercomp)   free(buffercomp);
    }
    fclose(file);
  }
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PC1 ---------------------------------------
void Save_PC1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  int   size;
  short x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    // Allocation des buffers mémoire
    bufferdecomp=(byte *)malloc(32000);
    buffercomp  =(byte *)malloc(64066);
    // Codage de la résolution
    buffercomp[0]=0x80;
    buffercomp[1]=0x00;
    // Codage de la palette
    PI1_Coder_palette((byte *)Principal_Palette,buffercomp+2);
    // Codage de l'image
    ptr=bufferdecomp;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<Principal_Hauteur_image)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<Principal_Largeur_image);x_pos++)
          pixels[x_pos]=Lit_pixel_de_sauvegarde(x_pos,y_pos);
      }

      // Encodage de la scanline
      PC1_1lp_to_4pb(pixels,ptr,ptr+40,ptr+80,ptr+120);
      ptr+=160;
    }

    // Compression du buffer
    PC1_Compresser_PackBits(bufferdecomp,buffercomp+34,32000,&size);
    size+=34;
    for (x_pos=0;x_pos<16;x_pos++)
      buffercomp[size++]=0;

    if (write_bytes(file,buffercomp,size))
    {
      fclose(file);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
    // Libération des buffers mémoire
    free(bufferdecomp);
    free(buffercomp);
  }
  else
  {
    fclose(file);
    remove(Nom_du_fichier);
    Erreur_fichier=1;
  }
}

/******************************************************************************************
  Functions from op_c.c : load raw 24B (for testing) and load_tga (broken ? incomplete ?)
  Saving would be nice as well.
******************************************************************************************/
void Load_RAW_24B(int width,int height,Bitmap24B Source)
{
  FILE* file;

  file=fopen("TEST.RAW","rb");
  if (read_bytes(file,Source,width*height*sizeof(Composantes)))
    exit(3);
  fclose(file);
}

void Load_TGA(char * fname,Bitmap24B * dest,int * width,int * height)
{
  FILE* fichier;
  struct
  {
    byte Id_field_size;        // Taille des données spécifiques placées après le header
    byte Color_map_type;       // Présence d'une palette
    byte Image_type_code;      // Type d'image
    word Color_map_origin;     // Indice de départ de la palette
    word Color_map_length;     // Taille de la palette
    byte Color_map_entry_size; // Palette sur 16, 24 ou 32 bits
    word X_origin;             // Coordonnées de départ
    word Y_origin;
    word Width;                // width de l'image
    word Height;               // height de l'image
    byte Pixel_size;           // Pixels sur 16, 24 ou 32 bits
    byte Descriptor;           // Paramètres divers
  } TGA_header;
  int x,y,py,skip,t;
  byte * buffer;

  fichier=fopen(fname,"rb");
  read_bytes(fichier,&TGA_header,sizeof(TGA_header));
  if (TGA_header.Image_type_code==2)
  {
    *width=TGA_header.Width;
    *height=TGA_header.Height;
    *dest=(Bitmap24B)malloc((*width)*(*height)*3);

    // On saute l'ID field
    fseek(fichier,TGA_header.Id_field_size,SEEK_CUR);

    // On saute la palette
    if (TGA_header.Color_map_type==0)
      skip=0;
    else
    {
      skip=TGA_header.Color_map_length;
      if (TGA_header.Color_map_entry_size==16)
        skip*=2;
      else
      if (TGA_header.Color_map_entry_size==24)
        skip*=3;
      else
      if (TGA_header.Color_map_entry_size==32)
        skip*=4;
    }
    fseek(fichier,skip,SEEK_CUR);

    // Lecture des pixels
    skip=(*width);
    if (TGA_header.Pixel_size==16)
      skip*=2;
    else
    if (TGA_header.Pixel_size==24)
      skip*=3;
    else
    if (TGA_header.Pixel_size==32)
      skip*=4;

    buffer=(byte *)malloc(skip);
    for (y=0;y<(*height);y++)
    {
      read_bytes(fichier,buffer,skip);

      // Inversion du rouge et du bleu
      for (x=0;x<(*width);x++)
      {
        t=buffer[(x*3)+0];
        buffer[(x*3)+0]=buffer[(x*3)+2];
        buffer[(x*3)+2]=t;
      }

      // Prise en compte du sens d'écriture verticale
      if (TGA_header.Descriptor & 0x20)
        py=y;
      else
        py=(*height)-y-1;

      // Prise en compte de l'interleave verticale
      if (TGA_header.Descriptor & 0xC0)
        py=((py % (*height))*2)+(py/(*height));

      memcpy((*dest)+(py*(*width)),buffer,skip);
    }
    free(buffer);
  }
  fclose(fichier);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PNG ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifndef __no_pnglib__

// -- Tester si un fichier est au format PNG --------------------------------
void Test_PNG(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  byte PNG_header[8];
  
  Nom_fichier_complet(Nom_du_fichier,0);
  
  Erreur_fichier=1;

  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture du header du fichier
    if (read_bytes(file,PNG_header,8))
    {
      if ( !png_sig_cmp(PNG_header, 0, 8))
        Erreur_fichier=0;
    }
    fclose(file);
  }
}

png_bytep * row_pointers;
// -- Lire un fichier au format PNG -----------------------------------------
void Load_PNG(void)
{
  FILE *file;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  byte PNG_header[8];  
  dword Taille_image;
 
  png_structp png_ptr;
  png_infop info_ptr;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  
  if ((file=fopen(Nom_du_fichier, "rb")))
  {
    if (read_bytes(file,PNG_header,8))
    {
      if ( !png_sig_cmp(PNG_header, 0, 8))
      {
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr)
        {
          info_ptr = png_create_info_struct(png_ptr);
          if (info_ptr)
          {
            png_byte color_type;
            png_byte bit_depth;
            
            if (!setjmp(png_jmpbuf(png_ptr)))
            {
              png_init_io(png_ptr, file);
              png_set_sig_bytes(png_ptr, 8);
            
              png_read_info(png_ptr, info_ptr);
              color_type = info_ptr->color_type;
              bit_depth = info_ptr->bit_depth;
              
              if (bit_depth <= 8 && (color_type == PNG_COLOR_TYPE_PALETTE || PNG_COLOR_TYPE_GRAY))
              {
                int num_text;
                png_text *text_ptr;
                
                int unit_type;
                png_uint_32 res_x;
                png_uint_32 res_y;

                // Commentaire (tEXt)
                Principal_Commentaire[0]='\0'; // On efface le commentaire
                if ((num_text=png_get_text(png_ptr, info_ptr, &text_ptr, NULL)))
                {
                  while (num_text--)
                  {
                    if (!strcmp(text_ptr[num_text].key,"Title"))
                    {
                      int size;
                      size = Min(text_ptr[num_text].text_length, TAILLE_COMMENTAIRE);
                      strncpy(Principal_Commentaire, text_ptr[num_text].text, size);
                      Principal_Commentaire[size]='\0';
                      break; // Pas besoin de vérifier les suivants
                    }
                  }
                }
                // Pixel Ratio (pHYs)
                if (png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type))
                {
                  // Ignore unit, and use the X/Y ratio as a hint for
                  // WIDE or TALL pixels
                  if (res_x>0 && res_y>0)
                  {
                    if (res_y/res_x>1)
                    {
                      Ratio_image_chargee=PIXEL_WIDE;
                    }
                    else if (res_x/res_y>1)
                    {
                      Ratio_image_chargee=PIXEL_TALL;
                    }
                  }
                }
                Initialiser_preview(info_ptr->width,info_ptr->height,File_length_file(file),FORMAT_PNG);

                if (Erreur_fichier==0)
                {
                  int x,y;
                  png_colorp palette;
                  int num_palette;
                                    
                  if (color_type == PNG_COLOR_TYPE_GRAY)
                  {
                    if (bit_depth < 8)
                      png_set_gray_1_2_4_to_8(png_ptr);
                    // palette de niveaux de gris
                    for (x=0;x<num_palette;x++)
                    {
                      Principal_Palette[x].R=x;
                      Principal_Palette[x].G=x;
                      Principal_Palette[x].B=x;
                    } 
                  }
                  else
                  {
                    // image couleurs
                    if (bit_depth < 8)
                    {
                      png_set_packing(png_ptr);
                      if (Config.Clear_palette)
                        memset(Principal_Palette,0,sizeof(T_Palette));
                    }
                    png_get_PLTE(png_ptr, info_ptr, &palette,
                       &num_palette);
                    for (x=0;x<num_palette;x++)
                    {
                      Principal_Palette[x].R=palette[x].red;
                      Principal_Palette[x].G=palette[x].green;
                      Principal_Palette[x].B=palette[x].blue;
                    }
                    free(palette);
                  }
                  Set_palette(Principal_Palette);
                  Remapper_fileselect();
                  //
                  
                  Principal_Largeur_image=info_ptr->width;
                  Principal_Hauteur_image=info_ptr->height;
                  Taille_image=(dword)(Principal_Largeur_image*Principal_Hauteur_image);
                
                  png_set_interlace_handling(png_ptr);
                  png_read_update_info(png_ptr, info_ptr);
                  /* read file */
                  if (!setjmp(png_jmpbuf(png_ptr)))
                  {
                    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * Principal_Hauteur_image);
                    for (y=0; y<Principal_Hauteur_image; y++)
                      row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);
                    png_read_image(png_ptr, row_pointers);
                    
                    for (y=0; y<Principal_Hauteur_image; y++)
                      for (x=0; x<Principal_Largeur_image; x++)
                        Pixel_de_chargement(x, y, row_pointers[y][x]);
                    
                  }
                  else
                    Erreur_fichier=2;
                    
                  /* cleanup heap allocation */
                  for (y=0; y<Principal_Hauteur_image; y++)
                    free(row_pointers[y]);
                  free(row_pointers);
                }
                else
                  Erreur_fichier=2;
              }
              else
               Erreur_fichier=1;
            }
            else
             Erreur_fichier=1;
          }
          else
            Erreur_fichier=1;
        }
      }
      /*Close_lecture();*/
    }
    else // Lecture header impossible: Erreur ne modifiant pas l'image
      Erreur_fichier=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Erreur ne modifiant pas l'image
    Erreur_fichier=1;
}

void Save_PNG(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *file;
  int y;
  byte * PixelPtr;
  png_structp png_ptr;
  png_infop info_ptr;
  
  Nom_fichier_complet(Nom_du_fichier,0);
  Erreur_fichier=0;
  row_pointers = NULL;
  
  // Ouverture du fichier
  if ((file=fopen(Nom_du_fichier,"wb")))
  {
    /* initialisation */
    if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))
      && (info_ptr = png_create_info_struct(png_ptr)))
    {
  
      if (!setjmp(png_jmpbuf(png_ptr)))
      {    
        png_init_io(png_ptr, file);
      
        /* en-tete */
        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          png_set_IHDR(png_ptr, info_ptr, Principal_Largeur_image, Principal_Hauteur_image,
            8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

          png_set_PLTE(png_ptr, info_ptr, (png_colorp)Principal_Palette, 256);
          {
            // Commentaires texte PNG
            // Cette partie est optionnelle
            png_text text_ptr[2] = {
              {-1, "Software", "Grafx2", 6},
              {-1, "Title", NULL, 0}
            };
            int Nb_texte=1;
            if (Principal_Commentaire[0])
            {
              text_ptr[1].text=Principal_Commentaire;
              text_ptr[1].text_length=strlen(Principal_Commentaire);
              Nb_texte=2;
            }
            png_set_text(png_ptr, info_ptr, text_ptr, Nb_texte);
          }
          switch(Pixel_ratio)
          {
            case PIXEL_WIDE:
              png_set_pHYs(png_ptr, info_ptr, 3000, 6000, PNG_RESOLUTION_METER);
              break;
            case PIXEL_TALL:
              png_set_pHYs(png_ptr, info_ptr, 6000, 3000, PNG_RESOLUTION_METER);
              break;
            default:
              break;
          }          
          png_write_info(png_ptr, info_ptr);

          /* ecriture des pixels de l'image */
          row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * Principal_Hauteur_image);
          PixelPtr = (Lit_pixel_de_sauvegarde==Lit_pixel_dans_ecran_courant)?Principal_Ecran:Brosse;
          for (y=0; y<Principal_Hauteur_image; y++)
            row_pointers[y] = (png_byte*)(PixelPtr+y*Principal_Largeur_image);

          if (!setjmp(png_jmpbuf(png_ptr)))
          {
            png_write_image(png_ptr, row_pointers);
          
            /* cloture png */
            if (!setjmp(png_jmpbuf(png_ptr)))
            {          
              png_write_end(png_ptr, NULL);
            }
            else
              Erreur_fichier=1;
          }
          else
            Erreur_fichier=1;
        }
        else
          Erreur_fichier=1;
      }
      else
      {
        Erreur_fichier=1;
      }
      png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    else
      Erreur_fichier=1;
    // fermeture du fichier
    fclose(file);
  }

  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri trainait... Ca fait pas propre.
  if (Erreur_fichier)
    remove(Nom_du_fichier);
  
  if (row_pointers)
  {
    free(row_pointers);
    row_pointers=NULL;
  }
}
#endif  // __no_pnglib__

// Saves an image.
// This routine will only be called when all hope is lost, memory thrashed, etc
// It's the last chance to save anything, but the code has to be extremely careful,
// anything could happen.
// The chosen format is IMG since it's extremely simple, difficult to make it create an unusable image.
void Emergency_backup(const char *Fname, byte *Source, int width, int height, T_Palette *palette)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_Header_IMG IMG_header;

  strcpy(Nom_du_fichier,Repertoire_de_configuration);
  strcat(Nom_du_fichier,Fname);

  // Ouverture du fichier
  file=fopen(Nom_du_fichier,"wb");
  if (!file)
    return;

  memcpy(IMG_header.Filler1,"\x01\x00\x47\x12\x6D\xB0",6);
  memset(IMG_header.Filler2,0,118);
  IMG_header.Filler2[4]=0xFF;
  IMG_header.Filler2[22]=64; // Lo(Longueur de la signature)
  IMG_header.Filler2[23]=0;  // Hi(Longueur de la signature)
  memcpy(IMG_header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

  if (!write_bytes(file,IMG_header.Filler1,6) ||
      !write_word_le(file,width) ||
      !write_word_le(file,height) ||
      !write_bytes(file,IMG_header.Filler2,118) ||
      !write_bytes(file,palette,sizeof(T_Palette)))
    {
      fclose(file);
      return;
    }

  for (y_pos=0; ((y_pos<height) && (!Erreur_fichier)); y_pos++)
    for (x_pos=0; x_pos<width; x_pos++)
      if (!write_byte(file,*(Source+y_pos*width+x_pos)))
      {
        fclose(file);
        return;
      }

  // Ouf, sauvé
  fclose(file);
}

void Image_emergency_backup()
{
  Emergency_backup("phoenix.img",Principal_Ecran, Principal_Largeur_image, Principal_Hauteur_image, &Principal_Palette);
  Emergency_backup("phoenix2.img",Brouillon_Ecran, Brouillon_Largeur_image, Brouillon_Hauteur_image, &Brouillon_Palette);
}
