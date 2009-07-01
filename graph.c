/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Franck Charlet
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>

********************************************************************************

    Drawing functions and effects.

*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "struct.h"
#include "engine.h"
#include "buttons.h"
#include "pages.h"
#include "errors.h"
#include "sdlscreen.h"
#include "graph.h"
#include "misc.h"
#include "pxsimple.h"
#include "pxtall.h"
#include "pxwide.h"
#include "pxdouble.h"
#include "pxtriple.h"
#include "pxwide2.h"
#include "pxtall2.h"
#include "pxquad.h"
#include "windows.h"
#include "input.h"

// Generic pixel-drawing function.
Func_pixel Pixel_figure;

// Fonction qui met à jour la zone de l'image donnée en paramètre sur l'écran.
// Tient compte du décalage X et Y et du zoom, et fait tous les controles nécessaires
void Update_part_of_screen(short x, short y, short width, short height)
{
  short effective_w, effective_h;
  short effective_X;
  short effective_Y;
  short diff;

  // Première étape, si L ou H est négatif, on doit remettre la zone à l'endroit
  if (width < 0)
  {
    x += width;
    width = - width;
  }

  if (height < 0)
  {
    y += height;
    height = - height;
  }

  // D'abord on met à jour dans la zone écran normale
  diff = x-Main_offset_X;
  if (diff<0)
  {
    effective_w = width + diff;
    effective_X = 0;
  }
  else
  {
    effective_w = width;
    effective_X = diff;
  }
  diff = y-Main_offset_Y;
  if (diff<0)
  {
    effective_h = height + diff;
    effective_Y = 0;
  }
  else
  {
    effective_h = height;
    effective_Y = diff;
  }

  // Normalement il ne faudrait pas updater au delà du split quand on est en mode loupe,
  // mais personne ne devrait demander d'update en dehors de cette limite, même le fill est contraint
  // a rester dans la zone visible de l'image
  // ...Sauf l'affichage de brosse en preview - yr
  if(Main_magnifier_mode && effective_X + effective_w > Main_separator_position)
    effective_w = Main_separator_position - effective_X;
  else if(effective_X + effective_w > Screen_width)
    effective_w = Screen_width - effective_X;

  if(effective_Y + effective_h > Menu_Y)
    effective_h = Menu_Y - effective_Y;
  /*
  SDL_Rect r;
  r.x=effective_X;
  r.y=effective_Y;
  r.h=effective_h;
  r.w=effective_w;
  SDL_FillRect(Screen_SDL,&r,3);
  */
  Update_rect(effective_X,effective_Y,effective_w,effective_h);

  // Et ensuite dans la partie zoomée
  if(Main_magnifier_mode)
  {
    // Clipping en X
    effective_X = (x-Main_magnifier_offset_X)*Main_magnifier_factor;
    effective_Y = (y-Main_magnifier_offset_Y)*Main_magnifier_factor;
    effective_w = width * Main_magnifier_factor;
    effective_h = height * Main_magnifier_factor;

    if (effective_X < 0)
    {
      effective_w+=effective_X;
      if (effective_w<0)
        return;

      effective_X = Main_separator_position + SEPARATOR_WIDTH*Menu_factor_X;
    }
    else
      effective_X += Main_separator_position + SEPARATOR_WIDTH*Menu_factor_X;
    diff = effective_X+effective_w-Screen_width;
    if (diff>0)
    {
      effective_w -=diff;
      if (effective_w<0)
        return;
    }


    // Clipping en Y
    if (effective_Y < 0)
    {
      effective_h+=effective_Y;
      if (effective_h<0)
        return;
      effective_Y = 0;
    }
    diff = effective_Y+effective_h-Menu_Y;
    if (diff>0)
    {
      effective_h -=diff;
      if (effective_h<0)
        return;
    }

 // Très utile pour le debug :)
    /*SDL_Rect r;
    r.x=effective_X;
    r.y=effective_Y;
    r.h=effective_h;
    r.w=effective_w;
    SDL_FillRect(Screen_SDL,&r,3);*/

    Update_rect(effective_X,effective_Y,effective_w,effective_h);
  }
}



void Transform_point(short x, short y, float cos_a, float sin_a,
                       short * rx, short * ry)
{
  *rx=Round(((float)x*cos_a)+((float)y*sin_a));
  *ry=Round(((float)y*cos_a)-((float)x*sin_a));
}


//--------------------- Initialisation d'un mode vidéo -----------------------

