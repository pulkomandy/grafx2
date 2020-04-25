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
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER < 1900
	#define snprintf _snprintf
#endif
#include "gfx2mem.h"
#include "gfx2log.h"
#include "screen.h"
#include "errors.h"
#include "windows.h"
#include "input.h"
#include "keyboard.h"
#include "unicode.h"

extern int Handle_special_key_press(void);
extern int Release_control(int key_code, int modifier);

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

void * GFX2_Get_Window_Handle()
{
  return Win32_hwnd;
}

/// Blit our "framebuffer" bitmap to the Window.
static void Win32_Repaint(HWND hwnd)
{
  PAINTSTRUCT ps;
  HDC dc;
  HDC dc2;
  HBITMAP old_bmp;
  RECT rect;

  if (!GetUpdateRect(hwnd, &rect, FALSE)) return;
  //GFX2_Log(GFX2_DEBUG, "Repaint rect : (%d,%d)-(%d,%d)\n", rect.left, rect.top, rect.right, rect.bottom);
  dc = BeginPaint(hwnd, &ps);
  dc2 = CreateCompatibleDC(dc);
  old_bmp = (HBITMAP)SelectObject(dc2, Windows_DIB);
  if (!BitBlt(dc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
              dc2, rect.left, rect.top,
              SRCCOPY))
    GFX2_Log(GFX2_INFO, "BitBlt(dc, %d, %d, %d, %d, dc2, %d, %d, SRCCOPY) FAILED\n",
             rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
             rect.left, rect.top);
  SelectObject(dc2, old_bmp);
	DeleteDC(dc2);
	EndPaint(hwnd, &ps);
}

