.386P
.MODEL FLAT

_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA

; -- Fonctions MCGA --
public Set_mode_MCGA
public Pixel_MCGA
public Lit_pixel_MCGA
public Effacer_tout_l_ecran_MCGA
public Block_MCGA
public Pixel_Preview_Normal_MCGA
public Pixel_Preview_Loupe_MCGA
public Ligne_horizontale_XOR_MCGA
public Ligne_verticale_XOR_MCGA
; -- Fonctions Mode X --
public Set_mode_X
public Pixel_mode_X
public Lit_pixel_mode_X
public Effacer_tout_l_ecran_mode_X
public Block_mode_X
public Pixel_Preview_Normal_mode_X
public Pixel_Preview_Loupe_mode_X
public Ligne_horizontale_XOR_mode_X
public Ligne_verticale_XOR_mode_X
; -- Fonctions VESA --
public VESA_Change_banque_Fenetre_A
public VESA_Change_banque_Fenetre_B
public VESA_Change_banque_Fenetre_A_et_B
public VESA_Change_banque_Fenetre_A_FAST
public VESA_Change_banque_Fenetre_B_FAST
public VESA_Change_banque_Fenetre_A_et_B_FAST
;public Support_VESA
;public Mode_VESA_supporte
public Initialiser_mode_video_VESA
public Retoucher_CRTC
public Pixel_VESA
public Lit_pixel_VESA
public Effacer_tout_l_ecran_VESA
public Block_VESA
public Pixel_Preview_Normal_VESA
public Pixel_Preview_Loupe_VESA
public Ligne_horizontale_XOR_VESA
public Ligne_verticale_XOR_VESA

public Display_brush_Color_MCGA
public Display_brush_Mono_MCGA
public Clear_brush_MCGA

public Display_brush_Color_VESA
public Display_brush_Mono_VESA
public Clear_brush_VESA

public Display_brush_Color_mode_X
public Display_brush_Mono_mode_X
public Clear_brush_mode_X

public Remap_screen_MCGA
public Remap_screen_mode_X
public Remap_screen_VESA

public Afficher_partie_de_l_ecran_MCGA
public Afficher_une_ligne_a_l_ecran_MCGA
public Lire_une_ligne_a_l_ecran_MCGA
public Afficher_partie_de_l_ecran_zoomee_MCGA
public Afficher_partie_de_l_ecran_mode_X
public Afficher_une_ligne_a_l_ecran_mode_X
public Lire_une_ligne_a_l_ecran_mode_X
public Afficher_partie_de_l_ecran_zoomee_mode_X
public Afficher_partie_de_l_ecran_VESA
public Afficher_une_ligne_a_l_ecran_VESA
public Lire_une_ligne_a_l_ecran_VESA
public Afficher_partie_de_l_ecran_zoomee_VESA

public Display_brush_Color_Zoom_MCGA
public Display_brush_Mono_Zoom_MCGA
public Clear_brush_Zoom_MCGA
public Display_brush_Color_Zoom_mode_X
public Display_brush_Mono_Zoom_mode_X
public Clear_brush_Zoom_mode_X
public Display_brush_Color_Zoom_VESA
public Display_brush_Mono_Zoom_VESA
public Clear_brush_Zoom_VESA


; -- Variables externes relatives au Mode X --
extrn Mode_X_Ptr   :dword
extrn MODE_X_Decalage_synchro      :dword
extrn MODE_X_Largeur_de_ligne      :word
extrn MODE_X_Valeur_initiale_de_esi:dword
extrn MODE_X_Valeur_initiale_de_edi:dword

; -- Variables externes relatives au VESA --
extrn Granularite  :byte
extrn Mode_X_Ptr   :dataptr     ; Table de modification des registres du CRTC
extrn VESA_Erreur                  :byte
extrn VESA_WinFuncPtr              :dword
extrn VESA_Liste_des_modes         :dword
extrn VESA_Decalage_synchro        :dword
extrn VESA_Largeur_ecran_en_dword  :word
extrn VESA_Banque_en_cours         :byte
extrn VESA_Derniere_banque_Fenetre_A_utilisee:byte
extrn VESA_Derniere_banque_Fenetre_B_utilisee:byte
extrn VESA_Change_banque_lecture   :dword
extrn VESA_Change_banque_ecriture  :dword
extrn VESA_Change_banque_lect_ecr  :dword
extrn VESA_Version_Unite           :byte
extrn VESA_Version_Decimale        :byte
extrn VESA_Constructeur            :dword
extrn VESA_Taille_memoire          :word

; -- Autres variables externes --
extrn Brosse:dword
extrn Principal_Ecran:dword
extrn Principal_Largeur_image:word
extrn Principal_Hauteur_image:word
extrn Principal_Decalage_X:word
extrn Principal_Decalage_Y:word
extrn Largeur_ecran:word
extrn Hauteur_ecran:word
extrn Menu_Ordonnee:word
extrn Principal_X_Zoom:word

extrn Table_mul_facteur_zoom:dataptr ; Table de multiplication par le facteur
extrn Loupe_Decalage_X:word
extrn Loupe_Decalage_Y:word
extrn Loupe_Facteur   :word

extrn Limite_Haut_Zoom          :word
extrn Limite_Gauche_Zoom        :word
extrn Limite_visible_Bas_Zoom   :word
extrn Limite_visible_Droite_Zoom:word

; -- Proc‚dures externes --
extrn Zoomer_une_ligne:near



; ***************************************************************************
; ***************************************************************************
; ********************************** MCGA ***********************************
; ***************************************************************************
; ***************************************************************************


Set_mode_MCGA proc near

  pushad


  mov   ax,13h
  int   10h


  popad

  ret

Set_mode_MCGA endp



Pixel_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  mov  eax,0A0140h
  mov  cl,Couleur
  mul  Y
  add  ax,X
  mov  [eax],cl


  mov  esp,ebp
  pop  ebp

  ret

Pixel_MCGA endp




Lit_pixel_MCGA proc near

  push  ebp
  mov   ebp,esp

  arg   X:word,Y:word


  mov   eax,0A0140h
  mul   Y
  add   ax,X
  mov   al,[eax]


  mov   esp,ebp
  pop   ebp

  ret

Lit_pixel_MCGA endp




Effacer_tout_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte

  push edi


  ; On place EDI en d‚but d'‚cran:

  mov  edi,0A0000h

  ; On met (E)CX … 320*Menu_Ordonnee/4:   ( *80 <=> <<6 + <<4)

  xor  ecx,ecx
  mov  ax,Menu_Ordonnee
  mov  cx,ax
  shl  ax,6
  shl  cx,4
  add  cx,ax

  ; On met dans EAX 4 fois la couleur:

  mov  dl,Couleur
  mov  dh,dl
  mov  ax,dx
  shl  eax,16
  mov  ax,dx


  rep  stosd

  pop  edi

  mov  esp,ebp
  pop  ebp

  ret

Effacer_tout_l_ecran_MCGA endp




Block_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Debut_X:word, Debut_Y:word, Largeur:word, Hauteur:word, Couleur:byte

  push esi
  push edi


  ; On met dans EDI la position de d‚part du dessin de block

  mov  edi,0A0000h
  mov  ax,Debut_Y
  mov  dx,ax
  shl  ax,8
  add  ax,Debut_X
  shl  dx,6
  add  ax,dx
  mov  di,ax

  ; On met dans SI l'incr‚mentation de DI … chaque ligne

  mov  si,320
  sub  si,Largeur

  ; On remplit EAX avec 4 fois la couleur

  mov  dl,Couleur
  mov  dh,dl
  mov  ax,dx
  shl  eax,16
  mov  ax,dx

  mov  dx,Hauteur

  xor  ecx,ecx

  Block_MCGA_Pour_chaque_ligne:

    mov  cx,Largeur                  ; CX = Nb pixels … afficher sur la ligne

    shr  cx,1
    jnc  Block_MCGA_Pair
    stosb                        ; On commence par se d‚barrasser du 1er byte
				 ; en cas d'imparit‚}

    Block_MCGA_Pair:

    shr  cx,1
    jnc  Block_MCGA_test_de_superiorite_a_3
    stosw                        ; On se d‚barrasse des 2Šme et 3Šme bytes en
				 ; cas de "non-multiplicit‚-de-4"
    Block_MCGA_test_de_superiorite_a_3:

    ; copie 32 bits
    rep  stosd

    add  di,si
    dec  dx

  jnz  Block_MCGA_Pour_chaque_ligne


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Block_MCGA endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode normal (pas en mode loupe) --

; Note: si on modifie cette proc‚dure, il faudra penser … faire ‚galement la
;       modif dans la proc‚dure Pixel_Preview_Loupe_MCGA.

Pixel_Preview_Normal_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  mov  dl,Couleur
  mov  cx,Y
  mov  ax,X
  sub  cx,Principal_Decalage_Y
  sub  ax,Principal_Decalage_X
  push edx
  push ecx
  push eax

  call Pixel_MCGA


  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Normal_MCGA endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode loupe --

Pixel_Preview_Loupe_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx


  ; On affiche d'abord le pixel dans la partie non-zoom‚e

  mov  dl,Couleur               ; Ca ne co–te pas trop
  mov  cx,Y                     ; en code de recopier
  mov  ax,X                     ; le contenu de la
  sub  cx,Principal_Decalage_Y  ; proc‚dure
  sub  ax,Principal_Decalage_X  ; Pixel_Preview_Normal_MCGA
  push edx                      ; et ‡a fait gagner du
  push ecx                      ; temps, et ce temps est
  push eax                      ; assez pr‚cieux vu que
  call Pixel_MCGA               ; c'est appel‚ TRES souvent.
  add  esp,12  ; 3 paramŠtres dword

  ; On regarde si on peut afficher le pixel ‚galement dans la loupe

  mov  ax,Y
  cmp  ax,Limite_Haut_Zoom
  jb   Pixel_Preview_Loupe_MCGA_Fin
  cmp  ax,Limite_visible_Bas_Zoom
  ja   Pixel_Preview_Loupe_MCGA_Fin
  mov  ax,X
  cmp  ax,Limite_Gauche_Zoom
  jb   Pixel_Preview_Loupe_MCGA_Fin
  cmp  ax,Limite_visible_Droite_Zoom
  ja   Pixel_Preview_Loupe_MCGA_Fin

  ; On passe la couleur

  mov  al,Couleur
  mov  edx,Table_mul_facteur_zoom       ; On pointe sur la table de x (OPTI)
  push eax

  ; On calcule dans EAX (X) et ECX (Y) le d‚marrage du bloc:

  xor  eax,eax
  xor  ecx,ecx
  mov  ax,X
  mov  cx,Y
  sub  ax,Loupe_Decalage_X
  sub  cx,Loupe_Decalage_Y
  mov  ax,[edx+2*eax]
  mov  cx,[edx+2*ecx]

  ; On sauve ces valeurs dans BX (X) et DX (Y):

  mov  bx,ax
  mov  dx,cx

  ; On s'occupe de la hauteur:

  add  cx,Loupe_Facteur
  cmp  cx,Menu_Ordonnee
  ja   Pixel_Preview_Loupe_MCGA_Tronque_en_hauteur

  mov  cx,Loupe_Facteur
  push ecx
  jmp  Pixel_Preview_Loupe_MCGA_Traitement_de_la_largeur

  Pixel_Preview_Loupe_MCGA_Tronque_en_hauteur:

  mov  cx,dx
  neg  cx
  add  cx,Menu_Ordonnee
  push ecx

  ; On s'occupe de la largeur:

  Pixel_Preview_Loupe_MCGA_Traitement_de_la_largeur:

  add  bx,Principal_X_Zoom

  mov  ax,Loupe_Facteur
  push eax

  ; On passe le d‚but en Y:
  push edx

  ; On passe le d‚but en X:
  push ebx

  call Block_MCGA


  add  esp,20                   ; 20 = 5 paramŠtres de type dword
  pop  ebx

  Pixel_Preview_Loupe_MCGA_Fin:

  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Loupe_MCGA endp




Ligne_horizontale_XOR_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Largeur:word

  push edi


  ; On met dans EDI la position de d‚part du dessin de la ligne

  mov  edi,0A0000h
  mov  ax,Pos_Y
  mov  dx,ax
  shl  ax,8
  add  ax,Pos_X
  shl  dx,6
  add  ax,dx
  mov  di,ax


  mov  cx,Largeur             ; CX = Nb pixels … afficher sur la ligne

  shr  cx,1
  jnc  LH_MCGA_Pair
  not  byte ptr[edi]           ; On commence par se d‚barrasser du 1er byte
  inc  edi                     ; en cas d'imparit‚


  LH_MCGA_Pair:

  shr  cx,1
  jnc  LH_MCGA_test_de_superiorite_a_3
  not  word ptr[edi]           ; On se d‚barrasse des 2Šme et 3Šme bytes en
  add  edi,2                   ; cas de "non-multiplicit‚-de-4"

  LH_MCGA_test_de_superiorite_a_3:

  or   cx,cx
  jz   LH_MCGA_Fin

  ; copie 32 bits
  LH_MCGA_Copie_32_bits:
    not  dword ptr[edi]
    add  edi,4
    dec  cx
  jnz  LH_MCGA_Copie_32_bits

  LH_MCGA_Fin:


  pop  edi

  mov  esp,ebp
  pop  ebp

  ret

Ligne_horizontale_XOR_MCGA endp




Ligne_verticale_XOR_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Hauteur:word


  ; On met dans EDX la position de d‚part du dessin de la ligne

  mov  edx,0A0000h
  mov  ax,Pos_Y
  mov  cx,ax
  shl  ax,8
  add  ax,Pos_X
  shl  cx,6
  add  ax,cx
  mov  dx,ax


  mov  cx,Hauteur             ; CX = Nb pixels … afficher sur la ligne

  LV_MCGA_Boucle:

    not  byte ptr[edx]
    add  dx,320

    dec  cx
    jnz  LV_MCGA_Boucle


  mov  esp,ebp
  pop  ebp

  ret

Ligne_verticale_XOR_MCGA endp




; ***************************************************************************
; ***************************************************************************
; ********************************* MODE X **********************************
; ***************************************************************************
; ***************************************************************************




Set_mode_X proc near

  pushad

  mov  ax,13h
  int  10h

  mov  dx,3C6h         ; \
  xor  al,al           ; -+-> Palette Off
  out  dx,al           ; /
  mov  dx,3C4h
  mov  ax,0604h
  out  dx,ax
  mov  ax,0100h
  out  dx,ax

  mov  al,01h          ; \
  out  dx,al           ;  +
  inc  dx              ;  |
  in   al,dx           ;  |
  mov  ah,al           ;  |
  mov  al,01h          ;  |
  push ax              ;  |
  mov  al,ah           ;  |
  or   al,20h          ;  +-> Screen Off
  out  dx,al           ; /

  mov  esi,Mode_X_Ptr
  cld
  lodsb
  or   al,al
  jz   DontSetDot
  mov  dx,3C2h
  out  dx,al
  DontSetDot:

  mov  dx,3C4h
  mov  ax,0300h
  out  dx,ax
  mov  dx,3D4h
  mov  al,11h
  out  dx,al
  inc  dx
  in   al,dx
  and  al,7Fh
  out  dx,al

  dec  dx
  lodsb
  xor  ecx,ecx
  mov  cl,al
  rep  outsw

  mov  ax,Largeur_ecran
  shr  ax,3
  mov  ah,al
  mov  al,13h
  out  dx,ax

  mov  dx,3C4h
  mov  ax,0F02h
  out  dx,ax

  mov  edi,0A0000h
  xor  eax,eax
  mov  ecx,4000h
  rep  stosd


  ; On calcule le d‚calage qu'il y a entre la fin de l'‚cran et le d‚but
  ; de la ligne suivante:
  ;
  ; Decalage = (Largeur_image - Largeur_ecran)

  xor  ebx,ebx
  mov   bx,Principal_Largeur_image
  sub  bx,Largeur_ecran
  mov  MODE_X_Decalage_synchro,ebx

  ; On calcule la largeur logique d'une ligne:
  ;
  ; Largeur_de_ligne = Largeur_ecran shr 2

  mov  ax,Largeur_ecran
  shr  ax,2
  mov  MODE_X_Largeur_de_ligne,ax

  mov  dx,3C4h      ; \
  pop  ax           ; -+-> Screen On
  out  dx,ax        ; /
  mov  dx,3C6h      ; \
  mov  al,0FFh      ; -+-> Palette On
  out  dx,al        ; /


  popad

  ret

Set_mode_X endp



