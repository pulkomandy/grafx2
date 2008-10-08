;  Grafx2 - The Ultimate 256-color bitmap paint program
;
;  Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)
;
;  Grafx2 is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License
;  as published by the Free Software Foundation; version 2
;  of the License.
;
;  Grafx2 is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
;  write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

.386P
.MODEL FLAT


_DATA Segment dword public 'data'
      Assume cs:_TEXT, ds:_DATA



; ---- Variables export‚es ----

; ---- D‚claration des variables ----

; Variables … utilisation g‚n‚rale

  VarA dd 0
  VarB dd 0
  VarC dd 0
  VarD dd 0
  VarE dd 0
  VarF dd 0

; Variables pour le dithering Flloyd-Steinberg

  DSFRouge   dd 0
  DSFVert    dd 0
  DSFBleu    dd 0
  DSFRougeAD dd 0
  DSFVertAD  dd 0
  DSFBleuAD  dd 0



_DATA ENDS









_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA



; ---- Fonctions export‚es ----

; -- Fonctions de dithering Flloyd-Steinberg --
public OPASM_DitherFS_6123
public OPASM_DitherFS_623
public OPASM_DitherFS_12
public OPASM_DitherFS_6
public OPASM_DitherFS
public OPASM_DitherFS_2
public OPASM_Compter_occurences
public OPASM_Analyser_cluster
public OPASM_Split_cluster_Rouge
public OPASM_Split_cluster_Vert
public OPASM_Split_cluster_Bleu




OPASM_DitherFS_6123 proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,Largeur:dword,Palette:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On place dans VarA la largeur … traiter
  mov  eax,Largeur
  mov  VarA,eax

  ; Pour chacun des pixels sur la largeur:
  DFS6123_Pour_chaque_pixel:

    ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
    mov  edx,[esi]   ; EDX = XBVR
    xor  eax,eax     ; EAX = 0000
    xor  ebx,ebx     ; EBX = 0000
    mov  al,dl       ; EAX = 000R
    mov  bl,dh       ; EBX = 000V
    shr  edx,16      ; EDX = 00XB
    mov  DSFRouge,eax
    xor  dh,dh       ; EDX = 000B
    mov  DSFVert ,ebx
    mov  DSFBleu ,edx
    mov  cl,ReducR
    shr  eax,cl      ; EAX = 000r
    mov  cl,ReducV
    shr  ebx,cl      ; EBX = 000v
    mov  cl,ReducB
    shr  edx,cl      ; EDX = 000b
    mov  cl,NbbV
    shl  eax,cl      ; EAX = 00r0
    or   eax,ebx     ; EAX = 00rv
    mov  cl,NbbB
    shl  eax,cl      ; EAX = 0rv0
    mov  ebx,TableC  ; EBX = Table conversion
    or   edx,eax     ; EDX = 0rvb
    xor  eax,eax
    mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

    ; On le place dans la destination
    mov  [edi],al

    ; On regarde la couleur de la palette
    mov  ebx,Palette
    add  ebx,eax
    shl  eax,1
    add  ebx,eax        ; EBX = &Palette[Table[R,V,B]].R
    xor  eax,eax
    xor  ecx,ecx
    xor  edx,edx
    mov   al,[ebx+0]    ; EAX = 000R
    mov   cl,[ebx+1]    ; ECX = 000V
    mov   dl,[ebx+2]    ; EDX = 000B

    ; On calcule l'erreur
    sub  eax,DSFRouge
    sub  ecx,DSFVert
    sub  edx,DSFBleu
    neg  eax
    neg  ecx
    neg  edx
    mov  DSFRouge,eax
    mov  DSFVert ,ecx
    mov  DSFBleu ,edx

    ; On initialise la quantit‚ d'erreur … diffuser
    mov  DSFRougeAD,eax
    mov  DSFVertAD ,ecx
    mov  DSFBleuAD ,edx

    ; On diffuse l'erreur sur le pixel de droite (6)
    mov  eax,DSFRouge
    mov  ebx,DSFVert
    mov  ecx,DSFBleu
    ; "Multiplication sign‚e par 7"
    sal  eax,3
    sal  ebx,3
    sal  ecx,3
    sub  eax,DSFRouge
    sub  ebx,DSFVert
    sub  ecx,DSFBleu
    ; Division sign‚e par 16
    sar  eax,4
    sar  ebx,4
    sar  ecx,4
    ; Mise … jour de la quantit‚ … diffuser
    sub  DSFRougeAD,eax
    sub  DSFVertAD ,ebx
    sub  DSFBleuAD ,ecx
    ; Diffusion de l'erreur dans la source
    add  esi,3
    add  al,[esi+0]
    adc  ah,0
    jz   DSF6123_6R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF6123_6R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF6123_6V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF6123_6V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF6123_6B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF6123_6B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On diffuse l'erreur sur le pixel en bas … gauche (1)
    mov  eax,DSFRouge
    mov  ebx,DSFVert
    mov  ecx,DSFBleu
    ; "Multiplication sign‚e par 3"
    sal  eax,1
    sal  ebx,1
    sal  ecx,1
    add  eax,DSFRouge
    add  ebx,DSFVert
    add  ecx,DSFBleu
    ; Division sign‚e par 16
    sar  eax,4
    sar  ebx,4
    sar  ecx,4
    ; Mise … jour de la quantit‚ … diffuser
    sub  DSFRougeAD,eax
    sub  DSFVertAD ,ebx
    sub  DSFBleuAD ,ecx
    ; Diffusion de l'erreur dans la source
    mov  edx,Largeur
    shl  edx,1
    add  edx,Largeur
    add  esi,edx
    add  al,[esi+0]
    adc  ah,0
    jz   DSF6123_1R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF6123_1R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF6123_1V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF6123_1V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF6123_1B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF6123_1B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On diffuse l'erreur sur le pixel en bas (2)
    mov  eax,DSFRouge
    mov  ebx,DSFVert
    mov  ecx,DSFBleu
    ; Division sign‚e par 4
    sar  eax,2
    sar  ebx,2
    sar  ecx,2
    ; Mise … jour de la quantit‚ … diffuser
    sub  DSFRougeAD,eax
    sub  DSFVertAD ,ebx
    sub  DSFBleuAD ,ecx
    ; Diffusion de l'erreur dans la source
    add  esi,3
    add  al,[esi+0]
    adc  ah,0
    jz   DSF6123_2R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF6123_2R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF6123_2V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF6123_2V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF6123_2B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF6123_2B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On diffuse l'erreur sur le pixel en bas … droite (3)
    mov  eax,DSFRougeAD
    mov  ebx,DSFVertAD
    mov  ecx,DSFBleuAD
    ; Diffusion de l'erreur dans la source
    add  esi,3
    add  al,[esi+0]
    adc  ah,0
    jz   DSF6123_3R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF6123_3R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF6123_3V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF6123_3V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF6123_3B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF6123_3B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On passe au pixel suivant
    sub  esi,edx
    sub  esi,6
    inc  edi
    dec  VarA
    jnz  DFS6123_Pour_chaque_pixel


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS_6123 endp



