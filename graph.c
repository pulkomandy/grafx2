/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
#include "sdlscreen.h"
#include "graph.h"
#include "divers.h"
#include <math.h>
#include <malloc.h>

#include <string.h>
#include <stdlib.h>
#include "moteur.h"
#include "boutons.h"
#include "pages.h"
#include "global.h"
#include "struct.h"
#include "erreurs.h"

#ifdef __linux__
    #include <sys/sysinfo.h>
#elif __WATCOMC__
    #define _WIN32_WINNT 0x0500
    #include <windows.h>
#endif

// Fonction qui met à jour la zone de l'image donnée en paramètre sur l'écran.
// Tient compte du décalage X et Y et du zoom, et fait tous les controles nécessaires
void Mettre_Ecran_A_Jour(short X, short Y, short Largeur, short Hauteur)
{
	short L_effectif, H_effectif;
	short X_effectif;
	short Y_effectif;

	// Première étape, si L ou H est négatif, on doit remettre la zone à l'endroit
	if (Largeur < 0)
	{
		X += Largeur;
		Largeur = - Largeur;
	}

	if (Hauteur < 0)
	{
		Y += Hauteur;
		Hauteur = - Hauteur;
	}

	// D'abord on met à jour dans la zone écran normale
	X_effectif = Max(X-Principal_Decalage_X,0);
	Y_effectif = Max(Y-Principal_Decalage_Y,0);

	// Normalement il ne faudrait pas updater au delà du split quand on est en mode loupe,
	// mais personne ne devrait demander d'update en dehors de cette limite, même le fill est contraint
	// a rester dans la zone visible de l'image
	if(X_effectif + Largeur <= Principal_Largeur_image) L_effectif = Largeur;
	else L_effectif = Principal_Largeur_image - X_effectif;

	if(Y_effectif + Hauteur <= Menu_Ordonnee) H_effectif = Hauteur;
	else H_effectif = Menu_Ordonnee - Y_effectif;

	SDL_UpdateRect(Ecran_SDL,X_effectif,Y_effectif,L_effectif,H_effectif);

	// Et ensuite dans la partie zoomée
	if(Loupe_Mode)
	{
		X_effectif = Min(Max((X-Loupe_Decalage_X + 1)*Loupe_Facteur + 2 + Principal_Split + LARGEUR_BARRE_SPLIT,0), Largeur_ecran);
		Y_effectif = Min(Max(Y-Loupe_Decalage_Y,0) * Loupe_Facteur, Menu_Ordonnee);

		Largeur *= Loupe_Facteur / 2; // ???!
		Hauteur *= Loupe_Facteur;

		// Normalement il ne faudrait pas updater au delà du split quand on est en mode loupe,
		// mais personne ne devrait demander d'update en dehors de cette limite, même le fill est contraint
		// a rester dans la zone visible de l'image
		if(X_effectif + Largeur < Largeur_ecran) L_effectif = (Largeur+2) * Menu_Facteur_X;
		else L_effectif = Largeur_ecran - X_effectif;

		if(Y_effectif + Hauteur <= Menu_Ordonnee) H_effectif = Hauteur;
		else H_effectif = Menu_Ordonnee - Y_effectif;

		SDL_UpdateRect(Ecran_SDL,X_effectif,Y_effectif,L_effectif,H_effectif);
	}
}

byte Meilleure_couleur(byte R,byte V,byte B)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist=0x7FFFFFFF;
  byte  Best_color=0;

  for (Coul=0; Coul<256; Coul++)
  {
    if (!Exclude_color[Coul])
    {
      Delta_R=(int)Principal_Palette[Coul].R-R;
      Delta_V=(int)Principal_Palette[Coul].V-V;
      Delta_B=(int)Principal_Palette[Coul].B-B;

      if (!(Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11)))
        return Coul;

      if (Dist<Best_dist)
      {
        Best_dist=Dist;
        Best_color=Coul;
      }
    }
  }

  return Best_color;
}

void Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
     (byte Rouge, byte Vert, byte Bleu, struct Composantes * Palette, byte * Table)
{
  short Coul;
  int   Delta_R,Delta_V,Delta_B;
  int   Dist;
  int   Best_dist[4]={0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};


  for (Coul=0; Coul<256; Coul++)
  {
    Delta_R=(int)Palette[Coul].R-Rouge;
    Delta_V=(int)Palette[Coul].V-Vert;
    Delta_B=(int)Palette[Coul].B-Bleu;

    Dist=(Delta_R*Delta_R*30)+(Delta_V*Delta_V*59)+(Delta_B*Delta_B*11);

    if (Dist<Best_dist[0])
    {
      Best_dist[3]=Best_dist[2];
      Best_dist[2]=Best_dist[1];
      Best_dist[1]=Best_dist[0];
      Best_dist[0]=Dist;
      Table[3]=Table[2];
      Table[2]=Table[1];
      Table[1]=Table[0];
      Table[0]=Coul;
    }
    else
    {
      if (Dist<Best_dist[1])
      {
        Best_dist[3]=Best_dist[2];
        Best_dist[2]=Best_dist[1];
        Best_dist[1]=Dist;
        Table[3]=Table[2];
        Table[2]=Table[1];
        Table[1]=Coul;
      }
      else
      {
        if (Dist<Best_dist[2])
        {
          Best_dist[3]=Best_dist[2];
          Best_dist[2]=Dist;
          Table[3]=Table[2];
          Table[2]=Coul;
        }
        else
        if (Dist<Best_dist[3])
        {
          Best_dist[3]=Dist;
          Table[3]=Coul;
        }
      }
    }
  }
}



byte Old_Noir;
byte Old_Fonce;
byte Old_Clair;
byte Old_Blanc;
byte Old_Trans;

void Remap_pixel(byte * Pix)
{
  if (*Pix==Old_Clair)         // On commence par tester le Gris clair
    *Pix=CM_Clair;             // qui est pas mal utilisé.
  else
  {
    if (*Pix==Old_Noir)        // Puis le Noir...
      *Pix=CM_Noir;
    else
    {
      if (*Pix==Old_Fonce)     // etc...
        *Pix=CM_Fonce;
      else
      {
        if (*Pix==Old_Blanc)
          *Pix=CM_Blanc;
        else
        {
          if (*Pix==Old_Trans)
            *Pix=CM_Trans;
        }
      }
    }
  }
}



void Afficher_palette_du_menu_en_evitant_la_fenetre(byte * Table)
{
  // On part du principe qu'il n'y a que le bas d'une fenêtre qui puisse
  // empiéter sur la palette... Et c'est déjà pas mal!
  byte Couleur,Vraie_couleur;
  word Debut_X,Debut_Y;
  word Fin_X,Fin_Y;
  word Largeur;
  word Hauteur;
  word Coin_X=Fenetre_Pos_X+Fenetre_Largeur; // |_ Coin bas-droit
  word Coin_Y=Fenetre_Pos_Y+Fenetre_Hauteur; // |  de la fenêtre +1


  if (Config.Couleurs_separees)
  {
    Largeur=(Menu_Taille_couleur-1)*Menu_Facteur_X;
    Hauteur=Menu_Facteur_Y*3;
  }
  else
  {
    Largeur=Menu_Taille_couleur*Menu_Facteur_X;
    Hauteur=Menu_Facteur_Y<<2;
  }

  for (Couleur=0,Vraie_couleur=Couleur_debut_palette;Couleur<64;Couleur++,Vraie_couleur++)
  {
    if (Table[Vraie_couleur]!=Vraie_couleur)
    {
      Debut_X=(LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee_avant_fenetre+((2+((Couleur&7)<<2))*Menu_Facteur_Y);
      Fin_X=Debut_X+Largeur;
      Fin_Y=Debut_Y+Hauteur;

      //   On affiche le bloc en entier si on peut, sinon on le découpe autour
      // de la fenêtre.
      if ( (Debut_Y>=Coin_Y) || (Fin_X<=Fenetre_Pos_X) || (Debut_X>=Coin_X) )
        Block(Debut_X,Debut_Y,Largeur,Hauteur,Vraie_couleur);
      else
      {

        if (Debut_X>=Fenetre_Pos_X)
        {
          if ( (Fin_X>Coin_X) || (Fin_Y>Coin_Y) )
          {
            if ( (Fin_X>Coin_X) && (Fin_Y>Coin_Y) )
            {
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              if (Fin_Y>Coin_Y)
                Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
              else
                Block(Coin_X,Debut_Y,Fin_X-Coin_X,Hauteur,Vraie_couleur);
            }
          }
        }
        else
        {
          if (Fin_X<Coin_X)
          {
            if (Fin_Y>Coin_Y)
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Hauteur,Vraie_couleur);
          }
          else
          {
            if (Fin_Y>Coin_Y)
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Coin_Y-Debut_Y,Vraie_couleur);
              Block(Debut_X,Coin_Y,Largeur,Fin_Y-Coin_Y,Vraie_couleur);
            }
            else
            {
              Block(Debut_X,Debut_Y,Fenetre_Pos_X-Debut_X,Hauteur,Vraie_couleur);
              Block(Coin_X,Debut_Y,Fin_X-Coin_X,Hauteur,Vraie_couleur);
            }
          }
        }
      }
    }
  }
  SDL_UpdateRect(Ecran_SDL,(LARGEUR_MENU+1)*Menu_Facteur_X,Menu_Ordonnee*Menu_Facteur_Y,Largeur,Hauteur);
//    SDL_UpdateRect(Ecran_SDL,LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y);
}


void Remapper_ecran_apres_changement_couleurs_menu(void)
{
  short Indice;
  byte  Table_de_conversion[256];
  short Temp/*,Temp2*/;

  if ( (CM_Clair!=Old_Clair) || (CM_Fonce!=Old_Fonce) || (CM_Blanc!=Old_Blanc) || (CM_Noir !=Old_Noir )
    || (CM_Trans!=Old_Trans) )
  {
    // Création de la table de conversion
    for (Indice=0; Indice<256; Indice++)
      Table_de_conversion[Indice]=Indice;

    Table_de_conversion[Old_Noir ]=CM_Noir;
    Table_de_conversion[Old_Fonce]=CM_Fonce;
    Table_de_conversion[Old_Clair]=CM_Clair;
    Table_de_conversion[Old_Blanc]=CM_Blanc;

    // Remappage de l'écran

    Temp=Fenetre_Hauteur*Menu_Facteur_Y;

    Remap_screen(Fenetre_Pos_X, Fenetre_Pos_Y,
                 Fenetre_Largeur*Menu_Facteur_X,
                 (Fenetre_Pos_Y+Temp<Menu_Ordonnee_avant_fenetre)?Temp:Menu_Ordonnee_avant_fenetre-Fenetre_Pos_Y,
                 Table_de_conversion);

    if (Menu_visible_avant_fenetre)
    {
      Remap_screen(0, Menu_Ordonnee_avant_fenetre,
                   Largeur_ecran, Hauteur_ecran-Menu_Ordonnee_avant_fenetre,
                   Table_de_conversion);
      // On passe la table juste pour ne rafficher que les couleurs modifiées
      Afficher_palette_du_menu_en_evitant_la_fenetre(Table_de_conversion);
    }
    /*
       Il faudrait peut-être remapper les pointillés délimitant l'image.
       Mais ça va être chiant parce qu'ils peuvent être affichés en mode Loupe.
       Mais de toutes façons, c'est franchement facultatif...
    */
  }
}




void Calculer_couleurs_menu_optimales(struct Composantes * Palette)
{
  byte Table[4];
  short I,J,K;


  Old_Noir =CM_Noir;
  Old_Fonce=CM_Fonce;
  Old_Clair=CM_Clair;
  Old_Blanc=CM_Blanc;
  Old_Trans=CM_Trans;

  // Recherche du noir
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[0].R, Coul_menu_pref[0].V, Coul_menu_pref[0].B,Palette,Table);
  CM_Noir=Table[0];

  // Recherche du blanc
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[3].R, Coul_menu_pref[3].V, Coul_menu_pref[3].B,Palette,Table);
  if (CM_Noir!=Table[0])
    CM_Blanc=Table[0];
  else
    CM_Blanc=Table[1];

  // Recherche du gris clair
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[2].R, Coul_menu_pref[2].V, Coul_menu_pref[2].B,Palette,Table);
  if ( (CM_Noir!=Table[0]) && (CM_Blanc!=Table[0]) )
    CM_Clair=Table[0];
  else
  {
    if ( (CM_Noir!=Table[1]) && (CM_Blanc!=Table[1]) )
      CM_Clair=Table[1];
    else
      CM_Clair=Table[2];
  }

  // Recherche du gris foncé
  Calculer_les_4_meilleures_couleurs_pour_1_couleur_du_menu
    (Coul_menu_pref[1].R, Coul_menu_pref[1].V, Coul_menu_pref[1].B,Palette,Table);
  if ( (CM_Noir!=Table[0]) && (CM_Blanc!=Table[0]) && (CM_Clair!=Table[0]) )
    CM_Fonce=Table[0];
  else
  {
    if ( (CM_Noir!=Table[1]) && (CM_Blanc!=Table[1]) && (CM_Clair!=Table[1]) )
      CM_Fonce=Table[1];
    else
    {
      if ( (CM_Noir!=Table[2]) && (CM_Blanc!=Table[2]) && (CM_Clair!=Table[2]) )
        CM_Fonce=Table[2];
      else
        CM_Fonce=Table[3];
    }
  }

  // C'est peu probable mais il est possible que CM_Clair soit plus foncée que
  // CM_Fonce. Dans ce cas, on les inverse.
  if ( ((Palette[CM_Clair].R*30)+(Palette[CM_Clair].V*59)+(Palette[CM_Clair].B*11)) <
       ((Palette[CM_Fonce].R*30)+(Palette[CM_Fonce].V*59)+(Palette[CM_Fonce].B*11)) )
  {
    I=CM_Clair;
    CM_Clair=CM_Fonce;
    CM_Fonce=I;
  }

  // On cherche une couleur de transparence différente des 4 autres.
  for (CM_Trans=0; ((CM_Trans==CM_Noir) || (CM_Trans==CM_Fonce) ||
                   (CM_Trans==CM_Clair) || (CM_Trans==CM_Blanc)); CM_Trans++);

  // Et maintenant, on "remappe" tous les sprites, etc...
  if ( (CM_Clair!=Old_Clair)
    || (CM_Fonce!=Old_Fonce)
    || (CM_Blanc!=Old_Blanc)
    || (CM_Noir !=Old_Noir )
    || (CM_Trans!=Old_Trans) )
  {
      // Sprites du curseur
    for (K=0; K<NB_SPRITES_CURSEUR; K++)
      for (J=0; J<HAUTEUR_SPRITE_CURSEUR; J++)
        for (I=0; I<LARGEUR_SPRITE_CURSEUR; I++)
          Remap_pixel(&SPRITE_CURSEUR[K][J][I]);
      // Le menu
    for (J=0; J<HAUTEUR_MENU; J++)
      for (I=0; I<LARGEUR_MENU; I++)
        Remap_pixel(&BLOCK_MENU[J][I]);
      // Sprites du menu
    for (K=0; K<NB_SPRITES_MENU; K++)
      for (J=0; J<HAUTEUR_SPRITE_MENU; J++)
        for (I=0; I<LARGEUR_SPRITE_MENU; I++)
          Remap_pixel(&SPRITE_MENU[K][J][I]);
      // Sprites d'effets
    for (K=0; K<NB_SPRITES_EFFETS; K++)
      for (J=0; J<HAUTEUR_SPRITE_MENU; J++)
        for (I=0; I<LARGEUR_SPRITE_MENU; I++)
          Remap_pixel(&SPRITE_EFFET[K][J][I]);
      // Fonte de l'aide
    for (K=0; K<315; K++)
      for (J=0; J<8; J++)
        for (I=0; I<6; I++)
          Remap_pixel(&Fonte_help[K][I][J]);
      // Sprites de lecteurs (drives)
    for (K=0; K<NB_SPRITES_DRIVES; K++)
      for (J=0; J<HAUTEUR_SPRITE_DRIVE; J++)
        for (I=0; I<LARGEUR_SPRITE_DRIVE; I++)
          Remap_pixel(&SPRITE_DRIVE[K][J][I]);
  }
}



// Indique quelle est la mémoire disponible
unsigned long Memoire_libre(void)
{
  // On appelle la fonction qui optimise la mémoire libre afin d'en 
  // regagner un maximum. Sinon, tous les "free" libèrent une mémoire qui 
  // n'est pas prise en compte par la fonction, et on se retrouve avec un 
  // manque alarmant de mémoire.
  /*
  A revoir, mais est-ce vraiment utile?
  _heapmin();
  */
    #ifdef __linux__
        struct sysinfo info;
        sysinfo(&info);
        return info.freeram*info.mem_unit;
    #elif __WATCOMC__
         MEMORYSTATUSEX mstt;
         mstt.dwLength = sizeof(MEMORYSTATUSEX);
         GlobalMemoryStatusEx(&mstt);
         return mstt.ullAvailPhys;
    #else
        return 10*1024*1024;
    #endif
}


