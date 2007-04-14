//////////////////////////////////////////////////////////////////////////
/////////////////////////// GESTION DU BACKUP ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "linux.h"
#include "global.h"
#include "pages.h"
#include "graph.h"

//  On d‚clare m‚chamment le prototype de Erreur pour ‚viter de faire un
// fichier "main.h":
void Erreur(int Code);

  ///
  /// GESTION DES PAGES
  ///

void Initialiser_S_Page(S_Page * Page)
{
  // Important: appeler cette fonction sur toute nouvelle structure S_Page!

  if (Page!=NULL)
  {
    Page->Image=NULL;
    Page->Largeur=0;
    Page->Hauteur=0;
    memset(Page->Palette,0,sizeof(T_Palette));
    memset(Page->Commentaire,0,TAILLE_COMMENTAIRE+1);
    memset(Page->Repertoire_fichier,0,256);
    memset(Page->Nom_fichier,0,13);
    Page->Format_fichier=FORMAT_PAR_DEFAUT;
/*
    Page->Decalage_X=0;
    Page->Decalage_Y=0;
    Page->Ancien_Decalage_X=0;
    Page->Ancien_Decalage_Y=0;
    Page->Split=0;
    Page->X_Zoom=0;
    Page->Proportion_split=PROPORTION_SPLIT;
    Page->Loupe_Mode=0;
    Page->Loupe_Facteur=FACTEUR_DE_ZOOM_PAR_DEFAUT;
    Page->Loupe_Hauteur=0;
    Page->Loupe_Largeur=0;
    Page->Loupe_Decalage_X=0;
    Page->Loupe_Decalage_Y=0;
*/
  }
}

void Download_infos_page_principal(S_Page * Page)
{
  int Indice_facteur;
  int Dimensions_modifiees;

  if (Page!=NULL)
  {
    Dimensions_modifiees=(Principal_Largeur_image!=Page->Largeur) ||
                         (Principal_Hauteur_image!=Page->Hauteur);

    Principal_Ecran=Page->Image;
    Principal_Largeur_image=Page->Largeur;
    Principal_Hauteur_image=Page->Hauteur;
    memcpy(Principal_Palette,Page->Palette,sizeof(T_Palette));
    memcpy(Principal_Commentaire,Page->Commentaire,TAILLE_COMMENTAIRE+1);
    memcpy(Principal_Repertoire_fichier,Page->Repertoire_fichier,256);
    memcpy(Principal_Nom_fichier,Page->Nom_fichier,13);
    Principal_Format_fichier=Page->Format_fichier;
/*
    Principal_Decalage_X=Page->Decalage_X;
    Principal_Decalage_Y=Page->Decalage_Y;

    // On corrige les d‚calages en fonction de la dimension de l'‚cran
    if ( (Principal_Decalage_X>0) &&
         (Principal_Decalage_X+Largeur_ecran>Principal_Largeur_image) )
      Principal_Decalage_X=Max(0,Principal_Largeur_image-Largeur_ecran);
    if ( (Principal_Decalage_Y>0) &&
         (Principal_Decalage_Y+Menu_Ordonnee>Principal_Hauteur_image) )
      Principal_Decalage_Y=Max(0,Principal_Hauteur_image-Menu_Ordonnee);

    Ancien_Principal_Decalage_X=Page->Ancien_Decalage_X;
    Ancien_Principal_Decalage_Y=Page->Ancien_Decalage_Y;
    Principal_Split=Page->Split;
    Principal_X_Zoom=Page->X_Zoom;
    Principal_Proportion_split=Page->Proportion_split;
    Loupe_Mode=Page->Loupe_Mode;
    Loupe_Facteur=Page->Loupe_Facteur;
    Loupe_Hauteur=Page->Loupe_Hauteur;
    Loupe_Largeur=Page->Loupe_Largeur;
    Loupe_Decalage_X=Page->Loupe_Decalage_X;
    Loupe_Decalage_Y=Page->Loupe_Decalage_Y;

    // Comme le facteur de zoom a des chances d'avoir chang‚, on appelle
    // "Changer_facteur_loupe".
    for (Indice_facteur=0; FACTEUR_ZOOM[Indice_facteur]!=Loupe_Facteur; Indice_facteur++);
    Changer_facteur_loupe(Indice_facteur);
*/
    if (Dimensions_modifiees)
    {
      Loupe_Mode=0;
      Principal_Decalage_X=0;
      Principal_Decalage_Y=0;
      Pixel_Preview=Pixel_Preview_Normal;
      Calculer_limites();
      Calculer_coordonnees_pinceau();
    }
  }
}

