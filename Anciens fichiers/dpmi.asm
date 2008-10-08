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



_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA


; -- Fonctions DPMI --
public Physical_address_mapping
public Free_physical_address_mapping
public Lock_linear_region
public Unlock_linear_region
public Allocate_ldt_descriptor
public Free_ldt_descriptor
public Set_segment_base_address
public Set_segment_limit
public Set_descriptor_access_rights
public Get_segment_base_address





Physical_address_mapping proc near

  push ebp
  mov  ebp,esp

  arg  Physical_address:dword,Physical_size:dword,Linear_address_pointer:dword

  push ebx
  push esi
  push edi

  ; On met dans BX:CX l'adresse physique … mapper
  mov  eax,Physical_address
  mov  cx,ax
  shr  eax,16
  mov  bx,ax

  ; On met dans SI:DI la taille de l'adresse physique … mapper
  mov  eax,Physical_size
  mov  di,ax
  shr  eax,16
  mov  si,ax

  ; On appelle le service DPMI de mappage d'adresse physique
  mov  ax,0800h
  int  31h

  jc   Physical_address_mapping_Erreur

    ; On sauve l'adresse lin‚aire … l'adresse donn‚e
    mov  eax,Linear_address_pointer
    mov  [eax+00h],cx
    mov  [eax+02h],bx
    ; Et on renvoie un code d'erreur nul
    xor  ax,ax

  Physical_address_mapping_Erreur:

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Physical_address_mapping endp





Free_physical_address_mapping proc near

  push ebp
  mov  ebp,esp

  arg  Linear_address:dword

  push ebx

  ; On met dans BX:CX l'adresse lin‚aire … d‚mapper
  mov  eax,Linear_address
  mov  cx,ax
  shr  eax,16
  mov  bx,ax

  ; On appel le service DPMI de lib‚ration d'un mappage d'adresse physique
  mov  ax,0801h
  int  31h

  jc   Free_physical_address_mapping_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Free_physical_address_mapping_Erreur:

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Free_physical_address_mapping endp





Lock_linear_region proc near

  push ebp
  mov  ebp,esp

  arg  Linear_address:dword,Linear_size:dword

  push ebx
  push esi
  push edi

  ; On met dans BX:CX l'adresse lin‚aire … locker
  mov  eax,Linear_address
  mov  cx,ax
  shr  eax,16
  mov  bx,ax

  ; On met dans SI:DI la taille de l'adresse lin‚aire … locker
  mov  eax,Linear_size
  mov  di,ax
  shr  eax,16
  mov  si,ax

  ; On appel le service DPMI de lockage d'adresse lin‚aire
  mov  ax,0600h
  int  31h

  jc   Lock_linear_region_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Lock_linear_region_Erreur:

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Lock_linear_region endp






Unlock_linear_region proc near

  push ebp
  mov  ebp,esp

  arg  Linear_address:dword,Linear_size:dword

  push ebx
  push esi
  push edi

  ; On met dans BX:CX l'adresse lin‚aire … d‚locker
  mov  eax,Linear_address
  mov  cx,ax
  shr  eax,16
  mov  bx,ax

  ; On met dans SI:DI la taille de l'adresse lin‚aire … d‚locker
  mov  eax,Linear_size
  mov  di,ax
  shr  eax,16
  mov  si,ax

  ; On appel le service DPMI de d‚lockage d'adresse lin‚aire
  mov  ax,0601h
  int  31h

  jc   Unlock_linear_region_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Unlock_linear_region_Erreur:

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Unlock_linear_region endp







Allocate_ldt_descriptor proc near

  push ebp
  mov  ebp,esp

  arg  Nombre_de_descripteurs:word,Base_selector_pointer:dword

  push ebx
  push esi
  push edi

  ; On met dans CX le nombre de descripteurs ldt … allouer
  mov  cx,Nombre_de_descripteurs

  ; On appel le service DPMI d'allocation de descripteurs ldt
  mov  ax,0000h
  int  31h

  jc   Allocate_ldt_descriptor_Erreur

    ; On sauve la valeur du s‚lecteur de base
    mov  ebx,Base_selector_pointer
    mov  [ebx],ax
    ; Et on renvoie un code d'erreur nul
    xor  ax,ax

  Allocate_ldt_descriptor_Erreur:

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Allocate_ldt_descriptor endp






Free_ldt_descriptor proc near

  push ebp
  mov  ebp,esp

  arg  Selector:word

  push ebx
  push esi
  push edi

  ; On met dans BX le descripteur ldt … lib‚rer
  mov  bx,Selector

  ; On appel le service DPMI de lib‚ration de descripteur ldt
  mov  ax,0001h
  int  31h

  jc   Free_ldt_descriptor_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Free_ldt_descriptor_Erreur:

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Free_ldt_descriptor endp






Set_segment_base_address proc near

  push ebp
  mov  ebp,esp

  arg  Selector:word,Linear_base_address:dword

  push ebx

  ; On met dans CX:DX l'adresse de base lin‚aire … assigner au segment
  mov  eax,Linear_base_address
  mov  dx,ax
  shr  eax,16
  mov  cx,ax

  ; On met dans BX le s‚lecteur auquel il faut assigner l'adresse de base
  mov  bx,Selector

  ; On appel le service DPMI d'assignation d'adresse de base … un segment
  mov  ax,0007h
  int  31h

  jc   Set_segment_base_address_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Set_segment_base_address_Erreur:

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Set_segment_base_address endp






Set_segment_limit proc near

  push ebp
  mov  ebp,esp

  arg  Selector:word,Segment_limit:dword

  push ebx

  ; On met dans CX:DX la limite (taille) … assigner au segment
  mov  eax,Segment_limit
  mov  dx,ax
  shr  eax,16
  mov  cx,ax

  ; On met dans BX le s‚lecteur auquel il faut assigner une limite
  mov  bx,Selector

  ; On appel le service DPMI d'assignation de limite … un segment
  mov  ax,0008h
  int  31h

  jc   Set_segment_limit_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Set_segment_limit_Erreur:

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Set_segment_limit endp





Set_descriptor_access_rights proc near

  push ebp
  mov  ebp,esp

  arg  Selector:word,Rights:word

  push ebx

  ; On met dans CX les droits … assigner au segment
  mov  cx,Rights

  ; On met dans BX le s‚lecteur auquel il faut assigner des droits
  mov  bx,Selector

  ; On appel le service DPMI d'assignation de droits … un segment
  mov  ax,0009h
  int  31h

  jc   Set_descriptor_access_rights_Erreur

    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Set_descriptor_access_rights_Erreur:

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret



Set_descriptor_access_rights endp






Get_segment_base_address proc near

  push ebp
  mov  ebp,esp

  arg  Selector:word,Linear_base_address_pointer:dword

  push ebx

  ; On met dans BX le s‚lecteur dont il faut lire l'adresse de base
  mov  bx,Selector

  ; On appel le service DPMI de lecture d'adresse de base d'un segment
  mov  ax,0006h
  int  31h

  jc   Get_segment_base_address_Erreur

    ; On sauve l'adresse de base lin‚aire du segment
    mov  eax,Linear_base_address_pointer
    mov  [eax+00h],dx
    mov  [eax+02h],cx
    ; On renvoie un code d'erreur nul
    xor  ax,ax

  Get_segment_base_address_Erreur:

  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Get_segment_base_address endp





_TEXT ENDS
END
