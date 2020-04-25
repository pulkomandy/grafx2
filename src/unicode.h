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
#ifndef UNICODE_H_INCLUDED
#define UNICODE_H_INCLUDED

#include "struct.h"

//////////////////////////////////////////////////////////////////////////////
///@file unicode.h
/// Functions supporting Unicode text with 16 bits per characters.
/// It is just like UCS-2, using the native byte order.
/// This is not UTF16, as UTF16 encodes some characters using 2 16bit words.
/// But it matches UTF16 on all the other characters.
/// Also the "BOM" are not supported
//////////////////////////////////////////////////////////////////////////////


/// equivalent of strlen() for out Unicode strings
/// return the number of characters (words), so there is twice as much bytes
size_t Unicode_strlen(const word * str);

/// equivalent of strdup() for our Unicode strings
word * Unicode_strdup(const word * str);

/// Copy unicode string
void Unicode_strlcpy(word * dst, const word * src, size_t len);

/// Append unicode string to another
void Unicode_strlcat(word * dst, const word * src, size_t len);

/// Compare two unicode strings
int Unicode_strcmp(const word * s1, const word * s2);

/// Compare an unicode string with a regular Latin1 string
int Unicode_char_strcmp(const word * s1, const char * s2);

/// Compare an unicode string with a regular Latin1 string. Ignoring case
int Unicode_char_strcasecmp(const word * s1, const char * s2);

/// Copy a regular Latin1 string to an unicode string
void Unicode_char_strlcpy(word * dst, const char * src, size_t len);

/// Append a regular Latin1 string to an unicode string
void Unicode_char_strlcat(word * dst, const char * src, size_t len);

#endif
