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

void Liste2tables(word * Liste, short Pas, byte Mode, byte * Table_inc,
        byte * Table_dec
);

void Transformer_point(short X, short Y,
                       float cosA, float sinA, short * Xr, short * Yr);

int Initialiser_mode_video(int Largeur, int Hauteur, int Fullscreen);

byte Aucun_effet(word X,word Y,byte Couleur);
byte Effet_Shade(word X,word Y,byte Couleur);
byte Effet_Quick_shade(word X,word Y,byte Couleur);
byte Effet_Tiling(word X,word Y,byte Couleur);
byte Effet_Smooth(word X,word Y,byte Couleur);

void Afficher_foreback(void);


void Afficher_pixel(word X,word Y,byte Couleur);

void Afficher_pinceau(short X,short Y,byte Couleur,byte Preview);
void Effacer_pinceau(short X,short Y);
void Effacer_curseur(void);
void Afficher_curseur(void);

void Redimentionner_image(word Largeur_choisie,word Hauteur_choisie);

void Remplir(byte Couleur_de_remplissage);
void Remplacer(byte Nouvelle_couleur);

void Pixel_figure_Preview    (word Pos_X,word Pos_Y,byte Couleur);
void Pixel_figure_Preview_auto(word Pos_X,word Pos_Y);
void Pixel_figure_Preview_xor(word Pos_X,word Pos_Y,byte Couleur);
void Pixel_figure_Preview_xorback(word Pos_X,word Pos_Y,byte Couleur);
void Pixel_figure_Dans_brosse(word Pos_X,word Pos_Y,byte Couleur);

void Tracer_cercle_vide_Definitif(short Centre_X,short Centre_Y,short Rayon,byte Couleur);
void Tracer_cercle_vide_Preview  (short Centre_X,short Centre_Y,short Rayon,byte Couleur);
void Effacer_cercle_vide_Preview (short Centre_X,short Centre_Y,short Rayon);
void Tracer_cercle_plein         (short Centre_X,short Centre_Y,short Rayon,byte Couleur);

void Tracer_ellipse_vide_Definitif(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);
void Tracer_ellipse_vide_Preview  (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);
void Effacer_ellipse_vide_Preview (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical);
void Tracer_ellipse_pleine        (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);

void Rectifier_coordonnees_a_45_degres(short AX, short AY, short* BX, short* BY);
void Tracer_ligne_General(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y, byte Couleur);
void Tracer_ligne_Definitif  (short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_ligne_Preview    (short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_ligne_Preview_xor(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_ligne_Preview_xorback(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Effacer_ligne_Preview   (short Debut_X,short Debut_Y,short Fin_X,short Fin_Y);

void Tracer_rectangle_vide(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_rectangle_plein(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);

void Tracer_courbe_Definitif(short X1, short Y1, short X2, short Y2, short X3, short Y3, short X4, short Y4, byte Couleur);
void Tracer_courbe_Preview  (short X1, short Y1, short X2, short Y2, short X3, short Y3, short X4, short Y4, byte Couleur);
void Effacer_courbe_Preview (short X1, short Y1, short X2, short Y2, short X3, short Y3, short X4, short Y4, byte Couleur);

void Aerographe(short Bouton_clicke);

void Degrade_de_base           (long Indice,short Pos_X,short Pos_Y);
void Degrade_de_trames_simples (long Indice,short Pos_X,short Pos_Y);
void Degrade_de_trames_etendues(long Indice,short Pos_X,short Pos_Y);
void Degrade_aleatoire         (long Indice,short Pos_X,short Pos_Y);

void Tracer_cercle_degrade  (short Centre_X,short Centre_Y,short Rayon,short Eclairage_X,short Eclairage_Y);
void Tracer_ellipse_degradee(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,short Eclairage_X,short Eclairage_Y);
void Tracer_rectangle_degrade(short RAX,short RAY,short RBX,short RBY,short VAX,short VAY, short VBX, short VBY);

void Polyfill_General(int Vertices, short * Points, int Color);
void Polyfill(int Vertices, short * Points, int Color);

// Gestion des backups:
void Download_infos_page_principal(S_Page * Page);
void Download_infos_page_brouillon(S_Page * Page);
void Download_infos_backup(S_Liste_de_pages * Liste);
void Detruire_les_listes_de_backups_en_fin_de_programme(void);
void Nouveau_nombre_de_backups(int Nouveau);
int  Backup_avec_nouvelles_dimensions(int Upload,int Largeur,int Hauteur);
int  Backuper_et_redimensionner_brouillon(int Largeur,int Hauteur);
void Undo(void);
void Redo(void);
void Detruire_la_page_courante(void);
void Interchanger_image_principale_et_brouillon(void);

void Changer_facteur_loupe(byte Indice_facteur);

void Remap_picture(void);

#define ToWinX(x) (((x)*Menu_Facteur_X)+Fenetre_Pos_X)
#define ToWinY(y) (((y)*Menu_Facteur_Y)+Fenetre_Pos_Y)
#define ToWinL(l) ((l)*Menu_Facteur_X)
#define ToWinH(h) ((h)*Menu_Facteur_Y)

// Définition d'une fonction générique de traçage de figures:
fonction_afficheur Pixel_figure;
void Mettre_Ecran_A_Jour(short X, short Y, short Largeur, short Hauteur);
