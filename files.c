/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
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
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
    #include <proto/dos.h>
    #include <dirent.h>
    #define isHidden(x) (0)
#elif defined(__WIN32__)
    #include <dirent.h>
    #include <windows.h>
    #define isHidden(x) (GetFileAttributesA((x)->d_name)&FILE_ATTRIBUTE_HIDDEN)
#else
    #include <dirent.h>
    #define isHidden(x) ((x)->d_name[0]=='.')
#endif

#include "const.h"
#include "struct.h"
#include "global.h"
#include "divers.h"
#include "erreurs.h"
#include "io.h"
#include "windows.h"
#include "loadsave.h"
#include "mountlist.h"
#include "moteur.h"

#define COULEUR_FICHIER_NORMAL    CM_Clair // Couleur du texte pour une ligne de fichier non sélectionné
#define COULEUR_REPERTOIRE_NORMAL CM_Fonce // Couleur du texte pour une ligne de répertoire non sélectionné
#define COULEUR_FOND_NORMAL       CM_Noir  // Couleur du fond  pour une ligne non sélectionnée
#define COULEUR_FICHIER_SELECT    CM_Blanc // Couleur du texte pour une ligne de fichier    sélectionnée
#define COULEUR_REPERTOIRE_SELECT CM_Clair // Couleur du texte pour une ligne de repértoire sélectionnée
#define COULEUR_FOND_SELECT       CM_Fonce // Couleur du fond  pour une ligne sélectionnée

#define FILENAMESPACE 13

int Determiner_repertoire_courant(void)
// Modifie Principal_Repertoire_courant en y mettant sa nouvelle valeur (avec le nom du
// disque)
//
// Renvoie 1 s'il y a eu une erreur d'accès
{
  return (getcwd(Principal_Repertoire_courant,256)==NULL);
}

// Conventions:
//
// * Le fileselect modifie le répertoire courant. Ceci permet de n'avoir
//   qu'un findfirst dans le répertoire courant à faire:


// -- Déstruction de la liste chaînée ---------------------------------------
void Detruire_liste_du_fileselect(void)
//  Cette procédure détruit la chaine des fichiers. Elle doit être appelée
// avant de rappeler la fonction Lire_liste_des_fichiers, ainsi qu'en fin de
// programme.
{
  // Pointeur temporaire de destruction
  Element_de_liste_de_fileselect * Element_temporaire;

  while (Liste_du_fileselect!=NULL)
  {
    // On mémorise l'adresse du premier élément de la liste
    Element_temporaire =Liste_du_fileselect;
    // On fait avancer la tête de la liste
    Liste_du_fileselect=Liste_du_fileselect->Suivant;
    // Et on efface l'ancien premier élément de la liste
    free(Element_temporaire);
  }
}


// -- Formatage graphique des noms de fichier / répertoire ------------------
char * Nom_formate(char * fname, int Type)
{
  static char Resultat[13];
  int         c;
  int         Autre_curseur;
  int         Pos_DernierPoint;

  if (strcmp(fname,PARENT_DIR)==0)
  {
    strcpy(Resultat,"<-PARENT DIR");
  }
  else if (fname[0]=='.' || Type==2)
  {
    // Fichiers ".quelquechose" ou lecteurs: Calé à gauche sur 12 caractères maximum.
    strcpy(Resultat,"            ");
    for (c=0;fname[c]!='\0' && c < 12;c++)
      Resultat[c]=fname[c];
    // Un caractère spécial pour indiquer que l'affichage est tronqué
    if (c >= 12)
      Resultat[11]=CARACTERE_SUSPENSION;
  }
  else
  {
    strcpy(Resultat,"        .   ");
    // On commence par recopier la partie précédent le point:
    for (c=0;( (fname[c]!='.') && (fname[c]!='\0') );c++)
    {
      if (c < 8)
        Resultat[c]=fname[c];
    }
    // Un caractère spécial pour indiquer que l'affichage est tronqué
    if (c > 8)
      Resultat[7]=CARACTERE_SUSPENSION;
    // On recherche le dernier point dans le reste du nom
    for (Pos_DernierPoint = c; fname[c]!='\0'; c++)
      if (fname[c]=='.')
        Pos_DernierPoint = c;

    // Ensuite on recopie la partie qui suit le point (si nécessaire):
    if (fname[Pos_DernierPoint])
    {
      for (c = Pos_DernierPoint+1,Autre_curseur=9;fname[c]!='\0' && Autre_curseur < 12;c++,Autre_curseur++)
        Resultat[Autre_curseur]=fname[c];
    }
  }
  return Resultat;
}


