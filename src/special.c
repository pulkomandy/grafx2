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
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "engine.h"
#include "windows.h"
#include "special.h"
#include "pages.h"
#include "misc.h"
#include "osdep.h"
#include "buttons.h"





//---------------------- Modifier le pinceau spécial -------------------------

void Set_paintbrush_size(int width, int height)
{
  int x_pos,y_pos;
  int x,y;
  int radius2;

  if (width<1) width=1;
  if (height<1) height=1;
  if (width>MAX_PAINTBRUSH_SIZE) width=MAX_PAINTBRUSH_SIZE;
  if (height>MAX_PAINTBRUSH_SIZE) height=MAX_PAINTBRUSH_SIZE;
  Paintbrush_width=width;
  Paintbrush_height=height;
  Paintbrush_offset_X=Paintbrush_width>>1;
  Paintbrush_offset_Y=Paintbrush_height>>1;
  switch (Paintbrush_shape)
  {
    case PAINTBRUSH_SHAPE_ROUND :
      radius2=Circle_squared_diameter(Paintbrush_width);
   
      for (y_pos=0, y=1-Paintbrush_height; y_pos<Paintbrush_height; y_pos++,y+=2)
        for (x_pos=0, x=1-Paintbrush_width; x_pos<Paintbrush_width; x_pos++,x+=2)
        {
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=( ((x*x)+(y*y)) <= radius2 );
        }
      break;
    case PAINTBRUSH_SHAPE_SQUARE :
      for (x_pos=0,y_pos=0; x_pos<Paintbrush_height; x_pos++,y_pos+=MAX_PAINTBRUSH_SIZE)
        memset(Paintbrush_sprite+y_pos,1,Paintbrush_width);
      break;
    case PAINTBRUSH_SHAPE_SIEVE_ROUND :
    {
      int reminder=0;
      if (Paintbrush_width==1)
        reminder = 1;
      
      radius2=Circle_squared_diameter(Paintbrush_width);
   
      for (y_pos=0, y=1-Paintbrush_height; y_pos<Paintbrush_height; y_pos++,y+=2)
        for (x_pos=0, x=1-Paintbrush_width; x_pos<Paintbrush_width; x_pos++,x+=2)
        {
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=( ((x_pos+y_pos+reminder)&1) && (((x*x)+(y*y)) < radius2));
        }
      break;
    }
    case PAINTBRUSH_SHAPE_SIEVE_SQUARE:
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=!((x_pos+y_pos)&1);
      break;
    case PAINTBRUSH_SHAPE_PLUS:
      x=Paintbrush_width>>1;
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=((x_pos==x) || (y_pos==x));
      break;
    case PAINTBRUSH_SHAPE_SLASH:
      x=Paintbrush_width>>1;
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=(x_pos==(Paintbrush_width-(y_pos+1)));
      break;
    case PAINTBRUSH_SHAPE_ANTISLASH:
      x=Paintbrush_width>>1;
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=(x_pos==y_pos);
      break;
    case PAINTBRUSH_SHAPE_HORIZONTAL_BAR:
      memset(Paintbrush_sprite,1,Paintbrush_width);
      break;
    case PAINTBRUSH_SHAPE_VERTICAL_BAR:
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)]=1;
      break;
    case PAINTBRUSH_SHAPE_CROSS:
      x=Paintbrush_width>>1;
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=( (x_pos==y_pos) || (x_pos==(Paintbrush_height-(y_pos+1))) );
      break;
    case PAINTBRUSH_SHAPE_DIAMOND:
      x=Paintbrush_width>>1;
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
        {
          if (x_pos<=x)
            y=x-x_pos;
          else
            y=x_pos-x;
          if (y_pos<=x)
            y+=x-y_pos;
          else
            y+=y_pos-x;
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=(y<=x);
        }
      break;
    case PAINTBRUSH_SHAPE_RANDOM:
      // Init with blank
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        memset(Paintbrush_sprite+y_pos*MAX_PAINTBRUSH_SIZE,0,Paintbrush_width);

      radius2=Circle_squared_diameter(Paintbrush_width);
   
      for (y_pos=0, y=1-Paintbrush_height; y_pos<Paintbrush_height; y_pos++,y+=2)
        for (x_pos=0, x=1-Paintbrush_width; x_pos<Paintbrush_width; x_pos++,x+=2)
        {
          if ((x*x)+(y*y) < radius2 && !(rand()&7))
          {
              Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=1;
              // This prevents having a pixels that touch each other.
              if (x_pos>0)
                Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos-1]=0;
              if (y_pos>0)
                Paintbrush_sprite[((y_pos-1)*MAX_PAINTBRUSH_SIZE)+x_pos]=0;
          }
        }
  }
}

