/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2009 Petter Lindquist
    Copyright 2008 Yves Rizoud
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
*/
#define _XOPEN_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <SDL_image.h>
#include <SDL_endian.h>

#include "buttons.h"
#include "const.h"
#include "errors.h"
#include "global.h"
#include "io.h"
#include "loadsave.h"
#include "misc.h"
#include "graph.h"
#include "op_c.h"
#include "pages.h"
#include "palette.h"
#include "sdlscreen.h"
#include "struct.h"
#include "windows.h"
#include "engine.h"

// -- PKM -------------------------------------------------------------------
void Test_PKM(void);
void Load_PKM(void);
void Save_PKM(void);

// -- LBM -------------------------------------------------------------------
void Test_LBM(void);
void Load_LBM(void);
void Save_LBM(void);

// -- GIF -------------------------------------------------------------------
void Test_GIF(void);
void Load_GIF(void);
void Save_GIF(void);

// -- PCX -------------------------------------------------------------------
void Test_PCX(void);
void Load_PCX(void);
void Save_PCX(void);

// -- BMP -------------------------------------------------------------------
void Test_BMP(void);
void Load_BMP(void);
void Save_BMP(void);

// -- IMG -------------------------------------------------------------------
void Test_IMG(void);
void Load_IMG(void);
void Save_IMG(void);

// -- SCx -------------------------------------------------------------------
void Test_SCx(void);
void Load_SCx(void);
void Save_SCx(void);

// -- CEL -------------------------------------------------------------------
void Test_CEL(void);
void Load_CEL(void);
void Save_CEL(void);

// -- KCF -------------------------------------------------------------------
void Test_KCF(void);
void Load_KCF(void);
void Save_KCF(void);

// -- PAL -------------------------------------------------------------------
void Test_PAL(void);
void Load_PAL(void);
void Save_PAL(void);

// -- PI1 -------------------------------------------------------------------
void Test_PI1(void);
void Load_PI1(void);
void Save_PI1(void);

// -- PC1 -------------------------------------------------------------------
void Test_PC1(void);
void Load_PC1(void);
void Save_PC1(void);

// -- NEO -------------------------------------------------------------------
void Test_NEO(void);
void Load_NEO(void);
void Save_NEO(void);

// -- C64 -------------------------------------------------------------------
void Test_C64(void);
void Load_C64(void);
void Save_C64(void);

// -- SCR (Amstrad CPC)
void Save_SCR(void);

// -- PNG -------------------------------------------------------------------
#ifndef __no_pnglib__
void Test_PNG(void);
void Load_PNG(void);
void Save_PNG(void);
#endif

// -- SDL_Image -------------------------------------------------------------
// (TGA, BMP, PNM, XPM, XCF, PCX, GIF, JPG, TIF, LBM, PNG, ICO)
void Load_SDL_Image(void);

