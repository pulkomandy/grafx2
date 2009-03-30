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
#define GLOBAL_VARIABLES

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
#include "misc.h"
#include "init.h"
#include "buttons.h"
#include "engine.h"
#include "pages.h"
#include "loadsave.h"
#include "sdlscreen.h"
#include "errors.h"
#include "readini.h"
#include "saveini.h"
#include "io.h"
#include "text.h"
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

#if defined (__WIN32__)
  // On Windows, SDL_putenv is not present in any compilable header.
  // It can be linked anyway, this declaration only avoids
  // a compilation warning.
  extern DECLSPEC int SDLCALL SDL_putenv(const char *variable);
#endif

byte Old_lines_number;                // old nombre de lignes de l'écran


//--- Affichage de la syntaxe, et de la liste des modes vidéos disponibles ---
void Display_syntax(void)
{
  int mode_index;
  printf("Syntax: GFX2 [<arguments>] [<picture>]\n\n");
  printf("<arguments> can be:]\n");
  printf("\t/? /h /help        for this help screen\n");
  printf("\t/wide              to emulate a video mode with wide pixels (2x1)\n");
  printf("\t/tall              to emulate a video mode with tall pixels (1x2)\n");
  printf("\t/double            to emulate a video mode with double pixels (2x2)\n");
  printf("\t/mode <videomode>  to set a video mode\n\n");
  printf("Available video modes:\n\n");
  for (mode_index=0; mode_index<Nb_video_modes; mode_index++)
    printf("\t%s\n",Mode_label(mode_index));
}


// ---------------------------- Sortie impromptue ----------------------------
void Error_function(int error_code, const char *filename, int line_number, const char *function_name)
{
  T_Palette temp_palette;
  int       index;
  printf("Error number %d occured in file %s, line %d, function %s.\n", error_code, filename,line_number,function_name);

  if (error_code==0)
  {
    // L'erreur 0 n'est pas une vraie erreur, elle fait seulement un flash rouge de l'écran pour dire qu'il y a un problème.
    // Toutes les autres erreurs déclenchent toujours une sortie en catastrophe du programme !
    memcpy(temp_palette,Main_palette,sizeof(T_Palette));
    for (index=0;index<=255;index++)
      temp_palette[index].R=255;
    Set_palette(temp_palette);
    SDL_Delay(500);
    Set_palette(Main_palette);
  }
  else
  {
    switch (error_code)
    {
      case ERROR_GUI_MISSING         : printf("Error: File gfx2gui.gif is missing!\n");
                                       printf("This program cannot run without this file.\n");
                                       break;
      case ERROR_GUI_CORRUPTED       : printf("Error: File gfx2gui.gif is corrupt!\n");
                                       printf("This program cannot run without a correct version of this file.\n");
                                       break;
      case ERROR_INI_MISSING         : printf("Error: File gfx2def.ini is missing!\n");
                                       printf("This program cannot run without this file.\n");
                                       break;
      case ERROR_CFG_MISSING         : printf("Error: File GFX2.CFG is missing!\n");
                                       printf("Please run GFXCFG to create it.\n");
                                       break;
      case ERROR_CFG_CORRUPTED       : printf("Error: File GFX2.CFG is corrupt!\n");
                                       printf("Please run GFXCFG to make a valid file.\n");
                                       break;
      case ERROR_CFG_OLD         : printf("Error: File GFX2.CFG is from another version of GrafX2.\n");
                                       printf("Please run GFXCFG to update this file.\n");
                                       break;
      case ERROR_MEMORY            : printf("Error: Not enough memory!\n\n");
                                       printf("You should try exiting other programs to free some bytes for Grafx2.\n\n");
                                       break;
      case ERROR_MOUSE_DRIVER      : printf("Error: No mouse detected!\n");
                                       printf("Check if a mouse driver is installed and if your mouse is correctly connected.\n");
                                       break;
      case ERROR_FORBIDDEN_MODE      : printf("Error: The requested video mode has been disabled from the resolution menu!\n");
                                       printf("If you want to run the program in this mode, you'll have to start it with an\n");
                                       printf("enabled mode, then enter the resolution menu and enable the mode you want.\n");
                                       printf("Check also if the 'Default_video_mode' parameter in GFX2.INI is correct.\n");
                                       break;
      case ERROR_COMMAND_LINE     : printf("Error: Invalid parameter or file not found.\n\n");
                                       Display_syntax();
                                       break;
      case ERROR_SAVING_CFG     : printf("Error: Write error while saving settings!\n");
                                       printf("Settings have not been saved correctly, and the GFX2.CFG file may have been\n");
                                       printf("corrupt. If so, please run GFXCFG to make a new valid file.\n");
                                       break;
      case ERROR_MISSING_DIRECTORY : printf("Error: Directory you ran the program from not found!\n");
                                       break;
      case ERROR_INI_CORRUPTED       : printf("Error: File GFX2.INI is corrupt!\n");
                                       printf("It contains bad values at line %d.\n",Line_number_in_INI_file);
                                       printf("You can re-generate it by deleting the file and running GrafX2 again.\n");
                                       break;
      case ERROR_SAVING_INI     : printf("Error: Cannot rewrite file GFX2.INI!\n");
                                       break;
      case ERROR_SORRY_SORRY_SORRY  : printf("Error: Sorry! Sorry! Sorry! Please forgive me!\n");
                                       break;
    }

    SDL_Quit();
    exit(error_code);
  }
}

