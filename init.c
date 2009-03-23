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

// Signal handler: I activate it for the two platforms who certainly
// support them. Feel free to check with others.
#if defined(__WIN32__) || defined(__linux__)
  #define GRAFX2_CATCHES_SIGNALS
#endif
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL_byteorder.h>
#include <SDL_image.h>
#if defined(__WIN32__)
  #include <windows.h> // GetLogicalDrives(), GetDriveType(), DRIVE_*
#endif
#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
  #include <proto/exec.h>
  #include <proto/dos.h>
#endif
#ifdef GRAFX2_CATCHES_SIGNALS
  #include <signal.h>
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
#include "windows.h"
#include "sdlscreen.h"
#include "mountlist.h" // read_file_system_list
#include "loadsave.h" // Image_emergency_backup

// Rechercher la liste et le type des lecteurs de la machine

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
void bstrtostr( BSTR in, STRPTR out, TEXT max );
#endif

// Fonctions de lecture dans la skin de l'interface graphique
void Chercher_bas(SDL_Surface *gui, int *Debut_X, int *Debut_Y, byte Couleur_neutre,char * Section)
{
  byte Couleur;
  int Y;
  Y=*Debut_Y;
  *Debut_X=0;
  do
  {
    Couleur=Sdl_Get_pixel_8(gui,*Debut_X,Y);
    if (Couleur!=Couleur_neutre)
    {
      *Debut_Y=Y;
      return;
    }
    Y++;
  } while (Y<gui->h);
  
  printf("Error in skin file: Was looking down from %d,%d for a '%s', and reached the end of the image\n",
    *Debut_X, *Debut_Y, Section);
  Erreur(ERREUR_GUI_CORROMPU);
}

void Chercher_droite(SDL_Surface *gui, int *Debut_X, int Debut_Y, byte Couleur_neutre, char * Section)
{
  byte Couleur;
  int X;
  X=*Debut_X;
  
  do
  {
    Couleur=Sdl_Get_pixel_8(gui,X,Debut_Y);
    if (Couleur!=Couleur_neutre)
    {
      *Debut_X=X;
      return;
    }
    X++;
  } while (X<gui->w);
  
  printf("Error in skin file: Was looking right from %d,%d for a '%s', and reached the edege of the image\n",
    *Debut_X, Debut_Y, Section);
  Erreur(ERREUR_GUI_CORROMPU);
}

void Lire_bloc(SDL_Surface *gui, int Debut_X, int Debut_Y, void *Dest, int width, int height, char * Section, int Type)
{
  // Type: 0 = normal GUI element, only 4 colors allowed
  // Type: 1 = mouse cursor, 4 colors allowed + transparent
  // Type: 2 = brush icon or sieve pattern (only CM_Blanc and CM_Trans)
  // Type: 3 = raw bitmap (splash screen)
  
  byte * Ptr=Dest;
  int X,Y;
  byte Couleur;

  // Verification taille
  if (Debut_Y+height>=gui->h || Debut_X+width>=gui->w)
  {
    printf("Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but it doesn't fit the image.\n",
      Debut_X, Debut_Y, height, width, Section);
    Erreur(ERREUR_GUI_CORROMPU);
  }

  for (Y=Debut_Y; Y<Debut_Y+height; Y++)
  {
    for (X=Debut_X; X<Debut_X+width; X++)
    {
      Couleur=Sdl_Get_pixel_8(gui,X,Y);
      if (Type==0 && (Couleur != CM_Noir && Couleur != CM_Fonce && Couleur != CM_Clair && Couleur != CM_Blanc))
      {
        printf("Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the GUI colors (which were detected as %d,%d,%d,%d.\n",
          Debut_X, Debut_Y, height, width, Section, X, Y, Couleur, CM_Noir, CM_Fonce, CM_Clair, CM_Blanc);
        Erreur(ERREUR_GUI_CORROMPU);
      }
      if (Type==1 && (Couleur != CM_Noir && Couleur != CM_Fonce && Couleur != CM_Clair && Couleur != CM_Blanc && Couleur != CM_Trans))
      {
        printf("Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the mouse colors (which were detected as %d,%d,%d,%d,%d.\n",
          Debut_X, Debut_Y, height, width, Section, X, Y, Couleur, CM_Noir, CM_Fonce, CM_Clair, CM_Blanc, CM_Trans);
        Erreur(ERREUR_GUI_CORROMPU);
      }
      if (Type==2)
      {
        if (Couleur != CM_Blanc && Couleur != CM_Trans)
        {
          printf("Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the brush colors (which were detected as %d on %d.\n",
            Debut_X, Debut_Y, height, width, Section, X, Y, Couleur, CM_Blanc, CM_Trans);
          Erreur(ERREUR_GUI_CORROMPU);
        }
        // Conversion en 0/1 pour les brosses monochromes internes
        Couleur = (Couleur != CM_Trans);
      }
      *Ptr=Couleur;
      Ptr++;
    }
  }
}

