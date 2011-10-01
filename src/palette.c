/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "struct.h"
#include "global.h"
#include "misc.h"
#include "engine.h"
#include "readline.h"
#include "buttons.h"
#include "pages.h"
#include "help.h"
#include "sdlscreen.h"
#include "errors.h"
#include "op_c.h"
#include "windows.h"
#include "input.h"
#include "palette.h"
#include "shade.h"

byte Palette_view_is_RGB = 1; // Indique si on est en HSL ou en RGB

// Coordinates of the color count (on histogram button)
static const int COUNT_X = 258;
static const int COUNT_Y = 49;
  

// Nombre de graduations pour une composante RGB
int RGB_scale = 256; // 24bit
//int RGB_scale = 64; // VGA
//int RGB_scale = 16; // Amiga
//int RGB_scale =  4; // MSX2
//int RGB_scale =  3; // Amstrad CPC

// Nombre de graduations pour une composante dans le mode actuel
int Color_count=256;
// Les composantes vont de 0 à (Color_count-1)
int Color_max=255;
// Le demi-pas est une quantité que l'on ajoute à une composante
// avant de faire un arrondi par division.
int Color_halfstep=0;


void Set_palette_RGB_scale(int scale)
{
  if (scale>= 2 && scale <= 256)
    RGB_scale = scale;
}

int Get_palette_RGB_scale(void)
{
  return RGB_scale;
}

///
/// Round a 0-255 RGB component according to the RGB_scale.
/// The result is also in the 0-255 range.
byte Round_palette_component(byte comp)
{
  return ((comp+128/RGB_scale)*(RGB_scale-1)/255*255+(RGB_scale&1?1:0))/(RGB_scale-1);
}

///
/// Turns a RGB component from 0-255 scale to 0-(RGB_scale-1).
/// The passed value should come from Round_palette_component(),
/// otherwise the rounding will be "down".
int Reduce_component(int comp)
{
  return (comp)*255/Color_max;
}

///
/// Turns a RGB component from 0-(RGB_scale-1) to 0-255.
int Expand_component(int comp)
{
  if (Color_max==255)
    return comp;
  return (comp+1)*Color_max/255;
  // The +1 cancel any rounding down, the above test prevents
  // the only case where it would be too much.
}

// Définir les unités pour les graduations R G B ou H S V
void Component_unit(int count)
{
  Color_count = count;
  Color_max = count-1;
  Color_halfstep = 256/count/2;
}

void Set_HSL(T_Palette start_palette, T_Palette end_palette, byte color, short diff_h, short diff_s, short diff_l)
{
    byte h, s, l;
    RGB_to_HSL(start_palette[color].R,start_palette[color].G,start_palette[color].B,&h,&s,&l);
    // La teinte (Hue) est cyclique
    h=(diff_h+256+h);
    // Pour les autres (Saturation, Lightness), au lieu d'additionner,
    // on va faire un ratio, cela utilise mieux la plage de valeurs 0-255
    if (diff_s<0)
      s=(255+diff_s)*s/255;
    else if (diff_s>0)
      s=255-(255-diff_s)*(255-s)/255;
    if (diff_l<0)
      l=(255+diff_l)*l/255;
    else if (diff_l>0)
      l=255-(255-diff_l)*(255-l)/255;
    HSL_to_RGB(h,s,l,&end_palette[color].R,&end_palette[color].G,&end_palette[color].B);
}

void Set_red(byte color, short new_color, T_Palette palette)
{
  if (new_color< 0)
    new_color= 0;
  if (new_color>255)
    new_color=255;
  // Arrondi
  new_color=Round_palette_component(new_color);

  palette[color].R=new_color;
}


void Set_green(byte color, short new_color, T_Palette palette)
{
  if (new_color< 0)
    new_color= 0;
  if (new_color>255)
    new_color=255;
  // Arrondi
  new_color=Round_palette_component(new_color);

  palette[color].G=new_color;
}


void Set_blue(byte color, short new_color, T_Palette palette)
{
  if (new_color< 0)
    new_color= 0;
  if (new_color>255)
    new_color=255;
  // Arrondi
  new_color=Round_palette_component(new_color);

  palette[color].B=new_color;
}

void Format_component(byte value, char *str)
// Formate une chaine de 4 caractères+\0 : "nnn "
{
  Num2str(value,str,3);
  str[3]=' ';
  str[4]='\0';
}

void Spread_colors(short start,short end,T_Palette palette)
// Modifie la palette pour obtenir un dégradé de couleur entre les deux bornes
// passées en paramètre
{
  short start_red;
  short start_green;
  short start_blue;
  short end_red;
  short end_green;
  short end_blue;
  short index;

  // On vérifie qu'il y ait assez de couleurs entre le début et la fin pour
  // pouvoir faire un dégradé:
  if ( (start!=end) && (start+1!=end) )
  {
    start_red=palette[start].R;
    start_green =palette[start].G;
    start_blue =palette[start].B;

    end_red  =palette[end  ].R;
    end_green   =palette[end  ].G;
    end_blue   =palette[end  ].B;

    for (index=start+1;index<end;index++)
    {
      Set_red(index, ((end_red-start_red) * (index-start))/(end-start) + start_red,palette);
      Set_green (index, ((end_green -start_green ) * (index-start))/(end-start) + start_green ,palette);
      Set_blue (index, ((end_blue -start_blue ) * (index-start))/(end-start) + start_blue ,palette);
    }
    Set_palette(palette);
  }
}

// FIXME: keep them in main function but pass them to Palette_edit_*()
  T_Components * backup_palette;
  T_Components * temp_palette;
  T_Components * working_palette;
  
byte Palette_undo_state = 0;
byte Palette_change_state = 0;

/// Backup before doing one self-complete change.
void Palette_edit_step()
{
  // back up
  memcpy(backup_palette,working_palette,sizeof(T_Palette));
  Palette_change_state=0;
  Palette_undo_state=0;
}

/// Mode for incremental changes.
void Palette_edit_select_range()
{
  if (Palette_change_state)
  {/*
    // acknowledge pending changes and back up
    memcpy(backup_palette,working_palette,sizeof(T_Palette));
    memcpy(temp_palette,working_palette,sizeof(T_Palette));
  */}
  Palette_change_state=0;
}

/// Perform incremental change in RGB channel of some color(s).
void Palette_edit_alter_channel()
{
  if (!Palette_change_state)
  {
    memcpy(backup_palette,working_palette,sizeof(T_Palette));
    memcpy(temp_palette,working_palette,sizeof(T_Palette));
  }
  Palette_change_state=1;
}

/// Undo
void Palette_edit_undo_redo()
{
  if (Palette_change_state)
  {
    // swap backup and working (temp serves as intermediate)
    memcpy(temp_palette,backup_palette,sizeof(T_Palette));
    memcpy(backup_palette,working_palette,sizeof(T_Palette));
    memcpy(working_palette,temp_palette,sizeof(T_Palette));
  }
  else
  {
    // swap backup and working (temp serves as intermediate)
    memcpy(temp_palette,backup_palette,sizeof(T_Palette));
    memcpy(backup_palette,working_palette,sizeof(T_Palette));
    memcpy(working_palette,temp_palette,sizeof(T_Palette));
  }
  Palette_undo_state=(Palette_undo_state==0);
}


void Update_color_count(short * used_colors, dword * color_usage)
{
  char   str[4];

  Hide_cursor();
  Cursor_shape=CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  *used_colors=Count_used_colors(color_usage);
  Num2str(*used_colors,str,3);
  Hide_cursor();
  Print_in_window(COUNT_X,COUNT_Y,str,MC_Black,MC_Light);
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Display_cursor();
}

void Remap_zone_highlevel(short x1, short y1, short x2, short y2,
                     byte * conversion_table)
// Attention: Remappe une zone de coins x1,y1 et x2-1,y2-1 !!!
{
  short x_pos;
  short y_pos;

  for (y_pos=y1;y_pos<y2;y_pos++)
    for (x_pos=x1;x_pos<x2;x_pos++)
    {
      if ((y_pos>=Window_pos_Y) && (y_pos<Window_pos_Y+(Window_height*Menu_factor_Y)) &&
          (x_pos>=Window_pos_X) && (x_pos<Window_pos_X+(Window_width*Menu_factor_X)) )
        x_pos=Window_pos_X+(Window_width*Menu_factor_X)-1;
      else
        Pixel(x_pos,y_pos,conversion_table[Read_pixel(x_pos,y_pos)]);
    }
}

void Remap_image_highlevel(byte * conversion_table)
{
  short end_x;
  short end_y;
  short end_x_mag=0;
  short end_y_mag=0;
  int layer;

  // Remap the flatenned image view
  Remap_general_lowlevel(conversion_table,Main_screen,Main_screen,
                         Main_image_width,Main_image_height,Main_image_width);

  // Remap all layers
  for (layer=0; layer<Main_backups->Pages->Nb_layers; layer++)
    Remap_general_lowlevel(conversion_table,Main_backups->Pages->Image[layer],Main_backups->Pages->Image[layer],Main_image_width,Main_image_height,Main_image_width);

  // Remap transparent color
  Main_backups->Pages->Transparent_color = 
    conversion_table[Main_backups->Pages->Transparent_color];

  // On calcule les limites à l'écran de l'image
  if (Main_image_height>=Menu_Y_before_window)
    end_y=Menu_Y_before_window;
  else
    end_y=Main_image_height;

  if (!Main_magnifier_mode)
  {
    if (Main_image_width>=Screen_width)
      end_x=Screen_width;
    else
      end_x=Main_image_width;

  }
  else
  {
    if (Main_image_width>=Main_separator_position)
      end_x=Main_separator_position;
    else
      end_x=Main_image_width;

    if ((Main_X_zoom+(Main_image_width*Main_magnifier_factor))>=Screen_width)
      end_x_mag=Screen_width;
    else
      end_x_mag=(Main_X_zoom+(Main_image_width*Main_magnifier_factor));

    if (Main_image_height*Main_magnifier_factor>=Menu_Y_before_window)
      end_y_mag=Menu_Y_before_window;
    else
      end_y_mag=Main_image_height*Main_magnifier_factor;
  }

  // On doit maintenant faire la traduction à l'écran
  Remap_zone_highlevel(0,0,end_x,end_y,conversion_table);

  if (Main_magnifier_mode)
  {
    Remap_zone_highlevel(Main_separator_position,0,end_x_mag,end_y_mag,conversion_table);
    // Il peut encore rester le bas de la barre de split à remapper si la
    // partie zoomée ne descend pas jusqu'en bas...
    Remap_zone_highlevel(Main_separator_position,end_y_mag,
                    (Main_separator_position+(SEPARATOR_WIDTH*Menu_factor_X)),
                    Menu_Y_before_window,conversion_table);
  }
  // Remappe tous les fonds de fenetre (qui doivent contenir un bout d'écran)
  Remap_window_backgrounds(conversion_table, 0, Menu_Y_before_window);
}