Pixel_mode_X proc near

  push ebp
  mov  ebp,esp

  arg X:word,Y:word,Couleur:byte

  pushad

  mov  dx,3C4h
  mov  ax,102h
  mov  bx,X
  mov  cx,bx
  and  cl,03d
  shl  ah,cl
  out  dx,ax

  xor  eax,eax
  mov  ax,MODE_X_Largeur_de_ligne
  mul  Y
  add  eax,0A0000h

  shr  bx,2
  add  ax,bx
  mov  bl,Couleur
  mov  [eax],bl

  popad

  mov  esp,ebp
  pop  ebp

  ret

Pixel_mode_X endp



Lit_pixel_mode_X proc near

  push  ebp
  mov   ebp,esp

  arg   X:word,Y:word

  push  bx

  mov   bx,X
  mov   cx,bx
  and   cl,03d
  mov   dx,3CEh
  mov   al,04h
  mov   ah,cl
  out   dx,ax

  xor   eax,eax
  mov   ax,MODE_X_Largeur_de_ligne
  mul   Y
  add   eax,0A0000h

  shr   bx,02d
  add   ax,bx
  mov   al,[eax]


  pop   bx

  mov   esp,ebp
  pop   ebp

  ret

Lit_pixel_mode_X endp



Effacer_tout_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte

  pushad


  ; On fait passer la carte VGA en mode ‚criture sur 4 plans simultan‚s

  mov  dx,3C4h
  mov  ax,0F02h
  out  dx,ax

  ; On place EDI en d‚but d'‚cran:

  mov  edi,0A0000h

  ; On met EBX … 4 fois la couleur:

  mov  al,Couleur
  mov  ah,al
  mov  bx,ax
  shl  ebx,16
  mov  bx,ax

  ; On calcule le nb de dwords visibles dans l'image

  mov  ax,Menu_ordonnee
  mul  MODE_X_Largeur_de_ligne
  shr  ax,2
  mov  cx,ax
  jnc  @@Pas_de_probleme

  mov  [edi],bx
  add  edi,2

  @@Pas_de_probleme:

  @@Boucle:

    mov  [edi],ebx
    add  edi,4
    dec  cx
    jnz  @@Boucle


  popad

  mov  esp,ebp
  pop  ebp

  ret

Effacer_tout_l_ecran_mode_X endp




Block_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Debut_X:word,Debut_Y:word,Largeur:word,Hauteur:word,Couleur:byte

  pushad


  ; On met dans CL le plan initial

  mov  bx,Debut_X
  mov  cl,bl
  and  cl,00000011b

  ; On met dans CH la couleur

  mov  ch,Couleur

  ; On met dans ESI la valeur initiale de EDI

  mov  ax,Debut_Y
  mul  MODE_X_Largeur_de_ligne
  shr  bx,2
  add  ax,bx
  mov  esi,0A0000h
  mov  si,ax

  mov  bx,Largeur                            ; BX = Nb de colonnes … afficher
  mov  dx,3C4h                            ; DX = Port du CRTC (chngt de plan)

  Block_mode_X_Pour_chaque_colonne:

    ; On passe dans le plan d‚tenu par CL

    mov  ax,102h
    shl  ah,cl
    out  dx,ax

    ; On dessine la colonne en partant de ESI, et sur Hauteur pixels

    mov  edi,esi
    mov  ax,Hauteur                            ; AX = Nb de pixels … dessiner

    Block_mode_X_Pour_chaque_ligne_du_block:

      ; On recopie un pixel de la couleur demand‚e

      mov  [edi],ch

      ; On passe … la ligne suivante

      add  di,MODE_X_Largeur_de_ligne
      dec  ax
      jnz  Block_mode_X_Pour_chaque_ligne_du_block

    ; On change de colonne:

    dec  bx                        ; Si plus de colonnes … traiter
    jz   Block_mode_X_Fin_de_traitement ; -> On arrˆte le traitement

    inc  cl                        ; Sinon, on se prepare … passer au plan
    and  cl,00000011b              ; suivant. Si Plan <> 0
    jnz  Block_mode_X_Pour_chaque_colonne     ;          -> On reprend simplement

    inc  esi                       ; Sinon, on incr‚mente l'adresse
    jmp  Block_mode_X_Pour_chaque_colonne     ; et on reprend

  Block_mode_X_Fin_de_traitement:


  popad

  mov  esp,ebp
  pop  ebp

  ret

Block_mode_X endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode normal (pas en mode loupe) --

; Note: si on modifie cette proc‚dure, il faudra penser … faire ‚galement la
;       modif dans la proc‚dure Pixel_Preview_Loupe_mode_X.

Pixel_Preview_Normal_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  mov  dl,Couleur
  mov  cx,Y
  mov  ax,X
  sub  cx,Principal_Decalage_Y
  sub  ax,Principal_Decalage_X
  push edx
  push ecx
  push eax

  call Pixel_mode_X


  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Normal_mode_X endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode loupe --

Pixel_Preview_Loupe_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx


  ; On affiche d'abord le pixel dans la partie non-zoom‚e

  mov  dl,Couleur               ; Ca ne co–te pas trop
  mov  cx,Y                     ; en code de recopier
  mov  ax,X                     ; le contenu de la
  sub  cx,Principal_Decalage_Y  ; proc‚dure
  sub  ax,Principal_Decalage_X  ; Pixel_Preview_Normal_mode_X
  push edx                      ; et ‡a fait gagner du
  push ecx                      ; temps, et ce temps est
  push eax                      ; assez pr‚cieux vu que
  call Pixel_mode_X             ; c'est appel‚ TRES souvent.
  add  esp,12  ; 3 paramŠtres dword

  ; On regarde si on peut afficher le pixel ‚galement dans la loupe

  mov  ax,Y
  cmp  ax,Limite_Haut_Zoom
  jb   Pixel_Preview_Loupe_mode_X_Fin
  cmp  ax,Limite_visible_Bas_Zoom
  ja   Pixel_Preview_Loupe_mode_X_Fin
  mov  ax,X
  cmp  ax,Limite_Gauche_Zoom
  jb   Pixel_Preview_Loupe_mode_X_Fin
  cmp  ax,Limite_visible_Droite_Zoom
  ja   Pixel_Preview_Loupe_mode_X_Fin

  ; On passe la couleur

  mov  al,Couleur
  mov  edx,Table_mul_facteur_zoom       ; On pointe sur la table de x (OPTI)
  push eax

  ; On calcule dans EAX (X) et ECX (Y) le d‚marrage du bloc:

  xor  eax,eax
  xor  ecx,ecx
  mov  ax,X
  mov  cx,Y
  sub  ax,Loupe_Decalage_X
  sub  cx,Loupe_Decalage_Y
  mov  ax,[edx+2*eax]
  mov  cx,[edx+2*ecx]

  ; On sauve ces valeurs dans BX (X) et DX (Y):

  mov  bx,ax
  mov  dx,cx

  ; On s'occupe de la hauteur:

  add  cx,Loupe_Facteur
  cmp  cx,Menu_Ordonnee
  ja   Pixel_Preview_Loupe_mode_X_Tronque_en_hauteur

  mov  cx,Loupe_Facteur
  push ecx
  jmp  Pixel_Preview_Loupe_mode_X_Traitement_de_la_largeur

  Pixel_Preview_Loupe_mode_X_Tronque_en_hauteur:

  mov  cx,dx
  neg  cx
  add  cx,Menu_Ordonnee
  push ecx

  ; On s'occupe de la largeur:

  Pixel_Preview_Loupe_mode_X_Traitement_de_la_largeur:

  add  bx,Principal_X_Zoom

  mov  ax,Loupe_Facteur
  push eax

  ; On passe le d‚but en Y:
  push edx

  ; On passe le d‚but en X:
  push ebx

  call Block_mode_X


  add  esp,20                   ; 20 = 5 paramŠtres de type dword
  pop  ebx

  Pixel_Preview_Loupe_mode_X_Fin:

  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Loupe_mode_X endp




Ligne_horizontale_XOR_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Largeur:word

  push ebx
  push esi
  push edi


  ; On met dans CL le plan initial

  mov  bx,Pos_X
  mov  cl,bl
  and  cl,00000011b

  ; On met dans ESI la valeur initiale de EDI
  ; (ESI=D‚but de ligne, EDI=Position dans la ligne)

  mov  esi,0A0000h
  mov  ax,Pos_Y
  mul  MODE_X_Largeur_de_ligne
  shr  bx,2
  add  ax,bx
  mov  si,ax

  ; On met dans BX la coordonn‚e de fin +1

  mov  bx,Pos_X
  add  bx,Largeur

  ; On place dans CH le nb de plans … traiter

  mov  ch,4

  LH_Pour_chaque_plan:

    ; On initialise EDI

    mov  edi,esi

    ; On passe dans le plan d‚tenu par CL

      ; Plan d'‚criture:

    mov  dx,3C4h                          ; DX = Port du CRTC (chngt de plan)
    mov  ax,102h
    shl  ah,cl
    out  dx,ax

      ; Plan de lecture:

    mov  dx,3CEh                          ; DX = Port du TS (enfin, je crois)
    mov  ah,cl
    mov  al,04h
    out  dx,ax

    ; On met dans AX la coordonn‚e de d‚part

    mov  ax,Pos_X

    LH_Boucle:

      cmp  ax,bx
      jae  LH_Fin_de_traitement

      not  byte ptr[edi] ; On inverse le pixel courant

      inc  edi  ; On passe … la coordonn‚e +4
      add  ax,4

      jmp  LH_Boucle

    LH_Fin_de_traitement:

    ; On pr‚pare CL pour le prochain plan:

    inc  cl
    and  cl,00000011b
    jnz  LH_On_repasse_pas_dans_le_plan_0

    ; On pr‚pare ESI pour le prochain plan:

    inc  esi

    LH_On_repasse_pas_dans_le_plan_0:

    ; On pr‚pare la coordonn‚e de d‚part pour le prochain plan:

    inc  Pos_X

    ; On regarde s'il reste un plan … traiter:

    dec  ch
    jnz  LH_Pour_chaque_plan


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Ligne_horizontale_XOR_mode_X endp




Ligne_verticale_XOR_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Hauteur:word


  ; On met dans CL le plan initial

  mov  cx,Pos_X
  and  cl,00000011b

  ; On passe dans le plan d‚tenu par CL

    ; Plan d'‚criture:

  mov  dx,3C4h                          ; DX = Port du CRTC (chngt de plan)
  mov  ax,102h
  shl  ah,cl
  out  dx,ax

    ; Plan de lecture:

  mov  dx,3CEh                          ; DX = Port du TS (enfin, je crois)
  mov  ah,cl
  mov  al,04h
  out  dx,ax

  ; On met dans EDX la coordonn‚e initiale

  mov  edx,0A0000h
  mov  cx,Pos_X
  mov  ax,Pos_Y
  mul  MODE_X_Largeur_de_ligne
  shr  cx,2
  add  ax,cx
  mov  dx,ax

  ; On place dans CX le nb de pixels … traiter

  mov  cx,Hauteur

  ; On place dans AX la distance qu'il y a d'une ligne … l'autre:

  mov  ax,Mode_X_Largeur_de_ligne

  LV_Pour_chaque_ligne:

    not  byte ptr[edx]
    add  dx,ax

    dec  cx
    jnz  LV_Pour_chaque_ligne


  mov  esp,ebp
  pop  ebp

  ret

Ligne_verticale_XOR_mode_X endp




; ***************************************************************************
; ***************************************************************************
; ********************************** VESA ***********************************
; ***************************************************************************
; ***************************************************************************



VESA_Change_banque_Fenetre_A_FAST proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans la fenˆtre A

  cmp  al,VESA_Derniere_banque_Fenetre_A_utilisee
  je   VESA_Change_banque_Fenetre_A_FAST_Pas_besoin_de_changer

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    xor  bx,bx
    shl  dx,cl
    mov  ax,4F05h
    call [VESA_WinFuncPtr]
    popad
    mov  VESA_Derniere_banque_Fenetre_A_utilisee,al

  VESA_Change_banque_Fenetre_A_FAST_Pas_besoin_de_changer:

  ret

VESA_Change_banque_Fenetre_A_FAST endp



VESA_Change_banque_Fenetre_B_FAST proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans la fenˆtre B

  cmp  al,VESA_Derniere_banque_Fenetre_B_utilisee
  je   VESA_Change_banque_Fenetre_B_FAST_Pas_besoin_de_changer

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    mov  bx,1
    shl  dx,cl
    mov  ax,4F05h
    call [VESA_WinFuncPtr]
    popad
    mov  VESA_Derniere_banque_Fenetre_B_utilisee,al

  VESA_Change_banque_Fenetre_B_FAST_Pas_besoin_de_changer:

  ret

VESA_Change_banque_Fenetre_B_FAST endp



VESA_Change_banque_Fenetre_A_et_B_FAST proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans les fenˆtres A et B

  cmp  al,VESA_Derniere_banque_Fenetre_A_utilisee
  je   VESA_Change_banque_Fenetre_A_et_B_FAST_Pas_besoin_de_changer_A

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    xor  bx,bx
    shl  dx,cl
    mov  ax,4F05h
    call [VESA_WinFuncPtr]
    popad
    mov  VESA_Derniere_banque_Fenetre_A_utilisee,al

  VESA_Change_banque_Fenetre_A_et_B_FAST_Pas_besoin_de_changer_A:

  cmp  al,VESA_Derniere_banque_Fenetre_B_utilisee
  je   VESA_Change_banque_Fenetre_A_et_B_FAST_Pas_besoin_de_changer_B

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    mov  bx,1
    shl  dx,cl
    mov  ax,4F05h
    call [VESA_WinFuncPtr]
    popad
    mov  VESA_Derniere_banque_Fenetre_B_utilisee,al

  VESA_Change_banque_Fenetre_A_et_B_FAST_Pas_besoin_de_changer_B:

  ret

VESA_Change_banque_Fenetre_A_et_B_FAST endp



VESA_Change_banque_Fenetre_A proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans la fenˆtre A

  cmp  al,VESA_Derniere_banque_Fenetre_A_utilisee
  je   VESA_Change_banque_Fenetre_A_Pas_besoin_de_changer

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    xor  bx,bx
    shl  dx,cl
    mov  ax,4F05h
    int  10h
    popad
    mov  VESA_Derniere_banque_Fenetre_A_utilisee,al

  VESA_Change_banque_Fenetre_A_Pas_besoin_de_changer:

  ret

VESA_Change_banque_Fenetre_A endp



VESA_Change_banque_Fenetre_B proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans la fenˆtre B

  cmp  al,VESA_Derniere_banque_Fenetre_B_utilisee
  je   VESA_Change_banque_Fenetre_B_Pas_besoin_de_changer

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    mov  bx,1
    shl  dx,cl
    mov  ax,4F05h
    int  10h
    popad
    mov  VESA_Derniere_banque_Fenetre_B_utilisee,al

  VESA_Change_banque_Fenetre_B_Pas_besoin_de_changer:

  ret

VESA_Change_banque_Fenetre_B endp



VESA_Change_banque_Fenetre_A_et_B proc near

; ParamŠtre en entr‚e:
;   AL = Banque sur laquelle aller dans les fenˆtres A et B

  cmp  al,VESA_Derniere_banque_Fenetre_A_utilisee
  je   VESA_Change_banque_Fenetre_A_et_B_Pas_besoin_de_changer_A

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    xor  bx,bx
    shl  dx,cl
    mov  ax,4F05h
    int  10h
    popad
    mov  VESA_Derniere_banque_Fenetre_A_utilisee,al

  VESA_Change_banque_Fenetre_A_et_B_Pas_besoin_de_changer_A:

  cmp  al,VESA_Derniere_banque_Fenetre_B_utilisee
  je   VESA_Change_banque_Fenetre_A_et_B_Pas_besoin_de_changer_B

    pushad
    xor  ah,ah
    mov  cl,Granularite
    mov  dx,ax
    mov  bx,1
    shl  dx,cl
    mov  ax,4F05h
    int  10h
    popad
    mov  VESA_Derniere_banque_Fenetre_B_utilisee,al

  VESA_Change_banque_Fenetre_A_et_B_Pas_besoin_de_changer_B:

  ret

VESA_Change_banque_Fenetre_A_et_B endp