int Init_mode_video(int width, int height, int fullscreen, int pix_ratio)
{
  int index;
  int factor;
  int pix_width;
  int pix_height;
  byte screen_changed;
  byte pixels_changed;
  int absolute_mouse_x=Mouse_X*Pixel_width;
  int absolute_mouse_y=Mouse_Y*Pixel_height;
  
  
  switch (pix_ratio)
  {
      default:
      case PIXEL_SIMPLE:
          pix_width=1;
          pix_height=1;
      break;
      case PIXEL_TALL:
          pix_width=1;
          pix_height=2;
      break;
      case PIXEL_WIDE:
          pix_width=2;
          pix_height=1;
      break;
      case PIXEL_DOUBLE:
          pix_width=2;
          pix_height=2;
      break;
      case PIXEL_TRIPLE:
          pix_width=3;
          pix_height=3;
      break;
      case PIXEL_WIDE2:
          pix_width=4;
          pix_height=2;
      break;
      case PIXEL_TALL2:
          pix_width=2;
          pix_height=4;
      break;
      case PIXEL_QUAD:
          pix_width=4;
          pix_height=4;
      break;
  }

  screen_changed = (Screen_width*Pixel_width!=width ||
                    Screen_height*Pixel_height!=height ||
                    Video_mode[Current_resolution].Fullscreen != fullscreen);

  // Valeurs raisonnables: minimum 320x200
  if (!fullscreen)
  {
    if (width < 320*pix_width)
    {
        width = 320*pix_width;
        screen_changed=1;
    }
    if (height < 200*pix_height)
    {
        height = 200*pix_height;
        screen_changed=1;
    }
    Video_mode[0].Width = width;
    Video_mode[0].Height = height;

  }
  else
  {
    if (width < 320*pix_width || height < 200*pix_height)
      return 1;
  }
  // La largeur doit être un multiple de 4
  #ifdef __amigaos4__
      // On AmigaOS the systems adds some more constraints on that ...
      width = (width + 15) & 0xFFFFFFF0;
  #else
      //width = (width + 3 ) & 0xFFFFFFFC;
  #endif  

  pixels_changed = (Pixel_ratio!=pix_ratio);
  
  if (!screen_changed && !pixels_changed)
    return 0;
  if (screen_changed)
  {
    Set_mode_SDL(&width, &height,fullscreen);
  }
  
  if (screen_changed || pixels_changed)
  {
    Pixel_ratio=pix_ratio;
    Pixel_width=pix_width;
    Pixel_height=pix_height;
    switch (Pixel_ratio)
    {
        default:
        case PIXEL_SIMPLE:
            Pixel = Pixel_simple ;
            Read_pixel= Read_pixel_simple ;
            Display_screen = Display_part_of_screen_simple ;
            Block = Block_simple ;
            Pixel_preview_normal = Pixel_preview_normal_simple ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_simple ;
            Horizontal_XOR_line = Horizontal_XOR_line_simple ;
            Vertical_XOR_line = Vertical_XOR_line_simple ;
            Display_brush_color = Display_brush_color_simple ;
            Display_brush_mono = Display_brush_mono_simple ;
            Clear_brush = Clear_brush_simple ;
            Remap_screen = Remap_screen_simple ;
            Display_line = Display_line_on_screen_simple ;
            Display_line_fast = Display_line_on_screen_simple ;
            Read_line = Read_line_screen_simple ;
            Display_zoomed_screen = Display_part_of_screen_scaled_simple ;
            Display_brush_color_zoom = Display_brush_color_zoom_simple ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_simple ;
            Clear_brush_scaled = Clear_brush_scaled_simple ;
            Display_brush = Display_brush_simple ;
        break;
        case PIXEL_TALL:
            Pixel = Pixel_tall;
            Read_pixel= Read_pixel_tall;
            Display_screen = Display_part_of_screen_tall;
            Block = Block_tall;
            Pixel_preview_normal = Pixel_preview_normal_tall;
            Pixel_preview_magnifier = Pixel_preview_magnifier_tall;
            Horizontal_XOR_line = Horizontal_XOR_line_tall;
            Vertical_XOR_line = Vertical_XOR_line_tall;
            Display_brush_color = Display_brush_color_tall;
            Display_brush_mono = Display_brush_mono_tall;
            Clear_brush = Clear_brush_tall;
            Remap_screen = Remap_screen_tall;
            Display_line = Display_line_on_screen_tall;
            Display_line_fast = Display_line_on_screen_tall;
            Read_line = Read_line_screen_tall;
            Display_zoomed_screen = Display_part_of_screen_scaled_tall;
            Display_brush_color_zoom = Display_brush_color_zoom_tall;
            Display_brush_mono_zoom = Display_brush_mono_zoom_tall;
            Clear_brush_scaled = Clear_brush_scaled_tall;
            Display_brush = Display_brush_tall;
        break;
        case PIXEL_WIDE:
            Pixel = Pixel_wide ;
            Read_pixel= Read_pixel_wide ;
            Display_screen = Display_part_of_screen_wide ;
            Block = Block_wide ;
            Pixel_preview_normal = Pixel_preview_normal_wide ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_wide ;
            Horizontal_XOR_line = Horizontal_XOR_line_wide ;
            Vertical_XOR_line = Vertical_XOR_line_wide ;
            Display_brush_color = Display_brush_color_wide ;
            Display_brush_mono = Display_brush_mono_wide ;
            Clear_brush = Clear_brush_wide ;
            Remap_screen = Remap_screen_wide ;
            Display_line = Display_line_on_screen_wide ;
            Display_line_fast = Display_line_on_screen_fast_wide ;
            Read_line = Read_line_screen_wide ;
            Display_zoomed_screen = Display_part_of_screen_scaled_wide ;
            Display_brush_color_zoom = Display_brush_color_zoom_wide ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_wide ;
            Clear_brush_scaled = Clear_brush_scaled_wide ;
            Display_brush = Display_brush_wide ;
        break;
        case PIXEL_DOUBLE:
            Pixel = Pixel_double ;
            Read_pixel= Read_pixel_double ;
            Display_screen = Display_part_of_screen_double ;
            Block = Block_double ;
            Pixel_preview_normal = Pixel_preview_normal_double ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_double ;
            Horizontal_XOR_line = Horizontal_XOR_line_double ;
            Vertical_XOR_line = Vertical_XOR_line_double ;
            Display_brush_color = Display_brush_color_double ;
            Display_brush_mono = Display_brush_mono_double ;
            Clear_brush = Clear_brush_double ;
            Remap_screen = Remap_screen_double ;
            Display_line = Display_line_on_screen_double ;
            Display_line_fast = Display_line_on_screen_fast_double ;
            Read_line = Read_line_screen_double ;
            Display_zoomed_screen = Display_part_of_screen_scaled_double ;
            Display_brush_color_zoom = Display_brush_color_zoom_double ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_double ;
            Clear_brush_scaled = Clear_brush_scaled_double ;
            Display_brush = Display_brush_double ;
        break;
        case PIXEL_TRIPLE:
            Pixel = Pixel_triple ;
            Read_pixel= Read_pixel_triple ;
            Display_screen = Display_part_of_screen_triple ;
            Block = Block_triple ;
            Pixel_preview_normal = Pixel_preview_normal_triple ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_triple ;
            Horizontal_XOR_line = Horizontal_XOR_line_triple ;
            Vertical_XOR_line = Vertical_XOR_line_triple ;
            Display_brush_color = Display_brush_color_triple ;
            Display_brush_mono = Display_brush_mono_triple ;
            Clear_brush = Clear_brush_triple ;
            Remap_screen = Remap_screen_triple ;
            Display_line = Display_line_on_screen_triple ;
            Display_line_fast = Display_line_on_screen_fast_triple ;
            Read_line = Read_line_screen_triple ;
            Display_zoomed_screen = Display_part_of_screen_scaled_triple ;
            Display_brush_color_zoom = Display_brush_color_zoom_triple ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_triple ;
            Clear_brush_scaled = Clear_brush_scaled_triple ;
            Display_brush = Display_brush_triple ;
        break;
        case PIXEL_WIDE2:
            Pixel = Pixel_wide2 ;
            Read_pixel= Read_pixel_wide2 ;
            Display_screen = Display_part_of_screen_wide2 ;
            Block = Block_wide2 ;
            Pixel_preview_normal = Pixel_preview_normal_wide2 ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_wide2 ;
            Horizontal_XOR_line = Horizontal_XOR_line_wide2 ;
            Vertical_XOR_line = Vertical_XOR_line_wide2 ;
            Display_brush_color = Display_brush_color_wide2 ;
            Display_brush_mono = Display_brush_mono_wide2 ;
            Clear_brush = Clear_brush_wide2 ;
            Remap_screen = Remap_screen_wide2 ;
            Display_line = Display_line_on_screen_wide2 ;
            Display_line_fast = Display_line_on_screen_fast_wide2 ;
            Read_line = Read_line_screen_wide2 ;
            Display_zoomed_screen = Display_part_of_screen_scaled_wide2 ;
            Display_brush_color_zoom = Display_brush_color_zoom_wide2 ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_wide2 ;
            Clear_brush_scaled = Clear_brush_scaled_wide2 ;
            Display_brush = Display_brush_wide2 ;
        break;
        case PIXEL_TALL2:
            Pixel = Pixel_tall2 ;
            Read_pixel= Read_pixel_tall2 ;
            Display_screen = Display_part_of_screen_tall2 ;
            Block = Block_tall2 ;
            Pixel_preview_normal = Pixel_preview_normal_tall2 ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_tall2 ;
            Horizontal_XOR_line = Horizontal_XOR_line_tall2 ;
            Vertical_XOR_line = Vertical_XOR_line_tall2 ;
            Display_brush_color = Display_brush_color_tall2 ;
            Display_brush_mono = Display_brush_mono_tall2 ;
            Clear_brush = Clear_brush_tall2 ;
            Remap_screen = Remap_screen_tall2 ;
            Display_line = Display_line_on_screen_tall2 ;
            Display_line_fast = Display_line_on_screen_fast_tall2 ;
            Read_line = Read_line_screen_tall2 ;
            Display_zoomed_screen = Display_part_of_screen_scaled_tall2 ;
            Display_brush_color_zoom = Display_brush_color_zoom_tall2 ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_tall2 ;
            Clear_brush_scaled = Clear_brush_scaled_tall2 ;
            Display_brush = Display_brush_tall2 ;
        break;
        case PIXEL_QUAD:
            Pixel = Pixel_quad ;
            Read_pixel= Read_pixel_quad ;
            Display_screen = Display_part_of_screen_quad ;
            Block = Block_quad ;
            Pixel_preview_normal = Pixel_preview_normal_quad ;
            Pixel_preview_magnifier = Pixel_preview_magnifier_quad ;
            Horizontal_XOR_line = Horizontal_XOR_line_quad ;
            Vertical_XOR_line = Vertical_XOR_line_quad ;
            Display_brush_color = Display_brush_color_quad ;
            Display_brush_mono = Display_brush_mono_quad ;
            Clear_brush = Clear_brush_quad ;
            Remap_screen = Remap_screen_quad ;
            Display_line = Display_line_on_screen_quad ;
            Display_line_fast = Display_line_on_screen_fast_quad ;
            Read_line = Read_line_screen_quad ;
            Display_zoomed_screen = Display_part_of_screen_scaled_quad ;
            Display_brush_color_zoom = Display_brush_color_zoom_quad ;
            Display_brush_mono_zoom = Display_brush_mono_zoom_quad ;
            Clear_brush_scaled = Clear_brush_scaled_quad ;
            Display_brush = Display_brush_quad ;
        break;
    }
  }
  Screen_width = width/Pixel_width;
  Screen_height = height/Pixel_height;

  Clear_border(MC_Black); // Requires up-to-date Screen_* and Pixel_*

  // Taille des menus
  if (Screen_width/320 > Screen_height/200)
    factor=Screen_height/200;
  else
    factor=Screen_width/320;

  switch (Config.Ratio)
  {
    case 1: // adapter tout
      Menu_factor_X=factor;
      Menu_factor_Y=factor;
      break;
    case 2: // adapter légèrement
      Menu_factor_X=factor-1;
      if (Menu_factor_X<1) Menu_factor_X=1;
      Menu_factor_Y=factor-1;
      if (Menu_factor_Y<1) Menu_factor_Y=1;
      break;
    default: // ne pas adapter
      Menu_factor_X=1;
      Menu_factor_Y=1;
  }
  if (Pixel_height>Pixel_width && Screen_width>=Menu_factor_X*2*320)
    Menu_factor_X*=2;
  else if (Pixel_width>Pixel_height && Screen_height>=Menu_factor_Y*2*200)
    Menu_factor_Y*=2;
    
  if (Horizontal_line_buffer)
    free(Horizontal_line_buffer);
  Horizontal_line_buffer=(byte *)malloc(Pixel_width*((Screen_width>Main_image_width)?Screen_width:Main_image_width));

  Set_palette(Main_palette);

  Current_resolution=0;
  if (fullscreen)
  {
    for (index=1; index<Nb_video_modes; index++)
    {
      if (Video_mode[index].Width/Pixel_width==Screen_width &&
          Video_mode[index].Height/Pixel_height==Screen_height)
      {
        Current_resolution=index;
        break;
      }
    }
  }

  Change_palette_cells();
  
  Menu_Y = Screen_height;
  if (Menu_is_visible)
    Menu_Y -= MENU_HEIGHT * Menu_factor_Y;
  Menu_status_Y = Screen_height-(Menu_factor_Y<<3);

  Adjust_mouse_sensitivity(fullscreen);

  Mouse_X=absolute_mouse_x/Pixel_width;
  if (Mouse_X>=Screen_width)
    Mouse_X=Screen_width-1;
  Mouse_Y=absolute_mouse_y/Pixel_height;
  if (Mouse_Y>=Screen_height)
    Mouse_Y=Screen_height-1;
  if (fullscreen)
    Set_mouse_position();
  
  Spare_offset_X=0; // |  Il faut penser à éviter les incohérences
  Spare_offset_Y=0; // |- de décalage du brouillon par rapport à
  Spare_magnifier_mode=0; // |  la résolution.

  if (Main_magnifier_mode)
  {
    Pixel_preview=Pixel_preview_magnifier;
  }
  else
  {
    Pixel_preview=Pixel_preview_normal;
    // Recaler la vue (meme clipping que dans Scroll_screen())
    if (Main_offset_X+Screen_width>Main_image_width)
      Main_offset_X=Main_image_width-Screen_width;
    if (Main_offset_X<0)
      Main_offset_X=0;
    if (Main_offset_Y+Menu_Y>Main_image_height)
      Main_offset_Y=Main_image_height-Menu_Y;
    if (Main_offset_Y<0)
      Main_offset_Y=0;
  }

  Compute_magnifier_data();
  if (Main_magnifier_mode)
    Position_screen_according_to_zoom();
  Compute_limits();
  Compute_paintbrush_coordinates();
  Display_all_screen();
  
  Resize_width=0;
  Resize_height=0;
  return 0;
}



  // -- Redimentionner l'image (nettoie l'écran virtuel) --

