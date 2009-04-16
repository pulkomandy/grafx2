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
#include "errors.h"
#include "buttons.h"
#include "engine.h"
#include "misc.h"
#include "keyboard.h"
#include "sdlscreen.h"
#include "windows.h"
#include "palette.h"
#include "input.h"

word Count_used_colors(dword* usage)
{
  int nb_pixels=0;
  Uint8* current_pixel=Main_screen;
  Uint8 color;
  word nb_colors=0;
  int i;

  for (i=0;i<256;i++) usage[i]=0;

  //Calcul du nombre de pixels dans l'image
  nb_pixels=Main_image_height*Main_image_width;

  // On parcourt l'écran courant pour compter les utilisations des couleurs
  for(i=0;i<nb_pixels;i++)
  {
    color=*current_pixel; //on lit la couleur dans l'écran

    usage[color]++; //Un point de plus pour cette couleur

    // On passe au pixel suivant
    current_pixel++;
  }

  //On va maintenant compter dans la table les couleurs utilisées:
  for(i=0;i<256;i++)
  {
    if (usage[i]!=0)
        nb_colors++;
  }

  return nb_colors;
}

/// Same as ::Count_used_colors, but for a given rectangle.
word Count_used_colors_area(dword* usage, word start_x, word start_y, word width, word height)
{
  Uint8 color;
  word x,y;
  word nb_colors=0;
  int i;

  // Init usage table
  for (i=0;i<256;i++) usage[i]=0;

  // On parcourt l'écran courant pour compter les utilisations des couleurs
  for(y=0;y<height;y++)
  {
    for(x=0;x<width;x++)
    {
      color=*(Screen_pixels+((start_x+x)+(start_y+y)*Screen_width*Pixel_height)*Pixel_width); //on lit la couleur dans l'écran
      usage[color]++; //Un point de plus pour cette couleur
    }
  }
  //On va maintenant compter dans la table les couleurs utilisées:
  for(i=0;i<256;i++)
  {
    if (usage[i]!=0)
        nb_colors++;
  }
  return nb_colors;
}


void Set_palette(T_Palette palette)
{
  register int i;
  SDL_Color PaletteSDL[256];
  for(i=0;i<256;i++)
  {
    PaletteSDL[i].r=(palette[i].R=Round_palette_component(palette[i].R));
    PaletteSDL[i].g=(palette[i].G=Round_palette_component(palette[i].G));
    PaletteSDL[i].b=(palette[i].B=Round_palette_component(palette[i].B));
  }
  SDL_SetPalette(Screen_SDL, SDL_PHYSPAL | SDL_LOGPAL, PaletteSDL,0,256);
}

void Set_color(byte color, byte red, byte green, byte blue)
{
  SDL_Color comp;
  comp.r=red;
  comp.g=green;
  comp.b=blue;
  SDL_SetPalette(Screen_SDL, SDL_LOGPAL, &comp, color, 1);
}

void Wait_end_of_click(void)
{
    // On désactive tous les raccourcis clavier

    while(Mouse_K) if(!Get_input()) Wait_VBL();
}

void Hide_current_image_with_stencil(byte color, byte * stencil)
//Effacer l'image courante avec une certaine couleur en mode Stencil
{
  int nb_pixels=0; //ECX
  //al=color
  //edi=Screen_pixels
  byte* Pixel_Courant=Screen_pixels; //dl
  int i;

  nb_pixels=Main_image_height*Main_image_width;

  for(i=0;i<nb_pixels;i++)
  {
    if (stencil[*Pixel_Courant]==0)
      *Pixel_Courant=color;
    Pixel_Courant++;
  }
}

void Hide_current_image(byte color)
// Effacer l'image courante avec une certaine couleur
{
  memset(
    Main_screen ,
    color ,
    Main_image_width * Main_image_height
  );
}

void Mouse_sensitivity(__attribute__((unused)) word x,__attribute__((unused)) word y)
{

}

void Init_chrono(dword delay)
// Démarrer le chrono
{
  Timer_delay = delay;
  Timer_start = SDL_GetTicks()/55;
  return;
}