;; - -- --- ----\/\ Detection du support VESA sur la machine: /\/---- --- -- -
;
;Support_VESA proc near
;
;  pushad
;
;  ; On pr‚suppose qu'il n'y a pas d'erreur (pour l'instant)
;
;  mov  VESA_Erreur,0
;
;  ; On passe en mode graphique, o— nous attend un super buffer de 64Ko
;
;  mov  ax,0013h
;  int  10h
;  mov  dx,03C6h  ; On ‚teint la palette
;  xor  al,al
;  out  dx,al
;
;  ; On demande les infos VESA: (AX=4F00, ES:DI=Adresse du buffer)
;
;  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
;  mov  bl,10h      ; Interruption … appeler (int vid‚o)
;  xor  bh,bh       ; Etat des flags pass‚ … l'interruption
;  xor  cx,cx       ; Nb d'elements de la pile … passer
;  mov  edi,0A8000h ; Adresse des valeurs des registres … passer … l'interruption
;  mov  dword ptr[edi+28],00004F00h ; EAX pass‚ … l'interruption
;  mov  dword ptr[edi+00],00000000h ; EDI pass‚ … l'interruption
;  mov   word ptr[edi+34],0A000h    ; ES  pass‚ … l'interruption
;  mov  edx,0A0000h
;  mov  dword ptr[edx],32454256h ; valeur hexa de "VBE2"
;  int  31h        ; Appel de l'interruption du DPMI
;
;  ; Si AX <> 004F => Echec
;
;  mov  eax,dword ptr[edi+28]
;  cmp  ax,004Fh
;  je   Support_VESA_Pas_d_echec
;
;  ; L'appel … l'interruption a ‚chou‚
;
;  Support_VESA_Echec:
;
;  mov  VESA_Erreur,1
;  jmp  Support_VESA_Fin_traitement
;
;  Support_VESA_Pas_d_echec:
;
;  ; L'appel … l'interruption n'a pas ‚chou‚
;
;  ; On v‚rifie que la signature soit bien ‚gale … VESA:
;
;  mov  esi,0A0000h
;  cmp  byte ptr[esi+0],'V'
;  jne  Support_VESA_Echec
;
;  cmp  byte ptr[esi+1],'E'
;  jne  Support_VESA_Echec
;
;  cmp  byte ptr[esi+2],'S'
;  jne  Support_VESA_Echec
;
;  cmp  byte ptr[esi+3],'A'
;  jne  Support_VESA_Echec
;
;  ; La signature est bien pr‚sente
;
;  ; On enregistre l'adresse de la liste des modes VESA support‚s:
;
;  xor  ebx,ebx
;  xor  eax,eax
;  mov  bx,[esi+16] ; (Lst_Seg)
;  mov  ax,[esi+14] ; (Lst_Ofs)
;  shl  ebx,4
;  add  ebx,eax
;  mov  VESA_Liste_des_modes,ebx
;
;  ; On enregistre le num‚ro de version de VESA
;
;  mov  ax,word ptr[esi+4]
;  mov  VESA_Version_Unite,ah
;  mov  VESA_Version_Decimale,al
;
;  ; On enregistre l'adresse de la chaŒne du constructeur
;
;  xor  ebx,ebx
;  xor  eax,eax
;  mov  bx,word ptr[esi+8]
;  mov  ax,word ptr[esi+6]
;  shl  ebx,4
;  add  ebx,eax
;  mov  VESA_Constructeur,ebx
;
;  ; On enregistre la taille totale de m‚moire vid‚o
;
;  mov  ax,[esi+18]
;  mov  VESA_Taille_memoire,ax
;
;
;  Support_VESA_Fin_Traitement:
;
;  popad
;
;  ret
;
;Support_VESA endp
;
;
;
;; - -- --- -----\/\ Detection du support VESA pour un mode: /\/----- --- -- -
;
;Mode_VESA_supporte proc near
;
;  push ebp
;  mov  ebp,esp
;
;  arg  Mode:word
;
;  pushad
;
;  ; On pressupose qu'il n'y a pas d'erreur (pour l'instant)
;  ; On se servira de la valeur de SI comme code d'erreur pour faciliter le
;  ; d‚bugging
;
;  mov  si,4 ; Code d'erreur de base
;  mov  VESA_Erreur,0
;
;  ; On va commencer par regarder si le mode n'est pas dans la liste de ceux
;  ; support‚s par la carte. Pour cela, il nous faut redemander cette liste
;  ; car certaines cartes la "perdent" d'un appel sur l'autre.
;
;  ; On passe en mode graphique, o— nous attend un super buffer de 64Ko
;
;  mov  ax,0013h
;  int  10h
;  mov  dx,03C6h  ; On ‚teint la palette
;  xor  al,al
;  out  dx,al
;
;  ; On demande les infos VESA: (AX=4F00, ES:DI=Adresse du buffer)
;
;  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
;  mov  bl,10h      ; Interruption … appeler (int vid‚o)
;  xor  bh,bh       ; Etat des flags pass‚ … l'interruption
;  xor  cx,cx       ; Nb d'elements de la pile … passer
;  mov  edi,0A8000h ; Adresse des valeurs des registres … passer … l'interruption
;  mov  dword ptr[edi+28],00004F00h ; EAX pass‚ … l'interruption
;  mov  dword ptr[edi+00],00000000h ; EDI pass‚ … l'interruption
;  mov   word ptr[edi+34],0A000h    ; ES  pass‚ … l'interruption
;  mov  edx,0A0000h
;  mov  dword ptr[edx],32454256h ; valeur hexa de "VBE2"
;  int  31h        ; Appel de l'interruption du DPMI
;
;  ; Si AX <> 004F => Echec
;
;  mov  eax,dword ptr[edi+28]
;  cmp  ax,004Fh
;  jne  Mode_VESA_supporte_Echec
;
;  ; On enregistre l'adresse de la liste des modes VESA support‚s dans EDI
;
;  mov  edi,0A0000h
;  xor  ebx,ebx
;  xor  eax,eax
;  mov  bx,[edi+16] ; (Lst_Seg)
;  mov  ax,[edi+14] ; (Lst_Ofs)
;  shl  ebx,4
;  add  ebx,eax
;  mov  edi,ebx
;
;  ; Et on parcours la liste pour chercher le mode demand‚
;
;  mov  ax,Mode
;
;  Mode_VESA_supporte_Boucle:
;
;    cmp  [edi],ax
;    je   Mode_VESA_supporte_Fin_boucle
;
;    cmp  word ptr[edi],0FFFFh
;    je   Mode_VESA_supporte_Fin_boucle
;
;    add  edi,2
;    jmp  Mode_VESA_supporte_Boucle
;
;  Mode_VESA_supporte_Fin_boucle:
;
;  ; Si le mode en cours = FFFF => Echec
;
;  mov  si,5 ; Nouveau code d'erreur
;
;  cmp  word ptr[edi],ax
;  je   Mode_VESA_supporte_Pas_echec
;
;  ; Ce mode vid‚o VESA n'est pas support‚:
;
;  Mode_VESA_supporte_Echec:
;
;  mov  ax,si
;  mov  VESA_Erreur,al
;  jmp  Mode_VESA_supporte_Fin_traitement
;
;  Mode_VESA_supporte_Pas_echec:
;
;  ; Ce mode vid‚o VESA est un ‚l‚ment de la liste:
;
;  ; On passe en mode graphique, o— nous attend un super buffer de 64Ko (c00l)
;
;  mov  ax,0013h
;  int  10h
;  mov  dx,03C6h  ; On ‚teint la palette
;  xor  al,al
;  out  dx,al
;
;  ; On demande les infos VESA du mode: (AX=4F01, CX=Mode, ES:DI=Adresse du buffer)
;
;  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
;  mov  bl,10h      ; Interruption … appeler (int vid‚o)
;  xor  bh,bh       ; Etat des flags pass‚ … l'interruption
;  xor  cx,cx       ; Nb d'elements de la pile … passer
;  mov  edi,0A8000h ; Adresse des valeurs des registres … passer … l'interruption
;  mov  dword ptr[edi+28],00004F01h ; AX pass‚ … l'interruption
;  xor  edx,edx
;  mov  dx,Mode
;  mov  dword ptr[edi+24],edx       ; ECX pass‚ … l'interruption
;  mov  dword ptr[edi+00],00000000h ; EDI pass‚ … l'interruption
;  mov   word ptr[edi+34],0A000h    ;  ES pass‚ … l'interruption
;  int  31h        ; Appel de l'interruption du DPMI
;
;  ; Si AX <> 004F => Echec
;
;  mov  si,6 ; Nouveau code d'erreur
;
;  mov  eax,dword ptr[edi+28]
;  cmp  ax,004Fh
;  jne  Mode_VESA_supporte_Echec
;
;  ; L'appel … l'interruption n'a pas ‚chou‚
;
;  ; On v‚rifie l'utilit‚ des deux fenˆtres:
;
;  mov  edi,0A0000h
;  mov  al,[edi+2]
;  and  al,7
;  cmp  al,7
;  jne  Mode_VESA_supporte_Fenetre_A_insuffisante
;
;  mov  VESA_Erreur,0
;  jmp  Mode_VESA_supporte_Traitement_fenetres_termine
;
;  Mode_VESA_supporte_Fenetre_A_insuffisante:
;
;  mov  al,[edi+3]
;  and  al,7
;  cmp  al,7
;  jne  Mode_VESA_supporte_Fenetre_B_insuffisante
;
;  mov  VESA_Erreur,1
;  jmp  Mode_VESA_supporte_Traitement_fenetres_termine
;
;  Mode_VESA_supporte_Fenetre_B_insuffisante:
;
;  mov  al,[edi+2]
;  test al,1
;  jz   Mode_VESA_supporte_Pas_fenetres_par_defaut
;
;  mov  VESA_Erreur,2
;  jmp  Mode_VESA_supporte_Traitement_fenetres_termine
;
;  Mode_VESA_supporte_Pas_fenetres_par_defaut:
;
;  test al,2
;  jz   Mode_VESA_supporte_Fenetre_A_pas_lisible
;
;  mov  VESA_Erreur,3
;  jmp  Mode_VESA_supporte_Traitement_fenetres_termine
;
;  Mode_VESA_supporte_Fenetre_A_pas_lisible:
;
;  mov  VESA_Erreur,2
;
;  Mode_VESA_supporte_Traitement_fenetres_termine:
;
;  ; On v‚rifie que la taille des fenˆtres soit bien de 64Ko:
;
;  mov  si,7 ; Nouveau code d'erreur
;
;  cmp  word ptr[edi+6],64
;  jne  Mode_VESA_supporte_Echec
;
;  ; On lit la valeur de granularite:
;
;  mov  ax,[edi+4]
;
;  ; Et on en d‚duit un facteur de granularite:
;
;  ; Test si granularite = 64:
;
;  cmp  ax,64
;  jne  Mode_VESA_supporte_Gran_pas_64
;
;  mov  Granularite,0
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_64:
;
;  ; Test si granularite = 32:
;
;  cmp  ax,32
;  jne  Mode_VESA_supporte_Gran_pas_32
;
;  mov  Granularite,1
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_32:
;
;  ; Test si granularite = 16:
;
;  cmp  ax,16
;  jne  Mode_VESA_supporte_Gran_pas_16
;
;  mov  Granularite,2
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_16:
;
;  ; Test si granularite = 8:
;
;  cmp  ax,8
;  jne  Mode_VESA_supporte_Gran_pas_8
;
;  mov  Granularite,3
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_8:
;
;  ; Test si granularite = 4:
;
;  cmp  ax,4
;  jne  Mode_VESA_supporte_Gran_pas_4
;
;  mov  Granularite,4
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_4:
;
;  ; Test si granularite = 2:
;
;  cmp  ax,2
;  jne  Mode_VESA_supporte_Gran_pas_2
;
;  mov  Granularite,5
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_2:
;
;  ; Test si granularite = 1:
;
;  cmp  ax,1
;  jne  Mode_VESA_supporte_Gran_pas_1
;
;  mov  Granularite,6
;  jmp  Mode_VESA_supporte_Fin_traitement_granularite
;
;  Mode_VESA_supporte_Gran_pas_1:
;
;  mov  VESA_Erreur,4
;
;  Mode_VESA_supporte_Fin_traitement_granularite:
;
;
;
;  mov  VESA_WinFuncPtr,00000000h
;
;  ; On passe en mode graphique, o— nous attend un super buffer de 64Ko (c00l)
;
;  mov  ax,0013h
;  int  10h
;  mov  dx,03C6h  ; On ‚teint la palette
;  xor  al,al
;  out  dx,al
;
;  ; On demande le code de changement de banque
;
;  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
;  mov  bl,10h      ; Interruption … appeler (int vid‚o)
;  xor  bh,bh       ; Etat des flags pass‚ … l'interruption
;  xor  cx,cx       ; Nb d'elements de la pile … passer
;  mov  edi,0A8000h ; Adresse des valeurs des registres … passer … l'interruption
;  mov  dword ptr[edi+28],00004F0Ah ; EAX pass‚ … l'interruption
;  mov  dword ptr[edi+16],00000000h ; EBX pass‚ … l'interruption
;  int  31h         ; Appel de l'interruption du DPMI
;
;  ; Si AX <> 004F => Echec
;
;  mov  si,8 ; Nouveau code d'erreur
;
;  mov  eax,dword ptr[edi+28]
;  cmp  ax,004Fh
;  jne  Mode_VESA_supporte_Fin_traitement
;
;  ; On note l'adresse du handler software de changement de banque
;
;  xor  eax,eax
;  xor  ecx,ecx
;  mov  ax,word ptr[edi+34]  ; ES retourn‚ par l'interruption
;  mov  cx,word ptr[edi+00]  ; DI retourn‚ par l'interruption
;  shl  eax,4
;  add  eax,ecx
;  mov  cx,[eax]
;  add  eax,ecx
;  mov  VESA_WinFuncPtr,eax
;
;  Mode_VESA_supporte_Fin_traitement:
;
;
;  popad
;
;  mov  esp,ebp
;  pop  ebp
;
;  ret
;
;Mode_VESA_supporte endp



; -- Initialisation du Mode Graphique VESA --

Initialiser_mode_video_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Video_mode:word

  push ebx

  mov  VESA_Derniere_banque_Fenetre_A_utilisee,0
  mov  VESA_Derniere_banque_Fenetre_B_utilisee,0

  mov  ax,4F02h
  mov  bx,Video_mode
  int  10h

  ; On donne la largeur logique de l'‚cran car certaines cartes vid‚o la
  ; reconnaissent mal... (va savoir pourquoi!?)
  mov  ax,4F06h
  xor  bl,bl
  mov  cx,Largeur_ecran
  int  10h

  ; On calcule la distance d'octet en m‚moire qu'il y a entre la fin d'une
  ; ligne d'image … l'‚cran et le d‚but de la suivante … l'‚cran:
  ;
  ; Decalage = (Largeur_image - Largeur_ecran)

  xor  ebx,ebx
  mov   bx,Principal_Largeur_image
  sub  bx,Largeur_ecran
  mov  VESA_Decalage_synchro,ebx

  ; On calcule la largeur d'une ligne de l'‚cran en dword:
  ;
  ; Largeur en dword = Largeur_ecran shr 2

  mov  ax,Largeur_ecran
  shr  ax,2
  mov  VESA_Largeur_ecran_en_dword,ax

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Initialiser_mode_video_VESA endp



; - --/\/ Retoucher les registres du CRTC pour redimensionner l'‚cran \/\-- -

Retoucher_CRTC proc near

  pushad

  mov  esi,Mode_X_Ptr
  cld
  lodsb
  or   al,al
  jz   NoThankYou
  mov  dx,3C2h
  out  dx,al
  NoThankYou:

  mov  dx,3D4h
  mov  al,11h
  out  dx,al
  inc  dx
  in   al,dx
  and  al,7Fh
  out  dx,al

  dec  dx
  lodsb
  xor  ecx,ecx
  mov  cl,al
  rep  outsw

  popad

  ret

Retoucher_CRTC endp




; -- Affichage d'un point graphique en X,Y --

Pixel_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Couleur:byte


  mov  edx,0A0000h
  mov  cl,Couleur

  mov  ax,Pos_Y
  mul  Largeur_ecran
  add  ax,Pos_X
  adc  dx,0

  xchg dx,ax
  call [VESA_Change_banque_ecriture]

  mov  [edx],cl


  mov  esp,ebp
  pop  ebp

  ret

Pixel_VESA endp




; -- R‚cup‚ration d'un point graphique en X,Y --

Lit_pixel_VESA proc near

  push  ebp
  mov   ebp,esp

  ARG   Pos_X:word,Pos_Y:word


  xor   eax,eax
  mov   edx,0A0000h

  mov   ax,Pos_Y
  mul   Largeur_ecran
  add   ax,Pos_X
  adc   dx,0

  xchg  dx,ax
  call  [VESA_Change_banque_lecture]

  mov   al,byte ptr[edx]


  mov   esp,ebp
  pop   ebp

  ret

