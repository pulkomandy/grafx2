#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <SDL/SDL.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "graph.h"
#include "moteur.h"
#include "readline.h"
#include "files.h"
#include "loadsave.h"
#include "init.h"
#include "boutons.h"
#include "operatio.h"
#include "pages.h"
#include "erreurs.h"
#include "readini.h"
#include "saveini.h"
#include "shade.h"

#ifdef __WATCOMC__
    #include <windows.h>
    #define chdir(x) SetCurrentDirectory(x)
    #define rmdir(x) DeleteFile(x)
#endif

#define FILENAMESPACE 13

//-- MODELE DE BOUTON DE MENU ------------------------------------------------
/*
void Bouton_***(void)
{
  short Bouton_clicke;

  Ouvrir_fenetre(310,190,"***");

  Fenetre_Definir_bouton_normal(103,137,80,14,"OK",0,1,SDLK_RETURN); // 1
  Fenetre_Definir_bouton_scroller(18,44,88,16,4,0);             // 2

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke!=1);

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_***);
  Afficher_curseur();
}
*/


void Message_Non_disponible(void)
{
  short Bouton_clicke;

  Ouvrir_fenetre(160,76,"Not available yet!");

  Print_dans_fenetre(8,20,"This function will",CM_Noir,CM_Clair);
  Print_dans_fenetre(12,28,"be implemented in",CM_Noir,CM_Clair);
  Print_dans_fenetre(16,36,"a later version.",CM_Noir,CM_Clair);
  Fenetre_Definir_bouton_normal(60,53,40,14,"OK",1,1,SDLK_RETURN); // 1
  Afficher_curseur();

  do
    Bouton_clicke=Fenetre_Bouton_clicke();
  while ((Bouton_clicke<=0) && (Touche!=SDLK_ESCAPE) && (Touche!=SDLK_o));

  Fermer_fenetre();

  //   Puisque cette fonction peut être appelée par plusieurs boutons et qu'on
  // ne sait pas lequel c'est, on les désenclenche tous. De toutes façons, ça
  // ne sert à rien d'essayer d'optimiser ça puisque l'utilisateur ne devrait
  // pas souvent l'appeler, et en plus y'en a pas beaucoup à désenclencher. ;)
  Desenclencher_bouton(BOUTON_GRADRECT);
  Desenclencher_bouton(BOUTON_TEXTE);

  Afficher_curseur();
}


void Message_Memoire_insuffisante(void)
{
  short Bouton_clicke;

  Ouvrir_fenetre(216,76,"Not enough memory!");

  Print_dans_fenetre(8,20,"Please consult the manual",CM_Noir,CM_Clair);
  Print_dans_fenetre(24,28,"to know how to obtain",CM_Noir,CM_Clair);
  Print_dans_fenetre(36,36,"more memory space.",CM_Noir,CM_Clair);
  Fenetre_Definir_bouton_normal(60,53,40,14,"OK",1,1,SDLK_RETURN); // 1
  Afficher_curseur();

  do
    Bouton_clicke=Fenetre_Bouton_clicke();
  while ((Bouton_clicke<=0) && (Touche!=SDLK_ESCAPE) && (Touche!=SDLK_o));

  Fermer_fenetre();
  Afficher_curseur();
}


void Bouton_Message_initial(void)
{
  short Bouton_clicke;
  char  Chaine[21];
  int   Pos_X,Offs_Y,X,Y;

  sprintf(Chaine,"GrafX %d.%.2d%s%s",VERSION1, VERSION2, ALPHA_BETA,POURCENTAGE_VERSION);
  Ouvrir_fenetre(260,172,Chaine);

  Fenetre_Afficher_cadre_creux(10,20,239,62);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*11),
        Fenetre_Pos_Y+(Menu_Facteur_Y*21),
        Menu_Facteur_X*237,Menu_Facteur_Y*60,CM_Noir);
  for (Y=23,Offs_Y=0; Y<79; Offs_Y+=231,Y++)
    for (X=14,Pos_X=0; Pos_X<231; Pos_X++,X++)
      Pixel_dans_fenetre(X,Y,Logo_GrafX2[Offs_Y+Pos_X]);

  Print_dans_fenetre(27, 88,"Copyright (c) 1996-1999 by",CM_Fonce,CM_Clair);
  Print_dans_fenetre(79, 96,"Sunset Design",CM_Noir,CM_Clair);
  Print_dans_fenetre(55,104,"All rights reserved",CM_Fonce,CM_Clair);

  Print_dans_fenetre( 7,120,"You should read the README!.1ST",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 7,128,"file before using this program",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 7,136,"for the first time.",CM_Fonce,CM_Clair);

    SDL_UpdateRect(Ecran_SDL,(Largeur_ecran-(260*Menu_Facteur_X))>>1,(Hauteur_ecran-(172*Menu_Facteur_Y))>>1,260*Menu_Facteur_X,172*Menu_Facteur_Y);
  Fenetre_Definir_bouton_normal(90,151,80,14,"OK",0,1,SDLK_RETURN); // 1

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while ((Bouton_clicke!=1) && (Touche!=SDLK_ESCAPE) && (Touche!=SDLK_o));

  Fermer_fenetre();
  Afficher_curseur();
}



void Changer_la_forme_du_pinceau(byte Forme)
{
  Pinceau_Forme=Forme;
  Afficher_pinceau_dans_menu();

  switch (Operation_en_cours)
  {
    case OPERATION_FILL :
      Pinceau_Forme_avant_fill=Forme;
      Pinceau_Forme=FORME_PINCEAU_POINT;
      break;
    case OPERATION_PIPETTE :
      Pinceau_Forme_avant_pipette=Forme;
      Pinceau_Forme=FORME_PINCEAU_POINT;
      break;
    // Note: Il existe un Pinceau_Forme_avant_lasso, mais comme le lasso aura
    // été automatiquement désactivé avant d'arriver ici, y'a pas de problème.
  }
}


//-------------------------------- UNDO/REDO ---------------------------------
void Bouton_Undo(void)
{
  Effacer_curseur();
  Undo();

  Set_palette(Principal_Palette);
  Calculer_couleurs_menu_optimales(Principal_Palette);

  Afficher_ecran();
  Desenclencher_bouton(BOUTON_UNDO);
  Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,Loupe_Mode);
  Afficher_menu();
  Afficher_curseur();
}

void Bouton_Redo(void)
{
  Effacer_curseur();
  Redo();

  Set_palette(Principal_Palette);
  Calculer_couleurs_menu_optimales(Principal_Palette);

  Afficher_ecran();
  Desenclencher_bouton(BOUTON_UNDO);
  Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,Loupe_Mode);
  Afficher_menu();
  Afficher_curseur();
}


//---------------------------- SCROLL PALETTE LEFT ---------------------------
void Bouton_Pal_left(void)
{
  Effacer_curseur();
  if (Couleur_debut_palette)
  {
    Couleur_debut_palette-=8;
    Afficher_palette_du_menu();
  }
  Desenclencher_bouton(BOUTON_PAL_LEFT);
  Afficher_curseur();
}

void Bouton_Pal_left_fast(void)
{
  Effacer_curseur();
  if (Couleur_debut_palette)
  {
    Couleur_debut_palette=(Couleur_debut_palette>=64)? Couleur_debut_palette-64 : 0;
    Afficher_palette_du_menu();
  }
  Desenclencher_bouton(BOUTON_PAL_LEFT);
  Afficher_curseur();
}


//--------------------------- SCROLL PALETTE RIGHT ---------------------------
void Bouton_Pal_right(void)
{
  Effacer_curseur();
  if (Couleur_debut_palette<=184)
  {
    Couleur_debut_palette+=8;
    Afficher_palette_du_menu();
  }
  Desenclencher_bouton(BOUTON_PAL_RIGHT);
  Afficher_curseur();
}

void Bouton_Pal_right_fast(void)
{
  Effacer_curseur();
  if (Couleur_debut_palette<=184)
  {
    Couleur_debut_palette=(Couleur_debut_palette<=120)? Couleur_debut_palette+64 : 192;
    Afficher_palette_du_menu();
  }
  Desenclencher_bouton(BOUTON_PAL_RIGHT);
  Afficher_curseur();
}

//-------------------- Choix de la forecolor dans le menu --------------------
void Bouton_Choix_forecolor(void)
{
  word Pos_X, Pos_Y;

  Pos_X=(Mouse_X/Menu_Facteur_X)-(LARGEUR_MENU+1);
  Pos_Y=((Mouse_Y-Menu_Ordonnee)/Menu_Facteur_Y)-2;

  if ((!Config.Couleurs_separees)
   || ((Pos_X%Menu_Taille_couleur!=Menu_Taille_couleur-1) && ((Pos_Y&3)!=3)))
  {
    Effacer_curseur();
    Encadrer_couleur_menu(CM_Noir);
    Fore_color=Couleur_debut_palette+((Pos_X/Menu_Taille_couleur)<<3)+(Pos_Y>>2);
    Encadrer_couleur_menu(CM_Blanc);
    Afficher_foreback();
    Afficher_curseur();
  }
}

//-------------------- Choix de la backcolor dans le menu --------------------
void Bouton_Choix_backcolor(void)
{
  word Pos_X, Pos_Y;

  Pos_X=(Mouse_X/Menu_Facteur_X)-(LARGEUR_MENU+1);
  Pos_Y=((Mouse_Y-Menu_Ordonnee)/Menu_Facteur_Y)-2;

  if ((!Config.Couleurs_separees)
   || ((Pos_X%Menu_Taille_couleur!=Menu_Taille_couleur-1) && ((Pos_Y&3)!=3)))
  {
    Effacer_curseur();
    Back_color=Couleur_debut_palette+((Pos_X/Menu_Taille_couleur)<<3)+(Pos_Y>>2);
    Afficher_foreback();
    Afficher_curseur();
  }
}


//---------------------- Cacher ou réafficher le menu ------------------------
void Pixel_dans_barre_d_outil_cachee(word X,word Y,byte Couleur)
{
  // C'est fait exprès que ce soit vide...
  // C'est parce que y'a rien du tout à afficher vu que la barre d'outil est
  // cachée... C'est simple non?
}


void Bouton_Cacher_menu(void)
{
  Effacer_curseur();
  if (Menu_visible)
  {
    Menu_visible=0;
    Pixel_dans_menu=Pixel_dans_barre_d_outil_cachee;
    Menu_Ordonnee=Hauteur_ecran;

    if (Loupe_Mode)
    {
      Calculer_donnees_loupe();
      if (Loupe_Decalage_Y+Loupe_Hauteur>Principal_Hauteur_image)
      {
        if (Loupe_Hauteur>Principal_Hauteur_image)
          Loupe_Decalage_Y=0;
        else
          Loupe_Decalage_Y=Principal_Hauteur_image-Loupe_Hauteur;
      }
    }

    //   On repositionne le décalage de l'image pour qu'il n'y ait pas d'in-
    // -cohérences lorsqu'on sortira du mode Loupe.
    if (Principal_Decalage_Y+Hauteur_ecran>Principal_Hauteur_image)
    {
      if (Hauteur_ecran>Principal_Hauteur_image)
        Principal_Decalage_Y=0;
      else
        Principal_Decalage_Y=Principal_Hauteur_image-Hauteur_ecran;
    }
    // On fait pareil pour le brouillon
    if (Brouillon_Decalage_Y+Hauteur_ecran>Brouillon_Hauteur_image)
    {
      if (Hauteur_ecran>Brouillon_Hauteur_image)
        Brouillon_Decalage_Y=0;
      else
        Brouillon_Decalage_Y=Brouillon_Hauteur_image-Hauteur_ecran;
    }

    Calculer_donnees_loupe();
    if (Loupe_Mode)
      Recadrer_ecran_par_rapport_au_zoom();
    Calculer_limites();
    Calculer_coordonnees_pinceau();
    Afficher_ecran();
  }
  else
  {
    Menu_visible=1;
    Pixel_dans_menu=Pixel_dans_barre_d_outil;
    Menu_Ordonnee=Hauteur_ecran-(HAUTEUR_MENU*Menu_Facteur_Y);

    Calculer_donnees_loupe();
    if (Loupe_Mode)
      Recadrer_ecran_par_rapport_au_zoom();
    Calculer_limites();
    Calculer_coordonnees_pinceau();
    Afficher_menu();
    if (Loupe_Mode)
      Afficher_ecran();
  }
  Desenclencher_bouton(BOUTON_CACHER);
  Afficher_curseur();
}


//--------------------------- Quitter le programme ---------------------------
byte Bouton_Quitter_Routine_locale(void)
{
  short Bouton_clicke;
  //byte  Enregistrer;
  static char  Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  byte  Ancienne_forme_curseur;

  if (!Principal_Image_modifiee)
    return 1;

  // On commence par afficher la fenêtre de QUIT
  Ouvrir_fenetre(160,84,"Quit ?");
  Fenetre_Definir_bouton_normal(20,20,120,14,"Stay",0,1,SDLK_ESCAPE);          // 1
  Fenetre_Definir_bouton_normal(20,40,120,14,"Save & quit",1,1,SDLK_s);   // 2
  Fenetre_Definir_bouton_normal(20,60,120,14,"Discard (Quit)",1,1,SDLK_d);// 3
  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke<=0);
  Attendre_fin_de_click();

  Fermer_fenetre();
  Afficher_curseur();

  switch(Bouton_clicke)
  {
    case 1 : return 0; // Rester
    case 2 : // Sauver et enregistrer
             Nom_fichier_complet(Nom_du_fichier,0);
             if ( (!Fichier_existe(Nom_du_fichier)) || Demande_de_confirmation("Erase old file ?") )
             {
               Effacer_curseur();
               Ancienne_forme_curseur=Forme_curseur;
               Forme_curseur=FORME_CURSEUR_SABLIER;
               Afficher_curseur();

               Sauver_image(1);

               Effacer_curseur();
               Forme_curseur=Ancienne_forme_curseur;
               Afficher_curseur();

               if (!Erreur_fichier)
                 // L'ayant sauvée avec succès,
                 return 1; // On peut quitter
               else
                 // Il y a eu une erreur lors de la sauvegarde,
                 return 0; // On ne peut donc pas quitter
             }
             else
               // L'utilisateur ne veut pas écraser l'ancien fichier,
               return 0; // On doit donc rester
    case 3 : return 1; // Quitter
  }
  return 0;
}


void Bouton_Quit(void)
{
  //short Bouton_clicke;

  if (Bouton_Quitter_Routine_locale())
  {
    if (Brouillon_Image_modifiee)
    {
      Bouton_Page(); // On passe sur le brouillon
      // Si l'utilisateur présente les derniers symptomes de l'abandon
      if (Bouton_Quitter_Routine_locale())
        Sortir_du_programme=1;
    }
    else
      Sortir_du_programme=1;
  }

  if ( (Menu_visible) && (Mouse_Y+8>Menu_Ordonnee) )
    Effacer_curseur();

  Desenclencher_bouton(BOUTON_QUIT);

  if ( (Menu_visible) && (Mouse_Y+8>Menu_Ordonnee) )
    Afficher_curseur();
}


//---------------------------- Effacer l'écran -------------------------------
void Bouton_Clear(void)
{
  Effacer_curseur();
  Backup();
  if (Stencil_Mode && Config.Clear_with_stencil)
    Effacer_image_courante_Stencil(0,Stencil);
  else
    Effacer_image_courante(0);
  Afficher_ecran();
  Desenclencher_bouton(BOUTON_CLEAR);
  Afficher_curseur();
}

void Bouton_Clear_colore(void)
{
  Effacer_curseur();
  Backup();
  if (Stencil_Mode && Config.Clear_with_stencil)
    Effacer_image_courante_Stencil(Back_color,Stencil);
  else
    Effacer_image_courante(Back_color);
  Afficher_ecran();
  Desenclencher_bouton(BOUTON_CLEAR);
  Afficher_curseur();
}


//---------- Menu dans lequel on tagge des couleurs (genre Stencil) ----------
void Menu_Tag_couleurs(char * En_tete, byte * Table, byte * Mode, byte Cancel)
{
  short Bouton_clicke;
  byte Backup_table[256];
  word Indice;
  word Ancien_Mouse_X;
  word Ancien_Mouse_Y;
  byte Ancien_Mouse_K;
  byte Couleur_taggee;
  byte Couleur;
  byte Click;


  Ouvrir_fenetre(176,150,En_tete);

  Fenetre_Definir_bouton_palette(6,38);                            // 1
  Fenetre_Definir_bouton_normal( 7, 19,78,14,"Clear" ,1,1,SDLK_c); // 2
  Fenetre_Definir_bouton_normal(91, 19,78,14,"Invert",1,1,SDLK_i); // 3
  if (Cancel)
  {
    Fenetre_Definir_bouton_normal(91,129,78,14,"OK"    ,0,1,SDLK_RETURN); // 4
    Fenetre_Definir_bouton_normal( 7,129,78,14,"Cancel",0,1,SDLK_ESCAPE); // 5
    // On enregistre la table dans un backup au cas où on ferait Cancel
    memcpy(Backup_table,Table,256);
  }
  else
    Fenetre_Definir_bouton_normal(49,129,78,14,"OK"    ,0,1,SDLK_RETURN); // 4

  // On affiche l'état actuel de la table
  for (Indice=0; Indice<=255; Indice++)
    Stencil_Tagger_couleur(Indice, (Table[Indice])?CM_Noir:CM_Clair);

  Afficher_curseur();

  do
  {
    Ancien_Mouse_X=Mouse_X;
    Ancien_Mouse_Y=Mouse_Y;
    Ancien_Mouse_K=Mouse_K;

    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case  0 :
        break;
      case -1 :
      case  1 : // Palette
        if ( (Mouse_X!=Ancien_Mouse_X) || (Mouse_Y!=Ancien_Mouse_Y) || (Mouse_K!=Ancien_Mouse_K) )
        {
          Effacer_curseur();
          Couleur_taggee=Lit_pixel(Mouse_X,Mouse_Y);
          Table[Couleur_taggee]=(Mouse_K==A_GAUCHE);
          Stencil_Tagger_couleur(Couleur_taggee,(Mouse_K==A_GAUCHE)?CM_Noir:CM_Clair);
          Afficher_curseur();
        }
        break;
      case  2 : // Clear
        memset(Table,0,256);
        Effacer_curseur();
        for (Indice=0; Indice<=255; Indice++)
          Stencil_Tagger_couleur(Indice,CM_Clair);
        Afficher_curseur();
        break;
      case  3 : // Invert
        Effacer_curseur();
        for (Indice=0; Indice<=255; Indice++)
          Stencil_Tagger_couleur(Indice,(Table[Indice]^=1)?CM_Noir:CM_Clair);
        Afficher_curseur();
    }

    if (!Mouse_K)
    switch (Touche)
    {
      case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case SDLK_COMMA :
        Recuperer_couleur_derriere_fenetre(&Couleur,&Click);
        if (Click)
        {
          Effacer_curseur();
          Couleur_taggee=Couleur;
          Table[Couleur_taggee]=(Click==A_GAUCHE);
          Stencil_Tagger_couleur(Couleur_taggee,(Click==A_GAUCHE)?CM_Noir:CM_Clair);
          Afficher_curseur();
        }
    }
  }
  while (Bouton_clicke<4);

  Fermer_fenetre();

  if (Bouton_clicke==5) // Cancel
    memcpy(Table,Backup_table,256);
  else // OK
    *Mode=1;

  Afficher_curseur();
}


//--------------------------------- Stencil ----------------------------------
void Bouton_Stencil_Mode(void)
{
  Stencil_Mode=!Stencil_Mode;
}


void Stencil_Tagger_couleur(byte Couleur, byte Couleur_de_taggage)
{
  Block(Fenetre_Pos_X+(Menu_Facteur_X*(Fenetre_Liste_boutons_palette->Pos_X+4+(Couleur >> 4)*10)),
        Fenetre_Pos_Y+(Menu_Facteur_Y*(Fenetre_Liste_boutons_palette->Pos_Y+3+(Couleur & 15)* 5)),
        Menu_Facteur_X<<1,Menu_Facteur_Y*5,Couleur_de_taggage);
}


void Bouton_Menu_Stencil(void)
{
  Menu_Tag_couleurs("Stencil",Stencil,&Stencil_Mode,1);
}


//--------------------------------- Masque -----------------------------------
void Bouton_Mask_Mode(void)
{
  Mask_Mode=!Mask_Mode;
}


void Bouton_Mask_Menu(void)
{
  Menu_Tag_couleurs("Mask",Mask,&Mask_Mode,1);
}


//------------------------------- Paramètres ---------------------------------

