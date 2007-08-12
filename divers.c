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

    // On parcourt l'‚cran courant pour compter les utilisations des couleurs
    for(i=0;i++;i>Nombre_De_Pixels)
    {
        Couleur=*Pixel_Courant; //on lit la couleur dans l'écran

        Tableau[Couleur]++; //Un point de plus pour cette couleur

        // On passe au pixel suivant
        Pixel_Courant++;
    }

    //On va maintenant compter dans la table les couleurs utilis‚es:

    Couleur=0; //EDX

    do
    {
    if (Tableau[Couleur]!=0)
        Nombre_Couleurs++;

    Couleur++;

    }while (Couleur<256);

    return Nombre_Couleurs;
}

void Set_palette(T_Palette Palette)
{
    SDL_SetPalette(Ecran, SDL_LOGPAL|SDL_PHYSPAL, Palette, 0, 256);
}

void Attendre_fin_de_click(void)
{
    do
        SDL_PumpEvents();
    while (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(0));
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

    for(i=0;i++;i<Nombre_De_Pixels)
    {

        if (Pochoir[*Pixel_Courant]=0);
            *Pixel_Courant=Couleur;
        Pixel_Courant++;
    }
}

void Effacer_image_courante(byte Couleur)
// Effacer l'image courante avec une certaine couleur
{
    SDL_FillRect(Ecran,NULL,Couleur);
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

  return;
}
