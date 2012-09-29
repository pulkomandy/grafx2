/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
    Copyright 2010 Alexander Filyanov
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
#define _XOPEN_SOURCE 500

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
#include "brush.h"
#include "setup.h"
#include "filesel.h"

// -- PKM -------------------------------------------------------------------
void Test_PKM(T_IO_Context *);
void Load_PKM(T_IO_Context *);
void Save_PKM(T_IO_Context *);

// -- IFF -------------------------------------------------------------------
void Test_LBM(T_IO_Context *);
void Test_PBM(T_IO_Context *);
void Load_IFF(T_IO_Context *);
void Save_IFF(T_IO_Context *);

// -- GIF -------------------------------------------------------------------
void Test_GIF(T_IO_Context *);
void Load_GIF(T_IO_Context *);
void Save_GIF(T_IO_Context *);

// -- PCX -------------------------------------------------------------------
void Test_PCX(T_IO_Context *);
void Load_PCX(T_IO_Context *);
void Save_PCX(T_IO_Context *);

// -- BMP -------------------------------------------------------------------
void Test_BMP(T_IO_Context *);
void Load_BMP(T_IO_Context *);
void Save_BMP(T_IO_Context *);

// -- IMG -------------------------------------------------------------------
void Test_IMG(T_IO_Context *);
void Load_IMG(T_IO_Context *);
void Save_IMG(T_IO_Context *);

// -- SCx -------------------------------------------------------------------
void Test_SCx(T_IO_Context *);
void Load_SCx(T_IO_Context *);
void Save_SCx(T_IO_Context *);

// -- CEL -------------------------------------------------------------------
void Test_CEL(T_IO_Context *);
void Load_CEL(T_IO_Context *);
void Save_CEL(T_IO_Context *);

// -- KCF -------------------------------------------------------------------
void Test_KCF(T_IO_Context *);
void Load_KCF(T_IO_Context *);
void Save_KCF(T_IO_Context *);

// -- PAL -------------------------------------------------------------------
void Test_PAL(T_IO_Context *);
void Load_PAL(T_IO_Context *);
void Save_PAL(T_IO_Context *);

// -- PI1 -------------------------------------------------------------------
void Test_PI1(T_IO_Context *);
void Load_PI1(T_IO_Context *);
void Save_PI1(T_IO_Context *);

// -- PC1 -------------------------------------------------------------------
void Test_PC1(T_IO_Context *);
void Load_PC1(T_IO_Context *);
void Save_PC1(T_IO_Context *);

// -- NEO -------------------------------------------------------------------
void Test_NEO(T_IO_Context *);
void Load_NEO(T_IO_Context *);
void Save_NEO(T_IO_Context *);

// -- C64 -------------------------------------------------------------------
void Test_C64(T_IO_Context *);
void Load_C64(T_IO_Context *);
void Save_C64(T_IO_Context *);

// -- SCR (Amstrad CPC)
void Save_SCR(T_IO_Context *);

// -- CM5 (Amstrad CPC)
void Test_CM5(T_IO_Context *);
void Load_CM5(T_IO_Context *);
void Save_CM5(T_IO_Context *);

// -- XPM (X PixMap)
// Loading is done through SDL_Image 
void Save_XPM(T_IO_Context*);

// -- PNG -------------------------------------------------------------------
#ifndef __no_pnglib__
void Test_PNG(T_IO_Context *);
void Load_PNG(T_IO_Context *);
void Save_PNG(T_IO_Context *);
#endif

// -- SDL_Image -------------------------------------------------------------
// (TGA, BMP, PNM, XPM, XCF, PCX, GIF, JPG, TIF, IFF, PNG, ICO)
void Load_SDL_Image(T_IO_Context *);

// ENUM     Name  TestFunc LoadFunc SaveFunc PalOnly Comment Layers Ext Exts  
T_Format File_formats[] = {
  {FORMAT_ALL_IMAGES, "(all)", NULL, NULL, NULL, 0, 0, 0, "", "gif;png;bmp;pcx;pkm;iff;lbm;ilbm;img;sci;scq;scf;scn;sco;pi1;pc1;cel;neo;kcf;pal;c64;koa;koala;fli;bml;cdu;prg;tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff;ico;cm5"},
  {FORMAT_ALL_FILES, "(*.*)", NULL, NULL, NULL, 0, 0, 0, "", "*"},
  {FORMAT_GIF, " gif", Test_GIF, Load_GIF, Save_GIF, 0, 1, 1, "gif", "gif"},
#ifndef __no_pnglib__
  {FORMAT_PNG, " png", Test_PNG, Load_PNG, Save_PNG, 0, 1, 0, "png", "png"},
#endif
  {FORMAT_BMP, " bmp", Test_BMP, Load_BMP, Save_BMP, 0, 0, 0, "bmp", "bmp"},
  {FORMAT_PCX, " pcx", Test_PCX, Load_PCX, Save_PCX, 0, 0, 0, "pcx", "pcx"},
  {FORMAT_PKM, " pkm", Test_PKM, Load_PKM, Save_PKM, 0, 1, 0, "pkm", "pkm"},
  {FORMAT_LBM, " lbm", Test_LBM, Load_IFF, Save_IFF, 0, 0, 0, "iff", "iff;lbm;ilbm"},
  {FORMAT_PBM, " pbm", Test_PBM, Load_IFF, Save_IFF, 0, 0, 0, "iff", "iff;pbm"},
  {FORMAT_IMG, " img", Test_IMG, Load_IMG, Save_IMG, 0, 0, 0, "img", "img"},
  {FORMAT_SCx, " sc?", Test_SCx, Load_SCx, Save_SCx, 0, 0, 0, "sc?", "sci;scq;scf;scn;sco"},
  {FORMAT_PI1, " pi1", Test_PI1, Load_PI1, Save_PI1, 0, 0, 0, "pi1", "pi1"},
  {FORMAT_PC1, " pc1", Test_PC1, Load_PC1, Save_PC1, 0, 0, 0, "pc1", "pc1"},
  {FORMAT_CEL, " cel", Test_CEL, Load_CEL, Save_CEL, 0, 0, 0, "cel", "cel"},
  {FORMAT_NEO, " neo", Test_NEO, Load_NEO, Save_NEO, 0, 0, 0, "neo", "neo"},
  {FORMAT_KCF, " kcf", Test_KCF, Load_KCF, Save_KCF, 1, 0, 0, "kcf", "kcf"},
  {FORMAT_PAL, " pal", Test_PAL, Load_PAL, Save_PAL, 1, 0, 0, "pal", "pal"},
  {FORMAT_C64, " c64", Test_C64, Load_C64, Save_C64, 0, 1, 0, "c64", "c64;koa;koala;fli;bml;cdu;prg"},
  {FORMAT_SCR, " cpc", NULL,     NULL,     Save_SCR, 0, 0, 0, "cpc", "cpc;scr"},
  {FORMAT_CM5, " cm5", Test_CM5, Load_CM5, Save_CM5, 0, 0, 1, "cm5", "cm5"},
  {FORMAT_XPM, " xpm", NULL,     NULL,     Save_XPM, 0, 0, 0, "xpm", "xpm"},
  {FORMAT_MISC,"misc.",NULL,     NULL,     NULL,     0, 0, 0, "",    "tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff;ico"},
};

