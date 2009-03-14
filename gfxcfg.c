/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
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

#define TOUCHE_MOUSEMIDDLE     (SDLK_LAST+1)
#define TOUCHE_MOUSEWHEELUP    (SDLK_LAST+2)
#define TOUCHE_MOUSEWHEELDOWN  (SDLK_LAST+3)
#define TOUCHE_BUTTON          (SDLK_LAST+4)

#ifdef __gp2x__
  #define TOUCHE_ESC (TOUCHE_BUTTON+GP2X_BUTTON_X)
#else
  #define TOUCHE_ESC SDLK_ESCAPE
#endif

#include "struct.h"
#include "clavier.h"
#include "const.h"
#include "io.h"
#include "hotkeys.h"
#include "setup.h"

/*** Constants ***/
#define NB_MAX_TOUCHES 134
#define HAUTEUR_DEBUT_SETUP 7
#define HAUTEUR_FIN_SETUP 44
#define Header_size sizeof(Config_Header)
#define Chunk_size sizeof(Config_Chunk)

/* Colors */
#define COULEUR_SETUP 1
#define COULEUR_SELECT 8



/*** Global variables ***/
SFont_Font* MyFont;
SDL_Surface* Ecran;
char Repertoire_config[TAILLE_CHEMIN_FICHIER];
char Repertoire_donnees[TAILLE_CHEMIN_FICHIER];

bool Erreur[NB_MAX_TOUCHES];
uint8_t Choix_enreg;
uint16_t Decalage_curseur=0;
uint16_t Position_curseur=0;

byte * FichierConfig = NULL;
byte * ChunkData[CHUNK_MAX];
Config_Chunk Chunk[CHUNK_MAX];


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
    SFont_Write(Ecran, MyFont, 8,18,"Use Up/Down arrows & Page-Up/Page-Down to scroll, Enter to modify,");
    SFont_Write(Ecran, MyFont, 8,30,"Delete to remove a hot-key and Escape to validate or cancel.");

    Cadre(3,47,630,10,86);
    SFont_Write(Ecran,MyFont,8   ,48,"Option");
    SFont_Write(Ecran,MyFont,8*40,48,"Hot-Key");
    SFont_Write(Ecran,MyFont,8*75,48,"Err");

    SDL_UpdateRect(Ecran,0,0,640,480);
}

/* Displays informations about an option */
void Ecrire(uint8_t Ligne, uint16_t Num_option, uint8_t Couleur)
{
    Cadre(3,Ligne*9,630,9,Couleur);
    SFont_Write(Ecran,MyFont,8,Ligne*9,ConfigTouche[Num_option].Libelle);
    SFont_Write(Ecran,MyFont,40*8,Ligne*9,Nom_touche(ConfigTouche[Num_option].Touche));
    if(Erreur[Num_option])
        SFont_Write(Ecran,MyFont,77*8,Ligne*9,"X");
    Cadre(39*8,Ligne*9,1,9,255);
    Cadre(74*8,Ligne*9,1,9,255);
    SDL_UpdateRect(Ecran,3,Ligne*9,630,9);
}

/* Displays comment about an option */
void Ecrire_commentaire(uint16_t Num_option)
{
    Cadre(8,50*9,630,18,0);
    SFont_Write(Ecran,MyFont,8,50*9,ConfigTouche[Num_option].Explic1);
    SFont_Write(Ecran,MyFont,8,51*9,ConfigTouche[Num_option].Explic2);
    SDL_UpdateRect(Ecran,8,50*9,631,19);
}

/* Display the options list */
void Tout_ecrire()
{
    uint8_t i = HAUTEUR_DEBUT_SETUP;

    Cadre(3,64,630,340,COULEUR_SETUP);
    while(i<=HAUTEUR_FIN_SETUP && i < NB_MAX_TOUCHES + HAUTEUR_DEBUT_SETUP)
    {
        Ecrire(i,Decalage_curseur+i-HAUTEUR_DEBUT_SETUP,
                (i==HAUTEUR_DEBUT_SETUP+Position_curseur)?COULEUR_SELECT:COULEUR_SETUP);
        i++;
    }

    Cadre(39*8,46,1,360,255);
    Cadre(74*8,46,1,360,255);

    SDL_UpdateRect(Ecran,0,0,640,480);

    Ecrire_commentaire(Decalage_curseur+Position_curseur);
}

