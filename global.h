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
#ifdef VARIABLES_GLOBALES
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//        Ce fichier contient les déclaration des variables globales        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


// -- Section des variables de CONFIGURATION ---------------------------------

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <SDL/SDL.h>
#include "struct.h"

GLOBAL struct S_Config
{
  byte Fonte;
  int  Lire_les_fichiers_caches;
  int  Lire_les_repertoires_caches;
  int  Lire_les_repertoires_systemes;
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
  byte Valeur_tempo_jauge_gauche;
  byte Valeur_tempo_jauge_droite;
  long Chrono_delay;
  struct Composantes Coul_menu_pref[4];
  int  Nb_max_de_vertex_par_polygon;
  byte Clear_palette;
  byte Set_resolution_according_to;
  byte Ratio;
  byte Fast_zoom;
  byte Find_file_fast;
  byte Couleurs_separees;
  byte FX_Feedback;
  byte Safety_colors;
  byte Opening_message;
  byte Clear_with_stencil;
  byte Auto_discontinuous;
  byte Taille_ecran_dans_GIF;
  byte Auto_nb_used;
  byte Resolution_par_defaut;
} Config;

  // Tableau des touches spéciales
GLOBAL word Config_Touche[NB_TOUCHES_SPECIALES];


struct S_Mode_video
{
  short  Largeur;
  short  Hauteur;
  byte   Mode;
  word   Fullscreen;
  byte   Etat; // 0:Cool 1:OK ; 2:Bof ; 3:Naze ; si on rajoute +128 => incompatible
};
GLOBAL struct S_Mode_video Mode_video[MAX_MODES_VIDEO];

GLOBAL int  Nb_modes_video; // Nombre de modes réellement recensés dans Mode_video[]

GLOBAL struct
{
  byte   Granularite;   // Facteur de gestion de la granularité
  byte   Code_fenetres; // Utilisation des fenêtres: 0=AA 1=BB 2=AB 3=BA
  byte * WinFuncPtr;
  byte * Adresse_physique_LFB; // Si = 0 => Pas de LFB
  dword  Taille_LFB;           // Taille de la mémoire LFB
} VESA_Mode_Infos[4];



  // Palette par défaut

GLOBAL T_Palette Palette_defaut;

  // Couleurs du menu

GLOBAL byte CM_Noir;
GLOBAL byte CM_Fonce;
GLOBAL byte CM_Clair;
GLOBAL byte CM_Blanc;
GLOBAL byte CM_Trans;
GLOBAL struct Composantes Coul_menu_pref[4];

  // Etat des entrées

GLOBAL word Mouse_X; // Abscisse de la souris
GLOBAL word Mouse_Y; // Ordonnée de la souris
GLOBAL byte Mouse_K; // Etat des boutons de la souris
GLOBAL dword Touche; // Touche tapée
GLOBAL dword Touche_ANSI; // Caractère tapé
GLOBAL Uint8* Etat_Du_Clavier;  // Scancode de la touche en cours et etat des touches de ctrl
// Modificateurs pour Touche
#define MOD_SHIFT 0x1000
#define MOD_CTRL  0x2000
#define MOD_ALT   0x4000
GLOBAL byte Quit_demande; // !=0 lorsque l'utilisateur demande la fermeture de fenêtre.

GLOBAL byte Mouse_Facteur_de_correction_X;
GLOBAL byte Mouse_Facteur_de_correction_Y;

GLOBAL byte Autoriser_changement_de_couleur_pendant_operation;

  // Données sur le curseur

GLOBAL byte Forme_curseur;
GLOBAL byte Forme_curseur_avant_fenetre; // Forme du curseur avant l'ouverture d'une fenêtre
GLOBAL byte Forcer_affichage_curseur;    // Forcer l'affichage du curseur au prochain Get_input();
GLOBAL byte Cacher_curseur;
GLOBAL byte Curseur_dans_menu;           // Booléen "Le curseur se trouve dans le menu"
GLOBAL byte Curseur_dans_menu_precedent; // Booléen "Le curseur se trouvait précédemment dans le menu"
GLOBAL word Curseur_Decalage_X[NB_SPRITES_CURSEUR]; // Coordonnées X du point sensible de curseurs en sprite
GLOBAL word Curseur_Decalage_Y[NB_SPRITES_CURSEUR]; // Coordonnées Y du point sensible de curseurs en sprite
GLOBAL byte SPRITE_CURSEUR[NB_SPRITES_CURSEUR][HAUTEUR_SPRITE_CURSEUR][LARGEUR_SPRITE_CURSEUR]; // Dessins des sprites de curseur
GLOBAL byte FOND_CURSEUR[HAUTEUR_SPRITE_CURSEUR][LARGEUR_SPRITE_CURSEUR]; // Contenu du dessous du curseur

  // Données sur le pinceau