// ENUM			Name	TestFunc LoadFunc SaveFunc Backup Comment Layers Ext Exts	
T_Format File_formats[NB_KNOWN_FORMATS] = {
  {FORMAT_ALL_IMAGES, "(all)", NULL, NULL, NULL, 0, 0, 0, "", "gif;png;bmp;pcx;pkm;lbm;iff;img;sci;scq;scf;scn;sco;pi1;pc1;cel;neo;kcf;pal;c64;koa;tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff;ico"},
  {FORMAT_ALL_FILES, "(*.*)", NULL, NULL, NULL, 0, 0, 0, "", "*"},
  {FORMAT_GIF, " gif", Test_GIF, Load_GIF, Save_GIF, 1, 1, 1, "gif", "gif"},
#ifndef __no_pnglib__
  {FORMAT_PNG, " png", Test_PNG, Load_PNG, Save_PNG, 1, 1, 0, "png", "png"},
#endif
  {FORMAT_BMP, " bmp", Test_BMP, Load_BMP, Save_BMP, 1, 0, 0, "bmp", "bmp"},
  {FORMAT_PCX, " pcx", Test_PCX, Load_PCX, Save_PCX, 1, 0, 0, "pcx", "pcx"},
  {FORMAT_PKM, " pkm", Test_PKM, Load_PKM, Save_PKM, 0, 1, 0, "pkm", "pkm"}, // Not a backup since it does not save the full palette
  {FORMAT_LBM, " lbm", Test_LBM, Load_LBM, Save_LBM, 1, 0, 0, "lbm", "lbm;iff"},
  {FORMAT_IMG, " img", Test_IMG, Load_IMG, Save_IMG, 1, 0, 0, "img", "img"},
  {FORMAT_SCx, " sc?", Test_SCx, Load_SCx, Save_SCx, 1, 0, 0, "sc?", "sci;scq;scf;scn;sco"},
  {FORMAT_PI1, " pi1", Test_PI1, Load_PI1, Save_PI1, 0, 0, 0, "pi1", "pi1"},
  {FORMAT_PC1, " pc1", Test_PC1, Load_PC1, Save_PC1, 0, 0, 0, "pc1", "pc1"},
  {FORMAT_CEL, " cel", Test_CEL, Load_CEL, Save_CEL, 1, 0, 0, "cel", "cel"},
  {FORMAT_NEO, " neo", Test_NEO, Load_NEO, Save_NEO, 0, 0, 0, "neo", "neo"},
  {FORMAT_KCF, " kcf", Test_KCF, Load_KCF, Save_KCF, 0, 0, 0, "kcf", "kcf"},
  {FORMAT_PAL, " pal", Test_PAL, Load_PAL, Save_PAL, 0, 0, 0, "pal", "pal"},
  {FORMAT_C64, " c64", Test_C64, Load_C64, Save_C64, 0, 1, 0, "c64", "c64;koa"},
  {FORMAT_SCR, " cpc", NULL,     NULL,     Save_SCR, 0, 0, 0, "cpc", "cpc;scr"},
  {FORMAT_MISC,"misc.",NULL,     NULL,     NULL,     1, 0, 0, "",    "tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff;ico"},
};

// Cette variable est alimentée après chargement réussi d'une image.
// Actuellement seul le format PNG peut donner autre chose que PIXEL_SIMPLE.
enum PIXEL_RATIO Ratio_of_loaded_image=PIXEL_SIMPLE;

// Chargement des pixels dans l'écran principal
void Pixel_load_in_current_screen(word x_pos,word y_pos,byte color)
{
  //if ((x_pos>=0) && (y_pos>=0)) //Toujours vrai ?
  if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
    Pixel_in_current_screen(x_pos,y_pos,color,0);
}


// Chargement des pixels dans la brosse
void Pixel_load_in_brush(word x_pos,word y_pos,byte color)
{
  //if ((x_pos>=0) && (y_pos>=0))
  if ((x_pos<Brush_width) && (y_pos<Brush_height))
    Pixel_in_brush(x_pos,y_pos,color);
}


short Preview_factor_X;
short Preview_factor_Y;
short Preview_pos_X;
short Preview_pos_Y;


// Chargement des pixels dans la preview
void Pixel_load_in_preview(word x_pos,word y_pos,byte color)
{
  if (((x_pos % Preview_factor_X)==0) && ((y_pos % Preview_factor_Y)==0))
    if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
    {
      if (Ratio_of_loaded_image == PIXEL_WIDE && 
        Pixel_ratio != PIXEL_WIDE &&
        Pixel_ratio != PIXEL_WIDE2)
      {
         Pixel(Preview_pos_X+(x_pos/Preview_factor_X*2),
               Preview_pos_Y+(y_pos/Preview_factor_Y),
               color);
         Pixel(Preview_pos_X+(x_pos/Preview_factor_X*2)+1,
               Preview_pos_Y+(y_pos/Preview_factor_Y),
               color);
      }
      else if (Ratio_of_loaded_image == PIXEL_TALL && 
        Pixel_ratio != PIXEL_TALL &&
        Pixel_ratio != PIXEL_TALL2)
      {
         Pixel(Preview_pos_X+(x_pos/Preview_factor_X),
               Preview_pos_Y+(y_pos/Preview_factor_Y*2),
               color);
         Pixel(Preview_pos_X+(x_pos/Preview_factor_X),
               Preview_pos_Y+(y_pos/Preview_factor_Y*2)+1,
               color);
      }
      else
        Pixel(Preview_pos_X+(x_pos/Preview_factor_X),
              Preview_pos_Y+(y_pos/Preview_factor_Y),
              color);
    }
}


