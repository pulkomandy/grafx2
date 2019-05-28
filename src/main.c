/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Pasi Kallinen
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
/**
 * @file main.c
 * Program entry point, global variables and global functions.
 *
 * @mainpage
 * GrafX2 is a bitmap paint program inspired by the Amiga programs
 * Deluxe Paint and Brilliance. Specialized in 256-color drawing,
 * it includes a very large number of tools and effects that make
 * it particularly suitable for pixel art, game graphics,
 * and generally any detailed graphics painted with a mouse.
 *
 * The program is mostly developed on Haiku, Linux, FreeBSD
 * and Windows, but is also portable on many other platforms :
 * It can be built using SDL 1.x or SDL 2.x libraries (see https://www.libsdl.org/)
 * or Xlib or Win32 API.
 *
 * Web for the project users is http://grafx2.tk/.
 *
 * Developpers are welcome to contribute :
 * the code is hosted on gitlab https://gitlab.com/GrafX2/grafX2
 * and a bug tracker, wiki, etc. is available on
 * https://pulkomandy.tk/projects/GrafX2.
 *
 * This Doxygen documentation is browsable on
 * https://pulkomandy.tk/projects/GrafX2/doxygen/ (updated nightly).
 *
 * The inline help is also available here :
 * http://pulkomandy.tk/GrafX2/
 */
/// declare global variables in main.c
#define GLOBAL_VARIABLES

// time.h defines timeval which conflicts with the one in amiga SDK
#ifdef __amigaos__
  #include <devices/timer.h>
#else
  #include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#define strdup _strdup
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#include <SDL_image.h>
#endif

#if defined(WIN32)
#include <windows.h>
#include <shlwapi.h>
#include <io.h>
#include <fcntl.h>
#elif defined (__MINT__)
#include <mint/osbind.h>
#elif defined(__macosx__)
#import <CoreFoundation/CoreFoundation.h>
#import <sys/param.h>
#elif defined(__FreeBSD__)
#include <sys/param.h>
#endif

#if defined(__macosx__)
#include <machine/endian.h>
#elif defined(__SWITCH__)
#include <machine/endian.h>
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#elif !defined(WIN32)
#include <endian.h>
#endif

#include "gfx2log.h"
#include "gfx2mem.h"
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
#include "screen.h"
#include "errors.h"
#include "readini.h"
#include "saveini.h"
#include "io.h"
#include "text.h"
#include "setup.h"
#include "windows.h"
#include "brush.h"
#include "palette.h"
#include "realpath.h"
#include "unicode.h"
#include "input.h"
#include "help.h"
#include "filesel.h"
#include "factory.h"
#if defined(WIN32) && !(defined(USE_SDL) || defined(USE_SDL2))
#include "win32screen.h"
#endif


#if defined (WIN32) && (defined(USE_SDL) || defined(USE_SDL2))
  // On Windows, SDL_putenv is not present in any compilable header.
  // It can be linked anyway, this declaration only avoids
  // a compilation warning.
  extern DECLSPEC int SDLCALL SDL_putenv(const char *variable);
#endif

extern char Program_version[]; // generated in pversion.c

static int setsize_width;
static int setsize_height;

#if (defined(USE_SDL) || defined(USE_SDL2)) && defined(USE_JOYSTICK)
/// Pointer to the current joystick controller.
static SDL_Joystick* Joystick;
#endif

/**
 * Show the command line syntax and available video modes.
 *
 * Output to standard outout (stdout) and show a message box under MS Windows,
 * where standard output is not available
 */
void Display_syntax(void)
{
  int mode_index, i;
  char modes[1024*2];
  const char * syntax =
    "Syntax: grafx2 [<arguments>] [<picture1>] [<picture2>]\n\n"
    "<arguments> can be:\n"
    "\t-? -h -H -help     for this help screen\n"
    "\t-verbose           to increase log verbosity\n"
    "\t-wide              to emulate a video mode with wide pixels (2x1)\n"
    "\t-tall              to emulate a video mode with tall pixels (1x2)\n"
    "\t-double            to emulate a video mode with double pixels (2x2)\n"
    "\t-wide2             to emulate a video mode with double wide pixels (4x2)\n"
    "\t-tall2             to emulate a video mode with double tall pixels (2x4)\n"
    "\t-triple            to emulate a video mode with triple pixels (3x3)\n"
    "\t-quadruple         to emulate a video mode with quadruple pixels (4x4)\n"
    "\t-rgb n             to reduce RGB precision (2 to 256, 256=max precision)\n"
    "\t-gamma n           to adjust Gamma correction (1 to 30, 10=no correction)\n"
    "\t-skin <filename>   to use an alternate file with the menu graphics\n"
    "\t-mode <videomode>  to set a video mode\n"
    "\t-size <resolution> to set the image size\n"
    "Arguments can be prefixed either by / - or --\n"
    "They can also be abbreviated.\n\n";
  fputs(syntax, stdout);

  i = snprintf(modes, sizeof(modes), "Available video modes:\n\n");
  for (mode_index = 0; mode_index < Nb_video_modes; mode_index += 6)
  {
    int k;
    for (k = 0; k < 6; k++)
    {
      if (mode_index + k >= Nb_video_modes) break;
      i += snprintf(modes + i, sizeof(modes) - i, "%12s", Mode_label(mode_index + k));
    }
    i += snprintf(modes + i, sizeof(modes) - i, "\n");
  }
  fputs(modes, stdout);

#if defined(WIN32)
  MessageBoxA(GFX2_Get_Window_Handle(), syntax, "GrafX2", MB_OK);
  MessageBoxA(GFX2_Get_Window_Handle(), modes, "GrafX2", MB_OK);
#endif
}

// ---------------------------- Sortie impromptue ----------------------------
void Warning_function(const char *message, const char *filename, int line_number, const char *function_name)
{
  GFX2_Log(GFX2_WARNING, "Warning in file %s, line %d, function %s : %s\n", filename, line_number, function_name, message);
}