GLOBAL byte  Pinceau_Forme;
GLOBAL byte  Pinceau_Forme_avant_fill;
GLOBAL byte  Pinceau_Forme_avant_pipette;
GLOBAL byte  Pinceau_Forme_avant_lasso;
GLOBAL byte  Cacher_pinceau;
GLOBAL short Pinceau_X;
GLOBAL short Pinceau_Y;
GLOBAL byte  SPRITE_PINCEAU [NB_SPRITES_PINCEAU][HAUTEUR_PINCEAU][LARGEUR_PINCEAU];
GLOBAL word  Pinceau_predefini_Largeur[NB_SPRITES_PINCEAU];
GLOBAL word  Pinceau_predefini_Hauteur[NB_SPRITES_PINCEAU];
GLOBAL byte  Pinceau_Type[NB_SPRITES_PINCEAU];
GLOBAL word  Pinceau_predefini_Decalage_X[NB_SPRITES_PINCEAU];
GLOBAL word  Pinceau_predefini_Decalage_Y[NB_SPRITES_PINCEAU];
GLOBAL byte * Pinceau_Sprite;
GLOBAL short Pinceau_Largeur;
GLOBAL short Pinceau_Hauteur;
GLOBAL short Pinceau_Decalage_X;
GLOBAL short Pinceau_Decalage_Y;

  // Commandes graphiques

GLOBAL fonction_afficheur Pixel;          // Affiche un point à l'écran
GLOBAL fonction_afficheur Pixel_dans_menu;// Affiche un point dans le menu (ou pas si le menu est invisible)
GLOBAL fonction_lecteur   Lit_pixel;      // Teste la couleur d'un pixel dans l'écran
GLOBAL fonction_effaceur  Clear_screen;   // Efface rapidement tout l'écran (en faisant attention de ne pas effacer le menu)
GLOBAL fonction_display   Display_screen; // Affiche rapidement tout l'écran (en faisant attention de ne pas effacer le menu)
GLOBAL fonction_block     Block;          // Affiche rapidement un bloc à l'écran
GLOBAL fonction_afficheur Pixel_Preview_Normal; // Affiche un point de l'image à l'écran en mode normal (pas en mode loupe)
GLOBAL fonction_afficheur Pixel_Preview_Loupe;  // Affiche un point de l'image à l'écran en mode loupe
GLOBAL fonction_afficheur Pixel_Preview;        // Affiche un point de l'image à l'écran en fonction de l'état du mode loupe
GLOBAL fonction_Ligne_XOR Ligne_horizontale_XOR;// Affiche une ligne horizontale en XOR (pour placer la loupe)
GLOBAL fonction_Ligne_XOR Ligne_verticale_XOR;  // Affiche une ligne verticale en XOR (pour placer la loupe)
GLOBAL fonction_display_brush_Color Display_brush_Color; // Affiche une partie de la brosse en couleur
GLOBAL fonction_display_brush_Mono  Display_brush_Mono;  // Affiche une partie de la brosse en monochrome
GLOBAL fonction_display_brush_Color Clear_brush;         // Efface la partie de la brosse affichée à l'écran
GLOBAL fonction_remap     Remap_screen;   // Remappe une partie de l'écran avec les nouvelles couleurs du menu
GLOBAL fonction_procsline Afficher_ligne;  // Afficher une ligne
GLOBAL fonction_procsline Lire_ligne;      // Afficher ou lire une ligne
GLOBAL fonction_display_zoom Display_zoomed_screen; // Affiche rapidement toute la partie zoomée à l'écran (en faisant attention de ne pas effacer le menu)
GLOBAL fonction_display_brush_Color_zoom Display_brush_Color_zoom;
GLOBAL fonction_display_brush_Mono_zoom  Display_brush_Mono_zoom;
GLOBAL fonction_display_brush_Color_zoom Clear_brush_zoom;

  // Données sur les dimensions de l'écran

GLOBAL int   Resize_Largeur;      // \__ Positionnées lorsque l'utilisateur tire
GLOBAL int   Resize_Hauteur;      // /      un bord de la fenêtre.
GLOBAL int   Resolution_actuelle; // Résolution graphique courante
GLOBAL short Ecran_original_X;    // |_ Dimensions de l'écran d'origine de
GLOBAL short Ecran_original_Y;    // |  l'image qui vient d'être chargée.
GLOBAL short Largeur_ecran;       // Largeur de l'écran
GLOBAL short Hauteur_ecran;       // Hauteur de l'écran
GLOBAL byte Plein_ecran;	  // Indique si on est en mode plein écran ou fenêtré
GLOBAL short Limite_Haut;         // |
GLOBAL short Limite_Bas;          // |_ Limites dans lesquelles
GLOBAL short Limite_Gauche;       // |  on peut écrire
GLOBAL short Limite_Droite;       // |
GLOBAL short Limite_visible_Bas;    // |_ Derniers points visibles
GLOBAL short Limite_visible_Droite; // |  "à l'image"

GLOBAL short Limite_Haut_Zoom;         // |
GLOBAL short Limite_Bas_Zoom;          // |_ Limites dans lesquelles on peut
GLOBAL short Limite_Gauche_Zoom;       // |  écrire dans la partie zoomée
GLOBAL short Limite_Droite_Zoom;       // |
GLOBAL short Limite_visible_Bas_Zoom;    // |_ Derniers points visibles "à
GLOBAL short Limite_visible_Droite_Zoom; // |  l'image" dans la partie zoomée

