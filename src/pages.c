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
//////////////////////////////////////////////////////////////////////////
/////////////////////////// GESTION DU BACKUP ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#define strdup _strdup
#endif

#include "gfx2mem.h"
#include "global.h"
#include "pages.h"
#include "errors.h"
#include "loadsave.h"
#include "misc.h"
#include "windows.h"
#include "tiles.h"
#include "graph.h"
#include "layers.h"
#include "unicode.h"

// -- Layers data

/// Array of two images, that contains the "flattened" version of the visible layers.
//T_Bitmap Main_visible_image;
T_Bitmap Main_visible_image_backup;
T_Bitmap Main_visible_image_depth_buffer;
//T_Bitmap Spare_visible_image;

  ///
  /// GESTION DES PAGES
  ///

/// Bitfield which records which layers are backed up in Page 0.
static dword Last_backed_up_layers=0;

/// Total number of unique bitmaps (layers, animation frames, backups)
long Stats_pages_number=0;
/// Total memory used by bitmaps (layers, animation frames, backups)
long long Stats_pages_memory=0;

/// Allocate and initialize a new page.
T_Page * New_page(int nb_layers)
{
  T_Page * page;
  
  page = (T_Page *)GFX2_malloc(sizeof(T_Page)+nb_layers*sizeof(T_Image));
  if (page!=NULL)
  {
    int i;
    for (i=0; i<nb_layers; i++)
    {
      page->Image[i].Pixels = NULL;
      page->Image[i].Duration = 100;
    }
    page->Width=0;
    page->Height=0;
    page->Image_mode = IMAGE_MODE_LAYERED;
    memset(page->Palette, 0, sizeof(T_Palette));
    page->Comment[0] = '\0';
    page->File_directory = NULL;
    page->Filename = NULL;
    page->Filename_unicode = NULL;
    page->File_format = DEFAULT_FILEFORMAT;
    page->Nb_layers = nb_layers;
    page->Gradients = NULL;
    page->Transparent_color = 0; // Default transparent color
    page->Background_transparent = 0;
    page->Next = page->Prev = NULL;
  }
  return page;
}

// ==============================================================
// Layers allocation functions.
//
// Layers are made of a "number of users" (short), followed by
// the actual pixel data (a large number of bytes).
// Every time a layer is 'duplicated' as a reference, the number
// of users is incremented.
// Every time a layer is freed, the number of users is decreased,
// and only when it reaches zero the pixel data is freed.
// ==============================================================

/// Allocate a new layer
byte * New_layer(long pixel_size)
{
  short * ptr = GFX2_malloc(sizeof(short)+pixel_size);
  if (ptr==NULL)
    return NULL;
    
  // Stats
  Stats_pages_number++;
  Stats_pages_memory+=pixel_size;
  
  *ptr = 1;
  return (byte *)(ptr+1);
}

/// Free a layer
void Free_layer(T_Page * page, int layer)
{
  short * ptr;
  if (page->Image[layer].Pixels==NULL)
    return;
    
  ptr = (short *)(page->Image[layer].Pixels);
  if (-- (*(ptr-1))) // Users--
    return;
  else {
    free(ptr-1);
  }
    
  // Stats
  Stats_pages_number--;
  Stats_pages_memory-=page->Width * page->Height;
}

/// Duplicate a layer (new reference)
byte * Dup_layer(byte * layer)
{
  short * ptr = (short *)(layer);
  
  if (layer==NULL)
    return NULL;
  
  (*(ptr-1)) ++; // Users ++
  return layer;
}

// ==============================================================

/// Adds a shared reference to the gradient data of another page. Pass NULL for new.
T_Gradient_array *Dup_gradient(T_Page * page)
{
  // new
  if (page==NULL || page->Gradients==NULL)
  {
    T_Gradient_array *array;
    array=(T_Gradient_array *)calloc(1, sizeof(T_Gradient_array));
    if (!array)
      return NULL;
    array->Used=1;
    return array;
  }
  // shared
  page->Gradients->Used++;
  return page->Gradients;
}

void Download_infos_page_main(T_Page * page)
// Affiche la page à l'écran
{
  //int factor_index;
  int size_is_modified;
  
  if (page!=NULL)
  {
    size_is_modified=(Main.image_width!=page->Width) ||
                         (Main.image_height!=page->Height);

    Main.image_width=page->Width;
    Main.image_height=page->Height;
    memcpy(Main.palette,page->Palette,sizeof(T_Palette));
    Main.fileformat=page->File_format;

    if (size_is_modified)
    {
      Main.magnifier_mode=0;
      Main.offset_X=0;
      Main.offset_Y=0;
      Pixel_preview=Pixel_preview_normal;
      Compute_limits();
      Compute_paintbrush_coordinates();
    }
    
  }
  //Update_buffers( page->Width, page->Height);
  //memcpy(Main_screen, page->Image[Main.current_layer].Pixels, page->Width*page->Height);
  
}

