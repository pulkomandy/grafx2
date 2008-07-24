#include <SDL/SDL.h>
#include "struct.h"
#include "sdlscreen.h"
#include "global.h"
#include "graph.h" //Afficher_curseur
#include "erreurs.h"

// Gestion du mode texte de départ (pour pouvoir y retourner en cas de problème
byte Recuperer_nb_lignes(void)
{
	puts("Recuperer_nb_lignes non implémenté!\n");
	return 0;
}

word Palette_Compter_nb_couleurs_utilisees(dword* Tableau)
{
    int Nombre_De_Pixels=0;
    Uint8* Pixel_Courant=Principal_Ecran;
    Uint8 Couleur;
    word Nombre_Couleurs=0;
    int i;

    //Calcul du nombre de pixels dans l'image
    Nombre_De_Pixels=Principal_Hauteur_image*Principal_Largeur_image;

    // On parcourt l'écran courant pour compter les utilisations des couleurs
    for(i=0;i<Nombre_De_Pixels;i++)
    {
        Couleur=*Pixel_Courant; //on lit la couleur dans l'écran

        Tableau[Couleur]++; //Un point de plus pour cette couleur

        // On passe au pixel suivant
        Pixel_Courant++;
    }

    //On va maintenant compter dans la table les couleurs utilisées:
    for(i=0;i<256;i++)
    {
	if (Tableau[i]!=0)
	    Nombre_Couleurs++;
    }

    return Nombre_Couleurs;
}

void Set_palette(T_Palette Palette)
{
	register int i;
	SDL_Color PaletteSDL[256];
	for(i=0;i<256;i++)
	{
		PaletteSDL[i].r=Palette[i].R*4; //Les couleurs VGA ne vont que de 0 à 63
		PaletteSDL[i].g=Palette[i].V*4;
		PaletteSDL[i].b=Palette[i].B*4;
	}
	SDL_SetPalette(Ecran_SDL,SDL_PHYSPAL|SDL_LOGPAL,PaletteSDL,0,256);
}

void Attendre_fin_de_click(void)
{
    SDL_Event event;

    //On attend que l'utilisateur relache la souris. Tous les autres évènements
    //sont ignorés
    while(SDL_PollEvent(&event) && event.type == SDL_MOUSEBUTTONUP);

    //On indique à la gestion des E/S que le bouton est laché et on rend la main
    Mouse_K=0;
    INPUT_Nouveau_Mouse_K=0;
}

void Effacer_image_courante_Stencil(byte Couleur, byte * Pochoir)
//Effacer l'image courante avec une certaine couleur en mode Stencil
{
    int Nombre_De_Pixels=0; //ECX
    //al=Couleur
    //edi=Ecran
    Uint8* Pixel_Courant=Ecran; //dl
    int i;

    Nombre_De_Pixels=Principal_Hauteur_image*Principal_Largeur_image;

    for(i=0;i<Nombre_De_Pixels;i++)
    {

        if (Pochoir[*Pixel_Courant]==0);
            *Pixel_Courant=Couleur;
        Pixel_Courant++;
    }
}

void Effacer_image_courante(byte Couleur)
// Effacer l'image courante avec une certaine couleur
{
    memset(
	Principal_Ecran ,
	Couleur ,
	Principal_Largeur_image * Principal_Hauteur_image
    );
}

void Sensibilite_souris(word X,word Y)
{
	puts("Sensibilite_souris non implémenté!");
}

