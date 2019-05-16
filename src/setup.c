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
  #define strdup _strdup
  #if _MSC_VER < 1900
    #define snprintf _snprintf
  #endif
#endif
#elif defined(__macosx__)
  #import <CoreFoundation/CoreFoundation.h>
  #import <sys/param.h>
#elif defined(__FreeBSD__)
  #include <sys/param.h>
#elif defined(__MINT__)
    #include <mint/osbind.h>
    #include <mint/sysbind.h>
#elif defined(__linux__)
  #include <limits.h>
  #include <unistd.h>
  #include <errno.h>
#elif defined(__HAIKU__)
  #include <FindDirectory.h>
#endif

#include "struct.h"
#include "io.h"
#include "setup.h"
#include "global.h"
#include "gfx2log.h"

#ifndef PATH_MAX
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
// OUT: program_dir. Trailing / or \ is kept.
// Note : in fact this is only used to check for the datafiles and fonts in 
// this same directory.
char * Get_program_directory(const char * argv0)
{
  char * program_dir;
  // MacOSX
  #if defined(__macosx__)
    program_dir = malloc(MAXPATHLEN);
    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    (void)argv0; // unused
    CFURLGetFileSystemRepresentation(url,true,(UInt8*)program_dir,MAXPATHLEN);
    CFRelease(url);
    // Append trailing slash
    strcat(program_dir    ,"/");
  
  // AmigaOS and alike: hard-coded volume name.
  #elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    (void)argv0; // unused
    program_dir = strdup("PROGDIR:");
  #elif defined(__MINT__)
  static char path[1024]={0};
  char currentDrive;

  (void)argv0; // unused
  currentDrive = 'A' + Dgetdrv();
  Dgetpath(path, 0);
  program_dir = malloc(4 + strlen(path) + 1);
  sprintf(program_dir,"%c:\\%s%s", currentDrive, path, PATH_SEPARATOR);
  #elif defined(__ANDROID__)
  (void)argv0; // unused
  progam_dir = malloc(MAX_PATH_CHARACTERS);
  getcwd(program_dir, MAX_PATH_CHARACTERS);
  strcat(program_dir, "/");
  // Linux: argv[0] unreliable
  #elif defined(__linux__)
  if (argv0[0]!='/')
  {
    ssize_t path_len;
    char path[PATH_MAX];
    path_len = readlink("/proc/self/exe", path, sizeof(path));
    if (path_len >= 0)
    {
      path[path_len] = '\0';  // add null terminating char
      GFX2_Log(GFX2_DEBUG, "binary path resolved to : %s\n", path);
      program_dir = Extract_path(NULL, path);
    }
    else
      GFX2_Log(GFX2_WARNING, "readlink(%s) failed : %s\n", "/proc/self/exe", strerror(errno));
  }
  else
    program_dir = Extract_path(NULL, argv0);
  
  // Others: The part of argv[0] before the executable name.    
  // Keep the last \ or /.
  // On Windows, Mingw32 already provides the full path in all cases.
  #else
  program_dir = Extract_path(NULL, argv0);
  #endif
  return program_dir;
}

