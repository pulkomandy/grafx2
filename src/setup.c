/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2014 Sergii Pylypenko
    Copyright 2011 Pawel Góralski
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
#if defined(__WIN32__) || defined(WIN32)
  #include <windows.h>
#ifdef _MSC_VER
  #include <direct.h>
#endif
#elif defined(__macosx__)
  #import <corefoundation/corefoundation.h>
  #import <sys/param.h>
#elif defined(__FreeBSD__)
  #include <sys/param.h>
#elif defined(__MINT__)
    #include <mint/osbind.h>
    #include <mint/sysbind.h>
#elif defined(__linux__)
  #include <limits.h>
  #include <unistd.h>
#elif defined(__HAIKU__)
  #include <FindDirectory.h>
#endif

#include "struct.h"
#include "io.h"
#include "setup.h"

#if defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
    // This is a random default value ...
    #define PATH_MAX 32768
#endif

int Create_ConfigDirectory(const char * config_dir)
{
  #if defined(__WIN32__) || defined(WIN32)
    return CreateDirectoryA(config_dir, NULL) ? 0 : -1;
  #else
    return mkdir(config_dir,S_IRUSR|S_IWUSR|S_IXUSR);
  #endif
}

// Determine which directory contains the executable.
// IN: Main's argv[0], some platforms need it, some don't.
// OUT: Write into program_dir. Trailing / or \ is kept.
// Note : in fact this is only used to check for the datafiles and fonts in 
// this same directory.
void Set_program_directory(const char * argv0, char * program_dir)
{
  // MacOSX
  #if defined(__macosx__)
    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    (void)argv0; // unused
    CFURLGetFileSystemRepresentation(url,true,(UInt8*)program_dir,MAXPATHLEN);
    CFRelease(url);
    // Append trailing slash
    strcat(program_dir    ,"/");
  
  // AmigaOS and alike: hard-coded volume name.
  #elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    (void)argv0; // unused
    strcpy(program_dir,"PROGDIR:");
  #elif defined(__MINT__)
  static char path[1024]={0};
  char currentDrive='A';
  (void)argv0; // unused
  currentDrive=currentDrive+Dgetdrv();
  
  Dgetpath(path,0);
  sprintf(program_dir,"%c:\%s",currentDrive,path);
  // Append trailing slash
  strcat(program_dir,PATH_SEPARATOR);
  #elif defined(__ANDROID__)
  (void)argv0; // unused
  getcwd(program_dir, MAX_PATH_CHARACTERS);
  strcat(program_dir, "/");
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
  // On GP2X, AROS and Android, executable is not in bin/
  #elif defined (__GP2X__) || defined (__gp2x__) || defined (__WIZ__) || defined (__CAANOO__) || defined(GCWZERO) || defined(__AROS__) || defined(__ANDROID__)
    strcat(data_dir,"share/grafx2/");
  //on tos, the same directory is used for everything
  #elif defined (__MINT__)
    strcpy(data_dir, program_dir);
  // Haiku provides us with an API to find it.
  #elif defined(__HAIKU__)
    if (find_path(Set_data_directory, B_FIND_PATH_DATA_DIRECTORY, "grafx2/", data_dir, PATH_MAX) != B_OK)
    {
      // If the program is not installed, find_path will fail. Try from local dir then.
      strcat(data_dir,"../share/grafx2/");
    }

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
  // AmigaOS4 provides the PROGIR: alias to the directory where the executable is.
  #if defined(__amigaos4__) || defined(__AROS__)
    strcpy(config_dir,"PROGDIR:");
  // GP2X
  #elif defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
    // On the GP2X, the program is installed to the sdcard, and we don't want to mess with the system tree which is
    // on an internal flash chip. So, keep these settings locals.
    strcpy(config_dir,program_dir);
  // For TOS we store everything in the program dir
  #elif defined(__MINT__)
    strcpy(config_dir,program_dir);
  // For all other platforms, there is some kind of settigns dir to store this.
  #else
    char filename[MAX_PATH_CHARACTERS];
    #ifdef GCWZERO
      strcpy(config_dir, "/media/home/.grafx2/");
    #else
      // In priority: check root directory
      strcpy(config_dir, program_dir);
    #endif
    // On all the remaining targets except OSX, the executable is in ./bin
    #if !defined(__macosx__)
      strcat(config_dir, "../");
    #endif
    strcpy(filename, config_dir);
    strcat(filename, CONFIG_FILENAME);

    if (!File_exists(filename))
    {
      char *config_parent_dir;
      #if defined(__WIN32__) || defined(WIN32)
        // "%APPDATA%\GrafX2"
        const char* Config_SubDir = "GrafX2";
        config_parent_dir = getenv("APPDATA");
      #elif defined(__BEOS__) || defined(__HAIKU__)
        // "`finddir B_USER_SETTINGS_DIRECTORY`/grafx2"
        const char* Config_SubDir = "grafx2";
        {
          static char parent[MAX_PATH_CHARACTERS];
          find_directory(B_USER_SETTINGS_DIRECTORY, 0, false, parent,
            MAX_PATH_CHARACTERS);
          config_parent_dir = parent;
        }
      #elif defined(__macosx__)
        // "~/Library/Preferences/com.googlecode.grafx2"
        const char* Config_SubDir = "Library/Preferences/com.googlecode.grafx2";
        config_parent_dir = getenv("HOME");
      #elif defined(__MINT__)
         const char* Config_SubDir = "";
         printf("GFX2.CFG not found in %s\n",filename);
         strcpy(config_parent_dir, config_dir);
      #else
         // ~/.config/grafx2
         const char* Config_SubDir;
         config_parent_dir = getenv("XDG_CONFIG_HOME");
         if (config_parent_dir)
           Config_SubDir = "grafx2";
         else {
            Config_SubDir = ".config/grafx2";
            config_parent_dir = getenv("HOME");
         }
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
            #if defined(__macosx__)
              strcat(config_dir, "../");
            #endif
          }
        }
      }
    }
  #endif
}