void Upload_infos_page_principal(S_Page * Page)
{
  if (Page!=NULL)
  {
    Page->Image=Principal_Ecran;
    Page->Largeur=Principal_Largeur_image;
    Page->Hauteur=Principal_Hauteur_image;
    memcpy(Page->Palette,Principal_Palette,sizeof(T_Palette));
    memcpy(Page->Commentaire,Principal_Commentaire,TAILLE_COMMENTAIRE+1);
    memcpy(Page->Repertoire_fichier,Principal_Repertoire_fichier,256);
    memcpy(Page->Nom_fichier,Principal_Nom_fichier,13);
    Page->Format_fichier=Principal_Format_fichier;
/*
    Page->Decalage_X=Principal_Decalage_X;
    Page->Decalage_Y=Principal_Decalage_Y;
    Page->Ancien_Decalage_X=Ancien_Principal_Decalage_X;
    Page->Ancien_Decalage_X=Ancien_Principal_Decalage_Y;
    Page->Split=Principal_Split;
    Page->X_Zoom=Principal_X_Zoom;
    Page->Proportion_split=Principal_Proportion_split;
    Page->Loupe_Mode=Loupe_Mode;
    Page->Loupe_Facteur=Loupe_Facteur;
    Page->Loupe_Hauteur=Loupe_Hauteur;
    Page->Loupe_Largeur=Loupe_Largeur;
    Page->Loupe_Decalage_X=Loupe_Decalage_X;
    Page->Loupe_Decalage_Y=Loupe_Decalage_Y;
*/
  }
}

void Download_infos_page_brouillon(S_Page * Page)
{
  if (Page!=NULL)
  {
    Brouillon_Ecran=Page->Image;
    Brouillon_Largeur_image=Page->Largeur;
    Brouillon_Hauteur_image=Page->Hauteur;
    memcpy(Brouillon_Palette,Page->Palette,sizeof(T_Palette));
    memcpy(Brouillon_Commentaire,Page->Commentaire,TAILLE_COMMENTAIRE+1);
    memcpy(Brouillon_Repertoire_fichier,Page->Repertoire_fichier,256);
    memcpy(Brouillon_Nom_fichier,Page->Nom_fichier,13);
    Brouillon_Format_fichier=Page->Format_fichier;
/*
    Brouillon_Decalage_X=Page->Decalage_X;
    Brouillon_Decalage_Y=Page->Decalage_Y;
    Ancien_Brouillon_Decalage_X=Page->Ancien_Decalage_X;
    Ancien_Brouillon_Decalage_Y=Page->Ancien_Decalage_Y;
    Brouillon_Split=Page->Split;
    Brouillon_X_Zoom=Page->X_Zoom;
    Brouillon_Proportion_split=Page->Proportion_split;
    Brouillon_Loupe_Mode=Page->Loupe_Mode;
    Brouillon_Loupe_Facteur=Page->Loupe_Facteur;
    Brouillon_Loupe_Hauteur=Page->Loupe_Hauteur;
    Brouillon_Loupe_Largeur=Page->Loupe_Largeur;
    Brouillon_Loupe_Decalage_X=Page->Loupe_Decalage_X;
    Brouillon_Loupe_Decalage_Y=Page->Loupe_Decalage_Y;
*/
  }
}

void Upload_infos_page_brouillon(S_Page * Page)
{
  if (Page!=NULL)
  {
    Page->Image=Brouillon_Ecran;
    Page->Largeur=Brouillon_Largeur_image;
    Page->Hauteur=Brouillon_Hauteur_image;
    memcpy(Page->Palette,Brouillon_Palette,sizeof(T_Palette));
    memcpy(Page->Commentaire,Brouillon_Commentaire,TAILLE_COMMENTAIRE+1);
    memcpy(Page->Repertoire_fichier,Brouillon_Repertoire_fichier,256);
    memcpy(Page->Nom_fichier,Brouillon_Nom_fichier,13);
    Page->Format_fichier=Brouillon_Format_fichier;
/*
    Page->Decalage_X=Brouillon_Decalage_X;
    Page->Decalage_Y=Brouillon_Decalage_Y;
    Page->Ancien_Decalage_X=Ancien_Brouillon_Decalage_X;
    Page->Ancien_Decalage_Y=Ancien_Brouillon_Decalage_Y;
    Page->Split=Brouillon_Split;
    Page->X_Zoom=Brouillon_X_Zoom;
    Page->Proportion_split=Brouillon_Proportion_split;
    Page->Loupe_Mode=Brouillon_Loupe_Mode;
    Page->Loupe_Facteur=Brouillon_Loupe_Facteur;
    Page->Loupe_Hauteur=Brouillon_Loupe_Hauteur;
    Page->Loupe_Largeur=Brouillon_Loupe_Largeur;
    Page->Loupe_Decalage_X=Brouillon_Loupe_Decalage_X;
    Page->Loupe_Decalage_Y=Brouillon_Loupe_Decalage_Y;
*/
  }
}

void Download_infos_backup(S_Liste_de_pages * Liste)
{
  Ecran_backup=Liste->Pages[1].Image;

  if (Config.FX_Feedback)
    FX_Feedback_Ecran=Liste->Pages[0].Image;
  else
    FX_Feedback_Ecran=Liste->Pages[1].Image;
}

int Allouer_une_page(S_Page * Page,int Largeur,int Hauteur)
{
  // Important: la S_Page ne doit pas d‚j… d‚signer une page allou‚e auquel
  //            cas celle-ci serait perdue.

  /* Debug : if (Page->Image!=NULL) exit(666); */

  // On alloue la m‚moire pour le bitmap
  Page->Image=(byte *)malloc(Largeur*Hauteur);

  // On v‚rifie que l'allocation se soit bien pass‚e
  if (Page->Image==NULL)
    return 0; // Echec
  else
  {
    Page->Largeur=Largeur;
    Page->Hauteur=Hauteur;
    // Important: La mise … jour des autres infos est du ressort de
    //            l'appelant.

    return 1; // SuccŠs
  }
}

