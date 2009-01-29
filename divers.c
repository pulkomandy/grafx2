/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2007 Adrien Destugues
    Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <SDL.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "struct.h"
#include "sdlscreen.h"
#include "global.h"
#include "graph.h" //Afficher_curseur
#include "erreurs.h"
#include "boutons.h"
#include "moteur.h"
#include "divers.h"
#include "clavier.h"
#include "sdlscreen.h"
#include "windows.h"
#include "palette.h"
#include "input.h"

word Palette_Compter_nb_couleurs_utilisees(dword* Tableau)
{
  int Nombre_De_Pixels=0;
  Uint8* Pixel_Courant=Principal_Ecran;
  Uint8 Couleur;
  word Nombre_Couleurs=0;
  int i;

  for (i=0;i<256;i++) Tableau[i]=0;

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
    PaletteSDL[i].r=(Palette[i].R=Palette_Scale_Component(Palette[i].R));
    PaletteSDL[i].g=(Palette[i].V=Palette_Scale_Component(Palette[i].V));
    PaletteSDL[i].b=(Palette[i].B=Palette_Scale_Component(Palette[i].B));
  }
  SDL_SetPalette(Ecran_SDL, SDL_PHYSPAL | SDL_LOGPAL, PaletteSDL,0,256);
}

void Set_color(byte Couleur, byte Rouge, byte Vert, byte Bleu)
{
  SDL_Color comp;
  comp.r=Rouge;
  comp.g=Vert;
  comp.b=Bleu;
  SDL_SetPalette(Ecran_SDL, SDL_LOGPAL, &comp, Couleur, 1);
}

