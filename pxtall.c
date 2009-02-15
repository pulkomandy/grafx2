/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include "global.h"
#include "sdlscreen.h"
#include "divers.h"
#include "pxsimple.h"

void Pixel_Tall (word X,word Y,byte Couleur)
/* Affiche un pixel de la Couleur aux coords X;Y à l'écran */
{
  *(Ecran + X + Y * 2 * Largeur_ecran)=Couleur;
  *(Ecran + X + (Y * 2 + 1) * Largeur_ecran)=Couleur;
}

byte Lit_Pixel_Tall (word X,word Y)
/* On retourne la couleur du pixel aux coords données */
{
  return *( Ecran + Y * 2 * Largeur_ecran + X );
}

void Block_Tall (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur)
/* On affiche un rectangle de la couleur donnée */
{
  SDL_Rect rectangle;
  rectangle.x=Debut_X;
  rectangle.y=Debut_Y*2;
  rectangle.w=Largeur;
  rectangle.h=Hauteur*2;
  SDL_FillRect(Ecran_SDL,&rectangle,Couleur);
}

void Afficher_partie_de_l_ecran_Tall (word Largeur,word Hauteur,word Largeur_image)
/* Afficher une partie de l'image telle quelle sur l'écran */
{
  byte* Dest=Ecran; //On va se mettre en 0,0 dans l'écran (Dest)
  byte* Src=Principal_Decalage_Y*Largeur_image+Principal_Decalage_X+Principal_Ecran; //Coords de départ ds la source (Src)
  int dx;

  for(dx=Hauteur;dx!=0;dx--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    memcpy(Dest,Src,Largeur);
    Dest+=Largeur_ecran;
    memcpy(Dest,Src,Largeur);

    // On passe à la ligne suivante
    Src+=Largeur_image;
    Dest+=Largeur_ecran;
  }
  //UpdateRect(0,0,Largeur,Hauteur);
}

void Pixel_Preview_Normal_Tall (word X,word Y,byte Couleur)
/* Affichage d'un pixel dans l'écran, par rapport au décalage de l'image 
 * dans l'écran, en mode normal (pas en mode loupe)
 * Note: si on modifie cette procédure, il faudra penser à faire également 
 * la modif dans la procédure Pixel_Preview_Loupe_SDL. */
{
//  if(X-Principal_Decalage_X >= 0 && Y - Principal_Decalage_Y >= 0)
  Pixel_Tall(X-Principal_Decalage_X,Y-Principal_Decalage_Y,Couleur);
}

void Pixel_Preview_Loupe_Tall  (word X,word Y,byte Couleur)
{
  // Affiche le pixel dans la partie non zoomée
  Pixel_Tall(X-Principal_Decalage_X,Y-Principal_Decalage_Y,Couleur);
  
  // Regarde si on doit aussi l'afficher dans la partie zoomée
  if (Y >= Limite_Haut_Zoom && Y <= Limite_visible_Bas_Zoom
          && X >= Limite_Gauche_Zoom && X <= Limite_visible_Droite_Zoom)
  {
    // On est dedans
    int hauteur;
    int Y_Zoom = Table_mul_facteur_zoom[Y-Loupe_Decalage_Y];

    if (Menu_Ordonnee - Y_Zoom < Loupe_Facteur)
      // On ne doit dessiner qu'un morceau du pixel
      // sinon on dépasse sur le menu
      hauteur = Menu_Ordonnee - Y_Zoom;
    else
      hauteur = Loupe_Facteur;

    Block_Tall(
      Table_mul_facteur_zoom[X-Loupe_Decalage_X]+Principal_X_Zoom, 
      Y_Zoom, Loupe_Facteur, hauteur, Couleur
      );
  }
}

void Ligne_horizontale_XOR_Tall(word Pos_X,word Pos_Y,word Largeur)
{
  //On calcule la valeur initiale de Dest:
  byte* Dest=Pos_Y*2*Largeur_ecran+Pos_X+Ecran;

  int ecx;

  for (ecx=0;ecx<Largeur;ecx++)
    *(Dest+ecx)=~*(Dest+ecx);

  Dest=(Pos_Y*2+1)*Largeur_ecran+Pos_X+Ecran;
  for (ecx=0;ecx<Largeur;ecx++)
    *(Dest+ecx)=~*(Dest+ecx);
}

void Ligne_verticale_XOR_Tall(word Pos_X,word Pos_Y,word Hauteur)
{
  int i;
  byte color;
  for (i=Pos_Y*2;i<(Pos_Y+Hauteur)*2;i++)
  {
    color=*(Ecran+Pos_X+i*Largeur_ecran);
    *(Ecran+Pos_X+i*Largeur_ecran)=~color;
  }
}

