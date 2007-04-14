program makedat;

type tpalette=array[0..767] of byte;
var  palette:tpalette;

{****************************************************************************}
{****************************************************************************}
{****************************************************************************}

Procedure XMODE;assembler;
const
  X360Y270 : array[0..18] of word =
  ( $12E7, $6B00, $5901, $5A02, $8E03, $5E04, $8A05, $3006, $F007, $0008,
    $6109, $2010, $A911, $1B12, $2D13, $0014, $1F15, $2F16, $E317 );
asm
  push ds

  mov   ax,13h
  int   10h

  mov   dx,3c4h
  mov   ax,0604h
  out   dx,ax
  mov   ax,0100h
  out   dx,ax

  lea si,X360Y270
  lodsb
  or    al,al
  jz    @@DontSetDot
  mov   dx,3C2h
  out   dx,al

  @@DontSetDot:
  mov   dx,3C4h
  mov   ax,0300h
  out   dx,ax
  mov   dx,3D4h
  mov   al,11h
  out   dx,al
  inc   dx
  in    al,dx
  and   al,07fh
  out   dx,al
  dec   dx
  cld
  xor   cx,cx
  lodsb
  mov   cl,al

  @@SetCRTParmsLoop:
  lodsw
  out   dx,ax
  loop  @@SetCRTParmsLoop

  mov   dx,3C4h
  mov   ax,0f02h
  out   dx,ax
  mov   ax,0A000h
  mov   es,ax
  sub   di,di
  sub   ax,ax
  mov   cx,8000h
  rep   stosw

  mov   dx,3D4h
  mov   al,13h
  out   dx,al
  inc   dx
  mov   ax,90
  shr   ax,1
  out   dx,al

  pop ds
end;

Procedure pixel(x,y : word;coul : byte);assembler;
asm
  mov dx,3C4h
  mov ax,0102h
  mov cx,x
  and cx,3
  shl ah,cl
  out dx,ax

  mov ax,0A000h
  mov es,ax

  mov ax,y
  mov bx,90
  mul bx

  mov bx,x
  shr bx,2
  add ax,bx
  mov di,ax
  mov al,coul
  stosb
end;

Function lit_pixel(x,y : word):byte;
var a : byte;
begin
  asm
    mov ax,0A000h
    mov es,ax

    mov ax,y
    mov bx,90
    mul bx

    mov bx,x
    mov cl,bl
    and cl,3

    shr bx,2
    add ax,bx
    mov di,ax

    mov dx,3CEh
    mov al,4
    mov ah,cl
    out dx,ax

    mov al,[es:di]
    mov a,al
  end;
  lit_pixel:=a;
end;


procedure set_palette; assembler;
asm
  push ds
  lea si,palette
  mov dx,3C8h
  xor al,al
  out dx,al
  inc dl
  mov cx,768
  rep outsb
  pop ds
end;


{----------------------------------------------------------------------------}
{----------------------------------------------------------------------------}
{----------------------------------------------------------------------------}

type t_os_tampon=array[0..63999] of byte;
var  os_index:word;
var  os_tampon:^t_os_tampon;

function octet_suivant(var f:file):byte;
var effect_result:word;
begin
  inc(os_index);
  if (os_index>=64000) then
  begin
    {$I-}
    blockread(f,os_tampon^,64000,effect_result);
    {$I+}
    os_index:=0;
  end;
  octet_suivant:=os_tampon^[os_index];
end;

procedure init_os;
begin
  new(os_tampon);
  os_index:=64000;
end;

procedure close_os;
begin
  dispose(os_tampon);
end;

{----------------------------------------------------------------------------}

var eo_index:word;
var eo_tampon:^t_os_tampon;

procedure ecrire_octet(var f:file; octet:byte);
var
    effect_result:word;
begin
  eo_tampon^[eo_index]:=octet;
  inc(eo_index);
  if (eo_index>=64000) then
  begin
    blockwrite(f,eo_tampon^,64000,effect_result);
    eo_index:=0;
  end;
end;

procedure init_eo;
begin
  new(eo_tampon);
  eo_index:=0;
end;

procedure close_eo(var f:file);
var effect_result:word;
begin
  if eo_index>0 then blockwrite(f,eo_tampon^,eo_index,effect_result);
  dispose(eo_tampon);
end;