void Liberer_une_page(S_Page * Page)
{
  // On peut appeler cette fonction sur une page non allou‚e.

  if (Page->Image!=NULL)
    free(Page->Image);
  Page->Image=NULL;
  Page->Largeur=0;
  Page->Hauteur=0;
  // On ne se pr‚occupe pas de ce que deviens le reste des infos de l'image.
}

void Copier_S_page(S_Page * Destination,S_Page * Source)
{
  *Destination=*Source;
}

int Taille_d_une_page(S_Page * Page)
{
  return sizeof(S_Page)+(Page->Largeur*Page->Hauteur)+8;
  // 8 = 4 + 4
  // (Toute zone allou‚e en m‚moire est pr‚c‚d‚e d'un mot double indiquant sa
  // taille, or la taille d'un mot double est de 4 octets, et on utilise deux
  // allocations de m‚moires: une pour la S_Page et une pour l'image)
}


  ///
  /// GESTION DES LISTES DE PAGES
  ///

void Initialiser_S_Liste_de_pages(S_Liste_de_pages * Liste)
{
  // Important: appeler cette fonction sur toute nouvelle structure
  //            S_Liste_de_pages!

  Liste->Taille_liste=0;
  Liste->Nb_pages_allouees=0;
  Liste->Pages=NULL;
}

int Allouer_une_liste_de_pages(S_Liste_de_pages * Liste,int Taille)
{
  // Important: la S_Liste_de_pages ne doit pas d‚j… d‚signer une liste de
  //            pages allou‚e auquel cas celle-ci serait perdue.
  int Indice;

  /* Debug : if (Liste->Pages!=NULL) exit(666); */

  // On alloue la m‚moire pour la liste
  Liste->Pages=(S_Page *)malloc(Taille*sizeof(S_Page));

  // On v‚rifie que l'allocation se soit bien pass‚e
  if (Liste->Pages==NULL)
    return 0; // Echec
  else
  {
    // On initialise chacune des nouvelles pages
    for (Indice=0;Indice<Taille;Indice++)
      Initialiser_S_Page(Liste->Pages+Indice);
    Liste->Taille_liste=Taille;
    Liste->Nb_pages_allouees=0;

    return 1; // SuccŠs
  }
}

void Liberer_une_liste_de_pages(S_Liste_de_pages * Liste)
{
  // On peut appeler cette fonction sur une liste de pages non allou‚e.

  // Important: cette fonction ne libŠre pas les pages de la liste. Il faut
  //            donc le faire pr‚alablement si n‚cessaire.

  if (Liste->Pages!=NULL)
    free(Liste->Pages);
  Liste->Pages=NULL;
  Liste->Taille_liste=0;
  Liste->Nb_pages_allouees=0;
}

int Taille_d_une_liste_de_pages(S_Liste_de_pages * Liste)
{
  int Resultat=0;
  int Indice;

  for (Indice=0;Indice<Liste->Nb_pages_allouees;Indice++)
    Resultat+=Taille_d_une_page(Liste->Pages+Indice);

  return Resultat+sizeof(S_Liste_de_pages)+4;

  // C.F. la remarque … propos de Taille_d_une_page pour la valeur 4.
}

void Reculer_dans_une_liste_de_pages(S_Liste_de_pages * Liste)
{
  // Cette fonction fait l'‚quivalent d'un "Undo" dans la liste de pages.
  // Elle effectue une sorte de ROL (Rotation Left) sur la liste:
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º0º1³2³3³4³5³6³7³8³9³Aº  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |  0=Page courante
  //  ³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³   |_ A=Page la plus ancienne
  //  v v v v v v v v v v v   |  1=DerniŠre page (1er backup)
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º1º2³3³4³5³6³7³8³9³A³0º  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |

  // Pour simuler un v‚ritable Undo, l'appelant doit mettre la structure
  // de page courante … jour avant l'appel, puis en r‚extraire les infos en
  // sortie, ainsi que celles relatives … la plus r‚cente page d'undo (1Šre
  // page de la liste).

  int Indice;
  S_Page * Page_tempo;

  if (Liste->Nb_pages_allouees>1)
  {
    // On cr‚e la page tempo
    Page_tempo=(S_Page *)malloc(sizeof(S_Page));
    Initialiser_S_Page(Page_tempo);

    // On copie la 1Šre page (Page 0) dans la page temporaire
    Copier_S_page(Page_tempo,Liste->Pages);

    // On copie toutes les pages 1-A … leur gauche
    for (Indice=1;Indice<Liste->Nb_pages_allouees;Indice++)
      Copier_S_page(Liste->Pages+Indice-1,Liste->Pages+Indice);

    // On copie la page 0 (dont la sauvegarde a ‚t‚ effectu‚e dans la page
    // temporaire) en derniŠre position
    Copier_S_page(Liste->Pages+Liste->Nb_pages_allouees-1,Page_tempo);

    // On d‚truit la page tempo
    free(Page_tempo);
  }
}

