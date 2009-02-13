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

void Pixel_Chargement_dans_ecran_courant(word Pos_X,word Pos_Y,byte Couleur);
void Pixel_Chargement_dans_preview      (word Pos_X,word Pos_Y,byte Couleur);
void Pixel_Chargement_dans_brosse       (word Pos_X,word Pos_Y,byte Couleur);

void Nom_fichier_complet(char * Nom_du_fichier, byte Sauve_Colorix);

void Charger_image(byte Image);
void Sauver_image(byte Image);

typedef struct {
  char *Extension;
  fonction_action Test;
  fonction_action Load;
  fonction_action Save;
  byte Backup_done; // Le format enregistre toute l'image, on la considère à jour.
  byte Commentaire; // Le format de fichier autorise un commentaire.
} T_Format;

// Tableau des formats connus
extern T_Format FormatFichier[NB_FORMATS_CONNUS];

// Fonction de sauvegarde en cas de probleme
void Image_emergency_backup(void);