void Redraw_layered_image(void)
{
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // Re-construct the image with the visible layers
    byte layer=0;  
    // First layer
    if ((Main.backups->Pages->Image_mode == IMAGE_MODE_MODE5
		|| Main.backups->Pages->Image_mode == IMAGE_MODE_RASTER) && Main.layers_visible & (1<<4))
    {
      // The raster result layer is visible: start there
      // Copy it in Main_visible_image
      int i;
      for (i=0; i< Main.image_width*Main.image_height; i++)
      {
        layer = *(Main.backups->Pages->Image[4].Pixels+i);
        if (Main.layers_visible & (1 << layer))
          Main.visible_image.Image[i]=*(Main.backups->Pages->Image[layer].Pixels+i);
        else
          Main.visible_image.Image[i] = layer;
      }
        
      // Copy it to the depth buffer
      memcpy(Main_visible_image_depth_buffer.Image,
        Main.backups->Pages->Image[4].Pixels,
        Main.image_width*Main.image_height);
        
      // Next
      layer= (1<<4)+1;
    }
    else
    {
      for (layer=0; layer<Main.backups->Pages->Nb_layers; layer++)
      {
        if ((1<<layer) & Main.layers_visible)
        {
           // Copy it in Main_visible_image
           memcpy(Main.visible_image.Image,
             Main.backups->Pages->Image[layer].Pixels,
             Main.image_width*Main.image_height);
           
           // Initialize the depth buffer
           memset(Main_visible_image_depth_buffer.Image,
             layer,
             Main.image_width*Main.image_height);
           
           // skip all other layers
           layer++;
           break;
        }
      }
    }
    // subsequent layer(s)
    for (; layer<Main.backups->Pages->Nb_layers; layer++)
    {
      if ((1<<layer) & Main.layers_visible)
      {
        int i;
        for (i=0; i<Main.image_width*Main.image_height; i++)
        {
          byte color = *(Main.backups->Pages->Image[layer].Pixels+i);
          if (color != Main.backups->Pages->Transparent_color) // transparent color
          {
            *(Main.visible_image.Image+i) = color;
            if (layer != Main.current_layer)
              *(Main_visible_image_depth_buffer.Image+i) = layer;
          }
        }
      }
    }
  }
  else
  {
    Update_screen_targets();
  }
  Update_FX_feedback(Config.FX_Feedback);
}

void Update_depth_buffer(void)
{
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // Re-construct the depth buffer with the visible layers.
    // This function doesn't touch the visible buffer, it assumes
    // that it was already up-to-date. (Ex. user only changed active layer)
  
    int layer;  
    // First layer
    for (layer=0; layer<Main.backups->Pages->Nb_layers; layer++)
    {
      if ((1<<layer) & Main.layers_visible)
      {
         // Initialize the depth buffer
         memset(Main_visible_image_depth_buffer.Image,
           layer,
           Main.image_width*Main.image_height);
         
         // skip all other layers
         layer++;
         break;
      }
    }
    // subsequent layer(s)
    for (; layer<Main.backups->Pages->Nb_layers; layer++)
    {
      // skip the current layer, whenever we reach it
      if (layer == Main.current_layer)
        continue;
        
      if ((1<<layer) & Main.layers_visible)
      {
        int i;
        for (i=0; i<Main.image_width*Main.image_height; i++)
        {
          byte color = *(Main.backups->Pages->Image[layer].Pixels+i);
          if (color != Main.backups->Pages->Transparent_color) // transparent color
          {
            *(Main_visible_image_depth_buffer.Image+i) = layer;
          }
        }
      }
    }
  }
  Update_FX_feedback(Config.FX_Feedback);
}

void Redraw_spare_image(void)
{
  if (Spare.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // Re-construct the image with the visible layers
    byte layer;  
    // First layer
    for (layer=0; layer<Spare.backups->Pages->Nb_layers; layer++)
    {
      if ((1<<layer) & Spare.layers_visible)
      {
         // Copy it in Spare_visible_image
         memcpy(Spare.visible_image.Image,
           Spare.backups->Pages->Image[layer].Pixels,
           Spare.image_width*Spare.image_height);
         
         // No depth buffer in the spare
         //memset(Spare_visible_image_depth_buffer.Image,
         //  layer,
         //  Spare.image_width*Spare.image_height);
         
         // skip all other layers
         layer++;
         break;
      }
    }
    // subsequent layer(s)
    for (; layer<Spare.backups->Pages->Nb_layers; layer++)
    {
      if ((1<<layer) & Spare.layers_visible)
      {
        int i;
        for (i=0; i<Spare.image_width*Spare.image_height; i++)
        {
          byte color = *(Spare.backups->Pages->Image[layer].Pixels+i);
          if (color != Spare.backups->Pages->Transparent_color) // transparent color
          {
            *(Spare.visible_image.Image+i) = color;
            //if (layer != Spare.current_layer)
            //  *(Spare_visible_image_depth_buffer.Image+i) = layer;
          }
        }
      }
    }
  }
}

void Redraw_current_layer(void)
{
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    int i;
    for (i=0; i<Main.image_width*Main.image_height; i++)
    {
      byte depth = *(Main_visible_image_depth_buffer.Image+i);
      if (depth<=Main.current_layer)
      {
        byte color = *(Main.backups->Pages->Image[Main.current_layer].Pixels+i);
        if (color != Main.backups->Pages->Transparent_color) // transparent color
        {
          *(Main.visible_image.Image+i) = color;
        }
        else
        {
          *(Main.visible_image.Image+i) = *(Main.backups->Pages->Image[depth].Pixels+i);
        }
      }
    }
  }
}

void Upload_infos_page(T_Document * doc)
// Sauve l'écran courant dans la page
{
  if (doc->backups->Pages != NULL)
  {
    doc->backups->Pages->Width = doc->image_width;
    doc->backups->Pages->Height = doc->image_height;
    memcpy(doc->backups->Pages->Palette, doc->palette, sizeof(T_Palette));
    doc->backups->Pages->File_format = doc->fileformat;
  }
}

void Download_infos_page_spare(T_Page * page)
{
  if (page!=NULL)
  {
    Spare.image_width=page->Width;
    Spare.image_height=page->Height;
    memcpy(Spare.palette,page->Palette,sizeof(T_Palette));
    Spare.fileformat=page->File_format;
  }
}

