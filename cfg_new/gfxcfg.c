/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Adrien Destugues
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
//C
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//POSIX
#include <unistd.h>

//SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_events.h>
//#include <SDL_gfxPrimitives.h>

//mine
#include "SFont.h"

// Modificateurs pour Touche
#define MOD_SHIFT 0x1000
#define MOD_CTRL  0x2000
#define MOD_ALT   0x4000

#include "../struct.h"
#include "../clavier.h"
#include "../const.h"
#include "../io.h"


/*** Constants ***/
#define NB_MAX_OPTIONS 134
#define HAUTEUR_DEBUT_SETUP 7
#define HAUTEUR_FIN_SETUP 44
#define Header_size sizeof(struct Config_Header)
#define Chunk_size sizeof(struct Config_Chunk)

/* Colors */
#define COULEUR_SETUP 1
#define COULEUR_SELECT 8

/*** Types definitions and structs ***/

typedef struct{
    uint16_t Numero;
    uint16_t Touche;
    uint16_t Touche2;
    char Libelle[36];
    char Explic1[77];
    char Explic2[77];
    bool Erreur;
    bool Suppr;
} __attribute__((__packed__)) Options;

/*** Global variables ***/
SFont_Font* MyFont;
SDL_Surface* Ecran;
int Numero_definition_option;
Options Config[NB_MAX_OPTIONS];
uint8_t Choix_enreg;
uint16_t NB_OPTIONS;
uint16_t Decalage_curseur=0;
uint16_t Position_curseur=0;

byte * FichierConfig = NULL;
byte * ChunkData[CHUNK_MAX];
struct Config_Chunk Chunk[CHUNK_MAX];


uint8_t Fenetre_choix(int Largeur, int Hauteur, const char* Titre, const char* Choix, uint8_t Choix_debut,
	uint8_t Couleur,uint8_t Couleur_choix);

/*** Fonctions de gestion des évènements SDL ***/

/* Attend qu'une touche soit pressée. Retourne le code touche. */
word Lire_Touche(void)
{
    SDL_Event Event;

    do{
        SDL_WaitEvent(&Event);
        if(Event.type==SDL_KEYDOWN)
        {
          word Touche = Conversion_Touche(Event.key.keysym);
          if (Touche != 0)
            return Touche;
        }
    }while(1);
}

/*** Drawing functions ***/

/* Draws a filled rectanble */
void Cadre(int x,int y,int w,int h,uint8_t color)
{
    SDL_Rect rct;
    rct.x=x;
    rct.y=y;
    rct.w=w;
    rct.h=h;

    SDL_FillRect(Ecran, &rct, color);
}

/* Draws the main screen and welcome message */
void Dessiner_ecran_principal()
{
    Cadre(3,3,630,40,2);
    SFont_Write(Ecran, MyFont, 8,6,"Setup program for Grafx2 (c) 1996-98 Sunset Design and 2008 PulkoMandy");
    SFont_Write(Ecran, MyFont, 8,18,"Use Up/Down arrows & Page-Up/Page-Down to scroll, Enter to modify, Delete to remove a hot-key, and Escape to validate or cancel.");
    SFont_Write(Ecran, MyFont, 8,30,"DO NOT USE Print-screen, Pause, and other special keys!");

//    Cadre(3,46,630,400,COULEUR_SETUP);
    SFont_Write(Ecran,MyFont,8,48,"Option");
    SFont_Write(Ecran,MyFont,8*38,48,"Hot-Key");
    SFont_Write(Ecran,MyFont,8*75,48,"Err");

    SDL_UpdateRect(Ecran,0,0,640,480);
}

/* Displays informations about an option */
void Ecrire(uint8_t Ligne, uint16_t Num_option, uint8_t Couleur)
{
    Cadre(3,Ligne*9,630,9,Couleur);
    SFont_Write(Ecran,MyFont,8,Ligne*9,Config[Num_option].Libelle);
    SFont_Write(Ecran,MyFont,40*8,Ligne*9,Nom_touche(Config[Num_option].Touche));
    if(Config[Num_option].Erreur)
        SFont_Write(Ecran,MyFont,77*8,Ligne*9,"X");
    Cadre(36*8,Ligne*9,1,9,255);
    Cadre(74*8,Ligne*9,1,9,255);
    SDL_UpdateRect(Ecran,3,Ligne*9,630,9);
}

/* Displays comment about an option */
void Ecrire_commentaire(uint16_t Num_option)
{
    Cadre(8,50*9,630,18,0);
    SFont_Write(Ecran,MyFont,8,50*9,Config[Num_option].Explic1);
    SFont_Write(Ecran,MyFont,8,51*9,Config[Num_option].Explic2);
    SDL_UpdateRect(Ecran,8,50*9,631,19);
}

/* Display the options list */
void Tout_ecrire()
{
    uint8_t i = HAUTEUR_DEBUT_SETUP;

    Cadre(3,46,630,400,COULEUR_SETUP);
    while(i<=HAUTEUR_FIN_SETUP && i <= NB_OPTIONS + HAUTEUR_DEBUT_SETUP)
    {
        Ecrire(i,Decalage_curseur+i-HAUTEUR_DEBUT_SETUP,
                (i==HAUTEUR_DEBUT_SETUP+Position_curseur)?COULEUR_SELECT:COULEUR_SETUP);
        i++;
    }

    Cadre(36*8,46,1,400,255);
    Cadre(74*8,46,1,400,255);

    SDL_UpdateRect(Ecran,0,0,640,480);

    Ecrire_commentaire(Decalage_curseur+Position_curseur);
}

/*** Configuration handling functions ***/


