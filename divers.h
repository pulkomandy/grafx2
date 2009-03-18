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
void Effacer_ecran_courant(void);
void Copier_ecran_sur_brouillon(void);
void Copier_image_dans_brosse(short Debut_X,short Debut_Y,short Brosse_Largeur,short Brosse_Hauteur,word Largeur_image);
void Permuter_dans_l_image_les_couleurs(byte Couleur_1,byte Couleur_2);
void Remap_general_LOWLEVEL(byte * Table_conv,byte * Buffer,short Largeur,short Hauteur,short Largeur_buffer);
void Scroll_picture(short x_offset,short y_offset);
void Set_mouse_video_mode_number(void);
void Set_mouse_position(void);
void Attendre_fin_de_click(void);
void Sensibilite_souris(word X,word Y);
void Set_color(byte Couleur, byte Rouge, byte Vert, byte Bleu);
void Set_palette(T_Palette Palette);
void Palette_256_to_64(T_Palette Palette);
void Palette_64_to_256(T_Palette Palette);
void Effacer_image_courante(byte Couleur);
void Effacer_image_courante_Stencil(byte Couleur, byte * Pochoir);
void Wait_VBL(void);
void Tempo_jauge(byte Vitesse);
dword Round_div(dword Numerateur,dword Diviseur);
word Palette_Compter_nb_couleurs_utilisees(dword * Tableau);

void Pixel_dans_ecran_courant      (word X,word Y,byte Couleur);
void Pixel_dans_brosse             (word X,word Y,byte Couleur);
byte Lit_pixel_dans_ecran_courant  (word X,word Y);
byte Lit_pixel_dans_ecran_brouillon(word X,word Y);
byte Lit_pixel_dans_ecran_backup   (word X,word Y);
byte Lit_pixel_dans_ecran_feedback (word X,word Y);
byte Lit_pixel_dans_brosse         (word X,word Y);

void Ellipse_Calculer_limites(short Rayon_horizontal,short Rayon_vertical);
// Calcule les valeurs suivantes en fonction des deux paramètres:
//
// Ellipse_Rayon_vertical_au_carre
// Ellipse_Rayon_horizontal_au_carre
// Ellipse_Limite_High
// Ellipse_Limite_Low


byte Pixel_dans_ellipse(void);
//   Indique si le pixel se trouvant à Ellipse_Curseur_X pixels
// (Ellipse_Curseur_X>0 = à droite, Ellipse_Curseur_X<0 = à gauche) et à
// Ellipse_Curseur_Y pixels (Ellipse_Curseur_Y>0 = en bas,
// Ellipse_Curseur_Y<0 = en haut) du centre se trouve dans l'ellipse en
// cours.

byte Pixel_dans_cercle(void);
//   Indique si le pixel se trouvant à Cercle_Curseur_X pixels
// (Cercle_Curseur_X>0 = à droite, Cercle_Curseur_X<0 = à gauche) et à
// Cercle_Curseur_Y pixels (Cercle_Curseur_Y>0 = en bas,
// Cercle_Curseur_Y<0 = en haut) du centre se trouve dans le cercle en
// cours.

// Gestion du chrono dans les fileselects
void Initialiser_chrono(dword Delai);
void Tester_chrono(void);

void Remplacer_une_couleur(byte Ancienne_couleur, byte Nouvelle_couleur);
void Remplacer_toutes_les_couleurs_dans_limites(byte * Table_de_remplacement);

byte Effet_Colorize_interpole  (word X,word Y,byte Couleur);
byte Effet_Colorize_additif    (word X,word Y,byte Couleur);
byte Effet_Colorize_soustractif(word X,word Y,byte Couleur);
byte Effet_Trame(word X,word Y);

void Flip_Y_LOWLEVEL(void);
void Flip_X_LOWLEVEL(void);
void Rotate_90_deg_LOWLEVEL(byte * Source,byte * dest);
void Rotate_180_deg_LOWLEVEL(void);
void Zoomer_une_ligne(byte * Ligne_originale,byte * Ligne_zoomee,word factor,word Largeur);
void Copier_une_partie_d_image_dans_une_autre(byte * Source,word S_Pos_X,word S_Pos_Y,word Largeur,word Hauteur,word Largeur_source,byte * dest,word D_Pos_X,word D_Pos_Y,word Largeur_destination);

// -- Gestion du chrono --
byte Etat_chrono; // Etat du chrono: 0=Attente d'un Xème de seconde
                  //                 1=Il faut afficher la preview
                  //                 2=Plus de chrono à gerer pour l'instant
dword Chrono_delay;     // Nombre de 18.2ème de secondes demandés
dword Chrono_cmp;       // Heure de départ du chrono
byte Nouvelle_preview; // Booléen "Il faut relancer le chrono de preview"


unsigned long Memoire_libre(void);

void  Num2str(dword number,char * Chaine,byte Taille);
int   Str2num(char * Chaine);

short Round(float Valeur);
short Round_max(short Numerateur,short Diviseur);
short Round_div_max(short Numerateur,short Diviseur);

int Min(int a,int b);
int Max(int a,int b);

char* Libelle_mode(int Mode);
int Conversion_argument_mode(const char *Argument);