void Lire_trame(SDL_Surface *gui, int Debut_X, int Debut_Y, word *Dest, char * Section)
{
  byte Buffer[256];
  int X,Y;
  
  Lire_bloc(gui, Debut_X, Debut_Y, Buffer, 16, 16, Section, 2);

  for (Y=0; Y<16; Y++)
  {
    *Dest=0;
    for (X=0; X<16; X++)
    {
      *Dest=*Dest | Buffer[Y*16+X]<<X;
    }
    Dest++;
  }
}


void Charger_DAT(void)
{
  int  Indice;
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  SDL_Surface * gui;
  SDL_Palette * SDLPal;
  int i;
  int Curseur_X=0,Curseur_Y=0;
  byte Couleur;
  byte CM_Neutre; // Couleur neutre utilisée pour délimiter les éléments GUI
  int Car_1=0;  // Indices utilisés pour les 4 "fontes" qui composent les
  int Car_2=0;  // grands titres de l'aide. Chaque indice avance dans 
  int Car_3=0;  // l'une des fontes dans l'ordre :  1 2
  int Car_4=0;  //                                  3 4
  
  // Lecture du fichier "skin"
  strcpy(Nom_du_fichier,Repertoire_des_donnees);
  strcat(Nom_du_fichier,"gfx2gui.gif");
  gui=IMG_Load(Nom_du_fichier);
  if (!gui)
  {
    Erreur(ERREUR_GUI_ABSENT);
  }
  if (!gui->format || gui->format->BitsPerPixel != 8)
  {
    printf("Not a 8-bit image");
    Erreur(ERREUR_GUI_CORROMPU);
  }
  SDLPal=gui->format->palette;
  if (!SDLPal || SDLPal->ncolors!=256)
  {
    printf("Not a 256-color palette");
    Erreur(ERREUR_GUI_CORROMPU);
  }
  // Lecture de la palette par défaut
  for (i=0; i<256; i++)
  {
    Palette_defaut[i].R=SDLPal->colors[i].r;
    Palette_defaut[i].G=SDLPal->colors[i].g;
    Palette_defaut[i].B=SDLPal->colors[i].b;
  }
  
  // Carré "noir"
  CM_Noir = Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  do
  {
    if (++Curseur_X>=gui->w)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
    Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  } while(Couleur==CM_Noir);
  // Carré "foncé"
  CM_Fonce=Couleur;
  do
  {
    if (++Curseur_X>=gui->w)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
    Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  } while(Couleur==CM_Fonce);
  // Carré "clair"
  CM_Clair=Couleur;
  do
  {
    if (++Curseur_X>gui->w)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
    Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  } while(Couleur==CM_Clair);
  // Carré "blanc"
  CM_Blanc=Couleur;
  do
  {
    if (++Curseur_X>=gui->w)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
    Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  } while(Couleur==CM_Blanc);
  // Carré "transparent"
  CM_Trans=Couleur;
  do
  {
    if (++Curseur_X>=gui->w)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
    Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y);
  } while(Couleur==CM_Trans);
  // Reste : couleur neutre
  CM_Neutre=Couleur;

  
  Curseur_X=0;
  Curseur_Y=1;
  while ((Couleur=Sdl_Get_pixel_8(gui,Curseur_X,Curseur_Y))==CM_Noir)
  {
    Curseur_Y++;
    if (Curseur_Y>=gui->h)
    {
      printf("Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      Erreur(ERREUR_GUI_CORROMPU);
    }
  }
  
  // Menu
  Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "menu");
  Lire_bloc(gui, Curseur_X, Curseur_Y, BLOCK_MENU, LARGEUR_MENU, HAUTEUR_MENU,"menu",0);
  Curseur_Y+=HAUTEUR_MENU;

  // Effets
  for (i=0; i<NB_SPRITES_EFFETS; i++)
  {
    if (i==0)
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "effect sprite");
    else
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "effect sprite");
    Lire_bloc(gui, Curseur_X, Curseur_Y, SPRITE_EFFET[i], LARGEUR_SPRITE_MENU, HAUTEUR_SPRITE_MENU, "effect sprite",0);
    Curseur_X+=LARGEUR_SPRITE_MENU;
  }
  Curseur_Y+=HAUTEUR_SPRITE_MENU;
  
  // Curseurs souris
  for (i=0; i<NB_SPRITES_CURSEUR; i++)
  {
    if (i==0)
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "mouse cursor");
    else
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "mouse cursor");
    Lire_bloc(gui, Curseur_X, Curseur_Y, SPRITE_CURSEUR[i], LARGEUR_SPRITE_CURSEUR, HAUTEUR_SPRITE_CURSEUR, "mouse cursor",1);
    Curseur_X+=LARGEUR_SPRITE_CURSEUR;
  }
  Curseur_Y+=HAUTEUR_SPRITE_CURSEUR;
  
  // Sprites menu
  for (i=0; i<NB_SPRITES_MENU; i++)
  {
    if (i==0)
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "menu sprite");
    else
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "menu sprite");
    Lire_bloc(gui, Curseur_X, Curseur_Y, SPRITE_MENU[i], LARGEUR_SPRITE_MENU, HAUTEUR_SPRITE_MENU, "menu sprite",1);
    Curseur_X+=LARGEUR_SPRITE_MENU;
  }
  Curseur_Y+=HAUTEUR_SPRITE_MENU;
  
  // Icones des Pinceaux
  for (i=0; i<NB_SPRITES_PINCEAU; i++)
  {
    // Rangés par ligne de 12
    if ((i%12)==0)
    {
      if (i!=0)
        Curseur_Y+=HAUTEUR_PINCEAU;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "brush icon");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "brush icon");
    }
    Lire_bloc(gui, Curseur_X, Curseur_Y, SPRITE_PINCEAU[i], LARGEUR_PINCEAU, HAUTEUR_PINCEAU, "brush icon",2);
    Curseur_X+=LARGEUR_PINCEAU;
  }
  Curseur_Y+=HAUTEUR_PINCEAU;

  // Sprites drive
  for (i=0; i<NB_SPRITES_DRIVES; i++)
  {
    if (i==0)
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "sprite drive");
    else
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "sprite drive");
    Lire_bloc(gui, Curseur_X, Curseur_Y, SPRITE_DRIVE[i], LARGEUR_SPRITE_DRIVE, HAUTEUR_SPRITE_DRIVE, "sprite drive",1);
    Curseur_X+=LARGEUR_SPRITE_DRIVE;
  }
  Curseur_Y+=HAUTEUR_SPRITE_DRIVE;

  // Logo splash screen
  if (!(Logo_GrafX2=(byte *)malloc(231*56)))
    Erreur(ERREUR_MEMOIRE);

  Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "logo menu");
  Lire_bloc(gui, Curseur_X, Curseur_Y, Logo_GrafX2, 231, 56, "logo menu",3);
  Curseur_Y+=56;
  
  // Trames
  for (i=0; i<NB_TRAMES_PREDEFINIES; i++)
  {
    if (i==0)
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "sieve pattern");
    else
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "sieve pattern");
    Lire_trame(gui, Curseur_X, Curseur_Y, TRAME_PREDEFINIE[i],"sieve pattern");
    Curseur_X+=16;
  }
  Curseur_Y+=16;
  
  // Fonte Système
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        Curseur_Y+=8;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "system font");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "system font");
    }
    Lire_bloc(gui, Curseur_X, Curseur_Y, &Fonte_systeme[i*64], 8, 8, "system font",2);
    Curseur_X+=8;
  }
  Curseur_Y+=8;
  Fonte=Fonte_systeme;

  // Fonte Fun
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        Curseur_Y+=8;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "fun font");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "fun font");
    }
    Lire_bloc(gui, Curseur_X, Curseur_Y, &Fonte_fun[i*64], 8, 8, "fun font",2);
    Curseur_X+=8;
  }
  Curseur_Y+=8;

  // Fonte help normale
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        Curseur_Y+=8;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "help font (norm)");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "help font (norm)");
    }
    Lire_bloc(gui, Curseur_X, Curseur_Y, &(Fonte_help_norm[i][0][0]), 6, 8, "help font (norm)",0);
    Curseur_X+=6;
  }
  Curseur_Y+=8;
  
  // Fonte help bold
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        Curseur_Y+=8;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "help font (bold)");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "help font (bold)");
    }
    Lire_bloc(gui, Curseur_X, Curseur_Y, &(Fonte_help_bold[i][0][0]), 6, 8, "help font (bold)",0);
    Curseur_X+=6;
  }
  Curseur_Y+=8;

  // Fonte help titre
  for (i=0; i<256; i++)
  {
    byte * Dest;
    // Rangés par ligne de 64
    if ((i%64)==0)
    {
      if (i!=0)
        Curseur_Y+=8;
      Chercher_bas(gui, &Curseur_X, &Curseur_Y, CM_Neutre, "help font (title)");
    }
    else
    {
      Chercher_droite(gui, &Curseur_X, Curseur_Y, CM_Neutre, "help font (title)");
    }
    
    if (i&1)
      if (i&64)
        Dest=&(Fonte_help_t4[Car_4++][0][0]);
      else
        Dest=&(Fonte_help_t2[Car_2++][0][0]);
    else
      if (i&64)
        Dest=&(Fonte_help_t3[Car_3++][0][0]);
      else
        Dest=&(Fonte_help_t1[Car_1++][0][0]);
    
    Lire_bloc(gui, Curseur_X, Curseur_Y, Dest, 6, 8, "help font (title)",0);
    Curseur_X+=6;
  }
  Curseur_Y+=8;
  
  // Terminé: libération de l'image skin
  SDL_FreeSurface(gui);

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