/// Total number of known file formats
unsigned int Nb_known_formats(void)
{
  return sizeof(File_formats)/sizeof(File_formats[0]);
}

/// Set the color of a pixel (on load)
void Set_pixel(T_IO_Context *context, short x_pos, short y_pos, byte color)
{
  // Clipping
  if ((x_pos>=context->Width) || (y_pos>=context->Height))
    return;
    
  switch (context->Type)
  {
    // Chargement des pixels dans l'écran principal
    case CONTEXT_MAIN_IMAGE:
      Pixel_in_current_screen(x_pos,y_pos,color);
      break;
      
    // Chargement des pixels dans la brosse
    case CONTEXT_BRUSH:
      //Pixel_in_brush(x_pos,y_pos,color);
      *(context->Buffer_image + y_pos * context->Pitch + x_pos)=color;
      break;
      
    // Chargement des pixels dans la preview
    case CONTEXT_PREVIEW:
      // Skip pixels of transparent index if :
      // it's a layer above the first one
      if (color == context->Transparent_color && context->Current_layer > 0)
        break;
      
      if (((x_pos % context->Preview_factor_X)==0) && ((y_pos % context->Preview_factor_Y)==0))
      {
        // Tag the color as 'used'
        context->Preview_usage[color]=1;
        
        // Store pixel
        if (context->Ratio == PIXEL_WIDE && 
          Pixel_ratio != PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE2)
        {
          context->Preview_bitmap[x_pos/context->Preview_factor_X*2 + (y_pos/context->Preview_factor_Y)*PREVIEW_WIDTH*Menu_factor_X]=color;
          context->Preview_bitmap[x_pos/context->Preview_factor_X*2+1 + (y_pos/context->Preview_factor_Y)*PREVIEW_WIDTH*Menu_factor_X]=color;
        }
        else if (context->Ratio == PIXEL_TALL && 
          Pixel_ratio != PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL2)
        {
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y*2)*PREVIEW_WIDTH*Menu_factor_X]=color;
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y*2+1)*PREVIEW_WIDTH*Menu_factor_X]=color;
        }
        else
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y)*PREVIEW_WIDTH*Menu_factor_X]=color;
      }

      break;
      
    // Load pixels in a SDL_Surface
    case CONTEXT_SURFACE:
      if (x_pos>=0 && y_pos>=0 && x_pos<context->Surface->w && y_pos<context->Surface->h)
        *(((byte *)(context->Surface->pixels)) + context->Surface->pitch * y_pos + x_pos) = color;
      break;
  
  }

}

void Fill_canvas(T_IO_Context *context, byte color)
{
  switch (context->Type)
  {
    case CONTEXT_PREVIEW:
      if (context->Current_layer!=0)
        return;
      memset(context->Preview_bitmap, color, PREVIEW_WIDTH*PREVIEW_HEIGHT*Menu_factor_X*Menu_factor_Y);
      break;
    case CONTEXT_MAIN_IMAGE:
      memset(
        Main_backups->Pages->Image[Main_current_layer].Pixels,
        color,
        Main_backups->Pages->Width*Main_backups->Pages->Height);
      break;
    case CONTEXT_BRUSH:
      memset(context->Buffer_image, color, (long)context->Height*context->Pitch);
      break;
    case CONTEXT_SURFACE:
      break;
  }
}

void Palette_loaded(T_IO_Context *context)
{
  // Update the current screen to the loaded palette
  switch (context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
    case CONTEXT_PREVIEW:
    case CONTEXT_BRUSH:
    case CONTEXT_SURFACE:
      break;
  }

  switch (context->Type)
  {
    case CONTEXT_PREVIEW:      
    case CONTEXT_MAIN_IMAGE:
    case CONTEXT_BRUSH:
    case CONTEXT_SURFACE:
      break;
  }
}

// Chargement des pixels dans le buffer 24b
void Set_pixel_24b(T_IO_Context *context, short x_pos, short y_pos, byte r, byte g, byte b)
{
  byte color;
  
  // Clipping
  if (x_pos<0 || y_pos<0 || x_pos>=context->Width || y_pos>=context->Height)
    return;
        
  switch(context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
    case CONTEXT_BRUSH:
    case CONTEXT_SURFACE:
      {
        int index;
        
        index=(y_pos*context->Width)+x_pos;
        context->Buffer_image_24b[index].R=r;
        context->Buffer_image_24b[index].G=g;
        context->Buffer_image_24b[index].B=b;
      }
      break;
      
    case CONTEXT_PREVIEW:
      
      if (((x_pos % context->Preview_factor_X)==0) && ((y_pos % context->Preview_factor_Y)==0))
      {
        color=((r >> 5) << 5) |
                ((g >> 5) << 2) |
                ((b >> 6));
        
        // Tag the color as 'used'
        context->Preview_usage[color]=1;
        
        context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y)*PREVIEW_WIDTH*Menu_factor_X]=color;
      }
      break;
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