/* Reads the config file */
/* returns an error message, or NULL if everything OK */
char * Interpretation_du_fichier_config()
{

  FILE* Fichier;
  long int Taille_fichier;
  byte Numero_chunk;
  word Taille_chunk;
  byte * Ptr;
  int i;

  Fichier = fopen("gfx2.cfg","rb");
  if (!Fichier)
  {
    return "gfx2.cfg is missing! Please run the\nmain program to generate it.";
  }
  fseek(Fichier,0,SEEK_END); // Positionnement à la fin
  Taille_fichier = ftell(Fichier);
  if (!Taille_fichier)
  {
    fclose(Fichier);
    return "gfx2.cfg is empty. Please run the main\nprogram to generate it.";
  }
  FichierConfig = malloc(Taille_fichier);
  if (!Fichier)
  {
    fclose(Fichier);
    return "Out of memory when reading gfx2.cfg.";
  }
  fseek(Fichier,0,SEEK_SET); // Positionnement au début
  if (! read_bytes(Fichier, FichierConfig, Taille_fichier))
  {
    fclose(Fichier);
    return "Error while reading gfx2.cfg.";
  }
  fclose(Fichier);
  
  // Initialisation des "index"
  for (i=0; i<CHUNK_MAX; i++)
  {
    Chunk[i].Taille=0;
    Chunk[i].Numero=i;
    ChunkData[i]=NULL;
  }
  
  // Pour faire simple, on saute l'en-tete
  Ptr = FichierConfig + Header_size;
  while (Ptr < FichierConfig + Taille_fichier)
  {
    // Lecture chunk
    Numero_chunk = *Ptr;
    Ptr++;
    Taille_chunk = endian_magic16(*((word *)Ptr)); 
    Ptr+=2;
    if (Numero_chunk>= CHUNK_MAX)
      return "File gfx2.cfg invalid.";

    ChunkData[Numero_chunk] = Ptr;
    Chunk[Numero_chunk].Taille = Taille_chunk;
    printf("%d %6X %d\n", Numero_chunk, Ptr - FichierConfig, Taille_chunk);
    Ptr+=Taille_chunk;
  }
  // Si la config contenait des touches, on les initialise:
  if (Chunk[CHUNK_TOUCHES].Taille)
  {
    int Indice_config;
    Ptr = ChunkData[CHUNK_TOUCHES];
    for (Indice_config=0; Indice_config< Chunk[CHUNK_TOUCHES].Taille / sizeof(struct Config_Infos_touche) ; Indice_config++)
    {
      word Numero;
      word Touche;
      word Touche2;
      int Indice_touche;
      
      Numero = endian_magic16(*((word *)Ptr));
      Ptr+=2;
      Touche = endian_magic16(*((word *)Ptr));
      Ptr+=2;
      Touche2 = endian_magic16(*((word *)Ptr));
      Ptr+=2;
      //printf("%4d %4X %4X\t", Numero, Touche, Touche2);
      // Recherche de la touche qui porte ce numéro
      for (Indice_touche=0; Indice_touche <= NB_OPTIONS; Indice_touche ++)
      {
        if (Config[Indice_touche].Numero == Numero)
        {
          Config[Indice_touche].Touche = Touche;
          Config[Indice_touche].Touche2 = Touche2;
// Utilisé pour afficher la liste complète des raccourcis dans le format du wiki...
/*          printf("||%s||%s||%s %s||\n", Config[Indice_touche].Libelle, 
			    Nom_touche(Touche),
			    Config[Indice_touche].Explic1, 
			    Config[Indice_touche].Explic2); */
          break;
        }
      }
      
    }
  } 
  return NULL;
}

/* Defines an option */
void Definir_option(uint16_t Numero, char* Libelle, char* Explic1, char* Explic2, bool Deletable, 
        uint16_t Default_key)
{
    Config[Numero_definition_option].Numero = Numero;
    Config[Numero_definition_option].Touche = Default_key;
    Config[Numero_definition_option].Touche2 = 0xFF;
    strncpy(Config[Numero_definition_option].Libelle,Libelle,36);
    strncpy(Config[Numero_definition_option].Explic1,Explic1,77);
    strncpy(Config[Numero_definition_option].Explic2,Explic2,77);
    Config[Numero_definition_option].Erreur = false;
    Config[Numero_definition_option].Suppr = Deletable;
    Numero_definition_option ++ ;	
}

