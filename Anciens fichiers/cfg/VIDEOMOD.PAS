{ Grafx2 - The Ultimate 256-color bitmap paint program

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
}
const
  MODE_MCGA =$00;
  MODE_X    =$40;
  MODE_VESA =$80;
  MODE_XVESA=$C0;

  Table_modes_video:array[1..NB_MODES_VIDEO,1..3] of word=
  (
    (MODE_MCGA    ,320,200),
    (MODE_X+1     ,320,224),
    (MODE_X+1     ,320,240),
    (MODE_X+1     ,320,256),
    (MODE_X+1     ,320,270),
    (MODE_X+1     ,320,282),
    (MODE_X+1     ,320,300),
    (MODE_X+1     ,320,360),
    (MODE_X+1     ,320,400),
    (MODE_X+1     ,320,448),
    (MODE_X+1     ,320,480),
    (MODE_X+1     ,320,512),
    (MODE_X+1     ,320,540),
    (MODE_X+1     ,320,564),
    (MODE_X+1     ,320,600),

    (MODE_X+1     ,360,200),
    (MODE_X+1     ,360,224),
    (MODE_X+1     ,360,240),
    (MODE_X+1     ,360,256),
    (MODE_X+1     ,360,270),
    (MODE_X+1     ,360,282),
    (MODE_X+1     ,360,300),
    (MODE_X+1     ,360,360),
    (MODE_X+1     ,360,400),
    (MODE_X+1     ,360,448),
    (MODE_X+1     ,360,480),
    (MODE_X+1     ,360,512),
    (MODE_X+1     ,360,540),
    (MODE_X+1     ,360,564),
    (MODE_X+1     ,360,600),

    (MODE_X+1     ,400,200),
    (MODE_X+1     ,400,224),
    (MODE_X+1     ,400,240),
    (MODE_X+1     ,400,256),
    (MODE_X+1     ,400,270),
    (MODE_X+1     ,400,282),
    (MODE_X+1     ,400,300),
    (MODE_X+1     ,400,360),
    (MODE_X+1     ,400,400),
    (MODE_X+1     ,400,448),
    (MODE_X+1     ,400,480),
    (MODE_X+1     ,400,512),
    (MODE_X+1     ,400,540),
    (MODE_X+1     ,400,564),
    (MODE_X+1     ,400,600),

    (MODE_XVESA+1 ,640,224),
    (MODE_XVESA+1 ,640,240),
    (MODE_XVESA+1 ,640,256),
    (MODE_XVESA+1 ,640,270),
    (MODE_XVESA+1 ,640,300),
    (MODE_XVESA+1 ,640,350),
    (MODE_VESA+1  ,640,400),
    (MODE_XVESA+1 ,640,448),
    (MODE_VESA+1  ,640,480),
    (MODE_XVESA+1 ,640,512),
    (MODE_XVESA+1 ,640,540),
    (MODE_XVESA+1 ,640,564),
    (MODE_XVESA+1 ,640,600),
    (MODE_VESA+1  ,800,600),
    (MODE_VESA+1 ,1024,768)
  );
