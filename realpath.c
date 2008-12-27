/*   Found in:
     http://amiga.sourceforge.net/amigadevhelp/FUNCTIONS/GeekGadgets/realpath/ex02_realpath.c
*/

#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

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

char *realpath(const char *_path, char *resolved_path)
{
	int fd = open(".", O_RDONLY), l;
	char current_dir_path[PATH_MAX];
	char path[PATH_MAX], lnk[PATH_MAX], *tmp = (char *)"";
	
	if (fd < 0) {
		return NULL;
	}
	getcwd(current_dir_path,PATH_MAX);
	strncpy(path, _path, PATH_MAX);
	
	if (chdir(path)) {
		if (errno == ENOTDIR) {
		  #if defined(__WIN32__)
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

