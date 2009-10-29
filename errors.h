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
///@file errors.h
/// Functions and macros for tracing and error reporting.
//////////////////////////////////////////////////////////////////////////////

#ifdef __VBCC__
    #define __func__ "stupid compiler !"
#endif

/// Prints the source filename, line number, function name, a string and an integer.
#define DEBUG(y,z) printf("%s %d %s | %s : %d###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

/// Same as ::DEBUG but in hexadecimal
#define DEBUGX(y,z) printf("%s %d %s | %s : %X###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

/// Helper function used by the macro ::Error
void Error_function(int error_code, const char *filename, int line_number, const char *function_name);

///
/// Report a run-time error: It will print to standard output some information
/// about the calling function, and then:
/// - If the error code is 0, just do a red screen flash and resume.
/// - If the error code is non-zero, abort the program.
#define Error(n) Error_function(n, __FILE__,__LINE__,__func__)
