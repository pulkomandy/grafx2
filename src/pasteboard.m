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
  NSData *data = [pasteboard dataForType:NSTIFFPboardType];
  if (data == nil)
    return NULL;
  *size = [data length];
  return [data bytes];
}
