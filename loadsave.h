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

// -- PKM -------------------------------------------------------------------
void Test_PKM(void);
void Load_PKM(void);
void Save_PKM(void);

// -- LBM -------------------------------------------------------------------
void Test_LBM(void);
void Load_LBM(void);
void Save_LBM(void);

  byte * LBM_Buffer;


// -- GIF -------------------------------------------------------------------
void Test_GIF(void);
void Load_GIF(void);
void Save_GIF(void);

// -- PCX -------------------------------------------------------------------
void Test_PCX(void);
void Load_PCX(void);
void Save_PCX(void);

// -- BMP -------------------------------------------------------------------
void Test_BMP(void);
void Load_BMP(void);
void Save_BMP(void);

// -- IMG -------------------------------------------------------------------
void Test_IMG(void);
void Load_IMG(void);
void Save_IMG(void);

// -- SCx -------------------------------------------------------------------
void Test_SCx(void);
void Load_SCx(void);
void Save_SCx(void);

// -- CEL -------------------------------------------------------------------
void Test_CEL(void);
void Load_CEL(void);
void Save_CEL(void);

// -- KCF -------------------------------------------------------------------
void Test_KCF(void);
void Load_KCF(void);
void Save_KCF(void);

// -- PAL -------------------------------------------------------------------
void Test_PAL(void);
void Load_PAL(void);
void Save_PAL(void);

// -- PI1 -------------------------------------------------------------------
void Test_PI1(void);
void Load_PI1(void);
void Save_PI1(void);

// -- PC1 -------------------------------------------------------------------
void Test_PC1(void);
void Load_PC1(void);
void Save_PC1(void);