// -- Rajouter a la liste des elements de la liste un element ---------------
void Ajouter_element_a_la_liste(char * fname, int Type)
//  Cette procedure ajoute a la liste chainee un fichier passé en argument.
{
  // Pointeur temporaire d'insertion
  Element_de_liste_de_fileselect * Element_temporaire;

  // On alloue de la place pour un nouvel element
  Element_temporaire=(Element_de_liste_de_fileselect *)malloc(sizeof(Element_de_liste_de_fileselect));

  // On met a jour le nouvel emplacement:
  strcpy(Element_temporaire->NomAbrege,Nom_formate(fname, Type));
  strcpy(Element_temporaire->NomComplet,fname);
  Element_temporaire->Type = Type;

  Element_temporaire->Suivant  =Liste_du_fileselect;
  Element_temporaire->Precedent=NULL;

  if (Liste_du_fileselect!=NULL)
    Liste_du_fileselect->Precedent=Element_temporaire;
  Liste_du_fileselect=Element_temporaire;
}

// -- Vérification si un fichier a l'extension demandée.
// Autorise les '?', et '*' si c'est le seul caractère.
int VerifieExtension(const char *NomFichier, char * Filtre)
{
  int Pos_DernierPoint = -1;
  int c = 0;

  if (Filtre[0] == '*')
    return 1;
  // On recherche la position du dernier . dans le nom
  for (c = 0; NomFichier[c]!='\0'; c++)
    if (NomFichier[c]=='.')
      Pos_DernierPoint = c;
  // Fichier sans extension (ca arrive)
  if (Pos_DernierPoint == -1)
    return (Filtre[0] == '\0');

  // Vérification caractère par caractère, case-insensitive.
  c = 0;
  do
  {
    if (Filtre[c] != '?' &&
      tolower(Filtre[c]) != tolower(NomFichier[Pos_DernierPoint + 1 + c]))
      return 0;

     c++;
  } while (Filtre[c++] != '\0');

  return 1;
}


// -- Lecture d'une liste de fichiers ---------------------------------------
void Lire_liste_des_fichiers(byte Format_demande)
//  Cette procédure charge dans la liste chainée les fichiers dont l'extension
// correspond au format demandé.
{
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* entry; // Structure de lecture des éléments
  char * Filtre = "*"; // Extension demandée
  struct stat Infos_enreg;
  char * Chemin_courant;

  // Tout d'abord, on déduit du format demandé un filtre à utiliser:
  if (Format_demande) // Format (extension) spécifique
    Filtre = FormatFichier[Format_demande-1].Extension;

  // Ensuite, on vide la liste actuelle:
  Detruire_liste_du_fileselect();
  // Après effacement, il ne reste ni fichier ni répertoire dans la liste
  Liste_Nb_fichiers=0;
  Liste_Nb_repertoires=0;

  // On lit tous les répertoires:
  Chemin_courant=getcwd(NULL,0);
  Repertoire_Courant=opendir(Chemin_courant);
  while ((entry=readdir(Repertoire_Courant)))
  {
    // On ignore le répertoire courant
    if ( !strcmp(entry->d_name, "."))
    {
      continue;
    }
    stat(entry->d_name,&Infos_enreg);
    // et que l'élément trouvé est un répertoire
    if( S_ISDIR(Infos_enreg.st_mode) &&
      // et que c'est ".."
      (!strcmp(entry->d_name, PARENT_DIR) ||
      // ou qu'il n'est pas caché
       Config.Lire_les_repertoires_caches ||
     !isHidden(entry)))
    {
      // On rajoute le répertoire à la liste
      Ajouter_element_a_la_liste(entry->d_name, 1);
      Liste_Nb_repertoires++;
    }
    else if (S_ISREG(Infos_enreg.st_mode) && //Il s'agit d'un fichier
      (Config.Lire_les_fichiers_caches || //Il n'est pas caché
      !isHidden(entry)))
    {
      if (VerifieExtension(entry->d_name, Filtre))
      {
        // On rajoute le fichier à la liste
        Ajouter_element_a_la_liste(entry->d_name, 0);
        Liste_Nb_fichiers++;
      }
    }
  }

#if defined(__MORPHOS__) || defined (__amigaos4__) || defined(__amigaos__)
  Ajouter_element_a_la_liste("/",1); // on amiga systems, / means parent. And there is no ..
  Liste_Nb_repertoires ++;
#endif

  closedir(Repertoire_Courant);
  free(Chemin_courant);

  Liste_Nb_elements=Liste_Nb_repertoires+Liste_Nb_fichiers;
}

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
void bstrtostr( BSTR in, STRPTR out, TEXT max )
{
  STRPTR iptr;
  dword i;

  iptr = BADDR( in );

  if( max > iptr[0] ) max = iptr[0];

  for( i=0; i<max; i++ ) out[i] = iptr[i+1];
  out[i] = 0;
}
#endif

