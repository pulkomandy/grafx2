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
#include "struct.h"

  void Pixel_Double                      (word X,word Y,byte Couleur);
  byte Lit_Pixel_Double                  (word X,word Y);
  void Block_Double                      (word Debut_X,word Debut_Y,word width,word height,byte Couleur);
  void Pixel_Preview_Normal_Double       (word X,word Y,byte Couleur);
  void Pixel_Preview_Loupe_Double        (word X,word Y,byte Couleur);
  void Ligne_horizontale_XOR_Double      (word x_pos,word y_pos,word width);
  void Ligne_verticale_XOR_Double        (word x_pos,word y_pos,word height);
  void Display_brush_Color_Double        (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,word Largeur_brosse);
  void Display_brush_Mono_Double         (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse);
  void Clear_brush_Double                (word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,word image_width);
  void Remap_screen_Double               (word x_pos,word y_pos,word width,word height,byte * Table_de_conversion);
  void Afficher_partie_de_l_ecran_Double (word width,word height,word image_width);
  void Afficher_une_ligne_ecran_Double   (word x_pos,word y_pos,word width,byte * line);
  void Lire_une_ligne_ecran_Double       (word x_pos,word y_pos,word width,byte * line);
  void Afficher_partie_de_l_ecran_zoomee_Double(word width,word height,word image_width,byte * Buffer);
  void Display_brush_Color_zoom_Double   (word x_pos,word y_pos,word x_offset,word y_offset,word width,word Pos_Y_Fin,byte Couleur_de_transparence,word Largeur_brosse,byte * Buffer);
  void Display_brush_Mono_zoom_Double    (word x_pos,word y_pos,word x_offset,word y_offset,word width,word Pos_Y_Fin,byte Couleur_de_transparence,byte Couleur,word Largeur_brosse,byte * Buffer);
  void Clear_brush_zoom_Double           (word x_pos,word y_pos,word x_offset,word y_offset,word width,word Pos_Y_Fin,byte Couleur_de_transparence,word image_width,byte * Buffer);
  void Affiche_brosse_Double             (byte * brush, word x_pos,word y_pos,word x_offset,word y_offset,word width,word height,byte Couleur_de_transparence,word Largeur_brosse);

  void Afficher_une_ligne_ecran_fast_Double   (word x_pos,word y_pos,word width,byte * line);
