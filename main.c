/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
    Copyright 2008 Franck Charlet
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
#define VARIABLES_GLOBALES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include "init.h"
#include "boutons.h"
#include "moteur.h"
#include "pages.h"
#include "files.h"
#include "loadsave.h"
#include "sdlscreen.h"
#include "erreurs.h"
#include "readini.h"
#include "saveini.h"
#include "io.h"
#include "texte.h"
#include "setup.h"
#include "windows.h"
#include "brush.h"
#include "palette.h"

#if defined(__WIN32__)
    #include <windows.h>
    #include <shlwapi.h>
    #define chdir(dir) SetCurrentDirectory(dir)
#elif defined(__macosx__)
    #import <corefoundation/corefoundation.h>
    #import <sys/param.h>
#elif defined(__FreeBSD__)
    #import <sys/param.h>
#elif defined(__AROS__) || defined(__BEOS__) || defined(__MORPHOS__) || defined(__GP2X__)
    #include "realpath.h"
#endif

byte Ancien_nb_lignes;                // Ancien nombre de lignes de l'écran


//--- Affichage de la syntaxe, et de la liste des modes vidéos disponibles ---
void Afficher_syntaxe(void)
{
  int Indice_mode;
  printf("Syntax: GFX2 [<arguments>] [<picture>]\n\n");
  printf("<arguments> can be:]\n");
  printf("\t/? /h /help        for this help screen\n");
  printf("\t/wide              to emulate a video mode with wide pixels (2x1)\n");
  printf("\t/tall              to emulate a video mode with tall pixels (1x2)\n");
  printf("\t/double            to emulate a video mode with double pixels (2x2)\n");
  printf("\t/mode <videomode>  to set a video mode\n\n");
  printf("Available video modes:\n\n");
  for (Indice_mode=0; Indice_mode<Nb_modes_video; Indice_mode++)
    printf("\t%s\n",Libelle_mode(Indice_mode));
}


// ---------------------------- Sortie impromptue ----------------------------
void Erreur_fonction(int Code, const char *Nom_fichier, int Numero_ligne, const char *Nom_fonction)
{
  T_Palette Palette_temporaire;
  int       Indice;
  printf("Error number %d occured in file %s, line %d, function %s.\n", Code, Nom_fichier,Numero_ligne,Nom_fonction);

  if (Code==0)
  {
    // L'erreur 0 n'est pas une vraie erreur, elle fait seulement un flash rouge de l'écran pour dire qu'il y a un problème.
    // Toutes les autres erreurs déclenchent toujours une sortie en catastrophe du programme !
    memcpy(Palette_temporaire,Principal_Palette,sizeof(T_Palette));
    for (Indice=0;Indice<=255;Indice++)
      Palette_temporaire[Indice].R=255;
    Set_palette(Palette_temporaire);
    SDL_Delay(500);
    Set_palette(Principal_Palette);
  }
  else
  {
    switch (Code)
    {
      case ERREUR_GUI_ABSENT         : printf("Error: File gfx2gui.gif is missing!\n");
                                       printf("This program cannot run without this file.\n");
                                       break;
      case ERREUR_GUI_CORROMPU       : printf("Error: File gfx2gui.gif is corrupt!\n");
                                       printf("This program cannot run without a correct version of this file.\n");
                                       break;
      case ERREUR_INI_ABSENT         : printf("Error: File gfx2def.ini is missing!\n");
                                       printf("This program cannot run without this file.\n");
                                       break;
      case ERREUR_CFG_ABSENT         : printf("Error: File GFX2.CFG is missing!\n");
                                       printf("Please run GFXCFG to create it.\n");
                                       break;
      case ERREUR_CFG_CORROMPU       : printf("Error: File GFX2.CFG is corrupt!\n");
                                       printf("Please run GFXCFG to make a valid file.\n");
                                       break;
      case ERREUR_CFG_ANCIEN         : printf("Error: File GFX2.CFG is from another version of GrafX2.\n");
                                       printf("Please run GFXCFG to update this file.\n");
                                       break;
      case ERREUR_MEMOIRE            : printf("Error: Not enough memory!\n\n");
                                       printf("You should try exiting other programs to free some bytes for Grafx2.\n\n");
                                       break;
      case ERREUR_DRIVER_SOURIS      : printf("Error: No mouse detected!\n");
                                       printf("Check if a mouse driver is installed and if your mouse is correctly connected.\n");
                                       break;
      case ERREUR_MODE_INTERDIT      : printf("Error: The requested video mode has been disabled from the resolution menu!\n");
                                       printf("If you want to run the program in this mode, you'll have to start it with an\n");
                                       printf("enabled mode, then enter the resolution menu and enable the mode you want.\n");
                                       printf("Check also if the 'Default_video_mode' parameter in GFX2.INI is correct.\n");
                                       break;
      case ERREUR_LIGNE_COMMANDE     : printf("Error: Invalid parameter or file not found.\n\n");
                                       Afficher_syntaxe();
                                       break;
      case ERREUR_SAUVEGARDE_CFG     : printf("Error: Write error while saving settings!\n");
                                       printf("Settings have not been saved correctly, and the GFX2.CFG file may have been\n");
                                       printf("corrupt. If so, please run GFXCFG to make a new valid file.\n");
                                       break;
      case ERREUR_REPERTOIRE_DISPARU : printf("Error: Directory you ran the program from not found!\n");
                                       break;
      case ERREUR_INI_CORROMPU       : printf("Error: File GFX2.INI is corrupt!\n");
                                       printf("It contains bad values at line %d.\n",Ligne_INI);
                                       printf("You can re-generate it by deleting the file and running GrafX2 again.\n");
                                       break;
      case ERREUR_SAUVEGARDE_INI     : printf("Error: Cannot rewrite file GFX2.INI!\n");
                                       break;
      case ERREUR_SORRY_SORRY_SORRY  : printf("Error: Sorry! Sorry! Sorry! Please forgive me!\n");
                                       break;
    }

    SDL_Quit();
    exit(Code);
  }
}

