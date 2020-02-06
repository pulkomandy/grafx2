/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2014 Sergii Pylypenko
    Copyright 2011 Pawel Góralski
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_endian.h>
#include <SDL_image.h>
#if defined(__WIN32__)
    #include <windows.h>
#endif
// There is no WM on the GP2X...
#ifndef __GP2X__
    #include <SDL_syswm.h>
#endif

#include "global.h"
#include "sdlscreen.h"
#include "errors.h"
#include "misc.h"
#include "gfx2log.h"
#include "io.h"

// Update method that does a large number of small rectangles, aiming
// for a minimum number of total pixels updated.
#define UPDATE_METHOD_MULTI_RECTANGLE 1
// Intermediate update method, does only one update with the smallest
// rectangle that includes all modified pixels.
#define UPDATE_METHOD_CUMULATED       2
// Total screen update, for platforms that impose a Vsync on each SDL update.
#define UPDATE_METHOD_FULL_PAGE       3

// UPDATE_METHOD can be set from makefile, otherwise it's selected here
// depending on the platform :
#ifndef UPDATE_METHOD
  #if defined(__macosx__)
    #define UPDATE_METHOD     UPDATE_METHOD_FULL_PAGE
  #elif defined(__MINT__)
    #define UPDATE_METHOD     UPDATE_METHOD_CUMULATED
  #elif defined(GCWZERO)
    #define UPDATE_METHOD     UPDATE_METHOD_FULL_PAGE
  #elif defined(__ANDROID__)
    #define UPDATE_METHOD     UPDATE_METHOD_FULL_PAGE
  #elif defined(__SWITCH__)
    #define UPDATE_METHOD     UPDATE_METHOD_CUMULATED
  #else
    #define UPDATE_METHOD     UPDATE_METHOD_CUMULATED
  #endif
#endif

static SDL_Surface * Screen_SDL = NULL;
#if defined(USE_SDL2)
static SDL_Window * Window_SDL = NULL;
static SDL_Renderer * Renderer_SDL = NULL;
static SDL_Texture * Texture_SDL = NULL;
static SDL_Surface * icon = NULL;
#endif

volatile int Allow_colorcycling=1;

byte Get_Screen_pixel(int x, int y)
{
  if (y < 0 || x < 0 || y >= Screen_SDL->h || x >= Screen_SDL->w)
  {
    Warning("Get_Screen_pixel() coordinates out of bound");
    return 0;
  }
  return ((byte *)Screen_SDL->pixels)[x + y*(Screen_SDL->pitch)];
}

void Set_Screen_pixel(int x, int y, byte value)
{
  if (y < 0 || x < 0 || y >= Screen_SDL->h || x >= Screen_SDL->w)
  {
    Warning("Set_Screen_pixel() coordinates out of bound");
    return;
  }
  ((byte *)Screen_SDL->pixels)[x + y*(Screen_SDL->pitch)] = value;
}

byte* Get_Screen_pixel_ptr(int x, int y)
{
  if (y < 0 || x < 0 || y >= Screen_SDL->h || x >= Screen_SDL->w)
  {
    GFX2_Log(GFX2_WARNING, "Get_Screen_pixel_ptr(%d, %d): coordinates out of bound\n", x, y);
    return NULL;
  }
  return (byte *)Screen_SDL->pixels + x + y*(Screen_SDL->pitch);
}

void Screen_FillRect(int x, int y, int w, int h, byte color)
{
  SDL_Rect rectangle;
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  SDL_FillRect(Screen_SDL,&rectangle,color);
}

