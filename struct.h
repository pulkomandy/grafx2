#ifndef _STRUCT_H_
#define _STRUCT_H_

#include "const.h"

// Déclaration des types de base /////////////////////////////////////////////

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

typedef void (* fonction_action)    (void);
typedef void (* fonction_afficheur) (word,word,byte);
typedef byte (* fonction_lecteur)   (word,word);
typedef void (* fonction_effaceur)  (byte);
typedef void (* fonction_display)   (word,word,word);
typedef byte (* fonction_effet)     (word,word,byte);
typedef void (* fonction_block)     (word,word,word,word,byte);
typedef byte (* fonction_test_sign) (void);
typedef void (* fonction_Ligne_XOR) (word,word,word);
typedef void (* fonction_display_brush_Color) (word,word,word,word,word,word,byte,word);
typedef void (* fonction_display_brush_Mono)  (word,word,word,word,word,word,byte,byte,word);
typedef void (* fonction_degrade)   (long,short,short);
typedef void (* fonction_remap)     (word,word,word,word,byte *);
typedef void (* fonction_procsline) (word,word,word,byte *);
typedef void (* fonction_display_zoom) (word,word,word,byte *);
typedef void (* fonction_display_brush_Color_zoom) (word,word,word,word,word,word,byte,word,byte *);
typedef void (* fonction_display_brush_Mono_zoom)  (word,word,word,word,word,word,byte,byte,word,byte *);

struct Composantes
{
  byte R;
  byte V;
  byte B;
};
typedef struct Composantes T_Palette[256];



struct Fenetre_Bouton_normal
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Largeur;
  word Hauteur;
  byte Clickable;
  word Raccourci;
  struct Fenetre_Bouton_normal * Next;
};

struct Fenetre_Bouton_palette
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  struct Fenetre_Bouton_palette * Next;
};

struct Fenetre_Bouton_scroller
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Hauteur;
  word Nb_elements;
  word Nb_visibles;
  word Position;
  word Hauteur_curseur;
  struct Fenetre_Bouton_scroller * Next;
};

struct Fenetre_Bouton_special
{
  short Numero;
  word Pos_X;
  word Pos_Y;
  word Largeur;
  word Hauteur;
  struct Fenetre_Bouton_special * Next;
};


struct T_Drive
{
  char Lettre;
  byte Type; // 0: Diskette 3"« / 1: Diskette 5"¬ / 2: HDD / 3: CD-ROM / 4: Logique
  word Touche;
};


// Déclaration du type d'élément qu'on va mémoriser dans la liste:
struct Element_de_liste_de_fileselect
{
  char Nom[13]; // Le nom du fichier ou du répertoire
  byte Type;    // Type d'élément : 0 = Fichier, 1 = Répertoire

  // données de chaŒnage de la liste
  struct Element_de_liste_de_fileselect * Suivant;
  struct Element_de_liste_de_fileselect * Precedent;
};



// Déclaration d'une section d'aide:
struct Section_d_aide
{
  word Nombre_de_lignes;
  byte * Debut_de_la_liste; // PremiŠre ligne de la liste
};

// Déclaration d'une info sur un dégradé
struct T_Degrade_Tableau
{
  byte Debut;     // PremiŠre couleur du dégradé
  byte Fin;       // DerniŠre couleur du dégradé
  int  Inverse;   // "Le dégradé va de Fin … Debut"
  long Melange;   // Valeur de mélange du dégradé (0-255)
  int  Technique; // Technique … utiliser (0-2)
};

// Déclaration d'une info de shade
struct T_Shade
{
  word Liste[512]; // Liste de couleurs
  byte Pas;        // Pas d'incrémentation/décrémentation
  byte Mode;       // Mode d'utilisation (Normal/Boucle/Non-saturé)
};



// Structure des données dans le fichier de config.

struct Config_Mode_video
{
  byte Etat;
  word Largeur;
  word Hauteur;
};

struct Config_Header
{
  char Signature[3];
  byte Version1;
  byte Version2;
  byte Beta1;
  byte Beta2;
};

struct Config_Chunk
{
  byte Numero;
  word Taille;
};

struct Config_Infos_touche
{
  word Numero;
  word Touche;
  word Touche2;
};



// Structures utilisées pour les descriptions de pages et de liste de pages.
// Lorsqu'on gŠrera les animations, il faudra aussi des listes de listes de
// pages.

// Ces structures sont manipulées … travers des fonctions de gestion du
// backup dans "graph.c".

typedef struct
{
  byte *    Image;   // Bitmap de l'image
  int       Largeur; // Largeur du bitmap
  int       Hauteur; // Hauteur du bitmap
  T_Palette Palette; // Palette de l'image

  char      Commentaire[TAILLE_COMMENTAIRE+1]; // Commentaire de l'image

  char      Repertoire_fichier[256]; // |_ Nom complet =
  char      Nom_fichier[13];         // |  Repertoire_fichier+"\"+Nom_fichier
  byte      Format_fichier;          // Format auquel il faut lire et écrire le fichier

/*
  short     Decalage_X; // Décalage en X de l'écran par rapport au début de l'image
  short     Decalage_Y; // Décalage en Y de l'écran par rapport au début de l'image
  short     Ancien_Decalage_X; // Le mˆme avant le passage en mode loupe
  short     Ancien_Decalage_Y; // Le mˆme avant le passage en mode loupe

  short     Split; // Position en X du bord gauche du split de la loupe
  short     X_Zoom; // (Menu_Facteur_X) + Position en X du bord droit du split de la loupe
  float     Proportion_split; // Proportion de la zone non-zoomée par rapport … l'écran

  byte      Loupe_Mode;       // On est en mode loupe
  word      Loupe_Facteur;    // Facteur de zoom
  word      Loupe_Hauteur;    // Largeur de la fenˆtre de zoom
  word      Loupe_Largeur;    // Hauteur de la fenˆtre de zoom
  short     Loupe_Decalage_X; // Decalage horizontal de la fenˆtre de zoom
  short     Loupe_Decalage_Y; // Decalage vertical   de la fenˆtre de zoom
*/
} S_Page;

typedef struct
{
  int      Taille_liste;      // Nb de S_Page dans le vecteur "Pages"
  int      Nb_pages_allouees; // Nb de S_Page désignant des pages allouées
  S_Page * Pages;             // Liste de pages (Taille_liste éléments)
} S_Liste_de_pages;



#endif
