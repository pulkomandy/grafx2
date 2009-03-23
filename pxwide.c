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

void Pixel_Wide (word x,word y,byte Couleur)
/* Affiche un pixel de la Couleur aux coords x;y à l'écran */
{
  *(Ecran + x * 2 + y * 2 * Largeur_ecran)=Couleur;
  *(Ecran + x * 2 + y * 2 * Largeur_ecran + 1)=Couleur;
}

byte Lit_Pixel_Wide (word x,word y)
/* On retourne la couleur du pixel aux coords données */
{
  return *( Ecran + y * 2 * Largeur_ecran + x * 2);
}

void Block_Wide (word Debut_X,word Debut_Y,word width,word height,byte Couleur)
/* On affiche un rectangle de la couleur donnée */
{
  SDL_Rect rectangle;
  rectangle.x=Debut_X*2;
  rectangle.y=Debut_Y;
  rectangle.w=width*2;
  rectangle.h=height;
  SDL_FillRect(Ecran_SDL,&rectangle,Couleur);
}

void Afficher_partie_de_l_ecran_Wide (word width,word height,word image_width)
/* Afficher une partie de l'image telle quelle sur l'écran */
{
  byte* Dest=Ecran; //On va se mettre en 0,0 dans l'écran (Dest)
  byte* Src=Principal_Decalage_Y*image_width+Principal_Decalage_X+Principal_Ecran; //Coords de départ ds la source (Src)
  int y;
  int dy;

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    // On fait une copie de la ligne
    for (dy=width;dy>0;dy--)
    {
      *(Dest+1)=*Dest=*Src;
      Src++;
      Dest+=2;
    }

    // On passe à la ligne suivante
    Src+=image_width-width;
    Dest+=(Largeur_ecran - width)*2;
  }
  //UpdateRect(0,0,width,height);
}

void Pixel_Preview_Normal_Wide (word x,word y,byte Couleur)
/* Affichage d'un pixel dans l'écran, par rapport au décalage de l'image 
 * dans l'écran, en mode normal (pas en mode loupe)
 * Note: si on modifie cette procédure, il faudra penser à faire également 
 * la modif dans la procédure Pixel_Preview_Loupe_SDL. */
{
//  if(x-Principal_Decalage_X >= 0 && y - Principal_Decalage_Y >= 0)
  Pixel_Wide(x-Principal_Decalage_X,y-Principal_Decalage_Y,Couleur);
}

void Pixel_Preview_Loupe_Wide  (word x,word y,byte Couleur)
{
  // Affiche le pixel dans la partie non zoomée
  Pixel_Wide(x-Principal_Decalage_X,y-Principal_Decalage_Y,Couleur);
  
  // Regarde si on doit aussi l'afficher dans la partie zoomée
  if (y >= Limite_Haut_Zoom && y <= Limite_visible_Bas_Zoom
          && x >= Limite_Gauche_Zoom && x <= Limite_visible_Droite_Zoom)
  {
    // On est dedans
    int height;
    int Y_Zoom = Table_mul_facteur_zoom[y-Loupe_Decalage_Y];

    if (Menu_Ordonnee - Y_Zoom < Loupe_Facteur)
      // On ne doit dessiner qu'un morceau du pixel
      // sinon on dépasse sur le menu
      height = Menu_Ordonnee - Y_Zoom;
    else
      height = Loupe_Facteur;

    Block_Wide(
      Table_mul_facteur_zoom[x-Loupe_Decalage_X]+Principal_X_Zoom, 
      Y_Zoom, Loupe_Facteur, height, Couleur
      );
  }
}

void Ligne_horizontale_XOR_Wide(word x_pos,word y_pos,word width)
{
  //On calcule la valeur initiale de Dest:
  byte* Dest=y_pos*2*Largeur_ecran+x_pos*2+Ecran;

  int x;

  for (x=0;x<width*2;x+=2)
    *(Dest+x+1)=*(Dest+x)=~*(Dest+x);
}

