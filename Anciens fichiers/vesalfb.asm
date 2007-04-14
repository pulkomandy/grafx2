.386P
.MODEL FLAT


_DATA Segment dword public 'data'
      Assume cs:_TEXT, ds:_DATA



; ---- Variables export‚es ----

public LFB_Selecteur
public LFB_Adresse
public LFB_Taille



; ---- D‚claration des variables ----

  LFB_Selecteur dw 0     ; S‚lecteur utilis‚ par le mapping du LFB
  LFB_Adresse   dd 0     ; Adresse o— est mapp‚e le LFB
  LFB_Taille    dd 0     ; Taille de la m‚moire LFB

  Handle_temporaire dw 0 ; Handle de bloc m‚moire DOS utilis‚ temporairement


_DATA ENDS









_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA



; ---- Fonctions export‚es ----

; -- Fonctions VESA --
public Get_VESA_info
public Get_VESA_mode_info
public Get_VESA_protected_mode_WinFuncPtr
public Set_VESA_mode
public Initialiser_le_LFB
public Fermer_le_LFB

public Pixel_VESA_LFB
public Lit_pixel_VESA_LFB
public Effacer_tout_l_ecran_VESA_LFB
public Block_VESA_LFB
public Pixel_Preview_Normal_VESA_LFB
public Pixel_Preview_Loupe_VESA_LFB
public Ligne_horizontale_XOR_VESA_LFB
public Ligne_verticale_XOR_VESA_LFB
public Display_brush_Color_VESA_LFB
public Display_brush_Mono_VESA_LFB
public Clear_brush_VESA_LFB
public Remap_screen_VESA_LFB
public Afficher_partie_de_l_ecran_VESA_LFB
public Afficher_une_ligne_a_l_ecran_VESA_LFB
public Lire_une_ligne_a_l_ecran_VESA_LFB
public Afficher_partie_de_l_ecran_zoomee_VESA_LFB
public Display_brush_Color_Zoom_VESA_LFB
public Display_brush_Mono_Zoom_VESA_LFB
public Clear_brush_Zoom_VESA_LFB

; ---- Fonctions import‚es ----

; -- Fonctions DPMI --
extrn Physical_address_mapping     :near
extrn Free_physical_address_mapping:near
extrn Lock_linear_region           :near
extrn Unlock_linear_region         :near
extrn Allocate_ldt_descriptor      :near
extrn Free_ldt_descriptor          :near
extrn Set_segment_base_address     :near
extrn Set_segment_limit            :near
extrn Set_descriptor_access_rights :near
extrn Get_segment_base_address     :near

; -- Autres variables externes --
extrn Brosse:dword
extrn Principal_Ecran:dword
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

extrn VESA_WinFuncPtr:dword

; -- Proc‚dures externes --
extrn Zoomer_une_ligne:near




; - -- --- ----\/\ Lecture des informations VESA : /\/---- --- -- -

Convertir_adresse_MR_MP proc near

  ; EDI = Adresse d'une adresse … corriger

  push eax
  push ecx

  mov  eax,[edi] ; EAX = adresse … corriger
  mov  ecx,eax
  shr  eax,16
  shl  eax,4
  and  ecx,0FFFFh
  add  eax,ecx
  mov  [edi],eax

  pop  ecx
  pop  eax

  ret

Convertir_adresse_MR_MP endp



Get_VESA_info proc near

  push ebp
  mov  ebp,esp

  arg  Buffer:dword

  push ebx
  push esi
  push edi


  ; On commence par allouer un buffer en mode r‚el de 1024+64 octets
  mov  ax,0100h    ; "DPMI : Allocate Dos memory blocks"
  mov  bx,68d      ; BX = (1024+64)/16 = 68 paragraphes
  int  31h
  jc   GVI_Erreur

  ; On sauve l'handle du buffer dans la pile
  push dx

  ; On place dans EDX l'adresse du buffer
  xor  edx,edx
  mov  dx,ax
  shl  edx,4

  ; On nettoie ce(s) buffer(s)
  mov  edi,edx     ; On place en EDX
  xor  eax,eax     ; ... la valeur 0
  mov  ecx,0110h   ; ... sur 1024+64 positions
  rep  stosd       ; !!! Maintenant !!!

  ; On demande les infos VESA: (AX=4F00, ES:DI=Adresse du buffer)
  mov  edi,edx     ; Adresse des valeurs des registres … passer … l'interruption
  add  edx,40h     ; EDX=segment du buffer d'infos VESA
  shr  edx,4
  mov  dword ptr[edi+1Ch],00004F00h ; EAX pass‚ … l'interruption
  mov  dword ptr[edi+00h],00000000h ; EDI pass‚ … l'interruption
  mov   word ptr[edi+22h],dx        ; ES  pass‚ … l'interruption
  shl  edx,4
  mov  dword ptr[edx],32454256h     ; valeur hexa de "VBE2"
  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
  mov  bl,10h      ; Interruption … appeler (int vid‚o)
  xor  bh,bh       ; Flags de mode d'appel d'interruption
  xor  cx,cx       ; Nb d'elements de la pile … passer
  int  31h         ; Appel de l'interruption du DPMI

  ; On place dans EBX la valeur EAX de retour de l'interruption
  mov  ebx,dword ptr[edi+1Ch]

  ; On recopie le buffer du mode r‚el vers le mode prot‚g‚
  mov  ecx,256    ; (256 dword = 1024 octets)
  mov  esi,edx
  mov  edi,Buffer
  rep  movsd

  ; Puis on corrige chacune des adresses:
  mov  edi,Buffer
  add  edi,06h
  call Convertir_adresse_MR_MP               ; Fabricant
  add  edi,08h
  call Convertir_adresse_MR_MP               ; Liste des modes
  add  edi,08h
  call Convertir_adresse_MR_MP               ; Vendeur
  add  edi,04h
  call Convertir_adresse_MR_MP               ; Produit
  add  edi,04h
  call Convertir_adresse_MR_MP               ; R‚vision

  ; On libŠre le buffer en mode r‚el de 1024+64 octets
  mov  ax,0101h    ; "DPMI : Free Dos memory blocks"
  pop  dx          ; DX = handle du buffer
  int  31h
  jnc  GVI_Pas_d_erreur


  GVI_Erreur:

    mov  bx,0FF00h

  GVI_Pas_d_erreur:

  mov  ax,bx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Get_VESA_info endp






