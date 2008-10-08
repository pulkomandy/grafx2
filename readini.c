/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "const.h"
#include "global.h"
#include "graph.h"

void Charger_INI_Clear_string(char * String)
{
  int Indice;

  for (Indice=0;String[Indice]!='\0';)
  {
    if ((String[Indice]==' ') ||
        (String[Indice]=='\t'))
    {
      // Suppression d'un espace ou d'un tab:

      memmove(String+Indice,String+Indice+1,strlen(String+Indice));
    }
    else
    if ((String[Indice]==';') ||
        (String[Indice]=='#') ||
        (String[Indice]=='\r') ||
        (String[Indice]=='\n'))
    {
      // Rencontre d'un commentaire ou d'un saut de ligne:

      String[Indice]='\0';
    }
    else
    {
      // Passage en majuscule d'un caractère:

      String[Indice]=toupper((int)String[Indice]);
      Indice++;
    }
  }
}



int Charger_INI_Seek_pattern(char * Buffer,char * Pattern)
{
  int Indice_buffer;
  int Indice_pattern;

  // A partir de chaque lettre de la chaîne Buffer
  for (Indice_buffer=0;Buffer[Indice_buffer]!='\0';Indice_buffer++)
  {
    // On regarde si la chaîne Pattern est équivalente à la position courante
    // de la chaîne Buffer:
    for (Indice_pattern=0;(Pattern[Indice_pattern]!='\0') && (Buffer[Indice_buffer+Indice_pattern]==Pattern[Indice_pattern]);Indice_pattern++);

    // Si on a trouvé la chaîne Pattern dans la chaîne Buffer, on renvoie la
    // position à laquelle on l'a trouvée (+1 pour que si on la trouve au
    // début ça ne renvoie pas la même chose que si on ne l'avait pas
    // trouvée):
    if (Pattern[Indice_pattern]=='\0')
      return (Indice_buffer+1);
  }

  // Si on ne l'a pas trouvée, on renvoie 0:
  return 0;
}



int Charger_INI_Reach_group(FILE * File,char * Buffer,char * Group)
{
  int    Arret;
  char * Group_upper;
  char * Buffer_upper;

  // On alloue les zones de mémoire:
  Group_upper=(char *)malloc(1024);
  Buffer_upper=(char *)malloc(1024);

  // On commence par se faire une version majuscule du groupe à rechercher:
  strcpy(Group_upper,Group);
  Charger_INI_Clear_string(Group_upper);

  Arret=0;
  do
  {
    // On lit une ligne dans le fichier:
    if (fgets(Buffer,1024,File)==0)
    {
      free(Buffer_upper);
      free(Group_upper);
      return ERREUR_INI_CORROMPU;
    }

    Ligne_INI++;

    // On s'en fait une version en majuscule:
    strcpy(Buffer_upper,Buffer);
    Charger_INI_Clear_string(Buffer_upper);

    // On compare la chaîne avec le groupe recherché:
    Arret=Charger_INI_Seek_pattern(Buffer_upper,Group_upper);
  }
  while (!Arret);

  free(Buffer_upper);
  free(Group_upper);

  return 0;
}

int Charger_INI_Get_string(FILE * File,char * Buffer,char * Option,char * Retour)
{
  int    Arret;
  char * Option_upper;
  char * Buffer_upper;
  int    Indice_buffer;

  // On alloue les zones de mémoire:
  Option_upper=(char *)malloc(1024);
  Buffer_upper=(char *)malloc(1024);

  // On commence par se faire une version majuscule de l'option à rechercher:
  strcpy(Option_upper,Option);
  Charger_INI_Clear_string(Option_upper);

  Arret=0;
  do
  {
    // On lit une ligne dans le fichier:
    if (fgets(Buffer,1024,File)==0)
    {
      free(Buffer_upper);
      free(Option_upper);
      return ERREUR_INI_CORROMPU;
    }

    Ligne_INI++;

    // On s'en fait une version en majuscule:
    strcpy(Buffer_upper,Buffer);
    Charger_INI_Clear_string(Buffer_upper);

    // On compare la chaîne avec l'option recherchée:
    Arret=Charger_INI_Seek_pattern(Buffer_upper,Option_upper);

    // Si on l'a trouvée:
    if (Arret)
    {
      // On se positionne juste après la chaîne "="
      Indice_buffer=Charger_INI_Seek_pattern(Buffer_upper,"=");

      strcpy(Retour, Buffer_upper + Indice_buffer);
      // On coupe la chaine au premier espace ou ; (commentaire)
      for (Indice_buffer=0; Retour[Indice_buffer]!='\0' && Retour[Indice_buffer]!=' ' && Retour[Indice_buffer]!=';'; Indice_buffer++)
        ;
      Retour[Indice_buffer]='\0';
    }
  }
  while (!Arret);

  free(Buffer_upper);
  free(Option_upper);

  return 0;
}