GLOBAL byte * Buffer_de_ligne_horizontale; // Buffer d'affichage de lignes

  // Données sur l'image actuelle:

GLOBAL byte *    Principal_Ecran;   // Ecran virtuel courant
GLOBAL T_Palette Principal_Palette; // Palette de l'écran en cours

GLOBAL byte  Principal_Image_modifiee; // L'image courante a été modifiée
GLOBAL short Principal_Largeur_image;  // Largeur de l'image dans laquelle l'utilisateur désire travailler
GLOBAL short Principal_Hauteur_image;  // Hauteur de l'image dans laquelle l'utilisateur désire travailler
GLOBAL short Principal_Decalage_X;     // Décalage en X de l'écran par rapport au début de l'image
GLOBAL short Principal_Decalage_Y;     // Décalage en Y de l'écran par rapport au début de l'image
GLOBAL short Ancien_Principal_Decalage_X;
GLOBAL short Ancien_Principal_Decalage_Y;

#ifdef __WATCOMC__
    GLOBAL char Principal_Drive_fichier[2];
#endif

GLOBAL char  Principal_Repertoire_fichier[1024]; // |_ Nom complet =
GLOBAL char  Principal_Nom_fichier[256];         // |  Repertoire_fichier+"\"+Nom_fichier
GLOBAL byte  Principal_Format_fichier;          // Format auquel il faut lire et écrire le fichier
GLOBAL byte  Principal_Format;               // Format du fileselect
GLOBAL short Principal_File_list_Position; // Début de la partie affichée dans la liste de fichiers
GLOBAL short Principal_File_list_Decalage; // Décalage de la barre de sélection dans le fileselector
GLOBAL char  Principal_Repertoire_courant[1024]; // Répertoire actuel sur disque
GLOBAL char  Principal_Commentaire[TAILLE_COMMENTAIRE+1]; // Commentaire de l'image

GLOBAL short Principal_Split; // Position en X du bord gauche du split de la loupe
GLOBAL short Principal_X_Zoom; // (Menu_Facteur_X) + Position en X du bord droit du split de la loupe
GLOBAL float Principal_Proportion_split; // Proportion de la zone non-zoomée par rapport à l'écran

  // Données sur le brouillon:

GLOBAL byte *    Brouillon_Ecran;   // Ecran virtuel brouillon
GLOBAL T_Palette Brouillon_Palette; // Palette de l'écran de brouillon

GLOBAL byte  Brouillon_Image_modifiee; // Le brouillon a été modifié
GLOBAL short Brouillon_Largeur_image;  // Largeur du brouillon dans laquelle l'utilisateur désire travailler
GLOBAL short Brouillon_Hauteur_image;  // Hauteur du brouillon dans laquelle l'utilisateur désire travailler
GLOBAL short Brouillon_Decalage_X;     // Décalage en X du brouillon par rapport au début de l'image
GLOBAL short Brouillon_Decalage_Y;     // Décalage en Y du brouillon par rapport au début de l'image
GLOBAL short Ancien_Brouillon_Decalage_X;
GLOBAL short Ancien_Brouillon_Decalage_Y;

GLOBAL char  Brouillon_Repertoire_fichier[TAILLE_CHEMIN_FICHIER]; // |_ Nom complet =
GLOBAL char  Brouillon_Nom_fichier[TAILLE_CHEMIN_FICHIER];        // |  Repertoire_fichier+"\"+Nom_fichier
GLOBAL byte  Brouillon_Format_fichier;          // Format auquel il faut lire et écrire le fichier
GLOBAL byte  Brouillon_Format;               // Format du fileselect
GLOBAL short Brouillon_File_list_Position; // Début de la partie affichée dans la liste de fichiers
GLOBAL short Brouillon_File_list_Decalage; // Décalage de la barre de sélection dans le fileselector
GLOBAL char  Brouillon_Repertoire_courant[TAILLE_CHEMIN_FICHIER]; // Répertoire actuel sur disque
GLOBAL char  Brouillon_Commentaire[TAILLE_COMMENTAIRE+1]; // Commentaire de l'image

GLOBAL short Brouillon_Split; // Position en X du bord gauche du split de la loupe
GLOBAL short Brouillon_X_Zoom; // (Menu_Facteur_X) + Position en X du bord droit du split de la loupe
GLOBAL float Brouillon_Proportion_split; // Proportion de la zone non-zoomée par rapport à l'écran

GLOBAL byte  Brouillon_Loupe_Mode;      // On est en mode loupe dans le brouillon
GLOBAL word  Brouillon_Loupe_Facteur;   // Facteur de zoom dans le brouillon
GLOBAL word  Brouillon_Loupe_Hauteur;   // Largeur de la fenêtre de zoom dans le brouillon
GLOBAL word  Brouillon_Loupe_Largeur;   // Hauteur de la fenêtre de zoom dans le brouillon
GLOBAL short Brouillon_Loupe_Decalage_X;// Decalage horizontal de la fenêtre de zoom dans le brouillon
GLOBAL short Brouillon_Loupe_Decalage_Y;// Decalage vertical   de la fenêtre de zoom dans le brouillon

