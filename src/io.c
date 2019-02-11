/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2008 Yves Rizoud
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

// Fonctions de lecture/ecriture file, gèrent les systèmes big-endian et
// little-endian.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#define strdup _strdup
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    #include <proto/dos.h>
    #include <sys/types.h>
    #include <dirent.h>
#elif defined(WIN32)
#ifdef _MSC_VER
    #include <direct.h>
#else
    #include <dirent.h>
#endif
    #include <windows.h>
    //#include <commdlg.h>
#elif defined(__MINT__)
    #include <mint/osbind.h>
    #include <mint/sysbind.h>
    #include <dirent.h>
#else
    #include <dirent.h>
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_endian.h>
#endif

#include "struct.h"
#include "io.h"
#include "realpath.h"
#include "unicode.h"
#include "global.h"
#include "gfx2log.h"
#include "gfx2mem.h"
// for the network browse
#include "filesel.h"

// Lit un octet
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_byte(FILE *file, byte *dest)
{
  return fread(dest, 1, 1, file) == 1;
}
// Ecrit un octet
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_byte(FILE *file, byte b)
{
  return fwrite(&b, 1, 1, file) == 1;
}
// Lit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_bytes(FILE *file, void *dest, size_t size)
{
  return fread(dest, 1, size, file) == size;
}
// Read a line
// returns -1 if OK, 0 in case of error
int Read_byte_line(FILE *file, char *line, size_t size)
{
  return fgets(line, size, file) != NULL;
}
// Ecrit des octets
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_bytes(FILE *file, const void *src, size_t size)
{
  return fwrite(src, 1, size, file) == size;
}

// Lit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_word_le(FILE *file, word *dest)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  if (fread(dest, 1, sizeof(word), file) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *dest = SDL_Swap16(*dest);
  #endif
  return -1;
#else
  byte buffer[2];
  if (fread(buffer, 1, 2, file) != 2)
    return 0;
  *dest = (word)buffer[0] | (word)buffer[1] << 8;
  return -1;
#endif
}
// Ecrit un word (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_word_le(FILE *file, word w)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    w = SDL_Swap16(w);
  #endif
  return fwrite(&w, 1, sizeof(word), file) == sizeof(word);
#else
  if (fputc((w >> 0) & 0xff, file) == EOF)
    return 0;
  if (fputc((w >> 8) & 0xff, file) == EOF)
    return 0;
  return -1;
#endif
}
// Lit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_word_be(FILE *file, word *dest)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  if (fread(dest, 1, sizeof(word), file) != sizeof(word))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *dest = SDL_Swap16(*dest);
  #endif
  return -1;
#else
  byte buffer[2];
  if (fread(buffer, 1, 2, file) != 2)
    return 0;
  *dest = (word)buffer[0] << 8 | (word)buffer[1];
  return -1;
#endif
}
// Ecrit un word (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_word_be(FILE *file, word w)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    w = SDL_Swap16(w);
  #endif
  return fwrite(&w, 1, sizeof(word), file) == sizeof(word);
#else
  if (fputc((w >> 8) & 0xff, file) == EOF)
    return 0;
  if (fputc((w >> 0) & 0xff, file) == EOF)
    return 0;
  return -1;
#endif
}
// Lit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_dword_le(FILE *file, dword *dest)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  if (fread(dest, 1, sizeof(dword), file) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    *dest = SDL_Swap32(*dest);
  #endif
  return -1;
#else
  byte buffer[4];
  if (fread(buffer, 1, 4, file) != 4)
    return 0;
  *dest = (dword)buffer[0] | (dword)buffer[1] << 8 | (dword)buffer[2] << 16 | (dword)buffer[3] << 24;
  return -1;
#endif
}
// Ecrit un dword (little-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_dword_le(FILE *file, dword dw)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  #if SDL_BYTEORDER != SDL_LIL_ENDIAN
    dw = SDL_Swap32(dw);
  #endif
  return fwrite(&dw, 1, sizeof(dword), file) == sizeof(dword);
#else
  if (fputc((dw >> 0) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 8) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 16) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 24) & 0xff, file) == EOF)
    return 0;
  return -1;
#endif
}

// Lit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Read_dword_be(FILE *file, dword *dest)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  if (fread(dest, 1, sizeof(dword), file) != sizeof(dword))
    return 0;
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    *dest = SDL_Swap32(*dest);
  #endif
  return -1;