void Settings_Afficher_config(struct S_Config * Conf)
#define YES "YES"
#define NO  " NO"
{
  struct Fenetre_Bouton_scroller * Jauge=Fenetre_Liste_boutons_scroller;
  char Chaine[4];

  Effacer_curseur();

  // Jauge = Jauge de sensibilité Y
  Jauge->Position=Conf->Indice_Sensibilite_souris_Y-1;
  Fenetre_Dessiner_jauge(Jauge);

  Jauge=Jauge->Next;
  // Jauge = Jauge de sensibilité X
  Jauge->Position=Conf->Indice_Sensibilite_souris_X-1;
  Fenetre_Dessiner_jauge(Jauge);

  Print_dans_fenetre(273, 31,(Conf->Lire_les_fichiers_caches)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(273, 46,(Conf->Lire_les_repertoires_caches)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(273, 61,(Conf->Lire_les_repertoires_systemes)?YES:NO,CM_Noir,CM_Clair);

  Print_dans_fenetre(223, 84,(Conf->Safety_colors)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(223, 99,(Conf->Adjust_brush_pick)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(223,114,(Conf->Couleurs_separees)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(223,129,(Conf->Auto_set_res)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre(183,144,(Conf->Coords_rel)?"Relative":"Absolute",CM_Noir,CM_Clair);

  Print_dans_fenetre( 91, 84,(Conf->Afficher_limites_image)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre( 91, 99,(Conf->Clear_palette)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre( 91,114,(Conf->Maximize_preview)?YES:NO,CM_Noir,CM_Clair);
  Print_dans_fenetre( 91,129,(Conf->Backup)?YES:NO,CM_Noir,CM_Clair);
  switch (Conf->Curseur)
  {
    case 0 : Print_dans_fenetre(67,144," Solid",CM_Noir,CM_Clair); break;
    case 1 : Print_dans_fenetre(67,144,"Transp",CM_Noir,CM_Clair); break;
    default: Print_dans_fenetre(67,144,"  Thin",CM_Noir,CM_Clair);
  }

  if (Conf->Fonte)
  { // Fun
    Print_dans_fenetre(  8,31," ",CM_Noir,CM_Clair);
    Print_dans_fenetre( 78,31," ",CM_Noir,CM_Clair);
    Print_dans_fenetre( 82,31,"\020",CM_Noir,CM_Clair);
    Print_dans_fenetre(152,31,"\021",CM_Noir,CM_Clair);
  }
  else
  { // Classic
    Print_dans_fenetre( 82,31," ",CM_Noir,CM_Clair);
    Print_dans_fenetre(152,31," ",CM_Noir,CM_Clair);
    Print_dans_fenetre(  8,31,"\020",CM_Noir,CM_Clair);
    Print_dans_fenetre( 78,31,"\021",CM_Noir,CM_Clair);
  }

  Print_dans_fenetre(155,166,(Conf->Auto_save)?YES:NO,CM_Noir,CM_Clair);

  Num2str(Conf->Nb_pages_Undo,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);

  Afficher_curseur();
}

void Settings_Sauver_config(struct S_Config * Conf)
{
  if (Sauver_CFG())
    Erreur(0);
  else
    if (Sauver_INI(Conf))
      Erreur(0);
}

void Settings_Charger_config(struct S_Config * Conf)
{
  if (Charger_CFG(0))
    Erreur(0);
  else
    if (Charger_INI(Conf))
      Erreur(0);
}

void Bouton_Settings(void)
{
  int Sensibilite_X;
  int Sensibilite_Y;
  short Bouton_clicke;
  struct S_Config Config_choisie;
  char Chaine[3];
  byte On_a_recharge_la_config=0;

  Config_choisie=Config;

  Ouvrir_fenetre(307,182,"Settings");

  // On commence par dessiner tous les Cadres
  Fenetre_Afficher_cadre(  5, 16,157,30);
  Fenetre_Afficher_cadre(  5, 47,157,17);
  Fenetre_Afficher_cadre(163, 16,139,60);
  Fenetre_Afficher_cadre(253, 77, 49,82);
  Fenetre_Afficher_cadre(  5, 77,247,82); // |_ Misc.
  Fenetre_Afficher_cadre(  5, 65,157,14); // |
  // On découpe le Cadre bizarre des "Miscellaneous"
  Pixel_dans_fenetre(6,77,CM_Blanc);
  Pixel_dans_fenetre(5,78,CM_Fonce);
  Block(Fenetre_Pos_X+(7*Menu_Facteur_X),Fenetre_Pos_Y+(77*Menu_Facteur_Y),
        Menu_Facteur_X*154,Menu_Facteur_Y<<1,CM_Clair);
  Pixel_dans_fenetre(161,77,CM_Clair);
  Pixel_dans_fenetre(160,77,CM_Fonce);

  // On affiche maintenant tout le blabla
  Print_dans_fenetre( 69, 19,"Font"            ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(169, 19,"Show in filelist",CM_Fonce,CM_Clair);
  Print_dans_fenetre(  9, 52,"Nb of UNDO pages",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 32, 70,"Miscellaneous"   ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(258, 80,"Mouse"           ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(258, 88,"Sens."           ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(256,123,"X"               ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(292,123,"Y"               ,CM_Fonce,CM_Clair);

  // Boutons de fontes
  Fenetre_Definir_bouton_normal(17,28,59,14,"Classic",0,1,SDLK_LAST); // 1
  Fenetre_Definir_bouton_normal(91,28,59,14,"Fun"    ,0,1,SDLK_LAST); // 2

  // Bouton Show/Hide dans le fileselect
  Fenetre_Definir_bouton_normal(167, 28,131,14,"Hidden files:   ",0,1,SDLK_LAST); // 3
  Fenetre_Definir_bouton_normal(167, 43,131,14,"Hidden dir. :   ",0,1,SDLK_LAST); // 4
  Fenetre_Definir_bouton_normal(167, 58,131,14,"System dir. :   ",0,1,SDLK_LAST); // 5

  // Bouton Show/Hide Picture limits
  Fenetre_Definir_bouton_normal(  9, 81,107,14,"Limits   :   ",0,1,SDLK_LAST); // 6
  // Bouton Show/Hide Picture limits
  Fenetre_Definir_bouton_normal(  9, 96,107,14,"Clear pal:   ",0,1,SDLK_LAST); // 7
  // Bouton Show/Hide Picture limits
  Fenetre_Definir_bouton_normal(  9,111,107,14,"Max prev.:   ",0,1,SDLK_LAST); // 8
  // Bouton Effectuer des backups à chaque sauvegarde
  Fenetre_Definir_bouton_normal(  9,126,107,14,"Backup   :   ",0,1,SDLK_LAST); // 9
  // Bouton Choix du curseur
  Fenetre_Definir_bouton_normal(  9,141,107,14,"Cursor:      ",0,1,SDLK_LAST); // 10

  // Bouton Safety colors
  Fenetre_Definir_bouton_normal(117, 81,131,14,"Safe. colors:   ",0,1,SDLK_LAST); // 11
  // Bouton Adjust Brush Pick
  Fenetre_Definir_bouton_normal(117, 96,131,14,"AdjBrushPick:   ",0,1,SDLK_LAST); // 12
  // Bouton Separate colors
  Fenetre_Definir_bouton_normal(117,111,131,14,"Separate col:   ",0,1,SDLK_LAST); // 13
  // Bouton Passer dans la résolution appropriée après un chargement
  Fenetre_Definir_bouton_normal(117,126,131,14,"Auto-set res:   ",0,1,SDLK_LAST); // 14
  // Bouton Adapter la palette après un chargement (<=> Shift+BkSpc)
  Fenetre_Definir_bouton_normal(117,141,131,14,"Coords:         ",0,1,SDLK_LAST); // 15

    // Bouton Reload
  Fenetre_Definir_bouton_normal(  6,163, 51,14,"Reload"       ,0,1,SDLK_LAST); // 16
    // Bouton Auto-save
  Fenetre_Definir_bouton_normal( 73,163,107,14,"Auto-save:   ",0,1,SDLK_LAST); // 17
    // Bouton Save
  Fenetre_Definir_bouton_normal(183,163, 51,14,"Save"         ,0,1,SDLK_LAST); // 18
    // Bouton Close
  Fenetre_Definir_bouton_normal(250,163, 51,14,"Close"        ,0,1,SDLK_ESCAPE); // 19

  // Jauges de sensibilité de la souris (X puis Y)
  Fenetre_Definir_bouton_scroller(265,99,56,255,1,0); // 20
  Fenetre_Definir_bouton_scroller(279,99,56,255,1,0); // 21

  // Zone de saisie du nb de pages de Undo
  Fenetre_Definir_bouton_saisie(140,50,2);           // 22

  SDL_UpdateRect(Ecran_SDL,(Largeur_ecran-(307*Menu_Facteur_X))>>1,(Hauteur_ecran-(182*Menu_Facteur_Y))>>1,307,182);

  Afficher_curseur();

  Settings_Afficher_config(&Config_choisie);


  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch(Bouton_clicke)
    {
      case  1 : // Classic
        Config_choisie.Fonte=0;
        break;
      case  2 : // Fun
        Config_choisie.Fonte=1;
        break;
      case  3 : // Hidden files
        Config_choisie.Lire_les_fichiers_caches=(Config_choisie.Lire_les_fichiers_caches)?0:-1;
        break;
      case  4 : // Hidden dir.
        Config_choisie.Lire_les_repertoires_caches=(Config_choisie.Lire_les_repertoires_caches)?0:-1;
        break;
      case  5 : // System dir.
        Config_choisie.Lire_les_repertoires_systemes=(Config_choisie.Lire_les_repertoires_systemes)?0:-1;
        break;
      case  6 : // Draw limits
        Config_choisie.Afficher_limites_image=!Config_choisie.Afficher_limites_image;
        break;
      case  7 : // Clear palette
        Config_choisie.Clear_palette=!Config_choisie.Clear_palette;
        break;
      case  8 : // Maximize preview
        Config_choisie.Maximize_preview=!Config_choisie.Maximize_preview;
        break;
      case  9 : // Backup
        Config_choisie.Backup=!Config_choisie.Backup;
        break;
      case 10 : // Curseur
        Config_choisie.Curseur=(Config_choisie.Curseur+1)%3;
        break;
      case 11 : // Safety colors
        Config_choisie.Safety_colors=!Config_choisie.Safety_colors;
        break;
      case 12 : // Adjust brush pick
        Config_choisie.Adjust_brush_pick=!Config_choisie.Adjust_brush_pick;
        break;
      case 13 : // Separate colors
        Config_choisie.Couleurs_separees=!Config_choisie.Couleurs_separees;
        break;
      case 14 : // Auto-set resolution
        Config_choisie.Auto_set_res=!Config_choisie.Auto_set_res;
        break;
      case 15 : // Coordonnées
        Config_choisie.Coords_rel=!Config_choisie.Coords_rel;
        break;
      case 16 : // Reload
        Settings_Charger_config(&Config_choisie);
        On_a_recharge_la_config=1;
        break;
      case 17 : // Auto-save
        Config_choisie.Auto_save=!Config_choisie.Auto_save;
        break;
      case 18 : // Save
        Settings_Sauver_config(&Config_choisie);
        break;
      case 20 : // X Sensib.
        Config_choisie.Indice_Sensibilite_souris_X=Fenetre_Attribut2+1;
        break;
      case 21 : // Y Sensib.
        Config_choisie.Indice_Sensibilite_souris_Y=Fenetre_Attribut2+1;
        break;
      case 22 : // Nb pages Undo
        Effacer_curseur();
        Num2str(Config_choisie.Nb_pages_Undo,Chaine,2);
        Readline(142,52,Chaine,2,1);
        Config_choisie.Nb_pages_Undo=atoi(Chaine);
        // On corrige la valeur
        if (Config_choisie.Nb_pages_Undo>NB_PAGES_UNDO_MAX)
        {
          Config_choisie.Nb_pages_Undo=NB_PAGES_UNDO_MAX;
          Num2str(Config_choisie.Nb_pages_Undo,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
        }
        else if (!Config_choisie.Nb_pages_Undo)
        {
          Config_choisie.Nb_pages_Undo=1;
          Num2str(Config_choisie.Nb_pages_Undo,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
        }
        Afficher_curseur();
    }

    if ((Bouton_clicke>=3) && (Bouton_clicke<=5))
    {
      Principal_File_list_Position=0;
      Principal_File_list_Decalage=0;
      Brouillon_File_list_Position=0;
      Brouillon_File_list_Decalage=0;
    }

    if ((Bouton_clicke>=1) && (Bouton_clicke<=17))
      Settings_Afficher_config(&Config_choisie);
  }
  while ( (Bouton_clicke!=19) && (Touche!=SDLK_RETURN) );

  Config=Config_choisie;

  // Prise en compte de la nouvelle config
    // Gestion de la sensibilité
  Sensibilite_X=(Config.Indice_Sensibilite_souris_X/Menu_Facteur_X);
  Sensibilite_Y=(Config.Indice_Sensibilite_souris_Y/Menu_Facteur_Y);
  Sensibilite_X>>=Mouse_Facteur_de_correction_X;
  Sensibilite_Y>>=Mouse_Facteur_de_correction_Y;

  Sensibilite_souris(Sensibilite_X?Sensibilite_X:1,
                     Sensibilite_Y?Sensibilite_Y:1);
    // Gestion des fontes
  if (Config.Fonte)
    Fonte=Fonte_fun;
  else
    Fonte=Fonte_systeme;

  if (On_a_recharge_la_config)
    Calculer_couleurs_menu_optimales(Principal_Palette);

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_PARAMETRES);
  // Raffichage du menu pour que les inscriptions qui y figurent soient retracées avec la nouvelle fonte
  Afficher_menu();
  Afficher_curseur();

  // On vérifie qu'on peut bien allouer le nombre de pages Undo.
  Nouveau_nombre_de_backups(Config.Nb_pages_Undo);
}


//---------------------------- Changement de page ----------------------------
void Bouton_Page(void)
{
  byte   Octet_temporaire;
  word   Mot_temporaire;
  short  Short_temporaire;
  float  Float_temporaire;
  char   Zone_temporaire[256];

  Effacer_curseur();

  // On dégrossit le travail avec les infos des listes de pages
  Interchanger_image_principale_et_brouillon();

  // On fait le reste du travail "à la main":
  Short_temporaire=Brouillon_Decalage_X;
  Brouillon_Decalage_X=Principal_Decalage_X;
  Principal_Decalage_X=Short_temporaire;

  Short_temporaire=Brouillon_Decalage_Y;
  Brouillon_Decalage_Y=Principal_Decalage_Y;
  Principal_Decalage_Y=Short_temporaire;

  Short_temporaire=Ancien_Brouillon_Decalage_X;
  Ancien_Brouillon_Decalage_X=Ancien_Principal_Decalage_X;
  Ancien_Principal_Decalage_X=Short_temporaire;

  Short_temporaire=Ancien_Brouillon_Decalage_Y;
  Ancien_Brouillon_Decalage_Y=Ancien_Principal_Decalage_Y;
  Ancien_Principal_Decalage_Y=Short_temporaire;

  Short_temporaire=Brouillon_Split;
  Brouillon_Split=Principal_Split;
  Principal_Split=Short_temporaire;

  Short_temporaire=Brouillon_X_Zoom;
  Brouillon_X_Zoom=Principal_X_Zoom;
  Principal_X_Zoom=Short_temporaire;

  Float_temporaire=Brouillon_Proportion_split;
  Brouillon_Proportion_split=Principal_Proportion_split;
  Principal_Proportion_split=Float_temporaire;

  Octet_temporaire=Brouillon_Loupe_Mode;
  Brouillon_Loupe_Mode=Loupe_Mode;
  Loupe_Mode=Octet_temporaire;

  Pixel_Preview=(Loupe_Mode)?Pixel_Preview_Loupe:Pixel_Preview_Normal;

  Mot_temporaire=Brouillon_Loupe_Facteur;
  Brouillon_Loupe_Facteur=Loupe_Facteur;
  Loupe_Facteur=Mot_temporaire;

  Mot_temporaire=Brouillon_Loupe_Hauteur;
  Brouillon_Loupe_Hauteur=Loupe_Hauteur;
  Loupe_Hauteur=Mot_temporaire;

  Mot_temporaire=Brouillon_Loupe_Largeur;
  Brouillon_Loupe_Largeur=Loupe_Largeur;
  Loupe_Largeur=Mot_temporaire;

  Short_temporaire=Brouillon_Loupe_Decalage_X;
  Brouillon_Loupe_Decalage_X=Loupe_Decalage_X;
  Loupe_Decalage_X=Short_temporaire;

  Short_temporaire=Brouillon_Loupe_Decalage_Y;
  Brouillon_Loupe_Decalage_Y=Loupe_Decalage_Y;
  Loupe_Decalage_Y=Short_temporaire;

  // Swap du booléen "Image modifiée"
  Octet_temporaire        =Brouillon_Image_modifiee;
  Brouillon_Image_modifiee=Principal_Image_modifiee;
  Principal_Image_modifiee=Octet_temporaire;

  // Swap des infos sur les fileselects
  strcpy(Zone_temporaire             ,Brouillon_Repertoire_courant);
  strcpy(Brouillon_Repertoire_courant,Principal_Repertoire_courant);
  strcpy(Principal_Repertoire_courant,Zone_temporaire             );

  Octet_temporaire=Brouillon_Format;
  Brouillon_Format=Principal_Format;
  Principal_Format=Octet_temporaire;

  Mot_temporaire              =Brouillon_File_list_Position;
  Brouillon_File_list_Position=Principal_File_list_Position;
  Principal_File_list_Position=Mot_temporaire;

  Mot_temporaire              =Brouillon_File_list_Decalage;
  Brouillon_File_list_Decalage=Principal_File_list_Decalage;
  Principal_File_list_Decalage=Mot_temporaire;

  // A la fin, on affiche l'écran
  for (Octet_temporaire=0; FACTEUR_ZOOM[Octet_temporaire]!=Loupe_Facteur; Octet_temporaire++);
  Changer_facteur_loupe(Octet_temporaire);

  Set_palette(Principal_Palette);
  Calculer_couleurs_menu_optimales(Principal_Palette);
  Afficher_ecran();
  Desenclencher_bouton(BOUTON_PAGE);
  Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,Loupe_Mode);
  Afficher_menu();

  Afficher_curseur();
}


// -- Copie de page ---------------------------------------------------------

void Copier_image_seule(void)
{
  if (Backuper_et_redimensionner_brouillon(Principal_Largeur_image,Principal_Hauteur_image))
  {
    // copie de l'image
    memcpy(Brouillon_Ecran,Principal_Ecran,Principal_Largeur_image*Principal_Hauteur_image);

    // Copie des dimensions de l'image
    /*
      C'est inutile, le "Backuper et redimensionner brouillon" a déjà modifié
      ces valeurs pour qu'elles soient correctes.
    */
    Brouillon_Largeur_image=Principal_Largeur_image;
    Brouillon_Hauteur_image=Principal_Hauteur_image;

    // Copie des décalages de la fenêtre principale (non zoomée) de l'image
    Brouillon_Decalage_X=Principal_Decalage_X;
    Brouillon_Decalage_Y=Principal_Decalage_Y;

    // Copie du booléen "Mode loupe" de l'image
    Brouillon_Loupe_Mode=Loupe_Mode;

    // Copie du facteur de zoom du brouillon
    Brouillon_Loupe_Facteur=Loupe_Facteur;

    // Copie des dimensions de la fenêtre de zoom
    Brouillon_Loupe_Largeur=Loupe_Largeur;
    Brouillon_Loupe_Hauteur=Loupe_Hauteur;

    // Copie des décalages de la fenêtre de zoom
    Brouillon_Loupe_Decalage_X=Loupe_Decalage_X;
    Brouillon_Loupe_Decalage_Y=Loupe_Decalage_Y;

    // Copie des données du split du zoom
    Brouillon_Split=Principal_Split;
    Brouillon_X_Zoom=Principal_X_Zoom;
    Brouillon_Proportion_split=Principal_Proportion_split;
  }
  else
    Message_Memoire_insuffisante();
}


void Copier_certaines_couleurs(void)
{
  short Indice;

  Menu_Tag_couleurs("Tag colors to copy",Masque_copie_couleurs,NULL,0);

  if ( (!Brouillon_Image_modifiee)
    || (Demande_de_confirmation("Spare page was modified. Proceed?")) )
    for (Indice=0; Indice<256; Indice++)
    {
      if (Masque_copie_couleurs[Indice])
        memcpy(Brouillon_Palette+Indice,Principal_Palette+Indice,
               sizeof(struct Composantes));
    }
}


void Bouton_Copy_page(void)
{
  short Bouton_clicke;


  Ouvrir_fenetre(168,137,"Copy to spare page");

  Fenetre_Definir_bouton_normal(10, 20,148,14,"Pixels + palette" , 0,1,SDLK_RETURN); // 1
  Fenetre_Definir_bouton_normal(10, 37,148,14,"Pixels only"      , 3,1,SDLK_x); // 2
  Fenetre_Definir_bouton_normal(10, 54,148,14,"Palette only"     , 1,1,SDLK_p); // 3
  Fenetre_Definir_bouton_normal(10, 71,148,14,"Some colors only" , 6,1,SDLK_c); // 4
  Fenetre_Definir_bouton_normal(10, 88,148,14,"Palette and remap",13,1,SDLK_r); // 5
  Fenetre_Definir_bouton_normal(44,114, 80,14,"Cancel"           , 0,1,SDLK_ESCAPE); // 6

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke<=0);

  Fermer_fenetre();
  Afficher_curseur();

  if (Bouton_clicke!=6)
  {
    if (Bouton_clicke==4)
      Copier_certaines_couleurs();
    else
    {
      if ( (!Brouillon_Image_modifiee)
        || (Demande_de_confirmation("Spare page was modified. Proceed?")) )
      {
        if (Bouton_clicke<=2)
          Copier_image_seule();

        if (Bouton_clicke==5)
          Remap_picture();

        if (Bouton_clicke!=2) // copie de la palette
          memcpy(Brouillon_Palette,Principal_Palette,sizeof(T_Palette));

        Brouillon_Image_modifiee=1;
      }
    }
  }

  Effacer_curseur();
  Desenclencher_bouton(BOUTON_PAGE);
  Afficher_curseur();
}


// -- Suppression d'une page -------------------------------------------------
void Bouton_Kill(void)
{
  if ( (Principal_Backups->Nb_pages_allouees==1)
    || (!Demande_de_confirmation("Delete the current page?")) )
  {
    if (Principal_Backups->Nb_pages_allouees==1)
      Warning_message("You can't delete the last page.");
    Effacer_curseur();
    Desenclencher_bouton(BOUTON_KILL);
    Afficher_curseur();
  }
  else
  {
    Effacer_curseur();
    Detruire_la_page_courante();

    Set_palette(Principal_Palette);
    Calculer_couleurs_menu_optimales(Principal_Palette);

    Afficher_ecran();
    Desenclencher_bouton(BOUTON_KILL);
    Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,Loupe_Mode);
    Afficher_menu();
    Afficher_curseur();
  }
}


//------------------------- Dimensions Image/Ecran ---------------------------

void Cocher_bouton_mode(short Pos_X, short Pos_Y, byte Etat)
{
  byte Couleur;

  if (Etat>=128)
    Etat-=128;
  switch (Etat)
  {
    case 0 : Couleur=CM_Blanc; break;
    case 1 : Couleur=CM_Clair; break;
    case 2 : Couleur=CM_Fonce; break;
    default: Couleur=CM_Noir;
  }
  Block(Fenetre_Pos_X+Menu_Facteur_X*Pos_X,Fenetre_Pos_Y+Menu_Facteur_Y*Pos_Y,
        Menu_Facteur_X*9,Menu_Facteur_Y*3,Couleur);
        
  SDL_UpdateRect(Ecran_SDL,Fenetre_Pos_X+Menu_Facteur_X*Pos_X,Fenetre_Pos_Y+Menu_Facteur_Y*Pos_Y,
        Menu_Facteur_X*9,Menu_Facteur_Y*3);
}


void Afficher_liste_modes(short Debut_liste, short Position_curseur)
{
  short Indice,Mode_courant;
  short Pos_Y;
  byte  Couleur_texte,Couleur_fond;
  char Chaine[29];
  char *Ratio;

  for (Mode_courant=Debut_liste,Indice=0; Indice<12; Indice++,Mode_courant++)
  {
    Pos_Y=70+(Indice<<3);
    Cocher_bouton_mode(19,Pos_Y+2,Mode_video[Mode_courant].Etat);

    if (Position_curseur!=Indice)
    {
      Couleur_fond =CM_Noir;
      if (Mode_video[Mode_courant].Etat<128)
        Couleur_texte=CM_Clair;
      else
        Couleur_texte=CM_Fonce;
    }
    else
    {
      Couleur_fond =CM_Fonce;
      if (Mode_video[Mode_courant].Etat<128)
        Couleur_texte=CM_Blanc;
      else
        Couleur_texte=CM_Clair;
    }
    Num2str(Mode_video[Mode_courant].Largeur,Chaine,4);
    Num2str(Mode_video[Mode_courant].Hauteur,Chaine+4,4);

    if(Mode_video[Mode_courant].Fullscreen == 1)
	    memcpy(Chaine+8,"  Fullscreen  ",15);
    else
	    memcpy(Chaine+8,"    Window    ",15);

    if (Mode_video[Mode_courant].Largeur*3 == Mode_video[Mode_courant].Hauteur*4)
      Ratio=" 4/3  ";
    else if (Mode_video[Mode_courant].Largeur*9 == Mode_video[Mode_courant].Hauteur*16)
      Ratio="16/9  ";
    else if (Mode_video[Mode_courant].Largeur*10 == Mode_video[Mode_courant].Hauteur*16)
      Ratio="16/10 ";
    else
      Ratio="      ";
    strcat(Chaine,Ratio);

    Print_dans_fenetre(39,Pos_Y,Chaine,Couleur_texte,Couleur_fond);
  }
}


void Scroller_la_liste_des_modes(short Debut_liste, short Position_curseur, int * Mode_choisi)
{
  Effacer_curseur();
  *Mode_choisi=Debut_liste+Position_curseur;
  if (Fenetre_Liste_boutons_scroller->Position!=Debut_liste)
  {
    Fenetre_Liste_boutons_scroller->Position=Debut_liste;
    Fenetre_Dessiner_jauge(Fenetre_Liste_boutons_scroller);
  }
  Afficher_liste_modes(Debut_liste,Position_curseur);
  Afficher_curseur();
}


void Bouton_Resol(void)
{
  short Bouton_clicke;
  int   Mode_choisi;
  word  Largeur_choisie;
  word  Hauteur_choisie;
  short Debut_liste;
  short Position_curseur;
  short Temp;
  byte  Temp2;
  char  Chaine[5];
  struct Fenetre_Bouton_special * Bouton_saisie_Width, * Bouton_saisie_Height;

  Ouvrir_fenetre(299,190,"Picture & screen sizes");

  Fenetre_Afficher_cadre      ( 8,17,195, 33);
  Fenetre_Afficher_cadre      ( 8,56,283,126);
  Fenetre_Afficher_cadre_creux(37,68,228,100);
  Block(Fenetre_Pos_X+Menu_Facteur_X*38,Fenetre_Pos_Y+Menu_Facteur_Y*69,
        Menu_Facteur_X*226,Menu_Facteur_Y*98,CM_Noir);

  Print_dans_fenetre( 12, 21,"Picture size:"   ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 12, 37,"Width:"          ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(108, 37,"Height:"         ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 16, 60,"OK"              ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 55, 60,"X   Y"           ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(120, 60,"Win / Full"      ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(219, 60,"Ratio"           ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 30,170,"\03"             ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 62,170,"OK"              ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(102,170,"Imperfect"       ,CM_Fonce,CM_Clair);
  Print_dans_fenetre(196,170,"Unsupported"     ,CM_Fonce,CM_Clair);

  Fenetre_Definir_bouton_normal(223, 18,67,14,"OK"      ,0,1,SDLK_RETURN); // 1
  Fenetre_Definir_bouton_normal(223, 35,67,14,"Cancel"  ,0,1,SDLK_ESCAPE); // 2

  Fenetre_Definir_bouton_saisie( 60, 35,4);                           // 3
  Bouton_saisie_Width=Fenetre_Liste_boutons_special;
  Fenetre_Definir_bouton_saisie(164, 35,4);                           // 4
  Bouton_saisie_Height=Fenetre_Liste_boutons_special;

  Fenetre_Definir_bouton_special(38,70,225,96);                       // 5

  Mode_choisi=Resolution_actuelle;

  if (Mode_choisi>=6)
  {
    if (Mode_choisi<NB_MODES_VIDEO-6)
    {
      Debut_liste=Mode_choisi-5;
      Position_curseur=5;
    }
    else
    {
      Debut_liste=NB_MODES_VIDEO-12;
      Position_curseur=Mode_choisi-Debut_liste;
    }
  }
  else
  {
    Debut_liste=0;
    Position_curseur=Mode_choisi;
  }

  Fenetre_Definir_bouton_scroller(271,69,97,NB_MODES_VIDEO,12,Debut_liste); // 6

  // Les 12 petits boutons indiquant l'état des modes
  for (Temp=0; Temp<12; Temp++)
    Fenetre_Definir_bouton_normal(17,70+(Temp<<3),13,7,"",0,1,SDLK_LAST);// 7..18

  Fenetre_Dessiner_bouton_normal( 16,170,13,7,"",0,0);
  Cocher_bouton_mode( 18,172,0);
  Fenetre_Dessiner_bouton_normal( 48,170,13,7,"",0,0);
  Cocher_bouton_mode( 50,172,1);
  Fenetre_Dessiner_bouton_normal( 88,170,13,7,"",0,0);
  Cocher_bouton_mode( 90,172,2);
  Fenetre_Dessiner_bouton_normal(182,170,13,7,"",0,0);
  Cocher_bouton_mode(184,172,3);

  Largeur_choisie=Principal_Largeur_image;
  Num2str(Largeur_choisie,Chaine,4);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_Width,Chaine);

  Hauteur_choisie=Principal_Hauteur_image;
  Num2str(Hauteur_choisie,Chaine,4);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_Height,Chaine);

  Afficher_liste_modes(Debut_liste,Position_curseur);

    SDL_UpdateRect(Ecran_SDL,(Largeur_ecran-(299*Menu_Facteur_X))>>1,(Hauteur_ecran-(190*Menu_Facteur_Y))>>1,299,190);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    { case -1: case 0: case 1: case 2:
        break;

      case 3 : // Largeur
        Effacer_curseur();
        Num2str(Largeur_choisie,Chaine,4);
        Readline(62,37,Chaine,4,1);
        Largeur_choisie=atoi(Chaine);
        // On corrige les dimensions
        if (Largeur_choisie==0)
        {
          Largeur_choisie=1;
          Num2str(Largeur_choisie,Chaine,4);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_Width,Chaine);
        }
        Afficher_curseur();
        break;

      case 4 : // Hauteur
        Effacer_curseur();
        Num2str(Hauteur_choisie,Chaine,4);
        Readline(166,37,Chaine,4,1);
        Hauteur_choisie=atoi(Chaine);
        // On corrige les dimensions
        if (Hauteur_choisie==0)
        {
          Hauteur_choisie=1;
          Num2str(Hauteur_choisie,Chaine,4);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_Height,Chaine);
        }
        Afficher_curseur();
        break;

      case 5: // Liste des modes
        Temp=(((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-70)>>3;
        if ((Mouse_K==2) || (Temp!=Position_curseur))
        {
          Effacer_curseur();
          if (Temp!=Position_curseur)
          {
            Position_curseur=Temp;
            Afficher_liste_modes(Debut_liste,Position_curseur);
          }
          Mode_choisi=Debut_liste+Position_curseur;
          // Si l'utilisateur s'est servi du bouton droit de la souris:
          if (Mouse_K==2)
          {
            // On affecte également les dimensions de l'image:
            Largeur_choisie=Mode_video[Mode_choisi].Largeur;
            Num2str(Largeur_choisie,Chaine,4);
            Fenetre_Contenu_bouton_saisie(Bouton_saisie_Width,Chaine);

            Hauteur_choisie=Mode_video[Mode_choisi].Hauteur;
            Num2str(Hauteur_choisie,Chaine,4);
            Fenetre_Contenu_bouton_saisie(Bouton_saisie_Height,Chaine);
          }
          Afficher_curseur();
        }
        Attendre_fin_de_click();
        break;

      case 6: // Scroller
        Debut_liste=Fenetre_Attribut2;
        Mode_choisi=Debut_liste+Position_curseur;
        Afficher_liste_modes(Debut_liste,Position_curseur);
        break;

      default: // Boutons de tag des états des modes
        Temp=Debut_liste+Bouton_clicke-7;
        if (Temp) // On n'a pas le droit de cocher le mode fenêtré
        {
          Temp2=(Mode_video[Temp].Etat & 0x80)?128:0;

          if (Fenetre_Attribut1==A_GAUCHE)
            Mode_video[Temp].Etat=Temp2+(((Mode_video[Temp].Etat&0x7F)+1)&3);
          else
            Mode_video[Temp].Etat=Temp2+(((Mode_video[Temp].Etat&0x7F)+3)&3);

          Effacer_curseur();
          Cocher_bouton_mode(19,16+(Bouton_clicke<<3),Mode_video[Temp].Etat);
          Afficher_curseur();
        }
    }

    // Gestion des touches de déplacement dans la liste
    switch (Touche)
    {
      case SDLK_UP : // Haut
        if (Position_curseur>0)
          Position_curseur--;
        else
          if (Debut_liste>0)
            Debut_liste--;
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
      case SDLK_DOWN : // Bas
        if (Position_curseur<11)
          Position_curseur++;
        else
          if (Debut_liste<NB_MODES_VIDEO-12)
            Debut_liste++;
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
      case SDLK_PAGEUP : // PageUp
        if (Position_curseur>0)
          Position_curseur=0;
        else
        {
          if (Debut_liste>11)
            Debut_liste-=11;
          else
            Debut_liste=0;
        }
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
      case SDLK_PAGEDOWN : // PageDown
        if (Position_curseur<11)
          Position_curseur=11;
        else
        {
          if (Debut_liste<NB_MODES_VIDEO-23)
            Debut_liste+=11;
          else
            Debut_liste=NB_MODES_VIDEO-12;
        }
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
      case SDLK_HOME : // Home
        Debut_liste=0;
        Position_curseur=0;
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
      case SDLK_END : // End
        Debut_liste=NB_MODES_VIDEO-12;
        Position_curseur=11;
        Scroller_la_liste_des_modes(Debut_liste,Position_curseur,&Mode_choisi);
        break;
    }

  }
  while ((Bouton_clicke!=1) && (Bouton_clicke!=2));

  Fermer_fenetre();

  if (Bouton_clicke==1) // OK
  {
    if (Loupe_Mode)
      Desenclencher_bouton(BOUTON_LOUPE);

    if ( (Largeur_choisie!=Principal_Largeur_image)
      || (Hauteur_choisie!=Principal_Hauteur_image) )
      Redimentionner_image(Largeur_choisie,Hauteur_choisie);

    if (Mode_video[Mode_choisi].Etat<=2)
      Initialiser_mode_video(
        Mode_video[Mode_choisi].Largeur,
        Mode_video[Mode_choisi].Hauteur,
        Mode_video[Mode_choisi].Fullscreen);
    else
    {
      Erreur(0); // On signale à l'utilisateur que c'est un mode invalide
      Initialiser_mode_video(
        Mode_video[Resolution_actuelle].Largeur,
        Mode_video[Resolution_actuelle].Hauteur,
        Mode_video[Resolution_actuelle].Fullscreen);
    }

    Afficher_menu();
    Afficher_ecran();
  }
  Mouse_X = Largeur_ecran >> 1;
  Mouse_Y = Hauteur_ecran >> 1;
  Set_mouse_position();


  Desenclencher_bouton(BOUTON_RESOL);
  Afficher_curseur();
}


void Bouton_Safety_resol(void)
{
  Effacer_curseur();

  Desenclencher_bouton(BOUTON_LOUPE);
  Initialiser_mode_video(640, 400, 0);
  Resolution_actuelle=0;
  Afficher_menu();
  Afficher_ecran();

  Desenclencher_bouton(BOUTON_RESOL);
  Afficher_curseur();
}


//------------------ Gestion des boutons de dessin à la main -----------------

void Bouton_Dessin(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(Mode_de_dessin_en_cours);
  Afficher_curseur();
}


void Bouton_Dessin_Switch_mode(void)
{
  Mode_de_dessin_en_cours++;
  if (Mode_de_dessin_en_cours>OPERATION_DESSIN_POINT)
    Mode_de_dessin_en_cours=OPERATION_DESSIN_CONTINU;

  Effacer_curseur();
  Afficher_sprite_dans_menu(BOUTON_DESSIN,Mode_de_dessin_en_cours);
  Demarrer_pile_operation(Mode_de_dessin_en_cours);
  Afficher_curseur();
}


// -- Gestion des boutons de rectangle vide et plein ------------------------

void Bouton_Rectangle_vide(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_RECTANGLE_VIDE);
  Afficher_curseur();
}


void Bouton_Rectangle_plein(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_RECTANGLE_PLEIN);
  Afficher_curseur();
}


// -- Gestion des boutons de cercle (ellipse) vide et plein(e) --------------

void Bouton_Cercle_vide(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_CERCLE_VIDE);
  Afficher_curseur();
}


void Bouton_Ellipse_vide(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_ELLIPSE_VIDE);
  Afficher_curseur();
}


void Bouton_Cercle_plein(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_CERCLE_PLEIN);
  Afficher_curseur();
}


void Bouton_Ellipse_pleine(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_ELLIPSE_PLEINE);
  Afficher_curseur();
}


// -- Gestion du menu des dégradés ------------------------------------------
void Degrade_Dessiner_bouton_de_technique(short Pos_X,short Pos_Y,int Technique)
{
  short Ligne;

  // On commence par afficher les 2 côtés qui constituent le dégradé de base:
    // Côté gauche (noir)
  Block(Fenetre_Pos_X+((Pos_X+2)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+2)*Menu_Facteur_Y),
        Menu_Facteur_X*6,
        Menu_Facteur_Y*10,CM_Noir);
    // Côté droit (blanc)
  Block(Fenetre_Pos_X+((Pos_X+8)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+2)*Menu_Facteur_Y),
        Menu_Facteur_X*5,
        Menu_Facteur_Y*10,CM_Blanc);

  switch(Technique)
  {
    case 1 : // Dégradé de trames simples
      // Au centre, on place 10 lignes tramées simplement
      for (Ligne=2;Ligne<2+10;Ligne++)
        if (Ligne&1)
        {
          // Lignes impaires
          Pixel_dans_fenetre(Pos_X+ 5,Pos_Y+Ligne,CM_Blanc);
          Pixel_dans_fenetre(Pos_X+ 7,Pos_Y+Ligne,CM_Blanc);
          Pixel_dans_fenetre(Pos_X+ 8,Pos_Y+Ligne,CM_Noir);
        }
        else
        {
          // Lignes paires
          Pixel_dans_fenetre(Pos_X+ 6,Pos_Y+Ligne,CM_Blanc);
          Pixel_dans_fenetre(Pos_X+ 9,Pos_Y+Ligne,CM_Noir);
        }
      break;
    case 2 : // Dégradé de trames étendues
      // Au centre, on place 10 lignes tramées de façon compliquée
      for (Ligne=2;Ligne<2+10;Ligne++)
        if (Ligne&1)
        {
          // Lignes impaires
          Pixel_dans_fenetre(Pos_X+ 7,Pos_Y+Ligne,CM_Blanc);
          Pixel_dans_fenetre(Pos_X+ 8,Pos_Y+Ligne,CM_Noir);
          Pixel_dans_fenetre(Pos_X+10,Pos_Y+Ligne,CM_Noir);
        }
        else
        {
          // Lignes paires
          Pixel_dans_fenetre(Pos_X+ 4,Pos_Y+Ligne,CM_Blanc);
          Pixel_dans_fenetre(Pos_X+ 6,Pos_Y+Ligne,CM_Blanc);
        }
  }
  
    SDL_UpdateRect(Ecran_SDL,Fenetre_Pos_X+((Pos_X+2)*Menu_Facteur_X),Fenetre_Pos_Y+((Pos_Y+2)*Menu_Facteur_Y),
                    Menu_Facteur_X*10,Menu_Facteur_Y*10);  
}

void Degrade_Charger_infos_du_tableau(int Indice)
{
  Degrade_Borne_Inferieure =Degrade_Tableau[Indice].Debut;
  Degrade_Borne_Superieure =Degrade_Tableau[Indice].Fin;
  Degrade_Inverse          =Degrade_Tableau[Indice].Inverse;
  Degrade_Melange_aleatoire=Degrade_Tableau[Indice].Melange+1;

  Degrade_Intervalle_bornes=(Degrade_Borne_Inferieure<Degrade_Borne_Superieure)?
                            Degrade_Borne_Superieure-Degrade_Borne_Inferieure:
                            Degrade_Borne_Inferieure-Degrade_Borne_Superieure;
  Degrade_Intervalle_bornes++;

  switch(Degrade_Tableau[Indice].Technique)
  {
    case 0 : // Degradé de base
      Traiter_degrade=Degrade_de_base;
      break;
    case 1 : // Dégradé de trames simples
      Traiter_degrade=Degrade_de_trames_simples;
      break;
    case 2 : // Dégradé de trames étendues
      Traiter_degrade=Degrade_de_trames_etendues;
  }
}

void Degrade_Dessiner_preview(short Debut_X,short Debut_Y,short Largeur,short Hauteur,int Indice)
{
  short Pos_X; // Variables de balayage du block en bas de l'écran.
  short Pos_Y;
  short Fin_X;
  short Fin_Y;

  Degrade_Charger_infos_du_tableau(Indice);

  Debut_X=Fenetre_Pos_X+(Debut_X*Menu_Facteur_X);
  Debut_Y=Fenetre_Pos_Y+(Debut_Y*Menu_Facteur_Y);

  Degrade_Intervalle_total=Largeur*Menu_Facteur_X;

  Fin_X=Debut_X+Degrade_Intervalle_total;
  Fin_Y=Debut_Y+(Hauteur*Menu_Facteur_Y);

  for (Pos_Y=Debut_Y;Pos_Y<Fin_Y;Pos_Y++)
    for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
      Traiter_degrade(Pos_X-Debut_X,Pos_X,Pos_Y);
  SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Largeur*Menu_Facteur_X,Hauteur*Menu_Facteur_Y);
}

void Bouton_Degrades(void)
{
  short Bouton_clicke;
  char  Chaine[3];
  struct T_Degrade_Tableau Backup_Degrade_Tableau[16];
  int   Ancien_Degrade_Courant;
  struct Fenetre_Bouton_scroller * Scroller_de_melange;
  short Ancien_Mouse_X;
  short Ancien_Mouse_Y;
  byte  Ancien_Mouse_K;
  byte  Couleur_temporaire;
  byte  Premiere_couleur;
  byte  Derniere_couleur;
  byte  Couleur;
  byte  Click;


  Traiter_pixel_de_degrade=Pixel;
  Ancien_Degrade_Courant=Degrade_Courant;
  memcpy(Backup_Degrade_Tableau,Degrade_Tableau,sizeof(struct T_Degrade_Tableau)*16);

  Ouvrir_fenetre(237,133,"Gradation menu");

  Fenetre_Definir_bouton_palette(48,21);                            // 1
    // Définition du scrolleur <=> indice du dégradé dans le tableau
  Fenetre_Definir_bouton_scroller(218,22,75,16,1,Degrade_Courant);  // 2
    // Définition du scrolleur de mélange du dégradé
  Fenetre_Definir_bouton_scroller(31,22,84,256,1,Degrade_Tableau[Degrade_Courant].Melange); // 3
  Scroller_de_melange=Fenetre_Liste_boutons_scroller;
    // Définition du bouton de sens
  Fenetre_Definir_bouton_normal(8,22,15,14,
    (Degrade_Tableau[Degrade_Courant].Inverse)?"\033":"\032",0,1,SDLK_TAB); // 4
    // Définition du bouton de technique
  Fenetre_Definir_bouton_normal(8,92,15,14,"",0,1,SDLK_TAB|MOD_SHIFT); // 5
  Degrade_Dessiner_bouton_de_technique(8,92,Degrade_Tableau[Degrade_Courant].Technique);

  Fenetre_Definir_bouton_normal(178,112,51,14,"OK",0,1,SDLK_RETURN);     // 6
  Fenetre_Definir_bouton_normal(123,112,51,14,"Cancel",0,1,SDLK_ESCAPE); // 7

  Print_dans_fenetre(5,60,"MIX",CM_Fonce,CM_Clair);

  // On tagge les couleurs qui vont avec
  Tagger_intervalle_palette(Degrade_Tableau[Degrade_Courant].Debut,Degrade_Tableau[Degrade_Courant].Fin);

  Num2str(Degrade_Courant+1,Chaine,2);
  Print_dans_fenetre(215,100,Chaine,CM_Noir,CM_Clair);

    // On affiche le cadre autour de la préview
  Fenetre_Afficher_cadre_creux(7,111,110,16);
    // On affiche la preview
  Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);

  Premiere_couleur=Derniere_couleur=(Degrade_Tableau[Degrade_Courant].Inverse)?Degrade_Tableau[Degrade_Courant].Fin:Degrade_Tableau[Degrade_Courant].Debut;

  Afficher_curseur();

  do
  {
    Ancien_Mouse_X=Mouse_X;
    Ancien_Mouse_Y=Mouse_Y;
    Ancien_Mouse_K=Mouse_K;

    Bouton_clicke=Fenetre_Bouton_clicke();

    switch(Bouton_clicke)
    {
      case -1 :
      case  1 : // Palette
        if ( (Mouse_X!=Ancien_Mouse_X) || (Mouse_Y!=Ancien_Mouse_Y) || (Mouse_K!=Ancien_Mouse_K) )
        {
          Effacer_curseur();
          Couleur_temporaire=Lit_pixel(Mouse_X,Mouse_Y);

          if (!Ancien_Mouse_K)
          {
            // On vient de clicker

            // On met à jour l'intervalle du dégradé
            Premiere_couleur=Derniere_couleur=Degrade_Tableau[Degrade_Courant].Debut=Degrade_Tableau[Degrade_Courant].Fin=Couleur_temporaire;
            // On tagge le bloc
            Tagger_intervalle_palette(Degrade_Tableau[Degrade_Courant].Debut,Degrade_Tableau[Degrade_Courant].Fin);
            // Tracé de la preview:
            Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);
          }
          else
          {
            // On maintient le click, on va donc tester si le curseur bouge
            if (Couleur_temporaire!=Derniere_couleur)
            {
              // On commence par ordonner la 1ère et dernière couleur du bloc
              if (Premiere_couleur<Couleur_temporaire)
              {
                Degrade_Tableau[Degrade_Courant].Debut=Premiere_couleur;
                Degrade_Tableau[Degrade_Courant].Fin  =Couleur_temporaire;
              }
              else if (Premiere_couleur>Couleur_temporaire)
              {
                Degrade_Tableau[Degrade_Courant].Debut=Couleur_temporaire;
                Degrade_Tableau[Degrade_Courant].Fin  =Premiere_couleur;
              }
              else
                Degrade_Tableau[Degrade_Courant].Debut=Degrade_Tableau[Degrade_Courant].Fin=Premiere_couleur;
              // On tagge le bloc
              Tagger_intervalle_palette(Degrade_Tableau[Degrade_Courant].Debut,Degrade_Tableau[Degrade_Courant].Fin);
              // Tracé de la preview:
              Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);
              Derniere_couleur=Couleur_temporaire;
            }
          }
          Afficher_curseur();
        }
        break;
      case  2 : // Nouvel indice de dégradé
        Effacer_curseur();
        // Nouvel indice dans Fenetre_Attribut2
        Degrade_Courant=Fenetre_Attribut2;

        // On affiche la valeur sous la jauge
        Num2str(Degrade_Courant+1,Chaine,2);
        Print_dans_fenetre(215,100,Chaine,CM_Noir,CM_Clair);

        // On tagge les couleurs qui vont avec
        Tagger_intervalle_palette(Degrade_Tableau[Degrade_Courant].Debut,Degrade_Tableau[Degrade_Courant].Fin);

        // On affiche le sens qui va avec
        Print_dans_fenetre(12,25,(Degrade_Tableau[Degrade_Courant].Inverse)?"\033":"\032",CM_Noir,CM_Clair);

        // On raffiche le mélange (jauge) qui va avec
        Scroller_de_melange->Position=Degrade_Tableau[Degrade_Courant].Melange;
        Fenetre_Dessiner_jauge(Scroller_de_melange);

        // On raffiche la technique qui va avec
        Degrade_Dessiner_bouton_de_technique(8,92,Degrade_Tableau[Degrade_Courant].Technique);

        // On affiche la nouvelle preview
        Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);

        Afficher_curseur();
        break;
      case  3 : // Nouveau mélange de dégradé
        Effacer_curseur();
        // Nouvel mélange dans Fenetre_Attribut2
        Degrade_Tableau[Degrade_Courant].Melange=Fenetre_Attribut2;
        // On affiche la nouvelle preview
        Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);
        Afficher_curseur();
        break;
      case  4 : // Changement de sens
        Effacer_curseur();
        // On inverse le sens (par un XOR de 1)
        Degrade_Tableau[Degrade_Courant].Inverse^=1;
        Print_dans_fenetre(12,25,(Degrade_Tableau[Degrade_Courant].Inverse)?"\033":"\032",CM_Noir,CM_Clair);
        // On affiche la nouvelle preview
        Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);
        Afficher_curseur();
        break;
      case  5 : // Changement de technique
        Effacer_curseur();
        // On change la technique par (+1)%3
        Degrade_Tableau[Degrade_Courant].Technique=(Degrade_Tableau[Degrade_Courant].Technique+1)%3;
        Degrade_Dessiner_bouton_de_technique(8,92,Degrade_Tableau[Degrade_Courant].Technique);
        // On affiche la nouvelle preview
        Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);
        Afficher_curseur();
    }

    if (!Mouse_K)
    switch (Touche)
    {
      case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case SDLK_COMMA :
        Recuperer_couleur_derriere_fenetre(&Couleur,&Click);
        if (Click)
        {
          Effacer_curseur();
          Couleur_temporaire=Couleur;

          // On met à jour l'intervalle du dégradé
          Premiere_couleur=Derniere_couleur=Degrade_Tableau[Degrade_Courant].Debut=Degrade_Tableau[Degrade_Courant].Fin=Couleur_temporaire;
          // On tagge le bloc
          Tagger_intervalle_palette(Degrade_Tableau[Degrade_Courant].Debut,Degrade_Tableau[Degrade_Courant].Fin);
          // Tracé de la preview:
          Degrade_Dessiner_preview(8,112,108,14,Degrade_Courant);

          Afficher_curseur();
        }
    }
  }
  while (Bouton_clicke<6);

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_GRADMENU);
  Afficher_curseur();

  Traiter_pixel_de_degrade=Afficher_pixel;
  if (Bouton_clicke==7) // Cancel
  {
    Degrade_Courant=Ancien_Degrade_Courant;
    memcpy(Degrade_Tableau,Backup_Degrade_Tableau,sizeof(struct T_Degrade_Tableau)*16);
    Degrade_Charger_infos_du_tableau(Degrade_Courant);
  }
}


