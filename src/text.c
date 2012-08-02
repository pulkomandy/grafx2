/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2008 Adrien Destugues
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

// Pour désactiver le support TrueType, définir NOTTF
// To disable TrueType support, define NOTTF

#include <string.h>
#include <stdlib.h>
#include <ctype.h> // tolower()

// TrueType
#ifndef NOTTF
#if defined(__macosx__)
  #include <SDL_ttf/SDL_ttf.h>
#else
  #include <SDL_ttf.h>
#endif

#if defined(__CAANOO__) || defined(__WIZ__) || defined(__GP2X__)
// No X11
#elif defined(__linux__)
  #include <X11/Xlib.h>
#endif
#endif

#if defined(__macosx__)
  #import <corefoundation/corefoundation.h>
  #import <sys/param.h>
#endif

#include <SDL_image.h>
#include "SFont.h"

#include "struct.h"
#include "global.h"
#include "sdlscreen.h"
#include "io.h"
#include "errors.h"
#include "windows.h"
#include "misc.h"
#include "setup.h"

typedef struct T_Font
{
  char * Name;
  int    Is_truetype;
  int    Is_bitmap;
  char   Label[22];
  
  // Liste chainée simple
  struct T_Font * Next;
  struct T_Font * Previous;
} T_Font;
// Liste chainée des polices de texte
T_Font * font_list_start;
int Nb_fonts;

// Inspiré par Allegro
#define EXTID(a,b,c) ((((a)&255)<<16) | (((b)&255)<<8) | (((c)&255)))
#define EXTID4(a,b,c,d) ((((a)&255)<<24) | (((b)&255)<<16) | (((c)&255)<<8) | (((d)&255)))

int Compare_fonts(T_Font * font_1, T_Font * font_2)
{
  if (font_1->Is_bitmap && !font_2->Is_bitmap)
    return -1;
  if (font_2->Is_bitmap && !font_1->Is_bitmap)
    return 1;
  return strcmp(font_1->Label, font_2->Label);
}

// Ajout d'une fonte à la liste.
void Add_font(const char *name)
{
  char * font_name;
  T_Font * font;
  int size=strlen(name)+1;
  int index;
  
  // Détermination du type:

#if defined(__macosx__)

  if (size < 6) return;
  
  char strFontName[512];
  CFStringRef CFSFontName;// = CFSTR(name);

  CFSFontName = CFStringCreateWithBytes(NULL, (UInt8 *) name, size - 1, kCFStringEncodingASCII, false);
  // Fix some funny names
  CFStringGetCString(CFSFontName, strFontName, 512, kCFStringEncodingASCII);

  // Now we have a printable font name, use it
  name = strFontName;

#else
  if (size<5 ||
      name[size-5]!='.')
    return;
#endif

  font = (T_Font *)malloc(sizeof(T_Font));

  switch (EXTID(tolower(name[size-4]), tolower(name[size-3]), tolower(name[size-2])))
  {
    case EXTID('t','t','f'):
    case EXTID('f','o','n'):
    case EXTID('o','t','f'):
    case EXTID('p','f','b'):
      font->Is_truetype = 1;
      font->Is_bitmap = 0;
      break;
    case EXTID('b','m','p'):
    case EXTID('g','i','f'):
    case EXTID('j','p','g'):
    case EXTID('l','b','m'):
    case EXTID('p','c','x'):
    case EXTID('p','n','g'):
    case EXTID('t','g','a'):
    case EXTID('t','i','f'):
    case EXTID('x','c','f'):
    case EXTID('x','p','m'):
    case EXTID('.','x','v'):
      font->Is_truetype = 0;
      font->Is_bitmap = 1;
      break;
    default:
      #if defined(__macosx__)
         if(strcasecmp(&name[size-6], "dfont") == 0)
         {
           font->Is_truetype = 1;
           font->Is_bitmap = 0;
         }
         else
         {
            free(font);
            return;
         }
      #else
         free(font);
         return;
      #endif
  }

  font->Name = (char *)malloc(size);
  strcpy(font->Name, name);
  // Label
  strcpy(font->Label, "                   ");
  if (font->Is_truetype)
    font->Label[17]=font->Label[18]='T'; // Logo TT
  font_name=Find_last_slash(font->Name);
  if (font_name==NULL)
    font_name=font->Name;
  else
    font_name++;
  for (index=0; index < 17 && font_name[index]!='\0' && font_name[index]!='.'; index++)
    font->Label[index]=font_name[index];

  // Gestion Liste
  font->Next = NULL;
  font->Previous = NULL;
  if (font_list_start==NULL)
  {
    // Premiere (liste vide)
    font_list_start = font;
    Nb_fonts++;
  }
  else
  {
    int compare;
    compare = Compare_fonts(font, font_list_start);
    if (compare<=0)
    {
      if (compare==0 && !strcmp(font->Name, font_list_start->Name))
      {
        // Doublon
        free(font->Name);
        free(font);
        return;
      }
      // Avant la premiere
      font->Next=font_list_start;
      font_list_start=font;
      Nb_fonts++;
    }
    else
    {
      T_Font *searched_font;
      searched_font=font_list_start;
      while (searched_font->Next && (compare=Compare_fonts(font, searched_font->Next))>0)
        searched_font=searched_font->Next;
      // Après searched_font
      if (compare==0 && strcmp(font->Name, searched_font->Next->Name)==0)
      {
        // Doublon
        free(font->Name);
        free(font);
        return;
      }
      font->Next=searched_font->Next;
      searched_font->Next=font;
      Nb_fonts++;
    }
  }
}


