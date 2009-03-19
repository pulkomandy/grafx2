/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
#ifndef _STRUCT_H_
#define _STRUCT_H_

#if defined(__BEOS__)
    #include <inttypes.h>
#else
    #include <stdint.h>
#endif

#include "const.h"

// Déclaration des types de base /////////////////////////////////////////////

typedef uint8_t  byte;
typedef uint16_t word;
typedef uint32_t  dword;

typedef void (* fonction_action)    (void);
typedef void (* fonction_afficheur) (word,word,byte);
typedef byte (* fonction_lecteur)   (word,word);
typedef void (* fonction_effaceur)  (byte);
typedef void (* fonction_display)   (word,word,word);
typedef byte (* fonction_effet)     (word,word,byte);
typedef void (* fonction_block)     (word,word,word,word,byte);
typedef void (* fonction_Ligne_XOR) (word,word,word);
typedef void (* fonction_display_brush_Color) (word,word,word,word,word,word,byte,word);
typedef void (* fonction_display_brush_Mono)  (word,word,word,word,word,word,byte,byte,word);
typedef void (* fonction_degrade)   (long,short,short);
typedef void (* fonction_remap)     (word,word,word,word,byte *);
typedef void (* fonction_procsline) (word,word,word,byte *);
typedef void (* fonction_display_zoom) (word,word,word,byte *);
typedef void (* fonction_display_brush_Color_zoom) (word,word,word,word,word,word,byte,word,byte *);
typedef void (* fonction_display_brush_Mono_zoom)  (word,word,word,word,word,word,byte,byte,word,byte *);
typedef void (* fonction_affiche_brosse) (byte *,word,word,word,word,word,word,byte,word);

typedef struct
{
  byte R;
  byte V;
  byte B;
}__attribute__ ((__packed__)) Composantes, T_Palette[256];

typedef struct T_Bouton_normal
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Width;
  word Height;
  byte Clickable;
  byte Repetable;
  word Raccourci;
  struct T_Bouton_normal * Next;
} T_Bouton_normal;

typedef struct T_Bouton_palette
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  struct T_Bouton_palette * Next;
} T_Bouton_palette;

typedef struct T_Bouton_scroller
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Height;
  word Nb_elements;
  word Nb_visibles;
  word Position;
  word Hauteur_curseur;
  struct T_Bouton_scroller * Next;
} T_Bouton_scroller;

typedef struct T_Bouton_special
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Width;
  word Height;
  struct T_Bouton_special * Next;
} T_Bouton_special;

typedef struct T_Dropdown_choix
{
  short Numero;
  const char * Label;
  struct T_Dropdown_choix * Next;
} T_Dropdown_choix;

typedef struct T_Bouton_dropdown
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Width;
  word Height;
  byte Affiche_choix;  // The selected item's label is printed in the dropdown area
  byte Affiche_centre; // Center labels (otherwise, align left)
  byte Affiche_fleche; // Display a "down" arrow box in top right
  byte Bouton_actif; // Mouse button: A_GAUCHE || A_DROITE || (A_GAUCHE|A_DROITE)
  word Largeur_choix; // 0 for "same as control"
  T_Dropdown_choix * Premier_choix;
  struct T_Bouton_dropdown * Next;
} T_Bouton_dropdown;

// Déclaration du type d'élément qu'on va mémoriser dans la liste:
typedef struct Element_de_liste_de_fileselect
{
  char NomAbrege[13]; // Le nom tel qu'affiché dans le fileselector
  char NomComplet[256]; // Le nom du fichier ou du répertoire
  byte Type;    // Type d'élément : 0 = Fichier, 1 = Répertoire, 2 = Lecteur

  // données de chaînage de la liste
  struct Element_de_liste_de_fileselect * Suivant;
  struct Element_de_liste_de_fileselect * Precedent;
} Element_de_liste_de_fileselect;

typedef struct {
  char Line_type;
  char * Text;
  int Line_parameter;
} T_TABLEAIDE;

// Déclaration d'une section d'aide:
typedef struct
{
  const T_TABLEAIDE* Table_aide; // Pointeur sur le début de la table d'aide
  word Nombre_de_lignes;
} Section_d_aide;

// Déclaration d'une info sur un dégradé
typedef struct
{
  byte Debut;     // Première couleur du dégradé
  byte Fin;       // Dernière couleur du dégradé
  dword Inverse;   // "Le dégradé va de Fin à Debut" //INT
  dword Melange;   // Valeur de mélange du dégradé (0-255) //LONG
  dword Technique; // Technique à utiliser (0-2) //INT
} __attribute__((__packed__)) T_Degrade_Tableau;