// --------------------- Analyse de la ligne de commande ---------------------
void Analyse_de_la_ligne_de_commande(int argc,char * argv[])
{
  char *Buffer ;
  int Indice;


  Un_fichier_a_ete_passe_en_parametre=0;
  Une_resolution_a_ete_passee_en_parametre=0;
  
  Resolution_actuelle=Config.Resolution_par_defaut;
  
  for (Indice=1; Indice<argc; Indice++)
  {
    if ( !strcmp(argv[Indice],"/?") ||
         !strcmp(argv[Indice],"/h") ||
         !strcmp(argv[Indice],"/H") )
    {
      // help
      Afficher_syntaxe();
      exit(0);
    }
    else if ( !strcmp(argv[Indice],"/mode") )
    {
      // mode
      Indice++;
      if (Indice<argc)
      {    
        Une_resolution_a_ete_passee_en_parametre = 1;
        Resolution_actuelle=Conversion_argument_mode(argv[Indice]);
        if (Resolution_actuelle == -1)
        {
          Erreur(ERREUR_LIGNE_COMMANDE);
          Afficher_syntaxe();
          exit(0);
        }
        if ((Mode_video[Resolution_actuelle].Etat & 0x7F) == 3)
        {
          Erreur(ERREUR_MODE_INTERDIT);
          exit(0);
        }
      }
      else
      {
        Erreur(ERREUR_LIGNE_COMMANDE);
        Afficher_syntaxe();
        exit(0);
      }
    }
    else if ( !strcmp(argv[Indice],"/tall") )
    {
      Pixel_ratio = PIXEL_TALL;
    }
    else if ( !strcmp(argv[Indice],"/wide") )
    {
      Pixel_ratio = PIXEL_WIDE;
    }
    else if ( !strcmp(argv[Indice],"/double") )
    {
      Pixel_ratio = PIXEL_DOUBLE;
    }
    else if ( !strcmp(argv[Indice],"/rgb") )
    {
      // echelle des composants RGB
      Indice++;
      if (Indice<argc)
      {
        int Grad;
        Grad = atoi(argv[Indice]);
        if (Grad < 2 || Grad > 256)
        {
          Erreur(ERREUR_LIGNE_COMMANDE);
          Afficher_syntaxe();
          exit(0);
        }
        Set_Palette_RGB_Scale(Grad);
      }
      else
      {
        Erreur(ERREUR_LIGNE_COMMANDE);
        Afficher_syntaxe();
        exit(0);
      }
    }
    else
    {
      // Si ce n'est pas un paramètre, c'est le nom du fichier à ouvrir
      if (Un_fichier_a_ete_passe_en_parametre)
      {
        // plusieurs noms de fichier en argument
        Erreur(ERREUR_LIGNE_COMMANDE);
        Afficher_syntaxe();
        exit(0);
      }
      else if (Fichier_existe(argv[Indice]))
      {
        Un_fichier_a_ete_passe_en_parametre=1;

        // On récupère le chemin complet du paramètre
        // Et on découpe ce chemin en répertoire(path) + fichier(.ext)
        #if defined(__WIN32__)
          Buffer=_fullpath(NULL,argv[Indice],TAILLE_CHEMIN_FICHIER);
        #else
          Buffer=realpath(argv[Indice],NULL);
        #endif
        Extraire_chemin(Principal_Repertoire_fichier, Buffer);
        Extraire_nom_fichier(Principal_Nom_fichier, Buffer);
        free(Buffer);
        chdir(Principal_Repertoire_fichier);
      }
      else
      {
        Erreur(ERREUR_LIGNE_COMMANDE);
        Afficher_syntaxe();
        exit(0);
      }
    }
  }
}

