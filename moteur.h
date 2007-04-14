
void Gestion_principale            (void);
void Tracer_cadre_de_bouton_du_menu(byte Numero,byte Enfonce);
void Desenclencher_bouton          (int Numero);
void Enclencher_bouton             (int Numero,byte Click);
void Annuler_les_effets            (void);
void Restaurer_les_effets          (void);
void Ouvrir_fenetre                (word Largeur,word Hauteur, char * Titre);
void Fermer_fenetre                (void);

void Fenetre_Dessiner_bouton_normal(word Pos_X,word Pos_Y,word Largeur,word Hauteur,
                                    char * Titre,byte Lettre_soulignee,byte Clickable);
void Fenetre_Enfoncer_bouton_normal(word Pos_X,word Pos_Y,word Largeur,word Hauteur);
void Fenetre_Desenfoncer_bouton_normal(word Pos_X,word Pos_Y,word Largeur,word Hauteur);
void Fenetre_Dessiner_bouton_palette(word Pos_X,word Pos_Y);

void Calculer_hauteur_curseur_jauge(struct Fenetre_Bouton_scroller * Enreg);
void Fenetre_Dessiner_jauge(struct Fenetre_Bouton_scroller * Enreg);
void Fenetre_Dessiner_bouton_scroller(struct Fenetre_Bouton_scroller * Enreg);

void Fenetre_Contenu_bouton_saisie(struct Fenetre_Bouton_special * Enreg, char * Contenu);
void Fenetre_Dessiner_bouton_saisie(word Pos_X,word Pos_Y,word Largeur_en_caracteres);

void Fenetre_Definir_bouton_normal(word Pos_X, word Pos_Y,
                                   word Largeur, word Hauteur,
                                   char * Titre,byte Lettre_soulignee,
                                   byte Clickable, word Raccourci);

void Fenetre_Definir_bouton_palette(word Pos_X, word Pos_Y);
void Fenetre_Effacer_tags(void);
void Tagger_intervalle_palette(byte Debut,byte Fin);

void Fenetre_Definir_bouton_scroller(word Pos_X, word Pos_Y,
                                     word Hauteur,
                                     word Nb_elements,
                                     word Nb_elements_visibles,
                                     word Position_initiale);
void Fenetre_Definir_bouton_special(word Pos_X,word Pos_Y,word Largeur,word Hauteur);
void Fenetre_Definir_bouton_saisie(word Pos_X,word Pos_Y,word Largeur_en_caracteres);

byte Fenetre_click_dans_zone(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y);
short Attendre_click_dans_palette(struct Fenetre_Bouton_palette * Enreg);
void Recuperer_couleur_derriere_fenetre(byte * Couleur, byte * Click);

short Fenetre_Bouton_clicke(void);