void Attendre_fin_de_click(void)
{
    SDL_Event event;

    if(Mouse_K==0) return; // Le bouton était déjà relaché

    //On attend que l'utilisateur relache la souris. Tous les autres évènements
    //sont ignorés
    while(SDL_PollEvent(&event)) // ça devrait être un Wait_event, mais ça foire méchament
        // 1 ça bloque le prog complètement
        // 2 ça marche pas quand on appuie sur espace
    {
      Gere_Evenement_SDL(&event);
      if (event.type == SDL_MOUSEBUTTONUP)
        break;
    }

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
  byte* Pixel_Courant=Ecran; //dl
  int i;

  Nombre_De_Pixels=Principal_Hauteur_image*Principal_Largeur_image;

  for(i=0;i<Nombre_De_Pixels;i++)
  {
    if (Pochoir[*Pixel_Courant]==0)
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

void Sensibilite_souris(__attribute__((unused)) word X,__attribute__((unused)) word Y)
{

}

void Initialiser_chrono(dword Delai)
// Démarrer le chrono
{
  Chrono_delay = Delai;
  Chrono_cmp = SDL_GetTicks()/55;
  return;
}

void Wait_VBL(void)
// Attente de VBL. Pour avoir des scrollbars qui ont une vitesse raisonnable par exemple.
// SDL ne sait pas faire, alors on simule un timer qui a une fréquence de 100Hz,
// sans charger inutilement le CPU par du busy-wait (on n'est pas à 10ms près)
{
  const int Delai = 10;

  Uint32 debut;
  debut = SDL_GetTicks();
  // Première attente : le complément de "Delai" millisecondes
  SDL_Delay(Delai - (debut % Delai));
  // Si ça ne suffit pas, on complète par des attentes successives de "1ms".
  // (Remarque, Windows arrondit généralement aux 10ms supérieures)
  while (SDL_GetTicks() / Delai <= debut / Delai)
  {
    SDL_Delay(1);
  }
}

void Pixel_dans_brosse             (word X,word Y,byte Couleur)
{
  *(Brosse+Y*Brosse_Largeur+X)=Couleur;
}

byte Lit_pixel_dans_brosse         (word X,word Y)
{
  return *(Brosse + Y * Brosse_Largeur + X);
}


byte Lit_pixel_dans_ecran_courant  (word X,word Y)
{
        return *(Principal_Ecran+Y*Principal_Largeur_image+X);
}

void Pixel_dans_ecran_courant      (word X,word Y,byte Couleur)
{
    byte* dest=(X+Y*Principal_Largeur_image+Principal_Ecran);
    *dest=Couleur;
}

void Remplacer_une_couleur(byte Ancienne_couleur, byte Nouvelle_couleur)
{
  byte* edi;

  // pour chaque pixel :
  for(edi = Principal_Ecran;edi < Principal_Ecran + Principal_Hauteur_image * Principal_Largeur_image;edi++)
    if (*edi == Ancienne_couleur)
      *edi = Nouvelle_couleur;
  UpdateRect(0,0,0,0); // On peut TOUT a jour
  // C'est pas un problème car il n'y a pas de preview
}

void Ellipse_Calculer_limites(short Rayon_horizontal,short Rayon_vertical)
{
  Ellipse_Rayon_horizontal_au_carre =
    Rayon_horizontal * Rayon_horizontal;
  Ellipse_Rayon_vertical_au_carre =
    Rayon_vertical * Rayon_vertical;
  Ellipse_Limite = Ellipse_Rayon_horizontal_au_carre * Ellipse_Rayon_vertical_au_carre;
}

byte Pixel_dans_ellipse(void)
{
  uint64_t ediesi = Ellipse_Curseur_X * Ellipse_Curseur_X * Ellipse_Rayon_vertical_au_carre +
        Ellipse_Curseur_Y * Ellipse_Curseur_Y * Ellipse_Rayon_horizontal_au_carre;
  if((ediesi) <= Ellipse_Limite) return 255;

        return 0;
}

byte Pixel_dans_cercle(void)
{
        if(Cercle_Curseur_X * Cercle_Curseur_X +
            Cercle_Curseur_Y * Cercle_Curseur_Y <= Cercle_Limite)
                return 255;
        return 0;
}

void Copier_une_partie_d_image_dans_une_autre(byte * Source,word S_Pos_X,word S_Pos_Y,word Largeur,word Hauteur,word Largeur_source,byte * Destination,word D_Pos_X,word D_Pos_Y,word Largeur_destination)
{
        // ESI = adresse de la source en (S_Pox_X,S_Pos_Y)
        byte* esi = Source + S_Pos_Y * Largeur_source + S_Pos_X;

        // EDI = adresse de la destination (D_Pos_X,D_Pos_Y)
        byte* edi = Destination + D_Pos_Y * Largeur_destination + D_Pos_X;

        int Ligne;

        // Pour chaque ligne
        for (Ligne=0;Ligne < Hauteur; Ligne++)
        {
                memcpy(edi,esi,Largeur);

                // Passe à la ligne suivante
                esi+=Largeur_source;
                edi+=Largeur_destination;
        }


}

byte Lit_pixel_dans_ecran_brouillon(word X,word Y)
{
  return *(Brouillon_Ecran+Y*Brouillon_Largeur_image+X);
}

void Rotate_90_deg_LOWLEVEL(byte * Source,byte * Destination)
{
  byte* esi;
  byte* edi;
  word dx,bx,cx;

  //ESI = Point haut droit de la source
  byte* Debut_de_colonne = Source + Brosse_Largeur - 1;
  edi = Destination;

  // Largeur de la source = Hauteur de la destination
  dx = bx = Brosse_Largeur;

  // Pour chaque ligne
  for(dx = Brosse_Largeur;dx>0;dx--)
  {
    esi = Debut_de_colonne;
    // Pout chaque colonne
    for(cx=Brosse_Hauteur;cx>0;cx--)
    {
      *edi = *esi;
      esi+=Brosse_Largeur;
      edi++;
    }

    Debut_de_colonne--;
  }
}

// Remplacer une couleur par une autre dans un buffer

void Remap_general_LOWLEVEL(byte * Table_conv,byte * Buffer,short Largeur,short Hauteur,short Largeur_buffer)
{
  int dx,cx;

  // Pour chaque ligne
  for(dx=Hauteur;dx>0;dx--)
  {
    // Pour chaque pixel
    for(cx=Largeur;cx>0;cx--)
    {
    *Buffer = Table_conv[*Buffer];
    Buffer++;
    }
    Buffer += Largeur_buffer-Largeur;
  }
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
  return *(FX_Feedback_Ecran+Y*Principal_Largeur_image+X);
}

dword Round_div(dword Numerateur,dword Diviseur)
{
        return Numerateur/Diviseur;
}

byte Effet_Trame(word X,word Y)
{
  return Trame[X % Trame_Largeur][Y % Trame_Hauteur];
}

void Set_mouse_position(void)
{
    SDL_WarpMouse(
        Mouse_X*Pixel_width,
        Mouse_Y*Pixel_height
    );
}

void Remplacer_toutes_les_couleurs_dans_limites(byte * Table_de_remplacement)
{
  int Ligne;
  int Compteur;
  byte* Adresse;

  byte Ancien;

  // Pour chaque ligne :
  for(Ligne = Limite_Haut;Ligne <= Limite_Bas; Ligne++)
  {
    // Pour chaque pixel sur la ligne :
    for (Compteur = Limite_Gauche;Compteur <= Limite_Droite;Compteur ++)
    {
      Adresse = Principal_Ecran+Ligne*Principal_Largeur_image+Compteur;
      Ancien=*Adresse;
      *Adresse = Table_de_remplacement[Ancien];
    }
  }
}

byte Lit_pixel_dans_ecran_backup (word X,word Y)
{
  return *(Ecran_backup + X + Principal_Largeur_image * Y);
}

void Palette_256_to_64(T_Palette Palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    Palette[i].R = Palette[i].R >> 2;
    Palette[i].V = Palette[i].V >> 2;
    Palette[i].B = Palette[i].B >> 2;
  }
}

void Palette_64_to_256(T_Palette Palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    Palette[i].R = (Palette[i].R << 2)|(Palette[i].R >> 4);
    Palette[i].V = (Palette[i].V << 2)|(Palette[i].V >> 4);
    Palette[i].B = (Palette[i].B << 2)|(Palette[i].B >> 4);
  }
}

