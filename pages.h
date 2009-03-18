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
#ifndef _PAGES_H_
#define _PAGES_H_


//////////////////////////////////////////////////////////////////////////
/////////////////////////// GESTION DU BACKUP ////////////////////////////
//////////////////////////////////////////////////////////////////////////


  ///
  /// GESTION DES PAGES
  ///

void Initialiser_S_Page(S_Page * Page);
void Download_infos_page_principal(S_Page * Page);
void Upload_infos_page_principal(S_Page * Page);
void Download_infos_page_brouillon(S_Page * Page);
void Upload_infos_page_brouillon(S_Page * Page);
void Download_infos_backup(S_Liste_de_pages * Liste);
int Allouer_une_page(S_Page * Page,int Largeur,int Hauteur);
void Liberer_une_page(S_Page * Page);
void Copier_S_page(S_Page * dest,S_Page * Source);
int Taille_d_une_page(S_Page * Page);



  ///
  /// GESTION DES LISTES DE PAGES
  ///

void Initialiser_S_Liste_de_pages(S_Liste_de_pages * Liste);
int Allouer_une_liste_de_pages(S_Liste_de_pages * Liste,int Taille);
void Liberer_une_liste_de_pages(S_Liste_de_pages * Liste);
int Taille_d_une_liste_de_pages(S_Liste_de_pages * Liste);
void Reculer_dans_une_liste_de_pages(S_Liste_de_pages * Liste);
void Avancer_dans_une_liste_de_pages(S_Liste_de_pages * Liste);
int Nouvelle_page_possible(S_Page * Nouvelle_page,S_Liste_de_pages * Liste_courante,S_Liste_de_pages * Liste_secondaire);
void Detruire_derniere_page_allouee_de_la_liste(S_Liste_de_pages * Liste);
void Creer_nouvelle_page(S_Page * Nouvelle_page,S_Liste_de_pages * Liste_courante,S_Liste_de_pages * Liste_secondaire);
void Changer_nombre_de_pages_d_une_liste(S_Liste_de_pages * Liste,int Nb);
void Detruire_la_page_courante_d_une_liste(S_Liste_de_pages * Liste);



  ///
  /// GESTION DES BACKUPS
  ///

int Initialiser_les_listes_de_backups_en_debut_de_programme(int Taille,int Largeur,int Hauteur);
void Detruire_les_listes_de_backups_en_fin_de_programme(void);
void Nouveau_nombre_de_backups(int Nouveau);
int Backup_avec_nouvelles_dimensions(int Upload,int Largeur,int Hauteur);
int Backuper_et_redimensionner_brouillon(int Largeur,int Hauteur);
void Backup(void);
void Undo(void);
void Redo(void);
void Detruire_la_page_courante(void);
void Interchanger_image_principale_et_brouillon(void);



  ///
  /// GESTION DES EMPRUNTS DE MEMOIRE DE PAGE
  ///

int Emprunt_memoire_de_page_possible(int Taille);
void * Emprunter_memoire_de_page(int Taille);



#endif
