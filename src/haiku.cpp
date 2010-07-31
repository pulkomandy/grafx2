/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007 Adrien Destugues

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
#include "struct.h"

#ifdef __HAIKU__
#include <Entry.h>
#include <Volume.h>

extern "C" qword haiku_get_free_space(char* path);

qword haiku_get_free_space(char* path)
{
	BEntry bpath(path);
	entry_ref ref;
	bpath.GetRef(&ref);
	BVolume disk(ref.device);
	return (qword) disk.Capacity();	
}
#endif
