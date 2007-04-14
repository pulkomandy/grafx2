MC_POUR_P2=0

.386P
.MODEL FLAT

_TEXT Segment dword public 'code'
      Assume cs:_TEXT, ds:_DATA

public Effacer_ecran_courant
public Copier_image_dans_brosse
public Permuter_dans_l_image_les_couleurs
public Remap_general_LOWLEVEL
public Scroll_picture
public Clavier_americain
public Clavier_de_depart
public Recuperer_nb_lignes
public Passer_en_mode_texte
public Detection_souris
public Get_input
public Set_mouse_position
public Clip_mouse
public Sensibilite_souris
public Attendre_fin_de_click
public Set_color
public Set_palette
public Palette_64_to_256
public Palette_256_to_64
public Effacer_image_courante
public Effacer_image_courante_Stencil
public Wait_VBL
public Tempo_jauge
public Round_div
public Palette_Compter_nb_couleurs_utilisees
public Get_key
public Pixel_dans_ecran_courant
public Lit_pixel_dans_ecran_courant
public Lit_pixel_dans_ecran_brouillon
public Lit_pixel_dans_ecran_backup
public Lit_pixel_dans_ecran_feedback
public Pixel_dans_brosse
public Lit_pixel_dans_brosse
public Freespace
public Type_de_lecteur_de_disquette
public Disk_map
public Disque_dur_present
public Lecteur_CDROM_present
public Ellipse_Calculer_limites
public Pixel_dans_ellipse
public Pixel_dans_cercle
public Bit
public Couleur_ILBM_line
public Initialiser_chrono
public Tester_chrono
public Remplacer_une_couleur
public Remplacer_toutes_les_couleurs_dans_limites
public Meilleure_couleur
public Meilleure_couleur_sans_exclusion
public Effet_Colorize_interpole
public Effet_Colorize_additif
public Effet_Colorize_soustractif
public Effet_Trame
public Flip_X_LOWLEVEL
public Flip_Y_LOWLEVEL
public Rotate_90_deg_LOWLEVEL
public Rotate_180_deg_LOWLEVEL
public Zoomer_une_ligne
public Copier_une_partie_d_image_dans_une_autre


extrn Ecran_backup                :dword
extrn Largeur_ecran               :word
extrn Hauteur_ecran               :word
extrn Forcer_affichage_curseur    :byte
extrn Afficher_curseur            :near
extrn Effacer_curseur             :near
extrn Calculer_coordonnees_pinceau:near
extrn Principal_Palette           :dataptr
extrn Exclude_color               :dataptr

; Donn‚es pour le colorize

extrn Table_de_multiplication_par_Facteur_A:dataptr
extrn Table_de_multiplication_par_Facteur_B:dataptr
extrn FX_Feedback_Ecran:dword

; Donn‚es pour les trames

extrn Trame:dataptr      ; Sprite de la trame
extrn Trame_Largeur:word ; Largeur de la trame
extrn Trame_Hauteur:word ; Hauteur de la trame

; Donn‚es sur l'image

extrn Principal_Ecran             :dword
extrn Principal_Image_modifiee    :byte
extrn Principal_Largeur_image     :word
extrn Principal_Hauteur_image     :word

extrn Principal_Split             :word
extrn Principal_X_Zoom            :word
extrn Loupe_Mode:byte

extrn Limite_Haut                 :word
extrn Limite_Bas                  :word
extrn Limite_Gauche               :word
extrn Limite_Droite               :word

; Donn‚es sur le brouillon

extrn Brouillon_Ecran             :dword
extrn Brouillon_Largeur_image     :word
extrn Brouillon_Hauteur_image     :word

; Donn‚es sur la brosse

extrn Brosse                      :dword
extrn Brosse_Largeur              :word
extrn Brosse_Hauteur              :word

; Donn‚es sur le menu

extrn Menu_Ordonnee:word

; Donn‚es sur la souris

extrn Mouse_X:word
extrn Mouse_Y:word
extrn Mouse_K:byte
extrn INPUT_Nouveau_Mouse_X:word
extrn INPUT_Nouveau_Mouse_Y:word
extrn INPUT_Nouveau_Mouse_K:byte
extrn Mouse_Facteur_de_correction_X:byte
extrn Mouse_Facteur_de_correction_Y:byte

extrn Operation_Taille_pile:byte
extrn Operation_en_cours:word
extrn Operation_dans_loupe:byte

; Donn‚es sur les touches

extrn Touche         :word
extrn Touche_ASCII   :byte
extrn Config_Touche  :dword
extrn INPUT_Keyb_mode:byte

extrn Autoriser_changement_de_couleur_pendant_operation:byte

; Donn‚es sur les ellipses et les cercles:

extrn Table_des_carres                 :dataptr
extrn Ellipse_Curseur_X                :dword
extrn Ellipse_Curseur_Y                :dword
extrn Ellipse_Rayon_vertical_au_carre  :dword
extrn Ellipse_Rayon_horizontal_au_carre:dword
extrn Ellipse_Limite_High              :dword
extrn Ellipse_Limite_Low               :dword
extrn Cercle_Curseur_X                 :dword
extrn Cercle_Curseur_Y                 :dword
extrn Cercle_Limite                    :dword

extrn LBM_Buffer:dataptr
extrn HBPm1     :byte

extrn MODE_X_Valeur_initiale_de_esi:dword  ; Variables de travail (aucun
extrn MODE_X_Valeur_initiale_de_edi:dword  ; rapport avec leurs noms !!)

extrn MC_Indice:dword             ; pour indexer la table d'exclusion
extrn MC_Table_differences:dword  ; Table pr‚calcul‚e des diff‚rences
extrn MC_DR:dword
extrn MC_DV:dword
extrn MC_DB:dword

extrn Etat_chrono :byte
extrn Chrono_cmp  :dword
extrn Chrono_delay:dword


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; -- N‚ttoyer l'‚cran courant --

Effacer_ecran_courant proc near

  push edi

  mov  edi,Principal_Ecran

  xor  eax,eax
  xor  ecx,ecx
  mov   ax,Principal_Largeur_image
  mov   cx,Principal_Hauteur_image
  mul  ecx
  mov  ecx,eax
  xor  eax,eax

  ; On efface ECX pixels:
  shr  ecx,1
  jnc  Effacer_ecran_courant_Longueur_multiple_de_2
  stosb
  Effacer_ecran_courant_Longueur_multiple_de_2:
  shr  ecx,1
  jnc  Effacer_ecran_courant_Longueur_multiple_de_4
  stosw
  Effacer_ecran_courant_Longueur_multiple_de_4:
  rep  stosd

  pop  edi

  ret

Effacer_ecran_courant endp



; -- Copier une partie de l'image dans la brosse --

Copier_image_dans_brosse proc near

  push ebp
  mov  ebp,esp

  arg  Debut_X:word,Debut_Y:word,Largeur:word,Hauteur:word,Largeur_image:word

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse de d‚part de l'image:
  xor   eax,eax
  xor   ebx,ebx
  xor   ecx,ecx
  mov    ax,Debut_Y
  mov    bx,Largeur_image
  mov    cx,Debut_X
  mul   ebx
  mov   esi,Principal_Ecran
  add   eax,ecx
  add   esi,eax

  ; On place dans EDI l'adresse de d‚part de la brosse:
  mov  edi,Brosse

  ; On place dans EAX la distance entre la fin d'une ligne de l'image et le
  ; d‚but de la suivante:
  xor  eax,eax
  mov   ax,Largeur_image
  sub   ax,Largeur

  ; On place dans DX le nombre de lignes … copier:
  mov  dx,Hauteur

  xor  ecx,ecx
  Copier_image_dans_brosse_Pour_chaque_ligne:

    ; On met dans CX le nombre de pixels … copier sur la ligne:
    mov  cx,Largeur

    ; On recopie CX fois les pixels:
    shr  cx,1
    jnc  Copier_image_dans_brosse_Largeur_multiple_de_2
    movsb
    Copier_image_dans_brosse_Largeur_multiple_de_2:
    shr  cx,1
    jnc  Copier_image_dans_brosse_Largeur_multiple_de_4
    movsw
    Copier_image_dans_brosse_Largeur_multiple_de_4:
    rep  movsd

    ; On rajoute … ESI la distance EAX
    add  esi,eax

    ; On passe … la ligne suivante s'il y en a une:
    dec  dx
    jnz  Copier_image_dans_brosse_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Copier_image_dans_brosse endp




; -- Permuter deux couleurs dans l'image --

Permuter_dans_l_image_les_couleurs proc near

  push ebp
  mov  ebp,esp

  arg  Couleur_1:byte,Couleur_2:byte

  push ebx


  ; On place dans EBX l'adresse du d‚but de l'image
  mov  ebx,Principal_Ecran

  ; On place dans ECX le nb de pixels … traiter
  xor  eax,eax
  xor  ecx,ecx
  mov   ax,Principal_Largeur_image
  mov   cx,Principal_Hauteur_image
  mul  ecx
  mov  ecx,eax

  ; On place dans DL la 1Šre couleur … ‚changer
  mov  dl,Couleur_1

  ; On place dans DH la 2Šme couleur … ‚changer
  mov  dh,Couleur_2

  Permuter_dans_l_image_les_couleurs_Pour_chaque_pixel:

    ; On lit la couleur du pixel en cours:
    mov  al,[ebx]

    ; Si le pixel est de Couleur_1
    cmp  al,dl
    jne  Permuter_dans_l_image_les_couleurs_Test_1
    ; On le met en Couleur_2
    mov  [ebx],dh
    jmp  Permuter_dans_l_image_les_couleurs_Fin_des_tests
    Permuter_dans_l_image_les_couleurs_Test_1:

    ; Si le pixel est de Couleur_2
    cmp  al,dh
    jne  Permuter_dans_l_image_les_couleurs_Fin_des_tests
    ; On le met en Couleur_1
    mov  [ebx],dl
    Permuter_dans_l_image_les_couleurs_Fin_des_tests:


    ; On passe au pixel suivant:
    dec  ecx
    jnz  Permuter_dans_l_image_les_couleurs_Pour_chaque_pixel


  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Permuter_dans_l_image_les_couleurs endp



; -- Remplacer une couleur par une autre dans un buffer --

Remap_general_LOWLEVEL proc near

  push ebp
  mov  ebp,esp

  arg  Tab_conv:dword,Buffer:dword,Largeur:word,Hauteur:word,Largeur_buf:word

  push ebx
  push edi
  push esi


  xor  esi,esi

  ; On place dans EBX l'adresse de la table de conversion
  mov  ebx,Tab_conv

  ; On place dans EDI l'adresse du d‚but de l'image
  mov  edi,Buffer

  ; On place dans ESI la distance qu'il y a entre la fin d'une ligne de
  ; l'image et le d‚but de la suivante:
  mov  si,Largeur_buf
  xor  eax,eax       ; Ces deux initialisations sont faites ici pour ‚viter
  mov  dx,Hauteur    ; une p‚nalit‚ sur la modificatione de SI
  sub  si,Largeur

  Remap_general_Pour_chaque_ligne:

    mov  cx,Largeur
    Remap_general_Pour_chaque_pixel:

      ; On lit la couleur du pixel en cours:
      mov  al,[edi]
      ; On fait la conversion de la couleur … l'aide de la table de conversion
      mov  al,[ebx+eax]
      ; On replace la nouvelle couleur … l'emplacement en cours
      mov  [edi],al

      ; On passe au pixel suivant:
      inc  edi
      dec  cx
    jnz  Remap_general_Pour_chaque_pixel

    ; On passe … la ligne suivante:
    add  edi,esi
    dec  dx
  jnz  Remap_general_Pour_chaque_ligne


  pop  esi
  pop  edi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Remap_general_LOWLEVEL endp



