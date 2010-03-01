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

// --------------------------- Menu des palettes -----------------------------
char * Palette_reduce_label[7]=
{
  "128"," 64"," 32"," 16","  8","  4","  2"
};

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

byte Round_palette_component(byte comp)
{
  return ((comp+128/RGB_scale)*(RGB_scale-1)/255*255+(RGB_scale&1?1:0))/(RGB_scale-1);
}

// Définir les unités pour les graduations R G B ou H S V
void Componant_unit(int count)
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
  Set_color(color,palette[color].R,palette[color].G,palette[color].B);
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
  Set_color(color,palette[color].R,palette[color].G,palette[color].B);
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
  Set_color(color,palette[color].R,palette[color].G,palette[color].B);
}

void Format_componant(byte value, char *str)
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
  }
}


void Update_color_count(short * used_colors, dword * color_usage)
{
  char   str[10];

  Hide_cursor();
  Cursor_shape=CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  *used_colors=Count_used_colors(color_usage);
  strcpy(str,"Used: ");
  Num2str(*used_colors,str+6,3);
  Hide_cursor();
  Window_draw_normal_bouton(132,20,83,14,str,4,1);
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
  Remap_general_lowlevel(conversion_table,Main_screen,
                         Main_image_width,Main_image_height,Main_image_width);

  // Remap all layers
  for (layer=0; layer<Main_backups->Pages->Nb_layers; layer++)
    Remap_general_lowlevel(conversion_table,Main_backups->Pages->Image[layer],Main_image_width,Main_image_height,Main_image_width);

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
  dword temp;
  byte  conversion_table[256];

  T_Components temp_palette[256];
  dword Utilisation_temporaire[256];

  // On fait une copie de la palette
  memcpy(temp_palette, palette, sizeof(T_Palette));

  // On fait une copie de la table d'utilisation des couleurs
  memcpy(Utilisation_temporaire, color_usage, sizeof(dword) * 256);

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
      color_usage[pos_1]=Utilisation_temporaire[pos_2];
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
      color_usage[pos_1]=Utilisation_temporaire[pos_2];
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

      //   On intervertit le nombre d'utilisation des couleurs pour garder une
      // cohérence lors d'un éventuel "Zap unused".
      temp                     =color_usage[pos_1];
      color_usage[pos_1]=color_usage[pos_2];
      color_usage[pos_2]=temp;

      // On fait un changement de teinte:
      temp           =palette[pos_1].R;
      palette[pos_1].R=palette[pos_2].R;
      palette[pos_2].R=temp;

      temp           =palette[pos_1].G;
      palette[pos_1].G=palette[pos_2].G;
      palette[pos_2].G=temp;

      temp           =palette[pos_1].B;
      palette[pos_1].B=palette[pos_2].B;
      palette[pos_2].B=temp;
    }
  }

  if (with_remap)
  {
    Remap_image_highlevel(conversion_table);
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
    color=new_colors[index];
    rgb[index].R=Main_palette[color].R;
    rgb[index].G=Main_palette[color].G;
    rgb[index].B=Main_palette[color].B;
    Main_palette[color].R=Config.Fav_menu_colors[index].R;
    Main_palette[color].G=Config.Fav_menu_colors[index].G;
    Main_palette[color].B=Config.Fav_menu_colors[index].B;
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
  int   difference;
  int   best_difference;
  dword Utilisation;
  dword Meilleure_utilisation;

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
  while ((*used_colors)>nb_colors_asked)
  {
    //   Il s'agit de trouver les 2 couleurs qui se ressemblent le plus
    // parmis celles qui sont utilisées (bien sûr) et de les remplacer par
    // une seule couleur qui est la moyenne pondérée de ces 2 couleurs
    // en fonction de leur utilisation dans l'image.

    best_difference =0x7FFF;
    Meilleure_utilisation=0x7FFFFFFF;

    for (color_1=0;color_1<(*used_colors);color_1++)
      for (color_2=color_1+1;color_2<(*used_colors);color_2++)
        if (color_1!=color_2)
        {
          difference =abs((short)palette[color_1].R-palette[color_2].R)+
                      abs((short)palette[color_1].G-palette[color_2].G)+
                      abs((short)palette[color_1].B-palette[color_2].B);

          if (difference<=best_difference)
          {
            Utilisation=color_usage[color_1]+color_usage[color_2];
            if ((difference<best_difference) || (Utilisation<Meilleure_utilisation))
            {
              best_difference =difference;
              Meilleure_utilisation=Utilisation;
              best_color_1  =color_1;
              best_color_2  =color_2;
            }
          }
        }

    //   Maintenant qu'on les a trouvées, on va pouvoir mettre à jour nos
    // données pour que le remplacement se fasse sans encombres.

    // En somme, on va remplacer best_color_2 par best_color_1,
    // mais attention, on ne remplace pas best_color_1 par
    // best_color_2 !

    // On met à jour la palette.
    palette[best_color_1].R=Round_div((color_usage[best_color_1]*palette[best_color_1].R)+
                                             (color_usage[best_color_2]*palette[best_color_2].R),
                                             Meilleure_utilisation);
    palette[best_color_1].G=Round_div((color_usage[best_color_1]*palette[best_color_1].G)+
                                             (color_usage[best_color_2]*palette[best_color_2].G),
                                             Meilleure_utilisation);
    palette[best_color_1].B=Round_div((color_usage[best_color_1]*palette[best_color_1].B)+
                                             (color_usage[best_color_2]*palette[best_color_2].B),
                                             Meilleure_utilisation);

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
    Print_in_window(186,23,str,MC_Black,MC_Light);
  }

  //   Maintenant, tous ces calculs doivent êtres pris en compte dans la
  // palette, l'image et à l'écran.
  Remap_image_highlevel(conversion_table); // Et voila pour l'image et l'écran
  Display_cursor();
}