void Resize_image(word chosen_width,word chosen_height)
{
  word old_width=Main_image_width;
  word old_height=Main_image_height;

  // +-+-+
  // |C| |  A+B+C = Ancienne image
  // +-+A|
  // |B| |    C   = Nouvelle image
  // +-+-+

  if (Backup_with_new_dimensions(1,chosen_width,chosen_height))
  {
    // La nouvelle page a pu être allouée, elle est pour l'instant pleine de
    // 0s. Elle fait Main_image_width de large.

    Main_image_is_modified=1;

    // On copie donc maintenant la partie C dans la nouvelle image.
    Copy_part_of_image_to_another(
      Screen_backup,0,0,Min(old_width,Main_image_width),
      Min(old_height,Main_image_height),old_width,
      Main_screen,0,0,Main_image_width);
  }
  else
  {
    // Afficher un message d'erreur
    Display_cursor();
    Message_out_of_memory();
    Hide_cursor();
  }
}



void Remap_picture(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  used[256]; // Tableau de booléens "La couleur est utilisée"
  int   color;

  // On commence par initialiser le tableau de booléens à faux
  for (color=0;color<=255;color++)
    used[color]=0;

  // On calcule la table d'utilisation des couleurs
  for (y_pos=0;y_pos<Spare_image_height;y_pos++)
    for (x_pos=0;x_pos<Spare_image_width;x_pos++)
      used[Read_pixel_from_spare_screen(x_pos,y_pos)]=1;

  //   On va maintenant se servir de la table "used" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans l'image, donc elles
  //       ne seront pas utilisées par Remap_general_lowlevel.
  for (color=0;color<=255;color++)
    if (used[color])
      used[color]=Best_color(Spare_palette[color].R,Spare_palette[color].G,Spare_palette[color].B);

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_lowlevel(used,Spare_screen,Spare_image_width,Spare_image_height,Spare_image_width);
}