void Swap(int with_remap,short block_1_start,short block_2_start,short block_size,T_Palette palette, dword * color_usage)
{
  short pos_1;
  short pos_2;
  short end_1;
  short end_2;
  byte  conversion_table[256];

  T_Components temp_palette[256];
  dword temp_usage[256];

  // On fait une copie de la palette
  memcpy(temp_palette, palette, sizeof(T_Palette));

  // On fait une copie de la table d'used des couleurs
  memcpy(temp_usage, color_usage, sizeof(dword) * 256);

  // On commence à initialiser la table de conversion à un état où elle ne
  // fera aucune conversion.
  for (pos_1=0;pos_1<=255;pos_1++)
    conversion_table[pos_1]=pos_1;

  // On calcul les dernières couleurs de chaque bloc.
  end_1=block_1_start+block_size-1;
  end_2=block_2_start+block_size-1;

  if ((block_2_start>=block_1_start) && (block_2_start<=end_1))
  {
    // Le bloc destination commence dans le bloc source.

    for (pos_1=block_1_start,pos_2=end_1+1;pos_1<=end_2;pos_1++)
    {
      // Il faut transformer la couleur pos_1 en pos_2:

      conversion_table[pos_2]=pos_1;
      color_usage[pos_1]=temp_usage[pos_2];
      palette[pos_1].R=temp_palette[pos_2].R;
      palette[pos_1].G=temp_palette[pos_2].G;
      palette[pos_1].B=temp_palette[pos_2].B;

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

      conversion_table[pos_2]=pos_1;
      color_usage[pos_1]=temp_usage[pos_2];
      palette[pos_1].R=temp_palette[pos_2].R;
      palette[pos_1].G=temp_palette[pos_2].G;
      palette[pos_1].B=temp_palette[pos_2].B;

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
      // Il va falloir permutter la couleur pos_1 avec la couleur pos_2
      conversion_table[pos_1]=pos_2;
      conversion_table[pos_2]=pos_1;

      //   On intervertit le nombre d'used des couleurs pour garder une
      // cohérence lors d'un éventuel "Zap unused".
      SWAP_DWORDS(color_usage[pos_1], color_usage[pos_2])

      // On fait un changement de teinte:
      SWAP_BYTES(palette[pos_1].R, palette[pos_2].R)
      SWAP_BYTES(palette[pos_1].G, palette[pos_2].G)
      SWAP_BYTES(palette[pos_1].B, palette[pos_2].B)
    }
  }

  if (with_remap)
  {
    Remap_image_highlevel(conversion_table);
  }
  else
  {
    // Restore color usage. Shouldn't have reordered it in the first place.
    memcpy(color_usage, temp_usage, sizeof(dword) * 256);
  }
}



void Set_nice_menu_colors(dword * color_usage,int not_picture)
{
  short index,index2;
  byte color;
  byte replace_table[256];
  T_Components rgb[4];
  short new_colors[4]={255,254,253,252};

  // On initialise la table de remplacement
  for (index=0; index<256; index++)
    replace_table[index]=index;

  // On recherche les 4 couleurs les moins utilisées dans l'image pour pouvoir
  // les remplacer par les nouvelles couleurs.
  for (index2=0; index2<4; index2++)
    for (index=255; index>=0; index--)
    {
      if ((index!=new_colors[0]) && (index!=new_colors[1])
       && (index!=new_colors[2]) && (index!=new_colors[3])
       && (color_usage[index]<color_usage[new_colors[index2]]))
        new_colors[index2]=index;
    }

  // On trie maintenant la table dans le sens décroissant.
  // (Ce n'est pas indispensable, mais ça fera plus joli dans la palette).
  do
  {
    color=0; // Booléen qui dit si le tri n'est pas terminé.
    for (index=0; index<3; index++)
    {
      if (new_colors[index]>new_colors[index+1])
      {
        index2            =new_colors[index];
        new_colors[index]  =new_colors[index+1];
        new_colors[index+1]=index2;
        color=1;
      }
    }
  } while (color);

  //   On sauvegarde dans rgb les teintes qu'on va remplacer et on met les
  // couleurs du menu par défaut
  for (index=0; index<4; index++)
  {
    const T_Components * target_rgb;

    target_rgb=Favorite_GUI_color(index);
    color=new_colors[index];
    rgb[index].R=Main_palette[color].R;
    rgb[index].G=Main_palette[color].G;
    rgb[index].B=Main_palette[color].B;
    Main_palette[color].R=Round_palette_component(target_rgb->R);
    Main_palette[color].G=Round_palette_component(target_rgb->G);
    Main_palette[color].B=Round_palette_component(target_rgb->B);
  }

  //   Maintenant qu'on a placé notre nouvelle palette, on va chercher quelles
  // sont les couleurs qui peuvent remplacer les anciennes
  Hide_cursor();
  for (index=0; index<4; index++)
    replace_table[new_colors[index]]=Best_color_nonexcluded
                                  (rgb[index].R,rgb[index].G,rgb[index].B);

  if (not_picture)
  {
    // Remap caused by preview. Only remap screen
    Remap_zone_highlevel(0,0,Screen_width,Screen_height,replace_table);
  }
  else
  {
    // On fait un changement des couleurs visibles à l'écran et dans l'image
    Remap_image_highlevel(replace_table);
  }
  Display_cursor();
}



void Reduce_palette(short * used_colors,int nb_colors_asked,T_Palette palette,dword * color_usage)
{
  char  str[5];                // buffer d'affichage du compteur
  byte  conversion_table[256]; // Table de conversion
  int   color_1;                // |_ Variables de balayages
  int   color_2;                // |  de la palette
  int   best_color_1=0;
  int   best_color_2=0;
  long   difference;
  long   best_difference;
  dword used;
  dword best_used;

  //   On commence par initialiser la table de conversion dans un état où
  // aucune conversion ne sera effectuée.
  for (color_1=0; color_1<=255; color_1++)
    conversion_table[color_1]=color_1;

  //   Si on ne connait pas encore le nombre de couleurs utilisées, on le
  // calcule! (!!! La fonction appelée Efface puis Affiche le curseur !!!)
  if ((*used_colors)<0)
    Update_color_count(used_colors,color_usage);

  Hide_cursor();

  //   On tasse la palette vers le début parce qu'elle doit ressembler à
  // du Gruyère (et comme Papouille il aime pas le fromage...)

  // Pour cela, on va scruter la couleur color_1 et se servir de l'indice
  // color_2 comme position de destination.
  for (color_1=color_2=0;color_1<=255;color_1++)
  {
    if (color_usage[color_1])
    {
      // On commence par s'occuper des teintes de la palette
      palette[color_2].R=palette[color_1].R;
      palette[color_2].G=palette[color_1].G;
      palette[color_2].B=palette[color_1].B;

      // Ensuite, on met à jour le tableau d'occupation des couleurs.
      color_usage[color_2]=color_usage[color_1];

      // On va maintenant s'occuper de la table de conversion:
      conversion_table[color_1]=color_2;

      // Maintenant, la place désignée par color_2 est occupée, alors on
      // doit passer à un indice de destination suivant.
      color_2++;
    }
  }

  // On met toutes les couleurs inutilisées en noir
  for (;color_2<256;color_2++)
  {
    palette[color_2].R=0;
    palette[color_2].G=0;
    palette[color_2].B=0;
    color_usage[color_2]=0;
  }

  //   Maintenant qu'on a une palette clean, on va boucler en réduisant
  // le nombre de couleurs jusqu'à ce qu'on atteigne le nombre désiré.
  // (The stop condition is further down)
  while (1)
  {
    //   Il s'agit de trouver les 2 couleurs qui se ressemblent le plus
    // parmis celles qui sont utilisées (bien sûr) et de les remplacer par
    // une seule couleur qui est la moyenne pondérée de ces 2 couleurs
    // en fonction de leur utilisation dans l'image.

    best_difference =26*255*26*255+55*255*255+19*255*19*255;
    best_used=0x7FFFFFFF;

    for (color_1=0;color_1<(*used_colors);color_1++)
      for (color_2=color_1+1;color_2<(*used_colors);color_2++)
        if (color_1!=color_2)
        {
          difference =26*abs((long)palette[color_1].R-palette[color_2].R)*
                      26*abs((long)palette[color_1].R-palette[color_2].R)+
                      55*abs((long)palette[color_1].G-palette[color_2].G)*
                      55*abs((long)palette[color_1].G-palette[color_2].G)+
                      19*abs((long)palette[color_1].B-palette[color_2].B)*
                      19*abs((long)palette[color_1].B-palette[color_2].B);

          if (difference<=best_difference)
          {
            used=color_usage[color_1]+color_usage[color_2];
            if ((difference<best_difference) || (used<best_used))
            {
              best_difference =difference;
              best_used=used;
              best_color_1  =color_1;
              best_color_2  =color_2;
            }
          }
        }
    
    // Stop condition: when no more duplicates exist
    // and the number of colors has reached the target.
    if (best_difference!=0 && (*used_colors)<=nb_colors_asked)
      break;

    //   Maintenant qu'on les a trouvées, on va pouvoir mettre à jour nos
    // données pour que le remplacement se fasse sans encombres.

    // En somme, on va remplacer best_color_2 par best_color_1,
    // mais attention, on ne remplace pas best_color_1 par
    // best_color_2 !

    // On met à jour la palette.
    palette[best_color_1].R=Round_div((color_usage[best_color_1]*palette[best_color_1].R)+
                                             (color_usage[best_color_2]*palette[best_color_2].R),
                                             best_used);
    palette[best_color_1].G=Round_div((color_usage[best_color_1]*palette[best_color_1].G)+
                                             (color_usage[best_color_2]*palette[best_color_2].G),
                                             best_used);
    palette[best_color_1].B=Round_div((color_usage[best_color_1]*palette[best_color_1].B)+
                                             (color_usage[best_color_2]*palette[best_color_2].B),
                                             best_used);

    // On met à jour la table d'utilisation.
    color_usage[best_color_1]+=color_usage[best_color_2];
    color_usage[best_color_2]=0;

    // On met à jour la table de conversion.
    for (color_1=0;color_1<=255;color_1++)
    {
      if (conversion_table[color_1]==best_color_2)
      {
        //   La color_1 avait déjà prévue de se faire remplacer par la
        // couleur que l'on veut maintenant éliminer. On va maintenant
        // demander à ce que la color_1 se fasse remplacer par la
        // best_color_1.
        conversion_table[color_1]=best_color_1;
      }
    }

    //   Bon, maintenant que l'on a fait bouger nos petites choses concernants
    // la couleur à éliminer, on va s'occuper de faire bouger les couleurs
    // situées après la couleur à éliminer pour qu'elles se déplaçent d'une
    // couleur en arrière.
    for (color_1=0;color_1<=255;color_1++)
    {
      //   Commençons par nous occuper des tables d'utilisation et de la
      // palette.

      if (color_1>best_color_2)
      {
        // La color_1 va scroller en arrière.

        //   Donc on transfère son utilisation dans l'utilisation de la
        // couleur qui la précède.
        color_usage[color_1-1]=color_usage[color_1];

        //   Et on transfère ses teintes dans les teintes de la couleur qui
        // la précède.
        palette[color_1-1].R=palette[color_1].R;
        palette[color_1-1].G=palette[color_1].G;
        palette[color_1-1].B=palette[color_1].B;
      }

      //   Une fois la palette et la table d'utilisation gérées, on peut
      // s'occuper de notre table de conversion.
      if (conversion_table[color_1]>best_color_2)
        //   La color_1 avait l'intention de se faire remplacer par une
        // couleur que l'on va (ou que l'on a déjà) bouger en arrière.
        conversion_table[color_1]--;
    }

    //   On vient d'éjecter une couleur, donc on peut mettre à jour le nombre
    // de couleurs utilisées.
    (*used_colors)--;

    // A la fin, on doit passer (dans la palette) les teintes du dernier
    // élément de notre ensemble en noir.
    palette[*used_colors].R=0;
    palette[*used_colors].G=0;
    palette[*used_colors].B=0;

    // Au passage, on va s'assurer que l'on a pas oublié de la mettre à une
    // utilisation nulle.
    color_usage[*used_colors]=0;

    // Après avoir éjecté une couleur, on le fait savoir à l'utilisateur par
    // l'intermédiaire du compteur de nombre utilisées.
    Num2str(*used_colors,str,3);
    Print_in_window(COUNT_X,COUNT_Y,str,MC_Black,MC_Light);
  }

  //   Maintenant, tous ces calculs doivent êtres pris en compte dans la
  // palette, l'image et à l'écran.
  Remap_image_highlevel(conversion_table); // Et voila pour l'image et l'écran
  Display_cursor();
}


