/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

///@file c64load.h

#ifndef C64LOAD_H_INCLUDED
#define C64LOAD_H_INCLUDED

#define C64_VICMODE_MULTI  0x10
#define C64_VICMODE_TEXT   0x00
#define C64_VICMODE_BITMAP 0x20
#define C64_VICMODE_EXTBKG 0x40
#define C64_VICMODE_FLI    0x80

struct c64state {
  void * cpu;
  byte * ram;
  word screen;
  word bitmap;
  word keyjoyread;
  word irqrasterline;
  byte fliscreens[8];
  byte backgrounds[200];
  byte vicmode;
  byte ioaccess[256];
};

word C64_isBinaryProgram(FILE * f);
int C64_LoadPrg(struct c64state * c64, FILE * file, word start);

#endif

