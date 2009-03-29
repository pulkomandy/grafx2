/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
#ifndef _CONST_H_
#define _CONST_H_



// Déclaration des constantes ////////////////////////////////////////////////

#define M_2PI 6.28318530717958647692528676656

#define PERCENTAGE_VERSION     "98.0%" // Libellé du pourcentage de la version ß
#define VERSION1                  2     // |
#define VERSION2                  0     // |_ Numéro de version découpé en
#define BETA1                     98    // |  plusieurs parties => 2.0 ß95.5%
#define BETA2                     0     // |  (utilisé pour le fichier de config)
#define ALPHA_BETA                "ß"   // Type de la version "Þ" ou "ß"
#define MAX_VIDEO_MODES           100   // Nombre de modes vidéo maxi
#define NB_BUTTONS                38    // Nombre de boutons à gérer
#define NB_SHORTCUTS              134   // Nombre de combinaisons de touches
#define NB_SPECIAL_SHORTCUTS      72    // Nombre de touches spéciales
#define NB_OPERATIONS             33    // Nombre d'opérations gérées par le moteur
#define NB_ZOOM_FACTORS           12    // Nombre de facteurs de zoom
#define MENU_WIDTH                254   // Largeur du menu en taille réelle
#define MENU_HEIGHT               44    // Hauteur du menu en taille réelle
#define NB_CURSOR_SPRITES         8     // Nombre de sprites de curseur
#define CURSOR_SPRITE_WIDTH       15    // Largeur d'un sprite de curseur en pixels
#define CURSOR_SPRITE_HEIGHT      15    // Hauteur d'un sprite de curseur en pixels
#define NB_EFFECTS_SPRITES        9     // Nombre de sprites d'effets
#define NB_MENU_SPRITES           20    // Nombre de sprites de menu
#define MENU_SPRITE_WIDTH         14    // Largeur d'un sprite de menu en pixels
#define MENU_SPRITE_HEIGHT        14    // Hauteur d'un sprite de menu en pixels
#define PAINTBRUSH_WIDTH          16    // Largeur d'un sprite de pinceau prédéfini
#define PAINTBRUSH_HEIGHT         16    // Hauteur d'un sprite de pinceau prédéfini
#define MAX_PAINTBRUSH_SIZE       127   // Taille maxi des pinceaux
#define NB_ICON_SPRITES           7     // Nombre de sprites de drives
#define ICON_SPRITE_WIDTH         8     // Largeur d'un sprite de drive en pixels
#define ICON_SPRITE_HEIGHT        8     // Hauteur d'un sprite de drive en pixels
#define NB_PAINTBRUSH_SPRITES     48    // Nombre de sprites de pinceau
#define NB_PRESET_SIEVE           12    // Nombre de trames prédéfinies
#define OPERATION_STACK_SIZE      16    // Nombre maximum d'éléments utilisés par les opérations
#define MAX_DISPLAYABLE_PATH      37    // Taille maximum affichable du répertoire courant dans les fenêtres du fileselect
#define COMMENT_SIZE              32    // Taille maxi des commentaires pour le PKM
#define NB_MAX_PAGES_UNDO         99    // Nombre maximal de pages d'undo
#define DEFAULT_ZOOM_FACTOR        4    // Facteur de zoom initial
#define MAX_PATH_CHARACTERS      260    // Le nombre de caractères maxi pour un nom de fichier avec chemin complet
#define NB_BOOKMARKS               4    // Nombre de boutons "signet" dans l'ecran Save/Load
// Caractères présents dans les deux fontes
#define RIGHT_TRIANGLE_CHARACTER  16
#define LEFT_TRIANGLE_CHARACTER   17
#define ELLIPSIS_CHARACTER       '…'

// On impose à l'allocation dynamique des pages de backups de conserver un
// minimum de 256 Ko pour que le reste du programme puisse continuer à
// fonctionner.
#define MINIMAL_MEMORY_TO_RESERVE (256*1024)

#define LEFT_SIDE                    1     // Indique une direction (ou click) à gauche
#define RIGHT_SIDE                   2     // Indique une direction (ou click) à droite