GLOBAL byte Masque_copie_couleurs[256]; // Tableau des couleurs à copier vers le brouillon

  // Sauvegarde de l'image:

GLOBAL byte * Ecran_backup;     // Sauvegarde de l'écran virtuel courant
GLOBAL S_Liste_de_pages * Principal_Backups; // Liste des pages de backup de la page principale
GLOBAL S_Liste_de_pages * Brouillon_Backups; // Liste des pages de backup de la page de brouillon


  // Données sur la brosse:

GLOBAL byte * Brosse;          // Sprite de la brosse
GLOBAL word Brosse_Decalage_X; // Centre horizontal de la brosse
GLOBAL word Brosse_Decalage_Y; // Centre vertical de la brosse
GLOBAL word Brosse_Largeur;    // Largeur de la brosse
GLOBAL word Brosse_Hauteur;    // Hauteur de la brosse

GLOBAL char  Brosse_Repertoire_fichier[TAILLE_CHEMIN_FICHIER];// |
GLOBAL char  Brosse_Nom_fichier[TAILLE_CHEMIN_FICHIER];       // |
GLOBAL byte  Brosse_Format_fichier;                           // |  Infos sur le
GLOBAL byte  Brosse_Format;                                   // |_ sélecteur de
GLOBAL short Brosse_File_list_Position;                       // |  fichiers de la
GLOBAL short Brosse_File_list_Decalage;                       // |  brosse.
GLOBAL char  Brosse_Repertoire_courant[256];                  // |
GLOBAL char  Brosse_Commentaire[TAILLE_COMMENTAIRE+1];        // |

GLOBAL byte  Brosse_Centre_rotation_defini; // |  Infos sur le
GLOBAL short Brosse_Centre_rotation_X;      // |- centre de rotation
GLOBAL short Brosse_Centre_rotation_Y;      // |  de la brosse

  // Données sur le menu

GLOBAL byte  Menu_visible;        // Le menu est actif à l'écran
GLOBAL word  Menu_Ordonnee;       // Ordonnée où commence le menu
GLOBAL word  Menu_Ordonnee_Texte; // Ordonnée où commence le texte dans le menu
GLOBAL word  Menu_Facteur_X;      // Facteur de grossissement du menu en X
GLOBAL word  Menu_Facteur_Y;      // Facteur de grossissement du menu en Y
GLOBAL word  Menu_Taille_couleur; // Taille d'une couleur de la palette du menu


  // Données sur la fenêtre de menu

GLOBAL byte Fenetre; // Nombre de fenetres empilées. 0 si pas de fenetre ouverte.

GLOBAL byte Menu_visible_avant_fenetre;  // Le menu était visible avant d'ouvir une fenêtre
GLOBAL word Menu_Ordonnee_avant_fenetre; // Ordonnée du menu avant d'ouvrir une fenêtre
GLOBAL byte Cacher_pinceau_avant_fenetre;// Le pinceau étatit déjà caché avant l'ouverture de la fenetre?

GLOBAL word Pile_Fenetre_Pos_X[8];   // Position du bord gauche de la fenêtre dans l'écran
#define Fenetre_Pos_X Pile_Fenetre_Pos_X[Fenetre-1]

GLOBAL word Pile_Fenetre_Pos_Y[8];   // Position du bord haut   de la fenêtre dans l'écran
#define Fenetre_Pos_Y Pile_Fenetre_Pos_Y[Fenetre-1]

GLOBAL word Pile_Fenetre_Largeur[8]; // Largeur de la fenêtre
#define Fenetre_Largeur Pile_Fenetre_Largeur[Fenetre-1]

GLOBAL word Pile_Fenetre_Hauteur[8]; // Hauteur de la fenêtre
#define Fenetre_Hauteur Pile_Fenetre_Hauteur[Fenetre-1]

GLOBAL word Pile_Nb_boutons_fenetre[8];
#define Nb_boutons_fenetre Pile_Nb_boutons_fenetre[Fenetre-1]

GLOBAL struct Fenetre_Bouton_normal   * Pile_Fenetre_Liste_boutons_normal[8];
#define Fenetre_Liste_boutons_normal Pile_Fenetre_Liste_boutons_normal[Fenetre-1]

GLOBAL struct Fenetre_Bouton_palette  * Pile_Fenetre_Liste_boutons_palette[8];
#define Fenetre_Liste_boutons_palette Pile_Fenetre_Liste_boutons_palette[Fenetre-1]

GLOBAL struct Fenetre_Bouton_scroller * Pile_Fenetre_Liste_boutons_scroller[8];
#define Fenetre_Liste_boutons_scroller Pile_Fenetre_Liste_boutons_scroller[Fenetre-1]

GLOBAL struct Fenetre_Bouton_special  * Pile_Fenetre_Liste_boutons_special[8];
#define Fenetre_Liste_boutons_special Pile_Fenetre_Liste_boutons_special[Fenetre-1]