void Initialiser_bouton(byte   btn_number,
                        word   x_offset      , word   y_offset,
                        word   width         , word   height,
                        byte   shape,
                        fonction_action Gauche , fonction_action Droite,
                        fonction_action Desenclencher,
                        byte   family)
{
  Bouton[btn_number].Decalage_X      =x_offset;
  Bouton[btn_number].Decalage_Y      =y_offset;
  Bouton[btn_number].Width         =width-1;
  Bouton[btn_number].Height         =height-1;
  Bouton[btn_number].Enfonce         =0;
  Bouton[btn_number].Shape           =shape;
  Bouton[btn_number].Gauche          =Gauche;
  Bouton[btn_number].Droite          =Droite;
  Bouton[btn_number].Desenclencher   =Desenclencher;
  Bouton[btn_number].Famille         =family;
}


  // Initiliseur de tous les boutons:

void Initialisation_des_boutons(void)
{
  byte Indice_bouton;

  for (Indice_bouton=0;Indice_bouton<NB_BOUTONS;Indice_bouton++)
  {
    Bouton[Indice_bouton].Raccourci_gauche[0]=0;
    Bouton[Indice_bouton].Raccourci_gauche[1]=0;
    Bouton[Indice_bouton].Raccourci_droite[0]=0;
    Bouton[Indice_bouton].Raccourci_droite[1]=0;
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
                     Bouton_Ajuster,Message_Non_disponible,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

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

  Initialiser_bouton(BOUTON_GRADRECT,
                     85,1,
                     15,15,
                     FORME_BOUTON_TRIANGLE_HAUT_GAUCHE,
                     Bouton_Rectangle_degrade,Bouton_Rectangle_degrade,
                     Rien_du_tout,
                     FAMILLE_OUTIL);

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

  Initialiser_bouton(BOUTON_TEXTE,
                     123,18,
                     16,16,
                     FORME_BOUTON_RECTANGLE,
                     Bouton_Texte,Bouton_Texte,
                     Rien_du_tout,
                     FAMILLE_INSTANTANE);

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
  byte number; // Numéro de l'option en cours d'auto-initialisation
  byte Bouton; // Bouton souris en cours d'auto-initialisation
  byte Taille; // Taille de la pile en cours d'auto-initialisation

  // Auto-initialisation des opérations (vers des actions inoffensives)

  for (number=0;number<NB_OPERATIONS;number++)
    for (Bouton=0;Bouton<3;Bouton++)
      for (Taille=0;Taille<TAILLE_PILE_OPERATIONS;Taille++)
        Initialiser_operation(number,Bouton,Taille,Print_coordonnees,0);


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
                        Fill_1_0,0);
  Initialiser_operation(OPERATION_FILL,2,0,
                        Fill_2_0,0);

  Initialiser_operation(OPERATION_REMPLACER,1,0,
                        Remplacer_1_0,0);
  Initialiser_operation(OPERATION_REMPLACER,2,0,
                        Remplacer_2_0,0);

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
                        Loupe_12_0,0);
  Initialiser_operation(OPERATION_LOUPE,2,0,
                        Loupe_12_0,0);

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
                        Courbe_4_points_2_9,0);

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
                        Courbe_3_points_12_11,0);
  Initialiser_operation(OPERATION_COURBE_3_POINTS,2,11,
                        Courbe_3_points_12_11,0);

  Initialiser_operation(OPERATION_SPRAY,1,0,
                        Spray_1_0,0);
  Initialiser_operation(OPERATION_SPRAY,2,0,
                        Spray_2_0,0);
  Initialiser_operation(OPERATION_SPRAY,1,2,
                        Spray_12_2,0);
  Initialiser_operation(OPERATION_SPRAY,2,2,
                        Spray_12_2,0);
  Initialiser_operation(OPERATION_SPRAY,0,2,
                        Spray_0_2,0);

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
                        Polyfill_12_9,0);
  Initialiser_operation(OPERATION_POLYFILL,2,9,
                        Polyfill_12_9,0);
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

  Initialiser_operation(OPERATION_FILLED_CONTOUR,1,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_FILLED_CONTOUR,2,0,
                        Filled_polyform_12_0,1);
  Initialiser_operation(OPERATION_FILLED_CONTOUR,1,8,
                        Filled_polyform_12_8,0);
  Initialiser_operation(OPERATION_FILLED_CONTOUR,2,8,
                        Filled_polyform_12_8,0);
  Initialiser_operation(OPERATION_FILLED_CONTOUR,0,8,
                        Filled_contour_0_8,0);

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
                        Cercle_degrade_12_8,0);
  Initialiser_operation(OPERATION_CERCLE_DEGRADE,2,8,
                        Cercle_degrade_12_8,0);
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
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,2,7,Rectangle_Degrade_12_7,1);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,1,9,Rectangle_Degrade_12_9,1);
  Initialiser_operation(OPERATION_RECTANGLE_DEGRADE,0,9,Rectangle_Degrade_0_9,0);


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

