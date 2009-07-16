/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008      Franck Charlet
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

********************************************************************************

    Brush manipulation functions
*/

#include <math.h>
#include <stdlib.h>
#include <string.h> // memset()

#include "global.h"
#include "graph.h"
#include "misc.h"
#include "errors.h"
#include "windows.h"
#include "sdlscreen.h"
#include "brush.h"

// Calcul de redimensionnement du pinceau pour éviter les débordements de
// l'écran et de l'image
void Compute_clipped_dimensions(short * x,short * y,short * width,short * height)
{
  if ((*x)<Limit_left)
  {
    (*width)-=(Limit_left-(*x));
    (*x)=Limit_left;
  }

  if (((*x)+(*width))>(Limit_right+1))
  {
    (*width)=(Limit_right-(*x))+1;
  }

  if ((*y)<Limit_top)
  {
    (*height)-=(Limit_top-(*y));
    (*y)=Limit_top;
  }

  if (((*y)+(*height))>(Limit_bottom+1))
  {
    (*height)=(Limit_bottom-(*y))+1;
  }
}

  // -- Calcul de redimensionnement du pinceau pour éviter les débordements
  //    de l'écran zoomé et de l'image --

void Compute_clipped_dimensions_zoom(short * x,short * y,short * width,short * height)
{
  if ((*x)<Limit_left_zoom)
  {
    (*width)-=(Limit_left_zoom-(*x));
    (*x)=Limit_left_zoom;
  }

  if (((*x)+(*width))>(Limit_right_zoom+1))
  {
    (*width)=(Limit_right_zoom-(*x))+1;
  }

  if ((*y)<Limit_top_zoom)
  {
    (*height)-=(Limit_top_zoom-(*y));
    (*y)=Limit_top_zoom;
  }

  if (((*y)+(*height))>(Limit_bottom_zoom+1))
  {
    (*height)=(Limit_bottom_zoom-(*y))+1;
  }
}


  // -- Afficher le pinceau (de façon définitive ou non) --