void Remap_fileselector(void)
{
  if (Pixel_load_function==Pixel_load_in_preview)
  {
    Compute_optimal_menu_colors(Main_palette);

    if(
            (
            Main_palette[MC_Black].R==Main_palette[MC_Dark].R &&
            Main_palette[MC_Black].G==Main_palette[MC_Dark].G &&
            Main_palette[MC_Black].B==Main_palette[MC_Dark].B
            ) ||
            (
            Main_palette[MC_Light].R==Main_palette[MC_Dark].R &&
            Main_palette[MC_Light].G==Main_palette[MC_Dark].G &&
            Main_palette[MC_Light].B==Main_palette[MC_Dark].B
            ) ||
            (
            Main_palette[MC_White].R==Main_palette[MC_Light].R &&
            Main_palette[MC_White].G==Main_palette[MC_Light].G &&
            Main_palette[MC_White].B==Main_palette[MC_Light].B
            )
      )
    {
        // Si on charge une image monochrome, le fileselect ne sera plus visible. Dans ce cas on force quelques couleurs à des valeurs sures

        int black =
            Main_palette[MC_Black].R +
            Main_palette[MC_Black].G +
            Main_palette[MC_Black].B;
        int white =
            Main_palette[MC_White].R +
            Main_palette[MC_White].G +
            Main_palette[MC_White].B;

        //Set_color(MC_Light,(2*white+black)/9,(2*white+black)/9,(2*white+black)/9);
        //Set_color(MC_Dark,(2*black+white)/9,(2*black+white)/9,(2*black+white)/9);
        Main_palette[MC_Dark].R=(2*black+white)/9;
        Main_palette[MC_Dark].G=(2*black+white)/9;
        Main_palette[MC_Dark].B=(2*black+white)/9;
        Main_palette[MC_Light].R=(2*white+black)/9;
        Main_palette[MC_Light].G=(2*white+black)/9;
        Main_palette[MC_Light].B=(2*white+black)/9;

        Set_palette(Main_palette);
    }
    Remap_screen_after_menu_colors_change();
  }
}

int Image_24b;
T_Components *   Buffer_image_24b;
Func_24b_display Pixel_load_24b;


// Chargement des pixels dans le buffer 24b
void Pixel_load_in_24b_buffer(short x_pos,short y_pos,byte r,byte g,byte b)
{
  int index;

  if ((x_pos>=0) && (y_pos>=0))
  if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
  {
    index=(y_pos*Main_image_width)+x_pos;
    Buffer_image_24b[index].R=r;
    Buffer_image_24b[index].G=g;
    Buffer_image_24b[index].B=b;
  }
}

// Chargement des pixels dans la preview en 24b
void Pixel_load_in_24b_preview(short x_pos,short y_pos,byte r,byte g,byte b)
{
  byte color;

  if (((x_pos % Preview_factor_X)==0) && ((y_pos % Preview_factor_Y)==0))
  if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
  {
    color=((r >> 5) << 5) |
            ((g >> 5) << 2) |
            ((b >> 6));
    Pixel(Preview_pos_X+(x_pos/Preview_factor_X),
          Preview_pos_Y+(y_pos/Preview_factor_Y),
          color);
  }
}

// Création d'une palette fake
void Set_palette_fake_24b(T_Palette palette)
{
  int color;

  // Génération de la palette
  for (color=0;color<256;color++)
  {
    palette[color].R=((color & 0xE0)>>5)<<5;
    palette[color].G=((color & 0x1C)>>2)<<5;
    palette[color].B=((color & 0x03)>>0)<<6;
  }
}

// Initialization for a 24bit image
void Init_preview_24b(short width,short height,long size,int format)
{
  // Call common processing
  Init_preview(width,height,size,format, PIXEL_SIMPLE);

  if (File_error)
    return;

  if (Pixel_load_function==Pixel_load_in_preview)
  {
    // Choose 24bit pixel "writer"
    Pixel_load_24b=Pixel_load_in_24b_preview;

    // Load palette
    Set_palette_fake_24b(Main_palette);
    Set_palette(Main_palette);
    Remap_fileselector();
  }
  else
  {
    // Choose 24bit pixel "writer"
    Pixel_load_24b=Pixel_load_in_24b_buffer;

    // Allocate 24bit buffer
    Buffer_image_24b=
      (T_Components *)malloc(width*height*sizeof(T_Components));
    if (!Buffer_image_24b)
    {
      // Print an error message

      // The following is to be sure the messagfe is readable
      Compute_optimal_menu_colors(Main_palette);
      Message_out_of_memory();
      if (Pixel_load_function==Pixel_load_in_current_screen)
        File_error=1; // 1 => On n'a pas perdu l'image courante
      else
        File_error=3; // 3 => Chargement de brosse échoué
    }
    else
      Image_24b=1;        // On a un buffer à traiter en fin de chargement
  }
}




