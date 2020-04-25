/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

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
#include <SDL_ttf.h>

#if defined(__CAANOO__) || defined(__WIZ__) || defined(__GP2X__)
// No fontconfig
#elif defined(USE_FC) && !defined(NOTTF)
  #include <fontconfig/fontconfig.h>
#endif
#endif

#if defined(__macosx__)
  #import <CoreFoundation/CoreFoundation.h>
  #import <sys/param.h>
#endif

#ifdef _MSC_VER
#include <stdio.h>
#define strdup _strdup
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif
#if defined(WIN32)
#include <windows.h>
#include <malloc.h>
#endif

#if defined(USE_SDL) || defined(USE_SDL2)
#include <SDL_image.h>
#include "sdlscreen.h"
#endif

#include "struct.h"
#include "global.h"
#include "io.h"
#include "errors.h"
#include "windows.h"
#include "misc.h"
#include "setup.h"
#include "loadsave.h"
#include "SFont.h"

/**
 * Element of the font linked list
 */
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

/**
 * Head of the font linked list
 */
T_Font * font_list_start;

// Inspiré par Allegro
#define EXTID(a,b,c) ((((a)&255)<<16) | (((b)&255)<<8) | (((c)&255)))
#define EXTID4(a,b,c,d) ((((a)&255)<<24) | (((b)&255)<<16) | (((c)&255)<<8) | (((d)&255)))

static int Compare_fonts(T_Font * font_1, T_Font * font_2)
{
  if (font_1->Is_bitmap && !font_2->Is_bitmap)
    return -1;
  if (font_2->Is_bitmap && !font_1->Is_bitmap)
    return 1;
  return strcmp(font_1->Label, font_2->Label);
}

static void Insert_font(T_Font * font)
{
  // Gestion Liste
  font->Next = NULL;
  font->Previous = NULL;
  if (font_list_start==NULL)
  {
    // Premiere (liste vide)
    font_list_start = font;
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
    }
  }
}

// Ajout d'une fonte à la liste.
static void Add_font(const char *name, const char * font_name)
{
  T_Font * font;
  int size=strlen(name)+1;
  int index;

  // Détermination du type:

#if defined(__macosx__)
  char strFontName[512];
  CFStringRef CFSFontName;// = CFSTR(name);

  if (size < 6) return;

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

#if defined(WIN32) && defined(NOTTF)
  // Register font in windows so they will be visible with EnumFontFamiliesEx()
  if (AddFontResourceExA(name, FR_PRIVATE, 0) > 0)
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

  font->Name = strdup(name);
  // Label
  memset(font->Label, ' ', sizeof(font->Label));
  font->Label[19] = '\0';
  if (font->Is_truetype)
    font->Label[17]=font->Label[18]='T'; // Logo TT
  for (index=0; index < 17 && font_name[index]!='\0' && font_name[index]!='.'; index++)
    font->Label[index]=font_name[index];

  Insert_font(font);
}


// Trouve le nom d'une fonte par son numéro
const char * Font_name(int index)
{
  T_Font *font = font_list_start;
  if (index < 0 || font == NULL)
    return "";
  while (index--)
  {
    font = font->Next;
    if (font == NULL)
      return "";
  }
  return font->Name;
}


// Trouve le libellé d'affichage d'une fonte par son numéro
// Renvoie un pointeur sur un buffer statique de 20 caracteres.
const char * Font_label(int index)
{
  T_Font *font = font_list_start;
  
  font = font_list_start;
  if (index < 0 || font == NULL)
    return "                   ";
  while (index--)
  {
    font = font->Next;
    if (font == NULL)
      return "                   ";
  }
  return font->Label;
}


// Vérifie si une fonte donnée est TrueType
int TrueType_font(int index)
{
  T_Font *font = font_list_start;
  if (index < 0 || font == NULL)
    return 0;
  while (index--)
  {
    font = font->Next;
    if (font == NULL)
      return 0;
  }
  return font->Is_truetype;
}

int Font_count(void)
{
  T_Font *font = font_list_start;
  int count = 0;

  while (font != NULL)
  {
    count++;
    font = font->Next;
  }
  return count;
}