// --------------------- Analyse de la ligne de commande ---------------------
void Analyze_command_line(int argc,char * argv[])
{
  char *buffer ;
  int index;


  File_in_command_line=0;
  Resolution_in_command_line=0;
  
  Current_resolution=Config.Default_resolution;
  
  for (index=1; index<argc; index++)
  {
    if ( !strcmp(argv[index],"/?") ||
         !strcmp(argv[index],"/h") ||
         !strcmp(argv[index],"/H") )
    {
      // help
      Display_syntax();
      exit(0);
    }
    else if ( !strcmp(argv[index],"/mode") )
    {
      // mode
      index++;
      if (index<argc)
      {    
        Resolution_in_command_line = 1;
        Current_resolution=Convert_videomode_arg(argv[index]);
        if (Current_resolution == -1)
        {
          Error(ERROR_COMMAND_LINE);
          Display_syntax();
          exit(0);
        }
        if ((Video_mode[Current_resolution].State & 0x7F) == 3)
        {
          Error(ERROR_FORBIDDEN_MODE);
          exit(0);
        }
      }
      else
      {
        Error(ERROR_COMMAND_LINE);
        Display_syntax();
        exit(0);
      }
    }
    else if ( !strcmp(argv[index],"/tall") )
    {
      Pixel_ratio = PIXEL_TALL;
    }
    else if ( !strcmp(argv[index],"/wide") )
    {
      Pixel_ratio = PIXEL_WIDE;
    }
    else if ( !strcmp(argv[index],"/double") )
    {
      Pixel_ratio = PIXEL_DOUBLE;
    }
    else if ( !strcmp(argv[index],"/rgb") )
    {
      // echelle des composants RGB
      index++;
      if (index<argc)
      {
        int scale;
        scale = atoi(argv[index]);
        if (scale < 2 || scale > 256)
        {
          Error(ERROR_COMMAND_LINE);
          Display_syntax();
          exit(0);
        }
        Set_palette_RGB_scale(scale);
      }
      else
      {
        Error(ERROR_COMMAND_LINE);
        Display_syntax();
        exit(0);
      }
    }
    else
    {
      // Si ce n'est pas un paramètre, c'est le nom du fichier à ouvrir
      if (File_in_command_line)
      {
        // plusieurs noms de fichier en argument
        Error(ERROR_COMMAND_LINE);
        Display_syntax();
        exit(0);
      }
      else if (File_exists(argv[index]))
      {
        File_in_command_line=1;

        // On récupère le chemin complet du paramètre
        // Et on découpe ce chemin en répertoire(path) + fichier(.ext)
        #if defined(__WIN32__)
          buffer=_fullpath(NULL,argv[index],MAX_PATH_CHARACTERS);
        #else
          buffer=realpath(argv[index],NULL);
        #endif
        Extract_path(Main_file_directory, buffer);
        Extract_filename(Main_filename, buffer);
        free(buffer);
        chdir(Main_file_directory);
      }
      else
      {
        Error(ERROR_COMMAND_LINE);
        Display_syntax();
        exit(0);
      }
    }
  }
}

