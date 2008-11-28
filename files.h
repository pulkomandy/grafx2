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
// Modifie Principal_Repertoire_courant en y mettant sa nouvelle valeur
// (avec le nom du disque)
int  Determiner_repertoire_courant(void);

// -- Destruction de la liste chaînée ---------------------------------------
void Detruire_liste_du_fileselect(void);
// -- Lecture d'une liste de fichiers ---------------------------------------
void Lire_liste_des_fichiers(byte Format_demande);
// -- Lecture d'une liste de lecteurs / volumes -----------------------------
void Lire_liste_des_lecteurs(void);
// -- Tri de la liste des fichiers et répertoires ---------------------------
void Trier_la_liste_des_fichiers(void);
// -- Affichage des éléments de la liste de fichier / répertoire ------------
void Afficher_la_liste_des_fichiers(short Decalage_premier,short Decalage_select);
// -- Récupérer le libellé d'un élément de la liste -------------------------
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * Libelle,int *Type);

// -- Déplacements dans la liste des fichiers -------------------------------

void Select_Scroll_Down(short * Decalage_premier,short * Decalage_select);
void Select_Scroll_Up  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Down  (short * Decalage_premier,short * Decalage_select);
void Select_Page_Up    (short * Decalage_premier,short * Decalage_select);
void Select_End        (short * Decalage_premier,short * Decalage_select);
void Select_Home       (short * Decalage_premier,short * Decalage_select);

short Calculer_decalage_click_dans_fileselector(void);

char * Nom_formate(char * Nom, int Type);

// Scans a directory, calls Callback for each file in it,
void for_each_file(const char * Nom_repertoire, void Callback(const char *));

