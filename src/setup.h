/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
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


/// Name of the subdirectory containing fonts, under the data directory (::Set_data_directory())
#if defined (__MINT__)
  #define FONTS_SUBDIRECTORY "FONTS"
#else
  #define FONTS_SUBDIRECTORY "fonts"
#endif

/// Name of the subdirectory containing fonts, under the data directory (::Set_data_directory())
#if defined (__MINT__)
  #define SKINS_SUBDIRECTORY "SKINS"
#else
  #define SKINS_SUBDIRECTORY "skins"
#endif

/// Name of the subdirectory containing scripts
#if defined (__MINT__)
  #define SCRIPTS_SUBDIRECTORY "SCRIPTS"
#elif defined(__AROS__)
  #define SCRIPTS_SUBDIRECTORY "share/grafx2/scripts"
#else
  #define SCRIPTS_SUBDIRECTORY "scripts"
#endif

/// LUA directory prefix
#if defined (__MINT__)
  #define LUALIB_SUBDIRECTORY "LIBS"
#else
  #define LUALIB_SUBDIRECTORY "libs"
#endif



/// Name of the binary file containing some configuration settings.
#if defined (__MINT__)
  #define CONFIG_FILENAME "GFX2.CFG"
#else
  #define CONFIG_FILENAME "gfx2.cfg"
#endif

/// Name of the text file containing some settings in INI format.
#if defined (__MINT__)
  #define INI_FILENAME "GFX2.INI"
#else
  #define INI_FILENAME "gfx2.ini"
#endif

/// Name of the backup of the INI file.
#if defined (__MINT__)
  #define INISAVE_FILENAME "GFX2.$$$"
#else
  #define INISAVE_FILENAME "gfx2.$$$"
#endif

/// Name of the default .INI file (read-only: gives .INI format and defaults)
#if defined (__MINT__)
  #define INIDEF_FILENAME "GFX2DEF.INI"
#else
  #define INIDEF_FILENAME "gfx2def.ini"
#endif

/// Prefix for filenames of safety backups (main)
#if defined (__MINT__)
  #define SAFETYBACKUP_PREFIX_A "A"
#else
  #define SAFETYBACKUP_PREFIX_A "a"
#endif

/// Prefix for filenames of safety backups (spare)
#if defined (__MINT__)
  #define SAFETYBACKUP_PREFIX_B "B"
#else
  #define SAFETYBACKUP_PREFIX_B "b"
#endif

/// Name of the image file that serves as an application icon.
#if defined (__MINT__)
  #define GFX2_ICON_FILENAME "GFX2.GIF"
#else
  #define GFX2_ICON_FILENAME "gfx2.gif"
#endif

/// Name of the image file for the default (and fallback) GUI skin.
#if defined (__MINT__)
  #define DEFAULT_SKIN_FILENAME "SDPAINT.PNG"
#else
  #define DEFAULT_SKIN_FILENAME "skin_DPaint.png"
#endif

/// Name of the image file for the default (and fallback) 8x8 font.
#if defined (__MINT__)
  #define DEFAULT_FONT_FILENAME "FDPAINT.PNG"
#else
  #define DEFAULT_FONT_FILENAME "font_DPaint.png"
#endif

/// File extension for safety backups
#if defined (__MINT__)
  #define BACKUP_FILE_EXTENSION ".BKP"
#else
  #define BACKUP_FILE_EXTENSION ".bkp"
#endif

/// File prefix for fonts
#if defined (__MINT__)
  #define FONT_PREFIX "F"
#else
  #define FONT_PREFIX "font_"
#endif

/// File prefix for skins
#if defined (__MINT__)
  #define SKIN_PREFIX "S"
#else
  #define SKIN_PREFIX "skin_"
#endif