#if defined(WIN32) && defined(NOTTF)
static int CALLBACK EnumFontFamCallback(CONST LOGFONTA *lpelf, CONST TEXTMETRICA *lpntm, DWORD FontType, LPARAM lParam)
{
  char type[3] = { ' ', ' ', '\0' };
  T_Font * font;

  (void)lParam; // we don't use the custom callback param
  if (FontType & TRUETYPE_FONTTYPE)
    type[0] = type[1] = 'T';
  else if (FontType & RASTER_FONTTYPE)
    type[0] = 'R';  // Raster font.
  if (((CONST NEWTEXTMETRICA *)lpntm)->ntmFlags & NTM_TT_OPENTYPE)
    type[0] = 'O';  // OpenType font
  font = (T_Font *)malloc(sizeof(T_Font));
  font->Is_bitmap = 0;
  font->Is_truetype = 1;
  snprintf(font->Label, sizeof(font->Label), "%-17.17s%s", lpelf->lfFaceName, type);
  font->Name = strdup(lpelf->lfFaceName);

  Insert_font(font);
  return 1; // non-zero : continue enumeration
}
#endif


// Initialisation à faire une fois au début du programme
void Init_text(void)
{
  char * directory_name;
  #ifndef NOTTF
  // Initialisation de TTF
  TTF_Init();
  #endif
  
  // Initialisation des fontes
  font_list_start = NULL;
  // Parcours du répertoire "fonts"
  directory_name = Filepath_append_to_dir(Data_directory, FONTS_SUBDIRECTORY);
  For_each_file(directory_name, Add_font);
  free(directory_name);
  // fonts subdirectory in Config_directory
  directory_name = Filepath_append_to_dir(Config_directory, "fonts");
  For_each_file(directory_name, Add_font);
  free(directory_name);
  
  #if defined(WIN32)
    // Parcours du répertoire systeme windows "fonts"
    #ifndef NOTTF
    {
      char * WindowsPath = getenv("windir");
      if (WindowsPath)
      {
        directory_name = Filepath_append_to_dir(WindowsPath, "FONTS");
        For_each_file(directory_name, Add_font);
        free(directory_name);
      }
    }
    #else
    {
      LOGFONTA lf;
      HDC dc = GetDC(NULL);
      memset(&lf, 0, sizeof(lf));
      lf.lfCharSet = ANSI_CHARSET /* DEFAULT_CHARSET*/;
      lf.lfPitchAndFamily = 0;
      //EnumFontsA(dc, NULL, EnumFontCallback, NULL);
      //EnumFontFamiliesA(dc, NULL, EnumFontFamCallback, 0);
      EnumFontFamiliesExA(dc, &lf, EnumFontFamCallback, 0, 0);
      ReleaseDC(NULL, dc);
    }
    #endif
  #elif defined(__macosx__)
    // Récupération de la liste des fonts avec fontconfig
    #ifndef NOTTF
    {

      int i,number;
      char * home_dir = NULL;
      const char *font_path_list[3] = {
         "/System/Library/Fonts",
         "/Library/Fonts"
      };
      number = 2;
      // Make sure we also search into the user's fonts directory
      //CFURLRef url = (CFURLRef) CFCopyHomeDirectoryURLForUser(NULL);
      //CFURLGetFileSystemRepresentation(url, true, (UInt8 *) home_dir, MAXPATHLEN);
      if (getenv("HOME") != NULL)
      {
        home_dir = Filepath_append_to_dir(getenv("HOME"), "Library/Fonts");
        font_path_list[number++] = home_dir;
      }

      for(i=0;i<number;i++)
         For_each_file(font_path_list[i],Add_font);

      free(home_dir);
      //CFRelease(url);
    }
    #endif

  #elif defined(__CAANOO__) || defined(__WIZ__) || defined(__GP2X__)
  // No fontconfig : Only use fonts from Grafx2
  #elif defined(USE_FC)
    #ifndef NOTTF
       {
	FcStrList* dirs;
        FcChar8 * fdir;
	dirs = FcConfigGetFontDirs(NULL);
	fdir = FcStrListNext(dirs);
        while(fdir != NULL)
	{
            For_each_file((char *)fdir,Add_font);
	    fdir = FcStrListNext(dirs);
	}

        FcStrListDone(dirs);
       }
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
  #if !defined(NOTTF) || defined(WIN32)
  return 1;
  #else
  return 0;
  #endif
}