Scroll_picture proc near

  push ebp
  mov  ebp,esp

  arg  Decalage_X:word,Decalage_Y:word

  push ebx
  push esi
  push edi


  ; On place dans ESI l'adresse du backup, qui sera la source de la copie
  mov  esi,Ecran_backup

  ; On place dans EDI l'adresse de Principal_Ecran[Decalage_Y][Decalage_X]
  mov  edi,Principal_Ecran
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,Decalage_Y
  mov   bx,Principal_Largeur_image       ; EBX contient la largeur de l'image
  mov   cx,Decalage_X                    ; ECX contient le Decalage_X
  mul  ebx
  add  edi,eax
  add  edi,ecx

  ;   On place dans AX le nb de pixels … copier … la droite de
  ; (Decalage_X,Decalage_Y)
  mov  ax,bx
  sub  ax,cx

  ; On place dans DX le nb de lignes … copier sous (Decalage_X,Decalage_Y)
  mov  dx,Principal_Hauteur_image
  sub  dx,Decalage_Y
  jz   Scroll_picture_Fin_de_traitement_Bas

  Scroll_picture_Pour_chaque_ligne_Bas:

    ; On place dans CX le nb de pixels … copier
    mov  cx,ax

    ; On fait une copier 32 bits de CX pixels de ESI vers EDI
    shr  cx,1
    jnc  Scroll_picture_Bas_Droite_Multiple_de_2
      movsb
    Scroll_picture_Bas_Droite_Multiple_de_2:
    shr  cx,1
    jnc  Scroll_picture_Bas_Droite_Multiple_de_4
      movsw
    Scroll_picture_Bas_Droite_Multiple_de_4:
    rep  movsd

    ; On replace EDI en tout d‚but de ligne
    sub  edi,ebx

    ; On place dans CX le nb de pixels … copier
    mov  cx,Decalage_X

    ; On fait une copier 32 bits de CX pixels de ESI vers EDI
    shr  cx,1
    jnc  Scroll_picture_Bas_Gauche_Multiple_de_2
      movsb
    Scroll_picture_Bas_Gauche_Multiple_de_2:
    shr  cx,1
    jnc  Scroll_picture_Bas_Gauche_Multiple_de_4
      movsw
    Scroll_picture_Bas_Gauche_Multiple_de_4:
    rep  movsd

    ; On passe … la ligne suivante
;;    add  esi,ebx
    add  edi,ebx
    dec  dx
    jnz  Scroll_picture_Pour_chaque_ligne_Bas

  Scroll_picture_Fin_de_traitement_Bas:

  ;   On vient de faire le traitement pour toutes les lignes plac‚es sous
  ; Decalage_Y. On va maintenant s'occuper de la partie au-dessus.

  ; On place dans EDI l'adresse de Principal_Ecran[0][Decalage_X]
  movzx edi,Decalage_X
  add  edi,Principal_Ecran

  ;   On place dans DX le nb de lignes … copier au-dessus de
  ; (Decalage_X,Decalage_Y)
  mov  dx,Decalage_Y
  cmp  dx,0
  je   Scroll_picture_Fin_de_traitement_Haut

  Scroll_picture_Pour_chaque_ligne_Haut:

    ; On place dans CX le nb de pixels … copier
    mov  cx,ax

    ; On fait une copier 32 bits de CX pixels de ESI vers EDI
    shr  cx,1
    jnc  Scroll_picture_Haut_Droite_Multiple_de_2
      movsb
    Scroll_picture_Haut_Droite_Multiple_de_2:
    shr  cx,1
    jnc  Scroll_picture_Haut_Droite_Multiple_de_4
      movsw
    Scroll_picture_Haut_Droite_Multiple_de_4:
    rep  movsd

    ; On replace EDI en tout d‚but de ligne
    sub  edi,ebx

    ; On place dans CX le nb de pixels … copier
    mov  cx,Decalage_X

    ; On fait une copier 32 bits de CX pixels de ESI vers EDI
    shr  cx,1
    jnc  Scroll_picture_Haut_Gauche_Multiple_de_2
      movsb
    Scroll_picture_Haut_Gauche_Multiple_de_2:
    shr  cx,1
    jnc  Scroll_picture_Haut_Gauche_Multiple_de_4
      movsw
    Scroll_picture_Haut_Gauche_Multiple_de_4:
    rep  movsd

    ; On passe … la ligne suivante
;;    add  esi,ebx
    add  edi,ebx
    dec  dx
    jnz  Scroll_picture_Pour_chaque_ligne_Haut

  Scroll_picture_Fin_de_traitement_Haut:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Scroll_picture endp



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ---- Gestion du mode texte pour pouvoir repasser dans le mode d'origine ----

; -- R‚cup‚rer les informations du mode texte initial --
Recuperer_nb_lignes proc near

  pushad

  mov  ax,1130h
  xor  bh,bh
  push es
  int  10h
  pop  es
  inc  dl
  mov  [esp+28],dl

  popad

  ret

Recuperer_nb_lignes endp


; -- Passer dans un mode texte 25 ou 50 lignes --
Passer_en_mode_texte proc near

  push ebp
  mov  ebp,esp

  arg  Nb_lignes:byte

  push ax
  push bx
  push dx

  mov  ax,3
  int  10h
  cmp  Nb_lignes,50
  jne  pas_mode_50
  mov  ax,1112h
  xor  bl,bl
  mov  dl,50
  int  10h
  pas_mode_50:

  pop  dx
  pop  bx
  pop  ax

  mov  esp,ebp
  pop  ebp

  ret

Passer_en_mode_texte endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ----------------------- Passer en clavier am‚ricain ------------------------

Clavier_americain proc near

  push ax
  push bx

  mov  ax,0AD83h
  int  2Fh
  mov  INPUT_Keyb_mode,bl
  mov  ax,0AD82h
  xor  bl,bl
  int  2Fh

  pop  bx
  pop  ax

  ret

Clavier_americain endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -------------------- Repasser dans le clavier de d‚part --------------------

Clavier_de_depart proc near

  push ax
  push bx

  mov  ax,0AD82h
  mov  bl,INPUT_Keyb_mode
  int  2Fh

  pop  bx
  pop  ax

  ret

Clavier_de_depart endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ---------------- D‚tection de la pr‚sence d'un driver souris ---------------

Detection_souris proc near

  push bx

  xor  ax,ax
  int  33h

  pop  bx

  ret

Detection_souris endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ------- R‚cup‚ration des entr‚es de l'utilisateur (souris/clavier) ---------

Get_input proc near

  pushad


  ; Gestion de la souris:

    ; Appel … l'interruption pour connaŒtre les nouvelles coordonn‚es:

    mov  ax,0003h
    int  33h

    ; CX = Position X
    ; DX = Position Y
    ; BL = Etat des boutons

    ; Dans le cas o— les 2 boutons sont enfonc‚s...

    cmp  bl,3
    jb   Get_input_Saut_inhibe_2_boutons

    ; ... on n'en considŠre aucun:

    xor  bl,bl

    Get_input_Saut_inhibe_2_boutons:

    ; On transfŠre dans les variables les nouvelles valeurs:

    mov  ax,cx
    mov  cl,Mouse_Facteur_de_correction_X
    shr  ax,cl
    mov  cl,Mouse_Facteur_de_correction_Y
    shr  dx,cl
    mov  INPUT_Nouveau_Mouse_X,ax
    mov  INPUT_Nouveau_Mouse_Y,dx
    mov  INPUT_Nouveau_Mouse_K,bl


  ; Gestion des touches:

    ; Par d‚faut -> pas de touches enfonc‚es

    mov  Touche,0
    mov  Touche_ASCII,0

    ; Appel … l'interruption pour savoir si une touche est appuy‚e

    mov  ah,11h
    int  16h

      ; ZERO -> Pas de touche

    ; S'il n'y en a pas, on se contente de la valeur par d‚faut et on sort

    jz   Get_input_Pas_de_touche

    ; Sinon, appel … l'interruption pour connaŒtre son scancode

    mov  ah,10h
    int  16h

      ; AH = Scancode
      ; AL = CaractŠre ASCII

    ; On place le scan code dans la partie basse de [Touche]

    mov  byte ptr[Touche],ah
    mov  Touche_ASCII,al      ; ... et le code ASCII dans Touche_ASCII

    ; Appel … l'interruption pour connaitre l'‚tat des touches de contr“le

    mov  ah,02h
    int  16h

      ; AL = Etat des touches de contr“le


    ; On oublie les informations sur Insert, CapsLock, NumLock et ScrollLock

    and  al,00001111b

    ; On associe les deux Shifts (2 bits de poids faible)

    mov  ah,al
    and  ah,00000001b
    shr  al,1
    or   al,ah

    ; On met le resultat dans la partie haute de [Touche]

    mov  byte ptr[Touche+1],al


    ; On gŠre le cas o— [Touche] est un d‚placement du curseur

    mov  ax,Touche

    ; Test [Touche] = Emulation de MOUSE UP

    cmp  ax,word ptr[Config_Touche]
    jne  Get_input_Pas_emulation_Haut

    cmp  INPUT_Nouveau_Mouse_Y,0
    je   Get_input_Pas_de_touche

    dec  INPUT_Nouveau_Mouse_Y

    jmp  Get_input_Fin_emulation

    Get_input_Pas_emulation_Haut:

    ; Test [Touche] = Emulation de MOUSE DOWN

    cmp  ax,word ptr[Config_Touche+2]
    jne  Get_input_Pas_emulation_Bas

    mov  ax,INPUT_Nouveau_Mouse_Y
    mov  bx,Hauteur_ecran
    dec  bx
    cmp  ax,bx
    jae  Get_input_Pas_de_touche

    inc  ax
    mov  INPUT_Nouveau_Mouse_Y,ax

    jmp  Get_input_Fin_emulation

    Get_input_Pas_emulation_Bas:

    ; Test [Touche] = Emulation de MOUSE LEFT

    cmp  ax,word ptr[Config_Touche+4]
    jne  Get_input_Pas_emulation_Gauche

    cmp  INPUT_Nouveau_Mouse_X,0
    je   Get_input_Pas_de_touche

    dec  INPUT_Nouveau_Mouse_X

    jmp  Get_input_Fin_emulation

    Get_input_Pas_emulation_Gauche:

    ; Test [Touche] = Emulation de MOUSE RIGHT

    cmp  ax,word ptr[Config_Touche+6]
    jne  Get_input_Pas_emulation_Droite

    mov  ax,INPUT_Nouveau_Mouse_X
    mov  bx,Largeur_ecran
    dec  bx
    cmp  ax,bx
    jae  Get_input_Pas_de_touche

    inc  ax
    mov  INPUT_Nouveau_Mouse_X,ax
    jmp  Get_input_Fin_emulation

    Get_input_Pas_emulation_Droite:

    ; Test [Touche] = Emulation de MOUSE CLICK LEFT

    cmp  ax,word ptr[Config_Touche+8]
    jne  Get_input_Pas_emulation_Click_gauche

    mov  INPUT_Nouveau_Mouse_K,1
    jmp  Get_input_Pas_de_touche

    Get_input_Pas_emulation_Click_gauche:

    ; Test [Touche] = Emulation de MOUSE CLICK RIGHT

    cmp  ax,word ptr[Config_Touche+10]
    jne  Get_input_Pas_de_touche

    mov  INPUT_Nouveau_Mouse_K,2
    jmp  Get_input_Pas_de_touche

    Get_input_Fin_emulation:

    mov  cl,Mouse_Facteur_de_correction_X
    mov  ax,INPUT_Nouveau_Mouse_X
    mov  dx,INPUT_Nouveau_Mouse_Y
    shl  ax,cl
    mov  cl,Mouse_Facteur_de_correction_Y
    shl  dx,cl
    mov  cx,ax
    mov  ax,0004h
    int  33h

    Get_input_Pas_de_touche:


  ; Gestion "avanc‚e" du curseur: interdire la descente du curseur dans le
  ; menu lorsqu'on est en train de travailler dans l'image


    cmp  Operation_Taille_pile,0
    je   Get_input_Pas_de_correction

      xor  bl,bl  ; BL va indiquer si on doit corriger la position du curseur

      ; Si le curseur ne se trouve plus dans l'image
      mov  ax,Menu_Ordonnee
      cmp  INPUT_Nouveau_Mouse_Y,ax
      jb   Get_input_Fin_correction_Y

        ; On bloque le curseur en fin d'image
        dec  ax  ; La ligne !!au-dessus!! du menu
        inc  bl
        mov  INPUT_Nouveau_Mouse_Y,ax

      Get_input_Fin_correction_Y:


      cmp  Loupe_Mode,0
      jz   Get_input_Fin_correction_X

      mov  ax,INPUT_Nouveau_Mouse_X
      cmp  Operation_dans_loupe,0
      jnz  Get_input_X_dans_loupe

        mov dx,Principal_Split
        cmp ax,dx
        jb  Get_input_Fin_correction_X

        dec dx
        inc bl
        mov INPUT_Nouveau_Mouse_X,dx

      jmp  Get_input_Fin_correction_X
      Get_input_X_dans_loupe:

        mov dx,Principal_X_Zoom
        cmp ax,dx
        jae Get_input_Fin_correction_X

        inc bl
        mov INPUT_Nouveau_Mouse_X,dx

      Get_input_Fin_correction_X:


      or   bl,bl
      jz   Get_input_Pas_de_correction_du_curseur

        mov  cl,Mouse_Facteur_de_correction_X
        mov  ax,INPUT_Nouveau_Mouse_X
        mov  dx,INPUT_Nouveau_Mouse_Y
        shl  ax,cl
        mov  cl,Mouse_Facteur_de_correction_Y
        shl  dx,cl
        mov  cx,ax
        mov  ax,0004h
        int  33h

      Get_input_Pas_de_correction_du_curseur:


      mov  ax,Touche
      or   ax,ax
      jz   Get_input_Pas_de_correction
      ; Enfin, on inhibe les touches (sauf si c'est un changement de couleur
      ; ou de taille de pinceau lors d'une des operations suivantes:
      ; OPERATION_DESSIN_CONTINU, OPERATION_DESSIN_DISCONTINU, OPERATION_SPRAY
      cmp  Autoriser_changement_de_couleur_pendant_operation,0
      jz   Get_input_Il_faut_inhiber_les_touches

      ; A ce stade l…, on sait qu'on est dans une des 3 op‚rations supportant
      ; le changement de couleur ou de taille de pinceau.
      cmp  ax,word ptr [Config_Touche+12]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+14]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+16]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+18]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+20]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+22]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+24]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+26]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+28]
      je   Get_input_Pas_de_correction
      cmp  ax,word ptr [Config_Touche+30]
      je   Get_input_Pas_de_correction

      Get_input_Il_faut_inhiber_les_touches:
      mov  word ptr Touche,0

    Get_input_Pas_de_correction:


  ; On commence … mettre … jour les variables globales

  mov  ax,INPUT_Nouveau_Mouse_X
  mov  bx,INPUT_Nouveau_Mouse_Y
  mov  cl,INPUT_Nouveau_Mouse_K

  cmp  ax,Mouse_X
  jne  Get_input_Affichage_curseur

  cmp  bx,Mouse_Y
  jne  Get_input_Affichage_curseur

  cmp  cl,Mouse_K
  jne  Get_input_Affichage_curseur

  cmp  Forcer_affichage_curseur,0
  je   Get_input_Fin_Get_input

  Get_input_Affichage_curseur:

    mov  Forcer_affichage_curseur,0

    pushad
    call Effacer_curseur
    popad

    mov  Mouse_X,ax
    mov  Mouse_Y,bx
    mov  Mouse_K,cl

    ; Calcul des coordonn‚es du pinceau:
    call Calculer_coordonnees_pinceau

    pushad
    call Afficher_curseur
    popad

  Get_input_Fin_Get_input:

  popad

  ret