// Transformer une liste de shade en deux tables de conversion
void Liste2tables(word * Liste,short Pas,byte Mode,byte * Table_inc,byte * Table_dec)
{
  int Indice;
  int Premier;
  int Dernier;
  int Couleur;
  int Temp;


  // On initialise les deux tables de conversion en Identité
  for (Indice=0;Indice<256;Indice++)
  {
    Table_inc[Indice]=Indice;
    Table_dec[Indice]=Indice;
  }

  // On s'apprête à examiner l'ensemble de la liste
  for (Indice=0;Indice<512;Indice++)
  {
    // On recherche la première case de la liste non vide (et non inhibée)
    while ((Indice<512) && (Liste[Indice]>255))
      Indice++;

    // On note la position de la première case de la séquence
    Premier=Indice;

    // On recherche la position de la dernière case de la séquence
    for (Dernier=Premier;Liste[Dernier+1]<256;Dernier++);

    // Pour toutes les cases non vides (et non inhibées) qui suivent
    switch (Mode)
    {
      case MODE_SHADE_NORMAL :
        for (;(Indice<512) && (Liste[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=Liste[Indice];
          Table_inc[Couleur]=Liste[(Indice+Pas<=Dernier)?Indice+Pas:Dernier];
          Table_dec[Couleur]=Liste[(Indice-Pas>=Premier)?Indice-Pas:Premier];
        }
        break;
      case MODE_SHADE_BOUCLE :
        Temp=1+Dernier-Premier;
        for (;(Indice<512) && (Liste[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=Liste[Indice];
          Table_inc[Couleur]=Liste[Premier+((Pas+Indice-Premier)%Temp)];
          Table_dec[Couleur]=Liste[Premier+(((Temp-Pas)+Indice-Premier)%Temp)];
        }
        break;
      default : // MODE_SHADE_NOSAT
        for (;(Indice<512) && (Liste[Indice]<256);Indice++)
        { // On met à jour les tables de conversion
          Couleur=Liste[Indice];
          if (Indice+Pas<=Dernier)
            Table_inc[Couleur]=Liste[Indice+Pas];
          if (Indice-Pas>=Premier)
            Table_dec[Couleur]=Liste[Indice-Pas];
        }
    }
  }
}


// Transformer un nombre (entier naturel) en chaîne
void Num2str(dword Nombre,char * Chaine,byte Taille)
{
  int Indice;

  for (Indice=Taille-1;Indice>=0;Indice--)
  {
    Chaine[Indice]=(Nombre%10)+'0';
    Nombre/=10;
    if (Nombre==0)
      for (Indice--;Indice>=0;Indice--)
        Chaine[Indice]=' ';
  }
  Chaine[Taille]='\0';
}

// Transformer une chaîne en un entier naturel (renvoie -1 si ch. invalide)
int Str2num(char * Chaine)
{
  int Valeur=0;

  for (;*Chaine;Chaine++)
  {
    if ( (*Chaine>='0') && (*Chaine<='9') )
      Valeur=(Valeur*10)+(*Chaine-'0');
    else
      return -1;
  }
  return Valeur;
}


// Arrondir un nombre réel à la valeur entière la plus proche
short Round(float Valeur)
{
  short Temp=Valeur;

  if (Valeur>=0)
    { if ((Valeur-Temp)>= 0.5) Temp++; }
  else
    { if ((Valeur-Temp)<=-0.5) Temp--; }

  return Temp;
}

// Arrondir le résultat d'une division à la valeur entière supérieure
short Round_div_max(short Numerateur,short Diviseur)
{
  if (!(Numerateur % Diviseur))
    return (Numerateur/Diviseur);
  else
    return (Numerateur/Diviseur)+1;
}

// Retourne le minimum entre deux nombres
int Min(int A,int B)
{
  return (A<B)?A:B;
}

// Retourne le maximum entre deux nombres
int Max(int A,int B)
{
  return (A>B)?A:B;
}

void Transformer_point(short X, short Y, float cosA, float sinA,
                       short * Xr, short * Yr)
{
  *Xr=Round(((float)X*cosA)+((float)Y*sinA));
  *Yr=Round(((float)Y*cosA)-((float)X*sinA));
}



// -- Recadrer la partie non-zoomée de l'image par rapport à la partie zoomée
//    lorsqu'on scrolle en mode Loupe --
void Recadrer_ecran_par_rapport_au_zoom(void)
{
  // Centrage en X
  if (Principal_Largeur_image>Principal_Split)
  {
    Principal_Decalage_X=Loupe_Decalage_X+(Loupe_Largeur>>1)
                         -(Principal_Split>>1);
    if (Principal_Decalage_X<0)
      Principal_Decalage_X=0;
    else
    if (Principal_Largeur_image<Principal_Decalage_X+Principal_Split)
      Principal_Decalage_X=Principal_Largeur_image-Principal_Split;
  }
  else
    Principal_Decalage_X=0;

  // Centrage en Y
  if (Principal_Hauteur_image>Menu_Ordonnee)
  {
    Principal_Decalage_Y=Loupe_Decalage_Y+(Loupe_Hauteur>>1)
                         -(Menu_Ordonnee>>1);
    if (Principal_Decalage_Y<0)
      Principal_Decalage_Y=0;
    else
    if (Principal_Hauteur_image<Principal_Decalage_Y+Menu_Ordonnee)
      Principal_Decalage_Y=Principal_Hauteur_image-Menu_Ordonnee;
  }
  else
    Principal_Decalage_Y=0;
}


// - Calcul des données du split en fonction de la proportion de chaque zone -
void Calculer_split(void)
{
  //short Temp;
  short X_theorique=Round(Principal_Proportion_split*Largeur_ecran);

  Principal_X_Zoom=Largeur_ecran-(((Largeur_ecran+(Loupe_Facteur>>1)-X_theorique)/Loupe_Facteur)*Loupe_Facteur);
  Principal_Split=Principal_X_Zoom-(Menu_Facteur_X*LARGEUR_BARRE_SPLIT);

  // Correction en cas de débordement sur la gauche
  while (Principal_Split*(Loupe_Facteur+1)<Largeur_ecran-(Menu_Facteur_X*LARGEUR_BARRE_SPLIT))
  {
    Principal_Split+=Loupe_Facteur;
    Principal_X_Zoom+=Loupe_Facteur;
  }
  // Correction en cas de débordement sur la droite
  X_theorique=Largeur_ecran-((NB_PIXELS_ZOOMES_MIN-1)*Loupe_Facteur);
  while (Principal_X_Zoom>=X_theorique)
  {
    Principal_Split-=Loupe_Facteur;
    Principal_X_Zoom-=Loupe_Facteur;
  }
}



// -------------------- Calcul des information de la loupe -------------------
void Calculer_donnees_loupe(void)
/*
  Après modification des données de la loupe, il faut recalculer les limites.
*/
{
  Calculer_split();

  Loupe_Largeur=(Largeur_ecran-Principal_X_Zoom)/Loupe_Facteur;

  Loupe_Hauteur=Menu_Ordonnee/Loupe_Facteur;
  if (Menu_Ordonnee%Loupe_Facteur)
    Loupe_Hauteur++;

  if (Loupe_Mode && Loupe_Decalage_X)
  {
    if (Principal_Largeur_image<Loupe_Decalage_X+Loupe_Largeur)
      Loupe_Decalage_X=Principal_Largeur_image-Loupe_Largeur;
    if (Loupe_Decalage_X<0) Loupe_Decalage_X=0;
  }
}


// -------- Calcul des bornes de la partie d'image visible à l'écran ---------
void Calculer_limites(void)
/*
  Avant l'appel à cette fonction, les données de la loupe doivent être à jour.
*/
{
  if (Loupe_Mode)
  {
    // -- Calcul des limites de la partie non zoomée de l'image --
    Limite_Haut  =Principal_Decalage_Y;
    Limite_Gauche=Principal_Decalage_X;
    Limite_visible_Bas   =Limite_Haut+Menu_Ordonnee-1;
    Limite_visible_Droite=Limite_Gauche+Principal_Split-1;

    if (Limite_visible_Bas>=Principal_Hauteur_image)
      Limite_Bas=Principal_Hauteur_image-1;
    else
      Limite_Bas=Limite_visible_Bas;

    if (Limite_visible_Droite>=Principal_Largeur_image)
      Limite_Droite=Principal_Largeur_image-1;
    else
      Limite_Droite=Limite_visible_Droite;

    // -- Calcul des limites de la partie zoomée de l'image --
    Limite_Haut_Zoom  =Loupe_Decalage_Y;
    Limite_Gauche_Zoom=Loupe_Decalage_X;
    Limite_visible_Bas_Zoom   =Limite_Haut_Zoom+Loupe_Hauteur-1;
    Limite_visible_Droite_Zoom=Limite_Gauche_Zoom+Loupe_Largeur-1;

    if (Limite_visible_Bas_Zoom>=Principal_Hauteur_image)
      Limite_Bas_Zoom=Principal_Hauteur_image-1;
    else
      Limite_Bas_Zoom=Limite_visible_Bas_Zoom;

    if (Limite_visible_Droite_Zoom>=Principal_Largeur_image)
      Limite_Droite_Zoom=Principal_Largeur_image-1;
    else
      Limite_Droite_Zoom=Limite_visible_Droite_Zoom;
  }
  else
  {
    // -- Calcul des limites de la partie visible de l'image --
    Limite_Haut  =Principal_Decalage_Y;
    Limite_Gauche=Principal_Decalage_X;
    Limite_visible_Bas   =Limite_Haut+(Menu_visible?Menu_Ordonnee:Hauteur_ecran)-1; // A REVOIR POUR SIMPLIFICATION
    Limite_visible_Droite=Limite_Gauche+Largeur_ecran-1;

    if (Limite_visible_Bas>=Principal_Hauteur_image)
      Limite_Bas=Principal_Hauteur_image-1;
    else
      Limite_Bas=Limite_visible_Bas;

    if (Limite_visible_Droite>=Principal_Largeur_image)
      Limite_Droite=Principal_Largeur_image-1;
    else
      Limite_Droite=Limite_visible_Droite;
  }
}


// -- Calculer les coordonnées du pinceau en fonction du snap et de la loupe -
void Calculer_coordonnees_pinceau(void)
{
  if ((Loupe_Mode) && (Mouse_X>=Principal_X_Zoom))
  {
    Pinceau_X=((Mouse_X-Principal_X_Zoom)/Loupe_Facteur)+Loupe_Decalage_X;
    Pinceau_Y=(Mouse_Y/Loupe_Facteur)+Loupe_Decalage_Y;
  }
  else
  {
    Pinceau_X=Mouse_X+Principal_Decalage_X;
    Pinceau_Y=Mouse_Y+Principal_Decalage_Y;
  }

  if (Snap_Mode)
  {
    Pinceau_X=(((Pinceau_X+(Snap_Largeur>>1)-Snap_Decalage_X)/Snap_Largeur)*Snap_Largeur)+Snap_Decalage_X;
    Pinceau_Y=(((Pinceau_Y+(Snap_Hauteur>>1)-Snap_Decalage_Y)/Snap_Hauteur)*Snap_Hauteur)+Snap_Decalage_Y;
  }
}


// ------------ Changer le facteur de zoom et tout mettre à jour -------------
void Changer_facteur_loupe(byte Indice_facteur)
{
  short Centre_X;
  short Centre_Y;

  Centre_X=Loupe_Decalage_X+(Loupe_Largeur>>1);
  Centre_Y=Loupe_Decalage_Y+(Loupe_Hauteur>>1);

  Loupe_Facteur=FACTEUR_ZOOM[Indice_facteur];
  Table_mul_facteur_zoom=TABLE_ZOOM[Indice_facteur];
  Calculer_donnees_loupe();

  if (Loupe_Mode)
  {
    // Recalculer le décalage de la loupe
    // Centrage "brut" de lécran par rapport à la loupe
    Loupe_Decalage_X=Centre_X-(Loupe_Largeur>>1);
    Loupe_Decalage_Y=Centre_Y-(Loupe_Hauteur>>1);
    // Correction en cas de débordement de l'image
    if (Loupe_Decalage_X+Loupe_Largeur>Principal_Largeur_image)
      Loupe_Decalage_X=Principal_Largeur_image-Loupe_Largeur;
    if (Loupe_Decalage_Y+Loupe_Hauteur>Principal_Hauteur_image)
      Loupe_Decalage_Y=Principal_Hauteur_image-Loupe_Hauteur;
    if (Loupe_Decalage_X<0)
      Loupe_Decalage_X=0;
    if (Loupe_Decalage_Y<0)
      Loupe_Decalage_Y=0;

    Recadrer_ecran_par_rapport_au_zoom();

    Pixel_Preview=Pixel_Preview_Loupe;
  }
  else
    Pixel_Preview=Pixel_Preview_Normal;

  Calculer_limites();
  Calculer_coordonnees_pinceau();
}


// -- Affichage de la limite de l'image -------------------------------------
void Afficher_limites_de_l_image(void)
{
  short Debut;
  short Pos;
  short Fin;
  byte Droite_visible;
  byte Bas_visible;
  short Ancienne_Limite_Zoom;

  Droite_visible=Principal_Largeur_image<((Loupe_Mode)?Principal_Split:Largeur_ecran);
  Bas_visible   =Principal_Hauteur_image<Menu_Ordonnee;


  // On vérifie que la limite à droite est visible:
  if (Droite_visible)
  {
    Debut=Limite_Haut;
    Fin=(Limite_Bas<Principal_Hauteur_image)?
        Limite_Bas:Principal_Hauteur_image;

    if (Bas_visible)
      Fin++;

    // Juste le temps d'afficher les limites, on étend les limites de la loupe
    // aux limites visibles, car sinon Pixel_Preview ne voudra pas afficher.
    Ancienne_Limite_Zoom=Limite_Droite_Zoom;
    Limite_Droite_Zoom=Limite_visible_Droite_Zoom;

    for (Pos=Debut;Pos<=Fin;Pos++)
      Pixel_Preview(Principal_Largeur_image,Pos,((Pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    SDL_UpdateRect(Ecran_SDL,Principal_Largeur_image,Debut,1,Fin-Debut + 1);

    // On restaure la bonne valeur des limites
    Limite_Droite_Zoom=Ancienne_Limite_Zoom;
  }

  // On vérifie que la limite en bas est visible:
  if (Bas_visible)
  {
    Debut=Limite_Gauche;
    Fin=(Limite_Droite<Principal_Largeur_image)?
        Limite_Droite:Principal_Largeur_image;

    // On étend également les limites en bas (comme pour la limite droit)
    Ancienne_Limite_Zoom=Limite_Bas_Zoom;
    Limite_Bas_Zoom=Limite_visible_Bas_Zoom;

    for (Pos=Debut;Pos<=Fin;Pos++)
      Pixel_Preview(Pos,Principal_Hauteur_image,((Pos+Principal_Hauteur_image)&1)?CM_Blanc:CM_Noir);

    SDL_UpdateRect(Ecran_SDL,Debut,Principal_Hauteur_image,Fin-Debut + 1,1);

    // On restaure la bonne valeur des limites
    Limite_Bas_Zoom=Ancienne_Limite_Zoom;
  }
}



// Fonction retournant le libellé d'une mode (ex: " 320x200")
char * Libelle_mode(int Mode)
{
  static char Chaine[24];
  if (! Mode_video[Mode].Fullscreen)
    return "window";
  sprintf(Chaine, "%dx%d", Mode_video[Mode].Largeur, Mode_video[Mode].Hauteur);

  return Chaine;
}

// Trouve un mode video à partir d'une chaine: soit "window",
// soit de la forme "320x200"
// Renvoie -1 si la chaine n'est pas convertible
int Conversion_argument_mode(const char *Argument)
{
  // Je suis paresseux alors je vais juste tester les libellés
  int Indice_mode;
  for (Indice_mode=0; Indice_mode<Nb_modes_video; Indice_mode++)
    if (!strcmp(Libelle_mode(Indice_mode), Argument))
      return Indice_mode;

  return -1;
}


//--------------------- Initialisation d'un mode vidéo -----------------------

void * Mode_X_Ptr; // Pointeur sur la table à utiliser pour le changement de
                   // mode vidéo X

void Initialiser_mode_video(int Largeur, int Hauteur, int Fullscreen)
{
  int Sensibilite_X;
  int Sensibilite_Y;
  int Indice;
  
  if (Largeur_ecran!=Largeur ||
      Hauteur_ecran!=Hauteur ||
      Mode_video[Resolution_actuelle].Fullscreen != Fullscreen)
  {
    // Valeurs raisonnables: minimum 320x200
    if (Largeur < 320)
      Largeur = 320;
    if (Hauteur < 200)
      Hauteur = 200;
    // La largeur doit être un multiple de 4
    Largeur = (Largeur + 3 ) & 0xFFFFFFFC;

    // Taille des menus
    int Facteur;
    if (Largeur/320 > Hauteur/200)
      Facteur=Hauteur/200;
    else
      Facteur=Largeur/320;
  
    Largeur_ecran = Largeur;
    Hauteur_ecran = Hauteur;
    Plein_ecran   = Fullscreen;

    switch (Config.Ratio)
    {
      case 1: // adapter tout
        Menu_Facteur_X=Facteur;
        Menu_Facteur_Y=Facteur;
        break;
      case 2: // adapter légèrement
        Menu_Facteur_X=Facteur-1;
        if (Menu_Facteur_X<1) Menu_Facteur_X=1;
        Menu_Facteur_Y=Facteur-1;
        if (Menu_Facteur_Y<1) Menu_Facteur_Y=1;
        break;
      default: // ne pas adapter
        Menu_Facteur_X=1;
        Menu_Facteur_Y=1;
    }

    if (Buffer_de_ligne_horizontale)
      free(Buffer_de_ligne_horizontale);

    Buffer_de_ligne_horizontale=(byte *)malloc((Largeur_ecran>Principal_Largeur_image)?Largeur_ecran:Principal_Largeur_image);

    switch (MODE_SDL)
    {
        case MODE_SDL:
            Pixel = Pixel_SDL;
            Lit_pixel= Lit_Pixel_SDL;
            Clear_screen = Effacer_Tout_l_Ecran_SDL;
            Display_screen = Afficher_partie_de_l_ecran_SDL;
            Block = Block_SDL;
            Pixel_Preview_Normal = Pixel_Preview_Normal_SDL;
            Pixel_Preview_Loupe = Pixel_Preview_Loupe_SDL;
            Ligne_horizontale_XOR = Ligne_horizontale_XOR_SDL;
            Ligne_verticale_XOR = Ligne_verticale_XOR_SDL;
            Display_brush_Color = Display_brush_Color_SDL;
            Display_brush_Mono = Display_brush_Mono_SDL;
            Clear_brush = Clear_brush_SDL;
            Remap_screen = Remap_screen_SDL;
            Afficher_ligne = Afficher_une_ligne_ecran_SDL;
            Lire_ligne = Lire_une_ligne_ecran_SDL;
            Display_zoomed_screen = Afficher_partie_de_l_ecran_zoomee_SDL;
            Display_brush_Color_zoom = Display_brush_Color_zoom_SDL;
            Display_brush_Mono_zoom = Display_brush_Mono_zoom_SDL;
            Clear_brush_zoom = Clear_brush_zoom_SDL;
            Set_Mode_SDL();
        break;
    }
    
    Set_palette(Principal_Palette);

    if (!Fullscreen)
      Resolution_actuelle=0;
    else
      for (Indice=1; Indice<Nb_modes_video; Indice++)
      {
        if (Mode_video[Indice].Largeur==Largeur_ecran &&
            Mode_video[Indice].Hauteur==Hauteur_ecran)
        {
          Resolution_actuelle=Indice;
          break;
        }
      }

    Menu_Taille_couleur = ((Largeur_ecran/Menu_Facteur_X)-(LARGEUR_MENU+2)) >> 3;
    Menu_Ordonnee = Hauteur_ecran;
    if (Menu_visible)
      Menu_Ordonnee -= HAUTEUR_MENU * Menu_Facteur_Y;
    Menu_Ordonnee_Texte = Hauteur_ecran-(Menu_Facteur_Y<<3);
    Bouton[BOUTON_CHOIX_COL].Largeur=(Menu_Taille_couleur<<3)-1;

    Sensibilite_X = Config.Indice_Sensibilite_souris_X;
    Sensibilite_Y = Config.Indice_Sensibilite_souris_Y;
    Sensibilite_X>>=Mouse_Facteur_de_correction_X;
    Sensibilite_Y>>=Mouse_Facteur_de_correction_Y;
    Sensibilite_souris(Sensibilite_X?Sensibilite_X:1,Sensibilite_Y?Sensibilite_Y:1);

    Brouillon_Decalage_X=0; // |  Il faut penser à éviter les incohérences
    Brouillon_Decalage_Y=0; // |- de décalage du brouillon par rapport à
    Brouillon_Loupe_Mode=0; // |  la résolution.
  }

  Pixel_Preview=Pixel_Preview_Normal;

  Principal_Decalage_X=0; // Il faut quand même modifier ces valeurs à chaque
  Principal_Decalage_Y=0; // fois car on n'est pas à l'abri d'une modification
                          // des dimensions de l'image.
  Calculer_donnees_loupe();
  Calculer_limites();
  Calculer_coordonnees_pinceau();
  
  Resize_Largeur=0;
  Resize_Hauteur=0;
}


// -- Interface avec l'image, affectée par le facteur de grossissement -------

  // fonction d'affichage "Pixel" utilisée pour les opérations définitivement
  // Ne doit à aucune condition être appelée en dehors de la partie visible
  // de l'image dans l'écran (ça pourrait être grave)
void Afficher_pixel(word X,word Y,byte Couleur)
  // X & Y    sont la position d'un point dans l'IMAGE
  // Couleur  est la couleur du point
  // Le Stencil est géré.
  // Les effets sont gérés par appel à Fonction_effet().
  // La Loupe est gérée par appel à Pixel_Preview().
{
  if ( ( (!Trame_Mode)   || (Effet_Trame(X,Y)) )
    && (!((Stencil_Mode) && (Stencil[Lit_pixel_dans_ecran_courant(X,Y)])))
    && (!((Mask_Mode)    && (Mask[Lit_pixel_dans_ecran_brouillon(X,Y)]))) )
  {
    Couleur=Fonction_effet(X,Y,Couleur);
    Pixel_dans_ecran_courant(X,Y,Couleur);
    Pixel_Preview(X,Y,Couleur);
  }
}


// -- Interface avec le menu et les fenêtres ---------------------------------

  // Affichage d'un pixel dans le menu (le menu doît être visible)

void Pixel_dans_barre_d_outil(word X,word Y,byte Couleur)
{
  Block(X*Menu_Facteur_X,(Y*Menu_Facteur_Y)+Menu_Ordonnee,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}

  // Affichage d'un pixel dans la fenêtre (la fenêtre doît être visible)

void Pixel_dans_fenetre(word X,word Y,byte Couleur)
{
    Block((X*Menu_Facteur_X)+Fenetre_Pos_X,(Y*Menu_Facteur_Y)+Fenetre_Pos_Y,Menu_Facteur_X,Menu_Facteur_Y,Couleur);
}


// -- Affichages de différents cadres dans une fenêtre -----------------------

  // -- Cadre général avec couleurs paramètrables --

void Fenetre_Afficher_cadre_general(word Pos_X,word Pos_Y,word Largeur,word Hauteur,
                                    byte Couleur_HG,byte Couleur_BD,byte Couleur_S,byte Couleur_CHG,byte Couleur_CBD)
// Paramètres de couleurs:
// Couleur_HG =Bords Haut et Gauche
// Couleur_BD =Bords Bas et Droite
// Couleur_S  =Coins Haut-Droite et Bas-Gauche
// Couleur_CHG=Coin Haut-Gauche
// Couleur_CBD=Coin Bas-Droite
{
  // Bord haut (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+(Pos_Y*Menu_Facteur_Y),
        (Largeur-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_HG);

  // Bord bas (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+Hauteur-1)*Menu_Facteur_Y),
        (Largeur-2)*Menu_Facteur_X,Menu_Facteur_Y,Couleur_BD);

  // Bord gauche (sans les extrémités)
  Block(Fenetre_Pos_X+(Pos_X*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(Hauteur-2)*Menu_Facteur_Y,Couleur_HG);

  // Bord droite (sans les extrémités)
  Block(Fenetre_Pos_X+((Pos_X+Largeur-1)*Menu_Facteur_X),
        Fenetre_Pos_Y+((Pos_Y+1)*Menu_Facteur_Y),
        Menu_Facteur_X,(Hauteur-2)*Menu_Facteur_Y,Couleur_BD);

  // Coin haut gauche
  Pixel_dans_fenetre(Pos_X,Pos_Y,Couleur_CHG);
  // Coin haut droite
  Pixel_dans_fenetre(Pos_X+Largeur-1,Pos_Y,Couleur_S);
  // Coin bas droite
  Pixel_dans_fenetre(Pos_X+Largeur-1,Pos_Y+Hauteur-1,Couleur_CBD);
  // Coin bas gauche
  Pixel_dans_fenetre(Pos_X,Pos_Y+Hauteur-1,Couleur_S);
}

  // -- Cadre dont tout le contour est d'une seule couleur --

void Fenetre_Afficher_cadre_mono(word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte Couleur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,Couleur,Couleur,Couleur,Couleur,Couleur);
}

  // -- Cadre creux: foncé en haut-gauche et clair en bas-droite --

void Fenetre_Afficher_cadre_creux(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,CM_Fonce,CM_Blanc,CM_Clair,CM_Fonce,CM_Blanc);
}

  // -- Cadre bombé: clair en haut-gauche et foncé en bas-droite --

void Fenetre_Afficher_cadre_bombe(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_general(Pos_X,Pos_Y,Largeur,Hauteur,CM_Blanc,CM_Fonce,CM_Clair,CM_Blanc,CM_Fonce);
}

  // -- Cadre de séparation: un cadre bombé dans un cadre creux (3D!!!) --

void Fenetre_Afficher_cadre(word Pos_X,word Pos_Y,word Largeur,word Hauteur)
{
  Fenetre_Afficher_cadre_creux(Pos_X,Pos_Y,Largeur,Hauteur);
  Fenetre_Afficher_cadre_bombe(Pos_X+1,Pos_Y+1,Largeur-2,Hauteur-2);
}


//-- Affichages relatifs à la palette dans le menu ---------------------------

  // -- Affichage des couleurs courante (fore/back) de pinceau dans le menu --

void Afficher_foreback(void)
{
  if (Menu_visible)
  {
    Block((LARGEUR_MENU-17)*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y,Menu_Facteur_X<<4,Menu_Facteur_Y*7,Back_color);
    Block((LARGEUR_MENU-13)*Menu_Facteur_X,Menu_Ordonnee+(Menu_Facteur_Y<<1),Menu_Facteur_X<<3,Menu_Facteur_Y*5,Fore_color);

    SDL_UpdateRect(Ecran_SDL,(LARGEUR_MENU-17)*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y,Menu_Facteur_X<<4,Menu_Facteur_Y*7);
  }
}

  // -- Tracer un cadre de couleur autour de la Fore_color dans le menu --

void Encadrer_couleur_menu(byte Couleur)
{
  word Debut_X,Debut_Y,Fin_X,Fin_Y;
  word Indice;

  if ((Fore_color>=Couleur_debut_palette) && (Fore_color<Couleur_debut_palette+64) && (Menu_visible))
  {
    if (Config.Couleurs_separees)
    {
      Debut_X=(LARGEUR_MENU+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
      Debut_Y=Menu_Ordonnee+((1+(((Fore_color-Couleur_debut_palette)&7)<<2))*Menu_Facteur_Y);

      Block(Debut_X,Debut_Y,(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);
      Block(Debut_X,Debut_Y+(Menu_Facteur_Y<<2),(Menu_Taille_couleur+1)*Menu_Facteur_X,Menu_Facteur_Y,Couleur);

      Block(Debut_X,Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*3,Couleur);
      Block(Debut_X+(Menu_Taille_couleur*Menu_Facteur_X),Debut_Y+Menu_Facteur_Y,Menu_Facteur_X,Menu_Facteur_Y*3,Couleur);
    }
    else
    {
      if (Couleur==CM_Noir)
      {
        Debut_X=(LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur)*Menu_Facteur_X;
        Debut_Y=Menu_Ordonnee+((2+(((Fore_color-Couleur_debut_palette)&7)<<2))*Menu_Facteur_Y);

        Block(Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y<<2,Fore_color);

        SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Menu_Taille_couleur*Menu_Facteur_X,Menu_Facteur_Y*4); // TODO On met à jour toute la palette... peut mieux faire
      }
      else
      {
        Debut_X=LARGEUR_MENU+1+((Fore_color-Couleur_debut_palette)>>3)*Menu_Taille_couleur;
        Debut_Y=2+(((Fore_color-Couleur_debut_palette)&7)<<2);

        Fin_X=Debut_X+Menu_Taille_couleur-1;
        Fin_Y=Debut_Y+3;

        for (Indice=Debut_X; Indice<=Fin_X; Indice++)
          Block(Indice*Menu_Facteur_X,Menu_Ordonnee+(Debut_Y*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Debut_Y)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_Y+1; Indice<Fin_Y; Indice++)
          Block(Debut_X*Menu_Facteur_X,Menu_Ordonnee+(Indice*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Debut_X)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_Y+1; Indice<Fin_Y; Indice++)
          Block(Fin_X*Menu_Facteur_X,Menu_Ordonnee+(Indice*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Fin_X)&1)?CM_Blanc:CM_Noir);

        for (Indice=Debut_X; Indice<=Fin_X; Indice++)
          Block(Indice*Menu_Facteur_X,Menu_Ordonnee+(Fin_Y*Menu_Facteur_Y),
                Menu_Facteur_X,Menu_Facteur_Y,
                ((Indice+Fin_Y)&1)?CM_Blanc:CM_Noir);

        SDL_UpdateRect(Ecran_SDL,Debut_X*Menu_Facteur_X,Debut_Y*Menu_Facteur_X,Menu_Taille_couleur*Menu_Facteur_X,Menu_Ordonnee+Menu_Facteur_Y*4);
      }
    }
  }
}

  // -- Afficher la palette dans le menu --

void Afficher_palette_du_menu(void)
{
  byte Couleur;

  if (Menu_visible)
  {
    Block(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y,CM_Noir);

    if (Config.Couleurs_separees)
      for (Couleur=0;Couleur<64;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur&7)<<2))*Menu_Facteur_Y),
              (Menu_Taille_couleur-1)*Menu_Facteur_X,
              Menu_Facteur_Y*3,
              Couleur_debut_palette+Couleur);
    else
      for (Couleur=0;Couleur<64;Couleur++)
        Block((LARGEUR_MENU+1+(Couleur>>3)*Menu_Taille_couleur)*Menu_Facteur_X,
              Menu_Ordonnee+((2+((Couleur&7)<<2))*Menu_Facteur_Y),
              Menu_Taille_couleur*Menu_Facteur_X,
              Menu_Facteur_Y<<2,
              Couleur_debut_palette+Couleur);

    Encadrer_couleur_menu(CM_Blanc);
    SDL_UpdateRect(Ecran_SDL,LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),(HAUTEUR_MENU-9)*Menu_Facteur_Y);
  }
}

  // -- Recalculer l'origine de la palette dans le menu pour rendre la
  //    Fore_color visible --

void Recadrer_palette(void)
{
  byte Ancienne_couleur=Couleur_debut_palette;

  if (Fore_color<Couleur_debut_palette)
  {
    while (Fore_color<Couleur_debut_palette)
      Couleur_debut_palette-=8;
  }
  else
  {
    while (Fore_color>=Couleur_debut_palette+64)
      Couleur_debut_palette+=8;
  }
  if (Ancienne_couleur!=Couleur_debut_palette)
    Afficher_palette_du_menu();
}


  // -- Afficher la barre de séparation entre les parties zoomées ou non en
  //    mode Loupe --

void Afficher_barre_de_split(void)
{
  // Partie grise du milieu
  Block(Principal_Split+(Menu_Facteur_X<<1),Menu_Facteur_Y,
        (LARGEUR_BARRE_SPLIT-4)*Menu_Facteur_X,
        Menu_Ordonnee-(Menu_Facteur_Y<<1),CM_Clair);

  // Barre noire de gauche
  Block(Principal_Split,0,Menu_Facteur_X,Menu_Ordonnee,CM_Noir);

  // Barre noire de droite
  Block(Principal_X_Zoom-Menu_Facteur_X,0,Menu_Facteur_X,Menu_Ordonnee,CM_Noir);

  // Bord haut (blanc)
  Block(Principal_Split+Menu_Facteur_X,0,
        (LARGEUR_BARRE_SPLIT-3)*Menu_Facteur_X,Menu_Facteur_Y,CM_Blanc);

  // Bord gauche (blanc)
  Block(Principal_Split+Menu_Facteur_X,Menu_Facteur_Y,
        Menu_Facteur_X,(Menu_Ordonnee-(Menu_Facteur_Y<<1)),CM_Blanc);

  // Bord droite (gris foncé)
  Block(Principal_X_Zoom-(Menu_Facteur_X<<1),Menu_Facteur_Y,
        Menu_Facteur_X,(Menu_Ordonnee-(Menu_Facteur_Y<<1)),CM_Fonce);

  // Bord bas (gris foncé)
  Block(Principal_Split+(Menu_Facteur_X<<1),Menu_Ordonnee-Menu_Facteur_Y,
        (LARGEUR_BARRE_SPLIT-3)*Menu_Facteur_X,Menu_Facteur_Y,CM_Fonce);

  // Coin bas gauche
  Block(Principal_Split+Menu_Facteur_X,Menu_Ordonnee-Menu_Facteur_Y,
        Menu_Facteur_X,Menu_Facteur_Y,CM_Clair);
  // Coin haut droite
  Block(Principal_X_Zoom-(Menu_Facteur_X<<1),0,
        Menu_Facteur_X,Menu_Facteur_Y,CM_Clair);

  SDL_UpdateRect(Ecran_SDL,Principal_Split,0,LARGEUR_BARRE_SPLIT*Menu_Facteur_X,Menu_Ordonnee); // On réaffiche toute la partie à gauche du split, ce qui permet d'effacer son ancienne position
}

  // -- Afficher tout le menu --

void Afficher_menu(void)
{
  word Pos_X;
  word Pos_Y;
  char Chaine[4];


  if (Menu_visible)
  {
    // Affichage du sprite du menu
    for (Pos_Y=0;Pos_Y<HAUTEUR_MENU;Pos_Y++)
      for (Pos_X=0;Pos_X<LARGEUR_MENU;Pos_X++)
        Pixel_dans_menu(Pos_X,Pos_Y,BLOCK_MENU[Pos_Y][Pos_X]);
    // Affichage de la bande grise sous la palette
    Block(LARGEUR_MENU*Menu_Facteur_X,Menu_Ordonnee_Texte-Menu_Facteur_Y,Largeur_ecran-(LARGEUR_MENU*Menu_Facteur_X),9*Menu_Facteur_Y,CM_Clair);

    // Affichage de la palette
    Afficher_palette_du_menu();

    // Affichage des couleurs de travail
    Afficher_foreback();

    if (!Une_fenetre_est_ouverte)
    {
      if ((Mouse_Y<Menu_Ordonnee) &&
          ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ))
      {
        if ( (Operation_en_cours!=OPERATION_PIPETTE)
          && (Operation_en_cours!=OPERATION_REMPLACER) )
          Print_dans_menu("X:       Y:             ",0);
        else
        {
          Print_dans_menu("X:       Y:       (    )",0);
          Num2str(Pipette_Couleur,Chaine,3);
          Print_dans_menu(Chaine,20);
          Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
        }
        Print_coordonnees();
      }
      Print_nom_fichier();
    }
    SDL_UpdateRect(Ecran_SDL,0,Menu_Ordonnee,LARGEUR_MENU*Menu_Facteur_X,HAUTEUR_MENU*Menu_Facteur_Y);
  }
}

// Table de conversion ANSI->OEM
// Les deux fontes générales sont en encodage OEM
unsigned char Caractere_OEM[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
 63, 63, 39, 159, 34, 46, 197, 206, 94, 37, 83, 60, 79, 63, 90, 63, 
 63, 39, 39, 34, 34, 7, 45, 45, 126, 84, 115, 62, 111, 63, 122, 89, 
 255, 173, 189, 156, 207, 190, 221, 245, 249, 184, 166, 174, 170, 240, 169, 238, 
 248, 241, 253, 252, 239, 230, 244, 250, 247, 251, 167, 175, 172, 171, 243, 168, 
 183, 181, 182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222, 214, 215, 216, 
 209, 165, 227, 224, 226, 229, 153, 158, 157, 235, 233, 234, 154, 237, 232, 225, 
 133, 160, 131, 198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139, 
 208, 164, 149, 162, 147, 228, 148, 246, 155, 151, 163, 150, 129, 236, 231}; 

// -- Affichage de texte -----------------------------------------------------

  // -- Afficher une chaîne n'importe où à l'écran --

