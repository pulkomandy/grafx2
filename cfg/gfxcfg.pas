program GrafX2_Setup;

{============================================================================}
const
  TAILLE_FICHIER_CONFIG=10351;
  TAILLE_INI           =16203;
  NB_MAX_OPTIONS       =134;
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

type
  Options=record
            Numero : word;
            Touche : word;
            Touche2: word;
            Libelle: string[35];
            Explic1: string[76];
            Explic2: string[76];
            Erreur : boolean;
            Suppr  : boolean;
          end;

var
  Config:array [1..NB_MAX_OPTIONS] of Options;
  NB_OPTIONS:word;
  Numero_definition_option:word;
  Choix_enreg:byte;



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

{------- Ecrire une chaine d'une certaine couleur … un certain endroit ------}
procedure Print(X,Y:word; Chaine:string; Attrib:byte); assembler;
asm
  mov  ax,0B800h
  mov  es,ax
  mov  ax,Y
  mov  bl,80
  mul  bl
  mov  bx,X
  add  ax,bx
  shl  ax,1
  mov  di,ax

  push ds
  mov  si,word ptr [Chaine]
  mov  ds,word ptr [Chaine+2]

  mov  cl,[ds:si]
  inc  si
  mov  al,Attrib
  @Ecrire_loop:
    movsb
    stosb
  dec  cl
  jnz  @Ecrire_loop
  pop  ds
end;

{-------------- Dessiner un cadre plein d'une certaine couleur --------------}
procedure Cadre(X1,Y1,X2,Y2,Couleur:byte);
var
    Tampon:string;
    i,j:byte;
begin
  Tampon[0]:=chr(X2-X1+1);
  for j:=Y1 to Y2 do
  begin
    if j=Y1 then
    begin
      Tampon[1]:='É';
      Tampon[X2-X1+1]:='»';
      for i:=X1+1 to X2-1 do Tampon[i-X1+1]:='Í';
    end
    else
    if j=Y2 then
    begin
      Tampon[1]:='È';
      Tampon[X2-X1+1]:='¼';
      for i:=X1+1 to X2-1 do Tampon[i-X1+1]:='Í';
    end
    else
    begin
      Tampon[1]:='º';
      Tampon[X2-X1+1]:='º';
      for i:=X1+1 to X2-1 do Tampon[i-X1+1]:=' ';
    end;

    Print(X1,j,Tampon,Couleur);
  end;
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

{--------------------------- Lecture d'une touche ---------------------------}
function Lire_touche:word; assembler;
var
    Touche:word;
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
  mov  ax,Touche
end;

{--------------- Dessiner une jolie fenˆtre au premier plan -----------------}
function Fenetre_choix(Largeur,Hauteur:byte; Titre,Choix:string; Choix_debut,Couleur,Couleur_choix:byte):byte;
var
    Temp:string;
    i,Num_titre,Num_choix:byte;
    X1,X2,Y1,Y2:byte;
    Option_choisie:byte;
    Touche:word;
    Memoire_video:array[0..7999] of byte;
begin
  move(mem[$B800:0000],Memoire_video,8000);

  X1:=40-(Largeur shr 1);
  Y1:=25-(Hauteur shr 1);
  X2:=X1+Largeur-1;
  Y2:=Y1+Hauteur-1;

  Cadre(X1,Y1,X2,Y2,Couleur);
  if Y2<49 then Set_color_of_line(Y2+1,X1+1,X2+1,$08);
  if X2<79 then for i:=Y1+1 to Y2 do Set_color_of_line(i,X2+1,X2+1,$08);


  Num_titre:=1;
  Temp:='';
  for i:=1 to length(Titre) do
  begin
    if Titre[i]<>'^'
    then Temp:=Temp+Titre[i]
    else
    begin
      Print(X1+(X2-X1+1-length(Temp)) shr 1,Y1+1+Num_titre,Temp,Couleur);
      Temp:='';
      inc(Num_titre);
    end;
  end;

  Num_choix:=1;
  Temp:='';
  for i:=1 to length(Choix) do
  begin
    if Choix[i]<>'^'
    then Temp:=Temp+Choix[i]
    else
    begin
      Print(X1+(X2-X1+1-length(Temp)) shr 1,Y1+2+Num_choix+Num_titre,Temp,Couleur);
      Temp:='';
      inc(Num_choix);
    end;
  end;
  dec(Num_choix);

  Option_choisie:=Choix_debut;
  Set_color_of_line(Y1+2+Num_titre+Option_choisie,X1+2,X2-2,Couleur_choix);
  Repeat
    Touche:=Lire_touche;
    case Touche of
      $0048 : begin
                Set_color_of_line(Y1+2+Num_titre+Option_choisie,X1+2,X2-2,Couleur);
                if Option_choisie=1
                then Option_choisie:=Num_choix
                else dec(Option_choisie);
                Set_color_of_line(Y1+2+Num_titre+Option_choisie,X1+2,X2-2,Couleur_choix);
              end;
      $0050 : begin
                Set_color_of_line(Y1+2+Num_titre+Option_choisie,X1+2,X2-2,Couleur);
                if Option_choisie=Num_choix
                then Option_choisie:=1
                else inc(Option_choisie);
                Set_color_of_line(Y1+2+Num_titre+Option_choisie,X1+2,X2-2,Couleur_choix);
              end;
    end;
  until Touche=$001C;

  move(Memoire_video,mem[$B800:0000],8000);

  Fenetre_choix:=Option_choisie;
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

{------------------------- D‚finition d'une option --------------------------}
procedure Definir_option(Numero:word;Libelle,Explic1,Explic2:string; Supprimable:boolean; Touche_par_defaut:word);
begin
  Config[Numero_definition_option].Numero :=Numero;
  Config[Numero_definition_option].Touche :=Touche_par_defaut;
  Config[Numero_definition_option].Touche2:=$00FF;
  Config[Numero_definition_option].Libelle:=Libelle;
  Config[Numero_definition_option].Explic1:=Explic1;
  Config[Numero_definition_option].Explic2:=Explic2;
  Config[Numero_definition_option].Erreur :=false;
  Config[Numero_definition_option].Suppr  :=Supprimable;
  inc(Numero_definition_option);
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

{------------- Renvoyer une chaŒne donnant une touche "en clair" ------------}
function Nom_touche(Touche:word):string;
var
    Temp,Temp2:string;
    Num_table:byte;
begin
  Num_table:=1;
  Temp:='';
  if (hi(Touche) and 1)>0 then
  begin
    Temp:=Temp+'<Shift> + ';
    Num_table:=2;
  end;
  if (hi(Touche) and 2)>0 then
  begin
    Temp:=Temp+'<Ctrl> + ';
    Num_table:=3;
  end;
  if (hi(Touche) and 4)>0 then
  begin
    Temp:=Temp+'<Alt> + ';
    Num_table:=4;
  end;
  case Num_table of
    1: begin
         Temp2:=Pchar2string(Table_Normal[lo(Touche)]);
         if Temp2='???'
         then Temp:='********** Invalid key! **********'
         else if Temp2=''
         then Temp:=''
         else Temp:=Temp+'<'+Temp2+'>';
       end;
    2: begin
         Temp2:=Pchar2string(Table_Shift[lo(Touche)]);
         if Temp2='???'
         then Temp:='**** Invalid key combination! ****'
         else if Temp2=''
         then Temp:=''
         else Temp:=Temp+'<'+Temp2+'>';
       end;
    3: begin
         Temp2:=Pchar2string(Table_Ctrl[lo(Touche)]);
         if Temp2='???'
         then Temp:='**** Invalid key combination! ****'
         else if Temp2=''
         then Temp:=''
         else Temp:=Temp+'<'+Temp2+'>';
       end;
    4: begin
         Temp2:=Pchar2string(Table_Alt[lo(Touche)]);
         if Temp2='???'
         then Temp:='**** Invalid key combination! ****'
         else if Temp2=''
         then Temp:=''
         else Temp:=Temp+'<'+Temp2+'>';
       end;
  end;
  Nom_touche:=Temp;
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

{------------------- Ecrire les informations sur une option -----------------}
procedure Ecrire(Ligne:byte; Num_option:word; Couleur:byte);
begin
  Print( 2,Ligne,Completer_chaine(Config[Num_option].Libelle,35,' '),Couleur);
  Print(40,Ligne,Completer_chaine(Nom_touche(Config[Num_option].Touche),34,' '),Couleur);
  if Config[Num_option].Erreur
  then Print(77,Ligne,'X',Couleur)
  else Print(77,Ligne,' ',Couleur);
end;

{-------------------- Ecrire un commentaire sur une option ------------------}
procedure Ecrire_commentaire(Num_option:word);
begin
  Print(2,47,Completer_chaine(Config[Num_option].Explic1,76,' '),COULEUR_EXPLIC);
  Print(2,48,Completer_chaine(Config[Num_option].Explic2,76,' '),COULEUR_EXPLIC);
end;

{----------------- Ecrire toutes les premieres lignes visibles --------------}
procedure Tout_ecrire(Decalage_curseur, Position_curseur:word);
var
    i:byte;
begin
  i:=HAUTEUR_DEBUT_SETUP;
  while (i<=HAUTEUR_FIN_SETUP) and (i<NB_OPTIONS+HAUTEUR_DEBUT_SETUP) do
  begin
    Ecrire(i,Decalage_curseur+i+1-HAUTEUR_DEBUT_SETUP,COULEUR_SETUP);
    inc(i);
  end;
  for i:=HAUTEUR_DEBUT_SETUP to HAUTEUR_FIN_SETUP do
  begin
    Print(38,i,'³',COULEUR_SETUP);
    Print(75,i,'³',COULEUR_SETUP);
  end;
  Set_color_of_line(HAUTEUR_DEBUT_SETUP+Position_curseur-1,1,78,COULEUR_SELECT);
  Ecrire_commentaire(Decalage_curseur+Position_curseur);
end;

{-------------- Faire scroller tout l'ecran de setup vers le bas ------------}
procedure Tout_scroller_bas(Decalage_curseur, Position_curseur:word);
begin
  asm
    mov  ax,0B800h
    mov  es,ax
    mov  ax,HAUTEUR_FIN_SETUP
    mov  bl,160
    mul  bl
    sub  ax,4
    mov  si,ax
    mov  di,si
    add  di,160

    push ds
    push es
    pop  ds

    mov  cx,TAILLE_SETUP
    std
    db   66h
    rep  movsw
    cld
    pop  ds
  end;
  Ecrire(HAUTEUR_DEBUT_SETUP,Decalage_curseur+Position_curseur,COULEUR_SELECT);
  Set_color_of_line(HAUTEUR_DEBUT_SETUP,1,78,COULEUR_SELECT);
end;

{------------- Faire scroller tout l'ecran de setup vers le haut ------------}
procedure Tout_scroller_haut(Decalage_curseur, Position_curseur:word);
begin
  asm
    mov  ax,0B800h
    mov  es,ax
    mov  ax,HAUTEUR_DEBUT_SETUP+1
    mov  bl,160
    mul  bl
    mov  di,ax
    mov  si,di
    sub  di,160

    push ds
    push es
    pop  ds

    mov  cx,TAILLE_SETUP
    db   66h
    rep  movsw
    pop  ds
  end;
  Ecrire(HAUTEUR_FIN_SETUP,Decalage_curseur+Position_curseur,COULEUR_SELECT);
  Set_color_of_line(HAUTEUR_FIN_SETUP,1,78,COULEUR_SELECT);
end;

{-------------------------- Scroller vers le haut ---------------------------}
procedure Scroll_haut(var Decalage_curseur, Position_curseur:word);
begin
  if Position_curseur+Decalage_Curseur>0 then
  begin
    if Position_curseur>1 then
    begin
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
      dec(Position_curseur);
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SELECT);
    end
    else
    begin
      if Decalage_curseur>0 then
      begin
        Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
        dec(Decalage_curseur);
        Tout_Scroller_bas(Decalage_curseur,Position_curseur);
      end;
    end;
    Ecrire_commentaire(Position_curseur+Decalage_curseur);
  end;
end;

{-------------------------- Scroller vers le bas ----------------------------}
procedure Scroll_bas(var Decalage_curseur, Position_curseur:word);
begin
  if Position_curseur+Decalage_Curseur<NB_OPTIONS then
  begin
    if Position_curseur<=(HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP) then
    begin
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
      inc(Position_curseur);
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SELECT);
    end
    else
    begin
      if Decalage_curseur+Position_curseur<NB_OPTIONS then
      begin
        Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
        inc(Decalage_curseur);
        Tout_Scroller_haut(Decalage_curseur,Position_curseur);
      end;
    end;
    Ecrire_commentaire(Position_curseur+Decalage_curseur);
  end;
end;

{-------------------------- Scroller vers le haut ---------------------------}
procedure Page_up(var Decalage_curseur, Position_curseur:word);
begin
  if Position_curseur+Decalage_Curseur>0 then
  begin
    if Position_curseur>1 then
    begin
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
      Position_curseur:=1;
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SELECT);
    end
    else
    begin
      if Decalage_curseur>0 then
      begin
        Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
        if Decalage_curseur>HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP
        then dec(Decalage_curseur,HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)
        else Decalage_curseur:=0;
        Tout_ecrire(Decalage_curseur,Position_curseur);
      end;
    end;
    Ecrire_commentaire(Position_curseur+Decalage_curseur);
  end;
end;

{-------------------------- Scroller vers le bas ----------------------------}
procedure Page_down(var Decalage_curseur, Position_curseur:word);
begin
  if Position_curseur+Decalage_Curseur<NB_OPTIONS then
  begin
    if Position_curseur<=(HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP) then
    begin
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
      Position_curseur:=HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP+1;
      Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SELECT);
    end
    else
    begin
      if Decalage_curseur+Position_curseur<NB_OPTIONS then
      begin
        Set_color_of_line(Position_Curseur+HAUTEUR_DEBUT_SETUP-1,1,78,COULEUR_SETUP);
        if (Decalage_curseur+Position_curseur+HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)<NB_OPTIONS
        then inc(Decalage_curseur,HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)
        else Decalage_curseur:=NB_OPTIONS-(HAUTEUR_FIN_SETUP-HAUTEUR_DEBUT_SETUP)-1;
        Tout_ecrire(Decalage_curseur,Position_curseur);
      end;
    end;
    Ecrire_commentaire(Position_curseur+Decalage_curseur);
  end;
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
  end;
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
  end;
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


{------------------- Initialisation de la configuration ---------------------}
function Initialiser_config:boolean;
var
  Erreur:boolean;
  Fichier_INI:file;
begin
  Erreur:=false;

  Numero_definition_option:=1;

  {!!! LES TOUCHES SONT CONFIGUREES POUR LES CLAVIERS QWERTY !!!}

  Definir_option(0,'Scroll up',
                 'Used to scroll upwards in the picture, either in magnify and normal mode.',
                 'This hot-key cannot be removed.',
                 false,$0048); {Haut}
  Definir_option(1,'Scroll down',
                 'Used to scroll downwards in the picture, either in magnify and normal mode.',
                 'This hot-key cannot be removed.',
                 false,$0050); {Bas}
  Definir_option(2,'Scroll left',
                 'Used to scroll to the left in the picture, either in magnify and normal',
                 'mode. This hot-key cannot be removed.',
                 false,$004B); {Gauche}
  Definir_option(3,'Scroll right',
                 'Used to scroll to the right in the picture, either in magnify and normal',
                 'mode. This hot-key cannot be removed.',
                 false,$004D); {Droite}
  Definir_option(4,'Faster scroll up',
                 'Used to scroll upwards in the picture fast, either in magnify and normal',
                 'mode.',
                 true,$0148);  {Shift + Haut}
  Definir_option(5,'Faster scroll down',
                 'Used to scroll downwards in the picture fast, either in magnify and normal',
                 'mode.',
                 true,$0150);  {Shift + Bas}
  Definir_option(6,'Faster scroll left',
                 'Used to scroll to the left in the picture fast, either in magnify and normal',
                 'mode.',
                 true,$014B);  {Shift + Gauche}
  Definir_option(7,'Faster scroll right',
                 'Used to scroll to the right in the picture fast, either in magnify and',
                 'normal mode.',
                 true,$014D);  {Shift + Droite}
  Definir_option(8,'Slower scroll up',
                 'Used to scroll upwards in the picture pixel by pixel, either in magnify and',
                 'normal mode.',
                 true,$0498);  {Alt + Haut}
  Definir_option(9,'Slower scroll down',
                 'Used to scroll downwards in the picture pixel by pixel, either in magnify and',
                 'normal mode.',
                 true,$04A0);  {Alt + Bas}
  Definir_option(10,'Slower scroll left',
                 'Used to scroll to the left in the picture pixel by pixel, either in magnify',
                 'and normal mode.',
                 true,$049B);  {Alt + Gauche}
  Definir_option(11,'Slower scroll right',
                 'Used to scroll to the right in the picture pixel by pixel, either in magnify',
                 'and normal mode.',
                 true,$049D);  {Alt + Droite}
  Definir_option(12,'Move mouse cursor 1 pixel up',
                 'Used to simulate a very small mouse deplacement upwards.',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$028D);  {Ctrl + Haut}
  Definir_option(13,'Move mouse cursor 1 pixel down',
                 'Used to simulate a very small mouse deplacement downwards.',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$0291);  {Ctrl + Bas}
  Definir_option(14,'Move mouse cursor 1 pixel left',
                 'Used to simulate a very small mouse deplacement to the left.',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$0273);  {Ctrl + Gauche}
  Definir_option(15,'Move mouse cursor 1 pixel right',
                 'Used to simulate a very small mouse deplacement to the right.',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$0274);  {Ctrl + Droite}
  Definir_option(16,'Simulate left mouse click',
                 'Used to simulate a click with the left mouse button..',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$0039);  {Space}
  Definir_option(17,'Simulate right mouse click',
                 'Used to simulate a click with the right mouse button..',
                 'It''s very useful when you want a ultra-high precision.',
                 true,$0139);  {Shift + Space}
  Definir_option(18,'Show/hide option menu',
                 'Switch the tool bar display on/off.',
                 'This hot-key cannot be removed.',
                 false,$0044); {F10}
  Definir_option(19,'Show/hide cursor',
                 'Switch the cursor display on/off.',
                 'This only works on the "small cross" and "hand" cursors.',
                 true,$0043);  {F9}
  Definir_option(20,'Set paintbrush to 1 pixel',
                 'Useful when you want to use a "single-pixel-brush".',
                 '',
                 true,$0053);  {Del}
  Definir_option(21,'Paintbrush choice',
                 'Opens a menu where you can choose a paintbrush out of 24 predefined ones.',
                 '',
                 true,$003E);  {F4}
  Definir_option(22,'Monochrome brush',
                 'Turn your current user-defined brush into a single colored one.',
                 'All non-transparent colors are set to current foreground color.',
                 true,$0157);  {Shift + F4}
  Definir_option(23,'Freehand drawing',
                 'Set the drawing mode to the classical freehand one.',
                 '',
                 true,$0020);  {D}
  Definir_option(24,'Switch freehand drawing mode',
                 'Switch between the 3 ways to use freehand drawing.',
                 'These modes are: continuous, discontinuous and point by point.',
                 true,$0120);  {Shift + D}
  Definir_option(25,'Continuous freehand drawing',
                 'Switch directly to continuous freehand drawing mode.',
                 '',
                 true,$0220);  {Ctrl + D}
  Definir_option(26,'Line',
                 'Allows you to draw lines.',
                 '',
                 true,$0026);  {L}
  Definir_option(27,'Knotted lines',
                 'Allows you to draw linked lines.',
                 'This mode can also be called "Polyline".',
                 true,$0126);  {Shift + L}
  Definir_option(28,'Spray',
                 'Allows you to spray brushes randomly in the picture.',
                 '',
                 true,$001E);  {A (Q en AZERTY)}
  Definir_option(29,'Spray menu',
                 'Opens a menu in which you can configure the spray flow and size.',
                 '',
                 true,$011E);  {Shift + A}
  Definir_option(30,'Flood-fill',
                 'Allows you to fill an area of the picture made of pixels of the same color.',
                 '',
                 true,$0021);  {F}
  Definir_option(124,'Replace color',
                 'Allows you to replace all the pixels of the color pointed by the mouse with',
                 'the fore-color or the back-color.',
                 true,$0121);  {Shift + F}
  Definir_option(31,'B‚zier''s curves',
                 'Allows you to draw B‚zier''s curves.',
                 '',
                 true,$0017);  {I}
  Definir_option(32,'B‚zier''s curve with 3 or 4 points',
                 'Allows you to choose whether you want to draw B‚zier''s curves with 3 or 4',
                 'points.',
                 true,$0117);  {Shift + I}
  Definir_option(33,'Empty rectangle',
                 'Allows you to draw a rectangle using the brush.',
                 '',
                 true,$0013);  {R}
  Definir_option(34,'Filled rectangle',
                 'Allows you to draw a filled rectangle.',
                 '',
                 true,$0113);  {Shift + R}
  Definir_option(35,'Empty circle',
                 'Allows you to draw a circle using the brush.',
                 '',
                 true,$002E);  {C}
  Definir_option(36,'Empty ellipse',
                 'Allows you to draw an ellipse using the brush.',
                 '',
                 true,$022E);  {Ctrl + C}
  Definir_option(37,'Filled circle',
                 'Allows you to draw a filled circle.',
                 '',
                 true,$012E);  {Shift + C}
  Definir_option(38,'Filled ellipse',
                 'Allows you to draw a filled ellipse.',
                 '',
                 true,$032E);  {Shift + Ctrl + C}
  Definir_option(39,'Empty polygon',
                 'Allows you to draw a polygon using the brush.',
                 '',
                 true,$0031);  {N}
  Definir_option(40,'Empty "polyform"',
                 'Allows you to draw a freehand polygon using the brush.',
                 '',
                 true,$0231);  {Ctrl + N}
  Definir_option(41,'Filled polygon',
                 'Allows you to draw a filled polygon.',
                 '',
                 true,$0131);  {Shift + N}
  Definir_option(42,'Filled "polyform"',
                 'Allows you to draw a filled freehand polygon.',
                 '',
                 true,$0331);  {Shift + Ctrl + N}
  Definir_option(43,'Rectangle with gradation',
                 'Allows you to draw a rectangle with a color gradation.',
                 '',
                 true,$0413);  {Alt + R}
  Definir_option(44,'Gradation menu',
                 'Allows you to configure the way color gradations are calculated.',
                 '',
                 true,$0422);  {Alt + G}
  Definir_option(45,'Sphere with gradation',
                 'Allows you to draw a rectangle with a color gradation.',
                 '',
                 true,$042E);  {Alt + C}
  Definir_option(46,'Ellipse with gradation',
                 'Allows you to draw an ellipse filled with a color gradation.',
                 '',
                 true,$052E);  {Shift + Alt + C}
  Definir_option(47,'Adjust picture',
                 'Allows you to move the whole picture in order to re-center it.',
                 'Notice that what gets out from a side reappears on the other.',
                 true,$004C);  {Kpad5}
  Definir_option(48,'Flip/shrink picture menu',
                 'Opens a menu which allows you to flip the picture horizontally/vertically or',
                 'to shrink it to half-scale horizontally and/or vertically.',
                 true,$014C);  {Shift + Kpad5}
  Definir_option(49,'Drawing effects',
                 'Opens a menu where you can enable/disable and configure the drawing effects',
                 'listed below.',
                 true,$0012);  {E}
  Definir_option(50,'Shade mode',
                 'Allows you to shade or lighten some pixels of the picture belonging to a',
                 'color range, in addition of any drawing tool.',
                 true,$003F);  {F5}
  Definir_option(51,'Shade menu',
                 'Opens a menu where you can choose color ranges to use with the Shade mode.',
                 'This menu also contains parameters used both in Shade and Quick-shade modes.',
                 true,$0158);  {Shift + F5}
  Definir_option(131,'Quick-shade mode',
                 'Does the same thing as shade mode with a simpler method (faster to define',
                 'but a bit less powerful).',
                 true,$0262);  {Ctrl + F5}
  Definir_option(132,'Quick-shade menu',
                 'Opens a menu where you can define the parameters of the quick-shade mode.',
                 '',
                 true,$0362);  {Shift + Ctrl + F5}
  Definir_option(52,'Stencil mode',
                 'Allows you to mask colors that must not be affected when you are drawing.',
                 '',
                 true,$0040);  {F6}
  Definir_option(53,'Stencil menu',
                 'Opens a menu where you can choose colors masked by the Stencil mode.',
                 '',
                 true,$0159);  {Shift + F6}
  Definir_option(54,'Mask mode',
                 'Allows you to mask colors of the spare page that will keep you from ',
                 'drawing. This mode should be called "True stencil".',
                 true,$046D);  {Alt + F6}
  Definir_option(55,'Mask menu',
                 'Opens a menu where you can choose colors for the Mask mode.',
                 '',
                 true,$056D);  {Shift + Alt + F6}
  Definir_option(56,'Grid mode',
                 'Force the cursor to snap up grid points.',
                 '',
                 true,$0022);  {G}
  Definir_option(57,'Grid menu',
                 'Open a menu where you can configure the grid used by Grid mode.',
                 '',
                 true,$0122);  {Shift + G}
  Definir_option(58,'Sieve mode',
                 'Only draws pixels on certain positions matching with a sieve.',
                 '',
                 true,$0222);  {Ctrl + G}
  Definir_option(59,'Sieve menu',
                 'Opens a menu where you can configure the sieve.',
                 '',
                 true,$0322);  {Shift + Ctrl + G}
  Definir_option(60,'Invert sieve',
                 'Inverts the pattern defined in the Sieve menu.',
                 '',
                 true,$0622);  {Ctrl + Alt + G}
  Definir_option(61,'Colorize mode',
                 'Allows you to colorize the pixels on which your brush is pasted.',
                 'This permits you to make transparency effects.',
                 true,$0041);  {F7}
  Definir_option(62,'Colorize menu',
                 'Opens a menu where you can give the opacity percentage for Colorize mode.',
                 '',
                 true,$015A);  {Shift + F7}
  Definir_option(63,'Smooth mode',
                 'Soften pixels on which your brush is pasted.',
                 '',
                 true,$0042);  {F8}
  Definir_option(123,'Smooth menu',
                 'Opens a menu where you can define the Smooth matrix.',
                 '',
                 true,$015B);  {Shift + F8}
  Definir_option(64,'Smear mode',
                 'Smears the pixels when you move your brush on the picture.',
                 '',
                 true,$046F);  {Alt + F8}
  Definir_option(65,'Tiling mode',
                 'Puts parts of the brush where you draw.',
                 '',
                 true,$0430);  {Alt + B}
  Definir_option(66,'Tiling menu',
                 'Opens a menu where you can configure the origin of the tiling.',
                 '',
                 true,$0530);  {Shift + Alt + B}
  Definir_option(67,'Classical brush grabbing',
                 'Allows you to pick a brush defined within a rectangle.',
                 '',
                 true,$0030);  {B}
  Definir_option(68,'"Lasso" brush grabbing',
                 'Allows you to pick a brush defined within a freehand polygon.',
                 '',
                 true,$0230);  {Ctrl + B}
  Definir_option(69,'Get previous brush back',
                 'Restore the last user-defined brush.',
                 '',
                 true,$0130);  {Shift + B}
  Definir_option(70,'Horizontal brush flipping',
                 'Reverse brush horizontally.',
                 '',
                 true,$002D);  {X}
  Definir_option(71,'Vertical brush flipping',
                 'Reverse brush vertically.',
                 '',
                 true,$0015);  {Y}
  Definir_option(72,'90ø brush rotation',
                 'Rotate the user-defined brush by 90ø (counter-clockwise).',
                 '',
                 true,$002C);  {Z (W en AZERTY)}
  Definir_option(73,'180ø brush rotation',
                 'Rotate the user-defined brush by 180ø.',
                 '',
                 true,$012C);  {Shift + Z}
  Definir_option(74,'Strech brush',
                 'Allows you to resize the user-defined brush.',
                 '',
                 true,$001F);  {S}
  Definir_option(75,'Distort brush',
                 'Allows you to distort the user-defined brush.',
                 '',
                 true,$011F);  {Shift + S}
  Definir_option(76,'Outline brush',
                 'Outlines the user-defined brush with the fore color.',
                 '',
                 true,$0018);  {O}
  Definir_option(77,'Nibble brush',
                 'Deletes the borders of the user-defined brush.',
                 'This does the opposite of the Outline option.',
                 true,$0118);  {Shift + O}
  Definir_option(78,'Get colors from brush',
                 'Copy colors of the spare page that are used in the brush.',
                 '',
                 true,$0085);  {F11}
  Definir_option(79,'Recolorize brush',
                 'Recolorize pixels of the user-defined brush in order to get a brush which',
                 'looks like the one grabbed in the spare page.',
                 true,$0086);  {F12}
  Definir_option(80,'Rotate by any angle',
                 'Rotate the brush by an angle that you can define.',
                 '',
                 true,$0011);  {W (Z en AZERTY)}
  Definir_option(81,'Pipette',
                 'Allows you to copy the color of a pixel in the picture into the foreground',
                 'or background color.',
                 true,$0029);  {`~ (Touche sous le Esc - ý en AZERTY)}
  Definir_option(82,'Swap foreground/background colors',
                 'Invert foreground and background colors.',
                 '',
                 true,$0129);  {Shift + `~}
  Definir_option(83,'Magnifier mode',
                 'Allows you to zoom into the picture.',
                 '',
                 true,$0032);  {M (,? sur AZERTY)}
  Definir_option(84,'Zoom factor menu',
                 'Opens a menu where you can choose a magnifying factor.',
                 '',
                 true,$0132);  {Shift + M}
  Definir_option(85,'Zoom in',
                 'Increase magnifying factor.',
                 '',
                 true,$004E);  {Grey +}
  Definir_option(86,'Zoom out',
                 'Decrease magnifying factor.',
                 '',
                 true,$004A);  {Grey -}
  Definir_option(87,'Brush effects menu',
                 'Opens a menu which proposes different effects on the user-defined brush.',
                 '',
                 true,$0630);  {Ctrl + Alt + B}
  Definir_option(88,'Text',
                 'Opens a menu which permits you to type in a character string and to choose a',
                 'font, and then creates a new user-defined brush fitting to your choices.',
                 true,$0014);  {T}
  Definir_option(89,'Screen resolution menu',
                 'Opens a menu where you can choose the dimensions of the screen in which you',
                 'want to draw among the numerous X and SVGA proposed modes.',
                 true,$001C);  {Enter}
  Definir_option(90,'"Safety" resolution',
                 'Set resolution to 320x200. This can be useful if you choosed a resolution',
                 'that is not supported by your monitor and video card. Cannot be removed.',
                 false,$011C); {Shift + Enter}
  Definir_option(91,'Help and credits',
                 'Opens a window where you can get information about the program.',
                 '',
                 true,$003B);  {F1}
  Definir_option(92,'Statistics',
                 'Displays miscellaneous more or less useful information.',
                 '',
                 true,$0154);  {Shift + F1}
  Definir_option(93,'Jump to spare page',
                 'Swap current page and spare page.',
                 '',
                 true,$000F);  {Tab}
  Definir_option(94,'Copy current page to spare page',
                 'Copy current page to spare page.',
                 '',
                 true,$010F);  {Shift + Tab}
  Definir_option(95,'Save picture as...',
                 'Opens a file-selector that allows you to save your picture with a new',
                 'path-name.',
                 true,$003C);  {F2}
  Definir_option(96,'Save picture',
                 'Saves your picture with the last name you gave it.',
                 '',
                 true,$0155);  {Shift + F2}
  Definir_option(97,'Load picture',
                 'Opens a file-selector that allows you to load a new picture.',
                 '',
                 true,$003D);  {F3}
  Definir_option(98,'Re-load picture',
                 'Re-load the current picture.',
                 'This allows you to cancel modifications made since last saving.',
                 true,$0156);  {Shift + F3}
  Definir_option(99,'Save brush',
                 'Opens a file-selector that allows you to save your current user-defined',
                 'brush.',
                 true,$025F);  {Ctrl + F2}
  Definir_option(100,'Load brush',
                 'Opens a file-selector that allows you to load a brush.',
                 '',
                 true,$0260);  {Ctrl + F3}
  Definir_option(101,'Settings',
                 'Opens a menu which permits you to set the dimension of your picture, and to',
                 'modify some parameters of the program.',
                 true,$015D);  {Shift + F10}
  Definir_option(102,'Undo (Oops!)',
                 'Cancel the last action which modified the picture. This has no effect after',
                 'a jump to the spare page, loading a picture or modifying its size.',
                 true,$0016);  {U}
  Definir_option(103,'Redo',
                 'Redo the last undone action. This has no effect after a jump to the spare',
                 'page, loading a picture or modifying its size.',
                 true,$0116);  {Shift + U}
  Definir_option(133,'Kill',
                 'Kills the current page. It actually removes the current page from the list',
                 'of "Undo" pages.',
                 true,$0153);  {Shift + Suppr}
  Definir_option(104,'Clear page',
                 'Clears the picture with the first color of the palette (usually black).',
                 '',
                 true,$000E);  {BackSpace}
  Definir_option(105,'Clear page with backcolor',
                 'Clears the picture with the backcolor.',
                 '',
                 true,$010E);  {Shift + BackSpace}
  Definir_option(106,'Quit program',
                 'Allows you to leave the program.',
                 'If modifications were not saved, confirmation is asked.',
                 false,$0010);  {Q (A en AZERTY)}
  Definir_option(107,'Palette menu',
                 'Opens a menu which allows you to modify the current palette.',
                 '',
                 true,$0019);  {P}
  Definir_option(125,'Secondary palette menu',
                 'Opens a menu which allows you to define color series and some tagged colors.',
                 '',
                 true,$0119);  {Shift + P}
  Definir_option(130,'Exclude colors menu',
                 'Opens a menu which allows you to define the colors you don''t want to use in',
                 'modes such as Smooth and Transparency, or when remapping a brush.',
                 true,$0219);  {Ctrl + P}
  Definir_option(108,'Scroll palette to the left',
                 'Scroll palette in the tool bar to the left, column by column.',
                 '',
                 true,$0049);  {PgUp}
  Definir_option(109,'Scroll palette to the right',
                 'Scroll palette in the tool bar to the right, column by column.',
                 '',
                 true,$0051);  {PgDn}
  Definir_option(110,'Scroll palette to the left faster',
                 'Scroll palette in the tool bar to the left, 8 columns by 8 columns.',
                 '',
                 true,$0149);  {Shift + PgUp}
  Definir_option(111,'Scroll palette to the right faster',
                 'Scroll palette in the tool bar to the right, 8 columns by 8 columns.',
                 '',
                 true,$0151);  {Shift + PgDn}
  Definir_option(112,'Center brush attachment point',
                 'Set the attachement of the user-defined brush to its center.',
                 '',
                 true,$028F);  {Ctrl + 5 (pav‚ num‚rique)}
  Definir_option(113,'Top-left brush attachment point',
                 'Set the attachement of the user-defined brush to its top-left corner.',
                 '',
                 true,$0277);  {Ctrl + 7}
  Definir_option(114,'Top-right brush attachment point',
                 'Set the attachement of the user-defined brush to its top-right corner.',
                 '',
                 true,$0284);  {Ctrl + 9}
  Definir_option(115,'Bottom-left brush attachment point',
                 'Set the attachement of the user-defined brush to its bottom-left corner.',
                 '',
                 true,$0275);  {Ctrl + 1}
  Definir_option(116,'Bottom-right brush attachment point',
                 'Set the attachement of the user-defined brush to its bottom-right corner.',
                 '',
                 true,$0276);  {Ctrl + 3}
  Definir_option(117,'Next foreground color',
                 'Set the foreground color to the next in the palette.',
                 '',
                 true,$001B);  {] ($ en AZERTY)}
  Definir_option(118,'Previous foreground color',
                 'Set the foreground color to the previous in the palette.',
                 '',
                 true,$001A);  {[ (^ en AZERTY)}
  Definir_option(119,'Next background color',
                 'Set the background color to the next in the palette.',
                 '',
                 true,$011B);  {Shift + ]}
  Definir_option(120,'Previous background color',
                 'Set the background color to the previous in the palette.',
                 '',
                 true,$011A);  {Shift + [}
  Definir_option(126,'Next user-defined forecolor',
                 'Set the foreground color to the next in the user-defined color series.',
                 '',
                 true,$000D);  {'=+'}
  Definir_option(127,'Previous user-defined forecolor',
                 'Set the foreground color to the previous in the user-defined color series.',
                 '',
                 true,$000C);  {'-_' (')ø' en AZERTY}
  Definir_option(128,'Next user-defined backcolor',
                 'Set the background color to the next in the user-defined color series.',
                 '',
                 true,$010D);  {Shift + '=+'}
  Definir_option(129,'Previous user-defined backcolor',
                 'Set the background color to the previous in the user-defined color series.',
                 '',
                 true,$010C);  {Shift + '-_' (')ø' en AZERTY}
  Definir_option(121,'Shrink paintbrush',
                 'Decrease the width of the paintbrush if it is special circle or square.',
                 '',
                 true,$0033);  {,< (;. en AZERTY)}
  Definir_option(122,'Enlarge paintbrush',
                 'Increase the width of the paintbrush if it is special circle or square.',
                 '',
                 true,$0034);  {.> (:/ en AZERTY)}
  NB_OPTIONS:=Numero_definition_option-1;

  assign(Fichier_INI,'GFX2.INI');
  {$I-}
  reset(Fichier_INI,1);
  {$I+}
  if IOresult<>0 then {Si GFX2.INI est absent, on l'extrait de GFX2.DAT}
    Erreur:=Recreer_INI
  else
    close(Fichier_INI);

  if not Erreur then Interpretation_du_fichier_config;

  Initialiser_config:=Erreur;
end;


{------------------- Enregistrement de la configuration ---------------------}
procedure Enregistrer_config;
var
    Fichier:file;
    Indice:word;
    Octet:byte;
begin
  if Choix_enreg=1 then {Enregistrement des touches (si souhait‚)}
  begin
    assign(Fichier,'GFX2.CFG');
    reset(Fichier,1);

    seek(Fichier,sizeof(Type_header)+Sizeof(Type_chunk)); {Positionnement sur la 1Šre touche}

    for Indice:=1 to NB_OPTIONS do
      blockwrite(Fichier,Config[Indice],sizeof(Type_infos_touche)); {Les 3 premiers champs (words) sont: Num‚ro,Touche,Touche2}

    close(Fichier);
  end;
end;

{---------- V‚rifier qu'on peut ‚crire sur le lecteur de lancement ----------}
function Verifier_ecriture_possible:boolean;
var
    Fichier:file;
begin
  assign(Fichier,'GFX2.$$$');
  {$I-}
  rewrite(Fichier,1);
  close(Fichier);
  erase(Fichier);
  {$I+}
  Verifier_ecriture_possible:=(IOresult=0);
end;

{============================================================================}
var
    Keyb_mode:byte;

begin
  if Verifier_ecriture_possible then
  begin

    asm
      mov  ax,0AD83h
      int  2Fh
      mov  Keyb_mode,bl
      mov  ax,0AD82h
      xor  bl,bl
      int  2Fh
    end;

    Text_mode_80x50;
    Cacher_curseur;
    Dessiner_ecran_principal;
    if not Initialiser_config then
    begin
      Setup;
      Enregistrer_config;

      Text_mode_80x25;
      Montrer_curseur;

      asm
        mov  ax,0AD82h
        mov  bl,Keyb_mode
        int  2Fh
      end;

      halt(0);
    end
    else
    begin
      Text_mode_80x25;
      writeln('Error reading GFX2.DAT! This file is either absent or corrupt.');
      Montrer_curseur;

      asm
        mov  ax,0AD82h
        mov  bl,Keyb_mode
        int  2Fh
      end;
      halt(1);
    end;
  end
  else
  begin
    writeln('Error: You mustn''t run this setup program from a read-only drive!');
    writeln;
    writeln('The most probable cause of this error is that you are either running this');
    writeln('program from a CD-ROM or a protected floppy disk.');
    writeln('You should try to copy all the files of GrafX 2.00 on a hard drive, or to');
    writeln('unprotect your floppy disk if you really want to run it from this medium.');
    halt(1);
  end;
end.