/// WindowProc callback function
static LRESULT CALLBACK Win32_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_MOVE:   // Gives the client area coordinates
    GFX2_Log(GFX2_DEBUG, "WM_MOVE : (%hd,%hd)\n", (short)LOWORD(lParam), (short)HIWORD(lParam));
    return 0;
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
      if (!Win32_Is_Fullscreen && !(pos->flags & SWP_NOMOVE) && !(pos->flags & SWP_NOACTIVATE))
      {
        // Windows NT "Minimizes" windows by sending them to (-32000,-32000)
        if (!(pos->x == -32000 && pos->y == -32000))
        {
          Config.Window_pos_x = pos->x;
          Config.Window_pos_y = pos->y;
        }
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
    GFX2_Log(GFX2_DEBUG, "WM_ACTIVATE : activated=%d minimized=%d Handle %08x\n", (int)LOWORD(wParam), (int)HIWORD(wParam), lParam);
    break;
  case WM_ACTIVATEAPP:
    GFX2_Log(GFX2_DEBUG, "WM_ACTIVATEAPP : activate=%d thread %08x\n", (int)wParam, lParam);
    break;
  case WM_SHOWWINDOW: // window is about to be hidden or shown
    GFX2_Log(GFX2_DEBUG, "WM_SHOWWINDOW : show=%d status=%d\n", (int)wParam, (int)lParam);
    break;
  case WM_SETFOCUS:   // We gained keyboard focus
    break;
  case WM_KILLFOCUS:  // We lost keyboard focus
    break;
  case WM_SIZE:
    GFX2_Log(GFX2_DEBUG, "WM_SIZE : %dx%d type=%d\n", LOWORD(lParam), HIWORD(lParam), wParam);
    if (wParam == SIZE_MINIMIZED)
      Window_state = GFX2_WINDOW_MINIMIZED;
    else
    {
      Resize_width = LOWORD(lParam);
      Resize_height = HIWORD(lParam);
      if (wParam == SIZE_MAXIMIZED)
        Window_state = GFX2_WINDOW_MAXIMIZED;
      else if (wParam == SIZE_RESTORED)
        Window_state = GFX2_WINDOW_STANDARD;
    }
    return 0;
  case WM_CLOSE:
    Quit_is_required = 1;
    user_feedback_required = 1;
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
    {
      int x, y;
      x = GET_X_LPARAM(lParam) / Pixel_width;
      y = GET_Y_LPARAM(lParam) / Pixel_height;
      user_feedback_required = Move_cursor_with_constraints(x, y);
    }
    return 0;
  case WM_LBUTTONDOWN:
    SetCapture(hwnd); // capture mouse when the button is pressed
    Input_new_mouse_K |= 1;
    Handle_mouse_btn_change();
    user_feedback_required = 1;
    return 0;
  case WM_LBUTTONUP:
    ReleaseCapture(); // Release mouse when the button is released
    Input_new_mouse_K &= ~1;
    Handle_mouse_btn_change();
    user_feedback_required = 1;
    return 0;
// WM_LBUTTONDBLCLK
  case WM_RBUTTONDOWN:
    SetCapture(hwnd); // capture mouse when the button is pressed
    Input_new_mouse_K |= 2;
    Handle_mouse_btn_change();
    user_feedback_required = 1;
    return 0;
  case WM_RBUTTONUP:
    ReleaseCapture(); // Release mouse when the button is released
    Input_new_mouse_K &= ~2;
    Handle_mouse_btn_change();
    user_feedback_required = 1;
    return 0;
// WM_RBUTTONDBLCLK
  case WM_MBUTTONDOWN:
    Key = KEY_MOUSEMIDDLE|Get_Key_modifiers();
    user_feedback_required = 1;
    return 0;
  case WM_MBUTTONUP:
    return 0;
// WM_MBUTTONDBLCLK
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
#if (_WIN32_WINNT >= 0x0600)
  case WM_MOUSEHWHEEL:
    {
      short delta = HIWORD(wParam);
      if (delta > 0)
        Key = KEY_MOUSEWHEELRIGHT | Get_Key_modifiers();
      else
        Key = KEY_MOUSEWHEELLEFT | Get_Key_modifiers();
    }
    user_feedback_required = 1;
    return 0;
#endif
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
      Handle_special_key_press();
      user_feedback_required = 1;
    }
    return 0;
  case WM_SYSKEYUP:
  case WM_KEYUP:
    {
      int mod = 0;
      switch (wParam)
      {
      case VK_SHIFT:
        mod = GFX2_MOD_SHIFT;
        break;
      case VK_CONTROL:
        mod = GFX2_MOD_CTRL;
        break;
      case VK_MENU: // ALT
        mod = GFX2_MOD_ALT;
        break;
      case VK_LWIN:
      case VK_RWIN:
        mod = GFX2_MOD_META;
      }
      Release_control(wParam, mod);
    }
    return 0;
  case WM_SYSCHAR:  // Character key when ALT key is down
    GFX2_Log(GFX2_DEBUG, "WM_SYSCHAR : '%c' (0x%02x) lParam=%08lx\n", wParam, wParam, lParam);
    return 0;
  case WM_CHAR:
    Key_ANSI = Key_UNICODE = wParam;
    return 0;
  case WM_SYSDEADCHAR:
    GFX2_Log(GFX2_DEBUG, "WM_SYSDEADCHAR : '%c' (0x%02x) lParam=%08lx\n", wParam, wParam, lParam);
    return 0;
  case WM_DEADCHAR:
    GFX2_Log(GFX2_DEBUG, "WM_DEADCHAR : '%c' (0x%02x) lParam=%08lx\n", wParam, wParam, lParam);
    return 0;
  case WM_DROPFILES:
    {
      UINT file_count;
      HDROP hDrop = (HDROP)wParam;

      file_count = DragQueryFileW(hDrop, (UINT)-1, NULL, 0);
      if (file_count == 0)
        GFX2_Log(GFX2_WARNING, "WM_DROPFILES but 0 files\n");
      else
      {
        UINT longLen;

        GFX2_Log(GFX2_DEBUG, "WM_DROPFILES %u files\n", file_count);
        longLen = DragQueryFileW(hDrop, 0, NULL, 0);
        if (longLen == 0)
          GFX2_Log(GFX2_ERROR, "DragQueryFileW(%x, 0, NULL, 0) failed\n", hDrop);
        else
        {
          Drop_file_name_unicode = GFX2_malloc(++longLen * sizeof(word)); // increment for NULL terminator
          if (Drop_file_name_unicode != NULL)
          {
            if (DragQueryFileW(hDrop, 0, Drop_file_name_unicode, longLen) == 0)
              GFX2_Log(GFX2_ERROR, "DragQueryFileW(%x, 0, %p, %u) failed\n", hDrop, Drop_file_name_unicode, longLen);
            else
            {
              // get the short path name
              UINT shortLen = GetShortPathNameW(Drop_file_name_unicode, NULL, 0);
              if (shortLen == 0)
                GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, NULL, 0) failed\n", Drop_file_name_unicode);
              else
              {
                WCHAR * ShortDropFileNameW = GFX2_malloc(shortLen * sizeof(WCHAR));
                if (ShortDropFileNameW != NULL)
                {
                  if (GetShortPathNameW(Drop_file_name_unicode, ShortDropFileNameW, shortLen) == 0)
                    GFX2_Log(GFX2_ERROR, "GetShortPathNameW(%p, %p, %u) failed\n", Drop_file_name_unicode, ShortDropFileNameW, shortLen);
                  else
                  {
                    // convert to ANSI/ASCII
                    Drop_file_name = (char *)GFX2_malloc(shortLen);
                    if (Drop_file_name != NULL)
                    {
                      int i;

                      for (i = 0; ShortDropFileNameW[i] != 0; i++)
                        Drop_file_name[i] = (char)ShortDropFileNameW[i];
                      Drop_file_name[i] = 0;
                    }
                  }
                  free(ShortDropFileNameW);
                }
              }
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
#if(_WIN32_WINNT >= 0x0600)
  case WM_DWMNCRENDERINGCHANGED:
    GFX2_Log(GFX2_DEBUG, "WM_DWMNCRENDERINGCHANGED : enabled=%d\n", (int)wParam);
    break;
#endif /* WINVER >= 0x0600 */
  default:
    GFX2_Log(GFX2_INFO, "Win32_WindowProc() unknown Message : 0x%04x wParam=%08x lParam=%08lx\n", uMsg, wParam, lParam);
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int Init_Win32(HINSTANCE hInstance, HINSTANCE hPrevInstance)
{
  WNDCLASS wc;
  (void)hPrevInstance;

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
		GFX2_Log(GFX2_WARNING, "RegisterClass failed\n");
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
		GFX2_Log(GFX2_WARNING, "CreateDIBSection failed\n");
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
  if (width == 0 && height == 0)
  {
    // update whole window
    InvalidateRect(Win32_hwnd, NULL, FALSE/*TRUE*/);
  }
  else
  {
    RECT rect;
    rect.left = x * Pixel_width;
    rect.top = y * Pixel_height;
    rect.right = (x + width) * Pixel_width;
    rect.bottom = (y + height) * Pixel_height;
    InvalidateRect(Win32_hwnd, &rect, FALSE/*TRUE*/);
  }
}

void Flush_update(void)
{
}

void Update_status_line(short char_pos, short width)
{
  Update_rect((18+char_pos*8)*Menu_factor_X, Menu_status_Y,
              width*8*Menu_factor_X, 8*Menu_factor_Y);
}

int GFX2_SetPalette(const T_Components * colors, int firstcolor, int ncolors)
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
  (void)color;
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

void Set_mouse_position(void)
{
  POINT pt;
  pt.x = Mouse_X * Pixel_width;
  pt.y = Mouse_Y * Pixel_height;
  if (!ClientToScreen(Win32_hwnd, &pt))
    GFX2_Log(GFX2_WARNING, "ClientToScreen(%08x, %p) failed\n", Win32_hwnd, &pt);
  else
  {
    if (!SetCursorPos(pt.x, pt.y))
      GFX2_Log(GFX2_WARNING, "SetCursorPos(%ld, %ld) failed\n", pt.x, pt.y);
  }
}

int GFX2_GetScreenSize(int * width, int * height)
{
  if (width == NULL || height == NULL)
    return 0;

  *width = GetSystemMetrics(SM_CXSCREEN);
  *height = GetSystemMetrics(SM_CYSCREEN);

  return (*width > 0 && *height > 0);
}

int GFX2_MessageBox(const char * text, const char * caption, unsigned int type)
{
  return MessageBoxA(GFX2_Get_Window_Handle(), text, caption, type);
}