void Print_general(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  word  Indice;
  short Pos_X;
  short Pos_Y;
  unsigned char Caractere;
  short Reel_X;
  short Reel_Y;
  short Largeur;
  short Repeat_Menu_Facteur_X;
  short Repeat_Menu_Facteur_Y;

  Reel_Y=Y;
  Largeur=strlen(Chaine)*Menu_Facteur_X*8;
  for (Pos_Y=0;Pos_Y<8;Pos_Y++)
  {
    Reel_X=0; // Position dans le buffer
    for (Indice=0;Chaine[Indice]!='\0';Indice++)
    {
      Caractere=Chaine[Indice];
      for (Pos_X=0;Pos_X<8;Pos_X++)
        for (Repeat_Menu_Facteur_X=0;Repeat_Menu_Facteur_X<Menu_Facteur_X;Repeat_Menu_Facteur_X++)
          Buffer_de_ligne_horizontale[Reel_X++]=(*(Fonte+(*(Caractere_OEM+Caractere)<<6)+(Pos_X<<3)+Pos_Y)?Couleur_texte:Couleur_fond);
    }
    for (Repeat_Menu_Facteur_Y=0;Repeat_Menu_Facteur_Y<Menu_Facteur_Y;Repeat_Menu_Facteur_Y++)
      Afficher_ligne(X,Reel_Y++,Largeur,Buffer_de_ligne_horizontale);
  }
  SDL_UpdateRect(Ecran_SDL,X,Y,Largeur,8*Menu_Facteur_Y); // TODO: pas utile dans Print_Dans_Fenetre, donc voir ou on en a vraiment besoin...
}

  // -- Afficher un caractère dans une fenêtre --

void Print_char_dans_fenetre(short Pos_X,short Pos_Y,unsigned char Caractere,byte Couleur_texte,byte Couleur_fond)
{
  short X,Y;
  Pos_X=(Pos_X*Menu_Facteur_X)+Fenetre_Pos_X;
  Pos_Y=(Pos_Y*Menu_Facteur_Y)+Fenetre_Pos_Y;

  for (X=0;X<8;X++)
    for (Y=0;Y<8;Y++)
      Block(Pos_X+(X*Menu_Facteur_X), Pos_Y+(Y*Menu_Facteur_Y),
            Menu_Facteur_X, Menu_Facteur_Y,
            (*(Fonte+(*(Caractere_OEM+Caractere)<<6)+(X<<3)+Y)?Couleur_texte:Couleur_fond));
}

  // -- Afficher un caractère sans fond dans une fenêtre --

void Print_char_transparent_dans_fenetre(short Pos_X,short Pos_Y,unsigned char Caractere,byte Couleur)
{
  short X,Y;

  Pos_X=(Pos_X*Menu_Facteur_X)+Fenetre_Pos_X;
  Pos_Y=(Pos_Y*Menu_Facteur_Y)+Fenetre_Pos_Y;

  for (X=0;X<8;X++)
    for (Y=0;Y<8;Y++)
    {
      if (*(Fonte+(*(Caractere_OEM+Caractere)<<6)+(X<<3)+Y))
        Block(Pos_X+(X*Menu_Facteur_X), Pos_Y+(Y*Menu_Facteur_Y),
              Menu_Facteur_X, Menu_Facteur_Y, Couleur);
    }
}

  // -- Afficher une chaîne dans une fenêtre, avec taille maxi --

void Print_dans_fenetre_limite(short X,short Y,char * Chaine,byte Taille,byte Couleur_texte,byte Couleur_fond)
{
  char Chaine_affichee[256];
  strncpy(Chaine_affichee, Chaine, Taille);
  Chaine_affichee[255]='\0';
  
  if (strlen(Chaine_affichee) > Taille)
  {
    Chaine_affichee[Taille-1]=CARACTERE_TRIANGLE_DROIT;
    Chaine_affichee[Taille]='\0';
  }
  Print_dans_fenetre(X, Y, Chaine_affichee, Couleur_texte, Couleur_fond);
}

  // -- Afficher une chaîne dans une fenêtre --

void Print_dans_fenetre(short X,short Y,char * Chaine,byte Couleur_texte,byte Couleur_fond)
{
  Print_general((X*Menu_Facteur_X)+Fenetre_Pos_X,
                (Y*Menu_Facteur_Y)+Fenetre_Pos_Y,
                Chaine,Couleur_texte,Couleur_fond);
}

  // -- Afficher une chaîne dans le menu --

void Print_dans_menu(char * Chaine, short Position)
{
  Print_general((18+(Position<<3))*Menu_Facteur_X,Menu_Ordonnee_Texte,Chaine,CM_Noir,CM_Clair);
}

  // -- Afficher les coordonnées du pinceau dans le menu --

void Print_coordonnees(void)
{
  char Tempo[5];

  if (Menu_visible)
  {
    if ( (Operation_en_cours==OPERATION_PIPETTE)
      || (Operation_en_cours==OPERATION_REMPLACER) )
    {
      if ( (Pinceau_X>=0) && (Pinceau_Y>=0)
        && (Pinceau_X<Principal_Largeur_image)
        && (Pinceau_Y<Principal_Hauteur_image) )
        Pipette_Couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
      else
        Pipette_Couleur=0;
      Pipette_X=Pinceau_X;
      Pipette_Y=Pinceau_Y;

      Num2str(Pipette_Couleur,Tempo,3);
      Print_dans_menu(Tempo,20);
      Print_general(170*Menu_Facteur_X,Menu_Ordonnee_Texte," ",0,Pipette_Couleur);
    }

    Num2str((dword)Pinceau_X,Tempo,4);
    Print_dans_menu(Tempo,2);
    Num2str((dword)Pinceau_Y,Tempo,4);
    Print_dans_menu(Tempo,11);
  }
}

  // -- Afficher le nom du fichier dans le menu --

void Print_nom_fichier(void)
{
  short Debut_X;
  
  if (Menu_visible)
  {
    // Si le nom de fichier fait plus de 12 caractères, on n'affiche que les 12 derniers
    char * Nom_affiche = Principal_Nom_fichier;
    int Taille_nom =strlen(Principal_Nom_fichier);
    if (Taille_nom>12)
    {
      Nom_affiche=Principal_Nom_fichier + Taille_nom - 12;
      Taille_nom = 12;
    }
    
    Block((LARGEUR_MENU+2+((Menu_Taille_couleur-12)<<3))*Menu_Facteur_X,
          Menu_Ordonnee_Texte,Menu_Facteur_X*96,Menu_Facteur_Y<<3,CM_Clair);

    Debut_X=LARGEUR_MENU+2+((Menu_Taille_couleur-Taille_nom)<<3);

    Print_general(Debut_X*Menu_Facteur_X,Menu_Ordonnee_Texte,Nom_affiche,CM_Noir,CM_Clair);
  }
}


// -- Calcul des différents effets -------------------------------------------

  // -- Aucun effet en cours --

byte Aucun_effet(word X,word Y,byte Couleur)
{
  return Couleur;
}

  // -- Effet de Shading --

byte Effet_Shade(word X,word Y,byte Couleur)
{
  return Shade_Table[Lit_pixel_dans_ecran_feedback(X,Y)];
}

byte Effet_Quick_shade(word X,word Y,byte Couleur)
{
  int C=Couleur=Lit_pixel_dans_ecran_feedback(X,Y);
  int Sens=(Fore_color<=Back_color);
  byte Debut,Fin;
  int Largeur;

  if (Sens)
  {
    Debut=Fore_color;
    Fin  =Back_color;
  }
  else
  {
    Debut=Back_color;
    Fin  =Fore_color;
  }

  if ((C>=Debut) && (C<=Fin) && (Debut!=Fin))
  {
    Largeur=1+Fin-Debut;

    if ( ((Mouse_K==A_GAUCHE) && Sens) || ((Mouse_K==A_DROITE) && (!Sens)) )
      C-=Quick_shade_Step%Largeur;
    else
      C+=Quick_shade_Step%Largeur;

    if (C<Debut)
      switch (Quick_shade_Loop)
      {
        case MODE_SHADE_NORMAL : return Debut;
        case MODE_SHADE_BOUCLE : return (Largeur+C);
        default : return Couleur;
      }

    if (C>Fin)
      switch (Quick_shade_Loop)
      {
        case MODE_SHADE_NORMAL : return Fin;
        case MODE_SHADE_BOUCLE : return (C-Largeur);
        default : return Couleur;
      }
  }

  return C;
}

  // -- Effet de Tiling --

byte Effet_Tiling(word X,word Y,byte Couleur)
{
  return Lit_pixel_dans_brosse((X+Brosse_Largeur-Tiling_Decalage_X)%Brosse_Largeur,
                               (Y+Brosse_Hauteur-Tiling_Decalage_Y)%Brosse_Hauteur);
}

  // -- Effet de Smooth --

byte Effet_Smooth(word X,word Y,byte Couleur)
{
  int R,V,B;
  byte C;
  int Poids,Poids_total;
  byte X2=((X+1)<Principal_Largeur_image);
  byte Y2=((Y+1)<Principal_Hauteur_image);

  // On commence par le pixel central
  C=Lit_pixel_dans_ecran_feedback(X,Y);
  Poids_total=Smooth_Matrice[1][1];
  R=Poids_total*Principal_Palette[C].R;
  V=Poids_total*Principal_Palette[C].V;
  B=Poids_total*Principal_Palette[C].B;

  if (X)
  {
    C=Lit_pixel_dans_ecran_feedback(X-1,Y);
    Poids_total+=(Poids=Smooth_Matrice[0][1]);
    R+=Poids*Principal_Palette[C].R;
    V+=Poids*Principal_Palette[C].V;
    B+=Poids*Principal_Palette[C].B;

    if (Y)
    {
      C=Lit_pixel_dans_ecran_feedback(X-1,Y-1);
      Poids_total+=(Poids=Smooth_Matrice[0][0]);
      R+=Poids*Principal_Palette[C].R;
      V+=Poids*Principal_Palette[C].V;
      B+=Poids*Principal_Palette[C].B;

      if (Y2)
      {
        C=Lit_pixel_dans_ecran_feedback(X-1,Y+1);
        Poids_total+=(Poids=Smooth_Matrice[0][2]);
        R+=Poids*Principal_Palette[C].R;
        V+=Poids*Principal_Palette[C].V;
        B+=Poids*Principal_Palette[C].B;
      }
    }
  }

  if (X2)
  {
    C=Lit_pixel_dans_ecran_feedback(X+1,Y);
    Poids_total+=(Poids=Smooth_Matrice[2][1]);
    R+=Poids*Principal_Palette[C].R;
    V+=Poids*Principal_Palette[C].V;
    B+=Poids*Principal_Palette[C].B;

    if (Y)
    {
      C=Lit_pixel_dans_ecran_feedback(X+1,Y-1);
      Poids_total+=(Poids=Smooth_Matrice[2][0]);
      R+=Poids*Principal_Palette[C].R;
      V+=Poids*Principal_Palette[C].V;
      B+=Poids*Principal_Palette[C].B;

      if (Y2)
      {
        C=Lit_pixel_dans_ecran_feedback(X+1,Y+1);
        Poids_total+=(Poids=Smooth_Matrice[2][2]);
        R+=Poids*Principal_Palette[C].R;
        V+=Poids*Principal_Palette[C].V;
        B+=Poids*Principal_Palette[C].B;
      }
    }
  }

  if (Y)
  {
    C=Lit_pixel_dans_ecran_feedback(X,Y-1);
    Poids_total+=(Poids=Smooth_Matrice[1][0]);
    R+=Poids*Principal_Palette[C].R;
    V+=Poids*Principal_Palette[C].V;
    B+=Poids*Principal_Palette[C].B;
  }

  if (Y2)
  {
    C=Lit_pixel_dans_ecran_feedback(X,Y+1);
    Poids_total+=(Poids=Smooth_Matrice[1][2]);
    R+=Poids*Principal_Palette[C].R;
    V+=Poids*Principal_Palette[C].V;
    B+=Poids*Principal_Palette[C].B;
  }

  return (Poids_total)? // On regarde s'il faut éviter le 0/0.
    Meilleure_couleur(Round_div(R,Poids_total),
                      Round_div(V,Poids_total),
                      Round_div(B,Poids_total)):
    Lit_pixel_dans_ecran_courant(X,Y); // C'est bien l'écran courant et pas
                                       // l'écran feedback car il s'agit de ne
}                                      // pas modifier l'écran courant.



// -- Fonctions de manipulation du pinceau -----------------------------------

  // -- Calcul de redimensionnement du pinceau pour éviter les débordements
  //    de l'écran et de l'image --

void Calculer_dimensions_clipees(short * X,short * Y,short * Largeur,short * Hauteur)
{
  if ((*X)<Limite_Gauche)
  {
    (*Largeur)-=(Limite_Gauche-(*X));
    (*X)=Limite_Gauche;
  }

  if (((*X)+(*Largeur))>(Limite_Droite+1))
  {
    (*Largeur)=(Limite_Droite-(*X))+1;
  }

  if ((*Y)<Limite_Haut)
  {
    (*Hauteur)-=(Limite_Haut-(*Y));
    (*Y)=Limite_Haut;
  }

  if (((*Y)+(*Hauteur))>(Limite_Bas+1))
  {
    (*Hauteur)=(Limite_Bas-(*Y))+1;
  }
}

  // -- Calcul de redimensionnement du pinceau pour éviter les débordements
  //    de l'écran zoomé et de l'image --

void Calculer_dimensions_clipees_zoom(short * X,short * Y,short * Largeur,short * Hauteur)
{
  if ((*X)<Limite_Gauche_Zoom)
  {
    (*Largeur)-=(Limite_Gauche_Zoom-(*X));
    (*X)=Limite_Gauche_Zoom;
  }

  if (((*X)+(*Largeur))>(Limite_Droite_Zoom+1))
  {
    (*Largeur)=(Limite_Droite_Zoom-(*X))+1;
  }

  if ((*Y)<Limite_Haut_Zoom)
  {
    (*Hauteur)-=(Limite_Haut_Zoom-(*Y));
    (*Y)=Limite_Haut_Zoom;
  }

  if (((*Y)+(*Hauteur))>(Limite_Bas_Zoom+1))
  {
    (*Hauteur)=(Limite_Bas_Zoom-(*Y))+1;
  }
}


  // -- Afficher le pinceau (de façon définitive ou non) --

void Afficher_pinceau(short X,short Y,byte Couleur,byte Preview)
  // X,Y: position du centre du pinceau
  // Couleur: couleur à appliquer au pinceau
  // Preview: "Il ne faut l'afficher qu'à l'écran"
{
  short Debut_X; // Position X (dans l'image) à partir de laquelle on 
        // affiche la brosse/pinceau
  short Debut_Y; // Position Y (dans l'image) à partir de laquelle on 
        // affiche la brosse/pinceau
  short Largeur; // Largeur dans l'écran selon laquelle on affiche la 
        // brosse/pinceau
  short Hauteur; // Hauteur dans l'écran selon laquelle on affiche la 
        // brosse/pinceau
  short Debut_Compteur_X; // Position X (dans la brosse/pinceau) à partir 
        // de laquelle on affiche la brosse/pinceau
  short Debut_Compteur_Y; // Position Y (dans la brosse/pinceau) à partir 
        // de laquelle on affiche la brosse/pinceau
  short Pos_X; // Position X (dans l'image) en cours d'affichage
  short Pos_Y; // Position Y (dans l'image) en cours d'affichage
  short Compteur_X; // Position X (dans la brosse/pinceau) en cours 
        // d'affichage
  short Compteur_Y; // Position Y (dans la brosse/pinceau) en cours 
        // d'affichage
  short Fin_Compteur_X; // Position X ou s'arrête l'affichade de la 
        // brosse/pinceau
  short Fin_Compteur_Y; // Position Y ou s'arrête l'affichade de la 
        // brosse/pinceau
  byte  Couleur_temporaire; // Couleur de la brosse en cours d'affichage
  int Position;
  byte * Temp;

  if (!(Preview && Mouse_K)) // Si bouton enfoncé & preview > pas de dessin
  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_POINT : // !!! TOUJOURS EN PREVIEW !!!
      if ( (Pinceau_X>=Limite_Gauche)
        && (Pinceau_X<=Limite_Droite)
        && (Pinceau_Y>=Limite_Haut)
        && (Pinceau_Y<=Limite_Bas) )
        {
                Pixel_Preview(Pinceau_X,Pinceau_Y,Couleur);
                if(Loupe_Mode) Mettre_Ecran_A_Jour(Pinceau_X,Pinceau_Y,1,1);
        }
      break;

    case FORME_PINCEAU_BROSSE_COULEUR : // Brosse en couleur

      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      Largeur=Brosse_Largeur;
      Hauteur=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+Largeur;
      Fin_Compteur_Y=Debut_Compteur_Y+Hauteur;

      if (Preview)
      {
        if ( (Largeur>0) && (Hauteur>0) )
          Display_brush_Color(
                Debut_X-Principal_Decalage_X,
                Debut_Y-Principal_Decalage_Y,
                Debut_Compteur_X,
                Debut_Compteur_Y,
                Largeur,
                Hauteur,
                Back_color,
                Brosse_Largeur
          );

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&Largeur,
                &Hauteur
          );

          Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);

          if ( (Largeur>0) && (Hauteur>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            Hauteur=Debut_Y+(Hauteur*Loupe_Facteur);
            if (Hauteur>Menu_Ordonnee)
              Hauteur=Menu_Ordonnee;

            Display_brush_Color_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                     Debut_Compteur_X,Debut_Compteur_Y,
                                     Largeur,Hauteur,Back_color,
                                     Brosse_Largeur,
                                     Buffer_de_ligne_horizontale);
          }
        }

	Mettre_Ecran_A_Jour(X-Brosse_Decalage_X,Y-Brosse_Decalage_Y,Brosse_Largeur,Brosse_Hauteur);

      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((Largeur>0) && (Hauteur>0))
            {
              Copier_une_partie_d_image_dans_une_autre(
                Principal_Ecran, Debut_X, Debut_Y, Largeur, Hauteur,
                Principal_Largeur_image, Smear_Brosse,
                Debut_Compteur_X, Debut_Compteur_Y,
                Smear_Brosse_Largeur
              );
              // UPDATERECT
            }
            Smear_Debut=0;
          }
          else
          {
            for (Pos_Y = Debut_Y, Compteur_Y = Debut_Compteur_Y;
                Compteur_Y < Fin_Compteur_Y;
                Pos_Y++, Compteur_Y++
            )
              for (Pos_X = Debut_X, Compteur_X = Debut_Compteur_X;
                Compteur_X < Fin_Compteur_X;
                Pos_X++, Compteur_X++
              )
              {
                Couleur_temporaire = Lit_pixel_dans_ecran_courant(
                        Pos_X,Pos_Y
                );
                Position = (Compteur_Y * Smear_Brosse_Largeur)+ Compteur_X;
                if ( (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y) != Back_color)
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                        Afficher_pixel(Pos_X,Pos_Y,Smear_Brosse[Position]);
                Smear_Brosse[Position]=Couleur_temporaire;
              }

              SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0), 
                Fin_Compteur_X,Fin_Compteur_Y );
          }

          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          if (Shade_Table==Shade_Table_gauche)
            for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
              for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_brosse(Compteur_X,Compteur_Y);
                if (Couleur_temporaire!=Back_color)
                  Afficher_pixel(Pos_X,Pos_Y,Couleur_temporaire);
              }
          else
            for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
              for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
              {
                if (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                  Afficher_pixel(Pos_X,Pos_Y,Couleur);
              }
        }
        SDL_UpdateRect(Ecran_SDL, Max(Debut_X,0), Max(Debut_Y,0), Fin_Compteur_X, Fin_Compteur_Y);

      }
      break;
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      Largeur=Brosse_Largeur;
      Hauteur=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+Largeur;
      Fin_Compteur_Y=Debut_Compteur_Y+Hauteur;
      if (Preview)
      {
        if ( (Largeur>0) && (Hauteur>0) )
          Display_brush_Mono(Debut_X-Principal_Decalage_X,
                             Debut_Y-Principal_Decalage_Y,
                             Debut_Compteur_X,Debut_Compteur_Y,
                             Largeur,Hauteur,
                             Back_color,Fore_color,
                             Brosse_Largeur);

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
          Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);

          if ( (Largeur>0) && (Hauteur>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            Hauteur=Debut_Y+(Hauteur*Loupe_Facteur);
            if (Hauteur>Menu_Ordonnee)
              Hauteur=Menu_Ordonnee;

            Display_brush_Mono_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                    Debut_Compteur_X,Debut_Compteur_Y,
                                    Largeur,Hauteur,
                                    Back_color,Fore_color,
                                    Brosse_Largeur,
                                    Buffer_de_ligne_horizontale);

          }
        }
      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((Largeur>0) && (Hauteur>0))
              Copier_une_partie_d_image_dans_une_autre(Principal_Ecran,
                                                       Debut_X,Debut_Y,
                                                       Largeur,Hauteur,
                                                       Principal_Largeur_image,
                                                       Smear_Brosse,
                                                       Debut_Compteur_X,
                                                       Debut_Compteur_Y,
                                                       Smear_Brosse_Largeur);
                                                       //UPDATERECT
            Smear_Debut=0;
          }
          else
          {
            for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
              for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_ecran_courant(Pos_X,Pos_Y);
                Position=(Compteur_Y*Smear_Brosse_Largeur)+Compteur_X;
                if ( (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                  Afficher_pixel(Pos_X,Pos_Y,Smear_Brosse[Position]);
                Smear_Brosse[Position]=Couleur_temporaire;
              }

              SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),
                Fin_Compteur_X,Fin_Compteur_Y
              );

          }

          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
            for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
            {
              if (Lit_pixel_dans_brosse(Compteur_X,Compteur_Y)!=Back_color)
                Afficher_pixel(Pos_X,Pos_Y,Couleur);
            }
//ok
          if(Loupe_Mode) Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_Compteur_X-Debut_Compteur_X,Fin_Compteur_Y-Debut_Compteur_Y);
        }
      }
	Mettre_Ecran_A_Jour(X-Brosse_Decalage_X,Y-Brosse_Decalage_Y,Brosse_Largeur,Brosse_Hauteur);
      break;
    default : // Pinceau
      Debut_X=X-Pinceau_Decalage_X;
      Debut_Y=Y-Pinceau_Decalage_Y;
      Largeur=Pinceau_Largeur;
      Hauteur=Pinceau_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
      Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+Largeur;
      Fin_Compteur_Y=Debut_Compteur_Y+Hauteur;
      if (Preview)
      {
        Temp=Brosse;
        Brosse=Pinceau_Sprite;

        if ( (Largeur>0) && (Hauteur>0) )
          Display_brush_Mono(Debut_X-Principal_Decalage_X,
                             Debut_Y-Principal_Decalage_Y,
                             Debut_Compteur_X,Debut_Compteur_Y,
                             Largeur,Hauteur,
                             0,Fore_color,
                             TAILLE_MAXI_PINCEAU);

        if (Loupe_Mode)
        {
          Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
          Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
          Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);

          if ( (Largeur>0) && (Hauteur>0) )
          {
            // Corrections dues au Zoom:
            Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
            Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
            Hauteur=Debut_Y+(Hauteur*Loupe_Facteur);
            if (Hauteur>Menu_Ordonnee)
              Hauteur=Menu_Ordonnee;

            Display_brush_Mono_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                                    Debut_Compteur_X,Debut_Compteur_Y,
                                    Largeur,Hauteur,
                                    0,Fore_color,
                                    TAILLE_MAXI_PINCEAU,
                                    Buffer_de_ligne_horizontale);

          }
        }

        Brosse=Temp;
      }
      else
      {
        if ((Smear_Mode) && (Shade_Table==Shade_Table_gauche))
        {
          if (Smear_Debut)
          {
            if ((Largeur>0) && (Hauteur>0))
              Copier_une_partie_d_image_dans_une_autre(Principal_Ecran,
                                                       Debut_X,Debut_Y,
                                                       Largeur,Hauteur,
                                                       Principal_Largeur_image,
                                                       Smear_Brosse,
                                                       Debut_Compteur_X,
                                                       Debut_Compteur_Y,
                                                       Smear_Brosse_Largeur);
            // UPDATERECT
            Smear_Debut=0;
          }
          else
          {
            for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
              for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
              {
                Couleur_temporaire=Lit_pixel_dans_ecran_courant(Pos_X,Pos_Y);
                Position=(Compteur_Y*Smear_Brosse_Largeur)+Compteur_X;
                if ( (Pinceau_Sprite[(TAILLE_MAXI_PINCEAU*Compteur_Y)+Compteur_X])
                  && (Compteur_Y<Smear_Max_Y) && (Compteur_X<Smear_Max_X)
                  && (Compteur_Y>=Smear_Min_Y) && (Compteur_X>=Smear_Min_X) )
                  Afficher_pixel(Pos_X,Pos_Y,Smear_Brosse[Position]);
                Smear_Brosse[Position]=Couleur_temporaire;
              }
          }

          SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,
                Fin_Compteur_X,Fin_Compteur_Y
          );

          Smear_Min_X=Debut_Compteur_X;
          Smear_Min_Y=Debut_Compteur_Y;
          Smear_Max_X=Fin_Compteur_X;
          Smear_Max_Y=Fin_Compteur_Y;
        }
        else
        {
          for (Pos_Y=Debut_Y,Compteur_Y=Debut_Compteur_Y;Compteur_Y<Fin_Compteur_Y;Pos_Y++,Compteur_Y++)
            for (Pos_X=Debut_X,Compteur_X=Debut_Compteur_X;Compteur_X<Fin_Compteur_X;Pos_X++,Compteur_X++)
            {
              if (Pinceau_Sprite[(TAILLE_MAXI_PINCEAU*Compteur_Y)+Compteur_X])
                Afficher_pixel(Pos_X,Pos_Y,Couleur);
            }
                        Mettre_Ecran_A_Jour(Debut_X,Debut_Y,
                                Fin_Compteur_X-Debut_Compteur_X,
                                Fin_Compteur_Y-Debut_Compteur_Y);
        }
      }
  }
}

