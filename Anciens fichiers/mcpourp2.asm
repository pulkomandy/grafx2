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

  Meilleure_couleur_Pour_chaque_couleur:

    mov  ebx,MC_Indice
    cmp  byte ptr [ebx],0
    jnz  Meilleure_couleur_Pas_mieux

    ; On calcule la diff‚rence:

    ; On s'occupe de la composante Rouge
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi]
    mov  al,Rouge
    sub  eax,ebx
    imul eax
    mov  MC_DR,eax

    ; On s'occupe de la composante Verte
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi+1]
    mov  al,Vert
    sub  eax,ebx
    imul eax
    mov  MC_DV,eax

    ; On s'occupe de la composante Bleue
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi+2]
    mov  al,Bleu
    sub  eax,ebx
    imul eax
    mov  MC_DB,eax

    ; On totalise la "distance" dans EBX
    mov  eax,30
    mul  MC_DR
    mov  ebx,eax

    mov  eax,59
    mul  MC_DV
    add  ebx,eax

    mov  eax,11
    mul  MC_DB
    add  ebx,eax
    jz   Meilleure_couleur_Correspond_impeccablement

    cmp  ebx,edi
    jae  Meilleure_couleur_Pas_mieux

      ; L…, on sait que CL contient une meilleure couleur que CH

      mov  ch,cl   ; On commence par mettre CH … CL
      mov  edi,ebx ; Ensuite, on peut mettre DI (meilleure diff‚rence) … jour

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

  MCSE_Pour_chaque_couleur:

    ; On calcule la diff‚rence:

    ; On s'occupe de la composante Rouge
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi]
    mov  al,Rouge
    sub  eax,ebx
    imul eax
    mov  MC_DR,eax

    ; On s'occupe de la composante Verte
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi+1]
    mov  al,Vert
    sub  eax,ebx
    imul eax
    mov  MC_DV,eax

    ; On s'occupe de la composante Bleue
    xor  ebx,ebx
    xor  eax,eax
    mov  bl,[esi+2]
    mov  al,Bleu
    sub  eax,ebx
    imul eax
    mov  MC_DB,eax

    ; On totalise la "distance" dans EBX
    mov  eax,30
    mul  MC_DR
    mov  ebx,eax

    mov  eax,59
    mul  MC_DV
    add  ebx,eax

    mov  eax,11
    mul  MC_DB
    add  ebx,eax
    jz   MCSE_Correspond_impeccablement

    cmp  ebx,edi
    jae  MCSE_Pas_mieux

      ; L…, on sait que CL contient une meilleure couleur que CH

      mov  ch,cl   ; On commence par mettre CH … CL
      mov  edi,ebx ; Ensuite, on peut mettre DI (meilleure diff‚rence) … jour

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
