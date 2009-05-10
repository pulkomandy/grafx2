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

  ///
  /// GESTION DES PAGES
  ///

void Init_page(T_Page * page)
{
  // Important: appeler cette fonction sur toute nouvelle structure T_Page!

  if (page!=NULL)
  {
    page->Image=NULL;
    page->Width=0;
    page->Height=0;
    memset(page->Palette,0,sizeof(T_Palette));
    page->Comment[0]='\0';
    page->File_directory[0]='\0';
    page->Filename[0]='\0';
    page->File_format=DEFAULT_FILEFORMAT;
/*
    page->X_offset=0;
    page->Y_offset=0;
    page->old_offset_x=0;
    page->old_offset_y=0;
    page->Split=0;
    page->X_zoom=0;
    page->Separator_proportion=INITIAL_SEPARATOR_PROPORTION;
    page->Main_magnifier_mode=0;
    page->Main_magnifier_factor=DEFAULT_ZOOM_FACTOR;
    page->Main_magnifier_height=0;
    page->Main_magnifier_width=0;
    page->Main_magnifier_offset_X=0;
    page->Main_magnifier_offset_Y=0;
*/
  }
}

void Download_infos_page_main(T_Page * page)
// Affiche la page à l'écran
{
  //int factor_index;
  int size_is_modified;

  if (page!=NULL)
  {
    size_is_modified=(Main_image_width!=page->Width) ||
                         (Main_image_height!=page->Height);

    Main_screen=page->Image;
    Main_image_width=page->Width;
    Main_image_height=page->Height;
    memcpy(Main_palette,page->Palette,sizeof(T_Palette));
    strcpy(Main_comment,page->Comment);
    strcpy(Main_file_directory,page->File_directory);
    strcpy(Main_filename,page->Filename);
    Main_fileformat=page->File_format;
/*
    Main_offset_X=page->X_offset;
    Main_offset_Y=page->Y_offset;

    // On corrige les décalages en fonction de la dimension de l'écran
    if ( (Main_offset_X>0) &&
         (Main_offset_X+Screen_width>Main_image_width) )
      Main_offset_X=Max(0,Main_image_width-Screen_width);
    if ( (Main_offset_Y>0) &&
         (Main_offset_Y+Menu_Y>Main_image_height) )
      Main_offset_Y=Max(0,Main_image_height-Menu_Y);

    Old_main_offset_X=page->old_offset_x;
    Old_main_offset_Y=page->old_offset_y;
    Main_separator_position=page->Split;
    Main_X_zoom=page->X_zoom;
    Main_separator_proportion=page->Separator_proportion;
    Main_magnifier_mode=page->Main_magnifier_mode;
    Main_magnifier_factor=page->Main_magnifier_factor;
    Main_magnifier_height=page->Main_magnifier_height;
    Main_magnifier_width=page->Main_magnifier_width;
    Main_magnifier_offset_X=page->Main_magnifier_offset_X;
    Main_magnifier_offset_Y=page->Main_magnifier_offset_Y;

    // Comme le facteur de zoom a des chances d'avoir changé, on appelle
    // "Change_magnifier_factor".
    for (factor_index=0; ZOOM_FACTOR[factor_index]!=Main_magnifier_factor; factor_index++);
    Change_magnifier_factor(factor_index);
*/
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
}

void Upload_infos_page_main(T_Page * page)
// Sauve l'écran courant dans la page
{
  if (page!=NULL)
  {
    page->Image=Main_screen;
    page->Width=Main_image_width;
    page->Height=Main_image_height;
    memcpy(page->Palette,Main_palette,sizeof(T_Palette));
    strcpy(page->Comment,Main_comment);
    strcpy(page->File_directory,Main_file_directory);
    strcpy(page->Filename,Main_filename);
    page->File_format=Main_fileformat;
/*
    page->X_offset=Main_offset_X;
    page->Y_offset=Main_offset_Y;
    page->old_offset_x=Old_main_offset_X;
    page->old_offset_x=Old_main_offset_Y;
    page->Split=Main_separator_position;
    page->X_zoom=Main_X_zoom;
    page->Separator_proportion=Main_separator_proportion;
    page->Main_magnifier_mode=Main_magnifier_mode;
    page->Main_magnifier_factor=Main_magnifier_factor;
    page->Main_magnifier_height=Main_magnifier_height;
    page->Main_magnifier_width=Main_magnifier_width;
    page->Main_magnifier_offset_X=Main_magnifier_offset_X;
    page->Main_magnifier_offset_Y=Main_magnifier_offset_Y;
*/
  }
}

void Download_infos_page_spare(T_Page * page)
{
  if (page!=NULL)
  {
    Spare_screen=page->Image;
    Spare_image_width=page->Width;
    Spare_image_height=page->Height;
    memcpy(Spare_palette,page->Palette,sizeof(T_Palette));
    strcpy(Spare_comment,page->Comment);
    strcpy(Spare_file_directory,page->File_directory);
    strcpy(Spare_filename,page->Filename);
    Spare_fileformat=page->File_format;
/*
    Spare_offset_X=page->X_offset;
    Spare_offset_Y=page->Y_offset;
    Old_spare_offset_X=page->old_offset_x;
    Old_spare_offset_Y=page->old_offset_y;
    Spare_separator_position=page->Split;
    Spare_X_zoom=page->X_zoom;
    Spare_separator_proportion=page->Separator_proportion;
    Spare_magnifier_mode=page->Main_magnifier_mode;
    Spare_magnifier_factor=page->Main_magnifier_factor;
    Spare_magnifier_height=page->Main_magnifier_height;
    Spare_magnifier_width=page->Main_magnifier_width;
    Spare_magnifier_offset_X=page->Main_magnifier_offset_X;
    Spare_magnifier_offset_Y=page->Main_magnifier_offset_Y;
*/
  }
}

void Upload_infos_page_spare(T_Page * page)
{
  if (page!=NULL)
  {
    page->Image=Spare_screen;
    page->Width=Spare_image_width;
    page->Height=Spare_image_height;
    memcpy(page->Palette,Spare_palette,sizeof(T_Palette));
    strcpy(page->Comment,Spare_comment);
    strcpy(page->File_directory,Spare_file_directory);
    strcpy(page->Filename,Spare_filename);
    page->File_format=Spare_fileformat;
/*
    page->X_offset=Spare_offset_X;
    page->Y_offset=Spare_offset_Y;
    page->old_offset_x=Old_spare_offset_X;
    page->old_offset_y=Old_spare_offset_Y;
    page->Split=Spare_separator_position;
    page->X_zoom=Spare_X_zoom;
    page->Separator_proportion=Spare_separator_proportion;
    page->Main_magnifier_mode=Spare_magnifier_mode;
    page->Main_magnifier_factor=Spare_magnifier_factor;
    page->Main_magnifier_height=Spare_magnifier_height;
    page->Main_magnifier_width=Spare_magnifier_width;
    page->Main_magnifier_offset_X=Spare_magnifier_offset_X;
    page->Main_magnifier_offset_Y=Spare_magnifier_offset_Y;
*/
  }
}

void Download_infos_backup(T_List_of_pages * list)
{
  Screen_backup=list->Pages[1].Image;

  if (Config.FX_Feedback)
    FX_feedback_screen=list->Pages[0].Image;
  else
    FX_feedback_screen=list->Pages[1].Image;
}

void Free_a_page(T_Page * page)
{
  // On peut appeler cette fonction sur une page non allouée.

  if (page->Image!=NULL)
    free(page->Image);
  page->Image=NULL;
  page->Width=0;
  page->Height=0;
  // On ne se préoccupe pas de ce que deviens le reste des infos de l'image.
}

void Copy_S_page(T_Page * dest,T_Page * source)
{
  *dest=*source;
}

int Size_of_a_page(T_Page * page)
{
  return sizeof(T_Page)+(page->Width*page->Height)+8;
  // 8 = 4 + 4
  // (Toute zone allouée en mémoire est précédée d'un mot double indiquant sa
  // taille, or la taille d'un mot double est de 4 octets, et on utilise deux
  // allocations de mémoires: une pour la T_Page et une pour l'image)
}


  ///
  /// GESTION DES LISTES DE PAGES
  ///

void Init_list_of_pages(T_List_of_pages * list)
{
  // Important: appeler cette fonction sur toute nouvelle structure
  //            T_List_of_pages!

  list->List_size=0;
  list->Nb_pages_allocated=0;
  list->Pages=NULL;
}

int Allocate_list_of_pages(T_List_of_pages * list,int size)
{
  // Important: la T_List_of_pages ne doit pas déjà désigner une liste de
  //            pages allouée auquel cas celle-ci serait perdue.
  int index;

  /* Debug : if (list->Pages!=NULL) exit(666); */

  // On alloue la mémoire pour la liste
  list->Pages=(T_Page *)malloc(size*sizeof(T_Page));

  // On vérifie que l'allocation se soit bien passée
  if (list->Pages==NULL)
    return 0; // Echec
  else
  {
    // On initialise chacune des nouvelles pages
    for (index=0;index<size;index++)
      Init_page(list->Pages+index);
    list->List_size=size;
    list->Nb_pages_allocated=0;

    return 1; // Succès
  }
}

void Free_a_list_of_pages(T_List_of_pages * list)
{
  // On peut appeler cette fonction sur une liste de pages non allouée.

  // Important: cette fonction ne libère pas les pages de la liste. Il faut
  //            donc le faire préalablement si nécessaire.

  if (list->Pages!=NULL)
    free(list->Pages);
  list->Pages=NULL;
  list->List_size=0;
  list->Nb_pages_allocated=0;
}

int Size_of_a_list_of_pages(T_List_of_pages * list)
{
  int result=0;
  int index;

  for (index=0;index<list->Nb_pages_allocated;index++)
    result+=Size_of_a_page(list->Pages+index);

  return result+sizeof(T_List_of_pages)+4;

  // C.F. la remarque à propos de Size_of_a_page pour la valeur 4.
}

void Backward_in_list_of_pages(T_List_of_pages * list)
{
  // Cette fonction fait l'équivalent d'un "Undo" dans la liste de pages.
  // Elle effectue une sorte de ROL (Rotation Left) sur la liste:
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º0º1³2³3³4³5³6³7³8³9³Aº  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |  0=page courante
  //  ³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³   |_ A=page la plus ancienne
  //  v v v v v v v v v v v   |  1=Dernière page (1er backup)
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º1º2³3³4³5³6³7³8³9³A³0º  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |

  // Pour simuler un véritable Undo, l'appelant doit mettre la structure
  // de page courante à jour avant l'appel, puis en réextraire les infos en
  // sortie, ainsi que celles relatives à la plus récente page d'undo (1ère
  // page de la liste).

  int index;
  T_Page * temp_page;

  if (list->Nb_pages_allocated>1)
  {
    // On crée la page tempo
    temp_page=(T_Page *)malloc(sizeof(T_Page));
    Init_page(temp_page);

    // On copie la 1ère page (page 0) dans la page temporaire
    Copy_S_page(temp_page,list->Pages);

    // On copie toutes les pages 1-A à leur gauche
    for (index=1;index<list->Nb_pages_allocated;index++)
      Copy_S_page(list->Pages+index-1,list->Pages+index);

    // On copie la page 0 (dont la sauvegarde a été effectuée dans la page
    // temporaire) en dernière position
    Copy_S_page(list->Pages+list->Nb_pages_allocated-1,temp_page);

    // On détruit la page tempo
    free(temp_page);
  }
}

void Advance_in_list_of_pages(T_List_of_pages * list)
{
  // Cette fonction fait l'équivalent d'un "Redo" dans la liste de pages.
  // Elle effectue une sorte de ROR (Rotation Right) sur la liste:
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // º0º1³2³3³4³5³6³7³8³9³Aº  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |  0=page courante
  //  ³ ³ ³ ³ ³ ³ ³ ³ ³ ³ ³   |_ A=page la plus ancienne
  //  v v v v v v v v v v v   |  1=Dernière page (1er backup)
  // ÉÍËÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍÑÍ»  |
  // ºAº0³1³2³3³4³5³6³7³8³9º  |
  // ÈÍÊÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍÏÍ¼  |

  // Pour simuler un véritable Redo, l'appelant doit mettre la structure
  // de page courante à jour avant l'appel, puis en réextraire les infos en
  // sortie, ainsi que celles relatives à la plus récente page d'undo (1ère
  // page de la liste).

  int index;
  T_Page * temp_page;

  if (list->Nb_pages_allocated>1)
  {
    // On crée la page tempo
    temp_page=(T_Page *)malloc(sizeof(T_Page));
    Init_page(temp_page);

    // On copie la dernière page dans la page temporaire
    Copy_S_page(temp_page,list->Pages+list->Nb_pages_allocated-1);

    // On copie toutes les pages 0-9 à leur droite
    for (index=list->Nb_pages_allocated-1;index>0;index--)
      Copy_S_page(list->Pages+index,list->Pages+index-1);

    // On copie la page plus ancienne page (la "A", dont la sauvegarde a été
    // effectuée dans la page temporaire) en 1ère position
    Copy_S_page(list->Pages,temp_page);

    // On détruit la page tempo
    free(temp_page);
  }
}

int New_page_is_possible(
        T_Page           * new_page,
        T_List_of_pages * current_list,
        T_List_of_pages * secondary_list
)
{
  unsigned long mem_available_now;
  unsigned long current_list_size;
  unsigned long spare_list_size;
  unsigned long current_page_size;
  unsigned long spare_page_size;
  unsigned long new_page_size;

  mem_available_now = Memory_free()
        - MINIMAL_MEMORY_TO_RESERVE;
  current_list_size =Size_of_a_list_of_pages(current_list);
  spare_list_size=Size_of_a_list_of_pages(secondary_list);
  current_page_size  =Size_of_a_page(current_list->Pages);
  spare_page_size =Size_of_a_page(secondary_list->Pages);
  new_page_size  =Size_of_a_page(new_page);

  // Il faut pouvoir loger la nouvelle page et son backup dans la page
  // courante, en conservant au pire la 1ère page de brouillon.
  if ( (mem_available_now + current_list_size +
       spare_list_size - spare_page_size)
       < (2*new_page_size) )
    return 0;

  // Il faut pouvoir loger le brouillon et son backup dans la page de
  // brouillon, en conservant au pire un exemplaire de la nouvelle page dans
  // la page courante. (pour permettre à l'utilisateur de travailler sur son
  // brouillon)
  if ((mem_available_now+current_list_size+
       spare_list_size-new_page_size)<(2*spare_page_size))
    return 0;

  return 1;
}

void Free_last_page_of_list(T_List_of_pages * list)
{
  if (list!=NULL)
  {
    if (list->Nb_pages_allocated>0)
    {
      list->Nb_pages_allocated--;
      Free_a_page(list->Pages+list->Nb_pages_allocated);
    }
  }
}

void Create_new_page(T_Page * new_page,T_List_of_pages * current_list,T_List_of_pages * secondary_list)
{

//   Cette fonction crée une nouvelle page dont les attributs correspondent à
// ceux de new_page (width,height,...) (le champ Image est invalide
// à l'appel, c'est la fonction qui le met à jour), et l'enfile dans
// current_list.
//   Il est impératif que la création de cette page soit possible,
// éventuellement au détriment des backups de la page de brouillon
// (secondary_list). Afin de s'en assurer, il faut vérifier cette
// possibilité à l'aide de
// New_page_is_possible(new_page,current_list,secondary_list) avant
// l'appel à cette fonction.
//   De plus, il faut qu'il y ait au moins une page dans chacune des listes.

  int                need_to_free;
  T_List_of_pages * list_to_reduce=NULL;
  T_Page *           page_to_delete;
  int                index;

  // On regarde s'il faut libérer des pages:
  need_to_free=
    // C'est le cas si la current_list est pleine
  (  (current_list->List_size==current_list->Nb_pages_allocated)
    // ou qu'il ne reste plus assez de place pour allouer la new_page
  || ( (Memory_free()-MINIMAL_MEMORY_TO_RESERVE)<
       (unsigned long)(new_page->Height*new_page->Width) )  );

  if (!need_to_free)
  {
    // On a assez de place pour allouer une page, et de plus la current_list
    // n'est pas pleine. On n'a donc aucune page à supprimer. On peut en
    // allouer une directement.
    new_page->Image=(byte *)malloc(new_page->Height*new_page->Width);
  }
  else
  {
    // On manque de mémoire ou la current_list est pleine. Dans tous les
    // cas, il faut libérer une page... qui peut-être pourra re-servir.

    // Tant qu'il faut libérer
    while (need_to_free)
    {
      // On cherche sur quelle liste on va virer une page

      // S'il reste des pages à libérer dans la current_list
      if (current_list->Nb_pages_allocated>1)
        // Alors on va détruire la dernière page allouée de la current_list
        list_to_reduce=current_list;
      else
      {
        if (secondary_list->Nb_pages_allocated>1)
        {
          // Sinon on va détruire la dernière page allouée de la
          // secondary_list
          list_to_reduce=secondary_list;
        }
        else
        {
          // Bon, alors là, on vient de vider toutes les pages et on a toujours pas asez de mémoire... C'est donc qu'un vilain programmeur a oublié de vérifier avec Noiuvelle_page_possible avant de venir ici.
          // On sort méchament du programme sans sauvegarde ni rien. De toutes façons, ça ne devrait jamais se produire...
          Error(ERROR_SORRY_SORRY_SORRY);
        }
      }

      // Puis on détermine la page que l'on va supprimer (c'est la dernière de
      // la liste)
      page_to_delete=list_to_reduce->Pages+(list_to_reduce->Nb_pages_allocated)-1;

      // On regarde si on peut recycler directement la page (cas où elle
      // aurait la même surface que la new_page)
      if ((page_to_delete->Height*page_to_delete->Width)==
          (new_page->Height*new_page->Width))
      {
        // Alors
        // On récupère le bitmap de la page à supprimer (évite de faire des
        // allocations/désallocations fastidieuses et inutiles)
        new_page->Image=page_to_delete->Image;

        // On fait semblant que la dernière page allouée ne l'est pas
        list_to_reduce->Nb_pages_allocated--;

        // On n'a plus besoin de libérer de la mémoire puisqu'on a refilé à
        // new_page un bitmap valide
        need_to_free=0;
      }
      else
      {
        // Sinon

        // Détruire la dernière page allouée dans la Liste_à_raboter
        Free_last_page_of_list(list_to_reduce);

        // On regarde s'il faut continuer à libérer de la place
        need_to_free=(Memory_free()-MINIMAL_MEMORY_TO_RESERVE)
                       <(unsigned long)(new_page->Height*new_page->Width);

        // S'il ne faut pas, c'est qu'on peut allouer un bitmap
        // pour la new_page
        if (!need_to_free)
          new_page->Image=(byte *)malloc(new_page->Height*new_page->Width);
      }
    }
  }

  // D'après l'hypothèse de départ, la boucle ci-dessus doit s'arrêter car
  // on a assez de mémoire pour allouer la nouvelle page.
  // Désormais new_page contient un pointeur sur une zone bitmap valide.

  // Décaler la current_list d'un cran vers le passé.
  for (index=current_list->List_size-1;index>0;index--)
    Copy_S_page(current_list->Pages+index,current_list->Pages+index-1);

  // Recopier la new_page en 1ère position de la current_list
  Copy_S_page(current_list->Pages,new_page);
  current_list->Nb_pages_allocated++;
}

void Change_page_number_of_list(T_List_of_pages * list,int number)
{
  int index;
  T_Page * new_pages;

  // Si la liste a déjà la taille demandée
  if (list->List_size==number)
    // Alors il n'y a rien à faire
    return;

  // Si la liste contient plus de pages que souhaité
  if (list->List_size>number)
    // Alors pour chaque page en excés
    for (index=number;index<list->List_size;index++)
      // On libère la page
      Free_a_page(list->Pages+index);

  // On fait une nouvelle liste de pages:
  new_pages=(T_Page *)malloc(number*sizeof(T_Page));
  for (index=0;index<number;index++)
    Init_page(new_pages+index);

  // On recopie les pages à conserver de l'ancienne liste
  for (index=0;index<Min(number,list->List_size);index++)
    Copy_S_page(new_pages+index,list->Pages+index);

  // On libère l'ancienne liste
  free(list->Pages);

  // On met à jour les champs de la nouvelle liste
  list->Pages=new_pages;
  list->List_size=number;
  if (list->Nb_pages_allocated>number)
    list->Nb_pages_allocated=number;
}

void Free_page_of_a_list(T_List_of_pages * list)
{
  // On ne peut pas détruire la page courante de la liste si après
  // destruction il ne reste pas encore au moins une page.
  if (list->Nb_pages_allocated>1)
  {
    // On fait faire un undo à la liste, comme ça, la nouvelle page courante
    // est la page précédente
    Backward_in_list_of_pages(Main_backups);

    // Puis on détruit la dernière page, qui est l'ancienne page courante
    Free_last_page_of_list(list);
  }
}


  ///
  /// GESTION DES BACKUPS
  ///

int Init_all_backup_lists(int size,int width,int height)
{
  // size correspond au nombre de pages que l'on souhaite dans chaque liste
  // (1 pour la page courante, puis 1 pour chaque backup, soit 2 au minimum).
  // width et height correspondent à la dimension des images de départ.

  T_Page * page;
  int return_code=0;

  if (Allocate_list_of_pages(Main_backups,size) &&
      Allocate_list_of_pages(Spare_backups,size))
  {
    // On a réussi à allouer deux listes de pages dont la taille correspond à
    // celle demandée par l'utilisateur.

    // On crée un descripteur de page correspondant à la page principale
    page=(T_Page *)malloc(sizeof(T_Page));
    Init_page(page);
    Upload_infos_page_main(page);
    // On y met les infos sur la dimension de démarrage
    page->Width=width;
    page->Height=height;

    // On regarde si on peut ajouter cette page
    if (New_page_is_possible(page,Main_backups,Spare_backups))
    {
      // On peut, donc on va la créer
      Create_new_page(page,Main_backups,Spare_backups);
      Download_infos_page_main(page);
      Download_infos_backup(Main_backups);

      // Maintenant, on regarde si on a le droit de créer la même page dans
      // la page de brouillon.
      if (New_page_is_possible(page,Spare_backups,Main_backups))
      {
        // On peut donc on le fait
        Create_new_page(page,Spare_backups,Main_backups);
        Download_infos_page_spare(page);

        // Et on efface les 2 images en les remplacant de "0"
        memset(Main_screen,0,Main_image_width*Main_image_height);
        memset(Spare_screen,0,Spare_image_width*Spare_image_height);

        return_code=1;
      }
      else
      {
        // Il n'est pas possible de démarrer le programme avec la page 
        // principale et la page de brouillon aux dimensions demandée par 
        // l'utilisateur. ==> On l'envoie ballader
        return_code=0;
      }
    }
    else
    {
      // On ne peut pas démarrer le programme avec ne serait-ce qu'une
      // page de la dimension souhaitée, donc on laisse tout tomber et on
      // le renvoie chier.
      free(page);
      return_code=0;
    }
  }
  else
  {
    // On n'a même pas réussi à créer les listes. Donc c'est même pas la 
    // peine de continuer : l'utilisateur ne pourra jamais rien faire, 
    // autant avorter le chargement du programme.
    return_code=0;
  }

  return return_code;
}

void Set_number_of_backups(int nb_backups)
{
  Change_page_number_of_list(Main_backups,nb_backups+1);
  Change_page_number_of_list(Spare_backups,nb_backups+1);

  // Le +1 vient du fait que dans chaque liste, en 1ère position on retrouve
  // les infos de la page courante sur le brouillon et la page principale.
  // (nb_backups = Nombre de backups, sans compter les pages courantes)
}

int Backup_with_new_dimensions(int upload,int width,int height)
{
  // Retourne 1 si une nouvelle page est disponible (alors pleine de 0) et
  // 0 sinon.

  T_Page * new_page;
  int return_code=0;

  if (upload)
    // On remet à jour l'état des infos de la page courante (pour pouvoir les
    // retrouver plus tard)
    Upload_infos_page_main(Main_backups->Pages);

  // On crée un descripteur pour la nouvelle page courante
  new_page=(T_Page *)malloc(sizeof(T_Page));
  Init_page(new_page);

  Upload_infos_page_main(new_page);
  new_page->Width=width;
  new_page->Height=height;
  if (New_page_is_possible(new_page,Main_backups,Spare_backups))
  {
    Create_new_page(new_page,Main_backups,Spare_backups);
    Download_infos_page_main(new_page);
    Download_infos_backup(Main_backups);
    // On nettoie la nouvelle image:
    memset(Main_screen,0,Main_image_width*Main_image_height);
    return_code=1;
  }

  // On détruit le descripteur de la page courante
  free(new_page);

  return return_code;
}

int Backup_and_resize_the_spare(int width,int height)
{
  // Retourne 1 si la page de dimension souhaitee est disponible en brouillon
  // et 0 sinon.

  T_Page * new_page;
  int return_code=0;

  // On remet à jour l'état des infos de la page de brouillon (pour pouvoir
  // les retrouver plus tard)
  Upload_infos_page_spare(Spare_backups->Pages);

  // On crée un descripteur pour la nouvelle page de brouillon
  new_page=(T_Page *)malloc(sizeof(T_Page));
  Init_page(new_page);

  Upload_infos_page_spare(new_page);
  new_page->Width=width;
  new_page->Height=height;
  if (New_page_is_possible(new_page,Spare_backups,Main_backups))
  {
    Create_new_page(new_page,Spare_backups,Main_backups);
    Download_infos_page_spare(new_page);
    return_code=1;
  }

  // On détruit le descripteur de la page courante
  free(new_page);

  return return_code;
}

void Backup(void)
// Sauve la page courante comme première page de backup et crée une nouvelle page
// pur continuer à dessiner. Utilisé par exemple pour le fill
{
  #if defined(__macosx__) || defined(__FreeBSD__)
    T_Page new_page;
  #else
    T_Page *new_page;
  #endif

  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);

  // On crée un descripteur pour la nouvelle page courante
#if defined(__macosx__) || defined(__FreeBSD__)
  Init_page(&new_page);

  // Enrichissement de l'historique
  Copy_S_page(&new_page,Main_backups->Pages);
  Create_new_page(&new_page,Main_backups,Spare_backups);
  Download_infos_page_main(&new_page);
#else
  new_page=(T_Page *)malloc(sizeof(T_Page));
  Init_page(new_page);

  // Enrichissement de l'historique
  Copy_S_page(new_page,Main_backups->Pages);
  Create_new_page(new_page,Main_backups,Spare_backups);
  Download_infos_page_main(new_page);
#endif

  Download_infos_backup(Main_backups);

  // On copie l'image du backup vers la page courante:
  memcpy(Main_screen,Screen_backup,Main_image_width*Main_image_height);

  // On détruit le descripteur de la page courante
#if !(defined(__macosx__) || defined(__FreeBSD__))
  free(new_page);
#endif

  // On allume l'indicateur de modification de l'image
  Main_image_is_modified=1;
}

