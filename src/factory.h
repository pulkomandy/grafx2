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
#ifndef FACTORY_H__
#define FACTORY_H__

void Button_Brush_Factory(int);
void Repeat_script(void);

/// Lua scripts bound to shortcut keys.
extern char * Bound_script[10];

///
/// Run a lua script linked to a shortcut, 0-9.
/// Before: Cursor hidden
/// After: Cursor shown
void Run_numbered_script(byte index);

///
/// Returns a string stating the included Lua engine version,
/// or "Disabled" if Grafx2 is compiled without Lua.
const char * Lua_version(void);

#endif