byte * FX_feedback_screen;

void Update_FX_feedback(byte with_feedback)
{

  if (with_feedback)
    FX_feedback_screen=Main.backups->Pages->Image[Main.current_layer].Pixels;
  else
    FX_feedback_screen=Main.backups->Pages->Next->Image[Main.current_layer].Pixels;
}

void Clear_page(T_Page * page)
{
  // On peut appeler cette fonction sur une page non allouée.
  int i;
  for (i=0; i<page->Nb_layers; i++)
  {
    Free_layer(page, i);
    page->Image[i].Pixels=NULL;
    page->Image[i].Duration=0;
  }

  // Free_gradient() : This data is reference-counted
  if (page->Gradients)
  {
    page->Gradients->Used--;
    if (page->Gradients->Used==0)
      free(page->Gradients);
    page->Gradients=NULL;
  }

  page->Width=0;
  page->Height=0;
  // On ne se préoccupe pas de ce que deviens le reste des infos de l'image.
}

void Copy_S_page(T_Page * dest, T_Page * source)
{
  *dest = *source;
  dest->Gradients = Dup_gradient(source);
  if (source->File_directory != NULL)
    dest->File_directory = strdup(source->File_directory);
  if (source->Filename != NULL)
    dest->Filename = strdup(source->Filename);
  if (source->Filename_unicode != NULL)
    dest->Filename_unicode = Unicode_strdup(source->Filename_unicode);
}


  ///
  /// GESTION DES LISTES DE PAGES
  ///

void Init_list_of_pages(T_List_of_pages * list)
{
  // Important: appeler cette fonction sur toute nouvelle structure
  //            T_List_of_pages!

  list->List_size=0;
  list->Pages=NULL;
}

int Allocate_list_of_pages(T_List_of_pages * list)
{
  // Important: la T_List_of_pages ne doit pas déjà désigner une liste de
  //            pages allouée auquel cas celle-ci serait perdue.
  T_Page * page;

  // On initialise chacune des nouvelles pages
  page=New_page(1);
  if (!page)
    return 0;
  
  // Set as first page of the list
  page->Next = page;
  page->Prev = page;
  list->Pages = page;

  list->List_size=1;

  page->Gradients = Dup_gradient(NULL);
  if (!page->Gradients)
    return 0;
  
  return 1; // Succès
}


void Backward_in_list_of_pages(T_List_of_pages * list)
{
  // Cette fonction fait l'équivalent d'un "Undo" dans la liste de pages.
  // Elle effectue une sorte de ROL (Rotation Left) sur la liste:
  // +---+-+-+-+-+-+-+-+-+-+  |
  // ¦0¦1¦2¦3¦4¦5¦6¦7¦8¦9¦A¦  |
  // +---+-+-+-+-+-+-+-+-+-+  |  0=page courante
  //  ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦   |_ A=page la plus ancienne
  //  v v v v v v v v v v v   |  1=DerniÞre page (1er backup)
  // +---+-+-+-+-+-+-+-+-+-+  |
  // ¦1¦2¦3¦4¦5¦6¦7¦8¦9¦A¦0¦  |
  // +---+-+-+-+-+-+-+-+-+-+  |

  // Pour simuler un véritable Undo, l'appelant doit mettre la structure
  // de page courante à jour avant l'appel, puis en réextraire les infos en
  // sortie, ainsi que celles relatives à la plus récente page d'undo (1ère
  // page de la liste).

  if (Last_backed_up_layers)
  {
    // First page contains a ready-made backup of its ->Next.
    // We have swap the first two pages, so the original page 0
    // will end up in position 0 again, and then overwrite it with a backup
    // of the 'new' page1.
    T_Page * page0;
    T_Page * page1;

      page0 = list->Pages;
      page1 = list->Pages->Next;
      
      page0->Next = page1->Next;
      page1->Prev = page0->Prev;
      page0->Prev = page1;
      page1->Next = page0;
      list->Pages = page0;
      return;
  }
  list->Pages = list->Pages->Next;
}

void Advance_in_list_of_pages(T_List_of_pages * list)
{
  // Cette fonction fait l'équivalent d'un "Redo" dans la liste de pages.
  // Elle effectue une sorte de ROR (Rotation Right) sur la liste:
  // +-+-+-+-+-+-+-+-+-+-+-+  |
  // |0|1|2|3|4|5|6|7|8|9|A|  |
  // +-+-+-+-+-+-+-+-+-+-+-+  |  0=page courante
  //  | | | | | | | | | | |   |_ A=page la plus ancienne
  //  v v v v v v v v v v v   |  1=Dernière page (1er backup)
  // +-+-+-+-+-+-+-+-+-+-+-+  |
  // |A|0|1|2|3|4|5|6|7|8|9|  |
  // +-+-+-+-+-+-+-+-+-+-+-+  |

  // Pour simuler un véritable Redo, l'appelant doit mettre la structure
  // de page courante à jour avant l'appel, puis en réextraire les infos en
  // sortie, ainsi que celles relatives à la plus récente page d'undo (1ère
  // page de la liste).
  if (Last_backed_up_layers)
  {
    // First page contains a ready-made backup of its ->Next.
    // We have swap the first two pages, so the original page 0
    // will end up in position -1 again, and then overwrite it with a backup
    // of the 'new' page1.
    T_Page * page0;
    T_Page * page1;

      page0 = list->Pages;
      page1 = list->Pages->Prev;
      
      page0->Prev = page1->Prev;
      page1->Next = page0->Next;
      page0->Next = page1;
      page1->Prev = page0;
      list->Pages = page1;
      return;
  }
  list->Pages = list->Pages->Prev;
}