// Position of the numeric values of the R G B sliders
static const int NUMERIC_R_X = 176;
static const int NUMERIC_G_X = 203;
static const int NUMERIC_B_X = 230;
static const int NUMERIC_Y   = 171;
// Position of the whole button
static const int NUMERIC_BOX_X = 175;
static const int NUMERIC_BOX_Y = 169;
static const int NUMERIC_BOX_W = 81;
static const int NUMERIC_BOX_H = 12;

void Set_palette_slider(T_Scroller_button * slider,
                            word nb_elements, word position,
                            char * value, short x_pos)
{
  slider->Nb_elements=nb_elements;
  slider->Position=position;
  Compute_slider_cursor_length(slider);
  Window_draw_slider(slider);
  Print_counter(x_pos,NUMERIC_Y,value,MC_Black,MC_Light);
}



void Display_sliders(T_Scroller_button * red_slider,
                         T_Scroller_button * green_slider,
                         T_Scroller_button * blue_slider,
                         byte block_is_selected, T_Components * palette)
{
  char str[5];

  if (block_is_selected)
  {
    Set_palette_slider(red_slider,Color_max*2+1,Color_max,"±  0",NUMERIC_R_X);
    Set_palette_slider(green_slider,Color_max*2+1,Color_max,"±  0",NUMERIC_G_X);
    Set_palette_slider(blue_slider,Color_max*2+1,Color_max,"±  0",NUMERIC_B_X);
  }
  else
  {
    byte j1, j2, j3;
    j1= palette[Fore_color].R;
    j2= palette[Fore_color].G;
    j3= palette[Fore_color].B;
    if (!Palette_view_is_RGB)
    {
      RGB_to_HSL(j1,j2,j3,&j1,&j2,&j3);
    }

    Format_component(j1*Color_count/256,str);
    Set_palette_slider(red_slider,Color_count,Color_max-Expand_component(j1),str,NUMERIC_R_X);
    Format_component(j2*Color_count/256,str);
    Set_palette_slider(green_slider,Color_count,Color_max-Expand_component(j2),str,NUMERIC_G_X);
    Format_component(j3*Color_count/256,str);
    Set_palette_slider(blue_slider,Color_count,Color_max-Expand_component(j3),str,NUMERIC_B_X);
  }
}

void Draw_all_palette_sliders(T_Scroller_button * red_slider,
                               T_Scroller_button * green_slider,
                               T_Scroller_button * blue_slider,
                               T_Palette palette,byte start,byte end)
{
  char str[5];

  Hide_cursor();
  // Réaffichage des jauges:
  if (start!=end)
  {
    // Dans le cas d'un bloc, tout à 0.
    red_slider->Position   =Color_max;
    Window_draw_slider(red_slider);
    Print_counter(NUMERIC_R_X,NUMERIC_Y,"±  0",MC_Black,MC_Light);

    green_slider->Position   =Color_max;
    Window_draw_slider(green_slider);
    Print_counter(NUMERIC_G_X,NUMERIC_Y,"±  0",MC_Black,MC_Light);

    blue_slider->Position   =Color_max;
    Window_draw_slider(blue_slider);
    Print_counter(NUMERIC_B_X,NUMERIC_Y,"±  0",MC_Black,MC_Light);
  }
  else
  {
    // Dans le cas d'une seule couleur, composantes.
    byte j1, j2, j3;
    j1= palette[start].R;
    j2= palette[start].G;
    j3= palette[start].B;
    if (!Palette_view_is_RGB)
    {
      RGB_to_HSL(j1,j2,j3,&j1,&j2,&j3);
    }
    DEBUG("j1",j1);
    Format_component(j1*Color_count/256,str);
    red_slider->Position=Color_max-Expand_component(j1);
    Window_draw_slider(red_slider);
    Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);

    Format_component(j2*Color_count/256,str);
    green_slider->Position=Color_max-Expand_component(j2);
    Window_draw_slider(green_slider);
    Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);

    Format_component(j3*Color_count/256,str);
    blue_slider->Position=Color_max-Expand_component(j3);
    Window_draw_slider(blue_slider);
    Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
  }
  Display_cursor();
}


int Window_Histogram(unsigned char block_start, unsigned char block_end, dword* color_usage)
{
  int i, j;
  unsigned int max_count = 0;
  int old_height=0;
  int hovered_color=-1;
  int new_hovered_color;
  int bar_width;
  T_Special_button *histo;
  int clicked_button;

  /* Draws an histogram of the selected range in a separate window */
  
  if (block_start == block_end) {
    // only one color selected: auto-detect the range
    for (block_start=0; block_start!=255; block_start++)
      if (color_usage[block_start])
        break;
    for (block_end=255; block_end!=0; block_end--)
      if (color_usage[block_end])
        break;
  }

  // Normalize the histogram towards the most used color
  // Step 1 : find the most used color in the range
  for(i=block_start; i<= block_end; i++) {
    if(color_usage[i] > max_count) max_count = color_usage[i];
  }

  if (max_count == 0)
  {
    Warning_message("All these colors are unused!");
    Hide_cursor();
    return -1;
  }
  
  Open_window(263, 150, "Histogram");
  Window_set_normal_button(120, 130, 42, 14, "Close",-1,1,SDLK_RETURN);

  Print_in_window(6, 17, "Color:", MC_Dark, MC_Light);
  Print_in_window(110+12*8, 17, "Pixels", MC_Dark, MC_Light);

  // Step 2 : draw bars
  bar_width=256/(block_end-block_start+1);
  j = 0;
  for(i=block_start; i<= block_end; i++) {
    int height = 100*color_usage[i]/max_count;
    // Don't draw anything if the color is unused
    if (color_usage[i]!=0)
    {
      // Draw at least one pixel if the color is used
      if (height==0)
        height=1;
        
      Window_rectangle(
            3+j*bar_width,
            127-height,
            bar_width,
            height, i);
    
      //if (i == MC_Light) {
        Window_rectangle(
              3+j*bar_width,
              126-height,
              bar_width,
              1,MC_Black);
      //}
    }
    // vertical outline
    if (height>old_height)
      Window_rectangle(
              2+j*bar_width,
              126-height,
              1,
              height-old_height+1,MC_Black);
    else if (old_height>height)
      Window_rectangle(
              3+j*bar_width,
              126-old_height,
              1,
              old_height-height+1,MC_Black);
      
    old_height=height;
    j++;
  }
  // Last vertical outline
  if (old_height!=0)
  Window_rectangle(
      3+j*(256/(block_end-block_start+1)),
      126-old_height,
      1,
      old_height+1,MC_Black);

  histo = Window_set_special_button(3, 27, j*bar_width, 100); // 2

  Update_window_area(0,0,263,150);
  Display_cursor();
  do
  {
    // Find hovered area
    if (Window_click_in_rectangle(histo->Pos_X,histo->Pos_Y,histo->Pos_X+histo->Width-1,histo->Pos_Y+histo->Height-1))
    {
      short x_pos;
      x_pos=((short)Mouse_X-Window_pos_X)/Menu_factor_X;
      new_hovered_color=block_start+(x_pos-histo->Pos_X)/bar_width;
    }
    else
      new_hovered_color=-1;
    
    // When changing hovered color, update the info area
    if (new_hovered_color!=hovered_color)
    {
      char str[12];
      
      hovered_color=new_hovered_color;
      Hide_cursor();
      if (hovered_color==-1)
      {
        Window_rectangle(6+6*8,17,3*8,7,MC_Light);
        Update_window_area(6+6*8,17,3*8,7);
        Window_rectangle(86,17,2*8,8,MC_Light);
        Update_window_area(86,17,2*8,8);
        Window_rectangle(110,17,11*8,7,MC_Light);
        Update_window_area(110,17,11*8,7);
      }
      else
      {
        Num2str(hovered_color,str  ,3);
        Print_in_window(6+6*8,17,str,MC_Black,MC_Light);
        Window_rectangle(86,17,2*8,8,hovered_color);
        Update_window_area(86,17,2*8,8);
        Num2str(color_usage[hovered_color],str  ,11);
        Print_in_window(110,17,str,MC_Black,MC_Light);
      }
      Display_cursor();
    }
    clicked_button=Window_clicked_button();
    if (Key == KEY_ESC)
      clicked_button=1;
      
  } while( clicked_button < 1);
  Close_window();
  
  if (clicked_button==2)
  {
    // This is a counter-hack. Close_window() sets Mouse_K to zero
    // on exit, I don't know why (It will become 1 again if you move
    // the mouse slightly)
    // Here I force it back to 1, so that the Wait_end_of_click()
    // will really wait for a release of mouse button.
    Mouse_K=1;
    return hovered_color;  
  }
  return -1;
}

void Print_RGB_or_HSL(byte mode)
{
  Print_in_window(184,68,mode?"H":"R",MC_Dark,MC_Light);
  Print_in_window(211,68,mode?"S":"G",MC_Dark,MC_Light);
  Print_in_window(238,68,mode?"L":"B",MC_Dark,MC_Light);
}

void Tag_used_colors(byte color, dword color_usage[])
{
  word index;

  for (index=0;index<=255;index++)
  {
    short x_pos=Window_palette_button_list->Pos_X+6+((index>>4)*10);
    short y_pos=Window_palette_button_list->Pos_Y+3+((index&15)* 5);
    byte col;
    
    col=(color&&color_usage[index])?MC_White:MC_Light;
    Window_rectangle(x_pos+5,y_pos+0,1,5,col);
  }

  Update_window_area(Window_palette_button_list->Pos_X+3,Window_palette_button_list->Pos_Y+3,12*16,5*16);
}