void Set_frame_duration(T_IO_Context *context, int duration)
{
  switch(context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
      Main_backups->Pages->Image[context->Current_layer].Duration = duration;
      break;
    default:
      break;
  }
}

int Get_frame_duration(T_IO_Context *context)
{
  switch(context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
      return Main_backups->Pages->Image[context->Current_layer].Duration;
    default:
      return 0;
  }
}

///
/// Generic allocation and similar stuff, done at beginning of image load,
/// as soon as size is known.
void Pre_load(T_IO_Context *context, short width, short height, long file_size, int format, enum PIXEL_RATIO ratio, byte truecolor)
{
  char  str[10];

  context->Pitch = width; // default
  context->Width = width;
  context->Height = height;
  context->Ratio = ratio;
  context->Nb_layers = 1;
  context->Transparent_color=0;
  context->Background_transparent=0;
  
  switch(context->Type)
  {
    // Preview
    case CONTEXT_PREVIEW:
      // Préparation du chargement d'une preview:
      
      context->Preview_bitmap=calloc(1, PREVIEW_WIDTH*PREVIEW_HEIGHT*Menu_factor_X*Menu_factor_Y);
      if (!context->Preview_bitmap)
        File_error=1;
            
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
      if (file_size<1048576)
      {
        // Le fichier fait moins d'un Mega, on affiche sa taille direct
        Num2str(file_size,str,7);
        Print_in_window(236,59,str,MC_Black,MC_Light);
      }
      else if ((file_size/1024)<100000)
      {
        // Le fichier fait plus d'un Mega, on peut afficher sa taille en Ko
        Num2str(file_size/1024,str,5);
        strcpy(str+5,"Kb");
        Print_in_window(236,59,str,MC_Black,MC_Light);
      }
      else
      {
        // Le fichier fait plus de 100 Mega octets (cas très rare :))
        Print_in_window(236,59,"LARGE!!",MC_Black,MC_Light);
      }
  
      // Affichage du vrai format
      if (format!=Selector->Format_filter)
      {
        Print_in_window( 59,59,Get_fileformat(format)->Label,MC_Black,MC_Light);
      }

      // On efface le commentaire précédent
      Window_rectangle(45,70,32*8,8,MC_Light);
  
      // Calcul des données nécessaires à l'affichage de la preview:
      if (ratio == PIXEL_WIDE && 
          Pixel_ratio != PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE2)
        width*=2;
      else if (ratio == PIXEL_TALL && 
          Pixel_ratio != PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL2)
        height*=2;
      
      context->Preview_factor_X=Round_div_max(width,120*Menu_factor_X);
      context->Preview_factor_Y=Round_div_max(height, 80*Menu_factor_Y);
  
      if ( (!Config.Maximize_preview) && (context->Preview_factor_X!=context->Preview_factor_Y) )
      {
        if (context->Preview_factor_X>context->Preview_factor_Y)
          context->Preview_factor_Y=context->Preview_factor_X;
        else
          context->Preview_factor_X=context->Preview_factor_Y;
      }
  
      context->Preview_pos_X=Window_pos_X+183*Menu_factor_X;
      context->Preview_pos_Y=Window_pos_Y+ 95*Menu_factor_Y;
  
      // On nettoie la zone où va s'afficher la preview:
      Window_rectangle(183,95,PREVIEW_WIDTH,PREVIEW_HEIGHT,MC_Light);
      
      // Un update pour couvrir les 4 zones: 3 libellés plus le commentaire
      Update_window_area(45,48,256,30);
      // Zone de preview
      Update_window_area(183,95,PREVIEW_WIDTH,PREVIEW_HEIGHT);
      break;
      
    // Other loading
    case CONTEXT_MAIN_IMAGE:
      if (Backup_new_image(1,width,height))
      {
        // La nouvelle page a pu être allouée, elle est pour l'instant pleine
        // de 0s. Elle fait Main_image_width de large.
        // Normalement tout va bien, tout est sous contrôle...
        
        // Load into layer 0, by default.
        context->Nb_layers=1;
        Main_current_layer=0;
        Main_layers_visible=1<<0;
        Set_loading_layer(context,0);
        
        // Remove previous comment, unless we load just a palette
        if (! Get_fileformat(context->Format)->Palette_only)
          context->Comment[0]='\0';
      }
      else
      {
        // Afficher un message d'erreur
        // Pour être sûr que ce soit lisible.
        Compute_optimal_menu_colors(context->Palette);
        Message_out_of_memory();
        File_error=1; // 1 => On n'a pas perdu l'image courante
      }
      break;
      
    case CONTEXT_BRUSH:
      context->Buffer_image = (byte *)malloc(width*height);
      if (! context->Buffer_image)
      {
        File_error=3;
        return;
      }
      context->Target_address=context->Buffer_image;
      
      break;
      
    case CONTEXT_SURFACE:
      context->Surface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCCOLORKEY, width, height, 8, 0, 0, 0, 0);
      if (! context->Surface)
      {
        File_error=1;
        return;
      }
      //context->Pitch = context->Surface->pitch;
      //context->Target_address = context->Surface->pixels;
      break;
  }

  if (File_error)
    return;
    
  // Extra process for truecolor images
  if (truecolor)
  {
    //context->Is_truecolor = 1;
    
    switch(context->Type)
    {
      case CONTEXT_MAIN_IMAGE:
      case CONTEXT_BRUSH:
      case CONTEXT_SURFACE:
        // Allocate 24bit buffer
        context->Buffer_image_24b=
          (T_Components *)malloc(width*height*sizeof(T_Components));
        if (!context->Buffer_image_24b)
        {
          // Print an error message
          // The following is to be sure the message is readable
          Compute_optimal_menu_colors(context->Palette);
          Message_out_of_memory();
          File_error=1;
        }
        break;
      
      case CONTEXT_PREVIEW:
        // Load palette
        Set_palette_fake_24b(context->Palette);
        Palette_loaded(context);
        break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//                    Gestion des lectures et écritures                    //
/////////////////////////////////////////////////////////////////////////////

void Write_one_byte(FILE *file, byte b)
{
  if (! Write_byte(file,b))
      File_error=1;
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
void Load_image(T_IO_Context *context)
{
  unsigned int index; // index de balayage des formats
  T_Format *format = &(File_formats[2]); // Format du fichier à charger
  int i;
  
  // Not sure it's the best place...
  context->Color_cycles=0;

  // On place par défaut File_error à vrai au cas où on ne sache pas
  // charger le format du fichier:
  File_error=1;

  if (context->Format>FORMAT_ALL_FILES)
  {
    format = Get_fileformat(context->Format);
    if (format->Test)
      format->Test(context);
  }

  if (File_error)
  {
    //  Sinon, on va devoir scanner les différents formats qu'on connait pour
    // savoir à quel format est le fichier:
    for (index=0; index < Nb_known_formats(); index++)
    {
      format = Get_fileformat(index);
      // Loadable format
      if (format->Test == NULL)
        continue;
        
      // On appelle le testeur du format:
      format->Test(context);
      // On s'arrête si le fichier est au bon format:
      if (File_error==0)
        break;
    }
  }
  
  if (File_error)
  {
    context->Format = DEFAULT_FILEFORMAT;
    // Last try: with SDL_image
    Load_SDL_Image(context);

    if (File_error)
    { 
      // Sinon, l'appelant sera au courant de l'échec grace à File_error;
      // et si on s'apprêtait à faire un chargement définitif de l'image (pas
      // une preview), alors on flash l'utilisateur.
      //if (Pixel_load_function!=Pixel_load_in_preview)
      //  Error(0);
      return;
    }
  }
  else
  // Si on a su déterminer avec succès le format du fichier:
  {
    context->Format = format->Identifier;
    // On peut charger le fichier:
    // Dans certains cas il est possible que le chargement plante
    // après avoir modifié la palette. TODO
    format->Load(context);
  }

  if (File_error>0)
  {
    fprintf(stderr,"Unable to load file %s (error %d)!\n",context->File_name, File_error);
    if (context->Type!=CONTEXT_SURFACE)
      Error(0);
  }
  
  // Post-load

  if (context->Buffer_image_24b)
  {
    // On vient de charger une image 24b
    if (!File_error)
    {
      switch(context->Type)
      {
        case CONTEXT_MAIN_IMAGE:
          // Cas d'un chargement dans l'image
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_HOURGLASS;
          Display_cursor();
          Flush_update();
          if (Convert_24b_bitmap_to_256(Main_backups->Pages->Image[0].Pixels,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
            File_error=2;
          else
          {
            Palette_loaded(context);
          }
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_ARROW;
          Display_cursor();
          break;
          
        case CONTEXT_BRUSH:
          // Cas d'un chargement dans la brosse
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_HOURGLASS;
          Display_cursor();
          Flush_update();
          if (Convert_24b_bitmap_to_256(context->Buffer_image,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
            File_error=2;
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_ARROW;
          Display_cursor();
          break;

        case CONTEXT_PREVIEW:
          // nothing to do
          break;
          
        case CONTEXT_SURFACE:
          if (Convert_24b_bitmap_to_256(context->Surface->pixels,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
          File_error=1;
          break;

      }
    }
    free(context->Buffer_image_24b);
    context->Buffer_image_24b = NULL;
  }
  else if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // Non-24b main image: Add menu colors
    if (Config.Safety_colors)
    {
      dword color_usage[256];
      memset(color_usage,0,sizeof(color_usage));
      if (Count_used_colors(color_usage)<252)
      {
        int gui_index;
        // From white to black
        for (gui_index=3; gui_index>=0; gui_index--)
        {
          int c;
          T_Components gui_color;
          
          gui_color=*Favorite_GUI_color(gui_index);
          // Try find a very close match (ignore last 2 bits)
          for (c=255; c>=0; c--)
          {
            if ((context->Palette[c].R|3) == (gui_color.R|3)
             && (context->Palette[c].G|3) == (gui_color.G|3) 
             && (context->Palette[c].B|3) == (gui_color.B|3) )
             break;
          }
          if (c<0) // Not found
          {
            // Find an unused slot at end of palette
            for (c=255; c>=0; c--)
            {
              if (color_usage[c]==0)
              {
                context->Palette[c]=gui_color;
                // Tag as a used color
                color_usage[c]=1;
                break;
              }
            }
          }
        }
      }
    }
  }

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    if ( File_error!=1)
    {
      Set_palette(context->Palette);
      if (format->Palette_only)
      {
        // Make a backup step
        Backup_layers(0);
      }
      // Copy the loaded palette
      memcpy(Main_palette, context->Palette, sizeof(T_Palette));
      memcpy(Main_backups->Pages->Palette, context->Palette, sizeof(T_Palette));

      // For formats that handle more than just the palette:
      // Transfer the data to main image.
      if (!format->Palette_only)
      {
        if (context->Original_file_name && context->Original_file_name[0]
          && context->Original_file_directory && context->Original_file_directory[0])
        {
          strcpy(Main_backups->Pages->Filename,context->Original_file_name);
          strcpy(Main_backups->Pages->File_directory,context->Original_file_directory);
        }
        else
        {
          strcpy(Main_backups->Pages->Filename,context->File_name);
          strcpy(Main_backups->Pages->File_directory,context->File_directory);
        }
        
        // On considère que l'image chargée n'est plus modifiée
        Main_image_is_modified=0;
        // Et on documente la variable Main_fileformat avec la valeur:
        Main_fileformat=format->Identifier;
  
        // already done initially on Backup_with_new_dimensions
        //Main_image_width= context->Width;
        //Main_image_height= context->Height;
        
        if (Main_backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
        {
          Main_current_layer = 0;
        }
        else
        {
          Main_current_layer = context->Nb_layers - 1;
          Main_layers_visible = (2<<Main_current_layer)-1;
        }
        
        // Load the transparency data
        Main_backups->Pages->Transparent_color = context->Transparent_color;
        Main_backups->Pages->Background_transparent = context->Background_transparent;
  
        // Correction des dimensions
        if (Main_image_width<1)
          Main_image_width=1;
        if (Main_image_height<1)
          Main_image_height=1;

        // Color cyling ranges:
        for (i=0; i<16; i++)
          Main_backups->Pages->Gradients->Range[i].Speed=0;
        for (i=0; i<context->Color_cycles; i++)
        {
          Main_backups->Pages->Gradients->Range[i].Start=context->Cycle_range[i].Start;
          Main_backups->Pages->Gradients->Range[i].End=context->Cycle_range[i].End;
          Main_backups->Pages->Gradients->Range[i].Inverse=context->Cycle_range[i].Inverse;
          Main_backups->Pages->Gradients->Range[i].Speed=context->Cycle_range[i].Speed;
        }
        
        // Comment
        strcpy(Main_backups->Pages->Comment, context->Comment);

      }
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
  else if (context->Type == CONTEXT_BRUSH && File_error==0)
  {
    
    if (Realloc_brush(context->Width, context->Height, context->Buffer_image, NULL))
    {
      File_error=3;
      free(context->Buffer_image);
    }
    memcpy(Brush_original_palette, context->Palette, sizeof(T_Palette));
    Remap_brush();

    context->Buffer_image = NULL;
  }
  else if (context->Type == CONTEXT_SURFACE)
  {
    if (File_error == 0)
    {
      // Copy the palette
      SDL_Color colors[256];
      int i;
      
      for (i=0; i<256; i++)
      {
        colors[i].r=context->Palette[i].R;
        colors[i].g=context->Palette[i].G;
        colors[i].b=context->Palette[i].B;
      }
      SDL_SetColors(context->Surface, colors, 0, 256);
    }
  }
  else if (context->Type == CONTEXT_PREVIEW
    /*&& !context->Buffer_image_24b*/
    /*&& !Get_fileformat(context->Format)->Palette_only*/)
  {
  
    // Try to adapt the palette to accomodate the GUI.
    int c;
    int count_unused;
    byte unused_color[4];
    
    count_unused=0;
    // Try find 4 unused colors and insert good colors there
    for (c=255; c>=0 && count_unused<4; c--)
    {
      if (!context->Preview_usage[c])
      {
        unused_color[count_unused]=c;
        count_unused++;
      }
    }
    // Found! replace them with some favorites
    if (count_unused==4)
    {
      int gui_index;
      for (gui_index=0; gui_index<4; gui_index++)
      {
        context->Palette[unused_color[gui_index]]=*Favorite_GUI_color(gui_index);
      }
    }
    // All preview display is here
    
    // Update palette and screen first
    Compute_optimal_menu_colors(context->Palette);
    Remap_screen_after_menu_colors_change();
    Set_palette(context->Palette);
    
    // Display palette preview
    if (Get_fileformat(context->Format)->Palette_only)
    {
      short index;
    
      if (context->Type == CONTEXT_PREVIEW)
        for (index=0; index<256; index++)
          Window_rectangle(183+(index/16)*7,95+(index&15)*5,5,5,index);
    
    }
    // Display normal image
    else if (context->Preview_bitmap)
    {
      int x_pos,y_pos;
      int width,height;
      width=context->Width/context->Preview_factor_X;
      height=context->Height/context->Preview_factor_Y;
      if (context->Ratio == PIXEL_WIDE && 
          Pixel_ratio != PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE2)
        width*=2;
      else if (context->Ratio == PIXEL_TALL && 
          Pixel_ratio != PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL2)
        height*=2;
      
      for (y_pos=0; y_pos<height;y_pos++)
        for (x_pos=0; x_pos<width;x_pos++)
        {
          byte color=context->Preview_bitmap[x_pos+y_pos*PREVIEW_WIDTH*Menu_factor_X];

          // Skip transparent if image has transparent background.
          if (color == context->Transparent_color && context->Background_transparent)
            color=MC_Window;

          Pixel(context->Preview_pos_X+x_pos,
                context->Preview_pos_Y+y_pos,
                color);
        }
    }    
    // Refresh modified part
    Update_window_area(183,95,PREVIEW_WIDTH,PREVIEW_HEIGHT);
    
    // Preview comment
    Print_in_window(45,70,context->Comment,MC_Black,MC_Light);
    //Update_window_area(45,70,32*8,8);

  }
  
}


// -- Sauver n'importe quel type connu de fichier d'image (ou palette) ------
void Save_image(T_IO_Context *context)
{
  T_Format *format;
  
  // On place par défaut File_error à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  File_error=1;

  switch (context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
      if ((!Get_fileformat(context->Format)->Supports_layers)
        && (Main_backups->Pages->Nb_layers > 1)
        && (!Get_fileformat(context->Format)->Palette_only))
      {
        if (Main_backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
        {
          if (! Confirmation_box("This format doesn't support\nanimation and will save only\ncurrent frame. Proceed?"))
          {
            // File_error is already set to 1.
            return;
          }
          // current layer
          context->Nb_layers=1;
          context->Target_address=Main_backups->Pages->Image[Main_current_layer].Pixels;
        }
        else // all other layer-based formats
        {
          int clicked_button;
          
          Open_window(208,100,"Format warning");
          Print_in_window( 8, 20,"This file format doesn't",MC_Black,MC_Light);
          Print_in_window( 8, 30,"support layers.",MC_Black,MC_Light);
          
          Window_set_normal_button(23,44, 162,14,"Save flattened copy",0,1,KEY_NONE); // 1
          Window_set_normal_button(23,62, 162,14,"Save current frame" ,0,1,KEY_NONE); // 2
          Window_set_normal_button(23,80, 162,14,"Cancel"             ,0,1,KEY_ESC);  // 3
          Update_window_area(0,0,Window_width, Window_height);
          Display_cursor();
          do
          {
            clicked_button=Window_clicked_button();
            // Some help on file formats ?
            //if (Is_shortcut(Key,0x100+BUTTON_HELP))
            //{
            //  Key=0;
            //  Window_help(???, NULL);
            //}
          }
          while (clicked_button<=0);
          Close_window();
          Display_cursor();
          switch(clicked_button)
          {
            case 1: // flatten
              context->Nb_layers=1;
              context->Target_address=Main_visible_image.Image;
              break;
            case 2: // current layer
              context->Nb_layers=1;
              context->Target_address=Main_backups->Pages->Image[Main_current_layer].Pixels;
              break;
            default: // Cancel
              // File_error is already set to 1.
              return;
          }
        }
      }
      break;
      
    case CONTEXT_BRUSH:
      break;
      
    case CONTEXT_PREVIEW:
      break;
      
    case CONTEXT_SURFACE:
      break;
  }

  format = Get_fileformat(context->Format);
  if (format->Save)
    format->Save(context);

  if (File_error)
  {
    Error(0);
    return;
  }
}  
   
   
void Load_SDL_Image(T_IO_Context *context)
{  
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  word x_pos,y_pos;
  // long file_size;
  dword pixel;
  long file_size;
  SDL_Surface * surface;


  Get_full_filename(filename, context->File_name, context->File_directory);
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
    
    Pre_load(context, surface->w, surface->h, file_size ,FORMAT_MISC, PIXEL_SIMPLE, 0);

    // Read palette
    if (surface->format->palette)
    {
      Get_SDL_Palette(surface->format->palette, context->Palette);
    }
    
    for (y_pos=0; y_pos<context->Height; y_pos++)
    {
      for (x_pos=0; x_pos<context->Width; x_pos++)
      {
        Set_pixel(context, x_pos, y_pos, Get_SDL_pixel_8(surface, x_pos, y_pos));
      }
    }

  }
  else
  {
    {
      // Hi/Trucolor
      Pre_load(context, surface->w, surface->h, file_size ,FORMAT_ALL_IMAGES, PIXEL_SIMPLE, 1);
    }
    
    for (y_pos=0; y_pos<context->Height; y_pos++)
    {
      for (x_pos=0; x_pos<context->Width; x_pos++)
      {
        pixel = Get_SDL_pixel_hicolor(surface, x_pos, y_pos);
        Set_pixel_24b(
          context,
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

///
/// Load an arbitrary SDL_Surface.
/// @param gradients Pass the address of a target T_Gradient_array if you want the gradients, NULL otherwise
SDL_Surface * Load_surface(char *full_name, T_Gradient_array *gradients)
{
  SDL_Surface * bmp=NULL;
  T_IO_Context context;
  
  Init_context_surface(&context, full_name, "");
  Load_image(&context);
  
  if (context.Surface)
  {
    bmp=context.Surface;
    // Caller wants the gradients:
    if (gradients != NULL)
    {
      int i;
      
      memset(gradients, 0, sizeof(T_Gradient_array));
      for (i=0; i<context.Color_cycles; i++)
      {
        gradients->Range[i].Start=context.Cycle_range[i].Start;
        gradients->Range[i].End=context.Cycle_range[i].End;
        gradients->Range[i].Inverse=context.Cycle_range[i].Inverse;
        gradients->Range[i].Speed=context.Cycle_range[i].Speed;
      }
    }
  } 
  Destroy_context(&context);

  return bmp;
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

  if (width == 0 || height == 0 || source == NULL)
    return;
  
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
  if (Main_backups && Main_backups->Pages && Main_backups->Pages->Nb_layers == 1)
    Emergency_backup(SAFETYBACKUP_PREFIX_A "999999" BACKUP_FILE_EXTENSION,Main_screen, Main_image_width, Main_image_height, &Main_palette);
  if (Spare_backups && Spare_backups->Pages && Spare_backups->Pages->Nb_layers == 1)
    Emergency_backup(SAFETYBACKUP_PREFIX_B "999999" BACKUP_FILE_EXTENSION,Spare_visible_image.Image, Spare_image_width, Spare_image_height, &Spare_palette);
}

T_Format * Get_fileformat(byte format)
{
  unsigned int i;
  T_Format * safe_default = File_formats;
  
  for (i=0; i < Nb_known_formats(); i++)
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

/// Query the color of a pixel (to save)
byte Get_pixel(T_IO_Context *context, short x, short y)
{
  return *(context->Target_address + y*context->Pitch + x);
}

/// Cleans up resources
void Destroy_context(T_IO_Context *context)
{
  free(context->Buffer_image_24b);
  free(context->Buffer_image);
  free(context->Preview_bitmap);
  memset(context, 0, sizeof(T_IO_Context));
}

/// Setup for loading a preview in fileselector
void Init_context_preview(T_IO_Context * context, char *file_name, char *file_directory)
{
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_PREVIEW;
  context->File_name = file_name;
  context->File_directory = file_directory;
  context->Format = Main_fileformat; // FIXME ?
}

// Setup for loading/saving an intermediate backup
void Init_context_backup_image(T_IO_Context * context, char *file_name, char *file_directory)
{
  Init_context_layered_image(context, file_name, file_directory);
}

/// Setup for loading/saving the current main image
void Init_context_layered_image(T_IO_Context * context, char *file_name, char *file_directory)
{
  int i;
  
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_MAIN_IMAGE;
  context->File_name = file_name;
  context->File_directory = file_directory;
  context->Format = Main_fileformat;
  memcpy(context->Palette, Main_palette, sizeof(T_Palette));
  context->Width = Main_image_width;
  context->Height = Main_image_height;
  context->Nb_layers = Main_backups->Pages->Nb_layers;
  strcpy(context->Comment, Main_backups->Pages->Comment);
  context->Transparent_color=Main_backups->Pages->Transparent_color;
  context->Background_transparent=Main_backups->Pages->Background_transparent;
  if (Pixel_ratio == PIXEL_WIDE || Pixel_ratio == PIXEL_WIDE2)
    context->Ratio=PIXEL_WIDE;
  else if (Pixel_ratio == PIXEL_TALL || Pixel_ratio == PIXEL_TALL2)
    context->Ratio=PIXEL_TALL;
  else
    context->Ratio=PIXEL_SIMPLE;
  context->Target_address=Main_backups->Pages->Image[0].Pixels;
  context->Pitch=Main_image_width;
  
  // Color cyling ranges:
  for (i=0; i<16; i++)
  {
    if (Main_backups->Pages->Gradients->Range[i].Start!=Main_backups->Pages->Gradients->Range[i].End)
    {
      context->Cycle_range[context->Color_cycles].Start=Main_backups->Pages->Gradients->Range[i].Start;
      context->Cycle_range[context->Color_cycles].End=Main_backups->Pages->Gradients->Range[i].End;
      context->Cycle_range[context->Color_cycles].Inverse=Main_backups->Pages->Gradients->Range[i].Inverse;
      context->Cycle_range[context->Color_cycles].Speed=Main_backups->Pages->Gradients->Range[i].Speed;
      context->Color_cycles++;
    }
  }
}

/// Setup for loading/saving the flattened version of current main image
//void Init_context_flat_image(T_IO_Context * context, char *file_name, char *file_directory)
//{

//}

/// Setup for loading/saving the user's brush
void Init_context_brush(T_IO_Context * context, char *file_name, char *file_directory)
{
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_BRUSH;
  context->File_name = file_name;
  context->File_directory = file_directory;
  context->Format = Brush_fileformat;
  // Use main screen's palette
  memcpy(context->Palette, Main_palette, sizeof(T_Palette));
  context->Width = Brush_width;
  context->Height = Brush_height;
  context->Nb_layers = 1;
  // Solid save... could use BG color maybe
  context->Transparent_color=0;
  context->Background_transparent=0;
  context->Ratio=PIXEL_SIMPLE;
  context->Target_address=Brush;
  context->Pitch=Brush_width;

}

// Setup for loading an image into a new SDL surface.
void Init_context_surface(T_IO_Context * context, char *file_name, char *file_directory)
{
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_SURFACE;
  context->File_name = file_name;
  context->File_directory = file_directory;
  context->Format = DEFAULT_FILEFORMAT;
  // context->Palette
  // context->Width
  // context->Height
  context->Nb_layers = 1;
  context->Transparent_color=0;
  context->Background_transparent=0;
  context->Ratio=PIXEL_SIMPLE;
  //context->Target_address
  //context->Pitch

}
/// Function to call when need to switch layers.
void Set_saving_layer(T_IO_Context *context, int layer)
{
  context->Current_layer = layer;

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    if (context->Nb_layers==1 && Main_backups->Pages->Nb_layers!=1)
    {
      // Context is set to saving a single layer: do nothing
    }
    else
    {
      context->Target_address=Main_backups->Pages->Image[layer].Pixels;
    }
  }
}

/// Function to call when need to switch layers.
void Set_loading_layer(T_IO_Context *context, int layer)
{
  context->Current_layer = layer;

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // This awful thing is the part that happens on load
    while (layer >= context->Nb_layers)
    {
      if (Add_layer(Main_backups, layer))
      {
        // Failure to add a layer on load:
        // Position on last layer
        layer = context->Nb_layers-1;
        break;
      }
      context->Nb_layers = Main_backups->Pages->Nb_layers;
      Main_layers_visible = (2<<layer)-1;
    }
    Main_current_layer = layer;
    context->Target_address=Main_backups->Pages->Image[layer].Pixels;
    
    Update_pixel_renderer();
  }
}

// ============================================
// Safety backups
// ============================================


typedef struct T_String_list
{
  char * String;
  struct T_String_list * Next;
} T_String_list;

/// A list of files, used for scanning a directory
T_String_list *Backups_main = NULL;
/// A list of files, used for scanning a directory
T_String_list *Backups_spare = NULL;


// Settings for safety backup (frequency, numbers, etc)

const int Rotation_safety_backup = 8;

const int Min_interval_for_safety_backup = 30000;
const int Min_edits_for_safety_backup = 10;

const int Max_interval_for_safety_backup = 60000;
const int Max_edits_for_safety_backup = 30;

///
/// Adds a file to Backups_main or Backups_spare lists, if it's a backup.
///
void Add_backup_file(const char *name)
{
  T_String_list ** list;
  T_String_list * elem;
  int i;
  char file_name[MAX_PATH_CHARACTERS];
  
  // Only files names of the form a0000000.* and b0000000.* are expected
  
  Extract_filename(file_name, name);
  
  // Check first character
  if (file_name[0]==Main_safety_backup_prefix)
    list = &Backups_main;
  else if (file_name[0]==Spare_safety_backup_prefix)
    list = &Backups_spare;
   else {
    // Not a good file
    return;
  }
  
  // Check next characters till file extension
  i = 1;
  while (file_name[i]!='\0' && file_name[i]!='.')
  {
    if (file_name[i]< '0' || file_name[i] > '9')
    {
      // Not a good file
      return;
    }
    i++;
  }
  
  // Add to list (top insertion)
  elem = (T_String_list *)malloc(sizeof(T_String_list));
  elem->String=strdup(file_name);
  elem->Next=*list;
  *list=elem;
}


/// String comparer for sorting
int String_compare (const void * a, const void * b)
{
  return strcmp(*(char**)a,*(char**)b);
}

///
/// Reload safety backups, by loading several files in the right order.
///
byte Process_backups(T_String_list **list)
{
  int nb_files;
  int i;
  char ** files_vector;
  T_String_list *element;
  byte backup_max_undo_pages;

  if (*list == NULL)
    return 0;

  // Save the maximum number of pages
  // (It's used in Create_new_page() which gets called on each Load_image)
  backup_max_undo_pages = Config.Max_undo_pages;
  Config.Max_undo_pages = 99;

  // Count files
  nb_files=0;
  element=*list;
  while (element != NULL)
  {
    nb_files++;
    element = element->Next;
  }
  // Allocate a vector
  files_vector = (char **)malloc(sizeof(char *) * nb_files);
  // Copy from list to vector
  for (i=0;i<nb_files;i++)
  {
    T_String_list *next;
    
    files_vector[i]=(*list)->String;
    next = (*list)->Next;
    free(*list);
    *list = next;
  }
  
  // Sort the vector
  qsort (files_vector, nb_files , sizeof(char **), String_compare);
  
  for (i=0; i < nb_files; i++)
  {
    // Load this file
    T_IO_Context context;
    char file_name[MAX_PATH_CHARACTERS]="";
    char file_directory[MAX_PATH_CHARACTERS]="";
    
    Init_context_backup_image(&context, files_vector[i], Config_directory);
    // Provide buffers to read original location
    context.Original_file_name = file_name;
    context.Original_file_directory = file_directory;
    Load_image(&context);
    Main_image_is_modified=1;
    Destroy_context(&context);
    Redraw_layered_image();
    Display_all_screen();
  }

  // Done with the vector
  for (i=0; i < nb_files; i++)
  {
    free(files_vector[i]);
  }
  free(files_vector);
  files_vector = NULL;
  
  // Restore the maximum number of pages
  Config.Max_undo_pages = backup_max_undo_pages;
  
  return nb_files;
}


/// Global indicator that tells if the safety backup system is active
byte Safety_backup_active = 0;

///
/// Checks if there are any pending safety backups, and then opens them.
/// @return 0 if no problem, -1 if the backup system cannot be activated, >=1 if some backups are restored
int Check_recovery(void)
{
  int restored_spare;
  int restored_main;
  
  // First check if can write backups
#if defined (__MINT__) 
   //TODO: enable file lock under Freemint only
   return 0;
#else  
if (Create_lock_file(Config_directory))
    return -1;
#endif

  Safety_backup_active=1;
    
  Backups_main = NULL;
  Backups_spare = NULL;
  For_each_file(Config_directory, Add_backup_file);
  
  // Do the processing twice: once for possible backups of the main page,
  // once for possible backups of the spare.  
  
  restored_spare = Process_backups(&Backups_spare);
  if (restored_spare)
  {
    Main_offset_X=0;
    Main_offset_Y=0;
    Compute_limits();
    Compute_paintbrush_coordinates();
    if (Backups_main)
      Button_Page();
  }
  restored_main = Process_backups(&Backups_main);
  
  if (restored_main)
  {
    Main_offset_X=0;
    Main_offset_Y=0;
    Compute_limits();
    Compute_paintbrush_coordinates();
  }
  return restored_main + restored_spare;
}

void Rotate_safety_backups(void)
{
  Uint32 now;
  T_IO_Context context;
  char file_name[12+1];
  char deleted_file[MAX_PATH_CHARACTERS];

  if (!Safety_backup_active)
    return;
    
  now = SDL_GetTicks();
  // It's time to save if either:
  // - Many edits have taken place
  // - A minimum number of edits have taken place AND a minimum time has passed
  // - At least one edit was done, and a maximum time has passed
  if ((Main_edits_since_safety_backup > Max_edits_for_safety_backup) ||
      (Main_edits_since_safety_backup > Min_edits_for_safety_backup && 
      now > Main_time_of_safety_backup + Min_interval_for_safety_backup) ||
      (Main_edits_since_safety_backup > 1 && 
      now > Main_time_of_safety_backup + Max_interval_for_safety_backup))
  {
    
    // Clear a previous save (rotating saves)
    sprintf(deleted_file, "%s%c%6.6d" BACKUP_FILE_EXTENSION,
      Config_directory,
      Main_safety_backup_prefix,
      (Uint32)(Main_safety_number + 1000000l - Rotation_safety_backup) % (Uint32)1000000l);
    remove(deleted_file); // no matter if fail
    
    // Reset counters
    Main_edits_since_safety_backup=0;
    Main_time_of_safety_backup=now;

    // Create a new file name and save
    sprintf(file_name, "%c%6.6d" BACKUP_FILE_EXTENSION,
      Main_safety_backup_prefix,
      (Uint32)Main_safety_number);
    Init_context_backup_image(&context, file_name, Config_directory);
    context.Format=FORMAT_GIF;
    // Provide original file data, to store as a GIF Application Extension
    context.Original_file_name = Main_backups->Pages->Filename;
    context.Original_file_directory = Main_backups->Pages->File_directory;
    
    Save_image(&context);
    Destroy_context(&context);
    
    Main_safety_number++;
  }
}

/// Remove safety backups. Need to call on normal program exit.
void Delete_safety_backups(void)
{
  T_String_list *element;

  if (!Safety_backup_active)
    return;
  
  Backups_main = NULL;
  Backups_spare = NULL;
  
  For_each_file(Config_directory, Add_backup_file);
  
  chdir(Config_directory);
  for (element=Backups_main; element!=NULL; element=element->Next)
  {
    if(remove(element->String))
      printf("Failed to delete %s\n",element->String);
  }
  for (element=Backups_spare; element!=NULL; element=element->Next)
  {
    if(remove(element->String))
      printf("Failed to delete %s\n",element->String);
  }
  
  // Release lock file
#if defined (__MINT__) 
  //TODO: release file lock under Freemint only
#else 
  Release_lock_file(Config_directory);
#endif
  
}
