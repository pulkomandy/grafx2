/* vim:expandtab:ts=2 sw=2:
*/
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
*/
//////////////////////////////////////////////////////////////////////////
/////////////////////////// GESTION DU BACKUP ////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "pages.h"
#include "errors.h"
#include "misc.h"
#include "windows.h"

// -- Layers data

/// Array of two images, that contains the "flattened" version of the visible layers.
#ifndef NOLAYERS
T_Image Visible_image[2];
T_Image Visible_image_depth_buffer;
#endif


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
T_Page * New_page(byte nb_layers)
{
  T_Page * page;
  
  page = (T_Page *)malloc(sizeof(T_Page)+nb_layers*sizeof(byte *));
  if (page!=NULL)
  {
    int i;
    for (i=0; i<nb_layers; i++)
      page->Image[i]=NULL;
    page->Width=0;
    page->Height=0;
    memset(page->Palette,0,sizeof(T_Palette));
    page->Comment[0]='\0';
    page->File_directory[0]='\0';
    page->Filename[0]='\0';
    page->File_format=DEFAULT_FILEFORMAT;
    page->Nb_layers=nb_layers;
    page->Transparent_color=0; // Default transparent color
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
  short * ptr = malloc(sizeof(short)+pixel_size);
  if (ptr==NULL)
    return NULL;
    
  // Stats
  Stats_pages_number++;
  Stats_pages_memory+=pixel_size;
  
  *ptr = 1;
  return (byte *)(ptr+1);
}

/// Free a layer
void Free_layer(T_Page * page, byte layer)
{
  short * ptr;
  if (page->Image[layer]==NULL)
    return;
    
  ptr = (short *)(page->Image[layer]);
  if (-- (*(ptr-1))) // Users--
    return;
  else
    free(ptr-1);
    
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

void Download_infos_page_main(T_Page * page)
// Affiche la page à l'écran
{
  //int factor_index;
  int size_is_modified;
  
  if (page!=NULL)
  {
    size_is_modified=(Main_image_width!=page->Width) ||
                         (Main_image_height!=page->Height);

    Main_image_width=page->Width;
    Main_image_height=page->Height;
    memcpy(Main_palette,page->Palette,sizeof(T_Palette));
    strcpy(Main_comment,page->Comment);
    strcpy(Main_file_directory,page->File_directory);
    strcpy(Main_filename,page->Filename);
    Main_fileformat=page->File_format;

    if (size_is_modified)
    {
      Main_magnifier_mode=0;
      Main_offset_X=0;
      Main_offset_Y=0;
      Pixel_preview=Pixel_preview_normal;
      Compute_limits();
      Compute_paintbrush_coordinates();
    }
    
  }
  //Update_buffers( page->Width, page->Height);
  //memcpy(Main_screen, page->Image[Main_current_layer], page->Width*page->Height);
  
}

void Redraw_layered_image(void)
{
  #ifndef NOLAYERS
  // Re-construct the image with the visible layers
  int layer;  
  // First layer
  for (layer=0; layer<Main_backups->Pages->Nb_layers; layer++)
  {
    if ((1<<layer) & Main_layers_visible)
    {
       // Copy it in Visible_image[0]
       memcpy(Visible_image[0].Image,
         Main_backups->Pages->Image[layer],
         Main_image_width*Main_image_height);
       
       // Initialize the depth buffer
       memset(Visible_image_depth_buffer.Image,
         layer,
         Main_image_width*Main_image_height);
       
       // skip all other layers
       layer++;
       break;
    }
  }
  // subsequent layer(s)
  for (; layer<Main_backups->Pages->Nb_layers; layer++)
  {
    if ((1<<layer) & Main_layers_visible)
    {
      int i;
      for (i=0; i<Main_image_width*Main_image_height; i++)
      {
        byte color = *(Main_backups->Pages->Image[layer]+i);
        if (color != Main_backups->Pages->Transparent_color) // transparent color
        {
          *(Visible_image[0].Image+i) = color;
          if (layer != Main_current_layer)
            *(Visible_image_depth_buffer.Image+i) = layer;
        }
      }
    }
  }
  #else
  Update_screen_targets();
  #endif
  Download_infos_backup(Main_backups);
}

void Update_depth_buffer(void)
{
  #ifndef NOLAYERS
  // Re-construct the depth buffer with the visible layers.
  // This function doesn't touch the visible buffer, it assumes
  // that it was already up-to-date. (Ex. user only changed active layer)

  int layer;  
  // First layer
  for (layer=0; layer<Main_backups->Pages->Nb_layers; layer++)
  {
    if ((1<<layer) & Main_layers_visible)
    {
       // Initialize the depth buffer
       memset(Visible_image_depth_buffer.Image,
         layer,
         Main_image_width*Main_image_height);
       
       // skip all other layers
       layer++;
       break;
    }
  }
  // subsequent layer(s)
  for (; layer<Main_backups->Pages->Nb_layers; layer++)
  {
    // skip the current layer, whenever we reach it
    if (layer == Main_current_layer)
      continue;
      
    if ((1<<layer) & Main_layers_visible)
    {
      int i;
      for (i=0; i<Main_image_width*Main_image_height; i++)
      {
        byte color = *(Main_backups->Pages->Image[layer]+i);
        if (color != Main_backups->Pages->Transparent_color) // transparent color
        {
          *(Visible_image_depth_buffer.Image+i) = layer;
        }
      }
    }
  }
  #endif
  Download_infos_backup(Main_backups);
}

void Redraw_current_layer(void)
{
#ifndef NOLAYERS
  int i;
  for (i=0; i<Main_image_width*Main_image_height; i++)
  {
    byte depth = *(Visible_image_depth_buffer.Image+i);
    if (depth<=Main_current_layer)
    {
      byte color = *(Main_backups->Pages->Image[Main_current_layer]+i);
      if (color != Main_backups->Pages->Transparent_color) // transparent color
      {
        *(Visible_image[0].Image+i) = color;
      }
      else
      {
        *(Visible_image[0].Image+i) = *(Main_backups->Pages->Image[depth]+i);
      }
    }
  }
#endif
}

void Upload_infos_page_main(T_Page * page)
// Sauve l'écran courant dans la page
{
  if (page!=NULL)
  {
    //page->Image[Main_current_layer]=Main_screen;
    page->Width=Main_image_width;
    page->Height=Main_image_height;
    memcpy(page->Palette,Main_palette,sizeof(T_Palette));
    strcpy(page->Comment,Main_comment);
    strcpy(page->File_directory,Main_file_directory);
    strcpy(page->Filename,Main_filename);
    page->File_format=Main_fileformat;
  }
}

void Download_infos_page_spare(T_Page * page)
{
  if (page!=NULL)
  {
    //Spare_screen=page->Image[Spare_current_layer];
    Spare_image_width=page->Width;
    Spare_image_height=page->Height;
    memcpy(Spare_palette,page->Palette,sizeof(T_Palette));
    strcpy(Spare_comment,page->Comment);
    strcpy(Spare_file_directory,page->File_directory);
    strcpy(Spare_filename,page->Filename);
    Spare_fileformat=page->File_format;
  }
}

void Upload_infos_page_spare(T_Page * page)
{
  if (page!=NULL)
  {
    //page->Image[Spare_current_layer]=Spare_screen;
    page->Width=Spare_image_width;
    page->Height=Spare_image_height;
    memcpy(page->Palette,Spare_palette,sizeof(T_Palette));
    strcpy(page->Comment,Spare_comment);
    strcpy(page->File_directory,Spare_file_directory);
    strcpy(page->Filename,Spare_filename);
    page->File_format=Spare_fileformat;
  }
}

void Download_infos_backup(T_List_of_pages * list)
{
  //list->Pages->Next->Image[Main_current_layer];

  if (Config.FX_Feedback)
    FX_feedback_screen=list->Pages->Image[Main_current_layer];
  else
    FX_feedback_screen=list->Pages->Next->Image[Main_current_layer];
}

void Clear_page(T_Page * page)
{
  // On peut appeler cette fonction sur une page non allouée.
  int i;
  for (i=0; i<page->Nb_layers; i++)
  {
    Free_layer(page, i);
    page->Image[i]=NULL;
  }
  page->Width=0;
  page->Height=0;
  // On ne se préoccupe pas de ce que deviens le reste des infos de l'image.
}

void Copy_S_page(T_Page * dest,T_Page * source)
{
  *dest=*source;
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
  page=New_page(NB_LAYERS);
  if (!page)
    return 0;
  
  // Set as first page of the list
  page->Next = page;
  page->Prev = page;
  list->Pages = page;

  list->List_size=1;

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
        free(page);
        list->List_size--;
    }
  }
}

