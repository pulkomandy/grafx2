#include <SDL/SDL.h>

#include "struct.h"
#include "sdlscreen.h"
#include "global.h"

word Palette_Compter_nb_couleurs_utilisees(dword* Tableau)
{
    int Nombre_De_Pixels=0; //ECX
    Uint8* Pixel_Courant=Ecran; //ESI
    Uint8 Couleur; //AL/EAX
    word Nombre_Couleurs=0;
    int i;

    //Calcul du nombre de pixels dans l'image
    Nombre_De_Pixels=Principal_Hauteur_image*Principal_Largeur_image;

    // On parcourt l'écran courant pour compter les utilisations des couleurs
    for(i=0;i>Nombre_De_Pixels;i++)
    {
        Couleur=*Pixel_Courant; //on lit la couleur dans l'écran

        Tableau[Couleur]++; //Un point de plus pour cette couleur

        // On passe au pixel suivant
        Pixel_Courant++;
    }

    //On va maintenant compter dans la table les couleurs utilisées:
    Couleur=0;
	 do
    {
		if (Tableau[Couleur]!=0)
        Nombre_Couleurs++;
		Couleur++;
    }while(Couleur!=0); //On sort quand on a fait le tour (la var est sur 8 bits donc 255+1=0)

    return Nombre_Couleurs;
}

void Set_palette(T_Palette Palette)
{
    puts("Set_Palette non implémenté!\n");
}

void Attendre_fin_de_click(void)
{
	puts("Attendre_fin_de_click non implémenté!\n");
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
    puts("Effacer_image_courante non implémenté!\n");
}

void Sensibilite_souris(word X,word Y)
{;//TODO Implémenter la sensibilité souris
}

void Get_input(void)
{
    SDL_PollEvent(Evenement_SDL);
}


void Initialiser_chrono(dword Delai)
{// Démarrer le chrono
	puts("Initialiser_chrono non implémenté!\n");
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
{
	puts("Wait_VBL non implémenté!\n");
}

void Passer_en_mode_texte(byte Nb_lignes)
{
	puts("Passer_en_mode_texte non implémenté!\n");
}

void Pixel_dans_brosse             (word X,word Y,byte Couleur)
{
	puts("Pixel_dans_brosse non implémenté!\n");
}

byte Lit_pixel_dans_brosse         (word X,word Y)
{
	puts("Lit_pixel_dans_brosse non implémenté!\n");
	return 0;
}
