/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Adrien Destugues

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
// Affiche nom fichier, n° ligne, nom fonction, une chaine et un entier.
#define DEBUG(y,z) printf("%s %d %s | %s : %d###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

// DEBUG en hexadécimal
#define DEBUGX(y,z) printf("%s %d %s | %s : %X###\n",__FILE__,__LINE__,__func__,y,(unsigned int)z)

#define UNIMPLEMENTED printf("%s %d %s non implémenté !\n",__FILE__,__LINE__,__func__);

#define UNTESTED printf("%s %d %s à tester !\n",__FILE__,__LINE__,__func__);

void Erreur_fonction(int error_code, const char *filename, int Numero_ligne, const char *Nom_fonction);
#define Erreur(n) Erreur_fonction(n, __FILE__,__LINE__,__func__)

