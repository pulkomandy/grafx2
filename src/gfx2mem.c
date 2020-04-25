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
#include "gfx2mem.h"
#include "gfx2log.h"

void * GFX2_malloc_and_log(size_t size, const char * file, unsigned line)
{
  void * p = malloc(size);
  if (p == NULL)
    GFX2_Log(GFX2_ERROR, "%s:%u Failed to allocate %lu bytes",
             file, line, (unsigned long)size);
  return p;
}

int GFX2_is_mem_filled_with(const void * p, unsigned char b, size_t len)
{
  const unsigned char * pp = (const unsigned char *)p;
  while (len-- > 0)
  {
    if (*pp++ != b)
      return 0;
  }
  return 1;
}
