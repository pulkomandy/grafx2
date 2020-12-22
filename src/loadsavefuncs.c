/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

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
//////////////////////////////////////////////////////////////////////////////
///@file loadsavefuncs.c
/// helper functions for load/save
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#include <unistd.h>
#endif
#if defined(WIN32)
#include <windows.h>
#if defined(_MSC_VER)
#define strdup _strdup
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif
#endif
#include "struct.h"
#include "global.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "io.h"
#include "unicode.h"
#include "gfx2mem.h"
#include "gfx2log.h"

/// For use by Save_XXX() functions
FILE * Open_file_write(T_IO_Context *context)
{
  FILE * f;
  char * filename; // filename with full path
#if defined(WIN32)
  if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
  {
    size_t len;
    WCHAR * filename_unicode;

    len = strlen(context->File_directory) + strlen(PATH_SEPARATOR)
        + Unicode_strlen(context->File_name_unicode) + 1;
    filename_unicode = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * len);
    if (filename_unicode == NULL)
      return NULL;

    Unicode_char_strlcpy((word *)filename_unicode, context->File_directory, len);
    Unicode_char_strlcat((word *)filename_unicode, PATH_SEPARATOR, len);
    Unicode_strlcat((word *)filename_unicode, context->File_name_unicode, len);

    f = _wfopen(filename_unicode, L"wb");
    if (f != NULL)
    {
      // Now the file has been created, retrieve its short (ASCII) name
      len = GetShortPathNameW(filename_unicode, NULL, 0);
      if (len > 0)
      {
        WCHAR * shortpath = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * len);
        if (shortpath != NULL)
        {
          len = GetShortPathNameW(filename_unicode, shortpath, len);
          if (len > 0)
          {
            DWORD start, index;
            for (start = len; start > 0 && shortpath[start-1] != '\\'; start--);
            free(context->File_name);
            context->File_name = (char *)GFX2_malloc(len + 1 - start);
            if (context->File_name != NULL)
            {
              for (index = 0; index < len - start; index++)
                context->File_name[index] = shortpath[start + index];
              context->File_name[index] = '\0';
            }
          }
          else
          {
            GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, %p, %u) failed !\n", filename_unicode, shortpath, len);
          }
        }
      }
      else
      {
        GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, NULL, 0) failed !\n", filename_unicode);
      }
    }
    free(filename_unicode);
    return f;
  }
#endif

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  if (filename == NULL)
    return NULL;
  f = fopen(filename, "wb");
  free(filename);
  return f;
}

FILE * Open_file_write_with_alternate_ext(T_IO_Context *context, const char * ext)
{
  FILE * f;
  char *p;
  char * filename; // filename with full path
#if defined(WIN32)
  if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
  {
    size_t len;
    WCHAR * filename_unicode;
    WCHAR * pw;

    len = strlen(context->File_directory) + strlen(PATH_SEPARATOR)
        + Unicode_strlen(context->File_name_unicode) + strlen(ext) + 1 + 1;
    filename_unicode = (WCHAR *)GFX2_malloc(len * sizeof(WCHAR));
    if (filename_unicode == NULL)
      return NULL;
    Unicode_char_strlcpy((word *)filename_unicode, context->File_directory, len);
    Unicode_char_strlcat((word *)filename_unicode, PATH_SEPARATOR, len);
    Unicode_strlcat((word *)filename_unicode, context->File_name_unicode, len);
    pw = wcschr(filename_unicode, (WCHAR)'.');
    if (pw != NULL)
      *pw = 0;
    Unicode_char_strlcat((word *)filename_unicode, ".", len);
    Unicode_char_strlcat((word *)filename_unicode, ext, len);

    f = _wfopen(filename_unicode, L"wb");
    free(filename_unicode);
    return f;
  }
#endif
  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
// TODO: fix ! (realloc if not enough space)
  p = strrchr(filename, '.');
  if (p != NULL)
    *p = '\0';
  strcat(filename, ".");
  strcat(filename, ext);

  f = fopen(filename, "wb");
  free(filename);
  return f;
}

/// For use by Load_XXX() and Test_XXX() functions
FILE * Open_file_read(T_IO_Context *context)
{
  FILE * f;
  char * filename; // filename with full path

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  f = fopen(filename, "rb");
  free(filename);
  return f;
}

struct T_Find_alternate_ext_data
{
  const char * ext;
  char * basename;
  word * basename_unicode;
  char * foundname;
  word * foundname_unicode;
};