void Get_colors_from_brush(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  used[256]; // Tableau de booléens "La couleur est utilisée"
  int   color;

  if (Confirmation_box("Modify current palette ?"))
  {
    Backup();

    // On commence par initialiser le tableau de booléen à faux
    for (color=0;color<=255;color++)
      used[color]=0;

    // On calcule la table d'utilisation des couleurs
    for (y_pos=0;y_pos<Brush_height;y_pos++)
      for (x_pos=0;x_pos<Brush_width;x_pos++)
        used[Read_pixel_from_brush(x_pos,y_pos)]=1;

    // On recopie dans la palette principale les teintes des couleurs utilisées
    // dans la palette du brouillon
    for (color=0;color<=255;color++)
      if (used[color])
      {
        Main_palette[color].R=Spare_palette[color].R;
        Main_palette[color].G=Spare_palette[color].G;
        Main_palette[color].B=Spare_palette[color].B;
      }

    Set_palette(Main_palette);
    Compute_optimal_menu_colors(Main_palette);
    Hide_cursor();
    Display_all_screen();
    Display_menu();
    Display_cursor();

    Main_image_is_modified=1;
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////////////////// GESTION DU FILLER /////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void Fill(short * top_reached  , short * bottom_reached,
          short * left_reached, short * right_reached)
//
//   Cette fonction fait un remplissage classique d'une zone délimitée de
// l'image. Les limites employées sont Limit_top, Limit_bottom, Limit_left
// et Limit_right. Le point de départ du remplissage est Paintbrush_X,Paintbrush_Y
// et s'effectue en théorie sur la couleur 1 et emploie la couleur 2 pour le
// remplissage. Ces restrictions sont dûes à l'utilisation qu'on en fait dans
// la fonction principale "Fill_general", qui se charge de faire une gestion de
// tous les effets.
//   Cette fonction ne doit pas être directement appelée.
//
{
  short x_pos;   // Abscisse de balayage du segment, utilisée lors de l'"affichage"
  short line;   // Ordonnée de la ligne en cours de traitement
  short start_x; // Abscisse de départ du segment traité
  short end_x;   // Abscisse de fin du segment traité
  int   changes_made;    // Booléen "On a fait une modif dans le dernier passage"
  int   can_propagate; // Booléen "On peut propager la couleur dans le segment"
  short current_limit_bottom;  // Intervalle vertical restreint
  short current_limit_top;
  int   line_is_modified;       // Booléen "On a fait une modif dans la ligne"

  changes_made=1;
  current_limit_top=Paintbrush_Y;
  current_limit_bottom =Min(Paintbrush_Y+1,Limit_bottom);
  *left_reached=Paintbrush_X;
  *right_reached=Paintbrush_X+1;
  Pixel_in_current_screen(Paintbrush_X,Paintbrush_Y,2);

  while (changes_made)
  {
    changes_made=0;

    for (line=current_limit_top;line<=current_limit_bottom;line++)
    {
      line_is_modified=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      start_x=Limit_left;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (start_x<=Limit_right)
      {
        // On cherche son début
        while((start_x<=Limit_right) &&
                (Read_pixel_from_current_screen(start_x,line)!=1))
             start_x++;

        if (start_x<=Limit_right)
        {
          // Un segment de couleur 1 existe et commence à la position start_x.
          // On va donc en chercher la fin.
          for (end_x=start_x+1;(end_x<=Limit_right) &&
               (Read_pixel_from_current_screen(end_x,line)==1);end_x++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // start_x et qui se termine en end_x-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          can_propagate=(
            // Test de la présence d'un point à gauche du segment
            ((start_x>Limit_left) &&
             (Read_pixel_from_current_screen(start_x-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((end_x-1<Limit_right) &&
             (Read_pixel_from_current_screen(end_x    ,line)==2))
                               );

          // Test de la présence d'un point en haut du segment
          if (!can_propagate && (line>Limit_top))
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              if (Read_pixel_from_current_screen(x_pos,line-1)==2)
              {
                can_propagate=1;
                break;
              }

          if (can_propagate)
          {
            if (start_x<*left_reached)
              *left_reached=start_x;
            if (end_x>*right_reached)
              *right_reached=end_x;
            // On remplit le segment de start_x à end_x-1.
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              Pixel_in_current_screen(x_pos,line,2);
            // On vient d'effectuer des modifications.
            changes_made=1;
            line_is_modified=1;
          }

          start_x=end_x+1;
        }
      }

      // Si on est en bas, et qu'on peut se propager vers le bas...
      if ( (line==current_limit_bottom) &&
           (line_is_modified) &&
           (current_limit_bottom<Limit_bottom) )
        current_limit_bottom++; // On descend cette limite vers le bas
    }

    // Pour le prochain balayage vers le haut, on va se permettre d'aller
    // voir une ligne plus haut.
    // Si on ne le fait pas, et que la première ligne (current_limit_top)
    // n'était pas modifiée, alors cette limite ne serait pas remontée, donc
    // le filler ne progresserait pas vers le haut.
    if (current_limit_top>Limit_top)
      current_limit_top--;

    for (line=current_limit_bottom;line>=current_limit_top;line--)
    {
      line_is_modified=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      start_x=Limit_left;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (start_x<=Limit_right)
      {
        // On cherche son début
        for (;(start_x<=Limit_right) &&
             (Read_pixel_from_current_screen(start_x,line)!=1);start_x++);

        if (start_x<=Limit_right)
        {
          // Un segment de couleur 1 existe et commence à la position start_x.
          // On va donc en chercher la fin.
          for (end_x=start_x+1;(end_x<=Limit_right) &&
               (Read_pixel_from_current_screen(end_x,line)==1);end_x++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // start_x et qui se termine en end_x-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          can_propagate=(
            // Test de la présence d'un point à gauche du segment
            ((start_x>Limit_left) &&
             (Read_pixel_from_current_screen(start_x-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((end_x-1<Limit_right) &&
             (Read_pixel_from_current_screen(end_x    ,line)==2))
                               );

          // Test de la présence d'un point en bas du segment
          if (!can_propagate && (line<Limit_bottom))
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              if (Read_pixel_from_current_screen(x_pos,line+1)==2)
              {
                can_propagate=1;
                break;
              }

          if (can_propagate)
          {
            if (start_x<*left_reached)
              *left_reached=start_x;
            if (end_x>*right_reached)
              *right_reached=end_x;
            // On remplit le segment de start_x à end_x-1.
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              Pixel_in_current_screen(x_pos,line,2);
            // On vient d'effectuer des modifications.
            changes_made=1;
            line_is_modified=1;
          }

          start_x=end_x+1;
        }
      }

      // Si on est en haut, et qu'on peut se propager vers le haut...
      if ( (line==current_limit_top) &&
           (line_is_modified) &&
           (current_limit_top>Limit_top) )
        current_limit_top--; // On monte cette limite vers le haut
    }
  }

  *top_reached=current_limit_top;
  *bottom_reached =current_limit_bottom;
  (*right_reached)--;
} // end de la routine de remplissage "Fill"


void Fill_general(byte fill_color)
//
//  Cette fonction fait un remplissage qui gère tous les effets. Elle fait
// appel à "Fill()".
//
{
  byte   cursor_shape_before_fill;
  byte * old_fx_feedback_screen;
  short  x_pos,y_pos;
  short  top_reached  ,bottom_reached;
  short  left_reached,right_reached;
  byte   replace_table[256];


  // Avant toute chose, on vérifie que l'on n'est pas en train de remplir
  // en dehors de l'image:

  if ( (Paintbrush_X>=Limit_left) &&
       (Paintbrush_X<=Limit_right) &&
       (Paintbrush_Y>=Limit_top)   &&
       (Paintbrush_Y<=Limit_bottom) )
  {
    // On suppose que le curseur est déjà caché.
    // Hide_cursor();

    //   On va faire patienter l'utilisateur en lui affichant un joli petit
    // sablier:
    cursor_shape_before_fill=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    // On commence par effectuer un backup de l'image.
    Backup();

    // On fait attention au Feedback qui DOIT se faire avec le backup.
    old_fx_feedback_screen=FX_feedback_screen;
    FX_feedback_screen=Screen_backup;

    // On va maintenant "épurer" la zone visible de l'image:
    memset(replace_table,0,256);
    replace_table[Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y)]=1;
    Replace_colors_within_limits(replace_table);

    // On fait maintenant un remplissage classique de la couleur 1 avec la 2
   Fill(&top_reached  ,&bottom_reached,
         &left_reached,&right_reached);

    //  On s'apprête à faire des opérations qui nécessitent un affichage. Il
    // faut donc retirer de l'écran le curseur:
    Hide_cursor();
    Cursor_shape=cursor_shape_before_fill;

    //  Il va maintenant falloir qu'on "turn" ce gros caca "into" un truc qui
    // ressemble un peu plus à ce à quoi l'utilisateur peut s'attendre.
    if (top_reached>Limit_top)
      Copy_part_of_image_to_another(Screen_backup,                    // source
                                               Limit_left,Limit_top,       // Pos X et Y dans source
                                               (Limit_right-Limit_left)+1, // width copie
                                               top_reached-Limit_top,// height copie
                                               Main_image_width,         // width de la source
                                               Main_screen,                 // Destination
                                               Limit_left,Limit_top,       // Pos X et Y destination
                                               Main_image_width);        // width destination
    if (bottom_reached<Limit_bottom)
      Copy_part_of_image_to_another(Screen_backup,
                                               Limit_left,bottom_reached+1,
                                               (Limit_right-Limit_left)+1,
                                               Limit_bottom-bottom_reached,
                                               Main_image_width,Main_screen,
                                               Limit_left,bottom_reached+1,Main_image_width);
    if (left_reached>Limit_left)
      Copy_part_of_image_to_another(Screen_backup,
                                               Limit_left,top_reached,
                                               left_reached-Limit_left,
                                               (bottom_reached-top_reached)+1,
                                               Main_image_width,Main_screen,
                                               Limit_left,top_reached,Main_image_width);
    if (right_reached<Limit_right)
      Copy_part_of_image_to_another(Screen_backup,
                                               right_reached+1,top_reached,
                                               Limit_right-right_reached,
                                               (bottom_reached-top_reached)+1,
                                               Main_image_width,Main_screen,
                                               right_reached+1,top_reached,Main_image_width);

    for (y_pos=top_reached;y_pos<=bottom_reached;y_pos++)
      for (x_pos=left_reached;x_pos<=right_reached;x_pos++)
        if (Read_pixel_from_current_screen(x_pos,y_pos)==2)
        {
          //   Si le pixel en cours de traitement a été touché par le Fill()
          // on se doit d'afficher le pixel modifié par la couleur de
          // remplissage:

          //  Ceci se fait en commençant par restaurer la couleur qu'il y avait
          // précédemment (c'est important pour que les effets ne s'emmèlent
          // pas le pinceaux)
          Pixel_in_current_screen(x_pos,y_pos,Read_pixel_from_backup_screen(x_pos,y_pos));

          //  Enfin, on peut afficher le pixel, en le soumettant aux effets en
          // cours:
          Display_pixel(x_pos,y_pos,fill_color);
        }
        else
          Pixel_in_current_screen(x_pos,y_pos,Read_pixel_from_backup_screen(x_pos,y_pos));

    FX_feedback_screen=old_fx_feedback_screen;

    //   A la fin, on n'a pas besoin de réafficher le curseur puisque c'est
    // l'appelant qui s'en charge, et on n'a pas besoin de rafficher l'image
    // puisque les seuls points qui ont changé dans l'image ont été raffichés
    // par l'utilisation de "Display_pixel()", et que les autres... eh bein
    // on n'y a jamais touché à l'écran les autres: ils sont donc corrects.

    Update_rect(0,0,0,0);
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////// TRACéS DE FIGURES GéOMéTRIQUES STANDARDS //////////////////
////////////////////////// avec gestion de previews //////////////////////////
//////////////////////////////////////////////////////////////////////////////


  // Affichage d'un point de façon définitive (utilisation du pinceau)
  void Pixel_figure_permanent(word x_pos,word y_pos,byte color)
  {
    Display_paintbrush(x_pos,y_pos,color,0);
  }

  // Affichage d'un point de façon définitive
  void Pixel_clipped(word x_pos,word y_pos,byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
    Display_pixel(x_pos,y_pos,color);
  }
  
  // Affichage d'un point pour une preview
  void Pixel_figure_preview(word x_pos,word y_pos,byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,color);
  }
  // Affichage d'un point pour une preview, avec sa propre couleur
  void Pixel_figure_preview_auto(word x_pos,word y_pos)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,Read_pixel_from_current_screen(x_pos,y_pos));
  }

  // Affichage d'un point pour une preview en xor
  void Pixel_figure_preview_xor(word x_pos,word y_pos,__attribute__((unused)) byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,~Read_pixel(x_pos-Main_offset_X,
                                           y_pos-Main_offset_Y));
  }
  
  // Affichage d'un point pour une preview en xor additif
  // (Il lit la couleur depuis la page backup)
  void Pixel_figure_preview_xorback(word x_pos,word y_pos,__attribute__((unused)) byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,~Screen_backup[x_pos+y_pos*Main_image_width]);
  }
  

  // Effacement d'un point de preview
  void Pixel_figure_clear_preview(word x_pos,word y_pos,__attribute__((unused)) byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,Read_pixel_from_current_screen(x_pos,y_pos));
  }

  // Affichage d'un point dans la brosse
  void Pixel_figure_in_brush(word x_pos,word y_pos,byte color)
  {
    x_pos-=Brush_offset_X;
    y_pos-=Brush_offset_Y;
    if ( (x_pos<Brush_width) && // Les pos sont des word donc jamais < 0 ...
         (y_pos<Brush_height) )
      Pixel_in_brush(x_pos,y_pos,color);
  }


  // -- Tracer général d'un cercle vide -------------------------------------

void Draw_empty_circle_general(short center_x,short center_y,short radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;

  // Ensuite, on va parcourire le quart haut gauche du cercle
  start_x=center_x-radius;
  start_y=center_y-radius;

  // Affichage des extremitées du cercle sur chaque quart du cercle:
  for (y_pos=start_y,Circle_cursor_Y=-radius;y_pos<center_y;y_pos++,Circle_cursor_Y++)
    for (x_pos=start_x,Circle_cursor_X=-radius;x_pos<center_x;x_pos++,Circle_cursor_X++)
      if (Pixel_in_circle())
      {
        // On vient de tomber sur le premier point sur la ligne horizontale
        // qui fait partie du cercle.
        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,color);
         // Quart Haut-droite
        Pixel_figure((center_x<<1)-x_pos,y_pos,color);
         // Quart Bas-droite
        Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(center_y<<1)-y_pos,color);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas au cercle:
        for (Circle_cursor_Y--,x_pos++,Circle_cursor_X++;x_pos<center_x;x_pos++,Circle_cursor_X++)
          if (!Pixel_in_circle())
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,color);
             // Quart Haut-droite
            Pixel_figure((center_x<<1)-x_pos,y_pos,color);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
             // Quart Bas-droite
            Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
          }
          else
            break;

        Circle_cursor_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:
  Pixel_figure(center_x,center_y-radius,color); // Haut
  Pixel_figure(center_x-radius,center_y,color); // Gauche
  Pixel_figure(center_x+radius,center_y,color); // Droite
  Pixel_figure(center_x,center_y+radius,color); // Bas

  if(Main_magnifier_mode) Update_part_of_screen(center_x-radius,center_y-radius,2*radius+1,2*radius+1);
}

  // -- Tracé définitif d'un cercle vide --

