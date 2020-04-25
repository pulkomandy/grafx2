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

/// @file init_buttons.h
/// This are the button positions, shape, actions, etc.
///
/// To use this file, define a Init_button() function or macro :
/// Init_button(btn_number, tooltip, x_offset, y_offset, width, height, shape,
///             left_action, right_action, left_instant, right_instant,
///             unselect_action, family)

  Init_button(BUTTON_PAINTBRUSHES,
              "Paintbrush choice       ",
              0,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Paintbrush_menu,Button_Brush_monochrome,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_ADJUST,
              "Adjust / Transform menu ",
              0,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Adjust,Button_Transform_menu,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_DRAW,
              "Freehand draw. / Toggle ",
              17,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Draw,Button_Draw_switch_mode,
              0,1,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_CURVES,
              "Splines / Toggle        ",
              17,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Curves,Button_Curves_switch_mode,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_LINES,
              "Lines / Toggle          ",
              34,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Lines,Button_Lines_switch_mode,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_AIRBRUSH,
              "Spray / Menu            ",
              34,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Airbrush,Button_Airbrush_menu,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_FLOODFILL,
              "Floodfill / Replace col.",
              51,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Fill,Button_Replace,
              0,0,
              Button_Unselect_fill,
              FAMILY_TOOL);

  Init_button(BUTTON_POLYGONS,
              "Polylines / Polyforms   ",
              51,18,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_polygon,Button_Polyform,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_POLYFILL,
              "Polyfill / Filled Pforms",
              52,19,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_Polyfill,Button_Filled_polyform,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_RECTANGLES,
              "Empty rectangles        ",
              68,1,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_Empty_rectangle,Button_Empty_rectangle,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_FILLRECT,
              "Filled rectangles       ",
              69,2,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_Filled_rectangle,Button_Filled_rectangle,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_CIRCLES,
              "Empty circles / Toggle  ",
              68,18,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_circle_ellipse,Button_Circle_switch_mode,
              0,1,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_FILLCIRC,
              "Filled circles / Toggle ",
              69,19,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_circle_ellipse,Button_Circle_switch_mode,
              0,1,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_GRADRECT,
              "Grad. rect / Grad. menu ",
              85,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Grad_rectangle,Button_Gradients,
              0,0,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_SPHERES,
              "Grad. spheres / Toggle. ",
              85,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_circle_ellipse,Button_Circle_switch_mode,
              0,1,
              Do_nothing,
              FAMILY_TOOL);

  Init_button(BUTTON_BRUSH,
              "Brush grab. / Restore   ",
              106,1,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_Brush,Button_Restore_brush,
              0,0,
              Button_Unselect_brush,
              FAMILY_INTERRUPTION);

  Init_button(BUTTON_POLYBRUSH,
              "Lasso / Restore brush   ",
              107,2,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_Lasso,Button_Restore_brush,
              0,0,
              Button_Unselect_lasso,
              FAMILY_INTERRUPTION);

  Init_button(BUTTON_BRUSH_EFFECTS,
#ifdef __ENABLE_LUA__
              "Brush effects / factory ",
#else
              "Brush effects           ",
#endif
              106, 18,
              16, 16,
              BUTTON_SHAPE_RECTANGLE,
#ifdef __ENABLE_LUA__
              Button_Brush_FX, Button_Brush_Factory,
#else
              Button_Brush_FX, Button_Brush_FX,
#endif
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_EFFECTS,
              "Drawing modes (effects) ",
              123,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Effects,Button_Effects,
              0,0,
              Do_nothing,
              FAMILY_EFFECTS);

  Init_button(BUTTON_TEXT,
              "Text                    ",
              123,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Text,Button_Text,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_MAGNIFIER,
              "Magnify mode / Menu     ",
              140,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Magnify,Button_Magnify_menu,
              0,1,
              Button_Unselect_magnifier,
              FAMILY_INTERRUPTION);

  Init_button(BUTTON_COLORPICKER,
              "Pipette / Invert colors ",
              140,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Colorpicker,Button_Invert_foreback,
              0,0,
              Button_Unselect_colorpicker,
              FAMILY_INTERRUPTION);

  Init_button(BUTTON_RESOL,
              "Screen size / Safe. res.",
              161,1,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Resolution,Button_Safety_resolution,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_PAGE,
              "Go / Copy to other page ",
              161,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Page,Button_Copy_page,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_SAVE,
              "Save as / Save          ",
              178,1,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_Save,Button_Autosave,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_LOAD,
              "Load / Re-load          ",
              179,2,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_Load,Button_Reload,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_SETTINGS,
              "Settings / Skins        ",
              178,18,
              16,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Settings,Button_Skins,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_CLEAR,
              "Clear / with backcolor  ",
              195,1,
              17,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Clear,Button_Clear_with_backcolor,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_HELP,
              "Help / Statistics       ",
              195,18,
              17,16,
              BUTTON_SHAPE_RECTANGLE,
              Button_Help,Button_Stats,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_UNDO,
              "Undo / Redo             ",
              213,1,
              19,12,
              BUTTON_SHAPE_RECTANGLE,
              Button_Undo,Button_Redo,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_KILL,
              "Kill current page       ",
              213,14,
              19,7,
              BUTTON_SHAPE_RECTANGLE,
              Button_Kill,Button_Kill,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_QUIT,
              "Quit                    ",
              213,22,
              19,12,
              BUTTON_SHAPE_RECTANGLE,
              Button_Quit,Button_Quit,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_PALETTE,
              "Palette editor / setup  ",
              237,9,
              16,8,
              BUTTON_SHAPE_RECTANGLE,
              Button_Palette,Button_Secondary_palette,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_PAL_LEFT,
              "Scroll pal. bkwd / Fast ",
              237,18,
              15,15,
              BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
              Button_Pal_left,Button_Pal_left_fast,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_PAL_RIGHT,
              "Scroll pal. fwd / Fast  ",
              238,19,
              15,15,
              BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
              Button_Pal_right,Button_Pal_right_fast,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  Init_button(BUTTON_CHOOSE_COL,
              "Color #"                 ,
              MENU_WIDTH+1,1,
              1,32, // La largeur est mise à jour à chq chngmnt de mode
              BUTTON_SHAPE_NO_FRAME,
              Button_Select_forecolor,Button_Select_backcolor,
              1,1,
              Do_nothing,
              FAMILY_INSTANT);

  // Layer bar

  Init_button(BUTTON_LAYER_MENU,
              "Layers manager          ",
              0,0,
              57,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_menu, Button_Layer_menu,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_COLOR,
              "Get/Set transparent col.",
              58,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_get_transparent, Button_Layer_set_transparent,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_MERGE,
              "Merge layer             ",
              72,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_merge, Button_Layer_merge,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_ADD,
              "Add/Duplicate  layer    ",
              86,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_add, Button_Layer_duplicate,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_REMOVE,
              "Drop layer              ",
              100,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_remove, Button_Layer_remove,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_DOWN,
              "Lower layer             ",
              114,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_down, Button_Layer_down,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_UP,
              "Raise layer             ",
              128,0,
              13,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_up, Button_Layer_up,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_LAYER_SELECT,
              "Layer select / toggle   ",
              142,0,
              13,9, // Will be updated according to actual number of layers
              BUTTON_SHAPE_NO_FRAME,
              Button_Layer_select, Button_Layer_toggle,
              1,1,
              Do_nothing,
              FAMILY_INSTANT);

 // Anim bar

  Init_button(BUTTON_LAYER_MENU2,
              "Layers manager          ",
              0,0,
              44,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_menu, Button_Layer_menu,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_TIME,
              "Set frame time          ",
              45,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Anim_time, Button_Anim_time,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_FIRST_FRAME,
              "Go to first frame       ",
              116,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Anim_first_frame, Button_Anim_first_frame,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_PREV_FRAME,
              "Go to prev. frame/Rewind",
              130,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Anim_prev_frame, Button_Anim_continuous_prev,
              0,1,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_NEXT_FRAME,
              "Go to next frame / Play ",
              144,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Anim_next_frame, Button_Anim_continuous_next,
              0,1,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_LAST_FRAME,
              "Go to last frame        ",
              158,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Anim_last_frame, Button_Anim_last_frame,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_ADD_FRAME,
              "Add frame               ",
              177,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_duplicate, Button_Layer_duplicate,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_REMOVE_FRAME,
              "Drop frame              ",
              191,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_remove, Button_Layer_remove,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_DOWN_FRAME,
              "Move frame back         ",
              205,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_down, Button_Layer_down,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);
  Init_button(BUTTON_ANIM_UP_FRAME,
              "Move frame forwards     ",
              219,0,
              13,13,
              BUTTON_SHAPE_RECTANGLE,
              Button_Layer_up, Button_Layer_up,
              0,0,
              Do_nothing,
              FAMILY_INSTANT);

  // Status bar
  Init_button(BUTTON_HIDE,
              "Hide toolbars / Select  ",
              0,0,
              16,9,
              BUTTON_SHAPE_RECTANGLE,
              Button_Toggle_all_toolbars, Button_Toggle_toolbar,
              0,1,
              Do_nothing,
              FAMILY_TOOLBAR);