// -- Effacer le pinceau -- //
//
void Effacer_pinceau(short X,short Y)
  // X,Y: position du centre du pinceau
{
  short Debut_X; // Position X (dans l'image) à partir de laquelle on 
        // affiche la brosse/pinceau
  short Debut_Y; // Position Y (dans l'image) à partir de laquelle on 
        // affiche la brosse/pinceau
  short Largeur; // Largeur dans l'écran selon laquelle on affiche la 
        // brosse/pinceau
  short Hauteur; // Hauteur dans l'écran selon laquelle on affiche la 
        // brosse/pinceau
  short Debut_Compteur_X; // Position X (dans la brosse/pinceau) à partir 
        // de laquelle on affiche la brosse/pinceau
  short Debut_Compteur_Y; // Position Y (dans la brosse/pinceau) à partir 
        // de laquelle on affiche la brosse/pinceau
  //short Pos_X; // Position X (dans l'image) en cours d'affichage
  //short Pos_Y; // Position Y (dans l'image) en cours d'affichage
  //short Compteur_X; // Position X (dans la brosse/pinceau) en cours 
        //d'affichage
  //short Compteur_Y; // Position Y (dans la brosse/pinceau) en cours d'affichage
  short Fin_Compteur_X; // Position X ou s'arrête l'affichade de la brosse/pinceau
  short Fin_Compteur_Y; // Position Y ou s'arrête l'affichade de la brosse/pinceau
  byte * Temp;

  if (!Mouse_K)
  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_POINT :
      if ( (Pinceau_X>=Limite_Gauche)
        && (Pinceau_X<=Limite_Droite)
        && (Pinceau_Y>=Limite_Haut)
        && (Pinceau_Y<=Limite_Bas) )
      {
        Pixel_Preview(Pinceau_X,Pinceau_Y,Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y));
        if(Loupe_Mode) Mettre_Ecran_A_Jour(Pinceau_X,Pinceau_Y,1,1);
      }
      break;
    case FORME_PINCEAU_BROSSE_COULEUR :    // Brosse en couleur
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      Debut_X=X-Brosse_Decalage_X;
      Debut_Y=Y-Brosse_Decalage_Y;
      Largeur=Brosse_Largeur;
      Hauteur=Brosse_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
      Debut_Compteur_X=Debut_X-(X-Brosse_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Brosse_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+Largeur;
      Fin_Compteur_Y=Debut_Compteur_Y+Hauteur;

      if ( (Largeur>0) && (Hauteur>0) )
        Clear_brush(Debut_X-Principal_Decalage_X,
                    Debut_Y-Principal_Decalage_Y,
                    Debut_Compteur_X,Debut_Compteur_Y,
                    Largeur,Hauteur,Back_color,
                    Principal_Largeur_image);

      if (Loupe_Mode)
      {
        Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
        Debut_Compteur_X=Debut_X;
        Debut_Compteur_Y=Debut_Y;

        if ( (Largeur>0) && (Hauteur>0) )
        {
          // Corrections dues au Zoom:
          Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
          Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
          Hauteur=Debut_Y+(Hauteur*Loupe_Facteur);
          if (Hauteur>Menu_Ordonnee)
            Hauteur=Menu_Ordonnee;

          Clear_brush_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                           Debut_Compteur_X,Debut_Compteur_Y,
                           Largeur,Hauteur,Back_color,
                           Principal_Largeur_image,
                           Buffer_de_ligne_horizontale);
        }
      }
      break;
    default: // Pinceau
      Debut_X=X-Pinceau_Decalage_X;
      Debut_Y=Y-Pinceau_Decalage_Y;
      Largeur=Pinceau_Largeur;
      Hauteur=Pinceau_Hauteur;
      Calculer_dimensions_clipees(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
      Debut_Compteur_X=Debut_X-(X-Pinceau_Decalage_X);
      Debut_Compteur_Y=Debut_Y-(Y-Pinceau_Decalage_Y);
      Fin_Compteur_X=Debut_Compteur_X+Largeur;
      Fin_Compteur_Y=Debut_Compteur_Y+Hauteur;

      Temp=Brosse;
      Brosse=Pinceau_Sprite;

      if ( (Largeur>0) && (Hauteur>0) )
      {
        Clear_brush(Debut_X-Principal_Decalage_X,
                    Debut_Y-Principal_Decalage_Y,
                    Debut_Compteur_X,Debut_Compteur_Y,
                    Largeur,Hauteur,0,
                    Principal_Largeur_image);
      }

      if (Loupe_Mode)
      {
        Calculer_dimensions_clipees_zoom(&Debut_X,&Debut_Y,&Largeur,&Hauteur);
        Debut_Compteur_X=Debut_X;
        Debut_Compteur_Y=Debut_Y;

        if ( (Largeur>0) && (Hauteur>0) )
        {
          // Corrections dues au Zoom:
          Debut_X=(Debut_X-Loupe_Decalage_X)*Loupe_Facteur;
          Debut_Y=(Debut_Y-Loupe_Decalage_Y)*Loupe_Facteur;
          Hauteur=Debut_Y+(Hauteur*Loupe_Facteur);
          if (Hauteur>Menu_Ordonnee)
            Hauteur=Menu_Ordonnee;

          Clear_brush_zoom(Principal_X_Zoom+Debut_X,Debut_Y,
                           Debut_Compteur_X,Debut_Compteur_Y,
                           Largeur,Hauteur,0,
                           Principal_Largeur_image,
                           Buffer_de_ligne_horizontale);
        }
      }

      Brosse=Temp;
      break;
  }
}


// -- Fonctions de manipulation du curseur -----------------------------------


  // -- Afficher une barre horizontale XOR zoomée

void Ligne_horizontale_XOR_Zoom(short Pos_X, short Pos_Y, short Largeur)
{
  short Pos_X_reelle=Principal_X_Zoom+(Pos_X-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(Pos_Y-Loupe_Decalage_Y)*Loupe_Facteur;
  short Largeur_reelle=Largeur*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+Loupe_Facteur<Menu_Ordonnee)?Pos_Y_reelle+Loupe_Facteur:Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Largeur_reelle);

  SDL_UpdateRect(Ecran_SDL,Pos_X_reelle,Pos_Y_reelle,Largeur_reelle,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher une barre verticale XOR zoomée

void Ligne_verticale_XOR_Zoom(short Pos_X, short Pos_Y, short Hauteur)
{
  short Pos_X_reelle=Principal_X_Zoom+(Pos_X-Loupe_Decalage_X)*Loupe_Facteur;
  short Pos_Y_reelle=(Pos_Y-Loupe_Decalage_Y)*Loupe_Facteur;
  short Pos_Y_Fin=(Pos_Y_reelle+(Hauteur*Loupe_Facteur<Menu_Ordonnee))?Pos_Y_reelle+(Hauteur*Loupe_Facteur):Menu_Ordonnee;
  short Indice;

  for (Indice=Pos_Y_reelle; Indice<Pos_Y_Fin; Indice++)
    Ligne_horizontale_XOR(Pos_X_reelle,Indice,Loupe_Facteur);

  SDL_UpdateRect(Ecran_SDL,Pos_X_reelle,Pos_Y_reelle,Loupe_Facteur,Pos_Y_Fin-Pos_Y_reelle);
}


  // -- Afficher le curseur --

void Afficher_curseur(void)
{
  byte  Forme;
  short Debut_X;
  short Debut_Y;
  short Fin_X;
  short Fin_Y;
  short Pos_X;
  short Pos_Y;
  short Compteur_X;
  short Compteur_Y;
  //short Fin_Compteur_X; // Position X ou s'arrête l'affichage de la brosse/pinceau
  //short Fin_Compteur_Y; // Position Y ou s'arrête l'affichage de la brosse/pinceau
  int   Temp;
  byte  Couleur;
  float cosA,sinA;
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;

  if ( ( (Mouse_Y<Menu_Ordonnee)
      && ( (!Loupe_Mode) || (Mouse_X<Principal_Split) || (Mouse_X>=Principal_X_Zoom) ) )
    || (Une_fenetre_est_ouverte) || (Forme_curseur==FORME_CURSEUR_SABLIER) )
    Forme=Forme_curseur;
  else
    Forme=FORME_CURSEUR_FLECHE;

  switch(Forme)
  {
    case FORME_CURSEUR_CIBLE :
      if (!Cacher_pinceau)
        Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,1);
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          Debut_Y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (Debut_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-6,4-Debut_Y);

          Debut_X=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (Debut_X<4)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-6,Mouse_Y,4-Debut_X);

          Fin_X=(Mouse_X+7>Largeur_ecran)?Mouse_X+7-Largeur_ecran:0;
          if (Fin_X<4)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,4-Fin_X);

          Fin_Y=(Mouse_Y+7>/*Menu_Ordonnee*/Hauteur_ecran)?Mouse_Y+7-/*Menu_Ordonnee*/Hauteur_ecran:0;
          if (Fin_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,4-Fin_Y);

          SDL_UpdateRect(Ecran_SDL,Mouse_X+Debut_X-6,Mouse_Y+Debut_Y-6,12-Fin_X,12-Fin_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_FINE:FORME_CURSEUR_CIBLE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
            {
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran)
                && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
              {
                FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
                if (Couleur!=CM_Trans)
                  Pixel(Pos_X,Pos_Y,Couleur);
              }
            }

          SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),16,16);
        }
      }
      break;
    case FORME_CURSEUR_CIBLE_PIPETTE:
      if (!Cacher_pinceau)
        Afficher_pinceau(Pinceau_X,Pinceau_Y,Fore_color,1);
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          // Barres formant la croix principale

          Debut_Y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (Debut_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-5,3-Debut_Y);

          Debut_X=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (Debut_X<3)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-5,Mouse_Y,3-Debut_X);

          Fin_X=(Mouse_X+6>Largeur_ecran)?Mouse_X+6-Largeur_ecran:0;
          if (Fin_X<3)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,3-Fin_X);

          Fin_Y=(Mouse_Y+6>Menu_Ordonnee/*Hauteur_ecran*/)?Mouse_Y+6-Menu_Ordonnee/*Hauteur_ecran*/:0;
          if (Fin_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,3-Fin_Y);

          // Petites barres aux extrémités

          Debut_X=(!Mouse_X);
          Debut_Y=(!Mouse_Y);
          Fin_X=(Mouse_X>=Largeur_ecran-1);
          Fin_Y=(Mouse_Y>=Menu_Ordonnee-1);

          if (Mouse_Y>5)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y-6,3-(Debut_X+Fin_X));

          if (Mouse_X>5)
            Ligne_verticale_XOR  (Mouse_X-6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_X<Largeur_ecran-6)
            Ligne_verticale_XOR  (Mouse_X+6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_Y<Menu_Ordonnee-6)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y+6,3-(Debut_X+Fin_X));
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_PIPETTE_FINE:FORME_CURSEUR_CIBLE_PIPETTE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
            {
              Couleur=SPRITE_CURSEUR[Temp][Compteur_Y][Compteur_X];
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran)
                && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
              {
                FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
                if (Couleur!=CM_Trans)
                  Pixel(Pos_X,Pos_Y,Couleur);
              }
            }
          SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,16,16);
        }
      }
      break;
    case FORME_CURSEUR_MULTIDIRECTIONNEL :
    case FORME_CURSEUR_HORIZONTAL :
      if (Cacher_curseur)
        break;
    case FORME_CURSEUR_FLECHE :
    case FORME_CURSEUR_SABLIER :
      Debut_X=Mouse_X-Curseur_Decalage_X[Forme];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[Forme];
      for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
        for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
        {
          Couleur=SPRITE_CURSEUR[Forme][Compteur_Y][Compteur_X];
          if ( (Pos_X<Largeur_ecran) && (Pos_Y<Hauteur_ecran)
            && (Pos_X>=0)            && (Pos_Y>=0) )
          {
            // On sauvegarde dans FOND_CURSEUR pour restaurer plus tard
            FOND_CURSEUR[Compteur_Y][Compteur_X]=Lit_pixel(Pos_X,Pos_Y);
            if (Couleur!=CM_Trans)
              Pixel(Pos_X,Pos_Y,Couleur);
          }
        }
      SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),16,16);
      break;
    case FORME_CURSEUR_CIBLE_XOR :
      Pos_X=Pinceau_X-Principal_Decalage_X;
      Pos_Y=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // Largeur de la barre XOR
      if ((Pos_Y<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        SDL_UpdateRect(Ecran_SDL,0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((Pos_X<Compteur_X) && (Pinceau_X>=Limite_Gauche))
      {
        Ligne_verticale_XOR(Pinceau_X-Principal_Decalage_X,0,Menu_Ordonnee);
        SDL_UpdateRect(Ecran_SDL,Pinceau_X-Principal_Decalage_X,0,1,Menu_Ordonnee);
      }

      if (Loupe_Mode)
      {
        // UPDATERECT
        if ((Pinceau_Y>=Limite_Haut_Zoom) && (Pinceau_Y<=Limite_visible_Bas_Zoom))
          Ligne_horizontale_XOR_Zoom(Limite_Gauche_Zoom,Pinceau_Y,Loupe_Largeur);
        if ((Pinceau_X>=Limite_Gauche_Zoom) && (Pinceau_X<=Limite_visible_Droite_Zoom))
          Ligne_verticale_XOR_Zoom(Pinceau_X,Limite_Haut_Zoom,Loupe_Hauteur);
      }
      break;
    case FORME_CURSEUR_RECTANGLE_XOR :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      Debut_X=(Mouse_X-3);
      Debut_Y=(Mouse_Y-3);
      Fin_X  =(Mouse_X+4);
      Fin_Y  =(Mouse_Y+4);
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      if (Fin_X>Largeur_ecran)
        Fin_X=Largeur_ecran;
      if (Fin_Y>Menu_Ordonnee)
        Fin_Y=Menu_Ordonnee;

      Ligne_horizontale_XOR(Debut_X,Mouse_Y,Fin_X-Debut_X);
      Ligne_verticale_XOR  (Mouse_X,Debut_Y,Fin_Y-Debut_Y);

      // Grand rectangle autour
      Debut_X=Mouse_X-(Loupe_Largeur>>1);
      Debut_Y=Mouse_Y-(Loupe_Hauteur>>1);
      if (Debut_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
        Debut_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
      if (Debut_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
        Debut_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      Fin_X=Debut_X+Loupe_Largeur-1;
      Fin_Y=Debut_Y+Loupe_Hauteur-1;

      Ligne_horizontale_XOR(Debut_X,Debut_Y,Loupe_Largeur);
      Ligne_verticale_XOR(Debut_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_verticale_XOR(  Fin_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_horizontale_XOR(Debut_X,  Fin_Y,Loupe_Largeur);

      SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);

      break;
    default: //case FORME_CURSEUR_ROTATE_XOR :
      Debut_X=1-(Brosse_Largeur>>1);
      Debut_Y=1-(Brosse_Hauteur>>1);
      Fin_X=Debut_X+Brosse_Largeur-1;
      Fin_Y=Debut_Y+Brosse_Hauteur-1;

      if (Brosse_Centre_rotation_defini)
      {
        if ( (Brosse_Centre_rotation_X==Pinceau_X)
          && (Brosse_Centre_rotation_Y==Pinceau_Y) )
        {
          cosA=1.0;
          sinA=0.0;
        }
        else
        {
          Pos_X=Pinceau_X-Brosse_Centre_rotation_X;
          Pos_Y=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)Pos_X/sqrt((Pos_X*Pos_X)+(Pos_Y*Pos_Y));
          sinA=sin(acos(cosA));
          if (Pos_Y>0) sinA=-sinA;
        }

        Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
        Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
        Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
        Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

        X1+=Brosse_Centre_rotation_X;
        Y1+=Brosse_Centre_rotation_Y;
        X2+=Brosse_Centre_rotation_X;
        Y2+=Brosse_Centre_rotation_Y;
        X3+=Brosse_Centre_rotation_X;
        Y3+=Brosse_Centre_rotation_Y;
        X4+=Brosse_Centre_rotation_X;
        Y4+=Brosse_Centre_rotation_Y;
        Pixel_figure_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,0);
        Tracer_ligne_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,Pinceau_X,Pinceau_Y,0);
      }
      else
      {
        X1=X3=1-Brosse_Largeur;
        Y1=Y2=Debut_Y;
        X2=X4=Pinceau_X;
        Y3=Y4=Fin_Y;

        X1+=Pinceau_X;
        Y1+=Pinceau_Y;
        Y2+=Pinceau_Y;
        X3+=Pinceau_X;
        Y3+=Pinceau_Y;
        Y4+=Pinceau_Y;
        Pixel_figure_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,0);
        Tracer_ligne_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,Pinceau_X,Pinceau_Y,0);
      }

      Tracer_ligne_Preview_xor(X1,Y1,X2,Y2,0);
      Tracer_ligne_Preview_xor(X2,Y2,X4,Y4,0);
      Tracer_ligne_Preview_xor(X4,Y4,X3,Y3,0);
      Tracer_ligne_Preview_xor(X3,Y3,X1,Y1,0);
  }
}

  // -- Effacer le curseur --

void Effacer_curseur(void)
{
  byte  Forme;
  int Debut_X; // int car sont parfois négatifs ! (quand on dessine sur un bord)
  int Debut_Y;
  short Fin_X;
  short Fin_Y;
  int Pos_X;
  int Pos_Y;
  short Compteur_X;
  short Compteur_Y;
  //short Fin_Compteur_X; // Position X ou s'arrête l'affichage de la brosse/pinceau
  //short Fin_Compteur_Y; // Position Y ou s'arrête l'affichage de la brosse/pinceau
  int   Temp;
  //byte  Couleur;
  float cosA,sinA;
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;

  if ( ( (Mouse_Y<Menu_Ordonnee)
      && ( (!Loupe_Mode) || (Mouse_X<Principal_Split) 
                         || (Mouse_X>=Principal_X_Zoom) ) )
    || (Une_fenetre_est_ouverte) || (Forme_curseur==FORME_CURSEUR_SABLIER) )
    Forme=Forme_curseur;
  else
    Forme=FORME_CURSEUR_FLECHE;

  switch(Forme)
  {
    case FORME_CURSEUR_CIBLE :
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          Debut_Y=(Mouse_Y<6)?6-Mouse_Y:0;
          if (Debut_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-6,4-Debut_Y);

          Debut_X=(Mouse_X<6)?(short)6-Mouse_X:0;
          if (Debut_X<4)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-6,Mouse_Y,4-Debut_X);

          Fin_X=(Mouse_X+7>Largeur_ecran)?Mouse_X+7-Largeur_ecran:0;
          if (Fin_X<4)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,4-Fin_X);

          Fin_Y=(Mouse_Y+7>Hauteur_ecran)?Mouse_Y+7-Hauteur_ecran:0;
          if (Fin_Y<4)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,4-Fin_Y);

          SDL_UpdateRect(Ecran_SDL,Mouse_X+Debut_X-6,Mouse_Y+Debut_Y-6,12-Fin_X,12-Fin_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_FINE:FORME_CURSEUR_CIBLE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
            for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran) && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
                Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);

          SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),16,16);
        }
      }
      if (!Cacher_pinceau)
      {
        Effacer_pinceau(Pinceau_X,Pinceau_Y);
      }
      break;
    case FORME_CURSEUR_CIBLE_PIPETTE:
      if (!Cacher_curseur)
      {
        if (Config.Curseur==1)
        {
          // Barres formant la croix principale

          Debut_Y=(Mouse_Y<5)?5-Mouse_Y:0;
          if (Debut_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+Debut_Y-5,3-Debut_Y);

          Debut_X=(Mouse_X<5)?(short)5-Mouse_X:0;
          if (Debut_X<3)
            Ligne_horizontale_XOR(Mouse_X+Debut_X-5,Mouse_Y,3-Debut_X);

          Fin_X=(Mouse_X+6>Largeur_ecran)?Mouse_X+6-Largeur_ecran:0;
          if (Fin_X<3)
            Ligne_horizontale_XOR(Mouse_X+3,Mouse_Y,3-Fin_X);

          Fin_Y=(Mouse_Y+6>Hauteur_ecran)?Mouse_Y+6-Hauteur_ecran:0;
          if (Fin_Y<3)
            Ligne_verticale_XOR  (Mouse_X,Mouse_Y+3,3-Fin_Y);

          Debut_X=(!Mouse_X);
          Debut_Y=(!Mouse_Y);
          Fin_X=(Mouse_X>=Largeur_ecran-1);
          Fin_Y=(Mouse_Y>=Menu_Ordonnee-1);

          if (Mouse_Y>5)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y-6,3-(Debut_X+Fin_X));

          if (Mouse_X>5)
            Ligne_verticale_XOR  (Mouse_X-6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_X<Largeur_ecran-6)
            Ligne_verticale_XOR  (Mouse_X+6,Debut_Y+Mouse_Y-1,3-(Debut_Y+Fin_Y));

          if (Mouse_Y<Menu_Ordonnee-6)
            Ligne_horizontale_XOR(Debut_X+Mouse_X-1,Mouse_Y+6,3-(Debut_X+Fin_X));

          SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Fin_X-Debut_X,Fin_Y-Debut_Y);
        }
        else
        {
          Temp=(Config.Curseur)?FORME_CURSEUR_CIBLE_PIPETTE_FINE:FORME_CURSEUR_CIBLE_PIPETTE;
          Debut_X=Mouse_X-Curseur_Decalage_X[Temp];
          Debut_Y=Mouse_Y-Curseur_Decalage_Y[Temp];

          for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
            for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
              if ( (Pos_X>=0) && (Pos_X<Largeur_ecran) && (Pos_Y>=0) && (Pos_Y<Hauteur_ecran) )
                Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);

          SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),16,16);
        }
      }
      if (!Cacher_pinceau)
        Effacer_pinceau(Pinceau_X,Pinceau_Y);
      break;
    case FORME_CURSEUR_MULTIDIRECTIONNEL :
    case FORME_CURSEUR_HORIZONTAL :
      if (Cacher_curseur)
        break;
    case FORME_CURSEUR_FLECHE :
    case FORME_CURSEUR_SABLIER :
      Debut_X=Mouse_X-Curseur_Decalage_X[Forme];
      Debut_Y=Mouse_Y-Curseur_Decalage_Y[Forme];

      for (Pos_X=Debut_X,Compteur_X=0;Compteur_X<15;Pos_X++,Compteur_X++)
        for (Pos_Y=Debut_Y,Compteur_Y=0;Compteur_Y<15;Pos_Y++,Compteur_Y++)
          if ( (Pos_X<Largeur_ecran) && (Pos_Y<Hauteur_ecran)
            && (Pos_X>=0)            && (Pos_Y>=0) )
            Pixel(Pos_X,Pos_Y,FOND_CURSEUR[Compteur_Y][Compteur_X]);
      SDL_UpdateRect(Ecran_SDL,Max(Debut_X,0),Max(Debut_Y,0),16,16);
      break;

    case FORME_CURSEUR_CIBLE_XOR :
      Pos_X=Pinceau_X-Principal_Decalage_X;
      Pos_Y=Pinceau_Y-Principal_Decalage_Y;

      Compteur_X=(Loupe_Mode)?Principal_Split:Largeur_ecran; // Largeur de la barre XOR
      if ((Pos_Y<Menu_Ordonnee) && (Pinceau_Y>=Limite_Haut))
      {
        Ligne_horizontale_XOR(0,Pinceau_Y-Principal_Decalage_Y,Compteur_X);
        SDL_UpdateRect(Ecran_SDL,0,Pinceau_Y-Principal_Decalage_Y,Compteur_X,1);
      }

      if ((Pos_X<Compteur_X) && (Pinceau_X>=Limite_Gauche))
      {
        Ligne_verticale_XOR(Pinceau_X-Principal_Decalage_X,0,Menu_Ordonnee);
        SDL_UpdateRect(Ecran_SDL,Pinceau_X-Principal_Decalage_X,0,1,Menu_Ordonnee);
      }

      if (Loupe_Mode)
      {
        // UPDATERECT
        if ((Pinceau_Y>=Limite_Haut_Zoom) && (Pinceau_Y<=Limite_visible_Bas_Zoom))
          Ligne_horizontale_XOR_Zoom(Limite_Gauche_Zoom,Pinceau_Y,Loupe_Largeur);
        if ((Pinceau_X>=Limite_Gauche_Zoom) && (Pinceau_X<=Limite_visible_Droite_Zoom))
          Ligne_verticale_XOR_Zoom(Pinceau_X,Limite_Haut_Zoom,Loupe_Hauteur);
      }


      break;
    case FORME_CURSEUR_RECTANGLE_XOR :
      // !!! Cette forme ne peut pas être utilisée en mode Loupe !!!

      // Petite croix au centre
      Debut_X=(Mouse_X-3);
      Debut_Y=(Mouse_Y-3);
      Fin_X  =(Mouse_X+4);
      Fin_Y  =(Mouse_Y+4);
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      if (Fin_X>Largeur_ecran)
        Fin_X=Largeur_ecran;
      if (Fin_Y>Menu_Ordonnee)
        Fin_Y=Menu_Ordonnee;

      Ligne_horizontale_XOR(Debut_X,Mouse_Y,Fin_X-Debut_X);
      Ligne_verticale_XOR  (Mouse_X,Debut_Y,Fin_Y-Debut_Y);

      // Grand rectangle autour

      Debut_X=Mouse_X-(Loupe_Largeur>>1);
      Debut_Y=Mouse_Y-(Loupe_Hauteur>>1);
      if (Debut_X+Loupe_Largeur>=Limite_Droite-Principal_Decalage_X)
        Debut_X=Limite_Droite-Loupe_Largeur-Principal_Decalage_X+1;
      if (Debut_Y+Loupe_Hauteur>=Limite_Bas-Principal_Decalage_Y)
        Debut_Y=Limite_Bas-Loupe_Hauteur-Principal_Decalage_Y+1;
      if (Debut_X<0)
        Debut_X=0;
      if (Debut_Y<0)
        Debut_Y=0;
      Fin_X=Debut_X+Loupe_Largeur-1;
      Fin_Y=Debut_Y+Loupe_Hauteur-1;

      Ligne_horizontale_XOR(Debut_X,Debut_Y,Loupe_Largeur);
      Ligne_verticale_XOR(Debut_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_verticale_XOR(  Fin_X,Debut_Y+1,Loupe_Hauteur-2);
      Ligne_horizontale_XOR(Debut_X,  Fin_Y,Loupe_Largeur);

      SDL_UpdateRect(Ecran_SDL,Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);

      break;
    default: //case FORME_CURSEUR_ROTATE_XOR :
      Debut_X=1-(Brosse_Largeur>>1);
      Debut_Y=1-(Brosse_Hauteur>>1);
      Fin_X=Debut_X+Brosse_Largeur-1;
      Fin_Y=Debut_Y+Brosse_Hauteur-1;

      if (Brosse_Centre_rotation_defini)
      {
        if ( (Brosse_Centre_rotation_X==Pinceau_X)
          && (Brosse_Centre_rotation_Y==Pinceau_Y) )
        {
          cosA=1.0;
          sinA=0.0;
        }
        else
        {
          Pos_X=Pinceau_X-Brosse_Centre_rotation_X;
          Pos_Y=Pinceau_Y-Brosse_Centre_rotation_Y;
          cosA=(float)Pos_X/sqrt((Pos_X*Pos_X)+(Pos_Y*Pos_Y));
          sinA=sin(acos(cosA));
          if (Pos_Y>0) sinA=-sinA;
        }

        Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
        Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
        Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
        Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

        X1+=Brosse_Centre_rotation_X;
        Y1+=Brosse_Centre_rotation_Y;
        X2+=Brosse_Centre_rotation_X;
        Y2+=Brosse_Centre_rotation_Y;
        X3+=Brosse_Centre_rotation_X;
        Y3+=Brosse_Centre_rotation_Y;
        X4+=Brosse_Centre_rotation_X;
        Y4+=Brosse_Centre_rotation_Y;
        Pixel_figure_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,0);
        Tracer_ligne_Preview_xor(Brosse_Centre_rotation_X,Brosse_Centre_rotation_Y,Pinceau_X,Pinceau_Y,0);
      }
      else
      {
        X1=X3=1-Brosse_Largeur;
        Y1=Y2=Debut_Y;
        X2=X4=Pinceau_X;
        Y3=Y4=Fin_Y;

        X1+=Pinceau_X;
        Y1+=Pinceau_Y;
        Y2+=Pinceau_Y;
        X3+=Pinceau_X;
        Y3+=Pinceau_Y;
        Y4+=Pinceau_Y;
        Pixel_figure_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,0);
        Tracer_ligne_Preview_xor(Pinceau_X-Fin_X,Pinceau_Y,Pinceau_X,Pinceau_Y,0);
      }

      Tracer_ligne_Preview_xor(X1,Y1,X2,Y2,0);
      Tracer_ligne_Preview_xor(X2,Y2,X4,Y4,0);
      Tracer_ligne_Preview_xor(X4,Y4,X3,Y3,0);
      Tracer_ligne_Preview_xor(X3,Y3,X1,Y1,0);
  }
}