Get_VESA_mode_info proc near

  push ebp
  mov  ebp,esp

  arg  Mode:word,Buffer:dword

  push ebx
  push esi
  push edi


  ; On commence par allouer un buffer en mode r‚el de 256+64 octets
  mov  ax,0100h    ; "DPMI : Allocate Dos memory blocks"
  mov  bx,20d      ; BX = (256+64)/16 = 20 paragraphes
  int  31h
  jc   GVMI_Erreur

  ; On sauve l'handle du buffer dans la pile
  push dx

  ; On place dans EDX l'adresse du buffer
  xor  edx,edx
  mov  dx,ax
  shl  edx,4

  ; On nettoie ce(s) buffer(s)
  mov  edi,edx     ; On place en EDX
  xor  eax,eax     ; ... la valeur 0
  mov  ecx,0080d   ; ... sur 256+64 positions
  rep  stosd       ; !!! Maintenant !!!

  ; On demande les infos VESA: (AX=4F01, CX=Mode, ES:DI=Adresse du buffer)
  mov  edi,edx     ; Adresse des valeurs des registres … passer … l'interruption
  add  edx,40h     ; EDX=segment du buffer d'infos VESA
  shr  edx,4
  mov  ax,Mode     ; ECX=Mode vid‚o dont ont recherche les informations
  mov  dword ptr[edi+1Ch],00004F01h ; EAX pass‚ … l'interruption
  mov   word ptr[edi+18h],ax        ; ECX pass‚ … l'interruption
  mov  dword ptr[edi+00h],00000000h ; EDI pass‚ … l'interruption
  mov   word ptr[edi+22h],dx        ; ES  pass‚ … l'interruption
  shl  edx,4
  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
  mov  bl,10h      ; Interruption … appeler (int vid‚o)
  xor  bh,bh       ; Flags de mode d'appel d'interruption
  xor  cx,cx       ; Nb d'elements de la pile … passer
  int  31h         ; Appel de l'interruption du DPMI

  ; On place dans EBX la valeur EAX de retour de l'interruption
  mov  ebx,dword ptr[edi+1Ch]

  ; On recopie le buffer du mode r‚el vers le mode prot‚g‚
  mov  ecx,64      ; (64 dword = 256 octets)
  mov  esi,edx
  mov  edi,Buffer
  rep  movsd

  ; Puis on corrige chacune des adresses:
  mov  edi,Buffer
  add  edi,0Ch
  call Convertir_adresse_MR_MP               ; Adresse WinFuncPtr (Mode r‚el)

  ; On libŠre le buffer en mode r‚el de 256+64 octets
  mov  ax,0101h    ; "DPMI : Free Dos memory blocks"
  pop  dx          ; DX = handle du buffer
  int  31h
  jnc  GVMI_Pas_d_erreur


  GVMI_Erreur:

    mov  bx,0FF00h

  GVMI_Pas_d_erreur:

  mov  ax,bx


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Get_VESA_mode_info endp





; Renvoie dans EAX l'adresse de la fonction WinFuncPtr du mode prot‚g‚