void Display_brush_Color_Tall(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse)
{
  // Dest = Position à l'écran
  byte* Dest = Ecran + Pos_Y * 2 * Largeur_ecran + Pos_X;
  // Src = Position dans la brosse
  byte* Src = Brosse + Decalage_Y * Largeur_brosse + Decalage_X;

  word DX,CX;

  // Pour chaque ligne
  for(DX = Hauteur;DX > 0; DX--)
  {
    // Pour chaque pixel
    for(CX = Largeur;CX > 0; CX--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*Src != Couleur_de_transparence)
      {
        *Dest = *Src;
        *(Dest+Largeur_ecran) = *Src;
      }

      // Pixel suivant
      Src++; Dest++;
    }

    // On passe à la ligne suivante
    Dest = Dest + 2 * Largeur_ecran - Largeur;
    Src = Src + Largeur_brosse - Largeur;
  }
  UpdateRect(Pos_X,Pos_Y,Largeur,Hauteur);
}

void Display_brush_Mono_Tall(word Pos_X, word Pos_Y,
        word Decalage_X, word Decalage_Y, word Largeur, word Hauteur,
        byte Couleur_de_transparence, byte Couleur, word Largeur_brosse)
/* On affiche la brosse en monochrome */
{
  byte* Dest=Pos_Y*2*Largeur_ecran+Pos_X+Ecran; // Dest = adr Destination à 
      // l'écran
  byte* Src=Largeur_brosse*Decalage_Y+Decalage_X+Brosse; // Src = adr ds 
      // la brosse
  int dx,cx;

  for(dx=Hauteur;dx!=0;dx--)
  //Pour chaque ligne
  {
    for(cx=Largeur;cx!=0;cx--)
    //Pour chaque pixel
    {
      if (*Src!=Couleur_de_transparence)
      {
        *Dest=Couleur;
        *(Dest+Largeur_ecran)=Couleur;
      }

      // On passe au pixel suivant
      Src++;
      Dest++;
    }

    // On passe à la ligne suivante
    Src+=Largeur_brosse-Largeur;
    Dest+=2*Largeur_ecran-Largeur;
  }
  UpdateRect(Pos_X,Pos_Y,Largeur,Hauteur);
}

void Clear_brush_Tall(word Pos_X,word Pos_Y,__attribute__((unused)) word Decalage_X,__attribute__((unused)) word Decalage_Y,word Largeur,word Hauteur,__attribute__((unused))byte Couleur_de_transparence,word Largeur_image)
{
  byte* Dest=Ecran+Pos_X+Pos_Y*2*Largeur_ecran; //On va se mettre en 0,0 dans l'écran (Dest)
  byte* Src = ( Pos_Y + Principal_Decalage_Y ) * Largeur_image + Pos_X + Principal_Decalage_X + Principal_Ecran; //Coords de départ ds la source (Src)
  int dx;

  for(dx=Hauteur;dx!=0;dx--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    memcpy(Dest,Src,Largeur);
    Dest+=Largeur_ecran;
    memcpy(Dest,Src,Largeur);

    // On passe à la ligne suivante
    Src+=Largeur_image;
    Dest+=Largeur_ecran;
  }
  UpdateRect(Pos_X,Pos_Y,Largeur,Hauteur);
}

// Affiche une brosse (arbitraire) à l'écran
void Affiche_brosse_Tall(byte * B, word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse)
{
  // Dest = Position à l'écran
  byte* Dest = Ecran + Pos_Y * 2 * Largeur_ecran + Pos_X;
  // Src = Position dans la brosse
  byte* Src = B + Decalage_Y * Largeur_brosse + Decalage_X;
  
  word DX,CX;
  
  // Pour chaque ligne
  for(DX = Hauteur;DX > 0; DX--)
  {
    // Pour chaque pixel
    for(CX = Largeur;CX > 0; CX--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*Src != Couleur_de_transparence)
      {
        *Dest = *Src;
        *(Dest+Largeur_ecran) = *Src;
      }

      // Pixel suivant
      Src++; Dest++;
    }

    // On passe à la ligne suivante
    Dest = Dest + Largeur_ecran * 2 - Largeur;
    Src = Src + Largeur_brosse - Largeur;
  }
}

void Remap_screen_Tall(word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion)
{
  // Dest = coords a l'écran
  byte* Dest = Ecran + Pos_Y * 2 * Largeur_ecran + Pos_X;
  int dx,cx;

  // Pour chaque ligne
  for(dx=Hauteur*2;dx>0;dx--)
  {
    // Pour chaque pixel
    for(cx=Largeur;cx>0;cx--)
    {
      *Dest = Table_de_conversion[*Dest];
      Dest ++;
    }

    Dest = Dest + Largeur_ecran - Largeur;
  }

  UpdateRect(Pos_X,Pos_Y,Largeur,Hauteur);
}

void Afficher_une_ligne_ecran_Tall(word Pos_X,word Pos_Y,word Largeur,byte * Ligne)
/* On affiche toute une ligne de pixels. Utilisé pour les textes. */
{
  memcpy(Ecran+Pos_X+Pos_Y*2*Largeur_ecran,Ligne,Largeur);
  memcpy(Ecran+Pos_X+(Pos_Y*2+1)*Largeur_ecran,Ligne,Largeur);
}

void Lire_une_ligne_ecran_Tall(word Pos_X,word Pos_Y,word Largeur,byte * Ligne)
{
  memcpy(Ligne,Largeur_ecran * 2 * Pos_Y + Pos_X + Ecran,Largeur);
}

