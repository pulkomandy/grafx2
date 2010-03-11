/* vim:expandtab:ts=2 sw=2:
*/
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
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
#elif defined(__FreeBSD__)
  #import <sys/param.h>
#elif defined(__linux__)
  #include <limits.h>
  #include <unistd.h>
#endif

#include "struct.h"
#include "io.h"
#include "setup.h"

int Create_ConfigDirectory(char * config_dir)
{
  #ifdef __WIN32__
    // Mingw's mkdir has a weird name and only one argument
    return _mkdir(config_dir);
  #else
    return mkdir(config_dir,S_IRUSR|S_IWUSR|S_IXUSR);
  #endif
}

#if defined(__macosx__) || defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__) && !defined(__VBCC__)
  #define ARG_UNUSED __attribute__((unused))
#else
  #define ARG_UNUSED
#endif
// Determine which directory contains the executable.
// IN: Main's argv[0], some platforms need it, some don't.
// OUT: Write into program_dir. Trailing / or \ is kept.
// Note : in fact this is only used to check for the datafiles and fonts in 
// this same directory.
void Set_program_directory(ARG_UNUSED const char * argv0,char * program_dir)
{
  #undef ARG_UNUSED

  // MacOSX
  #if defined(__macosx__)
    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(url,true,(UInt8*)program_dir,MAXPATHLEN);
    CFRelease(url);
    // Append trailing slash
    strcat(program_dir    ,"/");
  
  // AmigaOS and alike: hard-coded volume name.
  #elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    strcpy(program_dir,"PROGDIR:");

  // Linux: argv[0] unreliable
  #elif defined(__linux__)
  if (argv0[0]!='/')
  {
    char path[PATH_MAX];
    readlink("/proc/self/exe", path, sizeof(path));
    Extract_path(program_dir, path);
    return;
  }  
  Extract_path(program_dir, argv0);
  
  // Others: The part of argv[0] before the executable name.    
  // Keep the last \ or /.
  // On Windows, Mingw32 already provides the full path in all cases.
  #else
    Extract_path(program_dir, argv0);
  #endif
}

// Determine which directory contains the read-only data.
// IN: The directory containing the executable
// OUT: Write into data_dir. Trailing / or \ is kept.
void Set_data_directory(const char * program_dir, char * data_dir)
{
  // On all platforms, data is relative to the executable's directory
  strcpy(data_dir,program_dir);
  // On MacOSX,  it is stored in a special folder:
  #if defined(__macosx__)
    strcat(data_dir,"Contents/Resources/");
  // On GP2X, executable is not in bin/
  #elif defined (__gp2x__)
    strcat(data_dir,"share/grafx2/");
  // All other targets, program is in a "bin" subdirectory
  #else
    strcat(data_dir,"../share/grafx2/");
  #endif
}

// Determine which directory should store the user's configuration.
//
// For most Unix and Windows platforms:
// If a config file already exists in program_dir, it will return it in priority
// (Useful for development, and possibly for upgrading from DOS version)
// If the standard directory doesn't exist yet, this function will attempt 
// to create it ($(HOME)/.grafx2, or %APPDATA%\GrafX2)
// If it cannot be created, this function will return the executable's
// own directory.
// IN: The directory containing the executable
// OUT: Write into config_dir. Trailing / or \ is kept.
void Set_config_directory(const char * program_dir, char * config_dir)
{
  // AmigaOS4
  #if defined(__amigaos4__) || defined(__AROS__)
    strcpy(config_dir,"PROGDIR:");
  // GP2X
  #elif defined(__GP2X__)
    // On the GP2X, the program is installed to the sdcard, and we don't want to mess with the system tree which is
    // on an internal flash chip. So, keep these settings locals.
    strcpy(config_dir,program_dir);
  #else
    char filename[MAX_PATH_CHARACTERS];

    // In priority: check root directory
    strcpy(config_dir, program_dir);
    // On all these targets except OSX and GP2X, the executable is in ./bin
    #if !defined(__macosx__) && !defined(__gp2x__)
      strcat(config_dir, "../");
    #endif
    strcpy(filename, config_dir);
    strcat(filename, "gfx2.cfg");

    if (!File_exists(filename))
    {
      char *config_parent_dir;
      #if defined(__WIN32__)
        // "%APPDATA%\GrafX2"
        const char* Config_SubDir = "GrafX2";
        config_parent_dir = getenv("APPDATA");
      #elif defined(__BEOS__) || defined(__HAIKU__)
        // "~/.grafx2", the BeOS way
        const char* Config_SubDir = ".grafx2";
        config_parent_dir = getenv("$HOME");
      #elif defined(__macosx__)
        // "~/Library/Preferences/com.googlecode.grafx2"
        const char* Config_SubDir = "Library/Preferences/com.googlecode.grafx2";
        config_parent_dir = getenv("HOME");
      #else
        // "~/.grafx2"      
        const char* Config_SubDir = ".grafx2";
        config_parent_dir = getenv("HOME");
      #endif

      if (config_parent_dir && config_parent_dir[0]!='\0')
      {
        int size = strlen(config_parent_dir);
        strcpy(config_dir, config_parent_dir);
        if (config_parent_dir[size-1] != '\\' && config_parent_dir[size-1] != '/')
        {
          strcat(config_dir,PATH_SEPARATOR);
        }
        strcat(config_dir,Config_SubDir);
        if (Directory_exists(config_dir))
        {
          // Répertoire trouvé, ok
          strcat(config_dir,PATH_SEPARATOR);
        }
        else
        {
          // Tentative de création
          if (!Create_ConfigDirectory(config_dir)) 
          {
            // Réussi
            strcat(config_dir,PATH_SEPARATOR);
          }
          else
          {
            // Echec: on se rabat sur le repertoire de l'executable.
            strcpy(config_dir,program_dir);
            #if !defined(__macosx__) && !defined(__gp2x__)
              strcat(config_dir, "../");
            #endif
          }
        }
      }
    }
  #endif
}
