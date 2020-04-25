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
///@file bitcount.h
/// helper functions (or macros) to count bits in machine words
//////////////////////////////////////////////////////////////////////////////

#ifndef BITCOUNT_H__
#define BITCOUNT_H__

#if defined(__GNUC__) && __GNUC__ > 2
/* use GCC built in's */
#define count_set_bits __builtin_popcount
#define count_trailing_zeros __builtin_ctz
#else

/**
 * Count the number of bit sets "Popcount"
 *
 * Based on Wikipedia article for Hamming_weight, it's optimized
 * for cases when zeroes are more frequent.
 */
static int count_set_bits(unsigned int value)
{
  int count;

  for (count = 0; value != 0; count++)
    value &= value-1;
  return count;
}

#if defined(_MSC_VER)

#include <intrin.h>
// beware : the MSVC __popcnt intrinsic is not compatible with all CPU's
// _BitScanForward() is available on all architectures.

static int count_trailing_zeros(unsigned int value)
{
  unsigned long count;

  if (_BitScanForward(&count, value))
    return count;
  else
    return -1;
}
#else
/**
 * Count the number of low order zero's before the first bit set
 */
static int count_trailing_zeros(unsigned int value)
{
  int count;

  if (value == 0)
    return -1;
  for (count = 0; (value & 1) == 0; value >>= 1)
    count++;
  return count;
}
#endif
#endif

#endif