Get_input endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -------- Repositionner la souris en fonction de Mouse_X et Mouse_Y ---------

Set_mouse_position proc near

  push ax
  push cx
  push dx


  mov  cl,Mouse_Facteur_de_correction_X
  mov  ax,Mouse_X
  mov  dx,Mouse_Y
  shl  ax,cl
  mov  cl,Mouse_Facteur_de_correction_Y
  shl  dx,cl
  mov  cx,ax
  mov  ax,0004h
  int  33h

  pop  dx
  pop  cx
  pop  ax

  ret

Set_mouse_position endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ---- D‚finir le champ de d‚placement de la souris suivant la r‚solution ----

Clip_mouse proc near

  push ax
  push cx
  push dx


  ; RŠgle les dimensions de l'espace virtuel de la souris et corrige le
  ; programme pour une pr‚cision au pixel prŠs.


  ; On rŠgle le clip en hauteur:

  mov  cl,Mouse_Facteur_de_correction_Y
  mov  dx,Hauteur_ecran
  mov  ax,0008h
  dec  dx
  shl  dx,cl
  xor  cx,cx
  int  33h

  ; On rŠgle le clip en largeur:

  mov  cl,Mouse_Facteur_de_correction_X
  mov  dx,Largeur_ecran
  mov  ax,0007h
  dec  dx
  shl  dx,cl
  xor  cx,cx
  int  33h

  pop  dx
  pop  cx
  pop  ax

  ret

Clip_mouse endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


Sensibilite_souris proc near

  push ebp
  mov  ebp,esp

  arg Sensibilite_X:word,Sensibilite_Y:word

  push ax
  push cx
  push dx


  ; RŠglage de la sensibilit‚

  mov ax,0Fh
  mov cx,Sensibilite_X
  mov dx,Sensibilite_Y
  int 33h


  pop  dx
  pop  cx
  pop  ax

  mov  esp,ebp
  pop  ebp

  ret

Sensibilite_souris endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


Attendre_fin_de_click proc near

  pushad


  Attendre_fin_de_click_Boucle:

  mov  ax,0003h
  int  33h                      ; Fout en l'air AX,BC,CX et DX
  or   bl,bl
  jnz  Attendre_fin_de_click_Boucle

  mov  Mouse_K,0


  popad

  ret

Attendre_fin_de_click endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; --------------------------- D‚finir une couleur ----------------------------

Set_color proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte, Rouge:byte , Vert:byte, Bleu:byte


  mov  dx,3C8h
  mov  al,Couleur
  out  dx,al
  inc  dx
  mov  al,Rouge
  out  dx,al
  mov  al,Vert
  out  dx,al
  mov  al,Bleu
  out  dx,al


  mov  esp,ebp
  pop  ebp

  ret

Set_color endp

; --------------------------- D‚finir une palette ----------------------------

Set_palette proc near

  push ebp
  mov  ebp,esp

  arg  Palette:dataptr

  push esi


;  xor  al,al            ; M‚thode rapide mais qui ne fonctionne pas avec
;  mov  dx,3C8h          ; toutes les cartes vid‚o.
;  out  dx,al
;
;  inc  dx
;  mov  esi,Palette
;  mov  ecx,768
;  rep  outsb

  mov  esi,Palette       ; M‚thode un peu plus lente mais qui fonctionne avec
  mov  dx,3C9h           ; toutes les cartes vid‚o compatibles VGA.
  xor  cl,cl
  Set_palette_Loop:
    dec  dx
    mov  al,cl
    out  dx,al
    inc  dx
    outsb
    outsb
    outsb
    inc  cl              ; On incr‚mente le num‚ro de couleur et
  jnz  Set_palette_Loop  ; s'il repasse … 0, c'est qu'on a termin‚.


  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Set_palette endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ----------- Conversion d'une palette 6 bits en une palette 8 bits ----------

Palette_64_to_256 proc near

  push ebp
  mov  ebp,esp

  arg  Palette:dataptr

  pushad


  mov  esi,Palette
  mov  cx,768

  Boucle_64_to_256:

    mov  al,[esi]
    shl  al,2
    mov  [esi],al
    inc  esi
    dec  cx
    jnz  Boucle_64_to_256


  popad

  mov  esp,ebp
  pop  ebp

  ret

Palette_64_to_256 endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ----------- Conversion d'une palette 8 bits en une palette 6 bits ----------

Palette_256_to_64 proc near

  push ebp
  mov  ebp,esp

  arg  Palette:dataptr

  pushad


  mov  esi,Palette
  mov  cx,768

  Boucle_256_to_64:

    mov  al,[esi]
    shr  al,2
    mov  [esi],al
    inc  esi
    dec  cx
    jnz  Boucle_256_to_64


  popad

  mov  esp,ebp
  pop  ebp

  ret

Palette_256_to_64 endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ------------ Effacer l'image courante avec une certaine couleur ------------

Effacer_image_courante proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte

  push edi


  xor  edx,edx
  xor  eax,eax
  mov   dx,Principal_Hauteur_image
  mov   ax,Principal_Largeur_image
  mul  edx
  mov  ecx,eax

  mov  dl,Couleur
  mov  dh,dl
  mov  ax,dx
  shl  eax,16
  mov  ax,dx

  mov  edi,Principal_Ecran

  shr  ecx,1
  jnc  Effacer_image_Pair
  stosb                         ; On commence par se d‚barrasser du 1er byte
				; en cas d'imparit‚
  Effacer_image_Pair:

  shr  ecx,1
  jnc  Effacer_image_test_de_superiorite_a_3
  stosw                         ; On se d‚barrasse des 2Šme et 3Šme bytes en
				; cas de "non-multiplicit‚-de-4"
  Effacer_image_test_de_superiorite_a_3:

  ; copie 32 bits
  rep  stosd


  pop  edi

  mov  esp,ebp
  pop  ebp

  ret

Effacer_image_courante endp



; ---- Effacer l'image courante avec une certaine couleur en mode Stencil ----

Effacer_image_courante_Stencil proc near

  push ebp
  mov  ebp,esp

  arg  Couleur:byte, Pochoir:dataptr

  push ebx
  push edi


  ; Dans ECX on met le nombre de pixels dans l'image
  xor  edx,edx
  xor  eax,eax
  mov   dx,Principal_Hauteur_image
  mov   ax,Principal_Largeur_image
  mul  edx
  mov  ecx,eax

  ; On met dans EBX l'adresse de la table de pochoir
  mov  ebx,Pochoir

  ; On nettoie la partie haute de EDX
  xor  edx,edx

  ; On met dans AL la couleur d'effacement
  mov  al,Couleur

  ; On met dans EDI l'adresse de l'image … effacer
  mov  edi,Principal_Ecran

  Effacer_image_Stencil_Pour_chaque_pixel:

    mov  dl,[edi]
    cmp  byte ptr [ebx+edx],0
    jne  Effacer_image_Stencil_Pas_de_modif
      mov  [edi],al
    Effacer_image_Stencil_Pas_de_modif:

    inc  edi
    dec  ecx
    jnz  Effacer_image_Stencil_Pour_chaque_pixel



  pop  edi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Effacer_image_courante_Stencil endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ----------------- Attendre un balayage vertical de l'‚cran -----------------

Wait_VBL proc near

  mov  dx,3DAh

  VBL_Boucle:
  in   al,dx
  test al,08h
  jnz  VBL_Boucle

  VBL_Attente:
  in   al,dx
  test al,08h
  jz   VBL_Attente

  ret

Wait_VBL endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; ------ Attendre une certain laps de temps pour temporiser les jauges -------

Tempo_jauge proc near

  push ebp
  mov  ebp,esp

  arg  Vitesse:byte;


  mov  cl,Vitesse
  Tempo_Boucle:
    call Wait_VBL
    dec  cl
  jnz Tempo_Boucle


  mov  esp,ebp
  pop  ebp

  ret

Tempo_jauge endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