/// Determine which directory contains the read-only data.
/// @param program_dir The directory containing the executable
/// @return the path. Trailing / or \ is kept.
char * Get_data_directory(const char * program_dir)
{
  const char * to_append;
  // On all platforms, data is relative to the executable's directory
  #if defined(__macosx__)
    // On MacOSX,  it is stored in a special folder:
    to_append = "Contents/Resources/";
  #elif defined (__GP2X__) || defined (__gp2x__) || defined (__WIZ__) || defined (__CAANOO__) || defined(GCWZERO) || defined(__AROS__) || defined(__ANDROID__)
    // On GP2X, AROS and Android, executable is not in bin/
    to_append = "data/";
  #elif defined (__MINT__)
    //on tos, the same directory is used for everything
    return strdup(program_dir);
  #elif defined(__SWITCH__)
    //on switch, we store everything in the SD card in /switch/grafx2
    return strdup("/switch/grafx2/");
  #elif defined(__HAIKU__)
    // Haiku provides us with an API to find it.
    char * data_dir = malloc(PATH_MAX);
    if (find_path(Get_data_directory, B_FIND_PATH_DATA_DIRECTORY, "grafx2/", data_dir, PATH_MAX) == B_OK)
    {
      return data_dir;
    }
    else
    {
      // If the program is not installed, find_path will fail. Try from local dir then.
      free(data_dir);
      to_append = "../share/grafx2/";
    }

  #elif defined(WIN32)
    to_append = "..\\share\\grafx2\\";
  #else
    // All other targets, program is in a "bin" subdirectory
    to_append = "../share/grafx2/";
  #endif

  return Filepath_append_to_dir(program_dir, to_append);
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
char * Get_config_directory(const char * program_dir)
{
  // AmigaOS4 provides the PROGDIR: alias to the directory where the executable is.
  #if defined(__amigaos4__) || defined(__AROS__)
    return strdup("PROGDIR:");
  // GP2X
  #elif defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__)
    // On the GP2X, the program is installed to the sdcard, and we don't want to mess with the system tree which is
    // on an internal flash chip. So, keep these settings locals.
    return strdup(program_dir);
  // For TOS we store everything in the program dir
  #elif defined(__MINT__)
    return strdup(program_dir);
  //on switch, we store everything in the SD card in /switch/grafx2
  #elif defined(__SWITCH__)
    return strdup("/switch/grafx2/");
  // For all other platforms, there is some kind of settings dir to store this.
  #else
    char * config_dir;
    size_t len;
    char * filename;
    #ifdef GCWZERO
      config_dir = strdup("/media/home/.grafx2/");
    #elif defined(__macosx__)
      // On all the remaining targets except OSX, the executable is in ./bin
      config_dir = strdup(program_dir);
    #else
      len = strlen(program_dir) + 2 + strlen(PATH_SEPARATOR) + 1;
      config_dir = malloc(len);
      snprintf(config_dir, len, "%s%s", program_dir, ".." PATH_SEPARATOR);
    #endif

    filename = Filepath_append_to_dir(config_dir, CONFIG_FILENAME);

    if (File_exists(filename))
    {
      // gfx2.cfg found, this is a portable installation
      Portable_Installation_Detected = 1;
    }
    else
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
         config_parent_dir = strdup(config_dir);
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
      Portable_Installation_Detected = 0;

      if (config_parent_dir && config_parent_dir[0]!='\0')
      {
        size_t size = strlen(config_parent_dir);
        free(config_dir);
        len = size + strlen(Config_SubDir) + strlen(PATH_SEPARATOR) + 1;
        if (config_parent_dir[size-1] != '\\' && config_parent_dir[size-1] != '/')
        {
          len += strlen(PATH_SEPARATOR);
          config_dir = malloc(len);
          snprintf(config_dir, len, "%s%s%s%s",
                   config_parent_dir, PATH_SEPARATOR,
                   Config_SubDir, PATH_SEPARATOR);
        }
        else
        {
          config_dir = malloc(len);
          snprintf(config_dir, len, "%s%s%s",
                   config_parent_dir,
                   Config_SubDir, PATH_SEPARATOR);
        }
        if (!Directory_exists(config_dir))
        {
          // try to create it
          if (Create_ConfigDirectory(config_dir) < 0)
          {
            GFX2_Log(GFX2_WARNING, "Failed to create directory \"%s\"\n", config_dir);
            // Echec: on se rabat sur le repertoire de l'executable.
            Portable_Installation_Detected = 1;
            free(config_dir);
            #if defined(__macosx__)
              len = strlen(program_dir) + 4;
              config_dir = malloc(len);
              snprintf(config_dir, len, "%s%s", program_dir, "../");
            #else
              config_dir = strdup(program_dir);
            #endif
          }
            GFX2_Log(GFX2_INFO, "\"%s\" directory created.\n", config_dir);
        }
      }
    }
    free(filename);
    return config_dir;
  #endif
}