// layer_mask tells which layers have to be fresh copies instead of references
int Create_new_page(T_Page * new_page, T_List_of_pages * list, dword layer_mask)
{

//   Cette fonction crée une nouvelle page dont les attributs correspondent à
// ceux de new_page (width,height,...) (le champ Image est invalide
// à l'appel, c'est la fonction qui le met à jour), et l'enfile dans
// list.


  if (list->List_size >= (Config.Max_undo_pages+1))
  {
    // On manque de mémoire ou la list est pleine. Dans tous les
    // cas, il faut libérer une page.
    
    // Détruire la dernière page allouée dans la Liste_à_raboter
    Free_last_page_of_list(list);
  }
  {
    int i;
    for (i=0; i<new_page->Nb_layers; i++)
    {
      if ((1<<i) & layer_mask)
        new_page->Image[i]=New_layer(new_page->Height*new_page->Width);
      else
        new_page->Image[i]=Dup_layer(list->Pages->Image[i]);
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
    Backward_in_list_of_pages(Main_backups);

    // Puis on détruit la dernière page, qui est l'ancienne page courante
    Free_last_page_of_list(list);
  }
}

/// Resize one of the special bitmap buffers, if necessary.
int Update_buffer(T_Image * image, int width, int height)
{
  // At least one dimension is different
  if (image->Width != width || image->Height != height)
  {
    // Actual size difference
    if (image->Width * image->Height != width * height)
    {
      free(image->Image);
      image->Image = (byte *)malloc(width * height);
      if (image->Image == NULL)
        return 0;
    }
    image->Width = width;
    image->Height = height;
  }
  return 1;
}

void Update_screen_targets(void)
{
  #ifndef NOLAYERS
    Main_screen=Visible_image[0].Image;
    Screen_backup=Visible_image[1].Image;
  #else
    Main_screen=Main_backups->Pages->Image[Main_current_layer];
    Screen_backup=Main_backups->Pages->Next->Image[Main_current_layer];
  #endif
}

/// Update all the special image buffers, if necessary.
int Update_buffers(int width, int height)
{
#ifndef NOLAYERS
  if (! Update_buffer(&Visible_image_depth_buffer, width, height))
    return 0;
  if (! Update_buffer(&Visible_image[0], width, height))
    return 0;
  if (! Update_buffer(&Visible_image[1], width, height))
    return 0;
#endif
  Update_screen_targets();
  return 1;
}

///
/// GESTION DES BACKUPS
///

int Init_all_backup_lists(int width,int height)
{
  // width et height correspondent à la dimension des images de départ.
  int i;

  if (! Allocate_list_of_pages(Main_backups) ||
      ! Allocate_list_of_pages(Spare_backups))
    return 0;
  // On a réussi à allouer deux listes de pages dont la taille correspond à
  // celle demandée par l'utilisateur.

  // On crée un descripteur de page correspondant à la page principale
  Upload_infos_page_main(Main_backups->Pages);
  // On y met les infos sur la dimension de démarrage
  Main_backups->Pages->Width=width;
  Main_backups->Pages->Height=height;
  for (i=0; i<Main_backups->Pages->Nb_layers; i++)
  {
    Main_backups->Pages->Image[i]=New_layer(width*height);
    if (! Main_backups->Pages->Image[i])
      return 0;
    memset(Main_backups->Pages->Image[i], 0, width*height);
  }
#ifndef NOLAYERS
  Visible_image[0].Width = 0;
  Visible_image[0].Height = 0;
  Visible_image[0].Image = NULL;

  Visible_image[1].Width = 0;
  Visible_image[1].Height = 0;
  Visible_image[1].Image = NULL;

  Visible_image_depth_buffer.Width = 0;
  Visible_image_depth_buffer.Height = 0;
  Visible_image_depth_buffer.Image = NULL;
#endif
  if (!Update_buffers(width, height))
    return 0;
#ifndef NOLAYERS
  // For speed, instead of Redraw_layered_image() we'll directly set the buffers.
  memset(Visible_image[0].Image, 0, width*height);
  memset(Visible_image[1].Image, 0, width*height);
  memset(Visible_image_depth_buffer.Image, 0, width*height);
#endif      
  Download_infos_page_main(Main_backups->Pages); 
  Download_infos_backup(Main_backups);

  // Default values for spare page
  Spare_backups->Pages->Width = width;
  Spare_backups->Pages->Height = height;
  memcpy(Spare_backups->Pages->Palette,Main_palette,sizeof(T_Palette));
  strcpy(Spare_backups->Pages->Comment,"");
  strcpy(Spare_backups->Pages->File_directory,Spare_current_directory);
  strcpy(Spare_backups->Pages->Filename,"NO_NAME.GIF");
  Spare_backups->Pages->File_format=DEFAULT_FILEFORMAT;
  // Copy this informations in the global Spare_ variables
  Download_infos_page_spare(Spare_backups->Pages);
    
  // Clear the initial Visible buffer
  //memset(Main_screen,0,Main_image_width*Main_image_height);

  // Spare
  for (i=0; i<NB_LAYERS; i++)
  {
    Spare_backups->Pages->Image[i]=New_layer(width*height);
    if (! Spare_backups->Pages->Image[i])
      return 0;
    memset(Spare_backups->Pages->Image[i], 0, width*height);

  }
  //memset(Spare_screen,0,Spare_image_width*Spare_image_height);

  End_of_modification();
  return 1;
}

void Set_number_of_backups(int nb_backups)
{
  Change_page_number_of_list(Main_backups,nb_backups+1);
  Change_page_number_of_list(Spare_backups,nb_backups+1);

  // Le +1 vient du fait que dans chaque liste, en 1ère position on retrouve
  // les infos de la page courante sur le brouillon et la page principale.
  // (nb_backups = Nombre de backups, sans compter les pages courantes)
}

int Backup_with_new_dimensions(int upload,byte layers,int width,int height)
{
  // Retourne 1 si une nouvelle page est disponible (alors pleine de 0) et
  // 0 sinon.

  T_Page * new_page;
  int return_code=0;
  int i;

  if (upload)
    // On remet à jour l'état des infos de la page courante (pour pouvoir les
    // retrouver plus tard)
    Upload_infos_page_main(Main_backups->Pages);

  // On crée un descripteur pour la nouvelle page courante
  new_page=New_page(layers);
  if (!new_page)
  {
    Error(0);
    return 0;
  }
  Upload_infos_page_main(new_page);
  new_page->Width=width;
  new_page->Height=height;
  if (Create_new_page(new_page,Main_backups,0xFFFFFFFF))
  {
    for (i=0; i<layers;i++)
    {
      memset(Main_backups->Pages->Image[i], 0, width*height);
    }
    
    Update_buffers(width, height);

    Download_infos_page_main(Main_backups->Pages);
    Download_infos_backup(Main_backups);
    
    return_code=1;
  }
  return return_code;
}

int Backup_and_resize_the_spare(int width,int height)
{
  // Retourne 1 si la page de dimension souhaitee est disponible en brouillon
  // et 0 sinon.

  T_Page * new_page;
  int return_code=0;
  byte nb_layers;


  // On remet à jour l'état des infos de la page de brouillon (pour pouvoir
  // les retrouver plus tard)
  Upload_infos_page_spare(Spare_backups->Pages);

  nb_layers=Spare_backups->Pages->Nb_layers;
  // On crée un descripteur pour la nouvelle page de brouillon
  new_page=New_page(nb_layers);
  if (!new_page)
  {
    Error(0);
    return 0;
  }
  Upload_infos_page_spare(new_page);
  new_page->Width=width;
  new_page->Height=height;
  if (Create_new_page(new_page,Spare_backups,0xFFFFFFFF))
  {
    byte i;
    
    for (i=0; i<nb_layers;i++)
    {
      memset(Spare_backups->Pages->Image[i], 0, width*height);
    }
    
    // Update_buffers(width, height); // Not for spare
    
    Download_infos_page_spare(Spare_backups->Pages);
    
    return_code=1;
  }
  return return_code;
}

void Backup(void)
// Sauve la page courante comme première page de backup et crée une nouvelle page
// pur continuer à dessiner. Utilisé par exemple pour le fill
{
  Backup_layers(1<<Main_current_layer);
}

void Backup_layers(dword layer_mask)
{
  int i;
  T_Page *new_page;

  /*
  if (Last_backed_up_layers == (1<<Main_current_layer))
    return; // Already done.
  */

  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);

  // On crée un descripteur pour la nouvelle page courante
  new_page=New_page(Main_backups->Pages->Nb_layers);
  if (!new_page)
  {
    Error(0);
    return;
  }
  
  // Enrichissement de l'historique
  Copy_S_page(new_page,Main_backups->Pages);
  Create_new_page(new_page,Main_backups,layer_mask);
  Download_infos_page_main(new_page);

  Download_infos_backup(Main_backups);

  // On copie l'image du backup vers la page courante:
  for (i=0; i<Main_backups->Pages->Nb_layers;i++)
  {
    if ((1<<i) & layer_mask)
      memcpy(Main_backups->Pages->Image[i],
             Main_backups->Pages->Next->Image[i],
             Main_image_width*Main_image_height);
  }
  // On allume l'indicateur de modification de l'image
  Main_image_is_modified=1;
  
  /*
  Last_backed_up_layers = 1<<Main_current_layer;
  */
}

