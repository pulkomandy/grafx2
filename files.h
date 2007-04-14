// Modifie Principal_Repertoire_courant en y mettant sa nouvelle valeur
// (avec le nom du disque)
int  Determiner_repertoire_courant(void);
//   D‚termine si un r‚pertoire pass‚ en paramŠtre existe ou non dans le
// r‚pertoire courant.
int  Repertoire_existe(char * Repertoire);
//   D‚termine si un fichier pass‚ en paramŠtre existe ou non dans le
// r‚pertoire courant.
int Fichier_existe(char * Fichier);

// -- Destruction de la liste chaŒn‚e ---------------------------------------
void Detruire_liste_du_fileselect(void);
// -- Lecture d'une liste de fichiers ---------------------------------------
void Lire_liste_des_fichiers(byte Format_demande);
// -- Tri de la liste des fichiers et r‚pertoires ---------------------------
void Trier_la_liste_des_fichiers(void);
// -- Affichage des ‚l‚ments de la liste de fichier / r‚pertoire ------------
void Afficher_la_liste_des_fichiers(short Decalage_premier,short Decalage_select);
// -- R‚cup‚rer le libell‚ d'un ‚l‚ment de la liste -------------------------
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * Libelle);

// -- D‚placements dans la liste des fichiers -------------------------------

void Select_Scroll_Down(short * Decalage_premier,short * Decalage_select);
void Select_Scroll_Up  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Down  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Up    (short * Decalage_premier,short * Decalage_select);
void Select_End        (short * Decalage_premier,short * Decalage_select);
void Select_Home       (short * Decalage_premier,short * Decalage_select);

short Calculer_decalage_click_dans_fileselector(void);

char * Nom_formate(char * Nom);