Lit_pixel_VESA endp



Effacer_tout_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte

  pushad


  ; On met dans EBX 4 fois la couleur

  mov  al,Couleur
  mov  ah,al
  mov  bx,ax
  shl  ebx,16
  mov  bx,ax

  ; On calcule l'endroit jusqu'o— on efface l'ecran

  mov  ax,Menu_ordonnee
  mul  Largeur_ecran

  ; DX = DerniŠre banque touch‚e
  ; AX = DerniŠre position touch‚e

  shr  ax,2 ; On divise AX par 4 puisqu'on travaille avec des dword

  Pour_chaque_banque:

    ; On passe … la banque DL

    xchg dx,ax
    call [VESA_Change_banque_ecriture]
    xchg dx,ax

    ; On se positionne en d‚but de banque:

    mov  edi,0A0000h

    ; On efface AX mots:

    mov  cx,ax

    ; On met dans EAX 4 fois la couleur:

    mov  eax,ebx

    Boucle:

      mov  [edi],eax
      add  edi,4
      dec  cx
      jnz  Boucle

    mov  ax,4000h ; On demande pour la prochaine banque un remplissage sur
		  ; toute sa taille

    dec  dl
    jns  Pour_chaque_banque


  popad

  mov  esp,ebp
  pop  ebp

  ret

Effacer_tout_l_ecran_VESA endp





