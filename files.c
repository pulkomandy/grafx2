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

#if defined(__amigaos4__) || defined(__AROS__)
    #include <proto/dos.h>
    #include <dirent.h>
    #define isHidden(Enreg) (0)
#elif defined(__WATCOMC__)
    #include <direct.h>
    #define isHidden(Enreg) ((Enreg)->d_attr & _A_HIDDEN)
#elif defined(__WIN32__)
    #include <dirent.h>
    #include <windows.h>
    #define isHidden(Enreg) (GetFileAttributesA((Enreg)->d_name)&FILE_ATTRIBUTE_HIDDEN)
#else
    #include <dirent.h>
    #define isHidden(Enreg) ((Enreg)->d_name[0]=='.')
#endif

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include "erreurs.h"
#include "io.h"
#include "windows.h"

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
  struct Element_de_liste_de_fileselect * Element_temporaire;

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
char * Nom_formate(char * Nom, int Type)
{
  static char Resultat[13];
  int         Curseur;
  int         Autre_curseur;
  int         Pos_DernierPoint;

  if (strcmp(Nom,"..")==0)
  {
    strcpy(Resultat,"..          ");
  }
  else if (Nom[0]=='.' || Type==2)
  {
    // Fichiers ".quelquechose" ou lecteurs: Calé à gauche sur 12 caractères maximum.
    strcpy(Resultat,"            ");
    for (Curseur=0;Nom[Curseur]!='\0' && Curseur < 12;Curseur++)
      Resultat[Curseur]=Nom[Curseur];
    // Un caractère spécial pour indiquer que l'affichage est tronqué
    if (Curseur >= 12)
      Resultat[11]=CARACTERE_SUSPENSION;
  }
  else
  {
    strcpy(Resultat,"        .   ");
    // On commence par recopier la partie précédent le point:
    for (Curseur=0;( (Nom[Curseur]!='.') && (Nom[Curseur]!='\0') );Curseur++)
    {
      if (Curseur < 8)
        Resultat[Curseur]=Nom[Curseur];
    }
    // Un caractère spécial pour indiquer que l'affichage est tronqué
    if (Curseur > 8)
      Resultat[7]=CARACTERE_SUSPENSION;
    // On recherche le dernier point dans le reste du nom
    for (Pos_DernierPoint = Curseur; Nom[Curseur]!='\0'; Curseur++)
      if (Nom[Curseur]=='.')
        Pos_DernierPoint = Curseur;

    // Ensuite on recopie la partie qui suit le point (si nécessaire):
    if (Nom[Pos_DernierPoint])
    {
      for (Curseur = Pos_DernierPoint+1,Autre_curseur=9;Nom[Curseur]!='\0' && Autre_curseur < 12;Curseur++,Autre_curseur++)
        Resultat[Autre_curseur]=Nom[Curseur];
    }
  }
  return Resultat;
}


// -- Rajouter a la liste des elements de la liste un element ---------------
void Ajouter_element_a_la_liste(char * Nom, int Type)
//  Cette procedure ajoute a la liste chainee un fichier passé en argument.
{
  // Pointeur temporaire d'insertion
  struct Element_de_liste_de_fileselect * Element_temporaire;

  // On alloue de la place pour un nouvel element
  Element_temporaire=(struct Element_de_liste_de_fileselect *)malloc(sizeof(struct Element_de_liste_de_fileselect));

  // On met a jour le nouvel emplacement:
  strcpy(Element_temporaire->NomAbrege,Nom_formate(Nom, Type));
  strcpy(Element_temporaire->NomComplet,Nom);
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
  int Curseur = 0;

  if (Filtre[0] == '*')
    return 1;
  // On recherche la position du dernier . dans le nom
  for (Curseur = 0; NomFichier[Curseur]!='\0'; Curseur++)
    if (NomFichier[Curseur]=='.')
      Pos_DernierPoint = Curseur;
  // Fichier sans extension (ca arrive)
  if (Pos_DernierPoint == -1)
    return (Filtre[0] == '\0');
  
  // Vérification caractère par caractère, case-insensitive.
  Curseur = 0;
  do
  {
    if (Filtre[Curseur] != '?' &&
      tolower(Filtre[Curseur]) != tolower(NomFichier[Pos_DernierPoint + 1 + Curseur]))
      return 0;
    
     Curseur++;
  } while (Filtre[Curseur++] != '\0');
  
  return 1;
}