int Charger_INI_Get_value(char * String,int * Index,int * Value)
{
  // On teste si la valeur actuelle est YES (ou Y):

  if (Charger_INI_Seek_pattern(String+(*Index),"YES,")==1)
  {
    (*Value)=1;
    (*Index)+=4;
    return 0;
  }
  else
  if (strcmp(String+(*Index),"YES")==0)
  {
    (*Value)=1;
    (*Index)+=3;
    return 0;
  }
  else
  if (Charger_INI_Seek_pattern(String+(*Index),"Y,")==1)
  {
    (*Value)=1;
    (*Index)+=2;
    return 0;
  }
  else
  if (strcmp(String+(*Index),"Y")==0)
  {
    (*Value)=1;
    (*Index)+=1;
    return 0;
  }
  else

  // On teste si la valeur actuelle est NO (ou N):

  if (Charger_INI_Seek_pattern(String+(*Index),"NO,")==1)
  {
    (*Value)=0;
    (*Index)+=3;
    return 0;
  }
  else
  if (strcmp(String+(*Index),"NO")==0)
  {
    (*Value)=0;
    (*Index)+=2;
    return 0;
  }
  else
  if (Charger_INI_Seek_pattern(String+(*Index),"N,")==1)
  {
    (*Value)=0;
    (*Index)+=2;
    return 0;
  }
  else
  if (strcmp(String+(*Index),"N")==0)
  {
    (*Value)=0;
    (*Index)+=1;
    return 0;
  }
  else
  if (String[*Index]=='$')
  {
    (*Value)=0;

    for (;;)
    {
      (*Index)++;

      if ((String[*Index]>='0') && (String[*Index]<='9'))
        (*Value)=((*Value)*16)+String[*Index]-'0';
      else
      if ((String[*Index]>='A') && (String[*Index]<='F'))
        (*Value)=((*Value)*16)+String[*Index]-'A'+10;
      else
      if (String[*Index]==',')
      {
        (*Index)++;
        return 0;
      }
      else
      if (String[*Index]=='\0')
        return 0;
      else
        return ERREUR_INI_CORROMPU;
    }
  }
  else
  if ((String[*Index]>='0') && (String[*Index]<='9'))
  {
    (*Value)=0;

    for (;;)
    {
      if ((String[*Index]>='0') && (String[*Index]<='9'))
        (*Value)=((*Value)*10)+String[*Index]-'0';
      else
      if (String[*Index]==',')
      {
        (*Index)++;
        return 0;
      }
      else
      if (String[*Index]=='\0')
        return 0;
      else
        return ERREUR_INI_CORROMPU;

      (*Index)++;
    }
  }
  else
    return ERREUR_INI_CORROMPU;
}