void Wait_VBL(void)
// Attente de VBL. Pour avoir des scrollbars qui ont une vitesse raisonnable par exemple.
// SDL ne sait pas faire, alors on simule un timer qui a une fréquence de 100Hz,
// sans charger inutilement le CPU par du busy-wait (on n'est pas à 10ms près)
{
  const int delay = 10;

  Uint32 debut;
  debut = SDL_GetTicks();
  // Première attente : le complément de "delay" millisecondes
  SDL_Delay(delay - (debut % delay));
  // Si ça ne suffit pas, on complète par des attentes successives de "1ms".
  // (Remarque, Windows arrondit généralement aux 10ms supérieures)
  while (SDL_GetTicks() / delay <= debut / delay)
  {
    SDL_Delay(1);
  }
}

void Pixel_in_brush             (word x,word y,byte color)
{
  *(Brush+y*Brush_width+x)=color;
}

byte Read_pixel_from_brush         (word x,word y)
{
  return *(Brush + y * Brush_width + x);
}


byte Read_pixel_from_current_screen  (word x,word y)
{
        return *(Main_screen+y*Main_image_width+x);
}

void Pixel_in_current_screen      (word x,word y,byte color)
{
    byte* dest=(x+y*Main_image_width+Main_screen);
    *dest=color;
}

void Replace_a_color(byte old_color, byte New_color)
{
  byte* edi;

  // pour chaque pixel :
  for(edi = Main_screen;edi < Main_screen + Main_image_height * Main_image_width;edi++)
    if (*edi == old_color)
      *edi = New_color;
  Update_rect(0,0,0,0); // On peut TOUT a jour
  // C'est pas un problème car il n'y a pas de preview
}
// FIXME: move to graph.c, it's the only caller

void Ellipse_compute_limites(short horizontal_radius,short vertical_radius)
{
  Ellipse_horizontal_radius_squared =
    horizontal_radius * horizontal_radius;
  Ellipse_vertical_radius_squared =
    vertical_radius * vertical_radius;
  Ellipse_limit = Ellipse_horizontal_radius_squared * Ellipse_vertical_radius_squared;
}

// FIXME: move to graph.c, it's the only caller
byte Pixel_in_ellipse(void)
{
  qword ediesi = Ellipse_cursor_X * Ellipse_cursor_X * Ellipse_vertical_radius_squared +
        Ellipse_cursor_Y * Ellipse_cursor_Y * Ellipse_horizontal_radius_squared;
  if((ediesi) <= Ellipse_limit) return 255;

        return 0;
}

// FIXME: move to graph.c, it's the only caller
byte Pixel_in_circle(void)
{
        if(Circle_cursor_X * Circle_cursor_X +
            Circle_cursor_Y * Circle_cursor_Y <= Circle_limit)
                return 255;
        return 0;
}

void Copy_part_of_image_to_another(byte * source,word source_x,word source_y,word width,word height,word source_width,byte * dest,word dest_x,word dest_y,word destination_width)
{
        // ESI = adresse de la source en (S_Pox_X,source_y)
        byte* esi = source + source_y * source_width + source_x;

        // EDI = adresse de la destination (dest_x,dest_y)
        byte* edi = dest + dest_y * destination_width + dest_x;

        int line;

        // Pour chaque ligne
        for (line=0;line < height; line++)
        {
                memcpy(edi,esi,width);

                // Passe à la ligne suivante
                esi+=source_width;
                edi+=destination_width;
        }


}

byte Read_pixel_from_spare_screen(word x,word y)
{
  return *(Spare_screen+y*Spare_image_width+x);
}

void Rotate_90_deg_lowlevel(byte * source,byte * dest)
{
  byte* esi;
  byte* edi;
  word dx,bx,cx;

  //ESI = Point haut droit de la source
  byte* Debut_de_colonne = source + Brush_width - 1;
  edi = dest;

  // Largeur de la source = hauteur de la destination
  dx = bx = Brush_width;

  // Pour chaque ligne
  for(dx = Brush_width;dx>0;dx--)
  {
    esi = Debut_de_colonne;
    // Pout chaque colonne
    for(cx=Brush_height;cx>0;cx--)
    {
      *edi = *esi;
      esi+=Brush_width;
      edi++;
    }

    Debut_de_colonne--;
  }
}