#else
  byte buffer[4];
  if (fread(buffer, 1, 4, file) != 4)
    return 0;
  *dest = (dword)buffer[0] << 24 | (dword)buffer[1] << 16 | (dword)buffer[2] << 8 | (dword)buffer[3];
  return -1;
#endif
}
// Ecrit un dword (big-endian)
// Renvoie -1 si OK, 0 en cas d'erreur
int Write_dword_be(FILE *file, dword dw)
{
#if defined(USE_SDL) || defined(USE_SDL2)
  #if SDL_BYTEORDER != SDL_BIG_ENDIAN
    dw = SDL_Swap32(dw);
  #endif
  return fwrite(&dw, 1, sizeof(dword), file) == sizeof(dword);
#else
  if (fputc((dw >> 24) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 16) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 8) & 0xff, file) == EOF)
    return 0;
  if (fputc((dw >> 0) & 0xff, file) == EOF)
    return 0;
  return -1;
#endif
}

// Détermine la position du dernier '/' ou '\\' dans une chaine,
// typiquement pour séparer le nom de file d'un chemin.
// Attention, sous Windows, il faut s'attendre aux deux car 
// par exemple un programme lancé sous GDB aura comme argv[0]:
// d:\Data\C\GFX2\grafx2/grafx2.exe
char * Find_last_separator(const char * str)
{
  const char * position = NULL;

  if (str == NULL)
    return NULL;
  for (; *str != '\0'; str++)
    if (*str == PATH_SEPARATOR[0]
#if defined(__WIN32__) || defined(WIN32)
     || *str == '/'
#elif __AROS__
     || *str == ':'
#endif
     )
      position = str;
  return (char *)position;
}

word * Find_last_separator_unicode(const word * str)
{
  const word * position = NULL;
  for (; *str != 0; str++)
    if (*str == (byte)PATH_SEPARATOR[0]
#if defined(__WIN32__) || defined(WIN32)
     || *str == '/'
#elif __AROS__
     || *str == ':'
#endif
     )
      position = str;
  return (word *)position;
}

char * Filepath_append_to_dir(const char * dir, const char * filename)
{
  char * path;
  size_t len = dir == NULL ? 0 : strlen(dir);
  if (len == 0) // no directory
    return strdup(filename);
  if (dir[len-1] == PATH_SEPARATOR[0]
#if defined(__WIN32__) || defined(WIN32)
     || dir[len-1] == '/'
#elif __AROS__
     || dir[len-1] == ':'
#endif
    )
  {
    len += strlen(filename) + 1;
    path = GFX2_malloc(len);
    if (path == NULL)
      return NULL;
    snprintf(path, len, "%s%s", dir, filename);
  }
  else
  {
    // need to add a path separator
    len += strlen(PATH_SEPARATOR) + strlen(filename) + 1;
    path = GFX2_malloc(len);
    if (path == NULL)
      return NULL;
    snprintf(path, len, "%s%s%s", dir, PATH_SEPARATOR, filename);
  }
  return path;
}

// Récupère la partie "nom de file seul" d'un chemin
char * Extract_filename(char *dest, const char *source)
{
  const char * position = Find_last_separator(source);

  if (dest != NULL)
  {
    if (position)
      strcpy(dest,position+1);
    else
      strcpy(dest,source);
    return dest;
  }
  else
  {
    if (position)
      return strdup(position + 1);
    else
      return strdup(source);
  }
}

// Récupère la partie "répertoire+/" d'un chemin.
char * Extract_path(char *dest, const char *source)
{
  char * position;
  char * path;

  path = Realpath(source, dest);
  if (path == NULL)
  {
    GFX2_Log(GFX2_ERROR, "Realpath(\"%s\", %p) failed !\n", source, dest);
    return NULL;
  }
  position = Find_last_separator(path);
  if (position)
    position[1] = '\0';
  else
  {
    size_t len = strlen(path);
    if (dest != NULL)
      strcpy(path + len, PATH_SEPARATOR);
    else
    {
      char * tmp = realloc(path, len + strlen(PATH_SEPARATOR) + 1);
      if (tmp != NULL)
      {
        path = tmp;
        strcpy(path + len, PATH_SEPARATOR);
      }
      else
      {
        GFX2_Log(GFX2_ERROR, "Extract_path(): Failed to realloc %lu bytes\n",
                 (unsigned long)(len + strlen(PATH_SEPARATOR) + 1));
      }
    }
  }
  return path;
}