void Undo(void)
{
  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);
  // On fait faire un undo à la liste des backups de la page principale
  Backward_in_list_of_pages(Main_backups);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main_backups->Pages);
  // Et celles du backup
  Download_infos_backup(Main_backups);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Undo. Donc ça ne devrait pas
  //       poser de problèmes.
}

void Redo(void)
{
  // On remet à jour l'état des infos de la page courante (pour pouvoir les
  // retrouver plus tard)
  Upload_infos_page_main(Main_backups->Pages);
  // On fait faire un redo à la liste des backups de la page principale
  Advance_in_list_of_pages(Main_backups);

  // On extrait ensuite les infos sur la nouvelle page courante
  Download_infos_page_main(Main_backups->Pages);
  // Et celles du backup
  Download_infos_backup(Main_backups);
  // Note: le backup n'a pas obligatoirement les mêmes dimensions ni la même
  //       palette que la page courante. Mais en temps normal, le backup
  //       n'est pas utilisé à la suite d'un Redo. Donc ça ne devrait pas
  //       poser de problèmes.
}

void Free_current_page(void)
{
  // On détruit la page courante de la liste principale
  Free_page_of_a_list(Main_backups);
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

    /* SECTION GROS CACA PROUT PROUT */
    // Auparavant on ruse en mettant déjà à jour les dimensions de la
    // nouvelle page courante. Si on ne le fait pas, le "Download" va détecter
    // un changement de dimensions et va bêtement sortir du mode loupe, alors
    // que lors d'un changement de page, on veut bien conserver l'état du mode
    // loupe du brouillon.
    Main_image_width=Main_backups->Pages->Width;
    Main_image_height=Main_backups->Pages->Height;

  Download_infos_page_main(Main_backups->Pages);
  Download_infos_page_spare(Spare_backups->Pages);
  Download_infos_backup(Main_backups);
}


