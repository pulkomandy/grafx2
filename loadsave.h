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

void Pixel_load_in_current_screen(word x_pos,word y_pos,byte color);
void Pixel_load_in_preview      (word x_pos,word y_pos,byte color);
void Pixel_load_in_brush       (word x_pos,word y_pos,byte color);

void filename_complet(char * filename, byte is_colorix_format);

void Load_image(byte image);
void Save_image(byte image);

typedef struct {
  char *Extension;
  Func_action Test;
  Func_action Load;
  Func_action Save;
  byte Backup_done; // Le format enregistre toute l'image, on la considère à jour.
  byte Comment; // Le format de fichier autorise un commentaire.
} T_Format;

// Tableau des formats connus
extern T_Format File_formats[NB_KNOWN_FORMATS];

// Fonction de sauvegarde en cas de probleme
void Image_emergency_backup(void);