void Smaller_paintbrush(void)
{
  if ( (Paintbrush_shape<PAINTBRUSH_SHAPE_MISC)
    && ( (Paintbrush_width>1)
      || (Paintbrush_height>1) ) )
  {
    Hide_cursor();
    switch (Paintbrush_shape)
    {
      case PAINTBRUSH_SHAPE_CROSS:
      case PAINTBRUSH_SHAPE_PLUS:
      case PAINTBRUSH_SHAPE_DIAMOND:
        if (Paintbrush_width&1)
          Set_paintbrush_size(Paintbrush_width-2,Paintbrush_height-2);
        else
          Set_paintbrush_size(Paintbrush_width-1,Paintbrush_height-1);
        break;
      case PAINTBRUSH_SHAPE_SQUARE:
      case PAINTBRUSH_SHAPE_SLASH:
      case PAINTBRUSH_SHAPE_ANTISLASH:
      case PAINTBRUSH_SHAPE_SIEVE_SQUARE:
      case PAINTBRUSH_SHAPE_ROUND:
      case PAINTBRUSH_SHAPE_SIEVE_ROUND:
      case PAINTBRUSH_SHAPE_RANDOM:
        Set_paintbrush_size(Paintbrush_width-1,Paintbrush_height-1);
        break;
      case PAINTBRUSH_SHAPE_HORIZONTAL_BAR:
        Set_paintbrush_size(Paintbrush_width-1,1);
        break;
      case PAINTBRUSH_SHAPE_VERTICAL_BAR:
        Set_paintbrush_size(1,Paintbrush_height-1);
    }
    Display_paintbrush_in_menu();
    Display_cursor();
  }
}

void Bigger_paintbrush(void)
{
  if ( (Paintbrush_shape<PAINTBRUSH_SHAPE_MISC)
    && ( (Paintbrush_width<MAX_PAINTBRUSH_SIZE)
      || (Paintbrush_height<MAX_PAINTBRUSH_SIZE) ) )
  {
    Hide_cursor();
    switch (Paintbrush_shape)
    {
      case PAINTBRUSH_SHAPE_CROSS:
      case PAINTBRUSH_SHAPE_PLUS:
      case PAINTBRUSH_SHAPE_DIAMOND:
        if (Paintbrush_width&1)
          Set_paintbrush_size(Paintbrush_width+2,Paintbrush_height+2);
        else
          Set_paintbrush_size(Paintbrush_width+1,Paintbrush_height+1);
        break;
      case PAINTBRUSH_SHAPE_SQUARE:
      case PAINTBRUSH_SHAPE_SLASH:
      case PAINTBRUSH_SHAPE_ANTISLASH:
      case PAINTBRUSH_SHAPE_SIEVE_SQUARE:
      case PAINTBRUSH_SHAPE_ROUND:
      case PAINTBRUSH_SHAPE_SIEVE_ROUND:
      case PAINTBRUSH_SHAPE_RANDOM:
        Set_paintbrush_size(Paintbrush_width+1,Paintbrush_height+1);
        break;
      case PAINTBRUSH_SHAPE_HORIZONTAL_BAR:
        Set_paintbrush_size(Paintbrush_width+1,1);
        break;
      case PAINTBRUSH_SHAPE_VERTICAL_BAR:
        Set_paintbrush_size(1,Paintbrush_height+1);
    }
    Display_paintbrush_in_menu();
    Display_cursor();
  }
}


//--------------------- Increase the ForeColor -----------------------
void Special_next_forecolor(void)
{
  Hide_cursor();
  Set_fore_color(Fore_color+1);
  Display_cursor();
}

//--------------------- Decrease the ForeColor -----------------------
void Special_previous_forecolor(void)
{
  Hide_cursor();
  Set_fore_color(Fore_color-1);
  Display_cursor();
}

//--------------------- Increase the BackColor -----------------------
void Special_next_backcolor(void)
{
  Hide_cursor();
  Set_back_color(Back_color+1);
  Display_cursor();
}

//--------------------- Decrease the BackColor -----------------------
void Special_previous_backcolor(void)
{
  Hide_cursor();
  Set_back_color(Back_color-1);
  Display_cursor();
}

/// Picks the next foreground color, according to current shade table
void Special_next_user_forecolor(void)
{
  Hide_cursor();
  Set_fore_color(Shade_table_left[Fore_color]);
  Display_cursor();
}

/// Picks the previous foreground color, according to current shade table
void Special_previous_user_forecolor(void)
{
  Hide_cursor();
  Set_fore_color(Shade_table_right[Fore_color]);
  Display_cursor();
}