// Replace une couleur par une autre dans un buffer

void Remap_general_lowlevel(byte * conversion_table,byte * buffer,short width,short height,short buffer_width)
{
  int dx,cx;

  // Pour chaque ligne
  for(dx=height;dx>0;dx--)
  {
    // Pour chaque pixel
    for(cx=width;cx>0;cx--)
    {
    *buffer = conversion_table[*buffer];
    buffer++;
    }
    buffer += buffer_width-width;
  }
}

void Copy_image_to_brush(short start_x,short start_y,short Brush_width,short Brush_height,word image_width)
{
    byte* src=start_y*image_width+start_x+Main_screen; //Adr départ image (ESI)
    byte* dest=Brush; //Adr dest brosse (EDI)
    int dx;

  for (dx=Brush_height;dx!=0;dx--)
  //Pour chaque ligne
  {

    // On fait une copie de la ligne
    memcpy(dest,src,Brush_width);

    // On passe à la ligne suivante
    src+=image_width;
    dest+=Brush_width;
   }

}

byte Read_pixel_from_feedback_screen (word x,word y)
{
  return *(FX_feedback_screen+y*Main_image_width+x);
}

dword Round_div(dword numerator,dword divisor)
{
        return numerator/divisor;
}

byte Effect_sieve(word x,word y)
{
  return Sieve[x % Sieve_width][y % Sieve_height];
}

void Set_mouse_position(void)
{
    SDL_WarpMouse(
        Mouse_X*Pixel_width,
        Mouse_Y*Pixel_height
    );
}

void Replace_colors_within_limits(byte * replace_table)
{
  int line;
  int counter;
  byte* Adresse;

  byte old;

  // Pour chaque ligne :
  for(line = Limit_top;line <= Limit_bottom; line++)
  {
    // Pour chaque pixel sur la ligne :
    for (counter = Limit_left;counter <= Limit_right;counter ++)
    {
      Adresse = Main_screen+line*Main_image_width+counter;
      old=*Adresse;
      *Adresse = replace_table[old];
    }
  }
}

byte Read_pixel_from_backup_screen (word x,word y)
{
  return *(Screen_backup + x + Main_image_width * y);
}

void Palette_256_to_64(T_Palette palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    palette[i].R = palette[i].R >> 2;
    palette[i].G = palette[i].G >> 2;
    palette[i].B = palette[i].B >> 2;
  }
}

void Palette_64_to_256(T_Palette palette)
{
  int i;
  for(i=0;i<256;i++)
  {
    palette[i].R = (palette[i].R << 2)|(palette[i].R >> 4);
    palette[i].G = (palette[i].G << 2)|(palette[i].G >> 4);
    palette[i].B = (palette[i].B << 2)|(palette[i].B >> 4);
  }
}

byte Effect_interpolated_colorize  (word x,word y,byte color)
{
  // factor_a = 256*(100-Colorize_opacity)/100
  // factor_b = 256*(    Colorize_opacity)/100
  //
  // (Couleur_dessous*factor_a+color*facteur_B)/256
  //

  // On place dans ESI 3*Couleur_dessous ( = position de cette couleur dans la
  // palette des teintes) et dans EDI, 3*color.
  byte blue_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].B;
  byte blue=Main_palette[color].B;
  byte green_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].G;
  byte green=Main_palette[color].G;
  byte red_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].R;
  byte red=Main_palette[color].R;

  // On récupère les 3 composantes RVB

  // blue
  blue = (Facteur_B_table[blue]
    + Facteur_A_table[blue_under]) / 256;
  green = (Facteur_B_table[green]
    + Facteur_A_table[green_under]) / 256;
  red = (Facteur_B_table[red]
    + Facteur_A_table[red_under]) / 256;
  return Best_color(red,green,blue);

}