void Uninit_text(void)
{
#ifndef NOTTF
  TTF_Quit();
#if defined(USE_FC)
  FcFini();
#endif
#endif
  while (font_list_start != NULL)
  {
    T_Font * font = font_list_start->Next;
    free(font_list_start->Name);
    free(font_list_start);
    font_list_start = font;
  }
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
#if defined(USE_SDL)
  bg_color.unused=fg_color.unused=255;
#elif defined(USE_SDL2)
  bg_color.a=fg_color.a=255;
#endif
  
  // Text rendering: creates a 8bit surface with its dedicated palette
  #ifdef __ANDROID__
  if (antialias)
    text_surface=TTF_RenderUTF8_Shaded(font, str, fg_color, bg_color );
  else
    text_surface=TTF_RenderUTF8_Solid(font, str, fg_color);
  #else
  if (antialias)
    text_surface=TTF_RenderText_Shaded(font, str, fg_color, bg_color );
  else
    text_surface=TTF_RenderText_Solid(font, str, fg_color);
  #endif
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
      palette[Back_color].R=Main.palette[Back_color].R;
      palette[Back_color].G=Main.palette[Back_color].G;
      palette[Back_color].B=Main.palette[Back_color].B;
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
      new_fore=Best_color_perceptual_except(Main.palette[Back_color].R, Main.palette[Back_color].G, Main.palette[Back_color].B, Back_color);
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
    memcpy(palette, Main.palette, sizeof(T_Palette));
    
  }
  *width=text_surface->w;
  *height=text_surface->h;
  SDL_FreeSurface(text_surface);
  TTF_CloseFont(font);
  return new_brush;
}
#endif

