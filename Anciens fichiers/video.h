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
#include "struct.h"

// -- Headers des fonctions MCGA --

void Set_mode_MCGA             (void);
void Pixel_MCGA                (word X,word Y,byte Couleur);
byte Lit_pixel_MCGA            (word X,word Y);
void Effacer_tout_l_ecran_MCGA (byte Couleur);
void Block_MCGA                (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);

void Pixel_Preview_Normal_MCGA (word X,word Y,byte Couleur);
void Pixel_Preview_Loupe_MCGA  (word X,word Y,byte Couleur);
void Ligne_horizontale_XOR_MCGA(word Pos_X,word Pos_Y,word Largeur);
void Ligne_verticale_XOR_MCGA  (word Pos_X,word Pos_Y,word Hauteur);

void Display_brush_Color_MCGA  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);
void Display_brush_Mono_MCGA   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
void Clear_brush_MCGA          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_image);
void Remap_screen_MCGA         (word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion);


// -- Headers des fonctions Mode X --

void Set_mode_X                  (void);
void Pixel_mode_X                (word X,word Y,byte Couleur);
byte Lit_pixel_mode_X            (word X,word Y);
void Effacer_tout_l_ecran_mode_X (byte Couleur);
void Block_mode_X                (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);

void Pixel_Preview_Normal_mode_X (word X,word Y,byte Couleur);
void Pixel_Preview_Loupe_mode_X  (word X,word Y,byte Couleur);
void Ligne_horizontale_XOR_mode_X(word Pos_X,word Pos_Y,word Largeur);
void Ligne_verticale_XOR_mode_X  (word Pos_X,word Pos_Y,word Hauteur);

void Display_brush_Color_mode_X(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);
void Display_brush_Mono_mode_X (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
void Clear_brush_mode_X        (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_image);
void Remap_screen_mode_X       (word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion);

// -- Headers des fonctions VESA --

void VESA_Change_banque_Fenetre_A(void);
void VESA_Change_banque_Fenetre_B(void);
void VESA_Change_banque_Fenetre_A_et_B(void);
void VESA_Change_banque_Fenetre_A_FAST(void);
void VESA_Change_banque_Fenetre_B_FAST(void);
void VESA_Change_banque_Fenetre_A_et_B_FAST(void);
//void Support_VESA(void);
//void Mode_VESA_supporte(word Mode);
byte Initialiser_mode_video_VESA(word Mode);
void Retoucher_CRTC(void);
void Pixel_VESA(word X,word Y,byte Couleur);
byte Lit_pixel_VESA(word X,word Y);
void Effacer_tout_l_ecran_VESA(byte Couleur);
void Block_VESA(word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);

void Pixel_Preview_Normal_VESA (word X,word Y,byte Couleur);
void Pixel_Preview_Loupe_VESA  (word X,word Y,byte Couleur);
void Ligne_horizontale_XOR_VESA(word Pos_X,word Pos_Y,word Largeur);
void Ligne_verticale_XOR_VESA  (word Pos_X,word Pos_Y,word Hauteur);

void Display_brush_Color_VESA  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);
void Display_brush_Mono_VESA   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
void Clear_brush_VESA          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_image);
void Remap_screen_VESA         (word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion);

// -- Nouveaux trucs --

void Afficher_partie_de_l_ecran_MCGA         (word Largeur,word Hauteur,word Largeur_image);
void Afficher_une_ligne_a_l_ecran_MCGA       (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Lire_une_ligne_a_l_ecran_MCGA           (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Afficher_partie_de_l_ecran_zoomee_MCGA  (word Largeur,word Hauteur,word Largeur_image,byte * Buffer);
void Afficher_partie_de_l_ecran_mode_X       (word Largeur,word Hauteur,word Largeur_image);
void Afficher_une_ligne_a_l_ecran_mode_X     (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Lire_une_ligne_a_l_ecran_mode_X         (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Afficher_partie_de_l_ecran_zoomee_mode_X(word Largeur,word Hauteur,word Largeur_image,byte * Buffer);
void Afficher_partie_de_l_ecran_VESA         (word Largeur,word Hauteur,word Largeur_image);
void Afficher_une_ligne_a_l_ecran_VESA       (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Lire_une_ligne_a_l_ecran_VESA           (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
void Afficher_partie_de_l_ecran_zoomee_VESA  (word Largeur,word Hauteur,word Largeur_image,byte * Buffer);

void Display_brush_Color_Zoom_MCGA  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
void Display_brush_Mono_Zoom_MCGA   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
void Clear_brush_Zoom_MCGA          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_image,byte * Buffer);
void Display_brush_Color_Zoom_mode_X(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
void Display_brush_Mono_Zoom_mode_X (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
void Clear_brush_Zoom_mode_X        (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_image,byte * Buffer);
void Display_brush_Color_Zoom_VESA  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
void Display_brush_Mono_Zoom_VESA   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
void Clear_brush_Zoom_VESA          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_image,byte * Buffer);