void Init_preview(short width,short height,long size,int format, enum PIXEL_RATIO ratio)
//
//   Cette procédure doit être appelée par les routines de chargement
// d'images.
//   Elle doit être appelée entre le moment où l'on connait la dimension de
// l'image (dimension réelle, pas dimension tronquée) et l'affichage du
// premier point.
//
{
  char  str[10];

  if (Pixel_load_function==Pixel_load_in_preview)
  {
    // Préparation du chargement d'une preview:

    // Affichage des données "Image size:"
    if ((width<10000) && (height<10000))
    {
      Num2str(width,str,4);
      Num2str(height,str+5,4);
      str[4]='x';
      Print_in_window(143,59,str,MC_Black,MC_Light);
    }
    else
    {
      Print_in_window(143,59,"VERY BIG!",MC_Black,MC_Light);
    }

    // Affichage de la taille du fichier
    if (size<1048576)
    {
      // Le fichier fait moins d'un Mega, on affiche sa taille direct
      Num2str(size,str,7);
      Print_in_window(236,59,str,MC_Black,MC_Light);
    }
    else if ((size/1024)<100000)
    {
      // Le fichier fait plus d'un Mega, on peut afficher sa taille en Ko
      Num2str(size/1024,str,5);
      strcpy(str+5,"Kb");
      Print_in_window(236,59,str,MC_Black,MC_Light);
    }
    else
    {
      // Le fichier fait plus de 100 Mega octets (cas très rare :))
      Print_in_window(236,59,"LARGE!!",MC_Black,MC_Light);
    }

    // Affichage du vrai format
    if (format!=Main_format)
    {
      Print_in_window( 59,59,Get_fileformat(format)->Label,MC_Black,MC_Light);
    }

    // On efface le commentaire précédent
    Window_rectangle(45,70,32*8,8,MC_Light);
    // Affichage du commentaire
    if (Get_fileformat(format)->Comment)
      Print_in_window(45,70,Main_comment,MC_Black,MC_Light);

    // Calculs des données nécessaires à l'affichage de la preview:
    if (ratio == PIXEL_WIDE && 
        Pixel_ratio != PIXEL_WIDE &&
        Pixel_ratio != PIXEL_WIDE2)
      width*=2;
    else if (ratio == PIXEL_TALL && 
        Pixel_ratio != PIXEL_TALL &&
        Pixel_ratio != PIXEL_TALL2)
      height*=2;
    
    Preview_factor_X=Round_div_max(width,122*Menu_factor_X);
    Preview_factor_Y=Round_div_max(height, 82*Menu_factor_Y);

    if ( (!Config.Maximize_preview) && (Preview_factor_X!=Preview_factor_Y) )
    {
      if (Preview_factor_X>Preview_factor_Y)
        Preview_factor_Y=Preview_factor_X;
      else
        Preview_factor_X=Preview_factor_Y;
    }

    Preview_pos_X=Window_pos_X+183*Menu_factor_X;
    Preview_pos_Y=Window_pos_Y+ 95*Menu_factor_Y;

    // On nettoie la zone où va s'afficher la preview:
    Window_rectangle(183,95,120,80,MC_Light);
    
    // Un update pour couvrir les 4 zones: 3 libellés plus le commentaire
    Update_window_area(45,48,256,30);
    // Zone de preview
    Update_window_area(183,95,120,80);
  }
  else
  {
    if (Pixel_load_function==Pixel_load_in_current_screen)
    {
      if (Backup_with_new_dimensions(0,1,width,height))
      {
        // La nouvelle page a pu être allouée, elle est pour l'instant pleine
        // de 0s. Elle fait Main_image_width de large.
        // Normalement tout va bien, tout est sous contrôle...
        
        // Load into layer 0, by default.
        Main_current_layer=0;
      }
      else
      {
        // Afficher un message d'erreur

        // Pour être sûr que ce soit lisible.
        Compute_optimal_menu_colors(Main_palette);
        Message_out_of_memory();
        File_error=1; // 1 => On n'a pas perdu l'image courante
      }
    }
    else // chargement dans la brosse
    {
      free(Brush);
      free(Smear_brush);
      Brush=(byte *)malloc(width*height);
      Brush_width=width;
      Brush_height=height;
      if (Brush)
      {
        Smear_brush=(byte *)malloc(width*height);
        if (!Smear_brush)
          File_error=3;
      }
      else
        File_error=3;
    }
  }
}