// -- Lecture d'une liste de fichiers ---------------------------------------
void Lire_liste_des_fichiers(byte Format_demande)
//  Cette procédure charge dans la liste chainée les fichiers dont l'extension
// correspond au format demandé.
{
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* Enreg; // Structure de lecture des éléments
  char * Filtre = "*"; // Extension demandée
  struct stat Infos_enreg;
  char * Chemin_courant;

  // Tout d'abord, on déduit du format demandé un filtre à utiliser:
  if (Format_demande) // Format (extension) spécifique
    Filtre = Format_Extension[Format_demande-1];

  // Ensuite, on vide la liste actuelle:
  Detruire_liste_du_fileselect();
  // Après effacement, il ne reste ni fichier ni répertoire dans la liste
  Liste_Nb_fichiers=0;
  Liste_Nb_repertoires=0;

  // On lit tous les répertoires:
  Chemin_courant=getcwd(NULL,0);
  Repertoire_Courant=opendir(Chemin_courant);
  while ((Enreg=readdir(Repertoire_Courant)))
  {
    // On ignore le répertoire courant
    if ( !strcmp(Enreg->d_name, "."))
    {
      continue;
    }
    stat(Enreg->d_name,&Infos_enreg);
    // et que l'élément trouvé est un répertoire
    if( S_ISDIR(Infos_enreg.st_mode) &&
      // et que c'est ".."
      (!strcmp(Enreg->d_name, "..") ||
      // ou qu'il n'est pas caché
       Config.Lire_les_repertoires_caches ||
     !isHidden(Enreg)))
    {
      // On rajoute le répertoire à la liste
      Ajouter_element_a_la_liste(Enreg->d_name, 1);
      Liste_Nb_repertoires++;
    }
    else if (S_ISREG(Infos_enreg.st_mode) && //Il s'agit d'un fichier
      (Config.Lire_les_fichiers_caches || //Il n'est pas caché
      !isHidden(Enreg))) 
    {
      if (VerifieExtension(Enreg->d_name, Filtre))
      {
        // On rajoute le fichier à la liste
        Ajouter_element_a_la_liste(Enreg->d_name, 0);
        Liste_Nb_fichiers++;
      }
    }
  }

  closedir(Repertoire_Courant);
  free(Chemin_courant);

  Liste_Nb_elements=Liste_Nb_repertoires+Liste_Nb_fichiers;
}

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__)
void bstrtostr( BSTR in, STRPTR out, TEXT max )
{
  STRPTR iptr;
  uint32 i;
  
  iptr = BADDR( in );
  
  if( max > iptr[0] ) max = iptr[0];
  
  for( i=0; i<max; i++ ) out[i] = iptr[i+1];
  out[i] = 0;
}
#endif

// -- Lecture d'une liste de lecteurs / volumes -----------------------------
void Lire_liste_des_lecteurs(void)
{
#ifndef __amigaos4__
  int Indice;
#endif

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

  // Other platforms: simply read the "static" list of Drives.
  // This should be made dynamic because in the multitask world, user can mount new drives, 
  // connect to network ones, and so on, while Grafx2 is running.
  #else
    #warning "Your platform is missing some specific code here ! please check and correct ! :)"
    for (Indice=0; Indice<Nb_drives; Indice++)
    {
      // Add the drive's name ("c:\\", "/" etc.) to the list
      Ajouter_element_a_la_liste(Drive[Indice].Chemin, 2);
      Liste_Nb_repertoires++;
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
  byte   Inversion;          // Booléen "Il faut inverser les éléments"
  struct Element_de_liste_de_fileselect * Element_precedent;
  struct Element_de_liste_de_fileselect * Element_courant;
  struct Element_de_liste_de_fileselect * Element_suivant;
  struct Element_de_liste_de_fileselect * Element_suivant_le_suivant;

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
        Inversion=0;

        // Ensuite, on vérifie si les deux éléments sont bien dans l'ordre ou
        // non:

          // Si l'élément courant est un fichier est que le suivant est
          // un répertoire -> Inversion
        if ( Element_courant->Type < Element_suivant->Type )
          Inversion=1;
          // Si les deux éléments sont de même type et que le nom du suivant
          // est plus petit que celui du courant -> Inversion
        else if ( (Element_courant->Type==Element_suivant->Type) &&
                  (strcmp(Element_courant->NomComplet,Element_suivant->NomComplet)>0) )
          Inversion=1;


        if (Inversion)
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
  struct Element_de_liste_de_fileselect * Element_courant;
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
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * Libelle,int *Type)
//
// Decalage_premier = Décalage entre le premier fichier visible dans le
//                   sélecteur et le premier fichier de la liste
//
// Decalage_select  = Décalage entre le premier fichier visible dans le
//                   sélecteur et le fichier à récupérer
//
// Libelle          = Chaine de réception du libellé de l'élément
//
// Type             = Récupération du type: 0 fichier, 1 repertoire, 2 lecteur.
//                    Passer NULL si pas interessé.
{
  struct Element_de_liste_de_fileselect * Element_courant;

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
    strcpy(Libelle, Element_courant->NomComplet);
    
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


void Select_Page_Down(short * Decalage_premier,short * Decalage_select)
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
        *Decalage_premier+=9;
      else
      {
        *Decalage_premier=Liste_Nb_elements-10;
        *Decalage_select=9;
      }
    }
  }
  Afficher_la_liste_des_fichiers(*Decalage_premier,*Decalage_select);
}


void Select_Page_Up(short * Decalage_premier,short * Decalage_select)
{
  if (*Decalage_premier+*Decalage_select>0)
  {
    if (*Decalage_select>0)
      *Decalage_select=0;
    else
    {
      if (*Decalage_premier>9)
        *Decalage_premier-=9;
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

void for_each_file(const char * Nom_repertoire, void Callback(const char *))
{
  // Pour scan de répertoire
  DIR*  Repertoire_Courant; //Répertoire courant
  struct dirent* Enreg; // Structure de lecture des éléments
  char Nom_fichier_complet[TAILLE_CHEMIN_FICHIER];
  int Position_nom_fichier;
  strcpy(Nom_fichier_complet, Nom_repertoire);
  Repertoire_Courant=opendir(Nom_repertoire);
  if(Repertoire_Courant == NULL) return;        // Répertoire invalide ...
  strcat(Nom_fichier_complet, SEPARATEUR_CHEMIN);
  Position_nom_fichier = strlen(Nom_fichier_complet);
  while ((Enreg=readdir(Repertoire_Courant)))
  {
    struct stat Infos_enreg;
    strcpy(&Nom_fichier_complet[Position_nom_fichier], Enreg->d_name);
    stat(Nom_fichier_complet,&Infos_enreg);
    if (S_ISREG(Infos_enreg.st_mode))
    {
      Callback(Nom_fichier_complet);
    }
  }
}