/* Initialize configuration */
bool Initialiser_config()
{
    char * MessageErreur = NULL;

    Numero_definition_option = 0;

    Definir_option(0,"Scroll up",
            "Scrolls the picture up, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, SDLK_UP); // HAUT
    Definir_option(1,"Scroll down",
            "Scrolls the picture down, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, SDLK_DOWN); // BAS
    Definir_option(2,"Scroll left",
            "Scrolls the picture to the left, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, SDLK_LEFT); // GAUCHE
    Definir_option(3,"Scroll right",
            "Scrolls the picture to the right, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, SDLK_RIGHT); // DROITE
    Definir_option(4,"Faster scroll up",
	    "Used to scroll upwards in the picture fast, either in magnify and normal",
	    "mode.",
	    true,SDLK_UP|MOD_SHIFT);  /*Shift + Haut*/
    Definir_option(5,"Faster scroll down",
	    "Used to scroll downwards in the picture fast, either in magnify and normal",
	    "mode.",
	    true,SDLK_DOWN|MOD_SHIFT);  /*Shift + Bas*/
    Definir_option(6,"Faster scroll left",
	    "Used to scroll to the left in the picture fast, either in magnify and normal",
	    "mode.",
	    true,SDLK_LEFT|MOD_SHIFT);  /*Shift + Gauche*/
    Definir_option(7,"Faster scroll right",
	    "Used to scroll to the right in the picture fast, either in magnify and",
	    "normal mode.",
	    true,SDLK_RIGHT|MOD_SHIFT);  /*Shift + Droite*/
    Definir_option(8,"Slower scroll up",
	    "Used to scroll upwards in the picture pixel by pixel, either in magnify and",
	    "normal mode.",
	    true,SDLK_UP|MOD_ALT);  /*Alt + Haut*/
    Definir_option(9,"Slower scroll down",
	    "Used to scroll downwards in the picture pixel by pixel, either in magnify and",
	    "normal mode.",
	    true,SDLK_DOWN|MOD_ALT);  /*Alt + Bas*/
    Definir_option(10,"Slower scroll left",
	    "Used to scroll to the left in the picture pixel by pixel, either in magnify",
	    "and normal mode.",
	    true,SDLK_LEFT|MOD_ALT);  /*Alt + Gauche*/
    Definir_option(11,"Slower scroll right",
	    "Used to scroll to the right in the picture pixel by pixel, either in magnify",
	    "and normal mode.",
	    true,SDLK_RIGHT|MOD_ALT);  /*Alt + Droite*/
    Definir_option(12,"Move mouse cursor 1 pixel up",
	    "Used to simulate a very small mouse deplacement upwards.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_UP|MOD_CTRL);  /*Ctrl + Haut*/
    Definir_option(13,"Move mouse cursor 1 pixel down",
	    "Used to simulate a very small mouse deplacement downwards.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_DOWN|MOD_CTRL);  /*Ctrl + Bas*/
    Definir_option(14,"Move mouse cursor 1 pixel left",
	    "Used to simulate a very small mouse deplacement to the left.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_LEFT|MOD_CTRL);  /*Ctrl + Gauche*/
    Definir_option(15,"Move mouse cursor 1 pixel right",
	    "Used to simulate a very small mouse deplacement to the right.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_RIGHT|MOD_CTRL);  /*Ctrl + Droite*/
    Definir_option(16,"Simulate left mouse click",
	    "Used to simulate a click with the left mouse button..",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_SPACE);  /*Space*/
    Definir_option(17,"Simulate right mouse click",
	    "Used to simulate a click with the right mouse button..",
	    "It""s very useful when you want a ultra-high precision.",
	    true,SDLK_SPACE|MOD_SHIFT);  /*Shift + Space*/
    Definir_option(18,"Show/hide option menu",
	    "Switch the tool bar display on/off.",
	    "This hot-key cannot be removed.",
	    false,SDLK_F10); /*F10*/
    Definir_option(19,"Show/hide cursor",
	    "Switch the cursor display on/off.",
	    "This only works on the \"small cross\" and \"hand\" cursors.",
	    true,SDLK_F9);  /*F9*/
    Definir_option(20,"Set paintbrush to 1 pixel",
	    "Useful when you want to use a \"single-pixel-brush\".",
	    "",
	    true,SDLK_DELETE);  /*Del*/
    Definir_option(21,"Paintbrush choice",
	    "Opens a menu where you can choose a paintbrush out of 24 predefined ones.",
	    "",
	    true,SDLK_F4);  /*F4*/
    Definir_option(22,"Monochrome brush",
	    "Turn your current user-defined brush into a single colored one.",
	    "All non-transparent colors are set to current foreground color.",
	    true,SDLK_F4|MOD_SHIFT);  /*Shift + F4*/
    Definir_option(23,"Freehand drawing",
	    "Set the drawing mode to the classical freehand one.",
	    "",
	    true,SDLK_d);  /*D*/
    Definir_option(24,"Switch freehand drawing mode",
	    "Switch between the 3 ways to use freehand drawing.",
	    "These modes are: continuous, discontinuous and point by point.",
	    true,SDLK_d|MOD_SHIFT);  /*Shift + D*/
    Definir_option(25,"Continuous freehand drawing",
	    "Switch directly to continuous freehand drawing mode.",
	    "",
	    true,SDLK_d|MOD_CTRL);  /*Ctrl + D*/
    Definir_option(26,"Line",
	    "Allows you to draw lines.",
	    "",
	    true,SDLK_l);  /*L*/
    Definir_option(27,"Knotted lines",
	    "Allows you to draw linked lines.",
	    "This mode can also be called \"Polyline\".",
	    true,SDLK_l|MOD_SHIFT);  /*Shift + L*/
    Definir_option(28,"Spray",
	    "Allows you to spray brushes randomly in the picture.",
	    "",
	    true,SDLK_a);  /*A (Q en AZERTY)*/
    Definir_option(29,"Spray menu",
	    "Opens a menu in which you can configure the spray flow and size.",
	    "",
	    true,SDLK_a|MOD_SHIFT);  /*Shift + A*/
    Definir_option(30,"Flood-fill",
	    "Allows you to fill an area of the picture made of pixels of the same color.",
	    "",
	    true,SDLK_f);  /*F*/
    Definir_option(124,"Replace color",
	    "Allows you to replace all the pixels of the color pointed by the mouse with",
	    "the fore-color or the back-color.",
	    true,SDLK_f|MOD_SHIFT);  /*Shift + F*/
    Definir_option(31,"Bezier""s curves",
	    "Allows you to draw Bezier""s curves.",
	    "",
	    true,SDLK_i);  /*I*/
    Definir_option(32,"Bezier""s curve with 3 or 4 points",
	    "Allows you to choose whether you want to draw Bezier""s curves with 3 or 4",
	    "points.",
	    true,SDLK_i|MOD_SHIFT);  /*Shift + I*/
    Definir_option(33,"Empty rectangle",
	    "Allows you to draw a rectangle using the brush.",
	    "",
	    true,SDLK_r);  /*R*/
    Definir_option(34,"Filled rectangle",
	    "Allows you to draw a filled rectangle.",
	    "",
	    true,SDLK_r|MOD_SHIFT);  /*Shift + R*/
    Definir_option(35,"Empty circle",
	    "Allows you to draw a circle using the brush.",
	    "",
	    true,SDLK_c);  /*C*/
    Definir_option(36,"Empty ellipse",
	    "Allows you to draw an ellipse using the brush.",
	    "",
	    true,SDLK_c|MOD_CTRL);  /*Ctrl + C*/
    Definir_option(37,"Filled circle",
	    "Allows you to draw a filled circle.",
	    "",
	    true,SDLK_c|MOD_SHIFT);  /*Shift + C*/
    Definir_option(38,"Filled ellipse",
	    "Allows you to draw a filled ellipse.",
	    "",
	    true,SDLK_c|MOD_SHIFT|MOD_CTRL);  /*Shift + Ctrl + C*/
    Definir_option(39,"Empty polygon",
	    "Allows you to draw a polygon using the brush.",
	    "",
	    true,SDLK_n);  /*N*/
    Definir_option(40,"Empty \"polyform\"",
	    "Allows you to draw a freehand polygon using the brush.",
	    "",
	    true,SDLK_n|MOD_CTRL);  /*Ctrl + N*/
    Definir_option(41,"Filled polygon",
	    "Allows you to draw a filled polygon.",
	    "",
	    true,SDLK_n|MOD_SHIFT);  /*Shift + N*/
    Definir_option(42,"Filled \"polyform\"",
	    "Allows you to draw a filled freehand polygon.",
	    "",
	    true,SDLK_n|MOD_SHIFT|MOD_CTRL);  /*Shift + Ctrl + N*/
    Definir_option(43,"Rectangle with gradation",
	    "Allows you to draw a rectangle with a color gradation.",
	    "",
	    true,SDLK_r|MOD_ALT);  /*Alt + R*/
    Definir_option(44,"Gradation menu",
	    "Allows you to configure the way color gradations are calculated.",
	    "",
	    true,SDLK_g|MOD_ALT);  /*Alt + G*/
    Definir_option(45,"Sphere with gradation",
	    "Allows you to draw a rectangle with a color gradation.",
	    "",
	    true,SDLK_c|MOD_ALT);  /*Alt + C*/
    Definir_option(46,"Ellipse with gradation",
	    "Allows you to draw an ellipse filled with a color gradation.",
	    "",
	    true,SDLK_c|MOD_SHIFT|MOD_ALT);  /*Shift + Alt + C*/
    Definir_option(47,"Adjust picture",
	    "Allows you to move the whole picture in order to re-center it.",
	    "Notice that what gets out from a side reappears on the other.",
	    true,SDLK_KP5);  /*Kpad5*/
    Definir_option(48,"Flip/shrink picture menu",
	    "Opens a menu which allows you to flip the picture horizontally/vertically or",
	    "to shrink it to half-scale horizontally and/or vertically.",
	    true,SDLK_KP5|MOD_SHIFT);  /*Shift + Kpad5*/
    Definir_option(49,"Drawing effects",
	    "Opens a menu where you can enable/disable and configure the drawing effects",
	    "listed below.",
	    true,SDLK_e);  /*E*/
    Definir_option(50,"Shade mode",
	    "Allows you to shade or lighten some pixels of the picture belonging to a",
	    "color range, in addition of any drawing tool.",
	    true,SDLK_F5);  /*F5*/
    Definir_option(51,"Shade menu",
	    "Opens a menu where you can choose color ranges to use with the Shade mode.",
	    "This menu also contains parameters used both in Shade and Quick-shade modes.",
	    true,SDLK_F5|MOD_SHIFT);  /*Shift + F5*/
    Definir_option(131,"Quick-shade mode",
	    "Does the same thing as shade mode with a simpler method (faster to define",
	    "but a bit less powerful).",
	    true,SDLK_F5|MOD_CTRL);  /*Ctrl + F5*/
    Definir_option(132,"Quick-shade menu",
	    "Opens a menu where you can define the parameters of the quick-shade mode.",
	    "",
	    true,SDLK_F5|MOD_SHIFT|MOD_CTRL);  /*Shift + Ctrl + F5*/
    Definir_option(52,"Stencil mode",
	    "Allows you to mask colors that must not be affected when you are drawing.",
	    "",
	    true,SDLK_F6);  /*F6*/
    Definir_option(53,"Stencil menu",
	    "Opens a menu where you can choose colors masked by the Stencil mode.",
	    "",
	    true,SDLK_F6|MOD_SHIFT);  /*Shift + F6*/
    Definir_option(54,"Mask mode",
	    "Allows you to mask colors of the spare page that will keep you from ",
	    "drawing. This mode should be called \"True stencil\".",
	    true,SDLK_F6|MOD_ALT);  /*Alt + F6*/
    Definir_option(55,"Mask menu",
	    "Opens a menu where you can choose colors for the Mask mode.",
	    "",
	    true,SDLK_F6|MOD_SHIFT|MOD_ALT);  /*Shift + Alt + F6*/
    Definir_option(56,"Grid mode",
	    "Force the cursor to snap up grid points.",
	    "",
	    true,SDLK_g);  /*G*/
    Definir_option(57,"Grid menu",
	    "Open a menu where you can configure the grid used by Grid mode.",
	    "",
	    true,SDLK_g|MOD_SHIFT);  /*Shift + G*/
    Definir_option(58,"Sieve mode",
	    "Only draws pixels on certain positions matching with a sieve.",
	    "",
	    true,SDLK_g|MOD_CTRL);  /*Ctrl + G*/
    Definir_option(59,"Sieve menu",
	    "Opens a menu where you can configure the sieve.",
	    "",
	    true,SDLK_g|MOD_SHIFT|MOD_CTRL);  /*Shift + Ctrl + G*/
    Definir_option(60,"Invert sieve",
	    "Inverts the pattern defined in the Sieve menu.",
	    "",
	    true,SDLK_g|MOD_CTRL|MOD_ALT);  /*Ctrl + Alt + G*/
    Definir_option(61,"Colorize mode",
	    "Allows you to colorize the pixels on which your brush is pasted.",
	    "This permits you to make transparency effects.",
	    true,SDLK_F7);  /*F7*/
    Definir_option(62,"Colorize menu",
	    "Opens a menu where you can give the opacity percentage for Colorize mode.",
	    "",
	    true,SDLK_F7|MOD_SHIFT);  /*Shift + F7*/
    Definir_option(63,"Smooth mode",
	    "Soften pixels on which your brush is pasted.",
	    "",
	    true,SDLK_F8);  /*F8*/
    Definir_option(123,"Smooth menu",
	    "Opens a menu where you can define the Smooth matrix.",
	    "",
	    true,SDLK_F8|MOD_SHIFT);  /*Shift + F8*/
    Definir_option(64,"Smear mode",
	    "Smears the pixels when you move your brush on the picture.",
	    "",
	    true,SDLK_F8|MOD_ALT);  /*Alt + F8*/
    Definir_option(65,"Tiling mode",
	    "Puts parts of the brush where you draw.",
	    "",
	    true,SDLK_b|MOD_ALT);  /*Alt + B*/
    Definir_option(66,"Tiling menu",
	    "Opens a menu where you can configure the origin of the tiling.",
	    "",
	    true,SDLK_b|MOD_SHIFT|MOD_ALT);  /*Shift + Alt + B*/
    Definir_option(67,"Classical brush grabbing",
	    "Allows you to pick a brush defined within a rectangle.",
	    "",
	    true,SDLK_b);  /*B*/
    Definir_option(68,"\"Lasso\" brush grabbing",
	    "Allows you to pick a brush defined within a freehand polygon.",
	    "",
	    true,SDLK_b|MOD_CTRL);  /*Ctrl + B*/
    Definir_option(69,"Get previous brush back",
	    "Restore the last user-defined brush.",
	    "",
	    true,SDLK_b|MOD_SHIFT);  /*Shift + B*/
    Definir_option(70,"Horizontal brush flipping",
	    "Reverse brush horizontally.",
	    "",
	    true,SDLK_x);  /*X*/
    Definir_option(71,"Vertical brush flipping",
	    "Reverse brush vertically.",
	    "",
	    true,SDLK_y);  /*Y*/
    Definir_option(72,"90ø brush rotation",
	    "Rotate the user-defined brush by 90ø (counter-clockwise).",
	    "",
	    true,SDLK_z);  /*Z (W en AZERTY)*/
    Definir_option(73,"180ø brush rotation",
	    "Rotate the user-defined brush by 180ø.",
	    "",
	    true,SDLK_z|MOD_SHIFT);  /*Shift + Z*/
    Definir_option(74,"Strech brush",
	    "Allows you to resize the user-defined brush.",
	    "",
	    true,SDLK_s);  /*S*/
    Definir_option(75,"Distort brush",
	    "Allows you to distort the user-defined brush.",
	    "",
	    true,SDLK_s|MOD_SHIFT);  /*Shift + S*/
    Definir_option(76,"Outline brush",
	    "Outlines the user-defined brush with the fore color.",
	    "",
	    true,SDLK_o);  /*O*/
    Definir_option(77,"Nibble brush",
	    "Deletes the borders of the user-defined brush.",
	    "This does the opposite of the Outline option.",
	    true,SDLK_o|MOD_SHIFT);  /*Shift + O*/
    Definir_option(78,"Get colors from brush",
	    "Copy colors of the spare page that are used in the brush.",
	    "",
	    true,SDLK_F11);  /*F11*/
    Definir_option(79,"Recolorize brush",
	    "Recolorize pixels of the user-defined brush in order to get a brush which",
	    "looks like the one grabbed in the spare page.",
	    true,SDLK_F12);  /*F12*/
    Definir_option(80,"Rotate by any angle",
	    "Rotate the brush by an angle that you can define.",
	    "",
	    true,SDLK_w);  /*W (Z en AZERTY)*/
    Definir_option(81,"Pipette",
	    "Allows you to copy the color of a pixel in the picture into the foreground",
	    "or background color.",
	    true,SDLK_BACKQUOTE);  /*`~ (Touche sous le Esc - ² en AZERTY)*/
    Definir_option(82,"Swap foreground/background colors",
	    "Invert foreground and background colors.",
	    "",
	    true,SDLK_BACKQUOTE|MOD_SHIFT);  /*Shift + `~*/
    Definir_option(83,"Magnifier mode",
	    "Allows you to zoom into the picture.",
	    "",
	    true,SDLK_m);  /*M (,? sur AZERTY)*/
    Definir_option(84,"Zoom factor menu",
	    "Opens a menu where you can choose a magnifying factor.",
	    "",
	    true,SDLK_m|MOD_SHIFT);  /*Shift + M*/
    Definir_option(85,"Zoom in",
	    "Increase magnifying factor.",
	    "",
	    true,SDLK_KP_PLUS);  /*Grey +*/
    Definir_option(86,"Zoom out",
	    "Decrease magnifying factor.",
	    "",
	    true,SDLK_KP_MINUS);  /*Grey -*/
    Definir_option(87,"Brush effects menu",
	    "Opens a menu which proposes different effects on the user-defined brush.",
	    "",
	    true,SDLK_b|MOD_CTRL|MOD_ALT);  /*Ctrl + Alt + B*/
    Definir_option(88,"Text",
	    "Opens a menu which permits you to type in a character string and to choose a",
	    "font, and then creates a new user-defined brush fitting to your choices.",
	    true,SDLK_t);  /*T*/
    Definir_option(89,"Screen resolution menu",
	    "Opens a menu where you can choose the dimensions of the screen in which you",
	    "want to draw among the numerous X and SVGA proposed modes.",
	    true,SDLK_RETURN);  /*Enter*/
    Definir_option(90,"\"Safety\" resolution",
	    "Set resolution to 320x200. This can be useful if you choosed a resolution",
	    "that is not supported by your monitor and video card. Cannot be removed.",
	    false,SDLK_RETURN|MOD_SHIFT); /*Shift + Enter*/
    Definir_option(91,"Help and credits",
	    "Opens a window where you can get information about the program.",
	    "",
	    true,SDLK_F1);  /*F1*/
    Definir_option(92,"Statistics",
	    "Displays miscellaneous more or less useful information.",
	    "",
	    true,SDLK_F1|MOD_SHIFT);  /*Shift + F1*/
    Definir_option(93,"Jump to spare page",
	    "Swap current page and spare page.",
	    "",
	    true,SDLK_TAB);  /*Tab*/
    Definir_option(94,"Copy current page to spare page",
	    "Copy current page to spare page.",
	    "",
	    true,SDLK_TAB|MOD_SHIFT);  /*Shift + Tab*/
    Definir_option(95,"Save picture as...",
	    "Opens a file-selector that allows you to save your picture with a new",
	    "path-name.",
	    true,SDLK_F2);  /*F2*/
    Definir_option(96,"Save picture",
	    "Saves your picture with the last name you gave it.",
	    "",
	    true,SDLK_F2|MOD_SHIFT);  /*Shift + F2*/
    Definir_option(97,"Load picture",
	    "Opens a file-selector that allows you to load a new picture.",
	    "",
	    true,SDLK_F3);  /*F3*/
    Definir_option(98,"Re-load picture",
	    "Re-load the current picture.",
	    "This allows you to cancel modifications made since last saving.",
	    true,SDLK_F3|MOD_SHIFT);  /*Shift + F3*/
    Definir_option(99,"Save brush",
	    "Opens a file-selector that allows you to save your current user-defined",
	    "brush.",
	    true,SDLK_F2|MOD_CTRL);  /*Ctrl + F2*/
    Definir_option(100,"Load brush",
	    "Opens a file-selector that allows you to load a brush.",
	    "",
	    true,SDLK_F3|MOD_CTRL);  /*Ctrl + F3*/
    Definir_option(101,"Settings",
	    "Opens a menu which permits you to set the dimension of your picture, and to",
	    "modify some parameters of the program.",
	    true,SDLK_F10|MOD_SHIFT);  /*Shift + F10*/
    Definir_option(102,"Undo (Oops!)",
	    "Cancel the last action which modified the picture. This has no effect after",
	    "a jump to the spare page, loading a picture or modifying its size.",
	    true,SDLK_u);  /*U*/
    Definir_option(103,"Redo",
	    "Redo the last undone action. This has no effect after a jump to the spare",
	    "page, loading a picture or modifying its size.",
	    true,SDLK_u|MOD_SHIFT);  /*Shift + U*/
    Definir_option(133,"Kill",
	    "Kills the current page. It actually removes the current page from the list",
	    "of \"Undo\" pages.",
	    true,SDLK_DELETE|MOD_SHIFT);  /*Shift + Suppr*/
    Definir_option(104,"Clear page",
	    "Clears the picture with the first color of the palette (usually black).",
	    "",
	    true,SDLK_BACKSPACE);  /*BackSpace*/
    Definir_option(105,"Clear page with backcolor",
	    "Clears the picture with the backcolor.",
	    "",
	    true,SDLK_BACKSPACE|MOD_SHIFT);  /*Shift + BackSpace*/
    Definir_option(106,"Quit program",
	    "Allows you to leave the program.",
	    "If modifications were not saved, confirmation is asked.",
	    false,SDLK_q);  /*Q (A en AZERTY)*/
    Definir_option(107,"Palette menu",
	    "Opens a menu which allows you to modify the current palette.",
	    "",
	    true,SDLK_p);  /*P*/
    Definir_option(125,"Secondary palette menu",
	    "Opens a menu which allows you to define color series and some tagged colors.",
	    "",
	    true,SDLK_p|MOD_SHIFT);  /*Shift + P*/
    Definir_option(130,"Exclude colors menu",
	    "Opens a menu which allows you to define the colors you don""t want to use in",
	    "modes such as Smooth and Transparency, or when remapping a brush.",
	    true,SDLK_p|MOD_CTRL);  /*Ctrl + P*/
    Definir_option(108,"Scroll palette to the left",
	    "Scroll palette in the tool bar to the left, column by column.",
	    "",
	    true,SDLK_PAGEUP);  /*PgUp*/
    Definir_option(109,"Scroll palette to the right",
	    "Scroll palette in the tool bar to the right, column by column.",
	    "",
	    true,SDLK_PAGEDOWN);  /*PgDn*/
    Definir_option(110,"Scroll palette to the left faster",
	    "Scroll palette in the tool bar to the left, 8 columns by 8 columns.",
	    "",
	    true,SDLK_PAGEUP|MOD_SHIFT);  /*Shift + PgUp*/
    Definir_option(111,"Scroll palette to the right faster",
	    "Scroll palette in the tool bar to the right, 8 columns by 8 columns.",
	    "",
	    true,SDLK_PAGEDOWN|MOD_SHIFT);  /*Shift + PgDn*/
    Definir_option(112,"Center brush attachment point",
	    "Set the attachement of the user-defined brush to its center.",
	    "",
	    true,SDLK_KP5|MOD_CTRL);  /*Ctrl + 5 (pavé numérique)*/
    Definir_option(113,"Top-left brush attachment point",
	    "Set the attachement of the user-defined brush to its top-left corner.",
	    "",
	    true,SDLK_HOME|MOD_CTRL);  /*Ctrl + 7*/
    Definir_option(114,"Top-right brush attachment point",
	    "Set the attachement of the user-defined brush to its top-right corner.",
	    "",
	    true,SDLK_PAGEUP|MOD_CTRL);  /*Ctrl + 9*/
    Definir_option(115,"Bottom-left brush attachment point",
	    "Set the attachement of the user-defined brush to its bottom-left corner.",
	    "",
	    true,SDLK_END|MOD_CTRL);  /*Ctrl + 1*/
    Definir_option(116,"Bottom-right brush attachment point",
	    "Set the attachement of the user-defined brush to its bottom-right corner.",
	    "",
	    true,SDLK_PAGEDOWN|MOD_CTRL);  /*Ctrl + 3*/
    Definir_option(117,"Next foreground color",
	    "Set the foreground color to the next in the palette.",
	    "",
	    true,SDLK_RIGHTBRACKET);  /*] (0x en AZERTY)*/
    Definir_option(118,"Previous foreground color",
	    "Set the foreground color to the previous in the palette.",
	    "",
	    true,SDLK_LEFTBRACKET);  /*[ (^ en AZERTY)*/
    Definir_option(119,"Next background color",
	    "Set the background color to the next in the palette.",
	    "",
	    true,SDLK_RIGHTBRACKET|MOD_SHIFT);  /*Shift + ]*/
    Definir_option(120,"Previous background color",
	    "Set the background color to the previous in the palette.",
	    "",
	    true,SDLK_LEFTBRACKET|MOD_SHIFT);  /*Shift + [*/
    Definir_option(126,"Next user-defined forecolor",
	    "Set the foreground color to the next in the user-defined color series.",
	    "",
	    true,SDLK_EQUALS);  /*"=+"*/
    Definir_option(127,"Previous user-defined forecolor",
	    "Set the foreground color to the previous in the user-defined color series.",
	    "",
	    true,SDLK_MINUS);  /*"-_" (")ø" en AZERTY*/
    Definir_option(128,"Next user-defined backcolor",
	    "Set the background color to the next in the user-defined color series.",
	    "",
	    true,SDLK_EQUALS|MOD_SHIFT);  /*Shift + "=+"*/
    Definir_option(129,"Previous user-defined backcolor",
	    "Set the background color to the previous in the user-defined color series.",
	    "",
	    true,SDLK_MINUS|MOD_SHIFT);  /*Shift + "-_" (")ø" en AZERTY*/
    Definir_option(121,"Shrink paintbrush",
	    "Decrease the width of the paintbrush if it is special circle or square.",
	    "",
	    true,SDLK_COMMA);  /*,< (;. en AZERTY)*/
    Definir_option(122,"Enlarge paintbrush",
	    "Increase the width of the paintbrush if it is special circle or square.",
	    "",
	    true,SDLK_PERIOD);  /*.> (:/ en AZERTY)*/


    NB_OPTIONS = Numero_definition_option - 1;

    MessageErreur = Interpretation_du_fichier_config();

    if (MessageErreur)
    {
      Fenetre_choix(30,10,MessageErreur,"Ok",0,0x2A,0x6F);
      return 1;
    }
    return 0;
}

uint8_t Fenetre_choix(int Largeur, int Hauteur, const char* Titre, const char* Choix, uint8_t Choix_debut,
	uint8_t Couleur,uint8_t Couleur_choix)
{
    char Temp[70];
    uint8_t i,j,Num_titre,Num_choix;
    uint16_t x1,y1;
    uint8_t Option_choisie;
    word Touche;

    Hauteur *= 9;
    Largeur *= 9;
    x1=(640 - Largeur)/2;
    y1=(480 - Hauteur)/2;

    Cadre(x1+5,y1+5,Largeur,Hauteur,1);
    Cadre(x1,y1,Largeur,Hauteur,Couleur);

    Num_choix =  0;
    Num_titre=1;
    j=0;

    // SFont ne gère pas les \n donc on le fait nous même
    for(i=0;i<=strlen(Titre);i++)
    {   
	if (Titre[i]=='\n' || Titre[i]==0)
	{
	    memcpy(Temp,Titre+j,i-j);
	    Temp[i-j]=0;
	    j=i+1;
	    SFont_Write(Ecran,MyFont,x1+3,y1+Num_titre*9,Temp);
	    Num_titre++;
	}
    }

    // Maintenant on fait pareil pour les divers choix proposés
    j=0;

    // SFont ne gère pas les \n donc on le fait nous même
    for(i=0;i<=strlen(Choix);i++)
    {   
	if (Choix[i]=='\n' || Choix[i]==0)
	{
	    memcpy(Temp,Choix+j,i-j);
	    Temp[i-j]=0;
	    j=i+1;
	    SFont_Write(Ecran,MyFont,x1+3+50*Num_choix,y1+(Num_titre+2)*9,Temp);
	    Num_choix++;
	}
    }

    Option_choisie = Choix_debut;
    Cadre(x1+3+50*Option_choisie,y1+(Num_titre+3)*9,15,2,Couleur_choix);
    SDL_UpdateRect(Ecran,x1,y1,Largeur+5,Hauteur+5);

    do
    {
	Touche = Lire_Touche();
	switch(Touche)
	{
	    case SDLK_LEFT:
		Cadre(x1+3+50*Option_choisie,y1+(Num_titre+3)*9,15,2,Couleur);
		if(Option_choisie==0) Option_choisie = Num_choix - 1;
		else Option_choisie --;
		Cadre(x1+3+50*Option_choisie,y1+(Num_titre+3)*9,15,2,Couleur_choix);
		break;
	    case SDLK_RIGHT:
		Cadre(x1+3+50*Option_choisie,y1+(Num_titre+3)*9,15,2,Couleur);
		if(Option_choisie==Num_choix-1) Option_choisie = 0;
		else Option_choisie ++;
		Cadre(x1+3+50*Option_choisie,y1+(Num_titre+3)*9,15,2,Couleur_choix);
	    default:
		break;

	}
	SDL_UpdateRect(Ecran,x1+3,y1+(Num_titre+3)*9,50*Num_choix,2);
    }while(Touche!=SDLK_RETURN);

    Tout_ecrire(); // Efface la boite de dialogue
    return Option_choisie+1;
}

/* Checks if some key is used twice */
void Test_duplic()
{
    uint16_t i,j;
    bool Pas_encore_erreur;

    for(i=0;i<=NB_OPTIONS;i++)
    {
	if(Config[i].Touche!=0xFF) // FIXME
	{
	    j=0;
	    Pas_encore_erreur=true;
	    while(j<=NB_OPTIONS && Pas_encore_erreur)
	    {
		if(i!=j && Config[i].Touche==Config[j].Touche)
		{
		    Pas_encore_erreur = false;
		    Config[i].Erreur = true;
		}
		j++;
	    }
	    if (Pas_encore_erreur) Config[i].Erreur = false;
	}
    }
}

/* Checks if everything is OK */
bool Validation()
{
    bool Y_a_des_erreurs = false;
    uint16_t i = 0;

    while(i<=NB_OPTIONS && !Y_a_des_erreurs)
    {
	Y_a_des_erreurs = Config[i].Erreur;
	i++;
    }

    if (Y_a_des_erreurs)
	Choix_enreg = 4 - Fenetre_choix(30,12,
		"There are errors in the\nhot-keys configuration.\nCheck out the \"Err\" column\nin order to correct them.","OK\nQuit anyway",1,0x4C,0x3F);
    else
	Choix_enreg = Fenetre_choix(30,10,"Save configuration?","Yes\nNo\nCancel",1,0x2A,0x6F);
    if (Choix_enreg!=3) return true; else return false;
}

/* Move one line up */
void Scroll_haut()
{
    if(Position_curseur + Decalage_curseur > 0)
    {
	if(Position_curseur <= HAUTEUR_FIN_SETUP - HAUTEUR_DEBUT_SETUP && Position_curseur > 0)
	{
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SETUP);
	    (Position_curseur) -- ;
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SELECT);
	}
	else if(Decalage_curseur>0)
	{
	    (Decalage_curseur) -- ;
	    Tout_ecrire();
	}
	Ecrire_commentaire(Position_curseur + Decalage_curseur);
    }
}

/* Moves one line down */
void Scroll_bas()
{
    if(Position_curseur + Decalage_curseur < NB_OPTIONS)
    {
	if(Position_curseur < HAUTEUR_FIN_SETUP - HAUTEUR_DEBUT_SETUP)
	{
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur) ,Position_curseur + Decalage_curseur,
		    COULEUR_SETUP);
	    (Position_curseur) ++ ;
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur) ,Position_curseur + Decalage_curseur,
		    COULEUR_SELECT);
	}
	else if(Decalage_curseur < NB_OPTIONS)
	{
	    (Decalage_curseur) ++ ;
	    Tout_ecrire();
	}
	Ecrire_commentaire(Position_curseur + Decalage_curseur );
    }
}

/* Moves one screen up */
void Page_up()
{
    if(Position_curseur+Decalage_curseur>0)
    {
	if(Position_curseur>0)
	{
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SETUP);
	    Position_curseur = 0 ;
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SELECT);
	}
	else if(Decalage_curseur>0)
	{
	    if(Decalage_curseur > HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)
		Decalage_curseur-=HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP;
	    else Decalage_curseur=0;

	    Tout_ecrire();
	}
	Ecrire_commentaire(Position_curseur+Decalage_curseur);
    }
}