int Can_borrow_memory_from_page(int size)
{
  int mem_available_now;
  int current_list_size;
  int spare_list_size;
  int current_page_size;
  int spare_page_size;

  mem_available_now=Memory_free()-MINIMAL_MEMORY_TO_RESERVE;
  current_list_size =Size_of_a_list_of_pages(Main_backups);
  spare_list_size=Size_of_a_list_of_pages(Spare_backups);
  current_page_size  =Size_of_a_page(Main_backups->Pages);
  spare_page_size =Size_of_a_page(Spare_backups->Pages);

  // Il faut pouvoir loger la zone mémoire ainsi qu'un exemplaire de la page
  // courante, en conservant au pire la 1ère page de brouillon.
  if ((mem_available_now
      +current_list_size
      +spare_list_size
      -current_page_size
      -spare_page_size)<size)
    return 0;

  return 1;
}

void * Borrow_memory_from_page(int size)
{
  int                need_to_free;
  T_List_of_pages * list_to_reduce;
  T_Page *           page_to_delete;
  //int                index;

  if (Can_borrow_memory_from_page(size))
  {
    // On regarde s'il faut libérer des pages:
    need_to_free=
      (Memory_free()-MINIMAL_MEMORY_TO_RESERVE)<(unsigned long)size;

    if (!need_to_free)
    {
      // On a assez de place pour allouer une page. On n'a donc aucune page
      // à supprimer. On peut allouer de la mémoire directement.
      return malloc(size);
    }
    else
    {
      // On manque de mémoire. Il faut libérer une page...

      // Tant qu'il faut libérer
      while (need_to_free)
      {
        // On cherche sur quelle liste on va virer une page

        // S'il reste des pages à libérer dans la liste des brouillons
        if (Spare_backups->Nb_pages_allocated>1)
          // Alors on va détruire la dernière page allouée de la liste des
          // brouillons
          list_to_reduce=Spare_backups;
        else
        {
          if (Main_backups->Nb_pages_allocated>1)
          {
            // Sinon on va détruire la dernière page allouée de la
            // liste principale
            list_to_reduce=Main_backups;
          }
          else
          {
            // Dans cette branche, il était prévu qu'on obtienne la mémoire
            // nécessaire mais on n'arrive pas à la trouver. On indique donc
            // qu'elle n'est pas disponible, et on aura perdu des backups
            // pour rien
            return 0;
          }
        }

        // Puis on détermine la page que l'on va supprimer (c'est la dernière
        // de la liste)
        page_to_delete=list_to_reduce->Pages+(list_to_reduce->Nb_pages_allocated)-1;

        // Détruire la dernière page allouée dans la Liste_à_raboter
        Free_last_page_of_list(list_to_reduce);

        // On regarde s'il faut continuer à libérer de la place
        need_to_free=
          (Memory_free()-MINIMAL_MEMORY_TO_RESERVE)<(unsigned long)size;

        // S'il ne faut pas, c'est qu'on peut allouer un bitmap
        // pour la new_page
        if (!need_to_free)
          return malloc(size);
      }
    }
  }
  else
  {
    // Il n'y a pas assez de place pour allouer la mémoire temporaire dans
    // la mémoire réservée aux pages.
    return 0;
  }

  // Pour que le compilateur ne dise pas qu'il manque une valeur de sortie:
  return 0;
}
