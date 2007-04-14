void Effacer_ecran_courant(void);
void Copier_ecran_sur_brouillon(void);
void Copier_image_dans_brosse(short Debut_X,short Debut_Y,short Brosse_Largeur,short Brosse_Hauteur,word Largeur_image);
void Permuter_dans_l_image_les_couleurs(byte Couleur_1,byte Couleur_2);
void Remap_general_LOWLEVEL(byte * Table_conv,byte * Buffer,short Largeur,short Hauteur,short Largeur_buffer);
void Scroll_picture(short Decalage_X,short Decalage_Y);
void Clavier_americain(void);
void Clavier_de_depart(void);
byte Recuperer_nb_lignes(void);
void Passer_en_mode_texte(byte Nb_lignes);
word Detection_souris(void);
void Set_mouse_video_mode_number(void);
void Get_input(void);
void Set_mouse_position(void);
void Clip_mouse(void);
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
byte Get_key(void);

void Pixel_dans_ecran_courant      (word X,word Y,byte Couleur);
void Pixel_dans_brosse             (word X,word Y,byte Couleur);
byte Lit_pixel_dans_ecran_courant  (word X,word Y);
byte Lit_pixel_dans_ecran_brouillon(word X,word Y);
byte Lit_pixel_dans_ecran_backup   (word X,word Y);
byte Lit_pixel_dans_ecran_feedback (word X,word Y);
byte Lit_pixel_dans_brosse         (word X,word Y);

long Freespace(byte Numero_de_lecteur);
// Numero_de_lecteur = 0 : Disque courant
//                     1 : lecteur A:
//                     2 : lecteur B:
//                     3 : lecteur C:
//                   ... : ...
//
// R‚sultat = -1 si lecteur invalide, taille disponible sinon

byte Type_de_lecteur_de_disquette(byte Numero_de_lecteur);
// Numero_de_lecteur compris entre 0 et 3 (4 lecteurs de disquettes)
//
// R‚sultat = 0 : Pas de lecteur
//            1 : Lecteur 360 Ko
//            2 : Lecteur 1.2 Mo
//            3 : Lecteur 720 Ko
//            4 : Lecteur 1.4 Mo
//            5 : Lecteur 2.8 Mo (??? pas sur ???)
//            6 : Lecteur 2.8 Mo

byte Disk_map(byte Numero_de_lecteur);
// Renseigne sur la lettre logique d'un lecteur
// (utile pour tester si un lecteur de disquette est A: ou B: aux yeux du DOS)
//
// Entr‚e: Octet = nø du lecteur (1=A, 2=B ...)
//
// Sortie: Octet = 0FFh : Pas de lecteur (???)
//                 sinon: num‚ro repr‚sentant la lettre logique du lecteur
//                        (commence … 1)

byte Disque_dur_present(byte Numero_de_disque);
// Numero_de_disque = 0:C, 1:D, 2:E, ...
//
// R‚sultat = 0 : Pas de disque dur pr‚sent
//            1 : Disque dur pr‚sent

byte Lecteur_CDROM_present(byte Numero_de_lecteur);
// Numero_de_lecteur = 0 : lecteur A:
//                     1 : lecteur B:
//                     2 : lecteur C:
//                   ... : ...
//
// R‚sultat = 0 : Pas de lecteur CD-ROM pr‚sent
//            1 : Lecteur CD-ROM pr‚sent


void Ellipse_Calculer_limites(short Rayon_horizontal,short Rayon_vertical);
// Calcule les valeurs suivantes en fonction des deux paramŠtres:
//
// Ellipse_Rayon_vertical_au_carre
// Ellipse_Rayon_horizontal_au_carre
// Ellipse_Limite_High
// Ellipse_Limite_Low


byte Pixel_dans_ellipse(void);
//   Indique si le pixel se trouvant … Ellipse_Curseur_X pixels
// (Ellipse_Curseur_X>0 = … droite, Ellipse_Curseur_X<0 = … gauche) et …
// Ellipse_Curseur_Y pixels (Ellipse_Curseur_Y>0 = en bas,
// Ellipse_Curseur_Y<0 = en haut) du centre se trouve dans l'ellipse en
// cours.

byte Pixel_dans_cercle(void);
//   Indique si le pixel se trouvant … Cercle_Curseur_X pixels
// (Cercle_Curseur_X>0 = … droite, Cercle_Curseur_X<0 = … gauche) et …
// Cercle_Curseur_Y pixels (Cercle_Curseur_Y>0 = en bas,
// Cercle_Curseur_Y<0 = en haut) du centre se trouve dans le cercle en
// cours.

byte Bit(byte Octet, byte Rang);
// Extrait un bit d'un certain rang … partir d'un octet.

byte Couleur_ILBM_line(word Pos_X, word Vraie_taille_ligne);
// Renvoie la couleur du pixel (ILBM) en Pos_X.

// Gestion du chrono dans les fileselects
void Initialiser_chrono(long Delai);
void Tester_chrono(void);

void Remplacer_une_couleur(byte Ancienne_couleur, byte Nouvelle_couleur);
void Remplacer_toutes_les_couleurs_dans_limites(byte * Table_de_remplacement);

byte Meilleure_couleur(byte Rouge,byte Vert,byte Bleu);
byte Meilleure_couleur_sans_exclusion(byte Rouge,byte Vert,byte Bleu);

byte Effet_Colorize_interpole  (word X,word Y,byte Couleur);
byte Effet_Colorize_additif    (word X,word Y,byte Couleur);
byte Effet_Colorize_soustractif(word X,word Y,byte Couleur);
byte Effet_Trame(word X,word Y);

void Flip_Y_LOWLEVEL(void);
void Flip_X_LOWLEVEL(void);
void Rotate_90_deg_LOWLEVEL(byte * Source,byte * Destination);
void Rotate_180_deg_LOWLEVEL(void);
void Zoomer_une_ligne(byte * Ligne_originale,byte * Ligne_zoomee,word Facteur,word Largeur);
void Copier_une_partie_d_image_dans_une_autre(byte * Source,word S_Pos_X,word S_Pos_Y,word Largeur,word Hauteur,word Largeur_source,byte * Destination,word D_Pos_X,word D_Pos_Y,word Largeur_destination);