void Free_last_page_of_list(T_List_of_pages * list)
{
  if (list!=NULL)
  {
    if (list->List_size>0)
    {
        T_Page * page;
        // The last page is the one before first
        page = list->Pages->Prev;
        
        page->Next->Prev = page->Prev;
        page->Prev->Next = page->Next;
        Clear_page(page);
        free(page->File_directory);
        free(page->Filename);
        free(page->Filename_unicode);
        free(page);
        page = NULL;
        list->List_size--;
    }
  }
}

// layer tells which layers have to be fresh copies instead of references :
// it's a layer number (>=0) or LAYER_NONE or LAYER_ALL
int Create_new_page(T_Page * new_page, T_List_of_pages * list, int layer)
{

//   This function fills the "Image" field of a new Page,
// based on the pages's attributes (width,height,...)
// then pushes it on front of a Page list.

  if (list->List_size >= (Config.Max_undo_pages+1))
  {
    // List is full.
    // If some other memory-limit was to be implemented, here would
    // be the right place to do it.
    // For example, we could rely on Stats_pages_memory, 
    // because it's the sum of all bitmaps in use (in bytes).
    
    // Destroy the latest page
    Free_last_page_of_list(list);
  }
  {
    int i;
    for (i=0; i<new_page->Nb_layers; i++)
    {
      if (layer == LAYER_ALL || i == layer)
        new_page->Image[i].Pixels=New_layer(new_page->Height*new_page->Width);
      else
        new_page->Image[i].Pixels=Dup_layer(list->Pages->Image[i].Pixels);
      new_page->Image[i].Duration=list->Pages->Image[i].Duration;
    }
  }
  

  // Insert as first
  new_page->Next = list->Pages;
  new_page->Prev = list->Pages->Prev;
  list->Pages->Prev->Next = new_page;
  list->Pages->Prev = new_page;
  list->Pages = new_page;
  list->List_size++;
  
  return 1;
}

void Change_page_number_of_list(T_List_of_pages * list,int number)
{
  // Truncate the list if larger than requested
  while(list->List_size > number)
  {
    Free_last_page_of_list(list);
  }
}

void Free_page_of_a_list(T_List_of_pages * list)
{
  // On ne peut pas détruire la page courante de la liste si après
  // destruction il ne reste pas encore au moins une page.
  if (list->List_size>1)
  {
    // On fait faire un undo à la liste, comme ça, la nouvelle page courante
    // est la page précédente
    Backward_in_list_of_pages(Main.backups);

    // Puis on détruit la dernière page, qui est l'ancienne page courante
    Free_last_page_of_list(list);
  }
}

void Update_screen_targets(void)
{
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    Main_screen=Main.visible_image.Image;
    Screen_backup=Main_visible_image_backup.Image;
  }
  else
  {
    Main_screen=Main.backups->Pages->Image[Main.current_layer].Pixels;
    // Sometimes this function will be called in situations where the
    // current history step and previous one don't have as many layers.
    // I don't like the idea of letting Screen_backup NULL or dangling,
    // so in case Screen_backup was queried, it will point to a valid
    // readable bitmap of correct size : current image.
    if (Main.backups->Pages->Nb_layers != Main.backups->Pages->Next->Nb_layers
     || Main.backups->Pages->Width != Main.backups->Pages->Next->Width
     || Main.backups->Pages->Height != Main.backups->Pages->Next->Height)
      Screen_backup=Main_screen;
    else
      Screen_backup=Main.backups->Pages->Next->Image[Main.current_layer].Pixels;
  }
  Update_pixel_renderer();
}

/// Update all the special image buffers, if necessary.
int Update_buffers(int width, int height)
{
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // At least one dimension is different
    if (Main.visible_image.Width*Main.visible_image.Height != width*height)
    {
      // Current image
      free(Main.visible_image.Image);
      Main.visible_image.Image = (byte *)GFX2_malloc(width * height);
      if (Main.visible_image.Image == NULL)
        return 0;
    }
    Main.visible_image.Width = width;
    Main.visible_image.Height = height;

    if (Main_visible_image_backup.Width*Main_visible_image_backup.Height != width*height)
    {
      // Previous image
      free(Main_visible_image_backup.Image);
      Main_visible_image_backup.Image = (byte *)GFX2_malloc(width * height);
      if (Main_visible_image_backup.Image == NULL)
        return 0;
    }
    Main_visible_image_backup.Width = width;
    Main_visible_image_backup.Height = height;
  
    if (Main_visible_image_depth_buffer.Width*Main_visible_image_depth_buffer.Height != width*height)
    {      
      // Depth buffer
      free(Main_visible_image_depth_buffer.Image);
      Main_visible_image_depth_buffer.Image = (byte *)GFX2_malloc(width * height);
      if (Main_visible_image_depth_buffer.Image == NULL)
        return 0;
    }
    Main_visible_image_depth_buffer.Width = width;
    Main_visible_image_depth_buffer.Height = height;
  }
  Update_screen_targets();
  return 1;
}
/// Update all the special image buffers of the spare page, if necessary.
int Update_spare_buffers(int width, int height)
{
  if (Spare.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // At least one dimension is different
    if (Spare.visible_image.Width*Spare.visible_image.Height != width*height)
    {
      // Current image
      free(Spare.visible_image.Image);
      Spare.visible_image.Image = (byte *)GFX2_malloc(width * height);
      if (Spare.visible_image.Image == NULL)
        return 0;
    }
    Spare.visible_image.Width = width;
    Spare.visible_image.Height = height;
    
  }
  return 1;
}

///
/// GESTION DES BACKUPS
///