/* Moves one screen down */
void Page_down()
{
    if(Position_curseur+Decalage_curseur<NB_OPTIONS)
    {
	if(Position_curseur<HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)
	{ 
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SETUP);
	    Position_curseur = HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP ;
	    Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
		    COULEUR_SELECT);
	}
	else if(Decalage_curseur<NB_OPTIONS)
	{
	    if(Decalage_curseur + Position_curseur + HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP < NB_OPTIONS)
	    {
		Decalage_curseur+=HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP;
	    }
	    else Decalage_curseur=NB_OPTIONS-HAUTEUR_FIN_SETUP+HAUTEUR_DEBUT_SETUP;

	    Tout_ecrire();
	}
	Ecrire_commentaire(Position_curseur+Decalage_curseur);
    }
}

void Select()
{
  word Touche;
  Cadre(36*8+1,(HAUTEUR_DEBUT_SETUP+Position_curseur)*9,38*8-2,9,COULEUR_SETUP);
  SFont_Write(Ecran,MyFont,40*8,(HAUTEUR_DEBUT_SETUP+Position_curseur)*9,
    "-- Press a key --");
  SDL_UpdateRect(Ecran,3,(HAUTEUR_DEBUT_SETUP+Position_curseur)*9,630,9);
  while (1)
  {
  	Touche = Lire_Touche();
    if (Touche == SDLK_ESCAPE)
    {
      Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur) ,Position_curseur + Decalage_curseur,
		    COULEUR_SETUP);
		  return;
    }
    if (Touche != 0)
    {
      Config[Position_curseur+Decalage_curseur].Touche = Touche;
      Test_duplic();
      // Des X ont pu être ajoutés ou enlevés sur n'importe quelle ligne..
      // pour faire simple, on rafraîchit toute la page. 
	    Tout_ecrire();
      return;
    }
  }
}