// -- Gestion des boutons de cercle (ellipse) dégradé(e) --------------------

void Bouton_Cercle_degrade(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_CERCLE_DEGRADE);
  Afficher_curseur();
}


void Bouton_Ellipse_degrade(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_ELLIPSE_DEGRADEE);
  Afficher_curseur();
}


// -- Gestion du bouton de remplissage ---------------------------------------

void Bouton_Fill(void)
{
  if (Operation_en_cours!=OPERATION_FILL)
  {
    Effacer_curseur();
    if (Operation_en_cours!=OPERATION_REMPLACER)
    {
      Pinceau_Forme_avant_fill=Pinceau_Forme;
      Pinceau_Forme=FORME_PINCEAU_POINT;
    }
    else
      if ( (Mouse_Y<Menu_Ordonnee) && (Menu_visible) &&
           ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
        Print_dans_menu("X:       Y:             ",0);
    Demarrer_pile_operation(OPERATION_FILL);
    Afficher_curseur();
  }
}


void Bouton_Remplacer(void)
{
  if (Operation_en_cours!=OPERATION_REMPLACER)
  {
    Effacer_curseur();
    if (Operation_en_cours!=OPERATION_FILL)
    {
      Pinceau_Forme_avant_fill=Pinceau_Forme;
      Pinceau_Forme=FORME_PINCEAU_POINT;
    }
    if ( (Mouse_Y<Menu_Ordonnee) && (Menu_visible) &&
         ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
      Print_dans_menu("X:       Y:       (    )",0);
    Demarrer_pile_operation(OPERATION_REMPLACER);
    Afficher_curseur();
  }
}


void Bouton_desenclencher_Fill(void)
{
  Pinceau_Forme=Pinceau_Forme_avant_fill;

  if (Operation_en_cours==OPERATION_REMPLACER)
    if ( (Mouse_Y<Menu_Ordonnee) && (Menu_visible) &&
         ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
      Print_dans_menu("X:       Y:             ",0);
}


//---------------------------- Menu des pinceaux -----------------------------

void Bouton_Menu_pinceaux(void)
{
  short Bouton_clicke;
  short Pos_X,Pos_Y;
  byte Indice;

  Ouvrir_fenetre(310,155,"Paintbrush menu");

  Fenetre_Afficher_cadre(8,21,294,107);

  Fenetre_Definir_bouton_normal(122,133,67,14,"Cancel",0,1,SDLK_ESCAPE); // 1

  for (Indice=0; Indice<NB_SPRITES_PINCEAU; Indice++)
  {
    Pos_X=13+((Indice%12)*24);
    Pos_Y=27+((Indice/12)*25);
    Fenetre_Definir_bouton_normal(Pos_X  ,Pos_Y  ,20,20,"",0,1,SDLK_LAST);
    Afficher_pinceau_dans_fenetre(Pos_X+2,Pos_Y+2,Indice);
  }

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke<=0);

  Fermer_fenetre();

  if (Bouton_clicke!=1) // pas Cancel
  {
    Indice=Bouton_clicke-2;
    Pinceau_Forme=Pinceau_Type[Indice];
    Pinceau_Largeur=Pinceau_predefini_Largeur[Indice];
    Pinceau_Hauteur=Pinceau_predefini_Hauteur[Indice];
    Pinceau_Decalage_X=Pinceau_predefini_Decalage_X[Indice];
    Pinceau_Decalage_Y=Pinceau_predefini_Decalage_Y[Indice];
    for (Pos_Y=0; Pos_Y<Pinceau_Hauteur; Pos_Y++)
      for (Pos_X=0; Pos_X<Pinceau_Largeur; Pos_X++)
        Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X]=SPRITE_PINCEAU[Indice][Pos_Y][Pos_X];
    Changer_la_forme_du_pinceau(Pinceau_Type[Indice]);
  }

  Desenclencher_bouton(BOUTON_PINCEAUX);
  Afficher_curseur();
}


void Bouton_Brosse_monochrome(void)
{
  Effacer_curseur();
  // On passe en brosse monochrome:
  Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_MONOCHROME);

  Desenclencher_bouton(BOUTON_PINCEAUX);

  Afficher_curseur();
}