void Button_Palette(void)
{
  static const int BUTTON_PLUS_X = 268;
  static const int BUTTON_PLUS_Y = 74;
  static const int BUTTON_MINUS_X = 268;
  static const int BUTTON_MINUS_Y = 165;

  // Coordinates of the block that displays Backcolor
  static const int BGCOLOR_DISPLAY_X = 262;
  static const int BGCOLOR_DISPLAY_Y = 89;
  static const int BGCOLOR_DISPLAY_W = 24;
  static const int BGCOLOR_DISPLAY_H = 72;

  // Coordinates of the block that displays Forecolor
  static const int FGCOLOR_DISPLAY_X = 266;
  static const int FGCOLOR_DISPLAY_Y = 93;
  static const int FGCOLOR_DISPLAY_W = 16;
  static const int FGCOLOR_DISPLAY_H = 64;
  
  // Coordinates of the Color#
  static const int COLOR_X = 111;
  static const int COLOR_Y = 69;
  
  
  static short reduce_colors_number = 256;
  short temp_color; // Variable pouvant reservir pour différents calculs intermédiaires
  dword temp;
  byte  color,click; // Variables pouvant reservir pour différents calculs intermédiaires
  short clicked_button;
  word  old_mouse_x;
  word  old_mouse_y;
  byte  old_mouse_k;
  byte  block_start;
  byte  block_end;
  byte  first_color;
  byte  last_color;
  char  str[10];
  word  i;
  T_Normal_button   * button_used;
  T_Scroller_button * red_slider;
  T_Scroller_button * green_slider;
  T_Scroller_button * blue_slider;
  T_Dropdown_button * reduce_dropdown;
  T_Dropdown_button * sort_dropdown;
  byte   image_is_backed_up = 0;
  byte   need_to_remap = 0;

  dword  color_usage[256];
  short  used_colors = -1; // -1 <=> Inconnu
  byte   conversion_table[256];

  //T_Components * backup_palette;
  //T_Components * temp_palette;
  //T_Components * working_palette;
  
  static byte show_used_colors=0;

  backup_palette =(T_Components *)malloc(sizeof(T_Palette));
  temp_palette=(T_Components *)malloc(sizeof(T_Palette));
  working_palette=(T_Components *)malloc(sizeof(T_Palette));

  Component_unit(RGB_scale);

  Open_window(299, 188,"Palette");

  memcpy(working_palette, Main_palette, sizeof(T_Palette));
  Palette_edit_step();

  Window_set_palette_button(5, 79); // 1

  Window_display_frame (172, 63, 122, 121);

  // Graduation des jauges de couleur
  Window_rectangle(180,106,17,1,MC_Dark);
  Window_rectangle(207,106,17,1,MC_Dark);
  Window_rectangle(234,106,17,1,MC_Dark);
  Window_rectangle(180,122,17,1,MC_Dark);
  Window_rectangle(207,122,17,1,MC_Dark);
  Window_rectangle(234,122,17,1,MC_Dark);
  Window_rectangle(180,138,17,1,MC_Dark);
  Window_rectangle(207,138,17,1,MC_Dark);
  Window_rectangle(234,138,17,1,MC_Dark);
  // Jauges de couleur
  red_slider = Window_set_scroller_button(183, 79, 88,Color_count,1,Color_max-Reduce_component(working_palette[Fore_color].R));// 2
  green_slider = Window_set_scroller_button(210, 79, 88,Color_count,1,Color_max-Reduce_component(working_palette[Fore_color].G));// 3
  blue_slider = Window_set_scroller_button(237, 79, 88,Color_count,1,Color_max-Reduce_component(working_palette[Fore_color].B));// 4

  if(Palette_view_is_RGB==1) {
      Print_RGB_or_HSL(0);
      Component_unit(RGB_scale);
  } else {
      Print_RGB_or_HSL(1);
      Component_unit(256);
  }

  first_color=last_color=block_start=block_end=Fore_color;
  Tag_color_range(block_start,block_end);

  // Affichage dans le block de visu de la couleur en cours
  Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H,Back_color);
  Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);

  // Affichage des valeurs de la couleur courante (pour 1 couleur)
  Display_sliders(red_slider,green_slider,blue_slider,(block_start!=block_end),working_palette);

  Print_in_window(7, 69, "Color number:", MC_Dark, MC_Light);
  Num2str(Fore_color, str, 3);
  Print_in_window(COLOR_X, COLOR_Y, str, MC_Black, MC_Light);

  Window_set_normal_button(  7,16,55,14,"Merge" ,0,1,SDLK_m);    // 5
  Window_set_normal_button( 63,16,36,14,"Gray"   ,1,1,SDLK_g);   // 6
  Window_set_normal_button(  7,46,55,14,"Swap"   ,0,1,KEY_NONE); // 7
  Window_set_normal_button( 63,46,72,14,"X-Swap" ,1,1,SDLK_x);   // 8
  Window_set_normal_button(136,31,54,14,"Copy"   ,1,1,SDLK_c);   // 9
  Window_set_normal_button(136,46,54,14,"Spread" ,4,1,SDLK_e);   // 10

  reduce_dropdown = Window_set_dropdown_button(209, 46, 83, 14, 84, "Reduce", 0,
    0, 1, RIGHT_SIDE|LEFT_SIDE, 0); // 11
  Window_dropdown_add_item(reduce_dropdown, 256, "to uniques");
  Window_dropdown_add_item(reduce_dropdown, 128, "to 128");
  Window_dropdown_add_item(reduce_dropdown,  64, "to 64");
  Window_dropdown_add_item(reduce_dropdown,  32, "to 32");
  Window_dropdown_add_item(reduce_dropdown,  16, "to 16");
  Window_dropdown_add_item(reduce_dropdown,   8, "to 8");
  Window_dropdown_add_item(reduce_dropdown,   4, "to 4");
  Window_dropdown_add_item(reduce_dropdown,   2, "to 2");
  Window_dropdown_add_item(reduce_dropdown,   0, "Other");

  Window_set_normal_button(  6,168,35,14,"Undo"  ,1,1,SDLK_u);  // 12
  Window_set_normal_button( 62,168,51,14,"Cancel",0,1,KEY_ESC);  // 13
  Window_set_normal_button(117,168,51,14,"OK"    ,0,1,SDLK_RETURN);  // 14

  Window_set_normal_button(209,16,37,14,"Used",0,1,SDLK_d); // 15
  Window_set_normal_button(209,31,83,14,"Zap unused",0,1,SDLK_DELETE);//16
  
  Window_set_repeatable_button(BUTTON_PLUS_X, BUTTON_PLUS_Y,12,11,"+",0,1,SDLK_KP_PLUS);       // 17
  Window_set_repeatable_button(BUTTON_MINUS_X,BUTTON_MINUS_Y,12,11,"-",0,1,SDLK_KP_MINUS);       // 18

  Window_set_normal_button(100,16,35,14,"Neg"    ,1,1,SDLK_n);   // 19
  Window_set_normal_button(7,31,55,14,"Invert"    ,1,1,SDLK_i);   // 20
  Window_set_normal_button(63,31,72,14,"X-Invert" ,5,1,SDLK_v);   // 21

  // Button without outline
  Window_set_normal_button(175,66,81,11,""    ,0,1,SDLK_h);   // 22
  Window_display_frame_mono(175-1,66-1,81+2,11+2,MC_Light);

  sort_dropdown = Window_set_dropdown_button(136, 16, 54, 14, 80, " Sort", 0,
    1, 1, RIGHT_SIDE|LEFT_SIDE, 0); // 23
  Window_dropdown_add_item(sort_dropdown, 0, "Hue/Light");
  Window_dropdown_add_item(sort_dropdown, 1, "Lightness");
  
  Window_set_normal_button(NUMERIC_BOX_X,NUMERIC_BOX_Y,NUMERIC_BOX_W,NUMERIC_BOX_H,"" ,0,1,KEY_NONE);   // 24
  // Button without outline
  Window_display_frame_mono(NUMERIC_BOX_X-1,NUMERIC_BOX_Y-1,NUMERIC_BOX_W+2,NUMERIC_BOX_H+2,MC_Light);

  button_used = Window_set_normal_button(247,16,45,14,"Histo",0,1,KEY_NONE);// 25
  
  // Dessin des petits effets spéciaux pour les boutons [+] et [-]
  Draw_thingumajig(265, 74,MC_White,-1);
  Draw_thingumajig(282, 74,MC_White,+1);
  Draw_thingumajig(265,165,MC_Dark,-1);
  Draw_thingumajig(282,165,MC_Dark,+1);

  Display_cursor();

  Update_color_count(&used_colors,color_usage);
  if (show_used_colors)
    Tag_used_colors(1, color_usage);
    
  Update_window_area(0,0,299,188);
  
  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;
    clicked_button=Window_clicked_button();
    
    switch (clicked_button)
    {
      case  0 : // Nulle part
        break;
      case -1 : // Hors de la fenêtre
      case  1 : // palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          temp_color=(clicked_button==1) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);
          if (Mouse_K==RIGHT_SIDE)
          {
            // Contextual menu
            T_Dropdown_button dropdown;
            T_Dropdown_choice *item;
            
            dropdown.Pos_X         =0;
            dropdown.Pos_Y         =0;
            dropdown.Height        =0;
            dropdown.Dropdown_width=48;
            dropdown.First_item    =NULL;
            dropdown.Bottom_up     =1;
            
            Window_dropdown_add_item(&dropdown, 1, "Copy");
            Window_dropdown_add_item(&dropdown, 2, "Paste");
          
            item=Dropdown_activate(&dropdown,Mouse_X,Mouse_Y);
            
            if (item && item->Number == 1)
            {
              // Copy
              Set_clipboard_colors(block_end+1-block_start,working_palette + block_start);
              Display_cursor();
            }
            else if (item && item->Number == 2)
            {
              // Paste
              int nb_colors;
              
              // Backup
              Palette_edit_step();
              
              nb_colors = Get_clipboard_colors(working_palette, block_start);
              if (nb_colors>0)
              {
                memcpy(temp_palette,working_palette,sizeof(T_Palette));
                Set_palette(working_palette);
                need_to_remap=1;
                Display_cursor();
                Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
              }
              else
              {
                Display_cursor();
              }
            }
            else if (Back_color!=temp_color)
            {
              // Just select back color
              
              Back_color=temp_color;
              // 4 blocks de back_color entourant la fore_color
              Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y-BGCOLOR_DISPLAY_Y,Back_color);
              Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y+FGCOLOR_DISPLAY_H,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_Y+BGCOLOR_DISPLAY_H-FGCOLOR_DISPLAY_Y-FGCOLOR_DISPLAY_H,Back_color);
              Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_X-BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_H,Back_color);
              Window_rectangle(FGCOLOR_DISPLAY_X+FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_X+BGCOLOR_DISPLAY_W-FGCOLOR_DISPLAY_X-FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Back_color);
              Update_window_area(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H);
              
              Display_cursor();
            }
            else
            {
              Display_cursor();
            }
            
            
            Window_dropdown_clear_items(&dropdown);
          }
          else
          {
            if (!old_mouse_k)
            {
              // On vient de clicker sur une couleur (et une seule)
              if ( (Fore_color!=temp_color) || (block_start!=block_end) )
              {
                // La couleur en question est nouvelle ou elle annule un
                // ancien bloc. Il faut donc sélectionner cette couleur comme
                // unique couleur choisie.

                Fore_color=first_color=last_color=block_start=block_end=temp_color;
                Tag_color_range(block_start,block_end);

                // Affichage du n° de la couleur sélectionnée
                Window_rectangle(COLOR_X,COLOR_Y,56,7,MC_Light);
                Num2str(Fore_color,str,3);
                Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
                Update_window_area(COLOR_X,COLOR_Y,56,7);

                // Affichage des jauges
                Window_rectangle(NUMERIC_R_X,NUMERIC_Y,72,7,MC_Light);
                Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

                // Affichage dans le block de visu de la couleur en cours
                Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
                Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);

                Palette_edit_select_range();
              }
            }
            else
            {
              // On maintient le click, on va donc tester si le curseur bouge
              if (temp_color!=last_color)
              {
                // On commence par ordonner la 1ère et dernière couleur du bloc
                if (first_color<temp_color)
                {
                  block_start=first_color;
                  block_end=temp_color;

                  // Affichage du n° de la couleur sélectionnée
                  Num2str(block_start,str  ,3);
                  Num2str(block_end  ,str+4,3);
                  str[3]=26; // Flèche vers la droite
                  Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
                  
                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,1,NULL);

                  // Affichage dans le block de visu du bloc (dégradé) en cours
                  Display_grad_block_in_window(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,block_start,block_end);
                }
                else if (first_color>temp_color)
                {
                  block_start=temp_color;
                  block_end=first_color;

                  // Affichage du n° de la couleur sélectionnée
                  Num2str(block_start,str  ,3);
                  Num2str(block_end  ,str+4,3);
                  str[3]=26; // Flèche vers la droite
                  Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
                  
                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,1,NULL);

                  // Affichage dans le block de visu du bloc (dégradé) en cours
                  Display_grad_block_in_window(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,block_start,block_end);
                }
                else
                {
                  block_start=block_end=first_color;
                  Window_rectangle(NUMERIC_R_X,NUMERIC_Y,72,7,MC_Light);

                  // Affichage du n° de la couleur sélectionnée
                  Window_rectangle(COLOR_X+24,COLOR_Y,32,7,MC_Light);
                  Update_window_area(COLOR_X+24,COLOR_Y,32,7);
                  Num2str(Fore_color,str,3);
                  Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
                  
                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

                  // Affichage dans le block de visu de la couleur en cours
                  Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
                  Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);
                }

                // On tagge le bloc (ou la couleur)
                Tag_color_range(block_start,block_end);
              }

              last_color=temp_color;
            }
            Display_cursor();
          }
          
        }
        break;
      case  2 : // Jauge rouge
        Hide_cursor();
        Palette_edit_alter_channel();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_red(Fore_color,Reduce_component(Color_max-red_slider->Position),working_palette);
            Format_component((working_palette[Fore_color].R)*Color_count/256,str);
          } 
          else
          {
            HSL_to_RGB(
              255-red_slider->Position,
              255-green_slider->Position,
              255-blue_slider->Position,
              &working_palette[Fore_color].R,
              &working_palette[Fore_color].G,
              &working_palette[Fore_color].B);
            Format_component((int)255-red_slider->Position,str);
          }
          Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
            for (i=block_start; i<=block_end; i++)
              Set_red(i,temp_palette[i].R+Reduce_component(Color_max-red_slider->Position),working_palette);
          }
          else
          {
            byte greys=0;
            byte non_greys=0;
            // Check if the range contains both greys and non-greys
            for (i=block_start; i<=block_end; i++)
              if (temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B)
                non_greys=1;
              else
                greys=1;
            
            for (i=block_start; i<=block_end; i++)
            {
              byte is_grey = temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B;
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                is_grey && greys && non_greys ? 0 : Color_max-red_slider->Position,
                is_grey && greys && non_greys ? 0 : Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
            }
          }

          if (red_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-red_slider->Position),str,4);
            str[0]='-';
          }
          else if (red_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-red_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);

        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;
      case  3 : // Jauge verte
        Hide_cursor();
        Palette_edit_alter_channel();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_green (Fore_color,Reduce_component(Color_max-green_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].G*Color_count/256,str);
          } 
          else
          {
            HSL_to_RGB(
              255-red_slider->Position,
              255-green_slider->Position,
              255-blue_slider->Position,
              &working_palette[Fore_color].R,
              &working_palette[Fore_color].G,
              &working_palette[Fore_color].B);
            Format_component((int)255-green_slider->Position,str);
          }
          Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
            for (i=block_start; i<=block_end; i++)
              Set_green (i,temp_palette[i].G+Reduce_component(Color_max-green_slider->Position),working_palette);
          }
          else
          {
            byte greys=0;
            byte non_greys=0;
            // Check if the range contains both greys and non-greys
            for (i=block_start; i<=block_end; i++)
              if (temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B)
                non_greys=1;
              else
                greys=1;
            
            for (i=block_start; i<=block_end; i++)
            {
              byte is_grey = temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B;
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                is_grey && greys && non_greys ? 0 : Color_max-red_slider->Position,
                is_grey && greys && non_greys ? 0 : Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
            }
          }

          if (green_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-green_slider->Position),str,4);
            str[0]='-';
          }
          else if (green_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-green_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case  4 : // Jauge bleue
        Hide_cursor();
        Palette_edit_alter_channel();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_blue (Fore_color,Reduce_component(Color_max-blue_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].B*Color_count/256,str);
          } 
          else
          {
            HSL_to_RGB(
              255-red_slider->Position,
              255-green_slider->Position,
              255-blue_slider->Position,
              &working_palette[Fore_color].R,
              &working_palette[Fore_color].G,
              &working_palette[Fore_color].B);
            Format_component((int)255-blue_slider->Position,str);
          }          
          Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
          for (i=block_start; i<=block_end; i++)
              Set_blue(i,temp_palette[i].B+Reduce_component(Color_max-blue_slider->Position),working_palette);
          }
          else
          {
            byte greys=0;
            byte non_greys=0;
            // Check if the range contains both greys and non-greys
            for (i=block_start; i<=block_end; i++)
              if (temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B)
                non_greys=1;
              else
                greys=1;
            
            for (i=block_start; i<=block_end; i++)
            {
              byte is_grey = temp_palette[i].R==temp_palette[i].G && temp_palette[i].R == temp_palette[i].B;
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                is_grey && greys && non_greys ? 0 : Color_max-red_slider->Position,
                is_grey && greys && non_greys ? 0 : Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
            }
          }

          if (blue_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-blue_slider->Position),str,4);
            str[0]='-';
          }
          else if (blue_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-blue_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 5 : // Merge
        if (block_start!=block_end)
        {
          dword sum_r=0, sum_g=0, sum_b=0, used=0;

          Palette_edit_step();
          // Compute weighted average
          for (i=block_start; i<=block_end; i++)
          {
            used+=color_usage[i];
            sum_r+=working_palette[i].R * color_usage[i];
            sum_g+=working_palette[i].G * color_usage[i];
            sum_b+=working_palette[i].B * color_usage[i];
          }
          // Do normal average if no pixels used
          if (used==0)
          {
            sum_r=sum_g=sum_b=used=0;
            for (i=block_start; i<=block_end; i++)
            {
              used+=1;
              sum_r+=working_palette[i].R;
              sum_g+=working_palette[i].G;
              sum_b+=working_palette[i].B;
            }
          }
          for (i=block_start; i<=block_end; i++)
          {
            Set_red  (i,sum_r/used,working_palette);
            Set_green(i,sum_g/used,working_palette);
            Set_blue (i,sum_b/used,working_palette);
          }
        }
        else
        {
          temp_color=Wait_click_in_palette(Window_palette_button_list);
          if (temp_color>=0)
          {
            dword sum_r=0, sum_g=0, sum_b=0, used;
            Palette_edit_step();
            
            // Compute weighted average
            used=color_usage[temp_color]+color_usage[Fore_color];
            if (used)
            {
              sum_r=(working_palette[temp_color].R * color_usage[temp_color]
                +    working_palette[Fore_color].R * color_usage[Fore_color])
                / used;
              sum_g=(working_palette[temp_color].G * color_usage[temp_color]
                +    working_palette[Fore_color].G * color_usage[Fore_color])
                / used;
              sum_b=(working_palette[temp_color].B * color_usage[temp_color]
                +    working_palette[Fore_color].B * color_usage[Fore_color])
                / used;
            }
            else // Normal average
            {
              sum_r=(working_palette[temp_color].R+working_palette[Fore_color].R)/2;
              sum_g=(working_palette[temp_color].G+working_palette[Fore_color].G)/2;
              sum_b=(working_palette[temp_color].B+working_palette[Fore_color].B)/2;
            }
            Set_red  (temp_color,sum_r,working_palette);
            Set_green(temp_color,sum_g,working_palette);
            Set_blue (temp_color,sum_b,working_palette);
            Set_red  (Fore_color,sum_r,working_palette);
            Set_green(Fore_color,sum_g,working_palette);
            Set_blue (Fore_color,sum_b,working_palette);
            
            Wait_end_of_click();
          }
        }
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        // On prépare la "modifiabilité" des nouvelles couleurs
        Set_palette(working_palette);
        memcpy(temp_palette,working_palette,sizeof(T_Palette));
        need_to_remap=1;

        break;

      case 6 : // Grey scale
        // Backup
        Palette_edit_step();
        // Grey Scale
        for (i=block_start;i<=block_end;i++)
        {
          temp_color=(dword)( ((dword)working_palette[i].R*30) + ((dword)working_palette[i].G*59) + ((dword)working_palette[i].B*11) )/100;
          Set_red(i,temp_color,working_palette);
          Set_green (i,temp_color,working_palette);
          Set_blue (i,temp_color,working_palette);
        }
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        // On prépare la "modifiabilité" des nouvelles couleurs
        Set_palette(working_palette);
        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case  7 : // Swap
      case  8 : // X-Swap
        temp_color=Wait_click_in_palette(Window_palette_button_list);
        if ((temp_color>=0)
         && (temp_color!=block_start))
        {
          Hide_cursor();
          Palette_edit_step();

          // On calcule le nombre de couleurs a swapper sans risquer de sortir
          // de la palette (La var. first_color est utilisée pour économiser 1 var; c'est tout)
          first_color=(temp_color+block_end-block_start<=255)?block_end+1-block_start:256-temp_color;

          if (clicked_button==8) // On ne fait de backup de l'image que si on
                                // est en mode X-SWAP.
            if (!image_is_backed_up)
            {
              Backup_layers(-1);
              image_is_backed_up=1;
            }

          Swap(clicked_button==8,block_start,temp_color,first_color,working_palette,color_usage);

          memcpy(temp_palette,working_palette,sizeof(T_Palette));

          // On déplace le bloc vers les modifs:
          last_color=block_end=temp_color+first_color-1;
          Fore_color=first_color=block_start=temp_color;
          // On raffiche le n° des bornes du bloc:
          if (block_start!=block_end)
          {
            // Cas d'un bloc multi-couleur
            Num2str(block_start,str  ,3);
            Num2str(block_end  ,str+4,3);
            str[3]=26; // Flèche vers la droite
            // Affichage dans le block de visu du bloc (dégradé) en cours
            Display_grad_block_in_window(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,block_start,block_end);
          }
          else
          {
            // Cas d'une seule couleur
            Num2str(Fore_color,str,3);
            Window_rectangle(COLOR_X,COLOR_Y,56,7,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
          }
          Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
          // On tag le bloc (ou la couleur)
          Tag_color_range(block_start,block_end);
          if (show_used_colors)
            Tag_used_colors(1, color_usage);
          
          need_to_remap=1;

          Set_palette(working_palette);

          Display_cursor();
          Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
          
          // En cas de X-Swap, tout l'ecran a pu changer de couleur.
          if (clicked_button==8)
          {
            Palette_edit_step(); // Disable Undo
            Update_rect(0, 0, Screen_width, Menu_Y_before_window);
            End_of_modification();
          }
          Wait_end_of_click();
        }
        break;

      case  9 : // Copy (to other slot)
        temp_color=Wait_click_in_palette(Window_palette_button_list);
        if ((temp_color>=0) && (temp_color!=block_start))
        {
          Hide_cursor();
          Palette_edit_step();
          memcpy(working_palette+temp_color,backup_palette+block_start,
                 ((temp_color+block_end-block_start<=255)?block_end+1-block_start:256-temp_color)*3);
          memcpy(temp_palette,working_palette,sizeof(T_Palette));
          Set_palette(working_palette);
          // On déplace le bloc vers les modifs:
          last_color=block_end=((temp_color+block_end-block_start<=255)?(temp_color+block_end-block_start):255);
          Fore_color=first_color=block_start=temp_color;
          // On raffiche le n° des bornes du bloc:
          if (block_start!=block_end)
          {
            // Cas d'un bloc multi-couleur
            Num2str(block_start,str  ,3);
            Num2str(block_end  ,str+4,3);
            str[3]=26; // Flèche vers la droite
            // Affichage dans le block de visu du bloc (dégradé) en cours
            Display_grad_block_in_window(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,block_start,block_end);
          }
          else
          {
            // Cas d'une seule couleur
            Num2str(Fore_color,str,3);
            Window_rectangle(COLOR_X,COLOR_Y,56,7,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
          }
          Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
          // On tag le bloc (ou la couleur)
          Tag_color_range(block_start,block_end);
          
          need_to_remap=1;

          Display_cursor();
          Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
          Wait_end_of_click();
        }
        break;

      case 10 : // Spread
        if (block_start!=block_end)
        {
          Palette_edit_step();
          Spread_colors(block_start,block_end,working_palette);
        }
        else
        {
          temp_color=Wait_click_in_palette(Window_palette_button_list);
          if (temp_color>=0)
          {
            Palette_edit_step();
            if (temp_color<Fore_color)
              Spread_colors(temp_color,Fore_color,working_palette);
            else
              Spread_colors(Fore_color,temp_color,working_palette);
            Wait_end_of_click();
          }
        }

        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        // On prépare la "modifiabilité" des nouvelles couleurs

        Set_palette(working_palette);

        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case 11: // Reduce
        Palette_edit_step();
        if (Window_attribute2==0) // User picked "other" choice
        {
          int choice;
          
          choice=Requester_window("Enter the max. number of colors",
                        reduce_colors_number);
          
          if (choice < 2 || choice > 256)
            break; // Cancel
            
          reduce_colors_number = choice;
        }
        else
          // Each other dropdown item has a number of colors as id.
          reduce_colors_number = Window_attribute2;

        if (reduce_colors_number >= 2)
        {
            if (!image_is_backed_up)
            {
                Backup_layers(-1);
                image_is_backed_up = 1;
            }

            Reduce_palette(&used_colors, reduce_colors_number, working_palette,
                    color_usage);

            if ((Config.Safety_colors) && (used_colors<4))
            {
                memcpy(temp_palette, Main_palette, sizeof(T_Palette));
                memcpy(Main_palette, working_palette, sizeof(T_Palette));
                Set_nice_menu_colors(color_usage, 0);
                memcpy(working_palette, Main_palette, sizeof(T_Palette));
                memcpy(Main_palette, temp_palette, sizeof(T_Palette));
            }

            Set_palette(working_palette); // On définit la nouvelle palette
            Draw_all_palette_sliders(red_slider, green_slider, blue_slider,
                    working_palette, block_start, block_end);
            memcpy(temp_palette, working_palette, sizeof(T_Palette));
            Palette_edit_step(); // Disable Undo
            End_of_modification();
            need_to_remap = 1;
        }
        break;

      case 12: // Undo
        Palette_edit_undo_redo();
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        Set_palette(working_palette);

        need_to_remap=1;
        break;

      case 15 : // Used : show usage tags
        show_used_colors = !show_used_colors;
        Tag_used_colors(show_used_colors, color_usage);
        break;

      case 16 : // Zap unused
        Palette_edit_step();
        if (used_colors==-1)
          Update_color_count(&used_colors,color_usage);
        for (i=0; i<256; i++)
        {
          if (!color_usage[i])
          {
            temp_color=block_start+(i % (block_end+1-block_start));
            working_palette[i].R=backup_palette[temp_color].R;
            working_palette[i].G=backup_palette[temp_color].G;
            working_palette[i].B=backup_palette[temp_color].B;
          }
        }

        if ((Config.Safety_colors) && (used_colors<4) && (block_end==block_start))
        {
          memcpy(temp_palette,Main_palette,sizeof(T_Palette));
          memcpy(Main_palette,working_palette,sizeof(T_Palette));
          Set_nice_menu_colors(color_usage,0);
          memcpy(working_palette,Main_palette,sizeof(T_Palette));
          memcpy(Main_palette,temp_palette,sizeof(T_Palette));
        }

        Set_palette(working_palette);
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);

        need_to_remap=1;
        break;

      case 17 : // [+]
       if (!Palette_view_is_RGB)
          break;
        Hide_cursor();
        Palette_edit_alter_channel();
        if (block_start==block_end)
        {
          if (red_slider->Position)
          {
            (red_slider->Position)--;
            Window_draw_slider(red_slider);
            Set_red(Fore_color,Reduce_component(Color_max-red_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].R*Color_count/256,str);
            Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
          if (green_slider->Position)
          {
            (green_slider->Position)--;
            Window_draw_slider(green_slider);
            Set_green (Fore_color,Reduce_component(Color_max-green_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].G*Color_count/256,str);
            Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
          if (blue_slider->Position)
          {
            (blue_slider->Position)--;
            Window_draw_slider(blue_slider);
            Set_blue (Fore_color,Reduce_component(Color_max-blue_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].B*Color_count/256,str);
            Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
        }
        else
        {
          if (red_slider->Position)
          {
            (red_slider->Position)--;
            Window_draw_slider(red_slider);
          }
          if (green_slider->Position)
          {
            (green_slider->Position)--;
            Window_draw_slider(green_slider);
          }
          if (blue_slider->Position)
          {
            (blue_slider->Position)--;
            Window_draw_slider(blue_slider);
          }

          for (i=block_start; i<=block_end; i++)
          {
            Set_red(i,temp_palette[i].R+Reduce_component(Color_max-red_slider->Position),working_palette);
            Set_green (i,temp_palette[i].G+Reduce_component(Color_max-green_slider->Position),working_palette);
            Set_blue (i,temp_palette[i].B+Reduce_component(Color_max-blue_slider->Position),working_palette);
          }

          // -- red --
          if (red_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-red_slider->Position),str,4);
            str[0]='-';
          }
          else if (red_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-red_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);


          // -- green --
          if (green_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-green_slider->Position),str,4);
            str[0]='-';
          }
          else if (green_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-green_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);


          // -- blue --
          if (blue_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-blue_slider->Position),str,4);
            str[0]='-';
          }
          else if (blue_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-blue_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 18 : // [-]
        if (!Palette_view_is_RGB)
          break;
        Hide_cursor();
        Palette_edit_alter_channel();
        if (block_start==block_end)
        {
          if (red_slider->Position<Color_max)
          {
            (red_slider->Position)++;
            Window_draw_slider(red_slider);
            Set_red(Fore_color,Reduce_component(Color_max-red_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].R*Color_count/256,str);
            Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
          if (green_slider->Position<Color_max)
          {
            (green_slider->Position)++;
            Window_draw_slider(green_slider);
            Set_green (Fore_color,Reduce_component(Color_max-green_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].G*Color_count/256,str);
            Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
          if (blue_slider->Position<Color_max)
          {
            (blue_slider->Position)++;
            Window_draw_slider(blue_slider);
            Set_blue (Fore_color,Reduce_component(Color_max-blue_slider->Position),working_palette);
            Format_component(working_palette[Fore_color].B*Color_count/256,str);
            Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
          }
        }
        else
        {
          if (red_slider->Position<(Color_max*2))
          {
            (red_slider->Position)++;
            Window_draw_slider(red_slider);
          }
          if (green_slider->Position<(Color_max*2))
          {
            (green_slider->Position)++;
            Window_draw_slider(green_slider);
          }
          if (blue_slider->Position<(Color_max*2))
          {
            (blue_slider->Position)++;
            Window_draw_slider(blue_slider);
          }

          for (i=block_start; i<=block_end; i++)
          {
            Set_red(i,temp_palette[i].R+Reduce_component(Color_max-red_slider->Position),working_palette);
            Set_green (i,temp_palette[i].G+Reduce_component(Color_max-green_slider->Position),working_palette);
            Set_blue (i,temp_palette[i].B+Reduce_component(Color_max-blue_slider->Position),working_palette);
          }

          // -- red --
          if (red_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-red_slider->Position),str,4);
            str[0]='-';
          }
          else if (red_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-red_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_R_X,NUMERIC_Y,str,MC_Black,MC_Light);


          // -- green --
          if (green_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-green_slider->Position),str,4);
            str[0]='-';
          }
          else if (green_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-green_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_G_X,NUMERIC_Y,str,MC_Black,MC_Light);


          // -- blue --
          if (blue_slider->Position>Color_max)
          {
            // Jauge dans les négatifs:
            Num2str(-(Color_max-blue_slider->Position),str,4);
            str[0]='-';
          }
          else if (blue_slider->Position<Color_max)
          {
            // Jauge dans les positifs:
            Num2str(  Color_max-blue_slider->Position ,str,4);
            str[0]='+';
          }
          else
          {
            // Jauge nulle:
            strcpy(str,"±  0");
          }
          Print_counter(NUMERIC_B_X,NUMERIC_Y,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 19 : // Negative
        // Backup
        Palette_edit_step();
        // Negative
        for (i=block_start;i<=block_end;i++)
        {
          Set_red(i,255-working_palette[i].R,working_palette);
          Set_green (i,255-working_palette[i].G,working_palette);
          Set_blue (i,255-working_palette[i].B,working_palette);
        }
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        Set_palette(working_palette);
        // On prépare la "modifiabilité" des nouvelles couleurs
        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case 20 : // Inversion
      case 21 : // X-Inversion
        // Backup
        Palette_edit_step(); // Not undoable if X-Invert
        //   On initialise la table de conversion
        for (i=0; i<=255; i++)
          conversion_table[i]=i;
        // Inversion
        for (i=block_start; i < block_start + (block_end-block_start+1)/2;i++)
        {
          temp_color=block_end-(i-block_start);
          
          Set_red   (i,backup_palette[temp_color].R,working_palette);
          Set_green (i,backup_palette[temp_color].G,working_palette);
          Set_blue  (i,backup_palette[temp_color].B,working_palette);
          Set_red   (temp_color,backup_palette[i].R,working_palette);
          Set_green (temp_color,backup_palette[i].G,working_palette);
          Set_blue  (temp_color,backup_palette[i].B,working_palette);
          
          if (clicked_button==21)
          {
            conversion_table[i]=temp_color;
            conversion_table[temp_color]=i;

            temp=color_usage[i];
            color_usage[i]=color_usage[temp_color];
            color_usage[temp_color]=temp;
          }
        }
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        // Si on est en X-Invert, on remap l'image (=> on fait aussi 1 backup)
        if (clicked_button==21)
        {
          if (!image_is_backed_up)
          {
            Backup_layers(-1);
            image_is_backed_up=1;
          }
          Hide_cursor();
          Remap_image_highlevel(conversion_table);
          Display_cursor();
          End_of_modification();
          Palette_edit_step(); // Disable Undo
        }
        // On prépare la "modifiabilité" des nouvelles couleurs
        Set_palette(working_palette);
        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case 22 : // HSL <> RGB
        
        // Acte les changements en cours sur une ou plusieurs couleurs
        Palette_edit_select_range();

        Hide_cursor();
        
        Palette_view_is_RGB = !Palette_view_is_RGB;
        if(! Palette_view_is_RGB)
        {
          // On passe en HSL
          Print_RGB_or_HSL(1);
          Component_unit(256);

          // Display the + and - button as disabled
          Window_draw_normal_bouton(BUTTON_PLUS_X, BUTTON_PLUS_Y,12,11,"+",0,0);
          Window_draw_normal_bouton(BUTTON_MINUS_X,BUTTON_MINUS_Y,12,11,"-",0,0);
        }
        else
        {
          // On passe en RGB
          Print_RGB_or_HSL(0);
          Component_unit(RGB_scale);

          // Display the + and - button as enabled
          Window_draw_normal_bouton(BUTTON_PLUS_X, BUTTON_PLUS_Y,12,11,"+",0,1);
          Window_draw_normal_bouton(BUTTON_MINUS_X,BUTTON_MINUS_Y,12,11,"-",0,1);
        }
        Display_sliders(red_slider,green_slider,blue_slider,(block_start!=block_end),working_palette);
        
        Display_cursor();
        
        Update_window_area(BUTTON_PLUS_X-1,BUTTON_PLUS_Y-1,14,14);
        Update_window_area(BUTTON_MINUS_X-1,BUTTON_MINUS_Y-1,14,14);
      break;

      case 23 : // Sort palette
      {
        byte h = 0, l = 0, s=0;
        byte oh=0,ol=0,os=0; // Valeur pour la couleur précédente
        int swap=1;
        byte remap_table[256];
        byte inverted_table[256];
        byte begin, end;
        long lightness;
        long old_lightness;


        if(block_start==block_end)
        {
            begin = 0;
            end = 255;
        }
        else
        {
            begin = block_start;
            end = block_end;
        }
        
        // Init remap table
        for (i=0;i<256;i++)
          remap_table[i]=i;
        // Make a backup because remapping is an undoable modification
        if (!image_is_backed_up)
        {
          Backup_layers(-1);
          image_is_backed_up=1;
        }

        if(Window_attribute2==0)
        // Sort by Hue (H) and Lightness (L)
        while(swap==1)
        {
          swap=0;
          h=0;l=255;s=0;
          for(temp_color=begin;temp_color<=end;temp_color++)
          {
            oh=h; ol=l; os=s;
            RGB_to_HSL(working_palette[temp_color].R,
            working_palette[temp_color].G,
            working_palette[temp_color].B,&h,&s,&l);

            if(
                   ((s==0) && (os>0)) // A grey is before a saturated color
              || ((s>0 && os > 0) && (h<oh || (h==oh && l>ol))) // 2 saturated colors: sort by H, then by L
              || ((os==0 && s==0) && l>ol))  // Two greys: sort by L only
            {
              // Swap color with the previous one
              SWAP_BYTES(working_palette[temp_color].R, working_palette[temp_color-1].R)
              SWAP_BYTES(working_palette[temp_color].G, working_palette[temp_color-1].G)
              SWAP_BYTES(working_palette[temp_color].B, working_palette[temp_color-1].B)

              SWAP_DWORDS(color_usage[temp_color], color_usage[temp_color-1])

              SWAP_BYTES(remap_table[temp_color], remap_table[temp_color-1])
              
              swap=1;
            }
          }
        }

        else // Sort only on perceived lightness
        while(swap==1)
        {
          swap=0;
          lightness=Perceptual_lightness(working_palette+begin);
          for(temp_color=begin+1;temp_color<=end;temp_color++)
          {
            old_lightness=lightness; 
            lightness=Perceptual_lightness(working_palette+temp_color);

            if(lightness>old_lightness)
            {
              // Swap color with the previous one
              SWAP_BYTES(working_palette[temp_color].R, working_palette[temp_color-1].R)
              SWAP_BYTES(working_palette[temp_color].G, working_palette[temp_color-1].G)
              SWAP_BYTES(working_palette[temp_color].B, working_palette[temp_color-1].B)
      
              SWAP_DWORDS(color_usage[temp_color], color_usage[temp_color-1])
              
              SWAP_BYTES(remap_table[temp_color], remap_table[temp_color-1])
              
              swap=1;
            }
          }
        }

        for (i=0;i<256;i++)
          inverted_table[remap_table[i]]=i;
        Remap_image_highlevel(inverted_table);
        // Maintenant, tous ces calculs doivent êtres pris en compte dans la
        // palette, l'image et à l'écran.
        Set_palette(working_palette);
        Palette_edit_step(); // Disable Undo
        
        End_of_modification();
        need_to_remap=1;
      }
      break;
      case 24: // R G B value: Hex entry
      {
        char str[7];
        unsigned int new_color;
        
        Hide_cursor();
        Print_in_window(NUMERIC_BOX_X+2,NUMERIC_BOX_Y+2,"Hex",MC_Black,MC_Light);
        // Clear out remaining area
        Window_rectangle(NUMERIC_BOX_X+1+3*8,NUMERIC_BOX_Y+1,NUMERIC_BOX_W-3-3*8, NUMERIC_BOX_H-3,MC_Light);
        Update_window_area(NUMERIC_BOX_X+1+3*8,NUMERIC_BOX_Y+1,NUMERIC_BOX_W-3-3*8, NUMERIC_BOX_H-3);
        
        str[0]='\0';
        Display_cursor();
        if (Readline(NUMERIC_BOX_X+NUMERIC_BOX_W-2-6*8, NUMERIC_BOX_Y+2, str, 6, INPUT_TYPE_HEXA))
        {
          int length = strlen(str);
          short new_red, new_blue, new_green;
          
          if (length==3 || length==6)
          {
            sscanf(str, "%x", &new_color);
            if (length==3)
            {
              new_color =
                ((new_color&0xF00)*0x1100) |
                ((new_color&0x0F0)*0x110) |
                ((new_color&0x00F)*0x11); 
            }
            new_red=(new_color&0xFF0000) >> 16;
            new_green=(new_color&0x00FF00) >> 8;
            new_blue=(new_color&0x0000FF);
            
            // Backup
            Palette_edit_step();
            // Assign color
            for (i=block_start;i<=block_end;i++)
            {
              Set_red(i,new_red,working_palette);
              Set_green (i,new_green,working_palette);
              Set_blue (i,new_blue,working_palette);
            }
            // On prépare la "modifiabilité" des nouvelles couleurs
            Set_palette(working_palette);
            memcpy(temp_palette,working_palette,sizeof(T_Palette));
            need_to_remap=1;
          }
        }
        // Clear out numeric area
        Window_rectangle(NUMERIC_BOX_X+1,NUMERIC_BOX_Y+1,NUMERIC_BOX_W-2, NUMERIC_BOX_H-2,MC_Light);
        Update_window_area(NUMERIC_BOX_X+1,NUMERIC_BOX_Y+1,NUMERIC_BOX_W-2, NUMERIC_BOX_H-2);
        Display_cursor();
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
      }
      break;
      
      case 25: // Number of colors used: Open histogram
      {
        int selected_col;
        
        selected_col=Window_Histogram(block_start, block_end, color_usage);
        if (selected_col!=-1)
        {
          // Tag selected color
          Fore_color=first_color=last_color=block_start=block_end=selected_col;
          Tag_color_range(block_start,block_end);
      
          // Affichage du n° de la couleur sélectionnée
          Window_rectangle(COLOR_X,COLOR_Y,56,7,MC_Light);
          Num2str(Fore_color,str,3);
          Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
          Update_window_area(COLOR_X,COLOR_Y,56,7);
      
          // Affichage des jauges
          Window_rectangle(NUMERIC_R_X,NUMERIC_Y,72,7,MC_Light);
          Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);
      
          // Affichage dans le block de visu de la couleur en cours
          Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
          Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);
      
          Palette_edit_select_range();

        }
        Display_cursor();
        Input_sticky_control=0;
        Wait_end_of_click();
        break;
      }
    }


    if (!Mouse_K)
    {
      if (Key)
      {
        if (Is_shortcut(Key,SPECIAL_PREVIOUS_FORECOLOR)) // Décaler Forecolor vers la gauche
        {
          if (block_start==block_end)
          {
            Fore_color--;
            first_color--;
            last_color--;
            block_start--;
            block_end--;
            Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
            Hide_cursor();
            Tag_color_range(block_start,block_end);
            // Affichage du n° de la couleur sélectionnée
            Num2str(Fore_color,str,3);
            Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
            Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);
            Display_cursor();
          }
          Key=0;
        }
        else if (Is_shortcut(Key,SPECIAL_NEXT_FORECOLOR)) // Décaler Forecolor vers la droite
        {
          if (block_start==block_end)
          {
            Fore_color++;
            first_color++;
            last_color++;
            block_start++;
            block_end++;
            Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
            Hide_cursor();
            Tag_color_range(block_start,block_end);
            // Affichage du n° de la couleur sélectionnée
            Num2str(Fore_color,str,3);
            Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
            Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);
            Display_cursor();
          }
          Key=0;
        }
        else if (Is_shortcut(Key,SPECIAL_PREVIOUS_BACKCOLOR))
        {
          Back_color--;
          Hide_cursor();
          Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y-BGCOLOR_DISPLAY_Y,Back_color);
          Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y+FGCOLOR_DISPLAY_H,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_Y+BGCOLOR_DISPLAY_H-FGCOLOR_DISPLAY_Y-FGCOLOR_DISPLAY_H,Back_color);
          Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_X-BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_H,Back_color);
          Window_rectangle(FGCOLOR_DISPLAY_X+FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_X+BGCOLOR_DISPLAY_W-FGCOLOR_DISPLAY_X-FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Back_color);
          Update_window_area(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H);
          Display_cursor();
          Key=0;
        }
        else if (Is_shortcut(Key,SPECIAL_NEXT_BACKCOLOR))
        {
          Back_color++;
          Hide_cursor();
          Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y-BGCOLOR_DISPLAY_Y,Back_color);
          Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y+FGCOLOR_DISPLAY_H,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_Y+BGCOLOR_DISPLAY_H-FGCOLOR_DISPLAY_Y-FGCOLOR_DISPLAY_H,Back_color);
          Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_X-BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_H,Back_color);
          Window_rectangle(FGCOLOR_DISPLAY_X+FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_X+BGCOLOR_DISPLAY_W-FGCOLOR_DISPLAY_X-FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Back_color);
          Update_window_area(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H);
          Display_cursor();
          Key=0;
        }
        else if (Key == SDLK_BACKSPACE)
        // Remise des couleurs du menu à l'état normal en essayant
        // de ne pas trop modifier l'image.
        {
          if (!image_is_backed_up)
          {
            Backup_layers(-1);
            image_is_backed_up=1;
          }
          if (used_colors==-1)
            Update_color_count(&used_colors, color_usage);

          Palette_edit_step();
          memcpy(temp_palette,Main_palette,sizeof(T_Palette));
          memcpy(Main_palette,working_palette,sizeof(T_Palette));
          Set_nice_menu_colors(color_usage,0);
          memcpy(working_palette,Main_palette,sizeof(T_Palette));
          memcpy(Main_palette,temp_palette,sizeof(T_Palette));
          Set_palette(working_palette);
          memcpy(temp_palette,working_palette,sizeof(T_Palette));
          Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
          Update_color_count(&used_colors,color_usage);
          // End_of_modification();
          // Not really needed, the change was in palette entries
          need_to_remap=1;
          Key=0;
        }
        else if (Is_shortcut(Key,0x100+BUTTON_COLORPICKER))
        {
          // Récupération d'une couleur derrière le menu
          Get_color_behind_window(&color,&click);
          if (click)
          {
            Hide_cursor();
            if (click==RIGHT_SIDE)
            {
              if (Back_color!=color)
              {
                Back_color=color;
                // 4 blocks de back_color entourant la fore_color
                Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y-BGCOLOR_DISPLAY_Y,Back_color);
                Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y+FGCOLOR_DISPLAY_H,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_Y+BGCOLOR_DISPLAY_H-FGCOLOR_DISPLAY_Y-FGCOLOR_DISPLAY_H,Back_color);
                Window_rectangle(BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_X-BGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_H,Back_color);
                Window_rectangle(FGCOLOR_DISPLAY_X+FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_X+BGCOLOR_DISPLAY_W-FGCOLOR_DISPLAY_X-FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Back_color);
                Update_window_area(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H);
              }
            }
            else
            {
              Fore_color=first_color=last_color=block_start=block_end=color;
              Tag_color_range(block_start,block_end);

              // Affichage du n° de la couleur sélectionnée
              Window_rectangle(COLOR_X+24,COLOR_Y,32,7,MC_Light);
              Update_window_area(COLOR_X+24,COLOR_Y,32,7);
              Num2str(Fore_color,str,3);
              Print_in_window(COLOR_X,COLOR_Y,str,MC_Black,MC_Light);
              
              // Affichage des jauges
              Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

              // Affichage dans le block de visu de la couleur en cours
              Window_rectangle(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H,Fore_color);
              Update_window_area(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,FGCOLOR_DISPLAY_W,FGCOLOR_DISPLAY_H);
              
              Palette_edit_select_range();
            }
            Display_cursor();
            Wait_end_of_click();
          }
          Key=0;
        }
        else if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Key=0;
          Window_help(BUTTON_PALETTE, NULL);
        }
        else if (Is_shortcut(Key,0x100+BUTTON_PALETTE))
        {
          // Close (confirm)
          clicked_button=14;
        }
        else if (Key == SHORTCUT_COPY)
        {
          Set_clipboard_colors(block_end+1-block_start,working_palette + block_start);
        }
        else if (Key == SHORTCUT_PASTE)
        {
          int nb_colors;

          Hide_cursor();
          // Backup
          Palette_edit_step();
          
          nb_colors = Get_clipboard_colors(working_palette, block_start);
          if (nb_colors>0)
          {
            memcpy(temp_palette,working_palette,sizeof(T_Palette));
            Set_palette(working_palette);
            need_to_remap=1;
            Display_cursor();
            Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
          }
        }
      }

      if (need_to_remap)
      {
        Hide_cursor();
        Compute_optimal_menu_colors(working_palette);

        // On remappe brutalement
        Remap_screen_after_menu_colors_change();
        // Puis on remet les trucs qui ne devaient pas changer
        Window_draw_palette_bouton(5,79);
        if (show_used_colors)
          Tag_used_colors(1, color_usage);
        Window_rectangle(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H,Back_color);
        Update_window_area(BGCOLOR_DISPLAY_X,BGCOLOR_DISPLAY_Y,BGCOLOR_DISPLAY_W,BGCOLOR_DISPLAY_H);
        
        Display_grad_block_in_window(FGCOLOR_DISPLAY_X,FGCOLOR_DISPLAY_Y,block_start,block_end);

        Update_window_area(8,82,16*10,5*16);

        Display_cursor();
        need_to_remap=0;
      }
    }
  }
  while ((clicked_button!=13) && (clicked_button!=14));

  if (clicked_button==14)         // Sortie par OK
  {
    if ( (!image_is_backed_up)
      && memcmp(Main_palette,working_palette,sizeof(T_Palette)) )
      Backup_layers(-1);
    memcpy(Main_palette,working_palette,sizeof(T_Palette));
    End_of_modification();
    // Not really needed, the change was in palette entries
  }

  Compute_optimal_menu_colors(Main_palette);

  // La variable employée ici n'a pas vraiment de rapport avec son nom...
  need_to_remap=(Window_pos_Y+(Window_height*Menu_factor_Y)<Menu_Y_before_window);

  Close_window();
  Unselect_button(BUTTON_PALETTE);

  Reposition_palette();

  //   On affiche les "ForeBack" car le menu n'est raffiché que si la fenêtre
  // empiétait sur le menu. Mais si les couleurs on été modifiées, il faut
  // rafficher tout le menu remappé.
  if (need_to_remap)
    Display_menu();

  Display_cursor();

  if (clicked_button==13)                         // Sortie par CANCEL
  {
    Set_palette(Main_palette);
    if (image_is_backed_up)
      Select_button(BUTTON_UNDO,LEFT_SIDE);
  }

  free(backup_palette);
  free(temp_palette);
  free(working_palette);
  backup_palette = temp_palette = working_palette = NULL;
}