/// Sets the new screen/window dimensions.
void GFX2_Set_mode(int *width, int *height, int fullscreen)
{
  static SDL_Cursor* cur = NULL;
  static byte cursorData = 0;

#if defined(USE_SDL)
#ifdef GCWZERO
  Screen_SDL=SDL_SetVideoMode(*width,*height,8,SDL_HWSURFACE|SDL_TRIPLEBUF|(fullscreen?SDL_FULLSCREEN:0)|SDL_RESIZABLE);
#else
  Screen_SDL=SDL_SetVideoMode(*width,*height,8,(fullscreen?SDL_FULLSCREEN:0)|SDL_RESIZABLE);
#endif
  if(Screen_SDL != NULL)
  {
    // Check the mode we got, in case it was different from the one we requested.
    if (Screen_SDL->w != *width || Screen_SDL->h != *height)
    {
      GFX2_Log(GFX2_WARNING, "Got a different video mode than the requested one! %dx%d => %dx%d\n", *width, *height, Screen_SDL->w, Screen_SDL->h);
      *width = Screen_SDL->w;
      *height = Screen_SDL->h;
    }
    //Screen_pixels=Screen_SDL->pixels;
  }
  else
  {
    GFX2_Log(GFX2_ERROR, "Unable to change video mode!\n");
  }
#else
  // SDL2
  if (Window_SDL == NULL)
  {
    Window_SDL = SDL_CreateWindow("GrafX2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  *width, *height, (fullscreen?SDL_WINDOW_FULLSCREEN:SDL_WINDOW_RESIZABLE));
    SDL_SetWindowIcon(Window_SDL, icon);
    Renderer_SDL = SDL_CreateRenderer(Window_SDL, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  }
  else
  {
    SDL_SetWindowSize(Window_SDL, *width, *height);
    SDL_SetWindowFullscreen(Window_SDL, fullscreen?SDL_WINDOW_FULLSCREEN:0);
  }
  //SDL_GetWindowSize(Window_SDL, width, height);
  if (Texture_SDL != NULL)
    SDL_DestroyTexture(Texture_SDL);
  Texture_SDL = SDL_CreateTexture(Renderer_SDL, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, *width, *height);
  if (Screen_SDL != NULL)
    SDL_FreeSurface(Screen_SDL);
  Screen_SDL = SDL_CreateRGBSurface(0, *width, *height, 8, 0, 0, 0, 0);
#endif

  // Trick borrowed to Barrage (http://www.mail-archive.com/debian-bugs-dist@lists.debian.org/msg737265.html) :
  // Showing the cursor but setting it to fully transparent allows us to get absolute mouse coordinates,
  // this means we can use tablet in fullscreen mode.

  SDL_FreeCursor(cur);
  cur = SDL_CreateCursor(&cursorData, &cursorData, 1,1,0,0);
  if (cur != NULL)
  {
    SDL_SetCursor(cur);
    SDL_ShowCursor(SDL_ENABLE); // show the SDL 1 pixel transparent cursor
  }
  else
  {
    // failed to create the 1 pixel transparent cursor
    SDL_ShowCursor(SDL_DISABLE); // Hide the SDL mouse cursor, we use our own internal one
  }
}

#if defined(USE_SDL2)
static void GFX2_UpdateRect(int x, int y, int width, int height)
{
  byte * pixels;
  int pitch;
  int line;
  static SDL_Surface *RGBcopy = NULL;
  SDL_Rect source_rect;

  source_rect.x = x;
  source_rect.y = y;
  if (width == 0 && height == 0) {
    source_rect.w = Screen_SDL->w;
    source_rect.h = Screen_SDL->h;
  } else {
    source_rect.w = width;
    source_rect.h = height;
  }

  if (RGBcopy != NULL && (Screen_SDL->w > RGBcopy->w || Screen_SDL->h > RGBcopy->h))
  {
    SDL_FreeSurface(RGBcopy);
    RGBcopy = NULL;
  }

  if (RGBcopy == NULL)
  {
    RGBcopy = SDL_CreateRGBSurface(0,
    Screen_SDL->w, Screen_SDL->h,
    32, 0, 0, 0, 0);
  }
  // conversion ARGB
  SDL_BlitSurface(Screen_SDL, &source_rect, RGBcopy, &source_rect);
  // upload texture
  SDL_LockTexture(Texture_SDL, &source_rect, (void **)(&pixels), &pitch );
  for (line = 0; line < source_rect.h; line++)
  {
     memcpy(pixels + line * pitch, (const byte *)RGBcopy->pixels + source_rect.x * 4 + (source_rect.y+line)* RGBcopy->pitch, source_rect.w * 4 );
  }
  SDL_UnlockTexture(Texture_SDL);
  //SDL_RenderCopy(Renderer_SDL, Texture_SDL, &source_rect, &source_rect);
}

void GFX2_UpdateScreen(void)
{
  SDL_RenderCopy(Renderer_SDL, Texture_SDL, NULL, NULL);
  SDL_RenderPresent(Renderer_SDL);
}
#endif

#if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
short Min_X=0;
short Min_Y=0;
short Max_X=10000;
short Max_Y=10000;
short Status_line_dirty_begin=0;
short Status_line_dirty_end=0;
#endif

#if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  int update_is_required=0;
#endif

void Flush_update(void)
{
#if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  // Do a full screen update
  if (update_is_required)
  {
#ifdef GCWZERO
    SDL_Flip(Screen_SDL);
#elif defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, 0, 0, 0, 0);
#else
    GFX2_UpdateRect(0, 0, 0, 0);
#endif
    update_is_required=0;
  }
#endif
  #if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
  if (Min_X>=Max_X || Min_Y>=Max_Y)
  {
    ; // Nothing to do
  }
  else
  {
    if (Min_X<0)
      Min_X=0;
    if (Min_Y<0)
      Min_Y=0;
#if defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, Min_X*Pixel_width, Min_Y*Pixel_height, Min(Screen_width-Min_X, Max_X-Min_X)*Pixel_width, Min(Screen_height-Min_Y, Max_Y-Min_Y)*Pixel_height);
#else
    GFX2_UpdateRect(Min_X*Pixel_width, Min_Y*Pixel_height, Min(Screen_width-Min_X, Max_X-Min_X)*Pixel_width, Min(Screen_height-Min_Y, Max_Y-Min_Y)*Pixel_height);
#endif

    Min_X=Min_Y=10000;
    Max_X=Max_Y=0;
  }
  if (Status_line_dirty_end)
  {
#if defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, (18+(Status_line_dirty_begin*8))*Menu_factor_X*Pixel_width,Menu_status_Y*Pixel_height,(Status_line_dirty_end-Status_line_dirty_begin)*8*Menu_factor_X*Pixel_width,8*Menu_factor_Y*Pixel_height);
#else
    GFX2_UpdateRect((18+(Status_line_dirty_begin*8))*Menu_factor_X*Pixel_width,Menu_status_Y*Pixel_height,(Status_line_dirty_end-Status_line_dirty_begin)*8*Menu_factor_X*Pixel_width,8*Menu_factor_Y*Pixel_height);
#endif
  }
  Status_line_dirty_begin=25;
  Status_line_dirty_end=0;

    #endif

}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  #if (UPDATE_METHOD == UPDATE_METHOD_MULTI_RECTANGLE)
    #if defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, x*Pixel_width, y*Pixel_height, width*Pixel_width, height*Pixel_height);
    #else
    GFX2_UpdateRect(x*Pixel_width, y*Pixel_height, width*Pixel_width, height*Pixel_height);
    #endif
  #endif

  #if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
  if (width==0 || height==0)
  {
    Min_X=Min_Y=0;
    Max_X=Max_Y=10000;
  }
  else
  {
    if (x < Min_X)
      Min_X = x;
    if (y < Min_Y)
      Min_Y = y;
    if (x+width>Max_X)
      Max_X=x+width;
    if (y+height>Max_Y)
      Max_Y=y+height;
  }
  #endif

  #if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  (void)x; (void)y; (void)width; (void)height; // unused
  update_is_required=1;
  #endif

}

void Update_status_line(short char_pos, short width)
{
  #if (UPDATE_METHOD == UPDATE_METHOD_MULTI_RECTANGLE)
  #if defined(USE_SDL)
  SDL_UpdateRect(Screen_SDL, (18+char_pos*8)*Menu_factor_X*Pixel_width,Menu_status_Y*Pixel_height,width*8*Menu_factor_X*Pixel_width,8*Menu_factor_Y*Pixel_height);
  #else
  GFX2_UpdateRect((18+char_pos*8)*Menu_factor_X*Pixel_width,Menu_status_Y*Pixel_height,width*8*Menu_factor_X*Pixel_width,8*Menu_factor_Y*Pixel_height);
  #endif
  #endif

  #if (UPDATE_METHOD == UPDATE_METHOD_CUMULATED)
  // Merge the ranges
  if (Status_line_dirty_end < char_pos+width)
    Status_line_dirty_end=char_pos+width;
  if (Status_line_dirty_begin > char_pos)
    Status_line_dirty_begin=char_pos;
  #endif

  #if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  (void)char_pos; // unused parameter
  (void)width; // unused parameter
  update_is_required=1;
  #endif

}

///
/// Converts a SDL_Surface (indexed colors or RGB) into an array of bytes
/// (indexed colors).
/// If dest is NULL, it's allocated by malloc(). Otherwise, be sure to
/// pass a buffer of the right dimensions.
byte * Surface_to_bytefield(SDL_Surface *source, byte * dest)
{
  byte *src;
  byte *dest_ptr;
  int y;
  int remainder;

  // Support seulement des images 256 couleurs
  if (source->format->BytesPerPixel != 1)
    return NULL;

  if (source->w & 3)
    remainder=4-(source->w&3);
  else
    remainder=0;

  if (dest==NULL)
    dest=(byte *)malloc(source->w*source->h);

  dest_ptr=dest;
  src=(byte *)(source->pixels);
  for(y=0; y < source->h; y++)
  {
    memcpy(dest_ptr, src,source->w);
    dest_ptr += source->w;
    src += source->w + remainder;
  }
  return dest;

}

/// Gets the RGB 24-bit color currently associated with a palette index.
SDL_Color Color_to_SDL_color(byte index)
{
  SDL_Color color;
  color.r = Main.palette[index].R;
  color.g = Main.palette[index].G;
  color.b = Main.palette[index].B;
#if defined(USE_SDL)
  color.unused = 255;
#else
  color.a = 255;
#endif
  return color;
}

/// Reads a pixel in a 8-bit SDL surface.
byte Get_SDL_pixel_8(const SDL_Surface *bmp, int x, int y)
{
  return ((byte *)(bmp->pixels))[(y*bmp->pitch+x)];
}

/// Writes a pixel in a 8-bit SDL surface.
void Set_SDL_pixel_8(SDL_Surface *bmp, int x, int y, byte color)
{
  ((byte *)(bmp->pixels))[(y*bmp->pitch+x)]=color;
}


/// Reads a pixel in a multi-byte SDL surface.
dword Get_SDL_pixel_hicolor(const SDL_Surface *bmp, int x, int y)
{
  byte * ptr;

  switch(bmp->format->BytesPerPixel)
  {
    case 4:
    default:
      return *((dword *)((byte *)bmp->pixels+(y*bmp->pitch+x*4)));
    case 3:
      // Reading a 4-byte number starting at an address that isn't a multiple
      // of 2 (or 4?) is not supported on Caanoo console at least (ARM CPU)
      // So instead, we will read the 3 individual bytes, and re-construct the
      // "dword" expected by SDL.
      ptr = ((byte *)bmp->pixels)+(y*bmp->pitch+x*3);
      #ifdef SDL_LIL_ENDIAN
      // Read ABC, output _CBA : Most Significant Byte is zero.
      return (*ptr) | (*(ptr+1)<<8) | (*(ptr+2)<<16);
      #else
      // Read ABC, output ABC_ : Least Significant Byte is zero.
      return ((*ptr)<<24) | (*(ptr+1)<<16) | (*(ptr+2)<<8);
      #endif
    case 2:
      return *((word *)((byte *)bmp->pixels+(y*bmp->pitch+x*2)));
  }
}

/// Convert a SDL Palette to a grafx2 palette
void Get_SDL_Palette(const SDL_Palette * sdl_palette, T_Palette palette)
{
  int i;

  for (i=0; i<256; i++)
  {
    palette[i].R=sdl_palette->colors[i].r;
    palette[i].G=sdl_palette->colors[i].g;
    palette[i].B=sdl_palette->colors[i].b;
  }

}

int GFX2_SetPalette(const T_Components * colors, int firstcolor, int ncolors)
{
  int i;
  SDL_Color PaletteSDL[256];

  for (i = 0; i < ncolors; i++) {
    PaletteSDL[i].r = colors[i].R;
    PaletteSDL[i].g = colors[i].G;
    PaletteSDL[i].b = colors[i].B;
  }
#if defined(USE_SDL)
  return SDL_SetPalette(Screen_SDL, SDL_PHYSPAL | SDL_LOGPAL, PaletteSDL, firstcolor, ncolors);
#else
  // When using SDL2, we need to force screen update so the
  // 8bit => True color conversion will be performed
  i = SDL_SetPaletteColors(Screen_SDL->format->palette, PaletteSDL, firstcolor, ncolors);
  if (i == 0)
    Update_rect(0, 0, Screen_SDL->w, Screen_SDL->h);
  return i;
#endif
}

void Clear_border(byte color)
{
  int width;
  int height;

  // This function can be called before the graphics mode is set.
  // Nothing to do then.
  if (!Screen_SDL)
    return;

  width = Screen_SDL->w - Screen_width*Pixel_width;
  height = Screen_SDL->h - Screen_height*Pixel_height;
  if (width)
  {
    SDL_Rect r;
    r.x=Screen_SDL->w - width;
    r.y=0;
    r.h=Screen_SDL->h;
    r.w=width;
    SDL_FillRect(Screen_SDL,&r,color);
#if defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, r.x, r.y, r.w, r.h);
#else
    //SDL_RenderPresent(
#endif
  }
  if (height)
  {
    SDL_Rect r;
    r.x=0;
    r.y=Screen_SDL->h - height;
    r.h=height;
    r.w=Screen_SDL->w - height;
    SDL_FillRect(Screen_SDL,&r,color);
#if defined(USE_SDL)
    SDL_UpdateRect(Screen_SDL, r.x, r.y, r.w, r.h);
#else
// TODO
#endif
  }
}

#ifdef WIN32
HWND GFX2_Get_Window_Handle(void)
{
  SDL_SysWMinfo wminfo;

  SDL_VERSION(&wminfo.version);
#if defined(USE_SDL)
  if (SDL_GetWMInfo(&wminfo) <= 0)
    return NULL;
  return wminfo.window;
#else
  if (Window_SDL == NULL)
    return NULL;
  if (!SDL_GetWindowWMInfo(Window_SDL, &wminfo))
    return NULL;
  return wminfo.info.win.window;
#endif
}
#endif

#ifdef SDL_VIDEO_DRIVER_X11
int GFX2_Get_X11_Display_Window(Display * * display, Window * window)
{
  SDL_SysWMinfo wminfo;

  SDL_VERSION(&wminfo.version);
#if defined(USE_SDL)
  // SDL 1.x
  if (SDL_GetWMInfo(&wminfo) <= 0)
    return 0;
  *display = wminfo.info.x11.display;
  *window = wminfo.info.x11.wmwindow;
#else
  // SDL 2.x
  if (Window_SDL == NULL)
    return 0;
  if (!SDL_GetWindowWMInfo(Window_SDL, &wminfo))
    return 0;
  *display = wminfo.info.x11.display;
  *window = wminfo.info.x11.window;
#endif
  return 1;
}
#endif

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
#if defined(USE_SDL2)
  // SDL 2.x
  SDL_EventState(SDL_DROPFILE, flag ? SDL_ENABLE : SDL_DISABLE);
#else
  // SDL 1.x
  // Inform Windows that we accept drag-n-drop events or not
#ifdef __WIN32__
  DragAcceptFiles(GFX2_Get_Window_Handle(), flag?TRUE:FALSE);
  SDL_EventState (SDL_SYSWMEVENT, flag?SDL_ENABLE:SDL_DISABLE);
#elif defined(SDL_VIDEO_DRIVER_X11) && !defined(NO_X11)
  Atom version = flag ? 5 : 0;
  Display * display;
  Window window;

  if (GFX2_Get_X11_Display_Window(&display, &window))
  {
    if (display == NULL)
    {
      if (flag)
        GFX2_Log(GFX2_WARNING, "Drag&Drop with SDL needs the x11 video driver\n");
      return;
    }
    XChangeProperty(display, window,
                    XInternAtom(display, "XdndAware", False),
                    XA_ATOM, 32, PropModeReplace, (unsigned char *)&version, 1);
    SDL_EventState (SDL_SYSWMEVENT, flag?SDL_ENABLE:SDL_DISABLE);
  }
#else
  (void)flag; // unused
#endif
#endif
}