//------------------------ Chargements et sauvegardes ------------------------

void Print_repertoire_courant(void)
//
// Affiche Principal_Repertoire_courant sur 37 caractères
//
{
  char Nom_temporaire[TAILLE_MAXI_PATH+1]; // Nom tronqué
  int  Longueur; // Longueur du répertoire courant
  int  Indice;   // Indice de parcours de la chaine complète

  Block(Fenetre_Pos_X+(Menu_Facteur_X*7),Fenetre_Pos_Y+(Menu_Facteur_Y*43),Menu_Facteur_X*37*8,Menu_Facteur_Y<<3,CM_Clair);

  Longueur=strlen(Principal_Repertoire_courant);
  if (Longueur>TAILLE_MAXI_PATH)
  { // Doh! il va falloir tronquer le répertoire (bouh !)

    // On commence par copier bêtement les 3 premiers caractères (e.g. "C:\")
    for (Indice=0;Indice<3;Indice++)
      Nom_temporaire[Indice]=Principal_Repertoire_courant[Indice];

    // On y rajoute 3 petits points:
    strcpy(Nom_temporaire+3,"...");

    //  Ensuite, on cherche un endroit à partir duquel on pourrait loger tout
    // le reste de la chaine (Ouaaaaaah!!! Vachement fort le mec!!)
    for (Indice++;Indice<Longueur;Indice++)
      if ( (Principal_Repertoire_courant[Indice]=='\\') &&
           (Longueur-Indice<=TAILLE_MAXI_PATH-6) )
      {
        // Ouf: on vient de trouver un endroit dans la chaîne à partir duquel
        // on peut faire la copie:
        strcpy(Nom_temporaire+6,Principal_Repertoire_courant+Indice);
        break;
      }

    // Enfin, on peut afficher la chaîne tronquée
    Print_dans_fenetre(7,43,Nom_temporaire,CM_Noir,CM_Clair);
  }
  else // Ahhh! La chaîne peut loger tranquillement dans la fenêtre
    Print_dans_fenetre(7,43,Principal_Repertoire_courant,CM_Noir,CM_Clair);
}


void Print_Nom_fichier_dans_selecteur(void)
//
// Affiche Principal_Nom_fichier dans le Fileselect
//
{
  Block(Fenetre_Pos_X+(Menu_Facteur_X*(13+9*8)),Fenetre_Pos_Y+(Menu_Facteur_Y*90),Menu_Facteur_X*(27*8),Menu_Facteur_Y<<3,CM_Clair);
  Print_dans_fenetre_limite(13+9*8,90,Principal_Nom_fichier,27,CM_Noir,CM_Clair);
}


void Print_Format(void)
//
// Affiche le libellé correspondant à Principal_Format
//
{
  if (Principal_Format==0)
    Print_dans_fenetre(83,65,"*.*",CM_Noir,CM_Clair);
  else
    Print_dans_fenetre(83,65,Format_Extension[Principal_Format-1],CM_Noir,CM_Clair);
}



void Preparer_et_afficher_liste_fichiers(short Position, short Decalage,
                                         struct Fenetre_Bouton_scroller * Enreg)
{
  Enreg->Nb_elements=Liste_Nb_elements;
  Enreg->Position=Position;
  Calculer_hauteur_curseur_jauge(Enreg);
  Fenetre_Dessiner_jauge(Enreg);
  // On efface les anciens noms de fichier:
  Block(Fenetre_Pos_X+(Menu_Facteur_X<<3),Fenetre_Pos_Y+(Menu_Facteur_Y*(89+FILENAMESPACE)),Menu_Facteur_X*98,Menu_Facteur_Y*82,CM_Noir);
  // On affiche les nouveaux:
  Afficher_la_liste_des_fichiers(Position,Decalage);

  // On récupère le nom du schmilblick à "accéder"
  Determiner_element_de_la_liste(Position,Decalage,Principal_Nom_fichier);
  // On affiche le nouveau nom de fichier
  Print_Nom_fichier_dans_selecteur();
  // On affiche le nom du répertoire courant
  Print_repertoire_courant();
}


void Relire_liste_fichiers(byte Filtre, short Position, short Decalage,
                           struct Fenetre_Bouton_scroller * Enreg)
{
  Lire_liste_des_fichiers(Filtre);
  Trier_la_liste_des_fichiers();
  Preparer_et_afficher_liste_fichiers(Position,Decalage,Enreg);
}

void On_vient_de_scroller_dans_le_fileselect(struct Fenetre_Bouton_scroller * Scroller_de_fichiers)
{
  char Ancien_nom_de_fichier[TAILLE_CHEMIN_FICHIER];

  strcpy(Ancien_nom_de_fichier,Principal_Nom_fichier);

  // On regarde si la liste a bougé
  if (Scroller_de_fichiers->Position!=Principal_File_list_Position)
  {
    // Si c'est le cas, il faut mettre à jour la jauge
    Scroller_de_fichiers->Position=Principal_File_list_Position;
    Fenetre_Dessiner_jauge(Scroller_de_fichiers);
  }
  // On récupére le nom du schmilblick à "accéder"
  Determiner_element_de_la_liste(Principal_File_list_Position,Principal_File_list_Decalage,Principal_Nom_fichier);
  if (strcmp(Ancien_nom_de_fichier,Principal_Nom_fichier))
    Nouvelle_preview=1;

  // On affiche le nouveau nom de fichier
  Print_Nom_fichier_dans_selecteur();
  Afficher_curseur();
}


short Position_fichier_dans_liste(char * Nom)
{
  struct Element_de_liste_de_fileselect * Element_courant;
  short  Indice;

  for (Indice=0, Element_courant=Liste_du_fileselect;
       ((Element_courant!=NULL) && (strcmp(Element_courant->NomComplet,Nom)));
       Indice++,Element_courant=Element_courant->Suivant);

  return (Element_courant!=NULL)?Indice:0;
}


void Placer_barre_de_selection_sur(char * Nom)
{
  short Indice;

  Indice=Position_fichier_dans_liste(Nom);

  if ((Liste_Nb_elements<=10) || (Indice<5))
  {
    Principal_File_list_Position=0;
    Principal_File_list_Decalage=Indice;
  }
  else
  {
    if (Indice>=Liste_Nb_elements-5)
    {
      Principal_File_list_Position=Liste_Nb_elements-10;
      Principal_File_list_Decalage=Indice-Principal_File_list_Position;
    }
    else
    {
      Principal_File_list_Position=Indice-4;
      Principal_File_list_Decalage=4;
    }
  }
}


char FFF_Meilleur_nom[TAILLE_CHEMIN_FICHIER];
char * Nom_correspondant_le_mieux_a(char * Nom)
{
  char Nom_courant[TAILLE_CHEMIN_FICHIER];
  char * Pointeur1;
  char * Pointeur2;
  char * Pointeur_Meilleur_nom;
  struct Element_de_liste_de_fileselect * Element_courant;
  byte   Lettres_identiques=0;
  byte   Compteur;

  strcpy(FFF_Meilleur_nom,Principal_Nom_fichier);
  Pointeur_Meilleur_nom=NULL;

  for (Element_courant=Liste_du_fileselect; Element_courant!=NULL; Element_courant=Element_courant->Suivant)
  {
    if ( (!Config.Find_file_fast)
      || (Config.Find_file_fast==(Element_courant->Type+1)) )
    {
      // On copie le nom de la liste en cours de traitement dans Nom_courant
      // tout en le remettant sous forme normale.
      for (Pointeur1=Element_courant->NomComplet,Pointeur2=Nom_courant;*Pointeur1;Pointeur1++)
        if (*Pointeur1!=' ')
          *(Pointeur2++)=*Pointeur1;
      *Pointeur2=0;
      // On compare et si c'est mieux, on stocke dans Meilleur_nom
      for (Compteur=0; tolower(Nom_courant[Compteur])==tolower(Nom[Compteur]); Compteur++);
      if (Compteur>Lettres_identiques)
      {
        Lettres_identiques=Compteur;
        strcpy(FFF_Meilleur_nom,Nom_courant);
        Pointeur_Meilleur_nom=Element_courant->NomComplet;
      }
    }
  }

  return Pointeur_Meilleur_nom;
}