void Undo(void)
{
  if (Last_backed_up_layers)
  {
    Free_page_of_a_list(Main_backups);
    Last_backed_up_layers=0;
  }

  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);
  // On fait faire un undo à la liste des backups de la page principale
  Backward_in_list_of_pages(Main_backups);

  Update_buffers(Main_backups->Pages->Width, Main_backups->Pages->Height);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main_backups->Pages);
  // Et celles du backup
  Download_infos_backup(Main_backups);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Undo. Donc ça ne devrait pas
  //       poser de problèmes.
  
  if (Main_current_layer > Main_backups->Pages->Nb_layers-1)
    Main_current_layer = Main_backups->Pages->Nb_layers-1;
  
  Redraw_layered_image();
  
}

void Redo(void)
{
  if (Last_backed_up_layers)
  {
    Free_page_of_a_list(Main_backups);
    Last_backed_up_layers=0;
  }
  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);
  // On fait faire un redo à la liste des backups de la page principale
  Advance_in_list_of_pages(Main_backups);

  Update_buffers(Main_backups->Pages->Width, Main_backups->Pages->Height);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main_backups->Pages);
  // Et celles du backup
  Download_infos_backup(Main_backups);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Redo. Donc ça ne devrait pas
  //       poser de problèmes.
  
  if (Main_current_layer > Main_backups->Pages->Nb_layers-1)
    Main_current_layer = Main_backups->Pages->Nb_layers-1;

  Redraw_layered_image();

}