Block_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Debut_X:word,Debut_Y:word,Largeur:word,Hauteur:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI 4 fois la couleur

  mov  cl,Couleur
  mov  ch,cl
  mov  si,cx
  shl  esi,16
  mov  si,cx

  ; On calcule dans EDI la premiŠre coordonn‚e du bloc:

  mov  ax,Debut_Y
  mul  Largeur_ecran
  add  ax,Debut_X
  adc  dx,0
  mov  edi,0A0000h
  mov  di,ax

  ; On m‚morise la banque n‚cessaire

  mov  VESA_Banque_en_cours,dl

  ; On passe dans cette banque

  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On met dans BX le nombre de ligne … traiter:

  mov  bx,Hauteur

  ; On met dans DX le d‚calage qu'il faudra effectuer en fin de ligne sur DI
  ; pour qu'il pointe sur la nouvelle ligne

  mov  dx,Largeur_ecran
  sub  dx,Largeur

  xor  ecx,ecx

  Block_VESA_Pour_chaque_ligne:

    ; On regarde si la totalit‚ de la ligne loge dans la banque:

    mov  cx,di
    add  cx,Largeur
    jc   Block_VESA_Ligne_rompue

    ; A ce stade, on sait que la ligne loge entiŠrement sur l'‚cran

    ; On va donc placer dans EAX 4 fois la couleur

    mov  eax,esi

    ; On place dans CX le nb de pixels … afficher sur la ligne

    mov  cx,Largeur

    ; On commence par afficher les "d‚chets" de d‚but de ligne

    shr  cx,1
    jnc  Block_VESA_Ininterrompue_Largeur_paire
    stosb

    Block_VESA_Ininterrompue_Largeur_paire:

    shr  cx,1
    jnc  Block_VESA_Ininterrompue_Largeur_multiple_de_4
    stosw

    Block_VESA_Ininterrompue_Largeur_multiple_de_4:

    ; On affiche ce qui reste de la ligne

    rep  stosd

    ; On regarde si ce n'‚tait pas la derniŠre ligne

    dec  bx
    jz   Block_VESA_Fin_de_traitement

    ; Sinon,
    ; Maintenant que la ligne est affich‚e, on regarde si le changement de
    ; ligne ne d‚clenche pas un changement de banque:

    add  di,dx
    jnc  Block_VESA_Pour_chaque_ligne

    ; Si le changement de banque est n‚cessaire, on s'en fait un:

    inc  VESA_Banque_en_cours
    mov  al,VESA_Banque_en_cours
    call [VESA_Change_banque_ecriture]
    jmp  Block_VESA_Pour_chaque_ligne

  Block_VESA_Ligne_rompue:

    ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

    ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
    ; rupture:

    push cx

    ; On place dans EAX 4 fois la couleur

    mov  eax,esi

    ; On met dans CX le nb de pixels affichables avant la rupture

    mov  cx,di
    neg  cx

    ; On commence par afficher les "d‚chets" de d‚but de ligne

    shr  cx,1
    jnc  Block_VESA_Interrompue_Largeur_gauche_paire
    stosb

    Block_VESA_Interrompue_Largeur_gauche_paire:

    shr  cx,1
    jnc  Block_VESA_Interrompue_Largeur_gauche_multiple_de_4
    stosw

    Block_VESA_Interrompue_Largeur_gauche_multiple_de_4:

    ; On affiche ce qui reste de la partie gauche de la ligne

    rep  stosd

    ; On change de banque:

    inc  VESA_Banque_en_cours
    mov  al,VESA_Banque_en_cours
    call [VESA_Change_banque_ecriture]
    mov  eax,esi

    ; On replace EDI en d‚but de banque

    mov  edi,0A0000h

    ; On restaure le nb de pixels … afficher … droite de la rupture

    pop  cx

    ; On commence par afficher les "d‚chets" de d‚but de ligne

    shr  cx,1
    jnc  Block_VESA_Interrompue_Largeur_droite_paire
    stosb

    Block_VESA_Interrompue_Largeur_droite_paire:

    shr  cx,1
    jnc  Block_VESA_Interrompue_Largeur_droite_multiple_de_4
    stosw

    Block_VESA_Interrompue_Largeur_droite_multiple_de_4:

    ; On affiche ce qui reste de la partie droite de la ligne

    rep  stosd

    ; On regarde si ce n'‚tait pas la derniŠre ligne
    ; (on peut se permettre de supposer qu'aprŠs ce changement de banque,
    ;  on ne cours qu'un risque proche de 0 qu'un nouveau changement de
    ;  banque survienne lors du changement de ligne - pour ne pas dire que
    ;  c'est une chose impossible)

    add  di,dx
    dec  bx
    jnz  Block_VESA_Pour_chaque_ligne


  Block_VESA_Fin_de_traitement:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Block_VESA endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode normal (pas en mode loupe) --

; Note: si on modifie cette proc‚dure, il faudra penser … faire ‚galement la
;       modif dans la proc‚dure Pixel_Preview_Loupe_VESA.

Pixel_Preview_Normal_VESA proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  mov  dl,Couleur
  mov  cx,Y
  mov  ax,X
  sub  cx,Principal_Decalage_Y
  sub  ax,Principal_Decalage_X
  push edx
  push ecx
  push eax

  call Pixel_VESA


  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Normal_VESA endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode loupe --

Pixel_Preview_Loupe_VESA proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx


  ; On affiche d'abord le pixel dans la partie non-zoom‚e

  mov  dl,Couleur               ; Ca ne co–te pas trop
  mov  cx,Y                     ; en code de recopier
  mov  ax,X                     ; le contenu de la
  sub  cx,Principal_Decalage_Y  ; proc‚dure
  sub  ax,Principal_Decalage_X  ; Pixel_Preview_Normal_VESA
  push edx                      ; et ‡a fait gagner du
  push ecx                      ; temps, et ce temps est
  push eax                      ; assez pr‚cieux vu que
  call Pixel_VESA               ; c'est appel‚ TRES souvent.
  add  esp,12  ; 3 paramŠtres dword

  ; On regarde si on peut afficher le pixel ‚galement dans la loupe

  mov  ax,Y
  cmp  ax,Limite_Haut_Zoom
  jb   Pixel_Preview_Loupe_VESA_Fin
  cmp  ax,Limite_visible_Bas_Zoom
  ja   Pixel_Preview_Loupe_VESA_Fin
  mov  ax,X
  cmp  ax,Limite_Gauche_Zoom
  jb   Pixel_Preview_Loupe_VESA_Fin
  cmp  ax,Limite_visible_Droite_Zoom
  ja   Pixel_Preview_Loupe_VESA_Fin

  ; On passe la couleur

  mov  al,Couleur
  mov  edx,Table_mul_facteur_zoom       ; On pointe sur la table de x (OPTI)
  push eax

  ; On calcule dans EAX (X) et ECX (Y) le d‚marrage du bloc:

  xor  eax,eax
  xor  ecx,ecx
  mov  ax,X
  mov  cx,Y
  sub  ax,Loupe_Decalage_X
  sub  cx,Loupe_Decalage_Y
  mov  ax,[edx+2*eax]
  mov  cx,[edx+2*ecx]

  ; On sauve ces valeurs dans BX (X) et DX (Y):

  mov  bx,ax
  mov  dx,cx

  ; On s'occupe de la hauteur:

  add  cx,Loupe_Facteur
  cmp  cx,Menu_Ordonnee
  ja   Pixel_Preview_Loupe_VESA_Tronque_en_hauteur

  mov  cx,Loupe_Facteur
  push ecx
  jmp  Pixel_Preview_Loupe_VESA_Traitement_de_la_largeur

  Pixel_Preview_Loupe_VESA_Tronque_en_hauteur:

  mov  cx,dx
  neg  cx
  add  cx,Menu_Ordonnee
  push ecx

  ; On s'occupe de la largeur:

  Pixel_Preview_Loupe_VESA_Traitement_de_la_largeur:

  add  bx,Principal_X_Zoom

  mov  ax,Loupe_Facteur
  push eax

  ; On passe le d‚but en Y:
  push edx

  ; On passe le d‚but en X:
  push ebx

  call Block_VESA


  add  esp,20                   ; 20 = 5 paramŠtres de type dword
  pop  ebx

  Pixel_Preview_Loupe_VESA_Fin:

  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Loupe_VESA endp



Ligne_horizontale_XOR_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Largeur:word


  mov  edx,0A0000h
  mov  ax,Pos_Y
  mul  Largeur_ecran
  add  ax,Pos_X
  adc  dx,0

  xchg dx,ax
  call [VESA_Change_banque_lect_ecr]

  mov  cx,Largeur

  LH_VESA_Boucle:

    not  byte ptr[edx]

    inc  dx
    jnz  LH_VESA_Pas_de_changement_de_banque

    inc  al
    call [VESA_Change_banque_lect_ecr]

    LH_VESA_Pas_de_changement_de_banque:

    dec  cx
    jnz  LH_VESA_Boucle


  mov  esp,ebp
  pop  ebp

  ret

Ligne_horizontale_XOR_VESA endp




Ligne_verticale_XOR_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Pos_Y:word, Hauteur:word

  push ebx


  mov  bx,Largeur_ecran
  mov  edx,0A0000h
  mov  ax,Pos_Y
  mul  bx
  add  ax,Pos_X
  adc  dx,0

  xchg dx,ax
  call [VESA_Change_banque_lect_ecr]

  mov  cx,Hauteur

  LV_VESA_Boucle:

    not  byte ptr[edx]

    add  dx,bx
    jnc  LV_VESA_Pas_de_changement_de_banque

    inc  al
    call [VESA_Change_banque_lect_ecr]

    LV_VESA_Pas_de_changement_de_banque:

    dec  cx
    jnz  LV_VESA_Boucle


  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Ligne_verticale_XOR_VESA endp





; Afficher une partie de la brosse en couleur en MCGA

Display_brush_Color_MCGA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov   edi,0A0000h
  mov   di,Pos_Y   ; |
  mov   bx,di      ; |
  shl   di,8       ; |- DI <- Pos_Y * 320
  shl   bx,6       ; |
  add   di,bx      ; |
  add   di,Pos_X

  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov   ax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; On place dans DX le nb de lignes … traiter
  mov   dx,Hauteur

  ; On place dans BH la couleur de transparence
  mov   bh,Couleur_de_transparence

  ; On place dans EAX la distance entre deux lignes … l'‚cran
  mov   eax,320
  sub    ax,Largeur

  Display_brush_Color_MCGA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Display_brush_Color_MCGA_Pour_chaque_pixel:

      ; On lit le contenu de la brosse
      mov   bl,[esi]

      ; Gestion de la transparence
      cmp   bl,bh
      je    Display_brush_Color_MCGA_Pas_de_copie

	; On affiche le pixel de la brosse … l'‚cran
	mov   [edi],bl

      Display_brush_Color_MCGA_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc   edi
      dec   cx
      jnz   Display_brush_Color_MCGA_Pour_chaque_pixel

    ; On passe … la ligne suivante
    mov   cx,Largeur_brosse
    add   edi,eax
    sub   cx,Largeur
    add   esi,ecx
    dec   dx
    jnz   Display_brush_Color_MCGA_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Color_MCGA endp





; Afficher une partie de la brosse en monochrome en MCGA

Display_brush_Mono_MCGA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov   edi,0A0000h
  mov   di,Pos_Y   ; |
  mov   bx,di      ; |
  shl   di,8       ; |- DI <- Pos_Y * 320
  shl   bx,6       ; |
  add   di,bx      ; |
  add   di,Pos_X

  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov   ax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; On place dans DX le nb de lignes … traiter
  mov   dx,Hauteur

  ; On place dans BH la couleur de transparence
  mov   bh,Couleur_de_transparence

  ; On place dans BL la couleur de coloriage
  mov   bl,Couleur

  ; On place dans EAX la distance entre 2 lignes … l'‚cran
  mov   eax,320
  sub    ax,Largeur

  Display_brush_Mono_MCGA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Display_brush_Mono_MCGA_Pour_chaque_pixel:

      ; On v‚rifie que le contenu de la brosse ne soit pas transparent
      cmp   [esi],bh
      je    Display_brush_Mono_MCGA_Pas_de_copie

	; On affiche le pixel de la brosse … l'‚cran
	mov   [edi],bl

      Display_brush_Mono_MCGA_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc   edi
      dec   cx
      jnz   Display_brush_Mono_MCGA_Pour_chaque_pixel

    ; On passe … la ligne suivante
    mov   cx,Largeur_brosse
    add   edi,eax
    sub   cx,Largeur
    add   esi,ecx
    dec   dx
    jnz   Display_brush_Mono_MCGA_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Mono_MCGA endp





; Effacer la partie de la brosse affich‚e … l'‚cran en MCGA

Clear_brush_MCGA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_image:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov   edi,0A0000h
  mov   ax,Pos_Y   ; |
  mov   di,ax      ; |
  shl   ax,6       ; |- DI <- Pos_Y * 320
  shl   di,8       ; |
  add   di,ax      ; |
  add   di,Pos_X

  ; Calculer dans ESI les coordonn‚es
  ; (Pos_X+Principal_Decalage_X,Pos_Y+Principal_Decalage_Y) dans l'image
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov    ax,Pos_Y
  mov    bx,Largeur_image
  mov    cx,Pos_X
  add    ax,Principal_Decalage_Y
  add    cx,Principal_Decalage_X
  mul   ebx
  mov   esi,Principal_Ecran
  add   eax,ecx
  add   esi,eax

  ; On place dans DX le nb de lignes … traiter
  mov   dx,Hauteur

  ; On met dans EBX la distance entre 2 lignes … l'‚cran
  mov   ebx,320
  sub    bx,Largeur

  ; On met dans EAX la distance entre 2 lignes dans l'image
  xor   eax,eax
  mov    ax,Largeur_image
  sub    ax,Largeur

  ; On nettoie la partie haute de ECX:
  xor   ecx,ecx

  Clear_brush_MCGA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    ; On fait une copie de la ligne
    shr   cx,1
    jnc   Clear_brush_MCGA_Multiple_de_2
    movsb
    Clear_brush_MCGA_Multiple_de_2:
    shr   cx,1
    jnc   Clear_brush_MCGA_Multiple_de_4
    movsw
    Clear_brush_MCGA_Multiple_de_4:
    rep   movsd

    ; On passe … la ligne suivante
    add   esi,eax
    add   edi,ebx
    dec   dx
    jnz   Clear_brush_MCGA_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Clear_brush_MCGA endp






; Afficher une partie de la brosse en couleur en VESA

Display_brush_Color_VESA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov   ax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov   edi,0A0000h
  mov   ax,Pos_Y
  mul   Largeur_ecran
  add   ax,Pos_X
  adc   dx,0
  mov   di,ax

  ; On passe dans la banque n‚cessaire (et on la m‚morise dans AL)
  mov   al,dl
  call  [VESA_Change_banque_ecriture]

  ; On place dans AH la couleur de transparence
  mov   ah,Couleur_de_transparence

  ; On place dans BX la diff‚rence … l'‚cran entre 2 lignes:
  mov   bx,Largeur_ecran
  sub   bx,Largeur

  Display_brush_Color_VESA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Display_brush_Color_VESA_Pour_chaque_pixel:

      ; On lit le contenu de la brosse
      mov   dl,[esi]

      ; Gestion de la transparence
      cmp   dl,ah
      je    Display_brush_Color_VESA_Pas_de_copie

	; On affiche le pixel de la brosse … l'‚cran
	mov   [edi],dl

      Display_brush_Color_VESA_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc    di

      jz    Display_brush_Color_VESA_Debordement_1

      dec   cx
      jnz   Display_brush_Color_VESA_Pour_chaque_pixel
      jmp   Display_brush_Color_VESA_Fin_de_ligne

      Display_brush_Color_VESA_Debordement_1:

	; On passe dans la banque suivante
	inc   al
	call  [VESA_Change_banque_ecriture]
	dec   cx
	jnz   Display_brush_Color_VESA_Pour_chaque_pixel

      Display_brush_Color_VESA_Fin_de_ligne:

    ; On passe … la ligne suivante
    mov    cx,Largeur_brosse
    sub    cx,Largeur
    add   esi,ecx
    add    di,bx

    jc    Display_brush_Color_VESA_Debordement_2

    dec   Hauteur
    jnz   Display_brush_Color_VESA_Pour_chaque_ligne
    jmp   Display_brush_Color_VESA_Fin_de_traitement

    Display_brush_Color_VESA_Debordement_2:

      ; On passe dans la banque suivante
      inc   al
      call  [VESA_Change_banque_ecriture]
      dec   Hauteur
      jnz   Display_brush_Color_VESA_Pour_chaque_ligne

    Display_brush_Color_VESA_Fin_de_traitement:


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Color_VESA endp





; Afficher une partie de la brosse en monochrome en VESA

Display_brush_Mono_VESA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov   ax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov   edi,0A0000h
  mov   ax,Pos_Y
  mul   Largeur_ecran
  add   ax,Pos_X
  adc   dx,0
  mov   di,ax

  ; On passe dans la banque n‚cessaire (et on la m‚morise dans AL)
  mov   al,dl
  call  [VESA_Change_banque_ecriture]

  ; On place dans AH la couleur de transparence
  mov   ah,Couleur_de_transparence

  ; On place dans DL la couleur de coloriage
  mov   dl,Couleur

  ; On place dans BX la diff‚rence … l'‚cran entre 2 lignes:
  mov   bx,Largeur_ecran
  sub   bx,Largeur

  Display_brush_Mono_VESA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,word ptr[Largeur]

    Display_brush_Mono_VESA_Pour_chaque_pixel:

      ; On v‚rifie que le contenu de la brosse ne soit pas la couleur de
      ; transparence
      cmp   [esi],ah
      je    Display_brush_Mono_VESA_Pas_de_copie

	; On affiche un pixel de la couleur … l'‚cran
	mov   [edi],dl

      Display_brush_Mono_VESA_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc    di

      jz    Display_brush_Mono_VESA_Debordement_1

      dec   cx
      jnz   Display_brush_Mono_VESA_Pour_chaque_pixel
      jmp   Display_brush_Mono_VESA_Fin_de_ligne

      Display_brush_Mono_VESA_Debordement_1:

	; On passe dans la banque suivante
	inc   al
	call  [VESA_Change_banque_ecriture]
	dec   cx
	jnz   Display_brush_Mono_VESA_Pour_chaque_pixel

      Display_brush_Mono_VESA_Fin_de_ligne:

    ; On passe … la ligne suivante
    mov    cx,Largeur_brosse
    sub    cx,Largeur
    add   esi,ecx
    add    di,bx

    jc    Display_brush_Mono_VESA_Debordement_2

    dec   Hauteur
    jnz   Display_brush_Mono_VESA_Pour_chaque_ligne
    jmp   Display_brush_Mono_VESA_Fin_de_traitement

    Display_brush_Mono_VESA_Debordement_2:

      ; On passe dans la banque suivante
      inc   al
      call  [VESA_Change_banque_ecriture]
      dec   Hauteur
      jnz   Display_brush_Mono_VESA_Pour_chaque_ligne

    Display_brush_Mono_VESA_Fin_de_traitement:


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Mono_VESA endp






; Effacer la partie de la brosse affich‚e … l'‚cran en VESA

Clear_brush_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_image:word

  push ebx
  push esi
  push edi


  ; Calculer dans ESI les coordonn‚es
  ; (Pos_X+Principal_Decalage_X,Pos_Y+Principal_Decalage_Y) dans l'image
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov    ax,Pos_Y
  mov    bx,Largeur_image
  mov    cx,Pos_X
  add    ax,Principal_Decalage_Y
  add    cx,Principal_Decalage_X
  mul   ebx
  mov   esi,Principal_Ecran
  add   eax,ecx
  add   esi,eax

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  mov  edi,0A0000h
  mov  ax,Pos_Y
  mul  Largeur_ecran
  add  ax,Pos_X
  adc  dx,0
  mov  di,ax

  ; On passe dans la banque n‚cessaire (et on la m‚morise dans AL)
  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On place dans DX la diff‚rence entre 2 lignes … l'‚cran
  mov  dx,Largeur_ecran
  sub  dx,Largeur

  ; On place dans EBX la diff‚rence entre 2 lignes dans l'image
  mov  bx,Largeur_image
  sub  bx,Largeur

  Clear_brush_VESA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov  cx,Largeur

    Clear_brush_VESA_Pour_chaque_pixel:

      ; On recopie le pixel de l'image … l'‚cran
      mov  ah,[esi]
      inc  esi
      mov  [edi],ah
      inc  di

      jz   Clear_brush_VESA_Debordement_1

      dec  cx
      jnz  Clear_brush_VESA_Pour_chaque_pixel
      jmp  Clear_brush_VESA_Fin_de_ligne

      Clear_brush_VESA_Debordement_1:

	; On passe dans la banque suivante
	inc  al
	call [VESA_Change_banque_ecriture]

;        ; On r‚ajuste EDI qui vient de sortir du segment vid‚o
;        mov  edi,0A0000h

	dec  cx
	jnz  Clear_brush_VESA_Pour_chaque_pixel

      Clear_brush_VESA_Fin_de_ligne:

    ; On passe … la ligne suivante
    add  esi,ebx
    add  di,dx

    jc   Clear_brush_VESA_Debordement_2

    dec  Hauteur
    jnz  Clear_brush_VESA_Pour_chaque_ligne
    jmp  Clear_brush_VESA_Fin_de_traitement

    Clear_brush_VESA_Debordement_2:

      ; On passe dans la banque suivante
      inc  al
      call [VESA_Change_banque_ecriture]
      dec  Hauteur
      jnz  Clear_brush_VESA_Pour_chaque_ligne

    Clear_brush_VESA_Fin_de_traitement:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Clear_brush_VESA endp






; Afficher une partie de la brosse en couleur en mode X

Display_brush_Color_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_brosse:dword

  push ebx
  push esi
  push edi

  ; On nettoye la partie haute de Largeur_brosse
  and   Largeur_brosse,0FFFFh

  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   ebx,ebx
  xor   ecx,ecx
  mov   eax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran et dans AX
  ; le plan (AH = 1, 2, 4 ou 8) et le nø de registre de chgnt de plan
  mov  edi,0A0000h
  mov  ax,MODE_X_Largeur_de_ligne
  mov  cx,Pos_X
  mul  Pos_Y
  mov  dx,cx
  shr  dx,2
  add  ax,dx
  mov  di,ax

  mov  ax,0102h
  and  cx,03h
  shl  ah,cl

  ; On passe dans le plan de d‚part
  ;
  ; Pour passer dans le plan N (0-3) en ‚criture:
  ; out 3C4h,02h
  ; out 3C5h,1 shl N
  ;
  mov  dx,3C4h
  out  dx,ax

  ; On sauve la valeur initiale de EDI
  mov  MODE_X_Valeur_initiale_de_edi,edi

  ; On sauve la valeur initiale de ESI
  mov  MODE_X_Valeur_initiale_de_esi,esi

  ; On place dans BH la couleur de transparence
  mov  bh,Couleur_de_transparence

  Display_brush_Color_mode_X_Pour_chaque_colonne:

    ; On place dans CX le nb de pixels … traiter sur la colonne
    mov  cx,Hauteur

    ; On place dans DX la distance entre 2 lignes … l'‚cran
    mov  dx,MODE_X_Largeur_de_ligne

    Display_brush_Color_mode_X_Pour_chaque_ligne:

      ; On lit le contenu de la brosse
      mov  bl,[esi]

      ; Gestion de la transparence
      cmp  bl,bh
      je   Display_brush_Color_mode_X_Pas_de_copie

	; On affiche le pixel de la brosse … l'‚cran
	mov  [edi],bl

      Display_brush_Color_mode_X_Pas_de_copie:

      ; On passe … la ligne suivante
      add  esi,Largeur_brosse
      add  di,dx
      dec  cx
      jnz  Display_brush_Color_mode_X_Pour_chaque_ligne

    ; On passe … la colonne suivante
    inc  MODE_X_Valeur_initiale_de_esi
    mov  esi,MODE_X_Valeur_initiale_de_esi

    add  ah,ah
    cmp  ah,10h
    jne  Display_brush_Color_mode_X_Pas_de_reset_de_plan

      inc  MODE_X_Valeur_initiale_de_edi
      mov  ah,01h

    Display_brush_Color_mode_X_Pas_de_reset_de_plan:

    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le nouveau plan
    ;
    ; Pour passer dans le plan N (0-3) en ‚criture:
    ; out 3C4h,02h
    ; out 3C5h,1 shl N
    ;
    mov  dx,3C4h
    out  dx,ax

    dec  Largeur
    jnz  Display_brush_Color_mode_X_Pour_chaque_colonne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Color_mode_X endp






; Afficher une partie de la brosse en monochrome en mode X

Display_brush_Mono_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:dword

  push ebx
  push esi
  push edi


  ; On nettoye la partie haute de Largeur_brosse
  and   Largeur_brosse,0FFFFh

  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   ebx,ebx
  xor   ecx,ecx
  mov   eax,Largeur_brosse
  mov   bx,Decalage_Y
  mov   cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran et dans AX
  ; le plan (AH = 1, 2, 4 ou 8) et le nø de registre de chgnt de plan
  mov  edi,0A0000h
  mov  ax,MODE_X_Largeur_de_ligne
  mov  cx,Pos_X
  mul  Pos_Y
  mov  dx,cx
  shr  dx,2
  add  ax,dx
  mov  di,ax

  mov  ax,0102h
  and  cx,03h
  shl  ah,cl

  ; On passe dans le plan de d‚part
  ;
  ; Pour passer dans le plan N (0-3) en ‚criture:
  ; out 3C4h,02h
  ; out 3C5h,1 shl N
  ;
  mov  dx,3C4h
  out  dx,ax

  ; On sauve la valeur initiale de EDI
  mov  MODE_X_Valeur_initiale_de_edi,edi

  ; On sauve la valeur initiale de ESI
  mov  MODE_X_Valeur_initiale_de_esi,esi

  ; On place dans BH la couleur de transparence
  mov  bh,Couleur_de_transparence

  ; On place dans BL la couleur de coloriage
  mov  bl,Couleur

  Display_brush_Mono_mode_X_Pour_chaque_colonne:

    ; On place dans CX le nb de pixels … traiter sur la colonne
    mov  cx,Hauteur

    ; On place dans DX la distance entre 2 lignes … l'‚cran
    mov  dx,MODE_X_Largeur_de_ligne

    Display_brush_Mono_mode_X_Pour_chaque_ligne:

      ; On v‚rifie que le contenu de la brosse ne soit pas transparent
      cmp  [esi],bh
      je   Display_brush_Mono_mode_X_Pas_de_copie

	; On affiche le pixel de la brosse … l'‚cran
	mov  [edi],bl

      Display_brush_Mono_mode_X_Pas_de_copie:

      ; On passe … la ligne suivante
      add  esi,Largeur_brosse
      add  di,dx
      dec  cx
      jnz  Display_brush_Mono_mode_X_Pour_chaque_ligne

    ; On passe … la colonne suivante
    inc  MODE_X_Valeur_initiale_de_esi
    mov  esi,MODE_X_Valeur_initiale_de_esi

    add  ah,ah
    cmp  ah,10h
    jne  Display_brush_Mono_mode_X_Pas_de_reset_de_plan

      inc  MODE_X_Valeur_initiale_de_edi
      mov  ah,01h

    Display_brush_Mono_mode_X_Pas_de_reset_de_plan:

    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le nouveau plan
    ;
    ; Pour passer dans le plan N (0-3) en ‚criture:
    ; out 3C4h,02h
    ; out 3C5h,1 shl N
    ;
    mov  dx,3C4h
    out  dx,ax

    dec  Largeur
    jnz  Display_brush_Mono_mode_X_Pour_chaque_colonne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Mono_mode_X endp






; Effacer la partie visible de la brosse en mode X

Clear_brush_mode_X proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_image:dword

  push  ebx
  push  esi
  push  edi


  ; On nettoye la partie haute de Largeur_image
  and   Largeur_image,0FFFFh

  ; Calculer dans ESI les coordonn‚es
  ; (Pos_X+Principal_Decalage_X,Pos_Y+Principal_Decalage_Y) dans l'image
  xor   eax,eax
  xor   ecx,ecx
  mov    ax,Pos_Y
  mov   ebx,Largeur_image
  mov    cx,Pos_X
  add    ax,Principal_Decalage_Y
  add    cx,Principal_Decalage_X
  mul   ebx
  mov   esi,Principal_Ecran
  add   eax,ecx
  add   esi,eax

  ; On sauve la valeur initiale de ESI
  mov  MODE_X_Valeur_initiale_de_esi,esi

  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran et dans AX
  ; le plan (AH = 1, 2, 4 ou 8) et le nø de registre de chgnt de plan
  mov  edi,0A0000h
  mov  ax,MODE_X_Largeur_de_ligne
  mov  cx,Pos_X
  mul  Pos_Y
  mov  dx,cx
  shr  dx,2
  add  ax,dx
  mov  di,ax

  mov  ax,0102h
  and  cx,03h
  shl  ah,cl

  ; On passe dans le plan de d‚part
  ;
  ; Pour passer dans le plan N (0-3) en ‚criture:
  ; out 3C4h,02h
  ; out 3C5h,1 shl N
  ;
  mov  dx,3C4h
  out  dx,ax

  ; On sauve la valeur initiale de EDI
  mov  MODE_X_Valeur_initiale_de_edi,edi

  Clear_brush_mode_X_Pour_chaque_colonne:

    ; On place dans CX le nb de pixels … traiter sur la colonne
    mov  cx,Hauteur

    ; On place dans DX la distance entre 2 lignes … l'‚cran
    mov  dx,MODE_X_Largeur_de_ligne

    Clear_brush_mode_X_Pour_chaque_ligne:

      ; On copie le pixel de l'image vers l'‚cran en passant … la ligne
      ; suivante
      mov  bl,[esi]
      add  esi,Largeur_image
      mov  [edi],bl
      add  di,dx

      dec  cx
      jnz  Clear_brush_mode_X_Pour_chaque_ligne

    ; On passe … la colonne suivante
    inc  MODE_X_Valeur_initiale_de_esi
    mov  esi,MODE_X_Valeur_initiale_de_esi

    add  ah,ah
    cmp  ah,10h
    jne  Clear_brush_mode_X_Pas_de_reset_de_plan

      inc  MODE_X_Valeur_initiale_de_edi
      mov  ah,01h

    Clear_brush_mode_X_Pas_de_reset_de_plan:

    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le nouveau plan
    ;
    ; Pour passer dans le plan N (0-3) en ‚criture:
    ; out 3C4h,02h
    ; out 3C5h,1 shl N
    ;
    mov  dx,3C4h
    out  dx,ax

    dec  Largeur
    jnz  Clear_brush_mode_X_Pour_chaque_colonne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Clear_brush_mode_X endp







; Remapper une partie de l'‚cran en MCGA

Remap_screen_MCGA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Hauteur:word,Conversion:dword

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran

  mov   edi,0A0000h
  mov   di,Pos_Y   ; |
  mov   bx,di      ; |
  shl   di,8       ; |_ DI <- (Pos_Y*320) + Pos_X
  shl   bx,6       ; |
  add   di,bx      ; |
  add   di,Pos_X   ; |

  ; On place dans DX le nb de lignes … traiter

  mov   dx,Hauteur

  ; On place dans EBX l'adresse de la table de conversion

  mov   ebx,Conversion

  ; On nettoie (entre autres) la partie haute de EAX

  xor   eax,eax

  ; On met dans ESI la distance entre la fin d'une ligne et le d‚but de la
  ; suivante

  mov   esi,320
  sub   si,Largeur

  Remap_screen_MCGA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne

    mov   cx,Largeur

    Remap_screen_MCGA_Pour_chaque_pixel:

      mov  al,[edi]
      mov  al,[ebx+eax]
      mov  [edi],al

      ; On passe au pixel suivant
      inc  edi
      dec  cx
      jnz  Remap_screen_MCGA_Pour_chaque_pixel

    ; On passe … la ligne suivante
    add  edi,esi

    dec  dx
    jnz  Remap_screen_MCGA_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Remap_screen_MCGA endp







; Remapper une partie de l'‚cran en mode X

Remap_screen_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word,Hauteur:word,Conversion:dword

  push ebx
  push esi
  push edi


  ; On met dans CL le plan initial

  mov  bx,Pos_X
  mov  cl,bl
  and  cl,00000011b

  ; On met dans ESI la valeur initiale de EDI
  ; (ESI=D‚but de ligne, EDI=Position dans la ligne)

  mov  esi,0A0000h
  mov  ax,Pos_Y
  mul  MODE_X_Largeur_de_ligne
  shr  bx,2
  add  ax,bx
  mov  si,ax

  ; On met dans BX la coordonn‚e de fin +1 (horizontalement)

  mov  bx,Pos_X
  add  bx,Largeur

  Remap_screen_mode_X_Pour_chaque_ligne:

    ; On place dans CH le nb de plans … traiter

    mov  ch,4

    Remap_screen_mode_X_Pour_chaque_plan:

      ; On initialise EDI

      mov  edi,esi

      ; On passe dans le plan d‚tenu par CL

	; Plan d'‚criture:

      mov  dx,3C4h                         ; DX = Port du CRTC (chngt de plan)
      mov  ax,102h
      shl  ah,cl
      out  dx,ax

	; Plan de lecture:

      mov  dx,3CEh                         ; DX = Port du TS (enfin, je crois)
      mov  ah,cl
      mov  al,04h
      out  dx,ax

      ; On met dans AX la coordonn‚e de d‚part

      mov  ax,Pos_X

      Remap_screen_mode_X_Boucle:

	cmp  ax,bx
	jae  Remap_screen_mode_X_Fin_de_traitement

	; On convertit le pixel courant
	xor  edx,edx
	mov  dl,[edi]
	add  edx,Conversion
	mov  dl,[edx]
	mov  [edi],dl

	inc  edi  ; On passe … la coordonn‚e +4
	add  ax,4

	jmp  Remap_screen_mode_X_Boucle

      Remap_screen_mode_X_Fin_de_traitement:

      ; On pr‚pare CL pour le prochain plan:

      inc  cl
      and  cl,00000011b
      jnz  Remap_screen_mode_X_On_repasse_pas_dans_le_plan_0

      ; On pr‚pare ESI pour le prochain plan:

      inc  esi

      Remap_screen_mode_X_On_repasse_pas_dans_le_plan_0:

      ; On pr‚pare la coordonn‚e de d‚part pour le prochain plan:

      inc  Pos_X

      ; On regarde s'il reste un plan … traiter:

      dec  ch
      jnz  Remap_screen_mode_X_Pour_chaque_plan

    ; On passe … la ligne suivante
    dec si               ; ESI a forc‚ment ripp‚ sur la droite, on le corrige
    add si,MODE_X_Largeur_de_ligne                ; et on passe … la suivante
    sub Pos_X,4          ; On fait de mˆme avec Pos_X

    dec Hauteur
    jnz Remap_screen_mode_X_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Remap_screen_mode_X endp









; Remapper une partie de l'‚cran en VESA

Remap_screen_VESA proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Hauteur:word,Conversion:dword

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran

  mov   edi,0A0000h
  mov   ax,Pos_Y
  mul   Largeur_ecran
  add   ax,Pos_X
  adc   dx,0
  mov   di,ax

  ; On passe dans la banque n‚cessaire (et on la m‚morise dans AL)

  mov   al,dl
  call  [VESA_Change_banque_lect_ecr]

  ; On place dans EBX l'adresse de la table de conversion

  mov   ebx,Conversion

  ; On nettoie la partie haute de EDX

  xor   edx,edx

  ; On met dans SI la distance entre la fin d'une ligne et le d‚but de la
  ; suivante

  mov   si,Largeur_ecran
  sub   si,Largeur

  Remap_screen_VESA_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne

    mov   cx,Largeur

    Remap_screen_VESA_Pour_chaque_pixel:

      mov   dl,[edi]
      mov   dl,[edx+ebx]
      mov   [edi],dl

      ; On passe au pixel suivant

      inc   di
      jz    Remap_screen_VESA_Debordement_1

      dec   cx
      jnz   Remap_screen_VESA_Pour_chaque_pixel
      jmp   Remap_screen_VESA_Fin_de_ligne

      Remap_screen_VESA_Debordement_1:

      ; On passe dans la banque suivante

      inc   al
      call  [VESA_Change_banque_lect_ecr]

      dec   cx
      jnz   Remap_screen_VESA_Pour_chaque_pixel

      Remap_screen_VESA_Fin_de_ligne:

    ; On passe … la ligne suivante

    add   di,si
    jc    Remap_screen_VESA_Debordement_2

    dec   Hauteur
    jnz   Remap_screen_VESA_Pour_chaque_ligne
    jmp   Remap_screen_VESA_Fin_de_traitement

    Remap_screen_VESA_Debordement_2:

    ; On passe dans la banque suivante

    inc   al
    call  [VESA_Change_banque_lect_ecr]

    dec   Hauteur
    jnz   Remap_screen_VESA_Pour_chaque_ligne

    Remap_screen_VESA_Fin_de_traitement:


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Remap_screen_VESA endp





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Afficher une partie de l'image tel quel sur l'‚cran

Afficher_partie_de_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Principal_Decalage_Y*Largeur_image)+Principal_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Principal_Decalage_Y
  mov  cx,Principal_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h

  mov  edi,0A0000h

  ; On met dans EAX la distance qu'il y a entre le dernier pixel affich‚
  ; d'une ligne et le premier pixel affich‚ de la ligne suivante DANS L'IMAGE
  ;
  ; EAX = Largeur_image-Largeur

  xor  eax,eax
  mov   ax,Largeur_image
  sub   ax,Largeur

  ; On met dans EBX la distance qu'il y a entre le dernier pixel affich‚
  ; d'une ligne et le premier pixel affich‚ de la ligne suivante DANS L'ECRAN
  ;
  ; EBX = 320 - Largeur
  mov  ebx,320
  sub   bx,Largeur

  ; On met dans DX le nombre de lignes … traiter:
  mov  dx,Hauteur

  ; On nettoye la partie haute de ECX: (OPTIM.: VIRABLE CAR FAIT PLUS HAUT)
  xor  ecx,ecx

  APDLE_MCGA_Pour_chaque_ligne:

    ; On met dans ECX le nombre de pixels … traiter:

    mov  cx,Largeur

    ; On les traite au mieux par copie 8/16/32 bits
    shr  cx,1
    jnc  APDLE_MCGA_ECX_multiple_de_2
    movsb
    APDLE_MCGA_ECX_multiple_de_2:
    shr  cx,1
    jnc  APDLE_MCGA_ECX_multiple_de_4
    movsw
    APDLE_MCGA_ECX_multiple_de_4:
    rep  movsd

    ; On passe … la ligne suivante
    ; sans oublier d'incr‚menter correctement ESI et EDI
    add  esi,eax
    add  edi,ebx

    dec  dx
    jnz  APDLE_MCGA_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_MCGA endp