void Definir_mode_video(short  width,
                        short  height,
                        byte   mode,
                        word   fullscreen)
{
  byte Supporte = 0;

  if (Nb_modes_video >= MAX_MODES_VIDEO-1)
  {
    DEBUG("Error! Attempt to create too many videomodes. Maximum is:", MAX_MODES_VIDEO);
    return;
  }
  if (!fullscreen)
    Supporte = 128; // Prefere, non modifiable
  else if (SDL_VideoModeOK(width, height, 8, SDL_FULLSCREEN))
    Supporte = 1; // Supporte
  else
  {
    // Non supporte : on ne le prend pas
    return;
  }

  Mode_video[Nb_modes_video].Width          = width;
  Mode_video[Nb_modes_video].Height          = height;
  Mode_video[Nb_modes_video].Mode             = mode;
  Mode_video[Nb_modes_video].Fullscreen       = fullscreen;
  Mode_video[Nb_modes_video].Etat                   = Supporte;
  Nb_modes_video ++;
}

// Utilisé pour trier les modes retournés par SDL
int Compare_modes_video(const void *p1, const void *p2)
{
  const struct S_Mode_video *Mode1 = (const struct S_Mode_video *)p1;
  const struct S_Mode_video *Mode2 = (const struct S_Mode_video *)p2;

  // Tris par largeur
  if(Mode1->Width - Mode2->Width)
    return Mode1->Width - Mode2->Width;

  // Tri par hauteur
  return Mode1->Height - Mode2->Height;
}