// ------------------------ Initialiser le programme -------------------------
void Initialisation_du_programme(int argc,char * argv[])
{
  int Temp;
  int Mode_dans_lequel_on_demarre;
  char Repertoire_du_programme[TAILLE_CHEMIN_FICHIER];

  // On crée dès maintenant les descripteurs des listes de pages pour la page
  // principale et la page de brouillon afin que leurs champs ne soient pas
  // invalide lors des appels aux multiples fonctions manipulées à
  // l'initialisation du programme.
  Principal_Backups=(S_Liste_de_pages *)malloc(sizeof(S_Liste_de_pages));
  Brouillon_Backups=(S_Liste_de_pages *)malloc(sizeof(S_Liste_de_pages));
  Initialiser_S_Liste_de_pages(Principal_Backups);
  Initialiser_S_Liste_de_pages(Brouillon_Backups);

  // Determine the executable directory
  Set_Program_Directory(argv[0],Repertoire_du_programme);
  // Choose directory for data (read only)
  Set_Data_Directory(Repertoire_du_programme,Repertoire_des_donnees);
  // Choose directory for settings (read/write)
  Set_Config_Directory(Repertoire_du_programme,Repertoire_de_configuration);

  // On détermine le répertoire courant:
  Determiner_repertoire_courant();

  // On en profite pour le mémoriser dans le répertoire principal:
  strcpy(Repertoire_initial,Principal_Repertoire_courant);

  // On initialise les données sur le nom de fichier de l'image principale:
  strcpy(Principal_Repertoire_fichier,Principal_Repertoire_courant);
  strcpy(Principal_Nom_fichier,"NO_NAME.GIF");
  Principal_Format_fichier=FORMAT_PAR_DEFAUT;
  // On initialise les données sur le nom de fichier de l'image de brouillon:
  strcpy(Brouillon_Repertoire_courant,Principal_Repertoire_courant);
  strcpy(Brouillon_Repertoire_fichier,Principal_Repertoire_fichier);
  strcpy(Brouillon_Nom_fichier       ,Principal_Nom_fichier);
  Brouillon_Format_fichier    =Principal_Format_fichier;
  strcpy(Brosse_Repertoire_courant,Principal_Repertoire_courant);
  strcpy(Brosse_Repertoire_fichier,Principal_Repertoire_fichier);
  strcpy(Brosse_Nom_fichier       ,Principal_Nom_fichier);
  Brosse_Format_fichier    =Principal_Format_fichier;

  // On initialise ce qu'il faut pour que les fileselects ne plantent pas:
  Liste_du_fileselect=NULL;       // Au début, il n'y a pas de fichiers dans la liste
  Principal_File_list_Position=0; // Au début, le fileselect est en haut de la liste des fichiers
  Principal_File_list_Decalage=0; // Au début, le fileselect est en haut de la liste des fichiers
  Principal_Format=0;
  Brouillon_File_list_Position=0;
  Brouillon_File_list_Decalage=0;
  Brouillon_Format=0;
  Brosse_File_list_Position=0;
  Brosse_File_list_Decalage=0;
  Brosse_Format=0;

  // On initialise les commentaires des images à des chaînes vides
  Principal_Commentaire[0]='\0';
  Brouillon_Commentaire[0]='\0';
  Brosse_Commentaire[0]='\0';

  // On initialise d'ot' trucs
  Principal_Decalage_X=0;
  Principal_Decalage_Y=0;
  Ancien_Principal_Decalage_X=0;
  Ancien_Principal_Decalage_Y=0;
  Principal_Split=0;
  Principal_X_Zoom=0;
  Principal_Proportion_split=PROPORTION_SPLIT;
  Loupe_Mode=0;
  Loupe_Facteur=FACTEUR_DE_ZOOM_PAR_DEFAUT;
  Loupe_Hauteur=0;
  Loupe_Largeur=0;
  Loupe_Decalage_X=0;
  Loupe_Decalage_Y=0;
  Brouillon_Decalage_X=0;
  Brouillon_Decalage_Y=0;
  Ancien_Brouillon_Decalage_X=0;
  Ancien_Brouillon_Decalage_Y=0;
  Brouillon_Split=0;
  Brouillon_X_Zoom=0;
  Brouillon_Proportion_split=PROPORTION_SPLIT;
  Brouillon_Loupe_Mode=0;
  Brouillon_Loupe_Facteur=FACTEUR_DE_ZOOM_PAR_DEFAUT;
  Brouillon_Loupe_Hauteur=0;
  Brouillon_Loupe_Largeur=0;
  Brouillon_Loupe_Decalage_X=0;
  Brouillon_Loupe_Decalage_Y=0;

  // SDL

  SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
  joystick = SDL_JoystickOpen(0);
  SDL_EnableKeyRepeat(250, 32);
  SDL_EnableUNICODE(SDL_ENABLE);
  if(ALPHA_BETA[0]=='ß')
    SDL_WM_SetCaption("GrafX2 beta "POURCENTAGE_VERSION""" - USE AT YOUR OWN RISK","GrafX2");
  else
    SDL_WM_SetCaption("GrafX2 v2.00 final","GrafX2");
  {
    // Routine pour définir l'icone.
    char Chemin_icone[TAILLE_CHEMIN_FICHIER];
    SDL_Surface * Icone;
    sprintf(Chemin_icone, "%s%s", Repertoire_des_donnees, "gfx2.gif");
    Icone = IMG_Load(Chemin_icone);
    if (Icone)
    {
      byte *Masque_icone;
      int x,y;
      Masque_icone=malloc(128);
      memset(Masque_icone,0,128);
      for (y=0;y<32;y++)
        for (x=0;x<32;x++)
          if (((byte *)(Icone->pixels))[(y*32+x)] != 255)
            Masque_icone[(y*32+x)/8] |=0x80>>(x&7);
      SDL_WM_SetIcon(Icone,Masque_icone);
      free(Masque_icone);
      SDL_FreeSurface(Icone);
    }
  }
  // Texte
  Initialisation_Texte();

  // On initialise tous les modes vidéo
  Definition_des_modes_video();
  Pixel_ratio=PIXEL_SIMPLE;
  // On initialise les données sur l'état du programme:
    // Donnée sur la sortie du programme:
  Quit_demande=0;
  Sortir_du_programme=0;
    // Données sur l'état du menu:
  Pixel_dans_menu=Pixel_dans_barre_d_outil;
  Menu_visible=1;
    // Données sur les couleurs et la palette:
  Fore_color=15;
  Back_color=0;
  Couleur_debut_palette=0;
    // Données sur le curseur:
  Forme_curseur=FORME_CURSEUR_CIBLE;
  Cacher_curseur=0;
    // Données sur le pinceau:
  Pinceau_X=0;
  Pinceau_Y=0;
  Pinceau_Forme=FORME_PINCEAU_ROND;
  Cacher_pinceau=0;

  Pixel_de_chargement=Pixel_Chargement_dans_ecran_courant;

  Forcer_affichage_curseur=0;

  // On initialise tout ce qui concerne les opérations et les effets
  Operation_Taille_pile=0;
  Mode_de_dessin_en_cours=OPERATION_DESSIN_CONTINU;
  Ligne_en_cours         =OPERATION_LIGNE;
  Courbe_en_cours        =OPERATION_COURBE_3_POINTS;
  Fonction_effet=Aucun_effet;
    // On initialise les infos de la loupe:
  Loupe_Mode=0;
  Loupe_Facteur=FACTEUR_DE_ZOOM_PAR_DEFAUT;
  Initialiser_les_tables_de_multiplication();
  Table_mul_facteur_zoom=TABLE_ZOOM[2];
  Principal_Proportion_split=PROPORTION_SPLIT;
  Brouillon_Proportion_split=PROPORTION_SPLIT;
    // On initialise les infos du mode smear:
  Smear_Mode=0;
  Smear_Brosse_Largeur=LARGEUR_PINCEAU;
  Smear_Brosse_Hauteur=HAUTEUR_PINCEAU;
    // On initialise les infos du mode smooth:
  Smooth_Mode=0;
    // On initialise les infos du mode shade:
  Shade_Mode=0;     // Les autres infos du Shade sont chargées avec la config
  Quick_shade_Mode=0; // idem
    // On initialise les infos sur les dégradés:
  Traiter_pixel_de_degrade =Afficher_pixel; // Les autres infos sont chargées avec la config
    // On initialise les infos de la grille:
  Snap_Mode=0;
  Snap_Largeur=8;
  Snap_Hauteur=8;
  Snap_Decalage_X=0;
  Snap_Decalage_Y=0;
    // On initialise les infos du mode Colorize:
  Colorize_Mode=0;          // Mode colorize inactif par défaut
  Colorize_Opacite=50;      // Une interpolation de 50% par défaut
  Colorize_Mode_en_cours=0; // Par défaut, la méthode par interpolation
  Calculer_les_tables_de_Colorize();
    // On initialise les infos du mode Tiling:
  Tiling_Mode=0;  //   Pas besoin d'initialiser les décalages car ça se fait
                  // en prenant une brosse (toujours mis à 0).
    // On initialise les infos du mode Mask:
  Mask_Mode=0;

    // Infos du Spray
  Spray_Mode=1; // Mode Mono
  Spray_Size=31;
  Spray_Delay=1;
  Spray_Mono_flow=10;
  memset(Spray_Multi_flow,0,256);
  srand(time(NULL)); // On randomize un peu tout ça...

  // Initialisation des boutons
  Initialisation_des_boutons();
  // Initialisation des opérations
  Initialisation_des_operations();

  Fenetre=0;
  
  // Charger les sprites et la palette
  Charger_DAT();
  // Charger la configuration des touches
  Config_par_defaut();
  switch(Charger_CFG(1))
  {
    case ERREUR_CFG_ABSENT:
      // Pas un problème, on a les valeurs par défaut.
      break;
    case ERREUR_CFG_CORROMPU:
      DEBUG("Corrupted CFG file.",0);
      break;
    case ERREUR_CFG_ANCIEN:
      DEBUG("Unknown CFG file version, not loaded.",0);
      break;
  }
  // Charger la configuration du .INI
  Temp=Charger_INI(&Config);
  if (Temp)
    Erreur(Temp);

  // Transfert des valeurs du .INI qui ne changent pas dans des variables
  // plus accessibles:
  Palette_defaut[CM_Noir] =Coul_menu_pref[0]=Config.Coul_menu_pref[0];
  Palette_defaut[CM_Fonce]=Coul_menu_pref[1]=Config.Coul_menu_pref[1];
  Palette_defaut[CM_Clair]=Coul_menu_pref[2]=Config.Coul_menu_pref[2];
  Palette_defaut[CM_Blanc]=Coul_menu_pref[3]=Config.Coul_menu_pref[3];
  memcpy(Principal_Palette,Palette_defaut,sizeof(T_Palette));

  Calculer_couleurs_menu_optimales(Palette_defaut);

  // Infos sur les trames (Sieve)
  Trame_Mode=0;
  Copier_trame_predefinie(0);

  // On sélectionne toutes les couleurs pour le masque de copie de couleurs vers le brouillon
  memset(Masque_copie_couleurs,1,256);

  // Prise en compte de la fonte
  if (Config.Fonte)
    Fonte=Fonte_fun;
  else
    Fonte=Fonte_systeme;

  // Allocation de mémoire pour la brosse
  if (!(Brosse         =(byte *)malloc(   1*   1))) Erreur(ERREUR_MEMOIRE);
  if (!(Smear_Brosse   =(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU))) Erreur(ERREUR_MEMOIRE);

  // Pinceau
  if (!(Pinceau_Sprite=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU))) Erreur(ERREUR_MEMOIRE);
  *Pinceau_Sprite=1;
  Pinceau_Largeur=1;
  Pinceau_Hauteur=1;

  Analyse_de_la_ligne_de_commande(argc,argv);
  Mode_dans_lequel_on_demarre=Resolution_actuelle;
  Buffer_de_ligne_horizontale=NULL;
  Resolution_actuelle=-1; // On n'était pas dans un mode graphique
  
  Initialiser_mode_video(
    Mode_video[Mode_dans_lequel_on_demarre].Largeur,
    Mode_video[Mode_dans_lequel_on_demarre].Hauteur,
    Mode_video[Mode_dans_lequel_on_demarre].Fullscreen);

  Principal_Largeur_image=Largeur_ecran/Pixel_width;
  Principal_Hauteur_image=Hauteur_ecran/Pixel_height;
  Brouillon_Largeur_image=Largeur_ecran/Pixel_width;
  Brouillon_Hauteur_image=Hauteur_ecran/Pixel_height;
  // Allocation de mémoire pour les différents écrans virtuels (et brosse)
  if (Initialiser_les_listes_de_backups_en_debut_de_programme(Config.Nb_pages_Undo+1,Largeur_ecran,Hauteur_ecran)==0)
    Erreur(ERREUR_MEMOIRE);
  // On remet le nom par défaut pour la page de brouillon car il été modifié
  // par le passage d'un fichier en paramètre lors du traitement précédent.
  // Note: le fait que l'on ne modifie que les variables globales 
  // Brouillon_* et pas les infos contenues dans la page de brouillon 
  // elle-même ne m'inspire pas confiance mais ça a l'air de marcher sans 
  // poser de problèmes, alors...
  if (Un_fichier_a_ete_passe_en_parametre)
  {
    strcpy(Brouillon_Repertoire_fichier,Brouillon_Repertoire_courant);
    strcpy(Brouillon_Nom_fichier,"NO_NAME.GIF");
    Brouillon_Format_fichier=FORMAT_PAR_DEFAUT;
  }

  // Nettoyage de l'écran virtuel (les autres recevront celui-ci par copie)
  memset(Principal_Ecran,0,Principal_Largeur_image*Principal_Hauteur_image);

  // Initialisation de diverses variables par calcul:
  Calculer_donnees_loupe();
  Calculer_limites();
  Calculer_coordonnees_pinceau();

  // On affiche le menu:
  Afficher_menu();
  Afficher_pinceau_dans_menu();

  // On affiche le curseur pour débutter correctement l'état du programme:
  Afficher_curseur();

  Brouillon_Image_modifiee=0;
  Principal_Image_modifiee=0;

  // Gestionnaire de signaux, quand il ne reste plus aucun espoir
  Initialiser_sighandler();

  // Le programme débute en mode de dessin à la main
  Enclencher_bouton(BOUTON_DESSIN,A_GAUCHE);

  // On initialise la brosse initiale à 1 pixel blanc:
  Brosse_Largeur=1;
  Brosse_Hauteur=1;
  Capturer_brosse(0,0,0,0,0);
  *Brosse=CM_Blanc;
}

// ------------------------- Fermeture du programme --------------------------
void Fermeture_du_programme(void)
{
  int      Retour;

  // On libère le buffer de gestion de lignes
  free(Buffer_de_ligne_horizontale);

  // On libère le pinceau spécial
  free(Pinceau_Sprite);

  // On libère les différents écrans virtuels et brosse:
  free(Brosse);
  Nouveau_nombre_de_backups(0);
  free(Brouillon_Ecran);
  free(Principal_Ecran);

  // On prend bien soin de passer dans le répertoire initial:
  if (chdir(Repertoire_initial)!=-1)
  {
    // On sauvegarde les données dans le .CFG et dans le .INI
    if (Config.Auto_save)
    {
      Retour=Sauver_CFG();
      if (Retour)
        Erreur(Retour);
      Retour=Sauver_INI(&Config);
      if (Retour)
        Erreur(Retour);
    }
  }
  else
    Erreur(ERREUR_REPERTOIRE_DISPARU);
    
  SDL_Quit();
}


// -------------------------- Procédure principale ---------------------------
int main(int argc,char * argv[])
{
  int PhoenixTrouve=0;
  int Phoenix2Trouve=0;
  char Nom_du_fichier_Phoenix[TAILLE_CHEMIN_FICHIER];
  char Nom_du_fichier_Phoenix2[TAILLE_CHEMIN_FICHIER];

  Initialisation_du_programme(argc,argv);

  // Test de recuperation de fichiers sauvés
  strcpy(Nom_du_fichier_Phoenix,Repertoire_de_configuration);
  strcat(Nom_du_fichier_Phoenix,"phoenix.img");
  strcpy(Nom_du_fichier_Phoenix2,Repertoire_de_configuration);
  strcat(Nom_du_fichier_Phoenix2,"phoenix2.img");
  if (Fichier_existe(Nom_du_fichier_Phoenix))
    PhoenixTrouve=1;
  if (Fichier_existe(Nom_du_fichier_Phoenix2))
    Phoenix2Trouve=1;
  if (PhoenixTrouve || Phoenix2Trouve)
  {
    if (Phoenix2Trouve)
    {
      strcpy(Principal_Repertoire_fichier,Repertoire_de_configuration);
      strcpy(Principal_Nom_fichier,"phoenix2.img");
      chdir(Principal_Repertoire_fichier);
      Bouton_Reload();
      Principal_Image_modifiee=1;
      Warning_message("Spare page recovered");
      // I don't really like this, but...
      remove(Nom_du_fichier_Phoenix2);
      Bouton_Page();
    }
    if (PhoenixTrouve)
    {
      strcpy(Principal_Repertoire_fichier,Repertoire_de_configuration);
      strcpy(Principal_Nom_fichier,"phoenix.img");
      chdir(Principal_Repertoire_fichier);
      Bouton_Reload();
      Principal_Image_modifiee=1;
      Warning_message("Main page recovered");
      // I don't really like this, but...
      remove(Nom_du_fichier_Phoenix);
    }
  }
  else
  {
    if (Config.Opening_message && (!Un_fichier_a_ete_passe_en_parametre))
      Bouton_Message_initial();
    free(Logo_GrafX2); // Pas encore utilisé dans le About
  
    if (Un_fichier_a_ete_passe_en_parametre)
    {
      Bouton_Reload();
      Une_resolution_a_ete_passee_en_parametre=0;
    }
  }
  Gestion_principale();

  Fermeture_du_programme();
  return 0;
}