OPASM_DitherFS_623 proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,Largeur:dword,Palette:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
  mov  edx,[esi]   ; EDX = XBVR
  xor  eax,eax     ; EAX = 0000
  xor  ebx,ebx     ; EBX = 0000
  mov  al,dl       ; EAX = 000R
  mov  bl,dh       ; EBX = 000V
  shr  edx,16      ; EDX = 00XB
  mov  DSFRouge,eax
  xor  dh,dh       ; EDX = 000B
  mov  DSFVert ,ebx
  mov  DSFBleu ,edx
  mov  cl,ReducR
  shr  eax,cl      ; EAX = 000r
  mov  cl,ReducV
  shr  ebx,cl      ; EBX = 000v
  mov  cl,ReducB
  shr  edx,cl      ; EDX = 000b
  mov  cl,NbbV
  shl  eax,cl      ; EAX = 00r0
  or   eax,ebx     ; EAX = 00rv
  mov  cl,NbbB
  shl  eax,cl      ; EAX = 0rv0
  mov  ebx,TableC  ; EBX = Table conversion
  or   edx,eax     ; EDX = 0rvb
  xor  eax,eax
  mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

  ; On le place dans la destination
  mov  [edi],al

  ; On regarde la couleur de la palette
  mov  ebx,Palette
  add  ebx,eax
  shl  eax,1
  add  ebx,eax        ; EBX = &Palette[Table[R,V,B]].R
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov   al,[ebx+0]    ; EAX = 000R
  mov   cl,[ebx+1]    ; ECX = 000V
  mov   dl,[ebx+2]    ; EDX = 000B

  ; On calcule l'erreur
  sub  eax,DSFRouge
  sub  ecx,DSFVert
  sub  edx,DSFBleu
  neg  eax
  neg  ecx
  neg  edx
  mov  DSFRouge,eax
  mov  DSFVert ,ecx
  mov  DSFBleu ,edx

  ; On initialise la quantit‚ d'erreur … diffuser
  mov  DSFRougeAD,eax
  mov  DSFVertAD ,ecx
  mov  DSFBleuAD ,edx

  ; On diffuse l'erreur sur le pixel de droite (6)
  mov  eax,DSFRouge
  mov  ebx,DSFVert
  mov  ecx,DSFBleu
  ; "Multiplication sign‚e par 7"
  sal  eax,3
  sal  ebx,3
  sal  ecx,3
  sub  eax,DSFRouge
  sub  ebx,DSFVert
  sub  ecx,DSFBleu
  ; Division sign‚e par 16
  sar  eax,4
  sar  ebx,4
  sar  ecx,4
  ; Mise … jour de la quantit‚ … diffuser
  sub  DSFRougeAD,eax
  sub  DSFVertAD ,ebx
  sub  DSFBleuAD ,ecx
  ; Diffusion de l'erreur dans la source
  add  esi,3
  add  al,[esi+0]
  adc  ah,0
  jz   DSF623_6R_Pas_de_debordement
    sar  ax,16
    not  ax
  DSF623_6R_Pas_de_debordement:
  add  bl,[esi+1]
  adc  bh,0
  jz   DSF623_6V_Pas_de_debordement
    sar  bx,16
    not  bx
  DSF623_6V_Pas_de_debordement:
  add  cl,[esi+2]
  adc  ch,0
  jz   DSF623_6B_Pas_de_debordement
    sar  cx,16
    not  cx
  DSF623_6B_Pas_de_debordement:
  mov  [esi+0],al
  mov  [esi+1],bl
  mov  [esi+2],cl

  ; On diffuse l'erreur sur le pixel en bas … gauche (1)
  mov  eax,DSFRouge
  mov  ebx,DSFVert
  mov  ecx,DSFBleu
  ; "Multiplication sign‚e par 3"
  sal  eax,1
  sal  ebx,1
  sal  ecx,1
  add  eax,DSFRouge
  add  ebx,DSFVert
  add  ecx,DSFBleu
  ; Division sign‚e par 16
  sar  eax,4
  sar  ebx,4
  sar  ecx,4
  ; Mise … jour de la quantit‚ … diffuser
  sub  DSFRougeAD,eax
  sub  DSFVertAD ,ebx
  sub  DSFBleuAD ,ecx
  ; Diffusion de l'erreur dans la source
  mov  edx,Largeur
  shl  edx,1
  add  edx,Largeur
  add  esi,edx

  ; On diffuse l'erreur sur le pixel en bas (2)
  mov  eax,DSFRouge
  mov  ebx,DSFVert
  mov  ecx,DSFBleu
  ; Division sign‚e par 4
  sar  eax,2
  sar  ebx,2
  sar  ecx,2
  ; Mise … jour de la quantit‚ … diffuser
  sub  DSFRougeAD,eax
  sub  DSFVertAD ,ebx
  sub  DSFBleuAD ,ecx
  ; Diffusion de l'erreur dans la source
  add  esi,3
  add  al,[esi+0]
  adc  ah,0
  jz   DSF623_2R_Pas_de_debordement
    sar  ax,16
    not  ax
  DSF623_2R_Pas_de_debordement:
  add  bl,[esi+1]
  adc  bh,0
  jz   DSF623_2V_Pas_de_debordement
    sar  bx,16
    not  bx
  DSF623_2V_Pas_de_debordement:
  add  cl,[esi+2]
  adc  ch,0
  jz   DSF623_2B_Pas_de_debordement
    sar  cx,16
    not  cx
  DSF623_2B_Pas_de_debordement:
  mov  [esi+0],al
  mov  [esi+1],bl
  mov  [esi+2],cl

  ; On diffuse l'erreur sur le pixel en bas … droite (3)
  mov  eax,DSFRougeAD
  mov  ebx,DSFVertAD
  mov  ecx,DSFBleuAD
  ; Diffusion de l'erreur dans la source
  add  esi,3
  add  al,[esi+0]
  adc  ah,0
  jz   DSF623_3R_Pas_de_debordement
    sar  ax,16
    not  ax
  DSF623_3R_Pas_de_debordement:
  add  bl,[esi+1]
  adc  bh,0
  jz   DSF623_3V_Pas_de_debordement
    sar  bx,16
    not  bx
  DSF623_3V_Pas_de_debordement:
  add  cl,[esi+2]
  adc  ch,0
  jz   DSF623_3B_Pas_de_debordement
    sar  cx,16
    not  cx
  DSF623_3B_Pas_de_debordement:
  mov  [esi+0],al
  mov  [esi+1],bl
  mov  [esi+2],cl


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS_623 endp