void Avancer_dans_une_liste_de_pages(S_Liste_de_pages * Liste)
{
  // Cette fonction fait l'‚quivalent d'un "Redo" dans la liste de pages.
  // Elle effectue une sorte de ROR (Rotation Right) sur la liste:
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º0º1³2³3³4³5³6³7³8³9³Aº  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |  0=Page courante
  //  ³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³   |_ A=Page la plus ancienne
  //  v v v v v v v v v v v   |  1=DerniŠre page (1er backup)
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // ºAº0³1³2³3³4³5³6³7³8³9º  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |

  // Pour simuler un v‚ritable Redo, l'appelant doit mettre la structure
  // de page courante … jour avant l'appel, puis en r‚extraire les infos en
  // sortie, ainsi que celles relatives … la plus r‚cente page d'undo (1Šre
  // page de la liste).

  int Indice;
  S_Page * Page_tempo;

  if (Liste->Nb_pages_allouees>1)
  {
    // On cr‚e la page tempo
    Page_tempo=(S_Page *)malloc(sizeof(S_Page));
    Initialiser_S_Page(Page_tempo);

    // On copie la derniŠre page dans la page temporaire
    Copier_S_page(Page_tempo,Liste->Pages+Liste->Nb_pages_allouees-1);

    // On copie toutes les pages 0-9 … leur droite
    for (Indice=Liste->Nb_pages_allouees-1;Indice>0;Indice--)
      Copier_S_page(Liste->Pages+Indice,Liste->Pages+Indice-1);

    // On copie la page plus ancienne page (la "A", dont la sauvegarde a ‚t‚
    // effectu‚e dans la page temporaire) en 1Šre position
    Copier_S_page(Liste->Pages,Page_tempo);

    // On d‚truit la page tempo
    free(Page_tempo);
  }
}

int Nouvelle_page_possible(S_Page * Nouvelle_page,S_Liste_de_pages * Liste_courante,S_Liste_de_pages * Liste_secondaire)
{
  int Taille_immediatement_disponible;
  int Taille_liste_courante;
  int Taille_liste_brouillon;
  int Taille_page_courante;
  int Taille_page_brouillon;
  int Taille_nouvelle_page;

  Taille_immediatement_disponible=Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER;
  Taille_liste_courante =Taille_d_une_liste_de_pages(Liste_courante);
  Taille_liste_brouillon=Taille_d_une_liste_de_pages(Liste_secondaire);
  Taille_page_courante  =Taille_d_une_page(Liste_courante->Pages);
  Taille_page_brouillon =Taille_d_une_page(Liste_secondaire->Pages);
  Taille_nouvelle_page  =Taille_d_une_page(Nouvelle_page);

  // Il faut pouvoir loger la nouvelle page et son backup dans la page
  // courante, en conservant au pire la 1Šre page de brouillon.
  if ((Taille_immediatement_disponible+Taille_liste_courante+
       Taille_liste_brouillon-Taille_page_brouillon)<(2*Taille_nouvelle_page))
    return 0;

  // Il faut pouvoir loger le brouillon et son backup dans la page de
  // brouillon, en conservant au pire un exemplaire de la nouvelle page dans
  // la page courante. (pour permettre … l'utilisateur de travailler sur son
  // brouillon)
  if ((Taille_immediatement_disponible+Taille_liste_courante+
       Taille_liste_brouillon-Taille_nouvelle_page)<(2*Taille_page_brouillon))
    return 0;

  return 1;
}

void Detruire_derniere_page_allouee_de_la_liste(S_Liste_de_pages * Liste)
{
  if (Liste!=NULL)
  {
    if (Liste->Nb_pages_allouees>0)
    {
      Liste->Nb_pages_allouees--;
      Liberer_une_page(Liste->Pages+Liste->Nb_pages_allouees);
    }
  }
}