void Draw_empty_circle_permanent(short center_x,short center_y,short radius,byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Draw_empty_circle_general(center_x,center_y,radius,color);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Tracer la preview d'un cercle vide --

void Draw_empty_circle_preview(short center_x,short center_y,short radius,byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_empty_circle_general(center_x,center_y,radius,color);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Effacer la preview d'un cercle vide --

void Hide_empty_circle_preview(short center_x,short center_y,short radius)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_empty_circle_general(center_x,center_y,radius,0);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Tracer un cercle plein --

void Draw_filled_circle(short center_x,short center_y,short radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  short end_x;
  short end_y;

  start_x=center_x-radius;
  start_y=center_y-radius;
  end_x=center_x+radius;
  end_y=center_y+radius;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage du cercle
  for (y_pos=start_y,Circle_cursor_Y=(long)start_y-center_y;y_pos<=end_y;y_pos++,Circle_cursor_Y++)
    for (x_pos=start_x,Circle_cursor_X=(long)start_x-center_x;x_pos<=end_x;x_pos++,Circle_cursor_X++)
      if (Pixel_in_circle())
        Display_pixel(x_pos,y_pos,color);

  Update_part_of_screen(start_x,start_y,end_x+1-start_x,end_y+1-start_y);
}


  // -- Tracer général d'une ellipse vide -----------------------------------

void Draw_empty_ellipse_general(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;

  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1,vertical_radius+1);

  // Affichage des extremitées de l'ellipse sur chaque quart de l'ellipse:
  for (y_pos=start_y,Ellipse_cursor_Y=-vertical_radius;y_pos<center_y;y_pos++,Ellipse_cursor_Y++)
    for (x_pos=start_x,Ellipse_cursor_X=-horizontal_radius;x_pos<center_x;x_pos++,Ellipse_cursor_X++)
      if (Pixel_in_ellipse())
      {
        // On vient de tomber sur le premier point qui sur la ligne
        // horizontale fait partie de l'ellipse.

        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,color);
         // Quart Haut-droite
        Pixel_figure((center_x<<1)-x_pos,y_pos,color);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
         // Quart Bas-droite
        Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas à l'ellipse:
        for (Ellipse_cursor_Y--,x_pos++,Ellipse_cursor_X++;x_pos<center_x;x_pos++,Ellipse_cursor_X++)
          if (!Pixel_in_ellipse())
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,color);
             // Quart Haut-droite
            Pixel_figure((center_x<<1)-x_pos,y_pos,color);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
             // Quart Bas-droite
            Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
          }
          else
            break;

        Ellipse_cursor_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:

  // points verticaux:
  x_pos=center_x;
  Ellipse_cursor_X=-1;
  for (y_pos=center_y+1-vertical_radius,Ellipse_cursor_Y=-vertical_radius+1;y_pos<center_y+vertical_radius;y_pos++,Ellipse_cursor_Y++)
    if (!Pixel_in_ellipse())
      Pixel_figure(x_pos,y_pos,color);

  // points horizontaux:
  y_pos=center_y;
  Ellipse_cursor_Y=-1;
  for (x_pos=center_x+1-horizontal_radius,Ellipse_cursor_X=-horizontal_radius+1;x_pos<center_x+horizontal_radius;x_pos++,Ellipse_cursor_X++)
    if (!Pixel_in_ellipse())
      Pixel_figure(x_pos,y_pos,color);

  Pixel_figure(center_x,center_y-vertical_radius,color);   // Haut
  Pixel_figure(center_x-horizontal_radius,center_y,color); // Gauche
  Pixel_figure(center_x+horizontal_radius,center_y,color); // Droite
  Pixel_figure(center_x,center_y+vertical_radius,color);   // Bas

  Update_part_of_screen(center_x-horizontal_radius,center_y-vertical_radius,2*horizontal_radius+1,2*vertical_radius+1);
}

  // -- Tracé définitif d'une ellipse vide --

void Draw_empty_ellipse_permanent(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,color);
  Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius+1);
}

  // -- Tracer la preview d'une ellipse vide --

void Draw_empty_ellipse_preview(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,color);
  Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius +1);
}

  // -- Effacer la preview d'une ellipse vide --

void Hide_empty_ellipse_preview(short center_x,short center_y,short horizontal_radius,short vertical_radius)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,0);
  Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius+1);
}

  // -- Tracer une ellipse pleine --

void Draw_filled_ellipse(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  short end_x;
  short end_y;

  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;
  end_x=center_x+horizontal_radius;
  end_y=center_y+vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1,vertical_radius+1);

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage de l'ellipse
  for (y_pos=start_y,Ellipse_cursor_Y=start_y-center_y;y_pos<=end_y;y_pos++,Ellipse_cursor_Y++)
    for (x_pos=start_x,Ellipse_cursor_X=start_x-center_x;x_pos<=end_x;x_pos++,Ellipse_cursor_X++)
      if (Pixel_in_ellipse())
        Display_pixel(x_pos,y_pos,color);
  Update_part_of_screen(center_x-horizontal_radius,center_y-vertical_radius,2*horizontal_radius+1,2*vertical_radius+1);
}

/******************
* TRACÉ DE LIGNES *
******************/

/// Alters bx and by so the (AX,AY)-(BX,BY) segment becomes either horizontal,
/// vertical, 45degrees, or isometrical for pixelart (ie 2:1 ratio)
void Clamp_coordinates_45_degrees(short ax, short ay, short* bx, short* by)
{
    int dx, dy;
    float tan;

    dx = (*bx)-ax;
    dy = ay- *by; 
		// On prend l'opposée car à l'écran les Y sont positifs en bas, et en 
		// maths, positifs en haut

    if (dx==0) return; 
		// On est en lockx et de toutes façons le X n'a pas bougé, on sort tout
		// de suite pour éviter une méchante division par 0

    tan = (float)dy/(float)dx;

	// These equation look a bit more complex than they should, but that's
	// because we have to balance the line length to make it end near the
	// cursor
    if (tan <= 0.25 && tan >= -0.25)
    {
      // horizontal (OK)
      *by = ay;
    }
    else if ( tan > 0.25 && tan < 0.75)
    {
      // dx=2dy, iso (ok)
	  *bx = (2* *bx + ax + 2*dy)/3;
	  *by = ay - (*bx-ax)/2;
    }
	else if ( tan >=0.75 && tan <= 1.5 )
	{
      // dy=-dx, diagonal upright (ok)
      *by = (*by + ay - dx)/2;
      *bx = ax  + ay - *by;
	}
	else if ( tan > 1.5 && tan <= 3 )
	{
	  // "vertical iso"
	}
    else if (tan < -0.25 && tan >= -0.75)
    {
		// iso to bottom
    }
	else if ( tan <-0.75 && tan > -1.5 )
	{
      // dy=dx, downright diagonal (ok)
      *by = (*by + ay + dx)/2;
      *bx = ax  - ay + *by;
	}
	else if ( tan < -1.5 && tan >= -3 )
	{
		// vertical iso to bottom
	}
    else
    {
      // vertical (ok)
      *bx = ax;
    }

    return;
}

  // -- Tracer général d'une ligne ------------------------------------------

void Draw_line_general(short start_x,short start_y,short end_x,short end_y, byte color)
{
  short x_pos,y_pos;
  short incr_x,incr_y;
  short i,cumul;
  short delta_x,delta_y;


  x_pos=start_x;
  y_pos=start_y;

  if (start_x<end_x)
  {
    incr_x=+1;
    delta_x=end_x-start_x;
  }
  else
  {
    incr_x=-1;
    delta_x=start_x-end_x;
  }

  if (start_y<end_y)
  {
    incr_y=+1;
    delta_y=end_y-start_y;
  }
  else
  {
    incr_y=-1;
    delta_y=start_y-end_y;
  }

  if (delta_y>delta_x)
  {
    cumul=delta_y>>1;
    for (i=1; i<delta_y; i++)
    {
      y_pos+=incr_y;
      cumul+=delta_x;
      if (cumul>=delta_y)
      {
        cumul-=delta_y;
        x_pos+=incr_x;
      }
      Pixel_figure(x_pos,y_pos,color);
    }
  }
  else
  {
    cumul=delta_x>>1;
    for (i=1; i<delta_x; i++)
    {
      x_pos+=incr_x;
      cumul+=delta_y;
      if (cumul>=delta_x)
      {
        cumul-=delta_x;
        y_pos+=incr_y;
      }
      Pixel_figure(x_pos,y_pos,color);
    }
  }

  if ( (start_x!=end_x) || (start_y!=end_y) )
    Pixel_figure(end_x,end_y,color);

}

  // -- Tracer définitif d'une ligne --

