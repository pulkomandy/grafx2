/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
/// - Also, don't assume "/" or "\\", use PATH_SEPARATOR
/// If you don't, you break another platform.
//////////////////////////////////////////////////////////////////////////////

/// Reads a single byte from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_byte(FILE *file, byte *dest);
/// Writes a single byte to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_byte(FILE *file, byte b);

/// Reads several bytes from an open file. Returns true if OK, false if a file i/o error occurred.
int Read_bytes(FILE *file, void *dest, size_t size);
/// Writes several bytes to an open file. Returns true if OK, false if a file i/o error occurred.
int Write_bytes(FILE *file, void *dest, size_t size);

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

/// Extracts the filename part from a full file name.
void Extract_filename(char *dest, const char *source);
/// Extracts the directory from a full file name.
void Extract_path(char *dest, const char *source);

/// Finds the rightmost path separator in a full filename. Used to separate directory from file.
char * Find_last_slash(const char * str);

#if defined(__WIN32__)
  #define PATH_SEPARATOR "\\"
#else
  #define PATH_SEPARATOR "/"
#endif

/// Size of a file, in bytes. Returns 0 in case of error.
int File_length(const char *fname);

/// Size of a file, in bytes. Takes an open file as argument, returns 0 in case of error.
int File_length_file(FILE * file);

/// Returns true if a file passed as a parameter exists in the current directory.
int File_exists(char * fname);

/// Returns true if a directory passed as a parameter exists in the current directory.
int  Directory_exists(char * directory);

/// Scans a directory, calls Callback for each file in it,
void For_each_file(const char * directory_name, void Callback(const char *));

/// Scans a directory, calls Callback for each file or directory in it,
void For_each_directory_entry(const char * directory_name, void Callback(const char *, byte is_file, byte is_directory, byte is_hidden));


///
/// Creates a fully qualified name from a directory and filename.
/// The point is simply to insert a PATH_SEPARATOR when needed.
void Get_full_filename(char * output_name, char * file_name, char * directory_name);

///
/// Creates a lock file, to check if an other instance of Grafx2 is running.
/// @return 0 on success (first instance), -1 on failure (others are running)
byte Create_lock_file(const char *file_directory);

///
/// Release a lock file created by ::Create_Lock_file
void Release_lock_file(const char *file_directory);