Get_VESA_protected_mode_WinFuncPtr proc near

  push ebx
  push edi


  mov  VESA_WinFuncPtr,00000000h

  ; On commence par allouer un buffer en mode r‚el de 64 octets
  mov  ax,0100h    ; "DPMI : Allocate Dos memory blocks"
  mov  bx,4d       ; BX = 64/16 = 4 paragraphes
  int  31h
  jc   GVPMW_Erreur_d_allocation_de_memoire

  ; On sauve l'handle du buffer
  mov  Handle_temporaire,dx

  ; On place dans EDX l'adresse du buffer
  xor  edx,edx
  mov  dx,ax
  shl  edx,4

  ; On nettoie ce buffer
  mov  edi,edx     ; On place en EDX
  xor  eax,eax     ; ... la valeur 0
  mov  ecx,0016d   ; ... sur 64 positions
  rep  stosd       ; !!! Maintenant !!!

  ; On demande les infos VESA PMI: (AX=4F0A, BX=0)
  mov  ax,0300h    ; Service DPMI simuler une interruption en mode r‚el
  mov  bl,10h      ; Interruption … appeler (int vid‚o)
  xor  bh,bh       ; Flags de mode d'appel d'interruption
  xor  cx,cx       ; Nb d'elements de la pile … passer
  mov  edi,edx     ; Adresse des valeurs des registres … passer … l'interruption
  mov  dword ptr[edi+1Ch],00004F0Ah ; EAX pass‚ … l'interruption
  mov  dword ptr[edi+10h],00000000h ; EBX pass‚ … l'interruption
  int  31h         ; Appel de l'interruption du DPMI

  ; On v‚rifie que la valeur de AX de retour soit = 004Fh
  mov  eax,dword ptr[edi+1Ch]
  cmp  ax,004Fh
  jne  GVPMW_Fonction_VESA_non_supportee

  ; On sauve l'adresse dans VESA_WinFuncPtr
  xor  eax,eax
  xor  ecx,ecx
  mov   ax,word ptr[edi+34]  ; ES retourn‚ par l'interruption
  mov   cx,word ptr[edi+00]  ; DI retourn‚ par l'interruption
  shl  eax,4
  add  eax,ecx
  mov   cx,[eax]
  add  eax,ecx
  mov  VESA_WinFuncPtr,eax

  GVPMW_Fonction_VESA_non_supportee:

  ; On libŠre le buffer en mode r‚el de 64 octets
  mov  ax,0101h               ; "DPMI : Free Dos memory blocks"
  mov  dx,Handle_temporaire   ; DX = handle du buffer
  int  31h

  GVPMW_Erreur_d_allocation_de_memoire:


  pop  edi
  pop  ebx

  ret

Get_VESA_protected_mode_WinFuncPtr endp





; - -- --- ----\/\ Passage dans un mode vid‚o VESA : /\/---- --- -- -

Set_VESA_mode proc near

  push ebp
  mov  ebp,esp

  arg  Mode:word

  push ebx


  mov  ax,4F02h
  mov  bx,Mode
  int  10h


  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Set_VESA_mode endp





; - -- --- ----\/\ Initialiser le LFB d'un mode VESA 2.0 : /\/---- --- -- -

Initialiser_le_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Adresse_physique:dword,Taille:dword

  push ebx
  push esi
  push edi

  ; M‚morisation de la taille
  mov  eax,Taille
  mov  LFB_Taille,eax

  ; Allocation d'un descripteur LDT pour le mapping du LFB
  mov  eax,Offset LFB_Selecteur
  push eax
  mov  eax,1
  push eax
  call Allocate_ldt_descriptor
  add  esp,8

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_d_allocation_du_descripteur

  ; Modification des droits d'accŠs du descripteur
  mov  eax,00008092h
  push eax
  mov   ax,LFB_Selecteur
  push eax
  call Set_descriptor_access_rights
  add  esp,8

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_changement_des_droits_du_descripteur

  ; Mapping de l'adresse physique
  mov  eax,Offset LFB_Adresse
  push eax
  mov  eax,LFB_Taille
  push eax
  mov  eax,Adresse_physique
  push eax
  call Physical_address_mapping
  add  esp,12

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_mapping_de_l_adresse_physique

  ; Assignation de l'adresse de base du segment
  mov  eax,[eax]
  mov  eax,LFB_Adresse
  push eax
  mov   ax,LFB_Selecteur
  push eax
  call Set_segment_base_address
  add  esp,8

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_assignation_de_l_adresse_de_base_du_segment

  ; Assignation de la limite du segment
  mov  eax,LFB_Taille
  dec  eax
  push eax
  mov   ax,LFB_Selecteur
  push eax
  call Set_segment_limit
  add  esp,8

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_assignation_de_la_limite_du_segment

  ; V‚rouillage de la m‚moire lin‚aire
  mov  eax,LFB_Taille
  push eax
  mov  eax,LFB_Adresse
  push eax
  call Lock_linear_region
  add  esp,8

  or   ax,ax
  jnz  Initialiser_le_LFB_Erreur_verouillage_de_la_memoire_lineaire

  ; On renvoie un code d'erreur nul
  xor  eax,eax

  jmp  Initialiser_le_LFB_Fin_du_traitement


  Initialiser_le_LFB_Erreur_d_allocation_du_descripteur:
  Initialiser_le_LFB_Erreur_changement_des_droits_du_descripteur:
  Initialiser_le_LFB_Erreur_mapping_de_l_adresse_physique:
  Initialiser_le_LFB_Erreur_assignation_de_l_adresse_de_base_du_segment:
  Initialiser_le_LFB_Erreur_assignation_de_la_limite_du_segment:
  Initialiser_le_LFB_Erreur_verouillage_de_la_memoire_lineaire:

    mov  eax,1

  Initialiser_le_LFB_Fin_du_traitement:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Initialiser_le_LFB endp






; - -- --- ----\/\ Fermer le LFB d'un mode VESA 2.0 : /\/---- --- -- -