// -- Lecture d'une liste de lecteurs / volumes -----------------------------
void Lire_liste_des_lecteurs(void)
{

  // Empty the current content of fileselector:
  Detruire_liste_du_fileselect();
  // Reset number of items
  Liste_Nb_fichiers=0;
  Liste_Nb_repertoires=0;

  #if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
  {
    struct DosList *dl;
    char tmp[256];

    dl = LockDosList( LDF_VOLUMES | LDF_READ );
    if( dl )
    {
      while( ( dl = NextDosEntry( dl, LDF_VOLUMES | LDF_READ ) ) )
      {
        bstrtostr( dl->dol_Name, tmp, 254 );
        strcat( tmp, ":" );
        Ajouter_element_a_la_liste( tmp, 2 );
        Liste_Nb_repertoires++;
      }
      UnLockDosList( LDF_VOLUMES | LDF_READ );
    }
  }
  #elif defined (__WIN32__)
  {
    char NomLecteur[]="A:\\";
    int DriveBits = GetLogicalDrives();
    int IndiceLecteur;
    int IndiceBit;
    // Sous Windows, on a la totale, presque aussi bien que sous DOS:
    IndiceLecteur = 0;
    for (IndiceBit=0; IndiceBit<26 && IndiceLecteur<23; IndiceBit++)
    {
      if ( (1 << IndiceBit) & DriveBits )
      {
        // On a ce lecteur, il faut maintenant déterminer son type "physique".
        // pour profiter des jolies icones de X-man.
        int TypeLecteur;
        char CheminLecteur[]="A:\\";
        // Cette API Windows est étrange, je dois m'y faire...
        CheminLecteur[0]='A'+IndiceBit;
        switch (GetDriveType(CheminLecteur))
        {
          case DRIVE_CDROM:
            TypeLecteur=LECTEUR_CDROM;
            break;
          case DRIVE_REMOTE:
            TypeLecteur=LECTEUR_NETWORK;
            break;
          case DRIVE_REMOVABLE:
            TypeLecteur=LECTEUR_FLOPPY_3_5;
            break;
          case DRIVE_FIXED:
            TypeLecteur=LECTEUR_HDD;
            break;
          default:
            TypeLecteur=LECTEUR_NETWORK;
            break;
        }
    	NomLecteur[0]='A'+IndiceBit;
    	Ajouter_element_a_la_liste(NomLecteur,2);
    	Liste_Nb_repertoires++;
        IndiceLecteur++;
      }
    }
  }
  #else
  {
    //Sous les différents unix, on va mettre
    // un disque dur qui pointera vers la racine,
    // et un autre vers le home directory de l'utilisateur.

    // Ensuite on utilise read_file_system_list pour compléter

    struct mount_entry* Liste_points_montage;
    struct mount_entry* next;

    #if defined(__BEOS__) || defined(__HAIKU__)
        char * home_dir = getenv("$HOME");
    #else
        char * home_dir = getenv("HOME");
    #endif
    Ajouter_element_a_la_liste("/", 2);
    Liste_Nb_repertoires++;
    if(home_dir)
    {
        Ajouter_element_a_la_liste(home_dir, 2);
        Liste_Nb_repertoires++;
    }

    Liste_points_montage = read_file_system_list(0);

    while(Liste_points_montage != NULL)
    {
        if(Liste_points_montage->me_dummy == 0 && strcmp(Liste_points_montage->me_mountdir,"/") && strcmp(Liste_points_montage->me_mountdir,"/home"))
        {
            Ajouter_element_a_la_liste(Liste_points_montage->me_mountdir,2);
            Liste_Nb_repertoires++;
        }
        next = Liste_points_montage -> me_next;
        #if !(defined(__macosx__) || defined(__FreeBSD__))
          free(Liste_points_montage -> me_type);
        #endif
        free(Liste_points_montage -> me_devname);
        free(Liste_points_montage -> me_mountdir);
        free(Liste_points_montage);
        Liste_points_montage = next;
    }

  }
  #endif

  Liste_Nb_elements=Liste_Nb_repertoires+Liste_Nb_fichiers;
}


