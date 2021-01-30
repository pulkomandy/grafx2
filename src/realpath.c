/* vim:expandtab:ts=2 sw=2:
*/
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#if !defined(WIN32)
#include <limits.h>
#endif

#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < 1060)
#include "gfx2mem.h"
#endif

#if defined(__AROS__) || defined(__BEOS__) || defined(__MORPHOS__) || defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__) || defined(__amigaos__) || defined(__SWITCH__)

#include "io.h"
#include "gfx2log.h"
#include "gfx2mem.h"
// These platforms don't have realpath().
// Our custom implementation uses chdir() and relies on
// errno == ENOTDIR when trying to change directory to a file
char *Realpath(const char *_path)
{
  char * rpath = NULL;
  char * current_dir_save;

  // backup current directory
  current_dir_save = Get_current_directory(NULL, NULL, 0);
  if (current_dir_save == NULL) {
    // error
    return NULL;
  }

  if (chdir(_path) < 0) {
    if (errno == ENOTDIR) {
      const char * position;
      const char * filename;
      char * directory;
      position = Find_last_separator(_path);
      if (position != NULL) {
        size_t dirlen = position - _path;
        filename = position + 1;
        directory = GFX2_malloc(dirlen);
        if (directory != NULL) {
          memcpy(directory, _path, dirlen);
          directory[dirlen] = '\0';
          GFX2_Log(GFX2_DEBUG, "Directory : \"%s\", filename : \"%s\"\n", directory, filename);
          if (chdir(directory) == 0) {
            char * dirpart = Get_current_directory(NULL, NULL, 0);
            if (dirpart != NULL) {
              size_t len = strlen(dirpart) + strlen(filename) + 2;
              rpath = GFX2_malloc(len);
              if (rpath != NULL) {
                snprintf(rpath, len, "%s%s%s", dirpart, PATH_SEPARATOR, filename);
              }
              free(dirpart);
            }
          } else {
            GFX2_Log(GFX2_ERROR, "chdir(\"%s\") : %s\n", directory, strerror(errno));
          }
          free(directory);
        }
      }
    } else {
      GFX2_Log(GFX2_ERROR, "chdir(\"%s\") : %s\n", _path, strerror(errno));
    }
  } else {
    // _path is a directory
    rpath = Get_current_directory(NULL, NULL, 0);
  }

  // "restore" current directory
  chdir(current_dir_save);
  free(current_dir_save);
  return rpath;
}
            
#elif defined(__WIN32__) || defined(WIN32)
// MSVC / Mingw has a working equivalent. It only has reversed arguments.
char *Realpath(const char *_path)
{
  return _fullpath(NULL, _path, 260);
}
#else

// Use the stdlib function.
    char *Realpath(const char *_path)
    {
      /// POSIX 2004 states :
      /// If resolved_name is a null pointer, the behavior of realpath()
      /// is implementation-defined.
      ///
      /// but POSIX 2008 :
      /// If resolved_name is a null pointer, the generated pathname shall
      /// be stored as a null-terminated string in a buffer allocated as if
      /// by a call to malloc().
      ///
      /// So we assume all platforms now support passing NULL.
      /// If you find a platform where this is not the case,
      /// please add a new implementation with ifdef's.
      char * resolved_path = NULL;
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED < 1060)
      // realpath() accept NULL as 2nd argument since OSX 10.6
      resolved_path = GFX2_malloc(PATH_MAX);
      if (resolved_path == NULL)
        return NULL;
#endif
      return realpath(_path, resolved_path);
    }
#endif