void Set_palette_slider(T_Scroller_button * slider,
                            word nb_elements, word position,
                            char * value, short x_pos)
{
  slider->Nb_elements=nb_elements;
  slider->Position=position;
  Compute_slider_cursor_height(slider);
  Window_draw_slider(slider);
  Print_counter(x_pos,172,value,MC_Black,MC_Light);
}



void Display_sliders(T_Scroller_button * red_slider,
                         T_Scroller_button * green_slider,
                         T_Scroller_button * blue_slider,
                         byte block_is_selected, T_Components * palette)
{
  char str[5];

  if (block_is_selected)
  {
    Set_palette_slider(red_slider,Color_max*2+1,Color_max,"±  0",176);
    Set_palette_slider(green_slider,Color_max*2+1,Color_max,"±  0",203);
    Set_palette_slider(blue_slider,Color_max*2+1,Color_max,"±  0",230);
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

    Format_componant(j1*Color_max/255,str);
    Set_palette_slider(red_slider,Color_count,Color_max-j1*Color_max/255,str,176);
    Format_componant(j2*Color_max/255,str);
    Set_palette_slider(green_slider,Color_count,Color_max-j2*Color_max/255,str,203);
    Format_componant(j3*Color_max/255,str);
    Set_palette_slider(blue_slider,Color_count,Color_max-j3*Color_max/255,str,230);
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
    Print_counter(176,172,"±  0",MC_Black,MC_Light);

    green_slider->Position   =Color_max;
    Window_draw_slider(green_slider);
    Print_counter(203,172,"±  0",MC_Black,MC_Light);

    blue_slider->Position   =Color_max;
    Window_draw_slider(blue_slider);
    Print_counter(230,172,"±  0",MC_Black,MC_Light);
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
    Format_componant(j1*Color_max/255,str);
    red_slider->Position=Color_max-j1*Color_max/255;
    Window_draw_slider(red_slider);
    Print_counter(176,172,str,MC_Black,MC_Light);

    Format_componant(j2*Color_max/255,str);
    green_slider->Position=Color_max-j2*Color_max/255;
    Window_draw_slider(green_slider);
    Print_counter(203,172,str,MC_Black,MC_Light);

    Format_componant(j3*Color_max/255,str);
    blue_slider->Position=Color_max-j3*Color_max/255;
    Window_draw_slider(blue_slider);
    Print_counter(230,172,str,MC_Black,MC_Light);
  }
  Display_cursor();
}