OPASM_DitherFS_12 proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,Largeur:dword,Palette:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
  mov  edx,[esi]   ; EDX = XBVR
  xor  eax,eax     ; EAX = 0000
  xor  ebx,ebx     ; EBX = 0000
  mov  al,dl       ; EAX = 000R
  mov  bl,dh       ; EBX = 000V
  shr  edx,16      ; EDX = 00XB
  mov  DSFRouge,eax
  xor  dh,dh       ; EDX = 000B
  mov  DSFVert ,ebx
  mov  DSFBleu ,edx
  mov  cl,ReducR
  shr  eax,cl      ; EAX = 000r
  mov  cl,ReducV
  shr  ebx,cl      ; EBX = 000v
  mov  cl,ReducB
  shr  edx,cl      ; EDX = 000b
  mov  cl,NbbV
  shl  eax,cl      ; EAX = 00r0
  or   eax,ebx     ; EAX = 00rv
  mov  cl,NbbB
  shl  eax,cl      ; EAX = 0rv0
  mov  ebx,TableC  ; EBX = Table conversion
  or   edx,eax     ; EDX = 0rvb
  xor  eax,eax
  mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

  ; On le place dans la destination
  mov  [edi],al

  ; On regarde la couleur de la palette
  mov  ebx,Palette
  add  ebx,eax
  shl  eax,1
  add  ebx,eax        ; EBX = &Palette[Table[R,V,B]].R
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov   al,[ebx+0]    ; EAX = 000R
  mov   cl,[ebx+1]    ; ECX = 000V
  mov   dl,[ebx+2]    ; EDX = 000B

  ; On calcule l'erreur
  sub  eax,DSFRouge
  sub  ecx,DSFVert
  sub  edx,DSFBleu
  neg  eax
  neg  ecx
  neg  edx
  mov  DSFRouge,eax
  mov  DSFVert ,ecx
  mov  DSFBleu ,edx

  ; On diffuse l'erreur sur le pixel en bas … gauche (1)
  mov  eax,DSFRouge
  mov  ebx,DSFVert
  mov  ecx,DSFBleu
  ; "Multiplication sign‚e par 3"
  sal  eax,1
  sal  ebx,1
  sal  ecx,1
  add  eax,DSFRouge
  add  ebx,DSFVert
  add  ecx,DSFBleu
  ; Division sign‚e par 16
  sar  eax,4
  sar  ebx,4
  sar  ecx,4
  ; Diffusion de l'erreur dans la source
  mov  edx,Largeur
  add  esi,3
  shl  edx,1
  add  edx,Largeur
  add  esi,edx
  add  al,[esi+0]
  adc  ah,0
  jz   DSF12_1R_Pas_de_debordement
    sar  ax,16
    not  ax
  DSF12_1R_Pas_de_debordement:
  add  bl,[esi+1]
  adc  bh,0
  jz   DSF12_1V_Pas_de_debordement
    sar  bx,16
    not  bx
  DSF12_1V_Pas_de_debordement:
  add  cl,[esi+2]
  adc  ch,0
  jz   DSF12_1B_Pas_de_debordement
    sar  cx,16
    not  cx
  DSF12_1B_Pas_de_debordement:
  mov  [esi+0],al
  mov  [esi+1],bl
  mov  [esi+2],cl

  ; On diffuse l'erreur sur le pixel en bas (2)
  mov  eax,DSFRouge
  mov  ebx,DSFVert
  mov  ecx,DSFBleu
  ; Division sign‚e par 4
  sar  eax,2
  sar  ebx,2
  sar  ecx,2
  ; Diffusion de l'erreur dans la source
  add  esi,3
  add  al,[esi+0]
  adc  ah,0
  jz   DSF12_2R_Pas_de_debordement
    sar  ax,16
    not  ax
  DSF12_2R_Pas_de_debordement:
  add  bl,[esi+1]
  adc  bh,0
  jz   DSF12_2V_Pas_de_debordement
    sar  bx,16
    not  bx
  DSF12_2V_Pas_de_debordement:
  add  cl,[esi+2]
  adc  ch,0
  jz   DSF12_2B_Pas_de_debordement
    sar  cx,16
    not  cx
  DSF12_2B_Pas_de_debordement:
  mov  [esi+0],al
  mov  [esi+1],bl
  mov  [esi+2],cl


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS_12 endp



