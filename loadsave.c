/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
#ifndef __no_pnglib__
#include <png.h>
#endif

#include "buttons.h"
#include "const.h"
#include "errors.h"
#include "global.h"
#include "io.h"
#include "loadsave.h"
#include "misc.h"
#include "op_c.h"
#include "pages.h"
#include "palette.h"
#include "sdlscreen.h"
#include "struct.h"
#include "windows.h"

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

// -- PNG -------------------------------------------------------------------
#ifndef __no_pnglib__
void Test_PNG(void);
void Load_PNG(void);
void Save_PNG(void);
#endif

void Init_preview(short width,short height,long size,int format);

T_Format File_formats[NB_KNOWN_FORMATS] = {
  {"pkm", Test_PKM, Load_PKM, Save_PKM, 1, 1},
  {"lbm", Test_LBM, Load_LBM, Save_LBM, 1, 0},
  {"gif", Test_GIF, Load_GIF, Save_GIF, 1, 1},
  {"bmp", Test_BMP, Load_BMP, Save_BMP, 1, 0},
  {"pcx", Test_PCX, Load_PCX, Save_PCX, 1, 0},
  {"img", Test_IMG, Load_IMG, Save_IMG, 1, 0},
  {"sc?", Test_SCx, Load_SCx, Save_SCx, 1, 0},
  {"pi1", Test_PI1, Load_PI1, Save_PI1, 1, 0},
  {"pc1", Test_PC1, Load_PC1, Save_PC1, 1, 0},
  {"cel", Test_CEL, Load_CEL, Save_CEL, 1, 0},
  {"neo", Test_NEO, Load_NEO, Save_NEO, 1, 0},
  {"kcf", Test_KCF, Load_KCF, Save_KCF, 0, 0},
  {"pal", Test_PAL, Load_PAL, Save_PAL, 0, 0},
  {"c64", Test_C64, Load_C64, Save_C64, 1, 0},
#ifndef __no_pnglib__
  {"png", Test_PNG, Load_PNG, Save_PNG, 1, 1}
#endif
};

// Cette variable est alimentée après chargement réussi d'une image.
// Actuellement seul le format PNG peut donner autre chose que PIXEL_SIMPLE.
enum PIXEL_RATIO Ratio_of_loaded_image=PIXEL_SIMPLE;

// Chargement des pixels dans l'écran principal
void Pixel_load_in_current_screen(word x_pos,word y_pos,byte color)
{
  //if ((x_pos>=0) && (y_pos>=0)) //Toujours vrai ?
  if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
    Pixel_in_current_screen(x_pos,y_pos,color);
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

byte HBPm1; // header.BitPlanes-1


// Chargement des pixels dans la preview
void Pixel_load_in_preview(word x_pos,word y_pos,byte color)
{
  if (((x_pos % Preview_factor_X)==0) && ((y_pos % Preview_factor_Y)==0))
  if ((x_pos<Main_image_width) && (y_pos<Main_image_height))
    Pixel(Preview_pos_X+(x_pos/Preview_factor_X),
          Preview_pos_Y+(y_pos/Preview_factor_Y),
          color);
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



// Données pour la gestion du chargement en 24b
#define FORMAT_24B 0x100
typedef void (* Func_24b_display) (short,short,byte,byte,byte);
int                    Image_24b;
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
  Init_preview(width,height,size,format);

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
      (T_Components *)Borrow_memory_from_page(width*height*sizeof(T_Components));
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




void Init_preview(short width,short height,long size,int format)
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
      Print_in_window( 59,59,File_formats[format-1].Extension,MC_Black,MC_Light);
    }

    // On efface le commentaire précédent
    Window_rectangle(45,70,32*8,8,MC_Light);
    // Affichage du commentaire
    if (File_formats[format-1].Comment)
      Print_in_window(45,70,Main_comment,MC_Black,MC_Light);

    // Calculs des données nécessaires à l'affichage de la preview:
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
      if (Backup_with_new_dimensions(0,width,height))
      {
        // La nouvelle page a pu être allouée, elle est pour l'instant pleine
        // de 0s. Elle fait Main_image_width de large.
        // Normalement tout va bien, tout est sous contrôle...
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



void Draw_palette_preview(void)
{
  short index;

  if (Pixel_load_function==Pixel_load_in_preview)
    for (index=0; index<256; index++)
      Window_rectangle(183+(index/16)*7,95+(index&15)*5,5,5,index);

  Update_window_area(183,95,120,80);
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
  int  index; // index de balayage des formats
  int  format=0; // Format du fichier à charger


  // On place par défaut File_error à vrai au cas où on ne sache pas
  // charger le format du fichier:
  File_error=1;

  if (Main_format!=0)
  {
    File_formats[Main_format-1].Test();
    if (!File_error)
      // Si dans le sélecteur il y a un format valide on le prend tout de suite
      format=Main_format-1;
  }

  if (File_error)
  {
    //  Sinon, on va devoir scanner les différents formats qu'on connait pour
    // savoir à quel format est le fichier:
    for (index=0;index<NB_FORMATS_LOAD;index++)
    {
      // On appelle le testeur du format:
      File_formats[index].Test();
      // On s'arrête si le fichier est au bon format:
      if (File_error==0)
      {
        format=index;
        break;
      }
    }
  }

  // Si on a su déterminer avec succès le format du fichier:
  if (!File_error)
  {
    // On peut charger le fichier:
    Image_24b=0;
    Ratio_of_loaded_image=PIXEL_SIMPLE;
    // Dans certains cas il est possible que le chargement plante
    // après avoir modifié la palette. TODO
    File_formats[format].Load();

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
			if (Convert_24b_bitmap_to_256(Brush, Buffer_image_24b, Brush_width,
				Brush_height, Main_palette))
				File_error = 2;
        }
        //if (!File_error)
        // Palette_256_to_64(Main_palette);
        // Normalement plus besoin car 256 color natif, et c'etait probablement
        // un bug - yr
      }

	  free(Buffer_image_24b);
	}

	if (image)
	{
      if ( (File_error!=1) && (File_formats[format].Backup_done) )
      {
        if (Pixel_load_function==Pixel_load_in_preview)
        {
          dword color_usage[256];
          Count_used_colors_screen_area(color_usage, Preview_pos_X,
			Preview_pos_Y, Main_image_width / Preview_factor_X,
			Main_image_height / Preview_factor_Y);
          //Count_used_colors(color_usage);
          Display_cursor();
          Set_nice_menu_colors(color_usage, 1);
          Hide_cursor();
        }
      
        // On considère que l'image chargée n'est plus modifiée
        Main_image_is_modified = 0;
        // Et on documente la variable Main_fileformat avec la valeur:
        Main_fileformat = format + 1;

        // Correction des dimensions
        if (Main_image_width<1)
          Main_image_width = 1;
        if (Main_image_height<1)
          Main_image_height = 1;
      }
      else if (File_error!=1)
      {
        // On considère que l'image chargée est encore modifiée
        Main_image_is_modified = 1;
        // Et on documente la variable Main_fileformat avec la valeur:
        Main_fileformat = format + 1;
      }
      else
      {
        // Dans ce cas, on sait que l'image n'a pas changé, mais ses
        // paramètres (dimension, palette, ...) si. Donc on les restaures.
        Download_infos_page_main(Main_backups->Pages);
      }
    }
  }
  else
    // Sinon, l'appelant sera au courant de l'échec grace à File_error;
    // et si on s'apprêtait à faire un chargement définitif de l'image (pas
    // une preview), alors on flash l'utilisateur.
    if (Pixel_load_function!=Pixel_load_in_preview)
      Error(0);
}


// -- Sauver n'importe quel type connu de fichier d'image (ou palette) ------
void Save_image(byte image)
{
  // On place par défaut File_error à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  File_error=1;

  Read_pixel_function=(image)?Read_pixel_from_current_screen:Read_pixel_from_brush;

  File_formats[Main_fileformat-1].Save();

  if (File_error)
    Error(0);
  else
  {
    if ((image) && (File_formats[Main_fileformat-1].Backup_done))
      Main_image_is_modified=0;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PAL ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// -- Tester si un fichier est au format PAL --------------------------------
void Test_PAL(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  long file_size;   // Taille du fichier

  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture de la taille du fichier
    file_size=File_length_file(file);
    fclose(file);
    // Le fichier ne peut être au format PAL que si sa taille vaut 768 octets
    if (file_size==sizeof(T_Palette))
      File_error=0;
  }
}


// -- Lire un fichier au format PAL -----------------------------------------
void Load_PAL(void)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  //long  file_size;   // Taille du fichier


  Get_full_filename(filename,0);
  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    T_Palette palette_64;
    // Init_preview(???); // Pas possible... pas d'image...

    // Lecture du fichier dans Main_palette
    if (Read_bytes(file,palette_64,sizeof(T_Palette)))
    {
      Palette_64_to_256(palette_64);
      memcpy(Main_palette,palette_64,sizeof(T_Palette));
      Set_palette(Main_palette);
      Remap_fileselector();

      // On dessine une preview de la palette (si chargement=preview)
      Draw_palette_preview();
    }
    else
      File_error=2;

    // Fermeture du fichier
    fclose(file);
  }
  else
    // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
    File_error=1;
}


// -- Sauver un fichier au format PAL ---------------------------------------
void Save_PAL(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  //long file_size;   // Taille du fichier

  Get_full_filename(filename,0);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    T_Palette palette_64;
    memcpy(palette_64,Main_palette,sizeof(T_Palette));
    Palette_256_to_64(palette_64);
    // Enregistrement de Main_palette dans le fichier
    if (! Write_bytes(file,palette_64,sizeof(T_Palette)))
    {
      File_error=1;
      fclose(file);
      remove(filename);
    }
    else // Ecriture correcte => Fermeture normale du fichier
      fclose(file);
  }
  else // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
  {
    File_error=1;
    fclose(file);
    remove(filename);
                     //   On se fout du résultat de l'opération car si ça
                     // renvoie 0 c'est que le fichier avait été partiel-
                     // -lement écrit, sinon pas du tout. Or dans tous les
                     // cas ça revient au même pour nous: Sauvegarde ratée!
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// IMG ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  byte Filler1[6];
  word Width;
  word Height;
  byte Filler2[118];
  T_Palette Palette;
} __attribute__((__packed__)) T_IMG_Header;

// -- Tester si un fichier est au format IMG --------------------------------
void Test_IMG(void)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_IMG_Header IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};


  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture et vérification de la signature
    if (Read_bytes(file,&IMG_header,sizeof(T_IMG_Header)))
    {
      if ( (!memcmp(IMG_header.Filler1,signature,6))
        && IMG_header.Width && IMG_header.Height)
        File_error=0;
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format IMG -----------------------------------------
void Load_IMG(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte * buffer;
  FILE *file;
  word x_pos,y_pos;
  long width_read;
  long file_size;
  T_IMG_Header IMG_header;

  Get_full_filename(filename,0);
  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,&IMG_header,sizeof(T_IMG_Header)))
    {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    IMG_header.Width = SDL_Swap16(IMG_header.Width);
    IMG_header.Height = SDL_Swap16(IMG_header.Height);
#endif

      buffer=(byte *)malloc(IMG_header.Width);

      Init_preview(IMG_header.Width,IMG_header.Height,file_size,FORMAT_IMG);
      if (File_error==0)
      {
        memcpy(Main_palette,IMG_header.Palette,sizeof(T_Palette));
        Set_palette(Main_palette);
        Remap_fileselector();

        Main_image_width=IMG_header.Width;
        Main_image_height=IMG_header.Height;
        width_read=IMG_header.Width;

        for (y_pos=0;(y_pos<Main_image_height) && (!File_error);y_pos++)
        {
          if (Read_bytes(file,buffer,Main_image_width))
          {
            for (x_pos=0; x_pos<Main_image_width;x_pos++)
              Pixel_load_function(x_pos,y_pos,buffer[x_pos]);
          }
          else
            File_error=2;
        }
      }

      free(buffer);
    }
    else
      File_error=1;

    fclose(file);
  }
  else
    File_error=1;
}

// -- Sauver un fichier au format IMG ---------------------------------------
void Save_IMG(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_IMG_Header IMG_header;
  byte signature[6]={0x01,0x00,0x47,0x12,0x6D,0xB0};

  Get_full_filename(filename,0);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    memcpy(IMG_header.Filler1,signature,6);

    IMG_header.Width=Main_image_width;
    IMG_header.Height=Main_image_height;

    memset(IMG_header.Filler2,0,118);
    IMG_header.Filler2[4]=0xFF;
    IMG_header.Filler2[22]=64; // Lo(Longueur de la signature)
    IMG_header.Filler2[23]=0;  // Hi(Longueur de la signature)
    memcpy(IMG_header.Filler2+23,"GRAFX2 by SunsetDesign (IMG format taken from PV (c)W.Wiedmann)",64);

    memcpy(IMG_header.Palette,Main_palette,sizeof(T_Palette));

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    IMG_header.Width = SDL_Swap16(IMG_header.Width);
    IMG_header.Height = SDL_Swap16(IMG_header.Height);
#endif

    if (Write_bytes(file,&IMG_header,sizeof(T_IMG_Header)))
    {
      Init_write_buffer();

      for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
        for (x_pos=0; x_pos<Main_image_width; x_pos++)
          Write_one_byte(file,Read_pixel_function(x_pos,y_pos));

      End_write(file);
      fclose(file);

      if (File_error)
        remove(filename);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PKM ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  char Ident[3]; // Chaîne "PKM" }
  byte Method;  // Compression method
                 //   0 = compression en ligne (c)KM
                 //   autres = inconnues pour le moment
  byte recog1;   // Octet de reconnaissance sur 1 octet  }
  byte recog2;   // Octet de reconnaissance sur 2 octets }
  word Width;  // width de l'image
  word Height;  // height de l'image
  T_Palette Palette; // Palette RVB 256*3
  word Jump;     // Taille du saut entre le header et l'image:
                 //   On va s'en servir pour rajouter un commentaire
} __attribute__((__packed__)) T_PKM_Header;

// -- Tester si un fichier est au format PKM --------------------------------
void Test_PKM(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_PKM_Header header;


  Get_full_filename(filename,0);
  
  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture du header du fichier
    if (Read_bytes(file,&header,sizeof(T_PKM_Header)))
    {
      // On regarde s'il y a la signature PKM suivie de la méthode 0.
      // La constante "PKM" étant un chaîne, elle se termine toujours par 0.
      // Donc pas la peine de s'emm...er à regarder si la méthode est à 0.
      if ( (!memcmp(&header,"PKM",4)) && header.Width && header.Height)
        File_error=0;
    }
    fclose(file);
  }
}


// -- Lire un fichier au format PKM -----------------------------------------
void Load_PKM(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_PKM_Header header;
  byte  color;
  byte  temp_byte;
  word  len;
  word  index;
  dword Compteur_de_pixels;
  dword Compteur_de_donnees_packees;
  dword image_size;
  dword Taille_pack;
  long  file_size;

  Get_full_filename(filename,0);

  File_error=0;
  
  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,&header,sizeof(T_PKM_Header)))
    {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Width = SDL_Swap16(header.Width);
    header.Height = SDL_Swap16(header.Height);
    header.Jump = SDL_Swap16(header.Jump);
#endif

      Main_comment[0]='\0'; // On efface le commentaire
      if (header.Jump)
      {
        index=0;
        while ( (index<header.Jump) && (!File_error) )
        {
          if (Read_byte(file,&temp_byte))
          {
            index+=2; // On rajoute le "Field-id" et "le Field-size" pas encore lu
            switch (temp_byte)
            {
              case 0 : // Commentaire
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte>COMMENT_SIZE)
                  {
                    color=temp_byte;              // On se sert de color comme
                    temp_byte=COMMENT_SIZE;   // variable temporaire
                    color-=COMMENT_SIZE;
                  }
                  else
                    color=0;

                  if (Read_bytes(file,Main_comment,temp_byte))
                  {
                    index+=temp_byte;
                    Main_comment[temp_byte]='\0';
                    if (color)
                      if (fseek(file,color,SEEK_CUR))
                        File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              case 1 : // Dimensions de l'écran d'origine
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte==4)
                  {
                    index+=4;
                    if ( ! Read_word_le(file,(word *) &Original_screen_X)
                      || !Read_word_le(file,(word *) &Original_screen_Y) )
                      File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              case 2 : // color de transparence
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte==1)
                  {
                    index++;
                    if (! Read_byte(file,&Back_color))
                      File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              default:
                if (Read_byte(file,&temp_byte))
                {
                  index+=temp_byte;
                  if (fseek(file,temp_byte,SEEK_CUR))
                    File_error=2;
                }
                else
                  File_error=2;
            }
          }
          else
            File_error=2;
        }
        if ( (!File_error) && (index!=header.Jump) )
          File_error=2;
      }

      /*Init_lecture();*/

      if (!File_error)
      {
        Init_preview(header.Width,header.Height,file_size,FORMAT_PKM);
        if (File_error==0)
        {
          
          Main_image_width=header.Width;
          Main_image_height=header.Height;
          image_size=(dword)(Main_image_width*Main_image_height);
          // Palette lue en 64
          memcpy(Main_palette,header.Palette,sizeof(T_Palette));
          Palette_64_to_256(Main_palette);
          Set_palette(Main_palette);
          Remap_fileselector();

          Compteur_de_donnees_packees=0;
          Compteur_de_pixels=0;
          Taille_pack=(file_size)-sizeof(T_PKM_Header)-header.Jump;

          // Boucle de décompression:
          while ( (Compteur_de_pixels<image_size) && (Compteur_de_donnees_packees<Taille_pack) && (!File_error) )
          {
            if(Read_byte(file, &temp_byte)!=1) 
			{
				File_error=2;
				break;
			}

            // Si ce n'est pas un octet de reconnaissance, c'est un pixel brut
            if ( (temp_byte!=header.recog1) && (temp_byte!=header.recog2) )
            {
              Pixel_load_function(Compteur_de_pixels % Main_image_width,
                                  Compteur_de_pixels / Main_image_width,
                                  temp_byte);
              Compteur_de_donnees_packees++;
              Compteur_de_pixels++;
            }
            else // Sinon, On regarde si on va décompacter un...
            { // ... nombre de pixels tenant sur un byte
              if (temp_byte==header.recog1)
              {
                if(Read_byte(file, &color)!=1)
				{
					File_error=2;
					break;
				}
                if(Read_byte(file, &temp_byte)!=1)
				{
					File_error=2;
					break;
				}
                for (index=0; index<temp_byte; index++)
                  Pixel_load_function((Compteur_de_pixels+index) % Main_image_width,
                                      (Compteur_de_pixels+index) / Main_image_width,
                                      color);
                Compteur_de_pixels+=temp_byte;
                Compteur_de_donnees_packees+=3;
              }
              else // ... nombre de pixels tenant sur un word
              {
                if(Read_byte(file, &color)!=1)
				{
					File_error=2;
					break;
				}
                Read_word_be(file, &len);
                for (index=0; index<len; index++)
                  Pixel_load_function((Compteur_de_pixels+index) % Main_image_width,
                                      (Compteur_de_pixels+index) / Main_image_width,
                                      color);
                Compteur_de_pixels+=len;
                Compteur_de_donnees_packees+=4;
              }
            }
          }
        }
      }
      /*Close_lecture();*/
    }
    else // Lecture header impossible: Error ne modifiant pas l'image
      File_error=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Error ne modifiant pas l'image
    File_error=1;
}


// -- Sauver un fichier au format PKM ---------------------------------------

  // Trouver quels sont les octets de reconnaissance
  void Find_recog(byte * recog1, byte * recog2)
  {
    dword Find_recon[256]; // Table d'utilisation de couleurs
    byte  best;   // Meilleure couleur pour recon (recon1 puis recon2)
    dword NBest;  // Nombre d'occurences de cette couleur
    word  index;


    // On commence par compter l'utilisation de chaque couleurs
    Count_used_colors(Find_recon);

    // Ensuite recog1 devient celle la moins utilisée de celles-ci
    *recog1=0;
    best=1;
    NBest=INT_MAX; // Une même couleur ne pourra jamais être utilisée 1M de fois.
    for (index=1;index<=255;index++)
      if (Find_recon[index]<NBest)
      {
        best=index;
        NBest=Find_recon[index];
      }
    *recog1=best;

    // Enfin recog2 devient la 2ème moins utilisée
    *recog2=0;
    best=0;
    NBest=INT_MAX;
    for (index=0;index<=255;index++)
      if ( (Find_recon[index]<NBest) && (index!=*recog1) )
      {
        best=index;
        NBest=Find_recon[index];
      }
    *recog2=best;
  }