Round_div proc near

  push ebp
  mov  ebp,esp

  arg  Numerateur:dword,Diviseur:dword


  mov  eax,Numerateur
  mov  ecx,Diviseur
  xor  edx,edx
  div  ecx
  shr  ecx,1
  cmp  edx,ecx
  jbe   Round_div_Pas_arrondi_a_la_valeur_superieure

  inc  ax

  Round_div_Pas_arrondi_a_la_valeur_superieure:


  mov  esp,ebp
  pop  ebp

  ret

Round_div endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


Palette_Compter_nb_couleurs_utilisees proc near

  push ebp
  mov  ebp,esp

  arg  Tableau:dataptr

  pushad


  ; On commence par mettre toutes les valeurs … 0
  mov  ecx,256
  mov  edi,Tableau
  xor  eax,eax
  rep  stosd

  ; On parcourt ensuite l'‚cran courant pour compter les utilisations des
  ; couleurs:

  ; Dans ECX on met le nombre de pixels dans l'image
  xor  edx,edx
  xor  eax,eax
  mov   dx,Principal_Hauteur_image
  mov   ax,Principal_Largeur_image
  mul  edx
  mov  ecx,eax

  ; On place dans ESI le d‚but de l'image
  mov  esi,Principal_Ecran

  ; On place dans EBX le d‚but de la table d'utilisation des couleurs:
  mov  ebx,Tableau

  ; On nettoie la partie haute de EAX
  xor  eax,eax

  Compter_Pour_chaque_pixel:

    ; On lit la couleur dans AL (donc aussi dans EAX)
    mov  al,[esi]

    ; On incr‚mente l'utilisation de la couleur AL:
    inc  dword ptr[ebx+4*eax]

    ; On passe au pixel suivant
    inc  esi
    dec  ecx
    jnz  Compter_Pour_chaque_pixel

  ; On va maintenant compter dans la table les couleurs utilis‚es:

  xor  ax,ax   ; On initialise le compteur … 0
  mov  cx,256  ; On initialise le compte … rebours … 256
  xor  edx,edx ; On initialise l'indice … 0

  Compter_Pour_chaque_couleur:

    cmp  dword ptr[ebx+4*edx],0
    je   Compter_Couleur_pas_utilisee
      inc  ax
    Compter_Couleur_pas_utilisee:

    inc  dx
    dec  cx
    jnz  Compter_Pour_chaque_couleur

  ; On stock le r‚sultat (AX) dans la pile pour que le POPAD ne le d‚truise
  ; pas:


  mov  [esp+28],ax
  popad

  mov  esp,ebp
  pop  ebp

  ret

Palette_Compter_nb_couleurs_utilisees endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


Get_key proc near

  mov  ah,8
  int  21h

  ret

Get_key endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -- Afficher un pixel dans l'‚cran virtuel courant --

Pixel_dans_ecran_courant proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   cx,X
  mul  edx
  add  eax,ecx
  mov  dl,Couleur
  add  eax,Principal_Ecran

  mov  [eax],dl


  mov  esp,ebp
  pop  ebp

  ret

Pixel_dans_ecran_courant endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -- Lire la couleur d'un pixel dans l'‚cran virtuel courant --

Lit_pixel_dans_ecran_courant proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   cx,X
  mul  edx
  add  ecx,eax
  add  ecx,Principal_Ecran

  mov  al,[ecx]


  mov  esp,ebp
  pop  ebp

  ret

Lit_pixel_dans_ecran_courant endp



; -- Lire la couleur d'un pixel dans l'‚cran virtuel brouillon --

Lit_pixel_dans_ecran_brouillon proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Y
  mov   dx,Brouillon_Largeur_image
  mov   cx,X
  mul  edx
  add  ecx,eax
  add  ecx,Brouillon_Ecran

  mov  al,[ecx]


  mov  esp,ebp
  pop  ebp

  ret

Lit_pixel_dans_ecran_brouillon endp



; -- Lire la couleur d'un pixel dans l'‚cran virtuel de sauvegarde --

Lit_pixel_dans_ecran_backup proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  ecx,ecx
  xor  edx,edx
  xor  eax,eax
  mov   cx,X
  mov   dx,Y
  mov   ax,Principal_Largeur_image
  add  ecx,Ecran_backup
  mul  edx
  add  ecx,eax

  mov  al,[ecx]


  mov  esp,ebp
  pop  ebp

  ret

Lit_pixel_dans_ecran_backup endp



; -- Lire la couleur d'un pixel dans l'‚cran de feedback --

Lit_pixel_dans_ecran_feedback proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   cx,X
  mul  edx
  add  ecx,eax
  add  ecx,FX_Feedback_Ecran

  mov  al,[ecx]


  mov  esp,ebp
  pop  ebp

  ret

Lit_pixel_dans_ecran_feedback endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -- D‚finir la couleur d'un pixel dans la brosse --

Pixel_dans_brosse proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov  ax,Y
  mov  dx,Brosse_Largeur
  mov  cx,X
  mul  edx
  add  eax,ecx
  mov  dl,Couleur
  add  eax,Brosse

  mov  [eax],dl


  mov  esp,ebp
  pop  ebp

  ret

Pixel_dans_brosse endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; -- Lire la couleur d'un pixel dans la brosse --

Lit_pixel_dans_brosse proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  eax,eax
  xor  edx,edx
  xor  ecx,ecx
  mov  ax,Y
  mov  dx,Brosse_Largeur
  mov  cx,X
  mul  edx
  add  eax,ecx
  add  eax,Brosse

  mov  al,[eax]


  mov  esp,ebp
  pop  ebp

  ret

Lit_pixel_dans_brosse endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Renseigne sur la quantit‚ de place libre sur un lecteur quelconque
;
; Entr‚e: Octet = nø de lecteur (0:courant, 1:A, 2:B, 3:C, 4:D, ...)
;
; Sortie: Long  =   -1  : Lecteur invalide
;                 sinon : Place libre sur le lecteur

Freespace proc near

  push ebp
  mov  ebp,esp

  arg  Numero_de_lecteur:byte

  push ebx
  push cx
  push edx


  ; On appelle l'interruption du DOS

  mov  ah,36h
  mov  dl,Numero_de_lecteur
  int  21h

  ; On regarde si AX = 0FFFFh (dans ce cas => Erreur)

  cmp  ax,0FFFFh
  je   Freespace_Erreur

  ; Sinon:

  mul  bx      ; DX:AX = AX*BX

  push ax      ; EAX = DX:AX = AX*BX
  mov  ax,dx
  shl  eax,16
  pop  ax

  xor  ebx,ebx ; EDX:EAX = AX*BX*CX
  mov  bx,cx
  mul  ebx

  jmp  Freespace_Fin_de_traitement

  Freespace_Erreur:

  mov  eax,0FFFFFFFFh

  Freespace_Fin_de_traitement:


  pop  edx
  pop  cx
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Freespace endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Renseigne sur un lecteur de disquette:
;
; Entr‚e: Octet = nø du lecteur de disquette (commence … 0)
;
; Sortie: Octet = 0 : Pas de lecteur
;                 1 : Lecteur 360 Ko
;                 2 : Lecteur 1.2 Mo
;                 3 : Lecteur 720 Ko
;                 4 : Lecteur 1.4 Mo
;                 5 : Lecteur 2.8 Mo (???)
;                 6 : Lecteur 2.8 Mo

Type_de_lecteur_de_disquette proc near

  push ebp
  mov  ebp,esp

  arg  Numero_de_lecteur:byte

  xor  eax,eax
  mov  al,10h
  out  70h,al
  in   al,71h

  cmp  Numero_de_lecteur,0
  jne  Type_de_lecteur_de_disquette_B
    shr al,4
  jmp  Type_de_lecteur_de_disquette_Fin_de_traitement
  Type_de_lecteur_de_disquette_B:
    and al,0Fh

  Type_de_lecteur_de_disquette_Fin_de_traitement:

  mov  esp,ebp
  pop  ebp

  ret

Type_de_lecteur_de_disquette endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Renseigne sur la lettre logique d'un lecteur
; (utile pour tester si un lecteur de disquette est A: ou B: aux yeux du DOS)
;
; Entr‚e: Octet = nø du lecteur (1=A, 2=B ...)
;
; Sortie: Octet = 0FFh : Pas de lecteur (???)
;                 sinon: num‚ro repr‚sentant la lettre logique du lecteur
;                        (commence … 1)

Disk_map proc near

  push ebp
  mov  ebp,esp

  arg  Numero_de_lecteur:byte

  push bx


  xor  eax,eax
  mov  ax,440Eh
  mov  bl,Numero_de_lecteur
  int  21h

  jnc  Disk_map_OK

    mov  al,0FFh
    jmp  Disk_map_Fin

  Disk_map_OK:
  or   al,al
  jnz  Disk_map_Fin

    mov  al,Numero_de_lecteur

  Disk_map_Fin:
  xor  ah,ah


  pop  bx

  mov  esp,ebp
  pop  ebp

  ret

Disk_map endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Renseigne sur un disque dur:
;
; Entr‚e: Octet = nø du disque dur (0:C, 1:D, 2:E, ...)
;
; Sortie: Octet = 0 : Pas de lecteur
;                 1 : Disque dur pr‚sent

Disque_dur_present proc near

  push ebp
  mov  ebp,esp

  arg  Numero_de_disque:byte

  pushad


  ; On se renseigne sur le disque … l'aide de la fonction 08h de
  ; l'interruption 13h

  mov  ah,08h
  mov  dl,Numero_de_disque
  push es
  or   dl,80h
  int  13h     ; modifie ES!

  pop es   ; POP du ES memoris‚ juste avant l'INT 13h

  ; Le disque est absent si CF ou si AH<>0

  jc   Disque_dur_present_Pas_de_disque
  or   ah,ah
  jnz  Disque_dur_present_Pas_de_disque

  ; Il peut arriver que ces tests soient insuffisants, il faut alors comparer
  ; le nø du drive avec le nb de disques durs install‚s (nb dans DL)

  mov  ah,Numero_de_disque
  inc  ah
  cmp  ah,dl
  ja   Disque_dur_present_Pas_de_disque

  ; On doit maintenant pouvoir supposer que le disque en question est bien
  ; install‚

  mov  byte ptr[esp+28],1
  jmp  Disque_dur_present_Fin_de_traitement

  Disque_dur_present_Pas_de_disque:

  mov  byte ptr[esp+28],0

  Disque_dur_present_Fin_de_traitement:


  popad

  mov  esp,ebp
  pop  ebp

  ret

Disque_dur_present endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Renseigne sur un lecteur CD-ROM:
;
; Entr‚e : Octet = nø du lecteur (0:A, 1:B, 2:C, ...)
;
; Sortie : Octet = 0 : Le lecteur n'est pas un CD-ROM
;                  1 : Le lecteur est un CD-ROM

