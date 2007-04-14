#ifndef SDLSCREEN_H_INCLUDED
#define SDLSCREEN_H_INCLUDED

#include <SDL/SDL.h>
#include "struct.h"
//Set_Mode_SDL();

  void Pixel_SDL                (word X,word Y,byte Couleur);
  byte Lit_Pixel_SDL            (word X,word Y);
  void Effacer_Tout_l_Ecran_SDL (byte Couleur);
  void Block_SDL                (word Debut_X,word Debut_Y,word Largeur,word Hauteur,byte Couleur);
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

  SDL_Rect ** Liste_Modes_Videos_SDL;
  SDL_Surface* Ecran;

  SDL_Event* Evenement_SDL;

#endif // SDLSCREEN_H_INCLUDED
