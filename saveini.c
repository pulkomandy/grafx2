#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "const.h"
#include "global.h"
#include "readini.h"
#include "files.h"
#include "erreurs.h"

int Sauver_INI_Reach_group(FILE * Old_file,FILE * New_file,char * Buffer,char * Group)
{
  int    Arret;
  char * Group_upper;
  char * Buffer_upper;

  // On alloue les zones de mémoire:
  Group_upper =(char *)malloc(1024);
  Buffer_upper=(char *)malloc(1024);

  // On commence par se faire une version majuscule du groupe à rechercher:
  strcpy(Group_upper,Group);
  Charger_INI_Clear_string(Group_upper);

  Arret=0;
  do
  {
    // On lit une ligne dans le fichier:
    if (fgets(Buffer,1024,Old_file)==0)
    {
      free(Buffer_upper);
      free(Group_upper);
      return ERREUR_INI_CORROMPU;
    }

    // On s'en fait une version en majuscule:
    strcpy(Buffer_upper,Buffer);
    Charger_INI_Clear_string(Buffer_upper);

    // On compare la chaîne avec le groupe recherché:
    Arret=Charger_INI_Seek_pattern(Buffer_upper,Group_upper);
    if (fprintf(New_file,"%s",Buffer)<0)
    {
      free(Buffer_upper);
      free(Group_upper);
      return ERREUR_SAUVEGARDE_INI;
    }
  }
  while (Arret==0);

  free(Buffer_upper);
  free(Group_upper);

  return 0;
}



int Sauver_INI_char_in_value_alphabet(char Char)
{
  if (
       (                    // Chiffre
         (Char>='0') &&
         (Char<='9')
       ) ||
       (                    // Lettre majuscule
         (Char>='A') &&
         (Char<='Z')
       ) ||
       (                    // Lettre minuscule
         (Char>='a') &&
         (Char<='z')
       ) ||
       (Char == '$')        // Symbole d'hexadécimal
     )
    return 1;
  else
    return 0;
}



void Sauver_INI_Set_value(char * Destination,char * Source,int Nb_values_to_set,int * Values,int Litteral)
{
  int Indice_destination;
  int Indice_source;
  int Indice_valeur;

  // On commence par recopier tout jusqu'au symbole '=':
  for (Indice_source=0;Source[Indice_source]!='=';Indice_source++)
    Destination[Indice_source]=Source[Indice_source];

  // Puis on recopie le symbole '=':
  Destination[Indice_source]=Source[Indice_source];
  Indice_source++;

  // Puis on recopie tous les espaces qui suivent:
  for (;Source[Indice_source]==' ';Indice_source++)
    Destination[Indice_source]=Source[Indice_source];

  // Pour l'instant, la source et la destination en sont au même point:
  Indice_destination=Indice_source;

  // Puis pour chaque valeur à recopier:
  for (Indice_valeur=0;Indice_valeur<Nb_values_to_set;Indice_valeur++)
  {
    // Dans la destination, on écrit la valeur:
    if (Litteral)
    {
      // La valeur doit être écrite sous la forme Yes/No

      if (Values[Indice_valeur])
      {
        // La valeur <=> Yes  

        memcpy(Destination+Indice_destination,"yes",3);
        Indice_destination+=3;
      }
      else
      {
        // La valeur <=> No

        memcpy(Destination+Indice_destination,"no",2);
        Indice_destination+=2;
      }
    }
    else
    {
      // La valeur doit être écrite sous forme numérique

      if (Source[Indice_source]=='$')
      {
        // On va écrire la valeur sous forme hexadécimale:

        // On commence par inscrire le symbole '$':
        Destination[Indice_destination]='$';

        // Puis on y concatŠne la valeur:
        sprintf(Destination+Indice_destination+1,"%x",Values[Indice_valeur]);
        Indice_destination+=strlen(Destination+Indice_destination);
      }
      else
      {
        // On va écrire la valeur sous forme décimale:

        sprintf(Destination+Indice_destination,"%d",Values[Indice_valeur]);
        Indice_destination+=strlen(Destination+Indice_destination);
      }
    }

    // Dans la source, on saute la valeur:
    for (;Sauver_INI_char_in_value_alphabet(Source[Indice_source]) && (Source[Indice_source]!='\0');Indice_source++);

    if (Indice_valeur!=(Nb_values_to_set-1))
    {
      // Il reste d'autres valeurs à écrire

      // On recopie tous les caractŠres de la source jusqu'au suivant qui
      // désigne une valeur:
      for (;(!Sauver_INI_char_in_value_alphabet(Source[Indice_source])) && (Source[Indice_source]!='\0');Indice_source++,Indice_destination++)
        Destination[Indice_destination]=Source[Indice_source];
    }
    else
    {
      // C'est la derniŠre valeur à initialiser

      // On recopie toute la fin de la ligne:
      for (;Source[Indice_source]!='\0';Indice_source++,Indice_destination++)
        Destination[Indice_destination]=Source[Indice_source];

      // Et on n'oublie pas d'y mettre l''\0':
      Destination[Indice_destination]='\0';
    }
  }
}



