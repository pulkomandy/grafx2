/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
    Copyright 2008 Yves Rizoud
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

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL/SDL_byteorder.h>
#if defined(__WIN32__)
  #include <windows.h> // GetLogicalDrives(), GetDriveType(), DRIVE_*
#endif

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "boutons.h"
#include "palette.h"
#include "aide.h"
#include "operatio.h"
#include "divers.h"
#include "erreurs.h"
#include "clavier.h"
#include "io.h"
#include "hotkeys.h"
#include "files.h"
#include "setup.h"


// Ajouter un lecteur à la liste de lecteurs
void Ajouter_lecteur(char Lettre, byte Type, char *Chemin)
{ 
  Drive[Nb_drives].Lettre=Lettre;
  Drive[Nb_drives].Type  =Type;
  Drive[Nb_drives].Chemin=(char *)malloc(strlen(Chemin)+1);
  strcpy(Drive[Nb_drives].Chemin, Chemin);
  
  Nb_drives++;
}


// Rechercher la liste et le type des lecteurs de la machine
void Rechercher_drives(void)
{
  #if defined(__amigaos4__)

  // No icons by default.
  // It's possible to add some here.

  #elif defined (__WIN32__)

  char NomLecteur[]="A:\\";
	int DriveBits = GetLogicalDrives();
	int IndiceLecteur;
  int IndiceBit;
	// Sous Windows, on a la totale, presque aussi bien que sous DOS:
	IndiceLecteur = 0;
	for (IndiceBit=0; IndiceBit<26 && IndiceLecteur<23; IndiceBit++)
	{
	  if ( (1 << IndiceBit) & DriveBits )
	  {
	    // On a ce lecteur, il faut maintenant déterminer son type "physique".
	    // pour profiter des jolies icones de X-man.
	    int TypeLecteur;
	    char CheminLecteur[]="A:\\";
	    // Cette API Windows est étrange, je dois m'y faire...
	    CheminLecteur[0]='A'+IndiceBit;
	    switch (GetDriveType(CheminLecteur))
	    {
	      case DRIVE_CDROM:
	        TypeLecteur=LECTEUR_CDROM;
	        break;
	      case DRIVE_REMOTE:
	        TypeLecteur=LECTEUR_NETWORK;
	        break;
	      case DRIVE_REMOVABLE:
	        TypeLecteur=LECTEUR_FLOPPY_3_5;
	        break;
	      case DRIVE_FIXED:
	        TypeLecteur=LECTEUR_HDD;
	        break;
	      default:
	        TypeLecteur=LECTEUR_NETWORK;
	        break;
	    }
      NomLecteur[0]='A'+IndiceBit;
	    Ajouter_lecteur(NomLecteur[0], TypeLecteur,NomLecteur);
	    IndiceLecteur++;
	  }
	}

  #else

	//Sous les différents unix, il n'y a pas de lecteurs, on va juste mettre 
	// un disque dur qui pointera vers la racine,
	// et un autre vers le home directory de l'utilisateur.
	#if defined(__BEOS__) || defined(__HAIKU__)
  	char * Home = getenv("$HOME");
  #else
	  char * Home = getenv("HOME");
  #endif	
	Ajouter_lecteur('/', LECTEUR_HDD, "/");
	Ajouter_lecteur('~', LECTEUR_HDD, Home ? Home : "");
	#endif
}

// Active un lecteur, changeant normalement le répertoire en cours.
// Renvoie 0 si ok, -1 si problème.
int ActiverLecteur(int NumeroLecteur)
{
  return chdir(Drive[NumeroLecteur].Chemin);
}

