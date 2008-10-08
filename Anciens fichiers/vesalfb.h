/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

#ifndef _VESALFB_H_
#define _VESALFB_H_

#include "struct.h"
#include "global.h"

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
  byte Largeur_de_char;     // Largeur des caractères en pxls (16h)
  byte Hauteur_de_char;     // Hauteur des caractères en pxls (17h)
  byte Nb_plans;            // Nombre de plans de m‚moire     (18h)
  byte Nb_bits_par_pixel;   // Nombre de bits par pixel       (19h)
  byte Nb_banques;          // Nombre de banques              (1Ah)
  byte Modele_de_memoire;   // Modèle de m‚moire              (1Bh)
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



  extern word LFB_Selecteur; // S‚lecteur utilis‚ par le mapping du LFB
  extern word LFB_Adresse;   // Adresse o— est mapp‚e le LFB
  extern word LFB_Taille;    // Taille de la m‚moire LFB



  word Get_VESA_info     (           struct S_Buffer_VESA      * Buffer_VESA);
  word Get_VESA_mode_info(word Mode, struct S_Buffer_mode_VESA * Buffer_mode_VESA);
  void Get_VESA_protected_mode_WinFuncPtr(void);
  word Set_VESA_mode(word Mode);
  word Initialiser_le_LFB(byte * Adresse_physique,dword Taille);
  word Fermer_le_LFB(void);

  void Pixel_VESA_LFB                (word X,word Y,byte Couleur);
  byte Lit_pixel_VESA_LFB            (word X,word Y);
  void Effacer_tout_l_ecran_VESA_LFB (byte Couleur);
  void Block_VESA_LFB                (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);
  void Pixel_Preview_Normal_VESA_LFB (word X,word Y,byte Couleur);
  void Pixel_Preview_Loupe_VESA_LFB  (word X,word Y,byte Couleur);
  void Ligne_horizontale_XOR_VESA_LFB(word Pos_X,word Pos_Y,word Largeur);
  void Ligne_verticale_XOR_VESA_LFB  (word Pos_X,word Pos_Y,word Hauteur);
  void Display_brush_Color_VESA_LFB  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);
  void Display_brush_Mono_VESA_LFB   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
  void Clear_brush_VESA_LFB          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_image);
  void Remap_screen_VESA_LFB         (word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion);
  void Afficher_partie_de_l_ecran_VESA_LFB       (word Largeur,word Hauteur,word Largeur_image);
  void Afficher_une_ligne_a_l_ecran_VESA_LFB     (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
  void Lire_une_ligne_a_l_ecran_VESA_LFB         (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
  void Afficher_partie_de_l_ecran_zoomee_VESA_LFB(word Largeur,word Hauteur,word Largeur_image,byte * Buffer);
  void Display_brush_Color_Zoom_VESA_LFB(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
  void Display_brush_Mono_Zoom_VESA_LFB (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
  void Clear_brush_Zoom_VESA_LFB        (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_image,byte * Buffer);



#endif
