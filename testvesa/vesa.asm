.386P
.MODEL FLAT

_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA


; -- Fonctions VESA --
public Get_VESA_info
public Get_VESA_mode_info




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






_TEXT ENDS
END