///
/// Appends a file or directory name to an existing directory name.
/// As a special case, when the new item is equal to PARENT_DIR, this
/// will remove the rightmost directory name.
/// reverse_path is optional, if it's non-null, the function will
/// write there :
/// - if filename is ".." : The name of eliminated directory/file
/// - else: ".."
void Append_path(char *path, const char *filename, char *reverse_path)
{
  // Parent
  if (!strcmp(filename, PARENT_DIR))
  {
    // Going up one directory
    long len;
    char * separator_pos;

    // Remove trailing slash
    len=strlen(path);
    if (len && (!strcmp(path+len-1,PATH_SEPARATOR) 
    #ifdef __WIN32__
      || path[len-1]=='/'
    #endif
      ))
      path[len-1]='\0';
    
    separator_pos=Find_last_separator(path);
    if (separator_pos)
    {
      if (reverse_path)
        strcpy(reverse_path, separator_pos+1);
      #if defined(__AROS__)
      // Don't strip away the colon
      if (*separator_pos == ':') *(separator_pos+1)='\0';
      else *separator_pos='\0';
      #else
      *separator_pos='\0';
      #endif
    }
    else
    {
      if (reverse_path)
        strcpy(reverse_path, path);
      path[0]='\0';
    }
    #if defined(__WIN32__)
    // Roots of drives need a pending antislash
    if (path[0]!='\0' && path[1]==':' && path[2]=='\0')
    {
      strcat(path, PATH_SEPARATOR);
    }
    #endif
  }
  else
  // Sub-directory
  {
    long len;
    // Add trailing slash if needed
    len=strlen(path);
    if (len && (strcmp(path+len-1,PATH_SEPARATOR) 
    #ifdef __WIN32__
      && path[len-1]!='/'
    #elif __AROS__
      && path[len-1]!=':' // To avoid paths like volume:/dir
    #endif
      ))
    {
      strcpy(path+len, PATH_SEPARATOR);
      len+=strlen(PATH_SEPARATOR);
    }
    strcat(path, filename);
    
    if (reverse_path)
      strcpy(reverse_path, PARENT_DIR);
  }
}

int Position_last_dot(const char * fname)
{
  int pos_last_dot = -1;
  int c = 0;

  for (c = 0; fname[c] != '\0'; c++)
    if (fname[c] == '.')
      pos_last_dot = c;
  return pos_last_dot;
}

int Position_last_dot_unicode(const word * fname)
{
  int pos_last_dot = -1;
  int c = 0;

  for (c = 0; fname[c] != '\0'; c++)
    if (fname[c] == '.')
      pos_last_dot = c;
  return pos_last_dot;
}

int File_exists(const char * fname)
//   Détermine si un file passé en paramètre existe ou non dans le
// répertoire courant.
{
#if defined(WIN32)
  return (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(fname)) ? 0 : 1;
#else
    struct stat buf;
    int result;

    result=stat(fname,&buf);
    if (result!=0)
        return(errno!=ENOENT);
    else
        return 1;
#endif
}

int Directory_exists(const char * directory)
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
{
#if defined(WIN32)
  DWORD attr = GetFileAttributesA(directory);
  if (attr == INVALID_FILE_ATTRIBUTES)
    return 0;
  return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
#else
  DIR* entry;    // Structure de lecture des éléments

  if (strcmp(directory,PARENT_DIR)==0)
    return 1;
  else
  {
    //  On va chercher si le répertoire existe à l'aide d'un Opendir. S'il
    //  renvoie NULL c'est que le répertoire n'est pas accessible...

    entry=opendir(directory);
    if (entry==NULL)
        return 0;
    else
    {
        closedir(entry);
        return 1;
    }
  }
#endif
}

/// Check if a file or directory is hidden.
int File_is_hidden(const char *fname, const char *full_name)
{
#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__) || defined(__MINT__)|| defined(__SWITCH__)
  // False (unable to determine, or irrelevent for platform)
  (void)fname;//unused
  (void)full_name;//unused
  return 0;
#elif defined(__WIN32__)
  unsigned long att;
  if (full_name!=NULL)
    att = GetFileAttributesA(full_name);
  else
    att = GetFileAttributesA(fname);
  if (att==INVALID_FILE_ATTRIBUTES)
    return 0;
  return (att&FILE_ATTRIBUTE_HIDDEN)?1:0;
#else
  (void)full_name;//unused
   // On linux/unix (default), files are considered hidden if their name
   // begins with a .
   // As a special case, we'll consider 'parent directory' (..) never hidden.
  return fname[0]=='.' && strcmp(fname, PARENT_DIR);
#endif
}