void Charger_DAT(void)
{
  FILE*  Handle;
  int  Taille_fichier;
  int  Indice;
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  byte * Fonte_temporaire;
  byte Pos_X;
  byte Pos_Y;

  struct stat Informations_Fichier;

  strcpy(Nom_du_fichier,Repertoire_des_donnees);
  strcat(Nom_du_fichier,"gfx2.dat");
  
  if(stat(Nom_du_fichier,&Informations_Fichier))
  {
    switch errno
    {
      case EACCES: puts("La permission de parcours est refusée pour un des répertoires contenu dans le chemin path."); break;
      case EBADF:  puts("filedes est un mauvais descripteur."); break;
      case EFAULT: puts("Un pointeur se trouve en dehors de l'espace d'adressage."); break;
      case ENAMETOOLONG: puts("Nom de fichier trop long."); break;
      case ENOENT: puts("The file path is empty or points to a non-existing directory."); break;
      case ENOMEM: puts("Pas assez de mémoire pour le noyau."); break;
      case ENOTDIR: puts("Un composant du chemin d'accès n'est pas un répertoire."); break;
      #ifdef ELOOP
          case ELOOP:  puts("Trop de liens symboliques rencontrés dans le chemin d'accès."); break;
      #endif
    }
  }

  Taille_fichier=Informations_Fichier.st_size;
  if (Taille_fichier<DAT_DEBUT_INI_PAR_DEFAUT)
  {
    DEBUG("Taille",0);
    Erreur(ERREUR_DAT_CORROMPU);
  }

  Handle=fopen(Nom_du_fichier,"rb");
  if (Handle==NULL)
  {
	DEBUG("Absent",0);
  	Erreur(ERREUR_DAT_ABSENT);
  }

  if (!read_bytes(Handle, Palette_defaut,sizeof(T_Palette)))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, BLOCK_MENU,LARGEUR_MENU*HAUTEUR_MENU))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, SPRITE_EFFET,LARGEUR_SPRITE_MENU*HAUTEUR_SPRITE_MENU*NB_SPRITES_EFFETS))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, SPRITE_CURSEUR,LARGEUR_SPRITE_CURSEUR*HAUTEUR_SPRITE_CURSEUR*NB_SPRITES_CURSEUR))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, SPRITE_MENU,LARGEUR_SPRITE_MENU*HAUTEUR_SPRITE_MENU*NB_SPRITES_MENU))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, SPRITE_PINCEAU,LARGEUR_PINCEAU*HAUTEUR_PINCEAU*NB_SPRITES_PINCEAU))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, SPRITE_DRIVE,LARGEUR_SPRITE_DRIVE*HAUTEUR_SPRITE_DRIVE*NB_SPRITES_DRIVES))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!(Logo_GrafX2=(byte *)malloc(231*56)))
    Erreur(ERREUR_MEMOIRE);
  if (!read_bytes(Handle, Logo_GrafX2,231*56))
    Erreur(ERREUR_DAT_CORROMPU);

  if (!read_bytes(Handle, TRAME_PREDEFINIE,2*16*NB_TRAMES_PREDEFINIES))
    Erreur(ERREUR_DAT_CORROMPU);

  // Lecture des fontes 8x8:
  if (!(Fonte_temporaire=(byte *)malloc(2048)))
    Erreur(ERREUR_MEMOIRE);

  // Lecture de la fonte système
  if (!read_bytes(Handle, Fonte_temporaire,2048))
    Erreur(ERREUR_DAT_CORROMPU);
  for (Indice=0;Indice<256;Indice++)
    for (Pos_X=0;Pos_X<8;Pos_X++)
      for (Pos_Y=0;Pos_Y<8;Pos_Y++)
        Fonte_systeme[(Indice<<6)+(Pos_X<<3)+Pos_Y]=( ((*(Fonte_temporaire+(Indice*8)+Pos_Y))&(0x80>>Pos_X)) ? 1 : 0);

  // Lecture de la fonte alternative
  if (!read_bytes(Handle, Fonte_temporaire,2048))
    Erreur(ERREUR_DAT_CORROMPU);
  for (Indice=0;Indice<256;Indice++)
    for (Pos_X=0;Pos_X<8;Pos_X++)
      for (Pos_Y=0;Pos_Y<8;Pos_Y++)
        Fonte_fun[(Indice<<6)+(Pos_X<<3)+Pos_Y]=( ((*(Fonte_temporaire+(Indice*8)+Pos_Y))&(0x80>>Pos_X)) ? 1 : 0);

  free(Fonte_temporaire);

  Fonte=Fonte_systeme;

  // Lecture de la fonte 6x8: (spéciale aide)
  if (!read_bytes(Handle, Fonte_help,315*6*8))
    Erreur(ERREUR_DAT_CORROMPU);

  // Le reste est actuellement une copie du fichier INI par défaut:
  // Pas besoin ici.

  fclose(Handle);

  Section_d_aide_en_cours=0;
  Position_d_aide_en_cours=0;

  Pinceau_predefini_Largeur[ 0]= 1;
  Pinceau_predefini_Hauteur[ 0]= 1;
  Pinceau_Type             [ 0]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 1]= 2;
  Pinceau_predefini_Hauteur[ 1]= 2;
  Pinceau_Type             [ 1]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 2]= 3;
  Pinceau_predefini_Hauteur[ 2]= 3;
  Pinceau_Type             [ 2]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 3]= 4;
  Pinceau_predefini_Hauteur[ 3]= 4;
  Pinceau_Type             [ 3]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 4]= 5;
  Pinceau_predefini_Hauteur[ 4]= 5;
  Pinceau_Type             [ 4]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 5]= 7;
  Pinceau_predefini_Hauteur[ 5]= 7;
  Pinceau_Type             [ 5]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 6]= 8;
  Pinceau_predefini_Hauteur[ 6]= 8;
  Pinceau_Type             [ 6]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 7]=12;
  Pinceau_predefini_Hauteur[ 7]=12;
  Pinceau_Type             [ 7]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 8]=16;
  Pinceau_predefini_Hauteur[ 8]=16;
  Pinceau_Type             [ 8]=FORME_PINCEAU_CARRE;

  Pinceau_predefini_Largeur[ 9]=16;
  Pinceau_predefini_Hauteur[ 9]=16;
  Pinceau_Type             [ 9]=FORME_PINCEAU_CARRE_TRAME;

  Pinceau_predefini_Largeur[10]=15;
  Pinceau_predefini_Hauteur[10]=15;
  Pinceau_Type             [10]=FORME_PINCEAU_LOSANGE;

  Pinceau_predefini_Largeur[11]= 5;
  Pinceau_predefini_Hauteur[11]= 5;
  Pinceau_Type             [11]=FORME_PINCEAU_LOSANGE;

  Pinceau_predefini_Largeur[12]= 3;
  Pinceau_predefini_Hauteur[12]= 3;
  Pinceau_Type             [12]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[13]= 4;
  Pinceau_predefini_Hauteur[13]= 4;
  Pinceau_Type             [13]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[14]= 5;
  Pinceau_predefini_Hauteur[14]= 5;
  Pinceau_Type             [14]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[15]= 6;
  Pinceau_predefini_Hauteur[15]= 6;
  Pinceau_Type             [15]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[16]= 8;
  Pinceau_predefini_Hauteur[16]= 8;
  Pinceau_Type             [16]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[17]=10;
  Pinceau_predefini_Hauteur[17]=10;
  Pinceau_Type             [17]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[18]=12;
  Pinceau_predefini_Hauteur[18]=12;
  Pinceau_Type             [18]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[19]=14;
  Pinceau_predefini_Hauteur[19]=14;
  Pinceau_Type             [19]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[20]=16;
  Pinceau_predefini_Hauteur[20]=16;
  Pinceau_Type             [20]=FORME_PINCEAU_ROND;

  Pinceau_predefini_Largeur[21]=15;
  Pinceau_predefini_Hauteur[21]=15;
  Pinceau_Type             [21]=FORME_PINCEAU_ROND_TRAME;

  Pinceau_predefini_Largeur[22]=11;
  Pinceau_predefini_Hauteur[22]=11;
  Pinceau_Type             [22]=FORME_PINCEAU_ROND_TRAME;

  Pinceau_predefini_Largeur[23]= 5;
  Pinceau_predefini_Hauteur[23]= 5;
  Pinceau_Type             [23]=FORME_PINCEAU_ROND_TRAME;

  Pinceau_predefini_Largeur[24]= 2;
  Pinceau_predefini_Hauteur[24]= 1;
  Pinceau_Type             [24]=FORME_PINCEAU_BARRE_HORIZONTALE;

  Pinceau_predefini_Largeur[25]= 3;
  Pinceau_predefini_Hauteur[25]= 1;
  Pinceau_Type             [25]=FORME_PINCEAU_BARRE_HORIZONTALE;

  Pinceau_predefini_Largeur[26]= 4;
  Pinceau_predefini_Hauteur[26]= 1;
  Pinceau_Type             [26]=FORME_PINCEAU_BARRE_HORIZONTALE;

  Pinceau_predefini_Largeur[27]= 8;
  Pinceau_predefini_Hauteur[27]= 1;
  Pinceau_Type             [27]=FORME_PINCEAU_BARRE_HORIZONTALE;

  Pinceau_predefini_Largeur[28]= 1;
  Pinceau_predefini_Hauteur[28]= 2;
  Pinceau_Type             [28]=FORME_PINCEAU_BARRE_VERTICALE;

  Pinceau_predefini_Largeur[29]= 1;
  Pinceau_predefini_Hauteur[29]= 3;
  Pinceau_Type             [29]=FORME_PINCEAU_BARRE_VERTICALE;

  Pinceau_predefini_Largeur[30]= 1;
  Pinceau_predefini_Hauteur[30]= 4;
  Pinceau_Type             [30]=FORME_PINCEAU_BARRE_VERTICALE;

  Pinceau_predefini_Largeur[31]= 1;
  Pinceau_predefini_Hauteur[31]= 8;
  Pinceau_Type             [31]=FORME_PINCEAU_BARRE_VERTICALE;

  Pinceau_predefini_Largeur[32]= 3;
  Pinceau_predefini_Hauteur[32]= 3;
  Pinceau_Type             [32]=FORME_PINCEAU_X;

  Pinceau_predefini_Largeur[33]= 5;
  Pinceau_predefini_Hauteur[33]= 5;
  Pinceau_Type             [33]=FORME_PINCEAU_X;

  Pinceau_predefini_Largeur[34]= 5;
  Pinceau_predefini_Hauteur[34]= 5;
  Pinceau_Type             [34]=FORME_PINCEAU_PLUS;

  Pinceau_predefini_Largeur[35]=15;
  Pinceau_predefini_Hauteur[35]=15;
  Pinceau_Type             [35]=FORME_PINCEAU_PLUS;

  Pinceau_predefini_Largeur[36]= 2;
  Pinceau_predefini_Hauteur[36]= 2;
  Pinceau_Type             [36]=FORME_PINCEAU_SLASH;

  Pinceau_predefini_Largeur[37]= 4;
  Pinceau_predefini_Hauteur[37]= 4;
  Pinceau_Type             [37]=FORME_PINCEAU_SLASH;

  Pinceau_predefini_Largeur[38]= 8;
  Pinceau_predefini_Hauteur[38]= 8;
  Pinceau_Type             [38]=FORME_PINCEAU_SLASH;

  Pinceau_predefini_Largeur[39]= 2;
  Pinceau_predefini_Hauteur[39]= 2;
  Pinceau_Type             [39]=FORME_PINCEAU_ANTISLASH;

  Pinceau_predefini_Largeur[40]= 4;
  Pinceau_predefini_Hauteur[40]= 4;
  Pinceau_Type             [40]=FORME_PINCEAU_ANTISLASH;

  Pinceau_predefini_Largeur[41]= 8;
  Pinceau_predefini_Hauteur[41]= 8;
  Pinceau_Type             [41]=FORME_PINCEAU_ANTISLASH;

  Pinceau_predefini_Largeur[42]= 4;
  Pinceau_predefini_Hauteur[42]= 4;
  Pinceau_Type             [42]=FORME_PINCEAU_ALEATOIRE;

  Pinceau_predefini_Largeur[43]= 8;
  Pinceau_predefini_Hauteur[43]= 8;
  Pinceau_Type             [43]=FORME_PINCEAU_ALEATOIRE;

  Pinceau_predefini_Largeur[44]=13;
  Pinceau_predefini_Hauteur[44]=13;
  Pinceau_Type             [44]=FORME_PINCEAU_ALEATOIRE;

  Pinceau_predefini_Largeur[45]= 3;
  Pinceau_predefini_Hauteur[45]= 3;
  Pinceau_Type             [45]=FORME_PINCEAU_DIVERS;

  Pinceau_predefini_Largeur[46]= 3;
  Pinceau_predefini_Hauteur[46]= 3;
  Pinceau_Type             [46]=FORME_PINCEAU_DIVERS;

  Pinceau_predefini_Largeur[47]= 7;
  Pinceau_predefini_Hauteur[47]= 7;
  Pinceau_Type             [47]=FORME_PINCEAU_DIVERS;

  for (Indice=0;Indice<NB_SPRITES_PINCEAU;Indice++)
  {
    Pinceau_predefini_Decalage_X[Indice]=(Pinceau_predefini_Largeur[Indice]>>1);
    Pinceau_predefini_Decalage_Y[Indice]=(Pinceau_predefini_Hauteur[Indice]>>1);
  }

  Curseur_Decalage_X[FORME_CURSEUR_FLECHE]=0;
  Curseur_Decalage_Y[FORME_CURSEUR_FLECHE]=0;

  Curseur_Decalage_X[FORME_CURSEUR_CIBLE]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_CIBLE]=7;

  Curseur_Decalage_X[FORME_CURSEUR_CIBLE_PIPETTE]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_CIBLE_PIPETTE]=7;

  Curseur_Decalage_X[FORME_CURSEUR_SABLIER]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_SABLIER]=7;

  Curseur_Decalage_X[FORME_CURSEUR_MULTIDIRECTIONNEL]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_MULTIDIRECTIONNEL]=7;

  Curseur_Decalage_X[FORME_CURSEUR_HORIZONTAL]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_HORIZONTAL]=3;

  Curseur_Decalage_X[FORME_CURSEUR_CIBLE_FINE]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_CIBLE_FINE]=7;

  Curseur_Decalage_X[FORME_CURSEUR_CIBLE_PIPETTE_FINE]=7;
  Curseur_Decalage_Y[FORME_CURSEUR_CIBLE_PIPETTE_FINE]=7;
}


// Initialisation des boutons:

  // Action factice:

void Rien_du_tout(void)
{}

  // Initialiseur d'un bouton:

void Initialiser_bouton(byte   Numero,
                        word   Decalage_X      , word   Decalage_Y,
                        word   Largeur         , word   Hauteur,
                        byte   Forme,
                        fonction_action Gauche , fonction_action Droite,
                        fonction_action Desenclencher,
                        byte   Famille)
{
  Bouton[Numero].Decalage_X      =Decalage_X;
  Bouton[Numero].Decalage_Y      =Decalage_Y;
  Bouton[Numero].Largeur         =Largeur-1;
  Bouton[Numero].Hauteur         =Hauteur-1;
  Bouton[Numero].Enfonce         =0;
  Bouton[Numero].Forme           =Forme;
  Bouton[Numero].Gauche          =Gauche;
  Bouton[Numero].Droite          =Droite;
  Bouton[Numero].Desenclencher   =Desenclencher;
  Bouton[Numero].Famille         =Famille;
}


  // Initiliseur de tous les boutons:

void Initialisation_des_boutons(void)
{
  byte Indice_bouton;

  for (Indice_bouton=0;Indice_bouton<NB_BOUTONS;Indice_bouton++)
  {
    Bouton[Indice_bouton].Raccourci_gauche=0xFFFF;
    Bouton[Indice_bouton].Raccourci_droite=0xFFFF;
    Initialiser_bouton(Indice_bouton,
                       0,0,
                       1,1,
                       FORME_BOUTON_RECTANGLE,
                       Rien_du_tout,Rien_du_tout,
                       Rien_du_tout,
                       0);
  }

  // Ici viennent les déclarations des boutons que l'on sait gérer

  Initialiser_bouton(BOUTON_PINCEAUX,
                     0,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Menu_pinceaux,Bouton_Brosse_monochrome,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

// !!! TEMPORAIRE !!!
  Initialiser_bouton(BOUTON_AJUSTER,
                     0,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Ajuster,Bouton_Ajuster,
                     Rien_du_tout,
                     FAMILLE_OUTIL);
/*
  Initialiser_bouton(BOUTON_AJUSTER,
                     0,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Ajuster,Rien_du_tout,
                     Rien_du_tout,
                     FAMILLE_OUTIL);
*/

  Initialiser_bouton(BOUTON_DESSIN,
                     17,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Dessin,Bouton_Dessin_Switch_mode,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_COURBES,
                     17,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Courbes,Bouton_Courbes_Switch_mode,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_LIGNES,
                     34,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Lignes,Bouton_Lignes_Switch_mode,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_SPRAY,
                     34,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Spray,Bouton_Spray_Menu,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_FLOODFILL,
                     51,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Fill,Bouton_Remplacer,
                     Bouton_desenclencher_Fill,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_POLYGONES,
                     51,18,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Polygone,Bouton_Polyform,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_POLYFILL,
                     52,19,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Polyfill,Bouton_Filled_polyform,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_RECTANGLES,
                     68,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Rectangle_vide,Bouton_Rectangle_vide,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_FILLRECT,
                     69,2,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Rectangle_plein,Bouton_Rectangle_plein,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_CERCLES,
                     68,18,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Cercle_vide,Bouton_Ellipse_vide,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_FILLCERC,
                     69,19,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Cercle_plein,Bouton_Ellipse_pleine,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

// !!! TEMPORAIRE !!!
  Initialiser_bouton(BOUTON_GRADRECT,
                     85,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Rectangle_degrade,Bouton_Rectangle_degrade,
                     Rien_du_tout,
                     FAMILLE_OUTIL);
/*
  Initialiser_bouton(BOUTON_GRADRECT,
                     85,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Rien_du_tout,Rien_du_tout,
                     Rien_du_tout,
                     FAMILLE_OUTIL);
*/

  Initialiser_bouton(BOUTON_GRADMENU,
                     86,2,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Degrades,Bouton_Degrades,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_SPHERES,
                     85,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Cercle_degrade,Bouton_Ellipse_degrade,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

  Initialiser_bouton(BOUTON_BROSSE,
                     106,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Brosse,Bouton_Restaurer_brosse,
                     Bouton_desenclencher_Brosse,
                     FAMILLE_INTERRUPTION);

  Initialiser_bouton(BOUTON_POLYBROSSE,
                     107,2,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Lasso,Bouton_Restaurer_brosse,
                     Bouton_desenclencher_Lasso,
                     FAMILLE_INTERRUPTION);

  Initialiser_bouton(BOUTON_EFFETS_BROSSE,
                     106,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Brush_FX,Bouton_Brush_FX,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_EFFETS,
                     123,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Effets,Bouton_Effets,
                     Rien_du_tout,
                     FAMILLE_EFFETS);

// !!! TEMPORAIRE !!!
  Initialiser_bouton(BOUTON_TEXTE,
                     123,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Texte,Message_Non_disponible,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);
/*
  Initialiser_bouton(BOUTON_TEXTE,
                     123,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Rien_du_tout,Rien_du_tout,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);
*/

  Initialiser_bouton(BOUTON_LOUPE,
                     140,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Loupe,Bouton_Menu_Loupe,
                     Bouton_desenclencher_Loupe,
                     FAMILLE_INTERRUPTION);

  Initialiser_bouton(BOUTON_PIPETTE,
                     140,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Pipette,Bouton_Inverser_foreback,
                     Bouton_desenclencher_Pipette,
                     FAMILLE_INTERRUPTION);

  Initialiser_bouton(BOUTON_RESOL,
                     161,1,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Resol,Bouton_Safety_resol,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_PAGE,
                     161,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Page,Bouton_Copy_page,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_SAUVER,
                     178,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Save,Bouton_Autosave,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_CHARGER,
                     179,2,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Load,Bouton_Reload,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_PARAMETRES,
                     178,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Settings,Bouton_Settings,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_CLEAR,
                     195,1,
                     17,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Clear,Bouton_Clear_colore,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_AIDE,
                     195,18,
                     17,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Aide,Bouton_Stats,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_UNDO,
                     213,1,
                     19,12,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Undo,Bouton_Redo,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_KILL,
                     213,14,
                     19,7,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Kill,Bouton_Kill,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_QUIT,
                     213,22,
                     19,12,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Quit,Bouton_Quit,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_PALETTE,
                     237,9,
                     16,8,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Palette,Bouton_Palette_secondaire,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_PAL_LEFT,
                     237,18,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Pal_left,Bouton_Pal_left_fast,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_PAL_RIGHT,
                     238,19,
                     15,15,
                     FORME_BOUTON_TRIANGLE_BAS_DROITE,
                     Bouton_Pal_right,Bouton_Pal_right_fast,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_CHOIX_COL,
                     LARGEUR_MENU+1,2,
                     1,32, // La largeur est mise à jour à chq chngmnt de mode
                     FORME_BOUTON_SANS_CADRE,
                     Bouton_Choix_forecolor,Bouton_Choix_backcolor,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

  Initialiser_bouton(BOUTON_CACHER,
                     0,35,
                     16,9,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Cacher_menu,Bouton_Cacher_menu,
                     Rien_du_tout,
                     FAMILLE_TOOLBAR);
}



// Initialisation des opérations:

  // Initialiseur d'une opération:

void Initialiser_operation(byte Numero_operation,
                           byte Numero_bouton_souris,
                           byte Taille_de_pile,
                           fonction_action Action,
                           byte Effacer_curseur)
{
  Operation[Numero_operation][Numero_bouton_souris]
           [Taille_de_pile].Action=Action;
  Operation[Numero_operation][Numero_bouton_souris]
           [Taille_de_pile].Effacer_curseur=Effacer_curseur;
}


  // Initiliseur de toutes les opérations:

void Initialisation_des_operations(void)
{
  byte Numero; // Numéro de l'option en cours d'auto-initialisation
  byte Bouton; // Bouton souris en cours d'auto-initialisation
  byte Taille; // Taille de la pile en cours d'auto-initialisation

  // Auto-initialisation des opérations (vers des actions inoffensives)

  for (Numero=0;Numero<NB_OPERATIONS;Numero++)
    for (Bouton=0;Bouton<3;Bouton++)
      for (Taille=0;Taille<TAILLE_PILE_OPERATIONS;Taille++)
        Initialiser_operation(Numero,Bouton,Taille,Print_coordonnees,0);


  // Ici viennent les déclarations détaillées des opérations
  Initialiser_operation(OPERATION_DESSIN_CONTINU,1,0,
                        Freehand_Mode1_1_0,1);
  Initialiser_operation(OPERATION_DESSIN_CONTINU,1,2,
                        Freehand_Mode1_1_2,0);
  Initialiser_operation(OPERATION_DESSIN_CONTINU,0,2,
                        Freehand_Mode12_0_2,0);
  Initialiser_operation(OPERATION_DESSIN_CONTINU,2,0,
                        Freehand_Mode1_2_0,1);
  Initialiser_operation(OPERATION_DESSIN_CONTINU,2,2,
                        Freehand_Mode1_2_2,0);

  Initialiser_operation(OPERATION_DESSIN_DISCONTINU,1,0,
                        Freehand_Mode2_1_0,1);
  Initialiser_operation(OPERATION_DESSIN_DISCONTINU,1,2,
                        Freehand_Mode2_1_2,0);
  Initialiser_operation(OPERATION_DESSIN_DISCONTINU,0,2,
                        Freehand_Mode12_0_2,0);
  Initialiser_operation(OPERATION_DESSIN_DISCONTINU,2,0,
                        Freehand_Mode2_2_0,1);
  Initialiser_operation(OPERATION_DESSIN_DISCONTINU,2,2,
                        Freehand_Mode2_2_2,0);

  Initialiser_operation(OPERATION_DESSIN_POINT,1,0,
                        Freehand_Mode3_1_0,1);
  Initialiser_operation(OPERATION_DESSIN_POINT,2,0,
                        Freehand_Mode3_2_0,1);
  Initialiser_operation(OPERATION_DESSIN_POINT,0,1,
                        Freehand_Mode3_0_1,0);

  Initialiser_operation(OPERATION_LIGNE,1,0,
                        Ligne_12_0,1);
  Initialiser_operation(OPERATION_LIGNE,1,5,
                        Ligne_12_5,0);
  Initialiser_operation(OPERATION_LIGNE,0,5,
                        Ligne_0_5,1);
  Initialiser_operation(OPERATION_LIGNE,2,0,
                        Ligne_12_0,1);
  Initialiser_operation(OPERATION_LIGNE,2,5,
                        Ligne_12_5,0);

  Initialiser_operation(OPERATION_K_LIGNE,1,0,
                        K_Ligne_12_0,1);
  Initialiser_operation(OPERATION_K_LIGNE,1,6,
                        K_Ligne_12_6,0);
  Initialiser_operation(OPERATION_K_LIGNE,1,7,
                        K_Ligne_12_7,1);
  Initialiser_operation(OPERATION_K_LIGNE,2,0,
                        K_Ligne_12_0,1);
  Initialiser_operation(OPERATION_K_LIGNE,2,6,
                        K_Ligne_12_6,0);
  Initialiser_operation(OPERATION_K_LIGNE,2,7,
                        K_Ligne_12_7,1);
  Initialiser_operation(OPERATION_K_LIGNE,0,6,
                        K_Ligne_0_6,1);
  Initialiser_operation(OPERATION_K_LIGNE,0,7,
                        K_Ligne_12_6,0);

  Initialiser_operation(OPERATION_RECTANGLE_VIDE,1,0,
                        Rectangle_12_0,1);
  Initialiser_operation(OPERATION_RECTANGLE_VIDE,2,0,
                        Rectangle_12_0,1);
  Initialiser_operation(OPERATION_RECTANGLE_VIDE,1,5,
                        Rectangle_12_5,0);
  Initialiser_operation(OPERATION_RECTANGLE_VIDE,2,5,
                        Rectangle_12_5,0);
  Initialiser_operation(OPERATION_RECTANGLE_VIDE,0,5,
                        Rectangle_vide_0_5,1);

  Initialiser_operation(OPERATION_RECTANGLE_PLEIN,1,0,
                        Rectangle_12_0,1);
  Initialiser_operation(OPERATION_RECTANGLE_PLEIN,2,0,
                        Rectangle_12_0,1);
  Initialiser_operation(OPERATION_RECTANGLE_PLEIN,1,5,
                        Rectangle_12_5,0);
  Initialiser_operation(OPERATION_RECTANGLE_PLEIN,2,5,
                        Rectangle_12_5,0);
  Initialiser_operation(OPERATION_RECTANGLE_PLEIN,0,5,
                        Rectangle_plein_0_5,1);

  Initialiser_operation(OPERATION_CERCLE_VIDE,1,0,
                        Cercle_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_VIDE,2,0,
                        Cercle_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_VIDE,1,5,
                        Cercle_12_5,0);
  Initialiser_operation(OPERATION_CERCLE_VIDE,2,5,
                        Cercle_12_5,0);
  Initialiser_operation(OPERATION_CERCLE_VIDE,0,5,
                        Cercle_vide_0_5,1);

  Initialiser_operation(OPERATION_CERCLE_PLEIN,1,0,
                        Cercle_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_PLEIN,2,0,
                        Cercle_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_PLEIN,1,5,
                        Cercle_12_5,0);
  Initialiser_operation(OPERATION_CERCLE_PLEIN,2,5,
                        Cercle_12_5,0);
  Initialiser_operation(OPERATION_CERCLE_PLEIN,0,5,
                        Cercle_plein_0_5,1);

  Initialiser_operation(OPERATION_ELLIPSE_VIDE,1,0,
                        Ellipse_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_VIDE,2,0,
                        Ellipse_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_VIDE,1,5,
                        Ellipse_12_5,0);
  Initialiser_operation(OPERATION_ELLIPSE_VIDE,2,5,
                        Ellipse_12_5,0);
  Initialiser_operation(OPERATION_ELLIPSE_VIDE,0,5,
                        Ellipse_vide_0_5,1);

  Initialiser_operation(OPERATION_ELLIPSE_PLEINE,1,0,
                        Ellipse_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_PLEINE,2,0,
                        Ellipse_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_PLEINE,1,5,
                        Ellipse_12_5,0);
  Initialiser_operation(OPERATION_ELLIPSE_PLEINE,2,5,
                        Ellipse_12_5,0);
  Initialiser_operation(OPERATION_ELLIPSE_PLEINE,0,5,
                        Ellipse_pleine_0_5,1);

  Initialiser_operation(OPERATION_FILL,1,0,
                        Fill_1_0,1);
  Initialiser_operation(OPERATION_FILL,2,0,
                        Fill_2_0,1);

  Initialiser_operation(OPERATION_REMPLACER,1,0,
                        Remplacer_1_0,1);
  Initialiser_operation(OPERATION_REMPLACER,2,0,
                        Remplacer_2_0,1);

  Initialiser_operation(OPERATION_PRISE_BROSSE,1,0,
                        Brosse_12_0,1);
  Initialiser_operation(OPERATION_PRISE_BROSSE,2,0,
                        Brosse_12_0,1);
  Initialiser_operation(OPERATION_PRISE_BROSSE,1,5,
                        Brosse_12_5,0);
  Initialiser_operation(OPERATION_PRISE_BROSSE,2,5,
                        Brosse_12_5,0);
  Initialiser_operation(OPERATION_PRISE_BROSSE,0,5,
                        Brosse_0_5,1);

  Initialiser_operation(OPERATION_ETIRER_BROSSE,1,0,
                        Etirer_brosse_12_0,1);
  Initialiser_operation(OPERATION_ETIRER_BROSSE,2,0,
                        Etirer_brosse_12_0,1);
  Initialiser_operation(OPERATION_ETIRER_BROSSE,1,7,
                        Etirer_brosse_1_7,0);
  Initialiser_operation(OPERATION_ETIRER_BROSSE,0,7,
                        Etirer_brosse_0_7,0);
  Initialiser_operation(OPERATION_ETIRER_BROSSE,2,7,
                        Etirer_brosse_2_7,1);

  Initialiser_operation(OPERATION_TOURNER_BROSSE,1,0,
                        Tourner_brosse_12_0,1);
  Initialiser_operation(OPERATION_TOURNER_BROSSE,2,0,
                        Tourner_brosse_12_0,1);
  Initialiser_operation(OPERATION_TOURNER_BROSSE,1,5,
                        Tourner_brosse_1_5,0);
  Initialiser_operation(OPERATION_TOURNER_BROSSE,0,5,
                        Tourner_brosse_0_5,0);
  Initialiser_operation(OPERATION_TOURNER_BROSSE,2,5,
                        Tourner_brosse_2_5,1);

  Initialiser_operation(OPERATION_POLYBROSSE,1,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_POLYBROSSE,2,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_POLYBROSSE,1,8,
                        Polybrosse_12_8,0);
  Initialiser_operation(OPERATION_POLYBROSSE,2,8,
                        Polybrosse_12_8,0);
  Initialiser_operation(OPERATION_POLYBROSSE,0,8,
                        Filled_polyform_0_8,0);

  Pipette_Couleur=-1;
  Initialiser_operation(OPERATION_PIPETTE,1,0,
                        Pipette_12_0,1);
  Initialiser_operation(OPERATION_PIPETTE,2,0,
                        Pipette_12_0,0);
  Initialiser_operation(OPERATION_PIPETTE,1,1,
                        Pipette_1_1,0);
  Initialiser_operation(OPERATION_PIPETTE,2,1,
                        Pipette_2_1,0);
  Initialiser_operation(OPERATION_PIPETTE,0,1,
                        Pipette_0_1,1);

  Initialiser_operation(OPERATION_LOUPE,1,0,
                        Loupe_12_0,1);
  Initialiser_operation(OPERATION_LOUPE,2,0,
                        Loupe_12_0,1);

  Initialiser_operation(OPERATION_COURBE_4_POINTS,1,0,
                        Courbe_34_points_1_0,1);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,2,0,
                        Courbe_34_points_2_0,1);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,1,5,
                        Courbe_34_points_1_5,0);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,2,5,
                        Courbe_34_points_2_5,0);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,0,5,
                        Courbe_4_points_0_5,1);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,1,9,
                        Courbe_4_points_1_9,0);
  Initialiser_operation(OPERATION_COURBE_4_POINTS,2,9,
                        Courbe_4_points_2_9,1);

  Initialiser_operation(OPERATION_COURBE_3_POINTS,1,0,
                        Courbe_34_points_1_0,1);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,2,0,
                        Courbe_34_points_2_0,1);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,1,5,
                        Courbe_34_points_1_5,0);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,2,5,
                        Courbe_34_points_2_5,0);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,0,5,
                        Courbe_3_points_0_5,1);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,0,11,
                        Courbe_3_points_0_11,0);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,1,11,
                        Courbe_3_points_12_11,1);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,2,11,
                        Courbe_3_points_12_11,1);

  Initialiser_operation(OPERATION_SPRAY,1,0,
                        Spray_1_0,0);
  Initialiser_operation(OPERATION_SPRAY,2,0,
                        Spray_2_0,0);
  Initialiser_operation(OPERATION_SPRAY,1,3,
                        Spray_12_3,0);
  Initialiser_operation(OPERATION_SPRAY,2,3,
                        Spray_12_3,0);
  Initialiser_operation(OPERATION_SPRAY,0,3,
                        Spray_0_3,0);

  Initialiser_operation(OPERATION_POLYGONE,1,0,
                        Polygone_12_0,1);
  Initialiser_operation(OPERATION_POLYGONE,2,0,
                        Polygone_12_0,1);
  Initialiser_operation(OPERATION_POLYGONE,1,8,
                        K_Ligne_12_6,0);
  Initialiser_operation(OPERATION_POLYGONE,2,8,
                        K_Ligne_12_6,0);
  Initialiser_operation(OPERATION_POLYGONE,1,9,
                        Polygone_12_9,1);
  Initialiser_operation(OPERATION_POLYGONE,2,9,
                        Polygone_12_9,1);
  Initialiser_operation(OPERATION_POLYGONE,0,8,
                        K_Ligne_0_6,1);
  Initialiser_operation(OPERATION_POLYGONE,0,9,
                        K_Ligne_12_6,0);

  Initialiser_operation(OPERATION_POLYFILL,1,0,
                        Polyfill_12_0,1);
  Initialiser_operation(OPERATION_POLYFILL,2,0,
                        Polyfill_12_0,1);
  Initialiser_operation(OPERATION_POLYFILL,1,8,
                        Polyfill_12_8,0);
  Initialiser_operation(OPERATION_POLYFILL,2,8,
                        Polyfill_12_8,0);
  Initialiser_operation(OPERATION_POLYFILL,1,9,
                        Polyfill_12_9,1);
  Initialiser_operation(OPERATION_POLYFILL,2,9,
                        Polyfill_12_9,1);
  Initialiser_operation(OPERATION_POLYFILL,0,8,
                        Polyfill_0_8,1);
  Initialiser_operation(OPERATION_POLYFILL,0,9,
                        Polyfill_12_8,0);

  Initialiser_operation(OPERATION_POLYFORM,1,0,
                        Polyform_12_0,1);
  Initialiser_operation(OPERATION_POLYFORM,2,0,
                        Polyform_12_0,1);
  Initialiser_operation(OPERATION_POLYFORM,1,8,
                        Polyform_12_8,0);
  Initialiser_operation(OPERATION_POLYFORM,2,8,
                        Polyform_12_8,0);
  Initialiser_operation(OPERATION_POLYFORM,0,8,
                        Polyform_0_8,0);

  Initialiser_operation(OPERATION_FILLED_POLYFORM,1,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_FILLED_POLYFORM,2,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_FILLED_POLYFORM,1,8,
                        Filled_polyform_12_8,0);
  Initialiser_operation(OPERATION_FILLED_POLYFORM,2,8,
                        Filled_polyform_12_8,0);
  Initialiser_operation(OPERATION_FILLED_POLYFORM,0,8,
                        Filled_polyform_0_8,0);

  Initialiser_operation(OPERATION_SCROLL,1,0,
                        Scroll_12_0,1);
  Initialiser_operation(OPERATION_SCROLL,2,0,
                        Scroll_12_0,1);
  Initialiser_operation(OPERATION_SCROLL,1,4,
                        Scroll_12_4,0);
  Initialiser_operation(OPERATION_SCROLL,2,4,
                        Scroll_12_4,0);
  Initialiser_operation(OPERATION_SCROLL,0,4,
                        Scroll_0_4,1);

  Initialiser_operation(OPERATION_CERCLE_DEGRADE,1,0,
                        Cercle_degrade_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,2,0,
                        Cercle_degrade_12_0,1);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,1,6,
                        Cercle_degrade_12_6,0);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,2,6,
                        Cercle_degrade_12_6,0);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,0,6,
                        Cercle_degrade_0_6,1);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,1,8,
                        Cercle_degrade_12_8,1);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,2,8,
                        Cercle_degrade_12_8,1);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,0,8,
                        Cercle_ou_ellipse_degrade_0_8,0);

  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,0,8,
                        Cercle_ou_ellipse_degrade_0_8,0);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,1,0,
                        Ellipse_degradee_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,2,0,
                        Ellipse_degradee_12_0,1);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,1,6,
                        Ellipse_degradee_12_6,0);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,2,6,
                        Ellipse_degradee_12_6,0);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,0,6,
                        Ellipse_degradee_0_6,1);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,1,8,
                        Ellipse_degradee_12_8,1);
  Initialiser_operation(OPERATION_ELLIPSE_DEGRADEE,2,8,
                        Ellipse_degradee_12_8,1);

  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,1,0,Rectangle_Degrade_12_0,0);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,1,5,Rectangle_Degrade_12_5,0);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,0,5,Rectangle_Degrade_0_5,1);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,0,7,Rectangle_Degrade_0_7,0);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,1,7,Rectangle_Degrade_12_7,1);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,1,9,Rectangle_Degrade_12_9,1);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,0,9,Rectangle_Degrade_0_9,1);


  Initialiser_operation(OPERATION_LIGNES_CENTREES,1,0,
                        Lignes_centrees_12_0,1);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,2,0,
                        Lignes_centrees_12_0,1);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,1,3,
                        Lignes_centrees_12_3,0);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,2,3,
                        Lignes_centrees_12_3,0);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,0,3,
                        Lignes_centrees_0_3,1);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,1,7,
                        Lignes_centrees_12_7,0);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,2,7,
                        Lignes_centrees_12_7,0);
  Initialiser_operation(OPERATION_LIGNES_CENTREES,0,7,
                        Lignes_centrees_0_7,0);
}