void Creer_nouvelle_page(S_Page * Nouvelle_page,S_Liste_de_pages * Liste_courante,S_Liste_de_pages * Liste_secondaire)
{

//   Cette fonction cr‚e une nouvelle page dont les attributs correspondent …
// ceux de Nouvelle_page (Largeur,Hauteur,...) (le champ Image est invalide
// … l'appel, c'est la fonction qui le met … jour), et l'enfile dans
// Liste_courante.
//   Il est imp‚ratif que la cr‚ation de cette page soit possible,
// ‚ventuellement au d‚triment des backups de la page de brouillon
// (Liste_secondaire). Afin de s'en assurer, il faut v‚rifier cette
// possibilit‚ … l'aide de
// Nouvelle_page_possible(Nouvelle_page,Liste_courante,Liste_secondaire) avant
// l'appel … cette fonction.
//   De plus, il faut qu'il y ait au moins une page dans chacune des listes.

  int                Il_faut_liberer;
  S_Liste_de_pages * Liste_a_raboter;
  S_Page *           Page_a_supprimer;
  int                Indice;

  // On regarde s'il faut lib‚rer des pages:
  Il_faut_liberer=
    // C'est le cas si la Liste_courante est pleine
  (  (Liste_courante->Taille_liste==Liste_courante->Nb_pages_allouees)
    // ou qu'il ne reste plus assez de place pour allouer la Nouvelle_page
  || ( (Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER)<
       (Nouvelle_page->Hauteur*Nouvelle_page->Largeur) )  );

  if (!Il_faut_liberer)
  {
    // On a assez de place pour allouer une page, et de plus la Liste_courante
    // n'est pas pleine. On n'a donc aucune page … supprimer. On peut en
    // allouer une directement.
    Nouvelle_page->Image=(byte *)malloc(Nouvelle_page->Hauteur*Nouvelle_page->Largeur);
  }
  else
  {
    // On manque de m‚moire ou la Liste_courante est pleine. Dans tous les
    // cas, il faut lib‚rer une page... qui peut-ˆtre pourra re-servir.

    // Tant qu'il faut lib‚rer
    while (Il_faut_liberer)
    {
      // On cherche sur quelle liste on va virer une page

      // S'il reste des pages … lib‚rer dans la Liste_courante
      if (Liste_courante->Nb_pages_allouees>1)
        // Alors on va d‚truire la derniŠre page allou‚e de la Liste_courante
        Liste_a_raboter=Liste_courante;
      else
      {
        if (Liste_secondaire->Nb_pages_allouees>1)
        {
          // Sinon on va d‚truire la derniŠre page allou‚e de la
          // Liste_secondaire
          Liste_a_raboter=Liste_secondaire;
        }
        else
        {
          Erreur(ERREUR_SORRY_SORRY_SORRY);
        }
      }

      // Puis on d‚termine la page que l'on va supprimer (c'est la derniŠre de
      // la liste)
      Page_a_supprimer=Liste_a_raboter->Pages+(Liste_a_raboter->Nb_pages_allouees)-1;

      // On regarde si on peut recycler directement la page (cas o— elle
      // aurait la mˆme surface que la Nouvelle_page)
      if ((Page_a_supprimer->Hauteur*Page_a_supprimer->Largeur)==
          (Nouvelle_page->Hauteur*Nouvelle_page->Largeur))
      {
        // Alors
        // On r‚cupŠre le bitmap de la page … supprimer (‚vite de faire des
        // allocations/d‚sallocations fastidieuses et inutiles)
        Nouvelle_page->Image=Page_a_supprimer->Image;

        // On fait semblant que la derniŠre page allou‚e ne l'est pas
        Liste_a_raboter->Nb_pages_allouees--;

        // On n'a plus besoin de lib‚rer de la m‚moire puisqu'on a refil‚ …
        // Nouvelle_page un bitmap valide
        Il_faut_liberer=0;
      }
      else
      {
        // Sinon

        // D‚truire la derniŠre page allou‚e dans la Liste_…_raboter
        Detruire_derniere_page_allouee_de_la_liste(Liste_a_raboter);

        // On regarde s'il faut continuer … lib‚rer de la place
        Il_faut_liberer=(Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER)
                       <(Nouvelle_page->Hauteur*Nouvelle_page->Largeur);

        // S'il ne faut pas, c'est qu'on peut allouer un bitmap
        // pour la Nouvelle_page
        if (!Il_faut_liberer)
          Nouvelle_page->Image=(byte *)malloc(Nouvelle_page->Hauteur*Nouvelle_page->Largeur);
      }
    }
  }

  // D'aprŠs l'hypothŠse de d‚part, la boucle ci-dessus doit s'arrˆter car
  // on a assez de m‚moire pour allouer la nouvelle page.
  // D‚sormais Nouvelle_page contient un pointeur sur une zone bitmap valide.

  // D‚caler la Liste_courante d'un cran vers le pass‚.
  for (Indice=Liste_courante->Taille_liste-1;Indice>0;Indice--)
    Copier_S_page(Liste_courante->Pages+Indice,Liste_courante->Pages+Indice-1);

  // Recopier la Nouvelle_page en 1Šre position de la Liste_courante
  Copier_S_page(Liste_courante->Pages,Nouvelle_page);
  Liste_courante->Nb_pages_allouees++;
}

void Changer_nombre_de_pages_d_une_liste(S_Liste_de_pages * Liste,int Nb)
{
  int Indice;
  S_Page * Nouvelles_pages;

  // Si la liste a d‚j… la taille demand‚e
  if (Liste->Taille_liste==Nb)
    // Alors il n'y a rien … faire
    return;

  // Si la liste contient plus de pages que souhait‚
  if (Liste->Taille_liste>Nb)
    // Alors pour chaque page en exc‚s
    for (Indice=Nb;Indice<Liste->Taille_liste;Indice++)
      // On libŠre la page
      Liberer_une_page(Liste->Pages+Indice);

  // On fait une nouvelle liste de pages:
  Nouvelles_pages=(S_Page *)malloc(Nb*sizeof(S_Page));
  for (Indice=0;Indice<Nb;Indice++)
    Initialiser_S_Page(Nouvelles_pages+Indice);

  // On recopie les pages … conserver de l'ancienne liste
  for (Indice=0;Indice<Min(Nb,Liste->Taille_liste);Indice++)
    Copier_S_page(Nouvelles_pages+Indice,Liste->Pages+Indice);

  // On libŠre l'ancienne liste
  free(Liste->Pages);

  // On met … jour les champs de la nouvelle liste
  Liste->Pages=Nouvelles_pages;
  Liste->Taille_liste=Nb;
  if (Liste->Nb_pages_allouees>Nb)
    Liste->Nb_pages_allouees=Nb;
}