byte Effect_additive_colorize    (word x,word y,byte color)
{
  byte blue_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].B;
  byte green_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].G;
  byte red_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].R;
  byte blue=Main_palette[color].B;
  byte green=Main_palette[color].G;
  byte red=Main_palette[color].R;

  return Best_color(
    red>red_under?red:red_under,
    green>green_under?green:green_under,
    blue>blue_under?blue:blue_under);
}

byte Effect_substractive_colorize(word x,word y,byte color)
{
  byte blue_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].B;
  byte green_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].G;
  byte red_under=Main_palette[*(FX_feedback_screen + y * Main_image_width + x)].R;
  byte blue=Main_palette[color].B;
  byte green=Main_palette[color].G;
  byte red=Main_palette[color].R;

  return Best_color(
    red<red_under?red:red_under,
    green<green_under?green:green_under,
    blue<blue_under?blue:blue_under);
}

void Check_timer(void)
{
  if((SDL_GetTicks()/55)-Timer_delay>Timer_start) Timer_state=1;
}

// Effectue une inversion de la brosse selon une droite horizontale
void Flip_Y_lowlevel(void)
{
  // ESI pointe sur la partie haute de la brosse
  // EDI sur la partie basse
  byte* ESI = Brush ;
  byte* EDI = Brush + (Brush_height - 1) *Brush_width;
  byte tmp;
  word cx;

  while(ESI < EDI)
  {
    // Il faut inverser les lignes pointées par ESI et
    // EDI ("Brush_width" octets en tout)

    for(cx = Brush_width;cx>0;cx--)
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
    EDI -= 2 * Brush_width; // On recule de 2 lignes
  }
}

// Effectue une inversion de la brosse selon une droite verticale
void Flip_X_lowlevel(void)
{
  // ESI pointe sur la partie gauche et EDI sur la partie
  // droite
  byte* ESI = Brush;
  byte* EDI = Brush + Brush_width - 1;

  byte* Debut_Ligne;
  byte* Fin_Ligne;
  byte tmp;
  word cx;

  while(ESI<EDI)
  {
    Debut_Ligne = ESI;
    Fin_Ligne = EDI;

    // On échange par colonnes
    for(cx=Brush_height;cx>0;cx--)
    {
      tmp=*ESI;
      *ESI=*EDI;
      *EDI=tmp;
      EDI+=Brush_width;
      ESI+=Brush_width;
    }

    // On change de colonne
    // ESI > colonne suivante
    // EDI > colonne précédente
    ESI = Debut_Ligne + 1;
    EDI = Fin_Ligne - 1;
  }
}

// Faire une rotation de 180º de la brosse
void Rotate_180_deg_lowlevel(void)
{
  // ESI pointe sur la partie supérieure de la brosse
  // EDI pointe sur la partie basse
  byte* ESI = Brush;
  byte* EDI = Brush + Brush_height*Brush_width - 1;
  // EDI pointe sur le dernier pixel de la derniere ligne
  byte tmp;
  word cx;

  while(ESI < EDI)
  {
    // On échange les deux lignes pointées par EDI et
    // ESI (Brush_width octets)
    // En même temps, on échange les pixels, donc EDI
    // pointe sur la FIN de sa ligne

    for(cx=Brush_width;cx>0;cx--)
    {
      tmp = *ESI;
      *ESI = *EDI;
      *EDI = tmp;

      EDI--; // Attention ici on recule !
      ESI++;
    }
  }
}

void Slider_timer(byte speed)
//Boucle d'attente pour faire bouger les scrollbars à une vitesse correcte
{
  Uint32 end;
  byte original_mouse_k = Mouse_K;
  end = SDL_GetTicks() + speed*10;
  do
  {
    if (!Get_input()) Wait_VBL();
  } while (Mouse_K == original_mouse_k && SDL_GetTicks()<end);
}