void Save_PKM(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_PKM_Header header;
  dword Compteur_de_pixels;
  dword image_size;
  word  repetitions;
  byte  last_color;
  byte  pixel_value;
  byte  comment_size;



  // Construction du header
  memcpy(header.Ident,"PKM",3);
  header.Method=0;
  Find_recog(&header.recog1,&header.recog2);
  header.Width=Main_image_width;
  header.Height=Main_image_height;
  memcpy(header.Palette,Main_palette,sizeof(T_Palette));
  Palette_256_to_64(header.Palette);

  // Calcul de la taille du Post-header
  header.Jump=9; // 6 pour les dimensions de l'ecran + 3 pour la back-color
  comment_size=strlen(Main_comment);
  if (comment_size)
    header.Jump+=comment_size+2;


  Get_full_filename(filename,0);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Width = SDL_Swap16(header.Width);
    header.Height = SDL_Swap16(header.Height);
    header.Jump = SDL_Swap16(header.Jump);
#endif

    // Ecriture du header
    if (Write_bytes(file,&header,sizeof(T_PKM_Header)))
    {
      Init_write_buffer();

      // Ecriture du commentaire
      // (Compteur_de_pixels est utilisé ici comme simple index de comptage)
      if (comment_size)
      {
        Write_one_byte(file,0);
        Write_one_byte(file,comment_size);
        for (Compteur_de_pixels=0; Compteur_de_pixels<comment_size; Compteur_de_pixels++)
          Write_one_byte(file,Main_comment[Compteur_de_pixels]);
      }
      // Ecriture des dimensions de l'écran
      Write_one_byte(file,1);
      Write_one_byte(file,4);
      Write_one_byte(file,Screen_width&0xFF);
      Write_one_byte(file,Screen_width>>8);
      Write_one_byte(file,Screen_height&0xFF);
      Write_one_byte(file,Screen_height>>8);
      // Ecriture de la back-color
      Write_one_byte(file,2);
      Write_one_byte(file,1);
      Write_one_byte(file,Back_color);

      // Routine de compression PKM de l'image
      image_size=(dword)(Main_image_width*Main_image_height);
      Compteur_de_pixels=0;
      pixel_value=Read_pixel_function(0,0);

      while ( (Compteur_de_pixels<image_size) && (!File_error) )
      {
        Compteur_de_pixels++;
        repetitions=1;
        last_color=pixel_value;
        if(Compteur_de_pixels<image_size)
        {
          pixel_value=Read_pixel_function(Compteur_de_pixels % Main_image_width,Compteur_de_pixels / Main_image_width);
        }
        while ( (pixel_value==last_color)
             && (Compteur_de_pixels<image_size)
             && (repetitions<65535) )
        {
          Compteur_de_pixels++;
          repetitions++;
          if(Compteur_de_pixels>=image_size) break;
          pixel_value=Read_pixel_function(Compteur_de_pixels % Main_image_width,Compteur_de_pixels / Main_image_width);
        }

        if ( (last_color!=header.recog1) && (last_color!=header.recog2) )
        {
          if (repetitions==1)
            Write_one_byte(file,last_color);
          else
          if (repetitions==2)
          {
            Write_one_byte(file,last_color);
            Write_one_byte(file,last_color);
          }
          else
          if ( (repetitions>2) && (repetitions<256) )
          { // RECON1/couleur/nombre
            Write_one_byte(file,header.recog1);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions&0xFF);
          }
          else
          if (repetitions>=256)
          { // RECON2/couleur/hi(nombre)/lo(nombre)
            Write_one_byte(file,header.recog2);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions>>8);
            Write_one_byte(file,repetitions&0xFF);
          }
        }
        else
        {
          if (repetitions<256)
          {
            Write_one_byte(file,header.recog1);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions&0xFF);
          }
          else
          {
            Write_one_byte(file,header.recog2);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions>>8);
            Write_one_byte(file,repetitions&0xFF);
          }
        }
      }

      End_write(file);
    }
    else
      File_error=1;
    fclose(file);
  }
  else
  {
    File_error=1;
    fclose(file);
  }
  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri traîner... Ca fait pas propre.
  if (File_error)
    remove(filename);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LBM ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word  Width;
  word  Height;
  word  X_org;       // Inutile
  word  Y_org;       // Inutile
  byte  BitPlanes;
  byte  Mask;
  byte  Compression;
  byte  Pad1;       // Inutile
  word  Transp_col;
  byte  X_aspect;    // Inutile
  byte  Y_aspect;    // Inutile
  word  X_screen;
  word  Y_screen;
} __attribute__((__packed__)) T_LBM_Header;

byte * LBM_buffer;
FILE *LBM_file;

// -- Tester si un fichier est au format LBM --------------------------------

void Test_LBM(void)
{
  char filename[MAX_PATH_CHARACTERS];
  char  format[4];
  char  section[4];
  dword dummy;

  Get_full_filename(filename,0);

  File_error=0;

  if ((LBM_file=fopen(filename, "rb")))
  {
    if (! Read_bytes(LBM_file,section,4))
      File_error=1;
    else
    if (memcmp(section,"FORM",4))
      File_error=1;
    else
    {
      Read_dword_be(LBM_file, &dummy);
                   //   On aurait pu vérifier que ce long est égal à la taille
                   // du fichier - 8, mais ça aurait interdit de charger des
                   // fichiers tronqués (et déjà que c'est chiant de perdre
                   // une partie du fichier il faut quand même pouvoir en
                   // garder un peu... Sinon, moi je pleure :'( !!! )
      if (! Read_bytes(LBM_file,format,4))
        File_error=1;
      else
      if ( (memcmp(format,"ILBM",4)) && (memcmp(format,"PBM ",4)) )
        File_error=1;
    }
    fclose(LBM_file);
  }
  else
    File_error=1;
}


// -- Lire un fichier au format LBM -----------------------------------------

  byte Image_HAM;

  // ---------------- Adapter la palette pour les images HAM ----------------
  void Adapt_palette_HAM(void)
  {
    short i,j,temp;
    byte  color;

    if (Image_HAM==6)
    {
      for (i=1; i<=14; i++)
      {
        // On recopie a palette de base
        memcpy(Main_palette+(i<<4),Main_palette,48);
        // On modifie les teintes de cette palette
        for (j=0; j<16; j++)
        {
          color=(i<<4)+j;
          if (i<=7)
          {
            if (i&1)
            {
              temp=Main_palette[j].R+16;
              Main_palette[color].R=(temp<63)?temp:63;
            }
            if (i&2)
            {
              temp=Main_palette[j].G+16;
              Main_palette[color].G=(temp<63)?temp:63;
            }
            if (i&4)
            {
              temp=Main_palette[j].B+16;
              Main_palette[color].B=(temp<63)?temp:63;
            }
          }
          else
          {
            if ((i-7)&1)
            {
              temp=Main_palette[j].R-16;
              Main_palette[color].R=(temp>=0)?temp:0;
            }
            if ((i-7)&2)
            {
              temp=Main_palette[j].G-16;
              Main_palette[color].G=(temp>=0)?temp:0;
            }
            if ((i-7)&4)
            {
              temp=Main_palette[j].B-16;
              Main_palette[color].B=(temp>=0)?temp:0;
            }
          }
        }
      }
      // Ici, il reste les 16 dernières couleurs à modifier
      for (i=240,j=0; j<16; i++,j++)
      {
        temp=Main_palette[j].R+8;
        Main_palette[i].R=(temp<63)?temp:63;
        temp=Main_palette[j].G+8;
        Main_palette[i].G=(temp<63)?temp:63;
        temp=Main_palette[j].B+8;
        Main_palette[i].B=(temp<63)?temp:63;
      }
    }
    else if (Image_HAM==8)
    {
      for (i=1; i<=3; i++)
      {
        // On recopie la palette de base
        memcpy(Main_palette+(i<<6),Main_palette,192);
        // On modifie les teintes de cette palette
        for (j=0; j<64; j++)
        {
          color=(i<<6)+j;
          switch (i)
          {
            case 1 :
              temp=Main_palette[j].R+16;
              Main_palette[color].R=(temp<63)?temp:63;
              break;
            case 2 :
              temp=Main_palette[j].G+16;
              Main_palette[color].G=(temp<63)?temp:63;
              break;
            default:
              temp=Main_palette[j].B+16;
              Main_palette[color].B=(temp<63)?temp:63;
          }
        }
      }
    }
    else // Image 64 couleurs sauvée en 32.
    {
      for (i=0; i<32; i++)
      {
        j=i+32;
        Main_palette[j].R=Main_palette[i].R>>1;
        Main_palette[j].G=Main_palette[i].G>>1;
        Main_palette[j].B=Main_palette[i].B>>1;
      }
    }
  }

  // ------------------------- Attendre une section -------------------------
  byte Wait_for(byte * expected_section)
  {
    // Valeur retournée: 1=Section trouvée, 0=Section non trouvée (erreur)
    dword Taille_section;
    byte section_read[4];

    if (! Read_bytes(LBM_file,section_read,4))
      return 0;
    while (memcmp(section_read,expected_section,4)) // Sect. pas encore trouvée
    {
      if (!Read_dword_be(LBM_file,&Taille_section))
        return 0;
      if (Taille_section&1)
        Taille_section++;
      if (fseek(LBM_file,Taille_section,SEEK_CUR))
        return 0;
      if (! Read_bytes(LBM_file,section_read,4))
        return 0;
    }
    return 1;
  }

// Les images ILBM sont stockés en bitplanes donc on doit trifouiller les bits pour
// en faire du chunky

byte Color_ILBM_line(word x_pos, word real_line_size, byte HBPm1)
{
  // Renvoie la couleur du pixel (ILBM) en x_pos.
  // CL sera le rang auquel on extrait les bits de la couleur
  byte cl = 7 - (x_pos & 7);
  int ax,bh,dx;
  byte bl=0;

  for(dx=HBPm1;dx>=0;dx--)
  {
  //CIL_Loop
    ax = (real_line_size * dx + x_pos) >> 3;
    bh = (LBM_buffer[ax] >> cl) & 1;

    bl = (bl << 1) + bh;
  }

  return bl;
}

  // ----------------------- Afficher une ligne ILBM ------------------------
  void Draw_ILBM_line(short y_pos, short real_line_size)
  {
    byte  color;
    byte  red,green,blue;
    byte  temp;
    short x_pos;

    if (Image_HAM<=1)                                               // ILBM
    {
      for (x_pos=0; x_pos<Main_image_width; x_pos++)
      {
        Pixel_load_function(x_pos,y_pos,Color_ILBM_line(x_pos,real_line_size, HBPm1));
      }
    }
    else
    {
      color=0;
      red=Main_palette[0].R;
      green =Main_palette[0].G;
      blue =Main_palette[0].B;
      if (Image_HAM==6)
      for (x_pos=0; x_pos<Main_image_width; x_pos++)         // HAM6
      {
        temp=Color_ILBM_line(x_pos,real_line_size, HBPm1);
        switch (temp & 0xF0)
        {
          case 0x10: // blue
            blue=(temp&0x0F)<<2;
            color=Best_color(red,green,blue);
            break;
          case 0x20: // red
            red=(temp&0x0F)<<2;
            color=Best_color(red,green,blue);
            break;
          case 0x30: // green
            green=(temp&0x0F)<<2;
            color=Best_color(red,green,blue);
            break;
          default:   // Nouvelle couleur
            color=temp;
            red=Main_palette[color].R;
            green =Main_palette[color].G;
            blue =Main_palette[color].B;
        }
        Pixel_load_function(x_pos,y_pos,color);
      }
      else
      for (x_pos=0; x_pos<Main_image_width; x_pos++)         // HAM8
      {
        temp=Color_ILBM_line(x_pos,real_line_size, HBPm1);
        switch (temp & 0x03)
        {
          case 0x01: // blue
            blue=temp>>2;
            color=Best_color(red,green,blue);
            break;
          case 0x02: // red
            red=temp>>2;
            color=Best_color(red,green,blue);
            break;
          case 0x03: // green
            green=temp>>2;
            color=Best_color(red,green,blue);
            break;
          default:   // Nouvelle couleur
            color=temp;
            red=Main_palette[color].R;
            green =Main_palette[color].G;
            blue =Main_palette[color].B;
        }
        Pixel_load_function(x_pos,y_pos,color);
      }
    }
  }


void Load_LBM(void)
{
  char filename[MAX_PATH_CHARACTERS];
  T_LBM_Header header;
  char  format[4];
  char  section[4];
  byte  temp_byte;
  short b256;
  dword nb_colors;
  dword image_size;
  short x_pos;
  short y_pos;
  short counter;
  short line_size;       // Taille d'une ligne en octets
  short real_line_size; // Taille d'une ligne en pixels
  byte  color;
  long  file_size;
  dword dummy;

  Get_full_filename(filename,0);

  File_error=0;

  if ((LBM_file=fopen(filename, "rb")))
  {
    file_size=File_length_file(LBM_file);

    // On avance dans le fichier (pas besoin de tester ce qui l'a déjà été)
    Read_bytes(LBM_file,section,4);
    Read_dword_be(LBM_file,&dummy);
    Read_bytes(LBM_file,format,4);
    if (!Wait_for((byte *)"BMHD"))
      File_error=1;
    Read_dword_be(LBM_file,&dummy);

    // Maintenant on lit le header pour pouvoir commencer le chargement de l'image
    if ( (Read_word_be(LBM_file,&header.Width))
      && (Read_word_be(LBM_file,&header.Height))
      && (Read_word_be(LBM_file,&header.X_org))
      && (Read_word_be(LBM_file,&header.Y_org))
      && (Read_byte(LBM_file,&header.BitPlanes))
      && (Read_byte(LBM_file,&header.Mask))
      && (Read_byte(LBM_file,&header.Compression))
      && (Read_byte(LBM_file,&header.Pad1))
      && (Read_word_be(LBM_file,&header.Transp_col))
      && (Read_byte(LBM_file,&header.X_aspect))
      && (Read_byte(LBM_file,&header.Y_aspect))
      && (Read_word_be(LBM_file,&header.X_screen))
      && (Read_word_be(LBM_file,&header.Y_screen))
      && header.Width && header.Height)
    {
      if ( (header.BitPlanes) && (Wait_for((byte *)"CMAP")) )
      {
        Read_dword_be(LBM_file,&nb_colors);
        nb_colors/=3;

        if (((dword)1<<header.BitPlanes)!=nb_colors)
        {
          if ((nb_colors==32) && (header.BitPlanes==6))
          {              // Ce n'est pas une image HAM mais une image 64 coul.
            Image_HAM=1; // Sauvée en 32 coul. => il faut copier les 32 coul.
          }              // sur les 32 suivantes et assombrir ces dernières.
          else
          {
            if ((header.BitPlanes==6) || (header.BitPlanes==8))
              Image_HAM=header.BitPlanes;
            else
              /* File_error=1;*/  /* C'est censé être incorrect mais j'ai */
              Image_HAM=0;            /* trouvé un fichier comme ça, alors... */
          }
        }
        else
          Image_HAM=0;

        if ( (!File_error) && (nb_colors>=2) && (nb_colors<=256) )
        {
          HBPm1=header.BitPlanes-1;
          if (header.Mask==1)
            header.BitPlanes++;

          // Deluxe paint le fait... alors on le fait...
          Back_color=header.Transp_col;

          // On commence par passer la palette en 256 comme ça, si la nouvelle
          // palette a moins de 256 coul, la précédente ne souffrira pas d'un
          // assombrissement préjudiciable.
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));
          else
            Palette_64_to_256(Main_palette);
          //   On peut maintenant charger la nouvelle palette
          if (Read_bytes(LBM_file,Main_palette,3*nb_colors))
          {
            Palette_256_to_64(Main_palette);
            if (Image_HAM)
              Adapt_palette_HAM();
            Palette_64_to_256(Main_palette);
            Set_palette(Main_palette);
            Remap_fileselector();

            // On lit l'octet de padding du CMAP si la taille est impaire
            if (nb_colors&1)
              if (Read_byte(LBM_file,&temp_byte))
                File_error=2;

            if ( (Wait_for((byte *)"BODY")) && (!File_error) )
            {
              Read_dword_be(LBM_file,&image_size);
              //swab((char *)&header.Width ,(char *)&Main_image_width,2);
              //swab((char *)&header.Height,(char *)&Main_image_height,2);
              Main_image_width = header.Width;
              Main_image_height = header.Height;

              //swab((char *)&header.X_screen,(char *)&Original_screen_X,2);
              //swab((char *)&header.Y_screen,(char *)&Original_screen_Y,2);
              Original_screen_X = header.X_screen;
              Original_screen_Y = header.Y_screen;

              Init_preview(Main_image_width,Main_image_height,file_size,FORMAT_LBM);
              if (File_error==0)
              {
                if (!memcmp(format,"ILBM",4))    // "ILBM": InterLeaved BitMap
                {
                  // Calcul de la taille d'une ligne ILBM (pour les images ayant des dimensions exotiques)
                  if (Main_image_width & 15)
                  {
                    real_line_size=( (Main_image_width+16) >> 4 ) << 4;
                    line_size=( (Main_image_width+16) >> 4 )*(header.BitPlanes<<1);
                  }
                  else
                  {
                    real_line_size=Main_image_width;
                    line_size=(Main_image_width>>3)*header.BitPlanes;
                  }

                  if (!header.Compression)
                  {                                           // non compressé
                    LBM_buffer=(byte *)malloc(line_size);
                    for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
                    {
                      if (Read_bytes(LBM_file,LBM_buffer,line_size))
                        Draw_ILBM_line(y_pos,real_line_size);
                      else
                        File_error=2;
                    }
                    free(LBM_buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/

                    LBM_buffer=(byte *)malloc(line_size);

                    for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
                    {
                      for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
                      {
                        if(Read_byte(LBM_file, &temp_byte)!=1)
						{
							File_error=2;
							break;
						}
                        // Si temp_byte > 127 alors il faut répéter 256-'temp_byte' fois la couleur de l'octet suivant
                        // Si temp_byte <= 127 alors il faut afficher directement les 'temp_byte' octets suivants
                        if (temp_byte>127)
                        {
							if(Read_byte(LBM_file, &color)!=1)
							{
								File_error=2;
								break;
							}
                          b256=(short)(256-temp_byte);
                          for (counter=0; counter<=b256; counter++)
                            if (x_pos<line_size)
                              LBM_buffer[x_pos++]=color;
                            else
                              File_error=2;
                        }
                        else
                          for (counter=0; counter<=(short)(temp_byte); counter++)
                            if (x_pos>=line_size || Read_byte(LBM_file, &(LBM_buffer[x_pos++]))!=1)
								File_error=2;
                      }
                      if (!File_error)
                        Draw_ILBM_line(y_pos,real_line_size);
                    }

                    free(LBM_buffer);
                    /*Close_lecture();*/
                  }
                }
                else                               // "PBM ": Planar(?) BitMap
                {
                  real_line_size=Main_image_width+(Main_image_width&1);

                  if (!header.Compression)
                  {                                           // non compressé
                    LBM_buffer=(byte *)malloc(real_line_size);
                    for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
                    {
                      if (Read_bytes(LBM_file,LBM_buffer,real_line_size))
                        for (x_pos=0; x_pos<Main_image_width; x_pos++)
                          Pixel_load_function(x_pos,y_pos,LBM_buffer[x_pos]);
                      else
                        File_error=2;
                    }
                    free(LBM_buffer);
                  }
                  else
                  {                                               // compressé
                    /*Init_lecture();*/
                    for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
                    {
                      for (x_pos=0; ((x_pos<real_line_size) && (!File_error)); )
                      {
                        if(Read_byte(LBM_file, &temp_byte)!=1)
						{
							File_error=2;
							break;
						}
                        if (temp_byte>127)
                        {
                          if(Read_byte(LBM_file, &color)!=1)
						  {
							File_error=2;
							break;
						  }
                          b256=256-temp_byte;
                          for (counter=0; counter<=b256; counter++)
                            Pixel_load_function(x_pos++,y_pos,color);
                        }
                        else
                          for (counter=0; counter<=temp_byte; counter++)
                          {
                            byte byte_read=0;
                            if(Read_byte(LBM_file, &byte_read)!=1)
							{
								File_error=2;
								break;
							}
                            Pixel_load_function(x_pos++,y_pos,byte_read);
                          }
                      }
                    }
                    /*Close_lecture();*/
                  }
                }
              }
            }
            else
              Set_file_error(2);
          }
          else
          {
            File_error=1;
          }
        }
        else
          Set_file_error(1);
      }
      else
        File_error=1;
    }
    else
      File_error=1;

    fclose(LBM_file);
  }
  else
    File_error=1;
}