/* Let the user do things */
void Setup()
{
    bool Sortie_OK = false;
    word Touche;
    Test_duplic();
    Tout_ecrire();
    do{
	Touche = Lire_Touche();
	switch(Touche)
	{
	    case SDLK_UP:
		Scroll_haut();
		break;
	    case SDLK_DOWN:
		Scroll_bas();
		break;
	    case SDLK_PAGEUP:
		Page_up();
		break;
	    case SDLK_PAGEDOWN:
		Page_down();
		break;
	    case SDLK_RETURN:
		Select();
		break;
	    case SDLK_DELETE:
		//Unselect();
		break;
	    case SDLK_ESCAPE:
		Sortie_OK=Validation();
		break; 
	    default:
		break; // On ne fait rien pour les autres touches
	}
    } while(!Sortie_OK);

}

/*** File management functions ***/

/* Checks if we can write the config file */
bool Verifier_ecriture_possible()
{
    return access("./",W_OK) == 0;
}

/* Save the config file */
void Enregistrer_config()
{
    FILE* Fichier;
    struct Config_Header Header;
    int Indice_chunk;

    if(Choix_enreg==true) // Save keys if wanted
    {
    	Fichier = fopen("gfx2.cfg","wb");
      
      // En-tete
      sprintf(Header.Signature,"CFG");
      Header.Version1 = VERSION1;
      Header.Version2 = VERSION2;
      Header.Beta1 = BETA1;
      Header.Beta2 = BETA2;
      write_bytes(Fichier, &Header, sizeof(Header));

      Chunk[CHUNK_TOUCHES].Taille=sizeof(struct Config_Infos_touche)*(NB_OPTIONS+1);
      Chunk[CHUNK_TOUCHES].Numero=CHUNK_TOUCHES;

      for (Indice_chunk=0; Indice_chunk<CHUNK_MAX; Indice_chunk++)
      {
        // Ecriture en-tete chunk
        write_bytes(Fichier, &(Chunk[Indice_chunk].Numero), 1);
        write_word_le(Fichier, Chunk[Indice_chunk].Taille);

        if (Indice_chunk == CHUNK_TOUCHES)
        {
          int Indice_touche;

          for(Indice_touche=0; Indice_touche<=NB_OPTIONS;Indice_touche++)
          {
      	    write_word_le(Fichier,Config[Indice_touche].Numero);
      	    write_word_le(Fichier,Config[Indice_touche].Touche);
      	    write_word_le(Fichier,Config[Indice_touche].Touche2);
          }
        }
        else
        {
          write_bytes(Fichier, ChunkData[Indice_chunk], Chunk[Indice_chunk].Taille);
        }
      }
       fclose(Fichier);   	
    }
}