void Detruire_la_page_courante_d_une_liste(S_Liste_de_pages * Liste)
{
  // On ne peut pas d‚truire la page courante de la liste si aprŠs
  // destruction il ne reste pas encore au moins une page.
  if (Liste->Nb_pages_allouees>1)
  {
    // On fait faire un undo … la liste, comme ‡a, la nouvelle page courante
    // est la page pr‚c‚dente
    Reculer_dans_une_liste_de_pages(Principal_Backups);

    // Puis on d‚truit la derniŠre page, qui est l'ancienne page courante
    Detruire_derniere_page_allouee_de_la_liste(Liste);
  }
}


  ///
  /// GESTION DES BACKUPS
  ///

int Initialiser_les_listes_de_backups_en_debut_de_programme(int Taille,int Largeur,int Hauteur)
{
  // Taille correspond au nombre de pages que l'on souhaite dans chaque liste
  // (1 pour la page courante, puis 1 pour chaque backup, soit 2 au minimum).
  // Largeur et Hauteur correspondent … la dimension des images de d‚part.

  S_Page * Page;
  int Retour=0;

  if (Allouer_une_liste_de_pages(Principal_Backups,Taille) &&
      Allouer_une_liste_de_pages(Brouillon_Backups,Taille))
  {
    // On a r‚ussi … allouer deux listes de pages dont la taille correspond …
    // celle demand‚e par l'utilisateur.

    // On cr‚e un descripteur de page correspondant … la page principale
    Page=(S_Page *)malloc(sizeof(S_Page));
    Initialiser_S_Page(Page);
    Upload_infos_page_principal(Page);
    // On y met les infos sur la dimension de d‚marrage
    Page->Largeur=Largeur;
    Page->Hauteur=Hauteur;

    // On regarde si on peut ajouter cette page
    if (Nouvelle_page_possible(Page,Principal_Backups,Brouillon_Backups))
    {
      // On peut, donc on va la cr‚er
      Creer_nouvelle_page(Page,Principal_Backups,Brouillon_Backups);
      Download_infos_page_principal(Page);
      Download_infos_backup(Principal_Backups);

      // Maintenant, on regarde si on a le droit de cr‚er la mˆme page dans
      // la page de brouillon.
      if (Nouvelle_page_possible(Page,Brouillon_Backups,Principal_Backups))
      {
        // On peut donc on le fait
        Creer_nouvelle_page(Page,Brouillon_Backups,Principal_Backups);
        Download_infos_page_brouillon(Page);

        // Et on efface les 2 images en les remplacant de "0"
        memset(Principal_Ecran,0,Principal_Largeur_image*Principal_Hauteur_image);
        memset(Brouillon_Ecran,0,Brouillon_Largeur_image*Brouillon_Hauteur_image);

        Retour=1;
      }
      else
      {
        // Il n'est pas possible de d‚marrer le programme avec la page
        // principale et la page de brouillon aux dimensions demand‚e par
        // l'utilisateur. ==> On l'envoie ballader
        Retour=0;
      }
    }
    else
    {
      // On ne peut pas d‚marrer le programme avec ne serait-ce qu'une page
      // de la dimension souhait‚e, donc on laisse tout tomber et on le
      // renvoie chier.
      Retour=0;
    }
  }
  else
  {
    // On n'a mˆme pas r‚ussi … cr‚er les listes. Donc c'est mˆme pas la
    // peine de continuer : l'utilisateur ne pourra jamais rien faire, autant
    // avorter le chargement du programme.
    Retour=0;
  }

  return Retour;
}

void Detruire_les_listes_de_backups_en_fin_de_programme(void)
{
  // On commence par supprimer les pages une … une dans chacune des listes
    // Liste de la page principale
  while (Principal_Backups->Nb_pages_allouees>0)
    Detruire_derniere_page_allouee_de_la_liste(Principal_Backups);
    // Liste de la page de brouillon
  while (Brouillon_Backups->Nb_pages_allouees>0)
    Detruire_derniere_page_allouee_de_la_liste(Brouillon_Backups);

  // Puis on peut d‚truire les structures de liste elles-mˆmes
  Liberer_une_liste_de_pages(Principal_Backups);
  Liberer_une_liste_de_pages(Brouillon_Backups);
  free(Principal_Backups);
  free(Brouillon_Backups);
}

void Nouveau_nombre_de_backups(int Nouveau)
{
  Changer_nombre_de_pages_d_une_liste(Principal_Backups,Nouveau+1);
  Changer_nombre_de_pages_d_une_liste(Brouillon_Backups,Nouveau+1);

  // Le +1 vient du fait que dans chaque liste, en 1Šre position on retrouve
  // les infos de la page courante sur le brouillon et la page principale.
  // (Nouveau = Nombre de backups, sans compter les pages courantes)
}