// Calcul du nom complet du fichier
void Get_full_filename(char * filename, byte is_colorix_format)
{
    byte last_char;

    strcpy(filename,Main_file_directory);

    //On va ajouter un séparateur à la fin du chemin s'il n'y est pas encore
    if (filename[strlen(filename)-1]!=PATH_SEPARATOR[0])
        strcat(filename,PATH_SEPARATOR);

  // Si on est en train de sauvegarder une image Colorix, on calcule son ext.
  if (is_colorix_format)
  {
    last_char=strlen(Main_filename)-1;
    if (Main_filename[last_char]=='?')
    {
      if (Main_image_width<=320)
        Main_filename[last_char]='I';
      else
      {
        if (Main_image_width<=360)
          Main_filename[last_char]='Q';
        else
        {
          if (Main_image_width<=640)
            Main_filename[last_char]='F';
          else
          {
            if (Main_image_width<=800)
              Main_filename[last_char]='N';
            else
              Main_filename[last_char]='O';
          }
        }
      }
    }
  }

  strcat(filename,Main_filename);
}


/////////////////////////////////////////////////////////////////////////////
//                    Gestion des lectures et écritures                    //
/////////////////////////////////////////////////////////////////////////////

byte * Write_buffer;
word   Write_buffer_index;

void Init_write_buffer(void)
{
  Write_buffer=(byte *)malloc(64000);
  Write_buffer_index=0;
}

void Write_one_byte(FILE *file, byte b)
{
  Write_buffer[Write_buffer_index++]=b;
  if (Write_buffer_index>=64000)
  {
    if (! Write_bytes(file,Write_buffer,64000))
      File_error=1;
    Write_buffer_index=0;
  }
}

void End_write(FILE *file)
{
  if (Write_buffer_index)
    if (! Write_bytes(file,Write_buffer,Write_buffer_index))
      File_error=1;
  free(Write_buffer);
}


/////////////////////////////////////////////////////////////////////////////

// -------- Modifier la valeur du code d'erreur d'accès à un fichier --------
//   On n'est pas obligé d'utiliser cette fonction à chaque fois mais il est
// important de l'utiliser dans les cas du type:
//   if (!File_error) *** else File_error=***;
// En fait, dans le cas où l'on modifie File_error alors qu'elle contient
// dèjà un code d'erreur.
void Set_file_error(int value)
{
  if (File_error>=0)
    File_error=value;
}