void Get_input(void)
//Gestion des évènements: mouvement de la souris, clic sur les boutons, et utilisation du clavier.
{
    SDL_Event event;

    Touche=0;
    Touche_ASCII=0; // Par défaut, il n'y a pas d'action sur le clavier.

    if( SDL_PollEvent(&event)) /* Il y a un évènement en attente */
    {
	switch( event.type)
	{
	    case SDL_MOUSEMOTION:
		//Mouvement de la souris
		INPUT_Nouveau_Mouse_X = event.motion.x>>Mouse_Facteur_de_correction_X;
		INPUT_Nouveau_Mouse_Y = event.motion.y>>Mouse_Facteur_de_correction_Y;
	    break;
	    case SDL_MOUSEBUTTONDOWN:
		//Clic sur un des boutons de la souris
		INPUT_Nouveau_Mouse_K=event.button.button;
	    break;
	    case SDL_MOUSEBUTTONUP:
		//Bouton souris relaché
		INPUT_Nouveau_Mouse_K=0;
	    break;
	    case SDL_KEYDOWN:
		//Appui sur une touche du clavier

		//On met le scancode dans Touche"
		Touche = event.key.keysym.scancode;

		//...et le code ASCII dans Touche_ASCII
		Touche_ASCII=event.key.keysym.sym;

		//On ajoute aussi l'état des modifiers
		#define ekkm event.key.keysym.mod
		if (ekkm & (KMOD_LSHIFT | KMOD_RSHIFT))
		{
		    Touche |= 0x0100;
		}
		if (ekkm & (KMOD_LCTRL | KMOD_RCTRL))
		{
		    Touche |= 0x0200;
		}
		if (ekkm & (KMOD_LALT | KMOD_RALT))
		{
		    Touche |= 0x0400;
		}
		#undef ekkm

		//TODO revoir les scancodes qui sont dans le tableau
		//Config_Touche, ça correspond à rien !

		//Cas particulier: déplacement du curseur avec haut bas gauche droite
		//On doit interpréter ça comme un mvt de la souris
		byte ok=0;
		if(Touche == Config_Touche[0])
		{
		    //[Touche] = Emulation de MOUSE UP
		    //si on est déjà en haut on peut plus bouger
		    if(INPUT_Nouveau_Mouse_Y!=0)
		    {
			INPUT_Nouveau_Mouse_Y--;
			ok=1;
		    }
		}
		else if(Touche == Config_Touche[1])
		{
		    //[Touche] = Emulation de MOUSE DOWN
		    if(INPUT_Nouveau_Mouse_Y<Hauteur_ecran-1)
		    {
			INPUT_Nouveau_Mouse_Y++;
			ok=1;
		    }
		}
		else if(Touche == Config_Touche[2])
		{
		    //[Touche] = Emulation de MOUSE LEFT
		    if(INPUT_Nouveau_Mouse_X!=0)
		    {
			INPUT_Nouveau_Mouse_X--;
			ok=1;
		    }
		}
		else if(Touche == Config_Touche[3])
		{
		    //[Touche] = Emulation de MOUSE RIGHT

		    if(INPUT_Nouveau_Mouse_X<Largeur_ecran-1)
		    {
			INPUT_Nouveau_Mouse_X++;
			ok=1;
		    }
		}
		else if(Touche == Config_Touche[4])
		{
		    //[Touche] = Emulation de MOUSE CLICK LEFT
		    INPUT_Nouveau_Mouse_K=1;
		    ok=1;
		}
		else if(Touche == Config_Touche[5])
		{
		    //[Touche] = Emulation de MOUSE CLICK RIGHT
		    INPUT_Nouveau_Mouse_K=2;
		    ok=1;
		}

		if(ok)
		{
		    SDL_WarpMouse(
			INPUT_Nouveau_Mouse_X<<Mouse_Facteur_de_correction_X,
			INPUT_Nouveau_Mouse_Y<<Mouse_Facteur_de_correction_Y
		    );
		}
	    break;
	}
    }

    //Gestion "avancée" du curseur: interdire la descente du curseur dans le
    //menu lorsqu'on est en train de travailler dans l'image

    if(Operation_Taille_pile!=0)
    {
	byte bl=0;//BL va indiquer si on doit corriger la position du curseur

	//Si le curseur ne se trouve plus dans l'image
	if(Menu_Ordonnee<INPUT_Nouveau_Mouse_Y)
	{
	    //On bloque le curseur en fin d'image
	    bl++;
	    INPUT_Nouveau_Mouse_Y=Menu_Ordonnee-1; //La ligne !!au-dessus!! du menu
	}

	if(Loupe_Mode)
	{
	    if(Operation_dans_loupe==0)
	    {
		if(INPUT_Nouveau_Mouse_X>=Principal_Split)
		{
		    bl++;
		    INPUT_Nouveau_Mouse_X=Principal_Split-1;
		}
	    }
	    else
	    {
		if(INPUT_Nouveau_Mouse_X<Principal_X_Zoom)
		{
		    bl++;
		    INPUT_Nouveau_Mouse_X=Principal_X_Zoom;
		}
	    }
	}

	if (bl)
	{
	    SDL_WarpMouse(
		INPUT_Nouveau_Mouse_X<<Mouse_Facteur_de_correction_X,
		INPUT_Nouveau_Mouse_Y<<Mouse_Facteur_de_correction_Y
	    );
	}


	if (Touche != 0)
	{
	    //Enfin, on inhibe les touches (sauf si c'est un changement de couleur
	    //ou de taille de pinceau lors d'une des operations suivantes:
	    //OPERATION_DESSIN_CONTINU, OPERATION_DESSIN_DISCONTINU, OPERATION_SPRAY)
	    if(Autoriser_changement_de_couleur_pendant_operation)
	    {
		//A ce stade là, on sait qu'on est dans une des 3 opérations
		//supportant le changement de couleur ou de taille de pinceau.

		if(
		    (Touche != Config_Touche[6]) &&
		    (Touche != Config_Touche[7]) &&
		    (Touche != Config_Touche[8]) &&
		    (Touche != Config_Touche[9]) &&
		    (Touche != Config_Touche[10]) &&
		    (Touche != Config_Touche[11]) &&
		    (Touche != Config_Touche[12]) &&
		    (Touche != Config_Touche[13]) &&
		    (Touche != Config_Touche[14]) &&
		    (Touche != Config_Touche[15])
		)
		{
			Touche=0;
		}
	    }
	    else Touche = 0;
	}
    }

    if (
	(INPUT_Nouveau_Mouse_X != Mouse_X) ||
	(INPUT_Nouveau_Mouse_Y != Mouse_Y) ||
	(INPUT_Nouveau_Mouse_K != Mouse_K)
    )
    {
	Forcer_affichage_curseur=0;
	Effacer_curseur(); // On efface le curseur AVANT de le déplacer...
	Mouse_X=INPUT_Nouveau_Mouse_X;
	Mouse_Y=INPUT_Nouveau_Mouse_Y;
	Mouse_K=INPUT_Nouveau_Mouse_K;
	Calculer_coordonnees_pinceau();
	Afficher_curseur();
    }
}