// Trouve le nom d'une fonte par son numéro
char * Font_name(int index)
{
  T_Font *font = font_list_start;
  if (index<0 ||index>=Nb_fonts)
    return "";
  while (index--)
    font = font->Next;
  return font->Name;
}


// Trouve le libellé d'affichage d'une fonte par son numéro
// Renvoie un pointeur sur un buffer statique de 20 caracteres.
char * Font_label(int index)
{
  T_Font *font;
  static char label[20];
  
  strcpy(label, "                   ");
  
  // Recherche de la fonte
  font = font_list_start;
  if (index<0 ||index>=Nb_fonts)
    return label;
  while (index--)
    font = font->Next;
  
  // Libellé
  strcpy(label, font->Label);
  return label;
}


// Vérifie si une fonte donnée est TrueType
int TrueType_font(int index)
{
  T_Font *font = font_list_start;
  if (index<0 ||index>=Nb_fonts)
    return 0;
  while (index--)
    font = font->Next;
  return font->Is_truetype;
}



// Initialisation à faire une fois au début du programme
void Init_text(void)
{
  char directory_name[MAX_PATH_CHARACTERS];
  #ifndef NOTTF
  // Initialisation de TTF
  TTF_Init();
  #endif
  
  // Initialisation des fontes
  font_list_start = NULL;
  Nb_fonts=0;
  // Parcours du répertoire "fonts"
  strcpy(directory_name, Data_directory);
  strcat(directory_name, FONTS_SUBDIRECTORY);
  For_each_file(directory_name, Add_font);
  
  #if defined(__WIN32__)
    // Parcours du répertoire systeme windows "fonts"
    #ifndef NOTTF
    {
      char * WindowsPath=getenv("windir");
      if (WindowsPath)
      {
        sprintf(directory_name, "%s\\FONTS", WindowsPath);
        For_each_file(directory_name, Add_font);
      }
    }
    #endif
  #elif defined(__macosx__)
    // Récupération de la liste des fonts avec fontconfig
    #ifndef NOTTF


      int i,number;
      char home_dir[MAXPATHLEN];
      char *font_path_list[3] = {
         "/System/Library/Fonts",
         "/Library/Fonts"
      };
      number = 3;
      // Make sure we also search into the user's fonts directory
      CFURLRef url = (CFURLRef) CFCopyHomeDirectoryURLForUser(NULL);
      CFURLGetFileSystemRepresentation(url, true, (UInt8 *) home_dir, MAXPATHLEN);
      strcat(home_dir, "/Library/Fonts");
      font_path_list[2] = home_dir;

      for(i=0;i<number;i++)
         For_each_file(*(font_path_list+i),Add_font);

      CFRelease(url);
    #endif

  #elif defined(__CAANOO__) || defined(__WIZ__) || defined(__GP2X__)
  // No X11 : Only use fonts from Grafx2
  #elif defined(__linux__)
    #ifndef NOTTF
       #define USE_XLIB
    
       #ifdef USE_XLIB
       {
        int i,number;
        Display* dpy = XOpenDisplay(NULL);
        char** font_path_list = XGetFontPath(dpy,&number);
        XCloseDisplay(dpy);

        for(i=0;i<number;i++)
            For_each_file(*(font_path_list+i),Add_font);

        XFreeFontPath(font_path_list);
       }
       #endif
    #endif
  #elif defined(__amigaos4__) || defined(__amigaos__)
    #ifndef NOTTF
      For_each_file( "FONTS:_TrueType", Add_font );
    #endif
  #elif defined(__AROS__)
    #ifndef NOTTF
      For_each_file( "FONTS:TrueType", Add_font );
    #endif
  #elif defined(__BEOS__)
    #ifndef NOTTF
      For_each_file("/etc/fonts/ttfonts", Add_font);
    #endif
  #elif defined(__HAIKU__)
    #ifndef NOTTF
      For_each_file("/boot/system/data/fonts/ttfonts/", Add_font);
    #endif
  #elif defined(__SKYOS__)
    #ifndef NOTTF
      For_each_file("/boot/system/fonts", Add_font);
    #endif
  #elif defined(__MINT__)
    #ifndef NOTTF
      For_each_file("C:/BTFONTS", Add_font);
    #endif

  #endif
}