/// Set application icon(s)
void Define_icon(void)
{
#ifdef WIN32
  // Specific code for Win32:
  // Load icon from embedded resource.
  // This will provide both the 16x16 and 32x32 versions.
  do
  {
    HICON hicon;
    HRSRC hresource;
    HINSTANCE hInstance;
    LPVOID lpResIconDir;
    LPVOID lpResIcon16;
    LPVOID lpResIcon32;
    HGLOBAL hMem;
    WORD nID;

    hInstance = (HINSTANCE)GetModuleHandle(NULL);
    if (hInstance==NULL)
      break;

    // Icon is resource #1
    hresource = FindResource(hInstance,
      MAKEINTRESOURCE(100),
      RT_GROUP_ICON);
    if (hresource==NULL)
      break;

    // Load and lock the icon directory.
    hMem = LoadResource(hInstance, hresource);
    if (hMem==NULL)
      break;

    lpResIconDir = LockResource(hMem);
    if (lpResIconDir==NULL)
      break;

    //
    // 16x16
    //

    // Get the identifier of the 16x16 icon
    nID = LookupIconIdFromDirectoryEx((PBYTE) lpResIconDir, TRUE,
        16, 16, LR_DEFAULTCOLOR);
    if (nID==0)
      break;

    // Find the bits for the nID icon.
    hresource = FindResource(hInstance,
        MAKEINTRESOURCE(nID),
        MAKEINTRESOURCE((long)RT_ICON));
    if (hresource==NULL)
      break;

    // Load and lock the icon.
    hMem = LoadResource(hInstance, hresource);
    if (hMem==NULL)
      break;
    lpResIcon16 = LockResource(hMem);
    if (lpResIcon16==NULL)
      break;

    // Create a handle to the icon.
    hicon = CreateIconFromResourceEx((PBYTE) lpResIcon16,
        SizeofResource(hInstance, hresource), TRUE, 0x00030000,
        16, 16, LR_DEFAULTCOLOR);
    if (hicon==NULL)
      break;

    // Set it
		SetClassLongPtr(GFX2_Get_Window_Handle(), GCL_HICONSM, (LONG_PTR)hicon);


    //
    // 32x32
    //

    // Get the identifier of the 32x32 icon
    nID = LookupIconIdFromDirectoryEx((PBYTE) lpResIconDir, TRUE,
        32, 32, LR_DEFAULTCOLOR);
    if (nID==0)
      break;

    // Find the bits for the nID icon.
    hresource = FindResource(hInstance,
        MAKEINTRESOURCE(nID),
        MAKEINTRESOURCE((long)RT_ICON));
    if (hresource==NULL)
      break;

    // Load and lock the icon.
    hMem = LoadResource(hInstance, hresource);
    if (hMem==NULL)
      break;
    lpResIcon32 = LockResource(hMem);
    if (lpResIcon32==NULL)
      break;

    // Create a handle to the icon.
    hicon = CreateIconFromResourceEx((PBYTE) lpResIcon32,
        SizeofResource(hInstance, hresource), TRUE, 0x00030000,
        32, 32, LR_DEFAULTCOLOR);
    if (hicon==NULL)
      break;

    // Set it
		SetClassLongPtr(GFX2_Get_Window_Handle(), GCL_HICON, (LONG_PTR)hicon);


		// Success
		return;
  } while (0);
  // Failure: fall back on normal SDL version:

#endif

  // General version: Load icon from file
  {
#if defined(USE_SDL)
    SDL_Surface * icon;
#endif
    char * icon_path;

    // gfx2.gif : 32x32
    // gfx2.png : 48x48
    icon_path = Filepath_append_to_dir(Data_directory, "gfx2.png");
    icon = IMG_Load(icon_path);
    if (icon == NULL)
      GFX2_Log(GFX2_WARNING, "Failed to load icon %s\n", icon_path);
    else
    {
      Uint32 pink;
      pink = SDL_MapRGB(icon->format, 255, 0, 255);

      if (icon->format->BitsPerPixel == 8)
      {
        // NOTE : disable use of color key because SDL/SDL2
        // gets the transparency information from the .gif and .png
        // files by itself.
        // 8bit image: use color key
#if defined(USE_SDL)
        //SDL_SetColorKey(icon, SDL_SRCCOLORKEY, pink);
        SDL_WM_SetIcon(icon, NULL);
#else
        //SDL_SetColorKey(icon, SDL_TRUE, pink);
#endif
      }
      else
      {
        // 24bit image: need to build a mask on magic pink
        byte *icon_mask;
        int x,y;

        icon_mask = (byte *)malloc(icon->w * icon->h / 8);
        memset(icon_mask, 0, icon->w * icon->h / 8);
        for (y=0; y<icon->h; y++)
          for (x=0; x<icon->w; x++)
            if (Get_SDL_pixel_hicolor(icon, x, y) != pink)
              icon_mask[(y*icon->w+x)/8] |= 0x80 >> (x&7);
#if defined(USE_SDL)
        SDL_WM_SetIcon(icon, icon_mask);
#endif
        free(icon_mask);
        icon_mask = NULL;
      }
#if defined(USE_SDL)
      SDL_FreeSurface(icon);
#endif
    }
    free(icon_path);
  }
}

void Set_mouse_position(void)
{
#if defined(USE_SDL)
  SDL_WarpMouse(Mouse_X*Pixel_width, Mouse_Y*Pixel_height);
#elif defined(USE_SDL2)
  SDL_WarpMouseInWindow(NULL, Mouse_X*Pixel_width, Mouse_Y*Pixel_height);
#endif
}
