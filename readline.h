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

byte Readline(word x_pos,word y_pos,char * str,byte visible_size,byte input_type);
// Paramètres:
//   x_pos, y_pos    : Coordonnées de la saisie dans la fenêtre
//   str          : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   visible_size : Nombre de caractères logeant dans la zone de saisie
//   input_type     : 0=Chaîne, 1=Nombre, 2=Nom de fichier (255 caractères réels)
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)

byte Readline_ex(word x_pos,word y_pos,char * str,byte visible_size,byte max_size, byte input_type);
// Paramètres:
//   x_pos, y_pos    : Coordonnées de la saisie dans la fenêtre
//   str          : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   visible_size : Nombre de caractères logeant dans la zone de saisie
//   max_size     : Nombre de caractères logeant dans la zone de saisie
//   input_type     : 0=Chaîne, 1=Nombre, 2=Nom de fichier (255 caractères réels)
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