// -- Tri de la liste des fichiers et répertoires ---------------------------
void Trier_la_liste_des_fichiers(void)
// Tri la liste chainée existante dans l'ordre suivant:
//
// * Les répertoires d'abord, dans l'ordre alphabétique de leur nom
// * Les fichiers ensuite, dans l'ordre alphabétique de leur nom
{
  byte   La_liste_est_triee; // Booléen "La liste est triée"
  byte   need_swap;          // Booléen "Il faut inverser les éléments"
  Element_de_liste_de_fileselect * Element_precedent;
  Element_de_liste_de_fileselect * Element_courant;
  Element_de_liste_de_fileselect * Element_suivant;
  Element_de_liste_de_fileselect * Element_suivant_le_suivant;

  // Avant de trier quoi que ce soit, on vérifie qu'il y ait suffisamment
  // d'éléments pour qu'il soit possibles qu'ils soient en désordre:
  if (Liste_Nb_elements>1)
  {
    do
    {
      // Par défaut, on considère que la liste est triée
      La_liste_est_triee=1;

      Element_courant=Liste_du_fileselect;
      Element_suivant=Element_courant->Suivant;

      while ( (Element_courant!=NULL) && (Element_suivant!=NULL) )
      {
        // On commence par supposer qu'il n'y pas pas besoin d'inversion
        need_swap=0;

        // Ensuite, on vérifie si les deux éléments sont bien dans l'ordre ou
        // non:

          // Si l'élément courant est un fichier est que le suivant est
          // un répertoire -> need_swap
        if ( Element_courant->Type < Element_suivant->Type )
          need_swap=1;
          // Si les deux éléments sont de même type et que le nom du suivant
          // est plus petit que celui du courant -> need_swap
        else if ( (Element_courant->Type==Element_suivant->Type) &&
                  (strcmp(Element_courant->NomComplet,Element_suivant->NomComplet)>0) )
          need_swap=1;


        if (need_swap)
        {
          // Si les deux éléments nécessitent d'être inversé:

          // On les inverses:

          // On note avant tout les éléments qui encapsulent nos deux amis
          Element_precedent         =Element_courant->Precedent;
          Element_suivant_le_suivant=Element_suivant->Suivant;

          // On permute le chaînage des deux éléments entree eux
          Element_courant->Suivant  =Element_suivant_le_suivant;
          Element_courant->Precedent=Element_suivant;
          Element_suivant->Suivant  =Element_courant;
          Element_suivant->Precedent=Element_precedent;

          // On tente un chaînage des éléments encapsulant les compères:
          if (Element_precedent!=NULL)
            Element_precedent->Suivant=Element_suivant;
          if (Element_suivant_le_suivant!=NULL)
            Element_suivant_le_suivant->Precedent=Element_courant;

          // On fait bien attention à modifier la tête de liste en cas de besoin
          if (Element_courant==Liste_du_fileselect)
            Liste_du_fileselect=Element_suivant;

          // Ensuite, on se prépare à étudier les éléments précédents:
          Element_courant=Element_precedent;

          // Et on constate que la liste n'était pas encore génialement triée
          La_liste_est_triee=0;
        }
        else
        {
          // Si les deux éléments sont dans l'ordre:

          // On passe aux suivants
          Element_courant=Element_courant->Suivant;
          Element_suivant=Element_suivant->Suivant;
        }
      }
    }
    while (!La_liste_est_triee);
  }
}