// ------------------------ Initialiser le programme -------------------------
// Returns 0 on fail
int Init_program(int argc,char * argv[])
{
  int temp;
  int starting_videomode;
  char program_directory[MAX_PATH_CHARACTERS];

  // On crée dès maintenant les descripteurs des listes de pages pour la page
  // principale et la page de brouillon afin que leurs champs ne soient pas
  // invalide lors des appels aux multiples fonctions manipulées à
  // l'initialisation du programme.
  Main_backups=(T_List_of_pages *)malloc(sizeof(T_List_of_pages));
  Spare_backups=(T_List_of_pages *)malloc(sizeof(T_List_of_pages));
  Init_list_of_pages(Main_backups);
  Init_list_of_pages(Spare_backups);

  // Determine the executable directory
  Set_program_directory(argv[0],program_directory);
  // Choose directory for data (read only)
  Set_data_directory(program_directory,Repertoire_des_donnees);
  // Choose directory for settings (read/write)
  Set_config_directory(program_directory,Config_directory);

  // On détermine le répertoire courant:
  getcwd(Main_current_directory,256);

  // On en profite pour le mémoriser dans le répertoire principal:
  strcpy(Initial_directory,Main_current_directory);

  // On initialise les données sur le nom de fichier de l'image principale:
  strcpy(Main_file_directory,Main_current_directory);
  strcpy(Main_filename,"NO_NAME.GIF");
  Main_fileformat=DEFAULT_FILEFORMAT;
  // On initialise les données sur le nom de fichier de l'image de brouillon:
  strcpy(Spare_current_directory,Main_current_directory);
  strcpy(Spare_file_directory,Main_file_directory);
  strcpy(Spare_filename       ,Main_filename);
  Spare_fileformat    =Main_fileformat;
  strcpy(Brush_current_directory,Main_current_directory);
  strcpy(Brush_file_directory,Main_file_directory);
  strcpy(Brush_filename       ,Main_filename);
  Brush_fileformat    =Main_fileformat;

  // On initialise ce qu'il faut pour que les fileselects ne plantent pas:
  Filelist=NULL;       // Au début, il n'y a pas de fichiers dans la liste
  Main_fileselector_position=0; // Au début, le fileselect est en haut de la liste des fichiers
  Main_fileselector_offset=0; // Au début, le fileselect est en haut de la liste des fichiers
  Main_format=0;
  Spare_fileselector_position=0;
  Spare_fileselector_offset=0;
  Spare_format=0;
  Brush_fileselector_position=0;
  Brush_fileselector_offset=0;
  Brush_format=0;

  // On initialise les commentaires des images à des chaînes vides
  Main_comment[0]='\0';
  Spare_comment[0]='\0';
  Brush_comment[0]='\0';

  // On initialise d'ot' trucs
  Main_offset_X=0;
  Main_offset_Y=0;
  Old_main_offset_X=0;
  Old_main_offset_Y=0;
  Main_separator_position=0;
  Main_X_zoom=0;
  Main_separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Main_magnifier_mode=0;
  Main_magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Main_magnifier_height=0;
  Main_magnifier_width=0;
  Main_magnifier_offset_X=0;
  Main_magnifier_offset_Y=0;
  Spare_offset_X=0;
  Spare_offset_Y=0;
  Old_spare_offset_X=0;
  Old_spare_offset_Y=0;
  Spare_separator_position=0;
  Spare_X_zoom=0;
  Spare_separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Spare_magnifier_mode=0;
  Spare_magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Spare_magnifier_height=0;
  Spare_magnifier_width=0;
  Spare_magnifier_offset_X=0;
  Spare_magnifier_offset_Y=0;

  // SDL
  if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0)
  {
    // The program can't continue without that anyway
    printf("Couldn't initialize SDL.\n");
    return(0);
  }
  Joystick = SDL_JoystickOpen(0);
  SDL_EnableKeyRepeat(250, 32);
  SDL_EnableUNICODE(SDL_ENABLE);
  if(ALPHA_BETA[0]=='ß')
    SDL_WM_SetCaption("GrafX2 beta "PERCENTAGE_VERSION""" - USE AT YOUR OWN RISK","GrafX2");
  else
    SDL_WM_SetCaption("GrafX2 v2.00 final","GrafX2");
  {
    // Routine pour définir l'icone.
    char icon_path[MAX_PATH_CHARACTERS];
    SDL_Surface * icon;
    sprintf(icon_path, "%s%s", Repertoire_des_donnees, "gfx2.gif");
    icon = IMG_Load(icon_path);
    if (icon)
    {
      byte *icon_mask;
      int x,y;
      icon_mask=malloc(128);
      memset(icon_mask,0,128);
      for (y=0;y<32;y++)
        for (x=0;x<32;x++)
          if (((byte *)(icon->pixels))[(y*32+x)] != 255)
            icon_mask[(y*32+x)/8] |=0x80>>(x&7);
      SDL_WM_SetIcon(icon,icon_mask);
      free(icon_mask);
      SDL_FreeSurface(icon);
    }
  }
  // Texte
  Init_text();

  // On initialise tous les modes vidéo
  Set_all_video_modes();
  Pixel_ratio=PIXEL_SIMPLE;
  // On initialise les données sur l'état du programme:
    // Donnée sur la sortie du programme:
  Quit_is_required=0;
  Quitting=0;
    // Données sur l'état du menu:
  Pixel_in_menu=Pixel_in_toolbar;
  Menu_is_visible=1;
    // Données sur les couleurs et la palette:
  Fore_color=15;
  Back_color=0;
  First_color_in_palette=0;
    // Données sur le curseur:
  Cursor_shape=CURSOR_SHAPE_TARGET;
  Cursor_hidden=0;
    // Données sur le pinceau:
  Paintbrush_X=0;
  Paintbrush_Y=0;
  Paintbrush_shape=PAINTBRUSH_SHAPE_ROUND;
  Paintbrush_hidden=0;

  Pixel_load_function=Pixel_load_in_current_screen;

  // On initialise tout ce qui concerne les opérations et les effets
  Operation_stack_size=0;
  Selected_operation=OPERATION_CONTINUOUS_DRAW;
  Selected_line_mode         =OPERATION_LINE;
  Selected_curve_mode        =OPERATION_3_POINTS_CURVE;
  Effect_function=No_effect;
    // On initialise les infos de la loupe:
  Main_magnifier_mode=0;
  Main_magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Init_multiplication_tables();
  Zoom_factor_table=Magnify_table[2];
  Main_separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Spare_separator_proportion=INITIAL_SEPARATOR_PROPORTION;
    // On initialise les infos du mode smear:
  Smear_mode=0;
  Smear_brush_width=PAINTBRUSH_WIDTH;
  Smear_brush_height=PAINTBRUSH_HEIGHT;
    // On initialise les infos du mode smooth:
  Smooth_mode=0;
    // On initialise les infos du mode shade:
  Shade_mode=0;     // Les autres infos du Shade sont chargées avec la config
  Quick_shade_mode=0; // idem
    // On initialise les infos sur les dégradés:
  Gradient_pixel =Display_pixel; // Les autres infos sont chargées avec la config
    // On initialise les infos de la grille:
  Snap_mode=0;
  Snap_width=8;
  Snap_height=8;
  Snap_offset_X=0;
  Snap_offset_Y=0;
    // On initialise les infos du mode Colorize:
  Colorize_mode=0;          // Mode colorize inactif par défaut
  Colorize_opacity=50;      // Une interpolation de 50% par défaut
  Colorize_current_mode=0; // Par défaut, la méthode par interpolation
  Compute_colorize_table();
    // On initialise les infos du mode Tiling:
  Tiling_mode=0;  //   Pas besoin d'initialiser les décalages car ça se fait
                  // en prenant une brosse (toujours mis à 0).
    // On initialise les infos du mode Mask:
  Mask_mode=0;

    // Infos du Spray
  Airbrush_mode=1; // Mode Mono
  Airbrush_size=31;
  Airbrush_delay=1;
  Airbrush_mono_flow=10;
  memset(Airbrush_multi_flow,0,256);
  srand(time(NULL)); // On randomize un peu tout ça...

  // Initialisation des boutons
  Init_buttons();
  // Initialisation des opérations
  Init_operations();

  Windows_open=0;
  
  // Charger les sprites et la palette
  Load_DAT();
  // Charger la configuration des touches
  Set_config_defaults();
  switch(Load_CFG(1))
  {
    case ERROR_CFG_MISSING:
      // Pas un problème, on a les valeurs par défaut.
      break;
    case ERROR_CFG_CORRUPTED:
      DEBUG("Corrupted CFG file.",0);
      break;
    case ERROR_CFG_OLD:
      DEBUG("Unknown CFG file version, not loaded.",0);
      break;
  }
  // Charger la configuration du .INI
  temp=Load_INI(&Config);
  if (temp)
    Error(temp);

  // Transfert des valeurs du .INI qui ne changent pas dans des variables
  // plus accessibles:
  Default_palette[MC_Black] =Fav_menu_colors[0]=Config.Fav_menu_colors[0];
  Default_palette[MC_Dark]=Fav_menu_colors[1]=Config.Fav_menu_colors[1];
  Default_palette[MC_Light]=Fav_menu_colors[2]=Config.Fav_menu_colors[2];
  Default_palette[MC_White]=Fav_menu_colors[3]=Config.Fav_menu_colors[3];
  memcpy(Main_palette,Default_palette,sizeof(T_Palette));

  Compute_optimal_menu_colors(Default_palette);

  // Infos sur les trames (Sieve)
  Sieve_mode=0;
  Copy_preset_sieve(0);

  // Prise en compte de la fonte
  if (Config.Font)
    Menu_font=GFX_fun_font;
  else
    Menu_font=GFX_system_font;

  // Allocation de mémoire pour la brosse
  if (!(Brush         =(byte *)malloc(   1*   1))) Error(ERROR_MEMORY);
  if (!(Smear_brush   =(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE))) Error(ERROR_MEMORY);

  // Pinceau
  if (!(Paintbrush_sprite=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE))) Error(ERROR_MEMORY);
  *Paintbrush_sprite=1;
  Paintbrush_width=1;
  Paintbrush_height=1;

  Analyze_command_line(argc,argv);
  starting_videomode=Current_resolution;
  Horizontal_line_buffer=NULL;
  Screen_width=Screen_height=Current_resolution=0;
  
  Init_mode_video(
    Video_mode[starting_videomode].Width,
    Video_mode[starting_videomode].Height,
    Video_mode[starting_videomode].Fullscreen,
    Pixel_ratio);

  Main_image_width=Screen_width/Pixel_width;
  Main_image_height=Screen_height/Pixel_height;
  Spare_image_width=Screen_width/Pixel_width;
  Spare_image_height=Screen_height/Pixel_height;
  // Allocation de mémoire pour les différents écrans virtuels (et brosse)
  if (Init_all_backup_lists(Config.Max_undo_pages+1,Screen_width,Screen_height)==0)
    Error(ERROR_MEMORY);
  // On remet le nom par défaut pour la page de brouillon car il été modifié
  // par le passage d'un fichier en paramètre lors du traitement précédent.
  // Note: le fait que l'on ne modifie que les variables globales 
  // Brouillon_* et pas les infos contenues dans la page de brouillon 
  // elle-même ne m'inspire pas confiance mais ça a l'air de marcher sans 
  // poser de problèmes, alors...
  if (File_in_command_line)
  {
    strcpy(Spare_file_directory,Spare_current_directory);
    strcpy(Spare_filename,"NO_NAME.GIF");
    Spare_fileformat=DEFAULT_FILEFORMAT;
  }

  // Nettoyage de l'écran virtuel (les autres recevront celui-ci par copie)
  memset(Main_screen,0,Main_image_width*Main_image_height);

  // Initialisation de diverses variables par calcul:
  Compute_magnifier_data();
  Compute_limits();
  Compute_paintbrush_coordinates();

  // On affiche le menu:
  Display_menu();
  Display_paintbrush_in_menu();
  Display_sprite_in_menu(BUTTON_PAL_LEFT,18+(Config.Palette_vertical!=0));

  // On affiche le curseur pour débutter correctement l'état du programme:
  Display_cursor();

  Spare_image_is_modified=0;
  Main_image_is_modified=0;

  // Gestionnaire de signaux, quand il ne reste plus aucun espoir
  Init_sighandler();

  // Le programme débute en mode de dessin à la main
  Unselect_button(BUTTON_DRAW,LEFT_SIDE);

  // On initialise la brosse initiale à 1 pixel blanc:
  Brush_width=1;
  Brush_height=1;
  Capture_brush(0,0,0,0,0);
  *Brush=MC_White;
  return(1);
}

// ------------------------- Fermeture du programme --------------------------
void Program_shutdown(void)
{
  int      return_code;

  // On libère le buffer de gestion de lignes
  free(Horizontal_line_buffer);

  // On libère le pinceau spécial
  free(Paintbrush_sprite);

  // On libère les différents écrans virtuels et brosse:
  free(Brush);
  Set_number_of_backups(0);
  free(Spare_screen);
  free(Main_screen);

  // On prend bien soin de passer dans le répertoire initial:
  if (chdir(Initial_directory)!=-1)
  {
    // On sauvegarde les données dans le .CFG et dans le .INI
    if (Config.Auto_save)
    {
      return_code=Save_CFG();
      if (return_code)
        Error(return_code);
      return_code=Save_INI(&Config);
      if (return_code)
        Error(return_code);
    }
  }
  else
    Error(ERROR_MISSING_DIRECTORY);
    
  SDL_Quit();
}


// -------------------------- Procédure principale ---------------------------
int main(int argc,char * argv[])
{
  int phoenix_found=0;
  int phoenix2_found=0;
  char phoenix_filename1[MAX_PATH_CHARACTERS];
  char phoenix_filename2[MAX_PATH_CHARACTERS];

  if(!Init_program(argc,argv))
  {
    return 0;
  }

  // Test de recuperation de fichiers sauvés
  strcpy(phoenix_filename1,Config_directory);
  strcat(phoenix_filename1,"phoenix.img");
  strcpy(phoenix_filename2,Config_directory);
  strcat(phoenix_filename2,"phoenix2.img");
  if (File_exists(phoenix_filename1))
    phoenix_found=1;
  if (File_exists(phoenix_filename2))
    phoenix2_found=1;
  if (phoenix_found || phoenix2_found)
  {
    if (phoenix2_found)
    {
      strcpy(Main_file_directory,Config_directory);
      strcpy(Main_filename,"phoenix2.img");
      chdir(Main_file_directory);
      Button_Reload();
      Main_image_is_modified=1;
      Warning_message("Spare page recovered");
      // I don't really like this, but...
      remove(phoenix_filename2);
      Button_Page();
    }
    if (phoenix_found)
    {
      strcpy(Main_file_directory,Config_directory);
      strcpy(Main_filename,"phoenix.img");
      chdir(Main_file_directory);
      Button_Reload();
      Main_image_is_modified=1;
      Warning_message("Main page recovered");
      // I don't really like this, but...
      remove(phoenix_filename1);
    }
  }
  else
  {
    if (Config.Opening_message && (!File_in_command_line))
      Button_Message_initial();
    free(GFX_logo_grafx2); // Pas encore utilisé dans le About
  
    if (File_in_command_line)
    {
      Button_Reload();
      Resolution_in_command_line=0;
    }
  }
  Main_handler();

  Program_shutdown();
  return 0;
}
