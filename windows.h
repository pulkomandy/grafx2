/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
*/

#ifndef __WINDOWS_H_
#define __WINDOWS_H_

#include "struct.h"

void Afficher_curseur(void);
void Effacer_curseur(void);

void Remapper_ecran_apres_changement_couleurs_menu(void);
void Calculer_couleurs_menu_optimales(Composantes * Palette);

void Recadrer_ecran_par_rapport_au_zoom(void);
void Calculer_split(void);
void Calculer_donnees_loupe(void);
void Calculer_limites(void);
void Calculer_coordonnees_pinceau(void);

void Pixel_dans_barre_d_outil(word X,word Y,byte Couleur);
void Pixel_dans_fenetre(word X,word Y,byte Couleur);
void Encadrer_couleur_menu(byte Couleur);
void Afficher_palette_du_menu(void);
void Afficher_menu(void);
void Recadrer_palette(void);
void Changer_cellules_palette(void);
int Couleur_palette(void);
word Palette_Cells_X(void);
word Palette_Cells_Y(void);

void Print_general(short X,short Y,const char * Chaine,byte Couleur_texte,byte Couleur_fond);
void Print_dans_fenetre(short X,short Y,const char * Chaine,byte Couleur_texte,byte Couleur_fond);
void Print_dans_fenetre_limite(short X,short Y,const char * Chaine,byte Taille,byte Couleur_texte,byte Couleur_fond);
void Print_char_dans_fenetre(short Pos_X,short Pos_Y,const unsigned char c,byte Couleur_texte,byte Couleur_fond);
void Print_char_transparent_dans_fenetre(short Pos_X,short Pos_Y,const unsigned char c,byte Couleur);
void Print_dans_menu(const char * Chaine, short Position);
void Print_coordonnees(void);
void Print_nom_fichier(void);
void Print_compteur(short X,short Y,const char * Chaine,byte Couleur_texte,byte Couleur_fond);

byte Demande_de_confirmation(char * Message);
void Warning_message(char * Message);

void Afficher_limites_de_l_image(void);
void Afficher_ecran(void);
void Fenetre_Afficher_cadre_general(word Pos_X,word Pos_Y,word Largeur,word Hauteur,
                                    byte Couleur_HG,byte Couleur_BD,byte Couleur_S,byte Couleur_CHG,byte Couleur_CBD);
void Fenetre_Afficher_cadre_mono(word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte Couleur);
void Fenetre_Afficher_cadre_creux(word Pos_X,word Pos_Y,word Largeur,word Hauteur);
void Fenetre_Afficher_cadre_bombe(word Pos_X,word Pos_Y,word Largeur,word Hauteur);
void Fenetre_Afficher_cadre(word Pos_X,word Pos_Y,word Largeur,word Hauteur);

void Afficher_sprite_dans_menu(int Numero_bouton,int Numero_sprite);
void Afficher_pinceau_dans_menu(void);
void Afficher_pinceau_dans_fenetre(word X,word Y,int Numero);

void Dessiner_zigouigoui(word X,word Y, byte Couleur, short Sens);
void Bloc_degrade_dans_fenetre(word Pos_X,word Pos_Y,word Debut_block,word Fin_block);
void Fenetre_Afficher_sprite_drive(word Pos_X,word Pos_Y,byte Type);

byte Meilleure_couleur(byte Rouge,byte Vert,byte Bleu);
byte Meilleure_couleur_sans_exclusion(byte Rouge,byte Vert,byte Bleu);

void Ligne_horizontale_XOR_Zoom(short Pos_X, short Pos_Y, short Largeur);
void Ligne_verticale_XOR_Zoom(short Pos_X, short Pos_Y, short Hauteur);

#endif