Fermer_le_LFB proc near

  ; D‚v‚rouillage de la m‚moire lin‚aire
  mov  eax,LFB_Taille
  push eax
  mov  eax,LFB_Adresse
  push eax
  call Unlock_linear_region
  add  esp,8

  or   ax,ax
  jnz  Fermer_le_LFB_Erreur_deverouillage_de_la_memoire_lineaire

  ; D‚mapping de l'adresse physique
  mov  eax,LFB_Adresse
  push eax
  call Free_physical_address_mapping
  add  esp,4

  or   ax,ax
  jnz  Fermer_le_LFB_Erreur_demapping_de_l_adresse_physique

  ; Lib‚ration du descripteur LDT pour le mapping du LFB
  mov   ax,LFB_Selecteur
  push eax
  call Free_ldt_descriptor
  add  esp,4

  or   ax,ax
  jnz  Fermer_le_LFB_Erreur_de_liberation_du_descripteur

  ; On renvoie un code d'erreur nul
  xor  eax,eax

  jmp  Fermer_le_LFB_Fin_du_traitement


  Fermer_le_LFB_Erreur_deverouillage_de_la_memoire_lineaire:
  Fermer_le_LFB_Erreur_demapping_de_l_adresse_physique:
  Fermer_le_LFB_Erreur_de_liberation_du_descripteur:

    mov  eax,1

  Fermer_le_LFB_Fin_du_traitement:


  ret

Fermer_le_LFB endp














Pixel_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Largeur_ecran
  mov   dx,Y
  mov   cx,X
  mul  edx
  add  eax,ecx
  mov  cl,Couleur
  add  eax,LFB_Adresse
  mov  [eax],cl


  mov  esp,ebp
  pop  ebp

  ret

Pixel_VESA_LFB endp




Lit_pixel_VESA_LFB proc near

  push  ebp
  mov   ebp,esp

  arg   X:word,Y:word


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Largeur_ecran
  mov   dx,Y
  mov   cx,X
  mul  edx
  add  eax,ecx
  add  eax,LFB_Adresse
  mov  al,[eax]


  mov   esp,ebp
  pop   ebp

  ret

Lit_pixel_VESA_LFB endp




Effacer_tout_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte

  push edi


  ; On place EDI en d‚but d'‚cran:

  mov  edi,LFB_Adresse

  ; On met ECX … Largeur_ecran*Menu_Ordonnee/4:

  xor  eax,eax
  xor  edx,edx
  mov   ax,Largeur_ecran
  mov   dx,Menu_Ordonnee
  mul  edx
  shr  eax,2
  mov  ecx,eax

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

Effacer_tout_l_ecran_VESA_LFB endp





; - -- --- ----\/\ Afficher un bloc de couleur en FLAT : /\/---- --- -- -

Block_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word,Hauteur:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) dans la destination
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On met dans EAX 4 fois la couleur
  mov   cl,Couleur
  mov   ch,cl
  mov   ax,cx
  shl   eax,16
  mov   ax,cx

  ; On place dans DX le nb de lignes … traiter
  mov  dx,Hauteur

  ; On met dans EBX la distance entre 2 lignes de destination
  xor  ebx,ebx
  mov   bx,Largeur_ecran
  sub   bx,Largeur

  ; On met dans ESI la largeur … traiter:
  xor  esi,esi
  mov   si,Largeur

  Block_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov  ecx,esi

    ; On fait une copie de la ligne
    shr  cx,1
    jnc  Block_VESA_LFB_Multiple_de_2
      stosb
    Block_VESA_LFB_Multiple_de_2:
    shr  cx,1
    jnc  Block_VESA_LFB_Multiple_de_4
      stosw
    Block_VESA_LFB_Multiple_de_4:
    repne stosd

    ; On passe … la ligne suivante
    add  edi,ebx
    dec  dx
    jnz  Block_VESA_LFB_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Block_VESA_LFB endp







; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode normal (pas en mode loupe) --

; Note: si on modifie cette proc‚dure, il faudra penser … faire ‚galement la
;       modif dans la proc‚dure Pixel_Preview_Loupe_VESA_LFB.

Pixel_Preview_Normal_VESA_LFB proc near

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

  call Pixel_VESA_LFB


  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Normal_VESA_LFB endp




; -- Affichage d'un pixel dans l'‚cran, par rapport au d‚calage de l'image
;    dans l'‚cran, en mode loupe --

