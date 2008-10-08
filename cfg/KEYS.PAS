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
var
    Touche:word;

procedure Get_Key; assembler;
asm

    mov  Touche,0


    mov  ah,11h
    int  16h



    jz   @Get_input_Pas_de_touche


    mov  ah,10h
    int  16h


    mov  byte ptr[Touche],ah


    mov  ah,02h
    int  16h


    and  al,00001111b


    mov  ah,al
    and  ah,00000001b
    shr  al,1
    or   al,ah


    mov  byte ptr[Touche+1],al

    @Get_input_Pas_de_touche:
end;

const HEXA_ORDER:string='0123456789ABCDEF';
function Byte2Hexa(Valeur:byte):string;
var Temp:string;
begin
  Temp:='';
  while length(Temp)<2 do
  begin
    Temp:=HEXA_ORDER[(Valeur mod 16)+1]+Temp;
    Valeur:=Valeur div 16;
  end;
  Byte2Hexa:=Temp;
end;

BEGIN
  repeat
    Get_Key;
    if (Touche<>0)
    then write('(',hi(Touche),',',Byte2Hexa(lo(Touche)),') ');
  until Touche=$0001;
END.