// -- Charger n'importe connu quel type de fichier d'image (ou palette) -----
void Load_image(byte image)
{
  unsigned int index; // index de balayage des formats
  T_Format *format = &(File_formats[2]); // Format du fichier à charger


  // On place par défaut File_error à vrai au cas où on ne sache pas
  // charger le format du fichier:
  File_error=1;

  if (Main_format>FORMAT_ALL_FILES)
  {
    format = Get_fileformat(Main_format);
    if (format->Test)
      format->Test();
  }

  if (File_error)
  {
    //  Sinon, on va devoir scanner les différents formats qu'on connait pour
    // savoir à quel format est le fichier:
    for (index=0; index < NB_KNOWN_FORMATS; index++)
    {
      format = Get_fileformat(index);
      // Loadable format
      if (format->Test == NULL)
        continue;
        
      // On appelle le testeur du format:
      format->Test();
      // On s'arrête si le fichier est au bon format:
      if (File_error==0)
        break;
    }
  }
  
  if (File_error)
  {
    // Last try: with SDL_image
    Image_24b=0;
    Ratio_of_loaded_image=PIXEL_SIMPLE;

    Load_SDL_Image();

    if (File_error)
    { 
      // Sinon, l'appelant sera au courant de l'échec grace à File_error;
      // et si on s'apprêtait à faire un chargement définitif de l'image (pas
      // une preview), alors on flash l'utilisateur.
      if (Pixel_load_function!=Pixel_load_in_preview)
        Error(0);
      return;
    }
  }
  else
  // Si on a su déterminer avec succès le format du fichier:
  {
    // On peut charger le fichier:
    Image_24b=0;
    Ratio_of_loaded_image=PIXEL_SIMPLE;
    // Dans certains cas il est possible que le chargement plante
    // après avoir modifié la palette. TODO
    format->Load();
  }

  if (File_error>0)
  {
    Error(0);
  }

  if (Image_24b)
  {
    // On vient de charger une image 24b
    if (!File_error)
    {
      // Le chargement a réussi, on peut faire la conversion en 256 couleurs
      if (Pixel_load_function==Pixel_load_in_current_screen)
      {
        // Cas d'un chargement dans l'image
        Hide_cursor();
        Cursor_shape=CURSOR_SHAPE_HOURGLASS;
        Display_cursor();
        Flush_update();
        if (Convert_24b_bitmap_to_256(Main_screen,Buffer_image_24b,Main_image_width,Main_image_height,Main_palette))
          File_error=2;
        else
        {
          Set_palette(Main_palette);
        }
      }
      else
      {
        // Cas d'un chargement dans la brosse
        Hide_cursor();
        Cursor_shape=CURSOR_SHAPE_HOURGLASS;
        Display_cursor();
        Flush_update();
        if (Convert_24b_bitmap_to_256(Brush,Buffer_image_24b,Brush_width,Brush_height,Main_palette))
          File_error=2;
      }
    }

    free(Buffer_image_24b);
  }

  if (image)
  {
    if ( (File_error!=1) && (format->Backup_done) )
    {
      if (Pixel_load_function==Pixel_load_in_preview)
      {
        dword  color_usage[256];
        Count_used_colors_screen_area(color_usage,Preview_pos_X,Preview_pos_Y,Main_image_width/Preview_factor_X,Main_image_height/Preview_factor_Y);
        //Count_used_colors(color_usage);
        Display_cursor();
        Set_nice_menu_colors(color_usage,1);
        Hide_cursor();
      }
    
      // On considère que l'image chargée n'est plus modifiée
      Main_image_is_modified=0;
      // Et on documente la variable Main_fileformat avec la valeur:
      Main_fileformat=format->Identifier;

      // Correction des dimensions
      if (Main_image_width<1)
        Main_image_width=1;
      if (Main_image_height<1)
        Main_image_height=1;
    }
    else if (File_error!=1)
    {
      // On considère que l'image chargée est encore modifiée
      Main_image_is_modified=1;
      // Et on documente la variable Main_fileformat avec la valeur:
      Main_fileformat=format->Identifier;
    }
    else
    {
      // Dans ce cas, on sait que l'image n'a pas changé, mais ses
      // paramètres (dimension, palette, ...) si. Donc on les restaures.
      Download_infos_page_main(Main_backups->Pages);
    }
  }
}


// -- Sauver n'importe quel type connu de fichier d'image (ou palette) ------
void Save_image(byte image)
{
  T_Format *format;
  
  // On place par défaut File_error à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  File_error=1;

  if (image)
  {
    if (!File_formats[Main_fileformat-1].Supports_layers
      && Main_backups->Pages->Nb_layers > 1)
    {
      if (! Confirmation_box("This format will save a flattened copy."))
      {
        // File_error is already set to 1.
        return;
      }
      Read_pixel_function=Read_pixel_from_current_screen;
    }
    else
    {
      Read_pixel_function=Read_pixel_from_current_layer;
    }
  }
  else
  {
    Read_pixel_function=Read_pixel_from_brush;
  }
  

  format = Get_fileformat(Main_fileformat);
  if (format->Save)
    format->Save();

  if (File_error)
    Error(0);
  else
  {
    if ((image) && (Get_fileformat(Main_fileformat)->Backup_done))
      Main_image_is_modified=0;
  }
}


