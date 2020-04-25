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
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "graph.h"
#include "engine.h"
#include "errors.h"
#include "misc.h"
#include "readline.h"
#include "help.h"
#include "screen.h"
#include "windows.h"
#include "input.h"
#include "shade.h"
#include "keycodes.h"

void Button_Shade_mode(void)
{
  if (Shade_mode)
    Effect_function=No_effect;
  else
  {
    Effect_function=Effect_shade;
    Quick_shade_mode=0;
    Colorize_mode=0;
    Smooth_mode=0;
    Tiling_mode=0;
    Smear_mode=0;
  }
  Shade_mode=!Shade_mode;
}


void Button_Quick_shade_mode(void)
{
  if (Quick_shade_mode)
    Effect_function=No_effect;
  else
  {
    Effect_function=Effect_quick_shade;
    Shade_mode=0;
    Colorize_mode=0;
    Smooth_mode=0;
    Tiling_mode=0;
    Smear_mode=0;
  }
  Quick_shade_mode=!Quick_shade_mode;
}


void Shade_draw_grad_ranges(void)
{
  word  cursor=0;
  word  nb_shades=0;
  short shade_processed,shade_processed_old;
  word  shade_size=0;
  word  start_shade=0;
  short x_pos,y_pos;
  short x_size,y_size;
  short start_x,start_y,end_x,end_y;

  // On commence par compter le nombre de shades
  while (cursor<512)
  {
    while ((cursor<512) && (Shade_list[Shade_current].List[cursor]&0xFF00))
      cursor++;

    if (cursor<512)
    {
      nb_shades++;
      while ( (cursor<512)
         && (!(Shade_list[Shade_current].List[cursor]&0xFF00)) )
        cursor++;
    }
  }

  // Maintenant qu'on sait combien il y en a, on les affiche:
  if (nb_shades)
  {
    x_size=Menu_factor_X<<6;
    y_size=Menu_factor_Y*48;
    start_x=Window_pos_X+(Menu_factor_X*224);
    start_y=Window_pos_Y+(Menu_factor_Y*35);
    end_x=start_x+x_size;
    end_y=start_y+y_size;

    cursor=0;
    shade_processed_old=-1;

    for (y_pos=start_y;y_pos<end_y;y_pos++)
    {
      // On regarde quel shade on va afficher en preview
      shade_processed=((y_pos-start_y)*nb_shades)/y_size;
      // Si ce n'est pas le shade précédemment traité on calcule ses infos
      if (shade_processed>shade_processed_old)
      {
        // On commence par sauter tous les vides jusqu'au prochain shade
        while ((cursor<512) && (Shade_list[Shade_current].List[cursor]&0xFF00))
          cursor++;
        start_shade=cursor;
        // puis regarde sa taille
        while ((cursor<512) && (!(Shade_list[Shade_current].List[cursor]&0xFF00)))
          cursor++;
        shade_size=cursor-start_shade;
        shade_processed_old=shade_processed;
      }

      for (x_pos=start_x;x_pos<end_x;x_pos++)
      {
        Pixel(x_pos,y_pos,Shade_list[Shade_current].List
              [(((x_pos-start_x)*shade_size)/x_size)+start_shade]);
      }
    }
  }
  else
  {
    Window_display_frame_out(224,35,64,48);
    Window_rectangle(225,36,62,46,MC_Light);
  }
  Update_window_area(224,35,64,48);
}


