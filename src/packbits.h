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

///@file packbits.h
/// Packbits compression as used in IFF etc.

#ifndef PACKBITS_H_INCLUDED
#define PACKBITS_H_INCLUDED

// error codes :

#define PACKBITS_UNPACK_OK 0
#define PACKBITS_UNPACK_READ_ERROR -1
#define PACKBITS_UNPACK_OVERFLOW_ERROR -2

/**
 * @return PACKBITS_UNPACK_OK or PACKBITS_UNPACK_READ_ERROR or PACKBITS_UNPACK_OVERFLOW_ERROR
 */
int PackBits_unpack_from_file(FILE * f, byte * dest, unsigned int count);

/**
 * Data used by the PackBits packer
 */
typedef struct {
  FILE * f;
  int output_count;
  byte list_size;
  byte repetition_mode;
  byte list[129];
} T_PackBits_data;

/**
 * init before packing
 *
 * @param data storage for packbits data
 * @param f FILE output or NULL (for no output)
 */
void PackBits_pack_init(T_PackBits_data * data, FILE * f);

/**
 * Add a byte to the packbits stream
 * @return -1 for error, 0 if OK
 */
int PackBits_pack_add(T_PackBits_data * data, byte b);

/**
 * Flush the packed data to the file
 *
 * @return -1 for error, or the size of the packed stream so far
 */
int PackBits_pack_flush(T_PackBits_data * data);

/**
 * Pack a full buffer to FILE
 * @param f FILE output or NULL (for no output)
 * @param buffer input buffer
 * @param size byte size of input buffer
 * @return -1 for error, or the size of the packed stream so far
 */
int PackBits_pack_buffer(FILE * f, const byte * buffer, size_t size);

#endif