// ---------------------------- Sortie impromptue ----------------------------
void Error_function(int error_code, const char *filename, int line_number, const char *function_name)
{
  T_Palette temp_palette;
  T_Palette backup_palette;
  int       index;
  char msg_buffer[512];

  snprintf(msg_buffer, sizeof(msg_buffer), "Error number %d occurred in file %s, line %d, function %s.\n", error_code, filename,line_number,function_name);
  fputs(msg_buffer, stderr);
#if defined(_MSC_VER) && defined(_DEBUG)
  OutputDebugStringA(msg_buffer);
#endif

  if (error_code==0)
  {
    // L'erreur 0 n'est pas une vraie erreur, elle fait seulement un flash rouge de l'écran pour dire qu'il y a un problème.
    // Toutes les autres erreurs déclenchent toujours une sortie en catastrophe du programme !
    memcpy(backup_palette, Get_current_palette(), sizeof(T_Palette));
    memcpy(temp_palette, backup_palette, sizeof(T_Palette));
    for (index=0;index<=255;index++)
      temp_palette[index].R=255;
    Set_palette(temp_palette);
    Delay_with_active_mouse(50); // Half a second of red flash
    Set_palette(backup_palette);
  }
  else
  {
    const char * msg = NULL;
    switch (error_code)
    {
      case ERROR_GUI_MISSING         : snprintf(msg_buffer, sizeof(msg_buffer), "Error: File containing the GUI graphics is missing!\n"
                                             "This program cannot run without this file.\n"
                                              "\n%s", Gui_loading_error_message);
                                       msg = msg_buffer;
                                       break;
      case ERROR_GUI_CORRUPTED       : msg = "Error: File containing the GUI graphics couldn't be parsed!\n"
                                             "This program cannot run without a correct version of this file.\n";
                                       break;
      case ERROR_INI_MISSING         : msg = "Error: File gfx2def.ini is missing!\n"
                                             "This program cannot run without this file.\n";
                                       break;
      case ERROR_MEMORY              : msg = "Error: Not enough memory!\n\n"
                                             "You should try exiting other programs to free some bytes for Grafx2.\n\n";
                                       break;
      case ERROR_FORBIDDEN_MODE      : msg = "Error: The requested video mode has been disabled from the resolution menu!\n"
                                             "If you want to run the program in this mode, you'll have to start it with an\n"
                                             "enabled mode, then enter the resolution menu and enable the mode you want.\n"
                                             "Check also if the 'Default_video_mode' parameter in gfx2.ini is correct.\n";
                                       break;
      case ERROR_FORBIDDEN_SIZE      : msg = "Error: The image dimensions all have to be in the range 1-9999!\n";
                                       break;
      case ERROR_COMMAND_LINE     : msg = "Error: Invalid parameter or file not found.\n\n";
                                       break;
      case ERROR_SAVING_CFG     : msg = "Error: Write error while saving settings!\n"
                                        "Settings have not been saved correctly, and the gfx2.cfg file may have been\n"
                                        "corrupt. If so, please delete it and Grafx2 will restore default settings.\n";
                                       break;
      case ERROR_MISSING_DIRECTORY : msg = "Error: Directory you ran the program from not found!\n";
                                       break;
      case ERROR_INI_CORRUPTED       : snprintf(msg_buffer, sizeof(msg_buffer), "Error: File gfx2.ini is corrupt!\n"
                                                "It contains bad values at line %d.\n"
                                                "You can re-generate it by deleting the file and running GrafX2 again.\n",
                                                Line_number_in_INI_file);
                                       msg = msg_buffer;
                                       break;
      case ERROR_SAVING_INI     : msg = "Error: Cannot rewrite file gfx2.ini!\n";
                                       break;
      case ERROR_SORRY_SORRY_SORRY  : msg = "Error: Sorry! Sorry! Sorry! Please forgive me!\n";
                                       break;
    }

    if(msg != NULL)
    {
      fputs(msg, stderr);
#if defined(WIN32)
#if defined(_DEBUG)
      OutputDebugStringA(msg);
#endif
      MessageBoxA(GFX2_Get_Window_Handle(), msg, "GrafX2 error", MB_OK | MB_ICONERROR);
#endif
    }

    if (error_code == ERROR_COMMAND_LINE)
      Display_syntax();

#if defined(USE_SDL) || defined(USE_SDL2)
    SDL_Quit();
#endif
    exit(error_code);
  }
}

enum CMD_PARAMS
{
    CMDPARAM_HELP,
    CMDPARAM_MODE,
    CMDPARAM_PIXELRATIO_TALL,
    CMDPARAM_PIXELRATIO_WIDE,
    CMDPARAM_PIXELRATIO_DOUBLE,
    CMDPARAM_PIXELRATIO_TRIPLE,
    CMDPARAM_PIXELRATIO_QUAD,
    CMDPARAM_PIXELRATIO_TALL2,
    CMDPARAM_PIXELRATIO_TALL3,
    CMDPARAM_PIXELRATIO_WIDE2,
    CMDPARAM_RGB,
    CMDPARAM_GAMMA,
    CMDPARAM_SKIN,
    CMDPARAM_SIZE,
    CMDPARAM_VERBOSE,
};

struct {
    const char *param;
    int id;
} cmdparams[] = {
    {"?", CMDPARAM_HELP},
    {"h", CMDPARAM_HELP},
    {"H", CMDPARAM_HELP},
    {"help", CMDPARAM_HELP},
    {"mode", CMDPARAM_MODE},
    {"tall", CMDPARAM_PIXELRATIO_TALL},
    {"wide", CMDPARAM_PIXELRATIO_WIDE},
    {"double", CMDPARAM_PIXELRATIO_DOUBLE},
    {"triple", CMDPARAM_PIXELRATIO_TRIPLE},
    {"quadruple", CMDPARAM_PIXELRATIO_QUAD},
    {"tall2", CMDPARAM_PIXELRATIO_TALL2},
    {"tall3", CMDPARAM_PIXELRATIO_TALL3},
    {"wide2", CMDPARAM_PIXELRATIO_WIDE2},
    {"rgb", CMDPARAM_RGB},
    {"gamma", CMDPARAM_GAMMA},
    {"skin", CMDPARAM_SKIN},
    {"size", CMDPARAM_SIZE},
    {"verbose", CMDPARAM_VERBOSE},
};

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

/**
 * Parse the command line.
 *
 * @param argc argument count
 * @param argv argument values
 * @param filenames pointers to receive file names
 * @param directories pointers to receive file directories
 * @param videomode_arg pointer to receive the -mode argument
 * @param pixel_ratio pointer to receive the pixel ratio requested
 * @return the number of file to open (0, 1 or 2)
 */