void Display_paintbrush(short x,short y,byte color,byte is_preview)
  // x,y: position du centre du pinceau
  // color: couleur à appliquer au pinceau
  // is_preview: "Il ne faut l'afficher qu'à l'écran"
{
  short start_x; // Position X (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short start_y; // Position Y (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short width; // width dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short height; // height dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short start_x_counter; // Position X (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short start_y_counter; // Position Y (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short x_pos; // Position X (dans l'image) en cours d'affichage
  short y_pos; // Position Y (dans l'image) en cours d'affichage
  short counter_x; // Position X (dans la brosse/pinceau) en cours
        // d'affichage
  short counter_y; // Position Y (dans la brosse/pinceau) en cours
        // d'affichage
  short end_counter_x; // Position X ou s'arrête l'affichade de la
        // brosse/pinceau
  short end_counter_y; // Position Y ou s'arrête l'affichade de la
        // brosse/pinceau
  byte  temp_color; // color de la brosse en cours d'affichage
  int position;
  byte * temp;

  if (is_preview==0 || Mouse_K==0) // pas de curseur si on est en preview et 
	  							   // en train de cliquer
  switch (Paintbrush_shape)
  {
    case PAINTBRUSH_SHAPE_POINT : // !!! TOUJOURS EN PREVIEW !!!
      if ( (Paintbrush_X>=Limit_left)
        && (Paintbrush_X<=Limit_right)
        && (Paintbrush_Y>=Limit_top)
        && (Paintbrush_Y<=Limit_bottom) )
        {
                Pixel_preview(Paintbrush_X,Paintbrush_Y,color);
                Update_part_of_screen(x,y,1,1);
        }
      break;

    case PAINTBRUSH_SHAPE_COLOR_BRUSH : // Brush en couleur

      start_x=x-Brush_offset_X;
      start_y=y-Brush_offset_Y;
      width=Brush_width;
      height=Brush_height;
      Compute_clipped_dimensions(&start_x,&start_y,&width,&height);
      if (width<=0 || height<=0)
        break;
      start_x_counter=start_x-(x-Brush_offset_X);
      start_y_counter=start_y-(y-Brush_offset_Y);
      end_counter_x=start_x_counter+width;
      end_counter_y=start_y_counter+height;

      if (is_preview != 0)
      {
        if ( (width>0) && (height>0) )
          Display_brush_color(
                start_x-Main_offset_X,
                start_y-Main_offset_Y,
                start_x_counter,
                start_y_counter,
                width,
                height,
                Back_color,
                Brush_width
          );

        if (Main_magnifier_mode != 0)
        {
          Compute_clipped_dimensions_zoom(&start_x,&start_y,&width,
                &height
          );

          start_x_counter=start_x-(x-Brush_offset_X);
          start_y_counter=start_y-(y-Brush_offset_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            start_x=(start_x-Main_magnifier_offset_X)*Main_magnifier_factor;
            start_y=(start_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
            height=start_y+(height*Main_magnifier_factor);
            if (height>Menu_Y)
              height=Menu_Y;

            Display_brush_color_zoom(Main_X_zoom+start_x,start_y,
                                     start_x_counter,start_y_counter,
                                     width,height,Back_color,
                                     Brush_width,
                                     Horizontal_line_buffer);
          }
        }

        Update_part_of_screen(x-Brush_offset_X,y-Brush_offset_Y,Brush_width,Brush_height);

      }
      else
      {
        if ((Smear_mode != 0) && (Shade_table==Shade_table_left))
        {
          if (Smear_start != 0)
          {
            if ((width>0) && (height>0))
            {
              Copy_part_of_image_to_another(
                Main_screen, start_x, start_y, width, height,
                Main_image_width, Smear_brush,
                start_x_counter, start_y_counter,
                Smear_brush_width
              );

              Update_part_of_screen(start_x,start_y,width,height);
            }
            Smear_start=0;
          }
          else
          {
            for (y_pos = start_y, counter_y = start_y_counter;
                counter_y < end_counter_y;
                y_pos++, counter_y++
            )
              for (x_pos = start_x, counter_x = start_x_counter;
                counter_x < end_counter_x;
                x_pos++, counter_x++
              )
              {
                temp_color = Read_pixel_from_current_screen(
                        x_pos,y_pos
                );
                position = (counter_y * Smear_brush_width)+ counter_x;
                if ( (Read_pixel_from_brush(counter_x,counter_y) != Back_color)
                  && (counter_y<Smear_max_Y) && (counter_x<Smear_max_X)
                  && (counter_y>=Smear_min_Y) && (counter_x>=Smear_min_X) )
                        Display_pixel(x_pos,y_pos,Smear_brush[position]);
                Smear_brush[position]=temp_color;
              }

              Update_part_of_screen(start_x,start_y,width,height);
          }

          Smear_min_X=start_x_counter;
          Smear_min_Y=start_y_counter;
          Smear_max_X=end_counter_x;
          Smear_max_Y=end_counter_y;
        }
        else
        {
          if (Shade_table==Shade_table_left)
            for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
              for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
              {
                temp_color=Read_pixel_from_brush(counter_x,counter_y);
                if (temp_color!=Back_color)
                  Display_pixel(x_pos,y_pos,temp_color);
              }
          else
            for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
              for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
              {
                if (Read_pixel_from_brush(counter_x,counter_y)!=Back_color)
                  Display_pixel(x_pos,y_pos,color);
              }
        }
        Update_part_of_screen(start_x,start_y,width,height);

      }
      break;
    case PAINTBRUSH_SHAPE_MONO_BRUSH : // Brush monochrome
      start_x=x-Brush_offset_X;
      start_y=y-Brush_offset_Y;
      width=Brush_width;
      height=Brush_height;
      Compute_clipped_dimensions(&start_x,&start_y,&width,&height);
      start_x_counter=start_x-(x-Brush_offset_X);
      start_y_counter=start_y-(y-Brush_offset_Y);
      end_counter_x=start_x_counter+width;
      end_counter_y=start_y_counter+height;
      if (is_preview != 0)
      {
        if ( (width>0) && (height>0) )
          Display_brush_mono(start_x-Main_offset_X,
                             start_y-Main_offset_Y,
                             start_x_counter,start_y_counter,
                             width,height,
                             Back_color,Fore_color,
                             Brush_width);

        if (Main_magnifier_mode != 0)
        {
          Compute_clipped_dimensions_zoom(&start_x,&start_y,&width,&height);
          start_x_counter=start_x-(x-Brush_offset_X);
          start_y_counter=start_y-(y-Brush_offset_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            start_x=(start_x-Main_magnifier_offset_X)*Main_magnifier_factor;
            start_y=(start_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
            height=start_y+(height*Main_magnifier_factor);
            if (height>Menu_Y)
              height=Menu_Y;

            Display_brush_mono_zoom(Main_X_zoom+start_x,start_y,
                                    start_x_counter,start_y_counter,
                                    width,height,
                                    Back_color,Fore_color,
                                    Brush_width,
                                    Horizontal_line_buffer);

          }
        }

        Update_part_of_screen(x-Brush_offset_X,y-Brush_offset_Y,Brush_width,Brush_height);
      }
      else
      {
        if ((Smear_mode != 0) && (Shade_table==Shade_table_left))
        {
          if (Smear_start != 0)
          {
            if ((width>0) && (height>0))
            {
              Copy_part_of_image_to_another(Main_screen,
                                                       start_x,start_y,
                                                       width,height,
                                                       Main_image_width,
                                                       Smear_brush,
                                                       start_x_counter,
                                                       start_y_counter,
                                                       Smear_brush_width);
              Update_part_of_screen(start_x,start_y,width,height);
            }
            Smear_start=0;
          }
          else
          {
            for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
              for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
              {
                temp_color=Read_pixel_from_current_screen(x_pos,y_pos);
                position=(counter_y*Smear_brush_width)+counter_x;
                if ( (Read_pixel_from_brush(counter_x,counter_y)!=Back_color)
                  && (counter_y<Smear_max_Y) && (counter_x<Smear_max_X)
                  && (counter_y>=Smear_min_Y) && (counter_x>=Smear_min_X) )
                  Display_pixel(x_pos,y_pos,Smear_brush[position]);
                Smear_brush[position]=temp_color;
              }

            Update_part_of_screen(start_x,start_y,width,height);

          }

          Smear_min_X=start_x_counter;
          Smear_min_Y=start_y_counter;
          Smear_max_X=end_counter_x;
          Smear_max_Y=end_counter_y;
        }
        else
        {
          for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
            for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
            {
              if (Read_pixel_from_brush(counter_x,counter_y)!=Back_color)
                Display_pixel(x_pos,y_pos,color);
            }
          Update_part_of_screen(start_x,start_y,width,height);
        }
      }
      break;
    default : // Pinceau
      start_x=x-Paintbrush_offset_X;
      start_y=y-Paintbrush_offset_Y;
      width=Paintbrush_width;
      height=Paintbrush_height;
      Compute_clipped_dimensions(&start_x,&start_y,&width,&height);
      start_x_counter=start_x-(x-Paintbrush_offset_X);
      start_y_counter=start_y-(y-Paintbrush_offset_Y);
      end_counter_x=start_x_counter+width;
      end_counter_y=start_y_counter+height;
      if (is_preview != 0)
      {
        temp=Brush;
        Brush=Paintbrush_sprite;

        if ( (width>0) && (height>0) )
          Display_brush_mono(start_x-Main_offset_X,
                             start_y-Main_offset_Y,
                             start_x_counter,start_y_counter,
                             width,height,
                             0,Fore_color,
                             MAX_PAINTBRUSH_SIZE);

        if (Main_magnifier_mode != 0)
        {
          Compute_clipped_dimensions_zoom(&start_x,&start_y,&width,&height);
          start_x_counter=start_x-(x-Paintbrush_offset_X);
          start_y_counter=start_y-(y-Paintbrush_offset_Y);

          if ( (width>0) && (height>0) )
          {
            // Corrections dues au Zoom:
            start_x=(start_x-Main_magnifier_offset_X)*Main_magnifier_factor;
            start_y=(start_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
            height=start_y+(height*Main_magnifier_factor);
            if (height>Menu_Y)
              height=Menu_Y;

            Display_brush_mono_zoom(Main_X_zoom+start_x,start_y,
                                    start_x_counter,start_y_counter,
                                    width,height,
                                    0,Fore_color,
                                    MAX_PAINTBRUSH_SIZE,
                                    Horizontal_line_buffer);

          }
        }

        Brush=temp;
      }
      else
      {
        if ((Smear_mode != 0) && (Shade_table==Shade_table_left))
        {
          if (Smear_start != 0)
          {
            if ((width>0) && (height>0))
            {
              Copy_part_of_image_to_another(Main_screen,
                                                       start_x,start_y,
                                                       width,height,
                                                       Main_image_width,
                                                       Smear_brush,
                                                       start_x_counter,
                                                       start_y_counter,
                                                       Smear_brush_width);
              Update_part_of_screen(start_x,start_y,width,height);
            }
            Smear_start=0;
          }
          else
          {
            for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
              for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
              {
                temp_color=Read_pixel_from_current_screen(x_pos,y_pos);
                position=(counter_y*Smear_brush_width)+counter_x;
                if ( (Paintbrush_sprite[(MAX_PAINTBRUSH_SIZE*counter_y)+counter_x] != 0) 
					// Le pinceau sert de masque pour dire quels pixels on doit traiter dans le rectangle
                	&& (counter_y<Smear_max_Y) && (counter_x<Smear_max_X)
                	&& (counter_y>=Smear_min_Y) && (counter_x>=Smear_min_X)
					// On clippe l'effet smear entre Smear_Min et Smear_Max
				)
                	Display_pixel(x_pos,y_pos,Smear_brush[position]);
                Smear_brush[position]=temp_color;
              }
              Update_part_of_screen(start_x, start_y, width, height);
          }


          Smear_min_X=start_x_counter;
          Smear_min_Y=start_y_counter;
          Smear_max_X=end_counter_x;
          Smear_max_Y=end_counter_y;
        }
        else
        {
          for (y_pos=start_y,counter_y=start_y_counter;counter_y<end_counter_y;y_pos++,counter_y++)
            for (x_pos=start_x,counter_x=start_x_counter;counter_x<end_counter_x;x_pos++,counter_x++)
            {
              if (Paintbrush_sprite[(MAX_PAINTBRUSH_SIZE*counter_y)+counter_x] != 0)
                Display_pixel(x_pos,y_pos,color);
            }
          Update_part_of_screen(start_x,start_y,width,height);
        }
      }
  }
}

// -- Effacer le pinceau -- //
//
void Hide_paintbrush(short x,short y)
  // x,y: position du centre du pinceau
{
  short start_x; // Position X (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short start_y; // Position Y (dans l'image) à partir de laquelle on
        // affiche la brosse/pinceau
  short width; // width dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short height; // height dans l'écran selon laquelle on affiche la
        // brosse/pinceau
  short start_x_counter; // Position X (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  short start_y_counter; // Position Y (dans la brosse/pinceau) à partir
        // de laquelle on affiche la brosse/pinceau
  //short x_pos; // Position X (dans l'image) en cours d'affichage
  //short y_pos; // Position Y (dans l'image) en cours d'affichage
  //short counter_x; // Position X (dans la brosse/pinceau) en cours
        //d'affichage
  //short counter_y; // Position Y (dans la brosse/pinceau) en cours d'affichage
  short end_counter_x; // Position X ou s'arrête l'affichage de la brosse/pinceau
  short end_counter_y; // Position Y ou s'arrête l'affichage de la brosse/pinceau
  byte * temp;

  if (Mouse_K == 0)
  switch (Paintbrush_shape)
  {
    case PAINTBRUSH_SHAPE_POINT :
      if ( (Paintbrush_X>=Limit_left)
        && (Paintbrush_X<=Limit_right)
        && (Paintbrush_Y>=Limit_top)
        && (Paintbrush_Y<=Limit_bottom) )
      {
        Pixel_preview(Paintbrush_X,Paintbrush_Y,Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y));
        Update_part_of_screen(Paintbrush_X,Paintbrush_Y,1,1);
      }
      break;
    case PAINTBRUSH_SHAPE_COLOR_BRUSH :    // Brush en couleur
    case PAINTBRUSH_SHAPE_MONO_BRUSH : // Brush monochrome
      start_x=x-Brush_offset_X;
      start_y=y-Brush_offset_Y;
      width=Brush_width;
      height=Brush_height;
      Compute_clipped_dimensions(&start_x,&start_y,&width,&height);
      start_x_counter=start_x-(x-Brush_offset_X);
      start_y_counter=start_y-(y-Brush_offset_Y);
      end_counter_x=start_x_counter+width;
      end_counter_y=start_y_counter+height;

      if ( (width>0) && (height>0) )
        Clear_brush(start_x-Main_offset_X,
                    start_y-Main_offset_Y,
                    start_x_counter,start_y_counter,
                    width,height,Back_color,
                    Main_image_width);

      if (Main_magnifier_mode != 0)
      {
        Compute_clipped_dimensions_zoom(&start_x,&start_y,&width,&height);
        start_x_counter=start_x;
        start_y_counter=start_y;

        if ( (width>0) && (height>0) )
        {
          // Corrections dues au Zoom:
          start_x=(start_x-Main_magnifier_offset_X)*Main_magnifier_factor;
          start_y=(start_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
          height=start_y+(height*Main_magnifier_factor);
          if (height>Menu_Y)
            height=Menu_Y;

          Clear_brush_scaled(Main_X_zoom+start_x,start_y,
                           start_x_counter,start_y_counter,
                           width,height,Back_color,
                           Main_image_width,
                           Horizontal_line_buffer);
        }
      }
      break;
    default: // Pinceau
      start_x=x-Paintbrush_offset_X;
      start_y=y-Paintbrush_offset_Y;
      width=Paintbrush_width;
      height=Paintbrush_height;
      Compute_clipped_dimensions(&start_x,&start_y,&width,&height);
      start_x_counter=start_x-(x-Paintbrush_offset_X);
      start_y_counter=start_y-(y-Paintbrush_offset_Y);
      end_counter_x=start_x_counter+width;
      end_counter_y=start_y_counter+height;

      temp=Brush;
      Brush=Paintbrush_sprite;

      if ( (width>0) && (height>0) )
      {
        Clear_brush(start_x-Main_offset_X,
                    start_y-Main_offset_Y,
                    start_x_counter,start_y_counter,
                    width,height,0,
                    Main_image_width);
      }

      if (Main_magnifier_mode != 0)
      {
        Compute_clipped_dimensions_zoom(&start_x,&start_y,&width,&height);
        start_x_counter=start_x;
        start_y_counter=start_y;

        if ( (width>0) && (height>0) )
        {
          // Corrections dues au Zoom:
          start_x=(start_x-Main_magnifier_offset_X)*Main_magnifier_factor;
          start_y=(start_y-Main_magnifier_offset_Y)*Main_magnifier_factor;
          height=start_y+(height*Main_magnifier_factor);
          if (height>Menu_Y)
            height=Menu_Y;

          Clear_brush_scaled(Main_X_zoom+start_x,start_y,
                           start_x_counter,start_y_counter,
                           width,height,0,
                           Main_image_width,
                           Horizontal_line_buffer);
        }
      }

      Brush=temp;
      break;
  }
}



void Capture_brush(short start_x,short start_y,short end_x,short end_y,short clear)
{
  short temp;
  short x_pos;
  short y_pos;
  word  new_brush_width;
  word  new_brush_height;


  // On commence par "redresser" les bornes:
  if (start_x>end_x)
  {
    temp=start_x;
    start_x   =end_x;
    end_x     =temp;
  }
  if (start_y>end_y)
  {
    temp=start_y;
    start_y   =end_y;
    end_y     =temp;
  }

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((start_x<Main_image_width) && (start_y<Main_image_height))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_offset_X=0;
    Tiling_offset_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    new_brush_width=(end_x-start_x)+1;
    new_brush_height=(end_y-start_y)+1;

    if (start_x+new_brush_width>Main_image_width)
      new_brush_width=Main_image_width-start_x;
    if (start_y+new_brush_height>Main_image_height)
      new_brush_height=Main_image_height-start_y;

    if ( (((long)Brush_height)*Brush_width) !=
         (((long)new_brush_height)*new_brush_width) )
    {
      free(Brush);
      Brush=(byte *)malloc(((long)new_brush_height)*new_brush_width);
      if (Brush == NULL)
      {
        Error(0);

        Brush=(byte *)malloc(1*1);
		if(Brush == NULL)
		{
			Error(ERROR_MEMORY);
			exit(ERROR_MEMORY);
		}
        new_brush_height=new_brush_width=1;
        *Brush=Fore_color;
      }
    }
    Brush_width=new_brush_width;
    Brush_height=new_brush_height;

    free(Smear_brush);
    Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
    Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);

    if (Smear_brush == NULL) // On ne peut même pas allouer la brosse du smear!
    {
      Error(0);

      free(Brush);
      Brush=(byte *)malloc(1*1);
	  if(Brush == NULL)
	  {
		Error(ERROR_MEMORY);
		exit(ERROR_MEMORY);
	  }
      Brush_height=1;
      Brush_width=1;

      Smear_brush=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
	  if(Smear_brush == NULL)
	  {
		Error(ERROR_MEMORY);
		exit(ERROR_MEMORY);
	  }
      Smear_brush_height=MAX_PAINTBRUSH_SIZE;
      Smear_brush_width=MAX_PAINTBRUSH_SIZE;
    }

    Copy_image_to_brush(start_x,start_y,Brush_width,Brush_height,Main_image_width);

    // On regarde s'il faut effacer quelque chose:
    if (clear != 0)
    {
      for (y_pos=start_y;y_pos<start_y+Brush_height;y_pos++)
        for (x_pos=start_x;x_pos<start_x+Brush_width;x_pos++)
        {
          Pixel_in_current_screen(x_pos,y_pos,Back_color);
          Pixel_preview           (x_pos,y_pos,Back_color);
        }
      Update_part_of_screen(start_x,start_y,Brush_width,Brush_height);
    }

    // On centre la prise sur la brosse
    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);
  }
}


