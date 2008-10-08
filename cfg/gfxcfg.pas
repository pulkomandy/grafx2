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
program GrafX2_Setup;

{============================================================================}
const
  TAILLE_FICHIER_CONFIG=10351;
  TAILLE_INI           =16203;
  NB_MODES_VIDEO       =60;

  VERSION1             =2;
  VERSION2             =0;
  BETA1                =96;
  BETA2                =5;

  COULEUR_TITRE        =$5D;
  COULEUR_SETUP        =$1B;
  COULEUR_SELECT       =$3F;
  COULEUR_EXPLIC       =$2A;
  COULEUR_MESSAGE      =$3F;
  HAUTEUR_DEBUT_SETUP  =10;
  HAUTEUR_FIN_SETUP    =44;
  TAILLE_SETUP         =(HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)*40;

{$I SCANCODE.PAS}
{$I VIDEOMOD.PAS}

{============================================================================}

{------------------------ Passer en mode texte 80x50 ------------------------}
procedure Text_mode_80x50; assembler;
asm
  mov  ax,3
  int  10h
  mov  ax,1112h
  xor  bl,bl
  mov  dl,50
  int  10h
end;

{------------------------ Passer en mode texte 80x25 ------------------------}
procedure Text_mode_80x25; assembler;
asm
  mov  ax,3
  int  10h
end;

{---------------------------- Gestion du curseur ----------------------------}
procedure Montrer_Curseur; assembler;
asm
  mov ax,0103h
  mov cx,0E0Fh
  int 10h
end;

procedure Cacher_Curseur; assembler;
asm
  mov ax,0103h
  mov cx,0F0Eh
  int 10h
end;

procedure Goto_XY(X,Y:byte); assembler;
asm
    mov ah,2
    xor bh,bh
    mov dl,X
    mov dh,Y
    int 10h
end;