void Load_SDL_Image(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  word x_pos,y_pos;
  // long file_size;
  dword pixel;
  long file_size;
  SDL_Surface * surface;


  Get_full_filename(filename,0);
  File_error=0;
  
  surface = IMG_Load(filename);
  
  if (!surface)
  {
    File_error=1;
    return;
  }
  
  file_size=File_length(filename);
  
  if (surface->format->BytesPerPixel == 1)
  {
    // 8bpp image
    
    Init_preview(surface->w, surface->h, file_size ,FORMAT_MISC, PIXEL_SIMPLE);
    // Read palette
    if (surface->format->palette)
    {
      Get_SDL_Palette(surface->format->palette, Main_palette);
      Set_palette(Main_palette);
      Remap_fileselector();
    }
    Main_image_width=surface->w;
    Main_image_height=surface->h;
    
    for (y_pos=0; y_pos<Main_image_height; y_pos++)
    {
      for (x_pos=0; x_pos<Main_image_width; x_pos++)
      {
        Pixel_load_function(x_pos,y_pos,Get_SDL_pixel_8(surface, x_pos, y_pos));
      }
    }

  }
  else
  {
    // Hi/Trucolor
    Init_preview_24b(surface->w, surface->h, file_size ,FORMAT_ALL_IMAGES);
    Main_image_width=surface->w;
    Main_image_height=surface->h;
    
    for (y_pos=0; y_pos<Main_image_height; y_pos++)
    {
      for (x_pos=0; x_pos<Main_image_width; x_pos++)
      {
        pixel = Get_SDL_pixel_hicolor(surface, x_pos, y_pos);
        Pixel_load_24b(
          x_pos,
          y_pos, 
          ((pixel & surface->format->Rmask) >> surface->format->Rshift) << surface->format->Rloss,
          ((pixel & surface->format->Gmask) >> surface->format->Gshift) << surface->format->Gloss,
          ((pixel & surface->format->Bmask) >> surface->format->Bshift) << surface->format->Bloss);
      }
    }
  }

  SDL_FreeSurface(surface);
}

/// Saves an image.
/// This routine will only be called when all hope is lost, memory thrashed, etc
/// It's the last chance to save anything, but the code has to be extremely
/// careful, anything could happen.
/// The chosen format is IMG since it's extremely simple, difficult to make it
/// create an unusable image.
void Emergency_backup(const char *fname, byte *source, int width, int height, T_Palette *palette)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_IMG_Header IMG_header;

  strcpy(filename,Config_directory);
  strcat(filename,fname);

  // Ouverture du fichier
  file=fopen(filename,"wb");
  if (!file)
    return;

  memcpy(IMG_header.Filler1,"\x01\x00\x47\x12\x6D\xB0",6);
  memset(IMG_header.Filler2,0,118);
  IMG_header.Filler2[4]=0xFF;
  IMG_header.Filler2[22]=64; // Lo(Longueur de la signature)
  IMG_header.Filler2[23]=0;  // Hi(Longueur de la signature)
  memcpy(IMG_header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

  if (!Write_bytes(file,IMG_header.Filler1,6) ||
      !Write_word_le(file,width) ||
      !Write_word_le(file,height) ||
      !Write_bytes(file,IMG_header.Filler2,118) ||
      !Write_bytes(file,palette,sizeof(T_Palette)))
    {
      fclose(file);
      return;
    }

  for (y_pos=0; ((y_pos<height) && (!File_error)); y_pos++)
    for (x_pos=0; x_pos<width; x_pos++)
      if (!Write_byte(file,*(source+y_pos*width+x_pos)))
      {
        fclose(file);
        return;
      }

  // Ouf, sauvé
  fclose(file);
}

void Image_emergency_backup()
{
  Emergency_backup("phoenix.img",Main_screen, Main_image_width, Main_image_height, &Main_palette);
  Emergency_backup("phoenix2.img",Spare_screen, Spare_image_width, Spare_image_height, &Spare_palette);
}

T_Format * Get_fileformat(byte format)
{
  unsigned int i;
  T_Format * safe_default = File_formats;
  
  for (i=0; i < NB_KNOWN_FORMATS; i++)
  {
    if (File_formats[i].Identifier == format)
      return &(File_formats[i]);
  
    if (File_formats[i].Identifier == FORMAT_GIF)
      safe_default=&(File_formats[i]);
  }
  // Normally impossible to reach this point, unless called with an invalid
  // enum....
  return safe_default;
}