int Sauver_INI_Set_values(FILE * Old_file,FILE * New_file,char * Buffer,char * Option,int Nb_values_to_set,int * Values,int Litteral)
{
  int    Arret;
  char * Option_upper;
  char * Buffer_upper;
  char * Buffer_resultat;
  //int    Indice_buffer;

  // On alloue les zones de mémoire:
  Option_upper=(char *)malloc(1024);
  Buffer_upper=(char *)malloc(1024);
  Buffer_resultat=(char *)malloc(1024);

  // On commence par se faire une version majuscule de l'option à rechercher:
  strcpy(Option_upper,Option);
  Charger_INI_Clear_string(Option_upper);

  Arret=0;
  do
  {
    // On lit une ligne dans le fichier:
    if (fgets(Buffer,1024,Old_file)==0)
    {
      free(Buffer_resultat);
      free(Buffer_upper);
      free(Option_upper);
      return ERREUR_INI_CORROMPU;
    }

    // On s'en fait une version en majuscule:
    strcpy(Buffer_upper,Buffer);
    Charger_INI_Clear_string(Buffer_upper);

    // On compare la chaîne avec l'option recherchée:
    Arret=Charger_INI_Seek_pattern(Buffer_upper,Option_upper);

    if (Arret)
    {
      // On l'a trouvée:

      Sauver_INI_Set_value(Buffer_resultat,Buffer,Nb_values_to_set,Values,Litteral);
      if (fprintf(New_file,"%s",Buffer_resultat)<0)
      {
        free(Buffer_resultat);
        free(Buffer_upper);
        free(Option_upper);
        return ERREUR_SAUVEGARDE_INI;
      }
    }
    else
    {
      // On l'a pas trouvée:

      if (fprintf(New_file,"%s",Buffer)<0)
      {
        free(Buffer_resultat);
        free(Buffer_upper);
        free(Option_upper);
        return ERREUR_SAUVEGARDE_INI;
      }
    }
  }
  while (Arret==0);

  free(Buffer_resultat);
  free(Buffer_upper);
  free(Option_upper);

  return 0;
}



void Sauver_INI_Flush(FILE * Old_file,FILE * New_file,char * Buffer)
{
  while (fgets(Buffer,1024,Old_file)!=0)
    fprintf(New_file,"%s",Buffer);
}



