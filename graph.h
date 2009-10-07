/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007-2008 Adrien Destugues
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
///@file graph.h
/// Graphic functions that target the screen and/or image.
//////////////////////////////////////////////////////////////////////////////

void Shade_list_to_lookup_tables(word * list, short step, byte mode, byte * table_inc,
        byte * table_dec
);

void Transform_point(short x, short y,
                       float cos_a, float sin_a, short * rx, short * ry);

int Init_mode_video(int width, int height, int fullscreen,int pixel_ratio);

byte No_effect(word x,word y,byte color);
byte Effect_shade(word x,word y,byte color);
byte Effect_quick_shade(word x,word y,byte color);
byte Effect_tiling(word x,word y,byte color);
byte Effect_smooth(word x,word y,byte color);

void Display_foreback(void);


void Display_pixel(word x,word y,byte color);

void Display_paintbrush(short x,short y,byte color,byte is_preview);
void Hide_paintbrush(short x,short y);

void Resize_image(word chosen_width,word chosen_height);

void Fill_general(byte fill_color);
void Replace(byte New_color);

void Pixel_figure_preview    (word x_pos,word y_pos,byte color);
void Pixel_figure_preview_auto(word x_pos,word y_pos);
void Pixel_figure_preview_xor(word x_pos,word y_pos,byte color);
void Pixel_figure_preview_xorback(word x_pos,word y_pos,byte color);
void Pixel_figure_in_brush(word x_pos,word y_pos,byte color);

void Draw_empty_circle_permanent(short center_x,short center_y,short radius,byte color);
void Draw_empty_circle_preview  (short center_x,short center_y,short radius,byte color);
void Hide_empty_circle_preview (short center_x,short center_y,short radius);
void Draw_filled_circle         (short center_x,short center_y,short radius,byte color);

void Draw_empty_ellipse_permanent(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color);
void Draw_empty_ellipse_preview  (short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color);
void Hide_empty_ellipse_preview (short center_x,short center_y,short horizontal_radius,short vertical_radius);
void Draw_filled_ellipse        (short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color);

void Clamp_coordinates_regular_angle(short ax, short ay, short* bx, short* by);
void Draw_line_general(short start_x,short start_y,short end_x,short end_y, byte color);
void Draw_line_permanent  (short start_x,short start_y,short end_x,short end_y,byte color);
void Draw_line_preview    (short start_x,short start_y,short end_x,short end_y,byte color);
void Draw_line_preview_xor(short start_x,short start_y,short end_x,short end_y,byte color);
void Draw_line_preview_xorback(short start_x,short start_y,short end_x,short end_y,byte color);
void Hide_line_preview   (short start_x,short start_y,short end_x,short end_y);

void Draw_empty_rectangle(short start_x,short start_y,short end_x,short end_y,byte color);
void Draw_filled_rectangle(short start_x,short start_y,short end_x,short end_y,byte color);

void Draw_curve_permanent(short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4, byte color);
void Draw_curve_preview  (short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4, byte color);
void Hide_curve_preview (short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4, byte color);

void Airbrush(short clicked_button);

void Gradient_basic           (long index,short x_pos,short y_pos);
void Gradient_dithered (long index,short x_pos,short y_pos);
void Gradient_extra_dithered(long index,short x_pos,short y_pos);
void Degrade_aleatoire         (long index,short x_pos,short y_pos);

void Draw_grad_circle  (short center_x,short center_y,short radius,short spot_x,short spot_y);
void Draw_grad_ellipse(short center_x,short center_y,short horizontal_radius,short vertical_radius,short spot_x,short spot_y);
void Draw_grad_rectangle(short rax,short ray,short rbx,short rby,short vax,short vay, short vbx, short vby);

void Polyfill_general(int vertices, short * points, int color);
void Polyfill(int vertices, short * points, int color);

/// Remap the spare page according to the main page's palette
void Remap_spare(void);

///
/// All the figure-drawing functions work by calling this function for each
/// pixel to draw. Before calling these functions, you should assign
/// ::Pixel_figure depending on what you where you want to draw:
/// - ::Pixel_figure_preview : On screen.
/// - ::Pixel_figure_preview_xor : On screen, XORing the color.
/// - ::Pixel_figure_permanent : On screen and in the image.
/// - ::Pixel_figure_clear_preview : On screen, reverting to the image's pixels.
extern Func_pixel Pixel_figure;

void Update_part_of_screen(short x, short y, short width, short height);

void Redraw_grid(short x, short y, unsigned short w, unsigned short h);
