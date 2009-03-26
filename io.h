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

word  endian_magic16(word x);
dword endian_magic32(dword x);

int read_byte(FILE *file, byte *Dest);
int write_byte(FILE *file, byte b);

int read_bytes(FILE *file, void *Dest, size_t size);
int write_bytes(FILE *file, void *Dest, size_t size);

int read_word_le(FILE *file, word *Dest);
int write_word_le(FILE *file, word w);
int read_dword_le(FILE *file, dword *Dest);
int write_dword_le(FILE *file, dword dw);

int read_word_be(FILE *file, word *Dest);
int write_word_be(FILE *file, word w);
int read_dword_be(FILE *file, dword *Dest);
int write_dword_be(FILE *file, dword dw);

void Extraire_nom_fichier(char *dest, const char *Source);
void Extraire_chemin(char *dest, const char *Source);

char * Position_dernier_slash(const char * Chaine);

#if defined(__WIN32__)
  #define SEPARATEUR_CHEMIN "\\"
#else
  #define SEPARATEUR_CHEMIN "/"
#endif

// Taille de fichier, en octets
int FileLength(const char *fname);

// Taille de fichier, en octets
int File_length_file(FILE * file);

//   Détermine si un file passé en paramètre existe ou non dans le
// répertoire courant.
int Fichier_existe(char * fname);
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
int  Repertoire_existe(char * Repertoire);

// Scans a directory, calls Callback for each file in it,
void for_each_file(const char * Nom_repertoire, void Callback(const char *));