{---------------------- Change la couleur d'une ligne -----------------------}
procedure Set_color_of_line(Num_ligne:word; X1,X2,Couleur:byte); assembler;
asm
  mov  ax,0B800h
  mov  es,ax
  mov  ax,Num_ligne
  mov  bx,80
  mul  bl
  mov  bl,X1
  add  ax,bx
  add  ax,ax
  mov  di,ax

  mov  cl,X2
  sub  cl,X1
  inc  cl
  mov  al,Couleur
  @Set_col_loop:
    inc  di
    stosb
  dec  cl
  jnz  @Set_col_loop
end;

{------------------------ Passer en mode texte 80x25 ------------------------}
procedure Dessiner_ecran_principal;
var
    i:byte;
begin
  Cadre( 0, 0,79, 6,COULEUR_TITRE);
  Print( 2, 1,'Setup program for GRAFX 2.00 (c)1996-98 Sunset Design (G.DORME & K.MARITAUD)',COULEUR_TITRE);
  Print( 0, 2,'Ç',COULEUR_TITRE);
  Print(79, 2,'¶',COULEUR_TITRE);
  for i:=1 to 78 do
    Print(i,2,'Ä',COULEUR_TITRE);
  Print( 2, 3,'Use Up/Down arrows & Page-Up/Page-Down to scroll, Enter to modify, Delete to',COULEUR_TITRE);
  Print( 2, 4,'remove a hot-key, and Escape to validate or cancel. (Warning: QWERTY keys!)',COULEUR_TITRE);
  Print( 2, 5,'DO NOT USE Print-screen, Pause, and all other special keys!',COULEUR_TITRE);
  Cadre( 0, 7,79, 9,COULEUR_SETUP);
  Print( 2, 8,'Option',COULEUR_SETUP);
  Print(38, 8,'³ Hot-key',COULEUR_SETUP);
  Print(75, 8,'³Err',COULEUR_SETUP);
  Cadre( 0, 9,79,45,COULEUR_SETUP);
  Print( 0, 9,'Ì',COULEUR_SETUP);
  Print(79, 9,'¹',COULEUR_SETUP);
  Print(38, 7,'Ñ',COULEUR_SETUP);
  Print(75, 7,'Ñ',COULEUR_SETUP);
  Print(38, 9,'Ø',COULEUR_SETUP);
  Print(75, 9,'Ø',COULEUR_SETUP);
  Print(38,45,'Ï',COULEUR_SETUP);
  Print(75,45,'Ï',COULEUR_SETUP);
  Cadre( 0,46,79,49,COULEUR_EXPLIC);
end;

{--------------- Transformer une chaŒne AZT en String Pascal ----------------}
function Pchar2string(Chaine:pchar):string;
var
    Temp:string;
    i:byte;
begin
  Temp:='';
  i:=0;
  while Chaine[i]>#0 do
  begin
    Temp:=Temp+Chaine[i];
    inc(i);
  end;
  Pchar2string:=Temp;
end;

{------------------- ComplŠte une chaine avec un caractŠre ------------------}
function Completer_chaine(Chaine:string; Taille:byte; Caractere:char):string;
var
    i:byte;
begin
  for i:=length(Chaine)+1 to Taille do
    Chaine[i]:=Caractere;
  Chaine[0]:=chr(Taille);
  Completer_chaine:=Chaine;
end;

{--------------------- Test de duplication des touches ----------------------}
procedure Test_duplic;
var
    i,j:word;
    Pas_encore_erreur:boolean;
begin
  for i:=1 to NB_OPTIONS do
    if (Config[i].Touche<>$00FF) then
    begin
      j:=1;
      Pas_encore_erreur:=true;
      while (j<=NB_OPTIONS) and (Pas_encore_erreur) do
      begin
        if (i<>j) and (Config[i].Touche=Config[j].Touche) then
        begin
          Pas_encore_erreur:=false;
          Config[i].Erreur:=true;
        end;
        inc(j);
      end;
      if Pas_encore_erreur then Config[i].Erreur:=false;
    end;
end;

{------------------- S‚lectionner une touche … modifier ---------------------}
procedure Select(Decalage_curseur, Position_curseur:word);
var
    Touche,Num_ligne,Num_option,Num_table:word;
begin
  Num_ligne:=Position_curseur+HAUTEUR_DEBUT_SETUP-1;
  Num_option:=Position_curseur+Decalage_curseur;
  Config[Num_option].Touche:=$00FF;
  Ecrire(Num_ligne,Num_option,COULEUR_SELECT);
  Goto_XY(40,Num_ligne);
  Montrer_curseur;

  repeat
    Touche:=Lire_touche;
  until Touche<>$0000;
  Config[Num_option].Touche:=Touche;

  Cacher_curseur;
  Test_duplic;

  Num_table:=1;
  if (hi(Touche) and 1)>0
  then Num_table:=2;
  if (hi(Touche) and 2)>0
  then Num_table:=3;
  if (hi(Touche) and 4)>0
  then Num_table:=4;
  case Num_table of
    1: if Pchar2string(Table_Normal[lo(Touche)])='???'
       then Config[Num_option].Erreur:=true;
    2: if Pchar2string(Table_Shift[lo(Touche)])='???'
       then Config[Num_option].Erreur:=true;
    3: if Pchar2string(Table_Ctrl[lo(Touche)])='???'
       then Config[Num_option].Erreur:=true;
    4: if Pchar2string(Table_Alt[lo(Touche)])='???'
       then Config[Num_option].Erreur:=true;
  end;

  Tout_ecrire(Decalage_curseur,Position_curseur);
end;

{------------------------ D‚s‚lectionner une touche -------------------------}
procedure Unselect(Decalage_curseur, Position_curseur:word);
var
    Num_option:word;
begin
  Num_option:=Decalage_curseur+Position_curseur;
  Config[Num_option].Erreur:=false;
  Config[Num_option].Touche:=$00FF;
  Test_duplic;
  if not Config[Num_option].Suppr
  then Config[Num_option].Erreur:=true;
  Tout_ecrire(Decalage_curseur,Position_curseur);
end;

{---------------------- Validation de la configuration ----------------------}
function Validation:boolean;
var
    Y_a_des_erreurs:boolean;
    i:word;
begin
  Y_a_des_erreurs:=false;
  i:=1;
  while (i<=NB_OPTIONS) and not Y_a_des_erreurs do
  begin
    Y_a_des_erreurs:=Config[i].Erreur;
    inc(i);
  end;

  if Y_a_des_erreurs
  then Choix_enreg:=4-Fenetre_choix(30,12,
       'There are errors in the^hot-keys configuration.^Check out the "Err" column^in order to correct them.^',
       'OK^Quit anyway^',1,$4C,$3F)
  else Choix_enreg:=Fenetre_choix(30,10,'Save configuration?^','Yes^No^Cancel^',1,$2A,$6F);
  Validation:=(Choix_enreg<>3);
end;

{------------------------ Configuration des touches -------------------------}
procedure Setup;
var
    Sortie_OK:boolean;
    Touche:word;
    Decalage_curseur:word;
    Position_curseur:word;
begin
  Sortie_OK:=false;
  Decalage_curseur:=0;
  Position_curseur:=1;
  Test_duplic;
  Tout_ecrire(0,1);
  repeat
    Touche:=Lire_Touche;
    case Touche of
      $0048 : Scroll_haut(Decalage_curseur,Position_curseur);
      $0050 : Scroll_bas(Decalage_curseur,Position_curseur);
      $0049 : Page_up(Decalage_curseur,Position_curseur);
      $0051 : Page_down(Decalage_curseur,Position_curseur);
      $001C : Select(Decalage_curseur,Position_curseur);
      $0053 : Unselect(Decalage_curseur,Position_curseur);
      $0001 : Sortie_OK:=Validation;
    end;
  until Sortie_OK;
end;



{--- Lecture (et mise … jour s'il est ancien) du fichier de configuration ---}
type
  Type_shade=record
    Table:array[1..512] of word;
    Pas:byte;
    Mode:byte;
  end;
  Type_mode_video=record
    Etat   :byte;
    Largeur:word;
    Hauteur:word;
  end;{
  Type_header=record
    Signature:array[1..3] of char;
    Version1:byte;
    Version2:byte;
    Beta1:byte;
    Beta2:byte;
  end;
  Type_chunk=record
    Numero:byte;
    Taille:word;
  end;}
  Type_infos_touche=record
    Numero : word;
    Touche : word;
    Touche2: word;
  end;

var
  Modes_video   :array[1..NB_MODES_VIDEO] of Type_mode_video;
  Shade_actuel  :byte;
  Shades        :array[1..8] of Type_shade;
  Masque        :array[1..256] of byte;
  Stencil       :array[1..256] of byte;
  Infos_degrades:array[1..225] of byte;
  Smooth_Matrice:array[1..3,1..3] of byte;
  Exclude_color :array[1..256] of byte;
  Quick_shade   :array[1..2] of byte;

procedure Interpretation_du_fichier_config;
var
  Fichier       :file;
  Indice        :word;
  Indice2       :word;
  Taille_fichier:longint;
  Header        :Type_header;
  Chunk         :Type_chunk;
  Infos_touche  :Type_infos_touche;
  Mode_video    :Type_mode_video;
  OK            :boolean;
begin
  {- Modes vid‚os -}
  for Indice:=1 to NB_MODES_VIDEO do
  begin
    Modes_video[Indice].Etat   :=Table_modes_video[Indice,1];
    Modes_video[Indice].Largeur:=Table_modes_video[Indice,2];
    Modes_video[Indice].Hauteur:=Table_modes_video[Indice,3];
  end;
  {- Shade -}
  Shade_actuel:=0; {1er shade}
  for Indice:=1 to 8 do
  begin
    for Indice2:=1 to 512 do
      Shades[Indice].Table[Indice2]:=256; {Case vide pas tagg‚e}
    Shades[Indice].Pas:=1;                {Pas de 1}
    Shades[Indice].Mode:=0;               {Mode Normal}
  end;
  {- Masque -}
  fillchar(Masque,sizeof(Masque),0);
  {- Stencil -}
  fillchar(Stencil,sizeof(Stencil),0);
  {- Infos sur les d‚grad‚s -}
  fillchar(Infos_degrades,sizeof(Infos_degrades),0);
  {- Matrice du Smooth -}
  Smooth_matrice[1][1]:=1;
  Smooth_matrice[2][1]:=2;
  Smooth_matrice[3][1]:=1;
  Smooth_matrice[1][2]:=2;
  Smooth_matrice[2][2]:=4;
  Smooth_matrice[3][2]:=2;
  Smooth_matrice[1][3]:=1;
  Smooth_matrice[2][3]:=2;
  Smooth_matrice[3][3]:=1;
  {- Exclude colors -}
  fillchar(Exclude_color,sizeof(Exclude_color),0);
  {- Quick-shade -}
  Quick_shade[1]:=1; {Step}
  Quick_shade[2]:=0; {Loop mode (0=normal,1=loop,2=no sat.}

  Taille_fichier:=0;
  assign(Fichier,'GFX2.CFG');
  {$I-}
  reset(Fichier,1);
  {$I+}
  if (IOresult=0) then
  begin {On tente de r‚cup‚rer les infos du fichier}
    Taille_fichier:=filesize(Fichier);
    if (Taille_fichier>=7) then
    begin
      blockread(Fichier,Header,7);
      if Header.Signature='CFG' then
      begin
        {On regarde si c'est un fichier de la version actuelle}
        if (Taille_fichier =TAILLE_FICHIER_CONFIG) and
           (Header.Version1=VERSION1             ) and
           (Header.Version2=VERSION2             ) and
           (Header.Beta1   =BETA1                ) and
           (Header.Beta2   =BETA2                ) then
        begin {Si oui, on r‚cupŠre les touches et on s'arrˆte l…}
          seek(Fichier,sizeof(Header)+Sizeof(Chunk));
          for Indice:=1 to NB_OPTIONS do
            blockread(Fichier,Config[Indice],6);
          close(Fichier);
          exit;
        end
        else
        begin  {Si non, c'est une ancienne version et on r‚cupŠre tout ce qu'on peut}
          OK:=TRUE;
          {$I-}
          while OK(*not eof(Fichier)*) do
          begin
            blockread(Fichier,Chunk,sizeof(Chunk));
            if (IOresult<>0) then
              OK:=FALSE
            else
              case Chunk.Numero of
                0:begin {Touches}
                    for Indice:=1 to (Chunk.Taille div sizeof(Infos_touche)) do
                    begin
                      blockread(Fichier,Infos_touche,sizeof(Infos_touche));
                      if (IOresult<>0) then
                        OK:=FALSE
                      else
                      begin
                        Indice2:=1;
                        while (Indice2<=NB_OPTIONS) and
                              (Config[Indice2].Numero<>Infos_touche.Numero) do
                          inc(Indice2);
                        if (Indice2<=NB_OPTIONS) then
                        begin
                          Config[Indice2].Touche :=Infos_touche.Touche;
                          Config[Indice2].Touche2:=Infos_touche.Touche2;
                        end;
                      end;
                    end;
                  end;
                1:begin {Modes vid‚o}
                    for Indice:=1 to (Chunk.Taille div sizeof(Mode_video)) do
                    begin
                      blockread(Fichier,Mode_video,sizeof(Mode_video));
                      if (IOresult<>0) then
                        OK:=FALSE
                      else
                      begin
                        Indice2:=1;
                        while (Indice2<=NB_MODES_VIDEO) and
                              ( (Modes_video[Indice2].Largeur<>Mode_video.Largeur) or
                                (Modes_video[Indice2].Hauteur<>Mode_video.Hauteur) or
                                ((Modes_video[Indice2].Etat and $C0)<>(Mode_video.Etat and $C0)) ) do
                          inc(Indice2);
                        if (Indice2<=NB_MODES_VIDEO) then
                          Modes_video[Indice2].Etat:=Mode_video.Etat;
                      end;
                    end;
                  end;
                2:begin {Shade}
                    blockread(Fichier,Shade_actuel,sizeof(Shade_actuel));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet un shade actuel propre}
                      Shade_actuel:=0;
                    end
                    else
                    begin
                      blockread(Fichier,Shades,sizeof(Shades));
                      if (IOresult<>0) then
                      begin
                        OK:=FALSE; {Erreur => On remet des tables de shades propres}
                        for Indice:=1 to 8 do
                        begin
                          for Indice2:=1 to 512 do
                            Shades[Indice].Table[Indice2]:=256; {Case vide pas tagg‚e}
                          Shades[Indice].Pas:=1;                {Pas de 1}
                          Shades[Indice].Mode:=0;               {Mode Normal}
                        end;
                      end;
                    end;
                  end;
                3:begin {Masque}
                    blockread(Fichier,Masque,sizeof(Masque));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet un masque propre}
                      fillchar(Masque,sizeof(Masque),0);
                    end;
                  end;
                4:begin {Stencil}
                    blockread(Fichier,Stencil,sizeof(Stencil));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet un stencil propre}
                      fillchar(Stencil,sizeof(Stencil),0);
                    end;
                  end;
                5:begin {Infos sur les d‚grad‚s}
                    blockread(Fichier,Infos_degrades,sizeof(Infos_degrades));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet des infos sur les d‚grad‚s propres}
                      fillchar(Stencil,sizeof(Infos_degrades),0);
                    end;
                  end;
                6:begin {Matrice du Smooth}
                    blockread(Fichier,Smooth_Matrice,sizeof(Smooth_Matrice));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet une matrice de Smooth propre}
                      Smooth_matrice[1][1]:=1;
                      Smooth_matrice[2][1]:=2;
                      Smooth_matrice[3][1]:=1;
                      Smooth_matrice[1][2]:=2;
                      Smooth_matrice[2][2]:=4;
                      Smooth_matrice[3][2]:=2;
                      Smooth_matrice[1][3]:=1;
                      Smooth_matrice[2][3]:=2;
                      Smooth_matrice[3][3]:=1;
                    end;
                  end;
                7:begin {Exclude colors}
                    blockread(Fichier,Exclude_color,sizeof(Exclude_color));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet une table d'exclusion propre}
                      fillchar(Exclude_color,sizeof(Exclude_color),0);
                    end;
                  end;
                8:begin {Quick-shade}
                    blockread(Fichier,Quick_shade,sizeof(Quick_shade));
                    if (IOresult<>0) then
                    begin
                      OK:=FALSE; {Erreur => On remet un quick-shade propre}
                      Quick_shade[1]:=1; {Step}
                      Quick_shade[2]:=0; {Loop mode}
                    end;
                  end
                else
                begin
                  seek(Fichier,filepos(Fichier)+Chunk.Taille);
                  if (IOresult<>0) then
                    OK:=FALSE;
                end;
              end;
          end;
          {$I+}
        end;
      end;
    end;
  end;

  {- Ecriture du fichier d'options -}
  assign(Fichier,'GFX2.CFG');
  rewrite(Fichier,1);

  {Ecriture du header}
  Header.Signature:='CFG';
  Header.Version1 :=VERSION1;
  Header.Version2 :=VERSION2;
  Header.Beta1    :=BETA1;
  Header.Beta2    :=BETA2;
  blockwrite(Fichier,Header,sizeof(Header));

  {Enregistrement des touches}
  Chunk.Numero:=0;
  Chunk.Taille:=NB_OPTIONS*sizeof(Infos_touche);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  for Indice:=1 to NB_OPTIONS do
    blockwrite(Fichier,Config[Indice],sizeof(Infos_touche)); {Les 3 premiers champs (words) sont: Num‚ro,Touche,Touche2}

  {Sauvegarde de l'‚tat de chaque mode vid‚o}
  Chunk.Numero:=1;
  Chunk.Taille:=sizeof(Modes_video);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Modes_video,sizeof(Modes_video));

  {Ecriture des donn‚es du Shade (pr‚c‚d‚es du shade en cours)}
  Chunk.Numero:=2;
  Chunk.Taille:=sizeof(Shades)+Sizeof(Shade_actuel);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Shade_actuel,Sizeof(Shade_actuel));
  blockwrite(Fichier,Shades,Sizeof(Shades));

  {Ecriture des donn‚es du Masque}
  Chunk.Numero:=3;
  Chunk.Taille:=sizeof(Masque);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Masque,Sizeof(Masque));

  {Ecriture des donn‚es du Stencil}
  Chunk.Numero:=4;
  Chunk.Taille:=sizeof(Stencil);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Stencil,Sizeof(Stencil));

  {Ecriture des donn‚es sur les d‚grad‚s}
  Chunk.Numero:=5;
  Chunk.Taille:=sizeof(Infos_degrades);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Infos_degrades,Sizeof(Infos_degrades));

  {Ecriture de la matrice de Smooth}
  Chunk.Numero:=6;
  Chunk.Taille:=sizeof(Smooth_Matrice);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Smooth_Matrice,Sizeof(Smooth_Matrice));

  {Ecriture de la table d'exclusion de couleurs}
  Chunk.Numero:=7;
  Chunk.Taille:=sizeof(Exclude_color);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Exclude_color,Sizeof(Exclude_color));

  {Ecriture des donn‚es du Quick-shade}
  Chunk.Numero:=8;
  Chunk.Taille:=sizeof(Quick_shade);
  blockwrite(Fichier,Chunk,sizeof(Chunk));
  blockwrite(Fichier,Quick_shade,Sizeof(Quick_shade));

  close(Fichier);
end;

function Recreer_INI:boolean;
type
  T_Buffer=array[1..TAILLE_INI] of byte;
var
  Fichier_INI,Fichier_DAT:file;
  Buffer:pointer;{^T_Buffer;}
  Erreur:boolean;
begin
  assign(Fichier_INI,'GFX2.INI');

  assign(Fichier_DAT,'GFX2.DAT');
  {$I-}
  reset(Fichier_DAT,1);
  if IOresult<>0 then
    Erreur:=true
  else
  begin
    seek(Fichier_DAT,filesize(Fichier_DAT)-TAILLE_INI);
    if IOresult<>0 then
      Erreur:=true
    else
    begin
      Erreur:=false;
      rewrite(Fichier_INI,1);
      getmem(Buffer,TAILLE_INI);
      blockread (Fichier_DAT,Buffer^,TAILLE_INI);
      blockwrite(Fichier_INI,Buffer^,TAILLE_INI);
      freemem(Buffer,TAILLE_INI);
      close(Fichier_INI);
    end;
    close(Fichier_DAT);
  end;
  {$I+}

  Recreer_INI:=Erreur;
end;