int Charger_INI_Get_values(FILE * File,char * Buffer,char * Option,int Nb_values_expected,int * Values)
{
  int    Arret;
  char * Option_upper;
  char * Buffer_upper;
  int    Indice_buffer;
  int    Nb_valeurs;

  // On alloue les zones de mémoire:
  Option_upper=(char *)malloc(1024);
  Buffer_upper=(char *)malloc(1024);

  // On commence par se faire une version majuscule de l'option à rechercher:
  strcpy(Option_upper,Option);
  Charger_INI_Clear_string(Option_upper);

  Arret=0;
  do
  {
    // On lit une ligne dans le fichier:
    if (fgets(Buffer,1024,File)==0)
    {
      free(Buffer_upper);
      free(Option_upper);
      return ERREUR_INI_CORROMPU;
    }

    Ligne_INI++;

    // On s'en fait une version en majuscule:
    strcpy(Buffer_upper,Buffer);
    Charger_INI_Clear_string(Buffer_upper);

    // On compare la chaîne avec l'option recherchée:
    Arret=Charger_INI_Seek_pattern(Buffer_upper,Option_upper);

    // Si on l'a trouvée:
    if (Arret)
    {
      Nb_valeurs=0;

      // On se positionne juste après la chaîne "="
      Indice_buffer=Charger_INI_Seek_pattern(Buffer_upper,"=");

      // Tant qu'on a pas atteint la fin de la ligne
      while (Buffer_upper[Indice_buffer]!='\0')
      {
        if (Charger_INI_Get_value(Buffer_upper,&Indice_buffer,Values+Nb_valeurs))
        {
          free(Buffer_upper);
          free(Option_upper);
          return ERREUR_INI_CORROMPU;
        }

        if ( ((++Nb_valeurs) == Nb_values_expected) &&
             (Buffer_upper[Indice_buffer]!='\0') )
        {
          free(Buffer_upper);
          free(Option_upper);
          return ERREUR_INI_CORROMPU;
        }
      }
      if (Nb_valeurs<Nb_values_expected)
      {
        free(Buffer_upper);
        free(Option_upper);
        return ERREUR_INI_CORROMPU;
      }
    }
  }
  while (!Arret);

  free(Buffer_upper);
  free(Option_upper);

  return 0;
}