int Backup_avec_nouvelles_dimensions(int Upload,int Largeur,int Hauteur)
{
  // Retourne 1 si une nouvelle page est disponible (alors pleine de 0) et
  // 0 sinon.

  S_Page * Nouvelle_page;
  int Retour=0;

  if (Upload)
    // On remet … jour l'‚tat des infos de la page courante (pour pouvoir les
    // retrouver plus tard)
    Upload_infos_page_principal(Principal_Backups->Pages);

  // On cr‚e un descripteur pour la nouvelle page courante
  Nouvelle_page=(S_Page *)malloc(sizeof(S_Page));
  Initialiser_S_Page(Nouvelle_page);

  Upload_infos_page_principal(Nouvelle_page);
  Nouvelle_page->Largeur=Largeur;
  Nouvelle_page->Hauteur=Hauteur;
  if (Nouvelle_page_possible(Nouvelle_page,Principal_Backups,Brouillon_Backups))
  {
    Creer_nouvelle_page(Nouvelle_page,Principal_Backups,Brouillon_Backups);
    Download_infos_page_principal(Nouvelle_page);
    Download_infos_backup(Principal_Backups);
    // On nettoie la nouvelle image:
    memset(Principal_Ecran,0,Principal_Largeur_image*Principal_Hauteur_image);
    Retour=1;
  }

  // On d‚truit le descripteur de la page courante
  free(Nouvelle_page);

  return Retour;
}

int Backuper_et_redimensionner_brouillon(int Largeur,int Hauteur)
{
  // Retourne 1 si la page de dimension souhaitee est disponible en brouillon
  // et 0 sinon.

  S_Page * Nouvelle_page;
  int Retour=0;

  // On remet … jour l'‚tat des infos de la page de brouillon (pour pouvoir
  // les retrouver plus tard)
  Upload_infos_page_brouillon(Brouillon_Backups->Pages);

  // On cr‚e un descripteur pour la nouvelle page de brouillon
  Nouvelle_page=(S_Page *)malloc(sizeof(S_Page));
  Initialiser_S_Page(Nouvelle_page);

  Upload_infos_page_brouillon(Nouvelle_page);
  Nouvelle_page->Largeur=Largeur;
  Nouvelle_page->Hauteur=Hauteur;
  if (Nouvelle_page_possible(Nouvelle_page,Brouillon_Backups,Principal_Backups))
  {
    Creer_nouvelle_page(Nouvelle_page,Brouillon_Backups,Principal_Backups);
    Download_infos_page_brouillon(Nouvelle_page);
    Retour=1;
  }

  // On d‚truit le descripteur de la page courante
  free(Nouvelle_page);

  return Retour;
}

void Backup(void)
{
  S_Page * Nouvelle_page;

  // On remet … jour l'‚tat des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_principal(Principal_Backups->Pages);

  // On cr‚e un descripteur pour la nouvelle page courante
  Nouvelle_page=(S_Page *)malloc(sizeof(S_Page));
  Initialiser_S_Page(Nouvelle_page);

  // Enrichissement de l'historique
  Copier_S_page(Nouvelle_page,Principal_Backups->Pages);
  Creer_nouvelle_page(Nouvelle_page,Principal_Backups,Brouillon_Backups);
  Download_infos_page_principal(Nouvelle_page);
  Download_infos_backup(Principal_Backups);

  // On copie l'image du backup vers la page courante:
  memcpy(Principal_Ecran,Ecran_backup,Principal_Largeur_image*Principal_Hauteur_image);

  // On d‚truit le descripteur de la page courante
  free(Nouvelle_page);

  // On allume l'indicateur de modification de l'image
  Principal_Image_modifiee=1;
}

void Undo(void)
{
  // On remet … jour l'‚tat des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_principal(Principal_Backups->Pages);
  // On fait faire un undo … la liste des backups de la page principale
  Reculer_dans_une_liste_de_pages(Principal_Backups);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_principal(Principal_Backups->Pages);
  // Et celles du backup
  Download_infos_backup(Principal_Backups);
  // Note: le backup n'a pas obligatoirement les mˆmes dimensions ni la mˆme
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilis‚ … la suite d'un Undo. Donc ‡a ne devrait pas
  //       poser de problŠmes.
}

void Redo(void)
{
  // On remet … jour l'‚tat des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_principal(Principal_Backups->Pages);
  // On fait faire un redo … la liste des backups de la page principale
  Avancer_dans_une_liste_de_pages(Principal_Backups);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_principal(Principal_Backups->Pages);
  // Et celles du backup
  Download_infos_backup(Principal_Backups);
  // Note: le backup n'a pas obligatoirement les mˆmes dimensions ni la mˆme
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilis‚ … la suite d'un Redo. Donc ‡a ne devrait pas
  //       poser de problŠmes.
}

void Detruire_la_page_courante(void)
{
  // On d‚truit la page courante de la liste principale
  Detruire_la_page_courante_d_une_liste(Principal_Backups);
  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_principal(Principal_Backups->Pages);
  // Et celles du backup
  Download_infos_backup(Principal_Backups);
  // Note: le backup n'a pas obligatoirement les mˆmes dimensions ni la mˆme
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilis‚ … la suite d'une destruction de page. Donc ‡a ne
  //       devrait pas poser de problŠmes.
}