; Afficher une ligne … l'‚cran

Afficher_une_ligne_a_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  mov  ax,320
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  mov  di,ax

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  ; On les traite au mieux par copie 8/16/32 bits
  shr  cx,1
  jnc  AULALE_MCGA_ECX_multiple_de_2
  movsb
  AULALE_MCGA_ECX_multiple_de_2:
  shr  cx,1
  jnc  AULALE_MCGA_ECX_multiple_de_4
  movsw
  AULALE_MCGA_ECX_multiple_de_4:
  rep  movsd


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_a_l_ecran_MCGA endp





; Lire une ligne … l'‚cran et la stocker dans un buffer

Lire_une_ligne_a_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push esi
  push edi


  ; On met dans EDI l'adresse de la ligne … lire:

  mov  edi,Ligne

  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)

  mov  ax,320
  mov  esi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  mov  si,ax

  ; On met dans ECX le nombre de pixels … traiter:

  xor  ecx,ecx
  mov  cx,Largeur

  ; On les traite au mieux par copie 8/16/32 bits
  shr  cx,1
  jnc  LULALE_MCGA_ECX_multiple_de_2
  movsb
  LULALE_MCGA_ECX_multiple_de_2:
  shr  cx,1
  jnc  LULALE_MCGA_ECX_multiple_de_4
  movsw
  LULALE_MCGA_ECX_multiple_de_4:
  rep  movsd


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Lire_une_ligne_a_l_ecran_MCGA endp






; Afficher une partie de l'image zoom‚e … l'‚cran

Afficher_partie_de_l_ecran_zoomee_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word,Buffer:dword

  ; Largeur = Largeur non zoom‚e
  ; Hauteur = Hauteur zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Loupe_Decalage_Y*Largeur_image)+Loupe_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Loupe_Decalage_Y
  mov  cx,Loupe_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans EDX la ligne en cours de traitement:
  xor  edx,edx

  ; On met dans EBX la distance entre deux lignes d'image
  xor  ebx,ebx
  mov  bx,Largeur_image

  APDLEZ_MCGA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  cx,Loupe_Facteur

    APDLEZ_MCGA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      push cx
      push dx
      push Buffer     ; Ligne
      push edi        ; Largeur*Facteur
      push edx        ; Pos_Y
      mov  ax,Principal_X_Zoom
      push eax        ; Pos_X
      call Afficher_une_ligne_a_l_ecran_MCGA
      add  esp,16
      pop  dx
      pop  cx

      ; On passe … la ligne suivante
      inc  dx                                 ; -> On v‚rifie qu'on ne soit
      cmp  dx,Hauteur                         ;   pas arriv‚ … la ligne
      je   APDLEZ_MCGA_Menu_Ordonnee_atteinte ;   terminale
      dec  cx                                 ; -> ou que l'on ai termin‚ de
      jnz  APDLEZ_MCGA_Pour_chaque_ligne      ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne image suivante
    add  esi,ebx
    jmp  APDLEZ_MCGA_Pour_chaque_ligne_a_zoomer

  APDLEZ_MCGA_Menu_Ordonnee_atteinte:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_zoomee_MCGA endp







;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;







; Afficher une partie de l'image tel quel sur l'‚cran

Afficher_partie_de_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Principal_Decalage_Y*Largeur_image)+Principal_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Principal_Decalage_Y
  mov  cx,Principal_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h

  mov  edi,0A0000h
  ; Et on passe dans la banque correspondante
  xor  al,al
  call [VESA_Change_banque_ecriture]

  ; On met dans EBX la distance qu'il y a entre le dernier pixel affich‚
  ; d'une ligne et le premier pixel affich‚ de la ligne suivante DANS L'IMAGE
  ;
  ; EBX = Largeur_image-Largeur

  mov  bx,Largeur_image
  sub  bx,Largeur

  ; On met dans DX le nombre de lignes … traiter:

  mov  dx,Hauteur

  ; On nettoye la partie haute de ECX: (OPTIM.: VIRABLE CAR FAIT PLUS HAUT)

  xor  ecx,ecx

  APDLE_VESA_Pour_chaque_ligne:

    ; On met dans ECX le nombre de pixels … traiter:

    mov  cx,Largeur

    ; On regarde si la ligne loge totalement dans la banque en cours
    add  cx,di
    jc   APDLE_VESA_Debordement_de_ligne

    ; Si c'est le cas:
    mov  cx,Largeur     ; correction de CX … cause du test

    ; On effectue une copie 8/16/32 bits
    shr  cx,1
    jnc  APDLE_VESA_Nobreak_ECX_multiple_de_2
    movsb
    APDLE_VESA_Nobreak_ECX_multiple_de_2:
    shr  cx,1
    jnc  APDLE_VESA_Nobreak_ECX_multiple_de_4
    movsw
    APDLE_VESA_Nobreak_ECX_multiple_de_4:
    rep  movsd

    jmp  APDLE_VESA_Fin_traitement_ligne


    ; Si ce n'est pas le cas (d‚bordement de la ligne)
    APDLE_VESA_Debordement_de_ligne:

    ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

    ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
    ; rupture:
    push cx

    ; On met dans CX le nb de pixels affichables avant la rupture:
    mov  cx,di
    neg  cx

    ; On effectue une copie 8/16/32 bits de la partie gauche de la rupture:
    shr  cx,1
    jnc  APDLE_VESA_BreakLeft_ECX_multiple_de_2
    movsb
    APDLE_VESA_BreakLeft_ECX_multiple_de_2:
    shr  cx,1
    jnc  APDLE_VESA_BreakLeft_ECX_multiple_de_4
    movsw
    APDLE_VESA_BreakLeft_ECX_multiple_de_4:
    rep  movsd

    ; On change de banque:
    inc  al
    call [VESA_Change_banque_ecriture]

    ; On replace EDI en d‚but de banque
    mov  edi,0A0000h

    ; On restaure le nb de pixels … afficher … droite de la rupture
    pop  cx

    ; On effectue une copie 8/16/32 bits de la partie droite de la rupture:
    shr  cx,1
    jnc  APDLE_VESA_BreakRight_ECX_multiple_de_2
    movsb
    APDLE_VESA_BreakRight_ECX_multiple_de_2:
    shr  cx,1
    jnc  APDLE_VESA_BreakRight_ECX_multiple_de_4
    movsw
    APDLE_VESA_BreakRight_ECX_multiple_de_4:
    rep  movsd


    APDLE_VESA_Fin_traitement_ligne:
    ; Le traitement de la ligne est termin‚, on change de ligne:

    ; On passe … la ligne suivante en incr‚mentant correctement ESI et (E)DI:
    mov  cx,Largeur_ecran
    add  esi,ebx
    sub  cx,Largeur
    add  di,cx
    jnc  APDLE_VESA_Fin_traitement_ligne_Sans_cht_bnk

    ; Cas o— le changement de ligne entraŒne un changement de banque:
    inc  al
    call [VESA_Change_banque_ecriture]

    APDLE_VESA_Fin_traitement_ligne_Sans_cht_bnk:
    ; On a fait le changement de ligne, on peut voir s'il reste des lignes …
    ; traiter:

    dec  dx
    jnz  APDLE_VESA_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_VESA endp




; Afficher une ligne … l'‚cran

Afficher_une_ligne_a_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:

  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*Largeur_ecran)+Pos_X)

  mov  ax,Largeur_ecran
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  adc  dx,0
  mov  di,ax
  ; Et on passe dans la banque correspondante
  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  ; On regarde si la ligne loge totalement dans la banque en cours
  add  cx,di
  jc   AULALE_VESA_Debordement_de_ligne

  ; Si c'est le cas:
  mov  cx,Largeur     ; correction de CX … cause du test

  ; On effectue une copie 8/16/32 bits
  shr  cx,1
  jnc  AULALE_VESA_Nobreak_ECX_multiple_de_2
  movsb
  AULALE_VESA_Nobreak_ECX_multiple_de_2:
  shr  cx,1
  jnc  AULALE_VESA_Nobreak_ECX_multiple_de_4
  movsw
  AULALE_VESA_Nobreak_ECX_multiple_de_4:
  rep  movsd

  jmp  AULALE_VESA_Fin_traitement_ligne


  ; Si ce n'est pas le cas (d‚bordement de la ligne)
  AULALE_VESA_Debordement_de_ligne:

  ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

  ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
  ; rupture:
  push cx

  ; On met dans CX le nb de pixels affichables avant la rupture:
  mov  cx,di
  neg  cx

  ; On effectue une copie 8/16/32 bits de la partie gauche de la rupture:
  shr  cx,1
  jnc  AULALE_VESA_BreakLeft_ECX_multiple_de_2
  movsb
  AULALE_VESA_BreakLeft_ECX_multiple_de_2:
  shr  cx,1
  jnc  AULALE_VESA_BreakLeft_ECX_multiple_de_4
  movsw
  AULALE_VESA_BreakLeft_ECX_multiple_de_4:
  rep  movsd

  ; On change de banque:
  inc  al
  call [VESA_Change_banque_ecriture]

  ; On replace EDI en d‚but de banque
  mov  edi,0A0000h

  ; On restaure le nb de pixels … afficher … droite de la rupture
  pop  cx

  ; On effectue une copie 8/16/32 bits de la partie droite de la rupture:
  shr  cx,1
  jnc  AULALE_VESA_BreakRight_ECX_multiple_de_2
  movsb
  AULALE_VESA_BreakRight_ECX_multiple_de_2:
  shr  cx,1
  jnc  AULALE_VESA_BreakRight_ECX_multiple_de_4
  movsw
  AULALE_VESA_BreakRight_ECX_multiple_de_4:
  rep  movsd


  AULALE_VESA_Fin_traitement_ligne:


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_a_l_ecran_VESA endp