int Init_all_backup_lists(enum IMAGE_MODES image_mode, int width, int height)
{
  // width et height correspondent à la dimension des images de départ.
  int i;

  if (! Allocate_list_of_pages(Main.backups) ||
      ! Allocate_list_of_pages(Spare.backups))
    return 0;
  // On a réussi à allouer deux listes de pages dont la taille correspond à
  // celle demandée par l'utilisateur.

  // On crée un descripteur de page correspondant à la page principale
  Upload_infos_page(&Main);
  // On y met les infos sur la dimension de démarrage
  Main.backups->Pages->Width = width;
  Main.backups->Pages->Height = height;
  Main.backups->Pages->File_directory = strdup(Main.selector.Directory);
  Main.backups->Pages->Filename = strdup("NO_NAME.GIF");
  Main.backups->Pages->Filename_unicode = NULL;


  for (i=0; i<Main.backups->Pages->Nb_layers; i++)
  {
    Main.backups->Pages->Image[i].Pixels=New_layer(width*height);
    if (! Main.backups->Pages->Image[i].Pixels)
      return 0;
    memset(Main.backups->Pages->Image[i].Pixels, 0, width*height);
  }
  Main.visible_image.Width = 0;
  Main.visible_image.Height = 0;
  Main.visible_image.Image = NULL;
  Main_visible_image_backup.Image = NULL;
  Main_visible_image_depth_buffer.Image = NULL;
  Main.backups->Pages->Image_mode = image_mode;
  Spare.visible_image.Width = 0;
  Spare.visible_image.Height = 0;
  Spare.visible_image.Image = NULL;
  Spare.backups->Pages->Image_mode = image_mode;

  if (!Update_buffers(width, height))
    return 0;
  if (!Update_spare_buffers(width, height))
    return 0;
    
  // For speed, instead of Redraw_layered_image() we'll directly set the buffers.
  if (Main.visible_image.Image != NULL)
  {
    memset(Main.visible_image.Image, 0, width*height);
    memset(Main_visible_image_backup.Image, 0, width*height);
    memset(Main_visible_image_depth_buffer.Image, 0, width*height);
  }
  if (Spare.visible_image.Image != NULL)
    memset(Spare.visible_image.Image, 0, width*height);

  Download_infos_page_main(Main.backups->Pages);
  Update_FX_feedback(Config.FX_Feedback);

  // Default values for spare page
  Spare.backups->Pages->Width = width;
  Spare.backups->Pages->Height = height;
  memcpy(Spare.backups->Pages->Palette,Main.palette,sizeof(T_Palette));
  strcpy(Spare.backups->Pages->Comment,"");
  Spare.backups->Pages->File_directory = strdup(Main.selector.Directory);
  Spare.backups->Pages->Filename = strdup("NO_NAME2.GIF");
  Spare.backups->Pages->Filename_unicode = NULL;

  Spare.backups->Pages->File_format=DEFAULT_FILEFORMAT;
  // Copy this informations in the global Spare_ variables
  Download_infos_page_spare(Spare.backups->Pages);
    
  // Clear the initial Visible buffer
  //memset(Main_screen,0,Main.image_width*Main.image_height);

  // Spare
  for (i=0; i<NB_LAYERS; i++)
  {
    Spare.backups->Pages->Image[i].Pixels=New_layer(width*height);
    if (! Spare.backups->Pages->Image[i].Pixels)
      return 0;
    memset(Spare.backups->Pages->Image[i].Pixels, 0, width*height);

  }
  //memset(Spare_screen,0,Spare.image_width*Spare.image_height);

  End_of_modification();
  return 1;
}

void Set_number_of_backups(int nb_backups)
{
  Change_page_number_of_list(Main.backups,nb_backups+1);
  Change_page_number_of_list(Spare.backups,nb_backups+1);

  // Le +1 vient du fait que dans chaque liste, en 1ère position on retrouve
  // les infos de la page courante sur le brouillon et la page principale.
  // (nb_backups = Nombre de backups, sans compter les pages courantes)
}

int Backup_new_image(int layers,int width,int height)
{
  // Retourne 1 si une nouvelle page est disponible et 0 sinon
  T_Page * new_page;  

  // On crée un descripteur pour la nouvelle page courante
  new_page=New_page(layers);
  if (!new_page)
  {
    Error(0);
    return 0;
  }
  new_page->Width=width;
  new_page->Height=height;
  new_page->Transparent_color=0;
  new_page->Gradients = Dup_gradient(NULL);
  if (!Create_new_page(new_page,Main.backups,LAYER_ALL))
  {
    Error(0);
    return 0;
  }
  
  Update_buffers(width, height);
  memset(Main_visible_image_depth_buffer.Image, 0, width*height);
  
  Download_infos_page_main(Main.backups->Pages);
  
  return 1;
}