void Afficher_partie_de_l_ecran_zoomee_Tall(
        word Largeur, // Largeur non zoomée
        word Hauteur, // Hauteur zoomée
        word Largeur_image,byte * Buffer)
{
  byte* Src = Principal_Ecran + Loupe_Decalage_Y * Largeur_image 
                      + Loupe_Decalage_X;
  int EDX = 0; // Ligne en cours de traitement

  // Pour chaque ligne à zoomer
  while(1)
  {
    int CX;
    
    // On éclate la ligne
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,Largeur);
    // On l'affiche Facteur fois, sur des lignes consécutives
    CX = Loupe_Facteur*2;
    // Pour chaque ligne
    do{
      // On affiche la ligne zoomée
      Afficher_une_ligne_ecran_Simple(
        Principal_X_Zoom, EDX, Largeur*Loupe_Facteur,
        Buffer
      );
      // On passe à la suivante
      EDX++;
      if(EDX==Hauteur*2)
      {
        UpdateRect(Principal_X_Zoom,0,
          Largeur*Loupe_Facteur,Hauteur);
        return;
      }
      CX--;
    }while (CX > 0);
    Src += Largeur_image;
  }
// ATTENTION on n'arrive jamais ici !
}

// Affiche une partie de la brosse couleur zoomée
void Display_brush_Color_zoom_Tall(word Pos_X,word Pos_Y,
        word Decalage_X,word Decalage_Y,
        word Largeur, // Largeur non zoomée
        word Pos_Y_Fin,byte Couleur_de_transparence,
        word Largeur_brosse, // Largeur réelle de la brosse
        byte * Buffer)
{
  byte* Src = Brosse+Decalage_Y*Largeur_brosse + Decalage_X;
  word DX = Pos_Y;
  byte bx;

  // Pour chaque ligne
  while(1)
  {
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,Largeur);
    // On affiche facteur fois la ligne zoomée
    for(bx=Loupe_Facteur;bx>0;bx--)
    {
      Afficher_une_ligne_transparente_a_l_ecran_Simple(Pos_X,DX*2,Largeur*Loupe_Facteur,Buffer,Couleur_de_transparence);
      memcpy(Ecran + (DX*2 +1) * Largeur_ecran + Pos_X, Ecran + DX*2* Largeur_ecran + Pos_X, Largeur*Loupe_Facteur);
      DX++;
      if(DX==Pos_Y_Fin)
      {
        return;
      }
    }
    Src += Largeur_brosse;
  }
  // ATTENTION zone jamais atteinte
}

void Display_brush_Mono_zoom_Tall(word Pos_X, word Pos_Y,
        word Decalage_X, word Decalage_Y, 
        word Largeur, // Largeur non zoomée 
        word Pos_Y_Fin,
        byte Couleur_de_transparence, byte Couleur, 
        word Largeur_brosse, // Largeur réelle de la brosse
        byte * Buffer
)

{
  byte* Src = Brosse + Decalage_Y * Largeur_brosse + Decalage_X;
  int DX=Pos_Y*2;

  //Pour chaque ligne à zoomer :
  while(1)
  {
    int BX;
    // Src = Ligne originale
    // On éclate la ligne
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,Largeur);

    // On affiche la ligne Facteur fois à l'écran (sur des
    // lignes consécutives)
    BX = Loupe_Facteur*2;

    // Pour chaque ligne écran
    do
    {
      // On affiche la ligne zoomée
      Afficher_une_ligne_transparente_mono_a_l_ecran_Simple(
        Pos_X, DX, Largeur * Loupe_Facteur, 
        Buffer, Couleur_de_transparence, Couleur
      );
      // On passe à la ligne suivante
      DX++;
      // On vérifie qu'on est pas à la ligne finale
      if(DX == Pos_Y_Fin*2)
      {
        UpdateRect( Pos_X, Pos_Y,
          Largeur * Loupe_Facteur, Pos_Y_Fin - Pos_Y );
        return;
      }
      BX --;
    }
    while (BX > 0);
    
    // Passage à la ligne suivante dans la brosse aussi
    Src+=Largeur_brosse;
  }
}

void Clear_brush_zoom_Tall(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,__attribute__((unused)) byte Couleur_de_transparence,word Largeur_image,byte * Buffer)
{
  // En fait on va recopier l'image non zoomée dans la partie zoomée !
  byte* Src = Principal_Ecran + Decalage_Y * Largeur_image + Decalage_X;
  int DX = Pos_Y;
  int bx;

  // Pour chaque ligne à zoomer
  while(1){
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,Largeur);

    bx=Loupe_Facteur;

    // Pour chaque ligne
    do{
      Afficher_une_ligne_ecran_Tall(Pos_X,DX,
        Largeur * Loupe_Facteur,Buffer);

      // Ligne suivante
      DX++;
      if(DX==Pos_Y_Fin)
      {
        UpdateRect(Pos_X,Pos_Y,
          Largeur*Loupe_Facteur,Pos_Y_Fin-Pos_Y);
        return;
      }
      bx--;
    }while(bx!=0);

    Src+= Largeur_image;
  }
}