// Informe si texte.c a été compilé avec l'option de support TrueType ou pas.
int TrueType_is_supported()
{
  #ifdef NOTTF
  return 0;
  #else
  return 1;
  #endif
}

  
#ifndef NOTTF
byte *Render_text_TTF(const char *str, int font_number, int size, int antialias, int bold, int italic, int *width, int *height, T_Palette palette)
{
  TTF_Font *font;
  SDL_Surface * text_surface;
  byte * new_brush;
  int style;
  
  SDL_Color fg_color;
  SDL_Color bg_color;

  // Chargement de la fonte
  font=TTF_OpenFont(Font_name(font_number), size);
  if (!font)
  {
    return NULL;
  }
  
  // Style
  style=0;
  if (italic)
    style|=TTF_STYLE_ITALIC;
  if (bold)
    style|=TTF_STYLE_BOLD;
  TTF_SetFontStyle(font, style);
  
  // Colors: Text will be generated as white on black.
  fg_color.r=fg_color.g=fg_color.b=255;
  bg_color.r=bg_color.g=bg_color.b=0;
  // The following is alpha, supposedly unused
  bg_color.unused=fg_color.unused=255;
  
  // Text rendering: creates a 8bit surface with its dedicated palette
  if (antialias)
    text_surface=TTF_RenderText_Shaded(font, str, fg_color, bg_color );
  else
    text_surface=TTF_RenderText_Solid(font, str, fg_color);
  if (!text_surface)
  {
    TTF_CloseFont(font);
    return NULL;
  }
    
  new_brush=Surface_to_bytefield(text_surface, NULL);
  if (!new_brush)
  {
    SDL_FreeSurface(text_surface);
    TTF_CloseFont(font);
    return NULL;
  }
  
  // Import palette
  Get_SDL_Palette(text_surface->format->palette, palette);
  
  if (antialias)
  {
    int black_col;
    // Shaded text: X-Swap the color that is pure black with the BG color number,
    // so that the brush is immediately 'transparent'
    
    // Find black (c)
    for (black_col=0; black_col<256; black_col++)
    {
      if (palette[black_col].R==0 && palette[black_col].G==0 && palette[black_col].B==0)
        break;
    } // If not found: c = 256 = 0 (byte)
    
    if (black_col != Back_color)
    {
      int c;
      byte colmap[256];
      // Swap palette entries
      
      SWAP_BYTES(palette[black_col].R, palette[Back_color].R)
      SWAP_BYTES(palette[black_col].G, palette[Back_color].G)
      SWAP_BYTES(palette[black_col].B, palette[Back_color].B)
      
      // Define a colormap
      for (c=0; c<256; c++)
        colmap[c]=c;
      
      // The swap
      colmap[black_col]=Back_color;
      colmap[Back_color]=black_col;
      
      Remap_general_lowlevel(colmap, new_brush, new_brush, text_surface->w,text_surface->h, text_surface->w);
      
      // Also, make the BG color in brush palette have same RGB values as
      // the current BG color : this will help for remaps.
      palette[Back_color].R=Main_palette[Back_color].R;
      palette[Back_color].G=Main_palette[Back_color].G;
      palette[Back_color].B=Main_palette[Back_color].B;
    }
  }
  else
  {
    // Solid text: Was rendered as white on black. Now map colors:
    // White becomes FG color, black becomes BG. 2-color palette.
    // Exception: if BG==FG, FG will be set to black or white - any different color.
    long index;
    byte new_fore=Fore_color;

    if (Fore_color==Back_color)
    {
      new_fore=Best_color_perceptual_except(Main_palette[Back_color].R, Main_palette[Back_color].G, Main_palette[Back_color].B, Back_color);
    }
    
    for (index=0; index < text_surface->w * text_surface->h; index++)
    {
      if (palette[*(new_brush+index)].G < 128)
        *(new_brush+index)=Back_color;
      else
        *(new_brush+index)=new_fore;
    }
    
    // Now copy the current palette to brushe's, for consistency
    // with the indices.
    memcpy(palette, Main_palette, sizeof(T_Palette));
    
  }
  *width=text_surface->w;
  *height=text_surface->h;
  SDL_FreeSurface(text_surface);
  TTF_CloseFont(font);
  return new_brush;
}
#endif


