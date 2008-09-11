/************************************************************************
*                                                                       *
* READLINE (procÇdure permettant de saisir une chaåne de caractäres) *
*                                                                       *
************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "graph.h"
#include "divers.h"
#include "erreurs.h"

#ifdef __linux__
    #include "linux.h"
#endif

#define COULEUR_TEXTE         CM_Noir
#define COULEUR_FOND          CM_Clair
#define COULEUR_TEXTE_CURSEUR CM_Noir
#define COULEUR_FOND_CURSEUR  CM_Fonce

// VÈrification de la validitÈ d'un nom de fichier
byte Chaine_valide(char * Chaine)
{
  byte Point_trouve=0; // BoolÈen "on a trouvÈ un point dans la chaine"
  byte Taille_racine=0; // Taille de la racine du nom de fichier
  byte Taille_extension=0; // Taille de l'extension du nom de fichier
  byte Position; // Position du caractËre dans la chaÓne en cours d'Ètude

  for (Position=0;Chaine[Position]!='\0';Position++)
  {
    if (Chaine[Position]!='.')
    {
      if (Point_trouve)
        Taille_extension++;
      else
        Taille_racine++;
    }
    else
    {
      if (Point_trouve)
        return 0;
      else
        Point_trouve=1;
    }
  }

  return ( (Taille_racine>0) && (Taille_racine<=8) && (Taille_extension<=3) );
}


// Suppresion d'un caractäre Ö une certaine POSITION dans une CHAINE.
void Supprimer_caractere(char * Chaine, byte Position)
{
  for (;Chaine[Position]!='\0';Position++)
    Chaine[Position]=Chaine[Position+1];
}


void Inserer_caractere(char * Chaine, char Lettre, byte Position)
//  Insertion d'une LETTRE Ö une certaine POSITION
//  dans une CHAINE d'une certaine TAILLE.
{
  char Char_tempo;

  for (;Lettre!='\0';Position++)
  {
    // On mÇmorise le caractäre qui se trouve en "Position"
    Char_tempo=Chaine[Position];
    // On splotch la lettre Ö insÇrer
    Chaine[Position]=Lettre;
    // On place le caractäre mÇmorisÇ dans "Lettre" comme nouvelle lettre Ö insÇrer
    Lettre=Char_tempo;
  }
  // On termine la chaine
  Chaine[Position]='\0';
}


void Rafficher_toute_la_chaine(word Pos_X,word Pos_Y,char * Chaine,byte Position)
{
  Print_dans_fenetre(Pos_X,Pos_Y,Chaine,COULEUR_TEXTE,COULEUR_FOND);
  Print_char_dans_fenetre(Pos_X+(Position<<3),Pos_Y,Chaine[Position],COULEUR_TEXTE_CURSEUR,COULEUR_FOND_CURSEUR);
}


//****************************************************************************
//*           Enhanced super scanf deluxe pro plus giga mieux :-)            *
//****************************************************************************
byte Readline(word Pos_X,word Pos_Y,char * Chaine,byte Taille_maxi,byte Type_saisie)
// Paramätres:
//   Pos_X, Pos_Y : CoordonnÇes de la saisie dans la fenàtre
//   Chaine       : Chaåne recevant la saisie (et contenant Çventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractäres logeant dans la zone de saisie
//   Type_saisie  : 0=Chaåne, 1=Nombre, 2=Nom de fichier (12 carcactäres)
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  char Chaine_initiale[256];
  byte Position;
  byte Taille;
  word Touche_lue=0;
  byte Touche_autorisee;


  // Effacement de la chaåne
  Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
        Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);

  // Mise Ö jour des variables se rapportant Ö la chaåne en fonction de la chaåne initiale
  strcpy(Chaine_initiale,Chaine);

  if (Type_saisie==1)
    itoa(atoi(Chaine),Chaine,10); // On tasse la chaine Ö gauche
  //  Chaine[0]='\0';    // On efface la chaåne si c'est valeur numÇrique

  Taille=strlen(Chaine);
  Position=(Taille<Taille_maxi)? Taille:Taille-1;
  Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position);


  while ((Touche_lue!=SDLK_RETURN) && (Touche_lue!=SDLK_ESCAPE))
  {
    Touche_lue=Get_key();
    switch (Touche_lue)
    {
      case SDLK_DELETE : // Suppr.
            if (Position<Taille)
            {
              Supprimer_caractere(Chaine,Position);
              Taille--;
              // Effacement de la chaåne
              Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                    Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position);
            }
      break;
      case SDLK_LEFT : // Gauche
            if (Position)
            {
              // Effacement de la chaåne
              if (Position==Taille)
                Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                      Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,--Position);
            }
      break;
      case SDLK_RIGHT : // Droite
            if ((Position<Taille) && (Position<Taille_maxi-1))
              Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,++Position);
      break;
      case SDLK_HOME : // Home
            if (Position)
            {
              // Effacement de la chaåne
              if (Position==Taille)
                Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                      Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position=0);
            }
      break;
      case SDLK_END : // End
            if ((Position<Taille) && (Position<Taille_maxi-1))
              Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position=(Taille<Taille_maxi)?Taille:Taille-1);
      break;
      case  SDLK_BACKSPACE :
        if (Position)
        {
          Supprimer_caractere(Chaine,--Position);
          Taille--;
          // Effacement de la chaåne
          Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
          Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position);
        }
        break;
      case SDLK_RETURN :
        if ( (Type_saisie!=2) || (Chaine_valide(Chaine)) )
          break;
        // Si on Çtait en saisie de nom de fichier et qu'il y ait une erreur
        // dans la chaåne
        Erreur(0); // On flash en rouge & ...
        Touche_lue=SDLK_ESCAPE; // ... on simule l'appuie sur la touche [Esc]
      case SDLK_ESCAPE :
        // On restaure la chaine initiale
        strcpy(Chaine,Chaine_initiale);
        Taille=strlen(Chaine);
        break;
      default :
        if (Taille<Taille_maxi)
        {
          // On va regarder si l'utilisateur le droit de se servir de cette touche
          Touche_autorisee=0; // On commence par supposer qu'elle est interdite
          switch(Type_saisie)
          {
            case 0 : // N'importe quelle chaåne:
              if (Touche_lue>=' ')
                Touche_autorisee=1;
              break;
            case 1 : // Nombre
              if ( (Touche_lue>='0') && (Touche_lue<='9') )
                Touche_autorisee=1;
              break;
            default : // Nom de fichier
              // On commence par passez la lettre en majuscule
              if ( (Touche_lue>='a') && (Touche_lue<='z') )
                Touche_lue-=32;
              // Ensuite on regarde si la touche est autorisÇe
              if ( (Touche_lue> ' ') && (Touche_lue!='+') && (Touche_lue!='\\') &&
                   (Touche_lue!='>') && (Touche_lue!='<') && (Touche_lue!='*')  &&
                   (Touche_lue!='?') && (Touche_lue!=':') && (Touche_lue!='|')  &&
                   (Touche_lue!='/') && (Touche_lue!='"') && (Touche_lue!='=')  &&
                   (Touche_lue!=',') && (Touche_lue!=';') && (Touche_lue!='[')  &&
                   (Touche_lue!=']') )
                Touche_autorisee=1;
          } // Fin du "switch(Type_saisie)"

          // Si la touche Çtait autorisÇe...
          if (Touche_autorisee)
          {
            // ... alors on l'insäre ...
            Inserer_caractere(Chaine,Touche_lue,Position/*,Taille*/);
            // ce qui augmente la taille de la chaine
            Taille++;
            // et qui risque de dÇplacer le curseur vers la droite
            if (Taille<Taille_maxi)
              Position++;
            // Enfin, on raffiche la chaine
            Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine,Position);
          } // Fin du test d'autorisation de touche
        } // Fin du test de place libre
    } // Fin du "switch(Touche_lue)"
  } // Fin du "while"

  // Effacement de la chaåne
  Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
        Taille_maxi*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
  // On raffiche la chaine correctement
  if (Type_saisie==1)
  {
    if (Chaine[0]=='\0')
    {
      strcpy(Chaine,"0");
      Taille=1;
    }
    Print_dans_fenetre(Pos_X+((Taille_maxi-Taille)<<3),Pos_Y,Chaine,COULEUR_TEXTE,COULEUR_FOND);
  }
  else
    Print_dans_fenetre(Pos_X,Pos_Y,Chaine,COULEUR_TEXTE,COULEUR_FOND);

  return (Touche_lue==SDLK_RETURN);
}