void Rotate_90_deg()
{
  short temp;
  byte * new_brush;

  new_brush=(byte *)malloc(((long)Brush_height)*Brush_width);
  if (new_brush)
  {
    Rotate_90_deg_lowlevel(Brush,/*@out@*/ new_brush,Brush_width,Brush_height);
    free(Brush);
    Brush=new_brush;

    temp=Brush_width;
    Brush_width=Brush_height;
    Brush_height=temp;

    temp=Smear_brush_width;
    Smear_brush_width=Smear_brush_height;
    Smear_brush_height=temp;

    // On centre la prise sur la brosse
    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);
  }
  else
    Error(0);
}


void Remap_brush(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  used[256]; // Tableau de booléens "La couleur est utilisée"
  int   color;


  // On commence par initialiser le tableau de booléens à faux
  for (color=0;color<=255;color++)
    used[color]=0;

  // On calcule la table d'utilisation des couleurs
  for (y_pos=0;y_pos<Brush_height;y_pos++)
    for (x_pos=0;x_pos<Brush_width;x_pos++)
      used[Read_pixel_from_brush(x_pos,y_pos)]=1;

  //  On n'est pas censé remapper la couleur de transparence, sinon la brosse
  // changera de forme, donc on dit pour l'instant qu'elle n'est pas utilisée
  // ainsi on ne s'embêtera pas à la recalculer
  used[Back_color]=0;

  //   On va maintenant se servir de la table "used" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans la brosse, donc elles
  //       ne seront pas utilisées par Remap_brush_LOWLEVEL.
  for (color=0;color<=255;color++)
    if (used[color] != 0)
      used[color]=Best_color(Spare_palette[color].R,Spare_palette[color].G,Spare_palette[color].B);

  //   Il reste une couleur non calculée dans la table qu'il faut mettre à
  // jour: c'est la couleur de fond. On l'avait inhibée pour éviter son
  // calcul inutile, mais comme il est possible qu'elle soit quand même dans
  // la brosse, on va mettre dans la table une relation d'équivalence entre
  // les deux palettes: comme on ne veut pas que la couleur soit remplacée,
  // on va dire qu'on veut qu'elle soit remplacée par la couleur en question.
  used[Back_color]=Back_color;

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_lowlevel(used,Brush,Brush_width,Brush_height,Brush_width);
  //Remap_brush_LOWLEVEL(used);
}



