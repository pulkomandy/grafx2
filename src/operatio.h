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
///@file operatio.h
/// Code for the drawing tools operations.
//////////////////////////////////////////////////////////////////////////////

#include "struct.h"

// General operation handling functions. These may be moved to ops_handler.h when operatio.c grows over 5000 lines again...

/// Do some housekeeping before starting work on a operation.
void Start_operation_stack(word new_operation);
/// Put a value on ::Operation_stack
void Operation_push(short value);
/// Take a value off ::Operation_stack
void Operation_pop(short * value);
void Init_start_operation(void);
short Distance(short x1, short y1, short x2, short y2);

//////////////////////////////////////////////////// OPERATION_CONTINUOUS_DRAW
void Freehand_mode1_1_0(void);
void Freehand_mode1_1_2(void);
void Freehand_mode12_0_2(void);
void Freehand_mode1_2_0(void);
void Freehand_mode1_2_2(void);

///////////////////////////////////////////////// OPERATION_DISCONTINUOUS_DRAW
void Freehand_mode2_1_0(void);
void Freehand_mode2_1_2(void);
void Freehand_mode2_2_0(void);
void Freehand_mode2_2_2(void);

////////////////////////////////////////////////////// OPERATION_POINT_DRAW
void Freehand_mode3_1_0(void);
void Freehand_Mode3_2_0(void);
void Freehand_mode3_0_1(void);

///////////////////////////////////////////////////////////// OPERATION_LINE

void Line_12_0(void);
void Line_12_5(void);
void Line_0_5(void);

///////////////////////////////////////////////////////////// OPERATION_MAGNIFY

void Magnifier_12_0(void);

/////////////////////////////////////////////////// OPERATION_RECTANGLE_?????

void Rectangle_12_0(void);
void Rectangle_12_5(void);
void Empty_rectangle_0_5(void);
void Filled_rectangle_0_5(void);

////////////////////////////////////////////////////// OPERATION_CERCLE_?????

void Circle_12_0(void);
void Circle_12_5(void);
void Empty_circle_0_5(void);
void Filled_circle_0_5(void);

///////////////////////////////////////////////////// OPERATION_ELLIPSE_?????

void Ellipse_12_0(void);
void Ellipse_12_5(void);
void Empty_ellipse_0_5(void);
void Filled_ellipse_0_5(void);

////////////////////////////////////////////////////// OPERATION_GRAB_BRUSH

void Brush_12_0(void);
void Brush_12_5(void);
void Brush_0_5(void);

///////////////////////////////////////////////////// OPERATION_STRETCH_BRUSH

void Stretch_brush_12_0(void);
void Stretch_brush_1_7(void);
void Stretch_brush_0_7(void);
void Stretch_brush_2_7(void);

//////////////////////////////////////////////////// OPERATION_ROTATE_BRUSH

void Rotate_brush_12_0(void);
void Rotate_brush_1_5(void);
void Rotate_brush_0_5(void);
void Rotate_brush_2_5(void);

///////////////////////////////////////////////////// OPERATION_DISTORT_BRUSH

void Distort_brush_0_0(void);
void Distort_brush_1_0(void);
void Distort_brush_2_0(void);
void Distort_brush_1_8(void);
void Distort_brush_2_8(void);
void Distort_brush_1_9(void);
void Distort_brush_0_9(void);

//////////////////////////////////////////////////////// OPERATION_POLYBRUSH

void Polybrush_12_8(void);

////////////////////////////////////////////////////////////// OPERATION_FILL

void Fill_1_0(void);
void Fill_2_0(void);

///////////////////////////////////////////////////////// OPERATION_REPLACE

void Replace_1_0(void);
void Replace_2_0(void);

/////////////////////////////////////////////////////////// OPERATION_COLORPICK

void Pipette_0_0(void);
void Colorpicker_12_0(void);
void Colorpicker_1_1(void);
void Colorpicker_2_1(void);
void Colorpicker_0_1(void);

/////////////////////////////////////////////////////////// OPERATION_K_LINE

void K_line_12_0(void);
void K_line_12_6(void);
void K_line_0_6(void);
void K_line_12_7(void);

/////////////////////////////////////////////////// OPERATION_COURBE_?_POINTS

void Curve_34_points_1_0(void);
void Curve_34_points_2_0(void);
void Curve_34_points_1_5(void);
void Curve_34_points_2_5(void);

void Curve_4_points_0_5(void);
void Curve_4_points_1_9(void);
void Curve_4_points_2_9(void);

void Curve_3_points_0_5(void);
void Curve_3_points_0_11(void);
void Curve_3_points_12_11(void);

///////////////////////////////////////////////////////////// OPERATION_AIRBRUSH

void Airbrush_1_0(void);
void Airbrush_2_0(void);
void Airbrush_12_2(void);
void Airbrush_0_2(void);

//////////////////////////////////////////////////////////// OPERATION_*POLY*

void Polygon_12_0(void);
void Polygon_12_9(void);

void Polyfill_12_0(void);
void Polyfill_0_8(void);
void Polyfill_12_8(void);
void Polyfill_12_9(void);

void Polyform_12_0(void);
void Polyform_12_8(void);
void Polyform_0_8(void);

void Filled_polyform_12_0(void);
void Filled_polyform_12_8(void);
void Filled_polyform_0_8(void);
void Filled_contour_0_8(void);

//////////////////////////////////////////////////////////// OPERATION_SCROLL

void Scroll_12_0(void);
void Scroll_12_5(void);
void Scroll_0_5(void);

//////////////////////////////////////////////////// OPERATION_GRAD_CIRCLE

void Grad_circle_12_0(void);
void Grad_circle_12_6(void);
void Grad_circle_0_6(void);
void Grad_circle_12_8(void);
void Grad_circle_or_ellipse_0_8(void);

////////////////////////////////////////////////// OPERATION_GRAD_ELLIPSE

void Grad_ellipse_12_0(void);
void Grad_ellipse_12_6(void);
void Grad_ellipse_0_6(void);
void Grad_ellipse_12_8(void);

///////////////////////////////////////////////// OPERATION_GRAD_RECTANGLE

void Grad_rectangle_12_0(void);
void Grad_rectangle_12_5(void);
void Grad_rectangle_0_5(void);
void Grad_rectangle_0_7(void);
void Grad_rectangle_12_7(void);
void Grad_rectangle_12_9(void);
void Grad_rectangle_0_9(void);

/////////////////////////////////////////////////// OPERATION_CENTERED_LINES

void Centered_lines_12_0(void);
void Centered_lines_12_3(void);
void Centered_lines_0_3(void);
void Centered_lines_12_7(void);
void Centered_lines_0_7(void);

/////////////////////////////////////////////////// OPERATION_RMB_COLORPICK

byte Rightclick_colorpick(byte cursor_visible);
void Rightclick_colorpick_2_1(void);
void Rightclick_colorpick_0_1(void);

/////////////////////////////////////////////////// OPERATION_PAN_VIEW

void Pan_view_0_0(void);
void Pan_view_12_0(void);
void Pan_view_12_2(void);
void Pan_view_0_2(void);