Lecteur_CDROM_present proc near

  push ebp
  mov  ebp,esp

  arg  Numero_de_lecteur:byte

  push bx
  push cx
  push dx


  ; D‚tection du nø de version de MSCDEX

  mov  ax,150Ch
  int  2Fh
  cmp  bh,2
  jb   Lecteur_CDROM_present_Mauvaise_version

  ; On est donc avec une version de MSCDEX assez r‚cente (enfin, disons...
  ; ... pas trop obsolŠte).

  ; On demande des informations sur le lecteur de la part de MSCDEX v2.00

  mov  ax,150Bh
  xor  ch,ch
  mov  cl,Numero_de_lecteur
  int  2Fh

  ; On d‚termine maintenant si le lecteur est de type CD-ROM grƒce aux
  ; r‚sultats

  cmp  bx,0ADADh
  jne  Lecteur_CDROM_present_Pas_CDROM
  or   ax,ax
  jz   Lecteur_CDROM_present_Pas_CDROM

  jmp  Lecteur_CDROM_present_CDROM_OK

  Lecteur_CDROM_present_Mauvaise_version:

  ; On va ˆtre oblig‚ de traiter le problŠme avec une vieille version
  ; merdique de MSCDEX: ‡a me fait chier, plus personne ne devrait poss‚der
  ; des antiquit‚s pareilles. Moi je dit que ce genre de personne ne m‚rite
  ; pas de pouvoir utiliser GRAFX 2 !

  ; mais bon... comme je me trouve dans un bon jour, je vais faire un effort
  ; et je vais demander quelques informations … p‚p‚ MSCDEX. Mais faut pas
  ; abuser: le gars il a int‚rˆt … avoir ses lecteurs CD-ROM les uns … la
  ; suite des autres... je ne m'amuserai pas … prendre en compte un cas
  ; aussi chiant... enfin tu me diras, le type est suffisamment emmerdant pour
  ; avoir une vieille version de MSCDEX, il serait bien du genre … mettre ses
  ; lecteurs CD-ROM dans n'importe quel ordre, le gars.

  mov  ax,1500h
  xor  bx,bx
  int  2Fh

  cmp  Numero_de_lecteur,cl              ; Si le lecteur demand‚ se trouve avant le premier lecteur CD,
  jb   Lecteur_CDROM_present_Pas_CDROM   ; c'est que c'est pas un lecteur CD-ROM

  add  cx,bx
  cmp  Numero_de_lecteur,cl              ; Pareille, si le lecteur est aprŠs le dernier,
  jge  Lecteur_CDROM_present_Pas_CDROM   ; c'est que c'est pas un lecteur CD-ROM

  Lecteur_CDROM_present_CDROM_OK:

  mov  ax,1
  jmp  Lecteur_CDROM_present_Fin_de_traitement

  Lecteur_CDROM_present_Pas_CDROM:

  xor  ax,ax

  Lecteur_CDROM_present_Fin_de_traitement:


  pop  dx
  pop  cx
  pop  bx

  mov  esp,ebp
  pop  ebp

  ret

Lecteur_CDROM_present endp






; -- Calculer les variables globales n‚cessaires au trac‚ d'une ellipse --

Ellipse_Calculer_limites proc near

  push ebp
  mov  ebp,esp

  arg  Rayon_horizontal:word,Rayon_vertical:word

  push ebx


  mov  ebx,offset Table_des_carres
  xor  eax,eax
  xor  ecx,ecx
  mov  ax,Rayon_horizontal
  mov  cx,Rayon_vertical
  mov  eax,[ebx+4*eax]
  mov  ecx,[ebx+4*ecx]
  mov  Ellipse_Rayon_horizontal_au_carre,eax
  mov  Ellipse_Rayon_vertical_au_carre,ecx
  mul  ecx
  mov  Ellipse_Limite_High,edx
  mov  Ellipse_Limite_Low,eax


  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Ellipse_Calculer_limites endp






; -- Teste si un point se trouve ou non dans l'ellipse en cours de trac‚ --

Pixel_dans_ellipse proc near

  push  ebx
  push  esi
  push  edi


  ; On fait pointer EBX sur la table des puissances de 2:
  mov   ebx,offset Table_des_carres

  ; On place dans ECX la valeur absolue de X
  mov   ecx,Ellipse_Curseur_X
  or    ecx,ecx
  jnl   Pixel_dans_ellipse_X_positif
  neg   ecx
  Pixel_dans_ellipse_X_positif:

  ; On place dans EAX la valeur X^2
  mov   eax,[ebx+4*ecx]

  ; On met dans EDX:EAX la valeur ( B^2 x X^2 ):
  mul   dword ptr[Ellipse_Rayon_vertical_au_carre]

  ; On stock le r‚sultat ( B^2 x X^2 ) dans EDI:ESI :
  mov   edi,edx
  mov   esi,eax

  ; On place dans ECX la valeur absolue de Y
  mov   ecx,Ellipse_Curseur_Y
  or    ecx,ecx
  jnl   Pixel_dans_ellipse_Y_positif
  neg   ecx
  Pixel_dans_ellipse_Y_positif:

  ; On place dans EAX la valeur Y^2
  mov   eax,[ebx+4*ecx]

  ; On met dans EDX:EAX la valeur ( A^2 x Y^2 ):
  mul   dword ptr[Ellipse_Rayon_horizontal_au_carre]

  ; On fait la somme de ( B^2 x X^2 ) avec ( A^2 x Y^2 ) dans EDX:EAX :
  add   eax,esi
  adc   edx,edi

  ; On regarde si cette valeur est inferieure ou ‚gale … la limite de
  ; l'ellipse:
  cmp   edx,Ellipse_Limite_High
  je    Pixel_dans_ellipse_Regarder_la_partie_basse
  setb  al
  jmp   Pixel_dans_ellipse_Fin_de_traitement
  Pixel_dans_ellipse_Regarder_la_partie_basse:
  cmp   eax,Ellipse_Limite_Low
  setbe al
  Pixel_dans_ellipse_Fin_de_traitement:

  movzx eax,al

  pop   edi
  pop   esi
  pop   ebx

  ret

Pixel_dans_ellipse endp






; -- Teste si un point se trouve ou non dans le cercle en cours de trac‚ --

Pixel_dans_cercle proc near

  push  ebx


  ; On fait pointer EBX sur la table des puissances de 2:
  mov   ebx,offset Table_des_carres

  ; On place dans EAX et ECX les valeurs absolues de X et Y:
  mov   eax,Cercle_Curseur_X
  mov   ecx,Cercle_Curseur_Y
  or    eax,eax
  jnl   Pixel_dans_cercle_X_positif
  neg   eax
  Pixel_dans_cercle_X_positif:
  or    ecx,ecx
  jnl   Pixel_dans_cercle_Y_positif
  neg   ecx
  Pixel_dans_cercle_Y_positif:

  ; On place dans EAX et ECX les valeurs X^2 et Y^2 :
  mov   eax,[ebx+4*eax]
  mov   ecx,[ebx+4*ecx]

  ; On place dans EAX la somme ( X^2 + Y^2 ) :
  add   eax,ecx

  ; On regarde si la distance est dans la limite du cercle:
  cmp   eax,Cercle_Limite
  setbe al

  and   eax,0FFh


  pop   ebx

  ret

Pixel_dans_cercle endp






; -- Extraire 1 bit d'un octet --

Bit proc near

  push ebp
  mov  ebp,esp

  arg  Octet:byte,Rang:byte

  mov  al,Octet
  mov  cl,Rang
  shr  al,cl
  and  al,1

  mov  esp,ebp
  pop  ebp

  ret

Bit endp






; -- Trouver la couleur du pixel (ILBM) … afficher en Pos_X --

Couleur_ILBM_line proc near

  push ebp
  mov  ebp,esp

  arg  Pos_X:word, Vraie_taille_ligne:word

  push bx
  push esi


  xor  eax,eax
  mov  ax,Pos_X
  ; CL sera le rang auquel on extrait les bits de la couleur
  mov  cl,7
  and  al,7
  sub  cl,al

  ; ESI pointe sur le Buffer de ligne ILBM
  mov  esi,LBM_Buffer

  xor  bl,bl
  xor  dh,dh
  mov  dl,HBPm1
  CIL_Loop:
    push dx
    mov  ax,Vraie_taille_ligne
    mul  dx
    add  ax,Pos_X
    shr  ax,3

    mov  bh,[esi+eax]

    shr  bh,cl
    and  bh,1

    shl  bl,1
    add  bl,bh

    pop  dx
    dec  dl
  jns CIL_Loop

  mov  al,bl


  pop  esi
  pop  bx

  mov  esp,ebp
  pop  ebp

  ret

Couleur_ILBM_line endp






; -- D‚marrer le chrono --

Initialiser_chrono proc near

  push ebp
  mov  ebp,esp

  arg  Delai:dword

  mov  eax,Delai
  mov  Chrono_delay,eax

  xor  ah,ah
  int  1Ah
  mov  word ptr[Chrono_cmp+0],dx
  mov  word ptr[Chrono_cmp+2],cx

  mov  esp,ebp
  pop  ebp

  ret

Initialiser_chrono endp






; -- Met Etat_chrono … 1 si le compte … rebours est atteint --

Tester_chrono proc near

  xor  ah,ah
  int  1Ah

  ; On transfŠre CX:DX dans EAX
  mov  ax,cx
  shl  eax,16
  mov  ax,dx

  sub  eax,Chrono_delay
  cmp  eax,Chrono_cmp
  jna  Tester_Chrono_delay_pas_encore_atteint

    mov  Etat_chrono,1

  Tester_Chrono_delay_pas_encore_atteint:

  ret

Tester_chrono endp





; -- Remplacement d'une couleur dans l'image par une autre --

Remplacer_une_couleur proc near

  push ebp
  mov  ebp,esp

  arg  Ancienne_couleur:byte, Nouvelle_couleur:byte

  push edi


  ; Dans ECX on met le nombre de pixels dans l'image
  xor  edx,edx
  xor  eax,eax
  mov   dx,Principal_Hauteur_image
  mov   ax,Principal_Largeur_image
  mul  edx
  mov  ecx,eax

  ; Dans AL on place la couleur que l'on veut remplacer et dans AH la couleur
  ; par laquelle on veut la remplacer
  mov  al,Ancienne_couleur
  mov  ah,Nouvelle_couleur

  ; EDI pointe sur le d‚but de l'image
  mov  edi,Principal_Ecran

  RUC_Pour_chaque_pixel:

    cmp  al,[edi]
    jne  RUC_Pas_de_changement
      mov  [edi],ah
    RUC_Pas_de_changement:

    ; On change de pixel
    inc  edi
    dec  ecx
    jnz  RUC_Pour_chaque_pixel


  pop  edi

  mov  esp,ebp
  pop  ebp

  ret

Remplacer_une_couleur endp





; -- Remplacement de toutes les couleurs dans l'image … l'aide d'une table --
; -- et ceci en respectant les limites --

Remplacer_toutes_les_couleurs_dans_limites proc near

  push ebp
  mov  ebp,esp

  arg  Table_de_remplacement:dword


  push esi
  push edi
  push ebx

  xor  ebx,ebx

  ; ESI pointe sur le d‚but de la ligne d'‚cran actuelle
  mov  esi,Principal_Ecran
  mov  bx,Limite_Gauche
  add  esi,ebx
  xor  eax,eax
  mov  bx,Limite_Haut
  mov   ax,Principal_Largeur_image
  mul  ebx
  add  esi,eax
  ; EDI pointe sur la position actuelle dans l'‚cran
  mov  edi,esi
  ; EBX pointe sur la table de remplacement
  mov  ebx,Table_de_remplacement

  ; On place dans CX le nombre de lignes … traiter
  mov  cx,Limite_Bas
  sub  cx,Limite_Haut
  RTLCDL_Pour_chaque_ligne:

    ; On met dans DX le nombre de pixels … traiter et on nettoye la partie
    ; haute de EAX
    mov  dx,Limite_Droite
    xor  eax,eax
    sub  dx,Limite_Gauche
    RTLCDL_Pour_chaque_pixel_de_ligne:

      mov  al,[edi]
      mov  al,[eax+ebx]
      mov  [edi],al
      inc  edi

      dec  dx
      jns  RTLCDL_Pour_chaque_pixel_de_ligne

    ; On change de ligne
    mov  ax,Principal_Largeur_image
    add  esi,eax
    mov  edi,esi
    dec  cx
    jns  RTLCDL_Pour_chaque_ligne

  pop  ebx
  pop  edi
  pop  esi


  mov  esp,ebp
  pop  ebp

  ret

Remplacer_toutes_les_couleurs_dans_limites endp





;/////////////////////////////////////////////////////////////////////////////
;//     Inclusion du fichier contenant les fonctions Meilleure_couleur*     //
;/////////////////////////////////////////////////////////////////////////////

IF MC_POUR_P2
  INCLUDE mcpourp2.asm
