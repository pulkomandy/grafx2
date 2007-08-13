#include "global.h"

//byte Meilleure_couleur(byte Rouge, byte Vert, byte Bleu);

void Remapper_ecran_apres_changement_couleurs_menu(void);
void Calculer_couleurs_menu_optimales(struct Composantes * Palette);

dword Memoire_libre(void);

void Liste2tables(word * Liste,short Pas,byte Mode,byte * Table_inc,byte * Table_dec);

void  Num2str(dword Nombre,char * Chaine,byte Taille);
int   Str2num(char * Chaine);

short Round(float Valeur);
short Round_max(short Numerateur,short Diviseur);
short Round_div_max(short Numerateur,short Diviseur);

int Min(int A,int B);

void Transformer_point(short X, short Y,
                       float cosA, float sinA, short * Xr, short * Yr);

void Recadrer_ecran_par_rapport_au_zoom(void);
void Calculer_split(void);
void Calculer_donnees_loupe(void);
void Calculer_limites(void);
void Calculer_coordonnees_pinceau(void);

char* Libelle_mode(int Mode);

void Initialiser_mode_video(int Numero);
void Pixel_dans_barre_d_outil(word X,word Y,byte Couleur);
void Pixel_dans_fenetre(word X,word Y,byte Couleur);
void Encadrer_couleur_menu(byte Couleur);
void Afficher_palette_du_menu(void);
void Afficher_menu(void);
void Recadrer_palette(void);

void Print_general(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond);
void Print_dans_fenetre(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond);
void Print_char_dans_fenetre(short Pos_X,short Pos_Y,char Caractere,byte Couleur_texte,byte Couleur_fond);
void Print_char_transparent_dans_fenetre(short Pos_X,short Pos_Y,char Caractere,byte Couleur);
void Print_dans_menu(char * Chaine, short Position);
void Print_coordonnees(void);
void Print_nom_fichier(void);

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
void Redimentionner_image(word Largeur_choisie,word Hauteur_choisie);

void Fenetre_Afficher_sprite_drive(word Pos_X,word Pos_Y,byte Type);

void Capturer_brosse(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,short Effacement);
void Rotate_90_deg(void);
void Etirer_brosse(short X1, short Y1, short X2, short Y2);
void Etirer_brosse_preview(short X1, short Y1, short X2, short Y2);
void Tourner_brosse(float Angle);
void Tourner_brosse_preview(float Angle);

void Remap_brosse(void);
void Get_colors_from_brush(void);
void Outline_brush(void);
void Nibble_brush(void);

void Remplir(byte Couleur_de_remplissage);
void Remplacer(byte Nouvelle_couleur);

void Pixel_figure_Preview    (short Pos_X,short Pos_Y,byte Couleur);
void Pixel_figure_Preview_xor(short Pos_X,short Pos_Y,byte Couleur);

void Tracer_cercle_vide_Definitif(short Centre_X,short Centre_Y,short Rayon,byte Couleur);
void Tracer_cercle_vide_Preview  (short Centre_X,short Centre_Y,short Rayon,byte Couleur);
void Effacer_cercle_vide_Preview (short Centre_X,short Centre_Y,short Rayon);
void Tracer_cercle_plein         (short Centre_X,short Centre_Y,short Rayon,byte Couleur);

void Tracer_ellipse_vide_Definitif(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);
void Tracer_ellipse_vide_Preview  (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);
void Effacer_ellipse_vide_Preview (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical);
void Tracer_ellipse_pleine        (short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur);

void Tracer_ligne_Definitif  (short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_ligne_Preview    (short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
void Tracer_ligne_Preview_xor(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur);
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

void Polyfill(int Vertices, short * Points, int Color);
void Capturer_brosse_au_lasso(int Vertices, short * Points,short Effacement);

// Gestion des backups:
void Download_infos_page_principal(S_Page * Page);
void Upload_infos_page_principal(S_Page * Page);
void Download_infos_page_brouillon(S_Page * Page);
void Upload_infos_page_brouillon(S_Page * Page);
void Download_infos_backup(S_Liste_de_pages * Liste);
int  Initialiser_les_listes_de_backups_en_debut_de_programme(int Taille,int Largeur,int Hauteur);
void Detruire_les_listes_de_backups_en_fin_de_programme(void);
void Nouveau_nombre_de_backups(int Nouveau);
int  Backup_avec_nouvelles_dimensions(int Upload,int Largeur,int Hauteur);
int  Backuper_et_redimensionner_brouillon(int Largeur,int Hauteur);
void Backup(void);
void Undo(void);
void Redo(void);
void Detruire_la_page_courante(void);
void Interchanger_image_principale_et_brouillon(void);

void Changer_facteur_loupe(byte Indice_facteur);
