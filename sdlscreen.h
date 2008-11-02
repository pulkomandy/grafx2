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
#ifndef SDLSCREEN_H_INCLUDED
#define SDLSCREEN_H_INCLUDED

#include <SDL/SDL.h>
#include "struct.h"

  void Pixel_SDL                (word X,word Y,byte Couleur);
  byte Lit_Pixel_SDL            (word X,word Y);
  void Block_SDL                (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);
  void Block_SDL_Fast           (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);
  void Pixel_Preview_Normal_SDL (word X,word Y,byte Couleur);
  void Pixel_Preview_Loupe_SDL  (word X,word Y,byte Couleur);
  void Ligne_horizontale_XOR_SDL(word Pos_X,word Pos_Y,word Largeur);
  void Ligne_verticale_XOR_SDL  (word Pos_X,word Pos_Y,word Hauteur);
  void Display_brush_Color_SDL  (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);
  void Display_brush_Mono_SDL   (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
  void Clear_brush_SDL          (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_image);
  void Remap_screen_SDL         (word Pos_X,word Pos_Y,word Largeur,word Hauteur,byte * Table_de_conversion);
  void Afficher_partie_de_l_ecran_SDL       (word Largeur,word Hauteur,word Largeur_image);
  void Afficher_une_ligne_ecran_SDL     (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
  void Lire_une_ligne_ecran_SDL         (word Pos_X,word Pos_Y,word Largeur,byte * Ligne);
  void Afficher_partie_de_l_ecran_zoomee_SDL(word Largeur,word Hauteur,word Largeur_image,byte * Buffer);
  void Display_brush_Color_zoom_SDL(word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
  void Display_brush_Mono_zoom_SDL (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
  void Clear_brush_zoom_SDL        (word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_image,byte * Buffer);
  void Set_Mode_SDL();
  void Gere_Evenement_SDL(SDL_Event * event);

  SDL_Rect ** Liste_Modes_Videos_SDL;
  byte* Ecran;

  void UpdateRect(short X, short Y, unsigned short Largeur, unsigned short Hauteur);
  void Flush_update(void);
  byte * Surface_en_bytefield(SDL_Surface *Source, byte * Destination);
  SDL_Color Conversion_couleur_SDL(byte);
  void Affiche_brosse_SDL(byte * B, word Pos_X,word Pos_Y,word Decalage_X,word Decalage_Y,word Largeur,word Hauteur,byte Couleur_de_transparence,word Largeur_brosse);

#endif // SDLSCREEN_H_INCLUDED