//-- Définition des modes vidéo: --------------------------------------------

  // Définition d'un mode:

void Definir_mode_video(short  Largeur,
                        short  Hauteur,
                        byte   Mode,
                        word   Fullscreen)
{                   
  byte Supporte = 0;
  
  if (Nb_modes_video >= MAX_MODES_VIDEO-1)
  {
    DEBUG("Erreur! Tentative de créer un mode de trop! Limite:", MAX_MODES_VIDEO);
    return;
  }
  if (!Fullscreen)
    Supporte = 128; // Prefere, non modifiable
  else if (SDL_VideoModeOK(Largeur, Hauteur, 8, SDL_FULLSCREEN))
    Supporte = 1; // Supporte
  else
  {
    // Non supporte : on ne le prend pas
    return;
  }

  Mode_video[Nb_modes_video].Largeur          = Largeur;
  Mode_video[Nb_modes_video].Hauteur          = Hauteur;
  Mode_video[Nb_modes_video].Mode             = Mode;
  Mode_video[Nb_modes_video].Fullscreen	      = Fullscreen;
  Mode_video[Nb_modes_video].Etat	            = Supporte;
  Nb_modes_video ++;
}

// Utilisé pour trier les modes retournés par SDL
int Compare_modes_video(const void *p1, const void *p2)
{
  const struct S_Mode_video *Mode1 = (const struct S_Mode_video *)p1;
  const struct S_Mode_video *Mode2 = (const struct S_Mode_video *)p2;

  // Tris par largeur
  if(Mode1->Largeur - Mode2->Largeur)
    return Mode1->Largeur - Mode2->Largeur;

  // Tri par hauteur
  return Mode1->Hauteur - Mode2->Hauteur;
}


