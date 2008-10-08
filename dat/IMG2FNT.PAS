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
program IMG2FNT;

const
  COULEUR_TEXTE=15;

var
  Fichier:file;
  Image:array[0..20479] of byte;
  Fonte:array[0..2047] of byte;
  Offset_dans_image:longint;
  Masque:byte;
  Position:word;
  i,j,k,l:word;
  Errcode:integer;

begin
  if (paramcount<2) or (paramcount>3) then
  begin
    writeln('Syntaxe: IMG2FNT <Image> <Fonte> [offset dans l''image]');
    writeln('Attention: ne pas taper les extensions!');
    halt(1);
  end;

  if paramcount=3 then
    begin
      val(paramstr(3),Offset_dans_image,Errcode);
      if Errcode<>0 then
      begin
        writeln('Syntaxe: IMG2FNT <Image> <Fonte> [offset dans l''image]');
        writeln('Attention: ne pas taper les extensions!');
        halt(1);
      end;
    end
  else
    Offset_dans_image:=0;

  assign(Fichier,paramstr(1)+'.img');
  reset(Fichier,1);
  seek(Fichier,896+Offset_dans_image);
  blockread(Fichier,Image,17280);
  close(Fichier);

  {Transformation de l'image en 0 pour le fond et 255 pour le texte}
  for i:=0 to 17279 do
    if Image[i]<>COULEUR_TEXTE
      then Image[i]:=0
      else Image[i]:=255;

  for i:=0 to 255 do
    for j:=0 to 7 do
    begin
      Fonte[(i shl 3)+j]:=0;
      Masque:=128;
      for k:=0 to 7 do
      begin
        asm {Position:=((i DIV 45)*2880) + ((i MOD 45)*8) + (j*360) +k}
          mov  ax,i
          mov  bl,45
          div  bl
          push ax
          xor  ah,ah
          mov  bx,2880
          mul  bx
          mov  dx,ax
          pop  ax
          mov  cl,8
          shr  ax,cl
          mov  cl,3
          shl  ax,cl
          add  dx,ax
          add  dx,k
          mov  Position,dx
          mov  ax,j
          mov  bx,360
          mul  bx
          add  Position,ax
        end;
        Fonte[(i shl 3)+j]:=Fonte[(i shl 3)+j] + (Masque and Image[Position]);
        Masque:=Masque shr 1;
      end;
    end;

  assign(Fichier,Paramstr(2)+'.fnt');
  rewrite(Fichier,1);
  blockwrite(Fichier,Fonte,2048);
  close(Fichier);
end.

