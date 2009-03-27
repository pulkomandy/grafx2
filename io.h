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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

word  Endian_magic16(word x);
dword Endian_magic32(dword x);

int Read_byte(FILE *file, byte *dest);
int Write_byte(FILE *file, byte b);

int Read_bytes(FILE *file, void *dest, size_t size);
int Write_bytes(FILE *file, void *dest, size_t size);

int Read_word_le(FILE *file, word *dest);
int Write_word_le(FILE *file, word w);
int Read_dword_le(FILE *file, dword *dest);
int Write_dword_le(FILE *file, dword dw);

int Read_word_be(FILE *file, word *dest);
int Write_word_be(FILE *file, word w);
int Read_dword_be(FILE *file, dword *dest);
int Write_dword_be(FILE *file, dword dw);

void Extract_filename(char *dest, const char *source);
void Extract_path(char *dest, const char *source);

char * Find_last_slash(const char * str);

#if defined(__WIN32__)
  #define PATH_SEPARATOR "\\"
#else
  #define PATH_SEPARATOR "/"
#endif

// Taille de fichier, en octets
int File_length(const char *fname);

// Taille de fichier, en octets
int File_length_file(FILE * file);

//   Détermine si un file passé en paramètre existe ou non dans le
// répertoire courant.
int File_exists(char * fname);
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
int  Directory_exists(char * directory);

// Scans a directory, calls Callback for each file in it,
void For_each_file(const char * directory_name, void Callback(const char *));