// Initiliseur de tous les modes video:
void Definition_des_modes_video(void)
{                   // Numero       LargHaut Mode      FXFY Ratio Ref WinOnly Pointeur
  SDL_Rect** Modes;
  Nb_modes_video=0;
  // Doit être en premier pour avoir le numéro 0:
  Definir_mode_video( 640,480,MODE_SDL, 0);
 
  Definir_mode_video( 320,200,MODE_SDL, 1);
  Definir_mode_video( 320,224,MODE_SDL, 1);
  Definir_mode_video( 320,240,MODE_SDL, 1);
  Definir_mode_video( 320,256,MODE_SDL, 1);
  Definir_mode_video( 320,270,MODE_SDL, 1);
  Definir_mode_video( 320,282,MODE_SDL, 1);
  Definir_mode_video( 320,300,MODE_SDL, 1);
  Definir_mode_video( 320,360,MODE_SDL, 1);
  Definir_mode_video( 320,400,MODE_SDL, 1);
  Definir_mode_video( 320,448,MODE_SDL, 1);
  Definir_mode_video( 320,480,MODE_SDL, 1);
  Definir_mode_video( 320,512,MODE_SDL, 1);
  Definir_mode_video( 320,540,MODE_SDL, 1);
  Definir_mode_video( 320,564,MODE_SDL, 1);
  Definir_mode_video( 320,600,MODE_SDL, 1);
  Definir_mode_video( 360,200,MODE_SDL, 1);
  Definir_mode_video( 360,224,MODE_SDL, 1);
  Definir_mode_video( 360,240,MODE_SDL, 1);
  Definir_mode_video( 360,256,MODE_SDL, 1);
  Definir_mode_video( 360,270,MODE_SDL, 1);
  Definir_mode_video( 360,282,MODE_SDL, 1);
  Definir_mode_video( 360,300,MODE_SDL, 1);
  Definir_mode_video( 360,360,MODE_SDL, 1);
  Definir_mode_video( 360,400,MODE_SDL, 1);
  Definir_mode_video( 360,448,MODE_SDL, 1);
  Definir_mode_video( 360,480,MODE_SDL, 1);
  Definir_mode_video( 360,512,MODE_SDL, 1);
  Definir_mode_video( 360,540,MODE_SDL, 1);
  Definir_mode_video( 360,564,MODE_SDL, 1);
  Definir_mode_video( 360,600,MODE_SDL, 1);
  Definir_mode_video( 400,200,MODE_SDL, 1);
  Definir_mode_video( 400,224,MODE_SDL, 1);
  Definir_mode_video( 400,240,MODE_SDL, 1);
  Definir_mode_video( 400,256,MODE_SDL, 1);
  Definir_mode_video( 400,270,MODE_SDL, 1);
  Definir_mode_video( 400,282,MODE_SDL, 1);
  Definir_mode_video( 400,300,MODE_SDL, 1);
  Definir_mode_video( 400,360,MODE_SDL, 1);
  Definir_mode_video( 400,400,MODE_SDL, 1);
  Definir_mode_video( 400,448,MODE_SDL, 1);
  Definir_mode_video( 400,480,MODE_SDL, 1);
  Definir_mode_video( 400,512,MODE_SDL, 1);
  Definir_mode_video( 400,540,MODE_SDL, 1);
  Definir_mode_video( 400,564,MODE_SDL, 1);
  Definir_mode_video( 400,600,MODE_SDL, 1);
  Definir_mode_video( 640,224,MODE_SDL, 1);
  Definir_mode_video( 640,240,MODE_SDL, 1);
  Definir_mode_video( 640,256,MODE_SDL, 1);
  Definir_mode_video( 640,270,MODE_SDL, 1);
  Definir_mode_video( 640,300,MODE_SDL, 1);
  Definir_mode_video( 640,350,MODE_SDL, 1);
  Definir_mode_video( 640,400,MODE_SDL, 1);
  Definir_mode_video( 640,448,MODE_SDL, 1);
  Definir_mode_video( 640,480,MODE_SDL, 1);
  Definir_mode_video( 640,512,MODE_SDL, 1);
  Definir_mode_video( 640,540,MODE_SDL, 1);
  Definir_mode_video( 640,564,MODE_SDL, 1);
  Definir_mode_video( 640,600,MODE_SDL, 1);
  Definir_mode_video( 800,600,MODE_SDL, 1);
  Definir_mode_video(1024,768,MODE_SDL, 1);

  Modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
  if ((Modes != (SDL_Rect**)0) && (Modes!=(SDL_Rect**)-1))
  {
    int Indice;
    for (Indice=0; Modes[Indice]; Indice++)
    {
      int Indice2;
      for (Indice2=1; Indice2 < Nb_modes_video; Indice2++)
        if (Modes[Indice]->w == Mode_video[Indice2].Largeur && 
            Modes[Indice]->h == Mode_video[Indice2].Hauteur)
        {
          // Mode déja prévu: ok
          break;
        }
      if (Indice2 >= Nb_modes_video)
      {
        // Nouveau mode à ajouter à la liste
        Definir_mode_video(Modes[Indice]->w,Modes[Indice]->h,MODE_SDL, 1);
      }
    }
    // Tri des modes : ceux trouvés par SDL ont été listés à la fin.
    qsort(&Mode_video[1], Nb_modes_video - 1, sizeof(struct S_Mode_video), Compare_modes_video);
  }
}