void Free_current_page(void)
{
  // On détruit la page courante de la liste principale
  Free_page_of_a_list(Main_backups);
  
  Update_buffers(Main_backups->Pages->Width, Main_backups->Pages->Height);
  Redraw_layered_image();
  
  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main_backups->Pages);
  // Et celles du backup
  Download_infos_backup(Main_backups);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'une destruction de page. Donc ça ne
  //       devrait pas poser de problèmes.  
}

void Exchange_main_and_spare(void)
{
  T_List_of_pages * temp_list;

  // On commence par mettre à jour dans les descripteurs les infos sur les
  // pages qu'on s'apprête à échanger, pour qu'on se retrouve pas avec de
  // vieilles valeurs qui datent de mathuzalem.
  Upload_infos_page_main(Main_backups->Pages);
  Upload_infos_page_spare(Spare_backups->Pages);

  // On inverse les listes de pages
  temp_list=Main_backups;
  Main_backups=Spare_backups;
  Spare_backups=temp_list;

  // On extrait ensuite les infos sur les nouvelles pages courante, brouillon
  // et backup.

  Update_buffers(Main_backups->Pages->Width, Main_backups->Pages->Height);

  /* SECTION GROS CACA PROUT PROUT */
  // Auparavant on ruse en mettant déjà à jour les dimensions de la
  // nouvelle page courante. Si on ne le fait pas, le "Download" va détecter
  // un changement de dimensions et va bêtement sortir du mode loupe, alors
  // que lors d'un changement de page, on veut bien conserver l'état du mode
  // loupe du brouillon.
  Main_image_width=Main_backups->Pages->Width;
  Main_image_height=Main_backups->Pages->Height;

  Download_infos_page_main(Main_backups->Pages);
  Download_infos_backup(Main_backups);
  Download_infos_page_spare(Spare_backups->Pages);
  Redraw_layered_image();
}

