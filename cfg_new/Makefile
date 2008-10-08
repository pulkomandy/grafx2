#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2008 Adrien Destugues
#  Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)
#  
#  Grafx2 is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; version 2
#  of the License.
#  
#  Grafx2 is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
#  write to the Free Software Foundation, Inc.,
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

ifdef COMSPEC
COPT = -Wall -O -g -ggdb -Dmain=SDL_main
LOPT = `sdl-config --libs` -lSDL_image -g
else
COPT = -c `sdl-config --cflags` -Wall -g
LOPT = `sdl-config --libs` -lSDL_image -lSDL_gfx -g
endif

gfxcfg: gfxcfg.o SFont.o ../clavier.o ../io.o
	gcc gfxcfg.o ../clavier.o ../io.o SFont.o -o gfxcfg $(LOPT)

gfxcfg.o: gfxcfg.c SFont.h
	gcc -c gfxcfg.c -o gfxcfg.o $(COPT)

../clavier.o:  ../clavier.c
	gcc -c ../clavier.c -o ../clavier.o $(COPT)

../io.o:  ../io.c
	gcc -c ../io.c -o ../io.o $(COPT)
 
SFont.o: SFont.h SFont.c
	gcc -c SFont.c -o SFont.o $(COPT)
