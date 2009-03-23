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

void Gestion_principale            (void);
void Tracer_cadre_de_bouton_du_menu(byte btn_number,byte Enfonce);
void Desenclencher_bouton          (int btn_number);
void Enclencher_bouton             (int btn_number,byte click);
void Annuler_les_effets            (void);
void Restaurer_les_effets          (void);
void Ouvrir_fenetre                (word width,word height, char * title);
void Fermer_fenetre                (void);

void Ouvrir_popup                  (word x_pos, word y_pos, word width, word height);
void Fermer_popup                  (void);

void Fenetre_Dessiner_bouton_normal(word x_pos,word y_pos,word width,word height,
                                    char * title,byte Lettre_soulignee,byte clickable);
void Fenetre_Enfoncer_bouton_normal(word x_pos,word y_pos,word width,word height);
void Fenetre_Desenfoncer_bouton_normal(word x_pos,word y_pos,word width,word height);
void Fenetre_Dessiner_bouton_palette(word x_pos,word y_pos);

void Calculer_hauteur_curseur_jauge(T_Bouton_scroller * button);
void Fenetre_Dessiner_jauge(T_Bouton_scroller * button);
void Fenetre_Dessiner_bouton_scroller(T_Bouton_scroller * button);

void Fenetre_Contenu_bouton_saisie(T_Bouton_special * button, char * content);
void Fenetre_Effacer_bouton_saisie(T_Bouton_special * button);
void Fenetre_Dessiner_bouton_saisie(word x_pos,word y_pos,word Largeur_en_caracteres);

T_Bouton_normal * Fenetre_Definir_bouton_normal(word x_pos, word y_pos,
                                   word width, word height,
                                   char * title,byte Lettre_soulignee,
                                   byte clickable, word Raccourci);
T_Bouton_normal * Fenetre_Definir_bouton_repetable(word x_pos, word y_pos,
                                   word width, word height,
                                   char * title,byte Lettre_soulignee,
                                   byte clickable, word Raccourci);

T_Bouton_palette * Fenetre_Definir_bouton_palette(word x_pos, word y_pos);
void Fenetre_Effacer_tags(void);
void Tagger_intervalle_palette(byte start,byte end);

T_Bouton_scroller * Fenetre_Definir_bouton_scroller(word x_pos, word y_pos,
                                     word height,
                                     word nb_elements,
                                     word nb_elements_visible,
                                     word Position_initiale);
T_Bouton_special * Fenetre_Definir_bouton_special(word x_pos,word y_pos,word width,word height);
T_Bouton_special * Fenetre_Definir_bouton_saisie(word x_pos,word y_pos,word Largeur_en_caracteres);
T_Bouton_dropdown * Fenetre_Definir_bouton_dropdown(word x_pos,word y_pos,word width,word height,word dropdown_width,char *label,byte display_choice,byte display_centered,byte display_arrow,byte active_button);
void Fenetre_Dropdown_choix(T_Bouton_dropdown * dropdown, word btn_number, const char *label);
void Fenetre_Dropdown_vider_choix(T_Bouton_dropdown * dropdown);
byte Fenetre_click_dans_zone(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y);
short Attendre_click_dans_palette(T_Bouton_palette * button);
void Recuperer_couleur_derriere_fenetre(byte * Couleur, byte * click);

short Fenetre_Bouton_clicke(void);
int Numero_bouton_sous_souris(void);
short Fenetre_Numero_bouton_clicke(void);
void Remappe_fond_fenetres(byte * Table_de_conversion, int Min_Y, int Max_Y);
void Pixel_fond(int x_pos, int y_pos, byte Couleur);