// File size in bytes
unsigned long File_length(const char * fname)
{
#if defined(WIN32)
  WIN32_FILE_ATTRIBUTE_DATA infos;
  if (GetFileAttributesExA(fname, GetFileExInfoStandard, &infos))
  {
    return (unsigned long)(((DWORD64)infos.nFileSizeHigh << 32) + (DWORD64)infos.nFileSizeLow);
  }
  else
    return 0;
#else
  struct stat infos_fichier;
  if (stat(fname,&infos_fichier))
    return 0;
  return infos_fichier.st_size;
#endif
}

unsigned long File_length_file(FILE * file)
{
#if defined(WIN32)
  // revert to old school way of finding file size
  long offset_backup;
  long file_length;
  offset_backup = ftell(file);
  if (offset_backup < 0)
    return 0;
  if (fseek(file, 0, SEEK_END) < 0)
    return 0;
  file_length = ftell(file);
  if (file_length < 0)
    file_length = 0;
  fseek(file, offset_backup, SEEK_SET);
  return (unsigned long)file_length;
#else
  struct stat infos_fichier;
  if (fstat(fileno(file),&infos_fichier))
      return 0;
  return infos_fichier.st_size;
#endif
}

void For_each_file(const char * directory_name, void Callback(const char *, const char *))
{
#if defined(WIN32)
  WIN32_FIND_DATAA fd;
  char * full_filename;
  char * search_string;
  HANDLE h;

  full_filename = Realpath(directory_name, NULL);
  search_string = Filepath_append_to_dir((full_filename != NULL) ? full_filename : directory_name, "*");
  free(full_filename);
  h = FindFirstFileA(search_string, &fd);
  free(search_string);
  if (h != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;
      full_filename = Filepath_append_to_dir(directory_name, fd.cFileName);
      Callback(full_filename, fd.cFileName);
      free(full_filename);
    }
    while (FindNextFileA(h, &fd));
    FindClose(h);
  }
#else
  // directory traversal
  DIR*  current_directory;
  struct dirent* entry; // directory entry

  current_directory = opendir(directory_name);
  if(current_directory == NULL)
    return;        // Invalid directory
  while ((entry = readdir(current_directory)) != NULL)
  {
    char * full_filename;
    struct stat st;

    full_filename = Filepath_append_to_dir(directory_name, entry->d_name);
    // d_name is the only field you can count on in all POSIX systems.
    // Also we need to call stat() in order to follow symbolic links
    if (stat(full_filename, &st) < 0)
      GFX2_Log(GFX2_WARNING, "stat(\"%s\") failed\n", full_filename);
    else
    {
      if (S_ISREG(st.st_mode))
        Callback(full_filename, entry->d_name);
    }
    free(full_filename);
  }
  closedir(current_directory);
#endif
}