//---------------------- Menu de palettes secondaires ------------------------

void Button_Secondary_palette(void)
{
  short clicked_button;
  byte dummy;
  T_Scroller_button * columns_slider;
  T_Scroller_button * lines_slider;
  T_Scroller_button * rgb_scale_slider;
  char str[4];
  byte palette_vertical = Config.Palette_vertical;
  byte palette_cols, palette_lines;
  word rgb_scale;
  byte palette_needs_redraw=0;
  
  Open_window(200,146,"Palettes");

  Window_set_normal_button(10,20,180,14,"Colors for best match",12,1,SDLK_b); // 1
  Window_set_normal_button(10,37,180,14,"User's color series"  ,14,1,SDLK_s); // 2
  Window_set_normal_button(139,126,53,14,"OK"                  , 0,1,SDLK_RETURN); // 3
  Window_set_normal_button( 80,126,53,14,"Cancel"              , 0,1,KEY_ESC); // 4
  Window_display_frame(10,55,122,66);
  Print_in_window(18,59,"Palette layout",MC_Dark,MC_Light);
  Print_in_window(35,77,"Cols",MC_Dark,MC_Light);
  Print_in_window(84,77,"Lines",MC_Dark,MC_Light);
  Print_in_window(157,58,"RGB",MC_Dark,MC_Light);
  Print_in_window(152,68,"Scale",MC_Dark,MC_Light);
  
  columns_slider = Window_set_scroller_button(19,72,29,255,1,256-Config.Palette_cells_X);// 5
  Num2str(Config.Palette_cells_X,str,3);
  Print_in_window(38,89,str,MC_Black,MC_Light);
  
  lines_slider = Window_set_scroller_button(70,72,29,15,1,16-Config.Palette_cells_Y);// 6
  Num2str(Config.Palette_cells_Y,str,3);
  Print_in_window(94,89,str,MC_Black,MC_Light);
  
  rgb_scale_slider = Window_set_scroller_button(137,58,60,255,1,256-RGB_scale);// 7
  Num2str(RGB_scale,str,3);
  Print_in_window(157,78,str,MC_Black,MC_Light);

  Window_set_normal_button(35,106,13,11,"",-1,1,SDLK_LAST); // 8
  Print_in_window(38,108,(palette_vertical)?"X":" ",MC_Black,MC_Light);
  Print_in_window(51,108,"Vertical",MC_Dark,MC_Light);

  Window_set_normal_button(152,88,18,14,"x2"                  , 1,1,SDLK_x); // 9
  Window_set_normal_button(172,88,18,14,"÷2"                  , 0,1,SDLK_w); // 10
  
  Update_window_area(0,0,200,146);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      Key=0;
      Window_help(BUTTON_PALETTE, "PALETTE OPTIONS");
    }
    else if (Is_shortcut(Key,0x200+BUTTON_PALETTE))
      clicked_button=3;

    switch(clicked_button)
    {
      case 5:
        // Column slider
        Num2str(256-Window_attribute2,str,3);
        Print_in_window(38,89,str,MC_Black,MC_Light);
        break;    
      case 6:
        // Line slider
        Num2str(16-Window_attribute2,str,3);
        Print_in_window(94,89,str,MC_Black,MC_Light);
        break;
      case 7:
        // RGB scale slider
        Num2str(256-Window_attribute2,str,3);
        Hide_cursor();
        Print_in_window(157,78,str,MC_Black,MC_Light);
        Display_cursor();
        break;
      case 8:
        // Vertical switch
        palette_vertical = !palette_vertical;
        Hide_cursor();
        Print_in_window(38,108,(palette_vertical)?"X":" ",MC_Black,MC_Light);
        Display_cursor();
        break;

      case 9:
        // x2 RGB scale
        rgb_scale_slider->Position = rgb_scale_slider->Position>128?rgb_scale_slider->Position*2-256:0;
        Num2str(256-rgb_scale_slider->Position,str,3);
        Print_in_window(157,78,str,MC_Black,MC_Light);
        Window_draw_slider(rgb_scale_slider);
        break;

      case 10:
        // /2 RGB scale
        rgb_scale_slider->Position = rgb_scale_slider->Position>253?254:(rgb_scale_slider->Position)/2+128;
        Num2str(256-rgb_scale_slider->Position,str,3);
        Print_in_window(157,78,str,MC_Black,MC_Light);
        Window_draw_slider(rgb_scale_slider);
    }
  }
  while (clicked_button!=1 && clicked_button!=2 && clicked_button!=3 && clicked_button!=4);

  // We need to get the sliders positions before closing the window, because they will be freed.
  palette_cols=256-columns_slider->Position;
  palette_lines=16-lines_slider->Position;
  rgb_scale=256-rgb_scale_slider->Position;

  Close_window();
  Unselect_button(BUTTON_PALETTE);
  Display_cursor();
  
  if (clicked_button==4) // Cancel
    return;

  if (palette_vertical != Config.Palette_vertical)
  {
    Config.Palette_vertical=palette_vertical;
    palette_needs_redraw=1;
  }
  if (palette_cols!=Config.Palette_cells_X ||
    palette_lines!=Config.Palette_cells_Y)
  {
    Config.Palette_cells_X = palette_cols;
    Config.Palette_cells_Y = palette_lines;
    palette_needs_redraw=1;
  }
  if (rgb_scale!=RGB_scale)
  {
    Set_palette_RGB_scale(rgb_scale);
    Set_palette(Main_palette);
    Compute_optimal_menu_colors(Main_palette);
  }

  if (clicked_button==1)
  {
    Menu_tag_colors("Tag colors to exclude",Exclude_color,&dummy,1, NULL, SPECIAL_EXCLUDE_COLORS_MENU);
  }
  else if (clicked_button==2)
  {
    // Open the menu with Shade settings. Same as the shortcut, except
    // that this will not activate shade mode on exit.
    Shade_settings_menu();
  }
  
  if (palette_needs_redraw)
  {
    Change_palette_cells();
    Display_menu();
    Display_sprite_in_menu(BUTTON_PAL_LEFT,Config.Palette_vertical?MENU_SPRITE_VERTICAL_PALETTE_SCROLL:-1);
    Draw_menu_button(BUTTON_PAL_LEFT,BUTTON_RELEASED);
    Draw_menu_button(BUTTON_PAL_RIGHT,BUTTON_RELEASED);
  }
}