byte Effet_Colorize_interpole  (word X,word Y,byte Couleur)
{
  // Facteur_A = 256*(100-Colorize_Opacite)/100
  // Facteur_B = 256*(    Colorize_Opacite)/100
  //
  // (Couleur_dessous*Facteur_A+Couleur*facteur_B)/256
  //

  // On place dans ESI 3*Couleur_dessous ( = position de cette couleur dans la
  // palette des teintes) et dans EDI, 3*Couleur.
  byte Bleu_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].B;
  byte Bleu=Principal_Palette[Couleur].B;
  byte Vert_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].V;
  byte Vert=Principal_Palette[Couleur].V;
  byte Rouge_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].R;
  byte Rouge=Principal_Palette[Couleur].R;

  // On récupère les 3 composantes RVB

  // Bleu
  Bleu = (Table_de_multiplication_par_Facteur_B[Bleu]
    + Table_de_multiplication_par_Facteur_A[Bleu_dessous]) / 256;
  Vert = (Table_de_multiplication_par_Facteur_B[Vert]
    + Table_de_multiplication_par_Facteur_A[Vert_dessous]) / 256;
  Rouge = (Table_de_multiplication_par_Facteur_B[Rouge]
    + Table_de_multiplication_par_Facteur_A[Rouge_dessous]) / 256;
  return Meilleure_couleur(Rouge,Vert,Bleu);

}

byte Effet_Colorize_additif    (word X,word Y,byte Couleur)
{
  byte Bleu_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].B;
  byte Vert_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].V;
  byte Rouge_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].R;
  byte Bleu=Principal_Palette[Couleur].B;
  byte Vert=Principal_Palette[Couleur].V;
  byte Rouge=Principal_Palette[Couleur].R;

  return Meilleure_couleur(
    Rouge>Rouge_dessous?Rouge:Rouge_dessous,
    Vert>Vert_dessous?Vert:Vert_dessous,
    Bleu>Bleu_dessous?Bleu:Bleu_dessous);
}

byte Effet_Colorize_soustractif(word X,word Y,byte Couleur)
{
  byte Bleu_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].B;
  byte Vert_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].V;
  byte Rouge_dessous=Principal_Palette[*(FX_Feedback_Ecran + Y * Principal_Largeur_image + X)].R;
  byte Bleu=Principal_Palette[Couleur].B;
  byte Vert=Principal_Palette[Couleur].V;
  byte Rouge=Principal_Palette[Couleur].R;

  return Meilleure_couleur(
    Rouge<Rouge_dessous?Rouge:Rouge_dessous,
    Vert<Vert_dessous?Vert:Vert_dessous,
    Bleu<Bleu_dessous?Bleu:Bleu_dessous);
}

void Tester_chrono(void)
{
  if((SDL_GetTicks()/55)-Chrono_delay>Chrono_cmp) Etat_chrono=1;
}

// Effectue une inversion de la brosse selon une droite horizontale
void Flip_Y_LOWLEVEL(void)
{
  // ESI pointe sur la partie haute de la brosse
  // EDI sur la partie basse
  byte* ESI = Brosse ;
  byte* EDI = Brosse + (Brosse_Hauteur - 1) *Brosse_Largeur;
  byte tmp;
  word cx;

  while(ESI < EDI)
  {
    // Il faut inverser les lignes pointées par ESI et
    // EDI ("Brosse_Largeur" octets en tout)

    for(cx = Brosse_Largeur;cx>0;cx--)
    {
      tmp = *ESI;
      *ESI = *EDI;
      *EDI = tmp;
      ESI++;
      EDI++;
    }

    // On change de ligne :
    // ESI pointe déjà sur le début de la ligne suivante
    // EDI pointe sur la fin de la ligne en cours, il
    // doit pointer sur le début de la précédente...
    EDI -= 2 * Brosse_Largeur; // On recule de 2 lignes
  }
}