// -- Sauver un fichier au format LBM ---------------------------------------

  byte LBM_color_list[129];
  word LBM_list_size;
  byte LBM_repetition_mode;

  // ------------- Ecrire les couleurs que l'on vient de traiter ------------
  void Transfer_colors(void)
  {
    byte index;

    if (LBM_list_size>0)
    {
      if (LBM_repetition_mode)
      {
        Write_one_byte(LBM_file,257-LBM_list_size);
        Write_one_byte(LBM_file,LBM_color_list[0]);
      }
      else
      {
        Write_one_byte(LBM_file,LBM_list_size-1);
        for (index=0; index<LBM_list_size; index++)
          Write_one_byte(LBM_file,LBM_color_list[index]);
      }
    }
    LBM_list_size=0;
    LBM_repetition_mode=0;
  }

  // - Compresion des couleurs encore plus performante que DP2e et que VPIC -
  void New_color(byte color)
  {
    byte last_color;
    byte second_last_color;

    switch (LBM_list_size)
    {
      case 0 : // Première couleur
        LBM_color_list[0]=color;
        LBM_list_size=1;
        break;
      case 1 : // Deuxième couleur
        last_color=LBM_color_list[0];
        LBM_repetition_mode=(last_color==color);
        LBM_color_list[1]=color;
        LBM_list_size=2;
        break;
      default: // Couleurs suivantes
        last_color      =LBM_color_list[LBM_list_size-1];
        second_last_color=LBM_color_list[LBM_list_size-2];
        if (last_color==color)  // On a une répétition de couleur
        {
          if ( (LBM_repetition_mode) || (second_last_color!=color) )
          // On conserve le mode...
          {
            LBM_color_list[LBM_list_size]=color;
            LBM_list_size++;
            if (LBM_list_size==128)
              Transfer_colors();
          }
          else // On est en mode <> et on a 3 couleurs qui se suivent
          {
            LBM_list_size-=2;
            Transfer_colors();
            LBM_color_list[0]=color;
            LBM_color_list[1]=color;
            LBM_color_list[2]=color;
            LBM_list_size=3;
            LBM_repetition_mode=1;
          }
        }
        else // La couleur n'est pas la même que la précédente
        {
          if (!LBM_repetition_mode)                 // On conserve le mode...
          {
            LBM_color_list[LBM_list_size++]=color;
            if (LBM_list_size==128)
              Transfer_colors();
          }
          else                                        // On change de mode...
          {
            Transfer_colors();
            LBM_color_list[LBM_list_size]=color;
            LBM_list_size++;
          }
        }
    }
  }


void Save_LBM(void)
{
  char filename[MAX_PATH_CHARACTERS];
  T_LBM_Header header;
  word x_pos;
  word y_pos;
  byte temp_byte;
  word real_width;
  int file_size;

  File_error=0;
  Get_full_filename(filename,0);

  // Ouverture du fichier
  if ((LBM_file=fopen(filename,"wb")))
  {
    Write_bytes(LBM_file,"FORM",4);
    Write_dword_be(LBM_file,0); // On mettra la taille à jour à la fin

    Write_bytes(LBM_file,"PBM BMHD",8);
    Write_dword_be(LBM_file,20);

    // On corrige la largeur de l'image pour qu'elle soit multiple de 2
    real_width=Main_image_width+(Main_image_width&1);

    //swab((byte *)&real_width,(byte *)&header.Width,2);
    header.Width=Main_image_width;
    header.Height=Main_image_height;
    header.X_org=0;
    header.Y_org=0;
    header.BitPlanes=8;
    header.Mask=0;
    header.Compression=1;
    header.Pad1=0;
    header.Transp_col=Back_color;
    header.X_aspect=1;
    header.Y_aspect=1;
    header.X_screen = Screen_width;
    header.Y_screen = Screen_height;

    Write_word_be(LBM_file,header.Width);
    Write_word_be(LBM_file,header.Height);
    Write_word_be(LBM_file,header.X_org);
    Write_word_be(LBM_file,header.Y_org);
    Write_bytes(LBM_file,&header.BitPlanes,1);
    Write_bytes(LBM_file,&header.Mask,1);
    Write_bytes(LBM_file,&header.Compression,1);
    Write_bytes(LBM_file,&header.Pad1,1);
    Write_word_be(LBM_file,header.Transp_col);
    Write_bytes(LBM_file,&header.X_aspect,1);
    Write_bytes(LBM_file,&header.Y_aspect,1);
    Write_word_be(LBM_file,header.X_screen);
    Write_word_be(LBM_file,header.Y_screen);

    Write_bytes(LBM_file,"CMAP",4);
    Write_dword_be(LBM_file,sizeof(T_Palette));

    Write_bytes(LBM_file,Main_palette,sizeof(T_Palette));

    Write_bytes(LBM_file,"BODY",4);
    Write_dword_be(LBM_file,0); // On mettra la taille à jour à la fin

    Init_write_buffer();

    LBM_list_size=0;

    for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
    {
      for (x_pos=0; ((x_pos<real_width) && (!File_error)); x_pos++)
        New_color(Read_pixel_function(x_pos,y_pos));

      if (!File_error)
        Transfer_colors();
    }

    End_write(LBM_file);
    fclose(LBM_file);

    if (!File_error)
    {
      file_size=File_length(filename);
      
      LBM_file=fopen(filename,"rb+");
      fseek(LBM_file,820,SEEK_SET);
      Write_dword_be(LBM_file,file_size-824);

      if (!File_error)
      {
        fseek(LBM_file,4,SEEK_SET);

        //   Si la taille de la section de l'image (taille fichier-8) est
        // impaire, on rajoute un 0 (Padding) à la fin.
        if ((file_size) & 1)
        {
          Write_dword_be(LBM_file,file_size-7);
          fseek(LBM_file,0,SEEK_END);
          temp_byte=0;
          if (! Write_bytes(LBM_file,&temp_byte,1))
            File_error=1;
        }
        else
          Write_dword_be(LBM_file,file_size-8);

        fclose(LBM_file);

        if (File_error)
          remove(filename);
      }
      else
      {
        File_error=1;
        fclose(LBM_file);
        remove(filename);
      }
    }
    else // Il y a eu une erreur lors du compactage => on efface le fichier
      remove(filename);
  }
  else
    File_error=1;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// BMP ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

typedef struct
{
    word  Signature;   // ='BM' = 0x4D42
    dword Size_1;    // =Taille du fichier
    word  Reserved_1;    // =0
    word  Reserved_2;    // =0
    dword Offset;    // Nb octets avant les données bitmap

    dword Size_2;    // =40 
    dword Width;
    dword Height;
    word  Planes;       // =1
    word  Nb_bits;     // =1,4,8 ou 24
    dword Compression;
    dword Size_3;
    dword XPM;
    dword YPM;
    dword Nb_Clr;
    dword Clr_Imprt;
} __attribute__((__packed__)) T_BMP_Header;

// -- Tester si un fichier est au format BMP --------------------------------
void Test_BMP(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;

  File_error=1;
  Get_full_filename(filename,0);

  if ((file=fopen(filename, "rb")))
  {
    if (Read_bytes(file,&(header.Signature),2) // "BM"
     && Read_dword_le(file,&(header.Size_1))
     && Read_word_le(file,&(header.Reserved_1))
     && Read_word_le(file,&(header.Reserved_2))
     && Read_dword_le(file,&(header.Offset))
     && Read_dword_le(file,&(header.Size_2))
     && Read_dword_le(file,&(header.Width))
     && Read_dword_le(file,&(header.Height))
     && Read_word_le(file,&(header.Planes))
     && Read_word_le(file,&(header.Nb_bits))
     && Read_dword_le(file,&(header.Compression))
     && Read_dword_le(file,&(header.Size_3))
     && Read_dword_le(file,&(header.XPM))
     && Read_dword_le(file,&(header.YPM))
     && Read_dword_le(file,&(header.Nb_Clr))
     && Read_dword_le(file,&(header.Clr_Imprt))
        )
     {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      header.Signature = SDL_Swap16(header.Signature);
#endif

      if ( (header.Signature==0x4D42) && (header.Size_2==40)
        && header.Width && header.Height )
        File_error=0;
     }
    fclose(file);
  }
}

// Find the 8 important bits in a dword
byte Bitmap_mask(dword pixel, dword mask)
{
  byte result;
  int i;
  int bits_found;

  switch(mask)
  {
    // Shortcuts to quickly handle the common 24/32bit cases
    case 0x000000FF:
      return (pixel & 0x000000FF);
    case 0x0000FF00:
      return (pixel & 0x0000FF00)>>8;
    case 0x00FF0000:
      return (pixel & 0x00FF0000)>>16;
    case 0xFF000000:
      return (pixel & 0xFF000000)>>24;
  }
  // Uncommon : do it bit by bit.
  bits_found=0;
  result=0;
  // Process the mask from low to high bit
  for (i=0;i<32;i++)
  {
    // Found a bit in the mask
    if (mask & (1<<i))
    {
      if (pixel & 1<<i)
        result |= 1<<bits_found;
        
      bits_found++;
      
      if (bits_found>=8)
        return result;
    }
  }
  // Less than 8 bits in the mask: scale the result to 8 bits
  return result << (8-bits_found);
}

// -- Charger un fichier au format BMP --------------------------------------
void Load_BMP(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;
  byte * buffer;
  word  index;
  byte  local_palette[256][4]; // R,G,B,0
  word  nb_colors =  0;
  short x_pos;
  short y_pos;
  word  line_size;
  byte  a,b,c=0;
  long  file_size;

  Get_full_filename(filename,0);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if (Read_word_le(file,&(header.Signature))
     && Read_dword_le(file,&(header.Size_1))
     && Read_word_le(file,&(header.Reserved_1))
     && Read_word_le(file,&(header.Reserved_2))
     && Read_dword_le(file,&(header.Offset))
     && Read_dword_le(file,&(header.Size_2))
     && Read_dword_le(file,&(header.Width))
     && Read_dword_le(file,&(header.Height))
     && Read_word_le(file,&(header.Planes))
     && Read_word_le(file,&(header.Nb_bits))
     && Read_dword_le(file,&(header.Compression))
     && Read_dword_le(file,&(header.Size_3))
     && Read_dword_le(file,&(header.XPM))
     && Read_dword_le(file,&(header.YPM))
     && Read_dword_le(file,&(header.Nb_Clr))
     && Read_dword_le(file,&(header.Clr_Imprt))
    )
    {
      switch (header.Nb_bits)
      {
        case 1 :
        case 4 :
        case 8 :
          if (header.Nb_Clr)
            nb_colors=header.Nb_Clr;
          else
            nb_colors=1<<header.Nb_bits;
          break;
        default:
          File_error=1;
      }

      if (!File_error)
      {
        Init_preview(header.Width,header.Height,file_size,FORMAT_BMP);
        if (File_error==0)
        {
          if (Read_bytes(file,local_palette,nb_colors<<2))
          {
            if (Config.Clear_palette)
              memset(Main_palette,0,sizeof(T_Palette));
            //   On peut maintenant transférer la nouvelle palette
            for (index=0; index<nb_colors; index++)
            {
              Main_palette[index].R=local_palette[index][2];
              Main_palette[index].G=local_palette[index][1];
              Main_palette[index].B=local_palette[index][0];
            }
            Set_palette(Main_palette);
            Remap_fileselector();

            Main_image_width=header.Width;
            Main_image_height=header.Height;

            switch (header.Compression)
            {
              case 0 : // Pas de compression
                line_size=Main_image_width;
                x_pos=(32/header.Nb_bits); // x_pos sert de variable temporaire
                if (line_size % x_pos)
                  line_size=((line_size/x_pos)*x_pos)+x_pos;
                line_size=(line_size*header.Nb_bits)>>3;

                buffer=(byte *)malloc(line_size);
                for (y_pos=Main_image_height-1; ((y_pos>=0) && (!File_error)); y_pos--)
                {
                  if (Read_bytes(file,buffer,line_size))
                    for (x_pos=0; x_pos<Main_image_width; x_pos++)
                      switch (header.Nb_bits)
                      {
                        case 8 :
                          Pixel_load_function(x_pos,y_pos,buffer[x_pos]);
                          break;
                        case 4 :
                          if (x_pos & 1)
                            Pixel_load_function(x_pos,y_pos,buffer[x_pos>>1] & 0xF);
                          else
                            Pixel_load_function(x_pos,y_pos,buffer[x_pos>>1] >> 4 );
                          break;
                        case 1 :
                          if ( buffer[x_pos>>3] & (0x80>>(x_pos&7)) )
                            Pixel_load_function(x_pos,y_pos,1);
                          else
                            Pixel_load_function(x_pos,y_pos,0);
                      }
                  else
                    File_error=2;
                }
                free(buffer);
                break;

              case 1 : // Compression RLE 8 bits
                x_pos=0;
                y_pos=Main_image_height-1;

                /*Init_lecture();*/
                if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1)
					File_error=2;
                while (!File_error)
                {
                  if (a) // Encoded mode
                    for (index=1; index<=a; index++)
                      Pixel_load_function(x_pos++,y_pos,b);
                  else   // Absolute mode
                    switch (b)
                    {
                      case 0 : // End of line
                        x_pos=0;
                        y_pos--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                		if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1)
							File_error=2;
                        x_pos+=a;
                        y_pos-=b;
                        break;
                      default: // Nouvelle série
                        while (b)
                        {
                          if(Read_byte(file, &a)!=1)
							  File_error=2;
                          //Read_one_byte(file, &c);
                          Pixel_load_function(x_pos++,y_pos,a);
                          //if (--c)
                          //{
                          //  Pixel_load_function(x_pos++,y_pos,c);
                          //  b--;
                          //}
                          b--;
                        }
                        if (ftell(file) & 1) fseek(file, 1, SEEK_CUR);
                    }
                  if (a==0 && b==1)
                    break;
                  if(Read_byte(file, &a) !=1 || Read_byte(file, &b)!=1)
				  {
					File_error=2;
				  }
                }
                /*Close_lecture();*/
                break;

              case 2 : // Compression RLE 4 bits
                x_pos=0;
                y_pos=Main_image_height-1;

                /*Init_lecture();*/
                if(Read_byte(file, &a)!=1 ||  Read_byte(file, &b) != 1)
					File_error =2;
                while ( (!File_error) && ((a)||(b!=1)) )
                {
                  if (a) // Encoded mode (A fois les 1/2 pixels de B)
                    for (index=1; index<=a; index++)
                    {
                      if (index & 1)
                        Pixel_load_function(x_pos,y_pos,b>>4);
                      else
                        Pixel_load_function(x_pos,y_pos,b&0xF);
                      x_pos++;
                    }
                  else   // Absolute mode
                    switch (b)
                    {
                      case 0 : //End of line
                        x_pos=0;
                        y_pos--;
                        break;
                      case 1 : // End of bitmap
                        break;
                      case 2 : // Delta
                       if(Read_byte(file, &a)!=1 ||  Read_byte(file, &b)!=1)
						   File_error=2;
                        x_pos+=a;
                        y_pos-=b;
                        break;
                      default: // Nouvelle série (B 1/2 pixels bruts)
                        for (index=1; ((index<=b) && (!File_error)); index++,x_pos++)
                        {
                          if (index&1)
                          {
                            if(Read_byte(file, &c)!=1) File_error=2;
                            Pixel_load_function(x_pos,y_pos,c>>4);
                          }
                          else
                            Pixel_load_function(x_pos,y_pos,c&0xF);
                        }
                        //   On lit l'octet rendant le nombre d'octets pair, si
                        // nécessaire. Encore un truc de crétin "made in MS".
                        if ( ((b&3)==1) || ((b&3)==2) )
                        {
                          byte dummy;
                          if(Read_byte(file, &dummy)!=1) File_error=2;
                        }
                    }
                  if(Read_byte(file, &a)!=1 || Read_byte(file, &b)!=1) File_error=2;
                }
                /*Close_lecture();*/
            }
            fclose(file);
          }
          else
          {
            fclose(file);
            File_error=1;
          }
        }
      }
      else
      {
        // Image 16/24/32 bits
        dword red_mask;
        dword green_mask;
        dword blue_mask;
        if (header.Nb_bits == 16)
        {
          red_mask =   0x00007C00;
          green_mask = 0x000003E0;
          blue_mask =  0x0000001F;
        }
        else
        {
          red_mask = 0x00FF0000;
          green_mask = 0x0000FF00;
          blue_mask = 0x000000FF;
        }
        File_error=0;

        Main_image_width=header.Width;
        Main_image_height=header.Height;
        Init_preview_24b(header.Width,header.Height,file_size,FORMAT_BMP);
        if (File_error==0)
        {
          switch (header.Compression)
          {
            case 3: // BI_BITFIELDS
              if (!Read_dword_le(file,&red_mask) ||
                  !Read_dword_le(file,&green_mask) ||
                  !Read_dword_le(file,&blue_mask))
                File_error=2;
              break;
            default:
              break;
          }
          if (fseek(file, header.Offset, SEEK_SET))
            File_error=2;
        }
        if (File_error==0)
        {
          switch (header.Nb_bits)
          {
            // 24bit bitmap
            default:
            case 24:
              line_size=Main_image_width*3;
              x_pos=(line_size % 4); // x_pos sert de variable temporaire
              if (x_pos>0)
                line_size+=(4-x_pos);
    
              buffer=(byte *)malloc(line_size);
              for (y_pos=Main_image_height-1; ((y_pos>=0) && (!File_error)); y_pos--)
              {
                if (Read_bytes(file,buffer,line_size))
                  for (x_pos=0,index=0; x_pos<Main_image_width; x_pos++,index+=3)
                    Pixel_load_24b(x_pos,y_pos,buffer[index+2],buffer[index+1],buffer[index+0]);
                else
                  File_error=2;
              }
              break;

            // 32bit bitmap
            case 32:
              line_size=Main_image_width*4;
              buffer=(byte *)malloc(line_size);
              for (y_pos=Main_image_height-1; ((y_pos>=0) && (!File_error)); y_pos--)
              {
                if (Read_bytes(file,buffer,line_size))
                  for (x_pos=0; x_pos<Main_image_width; x_pos++)
                  {
                    dword pixel=*(((dword *)buffer)+x_pos);
                    Pixel_load_24b(x_pos,y_pos,Bitmap_mask(pixel,red_mask),Bitmap_mask(pixel,green_mask),Bitmap_mask(pixel,blue_mask));
                  }
                else
                  File_error=2;
              }
              break;

            // 16bit bitmap
            case 16:
              line_size=(Main_image_width*2) + (Main_image_width&1)*2;
              buffer=(byte *)malloc(line_size);
              for (y_pos=Main_image_height-1; ((y_pos>=0) && (!File_error)); y_pos--)
              {
                if (Read_bytes(file,buffer,line_size))
                  for (x_pos=0; x_pos<Main_image_width; x_pos++)
                  {
                    word pixel=*(((word *)buffer)+x_pos);
                    Pixel_load_24b(x_pos,y_pos,Bitmap_mask(pixel,red_mask),Bitmap_mask(pixel,green_mask),Bitmap_mask(pixel,blue_mask));
                  }
                else
                  File_error=2;
              }
              break;
            
          }
          free(buffer);
          fclose(file);
        }
      }
    }
    else
    {
      fclose(file);
      File_error=1;
    }
  }
  else
    File_error=1;
}