ELSE
  INCLUDE mcnormal.asm
ENDIF

;/////////////////////////////////////////////////////////////////////////////





; -- Gestion de l'effet Colorize … l'aide de la m‚thode d'interpolation

Effet_Colorize_interpole proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; Facteur_A=256*(100-Colorize_Opacite)/100
  ; Facteur_B=256*(    Colorize_Opacite)/100
  ;
  ; (Couleur_dessous*Facteur_A+Couleur*Facteur_B)/256

  ; On fait tout de suite pointer ESI sur les coordonn‚es (X,Y) dans l'image
  xor  eax,eax
  xor  edx,edx
  xor  esi,esi
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   si,X
  mul  edx
  add  esi,eax
  add  esi,FX_Feedback_Ecran

  xor  edx,edx
  xor  ecx,ecx
  xor  ebx,ebx
  xor  eax,eax

  ;  On place dans ESI 3*Couleur_dessous (ESI est la position dans la palette
  ; des teintes de la Couleur_dessous), et dans EDI 3*Couleur (EDI est la
  ; position dans la palette des teintes de la Couleur)
  mov  al,[esi]
  mov  bl,Couleur
  xor  esi,esi
  xor  edi,edi
  mov  si,ax
  mov  di,bx
  add  si,si
  add  di,di
  add  si,ax
  add  di,bx

  ; On fait pointer EDI et ESI correctement sur la palette
  add  esi,offset Principal_Palette
  add  edi,offset Principal_Palette

;  ; On m‚morise dans BL,CL,DL les 3 teintes de Couleur_dessous
;  mov  bl,[esi+0]
;  mov  cl,[esi+1]
;  mov  dl,[esi+2]
;
;  ; On fait pointer ESI sur la table de multiplication par le Facteur_A
;  mov  esi,offset Table_de_multiplication_par_Facteur_A
;
;  ; On m‚morise dans BH,CH,DH les 3 teintes de Couleur
;  mov  bh,[edi+0]
;  mov  ch,[edi+1]
;  mov  dh,[edi+2]
;
;  ; On fait pointer EDI sur la table de multiplication par le Facteur_B
;  mov  edi,offset Table_de_multiplication_par_Facteur_B

  ; On m‚morise dans BL,CL,DL les 3 teintes de Couleur_dessous
  ;          et dans BH,CH,DH les 3 teintes de Couleur
  mov  bl,[esi]
  mov  bh,[edi]
  inc  esi
  inc  edi
  mov  cl,[esi]
  mov  ch,[edi]
  inc  esi
  inc  edi
  mov  dl,[esi]
  mov  dh,[edi]

  ; On fait pointer ESI sur la table de multiplication par le Facteur_A
  ;              et EDI sur la table de multiplication par le Facteur_B
  mov  esi,offset Table_de_multiplication_par_Facteur_A
  mov  edi,offset Table_de_multiplication_par_Facteur_B

;  ; On va s'occuper de la composante bleue (DX)
;
;  xor  ah,ah
;  mov  al,dh
;  mov  ax,[edi+2*eax] ; AX  = (Bleu        *Facteur_B)
;  xor  dh,dh
;  add  ax,[esi+2*edx] ; AX += (Bleu_dessous*Facteur_A)
;  shr  ax,8
;  push eax
;
;  ; On va s'occuper de la composante verte (CX) et rouge (BX)
;
;  mov  dl,ch
;  mov  al,bh
;  mov  dx,[edi+2*edx] ; DX  = (Vert         *Facteur_B)
;  xor  ch,ch
;  add  dx,[esi+2*ecx] ; DX += (Vert_dessous *Facteur_A)
;  mov  ax,[edi+2*eax] ; AX  = (Rouge        *Facteur_B)
;  xor  bh,bh
;  add  ax,[esi+2*ebx] ; AX += (Rouge_dessous*Facteur_A)
;  shr  dx,8
;  shr  ax,8
;  push edx
;  push eax

  ; On va s'occuper de la composante bleue (DX)

  xor  eax,eax
  mov  al,dh
  xor  dh,dh
  add  eax,eax
  add  edx,edx
  add  eax,offset Table_de_multiplication_par_Facteur_B
  add  edx,offset Table_de_multiplication_par_Facteur_A
  mov  ax,[eax] ; AX  = (Bleu        *Facteur_B)
  add  ax,[edx] ; AX += (Bleu_dessous*Facteur_A)
  shr  ax,8
  push eax

  ; On va s'occuper de la composante verte (CX) et rouge (BX)

  xor  eax,eax
  xor  edx,edx
  mov  al,ch
  mov  dl,cl
  add  eax,eax
  add  edx,edx
  add  eax,offset Table_de_multiplication_par_Facteur_B
  add  edx,offset Table_de_multiplication_par_Facteur_A
  mov  ax,[eax] ; AX  = (Vert        *Facteur_B)
  add  ax,[edx] ; AX += (Vert_dessous*Facteur_A)
  shr  ax,8
  push eax

  xor  eax,eax
  xor  edx,edx
  mov  al,bh
  mov  dl,bl
  add  eax,eax
  add  edx,edx
  add  eax,offset Table_de_multiplication_par_Facteur_B
  add  edx,offset Table_de_multiplication_par_Facteur_A
  mov  ax,[eax] ; AX  = (Vert        *Facteur_B)
  add  ax,[edx] ; AX += (Vert_dessous*Facteur_A)
  shr  ax,8
  push eax

  call Meilleure_couleur
  add  esp,12


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Effet_Colorize_interpole endp







; -- Gestion de l'effet Colorize … l'aide de la m‚thode additive

Effet_Colorize_additif proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; On fait tout de suite pointer ESI sur les coordonn‚es (X,Y) dans l'image
  xor  eax,eax
  xor  edx,edx
  xor  esi,esi
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   si,X
  mul  edx
  add  esi,eax
  add  esi,FX_Feedback_Ecran

  xor  edx,edx
  xor  ecx,ecx
  xor  ebx,ebx
  xor  eax,eax

  ; On lit dans AL (EAX) la couleur en (X,Y)
  mov  al,[esi]

  ; On met dans CL (ECX) la couleur … rajouter
  mov  cl,Couleur

  ; On multiplie AX (EAX) par 3
  mov  dx,ax
  add  ax,ax
  add  ax,dx

  ; On multiplie CX (ECX) par 3
  mov  dx,cx
  add  cx,cx
  add  cx,dx

  ; On fait pointer EBX sur Principal_Palette
  mov  ebx,offset Principal_Palette


  ; On va traiter la composante bleue:


  ; On place dans DL Principal_Palette[AL].B
  mov  dl,[ebx+eax+2]

  ; On place dans DH Principal_Palette[CL].B
  mov  dh,[ebx+ecx+2]

  cmp  dh,dl
  ja   Effet_Colorize_additif_Bleu_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_additif_Bleu_Fin_de_traitement
  Effet_Colorize_additif_Bleu_Superieur:
    shr  dx,8
  Effet_Colorize_additif_Bleu_Fin_de_traitement:

  push edx ; On passe la composante bleue que l'on d‚sire


  ; On va traiter la composante verte:


  ; On place dans DL Principal_Palette[AL].V
  mov  dl,[ebx+eax+1]

  ; On place dans DH Principal_Palette[CL].V
  mov  dh,[ebx+ecx+1]

  cmp  dh,dl
  ja   Effet_Colorize_additif_Vert_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_additif_Vert_Fin_de_traitement
  Effet_Colorize_additif_Vert_Superieur:
    shr  dx,8
  Effet_Colorize_additif_Vert_Fin_de_traitement:

  push edx ; On passe la composante verte que l'on d‚sire


  ; On va traiter la composante rouge:


  ; On place dans DL Principal_Palette[AL].R
  mov  dl,[ebx+eax+0]

  ; On place dans DH Principal_Palette[CL].R
  mov  dh,[ebx+ecx+0]

  cmp  dh,dl
  ja   Effet_Colorize_additif_Rouge_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_additif_Rouge_Fin_de_traitement
  Effet_Colorize_additif_Rouge_Superieur:
    shr  dx,8
  Effet_Colorize_additif_Rouge_Fin_de_traitement:

  push edx ; On passe la composante rouge que l'on d‚sire
  call Meilleure_couleur
  add  esp,12


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Effet_Colorize_additif endp







; -- Gestion de l'effet Colorize … l'aide de la m‚thode soustractive

Effet_Colorize_soustractif proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; On fait tout de suite pointer ESI sur les coordonn‚es (X,Y) dans l'image
  xor  eax,eax
  xor  edx,edx
  xor  esi,esi
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   si,X
  mul  edx
  add  esi,eax
  add  esi,FX_Feedback_Ecran

  xor  edx,edx
  xor  ecx,ecx
  xor  ebx,ebx
  xor  eax,eax

  ; On lit dans AL (EAX) la couleur en (X,Y)
  mov  al,[esi]

  ; On met dans CL (ECX) la couleur … soustraire
  mov  cl,Couleur

  ; On multiplie AX (EAX) par 3
  mov  dx,ax
  add  ax,ax
  add  ax,dx

  ; On multiplie CX (ECX) par 3
  mov  dx,cx
  add  cx,cx
  add  cx,dx

  ; On fait pointer EBX sur Principal_Palette
  mov  ebx,offset Principal_Palette


  ; On va traiter la composante bleue:


  ; On place dans DL Principal_Palette[AL].B
  mov  dl,[ebx+eax+2]

  ; On place dans DH Principal_Palette[CL].B
  mov  dh,[ebx+ecx+2]

  cmp  dh,dl
  jb   Effet_Colorize_soustractif_Bleu_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_soustractif_Bleu_Fin_de_traitement
  Effet_Colorize_soustractif_Bleu_Superieur:
    shr  dx,8
  Effet_Colorize_soustractif_Bleu_Fin_de_traitement:

  push edx ; On passe la composante bleue que l'on d‚sire


  ; On va traiter la composante verte:


  ; On place dans DL Principal_Palette[AL].V
  mov  dl,[ebx+eax+1]

  ; On place dans DH Principal_Palette[CL].V
  mov  dh,[ebx+ecx+1]

  cmp  dh,dl
  jb   Effet_Colorize_soustractif_Vert_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_soustractif_Vert_Fin_de_traitement
  Effet_Colorize_soustractif_Vert_Superieur:
    shr  dx,8
  Effet_Colorize_soustractif_Vert_Fin_de_traitement:

  push edx ; On passe la composante verte que l'on d‚sire


  ; On va traiter la composante rouge:


  ; On place dans DL Principal_Palette[AL].R
  mov  dl,[ebx+eax+0]

  ; On place dans DH Principal_Palette[CL].R
  mov  dh,[ebx+ecx+0]

  cmp  dh,dl
  jb   Effet_Colorize_soustractif_Rouge_Superieur
    xor  dh,dh
    jmp  Effet_Colorize_soustractif_Rouge_Fin_de_traitement
  Effet_Colorize_soustractif_Rouge_Superieur:
    shr  dx,8
  Effet_Colorize_soustractif_Rouge_Fin_de_traitement:

  push edx ; On passe la composante rouge que l'on d‚sire
  call Meilleure_couleur
  add  esp,12


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Effet_Colorize_soustractif endp








; -- Gestion des trames --

Effet_Trame proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word


  xor  edx,edx

  ; renvoie Trame[X%Trame_Largeur][Y%Trame_Hauteur]

  mov  ax,X
  div  Trame_Largeur
  mov  cx,dx         ; CX contient X%Trame_Largeur

  mov  ax,Y
  xor  dx,dx
  div  Trame_Hauteur ; DX contient Y%Trame_Hauteur

  shl  cx,4          ; CX= (X%Trame_Largeur)*16
  add  dx,cx         ; DX=((X%Trame_Largeur)*16)+(Y%Trame_Hauteur)
  add  edx,offset Trame  ; EDX pointe sur Trame[X%Trame_Largeur][Y%Trame_Hauteur]

  mov  al,[edx]


  mov  esp,ebp
  pop  ebp

  ret