void Ligne_verticale_XOR_Wide(word x_pos,word y_pos,word height)
{
  int i;
  byte color;
  byte *Dest=Ecran+x_pos*2+y_pos*Largeur_ecran*2;
  for (i=height;i>0;i--)
  {
    color=~*Dest;
    *Dest=color;
    *(Dest+1)=color;
    Dest+=Largeur_ecran*2;
  }
}

void Display_brush_Color_Wide(word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,word Largeur_brosse)
{
  // Dest = Position à l'écran
  byte* Dest = Ecran + y_pos * 2 * Largeur_ecran + x_pos * 2;
  // Src = Position dans la brosse
  byte* Src = Brosse + y_offset * Largeur_brosse + x_offset;

  word x,y;

  // Pour chaque ligne
  for(y = height;y > 0; y--)
  {
    // Pour chaque pixel
    for(x = width;x > 0; x--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*Src != Couleur_de_transparence)
      {
        *(Dest+1) = *Dest = *Src;
      }

      // Pixel suivant
      Src++;
      Dest+=2;
    }

    // On passe à la ligne suivante
    Dest = Dest + (Largeur_ecran - width)*2;
    Src = Src + Largeur_brosse - width;
  }
  UpdateRect(x_pos,y_pos,width,height);
}

void Display_brush_Mono_Wide(word x_pos, word y_pos,
        word x_offset, word y_offset, word width, word height,
        byte Couleur_de_transparence, byte Couleur, word Largeur_brosse)
/* On affiche la brosse en monochrome */
{
  byte* Dest=y_pos*2*Largeur_ecran+x_pos*2+Ecran; // Dest = adr Destination à 
      // l'écran
  byte* Src=Largeur_brosse*y_offset+x_offset+Brosse; // Src = adr ds 
      // la brosse
  int x,y;

  for(y=height;y!=0;y--)
  //Pour chaque ligne
  {
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      if (*Src!=Couleur_de_transparence)
        *(Dest+1)=*Dest=Couleur;

      // On passe au pixel suivant
      Src++;
      Dest+=2;
    }

    // On passe à la ligne suivante
    Src+=Largeur_brosse-width;
    Dest+=(Largeur_ecran-width)*2;
  }
  UpdateRect(x_pos,y_pos,width,height);
}

void Clear_brush_Wide(word x_pos,word y_pos,__attribute__((unused)) word x_offset,__attribute__((unused)) word y_offset,word width,word height,__attribute__((unused))byte Couleur_de_transparence,word image_width)
{
  byte* Dest=Ecran+x_pos*2+y_pos*2*Largeur_ecran; //On va se mettre en 0,0 dans l'écran (Dest)
  byte* Src = ( y_pos + Principal_Decalage_Y ) * image_width + x_pos + Principal_Decalage_X + Principal_Ecran; //Coords de départ ds la source (Src)
  int y;
  int x;

  for(y=height;y!=0;y--)
  // Pour chaque ligne
  {
    for(x=width;x!=0;x--)
    //Pour chaque pixel
    {
      *(Dest+1)=*Dest=*Src;

      // On passe au pixel suivant
      Src++;
      Dest+=2;
    }

    // On passe à la ligne suivante
    Src+=image_width-width;
    Dest+=(Largeur_ecran-width)*2;
  }
  UpdateRect(x_pos,y_pos,width,height);
}

// Affiche une brosse (arbitraire) à l'écran
void Affiche_brosse_Wide(byte * brush, word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,word Largeur_brosse)
{
  // Dest = Position à l'écran
  byte* Dest = Ecran + y_pos * 2 * Largeur_ecran + x_pos * 2;
  // Src = Position dans la brosse
  byte* Src = brush + y_offset * Largeur_brosse + x_offset;
  
  word x,y;
  
  // Pour chaque ligne
  for(y = height;y > 0; y--)
  {
    // Pour chaque pixel
    for(x = width;x > 0; x--)
    {
      // On vérifie que ce n'est pas la transparence
      if(*Src != Couleur_de_transparence)
      {
        *(Dest+1) = *Dest = *Src;
      }

      // Pixel suivant
      Src++; Dest+=2;
    }

    // On passe à la ligne suivante
    Dest = Dest + (Largeur_ecran - width)*2;
    Src = Src + Largeur_brosse - width;
  }
}

