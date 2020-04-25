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

#include <stdlib.h>
#include <string.h>

#include "gfx2mem.h"
#include "unicode.h"

size_t Unicode_strlen(const word * str)
{
  size_t len;

  len = 0;
  while(str[len] != 0)
    len++;
  return len;
}

/// equivalent of strdup() for our Unicode strings
word * Unicode_strdup(const word * str)
{
  size_t byte_size;
  word * new_str;

  if (str == NULL)
    return NULL;
  byte_size = Unicode_strlen(str) * 2 + 2;
  new_str = GFX2_malloc(byte_size);
  if (new_str != NULL)
    memcpy(new_str, str, byte_size);
  return new_str;
}

/// Copy unicode string
void Unicode_strlcpy(word * dst, const word * src, size_t len)
{
  if (len == 0)
    return;
  while (len > 1)
  {
    *dst = *src;
    if (*src == '\0')
      return;
    dst++;
    src++;
    len--;
  }
  *dst = 0;
}

/// concatenate unicode string
void Unicode_strlcat(word * dst, const word * src, size_t len)
{
  size_t dst_len = Unicode_strlen(dst);
  if (dst_len >= len)
    return;
  Unicode_strlcpy(dst + dst_len, src, len - dst_len);
}

/// Compare two unicode strings
int Unicode_strcmp(const word * s1, const word * s2)
{
  while (*s1 == *s2)
  {
    if (*s1 == 0) return 0;
    s1++;
    s2++;
  }
  return (*s1 > *s2) ? 1 : -1;
}

/// Compare an unicode string with a regular Latin1 string
int Unicode_char_strcmp(const word * s1, const char * s2)
{
  const byte * str2 = (const byte *)s2;

  while (*s1 == *str2)
  {
    if (*s1 == 0) return 0;
    s1++;
    str2++;
  }
  return (*s1 > *str2) ? 1 : -1;
}

/// Compare an unicode string with a regular Latin1 string. Ignoring case
int Unicode_char_strcasecmp(const word * s1, const char * s2)
{
  const byte * str2 = (const byte *)s2;
  unsigned int c1, c2;

  for (;;)
  {
    c1 = *s1++;
    c2 = *str2++;
    // first convert to lower case
    if ('a' <= c1 && c1 <= 'z')
        c1 -= ('a'-'A');
    if ('a' <= c2 && c2 <= 'z')
        c2 -= ('a'-'A');
    if (c1 != c2)
      return (c1 > c2) ? 1 : -1;
    if (c1 == 0)
      return 0;
  }
}

/// Copy a regular Latin1 string to an unicode string
void Unicode_char_strlcpy(word * dst, const char * src, size_t len)
{
  const byte * s = (const byte *)src;

  if (len == 0)
    return;
  while (len > 1)
  {
    *dst = *s;
    if (*s == '\0')
      return;
    dst++;
    s++;
    len--;
  }
  *dst = 0;
}

/// Append a regular Latin1 string to an unicode string
void Unicode_char_strlcat(word * dst, const char * src, size_t len)
{
  size_t dst_len = Unicode_strlen(dst);
  if (dst_len >= len)
    return;
  Unicode_char_strlcpy(dst + dst_len, src, len - dst_len);
}