byte Bouton_Load_ou_Save(byte Load, byte Image)
  // Load=1 => On affiche le menu du bouton LOAD
  // Load=0 => On affiche le menu du bouton SAVE
{
  short Bouton_clicke;
  struct Fenetre_Bouton_scroller * Scroller_de_fichiers;
  short Temp;
  int Bidon=0;       // Sert à appeler SDL_GetKeyState
  word  Drives_Debut_Y;
  byte  Charger_ou_sauver_l_image=0;
  char  Nom_drive[3]="  ";
  byte  On_a_clicke_sur_OK=0;// Indique si on a clické sur Load ou Save ou sur
                             //un bouton enclenchant Load ou Save juste après.
  struct Composantes * Palette_initiale; // |  Données concernant l'image qui
  byte  Image_modifiee_initiale;         // |  sont mémorisées pour pouvoir
  short Largeur_image_initiale;          // |- être restaurées en sortant,
  short Hauteur_image_initiale;          // |  parce que la preview elle les
  byte  Back_color_initiale;             // |  fout en l'air (c'te conne).
  char  Nom_fichier_initial[TAILLE_CHEMIN_FICHIER]; // Sert à laisser le nom courant du fichier en cas de sauvegarde
  char  Repertoire_precedent[TAILLE_CHEMIN_FICHIER]; // Répertoire d'où l'on vient après un CHDIR
  char  Commentaire_initial[TAILLE_COMMENTAIRE+1];
  char  Fichier_recherche[TAILLE_CHEMIN_FICHIER]="";
  char * Fichier_le_plus_ressemblant;

  Palette_initiale=(struct Composantes *)malloc(sizeof(T_Palette));
  memcpy(Palette_initiale,Principal_Palette,sizeof(T_Palette));

  Back_color_initiale=Back_color;
  Image_modifiee_initiale=Principal_Image_modifiee;
  Largeur_image_initiale=Principal_Largeur_image;
  Hauteur_image_initiale=Principal_Hauteur_image;
  strcpy(Nom_fichier_initial,Principal_Nom_fichier);
  strcpy(Commentaire_initial,Principal_Commentaire);
  if (Load)
  {
    if (Image)
      Ouvrir_fenetre(310,187+FILENAMESPACE,"Load picture");
    else
      Ouvrir_fenetre(310,187+FILENAMESPACE,"Load brush");
    Fenetre_Definir_bouton_normal(125,157+FILENAMESPACE,51,14,"Load",0,1,SDLK_RETURN); // 1
  }
  else
  {
    if (Image)
      Ouvrir_fenetre(310,187+FILENAMESPACE,"Save picture");
    else
      Ouvrir_fenetre(310,187+FILENAMESPACE,"Save brush");
    Fenetre_Definir_bouton_normal(125,157+FILENAMESPACE,51,14,"Save",0,1,SDLK_RETURN); // 1
    if (Principal_Format==0) // Correction du *.*
    {
      Principal_Format=Principal_Format_fichier;
      Principal_File_list_Position=0;
      Principal_File_list_Decalage=0;
    }

    if (Principal_Format>NB_FORMATS_SAVE) // Correction d'un format insauvable
    {
      Principal_Format=FORMAT_PAR_DEFAUT;
      Principal_File_list_Position=0;
      Principal_File_list_Decalage=0;
    }
    // Affichage du commentaire
    if (Format_Commentaire[Principal_Format-1])
      Print_dans_fenetre(46,175+FILENAMESPACE,Principal_Commentaire,CM_Noir,CM_Clair);
  }

  Fenetre_Definir_bouton_normal(125,139+FILENAMESPACE,51,14,"Cancel",0,1,SDLK_ESCAPE); // 2
  Fenetre_Definir_bouton_normal(125, 89+FILENAMESPACE,51,14,"Delete",0,1,SDLK_DELETE); // 3

  // Cadre autour des formats
  Fenetre_Afficher_cadre(  7, 51,103, 35);
  // Cadre autour des infos sur le fichier de dessin
  Fenetre_Afficher_cadre(116, 51,187, 35);
  // Cadre autour de la preview
  Fenetre_Afficher_cadre_creux(179,88+FILENAMESPACE,124,84);
  // Cadre autour du fileselector
  Fenetre_Afficher_cadre_creux(  7,88+FILENAMESPACE,100,84);

  Fenetre_Definir_bouton_special(9,90+FILENAMESPACE,96,80);                      // 4

  // Scroller du fileselector
  Fenetre_Definir_bouton_scroller(110,89+FILENAMESPACE,82,1,10,0);               // 5
  Scroller_de_fichiers=Fenetre_Liste_boutons_scroller;

  // Scroller des formats
  Fenetre_Definir_bouton_scroller(12,55,27,(Load)?NB_FORMATS_LOAD+1:NB_FORMATS_SAVE,1,(Load)?Principal_Format:Principal_Format-1); // 6

  // Texte de commentaire des dessins
  Print_dans_fenetre(7,174+FILENAMESPACE,"Txt:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(44,173+FILENAMESPACE,TAILLE_COMMENTAIRE); // 7

  // Cadre autour du nom de fichier
  //Fenetre_Afficher_cadre_creux(  7,87,296,15);
  Print_dans_fenetre(9,90,"Filename:",CM_Fonce,CM_Clair);
  // Saisie du nom de fichier
  Fenetre_Definir_bouton_saisie(11+9*8,88,27); // 8

  Print_dans_fenetre( 27,65,"Format:",CM_Fonce,CM_Clair);
  Print_dans_fenetre(120,55,"Image size :",CM_Fonce,CM_Clair);
  Print_dans_fenetre(120,63,"File size  :",CM_Fonce,CM_Clair);
  Print_dans_fenetre(120,72,"Format     :",CM_Fonce,CM_Clair);

  Print_Format();

  // Définition des boutons représentant les lecteurs
  Drives_Debut_Y=(Nb_drives<=13)? 23 : 18;
  for (Temp=0; Temp<Nb_drives; Temp++)
  {
    Nom_drive[0]=Drive[Temp].Lettre;
    Fenetre_Definir_bouton_normal(8+((Temp%13)*20),Drives_Debut_Y+((Temp/13)*12),19,11,Nom_drive,0,1,(Temp < 10) ? SDLK_F1 + Temp : SDLK_LAST); // 9 et +
    Fenetre_Afficher_sprite_drive(18+((Temp%13)*20),Drives_Debut_Y+2+((Temp/13)*12),Drive[Temp].Type);
  }

  // On prend bien soin de passer dans le répertoire courant (le bon qui faut! Oui madame!)
  chdir(Principal_Repertoire_courant);
  /*_dos_setdrive(Principal_Repertoire_courant[0]-64,&Bidon);*/
  Determiner_repertoire_courant();

  // Affichage des premiers fichiers visibles:
  Relire_liste_fichiers(Principal_Format,Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);

  //   Je n'efface pas cette partie parce que l'idée n'était pas mauvaise mais
  // ça chie un maximum alors autant ne pas trop compliquer les choses...
  /*
  if (!Load)
  {
    // On initialise le nom de fichier à celui en cours et non pas celui sous
    // la barre de sélection
    strcpy(Principal_Nom_fichier,Nom_fichier_initial);
    // On affiche le nouveau nom de fichier
    Print_Nom_fichier_dans_selecteur();
  }
  */

  Pixel_de_chargement=Pixel_Chargement_dans_preview;
  Nouvelle_preview=1;

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case -1 :
      case  0 :
        break;

      case  1 : // Load ou Save
        On_a_clicke_sur_OK=1;
        break;

      case  2 : // Cancel
        break;

      case  3 : // Delete
        if (Liste_Nb_elements && (*Principal_Nom_fichier!='.'))
        {
          Effacer_curseur();
          // On affiche une demande de confirmation
          if (Principal_File_list_Position+Principal_File_list_Decalage>=Liste_Nb_repertoires)
          {
            Print_dans_fenetre(127,107+FILENAMESPACE,"Delete",CM_Fonce,CM_Clair);
            Print_dans_fenetre(127,115+FILENAMESPACE,"file ?",CM_Fonce,CM_Clair);
          }
          else
          {
            Print_dans_fenetre(127,107+FILENAMESPACE,"Remove",CM_Fonce,CM_Clair);
            Print_dans_fenetre(127,115+FILENAMESPACE,"dir. ?",CM_Fonce,CM_Clair);
          }
          Print_dans_fenetre(127,123+FILENAMESPACE,"Yes/No",CM_Fonce,CM_Clair);

          do
          {
            SDL_PumpEvents();
          	Etat_Du_Clavier=SDL_GetKeyState(&Bidon);
          } while ((Etat_Du_Clavier[SDLK_y]==0) && (Etat_Du_Clavier[SDLK_n]==0) && (Etat_Du_Clavier[SDLK_ESCAPE]==0));

          // On efface la demande de confirmation
          Block(Fenetre_Pos_X+127*Menu_Facteur_X,Fenetre_Pos_Y+(107+FILENAMESPACE)*Menu_Facteur_Y,
                Menu_Facteur_X*48,Menu_Facteur_Y*24,CM_Clair);

          // Si l'utilisateur confirme,
          if (Etat_Du_Clavier[SDLK_y])
          {
            // Si c'est un fichier
            if (Principal_File_list_Position+Principal_File_list_Decalage>=Liste_Nb_repertoires)
              // On efface le fichier (si on peut)
              Temp=(!remove(Principal_Nom_fichier));
            else // Si c'est un repertoire
              // On efface le repertoire (si on peut)
              Temp=(!rmdir(Principal_Nom_fichier));

            if (Temp) // Temp indique si l'effacement s'est bien passé
            {
              // On remonte si c'était le dernier élément de la liste
              if (Principal_File_list_Position+Principal_File_list_Decalage==Liste_Nb_elements-1)
              {
                if (Principal_File_list_Position)
                  Principal_File_list_Position--;
                else
                  if (Principal_File_list_Decalage)
                    Principal_File_list_Decalage--;
              }
              else // Si ce n'était pas le dernier, il faut faire gaffe à ce
              {    // que ses copains d'en dessous ne remontent pas trop.
                if ( (Principal_File_list_Position)
                  && (Principal_File_list_Position+10==Liste_Nb_elements) )
                  {
                    Principal_File_list_Position--;
                    Principal_File_list_Decalage++;
                  }
              }
              // On relit les informations
              Relire_liste_fichiers(Principal_Format,Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);
              // On demande la preview du nouveau fichier sur lequel on se trouve
              Nouvelle_preview=1;
            }
            else
              Erreur(0);

            // On place la barre de sélection du brouillon au début s'il a le
            // même répertoire que l'image principale.
            if (!strcmp(Principal_Repertoire_courant,Brouillon_Repertoire_courant))
            {
              Brouillon_File_list_Position=0;
              Brouillon_File_list_Decalage=0;
            }
          }
          Afficher_curseur();
        }
        break;

      case  4 : // Zone d'affichage de la liste de fichiers
        Effacer_curseur();

        Temp=Calculer_decalage_click_dans_fileselector();
        if (Temp>=0)
        {
          if (Temp!=Principal_File_list_Decalage)
          {
            // On met à jour le décalage
            Principal_File_list_Decalage=Temp;

            // On récupére le nom du schmilblick à "accéder"
            Determiner_element_de_la_liste(Principal_File_list_Position,Principal_File_list_Decalage,Principal_Nom_fichier);
            // On affiche le nouveau nom de fichier
            Print_Nom_fichier_dans_selecteur();
            // On affiche à nouveau la liste
            Afficher_la_liste_des_fichiers(Principal_File_list_Position,Principal_File_list_Decalage);

            // On vient de changer de nom de fichier, donc on doit s'appreter
            // a rafficher une preview
            Nouvelle_preview=1;
          }
          else
          {
            //   En sauvegarde, si on a double-clické sur un répertoire, il
            // faut mettre le nom de fichier au nom du répertoire. Sinon, dans
            // certains cas, on risque de sauvegarder avec le nom du fichier
            // actuel au lieu de changer de répertoire.
            if (Principal_File_list_Position+Principal_File_list_Decalage<Liste_Nb_repertoires)
              Determiner_element_de_la_liste(Principal_File_list_Position,Principal_File_list_Decalage,Principal_Nom_fichier);

            On_a_clicke_sur_OK=1;
            Nouvelle_preview=1;
          }
        }
        Afficher_curseur();
        Attendre_fin_de_click();
        break;

      case  5 : // Scroller de fichiers
        Effacer_curseur();
        Principal_File_list_Position=Fenetre_Attribut2;
        // On récupére le nom du schmilblick à "accéder"
        Determiner_element_de_la_liste(Principal_File_list_Position,Principal_File_list_Decalage,Principal_Nom_fichier);
        // On affiche le nouveau nom de fichier
        Print_Nom_fichier_dans_selecteur();
        // On affiche à nouveau la liste
        Afficher_la_liste_des_fichiers(Principal_File_list_Position,Principal_File_list_Decalage);
        Afficher_curseur();
        Nouvelle_preview=1;
        break;

      case  6 : // Scroller des formats
        Effacer_curseur();
        // On met à jour le format de browsing du fileselect:
        Principal_Format=(Load)?Fenetre_Attribut2:Fenetre_Attribut2+1;
        // On affiche le nouveau format de lecture:
        Print_Format();
        // Comme on change de liste, on se place en début de liste:
        Principal_File_list_Position=0;
        Principal_File_list_Decalage=0;
        // Affichage des premiers fichiers visibles:
        Relire_liste_fichiers(Principal_Format,Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);
        Afficher_curseur();
        Nouvelle_preview=1;
        break;
      case  7 : // Saisie d'un commentaire pour la sauvegarde
        if ( (!Load) && (Format_Commentaire[Principal_Format-1]) )
        {
          Effacer_curseur();
          Readline(46,175+FILENAMESPACE,Principal_Commentaire,32,0);
          Afficher_curseur();
        }
        break;
      case  8 : // Saisie du nom de fichier
        Effacer_curseur();
        if (Readline(13+9*8,90,Principal_Nom_fichier,27,2))
        {
          //   On regarde s'il faut rajouter une extension. C'est-à-dire s'il
          // n'y a pas de '.' dans le nom du fichier.
          for(Temp=0,Bidon=0; ((Principal_Nom_fichier[Temp]) && (!Bidon)); Temp++)
            if (Principal_Nom_fichier[Temp]=='.')
              Bidon=1;
          if (!Bidon)
          {
            strcat(Principal_Nom_fichier,".");
            if (Principal_Format)
              strcat(Principal_Nom_fichier,Format_Extension[Principal_Format-1]);
          }
          On_a_clicke_sur_OK=1;
        }
        Afficher_curseur();
        break;
      default : // Drives
        // On change de lecteur: Lecteur = Drive[Bouton_clicke-9].Lettre-64
        if (! ActiverLecteur(Bouton_clicke-9))
        {
          Effacer_curseur();
          // On lit le répertoire courant de ce lecteur
          Determiner_repertoire_courant();
          //   Comme on tombe sur un disque qu'on connait pas, on se place en
          // début de liste:
          Principal_File_list_Position=0;
          Principal_File_list_Decalage=0;
          // Affichage des premiers fichiers visibles:
          Relire_liste_fichiers(Principal_Format,Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);
          Afficher_curseur();
          Nouvelle_preview=1;
        }
        else
        {
          // Un ptit flash rouge pour signaler le lecteur invalide.
          Erreur(0);
        }
    }

    switch (Touche)
    {
      case SDLK_UNKNOWN : break;
      case SDLK_DOWN : // Bas
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_Scroll_Down(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_UP : // Haut
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_Scroll_Up(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_PAGEDOWN : // Page Down
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_Page_Down(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_PAGEUP : // Page Up
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_Page_Up(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_END : // End
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_End(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_HOME : // Home
        *Fichier_recherche=0;
        Effacer_curseur();
        Select_Home(&Principal_File_list_Position,&Principal_File_list_Decalage);
        On_vient_de_scroller_dans_le_fileselect(Scroller_de_fichiers);
        break;
      case SDLK_BACKSPACE : // Backspace
        *Fichier_recherche=0;
        if (Principal_Repertoire_courant[3]) // Si on n'est pas à la racine...
        {                              // ... on va dans le répertoire parent.
          strcpy(Principal_Nom_fichier,"..");
          On_a_clicke_sur_OK=1;
        }
        break;
      default: // Autre => On se place sur le nom de fichier qui correspond
        if (Bouton_clicke<=0)
        {
          Temp=strlen(Fichier_recherche);
          if (Touche_ANSI>= ' ' && Touche_ANSI < 255 && Temp<50)
          {
            Fichier_recherche[Temp]=toupper(Touche_ANSI);
            Fichier_recherche[Temp+1]=0;
            Fichier_le_plus_ressemblant=Nom_correspondant_le_mieux_a(Fichier_recherche);
            if ( (Fichier_le_plus_ressemblant)
              && (!memcmp(Fichier_recherche,FFF_Meilleur_nom,Temp+1)) )
            {
              Temp=Principal_File_list_Position+Principal_File_list_Decalage;
              Effacer_curseur();
              Placer_barre_de_selection_sur(Fichier_le_plus_ressemblant);
              Preparer_et_afficher_liste_fichiers(Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);
              Afficher_curseur();
              if (Temp!=Principal_File_list_Position+Principal_File_list_Decalage)
                Nouvelle_preview=1;
            }
            else
              Fichier_recherche[Temp]=0;
          }
        }
        else
          *Fichier_recherche=0;
    }

    if (On_a_clicke_sur_OK)
    {
      //   Si c'est un répertoire, on annule "On_a_clicke_sur_OK" et on passe
      // dedans.
      if (Repertoire_existe(Principal_Nom_fichier))
      {
        Effacer_curseur();
        On_a_clicke_sur_OK=0;

        // On mémorise le répertoire dans lequel on était
        if (strcmp(Principal_Nom_fichier,".."))
          strcpy(Repertoire_precedent,Nom_formate(".."));
        else
        {
          for (Temp=strlen(Principal_Repertoire_courant);
               (Temp>0) && (Principal_Repertoire_courant[Temp-1]!='\\');
               Temp--);
          strcpy(Repertoire_precedent,Nom_formate(Principal_Repertoire_courant+Temp));
        }

        // On doit rentrer dans le répertoire:
        chdir(Principal_Nom_fichier);
        Determiner_repertoire_courant();

        // On lit le nouveau répertoire
        Lire_liste_des_fichiers(Principal_Format);
        Trier_la_liste_des_fichiers();
        // On place la barre de sélection sur le répertoire d'où l'on vient
        Placer_barre_de_selection_sur(Repertoire_precedent);
        // Affichage des premiers fichiers visibles:
        Preparer_et_afficher_liste_fichiers(Principal_File_list_Position,Principal_File_list_Decalage,Scroller_de_fichiers);
        Afficher_curseur();
        Nouvelle_preview=1;
      }
      else  // Sinon on essaye de charger ou sauver le fichier
      {
        strcpy(Principal_Repertoire_fichier,Principal_Repertoire_courant);
        if (!Load)
          Principal_Format_fichier=Principal_Format;
        Charger_ou_sauver_l_image=1;
      }
    }

    // Gestion du chrono et des previews
    if (Nouvelle_preview)
    {
      // On efface les infos de la preview précédente s'il y en a une
      // d'affichée
      if (Etat_chrono==2)
      {
        Effacer_curseur();
        // On efface le commentaire précédent
        Block(Fenetre_Pos_X+ 46*Menu_Facteur_X,Fenetre_Pos_Y+(175+FILENAMESPACE)*Menu_Facteur_Y,
              Menu_Facteur_X<<8,Menu_Facteur_Y<<3,CM_Clair);
        // On nettoie la zone où va s'afficher la preview:
        Block(Fenetre_Pos_X+180*Menu_Facteur_X,Fenetre_Pos_Y+ (89+FILENAMESPACE)*Menu_Facteur_Y,
              Menu_Facteur_X*122,Menu_Facteur_Y*82,CM_Clair);
        // On efface les dimensions de l'image
        Block(Fenetre_Pos_X+226*Menu_Facteur_X,Fenetre_Pos_Y+ 55*Menu_Facteur_Y,
              Menu_Facteur_X*72,Menu_Facteur_Y<<3,CM_Clair);
        // On efface la taille du fichier
        Block(Fenetre_Pos_X+226*Menu_Facteur_X,Fenetre_Pos_Y+ 63*Menu_Facteur_Y,
              Menu_Facteur_X*72,Menu_Facteur_Y<<3,CM_Clair);
        // On efface le format du fichier
        Block(Fenetre_Pos_X+226*Menu_Facteur_X,Fenetre_Pos_Y+ 72*Menu_Facteur_Y,
              Menu_Facteur_X*72,Menu_Facteur_Y<<3,CM_Clair);
        // Affichage du commentaire
        if ( (!Load) && (Format_Commentaire[Principal_Format-1]) )
          Print_dans_fenetre(46,175+FILENAMESPACE,Principal_Commentaire,CM_Noir,CM_Clair);
        Afficher_curseur();

	SDL_UpdateRect(Ecran_SDL,Fenetre_Pos_X+27*Menu_Facteur_X,Fenetre_Pos_Y+(55+FILENAMESPACE)*Menu_Facteur_Y,Menu_Facteur_X<<9,Menu_Facteur_Y<<4);
      }

      Nouvelle_preview=0;
      Etat_chrono=0;         // Etat du chrono = Attente d'un Xème de seconde
      // On lit le temps de départ du chrono
      Initialiser_chrono(Config.Chrono_delay);
    }

    if (!Etat_chrono)  // Prendre une nouvelle mesure du chrono et regarder
      Tester_chrono(); // s'il ne faut pas afficher la preview

    if (Etat_chrono==1) // Il faut afficher la preview
    {
      if ( (Principal_File_list_Position+Principal_File_list_Decalage>=Liste_Nb_repertoires) && (Liste_Nb_elements) )
      {
        strcpy(Principal_Repertoire_fichier,Principal_Repertoire_courant);

        Effacer_curseur();
        Charger_image(Image);
	SDL_UpdateRect(Ecran_SDL,ToWinX(179),ToWinY(88+FILENAMESPACE),ToWinL(124),ToWinH(84));
        Afficher_curseur();

        // Après le chargement de la preview, on restaure tout ce qui aurait
        // pu être modifié par le chargement de l'image:
        memcpy(Principal_Palette,Palette_initiale,sizeof(T_Palette));
        Principal_Image_modifiee=Image_modifiee_initiale;
        Principal_Largeur_image=Largeur_image_initiale;
        Principal_Hauteur_image=Hauteur_image_initiale;
      }

      Etat_chrono=2; // On arrête le chrono
    }
  }
  while ( (!On_a_clicke_sur_OK) && (Bouton_clicke!=2) );

  // Si on annule, on restaure l'ancien commentaire
  if (Bouton_clicke==2)
    strcpy(Principal_Commentaire,Commentaire_initial);

  //   On restaure les données de l'image qui ont certainement été modifiées
  // par la preview.
  memcpy(Principal_Palette,Palette_initiale,sizeof(T_Palette));
  Back_color=Back_color_initiale;
  Principal_Image_modifiee=Image_modifiee_initiale;
  Principal_Largeur_image=Largeur_image_initiale;
  Principal_Hauteur_image=Hauteur_image_initiale;
  Set_palette(Principal_Palette);

  Calculer_couleurs_menu_optimales(Principal_Palette);
  Temp=(Fenetre_Pos_Y+(Fenetre_Hauteur*Menu_Facteur_Y)<Menu_Ordonnee_avant_fenetre);

  Fermer_fenetre();

  if (Temp)
    Afficher_menu();

  Desenclencher_bouton((Load)?BOUTON_CHARGER:BOUTON_SAUVER);
  Afficher_curseur();
  Detruire_liste_du_fileselect();

  Pixel_de_chargement=Pixel_Chargement_dans_ecran_courant;

  free(Palette_initiale);

  return Charger_ou_sauver_l_image;
}


// -- Fonction renvoyant le mode vidéo le plus adapté à l'image chargée -----
#define TOLERANCE_X 8
#define TOLERANCE_Y 4
int Meilleur_mode_video(void)
{
  short Meilleure_largeur,Meilleure_hauteur;
  int Meilleur_mode;
  short Temp_X,Temp_Y;
  int Mode;

  // Si mode fenêtre, on reste dans ce mode.
  if (Resolution_actuelle == 0)
    return 0;
    
  // On commence par borner les dimensions, ou du moins les rendre cohérentes
  if ((Ecran_original_X<=0) || (Config.Set_resolution_according_to==2))
    Ecran_original_X=Principal_Largeur_image;
  else
    if (Ecran_original_X<320)
      Ecran_original_X=320;

  if ((Ecran_original_Y<=0) || (Config.Set_resolution_according_to==2))
    Ecran_original_Y=Principal_Hauteur_image;
  else
    if (Ecran_original_Y<200)
      Ecran_original_Y=200;

  if ((Ecran_original_X>1024) || (Ecran_original_Y>768))
  {
    Ecran_original_X=1024;
    Ecran_original_Y=768;
  }

  // Maintenant on peut chercher le mode qui correspond le mieux
  Meilleur_mode=Resolution_actuelle;
  Meilleure_largeur=0;
  Meilleure_hauteur=0;


  for (Mode=0; Mode<=NB_MODES_VIDEO; Mode++)
  {
    if (Mode_video[Mode].Fullscreen && Mode_video[Mode].Etat<2)
    {
      Temp_X=Mode_video[Mode].Largeur;
      Temp_Y=Mode_video[Mode].Hauteur;

      if ( (Ecran_original_X-TOLERANCE_X<=Temp_X)
        && (Ecran_original_Y-TOLERANCE_Y<=Temp_Y) )
        return Mode;
      else
      {
        if ( (Meilleure_largeur<=Temp_X)
          && (Meilleure_hauteur<=Temp_Y)
          && (Temp_X-TOLERANCE_X<=Ecran_original_X)
          && (Temp_Y-TOLERANCE_Y<=Ecran_original_Y) )
        {
          Meilleure_largeur=Temp_X;
          Meilleure_hauteur=Temp_Y;
          Meilleur_mode=Mode;
        }
      }
    }
  }

  return Meilleur_mode;
}


void Swapper_infos_selecteurs_image_et_brosse(void)
{
  char  Chaine_temporaire[256];
  byte  Octet_temporaire;
  short Entier_temporaire;


  strcpy(Chaine_temporaire           ,Brosse_Repertoire_fichier);
  strcpy(Brosse_Repertoire_fichier   ,Principal_Repertoire_fichier);
  strcpy(Principal_Repertoire_fichier,Chaine_temporaire);

  strcpy(Chaine_temporaire    ,Brosse_Nom_fichier);
  strcpy(Brosse_Nom_fichier   ,Principal_Nom_fichier);
  strcpy(Principal_Nom_fichier,Chaine_temporaire);

  Octet_temporaire        =Brosse_Format_fichier;
  Brosse_Format_fichier   =Principal_Format_fichier;
  Principal_Format_fichier=Octet_temporaire;

  Octet_temporaire=Brosse_Format;
  Brosse_Format   =Principal_Format;
  Principal_Format=Octet_temporaire;

  Entier_temporaire            =Brosse_File_list_Position;
  Brosse_File_list_Position    =Principal_File_list_Position;
  Principal_File_list_Position=Entier_temporaire;

  Entier_temporaire            =Brosse_File_list_Decalage;
  Brosse_File_list_Decalage    =Principal_File_list_Decalage;
  Principal_File_list_Decalage=Entier_temporaire;

  strcpy(Chaine_temporaire           ,Brosse_Repertoire_courant);
  strcpy(Brosse_Repertoire_courant   ,Principal_Repertoire_courant);
  strcpy(Principal_Repertoire_courant,Chaine_temporaire);

  strcpy(Chaine_temporaire    ,Brosse_Commentaire);
  strcpy(Brosse_Commentaire   ,Principal_Commentaire);
  strcpy(Principal_Commentaire,Chaine_temporaire);
}


void Load_picture(byte Image)
  // Image=1 => On charge/sauve une image
  // Image=0 => On charge/sauve une brosse
{
  // Données initiales du fichier (au cas où on voudrait annuler)
  char  Repertoire_fichier_initial[TAILLE_CHEMIN_FICHIER];
  char  Nom_fichier_initial[TAILLE_CHEMIN_FICHIER];
  byte  Format_fichier_initial;
  byte  Ne_pas_restaurer;
  byte  Utiliser_palette_brosse = 0;
  struct Composantes * Palette_initiale=NULL;
  byte  Ancienne_forme_curseur;
  short Principal_Largeur_image_initiale=Principal_Largeur_image;
  short Principal_Hauteur_image_initiale=Principal_Hauteur_image;
  //char  Commentaire_initial[TAILLE_COMMENTAIRE+1];
  int   Nouveau_mode;


  if (!Image)
    Swapper_infos_selecteurs_image_et_brosse();

  strcpy(Repertoire_fichier_initial,Principal_Repertoire_fichier);
  strcpy(Nom_fichier_initial       ,Principal_Nom_fichier);
  Format_fichier_initial=Principal_Format_fichier;

  if (!Image)
  {
    Palette_initiale=(struct Composantes *)malloc(sizeof(T_Palette));
    memcpy(Palette_initiale,Principal_Palette,sizeof(T_Palette));
  }

  Ne_pas_restaurer=Bouton_Load_ou_Save(1,Image);

  if (Ne_pas_restaurer)
  {
    if (Image)
    {
      if (Principal_Image_modifiee)
        Ne_pas_restaurer=Demande_de_confirmation("Discard unsaved changes?");
    }
    else
      Utiliser_palette_brosse=Demande_de_confirmation("Use the palette of the brush?");
  }

  if (Ne_pas_restaurer)
  {
    Ancienne_forme_curseur=Forme_curseur;
    Effacer_curseur();
    Forme_curseur=FORME_CURSEUR_SABLIER;
    Afficher_curseur();

    if (Image)
    {
      // Si c'est une image qu'on charge, on efface l'ancien commentaire
      // C'est loin d'être indispensable, m'enfin bon...
      if (Format_Backup_done[Principal_Format_fichier-1])
        Principal_Commentaire[0]='\0';

      Ecran_original_X=0;
      Ecran_original_Y=0;
    }
    else
      Pixel_de_chargement=Pixel_Chargement_dans_brosse;

    Charger_image(Image);

    if (!Image)
    {
      if (!Utiliser_palette_brosse)
        memcpy(Principal_Palette,Palette_initiale,sizeof(T_Palette));

      if (Erreur_fichier==3) // On ne peut pas allouer la brosse
      {
        if (Brosse) free(Brosse);
        Brosse=(byte *)malloc(1*1);
        Brosse_Hauteur=1;
        Brosse_Largeur=1;
        *Brosse=Fore_color;

        if (Smear_Brosse) free(Smear_Brosse);
        Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
        Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
        Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
      }
      else
      {
        Brosse_Largeur=Principal_Largeur_image;
        Brosse_Hauteur=Principal_Hauteur_image;
        Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
        Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
      }

      Tiling_Decalage_X=0;
      Tiling_Decalage_Y=0;

      Brosse_Decalage_X=(Brosse_Largeur>>1);
      Brosse_Decalage_Y=(Brosse_Hauteur>>1);

      Principal_Largeur_image=Principal_Largeur_image_initiale;
      Principal_Hauteur_image=Principal_Hauteur_image_initiale;
      Pixel_de_chargement=Pixel_Chargement_dans_ecran_courant;

      Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE);
      if (Config.Auto_discontinuous)
      {
        // On se place en mode Dessin discontinu à la main
        while (Operation_en_cours!=OPERATION_DESSIN_DISCONTINU)
          Enclencher_bouton(BOUTON_DESSIN,A_DROITE);
      }
      Effacer_curseur();
      // On passe en brosse couleur:
      Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);
    }
    else
    {
      Effacer_curseur();
      Forme_curseur=Ancienne_forme_curseur;
    }

    if ( (Erreur_fichier==1) || (!Format_Backup_done[Principal_Format_fichier-1]) )
    {
      Ne_pas_restaurer=0;
      if (Erreur_fichier!=1)
        Calculer_couleurs_menu_optimales(Principal_Palette);
    }
    else
    {
      if (Image)
      {
        if (Loupe_Mode)
        {
          Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,0);
          Pixel_Preview=Pixel_Preview_Normal;
          Loupe_Mode=0;
        }

        Nouveau_mode=Meilleur_mode_video();
        if ((Config.Auto_set_res) && (Nouveau_mode!=Resolution_actuelle))
        {
          Initialiser_mode_video(
            Mode_video[Nouveau_mode].Largeur,
            Mode_video[Nouveau_mode].Hauteur,
            Mode_video[Nouveau_mode].Fullscreen);
          Afficher_menu();
        }
        else
        {
          Principal_Decalage_X=0;
          Principal_Decalage_Y=0;
          Calculer_limites();
          Calculer_coordonnees_pinceau();
        }
      }

      Calculer_couleurs_menu_optimales(Principal_Palette);
      Afficher_ecran();

      if (Image)
        Principal_Image_modifiee=0;
    }
    Afficher_menu();
    Afficher_curseur();
  }

  if (!Image)
    free(Palette_initiale);

  if (!Ne_pas_restaurer)
  {
    strcpy(Principal_Nom_fichier       ,Nom_fichier_initial);
    strcpy(Principal_Repertoire_fichier,Repertoire_fichier_initial);
    Principal_Format_fichier    =Format_fichier_initial;
  }

  if (!Image)
    Swapper_infos_selecteurs_image_et_brosse();

  Print_nom_fichier();
  Set_palette(Principal_Palette);
}


void Bouton_Load(void)
{
  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page_principal(Principal_Backups->Pages);

  Load_picture(1);
}


void Bouton_Reload(void)
{
  byte Ancienne_forme_curseur;
  int  Nouveau_mode;

  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page_principal(Principal_Backups->Pages);

  if ( (!Principal_Image_modifiee) || Demande_de_confirmation("Discard unsaved changes ?") )
  {
    Effacer_curseur();
    Ancienne_forme_curseur=Forme_curseur;
    Forme_curseur=FORME_CURSEUR_SABLIER;
    Afficher_curseur();

    Ecran_original_X=0;
    Ecran_original_Y=0;
    Charger_image(1);

    Effacer_curseur();
    Forme_curseur=Ancienne_forme_curseur;

    if (Erreur_fichier!=1)
    {
      if (Loupe_Mode)
      {
        Tracer_cadre_de_bouton_du_menu(BOUTON_LOUPE,0);
        Pixel_Preview=Pixel_Preview_Normal;
        Loupe_Mode=0;
      }

      Nouveau_mode=Meilleur_mode_video();
      if ( ((Config.Auto_set_res) && (Nouveau_mode!=Resolution_actuelle)) &&
           (!Une_resolution_a_ete_passee_en_parametre) )
      {
        Initialiser_mode_video(
        Mode_video[Nouveau_mode].Largeur,
        Mode_video[Nouveau_mode].Hauteur,
        Mode_video[Nouveau_mode].Fullscreen);
        Afficher_menu();
      }
      else
      {
        Principal_Decalage_X=0;
        Principal_Decalage_Y=0;
        Calculer_limites();
        Calculer_coordonnees_pinceau();
      }

      Afficher_ecran();

      Principal_Image_modifiee=0;
    }
  }
  else
    Effacer_curseur();

  Calculer_couleurs_menu_optimales(Principal_Palette);
  Afficher_menu();
  if (Config.Afficher_limites_image)
    Afficher_limites_de_l_image();

  Desenclencher_bouton(BOUTON_CHARGER);

  Afficher_curseur();
}


void Nom_fichier_backup(char * Nom, char * Nom_backup)
{
  short Curseur;

  strcpy(Nom_backup,Nom);
  for (Curseur=strlen(Nom)-strlen(Principal_Nom_fichier); Nom_backup[Curseur]!='.'; Curseur++);
  Nom_backup[Curseur+1]='\0';
  strcat(Nom_backup,"BAK");
}


void Backup_du_fichier_sauvegarde(void)
{
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier
  char Nouveau_nom_du_fichier[TAILLE_CHEMIN_FICHIER]; // Nom complet du fichier backup

  Nom_fichier_complet(Nom_du_fichier,0);
  // Calcul du nom complet du fichier backup
  Nom_fichier_backup(Nom_du_fichier,Nouveau_nom_du_fichier);

  Erreur_fichier=0;

  // On fait un backup si le nom du fichier n'est pas celui qu'on a choisi
  // pour nommer les backups (c'est évident!).
  if (strcmp(Nouveau_nom_du_fichier,Nom_du_fichier))
  {
    // S'il y avait déjà un fichier Backup, on l'efface
    if ((Fichier_existe(Nouveau_nom_du_fichier))
     && (remove(Nouveau_nom_du_fichier)!=0))
      Erreur_fichier=1;

    if ((!Erreur_fichier)
     && (rename(Nom_du_fichier,Nouveau_nom_du_fichier)!=0))
      Erreur_fichier=1;
  }
}


void Save_picture(byte Image)
  // Image=1 => On charge/sauve une image
  // Image=0 => On charge/sauve une brosse
{
  // Données initiales du fichier (au cas où on voudrait annuler)
  char  Repertoire_fichier_initial[TAILLE_CHEMIN_FICHIER];
  char  Nom_fichier_initial[TAILLE_CHEMIN_FICHIER];
  byte  Format_fichier_initial;
  byte  Ne_pas_restaurer;
  byte  Ancienne_forme_curseur;
  short Principal_Largeur_image_Backup=Principal_Largeur_image;
  short Principal_Hauteur_image_Backup=Principal_Hauteur_image;
  //char  Commentaire_initial[TAILLE_COMMENTAIRE+1];


  if (!Image)
    Swapper_infos_selecteurs_image_et_brosse();

  strcpy(Repertoire_fichier_initial,Principal_Repertoire_fichier);
  strcpy(Nom_fichier_initial       ,Principal_Nom_fichier);
  Format_fichier_initial=Principal_Format_fichier;

  Ne_pas_restaurer=Bouton_Load_ou_Save(0,Image);

  if (Ne_pas_restaurer && Fichier_existe(Principal_Nom_fichier))
  {
    Ne_pas_restaurer=Demande_de_confirmation("Erase old file ?");
    if ((Ne_pas_restaurer) && (Config.Backup))
    {
      Backup_du_fichier_sauvegarde();
      if (Erreur_fichier)
      {
        Ne_pas_restaurer=0;
        Erreur(0);
      }
    }
  }

  if (Ne_pas_restaurer)
  {
    Ancienne_forme_curseur=Forme_curseur;
    Effacer_curseur();
    Forme_curseur=FORME_CURSEUR_SABLIER;
    Afficher_curseur();

    if (Image)
      Sauver_image(Image);
    else
    {
      Principal_Largeur_image=Brosse_Largeur;
      Principal_Hauteur_image=Brosse_Hauteur;
      Sauver_image(Image);
      Principal_Largeur_image=Principal_Largeur_image_Backup;
      Principal_Hauteur_image=Principal_Hauteur_image_Backup;
    }

    Effacer_curseur();
    Forme_curseur=Ancienne_forme_curseur;

    if ((Erreur_fichier==1) || (!Format_Backup_done[Principal_Format_fichier-1]))
      Ne_pas_restaurer=0;

    Afficher_curseur();
  }

  if (!Ne_pas_restaurer)
  {
    strcpy(Principal_Nom_fichier       ,Nom_fichier_initial);
    strcpy(Principal_Repertoire_fichier,Repertoire_fichier_initial);
           Principal_Format_fichier    =Format_fichier_initial;
  }

  if (!Image)
    Swapper_infos_selecteurs_image_et_brosse();

  Print_nom_fichier();
  Set_palette(Principal_Palette);
}


void Bouton_Save(void)
{
  Save_picture(1);
}


void Bouton_Autosave(void)
{
  byte Ancienne_forme_curseur;
  static char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  byte Le_fichier_existe;


  Nom_fichier_complet(Nom_du_fichier,0);
  Le_fichier_existe=Fichier_existe(Nom_du_fichier);

  if ( (!Le_fichier_existe) || Demande_de_confirmation("Erase old file ?") )
  {
    if ((Le_fichier_existe) && (Config.Backup))
      Backup_du_fichier_sauvegarde();
    else
      Erreur_fichier=0;

    Effacer_curseur();

    if (!Erreur_fichier)
    {
      Ancienne_forme_curseur=Forme_curseur;
      Forme_curseur=FORME_CURSEUR_SABLIER;
      Afficher_curseur();

      Sauver_image(1);

      Effacer_curseur();
      Forme_curseur=Ancienne_forme_curseur;
    }
    else
      Erreur(0);
  }
  else
    Effacer_curseur();

  Desenclencher_bouton(BOUTON_SAUVER);

  Afficher_curseur();
}


// -- Gestion des boutons de ligne ------------------------------------------

void Bouton_Lignes(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(Ligne_en_cours);
  Afficher_curseur();
}


void Bouton_Lignes_Switch_mode(void)
{
  if (Ligne_en_cours==OPERATION_LIGNE)
    Ligne_en_cours=OPERATION_K_LIGNE;
  else
  {
    if (Ligne_en_cours==OPERATION_K_LIGNE)
      Ligne_en_cours=OPERATION_LIGNES_CENTREES;
    else
      Ligne_en_cours=OPERATION_LIGNE;
  }

  Effacer_curseur();
  Afficher_sprite_dans_menu(BOUTON_LIGNES,Ligne_en_cours-OPERATION_LIGNE+6);
  Demarrer_pile_operation(Ligne_en_cours);
  Afficher_curseur();
}


// -- Bouton de brosse ------------------------------------------------------

void Bouton_Brosse(void)
{
  Effacer_curseur();

  if (Operation_en_cours!=OPERATION_PRISE_BROSSE)
    Demarrer_pile_operation(OPERATION_PRISE_BROSSE);
  else
    Desenclencher_bouton(BOUTON_BROSSE);

  Afficher_curseur();
}


void Bouton_desenclencher_Brosse(void)
{
  // On fait de notre mieux pour restaurer l'ancienne opération:
  Demarrer_pile_operation(Operation_avant_interruption);
}


void Bouton_Restaurer_brosse(void)
{
  Effacer_curseur();
  // On passe en brosse couleur:
  Changer_la_forme_du_pinceau(FORME_PINCEAU_BROSSE_COULEUR);

  Desenclencher_bouton(BOUTON_BROSSE);
  Desenclencher_bouton(BOUTON_POLYBROSSE);

  Afficher_curseur();
}


// -- Bouton de prise de brosse au lasso ------------------------------------

void Bouton_Lasso(void)
{
  Effacer_curseur();

  if (Operation_en_cours!=OPERATION_POLYBROSSE)
  {
    Pinceau_Forme_avant_lasso=Pinceau_Forme;
    Pinceau_Forme=FORME_PINCEAU_POINT;
    Demarrer_pile_operation(OPERATION_POLYBROSSE);
  }
  else
    Desenclencher_bouton(BOUTON_POLYBROSSE);

  Afficher_curseur();
}


void Bouton_desenclencher_Lasso(void)
{
  // On fait de notre mieux pour restaurer l'ancienne opération:
  Demarrer_pile_operation(Operation_avant_interruption);
  Pinceau_Forme=Pinceau_Forme_avant_lasso;
}


// -- Bouton de pipette -----------------------------------------------------

void Bouton_Pipette(void)
{
  Effacer_curseur();

  if (Operation_en_cours!=OPERATION_PIPETTE)
  {
    Pipette_Couleur=-1;
    Demarrer_pile_operation(OPERATION_PIPETTE);
    Pinceau_Forme_avant_pipette=Pinceau_Forme;
    Pinceau_Forme=FORME_PINCEAU_POINT;
    if (Operation_avant_interruption!=OPERATION_REMPLACER)
      if ( (Mouse_Y<Menu_Ordonnee) && (Menu_visible) &&
           ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
        Print_dans_menu("X:       Y:       (    )",0);
  }
  else
    Desenclencher_bouton(BOUTON_PIPETTE);

  Afficher_curseur();
}


void Bouton_desenclencher_Pipette(void)
{
  if (Operation_avant_interruption!=OPERATION_REMPLACER)
    if ( (Mouse_Y<Menu_Ordonnee) && (Menu_visible) &&
         ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
      Print_dans_menu("X:       Y:             ",0);

  // On fait de notre mieux pour restaurer l'ancienne opération:
  if (Operation_en_cours==OPERATION_PIPETTE)
  {
    Demarrer_pile_operation(Operation_avant_interruption);
    Pinceau_Forme=Pinceau_Forme_avant_pipette;
  }
}


  // -- Inversion de la couleur Fore et de la couleur Back --
void Bouton_Inverser_foreback(void)
{
  byte Couleur_temporaire;

  Effacer_curseur();

  Encadrer_couleur_menu(CM_Noir);

  Couleur_temporaire=Fore_color;
  Fore_color        =Back_color;
  Back_color        =Couleur_temporaire;

  Recadrer_palette();

  Encadrer_couleur_menu(CM_Blanc);

  Afficher_foreback();
  Desenclencher_bouton(BOUTON_PIPETTE);

  Afficher_curseur();
}


// -- Gestion du bouton Loupe -----------------------------------------------

byte On_vient_du_menu_de_facteurs_de_zoom=0;

void Bouton_Loupe(void)
{
  Effacer_curseur();
  if ( (Operation_en_cours==OPERATION_LOUPE) || (Loupe_Mode) )
  {
    Desenclencher_bouton(BOUTON_LOUPE);
  }
  else
  {
    Ancien_Principal_Decalage_X=Principal_Decalage_X;
    Ancien_Principal_Decalage_Y=Principal_Decalage_Y;
    Calculer_donnees_loupe();
    if ((!Config.Fast_zoom) || (Mouse_Y>=Menu_Ordonnee) || On_vient_du_menu_de_facteurs_de_zoom)
    {
      On_vient_du_menu_de_facteurs_de_zoom=0;
      Demarrer_pile_operation(OPERATION_LOUPE);
    }
    else
    { /* Ceci est de la duplication de code de presque toute l'opération de */
      /* la loupe... Il serait peut-être plus propre de faire une procédure */
      /* qui s'en charge... */
      // On passe en mode loupe
      Loupe_Mode=1;

      // La fonction d'affichage dans la partie image est désormais un affichage
      // spécial loupe.
      Pixel_Preview=Pixel_Preview_Loupe;

      // On calcule l'origine de la loupe
      Loupe_Decalage_X=Mouse_X-(Loupe_Largeur>>1);
      Loupe_Decalage_Y=Mouse_Y-(Loupe_Hauteur>>1);

      // Calcul du coin haut_gauche de la fenêtre devant être zoomée DANS L'ECRAN
      if (Loupe_Decalage_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
        Loupe_Decalage_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
      if (Loupe_Decalage_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
        Loupe_Decalage_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;

      // Calcul des coordonnées absolues de ce coin DANS L'IMAGE
      Loupe_Decalage_X+=Principal_Decalage_X;
      Loupe_Decalage_Y+=Principal_Decalage_Y;

      if (Loupe_Decalage_X<0)
        Loupe_Decalage_X=0;
      if (Loupe_Decalage_Y<0)
        Loupe_Decalage_Y=0;

      // On calcule les bornes visibles dans l'écran
      Recadrer_ecran_par_rapport_au_zoom();
      Calculer_limites();
      Afficher_ecran();

      // Repositionner le curseur en fonction des coordonnées visibles
      Calculer_coordonnees_pinceau();
    }
  }
  Afficher_curseur();
}


void Bouton_Menu_Loupe(void)
{
  short Bouton_clicke;

  Ouvrir_fenetre(141,114,"Zoom factors");

  Fenetre_Definir_bouton_normal(45,88,51,14,"Cancel",0,1,SDLK_ESCAPE); // 1

  Fenetre_Definir_bouton_normal(  9,25,27,14, "x2",0,Loupe_Facteur!= 2,SDLK_F1);    // 2
  Fenetre_Definir_bouton_normal( 41,25,27,14, "x3",0,Loupe_Facteur!= 3,SDLK_F2);    // 3
  Fenetre_Definir_bouton_normal( 73,25,27,14, "x4",0,Loupe_Facteur!= 4,SDLK_F3);    // 4
  Fenetre_Definir_bouton_normal(105,25,27,14, "x5",0,Loupe_Facteur!= 5,SDLK_F4);    // 5
  Fenetre_Definir_bouton_normal(  9,45,27,14, "x6",0,Loupe_Facteur!= 6,SDLK_F5);    // 6
  Fenetre_Definir_bouton_normal( 41,45,27,14, "x8",0,Loupe_Facteur!= 8,SDLK_F6);    // 7
  Fenetre_Definir_bouton_normal( 73,45,27,14,"x10",0,Loupe_Facteur!=10,SDLK_F7);    // 8
  Fenetre_Definir_bouton_normal(105,45,27,14,"x12",0,Loupe_Facteur!=12,SDLK_F8);    // 9
  Fenetre_Definir_bouton_normal(  9,65,27,14,"x14",0,Loupe_Facteur!=14,SDLK_F9);    // 10
  Fenetre_Definir_bouton_normal( 41,65,27,14,"x16",0,Loupe_Facteur!=16,SDLK_F10);    // 11
  Fenetre_Definir_bouton_normal( 73,65,27,14,"x18",0,Loupe_Facteur!=18,SDLK_F11);    // 12
  Fenetre_Definir_bouton_normal(105,65,27,14,"x20",0,Loupe_Facteur!=20,SDLK_F12);    // 13

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke<=0);

  if (Bouton_clicke>1)
  {
    Menu_Ordonnee=Menu_Ordonnee_avant_fenetre;
    Changer_facteur_loupe(Bouton_clicke-2);
  }

  Fermer_fenetre();

  if ( (Bouton_clicke==1) && (!Loupe_Mode) && (Operation_en_cours!=OPERATION_LOUPE) )
    Desenclencher_bouton(BOUTON_LOUPE);

  Afficher_curseur();

  if ( (Bouton_clicke>1) && (!Loupe_Mode) && (Operation_en_cours!=OPERATION_LOUPE) )
  {
    On_vient_du_menu_de_facteurs_de_zoom=1;
    Enclencher_bouton(BOUTON_LOUPE,A_GAUCHE);
  }
}


void Bouton_desenclencher_Loupe(void)
{
  if (Loupe_Mode)
  {
    // On sort du mode loupe
    Loupe_Mode=0;

    /*
    // --> Recalculer le décalage de l'écran lorsqu'on sort de la loupe <--
    // Centrage "brut" de lécran par rapport à la loupe
    Principal_Decalage_X=Loupe_Decalage_X-((Largeur_ecran-Loupe_Largeur)>>1);
    Principal_Decalage_Y=Loupe_Decalage_Y-((Menu_Ordonnee-Loupe_Hauteur)>>1);
    */
    // Correction en cas de débordement de l'image
    if (Ancien_Principal_Decalage_X+Largeur_ecran>Principal_Largeur_image)
      Principal_Decalage_X=Principal_Largeur_image-Largeur_ecran;
    else
      Principal_Decalage_X=Ancien_Principal_Decalage_X;
    if (Principal_Decalage_X<0)
      Principal_Decalage_X=0;

    if (Ancien_Principal_Decalage_Y+Menu_Ordonnee>Principal_Hauteur_image)
      Principal_Decalage_Y=Principal_Hauteur_image-Menu_Ordonnee;
    else
      Principal_Decalage_Y=Ancien_Principal_Decalage_Y;
    if (Principal_Decalage_Y<0)
      Principal_Decalage_Y=0;

    // La fonction d'affichage dans l'image est désormais un affichage normal.
    Pixel_Preview=Pixel_Preview_Normal;

    // Calculer les bornes visibles dans l'écran
    Calculer_limites();
    Afficher_ecran();  // <=> Display_screen();
    // Repositionner le curseur en fonction des coordonnées visibles
    Calculer_coordonnees_pinceau();
  }
  else // On fait de notre mieux pour restaurer l'ancienne opération:
    Demarrer_pile_operation(Operation_avant_interruption);
}


// -------------------------------- Grille -----------------------------------

void Bouton_Snap_Mode(void)
{
  Effacer_curseur();
  Snap_Mode=!Snap_Mode;
  Calculer_coordonnees_pinceau();
  Afficher_curseur();
}


void Bouton_Menu_Grille(void)
{
  short Bouton_clicke;
  word  X_choisi =Snap_Largeur;
  word  Y_choisi =Snap_Hauteur;
  short dX_choisi=Snap_Decalage_X;
  short dY_choisi=Snap_Decalage_Y;

  struct Fenetre_Bouton_special * Bouton_saisie_X;
  struct Fenetre_Bouton_special * Bouton_saisie_Y;
  struct Fenetre_Bouton_special * Bouton_saisie_dX;
  struct Fenetre_Bouton_special * Bouton_saisie_dY;

  char Chaine[3];


  Ouvrir_fenetre(133,98,"Grid");

  Fenetre_Definir_bouton_normal(12,72,51,14,"Cancel",0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_normal(70,72,51,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Print_dans_fenetre(19,26, "X:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(37,24,2); // 3
  Bouton_saisie_X=Fenetre_Liste_boutons_special;
  Num2str(X_choisi,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_X,Chaine);

  Print_dans_fenetre(19,47, "Y:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(37,45,2); // 4
  Bouton_saisie_Y=Fenetre_Liste_boutons_special;
  Num2str(Y_choisi,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_Y,Chaine);

  Print_dans_fenetre(69,26,"dX:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(95,24,2); // 5
  Bouton_saisie_dX=Fenetre_Liste_boutons_special;
  Num2str(dX_choisi,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_dX,Chaine);

  Print_dans_fenetre(69,47,"dY:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(95,45,2); // 6
  Bouton_saisie_dY=Fenetre_Liste_boutons_special;
  Num2str(dY_choisi,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_dY,Chaine);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case 3 :
        Effacer_curseur();
        Num2str(X_choisi,Chaine,2);
        Readline(39,26,Chaine,2,1);
        X_choisi=atoi(Chaine);
        // On corrige les dimensions
        if ((!X_choisi) || (X_choisi>80))
        {
          if (!X_choisi)
            X_choisi=1;
          else
            X_choisi=80;
          Num2str(X_choisi,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_X,Chaine);
        }
        if (dX_choisi>=X_choisi)
        {
          dX_choisi=X_choisi-1;
          Num2str(dX_choisi,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_dX,Chaine);
        }
        Afficher_curseur();
        break;
      case 4 :
        Effacer_curseur();
        Num2str(Y_choisi,Chaine,2);
        Readline(39,47,Chaine,2,1);
        Y_choisi=atoi(Chaine);
        // On corrige les dimensions
        if ((!Y_choisi) || (Y_choisi>80))
        {
          if (!Y_choisi)
            Y_choisi=1;
          else
            Y_choisi=80;
          Num2str(Y_choisi,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_Y,Chaine);
        }
        if (dY_choisi>=Y_choisi)
        {
          dY_choisi=Y_choisi-1;
          Num2str(dY_choisi,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Bouton_saisie_dY,Chaine);
        }
        Afficher_curseur();
        break;
      case 5 :
        Effacer_curseur();
        Num2str(dX_choisi,Chaine,2);
        Readline(97,26,Chaine,2,1);
        dX_choisi=atoi(Chaine);
        // On corrige les dimensions
        if (dX_choisi>79)
          dX_choisi=79;
        if (dX_choisi>=X_choisi)
          dX_choisi=X_choisi-1;

        Num2str(dX_choisi,Chaine,2);
        Fenetre_Contenu_bouton_saisie(Bouton_saisie_dX,Chaine);

        Afficher_curseur();
        break;
      case 6 :
        Effacer_curseur();
        Num2str(dY_choisi,Chaine,2);
        Readline(97,47,Chaine,2,1);
        dY_choisi=atoi(Chaine);
        // On corrige les dimensions
        if (dY_choisi>79)
          dY_choisi=79;
        if (dY_choisi>=Y_choisi)
          dY_choisi=Y_choisi-1;

        Num2str(dY_choisi,Chaine,2);
        Fenetre_Contenu_bouton_saisie(Bouton_saisie_dY,Chaine);

        Afficher_curseur();
    }
  }
  while ( (Bouton_clicke!=1) && (Bouton_clicke!=2) );

  if (Bouton_clicke==2) // OK
  {
    Snap_Largeur=X_choisi;
    Snap_Hauteur=Y_choisi;
    Snap_Decalage_X=dX_choisi;
    Snap_Decalage_Y=dY_choisi;
    Snap_Mode=1;
  }

  Fermer_fenetre();

  if ( (Bouton_clicke==2) && (!Snap_Mode) )
    Bouton_Snap_Mode();

  Afficher_curseur();
}


// ----------------------- Modifications de brosse ---------------------------

void Bouton_Brush_FX(void)
{
  short Bouton_clicke;
  short Indice;

  Ouvrir_fenetre(310,162,"Brush effects");

  Fenetre_Afficher_cadre(  6,19,298,61);
  Fenetre_Afficher_cadre(  6,83,122,53);
  Fenetre_Afficher_cadre(137,83,167,53);

  Fenetre_Definir_bouton_normal(236,141, 67,14,"Cancel"          ,0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_normal( 19, 46, 27,14,"X\035"           ,1,1,SDLK_x); // 2
  Fenetre_Definir_bouton_normal( 19, 61, 27,14,"Y\022"           ,1,1,SDLK_y); // 3
  Fenetre_Definir_bouton_normal( 58, 46, 37,14,"90°"             ,0,1,SDLK_LAST); // 4
  Fenetre_Definir_bouton_normal( 96, 46, 37,14,"180°"            ,0,1,SDLK_LAST); // 5
  Fenetre_Definir_bouton_normal( 58, 61, 75,14,"any angle"       ,0,1,SDLK_LAST); // 6
  Fenetre_Definir_bouton_normal(145, 46, 67,14,"Stretch"         ,2,1,SDLK_t); // 7
  Fenetre_Definir_bouton_normal(145, 61, 67,14,"Distort"         ,1,1,SDLK_d); // 8
  Fenetre_Definir_bouton_normal(155, 99,131,14,"Recolorize"      ,1,1,SDLK_r); // 9
  Fenetre_Definir_bouton_normal(155,117,131,14,"Get brush colors",1,1,SDLK_g); // 10

  // Boutons représentant les coins du brush handle: (HG,HD,C,BG,BD)
  Fenetre_Definir_bouton_normal( 75, 90,11,11,"",0,1,SDLK_HOME); // 11
  Fenetre_Definir_bouton_normal(103, 90,11,11,"",0,1,SDLK_PAGEUP); // 12
  Fenetre_Definir_bouton_normal( 89,104,11,11,"",0,1,SDLK_KP5); // 13
  Fenetre_Definir_bouton_normal( 75,118,11,11,"",0,1,SDLK_END); // 14
  Fenetre_Definir_bouton_normal(103,118,11,11,"",0,1,SDLK_PAGEDOWN); // 15

  Fenetre_Definir_bouton_normal(224,46,67,14,"Outline",1,1,SDLK_o); // 16
  Fenetre_Definir_bouton_normal(224,61,67,14,"Nibble" ,1,1,SDLK_n); // 17

  Fenetre_Definir_bouton_normal(  7,141, 60,14,"Load",1,1,SDLK_l); // 18
  Fenetre_Definir_bouton_normal( 70,141, 60,14,"Save",1,1,SDLK_s); // 19

  Print_dans_fenetre( 80, 24,"Shape modifications",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 10, 36,"Mirror",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 72, 36,"Rotate",CM_Fonce,CM_Clair);
  Print_dans_fenetre(155, 36,"Deform",CM_Fonce,CM_Clair);
  Print_dans_fenetre(230, 36,"Borders",CM_Fonce,CM_Clair);
  Print_dans_fenetre(141, 88,"Colors modifications",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 20,102,"Brush",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 16,110,"handle",CM_Fonce,CM_Clair);

  // Dessin des pointillés pour le "brush handle"
  for (Indice=0; Indice<13; Indice+=2)
  {
    Pixel_dans_fenetre( 88+Indice, 92,CM_Fonce);
    Pixel_dans_fenetre( 88+Indice,126,CM_Fonce);
    Pixel_dans_fenetre( 77,103+Indice,CM_Fonce);
    Pixel_dans_fenetre(111,103+Indice,CM_Fonce);
  }
  // Dessin des coins et du centre pour les boutons du "brush handle"
    // Coin HG
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 77),Fenetre_Pos_Y+(Menu_Facteur_Y* 92),Menu_Facteur_X*7,Menu_Facteur_Y,CM_Noir);
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 77),Fenetre_Pos_Y+(Menu_Facteur_Y* 92),Menu_Facteur_X,Menu_Facteur_Y*7,CM_Noir);
    // Coin HD
  Block(Fenetre_Pos_X+(Menu_Facteur_X*105),Fenetre_Pos_Y+(Menu_Facteur_Y* 92),Menu_Facteur_X*7,Menu_Facteur_Y,CM_Noir);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*111),Fenetre_Pos_Y+(Menu_Facteur_Y* 92),Menu_Facteur_X,Menu_Facteur_Y*7,CM_Noir);
    // Centre
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 91),Fenetre_Pos_Y+(Menu_Facteur_Y*109),Menu_Facteur_X*7,Menu_Facteur_Y,CM_Noir);
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 94),Fenetre_Pos_Y+(Menu_Facteur_Y*106),Menu_Facteur_X,Menu_Facteur_Y*7,CM_Noir);
    // Coin BG
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 77),Fenetre_Pos_Y+(Menu_Facteur_Y*126),Menu_Facteur_X*7,Menu_Facteur_Y,CM_Noir);
  Block(Fenetre_Pos_X+(Menu_Facteur_X* 77),Fenetre_Pos_Y+(Menu_Facteur_Y*120),Menu_Facteur_X,Menu_Facteur_Y*7,CM_Noir);
    // Coin BD
  Block(Fenetre_Pos_X+(Menu_Facteur_X*105),Fenetre_Pos_Y+(Menu_Facteur_Y*126),Menu_Facteur_X*7,Menu_Facteur_Y,CM_Noir);
  Block(Fenetre_Pos_X+(Menu_Facteur_X*111),Fenetre_Pos_Y+(Menu_Facteur_Y*120),Menu_Facteur_X,Menu_Facteur_Y*7,CM_Noir);
  
  SDL_UpdateRect(Ecran_SDL,(Largeur_ecran-(310*Menu_Facteur_X))>>1,(Hauteur_ecran-(162*Menu_Facteur_Y))>>1,310,162);
  
  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
  }
  while (Bouton_clicke<=0);

  Fermer_fenetre();
  Desenclencher_bouton(BOUTON_EFFETS_BROSSE);

  // Gestion du bouton clické
  switch (Bouton_clicke)
  {
    case  2 : // Flip X
      Flip_X_LOWLEVEL();
      break;
    case  3 : // Flip Y
      Flip_Y_LOWLEVEL();
      break;
    case  4 : // 90° Rotation
      Rotate_90_deg();
      break;
    case  5 : // 180° Rotation
      if (Brosse_Hauteur&1)
      { // Brosse de hauteur impaire
        Flip_X_LOWLEVEL();
        Flip_Y_LOWLEVEL();
      }
      else
        Rotate_180_deg_LOWLEVEL();
      Brosse_Decalage_X=(Brosse_Largeur>>1);
      Brosse_Decalage_Y=(Brosse_Hauteur>>1);
      break;
    case  6 : // Any angle rotation
      Demarrer_pile_operation(OPERATION_TOURNER_BROSSE);
      break;
    case  7 : // Stretch
      Demarrer_pile_operation(OPERATION_ETIRER_BROSSE);
      break;
    case  8 : // Distort
      Afficher_curseur();
      Message_Non_disponible(); // !!! TEMPORAIRE !!!
      Effacer_curseur();
      break;
    case  9 : // Recolorize
      Remap_brosse();
      break;
    case 10 : // Get brush colors
      Afficher_curseur();
      Get_colors_from_brush();
      Effacer_curseur();
      break;
    case 11 : // Brush Attachment: Top-Left
      Brosse_Decalage_X=0;
      Brosse_Decalage_Y=0;
      break;
    case 12 : // Brush Attachment: Top-Right
      Brosse_Decalage_X=(Brosse_Largeur-1);
      Brosse_Decalage_Y=0;
      break;
    case 13 : // Brush Attachment: Center
      Brosse_Decalage_X=(Brosse_Largeur>>1);
      Brosse_Decalage_Y=(Brosse_Hauteur>>1);
      break;
    case 14 : // Brush Attachment: Bottom-Left
      Brosse_Decalage_X=0;
      Brosse_Decalage_Y=(Brosse_Hauteur-1);
      break;
    case 15 : // Brush Attachment: Bottom-Right
      Brosse_Decalage_X=(Brosse_Largeur-1);
      Brosse_Decalage_Y=(Brosse_Hauteur-1);
      break;
    case 16 : // Outline
      Outline_brush();
      break;
    case 17 : // Nibble
      Nibble_brush();
      break;
    case 18 : // Load
      Afficher_curseur();
      Load_picture(0);
      Effacer_curseur();
      break;
    case 19 : // Save
      Afficher_curseur();
      Save_picture(0);
      Effacer_curseur();
      break;
  }

  Afficher_curseur();
}