int Sauver_INI(struct S_Config * Conf)
{
  FILE * Ancien_fichier;
  FILE * Nouveau_fichier;
  char * Buffer;
  int    Valeurs[3];
  char   Nom_du_fichier[256];
  char   Nom_du_fichier_temporaire[256];
  int    Retour;
  char   Nom_du_fichier_DAT[256];
  int    Ini_existe;

  // On alloue les zones de mémoire:
  Buffer=(char *)malloc(1024);
  
  // On calcule les noms des fichiers qu'on manipule:
  strcpy(Nom_du_fichier,Repertoire_du_programme);
  strcat(Nom_du_fichier,"gfx2.ini");

  // On vérifie si le fichier INI existe
  if ((Ini_existe = Fichier_existe(Nom_du_fichier)))
  {
    strcpy(Nom_du_fichier_temporaire,Repertoire_du_programme);
    strcat(Nom_du_fichier_temporaire,"gfx2.$$$");
    
    // On renome l'ancienne version du fichier INI vers un fichier temporaire:
    if (rename(Nom_du_fichier,Nom_du_fichier_temporaire)!=0)
    {
      goto Erreur_ERREUR_SAUVEGARDE_INI;
    }
  }
  // On récupère un fichier INI "propre" dans GFX2.DAT
  strcpy(Nom_du_fichier_DAT,Repertoire_du_programme);
  strcat(Nom_du_fichier_DAT,"gfx2.dat");
  Ancien_fichier=fopen(Nom_du_fichier_DAT,"rb");
  if (Ancien_fichier==0)
  {
    fclose(Ancien_fichier);
    return ERREUR_DAT_ABSENT;
  }
  if (fseek(Ancien_fichier, DAT_DEBUT_INI_PAR_DEFAUT, SEEK_SET))
  {
    fclose(Ancien_fichier);
    return ERREUR_DAT_CORROMPU;
  }
  Nouveau_fichier=fopen(Nom_du_fichier,"wb");
  if (Nouveau_fichier==0)
  {
    free(Buffer);
    return ERREUR_SAUVEGARDE_INI;
  }
  if ((Retour=Sauver_INI_Reach_group(Ancien_fichier,Nouveau_fichier,Buffer,"[MOUSE]")))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Indice_Sensibilite_souris_X;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"X_sensitivity",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Indice_Sensibilite_souris_Y;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Y_sensitivity",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Mouse_Facteur_de_correction_X;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"X_correction_factor",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Mouse_Facteur_de_correction_Y;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Y_correction_factor",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=(Conf->Curseur)+1;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Cursor_aspect",1,Valeurs,0)))
    goto Erreur_Retour;


  if ((Retour=Sauver_INI_Reach_group(Ancien_fichier,Nouveau_fichier,Buffer,"[MENU]")))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Coul_menu_pref[2].R;
  Valeurs[1]=Conf->Coul_menu_pref[2].V;
  Valeurs[2]=Conf->Coul_menu_pref[2].B;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Light_color",3,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Coul_menu_pref[1].R;
  Valeurs[1]=Conf->Coul_menu_pref[1].V;
  Valeurs[2]=Conf->Coul_menu_pref[1].B;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Dark_color",3,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Ratio;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Menu_ratio",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=(Conf->Fonte)+1;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Font",1,Valeurs,0)))
    goto Erreur_Retour;


  if ((Retour=Sauver_INI_Reach_group(Ancien_fichier,Nouveau_fichier,Buffer,"[FILE_SELECTOR]")))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Lire_les_fichiers_caches?1:0;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Show_hidden_files",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Lire_les_repertoires_caches?1:0;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Show_hidden_directories",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Lire_les_repertoires_systemes?1:0;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Show_system_directories",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Chrono_delay;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Preview_delay",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Maximize_preview;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Maximize_preview",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Find_file_fast;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Find_file_fast",1,Valeurs,0)))
    goto Erreur_Retour;


  if ((Retour=Sauver_INI_Reach_group(Ancien_fichier,Nouveau_fichier,Buffer,"[LOADING]")))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Auto_set_res;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Auto_set_resolution",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Set_resolution_according_to;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Set_resolution_according_to",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Clear_palette;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Clear_palette",1,Valeurs,1)))
    goto Erreur_Retour;


  if ((Retour=Sauver_INI_Reach_group(Ancien_fichier,Nouveau_fichier,Buffer,"[MISCELLANEOUS]")))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Afficher_limites_image;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Draw_limits",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Adjust_brush_pick;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Adjust_brush_pick",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=2-Conf->Coords_rel;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Coordinates",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Backup;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Backup",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Nb_pages_Undo;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Undo_pages",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Valeur_tempo_jauge_gauche;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Gauges_scrolling_speed_Left",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Valeur_tempo_jauge_droite;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Gauges_scrolling_speed_Right",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Auto_save;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Auto_save",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Nb_max_de_vertex_par_polygon;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Vertices_per_polygon",1,Valeurs,0)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Fast_zoom;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Fast_zoom",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Couleurs_separees;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Separate_colors",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->FX_Feedback;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"FX_feedback",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Safety_colors;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Safety_colors",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Opening_message;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Opening_message",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Clear_with_stencil;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Clear_with_stencil",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Auto_discontinuous;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Auto_discontinuous",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Taille_ecran_dans_GIF;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Save_screen_size_in_GIF",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Auto_nb_used;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Auto_nb_colors_used",1,Valeurs,1)))
    goto Erreur_Retour;

  Valeurs[0]=Conf->Resolution_par_defaut;
  if ((Retour=Sauver_INI_Set_values (Ancien_fichier,Nouveau_fichier,Buffer,"Default_video_mode",1,Valeurs,0)))
    goto Erreur_Retour;

  Sauver_INI_Flush(Ancien_fichier,Nouveau_fichier,Buffer);

  fclose(Nouveau_fichier);
  fclose(Ancien_fichier);

  // On efface le fichier temporaire <=> Ancienne version du .INI
  if (Ini_existe)
    remove(Nom_du_fichier_temporaire);
  free(Buffer);
  return 0;

  // Gestion des erreurs:

  Erreur_Retour:

    fclose(Nouveau_fichier);
    fclose(Ancien_fichier);
    free(Buffer);
    return Retour;

  Erreur_ERREUR_SAUVEGARDE_INI:

    free(Buffer);
    return ERREUR_SAUVEGARDE_INI;
}