void Draw_line_permanet(short start_x,short start_y,short end_x,short end_y, byte color)
{

  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_permanent;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne --

void Draw_line_preview(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_preview;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor --

void Draw_line_preview_xor(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w, h;
  Pixel_figure=Pixel_figure_preview_xor;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  if (start_x<0)
    start_x=0;
  if (start_y<0)
    start_y=0;
  if (end_x<0)
    end_x=0;
  if (end_y<0)
    end_y=0;
  w = end_x-start_x;
  h = end_y-start_y;
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor additif --

void Draw_line_preview_xorback(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_preview_xorback;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Effacer la preview d'une ligne --

void Hide_line_preview(short start_x,short start_y,short end_x,short end_y)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_line_general(start_x,start_y,end_x,end_y,0);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}


  // -- Tracer un rectangle vide --

void Draw_empty_rectangle(short start_x,short start_y,short end_x,short end_y,byte color)
{
  short temp;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes soient dans le bon sens:
  if (start_x>end_x)
  {
    temp=start_x;
    start_x=end_x;
    end_x=temp;
  }
  if (start_y>end_y)
  {
    temp=start_y;
    start_y=end_y;
    end_y=temp;
  }

  // On trace le rectangle:

  for (x_pos=start_x;x_pos<=end_x;x_pos++)
    Display_paintbrush(x_pos,start_y,color,0);

  for (y_pos=start_y+1;y_pos<end_y;y_pos++)
  {
    Display_paintbrush(start_x,y_pos,color,0);
    Display_paintbrush(  end_x,y_pos,color,0);
  }

  for (x_pos=start_x;x_pos<=end_x;x_pos++)
    Display_paintbrush(x_pos,  end_y,color,0);
#if defined(__macosx__) || defined(__FreeBSD__)
  Update_part_of_screen(start_x,end_x,end_x-start_x,end_y-start_y);
#endif
}

  // -- Tracer un rectangle plein --

void Draw_filled_rectangle(short start_x,short start_y,short end_x,short end_y,byte color)
{
  short temp;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes sont dans le bon sens:
  if (start_x>end_x)
  {
    temp=start_x;
    start_x=end_x;
    end_x=temp;
  }
  if (start_y>end_y)
  {
    temp=start_y;
    start_y=end_y;
    end_y=temp;
  }

  // Correction en cas de dépassement des limites de l'image
  if (end_x>Limit_right)
    end_x=Limit_right;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;

  // On trace le rectangle:
  for (y_pos=start_y;y_pos<=end_y;y_pos++)
    for (x_pos=start_x;x_pos<=end_x;x_pos++)
      // Display_pixel traite chaque pixel avec tous les effets ! (smear, ...)
      // Donc on ne peut pas otimiser en traçant ligne par ligne avec memset :(
      Display_pixel(x_pos,y_pos,color);
  Update_part_of_screen(start_x,start_y,end_x-start_x,end_y-start_y);

}




  // -- Tracer une courbe de Bézier --

void Draw_curve_general(short x1, short y1,
                           short x2, short y2,
                           short x3, short y3,
                           short x4, short y4,
                           byte color)
{
  float delta,t,t2,t3;
  short x,y,old_x,old_y;
  word  i;
  int   cx[4];
  int   cy[4];

  // Calcul des vecteurs de coefficients
  cx[0]= -   x1 + 3*x2 - 3*x3 + x4;
  cx[1]= + 3*x1 - 6*x2 + 3*x3;
  cx[2]= - 3*x1 + 3*x2;
  cx[3]= +   x1;
  cy[0]= -   y1 + 3*y2 - 3*y3 + y4;
  cy[1]= + 3*y1 - 6*y2 + 3*y3;
  cy[2]= - 3*y1 + 3*y2;
  cy[3]= +   y1;

  // Traçage de la courbe
  old_x=x1;
  old_y=y1;
  Pixel_figure(old_x,old_y,color);
  delta=0.05; // 1.0/20
  t=0;
  for (i=1; i<=20; i++)
  {
    t=t+delta; t2=t*t; t3=t2*t;
    x=Round(t3*cx[0] + t2*cx[1] + t*cx[2] + cx[3]);
    y=Round(t3*cy[0] + t2*cy[1] + t*cy[2] + cy[3]);
    Draw_line_general(old_x,old_y,x,y,color);
    old_x=x;
    old_y=y;
  }

  x = Min(Min(x1,x2),Min(x3,x4));
  y = Min(Min(y1,y2),Min(y3,y4));
  old_x = Max(Max(x1,x2),Max(x3,x4)) - x;
  old_y = Max(Max(y1,y2),Max(y3,y4)) - y;
  Update_part_of_screen(x,y,old_x+1,old_y+1);
}

  // -- Tracer une courbe de Bézier définitivement --

void Draw_curve_permanent(short x1, short y1,
                             short x2, short y2,
                             short x3, short y3,
                             short x4, short y4,
                             byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}

  // -- Tracer la preview d'une courbe de Bézier --

void Draw_curve_preview(short x1, short y1,
                           short x2, short y2,
                           short x3, short y3,
                           short x4, short y4,
                           byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}

  // -- Effacer la preview d'une courbe de Bézier --

void Hide_curve_preview(short x1, short y1,
                            short x2, short y2,
                            short x3, short y3,
                            short x4, short y4,
                            byte color)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}




  // -- Spray : un petit coup de Pschiitt! --

void Airbrush(short clicked_button)
{
  short x_pos,y_pos;
  short radius=Airbrush_size>>1;
  long  radius_squared=(long)radius*radius;
  short index,count;
  byte  color_index;
  byte  direction;


  Hide_cursor();

  if (Airbrush_mode)
  {
    for (count=1; count<=Airbrush_mono_flow; count++)
    {
      x_pos=(rand()%Airbrush_size)-radius;
      y_pos=(rand()%Airbrush_size)-radius;
      if ( (x_pos*x_pos)+(y_pos*y_pos) <= radius_squared )
      {
        x_pos+=Paintbrush_X;
        y_pos+=Paintbrush_Y;
        if (clicked_button==1)
          Display_paintbrush(x_pos,y_pos,Fore_color,0);
        else
          Display_paintbrush(x_pos,y_pos,Back_color,0);
      }
    }
  }
  else
  {
    //   On essaye de se balader dans la table des flux de façon à ce que ce
    // ne soit pas toujours la dernière couleur qui soit affichée en dernier
    // Pour ça, on part d'une couleur au pif dans une direction aléatoire.
    direction=rand()&1;
    for (index=0,color_index=rand()/*%256*/; index<256; index++)
    {
      for (count=1; count<=Airbrush_multi_flow[color_index]; count++)
      {
        x_pos=(rand()%Airbrush_size)-radius;
        y_pos=(rand()%Airbrush_size)-radius;
        if ( (x_pos*x_pos)+(y_pos*y_pos) <= radius_squared )
        {
          x_pos+=Paintbrush_X;
          y_pos+=Paintbrush_Y;
          if (clicked_button==LEFT_SIDE)
            Display_paintbrush(x_pos,y_pos,color_index,0);
          else
            Display_paintbrush(x_pos,y_pos,Back_color,0);
        }
      }
      if (direction)
        color_index++;
      else
        color_index--;
    }
  }

  Display_cursor();
}



  //////////////////////////////////////////////////////////////////////////
  ////////////////////////// GESTION DES DEGRADES //////////////////////////
  //////////////////////////////////////////////////////////////////////////


  // -- Gestion d'un dégradé de base (le plus moche) --

void Gradient_basic(long index,short x_pos,short y_pos)
{
  long position;

  // On fait un premier calcul partiel
  position=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard
  position+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position-=(Gradient_total_range*Gradient_random_factor) >>7;

  position/=Gradient_total_range;

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position<0)
    position=0;
  else if (position>=Gradient_bounds_range)
    position=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    Gradient_pixel(x_pos,y_pos,Gradient_upper_bound-position);
  else
    Gradient_pixel(x_pos,y_pos,Gradient_lower_bound+position);
}


  // -- Gestion d'un dégradé par trames simples --

void Gradient_dithered(long index,short x_pos,short y_pos)
{
  long position_in_gradient;
  long position_in_segment;

  //
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "position_in_gradient", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  position_in_gradient=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard...
  position_in_gradient+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position_in_gradient-=(Gradient_total_range*Gradient_random_factor) >>7;

  if (position_in_gradient<0)
    position_in_gradient=0;

  // ... qui nous permet de calculer la position dans le segment
  position_in_segment=((position_in_gradient<<2)/Gradient_total_range)&3;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  position_in_gradient/=Gradient_total_range;

  // On va pouvoir discuter de la valeur de position_in_gradient en fonction
  // de la position dans l'écran et de la position_in_segment.

  switch (position_in_segment)
  {
    case 0 : // On est sur la gauche du segment
      if (((x_pos+y_pos)&1)==0)
        position_in_gradient--;
      break;

      // On n'a pas à traiter les cas 1 et 2 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 3 : // On est sur la droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        position_in_gradient++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position_in_gradient<0)
    position_in_gradient=0;
  else if (position_in_gradient>=Gradient_bounds_range)
    position_in_gradient=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    position_in_gradient=Gradient_upper_bound-position_in_gradient;
  else
    position_in_gradient=Gradient_lower_bound+position_in_gradient;

  Gradient_pixel(x_pos,y_pos,position_in_gradient);
}


  // -- Gestion d'un dégradé par trames étendues --

void Gradient_extra_dithered(long index,short x_pos,short y_pos)
{
  long position_in_gradient;
  long position_in_segment;

//
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "position_in_gradient", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  position_in_gradient=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard
  position_in_gradient+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position_in_gradient-=(Gradient_total_range*Gradient_random_factor) >>7;

  if (position_in_gradient<0)
    position_in_gradient=0;

  // Qui nous permet de calculer la position dans le segment
  position_in_segment=((position_in_gradient<<3)/Gradient_total_range)&7;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  position_in_gradient/=Gradient_total_range;

  // On va pouvoir discuter de la valeur de position_in_gradient en fonction
  // de la position dans l'écran et de la position_in_segment.

  switch (position_in_segment)
  {
    case 0 : // On est sur l'extrême gauche du segment
      if (((x_pos+y_pos)&1)==0)
        position_in_gradient--;
      break;

    case 1 : // On est sur la gauche du segment
    case 2 : // On est sur la gauche du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)==0))
        position_in_gradient--;
      break;

      // On n'a pas à traiter les cas 3 et 4 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 5 : // On est sur la droite du segment
    case 6 : // On est sur la droite du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)!=0))
        position_in_gradient++;
      break;

    case 7 : // On est sur l'extreme droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        position_in_gradient++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position_in_gradient<0)
    position_in_gradient=0;
  else if (position_in_gradient>=Gradient_bounds_range)
    position_in_gradient=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    position_in_gradient=Gradient_upper_bound-position_in_gradient;
  else
    position_in_gradient=Gradient_lower_bound+position_in_gradient;

  Gradient_pixel(x_pos,y_pos,position_in_gradient);
}



  // -- Tracer un cercle degradé (une sphère) --

void Draw_grad_circle(short center_x,short center_y,short radius,short spot_x,short spot_y)
{
  long start_x;
  long start_y;
  long x_pos;
  long y_pos;
  long end_x;
  long end_y;
  long distance_x; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long distance_y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage

  start_x=center_x-radius;
  start_y=center_y-radius;
  end_x=center_x+radius;
  end_y=center_y+radius;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  Gradient_total_range=Circle_limit+
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))+
                           (2L*radius*sqrt(
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))));

  if (Gradient_total_range==0)
    Gradient_total_range=1;

  // Affichage du cercle
  for (y_pos=start_y,Circle_cursor_Y=(long)start_y-center_y;y_pos<=end_y;y_pos++,Circle_cursor_Y++)
  {
    distance_y =(y_pos-spot_y);
    distance_y*=distance_y;
    for (x_pos=start_x,Circle_cursor_X=(long)start_x-center_x;x_pos<=end_x;x_pos++,Circle_cursor_X++)
      if (Pixel_in_circle())
      {
        distance_x =(x_pos-spot_x);
        distance_x*=distance_x;
        Gradient_function(distance_x+distance_y,x_pos,y_pos);
      }
  }

  Update_part_of_screen(center_x-radius,center_y-radius,2*radius+1,2*radius+1);
}


  // -- Tracer une ellipse degradée --

