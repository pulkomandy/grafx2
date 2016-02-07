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
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_endian.h>

#include "global.h"
#include "sdlscreen.h"
#include "errors.h"
#include "misc.h"

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
  #elif defined(__ANDROID__)
    #define UPDATE_METHOD     UPDATE_METHOD_FULL_PAGE
  #else
    #define UPDATE_METHOD     UPDATE_METHOD_CUMULATED
  #endif
#endif

volatile int Allow_colorcycling=1;
static SDL_Window *Window_SDL=NULL;
static SDL_Renderer *Renderer_SDL=NULL;
static SDL_Texture *Texture_SDL=NULL;
static SDL_Texture *Mouse_texture=NULL;

/// Sets the new screen/window dimensions.
void Set_mode_SDL(int *width, int *height, int fullscreen)
{
  static SDL_Cursor* cur = NULL;
  static byte cursorData = 0;

  // (re-)allocate an indexed surface with the size of the screen
  if(Screen_SDL != NULL && (Screen_SDL->w != *width || Screen_SDL->h != *height))
  {
    SDL_FreeSurface(Screen_SDL);
    Screen_SDL = NULL;
  }
  if (Screen_SDL == NULL)
  { 
    Screen_SDL = SDL_CreateRGBSurface(0, *width, *height, 8, 0, 0, 0, 0);
  }
  Screen_pixels=Screen_SDL->pixels;
  
  if (Texture_SDL!=NULL)
  {
    SDL_DestroyTexture(Texture_SDL);
    Texture_SDL=NULL;
  }
  // Renderer and window
  if (Renderer_SDL!=NULL)
  {
    SDL_DestroyRenderer(Renderer_SDL);
    Renderer_SDL=NULL;
  }
  if (Window_SDL!=NULL)
  {
    SDL_DestroyWindow(Window_SDL);
    Window_SDL=NULL;
  }
  
  Window_SDL = SDL_CreateWindow("GrafX2",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    *width, *height * 2,
    (fullscreen?SDL_WINDOW_FULLSCREEN:SDL_WINDOW_RESIZABLE)|SDL_WINDOW_SHOWN|SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_MOUSE_FOCUS);
  Renderer_SDL = SDL_CreateRenderer(Window_SDL, -1, SDL_RENDERER_ACCELERATED);
  //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(Renderer_SDL, *width, *height * 2);
  Texture_SDL = SDL_CreateTexture(Renderer_SDL, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, *width, *height);

  if(Screen_SDL != NULL)
  {
    // Check the mode we got, in case it was different from the one we requested.
  
  }
  else
  {
    DEBUG("Error: Unable to change video mode!",0);
  }

  // Trick borrowed to Barrage (http://www.mail-archive.com/debian-bugs-dist@lists.debian.org/msg737265.html) :
  // Showing the cursor but setting it to fully transparent allows us to get absolute mouse coordinates,
  // this means we can use tablet in fullscreen mode.
  SDL_ShowCursor(1); // Hide the SDL mouse cursor, we use our own

  SDL_FreeCursor(cur);
  cur = SDL_CreateCursor(&cursorData, &cursorData, 1,1,0,0);
  SDL_SetCursor(cur);
  
  {
    SDL_Surface *mouse_icon = NULL;
    mouse_icon = IMG_Load("pointer.png");
    Mouse_texture = SDL_CreateTextureFromSurface(Renderer_SDL, mouse_icon);
    SDL_FreeSurface(mouse_icon);
  }

}