OPASM_DitherFS_6 proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,Largeur:dword,Palette:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On place dans VarA la largeur … traiter
  mov  eax,Largeur
  mov  VarA,eax

  ; Pour chacun des pixels sur la largeur:
  DFS6_Pour_chaque_pixel:

    ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
    mov  edx,[esi]   ; EDX = XBVR
    xor  eax,eax     ; EAX = 0000
    xor  ebx,ebx     ; EBX = 0000
    mov  al,dl       ; EAX = 000R
    mov  bl,dh       ; EBX = 000V
    shr  edx,16      ; EDX = 00XB
    mov  DSFRouge,eax
    xor  dh,dh       ; EDX = 000B
    mov  DSFVert ,ebx
    mov  DSFBleu ,edx
    mov  cl,ReducR
    shr  eax,cl      ; EAX = 000r
    mov  cl,ReducV
    shr  ebx,cl      ; EBX = 000v
    mov  cl,ReducB
    shr  edx,cl      ; EDX = 000b
    mov  cl,NbbV
    shl  eax,cl      ; EAX = 00r0
    or   eax,ebx     ; EAX = 00rv
    mov  cl,NbbB
    shl  eax,cl      ; EAX = 0rv0
    mov  ebx,TableC  ; EBX = Table conversion
    or   edx,eax     ; EDX = 0rvb
    xor  eax,eax
    mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

    ; On le place dans la destination
    mov  [edi],al

    ; On regarde la couleur de la palette
    mov  ebx,Palette
    add  ebx,eax
    shl  eax,1
    add  ebx,eax        ; EBX = &Palette[Table[R,V,B]].R
    xor  eax,eax
    xor  ecx,ecx
    xor  edx,edx
    mov   al,[ebx+0]    ; EAX = 000R
    mov   cl,[ebx+1]    ; ECX = 000V
    mov   dl,[ebx+2]    ; EDX = 000B

    ; On calcule l'erreur
    sub  eax,DSFRouge
    sub  ecx,DSFVert
    sub  edx,DSFBleu
    neg  eax
    neg  ecx
    neg  edx
    mov  DSFRouge,eax
    mov  DSFVert ,ecx
    mov  DSFBleu ,edx

    ; On diffuse l'erreur sur le pixel de droite (6)
    mov  eax,DSFRouge
    mov  ebx,DSFVert
    mov  ecx,DSFBleu
    ; "Multiplication sign‚e par 7"
    sal  eax,3
    sal  ebx,3
    sal  ecx,3
    sub  eax,DSFRouge
    sub  ebx,DSFVert
    sub  ecx,DSFBleu
    ; Division sign‚e par 16
    sar  eax,4
    sar  ebx,4
    sar  ecx,4
    ; Diffusion de l'erreur dans la source
    add  esi,3
    add  al,[esi+0]
    adc  ah,0
    jz   DSF6_6R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF6_6R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF6_6V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF6_6V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF6_6B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF6_6B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On passe au pixel suivant
    inc  edi
    dec  VarA
    jnz  DFS6_Pour_chaque_pixel


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS_6 endp