// -- Sauvegarder un fichier au format BMP ----------------------------------
void Save_BMP(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_BMP_Header header;
  short x_pos;
  short y_pos;
  long line_size;
  word index;
  byte local_palette[256][4]; // R,G,B,0


  File_error=0;
  Get_full_filename(filename,0);

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    if (Main_image_width & 7)
      line_size=((Main_image_width >> 3)+1) << 3;
    else
      line_size=Main_image_width;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Signature  = 0x424D;
#else
    header.Signature  = 0x4D42;
#endif
    header.Size_1   =(line_size*Main_image_height)+1078;
    header.Reserved_1   =0;
    header.Reserved_2   =0;
    header.Offset   =1078;
    header.Size_2   =40;
    header.Width    =Main_image_width;
    header.Height    =Main_image_height;
    header.Planes      =1;
    header.Nb_bits    =8;
    header.Compression=0;
    header.Size_3   =0;
    header.XPM        =0;
    header.YPM        =0;
    header.Nb_Clr     =0;
    header.Clr_Imprt  =0;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    header.Size_1   = SDL_Swap32( header.Size_1 );
    header.Offset   = SDL_Swap32( header.Offset );
    header.Size_2   = SDL_Swap32( header.Size_2 );
    header.Width    = SDL_Swap32( header.Width );
    header.Height    = SDL_Swap32( header.Height );
    header.Planes      = SDL_Swap16( header.Planes );
    header.Nb_bits    = SDL_Swap16( header.Nb_bits );
    // If you ever set any more fields to non-zero, please swap here!
#endif

    if (Write_bytes(file,&header,sizeof(T_BMP_Header)))
    {
      //   Chez Bill, ils ont dit: "On va mettre les couleur dans l'ordre
      // inverse, et pour faire chier, on va les mettre sur une échelle de
      // 0 à 255 parce que le standard VGA c'est de 0 à 63 (logique!). Et
      // puis comme c'est pas assez débile, on va aussi y rajouter un octet
      // toujours à 0 pour forcer les gens à s'acheter des gros disques
      // durs... Comme ça, ça fera passer la pillule lorsqu'on sortira
      // Windows 95." ...
      for (index=0; index<256; index++)
      {
        local_palette[index][0]=Main_palette[index].B;
        local_palette[index][1]=Main_palette[index].G;
        local_palette[index][2]=Main_palette[index].R;
        local_palette[index][3]=0;
      }

      if (Write_bytes(file,local_palette,1024))
      {
        Init_write_buffer();

        // ... Et Bill, il a dit: "OK les gars! Mais seulement si vous rangez
        // les pixels dans l'ordre inverse, mais que sur les Y quand-même
        // parce que faut pas pousser."
        for (y_pos=Main_image_height-1; ((y_pos>=0) && (!File_error)); y_pos--)
          for (x_pos=0; x_pos<line_size; x_pos++)
                Write_one_byte(file,Read_pixel_function(x_pos,y_pos));

        End_write(file);
        fclose(file);

        if (File_error)
          remove(filename);
      }
      else
      {
        fclose(file);
        remove(filename);
        File_error=1;
      }

    }
    else
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
  }
  else
    File_error=1;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// GIF ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word Width; // width de l'écran virtuel
  word Height; // height de l'écran virtuel
  byte Resol;   // Informations sur la résolution (et autres)
  byte Backcol; // color de fond
  byte Aspect;  // Informations sur l'aspect ratio (et autres)
} __attribute__((__packed__)) T_GIF_LSDB; // Logical Screen Descriptor Block

typedef struct
{
  word Pos_X;         // Abscisse où devrait être affichée l'image
  word Pos_Y;         // Ordonnée où devrait être affichée l'image
  word Image_width; // width de l'image
  word Image_height; // height de l'image
  byte Indicator;    // Informations diverses sur l'image
  byte Nb_bits_pixel; // Nb de bits par pixel
} __attribute__((__packed__)) T_GIF_IDB; // Image Descriptor Block

// -- Tester si un fichier est au format GIF --------------------------------

void Test_GIF(void)
{
  char filename[MAX_PATH_CHARACTERS];
  char signature[6];
  FILE *file;


  File_error=1;
  Get_full_filename(filename,0);

  if ((file=fopen(filename, "rb")))
  {
    if (
        (Read_bytes(file,signature,6)) &&
        ((!memcmp(signature,"GIF87a",6))||(!memcmp(signature,"GIF89a",6)))
       )
      File_error=0;

    fclose(file);
  }
}


// -- Lire un fichier au format GIF -----------------------------------------

  // Définition de quelques variables globales au chargement du GIF87a
  word GIF_nb_bits;        // Nb de bits composants un code complet
  word GIF_remainder_bits;      // Nb de bits encore dispos dans GIF_last_byte
  byte GIF_remainder_byte;      // Nb d'octets avant le prochain bloc de Raster Data
  word GIF_current_code;    // Code traité (qui vient d'être lu en général)
  byte GIF_last_byte;      // Octet de lecture des bits
  word GIF_pos_X;          // Coordonnées d'affichage de l'image
  word GIF_pos_Y;
  word GIF_interlaced;     // L'image est entrelacée
  word GIF_finished_interlaced_image; // L'image entrelacée est finie de charger
  word GIF_pass;          // index de passe de l'image entrelacée
  FILE *GIF_file;        // L'handle du fichier

  // -- Lit le code à GIF_nb_bits suivant --

  word GIF_get_next_code(void)
  {
    word nb_bits_to_process=GIF_nb_bits;
    word nb_bits_processed  =0;
    word current_nb_bits;

    GIF_current_code=0;

    while (nb_bits_to_process)
    {
      if (GIF_remainder_bits==0) // Il ne reste plus de bits...
      {
        // Lire l'octet suivant:

        // Si on a atteint la fin du bloc de Raster Data
        if (GIF_remainder_byte==0)
          // Lire l'octet nous donnant la taille du bloc de Raster Data suivant
          if(Read_byte(GIF_file, &GIF_remainder_byte)!=1)
			  File_error=2;

		if(Read_byte(GIF_file,&GIF_last_byte)!=1)
			File_error = 2;
        GIF_remainder_byte--;
        GIF_remainder_bits=8;
      }

      current_nb_bits=(nb_bits_to_process<=GIF_remainder_bits)?nb_bits_to_process:GIF_remainder_bits;

      GIF_current_code|=(GIF_last_byte & ((1<<current_nb_bits)-1))<<nb_bits_processed;
      GIF_last_byte>>=current_nb_bits;
      nb_bits_processed  +=current_nb_bits;
      nb_bits_to_process-=current_nb_bits;
      GIF_remainder_bits    -=current_nb_bits;
    }

    return GIF_current_code;
  }

  // -- Affiche un nouveau pixel --

  void GIF_new_pixel(byte color)
  {
    Pixel_load_function(GIF_pos_X,GIF_pos_Y,color);

    GIF_pos_X++;

    if (GIF_pos_X>=Main_image_width)
    {
      GIF_pos_X=0;

      if (!GIF_interlaced)
        GIF_pos_Y++;
      else
      {
        switch (GIF_pass)
        {
          case 0 : GIF_pos_Y+=8;
                   break;
          case 1 : GIF_pos_Y+=8;
                   break;
          case 2 : GIF_pos_Y+=4;
                   break;
          default: GIF_pos_Y+=2;
        }

        if (GIF_pos_Y>=Main_image_height)
        {
          switch(++GIF_pass)
          {
          case 1 : GIF_pos_Y=4;
                   break;
          case 2 : GIF_pos_Y=2;
                   break;
          case 3 : GIF_pos_Y=1;
                   break;
          case 4 : GIF_finished_interlaced_image=1;
          }
        }
      }
    }
  }


void Load_GIF(void)
{
  char filename[MAX_PATH_CHARACTERS];
  char signature[6];

  word * alphabet_stack;     // Pile de décodage d'une chaîne
  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   alphabet_stack_pos; // Position dans la pile de décodage d'un chaîne

  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;

  word nb_colors;       // Nombre de couleurs dans l'image
  word color_index; // index de traitement d'une couleur
  byte size_to_read; // Nombre de données à lire      (divers)
  byte block_indentifier;  // Code indicateur du type de bloc en cours
  word initial_nb_bits;   // Nb de bits au début du traitement LZW
  word special_case=0;       // Mémoire pour le cas spécial
  word old_code=0;       // Code précédent
  word byte_read;         // Sauvegarde du code en cours de lecture
  word value_clr;        // Valeur <=> Clear tables
  word value_eof;        // Valeur <=> End d'image
  long file_size;
  int number_LID; // Nombre d'images trouvées dans le fichier

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  GIF_pos_X=0;
  GIF_pos_Y=0;
  GIF_last_byte=0;
  GIF_remainder_bits=0;
  GIF_remainder_byte=0;
  number_LID=0;
  
  Get_full_filename(filename,0);

  if ((GIF_file=fopen(filename, "rb")))
  {
    file_size=File_length_file(GIF_file);
    if ( (Read_bytes(GIF_file,signature,6)) &&
         ( (memcmp(signature,"GIF87a",6)==0) ||
           (memcmp(signature,"GIF89a",6)==0) ) )
    {

      // Allocation de mémoire pour les tables & piles de traitement:
      alphabet_stack   =(word *)malloc(4096*sizeof(word));
      alphabet_prefix=(word *)malloc(4096*sizeof(word));
      alphabet_suffix=(word *)malloc(4096*sizeof(word));

      if (Read_word_le(GIF_file,&(LSDB.Width))
      && Read_word_le(GIF_file,&(LSDB.Height))
      && Read_byte(GIF_file,&(LSDB.Resol))
      && Read_byte(GIF_file,&(LSDB.Backcol))
      && Read_byte(GIF_file,&(LSDB.Aspect))
        )
      {
        // Lecture du Logical Screen Descriptor Block réussie:

        Original_screen_X=LSDB.Width;
        Original_screen_Y=LSDB.Height;

        // Palette globale dispo = (LSDB.Resol  and $80)
        // Profondeur de couleur =((LSDB.Resol  and $70) shr 4)+1
        // Nombre de bits/pixel  = (LSDB.Resol  and $07)+1
        // Ordre de Classement   = (LSDB.Aspect and $80)

        alphabet_stack_pos=0;
        GIF_last_byte    =0;
        GIF_remainder_bits    =0;
        GIF_remainder_byte    =0;

        nb_colors=(1 << ((LSDB.Resol & 0x07)+1));
        initial_nb_bits=(LSDB.Resol & 0x07)+2;

        if (LSDB.Resol & 0x80)
        {
          // Palette globale dispo:

          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));

          //   On peut maintenant charger la nouvelle palette:
          if (!(LSDB.Aspect & 0x80))
            // Palette dans l'ordre:
            for(color_index=0;color_index<nb_colors;color_index++)
            {
              Read_byte(GIF_file,&Main_palette[color_index].R);
              Read_byte(GIF_file,&Main_palette[color_index].G);
              Read_byte(GIF_file,&Main_palette[color_index].B);
            }
          else
          {
            // Palette triée par composantes:
            for (color_index=0;color_index<nb_colors;color_index++)
              Read_byte(GIF_file,&Main_palette[color_index].R);
            for (color_index=0;color_index<nb_colors;color_index++)
              Read_byte(GIF_file,&Main_palette[color_index].G);
            for (color_index=0;color_index<nb_colors;color_index++)
              Read_byte(GIF_file,&Main_palette[color_index].B);
          }
          Set_palette(Main_palette);
        }

        // On lit un indicateur de block
        Read_byte(GIF_file,&block_indentifier);
        while (block_indentifier!=0x3B && !File_error)
        {
          switch (block_indentifier)
          {
            case 0x21: // Bloc d'extension
            {
              byte function_code;
              // Lecture du code de fonction:
              Read_byte(GIF_file,&function_code);   
              // Lecture de la taille du bloc:
              Read_byte(GIF_file,&size_to_read);
              while (size_to_read!=0 && !File_error)
              {
                switch(function_code)
                {
                  case 0xFE: // Comment Block Extension
                    // On récupère le premier commentaire non-vide, 
                    // on jette les autres.
                    if (Main_comment[0]=='\0')
                    {
                      int nb_char_to_keep=Min(size_to_read,COMMENT_SIZE);
                      
                      Read_bytes(GIF_file,Main_comment,nb_char_to_keep);
                      Main_comment[nb_char_to_keep+1]='\0';
                      // Si le commentaire etait trop long, on fait avance-rapide
                      // sur la suite.
                      if (size_to_read>nb_char_to_keep)
                        fseek(GIF_file,size_to_read-nb_char_to_keep,SEEK_CUR);
                    }
                    break;
                  case 0xF9: // Graphics Control Extension
                    // Prévu pour la transparence
                    
                  default:
                    // On saute le bloc:
                    fseek(GIF_file,size_to_read,SEEK_CUR);
                    break;
                }
                // Lecture de la taille du bloc suivant:
                Read_byte(GIF_file,&size_to_read);
              }
            }
            break;
            case 0x2C: // Local Image Descriptor
            {
              // Si on a deja lu une image, c'est une GIF animée ou bizarroide, on sort.
              if (number_LID!=0)
              {
                File_error=2;
                break;
              }
              number_LID++;
              
              // lecture de 10 derniers octets
              if ( Read_word_le(GIF_file,&(IDB.Pos_X))
                && Read_word_le(GIF_file,&(IDB.Pos_Y))
                && Read_word_le(GIF_file,&(IDB.Image_width))
                && Read_word_le(GIF_file,&(IDB.Image_height))
                && Read_byte(GIF_file,&(IDB.Indicator))
                && Read_byte(GIF_file,&(IDB.Nb_bits_pixel))
                && IDB.Image_width && IDB.Image_height)
              {
                Main_image_width=IDB.Image_width;
                Main_image_height=IDB.Image_height;
    
                Init_preview(IDB.Image_width,IDB.Image_height,file_size,FORMAT_GIF);
    
                // Palette locale dispo = (IDB.Indicator and $80)
                // Image entrelacée     = (IDB.Indicator and $40)
                // Ordre de classement  = (IDB.Indicator and $20)
                // Nombre de bits/pixel = (IDB.Indicator and $07)+1 (si palette locale dispo)
    
                if (IDB.Indicator & 0x80)
                {
                  // Palette locale dispo
    
                  nb_colors=(1 << ((IDB.Indicator & 0x07)+1));
                  initial_nb_bits=(IDB.Indicator & 0x07)+2;
    
                  if (!(IDB.Indicator & 0x40))
                    // Palette dans l'ordre:
                    for(color_index=0;color_index<nb_colors;color_index++)
                    {   
                      Read_byte(GIF_file,&Main_palette[color_index].R);
                      Read_byte(GIF_file,&Main_palette[color_index].G);
                      Read_byte(GIF_file,&Main_palette[color_index].B);
                    }
                  else
                  {
                    // Palette triée par composantes:
                    for (color_index=0;color_index<nb_colors;color_index++)
                      Read_byte(GIF_file,&Main_palette[color_index].R);
                    for (color_index=0;color_index<nb_colors;color_index++)
                      Read_byte(GIF_file,&Main_palette[color_index].G);
                    for (color_index=0;color_index<nb_colors;color_index++)
                      Read_byte(GIF_file,&Main_palette[color_index].B);
                  }
                  Set_palette(Main_palette);
                }
    
                Remap_fileselector();
    
                value_clr   =nb_colors+0;
                value_eof   =nb_colors+1;
                alphabet_free=nb_colors+2;
                GIF_nb_bits  =initial_nb_bits;
                alphabet_max      =((1 <<  GIF_nb_bits)-1);
                GIF_interlaced    =(IDB.Indicator & 0x40);
                GIF_pass         =0;
    
                /*Init_lecture();*/
    
                File_error=0;
                GIF_finished_interlaced_image=0;
    
                //////////////////////////////////////////// DECOMPRESSION LZW //
    
                while ( (GIF_get_next_code()!=value_eof) && (!File_error) )
                {
                  if (GIF_current_code<=alphabet_free)
                  {
                    if (GIF_current_code!=value_clr)
                    {
                      if (alphabet_free==(byte_read=GIF_current_code))
                      {
                        GIF_current_code=old_code;
                        alphabet_stack[alphabet_stack_pos++]=special_case;
                      }
    
                      while (GIF_current_code>value_clr)
                      {
                        alphabet_stack[alphabet_stack_pos++]=alphabet_suffix[GIF_current_code];
                        GIF_current_code=alphabet_prefix[GIF_current_code];
                      }
    
                      special_case=alphabet_stack[alphabet_stack_pos++]=GIF_current_code;
    
                      do
                        GIF_new_pixel(alphabet_stack[--alphabet_stack_pos]);
                      while (alphabet_stack_pos!=0);
    
                      alphabet_prefix[alphabet_free  ]=old_code;
                      alphabet_suffix[alphabet_free++]=GIF_current_code;
                      old_code=byte_read;
    
                      if (alphabet_free>alphabet_max)
                      {
                        if (GIF_nb_bits<12)
                          alphabet_max      =((1 << (++GIF_nb_bits))-1);
                      }
                    }
                    else // Code Clear rencontré
                    {
                      GIF_nb_bits       =initial_nb_bits;
                      alphabet_max      =((1 <<  GIF_nb_bits)-1);
                      alphabet_free     =nb_colors+2;
                      special_case       =GIF_get_next_code();
                      old_code       =GIF_current_code;
                      GIF_new_pixel(GIF_current_code);
                    }
                  }
                  else
                    File_error=2;
                } // Code End-Of-Information ou erreur de fichier rencontré
    
                /*Close_lecture();*/
    
                if (File_error>=0)
                if ( /* (GIF_pos_X!=0) || */
                     ( ( (!GIF_interlaced) && (GIF_pos_Y!=Main_image_height) ) ||
                       (  (GIF_interlaced) && (!GIF_finished_interlaced_image) )
                     ) )
                  File_error=2;
              } // Le fichier contenait un IDB
              else
                File_error=2;
            }
            default:
            break;
          }
          // Lecture du code de fonction suivant:
          Read_byte(GIF_file,&block_indentifier);
        }
      } // Le fichier contenait un LSDB
      else
        File_error=1;

      // Libération de la mémoire utilisée par les tables & piles de traitement:
      free(alphabet_suffix);
      free(alphabet_prefix);
      free(alphabet_stack);
    } // Le fichier contenait au moins la signature GIF87a ou GIF89a
    else
      File_error=1;

    fclose(GIF_file);

  } // Le fichier était ouvrable
  else
    File_error=1;
}


// -- Sauver un fichier au format GIF ---------------------------------------

  int  GIF_stop;         // "On peut arrêter la sauvegarde du fichier"
  byte GIF_buffer[256];   // buffer d'écriture de bloc de données compilées

  // -- Vider le buffer GIF dans le buffer KM --

  void GIF_empty_buffer(void)
  {
    word index;

    if (GIF_remainder_byte)
    {
      GIF_buffer[0]=GIF_remainder_byte;

      for (index=0;index<=GIF_remainder_byte;index++)
        Write_one_byte(GIF_file,GIF_buffer[index]);

      GIF_remainder_byte=0;
    }
  }

  // -- Ecrit un code à GIF_nb_bits --

  void GIF_set_code(word Code)
  {
    word nb_bits_to_process=GIF_nb_bits;
    word nb_bits_processed  =0;
    word current_nb_bits;

    while (nb_bits_to_process)
    {
      current_nb_bits=(nb_bits_to_process<=(8-GIF_remainder_bits))?nb_bits_to_process:(8-GIF_remainder_bits);

      GIF_last_byte|=(Code & ((1<<current_nb_bits)-1))<<GIF_remainder_bits;
      Code>>=current_nb_bits;
      GIF_remainder_bits    +=current_nb_bits;
      nb_bits_processed  +=current_nb_bits;
      nb_bits_to_process-=current_nb_bits;

      if (GIF_remainder_bits==8) // Il ne reste plus de bits à coder sur l'octet courant
      {
        // Ecrire l'octet à balancer:
        GIF_buffer[++GIF_remainder_byte]=GIF_last_byte;

        // Si on a atteint la fin du bloc de Raster Data
        if (GIF_remainder_byte==255)
          // On doit vider le buffer qui est maintenant plein
          GIF_empty_buffer();

        GIF_last_byte=0;
        GIF_remainder_bits=0;
      }
    }
  }


  // -- Lire le pixel suivant --

  byte GIF_next_pixel(void)
  {
    byte temp;

    temp=Read_pixel_function(GIF_pos_X,GIF_pos_Y);

    if (++GIF_pos_X>=Main_image_width)
    {
      GIF_pos_X=0;
      if (++GIF_pos_Y>=Main_image_height)
        GIF_stop=1;
    }

    return temp;
  }



