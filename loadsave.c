#define _XOPEN_SOURCE

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include "pages.h"
#include "op_c.h"
#include "boutons.h"
#include "erreurs.h"
#include "linux.h"
#include "io.h"

#define FILENAMESPACE 13

// Chargement des pixels dans l'écran principal
void Pixel_Chargement_dans_ecran_courant(word Pos_X,word Pos_Y,byte Couleur)
{
  //if ((Pos_X>=0) && (Pos_Y>=0)) //Toujours vrai ?
  if ((Pos_X<Principal_Largeur_image) && (Pos_Y<Principal_Hauteur_image))
    Pixel_dans_ecran_courant(Pos_X,Pos_Y,Couleur);
}


// Chargement des pixels dans la brosse
void Pixel_Chargement_dans_brosse(word Pos_X,word Pos_Y,byte Couleur)
{
  //if ((Pos_X>=0) && (Pos_Y>=0))
  if ((Pos_X<Brosse_Largeur) && (Pos_Y<Brosse_Hauteur))
    Pixel_dans_brosse(Pos_X,Pos_Y,Couleur);
}


short Preview_Facteur_X;
short Preview_Facteur_Y;
short Preview_Pos_X;
short Preview_Pos_Y;

byte HBPm1; // Header.BitPlanes-1


// Chargement des pixels dans la preview
void Pixel_Chargement_dans_preview(word Pos_X,word Pos_Y,byte Couleur)
{
  if (((Pos_X % Preview_Facteur_X)==0) && ((Pos_Y % Preview_Facteur_Y)==0))
  if ((Pos_X<Principal_Largeur_image) && (Pos_Y<Principal_Hauteur_image))
    Pixel(Preview_Pos_X+(Pos_X/Preview_Facteur_X),
          Preview_Pos_Y+(Pos_Y/Preview_Facteur_Y),
          Couleur);
}


void Remapper_fileselect(void)
{
  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
  {
    Calculer_couleurs_menu_optimales(Principal_Palette);
    Remapper_ecran_apres_changement_couleurs_menu();
  }
}



// Données pour la gestion du chargement en 24b
#define FORMAT_24B 0x100
typedef void (* fonction_afficheur_24b) (short,short,byte,byte,byte);
int                    Image_24b;
struct Composantes *   Buffer_image_24b;
fonction_afficheur_24b Pixel_Chargement_24b;


// Chargement des pixels dans le buffer 24b
void Pixel_Chargement_dans_buffer_24b(short Pos_X,short Pos_Y,byte R,byte V,byte B)
{
  int indice;

  if ((Pos_X>=0) && (Pos_Y>=0))
  if ((Pos_X<Principal_Largeur_image) && (Pos_Y<Principal_Hauteur_image))
  {
    indice=(Pos_Y*Principal_Largeur_image)+Pos_X;
    Buffer_image_24b[indice].R=R;
    Buffer_image_24b[indice].V=V;
    Buffer_image_24b[indice].B=B;
  }
}

// Chargement des pixels dans la preview en 24b
void Pixel_Chargement_dans_preview_24b(short Pos_X,short Pos_Y,byte R,byte V,byte B)
{
  byte Couleur;

  if (((Pos_X % Preview_Facteur_X)==0) && ((Pos_Y % Preview_Facteur_Y)==0))
  if ((Pos_X<Principal_Largeur_image) && (Pos_Y<Principal_Hauteur_image))
  {
    Couleur=((R >> 5) << 5) |
            ((V >> 5) << 2) |
            ((B >> 6));
    Pixel(Preview_Pos_X+(Pos_X/Preview_Facteur_X),
          Preview_Pos_Y+(Pos_Y/Preview_Facteur_Y),
          Couleur);
  }
}

// Création d'une palette fake
void Palette_fake_24b(T_Palette Palette)
{
  int Couleur;

  // Génération de la palette
  for (Couleur=0;Couleur<256;Couleur++)
  {
    Palette[Couleur].R=((Couleur & 0xE0)>>5)<<3;
    Palette[Couleur].V=((Couleur & 0x1C)>>2)<<3;
    Palette[Couleur].B=((Couleur & 0x03)>>0)<<4;
  }
}

// Supplément à faire lors de l'initialisation d'une preview dans le cas
// d'une image 24b
void Initialiser_preview_24b(int Largeur,int Hauteur)
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
      (struct Composantes *)Emprunter_memoire_de_page(Largeur*Hauteur*sizeof(struct Composantes));
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




void Initialiser_preview(short Largeur,short Hauteur,long Taille,int Format)
//
//   Cette procédure doit être appelée par les routines de chargement
// d'images.
//   Elle doit être appelée entre le moment où l'on connait la dimension de
// l'image (dimension réelle, pas dimension tronquée) et l'affichage du
// premier point.
//
{
  char  Chaine[256];
  int   Image_en_24b;

  Image_en_24b=Format & FORMAT_24B;
  Format      =Format & (~FORMAT_24B);

  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
  {
    // Préparation du chargement d'une preview:

    // Affichage des données "Image size:"
    if ((Largeur<10000) && (Hauteur<10000))
    {
      Num2str(Largeur,Chaine,4);
      Num2str(Hauteur,Chaine+5,4);
      Chaine[4]='x';
      Print_dans_fenetre(226,55,Chaine,CM_Noir,CM_Clair);
    }
    else
    {
      Print_dans_fenetre(226,55,"VERY BIG!",CM_Noir,CM_Clair);
    }

    // Affichage de la taille du fichier
    if (Taille<1048576)
    {
      // Le fichier fait moins d'un Mega, on affiche sa taille direct
      Num2str(Taille,Chaine,9);
      Print_dans_fenetre(226,63,Chaine,CM_Noir,CM_Clair);
    }
    else if ((Taille/1024)<10000000)
    {
      // Le fichier fait plus d'un Mega, on peut afficher sa taille en Ko
      Num2str(Taille/1024,Chaine,7);
      Chaine[7]='K';
      Chaine[8]='b';
      Print_dans_fenetre(226,63,Chaine,CM_Noir,CM_Clair);
    }
    else
    {
      // Le fichier fait plus de 10 Giga octets (cas très rare :))
      Print_dans_fenetre(226,63,"TOO BIG!!",CM_Noir,CM_Clair);
    }

    // Affichage du vrai format
    if (Format!=Principal_Format)
    {
      Print_dans_fenetre( 274,72,Format_Extension[Format-1],CM_Noir,CM_Clair);
    }

    // On efface le commentaire précédent
    Block(Fenetre_Pos_X+46*Menu_Facteur_X,Fenetre_Pos_Y+(175+FILENAMESPACE)*Menu_Facteur_Y,
          Menu_Facteur_X<<8,Menu_Facteur_Y<<3,CM_Clair);
    // Affichage du commentaire
    if (Format_Commentaire[Format-1])
      Print_dans_fenetre(46,175+FILENAMESPACE,Principal_Commentaire,CM_Noir,CM_Clair);

    // Calculs des données nécessaires à l'affichage de la preview:
    Preview_Facteur_X=Round_div_max(Largeur,122*Menu_Facteur_X);
    Preview_Facteur_Y=Round_div_max(Hauteur, 82*Menu_Facteur_Y);

    if ( (!Config.Maximize_preview) && (Preview_Facteur_X!=Preview_Facteur_Y) )
    {
      if (Preview_Facteur_X>Preview_Facteur_Y)
        Preview_Facteur_Y=Preview_Facteur_X;
      else
        Preview_Facteur_X=Preview_Facteur_Y;
    }

    Preview_Pos_X=Fenetre_Pos_X+180*Menu_Facteur_X;
    Preview_Pos_Y=Fenetre_Pos_Y+ (89+FILENAMESPACE)*Menu_Facteur_Y;

    // On nettoie la zone où va s'afficher la preview:
    Block(Preview_Pos_X,Preview_Pos_Y,
          Round_div_max(Largeur,Preview_Facteur_X),
          Round_div_max(Hauteur,Preview_Facteur_Y),
          CM_Noir);
  }
  else
  {
    if (Pixel_de_chargement==Pixel_Chargement_dans_ecran_courant)
    {
      if (Backup_avec_nouvelles_dimensions(0,Largeur,Hauteur))
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
      Brosse=(byte *)malloc(Largeur*Hauteur);
      Brosse_Largeur=Largeur;
      Brosse_Hauteur=Hauteur;
      if (Brosse)
      {
        Smear_Brosse=(byte *)malloc(Largeur*Hauteur);
        if (!Smear_Brosse)
          Erreur_fichier=3;
      }
      else
        Erreur_fichier=3;
    }
  }

  if (!Erreur_fichier)
    if (Image_en_24b)
      Initialiser_preview_24b(Largeur,Hauteur);
}



void Dessiner_preview_palette(void)
{
  short Indice;
  short Preview_Pos_X=Fenetre_Pos_X+186*Menu_Facteur_X;
  short Preview_Pos_Y=Fenetre_Pos_Y+ (90+FILENAMESPACE)*Menu_Facteur_Y;

  if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
    for (Indice=0; Indice<256; Indice++)
      Block(Preview_Pos_X+(((Indice>>4)*7)*Menu_Facteur_X),
            Preview_Pos_Y+(((Indice&15)*5)*Menu_Facteur_Y),
            5*Menu_Facteur_X,5*Menu_Facteur_Y,Indice);

  SDL_UpdateRect(Ecran_SDL,
    Preview_Pos_X*Menu_Facteur_X,
  Preview_Pos_Y*Menu_Facteur_Y,
  5*Menu_Facteur_X*256,5*Menu_Facteur_Y*256);
}