#define SEPARATOR_WIDTH              6     // [ Û±±° ]
#define INITIAL_SEPARATOR_PROPORTION 0.3   // proportion de la zone non-zoomée en largeur par rapport à l'écran
#define NB_ZOOMED_PIXELS_MIN         4     // Nombre minimal de pixels zoomés en largeur (Note: En dessous de 4, on ne peut plus scroller!)

#if defined(__MORPHOS__) || defined(__amigaos4__) || defined(__amigaos__)
   #define PARENT_DIR "/"
#else
   #define PARENT_DIR ".."
#endif

  // Les différents formats de fichiers:

#ifndef __no_pnglib__
#define NB_KNOWN_FORMATS         13    // Nombre de formats connus (devrait être la valeur maximale de NB_FORMATS_LOAD et NB_FORMATS_SAVE, mais plus généralement: Card({NB_FORMATS_LOAD} UNION {NB_FORMATS_SAVE}))
#define NB_FORMATS_LOAD          13    // Nombre de formats que l'on sait charger
#define NB_FORMATS_SAVE          13    // Nombre de formats que l'on sait sauver
#else
// Without pnglib
#define NB_KNOWN_FORMATS         12    // Nombre de formats connus (devrait être la valeur maximale de NB_FORMATS_LOAD et NB_FORMATS_SAVE, mais plus généralement: Card({NB_FORMATS_LOAD} UNION {NB_FORMATS_SAVE}))
#define NB_FORMATS_LOAD          12    // Nombre de formats que l'on sait charger
#define NB_FORMATS_SAVE          12    // Nombre de formats que l'on sait sauver
#endif

enum FILE_FORMATS
{
  FORMAT_PKM=1,      // |
  FORMAT_LBM,        // |
  FORMAT_GIF,        // |    Il faudra penser à réordonner
  FORMAT_BMP,        // |  les données sur les formats dans
  FORMAT_PCX,        // |  GLOBAL.H si on modifie ces cons-
  FORMAT_IMG,        // |_ tantes.
  FORMAT_SCx,        // |
  FORMAT_PI1,        // |    Elles représentent l'indice où
  FORMAT_PC1,        // |  l'on doit aller piocher ces
  FORMAT_CEL,        // |  données.
  FORMAT_KCF,        // |
  FORMAT_PAL,        // |
  FORMAT_PNG         // |
};

#define DEFAULT_FILEFORMAT    FORMAT_GIF // Format par défaut (ah bon? oh!)

  // Les différentes erreurs:

enum ERROR_CODES
{
  // 0 = Flash rouge de l'écran, erreur non critique
  ERROR_GUI_MISSING=1,          // Le fichier gfx2gui.gif est absent
  ERROR_GUI_CORRUPTED,          // Mauvais fichier gfx2gui.gif
  ERROR_INI_MISSING,            // Le fichier gfx2def.ini est absent
  ERROR_CFG_MISSING,            // Le fichier GFX2.CFG est absent
  ERROR_CFG_CORRUPTED,          // Mauvais fichier GFX2.CFG
  ERROR_CFG_OLD,            // Ancienne version du fichier GFX2.CFG
  ERROR_MEMORY,               // Plus de mémoire
  ERROR_COMMAND_LINE,        // Error sur la ligne de commande
  ERROR_MOUSE_DRIVER,         // Pas de driver souris installé
  ERROR_FORBIDDEN_MODE,         // Mode demandé sur la ligne de commande interdit (coché en noir)
  ERROR_SAVING_CFG,        // Error en écriture pour GFX2.CFG
  ERROR_MISSING_DIRECTORY,    // Le répertoire de lancement n'existe plus
  ERROR_INI_CORRUPTED,          // Le fichier GFX2.INI est corrompu
  ERROR_SAVING_INI,        // Le fichier GFX2.INI n'est pas inscriptible
  ERROR_SORRY_SORRY_SORRY      // On le refera plus, promis (erreur d'allocation de page qui ne devrait JAMAIS se produire)
};

  // Les différents types de pixels

enum PIXEL_RATIO
{
    PIXEL_SIMPLE,
    PIXEL_WIDE,
    PIXEL_TALL,
    PIXEL_DOUBLE
};

  // Les différentes catégories de bouton:

enum FAMILY_OF_BUTTONS
{
  FAMILY_TOOL=1,          // Outils de dessin (exemple : Freehand draw)
  FAMILY_INTERRUPTION,     // Opération éphémère (exemple : changement de brosse) > Interruption de l'opération courante pour faire autre chose, puis on revient
  FAMILY_INSTANT,       // Pif paf (exemple : changement de couleur) > ça sera fini dès qu'on lache le bouton, pas d'utilisation de la pile d'opérations
  FAMILY_TOOLBAR,          // Cache/Montre la barre d'outils
  FAMILY_EFFECTS            // Effets
};

  // Les différentes formes de bouton:

enum BUTTON_SHAPES
{
  BUTTON_SHAPE_NO_FRAME,            // Ex: la palette
  BUTTON_SHAPE_RECTANGLE,             // Ex: la plupart
  BUTTON_SHAPE_TRIANGLE_TOP_LEFT,  // Ex: Rectangles vides
  BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT    // Ex: Rectangles pleins
};

  // Les différentes formes de curseur:

enum CURSOR_SHAPES
{
  CURSOR_SHAPE_ARROW,
  CURSOR_SHAPE_TARGET,              // Utilise le pinceau
  CURSOR_SHAPE_COLORPICKER,      // Utilise le pinceau
  CURSOR_SHAPE_HOURGLASS,
  CURSOR_SHAPE_MULTIDIRECTIONNAL,
  CURSOR_SHAPE_HORIZONTAL,
  CURSOR_SHAPE_THIN_TARGET,         // Utilise le pinceau
  CURSOR_SHAPE_THIN_COLORPICKER, // Utilise le pinceau
  CURSOR_SHAPE_XOR_TARGET,
  CURSOR_SHAPE_XOR_RECTANGLE,
  CURSOR_SHAPE_XOR_ROTATION
};

  // Les différentes formes de pinceaux (les types de pinceaux doivent être au début)

enum PAINTBRUSH_SHAPES
{
  PAINTBRUSH_SHAPE_ROUND,
  PAINTBRUSH_SHAPE_SQUARE,
  PAINTBRUSH_SHAPE_HORIZONTAL_BAR,
  PAINTBRUSH_SHAPE_VERTICAL_BAR,
  PAINTBRUSH_SHAPE_SLASH,
  PAINTBRUSH_SHAPE_ANTISLASH,
  PAINTBRUSH_SHAPE_RANDOM,
  PAINTBRUSH_SHAPE_CROSS,
  PAINTBRUSH_SHAPE_PLUS,
  PAINTBRUSH_SHAPE_DIAMOND,
  PAINTBRUSH_SHAPE_SIEVE_ROUND,
  PAINTBRUSH_SHAPE_SIEVE_SQUARE,
  PAINTBRUSH_SHAPE_MISC,// Ce doit être le dernier des types de pinceaux, comme ça il indique le nombre de types de pinceaux (-1)
  PAINTBRUSH_SHAPE_POINT, // Utilisé pour réduire de pinceau à 1 point dans certaines opérations
  PAINTBRUSH_SHAPE_COLOR_BRUSH,
  PAINTBRUSH_SHAPE_MONO_BRUSH
};

  // Les différents états de bouton:

#define BUTTON_RELEASED 0
#define BUTTON_PRESSED 1

  // Les différents modes de Shade

enum SHADE_MODES
{
  SHADE_MODE_NORMAL,
  SHADE_MODE_LOOP,
  SHADE_MODE_NOSAT
};

  // Les différents chunks du fichier .CFG

enum CHUNKS_CFG
{
  CHUNK_KEYS            = 0,
  CHUNK_VIDEO_MODES        = 1,
  CHUNK_SHADE              = 2,
  CHUNK_MASK             = 3,
  CHUNK_STENCIL            = 4,
  CHUNK_GRADIENTS           = 5,
  CHUNK_SMOOTH             = 6,
  CHUNK_EXCLUDE_COLORS     = 7,
  CHUNK_QUICK_SHADE        = 8,
  CHUNK_GRID             = 9,
  CHUNK_MAX 
};

  // Les différents types de lecteurs:

enum ICON_TYPES
{
  ICON_FLOPPY_3_5,   // 0: Diskette 3"«
  ICON_FLOPPY_5_25,  // 1: Diskette 5"¬
  ICON_HDD,          // 2: HDD
  ICON_CDROM,        // 3: CD-ROM
  ICON_NETWORK,      // 4: Logique (réseau?)
};

  // Les différents boutons:

enum BUTTON_NUMBERS
{
  BUTTON_PAINTBRUSHES,
  BUTTON_ADJUST,
  BUTTON_DRAW,
  BUTTON_CURVES,
  BUTTON_LINES,
  BUTTON_AIRBRUSH,
  BUTTON_FLOODFILL,
  BUTTON_POLYGONS,
  BUTTON_POLYFILL,
  BUTTON_RECTANGLES,
  BUTTON_FILLRECT,
  BUTTON_CIRCLES,
  BUTTON_FILLCIRC,
  BUTTON_GRADRECT,
  BUTTON_GRADMENU,
  BUTTON_SPHERES,
  BUTTON_BRUSH,
  BUTTON_POLYBRUSH,
  BUTTON_BRUSH_EFFECTS,
  BUTTON_EFFECTS,
  BUTTON_TEXT,
  BUTTON_MAGNIFIER,
  BUTTON_COLORPICKER,
  BUTTON_RESOL,
  BUTTON_PAGE,
  BUTTON_SAVE,
  BUTTON_LOAD,
  BUTTON_SETTINGS,
  BUTTON_CLEAR,
  BUTTON_HELP,
  BUTTON_UNDO,
  BUTTON_KILL,
  BUTTON_QUIT,
  BUTTON_PALETTE,
  BUTTON_PAL_LEFT,
  BUTTON_PAL_RIGHT,
  BUTTON_CHOOSE_COL,
  BUTTON_HIDE
};

  // Les actions des touches spéciales

enum SPECIAL_ACTIONS
{
  SPECIAL_MOUSE_UP,                
  SPECIAL_MOUSE_DOWN,              
  SPECIAL_MOUSE_LEFT,              
  SPECIAL_MOUSE_RIGHT,             
  SPECIAL_CLICK_LEFT,               
  SPECIAL_CLICK_RIGHT,             
  SPECIAL_NEXT_FORECOLOR,         
  SPECIAL_PREVIOUS_FORECOLOR,      
  SPECIAL_NEXT_BACKCOLOR,          
  SPECIAL_PREVIOUS_BACKCOLOR,      
  SPECIAL_SMALLER_PAINTBRUSH,         
  SPECIAL_BIGGER_PAINTBRUSH,          
  SPECIAL_NEXT_USER_FORECOLOR,     
  SPECIAL_PREVIOUS_USER_FORECOLOR, 
  SPECIAL_NEXT_USER_BACKCOLOR,     
  SPECIAL_PREVIOUS_USER_BACKCOLOR, 
  SPECIAL_SCROLL_UP,
  SPECIAL_SCROLL_DOWN,
  SPECIAL_SCROLL_LEFT,
  SPECIAL_SCROLL_RIGHT,
  SPECIAL_SCROLL_UP_FAST,
  SPECIAL_SCROLL_DOWN_FAST,
  SPECIAL_SCROLL_LEFT_FAST,
  SPECIAL_SCROLL_RIGHT_FAST,
  SPECIAL_SCROLL_UP_SLOW,
  SPECIAL_SCROLL_DOWN_SLOW,
  SPECIAL_SCROLL_LEFT_SLOW,
  SPECIAL_SCROLL_RIGHT_SLOW,
  SPECIAL_SHOW_HIDE_CURSOR,
  SPECIAL_DOT_PAINTBRUSH,
  SPECIAL_CONTINUOUS_DRAW,
  SPECIAL_FLIP_X,
  SPECIAL_FLIP_Y,
  SPECIAL_ROTATE_90,
  SPECIAL_ROTATE_180,
  SPECIAL_STRETCH,
  SPECIAL_DISTORT,
  SPECIAL_OUTLINE,
  SPECIAL_NIBBLE,
  SPECIAL_GET_BRUSH_COLORS,
  SPECIAL_RECOLORIZE_BRUSH,
  SPECIAL_ROTATE_ANY_ANGLE,
  SPECIAL_LOAD_BRUSH,
  SPECIAL_SAVE_BRUSH,
  SPECIAL_INVERT_SIEVE,
  SPECIAL_ZOOM_IN,
  SPECIAL_ZOOM_OUT,
  SPECIAL_CENTER_ATTACHMENT,
  SPECIAL_TOP_LEFT_ATTACHMENT,
  SPECIAL_TOP_RIGHT_ATTACHMENT,
  SPECIAL_BOTTOM_LEFT_ATTACHMENT,
  SPECIAL_BOTTOM_RIGHT_ATTACHMENT,
  SPECIAL_EXCLUDE_COLORS_MENU,
  SPECIAL_SHADE_MODE,       // |
  SPECIAL_SHADE_MENU,       // |
  SPECIAL_QUICK_SHADE_MODE, // |
  SPECIAL_QUICK_SHADE_MENU, // |
  SPECIAL_STENCIL_MODE,     // |
  SPECIAL_STENCIL_MENU,     // |
  SPECIAL_MASK_MODE,        // |  Il faut que le premier effet soit
  SPECIAL_MASK_MENU,        // |  SPECIAL_SHADE_MODE, et que le
  SPECIAL_GRID_MODE,        // |  dernier soit SPECIAL_TILING_MENU,
  SPECIAL_GRID_MENU,        // |_ et que seuls des effets soient
  SPECIAL_SIEVE_MODE,       // |  définis entre ces deux là car
  SPECIAL_SIEVE_MENU,       // |  des tests sur cet intervalle sont
  SPECIAL_COLORIZE_MODE,    // |  faits dans le moteur.
  SPECIAL_COLORIZE_MENU,    // |
  SPECIAL_SMOOTH_MODE,      // |
  SPECIAL_SMOOTH_MENU,      // |
  SPECIAL_SMEAR_MODE,       // |
  SPECIAL_TILING_MODE,      // |
  SPECIAL_TILING_MENU       // |
};

  // Définition des opérations:

enum OPERATIONS
{
  OPERATION_CONTINUOUS_DRAW,     // Dessin à la main continu
  OPERATION_DISCONTINUOUS_DRAW,  // Dessin à la main discontinu
  OPERATION_POINT_DRAW,       // Dessin à la main point par point
  OPERATION_FILLED_CONTOUR,     // Contour rempli
  OPERATION_LINE,              // Lignes
  OPERATION_K_LIGNE,            // Lignes reliées
  OPERATION_CENTERED_LINES,    // Lignes concentriques
  OPERATION_EMPTY_RECTANGLE,     // Rectangle vide
  OPERATION_FILLED_RECTANGLE,    // Rectangle plein
  OPERATION_EMPTY_CIRCLE,        // Cercle vide
  OPERATION_FILLED_CIRCLE,       // Cercle plein
  OPERATION_EMPTY_ELLIPSE,       // Ellipse vide
  OPERATION_FILLED_ELLIPSE,     // Ellipse pleine
  OPERATION_FILL,               // Fill
  OPERATION_REPLACE,          // Replace couleur
  OPERATION_GRAB_BRUSH,       // Prise de brosse rectangulaire
  OPERATION_POLYBRUSH,         // Prise d'une brosse multiforme
  OPERATION_COLORPICK,            // Récupération d'une couleur
  OPERATION_MAGNIFY,              // Positionnement de la fenêtre de loupe
  OPERATION_3_POINTS_CURVE,    // Courbe à 3 points
  OPERATION_4_POINTS_CURVE,    // Courbe à 4 points
  OPERATION_AIRBRUSH,              // Spray
  OPERATION_POLYGON,           // Polygone
  OPERATION_POLYFORM,           // Polyforme
  OPERATION_POLYFILL,           // Polyfill
  OPERATION_FILLED_POLYFORM,    // Polyforme rempli
  OPERATION_SCROLL,             // Scroll
  OPERATION_GRAD_CIRCLE,     // Cercles dégradés
  OPERATION_GRAD_ELLIPSE,   // Ellipses dégradées
  OPERATION_ROTATE_BRUSH,     // Faire tourner brosse
  OPERATION_STRETCH_BRUSH,      // Etirer brosse
  OPERATION_DISTORT_BRUSH,    // Deformer brosse
  OPERATION_GRAD_RECTANGLE,  // Rectangle dégradé

  OPERATION_NONE
};



#endif