/*** Main program ***/

int main(int argc, char * argv[])
{	

  if (Verifier_ecriture_possible())
  {
  	/* On initialise SDL */
  	SDL_Init(SDL_INIT_VIDEO);
  	Ecran = SDL_SetVideoMode(640,480,8,0);
  	SDL_WM_SetCaption ("Grafx2 configuration tool","../gfx2.gif");
    SDL_EnableKeyRepeat(250, 32);
    SDL_EnableUNICODE(SDL_ENABLE);

  	/* On initialise SFont */
  	MyFont = SFont_InitFont(IMG_Load("8pxfont.png"));

  	Dessiner_ecran_principal();

  	if(!Initialiser_config())
  	{
	    Setup();
	    Enregistrer_config();

	    /* On fait un peu de nettoyage avant de s'en aller */
	    SFont_FreeFont(MyFont);
	    SDL_Quit();
	    exit(0);
	  }
  	else
  	{
	    SFont_FreeFont(MyFont);
	    SDL_Quit();
	    exit(1);
	  }
  }
  else
  {
  	puts("Error: you mustn't run this setup program from a read-only drive!\n");
  	puts("The most probable cause of this error is that you are running this program");
  	puts("from a CD-Rom or a protected floppy disk.");
  	puts("You should try to copy all the files of Grafx2 on a hard drive or to");
  	puts("unprotect your floppy disk if you really want to run it from this outdated medium.");
  	exit(1);
  }

  return 0;
}
