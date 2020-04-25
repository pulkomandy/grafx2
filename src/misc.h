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
//////////////////////////////////////////////////////////////////////////////
///@file misc.h
/// Miscellanous unsorted functions.
//////////////////////////////////////////////////////////////////////////////

#ifndef MISC_H__
#define MISC_H__

/* For sprintf() used in Num2str() and Dec2str(a,b,c) */
#include <stdio.h>

#include "struct.h"
#include "global.h"

#define SWAP_BYTES(a,b) { byte c=a; a=b; b=c;}
#define SWAP_WORDS(a,b) { word c=a; a=b; b=c;}
#define SWAP_DWORDS(a,b) { dword c=a; a=b; b=c;}
#define SWAP_SHORTS(a,b) { short c=a; a=b; b=c;}
#define SWAP_FLOATS(a,b) { float c=a; a=b; b=c;}
#define SWAP_PBYTES(a,b) { byte * c=a; a=b; b=c;}

void Copy_image_to_brush(short start_x,short start_y,short Brush_width,short Brush_height,word image_width);
void Remap_general_lowlevel(byte * conversion_table,byte * in_buffer, byte *out_buffer,short width,short height,short buffer_width);
void Scroll_picture(byte * main_src, byte * main_dest, short x_offset,short y_offset);
void Wait_end_of_click(void);
void Set_color(byte color, byte red, byte green, byte blue);
const T_Components * Get_current_palette(void);
void Set_palette(T_Palette palette);
void Clear_current_image(byte color);
void Clear_current_image_with_stencil(byte color, byte * stencil);
dword Round_div(dword numerator,dword divisor);
word Count_used_colors(dword * usage);
word Count_used_colors_area(dword* usage, word start_x, word start_y, word width, word height);
word Count_used_colors_screen_area(dword* usage, word start_x, word start_y, word width, word height);
void Pixel_in_brush             (word x,word y,byte color);
byte Read_pixel_from_spare_screen(word x,word y);
byte Read_pixel_from_backup_screen   (word x,word y);
byte Read_pixel_from_feedback_screen (word x,word y);
byte Read_pixel_from_brush         (word x,word y);

// Gestion du chrono dans les fileselects
void Init_chrono(dword delay);
void Check_timer(void);

void Replace_colors_within_limits(byte * replace_table);

byte Effect_interpolated_colorize  (word x,word y,byte color);
byte Effect_additive_colorize    (word x,word y,byte color);
byte Effect_substractive_colorize(word x,word y,byte color);
byte Effect_alpha_colorize(word x,word y,byte color);
byte Effect_sieve(word x,word y);

///
/// Inverts a pixel buffer, according to a horizontal axis.
/// @param src    Pointer to the pixel buffer to process.
/// @param width  Width of the buffer.
/// @param height Height of the buffer.
void Flip_Y_lowlevel(byte *src, short width, short height);

///
/// Inverts a pixel buffer, according to a vertical axis.
/// @param src    Pointer to the pixel buffer to process.
/// @param width  Width of the buffer.
/// @param height Height of the buffer.
void Flip_X_lowlevel(byte *src, short width, short height);
///
/// Rotate a pixel buffer by 90 degrees, clockwise.
/// @param source Source pixel buffer.
/// @param dest Destination pixel buffer.
/// @param width Width of the original buffer (height of the destination one).
/// @param height Height of the original buffer (width of the destination one).
void Rotate_90_deg_lowlevel(byte * source, byte * dest, short width, short height);
///
/// Rotate a pixel buffer by 90 degrees, counter-clockwise.
/// @param source Source pixel buffer.
/// @param dest Destination pixel buffer.
/// @param width Width of the original buffer (height of the destination one).
/// @param height Height of the original buffer (width of the destination one).
void Rotate_270_deg_lowlevel(byte * source, byte * dest, short width, short height);
///
/// Rotate a pixel buffer by 180 degrees.
/// @param src The pixel buffer (source and destination).
/// @param width Width of the buffer.
/// @param height Height of the buffer.
void Rotate_180_deg_lowlevel(byte *src, short width, short height);

///
/// Copies an image to another, rescaling it and optionally flipping it.
/// @param src_buffer Original image (address of first byte)
/// @param src_width  Original image's width in pixels
/// @param src_height Original image's height in pixels
/// @param dst_buffer Destination image (address of first byte)
/// @param dst_width  Destination image's width in pixels
/// @param dst_height Destination image's height in pixels
/// @param x_flipped  Boolean, true to flip the image horizontally
/// @param y_flipped  Boolean, true to flip the image vertically
void Rescale(byte *src_buffer, short src_width, short src_height, byte *dst_buffer, short dst_width, short dst_height, short x_flipped, short y_flipped);

void Zoom_a_line(byte * original_line,byte * zoomed_line,word factor,word width);
void Copy_part_of_image_to_another(byte * source,word source_x,word source_y,word width,word height,word source_width,byte * dest,word dest_x,word dest_y,word destination_width);

// -- Gestion du chrono --
GFX2_GLOBAL byte Timer_state; // State du chrono: 0=Attente d'un Xème de seconde
                              //                 1=Il faut afficher la preview
                              //                 2=Plus de chrono à gerer pour l'instant
GFX2_GLOBAL dword Timer_delay;     // Nombre de 18.2ème de secondes demandés
GFX2_GLOBAL dword Timer_start;       // Heure de départ du chrono

#define Num2str(a,b,c) snprintf(b,sizeof(b),"%*lu",(int)c,(unsigned long)(a))

#define Dec2str(a,b,c) sprintf(b,"%.*f",c,(double)(a))

short Round(float value);
short Round_div_max(short numerator,short divisor);

/* round number n to d decimal points */
double Fround(double n, unsigned d);



int Min(int a,int b);
int Max(int a,int b);

const char* Mode_label(int mode);
int Convert_videomode_arg(const char *argument);

#endif