//---------------------------------------------------------------------------




word Ordonnancement[NB_TOUCHES]=
{
  SPECIAL_SCROLL_UP,                // Scroll up
  SPECIAL_SCROLL_DOWN,              // Scroll down
  SPECIAL_SCROLL_LEFT,              // Scroll left
  SPECIAL_SCROLL_RIGHT,             // Scroll right
  SPECIAL_SCROLL_UP_FAST,           // Scroll up faster
  SPECIAL_SCROLL_DOWN_FAST,         // Scroll down faster
  SPECIAL_SCROLL_LEFT_FAST,         // Scroll left faster
  SPECIAL_SCROLL_RIGHT_FAST,        // Scroll right faster
  SPECIAL_SCROLL_UP_SLOW,           // Scroll up slower
  SPECIAL_SCROLL_DOWN_SLOW,         // Scroll down slower
  SPECIAL_SCROLL_LEFT_SLOW,         // Scroll left slower
  SPECIAL_SCROLL_RIGHT_SLOW,        // Scroll right slower
  SPECIAL_MOUSE_UP,                 // Emulate mouse up
  SPECIAL_MOUSE_DOWN,               // Emulate mouse down
  SPECIAL_MOUSE_LEFT,               // Emulate mouse left
  SPECIAL_MOUSE_RIGHT,              // Emulate mouse right
  SPECIAL_CLICK_LEFT,               // Emulate mouse click left
  SPECIAL_CLICK_RIGHT,              // Emulate mouse click right
  0x100+BOUTON_CACHER,              // Show / Hide menu
  SPECIAL_SHOW_HIDE_CURSOR,         // Show / Hide cursor
  SPECIAL_PINCEAU_POINT,            // Paintbrush = "."
  0x100+BOUTON_PINCEAUX,            // Paintbrush choice
  0x200+BOUTON_PINCEAUX,            // Monochrome brush
  0x100+BOUTON_DESSIN,              // Freehand drawing
  0x200+BOUTON_DESSIN,              // Switch freehand drawing mode
  SPECIAL_DESSIN_CONTINU,           // Continuous freehand drawing
  0x100+BOUTON_LIGNES,              // Line
  0x200+BOUTON_LIGNES,              // Knotted lines
  0x100+BOUTON_SPRAY,               // Spray
  0x200+BOUTON_SPRAY,               // Spray menu
  0x100+BOUTON_FLOODFILL,           // Floodfill
  0x200+BOUTON_FLOODFILL,           // Replace color
  0x100+BOUTON_COURBES,             // Bézier's curves
  0x200+BOUTON_COURBES,             // Bézier's curve with 3 or 4 points
  0x100+BOUTON_RECTANGLES,          // Empty rectangle
  0x100+BOUTON_FILLRECT,            // Filled rectangle
  0x100+BOUTON_CERCLES,             // Empty circle
  0x200+BOUTON_CERCLES,             // Empty ellipse
  0x100+BOUTON_FILLCERC,            // Filled circle
  0x200+BOUTON_FILLCERC,            // Filled ellipse
  0x100+BOUTON_POLYGONES,           // Empty polygon
  0x200+BOUTON_POLYGONES,           // Empty polyform
  0x100+BOUTON_POLYFILL,            // Polyfill
  0x200+BOUTON_POLYFILL,            // Filled polyform
  0x100+BOUTON_GRADRECT,            // Gradient rectangle
  0x100+BOUTON_GRADMENU,            // Gradation menu
  0x100+BOUTON_SPHERES,             // Spheres
  0x200+BOUTON_SPHERES,             // Gradient ellipses
  0x100+BOUTON_AJUSTER,             // Adjust picture
  0x200+BOUTON_AJUSTER,             // Flip picture menu
  0x100+BOUTON_EFFETS,              // Menu des effets
  SPECIAL_SHADE_MODE,               // Shade mode
  SPECIAL_SHADE_MENU,               // Shade menu
  SPECIAL_QUICK_SHADE_MODE,         // Quick-shade mode
  SPECIAL_QUICK_SHADE_MENU,         // Quick-shade menu
  SPECIAL_STENCIL_MODE,             // Stencil mode
  SPECIAL_STENCIL_MENU,             // Stencil menu
  SPECIAL_MASK_MODE,                // Mask mode
  SPECIAL_MASK_MENU,                // Mask menu
  SPECIAL_GRID_MODE,                // Grid mode
  SPECIAL_GRID_MENU,                // Grid menu
  SPECIAL_SIEVE_MODE,               // Sieve mode
  SPECIAL_SIEVE_MENU,               // Sieve menu
  SPECIAL_INVERT_SIEVE,             // Inverser la trame du mode Sieve
  SPECIAL_COLORIZE_MODE,            // Colorize mode
  SPECIAL_COLORIZE_MENU,            // Colorize menu
  SPECIAL_SMOOTH_MODE,              // Smooth mode
  SPECIAL_SMOOTH_MENU,              // Smooth menu
  SPECIAL_SMEAR_MODE,               // Smear mode
  SPECIAL_TILING_MODE,              // Tiling mode
  SPECIAL_TILING_MENU,              // Tiling menu
  0x100+BOUTON_BROSSE,              // Pick brush
  0x100+BOUTON_POLYBROSSE,          // Pick polyform brush
  0x200+BOUTON_BROSSE,              // Restore brush
  SPECIAL_FLIP_X,                   // Flip X
  SPECIAL_FLIP_Y,                   // Flip Y
  SPECIAL_ROTATE_90,                // 90° brush rotation
  SPECIAL_ROTATE_180,               // 180° brush rotation
  SPECIAL_STRETCH,                  // Stretch brush
  SPECIAL_DISTORT,                  // Distort brush
  SPECIAL_OUTLINE,                  // Outline brush
  SPECIAL_NIBBLE,                   // Nibble brush
  SPECIAL_GET_BRUSH_COLORS,         // Get colors from brush
  SPECIAL_RECOLORIZE_BRUSH,         // Recolorize brush
  SPECIAL_ROTATE_ANY_ANGLE,         // Rotate brush by any angle
  0x100+BOUTON_PIPETTE,             // Pipette
  0x200+BOUTON_PIPETTE,             // Swap fore/back color
  0x100+BOUTON_LOUPE,               // Magnifier mode
  0x200+BOUTON_LOUPE,               // Zoom factor menu
  SPECIAL_ZOOM_IN,                  // Zoom in
  SPECIAL_ZOOM_OUT,                 // Zoom out
  0x100+BOUTON_EFFETS_BROSSE,       // Brush effects menu
  0x100+BOUTON_TEXTE,               // Text
  0x100+BOUTON_RESOL,               // Resolution menu
  0x200+BOUTON_RESOL,               // Safety resolution
  0x100+BOUTON_AIDE,                // Help & credits
  0x200+BOUTON_AIDE,                // Statistics
  0x100+BOUTON_PAGE,                // Go to spare page
  0x200+BOUTON_PAGE,                // Copy to spare page
  0x100+BOUTON_SAUVER,              // Save as
  0x200+BOUTON_SAUVER,              // Save
  0x100+BOUTON_CHARGER,             // Load
  0x200+BOUTON_CHARGER,             // Re-load
  SPECIAL_SAVE_BRUSH,               // Save brush
  SPECIAL_LOAD_BRUSH,               // Load brush
  0x100+BOUTON_PARAMETRES,          // Settings
  0x100+BOUTON_UNDO,                // Undo
  0x200+BOUTON_UNDO,                // Redo
  0x100+BOUTON_KILL,                // Kill
  0x100+BOUTON_CLEAR,               // Clear
  0x200+BOUTON_CLEAR,               // Clear with backcolor
  0x100+BOUTON_QUIT,                // Quit
  0x100+BOUTON_PALETTE,             // Palette menu
  0x200+BOUTON_PALETTE,             // Palette menu secondaire
  SPECIAL_EXCLUDE_COLORS_MENU,      // Exclude colors menu
  0x100+BOUTON_PAL_LEFT,            // Scroll palette left
  0x100+BOUTON_PAL_RIGHT,           // Scroll palette right
  0x200+BOUTON_PAL_LEFT,            // Scroll palette left faster
  0x200+BOUTON_PAL_RIGHT,           // Scroll palette right faster
  SPECIAL_CENTER_ATTACHMENT,        // Center brush attachement
  SPECIAL_TOP_LEFT_ATTACHMENT,      // Top-left brush attachement
  SPECIAL_TOP_RIGHT_ATTACHMENT,     // Top-right brush attachement
  SPECIAL_BOTTOM_LEFT_ATTACHMENT,   // Bottom-left brush attachement
  SPECIAL_BOTTOM_RIGHT_ATTACHMENT,  // Bottom right brush attachement
  SPECIAL_NEXT_FORECOLOR,           // Next foreground color
  SPECIAL_PREVIOUS_FORECOLOR,       // Previous foreground color
  SPECIAL_NEXT_BACKCOLOR,           // Next background color
  SPECIAL_PREVIOUS_BACKCOLOR,       // Previous background color
  SPECIAL_NEXT_USER_FORECOLOR,      // Next user-defined foreground color
  SPECIAL_PREVIOUS_USER_FORECOLOR,  // Previous user-defined foreground color
  SPECIAL_NEXT_USER_BACKCOLOR,      // Next user-defined background color
  SPECIAL_PREVIOUS_USER_BACKCOLOR,  // Previous user-defined background color
  SPECIAL_RETRECIR_PINCEAU,         // Rétrécir le pinceau
  SPECIAL_GROSSIR_PINCEAU           // Grossir le pinceau
};