void Update_rectangle(SDL_Surface *surface, int x, int y, int width, int height)
{
  SDL_Rect source_rect;
  //SDL_Rect dest_rect;

  // safety clipping
  if (x==0 && width==0)
    width = surface->w;
  else if (x+width > surface->w)
    width = surface->w - x;
  if (y==0 && height==0)
    height = surface->h;
  else if (y+height > surface->h)
    height = surface->h - y;
  
  source_rect.x = x;
  source_rect.y = y;
  source_rect.w = width;
  source_rect.h = height;
  
  // This refreshes an existing Texture using a source Surface.
  // Usually, recreating the texture using SDL_CreateTextureFromSurface()
  // would be better, but only allows a complete surface (no rectangle)
  // !!! This assumes SDL_PIXELFORMAT_ABGR8888
  //{
  //  Uint32 * pixels;
  //  int pitch;
  //  int line, col;
  //  SDL_LockTexture(Texture_SDL, &source_rect, (void **)(&pixels), &pitch );
  //  for (line = 0; line < source_rect.h; line++)
  //  {
  //    for (col = 0; col < source_rect.w; col++)
  //    {
  //      byte index = Get_SDL_pixel_8(surface, source_rect.x + col, source_rect.y + line);
  //      //memcpy(pixels++, surface->format->palette->colors + index, 4);
  //      //*(pixels++) = *(Uint32 *)(surface->format->palette->colors + index);
  //      *((byte *)pixels) = surface->format->palette->colors[index].r;
  //      *(((byte *)pixels) + 1) = surface->format->palette->colors[index].g;
  //      *(((byte *)pixels) + 2) = surface->format->palette->colors[index].b;
  //      pixels++;
  //    }
  //    pixels += pitch/sizeof(uint32_t) - source_rect.w; 
  //  }
  //  SDL_UnlockTexture(Texture_SDL);
  //}
  
  // Method 2
  {
    byte * pixels;
    int pitch;
    int line;
    static SDL_Surface *RGBcopy = NULL;
    if  (RGBcopy == NULL)
    {
      RGBcopy = SDL_CreateRGBSurface(0,
      surface->w, surface->h,
      32, 0, 0, 0, 0);
    }
    // conversion ARGB
    SDL_BlitSurface(surface, &source_rect, RGBcopy, &source_rect);
    // upload texture
    SDL_LockTexture(Texture_SDL, &source_rect, (void **)(&pixels), &pitch );
    for (line = 0; line < source_rect.h; line++)
    {
       memcpy(pixels + line * pitch, RGBcopy->pixels + source_rect.x * 4 + (source_rect.y+line)* RGBcopy->pitch, source_rect.w * 4 );
    }
    SDL_UnlockTexture(Texture_SDL);
  }  
}

void Render_out_rectangle(int x, int y, int w, int h, int alpha)
{
  SDL_Rect rectangle;
  // Partie grise du milieu
  SDL_SetRenderDrawColor(Renderer_SDL, 128, 128, 128, alpha);
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);

  SDL_SetRenderDrawColor(Renderer_SDL, 255, 255, 255, alpha);
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w - 1*Menu_factor_X;
  rectangle.h = 1*Menu_factor_X;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
  rectangle.w = 1*Menu_factor_X;
  rectangle.h = h - 1*Menu_factor_Y;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);

  SDL_SetRenderDrawColor(Renderer_SDL, 64, 64, 64, alpha);
  rectangle.x = x + 1*Menu_factor_X;
  rectangle.y = y + h - 1*Menu_factor_Y;
  rectangle.w = w - 2*Menu_factor_X;
  rectangle.h = 1*Menu_factor_X;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
  rectangle.x = x + w -1*Menu_factor_X;
  rectangle.y = y + Menu_factor_Y;
  rectangle.w = 1*Menu_factor_X;
  rectangle.h = h - 1*Menu_factor_Y;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
}

void Render_separator(int x, int alpha)
{
  SDL_Rect rectangle;
  int height = (Windows_open == 0) ? Menu_Y : Menu_Y_before_window;
  //SDL_SetRenderDrawBlendMode(Renderer_SDL, SDL_BLENDMODE_BLEND);
  Render_out_rectangle(x+Menu_factor_X, 0, (SEPARATOR_WIDTH-2)*Menu_factor_X, height, alpha);
  
  SDL_SetRenderDrawColor(Renderer_SDL, 0, 0, 0, alpha);
  rectangle.x = x;
  rectangle.y = 0;
  rectangle.w = Menu_factor_X;
  rectangle.h = height;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
  rectangle.x = x + Menu_factor_X*(SEPARATOR_WIDTH-1);
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
}

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
  SDL_Rect r;