void Save_GIF(void)
{
  char filename[MAX_PATH_CHARACTERS];

  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word * alphabet_daughter;    // Table des chaînes filles (plus longues)
  word * alphabet_sister;    // Table des chaînes soeurs (même longueur)
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   start;            // Code précédent (sert au linkage des chaînes)
  int    descend;          // Booléen "On vient de descendre"

  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;


  byte block_indentifier;  // Code indicateur du type de bloc en cours
  word current_string;   // Code de la chaîne en cours de traitement
  byte current_char;         // Caractère à coder
  word index;            // index de recherche de chaîne


  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  GIF_pos_X=0;
  GIF_pos_Y=0;
  GIF_last_byte=0;
  GIF_remainder_bits=0;
  GIF_remainder_byte=0;

  Get_full_filename(filename,0);

  if ((GIF_file=fopen(filename,"wb")))
  {
    // On écrit la signature du fichier
    if (Write_bytes(GIF_file,"GIF89a",6))
    {
      // La signature du fichier a été correctement écrite.

      // Allocation de mémoire pour les tables
      alphabet_prefix=(word *)malloc(4096*sizeof(word));
      alphabet_suffix=(word *)malloc(4096*sizeof(word));
      alphabet_daughter  =(word *)malloc(4096*sizeof(word));
      alphabet_sister  =(word *)malloc(4096*sizeof(word));

      // On initialise le LSDB du fichier
      if (Config.Screen_size_in_GIF)
      {
        LSDB.Width=Screen_width;
        LSDB.Height=Screen_height;
      }
      else
      {
        LSDB.Width=Main_image_width;
        LSDB.Height=Main_image_height;
      }
      LSDB.Resol  =0x97;          // Image en 256 couleurs, avec une palette
      LSDB.Backcol=0;
      LSDB.Aspect =0;             // Palette normale

      // On sauve le LSDB dans le fichier

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      LSDB.Width = SDL_Swap16(LSDB.Width);
      LSDB.Height = SDL_Swap16(LSDB.Height);
#endif

      if (Write_bytes(GIF_file,&LSDB,sizeof(T_GIF_LSDB)))
      {
        // Le LSDB a été correctement écrit.

        // On sauve la palette
        if (Write_bytes(GIF_file,Main_palette,768))
        {
          // La palette a été correctement écrite.

          //   Le jour où on se servira des blocks d'extensions pour placer
          // des commentaires, on le fera ici.

          // Ecriture de la transparence
          //Write_bytes(GIF_file,"\x21\xF9\x04\x01\x00\x00\xNN\x00",8);

          // Ecriture du commentaire
          if (Main_comment[0])
          {
            Write_bytes(GIF_file,"\x21\xFE",2);
            Write_byte(GIF_file,strlen(Main_comment));
            Write_bytes(GIF_file,Main_comment,strlen(Main_comment)+1);
          }
                            

          
          // On va écrire un block indicateur d'IDB et l'IDB du fichier

          block_indentifier=0x2C;
          IDB.Pos_X=0;
          IDB.Pos_Y=0;
          IDB.Image_width=Main_image_width;
          IDB.Image_height=Main_image_height;
          IDB.Indicator=0x07;    // Image non entrelacée, pas de palette locale.
          IDB.Nb_bits_pixel=8; // Image 256 couleurs;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
          IDB.Image_width = SDL_Swap16(IDB.Image_width);
          IDB.Image_height = SDL_Swap16(IDB.Image_height);
#endif

          if ( Write_bytes(GIF_file,&block_indentifier,1) &&
               Write_bytes(GIF_file,&IDB,sizeof(T_GIF_IDB)) )
          {
            //   Le block indicateur d'IDB et l'IDB ont étés correctements
            // écrits.

            Init_write_buffer();

            index=4096;
            File_error=0;
            GIF_stop=0;

            // Réintialisation de la table:
            alphabet_free=258;
            GIF_nb_bits  =9;
            alphabet_max =511;
            GIF_set_code(256);
            for (start=0;start<4096;start++)
            {
              alphabet_daughter[start]=4096;
              alphabet_sister[start]=4096;
            }

            ////////////////////////////////////////////// COMPRESSION LZW //

            start=current_string=GIF_next_pixel();
            descend=1;

            do
            {
              current_char=GIF_next_pixel();

              //   On regarde si dans la table on aurait pas une chaîne
              // équivalente à current_string+Caractere

              while ( (index<alphabet_free) &&
                      ( (current_string!=alphabet_prefix[index]) ||
                        (current_char      !=alphabet_suffix[index]) ) )
              {
                descend=0;
                start=index;
                index=alphabet_sister[index];
              }

              if (index<alphabet_free)
              {
                //   On sait ici que la current_string+Caractere se trouve
                // en position index dans les tables.

                descend=1;
                start=current_string=index;
                index=alphabet_daughter[index];
              }
              else
              {
                // On fait la jonction entre la current_string et l'actuelle
                if (descend)
                  alphabet_daughter[start]=alphabet_free;
                else
                  alphabet_sister[start]=alphabet_free;

                // On rajoute la chaîne current_string+Caractere à la table
                alphabet_prefix[alphabet_free  ]=current_string;
                alphabet_suffix[alphabet_free++]=current_char;

                // On écrit le code dans le fichier
                GIF_set_code(current_string);

                if (alphabet_free>0xFFF)
                {
                  // Réintialisation de la table:
                  GIF_set_code(256);
                  alphabet_free=258;
                  GIF_nb_bits  =9;
                  alphabet_max =511;
                  for (start=0;start<4096;start++)
                  {
                    alphabet_daughter[start]=4096;
                    alphabet_sister[start]=4096;
                  }
                }
                else if (alphabet_free>alphabet_max+1)
                {
                  // On augmente le nb de bits

                  GIF_nb_bits++;
                  alphabet_max=(1<<GIF_nb_bits)-1;
                }

                // On initialise la current_string et le reste pour la suite
                index=alphabet_daughter[current_char];
                start=current_string=current_char;
                descend=1;
              }
            }
            while ((!GIF_stop) && (!File_error));

            if (!File_error)
            {
              // On écrit le code dans le fichier
              GIF_set_code(current_string); // Dernière portion d'image

              //   Cette dernière portion ne devrait pas poser de problèmes
              // du côté GIF_nb_bits puisque pour que GIF_nb_bits change de
              // valeur, il faudrait que la table de chaîne soit remplie or
              // c'est impossible puisqu'on traite une chaîne qui se trouve
              // déjà dans la table, et qu'elle n'a rien d'inédit. Donc on
              // ne devrait pas avoir à changer de taille, mais je laisse
              // quand même en remarque tout ça, au cas où il subsisterait
              // des problèmes dans certains cas exceptionnels.
              //
              // Note: de toutes façons, ces lignes en commentaires ont étés
              //      écrites par copier/coller du temps où la sauvegarde du
              //      GIF déconnait. Il y a donc fort à parier qu'elles ne
              //      sont pas correctes.

              /*
              if (current_string==alphabet_max)
              {
                if (alphabet_max==0xFFF)
                {
                  // On balargue un Clear Code
                  GIF_set_code(256);

                  // On réinitialise les données LZW
                  alphabet_free=258;
                  GIF_nb_bits  =9;
                  alphabet_max =511;
                }
                else
                {
                  GIF_nb_bits++;
                  alphabet_max=(1<<GIF_nb_bits)-1;
                }
              }
              */

              GIF_set_code(257);             // Code de End d'image
              if (GIF_remainder_bits!=0)
                GIF_set_code(0);             // Code bidon permettant de s'assurer que tous les bits du dernier code aient bien étés inscris dans le buffer GIF
              GIF_empty_buffer();         // On envoie les dernières données du buffer GIF dans le buffer KM
              End_write(GIF_file);   // On envoie les dernières données du buffer KM  dans le fichier

              // On écrit un \0
              if (! Write_byte(GIF_file,'\x00'))
                File_error=1;
              // On écrit un GIF TERMINATOR, exigé par SVGA et SEA.
              if (! Write_byte(GIF_file,'\x3B'))
                File_error=1;

              
            }

          } // On a pu écrire l'IDB
          else
            File_error=1;

        } // On a pu écrire la palette
        else
          File_error=1;

      } // On a pu écrire le LSDB
      else
        File_error=1;

      // Libération de la mémoire utilisée par les tables
      free(alphabet_sister);
      free(alphabet_daughter);
      free(alphabet_suffix);
      free(alphabet_prefix);

    } // On a pu écrire la signature du fichier
    else
      File_error=1;

    fclose(GIF_file);
    if (File_error)
      remove(filename);

  } // On a pu ouvrir le fichier en écriture
  else
    File_error=1;
}






/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PCX ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
  {
    byte Manufacturer;       // |_ Il font chier ces cons! Ils auraient pu
    byte Version;            // |  mettre une vraie signature!
    byte Compression;        // L'image est-elle compressée?
    byte Depth;              // Nombre de bits pour coder un pixel (inutile puisqu'on se sert de Plane)
    word X_min;              // |_ Coin haut-gauche   |
    word Y_min;              // |  de l'image         |_ (Crétin!)
    word X_max;              // |_ Coin bas-droit     |
    word Y_max;              // |  de l'image         |
    word X_dpi;              // |_ Densité de |_ (Presque inutile parce que
    word Y_dpi;              // |  l'image    |  aucun moniteur n'est pareil!)
    byte Palette_16c[48];    // Palette 16 coul (inutile pour 256c) (débile!)
    byte Reserved;           // Ca me plait ça aussi!
    byte Plane;              // 4 => 16c , 1 => 256c , ...
    word Bytes_per_plane_line;// Doit toujours être pair
    word Palette_info;       // 1 => color , 2 => Gris (ignoré à partir de la version 4)
    word Screen_X;           // |_ Dimensions de
    word Screen_Y;           // |  l'écran d'origine
    byte Filler[54];         // Ca... J'adore!
  } __attribute__((__packed__)) T_PCX_Header;

T_PCX_Header PCX_header;

// -- Tester si un fichier est au format PCX --------------------------------

void Test_PCX(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;

  File_error=0;
  Get_full_filename(filename,0);

  if ((file=fopen(filename, "rb")))
  {
    if (Read_byte(file,&(PCX_header.Manufacturer)) &&
        Read_byte(file,&(PCX_header.Version)) &&
        Read_byte(file,&(PCX_header.Compression)) &&
        Read_byte(file,&(PCX_header.Depth)) &&
        Read_word_le(file,&(PCX_header.X_min)) &&
        Read_word_le(file,&(PCX_header.Y_min)) &&
        Read_word_le(file,&(PCX_header.X_max)) &&
        Read_word_le(file,&(PCX_header.Y_max)) &&
        Read_word_le(file,&(PCX_header.X_dpi)) &&
        Read_word_le(file,&(PCX_header.Y_dpi)) &&
        Read_bytes(file,&(PCX_header.Palette_16c),48) &&        
        Read_byte(file,&(PCX_header.Reserved)) &&
        Read_byte(file,&(PCX_header.Plane)) &&
        Read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
        Read_word_le(file,&(PCX_header.Palette_info)) &&
        Read_word_le(file,&(PCX_header.Screen_X)) &&
        Read_word_le(file,&(PCX_header.Screen_Y)) &&
        Read_bytes(file,&(PCX_header.Filler),54) )
    {
    
      //   Vu que ce header a une signature de merde et peu significative, il
      // va falloir que je teste différentes petites valeurs dont je connais
      // l'intervalle. Grrr!
      if ( (PCX_header.Manufacturer!=10)
        || (PCX_header.Compression>1)
        || ( (PCX_header.Depth!=1) && (PCX_header.Depth!=2) && (PCX_header.Depth!=4) && (PCX_header.Depth!=8) )
        || ( (PCX_header.Plane!=1) && (PCX_header.Plane!=2) && (PCX_header.Plane!=4) && (PCX_header.Plane!=8) && (PCX_header.Plane!=3) )
        || (PCX_header.X_max<PCX_header.X_min)
        || (PCX_header.Y_max<PCX_header.Y_min)
        || (PCX_header.Bytes_per_plane_line&1) )
        File_error=1;
    }
    else
      File_error=1;

    fclose(file);
  }
}


// -- Lire un fichier au format PCX -----------------------------------------

  // -- Afficher une ligne PCX codée sur 1 seul plan avec moins de 256 c. --
  void Draw_PCX_line(short y_pos, byte depth)
  {
    short x_pos;
    byte  color;
    byte  reduction=8/depth;
    byte  byte_mask=(1<<depth)-1;
    byte  reduction_minus_one=reduction-1;

    for (x_pos=0; x_pos<Main_image_width; x_pos++)
    {
      color=(LBM_buffer[x_pos/reduction]>>((reduction_minus_one-(x_pos%reduction))*depth)) & byte_mask;
      Pixel_load_function(x_pos,y_pos,color);
    }
  }

void Load_PCX(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  
  short line_size;
  short real_line_size; // width de l'image corrigée
  short width_read;
  short x_pos;
  short y_pos;
  byte  byte1;
  byte  byte2;
  byte  index;
  dword nb_colors;
  long  file_size;
  byte  palette_CGA[9]={ 84,252,252,  252, 84,252,  252,252,252};

  long  position;
  long  image_size;
  byte * buffer;

  Get_full_filename(filename,0);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);
    /*
    if (Read_bytes(file,&PCX_header,sizeof(T_PCX_Header)))
    {*/
    
    if (Read_byte(file,&(PCX_header.Manufacturer)) &&
        Read_byte(file,&(PCX_header.Version)) &&
        Read_byte(file,&(PCX_header.Compression)) &&
        Read_byte(file,&(PCX_header.Depth)) &&
        Read_word_le(file,&(PCX_header.X_min)) &&
        Read_word_le(file,&(PCX_header.Y_min)) &&
        Read_word_le(file,&(PCX_header.X_max)) &&
        Read_word_le(file,&(PCX_header.Y_max)) &&
        Read_word_le(file,&(PCX_header.X_dpi)) &&
        Read_word_le(file,&(PCX_header.Y_dpi)) &&
        Read_bytes(file,&(PCX_header.Palette_16c),48) &&        
        Read_byte(file,&(PCX_header.Reserved)) &&
        Read_byte(file,&(PCX_header.Plane)) &&
        Read_word_le(file,&(PCX_header.Bytes_per_plane_line)) &&
        Read_word_le(file,&(PCX_header.Palette_info)) &&
        Read_word_le(file,&(PCX_header.Screen_X)) &&
        Read_word_le(file,&(PCX_header.Screen_Y)) &&
        Read_bytes(file,&(PCX_header.Filler),54) )
    {
      
      Main_image_width=PCX_header.X_max-PCX_header.X_min+1;
      Main_image_height=PCX_header.Y_max-PCX_header.Y_min+1;

      Original_screen_X=PCX_header.Screen_X;
      Original_screen_Y=PCX_header.Screen_Y;

      if (PCX_header.Plane!=3)
      {
        Init_preview(Main_image_width,Main_image_height,file_size,FORMAT_PCX);
        if (File_error==0)
        {
          // On prépare la palette à accueillir les valeurs du fichier PCX
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));
          nb_colors=(dword)(1<<PCX_header.Plane)<<(PCX_header.Depth-1);

          if (nb_colors>4)
            memcpy(Main_palette,PCX_header.Palette_16c,48);
          else
          {
            Main_palette[1].R=0;
            Main_palette[1].G=0;
            Main_palette[1].B=0;
            byte1=PCX_header.Palette_16c[3]>>5;
            if (nb_colors==4)
            { // Pal. CGA "alakon" (du Turc Allahkoum qui signifie "à la con" :))
              memcpy(Main_palette+1,palette_CGA,9);
              if (!(byte1&2))
              {
                Main_palette[1].B=84;
                Main_palette[2].B=84;
                Main_palette[3].B=84;
              }
            } // Palette monochrome (on va dire que c'est du N&B)
            else
            {
              Main_palette[1].R=252;
              Main_palette[1].G=252;
              Main_palette[1].B=252;
            }
          }

          //   On se positionne à la fin du fichier - 769 octets pour voir s'il y
          // a une palette.
          if ( (PCX_header.Depth==8) && (PCX_header.Version>=5) && (file_size>(256*3)) )
          {
            fseek(file,file_size-((256*3)+1),SEEK_SET);
            // On regarde s'il y a une palette après les données de l'image
            if (Read_byte(file,&byte1))
              if (byte1==12) // Lire la palette si c'est une image en 256 couleurs
              {
                int index;
                // On lit la palette 256c que ces crétins ont foutue à la fin du fichier
                for(index=0;index<256;index++)
                  if ( ! Read_byte(file,&Main_palette[index].R)
                   || ! Read_byte(file,&Main_palette[index].G)
                   || ! Read_byte(file,&Main_palette[index].B) )
                  {
                    File_error=2;
                    DEBUG("ERROR READING PCX PALETTE !",index);
                    break;
                  }
              }
          }
          Set_palette(Main_palette);
          Remap_fileselector();

          //   Maintenant qu'on a lu la palette que ces crétins sont allés foutre
          // à la fin, on retourne juste après le header pour lire l'image.
          fseek(file,128,SEEK_SET);
          if (!File_error)
          {
            line_size=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
            real_line_size=(short)PCX_header.Bytes_per_plane_line<<3;
            //   On se sert de données LBM car le dessin de ligne en moins de 256
            // couleurs se fait comme avec la structure ILBM.
            Image_HAM=0;
            HBPm1=PCX_header.Plane-1;
            LBM_buffer=(byte *)malloc(line_size);

            // Chargement de l'image
            if (PCX_header.Compression)  // Image compressée
            {
              /*Init_lecture();*/
  
              image_size=(long)PCX_header.Bytes_per_plane_line*Main_image_height;

              if (PCX_header.Depth==8) // 256 couleurs (1 plan)
              {
                for (position=0; ((position<image_size) && (!File_error));)
                {
                  // Lecture et décompression de la ligne
                  if(Read_byte(file,&byte1) !=1) File_error=2;
                  if (!File_error)
                  {
                    if ((byte1&0xC0)==0xC0)
                    {
                      byte1-=0xC0;               // facteur de répétition
                      if(Read_byte(file,&byte2)!=1) File_error = 2; // octet à répéter
                      if (!File_error)
                      {
                        for (index=0; index<byte1; index++,position++)
                          if (position<image_size)
                            Pixel_load_function(position%line_size,
                                                position/line_size,
                                                byte2);
                          else
                            File_error=2;
                      }
                    }
                    else
                    {
                      Pixel_load_function(position%line_size,
                                          position/line_size,
                                          byte1);
                      position++;
                    }
                  }
                }
              }
              else                 // couleurs rangées par plans
              {
                for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
                {
                  for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
                  {
                    if(Read_byte(file,&byte1)!=1) File_error = 2;
                    if (!File_error)
                    {
                      if ((byte1&0xC0)==0xC0)
                      {
                        byte1-=0xC0;               // facteur de répétition
                        if(Read_byte(file,&byte2)!=1) File_error=2; // octet à répéter
                        if (!File_error)
                        {
                          for (index=0; index<byte1; index++)
                            if (x_pos<line_size)
                              LBM_buffer[x_pos++]=byte2;
                            else
                              File_error=2;
                        }
                        else
                          Set_file_error(2);
                      }
                      else
                        LBM_buffer[x_pos++]=byte1;
                    }
                  }
                  // Affichage de la ligne par plan du buffer
                  if (PCX_header.Depth==1)
                    Draw_ILBM_line(y_pos,real_line_size);
                  else
                    Draw_PCX_line(y_pos,PCX_header.Depth);
                }
              }

              /*Close_lecture();*/
            }
            else                     // Image non compressée
            {
              for (y_pos=0;(y_pos<Main_image_height) && (!File_error);y_pos++)
              {
                if ((width_read=Read_bytes(file,LBM_buffer,line_size)))
                {
                  if (PCX_header.Plane==1)
                    for (x_pos=0; x_pos<Main_image_width;x_pos++)
                      Pixel_load_function(x_pos,y_pos,LBM_buffer[x_pos]);
                  else
                  {
                    if (PCX_header.Depth==1)
                      Draw_ILBM_line(y_pos,real_line_size);
                    else
                      Draw_PCX_line(y_pos,PCX_header.Depth);
                  }
                }
                else
                  File_error=2;
              }
            }

            free(LBM_buffer);
          }
        }
      }
      else
      {
        // Image 24 bits!!!

        Init_preview_24b(Main_image_width,Main_image_height,file_size,FORMAT_PCX);

        if (File_error==0)
        {
          line_size=PCX_header.Bytes_per_plane_line*3;
          buffer=(byte *)malloc(line_size);

          if (!PCX_header.Compression)
          {
            for (y_pos=0;(y_pos<Main_image_height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,buffer,line_size))
              {
                for (x_pos=0; x_pos<Main_image_width; x_pos++)
                  Pixel_load_24b(x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
              }
              else
                File_error=2;
            }
          }
          else
          {
            /*Init_lecture();*/

            for (y_pos=0,position=0;(y_pos<Main_image_height) && (!File_error);)
            {
              // Lecture et décompression de la ligne
              if(Read_byte(file,&byte1)!=1) File_error=2;
              if (!File_error)
              {
                if ((byte1 & 0xC0)==0xC0)
                {
                  byte1-=0xC0;               // facteur de répétition
                  if(Read_byte(file,&byte2)!=1) File_error=2; // octet à répéter
                  if (!File_error)
                  {
                    for (index=0; (index<byte1) && (!File_error); index++)
                    {
                      buffer[position++]=byte2;
                      if (position>=line_size)
                      {
                        for (x_pos=0; x_pos<Main_image_width; x_pos++)
                          Pixel_load_24b(x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                        y_pos++;
                        position=0;
                      }
                    }
                  }
                }
                else
                {
                  buffer[position++]=byte1;
                  if (position>=line_size)
                  {
                    for (x_pos=0; x_pos<Main_image_width; x_pos++)
                      Pixel_load_24b(x_pos,y_pos,buffer[x_pos+(PCX_header.Bytes_per_plane_line*0)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*1)],buffer[x_pos+(PCX_header.Bytes_per_plane_line*2)]);
                    y_pos++;
                    position=0;
                  }
                }
              }
            }
            if (position!=0)
              File_error=2;

            /*Close_lecture();*/
          }
          free(buffer);
        }
      }
    }
    else
    {
      File_error=1;
    }

    fclose(file);
  }
  else
    File_error=1;
}