void Tag_shades(word selection_start,word selection_end)
{
  word line, column;
  word position;
  word x_pos, y_pos;


  if (selection_end<selection_start)
  {
    position    =selection_end;
    selection_end  =selection_start;
    selection_start=position;
  }

  for (line=0; line<8; line++)
    for (column=0; column<64; column++)
    {
      position=(line<<6)+column;
      x_pos=Window_pos_X+(Menu_factor_X*((column<<2)+8));
      y_pos=Window_pos_Y+(Menu_factor_Y*((line*7)+131));

      // On regarde si la case est "disablée"
      if (Shade_list[Shade_current].List[position]&0x8000)
      {
        if ((position>=selection_start) && (position<=selection_end))
        {
          Block(x_pos,y_pos,Menu_factor_X<<2,Menu_factor_Y,MC_White);
          Block(x_pos,y_pos+Menu_factor_Y,Menu_factor_X<<2,Menu_factor_Y,MC_Black);
        }
        else
          Block(x_pos,y_pos,Menu_factor_X<<2,Menu_factor_Y<<1,MC_White);
      }
      else // "enablée"
      {
        if ((position>=selection_start) && (position<=selection_end))
          Block(x_pos,y_pos,Menu_factor_X<<2,Menu_factor_Y<<1,MC_Black);
        else
          Block(x_pos,y_pos,Menu_factor_X<<2,Menu_factor_Y<<1,MC_Light);
      }
    }
    Update_window_area(8,131,64<<2,8<<3);
}


void Display_selected_cell_color(word selection_start,word selection_end)
{
  char str[4];

  if ((selection_start!=selection_end)
   || (Shade_list[Shade_current].List[selection_start]&0x0100))
    strcpy(str,"   ");
  else
    Num2str(Shade_list[Shade_current].List[selection_start]&0xFF,str,3);

  Print_in_window(213,115,str,MC_Black,MC_Light);
}


void Display_selected_color(word selection_start,word selection_end)
{
  char str[4];

  if (selection_start!=selection_end)
    strcpy(str,"   ");
  else
    Num2str(selection_start,str,3);

  Print_in_window(213,106,str,MC_Black,MC_Light);
}


void Display_shade_mode(short x,short y,byte mode)
{
  char str[7];

  switch (mode)
  {
    case SHADE_MODE_NORMAL :
      strcpy(str,"Normal");
      break;
    case SHADE_MODE_LOOP :
      strcpy(str," Loop ");
      break;
    default : // SHADE_MODE_NOSAT
      strcpy(str,"No sat");
  }
  Print_in_window(x,y,str,MC_Black,MC_Light);
}


void Display_all_shade(word selection_start1,word selection_end1,
                            word selection_start2,word selection_end2)
{
  word line, column;
  word position;

  for (line=0; line<8; line++)
    for (column=0; column<64; column++)
    {
      position=(line<<6)+column;
      // On regarde si c'est une couleur ou un bloc vide
      if (Shade_list[Shade_current].List[position]&0x0100) // Vide
      {
        Window_display_frame_out((column<<2)+8,(line*7)+127,4,4);
        Window_rectangle((column<<2)+9,
              (line*7)+128,
              2,2,MC_Light);
      }
      else // color
        Window_rectangle((column<<2)+8,
              (line*7)+127,
              4,4,
              Shade_list[Shade_current].List[position]&0xFF);
    }
  Update_window_area(7,126,(64<<2)+2,(8<<2)+2);
  Tag_shades(selection_start2,selection_end2);
  Shade_draw_grad_ranges();
  Display_selected_cell_color(selection_start2,selection_end2);
  Display_selected_color(selection_start1,selection_end1);
  Display_shade_mode(250,110,Shade_list[Shade_current].Mode);
}


void Remove_shade(word selection_start,word selection_end)
{
  word temp;

  if (selection_end<selection_start)
  {
    temp        =selection_end;
    selection_end  =selection_start;
    selection_start=temp;
  }

  for (selection_end++;selection_end<512;selection_start++,selection_end++)
    Shade_list[Shade_current].List[selection_start]=Shade_list[Shade_current].List[selection_end];

  for (;selection_start<512;selection_start++)
    Shade_list[Shade_current].List[selection_start]=0x0100;
}