#if (UPDATE_METHOD == UPDATE_METHOD_FULL_PAGE)
  // Do a full screen update
  if (update_is_required)
  {
    Update_rectangle(Screen_SDL, 0, 0, 0, 0);
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
    Update_rectangle(Screen_SDL, Min_X, Min_Y, Min(Screen_width-Min_X, Max_X-Min_X), Min(Screen_height-Min_Y, Max_Y-Min_Y));

    Min_X=Min_Y=10000;
    Max_X=Max_Y=0;
  }
  if (Status_line_dirty_end)
  {
    Update_rectangle(Screen_SDL, (18+(Status_line_dirty_begin*8))*Menu_factor_X,Menu_status_Y,(Status_line_dirty_end-Status_line_dirty_begin)*8*Menu_factor_X,8*Menu_factor_Y);
  }
  Status_line_dirty_begin=25;
  Status_line_dirty_end=0;
  
    #endif
  
  SDL_SetRenderTarget(Renderer_SDL, NULL);
  // Clear - mostly for troubleshooting
  {
    static byte c=255;
    //c = c ^ 32;
    SDL_SetRenderDrawColor(Renderer_SDL, 0, 0, c, 255);
    SDL_RenderClear(Renderer_SDL);
  }

  // Copy the fullscreen(old) at the bottom
  r.x = 0;
  r.y = Screen_SDL->h;
  r.w = Screen_SDL->w;
  r.h = Screen_SDL->h;
  SDL_RenderCopy(Renderer_SDL, Texture_SDL, NULL, &r);
  // Version with CreateTextureFromSurface
  //SDL_Texture *temp_tx = SDL_CreateTextureFromSurface(Renderer_SDL, surface);
  //dest_rect.x = 0;
  //dest_rect.y = surface->h;
  //dest_rect.w = surface->w;
  //dest_rect.h = surface->h;
  //SDL_RenderCopy(Renderer_SDL, temp_tx, NULL, &dest_rect);
  //SDL_DestroyTexture(temp_tx);

  // Separator
  if (Main_magnifier_mode)
  {
    Render_separator(Main_separator_position, SDL_ALPHA_OPAQUE);
  }

  // While dragging separator - not needed
  //if (Cursor_shape==CURSOR_SHAPE_HORIZONTAL)
  //{
  //  Render_separator(Mouse_X, 64);
  //}

  // Stack of windows
  // -------------------------------
  {
    int i;
    for (i=0; i < Windows_open; i++)
    {
      r.x = Window_stack[i].Pos_X;
      r.y = Window_stack[i].Pos_Y;
      r.w = Window_stack[i].Width*Menu_factor_X;
      r.h = Window_stack[i].Height*Menu_factor_Y;
      SDL_RenderCopy(Renderer_SDL, Window_stack[i].Texture, NULL, &r);
    } 
  }   
      
  // -------------------------------
  // Mouse cursor
  if (Cursor_shape <= CURSOR_SHAPE_BUCKET)
  {
    byte shape;
    if ( ( (Mouse_Y<Menu_Y)
      && ( (!Main_magnifier_mode) || (Mouse_X<Main_separator_position) || (Mouse_X>=Main_X_zoom) ) )
      || (Windows_open) || (Cursor_shape==CURSOR_SHAPE_HOURGLASS) )
      shape=Cursor_shape;
    else
      shape=CURSOR_SHAPE_ARROW;
    
    r.x = Mouse_X-14;
    r.y = Mouse_Y-15;
    r.w = 29;
    r.h = 31;
    SDL_RenderCopy(Renderer_SDL, Gfx->Mouse_cursor[shape], NULL, &r);
  }
  // Flip display
  SDL_RenderPresent(Renderer_SDL);  

}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  #if (UPDATE_METHOD == UPDATE_METHOD_MULTI_RECTANGLE)
    Update_rectangle(Screen_SDL, x, y, width, height);
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
  update_is_required=1;
  #endif

}

void Update_status_line(short char_pos, short width)
{
  #if (UPDATE_METHOD == UPDATE_METHOD_MULTI_RECTANGLE)
  Update_rectangle(Screen_SDL, (18+char_pos*8)*Menu_factor_X,Menu_status_Y,width*8*Menu_factor_X,8*Menu_factor_Y);
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
  color.r = Main_palette[index].R;
  color.g = Main_palette[index].G;
  color.b = Main_palette[index].B;
  color.a = 255;
  return color;
}