// Effectue une inversion de la brosse selon une droite verticale
void Flip_X_LOWLEVEL(void)
{
  // ESI pointe sur la partie gauche et EDI sur la partie
  // droite
  byte* ESI = Brosse;
  byte* EDI = Brosse + Brosse_Largeur - 1;

  byte* Debut_Ligne;
  byte* Fin_Ligne;
  byte tmp;
  word cx;

  while(ESI<EDI)
  {
    Debut_Ligne = ESI;
    Fin_Ligne = EDI;

    // On échange par colonnes
    for(cx=Brosse_Hauteur;cx>0;cx--)
    {
      tmp=*ESI;
      *ESI=*EDI;
      *EDI=tmp;
      EDI+=Brosse_Largeur;
      ESI+=Brosse_Largeur;
    }

    // On change de colonne
    // ESI > colonne suivante
    // EDI > colonne précédente
    ESI = Debut_Ligne + 1;
    EDI = Fin_Ligne - 1;
  }
}

// Faire une rotation de 180º de la brosse
void Rotate_180_deg_LOWLEVEL(void)
{
  // ESI pointe sur la partie supérieure de la brosse
  // EDI pointe sur la partie basse
  byte* ESI = Brosse;
  byte* EDI = Brosse + Brosse_Hauteur*Brosse_Largeur - 1;
  // EDI pointe sur le dernier pixel de la derniere ligne
  byte tmp;
  word cx;

  while(ESI < EDI)
  {
    // On échange les deux lignes pointées par EDI et
    // ESI (Brosse_Largeur octets)
    // En même temps, on échange les pixels, donc EDI
    // pointe sur la FIN de sa ligne

    for(cx=Brosse_Largeur;cx>0;cx--)
    {
      tmp = *ESI;
      *ESI = *EDI;
      *EDI = tmp;

      EDI--; // Attention ici on recule !
      ESI++;
    }
  }
}

void Tempo_jauge(byte Vitesse)
//Boucle d'attente pour faire bouger les scrollbars à une vitesse correcte
{
  Uint32 Fin;
  byte MouseK_Original = Mouse_K;
  Fin = SDL_GetTicks() + Vitesse*10;
  do
  {
    if (!Get_input()) Wait_VBL();
  } while (Mouse_K == MouseK_Original && SDL_GetTicks()<Fin);
}

void Scroll_picture(short Decalage_X,short Decalage_Y)
{
  byte* esi = Ecran_backup; //Source de la copie
  byte* edi = Principal_Ecran + Decalage_Y * Principal_Largeur_image + Decalage_X;
  const word ax = Principal_Largeur_image - Decalage_X; // Nombre de pixels à copier à droite
  word dx;
  for(dx = Principal_Hauteur_image - Decalage_Y;dx>0;dx--)
  {
  // Pour chaque ligne
    memcpy(edi,esi,ax);
    memcpy(edi - Decalage_X,esi+ax,Decalage_X);

    // On passe à la ligne suivante
    edi += Principal_Largeur_image;
    esi += Principal_Largeur_image;
  }

  // On vient de faire le traitement pour otutes les lignes au-dessous de Decalage_Y
  // Maintenant on traite celles au dessus
  edi = Decalage_X + Principal_Ecran;
  for(dx = Decalage_Y;dx>0;dx--)
  {
    memcpy(edi,esi,ax);
    memcpy(edi - Decalage_X,esi+ax,Decalage_X);

    edi += Principal_Largeur_image;
    esi += Principal_Largeur_image;
  }

  UpdateRect(0,0,0,0);
}

