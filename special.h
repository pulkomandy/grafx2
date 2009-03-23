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
void Modifier_pinceau(int width, int height);
void Retrecir_pinceau(void);
void Grossir_pinceau(void);

void Special_Next_forecolor(void);
void Special_Previous_forecolor(void);
void Special_Next_backcolor(void);
void Special_Previous_backcolor(void);

void Scroller_ecran(short Decalage_en_X,short Decalage_en_Y);
void Scroller_loupe(short Decalage_en_X,short Decalage_en_Y);

void Zoom(short delta);