void Button_Palette(void)
{
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
  byte   image_is_backed_up = 0;
  byte   need_to_remap = 0;

  dword  color_usage[256];
  short  used_colors = -1; // -1 <=> Inconnu
  byte   conversion_table[256];

  T_Components * backup_palette;
  T_Components * temp_palette;
  T_Components * working_palette;

  backup_palette =(T_Components *)malloc(sizeof(T_Palette));
  temp_palette=(T_Components *)malloc(sizeof(T_Palette));
  working_palette=(T_Components *)malloc(sizeof(T_Palette));

  Componant_unit(RGB_scale);

  Open_window(299, 188,"Palette");

  memcpy(working_palette, Main_palette, sizeof(T_Palette));
  memcpy(backup_palette, Main_palette, sizeof(T_Palette));
  memcpy(temp_palette, Main_palette, sizeof(T_Palette));

  Window_set_palette_button(5, 79); // 1

  Window_display_frame (173, 67, 121, 116);
  Window_display_frame (128, 16, 91, 39);

  // Graduation des jauges de couleur
  Block(Window_pos_X + (Menu_factor_X * 179),
    Window_pos_Y + (Menu_factor_Y * 109), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 206),
    Window_pos_Y + (Menu_factor_Y * 109), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 233),
    Window_pos_Y + (Menu_factor_Y * 109), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 179),
    Window_pos_Y + (Menu_factor_Y * 125), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 206),
    Window_pos_Y + (Menu_factor_Y * 125), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 233),
    Window_pos_Y + (Menu_factor_Y * 125), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 179),
    Window_pos_Y + (Menu_factor_Y * 141), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 206),
    Window_pos_Y + (Menu_factor_Y * 141), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  Block(Window_pos_X + (Menu_factor_X * 233),
    Window_pos_Y + (Menu_factor_Y * 141), Menu_factor_X * 17, Menu_factor_Y,
    MC_Dark);
  // Jauges de couleur
  red_slider = Window_set_scroller_button(182, 81, 88,Color_count,1,Color_max-working_palette[Fore_color].R*Color_max/255);// 2
  green_slider = Window_set_scroller_button(209, 81, 88,Color_count,1,Color_max-working_palette[Fore_color].G*Color_max/255);// 3
  blue_slider = Window_set_scroller_button(236, 81, 88,Color_count,1,Color_max-working_palette[Fore_color].B*Color_max/255);// 4

  if(Palette_view_is_RGB==1) {
      Print_in_window(184,71,"R",MC_Dark,MC_Light);
      Print_in_window(211,71,"G",MC_Dark,MC_Light);
      Print_in_window(238,71,"B",MC_Dark,MC_Light);
      Componant_unit(RGB_scale);
  } else {
      Print_in_window(184,71,"H",MC_Dark,MC_Light);
      Print_in_window(211,71,"S",MC_Dark,MC_Light);
      Print_in_window(238,71,"L",MC_Dark,MC_Light);
      Componant_unit(256);
  }

  first_color=last_color=block_start=block_end=Fore_color;
  Tag_color_range(block_start,block_end);

  // Affichage dans le block de visu de la couleur en cours
  Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y*89),Menu_factor_X*24,Menu_factor_Y*72,Back_color);
  Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);

  // Affichage des valeurs de la couleur courante (pour 1 couleur)
  Display_sliders(red_slider,green_slider,blue_slider,(block_start!=block_end),working_palette);

  Print_in_window(129, 58, "Color number:", MC_Dark, MC_Light);
  Num2str(Fore_color, str, 3);
  Print_in_window(237, 58, str, MC_Black, MC_Light);

  Window_set_normal_button( 6,17,59,14,"Default",3,1,SDLK_f);   // 5
  Window_set_normal_button(66,17,29,14,"Gry"    ,1,1,SDLK_g);   // 6
  Window_set_normal_button(66,47,29,14,"Swp"    ,0,1,KEY_NONE);   // 7
  Window_set_normal_button( 6,47,59,14,"X-Swap" ,1,1,SDLK_x);   // 8
  Window_set_normal_button(66,32,29,14,"Cpy"    ,1,1,SDLK_c);   // 9
  Window_set_normal_button( 6,32,59,14,"Spread" ,4,1,SDLK_e);   // 10

  reduce_dropdown = Window_set_dropdown_button(222, 17, 60, 14, 60, "Reduce", 0,
    0, 1, LEFT_SIDE, 0); // 11
  Window_dropdown_add_item(reduce_dropdown, 0, "to 128");
  Window_dropdown_add_item(reduce_dropdown, 1, "to 64");
  Window_dropdown_add_item(reduce_dropdown, 2, "to 32");
  Window_dropdown_add_item(reduce_dropdown, 3, "to 16");
  Window_dropdown_add_item(reduce_dropdown, 4, "to 8");
  Window_dropdown_add_item(reduce_dropdown, 5, "to 4");
  Window_dropdown_add_item(reduce_dropdown, 6, "to 2");
  Window_dropdown_add_item(reduce_dropdown, 7, "Other");

  Window_set_normal_button(  6,168,35,14,"Undo"  ,1,1,SDLK_u);  // 12
  Window_set_normal_button( 62,168,51,14,"Cancel",0,1,KEY_ESC);  // 13
  Window_set_normal_button(117,168,51,14,"OK"    ,0,1,SDLK_RETURN);  // 14

  button_used = Window_set_normal_button(132,20,83,14,"Used: ???",4,1,SDLK_d);// 15
  Window_set_normal_button(132,37,83,14,"Zap unused",0,1,SDLK_DELETE);//16

  Window_set_repeatable_button(266, 74,12,11,"+",0,1,SDLK_KP_PLUS);       // 17
  Window_set_repeatable_button(266,165,12,11,"-",0,1,SDLK_KP_MINUS);       // 18

  Window_set_normal_button(96,17,29,14,"Neg"    ,1,1,SDLK_n);   // 19
  Window_set_normal_button(66,62,29,14,"Inv"    ,1,1,SDLK_i);   // 20
  Window_set_normal_button( 6,62,59,14,"X-Inv." ,5,1,SDLK_v);   // 21

  Window_set_normal_button(96,32,29,14,"HSL"    ,1,1,SDLK_h);   // 22
  Window_set_normal_button(96,47,29,14,"Srt"    ,1,1,SDLK_s);   // 23

  // Dessin des petits effets spéciaux pour les boutons [+] et [-]
  Draw_thingumajig(263, 74,MC_White,-1);
  Draw_thingumajig(280, 74,MC_White,+1);
  Draw_thingumajig(263,165,MC_Dark,-1);
  Draw_thingumajig(280,165,MC_Dark,+1);

  Display_cursor();

  if (Config.Auto_nb_used)
  {
    Update_color_count(&used_colors,color_usage);
    
    Hide_cursor();
    Print_in_window(222, 42, "pixels", MC_Dark, MC_Light);
    Num2str(color_usage[Fore_color], str, 6);
    Print_in_window(222, 33, str, MC_Black, MC_Light);
    Display_cursor();
  }
  
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
            if (Back_color!=temp_color)
            {
              Back_color=temp_color;
              // 4 blocks de back_color entourant la fore_color
              Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
              Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y*157),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
              Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
              Block(Window_pos_X+(Menu_factor_X*280),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
              Update_rect(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*32,Menu_factor_Y*72);
            }
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
                Block(Window_pos_X+(Menu_factor_X*237),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*56,Menu_factor_Y*7,MC_Light);
                Num2str(Fore_color,str,3);
                Print_in_window(237,58,str,MC_Black,MC_Light);
                Num2str(color_usage[Fore_color], str, 6);
                Print_in_window(222, 33, str, MC_Black, MC_Light);
                Update_rect(Window_pos_X+(Menu_factor_X*237),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*56,Menu_factor_Y*7);

                // Affichage des jauges
                Block(Window_pos_X+(Menu_factor_X*176),Window_pos_Y+(Menu_factor_Y*172),Menu_factor_X*84,Menu_factor_Y*7,MC_Light);
                Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

                // Affichage dans le block de visu de la couleur en cours
                Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
                Update_rect(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64);

                memcpy(backup_palette    ,working_palette,sizeof(T_Palette));
                memcpy(temp_palette,working_palette,sizeof(T_Palette));
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
                  Print_in_window(237,58,str,MC_Black,MC_Light);
                  {
                    int pixel_count = 0;
                    for (i = block_start; i <= block_end; i++)
                    pixel_count += color_usage[i];
                    Num2str(pixel_count, str, 6);
                  }
                  Print_in_window(222, 33, str, MC_Black, MC_Light);

                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,1,NULL);

                  // Affichage dans le block de visu du bloc (dégradé) en cours
                  Display_grad_block_in_window(264,93,block_start,block_end);
                }
                else if (first_color>temp_color)
                {
                  block_start=temp_color;
                  block_end=first_color;

                  // Affichage du n° de la couleur sélectionnée
                  Num2str(block_start,str  ,3);
                  Num2str(block_end  ,str+4,3);
                  str[3]=26; // Flèche vers la droite
                  Print_in_window(237,58,str,MC_Black,MC_Light);
                  {
                    int pixel_count = 0;
                    for (i = block_start; i <= block_end; i++)
                      pixel_count += color_usage[i];
                    Num2str(pixel_count, str, 6);
                  }
                  Print_in_window(222, 33, str, MC_Black, MC_Light);

                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,1,NULL);

                  // Affichage dans le block de visu du bloc (dégradé) en cours
                  Display_grad_block_in_window(264,93,block_start,block_end);
                }
                else
                {
                  block_start=block_end=first_color;
                  Block(Window_pos_X+(Menu_factor_X*176),Window_pos_Y+(Menu_factor_Y*172),Menu_factor_X*84,Menu_factor_Y*7,MC_Light);

                  // Affichage du n° de la couleur sélectionnée
                  Block(Window_pos_X+(Menu_factor_X*261),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*32,Menu_factor_Y*7,MC_Light);
                  Num2str(Fore_color,str,3);
                  Print_in_window(237,58,str,MC_Black,MC_Light);
                  Num2str(Fore_color, str, 6);
                  Print_in_window(222, 33, str, MC_Black, MC_Light);

                  // Affichage des jauges
                  Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

                  // Affichage dans le block de visu de la couleur en cours
                  Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
                  Update_rect(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64);
                }

                // On tagge le bloc (ou la couleur)
                Tag_color_range(block_start,block_end);
              }

              last_color=temp_color;
            }
          }
          Display_cursor();
        }
        break;
      case  2 : // Jauge rouge
        Hide_cursor();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_red(Fore_color,(Color_max-red_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].R*Color_max/255,str);
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
            Format_componant((int)255-red_slider->Position,str);
          }
          Print_counter(176,172,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
          for (i=block_start; i<=block_end; i++)
              Set_red(i,temp_palette[i].R+(Color_max-red_slider->Position)*255/Color_max,working_palette);
          }
          else
          {
            for (i=block_start; i<=block_end; i++)
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                Color_max-red_slider->Position,
                Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
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
          Print_counter(176,172,str,MC_Black,MC_Light);

        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;
      case  3 : // Jauge verte
        Hide_cursor();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_green (Fore_color,(Color_max-green_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].G*Color_max/255,str);
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
            Format_componant((int)255-green_slider->Position,str);
          }
          Print_counter(203,172,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
            for (i=block_start; i<=block_end; i++)
              Set_green (i,temp_palette[i].G+(Color_max-green_slider->Position)*255/Color_max,working_palette);
          }
          else
          {
          for (i=block_start; i<=block_end; i++)
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                Color_max-red_slider->Position,
                Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
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
          Print_counter(203,172,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case  4 : // Jauge bleue
        Hide_cursor();
        if (block_start==block_end)
        {
          if(Palette_view_is_RGB)
          {
            Set_blue (Fore_color,(Color_max-blue_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].B*Color_max/255,str);
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
            Format_componant((int)255-blue_slider->Position,str);
          }          
          Print_counter(230,172,str,MC_Black,MC_Light);
        }
        else
        {
          if(Palette_view_is_RGB)
          {
          for (i=block_start; i<=block_end; i++)
              Set_blue(i,temp_palette[i].B+(Color_max-blue_slider->Position)*255/Color_max,working_palette);
          }
          else
          {
            for (i=block_start; i<=block_end; i++)
              Set_HSL(
                temp_palette,
                working_palette,
                i,
                Color_max-red_slider->Position,
                Color_max-green_slider->Position,
                Color_max-blue_slider->Position
                );
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
          Print_counter(230,172,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 5 : // Default
        memcpy(backup_palette,working_palette,sizeof(T_Palette));
        memcpy(working_palette,Gfx->Default_palette,sizeof(T_Palette));
        memcpy(temp_palette,Gfx->Default_palette,sizeof(T_Palette));
        Set_palette(Gfx->Default_palette);
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        // On prépare la "modifiabilité" des nouvelles couleurs
        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case 6 : // Grey scale
        // Backup
        memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
          memcpy(backup_palette,working_palette,sizeof(T_Palette));

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
            Display_grad_block_in_window(264,93,block_start,block_end);
          }
          else
          {
            // Cas d'une seule couleur
            Num2str(Fore_color,str,3);
            Block(Window_pos_X+(Menu_factor_X*237),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*56,Menu_factor_Y* 7,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
          }
          Print_in_window(237,58,str,MC_Black,MC_Light);
          // On tag le bloc (ou la couleur)
          Tag_color_range(block_start,block_end);

          need_to_remap=1;

          Set_palette(working_palette);

          Display_cursor();
          Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
          
          // En cas de X-Swap, tout l'ecran a pu changer de couleur.
          if (clicked_button==8)
          {
            Update_rect(0, 0, Screen_width, Menu_Y_before_window);
            End_of_modification();
          }
          Wait_end_of_click();
        }
        break;

      case  9 : // Copy
        temp_color=Wait_click_in_palette(Window_palette_button_list);
        if ((temp_color>=0) && (temp_color!=block_start))
        {
          Hide_cursor();
          memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
            Display_grad_block_in_window(264,93,block_start,block_end);
          }
          else
          {
            // Cas d'une seule couleur
            Num2str(Fore_color,str,3);
            Block(Window_pos_X+(Menu_factor_X*237),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*56,Menu_factor_Y* 7,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
          }
          Print_in_window(237,58,str,MC_Black,MC_Light);
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
          memcpy(backup_palette,working_palette,sizeof(T_Palette));
          Spread_colors(block_start,block_end,working_palette);
        }
        else
        {
          temp_color=Wait_click_in_palette(Window_palette_button_list);
          if (temp_color>=0)
          {
            memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
        memcpy(backup_palette, working_palette, sizeof(T_Palette));
        switch(Window_attribute2) // Get the dropdown value
        {
            case 0: // 128
                reduce_colors_number = 128;
                break;
            case 1: // 64
                reduce_colors_number = 64;
                break;
            case 2: // 32
                reduce_colors_number = 32;
                break;
            case 3: // 16
                reduce_colors_number = 16;
                break;
            case 4: // 8
                reduce_colors_number = 8;
                break;
            case 5: // 4
                reduce_colors_number = 4;
                break;
            case 6: // 2
                reduce_colors_number = 2;
                break;
            case 7: // other
                reduce_colors_number
                    = Requester_window("Enter the max. number of colors",
                        reduce_colors_number);
                if (reduce_colors_number < 2 || reduce_colors_number >= 256)
                    reduce_colors_number = -1;
                break;
        }
        if (reduce_colors_number > 0)
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

            End_of_modification();
            need_to_remap = 1;
        }
        break;

      case 12: // Undo
        memcpy(temp_palette,backup_palette    ,sizeof(T_Palette));
        memcpy(backup_palette    ,working_palette,sizeof(T_Palette));
        memcpy(working_palette,temp_palette,sizeof(T_Palette));
        Draw_all_palette_sliders(red_slider,green_slider,blue_slider,working_palette,block_start,block_end);
        Set_palette(working_palette);

        need_to_remap=1;
        break;

      case 15 : // Used
        if (used_colors==-1)
          Update_color_count(&used_colors,color_usage);
        break;

      case 16 : // Zap unused
        memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
        if (block_start==block_end)
        {
          if (red_slider->Position)
          {
            (red_slider->Position)--;
            Window_draw_slider(red_slider);
            Set_red(Fore_color,(Color_max-red_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].R*Color_max/255,str);
            Print_counter(176,172,str,MC_Black,MC_Light);
          }
          if (green_slider->Position)
          {
            (green_slider->Position)--;
            Window_draw_slider(green_slider);
            Set_green (Fore_color,(Color_max-green_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].G*Color_max/255,str);
            Print_counter(203,172,str,MC_Black,MC_Light);
          }
          if (blue_slider->Position)
          {
            (blue_slider->Position)--;
            Window_draw_slider(blue_slider);
            Set_blue (Fore_color,(Color_max-blue_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].B*Color_max/255,str);
            Print_counter(230,172,str,MC_Black,MC_Light);
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
            Set_red(i,temp_palette[i].R+(Color_max-red_slider->Position)*255/Color_max,working_palette);
            Set_green (i,temp_palette[i].G+(Color_max-green_slider->Position)*255/Color_max,working_palette);
            Set_blue (i,temp_palette[i].B+(Color_max-blue_slider->Position)*255/Color_max,working_palette);
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
          Print_counter(176,172,str,MC_Black,MC_Light);


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
          Print_counter(203,172,str,MC_Black,MC_Light);


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
          Print_counter(230,172,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 18 : // [-]
        if (!Palette_view_is_RGB)
          break;
        Hide_cursor();
        if (block_start==block_end)
        {
          if (red_slider->Position<Color_max)
          {
            (red_slider->Position)++;
            Window_draw_slider(red_slider);
            Set_red(Fore_color,(Color_max-red_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].R*Color_max/255,str);
            Print_counter(176,172,str,MC_Black,MC_Light);
          }
          if (green_slider->Position<Color_max)
          {
            (green_slider->Position)++;
            Window_draw_slider(green_slider);
            Set_green (Fore_color,(Color_max-green_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].G*Color_max/255,str);
            Print_counter(203,172,str,MC_Black,MC_Light);
          }
          if (blue_slider->Position<Color_max)
          {
            (blue_slider->Position)++;
            Window_draw_slider(blue_slider);
            Set_blue (Fore_color,(Color_max-blue_slider->Position)*255/Color_max,working_palette);
            Format_componant(working_palette[Fore_color].B*Color_max/255,str);
            Print_counter(230,172,str,MC_Black,MC_Light);
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
            Set_red(i,temp_palette[i].R+(Color_max-red_slider->Position)*255/Color_max,working_palette);
            Set_green (i,temp_palette[i].G+(Color_max-green_slider->Position)*255/Color_max,working_palette);
            Set_blue (i,temp_palette[i].B+(Color_max-blue_slider->Position)*255/Color_max,working_palette);
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
          Print_counter(176,172,str,MC_Black,MC_Light);


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
          Print_counter(203,172,str,MC_Black,MC_Light);


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
          Print_counter(230,172,str,MC_Black,MC_Light);
        }

        need_to_remap=1;

        Display_cursor();
        Set_palette(working_palette);
        break;

      case 19 : // Negative
        // Backup
        memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
        memcpy(backup_palette,working_palette,sizeof(T_Palette));
        //   On initialise la table de conversion
        for (i=0; i<=255; i++)
          conversion_table[i]=i;
        // Inversion
        for (i=block_start; i< block_start + (block_end-block_start)/2;i++)
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
        }
        // On prépare la "modifiabilité" des nouvelles couleurs
        Set_palette(working_palette);
        memcpy(temp_palette,working_palette,sizeof(T_Palette));

        need_to_remap=1;
        break;

      case 22 : // HSL <> RGB
        
        // Acte les changements en cours sur une ou plusieurs couleurs
        memcpy(temp_palette,working_palette,sizeof(T_Palette));
        memcpy(backup_palette, working_palette,sizeof(T_Palette));

        Palette_view_is_RGB = !Palette_view_is_RGB;

        if(! Palette_view_is_RGB)
        {
          // On passe en HSL
          Print_in_window(184,71,"H",MC_Dark,MC_Light);
          Print_in_window(211,71,"S",MC_Dark,MC_Light);
          Print_in_window(238,71,"L",MC_Dark,MC_Light);
          Componant_unit(256);

          // Display the + and - button as disabled
            Window_draw_normal_bouton(266, 74,12,11,"+",0,0);
            Window_draw_normal_bouton(266,165,12,11,"-",0,0);
        }
        else
        {
          // On passe en RGB
          Print_in_window(184,71,"R",MC_Dark,MC_Light);
          Print_in_window(211,71,"G",MC_Dark,MC_Light);
          Print_in_window(238,71,"B",MC_Dark,MC_Light);
          Componant_unit(RGB_scale);

          // Display the + and - button as enabled
            Window_draw_normal_bouton(266, 74,12,11,"+",0,1);
            Window_draw_normal_bouton(266,165,12,11,"-",0,1);
        }
        Display_sliders(red_slider,green_slider,blue_slider,(block_start!=block_end),working_palette);
        Update_window_area(265,73,14,103);
      break;

      case 23 : // Sort palette
      {
        byte h = 0, l = 0, s=0;
        byte oh=0,ol=0,os=0; // Valeur pour la couleur précédente
        int swap=1;
        byte remap_table[256];
        byte inverted_table[256];
        byte begin, end;

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

        if(Window_attribute1==LEFT_SIDE)
        // Laft click on button: Sort by Hue (H) and Lightness (L)
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
              byte swap_color;
              Swap(0,temp_color,temp_color-1,1,working_palette,color_usage);
              
              swap_color=remap_table[temp_color];
              remap_table[temp_color]=remap_table[temp_color-1];
              remap_table[temp_color-1]=swap_color;
              
              swap=1;
            }
          }
        }

        else // Right click > Sort only on L
        while(swap==1)
        {
          swap=0;
          l=255;
          for(temp_color=begin;temp_color<=end;temp_color++)
          {
            ol=l; 
            RGB_to_HSL(working_palette[temp_color].R,
            working_palette[temp_color].G,
            working_palette[temp_color].B,&h,&s,&l);

            if(l>ol)
            {
              // Swap color with the previous one
              byte swap_color;
              Swap(0,temp_color,temp_color-1,1,working_palette,color_usage);
              
              swap_color=remap_table[temp_color];
              remap_table[temp_color]=remap_table[temp_color-1];
              remap_table[temp_color-1]=swap_color;
              
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
        
        End_of_modification();
        need_to_remap=1;
      }
      break;
    }


    if (!Mouse_K)
    {
      switch (Key)
      {
        case SDLK_LEFTBRACKET : // Décaler Forecolor vers la gauche
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
            Print_in_window(237,58,str,MC_Black,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
            Update_rect(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64);
            Display_cursor();
          }
          Key=0;
          break;

        case SDLK_RIGHTBRACKET : // Décaler Forecolor vers la droite
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
            Print_in_window(237,58,str,MC_Black,MC_Light);
            // Affichage dans le block de visu de la couleur en cours
            Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
            Update_rect(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64);
            Display_cursor();
          }
          Key=0;
          break;

        case (SDLK_LEFTBRACKET|MOD_SHIFT) : // Decaler Backcolor vers la gauche
          Back_color--;
        case (SDLK_RIGHTBRACKET|MOD_SHIFT) : // Decaler Backcolor vers la droite
          // attention: pas de break ci-dessus
          if (Key==(SDLK_RIGHTBRACKET|MOD_SHIFT))
            Back_color++;
          Hide_cursor();
          Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
          Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y*157),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
          Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
          Block(Window_pos_X+(Menu_factor_X*280),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
          Update_rect(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*32,Menu_factor_Y*72);
          Display_cursor();
          Key=0;
          break;

        case SDLK_BACKSPACE : // Remise des couleurs du menu à l'état normal en essayant
                      // de ne pas trop modifier l'image.
          if (!image_is_backed_up)
          {
            Backup_layers(-1);
            image_is_backed_up=1;
          }
          if (used_colors==-1)
            Update_color_count(&used_colors, color_usage);

          memcpy(backup_palette,working_palette,sizeof(T_Palette));
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
          break;

        case SDLK_BACKQUOTE : // Récupération d'une couleur derrière le menu
        case SDLK_COMMA :
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
                Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
                Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y*157),Menu_factor_X*24,Menu_factor_Y<<2,Back_color);
                Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
                Block(Window_pos_X+(Menu_factor_X*280),Window_pos_Y+(Menu_factor_Y* 93),Menu_factor_X<<2,Menu_factor_Y<<6,Back_color);
                Update_rect(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y* 89),Menu_factor_X*32,Menu_factor_Y*72);
              }
            }
            else
            {
              Fore_color=first_color=last_color=block_start=block_end=color;
              Tag_color_range(block_start,block_end);

              // Affichage du n° de la couleur sélectionnée
              Block(Window_pos_X+(Menu_factor_X*261),Window_pos_Y+(Menu_factor_Y*58),Menu_factor_X*32,Menu_factor_Y*7,MC_Light);
              Num2str(Fore_color,str,3);
              Print_in_window(237,58,str,MC_Black,MC_Light);
              Num2str(color_usage[Fore_color], str, 6);
              Print_in_window(222, 33, str, MC_Black, MC_Light);

              // Affichage des jauges
              Display_sliders(red_slider,green_slider,blue_slider,0,working_palette);

              // Affichage dans le block de visu de la couleur en cours
              Block(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64,Fore_color);
              Update_rect(Window_pos_X+(Menu_factor_X*264),Window_pos_Y+(Menu_factor_Y*93),Menu_factor_X<<4,Menu_factor_Y*64);
              
              memcpy(backup_palette    ,working_palette,sizeof(T_Palette));
              memcpy(temp_palette,working_palette,sizeof(T_Palette));
            }
            Display_cursor();
            Wait_end_of_click();
          }
          Key=0;
          break;
        default:
          if (Is_shortcut(Key,0x100+BUTTON_HELP))
          {
            Key=0;
            Window_help(BUTTON_PALETTE, NULL);
            break;
          }
          else if (Is_shortcut(Key,0x100+BUTTON_PALETTE))
            clicked_button=14;
      }

      if (need_to_remap)
      {
        Hide_cursor();
        Compute_optimal_menu_colors(working_palette);

        // On remappe brutalement
        Remap_screen_after_menu_colors_change();
        // Puis on remet les trucs qui ne devaient pas changer
        Window_draw_palette_bouton(5,79);
        Block(Window_pos_X+(Menu_factor_X*260),Window_pos_Y+(Menu_factor_Y*89),Menu_factor_X*24,Menu_factor_Y*72,Back_color);
        Display_grad_block_in_window(264,93,block_start,block_end);

        Update_rect(Window_pos_X+8*Menu_factor_X,Window_pos_Y+82*Menu_factor_Y,Menu_factor_X*16*10,Menu_factor_Y*5*16);

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
    Display_sprite_in_menu(BUTTON_PAL_LEFT,18+(Config.Palette_vertical!=0));
  }
}