byte *Render_text_SFont(const char *str, int font_number, int *width, int *height, T_Palette palette)
{
  SFont_Font *font;
  SDL_Surface * text_surface;
  SDL_Surface *font_surface;
  byte * new_brush;
  SDL_Rect rectangle;

  // Chargement de la fonte
  font_surface=IMG_Load(Font_name(font_number));
  if (!font_surface)
  {
    Verbose_message("Warning","Error loading font.\nThe file may be corrupt.");
    return NULL;
  }
  // Font is 24bit: Perform a color reduction
  if (font_surface->format->BitsPerPixel>8)
  {
    SDL_Surface * reduced_surface;
    int x,y,color;
    SDL_Color rgb;
    
    reduced_surface=SDL_CreateRGBSurface(SDL_SWSURFACE, font_surface->w, font_surface->h, 8, 0, 0, 0, 0);
    if (!reduced_surface)
    {
      SDL_FreeSurface(font_surface);
      return NULL;
    }
    // Set the quick palette
    for (color=0;color<256;color++)
    {
      rgb.r=((color & 0xE0)>>5)<<5;
      rgb.g=((color & 0x1C)>>2)<<5;
      rgb.b=((color & 0x03)>>0)<<6;
      SDL_SetColors(reduced_surface, &rgb, color, 1);
    }
    // Perform reduction
    for (y=0; y<font_surface->h; y++)
      for (x=0; x<font_surface->w; x++)
      {
        SDL_GetRGB(Get_SDL_pixel_hicolor(font_surface, x, y), font_surface->format, &rgb.r, &rgb.g, &rgb.b);
        color=((rgb.r >> 5) << 5) |
                ((rgb.g >> 5) << 2) |
                ((rgb.b >> 6));
        Set_SDL_pixel_8(reduced_surface, x, y, color);
      }
    
    SDL_FreeSurface(font_surface);
    font_surface=reduced_surface;
  }
  font=SFont_InitFont(font_surface);
  if (!font)
  {
    DEBUG("Font init failed",1);
    SDL_FreeSurface(font_surface);
    return NULL;
  }
  
  // Calcul des dimensions
  *height=SFont_TextHeight(font, str);
  *width=SFont_TextWidth(font, str);
  // Allocation d'une surface SDL
  text_surface=SDL_CreateRGBSurface(SDL_SWSURFACE, *width, *height, 8, 0, 0, 0, 0);
  // Copy palette
  SDL_SetPalette(text_surface, SDL_LOGPAL, font_surface->format->palette->colors, 0, 256);
  // Fill with transparent color
  rectangle.x=0;
  rectangle.y=0;
  rectangle.w=*width;
  rectangle.h=*height;
  SDL_FillRect(text_surface, &rectangle, font->Transparent);
  // Rendu du texte
  SFont_Write(text_surface, font, 0, 0, str);
  if (!text_surface)
  {
    DEBUG("Rendering failed",2);
    SFont_FreeFont(font);
    return NULL;
  }
    
  new_brush=Surface_to_bytefield(text_surface, NULL);
  if (!new_brush)
  {
    DEBUG("Converting failed",3);
    SDL_FreeSurface(text_surface);
    SFont_FreeFont(font);
    return NULL;
  }

  Get_SDL_Palette(font_surface->format->palette, palette);

  // Swap current BG color with font's transparent color
  if (font->Transparent != Back_color)
  {
    int c;
    byte colmap[256];
    // Swap palette entries
    
    SWAP_BYTES(palette[font->Transparent].R, palette[Back_color].R)
    SWAP_BYTES(palette[font->Transparent].G, palette[Back_color].G)
    SWAP_BYTES(palette[font->Transparent].B, palette[Back_color].B)
    
    // Define a colormap
    for (c=0; c<256; c++)
      colmap[c]=c;
    
    // The swap
    colmap[font->Transparent]=Back_color;
    colmap[Back_color]=font->Transparent;
    
    Remap_general_lowlevel(colmap, new_brush, new_brush, text_surface->w,text_surface->h, text_surface->w);
    
  }

  SDL_FreeSurface(text_surface);
  SFont_FreeFont(font);

  return new_brush;
}

#ifdef NOTTF
  #define TTFONLY __attribute__((unused))
#else
  #define TTFONLY
#endif

// Crée une brosse à partir des paramètres de texte demandés.
// Si cela réussit, la fonction place les dimensions dans width et height, 
// et retourne l'adresse du bloc d'octets.
byte *Render_text(const char *str, int font_number, TTFONLY int size, int TTFONLY antialias, TTFONLY int bold, TTFONLY int italic, int *width, int *height, T_Palette palette)
{
  T_Font *font = font_list_start;
  int index=font_number;
  
  // Verification type de la fonte
  if (font_number<0 ||font_number>=Nb_fonts)
    return NULL;
    
  while (index--)
    font = font->Next;
  if (font->Is_truetype)
  {
  #ifndef NOTTF 
    return Render_text_TTF(str, font_number, size, antialias, bold, italic, width, height, palette);
  #else
    return NULL;
  #endif
  }
  else
  {
    return Render_text_SFont(str, font_number, width, height, palette);
  }
}