{----------------------------------------------------------------------------}
{----------------------------------------------------------------------------}
{----------------------------------------------------------------------------}



{--------------------- DEPACKER une IMAGE au format PKM ---------------------}
procedure load_PKM(nom_fichier:string);
{errcode retournÇ: 0=OK , 1=pas un PKM , 2=taille incorrecte}
type
  header = record {taille: 780 octets}
             ident       : array[1..3] of char; { chaåne "PKM" }
             methode     : byte; { mÇthode de compression:           }
                                 { 0 = compression en ligne (c)KM    }
                                 { autres = inconnues pour le moment }
             Recon1      : byte; { octet de reconnaissance sur 1 octet  }
             Recon2      : byte; { octet de reconnaissance sur 2 octets }
             longx       : word; { taille de l'image en X }
             longy       : word; { taille de l'image en Y }
             palette     : tpalette; { palette RVB 256*3 }
             jump        : word; { taille du saut entre le header et l'image }
           end;
var
  TailleX,TailleY:word;
  errcode:byte;
  coul,octet:byte;
  taille_fichier:word;
  i,j,k:word;
  c:longint;
  offsX,offsY,X,Y:word;
  fich:file;
  head: header;
  taille_image: longint;
begin
  offsX:=0;
  offsY:=0;
  assign(fich,Nom_Fichier);
  reset(fich,1);
  taille_fichier:=filesize(fich);
  blockread(fich,head,SizeOf(Header));
  if Head.LongY>270 then Head.LongY:=270;

  if (head.ident<>'PKM') then
  begin
    errcode:=1;
    exit;
  end;
  if (head.longX>360) then
  begin
    errcode:=2;
    exit;
  end;

  { dÇfinition de la palette }
  Palette:=head.palette;

  set_palette;

  TailleX:=Head.LongX;
  TailleY:=Head.LongY;
  taille_image:=Head.LongX;
  taille_image:=taille_image*Head.LongY;
  Seek(fich,SizeOf(Header)+Head.Jump);

  init_os;

  j:=0;
  octet:=octet_suivant(fich);
  c:=0;
  while (c<taille_image) and (j<taille_fichier-SizeOf(Header)-Head.Jump) do
  begin
    if (octet<>Head.Recon1) and (octet<>Head.Recon2)
    then
    begin
      X:=c mod head.longX;
      Y:=c div head.longX;
      if (X<360) and (Y<270) then
        pixel(X,Y,octet);
      inc(j);
      octet:=octet_suivant(fich);
      inc(c);
    end
    else
    begin
      if octet=Head.Recon1
      then
      begin
        inc(j);
        octet:=octet_suivant(fich);
        coul:=octet;
        inc(j);
        octet:=octet_suivant(fich)-1;
        for i:=0 to octet do
        begin
          X:=(c+i) mod head.longX;
          Y:=(c+i) div head.longX;
          if (X<360) and (Y<270) then
            pixel(X,Y,coul);
        end;
        c:=c+octet+1;
        inc(j);
        octet:=octet_suivant(fich);
      end
      else
      begin
        inc(j);
        octet:=octet_suivant(fich);
        coul:=octet;
        inc(j);
        octet:=octet_suivant(fich);
        k:=(octet shl 8)+octet_suivant(fich);
        for i:=0 to (k-1) do
        begin
          X:=(c+i) mod head.longX;
          Y:=(c+i) div head.longX;
          if (X<360) and (Y<270) then
            pixel(X,Y,coul);
        end;
        c:=c+k;
        j:=j+2;
        octet:=octet_suivant(fich);
      end;
    end;
  end;
  close_os;
  close(fich);
  errcode:=0;
end;

{****************************************************************************}
{****************************************************************************}
{****************************************************************************}

var Crypt_curseur:byte;
const Crypt_cle:string[13]='Sunset Design';

function crypt(octet:byte):byte;
begin
  crypt:=octet xor ord(Crypt_cle[Crypt_curseur]);
  Crypt_curseur:=(Crypt_curseur mod 13)+1;
end;

procedure Ecrire_palette(var Fichier:file);
var i:word;
begin
  for i:=0 to 767 do ecrire_octet(Fichier,crypt(palette[i]));
end;

procedure Lire_et_ecrire_sprite(var Fichier:file; x1,y1,x2,y2:word);
var i,j:word;
begin
  for j:=y1 to y2 do
    for i:=x1 to x2 do
      ecrire_octet(Fichier,crypt(lit_pixel(i,j)));
end;


procedure Ecrire_trames_predefinies(var Fichier:file);
var
    n,i,j:word;
    Octet:byte;
    Pixel_lu:byte;
begin
  for n:=0 to 11 do {Pour chaque trame prÇdÇfinie}
  begin
    for j:=0 to 15 do
    begin
      Octet:=0;
      for i:=0 to 7 do
      begin
        Octet:=(Octet shl 1);
        Pixel_lu:=lit_pixel(9+(17*n)+i,196+j);
        if Pixel_lu>0 then
          inc(Octet);
      end;
      ecrire_octet(Fichier,crypt(Octet));

      Octet:=0;
      for i:=0 to 7 do
      begin
        Octet:=(Octet shl 1);
        Pixel_lu:=lit_pixel(1+(17*n)+i,196+j);
        if Pixel_lu>0 then
          inc(Octet);
      end;
      ecrire_octet(Fichier,crypt(Octet));
    end;
  end;
end;


procedure Rajouter_fichier(var Fichier:file;Nom_fichier:string;Crypter:boolean);
var
    Fichier_a_ajouter:file;
    Taille           :word;
    Indice           :word;
begin
  assign   (Fichier_a_ajouter,Nom_fichier);
  reset    (Fichier_a_ajouter,1);
  Taille:=filesize(Fichier_a_ajouter);

  init_os;
  if (Crypter) then
    for Indice:=1 to Taille do
      ecrire_octet(Fichier,crypt(octet_suivant(Fichier_a_ajouter)))
  else
    for Indice:=1 to Taille do
      ecrire_octet(Fichier,octet_suivant(Fichier_a_ajouter));
  close_os;

  close(Fichier_a_ajouter);
end;


procedure Coder_partie_de_fonte(var Fichier:file;Pos_X,Pos_Y,Nb_char:word);
var
    X,Y:word;
    Indice_char:word;
    Couleur_temporaire:byte;
begin
  for Indice_char:=1 to Nb_char do
  begin
    for X:=0 to 5 do
      for Y:=0 to 7 do
      begin
        Couleur_temporaire:=lit_pixel(Pos_X+X,Pos_Y+Y);

        if not (Couleur_temporaire in [25,7,15])
        then Couleur_temporaire:=0;

        ecrire_octet(Fichier,crypt(Couleur_temporaire));
      end;

    inc(Pos_X,6);
  end;
end;


var
    Fichier:file;
    Position_de_depart_de_la_section_d_aide:longint;
    Nombre_de_lignes_d_aide :word;
    Nombre_d_octets_utilises:word;


procedure Demarrer_section_d_aide;
begin
  Nombre_de_lignes_d_aide :=0;
  Nombre_d_octets_utilises:=0;
  assign(Fichier,'GFX2.DAT');
  reset (Fichier,1);
  Position_de_depart_de_la_section_d_aide:=filesize(Fichier);
  seek(Fichier,Position_de_depart_de_la_section_d_aide);
  {On Çcrit temporairement des valeurs erronÇes dans le fichier:}
  blockwrite(Fichier,Nombre_de_lignes_d_aide ,2); {Nb de lignes}
  blockwrite(Fichier,Nombre_d_octets_utilises,2); {Nb d'octets utilisÇs}
end;


procedure Terminer_section_d_aide;
begin
  seek      (Fichier,Position_de_depart_de_la_section_d_aide);
  {On Çcrit dÇfinitivement les valeurs correctes dans le fichier:}
  blockwrite(Fichier,Nombre_de_lignes_d_aide ,2); {Nb de lignes}
  blockwrite(Fichier,Nombre_d_octets_utilises,2); {Nb d'octets utilisÇs}
  close     (Fichier);
end;

const
      TITRE      = 1;
      SOUS_TITRE = 2;
      NORMAL     = 3;
      T          = TITRE;
      S          = SOUS_TITRE;
      N          = NORMAL;

{$I TABLES}

procedure Ecrire_ligne_d_aide(De_type:byte;Chaine:string);
var
    Code:byte;   {Code Ö inscrire dans le fichier}
    Indice:byte; {Indice de balayage de la chaåne}
begin

  if length(Chaine)>44 then
  begin
    close     (Fichier);
    asm
      mov ax,3
      int 10h
    end;
    writeln('ERREUR DANS L''AIDE !!! Il y a une ligne qui est trop longue !!!');
    halt(1);
  end;

  case De_type of

    TITRE :
      begin
        {1äre ligne:}

          {On code la taille de la 1äre ligne chaåne:}
          Code:=(length(Chaine) shl 1) or $80;
          blockwrite(Fichier,Code,1);

          {On code la 1äre ligne de la chaåne selon la table des caractäres}
          {de titrage:}
          for Indice:=1 to length(Chaine) do
          begin
            Code:=TABLE_TITRE[ord(Chaine[Indice])];
            blockwrite(Fichier,Code,1);
            inc(Code);
            blockwrite(Fichier,Code,1);
          end;

          {On met Ö jour les compteurs:}
          inc(Nombre_de_lignes_d_aide);
          inc(Nombre_d_octets_utilises,(length(Chaine) shl 1)+1);

        {2äme ligne:}

          {On code la taille de la 2äme ligne chaåne:}
          Code:=(length(Chaine) shl 1) or $80;
          blockwrite(Fichier,Code,1);

          {On code la 2äme ligne de la chaåne selon la table des caractäres}
          {de titrage:}
          for Indice:=1 to length(Chaine) do
          begin
            Code:=TABLE_TITRE[ord(Chaine[Indice])];
            if Chaine[Indice] in ['A'..'V']
            then inc(Code,44)
            else inc(Code,40);

            blockwrite(Fichier,Code,1);
            inc(Code);
            blockwrite(Fichier,Code,1);
          end;

          {On met Ö jour les compteurs:}
          inc(Nombre_de_lignes_d_aide);
          inc(Nombre_d_octets_utilises,(length(Chaine) shl 1)+1);
      end;

    SOUS_TITRE :
      begin
        {On code la taille de la chaåne:}
        Code:=length(Chaine);
        blockwrite(Fichier,Code,1);

        {On code la chaåne selon la table des sous-titres:}
        for Indice:=1 to length(Chaine) do
        begin
          Code:=TABLE_SOUS_TITRE[ord(Chaine[Indice])];
          blockwrite(Fichier,Code,1);
        end;

        {On met Ö jour les compteurs:}
        inc(Nombre_de_lignes_d_aide);
        inc(Nombre_d_octets_utilises,length(Chaine)+1);
      end;

    NORMAL :
      begin
        {On code la taille de la chaåne:}
        Code:=length(Chaine);
        blockwrite(Fichier,Code,1);

        {On code la chaåne selon la table des caractäres normaux:}
        for Indice:=1 to length(Chaine) do
        begin
          Code:=TABLE_NORMAL[ord(Chaine[Indice])];
          blockwrite(Fichier,Code,1);
        end;

        {On met Ö jour les compteurs:}
        inc(Nombre_de_lignes_d_aide);
        inc(Nombre_d_octets_utilises,length(Chaine)+1);
      end;

  end;

end;


{$I HLP_GRET}
{$I HLP_CRDT}
{$I HLP_RGST}


BEGIN
  XMODE;

  Load_PKM('GFX2.PKM');

  assign(Fichier,'GFX2.DAT');
  rewrite(Fichier,1);
  init_eo;

  Crypt_curseur:=1;

  Ecrire_palette(Fichier);
  Lire_et_ecrire_sprite(Fichier,  1,  1,254, 44); {Menu}

  Lire_et_ecrire_sprite(Fichier,  2, 47, 15, 60); {Shade}
  Lire_et_ecrire_sprite(Fichier, 19, 47, 32, 60); {Colorize}
  Lire_et_ecrire_sprite(Fichier, 36, 47, 49, 60); {Smooth}
  Lire_et_ecrire_sprite(Fichier, 53, 47, 66, 60); {Tiling}
  Lire_et_ecrire_sprite(Fichier, 70, 47, 83, 60); {Stencil}
  Lire_et_ecrire_sprite(Fichier, 87, 47,100, 60); {Sieve}
  Lire_et_ecrire_sprite(Fichier,104, 47,117, 60); {Grid}
  Lire_et_ecrire_sprite(Fichier,121, 47,134, 60); {Mask}
  Lire_et_ecrire_sprite(Fichier,138, 47,151, 60); {Smear}

  Lire_et_ecrire_sprite(Fichier,  1, 85, 15, 99); {Fleche}
  Lire_et_ecrire_sprite(Fichier, 17, 85, 31, 99); {Viseur}
  Lire_et_ecrire_sprite(Fichier, 65, 85, 79, 99); {Viseur pipette}
  Lire_et_ecrire_sprite(Fichier, 33, 85, 47, 99); {Sablier}
  Lire_et_ecrire_sprite(Fichier, 49, 85, 63, 99); {Multidirectionnel}
  Lire_et_ecrire_sprite(Fichier, 81, 85, 95, 99); {Horizontal}
  Lire_et_ecrire_sprite(Fichier, 97, 85,111, 99); {Viseur fin}
  Lire_et_ecrire_sprite(Fichier,113, 85,127, 99); {Viseur pipette fin}

  Lire_et_ecrire_sprite(Fichier,  2,102, 15,115); {Dessin continu}
  Lire_et_ecrire_sprite(Fichier, 19,102, 32,115); {Dessin interrompu}
  Lire_et_ecrire_sprite(Fichier, 36,102, 49,115); {Dessin point}
  Lire_et_ecrire_sprite(Fichier, 53,102, 66,115); {Pinceau-brosse}
  Lire_et_ecrire_sprite(Fichier, 70,102, 83,115); {Courbe 3 pts}
  Lire_et_ecrire_sprite(Fichier, 87,102,100,115); {Courbe 4 pts}
  Lire_et_ecrire_sprite(Fichier,104,102,117,115); {Ligne}
  Lire_et_ecrire_sprite(Fichier,121,102,134,115); {K-Line}
  Lire_et_ecrire_sprite(Fichier,138,102,151,115); {Lignes concentiques}

  Lire_et_ecrire_sprite(Fichier,  1,120, 16,135); {Pinceau  1}
  Lire_et_ecrire_sprite(Fichier, 18,120, 33,135); {Pinceau  2}
  Lire_et_ecrire_sprite(Fichier, 35,120, 50,135); {Pinceau  3}
  Lire_et_ecrire_sprite(Fichier, 52,120, 67,135); {Pinceau  4}
  Lire_et_ecrire_sprite(Fichier, 69,120, 84,135); {Pinceau  5}
  Lire_et_ecrire_sprite(Fichier, 86,120,101,135); {Pinceau  6}
  Lire_et_ecrire_sprite(Fichier,103,120,118,135); {Pinceau  7}
  Lire_et_ecrire_sprite(Fichier,120,120,135,135); {Pinceau  8}
  Lire_et_ecrire_sprite(Fichier,137,120,152,135); {Pinceau  9}
  Lire_et_ecrire_sprite(Fichier,154,120,169,135); {Pinceau 10}
  Lire_et_ecrire_sprite(Fichier,171,120,186,135); {Pinceau 11}
  Lire_et_ecrire_sprite(Fichier,188,120,203,135); {Pinceau 12}
  Lire_et_ecrire_sprite(Fichier,  1,137, 16,152); {Pinceau 13}
  Lire_et_ecrire_sprite(Fichier, 18,137, 33,152); {Pinceau 14}
  Lire_et_ecrire_sprite(Fichier, 35,137, 50,152); {Pinceau 15}
  Lire_et_ecrire_sprite(Fichier, 52,137, 67,152); {Pinceau 16}
  Lire_et_ecrire_sprite(Fichier, 69,137, 84,152); {Pinceau 17}
  Lire_et_ecrire_sprite(Fichier, 86,137,101,152); {Pinceau 18}
  Lire_et_ecrire_sprite(Fichier,103,137,118,152); {Pinceau 19}
  Lire_et_ecrire_sprite(Fichier,120,137,135,152); {Pinceau 20}
  Lire_et_ecrire_sprite(Fichier,137,137,152,152); {Pinceau 21}
  Lire_et_ecrire_sprite(Fichier,154,137,169,152); {Pinceau 22}
  Lire_et_ecrire_sprite(Fichier,171,137,186,152); {Pinceau 23}
  Lire_et_ecrire_sprite(Fichier,188,137,203,152); {Pinceau 24}
  Lire_et_ecrire_sprite(Fichier,  1,154, 16,169); {Pinceau 25}
  Lire_et_ecrire_sprite(Fichier, 18,154, 33,169); {Pinceau 26}
  Lire_et_ecrire_sprite(Fichier, 35,154, 50,169); {Pinceau 27}
  Lire_et_ecrire_sprite(Fichier, 52,154, 67,169); {Pinceau 28}
  Lire_et_ecrire_sprite(Fichier, 69,154, 84,169); {Pinceau 29}
  Lire_et_ecrire_sprite(Fichier, 86,154,101,169); {Pinceau 30}
  Lire_et_ecrire_sprite(Fichier,103,154,118,169); {Pinceau 31}
  Lire_et_ecrire_sprite(Fichier,120,154,135,169); {Pinceau 32}
  Lire_et_ecrire_sprite(Fichier,137,154,152,169); {Pinceau 33}
  Lire_et_ecrire_sprite(Fichier,154,154,169,169); {Pinceau 34}
  Lire_et_ecrire_sprite(Fichier,171,154,186,169); {Pinceau 35}
  Lire_et_ecrire_sprite(Fichier,188,154,203,169); {Pinceau 36}
  Lire_et_ecrire_sprite(Fichier,  1,171, 16,186); {Pinceau 37}
  Lire_et_ecrire_sprite(Fichier, 18,171, 33,186); {Pinceau 38}
  Lire_et_ecrire_sprite(Fichier, 35,171, 50,186); {Pinceau 39}
  Lire_et_ecrire_sprite(Fichier, 52,171, 67,186); {Pinceau 40}
  Lire_et_ecrire_sprite(Fichier, 69,171, 84,186); {Pinceau 41}
  Lire_et_ecrire_sprite(Fichier, 86,171,101,186); {Pinceau 42}
  Lire_et_ecrire_sprite(Fichier,103,171,118,186); {Pinceau 43}
  Lire_et_ecrire_sprite(Fichier,120,171,135,186); {Pinceau 44}
  Lire_et_ecrire_sprite(Fichier,137,171,152,186); {Pinceau 45}
  Lire_et_ecrire_sprite(Fichier,154,171,169,186); {Pinceau 46}
  Lire_et_ecrire_sprite(Fichier,171,171,186,186); {Pinceau 47}
  Lire_et_ecrire_sprite(Fichier,188,171,203,186); {Pinceau 48}

  Lire_et_ecrire_sprite(Fichier,  1,188,  7,194); {Diskette 3"´}
  Lire_et_ecrire_sprite(Fichier,  9,188, 15,194); {Diskette 5"¨}
  Lire_et_ecrire_sprite(Fichier, 17,188, 23,194); {HDD}
  Lire_et_ecrire_sprite(Fichier, 25,188, 31,194); {CD-ROM}
  Lire_et_ecrire_sprite(Fichier, 33,188, 39,194); {Network}

  Lire_et_ecrire_sprite(Fichier,  1,213,231,268); {Logo GrafX2}

  Ecrire_trames_predefinies(Fichier);

  Rajouter_fichier(Fichier,'FONTE1.FNT',true); {Fonte systäme}

  Rajouter_fichier(Fichier,'FONTE2.FNT',true); {Fonte exotique}

  Load_PKM('FONTS.PKM');

  Coder_partie_de_fonte(Fichier,0,121,45); {Petites fontes}
  Coder_partie_de_fonte(Fichier,0,129,45);
  Coder_partie_de_fonte(Fichier,0,137,45);
  Coder_partie_de_fonte(Fichier,0,145,12);
  Coder_partie_de_fonte(Fichier,0,153,44); {Grosse fonte}
  Coder_partie_de_fonte(Fichier,0,161,44);
  Coder_partie_de_fonte(Fichier,0,169,40);
  Coder_partie_de_fonte(Fichier,0,177,40);

  close_eo(Fichier);
  close (Fichier);

  asm
    mov ax,3
    int 10h
  end;

  Generer_l_aide_Credits;
  Generer_l_aide_Register;
  Generer_l_aide_Greetings;


  reset(Fichier,1);
  seek(Fichier,filesize(Fichier));
  init_eo;
  Rajouter_fichier(Fichier,'GFX2.INI',false);
  close_eo(Fichier);

  writeln('Le fichier GFX2.DAT est crÇÇ. (taille = ',filesize(Fichier),' octets)');
  close(Fichier);
END.