void Remap_screen_Wide(word x_pos,word y_pos,word width,word height,byte * Table_de_conversion)
{
  // Dest = coords a l'écran
  byte* Dest = Ecran + y_pos * 2 * Largeur_ecran + x_pos * 2;
  int x,y;

  // Pour chaque ligne
  for(y=height;y>0;y--)
  {
    // Pour chaque pixel
    for(x=width;x>0;x--)
    {
      *(Dest+1) = *Dest = Table_de_conversion[*Dest];
      Dest +=2;
    }

    Dest = Dest + (Largeur_ecran - width)*2;
  }

  UpdateRect(x_pos,y_pos,width,height);
}

void Afficher_une_ligne_ecran_fast_Wide(word x_pos,word y_pos,word width,byte * line)
/* On affiche toute une ligne de pixels telle quelle. */
/* Utilisée si le buffer contient déja des pixel doublés. */
{
  memcpy(Ecran+x_pos*2+y_pos*2*Largeur_ecran,line,width*2);
}

void Afficher_une_ligne_ecran_Wide(word x_pos,word y_pos,word width,byte * line)
/* On affiche une ligne de pixels en les doublant. */
{
  int x;
  byte *Dest;
  Dest=Ecran+x_pos*2+y_pos*2*Largeur_ecran;
  for(x=width;x>0;x--)
  {
    *Dest=*line;
    Dest++;
    *Dest=*line;
    Dest++;
    line++;
  }
}
void Afficher_une_ligne_transparente_mono_a_l_ecran_Wide(
        word x_pos, word y_pos, word width, byte* line, 
        byte Couleur_transparence, byte Couleur)
// Affiche une ligne à l'écran avec une couleur + transparence.
// Utilisé par les brosses en mode zoom
{
  byte* Dest = Ecran+ y_pos*2 * Largeur_ecran + x_pos*2;
  int x;
  // Pour chaque pixel
  for(x=0;x<width;x++)
  {
    if (Couleur_transparence!=*line)
    {
      *Dest = Couleur;
      *(Dest+1) = Couleur;
    }
    line ++; // Pixel suivant
    Dest+=2;
  }
}

void Lire_une_ligne_ecran_Wide(word x_pos,word y_pos,word width,byte * line)
{
  memcpy(line,Largeur_ecran * 2 * y_pos + x_pos * 2 + Ecran,width*2);
}

void Afficher_partie_de_l_ecran_zoomee_Wide(
        word width, // width non zoomée
        word height, // height zoomée
        word image_width,byte * Buffer)
{
  byte* Src = Principal_Ecran + Loupe_Decalage_Y * image_width 
                      + Loupe_Decalage_X;
  int y = 0; // Ligne en cours de traitement

  // Pour chaque ligne à zoomer
  while(1)
  {
    int x;
    
    // On éclate la ligne
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur*2,width);
    // On l'affiche Facteur fois, sur des lignes consécutives
    x = Loupe_Facteur;
    // Pour chaque ligne
    do{
      // On affiche la ligne zoomée
      Afficher_une_ligne_ecran_fast_Wide(
        Principal_X_Zoom, y, width*Loupe_Facteur,
        Buffer
      );
      // On passe à la suivante
      y++;
      if(y==height)
      {
        UpdateRect(Principal_X_Zoom,0,
          width*Loupe_Facteur,height);
        return;
      }
      x--;
    }while (x > 0);
    Src += image_width;
  }
// ATTENTION on n'arrive jamais ici !
}