/// Scans a directory, calls Callback for each file or directory in it,
void For_each_directory_entry(const char * directory_name, void * pdata, T_File_dir_cb Callback)
{
#if defined(WIN32)
  WIN32_FIND_DATAW fd;
  size_t len;
  word * search_string;
  HANDLE h;

  len = strlen(directory_name) + 3;
  search_string = (word *)GFX2_malloc(sizeof(word) * len);
  if (search_string == NULL)
    return;
  Unicode_char_strlcpy(search_string, directory_name, len);
  Unicode_char_strlcat(search_string, "\\*", len);
  h = FindFirstFileW((WCHAR *)search_string, &fd);
  free(search_string);
  if (h != INVALID_HANDLE_VALUE)
  {
    do
    {
      int i;
      char short_filename[16];
      if (fd.cAlternateFileName[0] != 0)
        for (i = 0; fd.cAlternateFileName[i] != 0 && i < (int)sizeof(short_filename) - 1; i++)
          short_filename[i] = (char)fd.cAlternateFileName[i];
      else  // normal name is short !
        for (i = 0; fd.cFileName[i] != 0 && i < (int)sizeof(short_filename) - 1; i++)
          short_filename[i] = (char)fd.cFileName[i];
      short_filename[i] = '\0';
      Callback(
        pdata,
        short_filename,
        (const word *)fd.cFileName,
        (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : 1,
        (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0,
        (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? 1 : 0
        );
    }
    while (FindNextFileW(h, &fd));

    FindClose(h);
  }
#else
  DIR*  current_directory; // current directory
  struct dirent* entry;    // directory entry struct

  current_directory = opendir(directory_name);
  if(current_directory == NULL)
    return;        // Invalid directory

  while ((entry = readdir(current_directory)) != NULL)
  {
    word * unicode_filename = NULL;
    char * full_filename;
    struct stat st;
#ifdef ENABLE_FILENAMES_ICONV
    if (cd_utf16 != (iconv_t)-1)
    {
      char * input = entry->d_name;
      size_t inbytesleft = strlen(entry->d_name);
      char * output;
      size_t outbytesleft;
      size_t r;

      unicode_filename = GFX2_malloc(sizeof(word) * (inbytesleft + 1));
      if (unicode_filename != NULL)
      {
        output = (char *)unicode_filename;
        outbytesleft = sizeof(word) * inbytesleft;
        r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
        if (r != (size_t)-1)
        {
          output[0] = '\0';
          output[1] = '\0';
        }
        else
        {
          free(unicode_filename);
          unicode_filename = NULL;
        }
      }
    }
#endif
    full_filename = Filepath_append_to_dir(directory_name, entry->d_name);
    if (stat(full_filename, &st) < 0)
      GFX2_Log(GFX2_WARNING, "stat(\"%s\") failed\n", full_filename);
    else
    {
      Callback(
        pdata,
        entry->d_name,
        unicode_filename,
        S_ISREG(st.st_mode),
        S_ISDIR(st.st_mode),
        File_is_hidden(entry->d_name, full_filename));
    }
    free(full_filename);
    free(unicode_filename);
  }
  closedir(current_directory);
#endif
}


/**
 * Convert a file name to unicode characters
 *
 * If the parametter is null, the buffer is malloc'ed
 *
 * @param filename_unicode the output buffer of MAX_PATH_CHARACTERS wide characters
 * @param filename the input file name
 * @param directory the input file directory
 * @return NULL if no conversion has taken place.
 * @return filename_unicode if the unicode filename has been retrieved
 */
word * Get_Unicode_Filename(word * filename_unicode, const char * filename, const char * directory)
{
#if defined(WIN32)
  int i = 0;
  DWORD len;
  WCHAR * shortPath;
  WCHAR * longPath;
  WCHAR * sep;

  shortPath = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * (strlen(filename) + strlen(directory) + 2));
  if (shortPath == NULL)
    return NULL;
  // copy the full path to a wide character buffer :
  while (directory[0] != '\0')
    shortPath[i++] = *directory++;
  if (i > 0 && shortPath[i-1] != '\\')   // add path separator only if it is missing
    shortPath[i++] = '\\';
  while (filename[0] != '\0')
    shortPath[i++] = *filename++;
  shortPath[i++] = 0;
  len = GetLongPathNameW(shortPath, NULL, 0);
  if (len == 0)
  {
    free(shortPath);
    return NULL;
  }
  longPath = (WCHAR *)GFX2_malloc(len * sizeof(WCHAR));
  if (longPath == NULL || GetLongPathNameW(shortPath, longPath, len) == 0)
  {
    free(shortPath);
    return NULL;
  }
  free(shortPath);
  sep = wcsrchr(longPath, '\\');
  if (sep == NULL)
  {
    if (filename_unicode == NULL)
      return (word *)longPath;
    memcpy(filename_unicode, longPath, sizeof(word) * len);
  }
  else
  {
    sep++;
    len = wcslen(sep) + 1;
    if (filename_unicode == NULL)
      filename_unicode = (word *)GFX2_malloc(sizeof(word) * len);
    if (filename_unicode != NULL)
      memcpy(filename_unicode, sep, sizeof(word) * len);
  }
  free(longPath);
  return filename_unicode;
#elif defined(ENABLE_FILENAMES_ICONV)
  int allocated_memory = 0;
  char * input = (char *)filename;
  size_t inbytesleft = strlen(filename);
  char * output;
  size_t outbytesleft;
  size_t r;

  (void)directory;  // unused
  if (cd_utf16 == (iconv_t)-1)
    return NULL;
  if (filename_unicode == NULL)
  {
    filename_unicode = GFX2_malloc(sizeof(word) * (inbytesleft + 1));
    if (filename_unicode == NULL)
      return NULL;
    allocated_memory = 1;
    outbytesleft = inbytesleft * 2;
  }
  else
    outbytesleft = (MAX_PATH_CHARACTERS - 1) * 2;
  output = (char *)filename_unicode;

  r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
  if (r != (size_t)-1)
  {
    output[0] = '\0';
    output[1] = '\0';
    return filename_unicode;
  }
  if (allocated_memory)
    free(filename_unicode);
  return NULL;
#else
  (void)filename_unicode;
  (void)filename;
  (void)directory;
  // not implemented
  return NULL;
#endif
}

/// Lock file used to prevent several instances of grafx2 from harming each others' backups
#ifdef WIN32
HANDLE Lock_file_handle = INVALID_HANDLE_VALUE;
#else
int Lock_file_handle = -1;
#endif

#define GFX2_LOCK_FILENAME "gfx2.lck"

byte Create_lock_file(const char *file_directory)
{
  #if defined (__amigaos__)||(__AROS__)||(__ANDROID__)
    #warning "Missing code for your platform, please check and correct!"
  #elif defined(__SWITCH__)
    // The switch can only run one application at a time, so we don't do anything special here
  #else
  char * lock_filename;
  
#ifdef GCWZERO
  lock_filename = Filepath_append_to_dir("/media/home/.grafx2/", GFX2_LOCK_FILENAME);
#else
  lock_filename = Filepath_append_to_dir(file_directory, GFX2_LOCK_FILENAME);
#endif
  
  #ifdef WIN32
  // Windowzy method for creating a lock file
  Lock_file_handle = CreateFileA(
    lock_filename,
    GENERIC_WRITE,
    0, // No sharing
    NULL,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  free(lock_filename);
  if (Lock_file_handle == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  #else
  // Unixy method for lock file
  Lock_file_handle = open(lock_filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
  free(lock_filename);
  if (Lock_file_handle == -1)
  {
    // Usually write-protected media
    return -1;
  }
  if (lockf(Lock_file_handle, F_TLOCK, 0)==-1)
  {
    close(Lock_file_handle);
    // Usually write-protected media
    return -1;
  }
  #endif
  #endif // __amigaos__ or __AROS__
  return 0;
}

void Release_lock_file(const char *file_directory)
{
  char * lock_filename;
    
  #ifdef WIN32
  if (Lock_file_handle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(Lock_file_handle);
  }
  #else
  if (Lock_file_handle != -1)
  {
    close(Lock_file_handle);
    Lock_file_handle = -1;
  }  
  #endif
  
  // Actual deletion
#ifdef GCWZERO
  lock_filename = Filepath_append_to_dir("/media/home/.grafx2/", GFX2_LOCK_FILENAME);
#else
  lock_filename = Filepath_append_to_dir(file_directory, GFX2_LOCK_FILENAME);
#endif
  Remove_path(lock_filename);
  free(lock_filename);
}

char * Get_current_directory(char * buf, word * * unicode, size_t size)
{
#if defined(__MINT__)
  if (buf == NULL)
  {
    buf = GFX2_malloc(MAX_PATH_CHARACTERS);
    if (buf == NULL)
      return NULL;
  }
  buf[0] = 'A'+Dgetdrv();
  buf[1] = ':';
  buf[2] = '\\';
  Dgetpath(buf+3,0);
  strcat(buf,PATH_SEPARATOR);

  if (unicode != NULL)
    *unicode = NULL; // no unicode support

  return buf;
#elif defined(WIN32)
  WCHAR * cur_dir, * short_dir;
  size_t size_cur, size_short;
  size_t i;

  // first get the current directory in unicode
  size_cur = (size_t)GetCurrentDirectoryW(0, NULL);
  if (size_cur == 0)
  {
    GFX2_Log(GFX2_ERROR, "GetCurrentDirectoryW(0, NULL) failed !\n");
    return NULL;
  }
  cur_dir = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * size_cur);
  if (cur_dir == NULL)
    return NULL;
  if (GetCurrentDirectoryW(size_cur, cur_dir) == 0)
  {
    GFX2_Log(GFX2_ERROR, "GetCurrentDirectoryW(%u, %p) failed !\n", (unsigned)size_cur, cur_dir);
    return NULL;
  }

  // convert to "short" path (ie C:\PROGRA~1\...)
  size_short = (size_t)GetShortPathNameW(cur_dir, NULL, 0);
  if (size_short == 0)
  {
    GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, NULL, 0) failed !\n", cur_dir);
    free(cur_dir);
    return NULL;
  }
  short_dir = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * size_short);
  if (short_dir == NULL)
  {
    free(cur_dir);
    return NULL;
  }
  if (GetShortPathNameW(cur_dir, short_dir, size_short) == 0)
  {
    GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, %p, %u) failed !\n", cur_dir, short_dir, (unsigned)size_short);
    free(cur_dir);
    free(short_dir);
    return NULL;
  }

  // now copy / return the path
  if (buf == NULL)
  {
    size = size_short;
    buf = (char *)GFX2_malloc(size);
    if (buf == NULL)
    {
      free(cur_dir);
      free(short_dir);
      return NULL;
    }
  }
  for (i = 0; i < (size - 1) && short_dir[i]; i++)
    buf[i] = (char)short_dir[i];
  buf[i] = '\0';
  free(short_dir);

  if (unicode != NULL)
  {
    WCHAR * long_dir;
    size_t size_long;

    *unicode = NULL;
    // convert to "long" path for display
    size_long = (size_t)GetLongPathNameW(cur_dir, NULL, 0);
    if (size_long == 0)
      GFX2_Log(GFX2_ERROR, "GetLongPathNameW(%p, NULL, 0) failed !\n", cur_dir);
    else
    {
      long_dir = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * size_long);
      if (long_dir != NULL)
      {
        if (GetLongPathNameW(cur_dir, long_dir, size_long) == 0)
        {
          GFX2_Log(GFX2_ERROR, "GetLongPathNameW(%p, %p, %u) failed !\n", cur_dir, long_dir, (unsigned)size_long);
          free(long_dir);
        }
        else
          *unicode = (word *)long_dir;
      }
    }
  }
  free(cur_dir);
  return buf;
#else
  char * ret = getcwd(buf, size);
  if (ret == NULL)
    GFX2_Log(GFX2_ERROR, "getcwd(%p, %lu) failed !\n", buf, (unsigned long)size);
#ifdef ENABLE_FILENAMES_ICONV
  if (ret != NULL && unicode != NULL)
  {
    char * input = ret;
    size_t inbytesleft = strlen(ret);
    word * buf_unicode = GFX2_malloc((inbytesleft + 1) * 2);
    char * output = (char *)buf_unicode;
    size_t outbytesleft = 2 * inbytesleft;
    if (cd_utf16 != (iconv_t)-1 && buf_unicode != NULL)
    {
      size_t r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
      if (r != (size_t)-1)
      {
        output[0] = '\0';
        output[1] = '\0';
        *unicode = buf_unicode;
      }
      else
        free(buf_unicode);
    }
  }
#else
  if (unicode != NULL)
    *unicode = NULL; // no unicode support
#endif
  return ret;
#endif
}