// -- Ecrire un fichier au format PCX ---------------------------------------

void Save_PCX(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;

  short line_size;
  short x_pos;
  short y_pos;
  byte  counter;
  byte  last_pixel;
  byte  pixel_read;



  Get_full_filename(filename,0);

  File_error=0;

  if ((file=fopen(filename,"wb")))
  {

    PCX_header.Manufacturer=10;
    PCX_header.Version=5;
    PCX_header.Compression=1;
    PCX_header.Depth=8;
    PCX_header.X_min=0;
    PCX_header.Y_min=0;
    PCX_header.X_max=Main_image_width-1;
    PCX_header.Y_max=Main_image_height-1;
    PCX_header.X_dpi=0;
    PCX_header.Y_dpi=0;
    memcpy(PCX_header.Palette_16c,Main_palette,48);
    PCX_header.Reserved=0;
    PCX_header.Plane=1;
    PCX_header.Bytes_per_plane_line=(Main_image_width&1)?Main_image_width+1:Main_image_width;
    PCX_header.Palette_info=1;
    PCX_header.Screen_X=Screen_width;
    PCX_header.Screen_Y=Screen_height;
    memset(PCX_header.Filler,0,54);

    if (Write_bytes(file,&(PCX_header.Manufacturer),1) &&
        Write_bytes(file,&(PCX_header.Version),1) &&
        Write_bytes(file,&(PCX_header.Compression),1) &&
        Write_bytes(file,&(PCX_header.Depth),1) &&
        Write_word_le(file,PCX_header.X_min) &&
        Write_word_le(file,PCX_header.Y_min) &&
        Write_word_le(file,PCX_header.X_max) &&
        Write_word_le(file,PCX_header.Y_max) &&
        Write_word_le(file,PCX_header.X_dpi) &&
        Write_word_le(file,PCX_header.Y_dpi) &&
        Write_bytes(file,&(PCX_header.Palette_16c),sizeof(PCX_header.Palette_16c)) &&        
        Write_bytes(file,&(PCX_header.Reserved),1) &&
        Write_bytes(file,&(PCX_header.Plane),1) &&
        Write_word_le(file,PCX_header.Bytes_per_plane_line) &&
        Write_word_le(file,PCX_header.Palette_info) &&
        Write_word_le(file,PCX_header.Screen_X) &&
        Write_word_le(file,PCX_header.Screen_Y) &&
        Write_bytes(file,&(PCX_header.Filler),sizeof(PCX_header.Filler)) )
    {
      line_size=PCX_header.Bytes_per_plane_line*PCX_header.Plane;
     
      Init_write_buffer();
     
      for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
      {
        pixel_read=Read_pixel_function(0,y_pos);
     
        // Compression et écriture de la ligne
        for (x_pos=0; ((x_pos<line_size) && (!File_error)); )
        {
          x_pos++;
          last_pixel=pixel_read;
          pixel_read=Read_pixel_function(x_pos,y_pos);
          counter=1;
          while ( (counter<63) && (x_pos<line_size) && (pixel_read==last_pixel) )
          {
            counter++;
            x_pos++;
            pixel_read=Read_pixel_function(x_pos,y_pos);
          }
      
          if ( (counter>1) || (last_pixel>=0xC0) )
            Write_one_byte(file,counter|0xC0);
          Write_one_byte(file,last_pixel);
      
        }
      }
      
      // Ecriture de l'octet (12) indiquant que la palette arrive
      if (!File_error)
        Write_one_byte(file,12);
      
      End_write(file);
      
      // Ecriture de la palette
      if (!File_error)
      {
        if (! Write_bytes(file,Main_palette,sizeof(T_Palette)))
          File_error=1;
      }
    }
    else
      File_error=1;
       
    fclose(file);
       
    if (File_error)
      remove(filename);
       
  }    
  else 
    File_error=1;
}      
       
       
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// CEL ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  word Width;              // width de l'image
  word Height;             // height de l'image
} __attribute__((__packed__))  T_CEL_Header1;

typedef struct
{
  byte Signature[4];           // Signature du format
  byte Kind;               // Type de fichier ($10=PALette $20=BitMaP)
  byte Nb_bits;             // Nombre de bits
  word Filler1;            // ???
  word Width;            // width de l'image
  word Height;            // height de l'image
  word X_offset;         // Offset en X de l'image
  word Y_offset;         // Offset en Y de l'image
  byte Filler2[16];        // ???
} __attribute__((__packed__))  T_CEL_Header2;

// -- Tester si un fichier est au format CEL --------------------------------

void Test_CEL(void)
{
  char filename[MAX_PATH_CHARACTERS];
  int  size;
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  int file_size;

  File_error=0;
  Get_full_filename(filename,0);
  file_size=File_length(filename);
  if (file_size==0)
  {
    File_error = 1; // Si on ne peut pas faire de stat il vaut mieux laisser tomber
    return;
  }
  
  if (! (file=fopen(filename, "rb")))
  {
    File_error = 1;
    return;
  }
  if (Read_word_le(file,&header1.Width) &&
      Read_word_le(file,&header1.Height) )
  {
      //   Vu que ce header n'a pas de signature, il va falloir tester la
      // cohérence de la dimension de l'image avec celle du fichier.
      
      size=file_size-sizeof(T_CEL_Header1);
      if ( (!size) || ( (((header1.Width+1)>>1)*header1.Height)!=size ) )
      {
        // Tentative de reconnaissance de la signature des nouveaux fichiers

        fseek(file,0,SEEK_SET);        
        if (Read_bytes(file,&header2.Signature,4) &&
            !memcmp(header2.Signature,"KiSS",4) &&
            Read_byte(file,&header2.Kind) &&
            (header2.Kind==0x20) &&
            Read_byte(file,&header2.Nb_bits) &&
            Read_word_le(file,&header2.Filler1) &&
            Read_word_le(file,&header2.Width) &&
            Read_word_le(file,&header2.Height) &&
            Read_word_le(file,&header2.X_offset) &&
            Read_word_le(file,&header2.Y_offset) &&
            Read_bytes(file,&header2.Filler2,16))
        {
          // ok
        }
        else
          File_error=1;
      }
      else
        File_error=1;
  }
  else
  {
    File_error=1;
  }
  fclose(file);    
}


// -- Lire un fichier au format CEL -----------------------------------------

void Load_CEL(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  long  file_size;

  File_error=0;
  Get_full_filename(filename,0);
  if ((file=fopen(filename, "rb")))
  {
    if (Read_bytes(file,&header1,sizeof(T_CEL_Header1)))
    {
      file_size=File_length_file(file);
      if ( (file_size>(long int)sizeof(T_CEL_Header1))
        && ( (((header1.Width+1)>>1)*header1.Height)==(file_size-(long int)sizeof(T_CEL_Header1)) ) )
      {
        // Chargement d'un fichier CEL sans signature (vieux fichiers)
        Main_image_width=header1.Width;
        Main_image_height=header1.Height;
        Original_screen_X=Main_image_width;
        Original_screen_Y=Main_image_height;
        Init_preview(Main_image_width,Main_image_height,file_size,FORMAT_CEL);
        if (File_error==0)
        {
          // Chargement de l'image
          /*Init_lecture();*/
          for (y_pos=0;((y_pos<Main_image_height) && (!File_error));y_pos++)
            for (x_pos=0;((x_pos<Main_image_width) && (!File_error));x_pos++)
              if ((x_pos & 1)==0)
              {
                if(Read_byte(file,&last_byte)!=1) File_error = 2;
                Pixel_load_function(x_pos,y_pos,(last_byte >> 4));
              }
              else
                Pixel_load_function(x_pos,y_pos,(last_byte & 15));
          /*Close_lecture();*/
        }
      }
      else
      {
        // On réessaye avec le nouveau format

        fseek(file,0,SEEK_SET);
        if (Read_bytes(file,&header2,sizeof(T_CEL_Header2)))
        {
          // Chargement d'un fichier CEL avec signature (nouveaux fichiers)

          Main_image_width=header2.Width+header2.X_offset;
          Main_image_height=header2.Height+header2.Y_offset;
          Original_screen_X=Main_image_width;
          Original_screen_Y=Main_image_height;
          Init_preview(Main_image_width,Main_image_height,file_size,FORMAT_CEL);
          if (File_error==0)
          {
            // Chargement de l'image
            /*Init_lecture();*/

            if (!File_error)
            {
              // Effacement du décalage
              for (y_pos=0;y_pos<header2.Y_offset;y_pos++)
                for (x_pos=0;x_pos<Main_image_width;x_pos++)
                  Pixel_load_function(x_pos,y_pos,0);
              for (y_pos=header2.Y_offset;y_pos<Main_image_height;y_pos++)
                for (x_pos=0;x_pos<header2.X_offset;x_pos++)
                  Pixel_load_function(x_pos,y_pos,0);

              switch(header2.Nb_bits)
              {
                case 4:
                  for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
                      if ((x_pos & 1)==0)
                      {
                        if(Read_byte(file,&last_byte)!=1) File_error=2;
                        Pixel_load_function(x_pos+header2.X_offset,y_pos+header2.Y_offset,(last_byte >> 4));
                      }
                      else
                        Pixel_load_function(x_pos+header2.X_offset,y_pos+header2.Y_offset,(last_byte & 15));
                  break;

                case 8:
                  for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
                    {
                      byte byte_read;
                      if(Read_byte(file,&byte_read)!=1) File_error = 2;
                      Pixel_load_function(x_pos+header2.X_offset,y_pos+header2.Y_offset,byte_read);
                      }
                  break;

                default:
                  File_error=1;
              }
            }
            /*Close_lecture();*/
          }
        }
        else
          File_error=1;
      }
      fclose(file);
    }
    else
      File_error=1;
  }
  else
    File_error=1;
}


// -- Ecrire un fichier au format CEL ---------------------------------------

void Save_CEL(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  dword Utilisation[256]; // Table d'utilisation de couleurs


  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(Utilisation);

  File_error=0;
  Get_full_filename(filename,0);
  if ((file=fopen(filename,"wb")))
  {
    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (x_pos=16;((x_pos<256) && (!Utilisation[x_pos]));x_pos++);

    if (x_pos==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      header1.Width =Main_image_width;
      header1.Height=Main_image_height;

      if (Write_bytes(file,&header1,sizeof(T_CEL_Header1)))
      {
        // Sauvegarde de l'image
        Init_write_buffer();
        for (y_pos=0;((y_pos<Main_image_height) && (!File_error));y_pos++)
        {
          for (x_pos=0;((x_pos<Main_image_width) && (!File_error));x_pos++)
            if ((x_pos & 1)==0)
              last_byte=(Read_pixel_function(x_pos,y_pos) << 4);
            else
            {
              last_byte=last_byte | (Read_pixel_function(x_pos,y_pos) & 15);
              Write_one_byte(file,last_byte);
            }

          if ((x_pos & 1)==1)
            Write_one_byte(file,last_byte);
        }
        End_write(file);
      }
      else
        File_error=1;
      fclose(file);
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      // Recherche du décalage
      for (y_pos=0;y_pos<Main_image_height;y_pos++)
      {
        for (x_pos=0;x_pos<Main_image_width;x_pos++)
          if (Read_pixel_function(x_pos,y_pos)!=0)
            break;
        if (Read_pixel_function(x_pos,y_pos)!=0)
          break;
      }
      header2.Y_offset=y_pos;
      for (x_pos=0;x_pos<Main_image_width;x_pos++)
      {
        for (y_pos=0;y_pos<Main_image_height;y_pos++)
          if (Read_pixel_function(x_pos,y_pos)!=0)
            break;
        if (Read_pixel_function(x_pos,y_pos)!=0)
          break;
      }
      header2.X_offset=x_pos;

      memcpy(header2.Signature,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x20;              // Initialisation du type (BitMaP)
      header2.Nb_bits=8;               // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (???)
      header2.Width=Main_image_width-header2.X_offset; // Initialisation de la largeur
      header2.Height=Main_image_height-header2.Y_offset; // Initialisation de la hauteur
      for (x_pos=0;x_pos<16;x_pos++)  // Initialisation du filler 2 (???)
        header2.Filler2[x_pos]=0;

      if (Write_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        // Sauvegarde de l'image
        Init_write_buffer();
        for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
          for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
            Write_one_byte(file,Read_pixel_function(x_pos+header2.X_offset,y_pos+header2.Y_offset));
        End_write(file);
      }
      else
        File_error=1;
      fclose(file);
    }

    if (File_error)
      remove(filename);
  }
  else
    File_error=1;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// KCF ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct
  {
    struct
    {
      byte Byte1;
      byte Byte2;
    } color[16];
  } Palette[10];
} __attribute__((__packed__)) T_KCF_Header;

// -- Tester si un fichier est au format KCF --------------------------------

void Test_KCF(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  int pal_index;
  int color_index;

  File_error=0;
  Get_full_filename(filename,0);
  if ((file=fopen(filename, "rb")))
  {
    if (File_length_file(file)==sizeof(T_KCF_Header))
    {
      Read_bytes(file,&buffer,sizeof(T_KCF_Header));
      // On vérifie une propriété de la structure de palette:
      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
          if ((buffer.Palette[pal_index].color[color_index].Byte2>>4)!=0)
            File_error=1;
    }
    else
    {
      if (Read_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        if (memcmp(header2.Signature,"KiSS",4)==0)
        {
          if (header2.Kind!=0x10)
            File_error=1;
        }
        else
          File_error=1;
      }
      else
        File_error=1;
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Lire un fichier au format KCF -----------------------------------------

void Load_KCF(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  long  file_size;


  File_error=0;
  Get_full_filename(filename,0);
  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);
    if (file_size==sizeof(T_KCF_Header))
    {
      // Fichier KCF à l'ancien format

      if (Read_bytes(file,&buffer,sizeof(T_KCF_Header)))
      {
        // Init_preview(???); // Pas possible... pas d'image...

        if (Config.Clear_palette)
          memset(Main_palette,0,sizeof(T_Palette));

        // Chargement de la palette
        for (pal_index=0;pal_index<10;pal_index++)
          for (color_index=0;color_index<16;color_index++)
          {
            index=16+(pal_index*16)+color_index;
            Main_palette[index].R=((buffer.Palette[pal_index].color[color_index].Byte1 >> 4) << 4);
            Main_palette[index].B=((buffer.Palette[pal_index].color[color_index].Byte1 & 15) << 4);
            Main_palette[index].G=((buffer.Palette[pal_index].color[color_index].Byte2 & 15) << 4);
          }

        for (index=0;index<16;index++)
        {
          Main_palette[index].R=Main_palette[index+16].R;
          Main_palette[index].G=Main_palette[index+16].G;
          Main_palette[index].B=Main_palette[index+16].B;
        }

        Set_palette(Main_palette);
        Remap_fileselector();
      }
      else
        File_error=1;
    }
    else
    {
      // Fichier KCF au nouveau format

      if (Read_bytes(file,&header2,sizeof(T_CEL_Header2)))
      {
        // Init_preview(???); // Pas possible... pas d'image...

        index=(header2.Nb_bits==12)?16:0;
        for (pal_index=0;pal_index<header2.Height;pal_index++)
        {
           // Pour chaque palette

           for (color_index=0;color_index<header2.Width;color_index++)
           {
             // Pour chaque couleur

             switch(header2.Nb_bits)
             {
               case 12: // RRRR BBBB | 0000 VVVV
                 Read_bytes(file,bytes,2);
                 Main_palette[index].R=(bytes[0] >> 4) << 4;
                 Main_palette[index].B=(bytes[0] & 15) << 4;
                 Main_palette[index].G=(bytes[1] & 15) << 4;
                 break;

               case 24: // RRRR RRRR | VVVV VVVV | BBBB BBBB
                 Read_bytes(file,bytes,3);
                 Main_palette[index].R=bytes[0];
                 Main_palette[index].G=bytes[1];
                 Main_palette[index].B=bytes[2];
             }

             index++;
           }
        }

        if (header2.Nb_bits==12)
          for (index=0;index<16;index++)
          {
            Main_palette[index].R=Main_palette[index+16].R;
            Main_palette[index].G=Main_palette[index+16].G;
            Main_palette[index].B=Main_palette[index+16].B;
          }

        Set_palette(Main_palette);
        Remap_fileselector();
      }
      else
        File_error=1;
    }
    fclose(file);
  }
  else
    File_error=1;

  if (!File_error) Draw_palette_preview();
}


// -- Ecrire un fichier au format KCF ---------------------------------------

void Save_KCF(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  dword Utilisation[256]; // Table d'utilisation de couleurs

  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(Utilisation);

  File_error=0;
  Get_full_filename(filename,0);
  if ((file=fopen(filename,"wb")))
  {
    // Sauvegarde de la palette

    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (index=16;((index<256) && (!Utilisation[index]));index++);

    if (index==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
        {
          index=16+(pal_index*16)+color_index;
          buffer.Palette[pal_index].color[color_index].Byte1=((Main_palette[index].R>>4)<<4) | (Main_palette[index].B>>4);
          buffer.Palette[pal_index].color[color_index].Byte2=Main_palette[index].G>>4;
        }

      if (! Write_bytes(file,&buffer,sizeof(T_KCF_Header)))
        File_error=1;
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      memcpy(header2.Signature,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x10;              // Initialisation du type (PALette)
      header2.Nb_bits=24;              // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (???)
      header2.Width=256;            // Initialisation du nombre de couleurs
      header2.Height=1;              // Initialisation du nombre de palettes
      header2.X_offset=0;           // Initialisation du décalage X
      header2.Y_offset=0;           // Initialisation du décalage Y
      for (index=0;index<16;index++) // Initialisation du filler 2 (???)
        header2.Filler2[index]=0;

      if (! Write_bytes(file,&header2,sizeof(T_CEL_Header2)))
        File_error=1;

      for (index=0;(index<256) && (!File_error);index++)
      {
        bytes[0]=Main_palette[index].R;
        bytes[1]=Main_palette[index].G;
        bytes[2]=Main_palette[index].B;
        if (! Write_bytes(file,bytes,3))
          File_error=1;
      }
    }

    fclose(file);

    if (File_error)
      remove(filename);
  }
  else
    File_error=1;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SCx ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
  byte Filler1[4];
  word Width;
  word Height;
  byte Filler2;
  byte Planes;
} __attribute__((__packed__)) T_SCx_Header;

// -- Tester si un fichier est au format SCx --------------------------------
void Test_SCx(void)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  //byte Signature[3];
  T_SCx_Header SCx_header;


  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture et vérification de la signature
    if ((Read_bytes(file,&SCx_header,sizeof(T_SCx_Header))))
    {
      if ( (!memcmp(SCx_header.Filler1,"RIX",3))
        && SCx_header.Width && SCx_header.Height)
      File_error=0;
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format SCx -----------------------------------------
void Load_SCx(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  long size,real_size;
  long file_size;
  T_SCx_Header SCx_header;
  T_Palette SCx_Palette;

  Get_full_filename(filename,0);

  File_error=0;

  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if ((Read_bytes(file,&SCx_header,sizeof(T_SCx_Header))))
    {
      Init_preview(SCx_header.Width,SCx_header.Height,file_size,FORMAT_SCx);
      if (File_error==0)
      {
        if (!SCx_header.Planes)
          size=sizeof(T_Palette);
        else
          size=sizeof(T_Components)*(1<<SCx_header.Planes);

        if (Read_bytes(file,SCx_Palette,size))
        {
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));

          Palette_64_to_256(SCx_Palette);
          memcpy(Main_palette,SCx_Palette,size);
          Set_palette(Main_palette);
          Remap_fileselector();

          Main_image_width=SCx_header.Width;
          Main_image_height=SCx_header.Height;

          if (!SCx_header.Planes)
          { // 256 couleurs (raw)
            LBM_buffer=(byte *)malloc(Main_image_width);

            for (y_pos=0;(y_pos<Main_image_height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,LBM_buffer,Main_image_width))
                for (x_pos=0; x_pos<Main_image_width;x_pos++)
                  Pixel_load_function(x_pos,y_pos,LBM_buffer[x_pos]);
              else
                File_error=2;
            }
          }
          else
          { // moins de 256 couleurs (planar)
            size=((Main_image_width+7)>>3)*SCx_header.Planes;
            real_size=(size/SCx_header.Planes)<<3;
            LBM_buffer=(byte *)malloc(size);
            HBPm1=SCx_header.Planes-1;
            Image_HAM=0;

            for (y_pos=0;(y_pos<Main_image_height) && (!File_error);y_pos++)
            {
              if (Read_bytes(file,LBM_buffer,size))
                Draw_ILBM_line(y_pos,real_size);
              else
                File_error=2;
            }
          }
          free(LBM_buffer);
        }
        else
          File_error=1;
      }
    }
    else
      File_error=1;

    fclose(file);
  }
  else
    File_error=1;
}