// -- Mode Smooth -----------------------------------------------------------
void Bouton_Smooth_Mode(void)
{
  if (Smooth_Mode)
    Fonction_effet=Aucun_effet;
  else
  {
    Fonction_effet=Effet_Smooth;
    Shade_Mode=0;
    Quick_shade_Mode=0;
    Colorize_Mode=0;
    Tiling_Mode=0;
    Smear_Mode=0;
  }
  Smooth_Mode=!Smooth_Mode;
}


byte Smooth_Matrice_defaut[4][3][3]=
{
 { {1,2,1}, {2,4,2}, {1,2,1} },
 { {1,3,1}, {3,9,3}, {1,3,1} },
 { {0,1,0}, {1,2,1}, {0,1,0} },
 { {2,3,2}, {3,1,3}, {2,3,2} }
};

void Bouton_Smooth_Menu(void)
{
  short Bouton_clicke;
  short X,Y,I,J;
  byte  Matrice_choisie[3][3];
  struct Fenetre_Bouton_special * Matrice_Zone_saisie[3][3];
  char  Chaine[3];

  Ouvrir_fenetre(142,109,"Smooth");

  Fenetre_Definir_bouton_normal(82,59,53,14,"Cancel",0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_normal(82,88,53,14,"OK"    ,0,1,SDLK_RETURN); // 2

  Fenetre_Afficher_cadre(6,17,130,37);
  for (X=11,Y=0; Y<4; X+=31,Y++)
  {
    Fenetre_Definir_bouton_normal(X,22,27,27,"",0,1,SDLK_LAST);      // 3,4,5,6
    for (J=0; J<3; J++)
      for (I=0; I<3; I++)
        Print_char_dans_fenetre(X+2+(I<<3),24+(J<<3),'0'+Smooth_Matrice_defaut[Y][I][J],CM_Noir,CM_Clair);
  }

  Fenetre_Afficher_cadre(6,58, 69,45);
  for (J=0; J<3; J++)
    for (I=0; I<3; I++)
    {
      Fenetre_Definir_bouton_saisie(10+(I*21),62+(J*13),2);       // 7..15
      Matrice_Zone_saisie[I][J]=Fenetre_Liste_boutons_special;
      Num2str(Matrice_choisie[I][J]=Smooth_Matrice[I][J],Chaine,2);
      Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
    }

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    if (Bouton_clicke>2)
    {
      if (Bouton_clicke<=6)
      {
        memcpy(Matrice_choisie,Smooth_Matrice_defaut[Bouton_clicke-3],sizeof(Matrice_choisie));
        Effacer_curseur();
        for (J=0; J<3; J++)
          for (I=0; I<3; I++)
          {
            Num2str(Matrice_choisie[I][J],Chaine,2);
            Fenetre_Contenu_bouton_saisie(Matrice_Zone_saisie[I][J],Chaine);
          }
        Afficher_curseur();
      }
      else
      {
        Effacer_curseur();
        I=Bouton_clicke-7; X=I%3; Y=I/3;
        Num2str(Matrice_choisie[X][Y],Chaine,2);
        Readline(Matrice_Zone_saisie[X][Y]->Pos_X+2,
                 Matrice_Zone_saisie[X][Y]->Pos_Y+2,
                 Chaine,2,1);
        Matrice_choisie[X][Y]=atoi(Chaine);
        Afficher_curseur();
      }
    }
  }
  while ((Bouton_clicke!=1) && (Bouton_clicke!=2));

  Fermer_fenetre();

  if (Bouton_clicke==2) // OK
  {
    memcpy(Smooth_Matrice,Matrice_choisie,sizeof(Smooth_Matrice));
    Smooth_Mode=0; // On le met à 0 car la fonct° suivante va le passer à 1
    Bouton_Smooth_Mode();
  }

  Afficher_curseur();
}