/// Picks the next background color, according to current shade table
void Special_next_user_backcolor(void)
{
  Hide_cursor();
  Set_back_color(Shade_table_left[Back_color]);
  Display_cursor();
}

/// Picks the previous background color, according to current shade table
void Special_previous_user_backcolor(void)
{
  Hide_cursor();
  Set_back_color(Shade_table_right[Back_color]);
  Display_cursor();
}

// ------------------- Scroller l'écran (pas en mode loupe) ------------------
void Scroll_screen(short delta_x,short delta_y)
{
  short temp_x_offset;
  short temp_y_offset;

  temp_x_offset=Main.offset_X+delta_x;
  temp_y_offset=Main.offset_Y+delta_y;

  if (temp_x_offset+Screen_width>Main.image_width)
    temp_x_offset=Main.image_width-Screen_width;
  if (temp_y_offset+Menu_Y>Main.image_height)
    temp_y_offset=Main.image_height-Menu_Y;
  if (temp_x_offset<0)
    temp_x_offset=0;
  if (temp_y_offset<0)
    temp_y_offset=0;

  if ( (Main.offset_X!=temp_x_offset) ||
       (Main.offset_Y!=temp_y_offset) )
  {
    Hide_cursor();
    Main.offset_X=temp_x_offset;
    Main.offset_Y=temp_y_offset;

    Compute_limits();
    Compute_paintbrush_coordinates();

    Display_all_screen();  // <=> Display_screen + Display_image_limits
    Display_cursor();
  }
}


// ---------------------- Scroller la fenêtre de la loupe --------------------
void Scroll_magnifier(short delta_x,short delta_y)
{
  short temp_x_offset;
  short temp_y_offset;

  temp_x_offset=Main.magnifier_offset_X+delta_x;
  temp_y_offset=Main.magnifier_offset_Y+delta_y;

  Clip_magnifier_offsets(&temp_x_offset, &temp_y_offset);

  if ( (Main.magnifier_offset_X!=temp_x_offset) ||
       (Main.magnifier_offset_Y!=temp_y_offset) )
  {
    Hide_cursor();
    Main.magnifier_offset_X=temp_x_offset;
    Main.magnifier_offset_Y=temp_y_offset;

    Position_screen_according_to_zoom();

    Compute_limits();
    Compute_paintbrush_coordinates();

    Display_all_screen();
    Display_cursor();
  }
}


// -------------- Changer le Zoom (grâce aux touches [+] et [-]) -------------
void Zoom(short delta)
{
  short index;
  for (index=0; ZOOM_FACTOR[index]!=Main.magnifier_factor; index++);
  index+=delta;

  if ( (index>=0) && (index<NB_ZOOM_FACTORS) )
  {
    Hide_cursor();
    Change_magnifier_factor(index,1);
    if (Main.magnifier_mode)
      Display_all_screen();
    Display_cursor();
  }
}

/**
  Set zoom value. Negative value means no zoom.
*/
void Zoom_set(int index)
{
  Hide_cursor();
  if (index<0)
  {
    /* Zoom 1:1 */
    if (Main.magnifier_mode)
      Unselect_button(BUTTON_MAGNIFIER);
  }
  else
  {
    Change_magnifier_factor(index,1);
    if (!Main.magnifier_mode)
      Select_button(BUTTON_MAGNIFIER,1);
    Display_all_screen();
  }
  Display_cursor();
}

void Transparency_set(byte amount)
{
  const int doubleclick_delay = Config.Double_key_speed;
  static long time_click = 0;
  long time_previous;
  
  if (!Colorize_mode)
  {
    // Activate mode
    switch(Colorize_current_mode)
    {
      case 0 :
        Effect_function=Effect_interpolated_colorize;
        break;
      case 1 :
        Effect_function=Effect_additive_colorize;
        break;
      case 2 :
        Effect_function=Effect_substractive_colorize;
        break;
      case 3 :
        Effect_function=Effect_alpha_colorize;
    }
    Shade_mode=0;
    Quick_shade_mode=0;
    Smooth_mode=0;
    Tiling_mode=0;

    Colorize_mode=1;
  }

  time_previous = time_click;
  time_click = GFX2_GetTicks();

  // Check if it's a quick re-press
  if (time_click - time_previous < doubleclick_delay)
  {
    // Use the typed amount as units, keep the tens.
    Colorize_opacity = ((Colorize_opacity%100) /10 *10) + amount;
    if (Colorize_opacity == 0)
      Colorize_opacity = 100;
  }
  else
  {
    // Use 10% units: "1"=10%, ... "0"=100%
    if (amount == 0)
      Colorize_opacity = 100;
    else
      Colorize_opacity = amount*10;
  }
  Compute_colorize_table();
}