GLOBAL int Pile_Fenetre_Attribut1[8];
#define Fenetre_Attribut1 Pile_Fenetre_Attribut1[Fenetre-1]

GLOBAL int Pile_Fenetre_Attribut2[8];
#define Fenetre_Attribut2 Pile_Fenetre_Attribut2[Fenetre-1]




// Définition des boutons ////////////////////////////////////////////////////

GLOBAL struct
{
  // Informations sur l'aspect du bouton (graphisme):
  word            Decalage_X;        // Décalage par rapport à la gauche du menu
  word            Decalage_Y;        // Décalage par rapport au haut du menu
  word            Largeur;           // Largeur du bouton
  word            Hauteur;           // Hauteur du bouton
  byte            Enfonce;           // Le bouton est enfoncé
  byte            Forme;             // Forme du bouton

  // Information sur les clicks de la souris:
  fonction_action Gauche;            // Action déclenchée par un click gauche sur le bouton
  fonction_action Droite;            // Action déclenchée par un click droit  sur le bouton
  word            Raccourci_gauche;  // Raccourci clavier équivalent à un click gauche sur le bouton
  word            Raccourci_droite;  // Raccourci clavier équivalent à un click droit  sur le bouton

  // Informations sur le désenclenchement du bouton géré par le moteur:
  fonction_action Desenclencher;     // Action appelée lors du désenclenchement du bouton
  byte            Famille;           // Ensemble de boutons auquel celui-ci appartient

} Bouton[NB_BOUTONS];



// Informations sur les différents modes de dessin

GLOBAL fonction_effet Fonction_effet;

GLOBAL byte * FX_Feedback_Ecran;

GLOBAL byte Exclude_color[256]; // Couleurs à exclure pour Meilleure_couleur

  // Mode smear:

GLOBAL byte  Smear_Mode;    // Le mode smear est enclenché
GLOBAL byte  Smear_Debut;   // On vient juste de commencer une opération en Smear
GLOBAL byte * Smear_Brosse; // Sprite de la brosse de Smear
GLOBAL word  Smear_Brosse_Largeur; // Largeur de la brosse de Smear
GLOBAL word  Smear_Brosse_Hauteur; // Hauteur de la brosse de Smear
GLOBAL short Smear_Min_X,Smear_Max_X,Smear_Min_Y,Smear_Max_Y; // Bornes de la Brosse du smear

  // Mode shade:

GLOBAL struct T_Shade Shade_Liste[8]; // Listes de shade
GLOBAL byte           Shade_Actuel;   // Numéro du shade en cours
GLOBAL byte *         Shade_Table;    // Table de conversion de shade en cours
GLOBAL byte           Shade_Table_gauche[256]; // Table de conversion de shade pour un clic gauche
GLOBAL byte           Shade_Table_droite[256]; // Table de conversion de shade pour un clic droit
GLOBAL byte           Shade_Mode;     // Le mode shade est enclenché

GLOBAL byte           Quick_shade_Mode; // Le mode quick-shade est enclenché
GLOBAL byte           Quick_shade_Step; // Pas du mode quick-shade
GLOBAL byte           Quick_shade_Loop; // Normal / Loop / No sat.

  // Mode stencil:

GLOBAL byte Stencil_Mode;  // Le mode stencil est enclenché
GLOBAL byte Stencil[256];  // Tableau des couleurs protégées

  // Mode grille:

GLOBAL byte  Snap_Mode;       // Le mode grille est enclenché
GLOBAL short Snap_Largeur;    // Largeur entre 2 points de la grille
GLOBAL short Snap_Hauteur;    // Hauteur entre 2 points de la grille
GLOBAL short Snap_Decalage_X; // Position en X du point le + à gauche
GLOBAL short Snap_Decalage_Y; // Position en Y du point le + en haut

  // Mode trame:

GLOBAL byte  Trame_Mode;    // Le mode Trame est enclenché
GLOBAL byte  Trame[16][16]; // Sprite de la trame
GLOBAL word  TRAME_PREDEFINIE[12][16]; // Trames préséfinies (compactées sur 16*16 bits)
GLOBAL short Trame_Largeur; // Largeur de la trame
GLOBAL short Trame_Hauteur; // Hauteur de la trame

  // Mode colorize:

GLOBAL byte Colorize_Mode;          // Le mode Colorize est enclenché
GLOBAL byte Colorize_Opacite;       // Intensité du Colorize
GLOBAL byte Colorize_Mode_en_cours; // Le type de Colorize en cours (0-2)
GLOBAL word Table_de_multiplication_par_Facteur_A[64];
GLOBAL word Table_de_multiplication_par_Facteur_B[64];

  // Mode smooth:

GLOBAL byte Smooth_Mode;          // Le mode Smooth est enclenché
GLOBAL byte Smooth_Matrice[3][3]; // La matrice du Smooth actuel

  // Mode Tiling:

GLOBAL byte  Tiling_Mode;       // Le mode Tiling est enclenché
GLOBAL short Tiling_Decalage_X; // Décalage du tiling en X
GLOBAL short Tiling_Decalage_Y; // Décalage du tiling en Y

  // Mode Mask

