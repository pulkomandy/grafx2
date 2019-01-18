/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2008 Yves Rizoud
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "screen.h"
#include "gfx2surface.h"
#include "loadsave.h"
#include "io.h"
#include "gfx2log.h"

Display * X11_display = NULL;
Window X11_window = 0;
static XImage * X11_image = NULL;
static XTextProperty windowName;
static GC X11_gc = 0;
static T_GFX2_Surface * screen = NULL;
static T_GFX2_Surface * icon = NULL;

void GFX2_Set_mode(int *width, int *height, int fullscreen)
{
  int s;
  int depth;
  unsigned long white, black;
  char * winName[] = { "GrafX2" };
  Visual * visual;
  (void)fullscreen;

  if (X11_display == NULL)
    X11_display = XOpenDisplay(NULL);// NULL is equivalent to getenv("DISPLAY")
  if (X11_display == NULL)
  {
    GFX2_Log(GFX2_ERROR, "X11: cannot open display\n");
    exit(1);
  }
  s = DefaultScreen(X11_display);
  black = BlackPixel(X11_display, s);
  white = WhitePixel(X11_display, s);
  visual = DefaultVisual(X11_display, s);

  {
    int i;
    int count = 0;
    int * depths = XListDepths(X11_display, s, &count);
    GFX2_Log(GFX2_DEBUG, "DefaultDepth = %d, DisplayPlanes = %d\n", DefaultDepth(X11_display, s), DisplayPlanes(X11_display, s));
    if (depths != NULL)
    {
      for (i = 0; i < count; i++)
        GFX2_Log(GFX2_DEBUG, " %d", depths[i]);
      GFX2_Log(GFX2_DEBUG, "\n");
      XFree(depths);
    }
  }
  depth = DisplayPlanes(X11_display, s);

  if (X11_window == 0)
  {
    static const char blank_data[1] = { 0 };
    Pixmap blank;
    Cursor cursor;
    XColor dummy;
    Atom wmDelete;
    XWMHints* hints;

    X11_window = XCreateSimpleWindow(X11_display, RootWindow(X11_display, s),
                                     0, 0, *width, *height, 0, white, black);

    // create blank 1x1 pixmap to make a 1x1 transparent cursor
    blank = XCreateBitmapFromData(X11_display, X11_window, blank_data, 1, 1);
    cursor = XCreatePixmapCursor(X11_display, blank, blank, &dummy, &dummy, 0, 0);
    //cursor = XCreateFontCursor(X11_display, 130 /*XC_tcross*/);
    XDefineCursor(X11_display, X11_window, cursor);
    XFreePixmap(X11_display, blank);
    XFreeCursor(X11_display, cursor);

    X11_gc = XCreateGC(X11_display, X11_window, 0, NULL);
    //XSetFunction(X11_display, X11_gc, GXcopy);

    XStringListToTextProperty(winName, 1, &windowName);
    XSetWMName(X11_display, X11_window, &windowName);

    // set icon
    if (icon != NULL)
    {
      XImage * icon_image = XCreateImage(X11_display, visual, depth,
                             ZPixmap, 0, malloc(icon->w * icon->h * 4),
                             icon->w, icon->h, 32, 0/**width * 4*/);
      if (icon_image != NULL)
      {
        char * transp_data;
        int x, y;
        Pixmap icon_mask;
        Pixmap icon_pixmap = XCreatePixmap(X11_display, X11_window, icon->w, icon->h, 24);

        transp_data = calloc(1, ((icon->w + 7) >> 3) * icon->h);
        for (y = 0; y < icon->h; y++)
        {
          for (x = 0; x < icon->w; x++)
          {
            byte v = icon->pixels[x + y * icon->w];
            if (v != 0) // assume 0 is transparent
              transp_data[(x >> 3) + ((icon->w + 7) >> 3) * y] |= (1 << (x & 7));
            XPutPixel(icon_image, x, y,
                      (unsigned)icon->palette[v].R << 16 | (unsigned)icon->palette[v].G << 8 | (unsigned)icon->palette[v].B);
          }
        }
        // Transfer icon_image to the icon_pixmap
        XPutImage(X11_display, icon_pixmap, X11_gc, icon_image,
                  0, 0, 0, 0, icon->w, icon->h);
        icon_mask = XCreateBitmapFromData(X11_display, X11_window, transp_data,
                                          icon->w, icon->h);

        hints = XAllocWMHints();
        if (hints != NULL)
        {
          hints->flags = IconPixmapHint | IconMaskHint;
          hints->icon_pixmap = icon_pixmap;
          hints->icon_mask = icon_mask;
          XSetWMHints(X11_display, X11_window, hints);
          XFree(hints);
        }
        free(transp_data);
      }
      XDestroyImage(icon_image);
    }

    XSelectInput(X11_display, X11_window,
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask
                 | KeyPressMask | KeyReleaseMask | ExposureMask | StructureNotifyMask);

    wmDelete = XInternAtom(X11_display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(X11_display, X11_window, &wmDelete, 1);

    XMapWindow(X11_display, X11_window);
    if (Config.Window_pos_x != 9999 && Config.Window_pos_y != 9999)
      XMoveWindow(X11_display, X11_window, Config.Window_pos_x, Config.Window_pos_y);
  }

  if (screen == NULL)
  {
    screen = New_GFX2_Surface(*width, *height);
    memset(screen->pixels, 0, *width * *height);
  }
  else if (*width > screen->w || *height > screen->h)
  {
    screen->pixels = realloc(screen->pixels, *width * *height);
    screen->w = *width;
    screen->h = *height;
    XDestroyImage(X11_image);
    X11_image = NULL;
  }

  if (X11_image == NULL)
  {
    char * image_pixels = NULL;

    image_pixels = malloc(*height * *width * 4);
    memset(image_pixels, 64, *height * *width * 4);
    X11_image = XCreateImage(X11_display, visual, depth,
                             ZPixmap, 0, image_pixels, *width, *height,
                             32, 0/**width * 4*/);
    if(X11_image == NULL)
    {
      GFX2_Log(GFX2_ERROR, "XCreateImage failed\n");
      exit(1);
    }
  }

  XFlush(X11_display);
}

byte Get_Screen_pixel(int x, int y)
{
  if(screen == NULL) return 0;
  return screen->pixels[x + y * screen->w];
}

void Set_Screen_pixel(int x, int y, byte value)
{
  if(screen == NULL) return;
  screen->pixels[x + y * screen->w] = value;
}

byte* Get_Screen_pixel_ptr(int x, int y)
{
  if(screen == NULL) return NULL;
  return screen->pixels + x + y * screen->w;
}

void Screen_FillRect(int x, int y, int w, int h, byte color)
{
  int i;
  byte * ptr;

  if (x < 0)
  {
    w += x;
    x = 0;
  }
  if (y < 0)
  {
    h += y;
    y = 0;
  }
  if (x > screen->w || y > screen->h)
    return;
  if ((x + w) > screen->w)
    w = screen->w - x;
  if ((y + h) > screen->h)
    h = screen->h - y;
  if (w <= 0 || h <= 0)
    return;
  for (i = 0; i < h; i++)
  {
    ptr = Get_Screen_pixel_ptr(x, y + i);
    memset(ptr, color, w);
  }
}

int SetPalette(const T_Components * colors, int firstcolor, int ncolors)
{
  if (screen == NULL) return 0;
  memcpy(screen->palette + firstcolor, colors, ncolors * sizeof(T_Components));
  // update full screen
  Update_rect(0, 0, screen->w, screen->h);
  return 1;
}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  int line, i;
  if (screen == NULL || X11_image == NULL) return;
  if (x == 0 && y == 0 && width == 0 && height == 0)
  {
    width = screen->w;
    height = screen->h;
  }
  x *= Pixel_width;
  width *= Pixel_width;
  y *= Pixel_height;
  height *= Pixel_height;
  //GFX2_Log(GFX2_DEBUG, "Update_rect(%d %d %d %d) %d %d\n", x, y, width, height, screen->w, screen->h);
  if (y >= screen->h || x >= screen->w) return;
  if (y + height > screen->h)
    height = screen->h - y;
  if (x + width > screen->w)
    width = screen->w - x;
  for (line = y; line < y + height; line++)
  {
#if 1
    const byte * src = Get_Screen_pixel_ptr(x, line);
    byte * dest = (byte *)X11_image->data + line * X11_image->bytes_per_line + x * 4;
    i = width;
    do
    {
      dest[0] = screen->palette[*src].B;
      dest[1] = screen->palette[*src].G;
      dest[2] = screen->palette[*src].R;
      dest[3] = 0;
      src++;
      dest += 4;
    }
    while(--i > 0);
#else
    for (i = 0; i < width; i++)
    {
      byte v = Get_Screen_pixel(x + i, line);
      XPutPixel(X11_image, x + i, line,
                (unsigned)screen->palette[v].R << 16 | (unsigned)screen->palette[v].G << 8 | (unsigned)screen->palette[v].B);
    }
#endif
  }
  XPutImage(X11_display, X11_window, X11_gc, X11_image,
            x, y, x, y, width, height);
  //XPutImage(X11_display, X11_window, X11_gc, X11_image,
  //          0, 0, 0, 0, X11_image->width, X11_image->height);
  //XSync(X11_display, False);
}

void Flush_update(void)
{
  if (X11_display != NULL)
    XFlush(X11_display);
}

void Update_status_line(short char_pos, short width)
{
  Update_rect((18+char_pos*8)*Menu_factor_X, Menu_status_Y,
              width*8*Menu_factor_X, 8*Menu_factor_Y);
}

void Clear_border(byte color)
{
(void)color;//TODO
}

volatile int Allow_colorcycling = 0;

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
  Atom version = flag ? 5 : 0;

  XChangeProperty(X11_display, X11_window, XInternAtom(X11_display, "XdndAware", False), XA_ATOM, 32, PropModeReplace, (unsigned char *)&version, 1);
}

void Define_icon(void)
{
  char * icon_path;

  icon_path = Filepath_append_to_dir(Data_directory, "gfx2.png"); // 48x48
  icon = Load_surface(icon_path, NULL);
  free(icon_path);
}

void Set_mouse_position(void)
{
  XWarpPointer(X11_display, None, X11_window,
               0, 0, 0, 0,
               Mouse_X * Pixel_width, Mouse_Y * Pixel_height);
}