int Charger_CFG(int Tout_charger)
{
  FILE*  Handle;
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  long Taille_fichier;
  int  Indice,Indice2;
  struct Config_Header       CFG_Header;
  struct Config_Chunk        Chunk;
  struct Config_Infos_touche CFG_Infos_touche;
  struct Config_Mode_video   CFG_Mode_video;
  struct stat Informations_Fichier;
  int Conversion_touches = 0;

  strcpy(Nom_du_fichier,Repertoire_de_configuration);
  strcat(Nom_du_fichier,"gfx2.cfg");

  stat(Nom_du_fichier,&Informations_Fichier);
  Taille_fichier=Informations_Fichier.st_size;

  if ((Handle=fopen(Nom_du_fichier,"rb"))==NULL)
    return ERREUR_CFG_ABSENT;

  if ( (Taille_fichier<(long)sizeof(CFG_Header))
    || (!read_bytes(Handle, &CFG_Header.Signature, 3))
    || memcmp(CFG_Header.Signature,"CFG",3)
    || (!read_byte(Handle, &CFG_Header.Version1))
    || (!read_byte(Handle, &CFG_Header.Version2))
    || (!read_byte(Handle, &CFG_Header.Beta1))
    || (!read_byte(Handle, &CFG_Header.Beta2)) )
      goto Erreur_lecture_config;

  // Version DOS de Robinson et X-Man
  if ( (CFG_Header.Version1== 2)
    && (CFG_Header.Version2== 0)
    && (CFG_Header.Beta1== 96))
  {
    // Les touches (scancodes) sont à convertir)
    Conversion_touches = 1;
  }
  // Version SDL
  else if ( (CFG_Header.Version1!=VERSION1)
    || (CFG_Header.Version2!=VERSION2)
    || (CFG_Header.Beta1!=BETA1)
    || (CFG_Header.Beta2!=BETA2) )
    goto Erreur_config_ancienne;

  // - Lecture des infos contenues dans le fichier de config -
  while (read_byte(Handle, &Chunk.Numero))
  {
    read_word_le(Handle, &Chunk.Taille);
    switch (Chunk.Numero)
    {
      case CHUNK_TOUCHES: // Touches
        if (Tout_charger)
        {
          for (Indice=0; Indice<(long)(Chunk.Taille/sizeof(CFG_Infos_touche)); Indice++)
          {
            if (!read_word_le(Handle, &CFG_Infos_touche.Numero) ||
                !read_word_le(Handle, &CFG_Infos_touche.Touche) ||
                !read_word_le(Handle, &CFG_Infos_touche.Touche2) )
              goto Erreur_lecture_config;
            else
            {
              if (Conversion_touches)
              {
                CFG_Infos_touche.Touche = Touche_pour_scancode(CFG_Infos_touche.Touche);
              }
              for (Indice2=0;
                 ((Indice2<NB_TOUCHES) && (ConfigTouche[Indice2].Numero!=CFG_Infos_touche.Numero));
                 Indice2++);
              if (Indice2<NB_TOUCHES)
              {
                switch(Ordonnancement[Indice2]>>8)
                {
                  case 0 :
                    Config_Touche[Ordonnancement[Indice2]&0xFF]=CFG_Infos_touche.Touche;
                    break;
                  case 1 :
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_gauche = CFG_Infos_touche.Touche;
                    break;
                  case 2 :
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_droite = CFG_Infos_touche.Touche;
                    break;
                }
              }
              else
                goto Erreur_lecture_config;
            }
          }
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_MODES_VIDEO: // Modes vidéo
        for (Indice=0; Indice<(long)(Chunk.Taille/sizeof(CFG_Mode_video)); Indice++)
        {
          if (!read_byte(Handle, &CFG_Mode_video.Etat) ||
              !read_word_le(Handle, &CFG_Mode_video.Largeur) ||
              !read_word_le(Handle, &CFG_Mode_video.Hauteur) )
            goto Erreur_lecture_config;

          for (Indice2=1; Indice2<Nb_modes_video; Indice2++)
          {
            if (Mode_video[Indice2].Largeur==CFG_Mode_video.Largeur &&
                Mode_video[Indice2].Hauteur==CFG_Mode_video.Hauteur)
            {
              // On ne prend le paramètre utilisateur que si la résolution
              // est effectivement supportée par SDL
              // Seules les deux petits bits sont récupérés, car les anciens fichiers
              // de configuration (DOS 96.5%) utilisaient d'autres bits.
              if (! (Mode_video[Indice2].Etat & 128))
                Mode_video[Indice2].Etat=CFG_Mode_video.Etat&3;
              break;
            }
          }
        }
        break;
      case CHUNK_SHADE: // Shade
        if (Tout_charger)
        {
          if (! read_byte(Handle, &Shade_Actuel) )
            goto Erreur_lecture_config;

          for (Indice=0; Indice<8; Indice++)
          {
            for (Indice2=0; Indice2<512; Indice2++)
            {
              if (! read_word_le(Handle, &Shade_Liste[Indice].Liste[Indice2]))
                goto Erreur_lecture_config;
            }
            if (! read_byte(Handle, &Shade_Liste[Indice].Pas) ||
              ! read_byte(Handle, &Shade_Liste[Indice].Mode) )
            goto Erreur_lecture_config;
          }
          Liste2tables(Shade_Liste[Shade_Actuel].Liste,
            Shade_Liste[Shade_Actuel].Pas,
            Shade_Liste[Shade_Actuel].Mode,
            Shade_Table_gauche,Shade_Table_droite);        
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_MASQUE: // Masque
        if (Tout_charger)
        {
          if (!read_bytes(Handle, Mask_table, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_STENCIL: // Stencil
        if (Tout_charger)
        {
          if (!read_bytes(Handle, Stencil, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_DEGRADES: // Infos sur les dégradés
        if (Tout_charger)
        {
          if (! read_byte(Handle, &Degrade_Courant))
            goto Erreur_lecture_config;
          for(Indice=0;Indice<16;Indice++)
          {
            if (!read_byte(Handle, &Degrade_Tableau[Indice].Debut) ||
                !read_byte(Handle, &Degrade_Tableau[Indice].Fin) ||
                !read_dword_le(Handle, &Degrade_Tableau[Indice].Inverse) ||
                !read_dword_le(Handle, &Degrade_Tableau[Indice].Melange) ||
                !read_dword_le(Handle, &Degrade_Tableau[Indice].Technique) )
            goto Erreur_lecture_config;
          }
          Degrade_Charger_infos_du_tableau(Degrade_Courant);
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_SMOOTH: // Matrice du smooth
        if (Tout_charger)
        {
          for (Indice=0; Indice<3; Indice++)
            for (Indice2=0; Indice2<3; Indice2++)
              if (!read_byte(Handle, &(Smooth_Matrice[Indice][Indice2])))
                goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_EXCLUDE_COLORS: // Exclude_color
        if (Tout_charger)
        {
          if (!read_bytes(Handle, Exclude_color, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_QUICK_SHADE: // Quick-shade
        if (Tout_charger)
        {
          if (!read_byte(Handle, &Quick_shade_Step))
            goto Erreur_lecture_config;
          if (!read_byte(Handle, &Quick_shade_Loop))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
        case CHUNK_GRILLE: // Grille
        if (Tout_charger)
        {
          if (!read_word_le(Handle, &Snap_Largeur))
            goto Erreur_lecture_config;
          if (!read_word_le(Handle, &Snap_Hauteur))
            goto Erreur_lecture_config;
          if (!read_word_le(Handle, &Snap_Decalage_X))
            goto Erreur_lecture_config;
          if (!read_word_le(Handle, &Snap_Decalage_Y))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Taille,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      default: // Chunk inconnu
        goto Erreur_lecture_config;
    }
  }

  if (fclose(Handle))
    return ERREUR_CFG_CORROMPU;

  return 0;

Erreur_lecture_config:
  fclose(Handle);
  return ERREUR_CFG_CORROMPU;
Erreur_config_ancienne:
  fclose(Handle);
  return ERREUR_CFG_ANCIEN;
}


int Sauver_CFG(void)
{
  FILE*  Handle;
  int  Indice;
  int  Indice2;
  int Modes_a_sauver;
  //byte Octet;
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  struct Config_Header       CFG_Header;
  struct Config_Chunk        Chunk;
  struct Config_Infos_touche CFG_Infos_touche;
  struct Config_Mode_video   CFG_Mode_video;

  strcpy(Nom_du_fichier,Repertoire_de_configuration);
  strcat(Nom_du_fichier,"gfx2.cfg");

  if ((Handle=fopen(Nom_du_fichier,"wb"))==NULL)
    return ERREUR_SAUVEGARDE_CFG;

  // Ecriture du header
  memcpy(CFG_Header.Signature,"CFG",3);
  CFG_Header.Version1=VERSION1;
  CFG_Header.Version2=VERSION2;
  CFG_Header.Beta1   =BETA1;
  CFG_Header.Beta2   =BETA2;
  if (!write_bytes(Handle, &CFG_Header.Signature,3) ||
      !write_byte(Handle, CFG_Header.Version1) ||
      !write_byte(Handle, CFG_Header.Version2) ||
      !write_byte(Handle, CFG_Header.Beta1) ||
      !write_byte(Handle, CFG_Header.Beta2) )
    goto Erreur_sauvegarde_config;

  // Enregistrement des touches
  Chunk.Numero=CHUNK_TOUCHES;
  Chunk.Taille=NB_TOUCHES*sizeof(CFG_Infos_touche);

  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<NB_TOUCHES; Indice++)
  {
    CFG_Infos_touche.Numero = ConfigTouche[Indice].Numero;
    switch(Ordonnancement[Indice]>>8)
    {
      case 0 : CFG_Infos_touche.Touche=Config_Touche[Ordonnancement[Indice]&0xFF]; break;
      case 1 : CFG_Infos_touche.Touche=Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche; break;
      case 2 : CFG_Infos_touche.Touche=Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite; break;
    }
    CFG_Infos_touche.Touche2=0x00FF;
    if (!write_word_le(Handle, CFG_Infos_touche.Numero) ||
        !write_word_le(Handle, CFG_Infos_touche.Touche) ||
        !write_word_le(Handle, CFG_Infos_touche.Touche2) )
      goto Erreur_sauvegarde_config;
  }

  // D'abord compter les modes pour lesquels l'utilisateur a mis une préférence
  Modes_a_sauver=0;
  for (Indice=1; Indice<Nb_modes_video; Indice++)
    if (Mode_video[Indice].Etat==0 || Mode_video[Indice].Etat==2 || Mode_video[Indice].Etat==3)
      Modes_a_sauver++;

  // Sauvegarde de l'état de chaque mode vidéo
  Chunk.Numero=CHUNK_MODES_VIDEO;
  Chunk.Taille=Modes_a_sauver * sizeof(CFG_Mode_video);

  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=1; Indice<Nb_modes_video; Indice++)
    if (Mode_video[Indice].Etat==0 || Mode_video[Indice].Etat==2 || Mode_video[Indice].Etat==3)
    {
      CFG_Mode_video.Etat   =Mode_video[Indice].Etat;
      CFG_Mode_video.Largeur=Mode_video[Indice].Largeur;
      CFG_Mode_video.Hauteur=Mode_video[Indice].Hauteur;
      
      if (!write_byte(Handle, CFG_Mode_video.Etat) ||
        !write_word_le(Handle, CFG_Mode_video.Largeur) ||
        !write_word_le(Handle, CFG_Mode_video.Hauteur) )
        goto Erreur_sauvegarde_config;
    }

  // Ecriture des données du Shade (précédées du shade en cours)
  Chunk.Numero=CHUNK_SHADE;
  Chunk.Taille=sizeof(Shade_Liste)+sizeof(Shade_Actuel);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Shade_Actuel))
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<8; Indice++)
  {
    for (Indice2=0; Indice2<512; Indice2++)
    {
      if (! write_word_le(Handle, Shade_Liste[Indice].Liste[Indice2]))
        goto Erreur_sauvegarde_config;
    }
    if (! write_byte(Handle, Shade_Liste[Indice].Pas) ||
      ! write_byte(Handle, Shade_Liste[Indice].Mode) )
    goto Erreur_sauvegarde_config;
  }    

  // Sauvegarde des informations du Masque
  Chunk.Numero=CHUNK_MASQUE;
  Chunk.Taille=sizeof(Mask_table);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_bytes(Handle, Mask_table,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Stencil
  Chunk.Numero=CHUNK_STENCIL;
  Chunk.Taille=sizeof(Stencil);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_bytes(Handle, Stencil,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations des dégradés
  Chunk.Numero=CHUNK_DEGRADES;
  Chunk.Taille=sizeof(Degrade_Tableau)+1;
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Degrade_Courant))
    goto Erreur_sauvegarde_config;
  for(Indice=0;Indice<16;Indice++)
  {
    if (!write_byte(Handle,Degrade_Tableau[Indice].Debut) ||
        !write_byte(Handle,Degrade_Tableau[Indice].Fin) ||
        !write_dword_le(Handle, Degrade_Tableau[Indice].Inverse) ||
        !write_dword_le(Handle, Degrade_Tableau[Indice].Melange) ||
        !write_dword_le(Handle, Degrade_Tableau[Indice].Technique) )
        goto Erreur_sauvegarde_config;
  }

  // Sauvegarde de la matrice du Smooth
  Chunk.Numero=CHUNK_SMOOTH;
  Chunk.Taille=sizeof(Smooth_Matrice);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<3; Indice++)
    for (Indice2=0; Indice2<3; Indice2++)
      if (!write_byte(Handle, Smooth_Matrice[Indice][Indice2]))
        goto Erreur_sauvegarde_config;

  // Sauvegarde des couleurs à exclure
  Chunk.Numero=CHUNK_EXCLUDE_COLORS;
  Chunk.Taille=sizeof(Exclude_color);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
 if (!write_bytes(Handle, Exclude_color, 256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Quick-shade
  Chunk.Numero=CHUNK_QUICK_SHADE;
  Chunk.Taille=sizeof(Quick_shade_Step)+sizeof(Quick_shade_Loop);
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Quick_shade_Step))
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Quick_shade_Loop))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations de la grille
  Chunk.Numero=CHUNK_GRILLE;
  Chunk.Taille=8;
  if (!write_byte(Handle, Chunk.Numero) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_word_le(Handle, Snap_Largeur))
    goto Erreur_sauvegarde_config;
  if (!write_word_le(Handle, Snap_Hauteur))
    goto Erreur_sauvegarde_config;
  if (!write_word_le(Handle, Snap_Decalage_X))
    goto Erreur_sauvegarde_config;
  if (!write_word_le(Handle, Snap_Decalage_Y))
    goto Erreur_sauvegarde_config;

  
  if (fclose(Handle))
    return ERREUR_SAUVEGARDE_CFG;

  return 0;

Erreur_sauvegarde_config:
  fclose(Handle);
  return ERREUR_SAUVEGARDE_CFG;
}


void Initialiser_les_tables_de_multiplication(void)
{
  word Indice_de_facteur;
  word Facteur_de_zoom;
  word Indice_de_multiplication;

  for (Indice_de_facteur=0;Indice_de_facteur<NB_FACTEURS_DE_ZOOM;Indice_de_facteur++)
  {
    Facteur_de_zoom=FACTEUR_ZOOM[Indice_de_facteur];

    for (Indice_de_multiplication=0;Indice_de_multiplication<512;Indice_de_multiplication++)
    {
      TABLE_ZOOM[Indice_de_facteur][Indice_de_multiplication]=Facteur_de_zoom*Indice_de_multiplication;
    }
  }
}

// (Ré)assigne toutes les valeurs de configuration par défaut
void Config_par_defaut(void)
{
  int Indice, Indice2;

  // Raccourcis clavier
  for (Indice=0; Indice<NB_TOUCHES; Indice++)
  {
    switch(Ordonnancement[Indice]>>8)
    {
      case 0 :
        Config_Touche[Ordonnancement[Indice]&0xFF]=ConfigTouche[Indice].Touche;
        break;
      case 1 :
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche = ConfigTouche[Indice].Touche;
        break;
      case 2 :
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite = ConfigTouche[Indice].Touche;
        break;
    }
  }
  // Shade
  Shade_Actuel=0;
  for (Indice=0; Indice<8; Indice++)
  {
    Shade_Liste[Indice].Pas=1;
    Shade_Liste[Indice].Mode=0;
    for (Indice2=0; Indice2<512; Indice2++)
      Shade_Liste[Indice].Liste[Indice2]=256;
  }
  // Shade par défaut pour la palette standard
  for (Indice=0; Indice<7; Indice++)
    for (Indice2=0; Indice2<16; Indice2++)
      Shade_Liste[0].Liste[Indice*17+Indice2]=Indice*16+Indice2+16;
  
  Liste2tables(Shade_Liste[Shade_Actuel].Liste,
            Shade_Liste[Shade_Actuel].Pas,
            Shade_Liste[Shade_Actuel].Mode,
            Shade_Table_gauche,Shade_Table_droite);

  // Masque
  for (Indice=0; Indice<256; Indice++)
    Mask_table[Indice]=0;

  // Stencil
  for (Indice=0; Indice<256; Indice++)
    Stencil[Indice]=1;

  // Dégradés
  Degrade_Courant=0;
  for(Indice=0;Indice<16;Indice++)
  {
    Degrade_Tableau[Indice].Debut=0;
    Degrade_Tableau[Indice].Fin=0;
    Degrade_Tableau[Indice].Inverse=0;
    Degrade_Tableau[Indice].Melange=0;
    Degrade_Tableau[Indice].Technique=0;
  }
  Degrade_Charger_infos_du_tableau(Degrade_Courant);
  
  // Smooth
  Smooth_Matrice[0][0]=1;
  Smooth_Matrice[0][1]=2;
  Smooth_Matrice[0][2]=1;
  Smooth_Matrice[1][0]=2;
  Smooth_Matrice[1][1]=4;
  Smooth_Matrice[1][2]=2;
  Smooth_Matrice[2][0]=1;
  Smooth_Matrice[2][1]=2;
  Smooth_Matrice[2][2]=1;

  // Exclude colors
  for (Indice=0; Indice<256; Indice++)
    Exclude_color[Indice]=0;

  // Quick shade    
  Quick_shade_Step=1;
  Quick_shade_Loop=0;
  
  // Grille
  Snap_Largeur=Snap_Hauteur=8;
  Snap_Decalage_X=Snap_Decalage_Y=0;
  
}
