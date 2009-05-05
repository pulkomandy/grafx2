/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Adrien Destugues

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
///@file realpath.h
/// Implementation of realpath() that is portable on all our platforms.
//////////////////////////////////////////////////////////////////////////////

#ifndef _REALPATH_H
#define _REALPATH_H

///
/// Makes an absolute filename, resolving symbolic links etc.
/// @param _path         Input path
/// @param resolved_path Output path, allocated by caller
/// @return              (points to resolved_path)
char *Realpath(const char *_path, char *resolved_path);

#endif
