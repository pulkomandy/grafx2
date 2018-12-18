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
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER < 1900
	#define snprintf _snprintf
#endif
#include "screen.h"
#include "errors.h"
#include "windows.h"
#include "input.h"
#include "keyboard.h"
#include "unicode.h"

extern int user_feedback_required;
extern word Input_new_mouse_X;
extern word Input_new_mouse_Y;
extern byte Input_new_mouse_K;

static HBITMAP Windows_DIB = NULL;
static void *Windows_Screen = NULL;
static int Windows_DIB_width = 0;
static int Windows_DIB_height = 0;
static HWND Win32_hwnd = NULL;
static int Win32_Is_Fullscreen = 0;

HWND GFX2_Get_Window_Handle()
{
  return Win32_hwnd;
}

static void Win32_Repaint(HWND hwnd)
{
  PAINTSTRUCT ps;
  HDC dc;
  HDC dc2;
  HBITMAP old_bmp;
  RECT rect;

  if (!GetUpdateRect(hwnd, &rect, FALSE)) return;
  dc = BeginPaint(hwnd, &ps);
  dc2 = CreateCompatibleDC(dc);
  old_bmp = (HBITMAP)SelectObject(dc2, Windows_DIB);
  BitBlt(dc, 0, 0, Windows_DIB_width, Windows_DIB_height,
					   dc2, 0, 0,
					   SRCCOPY);
  SelectObject(dc2, old_bmp);
	DeleteDC(dc2);
	EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK Win32_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_MOVE:   // Gives the client area coordinates
    GFX2_Log(GFX2_DEBUG, "WM_MOVE : (%d,%d)\n", LOWORD(lParam), HIWORD(lParam));
    break;
  case WM_GETMINMAXINFO: // size or position is about to change
    {
      RECT rect;
      LPMINMAXINFO minmaxinfo = (LPMINMAXINFO)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_GETMINMAXINFO : input ptMinTrackSize : %dx%d\n", minmaxinfo->ptMinTrackSize.x, minmaxinfo->ptMinTrackSize.y);
      rect.left = 0;
      rect.top = 0;
      rect.right = 320;
      rect.bottom = 200;
      // add the non client area overhead
      if(AdjustWindowRect(&rect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX, FALSE))
      {
        minmaxinfo->ptMinTrackSize.x = rect.right - rect.left;
        minmaxinfo->ptMinTrackSize.y = rect.bottom - rect.top;
        GFX2_Log(GFX2_DEBUG, "WM_GETMINMAXINFO : return ptMinTrackSize : %dx%d\n", minmaxinfo->ptMinTrackSize.x, minmaxinfo->ptMinTrackSize.y);
      }
    }
    return 0;
  case WM_WINDOWPOSCHANGING: // window size, position, or place in the Z order is about to change
    {
      LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_WINDOWPOSCHANGING : (%d,%d) %dx%d flags=%04x after=%x\n", pos->x, pos->y, pos->cx, pos->cy, pos->flags, pos->hwndInsertAfter);
    }
    break;
  case WM_WINDOWPOSCHANGED:
    {
      LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_WINDOWPOSCHANGED : (%d,%d) %dx%d flags=%04x after=%x\n", pos->x, pos->y, pos->cx, pos->cy, pos->flags, pos->hwndInsertAfter);
      if (!Win32_Is_Fullscreen)
      {
        Config.Window_pos_x = pos->x;
        Config.Window_pos_y = pos->y;
      }
    }
    break;  // call DefWindowProc() in order to receive the WM_SIZE msg
  case WM_NCCREATE:   // Sent before WM_CREATE
    {
      LPCREATESTRUCTA create = (LPCREATESTRUCTA)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_NCCREATE : (%d,%d) %dx%d\n", create->x, create->y, create->cx, create->cy);
    }
    break;
  case WM_NCCALCSIZE: // Sent when the size and position of a window's client area must be calculated
    if(wParam)
    {
      //LPNCCALCSIZE_PARAMS p = (LPNCCALCSIZE_PARAMS)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_NCCALCSIZE(request)\n");
    }
    else
    {
      LPRECT rect = (LPRECT)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_NCCALCSIZE(info): (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
      //return 0;
    }
    break;
  case WM_NCHITTEST: // send to test in which part of the windows the coordinates are
    break;
  case WM_NCPAINT:  // The WM_NCPAINT message is sent to a window when its frame must be painted.
    break;
  case WM_NCACTIVATE: // nonclient area needs to be changed to indicate an active or inactive state.
    break;
  case WM_CREATE:
    break;
  case WM_ACTIVATE:
    break;
  case WM_SETFOCUS:   // We gained keyboard focus
    break;
  case WM_KILLFOCUS:  // We lost keyboard focus
    break;
  case WM_SIZE:
    GFX2_Log(GFX2_DEBUG, "WM_SIZE : %dx%d\n", LOWORD(lParam), HIWORD(lParam));
    Resize_width = LOWORD(lParam);
    Resize_height = HIWORD(lParam);
    break;
  case WM_CLOSE:
    Quit_is_required = 1;
    return 0;
  case WM_ERASEBKGND:
    // the background should be erased
    break;
  case WM_PAINT:
    Win32_Repaint(hwnd);
    return 0;
  case WM_SETCURSOR:
    if (LOWORD(lParam) == HTCLIENT)
    {
      SetCursor(NULL);
      return TRUE;
    }
    break;
  case WM_MOUSELEAVE:
    //ShowCursor(TRUE);
    return 0;
  //case WM_MOUSEENTER:
    //ShowCursor(FALSE);
    //return 0;
  case WM_NCMOUSEMOVE:  // Mouse move in the non client area of the window
    break;
  case WM_MOUSEMOVE:
    //Hide_cursor();
    Input_new_mouse_X = (LOWORD(lParam))/Pixel_width;
    Input_new_mouse_Y = (HIWORD(lParam))/Pixel_height;
    //Display_cursor();
    Move_cursor_with_constraints();
    user_feedback_required = 1;
    return 0;
  case WM_LBUTTONDOWN:
    Input_new_mouse_K |= 1;
    Move_cursor_with_constraints();
    user_feedback_required = 1;
    return 0;
  case WM_LBUTTONUP:
    Input_new_mouse_K &= ~1;
    Move_cursor_with_constraints();
    user_feedback_required = 1;
    return 0;
// WM_LBUTTONDBLCLK
  case WM_RBUTTONDOWN:
    Input_new_mouse_K |= 2;
    Move_cursor_with_constraints();
    user_feedback_required = 1;
    return 0;
  case WM_RBUTTONUP:
    Input_new_mouse_K &= ~2;
    Move_cursor_with_constraints();
    user_feedback_required = 1;
    return 0;
// WM_RBUTTONDBLCLK
  case WM_MBUTTONDOWN:
  //case WM_MBUTTONUP:
    Key = KEY_MOUSEMIDDLE|Get_Key_modifiers();
    user_feedback_required = 1;
    return 0;
  case WM_MOUSEWHEEL:
    {
      short delta = HIWORD(wParam);
      if (delta > 0)
        Key = KEY_MOUSEWHEELUP|Get_Key_modifiers();
      else
        Key = KEY_MOUSEWHEELDOWN|Get_Key_modifiers();
    }
    user_feedback_required = 1;
    return 0;
// WM_MBUTTONDBLCLK
  case WM_SYSKEYDOWN: // Sent when ALT is pressed
  case WM_KEYDOWN:  // lParam & 0xffff => repeat count.   (lParam >> 16) & 0x1ff => scancode
    // lParam & 0x20000000 : context : 0 for WM_KEYDOWN; 1 for WM_SYSKEYDOWN if ALT is pressed
    // lParam & 0x40000000 : previous key state (1 down, 0 up)
    // lParam & 0x80000000 : transition state. 0 for WM_KEYDOWN and WM_SYSKEYDOWN
    GFX2_Log(GFX2_DEBUG, "KEYDOWN wParam=%04x lParam=%08x\n", wParam, lParam);
    switch (wParam)
    {
      // Ignore isolated shift, alt,  control and window keys
      // and numlock
    case VK_SHIFT:
    case VK_CONTROL:
    case VK_MENU: // ALT
    case VK_LWIN:
    case VK_RWIN:
    case VK_NUMLOCK:
    case 0xff:  // ignore 0xff which is invalid but returned with some specific keys
                // such as laptop Fn+something combinaisons
      break;
    default:
      Key = wParam|Get_Key_modifiers();
      user_feedback_required = 1;
    }
    return 0;
  case WM_SYSKEYUP:
  case WM_KEYUP:
    return 0;
  case WM_CHAR:
    Key_ANSI = Key_UNICODE = wParam;
    return 0;
  case WM_DROPFILES:
    {
      int file_count;
      HDROP hDrop = (HDROP)wParam;

      file_count = DragQueryFile(hDrop, (UINT)-1, NULL , 0);
      if (file_count > 0)
      {
        TCHAR LongDropFileName[MAX_PATH];
        TCHAR ShortDropFileName[MAX_PATH];
        if (DragQueryFile(hDrop, 0 , LongDropFileName ,(UINT) MAX_PATH))
        {
          Drop_file_name_unicode = Unicode_strdup((word *)LongDropFileName);
          if (GetShortPathName(LongDropFileName, ShortDropFileName, MAX_PATH))
          {
            Drop_file_name = (char *)malloc(lstrlen(ShortDropFileName) + 1);
            if (Drop_file_name != NULL)
            {
              int i;

              for (i = 0; ShortDropFileName[i] != 0; i++)
                Drop_file_name[i] = (char)ShortDropFileName[i];
              Drop_file_name[i] = 0;
            }
          }
        }
      }
    }
    return 0;
  case WM_STYLECHANGING:  // app can change the styles
  case WM_STYLECHANGED:   // app can not change the styles
    // wParam : GWL_EXSTYLE (extended style) / GWL_STYLE
    // lParam : pointer to STYLESTRUC
    // An application should return zero if it processes this message.
    break;
  case WM_GETICON:  // request icon
    GFX2_Log(GFX2_DEBUG, "WM_GETICON : type=%d dpi=%d\n", (int)wParam, (int)lParam);
    // wParam : ICON_BIG / ICON_SMALL / ICON_SMALL2
    // we can return a custom HICON
    break;
  case WM_SYSCOMMAND: // Window menu command (formerly known as the system or control menu)
    // or maximize button, minimize button, restore button, or close button.
    break;
  case WM_ENTERSIZEMOVE:  // enters the moving or sizing modal loop.
    break;
  case WM_EXITSIZEMOVE:   // the moving or sizing modal loop has exited.
    break;
#if(WINVER >= 0x0400)
  case WM_CAPTURECHANGED: // we lost the mouse capture to (HWND)lParam
    break;
  case WM_SIZING: // user is resizing the window
    {
      LPRECT rect = (LPRECT)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_SIZING : (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
    }
    break;
  case WM_MOVING: // user is moving the window
    {
      LPRECT rect = (LPRECT)lParam;
      GFX2_Log(GFX2_DEBUG, "WM_MOVING : (%d,%d)-(%d,%d)\n", rect->left, rect->top, rect->right, rect->bottom);
    }
    break;
  case WM_IME_SETCONTEXT: // window is activated
    break;
  case WM_IME_NOTIFY: // change of IME function
    break;
#endif
#if(WINVER >= 0x0500)
  case WM_NCMOUSEHOVER: // the cursor hovers over the nonclient area of the window
    break;
  case WM_NCMOUSELEAVE: // the cursor leaves the nonclient area of the window
    break;
#endif /* WINVER >= 0x0500 */
  default:
    GFX2_Log(GFX2_INFO, "Win32_WindowProc() unknown Message : 0x%04x wParam=%08x lParam=%08lx\n", uMsg, wParam, lParam);
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int Init_Win32(HINSTANCE hInstance, HINSTANCE hPrevInstance)
{
  WNDCLASS wc;
	//HINSTANCE hInstance;

	//hInstance = GetModuleHandle(NULL);

	wc.style = 0;
	wc.lpfnWndProc = Win32_WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(100));
	wc.hCursor = NULL;
	wc.hbrBackground = 0;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("grafx2");
	if (!RegisterClass(&wc)) {
		Warning("RegisterClass failed\n");
    Error(ERROR_INIT);
		return 0;
	}
  return 1; // OK
}

static int Video_AllocateDib(int width, int height)
{
	BITMAPINFO *bi;
  BITMAP bm;
	HDC dc;

	if (Windows_DIB != NULL) {
		DeleteObject(Windows_DIB);
		Windows_DIB = NULL;
		Windows_Screen = NULL;
	}
	bi = (BITMAPINFO*)_alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
	memset(bi, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biWidth = width;
	bi->bmiHeader.biHeight = -height;
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biBitCount = 8;
	bi->bmiHeader.biCompression = BI_RGB;

	dc = GetDC(NULL);
	Windows_DIB = CreateDIBSection(dc, bi, DIB_RGB_COLORS, &Windows_Screen, NULL, 0);
	if (Windows_DIB == NULL) {
		Warning("CreateDIBSection failed");
		return -1;
	}
	ReleaseDC(NULL, dc);
  if (GetObject(Windows_DIB, sizeof(bm), &bm) > 0)
  {
    Windows_DIB_width = bm.bmWidthBytes;
    Windows_DIB_height = bm.bmHeight;
  }
  else
  {
    Windows_DIB_width = width;
    Windows_DIB_height = height;
  }
	return 0;
}

static void Win32_CreateWindow(int width, int height, int fullscreen)
{
  DWORD style;
  RECT r;
  int x, y;

  if (fullscreen)
  {
    style = WS_POPUP;
  }
  else
  {
    style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
    /* allow window to be resized */
    style |= WS_THICKFRAME;
    style |= WS_MAXIMIZEBOX;
  }

  r.left = 0;
  r.top = 0;
  r.right = width;
  r.bottom = height;
  AdjustWindowRect(&r, style, FALSE);

  if (Config.Window_pos_x != 9999 && Config.Window_pos_y != 9999)
  {
    x = Config.Window_pos_x;
    y = Config.Window_pos_y;
  }
  else
  {
    x = y = CW_USEDEFAULT;
  }
  Win32_hwnd = CreateWindow(TEXT("grafx2"), TEXT("grafx2"), style, x, y,
                            r.right - r.left, r.bottom - r.top, NULL, NULL,
                            GetModuleHandle(NULL), NULL);
	if (Win32_hwnd == NULL)
  {
		Error(ERROR_INIT);
		return;
	}
	ShowWindow(Win32_hwnd, SW_SHOWNORMAL);
}

void GFX2_Set_mode(int *width, int *height, int fullscreen)
{
  Win32_Is_Fullscreen = fullscreen;
  Video_AllocateDib(*width, *height);
  if (Win32_hwnd == NULL)
    Win32_CreateWindow(*width, *height, fullscreen);
  else
  {
    DWORD style = GetWindowLong(Win32_hwnd, GWL_STYLE);
    if (fullscreen)
    {
      style &= ~WS_OVERLAPPEDWINDOW;
      style |= WS_POPUP;
      SetWindowLong(Win32_hwnd, GWL_STYLE, style);
      SetWindowPos(Win32_hwnd, HWND_TOPMOST, 0, 0, *width, *height, SWP_FRAMECHANGED | SWP_NOCOPYBITS);
    }
    else if ((style & WS_POPUP) != 0)
    {
      RECT r;
      style &= ~WS_POPUP;
      style |= WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;
      SetWindowLong(Win32_hwnd, GWL_STYLE, style);
      if (Config.Window_pos_x != 9999 && Config.Window_pos_y != 9999)
      {
        r.left = Config.Window_pos_x;
        r.top = Config.Window_pos_y;
      }
      else
      {
        r.left = 0;
        r.top = 0;
      }
      r.right = r.left + *width;
      r.bottom = r.top + *height;
      AdjustWindowRect(&r, style, FALSE);
      SetWindowPos(Win32_hwnd, HWND_TOPMOST,
        r.left, r.top,
        r.right - r.left, r.bottom - r.top,
        SWP_FRAMECHANGED | SWP_NOCOPYBITS);
    }
  }
}

byte Get_Screen_pixel(int x, int y)
{
  if (Windows_Screen == NULL) return 0;
  return *((byte *)Windows_Screen + x + y * Windows_DIB_width);
}

void Set_Screen_pixel(int x, int y, byte value)
{
  if (Windows_Screen == NULL) return;
  *((byte *)Windows_Screen + x + y * Windows_DIB_width) = value;
}

byte* Get_Screen_pixel_ptr(int x, int y)
{
  if (Windows_Screen == NULL) return NULL;
  return (byte *)Windows_Screen + x + y * Windows_DIB_width;
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
  if (x > Windows_DIB_width || y > Windows_DIB_height)
    return;
  if ((x + w) > Windows_DIB_width)
    w = Windows_DIB_width - x;
  if ((y + h) > Windows_DIB_height)
    h = Windows_DIB_height - y;
  if (w <= 0 || h <= 0)
    return;
  for (i = 0; i < h; i++) {
    ptr = Get_Screen_pixel_ptr(x, y + i);
    memset(ptr, color, w);
  }
}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  RECT rect;
  rect.left = x * Pixel_width;
  rect.top = y * Pixel_height;
  rect.right = (x + width) * Pixel_width;
  rect.bottom = (y + height) * Pixel_height;
  InvalidateRect(Win32_hwnd, &rect, FALSE/*TRUE*/);
}

void Flush_update(void)
{
}

void Update_status_line(short char_pos, short width)
{
  Update_rect((18+char_pos*8)*Menu_factor_X*Pixel_width, Menu_status_Y*Pixel_height,
              width*8*Menu_factor_X*Pixel_width, 8*Menu_factor_Y*Pixel_height);
}

int SetPalette(const T_Components * colors, int firstcolor, int ncolors)
{
  int i;
  RGBQUAD rgb[256];
	HDC dc;
	HDC dc2;
	HBITMAP old_bmp;

	for (i = 0; i < ncolors; i++) {
    rgb[i].rgbRed      = colors[i].R;
		rgb[i].rgbGreen    = colors[i].G;
		rgb[i].rgbBlue     = colors[i].B;
	}

	dc = GetDC(Win32_hwnd);
	dc2 = CreateCompatibleDC(dc);
	old_bmp = SelectObject(dc2, Windows_DIB);
	SetDIBColorTable(dc2, firstcolor, ncolors, rgb);
	SelectObject(dc2, old_bmp);
	DeleteDC(dc2);
	ReleaseDC(Win32_hwnd, dc);
  InvalidateRect(Win32_hwnd, NULL, FALSE);  // Refresh the whole window
  return 1;
}

void Clear_border(byte color)
{
}
  
volatile int Allow_colorcycling = 0;

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
  DragAcceptFiles(GFX2_Get_Window_Handle(), flag?TRUE:FALSE);
}

void Define_icon(void)
{
  // Do nothing because the icon is set in the window class
  // see Init_Win32()
}