// -- Mode Smear ------------------------------------------------------------
void Bouton_Smear_Mode(void)
{
  if (!Smear_Mode)
  {
    if (!Colorize_Mode)
      Fonction_effet=Aucun_effet;
    Shade_Mode=0;
    Quick_shade_Mode=0;
    Smooth_Mode=0;
    Tiling_Mode=0;
  }
  Smear_Mode=!Smear_Mode;
}


// -- Mode Colorize ---------------------------------------------------------
void Calculer_les_tables_de_Colorize(void)
{
  word  Indice;
  word  Facteur_A;
  word  Facteur_B;

  Facteur_A=256*(100-Colorize_Opacite)/100;
  Facteur_B=256*(    Colorize_Opacite)/100;

  for (Indice=0;Indice<64;Indice++)
  {
    Table_de_multiplication_par_Facteur_A[Indice]=Indice*Facteur_A;
    Table_de_multiplication_par_Facteur_B[Indice]=Indice*Facteur_B;
  }
}


void Bouton_Colorize_Mode(void)
{
  if (Colorize_Mode)
    Fonction_effet=Aucun_effet;
  else
  {
    switch(Colorize_Mode_en_cours)
    {
      case 0 :
        Fonction_effet=Effet_Colorize_interpole;
        break;
      case 1 :
        Fonction_effet=Effet_Colorize_additif;
        break;
      case 2 :
        Fonction_effet=Effet_Colorize_soustractif;
    }
    Shade_Mode=0;
    Quick_shade_Mode=0;
    Smooth_Mode=0;
    Tiling_Mode=0;
  }
  Colorize_Mode=!Colorize_Mode;
}


void Bouton_Colorize_Afficher_la_selection(int Numero)
{
  short Pos_Y=0; // Ligne où afficher les flèches de sélection

  // On commence par effacer les anciennes sélections:
    // Partie gauche
  Print_dans_fenetre(4,37," ",CM_Noir,CM_Clair);
  Print_dans_fenetre(4,57," ",CM_Noir,CM_Clair);
  Print_dans_fenetre(4,74," ",CM_Noir,CM_Clair);
    // Partie droite
  Print_dans_fenetre(129,37," ",CM_Noir,CM_Clair);
  Print_dans_fenetre(129,57," ",CM_Noir,CM_Clair);
  Print_dans_fenetre(129,74," ",CM_Noir,CM_Clair);

  // Ensuite, on affiche la flèche là où il le faut:
  switch(Numero)
  {
    case 0 : // Méthode interpolée
      Pos_Y=37;
      break;
    case 1 : // Méthode additive
      Pos_Y=57;
      break;
    case 2 : // Méthode soustractive
      Pos_Y=74;
  }
  Print_dans_fenetre(4,Pos_Y,"\020",CM_Noir,CM_Clair);
  Print_dans_fenetre(129,Pos_Y,"\021",CM_Noir,CM_Clair);
}

void Bouton_Colorize_Menu(void)
{
  short Opacite_choisie;
  short Mode_choisi;
  short Bouton_clicke;
  char  Chaine[4];

  Ouvrir_fenetre(140,118,"Transparency");

  Print_dans_fenetre(16,23,"Opacity:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(87,21,3);                               // 1
  Print_dans_fenetre(117,23,"%",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_normal(16,34,108,14,"Interpolate",1,1,SDLK_i); // 2
  Fenetre_Afficher_cadre(12,18,116,34);

  Fenetre_Definir_bouton_normal(16,54,108,14,"Additive"   ,2,1,SDLK_d); // 3
  Fenetre_Definir_bouton_normal(16,71,108,14,"Subtractive",1,1,SDLK_s); // 4

  Fenetre_Definir_bouton_normal(16,94, 51,14,"Cancel"     ,0,1,SDLK_ESCAPE); // 5
  Fenetre_Definir_bouton_normal(73,94, 51,14,"OK"         ,0,1,SDLK_RETURN); // 6

  Num2str(Colorize_Opacite,Chaine,3);
  Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
  Bouton_Colorize_Afficher_la_selection(Colorize_Mode_en_cours);

  Opacite_choisie=Colorize_Opacite;
  Mode_choisi    =Colorize_Mode_en_cours;

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch(Bouton_clicke)
    {
      case 1: // Zone de saisie de l'opacité
        Effacer_curseur();
        Num2str(Opacite_choisie,Chaine,3);
        Readline(89,23,Chaine,3,1);
        Opacite_choisie=atoi(Chaine);
        // On corrige le pourcentage
        if (Opacite_choisie>100)
        {
          Opacite_choisie=100;
          Num2str(Opacite_choisie,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Fenetre_Liste_boutons_special,Chaine);
        }
        Afficher_curseur();
        break;
      case 2: // Méthode interpolée
      case 3: // Méthode additive
      case 4: // Méthode soustractive
        Mode_choisi=Bouton_clicke-2;
        Effacer_curseur();
        Bouton_Colorize_Afficher_la_selection(Mode_choisi);
        Afficher_curseur();
    }
  }
  while (Bouton_clicke<5);

  Fermer_fenetre();

  if (Bouton_clicke==6) // OK
  {
    Colorize_Opacite      =Opacite_choisie;
    Colorize_Mode_en_cours=Mode_choisi;
    Calculer_les_tables_de_Colorize();
    Colorize_Mode=0; // On le met à 0 car la fonct° suivante va le passer à 1
    Bouton_Colorize_Mode();
  }

  Afficher_curseur();
}


// -- Mode Tiling -----------------------------------------------------------
void Bouton_Tiling_Mode(void)
{
  if (Tiling_Mode)
    Fonction_effet=Aucun_effet;
  else
  {
    Fonction_effet=Effet_Tiling;
    Shade_Mode=0;
    Quick_shade_Mode=0;
    Colorize_Mode=0;
    Smooth_Mode=0;
    Smear_Mode=0;
  }
  Tiling_Mode=!Tiling_Mode;
}


void Bouton_Tiling_Menu(void)
{
  short Bouton_clicke;
  short Offset_X_choisi=Tiling_Decalage_X;
  short Offset_Y_choisi=Tiling_Decalage_Y;
  char  Chaine[5];
  struct Fenetre_Bouton_special * Bouton_saisie_Decalage_X;
  struct Fenetre_Bouton_special * Bouton_saisie_Decalage_Y;

  Ouvrir_fenetre(138,79,"Tiling");

  Fenetre_Definir_bouton_normal(13,55,51,14,"Cancel",0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_normal(74,55,51,14,"OK"    ,0,1,SDLK_RETURN); // 2
  Fenetre_Definir_bouton_saisie(91,21,4);                         // 3
  Bouton_saisie_Decalage_X=Fenetre_Liste_boutons_special;
  Fenetre_Definir_bouton_saisie(91,35,4);                         // 4
  Bouton_saisie_Decalage_Y=Fenetre_Liste_boutons_special;
  Print_dans_fenetre(12,23,"Offset X:",CM_Fonce,CM_Clair);
  Print_dans_fenetre(12,37,"Offset Y:",CM_Fonce,CM_Clair);

  Num2str(Tiling_Decalage_X,Chaine,4);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_Decalage_X,Chaine);
  Num2str(Tiling_Decalage_Y,Chaine,4);
  Fenetre_Contenu_bouton_saisie(Bouton_saisie_Decalage_Y,Chaine);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    if (Bouton_clicke==3)  // Zone de saisie du décalage X
    {
      Effacer_curseur();
      Num2str(Offset_X_choisi,Chaine,4);
      Readline(93,23,Chaine,4,1);
      Offset_X_choisi=atoi(Chaine);
      // On corrige le décalage en X
      if (Offset_X_choisi>=Brosse_Largeur)
      {
        Offset_X_choisi=Brosse_Largeur-1;
        Num2str(Offset_X_choisi,Chaine,4);
        Fenetre_Contenu_bouton_saisie(Bouton_saisie_Decalage_X,Chaine);
      }
      Afficher_curseur();
    }
    else
    if (Bouton_clicke==4)  // Zone de saisie du décalage Y
    {
      Effacer_curseur();
      Num2str(Offset_Y_choisi,Chaine,4);
      Readline(93,37,Chaine,4,1);
      Offset_Y_choisi=atoi(Chaine);
      // On corrige le décalage en Y
      if (Offset_Y_choisi>=Brosse_Hauteur)
      {
        Offset_Y_choisi=Brosse_Hauteur-1;
        Num2str(Offset_Y_choisi,Chaine,4);
        Fenetre_Contenu_bouton_saisie(Bouton_saisie_Decalage_Y,Chaine);
      }
      Afficher_curseur();
    }
  }
  while ( (Bouton_clicke!=1) && (Bouton_clicke!=2) );

  Fermer_fenetre();

  if (Bouton_clicke==2) // OK
  {
    Tiling_Decalage_X=Offset_X_choisi;
    Tiling_Decalage_Y=Offset_Y_choisi;
    if (!Tiling_Mode)
      Bouton_Tiling_Mode();
  }

  Afficher_curseur();
}


//---------------------------- Courbes de Bézier ----------------------------

void Bouton_Courbes(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(Courbe_en_cours);
  Afficher_curseur();
}


void Bouton_Courbes_Switch_mode(void)
{
  if (Courbe_en_cours==OPERATION_COURBE_4_POINTS)
    Courbe_en_cours=OPERATION_COURBE_3_POINTS;
  else
    Courbe_en_cours=OPERATION_COURBE_4_POINTS;

  Effacer_curseur();
  Afficher_sprite_dans_menu(BOUTON_COURBES,Courbe_en_cours-OPERATION_COURBE_3_POINTS+4);
  Demarrer_pile_operation(Courbe_en_cours);
  Afficher_curseur();
}


//--------------------------------- Spray -----------------------------------

void Bouton_Spray(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_SPRAY);
  Afficher_curseur();
}


void Spray_Rafficher_infos(byte Couleur_selectionnee, byte Rafficher_jauge)
{
  char  Chaine[3];

  if (Rafficher_jauge)
  {
    Fenetre_Liste_boutons_scroller->Position=49-Spray_Multi_flow[Couleur_selectionnee];
    Fenetre_Dessiner_jauge(Fenetre_Liste_boutons_scroller);
  }
  Num2str(Spray_Multi_flow[Couleur_selectionnee],Chaine,2);
  Print_dans_fenetre(196,130,Chaine,CM_Noir,CM_Clair);
}


