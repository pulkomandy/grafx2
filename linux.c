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
// Diverses fonctions qui existaient sous dos mais pas sous linux...
#ifdef __linux__

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

int filelength(int fichier)
{
	struct stat infos_fichier;
	fstat(fichier,&infos_fichier);
	return infos_fichier.st_size;
}

void itoa(int source,char* dest, int longueur)
{
	snprintf(dest,longueur,"%d",source);
}

#endif