void Insert_shade(byte first_color, byte last_color, word selection_start)
{
  word cursor,limit;
  word temp;

  if (last_color<first_color)
  {
    temp            =last_color;
    last_color=first_color;
    first_color=temp;
  }

  // Avant d'insérer quoi que ce soit, on efface les éventuelles couleurs que
  // l'on va réinsérer:
  limit=512-selection_start;
  for (cursor=0; cursor<512; cursor++)
  {
    if (!(Shade_list[Shade_current].List[cursor]&0x0100))
      for (temp=first_color; temp<=last_color; temp++)
        if ( (temp-first_color<limit)
          && ((Shade_list[Shade_current].List[cursor]&0xFF)==temp) )
          Shade_list[Shade_current].List[cursor]=(Shade_list[Shade_current].List[cursor]&0x8000)|0x0100;
  }
  // Voilà... Maintenant on peut y aller peinard.

  temp=1+last_color-first_color;
  limit=selection_start+temp;
  if (limit>=512)
    temp=512-selection_start;

  for (cursor=511;cursor>=limit;cursor--)
    Shade_list[Shade_current].List[cursor]=Shade_list[Shade_current].List[cursor-temp];

  for (cursor=selection_start+temp;selection_start<cursor;selection_start++,first_color++)
    Shade_list[Shade_current].List[selection_start]=first_color;
}


void Insert_empty_cell_in_shade(word position)
{
  word cursor;

  if (position>=512) return;

  for (cursor=511;cursor>position;cursor--)
    Shade_list[Shade_current].List[cursor]=Shade_list[Shade_current].List[cursor-1];

  Shade_list[Shade_current].List[position]=0x0100;
}


