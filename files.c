#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "erreurs.h"

#ifdef __linux__
    #include "linux.h"
    #include <dirent.h>
    #define isHidden(Enreg) ((Enreg)->d_name[0]=='.')
#elif defined(__WATCOMC__)
    #include <direct.h>
    #define isHidden(Enreg) ((Enreg)->d_attr & _A_HIDDEN)
#elif defined(__MINGW32__)
    #include <dirent.h>
/* TODO: trouver une méthode portable pour déterminer si un fichier est caché */
    #define isHidden(Enreg) (0)
#endif

#define COULEUR_FICHIER_NORMAL    CM_Clair // Couleur du texte pour une ligne de fichier non sélectionné
#define COULEUR_REPERTOIRE_NORMAL CM_Fonce // Couleur du texte pour une ligne de répertoire non sélectionné
#define COULEUR_FOND_NORMAL       CM_Noir  // Couleur du fond  pour une ligne non sélectionnée
#define COULEUR_FICHIER_SELECT    CM_Blanc // Couleur du texte pour une ligne de fichier    sélectionnée
#define COULEUR_REPERTOIRE_SELECT CM_Clair // Couleur du texte pour une ligne de repértoire sélectionnée
#define COULEUR_FOND_SELECT       CM_Fonce // Couleur du fond  pour une ligne sélectionnée

#define FILENAMESPACE 16

int Determiner_repertoire_courant(void)
// Modifie Principal_Repertoire_courant en y mettant sa nouvelle valeur (avec le nom du
// disque)
//
// Renvoie 1 s'il y a eu une erreur d'accès
{
  return (getcwd(Principal_Repertoire_courant,256)==NULL);
}


int Repertoire_existe(char * Repertoire)
//   Détermine si un répertoire passé en paramètre existe ou non dans le
// répertoire courant.
{
  DIR* Enreg;    // Structure de lecture des éléments

  if (strcmp(Repertoire,"..")==0)
    return 1;
  else
  {
    //  On va chercher si le répertoire existe à l'aide d'un Opendir. S'il
    //  renvoie NULL c'est que le répertoire n'est pas accessible...

    Enreg=opendir(Repertoire);
    if (Enreg==NULL)
        return 0;
    else
    {
        closedir(Enreg);
        return 1;
    }
  }
}


int Fichier_existe(char * Fichier)
//   Détermine si un fichier passé en paramŠtre existe ou non dans le
// répertoire courant.
{
    struct stat buf;
    int Resultat;

    Resultat=stat(Fichier,&buf);
    if (Resultat!=0)
        return(errno!=ENOENT);
    else
        return 1;

}



// Conventions:
//
// * Le fileselect modifie le répertoire courant. Ceci permet de n'avoir
//   qu'un findfirst dans le répertoire courant … faire:


// -- Déstruction de la liste chaŒnée ---------------------------------------
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
char * Nom_formate(char * Nom)
{
  static char Resultat[13];
  int         Curseur;
  int         Autre_curseur;
  int         Pos_DernierPoint;

  if (strcmp(Nom,"..")==0)
  {
    strcpy(Resultat,"..          ");
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
void Ajouter_element_a_la_liste(struct dirent* Enreg)
//  Cette procedure ajoute a la liste chainee un fichier passé en argument.
{
  // Pointeur temporaire d'insertion
  struct Element_de_liste_de_fileselect * Element_temporaire;

  // On alloue de la place pour un nouvel element
  Element_temporaire=(struct Element_de_liste_de_fileselect *)malloc(sizeof(struct Element_de_liste_de_fileselect));

  // On met a jour le nouvel emplacement:
  strcpy(Element_temporaire->NomAbrege,Nom_formate(Enreg->d_name));
  strcpy(Element_temporaire->NomComplet,Enreg->d_name);
  #ifdef __linux__
    Element_temporaire->Type = (Enreg->d_type == DT_DIR);
  #elif __WATCOMC__
    Element_temporaire->Type = (Enreg->d_attr & _A_SUBDIR);
  #else
    struct stat Infos_enreg;
    stat(Enreg->d_name,&Infos_enreg);
    Element_temporaire->Type = S_ISDIR(Infos_enreg.st_mode);
  #endif  

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

  // Tout d'abord, on déduit du format demandé un filtre à utiliser:
  if (Format_demande) // Format (extension) spécifique
    Filtre = Format_Extension[Format_demande-1];

  // Ensuite, on vide la liste actuelle:
  Detruire_liste_du_fileselect();
  // Après effacement, il ne reste ni fichier ni répertoire dans la liste
  Liste_Nb_fichiers=0;
  Liste_Nb_repertoires=0;

  // On lit tous les répertoires:

  Repertoire_Courant=opendir(getcwd(NULL,0));
  Enreg=readdir(Repertoire_Courant);
  while (Enreg)
  {
    // Si l'élément n'est pas le répertoire courant
    if ( (Enreg->d_name[0]!='.') || (Enreg->d_name[1] != 0))
    {
    	stat(Enreg->d_name,&Infos_enreg);
        // et que l'élément trouvé est un répertoire
        if( S_ISDIR(Infos_enreg.st_mode) &&
          // et qu'il n'est pas caché
          ((!isHidden(Enreg)) || !Config.Lire_les_repertoires_caches))
        {
          // On rajoute le répertore à la liste
          Ajouter_element_a_la_liste(Enreg);
          Liste_Nb_repertoires++;
        }
        else if (S_ISREG(Infos_enreg.st_mode) //Il s'agit d'un fichier
          && ((!isHidden(Enreg)) || !Config.Lire_les_fichiers_caches)) //Il n'est pas caché
        {
          if (VerifieExtension(Enreg->d_name, Filtre))
          {
            // On rajoute le fichier à la liste
            Ajouter_element_a_la_liste(Enreg);
            Liste_Nb_fichiers++;
          }
        }
    }

    // On cherche l'élément suivant
    Enreg=readdir(Repertoire_Courant);
  }

  closedir(Repertoire_Courant);

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
      // Par défaut, on considŠre que la liste est triée
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
        if ( (Element_courant->Type==0) && (Element_suivant->Type==1) )
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

          // On permute le chaŒnage des deux éléments entree eux
          Element_courant->Suivant  =Element_suivant_le_suivant;
          Element_courant->Precedent=Element_suivant;
          Element_suivant->Suivant  =Element_courant;
          Element_suivant->Precedent=Element_precedent;

          // On tente un chaŒnage des éléments encapsulant les compŠres:
          if (Element_precedent!=NULL)
            Element_precedent->Suivant=Element_suivant;
          if (Element_suivant_le_suivant!=NULL)
            Element_suivant_le_suivant->Precedent=Element_courant;

          // On fait bien attention … modifier la tête de liste en cas de besoin
          if (Element_courant==Liste_du_fileselect)
            Liste_du_fileselect=Element_suivant;

          // Ensuite, on se prépare … étudier les éléments précédents:
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
    // On commence par chercher … pointer sur le premier fichier visible:
    Element_courant=Liste_du_fileselect;
    for (;Decalage_premier>0;Decalage_premier--)
      Element_courant=Element_courant->Suivant;

    // Pour chacun des 10 éléments inscriptibles … l'écran
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

      // On passe … la ligne suivante
      Decalage_select--;
      Element_courant=Element_courant->Suivant;
      if (!Element_courant)
        break;
    } // Fin de la boucle d'affichage

  } // Fin du test d'existence de fichiers
}


// -- Récupérer le libellé d'un élément de la liste -------------------------
void Determiner_element_de_la_liste(short Decalage_premier,short Decalage_select,char * Libelle)
//
// Decalage_premier = Décalage entre le premier fichier visible dans le
//                   sélecteur et le premier fichier de la liste
//
// Decalage_select  = Décalage entre le premier fichier visible dans le
//                   sélecteur et le fichier … récupérer
//
// Libelle          = Chaine de réception du libellé de l'élément
//
{
  struct Element_de_liste_de_fileselect * Element_courant;
  char * Curseur;


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
  Renvoie le décalage du dernier fichier si on a clické au del….
  Renvoie -1 si le sélecteur est vide.
*/
{
  short Decalage_calcule;

  Decalage_calcule=(((Mouse_Y-Fenetre_Pos_Y)/Menu_Facteur_Y)-(90+FILENAMESPACE))>>3;
  if (Decalage_calcule>=Liste_Nb_elements)
    Decalage_calcule=Liste_Nb_elements-1;

  return Decalage_calcule;
}