int Backup_with_new_dimensions(int width,int height)
{
  // Retourne 1 si une nouvelle page est disponible (alors pleine de 0) et
  // 0 sinon.

  T_Page * new_page;  
  int i;

  // On crée un descripteur pour la nouvelle page courante
  new_page=New_page(Main.backups->Pages->Nb_layers);
  if (!new_page)
  {
    Error(0);
    return 0;
  }
  new_page->Width=width;
  new_page->Height=height;
  new_page->Transparent_color=0;
  if (!Create_new_page(new_page,Main.backups,LAYER_ALL))
  {
    Error(0);
    return 0;
  }
  
  // Copy data from previous history step
  memcpy(Main.backups->Pages->Palette, Main.backups->Pages->Next->Palette, sizeof(T_Palette));
  strcpy(Main.backups->Pages->Comment ,Main.backups->Pages->Next->Comment);
  Main.backups->Pages->File_format = Main.backups->Pages->Next->File_format;
  Main.backups->Pages->Filename = strdup(Main.backups->Pages->Next->Filename);
  Main.backups->Pages->Filename_unicode = Unicode_strdup(Main.backups->Pages->Next->Filename_unicode);
  Main.backups->Pages->File_directory = strdup(Main.backups->Pages->Next->File_directory);
  Main.backups->Pages->Gradients = Dup_gradient(Main.backups->Pages->Next);
  Main.backups->Pages->Background_transparent = Main.backups->Pages->Next->Background_transparent;
  Main.backups->Pages->Transparent_color = Main.backups->Pages->Next->Transparent_color;
  Main.backups->Pages->Image_mode = Main.backups->Pages->Next->Image_mode;
  
  // Fill with transparent color
  for (i=0; i<Main.backups->Pages->Nb_layers;i++)
  {
    memset(Main.backups->Pages->Image[i].Pixels, Main.backups->Pages->Transparent_color, width*height);
  }
  
  Update_buffers(width, height);

  Download_infos_page_main(Main.backups->Pages);
  
  // Same code as in End_of_modification(),
  // Without saving a safety backup:
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    memcpy(Main_visible_image_backup.Image,
           Main.visible_image.Image,
           Main.image_width*Main.image_height);
  }
  else
  {
    Update_screen_targets();
  }
  Update_FX_feedback(Config.FX_Feedback);
  // --
  
  return 1;
}

///
/// Resizes a backup step in-place (doesn't add a Undo/Redo step).
/// Should only be called after an actual backup, because it loses the current.
/// pixels. This function is meant to be used from within Lua scripts.
int Backup_in_place(int width,int height)
{
  // Retourne 1 si une nouvelle page est disponible (alors pleine de 0) et
  // 0 sinon.

  int i;
  byte ** new_layer;

  // Perform all allocations first
  
  new_layer=calloc(Main.backups->Pages->Nb_layers,sizeof(byte *));
  if (!new_layer)
    return 0;
  
  for (i=0; i<Main.backups->Pages->Nb_layers; i++)
  {
    new_layer[i]=New_layer(height*width);
    if (!new_layer[i])
    {
      // Allocation error
      for (; i>0; i--)
        free(new_layer[i]);
      free(new_layer);
      return 0;
    }
  }
  
  // Now ok to proceed
  
  for (i=0; i<Main.backups->Pages->Nb_layers; i++)
  {
    // Replace layers
    Free_layer(Main.backups->Pages,i);
    Main.backups->Pages->Image[i].Pixels=new_layer[i];
    
    // Fill with transparency
    memset(Main.backups->Pages->Image[i].Pixels, Main.backups->Pages->Transparent_color, width*height);
  }
  
  Main.backups->Pages->Width=width;
  Main.backups->Pages->Height=height;

  Download_infos_page_main(Main.backups->Pages);
  
  // The following is part of Update_buffers()
  // (without changing the backup buffer)
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // At least one dimension is different
    if (Main.visible_image.Width*Main.visible_image.Height != width*height)
    {
      // Current image
      free(Main.visible_image.Image);
      Main.visible_image.Image = (byte *)GFX2_malloc(width * height);
      if (Main.visible_image.Image == NULL)
        return 0;
    }
    Main.visible_image.Width = width;
    Main.visible_image.Height = height;
  
    if (Main_visible_image_depth_buffer.Width*Main_visible_image_depth_buffer.Height != width*height)
    {      
      // Depth buffer
      free(Main_visible_image_depth_buffer.Image);
      Main_visible_image_depth_buffer.Image = (byte *)GFX2_malloc(width * height);
      if (Main_visible_image_depth_buffer.Image == NULL)
        return 0;
    }
    Main_visible_image_depth_buffer.Width = width;
    Main_visible_image_depth_buffer.Height = height;
    
  }
  Update_screen_targets();
  
  return 1;
}

int Backup_and_resize_the_spare(int width,int height)
{
  // Retourne 1 si la page de dimension souhaitee est disponible en brouillon
  // et 0 sinon.

  T_Page * new_page;
  int return_code=0;
  int nb_layers;

  nb_layers=Spare.backups->Pages->Nb_layers;
  // On crée un descripteur pour la nouvelle page de brouillon
  new_page=New_page(nb_layers);
  if (!new_page)
  {
    Error(0);
    return 0;
  }
  
  // Fill it with a copy of the latest history
  Copy_S_page(new_page,Spare.backups->Pages);
  
  new_page->Width=width;
  new_page->Height=height;
  if (Create_new_page(new_page,Spare.backups,LAYER_ALL))
  {
    byte i;
    
    for (i=0; i<nb_layers;i++)
    {
      memset(Spare.backups->Pages->Image[i].Pixels, Spare.backups->Pages->Transparent_color, width*height);
    }
    
    // Update_buffers(width, height); // Not for spare
    
    Download_infos_page_spare(Spare.backups->Pages);
    
    // Light up the 'has unsaved changes' indicator
    Spare.image_is_modified=1;
    
    return_code=1;
  }
  return return_code;
}

void Backup(void)
// Sauve la page courante comme première page de backup et crée une nouvelle page
// pur continuer à dessiner. Utilisé par exemple pour le fill
{
  Backup_layers(Main.current_layer);
}