/// Reads a pixel in a 8-bit SDL surface.
byte Get_SDL_pixel_8(SDL_Surface *bmp, int x, int y)
{
  return ((byte *)(bmp->pixels))[(y*bmp->pitch+x)];
}

/// Writes a pixel in a 8-bit SDL surface.
void Set_SDL_pixel_8(SDL_Surface *bmp, int x, int y, byte color)
{
  ((byte *)(bmp->pixels))[(y*bmp->pitch+x)]=color;
}


/// Reads a pixel in a multi-byte SDL surface.
dword Get_SDL_pixel_hicolor(SDL_Surface *bmp, int x, int y)
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

void Clear_border(byte color)
{
  int width;
  int height;
  
  // This function can be called before the graphics mode is set.
  // Nothing to do then.
  if (!Screen_SDL)
    return;
  
  width = Screen_SDL->w - Screen_width;
  height = Screen_SDL->h - Screen_height;
  if (width)
  {
    SDL_Rect r;
    r.x=Screen_SDL->w - width;
    r.y=0;
    r.h=Screen_SDL->h;
    r.w=width;
    SDL_FillRect(Screen_SDL,&r,color);
    Update_rectangle(Screen_SDL, r.x, r.y, r.w, r.h);
  }
  if (height)
  {
    SDL_Rect r;
    r.x=0;
    r.y=Screen_SDL->h - height;
    r.h=height;
    r.w=Screen_SDL->w - height;
    SDL_FillRect(Screen_SDL,&r,color);
    Update_rectangle(Screen_SDL, r.x, r.y, r.w, r.h);
  }  
}

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
  // Inform the system that we accept drag-n-drop events or not
  SDL_EventState (SDL_DROPFILE,flag?SDL_ENABLE:SDL_DISABLE );
}

void Set_mouse_position(void)
{
    SDL_WarpMouseInWindow(Window_SDL, Mouse_X, Mouse_Y);
}

void Set_surface_palette(const SDL_Surface *surface, const SDL_Color *palette)
{
  int i=0;
  for (i=0; i<256; i++)
  {
    surface->format->palette->colors[i].r = palette[i].r;
    surface->format->palette->colors[i].g = palette[i].g;
    surface->format->palette->colors[i].b = palette[i].b;
  }
  //Update_rect(0, 0, Screen_SDL->w, Screen_SDL->h);
}

SDL_Texture * Create_texture(SDL_Surface *source, int x, int y, int w, int h)
{
  SDL_Surface * sub_surface = NULL;
  SDL_Texture * texture = NULL;
  Uint32 transparent;
  
  sub_surface = SDL_CreateRGBSurfaceFrom((byte *)(source->pixels) + x + y*source->pitch, w, h, 8, source->pitch, 0, 0, 0, 0);
  SDL_SetPaletteColors(sub_surface->format->palette, source->format->palette->colors, 0, 256);
  if (!SDL_GetColorKey(source, &transparent))
      SDL_SetColorKey(sub_surface, SDL_TRUE, transparent);
  
  texture = SDL_CreateTextureFromSurface(Renderer_SDL, sub_surface);
  
  SDL_FreeSurface(sub_surface);
  return texture;  
}

SDL_Texture * Create_rendering_texture(int width, int height)
{
  SDL_Texture * texture = SDL_CreateTexture(Renderer_SDL, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, width*Menu_factor_X, height*Menu_factor_Y);
  SDL_SetRenderTarget(Renderer_SDL, texture);
  SDL_SetRenderDrawColor(Renderer_SDL, 64*Windows_open, 64*Windows_open, 64*Windows_open, 255);
  SDL_RenderFillRect(Renderer_SDL, NULL);
  
  return texture;
}
 
void Rectangle_on_texture(SDL_Texture *texture, int x, int y, int w, int h, int r, int g, int b)
{
  SDL_Rect rectangle;
  SDL_SetRenderTarget(Renderer_SDL, texture);
  SDL_SetRenderDrawColor(Renderer_SDL, r, g, b, 255);
  rectangle.x = x;
  rectangle.y = y;
  rectangle.w = w;
  rectangle.h = h;
  SDL_RenderFillRect(Renderer_SDL, &rectangle);
}