GLOBAL byte Mask_Mode;  // Le mode Masque est enclenché
GLOBAL byte Mask[256];  // Tableau des couleurs constituant le masque

  // Mode loupe:

GLOBAL byte  Loupe_Mode;
GLOBAL word  Loupe_Facteur;
GLOBAL word  Loupe_Hauteur;
GLOBAL word  Loupe_Largeur;
GLOBAL short Loupe_Decalage_X;
GLOBAL short Loupe_Decalage_Y;
GLOBAL word  * Table_mul_facteur_zoom;
GLOBAL word  TABLE_ZOOM[NB_FACTEURS_DE_ZOOM][512];

#ifdef VARIABLES_GLOBALES
  word FACTEUR_ZOOM[NB_FACTEURS_DE_ZOOM]={2,3,4,5,6,8,10,12,14,16,18,20};
#else
  extern word FACTEUR_ZOOM[NB_FACTEURS_DE_ZOOM];
#endif

  // Données sur les ellipses et les cercles:

GLOBAL long  Ellipse_Curseur_X;
GLOBAL long  Ellipse_Curseur_Y;
GLOBAL long  Ellipse_Rayon_vertical_au_carre;
GLOBAL long  Ellipse_Rayon_horizontal_au_carre;
//GLOBAL long  Ellipse_Limite_High;
//GLOBAL long  Ellipse_Limite_Low;
GLOBAL uint64_t Ellipse_Limite;
GLOBAL long  Cercle_Curseur_X;
GLOBAL long  Cercle_Curseur_Y;
GLOBAL long  Cercle_Limite;

  // Données sur les dégradés:

GLOBAL short Degrade_Borne_Inferieure;   // Plus petite couleur englobée par le dégradé
GLOBAL short Degrade_Borne_Superieure;   // Plus grande couleur englobée par le dégradé
GLOBAL int   Degrade_Inverse;            // Booléen "Le dégradé est en réalité inversé"
GLOBAL long  Degrade_Intervalle_bornes;  // = Abs(Degrade_Borne_Inferieure-Degrade_Borne_Superieure)+1
GLOBAL long  Degrade_Intervalle_total;   // Valeur maximum des indices passés à la fonction de dégradé (!!! >0 !!!)
GLOBAL long  Degrade_Melange_aleatoire;  // Facteur de mélange (1-256+) du dégradé
GLOBAL fonction_degrade Traiter_degrade; // Fonction de traitement du dégradé, varie selon la méthode choisie par l'utilisateur.
GLOBAL fonction_afficheur Traiter_pixel_de_degrade; // Redirection de l'affichage

GLOBAL struct T_Degrade_Tableau Degrade_Tableau[16]; // Données de tous les dégradés
GLOBAL byte Degrade_Courant;             // Indice du tableau correspondant au dégradé courant



  // Données sur le Spray:

GLOBAL byte  Spray_Mode;            // Mode Mono(1) ou Multicolore(0)
GLOBAL short Spray_Size;            // Diamètre du spray en pixels
GLOBAL byte  Spray_Delay;           // Délai en VBLs entre 2 "pschiitt"
GLOBAL byte  Spray_Mono_flow;       // Nombre de pixels qui sortent en même temps en mono
GLOBAL byte  Spray_Multi_flow[256]; // Idem pour chaque couleur


  // Données diverses sur le programme:

GLOBAL byte Sortir_du_programme;
GLOBAL char Repertoire_du_programme[256]; // Répertoire dans lequel se trouve le programme
GLOBAL char Repertoire_initial[256];      // Répertoire à partir duquel à été lancé le programme
GLOBAL byte Fore_color;
GLOBAL byte Back_color;
GLOBAL byte Mode_de_dessin_en_cours;
GLOBAL byte Courbe_en_cours;
GLOBAL byte Ligne_en_cours;
GLOBAL byte Couleur_debut_palette;
GLOBAL byte Un_fichier_a_ete_passe_en_parametre;
GLOBAL byte Une_resolution_a_ete_passee_en_parametre; // utilisée uniquement si la variable précédente est à 1

  // Variables concernant l'OBJ DIVERS

GLOBAL word INPUT_Nouveau_Mouse_X;
GLOBAL word INPUT_Nouveau_Mouse_Y;
GLOBAL byte INPUT_Nouveau_Mouse_K;
GLOBAL byte INPUT_Keyb_mode;

GLOBAL int MC_Indice;
GLOBAL int MC_DR;
GLOBAL int MC_DV;
GLOBAL int MC_DB;
GLOBAL int * MC_Table_differences;

  // Variables concernant l'OBJ VIDEO

    // Partie concernant le mode X:
GLOBAL dword MODE_X_Decalage_synchro;
GLOBAL word  MODE_X_Largeur_de_ligne;
GLOBAL dword MODE_X_Valeur_initiale_de_esi;
GLOBAL dword MODE_X_Valeur_initiale_de_edi;
    // Partie concernant le VESA:
GLOBAL byte   Granularite; // Facteur de gestion de la granularité de la carte
GLOBAL byte   VESA_Erreur;
GLOBAL byte * VESA_WinFuncPtr; // Handler software de changement de banque
GLOBAL word * VESA_Liste_des_modes;
GLOBAL dword  VESA_Decalage_synchro;
GLOBAL word   VESA_Largeur_ecran_en_dword;
GLOBAL byte   VESA_Banque_en_cours; // Variable normalement locale à VIDEO.ASM
GLOBAL byte   VESA_Derniere_banque_Fenetre_A_utilisee;
GLOBAL byte   VESA_Derniere_banque_Fenetre_B_utilisee;
GLOBAL fonction_action VESA_Change_banque_lecture;
GLOBAL fonction_action VESA_Change_banque_ecriture;
GLOBAL fonction_action VESA_Change_banque_lect_ecr;
GLOBAL byte VESA_Version_Unite;
GLOBAL byte VESA_Version_Decimale;
GLOBAL char VESA_Constructeur[TAILLE_NOM_CONSTRUCTEUR+1];
GLOBAL word VESA_Taille_memoire;

  // Les différents sprites:

GLOBAL byte BLOCK_MENU[HAUTEUR_MENU][LARGEUR_MENU];
GLOBAL byte SPRITE_MENU[NB_SPRITES_MENU][HAUTEUR_SPRITE_MENU][LARGEUR_SPRITE_MENU];
GLOBAL byte SPRITE_EFFET[NB_SPRITES_EFFETS][HAUTEUR_SPRITE_MENU][LARGEUR_SPRITE_MENU];

GLOBAL byte * Logo_GrafX2;

GLOBAL byte Fonte_systeme[256*8*8];
GLOBAL byte Fonte_fun    [256*8*8];
GLOBAL byte Fonte_help   [315][6][8];
GLOBAL byte * Fonte;

  // Les données de l'aide:

GLOBAL byte Section_d_aide_en_cours;  // Indice de la table d'aide en cours de consultation
GLOBAL word Position_d_aide_en_cours; // Numéro de la ligne d'aide en cours de consultation

  // Données sur les opérations

GLOBAL word Operation_avant_interruption; // N° de l'opération en cours avant l'utilisation d'une interruption
GLOBAL word Operation_en_cours;           // N° de l'opération en cours
GLOBAL word Operation_Pile[TAILLE_PILE_OPERATIONS]; // Pile simplifiée
GLOBAL byte Operation_Taille_pile;    // Taille effective de la pile (0=vide)
GLOBAL byte Operation_dans_loupe;     // Indique si l'opération a commencé dans la partie Zoomée ou non

GLOBAL short Pipette_Couleur;
GLOBAL short Pipette_X;
GLOBAL short Pipette_Y;


#ifdef VARIABLES_GLOBALES
  byte CURSEUR_D_OPERATION[NB_OPERATIONS]=
  {
    FORME_CURSEUR_CIBLE            , // Dessin à la main continu
    FORME_CURSEUR_CIBLE            , // Dessin à la main discontinu
    FORME_CURSEUR_CIBLE            , // Dessin à la main point par point
    FORME_CURSEUR_CIBLE            , // Lignes
    FORME_CURSEUR_CIBLE            , // Lignes reliées
    FORME_CURSEUR_CIBLE            , // Lignes centrées
    FORME_CURSEUR_CIBLE_XOR        , // Rectangle vide
    FORME_CURSEUR_CIBLE_XOR        , // Rectangle plein
    FORME_CURSEUR_CIBLE            , // Cercles vides
    FORME_CURSEUR_CIBLE            , // Cercles pleins
    FORME_CURSEUR_CIBLE            , // Ellipses vides
    FORME_CURSEUR_CIBLE            , // Ellipses pleines
    FORME_CURSEUR_CIBLE            , // Fill
    FORME_CURSEUR_CIBLE            , // Remplacer
    FORME_CURSEUR_CIBLE_XOR        , // Prise de brosse rectangulaire
    FORME_CURSEUR_CIBLE            , // Prise d'une brosse multiforme
    FORME_CURSEUR_CIBLE_PIPETTE    , // Récupération d'une couleur
    FORME_CURSEUR_RECTANGLE_XOR    , // Positionnement de la fenêtre de loupe
    FORME_CURSEUR_CIBLE            , // Courbe à 3 points
    FORME_CURSEUR_CIBLE            , // Courbe à 4 points
    FORME_CURSEUR_CIBLE            , // Spray
    FORME_CURSEUR_CIBLE            , // Polygone
    FORME_CURSEUR_CIBLE            , // Polyforme
    FORME_CURSEUR_CIBLE            , // Polyfill
    FORME_CURSEUR_CIBLE            , // Polyforme rempli
    FORME_CURSEUR_MULTIDIRECTIONNEL, // Scroll
    FORME_CURSEUR_CIBLE            , // Cercles dégradés
    FORME_CURSEUR_CIBLE            , // Ellipses dégradées
    FORME_CURSEUR_ROTATE_XOR       , // Faire tourner brosse
    FORME_CURSEUR_CIBLE_XOR        , // Etirer brosse
    FORME_CURSEUR_CIBLE              // Deformer brosse
  };