int Analyze_command_line(int argc, char * argv[], char * filenames[], char * directories[], const char ** videomode_arg, int * pixel_ratio)
{
  char *buffer;
  char *filename;
  int index;
  int file_in_command_line;

  file_in_command_line = 0;
  Resolution_in_command_line = 0;

  Current_resolution = Config.Default_resolution;

  for (index = 1; index<argc; index++)
  {
    char *s = argv[index];
    int is_switch = ((strchr(s,'/') == s) || (strchr(s,'-') == s) || (strstr(s,"--") == s));
    int tmpi;
    char *tmpcp;
    int paramtype = -1;
    if (is_switch)
    {
      int param_matches = 0;
      int param_match = -1;
      if (*s == '-')
      {
        s++;
        if (*s == '-')
          s++;
      }
      else
        s++;

#ifdef __macosx__
      // ignore -psn argument
      if (memcmp(s, "psn_", 4) ==0)
        continue;
#endif

      for (tmpi = 0; tmpi < ARRAY_SIZE(cmdparams); tmpi++)
      {
        if (!strcmp(s, cmdparams[tmpi].param))
        {
          paramtype = cmdparams[tmpi].id;
          break;
        }
        else if (strstr(cmdparams[tmpi].param, s))
        {
          param_matches++;
          param_match = cmdparams[tmpi].id;
        }
      }
      if (paramtype == -1 && param_matches == 1)
        paramtype = param_match;

    }
    switch (paramtype)
    {
      case CMDPARAM_HELP:
        Display_syntax();
        exit(0);
      case CMDPARAM_MODE:
        index++;
        if (index<argc)
        {
          // will be processed later, when video is initialized
          *videomode_arg = argv[index];
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
      case CMDPARAM_PIXELRATIO_TALL:
        *pixel_ratio = PIXEL_TALL;
        break;
      case CMDPARAM_PIXELRATIO_WIDE:
        *pixel_ratio = PIXEL_WIDE;
        break;
      case CMDPARAM_PIXELRATIO_DOUBLE:
        *pixel_ratio = PIXEL_DOUBLE;
        break;
      case CMDPARAM_PIXELRATIO_TRIPLE:
        *pixel_ratio = PIXEL_TRIPLE;
        break;
      case CMDPARAM_PIXELRATIO_QUAD:
        *pixel_ratio = PIXEL_QUAD;
        break;
      case CMDPARAM_PIXELRATIO_TALL2:
        *pixel_ratio = PIXEL_TALL2;
        break;
      case CMDPARAM_PIXELRATIO_TALL3:
        *pixel_ratio = PIXEL_TALL3;
        break;
      case CMDPARAM_PIXELRATIO_WIDE2:
        *pixel_ratio = PIXEL_WIDE2;
        break;
      case CMDPARAM_RGB:
        /* RGB scale */
        index++;
        if (index<argc)
        {
          int scale;
          scale = atoi(argv[index]);
          if (scale < 2 || scale > 256)
          {
            Error(ERROR_COMMAND_LINE);
            exit(0);
          }
          Set_palette_RGB_scale(scale);
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
      case CMDPARAM_GAMMA:
        /* Gamma correction */
        index++;
        if (index<argc)
        {
          int scale;
          scale = atoi(argv[index]);
          if (scale < 1 || scale > 30)
          {
            Error(ERROR_COMMAND_LINE);
            exit(0);
          }
          Set_palette_Gamma(scale);
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
      case CMDPARAM_SKIN:
        // GUI skin file
        index++;
        if (index<argc)
        {
          strcpy(Config.Skin_file,argv[index]);
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
      case CMDPARAM_SIZE:
        index++;
        if (index<argc)
        {
          setsize_width = atoi(argv[index]);
          tmpcp = strchr(argv[index], 'x');
          if (tmpcp == NULL)
            tmpcp = strchr(argv[index], 'X');
          if (tmpcp == NULL)
          {
            Error(ERROR_COMMAND_LINE);
            exit(0);
          }
          setsize_height = atoi(++tmpcp);
          if (setsize_height < 1 || setsize_height > 9999 ||
              setsize_width < 1 || setsize_width > 9999)
          {
            Error(ERROR_FORBIDDEN_SIZE);
            exit(0);
          }
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
      case CMDPARAM_VERBOSE:
        GFX2_verbosity_level++;
        break;
      default:
        // Si ce n'est pas un paramètre, c'est le nom du fichier à ouvrir
        if (file_in_command_line > 1)
        {
          // Il y a déjà 2 noms de fichiers et on vient d'en trouver un 3ème
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        else if (File_exists(argv[index]))
        {
          buffer = Realpath(argv[index], NULL);
          filename = Find_last_separator(buffer);
          if (filename != NULL)
          {
            *filename = '\0';
            filename++;
            directories[file_in_command_line] = strdup(buffer);
            filenames[file_in_command_line] = strdup(filename);
          }
          else
          {
            directories[file_in_command_line] = strdup(".");
            filenames[file_in_command_line] = strdup(buffer);
          }
          free(buffer);
          buffer = NULL;
          file_in_command_line++;
        }
        else
        {
          Error(ERROR_COMMAND_LINE);
          exit(0);
        }
        break;
    }
  }
  return file_in_command_line;
}

// Compile-time assertions:
#define CT_ASSERT(e) extern char (*ct_assert(void)) [sizeof(char[1 - 2*!(e)])]

// This line will raise an error at compile time
// when sizeof(T_Components) is not 3.
CT_ASSERT(sizeof(T_Components)==3);

// This line will raise an error at compile time
// when sizeof(T_Palette) is not 768.
CT_ASSERT(sizeof(T_Palette)==768);

#if defined(__MINT__)
static void Exit_handler(void)
{
  printf("Press any key to quit.\n");
  (void)Cnecin();
}
#endif

/**
 * Initialize the  program.
 *
 * @param argc command line argument count
 * @param argv command line argument values
 * @return 0 on fail
 * @return 1 on success
 */
int Init_program(int argc,char * argv[])
{
  int temp;
  int starting_videomode;
  enum IMAGE_MODES starting_image_mode;
  char * program_directory;
  T_Gui_skin *gfx;
  int file_in_command_line;
  T_Gradient_array initial_gradients;
  char * filenames[2] = {NULL, NULL};
  char * directories[2] = {NULL, NULL};
  const char * videomode = NULL;
  int cmdline_pixelratio = -1;

  #if defined(__MINT__)
  printf("===============================\n");
  printf(" /|\\ GrafX2 %.19s\n", Program_version);
  printf(" compilation date: %.16s\n", __DATE__);
  printf("===============================\n");
  atexit(Exit_handler);
  #endif

#ifdef ENABLE_FILENAMES_ICONV
  // iconv is used to convert filenames
  cd = iconv_open(TOCODE, FROMCODE);  // From UTF8 to ANSI
  cd_inv = iconv_open(FROMCODE, TOCODE);  // From ANSI to UTF8
#if (defined(SDL_BYTEORDER) && (SDL_BYTEORDER == SDL_BIG_ENDIAN)) || (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN))
  cd_utf16 = iconv_open("UTF-16BE", FROMCODE); // From UTF8 to UTF16
  cd_utf16_inv = iconv_open(FROMCODE, "UTF-16BE"); // From UTF16 to UTF8
#else
  cd_utf16 = iconv_open("UTF-16LE", FROMCODE); // From UTF8 to UTF16
  cd_utf16_inv = iconv_open(FROMCODE, "UTF-16LE"); // From UTF16 to UTF8
#endif
#endif /* ENABLE_FILENAMES_ICONV */

  // Analyse command-line as soon as possible.
  file_in_command_line = Analyze_command_line(argc, argv, filenames, directories, &videomode, &cmdline_pixelratio);

  // On crée dès maintenant les descripteurs des listes de pages pour la page
  // principale et la page de brouillon afin que leurs champs ne soient pas
  // invalide lors des appels aux multiples fonctions manipulées à
  // l'initialisation du programme.
  Main.backups=(T_List_of_pages *)GFX2_malloc(sizeof(T_List_of_pages));
  Spare.backups=(T_List_of_pages *)GFX2_malloc(sizeof(T_List_of_pages));
  Init_list_of_pages(Main.backups);
  Init_list_of_pages(Spare.backups);

  // Determine the executable directory
  program_directory = Get_program_directory(argv[0]);
  // Choose directory for data (read only)
  Data_directory = Get_data_directory(program_directory);
  // Choose directory for settings (read/write)
  Config_directory = Get_config_directory(program_directory);
  // Get current directory
  Main.selector.Directory = Get_current_directory(NULL, &Main.selector.Directory_unicode, 0);

  GFX2_Log(GFX2_DEBUG, "program directory : %s\n", program_directory);
  GFX2_Log(GFX2_DEBUG, "Data directory : %s\n", Data_directory);
  GFX2_Log(GFX2_DEBUG, "Config directory : %s\n", Config_directory);
  GFX2_Log(GFX2_DEBUG, "Initial_directory : %s (unicode : %p)\n",
           Main.selector.Directory, Main.selector.Directory_unicode);
  free(program_directory);

  // On en profite pour le mémoriser dans le répertoire principal:
  Initial_directory = strdup(Main.selector.Directory);

  // On initialise les données sur le nom de fichier de l'image de brouillon:
  Spare.selector.Directory = strdup(Main.selector.Directory);
  Spare.selector.Directory_unicode = Unicode_strdup(Main.selector.Directory_unicode);

  Main.fileformat  = DEFAULT_FILEFORMAT;
  Spare.fileformat = DEFAULT_FILEFORMAT;

  Brush_selector.Directory = strdup(Main.selector.Directory);
  Brush_selector.Directory_unicode = Unicode_strdup(Main.selector.Directory_unicode);
  Brush_file_directory = strdup(Main.selector.Directory);
  Brush_filename = strdup("NO_NAME.GIF");
  Brush_filename_unicode = NULL;
  Brush_fileformat = DEFAULT_FILEFORMAT;

  Palette_selector.Directory = strdup(Main.selector.Directory);
  Palette_selector.Directory_unicode = Unicode_strdup(Main.selector.Directory_unicode);

  // On initialise ce qu'il faut pour que les fileselects ne plantent pas:

  Main.selector.Position=0; // Au début, le fileselect est en haut de la liste des fichiers
  Main.selector.Offset=0; // Au début, le fileselect est en haut de la liste des fichiers
  Main.selector.Format_filter=FORMAT_ALL_IMAGES;

  Main.current_layer=0;
  Main.layers_visible=0xFFFFFFFF;
  Main.layers_visible_backup=0xFFFFFFFF;
  Spare.current_layer=0;
  Spare.layers_visible=0xFFFFFFFF;
  Spare.layers_visible_backup=0xFFFFFFFF;

  Spare.selector.Position=0;
  Spare.selector.Offset=0;
  Spare.selector.Format_filter=FORMAT_ALL_IMAGES;
  Brush_selector.Position=0;
  Brush_selector.Offset=0;
  Brush_selector.Format_filter=FORMAT_ALL_IMAGES;

  Palette_selector.Position=0;
  Palette_selector.Offset=0;
  Palette_selector.Format_filter=FORMAT_ALL_PALETTES;

  // On initialise d'ot' trucs
  Main.offset_X=0;
  Main.offset_Y=0;
  Main.separator_position=0;
  Main.X_zoom=0;
  Main.separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Main.magnifier_mode=0;
  Main.magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Main.magnifier_height=0;
  Main.magnifier_width=0;
  Main.magnifier_offset_X=0;
  Main.magnifier_offset_Y=0;
  Spare.offset_X=0;
  Spare.offset_Y=0;
  Spare.separator_position=0;
  Spare.X_zoom=0;
  Spare.separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Spare.magnifier_mode=0;
  Spare.magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Spare.magnifier_height=0;
  Spare.magnifier_width=0;
  Spare.magnifier_offset_X=0;
  Spare.magnifier_offset_Y=0;
  Keyboard_click_allowed = 1;

  Main.safety_backup_prefix = SAFETYBACKUP_PREFIX_A[0];
  Spare.safety_backup_prefix = SAFETYBACKUP_PREFIX_B[0];
  Main.time_of_safety_backup = 0;
  Spare.time_of_safety_backup = 0;


#if defined(USE_SDL) || defined(USE_SDL2)
  // SDL
  if (SDL_Init(SDL_INIT_VIDEO
#if defined(USE_JOYSTICK)
               | SDL_INIT_JOYSTICK
#endif
              ) < 0)
  {
    // The program can't continue without that anyway
    printf("Couldn't initialize SDL.\n");
    return(0);
  }

#if defined(USE_SDL2)
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif
#endif
#if defined(USE_SDL)
  SDL_EnableKeyRepeat(250, 32);
  SDL_EnableUNICODE(SDL_ENABLE);
  SDL_WM_SetCaption("GrafX2","GrafX2");
#endif
  Define_icon();

  // Texte
  Init_text();

  // Initialize all video modes
  Set_all_video_modes();

#if defined(USE_JOYSTICK) && (defined(USE_SDL) || defined(USE_SDL2))
  GFX2_Log(GFX2_DEBUG, "%d joystick(s) attached\n", SDL_NumJoysticks());
  if (SDL_NumJoysticks() > 0)
  {
    Joystick = SDL_JoystickOpen(0);
    if (Joystick == NULL)
    {
      GFX2_Log(GFX2_ERROR, "Failed to open joystick #0 : %s\n", SDL_GetError());
    }
    else
    {
      GFX2_Log(GFX2_DEBUG, "Joystick #0 open : \"%s\" %d axes, %d buttons, %d balls, %d hats\n",
               SDL_JoystickName(Joystick), SDL_JoystickNumAxes(Joystick),
               SDL_JoystickNumButtons(Joystick), SDL_JoystickNumBalls(Joystick),
               SDL_JoystickNumHats(Joystick));
      SDL_JoystickEventState(SDL_ENABLE);
    }
  }
#endif

  Pixel_ratio=PIXEL_SIMPLE;
  // On initialise les données sur l'état du programme:
  // Donnée sur la sortie du programme:
  Quit_is_required=0;
  Quitting=0;
  // Données sur l'état du menu:
  Menu_is_visible=1;
  // Données sur les couleurs et la palette:
  First_color_in_palette=0;
  // Données sur le curseur:
  Cursor_shape=CURSOR_SHAPE_TARGET;
  Cursor_hidden=0;
  // Données sur le pinceau:
  Paintbrush_X=0;
  Paintbrush_Y=0;
  Paintbrush_hidden=0;

  // On initialise tout ce qui concerne les opérations et les effets
  Operation_stack_size=0;
  Selected_freehand_mode=OPERATION_CONTINUOUS_DRAW;
  Selected_line_mode         =OPERATION_LINE;
  Selected_curve_mode        =OPERATION_3_POINTS_CURVE;
  Effect_function=No_effect;
    // On initialise les infos de la loupe:
  Main.magnifier_mode=0;
  Main.magnifier_factor=DEFAULT_ZOOM_FACTOR;
  Main.separator_proportion=INITIAL_SEPARATOR_PROPORTION;
  Spare.separator_proportion=INITIAL_SEPARATOR_PROPORTION;
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

  // Initialize the brush container
  Init_brush_container();

  Windows_open=0;

  // Paintbrush
  Paintbrush_sprite = (byte *)GFX2_malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
  if (!Paintbrush_sprite)
    Error(ERROR_MEMORY);

  // Load preset paintbrushes (uses Paintbrush_ variables)
  Init_paintbrushes();

  // Set a valid paintbrush afterwards
  *Paintbrush_sprite=1;
  Paintbrush_width=1;
  Paintbrush_height=1;
  Paintbrush_offset_X=0;
  Paintbrush_offset_Y=0;
  Paintbrush_shape=PAINTBRUSH_SHAPE_ROUND;

  #if defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
  // Prefer cycling active by default
  Cycling_mode=1;
  #endif

  // Charger la configuration des touches
  Set_config_defaults();

  switch(Load_CFG(1))
  {
    case ERROR_CFG_MISSING:
      // Pas un problème, on a les valeurs par défaut.
      break;
    case ERROR_CFG_CORRUPTED:
      GFX2_Log(GFX2_ERROR, "Corrupted CFG file.\n");
      break;
    case ERROR_CFG_OLD:
      GFX2_Log(GFX2_WARNING, "Unknown CFG file version, not loaded.\n");
      break;
  }
  // Charger la configuration du .INI
  temp=Load_INI(&Config);
  if (temp)
    Error(temp);

  if(!Config.Allow_multi_shortcuts)
  {
    Remove_duplicate_shortcuts();
  }

  Compute_menu_offsets();

  Current_help_section=0;
  Help_position=0;

  // Load sprites, palette etc.
  gfx = Load_graphics(Config.Skin_file, &initial_gradients);
  if (gfx == NULL)
  {
    gfx = Load_graphics(DEFAULT_SKIN_FILENAME, &initial_gradients);
    if (gfx == NULL)
    {
      Error(ERROR_GUI_MISSING);
    }
  }
  Set_current_skin(Config.Skin_file, gfx);
  // Override colors
  Gfx->Default_palette[MC_Black]=Config.Fav_menu_colors[0];
  Gfx->Default_palette[MC_Dark] =Config.Fav_menu_colors[1];
  Gfx->Default_palette[MC_Light]=Config.Fav_menu_colors[2];
  Gfx->Default_palette[MC_White]=Config.Fav_menu_colors[3];

  // Even when using the skin's palette, if RGB range is small
  // the colors will be unusable.
  Compute_optimal_menu_colors(Gfx->Default_palette);

  // Infos sur les trames (Sieve)
  Sieve_mode=0;
  Copy_preset_sieve(0);

  // Font
  if (!(Menu_font=Load_font(Config.Font_file, 1)))
    if (!(Menu_font=Load_font(DEFAULT_FONT_FILENAME, 1)))
      {
        snprintf(Gui_loading_error_message, sizeof(Gui_loading_error_message),
                 "Unable to open the default font file: %s\n", DEFAULT_FONT_FILENAME);
        Error(ERROR_GUI_MISSING);
      }
  Load_Unicode_fonts();

  memcpy(Main.palette, Gfx->Default_palette, sizeof(T_Palette));

  Fore_color=Best_color_range(255,255,255,Config.Palette_cells_X*Config.Palette_cells_Y);
  Back_color=Best_color_range(0,0,0,Config.Palette_cells_X*Config.Palette_cells_Y);

  // Allocation de mémoire pour la brosse
  Brush = (byte *)GFX2_malloc(1*1);
  if (!Brush)
    Error(ERROR_MEMORY);
  Smear_brush = (byte *)GFX2_malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
  if (!Smear_brush)
    Error(ERROR_MEMORY);

  // set videomode according to the command line
  if (videomode)
  {
    Resolution_in_command_line = 1;
    Current_resolution = Convert_videomode_arg(videomode);
    if (Current_resolution == -1)
    {
      Error(ERROR_COMMAND_LINE);
      exit(0);
    }
    if ((Video_mode[Current_resolution].State & 0x7F) == 3)
    {
      Error(ERROR_FORBIDDEN_MODE);
      exit(0);
    }
  }

  starting_videomode=Current_resolution;
  Horizontal_line_buffer=NULL;
  Screen_width=Screen_height=Current_resolution=0;
  if (cmdline_pixelratio >= 0 && cmdline_pixelratio < (int)PIXEL_MAX)
    Pixel_ratio = cmdline_pixelratio;

  Init_mode_video(
    Video_mode[starting_videomode].Width,
    Video_mode[starting_videomode].Height,
    Video_mode[starting_videomode].Fullscreen,
    Pixel_ratio);

  // Windows only: move back the window to its original position.
  #if defined(WIN32)
  if (!Video_mode[starting_videomode].Fullscreen)
  {
    if (Config.Window_pos_x != 9999 && Config.Window_pos_y != 9999)
    {
      //RECT r;
      #if defined(USE_SDL) || defined(USE_SDL2)
      //GetWindowRect(window, &r);
      SetWindowPos(GFX2_Get_Window_Handle(), 0, Config.Window_pos_x, Config.Window_pos_y, 0, 0, SWP_NOSIZE);
      #endif
    }
  }

  if ((unsigned)GFX2_verbosity_level >= (unsigned)GFX2_DEBUG)
  {
    // Open a console for debugging...
    if (AllocConsole())
    {
      HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
      int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
      FILE* hf_out = _fdopen(hCrt, "w");
      setvbuf(hf_out, NULL, _IONBF, 2);
      *stdout = *hf_out;
      *stderr = *hf_out;
    }
  }
  #endif

  Main.image_width=Screen_width/Pixel_width;
  Main.image_height=Screen_height/Pixel_height;
  Spare.image_width=Screen_width/Pixel_width;
  Spare.image_height=Screen_height/Pixel_height;

  // set the Mouse cursor at the center of the screen
  Mouse_X = Screen_width / 2;
  Mouse_Y = Screen_height / 2;

  starting_image_mode = Config.Default_mode_layers ?
    IMAGE_MODE_LAYERED : IMAGE_MODE_ANIMATION;
  // Allocation de mémoire pour les différents écrans virtuels (et brosse)
  if (Init_all_backup_lists(starting_image_mode , Screen_width, Screen_height)==0)
    Error(ERROR_MEMORY);
  // Update toolbars' visibility, now that the current image has a mode
  Check_menu_mode();

  // Nettoyage de l'écran virtuel (les autres recevront celui-ci par copie)
  memset(Main_screen,0,Main.image_width*Main.image_height);

  // If image size was specified on command line, set that now
  if (setsize_width != 0 && setsize_height != 0)
  {
    Main.image_width=setsize_width;
    Main.image_height=setsize_height;
    Spare.image_width=setsize_width;
    Spare.image_height=setsize_height;
  }

  // Now that the backup system is there, we can store the gradients.
  memcpy(Main.backups->Pages->Gradients->Range, initial_gradients.Range, sizeof(initial_gradients.Range));
  memcpy(Spare.backups->Pages->Gradients->Range, initial_gradients.Range, sizeof(initial_gradients.Range));

  Gradient_function=Gradient_basic;
  Gradient_lower_bound=0;
  Gradient_upper_bound=0;
  Gradient_random_factor=1;
  Gradient_bounds_range=1;

  Current_gradient=0;

  // Initialisation de diverses variables par calcul:
  Compute_magnifier_data();
  Compute_limits();
  Compute_paintbrush_coordinates();

  // On affiche le menu:
  Display_paintbrush_in_menu();
  Display_sprite_in_menu(BUTTON_PAL_LEFT,Config.Palette_vertical?MENU_SPRITE_VERTICAL_PALETTE_SCROLL:-1);
  Display_menu();
  Draw_menu_button(BUTTON_PAL_LEFT,BUTTON_RELEASED);
  Draw_menu_button(BUTTON_PAL_RIGHT,BUTTON_RELEASED);

  // On affiche le curseur pour débuter correctement l'état du programme:
  Display_cursor();

  Spare.image_is_modified=0;
  Main.image_is_modified=0;

  // Gestionnaire de signaux, quand il ne reste plus aucun espoir
  Init_sighandler();

  // Le programme débute en mode de dessin à la main
  Select_button(BUTTON_DRAW,LEFT_SIDE);

  // On initialise la brosse initiale à 1 pixel blanc:
  Brush_width=1;
  Brush_height=1;
  for (temp=0;temp<256;temp++)
    Brush_colormap[temp]=temp;
  Capture_brush(0,0,0,0,0);
  *Brush=MC_White;
  *Brush_original_pixels=MC_White;

  // Make sure the load dialog points to the right place when first shown.
  // Done after loading everything else, but before checking for emergency
  // backups
  if (file_in_command_line > 0 && directories[0] != NULL)
  {
    strcpy(Main.selector.Directory, directories[0]);
  }

  // Test de recuperation de fichiers sauvés
  switch (Check_recovery())
  {
    T_IO_Context context;

    default:
      // Some files were loaded from last crash-exit.
      // Do not load files from command-line, nor show splash screen.
      Compute_optimal_menu_colors(Main.palette);
      Check_menu_mode();
      Display_all_screen();
      Display_menu();
      Display_cursor();
      Verbose_message("Images recovered",
        "Grafx2 has recovered images from\n"
        "last session, before a crash or\n"
        "shutdown. Browse the history using\n"
        "the Undo/Redo button, and when\n"
        "you find a state that you want to\n"
        "save, use the 'Save as' button to\n"
        "save the image.\n"
        "Some backups can be present in\n"
        "the spare page too.\n");
      break;

    case -1: // Unable to write lock file
      Verbose_message("Warning",
        "Safety backups (every minute) are\n"
        "disabled because Grafx2 is running\n"
        "from a read-only device, or other\n"
        "instances are running.");
      break;

    case 0:

      switch (file_in_command_line)
      {
        case 0:
          if (Config.Opening_message)
            Button_Message_initial();
          // Load default palette
          {
            FILE * f;
            Init_context_layered_image(&context, DEFAULTPAL_FILENAME, Config_directory);
            context.Type = CONTEXT_PALETTE;
            context.Format = FORMAT_PAL;
            f = Open_file_read(&context);
            if (f != NULL)  // silently fail if the file cannot be open
            {
              fclose(f);
              Load_image(&context);
              if (File_error == 0)
              {
                Hide_cursor();
                Compute_optimal_menu_colors(Main.palette);
                Display_menu();
                Display_cursor();
                memcpy(Spare.palette, Main.palette, sizeof(T_Palette));
              }
            }
            Destroy_context(&context);
          }
          break;

        case 2:
          // Load this file
          Init_context_layered_image(&context, filenames[1], directories[1]);
          context.File_name_unicode = Get_Unicode_Filename(NULL, filenames[1], directories[1]);
          Load_image(&context);
          Destroy_context(&context);
          Redraw_layered_image();
          End_of_modification();

          Button_Page(BUTTON_PAGE);
          // no break ! proceed with the other file now
#if defined(__GNUC__) && (__GNUC__ >= 7)
          __attribute__ ((fallthrough));
#endif
        case 1:
          Init_context_layered_image(&context, filenames[0], directories[0]);
          context.File_name_unicode = Get_Unicode_Filename(NULL, filenames[0], directories[0]);
          Load_image(&context);
          Destroy_context(&context);
          Redraw_layered_image();
          End_of_modification();

          // If only one image was loaded, assume the spare has same image type
          if (file_in_command_line==1)
          {
            if (Main.backups->Pages->Image_mode <= IMAGE_MODE_ANIMATION)
              Spare.backups->Pages->Image_mode = Main.backups->Pages->Image_mode;
          }

          Hide_cursor();
          Compute_optimal_menu_colors(Main.palette);
          Back_color=Main.backups->Pages->Background_transparent ?
            Main.backups->Pages->Transparent_color :
            Best_color_range(0,0,0,Config.Palette_cells_X*Config.Palette_cells_Y);
          Fore_color=Main.palette[Back_color].R+Main.palette[Back_color].G+Main.palette[Back_color].B < 3*127 ?
            Best_color_range(255,255,255,Config.Palette_cells_X*Config.Palette_cells_Y) :
            Best_color_range(0,0,0,Config.Palette_cells_X*Config.Palette_cells_Y);
          Check_menu_mode();
          Display_all_screen();
          Display_menu();
          Display_cursor();
          Resolution_in_command_line = 0;
          break;

        default:
          break;
      }
  }

  Allow_drag_and_drop(1);

  while (file_in_command_line-- > 0)
  {
    free(directories[file_in_command_line]);
    free(filenames[file_in_command_line]);
  }
  return(1);
}

/// Free the memory and make sure the pointer is set to NULL
#define FREE_POINTER(p) free(p); p = NULL

/**
 * Program Shutdown.
 *
 * Free all allocated resources.
 */
void Program_shutdown(void)
{
  int      i;
  int      return_code;

  // Windows only: Recover the window position.
  #if defined(WIN32)
  {
    WINDOWPLACEMENT windowplacement;
    windowplacement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(GFX2_Get_Window_Handle(), &windowplacement))
    {
      Config.Window_pos_x = windowplacement.rcNormalPosition.left;
      Config.Window_pos_y = windowplacement.rcNormalPosition.top;
    }
  }
  #elif !defined(USE_X11)
  // All other targets: irrelevant dimensions.
  // Do not attempt to force them back on next program run.
    Config.Window_pos_x = 9999;
    Config.Window_pos_y = 9999;
  #endif

  // Remove the safety backups, this is normal exit
  Delete_safety_backups();

  // On libère le buffer de gestion de lignes
  free(Horizontal_line_buffer);
  Horizontal_line_buffer = NULL;

  // On libère le pinceau spécial
  FREE_POINTER(Paintbrush_sprite);

  // Free Brushes
  FREE_POINTER(Brush);
  FREE_POINTER(Smear_brush);
  FREE_POINTER(Brush_original_pixels);

  FREE_POINTER(Brush_file_directory);
  FREE_POINTER(Brush_filename);
  FREE_POINTER(Brush_filename_unicode);

  // Free all images
  Set_number_of_backups(-1); // even delete the main page

  FREE_POINTER(Main.visible_image.Image);
  FREE_POINTER(Spare.visible_image.Image);
  FREE_POINTER(Main_visible_image_backup.Image);
  FREE_POINTER(Main_visible_image_depth_buffer.Image);

  FREE_POINTER(Main.backups);
  FREE_POINTER(Spare.backups);

  // Free the skin (Gui graphics) data
  free(Gfx);
  Gfx=NULL;

  free(Menu_font);
  Menu_font = NULL;
  while (Unicode_fonts != NULL)
  {
    T_Unicode_Font * ufont = Unicode_fonts->Next;
    free(Unicode_fonts->FontData);
    free(Unicode_fonts);
    Unicode_fonts = ufont;
  }

  // On prend bien soin de passer dans le répertoire initial:
  if (Change_directory(Initial_directory)==0)
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

  FREE_POINTER(Initial_directory);
  FREE_POINTER(Config_directory);
  FREE_POINTER(Data_directory);

  FREE_POINTER(Main.selector.Directory);
  FREE_POINTER(Main.selector.Directory_unicode);
  FREE_POINTER(Main.selector.filename);
  FREE_POINTER(Main.selector.filename_unicode);
  FREE_POINTER(Spare.selector.Directory);
  FREE_POINTER(Spare.selector.Directory_unicode);
  FREE_POINTER(Spare.selector.filename);
  FREE_POINTER(Spare.selector.filename_unicode);
  FREE_POINTER(Brush_selector.Directory);
  FREE_POINTER(Brush_selector.Directory_unicode);
  FREE_POINTER(Brush_selector.filename);
  FREE_POINTER(Brush_selector.filename_unicode);
  FREE_POINTER(Palette_selector.Directory);
  FREE_POINTER(Palette_selector.Directory_unicode);
  FREE_POINTER(Palette_selector.filename);
  FREE_POINTER(Palette_selector.filename_unicode);

  // Free Config
  FREE_POINTER(Config.Skin_file);
  FREE_POINTER(Config.Font_file);
  for (i=0;i<NB_BOOKMARKS;i++)
  {
    FREE_POINTER(Config.Bookmark_directory[i]);
  }
  FREE_POINTER(Config.Scripts_directory);

  for (i = 0; i < 10; i++)
  {
    FREE_POINTER(Bound_script[i]);
  }

  Uninit_text();

#ifdef ENABLE_FILENAMES_ICONV
  if (cd != (iconv_t)-1)
    iconv_close(cd);
  if (cd_inv != (iconv_t)-1)
    iconv_close(cd_inv);
  if (cd_utf16 != (iconv_t)-1)
    iconv_close(cd_utf16);
  if (cd_utf16_inv != (iconv_t)-1)
    iconv_close(cd_utf16_inv);
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
  SDL_Quit();
#endif

  #if defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
  chdir("/usr/gp2x");
  execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
  #endif

}


/**
 * Program entry point
 */
#if defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main(int argc,char * argv[])
#endif
{
#if defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2)
  WCHAR * ModuleFileName = NULL;
  DWORD ModuleFileNameSize = 64;
  DWORD Ret;
  WCHAR * TmpArg = NULL;
  DWORD TmpArgSize = 0;
  WCHAR * ShortFileName;
  DWORD ShortFileNameSize;
  DWORD i, j, k;
  int inquote = 0;
  int argc = 0;
  char arg_buffer[4096];
  char * argv[64] = {NULL};

  Init_Win32(hInstance, hPrevInstance);

  // GetModuleFileNameW() returns the size argument when the
  // path is truncated.
  do
  {
    ModuleFileNameSize *= 2;
    free(ModuleFileName);
    ModuleFileName = (WCHAR *)malloc(sizeof(WCHAR) * ModuleFileNameSize);
    if (ModuleFileName == NULL)
    {
      MessageBoxA(NULL, "Error initializing program (alloc)", NULL, MB_OK | MB_ICONERROR);
      return 1;
    }
    Ret = GetModuleFileNameW(NULL, ModuleFileName, ModuleFileNameSize);
    if (Ret == 0)
    {
      MessageBoxA(NULL, "Error initializing program\nGetModuleFileName()", NULL, MB_OK | MB_ICONERROR);
      return 1;
    }
  }
  while (Ret >= ModuleFileNameSize);

  ShortFileNameSize = GetShortPathNameW(ModuleFileName, NULL, 0);
  if (ShortFileNameSize == 0)
  {
    MessageBoxA(NULL, "Error initializing program\nGetShortPathName()", NULL, MB_OK | MB_ICONERROR);
    return 1;
  }
  ShortFileName = (WCHAR *)malloc(sizeof(WCHAR) * ShortFileNameSize);
  if (ShortFileName == NULL)
  {
    MessageBoxA(NULL, "Error initializing program (alloc)", NULL, MB_OK | MB_ICONERROR);
    return 1;
  }
  if (GetShortPathNameW(ModuleFileName, ShortFileName, ShortFileNameSize) == 0)
  {
    MessageBoxA(NULL, "Error initializing program\nGetShortPathName()", NULL, MB_OK | MB_ICONERROR);
    return 1;
  }
  free(ModuleFileName);

  // set argv[0]
  argv[argc++] = arg_buffer;
  for (i = 0; i < (int)sizeof(arg_buffer); )
  {
    arg_buffer[i] = (char)ShortFileName[i];
    if (arg_buffer[i++] == 0) break;
  }
  free(ShortFileName);

  // set remaining argv[]
  k = 0;
  for (j = 0; pCmdLine[j] != 0; j++)
  {
    if (k >= TmpArgSize)
    {
      TmpArgSize = (TmpArgSize == 0) ? 16 : 2 * TmpArgSize;
      TmpArg = (WCHAR *)realloc(TmpArg, sizeof(WCHAR) * TmpArgSize);
      if (TmpArg == NULL)
      {
        MessageBoxA(NULL, "Error initializing program (realloc)", NULL, MB_OK | MB_ICONERROR);
        return 1;
      }
    }
    if (inquote)
    {
      if (pCmdLine[j] == '"')
      {
        inquote = 0;
        continue;
      }
    }
    else
    {
      if (pCmdLine[j] == '"')
      {
        inquote = 1;
        continue;
      }
      if (pCmdLine[j] == ' ' || pCmdLine[j] == '\t')
      { // next argument
        TmpArg[k++] = '\0';
        argv[argc++] = arg_buffer + i;
        ShortFileNameSize = GetShortPathNameW(TmpArg, NULL, 0);
        if (ShortFileNameSize > 0)
        {
          ShortFileName = (WCHAR *)malloc(sizeof(WCHAR) * ShortFileNameSize);
          Ret = GetShortPathNameW(TmpArg, ShortFileName, ShortFileNameSize);
          for (k = 0; ShortFileName[k] != 0; k++)
            arg_buffer[i++] = ShortFileName[k];
          free(ShortFileName);
        }
        else
        {
          for (k = 0; TmpArg[k] != 0; k++)
            arg_buffer[i++] = TmpArg[k];
        }
        arg_buffer[i++] = 0;
        k = 0;
        continue;
      }
    }
    TmpArg[k++] = pCmdLine[j];
  }
  // process the last argument
  if (k > 0)
  {
    TmpArg[k] = '\0';
    argv[argc++] = arg_buffer + i;
    ShortFileNameSize = GetShortPathNameW(TmpArg, NULL, 0);
    if (ShortFileNameSize > 0)
    {
      ShortFileName = (WCHAR *)malloc(sizeof(WCHAR) * ShortFileNameSize);
      Ret = GetShortPathNameW(TmpArg, ShortFileName, ShortFileNameSize);
      for (k = 0; ShortFileName[k] != 0; k++)
        arg_buffer[i++] = ShortFileName[k];
      free(ShortFileName);
    }
    else
    {
      for (k = 0; TmpArg[k] != 0; k++)
        arg_buffer[i++] = TmpArg[k];
    }
    arg_buffer[i++] = 0;
  }

  free(TmpArg);
  // TODO : nCmdShow indicates if the window must be maximized, etc.
  (void)nCmdShow;
#endif
  if(!Init_program(argc,argv))
  {
    Program_shutdown();
    return 0;
  }

#ifdef _MSC_VER
  GFX2_Log(GFX2_DEBUG, "built with _MSC_VER=%d   Windows ANSI Code Page=%u\n", _MSC_VER, GetACP());
#endif
  Main_handler();

  Program_shutdown();
  return 0;
}

#if defined(WIN32) && !defined(USE_SDL) && !defined(USE_SDL2) && !defined(_MSC_VER)
/**
 * MS Window entry point.
 *
 * This function is used when building with MinGW
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR _lpCmdLine, int nCmdShow)
{
  WCHAR *lpCmdLine = GetCommandLineW();
  (void)_lpCmdLine;

  if (__argc == 1)
  { // avoids GetCommandLineW bug that does not always quote the program name if no arguments
    do { ++lpCmdLine; } while (*lpCmdLine);
  }
  else
  {
    BOOL quoted = lpCmdLine[0] == L'"';
    ++lpCmdLine; // skips the " or the first letter (all paths are at least 1 letter)
    while (*lpCmdLine)
    {
      if (quoted && lpCmdLine[0] == L'"') quoted = FALSE; // found end quote
      else if (!quoted && lpCmdLine[0] == L' ')
      { // found an unquoted space, now skip all spaces
        do { ++lpCmdLine; } while (lpCmdLine[0] == L' ');
        break;
      }
      ++lpCmdLine;
    }
  }
  return wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#endif
