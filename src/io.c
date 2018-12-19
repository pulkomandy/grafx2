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
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
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

// Récupère la partie "nom de file seul" d'un chemin
void Extract_filename(char *dest, const char *source)
{
  const char * position = Find_last_separator(source);

  if (position)
    strcpy(dest,position+1);
  else
    strcpy(dest,source);
}
// Récupère la partie "répertoire+/" d'un chemin.
void Extract_path(char *dest, const char *source)
{
  char * position=NULL;

  Realpath(source,dest);
  position = Find_last_separator(dest);
  if (position)
    *(position+1) = '\0';
  else
    strcat(dest, PATH_SEPARATOR);
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
#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__) || defined(__MINT__)
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
  char full_filename[MAX_PATH_CHARACTERS];
#if defined(WIN32)
  WIN32_FIND_DATAA fd;
  char search_string[MAX_PATH_CHARACTERS];
  HANDLE h;

  if (Realpath(directory_name, full_filename))
    _snprintf(search_string, sizeof(search_string), "%s\\*", full_filename);
  else
    _snprintf(search_string, sizeof(search_string), "%s\\*", directory_name);
  h = FindFirstFileA(search_string, &fd);
  if (h != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;
      _snprintf(full_filename, sizeof(full_filename), "%s\\%s", directory_name, fd.cFileName);
      Callback(full_filename, fd.cFileName);
    }
    while (FindNextFileA(h, &fd));
    FindClose(h);
  }
#else
  // Pour scan de répertoire
  DIR*  current_directory; //Répertoire courant
  struct dirent* entry; // Structure de lecture des éléments
  int filename_position;
  strcpy(full_filename, directory_name);
  current_directory=opendir(directory_name);
  if(current_directory == NULL) return;        // Répertoire invalide ...
  filename_position = strlen(full_filename);
#if defined(__AROS__)
  if (filename_position==0 || (strcmp(full_filename+filename_position-1,PATH_SEPARATOR) && strcmp(full_filename+filename_position-1,":")))
#else
  if (filename_position==0 || strcmp(full_filename+filename_position-1,PATH_SEPARATOR))
#endif
  {
    strcat(full_filename, PATH_SEPARATOR);
    filename_position = strlen(full_filename);
  }
  while ((entry=readdir(current_directory)))
  {
    struct stat Infos_enreg;
    strcpy(&full_filename[filename_position], entry->d_name);
    stat(full_filename,&Infos_enreg);
    if (S_ISREG(Infos_enreg.st_mode))
    {
      Callback(full_filename, entry->d_name);
    }
  }
  closedir(current_directory);
#endif
}

/// Scans a directory, calls Callback for each file or directory in it,
void For_each_directory_entry(const char * directory_name, void * pdata, T_File_dir_cb Callback)
{
#if defined(WIN32)
  WIN32_FIND_DATAW fd;
  word search_string[MAX_PATH_CHARACTERS];
  HANDLE h;

  Unicode_char_strlcpy(search_string, directory_name, MAX_PATH_CHARACTERS);
  Unicode_char_strlcat(search_string, "\\*", MAX_PATH_CHARACTERS);
  h = FindFirstFileW((WCHAR *)search_string, &fd);
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
  char full_filename[MAX_PATH_CHARACTERS];
  word * unicode_filename = NULL;
  word unicode_buffer[MAX_PATH_CHARACTERS];
  int filename_position;

  current_directory=opendir(directory_name);
  if(current_directory == NULL) return;        // Invalid directory

  filename_position = strlen(directory_name);
  memcpy(full_filename, directory_name, filename_position+1);
#if defined(__AROS__)
  if (filename_position==0 || (strcmp(full_filename+filename_position-1,PATH_SEPARATOR) && strcmp(full_filename+filename_position-1,":")))
#else
  if (filename_position==0 || strcmp(full_filename+filename_position-1,PATH_SEPARATOR))
#endif
  {
    strcat(full_filename, PATH_SEPARATOR);
    filename_position = strlen(full_filename);
  }
  while ((entry=readdir(current_directory)))
  {
    struct stat Infos_enreg;
#ifdef ENABLE_FILENAMES_ICONV
    char * input = entry->d_name;
    size_t inbytesleft = strlen(entry->d_name);
    char * output = (char *)unicode_buffer;
    size_t outbytesleft = sizeof(unicode_buffer) - 2;
    unicode_filename = NULL;
    if (cd_utf16 != (iconv_t)-1)
    {
      size_t r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
      if (r != (size_t)-1)
      {
        output[0] = '\0';
        output[1] = '\0';
        unicode_filename = unicode_buffer;
      }
    }
#endif
    strcpy(&full_filename[filename_position], entry->d_name);
    stat(full_filename,&Infos_enreg);
    Callback(
      pdata,
      entry->d_name,
      unicode_filename,
      S_ISREG(Infos_enreg.st_mode), 
      S_ISDIR(Infos_enreg.st_mode), 
      File_is_hidden(entry->d_name, full_filename));
  }
  closedir(current_directory);
#endif
}