int Change_directory(const char * path)
{
  GFX2_Log(GFX2_DEBUG, "Change_directory(\"%s\")\n", path);
#if defined(__WIN32__) || defined(WIN32)
  return (SetCurrentDirectoryA(path) ? 0 : -1);
#else
  return chdir(path);
#endif
}

int Remove_path(const char * path)
{
#if defined(WIN32)
  return (DeleteFileA(path) ? 0 : -1);
#elif defined(__linux__)
  return unlink(path);
#else
  return remove(path);
#endif
}

///
/// Remove the directory
int Remove_directory(const char * path)
{
#if defined(WIN32)
  return RemoveDirectoryA(path) ? 0 : -1;
#else
  return rmdir(path);
#endif
}

///
/// Calculate relative path
char * Calculate_relative_path(const char * ref_path, const char * path)
{
  char * real_ref_path;
  char * rel_path = NULL;
  int last_separator = -1;
  int i;
  int separator_count = 0;
  size_t len;

  if (ref_path == NULL || path == NULL)
    return NULL;
  real_ref_path = Realpath(ref_path, NULL);
  if (real_ref_path == NULL)
    real_ref_path = strdup(ref_path);
#if defined(WIN32) || defined(__MINT__)
  if (real_ref_path[1] == ':' && path[1] == ':')
  {
    // use same case for drive letter
    real_ref_path[0] = (real_ref_path[0] & ~32) | (path[0] & 32);
    if (real_ref_path[0] != path[0])
    {
      free(real_ref_path);
      return NULL;  // path on different volumes, not possible
    }
  }
#endif
  // look for common path parts
  for (i = 0; real_ref_path[i] == path[i] && path[i] != '\0'; i++)
  {
    if (path[i] == PATH_SEPARATOR[0])
      last_separator = i;
#if defined(WIN32)
    else if(path[i] == '/')
      last_separator = i;
#endif
  }
  // at this point, all chars from index 0 to i-1 are identical in
  // real_ref_path and path.
  // real_ref_path[i] and path[i] are either different, or both '\0'
  if (real_ref_path[i] == PATH_SEPARATOR[0] && real_ref_path[i + 1] == '\0' && path[i] == '\0')
  {
    free(real_ref_path);
    return strdup("."); // path are identical (real_ref_path has additional trailing separator)
  }
  if (real_ref_path[i] == '\0')
  {
    if (path[i] == '\0')
    {
      free(real_ref_path);
      return strdup("."); // path are identical
    }
    // path is under ref_path
    if (path[i] == PATH_SEPARATOR[0])
    {
      free(real_ref_path);
      len = strlen(path + i) + 1;
      rel_path = GFX2_malloc(len + 1);
      if (rel_path != NULL)
        snprintf(rel_path, len, ".%s", path + i);
      return rel_path;
    }
    else if (i > 0 && real_ref_path[i - 1] == PATH_SEPARATOR[0])
    {
      free(real_ref_path);
      len = strlen(path + i - 1) + 1;
      rel_path = GFX2_malloc(len + 1);
      if (rel_path != NULL)
        snprintf(rel_path, len, ".%s", path + i - 1);
      return rel_path;
    }
  }
  if (last_separator <= 0)
  {
    free(real_ref_path);
    return strdup(path);  // no common part found return absolute path
  }
  // count the number of path separators in the reference path
  for (i = last_separator; real_ref_path[i] != '\0'; i++)
  {
    if (real_ref_path[i] == PATH_SEPARATOR[0] && real_ref_path[i + 1] != '\0')  // do not count the trailing separator
      separator_count++;
  }
  free(real_ref_path);
  i = 0;
  // construct the relative path
  len = separator_count * (2 + strlen(PATH_SEPARATOR)) + strlen(path + last_separator + 1) + 1;
  rel_path = GFX2_malloc(len + 1);
  if (rel_path != NULL)
  {
    while(separator_count-- > 0)
      i += snprintf(rel_path + i, len + 1 - i, "..%s", PATH_SEPARATOR);
    strncpy(rel_path + i, path + last_separator + 1, len + 1 - i);
    rel_path[len] = '\0';
  }
  return rel_path;
}