// -- Sauver un fichier au format SCx ---------------------------------------
void Save_SCx(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  T_SCx_Header SCx_header;

  Get_full_filename(filename,1);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    T_Palette palette_64;
    memcpy(palette_64,Main_palette,sizeof(T_Palette));
    Palette_256_to_64(palette_64);
    
    memcpy(SCx_header.Filler1,"RIX3",4);
    SCx_header.Width=Main_image_width;
    SCx_header.Height=Main_image_height;
    SCx_header.Filler2=0xAF;
    SCx_header.Planes=0x00;

    if (Write_bytes(file,&SCx_header,sizeof(T_SCx_Header)) &&
      Write_bytes(file,&palette_64,sizeof(T_Palette)))
    {
      Init_write_buffer();

      for (y_pos=0; ((y_pos<Main_image_height) && (!File_error)); y_pos++)
        for (x_pos=0; x_pos<Main_image_width; x_pos++)
          Write_one_byte(file,Read_pixel_function(x_pos,y_pos));

      End_write(file);
      fclose(file);

      if (File_error)
        remove(filename);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PI1 ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//// DECODAGE d'une partie d'IMAGE ////

void PI1_8b_to_16p(byte * src,byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de decodage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la source

  byte_mask=0x8000;
  w0=(((word)src[0])<<8) | src[1];
  w1=(((word)src[2])<<8) | src[3];
  w2=(((word)src[4])<<8) | src[5];
  w3=(((word)src[6])<<8) | src[7];
  for (i=0;i<16;i++)
  {
    // Pour décoder le pixel n°i, il faut traiter les 4 words sur leur bit
    // correspondant à celui du masque

    dest[i]=((w0 & byte_mask)?0x01:0x00) |
           ((w1 & byte_mask)?0x02:0x00) |
           ((w2 & byte_mask)?0x04:0x00) |
           ((w3 & byte_mask)?0x08:0x00);
    byte_mask>>=1;
  }
}

//// CODAGE d'une partie d'IMAGE ////

void PI1_16p_to_8b(byte * src,byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de codage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la destination

  byte_mask=0x8000;
  w0=w1=w2=w3=0;
  for (i=0;i<16;i++)
  {
    // Pour coder le pixel n°i, il faut modifier les 4 words sur leur bit
    // correspondant à celui du masque

    w0|=(src[i] & 0x01)?byte_mask:0x00;
    w1|=(src[i] & 0x02)?byte_mask:0x00;
    w2|=(src[i] & 0x04)?byte_mask:0x00;
    w3|=(src[i] & 0x08)?byte_mask:0x00;
    byte_mask>>=1;
  }
  dest[0]=w0 >> 8;
  dest[1]=w0 & 0x00FF;
  dest[2]=w1 >> 8;
  dest[3]=w1 & 0x00FF;
  dest[4]=w2 >> 8;
  dest[5]=w2 & 0x00FF;
  dest[6]=w3 >> 8;
  dest[7]=w3 & 0x00FF;
}

//// DECODAGE de la PALETTE ////

void PI1_decode_palette(byte * src,byte * palette)
{
  int i;  // Numéro de la couleur traitée
  int ip; // index dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  ip=0;
  for (i=0;i<16;i++)
  {
    w=((word)src[(i*2)+1]<<8) | src[(i*2)+0];

    // Traitement des couleurs rouge, verte et bleue:
    palette[ip++]=(((w & 0x0007) <<  1) | ((w & 0x0008) >>  3)) << 4;
    palette[ip++]=(((w & 0x7000) >> 11) | ((w & 0x8000) >> 15)) << 4;
    palette[ip++]=(((w & 0x0700) >>  7) | ((w & 0x0800) >> 11)) << 4;
  }
}

//// CODAGE de la PALETTE ////

void PI1_code_palette(byte * palette,byte * dest)
{
  int i;  // Numéro de la couleur traitée
  int ip; // index dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  ip=0;
  for (i=0;i<16;i++)
  {
    // Traitement des couleurs rouge, verte et bleue:
    w =(((word)(palette[ip]>>2) & 0x38) >> 3) | (((word)(palette[ip]>>2) & 0x04) <<  1); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 9) | (((word)(palette[ip]>>2) & 0x04) << 13); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 5) | (((word)(palette[ip]>>2) & 0x04) <<  9); ip++;

    dest[(i*2)+0]=w & 0x00FF;
    dest[(i*2)+1]=(w>>8);
  }
}


// -- Tester si un fichier est au format PI1 --------------------------------
void Test_PI1(void)
{
  FILE * file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size==32034) || (size==32066))
    {
      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0x0000)
          File_error=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PI1 -----------------------------------------
void Load_PI1(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffer,32034))
      {
        // Initialisation de la preview
        Init_preview(320,200,File_length_file(file),FORMAT_PI1);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));
          PI1_decode_palette(buffer+2,(byte *)Main_palette);
          Set_palette(Main_palette);
          Remap_fileselector();

          Main_image_width=320;
          Main_image_height=200;

          // Chargement/décompression de l'image
          ptr=buffer+34;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            for (x_pos=0;x_pos<(320>>4);x_pos++)
            {
              PI1_8b_to_16p(ptr,pixels+(x_pos<<4));
              ptr+=8;
            }
            for (x_pos=0;x_pos<320;x_pos++)
              Pixel_load_function(x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(buffer);
    }
    else
      File_error=1;
    fclose(file);
  }
  else
    File_error=1;
}


// -- Sauver un fichier au format PI1 ---------------------------------------
void Save_PI1(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32066);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)Main_palette,buffer+2);
    // Codage de l'image
    ptr=buffer+34;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<Main_image_height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<Main_image_width);x_pos++)
          pixels[x_pos]=Read_pixel_function(x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    memset(buffer+32034,0,32); // 32 extra NULL bytes at the end of the file to make ST Deluxe Paint happy

    if (Write_bytes(file,buffer,32066))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PC1 ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


//// DECOMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_uncompress_packbits(byte * src,byte * dest)
{
  int is,id; // Les indices de parcour des buffers
  int n;     // Octet de contrôle

  for (is=id=0;id<32000;)
  {
    n=src[is++];

    if (n & 0x80)
    {
      // Recopier src[is] -n+1 fois
      n=257-n;
      for (;(n>0) && (id<32000);n--)
        dest[id++]=src[is];
      is++;
    }
    else
    {
      // Recopier n+1 octets littéralement
      n=n+1;
      for (;(n>0) && (id<32000);n--)
        dest[id++]=src[is++];
    }

    // Contrôle des erreurs
    if (n>0)
      File_error=1;
  }
}

//// COMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_compress_packbits(byte * src,byte * dest,int source_size,int * dest_size)
{
  int is; // index dans la source
  int id; // index dans la destination
  int ir; // index de   la répétition
  int n;  // Taille des séquences
  int repet; // "Il y a répétition"

  for (is=id=0;is<source_size;)
  {
    // On recherche le 1er endroit où il y a répétition d'au moins 3 valeurs
    // identiques

    repet=0;
    for (ir=is;ir<source_size-2;ir++)
    {
      if ((src[ir]==src[ir+1]) && (src[ir+1]==src[ir+2]))
      {
        repet=1;
        break;
      }
      if ((ir-is)+1==40)
        break;
    }

    // On code la partie sans répétitions
    if (ir!=is)
    {
      n=(ir-is)+1;
      dest[id++]=n-1;
      for (;n>0;n--)
        dest[id++]=src[is++];
    }

    // On code la partie sans répétitions
    if (repet)
    {
      // On compte la quantité de fois qu'il faut répéter la valeur
      for (ir+=3;ir<source_size;ir++)
      {
        if (src[ir]!=src[is])
          break;
        if ((ir-is)+1==40)
          break;
      }
      n=(ir-is);
      dest[id++]=257-n;
      dest[id++]=src[is];
      is=ir;
    }
  }

  // On renseigne la taille du buffer compressé
  *dest_size=id;
}

//// DECODAGE d'une partie d'IMAGE ////

// Transformation de 4 plans de bits en 1 ligne de pixels

void PC1_4bp_to_1line(byte * src0,byte * src1,byte * src2,byte * src3,byte * dest)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    b0=src0[i];
    b1=src1[i];
    b2=src2[i];
    b3=src3[i];
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    for (j=0;j<8;j++)
    {
      dest[ip++]=((b0 & byte_mask)?0x01:0x00) |
                ((b1 & byte_mask)?0x02:0x00) |
                ((b2 & byte_mask)?0x04:0x00) |
                ((b3 & byte_mask)?0x08:0x00);
      byte_mask>>=1;
    }
  }
}

//// CODAGE d'une partie d'IMAGE ////

// Transformation d'1 ligne de pixels en 4 plans de bits

void PC1_1line_to_4bp(byte * src,byte * dst0,byte * dst1,byte * dst2,byte * dst3)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    b0=b1=b2=b3=0;
    for (j=0;j<8;j++)
    {
      b0|=(src[ip] & 0x01)?byte_mask:0x00;
      b1|=(src[ip] & 0x02)?byte_mask:0x00;
      b2|=(src[ip] & 0x04)?byte_mask:0x00;
      b3|=(src[ip] & 0x08)?byte_mask:0x00;
      ip++;
      byte_mask>>=1;
    }
    dst0[i]=b0;
    dst1[i]=b1;
    dst2[i]=b2;
    dst3[i]=b3;
  }
}


// -- Tester si un fichier est au format PC1 --------------------------------
void Test_PC1(void)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size<=32066))
    {
      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0x0080)
          File_error=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PC1 -----------------------------------------
void Load_PC1(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  int  size;
  word x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    size=File_length_file(file);
    // allocation des buffers mémoire
    buffercomp=(byte *)malloc(size);
    bufferdecomp=(byte *)malloc(32000);
    if ( (buffercomp!=NULL) && (bufferdecomp!=NULL) )
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffercomp,size))
      {
        // Initialisation de la preview
        Init_preview(320,200,File_length_file(file),FORMAT_PC1);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));
          PI1_decode_palette(buffercomp+2,(byte *)Main_palette);
          Set_palette(Main_palette);
          Remap_fileselector();

          Main_image_width=320;
          Main_image_height=200;

          // Décompression du buffer
          PC1_uncompress_packbits(buffercomp+34,bufferdecomp);

          // Décodage de l'image
          ptr=bufferdecomp;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            // Décodage de la scanline
            PC1_4bp_to_1line(ptr,ptr+40,ptr+80,ptr+120,pixels);
            ptr+=160;
            // Chargement de la ligne
            for (x_pos=0;x_pos<320;x_pos++)
              Pixel_load_function(x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(bufferdecomp);
      free(buffercomp);
    }
    else
    {
      File_error=1;
      free(bufferdecomp);
      free(buffercomp);
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Sauver un fichier au format PC1 ---------------------------------------
void Save_PC1(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  int   size;
  short x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // Allocation des buffers mémoire
    bufferdecomp=(byte *)malloc(32000);
    buffercomp  =(byte *)malloc(64066);
    // Codage de la résolution
    buffercomp[0]=0x80;
    buffercomp[1]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)Main_palette,buffercomp+2);
    // Codage de l'image
    ptr=bufferdecomp;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<Main_image_height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<Main_image_width);x_pos++)
          pixels[x_pos]=Read_pixel_function(x_pos,y_pos);
      }

      // Encodage de la scanline
      PC1_1line_to_4bp(pixels,ptr,ptr+40,ptr+80,ptr+120);
      ptr+=160;
    }

    // Compression du buffer
    PC1_compress_packbits(bufferdecomp,buffercomp+34,32000,&size);
    size+=34;
    for (x_pos=0;x_pos<16;x_pos++)
      buffercomp[size++]=0;

    if (Write_bytes(file,buffercomp,size))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération des buffers mémoire
    free(bufferdecomp);
    free(buffercomp);
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}


/// UNUSED and UNTESTED function to load a TGA file.
void Load_TGA(char * fname,T_Bitmap24B * dest,int * width,int * height)
{
  FILE* fichier;
  struct
  {
    byte Id_field_size;        // Taille des données spécifiques placées après le header
    byte Color_map_type;       // Présence d'une palette
    byte Image_type_code;      // Type d'image
    word Color_map_origin;     // index de départ de la palette
    word Color_map_length;     // Taille de la palette
    byte Color_map_entry_size; // Palette sur 16, 24 ou 32 bits
    word X_origin;             // Coordonnées de départ
    word Y_origin;
    word Width;                // width de l'image
    word Height;               // height de l'image
    byte Pixel_size;           // Pixels sur 16, 24 ou 32 bits
    byte Descriptor;           // Paramètres divers
  } TGA_header;
  int x,y,py,skip,t;
  byte * buffer;

  fichier=fopen(fname,"rb");
  Read_bytes(fichier,&TGA_header,sizeof(TGA_header));
  if (TGA_header.Image_type_code==2)
  {
    *width=TGA_header.Width;
    *height=TGA_header.Height;
    *dest=(T_Bitmap24B)malloc((*width)*(*height)*3);

    // On saute l'ID field
    fseek(fichier,TGA_header.Id_field_size,SEEK_CUR);

    // On saute la palette
    if (TGA_header.Color_map_type==0)
      skip=0;
    else
    {
      skip=TGA_header.Color_map_length;
      if (TGA_header.Color_map_entry_size==16)
        skip*=2;
      else
      if (TGA_header.Color_map_entry_size==24)
        skip*=3;
      else
      if (TGA_header.Color_map_entry_size==32)
        skip*=4;
    }
    fseek(fichier,skip,SEEK_CUR);

    // Lecture des pixels
    skip=(*width);
    if (TGA_header.Pixel_size==16)
      skip*=2;
    else
    if (TGA_header.Pixel_size==24)
      skip*=3;
    else
    if (TGA_header.Pixel_size==32)
      skip*=4;

    buffer=(byte *)malloc(skip);
    for (y=0;y<(*height);y++)
    {
      Read_bytes(fichier,buffer,skip);

      // Inversion du rouge et du bleu
      for (x=0;x<(*width);x++)
      {
        t=buffer[(x*3)+0];
        buffer[(x*3)+0]=buffer[(x*3)+2];
        buffer[(x*3)+2]=t;
      }

      // Prise en compte du sens d'écriture verticale
      if (TGA_header.Descriptor & 0x20)
        py=y;
      else
        py=(*height)-y-1;

      // Prise en compte de l'interleave verticale
      if (TGA_header.Descriptor & 0xC0)
        py=((py % (*height))*2)+(py/(*height));

      memcpy((*dest)+(py*(*width)),buffer,skip);
    }
    free(buffer);
  }
  fclose(fichier);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// NEO ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void Test_NEO(void)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename,0);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size==32128))
    {
	  // Flag word : toujours 0
	  if (Read_word_le(file,&resolution))
	  {
		  if (resolution == 0)
			  File_error = 0;
	  }

      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0 || resolution==1 || resolution==2)
          File_error |= 0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }

}

void Load_NEO(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32128);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffer,32128))
      {
        // Initialisation de la preview
        Init_preview(320,200,File_length_file(file),FORMAT_NEO);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(Main_palette,0,sizeof(T_Palette));
		  // on saute la résolution et le flag, chacun 2 bits
          PI1_decode_palette(buffer+4,(byte *)Main_palette);
          Set_palette(Main_palette);
          Remap_fileselector();

          Main_image_width=320;
          Main_image_height=200;

          // Chargement/décompression de l'image
          ptr=buffer+128;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            for (x_pos=0;x_pos<(320>>4);x_pos++)
            {
              PI1_8b_to_16p(ptr,pixels+(x_pos<<4));
              ptr+=8;
            }
            for (x_pos=0;x_pos<320;x_pos++)
              Pixel_load_function(x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(buffer);
    }
    else
      File_error=1;
    fclose(file);
  }
  else
    File_error=1;
}