OPASM_DitherFS proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
  mov  edx,[esi]   ; EDX = XBVR
  xor  eax,eax     ; EAX = 0000
  xor  ebx,ebx     ; EBX = 0000
  mov  al,dl       ; EAX = 000R
  mov  bl,dh       ; EBX = 000V
  shr  edx,16      ; EDX = 00XB
  mov  DSFRouge,eax
  xor  dh,dh       ; EDX = 000B
  mov  DSFVert ,ebx
  mov  DSFBleu ,edx
  mov  cl,ReducR
  shr  eax,cl      ; EAX = 000r
  mov  cl,ReducV
  shr  ebx,cl      ; EBX = 000v
  mov  cl,ReducB
  shr  edx,cl      ; EDX = 000b
  mov  cl,NbbV
  shl  eax,cl      ; EAX = 00r0
  or   eax,ebx     ; EAX = 00rv
  mov  cl,NbbB
  shl  eax,cl      ; EAX = 0rv0
  mov  ebx,TableC  ; EBX = Table conversion
  or   edx,eax     ; EDX = 0rvb
  xor  eax,eax
  mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

  ; On le place dans la destination
  mov  [edi],al


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS endp



OPASM_DitherFS_2 proc near

  push ebp
  mov  ebp,esp

  arg  Destination:dword,Source:dword,Hauteur:dword,Palette:dword,TableC:dword,ReducR:byte,ReducV:byte,ReducB:byte,NbbV:byte,NbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,Source
  ; On place dans EDI l'adresse de la destination
  mov edi,Destination

  ; On place dans VarA la hauteur … traiter
  mov  eax,Hauteur
  mov  VarA,eax

  ; Pour chacun des pixels sur la largeur:
  DFS2_Pour_chaque_pixel:

    ; On regarde la meilleure couleur d'aprŠs la table de conversion 24b->8b
    mov  edx,[esi]   ; EDX = XBVR
    xor  eax,eax     ; EAX = 0000
    xor  ebx,ebx     ; EBX = 0000
    mov  al,dl       ; EAX = 000R
    mov  bl,dh       ; EBX = 000V
    shr  edx,16      ; EDX = 00XB
    mov  DSFRouge,eax
    xor  dh,dh       ; EDX = 000B
    mov  DSFVert ,ebx
    mov  DSFBleu ,edx
    mov  cl,ReducR
    shr  eax,cl      ; EAX = 000r
    mov  cl,ReducV
    shr  ebx,cl      ; EBX = 000v
    mov  cl,ReducB
    shr  edx,cl      ; EDX = 000b
    mov  cl,NbbV
    shl  eax,cl      ; EAX = 00r0
    or   eax,ebx     ; EAX = 00rv
    mov  cl,NbbB
    shl  eax,cl      ; EAX = 0rv0
    mov  ebx,TableC  ; EBX = Table conversion
    or   edx,eax     ; EDX = 0rvb
    xor  eax,eax
    mov  al,[ebx+edx]   ; EAX = Table[R,V,B]

    ; On le place dans la destination
    mov  [edi],al

    ; On regarde la couleur de la palette
    mov  ebx,Palette
    add  ebx,eax
    shl  eax,1
    add  ebx,eax        ; EBX = &Palette[Table[R,V,B]].R
    xor  eax,eax
    xor  ecx,ecx
    xor  edx,edx
    mov   al,[ebx+0]    ; EAX = 000R
    mov   cl,[ebx+1]    ; ECX = 000V
    mov   dl,[ebx+2]    ; EDX = 000B

    ; On calcule l'erreur
    sub  eax,DSFRouge
    sub  ecx,DSFVert
    sub  edx,DSFBleu
    neg  eax
    neg  ecx
    neg  edx
    mov  DSFRouge,eax
    mov  DSFVert ,ecx
    mov  DSFBleu ,edx

    ; On diffuse l'erreur sur le pixel en bas (2)
    mov  eax,DSFRouge
    mov  ebx,DSFVert
    mov  ecx,DSFBleu
    ; Division sign‚e par 4
    sar  eax,2
    sar  ebx,2
    sar  ecx,2
    ; Diffusion de l'erreur dans la source
    add  esi,3
    add  al,[esi+0]
    adc  ah,0
    jz   DSF2_2R_Pas_de_debordement
      sar  ax,16
      not  ax
    DSF2_2R_Pas_de_debordement:
    add  bl,[esi+1]
    adc  bh,0
    jz   DSF2_2V_Pas_de_debordement
      sar  bx,16
      not  bx
    DSF2_2V_Pas_de_debordement:
    add  cl,[esi+2]
    adc  ch,0
    jz   DSF2_2B_Pas_de_debordement
      sar  cx,16
      not  cx
    DSF2_2B_Pas_de_debordement:
    mov  [esi+0],al
    mov  [esi+1],bl
    mov  [esi+2],cl

    ; On passe au pixel suivant
    inc  edi
    dec  VarA
    jnz  DFS2_Pour_chaque_pixel


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_DitherFS_2 endp