#if defined(WIN32)
static void Enumerate_Network_R(T_Fileselector *list, LPNETRESOURCEA lpnr)
{
  // Mpr.lib
  HANDLE hEnum;
  DWORD r;
  r = WNetOpenEnumA (RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, lpnr, &hEnum);
  if (r == NO_ERROR)
  {
    DWORD buffer_size = 16*1024;
    DWORD count = -1;
    LPNETRESOURCEA lpnrLocal = (LPNETRESOURCEA) GlobalAlloc(GPTR, buffer_size);
    do
    {
      ZeroMemory(lpnrLocal, buffer_size);
      r = WNetEnumResourceA(hEnum, &count, lpnrLocal, &buffer_size);
      if (r == NO_ERROR)
      {
        DWORD i;
        for (i = 0 ; i < count; i++)
        {
          GFX2_Log(GFX2_DEBUG, "%08x %08x %08x %s %s %s %s\n",
            lpnrLocal[i].dwType, lpnrLocal[i].dwDisplayType,
            lpnrLocal[i].dwUsage,
            lpnrLocal[i].lpProvider, lpnrLocal[i].lpLocalName,
            lpnrLocal[i].lpRemoteName, lpnrLocal[i].lpComment);
          if (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER)
          {
            Enumerate_Network_R(list, &lpnrLocal[i]);
          }
          if (lpnrLocal[i].dwType == RESOURCETYPE_DISK &&
            lpnrLocal[i].dwDisplayType == RESOURCEDISPLAYTYPE_SHARE)
          {
            Add_element_to_list(list, lpnrLocal[i].lpRemoteName,
                    Format_filename(lpnrLocal[i].lpRemoteName, 19-1, FSOBJECT_DRIVE),
                    FSOBJECT_DRIVE, ICON_NETWORK);
            list->Nb_directories++;
          }
        }
      }
      else
      {
      }
    }
    while (0);
    GlobalFree((HGLOBAL) lpnrLocal);
    WNetCloseEnum(hEnum);
  }
}

void Enumerate_Network(T_Fileselector *list)
{
  Enumerate_Network_R(list, NULL);
}
#endif
