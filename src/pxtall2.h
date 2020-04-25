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
///@file pxtall2.h
/// Renderer for double-tall pixels (2x4).
//////////////////////////////////////////////////////////////////////////////

#include "struct.h"

  void Pixel_tall2                      (word x,word y,byte color);
  byte Read_pixel_tall2                  (word x,word y);
  void Block_tall2                      (word start_x,word start_y,word width,word height,byte color);
  void Pixel_preview_normal_tall2       (word x,word y,byte color);
  void Pixel_preview_magnifier_tall2        (word x,word y,byte color);
  void Horizontal_XOR_line_tall2      (word x_pos,word y_pos,word width);
  void Vertical_XOR_line_tall2        (word x_pos,word y_pos,word height);
  void Display_brush_color_tall2        (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width);
  void Display_brush_mono_tall2         (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,byte color,word brush_width);
  void Clear_brush_tall2                (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word image_width);
  void Remap_screen_tall2               (word x_pos,word y_pos,word width,word height,byte * conversion_table);
  void Display_part_of_screen_tall2 (word width,word height,word image_width);
  void Display_line_on_screen_tall2   (word x_pos,word y_pos,word width,byte * line);
  void Read_line_screen_tall2       (word x_pos,word y_pos,word width,byte * line);
  void Display_part_of_screen_scaled_tall2(word width,word height,word image_width,byte * buffer);
  void Display_brush_color_zoom_tall2   (word x_pos,word y_pos,word x_offset,word y_offset,word width,word end_y_pos,byte transp_color,word brush_width,byte * buffer);
  void Display_brush_mono_zoom_tall2    (word x_pos,word y_pos,word x_offset,word y_offset,word width,word end_y_pos,byte transp_color,byte color,word brush_width,byte * buffer);
  void Clear_brush_scaled_tall2           (word x_pos,word y_pos,word x_offset,word y_offset,word width,word end_y_pos,byte transp_color,word image_width,byte * buffer);
  void Display_brush_tall2             (byte * brush, word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte transp_color,word brush_width);

  void Display_line_on_screen_fast_tall2   (word x_pos,word y_pos,word width,byte * line);
