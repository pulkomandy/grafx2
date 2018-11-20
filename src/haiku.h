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
#ifdef __cplusplus
}
#endif

#endif
