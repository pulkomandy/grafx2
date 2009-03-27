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
#include <stdbool.h>
#include <SDL.h>

/*** Types definitions and structs ***/

typedef struct
{
    word Number;
    char Label[36];
    char Explic1[37];
    char Explic2[37];
    char Explic3[37];
    bool Suppr;       // Shortcut facultatif
    word Key;
    word Key2;
} T_Key_config;

extern T_Key_config ConfigKey[NB_SHORTCUTS];
extern word Ordering[NB_SHORTCUTS];
