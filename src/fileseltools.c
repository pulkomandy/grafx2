/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018-2020 Thomas Bernard
    Copyright 2014 Sergii Pylypenko
    Copyright 2011 Pawel Góralski
    Copyright 2009 Franck Charlet
    Copyright 2008 Peter Gordon
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

#include <string.h>
#include <stdlib.h>
#include "struct.h"
#include "global.h"
#include "unicode.h"
#include "io.h"
#include "fileseltools.h"

/**
 * Add an item to the file selector linked list
 * @param list the linked list
 * @param full_name the file name
 * @param short_name the file name truncated to display in the file selector
 * @param type the type of the item : 0 = File, 1 = Directory, 2 = Drive
 * @param icon the icon for the item
 * @return a pointer to the newly added item
 * @return NULL in case of error
 */
T_Fileselector_item * Add_element_to_list(T_Fileselector *list, const char * full_name, const char *short_name, enum FSOBJECT_TYPE type, enum ICON_TYPES icon)
{
  // Working element
  T_Fileselector_item * temp_item;
  size_t full_name_len, short_name_len;

  full_name_len = strlen(full_name) + 1;
  short_name_len = strlen(short_name) + 1;
  // Allocate enough room for one struct + the visible label
  temp_item=(T_Fileselector_item *)malloc(sizeof(T_Fileselector_item)+full_name_len);
  if (temp_item == NULL)  // memory allocation error
    return NULL;
  memset(temp_item, 0, sizeof(T_Fileselector_item));

  if (short_name_len > sizeof(temp_item->Short_name))
    short_name_len = sizeof(temp_item->Short_name) - 1; // without terminating 0
  // Initialize element
  memcpy(temp_item->Short_name,short_name,short_name_len);
  memcpy(temp_item->Full_name,full_name,full_name_len);
  temp_item->Type = type;
  temp_item->Icon = icon;

  // Doubly-linked
  temp_item->Next    =list->First;
  temp_item->Previous=NULL;

  if (list->First!=NULL)
    list->First->Previous=temp_item;
    
  // Put new element at the beginning
  list->First=temp_item;
  return temp_item;
}

word * Format_filename_unicode(const word * fname, word max_length, int type)
{
  static word result[40];
  int         c;
  int         other_cursor;
  int         pos_last_dot;

 // safety
  if (max_length>40)
    max_length=40;

  if (Unicode_char_strcmp(fname,PARENT_DIR)==0)
  {
    Unicode_char_strlcpy(result, "\x11 PARENT DIRECTORY", 40);
    // Append spaces
    for (c=18; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';
  }
  else if (fname[0]=='.' || type==1 || type==2)
  {
    // Files ".something" or drives or directories: Aligned left on (max_length-1) chars max
    // Initialize as all spaces
    for (c=0; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';

    for (c=0;fname[c]!='\0' && c < max_length-1;c++)
      result[c]=fname[c];
    // A special character indicates the filename is truncated
    if (c >= max_length-1)
      result[max_length-2]=(byte)ELLIPSIS_CHARACTER;
  }
  else
  {
    // Initialize as all spaces
    for (c = 0; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';

    result[max_length-5]='.';

    // Look for the last dot in filename
    pos_last_dot = Position_last_dot_unicode(fname);

    // Copy the part before the dot
    for (c=0; c!=pos_last_dot && fname[c]!='\0'; c++)
    {
      if (c > max_length-6)
      {
        result[max_length-6]=(byte)ELLIPSIS_CHARACTER;
        break;
      }
      result[c]=fname[c];
    }

    // Ensuite on recopie la partie qui suit le point (si nécessaire):
    if (pos_last_dot != -1)
    {
      for (c = pos_last_dot+1,other_cursor=max_length-4;fname[c]!='\0' && other_cursor < max_length-1;c++,other_cursor++)
        result[other_cursor]=fname[c];
    }
  }
  return result;
}

char * Format_filename(const char * fname, word max_length, int type)
{
  static char result[40];
  int         c;
  int         other_cursor;
  int         pos_last_dot;
#ifdef ENABLE_FILENAMES_ICONV
  /* convert file name from UTF8 to ANSI */
  char        * converted_fname = NULL;
  {
    char * input = (char *)fname;
    size_t inbytesleft = strlen(fname);
    char * output = converted_fname;
    size_t outbytesleft = inbytesleft;
    converted_fname = malloc(outbytesleft + 1);
    output = converted_fname;
    if(cd != (iconv_t)-1 && (ssize_t)iconv(cd, &input, &inbytesleft, &output, &outbytesleft) >= 0)
    {
      *output = '\0';
      fname = converted_fname;
    }
  }
#endif /* ENABLE_FILENAMES_ICONV */

  // safety
  if (max_length>40)
    max_length=40;
  
  if (strcmp(fname,PARENT_DIR)==0)
  {
    strcpy(result,"\x11 PARENT DIRECTORY");
    // Append spaces
    for (c=18; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';
  }
  else if (fname[0]=='.' || type==1 || type==2)
  {
    // Files ".something" or drives or directories: Aligned left on (max_length-1) chars max
    // Initialize as all spaces
    for (c=0; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';
    
    for (c=0;fname[c]!='\0' && c < max_length-1;c++)
      result[c]=fname[c];
    // A special character indicates the filename is truncated
    if (c >= max_length-1)
      result[max_length-2]=ELLIPSIS_CHARACTER;
  }
  else
  {
    // Initialize as all spaces
    for (c = 0; c<max_length-1; c++)
      result[c]=' ';
    result[c]='\0';
       
    result[max_length-5]='.';
    
    // Look for the last dot in filename
    pos_last_dot = Position_last_dot(fname);

    // Copy the part before the dot
    for (c=0; c!=pos_last_dot && fname[c]!='\0'; c++)
    {
      if (c > max_length-6)
      {
        result[max_length-6]=ELLIPSIS_CHARACTER;
        break;
      }
      result[c]=fname[c];
    }

    // Ensuite on recopie la partie qui suit le point (si nécessaire):
    if (pos_last_dot != -1)
    {
      for (c = pos_last_dot+1,other_cursor=max_length-4;fname[c]!='\0' && other_cursor < max_length-1;c++,other_cursor++)
        result[other_cursor]=fname[c];
    }
  }
#ifdef ENABLE_FILENAMES_ICONV
  free(converted_fname);
#endif
  return result;
}


