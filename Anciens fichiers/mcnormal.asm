; -- Calcul de la meilleure couleur s'approchant d'une teinte particuliŠre --

Meilleure_couleur proc near

  push ebp
  mov  ebp,esp

  arg  Rouge:byte,Vert:byte,Bleu:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la palette
  mov  esi,offset Principal_Palette

  ; On place l'indice de la couleur au d‚but de la table d'exclusion de coul.
  mov  MC_Indice,offset Exclude_color

  ; EDI servira … stocker la meilleure diff‚rence
  mov  edi,07FFFFFFFh

  ; On se sert de CL pour compter la couleur en cours
  ; (et CH servira … stocker la meilleure couleur)
  xor  cx,cx

  ; On nettoie la partie haute de EAX:
  xor  eax,eax

  Meilleure_couleur_Pour_chaque_couleur:

    ; On v‚rifie que la couleur ne soit pas exclue
    mov  ebx,MC_Indice
    cmp  byte ptr [ebx],0
    jnz  Meilleure_couleur_Pas_mieux

    ; EBX sert … pointer la table pr‚calcul‚e
    mov  ebx,MC_Table_differences

    ; On calcule la diff‚rence:
    ;
    ; Le calcul s'effectue dans EDX (on y cumule les diff‚rences), en se
    ; servant de EBX pour pointer la table pr‚calcul‚e, et de AX comme
    ; indice dans la table

    ; On s'occupe de la composante Rouge
    mov  al,[esi]
    mov  dl,[esi+1]              ; et en entrelac‚ de la composante verte
    mov  ah,Rouge
    mov  dh,Vert
    sub  al,ah
    sub  dl,dh
    and  eax,127
    and  edx,127
    mov  edx,[ebx+4*edx+128*4]
    add  edx,[ebx+4*eax]

    ; On s'occupe de la composante Bleue
    mov  al,[esi+2]
    mov  ah,Bleu
    sub  al,ah
    and  eax,127
    add  edx,[ebx+4*eax+256*4]

    jz   Meilleure_couleur_Correspond_impeccablement

    cmp  edx,edi
    jae  Meilleure_couleur_Pas_mieux

      ; L…, on sait que CL contient une meilleure couleur que CH

      mov  ch,cl   ; On commence par mettre CH … CL
      mov  edi,edx ; Ensuite, on peut mettre DI (meilleure diff‚rence) … jour

    Meilleure_couleur_Pas_mieux:

    inc  MC_Indice
    add  esi,3

  inc  cl
  jnz  Meilleure_couleur_Pour_chaque_couleur

  ; Ici, on sait que CH contient d‚j… la meilleure couleur. On va donc faire
  ; une sortie rapide (c'est pas bien, mais ‡a fait gagner 3 cycles)

  mov  al,ch


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

  Meilleure_couleur_Correspond_impeccablement:

  ; Ici, on sait que CL contient la couleur qui correspond exactement aux
  ; exigences de l'appelant

  mov  al,cl


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Meilleure_couleur endp



; -- Calcul de la meilleure couleur s'approchant d'une teinte particuliŠre --
; -- et ceci SANS exclure certaines couleurs coch‚es par l'utilisateur --

Meilleure_couleur_sans_exclusion proc near

  push ebp
  mov  ebp,esp

  arg  Rouge:byte,Vert:byte,Bleu:byte

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de la palette
  mov  esi,offset Principal_Palette

  ; EDI servira … stocker la meilleure diff‚rence
  mov  edi,07FFFFFFFh

  ; On se sert de CL pour compter la couleur en cours
  ; (et CH servira … stocker la meilleure couleur)
  xor  cx,cx

  ; On nettoie la partie haute de EAX:
  xor  eax,eax

  ; EBX sert … pointer la table pr‚calcul‚e
  mov  ebx,MC_Table_differences

  MCSE_Pour_chaque_couleur:

    ; On calcule la diff‚rence:
    ;
    ; Le calcul s'effectue dans EDX (on y cumule les diff‚rences), en se
    ; servant de EBX pour pointer la table pr‚calcul‚e, et de AX comme
    ; indice dans la table

    ; On s'occupe de la composante Rouge
    mov  al,[esi]
    mov  dl,[esi+1]              ; et en entrelac‚ de la composante verte
    mov  ah,Rouge
    mov  dh,Vert
    sub  al,ah
    sub  dl,dh
    and  eax,127
    and  edx,127
    mov  edx,[ebx+4*edx+128*4]
    add  edx,[ebx+4*eax]

    ; On s'occupe de la composante Bleue
    mov  al,[esi+2]
    mov  ah,Bleu
    sub  al,ah
    and  eax,127
    add  edx,[ebx+4*eax+256*4]

    jz   MCSE_Correspond_impeccablement

    cmp  edx,edi
    jae  MCSE_Pas_mieux

      ; L…, on sait que CL contient une meilleure couleur que CH

      mov  ch,cl   ; On commence par mettre CH … CL
      mov  edi,edx ; Ensuite, on peut mettre DI (meilleure diff‚rence) … jour

    MCSE_Pas_mieux:

    add  esi,3

  inc  cl
  jnz  MCSE_Pour_chaque_couleur

  ; Ici, on sait que CH contient d‚j… la meilleure couleur. On va donc faire
  ; une sortie rapide (c'est pas bien, mais ‡a fait gagner 3 cycles)

  mov  al,ch


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

  MCSE_Correspond_impeccablement:

  ; Ici, on sait que CL contient la couleur qui correspond exactement aux
  ; exigences de l'appelant

  mov  al,cl


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Meilleure_couleur_sans_exclusion endp