Effet_Trame endp







; -- Effectuer une invertion de la brosse / une droite horizontale --

Flip_Y_LOWLEVEL proc near

  push ebx
  push esi
  push edi

  ; On va se servir de ESI pour pointer sur la partie sup‚rieure de la brosse
  ; et de EDI pour pointer sur la partie basse.

  mov  esi,Brosse

  xor  eax,eax
  xor  ebx,ebx
  mov  ax,Brosse_Hauteur
  mov  bx,Brosse_Largeur
  dec  ax
  mov  edi,Brosse
  mul  ebx
  add  edi,eax

  xor  ecx,ecx

  Flip_Y_Tant_que:

    cmp  edi,esi
    jbe  Flip_Y_Fin_tant_que

    ; Il faut inverser les lignes de la brosse point‚es par EDI et ESI
    ; ("Brosse_Largeur" octets en tout)

    mov  cx,Brosse_Largeur

    ; On va se pr‚parer … une inversion 32 bits de lignes. Pour cela, on
    ; traite tout de suite les exc‚dants

    shr  cx,1
    jnc  Flip_Y_Largeur_multiple_de_2

      mov  al,[esi]
      mov  ah,[edi]
      mov  [edi],al
      mov  [esi],ah
      inc  edi
      inc  esi

    Flip_Y_Largeur_multiple_de_2:

    shr  cx,1
    jnc  Flip_Y_Largeur_multiple_de_4

      mov  ax,[esi]
      mov  bx,[edi]
      mov  [edi],ax
      mov  [esi],bx
      add  edi,2
      add  esi,2

    Flip_Y_Largeur_multiple_de_4:

    ; Avant de se mettre … faire de l'inversion 32 bits, on v‚rifie qu'il
    ; reste bien des dwords … traiter (cas d'une brosse trop ‚troite)

    or  cx,cx
    jz  Flip_Y_Fin_de_ligne

    Flip_Y_Pour_chaque_pixel:

      mov  eax,[esi]
      mov  ebx,[edi]
      mov  [edi],eax
      mov  [esi],ebx
      add  edi,4
      add  esi,4

      dec  cx
      jnz  Flip_Y_Pour_chaque_pixel

    Flip_Y_Fin_de_ligne:

    ; On change de ligne, il faut donc mettre … jour ESI et EDI: ESI doit
    ; pointer sur le d‚but de la ligne suivante (ce qui est d‚ja le cas), et
    ; EDI sur le d‚but de la ligne pr‚c‚dente

    movzx ebx,Brosse_Largeur
    sub  edi,ebx  ; EDI se retrouve en d‚but de la ligne en cours
    sub  edi,ebx  ; EDI se retrouve en d‚but de ligne pr‚c‚dente

    jmp  Flip_Y_Tant_que

  Flip_Y_Fin_tant_que:


  pop  edi
  pop  esi
  pop  ebx

  ret

Flip_Y_LOWLEVEL endp







; -- Effectuer une invertion de la brosse / une droite verticale --

Flip_X_LOWLEVEL proc near

  push esi
  push edi

  ; On va se servir de ESI pour pointer sur la partie gauche de la brosse et
  ; de EDI pour pointer sur la partie droite.

  mov  esi,Brosse

  xor  eax,eax
  mov  edi,esi
  mov  ax,Brosse_Largeur
  dec  ax
  add  edi,eax

  ; On met dans EDX la largeur d'une ligne de brosse
  xor  edx,edx
  mov   dx,Brosse_largeur

  Flip_X_Tant_que:

    ; On sauve les valeurs initiales des pointeurs pour les changements de
    ; colonne

    mov  MODE_X_Valeur_initiale_de_esi,esi
    mov  MODE_X_Valeur_initiale_de_edi,edi

    cmp  edi,esi
    jbe  Flip_X_Fin_tant_que

    ; Il faut inverser les colonnes de la brosse point‚es par EDI et ESI
    ; ("Brosse_Hauteur" octets en tout)

    mov  cx,Brosse_Hauteur

    Flip_X_Pour_chaque_ligne:

      mov  al,[esi]
      mov  ah,[edi]
      mov  [edi],al
      mov  [esi],ah
      add  edi,edx
      add  esi,edx

      dec  cx
      jnz  Flip_X_Pour_chaque_ligne


    ; On change de colonne, il faut donc mettre … jour ESI et EDI: ESI doit
    ; pointer sur la colonne suivante, et EDI sur la colonne pr‚c‚dente

    mov  esi,MODE_X_Valeur_initiale_de_esi
    mov  edi,MODE_X_Valeur_initiale_de_edi
    inc  esi
    dec  edi

    jmp  Flip_X_Tant_que

  Flip_X_Fin_tant_que:


  pop  edi
  pop  esi

  ret

Flip_X_LOWLEVEL endp








; -- Effectuer une rotation de 90ø sur la brosse --

Rotate_90_deg_LOWLEVEL proc near

  push ebp
  mov  ebp,esp

  arg  Source:dword,Destination:dword

  ; Brosse_Largeur & Brosse_Hauteur doivent ˆtre les dimensions de la brosse
  ; Source

  push ebx
  push esi
  push edi

  ; On place ESI sur le point haut-droit de la brosse source:
  xor  eax,eax
  mov  esi,Source
  mov  ax,Brosse_Largeur
  add  esi,eax
  dec  esi

  ; Et on m‚morise son emplacement initial:
  mov  MODE_X_Valeur_initiale_de_esi,esi

  ; On place EDI sur le point haut-gauche de la brosse destination:
  mov  edi,Destination

  ; On place dans EBX la distance entre 2 lignes dans la source:
  xor  ebx,ebx
  mov  bx,Brosse_Largeur

  ; On place dans DX le nombre de lignes/destination (colonnes/source) …
  ; traiter:
  mov  dx,bx

  Rotate_90_deg_Pour_chaque_ligne:

    mov  esi,MODE_X_Valeur_initiale_de_esi

    ; On place dans CX le nombre de colonnes/destination (lignes/source) …
    ; traiter:
    mov  cx,Brosse_Hauteur

    Rotate_90_deg_Pour_chaque_colonne:

      ; On transfert le pixel:
      mov  al,[esi]
      mov  [edi],al

      ; Et on passe … la colonne de destination suivante:
      add  esi,ebx
      inc  edi
      dec  cx
      jnz  Rotate_90_deg_Pour_chaque_colonne

    ; Et on passe … la ligne de destination suivante:
    dec  MODE_X_Valeur_initiale_de_esi
    dec  dx
    jnz  Rotate_90_deg_Pour_chaque_ligne

  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Rotate_90_deg_LOWLEVEL endp








; -- Effectuer une rotation de 180ø sur la brosse (version compatible 386) --

Rotate_180_deg_LOWLEVEL proc near

  push ebx
  push esi
  push edi

  ; On va se servir de ESI pour pointer sur la partie sup‚rieure de la brosse
  ; et de EDI pour pointer sur la partie basse.

  mov  esi,Brosse

  xor  eax,eax
  xor  ebx,ebx
  mov  ax,Brosse_Hauteur
  mov  bx,Brosse_Largeur
  dec  ax
  mov  edi,Brosse
  mul  ebx
  add  edi,eax

  movzx edx,Brosse_Largeur ; EDX contient la largeur de la brosse

  Rotate_180_deg_Tant_que:

    cmp  edi,esi
    jbe  Rotate_180_deg_Fin_tant_que

    ; Il faut inverser les lignes de la brosse point‚es par EDI et ESI
    ; (EDX octets en tout)

    mov  ecx,edx

    ; Comme on fait pas un Flip_Y mais une rotation de 180ø, on "ejecte" EDI
    ; au bout de sa ligne
    add  edi,edx
    dec  edi

    ; On va se pr‚parer … une inversion 16 bits de lignes. Pour cela, on
    ; traite tout de suite les exc‚dants

    shr  cx,1
    jnc  Rotate_180_deg_Largeur_multiple_de_2

      mov  al,[esi]
      mov  ah,[edi]
      mov  [edi],al
      mov  [esi],ah
      dec  edi
      inc  esi

    Rotate_180_deg_Largeur_multiple_de_2:

    ;  On s'apprˆte … faire un traitement sur des words, on doit donc se
    ; laisser la place d'y acc‚der.

    dec  edi

    ; Avant de se mettre … faire de l'inversion 16 bits, on v‚rifie qu'il
    ; reste bien des words … traiter (cas d'une brosse trop ‚troite)

    or  cx,cx
    jz  Rotate_180_deg_Fin_de_ligne

    Rotate_180_deg_Pour_chaque_pixel:

      mov  ax,[esi]
      mov  bx,[edi]
      xchg ah,al ; On doit permutter les 2 octets pour que l'inversion soit correcte
      xchg bh,bl ; On doit permutter les 2 octets pour que l'inversion soit correcte
      mov  [edi],ax
      mov  [esi],bx
      sub  edi,2
      add  esi,2

      dec  cx
      jnz  Rotate_180_deg_Pour_chaque_pixel

    Rotate_180_deg_Fin_de_ligne:

    ; On change de ligne, il faut donc mettre … jour ESI et EDI: ESI doit
    ; pointer sur le d‚but de la ligne suivante (ce qui est d‚j… le cas), et
    ; EDI sur le d‚but de la ligne pr‚c‚dente

    ; Note: EDI se retrouve d‚j… au d‚but de la ligne en cours puisqu'on l'a
    ;      ‚ject‚ (au d‚but de traitement de la ligne) en fin de ligne, et
    ;      qu'il a fait le parcours de la ligne en sens inverse. Seulement,
    ;      il a l‚gŠrement d‚bord‚ sur la gauche, c'est pourquoi on doit le
    ;      corriger par une incr‚mentation de 2 (car on a trait‚ des words).

    add  edi,2   ; EDI se retrouve en debut de la ligne en cours
    sub  edi,edx ; EDI se retrouve en d‚but de ligne pr‚c‚dente

    jmp  Rotate_180_deg_Tant_que

  Rotate_180_deg_Fin_tant_que:


  pop  edi
  pop  esi
  pop  ebx

  ret

Rotate_180_deg_LOWLEVEL endp





; Etirer une ligne selon un facteur donn‚ vers un buffer

Zoomer_une_ligne proc near

  push ebp
  mov  ebp,esp

  arg  Ligne_originale:dword,Ligne_zoomee:dword,Facteur:word,Largeur:word

  push esi
  push edi

  ; On place dans ESI l'adresse de la ligne orignale (… zoomer):
  mov  esi,Ligne_originale

  ; On place dans EDI l'adresse de la ligne r‚sultat (zoom‚e):
  mov  edi,Ligne_zoomee

  ; On place dans DX le nombre de pixels … zoomer sur la ligne:
  mov  dx,Largeur

  ; On nettoye la partie haute de ECX:
  xor  ecx,ecx

  ZUL_Pour_chaque_pixel:

    ; On lit la couleur … recopier dans AL
    mov  cl,[esi]
    mov  ch,cl
    mov  ax,cx
    shl  eax,16
    mov  ax,cx

    ; On place dans ECX le facteur:
    mov  cx,Facteur

    ; On recopie ECX fois AL dans EDI le + vite possible par copie 8/16/32 bits
    shr  cx,1
    jnc  ZUL_ECX_multiple_de_2
    stosb
    ZUL_ECX_multiple_de_2:
    shr  cx,1
    jnc  ZUL_ECX_multiple_de_4
    stosw
    ZUL_ECX_multiple_de_4:
    rep  stosd

    ; On passe au pixel suivant:
    inc  esi
    dec  dx
    jnz  ZUL_Pour_chaque_pixel


  pop  edi
  pop  esi

  mov  esp,ebp
  pop  ebp

  ret

Zoomer_une_ligne endp






; -- Copier une partie d'une image vers une autre --

Copier_une_partie_d_image_dans_une_autre proc near

  push ebp
  mov  ebp,esp

  arg  Source:dword,S_Pos_X:word,S_Pos_Y:word,Largeur:word,Hauteur:word,Largeur_source:word,Destination:dword,D_Pos_X:word,D_Pos_Y:word,Largeur_destination:word

  push ebx
  push esi
  push edi

  ; On place dans ESI l'adresse de la source en (S_Pos_X,S_Pos_Y)
  xor  eax,eax
  xor  ebx,ebx
  xor  ecx,ecx
  mov   ax,S_Pos_Y
  mov   bx,Largeur_source
  mov   cx,S_Pos_X
  mul  ebx
  mov  esi,Source
  add  eax,ecx
  add  esi,eax

  ; On place dans EDI l'adresse de la destination en (D_Pos_X,D_Pos_Y)
  xor  eax,eax
  mov   ax,D_Pos_Y
  mov   bx,Largeur_destination
  mov   cx,D_Pos_X
  mul  ebx
  mov  edi,Destination
  add  eax,ecx
  add  edi,eax

  ; On place dans EAX la distance entre la fin d'une ligne de l'image source
  ; et le d‚but de la suivante … l'abscisse S_Pos_X:
  xor  eax,eax
  mov   ax,Largeur_source
  sub   ax,Largeur

  ; On place dans EBX la distance entre la fin d'une ligne de l'image
  ; destination et le d‚but de la suivante … l'abscisse D_Pos_X:
  xor  ebx,ebx
  mov   bx,Largeur_destination
  sub   bx,Largeur

  ; On place dans DX le nombre de lignes … traiter
  mov  dx,Hauteur

  CUPDIDUA_Pour_chaque_ligne:

    ; On place dans CX le nombre de pixels … traiter sur la ligne:
    mov  cx,Largeur

    ; On fait une copie au mieux par 8/16/32 bits:
    shr  cx,1
    jnc  CUPDIDUA_CX_multiple_de_2
      movsb
    CUPDIDUA_CX_multiple_de_2:
    shr  cx,1
    jnc  CUPDIDUA_CX_multiple_de_4
      movsw
    CUPDIDUA_CX_multiple_de_4:
    repnz movsd

    ; On passe … la ligne suivante:
    add  esi,eax
    add  edi,ebx
    dec  dx
    jnz  CUPDIDUA_Pour_chaque_ligne


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Copier_une_partie_d_image_dans_une_autre endp








_TEXT ENDS
END













; -- Gestion de l'effet Smooth

Effet_Smooth2 proc near

  push ebp
  mov  ebp,esp

  arg  X:word,Y:word,Couleur:byte

  push ebx
  push esi
  push edi


  ; On fait tout de suite pointer ESI sur les coordonn‚es (X,Y) dans l'image
  xor  eax,eax
  xor  edx,edx
  xor  esi,esi
  mov   ax,Y
  mov   dx,Principal_Largeur_image
  mov   si,X
  mul  edx
  add  esi,eax
  add  esi,FX_Feedback_Ecran

  ; On v‚rifie d'abord que le point ne soit pas … la p‚riph‚rie de l'image

  cmp  X,0
  je   Effet_Smooth_Sur_la_peripherie
  cmp  Y,0
  je   Effet_Smooth_Sur_la_peripherie
  mov  ax,Principal_Largeur_image
  mov  dx,Principal_Hauteur_image
  dec  ax
  dec  dx
  cmp  X,ax
  je   Effet_Smooth_Sur_la_peripherie
  cmp  Y,dx
  jne  Effet_Smooth_Traitement_normal

  Effet_Smooth_Sur_la_peripherie:

  ; On est sur la p‚riph‚rie, donc on ne modifie pas la couleur du point de
  ; l'image.

  mov  al,[esi]
  jmp  Effet_Smooth_Fin_de_traitement

  Effet_Smooth_Traitement_normal:

  ; On va se servir de BX,CX et DX comme cumuleurs de Rouge,Vert et Bleu

  xor  edi,edi
  xor  ah,ah
  mov  al,[esi]  ; AX = couleur du pixel en (X,Y)
  mov  di,ax     ; DI = couleur du pixel en (X,Y)
  add  ax,ax     ; AX = 2 * couleur du pixel en (X,Y)
  add  di,ax     ; DI = 3 * couleur du pixel en (X,Y)
  add  edi,offset Principal_Palette ; EDI pointe sur la palette[Couleur du pixel en (X,Y)].R
  xor  ah,ah
  mov  al,[edi+0]
  mov  bx,ax
  mov  al,[edi+1]
  mov  cx,ax
  mov  al,[edi+2]
  mov  dx,ax
  add  bx,bx     ; On double la valeur rouge du point (X,Y)
  add  cx,cx     ; On double la valeur verte du point (X,Y)
  add  dx,dx     ; On double la valeur bleue du point (X,Y)

  inc  esi       ; On fait pointer ESI sur (X+1,Y)
  xor  edi,edi
  xor  ah,ah
  mov  al,[esi]  ; AX = couleur du pixel en (X+1,Y)
  mov  di,ax     ; DI = couleur du pixel en (X+1,Y)
  add  ax,ax     ; AX = 2 * couleur du pixel en (X+1,Y)
  add  di,ax     ; DI = 3 * couleur du pixel en (X+1,Y)
  add  edi,offset Principal_Palette ; EDI pointe sur la palette[Couleur du pixel en (X+1,Y)].R
  xor  ah,ah      ; \_ AX = Palette[Couleur du pixel en (X+1,Y)].R
  mov  al,[edi+0] ; /
  add  bx,ax      ; On cumule les rouges dans BX
  mov  al,[edi+1] ; AX = Palette[Couleur du pixel en (X+1,Y)].V
  add  cx,ax      ; On cumule les verts dans CX
  mov  al,[edi+2] ; AX = Palette[Couleur du pixel en (X+1,Y)].B
  add  dx,ax      ; On cumule les bleus dans DX

  sub  esi,2     ; On fait pointer ESI sur (X-1,Y)
  xor  edi,edi
  xor  ah,ah
  mov  al,[esi]  ; AX = couleur du pixel en (X-1,Y)
  mov  di,ax     ; DI = couleur du pixel en (X-1,Y)
  add  ax,ax     ; AX = 2 * couleur du pixel en (X-1,Y)
  add  di,ax     ; DI = 3 * couleur du pixel en (X-1,Y)
  add  edi,offset Principal_Palette ; EDI pointe sur la palette[Couleur du pixel en (X-1,Y)].R
  xor  ah,ah      ; \_ AX = Palette[Couleur du pixel en (X-1,Y)].R
  mov  al,[edi+0] ; /
  add  bx,ax      ; On cumule les rouges dans BX
  mov  al,[edi+1] ; AX = Palette[Couleur du pixel en (X-1,Y)].V
  add  cx,ax      ; On cumule les verts dans CX
  mov  al,[edi+2] ; AX = Palette[Couleur du pixel en (X-1,Y)].B
  add  dx,ax      ; On cumule les bleus dans DX

  add  esi,1025  ; On fait pointer ESI sur (X,Y+1)
  xor  edi,edi
  xor  ah,ah
  mov  al,[esi]  ; AX = couleur du pixel en (X,Y+1)
  mov  di,ax     ; DI = couleur du pixel en (X,Y+1)
  add  ax,ax     ; AX = 2 * couleur du pixel en (X,Y+1)
  add  di,ax     ; DI = 3 * couleur du pixel en (X,Y+1)
  add  edi,offset Principal_Palette ; EDI pointe sur la palette[Couleur du pixel en (X,Y+1)].R
  xor  ah,ah      ; \_ AX = Palette[Couleur du pixel en (X,Y+1)].R
  mov  al,[edi+0] ; /
  add  bx,ax      ; On cumule les rouges dans BX
  mov  al,[edi+1] ; AX = Palette[Couleur du pixel en (X,Y+1)].V
  add  cx,ax      ; On cumule les verts dans CX
  mov  al,[edi+2] ; AX = Palette[Couleur du pixel en (X,Y+1)].B
  add  dx,ax      ; On cumule les bleus dans DX

  sub  esi,2048  ; On fait pointer ESI sur (X,Y-1)
  xor  edi,edi
  xor  ah,ah
  mov  al,[esi]  ; AX = couleur du pixel en (X,Y-1)
  mov  di,ax     ; DI = couleur du pixel en (X,Y-1)
  add  ax,ax     ; AX = 2 * couleur du pixel en (X,Y-1)
  add  di,ax     ; DI = 3 * couleur du pixel en (X,Y-1)
  add  edi,offset Principal_Palette ; EDI pointe sur la palette[Couleur du pixel en (X,Y-1)].R
  xor  ah,ah      ; \_ AX = Palette[Couleur du pixel en (X,Y-1)].R
  mov  al,[edi+0] ; /
  add  bx,ax      ; On cumule les rouges dans BX
  mov  al,[edi+1] ; AX = Palette[Couleur du pixel en (X,Y-1)].V
  add  cx,ax      ; On cumule les verts dans CX
  mov  al,[edi+2] ; AX = Palette[Couleur du pixel en (X,Y-1)].B
  add  dx,ax      ; On cumule les bleus dans DX

  mov  di,6

  mov  ax,dx   ; On passe le paramŠtre (Cumul_bleu / 6)
  xor  dx,dx
  div  di
  cmp  ah,3
  jb   Effet_Smooth_Pas_plus_bleu
    inc  al
  Effet_Smooth_Pas_plus_bleu:
  push eax

  mov  ax,cx   ; On passe le paramŠtre (Cumul_vert / 6)
  xor  dx,dx
  div  di
  cmp  ah,3
  jb   Effet_Smooth_Pas_plus_vert
    inc  al
  Effet_Smooth_Pas_plus_vert:
  push eax

  mov  ax,bx   ; On passe le paramŠtre (Cumul_rouge / 6)
  xor  dx,dx
  div  di
  cmp  ah,3
  jb   Effet_Smooth_Pas_plus_rouge
    inc  al
  Effet_Smooth_Pas_plus_rouge:
  push eax

  call Meilleure_couleur
  add  esp,12  ; 12 = 3 paramŠtres cod‚s sur 4 octets

  Effet_Smooth_Fin_de_traitement:


  pop  edi
  pop  esi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Effet_Smooth2 endp



; -- Remplacer une couleur par une autre dans la brosse --

Remap_brush_LOWLEVEL proc near

  push ebp
  mov  ebp,esp

  arg  Table_de_conversion:dword

  push ebx
  push edi


  ; On place dans EDI l'adresse du d‚but de la brosse
  mov  edi,Brosse

  ; On place dans EAX le nombre total de pixels … convertir
  xor  eax,eax
  xor  ecx,ecx
  mov  ax,Brosse_Largeur
  mov  cx,Brosse_Hauteur
  mul  ecx

  ; On place dans ECX le nb de pixels … traiter
  mov  ecx,eax

  ; On place dans EBX l'adresse de la table de conversion
  mov  ebx,Table_de_conversion

  ; On nettoie la partie haute de EAX pour qu'aprŠs chaque lecture de AL,
  ; EAX puisse servir d'index dans la table de conversion point‚e par EBX
  xor  eax,eax

  Remap_brush_Pour_chaque_pixel:

    ; On lit la couleur du pixel en cours:
    mov  al,[edi]

    ; On fait la conversion de la couleur … l'aide de la table de conversion
    mov  al,[ebx+eax]

    ; On replace la nouvelle couleur … l'emplacement en cours
    mov  [edi],al

    ; On passe au pixel suivant:
    inc  edi

    dec  ecx
    jnz  Remap_brush_Pour_chaque_pixel


  pop  edi
  pop  ebx

  mov  esp,ebp
  pop  ebp

  ret

Remap_brush_LOWLEVEL endp
