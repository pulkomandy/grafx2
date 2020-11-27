/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
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

///@file pasteboard.m
/// Support for Mac OS X PasteBoard
///

#import <AppKit/AppKit.h>

const void * get_tiff_paste_board(unsigned long * size)
{
  NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
  NSLog(@"types in pasteboard : %@", [pasteboard types]);
#if defined(MAC_OS_X_VERSION_10_14) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
  NSData *data = [pasteboard dataForType:NSPasteboardTypeTIFF];
#else
  NSData *data = [pasteboard dataForType:NSTIFFPboardType];
#endif
  if (data == nil)
    return NULL;
  *size = [data length];
  return [data bytes];
}

int set_tiff_paste_board(const void * tiff, unsigned long size)
{
  if (tiff == NULL || size == 0)
    return 0;
  NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
  NSData *data = [[NSData alloc] initWithBytes:tiff length:size];
#if defined(MAC_OS_X_VERSION_10_14) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
  [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeTIFF] owner:nil];
  BOOL b = [pasteboard setData:data forType:NSPasteboardTypeTIFF];
#else
  [pasteboard declareTypes:[NSArray arrayWithObject:NSTIFFPboardType] owner:nil];
  BOOL b = [pasteboard setData:data forType:NSTIFFPboardType];
#endif
  if (!b)
    NSLog(@"Failed to set data in pasteboard");
  [data release];
  return (int)b;
}