#else
  extern byte CURSEUR_D_OPERATION[NB_OPERATIONS];
#endif


  // Procédures à appeler: Opération,Mouse_K,Etat de la pile

GLOBAL struct
{
  byte Effacer_curseur; // Booléen "il faut effacer le curseur pour l'opéra."
  fonction_action Action;                                   // Action appelée
} Operation[NB_OPERATIONS][3][TAILLE_PILE_OPERATIONS];



// Informations sur les lecteurs

GLOBAL byte Nb_drives;
GLOBAL struct T_Drive Drive[26];
GLOBAL byte SPRITE_DRIVE[NB_SPRITES_DRIVES][HAUTEUR_SPRITE_DRIVE][LARGEUR_SPRITE_DRIVE];


// -- Section des informations sur les formats de fichiers ------------------

  // Comptage du nb d'éléments dans la liste:
GLOBAL short Liste_Nb_elements;
GLOBAL short Liste_Nb_fichiers;
GLOBAL short Liste_Nb_repertoires;
  // Tête de la liste chaînée:
GLOBAL struct Element_de_liste_de_fileselect * Liste_du_fileselect;

// ------------------- Inititialisation des formats connus -------------------

#include "loadsave.h"
void Rien_du_tout(void);

#ifdef VARIABLES_GLOBALES
  // Extension du format
  char Format_Extension[NB_FORMATS_CONNUS][4]=
  {
    "pkm", // PKM
    "lbm", // LBM
    "gif", // GIF
    "bmp", // BMP
    "pcx", // PCX
    "img", // IMG
    "sc?", // SCx
    "pi1", // PI1
    "pc1", // PC1
    "cel", // CEL
    "kcf", // KCF
    "pal"  // PAL
  };

  // Fonction à appeler pour vérifier la signature du fichier
  fonction_action Format_Test[NB_FORMATS_LOAD]=
  {
    Test_PKM, // PKM
    Test_LBM, // LBM
    Test_GIF, // GIF
    Test_BMP, // BMP
    Test_PCX, // PCX
    Test_IMG, // IMG
    Test_SCx, // SCx
    Test_PI1, // PI1
    Test_PC1, // PC1
    Test_CEL, // CEL
    Test_KCF, // KCF
    Test_PAL  // PAL
  };

  // Fonction à appeler pour charger l'image
  fonction_action Format_Load[NB_FORMATS_LOAD]=
  {
    Load_PKM, // PKM
    Load_LBM, // LBM
    Load_GIF, // GIF
    Load_BMP, // BMP
    Load_PCX, // PCX
    Load_IMG, // IMG
    Load_SCx, // SCx
    Load_PI1, // PI1
    Load_PC1, // PC1
    Load_CEL, // CEL
    Load_KCF, // KCF
    Load_PAL  // PAL
  };

  // Fonction à appeler pour sauvegarder l'image
  fonction_action Format_Save[NB_FORMATS_SAVE]=
  {
    Save_PKM, // PKM
    Save_LBM, // LBM
    Save_GIF, // GIF
    Save_BMP, // BMP
    Save_PCX, // PCX
    Save_IMG, // IMG
    Save_SCx, // SCx
    Save_PI1, // PI1
    Save_PC1, // PC1
    Save_CEL, // CEL
    Save_KCF, // KCF
    Save_PAL  // PAL
  };

  // indique si l'on doit considérer que l'image n'est plus modifiée
  byte Format_Backup_done[NB_FORMATS_CONNUS]=
  {
    1, // PKM
    1, // LBM
    1, // GIF
    1, // BMP
    1, // PCX
    1, // IMG
    1, // SCx
    1, // PI1
    1, // PC1
    1, // CEL
    0, // KCF
    0  // PAL
  };

  // Le format de fichier autorise un commentaire
  byte Format_Commentaire[NB_FORMATS_CONNUS]=
  {
    1, // PKM
    0, // LBM
    0, // GIF
    0, // BMP
    0, // PCX
    0, // IMG
    0, // SCx
    0, // PI1
    0, // PC1
    0, // CEL
    0, // KCF
    0  // PAL
  };
#else
  extern char Format_Extension[NB_FORMATS_CONNUS][4];
  extern fonction_action Format_Load[NB_FORMATS_LOAD];
  extern fonction_action Format_Save[NB_FORMATS_SAVE];
  extern fonction_action Format_Test[NB_FORMATS_LOAD];
  extern byte Format_Backup_done[NB_FORMATS_CONNUS];
  extern byte Format_Commentaire[NB_FORMATS_CONNUS];
#endif

GLOBAL signed char Erreur_fichier; // 0: opération I/O OK
                                   // 1: Erreur dès le début de l'opération
                                   // 2: Erreur durant l'opération => données modifiées
                                   //-1: Interruption du chargement d'une preview

GLOBAL int Ligne_INI;

GLOBAL fonction_afficheur Pixel_de_chargement;
GLOBAL fonction_lecteur   Lit_pixel_de_sauvegarde;

/********************
 * Spécifique à SDL *
 ********************/
GLOBAL SDL_Surface * Ecran_SDL;
#endif