; Lire une ligne … l'‚cran et la stocker dans un buffer

Lire_une_ligne_a_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push esi
  push edi


  ; On met dans EDI l'adresse de la ligne … lire:

  mov  edi,Ligne

  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*Largeur_ecran)+Pos_X)

  mov  ax,Largeur_ecran
  mov  esi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  adc  dx,0
  mov  si,ax
  ; Et on passe dans la banque correspondante
  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  ; On regarde si la ligne loge totalement dans la banque en cours
  add  cx,si
  jc   LULALE_VESA_Debordement_de_ligne

  ; Si c'est le cas:
  mov  cx,Largeur     ; correction de CX … cause du test

  ; On effectue une copie 8/16/32 bits
  shr  cx,1
  jnc  LULALE_VESA_Nobreak_ECX_multiple_de_2
  movsb
  LULALE_VESA_Nobreak_ECX_multiple_de_2:
  shr  cx,1
  jnc  LULALE_VESA_Nobreak_ECX_multiple_de_4
  movsw
  LULALE_VESA_Nobreak_ECX_multiple_de_4:
  rep  movsd

  jmp  LULALE_VESA_Fin_traitement_ligne


  ; Si ce n'est pas le cas (d‚bordement de la ligne)
  LULALE_VESA_Debordement_de_ligne:

  ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

  ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
  ; rupture:
  push cx

  ; On met dans CX le nb de pixels affichables avant la rupture:
  mov  cx,si
  neg  cx

  ; On effectue une copie 8/16/32 bits de la partie gauche de la rupture:
  shr  cx,1
  jnc  LULALE_VESA_BreakLeft_ECX_multiple_de_2
  movsb
  LULALE_VESA_BreakLeft_ECX_multiple_de_2:
  shr  cx,1
  jnc  LULALE_VESA_BreakLeft_ECX_multiple_de_4
  movsw
  LULALE_VESA_BreakLeft_ECX_multiple_de_4:
  rep  movsd

  ; On change de banque:
  inc  al
  call [VESA_Change_banque_ecriture]

  ; On replace ESI en d‚but de banque
  mov  esi,0A0000h

  ; On restaure le nb de pixels … afficher … droite de la rupture
  pop  cx

  ; On effectue une copie 8/16/32 bits de la partie droite de la rupture:
  shr  cx,1
  jnc  LULALE_VESA_BreakRight_ECX_multiple_de_2
  movsb
  LULALE_VESA_BreakRight_ECX_multiple_de_2:
  shr  cx,1
  jnc  LULALE_VESA_BreakRight_ECX_multiple_de_4
  movsw
  LULALE_VESA_BreakRight_ECX_multiple_de_4:
  rep  movsd


  LULALE_VESA_Fin_traitement_ligne:


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Lire_une_ligne_a_l_ecran_VESA endp






; Afficher une partie de l'image zoom‚e … l'‚cran

Afficher_partie_de_l_ecran_zoomee_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word,Buffer:dword

  ; Largeur = Largeur non zoom‚e
  ; Hauteur = Hauteur zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Loupe_Decalage_Y*Largeur_image)+Loupe_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Loupe_Decalage_Y
  mov  cx,Loupe_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans EDX la ligne en cours de traitement:
  xor  edx,edx

  ; On met dans EBX la distance entre deux lignes d'image
  xor  ebx,ebx
  mov  bx,Largeur_image

  APDLEZ_VESA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  cx,Loupe_Facteur

    APDLEZ_VESA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      push cx
      push dx
      push Buffer     ; Ligne
      push edi        ; Largeur*Facteur
      push edx        ; Pos_Y
      mov  ax,Principal_X_Zoom
      push eax        ; Pos_X
      call Afficher_une_ligne_a_l_ecran_VESA
      add  esp,16
      pop  dx
      pop  cx

      ; On passe … la ligne suivante
      inc  dx                                 ; -> On v‚rifie qu'on ne soit
      cmp  dx,Hauteur                         ;   pas arriv‚ … la ligne
      je   APDLEZ_VESA_Menu_Ordonnee_atteinte ;   terminale
      dec  cx                                 ; -> ou que l'on ai termin‚ de
      jnz  APDLEZ_VESA_Pour_chaque_ligne      ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne image suivante
    add  esi,ebx
    jmp  APDLEZ_VESA_Pour_chaque_ligne_a_zoomer

  APDLEZ_VESA_Menu_Ordonnee_atteinte:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_zoomee_VESA endp




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




; Afficher une partie de l'image tel quel sur l'‚cran

Afficher_partie_de_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Principal_Decalage_Y*Largeur_image)+Principal_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Principal_Decalage_Y
  mov  bx,Largeur_image
  mov  cx,Principal_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans EBX la distance qu'il y a entre la fin d'une ligne et le
  ; d‚but de la ligne suivante … l'‚cran:
  ; = DEJA FAIT

  ; On met dans CX le nø du 1er plan trait‚:
  mov  ecx,3

  APDLE_mode_X_Pour_chaque_plan:

    ; On passe dans le plan en question:
    mov  dx,3C4h
    mov  ax,102h
    shl  ah,cl
    out  dx,ax
    push esi
    push cx

    ; On fait pointer les ESI et EDI vers leurs bonnes positions:
    add  esi,ecx
    mov  edi,0A0000h
    ; Et on la garde bien au chaud dans un coin
    mov  MODE_X_Valeur_initiale_de_esi,esi
    mov  MODE_X_Valeur_initiale_de_edi,edi

    ; On lit le nombre de lignes … traiter:
    mov  dx,Hauteur

    APDLE_mode_X_Pour_chaque_ligne:

      ; On restaure la valeur de CX
      pop  cx
      push cx

      APDLE_mode_X_Pour_chaque_pixel:

	; On copie un pixel sur 4

	mov  eax,[esi]
	mov  [edi],al
	add  esi,4
	inc  edi

	add  cx,4
	cmp  cx,Largeur
	jb   APDLE_mode_X_Pour_chaque_pixel

      ; On saute la partie non visible de l'‚cran:
      mov  esi,MODE_X_Valeur_initiale_de_esi
      mov  edi,MODE_X_Valeur_initiale_de_edi
      add  esi,ebx
      add  di,MODE_X_Largeur_de_ligne
      mov  MODE_X_Valeur_initiale_de_esi,esi
      mov  MODE_X_Valeur_initiale_de_edi,edi

      dec  dx
      jnz  APDLE_mode_X_Pour_chaque_ligne


    pop  cx ; CX = nø du plan
    pop  esi
    dec  cx
    jns  APDLE_mode_X_Pour_chaque_plan


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_mode_X endp








; Afficher une ligne … l'‚cran

Afficher_une_ligne_a_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de EDI et le plan de d‚part dans CL:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov  ax,Largeur_ecran
  mov  dx,Pos_Y
  mov  cx,Pos_X
  mul  edx
  mov  edi,0A0000h
  add  eax,ecx
  shr  eax,2
  and  cx,3
  mov  di,ax
  mov  MODE_X_Valeur_initiale_de_edi,edi

  ; On met dans BX le nø d'ordre du plan trait‚ (pas le nø de plan):
  xor  ebx,ebx

  AULALE_mode_X_Pour_chacun_des_plans:

    ; On met dans ESI l'adresse de la ligne … copier:
    mov  esi,Ligne
    add  esi,ebx

    ; Et dans EDI l'adresse o— recopier
    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le plan en question:
    mov  dx,3C4h
    mov  ax,102h
    shl  ah,cl
    out  dx,ax
    push cx

    ; On met dans CX la largeur d‚j… trait‚e:
    mov  cx,bx

    AULALE_mode_X_Pour_chaque_pixel_du_plan:

      ; On v‚rifie qu'on ait pas d‚bord‚:
      cmp  cx,Largeur
      jae  AULALE_mode_X_Fin_du_traitement_du_plan

      ; On copie un pixel:
      mov  eax,[esi]
      mov  [edi],al
      add  esi,4
      inc  edi
      add  cx,4
      jmp  AULALE_mode_X_Pour_chaque_pixel_du_plan

    AULALE_mode_X_Fin_du_traitement_du_plan:

    ; On passe au plan suivant:
    pop  cx                                       ; -> Mise … jour de la
    inc  cx                                       ;    valeur du plan
    and  cx,3                                     ;
    jnz  AULALE_mode_X_Pas_de_changement_de_plan  ;
      inc  MODE_X_Valeur_initiale_de_edi          ;
    AULALE_mode_X_Pas_de_changement_de_plan:      ;
    inc  bx                                   ; -> Mise … jour du nø d'ordre
    cmp  bx,4                                 ;    du plan trait‚
    jne  AULALE_mode_X_Pour_chacun_des_plans  ;


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_a_l_ecran_mode_X endp





; Lire une ligne … l'‚cran et la stocker dans un buffer

Lire_une_ligne_a_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI et le plan de d‚part dans CL:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*Largeur_ecran)+Pos_X)
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov  ax,Largeur_ecran
  mov  dx,Pos_Y
  mov  cx,Pos_X
  mul  edx
  mov  esi,0A0000h
  add  eax,ecx
  shr  eax,2
  and  cx,3
  mov  si,ax
  mov  MODE_X_Valeur_initiale_de_esi,esi

  ; On met dans BX le nø d'ordre du plan trait‚ (pas le nø de plan):
  xor  ebx,ebx

  LULALE_mode_X_Pour_chacun_des_plans:

    ; On met dans EDI l'adresse de la ligne o— copier:
    mov  edi,Ligne
    add  edi,ebx

    ; Et dans ESI l'adresse … recopier
    mov  esi,MODE_X_Valeur_initiale_de_esi

    ; On passe dans le plan en question:
    mov  dx,3CEh                          ; DX = Port du TS (enfin, je crois)
    mov  ah,cl
    mov  al,04h
    out  dx,ax
    push cx

    ; On met dans CX la largeur d‚j… trait‚e:
    mov  cx,bx

    LULALE_mode_X_Pour_chaque_pixel_du_plan:

      ; On v‚rifie qu'on ait pas d‚bord‚:
      cmp  cx,Largeur
      jae  LULALE_mode_X_Fin_du_traitement_du_plan

      ; On copie un pixel:
      mov  al,[esi]
      mov  [edi],al
      inc  esi
      add  edi,4
      add  cx,4
      jmp  LULALE_mode_X_Pour_chaque_pixel_du_plan

    LULALE_mode_X_Fin_du_traitement_du_plan:

    ; On passe au plan suivant:
    pop  cx                                       ; -> Mise … jour de la
    inc  cx                                       ;    valeur du plan
    and  cx,3                                     ;
    jnz  LULALE_mode_X_Pas_de_changement_de_plan  ;
      inc  MODE_X_Valeur_initiale_de_esi          ;
    LULALE_mode_X_Pas_de_changement_de_plan:      ;
    inc  bx                                   ; -> Mise … jour du nø d'ordre
    cmp  bx,4                                 ;    du plan trait‚
    jne  LULALE_mode_X_Pour_chacun_des_plans  ;


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Lire_une_ligne_a_l_ecran_mode_X endp






; Afficher une partie de l'image zoom‚e … l'‚cran

Afficher_partie_de_l_ecran_zoomee_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Largeur:word,Hauteur:word,Largeur_image:word,Buffer:dword

  ; Largeur = Largeur non zoom‚e
  ; Hauteur = Hauteur zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Loupe_Decalage_Y*Largeur_image)+Loupe_Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Loupe_Decalage_Y
  mov  cx,Loupe_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans EDX la ligne en cours de traitement:
  xor  edx,edx

  ; On met dans EBX la distance entre deux lignes d'image
  xor  ebx,ebx
  mov  bx,Largeur_image

  APDLEZ_mode_X_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  cx,Loupe_Facteur

    APDLEZ_mode_X_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      push cx
      push dx
      push Buffer     ; Ligne
      push edi        ; Largeur*Facteur
      push edx        ; Pos_Y
      mov  ax,Principal_X_Zoom
      push eax        ; Pos_X
      call Afficher_une_ligne_a_l_ecran_mode_X
      add  esp,16
      pop  dx
      pop  cx

      ; On passe … la ligne suivante
      inc  dx                                 ; -> On v‚rifie qu'on ne soit
      cmp  dx,Hauteur                         ;   pas arriv‚ … la ligne
      je   APDLEZ_mode_X_Menu_Ordonnee_atteinte ;   terminale
      dec  cx                                 ; -> ou que l'on ai termin‚ de
      jnz  APDLEZ_mode_X_Pour_chaque_ligne      ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne image suivante
    add  esi,ebx
    jmp  APDLEZ_mode_X_Pour_chaque_ligne_a_zoomer

  APDLEZ_mode_X_Menu_Ordonnee_atteinte:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_zoomee_mode_X endp








;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;







; Afficher une ligne transparente en MCGA

Afficher_une_ligne_transparente_a_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  mov  ax,320
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  mov  di,ax

  ; On met dans AH la couleur de transparence:
  mov  ah,Couleur_transparence

  ; On met dans CX le nombre de pixels … traiter:
  mov  cx,Largeur

  ; Pour chaque pixel de la ligne
  AULTALE_Pour_chaque_pixel:

    ; On lit la source
    mov  al,[esi]
    inc  esi                                 ; MAJ du Pointeur de source

    ; On v‚rifie qu'elle soit <> de la couleur de transparence
    cmp  al,ah
    je   AULTALE_Pas_de_copie

      ; Et on la copie dans la destination
      mov  [edi],al
    AULTALE_Pas_de_copie:
    inc  edi                                 ; MAJ du Pointeur de destination

    ; Hop! et un de moins
    dec  cx
    jnz  AULTALE_Pour_chaque_pixel

  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_a_l_ecran_MCGA endp



; Afficher une ligne transparente en MCGA

Afficher_une_ligne_transparente_mono_a_l_ecran_MCGA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte,Couleur:byte

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  mov  ax,320
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  mov  di,ax

  ; On met dans AL la couleur de transparence:
  mov  al,Couleur_transparence

  ; On met dans DL la couleur … utiliser:
  mov  dl,Couleur

  ; On met dans CX le nombre de pixels … traiter:
  mov  cx,Largeur

  ; Pour chaque pixel de la ligne
  AULTMALE_Pour_chaque_pixel:

    ; On v‚rifie que la source soit <> de la couleur de transparence
    cmp  al,[esi]
    je   AULTMALE_Pas_de_copie

      ; Et on copie la couleur dans la destination
      mov  [edi],dl
    AULTMALE_Pas_de_copie:
    inc  esi                                 ; MAJ du Pointeur de source
    inc  edi                                 ; MAJ du Pointeur de destination

    ; Hop! et un de moins
    dec  cx
    jnz  AULTMALE_Pour_chaque_pixel

  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_mono_a_l_ecran_MCGA endp






; Afficher une ligne transparente … l'‚cran

Afficher_une_ligne_transparente_a_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:

  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*Largeur_ecran)+Pos_X)

  mov  ax,Largeur_ecran
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  adc  dx,0
  mov  di,ax
  ; Et on passe dans la banque correspondante
  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  ; On regarde si la ligne loge totalement dans la banque en cours
  add  cx,di
  jc   AULTALE_VESA_Debordement_de_ligne

  ; Si c'est le cas:
  mov  cx,Largeur     ; correction de CX … cause du test

  ; On effectue une copie de CX octets avec gestion de transparence
  mov  dh,Couleur_transparence
  AULTALE_VESA_No_break_Pour_chaque_pixel:
    mov  dl,[esi]
    cmp  dl,dh
    je   AULTALE_VESA_No_break_Pas_de_copie
      mov  [edi],dl
    AULTALE_VESA_No_break_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTALE_VESA_No_break_Pour_chaque_pixel

  jmp  AULTALE_VESA_Fin_traitement_ligne


  ; Si ce n'est pas le cas (d‚bordement de la ligne)
  AULTALE_VESA_Debordement_de_ligne:

  ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

  ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
  ; rupture:
  push cx

  ; On met dans CX le nb de pixels affichables avant la rupture:
  mov  cx,di
  neg  cx

  ; On effectue une copie de CX octets avec gestion de transparence
  mov  dh,Couleur_transparence
  AULTALE_VESA_BreakLeft_Pour_chaque_pixel:
    mov  dl,[esi]
    cmp  dl,dh
    je   AULTALE_VESA_BreakLeft_Pas_de_copie
      mov  [edi],dl
    AULTALE_VESA_BreakLeft_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTALE_VESA_BreakLeft_Pour_chaque_pixel

  ; On change de banque:
  inc  al
  call [VESA_Change_banque_ecriture]

  ; On replace EDI en d‚but de banque
  mov  edi,0A0000h

  ; On restaure le nb de pixels … afficher … droite de la rupture
  pop  cx
  or   cx,cx
  jz   AULTALE_VESA_Fin_traitement_ligne

  ; On effectue une copie de CX octets avec gestion de transparence
