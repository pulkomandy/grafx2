/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Adrien Destugues
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

// Initialisation à faire une fois au début du programme
void Initialisation_Texte(void);
// Informe si texte.c a été compilé avec l'option de support TrueType ou pas.
int Support_TrueType(void);
// Ajout d'une fonte à la liste.
void Ajout_fonte(char *Nom);
// Crée une brosse à partir des paramètres de texte demandés.
byte *Rendu_Texte(const char *Chaine, int Numero_fonte, int Taille, int AntiAlias, int *Largeur, int *Hauteur);
// Trouve le libellé d'affichage d'une fonte par son numéro
char * Libelle_fonte(int Indice);
// Trouve le nom d'une fonte par son numéro
char * Nom_fonte(int Indice);
// Nombre de fontes déclarées
extern int Fonte_nombre;
