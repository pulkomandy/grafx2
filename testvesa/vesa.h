
#ifndef _VESA_H_
#define _VESA_H_



struct S_Buffer_VESA
{
  // VESA 1.0
  char   Signature[4];    // = 'VESA'
  word   Version;         // Nø de version
  char * Fabricant;       // Nom du fabricant
  dword  Capacite;        // Bits de capacit‚ du DAC
  word * Liste_des_modes; // Liste des modes vid‚os support‚s par le VESA
  word   Memoire;         // Taille de la m‚moire en blocs de 64K

  // VESA 2.0
  word   Version_bios;    // Version du BIOS du fabricant
  char * Vendeur;         // Nom du vendeur
  char * Produit;         // Nom du produit
  char * Revision;        // Nom de la r‚vision

  // Filler
  byte   Filler[990];     // Reserv‚
};



struct S_Buffer_mode_VESA
{
  // VESA 1.0
  word Attributs;           // Attributs du mode              (00h)
  byte Attributs_fenetre_A; // Attributs de la fenˆtre A      (02h)
  byte Attributs_fenetre_B; // Attributs de la fenˆtre B      (03h)
  word Granularite;         // Granularit‚ des fenˆtres en Ko (04h)
  word Taille_fenetres;     // Taille des fenˆtres en Ko      (06h)
  word Segment_fenetre_A;   // Segment de la fenˆtre A        (08h)
  word Segment_fenetre_B;   // Segment de la fenˆtre B        (0Ah)
  byte * WinFuncPtr;        // Fonction d'AX=4F05 en mode r‚el(0Ch)
  word Octets_par_ligne;    // Nombre d'octets par ligne      (10h)

  // Optionnels ou OEM
  word Largeur;             // Largeur en pixels              (12h)
  word Hauteur;             // Hauteur en pixels              (14h)
  byte Largeur_de_char;     // Largeur des caractŠres en pxls (16h)
  byte Hauteur_de_char;     // Hauteur des caractŠres en pxls (17h)
  byte Nb_plans;            // Nombre de plans de m‚moire     (18h)
  byte Nb_bits_par_pixel;   // Nombre de bits par pixel       (19h)
  byte Nb_banques;          // Nombre de banques              (1Ah)
  byte Modele_de_memoire;   // ModŠle de m‚moire              (1Bh)
  byte Taille_des_banques;  // Taille des banques en Ko       (1Ch)
  byte Nombre_de_pages;     // Nombre de pages d'image        (1Dh)
  byte Reserve;             // Reserv‚ (=1)                   (1Eh)

  // VESA 1.2
  byte Taille_masque_rouge; // Taille du masque des rouges    (1Fh)
  byte Pos_masque_rouge;    // Position du masque des rouges  (20h)
  byte Taille_masque_vert;  // Taille du masque des verts     (21h)
  byte Pos_masque_vert;     // Position du masque des verts   (22h)
  byte Taille_masque_bleu;  // Taille du masque des bleus     (23h)
  byte Pos_masque_bleu;     // Position du masque des bleus   (24h)
  byte Taille_masque_res;   // Taille d'un masque reserv‚     (25h)
  byte Pos_masque_res;      // Position d'un masque reserv‚   (26h)
  byte Direct_screen_mode;  // Direct screen mode info        (27h)

  // VESA 2.0
  byte * Adresse_LFB;       // Adresse du LFB                 (28h)
  byte * Adresse_offscreen; // Pointeur vers le d‚but de la m‚moire offscreen (2Ch)
  word Taille_offscreen;    // Taille de la m‚moire offscreen en Ko (30h)

  // Filler
  byte Filler[206];
};



word Get_VESA_info     (           struct S_Buffer_VESA      * Buffer_VESA);
word Get_VESA_mode_info(word Mode, struct S_Buffer_mode_VESA * Buffer_mode_VESA);



#endif