// Calcul du nom complet du fichier
void Nom_fichier_complet(char * Nom_du_fichier, byte Sauve_Colorix)
{
    byte Pos;

    strcpy(Nom_du_fichier,Principal_Repertoire_fichier);

    //On va ajouter un / à la fin du chemin s'il n'y est pas encore
    //Attention sous windows il faut un \...
    if (Nom_du_fichier[strlen(Nom_du_fichier)-1]!='/')
        strcat(Nom_du_fichier,"/");

  // Si on est en train de sauvegarder une image Colorix, on calcule son ext.
  if (Sauve_Colorix)
  {
    Pos=strlen(Principal_Nom_fichier)-1;
    if (Principal_Nom_fichier[Pos]=='?')
    {
      if (Principal_Largeur_image<=320)
        Principal_Nom_fichier[Pos]='I';
      else
      {
        if (Principal_Largeur_image<=360)
          Principal_Nom_fichier[Pos]='Q';
        else
        {
          if (Principal_Largeur_image<=640)
            Principal_Nom_fichier[Pos]='F';
          else
          {
            if (Principal_Largeur_image<=800)
              Principal_Nom_fichier[Pos]='N';
            else
              Principal_Nom_fichier[Pos]='O';
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

void Lire_octet(FILE * Fichier, byte *Octet)
{
  // FIXME : Remplacer les appelants par read_bytes(), et gérer les retours d'erreur.
  if (!read_byte(Fichier, Octet))
  ;//  Erreur_fichier = 2;
}

// --------------------------------------------------------------------------

byte * Tampon_ecriture;
word   Index_ecriture;

void Init_ecriture(void)
{
  Tampon_ecriture=(byte *)malloc(64000);
  Index_ecriture=0;
}

void Ecrire_octet(FILE *Fichier, byte Octet)
{
  Tampon_ecriture[Index_ecriture++]=Octet;
  if (Index_ecriture>=64000)
  {
    if (! write_bytes(Fichier,Tampon_ecriture,64000))
      Erreur_fichier=1;
    Index_ecriture=0;
  }
}

void Close_ecriture(FILE *Fichier)
{
  if (Index_ecriture)
    if (! write_bytes(Fichier,Tampon_ecriture,Index_ecriture))
      Erreur_fichier=1;
  free(Tampon_ecriture);
}



/////////////////////////////////////////////////////////////////////////////

void (/*__interrupt __near*/ *Ancien_handler_clavier)(void);
void /*__interrupt __near*/ Nouveau_handler_clavier(void)
{
  /* A revoir ...
  _disable();
  if (!Erreur_fichier)
    Erreur_fichier=-1;
  _enable();
  _chain_intr(Ancien_handler_clavier);
  */
  ;
}


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
void Charger_image(byte Image)
{
  int  Indice; // Indice de balayage des formats
  int  Format=0; // Format du fichier à charger


  // On place par défaut Erreur_fichier à vrai au cas où on ne sache pas
  // charger le format du fichier:
  Erreur_fichier=1;

  if (Principal_Format!=0)
  {
    Format_Test[Principal_Format-1]();
    if (!Erreur_fichier)
      // Si dans le sélecteur il y a un format valide on le prend tout de suite
      Format=Principal_Format-1;
  }

  if (Erreur_fichier)
  {
    //  Sinon, on va devoir scanner les différents formats qu'on connait pour
    // savoir à quel format est le fichier:
    for (Indice=0;Indice<NB_FORMATS_LOAD;Indice++)
    {
      // On appelle le testeur du format:
      Format_Test[Indice]();
      // On s'arrête si le fichier est au bon format:
      if (Erreur_fichier==0)
      {
        Format=Indice;
        break;
      }
    }
  }

  // Si on a su déterminer avec succès le format du fichier:
  if (!Erreur_fichier)
  {
    // Installer le handler d'interruption du clavier pour stopper une preview
    if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
    {
        /* A revoir ...
      Ancien_handler_clavier=_dos_getvect(9);
      _dos_setvect(9,Nouveau_handler_clavier);
      */;
    }

    // On peut charger le fichier:
    Image_24b=0;
    Format_Load[Format]();

    // Désinstaller le handler d'interruption du clavier
    if (Pixel_de_chargement==Pixel_Chargement_dans_preview)
    /*A revoir ...
      _dos_setvect(9,Ancien_handler_clavier);
      */;

    if (Erreur_fichier>0)
      Erreur(0);

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
        if (!Erreur_fichier)
          Palette_256_to_64(Principal_Palette);
      }

      free(Buffer_image_24b);
    }

    if (Image)
    {
      if ( (Erreur_fichier!=1) && (Format_Backup_done[Format]) )
      {
        // On considère que l'image chargée n'est plus modifiée
        Principal_Image_modifiee=0;
        // Et on documente la variable Principal_Format_fichier avec la valeur:
        Principal_Format_fichier=Format+1;

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
        Principal_Format_fichier=Format+1;
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
void Sauver_image(byte Image)
{
  // On place par défaut Erreur_fichier à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  Erreur_fichier=1;

  Lit_pixel_de_sauvegarde=(Image)?Lit_pixel_dans_ecran_courant:Lit_pixel_dans_brosse;

  Format_Save[Principal_Format_fichier-1]();

  if (Erreur_fichier)
    Erreur(0);
  else
  {
    if ((Image) && (Format_Backup_done[Principal_Format_fichier-1]))
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
  FILE *Fichier;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  long Taille_du_fichier;   // Taille du fichier
  struct stat Informations_Fichier;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    stat(Nom_du_fichier,&Informations_Fichier);
    // Lecture de la taille du fichier
    Taille_du_fichier=Informations_Fichier.st_size;
    fclose(Fichier);
    // Le fichier ne peut être au format PAL que si sa taille vaut 768 octets
    if (Taille_du_fichier==sizeof(T_Palette))
      Erreur_fichier=0;
  }
}


// -- Lire un fichier au format PAL -----------------------------------------
void Load_PAL(void)
{
  FILE *Fichier;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //long  Taille_du_fichier;   // Taille du fichier


  Nom_fichier_complet(Nom_du_fichier,0);
  Erreur_fichier=0;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Initialiser_preview(???); // Pas possible... pas d'image...

    // Lecture du fichier dans Principal_Palette
    if (read_bytes(Fichier,Principal_Palette,sizeof(T_Palette)))
    {
      Set_palette(Principal_Palette);
      Remapper_fileselect();

      // On dessine une preview de la palette (si chargement=preview)
      Dessiner_preview_palette();
    }
    else
      Erreur_fichier=2;

    // Fermeture du fichier
    fclose(Fichier);
  }
  else
    // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PAL ---------------------------------------
void Save_PAL(void)
{
  FILE *Fichier;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //long Taille_du_fichier;   // Taille du fichier

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // Enregistrement de Principal_Palette dans le fichier
    if (! write_bytes(Fichier,Principal_Palette,sizeof(T_Palette)))
    {
      Erreur_fichier=1;
      fclose(Fichier);
      remove(Nom_du_fichier);
    }
    else // Ecriture correcte => Fermeture normale du fichier
      fclose(Fichier);
  }
  else // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
  {
    Erreur_fichier=1;
    fclose(Fichier);
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
  word Largeur;
  word Hauteur;
  byte Filler2[118];
  T_Palette Palette;
} __attribute__((__packed__)) T_Header_IMG;

// -- Tester si un fichier est au format IMG --------------------------------
void Test_IMG(void)
{
  FILE *Fichier;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_IMG IMG_Header;
  byte Signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture et vérification de la signature
    if (read_bytes(Fichier,&IMG_Header,sizeof(T_Header_IMG)))
    {
      if ( (!memcmp(IMG_Header.Filler1,Signature,6))
        && IMG_Header.Largeur && IMG_Header.Hauteur)
        Erreur_fichier=0;
    }
    // Fermeture du fichier
    fclose(Fichier);
  }
}


// -- Lire un fichier au format IMG -----------------------------------------
void Load_IMG(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  byte * Buffer;
  FILE *Fichier;
  word Pos_X,Pos_Y;
  long Largeur_lue;
  long Taille_du_fichier;
  T_Header_IMG IMG_Header;
  struct stat Informations_Fichier;


  stat(Nom_du_fichier,&Informations_Fichier);
  
  Nom_fichier_complet(Nom_du_fichier,0);
  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=Informations_Fichier.st_size;

    if (read_bytes(Fichier,&IMG_Header,sizeof(T_Header_IMG)))
    {
      Buffer=(byte *)malloc(IMG_Header.Largeur);

      Initialiser_preview(IMG_Header.Largeur,IMG_Header.Hauteur,Taille_du_fichier,FORMAT_IMG);
      if (Erreur_fichier==0)
      {
        //   On commence par passer la palette en 256 comme ça, si la nouvelle
        // palette a moins de 256 coul, la précédente ne souffrira pas d'un
        // assombrissement préjudiciable.
        Palette_64_to_256(Principal_Palette);
        //   On peut maintenant transférer la nouvelle palette
        memcpy(Principal_Palette,IMG_Header.Palette,sizeof(T_Palette));
        Palette_256_to_64(Principal_Palette);
        Set_palette(Principal_Palette);
        Remapper_fileselect();

        Principal_Largeur_image=IMG_Header.Largeur;
        Principal_Hauteur_image=IMG_Header.Hauteur;
        Largeur_lue=IMG_Header.Largeur;

        for (Pos_Y=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);Pos_Y++)
        {
          if (read_bytes(Fichier,Buffer,Principal_Largeur_image))
          {
            for (Pos_X=0; Pos_X<Principal_Largeur_image;Pos_X++)
              Pixel_de_chargement(Pos_X,Pos_Y,Buffer[Pos_X]);
          }
          else
            Erreur_fichier=2;
        }
      }

      free(Buffer);
    }
    else
      Erreur_fichier=1;

    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}

// -- Sauver un fichier au format IMG ---------------------------------------
void Save_IMG(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  short Pos_X,Pos_Y;
  T_Header_IMG IMG_Header;
  byte Signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    memcpy(IMG_Header.Filler1,Signature,6);

    IMG_Header.Largeur=Principal_Largeur_image;
    IMG_Header.Hauteur=Principal_Hauteur_image;

    memset(IMG_Header.Filler2,0,118);
    IMG_Header.Filler2[4]=0xFF;
    IMG_Header.Filler2[22]=64; // Lo(Longueur de la signature)
    IMG_Header.Filler2[23]=0;  // Hi(Longueur de la signature)
    memcpy(IMG_Header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

    Palette_64_to_256(Principal_Palette);
    memcpy(IMG_Header.Palette,Principal_Palette,sizeof(T_Palette));
    Palette_256_to_64(Principal_Palette);

    if (write_bytes(Fichier,&IMG_Header,sizeof(T_Header_IMG)))
    {
      Init_ecriture();

      for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
        for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
          Ecrire_octet(Fichier,Lit_pixel_de_sauvegarde(Pos_X,Pos_Y));

      Close_ecriture(Fichier);
      fclose(Fichier);

      if (Erreur_fichier)
        remove(Nom_du_fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(Fichier);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
  }
  else
  {
    fclose(Fichier);
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
  byte Methode;  // Méthode de compression:
                 //   0 = compression en ligne (c)KM
                 //   autres = inconnues pour le moment
  byte Recon1;   // Octet de reconnaissance sur 1 octet  }
  byte Recon2;   // Octet de reconnaissance sur 2 octets }
  word Largeur;  // Largeur de l'image
  word Hauteur;  // Hauteur de l'image
  T_Palette Palette; // Palette RVB 256*3
  word Jump;     // Taille du saut entre le header et l'image:
                 //   On va s'en servir pour rajouter un commentaire
} __attribute__((__packed__)) T_Header_PKM;

// -- Tester si un fichier est au format PKM --------------------------------
void Test_PKM(void)
{
  FILE *Fichier;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_PKM Head;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture du header du fichier
    if (read_bytes(Fichier,&Head,sizeof(T_Header_PKM)))
    {
      // On regarde s'il y a la signature PKM suivie de la méthode 0.
      // La constante "PKM" étant un chaîne, elle se termine toujours par 0.
      // Donc pas la peine de s'emm...er à regarder si la méthode est à 0.
      if ( (!memcmp(&Head,"PKM",4)) && Head.Largeur && Head.Hauteur)
        Erreur_fichier=0;
    }
    fclose(Fichier);
  }
}


// -- Lire un fichier au format PKM -----------------------------------------
void Load_PKM(void)
{
  FILE *Fichier;             // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  T_Header_PKM Head;
  byte  Couleur;
  byte  Octet;
  word  Mot;
  word  Indice;
  dword Compteur_de_pixels;
  dword Compteur_de_donnees_packees;
  dword Taille_image;
  dword Taille_pack;
  long  Taille_du_fichier;
  struct stat Informations_Fichier;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    stat(Nom_du_fichier,&Informations_Fichier);
    Taille_du_fichier=Informations_Fichier.st_size;

    if (read_bytes(Fichier,&Head,sizeof(T_Header_PKM)))
    {
      Principal_Commentaire[0]='\0'; // On efface le commentaire
      if (Head.Jump)
      {
        Indice=0;
        while ( (Indice<Head.Jump) && (!Erreur_fichier) )
        {
          if (read_byte(Fichier,&Octet))
          {
            Indice+=2; // On rajoute le "Field-id" et "le Field-size" pas encore lu
            switch (Octet)
            {
              case 0 : // Commentaire
                if (read_byte(Fichier,&Octet))
                {
                  if (Octet>TAILLE_COMMENTAIRE)
                  {
                    Couleur=Octet;              // On se sert de Couleur comme
                    Octet=TAILLE_COMMENTAIRE;   // variable temporaire
                    Couleur-=TAILLE_COMMENTAIRE;
                  }
                  else
                    Couleur=0;

                  if (read_bytes(Fichier,Principal_Commentaire,Octet))
                  {
                    Indice+=Octet;
                    Principal_Commentaire[Octet]='\0';
                    if (Couleur)
                      if (fseek(Fichier,Couleur,SEEK_CUR))
                        Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              case 1 : // Dimensions de l'écran d'origine
                if (read_byte(Fichier,&Octet))
                {
                  if (Octet==4)
                  {
                    Indice+=4;
                    if ( ! read_word_le(Fichier,&Ecran_original_X)
                      || !read_word_le(Fichier,&Ecran_original_Y) )
                      Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              case 2 : // Couleur de transparence
                if (read_byte(Fichier,&Octet))
                {
                  if (Octet==1)
                  {
                    Indice++;
                    if (! read_byte(Fichier,&Back_color))
                      Erreur_fichier=2;
                  }
                  else
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
                break;

              default:
                if (read_byte(Fichier,&Octet))
                {
                  Indice+=Octet;
                  if (fseek(Fichier,Octet,SEEK_CUR))
                    Erreur_fichier=2;
                }
                else
                  Erreur_fichier=2;
            }
          }
          else
            Erreur_fichier=2;
        }
        if ( (!Erreur_fichier) && (Indice!=Head.Jump) )
          Erreur_fichier=2;
      }

      /*Init_lecture();*/

      if (!Erreur_fichier)
      {
        Initialiser_preview(Head.Largeur,Head.Hauteur,Taille_du_fichier,FORMAT_PKM);
        if (Erreur_fichier==0)
        {
          Principal_Largeur_image=Head.Largeur;
          Principal_Hauteur_image=Head.Hauteur;
          Taille_image=(dword)(Principal_Largeur_image*Principal_Hauteur_image);

          memcpy(Principal_Palette,Head.Palette,sizeof(T_Palette));
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Compteur_de_donnees_packees=0;
          Compteur_de_pixels=0;
          Taille_pack=(Informations_Fichier.st_size)-sizeof(T_Header_PKM)-Head.Jump;

          // Boucle de décompression:
          while ( (Compteur_de_pixels<Taille_image) && (Compteur_de_donnees_packees<Taille_pack) && (!Erreur_fichier) )
          {
            Lire_octet(Fichier, &Octet);

            // Si ce n'est pas un octet de reconnaissance, c'est un pixel brut
            if ( (Octet!=Head.Recon1) && (Octet!=Head.Recon2) )
            {
              Pixel_de_chargement(Compteur_de_pixels % Principal_Largeur_image,
                                  Compteur_de_pixels / Principal_Largeur_image,
                                  Octet);
              Compteur_de_donnees_packees++;
              Compteur_de_pixels++;
            }
            else // Sinon, On regarde si on va décompacter un...
            { // ... nombre de pixels tenant sur un byte
              if (Octet==Head.Recon1)
              {
                Lire_octet(Fichier, &Couleur);
                Lire_octet(Fichier, &Octet);
                for (Indice=0; Indice<Octet; Indice++)
                  Pixel_de_chargement((Compteur_de_pixels+Indice) % Principal_Largeur_image,
                                      (Compteur_de_pixels+Indice) / Principal_Largeur_image,
                                      Couleur);
                Compteur_de_pixels+=Octet;
                Compteur_de_donnees_packees+=3;
              }
              else // ... nombre de pixels tenant sur un word
              {
                Lire_octet(Fichier, &Couleur);
                read_word_be(Fichier, &Mot);
                for (Indice=0; Indice<Mot; Indice++)
                  Pixel_de_chargement((Compteur_de_pixels+Indice) % Principal_Largeur_image,
                                      (Compteur_de_pixels+Indice) / Principal_Largeur_image,
                                      Couleur);
                Compteur_de_pixels+=Mot;
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

    fclose(Fichier);
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
  FILE *Fichier;
  T_Header_PKM Head;
  dword Compteur_de_pixels;
  dword Taille_image;
  word  Compteur_de_repetitions;
  byte  Derniere_couleur;
  byte  Valeur_pixel;
  byte  Taille_commentaire;



  // Construction du header
  memcpy(Head.Ident,"PKM",3);
  Head.Methode=0;
  Trouver_recon(&Head.Recon1,&Head.Recon2);
  Head.Largeur=Principal_Largeur_image;
  Head.Hauteur=Principal_Hauteur_image;
  memcpy(Head.Palette,Principal_Palette,sizeof(T_Palette));

  // Calcul de la taille du Post-Header
  Head.Jump=9; // 6 pour les dimensions de l'ecran + 3 pour la back-color
  Taille_commentaire=strlen(Principal_Commentaire);
  if (Taille_commentaire)
    Head.Jump+=Taille_commentaire+2;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // Ecriture du header
    if (write_bytes(Fichier,&Head,sizeof(T_Header_PKM)))
    {
      Init_ecriture();

      // Ecriture du commentaire
      // (Compteur_de_pixels est utilisé ici comme simple indice de comptage)
      if (Taille_commentaire)
      {
        Ecrire_octet(Fichier,0);
        Ecrire_octet(Fichier,Taille_commentaire);
        for (Compteur_de_pixels=0; Compteur_de_pixels<Taille_commentaire; Compteur_de_pixels++)
          Ecrire_octet(Fichier,Principal_Commentaire[Compteur_de_pixels]);
      }
      // Ecriture des dimensions de l'écran
      Ecrire_octet(Fichier,1);
      Ecrire_octet(Fichier,4);
      Ecrire_octet(Fichier,Largeur_ecran&0xFF);
      Ecrire_octet(Fichier,Largeur_ecran>>8);
      Ecrire_octet(Fichier,Hauteur_ecran&0xFF);
      Ecrire_octet(Fichier,Hauteur_ecran>>8);
      // Ecriture de la back-color
      Ecrire_octet(Fichier,2);
      Ecrire_octet(Fichier,1);
      Ecrire_octet(Fichier,Back_color);

      // Routine de compression PKM de l'image
      Taille_image=(dword)(Principal_Largeur_image*Principal_Hauteur_image);
      Compteur_de_pixels=0;
      Valeur_pixel=Lit_pixel_de_sauvegarde(0,0);

      while ( (Compteur_de_pixels<Taille_image) && (!Erreur_fichier) )
      {
        Compteur_de_pixels++;
        Compteur_de_repetitions=1;
        Derniere_couleur=Valeur_pixel;
        Valeur_pixel=Lit_pixel_de_sauvegarde(Compteur_de_pixels % Principal_Largeur_image,Compteur_de_pixels / Principal_Largeur_image);
        while ( (Valeur_pixel==Derniere_couleur)
             && (Compteur_de_pixels<Taille_image)
             && (Compteur_de_repetitions<65535) )
        {
          Compteur_de_repetitions++;
          Compteur_de_pixels++;
          Valeur_pixel=Lit_pixel_de_sauvegarde(Compteur_de_pixels % Principal_Largeur_image,Compteur_de_pixels / Principal_Largeur_image);
        }

        if ( (Derniere_couleur!=Head.Recon1) && (Derniere_couleur!=Head.Recon2) )
        {
          if (Compteur_de_repetitions==1)
            Ecrire_octet(Fichier,Derniere_couleur);
          else
          if (Compteur_de_repetitions==2)
          {
            Ecrire_octet(Fichier,Derniere_couleur);
            Ecrire_octet(Fichier,Derniere_couleur);
          }
          else
          if ( (Compteur_de_repetitions>2) && (Compteur_de_repetitions<256) )
          { // RECON1/couleur/nombre
            Ecrire_octet(Fichier,Head.Recon1);
            Ecrire_octet(Fichier,Derniere_couleur);
            Ecrire_octet(Fichier,Compteur_de_repetitions&0xFF);
          }
          else
          if (Compteur_de_repetitions>=256)
          { // RECON2/couleur/hi(nombre)/lo(nombre)
            Ecrire_octet(Fichier,Head.Recon2);
            Ecrire_octet(Fichier,Derniere_couleur);
            Ecrire_octet(Fichier,Compteur_de_repetitions>>8);
            Ecrire_octet(Fichier,Compteur_de_repetitions&0xFF);
          }
        }
        else
        {
          if (Compteur_de_repetitions<256)
          {
            Ecrire_octet(Fichier,Head.Recon1);
            Ecrire_octet(Fichier,Derniere_couleur);
            Ecrire_octet(Fichier,Compteur_de_repetitions&0xFF);
          }
          else
          {
            Ecrire_octet(Fichier,Head.Recon2);
            Ecrire_octet(Fichier,Derniere_couleur);
            Ecrire_octet(Fichier,Compteur_de_repetitions>>8);
            Ecrire_octet(Fichier,Compteur_de_repetitions&0xFF);
          }
        }
      }

      Close_ecriture(Fichier);
    }
    else
      Erreur_fichier=1;
    fclose(Fichier);
  }
  else
  {
    Erreur_fichier=1;
    fclose(Fichier);
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

// -- Tester si un fichier est au format LBM --------------------------------

  FILE *LBM_Fichier;

void Test_LBM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  char  Format[4];
  char  Section[4];
  dword Dummy;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((LBM_Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (! read_bytes(LBM_Fichier,Section,4))
      Erreur_fichier=1;
    else
    if (memcmp(Section,"FORM",4))
      Erreur_fichier=1;
    else
    {
      read_dword_be(LBM_Fichier, &Dummy);
                   //   On aurait pu vérifier que ce long est égal à la taille
                   // du fichier - 8, mais ça aurait interdit de charger des
                   // fichiers tronqués (et déjà que c'est chiant de perdre
                   // une partie du fichier il faut quand même pouvoir en
                   // garder un peu... Sinon, moi je pleure :'( !!! )
      if (! read_bytes(LBM_Fichier,Format,4))
        Erreur_fichier=1;
      else
      if ( (memcmp(Format,"ILBM",4)) && (memcmp(Format,"PBM ",4)) )
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
    short I,J,Temp;
    byte  Couleur;

    if (Image_HAM==6)
    {
      for (I=1; I<=14; I++)
      {
        // On recopie a palette de base
        memcpy(Principal_Palette+(I<<4),Principal_Palette,48);
        // On modifie les teintes de cette palette
        for (J=0; J<16; J++)
        {
          Couleur=(I<<4)+J;
          if (I<=7)
          {
            if (I&1)
            {
              Temp=Principal_Palette[J].R+16;
              Principal_Palette[Couleur].R=(Temp<63)?Temp:63;
            }
            if (I&2)
            {
              Temp=Principal_Palette[J].V+16;
              Principal_Palette[Couleur].V=(Temp<63)?Temp:63;
            }
            if (I&4)
            {
              Temp=Principal_Palette[J].B+16;
              Principal_Palette[Couleur].B=(Temp<63)?Temp:63;
            }
          }
          else
          {
            if ((I-7)&1)
            {
              Temp=Principal_Palette[J].R-16;
              Principal_Palette[Couleur].R=(Temp>=0)?Temp:0;
            }
            if ((I-7)&2)
            {
              Temp=Principal_Palette[J].V-16;
              Principal_Palette[Couleur].V=(Temp>=0)?Temp:0;
            }
            if ((I-7)&4)
            {
              Temp=Principal_Palette[J].B-16;
              Principal_Palette[Couleur].B=(Temp>=0)?Temp:0;
            }
          }
        }
      }
      // Ici, il reste les 16 dernières couleurs à modifier
      for (I=240,J=0; J<16; I++,J++)
      {
        Temp=Principal_Palette[J].R+8;
        Principal_Palette[I].R=(Temp<63)?Temp:63;
        Temp=Principal_Palette[J].V+8;
        Principal_Palette[I].V=(Temp<63)?Temp:63;
        Temp=Principal_Palette[J].B+8;
        Principal_Palette[I].B=(Temp<63)?Temp:63;
      }
    }
    else if (Image_HAM==8)
    {
      for (I=1; I<=3; I++)
      {
        // On recopie la palette de base
        memcpy(Principal_Palette+(I<<6),Principal_Palette,192);
        // On modifie les teintes de cette palette
        for (J=0; J<64; J++)
        {
          Couleur=(I<<6)+J;
          switch (I)
          {
            case 1 :
              Temp=Principal_Palette[J].R+16;
              Principal_Palette[Couleur].R=(Temp<63)?Temp:63;
              break;
            case 2 :
              Temp=Principal_Palette[J].V+16;
              Principal_Palette[Couleur].V=(Temp<63)?Temp:63;
              break;
            default:
              Temp=Principal_Palette[J].B+16;
              Principal_Palette[Couleur].B=(Temp<63)?Temp:63;
          }
        }
      }
    }
    else // Image 64 couleurs sauvée en 32.
    {
      for (I=0; I<32; I++)
      {
        J=I+32;
        Principal_Palette[J].R=Principal_Palette[I].R>>1;
        Principal_Palette[J].V=Principal_Palette[I].V>>1;
        Principal_Palette[J].B=Principal_Palette[I].B>>1;
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

  // ----------------------- Afficher une ligne ILBM ------------------------
  void Draw_ILBM_line(short Pos_Y, short Vraie_taille_ligne)
  {
    byte  Couleur;
    byte  Rouge,Vert,Bleu;
    byte  Temp;
    short Pos_X;

    if (Image_HAM<=1)                                               // ILBM
    {
      for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
      {
        Pixel_de_chargement(Pos_X,Pos_Y,Couleur_ILBM_line(Pos_X,Vraie_taille_ligne, HBPm1));
      }
    }
    else
    {
      Couleur=0;
      Rouge=Principal_Palette[0].R;
      Vert =Principal_Palette[0].V;
      Bleu =Principal_Palette[0].B;
      if (Image_HAM==6)
      for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)         // HAM6
      {
        Temp=Couleur_ILBM_line(Pos_X,Vraie_taille_ligne, HBPm1);
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
            Vert =Principal_Palette[Couleur].V;
            Bleu =Principal_Palette[Couleur].B;
        }
        Pixel_de_chargement(Pos_X,Pos_Y,Couleur);
      }
      else
      for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)         // HAM8
      {
        Temp=Couleur_ILBM_line(Pos_X,Vraie_taille_ligne, HBPm1);
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
            Vert =Principal_Palette[Couleur].V;
            Bleu =Principal_Palette[Couleur].B;
        }
        Pixel_de_chargement(Pos_X,Pos_Y,Couleur);
      }
    }
  }


void Load_LBM(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  T_Header_LBM Header;
  char  Format[4];
  char  Section[4];
  byte  Octet;
  short B256;
  dword Nb_couleurs;
  dword Taille_image;
  short Pos_X;
  short Pos_Y;
  short Compteur;
  short Taille_ligne;       // Taille d'une ligne en octets
  short Vraie_taille_ligne; // Taille d'une ligne en pixels
  byte  Couleur;
  long  Taille_du_fichier;
  struct stat Informations_Fichier;
  dword Dummy;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((LBM_Fichier=fopen(Nom_du_fichier, "rb")))
  {
      stat(Nom_du_fichier,&Informations_Fichier);
    Taille_du_fichier=Informations_Fichier.st_size;

    // On avance dans le fichier (pas besoin de tester ce qui l'a déjà été)
    read_bytes(LBM_Fichier,Section,4);
    read_dword_be(LBM_Fichier,&Dummy);
    read_bytes(LBM_Fichier,Format,4);
    if (!Wait_for((byte *)"BMHD"))
      Erreur_fichier=1;
    read_dword_be(LBM_Fichier,&Dummy);

    // Maintenant on lit le header pour pouvoir commencer le chargement de l'image
    if ( (read_word_be(LBM_Fichier,&Header.Width))
      && (read_word_be(LBM_Fichier,&Header.Height))
      && (read_word_be(LBM_Fichier,&Header.Xorg))
      && (read_word_be(LBM_Fichier,&Header.Yorg))
      && (read_byte(LBM_Fichier,&Header.BitPlanes))
      && (read_byte(LBM_Fichier,&Header.Mask))
      && (read_byte(LBM_Fichier,&Header.Compression))
      && (read_byte(LBM_Fichier,&Header.Pad1))
      && (read_word_be(LBM_Fichier,&Header.Transp_col))
      && (read_byte(LBM_Fichier,&Header.Xaspect))
      && (read_byte(LBM_Fichier,&Header.Yaspect))
      && (read_word_be(LBM_Fichier,&Header.Xscreen))
      && (read_word_be(LBM_Fichier,&Header.Yscreen))
      && Header.Width && Header.Height)
    {
      if ( (Header.BitPlanes) && (Wait_for((byte *)"CMAP")) )
      {
        read_dword_be(LBM_Fichier,&Nb_couleurs);
        Nb_couleurs/=3;

        if (((int)1<<Header.BitPlanes)!=Nb_couleurs)
        {
          if ((Nb_couleurs==32) && (Header.BitPlanes==6))
          {              // Ce n'est pas une image HAM mais une image 64 coul.
            Image_HAM=1; // Sauvée en 32 coul. => il faut copier les 32 coul.
          }              // sur les 32 suivantes et assombrir ces dernières.
          else
          {
            if ((Header.BitPlanes==6) || (Header.BitPlanes==8))
              Image_HAM=Header.BitPlanes;
            else
              // Erreur_fichier=1; /* C'est censé être incorrect mais j'ai */
              Image_HAM=0;         /* trouvé un fichier comme ça, alors... */
          }
        }
        else
          Image_HAM=0;

        if ( (!Erreur_fichier) && (Nb_couleurs>=2) && (Nb_couleurs<=256) )
        {
          HBPm1=Header.BitPlanes-1;
          if (Header.Mask==1)
            Header.BitPlanes++;

          // Deluxe paint le fait... alors on le fait...
          Back_color=Header.Transp_col;

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
            Set_palette(Principal_Palette);
            Remapper_fileselect();

            // On lit l'octet de padding du CMAP si la taille est impaire
            if (Nb_couleurs&1)
              if (read_byte(LBM_Fichier,&Octet))
                Erreur_fichier=2;

            if ( (Wait_for((byte *)"BODY")) && (!Erreur_fichier) )
            {
              read_dword_be(LBM_Fichier,&Taille_image);
              //swab((char *)&Header.Width ,(char *)&Principal_Largeur_image,2);
              //swab((char *)&Header.Height,(char *)&Principal_Hauteur_image,2);
              Principal_Largeur_image = Header.Width;
              Principal_Hauteur_image = Header.Height;

              //swab((char *)&Header.Xscreen,(char *)&Ecran_original_X,2);
              //swab((char *)&Header.Yscreen,(char *)&Ecran_original_Y,2);
              Ecran_original_X = Header.Xscreen;
              Ecran_original_Y = Header.Yscreen;

              Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_LBM);
              if (Erreur_fichier==0)
              {
                if (!memcmp(Format,"ILBM",4))    // "ILBM": InterLeaved BitMap
                {
                  // Calcul de la taille d'une ligne ILBM (pour les images ayant des dimensions exotiques)
                  if (Principal_Largeur_image & 15)
                  {
                    Vraie_taille_ligne=( (Principal_Largeur_image+16) >> 4 ) << 4;
                    Taille_ligne=( (Principal_Largeur_image+16) >> 4 )*(Header.BitPlanes<<1);
                  }
                  else
                  {
                    Vraie_taille_ligne=Principal_Largeur_image;
                    Taille_ligne=(Principal_Largeur_image>>3)*Header.BitPlanes;
                  }

                  if (!Header.Compression)
                  {                                           // non compressé
                    LBM_Buffer=(byte *)malloc(Taille_ligne);
                    for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
                    {
                      if (read_bytes(LBM_Fichier,LBM_Buffer,Taille_ligne))
                        Draw_ILBM_line(Pos_Y,Vraie_taille_ligne);
                      else
                        Erreur_fichier=2;
                    }
                    free(LBM_Buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/

                    LBM_Buffer=(byte *)malloc(Taille_ligne);

                    for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
                    {
                      for (Pos_X=0; ((Pos_X<Taille_ligne) && (!Erreur_fichier)); )
                      {
                        Lire_octet(LBM_Fichier, &Octet);
                        // Si Octet > 127 alors il faut répéter 256-'Octet' fois la couleur de l'octet suivant
                        // Si Octet <= 127 alors il faut afficher directement les 'Octet' octets suivants
                        if (Octet>127)
                        {
                          Lire_octet(LBM_Fichier, &Couleur);
                          B256=(short)(256-Octet);
                          for (Compteur=0; Compteur<=B256; Compteur++)
                            if (Pos_X<Taille_ligne)
                              LBM_Buffer[Pos_X++]=Couleur;
                            else
                              Erreur_fichier=2;
                        }
                        else
                          for (Compteur=0; Compteur<=(short)(Octet); Compteur++)
                            if (Pos_X<Taille_ligne)
                              Lire_octet(LBM_Fichier, &(LBM_Buffer[Pos_X++]));
                            else
                              Erreur_fichier=2;
                      }
                      if (!Erreur_fichier)
                        Draw_ILBM_line(Pos_Y,Vraie_taille_ligne);
                    }

                    free(LBM_Buffer);
                    /*Close_lecture();*/
                  }
                }
                else                               // "PBM ": Planar(?) BitMap
                {
                  Vraie_taille_ligne=Principal_Largeur_image+(Principal_Largeur_image&1);

                  if (!Header.Compression)
                  {                                           // non compressé
                    LBM_Buffer=(byte *)malloc(Vraie_taille_ligne);
                    for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
                    {
                      if (read_bytes(LBM_Fichier,LBM_Buffer,Vraie_taille_ligne))
                        for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
                          Pixel_de_chargement(Pos_X,Pos_Y,LBM_Buffer[Pos_X]);
                      else
                        Erreur_fichier=2;
                    }
                    free(LBM_Buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/
                    for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
                    {
                      for (Pos_X=0; ((Pos_X<Vraie_taille_ligne) && (!Erreur_fichier)); )
                      {
                        Lire_octet(LBM_Fichier, &Octet);
                        if (Octet>127)
                        {
                          Lire_octet(LBM_Fichier, &Couleur);
                          B256=256-Octet;
                          for (Compteur=0; Compteur<=B256; Compteur++)
                            Pixel_de_chargement(Pos_X++,Pos_Y,Couleur);
                        }
                        else
                          for (Compteur=0; Compteur<=Octet; Compteur++)
                          {
                            byte Lu=0;
                            Lire_octet(LBM_Fichier, &Lu);
                            Pixel_de_chargement(Pos_X++,Pos_Y,Lu);
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
            //   On restore l'ancienne palette en cas d'erreur...
            Palette_256_to_64(Principal_Palette);
            // ... ce qui permet de ne renvoyer qu'une erreur 1 (pas de modif)
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
  T_Header_LBM Header;
  word Pos_X;
  word Pos_Y;
  byte Octet;
  word Vraie_largeur;
  struct stat Informations_Fichier;


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

    //swab((byte *)&Vraie_largeur,(byte *)&Header.Width,2);
    Header.Width=Principal_Largeur_image;
    Header.Height=Principal_Hauteur_image;
    Header.Xorg=0;
    Header.Yorg=0;
    Header.BitPlanes=8;
    Header.Mask=0;
    Header.Compression=1;
    Header.Pad1=0;
    Header.Transp_col=Back_color;
    Header.Xaspect=1;
    Header.Yaspect=1;
    Header.Xscreen = Largeur_ecran;
    Header.Yscreen = Hauteur_ecran;

    write_word_be(LBM_Fichier,Header.Width);
    write_word_be(LBM_Fichier,Header.Height);
    write_word_be(LBM_Fichier,Header.Xorg);
    write_word_be(LBM_Fichier,Header.Yorg);
    write_bytes(LBM_Fichier,&Header.BitPlanes,1);
    write_bytes(LBM_Fichier,&Header.Mask,1);
    write_bytes(LBM_Fichier,&Header.Compression,1);
    write_bytes(LBM_Fichier,&Header.Pad1,1);
    write_word_be(LBM_Fichier,Header.Transp_col);
    write_bytes(LBM_Fichier,&Header.Xaspect,1);
    write_bytes(LBM_Fichier,&Header.Yaspect,1);
    write_word_be(LBM_Fichier,Header.Xscreen);
    write_word_be(LBM_Fichier,Header.Yscreen);

    write_bytes(LBM_Fichier,"CMAP",4);
    write_dword_be(LBM_Fichier,sizeof(T_Palette));

    Palette_64_to_256(Principal_Palette);
    write_bytes(LBM_Fichier,Principal_Palette,sizeof(T_Palette));
    Palette_256_to_64(Principal_Palette);

    write_bytes(LBM_Fichier,"BODY",4);
    write_dword_be(LBM_Fichier,0); // On mettra la taille à jour à la fin

    Init_ecriture();

    LBM_Taille_de_file=0;

    for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
    {
      for (Pos_X=0; ((Pos_X<Vraie_largeur) && (!Erreur_fichier)); Pos_X++)
        Nouvelle_couleur(Lit_pixel_de_sauvegarde(Pos_X,Pos_Y));

      if (!Erreur_fichier)
        Transferer_couleurs();
    }

    Close_ecriture(LBM_Fichier);
    fclose(LBM_Fichier);

    if (!Erreur_fichier)
    {
      stat(Nom_du_fichier,&Informations_Fichier);

      LBM_Fichier=fopen(Nom_du_fichier,"rb+");
      fseek(LBM_Fichier,820,SEEK_SET);
      write_dword_be(LBM_Fichier,Informations_Fichier.st_size-824);

      if (!Erreur_fichier)
      {
        fseek(LBM_Fichier,4,SEEK_SET);

        //   Si la taille de la section de l'image (taille fichier-8) est
        // impaire, on rajoute un 0 (Padding) à la fin.
        if ((Informations_Fichier.st_size) & 1)
        {
          write_dword_be(LBM_Fichier,Informations_Fichier.st_size-7);
          fseek(LBM_Fichier,0,SEEK_END);
          Octet=0;
          if (! write_bytes(LBM_Fichier,&Octet,1))
            Erreur_fichier=1;
        }
        else
          write_dword_be(LBM_Fichier,Informations_Fichier.st_size-8);

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
    word Signature;   // ='BM' = 0x4D42
    uint32_t Taille_1;    // =Taille du fichier
    word Reserv_1;    // =0
    word Reserv_2;    // =0
    uint32_t Decalage;    // Nb octets avant les données bitmap

    uint32_t Taille_2;    // =40 
    uint32_t Largeur;
    uint32_t Hauteur;
    word Plans;       // =1
    word Nb_bits;     // =1,4,8 ou 24
    uint32_t Compression;
    uint32_t Taille_3;
    uint32_t XPM;
    uint32_t YPM;
    uint32_t Nb_Clr;
    uint32_t Clr_Imprt;
} __attribute__((__packed__)) T_BMP_Header;

// -- Tester si un fichier est au format BMP --------------------------------
void Test_BMP(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_BMP_Header Header;

  Erreur_fichier=1;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (read_bytes(Fichier,&(Header.Signature),2) // "BM"
     && read_dword_le(Fichier,&(Header.Taille_1))
     && read_word_le(Fichier,&(Header.Reserv_1))
     && read_word_le(Fichier,&(Header.Reserv_2))
     && read_dword_le(Fichier,&(Header.Decalage))
     && read_dword_le(Fichier,&(Header.Taille_2))
     && read_dword_le(Fichier,&(Header.Largeur))
     && read_dword_le(Fichier,&(Header.Hauteur))
     && read_word_le(Fichier,&(Header.Plans))
     && read_word_le(Fichier,&(Header.Nb_bits))
     && read_dword_le(Fichier,&(Header.Compression))
     && read_dword_le(Fichier,&(Header.Taille_3))
     && read_dword_le(Fichier,&(Header.XPM))
     && read_dword_le(Fichier,&(Header.YPM))
     && read_dword_le(Fichier,&(Header.Nb_Clr))
     && read_dword_le(Fichier,&(Header.Clr_Imprt))
        )
     {
      if ( (Header.Signature==0x4D42) && (Header.Taille_2==40)
        && Header.Largeur && Header.Hauteur )
        Erreur_fichier=0;
     }
    fclose(Fichier);
  }
}


// -- Charger un fichier au format BMP --------------------------------------
void Load_BMP(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_BMP_Header Header;
  byte * Buffer;
  word  Indice;
  byte  Palette_locale[256][4]; // R,V,B,0
  word  Nb_Couleurs =  0;
  short Pos_X;
  short Pos_Y;
  word  Taille_ligne;
  byte  A,B,C=0;
  long  Taille_du_fichier;
  struct stat Informations_Fichier;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
      stat(Nom_du_fichier,&Informations_Fichier);
    Taille_du_fichier=Informations_Fichier.st_size;

    if (read_word_le(Fichier,&(Header.Signature))
     && read_dword_le(Fichier,&(Header.Taille_1))
     && read_word_le(Fichier,&(Header.Reserv_1))
     && read_word_le(Fichier,&(Header.Reserv_2))
     && read_dword_le(Fichier,&(Header.Decalage))
     && read_dword_le(Fichier,&(Header.Taille_2))
     && read_dword_le(Fichier,&(Header.Largeur))
     && read_dword_le(Fichier,&(Header.Hauteur))
     && read_word_le(Fichier,&(Header.Plans))
     && read_word_le(Fichier,&(Header.Nb_bits))
     && read_dword_le(Fichier,&(Header.Compression))
     && read_dword_le(Fichier,&(Header.Taille_3))
     && read_dword_le(Fichier,&(Header.XPM))
     && read_dword_le(Fichier,&(Header.YPM))
     && read_dword_le(Fichier,&(Header.Nb_Clr))
     && read_dword_le(Fichier,&(Header.Clr_Imprt))
    )
    {
      switch (Header.Nb_bits)
      {
        case 1 :
        case 4 :
        case 8 :
          if (Header.Nb_Clr)
            Nb_Couleurs=Header.Nb_Clr;
          else
            Nb_Couleurs=1<<Header.Nb_bits;
          break;
        default:
          Erreur_fichier=1;
      }

      if (!Erreur_fichier)
      {
        Initialiser_preview(Header.Largeur,Header.Hauteur,Taille_du_fichier,FORMAT_BMP);
        if (Erreur_fichier==0)
        {
          if (read_bytes(Fichier,Palette_locale,Nb_Couleurs<<2))
          {
            //   On commence par passer la palette en 256 comme ça, si la nouvelle
            // palette a moins de 256 coul, la précédente ne souffrira pas d'un
            // assombrissement préjudiciable.
            if (Config.Clear_palette)
              memset(Principal_Palette,0,sizeof(T_Palette));
            else
              Palette_64_to_256(Principal_Palette);
            //   On peut maintenant transférer la nouvelle palette
            for (Indice=0; Indice<Nb_Couleurs; Indice++)
            {
              Principal_Palette[Indice].R=Palette_locale[Indice][2];
              Principal_Palette[Indice].V=Palette_locale[Indice][1];
              Principal_Palette[Indice].B=Palette_locale[Indice][0];
            }
            Palette_256_to_64(Principal_Palette);
            Set_palette(Principal_Palette);
            Remapper_fileselect();

            Principal_Largeur_image=Header.Largeur;
            Principal_Hauteur_image=Header.Hauteur;

            switch (Header.Compression)
            {
              case 0 : // Pas de compression
                Taille_ligne=Principal_Largeur_image;
                Pos_X=(32/Header.Nb_bits); // Pos_X sert de variable temporaire
                if (Taille_ligne % Pos_X)
                  Taille_ligne=((Taille_ligne/Pos_X)*Pos_X)+Pos_X;
                Taille_ligne=(Taille_ligne*Header.Nb_bits)>>3;

                Buffer=(byte *)malloc(Taille_ligne);
                for (Pos_Y=Principal_Hauteur_image-1; ((Pos_Y>=0) && (!Erreur_fichier)); Pos_Y--)
                {
                  if (read_bytes(Fichier,Buffer,Taille_ligne))
                    for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
                      switch (Header.Nb_bits)
                      {
                        case 8 :
                          Pixel_de_chargement(Pos_X,Pos_Y,Buffer[Pos_X]);
                          break;
                        case 4 :
                          if (Pos_X & 1)
                            Pixel_de_chargement(Pos_X,Pos_Y,Buffer[Pos_X>>1] & 0xF);
                          else
                            Pixel_de_chargement(Pos_X,Pos_Y,Buffer[Pos_X>>1] >> 4 );
                          break;
                        case 1 :
                          if ( Buffer[Pos_X>>3] & (0x80>>(Pos_X&7)) )
                            Pixel_de_chargement(Pos_X,Pos_Y,1);
                          else
                            Pixel_de_chargement(Pos_X,Pos_Y,0);
                      }
                  else
                    Erreur_fichier=2;
                }
                free(Buffer);
                break;

              case 1 : // Compression RLE 8 bits
                Pos_X=0;
                Pos_Y=Principal_Hauteur_image-1;

                /*Init_lecture();*/
                Lire_octet(Fichier, &A);
                Lire_octet(Fichier, &B);
                while ( (!Erreur_fichier) && ((A)||(B!=1)) )
                {
                  if (A) // Encoded mode
                    for (Indice=1; Indice<=A; Indice++)
                      Pixel_de_chargement(Pos_X++,Pos_Y,B);
                  else   // Absolute mode
                    switch (B)
                    {
                      case 0 : // End of line
                        Pos_X=0;
                        Pos_Y--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                        Lire_octet(Fichier, &A);
                        Lire_octet(Fichier, &B);
                        Pos_X+=A;
                        Pos_Y-=B;
                        break;
                      default: // Nouvelle série
                        while (B)
                        {
                          Lire_octet(Fichier, &A);
                          Lire_octet(Fichier, &C);
                          Pixel_de_chargement(Pos_X++,Pos_Y,A);
                          if (--B)
                          {
                            Pixel_de_chargement(Pos_X++,Pos_Y,C);
                            B--;
                          }
                        }
                    }
                  Lire_octet(Fichier, &A);
                  Lire_octet(Fichier, &B);
                }
                /*Close_lecture();*/
                break;

              case 2 : // Compression RLE 4 bits
                Pos_X=0;
                Pos_Y=Principal_Hauteur_image-1;

                /*Init_lecture();*/
                Lire_octet(Fichier, &A);
                Lire_octet(Fichier, &B);
                while ( (!Erreur_fichier) && ((A)||(B!=1)) )
                {
                  if (A) // Encoded mode (A fois les 1/2 pixels de B)
                    for (Indice=1; Indice<=A; Indice++)
                    {
                      if (Indice & 1)
                        Pixel_de_chargement(Pos_X,Pos_Y,B>>4);
                      else
                        Pixel_de_chargement(Pos_X,Pos_Y,B&0xF);
                      Pos_X++;
                    }
                  else   // Absolute mode
                    switch (B)
                    {
                      case 0 : //End of line
                        Pos_X=0;
                        Pos_Y--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                        Lire_octet(Fichier, &A);
                        Lire_octet(Fichier, &B);
                        Pos_X+=A;
                        Pos_Y-=B;
                        break;
                      default: // Nouvelle série (B 1/2 pixels bruts)
                        for (Indice=1; ((Indice<=B) && (!Erreur_fichier)); Indice++,Pos_X++)
                        {
                          if (Indice&1)
                          {
                            Lire_octet(Fichier, &C);
                            Pixel_de_chargement(Pos_X,Pos_Y,C>>4);
                          }
                          else
                            Pixel_de_chargement(Pos_X,Pos_Y,C&0xF);
                        }
                        //   On lit l'octet rendant le nombre d'octets pair, si
                        // nécessaire. Encore un truc de crétin "made in MS".
                        if ( ((B&3)==1) || ((B&3)==2) )
                        {
                          byte Dummy;
                          Lire_octet(Fichier, &Dummy);
                        }
                    }
                  Lire_octet(Fichier, &A);
                  Lire_octet(Fichier, &B);
                }
                /*Close_lecture();*/
            }
            fclose(Fichier);
          }
          else
          {
            fclose(Fichier);
            Erreur_fichier=1;
          }
        }
      }
      else
      {
        // Image 24 bits!!!
        Erreur_fichier=0;

        Principal_Largeur_image=Header.Largeur;
        Principal_Hauteur_image=Header.Hauteur;
        Initialiser_preview(Header.Largeur,Header.Hauteur,Taille_du_fichier,FORMAT_BMP | FORMAT_24B);

        if (Erreur_fichier==0)
        {
          Taille_ligne=Principal_Largeur_image*3;
          Pos_X=(Taille_ligne % 4); // Pos_X sert de variable temporaire
          if (Pos_X>0)
            Taille_ligne+=(4-Pos_X);

          Buffer=(byte *)malloc(Taille_ligne);
          for (Pos_Y=Principal_Hauteur_image-1; ((Pos_Y>=0) && (!Erreur_fichier)); Pos_Y--)
          {
            if (read_bytes(Fichier,Buffer,Taille_ligne))
              for (Pos_X=0,Indice=0; Pos_X<Principal_Largeur_image; Pos_X++,Indice+=3)
                Pixel_Chargement_24b(Pos_X,Pos_Y,Buffer[Indice+2],Buffer[Indice+1],Buffer[Indice+0]);
            else
              Erreur_fichier=2;
          }
          free(Buffer);
          fclose(Fichier);
        }
      }
    }
    else
    {
      fclose(Fichier);
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
  FILE *Fichier;
  T_BMP_Header Header;
  short Pos_X;
  short Pos_Y;
  long Taille_ligne;
  word Indice;
  byte Palette_locale[256][4]; // R,V,B,0


  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    if (Principal_Largeur_image & 7)
      Taille_ligne=((Principal_Largeur_image >> 3)+1) << 3;
    else
      Taille_ligne=Principal_Largeur_image;

    Header.Signature  =0x4D42;
    Header.Taille_1   =(Taille_ligne*Principal_Hauteur_image)+1078;
    Header.Reserv_1   =0;
    Header.Reserv_2   =0;
    Header.Decalage   =1078;
    Header.Taille_2   =40;
    Header.Largeur    =Taille_ligne;
    Header.Hauteur    =Principal_Hauteur_image;
    Header.Plans      =1;
    Header.Nb_bits    =8;
    Header.Compression=0;
    Header.Taille_3   =0;
    Header.XPM        =0;
    Header.YPM        =0;
    Header.Nb_Clr     =0;
    Header.Clr_Imprt  =0;

    if (write_bytes(Fichier,&Header,sizeof(T_BMP_Header)))
    {
      //   Chez Bill, ils ont dit: "On va mettre les couleur dans l'ordre
      // inverse, et pour faire chier, on va les mettre sur une échelle de
      // 0 à 255 parce que le standard VGA c'est de 0 à 63 (logique!). Et
      // puis comme c'est pas assez débile, on va aussi y rajouter un octet
      // toujours à 0 pour forcer les gens à s'acheter des gros disques
      // durs... Comme ça, ça fera passer la pillule lorsqu'on sortira
      // Windows 95." ...
      Palette_64_to_256(Principal_Palette);
      for (Indice=0; Indice<256; Indice++)
      {
        Palette_locale[Indice][0]=Principal_Palette[Indice].B;
        Palette_locale[Indice][1]=Principal_Palette[Indice].V;
        Palette_locale[Indice][2]=Principal_Palette[Indice].R;
        Palette_locale[Indice][3]=0;
      }
      Palette_256_to_64(Principal_Palette);

      if (write_bytes(Fichier,Palette_locale,1024))
      {
        Init_ecriture();

        // ... Et Bill, il a dit: "OK les gars! Mais seulement si vous rangez
        // les pixels dans l'ordre inverse, mais que sur les Y quand-même
        // parce que faut pas pousser."
        for (Pos_Y=Principal_Hauteur_image-1; ((Pos_Y>=0) && (!Erreur_fichier)); Pos_Y--)
          for (Pos_X=0; Pos_X<Taille_ligne; Pos_X++)
            Ecrire_octet(Fichier,Lit_pixel_de_sauvegarde(Pos_X,Pos_Y));

        Close_ecriture(Fichier);
        fclose(Fichier);

        if (Erreur_fichier)
          remove(Nom_du_fichier);
      }
      else
      {
        fclose(Fichier);
        remove(Nom_du_fichier);
        Erreur_fichier=1;
      }

    }
    else
    {
      fclose(Fichier);
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
  word Largeur; // Largeur de l'écran virtuel
  word Hauteur; // Hauteur de l'écran virtuel
  byte Resol;   // Informations sur la résolution (et autres)
  byte Backcol; // Couleur de fond
  byte Aspect;  // Informations sur l'aspect ratio (et autres)
} __attribute__((__packed__)) T_LSDB; // Logical Screen Descriptor Block

typedef struct
{
  word Pos_X;         // Abscisse où devrait être affichée l'image
  word Pos_Y;         // Ordonnée où devrait être affichée l'image
  word Largeur_image; // Largeur de l'image
  word Hauteur_image; // Hauteur de l'image
  byte Indicateur;    // Informations diverses sur l'image
  byte Nb_bits_pixel; // Nb de bits par pixel
} __attribute__((__packed__)) T_IDB; // Image Descriptor Block

// -- Tester si un fichier est au format GIF --------------------------------

void Test_GIF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  char Signature[6];
  FILE *Fichier;


  Erreur_fichier=1;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (
        (read_bytes(Fichier,Signature,6)) &&
        ((!memcmp(Signature,"GIF87a",6))||(!memcmp(Signature,"GIF89a",6)))
       )
      Erreur_fichier=0;

    fclose(Fichier);
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
  char Signature[6];

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
  //word Indice_de_lecture; // Indice de lecture des données (divers)
  byte Block_indicateur;  // Code indicateur du type de bloc en cours
  word Nb_bits_initial;   // Nb de bits au début du traitement LZW
  word Cas_special=0;       // Mémoire pour le cas spécial
  word Code_ancien=0;       // Code précédent
  word Read_byte;         // Sauvegarde du code en cours de lecture
  word Valeur_Clr;        // Valeur <=> Clear tables
  word Valeur_Eof;        // Valeur <=> Fin d'image
  long Taille_du_fichier;
  struct stat Informations_Fichier;


  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  GIF_Pos_X=0;
  GIF_Pos_Y=0;
  GIF_Last_byte=0;
  GIF_Rest_bits=0;
  GIF_Rest_byte=0;

  Nom_fichier_complet(Nom_du_fichier,0);

  if ((GIF_Fichier=fopen(Nom_du_fichier, "rb")))
  {
      stat(Nom_du_fichier,&Informations_Fichier);
    Taille_du_fichier=Informations_Fichier.st_size;

    if ( (read_bytes(GIF_Fichier,Signature,6)) &&
         ( (memcmp(Signature,"GIF87a",6)==0) ||
           (memcmp(Signature,"GIF89a",6)==0) ) )
    {
      // Allocation de mémoire pour les tables & piles de traitement:
      Alphabet_Pile   =(word *)malloc(4096*sizeof(word));
      Alphabet_Prefixe=(word *)malloc(4096*sizeof(word));
      Alphabet_Suffixe=(word *)malloc(4096*sizeof(word));

      if (read_word_le(GIF_Fichier,&(LSDB.Largeur))
      && read_word_le(GIF_Fichier,&(LSDB.Hauteur))
      && read_byte(GIF_Fichier,&(LSDB.Resol))
      && read_byte(GIF_Fichier,&(LSDB.Backcol))
      && read_byte(GIF_Fichier,&(LSDB.Aspect))
        )
      {
        // Lecture du Logical Screen Descriptor Block réussie:

        Ecran_original_X=endian_magic16(LSDB.Largeur);
        Ecran_original_Y=endian_magic16(LSDB.Hauteur);

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

          //   On commence par passer la palette en 256 comme ça, si la
          // nouvelle palette a moins de 256 coul, la précédente ne souffrira
          // pas d'un assombrissement préjudiciable.
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          else
            Palette_64_to_256(Principal_Palette);

          //   On peut maintenant charger la nouvelle palette:
          if (!(LSDB.Aspect & 0x80))
            // Palette dans l'ordre:
            for(Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
            {
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].V);
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
            }
          else
          {
            // Palette triée par composantes:
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].V);
            for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
              read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
          }

          Palette_256_to_64(Principal_Palette);
          Set_palette(Principal_Palette);
        }

        // On s'apprête à sauter tous les blocks d'extension:

        // On lit un indicateur de block
        read_byte(GIF_Fichier,&Block_indicateur);
        // Si l'indicateur de block annonce un block d'extension:
        while (Block_indicateur==0x21)
        {
          // Lecture du code de fonction:
          read_byte(GIF_Fichier,&Block_indicateur);

          // On exploitera peut-être un jour ce code indicateur pour stocker
          // des remarques dans le fichier. En attendant d'en connaître plus
          // on se contente de sauter tous les blocs d'extension:

          // Lecture de la taille du bloc:
          read_byte(GIF_Fichier,&Taille_de_lecture);
          while (Taille_de_lecture!=0)
          {
            // On saute le bloc:
            fseek(GIF_Fichier,Taille_de_lecture,SEEK_CUR);
            // Lecture de la taille du bloc suivant:
            read_byte(GIF_Fichier,&Taille_de_lecture);
          }

          // Lecture du code de fonction suivant:
          read_byte(GIF_Fichier,&Block_indicateur);
        }

        if (Block_indicateur==0x2C)
        {
          // Présence d'un Image Separator Header

          // lecture de 10 derniers octets
          if ( read_word_le(GIF_Fichier,&(IDB.Pos_X))
            && read_word_le(GIF_Fichier,&(IDB.Pos_Y))
            && read_word_le(GIF_Fichier,&(IDB.Largeur_image))
            && read_word_le(GIF_Fichier,&(IDB.Hauteur_image))
            && read_byte(GIF_Fichier,&(IDB.Indicateur))
            && read_byte(GIF_Fichier,&(IDB.Nb_bits_pixel))
            && IDB.Largeur_image && IDB.Hauteur_image)
          {
            Principal_Largeur_image=endian_magic16(IDB.Largeur_image);
            Principal_Hauteur_image=endian_magic16(IDB.Hauteur_image);

            Initialiser_preview(endian_magic16(IDB.Largeur_image),endian_magic16(IDB.Hauteur_image),Taille_du_fichier,FORMAT_GIF);

            // Palette locale dispo = (IDB.Indicateur and $80)
            // Image entrelacée     = (IDB.Indicateur and $40)
            // Ordre de classement  = (IDB.Indicateur and $20)
            // Nombre de bits/pixel = (IDB.Indicateur and $07)+1 (si palette locale dispo)

            if (IDB.Indicateur & 0x80)
            {
              // Palette locale dispo

              Nb_couleurs=(1 << ((IDB.Indicateur & 0x07)+1));
              Nb_bits_initial=(IDB.Indicateur & 0x07)+2;

              //   On commence par passer la palette en 256 comme ça, si la
              // nouvelle palette a moins de 256 coul, la précédente ne
              // souffrira pas d'un assombrissement préjudiciable.
              Palette_64_to_256(Principal_Palette);

              if (!(IDB.Indicateur & 0x40))
                // Palette dans l'ordre:
                for(Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                {   
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].V);
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
                }
              else
              {
                // Palette triée par composantes:
                for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].R);
                for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].V);
                for (Indice_de_couleur=0;Indice_de_couleur<Nb_couleurs;Indice_de_couleur++)
                  read_byte(GIF_Fichier,&Principal_Palette[Indice_de_couleur].B);
              }

              Palette_256_to_64(Principal_Palette);
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
                  if (Alphabet_Free==(Read_byte=GIF_Code_actuel))
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
                  Code_ancien=Read_byte;

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

        } // Le fichier contenait une image
        else
          Erreur_fichier=2;

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
  byte Caractere;         // Caractère à coder
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
    if (write_bytes(GIF_Fichier,"GIF87a",6))
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
        LSDB.Largeur=Largeur_ecran;
        LSDB.Hauteur=Hauteur_ecran;
      }
      else
      {
        LSDB.Largeur=Principal_Largeur_image;
        LSDB.Hauteur=Principal_Hauteur_image;
      }
      LSDB.Resol  =0x97;          // Image en 256 couleurs, avec une palette
      LSDB.Backcol=0;
      LSDB.Aspect =0;             // Palette normale

      // On sauve le LSDB dans le fichier

      if (write_bytes(GIF_Fichier,&LSDB,sizeof(T_LSDB)))
      {
        // Le LSDB a été correctement écrit.

        // On sauve la palette

        Palette_64_to_256(Principal_Palette);
        if (write_bytes(GIF_Fichier,Principal_Palette,768))
        {
          // La palette a été correctement écrite.

          //   Le jour où on se servira des blocks d'extensions pour placer
          // des commentaires, on le fera ici.

          // On va écrire un block indicateur d'IDB et l'IDB du fichier

          Block_indicateur=0x2C;
          IDB.Pos_X=0;
          IDB.Pos_Y=0;
          IDB.Largeur_image=Principal_Largeur_image;
          IDB.Hauteur_image=Principal_Hauteur_image;
          IDB.Indicateur=0x07;    // Image non entrelacée, pas de palette locale.
          IDB.Nb_bits_pixel=8; // Image 256 couleurs;

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
              Caractere=GIF_Pixel_suivant();

              //   On regarde si dans la table on aurait pas une chaîne
              // équivalente à Chaine_en_cours+Caractere

              while ( (Indice<Alphabet_Free) &&
                      ( (Chaine_en_cours!=Alphabet_Prefixe[Indice]) ||
                        (Caractere      !=Alphabet_Suffixe[Indice]) ) )
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
                Alphabet_Suffixe[Alphabet_Free++]=Caractere;

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
                Indice=Alphabet_Fille[Caractere];
                Depart=Chaine_en_cours=Caractere;
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

              Chaine_en_cours=0x3B00;        // On écrit un GIF TERMINATOR, exigé par SVGA et SEA.
              if (! write_bytes(GIF_Fichier,&Chaine_en_cours,sizeof(Chaine_en_cours)))
                Erreur_fichier=1;
            }

          } // On a pu écrire l'IDB
          else
            Erreur_fichier=1;

        } // On a pu écrire la palette
        else
          Erreur_fichier=1;

        Palette_256_to_64(Principal_Palette);
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

T_PCX_Header PCX_Header;

// -- Tester si un fichier est au format PCX --------------------------------

void Test_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (read_byte(Fichier,&(PCX_Header.Manufacturer)) &&
        read_byte(Fichier,&(PCX_Header.Version)) &&
        read_byte(Fichier,&(PCX_Header.Compression)) &&
        read_byte(Fichier,&(PCX_Header.Depth)) &&
        read_word_le(Fichier,&(PCX_Header.X_min)) &&
        read_word_le(Fichier,&(PCX_Header.Y_min)) &&
        read_word_le(Fichier,&(PCX_Header.X_max)) &&
        read_word_le(Fichier,&(PCX_Header.Y_max)) &&
        read_word_le(Fichier,&(PCX_Header.X_dpi)) &&
        read_word_le(Fichier,&(PCX_Header.Y_dpi)) &&
        read_bytes(Fichier,&(PCX_Header.Palette_16c),48) &&        
        read_byte(Fichier,&(PCX_Header.Reserved)) &&
        read_byte(Fichier,&(PCX_Header.Plane)) &&
        read_word_le(Fichier,&(PCX_Header.Bytes_per_plane_line)) &&
        read_word_le(Fichier,&(PCX_Header.Palette_info)) &&
        read_word_le(Fichier,&(PCX_Header.Screen_X)) &&
        read_word_le(Fichier,&(PCX_Header.Screen_Y)) &&
        read_bytes(Fichier,&(PCX_Header.Filler),54) )
    {
    
      //   Vu que ce header a une signature de merde et peu significative, il
      // va falloir que je teste différentes petites valeurs dont je connais
      // l'intervalle. Grrr!
      if ( (PCX_Header.Manufacturer!=10)
        || (PCX_Header.Compression>1)
        || ( (PCX_Header.Depth!=1) && (PCX_Header.Depth!=2) && (PCX_Header.Depth!=4) && (PCX_Header.Depth!=8) )
        || ( (PCX_Header.Plane!=1) && (PCX_Header.Plane!=2) && (PCX_Header.Plane!=4) && (PCX_Header.Plane!=8) && (PCX_Header.Plane!=3) )
        || (PCX_Header.X_max<PCX_Header.X_min)
        || (PCX_Header.Y_max<PCX_Header.Y_min)
        || (PCX_Header.Bytes_per_plane_line&1) )
        Erreur_fichier=1;
    }
    else
      Erreur_fichier=1;

    fclose(Fichier);
  }
}


// -- Lire un fichier au format PCX -----------------------------------------

  // -- Afficher une ligne PCX codée sur 1 seul plan avec moins de 256 c. --
  void Draw_PCX_line(short Pos_Y, short Vraie_taille_ligne, byte Depth)
  {
    short Pos_X;
    byte  Couleur;
    byte  Reduction=8/Depth;
    byte  Masque=(1<<Depth)-1;
    byte  Reduction_moins_1=Reduction-1;

    for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
    {
      Couleur=(LBM_Buffer[Pos_X/Reduction]>>((Reduction_moins_1-(Pos_X%Reduction))*Depth)) & Masque;
      Pixel_de_chargement(Pos_X,Pos_Y,Couleur);
    }
  }

void Load_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  
  short Taille_ligne;
  short Vraie_taille_ligne; // Largeur de l'image corrigée
  short Largeur_lue;
  short Pos_X;
  short Pos_Y;
  byte  Octet1;
  byte  Octet2;
  byte  Indice;
  dword Nb_couleurs;
  long  Taille_du_fichier;
  byte  Palette_CGA[9]={ 84,252,252,  252, 84,252,  252,252,252};

  long  Position;
  long  Taille_image;
  byte * Buffer;
  struct stat Informations_Fichier;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
      stat(Nom_du_fichier,&Informations_Fichier);
    Taille_du_fichier=Informations_Fichier.st_size;
    /*
    if (read_bytes(Fichier,&PCX_Header,sizeof(T_PCX_Header)))
    {*/
    
    if (read_byte(Fichier,&(PCX_Header.Manufacturer)) &&
        read_byte(Fichier,&(PCX_Header.Version)) &&
        read_byte(Fichier,&(PCX_Header.Compression)) &&
        read_byte(Fichier,&(PCX_Header.Depth)) &&
        read_word_le(Fichier,&(PCX_Header.X_min)) &&
        read_word_le(Fichier,&(PCX_Header.Y_min)) &&
        read_word_le(Fichier,&(PCX_Header.X_max)) &&
        read_word_le(Fichier,&(PCX_Header.Y_max)) &&
        read_word_le(Fichier,&(PCX_Header.X_dpi)) &&
        read_word_le(Fichier,&(PCX_Header.Y_dpi)) &&
        read_bytes(Fichier,&(PCX_Header.Palette_16c),48) &&        
        read_byte(Fichier,&(PCX_Header.Reserved)) &&
        read_byte(Fichier,&(PCX_Header.Plane)) &&
        read_word_le(Fichier,&(PCX_Header.Bytes_per_plane_line)) &&
        read_word_le(Fichier,&(PCX_Header.Palette_info)) &&
        read_word_le(Fichier,&(PCX_Header.Screen_X)) &&
        read_word_le(Fichier,&(PCX_Header.Screen_Y)) &&
        read_bytes(Fichier,&(PCX_Header.Filler),54) )
    {
      
      Principal_Largeur_image=PCX_Header.X_max-PCX_Header.X_min+1;
      Principal_Hauteur_image=PCX_Header.Y_max-PCX_Header.Y_min+1;

      Ecran_original_X=PCX_Header.Screen_X;
      Ecran_original_Y=PCX_Header.Screen_Y;

      if (PCX_Header.Plane!=3)
      {
        Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_PCX);
        if (Erreur_fichier==0)
        {
          // On prépare la palette à accueillir les valeurs du fichier PCX
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));
          else
            Palette_64_to_256(Principal_Palette);
          Nb_couleurs=(dword)(1<<PCX_Header.Plane)<<(PCX_Header.Depth-1);

          if (Nb_couleurs>4)
            memcpy(Principal_Palette,PCX_Header.Palette_16c,48);
          else
          {
            Principal_Palette[1].R=0;
            Principal_Palette[1].V=0;
            Principal_Palette[1].B=0;
            Octet1=PCX_Header.Palette_16c[3]>>5;
            if (Nb_couleurs==4)
            { // Pal. CGA "alakon" (du Turc Allahkoum qui signifie "à la con" :))
              memcpy(Principal_Palette+1,Palette_CGA,9);
              if (!(Octet1&2))
              {
                Principal_Palette[1].B=84;
                Principal_Palette[2].B=84;
                Principal_Palette[3].B=84;
              }
            } // Palette monochrome (on va dire que c'est du N&B)
            else
            {
              Principal_Palette[1].R=252;
              Principal_Palette[1].V=252;
              Principal_Palette[1].B=252;
            }
          }

          //   On se positionne à la fin du fichier - 769 octets pour voir s'il y
          // a une palette.
          if ( (PCX_Header.Depth==8) && (PCX_Header.Version>=5) && (Taille_du_fichier>(256*3)) )
          {
            fseek(Fichier,Taille_du_fichier-((256*3)+1),SEEK_SET);
            // On regarde s'il y a une palette après les données de l'image
            if (read_byte(Fichier,&Octet1))
              if (Octet1==12) // Lire la palette si c'est une image en 256 couleurs
              {
                int indice;
                // On lit la palette 256c que ces crétins ont foutue à la fin du fichier
                for(indice=0;indice<256;indice++)
                  if ( ! read_byte(Fichier,&Principal_Palette[indice].R)
                   || ! read_byte(Fichier,&Principal_Palette[indice].V)
                   || ! read_byte(Fichier,&Principal_Palette[indice].B) )
                  {
                    Erreur_fichier=2;
                    DEBUG("ERROR READING PCX PALETTE !",indice);
                    break;
                  }
              }
          }
          Palette_256_to_64(Principal_Palette);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          //   Maintenant qu'on a lu la palette que ces crétins sont allés foutre
          // à la fin, on retourne juste après le header pour lire l'image.
          fseek(Fichier,128,SEEK_SET);
          if (!Erreur_fichier)
          {
            Taille_ligne=PCX_Header.Bytes_per_plane_line*PCX_Header.Plane;
            Vraie_taille_ligne=(short)PCX_Header.Bytes_per_plane_line<<3;
            //   On se sert de données LBM car le dessin de ligne en moins de 256
            // couleurs se fait comme avec la structure ILBM.
            Image_HAM=0;
            HBPm1=PCX_Header.Plane-1;
            LBM_Buffer=(byte *)malloc(Taille_ligne);

            // Chargement de l'image
            if (PCX_Header.Compression)  // Image compressée
            {
              /*Init_lecture();*/
  
              Taille_image=(long)PCX_Header.Bytes_per_plane_line*Principal_Hauteur_image;

              if (PCX_Header.Depth==8) // 256 couleurs (1 plan)
              {
                for (Position=0; ((Position<Taille_image) && (!Erreur_fichier));)
                {
                  // Lecture et décompression de la ligne
                  Lire_octet(Fichier,&Octet1);
                  if (!Erreur_fichier)
                  {
                    if ((Octet1&0xC0)==0xC0)
                    {
                      Octet1-=0xC0;               // facteur de répétition
                      Lire_octet(Fichier,&Octet2); // octet à répéter
                      if (!Erreur_fichier)
                      {
                        for (Indice=0; Indice<Octet1; Indice++,Position++)
                          if (Position<Taille_image)
                            Pixel_de_chargement(Position%Taille_ligne,
                                                Position/Taille_ligne,
                                                Octet2);
                          else
                            Erreur_fichier=2;
                      }
                    }
                    else
                    {
                      Pixel_de_chargement(Position%Taille_ligne,
                                          Position/Taille_ligne,
                                          Octet1);
                      Position++;
                    }
                  }
                }
              }
              else                 // couleurs rangées par plans
              {
                for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
                {
                  for (Pos_X=0; ((Pos_X<Taille_ligne) && (!Erreur_fichier)); )
                  {
                    Lire_octet(Fichier,&Octet1);
                    if (!Erreur_fichier)
                    {
                      if ((Octet1&0xC0)==0xC0)
                      {
                        Octet1-=0xC0;               // facteur de répétition
                        Lire_octet(Fichier,&Octet2); // octet à répéter
                        if (!Erreur_fichier)
                        {
                          for (Indice=0; Indice<Octet1; Indice++)
                            if (Pos_X<Taille_ligne)
                              LBM_Buffer[Pos_X++]=Octet2;
                            else
                              Erreur_fichier=2;
                        }
                        else
                          Modif_Erreur_fichier(2);
                      }
                      else
                        LBM_Buffer[Pos_X++]=Octet1;
                    }
                  }
                  // Affichage de la ligne par plan du buffer
                  if (PCX_Header.Depth==1)
                    Draw_ILBM_line(Pos_Y,Vraie_taille_ligne);
                  else
                    Draw_PCX_line(Pos_Y,Vraie_taille_ligne,PCX_Header.Depth);
                }
              }

              /*Close_lecture();*/
            }
            else                     // Image non compressée
            {
              for (Pos_Y=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);Pos_Y++)
              {
                if ((Largeur_lue=read_bytes(Fichier,LBM_Buffer,Taille_ligne)))
                {
                  if (PCX_Header.Plane==1)
                    for (Pos_X=0; Pos_X<Principal_Largeur_image;Pos_X++)
                      Pixel_de_chargement(Pos_X,Pos_Y,LBM_Buffer[Pos_X]);
                  else
                  {
                    if (PCX_Header.Depth==1)
                      Draw_ILBM_line(Pos_Y,Vraie_taille_ligne);
                    else
                      Draw_PCX_line(Pos_Y,Vraie_taille_ligne,PCX_Header.Depth);
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
          Taille_ligne=PCX_Header.Bytes_per_plane_line*3;
          Buffer=(byte *)malloc(Taille_ligne);

          if (!PCX_Header.Compression)
          {
            for (Pos_Y=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);Pos_Y++)
            {
              if (read_bytes(Fichier,Buffer,Taille_ligne))
              {
                for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
                  Pixel_Chargement_24b(Pos_X,Pos_Y,Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*0)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*1)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*2)]);
              }
              else
                Erreur_fichier=2;
            }
          }
          else
          {
            /*Init_lecture();*/

            for (Pos_Y=0,Position=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);)
            {
              // Lecture et décompression de la ligne
              Lire_octet(Fichier,&Octet1);
              if (!Erreur_fichier)
              {
                if ((Octet1 & 0xC0)==0xC0)
                {
                  Octet1-=0xC0;               // facteur de répétition
                  Lire_octet(Fichier,&Octet2); // octet à répéter
                  if (!Erreur_fichier)
                  {
                    for (Indice=0; (Indice<Octet1) && (!Erreur_fichier); Indice++)
                    {
                      Buffer[Position++]=Octet2;
                      if (Position>=Taille_ligne)
                      {
                        for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
                          Pixel_Chargement_24b(Pos_X,Pos_Y,Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*0)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*1)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*2)]);
                        Pos_Y++;
                        Position=0;
                      }
                    }
                  }
                }
                else
                {
                  Buffer[Position++]=Octet1;
                  if (Position>=Taille_ligne)
                  {
                    for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
                      Pixel_Chargement_24b(Pos_X,Pos_Y,Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*0)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*1)],Buffer[Pos_X+(PCX_Header.Bytes_per_plane_line*2)]);
                    Pos_Y++;
                    Position=0;
                  }
                }
              }
            }
            if (Position!=0)
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

    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Ecrire un fichier au format PCX ---------------------------------------

void Save_PCX(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;

  short Taille_ligne;
  short Pos_X;
  short Pos_Y;
  byte  Compteur;
  byte  Last_pixel;
  byte  Pixel_lu;



  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // On prépare la palette pour écrire les 16 premieres valeurs
    Palette_64_to_256(Principal_Palette);

    PCX_Header.Manufacturer=10;
    PCX_Header.Version=5;
    PCX_Header.Compression=1;
    PCX_Header.Depth=8;
    PCX_Header.X_min=0;
    PCX_Header.Y_min=0;
    PCX_Header.X_max=Principal_Largeur_image-1;
    PCX_Header.Y_max=Principal_Hauteur_image-1;
    PCX_Header.X_dpi=0;
    PCX_Header.Y_dpi=0;
    memcpy(PCX_Header.Palette_16c,Principal_Palette,48);
    PCX_Header.Reserved=0;
    PCX_Header.Plane=1;
    PCX_Header.Bytes_per_plane_line=(Principal_Largeur_image&1)?Principal_Largeur_image+1:Principal_Largeur_image;
    PCX_Header.Palette_info=1;
    PCX_Header.Screen_X=Largeur_ecran;
    PCX_Header.Screen_Y=Hauteur_ecran;
    memset(PCX_Header.Filler,0,54);

    if (write_bytes(Fichier,&(PCX_Header.Manufacturer),1) &&
        write_bytes(Fichier,&(PCX_Header.Version),1) &&
        write_bytes(Fichier,&(PCX_Header.Compression),1) &&
        write_bytes(Fichier,&(PCX_Header.Depth),1) &&
        write_word_le(Fichier,PCX_Header.X_min) &&
        write_word_le(Fichier,PCX_Header.Y_min) &&
        write_word_le(Fichier,PCX_Header.X_max) &&
        write_word_le(Fichier,PCX_Header.Y_max) &&
        write_word_le(Fichier,PCX_Header.X_dpi) &&
        write_word_le(Fichier,PCX_Header.Y_dpi) &&
        write_bytes(Fichier,&(PCX_Header.Palette_16c),sizeof(PCX_Header.Palette_16c)) &&        
        write_bytes(Fichier,&(PCX_Header.Reserved),1) &&
        write_bytes(Fichier,&(PCX_Header.Plane),1) &&
        write_word_le(Fichier,PCX_Header.Bytes_per_plane_line) &&
        write_word_le(Fichier,PCX_Header.Palette_info) &&
        write_word_le(Fichier,PCX_Header.Screen_X) &&
        write_word_le(Fichier,PCX_Header.Screen_Y) &&
        write_bytes(Fichier,&(PCX_Header.Filler),sizeof(PCX_Header.Filler)) )
    {
      Taille_ligne=PCX_Header.Bytes_per_plane_line*PCX_Header.Plane;
     
      Init_ecriture();
     
      for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
      {
        Pixel_lu=Lit_pixel_de_sauvegarde(0,Pos_Y);
     
        // Compression et écriture de la ligne
        for (Pos_X=0; ((Pos_X<Taille_ligne) && (!Erreur_fichier)); )
        {
          Pos_X++;
          Last_pixel=Pixel_lu;
          Pixel_lu=Lit_pixel_de_sauvegarde(Pos_X,Pos_Y);
          Compteur=1;
          while ( (Compteur<63) && (Pos_X<Taille_ligne) && (Pixel_lu==Last_pixel) )
          {
            Compteur++;
            Pos_X++;
            Pixel_lu=Lit_pixel_de_sauvegarde(Pos_X,Pos_Y);
          }
      
          if ( (Compteur>1) || (Last_pixel>=0xC0) )
            Ecrire_octet(Fichier,Compteur|0xC0);
          Ecrire_octet(Fichier,Last_pixel);
      
        }
      }
      
      // Ecriture de l'octet (12) indiquant que la palette arrive
      if (!Erreur_fichier)
        Ecrire_octet(Fichier,12);
      
      Close_ecriture(Fichier);
      
      // Ecriture de la palette
      if (!Erreur_fichier)
      {
        if (! write_bytes(Fichier,Principal_Palette,sizeof(T_Palette)))
          Erreur_fichier=1;
      }
    }
    else
      Erreur_fichier=1;
       
    fclose(Fichier);
       
    if (Erreur_fichier)
      remove(Nom_du_fichier);
       
    // On remet la palette à son état normal
    Palette_256_to_64(Principal_Palette);
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
  word Width;              // Largeur de l'image
  word Height;             // Hauteur de l'image
} __attribute__((__packed__))  T_CEL_Header1;

typedef struct
{
  byte Signa[4];           // Signature du format
  byte Kind;               // Type de fichier ($10=PALette $20=BitMaP)
  byte Nbbits;             // Nombre de bits
  word Filler1;            // ???
  word Largeur;            // Largeur de l'image
  word Hauteur;            // Hauteur de l'image
  word Decalage_X;         // Decalage en X de l'image
  word Decalage_Y;         // Decalage en Y de l'image
  byte Filler2[16];        // ???
} __attribute__((__packed__))  T_CEL_Header2;

// -- Tester si un fichier est au format CEL --------------------------------

void Test_CEL(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  int  Taille;
  FILE *Fichier;
  T_CEL_Header1 Header1;
  T_CEL_Header2 Header2;
  struct stat Informations_Fichier;

  Erreur_fichier=0;
  if (!stat(Nom_du_fichier,&Informations_Fichier))
  {
    Erreur_fichier = 1; // Si on ne peut pas faire de stat il vaut mieux laisser tomber
    return;
  }
  Nom_fichier_complet(Nom_du_fichier,0);
  if (! (Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Erreur_fichier = 1;
    return;
  }
  if (read_word_le(Fichier,&Header1.Width) &&
      read_word_le(Fichier,&Header1.Height) )
  {
      //   Vu que ce header n'a pas de signature, il va falloir tester la
      // cohérence de la dimension de l'image avec celle du fichier.
      
      Taille=(Informations_Fichier.st_size)-sizeof(T_CEL_Header1);
      if ( (!Taille) || ( (((Header1.Width+1)>>1)*Header1.Height)!=Taille ) )
      {
        // Tentative de reconnaissance de la signature des nouveaux fichiers

        fseek(Fichier,0,SEEK_SET);        
        if (read_bytes(Fichier,&Header2.Signa,4) &&
            !memcmp(Header2.Signa,"KiSS",4) &&
            read_byte(Fichier,&Header2.Kind) &&
            (Header2.Kind==0x20) &&
            read_byte(Fichier,&Header2.Nbbits) &&
            read_word_le(Fichier,&Header2.Filler1) &&
            read_word_le(Fichier,&Header2.Largeur) &&
            read_word_le(Fichier,&Header2.Hauteur) &&
            read_word_le(Fichier,&Header2.Decalage_X) &&
            read_word_le(Fichier,&Header2.Decalage_Y) &&
            read_bytes(Fichier,&Header2.Filler2,16))
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
  fclose(Fichier);    
}


// -- Lire un fichier au format CEL -----------------------------------------

void Load_CEL(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_CEL_Header1 Header1;
  T_CEL_Header2 Header2;
  short Pos_X;
  short Pos_Y;
  byte  Dernier_octet=0;
  long  Taille_du_fichier;
  struct stat Informations_Fichier;


  Erreur_fichier=0;
  stat(Nom_du_fichier,&Informations_Fichier);
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (read_bytes(Fichier,&Header1,sizeof(T_CEL_Header1)))
    {
      Taille_du_fichier=Informations_Fichier.st_size;
      if ( (Taille_du_fichier>sizeof(T_CEL_Header1))
        && ( (((Header1.Width+1)>>1)*Header1.Height)==(Taille_du_fichier-sizeof(T_CEL_Header1)) ) )
      {
        // Chargement d'un fichier CEL sans signature (vieux fichiers)
        Principal_Largeur_image=Header1.Width;
        Principal_Hauteur_image=Header1.Height;
        Ecran_original_X=Principal_Largeur_image;
        Ecran_original_Y=Principal_Hauteur_image;
        Initialiser_preview(Principal_Largeur_image,Principal_Hauteur_image,Taille_du_fichier,FORMAT_CEL);
        if (Erreur_fichier==0)
        {
          // Chargement de l'image
          /*Init_lecture();*/
          for (Pos_Y=0;((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier));Pos_Y++)
            for (Pos_X=0;((Pos_X<Principal_Largeur_image) && (!Erreur_fichier));Pos_X++)
              if ((Pos_X & 1)==0)
              {
                Lire_octet(Fichier,&Dernier_octet);
                Pixel_de_chargement(Pos_X,Pos_Y,(Dernier_octet >> 4));
              }
              else
                Pixel_de_chargement(Pos_X,Pos_Y,(Dernier_octet & 15));
          /*Close_lecture();*/
        }
      }
      else
      {
        // On réessaye avec le nouveau format

        fseek(Fichier,0,SEEK_SET);
        if (read_bytes(Fichier,&Header2,sizeof(T_CEL_Header2)))
        {
          // Chargement d'un fichier CEL avec signature (nouveaux fichiers)

          Principal_Largeur_image=Header2.Largeur+Header2.Decalage_X;
          Principal_Hauteur_image=Header2.Hauteur+Header2.Decalage_Y;
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
              for (Pos_Y=0;Pos_Y<Header2.Decalage_Y;Pos_Y++)
                for (Pos_X=0;Pos_X<Principal_Largeur_image;Pos_X++)
                  Pixel_de_chargement(Pos_X,Pos_Y,0);
              for (Pos_Y=Header2.Decalage_Y;Pos_Y<Principal_Hauteur_image;Pos_Y++)
                for (Pos_X=0;Pos_X<Header2.Decalage_X;Pos_X++)
                  Pixel_de_chargement(Pos_X,Pos_Y,0);

              switch(Header2.Nbbits)
              {
                case 4:
                  for (Pos_Y=0;((Pos_Y<Header2.Hauteur) && (!Erreur_fichier));Pos_Y++)
                    for (Pos_X=0;((Pos_X<Header2.Largeur) && (!Erreur_fichier));Pos_X++)
                      if ((Pos_X & 1)==0)
                      {
                        Lire_octet(Fichier,&Dernier_octet);
                        Pixel_de_chargement(Pos_X+Header2.Decalage_X,Pos_Y+Header2.Decalage_Y,(Dernier_octet >> 4));
                      }
                      else
                        Pixel_de_chargement(Pos_X+Header2.Decalage_X,Pos_Y+Header2.Decalage_Y,(Dernier_octet & 15));
                  break;

                case 8:
                  for (Pos_Y=0;((Pos_Y<Header2.Hauteur) && (!Erreur_fichier));Pos_Y++)
                    for (Pos_X=0;((Pos_X<Header2.Largeur) && (!Erreur_fichier));Pos_X++)
                    {
                      byte Lu;
                      Lire_octet(Fichier,&Lu);
                      Pixel_de_chargement(Pos_X+Header2.Decalage_X,Pos_Y+Header2.Decalage_Y,Lu);
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
      fclose(Fichier);
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
  FILE *Fichier;
  T_CEL_Header1 Header1;
  T_CEL_Header2 Header2;
  short Pos_X;
  short Pos_Y;
  byte  Dernier_octet=0;
  dword Utilisation[256]; // Table d'utilisation de couleurs


  // On commence par compter l'utilisation de chaque couleurs
  Palette_Compter_nb_couleurs_utilisees(Utilisation);

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (Pos_X=16;((Pos_X<256) && (!Utilisation[Pos_X]));Pos_X++);

    if (Pos_X==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      Header1.Width =Principal_Largeur_image;
      Header1.Height=Principal_Hauteur_image;

      if (write_bytes(Fichier,&Header1,sizeof(T_CEL_Header1)))
      {
        // Sauvegarde de l'image
        Init_ecriture();
        for (Pos_Y=0;((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier));Pos_Y++)
        {
          for (Pos_X=0;((Pos_X<Principal_Largeur_image) && (!Erreur_fichier));Pos_X++)
            if ((Pos_X & 1)==0)
              Dernier_octet=(Lit_pixel_de_sauvegarde(Pos_X,Pos_Y) << 4);
            else
            {
              Dernier_octet=Dernier_octet | (Lit_pixel_de_sauvegarde(Pos_X,Pos_Y) & 15);
              Ecrire_octet(Fichier,Dernier_octet);
            }

          if ((Pos_X & 1)==1)
            Ecrire_octet(Fichier,Dernier_octet);
        }
        Close_ecriture(Fichier);
      }
      else
        Erreur_fichier=1;
      fclose(Fichier);
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      // Recherche du décalage
      for (Pos_Y=0;Pos_Y<Principal_Hauteur_image;Pos_Y++)
      {
        for (Pos_X=0;Pos_X<Principal_Largeur_image;Pos_X++)
          if (Lit_pixel_de_sauvegarde(Pos_X,Pos_Y)!=0)
            break;
        if (Lit_pixel_de_sauvegarde(Pos_X,Pos_Y)!=0)
          break;
      }
      Header2.Decalage_Y=Pos_Y;
      for (Pos_X=0;Pos_X<Principal_Largeur_image;Pos_X++)
      {
        for (Pos_Y=0;Pos_Y<Principal_Hauteur_image;Pos_Y++)
          if (Lit_pixel_de_sauvegarde(Pos_X,Pos_Y)!=0)
            break;
        if (Lit_pixel_de_sauvegarde(Pos_X,Pos_Y)!=0)
          break;
      }
      Header2.Decalage_X=Pos_X;

      memcpy(Header2.Signa,"KiSS",4); // Initialisation de la signature
      Header2.Kind=0x20;              // Initialisation du type (BitMaP)
      Header2.Nbbits=8;               // Initialisation du nombre de bits
      Header2.Filler1=0;              // Initialisation du filler 1 (???)
      Header2.Largeur=Principal_Largeur_image-Header2.Decalage_X; // Initialisation de la largeur
      Header2.Hauteur=Principal_Hauteur_image-Header2.Decalage_Y; // Initialisation de la hauteur
      for (Pos_X=0;Pos_X<16;Pos_X++)  // Initialisation du filler 2 (???)
        Header2.Filler2[Pos_X]=0;

      if (write_bytes(Fichier,&Header2,sizeof(T_CEL_Header2)))
      {
        // Sauvegarde de l'image
        Init_ecriture();
        for (Pos_Y=0;((Pos_Y<Header2.Hauteur) && (!Erreur_fichier));Pos_Y++)
          for (Pos_X=0;((Pos_X<Header2.Largeur) && (!Erreur_fichier));Pos_X++)
            Ecrire_octet(Fichier,Lit_pixel_de_sauvegarde(Pos_X+Header2.Decalage_X,Pos_Y+Header2.Decalage_Y));
        Close_ecriture(Fichier);
      }
      else
        Erreur_fichier=1;
      fclose(Fichier);
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
      byte Octet1;
      byte Octet2;
    } Couleur[16];
  } Palette[10];
} __attribute__((__packed__)) T_KCF_Header;

// -- Tester si un fichier est au format KCF --------------------------------

void Test_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_KCF_Header Buffer;
  T_CEL_Header2 Header2;
  int Indice_palette;
  int Indice_couleur;

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    if (filelength(fileno(Fichier))==sizeof(T_KCF_Header))
    {
      read_bytes(Fichier,&Buffer,sizeof(T_KCF_Header));
      // On vérifie une propriété de la structure de palette:
      for (Indice_palette=0;Indice_palette<10;Indice_palette++)
        for (Indice_couleur=0;Indice_couleur<16;Indice_couleur++)
          if ((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet2>>4)!=0)
            Erreur_fichier=1;
    }
    else
    {
      if (read_bytes(Fichier,&Header2,sizeof(T_CEL_Header2)))
      {
        if (memcmp(Header2.Signa,"KiSS",4)==0)
        {
          if (Header2.Kind!=0x10)
            Erreur_fichier=1;
        }
        else
          Erreur_fichier=1;
      }
      else
        Erreur_fichier=1;
    }
    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Lire un fichier au format KCF -----------------------------------------

void Load_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_KCF_Header Buffer;
  T_CEL_Header2 Header2;
  byte Octet[3];
  int Indice_palette;
  int Indice_couleur;
  int Indice;
  long  Taille_du_fichier;


  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=filelength(fileno(Fichier));
    if (Taille_du_fichier==sizeof(T_KCF_Header))
    {
      // Fichier KCF à l'ancien format

      if (read_bytes(Fichier,&Buffer,sizeof(T_KCF_Header)))
      {
        // Initialiser_preview(???); // Pas possible... pas d'image...

        if (Config.Clear_palette)
          memset(Principal_Palette,0,sizeof(T_Palette));

        // Chargement de la palette
        for (Indice_palette=0;Indice_palette<10;Indice_palette++)
          for (Indice_couleur=0;Indice_couleur<16;Indice_couleur++)
          {
            Indice=16+(Indice_palette*16)+Indice_couleur;
            Principal_Palette[Indice].R=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet1 >> 4) << 2);
            Principal_Palette[Indice].B=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet1 & 15) << 2);
            Principal_Palette[Indice].V=((Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet2 & 15) << 2);
          }

        for (Indice=0;Indice<16;Indice++)
        {
          Principal_Palette[Indice].R=Principal_Palette[Indice+16].R;
          Principal_Palette[Indice].V=Principal_Palette[Indice+16].V;
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

      if (read_bytes(Fichier,&Header2,sizeof(T_CEL_Header2)))
      {
        // Initialiser_preview(???); // Pas possible... pas d'image...

        Indice=(Header2.Nbbits==12)?16:0;
        for (Indice_palette=0;Indice_palette<Header2.Hauteur;Indice_palette++)
        {
           // Pour chaque palette

           for (Indice_couleur=0;Indice_couleur<Header2.Largeur;Indice_couleur++)
           {
             // Pour chaque couleur

             switch(Header2.Nbbits)
             {
               case 12: // RRRR BBBB | 0000 VVVV
                 read_bytes(Fichier,Octet,2);
                 Principal_Palette[Indice].R=(Octet[0] >> 4) << 2;
                 Principal_Palette[Indice].B=(Octet[0] & 15) << 2;
                 Principal_Palette[Indice].V=(Octet[1] & 15) << 2;
                 break;

               case 24: // RRRR RRRR | VVVV VVVV | BBBB BBBB
                 read_bytes(Fichier,Octet,3);
                 Principal_Palette[Indice].R=Octet[0]>>2;
                 Principal_Palette[Indice].V=Octet[1]>>2;
                 Principal_Palette[Indice].B=Octet[2]>>2;
             }

             Indice++;
           }
        }

        if (Header2.Nbbits==12)
          for (Indice=0;Indice<16;Indice++)
          {
            Principal_Palette[Indice].R=Principal_Palette[Indice+16].R;
            Principal_Palette[Indice].V=Principal_Palette[Indice+16].V;
            Principal_Palette[Indice].B=Principal_Palette[Indice+16].B;
          }

        Set_palette(Principal_Palette);
        Remapper_fileselect();
      }
      else
        Erreur_fichier=1;
    }
    fclose(Fichier);
  }
  else
    Erreur_fichier=1;

  if (!Erreur_fichier) Dessiner_preview_palette();
}


// -- Ecrire un fichier au format KCF ---------------------------------------

void Save_KCF(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  FILE *Fichier;
  T_KCF_Header Buffer;
  T_CEL_Header2 Header2;
  byte Octet[3];
  int Indice_palette;
  int Indice_couleur;
  int Indice;
  dword Utilisation[256]; // Table d'utilisation de couleurs

  // On commence par compter l'utilisation de chaque couleurs
  Palette_Compter_nb_couleurs_utilisees(Utilisation);

  Erreur_fichier=0;
  Nom_fichier_complet(Nom_du_fichier,0);
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
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
          Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet1=((Principal_Palette[Indice].R>>2)<<4) | (Principal_Palette[Indice].B>>2);
          Buffer.Palette[Indice_palette].Couleur[Indice_couleur].Octet2=Principal_Palette[Indice].V>>2;
        }

      if (! write_bytes(Fichier,&Buffer,sizeof(T_KCF_Header)))
        Erreur_fichier=1;
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      memcpy(Header2.Signa,"KiSS",4); // Initialisation de la signature
      Header2.Kind=0x10;              // Initialisation du type (PALette)
      Header2.Nbbits=24;              // Initialisation du nombre de bits
      Header2.Filler1=0;              // Initialisation du filler 1 (???)
      Header2.Largeur=256;            // Initialisation du nombre de couleurs
      Header2.Hauteur=1;              // Initialisation du nombre de palettes
      Header2.Decalage_X=0;           // Initialisation du décalage X
      Header2.Decalage_Y=0;           // Initialisation du décalage Y
      for (Indice=0;Indice<16;Indice++) // Initialisation du filler 2 (???)
        Header2.Filler2[Indice]=0;

      if (! write_bytes(Fichier,&Header2,sizeof(T_CEL_Header2)))
        Erreur_fichier=1;

      for (Indice=0;(Indice<256) && (!Erreur_fichier);Indice++)
      {
        Octet[0]=Principal_Palette[Indice].R<<2;
        Octet[1]=Principal_Palette[Indice].V<<2;
        Octet[2]=Principal_Palette[Indice].B<<2;
        if (! write_bytes(Fichier,Octet,3))
          Erreur_fichier=1;
      }
    }

    fclose(Fichier);

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
  word Largeur;
  word Hauteur;
  byte Filler2;
  byte Plans;
} __attribute__((__packed__)) T_SCx_Header;

// -- Tester si un fichier est au format SCx --------------------------------
void Test_SCx(void)
{
  FILE *Fichier;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  //byte Signature[3];
  T_SCx_Header SCx_Header;


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Lecture et vérification de la signature
    if ((read_bytes(Fichier,&SCx_Header,sizeof(T_SCx_Header))))
    {
      if ( (!memcmp(SCx_Header.Filler1,"RIX",3))
        && SCx_Header.Largeur && SCx_Header.Hauteur)
      Erreur_fichier=0;
    }
    // Fermeture du fichier
    fclose(Fichier);
  }
}


// -- Lire un fichier au format SCx -----------------------------------------
void Load_SCx(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  word Pos_X,Pos_Y;
  long Taille,Vraie_taille;
  long Taille_du_fichier;
  T_SCx_Header SCx_Header;
  T_Palette SCx_Palette;

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;

  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille_du_fichier=filelength(fileno(Fichier));

    if ((read_bytes(Fichier,&SCx_Header,sizeof(T_SCx_Header))))
    {
      Initialiser_preview(SCx_Header.Largeur,SCx_Header.Hauteur,Taille_du_fichier,FORMAT_SCx);
      if (Erreur_fichier==0)
      {
        if (!SCx_Header.Plans)
          Taille=sizeof(T_Palette);
        else
          Taille=sizeof(struct Composantes)*(1<<SCx_Header.Plans);

        if (read_bytes(Fichier,SCx_Palette,Taille))
        {
          if (Config.Clear_palette)
            memset(Principal_Palette,0,sizeof(T_Palette));

          memcpy(Principal_Palette,SCx_Palette,Taille);
          Set_palette(Principal_Palette);
          Remapper_fileselect();

          Principal_Largeur_image=SCx_Header.Largeur;
          Principal_Hauteur_image=SCx_Header.Hauteur;

          if (!SCx_Header.Plans)
          { // 256 couleurs (raw)
            LBM_Buffer=(byte *)malloc(Principal_Largeur_image);

            for (Pos_Y=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);Pos_Y++)
            {
              if (read_bytes(Fichier,LBM_Buffer,Principal_Largeur_image))
                for (Pos_X=0; Pos_X<Principal_Largeur_image;Pos_X++)
                  Pixel_de_chargement(Pos_X,Pos_Y,LBM_Buffer[Pos_X]);
              else
                Erreur_fichier=2;
            }
          }
          else
          { // moins de 256 couleurs (planar)
            Taille=((Principal_Largeur_image+7)>>3)*SCx_Header.Plans;
            Vraie_taille=(Taille/SCx_Header.Plans)<<3;
            LBM_Buffer=(byte *)malloc(Taille);
            HBPm1=SCx_Header.Plans-1;
            Image_HAM=0;

            for (Pos_Y=0;(Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier);Pos_Y++)
            {
              if (read_bytes(Fichier,LBM_Buffer,Taille))
                Draw_ILBM_line(Pos_Y,Vraie_taille);
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

    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}

// -- Sauver un fichier au format SCx ---------------------------------------
void Save_SCx(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  short Pos_X,Pos_Y;
  T_SCx_Header SCx_Header;

  Nom_fichier_complet(Nom_du_fichier,1);

  Erreur_fichier=0;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    memcpy(SCx_Header.Filler1,"RIX3",4);
    SCx_Header.Largeur=Principal_Largeur_image;
    SCx_Header.Hauteur=Principal_Hauteur_image;
    SCx_Header.Filler2=0xAF;
    SCx_Header.Plans=0x00;

    if (write_bytes(Fichier,&SCx_Header,sizeof(T_SCx_Header)) &&
      write_bytes(Fichier,&Principal_Palette,sizeof(T_Palette)))
    {
      Init_ecriture();

      for (Pos_Y=0; ((Pos_Y<Principal_Hauteur_image) && (!Erreur_fichier)); Pos_Y++)
        for (Pos_X=0; Pos_X<Principal_Largeur_image; Pos_X++)
          Ecrire_octet(Fichier,Lit_pixel_de_sauvegarde(Pos_X,Pos_Y));

      Close_ecriture(Fichier);
      fclose(Fichier);

      if (Erreur_fichier)
        remove(Nom_du_fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(Fichier);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
  }
  else
  {
    fclose(Fichier);
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
  word masque;      // Masque de decodage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la source

  masque=0x8000;
  w0=(((word)Src[0])<<8) | Src[1];
  w1=(((word)Src[2])<<8) | Src[3];
  w2=(((word)Src[4])<<8) | Src[5];
  w3=(((word)Src[6])<<8) | Src[7];
  for (i=0;i<16;i++)
  {
    // Pour décoder le pixel n°i, il faut traiter les 4 words sur leur bit
    // correspondant à celui du masque

    Dst[i]=((w0 & masque)?0x01:0x00) |
           ((w1 & masque)?0x02:0x00) |
           ((w2 & masque)?0x04:0x00) |
           ((w3 & masque)?0x08:0x00);
    masque>>=1;
  }
}

//// CODAGE d'une partie d'IMAGE ////

void PI1_16p_to_8b(byte * Src,byte * Dst)
{
  int  i;           // Indice du pixel à calculer
  word masque;      // Masque de codage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la destination

  masque=0x8000;
  w0=w1=w2=w3=0;
  for (i=0;i<16;i++)
  {
    // Pour coder le pixel n°i, il faut modifier les 4 words sur leur bit
    // correspondant à celui du masque

    w0|=(Src[i] & 0x01)?masque:0x00;
    w1|=(Src[i] & 0x02)?masque:0x00;
    w2|=(Src[i] & 0x04)?masque:0x00;
    w3|=(Src[i] & 0x08)?masque:0x00;
    masque>>=1;
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

void PI1_Decoder_palette(byte * Src,byte * Pal)
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
    Pal[ip++]=(((w & 0x0007) <<  1) | ((w & 0x0008) >>  3)) << 2;
    Pal[ip++]=(((w & 0x7000) >> 11) | ((w & 0x8000) >> 15)) << 2;
    Pal[ip++]=(((w & 0x0700) >>  7) | ((w & 0x0800) >> 11)) << 2;
  }
}

//// CODAGE de la PALETTE ////

void PI1_Coder_palette(byte * Pal,byte * Dst)
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
    w =(((word)Pal[ip] & 0x38) >> 3) | (((word)Pal[ip] & 0x04) <<  1); ip++;
    w|=(((word)Pal[ip] & 0x38) << 9) | (((word)Pal[ip] & 0x04) << 13); ip++;
    w|=(((word)Pal[ip] & 0x38) << 5) | (((word)Pal[ip] & 0x04) <<  9); ip++;

    Dst[(i*2)+0]=w & 0x00FF;
    Dst[(i*2)+1]=(w>>8);
  }
}


// -- Tester si un fichier est au format PI1 --------------------------------
void Test_PI1(void)
{
  FILE * Fichier;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  int  Taille;              // Taille du fichier
  word Res;                 // Résolution de l'image


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Vérification de la taille
    Taille=filelength(fileno(Fichier));
    if ((Taille==32034) || (Taille==32066))
    {
      // Lecture et vérification de la résolution
      if (read_word_le(Fichier,&Res))
      {
        if (Res==0x0000)
          Erreur_fichier=0;
      }
    }
    // Fermeture du fichier
    fclose(Fichier);
  }
}


// -- Lire un fichier au format PI1 -----------------------------------------
void Load_PI1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  word Pos_X,Pos_Y;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (read_bytes(Fichier,buffer,32034))
      {
        // Initialisation de la preview
        Initialiser_preview(320,200,filelength(fileno(Fichier)),FORMAT_PI1);
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
          for (Pos_Y=0;Pos_Y<200;Pos_Y++)
          {
            for (Pos_X=0;Pos_X<(320>>4);Pos_X++)
            {
              PI1_8b_to_16p(ptr,pixels+(Pos_X<<4));
              ptr+=8;
            }
            for (Pos_X=0;Pos_X<320;Pos_X++)
              Pixel_de_chargement(Pos_X,Pos_Y,pixels[Pos_X]);
          }
        }
      }
      else
        Erreur_fichier=1;
      free(buffer);
    }
    else
      Erreur_fichier=1;
    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PI1 ---------------------------------------
void Save_PI1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  short Pos_X,Pos_Y;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_Coder_palette((byte *)Principal_Palette,buffer+2);
    // Codage de l'image
    ptr=buffer+34;
    for (Pos_Y=0;Pos_Y<200;Pos_Y++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (Pos_Y<Principal_Hauteur_image)
      {
        for (Pos_X=0;(Pos_X<320) && (Pos_X<Principal_Largeur_image);Pos_X++)
          pixels[Pos_X]=Lit_pixel_de_sauvegarde(Pos_X,Pos_Y);
      }

      for (Pos_X=0;Pos_X<(320>>4);Pos_X++)
      {
        PI1_16p_to_8b(pixels+(Pos_X<<4),ptr);
        ptr+=8;
      }
    }

    if (write_bytes(Fichier,buffer,32034))
    {
      fclose(Fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(Fichier);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
    // Libération du buffer mémoire
    free(buffer);
  }
  else
  {
    fclose(Fichier);
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
  byte masque;      // Masque de decodage
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
    masque=0x80;
    for (j=0;j<8;j++)
    {
      Dst[ip++]=((b0 & masque)?0x01:0x00) |
                ((b1 & masque)?0x02:0x00) |
                ((b2 & masque)?0x04:0x00) |
                ((b3 & masque)?0x08:0x00);
      masque>>=1;
    }
  }
}

//// CODAGE d'une partie d'IMAGE ////

// Transformation d'1 ligne de pixels en 4 plans de bits

void PC1_1lp_to_4pb(byte * Src,byte * Dst0,byte * Dst1,byte * Dst2,byte * Dst3)
{
  int  i,j;         // Compteurs
  int  ip;          // Indice du pixel à calculer
  byte masque;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    // Pour chacun des 8 bits des octets
    masque=0x80;
    b0=b1=b2=b3=0;
    for (j=0;j<8;j++)
    {
      b0|=(Src[ip] & 0x01)?masque:0x00;
      b1|=(Src[ip] & 0x02)?masque:0x00;
      b2|=(Src[ip] & 0x04)?masque:0x00;
      b3|=(Src[ip] & 0x08)?masque:0x00;
      ip++;
      masque>>=1;
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
  FILE *Fichier;              // Fichier du fichier
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  int  Taille;              // Taille du fichier
  word Res;                 // Résolution de l'image


  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=1;

  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    // Vérification de la taille
    Taille=filelength(fileno(Fichier));
    if ((Taille<=32066))
    {
      // Lecture et vérification de la résolution
      if (read_word_le(Fichier,&Res))
      {
        if (Res==0x0080)
          Erreur_fichier=0;
      }
    }
    // Fermeture du fichier
    fclose(Fichier);
  }
}


// -- Lire un fichier au format PC1 -----------------------------------------
void Load_PC1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  int  Taille;
  word Pos_X,Pos_Y;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  if ((Fichier=fopen(Nom_du_fichier, "rb")))
  {
    Taille=filelength(fileno(Fichier));
    // allocation des buffers mémoire
    buffercomp=(byte *)malloc(Taille);
    bufferdecomp=(byte *)malloc(32000);
    if ( (buffercomp!=NULL) && (bufferdecomp!=NULL) )
    {
      // Lecture du fichier dans le buffer
      if (read_bytes(Fichier,buffercomp,Taille))
      {
        // Initialisation de la preview
        Initialiser_preview(320,200,filelength(fileno(Fichier)),FORMAT_PC1);
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
          for (Pos_Y=0;Pos_Y<200;Pos_Y++)
          {
            // Décodage de la scanline
            PC1_4pb_to_1lp(ptr,ptr+40,ptr+80,ptr+120,pixels);
            ptr+=160;
            // Chargement de la ligne
            for (Pos_X=0;Pos_X<320;Pos_X++)
              Pixel_de_chargement(Pos_X,Pos_Y,pixels[Pos_X]);
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
    fclose(Fichier);
  }
  else
    Erreur_fichier=1;
}


// -- Sauver un fichier au format PC1 ---------------------------------------
void Save_PC1(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  FILE *Fichier;
  int   Taille;
  short Pos_X,Pos_Y;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Nom_fichier_complet(Nom_du_fichier,0);

  Erreur_fichier=0;
  // Ouverture du fichier
  if ((Fichier=fopen(Nom_du_fichier,"wb")))
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
    for (Pos_Y=0;Pos_Y<200;Pos_Y++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (Pos_Y<Principal_Hauteur_image)
      {
        for (Pos_X=0;(Pos_X<320) && (Pos_X<Principal_Largeur_image);Pos_X++)
          pixels[Pos_X]=Lit_pixel_de_sauvegarde(Pos_X,Pos_Y);
      }

      // Encodage de la scanline
      PC1_1lp_to_4pb(pixels,ptr,ptr+40,ptr+80,ptr+120);
      ptr+=160;
    }

    // Compression du buffer
    PC1_Compresser_PackBits(bufferdecomp,buffercomp+34,32000,&Taille);
    Taille+=34;
    for (Pos_X=0;Pos_X<16;Pos_X++)
      buffercomp[Taille++]=0;

    if (write_bytes(Fichier,buffercomp,Taille))
    {
      fclose(Fichier);
    }
    else // Erreur d'écriture (disque plein ou protégé)
    {
      fclose(Fichier);
      remove(Nom_du_fichier);
      Erreur_fichier=1;
    }
    // Libération des buffers mémoire
    free(bufferdecomp);
    free(buffercomp);
  }
  else
  {
    fclose(Fichier);
    remove(Nom_du_fichier);
    Erreur_fichier=1;
  }
}

/******************************************************************************************
  Functions from op_c.c : load raw 24B (for testing) and load_tga (broken ? incomplete ?)
  Saving would be nice as well.
******************************************************************************************/
void Load_RAW_24B(int Largeur,int Hauteur,Bitmap24B Source)
{
  FILE* Fichier;

  Fichier=fopen("TEST.RAW","rb");
  if (read_bytes(Fichier,Source,Largeur*Hauteur*sizeof(struct Composantes)))
    exit(3);
  fclose(Fichier);
}

void Load_TGA(char * nom,Bitmap24B * dest,int * larg,int * haut)
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
    word Width;                // Largeur de l'image
    word Height;               // Hauteur de l'image
    byte Pixel_size;           // Pixels sur 16, 24 ou 32 bits
    byte Descriptor;           // Paramètres divers
  } TGA_Header;
  int x,y,py,skip,t;
  byte * buffer;

  fichier=fopen(nom,"rb");
  read_bytes(fichier,&TGA_Header,sizeof(TGA_Header));
  if (TGA_Header.Image_type_code==2)
  {
    *larg=TGA_Header.Width;
    *haut=TGA_Header.Height;
    *dest=(Bitmap24B)malloc((*larg)*(*haut)*3);

    // On saute l'ID field
    fseek(fichier,TGA_Header.Id_field_size,SEEK_CUR);

    // On saute la palette
    if (TGA_Header.Color_map_type==0)
      skip=0;
    else
    {
      skip=TGA_Header.Color_map_length;
      if (TGA_Header.Color_map_entry_size==16)
        skip*=2;
      else
      if (TGA_Header.Color_map_entry_size==24)
        skip*=3;
      else
      if (TGA_Header.Color_map_entry_size==32)
        skip*=4;
    }
    fseek(fichier,skip,SEEK_CUR);

    // Lecture des pixels
    skip=(*larg);
    if (TGA_Header.Pixel_size==16)
      skip*=2;
    else
    if (TGA_Header.Pixel_size==24)
      skip*=3;
    else
    if (TGA_Header.Pixel_size==32)
      skip*=4;

    buffer=(byte *)malloc(skip);
    for (y=0;y<(*haut);y++)
    {
      read_bytes(fichier,buffer,skip);

      // Inversion du rouge et du bleu
      for (x=0;x<(*larg);x++)
      {
        t=buffer[(x*3)+0];
        buffer[(x*3)+0]=buffer[(x*3)+2];
        buffer[(x*3)+2]=t;
      }

      // Prise en compte du sens d'écriture verticale
      if (TGA_Header.Descriptor & 0x20)
        py=y;
      else
        py=(*haut)-y-1;

      // Prise en compte de l'interleave verticale
      if (TGA_Header.Descriptor & 0xC0)
        py=((py % (*haut))*2)+(py/(*haut));

      memcpy((*dest)+(py*(*larg)),buffer,skip);
    }
    free(buffer);
  }
  fclose(fichier);
}