void Bouton_Spray_Menu(void)
{
  static byte Spray_Init=0;
  short  Bouton_clicke;
  char   Chaine[4];
  word   Indice;
  byte   Couleur_selectionnee=Fore_color;
  byte   Old_Spray_Mode     =Spray_Mode;
  short  Old_Spray_Size     =Spray_Size;
  byte   Old_Spray_Delay    =Spray_Delay;
  byte   Old_Spray_Mono_flow=Spray_Mono_flow;
  byte   Old_Spray_Multi_flow[256];
  struct Fenetre_Bouton_special * Saisie_Size;
  struct Fenetre_Bouton_special * Saisie_Delay;
  struct Fenetre_Bouton_special * Saisie_Mono_flow;
  struct Fenetre_Bouton_special * Saisie_Init;
  word Ancien_Mouse_X;
  word Ancien_Mouse_Y;
  byte Ancien_Mouse_K;
  byte Couleur;
  byte Click;


  memcpy(Old_Spray_Multi_flow,Spray_Multi_flow,256);


  Ouvrir_fenetre(226,170,"Spray");

  Fenetre_Definir_bouton_normal(110,148,51,14,"Cancel"    ,0,1,SDLK_ESCAPE); // 1
  Fenetre_Definir_bouton_normal(166,148,51,14,"OK"        ,0,1,SDLK_RETURN); // 2

  Fenetre_Definir_bouton_scroller(178,62,74,50,1,49-Spray_Multi_flow[Couleur_selectionnee]); // 3

  Fenetre_Definir_bouton_palette(7,56);                                 // 4

  Fenetre_Definir_bouton_normal(  8,148,83,14,"Mode:     ",0,1,SDLK_TAB); // 5
  if (Spray_Mode)
    Print_dans_fenetre(50,151," Mono",CM_Noir,CM_Clair);
  else
    Print_dans_fenetre(50,151,"Multi",CM_Noir,CM_Clair);

  Fenetre_Definir_bouton_normal(194, 62,19,14,"+1"        ,0,1,SDLK_KP_PLUS); // 6
  Fenetre_Definir_bouton_normal(194, 79,19,14,"-1"        ,0,1,SDLK_KP_MINUS); // 7
  Fenetre_Definir_bouton_normal(194, 96,19,14,"x2"        ,0,1,SDLK_KP_MULTIPLY); // 8
  Fenetre_Definir_bouton_normal(194,113,19,14,"÷2"        ,0,1,SDLK_KP_ENTER); // 9

  Fenetre_Definir_bouton_normal(  8, 37,43,14,"Clear"     ,1,1,SDLK_c); // 10

  Print_dans_fenetre(142,25,"Size:"     ,CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(186,23,3);                              // 11
  Saisie_Size=Fenetre_Liste_boutons_special;
  Num2str(Spray_Size,Chaine,3);
  Fenetre_Contenu_bouton_saisie(Saisie_Size,Chaine);

  Print_dans_fenetre(142,39,"Delay:"    ,CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(194,37,2);                              // 12
  Saisie_Delay=Fenetre_Liste_boutons_special;
  Num2str(Spray_Delay,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Saisie_Delay,Chaine);

  Print_dans_fenetre( 27,24,"Mono-Flow:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(111,22,2);                              // 13
  Saisie_Mono_flow=Fenetre_Liste_boutons_special;
  Num2str(Spray_Mono_flow,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Saisie_Mono_flow,Chaine);

  Print_dans_fenetre( 67,40,"Init:",CM_Fonce,CM_Clair);
  Fenetre_Definir_bouton_saisie(111,38,2);                              // 14
  Saisie_Init=Fenetre_Liste_boutons_special;
  Num2str(Spray_Init,Chaine,2);
  Fenetre_Contenu_bouton_saisie(Saisie_Init,Chaine);

  Fenetre_Afficher_cadre(173,56,45,86);
  Fenetre_Afficher_cadre(137,19,81,33);

  // On tagge toutes les couleurs utilisées
  for (Indice=0; Indice<256; Indice++)
    if (Spray_Multi_flow[Indice])
      Stencil_Tagger_couleur(Indice,CM_Noir);
  // Et enfin, on tagge la couleur sélectionnée
  Stencil_Tagger_couleur(Couleur_selectionnee,CM_Blanc);
  Spray_Rafficher_infos(Couleur_selectionnee,0);

  Afficher_curseur();


  do
  {
    Ancien_Mouse_X=Mouse_X;
    Ancien_Mouse_Y=Mouse_Y;
    Ancien_Mouse_K=Mouse_K;

    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case  0 :
      case  2 : // OK
        break;

      case  1 : // Cancel
        Spray_Mode     =Old_Spray_Mode;
        Spray_Size     =Old_Spray_Size;
        Spray_Delay    =Old_Spray_Delay;
        Spray_Mono_flow=Old_Spray_Mono_flow;
        memcpy(Spray_Multi_flow,Old_Spray_Multi_flow,256);
        break;

      case  3 : // Scroller
        Effacer_curseur();
        Spray_Multi_flow[Couleur_selectionnee]=49-Fenetre_Attribut2;
        Spray_Rafficher_infos(Couleur_selectionnee,0);
        Afficher_curseur();
        break;

      case -1 :
      case  4 : // Palette
        if ( (Mouse_X!=Ancien_Mouse_X) || (Mouse_Y!=Ancien_Mouse_Y) || (Mouse_K!=Ancien_Mouse_K) )
        {
          Effacer_curseur();
          Stencil_Tagger_couleur(Couleur_selectionnee,(Spray_Multi_flow[Couleur_selectionnee])?CM_Noir:CM_Clair);

          // Mettre la couleur sélectionnée à jour suivant le click
          Couleur_selectionnee=Lit_pixel(Mouse_X,Mouse_Y);
          if (Mouse_K==2)
            Spray_Multi_flow[Couleur_selectionnee]=0;
          else
            if (Spray_Multi_flow[Couleur_selectionnee]==0)
              Spray_Multi_flow[Couleur_selectionnee]=Spray_Init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_Tagger_couleur(Couleur_selectionnee,CM_Blanc);
          Spray_Rafficher_infos(Couleur_selectionnee,1);
          Afficher_curseur();
        }
        break;

      case  5 : // Toggle Mode
        Spray_Mode=(Spray_Mode+1)&1;
        Effacer_curseur();
        if (Spray_Mode)
          Print_dans_fenetre(50,151," Mono",CM_Noir,CM_Clair);
        else
          Print_dans_fenetre(50,151,"Multi",CM_Noir,CM_Clair);
        Afficher_curseur();
        break;

      case  6 : // +1
        for (Indice=0; Indice<256; Indice++)
        {
          if ( (Spray_Multi_flow[Indice]) && (Spray_Multi_flow[Indice]<49) )
            Spray_Multi_flow[Indice]++;
        }
        Effacer_curseur();
        Spray_Rafficher_infos(Couleur_selectionnee,1);
        Afficher_curseur();
        break;

      case  7 : // -1
        for (Indice=0; Indice<256; Indice++)
        {
          if (Spray_Multi_flow[Indice]>1)
            Spray_Multi_flow[Indice]--;
        }
        Effacer_curseur();
        Spray_Rafficher_infos(Couleur_selectionnee,1);
        Afficher_curseur();
        break;

      case  8 : // x2
        for (Indice=0; Indice<256; Indice++)
        {
          if (Spray_Multi_flow[Indice])
          {
            Spray_Multi_flow[Indice]<<=1;
            if (Spray_Multi_flow[Indice]>49)
              Spray_Multi_flow[Indice]=49;
          }
        }
        Effacer_curseur();
        Spray_Rafficher_infos(Couleur_selectionnee,1);
        Afficher_curseur();
        break;

      case  9 : // ÷2
        for (Indice=0; Indice<256; Indice++)
        {
          if (Spray_Multi_flow[Indice]>1)
            Spray_Multi_flow[Indice]>>=1;
        }
        Effacer_curseur();
        Spray_Rafficher_infos(Couleur_selectionnee,1);
        Afficher_curseur();
        break;

      case 10 : // Clear
        memset(Spray_Multi_flow,0,256);
        // On raffiche les infos de la couleur sélectionnée
        Spray_Rafficher_infos(Couleur_selectionnee,1);
        // On efface les anciens TAGs
        Fenetre_Effacer_tags();
        // Tagger la couleur sélectionnée en blanc
        Stencil_Tagger_couleur(Couleur_selectionnee,CM_Blanc);
        break;

      case 11 : // Size
        Effacer_curseur();
        Num2str(Spray_Size,Chaine,3);
        Readline(188,25,Chaine,3,1);
        Spray_Size=atoi(Chaine);
        // On corrige les dimensions
        if (Spray_Size>256)
        {
          Spray_Size=256;
          Num2str(Spray_Size,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Saisie_Size,Chaine);
        }
        else if (!Spray_Size)
        {
          Spray_Size=1;
          Num2str(Spray_Size,Chaine,3);
          Fenetre_Contenu_bouton_saisie(Saisie_Size,Chaine);
        }
        Afficher_curseur();
        break;

      case 12 : // Delay
        Effacer_curseur();
        Num2str(Spray_Delay,Chaine,2);
        Readline(196,39,Chaine,2,1);
        Spray_Delay=atoi(Chaine);
        // On corrige le delai
        if (Spray_Delay>10)
        {
          Spray_Delay=10;
          Num2str(Spray_Delay,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Saisie_Delay,Chaine);
        }
        Afficher_curseur();
        break;

      case 13 : // Mono-Flow
        Effacer_curseur();
        Num2str(Spray_Mono_flow,Chaine,2);
        Readline(113,24,Chaine,2,1);
        Spray_Mono_flow=atoi(Chaine);
        // On corrige le flux
        if (!Spray_Mono_flow)
        {
          Spray_Mono_flow=1;
          Num2str(Spray_Mono_flow,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Saisie_Mono_flow,Chaine);
        }
        Afficher_curseur();
        break;

      case 14 : // Init
        Effacer_curseur();
        Num2str(Spray_Init,Chaine,2);
        Readline(113,40,Chaine,2,1);
        Spray_Init=atoi(Chaine);
        // On corrige la valeur
        if (Spray_Init>=50)
        {
          Spray_Init=49;
          Num2str(Spray_Init,Chaine,2);
          Fenetre_Contenu_bouton_saisie(Saisie_Init,Chaine);
        }
        Afficher_curseur();
        break;
    }

    if (!Mouse_K)
    switch (Touche)
    {
      case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case SDLK_COMMA :
        Recuperer_couleur_derriere_fenetre(&Couleur,&Click);
        if (Click)
        {
          Effacer_curseur();
          Stencil_Tagger_couleur(Couleur_selectionnee,(Spray_Multi_flow[Couleur_selectionnee])?CM_Noir:CM_Clair);

          // Mettre la couleur sélectionnée à jour suivant le click
          Couleur_selectionnee=Couleur;
          if (Click==2)
            Spray_Multi_flow[Couleur_selectionnee]=0;
          else
            if (Spray_Multi_flow[Couleur_selectionnee]==0)
              Spray_Multi_flow[Couleur_selectionnee]=Spray_Init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_Tagger_couleur(Couleur_selectionnee,CM_Blanc);
          Spray_Rafficher_infos(Couleur_selectionnee,1);
          Afficher_curseur();
        }
    }
  }
  while ( (Bouton_clicke!=1) && (Bouton_clicke!=2) );

  Fermer_fenetre();

/*
  //   Tant que l'on aura pas résolu le problème du désenclenchement du mode
  // de dessin précedent, il faudra laisser ça en remarque et donc passer en
  // spray même si on a clické sur Cancel (idem pour OK (un peu plus bas)).
  if (Bouton_clicke==1) // Cancel
  {
    if (Operation_en_cours!=OPERATION_SPRAY)
      Desenclencher_bouton(BOUTON_SPRAY);
  }
*/

  Afficher_curseur();

/*
  if (Bouton_clicke==2) // OK
*/
    if (Operation_en_cours!=OPERATION_SPRAY)
      Enclencher_bouton(BOUTON_SPRAY,A_GAUCHE);
}



// -- Mode Trame (Sieve) ----------------------------------------------------

void Bouton_Trame_Mode(void)
{
  Trame_Mode=!Trame_Mode;
}


void Dessiner_trame_zoomee(short Orig_X, short Orig_Y)
{
  short Pos_X;
  short Pos_Y;
  short Taille_X;
  short Taille_Y;
  short Debut_X=Fenetre_Pos_X+(Menu_Facteur_X*230);
  short Debut_Y=Fenetre_Pos_Y+(Menu_Facteur_Y*78);

  Taille_X=Menu_Facteur_X*5; // |_ Taille d'une case
  Taille_Y=Menu_Facteur_Y*5; // |  de la trame zoomée

  // On efface de contenu précédent
  Block(Orig_X,Orig_Y,
        Menu_Facteur_X*Fenetre_Liste_boutons_special->Largeur,
        Menu_Facteur_Y*Fenetre_Liste_boutons_special->Hauteur,CM_Clair);

  for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
    for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
    {
      // Bordures de la case
      Block(Orig_X+(Pos_X*Taille_X),
            Orig_Y+((Pos_Y+1)*Taille_Y)-Menu_Facteur_Y,
            Taille_X, Menu_Facteur_Y,CM_Fonce);
      Block(Orig_X+((Pos_X+1)*Taille_X)-Menu_Facteur_X,
            Orig_Y+(Pos_Y*Taille_Y),
            Menu_Facteur_X, Taille_Y-1,CM_Fonce);
      // Contenu de la case
      Block(Orig_X+(Pos_X*Taille_X), Orig_Y+(Pos_Y*Taille_Y),
            Taille_X-Menu_Facteur_X, Taille_Y-Menu_Facteur_Y,
            (Trame[Pos_X][Pos_Y])?CM_Blanc:CM_Noir);
    }

  // Dessiner la preview de la trame
  Taille_X=Menu_Facteur_X*51; // |_ Taille de la fenêtre
  Taille_Y=Menu_Facteur_Y*71; // |  de la preview
  for (Pos_Y=0; Pos_Y<Taille_Y; Pos_Y++)
    for (Pos_X=0; Pos_X<Taille_X; Pos_X++)
      Pixel(Debut_X+Pos_X,Debut_Y+Pos_Y,(Trame[Pos_X%Trame_Largeur][Pos_Y%Trame_Hauteur])?CM_Blanc:CM_Noir);
  SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Taille_X,Taille_Y);
}


void Dessiner_trames_predefinies(void)
{
  short Indice;
  short i,j;

  for (Indice=0; Indice<12; Indice++)
    for (j=0; j<16; j++)
      for (i=0; i<16; i++)
        Pixel_dans_fenetre((Indice*23)+10+i,22+j,
                           ((TRAME_PREDEFINIE[Indice][j]>>(15-i))&1)?CM_Blanc:CM_Noir);
  SDL_UpdateRect(Ecran_SDL,ToWinX(10),ToWinY(22),ToWinL(12*23+16),ToWinH(16));
}


void Copier_trame_predefinie(byte Indice)
{
  short i,j;

  for (j=0; j<16; j++)
    for (i=0; i<16; i++)
      Trame[i][j]=(TRAME_PREDEFINIE[Indice][j]>>(15-i))&1;
  Trame_Largeur=16;
  Trame_Hauteur=16;
}


void Inverser_trame(void)
{
  byte Pos_X,Pos_Y;

  for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
    for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
      Trame[Pos_X][Pos_Y]=!(Trame[Pos_X][Pos_Y]);
}


void Bouton_Trame_Menu(void)
{
  short Bouton_clicke;
  short Indice;
  short Pos_X;
  short Pos_Y;
  short Old_Pos_X=0;
  short Old_Pos_Y=0;
  short Orig_X;
  short Orig_Y;
  static byte Octet_insere=0;
  struct Fenetre_Bouton_normal * Bouton_Octet_insere;
  char  Chaine[3];
  byte  Temp; // Octet temporaire servant à n'importe quoi
  short Old_Trame_Largeur=Trame_Largeur;
  short Old_Trame_Hauteur=Trame_Hauteur;
  byte  Old_Trame[16][16];
  short Preview_Debut_X; // |  Données précalculées
  short Preview_Debut_Y; // |_ pour la preview
  short Preview_Fin_X;   // |  => plus grande
  short Preview_Fin_Y;   // |  rapidité.


  memcpy(Old_Trame,Trame,256);

  Ouvrir_fenetre(290,179,"Sieve");

  Preview_Debut_X=Fenetre_Pos_X+(Menu_Facteur_X*230);
  Preview_Debut_Y=Fenetre_Pos_Y+(Menu_Facteur_Y*78);
  Preview_Fin_X=Preview_Debut_X+(Menu_Facteur_X*51);
  Preview_Fin_Y=Preview_Debut_Y+(Menu_Facteur_Y*71);

  Fenetre_Afficher_cadre      (  7, 65,130,43);
  Fenetre_Afficher_cadre      (  7,110,130,43);
  Fenetre_Afficher_cadre_creux(142, 68, 82,82);
  Fenetre_Afficher_cadre_creux(229, 77, 53,73);

  Print_dans_fenetre(228, 68,"Preview",CM_Fonce,CM_Clair);
  Print_dans_fenetre( 27, 83,"Scroll" ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 23,120,"Width:" ,CM_Fonce,CM_Clair);
  Print_dans_fenetre( 15,136,"Height:",CM_Fonce,CM_Clair);

  Fenetre_Definir_bouton_special(143,69,80,80);                     // 1

  Fenetre_Definir_bouton_normal(175,157,51,14,"Cancel",0,1,SDLK_ESCAPE); // 2
  Fenetre_Definir_bouton_normal(230,157,51,14,"OK"    ,0,1,SDLK_RETURN); // 3

  Fenetre_Definir_bouton_normal(  8,157,51,14,"Clear" ,1,1,SDLK_c); // 4
  Fenetre_Definir_bouton_normal( 63,157,51,14,"Invert",1,1,SDLK_i); // 5

  Fenetre_Definir_bouton_normal(  8,46,131,14,"Get from brush"   ,1,1,SDLK_g); // 6
  Fenetre_Definir_bouton_normal(142,46,139,14,"Transfer to brush",1,1,SDLK_t); // 7

  Fenetre_Definir_bouton_normal(109,114,11,11,"\030",0,1,SDLK_UP|MOD_SHIFT); // 8
  Fenetre_Definir_bouton_normal(109,138,11,11,"\031",0,1,SDLK_DOWN|MOD_SHIFT); // 9
  Fenetre_Definir_bouton_normal( 97,126,11,11,"\033",0,1,SDLK_LEFT|MOD_SHIFT); // 10
  Fenetre_Definir_bouton_normal(121,126,11,11,"\032",0,1,SDLK_RIGHT|MOD_SHIFT); // 11
  Fenetre_Definir_bouton_normal(109,126,11,11,""    ,0,1,SDLK_INSERT); // 12
  Bouton_Octet_insere=Fenetre_Liste_boutons_normal;
  Block(Fenetre_Pos_X+(Menu_Facteur_X*(Bouton_Octet_insere->Pos_X+2)),
        Fenetre_Pos_Y+(Menu_Facteur_Y*(Bouton_Octet_insere->Pos_Y+2)),
        Menu_Facteur_X*7, Menu_Facteur_Y*7, (Octet_insere)?CM_Blanc:CM_Noir);

  Fenetre_Definir_bouton_normal(109, 69,11,11,"\030",0,1,SDLK_UP); // 13
  Fenetre_Definir_bouton_normal(109, 93,11,11,"\031",0,1,SDLK_DOWN); // 14
  Fenetre_Definir_bouton_normal( 97, 81,11,11,"\033",0,1,SDLK_LEFT); // 15
  Fenetre_Definir_bouton_normal(121, 81,11,11,"\032",0,1,SDLK_RIGHT); // 16

  for (Indice=0; Indice<12; Indice++)
    Fenetre_Definir_bouton_normal((Indice*23)+8,20,20,20,"",0,1,SDLK_F1+Indice); // 17 -> 28
  Dessiner_trames_predefinies();

  Orig_X=Fenetre_Pos_X+(Menu_Facteur_X*Fenetre_Liste_boutons_special->Pos_X);
  Orig_Y=Fenetre_Pos_Y+(Menu_Facteur_Y*Fenetre_Liste_boutons_special->Pos_Y);

  Num2str(Trame_Largeur,Chaine,2);
  Print_dans_fenetre(71,120,Chaine,CM_Noir,CM_Clair);
  Num2str(Trame_Hauteur,Chaine,2);
  Print_dans_fenetre(71,136,Chaine,CM_Noir,CM_Clair);
  Dessiner_trame_zoomee(Orig_X,Orig_Y);


  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    switch (Bouton_clicke)
    {
      case -1 :
      case  0 :
        break;

      case  1 : // Zone de dessin de la trame
        /* // Version qui n'accepte pas les clicks sur la grille
        Pos_X=(Mouse_X-Orig_X)/Menu_Facteur_X;
        Pos_Y=(Mouse_Y-Orig_Y)/Menu_Facteur_Y;
        if ( (Pos_X%5<4) && (Pos_Y%5<4) )
        {
          Pos_X/=5;
          Pos_Y/=5;
          if ( (Pos_X<Trame_Largeur) && (Pos_Y<Trame_Hauteur) )
        */
        Pos_X=(Mouse_X-Orig_X)/(Menu_Facteur_X*5);
        Pos_Y=(Mouse_Y-Orig_Y)/(Menu_Facteur_Y*5);
        if ( (Pos_X<Trame_Largeur) && (Pos_Y<Trame_Hauteur) )
        {
          Temp=(Mouse_K==A_GAUCHE);
          if ( (Pos_X!=Old_Pos_X) || (Pos_Y!=Old_Pos_Y)
            || (Trame[Pos_X][Pos_Y]!=Temp) )
          {
            Old_Pos_X=Pos_X;
            Old_Pos_Y=Pos_Y;
            Trame[Pos_X][Pos_Y]=Temp;
            Pos_X=Menu_Facteur_X*5;
            Pos_Y=Menu_Facteur_Y*5;
            Effacer_curseur();
            if (Temp)
              Temp=CM_Blanc;
            else
              Temp=CM_Noir;
            // Affichage du pixel dans la fenêtre zoomée
            Block(Orig_X+(Old_Pos_X*Pos_X), Orig_Y+(Old_Pos_Y*Pos_Y),
                  Pos_X-Menu_Facteur_X, Pos_Y-Menu_Facteur_Y, Temp);
            // Mise à jour de la preview
            for (Pos_Y=Preview_Debut_Y+Old_Pos_Y; Pos_Y<Preview_Fin_Y; Pos_Y+=Trame_Hauteur)
              for (Pos_X=Preview_Debut_X+Old_Pos_X; Pos_X<Preview_Fin_X; Pos_X+=Trame_Largeur)
                Pixel(Pos_X,Pos_Y,Temp);
            Afficher_curseur();
          }
        }
        break;

      case  2 : // Cancel
      case  3 : // OK
        break;

      case  4 : // Clear
        Effacer_curseur();
        memset(Trame,Octet_insere,256);
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case  5 : // Invert
        Effacer_curseur();
        Inverser_trame();
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case  6 : // Get from brush
        Effacer_curseur();
        Trame_Largeur=(Brosse_Largeur>16)?16:Brosse_Largeur;
        Trame_Hauteur=(Brosse_Hauteur>16)?16:Brosse_Hauteur;
        for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
          for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
            Trame[Pos_X][Pos_Y]=(Lit_pixel_dans_brosse(Pos_X,Pos_Y)!=Back_color);
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case  7 : // Transfer to brush
        free(Brosse);
        Brosse_Largeur=Trame_Largeur;
        Brosse_Hauteur=Trame_Hauteur;
        Brosse=(byte *)malloc(((long)Brosse_Hauteur)*Brosse_Largeur);
        for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
          for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
            Pixel_dans_brosse(Pos_X,Pos_Y,(Trame[Pos_X][Pos_Y])?Fore_color:Back_color);
        Brosse_Decalage_X=(Brosse_Largeur>>1);
        Brosse_Decalage_Y=(Brosse_Hauteur>>1);
        break;

      case  8 : // Réduire hauteur
        if (Trame_Hauteur>1)
        {
          Effacer_curseur();
          Trame_Hauteur--;
          Num2str(Trame_Hauteur,Chaine,2);
          Print_dans_fenetre(71,136,Chaine,CM_Noir,CM_Clair);
          Dessiner_trame_zoomee(Orig_X,Orig_Y);
          Afficher_curseur();
        }
        break;

      case  9 : // Agrandir hauteur
        if (Trame_Hauteur<16)
        {
          Effacer_curseur();
          for (Indice=0; Indice<Trame_Largeur; Indice++)
            Trame[Indice][Trame_Hauteur]=Octet_insere;
          Trame_Hauteur++;
          Num2str(Trame_Hauteur,Chaine,2);
          Print_dans_fenetre(71,136,Chaine,CM_Noir,CM_Clair);
          Dessiner_trame_zoomee(Orig_X,Orig_Y);
          Afficher_curseur();
        }
        break;

      case 10 : // Réduire largeur
        if (Trame_Largeur>1)
        {
          Effacer_curseur();
          Trame_Largeur--;
          Num2str(Trame_Largeur,Chaine,2);
          Print_dans_fenetre(71,120,Chaine,CM_Noir,CM_Clair);
          Dessiner_trame_zoomee(Orig_X,Orig_Y);
          Afficher_curseur();
        }
        break;

      case 11 : // Agrandir largeur
        if (Trame_Largeur<16)
        {
          Effacer_curseur();
          for (Indice=0; Indice<Trame_Hauteur; Indice++)
            Trame[Trame_Largeur][Indice]=Octet_insere;
          Trame_Largeur++;
          Num2str(Trame_Largeur,Chaine,2);
          Print_dans_fenetre(71,120,Chaine,CM_Noir,CM_Clair);
          Dessiner_trame_zoomee(Orig_X,Orig_Y);
          Afficher_curseur();
        }
        break;

      case 12 : // Toggle octets insérés
        Effacer_curseur();
        Octet_insere=!Octet_insere;
        Block(Fenetre_Pos_X+(Menu_Facteur_X*(Bouton_Octet_insere->Pos_X+2)),
              Fenetre_Pos_Y+(Menu_Facteur_Y*(Bouton_Octet_insere->Pos_Y+2)),
              Menu_Facteur_X*7, Menu_Facteur_Y*7, (Octet_insere)?CM_Blanc:CM_Noir);
        Afficher_curseur();
        break;

      case 13 : // Scroll vers le haut
        Effacer_curseur();
        for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
        {
          Temp=Trame[Pos_X][0]; // Octet temporaire
          for (Pos_Y=1; Pos_Y<Trame_Hauteur; Pos_Y++)
            Trame[Pos_X][Pos_Y-1]=Trame[Pos_X][Pos_Y];
          Trame[Pos_X][Trame_Hauteur-1]=Temp;
        }
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case 14 : // Scroll vers le bas
        Effacer_curseur();
        for (Pos_X=0; Pos_X<Trame_Largeur; Pos_X++)
        {
          Temp=Trame[Pos_X][Trame_Hauteur-1]; // Octet temporaire
          for (Pos_Y=Trame_Hauteur-1; Pos_Y>0; Pos_Y--)
            Trame[Pos_X][Pos_Y]=Trame[Pos_X][Pos_Y-1];
          Trame[Pos_X][0]=Temp;
        }
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case 15 : // Scroll vers la gauche
        Effacer_curseur();
        for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
        {
          Temp=Trame[0][Pos_Y]; // Octet temporaire
          for (Pos_X=1; Pos_X<Trame_Largeur; Pos_X++)
            Trame[Pos_X-1][Pos_Y]=Trame[Pos_X][Pos_Y];
          Trame[Trame_Largeur-1][Pos_Y]=Temp;
        }
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      case 16 : // Scroll vers la droite
        Effacer_curseur();
        for (Pos_Y=0; Pos_Y<Trame_Hauteur; Pos_Y++)
        {
          Temp=Trame[Trame_Largeur-1][Pos_Y]; // Octet temporaire
          for (Pos_X=Trame_Largeur-1; Pos_X>0; Pos_X--)
            Trame[Pos_X][Pos_Y]=Trame[Pos_X-1][Pos_Y];
          Trame[0][Pos_Y]=Temp;
        }
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
        break;

      default : // Boutons de trames prédéfinies
        Effacer_curseur();
        Copier_trame_predefinie(Bouton_clicke-17);
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Num2str(Trame_Largeur,Chaine,2);
        Print_dans_fenetre(71,120,Chaine,CM_Noir,CM_Clair);
        Num2str(Trame_Hauteur,Chaine,2);
        Print_dans_fenetre(71,136,Chaine,CM_Noir,CM_Clair);
        Dessiner_trame_zoomee(Orig_X,Orig_Y);
        Afficher_curseur();
    }
  }
  while ( (Bouton_clicke!=2) && (Bouton_clicke!=3) );


  Fermer_fenetre();

  if (Bouton_clicke==2) // Cancel
  {
    Trame_Largeur=Old_Trame_Largeur;
    Trame_Hauteur=Old_Trame_Hauteur;
    memcpy(Trame,Old_Trame,256);
  }

  if ( (Bouton_clicke==3) && (!Trame_Mode) ) // OK
    Bouton_Trame_Mode();

  Afficher_curseur();
}


// -- Gestion des boutons de polygone vide et plein -------------------------

void Bouton_Polygone(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_POLYGONE);
  Afficher_curseur();
}


void Bouton_Polyform(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_POLYFORM);
  Afficher_curseur();
}


void Bouton_Polyfill(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_POLYFILL);
  Afficher_curseur();
}


void Bouton_Filled_polyform(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_FILLED_POLYFORM);
  Afficher_curseur();
}


// -- Boutons d'ajustement de l'image ---------------------------------------

void Bouton_Ajuster(void)
{
  Effacer_curseur();
  Demarrer_pile_operation(OPERATION_SCROLL);
  Afficher_curseur();
}



// -- Menu des effets (Shade, Stencil, etc...) ------------------------------

void Afficher_sprite_effet(short Numero_sprite, short Debut_X, short Debut_Y)
{
  short X,Y,Pos_X,Pos_Y;

  for (Y=0,Pos_Y=Debut_Y;Y<HAUTEUR_SPRITE_MENU;Y++,Pos_Y++)
    for (X=0,Pos_X=Debut_X;X<LARGEUR_SPRITE_MENU;X++,Pos_X++)
      Pixel_dans_fenetre(Pos_X,Pos_Y,SPRITE_EFFET[Numero_sprite][Y][X]);

  SDL_UpdateRect(Ecran_SDL,ToWinX(Debut_X),ToWinY(Debut_Y),LARGEUR_SPRITE_MENU*Menu_Facteur_X,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y);
}


void Afficher_etat_effet(short X, short Y, char * Libelle, byte Etat)
{
  Block(Fenetre_Pos_X+(X*Menu_Facteur_X),Fenetre_Pos_Y+(Y*Menu_Facteur_Y),
        12*Menu_Facteur_X,Menu_Facteur_Y<<3,CM_Clair);

  Print_dans_fenetre(X,Y,Libelle,(Etat)?CM_Blanc:CM_Noir,CM_Clair);
  if (Etat)
    Print_dans_fenetre(X+56,Y,":ON ",CM_Blanc,CM_Clair);
  else
    Print_dans_fenetre(X+56,Y,":OFF",CM_Noir,CM_Clair);
}

void Afficher_etat_effets(void)
{
  Afficher_etat_effet( 30, 24,"Shade"  ,Shade_Mode);
  Afficher_etat_effet( 30, 43,"Q-shade",Quick_shade_Mode);
  Afficher_etat_effet( 30, 62,"Transp.",Colorize_Mode);
  Afficher_etat_effet( 30, 81,"Smooth" ,Smooth_Mode);
  Afficher_etat_effet( 30,100,"Smear"  ,Smear_Mode);
  Afficher_etat_effet(176, 24,"Stencil",Stencil_Mode);
  Afficher_etat_effet(176, 43,"Mask"   ,Mask_Mode);
  Afficher_etat_effet(176, 62,"Sieve"  ,Trame_Mode);
  Afficher_etat_effet(176, 81,"Grid"   ,Snap_Mode);
  Afficher_etat_effet(176,100,"Tiling" ,Tiling_Mode);
}


void Afficher_etat_feedback(void)
{
  Print_dans_fenetre(159,134,(Config.FX_Feedback)?"YES":" NO",CM_Noir,CM_Clair);
}


void Bouton_Effets(void)
{
  short Bouton_clicke;
  byte Sortie_par_close=0;

  Ouvrir_fenetre(270,152,"Drawing modes (effects)");

  Fenetre_Definir_bouton_normal(  7, 19, 16,16,"",0,1,SDLK_F1); // 1
  Fenetre_Definir_bouton_normal(  7, 38, 16,16,"",0,1,SDLK_F2); // 2
  Fenetre_Definir_bouton_normal(  7, 57, 16,16,"",0,1,SDLK_F3); // 3
  Fenetre_Definir_bouton_normal(  7, 76, 16,16,"",0,1,SDLK_F4); // 4
  Fenetre_Definir_bouton_normal(  7, 95, 16,16,"",0,1,SDLK_F5); // 5
  Fenetre_Definir_bouton_normal(153, 19, 16,16,"",0,1,SDLK_F6); // 6
  Fenetre_Definir_bouton_normal(153, 38, 16,16,"",0,1,SDLK_F7); // 7
  Fenetre_Definir_bouton_normal(153, 57, 16,16,"",0,1,SDLK_F8); // 8
  Fenetre_Definir_bouton_normal(153, 76, 16,16,"",0,1,SDLK_F9); // 9
  Fenetre_Definir_bouton_normal(153, 95, 16,16,"",0,1,SDLK_F10); // 10
  Fenetre_Definir_bouton_normal(195,131, 68,14,"Close",0,1,SDLK_RETURN); // 11
  Fenetre_Definir_bouton_normal(  7,131, 68,14,"All off",0,1,SDLK_DELETE); // 12
  Fenetre_Definir_bouton_normal( 83,131,104,14,"Feedback:   ",1,1,SDLK_f); // 13
  Afficher_etat_feedback();
  Afficher_sprite_effet(0,  8,20);
  Afficher_sprite_effet(0,  8,39);
  Afficher_sprite_effet(1,  8,58);
  Afficher_sprite_effet(2,  8,77);
  Afficher_sprite_effet(8,  8,96);
  Afficher_sprite_effet(4,154,20);
  Afficher_sprite_effet(7,154,39);
  Afficher_sprite_effet(5,154,58);
  Afficher_sprite_effet(6,154,77);
  Afficher_sprite_effet(3,154,96);
  Afficher_etat_effets();

  Print_dans_fenetre(12,117,"Click: Left:Switch / Right:Edit",CM_Fonce,CM_Clair);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();

    if (Touche==SDLK_ESCAPE)
      Bouton_clicke=11;

    switch (Bouton_clicke)
    {
      case 1 : // Shade
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Shade_Mode();
          Effacer_curseur();
          Afficher_etat_effets();
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Shade_Menu();
          Bouton_clicke=11;
        }
        break;
      case 2 : // Quick-shade
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Quick_shade_Mode();
          Effacer_curseur();
          Afficher_etat_effets();
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Quick_shade_Menu();
          Bouton_clicke=11;
        }
        break;
      case 3 : // Colorize / Transparency
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Colorize_Mode();
          Effacer_curseur();
          Afficher_etat_effets();
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Colorize_Menu();
          Bouton_clicke=11;
        }
        break;
      case 4 : // Smooth
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Smooth_Mode();
          Effacer_curseur();
          Afficher_etat_effets();
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Smooth_Menu();
          Bouton_clicke=11;
        }
        break;
      case 5 : // Smear
        Bouton_Smear_Mode();
        Effacer_curseur();
        Afficher_etat_effets();
        Afficher_curseur();
        break;
      case 6 : // Stencil
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Stencil_Mode();
          Effacer_curseur();
          Afficher_etat_effet(176,24,"Stencil",Stencil_Mode);
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Menu_Stencil();
          Bouton_clicke=11;
        }
        break;
      case 7 : // Mask
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Mask_Mode();
          Effacer_curseur();
          Afficher_etat_effet(176,43,"Mask",Mask_Mode);
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Mask_Menu();
          Bouton_clicke=11;
        }
        break;
      case 8 : // Sieve
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Trame_Mode();
          Effacer_curseur();
          Afficher_etat_effet(176,62,"Sieve",Trame_Mode);
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Trame_Menu();
          Bouton_clicke=11;
        }
        break;
      case 9 : // Grid
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Snap_Mode();
          Effacer_curseur();
          Afficher_etat_effet(176,81,"Grid",Snap_Mode);
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Menu_Grille();
          Bouton_clicke=11;
        }
        break;
      case 10 : // Tiling
        if (Fenetre_Attribut1==A_GAUCHE)
        {
          Bouton_Tiling_Mode();
          Effacer_curseur();
          Afficher_etat_effets();
          Afficher_curseur();
        }
        else
        {
          Fermer_fenetre();
          Afficher_curseur();
          Bouton_Tiling_Menu();
          Bouton_clicke=11;
        }
        break;
      case 11 : // Close
        Sortie_par_close=1;
        break;
      case 12 : // All off
        Fonction_effet=Aucun_effet;
        Shade_Mode=0;
        Quick_shade_Mode=0;
        Colorize_Mode=0;
        Smooth_Mode=0;
        Tiling_Mode=0;
        Smear_Mode=0;
        Stencil_Mode=0;
        Mask_Mode=0;
        Trame_Mode=0;
        Snap_Mode=0;
        Effacer_curseur();
        Afficher_etat_effets();
        Afficher_curseur();
        break;
      case 13 : // Feedback (pour Colorize et Shade)
        if ((Config.FX_Feedback=!Config.FX_Feedback)) //!!!
          FX_Feedback_Ecran=Principal_Ecran;
        else
          FX_Feedback_Ecran=Ecran_backup;
        Effacer_curseur();
        Afficher_etat_feedback();
        Afficher_curseur();
        break;
    }
  }
  while (Bouton_clicke!=11);

  if (Sortie_par_close)
    Fermer_fenetre();
  else
    Effacer_curseur();

  if (!(Shade_Mode||Quick_shade_Mode||Colorize_Mode||Smooth_Mode||Tiling_Mode||Smear_Mode||Stencil_Mode||Mask_Mode||Trame_Mode||Snap_Mode))
    Desenclencher_bouton(BOUTON_EFFETS);

  Afficher_curseur();
}





/*
BOUTON_GRADRECT
-BOUTON_SPHERES       (Ellipses dégradées à améliorer)
BOUTON_TEXTE
-BOUTON_AJUSTER       (Effets sur l'image)
-BOUTON_EFFETS_BROSSE (Distort, Rot. any angle)
*/