//---- Fenêtre demandant de confirmer une action et renvoyant la réponse -----
byte Demande_de_confirmation(char * Message)
{
  short Bouton_clicke;
  word  Largeur_de_la_fenetre;

  Largeur_de_la_fenetre=(strlen(Message)<<3)+20;

  if (Largeur_de_la_fenetre<120)
    Largeur_de_la_fenetre=120;

  Ouvrir_fenetre(Largeur_de_la_fenetre,60,"Confirmation");

  Print_dans_fenetre((Largeur_de_la_fenetre>>1)-(strlen(Message)<<2),20,Message,CM_Noir,CM_Clair);

  Fenetre_Definir_bouton_normal((Largeur_de_la_fenetre/3)-20     ,37,40,14,"Yes",1,1,SDLK_y); // 1
  Fenetre_Definir_bouton_normal(((Largeur_de_la_fenetre<<1)/3)-20,37,40,14,"No" ,1,1,SDLK_n); // 2

  SDL_UpdateRect(Ecran_SDL,Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*Largeur_de_la_fenetre,Menu_Facteur_Y*60);

  Afficher_curseur();

  do
  {
    Bouton_clicke=Fenetre_Bouton_clicke();
    if (Touche==SDLK_RETURN) Bouton_clicke=1;
    if (Touche==SDLK_ESCAPE) Bouton_clicke=2;
  }
  while (Bouton_clicke<=0);

  Fermer_fenetre();
  Afficher_curseur();

  return (Bouton_clicke==1)? 1 : 0;
}



//---- Fenêtre avertissant de quelque chose et attendant un click sur OK -----
void Warning_message(char * Message)
{
  short Bouton_clicke;
  word  Largeur_de_la_fenetre;

  Largeur_de_la_fenetre=(strlen(Message)<<3)+20;
  if (Largeur_de_la_fenetre<120)
    Largeur_de_la_fenetre=120;

  Ouvrir_fenetre(Largeur_de_la_fenetre,60,"Warning!");

  Print_dans_fenetre((Largeur_de_la_fenetre>>1)-(strlen(Message)<<2),20,Message,CM_Noir,CM_Clair);
  Fenetre_Definir_bouton_normal((Largeur_de_la_fenetre>>1)-20     ,37,40,14,"OK",1,1,SDLK_RETURN); // 1
  SDL_UpdateRect(Ecran_SDL,Fenetre_Pos_X,Fenetre_Pos_Y,Menu_Facteur_X*Largeur_de_la_fenetre,Menu_Facteur_Y*60);
  Afficher_curseur();

  do
    Bouton_clicke=Fenetre_Bouton_clicke();
  while ((Bouton_clicke<=0) && (Touche!=SDLK_ESCAPE) && (Touche!=SDLK_o));

  Fermer_fenetre();
  Afficher_curseur();
}



// -- Fonction diverses d'affichage ------------------------------------------

  // -- Reafficher toute l'image (en prenant en compte le facteur de zoom) --

void Afficher_ecran(void)
{
  word Largeur;
  word Hauteur;

  // ---/\/\/\  Partie non zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    if (Principal_Largeur_image<Principal_Split)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Principal_Split;
  }
  else
  {
    if (Principal_Largeur_image<Largeur_ecran)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Largeur_ecran;
  }
  if (Principal_Hauteur_image<Menu_Ordonnee)
    Hauteur=Principal_Hauteur_image;
  else
    Hauteur=Menu_Ordonnee;
  Display_screen(Largeur,Hauteur,Principal_Largeur_image);

  // Effacement de la partie non-image dans la partie non zoomée:
  if (Loupe_Mode)
  {
    if (Principal_Largeur_image<Principal_Split && Principal_Largeur_image < Largeur_ecran)
      Block(Principal_Largeur_image,0,(Principal_Split-Principal_Largeur_image),Menu_Ordonnee,0);
  }
  else
  {
    if (Principal_Largeur_image<Largeur_ecran)
      Block(Principal_Largeur_image,0,(Largeur_ecran-Principal_Largeur_image),Menu_Ordonnee,0);
  }
  if (Principal_Hauteur_image<Menu_Ordonnee)
    Block(0,Principal_Hauteur_image,Largeur,(Menu_Ordonnee-Hauteur),0);
  SDL_UpdateRect(Ecran_SDL,0,0,Largeur_ecran,Menu_Ordonnee); // TODO On peut faire plus fin, en évitant de mettre à jour la partie à droite du split quand on est en mode loupe. Mais c'est pas vraiment intéressant ?

  // ---/\/\/\  Partie zoomée: /\/\/\---
  if (Loupe_Mode)
  {
    // Affichage de la barre de split
    Afficher_barre_de_split();

    // Calcul de la largeur visible
    if (Principal_Largeur_image<Loupe_Largeur)
      Largeur=Principal_Largeur_image;
    else
      Largeur=Loupe_Largeur;

    // Calcul du nombre de lignes visibles de l'image zoomée
    if (Principal_Hauteur_image<Loupe_Hauteur)
      Hauteur=Principal_Hauteur_image*Loupe_Facteur;
    else
      Hauteur=Menu_Ordonnee;

    Display_zoomed_screen(Largeur,Hauteur,Principal_Largeur_image,Buffer_de_ligne_horizontale);

    // Effacement de la partie non-image dans la partie zoomée:
    if (Principal_Largeur_image<Loupe_Largeur)
      Block(Principal_X_Zoom+(Principal_Largeur_image*Loupe_Facteur),0,
            (Loupe_Largeur-Principal_Largeur_image)*Loupe_Facteur,
            Menu_Ordonnee,0);
    if (Principal_Hauteur_image<Loupe_Hauteur)
      Block(Principal_X_Zoom,Hauteur,Largeur*Loupe_Facteur,(Menu_Ordonnee-Hauteur),0);
  }

  // ---/\/\/\ Affichage des limites /\/\/\---
  if (Config.Afficher_limites_image)
    Afficher_limites_de_l_image();
}

  // -- Redessiner le sprite d'un bouton dans le menu --

void Afficher_sprite_dans_menu(int Numero_bouton,int Numero_sprite)
{
  word Pos_X;
  word Pos_Y;
  word Pos_menu_X;
  word Pos_menu_Y;
  byte Couleur;

  for (Pos_Y=0,Pos_menu_Y=Bouton[Numero_bouton].Decalage_Y+1;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_Y++,Pos_menu_Y++)
    for (Pos_X=0,Pos_menu_X=Bouton[Numero_bouton].Decalage_X+1;Pos_X<LARGEUR_SPRITE_MENU;Pos_X++,Pos_menu_X++)
    {
      Couleur=SPRITE_MENU[Numero_sprite][Pos_Y][Pos_X];
      Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
      BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
    }
  SDL_UpdateRect(Ecran_SDL,Menu_Facteur_X*(Bouton[Numero_bouton].Decalage_X+1),
  	(Bouton[Numero_bouton].Decalage_Y+1)*Menu_Facteur_Y+Menu_Ordonnee,
  	LARGEUR_SPRITE_MENU*Menu_Facteur_X,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y);
}

  // -- Redessiner la forme du pinceau dans le menu --

void Afficher_pinceau_dans_menu(void)
{
  short Pos_X,Pos_Y;
  short Debut_X;
  short Pos_menu_X,Pos_menu_Y;
  short Debut_menu_X;
  byte Couleur;

  switch (Pinceau_Forme)
  {
    case FORME_PINCEAU_BROSSE_COULEUR    : // Brosse en couleur
    case FORME_PINCEAU_BROSSE_MONOCHROME : // Brosse monochrome
      for (Pos_menu_Y=2,Pos_Y=0;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=1,Pos_X=0;Pos_X<LARGEUR_SPRITE_MENU;Pos_menu_X++,Pos_X++)
        {
          Couleur=SPRITE_MENU[3][Pos_Y][Pos_X];
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
      break;
    default : // Pinceau
      // On efface le pinceau précédent
      for (Pos_menu_Y=2,Pos_Y=0;Pos_Y<HAUTEUR_SPRITE_MENU;Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=1,Pos_X=0;Pos_X<LARGEUR_SPRITE_MENU;Pos_menu_X++,Pos_X++)
        {
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,CM_Clair);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=CM_Clair;
        }
      // On affiche le nouveau
      Debut_menu_X=8-Pinceau_Decalage_X;
      if (Debut_menu_X<1)
      {
        Debut_X=Pinceau_Decalage_X-7;
        Debut_menu_X=1;
      }
      else
        Debut_X=0;

      Pos_menu_Y=9-Pinceau_Decalage_Y;
      if (Pos_menu_Y<2)
      {
        Pos_Y=Pinceau_Decalage_Y-7;
        Pos_menu_Y=2;
      }
      else
        Pos_Y=0;

      for (;((Pos_Y<Pinceau_Hauteur) && (Pos_menu_Y<16));Pos_menu_Y++,Pos_Y++)
        for (Pos_menu_X=Debut_menu_X,Pos_X=Debut_X;((Pos_X<Pinceau_Largeur) && (Pos_menu_X<15));Pos_menu_X++,Pos_X++)
        {
          Couleur=(Pinceau_Sprite[(Pos_Y*TAILLE_MAXI_PINCEAU)+Pos_X])?CM_Noir:CM_Clair;
          Pixel_dans_menu(Pos_menu_X,Pos_menu_Y,Couleur);
          BLOCK_MENU[Pos_menu_Y][Pos_menu_X]=Couleur;
        }
  }
  SDL_UpdateRect(Ecran_SDL,0,Menu_Ordonnee,LARGEUR_SPRITE_MENU*Menu_Facteur_X+3,HAUTEUR_SPRITE_MENU*Menu_Facteur_Y+3);
}

  // -- Dessiner un pinceau prédéfini dans la fenêtre --

void Afficher_pinceau_dans_fenetre(word X,word Y,int Numero)
  // Pinceau = 0..NB_SPRITES_PINCEAU-1 : Pinceau prédéfini
{
  word Pos_X;
  word Pos_Y;
  word Pos_fenetre_X;
  word Pos_fenetre_Y;

  word Orig_X = X + 8 - Pinceau_predefini_Decalage_X[Numero];
  word Orig_Y = Y + 8 - Pinceau_predefini_Decalage_Y[Numero];

  for (Pos_fenetre_Y=Orig_Y,Pos_Y=0; Pos_Y<Pinceau_predefini_Hauteur[Numero]; Pos_fenetre_Y++,Pos_Y++)
    for (Pos_fenetre_X=Orig_X,Pos_X=0; Pos_X<Pinceau_predefini_Largeur[Numero]; Pos_fenetre_X++,Pos_X++)
      Pixel_dans_fenetre(Pos_fenetre_X,Pos_fenetre_Y,(SPRITE_PINCEAU[Numero][Pos_Y][Pos_X])?CM_Noir:CM_Clair);

  SDL_UpdateRect(Ecran_SDL, ToWinX(Orig_X), ToWinY(Orig_Y),
        ToWinL(Pinceau_predefini_Largeur[Numero]), 
        ToWinH(Pinceau_predefini_Hauteur[Numero])
  );
}

  // -- Dessiner des zigouigouis --

