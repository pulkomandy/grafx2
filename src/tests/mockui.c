/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2019      Thomas Bernard
    Copyright 2007-2018 Adrien Destugues
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

#include <stdio.h>
#include <stdarg.h>
#include "../struct.h"

void Warning_message(const char * message)
{
  puts(message);
}

void Warning_with_format(const char *template, ...)
{
  va_list arg_ptr;

  va_start(arg_ptr, template);
  vprintf(template, arg_ptr);
  va_end(arg_ptr);
  putchar('\n');
}

void Error_function(int error_code, const char *filename, int line_number, const char *function_name)
{
  printf("%s:%d %s(): Error %d\n", filename, line_number, function_name, error_code);
}

void Warning_function(const char *message, const char *filename, int line_number, const char *function_name)
{
  printf("%s:%d %s(): Warning: %s\n", filename, line_number, function_name, message);
}

void Window_help(int section, const char * sub_section)
{
  printf("Window_help(%d, %s)\n", section, sub_section);
}

void Open_window(word width, word height, const char * title)
{
  printf("Open_window() %hux%hu : %s\n", width, height, title);
}

void Close_window()
{
}

void Print_in_window(short x,short y,const char * str,byte text_color,byte background_color)
{
}

void Window_dropdown_add_item(T_Dropdown_button * dropdown, word btn_number, const char *label)
{
  printf("Window_dropdown_add_item(%p, %hu, %s)\n", dropdown, btn_number, label);
}

void Window_dropdown_clear_items(T_Dropdown_button * dropdown)
{
}

T_Dropdown_button * Window_set_dropdown_button(word x_pos, word y_pos,
  word width, word height, word dropdown_width, const char *label,
  byte display_choice, byte display_centered, byte display_arrow,
  byte active_button,byte bottom_up)
{
  return NULL;
}

T_Normal_button * Window_set_normal_button(word x_pos, word y_pos,
  word width, word height, const char * title, byte undersc_letter,
  byte clickable, word shortcut)
{
  return NULL;
}

short Window_clicked_button(void)
{
  // return 1, which is often the OK button
  return 1;
}

int Is_shortcut(word key, word function)
{
  printf("Is_shortcut(%04x, %04x)\n", key, function);
  return 0;
}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
}

void Display_cursor(void)
{
}

void Hide_cursor(void)
{
}

word Count_used_colors(dword * usage)
{
  return 256;
}

word Count_used_colors_area(dword* usage, word start_x, word start_y, word width, word height)
{
  return 256;
}