OPASM_Compter_occurences proc near

  push ebp
  mov  ebp,esp

  arg  destination:dword,source:dword,taille:dword,reducR:byte,reducV:byte,reducB:byte,nbbV:byte,nbbB:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la source
  mov esi,source
  ; On place dans EDI l'adresse de la destination
  mov edi,destination

  ; On place dans VarA le nb de pixels … traiter
  mov  eax,taille
  mov  VarA,eax

  ; Pour chacun des pixels:
  CO_Pour_chaque_pixel:

    ; On regarde la couleur
    xor  eax,eax     ; EAX = 0000
    xor  ebx,ebx     ; EBX = 0000
    xor  edx,edx     ; EDX = 0000
    mov   al,[esi]   ; EAX = 000R
    mov   dx,[esi+1] ; EDX = 00BV
    mov   cl,reducR
    xchg  bl,dh      ; EBX = 000B EDX = 000V
    shr  eax,cl      ; EAX = 000r
    mov   cl,reducV
    shr  edx,cl      ; EDX = 000v
    mov   cl,reducB
    shr  ebx,cl      ; EBX = 000b
    mov   cl,nbbV
    shl  eax,cl      ; EAX = 00r0
    or   eax,edx     ; EAX = 00rv
    mov   cl,nbbB
    shl  eax,cl      ; EAX = 0rv0
    or   eax,ebx     ; EAX = 0rvb
    shl  eax,2       ; EAX = 0rvb * 4
    inc  dword ptr[edi+eax]

    ; On passe au pixel suivant
    add  esi,3
    dec  VarA
    jnz  CO_Pour_chaque_pixel


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_Compter_occurences endp




