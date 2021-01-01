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
/** @file haiku.h
 * Declarations of HAIKU specific functions.
 *
 * Haiku specific code was moved here, because the API is C++.
 * It can't be compiled in the usual C files.
 * So we provide a C wrapper to the functions we need.
 */
#ifndef __HAIKU_H
#define __HAIKU_H
#include "struct.h"

#ifdef __cplusplus
extern "C" {
#endif
qword haiku_get_free_space(char* path);
char* haiku_get_clipboard();
const char* haiku_get_app_path();
#ifdef __cplusplus
}
#endif

#endif
