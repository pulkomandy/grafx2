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
void Copier_S_page(S_Page * Destination,S_Page * Source);
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

int Emprunt_memoire_de_page_possible(int taille);
void * Emprunter_memoire_de_page(int taille);



#endif