OPASM_Analyser_cluster proc near

  push ebp
  mov  ebp,esp

  arg  tableO:dword,rmin:dword,vmin:dword,bmin:dword,rmax:dword,vmax:dword,bmax:dword,rdec:dword,vdec:dword,bdec:dword,rinc:dword,vinc:dword,binc:dword,nbocc:dword

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de base de la table d'occurence
  mov  esi,tableO

  ; On place dans DSF????? la valeur minimale des teintes
  mov  eax,[rmin]
  mov  ebx,[vmin]
  mov  ecx,[bmin]
  mov  eax,[eax]
  mov  ebx,[ebx]
  mov  ecx,[ecx]
  mov  DSFRouge,eax
  mov  DSFVert ,ebx
  mov  DSFBleu ,ecx
  ; On place dans VarDEF la valeur maximale courante des teintes
  mov  VarD,eax
  mov  VarE,ebx
  mov  VarF,ecx
  ; On place dans DSF?????AD la valeur maximale des teintes
  mov  eax,[rmax]
  mov  ebx,[vmax]
  mov  ecx,[bmax]
  mov  eax,[eax]
  mov  ebx,[ebx]
  mov  ecx,[ecx]
  mov  DSFRougeAD,eax
  mov  DSFVertAD ,ebx
  mov  DSFBleuAD ,ecx
  ; On place dans VarABC la valeur minimale courante des teintes
  mov  VarA,eax
  mov  VarB,ebx
  mov  VarC,ecx

  ; On place dans EDI le nombre d'occurences
  xor  edi,edi

  ; Pour chaque Rouge
  mov  ebx,DSFRouge
  AC_Pour_chaque_rouge:

    ; Pour chaque Vert
    mov  ecx,DSFVert
    AC_Pour_chaque_vert:

      ; Pour chaque Bleu
      mov  edx,DSFBleu
      AC_Pour_chaque_bleu:

        ; On regarde le nombre d'occurences (=>EAX)
        mov  eax,ebx
        or   eax,ecx
        or   eax,edx
        shl  eax,2
        mov  eax,[esi+eax]

        ; S'il y a des occurences
        or   eax,eax
        jz   AC_Pas_d_occurences

          ; On incr‚mente le compteur
          add  edi,eax

          ; On met … jour les bornes
          cmp  ebx,VarA       ; r ? rmin
          jae  AC_r_sup_rmin
            mov  VarA,ebx
          AC_r_sup_rmin:
          cmp  ebx,VarD       ; r ? rmax
          jbe  AC_r_inf_rmax
            mov  VarD,ebx
          AC_r_inf_rmax:
          cmp  ecx,VarB       ; v ? vmin
          jae  AC_v_sup_vmin
            mov  VarB,ecx
          AC_v_sup_vmin:
          cmp  ecx,VarE       ; v ? vmax
          jbe  AC_v_inf_vmax
            mov  VarE,ecx
          AC_v_inf_vmax:
          cmp  edx,VarC       ; b ? bmin
          jae  AC_b_sup_bmin
            mov  VarC,edx
          AC_b_sup_bmin:
          cmp  edx,VarF       ; b ? bmax
          jbe  AC_b_inf_bmax
            mov  VarF,edx
          AC_b_inf_bmax:

        AC_Pas_d_occurences:

        ; On passe au bleu suivant
        add  edx,binc
        cmp  edx,DSFBleuAD
        jbe  AC_Pour_chaque_bleu

      ; On passe au vert suivant
      add  ecx,vinc
      cmp  ecx,DSFVertAD
      jbe  AC_Pour_chaque_vert

    ; On passe au rouge suivant
    add  ebx,rinc
    cmp  ebx,DSFRougeAD
    jbe  AC_Pour_chaque_rouge

  ; On retourne les r‚sultats
   ; Nb d'occurences
  mov  eax,nbocc
  mov  [eax],edi
   ; Rmin & Rmax
  mov  eax,VarA
  mov  ebx,VarD
  mov  ecx,rdec
  mov  edx,rmin
  mov  edi,rmax
  shr  eax,cl
  shr  ebx,cl
  mov  [edx],eax
  mov  [edi],ebx
   ; Vmin & Vmax
  mov  eax,VarB
  mov  ebx,VarE
  mov  ecx,vdec
  mov  edx,vmin
  mov  edi,vmax
  shr  eax,cl
  shr  ebx,cl
  mov  [edx],eax
  mov  [edi],ebx
   ; Bmin & Bmax
  mov  eax,VarC
  mov  ebx,VarF
  mov  ecx,bdec
  mov  edx,bmin
  mov  edi,bmax
  shr  eax,cl
  shr  ebx,cl
  mov  [edx],eax
  mov  [edi],ebx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_Analyser_cluster endp