// Initiliseur de tous les modes video:
void Definition_des_modes_video(void)
{                   // Numero       LargHaut Mode      FXFY Ratio Ref WinOnly Pointeur
  SDL_Rect** Modes;
  Nb_modes_video=0;
  // Doit être en premier pour avoir le numéro 0:
  Definir_mode_video( 640,480,0, 0);

  Definir_mode_video( 320,200,0, 1);
  Definir_mode_video( 320,224,0, 1);
  Definir_mode_video( 320,240,0, 1);
  Definir_mode_video( 320,256,0, 1);
  Definir_mode_video( 320,270,0, 1);
  Definir_mode_video( 320,282,0, 1);
  Definir_mode_video( 320,300,0, 1);
  Definir_mode_video( 320,360,0, 1);
  Definir_mode_video( 320,400,0, 1);
  Definir_mode_video( 320,448,0, 1);
  Definir_mode_video( 320,480,0, 1);
  Definir_mode_video( 320,512,0, 1);
  Definir_mode_video( 320,540,0, 1);
  Definir_mode_video( 320,564,0, 1);
  Definir_mode_video( 320,600,0, 1);
  Definir_mode_video( 360,200,0, 1);
  Definir_mode_video( 360,224,0, 1);
  Definir_mode_video( 360,240,0, 1);
  Definir_mode_video( 360,256,0, 1);
  Definir_mode_video( 360,270,0, 1);
  Definir_mode_video( 360,282,0, 1);
  Definir_mode_video( 360,300,0, 1);
  Definir_mode_video( 360,360,0, 1);
  Definir_mode_video( 360,400,0, 1);
  Definir_mode_video( 360,448,0, 1);
  Definir_mode_video( 360,480,0, 1);
  Definir_mode_video( 360,512,0, 1);
  Definir_mode_video( 360,540,0, 1);
  Definir_mode_video( 360,564,0, 1);
  Definir_mode_video( 360,600,0, 1);
  Definir_mode_video( 400,200,0, 1);
  Definir_mode_video( 400,224,0, 1);
  Definir_mode_video( 400,240,0, 1);
  Definir_mode_video( 400,256,0, 1);
  Definir_mode_video( 400,270,0, 1);
  Definir_mode_video( 400,282,0, 1);
  Definir_mode_video( 400,300,0, 1);
  Definir_mode_video( 400,360,0, 1);
  Definir_mode_video( 400,400,0, 1);
  Definir_mode_video( 400,448,0, 1);
  Definir_mode_video( 400,480,0, 1);
  Definir_mode_video( 400,512,0, 1);
  Definir_mode_video( 400,540,0, 1);
  Definir_mode_video( 400,564,0, 1);
  Definir_mode_video( 400,600,0, 1);
  Definir_mode_video( 640,224,0, 1);
  Definir_mode_video( 640,240,0, 1);
  Definir_mode_video( 640,256,0, 1);
  Definir_mode_video( 640,270,0, 1);
  Definir_mode_video( 640,300,0, 1);
  Definir_mode_video( 640,350,0, 1);
  Definir_mode_video( 640,400,0, 1);
  Definir_mode_video( 640,448,0, 1);
  Definir_mode_video( 640,480,0, 1);
  Definir_mode_video( 640,512,0, 1);
  Definir_mode_video( 640,540,0, 1);
  Definir_mode_video( 640,564,0, 1);
  Definir_mode_video( 640,600,0, 1);
  Definir_mode_video( 800,600,0, 1);
  Definir_mode_video(1024,768,0, 1);

  Modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
  if ((Modes != (SDL_Rect**)0) && (Modes!=(SDL_Rect**)-1))
  {
    int Indice;
    for (Indice=0; Modes[Indice]; Indice++)
    {
      int Indice2;
      for (Indice2=1; Indice2 < Nb_modes_video; Indice2++)
        if (Modes[Indice]->w == Mode_video[Indice2].Width &&
            Modes[Indice]->h == Mode_video[Indice2].Height)
        {
          // Mode déja prévu: ok
          break;
        }
      if (Indice2 >= Nb_modes_video && Modes[Indice]->w>=320 && Modes[Indice]->h>=200)
      {
        // Nouveau mode à ajouter à la liste
        Definir_mode_video(Modes[Indice]->w,Modes[Indice]->h,0, 1);
      }
    }
    // Tri des modes : ceux trouvés par SDL ont été listés à la fin.
    qsort(&Mode_video[1], Nb_modes_video - 1, sizeof(struct S_Mode_video), Compare_modes_video);
  }
}

