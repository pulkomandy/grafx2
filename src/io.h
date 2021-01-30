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
///@file io.h
/// Low-level endian-neutral file operations, and also some filesystem operations.
/// Many of these may seem trivial, but the wrappers eliminate the need for a
/// forest of preprocessor defines in each file.
/// You MUST use the functions in this file instead of:
/// - fread() and fwrite()
/// - stat()
/// - fstat()
/// - opendir()
/// - readdir()
/// - getcwd()
/// - chdir()
/// - Also, don't assume "/" or "\\", use PATH_SEPARATOR
/// If you don't, you break another platform.
//////////////////////////////////////////////////////////////////////////////
#ifndef IO_H__
#define IO_H__

#include <stdio.h>
#include "struct.h"


/** @defgroup io File input/output
 * Functions used to read and write from/to files.
 * Except for bytes ones, each function is available in Big-Endian and
 * Little-Endian flavour.
 * @{ */

/// Reads a single byte from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_byte(FILE *file, byte *dest);
/// Writes a single byte to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_byte(FILE *file, byte b);

/// Reads several bytes from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_bytes(FILE *file, void *dest, size_t size);
// Read a line from an open file. Returns true if OK, false in case of error
int Read_byte_line(FILE *file, char *line, size_t size);
/// Writes several bytes to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_bytes(FILE *file, const void *dest, size_t size);

/// Reads a 16-bit Low-Endian word from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_word_le(FILE *file, word *dest);
/// Writes a 16-bit Low-Endian word to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_word_le(FILE *file, word w);
/// Reads a 32-bit Low-Endian dword from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_dword_le(FILE *file, dword *dest);
/// Writes a 32-bit Low-Endian dword to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_dword_le(FILE *file, dword dw);

/// Reads a 16-bit Big-Endian word from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_word_be(FILE *file, word *dest);
/// Writes a 16-bit Big-Endian word to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_word_be(FILE *file, word w);
/// Reads a 32-bit Big-Endian dword from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_dword_be(FILE *file, dword *dest);
/// Writes a 32-bit Big-Endian dword to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_dword_be(FILE *file, dword dw);

/// Size of an open file, in bytes.
/// @param file an open file
/// @return the size in bytes
/// @return 0 in case of error
unsigned long File_length_file(FILE * file);
/** @}*/


/** @defgroup filename File path and name
 * Functions used to manipulate files path and names
 * @{ */

/// Construct full file path
char * Filepath_append_to_dir(const char * dir, const char * filename);
/// Extracts the filename part from a full file name.
char * Extract_filename(const char *source);
/// Extracts the directory from a full file name.
char * Extract_path(const char *source);

/// Finds the rightmost path separator in a full filename. Used to separate directory from file.
char * Find_last_separator(const char * str);

/// Finds the rightmost path separator in a full filename in unicode. Used to separate directory from file.
word * Find_last_separator_unicode(const word * str);

/// default path separator character
#if defined(WIN32) || defined(__MINT__)
  #define PATH_SEPARATOR "\\"
#else
  #define PATH_SEPARATOR "/"
#endif

/// finds the rightmost '.' character in fullname. Used to find file extension. returns -1 if not found
int Position_last_dot(const char * fname);

/// finds the rightmost '.' character in fullname. Used to find file extension. returns -1 if not found
int Position_last_dot_unicode(const word * fname);

/** @}*/


/** @defgroup filesystem File system
 * filesystem operations
 * - checking file/directory existence and properties
 * - directory listing
 * - current diretory request/change
 * - Lock files
 * - File delete
 * @{ */
/// Size of a file, in bytes. Returns 0 in case of error.
unsigned long File_length(const char *fname);

/// Returns true if a file passed as a parameter exists in the current directory.
int File_exists(const char * fname);

/// Returns true if a directory passed as a parameter exists in the current directory.
int  Directory_exists(const char * directory);

/**
 * Creates a directory
 * @return 0 on success, -1 on error
 */
int Directory_create(const char * directory);

/// Check if a file or directory is hidden. Full name (with directories) is optional.
int File_is_hidden(const char *fname, const char *full_name);

/// Scans a directory, calls Callback for each file in it,
void For_each_file(const char * directory_name, void Callback(const char * full_name, const char * file_name));

typedef void T_File_dir_cb(void * pdata, const char * filename, const word * unicode_filename, byte is_file, byte is_directory, byte is_hidden);

/// Scans a directory, calls Callback for each file or directory in it,
void For_each_directory_entry(const char * directory_name, void * pdata, T_File_dir_cb Callback);
/** @}*/


/** @ingroup filename
 * @{ */

word * Get_Unicode_Filename(word * filename_unicode, const char * filename, const char * directory);

///
/// Appends a file or directory name to an existing directory name.
/// As a special case, when the new item is equal to PARENT_DIR, this
/// will remove the rightmost directory name.
/// reverse_path is optional, if it's non-null, the function will
/// write there :
/// - if filename is ".." : The name of eliminated directory/file
/// - else: ".."
void Append_path(char *path, const char *filename, char *reverse_path);

/** @}*/

/** @ingroup filesystem
 * @{ */
///
/// Creates a lock file, to check if an other instance of Grafx2 is running.
/// @return 0 on success (first instance), -1 on failure (others are running)
byte Create_lock_file(const char *file_directory);

///
/// Release a lock file created by ::Create_lock_file
void Release_lock_file(const char *file_directory);

///
/// Return the current directory, equivalent to getcwd()
/// @param buf destination buffer. can be NULL
/// @param unicode destination pointer for the unicode version of the path
/// @param size destination buffer size, ignored if buf is NULL
/// @return NULL for error, buf or a malloc'ed buffer
char * Get_current_directory(char * buf, word ** unicode, size_t size);

///
/// Change current directory. return 0 for success, -1 in case of error
int Change_directory(const char * path);

///
/// Remove the file
int Remove_path(const char * path);

///
/// Remove the directory
int Remove_directory(const char * path);

///
/// Calculate relative path
char * Calculate_relative_path(const char * ref_path, const char * path);

#if defined(WIN32)
void Enumerate_Network(T_Fileselector *list);
#endif

/** @}*/
#endif