void Get_full_filename(char * output_name, const char * file_name, const char * directory_name)
{
  strcpy(output_name,directory_name);
  if (output_name[0] != '\0')
  {
    // Append a separator at the end of path, if there isn't one already.
    // This handles the case of directory variables which contain one,
    // as well as directories like "/" on Unix.
#if defined(__AROS__)
    // additional check for ':' to avoid paths like PROGDIR:/unnamed.gif
    if ((output_name[strlen(output_name)-1]!=PATH_SEPARATOR[0]) && (output_name[strlen(output_name)-1]!=':'))
#else
    if (output_name[strlen(output_name)-1]!=PATH_SEPARATOR[0])
#endif
        strcat(output_name,PATH_SEPARATOR);
  }
  strcat(output_name,file_name);
}

/**
 * Convert a file name to unicode characters
 *
 * @param filename_unicode the output buffer of MAX_PATH_CHARACTERS wide characters
 * @param filename the input file name
 * @param directory the input file directory
 * @return 0 if no conversion has taken place.
 * @return 1 if the unicode filename has been retrieved
 */
int Get_Unicode_Filename(word * filename_unicode, const char * filename, const char * directory)
{
#if defined(WIN32)
  int i = 0, j = 0;
  WCHAR shortPath[MAX_PATH_CHARACTERS];
  WCHAR longPath[MAX_PATH_CHARACTERS];

  // copy the full path to a wide character buffer :
  while (directory[0] != '\0')
    shortPath[i++] = *directory++;
  if (i > 0 && shortPath[i-1] != '\\')   // add path separator only if it is missing
    shortPath[i++] = '\\';
  while (filename[0] != '\0')
    shortPath[i++] = *filename++;
  shortPath[i++] = 0;
  if (GetLongPathNameW(shortPath, longPath, MAX_PATH_CHARACTERS) == 0)
    return 0;
  i = 0;
  while (longPath[j] != 0)
  {
    if (longPath[j] == '\\')
      i = 0;
    else
      filename_unicode[i++] = longPath[j];
    j++;
  }
  filename_unicode[i++] = 0;
  return 1;
#elif defined(ENABLE_FILENAMES_ICONV)
  char * input = (char *)filename;
  size_t inbytesleft = strlen(filename);
  char * output = (char *)filename_unicode;
  size_t outbytesleft = (MAX_PATH_CHARACTERS - 1) * 2;
  (void)directory;  // unused
  if (cd_utf16 != (iconv_t)-1)
  {
    size_t r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
    if (r != (size_t)-1)
    {
      output[0] = '\0';
      output[1] = '\0';
      return 1;
    }
  }
  return 0;
#else
  (void)filename_unicode;
  (void)filename;
  (void)directory;
  // not implemented
  return 0;
#endif
}

/// Lock file used to prevent several instances of grafx2 from harming each others' backups
#ifdef WIN32
HANDLE Lock_file_handle = INVALID_HANDLE_VALUE;
#else
int Lock_file_handle = -1;
#endif

byte Create_lock_file(const char *file_directory)
{
  #if defined (__amigaos__)||(__AROS__)||(__ANDROID__)
    #warning "Missing code for your platform, please check and correct!"
  #else
  char lock_filename[MAX_PATH_CHARACTERS];
  
#ifdef GCWZERO
  strcpy(lock_filename,"/media/home/.grafx2/");
#else
  strcpy(lock_filename,file_directory);
#endif
  strcat(lock_filename,"gfx2.lck");
  
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
  if (Lock_file_handle == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  #else
  // Unixy method for lock file
  Lock_file_handle = open(lock_filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
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
  char lock_filename[MAX_PATH_CHARACTERS];
    
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
  strcpy(lock_filename,file_directory);
  strcat(lock_filename,"gfx2.lck");
  remove(lock_filename);
}

const char * Get_current_directory(char * buf, word * buf_unicode, size_t size)
{
#if defined(__MINT__)
  buf[0] = 'A'+Dgetdrv();
  buf[1] = ':';
  buf[2] = '\\';
  Dgetpath(buf+3,0);
  strcat(buf,PATH_SEPARATOR);

  if (buf_unicode != NULL)
    buf_unicode[0] = 0; // no unicode support

  return buf;
#elif defined(WIN32)
  if (GetCurrentDirectoryA(size, buf) == 0)
    return NULL;
  if (buf_unicode != NULL)
  {
    int i;
    WCHAR temp[MAX_PATH_CHARACTERS];
    for (i = 0; i < MAX_PATH_CHARACTERS - 1 && buf[i] != '\0'; i++)
      temp[i] = buf[i];
    temp[i] = 0;
    buf_unicode[0] = 0;
    GetLongPathNameW(temp, (WCHAR *)buf_unicode, size);
    //GetCurrentDirectoryW(size, (WCHAR *)buf_unicode);
  }
  return buf;
#else
  char * ret = getcwd(buf, size);
#ifdef ENABLE_FILENAMES_ICONV
  if (ret != NULL && buf_unicode != NULL)
  {
    char * input = buf;
    size_t inbytesleft = strlen(buf);
    char * output = (char *)buf_unicode;
    size_t outbytesleft = 2 * (size - 1);
    if (cd_utf16 != (iconv_t)-1)
    {
      size_t r = iconv(cd_utf16, &input, &inbytesleft, &output, &outbytesleft);
      if (r != (size_t)-1)
      {
        output[0] = '\0';
        output[1] = '\0';
      }
    }
  }
#else
  if (buf_unicode != NULL)
    buf_unicode[0] = 0; // no unicode support
#endif
  return ret;
#endif
}

int Change_directory(const char * path)
{
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