Pixel_Preview_Loupe_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx


  ; On affiche d'abord le pixel dans la partie non-zoom‚e

  mov  dl,Couleur               ; Ca ne co–te pas trop
  mov  cx,Y                     ; en code de recopier
  mov  ax,X                     ; le contenu de la
  sub  cx,Principal_Decalage_Y  ; proc‚dure
  sub  ax,Principal_Decalage_X  ; Pixel_Preview_Normal_VESA_LFB
  push edx                      ; et ‡a fait gagner du
  push ecx                      ; temps, et ce temps est
  push eax                      ; assez pr‚cieux vu que
  call Pixel_VESA_LFB           ; c'est appel‚ TRES souvent.
  add  esp,12  ; 3 paramŠtres dword

  ; On regarde si on peut afficher le pixel ‚galement dans la loupe

  mov  ax,Y
  cmp  ax,Limite_Haut_Zoom
  jb   Pixel_Preview_Loupe_VESA_LFB_Fin
  cmp  ax,Limite_visible_Bas_Zoom
  ja   Pixel_Preview_Loupe_VESA_LFB_Fin
  mov  ax,X
  cmp  ax,Limite_Gauche_Zoom
  jb   Pixel_Preview_Loupe_VESA_LFB_Fin
  cmp  ax,Limite_visible_Droite_Zoom
  ja   Pixel_Preview_Loupe_VESA_LFB_Fin

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
  ja   Pixel_Preview_Loupe_VESA_LFB_Tronque_en_hauteur

  mov  cx,Loupe_Facteur
  push ecx
  jmp  Pixel_Preview_Loupe_VESA_LFB_Traitement_de_la_largeur

  Pixel_Preview_Loupe_VESA_LFB_Tronque_en_hauteur:

  mov  cx,dx
  neg  cx
  add  cx,Menu_Ordonnee
  push ecx

  ; On s'occupe de la largeur:

  Pixel_Preview_Loupe_VESA_LFB_Traitement_de_la_largeur:

  add  bx,Principal_X_Zoom

  mov  ax,Loupe_Facteur
  push eax

  ; On passe le d‚but en Y:
  push edx

  ; On passe le d‚but en X:
  push ebx

  call Block_VESA_LFB
  add  esp,20                   ; 20 = 5 paramŠtres de type dword


  Pixel_Preview_Loupe_VESA_LFB_Fin:


  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Pixel_Preview_Loupe_VESA_LFB endp




; - -- ---\/\ Afficher une ligne horizontale XOR en VESA LFB : /\/--- -- -

Ligne_horizontale_XOR_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word

  push ebx
  push edi


  ; On calcule la valeur initiale de EDI:
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov  cx,Largeur

  shr  cx,1
  jnc  Ligne_horizontale_XOR_VESA_LFB_CX_multiple_de_2
    not  byte ptr[edi]         ; On commence par se d‚barrasser du 1er byte
    inc  edi                   ; en cas d'imparit‚
  Ligne_horizontale_XOR_VESA_LFB_CX_multiple_de_2:
  shr  cx,1
  jnc  Ligne_horizontale_XOR_VESA_LFB_CX_multiple_de_4
    not  word ptr[edi]         ; On se d‚barrasse des 2Šme et 3Šme bytes en
    add  edi,2                 ; cas de "non-multiplicit‚-de-4"
  Ligne_horizontale_XOR_VESA_LFB_CX_multiple_de_4:

  or   cx,cx
  jz   Ligne_horizontale_XOR_VESA_LFB_Fin

  ; copie 32 bits
  Ligne_horizontale_XOR_VESA_LFB_Copie_32_bits:
    not  dword ptr[edi]
    add  edi,4
    dec  cx
  jnz  Ligne_horizontale_XOR_VESA_LFB_Copie_32_bits

  Ligne_horizontale_XOR_VESA_LFB_Fin:


  pop  edi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Ligne_horizontale_XOR_VESA_LFB endp






; - -- ---\/\ Afficher une ligne verticale XOR en VESA LFB : /\/--- -- -

Ligne_verticale_XOR_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Hauteur:word


  ; On calcule la valeur initiale de EDX:
  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   dx,Largeur_ecran
  mov   cx,Pos_X
  mul  edx
  mov  edx,LFB_Adresse
  add  eax,ecx
  add  edx,eax

  ; On met dans EAX la distance entre deux lignes:
  xor  eax,eax
  mov   ax,Largeur_ecran

  ; On met dans CX le nombre de lignes … traiter
  mov  cx,Hauteur

  Ligne_verticale_XOR_VESA_LFB_Boucle:

    not  byte ptr[edx]
    add  edx,eax

    dec  cx
    jnz  Ligne_verticale_XOR_VESA_LFB_Boucle


  mov  esp,ebp
  pop  ebp

  ret

Ligne_verticale_XOR_VESA_LFB endp






; Afficher une partie de la brosse en couleur en VESA LFB

Display_brush_Color_VESA_LFB proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   dx,Largeur_ecran
  mov   cx,Pos_X
  mul  edx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; Calculer dans ESI les coordonn‚es (Decalage_X,Decalage_Y) dans la brosse
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov    ax,Largeur_brosse
  mov    bx,Decalage_Y
  mov    cx,Decalage_X
  mul   ebx
  mov   esi,Brosse
  add   eax,ecx
  add   esi,eax

  ; On place dans DX le nb de lignes … traiter
  mov   dx,Hauteur

  ; On place dans BH la couleur de transparence
  mov   bh,Couleur_de_transparence

  ; On place dans EAX la distance entre deux lignes … l'‚cran
  xor   eax,eax
  mov    ax,Largeur_ecran
  sub    ax,Largeur

  Display_brush_Color_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Display_brush_Color_VESA_LFB_Pour_chaque_pixel:

      ; On lit le contenu de la brosse
      mov   bl,[esi]

      ; Gestion de la transparence
      cmp   bl,bh
      je    Display_brush_Color_VESA_LFB_Pas_de_copie

        ; On affiche le pixel de la brosse … l'‚cran
        mov   [edi],bl

      Display_brush_Color_VESA_LFB_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc   edi
      dec   cx
      jnz   Display_brush_Color_VESA_LFB_Pour_chaque_pixel

    ; On passe … la ligne suivante
    mov   cx,Largeur_brosse
    add   edi,eax
    sub   cx,Largeur
    add   esi,ecx
    dec   dx
    jnz   Display_brush_Color_VESA_LFB_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Color_VESA_LFB endp