#if defined(WIN32)
byte *Render_text_Win32(const char *str, int font_number, int size, int antialias, int bold, int italic, int *width, int *height, T_Palette palette)
{
  int str_len;
  HGDIOBJ oldobj;
  HGDIOBJ oldfont;
  RECT rect;
  BITMAPINFO *bi;
  HDC dc;
  HBITMAP bm;
  SIZE s;
  byte * pixels = NULL;
  byte * new_brush = NULL;
  int i;
  int x, y;
  HFONT font;
  const char * font_name;

  font_name = Font_name(font_number);

  font = CreateFontA(size, 0, 0, 0 /* nOrientation */,
                     bold ? FW_BOLD : FW_REGULAR, italic, 0 /* underline */, 0 /*strikeout */,
                     ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                     antialias ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY /* DEFAULT_QUALITY*/,
                     DEFAULT_PITCH | FF_DONTCARE, font_name);
  if (font == NULL)
    return NULL;

  dc = CreateCompatibleDC(NULL);
  oldfont = SelectObject(dc, font);
  str_len = (int)strlen(str);
  if (!GetTextExtentPoint32A(dc, str, str_len, &s))
  {
    s.cx = 320;
    s.cy = 32;
  }

  bi = (BITMAPINFO*)_alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
  memset(bi, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biWidth = (s.cx + 3) & ~3;
	bi->bmiHeader.biHeight = -s.cy;
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biBitCount = antialias ? 32 : 8; // the GDI does Antialiasing only on TrueColor DIB
	bi->bmiHeader.biCompression = BI_RGB;

  for (i = 0; i < 256; i++) {
    bi->bmiColors[i].rgbRed = i;
    bi->bmiColors[i].rgbGreen = i;
    bi->bmiColors[i].rgbBlue = i;
    palette[i].R = i;
    palette[i].G = i;
    palette[i].B = i;
  }
  bm = CreateDIBSection(dc, bi, DIB_RGB_COLORS, (void **)&pixels, NULL, 0);
  oldobj = SelectObject(dc, bm);

  SetTextColor(dc, RGB(255,255,255));
  SetBkColor(dc, RGB(0,0,0));
  SetBkMode(dc, OPAQUE);
  rect.left=0;
  rect.top=0;
  rect.right = s.cx;
  rect.bottom = s.cy;
  DrawTextA(dc, str, str_len, &rect, DT_LEFT | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX ) ;
  GdiFlush();
  SelectObject(dc, oldobj);
  SelectObject(dc, oldfont);

  new_brush = (byte *)malloc(s.cx*s.cy);
  if (antialias)
  {
    for (y = 0; y < s.cy; y++)
    {
      // get the value from the 1st color component.
      for (x = 0; x < s.cx; x++)
        new_brush[x + y * s.cx] = pixels[(x + y * bi->bmiHeader.biWidth) * 4];
    }
  }
  else
  {
    for (y = 0; y < s.cy; y++)
    {
      for (x = 0; x < s.cx; x++)
      {
        byte v = pixels[y*bi->bmiHeader.biWidth+x];
        new_brush[y*s.cx+x] = (v != 0) ? Fore_color : Back_color;
      }
    }
    memcpy(palette, Main.palette, sizeof(T_Palette));
  }
  *width = s.cx;
  *height = s.cy;

  DeleteObject(bm);
  ReleaseDC(NULL, dc);
  DeleteObject(font);
  return new_brush;
}
#endif

byte *Render_text_SFont(const char *str, int font_number, int *width, int *height, T_Palette palette)
{
  SFont_Font *font;
  T_GFX2_Surface * text_surface;
  T_GFX2_Surface *font_surface;
  byte * new_brush = NULL;

  // Chargement de la fonte
  font_surface = Load_surface(Font_name(font_number), NULL, NULL);
  if (!font_surface)
  {
    Verbose_message("Warning", "Error loading font");
    // TODO this leaves a non-erased cursor when the window closes.
    return NULL;
  }
  font=SFont_InitFont(font_surface);
  if (!font)
  {
    GFX2_Log(GFX2_ERROR, "Font init failed : %s\n", Font_name(font_number));
    Free_GFX2_Surface(font_surface);
    return NULL;
  }

  // Calcul des dimensions
  *height = SFont_TextHeight(font, str);
  *width = SFont_TextWidth(font, str);

  text_surface = New_GFX2_Surface(*width, *height);
  if (text_surface == NULL)
  {
    GFX2_Log(GFX2_WARNING, "Failed to allocate text surface\n");
    SFont_FreeFont(font);
    return NULL;
  }
  // Fill with transparent color
  memset(text_surface->pixels, font->Transparent, *width * *height);
  // Rendu du texte
  SFont_Write(text_surface, font, 0, 0, str);

  // Copy palette
  memcpy(palette, font_surface->palette, sizeof(T_Palette));

  new_brush = text_surface->pixels; // "Steal" pixels from surface

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
    
    Remap_general_lowlevel(colmap, new_brush, new_brush, text_surface->w, text_surface->h, text_surface->w);
    
  }

  SFont_FreeFont(font);
  free(text_surface); // Do not call Free_GFX2_Surface() because pixels was stolen

  return new_brush;
}

// Crée une brosse à partir des paramètres de texte demandés.
// Si cela réussit, la fonction place les dimensions dans width et height, 
// et retourne l'adresse du bloc d'octets.
byte *Render_text(const char *str, int font_number, int size, int antialias, int bold, int italic, int *width, int *height, T_Palette palette)
{
  T_Font *font = font_list_start;
  int index=font_number;
  #if defined(NOTTF) && !defined(WIN32)
    (void) size; // unused
    (void) antialias; // unused
    (void) bold; // unused
    (void) italic; // unused
  #endif
  
  // Verification type de la fonte
  if (font_number < 0 || font == NULL)
    return NULL;
    
  while (index--)
  {
    font = font->Next;
    if (font == NULL)
      return NULL;
  }
  if (font->Is_truetype)
  {
  #if !defined(NOTTF)
    return Render_text_TTF(str, font_number, size, antialias, bold, italic, width, height, palette);
  #elif defined(WIN32)
    return Render_text_Win32(str, font_number, size, antialias, bold, italic, width, height, palette);
  #else
    return NULL;
  #endif
  }
  else
  {
    return Render_text_SFont(str, font_number, width, height, palette);
  }
}