static void Look_for_alternate_ext(void * pdata, const char * filename, const word * filename_unicode, byte is_file, byte is_directory, byte is_hidden)
{
  size_t base_len;
  struct T_Find_alternate_ext_data * params = (struct T_Find_alternate_ext_data *)pdata;
  (void)is_hidden;
  (void)is_directory;

  if (!is_file)
    return;

  if (filename_unicode != NULL && params->basename_unicode != NULL)
  {
    if (params->foundname_unicode != NULL)
      return; // We already have found a file
    base_len = Unicode_strlen(params->basename_unicode);
    if (Unicode_strlen(filename_unicode) <= base_len)
      return; // No match.
    if (filename_unicode[base_len] != '.')
      return; // No match.
#if defined(WIN32)
    {
      int cmp;
      WCHAR * temp_string = (WCHAR *)GFX2_malloc((base_len + 1) * sizeof(WCHAR));
      if (temp_string == NULL)
        return;
      memcpy(temp_string, filename_unicode, base_len * sizeof(word));
      temp_string[base_len] = 0;
      cmp = _wcsicmp((const WCHAR *)params->basename_unicode, temp_string);
      free(temp_string);
      if (cmp != 0)
        return; // No match.
    }
#else
    if (memcmp(params->basename_unicode, filename_unicode, base_len * sizeof(word)) != 0)
      return; // No match.
#endif
    if (Unicode_char_strcasecmp(filename_unicode + base_len + 1, params->ext) != 0)
      return; // No match.
    // it is a match !
    free(params->foundname);
    params->foundname_unicode = Unicode_strdup(filename_unicode);
    params->foundname = strdup(filename);
  }
  else
  {
    if (params->foundname != NULL)
      return; // We already have found a file
    base_len = strlen(params->basename);
    if (filename[base_len] != '.')
      return; // No match.
#if defined(WIN32)
    if (_memicmp(params->basename, filename, base_len) != 0)  // Not case sensitive
      return; // No match.
#else
    if (memcmp(params->basename, filename, base_len) != 0)
      return; // No match.
#endif
    if (strcasecmp(filename + base_len + 1, params->ext) != 0)
      return; // No match.
    params->foundname_unicode = NULL;
    params->foundname = strdup(filename);
  }
}

FILE * Open_file_read_with_alternate_ext(T_IO_Context *context, const char * ext)
{
  FILE * f = NULL;
  char * p;
  struct T_Find_alternate_ext_data params;

  memset(&params, 0, sizeof(params));
  params.ext = ext;
  params.basename = strdup(context->File_name);
  if (params.basename == NULL)
  {
    GFX2_Log(GFX2_ERROR, "Open_file_read_with_alternate_ext() strdup() failed\n");
    return NULL;
  }
  p = strrchr(params.basename, '.');
  if (p != NULL)
    *p = '\0';
  if (context->File_name_unicode != NULL)
  {
    size_t i = Unicode_strlen(context->File_name_unicode);
    params.basename_unicode = GFX2_malloc(sizeof(word) * (i + 1));
    if (params.basename_unicode != NULL)
    {
      memcpy(params.basename_unicode, context->File_name_unicode, (i + 1) * sizeof(word));
      while (i-- > 0)
        if (params.basename_unicode[i] == (word)'.')
        {
          params.basename_unicode[i] = 0;
          break;
        }
    }
  }

  For_each_directory_entry(context->File_directory, &params, Look_for_alternate_ext);
  if (params.foundname != NULL)
  {
    char * filename; // filename with full path

    filename = Filepath_append_to_dir(context->File_directory, params.foundname);
    f = fopen(filename, "rb");
    free(filename);
  }
  free(params.basename);
  free(params.basename_unicode);
  free(params.foundname);
  free(params.foundname_unicode);
  return f;
}

/// For use by Save_XXX() functions
void Remove_file(T_IO_Context *context)
{
  char * filename; // filename with full path

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  Remove_path(filename);
  free(filename);
}

void Palette_256_to_64(T_Palette palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    palette[i].R = palette[i].R >> 2;
    palette[i].G = palette[i].G >> 2;
    palette[i].B = palette[i].B >> 2;
  }
}

void Palette_64_to_256(T_Palette palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    palette[i].R = (palette[i].R << 2)|(palette[i].R >> 4);
    palette[i].G = (palette[i].G << 2)|(palette[i].G >> 4);
    palette[i].B = (palette[i].B << 2)|(palette[i].B >> 4);
  }
}

word Current_layer_count_used_colors(T_IO_Context *context, dword *usage)
{
  dword nb_colors = 0;
  int i;
  word x, y;

  for (i = 0; i < 256; i++) usage[i] = 0;

  for (y = 0; y < context->Height; y++)
  {
    for (x = 0; x < context->Width; x++)
      usage[Get_pixel(context, x, y)]++;
  }

  // count the total number of unique used colors
  for (i = 0; i < 256; i++)
  {
    if (usage[i] != 0)
      nb_colors++;
  }

  return nb_colors;
}