void Afficher_une_ligne_transparente_a_l_ecran_Wide(word x_pos,word y_pos,word width,byte* line,byte Couleur_transparence)
{
  byte* Src = line;
  byte* Dest = Ecran + y_pos * 2 * Largeur_ecran + x_pos * 2;

  word x;

  // Pour chaque pixel de la ligne
  for(x = width;x > 0;x--)
  {
    if(*Src!=Couleur_transparence)
    {
      *Dest = *Src;
      *(Dest+1) = *Src;
    }
    Src++;
    Dest+=2;
  }
}

// Affiche une partie de la brosse couleur zoomée
void Display_brush_Color_zoom_Wide(word x_pos,word y_pos,
        word x_offset,word y_offset,
        word width, // width non zoomée
        word Pos_Y_Fin,byte Couleur_de_transparence,
        word Largeur_brosse, // width réelle de la brosse
        byte * Buffer)
{
  byte* Src = Brosse+y_offset*Largeur_brosse + x_offset;
  word y = y_pos;
  byte bx;

  // Pour chaque ligne
  while(1)
  {
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,width);
    // On affiche facteur fois la ligne zoomée
    for(bx=Loupe_Facteur;bx>0;bx--)
    {
      Afficher_une_ligne_transparente_a_l_ecran_Wide(x_pos,y,width*Loupe_Facteur,Buffer,Couleur_de_transparence);
      y++;
      if(y==Pos_Y_Fin)
      {
        return;
      }
    }
    Src += Largeur_brosse;
  }
  // ATTENTION zone jamais atteinte
}

void Display_brush_Mono_zoom_Wide(word x_pos, word y_pos,
        word x_offset, word y_offset, 
        word width, // width non zoomée 
        word Pos_Y_Fin,
        byte Couleur_de_transparence, byte Couleur, 
        word Largeur_brosse, // width réelle de la brosse
        byte * Buffer
)

{
  byte* Src = Brosse + y_offset * Largeur_brosse + x_offset;
  int y=y_pos;

  //Pour chaque ligne à zoomer :
  while(1)
  {
    int BX;
    // Src = Ligne originale
    // On éclate la ligne
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur,width);

    // On affiche la ligne Facteur fois à l'écran (sur des
    // lignes consécutives)
    BX = Loupe_Facteur;

    // Pour chaque ligne écran
    do
    {
      // On affiche la ligne zoomée
      Afficher_une_ligne_transparente_mono_a_l_ecran_Wide(
        x_pos, y, width * Loupe_Facteur, 
        Buffer, Couleur_de_transparence, Couleur
      );
      // On passe à la ligne suivante
      y++;
      // On vérifie qu'on est pas à la ligne finale
      if(y == Pos_Y_Fin)
      {
        UpdateRect( x_pos, y_pos,
          width * Loupe_Facteur, Pos_Y_Fin - y_pos );
        return;
      }
      BX --;
    }
    while (BX > 0);
    
    // Passage à la ligne suivante dans la brosse aussi
    Src+=Largeur_brosse;
  }
}

void Clear_brush_zoom_Wide(word x_pos,word y_pos,word x_offset,word y_offset,word width,word Pos_Y_Fin,__attribute__((unused)) byte Couleur_de_transparence,word image_width,byte * Buffer)
{
  // En fait on va recopier l'image non zoomée dans la partie zoomée !
  byte* Src = Principal_Ecran + y_offset * image_width + x_offset;
  int y = y_pos;
  int bx;

  // Pour chaque ligne à zoomer
  while(1){
    Zoomer_une_ligne(Src,Buffer,Loupe_Facteur*2,width);

    bx=Loupe_Facteur;

    // Pour chaque ligne
    do{
      Afficher_une_ligne_ecran_fast_Wide(x_pos,y,
        width * Loupe_Facteur,Buffer);

      // Ligne suivante
      y++;
      if(y==Pos_Y_Fin)
      {
        UpdateRect(x_pos,y_pos,
          width*Loupe_Facteur,Pos_Y_Fin-y_pos);
        return;
      }
      bx--;
    }while(bx!=0);

    Src+= image_width;
  }
}