; Afficher une partie de la brosse en monochrome en VESA LFB

Display_brush_Mono_VESA_LFB proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Couleur:byte,Largeur_brosse:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   dx,Largeur_ecran
  mov   cx,Pos_X
  mul  edx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

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
  xor   eax,eax
  mov    ax,Largeur_ecran
  sub    ax,Largeur

  Display_brush_Mono_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Display_brush_Mono_VESA_LFB_Pour_chaque_pixel:

      ; On v‚rifie que le contenu de la brosse ne soit pas transparent
      cmp   [esi],bh
      je    Display_brush_Mono_VESA_LFB_Pas_de_copie

        ; On affiche le pixel de la brosse … l'‚cran
        mov   [edi],bl

      Display_brush_Mono_VESA_LFB_Pas_de_copie:

      ; On passe au pixel suivant
      inc   esi
      inc   edi
      dec   cx
      jnz   Display_brush_Mono_VESA_LFB_Pour_chaque_pixel

    ; On passe … la ligne suivante
    mov   cx,Largeur_brosse
    add   edi,eax
    sub   cx,Largeur
    add   esi,ecx
    dec   dx
    jnz   Display_brush_Mono_VESA_LFB_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Display_brush_Mono_VESA_LFB endp





; Effacer la partie de la brosse affich‚e … l'‚cran en VESA LFB

Clear_brush_VESA_LFB proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Decalage_X:word,Decalage_Y:word,Largeur:word,Hauteur:word,Couleur_de_transparence:byte,Largeur_image:word

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   dx,Largeur_ecran
  mov   cx,Pos_X
  mul  edx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

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
  xor   ebx,ebx
  mov    bx,Largeur_ecran
  sub    bx,Largeur

  ; On met dans EAX la distance entre 2 lignes dans l'image
  xor   eax,eax
  mov    ax,Largeur_image
  sub    ax,Largeur

  ; On nettoie la partie haute de ECX:
  xor   ecx,ecx

  Clear_brush_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    ; On fait une copie de la ligne
    shr   cx,1
    jnc   Clear_brush_VESA_LFB_Multiple_de_2
      movsb
    Clear_brush_VESA_LFB_Multiple_de_2:
    shr   cx,1
    jnc   Clear_brush_VESA_LFB_Multiple_de_4
      movsw
    Clear_brush_VESA_LFB_Multiple_de_4:
    rep   movsd

    ; On passe … la ligne suivante
    add   esi,eax
    add   edi,ebx
    dec   dx
    jnz   Clear_brush_VESA_LFB_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Clear_brush_VESA_LFB endp






; Remapper une partie de l'‚cran en VESA LFB

Remap_screen_VESA_LFB proc near

  push  ebp
  mov   ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Hauteur:word,Conversion:dword

  push  ebx
  push  esi
  push  edi


  ; Calculer dans EDI les coordonn‚es (Pos_X,Pos_Y) … l'‚cran
  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   dx,Largeur_ecran
  mov   cx,Pos_X
  mul  edx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On place dans DX le nb de lignes … traiter
  mov   dx,Hauteur

  ; On place dans EBX l'adresse de la table de conversion
  mov   ebx,Conversion

  ; On nettoie (entre autres) la partie haute de EAX
  xor   eax,eax

  ; On met dans ESI la distance entre la fin d'une ligne et le d‚but de la
  ; suivante
  xor   esi,esi
  mov    si,Largeur_ecran
  sub    si,Largeur

  Remap_screen_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov   cx,Largeur

    Remap_screen_VESA_LFB_Pour_chaque_pixel:

      mov  al,[edi]
      mov  al,[ebx+eax]
      mov  [edi],al

      ; On passe au pixel suivant
      inc  edi
      dec  cx
      jnz  Remap_screen_VESA_LFB_Pour_chaque_pixel

    ; On passe … la ligne suivante
    add  edi,esi

    dec  dx
    jnz  Remap_screen_VESA_LFB_Pour_chaque_ligne


  pop   edi
  pop   esi
  pop   ebx

  mov   esp,ebp
  pop   ebp

  ret

Remap_screen_VESA_LFB endp





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Afficher une partie de l'image tel quel sur l'‚cran