/*** Configuration handling functions ***/


/* Reads the config file */
/* returns an error message, or NULL if everything OK */
char * Interpretation_du_fichier_config()
{
  char Nom_Fichier[TAILLE_CHEMIN_FICHIER];
  FILE* Fichier;
  long int Taille_fichier;
  byte Numero_chunk;
  word Taille_chunk;
  byte * Ptr;
  int i;

  strcpy(Nom_Fichier, Repertoire_config);
  strcat(Nom_Fichier, "gfx2.cfg");
  Fichier = fopen(Nom_Fichier,"rb");
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
//    printf("%d %6X %d\n", Numero_chunk, Ptr - FichierConfig, Taille_chunk);
    Ptr+=Taille_chunk;
  }
  // Si la config contenait des touches, on les initialise:
  if (Chunk[CHUNK_TOUCHES].Taille)
  {
    unsigned int Indice_config;
    Ptr = ChunkData[CHUNK_TOUCHES];
    for (Indice_config=0; Indice_config<Chunk[CHUNK_TOUCHES].Taille / sizeof(Config_Infos_touche) ; Indice_config++)
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
      for (Indice_touche=0; Indice_touche < NB_MAX_TOUCHES; Indice_touche ++)
      {
        if (ConfigTouche[Indice_touche].Numero == Numero)
        {
          ConfigTouche[Indice_touche].Touche = Touche;

// Utilisé pour afficher la liste complète des raccourcis dans le format du wiki...
/*          printf("||%s||%s||%s %s||\n", ConfigTouche[Indice_touche].Libelle, 
                            Nom_touche(Touche),
                            ConfigTouche[Indice_touche].Explic1, 
                            ConfigTouche[Indice_touche].Explic2); */
          break;
        }
      }
      
    }
  } 
  return NULL;
}

/* Initialize configuration */
bool Initialiser_config()
{
    char * MessageErreur = NULL;

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

    Hauteur *= 8;
    Largeur *= 8;
    x1=(640 - Largeur)/2;
    y1=(480 - Hauteur)/2;

    Cadre(x1+5,y1+5,Largeur,Hauteur,0);
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

    for(i=0;i<NB_MAX_TOUCHES;i++)
    {
        if(ConfigTouche[i].Touche!=0xFF) // FIXME
        {
            j=0;
            Pas_encore_erreur=true;
            while(j<NB_MAX_TOUCHES && Pas_encore_erreur)
            {
                if(i!=j && ConfigTouche[i].Touche==ConfigTouche[j].Touche)
                {
                    Pas_encore_erreur = false;
                    Erreur[i] = true;
                }
                j++;
            }
            if (Pas_encore_erreur) Erreur[i] = false;
        }
    }
}

/* Checks if everything is OK */
bool Validation()
{
    bool Y_a_des_erreurs = false;
    uint16_t i = 0;

    while(i<NB_MAX_TOUCHES && !Y_a_des_erreurs)
    {
        Y_a_des_erreurs = Erreur[i];
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
  if(Position_curseur + Decalage_curseur < (NB_MAX_TOUCHES-1))
  {
        if(Position_curseur < HAUTEUR_FIN_SETUP - HAUTEUR_DEBUT_SETUP)
        {
            Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur) ,Position_curseur + Decalage_curseur,
                    COULEUR_SETUP);
            (Position_curseur) ++ ;
            Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur) ,Position_curseur + Decalage_curseur,
                    COULEUR_SELECT);
        }
        else if(Decalage_curseur < (NB_MAX_TOUCHES-1))
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
  if(Position_curseur+Decalage_curseur<(NB_MAX_TOUCHES-1))
  {
        if(Position_curseur<HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)
        { 
            Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
                    COULEUR_SETUP);
            Position_curseur = HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP ;
            Ecrire(HAUTEUR_DEBUT_SETUP + (Position_curseur),Position_curseur + Decalage_curseur,
                    COULEUR_SELECT);
        }
        else if(Decalage_curseur<(NB_MAX_TOUCHES-1))
        {
            if(Decalage_curseur + Position_curseur + HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP < (NB_MAX_TOUCHES-1))
            {
                  Decalage_curseur+=HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP;
          }
          else
            Decalage_curseur=NB_MAX_TOUCHES-1-HAUTEUR_FIN_SETUP+HAUTEUR_DEBUT_SETUP;

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
      ConfigTouche[Position_curseur+Decalage_curseur].Touche = Touche;
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
  #ifdef __amigaos4__
    // TODO: Make it work :)
    #warning "There is some code missing here for your platform ! please check and correct :)"
    return 1;
  #else
    // Doesn't work on OS4.
    return access(Repertoire_config,W_OK) == 0;
  #endif
}

