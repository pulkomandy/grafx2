//C
#include <stdbool.h>

//POSIX
#include <unistd.h>

//SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>

//mine
#include "SFont.h"

#include "scancodes.h"

/*** Constants ***/
#define NB_MAX_OPTIONS 134
#define HAUTEUR_DEBUT_SETUP 7
#define HAUTEUR_FIN_SETUP 44

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
} Options;

typedef struct{
    char Signature[3];
    uint8_t Version1;
    uint8_t Version2;
    uint8_t Beta1;
    uint8_t Beta2;
} Type_header;

#define Header_size 7

typedef struct{
    uint8_t Numero;
    uint16_t Taille;
} Type_chunk;

#define Chunk_size 3

/*** Global variables ***/
SFont_Font* MyFont;
SDL_Surface* Ecran;
int Numero_definition_option;
Options Config[NB_MAX_OPTIONS];
uint8_t Choix_enreg;
uint16_t NB_OPTIONS;
uint16_t Decalage_curseur=0;
uint16_t Position_curseur=0;

/*** Fonctions de gestion des évènements SDL ***/

/* Attend qu'une touche soit pressée. Retourne le keysym. */
SDL_keysym Lire_Touche(void)
{
    SDL_Event Event;

    do{
        SDL_WaitEvent(&Event);
        if(Event.type==SDL_KEYDOWN) break;
    }while(1);

    return Event.key.keysym;
}

