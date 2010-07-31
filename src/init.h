/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
///@file init.h
/// Initialization (and some de-initialization) functions.
//////////////////////////////////////////////////////////////////////////////
  
T_Gui_skin *Load_graphics(const char * skin_file);
void Set_current_skin(const char *skinfile, T_Gui_skin *gfx);
void Init_buttons(void);
void Init_operations(void);
void Init_brush_container(void);
int  Load_CFG(int reload_all);
int  Save_CFG(void);
void Set_all_video_modes(void);
void Set_config_defaults(void);
void Init_sighandler(void);
void Init_paintbrushes(void);

/// Set application icon(s)
void Define_icon(void);

extern char Gui_loading_error_message[512];

///
/// Loads a 8x8 monochrome font, the kind used in all menus and screens.
/// This function allocates the memory, and returns a pointer to it when
/// successful.
/// If an error is encountered, it frees what needs it, prints an error message
/// in ::Gui_loading_error_message, and returns NULL.
byte * Load_font(const char * font_name);

///
/// Based on which toolbars are visible, updates their offsets and
/// computes ::Menu_height and ::Menu_Y
void Compute_menu_offsets(void);