Afficher_partie_de_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Largeur:word,Hauteur:word,Largeur_image:word

  push ebx
  push esi
  push edi


  ; Calculer dans EDI les coordonn‚es (0,0) dans la destination
  mov  edi,LFB_Adresse

  ; Calculer dans ESI les coordonn‚es (Pos_X_source,Pos_Y_source)
  ; dans la source
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Principal_Decalage_Y
  mov   bx,Largeur_image
  mov   cx,Principal_Decalage_X
  mul  ebx
  mov  esi,Principal_Ecran
  add  eax,ecx
  add  esi,eax

  ; On place dans DX le nb de lignes … traiter
  mov  dx,Hauteur

  ; On met dans EBX la distance entre 2 lignes de la destination
  xor  ebx,ebx
  mov   bx,Largeur_ecran
  sub   bx,Largeur

  ; On met dans EAX la distance entre 2 lignes de la source
  xor  eax,eax
  mov   ax,Largeur_image
  sub   ax,Largeur

  ; On nettoie la partie haute de ECX:
  xor  ecx,ecx

  Afficher_partie_de_l_ecran_VESA_LFB_Pour_chaque_ligne:

    ; On place dans CX le nb de pixels … traiter sur la ligne
    mov  cx,Largeur

    ; On fait une copie de la ligne
    shr  cx,1
    jnc  Afficher_partie_de_l_ecran_VESA_LFB_Multiple_de_2
      movsb
    Afficher_partie_de_l_ecran_VESA_LFB_Multiple_de_2:
    shr  cx,1
    jnc  Afficher_partie_de_l_ecran_VESA_LFB_Multiple_de_4
      movsw
    Afficher_partie_de_l_ecran_VESA_LFB_Multiple_de_4:
    rep  movsd

    ; On passe … la ligne suivante
    add  esi,eax
    add  edi,ebx
    dec  dx
    jnz  Afficher_partie_de_l_ecran_VESA_LFB_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_VESA_LFB endp







; Afficher une ligne … l'‚cran

Afficher_une_ligne_a_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push ebx
  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov   cx,Largeur

  ; On les traite au mieux par copie 8/16/32 bits
  shr  cx,1
  jnc  Afficher_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_2
    movsb
  Afficher_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_2:
  shr  cx,1
  jnc  Afficher_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_4
    movsw
  Afficher_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_4:
  rep  movsd


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_a_l_ecran_VESA_LFB endp













; Lire une ligne … l'‚cran et la stocker dans un buffer


Lire_une_ligne_a_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword

  push ebx
  push esi
  push edi


  ; On calcule la valeur initiale de ESI:
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  esi,LFB_Adresse
  add  eax,ecx
  add  esi,eax

  ; On met dans EDI l'adresse de la ligne … copier:
  mov  edi,Ligne

  ; On met dans ECX le nombre de pixels … traiter:
  xor  ecx,ecx
  mov   cx,Largeur

  ; On les traite au mieux par copie 8/16/32 bits
  shr  cx,1
  jnc  Lire_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_2
    movsb
  Lire_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_2:
  shr  cx,1
  jnc  Lire_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_4
    movsw
  Lire_une_ligne_a_l_ecran_VESA_LFB_ECX_multiple_de_4:
  rep  movsd


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Lire_une_ligne_a_l_ecran_VESA_LFB endp





; Afficher une partie de l'image zoom‚e … l'‚cran

Afficher_partie_de_l_ecran_zoomee_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Largeur:word,Hauteur:word,Largeur_image:word,Buffer:dword

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

  APDLEZ_VESA_LFB_Pour_chaque_ligne_a_zoomer:

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

    APDLEZ_VESA_LFB_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      push cx
      push dx
      push Buffer     ; Ligne
      push edi        ; Largeur*Facteur
      push edx        ; Pos_Y
      mov  ax,Principal_X_Zoom
      push eax        ; Pos_X
      call Afficher_une_ligne_a_l_ecran_VESA_LFB
      add  esp,16
      pop  dx
      pop  cx

      ; On passe … la ligne suivante
      inc  dx                                     ; -> On v‚rifie qu'on ne soit
      cmp  dx,Hauteur                             ;   pas arriv‚ … la ligne
      je   APDLEZ_VESA_LFB_Menu_Ordonnee_atteinte ;   terminale
      dec  cx                                     ; -> ou que l'on ai termin‚ de
      jnz  APDLEZ_VESA_LFB_Pour_chaque_ligne      ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne image suivante
    add  esi,ebx
    jmp  APDLEZ_VESA_LFB_Pour_chaque_ligne_a_zoomer

  APDLEZ_VESA_LFB_Menu_Ordonnee_atteinte:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_partie_de_l_ecran_zoomee_VESA_LFB endp





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;





; Afficher une ligne transparente en VESA LFB

Afficher_une_ligne_transparente_a_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte

  push ebx
  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On met dans AH la couleur de transparence:
  mov  ah,Couleur_transparence

  ; On met dans CX le nombre de pixels … traiter:
  mov  cx,Largeur

  ; Pour chaque pixel de la ligne
  AULTALE_VESA_LFB_Pour_chaque_pixel:

    ; On lit la source
    mov  al,[esi]
    inc  esi                                 ; MAJ du Pointeur de source

    ; On v‚rifie qu'elle soit <> de la couleur de transparence
    cmp  al,ah
    je   AULTALE_VESA_LFB_Pas_de_copie

      ; Et on la copie dans la destination
      mov  [edi],al

    AULTALE_VESA_LFB_Pas_de_copie:
    inc  edi                                 ; MAJ du Pointeur de destination

    ; Hop! et un de moins
    dec  cx
    jnz  AULTALE_VESA_LFB_Pour_chaque_pixel

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_a_l_ecran_VESA_LFB endp



; Afficher une ligne transparente en VESA LFB

