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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__WIN32__)
  #include <windows.h>
  #include <io.h> // Mingw's _mkdir()
#elif defined(__macosx__)
  #import <corefoundation/corefoundation.h>
  #import <sys/param.h>
#endif

#include "struct.h"
#include "io.h"
#include "files.h"

int Create_ConfigDirectory(char * Config_Dir)
{
  #ifdef __WIN32__
    // Mingw's mkdir has a weird name and only one argument
    return _mkdir(Config_Dir);
  #else
    return mkdir(Config_Dir,S_IRUSR|S_IWUSR|S_IXUSR);
  #endif
}

#if defined(__macosx__) || defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
  #define ARG_UNUSED __attribute__((unused))
#else
  #define ARG_UNUSED
#endif
// Determine which directory contains the executable.
// IN: Main's argv[0], some platforms need it, some don't.
// OUT: Write into Program_Dir. Trailing / or \ is kept.
// Note : in fact this is only used to check for the datafiles and fonts in this same directory.
void Set_Program_Directory(ARG_UNUSED const char * argv0,char * Program_Dir)
{
  #undef ARG_UNUSED

  // MacOSX
  #if defined(__macosx__)
    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(url,true,(UInt8*)Program_Dir,MAXPATHLEN);
    CFRelease(url);
    // Append trailing slash
    strcat(Program_Dir    ,"/");
  
  // AmigaOS4: hard-coded volume name.
  #elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
    strcpy(Program_Dir,"PROGDIR:");

  // Others: The part of argv[0] before the executable name.    
  // Keep the last \ or /.
  // Note that on Unix, once installed, the executable is called from a shell script
  // sitting in /usr/local/bin/, this allows argv[0] to contain the full path.
  // On Windows, Mingw32 already provides the full path in all cases.
  #else
    Extraire_chemin(Program_Dir, argv0);
  #endif
}
// Determine which directory contains the read-only data.
// IN: The directory containing the executable
// OUT: Write into Data_Dir. Trailing / or \ is kept.
void Set_Data_Directory(const char * Program_Dir, char * Data_Dir)
{
  // On all platforms, data is in the executable's directory
  strcpy(Data_Dir,Program_Dir);
  // Except MacOSX:
  #if defined(__macosx__)
    strcat(Data_Dir,"Contents/Resources/");
  #endif
}

// Determine which directory should store the user's configuration.
//
// For most Unix and Windows platforms:
// If a config file already exists in Program_Dir, it will return it in priority
// (Useful for development, and possibly for upgrading from DOS version)
// If the standard directory doesn't exist yet, this function will attempt 
// to create it ($(HOME)/.grafx2, or %APPDATA%\GrafX2)
// If it cannot be created, this function will return the executable's
// own directory.
// IN: The directory containing the executable
// OUT: Write into Config_Dir. Trailing / or \ is kept.
void Set_Config_Directory(const char * Program_Dir, char * Config_Dir)
{
  // MacOSX
  #if defined(__macosx__)
    strcpy(Config_Dir,Program_Dir);
    strcat(Config_Dir,"Contents/Resources/");
  // AmigaOS4
  #elif defined(__amigaos4__) || defined(__AROS__)
    strcpy(Config_Dir,"PROGDIR:");
  #else
    char FileName[TAILLE_CHEMIN_FICHIER];

    // In priority: check own directory
    strcpy(Config_Dir, Program_Dir);
    strcpy(FileName, Config_Dir);
    strcat(FileName, "gfx2.cfg");
    if (!Fichier_existe(FileName))
    {
      char *Config_ParentDir;
      #if defined(__WIN32__)
        // "%APPDATA%\GrafX2"
        const char* Config_SubDir = "GrafX2";
        Config_ParentDir = getenv("APPDATA");
      #elif defined(__BEOS__) || defined(__HAIKU__)
        // "~/.grafx2"
        const char* Config_SubDir = ".grafx2";
        Config_ParentDir = getenv("$HOME");
      #else
        // "~/.grafx2"      
        const char* Config_SubDir = ".grafx2";
        Config_ParentDir = getenv("HOME");
      #endif
      if (Config_ParentDir && Config_ParentDir[0]!='\0')
      {
        int Taille = strlen(Config_ParentDir);
        strcpy(Config_Dir, Config_ParentDir);
        if (Config_ParentDir[Taille-1] != '\\' && Config_ParentDir[Taille-1] != '/')
        {
          strcat(Config_Dir,SEPARATEUR_CHEMIN);
        }
        strcat(Config_Dir,Config_SubDir);
        if (Repertoire_existe(Config_Dir))
        {
          // Répertoire trouvé, ok
          strcat(Config_Dir,SEPARATEUR_CHEMIN);
        }
        else
        {
          // Tentative de création
          if (!Create_ConfigDirectory(Config_Dir)) 
          {
            // Réussi
            strcat(Config_Dir,SEPARATEUR_CHEMIN);
          }
          else
          {
            // Echec: on se rabat sur le repertoire de l'executable.
            strcpy(Config_Dir,Program_Dir);
          }
        }
      }
    }
  #endif
}