void Backup_layers(int layer)
{
  int i;
  T_Page *new_page;

  /*
  if (Last_backed_up_layers == (1<<Main.current_layer))
    return; // Already done.
  */

  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page(&Main);

  // Create a fresh Page descriptor
  new_page=New_page(Main.backups->Pages->Nb_layers);
  if (!new_page)
  {
    Error(0);
    return;
  }
  
  // Fill it with a copy of the latest history
  Copy_S_page(new_page,Main.backups->Pages);
  Create_new_page(new_page,Main.backups,layer);
  Download_infos_page_main(new_page);

  Update_FX_feedback(Config.FX_Feedback);

  // Copy the actual pixels from the backup to the latest page
  if (layer != LAYER_NONE)
  {
    for (i=0; i<Main.backups->Pages->Nb_layers;i++)
    {
      if (layer == LAYER_ALL || i == layer)
        memcpy(Main.backups->Pages->Image[i].Pixels,
               Main.backups->Pages->Next->Image[i].Pixels,
               Main.image_width*Main.image_height);
    }
  }
  // Light up the 'has unsaved changes' indicator
  Main.image_is_modified=1;
  
  /*
  Last_backed_up_layers = 1<<Main.current_layer;
  */
}

/// Backs up a layer, unless it's already different from previous history step.
// This function checks if a layer/frame shares the same
// bitmap as its Undo history parent.
// If this is the case, it instanciates a new copy, and returns true.
// Otherwise, it returns false.
int Dup_layer_if_shared(T_Page * page, int layer)
{
  if (page->Image[layer].Pixels == page->Next->Image[layer].Pixels)
  {
    Free_layer(page, layer);
    page->Image[layer].Pixels=New_layer(page->Height*page->Width);
    memcpy(
      page->Image[layer].Pixels,
      page->Next->Image[layer].Pixels,
      page->Width*page->Height);
    return 1;
  }
  return 0;
}

void Backup_the_spare(int layer)
{
  int i;
  T_Page *new_page;

  // Create a fresh Page descriptor
  new_page=New_page(Spare.backups->Pages->Nb_layers);
  if (!new_page)
  {
    Error(0);
    return;
  }
  
  // Fill it with a copy of the latest history
  Copy_S_page(new_page,Spare.backups->Pages);
  Create_new_page(new_page,Spare.backups,layer);

  // Copy the actual pixels from the backup to the latest page
  if (layer != LAYER_NONE)
  {
    for (i=0; i<Spare.backups->Pages->Nb_layers;i++)
    {
      if (layer == LAYER_ALL || i == layer)
        memcpy(Spare.backups->Pages->Image[i].Pixels,
               Spare.backups->Pages->Next->Image[i].Pixels,
               Spare.image_width*Spare.image_height);
    }
  }
  // Light up the 'has unsaved changes' indicator
  Spare.image_is_modified=1;

}

void Check_layers_limits()
{
  if (Main.current_layer > Main.backups->Pages->Nb_layers-1)
  {
    Main.current_layer = Main.backups->Pages->Nb_layers-1;
    Main.layers_visible |= 1<<Main.current_layer;
  }
}
    
void Undo(void)
{
  int width = Main.image_width;
  int height = Main.image_height;

  if (Last_backed_up_layers)
  {
    Free_page_of_a_list(Main.backups);
    Last_backed_up_layers=0;
  }

  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page(&Main);
  // On fait faire un undo à la liste des backups de la page principale
  Backward_in_list_of_pages(Main.backups);

  Update_buffers(Main.backups->Pages->Width, Main.backups->Pages->Height);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main.backups->Pages);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Undo. Donc ça ne devrait pas
  //       poser de problèmes.
  
  Check_layers_limits();
  Redraw_layered_image();
  End_of_modification();

  if (width != Main.image_width || height != Main.image_height)
    Tilemap_update();
}

void Redo(void)
{
  int width = Main.image_width;
  int height = Main.image_height;

  if (Last_backed_up_layers)
  {
    Free_page_of_a_list(Main.backups);
    Last_backed_up_layers=0;
  }
  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page(&Main);
  // On fait faire un redo à la liste des backups de la page principale
  Advance_in_list_of_pages(Main.backups);

  Update_buffers(Main.backups->Pages->Width, Main.backups->Pages->Height);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main.backups->Pages);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Redo. Donc ça ne devrait pas
  //       poser de problèmes.
  
  Check_layers_limits();
  Redraw_layered_image();
  End_of_modification();

  if (width != Main.image_width || height != Main.image_height)
    Tilemap_update();
}

void Free_current_page(void)
{
  // On détruit la page courante de la liste principale
  Free_page_of_a_list(Main.backups);
  
  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main.backups->Pages);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'une destruction de page. Donc ça ne
  //       devrait pas poser de problèmes.
   
  Update_buffers(Main.backups->Pages->Width, Main.backups->Pages->Height);
  Check_layers_limits();
  Redraw_layered_image();
  End_of_modification();
}

void End_of_modification(void)
{

  //Update_buffers(Main.image_width, Main.image_height);
  
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    // Backup buffer can have "wrong" size if a Lua script
    // performs a resize.
    Update_buffers(Main.image_width, Main.image_height);
    //
  
    memcpy(Main_visible_image_backup.Image,
           Main.visible_image.Image,
           Main.image_width*Main.image_height);
  }
  else
  {
    Update_screen_targets();
  }
  Update_FX_feedback(Config.FX_Feedback);
/*  
  Last_backed_up_layers = 0;
  Backup();
  */
  //
  // Processing safety backups
  //
  Main.edits_since_safety_backup++;
  Rotate_safety_backups();
}