// Déclaration d'une info de shade
typedef struct
{
  word List[512]; // Liste de couleurs
  byte Pas;        // Pas d'incrémentation/décrémentation
  byte Mode;       // Mode d'utilisation (Normal/Boucle/Non-saturé)
} T_Shade;



// Structure des données dans le fichier de config.

typedef struct
{
  byte Etat;
  word Width;
  word Height;
} __attribute__((__packed__)) Config_Mode_video;

typedef struct
{
  char Signature[3];
  byte Version1;
  byte Version2;
  byte Beta1;
  byte Beta2;
} __attribute__((__packed__)) Config_Header;

typedef struct
{
  byte Numero;
  word Taille;
} __attribute__((__packed__)) Config_Chunk;

typedef struct
{
  word Numero;
  word Touche;
  word Touche2;
} __attribute__((__packed__)) Config_Infos_touche;

typedef struct
{
  byte Fonte;
  int  Lire_les_fichiers_caches;
  int  Lire_les_repertoires_caches;
//  int  Lire_les_repertoires_systemes;
  byte Afficher_limites_image;
  byte Curseur;
  byte Maximize_preview;
  byte Auto_set_res;
  byte Coords_rel;
  byte Backup;
  byte Adjust_brush_pick;
  byte Auto_save;
  byte Nb_pages_Undo;
  byte Indice_Sensibilite_souris_X;
  byte Indice_Sensibilite_souris_Y;
  byte Mouse_Facteur_de_correction_X;
  byte Mouse_Facteur_de_correction_Y;
  byte Mouse_Merge_movement;
  byte Valeur_tempo_jauge_gauche;
  byte Valeur_tempo_jauge_droite;
  long Chrono_delay;
  Composantes Coul_menu_pref[4];
  int  Nb_max_de_vertex_par_polygon;
  byte Clear_palette;
  byte Set_resolution_according_to;
  byte Ratio;
  byte Fast_zoom;
  byte Find_file_fast;
  byte Couleurs_separees;
  word Palette_Cells_X;
  word Palette_Cells_Y;
  byte FX_Feedback;
  byte Safety_colors;
  byte Opening_message;
  byte Clear_with_stencil;
  byte Auto_discontinuous;
  byte Taille_ecran_dans_GIF;
  byte Auto_nb_used;
  byte Resolution_par_defaut;
  char *Bookmark_directory[NB_BOOKMARKS]; // independant malloc of adaptive size
  char Bookmark_label[NB_BOOKMARKS][8+1];
} T_Config;

// Structures utilisées pour les descriptions de pages et de liste de pages.
// Lorsqu'on gèrera les animations, il faudra aussi des listes de listes de
// pages.

// Ces structures sont manipulées à travers des fonctions de gestion du
// backup dans "graph.c".

typedef struct
{
  byte *    Image;   // Bitmap de l'image
  int       Width; // Largeur du bitmap
  int       Height; // Hauteur du bitmap
  T_Palette Palette; // Palette de l'image

  char      Commentaire[TAILLE_COMMENTAIRE+1]; // Commentaire de l'image

  char      Repertoire_fichier[TAILLE_CHEMIN_FICHIER]; // |_ Nom complet =
  char      Nom_fichier[TAILLE_CHEMIN_FICHIER];        // |  Repertoire_fichier+"\"+Nom_fichier
  byte      Format_fichier;          // Format auquel il faut lire et écrire le fichier

/*
  short     Decalage_X; // Décalage en X de l'écran par rapport au début de l'image
  short     Decalage_Y; // Décalage en Y de l'écran par rapport au début de l'image
  short     Ancien_Decalage_X; // Le même avant le passage en mode loupe
  short     Ancien_Decalage_Y; // Le même avant le passage en mode loupe

  short     Split; // Position en X du bord gauche du split de la loupe
  short     X_Zoom; // (Menu_Facteur_X) + Position en X du bord droit du split de la loupe
  float     Proportion_split; // Proportion de la zone non-zoomée par rapport à l'écran

  byte      Loupe_Mode;       // On est en mode loupe
  word      Loupe_Facteur;    // Facteur de zoom
  word      Loupe_Hauteur;    // Largeur de la fenêtre de zoom
  word      Loupe_Largeur;    // Hauteur de la fenêtre de zoom
  short     Loupe_Decalage_X; // Decalage horizontal de la fenêtre de zoom
  short     Loupe_Decalage_Y; // Decalage vertical   de la fenêtre de zoom
*/
} S_Page;

typedef struct
{
  int      Taille_liste;      // Nb de S_Page dans le vecteur "Pages"
  int      Nb_pages_allouees; // Nb de S_Page désignant des pages allouées
  S_Page * Pages;             // Liste de pages (Taille_liste éléments)
} S_Liste_de_pages;



#endif