int Charger_INI(struct S_Config * Conf)
{
  FILE * Fichier;
  char * Buffer;
  int    Valeurs[3];
  //int    Indice;
  char * Nom_du_fichier;
  int    Retour;
  char   Libelle_resolution[1024];

  Ligne_INI=0;

  // On alloue les zones de mémoire:
  Buffer=(char *)malloc(1024);
  Nom_du_fichier=(char *)malloc(256);

  // On calcule le nom du fichier qu'on manipule:
  strcpy(Nom_du_fichier,Repertoire_du_programme);
  strcat(Nom_du_fichier,"gfx2.ini");

  Fichier=fopen(Nom_du_fichier,"rb");
  if (Fichier==0)
  {
    // Si le fichier ini est absent on le relit depuis gfx2.dat
    strcpy(Nom_du_fichier,Repertoire_du_programme);
    strcat(Nom_du_fichier,"gfx2.dat");
    Fichier=fopen(Nom_du_fichier,"rb");
    if (Fichier == 0)
    {
      free(Nom_du_fichier);
      free(Buffer);
      return ERREUR_DAT_ABSENT;
    }
    if (fseek(Fichier, DAT_DEBUT_INI_PAR_DEFAUT ,SEEK_SET))
    {
      fclose(Fichier);
      free(Nom_du_fichier);
      free(Buffer);
      return ERREUR_DAT_CORROMPU;
    }
  }
  
  if ((Retour=Charger_INI_Reach_group(Fichier,Buffer,"[MOUSE]")))
    goto Erreur_Retour;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"X_sensitivity",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>255))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Indice_Sensibilite_souris_X=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Y_sensitivity",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>255))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Indice_Sensibilite_souris_Y=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"X_correction_factor",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>4))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Mouse_Facteur_de_correction_X=Mouse_Facteur_de_correction_X=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Y_correction_factor",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>4))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Mouse_Facteur_de_correction_Y=Mouse_Facteur_de_correction_Y=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Cursor_aspect",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>3))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Curseur=Valeurs[0]-1;


  if ((Retour=Charger_INI_Reach_group(Fichier,Buffer,"[MENU]")))
    goto Erreur_Retour;

  Conf->Coul_menu_pref[0].R=0;
  Conf->Coul_menu_pref[0].V=0;
  Conf->Coul_menu_pref[0].B=0;
  Conf->Coul_menu_pref[3].R=63;
  Conf->Coul_menu_pref[3].V=63;
  Conf->Coul_menu_pref[3].B=63;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Light_color",3,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  if ((Valeurs[1]<0) || (Valeurs[1]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  if ((Valeurs[2]<0) || (Valeurs[2]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Coul_menu_pref[2].R=Valeurs[0];
  Conf->Coul_menu_pref[2].V=Valeurs[1];
  Conf->Coul_menu_pref[2].B=Valeurs[2];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Dark_color",3,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  if ((Valeurs[1]<0) || (Valeurs[1]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  if ((Valeurs[2]<0) || (Valeurs[2]>63))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Coul_menu_pref[1].R=Valeurs[0];
  Conf->Coul_menu_pref[1].V=Valeurs[1];
  Conf->Coul_menu_pref[1].B=Valeurs[2];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Menu_ratio",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>2))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Ratio=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Font",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>2))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Fonte=Valeurs[0]-1;


  if ((Retour=Charger_INI_Reach_group(Fichier,Buffer,"[FILE_SELECTOR]")))
    goto Erreur_Retour;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Show_hidden_files",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Lire_les_fichiers_caches=Valeurs[0]?-1:0;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Show_hidden_directories",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Lire_les_repertoires_caches=Valeurs[0]?-1:0;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Show_system_directories",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Lire_les_repertoires_systemes=Valeurs[0]?-1:0;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Preview_delay",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>256))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Chrono_delay=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Maximize_preview",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Maximize_preview=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Find_file_fast",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>2))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Find_file_fast=Valeurs[0];


  if ((Retour=Charger_INI_Reach_group(Fichier,Buffer,"[LOADING]")))
    goto Erreur_Retour;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Auto_set_resolution",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Auto_set_res=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Set_resolution_according_to",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>2))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Set_resolution_according_to=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Clear_palette",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Clear_palette=Valeurs[0];


  if ((Retour=Charger_INI_Reach_group(Fichier,Buffer,"[MISCELLANEOUS]")))
    goto Erreur_Retour;

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Draw_limits",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Afficher_limites_image=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Adjust_brush_pick",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Adjust_brush_pick=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Coordinates",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>2))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Coords_rel=2-Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Backup",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Backup=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Undo_pages",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>99))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Nb_pages_Undo=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Gauges_scrolling_speed_Left",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>255))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Valeur_tempo_jauge_gauche=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Gauges_scrolling_speed_Right",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<1) || (Valeurs[0]>255))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Valeur_tempo_jauge_droite=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Auto_save",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Auto_save=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Vertices_per_polygon",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<2) || (Valeurs[0]>16384))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Nb_max_de_vertex_par_polygon=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Fast_zoom",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Fast_zoom=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Separate_colors",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Couleurs_separees=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"FX_feedback",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->FX_Feedback=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Safety_colors",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Safety_colors=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Opening_message",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Opening_message=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Clear_with_stencil",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Clear_with_stencil=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Auto_discontinuous",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Auto_discontinuous=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Save_screen_size_in_GIF",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Taille_ecran_dans_GIF=Valeurs[0];

  if ((Retour=Charger_INI_Get_values (Fichier,Buffer,"Auto_nb_colors_used",1,Valeurs)))
    goto Erreur_Retour;
  if ((Valeurs[0]<0) || (Valeurs[0]>1))
    goto Erreur_ERREUR_INI_CORROMPU;
  Conf->Auto_nb_used=Valeurs[0];

  // Optionnel, le mode video par défaut (à partir de beta 97.0%)
  Conf->Resolution_par_defaut=0;
  if (!Charger_INI_Get_string (Fichier,Buffer,"Default_video_mode",Libelle_resolution))
  {
    int Mode = Conversion_argument_mode(Libelle_resolution);
    if (Mode>=0)
      Conf->Resolution_par_defaut=Mode;
  }
  
  // Optionnel, les dimensions de la fenêtre (à partir de beta 97.0%)
  Mode_video[0].Largeur = 640;
  Mode_video[0].Hauteur = 480;
  if (!Charger_INI_Get_values (Fichier,Buffer,"Default_window_size",2,Valeurs))
  {
    if ((Valeurs[0]>=320))
      Mode_video[0].Largeur = Valeurs[0];
    if ((Valeurs[1]>=200))
      Mode_video[0].Hauteur = Valeurs[1];
  }
  
  fclose(Fichier);

  free(Nom_du_fichier);
  free(Buffer);
  return 0;

  // Gestion des erreurs:

  Erreur_Retour:

    fclose(Fichier);
    free(Nom_du_fichier);
    free(Buffer);
    return Retour;

  Erreur_ERREUR_INI_CORROMPU:

    fclose(Fichier);
    free(Nom_du_fichier);
    free(Buffer);
    return ERREUR_INI_CORROMPU;
}
