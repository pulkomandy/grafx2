/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007-2021 Adrien Destugues

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
/** @file haiku.cpp
 * Definition of Haiku specific functions.
 *
 * Haiku API is C++, so this is C++ code exporting as C symbols
 * to be called by GrafX2 C code.
 */
#ifdef __HAIKU__
#include <Clipboard.h>
#include <Entry.h>
#include <image.h>
#include <Volume.h>

#include <stdio.h>
#include <string.h>

#include "haiku.h"

qword haiku_get_free_space(char* path)
{
	BEntry bpath(path);
	entry_ref ref;
	bpath.GetRef(&ref);
	BVolume disk(ref.device);
	return (qword) disk.Capacity();	
}

/**
 * @return NULL or a string that should be free()'d by the caller
 */
char* haiku_get_clipboard()
{
	if (be_clipboard->Lock())
	{
		const char* value;
		ssize_t len;
		be_clipboard->Data()->FindData("text/plain", B_MIME_TYPE, (const void **)&value, &len);

		be_clipboard->Unlock();
		return strdup(value);
	}
	return NULL;
}

const char* haiku_get_app_path()
{
	static image_info info;
	int32 cookie = 0;

	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		if (info.type == B_APP_IMAGE)
			return info.name;
	}

	return NULL;
}
#endif