OPASM_Split_cluster_Rouge proc near

  push ebp
  mov  ebp,esp

  arg  tableO:dword,rmin:dword,vmin:dword,bmin:dword,rmax:dword,vmax:dword,bmax:dword,rinc:dword,vinc:dword,binc:dword,limite:dword,rdec:dword,rouge:dword

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de base de la table d'occurence
  mov  esi,tableO

  ; On place dans EDI le nombre d'occurences
  xor  edi,edi

  ; Pour chaque Rouge
  mov  ebx,rmin
  SCR_Pour_chaque_rouge:

    ; Pour chaque Vert
    mov  ecx,vmin
    SCR_Pour_chaque_vert:

      ; Pour chaque Bleu
      mov  edx,bmin
      SCR_Pour_chaque_bleu:

        ; On regarde le nombre d'occurences (=>EAX)
        mov  eax,ebx
        or   eax,ecx
        or   eax,edx
        shl  eax,2
        mov  eax,[esi+eax]

        ; On le rajoute … EDI
        add  edi,eax
        cmp  edi,limite
        jae  SCR_Fin_recherche

        ; On passe au bleu suivant
        add  edx,binc
        cmp  edx,bmax
        jbe  SCR_Pour_chaque_bleu

      ; On passe au vert suivant
      add  ecx,vinc
      cmp  ecx,vmax
      jbe  SCR_Pour_chaque_vert

    ; On passe au rouge suivant
    add  ebx,rinc
    cmp  ebx,rmax
    jbe  SCR_Pour_chaque_rouge

  SCR_Fin_recherche:

  ; On retourne le r‚sultat
  mov  ecx,rdec
  shr  ebx,cl
  mov  eax,rouge
  mov  [eax],ebx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_Split_cluster_Rouge endp



OPASM_Split_cluster_Vert proc near

  push ebp
  mov  ebp,esp

  arg  tableO:dword,rmin:dword,vmin:dword,bmin:dword,rmax:dword,vmax:dword,bmax:dword,rinc:dword,vinc:dword,binc:dword,limite:dword,vdec:dword,vert:dword

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de base de la table d'occurence
  mov  esi,tableO

  ; On place dans EDI le nombre d'occurences
  xor  edi,edi

  ; Pour chaque Vert
  mov  ebx,vmin
  SCV_Pour_chaque_vert:

    ; Pour chaque Rouge
    mov  ecx,rmin
    SCV_Pour_chaque_rouge:

      ; Pour chaque Bleu
      mov  edx,bmin
      SCV_Pour_chaque_bleu:

        ; On regarde le nombre d'occurences (=>EAX)
        mov  eax,ebx
        or   eax,ecx
        or   eax,edx
        shl  eax,2
        mov  eax,[esi+eax]

        ; On le rajoute … EDI
        add  edi,eax
        cmp  edi,limite
        jae  SCV_Fin_recherche

        ; On passe au bleu suivant
        add  edx,binc
        cmp  edx,bmax
        jbe  SCV_Pour_chaque_bleu

      ; On passe au rouge suivant
      add  ecx,rinc
      cmp  ecx,rmax
      jbe  SCV_Pour_chaque_rouge

    ; On passe au vert suivant
    add  ebx,vinc
    cmp  ebx,vmax
    jbe  SCV_Pour_chaque_vert

  SCV_Fin_recherche:

  ; On retourne le r‚sultat
  mov  ecx,vdec
  shr  ebx,cl
  mov  eax,vert
  mov  [eax],ebx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_Split_cluster_Vert endp



OPASM_Split_cluster_Bleu proc near

  push ebp
  mov  ebp,esp

  arg  tableO:dword,rmin:dword,vmin:dword,bmin:dword,rmax:dword,vmax:dword,bmax:dword,rinc:dword,vinc:dword,binc:dword,limite:dword,bdec:dword,bleu:dword

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de base de la table d'occurence
  mov  esi,tableO

  ; On place dans EDI le nombre d'occurences
  xor  edi,edi

  ; Pour chaque Bleu
  mov  ebx,bmin
  SCB_Pour_chaque_bleu:

    ; Pour chaque Rouge
    mov  ecx,rmin
    SCB_Pour_chaque_rouge:

      ; Pour chaque Vert
      mov  edx,vmin
      SCB_Pour_chaque_vert:

        ; On regarde le nombre d'occurences (=>EAX)
        mov  eax,ebx
        or   eax,ecx
        or   eax,edx
        shl  eax,2
        mov  eax,[esi+eax]

        ; On le rajoute … EDI
        add  edi,eax
        cmp  edi,limite
        jae  SCB_Fin_recherche

        ; On passe au vert suivant
        add  edx,vinc
        cmp  edx,vmax
        jbe  SCB_Pour_chaque_vert

      ; On passe au rouge suivant
      add  ecx,rinc
      cmp  ecx,rmax
      jbe  SCB_Pour_chaque_rouge

    ; On passe au bleu suivant
    add  ebx,binc
    cmp  ebx,bmax
    jbe  SCB_Pour_chaque_bleu

  SCB_Fin_recherche:

  ; On retourne le r‚sultat
  mov  ecx,bdec
  shr  ebx,cl
  mov  eax,bleu
  mov  [eax],ebx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

OPASM_Split_cluster_Bleu endp




_TEXT ENDS
END
