// Modifie Principal_Repertoire_courant en y mettant sa nouvelle valeur
// (avec le nom du disque)
int  Determiner_repertoire_courant(void);
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
int  Repertoire_existe(char * Repertoire);
//   Détermine si un fichier passé en paramètre existe ou non dans le
// répertoire courant.
int Fichier_existe(char * Fichier);

// -- Destruction de la liste chaînée ---------------------------------------
void Detruire_liste_du_fileselect(void);
// -- Lecture d'une liste de fichiers ---------------------------------------
void Lire_liste_des_fichiers(byte Format_demande);
// -- Tri de la liste des fichiers et répertoires ---------------------------
void Trier_la_liste_des_fichiers(void);
// -- Affichage des éléments de la liste de fichier / répertoire ------------
void Afficher_la_liste_des_fichiers(short Decalage_premier,short Decalage_select);
// -- Récupérer le libellé d'un élément de la liste -------------------------
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * Libelle);

// -- Déplacements dans la liste des fichiers -------------------------------

void Select_Scroll_Down(short * Decalage_premier,short * Decalage_select);
void Select_Scroll_Up  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Down  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Up    (short * Decalage_premier,short * Decalage_select);
void Select_End        (short * Decalage_premier,short * Decalage_select);
void Select_Home       (short * Decalage_premier,short * Decalage_select);

short Calculer_decalage_click_dans_fileselector(void);

char * Nom_formate(char * Nom);