// -- Affichage des éléments de la liste de fichier / répertoire ------------
void Afficher_la_liste_des_fichiers(short Decalage_premier,short Decalage_select)
//
// Decalage_premier = Décalage entre le premier fichier visible dans le
//                   sélecteur et le premier fichier de la liste
//
// Decalage_select  = Décalage entre le premier fichier visible dans le
//                   sélecteur et le fichier sélectionné dans la liste
//
{
  Element_de_liste_de_fileselect * Element_courant;
  byte   Indice;  // Indice du fichier qu'on affiche (0 -> 9)
  byte   Couleur_texte;
  byte   Couleur_fond;


  // On vérifie s'il y a au moins 1 fichier dans la liste:
  if (Liste_Nb_elements>0)
  {
    // On commence par chercher à pointer sur le premier fichier visible:
    Element_courant=Liste_du_fileselect;
    for (;Decalage_premier>0;Decalage_premier--)
      Element_courant=Element_courant->Suivant;

    // Pour chacun des 10 éléments inscriptibles à l'écran
    for (Indice=0;Indice<10;Indice++)
    {
      // S'il est sélectionné:
      if (!Decalage_select)
      {
        // Si c'est un fichier
        if (Element_courant->Type==0)
          Couleur_texte=COULEUR_FICHIER_SELECT;
        else
          Couleur_texte=COULEUR_REPERTOIRE_SELECT;

        Couleur_fond=COULEUR_FOND_SELECT;
      }
      else
      {
        // Si c'est un fichier
        if (Element_courant->Type==0)
          Couleur_texte=COULEUR_FICHIER_NORMAL;
        else
          Couleur_texte=COULEUR_REPERTOIRE_NORMAL;

        Couleur_fond=COULEUR_FOND_NORMAL;
      }

      // On affiche l'élément
      Print_dans_fenetre(9,90+FILENAMESPACE+(Indice<<3),Element_courant->NomAbrege,Couleur_texte,Couleur_fond);

      // On passe à la ligne suivante
      Decalage_select--;
      Element_courant=Element_courant->Suivant;
      if (!Element_courant)
        break;
    } // Fin de la boucle d'affichage

  } // Fin du test d'existence de fichiers
}


// -- Récupérer le libellé d'un élément de la liste -------------------------
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * label,int *Type)
//
// Decalage_premier = Décalage entre le premier fichier visible dans le
//                   sélecteur et le premier fichier de la liste
//
// Decalage_select  = Décalage entre le premier fichier visible dans le
//                   sélecteur et le fichier à récupérer
//
// label          = Chaine de réception du libellé de l'élément
//
// Type             = Récupération du type: 0 fichier, 1 repertoire, 2 lecteur.
//                    Passer NULL si pas interessé.
{
  Element_de_liste_de_fileselect * Element_courant;

  // On vérifie s'il y a au moins 1 fichier dans la liste:
  if (Liste_Nb_elements>0)
  {
    // On commence par chercher à pointer sur le premier fichier visible:
    Element_courant=Liste_du_fileselect;
    for (;Decalage_premier>0;Decalage_premier--)
      Element_courant=Element_courant->Suivant;

    // Ensuite, on saute autant d'éléments que le décalage demandé:
    for (;Decalage_select>0;Decalage_select--)
      Element_courant=Element_courant->Suivant;

    // On recopie la chaîne
    strcpy(label, Element_courant->NomComplet);

    if (Type != NULL)
      *Type=Element_courant->Type;
  } // Fin du test d'existence de fichiers
}


// ----------------- Déplacements dans la liste de fichiers -----------------

void Select_Scroll_Down(short * Decalage_premier,short * Decalage_select)
// Fait scroller vers le bas le sélecteur de fichier... (si possible)
{
  if ( ((*Decalage_select)<9)
    && ( (*Decalage_select)+1 < Liste_Nb_elements ) )
    // Si la sélection peut descendre
    Afficher_la_liste_des_fichiers(*Decalage_premier,++(*Decalage_select));
  else // Sinon, descendre la fenêtre (si possible)
  if ((*Decalage_premier)+10<Liste_Nb_elements)
    Afficher_la_liste_des_fichiers(++(*Decalage_premier),*Decalage_select);
}


void Select_Scroll_Up(short * Decalage_premier,short * Decalage_select)
// Fait scroller vers le haut le sélecteur de fichier... (si possible)
{
  if ((*Decalage_select)>0)
    // Si la sélection peut monter
    Afficher_la_liste_des_fichiers(*Decalage_premier,--(*Decalage_select));
  else // Sinon, monter la fenêtre (si possible)
  if ((*Decalage_premier)>0)
    Afficher_la_liste_des_fichiers(--(*Decalage_premier),*Decalage_select);
}


void Select_Page_Down(short * Decalage_premier,short * Decalage_select, short lines)
{
  if (Liste_Nb_elements-1>*Decalage_premier+*Decalage_select)
  {
    if (*Decalage_select<9)
    {
      if (Liste_Nb_elements<10)
      {
        *Decalage_premier=0;
        *Decalage_select=Liste_Nb_elements-1;
      }
      else *Decalage_select=9;
    }
    else
    {
      if (Liste_Nb_elements>*Decalage_premier+18)
        *Decalage_premier+=lines;
      else
      {
        *Decalage_premier=Liste_Nb_elements-10;
        *Decalage_select=9;
      }
    }
  }
  Afficher_la_liste_des_fichiers(*Decalage_premier,*Decalage_select);
}