void Draw_grad_ellipse(short center_x,short center_y,short horizontal_radius,short vertical_radius,short spot_x,short spot_y)
{
  long start_x;
  long start_y;
  long x_pos;
  long y_pos;
  long end_x;
  long end_y;
  long distance_x; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long distance_y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage


  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;
  end_x=center_x+horizontal_radius;
  end_y=center_y+vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1,vertical_radius+1);

  // On calcule la distance maximale:
  Gradient_total_range=(horizontal_radius*horizontal_radius)+
                           (vertical_radius*vertical_radius)+
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))+
                           (2L
                           *sqrt(
                           (horizontal_radius*horizontal_radius)+
                           (vertical_radius  *vertical_radius  ))
                           *sqrt(
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))));

  if (Gradient_total_range==0)
    Gradient_total_range=1;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage de l'ellipse
  for (y_pos=start_y,Ellipse_cursor_Y=start_y-center_y;y_pos<=end_y;y_pos++,Ellipse_cursor_Y++)
  {
    distance_y =(y_pos-spot_y);
    distance_y*=distance_y;
    for (x_pos=start_x,Ellipse_cursor_X=start_x-center_x;x_pos<=end_x;x_pos++,Ellipse_cursor_X++)
      if (Pixel_in_ellipse())
      {
        distance_x =(x_pos-spot_x);
        distance_x*=distance_x;
        Gradient_function(distance_x+distance_y,x_pos,y_pos);
      }
  }

  Update_part_of_screen(start_x,start_y,end_x-start_x+1,end_y-start_y+1);
}


// Tracé d'un rectangle (rax ray - rbx rby) dégradé selon le vecteur (vax vay - vbx - vby)
void Draw_grad_rectangle(short rax,short ray,short rbx,short rby,short vax,short vay, short vbx, short vby)
{
    short y_pos, x_pos;

    // On commence par s'assurer que le rectangle est à l'endroit
    if(rbx < rax)
    {
      x_pos = rbx;
      rbx = rax;
      rax = x_pos;
    }

    if(rby < ray)
    {
      y_pos = rby;
      rby = ray;
      ray = y_pos;
    }

    // Correction des bornes d'après les limites
    if (ray<Limit_top)
      ray=Limit_top;
    if (rby>Limit_bottom)
      rby=Limit_bottom;
    if (rax<Limit_left)
      rax=Limit_left;
    if (rbx>Limit_right)
      rbx=Limit_right;

    if(vbx == vax)
    {
      // Le vecteur est vertical, donc on évite la partie en dessous qui foirerait avec une division par 0...
      if (vby == vay) return;  // L'utilisateur fait n'importe quoi
      Gradient_total_range = abs(vby - vay);
      for(y_pos=ray;y_pos<=rby;y_pos++)
        for(x_pos=rax;x_pos<=rbx;x_pos++)
          Gradient_function(abs(vby - y_pos),x_pos,y_pos);

    }
    else
    {
      float a;
      float b;
      float distance_x, distance_y;

      Gradient_total_range = sqrt(pow(vby - vay,2)+pow(vbx - vax,2));
      a = (float)(vby - vay)/(float)(vbx - vax);
      b = vay - a*vax;
      
      for (y_pos=ray;y_pos<=rby;y_pos++)
        for (x_pos = rax;x_pos<=rbx;x_pos++)
        {
          // On calcule ou on en est dans le dégradé
          distance_x = pow((y_pos - vay),2)+pow((x_pos - vax),2);
          distance_y = pow((-a * x_pos + y_pos - b),2)/(a*a+1);
      
          Gradient_function((int)sqrt(distance_x - distance_y),x_pos,y_pos);
        }
    }
    Update_part_of_screen(rax,ray,rbx,rby);
}




// -- Tracer un polygône plein --

typedef struct T_Polygon_edge      /* an active edge */
{
    short top;                     /* top y position */
    short bottom;                  /* bottom y position */
    float x, dx;                   /* floating point x position and gradient */
    float w;                       /* width of line segment */
    struct T_Polygon_edge *prev;     /* doubly linked list */
    struct T_Polygon_edge *next;
} T_Polygon_edge;



/* Fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
void Fill_edge_structure(T_Polygon_edge *edge, short *i1, short *i2)
{
  short *it;

  if (i2[1] < i1[1])
  {
    it = i1;
    i1 = i2;
    i2 = it;
  }

  edge->top = i1[1];
  edge->bottom = i2[1] - 1;
  edge->dx = ((float) i2[0] - (float) i1[0]) / ((float) i2[1] - (float) i1[1]);
  edge->x = i1[0] + 0.4999999;
  edge->prev = NULL;
  edge->next = NULL;

  if (edge->dx+1 < 0.0)
    edge->x += edge->dx+1;

  if (edge->dx >= 0.0)
    edge->w = edge->dx;
  else
    edge->w = -(edge->dx);

  if (edge->w-1.0<0.0)
    edge->w = 0.0;
  else
    edge->w = edge->w-1;
}



/* Add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
T_Polygon_edge * Add_edge(T_Polygon_edge *list, T_Polygon_edge *edge, int sort_by_x)
{
  T_Polygon_edge *pos = list;
  T_Polygon_edge *prev = NULL;

  if (sort_by_x)
  {
    while ( (pos) && ((pos->x+((pos->w+pos->dx)/2)) < (edge->x+((edge->w+edge->dx)/2))) )
    {
      prev = pos;
      pos = pos->next;
    }
  }
  else
  {
    while ((pos) && (pos->top < edge->top))
    {
      prev = pos;
      pos = pos->next;
    }
  }

  edge->next = pos;
  edge->prev = prev;

  if (pos)
    pos->prev = edge;

  if (prev)
  {
    prev->next = edge;
    return list;
  }
  else
    return edge;
}



/* Remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
T_Polygon_edge * Remove_edge(T_Polygon_edge *list, T_Polygon_edge *edge)
{
  if (edge->next)
    edge->next->prev = edge->prev;

  if (edge->prev)
  {
    edge->prev->next = edge->next;
    return list;
  }
  else
    return edge->next;
}



/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the
 *  number of vertices, then an array containing a series of x, y points
 *  (a total of vertices*2 values).
 */
void Polyfill_general(int vertices, short * points, int color)
{
  short c;
  short top = 0x7FFF;
  short bottom = 0;
  short *i1, *i2;
  short x_pos,end_x;
  T_Polygon_edge *edge, *next_edge, *initial_edge;
  T_Polygon_edge *active_edges = NULL;
  T_Polygon_edge *inactive_edges = NULL;

  /* allocate some space and fill the edge table */
  initial_edge=edge=(T_Polygon_edge *) malloc(sizeof(T_Polygon_edge) * vertices);

  i1 = points;
  i2 = points + ((vertices-1)<<1);

  for (c=0; c<vertices; c++)
  {
    if (i1[1] != i2[1])
    {
      Fill_edge_structure(edge, i1, i2);

      if (edge->bottom >= edge->top)
      {
        if (edge->top < top)
          top = edge->top;

        if (edge->bottom > bottom)
          bottom = edge->bottom;

        inactive_edges = Add_edge(inactive_edges, edge, 0);
        edge++;
      }
    }
    i2 = i1;
    i1 += 2;
  }

  /* for each scanline in the polygon... */
  for (c=top; c<=bottom; c++)
  {
    /* check for newly active edges */
    edge = inactive_edges;
    while ((edge) && (edge->top == c))
    {
      next_edge = edge->next;
      inactive_edges = Remove_edge(inactive_edges, edge);
      active_edges = Add_edge(active_edges, edge, 1);
      edge = next_edge;
    }

    /* draw horizontal line segments */
    if ((c>=Limit_top) && (c<=Limit_bottom))
    {
      edge = active_edges;
      while ((edge) && (edge->next))
      {
        x_pos=/*Round*/(edge->x);
        end_x=/*Round*/(edge->next->x+edge->next->w);
        if (x_pos<Limit_left)
          x_pos=Limit_left;
        if (end_x>Limit_right)
          end_x=Limit_right;
        for (; x_pos<=end_x; x_pos++)
          Pixel_figure(x_pos,c,color);
        edge = edge->next->next;
      }
    }

    /* update edges, sorting and removing dead ones */
    edge = active_edges;
    while (edge)
    {
      next_edge = edge->next;
      if (c >= edge->bottom)
        active_edges = Remove_edge(active_edges, edge);
      else
      {
        edge->x += edge->dx;
        while ((edge->prev) && ( (edge->x+(edge->w/2)) < (edge->prev->x+(edge->prev->w/2))) )
        {
          if (edge->next)
            edge->next->prev = edge->prev;
          edge->prev->next = edge->next;
          edge->next = edge->prev;
          edge->prev = edge->prev->prev;
          edge->next->prev = edge;
          if (edge->prev)
            edge->prev->next = edge;
          else
            active_edges = edge;
        }
      }
      edge = next_edge;
    }
  }

  free(initial_edge);

  // On ne connait pas simplement les xmin et xmax ici, mais de toutes façon ce n'est pas utilisé en preview
  Update_part_of_screen(0,top,Main_image_width,bottom-top+1);
}