void Scroll_picture(short x_offset,short y_offset)
{
  byte* esi = Screen_backup; //source de la copie
  byte* edi = Main_screen + y_offset * Main_image_width + x_offset;
  const word ax = Main_image_width - x_offset; // Nombre de pixels à copier à droite
  word dx;
  for(dx = Main_image_height - y_offset;dx>0;dx--)
  {
  // Pour chaque ligne
    memcpy(edi,esi,ax);
    memcpy(edi - x_offset,esi+ax,x_offset);

    // On passe à la ligne suivante
    edi += Main_image_width;
    esi += Main_image_width;
  }

  // On vient de faire le traitement pour otutes les lignes au-dessous de y_offset
  // Maintenant on traite celles au dessus
  edi = x_offset + Main_screen;
  for(dx = y_offset;dx>0;dx--)
  {
    memcpy(edi,esi,ax);
    memcpy(edi - x_offset,esi+ax,x_offset);

    edi += Main_image_width;
    esi += Main_image_width;
  }

  Update_rect(0,0,0,0);
}

void Zoom_a_line(byte* original_line, byte* zoomed_line,
        word factor, word width
)
{
        byte color;
        word x;

        // Pour chaque pixel
        for(x=0;x<width;x++){
                color = *original_line;

                memset(zoomed_line,color,factor);
                zoomed_line+=factor;

                original_line++;
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
unsigned long Memory_free(void)
{
    // Memory is no longer relevant. If there is ANY problem or doubt here,
    // you can simply return 10*1024*1024 (10Mb), to make the "Pages"something
    // memory allocation functions happy.

    // However, it is still a good idea to make a proper function if you can...
    // If Grafx2 thinks the memory is full, weird things may happen. And if memory
    // ever becomes full and you're still saying there are 10MB free here, the
    // program will crash without saving any picture backup ! You've been warned...
    #if defined(__WIN32__)
        MEMORYSTATUS mstt;
        mstt.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus(&mstt);
        return mstt.dwAvailPhys;
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
void Num2str(dword number,char * str,byte nb_char)
{
  int index;

  for (index=nb_char-1;index>=0;index--)
  {
    str[index]=(number%10)+'0';
    number/=10;
    if (number==0)
      for (index--;index>=0;index--)
        str[index]=' ';
  }
  str[nb_char]='\0';
}

// Transformer une chaîne en un entier naturel (renvoie -1 si ch. invalide)
int Str2num(char * str)
{
  int value=0;

  for (;*str;str++)
  {
    if ( (*str>='0') && (*str<='9') )
      value=(value*10)+(*str-'0');
    else
      return -1;
  }
  return value;
}


// Arrondir un nombre réel à la valeur entière la plus proche
short Round(float value)
{
  short temp=value;

  if (value>=0)
    { if ((value-temp)>= 0.5) temp++; }
  else
    { if ((value-temp)<=-0.5) temp--; }

  return temp;
}

// Arrondir le résultat d'une division à la valeur entière supérieure
short Round_div_max(short numerator,short divisor)
{
  if (!(numerator % divisor))
    return (numerator/divisor);
  else
    return (numerator/divisor)+1;
}

// Retourne le minimum entre deux nombres
int Min(int a,int b)
{
  return (a<b)?a:b;
}

// Retourne le maximum entre deux nombres
int Max(int a,int b)
{
  return (a>b)?a:b;
}



// Fonction retournant le libellé d'une mode (ex: " 320x200")
char * Mode_label(int mode)
{
  static char str[24];
  if (! Video_mode[mode].Fullscreen)
    return "window";
  sprintf(str, "%dx%d", Video_mode[mode].Width, Video_mode[mode].Height);

  return str;
}

// Trouve un mode video à partir d'une chaine: soit "window",
// soit de la forme "320x200"
// Renvoie -1 si la chaine n'est pas convertible
int Convert_videomode_arg(const char *argument)
{
  // Je suis paresseux alors je vais juste tester les libellés
  int mode_index;
  for (mode_index=0; mode_index<Nb_video_modes; mode_index++)
    // Attention les vieilles fonctions de lecture .ini mettent tout en MAJUSCULE.
    if (!strcasecmp(Mode_label(mode_index), argument))
      return mode_index;

  return -1;
}