/* Save the config file */
void Enregistrer_config()
{
    FILE* Fichier;
    Config_Header Header;
    int Indice_chunk;

    if(Choix_enreg==true) // Save keys if wanted
    {
      char Nom_Fichier[TAILLE_CHEMIN_FICHIER];
      strcpy(Nom_Fichier, Repertoire_config);
      strcat(Nom_Fichier, "gfx2.cfg");
      Fichier = fopen(Nom_Fichier,"wb");
      
      // En-tete
      sprintf(Header.Signature,"CFG");
      Header.Version1 = VERSION1;
      Header.Version2 = VERSION2;
      Header.Beta1 = BETA1;
      Header.Beta2 = BETA2;
      write_bytes(Fichier, &Header, sizeof(Header));

      Chunk[CHUNK_TOUCHES].Taille=sizeof(Config_Infos_touche)*(NB_MAX_TOUCHES);
      Chunk[CHUNK_TOUCHES].Numero=CHUNK_TOUCHES;

      for (Indice_chunk=0; Indice_chunk<CHUNK_MAX; Indice_chunk++)
      {
        // Ecriture en-tete chunk
        write_bytes(Fichier, &(Chunk[Indice_chunk].Numero), 1);
        write_word_le(Fichier, Chunk[Indice_chunk].Taille);

        if (Indice_chunk == CHUNK_TOUCHES)
        {
          int Indice_touche;

          for(Indice_touche=0; Indice_touche<NB_MAX_TOUCHES;Indice_touche++)
          {
            write_word_le(Fichier,ConfigTouche[Indice_touche].Numero);
            write_word_le(Fichier,ConfigTouche[Indice_touche].Touche);
            write_word_le(Fichier,ConfigTouche[Indice_touche].Touche2);
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

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{       
  char Repertoire_programme[TAILLE_CHEMIN_FICHIER];
  char Nom_fichier[TAILLE_CHEMIN_FICHIER];

  Set_Program_Directory(argv[0],Repertoire_programme);
  Set_Config_Directory(Repertoire_programme,Repertoire_config);
  Set_Data_Directory(Repertoire_programme,Repertoire_donnees);

  if (Verifier_ecriture_possible())
  {
        /* On initialise SDL */
        SDL_Init(SDL_INIT_VIDEO);
    {
      // Routine pour définir l'icone.
      SDL_Surface * Icone;
      byte *Masque_icone;
      strcpy(Nom_fichier, Repertoire_donnees);
      strcat(Nom_fichier, "gfx2cfg.gif");
      Icone = IMG_Load(Nom_fichier);
      if (Icone)
      {
        int x,y;
        Masque_icone=malloc(128);
        memset(Masque_icone,0,128);
        for (y=0;y<32;y++)
          for (x=0;x<32;x++)
            if (((byte *)(Icone->pixels))[(y*32+x)] != 255)
              Masque_icone[(y*32+x)/8] |=0x80>>(x&7);
        SDL_WM_SetIcon(Icone,Masque_icone);
      }
    }
    Ecran = SDL_SetVideoMode(640,480,8,0);
    SDL_WM_SetCaption ("Grafx2 configuration tool","Gfx2Cfg");
    SDL_EnableKeyRepeat(250, 32);
    SDL_EnableUNICODE(SDL_ENABLE);

        /* On initialise SFont */
    strcpy(Nom_fichier, Repertoire_donnees);
    strcat(Nom_fichier, "fonts/8pxfont.png");
    MyFont = SFont_InitFont(IMG_Load(Nom_fichier));

        if(MyFont==NULL)
        {
            SDL_Quit();
            exit(0);
        }

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
