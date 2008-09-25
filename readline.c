/************************************************************************
*                                                                       *
* READLINE (procédure permettant de saisir une chaîne de caractères) *
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
#include "const.h"
#include "linux.h"

#define COULEUR_TEXTE         CM_Noir
#define COULEUR_FOND          CM_Clair
#define COULEUR_TEXTE_CURSEUR CM_Noir
#define COULEUR_FOND_CURSEUR  CM_Fonce

// Suppresion d'un caractère à une certaine POSITION dans une CHAINE.
void Supprimer_caractere(char * Chaine, byte Position)
{
  for (;Chaine[Position]!='\0';Position++)
    Chaine[Position]=Chaine[Position+1];
}


void Inserer_caractere(char * Chaine, char Lettre, byte Position)
//  Insertion d'une LETTRE à une certaine POSITION
//  dans une CHAINE d'une certaine TAILLE.
{
  char Char_tempo;

  for (;Lettre!='\0';Position++)
  {
    // On mémorise le caractère qui se trouve en "Position"
    Char_tempo=Chaine[Position];
    // On splotch la lettre à insérer
    Chaine[Position]=Lettre;
    // On place le caractère mémorisé dans "Lettre" comme nouvelle lettre à insérer
    Lettre=Char_tempo;
  }
  // On termine la chaine
  Chaine[Position]='\0';
}

int CaractereValide(int Caractere)
{
  // Sous Linux: Seul le / est strictement interdit, mais beaucoup
  // d'autres poseront des problèmes au shell, alors on évite.
  // Sous Windows : c'est moins grave car le fopen() échouerait de toutes façons.
  #ifdef __linux__  
  char CaracteresInterdits[] = {'/', '|', '?', '*', '<', '>'};
  #else
  char CaracteresInterdits[] = {'/', '|', '?', '*', '<', '>', ':', '\\'};
  #endif
  
  if (Caractere < ' ' || Caractere > 255)
    return 0;
  
  int Position;
  for (Position=0; Position<sizeof(CaracteresInterdits); Position++)
    if (Caractere == CaracteresInterdits[Position])
      return 0;
  return 1;
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
// Paramètres:
//   Pos_X, Pos_Y : Coordonnées de la saisie dans la fenêtre
//   Chaine       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractères logeant dans la zone de saisie
//   Type_saisie  : 0=Chaîne, 1=Nombre, 2=Nom de fichier
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
{
  char Chaine_initiale[256];
  char Chaine_affichee[256];
  byte Position;
  byte Taille;
  word Touche_lue=0;
  byte Touche_autorisee;
  byte Taille_affichee;
  // Grosse astuce pour les noms de fichiers: La taille affichée est différente
  // de la taille maximum gérée.
  if (Type_saisie == 2)
    Taille_affichee = 27;
  else
    Taille_affichee = Taille_maxi;
  byte Offset=0; // Indice du premier caractère affiché


  // Effacement de la chaîne
  Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
        Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);

  // Mise à jour des variables se rapportant à la chaîne en fonction de la chaîne initiale
  strcpy(Chaine_initiale,Chaine);

  if (Type_saisie==1)
    itoa(atoi(Chaine),Chaine,10); // On tasse la chaine à gauche
  //  Chaine[0]='\0';    // On efface la chaîne si c'est valeur numérique


  Taille=strlen(Chaine);
  Position=(Taille<Taille_maxi)? Taille:Taille-1;
  // Formatage d'une partie de la chaine (si trop longue pour tenir)
  strncpy(Chaine_affichee, Chaine + Offset, Taille_affichee);
  Chaine_affichee[Taille_affichee]='\0';
  if (Offset>0)
    Chaine_affichee[0]=CARACTERE_TRIANGLE_GAUCHE;
  if (Taille_affichee + Offset + 1 < Taille )
    Chaine_affichee[Taille_affichee-1]=CARACTERE_TRIANGLE_DROIT;
  
  Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine_affichee,Position - Offset);


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
              
              // Effacement de la chaîne
              Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                    Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              goto affichage;
            }
      break;
      case SDLK_LEFT : // Gauche
            if (Position>0)
            {
              // Effacement de la chaîne
              if (Position==Taille)
                Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                      Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              Position--;
              if (Offset > 0 && (Position == 0 || Position < (Offset + 1)))
                Offset--;
              goto affichage;
            }
      break;
      case SDLK_RIGHT : // Droite
            if ((Position<Taille) && (Position<Taille_maxi-1))
            {
              Position++;
              //if (Position > Taille_affichee + Offset - 2)
              //if (Offset + Taille_affichee < Taille_maxi && (Position == Taille || (Position > Taille_affichee + Offset - 2)))
              if (Chaine_affichee[Position-Offset]==CARACTERE_TRIANGLE_DROIT || Position-Offset>=Taille_affichee)
                Offset++;
              goto affichage;
            }
      break;
      case SDLK_HOME : // Home
            if (Position)
            {
              // Effacement de la chaîne
              if (Position==Taille)
                Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                      Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
              Position = 0;
              Offset = 0;
              goto affichage;
            }
      break;
      case SDLK_END : // End
            if ((Position<Taille) && (Position<Taille_maxi-1))
            {
              Position=(Taille<Taille_maxi)?Taille:Taille-1;
              if (Position-Offset>Taille_affichee)
                Offset=Position-Taille_affichee+1;
              goto affichage;
            }
      break;
      case  SDLK_BACKSPACE :
        if (Position)
        {
          Supprimer_caractere(Chaine,--Position);
          Taille--;
          // Effacement de la chaîne
          Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
                Taille_affichee*(Menu_Facteur_X<<3),(Menu_Facteur_Y<<3),COULEUR_FOND);
          goto affichage;
        }
        break;
      case SDLK_RETURN :
        break;
        
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
            case 0 : // N'importe quelle chaîne:
              if (Touche_lue>=' ' && Touche_lue<= 255)
                Touche_autorisee=1;
              break;
            case 1 : // Nombre
              if ( (Touche_lue>='0') && (Touche_lue<='9') )
                Touche_autorisee=1;
              break;
            default : // Nom de fichier
              // On regarde si la touche est autorisée
              if ( CaractereValide(Touche_lue))
                Touche_autorisee=1;
          } // Fin du "switch(Type_saisie)"

          // Si la touche était autorisée...
          if (Touche_autorisee)
          {
            // ... alors on l'insère ...
            Inserer_caractere(Chaine,Touche_lue,Position/*,Taille*/);
            // ce qui augmente la taille de la chaine
            Taille++;
            // et qui risque de déplacer le curseur vers la droite
            if (Taille<Taille_maxi)
            {
              Position++;
              //if (Position > Taille_affichee + Offset - 2 && !(Position<Taille) && (Position<Taille_maxi-1))
              //if (Offset + Taille_affichee < Taille_maxi && (Position == Taille || (Position > Taille_affichee + Offset - 2)))
              if (Chaine_affichee[Position-Offset]==CARACTERE_TRIANGLE_DROIT || Position-Offset>=Taille_affichee)
                Offset++;
            }
            // Enfin, on raffiche la chaine
            goto affichage;
          } // Fin du test d'autorisation de touche
        } // Fin du test de place libre
      break;
      
      affichage:
        Taille=strlen(Chaine);
        // Formatage d'une partie de la chaine (si trop longue pour tenir)
        strncpy(Chaine_affichee, Chaine + Offset, Taille_affichee);
        Chaine_affichee[Taille_affichee]='\0';
        if (Offset>0)
          Chaine_affichee[0]=CARACTERE_TRIANGLE_GAUCHE;
        if (Taille_affichee + Offset + 0 < Taille )
          Chaine_affichee[Taille_affichee-1]=CARACTERE_TRIANGLE_DROIT;
        
        Rafficher_toute_la_chaine(Pos_X,Pos_Y,Chaine_affichee,Position - Offset);
      
    } // Fin du "switch(Touche_lue)"
  } // Fin du "while"

  // Effacement de la chaîne
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
