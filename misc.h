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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
void Effacer_ecran_courant(void);
void Copier_ecran_sur_brouillon(void);
void Copy_image_to_brush(short start_x,short start_y,short Brush_width,short Brush_height,word image_width);
void Permuter_dans_l_image_les_couleurs(byte color_1,byte color_2);
void Remap_general_lowlevel(byte * conversion_table,byte * buffer,short width,short height,short buffer_width);
void Scroll_picture(short x_offset,short y_offset);
void Set_mouse_video_mode_number(void);
void Set_mouse_position(void);
void Wait_end_of_click(void);
void Mouse_sensitivity(word x,word y);
void Set_color(byte color, byte red, byte green, byte blue);
void Set_palette(T_Palette palette);
void Palette_256_to_64(T_Palette palette);
void Palette_64_to_256(T_Palette palette);
void Hide_current_image(byte color);
void Hide_current_image_with_stencil(byte color, byte * stencil);
void Wait_VBL(void);
void Slider_timer(byte speed);
dword Round_div(dword numerator,dword divisor);
word Count_used_colors(dword * usage);
word Count_used_colors_area(dword* usage, word start_x, word start_y, word width, word height);
void Pixel_in_current_screen      (word x,word y,byte color);
void Pixel_in_brush             (word x,word y,byte color);
byte Read_pixel_from_current_screen  (word x,word y);
byte Read_pixel_from_spare_screen(word x,word y);
byte Read_pixel_from_backup_screen   (word x,word y);
byte Read_pixel_from_feedback_screen (word x,word y);
byte Read_pixel_from_brush         (word x,word y);

void Ellipse_compute_limites(short horizontal_radius,short vertical_radius);
// Calcule les valeurs suivantes en fonction des deux paramètres:
//
// Ellipse_vertical_radius_squared
// Ellipse_horizontal_radius_squared
// Ellipse_Limit_High
// Ellipse_Limit_Low


byte Pixel_in_ellipse(void);
//   Indique si le pixel se trouvant à Ellipse_cursor_X pixels
// (Ellipse_cursor_X>0 = à droite, Ellipse_cursor_X<0 = à gauche) et à
// Ellipse_cursor_Y pixels (Ellipse_cursor_Y>0 = en bas,
// Ellipse_cursor_Y<0 = en haut) du centre se trouve dans l'ellipse en
// cours.

byte Pixel_in_circle(void);
//   Indique si le pixel se trouvant à Circle_cursor_X pixels
// (Circle_cursor_X>0 = à droite, Circle_cursor_X<0 = à gauche) et à
// Circle_cursor_Y pixels (Circle_cursor_Y>0 = en bas,
// Circle_cursor_Y<0 = en haut) du centre se trouve dans le cercle en
// cours.

// Gestion du chrono dans les fileselects
void Init_chrono(dword delay);
void Check_timer(void);

void Replace_a_color(byte old_color, byte New_color);
void Replace_colors_within_limits(byte * replace_table);

byte Effect_interpolated_colorize  (word x,word y,byte color);
byte Effect_additive_colorize    (word x,word y,byte color);
byte Effect_substractive_colorize(word x,word y,byte color);
byte Effect_sieve(word x,word y);

void Flip_Y_lowlevel(void);
void Flip_X_lowlevel(void);
void Rotate_90_deg_lowlevel(byte * source,byte * dest);
void Rotate_180_deg_lowlevel(void);
void Zoom_a_line(byte * original_line,byte * zoomed_line,word factor,word width);
void Copy_part_of_image_to_another(byte * source,word source_x,word source_y,word width,word height,word source_width,byte * dest,word dest_x,word dest_y,word destination_width);

// -- Gestion du chrono --
byte Timer_state; // State du chrono: 0=Attente d'un Xème de seconde
                  //                 1=Il faut afficher la preview
                  //                 2=Plus de chrono à gerer pour l'instant
dword Timer_delay;     // Nombre de 18.2ème de secondes demandés
dword Timer_start;       // Heure de départ du chrono
byte New_preview_is_needed; // Booléen "Il faut relancer le chrono de preview"


unsigned long Memory_free(void);

void  Num2str(dword number,char * str,byte nb_char);
int   Str2num(char * str);

short Round(float value);
short Round_div_max(short numerator,short divisor);

int Min(int a,int b);
int Max(int a,int b);

char* Mode_label(int mode);
int Convert_videomode_arg(const char *argument);
