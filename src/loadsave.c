/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#if defined(WIN32)
#include <windows.h>
#if defined(_MSC_VER)
#define strdup _strdup
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif
#endif
#include <limits.h>
#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_endian.h>
#endif
#ifndef __no_pnglib__
#include <png.h>
#endif

#include "gfx2log.h"
#include "gfx2mem.h"
#include "buttons.h"
#include "const.h"
#include "errors.h"
#include "global.h"
#include "keycodes.h"
#include "io.h"
#include "loadsave.h"
#include "misc.h"
#include "graph.h"
#include "op_c.h"
#include "pages.h"
#include "palette.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include "sdlscreen.h"
#endif
#include "screen.h"
#include "struct.h"
#include "windows.h"
#include "engine.h"
#include "brush.h"
#include "setup.h"
#include "filesel.h"
#include "unicode.h"
#include "fileformats.h"
#include "bitcount.h"

#if defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
#include "input.h"
#if defined(USE_X11)
extern Display * X11_display;
extern Window X11_window;
#endif
#endif

#if defined(__macosx__)
const void * get_tiff_paste_board(unsigned long * size);
int set_tiff_paste_board(const void * tiff, unsigned long size);
#endif


#if defined(USE_SDL) || defined(USE_SDL2)
// -- SDL_Image -------------------------------------------------------------
// (TGA, BMP, PNM, XPM, XCF, PCX, GIF, JPG, TIF, IFF, PNG, ICO)
void Load_SDL_Image(T_IO_Context *);
#endif

// -- Recoil ----------------------------------------------------------------
// 8bits and 16bits computer graphics
#ifndef NORECOIL
void Load_Recoil_Image(T_IO_Context *);
#endif

// clipboard
static void Load_ClipBoard_Image(T_IO_Context *);
static void Save_ClipBoard_Image(T_IO_Context *);


// ENUM     Name  TestFunc LoadFunc SaveFunc PalOnly Comment Layers Ext Exts  
const T_Format File_formats[] = {
  {FORMAT_ALL_IMAGES, "(all)", NULL, NULL, NULL, 0, 0, 0, "",
    "gif;png;bmp;2bp;pcx;pkm;iff;lbm;ilbm;sham;ham;ham6;ham8;acbm;pic;anim;img;sci;scq;scf;scn;sco;cel;"
    "pi1;pc1;pi2;pc2;pi3;pc3;neo;"
    "c64;p64;a64;pi;rp;aas;art;dd;iph;ipt;hpc;ocp;koa;koala;fli;bml;cdu;prg;pmg;rpm;"
    "gpx;"
    "cpc;scr;win;"
    "hgr;dhgr;"
    "tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff;ico;ic2;cur;cm5;pph;info;flc;bin;map"},
  {FORMAT_ALL_PALETTES, "(pal)", NULL, NULL, NULL, 1, 0, 0, "", "kcf;pal;gpl"},
  {FORMAT_ALL_FILES, "(*.*)", NULL, NULL, NULL, 0, 0, 0, "", "*"},
  {FORMAT_GIF, " gif", Test_GIF, Load_GIF, Save_GIF, 0, 1, 1, "gif", "gif"},
#ifndef __no_pnglib__
  {FORMAT_PNG, " png", Test_PNG, Load_PNG, Save_PNG, 0, 1, 0, "png", "png"},
#endif
  {FORMAT_BMP, " bmp", Test_BMP, Load_BMP, Save_BMP, 0, 0, 0, "bmp", "bmp;2bp"},
  {FORMAT_PCX, " pcx", Test_PCX, Load_PCX, Save_PCX, 0, 0, 0, "pcx", "pcx"},
  {FORMAT_PKM, " pkm", Test_PKM, Load_PKM, Save_PKM, 0, 1, 0, "pkm", "pkm"},
  {FORMAT_LBM, " lbm", Test_LBM, Load_IFF, Save_IFF, 0, 1, 0, "iff", "iff;lbm;ilbm;sham;ham;ham6;ham8;anim"},
  {FORMAT_PBM, " pbm", Test_PBM, Load_IFF, Save_IFF, 0, 1, 0, "iff", "iff;pbm;lbm"},
  {FORMAT_ACBM," acbm",Test_ACBM,Load_IFF, NULL,     0, 1, 0, "iff", "iff;pic;acbm"},
  {FORMAT_IMG, " img", Test_IMG, Load_IMG, Save_IMG, 0, 0, 0, "img", "img"},
  {FORMAT_SCx, " sc?", Test_SCx, Load_SCx, Save_SCx, 0, 0, 0, "sc?", "sci;scq;scf;scn;sco"},
  {FORMAT_PI1, " pi1", Test_PI1, Load_PI1, Save_PI1, 0, 0, 0, "pi1", "pi1;pi2;pi3"},
  {FORMAT_PC1, " pc1", Test_PC1, Load_PC1, Save_PC1, 0, 0, 0, "pc1", "pc1;pc2;pc3"},
  {FORMAT_CEL, " cel", Test_CEL, Load_CEL, Save_CEL, 0, 0, 0, "cel", "cel"},
  {FORMAT_NEO, " neo", Test_NEO, Load_NEO, Save_NEO, 0, 0, 0, "neo", "neo"},
  {FORMAT_KCF, " kcf", Test_KCF, Load_KCF, Save_KCF, 1, 0, 0, "kcf", "kcf"},
  {FORMAT_PAL, " pal", Test_PAL, Load_PAL, Save_PAL, 1, 0, 0, "pal", "pal"},
  {FORMAT_GPL, " gpl", Test_GPL, Load_GPL, Save_GPL, 1, 0, 0, "gpl", "gpl"},
  {FORMAT_C64, " c64", Test_C64, Load_C64, Save_C64, 0, 1, 1, "c64",
    "c64;p64;a64;pi;rp;aas;art;dd;iph;ipt;hpc;ocp;koa;koala;fli;bml;cdu;prg;pmg;rpm"},
  {FORMAT_GPX, " gpx", Test_GPX, Load_GPX, NULL,     0, 0, 0, "gpx", "gpx"},
  {FORMAT_SCR, " cpc", Test_SCR, Load_SCR, Save_SCR, 0, 0, 0, "scr", "cpc;scr;win"},
  {FORMAT_CM5, " cm5", Test_CM5, Load_CM5, Save_CM5, 0, 0, 1, "cm5", "cm5"},
  {FORMAT_PPH, " pph", Test_PPH, Load_PPH, Save_PPH, 0, 0, 1, "pph", "pph"},
  {FORMAT_XPM, " xpm", NULL,     NULL,     Save_XPM, 0, 0, 0, "xpm", "xpm"},
  {FORMAT_ICO, " ico", Test_ICO, Load_ICO, Save_ICO, 0, 0, 0, "ico", "ico;ic2;cur"},
  {FORMAT_INFO," info",Test_INFO,Load_INFO,NULL,     0, 0, 0, "info", "info"},
  {FORMAT_FLI, " flc", Test_FLI, Load_FLI, NULL,     0, 0, 0, "flc", "flc;fli;dat"},
  {FORMAT_MOTO," moto",Test_MOTO,Load_MOTO,Save_MOTO,0, 1, 0, "bin", "bin;map"},
  {FORMAT_HGR, " hgr", Test_HGR, Load_HGR, Save_HGR, 0, 0, 1, "hgr", "hgr;dhgr;bin"},
#ifndef __no_tifflib__
  {FORMAT_TIFF," tiff",Test_TIFF,Load_TIFF,Save_TIFF,0, 1, 1, "tif", "tif;tiff"},
#endif
  {FORMAT_MISC,"misc.",NULL,     NULL,     NULL,     0, 0, 0, "",    "tga;pnm;xpm;xcf;jpg;jpeg;tif;tiff"},
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
          Pixel_ratio != PIXEL_TALL2 &&
          Pixel_ratio != PIXEL_TALL3)
        {
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y*2)*PREVIEW_WIDTH*Menu_factor_X]=color;
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y*2+1)*PREVIEW_WIDTH*Menu_factor_X]=color;
        }
        else
          context->Preview_bitmap[x_pos/context->Preview_factor_X + (y_pos/context->Preview_factor_Y)*PREVIEW_WIDTH*Menu_factor_X]=color;
      }

      break;
      
    // Load pixels into a Surface
    case CONTEXT_SURFACE:
      if (x_pos>=0 && y_pos>=0 && x_pos<context->Surface->w && y_pos<context->Surface->h)
        Set_GFX2_Surface_pixel(context->Surface, x_pos, y_pos, color);
      break;
      
    case CONTEXT_PALETTE:
    case CONTEXT_PREVIEW_PALETTE:
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
        Main.backups->Pages->Image[Main.current_layer].Pixels,
        color,
        Main.backups->Pages->Width*Main.backups->Pages->Height);
      break;
    case CONTEXT_BRUSH:
      memset(context->Buffer_image, color, (long)context->Height*context->Pitch);
      break;
    case CONTEXT_SURFACE:
      break;
    case CONTEXT_PALETTE:
    case CONTEXT_PREVIEW_PALETTE:
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

    case CONTEXT_PREVIEW_PALETTE:
    case CONTEXT_PALETTE:
      // In a palette, there are no pixels!
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
      Main.backups->Pages->Image[context->Current_layer].Duration = duration;
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
      return Main.backups->Pages->Image[context->Current_layer].Duration;
    default:
      return 0;
  }
}

