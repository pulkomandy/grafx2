
word  endian_magic16(word x);
dword endian_magic32(dword x);

int read_byte(FILE *Fichier, byte *Dest);
int write_byte(FILE *Fichier, byte Byte);

int read_bytes(FILE *Fichier, void *Dest, size_t Taille);
int write_bytes(FILE *Fichier, void *Dest, size_t Taille);

int read_word_le(FILE *Fichier, word *Dest);
int write_word_le(FILE *Fichier, word Mot);
int read_dword_le(FILE *Fichier, dword *Dest);
int write_dword_le(FILE *Fichier, dword Mot);

int read_word_be(FILE *Fichier, word *Dest);
int write_word_be(FILE *Fichier, word Mot);
int read_dword_be(FILE *Fichier, dword *Dest);
int write_dword_be(FILE *Fichier, dword Mot);