void End_of_modification(void)
{

  //Update_buffers(Main_image_width, Main_image_height);
  
#ifndef NOLAYERS
  memcpy(Visible_image[1].Image,
         Visible_image[0].Image,
         Main_image_width*Main_image_height);
#else
  Update_screen_targets();
#endif
  
  Download_infos_backup(Main_backups);
/*  
  Last_backed_up_layers = 0;
  Backup();
  */
}

/// Add a new layer to latest page of a list. Returns 0 on success.
byte Add_layer(T_List_of_pages *list, byte layer)
{
  T_Page * source_page;
  T_Page * new_page;
  byte * new_image;
  int i;
  
  source_page = list->Pages;
  
  // Hard limit of 32 at the moment, because layer bitmasks are 32bit.
  if (list->Pages->Nb_layers == 32)
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
  new_page = realloc(source_page, sizeof(T_Page)+(list->Pages->Nb_layers+1)*sizeof(byte *));
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
  new_page->Image[layer]=new_image;
  // Fill with transparency, initially
  memset(new_image, Main_backups->Pages->Transparent_color, list->Pages->Height*list->Pages->Width); // transparent color
  
  // Done. Note that the visible buffer is already ok since we
  // only inserted a transparent "slide" somewhere.
  // The depth buffer is all wrong though.

  // Update the flags of visible layers. 
  {
    dword layers_before;
    dword layers_after;
    dword *visible_layers_flag;
    
    // Determine if we're modifying the spare or the main page.
    if (list == Main_backups)
    {
      visible_layers_flag = &Main_layers_visible;
      Main_current_layer = layer;
    }
    else
    {
      visible_layers_flag = &Spare_layers_visible;
      Spare_current_layer = layer;
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
byte Delete_layer(T_List_of_pages *list, byte layer)
{
  T_Page * page;
  int i;
  
  page = list->Pages;
   
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
    if (list == Main_backups)
    {
      visible_layers_flag = &Main_layers_visible;
      if (Main_current_layer>=layer && Main_current_layer>0)
        Main_current_layer--;
      new_current_layer = Main_current_layer;
    }
    else
    {
      visible_layers_flag = &Spare_layers_visible;
      if (Spare_current_layer>=layer && Spare_current_layer>0)
        Spare_current_layer--;
      new_current_layer = Spare_current_layer;
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
byte Merge_layer()
{
  int i;
  for (i=0; i<Main_image_width*Main_image_height; i++)
  {
    byte color = *(Main_backups->Pages->Image[Main_current_layer]+i);
    if (color != Main_backups->Pages->Transparent_color) // transparent color
      *(Main_backups->Pages->Image[Main_current_layer-1]+i) = color;
  }
  return Delete_layer(Main_backups,Main_current_layer);
}