void Set_image_mode(T_IO_Context *context, enum IMAGE_MODES mode)
{
  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    Main.backups->Pages->Image_mode = mode;
    //Switch_layer_mode(mode);
    Update_screen_targets();
    if (mode > IMAGE_MODE_ANIMATION)
      Selected_Constraint_Mode = mode;
    // update the "FX" button state
    Draw_menu_button(BUTTON_EFFECTS,Any_effect_active());
  }
}

///
/// Generic allocation and similar stuff, done at beginning of image load,
/// as soon as size is known.
void Pre_load(T_IO_Context *context, short width, short height, long file_size, int format, enum PIXEL_RATIO ratio, byte bpp)
{
  char  str[10];
  byte truecolor;

  if (width < 0 || width > 9999 || height < 0 || height > 9999)
  {
    File_error = 1;
    return;
  }

  if (bpp == 0)
    bpp = 8;  // default to 8bits
  truecolor = (bpp > 8) ? 1 : 0;
  context->bpp = bpp;
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
      memcpy(str, "VERY BIG!", 10); // default string
      if (context->Original_width != 0)
      {
        if (context->Original_width < 10000 && context->Original_height < 10000)
        {
          Num2str(context->Original_width,str,4);
          Num2str(context->Original_height,str+5,4);
          str[4]='x';
        }
      }
      else if ((width<10000) && (height<10000))
      {
        Num2str(width,str,4);
        Num2str(height,str+5,4);
        str[4]='x';
      }
      Print_in_window(101,59,str,MC_Black,MC_Light);
      snprintf(str, sizeof(str), "%2dbpp", bpp);
      Print_in_window(181,59,str,MC_Black,MC_Light);
  
      // Affichage de la taille du fichier
      if (file_size<1048576)
      {
        // Le fichier fait moins d'un Mega, on affiche sa taille direct
        Num2str(file_size,str,7);
      }
      else if (((file_size+512)/1024)<100000)
      {
        // Le fichier fait plus d'un Mega, on peut afficher sa taille en Ko
        Num2str((file_size+512)/1024,str,5);
        strcpy(str+5,"KB");
      }
      else
      {
        // Le fichier fait plus de 100 Mega octets (cas très rare :))
        memcpy(str,"LARGE!!",8);
      }
      Print_in_window(236,59,str,MC_Black,MC_Light);
  
      // Affichage du vrai format
      Print_in_window( 59,59,Get_fileformat(format)->Label,MC_Black,MC_Light);

      // On efface le commentaire précédent
      Window_rectangle(45,70,32*8,8,MC_Light);
  
      // Calcul des données nécessaires à l'affichage de la preview:
      if (ratio == PIXEL_WIDE && 
          Pixel_ratio != PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE2)
        width*=2;
      else if (ratio == PIXEL_TALL && 
          Pixel_ratio != PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL2 &&
          Pixel_ratio != PIXEL_TALL3)
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
        Main.current_layer=0;
        Main.layers_visible=1<<0;
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
      context->Buffer_image = (byte *)GFX2_malloc(width*height);
      if (! context->Buffer_image)
      {
        File_error=3;
        return;
      }
      context->Target_address=context->Buffer_image;
      
      break;
      
    case CONTEXT_SURFACE:
      context->Surface = New_GFX2_Surface(width, height);
      if (! context->Surface)
      {
        File_error=1;
        return;
      }
      //context->Pitch = context->Surface->pitch;
      //context->Target_address = context->Surface->pixels;
      break;

    case CONTEXT_PALETTE:
    case CONTEXT_PREVIEW_PALETTE:
      // In a palette, there are no pixels!
      break;
  }

  if (File_error)
    return;
    
  // Extra process for truecolor images
  if (truecolor)
  {
    switch(context->Type)
    {
      case CONTEXT_MAIN_IMAGE:
      case CONTEXT_BRUSH:
      case CONTEXT_SURFACE:
        // Allocate 24bit buffer
        context->Buffer_image_24b =
          (T_Components *)GFX2_malloc(width*height*sizeof(T_Components));
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
        // 3:3:2 "True Color" palette will be loaded lated in context->Palette
        // There can be an image palette used to decode the true color picture
        // Such as for HAM ILBM pictures
        break;

      case CONTEXT_PALETTE:
    case CONTEXT_PREVIEW_PALETTE:
        // In a palette, there are no pixels!
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
  const T_Format *format = &(File_formats[FORMAT_ALL_FILES+1]); // Format du fichier à charger
  int i;
  byte old_cursor_shape;
  FILE * f;
  
  // Not sure it's the best place...
  context->Color_cycles=0;

  // On place par défaut File_error à vrai au cas où on ne sache pas
  // charger le format du fichier:
  File_error=1;

  if (context->Format == FORMAT_CLIPBOARD)
  {
    Load_ClipBoard_Image(context);
    if (File_error != 0 && context->Format == FORMAT_CLIPBOARD)
    {
      Error(0);
      return;
    }
  }
  if (context->Format != FORMAT_CLIPBOARD)
  {
    if (context->File_name == NULL || context->File_directory == NULL)
    {
      GFX2_Log(GFX2_ERROR, "Load_Image() called with NULL file name or directory\n");
      Error(0);
      return;
    }

    f = Open_file_read(context);
    if (f == NULL)
    {
      Warning("Cannot open file for reading");
      Error(0);
      return;
    }

    if (context->Format > FORMAT_ALL_FILES)
    {
      format = Get_fileformat(context->Format);
      if (format->Test)
        format->Test(context, f);
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

        fseek(f, 0, SEEK_SET); // rewind
        // On appelle le testeur du format:
        format->Test(context, f);
        // On s'arrête si le fichier est au bon format:
        if (File_error==0)
          break;
      }
    }
    fclose(f);

    if (File_error)
    {
      context->Format = DEFAULT_FILEFORMAT;
      // try with recoil
#ifndef NORECOIL
      Load_Recoil_Image(context);
      if (File_error)
#endif
#if defined(USE_SDL) || defined(USE_SDL2)
      {
        // Last try: with SDL_image
        Load_SDL_Image(context);
      }

      if (File_error)
#endif
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
      GFX2_Log(GFX2_WARNING, "Unable to load file %s (error %d)! format:%s\n", context->File_name, File_error, format->Label);
      if (context->Type!=CONTEXT_SURFACE)
        Error(0);
    }
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
          old_cursor_shape=Cursor_shape;
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_HOURGLASS;
          Display_cursor();
          Flush_update();
          if (Convert_24b_bitmap_to_256(Main.backups->Pages->Image[0].Pixels,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
            File_error=2;
          Hide_cursor();
          Cursor_shape=old_cursor_shape;
          Display_cursor();
          break;
          
        case CONTEXT_BRUSH:
          // Cas d'un chargement dans la brosse
          old_cursor_shape=Cursor_shape;
          Hide_cursor();
          Cursor_shape=CURSOR_SHAPE_HOURGLASS;
          Display_cursor();
          Flush_update();
          if (Convert_24b_bitmap_to_256(context->Buffer_image,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
            File_error=2;
          Hide_cursor();
          Cursor_shape=old_cursor_shape;
          Display_cursor();
          break;

        case CONTEXT_PREVIEW:
          // in this context, context->Buffer_image_24b is not allocated
          // pixels are drawn to context->Preview_bitmap
          break;
          
        case CONTEXT_SURFACE:
          if (Convert_24b_bitmap_to_256(context->Surface->pixels,context->Buffer_image_24b,context->Width,context->Height,context->Palette))
          File_error=1;
          break;


        case CONTEXT_PALETTE:
        case CONTEXT_PREVIEW_PALETTE:
         // In a palette, there are no pixels!
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
        Backup_layers(LAYER_NONE);
      }
      // Copy the loaded palette
      memcpy(Main.palette, context->Palette, sizeof(T_Palette));
      memcpy(Main.backups->Pages->Palette, context->Palette, sizeof(T_Palette));

      // For formats that handle more than just the palette:
      // Transfer the data to main image.
      if (!format->Palette_only)
      {
        free(Main.backups->Pages->Filename);
        free(Main.backups->Pages->File_directory);
        free(Main.backups->Pages->Filename_unicode);
        Main.backups->Pages->Filename_unicode = NULL;
        if (context->Original_file_name != NULL
          && context->Original_file_directory != NULL)
        {
          Main.backups->Pages->Filename = context->Original_file_name;  // steal buffer !
          context->Original_file_name = NULL;
          Main.backups->Pages->File_directory = context->Original_file_directory;  // steal heap buffer
          context->Original_file_directory = NULL;
        }
        else
        {
          Main.backups->Pages->Filename = strdup(context->File_name);
          Main.backups->Pages->File_directory = strdup(context->File_directory);
          if (context->File_name_unicode)
            Main.backups->Pages->Filename_unicode = Unicode_strdup(context->File_name_unicode);
        }
        
        // On considère que l'image chargée n'est plus modifiée
        Main.image_is_modified=0;
        // Et on documente la variable Main_fileformat avec la valeur:
        Main.fileformat=format->Identifier;
  
        // already done initially on Backup_with_new_dimensions
        //Main_image_width= context->Width;
        //Main_image_height= context->Height;
        
        if (Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
        {
          Main.current_layer = 0;
        }
        else
        {
          Main.current_layer = context->Nb_layers - 1;
          Main.layers_visible = (2<<Main.current_layer)-1;
        }
        
        // Load the transparency data
        Main.backups->Pages->Transparent_color = context->Transparent_color;
        Main.backups->Pages->Background_transparent = context->Background_transparent;
  
        // Correction des dimensions
        if (Main.image_width<1)
          Main.image_width=1;
        if (Main.image_height<1)
          Main.image_height=1;

        // Color cyling ranges:
        for (i=0; i<16; i++)
          Main.backups->Pages->Gradients->Range[i].Speed=0;
        for (i=0; i<context->Color_cycles; i++)
        {
          Main.backups->Pages->Gradients->Range[i].Start=context->Cycle_range[i].Start;
          Main.backups->Pages->Gradients->Range[i].End=context->Cycle_range[i].End;
          Main.backups->Pages->Gradients->Range[i].Inverse=context->Cycle_range[i].Inverse;
          Main.backups->Pages->Gradients->Range[i].Speed=context->Cycle_range[i].Speed;
        }
        
        // Comment
        strcpy(Main.backups->Pages->Comment, context->Comment);

      }
    }
    else if (File_error!=1)
    {
      // On considère que l'image chargée est encore modifiée
      Main.image_is_modified=1;
      // Et on documente la variable Main_fileformat avec la valeur:
      Main.fileformat=format->Identifier;
    }
    else
    {
      // Dans ce cas, on sait que l'image n'a pas changé, mais ses
      // paramètres (dimension, palette, ...) si. Donc on les restaures.
      Download_infos_page_main(Main.backups->Pages);
    }
  }
  else if (context->Type == CONTEXT_PALETTE)
  {
    if ( File_error!=1)
    {
      Set_palette(context->Palette);
      // Make a backup step
      Backup_layers(LAYER_NONE);
      // Copy the loaded palette
      memcpy(Main.palette, context->Palette, sizeof(T_Palette));
      memcpy(Main.backups->Pages->Palette, context->Palette, sizeof(T_Palette));
    }
  }
  else if (context->Type == CONTEXT_BRUSH && File_error==0)
  {
    // For brushes, Back_color is the transparent color.
    // Set it before remapping, Remap_brush() will take care of it
    if (context->Background_transparent)
      Back_color = context->Transparent_color;
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
      memcpy(context->Surface->palette, context->Palette, sizeof(T_Palette));
    }
  }
  else if (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE
    /*&& !context->Buffer_image_24b*/
    /*&& !Get_fileformat(context->Format)->Palette_only*/)
  {
  
    // Try to adapt the palette to accomodate the GUI.
    int c;
    int count_unused;
    byte unused_color[4];
    
    if (context->Type == CONTEXT_PREVIEW && context->bpp > 8)
      Set_palette_fake_24b(context->Palette);

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
    if (Get_fileformat(context->Format)->Palette_only
        || context->Type == CONTEXT_PREVIEW_PALETTE)
    {
      short index;
    
      if (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE)
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
          Pixel_ratio != PIXEL_TALL2 &&
          Pixel_ratio != PIXEL_TALL3)
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
  const T_Format *format;
  
  // On place par défaut File_error à vrai au cas où on ne sache pas
  // sauver le format du fichier: (Est-ce vraiment utile??? Je ne crois pas!)
  File_error=1;
  format = Get_fileformat(context->Format);

  switch (context->Type)
  {
    case CONTEXT_MAIN_IMAGE:
      if ((context->Format == FORMAT_CLIPBOARD || !format->Supports_layers)
        && (Main.backups->Pages->Nb_layers > 1)
        && (!format->Palette_only))
      {
        if (Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
        {
          if (! Confirmation_box("This format doesn't support\nanimation and will save only\ncurrent frame. Proceed?"))
          {
            // File_error is already set to 1.
            return;
          }
          // current layer
          context->Nb_layers=1;
          context->Target_address=Main.backups->Pages->Image[Main.current_layer].Pixels;
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
              context->Target_address=Main.visible_image.Image;
              break;
            case 2: // current layer
              context->Nb_layers=1;
              context->Target_address=Main.backups->Pages->Image[Main.current_layer].Pixels;
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
    case CONTEXT_PREVIEW_PALETTE:
      break;
      
    case CONTEXT_SURFACE:
      break;

    case CONTEXT_PALETTE:
      // In a palette, there are no pixels!
      break;
  }

  if (context->Format == FORMAT_CLIPBOARD)
    Save_ClipBoard_Image(context);
  else
  {
    if (format->Save)
      format->Save(context);
  }

  if (File_error)
  {
    Error(0);
    return;
  }
}  


#if defined(USE_SDL) || defined(USE_SDL2)
void Load_SDL_Image(T_IO_Context *context)
{
  char * filename;  // full path
  word x_pos,y_pos;
  // long file_size;
  dword pixel;
  long file_size;
  SDL_Surface * surface;

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  file_size = File_length(filename);
  File_error = 0;

  surface = IMG_Load(filename);
  free(filename);

  if (!surface)
  {
    File_error=1;
    return;
  }

  if (surface->format->BytesPerPixel == 1)
  {
    // 8bpp image
    Pre_load(context, surface->w, surface->h, file_size ,FORMAT_MISC, PIXEL_SIMPLE, 8);

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
      Pre_load(context, surface->w, surface->h, file_size ,FORMAT_ALL_IMAGES, PIXEL_SIMPLE, 8 * surface->format->BytesPerPixel);
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
#endif

///
/// Load an arbitrary Surface.
/// @param full_name Full (absolute) path of the file to load.
/// @param gradients Pass the address of a target T_Gradient_array if you want the gradients, NULL otherwise
T_GFX2_Surface * Load_surface(char *full_name, T_Gradient_array *gradients)
{
  T_GFX2_Surface * bmp=NULL;
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


static void Load_ClipBoard_Image(T_IO_Context * context)
{
#ifdef WIN32
  UINT format;
  HANDLE clipboard;

  if (!OpenClipboard(GFX2_Get_Window_Handle()))
  {
    GFX2_Log(GFX2_ERROR, "Failed to open Clipboard\n");
    return;
  }

  format = EnumClipboardFormats(0);
  while (format != 0)
  {
    const char * format_name = NULL;
    char format_name_buffer[256];

    switch (format)
    {
      case CF_TEXT:
        format_name = "TEXT";
        break;
      case CF_OEMTEXT:
        format_name = "OEMTEXT";
        break;
      case CF_UNICODETEXT:
        format_name = "UNICODE TEXT";
        break;
#if(WINVER >= 0x0400)
      case CF_LOCALE:
        format_name = "Locale identifier";
        break;
      case CF_HDROP:
        format_name = "Drop Handle";
        break;
#endif
      case CF_DIB:
        format_name = "DIB (BITMAPINFO)";
        break;
      case CF_DIBV5:
        format_name = "DIBV5 (BITMAPV5HEADER)";
        break;
      case CF_BITMAP:
        format_name = "HBITMAP";
        break;
      case CF_METAFILEPICT:
        format_name = "METAFILEPICT";
        break;
      case CF_PALETTE:
        format_name = "Palette";
        break;
      case CF_TIFF:
        format_name = "TIFF";
        break;
      case CF_ENHMETAFILE:
        format_name = "HENMETAFILE";
        break;
      default:
      if (GetClipboardFormatNameA(format, format_name_buffer, sizeof(format_name_buffer)) <= 0)
        GFX2_Log(GFX2_WARNING, "Failed to get name for clipboard format %u\n", format);
      else
        format_name = format_name_buffer;
    }
    if (format_name != NULL)
      GFX2_Log(GFX2_DEBUG, "Available format %5u \"%s\"\n", format, format_name);

    format = EnumClipboardFormats(format);  // get next format
  }
  clipboard = GetClipboardData(CF_DIB);
  if (clipboard == NULL)
  {
    // Try to load the filename
    UINT filename_type = RegisterClipboardFormatA("FileName");
    GFX2_Log(GFX2_INFO, "Failed to get Clipboard in DIB (BITMAPINFO) format\n");
    if (filename_type == 0)
      GFX2_Log(GFX2_ERROR, "Failed to register \"FileName\" Clipboard format\n");
    else
    {
      clipboard = GetClipboardData(filename_type);
      if (clipboard == NULL)
        GFX2_Log(GFX2_INFO, "Failed to get Clipboard in \"FileName\" format\n");
      else
      {
        const char * filename = (const char *)GlobalLock(clipboard);
        if (filename == NULL)
          GFX2_Log(GFX2_ERROR, "GlobalLock() failed error 0x%08x\n", GetLastError());
        else
        {
          GFX2_Log(GFX2_DEBUG, "filename from clipboard : \"%s\"\n", filename);
          if (File_exists(filename))
          {
            free(context->File_name);
            context->File_name = Extract_filename(NULL, filename);
            free(context->File_directory);
            context->File_directory = Extract_path(NULL, filename);
            context->Format = DEFAULT_FILEFORMAT;
          }
          else
          {
            GFX2_Log(GFX2_WARNING, "file \"%s\" does not exist\n", filename);
          }
          GlobalUnlock(clipboard);
        }
      }
    }
  }
  else
  {
    // Load the DIB (BITMAPINFO)
    const PBITMAPINFO bmi = (PBITMAPINFO)GlobalLock(clipboard);
    if (bmi == NULL)
      GFX2_Log(GFX2_ERROR, "GlobalLock() failed error 0x%08x\n", GetLastError());
    else
    {
      unsigned long width, height;
      width = bmi->bmiHeader.biWidth;
      height = (bmi->bmiHeader.biHeight > 0) ? bmi->bmiHeader.biHeight : -bmi->bmiHeader.biHeight;

      GFX2_Log(GFX2_DEBUG, "DIB %ldx%ld planes=%u bpp=%u compression=%u size=%u ClrUsed=%u\n",
                bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight,
                bmi->bmiHeader.biPlanes, bmi->bmiHeader.biBitCount,
                bmi->bmiHeader.biCompression, bmi->bmiHeader.biSizeImage,
                bmi->bmiHeader.biClrUsed);
      if (width > 9999 || height > 9999)
        GFX2_Log(GFX2_INFO, "Image too big : %lux%lu\n", width, height);
      else if (bmi->bmiHeader.biCompression == BI_RGB)
      {
        unsigned i, color_count;
        const byte * pixels;
        unsigned int x, y;

        File_error = 0; // have to be set before calling Pre_load()
        Pre_load(context, width, height, bmi->bmiHeader.biSizeImage, FORMAT_CLIPBOARD, PIXEL_SIMPLE, bmi->bmiHeader.biBitCount);

        color_count = bmi->bmiHeader.biClrUsed;
        if (bmi->bmiHeader.biBitCount <= 8)
        { // get palette
          if (color_count == 0)
            color_count = 1 << bmi->bmiHeader.biBitCount;
          for (i = 0; i < color_count; i++)
          {
            context->Palette[i].R = bmi->bmiColors[i].rgbRed;
            context->Palette[i].G = bmi->bmiColors[i].rgbGreen;
            context->Palette[i].B = bmi->bmiColors[i].rgbBlue;
          }
        }
        pixels = (const byte *)(&bmi->bmiColors[color_count]);
        switch (bmi->bmiHeader.biBitCount)
        {
          case 8:
            for (y = 0; y < height; y++)
            {
              const byte * line;
              if (bmi->bmiHeader.biHeight > 0)
                line = pixels + (height - y - 1) * ((bmi->bmiHeader.biWidth + 3) & ~3);
              else
                line = pixels + y * ((bmi->bmiHeader.biWidth + 3) & ~3);
              for (x = 0; x < width; x++)
                Set_pixel(context, x, y, line[x]);
            }
            break;
          case 24:
            for (y = 0; y < height; y++)
            {
              const byte * line = pixels;
              if (bmi->bmiHeader.biHeight > 0)
                line += (height - y - 1) * ((bmi->bmiHeader.biWidth * 3 + 3) & ~3);
              else
                line += y * ((bmi->bmiHeader.biWidth * 3 + 3) & ~3);
              for (x = 0; x < width; x++)
                Set_pixel_24b(context, x, y, line[x*3 + 2], line[x*3 + 1], line[x*3]);
            }
            break;
          case 32:
            for (y = 0; y < height; y++)
            {
              const byte * line;
              if (bmi->bmiHeader.biHeight > 0)
                line = pixels + (height - y - 1) * bmi->bmiHeader.biWidth * 4;
              else
                line = pixels + y * bmi->bmiHeader.biWidth * 4;
              for (x = 0; x < width; x++)
                Set_pixel_24b(context, x, y, line[x*4 + 2], line[x*4 + 1], line[x*4]);
            }
            break;
          default:
            GFX2_Log(GFX2_ERROR, "Loading %ubpp pictures from Clipboard is not implemented yet!\n", bmi->bmiHeader.biBitCount);
            File_error = 1;
        }
      }
      else if (bmi->bmiHeader.biCompression == BI_BITFIELDS)
      {
        dword r_mask = *((const dword *)&bmi->bmiColors[0]);
        int r_bits = count_set_bits(r_mask);
        int r_shift = count_trailing_zeros(r_mask);
        dword g_mask = *((const dword *)&bmi->bmiColors[1]);
        int g_bits = count_set_bits(g_mask);
        int g_shift = count_trailing_zeros(g_mask);
        dword b_mask = *((const dword *)&bmi->bmiColors[2]);
        int b_bits = count_set_bits(b_mask);
        int b_shift = count_trailing_zeros(b_mask);
        const byte * pixels = (const byte *)&bmi->bmiColors[3];
        unsigned int bytes_per_pixel = (bmi->bmiHeader.biBitCount + 7) >> 3;
        int bytes_per_line = (bmi->bmiHeader.biWidth * bytes_per_pixel + 3) & ~3;
        unsigned int x, y;

        GFX2_Log(GFX2_DEBUG, "RGB%d%d%d, masks : %08x %08x %08x\n", r_bits, g_bits, b_bits, r_mask, g_mask, b_mask);

        File_error = 0; // have to be set before calling Pre_load()
        Pre_load(context, width, height, bmi->bmiHeader.biSizeImage, FORMAT_CLIPBOARD, PIXEL_SIMPLE, bmi->bmiHeader.biBitCount);

        for (y = 0; y < height; y++)
        {
          const byte * ptr;
          if (bmi->bmiHeader.biHeight > 0)
            ptr = pixels + (height - y - 1) * bytes_per_line;
          else
            ptr = pixels + y * bytes_per_line;
          for (x = 0; x < width; x++)
          {
            dword rgb, r, g, b;
            switch (bytes_per_pixel)
            {
              case 1:
                rgb = *ptr;
                break;
              case 2:
                rgb = *((const word *)ptr);
                break;
              case 3:
                rgb = ((dword)ptr[2] << 16) | ((dword)ptr[1] << 8) | (dword)ptr[0];
                break;
              default:
                rgb = *((const dword *)ptr);
            }
            r = (rgb & r_mask) >> r_shift;
            r = (r << (8 - r_bits)) | (r >> (2 * r_bits - 8));
            g = (rgb & g_mask) >> g_shift;
            g = (g << (8 - g_bits)) | (g >> (2 * g_bits - 8));
            b = (rgb & b_mask) >> b_shift;
            b = (b << (8 - b_bits)) | (b >> (2 * b_bits - 8));
            Set_pixel_24b(context, x, y, r, g, b);
            ptr += bytes_per_pixel;
          }
        }
      }
      else
        GFX2_Log(GFX2_INFO, "Unsupported DIB compression %u\n", bmi->bmiHeader.biCompression);
      GlobalUnlock(clipboard);
    }
  }
  CloseClipboard();
#elif defined(__macosx__)
  unsigned long size;
  const void * tiff = get_tiff_paste_board(&size);

  GFX2_Log(GFX2_DEBUG, "TIFF pasteboard : %p (%lu bytes)\n", tiff, size);
  if (tiff != NULL)
    Load_TIFF_from_memory(context, tiff, size);

#elif defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
  int i;
  Atom selection;
  Window selection_owner;
#if defined(SDL_VIDEO_DRIVER_X11)
  Display * X11_display;
  Window X11_window;
  int old_wmevent_state;

  if (!GFX2_Get_X11_Display_Window(&X11_display, &X11_window))
  {
    GFX2_Log(GFX2_ERROR, "Failed to get X11 display and window\n");
    return;
  }
  if (X11_display == NULL)
  {
#if defined(USE_SDL)
    char video_driver_name[32];
    GFX2_Log(GFX2_WARNING, "X11 display is NULL. X11 is needed for Copy/Paste. SDL video driver is currently %s\n", SDL_VideoDriverName(video_driver_name, sizeof(video_driver_name)));
#elif defined(USE_SDL2)
    GFX2_Log(GFX2_WARNING, "X11 display is NULL. X11 is needed for Copy/Paste. SDL video driver is currently %s\n", SDL_GetCurrentVideoDriver());
#endif
    return;
  }
#endif

  selection = XInternAtom(X11_display, "CLIPBOARD", False);
  selection_owner = XGetSelectionOwner(X11_display, selection);
  if (selection_owner == None)
  {
    GFX2_Log(GFX2_INFO, "No owner for the X11 \"CLIPBOARD\" selection\n");
    return;
  }
#if defined(USE_SDL) || defined(USE_SDL2)
  // Enable processing of X11 events
  old_wmevent_state = SDL_EventState(SDL_SYSWMEVENT, SDL_QUERY);
  SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

  // "TARGETS" is a special content type. The selection owner will
  // respond with a list of supported type. We will then choose our
  // prefered type and ask for it.
  // see Handle_SelectionNotify()
  // We could ask directly for "image/png" or "image/tiff" but that is
  // not sure this is supported by the selection owner.
  XConvertSelection(X11_display, selection, XInternAtom(X11_display, "TARGETS", False),
                    XInternAtom(X11_display, "GFX2_CLIP", False), /* Property */
                    X11_window, CurrentTime);
  // wait for the event to be received. 500ms maximum
  for(i = 0; X11_clipboard == NULL && i < 25; i++)
  {
    Get_input(20);
  }
#if defined(USE_SDL) || defined(USE_SDL2)
  SDL_EventState(SDL_SYSWMEVENT, old_wmevent_state);
#endif

  switch(X11_clipboard_type)
  {
    case X11_CLIPBOARD_NONE:
      GFX2_Log(GFX2_INFO, "Unable to retrieve X11 \"CLIPBOARD\" selection in a supported format. X11_clipboard=%p\n", X11_clipboard);
      break;
#ifndef __no_pnglib__
    case X11_CLIPBOARD_PNG:
      if (png_sig_cmp((byte *)X11_clipboard, 0, 8) == 0)
      {
        File_error = 0;
        Load_PNG_Sub(context, NULL, X11_clipboard, X11_clipboard_size);
      }
      else
        GFX2_Log(GFX2_WARNING, "Failed to load PNG Clipboard\n");
      break;
#endif
#ifndef __no_tifflib__
    case X11_CLIPBOARD_TIFF:
      Load_TIFF_from_memory(context, X11_clipboard, X11_clipboard_size);
      if (File_error != 0)
        GFX2_Log(GFX2_WARNING, "Failed to load TIFF Clipboard\n");
      break;
#endif
    case X11_CLIPBOARD_UTF8STRING:
    case X11_CLIPBOARD_URILIST:
      {
        char tmp_path[MAX_PATH_CHARACTERS];
        char * p;
        p = strchr(X11_clipboard, '\r');
        if (p != NULL)
          *p = '\0';
        p = strchr(X11_clipboard, '\n');
        if (p != NULL)
          *p = '\0';
        p = X11_clipboard;
        if (strncmp(p, "file://", 7) == 0)
        {
          //int i
          p += 7;
          for (i = 0; *p; i++)
          {
            // URLdecode
            if (p[0] == '%' && p[1] && p[2])
            {
              p++;
              tmp_path[i] = ((*p >= 'A' && *p <= 'F') ? *p - 'A' + 10 : *p - '0') << 4;
              p++;
              tmp_path[i] += ((*p >= 'A' && *p <= 'F') ? *p - 'A' + 10 : *p - '0');
            }
            else
              tmp_path[i] = *p;
            p++;
          }
          tmp_path[i] = '\0';
          p = tmp_path;
        }
        if (File_exists(p))
        {
          free(context->File_name);
          context->File_name = Extract_filename(NULL, p);
          free(context->File_directory);
          context->File_directory = Extract_path(NULL, p);
          context->Format = DEFAULT_FILEFORMAT;
        }
        else
        {
          GFX2_Log(GFX2_WARNING, "not a filename : \"%s\"\n", p);
        }
      }
      break;
    default:
      GFX2_Log(GFX2_WARNING, "Unsupported Clipboard format %d\n", (int)X11_clipboard_type);
  }
  free(X11_clipboard);
  X11_clipboard = NULL;
  X11_clipboard_size = 0;
  X11_clipboard_type = X11_CLIPBOARD_NONE;

#else
  GFX2_Log(GFX2_ERROR, "Load_ClipBoard_Image() not implemented on this platform yet\n");
  File_error = 1;
#endif
}

static void Save_ClipBoard_Image(T_IO_Context * context)
{
#ifdef WIN32
  if (!OpenClipboard(GFX2_Get_Window_Handle()))
  {
    GFX2_Log(GFX2_ERROR, "Failed to open Clipboard\n");
    return;
  }
  if (!EmptyClipboard())
    GFX2_Log(GFX2_ERROR, "EmptyClipboard() failed error 0x%08x\n", GetLastError());
  else
  {
    int line_width = (context->Width + 3) & ~3;
    HGLOBAL clipboard = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) + line_width * context->Height);
    if (clipboard == NULL)
      GFX2_Log(GFX2_ERROR, "GlobalAlloc() failed error 0x%08x\n", GetLastError());
    else
    {
      PBITMAPINFO bmi = (PBITMAPINFO)GlobalLock(clipboard);
      if (bmi == NULL)
        GFX2_Log(GFX2_ERROR, "GlobalLock() failed error 0x%08x\n", GetLastError());
      else
      {
        byte * pixels;
        unsigned int i;
        int x, y;

        bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
        bmi->bmiHeader.biWidth = context->Width;
        bmi->bmiHeader.biHeight = context->Height;
        bmi->bmiHeader.biPlanes = 1;
        bmi->bmiHeader.biBitCount = 8;
        bmi->bmiHeader.biCompression = BI_RGB;
        bmi->bmiHeader.biSizeImage = context->Height * line_width;
        //bmi->bmiHeader.biXPelsPerMeter;
        //bmi->bmiHeader.biYPelsPerMeter;
        for (i = 0; i < 256; i++)
        {
          bmi->bmiColors[i].rgbRed = context->Palette[i].R;
          bmi->bmiColors[i].rgbGreen = context->Palette[i].G;
          bmi->bmiColors[i].rgbBlue = context->Palette[i].B;
          bmi->bmiColors[i].rgbReserved = 0;
        }
        pixels = (byte *)&bmi->bmiColors[256] + bmi->bmiHeader.biSizeImage;
        for (y = 0; y < context->Height; y++)
        {
          pixels -= line_width;
          for (x = 0; x < context->Width; x++)
            pixels[x] = Get_pixel(context, x, y);
        }
        GlobalUnlock(clipboard);
        if (SetClipboardData(CF_DIB, clipboard) == NULL)
          GFX2_Log(GFX2_ERROR, "SetClipboardData() failed error 0x%08x\n", GetLastError());
        else
          File_error = 0;
      }
    }
  }
  CloseClipboard();
#elif defined(__macosx__)
  void * tiff = NULL;
  unsigned long size = 0;

  Save_TIFF_to_memory(context, &tiff, &size);
  if (File_error == 0 && tiff != NULL)
  {
    if(!set_tiff_paste_board(tiff, size))
      File_error = 1;
  }
  free(tiff);

#elif defined(USE_X11) || (defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11))
  Atom selection;
#if defined(SDL_VIDEO_DRIVER_X11)
  Display * X11_display;
  Window X11_window;
  //int old_wmevent_state;

  if (!GFX2_Get_X11_Display_Window(&X11_display, &X11_window))
  {
    GFX2_Log(GFX2_ERROR, "Failed to get X11 display and window\n");
    return;
  }
  if (X11_display == NULL)
  {
#if defined(USE_SDL)
    char video_driver_name[32];
    GFX2_Log(GFX2_WARNING, "X11 display is NULL. X11 is needed for Copy/Paste. SDL video driver is currently %s\n", SDL_VideoDriverName(video_driver_name, sizeof(video_driver_name)));
#elif defined(USE_SDL2)
    GFX2_Log(GFX2_WARNING, "X11 display is NULL. X11 is needed for Copy/Paste. SDL video driver is currently %s\n", SDL_GetCurrentVideoDriver());
#endif
    return;
  }
#endif

  File_error = 0;
  if (X11_clipboard != NULL)
  {
    free(X11_clipboard);
    X11_clipboard = NULL;
    X11_clipboard_size = 0;
  }
  Save_PNG_Sub(context, NULL, &X11_clipboard, &X11_clipboard_size);
  if (!File_error)
  {
#if defined(USE_SDL) || defined(USE_SDL2)
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif
    selection = XInternAtom(X11_display, "CLIPBOARD", False);
    XSetSelectionOwner(X11_display, selection, X11_window, CurrentTime);
  }
#else
  GFX2_Log(GFX2_ERROR, "Save_ClipBoard_Image() not implemented on this platform yet\n");
  File_error = 1;
#endif
}

/// Saves an image.
/// This routine will only be called when all hope is lost, memory thrashed, etc
/// It's the last chance to save anything, but the code has to be extremely
/// careful, anything could happen.
/// The chosen format is IMG since it's extremely simple, difficult to make it
/// create an unusable image.
void Emergency_backup(const char *fname, byte *source, int width, int height, T_Palette *palette)
{
  char * filename; // Full path name
  FILE *file;
  short x_pos,y_pos;
  T_IMG_Header IMG_header;

  if (width == 0 || height == 0 || source == NULL)
    return;
  
  // Open the file
  filename = Filepath_append_to_dir(Config_directory, fname);
  file = fopen(filename,"wb");
  free(filename);
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
  if (Main.backups && Main.backups->Pages && Main.backups->Pages->Nb_layers == 1)
    Emergency_backup(SAFETYBACKUP_PREFIX_A "999999" BACKUP_FILE_EXTENSION,Main_screen, Main.image_width, Main.image_height, &Main.palette);
  if (Spare.backups && Spare.backups->Pages && Spare.backups->Pages->Nb_layers == 1)
    Emergency_backup(SAFETYBACKUP_PREFIX_B "999999" BACKUP_FILE_EXTENSION,Spare.visible_image.Image, Spare.image_width, Spare.image_height, &Spare.palette);
}

const T_Format * Get_fileformat(byte format)
{
  unsigned int i;
  const T_Format * safe_default = File_formats;
  
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
  free(context->File_name);
  free(context->File_name_unicode);
  free(context->File_directory);
  free(context->Original_file_directory);
  free(context->Original_file_name);
  memset(context, 0, sizeof(T_IO_Context));
}

/// Setup for loading a preview in fileselector
void Init_context_preview(T_IO_Context * context, char *file_name, char *file_directory)
{
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_PREVIEW;
  context->File_name = file_name != NULL ? strdup(file_name) : NULL;
  context->File_directory = file_directory != NULL ? strdup(file_directory) : NULL;
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
  context->File_name = file_name != NULL ? strdup(file_name) : NULL;
  context->File_directory = file_directory != NULL ? strdup(file_directory) : NULL;
  context->Format = Main.fileformat;
  memcpy(context->Palette, Main.palette, sizeof(T_Palette));
  context->Width = Main.image_width;
  context->Height = Main.image_height;
  context->Nb_layers = Main.backups->Pages->Nb_layers;
  strcpy(context->Comment, Main.backups->Pages->Comment);
  context->Transparent_color=Main.backups->Pages->Transparent_color;
  context->Background_transparent=Main.backups->Pages->Background_transparent;
  context->Ratio = Pixel_ratio;
  context->Target_address=Main.backups->Pages->Image[0].Pixels;
  context->Pitch=Main.image_width;
  
  // Color cyling ranges:
  for (i=0; i<16; i++)
  {
    if (Main.backups->Pages->Gradients->Range[i].Start!=Main.backups->Pages->Gradients->Range[i].End)
    {
      context->Cycle_range[context->Color_cycles].Start=Main.backups->Pages->Gradients->Range[i].Start;
      context->Cycle_range[context->Color_cycles].End=Main.backups->Pages->Gradients->Range[i].End;
      context->Cycle_range[context->Color_cycles].Inverse=Main.backups->Pages->Gradients->Range[i].Inverse;
      context->Cycle_range[context->Color_cycles].Speed=Main.backups->Pages->Gradients->Range[i].Speed;
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
  context->File_name = file_name != NULL ? strdup(file_name) : NULL;
  context->File_directory = file_directory != NULL ? strdup(file_directory) : NULL;
  context->Format = Brush_fileformat;
  // Use main screen's palette
  memcpy(context->Palette, Main.palette, sizeof(T_Palette));
  context->Width = Brush_width;
  context->Height = Brush_height;
  context->Nb_layers = 1;
  context->Transparent_color=Back_color;  // Transparent color for brushes
  context->Background_transparent=1;
  context->Ratio=PIXEL_SIMPLE;
  context->Target_address=Brush;
  context->Pitch=Brush_width;

}

// Setup for loading an image into a new GFX2 surface.
void Init_context_surface(T_IO_Context * context, char *file_name, char *file_directory)
{
  memset(context, 0, sizeof(T_IO_Context));
  
  context->Type = CONTEXT_SURFACE;
  context->File_name = file_name != NULL ? strdup(file_name) : NULL;
  context->File_directory = file_directory != NULL ? strdup(file_directory) : NULL;
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
    if (context->Nb_layers==1 && Main.backups->Pages->Nb_layers!=1)
    {
      // Context is set to saving a single layer: do nothing
    }
    else
    {
      context->Target_address=Main.backups->Pages->Image[layer].Pixels;
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
      if (Add_layer(Main.backups, layer))
      {
        // Failure to add a layer on load:
        // Position on last layer
        layer = context->Nb_layers-1;
        break;
      }
      context->Nb_layers = Main.backups->Pages->Nb_layers;
      Main.layers_visible = (2<<layer)-1;
    }
    Main.current_layer = layer;
    context->Target_address=Main.backups->Pages->Image[layer].Pixels;
    
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
static void Add_backup_file(const char * full_name, const char *file_name)
{
  T_String_list ** list;
  T_String_list * elem;
  int i;
  (void)full_name;
  
  // Only files names of the form a0000000.* and b0000000.* are expected
  
  // Check first character
  if (file_name[0]==Main.safety_backup_prefix)
    list = &Backups_main;
  else if (file_name[0]==Spare.safety_backup_prefix)
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
  elem = (T_String_list *)GFX2_malloc(sizeof(T_String_list));
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
  files_vector = (char **)GFX2_malloc(sizeof(char *) * nb_files);
// TODO
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
    
    Init_context_backup_image(&context, files_vector[i], Config_directory);
    // Provide buffers to read original location
    Load_image(&context);
    Main.image_is_modified=1;
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
    Main.offset_X=0;
    Main.offset_Y=0;
    Compute_limits();
    Compute_paintbrush_coordinates();
    if (Backups_main)
      Button_Page(BUTTON_PAGE);
  }
  restored_main = Process_backups(&Backups_main);
  
  if (restored_main)
  {
    Main.offset_X=0;
    Main.offset_Y=0;
    Compute_limits();
    Compute_paintbrush_coordinates();
  }
  return restored_main + restored_spare;
}

void Rotate_safety_backups(void)
{
  dword now;
  T_IO_Context context;
  char file_name[12+1];

  if (!Safety_backup_active)
    return;
    
  now = GFX2_GetTicks();
  // It's time to save if either:
  // - Many edits have taken place
  // - A minimum number of edits have taken place AND a minimum time has passed
  // - At least one edit was done, and a maximum time has passed
  if ((Main.edits_since_safety_backup > Max_edits_for_safety_backup) ||
      (Main.edits_since_safety_backup > Min_edits_for_safety_backup &&
      now > Main.time_of_safety_backup + Min_interval_for_safety_backup) ||
      (Main.edits_since_safety_backup > 1 &&
      now > Main.time_of_safety_backup + Max_interval_for_safety_backup))
  {
    char * deleted_file;
    size_t len = strlen(Config_directory) + strlen(BACKUP_FILE_EXTENSION) + 1 + 6 + 1;

    deleted_file = GFX2_malloc(len);
    if (deleted_file == NULL)
      return;
    // Clear a previous save (rotating saves)
    snprintf(deleted_file, len, "%s%c%6.6d" BACKUP_FILE_EXTENSION,
      Config_directory,
      Main.safety_backup_prefix,
      (dword)(Main.safety_number + 1000000l - Rotation_safety_backup) % (dword)1000000l);
    Remove_path(deleted_file); // no matter if fail
    free(deleted_file);
    
    // Reset counters
    Main.edits_since_safety_backup=0;
    Main.time_of_safety_backup=now;

    // Create a new file name and save
    sprintf(file_name, "%c%6.6d" BACKUP_FILE_EXTENSION,
      Main.safety_backup_prefix,
      (int)Main.safety_number);
    Init_context_backup_image(&context, file_name, Config_directory);
    context.Format=FORMAT_GIF;
    // Provide original file data, to store as a GIF Application Extension
    context.Original_file_name = strdup(Main.backups->Pages->Filename);
    context.Original_file_directory = strdup(Main.backups->Pages->File_directory);
    
    Save_image(&context);
    Destroy_context(&context);
    
    Main.safety_number++;
  }
}

/// Remove safety backups. Need to call on normal program exit.
void Delete_safety_backups(void)
{
  T_String_list *element;
  T_String_list *next;

  if (!Safety_backup_active)
    return;
  
  Backups_main = NULL;
  Backups_spare = NULL;
  
  For_each_file(Config_directory, Add_backup_file);
  
  Change_directory(Config_directory);
  for (element=Backups_main; element!=NULL; element=next)
  {
    next = element->Next;
    if(remove(element->String))
      printf("Failed to delete %s\n",element->String);
    free(element->String);
    free(element);
  }
  Backups_main = NULL;
  for (element=Backups_spare; element!=NULL; element=next)
  {
    next = element->Next;
    if(remove(element->String))
      printf("Failed to delete %s\n",element->String);
    free(element->String);
    free(element);
  }
  Backups_spare = NULL;
  
  // Release lock file
#if defined (__MINT__) 
  //TODO: release file lock under Freemint only
#else 
  Release_lock_file(Config_directory);
#endif
  
}

/// For use by Save_XXX() functions
FILE * Open_file_write(T_IO_Context *context)
{
  FILE * f;
  char * filename; // filename with full path
#if defined(WIN32)
  if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
  {
    size_t len;
    WCHAR * filename_unicode;

    len = strlen(context->File_directory) + strlen(PATH_SEPARATOR)
        + Unicode_strlen(context->File_name_unicode) + 1;
    filename_unicode = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * len);
    if (filename_unicode == NULL)
      return NULL;

    Unicode_char_strlcpy((word *)filename_unicode, context->File_directory, len);
    Unicode_char_strlcat((word *)filename_unicode, PATH_SEPARATOR, len);
    Unicode_strlcat((word *)filename_unicode, context->File_name_unicode, len);

    f = _wfopen(filename_unicode, L"wb");
    if (f != NULL)
    {
      // Now the file has been created, retrieve its short (ASCII) name
      len = GetShortPathNameW(filename_unicode, NULL, 0);
      if (len > 0)
      {
        WCHAR * shortpath = (WCHAR *)GFX2_malloc(sizeof(WCHAR) * len);
        if (shortpath != NULL)
        {
          len = GetShortPathNameW(filename_unicode, shortpath, len);
          if (len > 0)
          {
            DWORD start, index;
            for (start = len; start > 0 && shortpath[start-1] != '\\'; start--);
            free(context->File_name);
            context->File_name = (char *)GFX2_malloc(len + 1 - start);
            if (context->File_name != NULL)
            {
              for (index = 0; index < len - start; index++)
                context->File_name[index] = shortpath[start + index];
              context->File_name[index] = '\0';
            }
          }
        }
      }
    }
    free(filename_unicode);
    return f;
  }
#endif

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  if (filename == NULL)
    return NULL;
  f = fopen(filename, "wb");
  free(filename);
  return f;
}

FILE * Open_file_write_with_alternate_ext(T_IO_Context *context, const char * ext)
{
  FILE * f;
  char *p;
  char * filename; // filename with full path
#if defined(WIN32)
  if (context->File_name_unicode != NULL && context->File_name_unicode[0] != 0)
  {
    size_t len;
    WCHAR * filename_unicode;
    WCHAR * pw;

    len = strlen(context->File_directory) + strlen(PATH_SEPARATOR)
        + Unicode_strlen(context->File_name_unicode) + strlen(ext) + 1 + 1;
    filename_unicode = (WCHAR *)GFX2_malloc(len * sizeof(WCHAR));
    if (filename_unicode == NULL)
      return NULL;
    Unicode_char_strlcpy((word *)filename_unicode, context->File_directory, len);
    Unicode_char_strlcat((word *)filename_unicode, PATH_SEPARATOR, len);
    Unicode_strlcat((word *)filename_unicode, context->File_name_unicode, len);
    pw = wcschr(filename_unicode, (WCHAR)'.');
    if (pw != NULL)
      *pw = 0;
    Unicode_char_strlcat((word *)filename_unicode, ".", len);
    Unicode_char_strlcat((word *)filename_unicode, ext, len);

    f = _wfopen(filename_unicode, L"wb");
    free(filename_unicode);
    return f;
  }
#endif
  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
// TODO: fix ! (realloc if not enough space)
  p = strrchr(filename, '.');
  if (p != NULL)
    *p = '\0';
  strcat(filename, ".");
  strcat(filename, ext);

  f = fopen(filename, "wb");
  free(filename);
  return f;
}

/// For use by Load_XXX() and Test_XXX() functions
FILE * Open_file_read(T_IO_Context *context)
{
  FILE * f;
  char * filename; // filename with full path

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  f = fopen(filename, "rb");
  free(filename);
  return f;
}

struct T_Find_alternate_ext_data
{
  const char * ext;
  char * basename;
  word * basename_unicode;
  char * foundname;
  word * foundname_unicode;
};

static void Look_for_alternate_ext(void * pdata, const char * filename, const word * filename_unicode, byte is_file, byte is_directory, byte is_hidden)
{
  size_t base_len;
  struct T_Find_alternate_ext_data * params = (struct T_Find_alternate_ext_data *)pdata;
  (void)is_hidden;
  (void)is_directory;

  if (!is_file)
    return;

  if (filename_unicode != NULL && params->basename_unicode != NULL)
  {
    if (params->foundname_unicode != NULL)
      return; // We already have found a file
    base_len = Unicode_strlen(params->basename_unicode);
    if (Unicode_strlen(filename_unicode) <= base_len)
      return; // No match.
    if (filename_unicode[base_len] != '.')
      return; // No match.
#if defined(WIN32)
    {
      int cmp;
      WCHAR * temp_string = (WCHAR *)GFX2_malloc((base_len + 1) * sizeof(WCHAR));
      if (temp_string == NULL)
        return;
      memcpy(temp_string, filename_unicode, base_len * sizeof(word));
      temp_string[base_len] = 0;
      cmp = _wcsicmp((const WCHAR *)params->basename_unicode, temp_string);
      free(temp_string);
      if (cmp != 0)
        return; // No match.
    }
#else
    if (memcmp(params->basename_unicode, filename_unicode, base_len * sizeof(word)) != 0)
      return; // No match.
#endif
    if (Unicode_char_strcasecmp(filename_unicode + base_len + 1, params->ext) != 0)
      return; // No match.
    // it is a match !
    free(params->foundname);
    params->foundname_unicode = Unicode_strdup(filename_unicode);
    params->foundname = strdup(filename);
  }
  else
  {
    if (params->foundname != NULL)
      return; // We already have found a file
    base_len = strlen(params->basename);
    if (filename[base_len] != '.')
      return; // No match.
#if defined(WIN32)
    if (_memicmp(params->basename, filename, base_len) != 0)  // Not case sensitive
      return; // No match.
#else
    if (memcmp(params->basename, filename, base_len) != 0)
      return; // No match.
#endif
    if (strcasecmp(filename + base_len + 1, params->ext) != 0)
      return; // No match.
    params->foundname_unicode = NULL;
    params->foundname = strdup(filename);
  }
}

FILE * Open_file_read_with_alternate_ext(T_IO_Context *context, const char * ext)
{
  FILE * f = NULL;
  char * p;
  struct T_Find_alternate_ext_data params;

  memset(&params, 0, sizeof(params));
  params.ext = ext;
  params.basename = strdup(context->File_name);
  if (params.basename == NULL)
  {
    GFX2_Log(GFX2_ERROR, "Open_file_read_with_alternate_ext() strdup() failed\n");
    return NULL;
  }
  p = strrchr(params.basename, '.');
  if (p != NULL)
    *p = '\0';
  if (context->File_name_unicode != NULL)
  {
    size_t i = Unicode_strlen(context->File_name_unicode);
    params.basename_unicode = GFX2_malloc(sizeof(word) * (i + 1));
    if (params.basename_unicode != NULL)
    {
      memcpy(params.basename_unicode, context->File_name_unicode, (i + 1) * sizeof(word));
      while (i-- > 0)
        if (params.basename_unicode[i] == (word)'.')
        {
          params.basename_unicode[i] = 0;
          break;
        }
    }
  }

  For_each_directory_entry(context->File_directory, &params, Look_for_alternate_ext);
  if (params.foundname != NULL)
  {
    char * filename; // filename with full path

    filename = Filepath_append_to_dir(context->File_directory, params.foundname);
    f = fopen(filename, "rb");
    free(filename);
  }
  free(params.basename);
  free(params.basename_unicode);
  free(params.foundname);
  free(params.foundname_unicode);
  return f;
}

/// For use by Save_XXX() functions
void Remove_file(T_IO_Context *context)
{
  char * filename; // filename with full path

  filename = Filepath_append_to_dir(context->File_directory, context->File_name);
  Remove_path(filename);
  free(filename);
}