// ========= Clipboard management ==============

int Palette_clipboard_count=0;
T_Palette Palette_clipboard;

/// Put some colors in the clipboard.
/// @param nb_colors Number of colors to push
/// @param colors First color of the input array
void Set_clipboard_colors(int nb_colors, T_Components *colors)
{
  Palette_clipboard_count=nb_colors;
  if (nb_colors)
  {
    memcpy(Palette_clipboard, colors, nb_colors*sizeof(T_Components));
  }
}

/// Get some RGB colors from clipboard.
/// @param palette Target palette
/// @param start_color  Index of first color to replace
/// @return        Number of colors retrieved (0-256)
int Get_clipboard_colors(T_Palette palette, byte start_color)
{
  int nb_colors = Palette_clipboard_count;
  
  if (nb_colors==0)
    return 0;

  if (start_color+nb_colors > 256)
  {
    nb_colors=256-start_color;
  }
  memcpy(palette+start_color, Palette_clipboard, nb_colors*sizeof(T_Components));
  return nb_colors;
}

/// Get the favorite color to use for GUI's black,dark,light or white.
const T_Components * Favorite_GUI_color(byte color_index)
{
  static const T_Components cpc_colors[4] = {
    {  0,  0,  0},
    {  0,  0,128}, // Dark blue
    {128,128,128}, // Grey
    {255,255,255}
  };
  
  if (RGB_scale==3)
  {
    // Check if ALL GUI colors are compatible with /rgb 3
    int i;
    for (i=0; i<4; i++)
    {
      T_Components col;
      col=Gfx->Default_palette[Gfx->Color[i]];
      if ((col.R!=255 && col.R!=128 && col.R!=0)
        ||(col.G!=255 && col.G!=128 && col.G!=0)
        ||(col.B!=255 && col.B!=128 && col.B!=0))
        // Specialized colors for CPC palette
        return &cpc_colors[color_index];
    }
    // Skin has suitable colors
    return &(Gfx->Default_palette[Gfx->Color[color_index]]);
  }
  else
    // Should be Config.Fav_menu_colors[index] if using user colors
      return &(Gfx->Default_palette[Gfx->Color[color_index]]);
}