void Save_NEO(void)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename,0);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32128);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    buffer[2]=0x00;
    buffer[3]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)Main_palette,buffer+4);
    // Codage de l'image
    ptr=buffer+128;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<Main_image_height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<Main_image_width);x_pos++)
          pixels[x_pos]=Read_pixel_function(x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    if (Write_bytes(file,buffer,32128))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// C64 ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void Test_C64(void)
{  
	FILE* file;
        char filename[MAX_PATH_CHARACTERS];
        long file_size;

        Get_full_filename(filename,0);

        file = fopen(filename,"rb");

        if(file)
        {
			file_size = File_length_file(file);
			switch(file_size)
			{
				case 8000: // raw bitmap
				case 8002: // raw bitmap with loadaddr
				case 9000: // bitmap + screen
				case 9002: // bitmap + screen + loadaddr
				case 10001: // multicolor
				case 10003: // multicolor + loadaddr
					File_error = 0;
					break;
				default: // then we don't know for now.
					File_error = 1;
				
			}
            fclose(file);
        }
        else
        {
            File_error = 1;
        }
}

void Load_C64_hires(byte *bitmap, byte *colors)
{	
	int cx,cy,x,y,c[4],pixel,color;
 
    for(cy=0; cy<25; cy++)
    {
    	for(cx=0; cx<40; cx++)
    	{
    		c[1]=colors[cy*40+cx]&15;
    		c[0]=colors[cy*40+cx]>>4;
    		for(y=0; y<8; y++)
    		{
    			pixel=bitmap[cy*320+cx*8+y];
    			for(x=0; x<8; x++)
    			{
    				color=c[pixel&(1<<(7-x))?1:0];
    				Pixel_load_function(cx*8+x,cy*8+y,color);
    			}
    		}
    	}
    }
}

void Load_C64_multi(byte *bitmap, byte *colors, byte *nybble, byte background)
{
	int cx,cy,x,y,c[4],pixel,color;
	c[0]=background;
	for(cy=0; cy<25; cy++)
    {
    	for(cx=0; cx<40; cx++)
    	{                		
    		c[1]=colors[cy*40+cx]>>4;
    		c[2]=colors[cy*40+cx]&15;
    		c[3]=nybble[cy*40+cx];
                		
    		for(y=0; y<8; y++)
    		{
    			pixel=bitmap[cy*320+cx*8+y];
    			for(x=0; x<4; x++)
    			{
                				
    				color=c[(pixel&3)];
    				pixel>>=2;
    				Pixel_load_function(cx*4+(3-x),cy*8+y,color);
    			}
    		}
    	}
    }
}

void Load_C64(void)
{    
	FILE* file;
   	char filename[MAX_PATH_CHARACTERS];
   	long file_size;
   	int newPixel_ratio;
   	byte background;
   	
   	// Palette from http://www.pepto.de/projects/colorvic/
   	byte pal[48]={
        0x00, 0x00, 0x00, 
        0xFF, 0xFF, 0xFF, 
        0x68, 0x37, 0x2B, 
        0x70, 0xA4, 0xB2, 
        0x6F, 0x3D, 0x86, 
        0x58, 0x8D, 0x43, 
        0x35, 0x28, 0x79, 
        0xB8, 0xC7, 0x6F, 
        0x6F, 0x4F, 0x25, 
        0x43, 0x39, 0x00, 
        0x9A, 0x67, 0x59, 
        0x44, 0x44, 0x44, 
        0x6C, 0x6C, 0x6C, 
        0x9A, 0xD2, 0x84, 
        0x6C, 0x5E, 0xB5, 
        0x95, 0x95, 0x95};

	byte bitmap[8000],colors[1000],nybble[1000];
	word width=320, height=200;
    
    Get_full_filename(filename,0);
    file = fopen(filename,"rb");

    if(file)
    {
        
        memcpy(Main_palette,pal,48); // this set the software palette for grafx2
        Set_palette(Main_palette); // this set the hardware palette for SDL
        Remap_fileselector(); // Always call it if you change the palette
				
        file_size = File_length_file(file);
                
        if(file_size>9002)width=160;
                
        Init_preview(width, height, file_size, FORMAT_C64); // Do this as soon as you can
					 
        Main_image_width = width ;                
        Main_image_height = height;
                
        Read_bytes(file,bitmap,8000);
        if(file_size>8002)Read_bytes(file,colors,1000);
						
        if(width==160)
        {
        	Read_bytes(file,nybble,1000);
        	Read_byte(file,&background);
        	Load_C64_multi(bitmap,colors,nybble,background);
        	newPixel_ratio = PIXEL_WIDE;
        }
        else
        {
        	Load_C64_hires(bitmap,colors);
        	newPixel_ratio = PIXEL_SIMPLE;
        }
                
        //Pixel_ratio = newPixel_ratio;
        
        File_error = 0;
        fclose(file);
    }
    else
    File_error = 1;
}

int Save_C64_hires(FILE *file)
{
	int cx,cy,x,y,c1,c2,i,pixel,bits;
	word numcolors;
	dword cusage[256];
	byte colors[1000];
	
	for(x=0;x<1000;x++)colors[x]=1; // init colormem to black/white
	
	for(cy=0; cy<25; cy++) // Character line, 25 lines
	{
		for(cx=0; cx<40; cx++) // Character column, 40 columns
		{
			for(i=0;i<256;i++) cusage[i]=0;
			
			numcolors=Count_used_colors_area(cusage,cx*8,cy*8,8,8);
			if(numcolors>2)
			{
				Warning_message("More than 2 colors in 8x8 pixels");
				// TODO here we should hilite the offending block
				printf("\nerror at %dx%d (%d colors)\n",cx*8,cy*8,numcolors);
				return 1;
			}
			for(i=0;i<16;i++)
			{
				if(cusage[i])
				{
					c2=i;
					break;
				}
			}
			c1=c2;
			for(i=c2+1;i<16;i++)
			{
				if(cusage[i])
				{
					c1=i;
					
				}
			}			
			colors[cx+cy*40]=(c2<<4)|c1;

			for(y=0; y<8; y++)
			{
				bits=0;
				for(x=0; x<8; x++)
				{
					pixel=Read_pixel_function(x+cx*8,y+cy*8);
					if(pixel>15) 
					{ 
						Warning_message("Color above 15 used"); 
						// TODO hilite offending block here too?
						// or make it smarter with color allocation?
						// However, the palette is fixed to the 16 first colors
						return 1;
					}
					bits=bits<<1;
					if(pixel==c1) bits|=1;					
				}
				Write_byte(file,bits&255);
			}
		}
	}
	Write_bytes(file,colors,1000);
	return 0;
}

int Save_C64_multi(FILE *file)
{
/* 
BITS 	COLOR INFORMATION COMES FROM
00 	Background color #0 (screen color)
01 	Upper 4 bits of screen memory
10 	Lower 4 bits of screen memory
11 	Color nybble (nybble = 1/2 byte = 4 bits)
*/

	int cx,cy,x,y,c[4]={0,0,0,0},color,lut[16],bits,pixel;
	byte screen[1000],nybble[1000];
	word numcolors,count;
	dword cusage[256];
	byte i,background=0;
	numcolors=Count_used_colors(cusage);
	
	count=0;
	for(x=0;x<16;x++)
	{
		//printf("color %d, pixels %d\n",x,cusage[x]);
		if(cusage[x]>count)
		{
			count=cusage[x];
			background=x;
			
		}
	}
	
	for(cy=0; cy<25; cy++)
	{
		//printf("\ny:%2d ",cy);
		for(cx=0; cx<40; cx++)
		{
			numcolors=Count_used_colors_area(cusage,cx*4,cy*8,4,8);
			if(numcolors>4)
			{
				Warning_message("More than 4 colors in 4x8");
				// TODO hilite offending block
				return 1;
			}
			color=1;
			c[0]=background;
			for(i=0; i<16; i++)
			{
				lut[i]=0;
				if(cusage[i])
				{
					if(i!=background)
					{
						lut[i]=color;
						c[color]=i;
						color++;
					}
					else
					{
						lut[i]=0;
					}
				}
			}
			// add to screen and nybble
			screen[cx+cy*40]=c[1]<<4|c[2];
			nybble[cx+cy*40]=c[3];
			//printf("%x%x%x ",c[1],c[2],c[3]);
			for(y=0;y<8;y++)
			{
				bits=0;
				for(x=0;x<4;x++)
				{					
					pixel=Read_pixel_function(cx*4+x,cy*8+y);
					if(pixel>15) 
					{ 
						Warning_message("Color above 15 used"); 
						// TODO hilite as in hires, you should stay to 
						// the fixed 16 color palette
						return 1;
					}
					bits=bits<<2;
					bits|=lut[pixel];

				}
				Write_byte(file,bits&255);
			}
		}
	}
	Write_bytes(file,screen,1000);
	Write_bytes(file,nybble,1000);
	Write_byte(file,background);
	//printf("\nbg:%d\n",background);
	return 0;
}

void Save_C64(void)
{
	FILE* file;
	char filename[MAX_PATH_CHARACTERS];
	dword numcolors,cusage[256];
	numcolors=Count_used_colors(cusage);

	if(numcolors>16)
	{
		Warning_message("Error: Max 16 colors");
		File_error = 1;
		return;
	}
	if(((Main_image_width!=320) && (Main_image_width!=160)) || Main_image_height!=200)
	{
		Warning_message("must be 320x200 or 160x200");
		File_error = 1;
		return;
	} 
	
   	Get_full_filename(filename,0);
	file = fopen(filename,"wb");
	
	if(!file)
	{
		Warning_message("File open failed");
		File_error = 1;
		return;
	}
	
	if(Main_image_width==320)
		File_error = Save_C64_hires(file); 
	else
	   	File_error = Save_C64_multi(file);
	
	fclose(file);
	
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// PNG ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifndef __no_pnglib__

// -- Tester si un fichier est au format PNG --------------------------------
void Test_PNG(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte png_header[8];
  
  Get_full_filename(filename,0);
  
  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture du header du fichier
    if (Read_bytes(file,png_header,8))
    {
      if ( !png_sig_cmp(png_header, 0, 8))
        File_error=0;
    }
    fclose(file);
  }
}

png_bytep * Row_pointers;
// -- Lire un fichier au format PNG -----------------------------------------
void Load_PNG(void)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  byte png_header[8];  
  byte row_pointers_allocated;
 
  png_structp png_ptr;
  png_infop info_ptr;

  Get_full_filename(filename,0);

  File_error=0;
  
  if ((file=fopen(filename, "rb")))
  {
    // Load header (8 first bytes)
    if (Read_bytes(file,png_header,8))
    {
      // Do we recognize a png file signature ?
      if ( !png_sig_cmp(png_header, 0, 8))
      {
        // Prepare internal PNG loader
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr)
        {
          // Prepare internal PNG loader
          info_ptr = png_create_info_struct(png_ptr);
          if (info_ptr)
          {
            png_byte color_type;
            png_byte bit_depth;
            
            // Setup a return point. If a pnglib loading error occurs
            // in this if(), the else will be executed.
            if (!setjmp(png_jmpbuf(png_ptr)))
            {
              png_init_io(png_ptr, file);
              // Inform pnglib we already loaded the header.
              png_set_sig_bytes(png_ptr, 8);
            
              // Load file information
              png_read_info(png_ptr, info_ptr);
              color_type = info_ptr->color_type;
              bit_depth = info_ptr->bit_depth;
              
              // If it's any supported file
              // (Note: As of writing this, this test covers every possible 
              // image format of libpng)
              if (color_type == PNG_COLOR_TYPE_PALETTE
               || color_type == PNG_COLOR_TYPE_GRAY
               || color_type == PNG_COLOR_TYPE_GRAY_ALPHA
               || color_type == PNG_COLOR_TYPE_RGB
               || color_type == PNG_COLOR_TYPE_RGB_ALPHA
              )
              {
                int num_text;
                png_text *text_ptr;
                
                int unit_type;
                png_uint_32 res_x;
                png_uint_32 res_y;

                // Comment (tEXt)
                Main_comment[0]='\0'; // Clear the previous comment
                if ((num_text=png_get_text(png_ptr, info_ptr, &text_ptr, NULL)))
                {
                  while (num_text--)
                  {
                    if (!strcmp(text_ptr[num_text].key,"Title"))
                    {
                      int size;
                      size = Min(text_ptr[num_text].text_length, COMMENT_SIZE);
                      strncpy(Main_comment, text_ptr[num_text].text, size);
                      Main_comment[size]='\0';
                      break; // Skip all others tEXt chunks
                    }
                  }
                }
                // Pixel Ratio (pHYs)
                if (png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type))
                {
                  // Ignore unit, and use the X/Y ratio as a hint for
                  // WIDE or TALL pixels
                  if (res_x>0 && res_y>0)
                  {
                    if (res_y/res_x>1)
                    {
                      Ratio_of_loaded_image=PIXEL_WIDE;
                    }
                    else if (res_x/res_y>1)
                    {
                      Ratio_of_loaded_image=PIXEL_TALL;
                    }
                  }
                }
                if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
                  Init_preview_24b(info_ptr->width,info_ptr->height,File_length_file(file),FORMAT_PNG);
                else
                  Init_preview(info_ptr->width,info_ptr->height,File_length_file(file),FORMAT_PNG);

                if (File_error==0)
                {
                  int x,y;
                  png_colorp palette;
                  int num_palette;

                  // 16-bit images
                  if (bit_depth == 16)
                  {
                    // Reduce to 8-bit
                    png_set_strip_16(png_ptr);
                  }
                  else if (bit_depth < 8)
                  {
                    // Inform libpng we want one byte per pixel,
                    // even though the file was less than 8bpp
                    png_set_packing(png_ptr);
                  }
                    
                  // Images with alpha channel
                  if (color_type & PNG_COLOR_MASK_ALPHA)
                  {
                    // Tell libpng to ignore it
                    png_set_strip_alpha(png_ptr);
                  }

                  // Greyscale images : 
                  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                  {
                    // Map low bpp greyscales to full 8bit (0-255 range)
                    if (bit_depth < 8)
                      png_set_gray_1_2_4_to_8(png_ptr);
                    
                    // Create greyscale palette
                    for (x=0;x<256;x++)
                    {
                      Main_palette[x].R=x;
                      Main_palette[x].G=x;
                      Main_palette[x].B=x;
                    } 
                  }
                  else if (color_type == PNG_COLOR_TYPE_PALETTE) // Palette images
                  {
                    
                    if (bit_depth < 8)
                    {
                      // Clear unused colors
                      if (Config.Clear_palette)
                        memset(Main_palette,0,sizeof(T_Palette));
                    }
                    // Load the palette
                    png_get_PLTE(png_ptr, info_ptr, &palette,
                       &num_palette);
                    for (x=0;x<num_palette;x++)
                    {
                      Main_palette[x].R=palette[x].red;
                      Main_palette[x].G=palette[x].green;
                      Main_palette[x].B=palette[x].blue;
                    }
                    free(palette);
                  }
                  if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGB_ALPHA)
                  {
                    Set_palette(Main_palette);
                    Remap_fileselector();
                  }
                  
                  Main_image_width=info_ptr->width;
                  Main_image_height=info_ptr->height;
                  
                  png_set_interlace_handling(png_ptr);
                  png_read_update_info(png_ptr, info_ptr);

                  // Allocate row pointers
                  Row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * Main_image_height);
                  row_pointers_allocated = 0;

                  /* read file */
                  if (!setjmp(png_jmpbuf(png_ptr)))
                  {
                    if (color_type == PNG_COLOR_TYPE_GRAY
                    ||  color_type == PNG_COLOR_TYPE_GRAY_ALPHA
                    ||  color_type == PNG_COLOR_TYPE_PALETTE
                    )
                    {
                      // 8bpp
                      
                      for (y=0; y<Main_image_height; y++)
                        Row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);
                      row_pointers_allocated = 1;
                      
                      png_read_image(png_ptr, Row_pointers);
                      
                      for (y=0; y<Main_image_height; y++)
                        for (x=0; x<Main_image_width; x++)
                          Pixel_load_function(x, y, Row_pointers[y][x]);
                    }
                    else
                    {
                      // 24bpp
                      
                      if (Pixel_load_24b==Pixel_load_in_24b_preview)
                      {
                        // It's a preview
                        // Unfortunately we need to allocate loads of memory
                        for (y=0; y<Main_image_height; y++)
                          Row_pointers[y] = (png_byte*) malloc(info_ptr->rowbytes);
                        row_pointers_allocated = 1;
                        
                        png_read_image(png_ptr, Row_pointers);
                        
                        for (y=0; y<Main_image_height; y++)
                          for (x=0; x<Main_image_width; x++)
                            Pixel_load_24b(x, y, Row_pointers[y][x*3],Row_pointers[y][x*3+1],Row_pointers[y][x*3+2]);
                      }
                      else
                      {
                        // It's loading an actual image
                        // We'll save memory and time by writing directly into
                        // our pre-allocated 24bit buffer
                        for (y=0; y<Main_image_height; y++)
                          Row_pointers[y] = (png_byte*) (&Buffer_image_24b[y * Main_image_width]);
                        png_read_image(png_ptr, Row_pointers);
                      }
                    }
                  }
                  else
                    File_error=2;
                    
                  /* cleanup heap allocation */
                  if (row_pointers_allocated)
                  {
                    for (y=0; y<Main_image_height; y++)
                      free(Row_pointers[y]);
                  }
                  free(Row_pointers);
                }
                else
                  File_error=2;
              }
              else
               // Unsupported image type
               File_error=1;
            }
            else
             File_error=1;
          }
          else
            File_error=1;
        }
      }
      /*Close_lecture();*/
    }
    else // Lecture header impossible: Error ne modifiant pas l'image
      File_error=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Error ne modifiant pas l'image
    File_error=1;
}

void Save_PNG(void)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  int y;
  byte * pixel_ptr;
  png_structp png_ptr;
  png_infop info_ptr;
  
  Get_full_filename(filename,0);
  File_error=0;
  Row_pointers = NULL;
  
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    /* initialisation */
    if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))
      && (info_ptr = png_create_info_struct(png_ptr)))
    {
  
      if (!setjmp(png_jmpbuf(png_ptr)))
      {    
        png_init_io(png_ptr, file);
      
        /* en-tete */
        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          png_set_IHDR(png_ptr, info_ptr, Main_image_width, Main_image_height,
            8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

          png_set_PLTE(png_ptr, info_ptr, (png_colorp)Main_palette, 256);
          {
            // Commentaires texte PNG
            // Cette partie est optionnelle
            png_text text_ptr[2] = {
              {-1, "Software", "Grafx2", 6},
              {-1, "Title", NULL, 0}
            };
            int nb_text_chunks=1;
            if (Main_comment[0])
            {
              text_ptr[1].text=Main_comment;
              text_ptr[1].text_length=strlen(Main_comment);
              nb_text_chunks=2;
            }
            png_set_text(png_ptr, info_ptr, text_ptr, nb_text_chunks);
          }
          switch(Pixel_ratio)
          {
            case PIXEL_WIDE:
              png_set_pHYs(png_ptr, info_ptr, 3000, 6000, PNG_RESOLUTION_METER);
              break;
            case PIXEL_TALL:
              png_set_pHYs(png_ptr, info_ptr, 6000, 3000, PNG_RESOLUTION_METER);
              break;
            default:
              break;
          }          
          png_write_info(png_ptr, info_ptr);

          /* ecriture des pixels de l'image */
          Row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * Main_image_height);
          pixel_ptr = (Read_pixel_function==Read_pixel_from_current_screen)?Main_screen:Brush;
          for (y=0; y<Main_image_height; y++)
            Row_pointers[y] = (png_byte*)(pixel_ptr+y*Main_image_width);

          if (!setjmp(png_jmpbuf(png_ptr)))
          {
            png_write_image(png_ptr, Row_pointers);
          
            /* cloture png */
            if (!setjmp(png_jmpbuf(png_ptr)))
            {          
              png_write_end(png_ptr, NULL);
            }
            else
              File_error=1;
          }
          else
            File_error=1;
        }
        else
          File_error=1;
      }
      else
      {
        File_error=1;
      }
      png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    else
      File_error=1;
    // fermeture du fichier
    fclose(file);
  }

  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri trainait... Ca fait pas propre.
  if (File_error)
    remove(filename);
  
  if (Row_pointers)
  {
    free(Row_pointers);
    Row_pointers=NULL;
  }
}
#endif  // __no_pnglib__

// Saves an image.
// This routine will only be called when all hope is lost, memory thrashed, etc
// It's the last chance to save anything, but the code has to be extremely careful,
// anything could happen.
// The chosen format is IMG since it's extremely simple, difficult to make it create an unusable image.
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
