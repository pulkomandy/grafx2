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

#ifndef __BOUTONS_H_
#define __BOUTONS_H_

/*!
    Displays an error message when calling an unavailable function.
    Only used in the beta versions for things we haven't coded yet.
*/
void Message_Non_disponible(void);

/*!
    Displays an error message when there is no more memory for the requested operation. 
*/
void Message_Memoire_insuffisante(void);

/*!
    Displays the splash screen at program startup.
*/
void Bouton_Message_initial(void);

/*!
    Changes brush shape. 
    This function saves the current brush shape and swith to the default one (single pixel brush) for the filler and the color picker. 
    These functions don't need (and will not work with) a custom brush.
*/
void Changer_la_forme_du_pinceau(byte Numero);

// Boutons relatifs aux couleurs


/*!
    Callback for the palette scroller buttons left click.
    Scrolls the menubar palette one column to the left.
*/
void Bouton_Pal_left(void);

/*!
    Callback for the palette scroller buttons right click.
    Scrolls the menubar palette faster to the left.
*/
void Bouton_Pal_left_fast(void);

/*!
    Callback for the palette scroller buttons left click.
    Scrolls the menubar palette one column to the right.
*/
void Bouton_Pal_right(void);

/*!
    Callback for the palette scroller buttons right click.
    Scrolls the menubar palette faster to the right.
*/
void Bouton_Pal_right_fast(void);

/*!
    Callback for the palette color buttons left click.
    Selects the foreground drawing color when clicking on the menubar palette.
*/
void Bouton_Choix_forecolor(void);

/*!
    Callback for the palette color buttons right click.
    Selects the background drawing color when clicking on the menubar palette.
*/
void Bouton_Choix_backcolor(void);

// Boutons relatifs au pinceaux

/*!
    Callback for the brush button left click.
    Selects the monochrome brush mode when right clicking on the brush button.
*/
void Bouton_Brosse_monochrome(void);

/*!
    Callback for the brush button right click.
    Displays the "Paintbrush menu".
*/
void Bouton_Menu_pinceaux(void);

// Boutons relatifs au mode de dessin à main levée

/*!
    Callback for the freehand draw button left click.
    Selects freehand drawing mode, depending on the current state of the freehand button.
*/
void Bouton_Dessin(void);

/*!
    Callback for the freehand draw button right click.
    Cycles the drawing modes for the freehand tool.
*/
void Bouton_Dessin_Switch_mode(void);

// Dessin par ligne

/*!
    Callback for the lines button left click.
    Selects lines drawing mode, depending on the current state of the lines button.
*/
void Bouton_Lignes(void);

/*!
    Callback for the lines button right click.
    Cycles the drawing modes for the lines tool.
*/
void Bouton_Lignes_Switch_mode(void);

// Bouton relatif au remplissage
void Bouton_Fill(void);
void Bouton_Remplacer(void);
void Bouton_desenclencher_Fill(void);

// Spray
void Bouton_Spray(void);
void Bouton_Spray_Menu(void);

// Courbes de Bézier
void Bouton_Courbes(void);
void Bouton_Courbes_Switch_mode(void);

// Boutons relatifs aux rectangles pleins et vides
void Bouton_Rectangle_vide(void);
void Bouton_Rectangle_plein(void);

// Boutons relatifs au texte
void Bouton_Texte(void);

// Boutons relatifs aux dégradés
void Bouton_Degrades(void);
void Degrade_Charger_infos_du_tableau(int Indice);

// Boutons relatifs aux cercles (ellipses) dégradé(e)s
void Bouton_Cercle_degrade(void);
void Bouton_Ellipse_degrade(void);
void Bouton_Rectangle_degrade(void);

// Boutons relatifs aux cercles (ellipses) plein(e)s et vides
void Bouton_Cercle_vide(void);
void Bouton_Ellipse_vide(void);
void Bouton_Cercle_plein(void);
void Bouton_Ellipse_pleine(void);

// Boutons relatifs aux polygones vides et pleins
void Bouton_Polygone(void);
void Bouton_Polyform(void);
void Bouton_Polyfill(void);
void Bouton_Filled_polyform(void);

// Boutons d'ajustement de l'image
void Bouton_Ajuster(void);

// Gestion du mode Shade
void Bouton_Shade_Mode(void);
void Bouton_Quick_shade_Mode(void);
void Bouton_Shade_Menu(void);
// Gestion du Stencil
void Bouton_Stencil_Mode(void);
void Bouton_Menu_Stencil(void);
// Gestion du Masque
void Bouton_Mask_Mode(void);
void Bouton_Mask_Menu(void);
// Mode grille (Snap)
void Bouton_Snap_Mode(void);
void Bouton_Menu_Grille(void);
// Mode trame (Sieve)
void Copier_trame_predefinie(byte Indice);
void Inverser_trame(void);
void Bouton_Trame_Mode(void);
void Bouton_Trame_Menu(void);
// Mode Smooth
void Bouton_Smooth_Mode(void);
void Bouton_Smooth_Mode(void);
// Boutons relatifs au mode Colorize
void Calculer_les_tables_de_Colorize(void);
void Bouton_Colorize_Mode(void);
void Bouton_Colorize_Menu(void);
// Boutons relatifs au mode Tiling
void Bouton_Tiling_Mode(void);
void Bouton_Tiling_Menu(void);
// Menu des effets
void Bouton_Effets(void);

// Prise de brosse
void Bouton_Brosse(void);
void Bouton_Restaurer_brosse(void);
void Bouton_desenclencher_Brosse(void);
// Prise de brosse au lasso
void Bouton_Lasso(void);
void Bouton_desenclencher_Lasso(void);

// Bouton relatifs à la pipette
void Bouton_Pipette(void);
void Bouton_desenclencher_Pipette(void);
void Bouton_Inverser_foreback(void);

// Mode loupe
void Bouton_Loupe(void);
void Bouton_Menu_Loupe(void);
void Bouton_desenclencher_Loupe(void);

// Les différents effets sur la brosse
void Bouton_Brush_FX(void);

// Boutons relatifs aux différentes pages
void Bouton_Page(void);
void Bouton_Copy_page(void);
void Copier_image_seule(void);
void Bouton_Kill(void);

// Boutons relatifs aux changements de résolution et de taille d'image
void Bouton_Resol(void);
void Bouton_Safety_resol(void);

// Boutons relatifs aux chargements et sauvegardes
void Bouton_Load(void);
void Bouton_Reload(void);
void Bouton_Save(void);
void Bouton_Autosave(void);

// Réglage des paramètres de l'utilisateur
void Bouton_Settings(void);

// Annulation de la dernière modification
void Bouton_Undo(void);
void Bouton_Redo(void);

// Boutons relatifs aux effacements d'images
void Bouton_Clear(void);
void Bouton_Clear_colore(void);

// Quitter le programme
void Bouton_Quit(void);

// Cacher le menu
void Bouton_Cacher_menu(void);

void Load_picture(byte Image);
void Save_picture(byte Image);

void Menu_Tag_couleurs(char * En_tete, byte * Table, byte * Mode, byte Cancel, const char *Section_aide);

void Bouton_Smooth_Menu(void);

void Bouton_Smear_Mode(void);

#endif

