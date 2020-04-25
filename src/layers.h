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
#ifndef LAYERS_H__
#define LAYERS_H__

int Layers_max(enum IMAGE_MODES mode);
void Button_Layer_add(int);
void Button_Layer_duplicate(int);
void Button_Layer_remove(int);
void Button_Layer_menu(int);
void Button_Layer_set_transparent(int);
void Button_Layer_get_transparent(int);
void Button_Layer_merge(int);
void Button_Layer_up(int);
void Button_Layer_down(int);
void Button_Layer_select(int);
void Button_Layer_toggle(int);
void Layer_activate(int layer, short side);
void Button_Anim_time(int);
void Button_Anim_first_frame(int);
void Button_Anim_prev_frame(int);
void Button_Anim_next_frame(int);
void Button_Anim_last_frame(int);
void Button_Anim_play(int);
void Button_Anim_continuous_prev(int);
void Button_Anim_continuous_next(int);
#endif