void Outline_brush(void)
{
  long /*Pos,*/x_pos,y_pos;
  byte state;
  byte * new_brush;
  byte * temp;
  word width;
  word height;


  width=Brush_width+2;
  height=Brush_height+2;
  new_brush=(byte *)malloc(((long)width)*height);

  if (new_brush)
  {
    // On remplit la bordure ajoutée par la Backcolor
    memset(new_brush,Back_color,((long)width)*height);

    // On copie la brosse courante dans la nouvelle
    Copy_part_of_image_to_another(Brush, // source
    	0, 0, Brush_width, Brush_height, Brush_width,
        new_brush, // Destination
        1, 1, width);

    // On intervertit la nouvelle et l'ancienne brosse:
    temp=Brush;
    Brush=new_brush;
    Brush_width+=2;
    Brush_height+=2;
    width-=2;
    height-=2;

    // Si on "outline" avec une couleur différente de la Back_color on y va!
    if (Fore_color!=Back_color)
    {
      // 1er balayage (horizontal)
      for (y_pos=1; y_pos<Brush_height-1; y_pos++)
      {
        state=0;
        for (x_pos=1; x_pos<Brush_width-1; x_pos++)
        {
          if (temp[((y_pos-1)*width)+x_pos-1]==Back_color)
          {
            if (state != 0)
            {
              Pixel_in_brush(x_pos,y_pos,Fore_color);
              state=0;
            }
          }
          else if (state == 0)
          {
        	Pixel_in_brush(x_pos-1,y_pos,Fore_color);
        	state=1;
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (state != 0)
          Pixel_in_brush(x_pos,y_pos,Fore_color);
      }

      // 2ème balayage (vertical)
      for (x_pos=1; x_pos<Brush_width-1; x_pos++)
      {
        state=0;
        for (y_pos=1; y_pos<Brush_height-1; y_pos++)
        {
          if (temp[((y_pos-1)*width)+x_pos-1]==Back_color)
          {
            if (state != 0)
            {
              Pixel_in_brush(x_pos,y_pos,Fore_color);
              state=0;
            }
          }
          else if (state == 0)
          {
              Pixel_in_brush(x_pos,y_pos-1,Fore_color);
              state=1;
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (state != 0)
          Pixel_in_brush(x_pos,y_pos,Fore_color);
      }
    }

    // On recentre la prise sur la brosse
    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);

    free(temp); // Libération de l'ancienne brosse

    // Réallocation d'un buffer de Smear
    free(Smear_brush);
    Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
    Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
    Smear_brush=(byte *)malloc(((long)Smear_brush_width)*Smear_brush_height);
  }
  else
    Error(0); // Pas assez de mémoire!
}


void Nibble_brush(void)
{
  long x_pos,y_pos;
  byte state;
  byte * new_brush;
  byte * temp;
  word width;
  word height;

  if ( (Brush_width>2) && (Brush_height>2) )
  {
    width=Brush_width-2;
    height=Brush_height-2;
    new_brush=(byte *)malloc(((long)width)*height);

    if (new_brush)
    {
      // On copie la brosse courante dans la nouvelle
      Copy_part_of_image_to_another(Brush, // source
                                               1,
                                               1,
                                               width,
                                               height,
                                               Brush_width,
                                               new_brush, // Destination
                                               0,
                                               0,
                                               width);

      // On intervertit la nouvelle et l'ancienne brosse:
      temp=Brush;
      Brush=new_brush;
      Brush_width-=2;
      Brush_height-=2;
      width+=2;
      height+=2;

      // 1er balayage (horizontal)
      for (y_pos=0; y_pos<Brush_height; y_pos++)
      {
        state=(temp[(y_pos+1)*width]!=Back_color);
        for (x_pos=0; x_pos<Brush_width; x_pos++)
        {
          if (temp[((y_pos+1)*width)+x_pos+1]==Back_color)
          {
            if (state != 0)
            {
              if (x_pos>0)
                Pixel_in_brush(x_pos-1,y_pos,Back_color);
              state=0;
            }
          }
          else
          {
            if (state == 0)
            {
              Pixel_in_brush(x_pos,y_pos,Back_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (temp[((y_pos+1)*width)+x_pos+1]==Back_color)
          Pixel_in_brush(x_pos-1,y_pos,Back_color);
      }

      // 2ème balayage (vertical)
      for (x_pos=0; x_pos<Brush_width; x_pos++)
      {
        state=(temp[width+x_pos+1]!=Back_color);;
        for (y_pos=0; y_pos<Brush_height; y_pos++)
        {
          if (temp[((y_pos+1)*width)+x_pos+1]==Back_color)
          {
            if (state)
            {
              if (y_pos>0)
                Pixel_in_brush(x_pos,y_pos-1,Back_color);
              state=0;
            }
          }
          else
          {
            if (state == 0)
            {
              Pixel_in_brush(x_pos,y_pos,Back_color);
              state=1;
            }
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (temp[((y_pos+1)*width)+x_pos+1]==Back_color)
          Pixel_in_brush(x_pos,y_pos-1,Back_color);
      }

      // On recentre la prise sur la brosse
      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);

      free(temp); // Libération de l'ancienne brosse

      // Réallocation d'un buffer de Smear
      free(Smear_brush);
      Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
      Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
      Smear_brush=(byte *)malloc(((long)Smear_brush_width)*Smear_brush_height);
    }
    else
      Error(0);  // Pas assez de mémoire!
  }
}



void Capture_brush_with_lasso(int vertices, short * points,short clear)
{
  short start_x=Limit_right+1;
  short start_y=Limit_bottom+1;
  short end_x=Limit_left-1;
  short end_y=Limit_top-1;
  unsigned short temp;
  short x_pos;
  short y_pos;
  word  new_brush_width;
  word  new_brush_height;


  // On recherche les bornes de la brosse:
  for (temp=0; temp<2*vertices; temp+=2)
  {
    x_pos=points[temp];
    y_pos=points[temp+1];
    if (x_pos<start_x)
      start_x=x_pos;
    if (x_pos>end_x)
      end_x=x_pos;
    if (y_pos<start_y)
      start_y=y_pos;
    if (y_pos>end_y)
      end_y=y_pos;
  }

  // On clippe ces bornes à l'écran:
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((start_x<Main_image_width) && (start_y<Main_image_height))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_offset_X=0;
    Tiling_offset_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    new_brush_width=(end_x-start_x)+1;
    new_brush_height=(end_y-start_y)+1;

    if ( (((long)Brush_height)*Brush_width) !=
         (((long)new_brush_height)*new_brush_width) )
    {
      free(Brush);
      Brush=(byte *)malloc(((long)new_brush_height)*new_brush_width);
      if (!Brush)
      {
        Error(0);

        Brush=(byte *)malloc(1*1);
		if(Brush==NULL) Error(ERROR_MEMORY);
        new_brush_height=new_brush_width=1;
        *Brush=Fore_color;
      }
    }
    Brush_width=new_brush_width;
    Brush_height=new_brush_height;

    free(Smear_brush);
    Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
    Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);

    if (!Smear_brush) // On ne peut même pas allouer la brosse du smear!
    {
      Error(0);

      free(Brush);
      Brush=(byte *)malloc(1*1);
	  if(Brush==NULL) Error(ERROR_MEMORY);
      Brush_height=1;
      Brush_width=1;

      Smear_brush=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
      Smear_brush_height=MAX_PAINTBRUSH_SIZE;
      Smear_brush_width=MAX_PAINTBRUSH_SIZE;
    }

    Brush_offset_X=start_x;
    Brush_offset_Y=start_y;
    Pixel_figure=Pixel_figure_in_brush;

    memset(Brush,Back_color,(long)Brush_width*Brush_height);
    Polyfill_general(vertices,points,~Back_color);

    // On retrace les bordures du lasso:
    for (temp=1; temp<vertices; temp++)
    {
      Draw_line_general(points[(temp-1)<<1],points[((temp-1)<<1)+1],
                           points[temp<<1],points[(temp<<1)+1],
                           ~Back_color);
    }
    Draw_line_general(points[(vertices-1)<<1],points[((vertices-1)<<1)+1],
                         points[0],points[1],
                         ~Back_color);

    // On scanne la brosse pour remplacer tous les pixels affectés par le
    // polyfill par ceux de l'image:
    for (y_pos=start_y;y_pos<=end_y;y_pos++)
      for (x_pos=start_x;x_pos<=end_x;x_pos++)
        if (Read_pixel_from_brush(x_pos-start_x,y_pos-start_y)!=Back_color)
        {
          Pixel_in_brush(x_pos-start_x,y_pos-start_y,Read_pixel_from_current_screen(x_pos,y_pos));
          // On regarde s'il faut effacer quelque chose:
          if (clear)
            Pixel_in_current_screen(x_pos,y_pos,Back_color);
        }

    // On centre la prise sur la brosse
    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);
  }
}



//------------------------- Etirement de la brosse ---------------------------

void Stretch_brush(short x1, short y1, short x2, short y2)
{
  byte * new_brush;
  int    new_brush_width;  // Width de la nouvelle brosse
  int    new_brush_height;  // Height de la nouvelle brosse
  int    x_flipped, y_flipped;
  
  // Compute new brush dimensions
  if ((new_brush_width=x1-x2)<0)
  {
    x_flipped=1;
    new_brush_width=-new_brush_width;
  }
  new_brush_width++;

  if ((new_brush_height=y1-y2)<0)
  {
    y_flipped=1;
    new_brush_height=-new_brush_height;
  }
  new_brush_height++;

  // Free some memory
  free(Smear_brush);

  if ((new_brush=((byte *)malloc(new_brush_width*new_brush_height))))
  {
    Rescale(Brush, Brush_width, Brush_height, new_brush, new_brush_width, new_brush_height, x2<x1, y2<y1);

    free(Brush);
    Brush=new_brush;

    Brush_width=new_brush_width;
    Brush_height=new_brush_height;

    Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
    Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);

    if (!Smear_brush) // On ne peut même pas allouer la brosse du smear!
    {
      Error(0);

      free(Brush);
      Brush=(byte *)malloc(1*1);
      Brush_height=1;
      Brush_width=1;

      Smear_brush=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
      Smear_brush_height=MAX_PAINTBRUSH_SIZE;
      Smear_brush_width=MAX_PAINTBRUSH_SIZE;
    }

    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffi donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_brush car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);
    Error(0);
  }
}



void Stretch_brush_preview(short x1, short y1, short x2, short y2)
{
  int    src_x_pos,src_y_pos;
  int    initial_src_x_pos,initial_src_y_pos;
  int    delta_x,delta_y;
  int    dest_x_pos,dest_y_pos;
  int    initial_dest_x_pos,initial_dest_y_pos;
  int    final_dest_x_pos,final_dest_y_pos;
  int    dest_width,dest_height;
  byte   color;


  // 1er calcul des positions destination extremes:
  initial_dest_x_pos=Min(x1,x2);
  initial_dest_y_pos=Min(y1,y2);
  final_dest_x_pos  =Max(x1,x2);
  final_dest_y_pos  =Max(y1,y2);

  // Calcul des dimensions de la destination:
  dest_width=final_dest_x_pos-initial_dest_x_pos+1;
  dest_height=final_dest_y_pos-initial_dest_y_pos+1;

  // Calcul des vecteurs d'incrémentation :
  delta_x=(Brush_width<<16)/dest_width;
  delta_y=(Brush_height<<16)/dest_height;

  // 1er calcul de la position X initiale dans la source:
  initial_src_x_pos=(Brush_width<<16)*
                     (Max(initial_dest_x_pos,Limit_left)-
                      initial_dest_x_pos)/dest_width;
  // Calcul du clip de la destination:
  initial_dest_x_pos=Max(initial_dest_x_pos,Limit_left);
  final_dest_x_pos  =Min(final_dest_x_pos  ,Limit_visible_right);
  // On discute selon l'inversion en X
  if (x1>x2)
  {
    // Inversion -> Inversion du signe de delta_x
    delta_x=-delta_x;
    initial_src_x_pos=(Brush_width<<16)-1-initial_src_x_pos;
  }

  // 1er calcul de la position Y initiale dans la source:
  initial_src_y_pos=(Brush_height<<16)*
                     (Max(initial_dest_y_pos,Limit_top)-
                      initial_dest_y_pos)/dest_height;
  // Calcul du clip de la destination:
  initial_dest_y_pos=Max(initial_dest_y_pos,Limit_top);
  final_dest_y_pos  =Min(final_dest_y_pos  ,Limit_visible_bottom);
  // On discute selon l'inversion en Y
  if (y1>y2)
  {
    // Inversion -> Inversion du signe de delta_y
    delta_y=-delta_y;
    initial_src_y_pos=(Brush_height<<16)-1-initial_src_y_pos;
  }

  // Pour chaque ligne :
  src_y_pos=initial_src_y_pos;
  for (dest_y_pos=initial_dest_y_pos;dest_y_pos<=final_dest_y_pos;dest_y_pos++)
  {
    // Pour chaque colonne:
    src_x_pos=initial_src_x_pos;
    for (dest_x_pos=initial_dest_x_pos;dest_x_pos<=final_dest_x_pos;dest_x_pos++)
    {
      color=Read_pixel_from_brush(src_x_pos>>16,src_y_pos>>16);
      if (color!=Back_color)
        Pixel_preview(dest_x_pos,dest_y_pos,color);

      src_x_pos+=delta_x;
    }

    src_y_pos+=delta_y;
  }
  Update_part_of_screen(initial_dest_x_pos,initial_dest_y_pos,dest_width,dest_height);
}

/// Returns the minimum of 4 integers.
int Min4(long int a, long int b, long int c, long int d)
{
  if (a<b)
    if (c<d)
      return a<c?a:c;
    else
      return a<d?a:d;
  else
    if (c<d)
      return b<c?b:c;
    else
      return b<d?b:d;
}

//-------------------- Brush distort -------------------------------

// That's a lot of globals, but it saves stack space in the recursive calls.
static Func_pixel Pixel_for_distort;
static byte *     Distort_buffer;
static short      Distort_buffer_width;

/// Draw a pixel in the target buffer. No clipping.
void Pixel_in_distort_buffer(word x_pos,word y_pos,byte color)
{
  *(Distort_buffer+y_pos*Distort_buffer_width+x_pos)=color;
}

/// Returns the maximum of 4 integers.
int Max4( long int a, long int b, long int c, long int d)
{
  if (a>b)
    if (c>d)
      return a>c?a:c;
    else
      return a>d?a:d;
  else
    if (c>d)
      return b>c?b:c;
    else
      return b>d?b:d;
}

// Recursive function for linear distortion.
void Draw_brush_linear_distort(unsigned long int tex_min_x,
                               unsigned long int tex_min_y,
                               unsigned long int tex_max_x,
                               unsigned long int tex_max_y,
                               long int x1,
                               long int y1,
                               long int x2,
                               long int y2,
                               long int x3,
                               long int y3,
                               long int x4,
                               long int y4)
{
  static byte color;
  // bounding rectangle
  static long int min_x, max_x, min_y, max_y;
  
  min_x=Min4(x1,x2,x3,x4);
  max_x=Max4(x1,x2,x3,x4);
  min_y=Min4(y1,y2,y3,y4);
  max_y=Max4(y1,y2,y3,y4);
   
  if ((max_x>>16) - (min_x>>16) <= 1 && (max_y>>16) - (min_y>>16) <= 1)
  //if (max_x - min_x <= 1<<16 && max_y - min_y <= 1<<16)
  {
    if ((min_x<(max_x&0x7FFF0000)) && (min_y<(max_y&0x7FFF0000)))  
    {
      color=Read_pixel_from_brush((tex_min_x)>>16,(tex_min_y)>>16);
      if (color!=Back_color)
        Pixel_for_distort(min_x>>16,min_y>>16,color);
    }
    return;
  }
  // Cut in 4 quarters and repeat
  // "top left"
  Draw_brush_linear_distort(tex_min_x,
                            tex_min_y,
                            (tex_min_x+tex_max_x)>>1,
                            (tex_min_y+tex_max_y)>>1,
                            x1,
                            y1,
                            (x1+x2)>>1,
                            (y1+y2)>>1,
                            (x1+x2+x3+x4)>>2,
                            (y1+y2+y3+y4)>>2,
                            (x1+x4)>>1,
                            (y1+y4)>>1);

  // "top right"
  Draw_brush_linear_distort((tex_min_x+tex_max_x)>>1,
                            tex_min_y,
                            tex_max_x,
                            (tex_min_y+tex_max_y)>>1,
                            (x1+x2)>>1,
                            (y1+y2)>>1,
                            x2,
                            y2,
                            (x2+x3)>>1,
                            (y2+y3)>>1,
                            (x1+x2+x3+x4)>>2,
                            (y1+y2+y3+y4)>>2);

  // "bottom right"
  Draw_brush_linear_distort((tex_min_x+tex_max_x)>>1,
                            (tex_min_y+tex_max_y)>>1,
                            tex_max_x,
                            tex_max_y,
                            (x1+x2+x3+x4)>>2,
                            (y1+y2+y3+y4)>>2,
                            (x2+x3)>>1,
                            (y2+y3)>>1,
                            x3,
                            y3,
                            (x3+x4)>>1,
                            (y3+y4)>>1);
  
  // "bottom left"
  Draw_brush_linear_distort(tex_min_x,
                            (tex_min_y+tex_max_y)>>1,
                            (tex_min_x+tex_max_x)>>1,
                            tex_max_y,
                            (x1+x4)>>1,
                            (y1+y4)>>1,
                            (x1+x2+x3+x4)>>2,
                            (y1+y2+y3+y4)>>2,
                            (x3+x4)>>1,
                            (y3+y4)>>1,
                            x4,
                            y4);

  return;
}

/// Draws a distorted version of the brush, mapped over the given quad (picture coordinates).
void Distort_brush_preview(short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4)
{
  Pixel_for_distort=Pixel_figure_preview;
  Draw_brush_linear_distort(0, 0, (Brush_width<<16), (Brush_height<<16), (x1<<16), (y1<<16), (x2<<16), (y2<<16), (x3<<16), (y3<<16), (x4<<16), (y4<<16));
}

/// Modifies the current brush, mapping it over the given quad.
void Distort_brush(short x1, short y1, short x2, short y2, short x3, short y3, short x4, short y4)
{
  short min_x, max_x, min_y, max_y;
  short width, height;
  byte * new_brush;
  byte * new_smear_brush;
  short new_smear_brush_width;
  short new_smear_brush_height;
  
  // Move all coordinates to start on (0,0)
  min_x=Min4(x1,x2,x3,x4);
  max_x=Max4(x1,x2,x3,x4);
  min_y=Min4(y1,y2,y3,y4);
  max_y=Max4(y1,y2,y3,y4);

  x1-=min_x;
  x2-=min_x;
  x3-=min_x;
  x4-=min_x;

  y1-=min_y;
  y2-=min_y;
  y3-=min_y;
  y4-=min_y;
  
  width=Max(max_x-min_x, 1);
  height=Max(max_y-min_y, 1);
    
  new_smear_brush_width=(width>MAX_PAINTBRUSH_SIZE)?width:MAX_PAINTBRUSH_SIZE;
  new_smear_brush_height=(height>MAX_PAINTBRUSH_SIZE)?height:MAX_PAINTBRUSH_SIZE;

  new_smear_brush=(byte *)malloc(((long)new_smear_brush_height)*new_smear_brush_width);
  if (! new_smear_brush)
  {
    // Out of memory while allocating new smear brush
    Error(0);
    return;
  }
  
  new_brush=((byte *)malloc((long)width*height));
  if (!new_brush)
  {
    // Out of memory while allocating new brush
    Error(0);
    free(new_smear_brush);
    return;
  }

  // Fill the new brush with backcolor, originally.
  memset(new_brush,Back_color,((long)width)*height);
  
  // Call distort routine
  Pixel_for_distort=Pixel_in_distort_buffer;
  Distort_buffer=new_brush;
  Distort_buffer_width=width;
  Draw_brush_linear_distort(0, 0, (Brush_width<<16), (Brush_height<<16), (x1<<16), (y1<<16), (x2<<16), (y2<<16), (x3<<16), (y3<<16), (x4<<16), (y4<<16));

  // Free old brushes
  free(Smear_brush);
  free(Brush);

  // Point to the new ones
  Brush=new_brush;
  Brush_width=width;
  Brush_height=height;

  Smear_brush=new_smear_brush;
  Smear_brush_width=new_smear_brush_width;
  Smear_brush_height=new_smear_brush_height;

  // Re-center brush handle  
  Brush_offset_X=(Brush_width>>1);
  Brush_offset_Y=(Brush_height>>1);
  
}

//------------------------- Rotation de la brosse ---------------------------

#define UNDEFINED (-1.0e20F)
float * ScanY_Xt[2];
float * ScanY_Yt[2];
float * ScanY_X[2];


void Interpolate_texture(int start_x,int start_y,int xt1,int yt1,
                        int end_x  ,int end_y  ,int xt2,int yt2,int height)
{
  int x_pos,y_pos;
  int incr_x,incr_y;
  int i,cumul;
  int delta_x,delta_y;
  int delta_xt=xt2-xt1;
  int delta_yt=yt2-yt1;
  int delta_x2=end_x-start_x;
  int delta_y2=end_y-start_y;
  float xt,yt;


  x_pos=start_x;
  y_pos=start_y;

  if (start_x<end_x)
  {
    incr_x=+1;
    delta_x=delta_x2;
  }
  else
  {
    incr_x=-1;
    delta_x=-delta_x2;
  }

  if (start_y<end_y)
  {
    incr_y=+1;
    delta_y=delta_y2;
  }
  else
  {
    incr_y=-1;
    delta_y=-delta_y2;
  }

  if (delta_x>delta_y)
  {
    cumul=delta_x>>1;
    for (i=0; i<=delta_x; i++)
    {
      if (cumul>=delta_x)
      {
        cumul-=delta_x;
        y_pos+=incr_y;
      }

      if ((y_pos>=0) && (y_pos<height))
      {
        xt=(((float)((x_pos-start_x)*delta_xt))/(float)delta_x2) + (float)xt1;
        yt=(((float)((x_pos-start_x)*delta_yt))/(float)delta_x2) + (float)yt1;
        if (ScanY_X[0][y_pos]==UNDEFINED) // Gauche non défini
        {
          ScanY_X[0][y_pos]=x_pos;
          ScanY_Xt[0][y_pos]=xt;
          ScanY_Yt[0][y_pos]=yt;
        }
        else
        {
          if (x_pos>=ScanY_X[0][y_pos])
          {
            if ((ScanY_X[1][y_pos]==UNDEFINED) // Droit non défini
             || (x_pos>ScanY_X[1][y_pos]))
            {
              ScanY_X[1][y_pos]=x_pos;
              ScanY_Xt[1][y_pos]=xt;
              ScanY_Yt[1][y_pos]=yt;
            }
          }
          else
          {
            if (ScanY_X[1][y_pos]==UNDEFINED) // Droit non défini
            {
              ScanY_X[1][y_pos]=ScanY_X[0][y_pos];
              ScanY_Xt[1][y_pos]=ScanY_Xt[0][y_pos];
              ScanY_Yt[1][y_pos]=ScanY_Yt[0][y_pos];
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=xt;
              ScanY_Yt[0][y_pos]=yt;
            }
            else
            {
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=xt;
              ScanY_Yt[0][y_pos]=yt;
            }
          }
        }
      }
      x_pos+=incr_x;
      cumul+=delta_y;
    }
  }
  else
  {
    cumul=delta_y>>1;
    for (i=0; i<=delta_y; i++)
    {
      if (cumul>=delta_y)
      {
        cumul-=delta_y;
        x_pos+=incr_x;
      }

      if ((y_pos>=0) && (y_pos<height))
      {
        xt=(((float)((y_pos-start_y)*delta_xt))/(float)delta_y2) + (float)xt1;
        yt=(((float)((y_pos-start_y)*delta_yt))/(float)delta_y2) + (float)yt1;
        if (ScanY_X[0][y_pos]==UNDEFINED) // Gauche non défini
        {
          ScanY_X[0][y_pos]=x_pos;
          ScanY_Xt[0][y_pos]=xt;
          ScanY_Yt[0][y_pos]=yt;
        }
        else
        {
          if (x_pos>=ScanY_X[0][y_pos])
          {
            if ((ScanY_X[1][y_pos]==UNDEFINED) // Droit non défini
             || (x_pos>ScanY_X[1][y_pos]))
            {
              ScanY_X[1][y_pos]=x_pos;
              ScanY_Xt[1][y_pos]=xt;
              ScanY_Yt[1][y_pos]=yt;
            }
          }
          else
          {
            if (ScanY_X[1][y_pos]==UNDEFINED) // Droit non défini
            {
              ScanY_X[1][y_pos]=ScanY_X[0][y_pos];
              ScanY_Xt[1][y_pos]=ScanY_Xt[0][y_pos];
              ScanY_Yt[1][y_pos]=ScanY_Yt[0][y_pos];
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=xt;
              ScanY_Yt[0][y_pos]=yt;
            }
            else
            {
              ScanY_X[0][y_pos]=x_pos;
              ScanY_Xt[0][y_pos]=xt;
              ScanY_Yt[0][y_pos]=yt;
            }
          }
        }
      }
      y_pos+=incr_y;
      cumul+=delta_x;
    }
  }
}



void Compute_quad_texture(int x1,int y1,int xt1,int yt1,
                           int x2,int y2,int xt2,int yt2,
                           int x3,int y3,int xt3,int yt3,
                           int x4,int y4,int xt4,int yt4,
                           byte * buffer, int width, int height)
{
  int x_min,/*x_max,*/y_min/*,y_max*/;
  int x,y,xt,yt;
  int start_x,end_x,line_width;
  float temp;
  //byte color;

  x_min=Min(Min(x1,x2),Min(x3,x4));
  y_min=Min(Min(y1,y2),Min(y3,y4));

  ScanY_Xt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(height*sizeof(float));
  ScanY_X[0] =(float *)malloc(height*sizeof(float));
  ScanY_X[1] =(float *)malloc(height*sizeof(float));

  // Fill_general avec des valeurs égales à UNDEFINED.
  for (y=0; y<height; y++)
  {
    ScanY_X[0][y]=UNDEFINED;
    ScanY_X[1][y]=UNDEFINED;
  }

  Interpolate_texture(x1-x_min,y1-y_min,xt1,yt1,x3-x_min,y3-y_min,xt3,yt3,height);
  Interpolate_texture(x3-x_min,y3-y_min,xt3,yt3,x4-x_min,y4-y_min,xt4,yt4,height);
  Interpolate_texture(x4-x_min,y4-y_min,xt4,yt4,x2-x_min,y2-y_min,xt2,yt2,height);
  Interpolate_texture(x2-x_min,y2-y_min,xt2,yt2,x1-x_min,y1-y_min,xt1,yt1,height);

  for (y=0; y<height; y++)
  {
    start_x=Round(ScanY_X[0][y]);
    end_x  =Round(ScanY_X[1][y]);

    line_width=1+end_x-start_x;

    for (x=0; x<start_x; x++)
      buffer[x+(y*width)]=Back_color;
    for (; x<=end_x; x++)
    {
      temp=(float)(0.5+(float)x-ScanY_X[0][y])/(float)line_width;
      xt=Round((float)(ScanY_Xt[0][y])+(temp*(ScanY_Xt[1][y]-ScanY_Xt[0][y])));
      yt=Round((float)(ScanY_Yt[0][y])+(temp*(ScanY_Yt[1][y]-ScanY_Yt[0][y])));
      if (xt>=0 && yt>=0)
        buffer[x+(y*width)]=Read_pixel_from_brush(xt,yt);
    }
    for (; x<width; x++)
      buffer[x+(y*width)]=Back_color;
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}



void Rotate_brush(float angle)
{
  byte * new_brush;
  int    new_brush_width;  // Width de la nouvelle brosse
  int    new_brush_height;  // Height de la nouvelle brosse

  short x1,y1,x2,y2,x3,y3,x4,y4;
  int start_x,end_x,start_y,end_y;
  int x_min,x_max,y_min,y_max;
  float cos_a=cos(angle);
  float sin_a=sin(angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  start_x=1-(Brush_width>>1);
  start_y=1-(Brush_height>>1);
  end_x=start_x+Brush_width-1;
  end_y=start_y+Brush_height-1;

  Transform_point(start_x,start_y, cos_a,sin_a, &x1,&y1);
  Transform_point(end_x  ,start_y, cos_a,sin_a, &x2,&y2);
  Transform_point(start_x,end_y  , cos_a,sin_a, &x3,&y3);
  Transform_point(end_x  ,end_y  , cos_a,sin_a, &x4,&y4);

  // Calcul des nouvelles dimensions de la brosse:
  x_min=Min(Min((int)x1,(int)x2),Min((int)x3,(int)x4));
  x_max=Max(Max((int)x1,(int)x2),Max((int)x3,(int)x4));
  y_min=Min(Min((int)y1,(int)y2),Min((int)y3,(int)y4));
  y_max=Max(Max((int)y1,(int)y2),Max((int)y3,(int)y4));

  new_brush_width=x_max+1-x_min;
  new_brush_height=y_max+1-y_min;

  free(Smear_brush); // On libère un peu de mémoire

  if ((new_brush=((byte *)malloc(new_brush_width*new_brush_height))))
  {
    // Et maintenant on calcule la nouvelle brosse tournée.
    Compute_quad_texture(x1,y1,               0,               0,
                          x2,y2,Brush_width-1,               0,
                          x3,y3,               0,Brush_height-1,
                          x4,y4,Brush_width-1,Brush_height-1,
                          new_brush,new_brush_width,new_brush_height);

    free(Brush);
    Brush=new_brush;

    Brush_width=new_brush_width;
    Brush_height=new_brush_height;

    Smear_brush_width=(Brush_width>MAX_PAINTBRUSH_SIZE)?Brush_width:MAX_PAINTBRUSH_SIZE;
    Smear_brush_height=(Brush_height>MAX_PAINTBRUSH_SIZE)?Brush_height:MAX_PAINTBRUSH_SIZE;
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);

    if (!Smear_brush) // On ne peut même pas allouer la brosse du smear!
    {
      Error(0);

      free(Brush);
      Brush=(byte *)malloc(1*1);
      Brush_height=1;
      Brush_width=1;

      Smear_brush=(byte *)malloc(MAX_PAINTBRUSH_SIZE*MAX_PAINTBRUSH_SIZE);
      Smear_brush_height=MAX_PAINTBRUSH_SIZE;
      Smear_brush_width=MAX_PAINTBRUSH_SIZE;
    }

    Brush_offset_X=(Brush_width>>1);
    Brush_offset_Y=(Brush_height>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffit donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_brush car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_brush=(byte *)malloc(((long)Smear_brush_height)*Smear_brush_width);
    Error(0);
  }
}



void Draw_quad_texture_preview(int x1,int y1,int xt1,int yt1,
                                   int x2,int y2,int xt2,int yt2,
                                   int x3,int y3,int xt3,int yt3,
                                   int x4,int y4,int xt4,int yt4)
{
  int x_min,x_max,y_min,y_max;
  int x,y,xt,yt;
  int y_,y_min_;
  int start_x,end_x,width,height;
  float temp;
  byte color;

  x_min=Min(Min(x1,x2),Min(x3,x4));
  x_max=Max(Max(x1,x2),Max(x3,x4));
  y_min=Min(Min(y1,y2),Min(y3,y4));
  y_max=Max(Max(y1,y2),Max(y3,y4));
  height=1+y_max-y_min;

  ScanY_Xt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(height*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(height*sizeof(float));
  ScanY_X[0] =(float *)malloc(height*sizeof(float));
  ScanY_X[1] =(float *)malloc(height*sizeof(float));

  // Fill_general avec des valeurs égales à UNDEFINED.
  for (y=0; y<height; y++)
  {
    ScanY_X[0][y]=UNDEFINED;
    ScanY_X[1][y]=UNDEFINED;
  }

  Interpolate_texture(x1,y1-y_min,xt1,yt1,x3,y3-y_min,xt3,yt3,height);
  Interpolate_texture(x3,y3-y_min,xt3,yt3,x4,y4-y_min,xt4,yt4,height);
  Interpolate_texture(x4,y4-y_min,xt4,yt4,x2,y2-y_min,xt2,yt2,height);
  Interpolate_texture(x2,y2-y_min,xt2,yt2,x1,y1-y_min,xt1,yt1,height);

  y_min_=y_min;
  if (y_min<Limit_top) y_min=Limit_top;
  if (y_max>Limit_bottom)  y_max=Limit_bottom;

  for (y_=y_min; y_<=y_max; y_++)
  {
    y=y_-y_min_;
    start_x=Round(ScanY_X[0][y]);
    end_x  =Round(ScanY_X[1][y]);

    width=1+end_x-start_x;

    if (start_x<Limit_left) start_x=Limit_left;
    if (  end_x>Limit_right)   end_x=Limit_right;

    for (x=start_x; x<=end_x; x++)
    {
      temp=(float)(0.5+(float)x-ScanY_X[0][y])/(float)width;
      xt=Round((float)(ScanY_Xt[0][y])+(temp*(ScanY_Xt[1][y]-ScanY_Xt[0][y])));
      yt=Round((float)(ScanY_Yt[0][y])+(temp*(ScanY_Yt[1][y]-ScanY_Yt[0][y])));
      if (xt>=0 && yt>=0)
      {
        color=Read_pixel_from_brush(xt,yt);
        if (color!=Back_color)
          Pixel_preview(x,y_,color);
      }
    }
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}


void Rotate_brush_preview(float angle)
{
  short x1,y1,x2,y2,x3,y3,x4,y4;
  int start_x,end_x,start_y,end_y;
  float cos_a=cos(angle);
  float sin_a=sin(angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  start_x=1-(Brush_width>>1);
  start_y=1-(Brush_height>>1);
  end_x=start_x+Brush_width-1;
  end_y=start_y+Brush_height-1;

  Transform_point(start_x,start_y, cos_a,sin_a, &x1,&y1);
  Transform_point(end_x  ,start_y, cos_a,sin_a, &x2,&y2);
  Transform_point(start_x,end_y  , cos_a,sin_a, &x3,&y3);
  Transform_point(end_x  ,end_y  , cos_a,sin_a, &x4,&y4);

  x1+=Brush_rotation_center_X;
  y1+=Brush_rotation_center_Y;
  x2+=Brush_rotation_center_X;
  y2+=Brush_rotation_center_Y;
  x3+=Brush_rotation_center_X;
  y3+=Brush_rotation_center_Y;
  x4+=Brush_rotation_center_X;
  y4+=Brush_rotation_center_Y;

  // Et maintenant on dessine la brosse tournée.
  Draw_quad_texture_preview(x1,y1,               0,               0,
                                x2,y2,Brush_width-1,               0,
                                x3,y3,               0,Brush_height-1,
                                x4,y4,Brush_width-1,Brush_height-1);
  start_x=Min(Min(x1,x2),Min(x3,x4));
  end_x=Max(Max(x1,x2),Max(x3,x4));
  start_y=Min(Min(y1,y2),Min(y3,y4));
  end_y=Max(Max(y1,y2),Max(y3,y4));
  Update_part_of_screen(start_x,start_y,end_x-start_x+1,end_y-start_y+1);
}
