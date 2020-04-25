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
#ifndef GFX2LOG_H_DEFINED
#define GFX2LOG_H_DEFINED

#include <stdarg.h>
#include "struct.h"

/**
 * @defgroup log error and debugging logs
 * Functions to log error and debugging messages.
 * @{
 */

typedef enum {
  GFX2_ERROR = 0,
  GFX2_WARNING,
  GFX2_INFO,
  GFX2_DEBUG
} GFX2_Log_priority_T;

extern GFX2_Log_priority_T GFX2_verbosity_level;

/**
 * Log error or debugging info
 */
extern void GFX2_Log(GFX2_Log_priority_T priority, const char * fmt, ...);

extern void GFX2_LogV(GFX2_Log_priority_T priority, const char * fmt, va_list ap);

extern void GFX2_LogHexDump(GFX2_Log_priority_T priority, const char * header, const byte * data, long offset, long count);

/** @} */
#endif