void Polyfill(int vertices, short * points, int color)
{
  int index;
  byte *old_fx_feedback_screen;

  Pixel_clipped(points[0],points[1],color);
  if (vertices==1)
  {
    Update_part_of_screen(points[0],points[1],1,1);
    return;
  }

  // Comme pour le Fill, cette operation fait un peu d'"overdraw"
  // (pixels dessinés plus d'une fois) alors on force le FX Feedback à OFF
  old_fx_feedback_screen=FX_feedback_screen;
  FX_feedback_screen=Screen_backup;

  Pixel_figure=Pixel_clipped;    
  Polyfill_general(vertices,points,color);

  // Remarque: pour dessiner la bordure avec la brosse en cours au lieu
  // d'un pixel de couleur premier-plan, il suffit de mettre ici:
  // Pixel_figure=Pixel_figure_permanent;

  // Dessin du contour
  for (index=0; index<vertices-1;index+=1)
    Draw_line_general(points[index*2],points[index*2+1],points[index*2+2],points[index*2+3],color);
  Draw_line_general(points[0],points[1],points[index*2],points[index*2+1],color);

  // restore de l'etat du FX Feedback  
  FX_feedback_screen=old_fx_feedback_screen;

}



//------------ Remplacement de la couleur pointée par une autre --------------

void Replace(byte New_color)
{
  byte old_color;

  if ((Paintbrush_X<Main_image_width)
   && (Paintbrush_Y<Main_image_height))
  {
    old_color=Read_pixel_from_current_screen(Paintbrush_X,Paintbrush_Y);
    if ( (old_color!=New_color)
      && ((!Stencil_mode) || (!Stencil[old_color])) )
    {
      Replace_a_color(old_color,New_color);
      Display_all_screen();
    }
  }
}



/******************************************************************************/
/********************************** SHADES ************************************/

// Transformer une liste de shade en deux tables de conversion
void Shade_list_to_lookup_tables(word * list,short step,byte mode,byte * table_inc,byte * table_dec)
{
  int index;
  int first;
  int last;
  int color;
  int temp;


  // On initialise les deux tables de conversion en Identité
  for (index=0;index<256;index++)
  {
    table_inc[index]=index;
    table_dec[index]=index;
  }

  // On s'apprête à examiner l'ensemble de la liste
  for (index=0;index<512;index++)
  {
    // On recherche la première case de la liste non vide (et non inhibée)
    while ((index<512) && (list[index]>255))
      index++;

    // On note la position de la première case de la séquence
    first=index;

    // On recherche la position de la dernière case de la séquence
    for (last=first;list[last+1]<256;last++);

    // Pour toutes les cases non vides (et non inhibées) qui suivent
    switch (mode)
    {
      case SHADE_MODE_NORMAL :
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          table_inc[color]=list[(index+step<=last)?index+step:last];
          table_dec[color]=list[(index-step>=first)?index-step:first];
        }
        break;
      case SHADE_MODE_LOOP :
        temp=1+last-first;
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          table_inc[color]=list[first+((step+index-first)%temp)];
          table_dec[color]=list[first+(((temp-step)+index-first)%temp)];
        }
        break;
      default : // SHADE_MODE_NOSAT
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          if (index+step<=last)
            table_inc[color]=list[index+step];
          if (index-step>=first)
            table_dec[color]=list[index-step];
        }
    }
  }
}



// -- Interface avec l'image, affectée par le facteur de grossissement -------

  // fonction d'affichage "Pixel" utilisée pour les opérations définitivement
  // Ne doit à aucune condition être appelée en dehors de la partie visible
  // de l'image dans l'écran (ça pourrait être grave)
void Display_pixel(word x,word y,byte color)
  // x & y    sont la position d'un point dans l'IMAGE
  // color  est la couleur du point
  // Le Stencil est géré.
  // Les effets sont gérés par appel à Effect_function().
  // La Loupe est gérée par appel à Pixel_preview().
{
  if ( ( (!Sieve_mode)   || (Effect_sieve(x,y)) )
    && (!((Stencil_mode) && (Stencil[Read_pixel_from_current_screen(x,y)])))
    && (!((Mask_mode)    && (Mask_table[Read_pixel_from_spare_screen(x,y)]))) )
  {
    color=Effect_function(x,y,color);
    Pixel_in_current_screen(x,y,color);
    Pixel_preview(x,y,color);
  }
}



// -- Calcul des différents effets -------------------------------------------

  // -- Aucun effet en cours --

byte No_effect(__attribute__((unused)) word x,__attribute__((unused)) word y,byte color)
{
  return color;
}

  // -- Effet de Shading --

byte Effect_shade(word x,word y,__attribute__((unused)) byte color)
{
  return Shade_table[Read_pixel_from_feedback_screen(x,y)];
}

byte Effect_quick_shade(word x,word y,byte color)
{
  int c=color=Read_pixel_from_feedback_screen(x,y);
  int direction=(Fore_color<=Back_color);
  byte start,end;
  int width;

  if (direction)
  {
    start=Fore_color;
    end  =Back_color;
  }
  else
  {
    start=Back_color;
    end  =Fore_color;
  }

  if ((c>=start) && (c<=end) && (start!=end))
  {
    width=1+end-start;

    if ( ((Shade_table==Shade_table_left) && direction) || ((Shade_table==Shade_table_right) && (!direction)) )
      c-=Quick_shade_step%width;
    else
      c+=Quick_shade_step%width;

    if (c<start)
      switch (Quick_shade_loop)
      {
        case SHADE_MODE_NORMAL : return start;
        case SHADE_MODE_LOOP : return (width+c);
        default : return color;
      }

    if (c>end)
      switch (Quick_shade_loop)
      {
        case SHADE_MODE_NORMAL : return end;
        case SHADE_MODE_LOOP : return (c-width);
        default : return color;
      }
  }

  return c;
}

  // -- Effet de Tiling --

byte Effect_tiling(word x,word y,__attribute__((unused)) byte color)
{
  return Read_pixel_from_brush((x+Brush_width-Tiling_offset_X)%Brush_width,
                               (y+Brush_height-Tiling_offset_Y)%Brush_height);
}

  // -- Effet de Smooth --

byte Effect_smooth(word x,word y,__attribute__((unused)) byte color)
{
  int r,g,b;
  byte c;
  int weight,total_weight;
  byte x2=((x+1)<Main_image_width);
  byte y2=((y+1)<Main_image_height);

  // On commence par le pixel central
  c=Read_pixel_from_feedback_screen(x,y);
  total_weight=Smooth_matrix[1][1];
  r=total_weight*Main_palette[c].R;
  g=total_weight*Main_palette[c].G;
  b=total_weight*Main_palette[c].B;

  if (x)
  {
    c=Read_pixel_from_feedback_screen(x-1,y);
    total_weight+=(weight=Smooth_matrix[0][1]);
    r+=weight*Main_palette[c].R;
    g+=weight*Main_palette[c].G;
    b+=weight*Main_palette[c].B;

    if (y)
    {
      c=Read_pixel_from_feedback_screen(x-1,y-1);
      total_weight+=(weight=Smooth_matrix[0][0]);
      r+=weight*Main_palette[c].R;
      g+=weight*Main_palette[c].G;
      b+=weight*Main_palette[c].B;

      if (y2)
      {
        c=Read_pixel_from_feedback_screen(x-1,y+1);
        total_weight+=(weight=Smooth_matrix[0][2]);
        r+=weight*Main_palette[c].R;
        g+=weight*Main_palette[c].G;
        b+=weight*Main_palette[c].B;
      }
    }
  }

  if (x2)
  {
    c=Read_pixel_from_feedback_screen(x+1,y);
    total_weight+=(weight=Smooth_matrix[2][1]);
    r+=weight*Main_palette[c].R;
    g+=weight*Main_palette[c].G;
    b+=weight*Main_palette[c].B;

    if (y)
    {
      c=Read_pixel_from_feedback_screen(x+1,y-1);
      total_weight+=(weight=Smooth_matrix[2][0]);
      r+=weight*Main_palette[c].R;
      g+=weight*Main_palette[c].G;
      b+=weight*Main_palette[c].B;

      if (y2)
      {
        c=Read_pixel_from_feedback_screen(x+1,y+1);
        total_weight+=(weight=Smooth_matrix[2][2]);
        r+=weight*Main_palette[c].R;
        g+=weight*Main_palette[c].G;
        b+=weight*Main_palette[c].B;
      }
    }
  }

  if (y)
  {
    c=Read_pixel_from_feedback_screen(x,y-1);
    total_weight+=(weight=Smooth_matrix[1][0]);
    r+=weight*Main_palette[c].R;
    g+=weight*Main_palette[c].G;
    b+=weight*Main_palette[c].B;
  }

  if (y2)
  {
    c=Read_pixel_from_feedback_screen(x,y+1);
    total_weight+=(weight=Smooth_matrix[1][2]);
    r+=weight*Main_palette[c].R;
    g+=weight*Main_palette[c].G;
    b+=weight*Main_palette[c].B;
  }

  return (total_weight)? // On regarde s'il faut éviter le 0/0.
    Best_color(Round_div(r,total_weight),
                      Round_div(g,total_weight),
                      Round_div(b,total_weight)):
    Read_pixel_from_current_screen(x,y); // C'est bien l'écran courant et pas
                                       // l'écran feedback car il s'agit de ne
}                                      // pas modifier l'écran courant.
