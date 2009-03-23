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
void Changer_la_forme_du_pinceau(byte shape);

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

/*!
    Callback for the fill button left click.
    Start the filling operation.
*/
void Bouton_Fill(void);

/*!
    Callback for the fill button right click.
    Start the color replace operation.
*/
void Bouton_Remplacer(void);

/*!
    Disable and release the fill button.
    Restores the cursor (custom brushes are disabled for the fill operation).
    Cleans the status bar if the color replacement tool put a preview color inside it.
*/
void Bouton_desenclencher_Fill(void);

// Spray

/*!
    Callback for the spray button left click.
    Start the spray operation.
*/
void Bouton_Spray(void);

/*!
    Callback for the spray button right click.
    Opens the spray's setup menu.
*/
void Bouton_Spray_Menu(void);

// Courbes de Bézier

/*!
    Callback for the curves button left click.
    Start curve operation according to the selected mode.
*/
void Bouton_Courbes(void);

/*!
    Callback for the curves button right click.
    Select the curve mode (1-point, 2-point)
*/
void Bouton_Courbes_Switch_mode(void);

// Boutons relatifs aux rectangles pleins et vides

/*!
    Callback for the empty rectangle button.
    Start the rectangle operation.
*/
void Bouton_Rectangle_vide(void);

/*!
    Callback for the filled rectangle button.
    Start the filled rectangle operation.
*/
void Bouton_Rectangle_plein(void);

// Boutons relatifs au texte

/*!
    Callback for the text button.
    Opens the text setup window.
*/
void Bouton_Texte(void);

// Boutons relatifs aux dégradés

/*!
    Callback for the gradation button.
    Opens the "Gradation menu". 
*/
void Bouton_Degrades(void);

/*!
    Gets the informations from the gradations table and set the global vars for the current gradation.
    @param Indice index of the selected gradation
*/
void Degrade_Charger_infos_du_tableau(int Indice);

// Boutons relatifs aux cercles (ellipses) dégradé(e)s

/*!
    Callback for the gradation circle button left click.
    Starts drawing a gradation circle.
*/
void Bouton_Cercle_degrade(void);

/*!
    Callback for the gradation circle right click.
    Starts drawing a gradation ellipsis.
*/
void Bouton_Ellipse_degrade(void);

/*!
   Callback for the gradation rectangle button.
   Starts the gradation rectangle drawing operation.
*/
void Bouton_Rectangle_degrade(void);

// Boutons relatifs aux cercles (ellipses) plein(e)s et vides

/*!
    Callback for the circle button left click.
    Starts drawing an empty circle
*/
void Bouton_Cercle_vide(void);

/*!
    Callback for the circle button left click.
    Starts drawing an empty ellipsis
*/
void Bouton_Ellipse_vide(void);

/*!
    Callback for the filled circle button ledt click.
    Starts drawing a filled circle.
*/
void Bouton_Cercle_plein(void);

/*!
    Callback for the filled circle right click.
    Starts drawing a filled ellipsis.
*/
void Bouton_Ellipse_pleine(void);

// Boutons relatifs aux polygones vides et pleins

/*!
    Callback for the polyline button left click.
    Starts drawing a polygon.
*/
void Bouton_Polygone(void);

/*!
    Callback for the polyline right click.
    Starts drawing a polyform.
*/
void Bouton_Polyform(void);

/*!
    Callback for the polyfill button left click.
    Starts drawing a filled polygon.
*/
void Bouton_Polyfill(void);

/*!
    Callback for the polyfill button right click.
    Starts drawing a filled polyform.
*/
void Bouton_Filled_polyform(void);

// Boutons d'ajustement de l'image

/*!
    Callback for the adjust picture button.
    Start the adjust picture operation.
*/
void Bouton_Ajuster(void);

// Gestion du mode Shade

/*!
    Callback for the shade button (in the FX window).
    Toogle the shade mode.
*/
void Bouton_Shade_Mode(void);

/*!
    Callback for the QSHade button (in the FX window).
    Toogle the Quick Shade effect.
*/
void Bouton_Quick_shade_Mode(void);

/*!
    Callback for the Shade button (in the FX window) right click.
    Displays the shade setup menu.
*/
void Bouton_Shade_Menu(void);

// Gestion du Stencil

/*!
    Callback for the Stencil button (in the FX window) left click.
    Toogle stencil mode.
*/
void Bouton_Stencil_Mode(void);

/*!
    Callback for the Stencil button (in the FX window) right click.
    Displays the stencil setup menu.
*/
void Bouton_Menu_Stencil(void);

// Gestion du Masque

/*!
    Callback for the Mask button (in the FX window) left click.
    Toogles the mask mode/
*/
void Bouton_Mask_Mode(void);

/*!
    Callback for the Mask button (in the FX window) right click.
    Displays the mask setup menu.
*/
void Bouton_Mask_Menu(void);

// Mode grille (Snap)

/*!
    Callback for the Grid button (in the FX window) left click.
    Toogle the grid.
*/

void Bouton_Snap_Mode(void);

/*!
    Callback for the Grid button (in the FX window) right click.
    Displays the grid setup menu.
*/
void Bouton_Menu_Grille(void);

// Mode trame (Sieve)

/*!
    In the sieve window, copy one of the presets patterns to the current one.
    @param Indice Index of the pattern to copy
*/
void Copier_trame_predefinie(byte Indice);

/*!
    In the sieve window, swaps black and white in the current pattern.
*/
void Inverser_trame(void);