void Interchanger_image_principale_et_brouillon(void)
{
  S_Liste_de_pages * Liste_tempo;

  // On commence par mettre … jour dans les descripteurs les infos sur les
  // pages qu'on s'apprˆte … ‚changer, pour qu'on se retrouve pas avec de
  // vieilles valeurs qui datent de mathuzalem.
  Upload_infos_page_principal(Principal_Backups->Pages);
  Upload_infos_page_brouillon(Brouillon_Backups->Pages);

  // On inverse les listes de pages
  Liste_tempo=Principal_Backups;
  Principal_Backups=Brouillon_Backups;
  Brouillon_Backups=Liste_tempo;

  // On extrait ensuite les infos sur les nouvelles pages courante, brouillon
  // et backup.

    /* SECTION GROS CACA PROUT PROUT */
    // Auparavant on ruse en mettant d‚j… … jour les dimensions de la
    // nouvelle page courante. Si on ne le fait pas, le "Download" va d‚tecter
    // un changement de dimensions et va bˆtement sortir du mode loupe, alors
    // que lors d'un changement de page, on veut bien conserver l'‚tat du mode
    // loupe du brouillon.
    Principal_Largeur_image=Principal_Backups->Pages->Largeur;
    Principal_Hauteur_image=Principal_Backups->Pages->Hauteur;

  Download_infos_page_principal(Principal_Backups->Pages);
  Download_infos_page_brouillon(Brouillon_Backups->Pages);
  Download_infos_backup(Principal_Backups);
}


int Emprunt_memoire_de_page_possible(int taille)
{
  int Taille_immediatement_disponible;
  int Taille_liste_courante;
  int Taille_liste_brouillon;
  int Taille_page_courante;
  int Taille_page_brouillon;

  Taille_immediatement_disponible=Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER;
  Taille_liste_courante =Taille_d_une_liste_de_pages(Principal_Backups);
  Taille_liste_brouillon=Taille_d_une_liste_de_pages(Brouillon_Backups);
  Taille_page_courante  =Taille_d_une_page(Principal_Backups->Pages);
  Taille_page_brouillon =Taille_d_une_page(Brouillon_Backups->Pages);

  // Il faut pouvoir loger la zone m‚moire ainsi qu'un exemplaire de la page
  // courante, en conservant au pire la 1Šre page de brouillon.
  if ((Taille_immediatement_disponible
      +Taille_liste_courante
      +Taille_liste_brouillon
      -Taille_page_courante
      -Taille_page_brouillon)<taille)
    return 0;

  return 1;
}

void * Emprunter_memoire_de_page(int taille)
{
  int                Il_faut_liberer;
  S_Liste_de_pages * Liste_a_raboter;
  S_Page *           Page_a_supprimer;
  int                Indice;

  if (Emprunt_memoire_de_page_possible(taille))
  {
    // On regarde s'il faut lib‚rer des pages:
    Il_faut_liberer=
      (Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER)<taille;

    if (!Il_faut_liberer)
    {
      // On a assez de place pour allouer une page. On n'a donc aucune page
      // … supprimer. On peut allouer de la m‚moire directement.
      return malloc(taille);
    }
    else
    {
      // On manque de m‚moire. Il faut lib‚rer une page...

      // Tant qu'il faut lib‚rer
      while (Il_faut_liberer)
      {
        // On cherche sur quelle liste on va virer une page

        // S'il reste des pages … lib‚rer dans la liste des brouillons
        if (Brouillon_Backups->Nb_pages_allouees>1)
          // Alors on va d‚truire la derniŠre page allou‚e de la liste des
          // brouillons
          Liste_a_raboter=Brouillon_Backups;
        else
        {
          if (Principal_Backups->Nb_pages_allouees>1)
          {
            // Sinon on va d‚truire la derniŠre page allou‚e de la
            // liste principale
            Liste_a_raboter=Principal_Backups;
          }
          else
          {
            // Dans cette branche, il ‚tait pr‚vu qu'on obtienne la m‚moire
            // n‚cessaire mais on n'arrive pas … la trouver. On indique donc
            // qu'elle n'est pas disponible, et on aura perdu des backups
            // pour rien
            return 0;
          }
        }

        // Puis on d‚termine la page que l'on va supprimer (c'est la derniŠre
        // de la liste)
        Page_a_supprimer=Liste_a_raboter->Pages+(Liste_a_raboter->Nb_pages_allouees)-1;

        // D‚truire la derniŠre page allou‚e dans la Liste_…_raboter
        Detruire_derniere_page_allouee_de_la_liste(Liste_a_raboter);

        // On regarde s'il faut continuer … lib‚rer de la place
        Il_faut_liberer=
          (Memoire_libre()-QUANTITE_MINIMALE_DE_MEMOIRE_A_CONSERVER)<taille;

        // S'il ne faut pas, c'est qu'on peut allouer un bitmap
        // pour la Nouvelle_page
        if (!Il_faut_liberer)
          return malloc(taille);
      }
    }
  }
  else
  {
    // Il n'y a pas assez de place pour allouer la m‚moire temporaire dans
    // la m‚moire r‚serv‚e aux pages.
    return 0;
  }

  // Pour que le compilateur ne dise pas qu'il manque une valeur de sortie:
  return 0;
}
