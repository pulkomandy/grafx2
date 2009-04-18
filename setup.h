/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2007 Adrien Destugues
    Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//////////////////////////////////////////////////////////////////////////////
///@file setup.h
/// Functions that determine where grafx2 is running, finds its data, and
/// reads and writes configuration files.
//////////////////////////////////////////////////////////////////////////////

///
/// Determine which directory contains the executable.
/// - IN: Main's argv[0], some platforms need it, some don't.
/// - OUT: Write into program_dir. Trailing / or \ is kept.
/// Note : in fact this is only used to check for the datafiles and fonts in this same directory.
void Set_program_directory(const char * argv0,char * program_dir);

///
/// Determine which directory contains the read-only data.
/// IN: The directory containing the executable
/// OUT: Write into data_dir. Trailing / or \ is kept.
void Set_data_directory(const char * program_dir, char * data_dir);

///
/// Determine which directory should store the user's configuration.
/// For most Unix and Windows platforms:
/// If a config file already exists in program_dir, it will return it in priority
/// (Useful for development, and possibly for upgrading from DOS version)
/// If the standard directory doesn't exist yet, this function will attempt 
/// to create it ($(HOME)/.grafx2, or %APPDATA%\\GrafX2)
/// If it cannot be created, this function will return the executable's
/// own directory.
/// IN: The directory containing the executable
/// OUT: Write into config_dir. Trailing / or \ is kept.
void Set_config_directory(const char * program_dir, char * config_dir);
  