void Initialiser_chrono(dword Delai)
// Démarrer le chrono
{
	puts("Initialiser_chrono non implémenté!");
/*
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
*/
  return;
}

void Wait_VBL(void)
// Attente de VBL. Pour avoir des scrollbars qui ont une vitesse raisonnable par exemple. SDL ne sait pas faire ?
{
	puts("Wait_VBL non implémenté!");
}

void Passer_en_mode_texte(byte Nb_lignes)
{
	SDL_Quit(); //Ceci sera appellé à chaque sortie de mode vidéo == sortie du programme. le Nb_Lignes serait à enlever, cela dit, SDL s'en occupe très bien tout seul.
}

void Pixel_dans_brosse             (word X,word Y,byte Couleur)
{
	puts("Pixel_dans_brosse non implémenté!");
}

byte Lit_pixel_dans_brosse         (word X,word Y)
{
	puts("Lit_pixel_dans_brosse non implémenté!");
	return 0;
}

void Clavier_de_depart(void)
{
	puts("Clavier_de_depart non implémenté!");
}

void Clavier_americain(void)
{
	puts("Clavier_americain non implémenté!");
}

word Detection_souris(void)
{
	puts("Detection_souris non implémenté!");
	return 0;
}

byte Lit_pixel_dans_ecran_courant  (word X,word Y)
{
	return (*(Principal_Ecran+Y*Principal_Largeur_image+X));
}

void Pixel_dans_ecran_courant      (word X,word Y,byte Couleur)
{
    byte* dest=(X+Y*Principal_Largeur_image+Principal_Ecran);
    *dest=Couleur;
}

void Remplacer_une_couleur(byte Ancienne_couleur, byte Nouvelle_couleur)
{
	puts("Remplacer_une_couleur non implémenté!");
}

void Ellipse_Calculer_limites(short Rayon_horizontal,short Rayon_vertical)
{
	puts("Ellipse_Calculer_limites non implémenté!");
}

byte Pixel_dans_ellipse(void)
{
	puts("Pixel_dans_ellipse non implémenté!");
	return 0;
}

byte Pixel_dans_cercle(void)
{
	puts("Pixel_dans_cercle non implémenté!");
	return 0;
}

void Copier_une_partie_d_image_dans_une_autre(byte * Source,word S_Pos_X,word S_Pos_Y,word Largeur,word Hauteur,word Largeur_source,byte * Destination,word D_Pos_X,word D_Pos_Y,word Largeur_destination)
{
	puts("Copier_une_partie_d_image_dans_une_autre non implémenté!");
}

byte Lit_pixel_dans_ecran_brouillon(word X,word Y)
{
	puts("Lit_pixel_dans_ecran_brouillon non implémenté!");
	return 0;
}

void Rotate_90_deg_LOWLEVEL(byte * Source,byte * Destination)
{
	puts("Rotate_90_deg_LOWLEVEL non implémenté!");
}

void Remap_general_LOWLEVEL(byte * Table_conv,byte * Buffer,short Largeur,short Hauteur,short Largeur_buffer)
{
	puts("Remap_general_LOWLEVEL non implémenté!");
}

void Copier_image_dans_brosse(short Debut_X,short Debut_Y,short Brosse_Largeur,short Brosse_Hauteur,word Largeur_image)
{
    byte* Src=Debut_Y*Largeur_image+Debut_X+Principal_Ecran; //Adr départ image (ESI)
    byte* Dest=Brosse; //Adr dest brosse (EDI)
    int dx;

  for (dx=Brosse_Hauteur;dx!=0;dx--)
  //Pour chaque ligne
  {

    // On fait une copie de la ligne
    memcpy(Dest,Src,Brosse_Largeur);

    // On passe à la ligne suivante
    Src+=Largeur_image;
    Dest+=Brosse_Largeur;
   }

}

