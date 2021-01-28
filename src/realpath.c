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
// These platforms don't have realpath().
char *Realpath(const char *_path)
{
  return strdup(_path);
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
