/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program
 *  Gif Analyzer tool

    Copyright 2018 Thomas Bernard

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define IFF_EOF -1
#define IFF_FILE_TOO_LONG -2
#define IFF_UNRECOGNIZED_CONTID -3
#define IFF_SIZE_MISMATCH -4

const char * parseiff_errorstr(int err)
{
  switch (err)
  {
    case IFF_EOF:
      return "prematurate End OF File";
    case IFF_FILE_TOO_LONG:
      return "Extra bytes in file after end of IFF";
    case IFF_UNRECOGNIZED_CONTID:
      return "Unrecognized IFF container ID (should probably be FORM)";
    case IFF_SIZE_MISMATCH:
      return "Size Mismatch";
    default:
      return "Unknown error";
  }
}

static int read_long_be(FILE * f, uint32_t * l)
{
  int i, b;
  for (i = 0; i < 4; i++)
  {
    b = getc(f);
    if (b == EOF)
      return IFF_EOF;
    *l = (*l << 8) | b;
  }
  return 0;
}

static const char * id_chunks[] = {
  "LIST", "FORM", "PROP", "CAT ", "    ", NULL
};

static int iscontainer(const char * chunkid)
{
  int i;
  for (i = 0; id_chunks[i]; i++)
  {
    if (memcmp(id_chunks[i], chunkid, 4) == 0)
      return 1;
  }
  return 0;
}

static int parseiff_chunks(FILE * f, uint32_t size, int level)
{
  int i, index;
  size_t n;
  char section[4];
  uint32_t section_size;

  //if (size&1)
  //{
  //  fprintf(stderr, "WARNING: odd size of Container chunk, adjusting\n");
  //  size++;
  //}
  index = 0;
  while (size >= 8)
  {
    printf("%06lX: ", ftell(f));
    for (i = 0; i < level; i++)
      printf("  ");
    printf("#%02d ", index++);
    n = fread(section, 1, 4, f);
    if (n != 4)
      return IFF_EOF;
    if (read_long_be(f, &section_size) < 0)
      return IFF_EOF;
    size -= 8;
    if (iscontainer(section))
    {
      int r;
      char format[4];

      n = fread(format, 1, 4, f);
      if (n != 4)
        return IFF_EOF;
      printf("%.4s %u %.4s\n", section, section_size, format);
      r = parseiff_chunks(f, section_size - 4, level+1);
      if (r < 0)
        return r;
    }
    else
    {
      printf("%.4s %u\n", section, section_size);
      if ((size & 1) == 0) // if container has EVEN size
        section_size = (section_size+1)&~1; // round to WORD boundary
      fseek(f, section_size, SEEK_CUR);
    }
    if (section_size > size)
    {
      fprintf(stderr, "remaining size in chunk : %u, section size is %u\n", size, section_size);
      return IFF_SIZE_MISMATCH;
    }
    size -= section_size;
  }
  if (size != 0)
  {
    fprintf(stderr, "level=%d size=%u\n", level, size);
    return IFF_SIZE_MISMATCH;
  }
  return 0;
}

static int parseiff_container(FILE * f, int level)
{
  int i;
  size_t n;
  char contid[4];
  char format[4];
  uint32_t size;

  printf("%06lX: ", ftell(f));
  for (i = 0; i < level; i++)
    printf("  ");
  n = fread(contid, 1, 4, f);
  if (n != 4)
    return IFF_EOF;
  if (read_long_be(f, &size) < 0)
    return IFF_EOF;
  printf("%.4s %u ", contid, size);
  if (iscontainer(contid))
  {
    n = fread(format, 1, 4, f);
    if (n != 4)
      return IFF_EOF;
    printf("%.4s\n", format);
    return parseiff_chunks(f, size - 4, level+1);
  }
  printf("\n");
  return IFF_UNRECOGNIZED_CONTID;
}

int parseiff(FILE * f)
{
  int r;
  long offset, file_size;
  r = parseiff_container(f, 0);
  if (r < 0)
    return r;
  // check we are at end of file
  offset = ftell(f);
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  fseek(f, offset, SEEK_SET);
  if (file_size > offset + 8)
  {
    fprintf(stderr, "Tying to parse the %ld extra bytes.\n", file_size - offset);
    r = parseiff_chunks(f, file_size - offset, 0);
    if (r < 0)
      return r;
  }
  offset = ftell(f);
  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  if (offset != file_size)
  {
    fprintf(stderr, "parsed %ld bytes, but file is %ld bytes long.\n", offset, file_size);
    return IFF_FILE_TOO_LONG;
  }
  return 0;
}

int main(int argc, char * * argv)
{
  const char * filename;
  FILE * f;
  int r;

  printf("IFF file parser. Displays structure of IFF files.\n");
  if (argc < 2)
  {
    printf("Usage: %s <file.iff>\n", argv[0]);
    return 1;
  }
  filename = argv[1];
  f = fopen(filename, "rb");
  if (f == NULL)
  {
    fprintf(stderr, "Can't open file %s for reading.\n", filename);
    return 2;
  }
  r = parseiff(f);
  if (r < 0)
  {
    putchar('\n');
    fprintf(stderr, "%s: ERROR %s (%d)\n", filename, parseiff_errorstr(r), r);
  }
  return r;
}
