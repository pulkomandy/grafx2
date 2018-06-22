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

extern int user_feedback_required;
extern word Input_new_mouse_X;
extern word Input_new_mouse_Y;
extern byte Input_new_mouse_K;

static HBITMAP Windows_DIB = NULL;
static void *Windows_Screen = NULL;
static int Windows_DIB_width = 0;
static int Windows_DIB_height = 0;
static HWND Win32_hwnd = NULL;

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
  case WM_NCHITTEST:
    // send to test in which part of the windows the coordinates are
    break;
  case WM_CREATE:
    break;
  case WM_SIZE:
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
  case WM_KEYDOWN:  // lParam & 0xffff => repeat count.   (lParam >> 16) & 0x1ff => scancode
    Key = wParam|Get_Key_modifiers();
    user_feedback_required = 1;
    return 0;
  case WM_KEYUP:
    return 0;
  case WM_CHAR:
    Key_ANSI = Key_UNICODE = wParam;
    return 0;
  default:
    {
      char msg[256];
      snprintf(msg, sizeof(msg), "unknown Message : 0x%x", uMsg);
      Warning(msg);
    }
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

	Win32_hwnd = CreateWindow(TEXT("grafx2"), TEXT("grafx2"), style, CW_USEDEFAULT, CW_USEDEFAULT,
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
  static RECT backup_pos = { 0 }; // Last window position used when not in fullscreen

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
      style &= ~WS_POPUP;
      style |= WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;
      SetWindowLong(Win32_hwnd, GWL_STYLE, style);
      if (backup_pos.bottom == 0 || backup_pos.left == 0)
      { // could happen if we started in fullscreen mode
        backup_pos.left = backup_pos.right + *width;
        backup_pos.bottom = backup_pos.top + *height;
        AdjustWindowRect(&backup_pos, style, FALSE);
      }
      SetWindowPos(Win32_hwnd, HWND_TOPMOST,
        backup_pos.left, backup_pos.top,
        backup_pos.right - backup_pos.left, backup_pos.bottom - backup_pos.top,
        SWP_FRAMECHANGED | SWP_NOCOPYBITS);
    }
  }
  if (!fullscreen)
    GetWindowRect(Win32_hwnd, &backup_pos);
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

  for (i = 0; i < h; i++) {
    ptr = Get_Screen_pixel_ptr(x, y + i);
    memset(ptr, color, w);
  }
}

void Update_rect(short x, short y, unsigned short width, unsigned short height)
{
  RECT rect;
  rect.left = x;
  rect.top = y;
  rect.right = x + width;
  rect.bottom = y + height;
  InvalidateRect(Win32_hwnd, &rect, TRUE);
}

void Flush_update(void)
{
}

void Update_status_line(short char_pos, short width)
{
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
  return 1;
}

void Clear_border(byte color)
{
}
  
volatile int Allow_colorcycling = 0;

/// Activates or desactivates file drag-dropping in program window.
void Allow_drag_and_drop(int flag)
{
}