short Wait_click_in_shade_table()
{
  short selected_cell=-1;
  byte  old_hide_cursor;


  Hide_cursor();
  old_hide_cursor=Cursor_hidden;
  Cursor_hidden=0;
  Cursor_shape=CURSOR_SHAPE_TARGET;
  Display_cursor();

  while (selected_cell<0)
  {
    Get_input(20);

    if ( (Mouse_K==LEFT_SIDE)
      && ( ( (Window_click_in_rectangle(8,127,263,179)) && (((((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-127)%7)<4) )
           || ( (Mouse_X<Window_pos_X) || (Mouse_Y<Window_pos_Y)
             || (Mouse_X>=Window_pos_X+(Window_width*Menu_factor_X))
             || (Mouse_Y>=Window_pos_Y+(Window_height*Menu_factor_Y)) ) )
       )
      selected_cell=(((((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-127)/7)<<6)+
                    ((((Mouse_X-Window_pos_X)/Menu_factor_X)-8 )>>2);

    if ((Mouse_K==RIGHT_SIDE) || (Key==KEY_ESC))
      selected_cell=512; // valeur indiquant que l'on n'a rien choisi
  }

  Hide_cursor();
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Cursor_hidden=old_hide_cursor;
  Display_cursor();
  return selected_cell;
}


void Swap_shade(short block_1_start,short block_2_start,short block_size)
{
  short  pos_1;
  short  pos_2;
  short  end_1;
  short  end_2;
  word   temp;
  word * temp_shade;

  // On fait une copie de la liste
  temp_shade=(word *)malloc(512*sizeof(word));
  memcpy(temp_shade,Shade_list[Shade_current].List,512*sizeof(word));

  // On calcul les dernières couleurs de chaque bloc.
  end_1=block_1_start+block_size-1;
  end_2=block_2_start+block_size-1;

  if ((block_2_start>=block_1_start) && (block_2_start<=end_1))
  {
    // Le bloc destination commence dans le bloc source.
    for (pos_1=block_1_start,pos_2=end_1+1;pos_1<=end_2;pos_1++)
    {
      // Il faut transformer la case pos_1 en pos_2:
      Shade_list[Shade_current].List[pos_1]=temp_shade[pos_2];
      // On gère la mise à jour de pos_2
      if (pos_2==end_2)
        pos_2=block_1_start;
      else
        pos_2++;
    }
  }
  else
  if ((block_2_start<block_1_start) && (end_2>=block_1_start))
  {
    // Le bloc destination déborde dans le bloc source.
    for (pos_1=block_2_start,pos_2=block_1_start;pos_1<=end_1;pos_1++)
    {
      // Il faut transformer la couleur pos_1 en pos_2:
      Shade_list[Shade_current].List[pos_1]=temp_shade[pos_2];
      // On gère la mise à jour de pos_2
      if (pos_2==end_1)
        pos_2=block_2_start;
      else
        pos_2++;
    }
  }
  else
  {
    // Le bloc source et le bloc destination sont distincts.
    for (pos_1=block_1_start,pos_2=block_2_start;pos_1<=end_1;pos_1++,pos_2++)
    {
      // On échange les cases
      temp                                  =Shade_list[Shade_current].List[pos_1];
      Shade_list[Shade_current].List[pos_1]=Shade_list[Shade_current].List[pos_2];
      Shade_list[Shade_current].List[pos_2]=temp;
    }
  }

  free(temp_shade);
}


int Menu_shade(void)
{
  short clicked_button; // Numéro du bouton sur lequel l'utilisateur a clické
  char str[4]; // str d'affichage du n° de shade actif et du Pas
  word old_mouse_x, old_mouse_x2; // Mémo. de l'ancienne pos. du curseur
  word old_mouse_y, old_mouse_y2;
  byte old_mouse_k, old_mouse_k2;
  byte temp_color; // Variables de gestion des clicks dans la palette
  byte first_color = Fore_color;
  byte last_color = Fore_color;
  word selection_start = 0;
  word selection_end = 0;
  T_Special_button * input_button;
  short temp, temp2;
  word temp_cell;
  word * buffer;       // buffer du Copy/Paste
  word * undo_buffer;  // buffer du Undo
  word * temp_ptr;
  byte color;
  byte click;


  buffer       =(word *)malloc(512*sizeof(word));
  undo_buffer  =(word *)malloc(512*sizeof(word));
  temp_ptr=(word *)malloc(512*sizeof(word));

  // Ouverture de la fenêtre du menu
  Open_window(310,190,"Shade");

  // Déclaration & tracé du bouton de palette
  Window_set_palette_button(5,16);                             // 1

  // Déclaration & tracé du scroller de sélection du n° de dégradé
  Window_set_scroller_button(192,17,84,8,1,Shade_current);      // 2

  // Déclaration & tracé de la zone de définition des dégradés
  Window_set_special_button(8,127,256,53,0);                     // 3

  // Déclaration & tracé des boutons de sortie
  Window_set_normal_button(207,17,51,14,"Cancel",0,1,KEY_ESC);   // 4
  Window_set_normal_button(261,17,43,14,"OK"    ,0,1,KEY_RETURN);  // 5

  // Déclaration & tracé des boutons de copie de shade
  Window_set_normal_button(206,87,27,14,"Cpy"   ,1,1,KEY_c);  // 6
  Window_set_normal_button(234,87,43,14,"Paste" ,1,1,KEY_p);  // 7

  // On tagge le bloc
  Tag_color_range(Fore_color,Fore_color);

  // Tracé d'un cadre creux autour du bloc dégradé
  Window_display_frame_in(171,26,18,66);
  Window_rectangle(172,27,16,64,MC_Black);
  // Tracé d'un cadre creux autour de tous les dégradés
  Window_display_frame_in(223,34,66,50);
  Shade_draw_grad_ranges();
  // Tracé d'un cadre autour de la zone de définition de dégradés
  Window_display_frame(5,124,262,61);
  Display_all_shade(first_color,last_color,selection_start,selection_end);

  // Déclaration & tracé des boutons d'édition de shade
  Window_set_normal_button(  6,107,27,14,"Ins"  ,0,1,KEY_INSERT);  // 8
  Window_set_normal_button( 38,107,27,14,"Del"  ,0,1,KEY_DELETE);  // 9
  Window_set_normal_button( 66,107,43,14,"Blank",1,1,KEY_b);  // 10
  Window_set_normal_button(110,107,27,14,"Inv"  ,1,1,KEY_i);  // 11
  Window_set_normal_button(138,107,27,14,"Swp"  ,1,1,KEY_s);  // 12

  // Déclaration & tracé des boutons de taggage
  Print_in_window(268,123,"Disbl"/*"Dsabl"*/,MC_Dark,MC_Light);
  Window_set_normal_button(274,133,27,14,"Set"   ,0,1,KEY_F1); // 13
  Window_set_normal_button(274,148,27,14,"Clr"   ,0,1,KEY_F2); // 14

  // Déclaration & tracé de la zone de saisie du pas
  Print_in_window(272,165,"Step",MC_Dark,MC_Light);
  input_button = Window_set_input_button(274,174,3);          // 15
  Num2str(Shade_list[Shade_current].Step,str,3);
  Window_input_content(input_button,str);

  // Button Undo
  Window_set_normal_button(170,107,35,14,"Undo",1,1,KEY_u);   // 16
  // Button Clear
  Window_set_normal_button(278,87,27,14,"Clr",0,1,KEY_BACKSPACE);     // 17

  // Button Mode
  Window_set_normal_button(244,107,60,14,"",0,1,KEY_TAB);       // 18

  // Affichage du n° de shade actif
  Num2str(Shade_current+1,str,1);
  Print_in_window(210,55,str,MC_Black,MC_Light);

  memcpy(buffer     ,Shade_list[Shade_current].List,512*sizeof(word));
  memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));

  Update_window_area(0,0,310,190);

  Display_cursor();

  do
  {
    old_mouse_x=old_mouse_x2=Mouse_X;
    old_mouse_y=old_mouse_y2=Mouse_Y;
    old_mouse_k=old_mouse_k2=Mouse_K;

    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case  0 :
        break;
      case -1 :
      case  1 : // Gestion de la palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          temp_color=(clicked_button==1) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);

          if (!old_mouse_k)
          { // On vient de clicker

            // On met à jour l'intervalle du Shade
            first_color=last_color=temp_color;
            // On tagge le bloc
            Tag_color_range(first_color,last_color);
            // Tracé du bloc dégradé:
            Display_grad_block_in_window(172,27,16,64,first_color,last_color);
          }
          else
          { // On maintient le click, on va donc tester si le curseur bouge
            if (temp_color!=last_color)
            {
              last_color=temp_color;

              // On tagge le bloc
              if (first_color<=temp_color)
              {
                Tag_color_range(first_color,last_color);
                Display_grad_block_in_window(172,27,16,64,first_color,last_color);
              }
              else
              {
                Tag_color_range(last_color,first_color);
                Display_grad_block_in_window(172,27,16,64,last_color,first_color);
              }
            }
          }

          // On affiche le numéro de la couleur sélectionnée
          Display_selected_color(first_color,last_color);

          Display_cursor();
        }
        break;

      case  2 : // Gestion du changement de Shade (scroller)
        Hide_cursor();
        Shade_current=Window_attribute2;
        // Affichade du n° de shade actif
        Num2str(Shade_current+1,str,1);
        Print_in_window(210,55,str,MC_Black,MC_Light);
        // Affichade du Pas
        Num2str(Shade_list[Shade_current].Step,str,3);
        Print_in_window(276,176,str,MC_Black,MC_Light);
        // Tracé du bloc dégradé:
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        // On place le nouveau shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        break;

      case  3 : // Gestion de la zone de définition de shades
        if (((((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-127)%7)<4)
        if ( (Mouse_X!=old_mouse_x2) || (Mouse_Y!=old_mouse_y2) || (Mouse_K!=old_mouse_k2) )
        {
          Hide_cursor();
          selection_end=(((((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-127)/7)<<6)+
                      ((((Mouse_X-Window_pos_X)/Menu_factor_X)-8 )>>2);
          if (!old_mouse_k2) // On vient de clicker
            selection_start=selection_end;
          Tag_shades(selection_start,selection_end);
          Display_selected_cell_color(selection_start,selection_end);
          Display_cursor();
        }
        break;

      case 5: // Ok
        if (selection_start == selection_end && Shade_list[Shade_current].List[selection_start] > 0)
          Set_fore_color(Shade_list[Shade_current].List[selection_start]);
        else if (first_color == last_color)
          Set_fore_color(first_color);
        break;

      case  6 : // Copy
        memcpy(buffer,Shade_list[Shade_current].List,512*sizeof(word));
        break;

      case  7 : // Paste
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        memcpy(Shade_list[Shade_current].List,buffer,512*sizeof(word));
        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case  8 : // Insert
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        if (first_color<=last_color)
          temp=last_color-first_color;
        else
          temp=first_color-last_color;

        if (selection_start==selection_end) // Une couleur sélectionnée
        {
          if (Window_attribute1==2)
            Remove_shade(selection_start,selection_start+temp);
        }
        else                          // Un bloc sélectionné
        {
          Remove_shade(selection_start,selection_end);

          if (first_color<=last_color)
            temp=last_color-first_color;
          else
            temp=first_color-last_color;

          if (selection_start<selection_end)
            selection_end=selection_start+temp;
          else
          {
            selection_start=selection_end;
            selection_end+=temp;
          }
        }

        if (selection_start<selection_end)
          selection_end=selection_start+temp;
        else
        {
          selection_start=selection_end;
          selection_end+=temp;
        }
        Insert_shade(first_color,last_color,selection_start);

        // On sélectionne la position juste après ce qu'on vient d'insérer
        selection_start+=temp+1;
        if (selection_start>=512)
          selection_start=511;
        selection_end=selection_start;

        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case  9 : // Delete
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        Remove_shade(selection_start,selection_end);
        if (selection_start<=selection_end)
          selection_end=selection_start;
        else
          selection_start=selection_end;
        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case 10 : // Blank
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        if (Window_attribute1==RIGHT_SIDE)  // Click droit
        {
          if (selection_start!=selection_end)
          {
            if (selection_start<=selection_end)
            {
              Insert_empty_cell_in_shade(selection_start);
              Insert_empty_cell_in_shade(selection_end+2);
            }
            else
            {
              Insert_empty_cell_in_shade(selection_end);
              Insert_empty_cell_in_shade(selection_start+2);
            }
          }
          else
            Insert_empty_cell_in_shade(selection_start);

          if (selection_start<511) selection_start++;
          if (selection_end<511) selection_end++;
        }
        else                              // Click gauche
        {
          if (selection_start<=selection_end)
          {
            temp=selection_start;
            temp2=selection_end;
          }
          else
          {
            temp=selection_end;
            temp2=selection_start;
          }
          while (temp<=temp2)
            Shade_list[Shade_current].List[temp++]=0x0100;
        }

        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case 11 : // Invert
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        if (selection_start<=selection_end)
        {
          temp=selection_start;
          temp2=selection_end;
        }
        else
        {
          temp=selection_end;
          temp2=selection_start;
        }

        for (;temp<temp2;temp++,temp2--)
        {
          temp_cell=Shade_list[Shade_current].List[temp];
          Shade_list[Shade_current].List[temp]=Shade_list[Shade_current].List[temp2];
          Shade_list[Shade_current].List[temp2]=temp_cell;
        }

        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case 12 : // Swap
        temp_cell=Wait_click_in_shade_table();
        if (temp_cell<512)
        {
          // On place le shade dans le buffer du Undo
          memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
          // Et on le modifie
          // On échange le bloc avec sa destination
          if (selection_start<=selection_end)
          {
            temp=(temp_cell+selection_end-selection_start<512)?selection_end+1-selection_start:512-temp_cell;
            Swap_shade(selection_start,temp_cell,temp);
          }
          else
          {
            temp=(temp_cell+selection_start-selection_end<512)?selection_start+1-selection_end:512-temp_cell;
            Swap_shade(selection_end,temp_cell,temp);
          }
          // On place la sélection sur la nouvelle position du bloc
          selection_start=temp_cell;
          selection_end=selection_start+temp-1;
          // Et on raffiche tout
          Hide_cursor();
          Display_all_shade(first_color,last_color,selection_start,selection_end);
          Display_cursor();
        }
        Wait_end_of_click();
        break;

      case 13 : // Set (disable)
      case 14 : // Clear (enable)
        // On place le shade dans le buffer du Undo
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        // Et on le modifie
        if (selection_start<selection_end)
        {
          temp=selection_start;
          temp2=selection_end;
        }
        else
        {
          temp=selection_end;
          temp2=selection_start;
        }

        if (clicked_button==13)
          for (;temp<=temp2;temp++)
            Shade_list[Shade_current].List[temp]|=0x8000;
        else
          for (;temp<=temp2;temp++)
            Shade_list[Shade_current].List[temp]&=0x7FFF;

        Hide_cursor();
        Tag_shades(selection_start,selection_end);
        Shade_draw_grad_ranges();
        Display_cursor();
        break;

      case 15 : // Saisie du pas
        Num2str(Shade_list[Shade_current].Step,str,3);
        Readline(276,176,str,3,INPUT_TYPE_INTEGER);
        temp=atoi(str);
        // On corrige le pas
        if (!temp)
        {
          temp=1;
          Num2str(temp,str,3);
          Window_input_content(input_button,str);
        }
        else if (temp>255)
        {
          temp=255;
          Num2str(temp,str,3);
          Window_input_content(input_button,str);
        }
        Shade_list[Shade_current].Step=temp;
        Display_cursor();
        break;

      case 16 : // Undo
        memcpy(temp_ptr,undo_buffer,512*sizeof(word));
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        memcpy(Shade_list[Shade_current].List,temp_ptr,512*sizeof(word));

        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case 17 : // Clear
        memcpy(undo_buffer,Shade_list[Shade_current].List,512*sizeof(word));
        for (temp=0;temp<512;temp++)
          Shade_list[Shade_current].List[temp]=0x0100;
        Hide_cursor();
        Display_all_shade(first_color,last_color,selection_start,selection_end);
        Display_cursor();
        break;

      case 18 : // Mode
        Shade_list[Shade_current].Mode=(Shade_list[Shade_current].Mode+1)%3;
        Hide_cursor();
        Display_shade_mode(250,110,Shade_list[Shade_current].Mode);
        Display_cursor();
    }

    if (!Mouse_K)
    switch (Key)
    {
      case KEY_LEFTBRACKET : // Décaler couleur dans palette vers la gauche
      case KEY_RIGHTBRACKET : // Décaler couleur dans palette vers la droite
        if (first_color==last_color)
        {
          if (Key==KEY_LEFTBRACKET)
          {
            first_color--;
            last_color--;
          }
          else
          {
            first_color++;
            last_color++;
          }
          Hide_cursor();
          Tag_color_range(first_color,first_color);
          Block(Window_pos_X+(Menu_factor_X*172),
                Window_pos_Y+(Menu_factor_Y*27),
                Menu_factor_X<<4,Menu_factor_Y*64,first_color);
          // On affiche le numéro de la couleur sélectionnée
          Display_selected_color(first_color,last_color);
          Display_cursor();
        }
        Key=0;
        break;

      case KEY_UP    : // Select Haut
      case KEY_DOWN  : // Select Bas
      case KEY_LEFT  : // Select Gauche
      case KEY_RIGHT : // Select Droite
        if (selection_start==selection_end)
        {
          switch (Key)
          {
            case KEY_UP : // Select Haut
              if (selection_start>=64)
              {
                selection_start-=64;
                selection_end-=64;
              }
              else
                selection_start=selection_end=0;
              break;
            case KEY_DOWN : // Select Bas
              if (selection_start<448)
              {
                selection_start+=64;
                selection_end+=64;
              }
              else
                selection_start=selection_end=511;
              break;
            case KEY_LEFT : // Select Gauche
              if (selection_start>0)
              {
                selection_start--;
                selection_end--;
              }
              break;
            default :     // Select Droite
              if (selection_start<511)
              {
                selection_start++;
                selection_end++;
              }
          }
          Hide_cursor();
          Tag_shades(selection_start,selection_start);
          Display_selected_cell_color(selection_start,selection_start);
          Display_cursor();
        }
        Key=0;
        break;

      case KEY_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case KEY_COMMA :
        Get_color_behind_window(&color,&click);
        if (click)
        {
          Hide_cursor();
          temp_color=color;

          // On met à jour l'intervalle du Shade
          first_color=last_color=temp_color;
          // On tagge le bloc
          Tag_color_range(first_color,last_color);
          // Tracé du bloc dégradé:
          Display_grad_block_in_window(172,27,16,64,first_color,last_color);

          // On affiche le numéro de la couleur sélectionnée
          Display_selected_color(first_color,last_color);

          Display_cursor();
          Wait_end_of_click();
        }
        Key=0;
        break;
      default:
        if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Key=0;
          Window_help(BUTTON_EFFECTS, "SHADE");
        }
        else if (Is_shortcut(Key,SPECIAL_SHADE_MENU))
          clicked_button=5;
    }
  }
  while ((clicked_button!=4) && (clicked_button!=5));

  Close_window();
  free(undo_buffer);
  free(buffer);
  free(temp_ptr);

  return (clicked_button==5);
}

/// Handles the screen with Shade settings.
/// @return true if user clicked ok, false if he cancelled
int Shade_settings_menu(void)
{
  T_Shade * initial_shade_list; // Anciennes données des shades
  byte old_shade; // old n° de shade actif
  int return_code;

  // Backup des anciennes données
  initial_shade_list=(T_Shade *)malloc(sizeof(Shade_list));
  memcpy(initial_shade_list,Shade_list,sizeof(Shade_list));
  old_shade=Shade_current;

  return_code = Menu_shade();
  if (!return_code) // Cancel
  {
    memcpy(Shade_list,initial_shade_list,sizeof(Shade_list));
    Shade_current=old_shade;
  }
  else // OK
  {
    Shade_list_to_lookup_tables(Shade_list[Shade_current].List,
                 Shade_list[Shade_current].Step,
                 Shade_list[Shade_current].Mode,
                 Shade_table_left,Shade_table_right);
  }

  free(initial_shade_list);

  Display_cursor();

  return return_code;
}


void Button_Shade_menu(void)
{
  if (Shade_settings_menu())
  {
    // If user clicked OK while in the menu, activate Shade mode.
    if (!Shade_mode)
      Button_Shade_mode();
  }
}


void Button_Quick_shade_menu(void)
{
  short clicked_button;
  int temp;
  char str[4];
  byte step_backup=Quick_shade_step; // Backup des
  byte loop_backup=Quick_shade_loop; // anciennes données
  T_Special_button * step_button;

  Open_window(142,56,"Quick-shade");

  Window_set_normal_button(76,36,60,14,"OK",0,1,KEY_RETURN);     // 1
  Window_set_normal_button( 6,36,60,14,"Cancel",0,1,KEY_ESC);  // 2
  Window_set_normal_button(76,18,60,14,"",0,1,KEY_TAB);          // 3
  Display_shade_mode(83,21,Quick_shade_loop);

  // Déclaration & tracé de la zone de saisie du pas
  Print_in_window(5,21,"Step",MC_Dark,MC_Light);
  step_button = Window_set_input_button(40,19,3);                  // 4
  Num2str(Quick_shade_step,str,3);
  Window_input_content(step_button,str);

  Update_window_area(0,0,142,56);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case 3 : // Mode
        Quick_shade_loop=(Quick_shade_loop+1)%3;
        Hide_cursor();
        Display_shade_mode(83,21,Quick_shade_loop);
        Display_cursor();
        break;

      case 4 : // Saisie du pas
        Num2str(Quick_shade_step,str,3);
        Readline(42,21,str,3,INPUT_TYPE_INTEGER);
        temp=atoi(str);
        // On corrige le pas
        if (!temp)
        {
          temp=1;
          Num2str(temp,str,3);
          Window_input_content(step_button,str);
        }
        else if (temp>255)
        {
          temp=255;
          Num2str(temp,str,3);
          Window_input_content(step_button,str);
        }
        Quick_shade_step=temp;
        Display_cursor();
    }
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_EFFECTS, "QUICK SHADE");
    else if (Is_shortcut(Key,SPECIAL_QUICK_SHADE_MENU))
      clicked_button=1;
  }
  while ((clicked_button!=1) && (clicked_button!=2));

  Close_window();

  if (clicked_button==2) // Cancel
  {
    Quick_shade_step=step_backup;
    Quick_shade_loop=loop_backup;
  }
  else // OK
  {
    // Si avant de rentrer dans le menu on n'était pas en mode Quick-Shade
    if (!Quick_shade_mode)
      Button_Quick_shade_mode(); // => On y passe (cool!)
  }

  Display_cursor();
}