void Select_Page_Up(short * Decalage_premier,short * Decalage_select, short lines)
{
  if (*Decalage_premier+*Decalage_select>0)
  {
    if (*Decalage_select>0)
      *Decalage_select=0;
    else
    {
      if (*Decalage_premier>lines)
        *Decalage_premier-=lines;
      else
        *Decalage_premier=0;
    }
  }
  Afficher_la_liste_des_fichiers(*Decalage_premier,*Decalage_select);
}


void Select_End(short * Decalage_premier,short * Decalage_select)
{
  if (Liste_Nb_elements<10)
  {
    *Decalage_premier=0;
    *Decalage_select=Liste_Nb_elements-1;
  }
  else
  {
    *Decalage_premier=Liste_Nb_elements-10;
    *Decalage_select=9;
  }
  Afficher_la_liste_des_fichiers(*Decalage_premier,*Decalage_select);
}


void Select_Home(short * Decalage_premier,short * Decalage_select)
{
  Afficher_la_liste_des_fichiers((*Decalage_premier)=0,(*Decalage_select)=0);
}



short Calculer_decalage_click_dans_fileselector(void)
/*
  Renvoie le décalage dans le sélecteur de fichier sur lequel on a clické.
  Renvoie le décalage du dernier fichier si on a clické au delà.
  Renvoie -1 si le sélecteur est vide.
*/
{
  short Decalage_calcule;

  Decalage_calcule=(((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-(90+FILENAMESPACE))>>3;
  if (Decalage_calcule>=Liste_Nb_elements)
    Decalage_calcule=Liste_Nb_elements-1;

  return Decalage_calcule;
}

void Afficher_bookmark(T_Bouton_dropdown * Bouton, int Numero_bookmark)
{
  if (Config.Bookmark_directory[Numero_bookmark])
  {
    int Taille;
    // Libellé
    Print_dans_fenetre_limite(Bouton->Pos_X+3+10,Bouton->Pos_Y+2,Config.Bookmark_label[Numero_bookmark],8,CM_Noir,CM_Clair);
    Taille=strlen(Config.Bookmark_label[Numero_bookmark]);
    if (Taille<8)
      Block(Fenetre_Pos_X+(Menu_Facteur_X*(Bouton->Pos_X+3+10+Taille*8)),Fenetre_Pos_Y+(Menu_Facteur_Y*(Bouton->Pos_Y+2)),Menu_Facteur_X*(8-Taille)*8,Menu_Facteur_Y*8,CM_Clair);
    // Menu apparait sur clic droit
    Bouton->Bouton_actif=A_DROITE;
    // Choix actifs
    Fenetre_Dropdown_vider_choix(Bouton);
    Fenetre_Dropdown_choix(Bouton,0,"Set");
    Fenetre_Dropdown_choix(Bouton,1,"Rename");
    Fenetre_Dropdown_choix(Bouton,2,"Clear");    
  }
  else
  {
    // Libellé
    Print_dans_fenetre(Bouton->Pos_X+3+10,Bouton->Pos_Y+2,"--------",CM_Fonce,CM_Clair);
    // Menu apparait sur clic droit ou gauche
    Bouton->Bouton_actif=A_DROITE|A_GAUCHE;
    // Choix actifs
    Fenetre_Dropdown_vider_choix(Bouton);
    Fenetre_Dropdown_choix(Bouton,0,"Set");
  }
}


void for_each_file(const char * Nom_repertoire, void Callback(const char *))
{
  // Pour scan de répertoire
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* entry; // Structure de lecture des éléments
  char Nom_fichier_complet[TAILLE_CHEMIN_FICHIER];
  int Position_nom_fichier;
  strcpy(Nom_fichier_complet, Nom_repertoire);
  Repertoire_Courant=opendir(Nom_repertoire);
  if(Repertoire_Courant == NULL) return;        // Répertoire invalide ...
  strcat(Nom_fichier_complet, SEPARATEUR_CHEMIN);
  Position_nom_fichier = strlen(Nom_fichier_complet);
  while ((entry=readdir(Repertoire_Courant)))
  {
    struct stat Infos_enreg;
    strcpy(&Nom_fichier_complet[Position_nom_fichier], entry->d_name);
    stat(Nom_fichier_complet,&Infos_enreg);
    if (S_ISREG(Infos_enreg.st_mode))
    {
      Callback(Nom_fichier_complet);
    }
  }
  closedir(Repertoire_Courant);
}