byte Lit_pixel_dans_ecran_feedback (word X,word Y)
{
	puts("Lit_pixel_dans_ecran_feedback non implémenté!");
	return 0;
}

dword Round_div(dword Numerateur,dword Diviseur)
{
	return Numerateur/Diviseur;
}

byte Effet_Trame(word X,word Y)
{
	puts("Effet_Trame non implémenté!");
	return 0;
}

void Set_mouse_position(void)
{
    SDL_WarpMouse(
	Mouse_X << Mouse_Facteur_de_correction_X,
	Mouse_Y << Mouse_Facteur_de_correction_Y
    );
}

void Clip_mouse(void)
{
	puts("Clip_mouse non implémenté!");
}

void Remplacer_toutes_les_couleurs_dans_limites(byte * Table_de_remplacement)
{
	puts("Remplacer_toutes_les_couleurs_dans_limites non implémenté!");
}

byte Lit_pixel_dans_ecran_backup   (word X,word Y)
{
	puts("Lit_pixel_dans_ecran_backup non implémenté!");
	return 0;
}

byte Type_de_lecteur_de_disquette(byte Numero_de_lecteur)
// Numero_de_lecteur compris entre 0 et 3 (4 lecteurs de disquettes)
//
// Résultat = 0 : Pas de lecteur
//            1 : Lecteur 360 Ko
//            2 : Lecteur 1.2 Mo
//            3 : Lecteur 720 Ko
//            4 : Lecteur 1.4 Mo
//            5 : Lecteur 2.8 Mo (??? pas sur ???)
//            6 : Lecteur 2.8 Mo
{
	puts("Type_de_lecteur_de_disquette non implémenté!");
	return 0;
}

byte Disk_map(byte Numero_de_lecteur)
{
	puts("Disk_map non implémenté!");
	return 0;
}

byte Disque_dur_present(byte Numero_de_disque)
{
	puts("Disque_dur_present non implémenté!");
	return 0;
}

byte Lecteur_CDROM_present(byte Numero_de_lecteur)
{
	puts("Lecteur_CDROM_present non implémenté!");
	return 0;
}

long Freespace(byte Numero_de_lecteur)
{
	puts("Freespace non implémenté!");
	return 0;
}

byte Couleur_ILBM_line(word Pos_X, word Vraie_taille_ligne)
{
	puts("Couleur_ILBM_line non implémenté!");
	return 0;
}

void Palette_256_to_64(T_Palette Palette)
{
	puts("Palette_256_to_64 non implémenté!");
}

void Palette_64_to_256(T_Palette Palette)
{
	puts("Palette_64_to_256 non implémenté!");
}

byte Effet_Colorize_interpole  (word X,word Y,byte Couleur)
{
	puts("Effet_Colorize_interpole non implémenté!");
	return 0;
}

byte Effet_Colorize_additif    (word X,word Y,byte Couleur)
{
	puts("Effet_Colorize_additif non implémenté!");
	return 0;
}

byte Effet_Colorize_soustractif(word X,word Y,byte Couleur)
{
	puts("Effet_Colorize_soustractif non implémenté!");
	return 0;
}

void Tester_chrono(void)
{
	puts("Tester_chrono non implémenté!");
}

void Flip_Y_LOWLEVEL(void)
{
	puts("Flip_Y_LOWLEVEL non implémenté!");
}

void Flip_X_LOWLEVEL(void)
{
	puts("Flip_X_LOWLEVEL non implémenté!");
}

void Rotate_180_deg_LOWLEVEL(void)
{
	puts("Rotate_180_deg_LOWLEVEL non implémenté!");
}

void Tempo_jauge(byte Vitesse)
//Boucle d'attente pour faire bouger les scrollbars à une vitesse correcte
{
	while (Vitesse!=0)
	{
	    Wait_VBL();
	    Vitesse--;
	}
}

byte Meilleure_couleur_sans_exclusion(byte Rouge,byte Vert,byte Bleu)
{
	puts("Meilleure_couleur_sans_exclusion non implémenté!");
	return 0;
}

void Set_color(byte Couleur, byte Rouge, byte Vert, byte Bleu)
{
	puts("Set_color non implémenté!");
}

void Scroll_picture(short Decalage_X,short Decalage_Y)
{
	puts("Scroll_picture non implémenté!");
}

byte Get_key(void)
{
	puts("Get_key non implémenté!");
	return 0;
}

void Zoomer_une_ligne(byte* Ligne_originale, byte* Ligne_zoomee, 
	word Facteur, word Largeur
)
{
	byte couleur;
	word larg;

	// Pour chaque pixel
	for(larg=0;larg<Largeur;larg++){
		couleur = *Ligne_originale;

		memset(Ligne_zoomee,couleur,Facteur);
		Ligne_zoomee+=Facteur;

		Ligne_originale++;
	}
}