word Get_key(void)
{
   SDL_Event event;

  Attendre_fin_de_click(); // On prend le controle de la boucle d'évènements, donc il ne faut pas qu'on rate la fin de click !
  while(1)
  {
    SDL_WaitEvent(&event);
    if(event.type == SDL_KEYDOWN)
    {
      return Conversion_ANSI(event.key.keysym);
    }
    else
      Gere_Evenement_SDL(&event);
  }
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

/*############################################################################*/

#if defined(__WIN32__)
    #define _WIN32_WINNT 0x0500
    #include <windows.h>
#elif defined(__macosx__) || defined(__FreeBSD__)
    #include <sys/sysctl.h>
#elif defined(__BEOS__) || defined(__HAIKU__)
    // sysinfo not implemented
#elif defined(__AROS__) || defined(__amigaos4__) || defined(__MORPHOS__)
    #include <proto/exec.h>
#elif defined(__SKYOS__)
    #include <skyos/sysinfo.h>
#else
    #include <sys/sysinfo.h> // sysinfo() for free RAM
#endif

// Indique quelle est la mémoire disponible
unsigned long Memoire_libre(void)
{
    // Memory is no longer relevant. If there is ANY problem or doubt here,
    // you can simply return 10*1024*1024 (10Mb), to make the "Pages"something
    // memory allocation functions happy.

    // However, it is still a good idea to make a proper function if you can...
    // If Grafx2 thinks the memory is full, weird things may happen. And if memory
    // ever becomes full and you're still saying there are 10MB free here, the
    // program will crash without saving any picture backup ! You've been warned...
    #if defined(__WIN32__)
        MEMORYSTATUSEX mstt;
        mstt.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mstt);
        return mstt.ullAvailPhys;
    #elif defined(__macosx__) || defined(__FreeBSD__)
        int mib[2];
        int maxmem;
        size_t len;

        mib[0] = CTL_HW;
        mib[1] = HW_USERMEM;
        len = sizeof(maxmem);
        sysctl(mib,2,&maxmem,&len,NULL,0);
        return maxmem;
    #elif defined(__BEOS__) || defined(__HAIKU__) || defined(__SKYOS__) || defined(__amigaos4__)
        // No <sys/sysctl.h> on BeOS or Haiku
        // AvailMem is misleading on os4 (os4 caches stuff in memory that you can still allocate)
        #warning "There is missing code there for your platform ! please check and correct :)"
        return 10*1024*1024;
    #elif defined(__AROS__) || defined(__MORPHOS__)
        return AvailMem(MEMF_ANY);
    #else
        struct sysinfo info;
        sysinfo(&info);
        return info.freeram*info.mem_unit;
    #endif
}



// Transformer un nombre (entier naturel) en chaîne
void Num2str(dword Nombre,char * Chaine,byte Taille)
{
  int Indice;

  for (Indice=Taille-1;Indice>=0;Indice--)
  {
    Chaine[Indice]=(Nombre%10)+'0';
    Nombre/=10;
    if (Nombre==0)
      for (Indice--;Indice>=0;Indice--)
        Chaine[Indice]=' ';
  }
  Chaine[Taille]='\0';
}

// Transformer une chaîne en un entier naturel (renvoie -1 si ch. invalide)
int Str2num(char * Chaine)
{
  int Valeur=0;

  for (;*Chaine;Chaine++)
  {
    if ( (*Chaine>='0') && (*Chaine<='9') )
      Valeur=(Valeur*10)+(*Chaine-'0');
    else
      return -1;
  }
  return Valeur;
}


// Arrondir un nombre réel à la valeur entière la plus proche
short Round(float Valeur)
{
  short Temp=Valeur;

  if (Valeur>=0)
    { if ((Valeur-Temp)>= 0.5) Temp++; }
  else
    { if ((Valeur-Temp)<=-0.5) Temp--; }

  return Temp;
}

// Arrondir le résultat d'une division à la valeur entière supérieure
short Round_div_max(short Numerateur,short Diviseur)
{
  if (!(Numerateur % Diviseur))
    return (Numerateur/Diviseur);
  else
    return (Numerateur/Diviseur)+1;
}

// Retourne le minimum entre deux nombres
int Min(int A,int B)
{
  return (A<B)?A:B;
}

// Retourne le maximum entre deux nombres
int Max(int A,int B)
{
  return (A>B)?A:B;
}



// Fonction retournant le libellé d'une mode (ex: " 320x200")
char * Libelle_mode(int Mode)
{
  static char Chaine[24];
  if (! Mode_video[Mode].Fullscreen)
    return "window";
  sprintf(Chaine, "%dx%d", Mode_video[Mode].Largeur, Mode_video[Mode].Hauteur);

  return Chaine;
}

// Trouve un mode video à partir d'une chaine: soit "window",
// soit de la forme "320x200"
// Renvoie -1 si la chaine n'est pas convertible
int Conversion_argument_mode(const char *Argument)
{
  // Je suis paresseux alors je vais juste tester les libellés
  int Indice_mode;
  for (Indice_mode=0; Indice_mode<Nb_modes_video; Indice_mode++)
    // Attention les vieilles fonctions de lecture .ini mettent tout en MAJUSCULE.
    if (!strcasecmp(Libelle_mode(Indice_mode), Argument))
      return Indice_mode;

  return -1;
}
