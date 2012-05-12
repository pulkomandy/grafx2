#include "struct.h"

/*
 * Haiku specific code was moved here, because the API is C++.
 * It can't be compiled in the usual C files.
 * So we provide a C wrapper to the functons we need.
 */

#ifndef __HAIKU_H
#define __HAIKU_H

qword haiku_get_free_space(char* path);
char* haiku_get_clipboard();

#endif