;  mov  dh,Couleur_transparence  ; LIGNE ECRITE PLUS HAUT, DH INCHANGE
  AULTALE_VESA_BreakRight_Pour_chaque_pixel:
    mov  dl,[esi]
    cmp  dl,dh
    je   AULTALE_VESA_BreakRight_Pas_de_copie
      mov  [edi],dl
    AULTALE_VESA_BreakRight_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTALE_VESA_BreakRight_Pour_chaque_pixel


  AULTALE_VESA_Fin_traitement_ligne:


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_a_l_ecran_VESA endp





; Afficher une ligne transparente mono … l'‚cran

Afficher_une_ligne_transparente_mono_a_l_ecran_VESA proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte,Couleur:byte

  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:

  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*Largeur_ecran)+Pos_X)

  mov  ax,Largeur_ecran
  mov  edi,0A0000h
  mul  Pos_Y
  add  ax,Pos_X
  adc  dx,0
  mov  di,ax
  ; Et on passe dans la banque correspondante
  mov  al,dl
  call [VESA_Change_banque_ecriture]

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  ; On regarde si la ligne loge totalement dans la banque en cours
  add  cx,di
  jc   AULTMALE_VESA_Debordement_de_ligne

  ; Si c'est le cas:
  mov  cx,Largeur     ; correction de CX … cause du test

  ; On effectue une copie de CX octets avec gestion de transparence
  mov  dl,Couleur_transparence
  mov  dh,Couleur
  AULTMALE_VESA_No_break_Pour_chaque_pixel:
    cmp  dl,[esi]
    je   AULTMALE_VESA_No_break_Pas_de_copie
      mov  [edi],dh
    AULTMALE_VESA_No_break_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTMALE_VESA_No_break_Pour_chaque_pixel

  jmp  AULTMALE_VESA_Fin_traitement_ligne


  ; Si ce n'est pas le cas (d‚bordement de la ligne)
  AULTMALE_VESA_Debordement_de_ligne:

  ; A ce stade, on sait que la ligne ne loge pas entiŠrement sur l'‚cran

  ; On m‚morise dans la pile ce qu'il restera … afficher … la droite de la
  ; rupture:
  push cx

  ; On met dans CX le nb de pixels affichables avant la rupture:
  mov  cx,di
  neg  cx

  ; On effectue une copie de CX octets avec gestion de transparence
  mov  dl,Couleur_transparence
  mov  dh,Couleur
  AULTMALE_VESA_BreakLeft_Pour_chaque_pixel:
    cmp  dl,[esi]
    je   AULTMALE_VESA_BreakLeft_Pas_de_copie
      mov  [edi],dh
    AULTMALE_VESA_BreakLeft_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTMALE_VESA_BreakLeft_Pour_chaque_pixel

  ; On change de banque:
  inc  al
  call [VESA_Change_banque_ecriture]

  ; On replace EDI en d‚but de banque
  mov  edi,0A0000h

  ; On restaure le nb de pixels … afficher … droite de la rupture
  pop  cx
  or   cx,cx
  jz   AULTMALE_VESA_Fin_traitement_ligne

  ; On effectue une copie de CX octets avec gestion de transparence
;  mov  dl,Couleur_transparence  ; LIGNE ECRITE PLUS HAUT, DL INCHANGE
;  mov  dh,Couleur               ; LIGNE ECRITE PLUS HAUT, DH INCHANGE
  AULTMALE_VESA_BreakRight_Pour_chaque_pixel:
    cmp  dl,[esi]
    je   AULTMALE_VESA_BreakRight_Pas_de_copie
      mov  [edi],dh
    AULTMALE_VESA_BreakRight_Pas_de_copie:
    inc  esi
    inc  edi
    dec  cx
    jnz  AULTMALE_VESA_BreakRight_Pour_chaque_pixel


  AULTMALE_VESA_Fin_traitement_ligne:


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_mono_a_l_ecran_VESA endp






; Afficher une ligne transparente … l'‚cran

Afficher_une_ligne_transparente_a_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de EDI et le plan de d‚part dans CL:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov  ax,Largeur_ecran
  mov  dx,Pos_Y
  mov  cx,Pos_X
  mul  edx
  mov  edi,0A0000h
  add  eax,ecx
  shr  eax,2
  and  cx,3
  mov  di,ax
  mov  MODE_X_Valeur_initiale_de_edi,edi

  ; On met dans BX le nø d'ordre du plan trait‚ (pas le nø de plan):
  xor  ebx,ebx

  AULTALE_mode_X_Pour_chacun_des_plans:

    ; On met dans ESI l'adresse de la ligne … copier:
    mov  esi,Ligne
    add  esi,ebx

    ; Et dans EDI l'adresse o— recopier
    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le plan en question:
    mov  dx,3C4h
    mov  ax,102h
    shl  ah,cl
    out  dx,ax
    push cx

    ; On met dans CX la largeur d‚j… trait‚e:
    mov  cx,bx

    ; On met dans AH la couleur de transparence
    mov  ah,Couleur_transparence

    AULTALE_mode_X_Pour_chaque_pixel_du_plan:

      ; On v‚rifie qu'on ai pas d‚bord‚:
      cmp  cx,Largeur
      jae  AULTALE_mode_X_Fin_du_traitement_du_plan

      ; On copie un pixel:
      mov  al,[esi]
      cmp  al,ah
      je   AULTALE_mode_X_Pas_de_copie
	mov  [edi],al
      AULTALE_mode_X_Pas_de_copie:
      add  esi,4
      inc  edi
      add  cx,4
      jmp  AULTALE_mode_X_Pour_chaque_pixel_du_plan

    AULTALE_mode_X_Fin_du_traitement_du_plan:

    ; On passe au plan suivant:
    pop  cx                                       ; -> Mise … jour de la
    inc  cx                                       ;    valeur du plan
    and  cx,3                                     ;
    jnz  AULTALE_mode_X_Pas_de_changement_de_plan  ;
      inc  MODE_X_Valeur_initiale_de_edi          ;
    AULTALE_mode_X_Pas_de_changement_de_plan:      ;
    inc  bx                                   ; -> Mise … jour du nø d'ordre
    cmp  bx,4                                 ;    du plan trait‚
    jne  AULTALE_mode_X_Pour_chacun_des_plans  ;


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_a_l_ecran_mode_X endp






; Afficher une ligne transparente mono … l'‚cran

Afficher_une_ligne_transparente_mono_a_l_ecran_mode_X proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte,Couleur:byte

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de EDI et le plan de d‚part dans CL:
  ;
  ; Valeur initiale=0A0000h+((Pos_Y*320)+Pos_X)
  xor  eax,eax
  xor  ecx,ecx
  xor  edx,edx
  mov  ax,Largeur_ecran
  mov  dx,Pos_Y
  mov  cx,Pos_X
  mul  edx
  mov  edi,0A0000h
  add  eax,ecx
  shr  eax,2
  and  cx,3
  mov  di,ax
  mov  MODE_X_Valeur_initiale_de_edi,edi

  ; On met dans BX le nø d'ordre du plan trait‚ (pas le nø de plan):
  xor  ebx,ebx

  AULTMALE_mode_X_Pour_chacun_des_plans:

    ; On met dans ESI l'adresse de la ligne … copier:
    mov  esi,Ligne
    add  esi,ebx

    ; Et dans EDI l'adresse o— recopier
    mov  edi,MODE_X_Valeur_initiale_de_edi

    ; On passe dans le plan en question:
    mov  dx,3C4h
    mov  ax,102h
    shl  ah,cl
    out  dx,ax
    push cx

    ; On met dans CX la largeur d‚j… trait‚e:
    mov  cx,bx

    ; On met dans AL la couleur de transparence
    mov  al,Couleur_transparence
    ; Et la couleur de coloriage dans AH
    mov  ah,Couleur

    AULTMALE_mode_X_Pour_chaque_pixel_du_plan:

      ; On v‚rifie qu'on ai pas d‚bord‚:
      cmp  cx,Largeur
      jae  AULTMALE_mode_X_Fin_du_traitement_du_plan

      ; On copie un pixel:
      cmp  al,[esi]
      je   AULTMALE_mode_X_Pas_de_copie
	mov  [edi],ah
      AULTMALE_mode_X_Pas_de_copie:
      add  esi,4
      inc  edi
      add  cx,4
      jmp  AULTMALE_mode_X_Pour_chaque_pixel_du_plan

    AULTMALE_mode_X_Fin_du_traitement_du_plan:

    ; On passe au plan suivant:
    pop  cx                                       ; -> Mise … jour de la
    inc  cx                                       ;    valeur du plan
    and  cx,3                                     ;
    jnz  AULTMALE_mode_X_Pas_de_changement_de_plan  ;
      inc  MODE_X_Valeur_initiale_de_edi          ;
    AULTMALE_mode_X_Pas_de_changement_de_plan:      ;
    inc  bx                                   ; -> Mise … jour du nø d'ordre
    cmp  bx,4                                 ;    du plan trait‚
    jne  AULTMALE_mode_X_Pour_chacun_des_plans  ;


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_mono_a_l_ecran_mode_X endp





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; Afficher une partie de la brosse zoom‚e en couleur en MCGA

Display_brush_Color_Zoom_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBCZ_MCGA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur_de_transparence
    push eax                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBCZ_MCGA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_a_l_ecran_MCGA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBCZ_MCGA_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBCZ_MCGA_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,12     ; (Largeur*Facteur + Ligne + Couleur de transparence)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBCZ_MCGA_Pour_chaque_ligne_a_zoomer

  DBCZ_MCGA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + ESI)


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Color_Zoom_MCGA endp







; Afficher une partie de la brosse zoom‚e en monochrome en MCGA

Display_brush_Mono_Zoom_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBMZ_MCGA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur
    mov  cl,Couleur_de_transparence
    push eax                                      ; Couleur d'affichage
    push ecx                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBMZ_MCGA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_mono_a_l_ecran_MCGA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBMZ_MCGA_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBMZ_MCGA_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBMZ_MCGA_Pour_chaque_ligne_a_zoomer

  DBMZ_MCGA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,20     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Mono_Zoom_MCGA endp





; Effacer une partie de la brosse zoom‚e en MCGA

Clear_brush_Zoom_MCGA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_image:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Decalage_Y*Largeur_image)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  CBZ_MCGA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    CBZ_MCGA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_a_l_ecran_MCGA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   CBZ_MCGA_Hauteur_atteinte          ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  CBZ_MCGA_Pour_chaque_ligne         ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,8      ; (Largeur*Facteur + Ligne)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_image ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  CBZ_MCGA_Pour_chaque_ligne_a_zoomer

  CBZ_MCGA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,12     ; (Largeur*Facteur + Ligne + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Clear_brush_Zoom_MCGA endp






;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; Afficher une partie de la brosse zoom‚e en couleur en mode X

Display_brush_Color_Zoom_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBCZ_mode_X_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur_de_transparence
    push eax                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBCZ_mode_X_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_a_l_ecran_mode_X
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBCZ_mode_X_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBCZ_mode_X_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,12     ; (Largeur*Facteur + Ligne + Couleur de transparence)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBCZ_mode_X_Pour_chaque_ligne_a_zoomer

  DBCZ_mode_X_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + ESI)


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Color_Zoom_mode_X endp







; Afficher une partie de la brosse zoom‚e en monochrome en mode X

Display_brush_Mono_Zoom_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBMZ_mode_X_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur
    mov  cl,Couleur_de_transparence
    push eax                                      ; Couleur d'affichage
    push ecx                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBMZ_mode_X_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_mono_a_l_ecran_mode_X
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBMZ_mode_X_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBMZ_mode_X_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBMZ_mode_X_Pour_chaque_ligne_a_zoomer

  DBMZ_mode_X_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,20     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Mono_Zoom_mode_X endp





; Effacer une partie de la brosse zoom‚e en mode X

Clear_brush_Zoom_mode_X proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_image:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Decalage_Y*Largeur_image)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  CBZ_mode_X_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    CBZ_mode_X_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_a_l_ecran_mode_X
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   CBZ_mode_X_Hauteur_atteinte          ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  CBZ_mode_X_Pour_chaque_ligne         ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,8      ; (Largeur*Facteur + Ligne)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_image ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  CBZ_mode_X_Pour_chaque_ligne_a_zoomer

  CBZ_mode_X_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,12     ; (Largeur*Facteur + Ligne + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Clear_brush_Zoom_mode_X endp






;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; Afficher une partie de la brosse zoom‚e en couleur en VESA

Display_brush_Color_Zoom_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBCZ_VESA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur_de_transparence
    push eax                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBCZ_VESA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_a_l_ecran_VESA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBCZ_VESA_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBCZ_VESA_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,12     ; (Largeur*Facteur + Ligne + Couleur de transparence)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBCZ_VESA_Pour_chaque_ligne_a_zoomer

  DBCZ_VESA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + ESI)


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Color_Zoom_VESA endp







; Afficher une partie de la brosse zoom‚e en monochrome en VESA

Display_brush_Mono_Zoom_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Brosse+((Decalage_Y*Largeur_brosse)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_brosse
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Brosse
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  DBMZ_VESA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    mov  al,Couleur
    mov  cl,Couleur_de_transparence
    push eax                                      ; Couleur d'affichage
    push ecx                                      ; Couleur de transparence
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    DBMZ_VESA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_mono_a_l_ecran_VESA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   DBMZ_VESA_Hauteur_atteinte         ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  DBMZ_VESA_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBMZ_VESA_Pour_chaque_ligne_a_zoomer

  DBMZ_VESA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,20     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Mono_Zoom_VESA endp





; Effacer une partie de la brosse zoom‚e en VESA

Clear_brush_Zoom_VESA proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Pos_Y_Fin:word,Couleur_de_transparence:byte,Largeur_image:word,Buffer:dword

  ; Largeur_brosse=Largeur r‚elle de la brosse
  ; Largeur = Largeur non zoom‚e

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  ;
  ; Valeur initiale=Principal_Ecran+((Decalage_Y*Largeur_image)+Decalage_X)

  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov  ax,Largeur_image
  mov  bx,Decalage_Y
  mov  cx,Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On met dans DI la valeur Largeur*Facteur
  mov  ax,Largeur
  mul  Loupe_Facteur
  mov  di,ax

  ; On met dans DX la ligne en cours de traitement:
  xor  dx,Pos_Y

  CBZ_VESA_Pour_chaque_ligne_a_zoomer:

    ; On eclate la ligne … zoomer:
    push dx
    mov  ax,Loupe_Facteur
    mov  cx,Largeur
    push ecx         ; Largeur
    push eax         ; Facteur
    push Buffer      ; Ligne zoom‚e
    push esi         ; Ligne originale
    call Zoomer_une_ligne
    add  esp,16
    pop  dx

    ; On sauve l'ancienne valeur de ESI:
    push esi

    ; On place dans ESI la position courante en Y
    mov  esi,edx

    ; On empile … l'avance les derniers paramŠtres qui ne changent jamais le
    ; long de la mˆme ligne zoom‚e:
    push Buffer                                   ; Ligne
    push edi                                      ; Largeur*Facteur

    ; On l'affiche Facteur fois … l'‚cran (sur des lignes cons‚cutives):
    mov  bx,Loupe_Facteur ; BX n'est pas foutu en l'air par la routine appel‚e

    CBZ_VESA_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_a_l_ecran_VESA
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                 ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                       ;   pas arriv‚ … la ligne
      je   CBZ_VESA_Hauteur_atteinte          ;   terminale
      dec  bx                                 ; -> ou que l'on ai termin‚ de
      jnz  CBZ_VESA_Pour_chaque_ligne         ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,8      ; (Largeur*Facteur + Ligne)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_image ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  CBZ_VESA_Pour_chaque_ligne_a_zoomer

  CBZ_VESA_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,12     ; (Largeur*Facteur + Ligne + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Clear_brush_Zoom_VESA endp






;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;






_TEXT ENDS
END