/// Add a new layer to latest page of a list. Returns 0 on success.
byte Add_layer(T_List_of_pages *list, int layer)
{
  T_Page * source_page;
  T_Page * new_page;
  byte * new_image;
  int i;
  int duration;
  
  source_page = list->Pages;

  if (list->Pages->Nb_layers >= Layers_max(list->Pages->Image_mode)) // MAX_NB_LAYERS
    return 1;
   
  // Keep the position reasonable
  if (layer > list->Pages->Nb_layers)
    layer = list->Pages->Nb_layers;
   
  // Allocate the pixel data
  new_image = New_layer(list->Pages->Height*list->Pages->Width);
  if (! new_image)
  {
    Error(0);
    return 1;
  }
  // Re-allocate the page itself, with room for one more pointer
  new_page = realloc(source_page, sizeof(T_Page)+(list->Pages->Nb_layers+1)*sizeof(T_Image));
  if (!new_page)
  {
    Error(0);
    return 1;
  }
  if (new_page != source_page)
  {
    // Need some housekeeping because the page moved in memory.
    // Update all pointers that pointed to it:
    new_page->Prev->Next = new_page;
    new_page->Next->Prev = new_page;
    list->Pages = new_page;
  }
  list->Pages->Nb_layers++;
  // Move around the pointers. This part is going to be tricky when we
  // have 'animations x layers' in this vector.
  for (i=list->Pages->Nb_layers-1; i>layer ; i--)
  {
    new_page->Image[i]=new_page->Image[i-1];
  }
  new_page->Image[layer].Pixels=new_image;
  if (list->Pages->Nb_layers==0)
    duration=100;
  else if (layer>0)
    duration=new_page->Image[layer-1].Duration;
  else
    duration=new_page->Image[1].Duration;
  new_page->Image[layer].Duration=duration;
  // Fill with transparency, initially
  memset(new_image, Main.backups->Pages->Transparent_color, list->Pages->Height*list->Pages->Width); // transparent color
  
  // Done. Note that the visible buffer is already ok since we
  // only inserted a transparent "slide" somewhere.
  // The depth buffer is all wrong though.

  // Update the flags of visible layers. 
  {
    dword layers_before;
    dword layers_after;
    dword *visible_layers_flag;
    
    // Determine if we're modifying the spare or the main page.
    if (list == Main.backups)
    {
      visible_layers_flag = &Main.layers_visible;
      Main.current_layer = layer;
    }
    else
    {
      visible_layers_flag = &Spare.layers_visible;
      Spare.current_layer = layer;
    }
    
    // Fun with binary!
    layers_before = ((1<<layer)-1) & *visible_layers_flag;
    layers_after = (*visible_layers_flag & (~layers_before))<<1;
    *visible_layers_flag = (1<<layer) | layers_before | layers_after;
  }
  
  // All ok
  return 0;
}

/// Delete a layer from the latest page of a list. Returns 0 on success.
byte Delete_layer(T_List_of_pages *list, int layer)
{
  int i;
  
  // Keep the position reasonable
  if (layer >= list->Pages->Nb_layers)
    layer = list->Pages->Nb_layers - 1;
  if (list->Pages->Nb_layers == 1)
    return 1;
   
  // For simplicity, we won't actually shrink the page in terms of allocation.
  // It would only save the size of a pointer, and anyway, as the user draws,
  // this page is going to fall off the end of the Undo-list
  // and so it will be cleared anyway.
  
  // Smart freeing of the pixel data
  Free_layer(list->Pages, layer);
  
  list->Pages->Nb_layers--;
  // Move around the pointers. This part is going to be tricky when we
  // have 'animations x layers' in this vector.
  for (i=layer; i < list->Pages->Nb_layers; i++)
  {
    list->Pages->Image[i]=list->Pages->Image[i+1];
  }
  
  // Done. At this point the visible buffer and the depth buffer are
  // all wrong.

  // Update the flags of visible layers. 
  {
    dword layers_before;
    dword layers_after;
    dword *visible_layers_flag;
    byte new_current_layer;
    
    // Determine if we're modifying the spare or the main page.
    if (list == Main.backups)
    {
      visible_layers_flag = &Main.layers_visible;
      if (Main.current_layer>=layer && Main.current_layer>0)
        Main.current_layer--;
      new_current_layer = Main.current_layer;
    }
    else
    {
      visible_layers_flag = &Spare.layers_visible;
      if (Spare.current_layer>=layer && Spare.current_layer>0)
        Spare.current_layer--;
      new_current_layer = Spare.current_layer;
    }

    // Fun with binary!
    layers_before = ((1<<layer)-1) & *visible_layers_flag;
    layers_after = (*visible_layers_flag & (~layers_before))>>1;
    *visible_layers_flag = layers_before | layers_after;
    // Ensure the current layer is part what is shown.
    *visible_layers_flag |= 1<<new_current_layer;
  }

  // All ok
  return 0;
}


/// Merges the current layer onto the one below it.
byte Merge_layer(void)
{
  int i;
  for (i=0; i<Main.image_width*Main.image_height; i++)
  {
    byte color = *(Main.backups->Pages->Image[Main.current_layer].Pixels+i);
    if (color != Main.backups->Pages->Transparent_color) // transparent color
      *(Main.backups->Pages->Image[Main.current_layer-1].Pixels+i) = color;
  }
  return Delete_layer(Main.backups,Main.current_layer);
}


void Switch_layer_mode(enum IMAGE_MODES new_mode)
{
  if (new_mode == Main.backups->Pages->Image_mode)
    return;

  Main.backups->Pages->Image_mode = new_mode;

  if (new_mode != IMAGE_MODE_ANIMATION)
  {
      Update_buffers(Main.image_width, Main.image_height);
      Redraw_layered_image();
  }
  // TODO Eventually, in animation mode we may clear the buffers to save a bit of memory...

  Update_pixel_renderer();
}