/* Writes human-readable key name to buffer Temp. Temp must be at least 35 bytes long ! */
void Nom_touche(uint16_t Touche,char* Temp)
{
    char Temp2[28];
    uint8_t Num_table =1;

    Temp[0] = Temp2[0] = 0;

    if((Touche & 0x0100) > 0)
    {
        strcat(Temp,"<Shift> + ");
        Num_table = 2;
    }

    if((Touche & 0x0200) > 0)
    {
        strcat(Temp,"<Ctrl> + ");
        Num_table = 3;
    }

    if((Touche & 0x0400) > 0)
    {
        strcat(Temp,"<Alt> + ");
        Num_table = 4;
    }

    switch(Num_table)
    {
        case 1:
            strcpy(Temp2,Table_Normal[Touche & 0xFF]);
            if (strcmp(Temp2,"???") == 0)
                strcpy(Temp,"********** Invalid key! **********");
            else if (Temp2[0]==0)
                Temp[0]=0;
            else
            {
                strcat(Temp,"<");
                strcat(Temp,Temp2);
                strcat(Temp,">");
            }
            break;
        case 2:
            strcpy(Temp2,Table_Shift[Touche & 0xFF]); 
            if (strcmp(Temp2,"???") == 0)
                strcpy(Temp,"**** Invalid key combination! ****");
            else if (Temp2[0]==0)
                Temp[0]=0;
            else
            {
                strcat(Temp,"<");
                strcat(Temp,Temp2);
                strcat(Temp,">");
            }
            break;
        case 3:
            strcpy(Temp2,Table_Ctrl[Touche & 0xFF]); 
            if (strcmp(Temp2,"???") == 0)
                strcpy(Temp,"**** Invalid key combination! ****");
            else if (Temp2[0]==0)
                Temp[0]=0;
            else
            {
                strcat(Temp,"<");
                strcat(Temp,Temp2);
                strcat(Temp,">");
            }
            break;
        case 4:
            strcpy(Temp2,Table_Alt[Touche & 0xFF]); 
            if (strcmp(Temp2,"???") == 0)
                strcpy(Temp,"**** Invalid key combination! ****");
            else if (Temp2[0]==0)
                Temp[0]=0;
            else
            {
                strcat(Temp,"<");
                strcat(Temp,Temp2);
                strcat(Temp,">");
            }
            break;
    }
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
    char NomTouche[35];
    Cadre(3,Ligne*9,630,9,Couleur);
    SFont_Write(Ecran,MyFont,8,Ligne*9,Config[Num_option].Libelle);
    Nom_touche(Config[Num_option].Touche,NomTouche);
    SFont_Write(Ecran,MyFont,40*8,Ligne*9,NomTouche);
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

/* Generate the INI file from the CFG */
bool Recreer_INI()
{
    puts("RECREER INI");
    return false;
}

/* Reads the config file */
void Interpretation_du_fichier_config()
{
    puts("INTERPRETATION FICHIER CONFIG");
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
    bool Erreur = false;
    FILE* Fichier_INI;

    Numero_definition_option = 0;

    Definir_option(0,"Scroll up",
            "Scrolls the picture up, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, 0x48); // HAUT
    Definir_option(1,"Scroll down",
            "Scrolls the picture down, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, 0x50); // BAS
    Definir_option(2,"Scroll left",
            "Scrolls the picture to the left, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, 0x4B); // GAUCHE
    Definir_option(3,"Scroll right",
            "Scrolls the picture to the right, both in magnify and normal mode.",
            "This hotkey cannot be removed.",
            false, 0x4D); // DROITE
    Definir_option(4,"Faster scroll up",
	    "Used to scroll upwards in the picture fast, either in magnify and normal",
	    "mode.",
	    true,0x0148);  /*Shift + Haut*/
    Definir_option(5,"Faster scroll down",
	    "Used to scroll downwards in the picture fast, either in magnify and normal",
	    "mode.",
	    true,0x0150);  /*Shift + Bas*/
    Definir_option(6,"Faster scroll left",
	    "Used to scroll to the left in the picture fast, either in magnify and normal",
	    "mode.",
	    true,0x014B);  /*Shift + Gauche*/
    Definir_option(7,"Faster scroll right",
	    "Used to scroll to the right in the picture fast, either in magnify and",
	    "normal mode.",
	    true,0x014D);  /*Shift + Droite*/
    Definir_option(8,"Slower scroll up",
	    "Used to scroll upwards in the picture pixel by pixel, either in magnify and",
	    "normal mode.",
	    true,0x0498);  /*Alt + Haut*/
    Definir_option(9,"Slower scroll down",
	    "Used to scroll downwards in the picture pixel by pixel, either in magnify and",
	    "normal mode.",
	    true,0x04A0);  /*Alt + Bas*/
    Definir_option(10,"Slower scroll left",
	    "Used to scroll to the left in the picture pixel by pixel, either in magnify",
	    "and normal mode.",
	    true,0x049B);  /*Alt + Gauche*/
    Definir_option(11,"Slower scroll right",
	    "Used to scroll to the right in the picture pixel by pixel, either in magnify",
	    "and normal mode.",
	    true,0x049D);  /*Alt + Droite*/
    Definir_option(12,"Move mouse cursor 1 pixel up",
	    "Used to simulate a very small mouse deplacement upwards.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x028D);  /*Ctrl + Haut*/
    Definir_option(13,"Move mouse cursor 1 pixel down",
	    "Used to simulate a very small mouse deplacement downwards.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x0291);  /*Ctrl + Bas*/
    Definir_option(14,"Move mouse cursor 1 pixel left",
	    "Used to simulate a very small mouse deplacement to the left.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x0273);  /*Ctrl + Gauche*/
    Definir_option(15,"Move mouse cursor 1 pixel right",
	    "Used to simulate a very small mouse deplacement to the right.",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x0274);  /*Ctrl + Droite*/
    Definir_option(16,"Simulate left mouse click",
	    "Used to simulate a click with the left mouse button..",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x0039);  /*Space*/
    Definir_option(17,"Simulate right mouse click",
	    "Used to simulate a click with the right mouse button..",
	    "It""s very useful when you want a ultra-high precision.",
	    true,0x0139);  /*Shift + Space*/
    Definir_option(18,"Show/hide option menu",
	    "Switch the tool bar display on/off.",
	    "This hot-key cannot be removed.",
	    false,0x0044); /*F10*/
    Definir_option(19,"Show/hide cursor",
	    "Switch the cursor display on/off.",
	    "This only works on the \"small cross\" and \"hand\" cursors.",
	    true,0x0043);  /*F9*/
    Definir_option(20,"Set paintbrush to 1 pixel",
	    "Useful when you want to use a \"single-pixel-brush\".",
	    "",
	    true,0x0053);  /*Del*/
    Definir_option(21,"Paintbrush choice",
	    "Opens a menu where you can choose a paintbrush out of 24 predefined ones.",
	    "",
	    true,0x003E);  /*F4*/
    Definir_option(22,"Monochrome brush",
	    "Turn your current user-defined brush into a single colored one.",
	    "All non-transparent colors are set to current foreground color.",
	    true,0x0157);  /*Shift + F4*/
    Definir_option(23,"Freehand drawing",
	    "Set the drawing mode to the classical freehand one.",
	    "",
	    true,0x0020);  /*D*/
    Definir_option(24,"Switch freehand drawing mode",
	    "Switch between the 3 ways to use freehand drawing.",
	    "These modes are: continuous, discontinuous and point by point.",
	    true,0x0120);  /*Shift + D*/
    Definir_option(25,"Continuous freehand drawing",
	    "Switch directly to continuous freehand drawing mode.",
	    "",
	    true,0x0220);  /*Ctrl + D*/
    Definir_option(26,"Line",
	    "Allows you to draw lines.",
	    "",
	    true,0x0026);  /*L*/
    Definir_option(27,"Knotted lines",
	    "Allows you to draw linked lines.",
	    "This mode can also be called \"Polyline\".",
	    true,0x0126);  /*Shift + L*/
    Definir_option(28,"Spray",
	    "Allows you to spray brushes randomly in the picture.",
	    "",
	    true,0x001E);  /*A (Q en AZERTY)*/
    Definir_option(29,"Spray menu",
	    "Opens a menu in which you can configure the spray flow and size.",
	    "",
	    true,0x011E);  /*Shift + A*/
    Definir_option(30,"Flood-fill",
	    "Allows you to fill an area of the picture made of pixels of the same color.",
	    "",
	    true,0x0021);  /*F*/
    Definir_option(124,"Replace color",
	    "Allows you to replace all the pixels of the color pointed by the mouse with",
	    "the fore-color or the back-color.",
	    true,0x0121);  /*Shift + F*/
    Definir_option(31,"B‚zier""s curves",
	    "Allows you to draw B‚zier""s curves.",
	    "",
	    true,0x0017);  /*I*/
    Definir_option(32,"B‚zier""s curve with 3 or 4 points",
	    "Allows you to choose whether you want to draw B‚zier""s curves with 3 or 4",
	    "points.",
	    true,0x0117);  /*Shift + I*/
    Definir_option(33,"Empty rectangle",
	    "Allows you to draw a rectangle using the brush.",
	    "",
	    true,0x0013);  /*R*/
    Definir_option(34,"Filled rectangle",
	    "Allows you to draw a filled rectangle.",
	    "",
	    true,0x0113);  /*Shift + R*/
    Definir_option(35,"Empty circle",
	    "Allows you to draw a circle using the brush.",
	    "",
	    true,0x002E);  /*C*/
    Definir_option(36,"Empty ellipse",
	    "Allows you to draw an ellipse using the brush.",
	    "",
	    true,0x022E);  /*Ctrl + C*/
    Definir_option(37,"Filled circle",
	    "Allows you to draw a filled circle.",
	    "",
	    true,0x012E);  /*Shift + C*/
    Definir_option(38,"Filled ellipse",
	    "Allows you to draw a filled ellipse.",
	    "",
	    true,0x032E);  /*Shift + Ctrl + C*/
    Definir_option(39,"Empty polygon",
	    "Allows you to draw a polygon using the brush.",
	    "",
	    true,0x0031);  /*N*/
    Definir_option(40,"Empty \"polyform\"",
	    "Allows you to draw a freehand polygon using the brush.",
	    "",
	    true,0x0231);  /*Ctrl + N*/
    Definir_option(41,"Filled polygon",
	    "Allows you to draw a filled polygon.",
	    "",
	    true,0x0131);  /*Shift + N*/
    Definir_option(42,"Filled \"polyform\"",
	    "Allows you to draw a filled freehand polygon.",
	    "",
	    true,0x0331);  /*Shift + Ctrl + N*/
    Definir_option(43,"Rectangle with gradation",
	    "Allows you to draw a rectangle with a color gradation.",
	    "",
	    true,0x0413);  /*Alt + R*/
    Definir_option(44,"Gradation menu",
	    "Allows you to configure the way color gradations are calculated.",
	    "",
	    true,0x0422);  /*Alt + G*/
    Definir_option(45,"Sphere with gradation",
	    "Allows you to draw a rectangle with a color gradation.",
	    "",
	    true,0x042E);  /*Alt + C*/
    Definir_option(46,"Ellipse with gradation",
	    "Allows you to draw an ellipse filled with a color gradation.",
	    "",
	    true,0x052E);  /*Shift + Alt + C*/
    Definir_option(47,"Adjust picture",
	    "Allows you to move the whole picture in order to re-center it.",
	    "Notice that what gets out from a side reappears on the other.",
	    true,0x004C);  /*Kpad5*/
    Definir_option(48,"Flip/shrink picture menu",
	    "Opens a menu which allows you to flip the picture horizontally/vertically or",
	    "to shrink it to half-scale horizontally and/or vertically.",
	    true,0x014C);  /*Shift + Kpad5*/
    Definir_option(49,"Drawing effects",
	    "Opens a menu where you can enable/disable and configure the drawing effects",
	    "listed below.",
	    true,0x0012);  /*E*/
    Definir_option(50,"Shade mode",
	    "Allows you to shade or lighten some pixels of the picture belonging to a",
	    "color range, in addition of any drawing tool.",
	    true,0x003F);  /*F5*/
    Definir_option(51,"Shade menu",
	    "Opens a menu where you can choose color ranges to use with the Shade mode.",
	    "This menu also contains parameters used both in Shade and Quick-shade modes.",
	    true,0x0158);  /*Shift + F5*/
    Definir_option(131,"Quick-shade mode",
	    "Does the same thing as shade mode with a simpler method (faster to define",
	    "but a bit less powerful).",
	    true,0x0262);  /*Ctrl + F5*/
    Definir_option(132,"Quick-shade menu",
	    "Opens a menu where you can define the parameters of the quick-shade mode.",
	    "",
	    true,0x0362);  /*Shift + Ctrl + F5*/
    Definir_option(52,"Stencil mode",
	    "Allows you to mask colors that must not be affected when you are drawing.",
	    "",
	    true,0x0040);  /*F6*/
    Definir_option(53,"Stencil menu",
	    "Opens a menu where you can choose colors masked by the Stencil mode.",
	    "",
	    true,0x0159);  /*Shift + F6*/
    Definir_option(54,"Mask mode",
	    "Allows you to mask colors of the spare page that will keep you from ",
	    "drawing. This mode should be called \"True stencil\".",
	    true,0x046D);  /*Alt + F6*/
    Definir_option(55,"Mask menu",
	    "Opens a menu where you can choose colors for the Mask mode.",
	    "",
	    true,0x056D);  /*Shift + Alt + F6*/
    Definir_option(56,"Grid mode",
	    "Force the cursor to snap up grid points.",
	    "",
	    true,0x0022);  /*G*/
    Definir_option(57,"Grid menu",
	    "Open a menu where you can configure the grid used by Grid mode.",
	    "",
	    true,0x0122);  /*Shift + G*/
    Definir_option(58,"Sieve mode",
	    "Only draws pixels on certain positions matching with a sieve.",
	    "",
	    true,0x0222);  /*Ctrl + G*/
    Definir_option(59,"Sieve menu",
	    "Opens a menu where you can configure the sieve.",
	    "",
	    true,0x0322);  /*Shift + Ctrl + G*/
    Definir_option(60,"Invert sieve",
	    "Inverts the pattern defined in the Sieve menu.",
	    "",
	    true,0x0622);  /*Ctrl + Alt + G*/
    Definir_option(61,"Colorize mode",
	    "Allows you to colorize the pixels on which your brush is pasted.",
	    "This permits you to make transparency effects.",
	    true,0x0041);  /*F7*/
    Definir_option(62,"Colorize menu",
	    "Opens a menu where you can give the opacity percentage for Colorize mode.",
	    "",
	    true,0x015A);  /*Shift + F7*/
    Definir_option(63,"Smooth mode",
	    "Soften pixels on which your brush is pasted.",
	    "",
	    true,0x0042);  /*F8*/
    Definir_option(123,"Smooth menu",
	    "Opens a menu where you can define the Smooth matrix.",
	    "",
	    true,0x015B);  /*Shift + F8*/
    Definir_option(64,"Smear mode",
	    "Smears the pixels when you move your brush on the picture.",
	    "",
	    true,0x046F);  /*Alt + F8*/
    Definir_option(65,"Tiling mode",
	    "Puts parts of the brush where you draw.",
	    "",
	    true,0x0430);  /*Alt + B*/
    Definir_option(66,"Tiling menu",
	    "Opens a menu where you can configure the origin of the tiling.",
	    "",
	    true,0x0530);  /*Shift + Alt + B*/
    Definir_option(67,"Classical brush grabbing",
	    "Allows you to pick a brush defined within a rectangle.",
	    "",
	    true,0x0030);  /*B*/
    Definir_option(68,"\"Lasso\" brush grabbing",
	    "Allows you to pick a brush defined within a freehand polygon.",
	    "",
	    true,0x0230);  /*Ctrl + B*/
    Definir_option(69,"Get previous brush back",
	    "Restore the last user-defined brush.",
	    "",
	    true,0x0130);  /*Shift + B*/
    Definir_option(70,"Horizontal brush flipping",
	    "Reverse brush horizontally.",
	    "",
	    true,0x002D);  /*X*/
    Definir_option(71,"Vertical brush flipping",
	    "Reverse brush vertically.",
	    "",
	    true,0x0015);  /*Y*/
    Definir_option(72,"90ø brush rotation",
	    "Rotate the user-defined brush by 90ø (counter-clockwise).",
	    "",
	    true,0x002C);  /*Z (W en AZERTY)*/
    Definir_option(73,"180ø brush rotation",
	    "Rotate the user-defined brush by 180ø.",
	    "",
	    true,0x012C);  /*Shift + Z*/
    Definir_option(74,"Strech brush",
	    "Allows you to resize the user-defined brush.",
	    "",
	    true,0x001F);  /*S*/
    Definir_option(75,"Distort brush",
	    "Allows you to distort the user-defined brush.",
	    "",
	    true,0x011F);  /*Shift + S*/
    Definir_option(76,"Outline brush",
	    "Outlines the user-defined brush with the fore color.",
	    "",
	    true,0x0018);  /*O*/
    Definir_option(77,"Nibble brush",
	    "Deletes the borders of the user-defined brush.",
	    "This does the opposite of the Outline option.",
	    true,0x0118);  /*Shift + O*/
    Definir_option(78,"Get colors from brush",
	    "Copy colors of the spare page that are used in the brush.",
	    "",
	    true,0x0085);  /*F11*/
    Definir_option(79,"Recolorize brush",
	    "Recolorize pixels of the user-defined brush in order to get a brush which",
	    "looks like the one grabbed in the spare page.",
	    true,0x0086);  /*F12*/
    Definir_option(80,"Rotate by any angle",
	    "Rotate the brush by an angle that you can define.",
	    "",
	    true,0x0011);  /*W (Z en AZERTY)*/
    Definir_option(81,"Pipette",
	    "Allows you to copy the color of a pixel in the picture into the foreground",
	    "or background color.",
	    true,0x0029);  /*`~ (Touche sous le Esc - ý en AZERTY)*/
    Definir_option(82,"Swap foreground/background colors",
	    "Invert foreground and background colors.",
	    "",
	    true,0x0129);  /*Shift + `~*/
    Definir_option(83,"Magnifier mode",
	    "Allows you to zoom into the picture.",
	    "",
	    true,0x0032);  /*M (,? sur AZERTY)*/
    Definir_option(84,"Zoom factor menu",
	    "Opens a menu where you can choose a magnifying factor.",
	    "",
	    true,0x0132);  /*Shift + M*/
    Definir_option(85,"Zoom in",
	    "Increase magnifying factor.",
	    "",
	    true,0x004E);  /*Grey +*/
    Definir_option(86,"Zoom out",
	    "Decrease magnifying factor.",
	    "",
	    true,0x004A);  /*Grey -*/
    Definir_option(87,"Brush effects menu",
	    "Opens a menu which proposes different effects on the user-defined brush.",
	    "",
	    true,0x0630);  /*Ctrl + Alt + B*/
    Definir_option(88,"Text",
	    "Opens a menu which permits you to type in a character string and to choose a",
	    "font, and then creates a new user-defined brush fitting to your choices.",
	    true,0x0014);  /*T*/
    Definir_option(89,"Screen resolution menu",
	    "Opens a menu where you can choose the dimensions of the screen in which you",
	    "want to draw among the numerous X and SVGA proposed modes.",
	    true,0x001C);  /*Enter*/
    Definir_option(90,"\"Safety\" resolution",
	    "Set resolution to 320x200. This can be useful if you choosed a resolution",
	    "that is not supported by your monitor and video card. Cannot be removed.",
	    false,0x011C); /*Shift + Enter*/
    Definir_option(91,"Help and credits",
	    "Opens a window where you can get information about the program.",
	    "",
	    true,0x003B);  /*F1*/
    Definir_option(92,"Statistics",
	    "Displays miscellaneous more or less useful information.",
	    "",
	    true,0x0154);  /*Shift + F1*/
    Definir_option(93,"Jump to spare page",
	    "Swap current page and spare page.",
	    "",
	    true,0x000F);  /*Tab*/
    Definir_option(94,"Copy current page to spare page",
	    "Copy current page to spare page.",
	    "",
	    true,0x010F);  /*Shift + Tab*/
    Definir_option(95,"Save picture as...",
	    "Opens a file-selector that allows you to save your picture with a new",
	    "path-name.",
	    true,0x003C);  /*F2*/
    Definir_option(96,"Save picture",
	    "Saves your picture with the last name you gave it.",
	    "",
	    true,0x0155);  /*Shift + F2*/
    Definir_option(97,"Load picture",
	    "Opens a file-selector that allows you to load a new picture.",
	    "",
	    true,0x003D);  /*F3*/
    Definir_option(98,"Re-load picture",
	    "Re-load the current picture.",
	    "This allows you to cancel modifications made since last saving.",
	    true,0x0156);  /*Shift + F3*/
    Definir_option(99,"Save brush",
	    "Opens a file-selector that allows you to save your current user-defined",
	    "brush.",
	    true,0x025F);  /*Ctrl + F2*/
    Definir_option(100,"Load brush",
	    "Opens a file-selector that allows you to load a brush.",
	    "",
	    true,0x0260);  /*Ctrl + F3*/
    Definir_option(101,"Settings",
	    "Opens a menu which permits you to set the dimension of your picture, and to",
	    "modify some parameters of the program.",
	    true,0x015D);  /*Shift + F10*/
    Definir_option(102,"Undo (Oops!)",
	    "Cancel the last action which modified the picture. This has no effect after",
	    "a jump to the spare page, loading a picture or modifying its size.",
	    true,0x0016);  /*U*/
    Definir_option(103,"Redo",
	    "Redo the last undone action. This has no effect after a jump to the spare",
	    "page, loading a picture or modifying its size.",
	    true,0x0116);  /*Shift + U*/
    Definir_option(133,"Kill",
	    "Kills the current page. It actually removes the current page from the list",
	    "of \"Undo\" pages.",
	    true,0x0153);  /*Shift + Suppr*/
    Definir_option(104,"Clear page",
	    "Clears the picture with the first color of the palette (usually black).",
	    "",
	    true,0x000E);  /*BackSpace*/
    Definir_option(105,"Clear page with backcolor",
	    "Clears the picture with the backcolor.",
	    "",
	    true,0x010E);  /*Shift + BackSpace*/
    Definir_option(106,"Quit program",
	    "Allows you to leave the program.",
	    "If modifications were not saved, confirmation is asked.",
	    false,0x0010);  /*Q (A en AZERTY)*/
    Definir_option(107,"Palette menu",
	    "Opens a menu which allows you to modify the current palette.",
	    "",
	    true,0x0019);  /*P*/
    Definir_option(125,"Secondary palette menu",
	    "Opens a menu which allows you to define color series and some tagged colors.",
	    "",
	    true,0x0119);  /*Shift + P*/
    Definir_option(130,"Exclude colors menu",
	    "Opens a menu which allows you to define the colors you don""t want to use in",
	    "modes such as Smooth and Transparency, or when remapping a brush.",
	    true,0x0219);  /*Ctrl + P*/
    Definir_option(108,"Scroll palette to the left",
	    "Scroll palette in the tool bar to the left, column by column.",
	    "",
	    true,0x0049);  /*PgUp*/
    Definir_option(109,"Scroll palette to the right",
	    "Scroll palette in the tool bar to the right, column by column.",
	    "",
	    true,0x0051);  /*PgDn*/
    Definir_option(110,"Scroll palette to the left faster",
	    "Scroll palette in the tool bar to the left, 8 columns by 8 columns.",
	    "",
	    true,0x0149);  /*Shift + PgUp*/
    Definir_option(111,"Scroll palette to the right faster",
	    "Scroll palette in the tool bar to the right, 8 columns by 8 columns.",
	    "",
	    true,0x0151);  /*Shift + PgDn*/
    Definir_option(112,"Center brush attachment point",
	    "Set the attachement of the user-defined brush to its center.",
	    "",
	    true,0x028F);  /*Ctrl + 5 (pav‚ num‚rique)*/
    Definir_option(113,"Top-left brush attachment point",
	    "Set the attachement of the user-defined brush to its top-left corner.",
	    "",
	    true,0x0277);  /*Ctrl + 7*/
    Definir_option(114,"Top-right brush attachment point",
	    "Set the attachement of the user-defined brush to its top-right corner.",
	    "",
	    true,0x0284);  /*Ctrl + 9*/
    Definir_option(115,"Bottom-left brush attachment point",
	    "Set the attachement of the user-defined brush to its bottom-left corner.",
	    "",
	    true,0x0275);  /*Ctrl + 1*/
    Definir_option(116,"Bottom-right brush attachment point",
	    "Set the attachement of the user-defined brush to its bottom-right corner.",
	    "",
	    true,0x0276);  /*Ctrl + 3*/
    Definir_option(117,"Next foreground color",
	    "Set the foreground color to the next in the palette.",
	    "",
	    true,0x001B);  /*] (0x en AZERTY)*/
    Definir_option(118,"Previous foreground color",
	    "Set the foreground color to the previous in the palette.",
	    "",
	    true,0x001A);  /*[ (^ en AZERTY)*/
    Definir_option(119,"Next background color",
	    "Set the background color to the next in the palette.",
	    "",
	    true,0x011B);  /*Shift + ]*/
    Definir_option(120,"Previous background color",
	    "Set the background color to the previous in the palette.",
	    "",
	    true,0x011A);  /*Shift + [*/
    Definir_option(126,"Next user-defined forecolor",
	    "Set the foreground color to the next in the user-defined color series.",
	    "",
	    true,0x000D);  /*"=+"*/
    Definir_option(127,"Previous user-defined forecolor",
	    "Set the foreground color to the previous in the user-defined color series.",
	    "",
	    true,0x000C);  /*"-_" (")ø" en AZERTY*/
    Definir_option(128,"Next user-defined backcolor",
	    "Set the background color to the next in the user-defined color series.",
	    "",
	    true,0x010D);  /*Shift + "=+"*/
    Definir_option(129,"Previous user-defined backcolor",
	    "Set the background color to the previous in the user-defined color series.",
	    "",
	    true,0x010C);  /*Shift + "-_" (")ø" en AZERTY*/
    Definir_option(121,"Shrink paintbrush",
	    "Decrease the width of the paintbrush if it is special circle or square.",
	    "",
	    true,0x0033);  /*,< (;. en AZERTY)*/
    Definir_option(122,"Enlarge paintbrush",
	    "Increase the width of the paintbrush if it is special circle or square.",
	    "",
	    true,0x0034);  /*.> (:/ en AZERTY)*/


    NB_OPTIONS = Numero_definition_option - 1;

    Fichier_INI = fopen("gfx2.ini","r");
    if(Fichier_INI == NULL)
	Erreur=Recreer_INI();
    else
	fclose(Fichier_INI);

    if(Erreur==0) Interpretation_du_fichier_config();

    return Erreur;
}

uint8_t Fenetre_choix(int Largeur, int Hauteur, const char* Titre, const char* Choix, uint8_t Choix_debut,
	uint8_t Couleur,uint8_t Couleur_choix)
{
    char Temp[70];
    uint8_t i,j,Num_titre,Num_choix;
    uint16_t x1,y1;
    uint8_t Option_choisie;
    SDL_keysym Touche;

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
	switch(Touche.sym)
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
    }while(Touche.sym!=SDLK_RETURN);

    Tout_ecrire(); // Efface la boite de dialogue
    return Option_choisie+1;
}

/* Checks if some key is used twice */
void Test_duplic()
{
    uint16_t i,j;
    bool Pas_encore_erreur;

    for(i=0;i<NB_OPTIONS;i++)
    {
	if(Config[i].Touche!=0xFF)
	{
	    j=1;
	    Pas_encore_erreur=true;
	    while(j<NB_OPTIONS && Pas_encore_erreur)
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

    while(i<NB_OPTIONS && !Y_a_des_erreurs)
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
    printf("%d %d \n",Position_curseur, Decalage_curseur);
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
    printf("%d %d \n",Position_curseur, Decalage_curseur);
}

/* Let the user do things */
void Setup()
{
    bool Sortie_OK = false;
    SDL_keysym Touche;
    Test_duplic();
    Tout_ecrire();
    do{
	Touche = Lire_Touche();
	switch(Touche.sym)
	{
	    case SDLK_UP:
		Scroll_haut();
		break;
	    case SDLK_DOWN:
		Scroll_bas();
		break;
	    case SDLK_PAGEUP:
	    case SDLK_PAGEDOWN:
	    case SDLK_RETURN:
	    case SDLK_DELETE:
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
    uint16_t Indice;

    if(Choix_enreg==true) // Save keys if wanted
    {
	Fichier = fopen("gfx2.cfg","wb");
	fseek(Fichier,Header_size+Chunk_size,SEEK_SET); // Positionnement sur la première touche

	for(Indice = 0;Indice < NB_OPTIONS;Indice++)
	{
	    fwrite(&Config[Indice].Numero,sizeof(uint16_t),1,Fichier);
	    fwrite(&Config[Indice].Touche,sizeof(uint16_t),1,Fichier);
	    fwrite(&Config[Indice].Touche2,sizeof(uint16_t),1,Fichier);
	}

	fclose(Fichier);
    }
}

/*** Main program ***/

int main(void)
{	

    if (Verifier_ecriture_possible())
    {
	/* On initialise SDL */
	SDL_Init(SDL_INIT_VIDEO);
	Ecran = SDL_SetVideoMode(640,480,8,0);
	SDL_WM_SetCaption ("Grafx2 configuration tool","../gfx2.gif");

	/* On initialise SFont */
	MyFont = SFont_InitFont(IMG_Load("5pxtinyfont.png"));

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
	    puts("Error reading GFX2.DAT! File is absent or corrupt.");
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
