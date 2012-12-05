/* vim:expandtab:ts=2 sw=2:
*/
#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#if defined(__AROS__) || defined(__linux__) || defined(__GLIBC__)|| defined(__MINT__)
#include <limits.h>
#endif

#if defined(__AROS__) || defined(__BEOS__) || defined(__MORPHOS__) || defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__) || defined(__amigaos__)
// These platforms don't have realpath().
// We use the following implementation, found in:
// http://amiga.sourceforge.net/amigadevhelp/FUNCTIONS/GeekGadgets/realpath/ex02_realpath.c
//
// When tested on Debian, this piece of code doesn't resolve
// symbolic link in the filename itself, only on the directories in
// the path. So this implementation is limited, it's really better to
// use realpath() if your platform has it.
  
    #if defined(__GP2X__) || defined(__WIZ__) || defined(__CAANOO__) || defined(__amigaos__)
        // This is a random default value ...
        #define PATH_MAX 32768
    #endif
  
    static char *sep(char *path)
    {
        char *tmp, c;
        
        tmp = strrchr(path, '/');
        if(tmp) {
            c = tmp[1];
            tmp[1] = 0;
            if (chdir(path)) {
                return NULL;
            }
            tmp[1] = c;
            
            return tmp + 1;
        }
        return path;
    }

	// Find the real path of _path by chdir to it and then getcwd.
	// If resolved_path is null, it is allocated.
    char *Realpath(const char *_path, char *resolved_path)
    {
        #if defined(__AROS__)
        int fd = open("", O_RDONLY); // GrafX2 is compiled without Unix support
        #else
        int fd = open(".", O_RDONLY);
        #endif
        int l;
        char current_dir_path[PATH_MAX];
        char path[PATH_MAX], lnk[PATH_MAX], *tmp = (char *)"";
        
        if (fd < 0) {
            return NULL;
        }
        getcwd(current_dir_path,PATH_MAX);
        strncpy(path, _path, PATH_MAX);
        
        if (chdir(path)) {
            if (errno == ENOTDIR) {
                #if defined(__WIN32__) || defined(__MORPHOS__) || defined(__amigaos__)
                    // No symbolic links and no readlink()
                    l = -1;
                #else
                    l = readlink(path, lnk, PATH_MAX);
                #endif
                if (!(tmp = sep(path))) {
                    resolved_path = NULL;
                    goto abort;
                }
                if (l < 0) {
                    if (errno != EINVAL) {
                        resolved_path = NULL;
                        goto abort;
                    }
                } else {
                    lnk[l] = 0;
                    if (!(tmp = sep(lnk))) {
                        resolved_path = NULL;
                        goto abort;
                    }
                }
            } else {
                resolved_path = NULL;
                goto abort;
            }
        }
        
        if(resolved_path==NULL) // if we called realpath with null as a 2nd arg
            resolved_path = (char*) malloc( PATH_MAX );
                
        if (!getcwd(resolved_path, PATH_MAX)) {
            resolved_path = NULL;
            goto abort;
        }
        
        if(strcmp(resolved_path, "/") && *tmp) {
            strcat(resolved_path, "/");
        }
        
        strcat(resolved_path, tmp);
      abort:
        chdir(current_dir_path);
        close(fd);
        return resolved_path;
    }
            
#elif defined (__WIN32__)
// Mingw has a working equivalent. It only has reversed arguments.
    char *Realpath(const char *_path, char *resolved_path)
    {
        return _fullpath(resolved_path,_path,260);
    }
#else
// Use the stdlib function.
    char *Realpath(const char *_path, char *resolved_path)
    {
		// While linux version of realpath handles the resolved_path being a
		// null pointer, this is not the case for other platforms (Haiku), nor
		// specified by the open group in POSIX. So, be safe and allocate
		// ourselves.
        if(resolved_path==NULL) // if we called realpath with null as a 2nd arg
            resolved_path = (char*) malloc( PATH_MAX );
        return realpath(_path, resolved_path);
    }
#endif