//---------------------------------------------------------------------------

int Charger_CFG(int Tout_charger)
{
  FILE*  Handle;
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  long Taille_fichier;
  int  Indice,Indice2;
  Config_Header       CFG_Header;
  Config_Chunk        Chunk;
  Config_Infos_touche CFG_Infos_touche;
  Config_Mode_video   CFG_Mode_video;
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
  // Version SDL jusqu'a 98%
  else if ( (CFG_Header.Version1== 2)
    && (CFG_Header.Version2== 0)
    && (CFG_Header.Beta1== 97))
  {
    // Les touches 00FF (pas de touche) sont a comprendre comme 0x0000
    Conversion_touches = 2;
  }
  // Version SDL
  else if ( (CFG_Header.Version1!=VERSION1)
    || (CFG_Header.Version2!=VERSION2)
    || (CFG_Header.Beta1!=BETA1)
    || (CFG_Header.Beta2!=BETA2) )
    goto Erreur_config_ancienne;

  // - Lecture des infos contenues dans le fichier de config -
  while (read_byte(Handle, &Chunk.Number))
  {
    read_word_le(Handle, &Chunk.Taille);
    switch (Chunk.Number)
    {
      case CHUNK_TOUCHES: // Touches
        if (Tout_charger)
        {
          for (Indice=0; Indice<(long)(Chunk.Taille/sizeof(CFG_Infos_touche)); Indice++)
          {
            if (!read_word_le(Handle, &CFG_Infos_touche.Number) ||
                !read_word_le(Handle, &CFG_Infos_touche.Touche) ||
                !read_word_le(Handle, &CFG_Infos_touche.Touche2) )
              goto Erreur_lecture_config;
            else
            {
              if (Conversion_touches==1)
              {
                CFG_Infos_touche.Touche = Touche_pour_scancode(CFG_Infos_touche.Touche);
              }
              else if (Conversion_touches==2)
              {
                if (CFG_Infos_touche.Touche == 0x00FF)
                  CFG_Infos_touche.Touche = 0x0000;
                if (CFG_Infos_touche.Touche2 == 0x00FF)
                  CFG_Infos_touche.Touche2 = 0x0000;
              }
              
              for (Indice2=0;
                 ((Indice2<NB_TOUCHES) && (ConfigTouche[Indice2].Number!=CFG_Infos_touche.Number));
                 Indice2++);
              if (Indice2<NB_TOUCHES)
              {
                switch(Ordonnancement[Indice2]>>8)
                {
                  case 0 :
                    Config_Touche[Ordonnancement[Indice2]&0xFF][0]=CFG_Infos_touche.Touche;
                    Config_Touche[Ordonnancement[Indice2]&0xFF][1]=CFG_Infos_touche.Touche2;
                    break;
                  case 1 :
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_gauche[0] = CFG_Infos_touche.Touche;
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_gauche[1] = CFG_Infos_touche.Touche2;
                    break;
                  case 2 :
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_droite[0] = CFG_Infos_touche.Touche;
                    Bouton[Ordonnancement[Indice2]&0xFF].Raccourci_droite[1] = CFG_Infos_touche.Touche2;
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
              !read_word_le(Handle, &CFG_Mode_video.Width) ||
              !read_word_le(Handle, &CFG_Mode_video.Height) )
            goto Erreur_lecture_config;

          for (Indice2=1; Indice2<Nb_modes_video; Indice2++)
          {
            if (Mode_video[Indice2].Width==CFG_Mode_video.Width &&
                Mode_video[Indice2].Height==CFG_Mode_video.Height)
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
              if (! read_word_le(Handle, &Shade_Liste[Indice].List[Indice2]))
                goto Erreur_lecture_config;
            }
            if (! read_byte(Handle, &Shade_Liste[Indice].Step) ||
              ! read_byte(Handle, &Shade_Liste[Indice].Mode) )
            goto Erreur_lecture_config;
          }
          Liste2tables(Shade_Liste[Shade_Actuel].List,
            Shade_Liste[Shade_Actuel].Step,
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
  char Nom_du_fichier[TAILLE_CHEMIN_FICHIER];
  Config_Header CFG_Header;
  Config_Chunk Chunk;
  Config_Infos_touche CFG_Infos_touche;
  Config_Mode_video   CFG_Mode_video;

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
  Chunk.Number=CHUNK_TOUCHES;
  Chunk.Taille=NB_TOUCHES*sizeof(CFG_Infos_touche);

  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<NB_TOUCHES; Indice++)
  {
    CFG_Infos_touche.Number = ConfigTouche[Indice].Number;
    switch(Ordonnancement[Indice]>>8)
    {
      case 0 :
        CFG_Infos_touche.Touche =Config_Touche[Ordonnancement[Indice]&0xFF][0]; 
        CFG_Infos_touche.Touche2=Config_Touche[Ordonnancement[Indice]&0xFF][1]; 
        break;
      case 1 :
        CFG_Infos_touche.Touche =Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche[0]; 
        CFG_Infos_touche.Touche2=Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche[1]; 
        break;
      case 2 : 
        CFG_Infos_touche.Touche =Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite[0]; 
        CFG_Infos_touche.Touche2=Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite[1]; 
        break;
    }
    if (!write_word_le(Handle, CFG_Infos_touche.Number) ||
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
  Chunk.Number=CHUNK_MODES_VIDEO;
  Chunk.Taille=Modes_a_sauver * sizeof(CFG_Mode_video);

  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=1; Indice<Nb_modes_video; Indice++)
    if (Mode_video[Indice].Etat==0 || Mode_video[Indice].Etat==2 || Mode_video[Indice].Etat==3)
    {
      CFG_Mode_video.Etat   =Mode_video[Indice].Etat;
      CFG_Mode_video.Width=Mode_video[Indice].Width;
      CFG_Mode_video.Height=Mode_video[Indice].Height;

      if (!write_byte(Handle, CFG_Mode_video.Etat) ||
        !write_word_le(Handle, CFG_Mode_video.Width) ||
        !write_word_le(Handle, CFG_Mode_video.Height) )
        goto Erreur_sauvegarde_config;
    }

  // Ecriture des données du Shade (précédées du shade en cours)
  Chunk.Number=CHUNK_SHADE;
  Chunk.Taille=sizeof(Shade_Liste)+sizeof(Shade_Actuel);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Shade_Actuel))
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<8; Indice++)
  {
    for (Indice2=0; Indice2<512; Indice2++)
    {
      if (! write_word_le(Handle, Shade_Liste[Indice].List[Indice2]))
        goto Erreur_sauvegarde_config;
    }
    if (! write_byte(Handle, Shade_Liste[Indice].Step) ||
      ! write_byte(Handle, Shade_Liste[Indice].Mode) )
    goto Erreur_sauvegarde_config;
  }

  // Sauvegarde des informations du Masque
  Chunk.Number=CHUNK_MASQUE;
  Chunk.Taille=sizeof(Mask_table);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_bytes(Handle, Mask_table,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Stencil
  Chunk.Number=CHUNK_STENCIL;
  Chunk.Taille=sizeof(Stencil);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_bytes(Handle, Stencil,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations des dégradés
  Chunk.Number=CHUNK_DEGRADES;
  Chunk.Taille=sizeof(Degrade_Tableau)+1;
  if (!write_byte(Handle, Chunk.Number) ||
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
  Chunk.Number=CHUNK_SMOOTH;
  Chunk.Taille=sizeof(Smooth_Matrice);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  for (Indice=0; Indice<3; Indice++)
    for (Indice2=0; Indice2<3; Indice2++)
      if (!write_byte(Handle, Smooth_Matrice[Indice][Indice2]))
        goto Erreur_sauvegarde_config;

  // Sauvegarde des couleurs à exclure
  Chunk.Number=CHUNK_EXCLUDE_COLORS;
  Chunk.Taille=sizeof(Exclude_color);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
 if (!write_bytes(Handle, Exclude_color, 256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Quick-shade
  Chunk.Number=CHUNK_QUICK_SHADE;
  Chunk.Taille=sizeof(Quick_shade_Step)+sizeof(Quick_shade_Loop);
  if (!write_byte(Handle, Chunk.Number) ||
      !write_word_le(Handle, Chunk.Taille) )
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Quick_shade_Step))
    goto Erreur_sauvegarde_config;
  if (!write_byte(Handle, Quick_shade_Loop))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations de la grille
  Chunk.Number=CHUNK_GRILLE;
  Chunk.Taille=8;
  if (!write_byte(Handle, Chunk.Number) ||
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
        Config_Touche[Ordonnancement[Indice]&0xFF][0]=ConfigTouche[Indice].Touche;
        Config_Touche[Ordonnancement[Indice]&0xFF][1]=ConfigTouche[Indice].Touche2;
        break;
      case 1 :
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche[0] = ConfigTouche[Indice].Touche;
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_gauche[1] = ConfigTouche[Indice].Touche2;
        break;
      case 2 :
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite[0] = ConfigTouche[Indice].Touche;
        Bouton[Ordonnancement[Indice]&0xFF].Raccourci_droite[1] = ConfigTouche[Indice].Touche2;
        break;
    }
  }
  // Shade
  Shade_Actuel=0;
  for (Indice=0; Indice<8; Indice++)
  {
    Shade_Liste[Indice].Step=1;
    Shade_Liste[Indice].Mode=0;
    for (Indice2=0; Indice2<512; Indice2++)
      Shade_Liste[Indice].List[Indice2]=256;
  }
  // Shade par défaut pour la palette standard
  for (Indice=0; Indice<7; Indice++)
    for (Indice2=0; Indice2<16; Indice2++)
      Shade_Liste[0].List[Indice*17+Indice2]=Indice*16+Indice2+16;

  Liste2tables(Shade_Liste[Shade_Actuel].List,
            Shade_Liste[Shade_Actuel].Step,
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

#ifdef GRAFX2_CATCHES_SIGNALS

#if defined(__WIN32__)
  #define SIGHANDLER_T __p_sig_fn_t
#elif defined(__macosx__)
  #define SIGHANDLER_T sig_t
#else
  #define SIGHANDLER_T __sighandler_t
#endif

// Memorize the signal handlers of SDL
SIGHANDLER_T Handler_TERM=SIG_DFL;
SIGHANDLER_T Handler_INT=SIG_DFL;
SIGHANDLER_T Handler_ABRT=SIG_DFL;
SIGHANDLER_T Handler_SEGV=SIG_DFL;
SIGHANDLER_T Handler_FPE=SIG_DFL;

void sig_handler(int sig)
{
  // Restore default behaviour
  signal(SIGTERM, Handler_TERM);
  signal(SIGINT, Handler_INT);
  signal(SIGABRT, Handler_ABRT);
  signal(SIGSEGV, Handler_SEGV);
  signal(SIGFPE, Handler_FPE);
  
  switch(sig)
  {
    case SIGTERM:
    case SIGINT:
    case SIGABRT:
    case SIGSEGV:
      Image_emergency_backup();
    default:
    break;
   }
}
#endif

void Initialiser_sighandler(void)
{
#ifdef GRAFX2_CATCHES_SIGNALS
  Handler_TERM=signal(SIGTERM,sig_handler);
  Handler_INT =signal(SIGINT,sig_handler);
  Handler_ABRT=signal(SIGABRT,sig_handler);
  Handler_SEGV=signal(SIGSEGV,sig_handler);
  Handler_FPE =signal(SIGFPE,sig_handler);
#endif
}