void Dessiner_zigouigoui(word X,word Y, byte Couleur, short Sens)
{
  word i;

  for (i=0; i<11; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens;
  for (i=1; i<10; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens+Sens;
  for (i=3; i<8; i++) Pixel_dans_fenetre(X,Y+i,Couleur);
  X+=Sens+Sens;
  Pixel_dans_fenetre(X,Y+5,Couleur);
}

  // -- Dessiner un bloc de couleurs dégradé verticalement

void Bloc_degrade_dans_fenetre(word Pos_X,word Pos_Y,word Debut_block,word Fin_block)
{
  word Total_lignes  =Menu_Facteur_Y<<6; // <=> à 64 lignes fct(Menu_Facteur)
  word Nb_couleurs   =(Debut_block<=Fin_block)?Fin_block-Debut_block+1:Debut_block-Fin_block+1;
  word Ligne_en_cours=(Debut_block<=Fin_block)?0:Total_lignes-1;

  word Debut_X       =Fenetre_Pos_X+(Menu_Facteur_X*Pos_X);
  word Largeur_ligne =Menu_Facteur_X<<4; // <=> à 16 pixels fct(Menu_Facteur)

  word Debut_Y       =Fenetre_Pos_Y+(Menu_Facteur_Y*Pos_Y);
  word Fin_Y         =Debut_Y+Total_lignes;
  word Indice;

  if (Debut_block>Fin_block)
  {
    Indice=Debut_block;
    Debut_block=Fin_block;
    Fin_block=Indice;
  }

  for (Indice=Debut_Y;Indice<Fin_Y;Indice++,Ligne_en_cours++)
    Block(Debut_X,Indice,Largeur_ligne,1,Debut_block+(Nb_couleurs*Ligne_en_cours)/Total_lignes);

  SDL_UpdateRect(Ecran_SDL,ToWinX(Pos_X),ToWinY(Pos_Y),ToWinL(16),ToWinH(64));
}



  // -- Redimentionner l'image (nettoie l'écran virtuel) --

void Redimentionner_image(word Largeur_choisie,word Hauteur_choisie)
{
  word Ancienne_largeur=Principal_Largeur_image;
  word Ancienne_hauteur=Principal_Hauteur_image;

  // +-+-+
  // |C| |  A+B+C = Ancienne image
  // +-+A|
  // |B| |    C   = Nouvelle image
  // +-+-+

  if (Backup_avec_nouvelles_dimensions(1,Largeur_choisie,Hauteur_choisie))
  {
    // La nouvelle page a pu être allouée, elle est pour l'instant pleine de
    // 0s. Elle fait Principal_Largeur_image de large.

    // On copie donc maintenant la partie C dans la nouvelle image.
    Copier_une_partie_d_image_dans_une_autre(
      Ecran_backup,0,0,Min(Ancienne_largeur,Principal_Largeur_image),
      Min(Ancienne_hauteur,Principal_Hauteur_image),Ancienne_largeur,
      Principal_Ecran,0,0,Principal_Largeur_image);
  }
  else
  {
    // Afficher un message d'erreur
    Afficher_curseur();
    Message_Memoire_insuffisante();
    Effacer_curseur();
  }
}

  // -- Dessiner un petit sprite représentant le type d'un drive --

void Fenetre_Afficher_sprite_drive(word Pos_X,word Pos_Y,byte Type)
{
  word i,j;

  for (j=0; j<HAUTEUR_SPRITE_DRIVE; j++)
    for (i=0; i<LARGEUR_SPRITE_DRIVE; i++)
      Pixel_dans_fenetre(Pos_X+i,Pos_Y+j,SPRITE_DRIVE[Type][j][i]);
  SDL_UpdateRect(Ecran_SDL,ToWinX(Pos_X),ToWinY(Pos_Y),ToWinL(LARGEUR_SPRITE_DRIVE),ToWinH(HAUTEUR_SPRITE_DRIVE));
}


void Capturer_brosse(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,short Effacement)
{
  short Temporaire;
  short Pos_X;
  short Pos_Y;
  word  Nouvelle_Brosse_Largeur;
  word  Nouvelle_Brosse_Hauteur;


  // On commence par "redresser" les bornes:
  if (Debut_X>Fin_X)
  {
    Temporaire=Debut_X;
    Debut_X   =Fin_X;
    Fin_X     =Temporaire;
  }
  if (Debut_Y>Fin_Y)
  {
    Temporaire=Debut_Y;
    Debut_Y   =Fin_Y;
    Fin_Y     =Temporaire;
  }

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((Debut_X<Principal_Largeur_image) && (Debut_Y<Principal_Hauteur_image))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_Decalage_X=0;
    Tiling_Decalage_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    Nouvelle_Brosse_Largeur=(Fin_X-Debut_X)+1;
    Nouvelle_Brosse_Hauteur=(Fin_Y-Debut_Y)+1;

    if (Debut_X+Nouvelle_Brosse_Largeur>Principal_Largeur_image)
      Nouvelle_Brosse_Largeur=Principal_Largeur_image-Debut_X;
    if (Debut_Y+Nouvelle_Brosse_Hauteur>Principal_Hauteur_image)
      Nouvelle_Brosse_Hauteur=Principal_Hauteur_image-Debut_Y;

    if ( (((long)Brosse_Hauteur)*Brosse_Largeur) !=
         (((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur) )
    {
      free(Brosse);
      Brosse=(byte *)malloc(((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur);
      if (!Brosse)
      {
        Erreur(0);

        Brosse=(byte *)malloc(1*1);
        Nouvelle_Brosse_Hauteur=Nouvelle_Brosse_Largeur=1;
        *Brosse=Fore_color;
      }
    }
    Brosse_Largeur=Nouvelle_Brosse_Largeur;
    Brosse_Hauteur=Nouvelle_Brosse_Hauteur;

    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Copier_image_dans_brosse(Debut_X,Debut_Y,Brosse_Largeur,Brosse_Hauteur,Principal_Largeur_image);

    // On regarde s'il faut effacer quelque chose:
    if (Effacement)
    {
      for (Pos_Y=Debut_Y;Pos_Y<Debut_Y+Brosse_Hauteur;Pos_Y++)
        for (Pos_X=Debut_X;Pos_X<Debut_X+Brosse_Largeur;Pos_X++)
        {
          Pixel_dans_ecran_courant(Pos_X,Pos_Y,Back_color);
          Pixel_Preview           (Pos_X,Pos_Y,Back_color);
        }
    }

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
}


void Rotate_90_deg()
{
  short Temporaire;
  byte * Nouvelle_Brosse;

  Nouvelle_Brosse=(byte *)malloc(((long)Brosse_Hauteur)*Brosse_Largeur);
  if (Nouvelle_Brosse)
  {
    Rotate_90_deg_LOWLEVEL(Brosse,Nouvelle_Brosse);
    free(Brosse);
    Brosse=Nouvelle_Brosse;

    Temporaire=Brosse_Largeur;
    Brosse_Largeur=Brosse_Hauteur;
    Brosse_Hauteur=Temporaire;

    Temporaire=Smear_Brosse_Largeur;
    Smear_Brosse_Largeur=Smear_Brosse_Hauteur;
    Smear_Brosse_Hauteur=Temporaire;

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
    Erreur(0);
}


void Remap_brosse(void)
{
  short Pos_X; // Variable de balayage de la brosse
  short Pos_Y; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;


  // On commence par initialiser le tableau de booléens à faux
  for (Couleur=0;Couleur<=255;Couleur++)
    Utilisee[Couleur]=0;

  // On calcule la table d'utilisation des couleurs
  for (Pos_Y=0;Pos_Y<Brosse_Hauteur;Pos_Y++)
    for (Pos_X=0;Pos_X<Brosse_Largeur;Pos_X++)
      Utilisee[Lit_pixel_dans_brosse(Pos_X,Pos_Y)]=1;

  //  On n'est pas censé remapper la couleur de transparence, sinon la brosse
  // changera de forme, donc on dit pour l'instant qu'elle n'est pas utilisée
  // ainsi on ne s'embêtera pas à la recalculer
  Utilisee[Back_color]=0;

  //   On va maintenant se servir de la table "Utilisee" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans la brosse, donc elles
  //       ne seront pas utilisées par Remap_brush_LOWLEVEL.
  for (Couleur=0;Couleur<=255;Couleur++)
    if (Utilisee[Couleur])
      Utilisee[Couleur]=Meilleure_couleur(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].V,Brouillon_Palette[Couleur].B);
      //Utilisee[Couleur]=Meilleure_couleur_sans_exclusion(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].V,Brouillon_Palette[Couleur].B);

  //   Il reste une couleur non calculée dans la table qu'il faut mettre à
  // jour: c'est la couleur de fond. On l'avait inhibée pour éviter son
  // calcul inutile, mais comme il est possible qu'elle soit quand même dans
  // la brosse, on va mettre dans la table une relation d'équivalence entre
  // les deux palettes: comme on ne veut pas que la couleur soit remplacée,
  // on va dire qu'on veut qu'elle soit remplacée par la couleur en question.
  Utilisee[Back_color]=Back_color;

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_LOWLEVEL(Utilisee,Brosse,Brosse_Largeur,Brosse_Hauteur,Brosse_Largeur);
  //Remap_brush_LOWLEVEL(Utilisee);
}



void Remap_picture(void)
{
  short Pos_X; // Variable de balayage de la brosse
  short Pos_Y; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;

  // On commence par initialiser le tableau de booléens à faux
  for (Couleur=0;Couleur<=255;Couleur++)
    Utilisee[Couleur]=0;

  // On calcule la table d'utilisation des couleurs
  for (Pos_Y=0;Pos_Y<Brouillon_Hauteur_image;Pos_Y++)
    for (Pos_X=0;Pos_X<Brouillon_Largeur_image;Pos_X++)
      Utilisee[Lit_pixel_dans_ecran_brouillon(Pos_X,Pos_Y)]=1;

  //   On va maintenant se servir de la table "Utilisee" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans l'image, donc elles
  //       ne seront pas utilisées par Remap_general_LOWLEVEL.
  for (Couleur=0;Couleur<=255;Couleur++)
    if (Utilisee[Couleur])
      Utilisee[Couleur]=Meilleure_couleur(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].V,Brouillon_Palette[Couleur].B);
      //Utilisee[Couleur]=Meilleure_couleur_sans_exclusion(Brouillon_Palette[Couleur].R,Brouillon_Palette[Couleur].V,Brouillon_Palette[Couleur].B);

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  Remap_general_LOWLEVEL(Utilisee,Brouillon_Ecran,Brouillon_Largeur_image,Brouillon_Hauteur_image,Brouillon_Largeur_image);
}



void Get_colors_from_brush(void)
{
  short Pos_X; // Variable de balayage de la brosse
  short Pos_Y; // Variable de balayage de la brosse
  byte  Utilisee[256]; // Tableau de booléens "La couleur est utilisée"
  int   Couleur;

  if (Demande_de_confirmation("Modify current palette ?"))
  {
    Backup();

    // On commence par initialiser le tableau de booléen à faux
    for (Couleur=0;Couleur<=255;Couleur++)
      Utilisee[Couleur]=0;

    // On calcule la table d'utilisation des couleurs
    for (Pos_Y=0;Pos_Y<Brosse_Hauteur;Pos_Y++)
      for (Pos_X=0;Pos_X<Brosse_Largeur;Pos_X++)
        Utilisee[Lit_pixel_dans_brosse(Pos_X,Pos_Y)]=1;

    // On recopie dans la palette principale les teintes des couleurs utilisées
    // dans la palette du brouillon
    for (Couleur=0;Couleur<=255;Couleur++)
      if (Utilisee[Couleur])
      {
        Principal_Palette[Couleur].R=Brouillon_Palette[Couleur].R;
        Principal_Palette[Couleur].V=Brouillon_Palette[Couleur].V;
        Principal_Palette[Couleur].B=Brouillon_Palette[Couleur].B;
      }

    Set_palette(Principal_Palette);
    Calculer_couleurs_menu_optimales(Principal_Palette);
    Effacer_curseur();
    Afficher_ecran();
    Afficher_menu();
    Afficher_curseur();

    Principal_Image_modifiee=1;
  }
}



void Outline_brush(void)
{
  long /*Pos,*/Pos_X,Pos_Y;
  byte Etat;
  byte * Nouvelle_brosse;
  byte * Temporaire;
  word Largeur;
  word Hauteur;


  Largeur=Brosse_Largeur+2;
  Hauteur=Brosse_Hauteur+2;
  Nouvelle_brosse=(byte *)malloc(((long)Largeur)*Hauteur);

  if (Nouvelle_brosse)
  {
    // On remplit la bordure ajoutée par la Backcolor
    memset(Nouvelle_brosse,Back_color,((long)Largeur)*Hauteur);

    // On copie la brosse courante dans la nouvelle
    Copier_une_partie_d_image_dans_une_autre(Brosse, // Source
                                             0, 0,
                                             Brosse_Largeur,
                                             Brosse_Hauteur,
                                             Brosse_Largeur,
                                             Nouvelle_brosse, // Destination
                                             1, 1,
                                             Largeur);

    // On intervertit la nouvelle et l'ancienne brosse:
    Temporaire=Brosse;
    Brosse=Nouvelle_brosse;
    Brosse_Largeur+=2;
    Brosse_Hauteur+=2;
    Largeur-=2;
    Hauteur-=2;

    // Si on "outline" avec une couleur différente de la Back_color on y va!
    if (Fore_color!=Back_color)
    {
      // 1er balayage (horizontal)
      for (Pos_Y=1; Pos_Y<Brosse_Hauteur-1; Pos_Y++)
      {
        Etat=0;
        for (Pos_X=1; Pos_X<Brosse_Largeur-1; Pos_X++)
        {
          if (Temporaire[((Pos_Y-1)*Largeur)+Pos_X-1]==Back_color)
          {
            if (Etat)
            {
              Pixel_dans_brosse(Pos_X,Pos_Y,Fore_color);
              Etat=0;
            }
          }
          else
          {
            if (!Etat)
            {
              Pixel_dans_brosse(Pos_X-1,Pos_Y,Fore_color);
              Etat=1;
            }
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (Etat)
          Pixel_dans_brosse(Pos_X,Pos_Y,Fore_color);
      }

      // 2ème balayage (vertical)
      for (Pos_X=1; Pos_X<Brosse_Largeur-1; Pos_X++)
      {
        Etat=0;
        for (Pos_Y=1; Pos_Y<Brosse_Hauteur-1; Pos_Y++)
        {
          if (Temporaire[((Pos_Y-1)*Largeur)+Pos_X-1]==Back_color)
          {
            if (Etat)
            {
              Pixel_dans_brosse(Pos_X,Pos_Y,Fore_color);
              Etat=0;
            }
          }
          else
          {
            if (!Etat)
            {
              Pixel_dans_brosse(Pos_X,Pos_Y-1,Fore_color);
              Etat=1;
            }
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (Etat)
          Pixel_dans_brosse(Pos_X,Pos_Y,Fore_color);
      }
    }

    // On recentre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);

    free(Temporaire); // Libération de l'ancienne brosse

    // Réallocation d'un buffer de Smear
    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Largeur)*Smear_Brosse_Hauteur);
  }
  else
    Erreur(0); // Pas assez de mémoire!
}


void Nibble_brush(void)
{
  long /*Pos,*/Pos_X,Pos_Y;
  byte Etat;
  byte * Nouvelle_brosse;
  byte * Temporaire;
  word Largeur;
  word Hauteur;

  if ( (Brosse_Largeur>2) && (Brosse_Hauteur>2) )
  {
    Largeur=Brosse_Largeur-2;
    Hauteur=Brosse_Hauteur-2;
    Nouvelle_brosse=(byte *)malloc(((long)Largeur)*Hauteur);

    if (Nouvelle_brosse)
    {
      // On copie la brosse courante dans la nouvelle
      Copier_une_partie_d_image_dans_une_autre(Brosse, // Source
                                               1,
                                               1,
                                               Largeur,
                                               Hauteur,
                                               Brosse_Largeur,
                                               Nouvelle_brosse, // Destination
                                               0,
                                               0,
                                               Largeur);

      // On intervertit la nouvelle et l'ancienne brosse:
      Temporaire=Brosse;
      Brosse=Nouvelle_brosse;
      Brosse_Largeur-=2;
      Brosse_Hauteur-=2;
      Largeur+=2;
      Hauteur+=2;

      // 1er balayage (horizontal)
      for (Pos_Y=0; Pos_Y<Brosse_Hauteur; Pos_Y++)
      {
        Etat=(Temporaire[(Pos_Y+1)*Largeur]!=Back_color);
        for (Pos_X=0; Pos_X<Brosse_Largeur; Pos_X++)
        {
          if (Temporaire[((Pos_Y+1)*Largeur)+Pos_X+1]==Back_color)
          {
            if (Etat)
            {
              if (Pos_X>0)
                Pixel_dans_brosse(Pos_X-1,Pos_Y,Back_color);
              Etat=0;
            }
          }
          else
          {
            if (!Etat)
            {
              Pixel_dans_brosse(Pos_X,Pos_Y,Back_color);
              Etat=1;
            }
          }
        }
        // Cas du dernier pixel à droite de la ligne
        if (Temporaire[((Pos_Y+1)*Largeur)+Pos_X+1]==Back_color)
          Pixel_dans_brosse(Pos_X-1,Pos_Y,Back_color);
      }

      // 2ème balayage (vertical)
      for (Pos_X=0; Pos_X<Brosse_Largeur; Pos_X++)
      {
        Etat=(Temporaire[Largeur+Pos_X+1]!=Back_color);;
        for (Pos_Y=0; Pos_Y<Brosse_Hauteur; Pos_Y++)
        {
          if (Temporaire[((Pos_Y+1)*Largeur)+Pos_X+1]==Back_color)
          {
            if (Etat)
            {
              if (Pos_Y>0)
                Pixel_dans_brosse(Pos_X,Pos_Y-1,Back_color);
              Etat=0;
            }
          }
          else
          {
            if (!Etat)
            {
              Pixel_dans_brosse(Pos_X,Pos_Y,Back_color);
              Etat=1;
            }
          }
        }
        // Cas du dernier pixel en bas de la colonne
        if (Temporaire[((Pos_Y+1)*Largeur)+Pos_X+1]==Back_color)
          Pixel_dans_brosse(Pos_X,Pos_Y-1,Back_color);
      }

      // On recentre la prise sur la brosse
      Brosse_Decalage_X=(Brosse_Largeur>>1);
      Brosse_Decalage_Y=(Brosse_Hauteur>>1);

      free(Temporaire); // Libération de l'ancienne brosse

      // Réallocation d'un buffer de Smear
      free(Smear_Brosse);
      Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
      Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Largeur)*Smear_Brosse_Hauteur);
    }
    else
      Erreur(0);  // Pas assez de mémoire!
  }
}


//////////////////////////////////////////////////////////////////////////////
////////////////////////////// GESTION DU FILLER /////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void Fill(short * Limite_atteinte_Haut  , short * Limite_atteinte_Bas,
          short * Limite_atteinte_Gauche, short * Limite_atteinte_Droite)
//
//   Cette fonction fait un remplissage classique d'une zone délimitée de
// l'image. Les limites employées sont Limite_Haut, Limite_Bas, Limite_Gauche
// et Limite_Droite. Le point de départ du remplissage est Pinceau_X,Pinceau_Y
// et s'effectue en théorie sur la couleur 1 et emploie la couleur 2 pour le
// remplissage. Ces restrictions sont dûes à l'utilisation qu'on en fait dans
// la fonction principale "Remplir", qui se charge de faire une gestion de
// tous les effets.
//   Cette fonction ne doit pas être directement appelée.
//
{
  short Pos_X;   // Abscisse de balayage du segment, utilisée lors de l'"affichage"
  short Ligne;   // Ordonnée de la ligne en cours de traitement
  short Debut_X; // Abscisse de départ du segment traité
  short Fin_X;   // Abscisse de fin du segment traité
  int   Modifs_effectuees;    // Booléen "On a fait une modif dans le dernier passage"
  int   Propagation_possible; // Booléen "On peut propager la couleur dans le segment"
  short Limite_courante_Bas;  // Intervalle vertical restreint
  short Limite_courante_Haut;
  int   Ligne_modifiee;       // Booléen "On a fait une modif dans la ligne"

  Modifs_effectuees=1;
  Limite_courante_Haut=Pinceau_Y;
  Limite_courante_Bas =Min(Pinceau_Y+1,Limite_Bas);
  *Limite_atteinte_Gauche=Pinceau_X;
  *Limite_atteinte_Droite=Pinceau_X+1;
  Pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y,2);

  while (Modifs_effectuees)
  {
    Modifs_effectuees=0;

    for (Ligne=Limite_courante_Haut;Ligne<=Limite_courante_Bas;Ligne++)
    {
      Ligne_modifiee=0;
      // On va traiter le cas de la ligne n° Ligne.

      // On commence le traitement à la gauche de l'écran
      Debut_X=Limite_Gauche;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (Debut_X<=Limite_Droite)
      {
        // On cherche son début
        while((Debut_X<=Limite_Droite) && 
                (Lit_pixel_dans_ecran_courant(Debut_X,Ligne)!=1))
             Debut_X++;

        if (Debut_X<=Limite_Droite)
        {
          // Un segment de couleur 1 existe et commence à la position Debut_X.
          // On va donc en chercher la fin.
          for (Fin_X=Debut_X+1;(Fin_X<=Limite_Droite) &&
               (Lit_pixel_dans_ecran_courant(Fin_X,Ligne)==1);Fin_X++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // Debut_X et qui se termine en Fin_X-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          Propagation_possible=(
            // Test de la présence d'un point à gauche du segment
            ((Debut_X>Limite_Gauche) &&
             (Lit_pixel_dans_ecran_courant(Debut_X-1,Ligne)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((Fin_X-1<Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Fin_X    ,Ligne)==2))
                               );

          // Test de la présence d'un point en haut du segment
          if (!Propagation_possible && (Ligne>Limite_Haut))
            for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
              if (Lit_pixel_dans_ecran_courant(Pos_X,Ligne-1)==2)
              {
                Propagation_possible=1;
                break;
              }

          if (Propagation_possible)
          {
            if (Debut_X<*Limite_atteinte_Gauche)
              *Limite_atteinte_Gauche=Debut_X;
            if (Fin_X>*Limite_atteinte_Droite)
              *Limite_atteinte_Droite=Fin_X;
            // On remplit le segment de Debut_X à Fin_X-1.
            for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
              Pixel_dans_ecran_courant(Pos_X,Ligne,2);
            // On vient d'effectuer des modifications.
            Modifs_effectuees=1;
            Ligne_modifiee=1;
          }

          Debut_X=Fin_X+1;
        }
      }

      // Si on est en bas, et qu'on peut se propager vers le bas...
      if ( (Ligne==Limite_courante_Bas) &&
           (Ligne_modifiee) &&
           (Limite_courante_Bas<Limite_Bas) )
        Limite_courante_Bas++; // On descend cette limite vers le bas
    }

    // Pour le prochain balayage vers le haut, on va se permettre d'aller
    // voir une ligne plus haut.
    // Si on ne le fait pas, et que la première ligne (Limite_courante_Haut)
    // n'était pas modifiée, alors cette limite ne serait pas remontée, donc
    // le filler ne progresserait pas vers le haut.
    if (Limite_courante_Haut>Limite_Haut)
      Limite_courante_Haut--;

    for (Ligne=Limite_courante_Bas;Ligne>=Limite_courante_Haut;Ligne--)
    {
      Ligne_modifiee=0;
      // On va traiter le cas de la ligne n° Ligne.

      // On commence le traitement à la gauche de l'écran
      Debut_X=Limite_Gauche;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (Debut_X<=Limite_Droite)
      {
        // On cherche son début
        for (;(Debut_X<=Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Debut_X,Ligne)!=1);Debut_X++);

        if (Debut_X<=Limite_Droite)
        {
          // Un segment de couleur 1 existe et commence à la position Debut_X.
          // On va donc en chercher la fin.
          for (Fin_X=Debut_X+1;(Fin_X<=Limite_Droite) &&
               (Lit_pixel_dans_ecran_courant(Fin_X,Ligne)==1);Fin_X++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // Debut_X et qui se termine en Fin_X-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          Propagation_possible=(
            // Test de la présence d'un point à gauche du segment
            ((Debut_X>Limite_Gauche) &&
             (Lit_pixel_dans_ecran_courant(Debut_X-1,Ligne)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((Fin_X-1<Limite_Droite) &&
             (Lit_pixel_dans_ecran_courant(Fin_X    ,Ligne)==2))
                               );

          // Test de la présence d'un point en bas du segment
          if (!Propagation_possible && (Ligne<Limite_Bas))
            for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
              if (Lit_pixel_dans_ecran_courant(Pos_X,Ligne+1)==2)
              {
                Propagation_possible=1;
                break;
              }

          if (Propagation_possible)
          {
            if (Debut_X<*Limite_atteinte_Gauche)
              *Limite_atteinte_Gauche=Debut_X;
            if (Fin_X>*Limite_atteinte_Droite)
              *Limite_atteinte_Droite=Fin_X;
            // On remplit le segment de Debut_X à Fin_X-1.
            for (Pos_X=Debut_X;Pos_X<Fin_X;Pos_X++)
              Pixel_dans_ecran_courant(Pos_X,Ligne,2);
            // On vient d'effectuer des modifications.
            Modifs_effectuees=1;
            Ligne_modifiee=1;
          }

          Debut_X=Fin_X+1;
        }
      }

      // Si on est en haut, et qu'on peut se propager vers le haut...
      if ( (Ligne==Limite_courante_Haut) &&
           (Ligne_modifiee) &&
           (Limite_courante_Haut>Limite_Haut) )
        Limite_courante_Haut--; // On monte cette limite vers le haut
    }
  }

  *Limite_atteinte_Haut=Limite_courante_Haut;
  *Limite_atteinte_Bas =Limite_courante_Bas;
  (*Limite_atteinte_Droite)--;
} // Fin de la routine de remplissage "Fill"


void Remplir(byte Couleur_de_remplissage)
//
//  Cette fonction fait un remplissage qui gère tous les effets. Elle fait
// appel à "Fill()".
//
{
  byte   Forme_curseur_avant_remplissage;
  byte * FX_Feedback_Ecran_avant_remplissage;
  short  Pos_X,Pos_Y;
  short  Limite_atteinte_Haut  ,Limite_atteinte_Bas;
  short  Limite_atteinte_Gauche,Limite_atteinte_Droite;
  byte   Table_de_remplacement[256];


  // Avant toute chose, on vérifie que l'on n'est pas en train de remplir
  // en dehors de l'image:

  if ( (Pinceau_X>=Limite_Gauche) &&
       (Pinceau_X<=Limite_Droite) &&
       (Pinceau_Y>=Limite_Haut)   &&
       (Pinceau_Y<=Limite_Bas) )
  {
    // On suppose que le curseur est déjà caché.
    // Effacer_curseur();

    //   On va faire patienter l'utilisateur en lui affichant un joli petit
    // sablier:
    Forme_curseur_avant_remplissage=Forme_curseur;
    Forme_curseur=FORME_CURSEUR_SABLIER;
    Afficher_curseur();

    // On commence par effectuer un backup de l'image.
    Backup();

    // On fait attention au Feedback qui DOIT se faire avec le backup.
    FX_Feedback_Ecran_avant_remplissage=FX_Feedback_Ecran;
    FX_Feedback_Ecran=Ecran_backup;

    // On va maintenant "épurer" la zone visible de l'image:
    memset(Table_de_remplacement,0,256);
    Table_de_remplacement[Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y)]=1;
    Remplacer_toutes_les_couleurs_dans_limites(Table_de_remplacement);

    // On fait maintenant un remplissage classique de la couleur 1 avec la 2
   Fill(&Limite_atteinte_Haut  ,&Limite_atteinte_Bas,
         &Limite_atteinte_Gauche,&Limite_atteinte_Droite);

    //  On s'apprête à faire des opérations qui nécessitent un affichage. Il
    // faut donc retirer de l'écran le curseur:
    Effacer_curseur();
    Forme_curseur=Forme_curseur_avant_remplissage;

    //  Il va maintenant falloir qu'on "turn" ce gros caca "into" un truc qui
    // ressemble un peu plus à ce à quoi l'utilisateur peut s'attendre.
    if (Limite_atteinte_Haut>Limite_Haut)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,					// Source
                                               Limite_Gauche,Limite_Haut,			// Pos X et Y dans source
                                               (Limite_Droite-Limite_Gauche)+1,			// Largeur copie
                                               Limite_atteinte_Haut-Limite_Haut,		// Hauteur copie
                                               Principal_Largeur_image,				// Largeur de la source
					       Principal_Ecran,					// Destination
                                               Limite_Gauche,Limite_Haut,			// Pos X et Y destination
					       Principal_Largeur_image);			// Largeur destination
    if (Limite_atteinte_Bas<Limite_Bas)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_Gauche,Limite_atteinte_Bas+1,
                                               (Limite_Droite-Limite_Gauche)+1,
                                               Limite_Bas-Limite_atteinte_Bas,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_Gauche,Limite_atteinte_Bas+1,Principal_Largeur_image);
    if (Limite_atteinte_Gauche>Limite_Gauche)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_Gauche,Limite_atteinte_Haut,
                                               Limite_atteinte_Gauche-Limite_Gauche,
                                               (Limite_atteinte_Bas-Limite_atteinte_Haut)+1,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_Gauche,Limite_atteinte_Haut,Principal_Largeur_image);
    if (Limite_atteinte_Droite<Limite_Droite)
      Copier_une_partie_d_image_dans_une_autre(Ecran_backup,
                                               Limite_atteinte_Droite+1,Limite_atteinte_Haut,
                                               Limite_Droite-Limite_atteinte_Droite,
                                               (Limite_atteinte_Bas-Limite_atteinte_Haut)+1,
                                               Principal_Largeur_image,Principal_Ecran,
                                               Limite_atteinte_Droite+1,Limite_atteinte_Haut,Principal_Largeur_image);

    for (Pos_Y=Limite_atteinte_Haut;Pos_Y<=Limite_atteinte_Bas;Pos_Y++)
      for (Pos_X=Limite_atteinte_Gauche;Pos_X<=Limite_atteinte_Droite;Pos_X++)
        if (Lit_pixel_dans_ecran_courant(Pos_X,Pos_Y)==2)
        {
          //   Si le pixel en cours de traitement a été touché par le Fill()
          // on se doit d'afficher le pixel modifié par la couleur de
          // remplissage:

          //  Ceci se fait en commençant par restaurer la couleur qu'il y avait
          // précédemment (c'est important pour que les effets ne s'emmèlent
          // pas le pinceaux)
          Pixel_dans_ecran_courant(Pos_X,Pos_Y,Lit_pixel_dans_ecran_backup(Pos_X,Pos_Y));

          //  Enfin, on peut afficher le pixel, en le soumettant aux effets en
          // cours:
          Afficher_pixel(Pos_X,Pos_Y,Couleur_de_remplissage);
        }
        else
          Pixel_dans_ecran_courant(Pos_X,Pos_Y,Lit_pixel_dans_ecran_backup(Pos_X,Pos_Y));

    FX_Feedback_Ecran=FX_Feedback_Ecran_avant_remplissage;

    //   A la fin, on n'a pas besoin de réafficher le curseur puisque c'est
    // l'appelant qui s'en charge, et on n'a pas besoin de rafficher l'image
    // puisque les seuls points qui ont changé dans l'image ont été raffichés
    // par l'utilisation de "Afficher_pixel()", et que les autres... eh bein
    // on n'y a jamais touché à l'écran les autres: ils sont donc corrects.

    SDL_UpdateRect(Ecran_SDL,0,0,0,0);
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////// TRACéS DE FIGURES GéOMéTRIQUES STANDARDS //////////////////
////////////////////////// avec gestion de previews //////////////////////////
//////////////////////////////////////////////////////////////////////////////

  // Définition d'une fonction générique de traçage de figures:
  fonction_afficheur Pixel_figure;

  // Affichage d'un point de façon définitive (utilisation du pinceau)
  void inline Pixel_figure_Definitif(word Pos_X,word Pos_Y,byte Couleur)
  {
    Afficher_pinceau(Pos_X,Pos_Y,Couleur,0);
  }

  // Affichage d'un point pour une preview
  void Pixel_figure_Preview(word Pos_X,word Pos_Y,byte Couleur)
  {
    if ( (Pos_X>=Limite_Gauche) &&
         (Pos_X<=Limite_Droite) &&
         (Pos_Y>=Limite_Haut)   &&
         (Pos_Y<=Limite_Bas) )
      Pixel_Preview(Pos_X,Pos_Y,Couleur);
  }

  // Affichage d'un point pour une preview en xor
  void Pixel_figure_Preview_xor(word Pos_X,word Pos_Y,byte Couleur)
  {
    if ( (Pos_X>=Limite_Gauche) &&
         (Pos_X<=Limite_Droite) &&
         (Pos_Y>=Limite_Haut)   &&
         (Pos_Y<=Limite_Bas) )
      Pixel_Preview(Pos_X,Pos_Y,~Lit_pixel(Pos_X-Principal_Decalage_X,
                                           Pos_Y-Principal_Decalage_Y));
  }

  // Effacement d'un point de preview
  void Pixel_figure_Effacer_preview(word Pos_X,word Pos_Y,byte Couleur)
  {
    if ( (Pos_X>=Limite_Gauche) &&
         (Pos_X<=Limite_Droite) &&
         (Pos_Y>=Limite_Haut)   &&
         (Pos_Y<=Limite_Bas) )
      Pixel_Preview(Pos_X,Pos_Y,Lit_pixel_dans_ecran_courant(Pos_X,Pos_Y));
  }

  // Affichage d'un point dans la brosse
  void Pixel_figure_Dans_brosse(word Pos_X,word Pos_Y,byte Couleur)
  {
    Pos_X-=Brosse_Decalage_X;
    Pos_Y-=Brosse_Decalage_Y;
    if ( (Pos_X<Brosse_Largeur) && // Les pos sont des word donc jamais < 0 ...
         (Pos_Y<Brosse_Hauteur) )
      Pixel_dans_brosse(Pos_X,Pos_Y,Couleur);
  }


  // -- Tracer général d'un cercle vide -------------------------------------

void Tracer_cercle_vide_General(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short Pos_X;
  short Pos_Y;

  // Ensuite, on va parcourire le quart haut gauche du cercle
  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;

  // Affichage des extremitées du cercle sur chaque quart du cercle:
  for (Pos_Y=Debut_Y,Cercle_Curseur_Y=-Rayon;Pos_Y<Centre_Y;Pos_Y++,Cercle_Curseur_Y++)
    for (Pos_X=Debut_X,Cercle_Curseur_X=-Rayon;Pos_X<Centre_X;Pos_X++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
      {
        // On vient de tomber sur le premier point sur la ligne horizontale
        // qui fait partie du cercle.
        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(Pos_X,Pos_Y,Couleur);
         // Quart Haut-droite
        Pixel_figure((Centre_X<<1)-Pos_X,Pos_Y,Couleur);
         // Quart Bas-droite
        Pixel_figure((Centre_X<<1)-Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
         // Quart Bas-gauche
        Pixel_figure(Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas au cercle:
        for (Cercle_Curseur_Y--,Pos_X++,Cercle_Curseur_X++;Pos_X<Centre_X;Pos_X++,Cercle_Curseur_X++)
          if (!Pixel_dans_cercle())
          {
             // Quart Haut-gauche
            Pixel_figure(Pos_X,Pos_Y,Couleur);
             // Quart Haut-droite
            Pixel_figure((Centre_X<<1)-Pos_X,Pos_Y,Couleur);
             // Quart Bas-gauche
            Pixel_figure(Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
             // Quart Bas-droite
            Pixel_figure((Centre_X<<1)-Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
          }
          else
            break;

        Cercle_Curseur_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:
  Pixel_figure(Centre_X,Centre_Y-Rayon,Couleur); // Haut
  Pixel_figure(Centre_X-Rayon,Centre_Y,Couleur); // Gauche
  Pixel_figure(Centre_X+Rayon,Centre_Y,Couleur); // Droite
  Pixel_figure(Centre_X,Centre_Y+Rayon,Couleur); // Bas

  if(Loupe_Mode) Mettre_Ecran_A_Jour(Centre_X-Rayon,Centre_Y-Rayon,2*Rayon+1,2*Rayon+1);
}

  // -- Tracé définitif d'un cercle vide --

void Tracer_cercle_vide_Definitif(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Tracer la preview d'un cercle vide --

void Tracer_cercle_vide_Preview(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Effacer la preview d'un cercle vide --

void Effacer_cercle_vide_Preview(short Centre_X,short Centre_Y,short Rayon)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_cercle_vide_General(Centre_X,Centre_Y,Rayon,0);
  Mettre_Ecran_A_Jour(Centre_X - Rayon, Centre_Y - Rayon, 2* Rayon+1, 2*Rayon+1);
}

  // -- Tracer un cercle plein --

void Tracer_cercle_plein(short Centre_X,short Centre_Y,short Rayon,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short Pos_X;
  short Pos_Y;
  short Fin_X;
  short Fin_Y;

  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;
  Fin_X=Centre_X+Rayon;
  Fin_Y=Centre_Y+Rayon;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage du cercle
  for (Pos_Y=Debut_Y,Cercle_Curseur_Y=(long)Debut_Y-Centre_Y;Pos_Y<=Fin_Y;Pos_Y++,Cercle_Curseur_Y++)
    for (Pos_X=Debut_X,Cercle_Curseur_X=(long)Debut_X-Centre_X;Pos_X<=Fin_X;Pos_X++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
        Afficher_pixel(Pos_X,Pos_Y,Couleur);

  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X+1-Debut_X,Fin_Y+1-Debut_Y);
}


  // -- Tracer général d'une ellipse vide -----------------------------------

void Tracer_ellipse_vide_General(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short Pos_X;
  short Pos_Y;

  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // Affichage des extremitées de l'ellipse sur chaque quart de l'ellipse:
  for (Pos_Y=Debut_Y,Ellipse_Curseur_Y=-Rayon_vertical;Pos_Y<Centre_Y;Pos_Y++,Ellipse_Curseur_Y++)
    for (Pos_X=Debut_X,Ellipse_Curseur_X=-Rayon_horizontal;Pos_X<Centre_X;Pos_X++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
      {
        // On vient de tomber sur le premier point qui sur la ligne
        // horizontale fait partie de l'ellipse.

        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(Pos_X,Pos_Y,Couleur);
         // Quart Haut-droite
        Pixel_figure((Centre_X<<1)-Pos_X,Pos_Y,Couleur);
         // Quart Bas-gauche
        Pixel_figure(Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
         // Quart Bas-droite
        Pixel_figure((Centre_X<<1)-Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas à l'ellipse:
        for (Ellipse_Curseur_Y--,Pos_X++,Ellipse_Curseur_X++;Pos_X<Centre_X;Pos_X++,Ellipse_Curseur_X++)
          if (!Pixel_dans_ellipse())
          {
             // Quart Haut-gauche
            Pixel_figure(Pos_X,Pos_Y,Couleur);
             // Quart Haut-droite
            Pixel_figure((Centre_X<<1)-Pos_X,Pos_Y,Couleur);
             // Quart Bas-gauche
            Pixel_figure(Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
             // Quart Bas-droite
            Pixel_figure((Centre_X<<1)-Pos_X,(Centre_Y<<1)-Pos_Y,Couleur);
          }
          else
            break;

        Ellipse_Curseur_Y++;
        break;
      }

  // On affiche à la fin les points cardinaux:

  // Points verticaux:
  Pos_X=Centre_X;
  Ellipse_Curseur_X=-1;
  for (Pos_Y=Centre_Y+1-Rayon_vertical,Ellipse_Curseur_Y=-Rayon_vertical+1;Pos_Y<Centre_Y+Rayon_vertical;Pos_Y++,Ellipse_Curseur_Y++)
    if (!Pixel_dans_ellipse())
      Pixel_figure(Pos_X,Pos_Y,Couleur);

  // Points horizontaux:
  Pos_Y=Centre_Y;
  Ellipse_Curseur_Y=-1;
  for (Pos_X=Centre_X+1-Rayon_horizontal,Ellipse_Curseur_X=-Rayon_horizontal+1;Pos_X<Centre_X+Rayon_horizontal;Pos_X++,Ellipse_Curseur_X++)
    if (!Pixel_dans_ellipse())
      Pixel_figure(Pos_X,Pos_Y,Couleur);

  Pixel_figure(Centre_X,Centre_Y-Rayon_vertical,Couleur);   // Haut
  Pixel_figure(Centre_X-Rayon_horizontal,Centre_Y,Couleur); // Gauche
  Pixel_figure(Centre_X+Rayon_horizontal,Centre_Y,Couleur); // Droite
  Pixel_figure(Centre_X,Centre_Y+Rayon_vertical,Couleur);   // Bas

  Mettre_Ecran_A_Jour(Centre_X-Rayon_horizontal,Centre_Y-Rayon_vertical,2*Rayon_horizontal+1,2*Rayon_vertical+1);
}

  // -- Tracé définitif d'une ellipse vide --

void Tracer_ellipse_vide_Definitif(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical+1);
}

  // -- Tracer la preview d'une ellipse vide --

void Tracer_ellipse_vide_Preview(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,Couleur);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical +1);
}

  // -- Effacer la preview d'une ellipse vide --

void Effacer_ellipse_vide_Preview(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_ellipse_vide_General(Centre_X,Centre_Y,Rayon_horizontal,Rayon_vertical,0);
  Mettre_Ecran_A_Jour(Centre_X - Rayon_horizontal, Centre_Y - Rayon_vertical, 2* Rayon_horizontal+1, 2*Rayon_vertical+1);
}

  // -- Tracer une ellipse pleine --

void Tracer_ellipse_pleine(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,byte Couleur)
{
  short Debut_X;
  short Debut_Y;
  short Pos_X;
  short Pos_Y;
  short Fin_X;
  short Fin_Y;

  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;
  Fin_X=Centre_X+Rayon_horizontal;
  Fin_Y=Centre_Y+Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage de l'ellipse
  for (Pos_Y=Debut_Y,Ellipse_Curseur_Y=Debut_Y-Centre_Y;Pos_Y<=Fin_Y;Pos_Y++,Ellipse_Curseur_Y++)
    for (Pos_X=Debut_X,Ellipse_Curseur_X=Debut_X-Centre_X;Pos_X<=Fin_X;Pos_X++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
        Afficher_pixel(Pos_X,Pos_Y,Couleur);
  Mettre_Ecran_A_Jour(Centre_X-Rayon_horizontal,Centre_Y-Rayon_vertical,2*Rayon_horizontal+1,2*Rayon_vertical+1);
}


  // -- Tracer général d'une ligne ------------------------------------------

void Tracer_ligne_General(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y, byte Couleur)
{
  short Pos_X,Pos_Y;
  short Incr_X,Incr_Y;
  short i,Cumul;
  short Delta_X,Delta_Y;


  Pos_X=Debut_X;
  Pos_Y=Debut_Y;

  if (Debut_X<Fin_X)
  {
    Incr_X=+1;
    Delta_X=Fin_X-Debut_X;
  }
  else
  {
    Incr_X=-1;
    Delta_X=Debut_X-Fin_X;
  }

  if (Debut_Y<Fin_Y)
  {
    Incr_Y=+1;
    Delta_Y=Fin_Y-Debut_Y;
  }
  else
  {
    Incr_Y=-1;
    Delta_Y=Debut_Y-Fin_Y;
  }

  if (Delta_Y>Delta_X)
  {
    Cumul=Delta_Y>>1;
    for (i=1; i<Delta_Y; i++)
    {
      Pos_Y+=Incr_Y;
      Cumul+=Delta_X;
      if (Cumul>=Delta_Y)
      {
        Cumul-=Delta_Y;
        Pos_X+=Incr_X;
      }
      Pixel_figure(Pos_X,Pos_Y,Couleur);
    }
  }
  else
  {
    Cumul=Delta_X>>1;
    for (i=1; i<Delta_X; i++)
    {
      Pos_X+=Incr_X;
      Cumul+=Delta_Y;
      if (Cumul>=Delta_X)
      {
        Cumul-=Delta_X;
        Pos_Y+=Incr_Y;
      }
      Pixel_figure(Pos_X,Pos_Y,Couleur);
    }
  }

  if ( (Debut_X!=Fin_X) || (Debut_Y!=Fin_Y) )
    Pixel_figure(Fin_X,Fin_Y,Couleur);

}

  // -- Tracer définitif d'une ligne --

void Tracer_ligne_Definitif(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y, byte Couleur)
{

  Pixel_figure=Pixel_figure_Definitif;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}

  // -- Tracer la preview d'une ligne --

void Tracer_ligne_Preview(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}

  // -- Tracer la preview d'une ligne en xor --

void Tracer_ligne_Preview_xor(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview_xor;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,Couleur);
//  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
  SDL_UpdateRect(Ecran_SDL,0,0,0,0);
}

  // -- Effacer la preview d'une ligne --

void Effacer_ligne_Preview(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_ligne_General(Debut_X,Debut_Y,Fin_X,Fin_Y,0);
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}


  // -- Tracer un rectangle vide --

void Tracer_rectangle_vide(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  short Tempo;
  short Pos_X;
  short Pos_Y;


  // On vérifie que les bornes soient dans le bon sens:
  if (Debut_X>Fin_X)
  {
    Tempo=Debut_X;
    Debut_X=Fin_X;
    Fin_X=Tempo;
  }
  if (Debut_Y>Fin_Y)
  {
    Tempo=Debut_Y;
    Debut_Y=Fin_Y;
    Fin_Y=Tempo;
  }

  // On trace le rectangle:

  for (Pos_X=Debut_X;Pos_X<=Fin_X;Pos_X++)
    Afficher_pinceau(Pos_X,Debut_Y,Couleur,0);

  for (Pos_Y=Debut_Y+1;Pos_Y<Fin_Y;Pos_Y++)
  {
    Afficher_pinceau(Debut_X,Pos_Y,Couleur,0);
    Afficher_pinceau(  Fin_X,Pos_Y,Couleur,0);
  }

  for (Pos_X=Debut_X;Pos_X<=Fin_X;Pos_X++)
    Afficher_pinceau(Pos_X,  Fin_Y,Couleur,0);
}

  // -- Tracer un rectangle plein --

void Tracer_rectangle_plein(short Debut_X,short Debut_Y,short Fin_X,short Fin_Y,byte Couleur)
{
  short Tempo;
  short Pos_X;
  short Pos_Y;


  // On vérifie que les bornes sont dans le bon sens:
  if (Debut_X>Fin_X)
  {
    Tempo=Debut_X;
    Debut_X=Fin_X;
    Fin_X=Tempo;
  }
  if (Debut_Y>Fin_Y)
  {
    Tempo=Debut_Y;
    Debut_Y=Fin_Y;
    Fin_Y=Tempo;
  }

  // Correction en cas de dépassement des limites de l'image
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;

  // On trace le rectangle:
  for (Pos_Y=Debut_Y;Pos_Y<=Fin_Y;Pos_Y++)
    for (Pos_X=Debut_X;Pos_X<=Fin_X;Pos_X++)
      // Afficher_pixel traite chaque pixel avec tous les effets ! (smear, ...)
      // Donc on ne peut pas otimiser en traçant ligne par ligne avec memset :(
      Afficher_pixel(Pos_X,Pos_Y,Couleur);
  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X,Fin_Y-Debut_Y);

}




  // -- Tracer une courbe de Bézier --

void Tracer_courbe_General(short X1, short Y1,
                           short X2, short Y2,
                           short X3, short Y3,
                           short X4, short Y4,
                           byte Couleur)
{
  float Delta,T,T2,T3;
  short X,Y,Old_X,Old_Y;
  word  i;
  int   CX[4];
  int   CY[4];

  // Calcul des vecteurs de coefficients
  CX[0]= -   X1 + 3*X2 - 3*X3 + X4;
  CX[1]= + 3*X1 - 6*X2 + 3*X3;
  CX[2]= - 3*X1 + 3*X2;
  CX[3]= +   X1;
  CY[0]= -   Y1 + 3*Y2 - 3*Y3 + Y4;
  CY[1]= + 3*Y1 - 6*Y2 + 3*Y3;
  CY[2]= - 3*Y1 + 3*Y2;
  CY[3]= +   Y1;

  // Traçage de la courbe
  Old_X=X1;
  Old_Y=Y1;
  Pixel_figure(Old_X,Old_Y,Couleur);
  Delta=0.05; // 1.0/20
  T=0;
  for (i=1; i<=20; i++)
  {
    T=T+Delta; T2=T*T; T3=T2*T;
    X=Round(T3*CX[0] + T2*CX[1] + T*CX[2] + CX[3]);
    Y=Round(T3*CY[0] + T2*CY[1] + T*CY[2] + CY[3]);
    Tracer_ligne_General(Old_X,Old_Y,X,Y,Couleur);
    Old_X=X;
    Old_Y=Y;
  }
  
  X = Min(Min(X1,X2),Min(X3,X4));
  Y = Min(Min(Y1,Y2),Min(Y3,Y4));
  Old_X = Max(Max(X1,X2),Max(X3,X4)) - X;
  Old_Y = Max(Max(Y1,Y2),Max(Y3,Y4)) - Y;
  Mettre_Ecran_A_Jour(X,Y,Old_X,Old_Y); //A optimiser !! on update à chaque pixel affiché !
}

  // -- Tracer une courbe de Bézier définitivement --

void Tracer_courbe_Definitif(short X1, short Y1,
                             short X2, short Y2,
                             short X3, short Y3,
                             short X4, short Y4,
                             byte Couleur)
{
  Pixel_figure=Pixel_figure_Definitif;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}

  // -- Tracer la preview d'une courbe de Bézier --

void Tracer_courbe_Preview(short X1, short Y1,
                           short X2, short Y2,
                           short X3, short Y3,
                           short X4, short Y4,
                           byte Couleur)
{
  Pixel_figure=Pixel_figure_Preview;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}

  // -- Effacer la preview d'une courbe de Bézier --

void Effacer_courbe_Preview(short X1, short Y1,
                            short X2, short Y2,
                            short X3, short Y3,
                            short X4, short Y4,
                            byte Couleur)
{
  Pixel_figure=Pixel_figure_Effacer_preview;
  Tracer_courbe_General(X1,Y1,X2,Y2,X3,Y3,X4,Y4,Couleur);
}




  // -- Spray : un petit coup de Pschiitt! --

void Aerographe(short Bouton_clicke)
{
  short Pos_X,Pos_Y;
  short Rayon=Spray_Size>>1;
  long  Rayon_au_carre=(long)Rayon*Rayon;
  short Indice,Count;
  byte  Indice_couleur;
  byte  Sens;


  Effacer_curseur();

  if (Spray_Mode)
  {
    for (Count=1; Count<=Spray_Mono_flow; Count++)
    {
      Pos_X=(rand()%Spray_Size)-Rayon;
      Pos_Y=(rand()%Spray_Size)-Rayon;
      if ( (Pos_X*Pos_X)+(Pos_Y*Pos_Y) <= Rayon_au_carre )
      {
        Pos_X+=Pinceau_X;
        Pos_Y+=Pinceau_Y;
        if (Bouton_clicke==1)
          Afficher_pinceau(Pos_X,Pos_Y,Fore_color,0);
        else
          Afficher_pinceau(Pos_X,Pos_Y,Back_color,0);
      }
    }
  }
  else
  {
    //   On essaye de se balader dans la table des flux de façon à ce que ce
    // ne soit pas toujours la dernière couleur qui soit affichée en dernier
    // Pour ça, on part d'une couleur au pif dans une direction aléatoire.
    Sens=rand()&1;
    for (Indice=0,Indice_couleur=rand()/*%256*/; Indice<256; Indice++)
    {
      for (Count=1; Count<=Spray_Multi_flow[Indice_couleur]; Count++)
      {
        Pos_X=(rand()%Spray_Size)-Rayon;
        Pos_Y=(rand()%Spray_Size)-Rayon;
        if ( (Pos_X*Pos_X)+(Pos_Y*Pos_Y) <= Rayon_au_carre )
        {
          Pos_X+=Pinceau_X;
          Pos_Y+=Pinceau_Y;
          if (Bouton_clicke==A_GAUCHE)
            Afficher_pinceau(Pos_X,Pos_Y,Indice_couleur,0);
          else
            Afficher_pinceau(Pos_X,Pos_Y,Back_color,0);
        }
      }
      if (Sens)
        Indice_couleur++;
      else
        Indice_couleur--;
    }
  }

  Afficher_curseur();

  for (Count=1; Count<=Spray_Delay; Count++)
    Wait_VBL();
}



  //////////////////////////////////////////////////////////////////////////
  ////////////////////////// GESTION DES DEGRADES //////////////////////////
  //////////////////////////////////////////////////////////////////////////


  // -- Gestion d'un dégradé de base (le plus moche) --

void Degrade_de_base(long Indice,short Pos_X,short Pos_Y)
{
  long Position;

  // On fait un premier calcul partiel
  Position=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard
  Position+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  Position-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  Position/=Degrade_Intervalle_total;

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (Position<0)
    Position=0;
  else if (Position>=Degrade_Intervalle_bornes)
    Position=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Traiter_pixel_de_degrade(Pos_X,Pos_Y,Degrade_Borne_Superieure-Position);
  else
    Traiter_pixel_de_degrade(Pos_X,Pos_Y,Degrade_Borne_Inferieure+Position);
}


  // -- Gestion d'un dégradé par trames simples --

void Degrade_de_trames_simples(long Indice,short Pos_X,short Pos_Y)
{
  long Position_dans_degrade;
  long Position_dans_segment;

  //
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "Position_dans_degrade", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  Position_dans_degrade=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard...
  Position_dans_degrade+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  Position_dans_degrade-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;

  // ... qui nous permet de calculer la position dans le segment
  Position_dans_segment=((Position_dans_degrade<<2)/Degrade_Intervalle_total)&3;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  Position_dans_degrade/=Degrade_Intervalle_total;

  // On va pouvoir discuter de la valeur de Position_dans_degrade en fonction
  // de la position dans l'écran et de la Position_dans_segment.

  switch (Position_dans_segment)
  {
    case 0 : // On est sur la gauche du segment
      if (((Pos_X+Pos_Y)&1)==0)
        Position_dans_degrade--;
      break;

      // On n'a pas à traiter les cas 1 et 2 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 3 : // On est sur la droite du segment
      if (((Pos_X+Pos_Y)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        Position_dans_degrade++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;
  else if (Position_dans_degrade>=Degrade_Intervalle_bornes)
    Position_dans_degrade=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Position_dans_degrade=Degrade_Borne_Superieure-Position_dans_degrade;
  else
    Position_dans_degrade=Degrade_Borne_Inferieure+Position_dans_degrade;

  Traiter_pixel_de_degrade(Pos_X,Pos_Y,Position_dans_degrade);
}


  // -- Gestion d'un dégradé par trames étendues --

void Degrade_de_trames_etendues(long Indice,short Pos_X,short Pos_Y)
{
  long Position_dans_degrade;
  long Position_dans_segment;

//
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "Position_dans_degrade", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  Position_dans_degrade=(Indice*Degrade_Intervalle_bornes);

  // On gère un déplacement au hasard
  Position_dans_degrade+=(Degrade_Intervalle_total*(rand()%Degrade_Melange_aleatoire)) >>6;
  Position_dans_degrade-=(Degrade_Intervalle_total*Degrade_Melange_aleatoire) >>7;

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;

  // Qui nous permet de calculer la position dans le segment
  Position_dans_segment=((Position_dans_degrade<<3)/Degrade_Intervalle_total)&7;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  Position_dans_degrade/=Degrade_Intervalle_total;

  // On va pouvoir discuter de la valeur de Position_dans_degrade en fonction
  // de la position dans l'écran et de la Position_dans_segment.

  switch (Position_dans_segment)
  {
    case 0 : // On est sur l'extrême gauche du segment
      if (((Pos_X+Pos_Y)&1)==0)
        Position_dans_degrade--;
      break;

    case 1 : // On est sur la gauche du segment
    case 2 : // On est sur la gauche du segment
      if (((Pos_X & 1)==0) && ((Pos_Y & 1)==0))
        Position_dans_degrade--;
      break;

      // On n'a pas à traiter les cas 3 et 4 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 5 : // On est sur la droite du segment
    case 6 : // On est sur la droite du segment
      if (((Pos_X & 1)==0) && ((Pos_Y & 1)!=0))
        Position_dans_degrade++;
      break;

    case 7 : // On est sur l'extreme droite du segment
      if (((Pos_X+Pos_Y)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        Position_dans_degrade++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (Position_dans_degrade<0)
    Position_dans_degrade=0;
  else if (Position_dans_degrade>=Degrade_Intervalle_bornes)
    Position_dans_degrade=Degrade_Intervalle_bornes-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Degrade_Inverse)
    Position_dans_degrade=Degrade_Borne_Superieure-Position_dans_degrade;
  else
    Position_dans_degrade=Degrade_Borne_Inferieure+Position_dans_degrade;

  Traiter_pixel_de_degrade(Pos_X,Pos_Y,Position_dans_degrade);
}







  // -- Tracer un cercle degradé (une sphère) --

void Tracer_cercle_degrade(short Centre_X,short Centre_Y,short Rayon,short Eclairage_X,short Eclairage_Y)
{
  long Debut_X;
  long Debut_Y;
  long Pos_X;
  long Pos_Y;
  long Fin_X;
  long Fin_Y;
  long Distance_X; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long Distance_Y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage

  Debut_X=Centre_X-Rayon;
  Debut_Y=Centre_Y-Rayon;
  Fin_X=Centre_X+Rayon;
  Fin_Y=Centre_Y+Rayon;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  Degrade_Intervalle_total=Cercle_Limite+
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))+
                           (2L*Rayon*sqrt(
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))));

  if (Degrade_Intervalle_total==0)
    Degrade_Intervalle_total=1;

  // Affichage du cercle
  for (Pos_Y=Debut_Y,Cercle_Curseur_Y=(long)Debut_Y-Centre_Y;Pos_Y<=Fin_Y;Pos_Y++,Cercle_Curseur_Y++)
  {
    Distance_Y =(Pos_Y-Eclairage_Y);
    Distance_Y*=Distance_Y;
    for (Pos_X=Debut_X,Cercle_Curseur_X=(long)Debut_X-Centre_X;Pos_X<=Fin_X;Pos_X++,Cercle_Curseur_X++)
      if (Pixel_dans_cercle())
      {
        Distance_X =(Pos_X-Eclairage_X);
        Distance_X*=Distance_X;
        Traiter_degrade(Distance_X+Distance_Y,Pos_X,Pos_Y);
      }
  }
  
  Mettre_Ecran_A_Jour(Centre_X-Rayon,Centre_Y-Rayon,2*Rayon+1,2*Rayon+1);
}


  // -- Tracer une ellipse degradée --

void Tracer_ellipse_degradee(short Centre_X,short Centre_Y,short Rayon_horizontal,short Rayon_vertical,short Eclairage_X,short Eclairage_Y)
{
  long Debut_X;
  long Debut_Y;
  long Pos_X;
  long Pos_Y;
  long Fin_X;
  long Fin_Y;
  long Distance_X; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long Distance_Y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage


  Debut_X=Centre_X-Rayon_horizontal;
  Debut_Y=Centre_Y-Rayon_vertical;
  Fin_X=Centre_X+Rayon_horizontal;
  Fin_Y=Centre_Y+Rayon_vertical;

  // Calcul des limites de l'ellipse
  Ellipse_Calculer_limites(Rayon_horizontal+1,Rayon_vertical+1);

  // On calcule la distance maximale:
  Degrade_Intervalle_total=(Rayon_horizontal*Rayon_horizontal)+
                           (Rayon_vertical*Rayon_vertical)+
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))+
                           (2L
                           *sqrt(
                           (Rayon_horizontal*Rayon_horizontal)+
                           (Rayon_vertical  *Rayon_vertical  ))
                           *sqrt(
                           ((Centre_X-Eclairage_X)*(Centre_X-Eclairage_X))+
                           ((Centre_Y-Eclairage_Y)*(Centre_Y-Eclairage_Y))));

  if (Degrade_Intervalle_total==0)
    Degrade_Intervalle_total=1;

  // Correction des bornes d'après les limites
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;

  // Affichage de l'ellipse
  for (Pos_Y=Debut_Y,Ellipse_Curseur_Y=Debut_Y-Centre_Y;Pos_Y<=Fin_Y;Pos_Y++,Ellipse_Curseur_Y++)
  {
    Distance_Y =(Pos_Y-Eclairage_Y);
    Distance_Y*=Distance_Y;
    for (Pos_X=Debut_X,Ellipse_Curseur_X=Debut_X-Centre_X;Pos_X<=Fin_X;Pos_X++,Ellipse_Curseur_X++)
      if (Pixel_dans_ellipse())
      {
        Distance_X =(Pos_X-Eclairage_X);
        Distance_X*=Distance_X;
        Traiter_degrade(Distance_X+Distance_Y,Pos_X,Pos_Y);
      }
  }

  Mettre_Ecran_A_Jour(Debut_X,Debut_Y,Fin_X-Debut_X+1,Fin_Y-Debut_Y+1);
}







  // -- Tracer un polygône plein --

typedef struct POLYGON_EDGE      /* an active edge */
{
  short top;                     /* top y position */
  short bottom;                  /* bottom y position */
  float x, dx;                   /* floating point x position and gradient */
  float w;                       /* width of line segment */
  struct POLYGON_EDGE *prev;     /* doubly linked list */
  struct POLYGON_EDGE *next;
} POLYGON_EDGE;



/* fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
void fill_edge_structure(POLYGON_EDGE *edge, short *i1, short *i2)
{
  short *it;

  if (i2[1] < i1[1])
  {
    it = i1;
    i1 = i2;
    i2 = it;
  }

  edge->top = i1[1];
  edge->bottom = i2[1] - 1;
  edge->dx = ((float) i2[0] - (float) i1[0]) / ((float) i2[1] - (float) i1[1]);
  edge->x = i1[0] + 0.4999999;
  edge->prev = NULL;
  edge->next = NULL;

  if (edge->dx+1 < 0.0)
    edge->x += edge->dx+1;

  if (edge->dx >= 0.0)
    edge->w = edge->dx;
  else
    edge->w = -(edge->dx);

  if (edge->w-1.0<0.0)
    edge->w = 0.0;
  else
    edge->w = edge->w-1;
}



/* add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
POLYGON_EDGE * add_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge, int sort_by_x)
{
  POLYGON_EDGE *pos = list;
  POLYGON_EDGE *prev = NULL;

  if (sort_by_x)
  {
    while ( (pos) && ((pos->x+((pos->w+pos->dx)/2)) < (edge->x+((edge->w+edge->dx)/2))) )
    {
      prev = pos;
      pos = pos->next;
    }
  }
  else
  {
    while ((pos) && (pos->top < edge->top))
    {
      prev = pos;
      pos = pos->next;
    }
  }

  edge->next = pos;
  edge->prev = prev;

  if (pos)
    pos->prev = edge;

  if (prev)
  {
    prev->next = edge;
    return list;
  }
  else
    return edge;
}



/* remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
POLYGON_EDGE * remove_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge)
{
  if (edge->next)
    edge->next->prev = edge->prev;

  if (edge->prev)
  {
    edge->prev->next = edge->next;
    return list;
  }
  else
    return edge->next;
}



/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the
 *  number of vertices, then an array containing a series of x, y points
 *  (a total of vertices*2 values).
 */
void Polyfill_General(int Vertices, short * Points, int Color)
{
  short c;
  short top = 0x7FFF;
  short bottom = 0;
  short *i1, *i2;
  short Pos_X,Fin_X;
  POLYGON_EDGE *edge, *next_edge, *initial_edge;
  POLYGON_EDGE *active_edges = NULL;
  POLYGON_EDGE *inactive_edges = NULL;


  /* allocate some space and fill the edge table */
  initial_edge=edge=(POLYGON_EDGE *) malloc(sizeof(POLYGON_EDGE) * Vertices);

  i1 = Points;
  i2 = Points + ((Vertices-1)<<1);

  for (c=0; c<Vertices; c++)
  {
    if (i1[1] != i2[1])
    {
      fill_edge_structure(edge, i1, i2);

      if (edge->bottom >= edge->top)
      {
        if (edge->top < top)
          top = edge->top;

        if (edge->bottom > bottom)
          bottom = edge->bottom;

        inactive_edges = add_edge(inactive_edges, edge, 0);
        edge++;
      }
    }
    i2 = i1;
    i1 += 2;
  }

  /* for each scanline in the polygon... */
  for (c=top; c<=bottom; c++)
  {
    /* check for newly active edges */
    edge = inactive_edges;
    while ((edge) && (edge->top == c))
    {
      next_edge = edge->next;
      inactive_edges = remove_edge(inactive_edges, edge);
      active_edges = add_edge(active_edges, edge, 1);
      edge = next_edge;
    }

    /* draw horizontal line segments */
    if ((c>=Limite_Haut) && (c<=Limite_Bas))
    {
      edge = active_edges;
      while ((edge) && (edge->next))
      {
        Pos_X=/*Round*/(edge->x);
        Fin_X=/*Round*/(edge->next->x+edge->next->w);
        if (Pos_X<Limite_Gauche)
          Pos_X=Limite_Gauche;
        if (Fin_X>Limite_Droite)
          Fin_X=Limite_Droite;
        for (; Pos_X<=Fin_X; Pos_X++)
          Pixel_figure(Pos_X,c,Color);
        edge = edge->next->next;
      }
    }

    /* update edges, sorting and removing dead ones */
    edge = active_edges;
    while (edge)
    {
      next_edge = edge->next;
      if (c >= edge->bottom)
        active_edges = remove_edge(active_edges, edge);
      else
      {
        edge->x += edge->dx;
        while ((edge->prev) && ( (edge->x+(edge->w/2)) < (edge->prev->x+(edge->prev->w/2))) )
        {
          if (edge->next)
            edge->next->prev = edge->prev;
          edge->prev->next = edge->next;
          edge->next = edge->prev;
          edge->prev = edge->prev->prev;
          edge->next->prev = edge;
          if (edge->prev)
            edge->prev->next = edge;
          else
            active_edges = edge;
        }
      }
      edge = next_edge;
    }
  }

  free(initial_edge);
  // On ne connait pas simplement les xmin et xmax ici, mais de toutes façon ce n'est pas utilisé en preview
  Mettre_Ecran_A_Jour(0,top,Principal_Largeur_image,bottom-top+1);
}


void Polyfill(int Vertices, short * Points, int Color)
{
  Pixel_figure=Afficher_pixel;
  Polyfill_General(Vertices,Points,Color);
}



void Capturer_brosse_au_lasso(int Vertices, short * Points,short Effacement)
{
  short Debut_X=Limite_Droite+1;
  short Debut_Y=Limite_Bas+1;
  short Fin_X=Limite_Gauche-1;
  short Fin_Y=Limite_Haut-1;
  short Temporaire;
  short Pos_X;
  short Pos_Y;
  word  Nouvelle_Brosse_Largeur;
  word  Nouvelle_Brosse_Hauteur;


  // On recherche les bornes de la brosse:
  for (Temporaire=0; Temporaire<Vertices; Temporaire++)
  {
    Pos_X=Points[Temporaire<<1];
    Pos_Y=Points[(Temporaire<<1)+1];
    if (Pos_X<Debut_X)
      Debut_X=Pos_X;
    if (Pos_X>Fin_X)
      Fin_X=Pos_X;
    if (Pos_Y<Debut_Y)
      Debut_Y=Pos_Y;
    if (Pos_Y>Fin_Y)
      Fin_Y=Pos_Y;
  }

  // On clippe ces bornes à l'écran:
  if (Debut_X<Limite_Gauche)
    Debut_X=Limite_Gauche;
  if (Fin_X>Limite_Droite)
    Fin_X=Limite_Droite;
  if (Debut_Y<Limite_Haut)
    Debut_Y=Limite_Haut;
  if (Fin_Y>Limite_Bas)
    Fin_Y=Limite_Bas;

  // On ne capture la nouvelle brosse que si elle est au moins partiellement
  // dans l'image:

  if ((Debut_X<Principal_Largeur_image) && (Debut_Y<Principal_Hauteur_image))
  {
    // On met les décalages du tiling à 0 pour eviter toute incohérence.
    // Si par hasard on voulait les mettre à
    //    min(Tiling_Decalage_?,Brosse_?a??eur-1)
    // il faudrait penser à les initialiser à 0 dans "MAIN.C".
    Tiling_Decalage_X=0;
    Tiling_Decalage_Y=0;

    // Ensuite, on calcule les dimensions de la brosse:
    Nouvelle_Brosse_Largeur=(Fin_X-Debut_X)+1;
    Nouvelle_Brosse_Hauteur=(Fin_Y-Debut_Y)+1;

    if ( (((long)Brosse_Hauteur)*Brosse_Largeur) !=
         (((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur) )
    {
      free(Brosse);
      Brosse=(byte *)malloc(((long)Nouvelle_Brosse_Hauteur)*Nouvelle_Brosse_Largeur);
      if (!Brosse)
      {
        Erreur(0);

        Brosse=(byte *)malloc(1*1);
        Nouvelle_Brosse_Hauteur=Nouvelle_Brosse_Largeur=1;
        *Brosse=Fore_color;
      }
    }
    Brosse_Largeur=Nouvelle_Brosse_Largeur;
    Brosse_Hauteur=Nouvelle_Brosse_Hauteur;

    free(Smear_Brosse);
    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=Debut_X;
    Brosse_Decalage_Y=Debut_Y;
    Pixel_figure=Pixel_figure_Dans_brosse;

    memset(Brosse,Back_color,(long)Brosse_Largeur*Brosse_Hauteur);
    Polyfill_General(Vertices,Points,~Back_color);

    // On retrace les bordures du lasso:
    for (Temporaire=1; Temporaire<Vertices; Temporaire++)
    {
      Tracer_ligne_General(Points[(Temporaire-1)<<1],Points[((Temporaire-1)<<1)+1],
                           Points[Temporaire<<1],Points[(Temporaire<<1)+1],
                           ~Back_color);
    }
    Tracer_ligne_General(Points[(Vertices-1)<<1],Points[((Vertices-1)<<1)+1],
                         Points[0],Points[1],
                         ~Back_color);

    // On scanne la brosse pour remplacer tous les pixels affectés par le
    // polyfill par ceux de l'image:
    for (Pos_Y=Debut_Y;Pos_Y<=Fin_Y;Pos_Y++)
      for (Pos_X=Debut_X;Pos_X<=Fin_X;Pos_X++)
        if (Lit_pixel_dans_brosse(Pos_X-Debut_X,Pos_Y-Debut_Y)!=Back_color)
        {
          Pixel_dans_brosse(Pos_X-Debut_X,Pos_Y-Debut_Y,Lit_pixel_dans_ecran_courant(Pos_X,Pos_Y));
          // On regarde s'il faut effacer quelque chose:
          if (Effacement)
            Pixel_dans_ecran_courant(Pos_X,Pos_Y,Back_color);
        }

    // On centre la prise sur la brosse
    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
}



//------------ Remplacement de la couleur pointée par une autre --------------

void Remplacer(byte Nouvelle_couleur)
{
  byte Ancienne_couleur;

  if ((Pinceau_X<Principal_Largeur_image)
   && (Pinceau_Y<Principal_Hauteur_image))
  {
    Ancienne_couleur=Lit_pixel_dans_ecran_courant(Pinceau_X,Pinceau_Y);
    if ( (Ancienne_couleur!=Nouvelle_couleur)
      && ((!Stencil_Mode) || (!Stencil[Ancienne_couleur])) )
    {
      Remplacer_une_couleur(Ancienne_couleur,Nouvelle_couleur);
      Afficher_ecran();
    }
  }
}


//------------------------- Etirement de la brosse ---------------------------

void Etirer_brosse(short X1, short Y1, short X2, short Y2)
{
  int    Offset,Ligne,Colonne;
  byte * New_Brosse;

  int    New_Brosse_Largeur;  // Largeur de la nouvelle brosse
  int    New_Brosse_Hauteur;  // Hauteur de la nouvelle brosse

  int    Pos_X_dans_brosse;   // Position courante dans l'ancienne brosse
  int    Pos_Y_dans_brosse;
  int    Delta_X_dans_brosse; // "Vecteur incrémental" du point précédent
  int    Delta_Y_dans_brosse;
  int    Pos_X_initial;       // Position X de début de parcours de ligne
  int    Dx,Dy;

  Dx=(X1<X2)?1:-1;
  Dy=(Y1<Y2)?1:-1;

  // Calcul des nouvelles dimensions de la brosse:
  if ((New_Brosse_Largeur=X1-X2)<0)
    New_Brosse_Largeur=-New_Brosse_Largeur;
  New_Brosse_Largeur++;

  if ((New_Brosse_Hauteur=Y1-Y2)<0)
    New_Brosse_Hauteur=-New_Brosse_Hauteur;
  New_Brosse_Hauteur++;

  // Calcul des anciennes dimensions de la brosse:

  // Calcul du "vecteur incrémental":
  Delta_X_dans_brosse=(Brosse_Largeur<<16)/(X2-X1+Dx);
  Delta_Y_dans_brosse=(Brosse_Hauteur<<16)/(Y2-Y1+Dy);

  // Calcul de la valeur initiale de Pos_X pour chaque ligne:
  if (Dx>=0)
    Pos_X_initial = 0;                // Pas d'inversion en X de la brosse
  else
    Pos_X_initial = (Brosse_Largeur<<16)-1; // Inversion en X de la brosse

  free(Smear_Brosse); // On libère un peu de mémoire

  if ((New_Brosse=((byte *)malloc(New_Brosse_Largeur*New_Brosse_Hauteur))))
  {
    Offset=0;

    // Calcul de la valeur initiale de Pos_Y:
    if (Dy>=0)
      Pos_Y_dans_brosse=0;                // Pas d'inversion en Y de la brosse
    else
      Pos_Y_dans_brosse=(Brosse_Hauteur<<16)-1; // Inversion en Y de la brosse

    // Pour chaque ligne
    for (Ligne=0;Ligne<New_Brosse_Hauteur;Ligne++)
    {
      // On repart du début de la ligne:
      Pos_X_dans_brosse=Pos_X_initial;

      // Pour chaque colonne:
      for (Colonne=0;Colonne<New_Brosse_Largeur;Colonne++)
      {
        // On copie le pixel:
        New_Brosse[Offset]=Lit_pixel_dans_brosse(Pos_X_dans_brosse>>16,Pos_Y_dans_brosse>>16);
        // On passe à la colonne de brosse suivante:
        Pos_X_dans_brosse+=Delta_X_dans_brosse;
        // On passe au pixel suivant de la nouvelle brosse:
        Offset++;
      }

      // On passe à la ligne de brosse suivante:
      Pos_Y_dans_brosse+=Delta_Y_dans_brosse;
    }

    free(Brosse);
    Brosse=New_Brosse;

    Brosse_Largeur=New_Brosse_Largeur;
    Brosse_Hauteur=New_Brosse_Hauteur;

    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffit donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_Brosse car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);
    Erreur(0);
  }
}



void Etirer_brosse_preview(short X1, short Y1, short X2, short Y2)
{
  int    Pos_X_src,Pos_Y_src;
  int    Pos_X_src_Initiale,Pos_Y_src_Initiale;
  int    Delta_X,Delta_Y;
  int    Pos_X_dest,Pos_Y_dest;
  int    Pos_X_dest_Initiale,Pos_Y_dest_Initiale;
  int    Pos_X_dest_Finale,Pos_Y_dest_Finale;
  int    Largeur_dest,Hauteur_dest;
  byte   Couleur;


  // 1er calcul des positions destination extremes:
  Pos_X_dest_Initiale=Min(X1,X2);
  Pos_Y_dest_Initiale=Min(Y1,Y2);
  Pos_X_dest_Finale  =Max(X1,X2);
  Pos_Y_dest_Finale  =Max(Y1,Y2);

  // Calcul des dimensions de la destination:
  Largeur_dest=Pos_X_dest_Finale-Pos_X_dest_Initiale+1;
  Hauteur_dest=Pos_Y_dest_Finale-Pos_Y_dest_Initiale+1;

  // Calcul des vecteurs d'incrémentation :
  Delta_X=(Brosse_Largeur<<16)/Largeur_dest;
  Delta_Y=(Brosse_Hauteur<<16)/Hauteur_dest;

  // 1er calcul de la position X initiale dans la source:
  Pos_X_src_Initiale=(Brosse_Largeur<<16)*
                     (Max(Pos_X_dest_Initiale,Limite_Gauche)-
                      Pos_X_dest_Initiale)/Largeur_dest;
  // Calcul du clip de la destination:
  Pos_X_dest_Initiale=Max(Pos_X_dest_Initiale,Limite_Gauche);
  Pos_X_dest_Finale  =Min(Pos_X_dest_Finale  ,Limite_visible_Droite);
  // On discute selon l'inversion en X
  if (X1>X2)
  {
    // Inversion -> Inversion du signe de Delta_X
    Delta_X=-Delta_X;
    Pos_X_src_Initiale=(Brosse_Largeur<<16)-1-Pos_X_src_Initiale;
  }

  // 1er calcul de la position Y initiale dans la source:
  Pos_Y_src_Initiale=(Brosse_Hauteur<<16)*
                     (Max(Pos_Y_dest_Initiale,Limite_Haut)-
                      Pos_Y_dest_Initiale)/Hauteur_dest;
  // Calcul du clip de la destination:
  Pos_Y_dest_Initiale=Max(Pos_Y_dest_Initiale,Limite_Haut);
  Pos_Y_dest_Finale  =Min(Pos_Y_dest_Finale  ,Limite_visible_Bas);
  // On discute selon l'inversion en Y
  if (Y1>Y2)
  {
    // Inversion -> Inversion du signe de Delta_Y
    Delta_Y=-Delta_Y;
    Pos_Y_src_Initiale=(Brosse_Hauteur<<16)-1-Pos_Y_src_Initiale;
  }

  // Pour chaque ligne :
  Pos_Y_src=Pos_Y_src_Initiale;
  for (Pos_Y_dest=Pos_Y_dest_Initiale;Pos_Y_dest<=Pos_Y_dest_Finale;Pos_Y_dest++)
  {
    // Pour chaque colonne:
    Pos_X_src=Pos_X_src_Initiale;
    for (Pos_X_dest=Pos_X_dest_Initiale;Pos_X_dest<=Pos_X_dest_Finale;Pos_X_dest++)
    {
      Couleur=Lit_pixel_dans_brosse(Pos_X_src>>16,Pos_Y_src>>16);
      if (Couleur!=Back_color)
        Pixel_Preview(Pos_X_dest,Pos_Y_dest,Couleur);

      Pos_X_src+=Delta_X;
    }

    Pos_Y_src+=Delta_Y;
  }

  SDL_UpdateRect(Ecran_SDL,Pos_X_dest_Initiale,Pos_Y_dest_Initiale,Largeur_dest,Hauteur_dest);
}



//------------------------- Rotation de la brosse ---------------------------

#define INDEFINI (-1.0e20F)
float * ScanY_Xt[2];
float * ScanY_Yt[2];
float * ScanY_X[2];


void Interpoler_texture(int Debut_X,int Debut_Y,int Xt1,int Yt1,
                        int Fin_X  ,int Fin_Y  ,int Xt2,int Yt2,int Hauteur)
{
  int Pos_X,Pos_Y;
  int Incr_X,Incr_Y;
  int i,Cumul;
  int Delta_X,Delta_Y;
  int Delta_Xt=Xt2-Xt1;
  int Delta_Yt=Yt2-Yt1;
  int Delta_X2=Fin_X-Debut_X;
  int Delta_Y2=Fin_Y-Debut_Y;
  float Xt,Yt;


  Pos_X=Debut_X;
  Pos_Y=Debut_Y;

  if (Debut_X<Fin_X)
  {
    Incr_X=+1;
    Delta_X=Delta_X2;
  }
  else
  {
    Incr_X=-1;
    Delta_X=-Delta_X2;
  }

  if (Debut_Y<Fin_Y)
  {
    Incr_Y=+1;
    Delta_Y=Delta_Y2;
  }
  else
  {
    Incr_Y=-1;
    Delta_Y=-Delta_Y2;
  }

  if (Delta_X>Delta_Y)
  {
    Cumul=Delta_X>>1;
    for (i=0; i<=Delta_X; i++)
    {
      if (Cumul>=Delta_X)
      {
        Cumul-=Delta_X;
        Pos_Y+=Incr_Y;
      }

      if ((Pos_Y>=0) && (Pos_Y<Hauteur))
      {
        Xt=(((float)((Pos_X-Debut_X)*Delta_Xt))/(float)Delta_X2) + (float)Xt1;
        Yt=(((float)((Pos_X-Debut_X)*Delta_Yt))/(float)Delta_X2) + (float)Yt1;
        if (ScanY_X[0][Pos_Y]==INDEFINI) // Gauche non défini
        {
          ScanY_X[0][Pos_Y]=Pos_X;
          ScanY_Xt[0][Pos_Y]=Xt;
          ScanY_Yt[0][Pos_Y]=Yt;
        }
        else
        {
          if (Pos_X>=ScanY_X[0][Pos_Y])
          {
            if ((ScanY_X[1][Pos_Y]==INDEFINI) // Droit non défini
             || (Pos_X>ScanY_X[1][Pos_Y]))
            {
              ScanY_X[1][Pos_Y]=Pos_X;
              ScanY_Xt[1][Pos_Y]=Xt;
              ScanY_Yt[1][Pos_Y]=Yt;
            }
          }
          else
          {
            if (ScanY_X[1][Pos_Y]==INDEFINI) // Droit non défini
            {
              ScanY_X[1][Pos_Y]=ScanY_X[0][Pos_Y];
              ScanY_Xt[1][Pos_Y]=ScanY_Xt[0][Pos_Y];
              ScanY_Yt[1][Pos_Y]=ScanY_Yt[0][Pos_Y];
              ScanY_X[0][Pos_Y]=Pos_X;
              ScanY_Xt[0][Pos_Y]=Xt;
              ScanY_Yt[0][Pos_Y]=Yt;
            }
            else
            {
              ScanY_X[0][Pos_Y]=Pos_X;
              ScanY_Xt[0][Pos_Y]=Xt;
              ScanY_Yt[0][Pos_Y]=Yt;
            }
          }
        }
      }
      Pos_X+=Incr_X;
      Cumul+=Delta_Y;
    }
  }
  else
  {
    Cumul=Delta_Y>>1;
    for (i=0; i<=Delta_Y; i++)
    {
      if (Cumul>=Delta_Y)
      {
        Cumul-=Delta_Y;
        Pos_X+=Incr_X;
      }

      if ((Pos_Y>=0) && (Pos_Y<Hauteur))
      {
        Xt=(((float)((Pos_Y-Debut_Y)*Delta_Xt))/(float)Delta_Y2) + (float)Xt1;
        Yt=(((float)((Pos_Y-Debut_Y)*Delta_Yt))/(float)Delta_Y2) + (float)Yt1;
        if (ScanY_X[0][Pos_Y]==INDEFINI) // Gauche non défini
        {
          ScanY_X[0][Pos_Y]=Pos_X;
          ScanY_Xt[0][Pos_Y]=Xt;
          ScanY_Yt[0][Pos_Y]=Yt;
        }
        else
        {
          if (Pos_X>=ScanY_X[0][Pos_Y])
          {
            if ((ScanY_X[1][Pos_Y]==INDEFINI) // Droit non défini
             || (Pos_X>ScanY_X[1][Pos_Y]))
            {
              ScanY_X[1][Pos_Y]=Pos_X;
              ScanY_Xt[1][Pos_Y]=Xt;
              ScanY_Yt[1][Pos_Y]=Yt;
            }
          }
          else
          {
            if (ScanY_X[1][Pos_Y]==INDEFINI) // Droit non défini
            {
              ScanY_X[1][Pos_Y]=ScanY_X[0][Pos_Y];
              ScanY_Xt[1][Pos_Y]=ScanY_Xt[0][Pos_Y];
              ScanY_Yt[1][Pos_Y]=ScanY_Yt[0][Pos_Y];
              ScanY_X[0][Pos_Y]=Pos_X;
              ScanY_Xt[0][Pos_Y]=Xt;
              ScanY_Yt[0][Pos_Y]=Yt;
            }
            else
            {
              ScanY_X[0][Pos_Y]=Pos_X;
              ScanY_Xt[0][Pos_Y]=Xt;
              ScanY_Yt[0][Pos_Y]=Yt;
            }
          }
        }
      }
      Pos_Y+=Incr_Y;
      Cumul+=Delta_X;
    }
  }
}



void Calculer_quad_texture(int X1,int Y1,int Xt1,int Yt1,
                           int X2,int Y2,int Xt2,int Yt2,
                           int X3,int Y3,int Xt3,int Yt3,
                           int X4,int Y4,int Xt4,int Yt4,
                           byte * Buffer, int Largeur, int Hauteur)
{
  int Xmin,/*Xmax,*/Ymin/*,Ymax*/;
  int X,Y,Xt,Yt;
  int Debut_X,Fin_X,Largeur_ligne;
  float Temp;
  //byte Couleur;

  Xmin=Min(Min(X1,X2),Min(X3,X4));
  Ymin=Min(Min(Y1,Y2),Min(Y3,Y4));

  ScanY_Xt[0]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_X[0] =(float *)malloc(Hauteur*sizeof(float));
  ScanY_X[1] =(float *)malloc(Hauteur*sizeof(float));

  // Remplir avec des valeurs égales à INDEFINI.
  for (Y=0; Y<Hauteur; Y++)
  {
    ScanY_X[0][Y]=INDEFINI;
    ScanY_X[1][Y]=INDEFINI;
  }

  Interpoler_texture(X1-Xmin,Y1-Ymin,Xt1,Yt1,X3-Xmin,Y3-Ymin,Xt3,Yt3,Hauteur);
  Interpoler_texture(X3-Xmin,Y3-Ymin,Xt3,Yt3,X4-Xmin,Y4-Ymin,Xt4,Yt4,Hauteur);
  Interpoler_texture(X4-Xmin,Y4-Ymin,Xt4,Yt4,X2-Xmin,Y2-Ymin,Xt2,Yt2,Hauteur);
  Interpoler_texture(X2-Xmin,Y2-Ymin,Xt2,Yt2,X1-Xmin,Y1-Ymin,Xt1,Yt1,Hauteur);

  for (Y=0; Y<Hauteur; Y++)
  {
    Debut_X=Round(ScanY_X[0][Y]);
    Fin_X  =Round(ScanY_X[1][Y]);

    Largeur_ligne=1+Fin_X-Debut_X;

    for (X=0; X<Debut_X; X++)
      Buffer[X+(Y*Largeur)]=Back_color;
    for (; X<=Fin_X; X++)
    {
      Temp=(float)(0.5+(float)X-ScanY_X[0][Y])/(float)Largeur_ligne;
      Xt=Round((float)(ScanY_Xt[0][Y])+(Temp*(ScanY_Xt[1][Y]-ScanY_Xt[0][Y])));
      Yt=Round((float)(ScanY_Yt[0][Y])+(Temp*(ScanY_Yt[1][Y]-ScanY_Yt[0][Y])));

      Buffer[X+(Y*Largeur)]=Lit_pixel_dans_brosse(Xt,Yt);
    }
    for (; X<Largeur; X++)
      Buffer[X+(Y*Largeur)]=Back_color;
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}



void Tourner_brosse(float Angle)
{
  byte * New_Brosse;
  int    New_Brosse_Largeur;  // Largeur de la nouvelle brosse
  int    New_Brosse_Hauteur;  // Hauteur de la nouvelle brosse

  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  int Debut_X,Fin_X,Debut_Y,Fin_Y;
  int Xmin,Xmax,Ymin,Ymax;
  float cosA=cos(Angle);
  float sinA=sin(Angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  Debut_X=1-(Brosse_Largeur>>1);
  Debut_Y=1-(Brosse_Hauteur>>1);
  Fin_X=Debut_X+Brosse_Largeur-1;
  Fin_Y=Debut_Y+Brosse_Hauteur-1;

  Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
  Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
  Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
  Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

  // Calcul des nouvelles dimensions de la brosse:
  Xmin=Min(Min((int)X1,(int)X2),Min((int)X3,(int)X4));
  Xmax=Max(Max((int)X1,(int)X2),Max((int)X3,(int)X4));
  Ymin=Min(Min((int)Y1,(int)Y2),Min((int)Y3,(int)Y4));
  Ymax=Max(Max((int)Y1,(int)Y2),Max((int)Y3,(int)Y4));

  New_Brosse_Largeur=Xmax+1-Xmin;
  New_Brosse_Hauteur=Ymax+1-Ymin;

  free(Smear_Brosse); // On libère un peu de mémoire

  if ((New_Brosse=((byte *)malloc(New_Brosse_Largeur*New_Brosse_Hauteur))))
  {
    // Et maintenant on calcule la nouvelle brosse tournée.
    Calculer_quad_texture(X1,Y1,               0,               0,
                          X2,Y2,Brosse_Largeur-1,               0,
                          X3,Y3,               0,Brosse_Hauteur-1,
                          X4,Y4,Brosse_Largeur-1,Brosse_Hauteur-1,
                          New_Brosse,New_Brosse_Largeur,New_Brosse_Hauteur);

    free(Brosse);
    Brosse=New_Brosse;

    Brosse_Largeur=New_Brosse_Largeur;
    Brosse_Hauteur=New_Brosse_Hauteur;

    Smear_Brosse_Largeur=(Brosse_Largeur>TAILLE_MAXI_PINCEAU)?Brosse_Largeur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse_Hauteur=(Brosse_Hauteur>TAILLE_MAXI_PINCEAU)?Brosse_Hauteur:TAILLE_MAXI_PINCEAU;
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);

    if (!Smear_Brosse) // On ne peut même pas allouer la brosse du smear!
    {
      Erreur(0);

      free(Brosse);
      Brosse=(byte *)malloc(1*1);
      Brosse_Hauteur=1;
      Brosse_Largeur=1;

      Smear_Brosse=(byte *)malloc(TAILLE_MAXI_PINCEAU*TAILLE_MAXI_PINCEAU);
      Smear_Brosse_Hauteur=TAILLE_MAXI_PINCEAU;
      Smear_Brosse_Largeur=TAILLE_MAXI_PINCEAU;
    }

    Brosse_Decalage_X=(Brosse_Largeur>>1);
    Brosse_Decalage_Y=(Brosse_Hauteur>>1);
  }
  else
  {
    // Ici la libération de mémoire n'a pas suffit donc on remet dans l'état
    // où c'etait avant. On a juste à réallouer la Smear_Brosse car il y a
    // normalement la place pour elle puisque rien d'autre n'a pu être alloué
    // entre temps.
    Smear_Brosse=(byte *)malloc(((long)Smear_Brosse_Hauteur)*Smear_Brosse_Largeur);
    Erreur(0);
  }
}



void Dessiner_quad_texture_preview(int X1,int Y1,int Xt1,int Yt1,
                                   int X2,int Y2,int Xt2,int Yt2,
                                   int X3,int Y3,int Xt3,int Yt3,
                                   int X4,int Y4,int Xt4,int Yt4)
{
  int Xmin,Xmax,Ymin,Ymax;
  int X,Y,Xt,Yt;
  int Y_,Ymin_;
  int Debut_X,Fin_X,Largeur,Hauteur;
  float Temp;
  byte Couleur;

  Xmin=Min(Min(X1,X2),Min(X3,X4));
  Xmax=Max(Max(X1,X2),Max(X3,X4));
  Ymin=Min(Min(Y1,Y2),Min(Y3,Y4));
  Ymax=Max(Max(Y1,Y2),Max(Y3,Y4));
  Hauteur=1+Ymax-Ymin;

  ScanY_Xt[0]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Xt[1]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Yt[0]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_Yt[1]=(float *)malloc(Hauteur*sizeof(float));
  ScanY_X[0] =(float *)malloc(Hauteur*sizeof(float));
  ScanY_X[1] =(float *)malloc(Hauteur*sizeof(float));

  // Remplir avec des valeurs égales à INDEFINI.
  for (Y=0; Y<Hauteur; Y++)
  {
    ScanY_X[0][Y]=INDEFINI;
    ScanY_X[1][Y]=INDEFINI;
  }

  Interpoler_texture(X1,Y1-Ymin,Xt1,Yt1,X3,Y3-Ymin,Xt3,Yt3,Hauteur);
  Interpoler_texture(X3,Y3-Ymin,Xt3,Yt3,X4,Y4-Ymin,Xt4,Yt4,Hauteur);
  Interpoler_texture(X4,Y4-Ymin,Xt4,Yt4,X2,Y2-Ymin,Xt2,Yt2,Hauteur);
  Interpoler_texture(X2,Y2-Ymin,Xt2,Yt2,X1,Y1-Ymin,Xt1,Yt1,Hauteur);

  Ymin_=Ymin;
  if (Ymin<Limite_Haut) Ymin=Limite_Haut;
  if (Ymax>Limite_Bas)  Ymax=Limite_Bas;

  for (Y_=Ymin; Y_<=Ymax; Y_++)
  {
    Y=Y_-Ymin_;
    Debut_X=Round(ScanY_X[0][Y]);
    Fin_X  =Round(ScanY_X[1][Y]);

    Largeur=1+Fin_X-Debut_X;

    if (Debut_X<Limite_Gauche) Debut_X=Limite_Gauche;
    if (  Fin_X>Limite_Droite)   Fin_X=Limite_Droite;

    for (X=Debut_X; X<=Fin_X; X++)
    {
      Temp=(float)(0.5+(float)X-ScanY_X[0][Y])/(float)Largeur;
      Xt=Round((float)(ScanY_Xt[0][Y])+(Temp*(ScanY_Xt[1][Y]-ScanY_Xt[0][Y])));
      Yt=Round((float)(ScanY_Yt[0][Y])+(Temp*(ScanY_Yt[1][Y]-ScanY_Yt[0][Y])));

      Couleur=Lit_pixel_dans_brosse(Xt,Yt);
      if (Couleur!=Back_color)
        Pixel_Preview(X,Y_,Couleur);
    }
  }

  free(ScanY_Xt[0]);
  free(ScanY_Xt[1]);
  free(ScanY_Yt[0]);
  free(ScanY_Yt[1]);
  free(ScanY_X[0]);
  free(ScanY_X[1]);
}


void Tourner_brosse_preview(float Angle)
{
  short X1,Y1,X2,Y2,X3,Y3,X4,Y4;
  int Debut_X,Fin_X,Debut_Y,Fin_Y;
  float cosA=cos(Angle);
  float sinA=sin(Angle);

  // Calcul des coordonnées des 4 coins:
  // 1 2
  // 3 4

  Debut_X=1-(Brosse_Largeur>>1);
  Debut_Y=1-(Brosse_Hauteur>>1);
  Fin_X=Debut_X+Brosse_Largeur-1;
  Fin_Y=Debut_Y+Brosse_Hauteur-1;

  Transformer_point(Debut_X,Debut_Y, cosA,sinA, &X1,&Y1);
  Transformer_point(Fin_X  ,Debut_Y, cosA,sinA, &X2,&Y2);
  Transformer_point(Debut_X,Fin_Y  , cosA,sinA, &X3,&Y3);
  Transformer_point(Fin_X  ,Fin_Y  , cosA,sinA, &X4,&Y4);

  X1+=Brosse_Centre_rotation_X;
  Y1+=Brosse_Centre_rotation_Y;
  X2+=Brosse_Centre_rotation_X;
  Y2+=Brosse_Centre_rotation_Y;
  X3+=Brosse_Centre_rotation_X;
  Y3+=Brosse_Centre_rotation_Y;
  X4+=Brosse_Centre_rotation_X;
  Y4+=Brosse_Centre_rotation_Y;

  // Et maintenant on dessine la brosse tournée.
  Dessiner_quad_texture_preview(X1,Y1,               0,               0,
                                X2,Y2,Brosse_Largeur-1,               0,
                                X3,Y3,               0,Brosse_Hauteur-1,
                                X4,Y4,Brosse_Largeur-1,Brosse_Hauteur-1);
}