/*!
    Callback for the Sieve button (in the FX window) left click.
    Toogle sieve mode.
*/
void Bouton_Trame_Mode(void);

/*!
    Callback for the Sieve button (in the FX window) right click.
    Displays the sieve setup menu.
*/
void Bouton_Trame_Menu(void);

// Mode Smooth

/*!
    Callback for the smooth button (in the FX window) left click.
    Toogles smooth mode.
*/
void Bouton_Smooth_Mode(void);

/*!
    Callback for the Smooth button (in the FX window) right click.
    Displays the smooth setup menu.
*/
void Bouton_Smooth_Mode(void);

// Boutons relatifs au mode Colorize

/*!
    Computes the tables used by the transparency/colorize mode.
    These tables are used to match the drawing color*picture color to the color that is painted on screen.
*/
void Calculer_les_tables_de_Colorize(void);

/*!
    Callback for the Tranparency button (in the FX window) left click.
    Toogles transparent drawing mode.
*/
void Bouton_Colorize_Mode(void);

/*!
    Callback for the Transparency button (in the FX window) right click.
    Displays the tranparency setup menu.
*/
void Bouton_Colorize_Menu(void);

// Boutons relatifs au mode Tiling

/*!
    Callback for the Tiling button (in the FX window) left click.
    Toogles tiling mode.
*/
void Bouton_Tiling_Mode(void);

/*!
    Callback for the Tiling button (in the FX window) right click.
    Displays the tiling setup menu.
*/
void Bouton_Tiling_Menu(void);
// Menu des effets

/*!
    Callback for the effects button click.
    Displays the effect selection menu.
*/
void Bouton_Effets(void);

// Prise de brosse

/*!
    Callback for the brush button left click.
    Start the brush picking operation.
*/
void Bouton_Brosse(void);

/*!
    Callback for the brush button right click.
    Activates the last captured custom brush.
*/
void Bouton_Restaurer_brosse(void);

/*!
    Disables the custom brush and set back a regular one.
*/
void Bouton_desenclencher_Brosse(void);
// Prise de brosse au lasso

/*!
    Callback for the freehand brush pick button left click.
    Starts freehand brush picking operation.
*/
void Bouton_Lasso(void);

/*!
    Disables the custom freehand brush and set back a regular one.
*/
void Bouton_desenclencher_Lasso(void);

// Bouton relatifs à la pipette

/*!
    Starts the color picking operation.
*/
void Bouton_Pipette(void);

/*!
    Disables the color picker button and get back to the previously selected drawing mode.
*/
void Bouton_desenclencher_Pipette(void);

/*!
    Swap fore- and background colors.
*/
void Bouton_Inverser_foreback(void);

// Mode loupe

/*!
    Enters magnify mode.
*/
void Bouton_Loupe(void);

/*!
    Displays magnify menu.
*/
void Bouton_Menu_Loupe(void);

/*!
    Exit magnify mode.
*/
void Bouton_desenclencher_Loupe(void);

// Les différents effets sur la brosse

/*!
    Display the Brush effects window.
*/
void Bouton_Brush_FX(void);

// Boutons relatifs aux différentes pages

/*!
    Swap main and spare drawing pages.
*/
void Bouton_Page(void);

/*!
    Copy main page to spare page.
*/
void Bouton_Copy_page(void);

/*!
    Copy only pixel data from main page to spare page (no palette copy).
*/
void Copier_image_seule(void);

/*!
    Kill (free from memory) the current page.
*/
void Bouton_Kill(void);

// Boutons relatifs aux changements de résolution et de taille d'image

/*!
    Display the screenmode menu.
*/
void Bouton_Resol(void);

/*!
    Set the screen to the "safe resolution" (320x200 pixel window).
*/
void Bouton_Safety_resol(void);

// Boutons relatifs aux chargements et sauvegardes

/*!
    Opens the load file dialog.
*/
void Bouton_Load(void);

/*!
    Reload current picture from disk.
*/
void Bouton_Reload(void);

/*!
    Open the save file dialog.
*/
void Bouton_Save(void);

/*!
    Saves the current file without asking for a new name.
*/
void Bouton_Autosave(void);

// Réglage des paramètres de l'utilisateur

/*!
    Display the setting menu.
*/
void Bouton_Settings(void);

// Annulation de la dernière modification

/*!
    Undo the last modification to the picture.
*/
void Bouton_Undo(void);

/*!
    Redo an operation that has been undone.
*/
void Bouton_Redo(void);

// Boutons relatifs aux effacements d'images

/*!
    Clear the whole screen with black (color index 0).
*/
void Bouton_Clear(void);

/*!
    Clear the screen with the selected backcolor.
*/
void Bouton_Clear_colore(void);

// Quitter le programme

/*!
    Quits the program. Display a requester to save the changes to the picture before exiting if the pic was modified since last save.
*/
void Bouton_Quit(void);

// Cacher le menu

/*!
    Hides the menubar.
*/
void Bouton_Cacher_menu(void);


/*!
    Load picture from file.
*/
void Load_picture(byte image);

/*!
    Save picture to file.
*/
void Save_picture(byte image);


/*!
    Generic color tagging menu, for various effects.
*/
void Menu_Tag_couleurs(char * En_tete, byte * Table, byte * mode, byte can_cancel, const char *Section_aide);


/*!
    Display the menu for the smooth effect.
*/
void Bouton_Smooth_Menu(void);


/*!
    Toogles the smear mode.
*/
void Bouton_Smear_Mode(void);

#endif