Afficher_une_ligne_transparente_mono_a_l_ecran_VESA_LFB proc near

  push ebp
  mov  ebp,esp

  arg   Pos_X:word,Pos_Y:word,Largeur:word,Ligne:dword,Couleur_transparence:byte,Couleur:byte

  push ebx
  push esi
  push edi


  ; On met dans ESI l'adresse de la ligne … copier:
  mov  esi,Ligne

  ; On calcule la valeur initiale de EDI:
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Pos_Y
  mov   bx,Largeur_ecran
  mov   cx,Pos_X
  mul  ebx
  mov  edi,LFB_Adresse
  add  eax,ecx
  add  edi,eax

  ; On met dans AL la couleur de transparence:
  mov  al,Couleur_transparence

  ; On met dans DL la couleur … utiliser:
  mov  dl,Couleur

  ; On met dans CX le nombre de pixels … traiter:
  mov  cx,Largeur

  ; Pour chaque pixel de la ligne
  AULTMALE_VESA_LFB_Pour_chaque_pixel:

    ; On v‚rifie que la source soit <> de la couleur de transparence
    cmp  al,[esi]
    je   AULTMALE_VESA_LFB_Pas_de_copie

      ; Et on copie la couleur dans la destination
      mov  [edi],dl

    AULTMALE_VESA_LFB_Pas_de_copie:
    inc  esi                                 ; MAJ du Pointeur de source
    inc  edi                                 ; MAJ du Pointeur de destination

    ; Hop! et un de moins
    dec  cx
    jnz  AULTMALE_VESA_LFB_Pour_chaque_pixel


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Afficher_une_ligne_transparente_mono_a_l_ecran_VESA_LFB endp




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; Afficher une partie de la brosse zoom‚e en couleur en VESA LFB

Display_brush_Color_Zoom_VESA_LFB proc near

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

  DBCZ_VESA_LFB_Pour_chaque_ligne_a_zoomer:

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

    DBCZ_VESA_LFB_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_a_l_ecran_VESA_LFB
      add  esp,8    ; (Pos_X & Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                     ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                           ;   pas arriv‚ … la ligne
      je   DBCZ_VESA_LFB_Hauteur_atteinte         ;   terminale
      dec  bx                                     ; -> ou que l'on ai termin‚ de
      jnz  DBCZ_VESA_LFB_Pour_chaque_ligne        ;   traiter la ligne … zoomer

    ; On passe … la ligne … zoomer suivante:
    ; sans oublier de passer … la ligne brosse suivante

    ; On commence par retirer les paramŠtres pr‚c‚demment pass‚s
    add  esp,12     ; (Largeur*Facteur & Ligne & Couleur de transparence)

    ; On sauve la ligne courante dans DX:
    mov  edx,esi

    ; On restaure l'ancienne valeur de ESI:
    pop  esi

    mov  bx,Largeur_brosse ; En th‚orie, la partie haute de EBX est propre
    add  esi,ebx
    jmp  DBCZ_VESA_LFB_Pour_chaque_ligne_a_zoomer

  DBCZ_VESA_LFB_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,16     ; (Largeur*Facteur + Ligne + Couleur de transparence + ESI)


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Color_Zoom_VESA_LFB endp







; Afficher une partie de la brosse zoom‚e en monochrome en VESA LFB

Display_brush_Mono_Zoom_VESA_LFB proc near

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

  DBMZ_VESA_LFB_Pour_chaque_ligne_a_zoomer:

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

    DBMZ_VESA_LFB_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_transparente_mono_a_l_ecran_VESA_LFB
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                     ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                           ;   pas arriv‚ … la ligne
      je   DBMZ_VESA_LFB_Hauteur_atteinte         ;   terminale
      dec  bx                                     ; -> ou que l'on ai termin‚ de
      jnz  DBMZ_VESA_LFB_Pour_chaque_ligne        ;   traiter la ligne … zoomer

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
    jmp  DBMZ_VESA_LFB_Pour_chaque_ligne_a_zoomer

  DBMZ_VESA_LFB_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,20     ; (Largeur*Facteur + Ligne + Couleur de transparence + Couleur + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Display_brush_Mono_Zoom_VESA_LFB endp





; Effacer une partie de la brosse zoom‚e en VESA LFB

Clear_brush_Zoom_VESA_LFB proc near

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

  CBZ_VESA_LFB_Pour_chaque_ligne_a_zoomer:

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

    CBZ_VESA_LFB_Pour_chaque_ligne:

      ; On affiche la ligne zoom‚e
      mov  ax,Pos_X
      push esi                                      ; Pos_Y
      push eax                                      ; Pos_X
      call Afficher_une_ligne_a_l_ecran_VESA_LFB
      add  esp,8    ; (Pos_X+Pos_Y)

      ; On passe … la ligne suivante
      inc  si                                     ; -> On v‚rifie qu'on ne soit
      cmp  si,Pos_Y_Fin                           ;   pas arriv‚ … la ligne
      je   CBZ_VESA_LFB_Hauteur_atteinte          ;   terminale
      dec  bx                                     ; -> ou que l'on ai termin‚ de
      jnz  CBZ_VESA_LFB_Pour_chaque_ligne         ;   traiter la ligne … zoomer

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
    jmp  CBZ_VESA_LFB_Pour_chaque_ligne_a_zoomer

  CBZ_VESA_LFB_Hauteur_atteinte:

  ; On ramŠne la pile dans un ‚tat normal
  add  esp,12     ; (Largeur*Facteur + Ligne + ESI)

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Clear_brush_Zoom_VESA_LFB endp






_TEXT ENDS
END



