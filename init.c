/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Peter Gordon
    Copyright 2008 Yves Rizoud
    Copyright 2009 Franck Charlet
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

// Signal handler: I activate it for the two platforms who certainly
// support them. Feel free to check with others.
#if defined(__WIN32__) || defined(__linux__)
  #define GRAFX2_CATCHES_SIGNALS
#endif
#include <stdio.h>
//#include <fcntl.h>
#include <string.h>

#ifndef __VBCC__
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <SDL_byteorder.h>
#include <SDL_image.h>
#if defined(__WIN32__)
  #include <windows.h> // GetLogicalDrives(), GetDriveType(), DRIVE_*
#endif
#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  #include <proto/exec.h>
  #include <proto/dos.h>
#endif
#ifdef GRAFX2_CATCHES_SIGNALS
  #include <signal.h>
#endif

#include "buttons.h"
#include "const.h"
#include "errors.h"
#include "global.h"
#include "graph.h"
#include "init.h"
#include "io.h"
#include "factory.h"
#include "help.h"
#include "hotkeys.h"
#include "keyboard.h"
#include "loadsave.h" // Image_emergency_backup
#include "misc.h"
#include "mountlist.h" // read_file_system_list
#include "operatio.h"
#include "palette.h"
#include "sdlscreen.h"
#include "setup.h"
#include "struct.h"
#include "transform.h"
#include "windows.h"

char Gui_loading_error_message[512];

// Rechercher la liste et le type des lecteurs de la machine

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
void bstrtostr( BSTR in, STRPTR out, TEXT max );
#endif

// Fonctions de lecture dans la skin de l'interface graphique
byte GUI_seek_down(SDL_Surface *gui, int *start_x, int *start_y, byte neutral_color,char * section)
{
  byte color;
  int y;
  y=*start_y;
  *start_x=0;
  do
  {
    color=Get_SDL_pixel_8(gui,*start_x,y);
    if (color!=neutral_color)
    {
      *start_y=y;
      return 0;
    }
    y++;
  } while (y<gui->h);
  
  sprintf(Gui_loading_error_message, "Error in skin file: Was looking down from %d,%d for a '%s', and reached the end of the image\n",
    *start_x, *start_y, section);
  return 1;
}

byte GUI_seek_right(SDL_Surface *gui, int *start_x, int start_y, byte neutral_color, char * section)
{
  byte color;
  int x;
  x=*start_x;
  
  do
  {
    color=Get_SDL_pixel_8(gui,x,start_y);
    if (color!=neutral_color)
    {
      *start_x=x;
      return 0;
    }
    x++;
  } while (x<gui->w);
  
  sprintf(Gui_loading_error_message, "Error in skin file: Was looking right from %d,%d for a '%s', and reached the edege of the image\n",
    *start_x, start_y, section);
  return 1;
}

byte Read_GUI_block(SDL_Surface *gui, int start_x, int start_y, void *dest, int width, int height, char * section, int type)
{
  // type: 0 = normal GUI element, only 4 colors allowed
  // type: 1 = mouse cursor, 4 colors allowed + transparent
  // type: 2 = brush icon or sieve pattern (only MC_White and MC_Trans)
  // type: 3 = raw bitmap (splash screen)
  
  byte * dest_ptr=dest;
  int x,y;
  byte color;

  // Verification taille
  if (start_y+height>=gui->h || start_x+width>=gui->w)
  {
    sprintf(Gui_loading_error_message, "Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but it doesn't fit the image.\n",
      start_x, start_y, height, width, section);
    return 1;
  }

  for (y=start_y; y<start_y+height; y++)
  {
    for (x=start_x; x<start_x+width; x++)
    {
      color=Get_SDL_pixel_8(gui,x,y);
      if (type==0 && (color != MC_Black && color != MC_Dark && color != MC_Light && color != MC_White))
      {
        sprintf(Gui_loading_error_message, "Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the GUI colors (which were detected as %d,%d,%d,%d.\n",
          start_x, start_y, height, width, section, x, y, color, MC_Black, MC_Dark, MC_Light, MC_White);
        return 1;
      }
      if (type==1 && (color != MC_Black && color != MC_Dark && color != MC_Light && color != MC_White && color != MC_Trans))
      {
        sprintf(Gui_loading_error_message, "Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the mouse colors (which were detected as %d,%d,%d,%d,%d.\n",
          start_x, start_y, height, width, section, x, y, color, MC_Black, MC_Dark, MC_Light, MC_White, MC_Trans);
        return 1;
      }
      if (type==2)
      {
        if (color != MC_White && color != MC_Trans)
        {
          sprintf(Gui_loading_error_message, "Error in skin file: Was looking at %d,%d for a %d*%d object (%s) but at %d,%d a pixel was found with color %d which isn't one of the brush colors (which were detected as %d on %d.\n",
            start_x, start_y, height, width, section, x, y, color, MC_White, MC_Trans);
          return 1;
        }
        // Conversion en 0/1 pour les brosses monochromes internes
        color = (color != MC_Trans);
      }
      *dest_ptr=color;
      dest_ptr++;
    }
  }
  return 0;
}

byte Read_GUI_pattern(SDL_Surface *gui, int start_x, int start_y, word *dest, char * section)
{
  byte buffer[256];
  int x,y;
  
  if (Read_GUI_block(gui, start_x, start_y, buffer, 16, 16, section, 2))
    return 1;

  for (y=0; y<16; y++)
  {
    *dest=0;
    for (x=0; x<16; x++)
    {
      *dest=*dest | buffer[y*16+x]<<x;
    }
    dest++;
  }
  return 0;
}

void Center_GUI_cursor(T_Gui_skin *gfx, byte *cursor_buffer, int cursor_number)
{
  int x,y;
  int start_x, start_y;
  byte found;

  // Locate first non-empty column
  found=0;
  for (start_x=0;start_x<14;start_x++)
  {
    for (y=0;y<29;y++)
    {
      if (cursor_buffer[y*29+start_x]!=MC_Trans)
      {
        found=1;
        break;
      }
    }
    if (found)
      break;
  }
  // Locate first non-empty line
  found=0;
  for (start_y=0;start_y<14;start_y++)
  {
    for (x=0;x<29;x++)
    {
      if (cursor_buffer[start_y*29+x]!=MC_Trans)
      {
        found=1;
        break;
      }
    }
    if (found)
      break;
  }
  gfx->Cursor_offset_X[cursor_number]=14-start_x;
  gfx->Cursor_offset_Y[cursor_number]=14-start_y;

  for (y=0;y<CURSOR_SPRITE_HEIGHT;y++)
    for (x=0;x<CURSOR_SPRITE_WIDTH;x++)
      gfx->Cursor_sprite[cursor_number][y][x]=cursor_buffer[(start_y+y)*29+start_x+x];
}

byte Parse_skin(SDL_Surface * gui, T_Gui_skin *gfx)
{
  int  index;
  int i;
  int cursor_x=0,cursor_y=0;
  byte color;
  byte neutral_color; // color neutre utilisée pour délimiter les éléments GUI
  int char_1=0;  // Indices utilisés pour les 4 "fontes" qui composent les
  int char_2=0;  // grands titres de l'aide. Chaque indice avance dans 
  int char_3=0;  // l'une des fontes dans l'ordre :  1 2
  int char_4=0;  //                                  3 4
  byte mouse_cursor_area[29][29];
  SDL_Palette * SDLPal;
  
  // Default palette
  if (!gui->format || gui->format->BitsPerPixel != 8)
  {
    sprintf(Gui_loading_error_message, "Not a 8-bit image");
    return 1;
  }
  SDLPal=gui->format->palette;
  if (!SDLPal || SDLPal->ncolors!=256)
  {
    sprintf(Gui_loading_error_message, "Not a 256-color palette");
    return 1;
  }
  // Read the default palette
  Get_SDL_Palette(SDLPal, gfx->Default_palette);

  // Carré "noir"
  MC_Black = Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  do
  {
    if (++cursor_x>=gui->w)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
    color=Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  } while(color==MC_Black);
  // Carré "foncé"
  MC_Dark=color;
  do
  {
    if (++cursor_x>=gui->w)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
    color=Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  } while(color==MC_Dark);
  // Carré "clair"
  MC_Light=color;
  do
  {
    if (++cursor_x>gui->w)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
    color=Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  } while(color==MC_Light);
  // Carré "blanc"
  MC_White=color;
  do
  {
    if (++cursor_x>=gui->w)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
    color=Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  } while(color==MC_White);
  // Carré "transparent"
  MC_Trans=color;
  do
  {
    if (++cursor_x>=gui->w)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
    color=Get_SDL_pixel_8(gui,cursor_x,cursor_y);
  } while(color==MC_Trans);
  // Reste : couleur neutre
  neutral_color=color;

  
  cursor_x=0;
  cursor_y=1;
  while ((color=Get_SDL_pixel_8(gui,cursor_x,cursor_y))==MC_Black)
  {
    cursor_y++;
    if (cursor_y>=gui->h)
    {
      sprintf(Gui_loading_error_message, "Error in GUI skin file: should start with 5 consecutive squares for black, dark, light, white, transparent, then a neutral color\n");
      return 1;
    }
  }
  
  // Menu
  if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "menu"))
    return 1;
  if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Menu_block, MENU_WIDTH, MENU_HEIGHT,"menu",0))
    return 1;

  cursor_x += MENU_WIDTH;
  if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "logo"))
	return 1;
  if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->preview, 173, 16, "logo", 0))
	return 1;
  cursor_y+=MENU_HEIGHT;

  // Effets
  for (i=0; i<NB_EFFECTS_SPRITES; i++)
  {
    if (i==0)
    {
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "effect sprite"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "effect sprite"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Effect_sprite[i], MENU_SPRITE_WIDTH, MENU_SPRITE_HEIGHT, "effect sprite",0))
      return 1;
    cursor_x+=MENU_SPRITE_WIDTH;
  }
  cursor_y+=MENU_SPRITE_HEIGHT;
  
  // Curseurs souris
  for (i=0; i<NB_CURSOR_SPRITES; i++)
  {
    if (i==0)
    {
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "mouse cursor"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "mouse cursor"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, mouse_cursor_area, 29, 29, "mouse cursor",1))
      return 1;
    Center_GUI_cursor(gfx, (byte *)mouse_cursor_area,i);
    cursor_x+=29;
  }
  cursor_y+=29;

  // Sprites menu
  for (i=0; i<NB_MENU_SPRITES; i++)
  {
    if (i==0)
    {
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "menu sprite"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "menu sprite"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Menu_sprite[i], MENU_SPRITE_WIDTH, MENU_SPRITE_HEIGHT, "menu sprite",1))
      return 1;
    cursor_x+=MENU_SPRITE_WIDTH;
  }
  cursor_y+=MENU_SPRITE_HEIGHT;
  
  // Icones des Pinceaux
  for (i=0; i<NB_PAINTBRUSH_SPRITES; i++)
  {
    // Rangés par ligne de 12
    if ((i%12)==0)
    {
      if (i!=0)
        cursor_y+=PAINTBRUSH_HEIGHT;
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "brush icon"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "brush icon"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Paintbrush_sprite[i], PAINTBRUSH_WIDTH, PAINTBRUSH_HEIGHT, "brush icon",2))
      return 1;
    cursor_x+=PAINTBRUSH_WIDTH;
  }
  cursor_y+=PAINTBRUSH_HEIGHT;

  // Sprites drive
  for (i=0; i<NB_ICON_SPRITES; i++)
  {
    if (i==0)
    {
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "sprite drive"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "sprite drive"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Icon_sprite[i], ICON_SPRITE_WIDTH, ICON_SPRITE_HEIGHT, "sprite drive",1))
      return 1;
    cursor_x+=ICON_SPRITE_WIDTH;
  }
  cursor_y+=ICON_SPRITE_HEIGHT;

  // Logo splash screen

  if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "logo menu"))
    return 1;
  if (Read_GUI_block(gui, cursor_x, cursor_y, gfx->Logo_grafx2, 231, 56, "logo menu",3))
    return 1;
  cursor_y+=56;
  
  // Trames
  for (i=0; i<NB_PRESET_SIEVE; i++)
  {
    if (i==0)
    {
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "sieve pattern"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "sieve pattern"))
        return 1;
    }
    if (Read_GUI_pattern(gui, cursor_x, cursor_y, gfx->Sieve_pattern[i],"sieve pattern"))
      return 1;
    cursor_x+=16;
  }
  cursor_y+=16;

  // Font help normale
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        cursor_y+=8;
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "help font (norm)"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "help font (norm)"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, &(gfx->Help_font_norm[i][0][0]), 6, 8, "help font (norm)",0))
      return 1;
    cursor_x+=6;
  }
  cursor_y+=8;
  
  // Font help bold
  for (i=0; i<256; i++)
  {
    // Rangés par ligne de 32
    if ((i%32)==0)
    {
      if (i!=0)
        cursor_y+=8;
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "help font (bold)"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "help font (bold)"))
        return 1;
    }
    if (Read_GUI_block(gui, cursor_x, cursor_y, &(gfx->Bold_font[i][0][0]), 6, 8, "help font (bold)",0))
      return 1;
    cursor_x+=6;
  }
  cursor_y+=8;

  // Font help titre
  for (i=0; i<256; i++)
  {
    byte * dest;
    // Rangés par ligne de 64
    if ((i%64)==0)
    {
      if (i!=0)
        cursor_y+=8;
      if (GUI_seek_down(gui, &cursor_x, &cursor_y, neutral_color, "help font (title)"))
        return 1;
    }
    else
    {
      if (GUI_seek_right(gui, &cursor_x, cursor_y, neutral_color, "help font (title)"))
        return 1;
    }
    
    if (i&1)
      if (i&64)
        dest=&(gfx->Help_font_t4[char_4++][0][0]);
      else
        dest=&(gfx->Help_font_t2[char_2++][0][0]);
    else
      if (i&64)
        dest=&(gfx->Help_font_t3[char_3++][0][0]);
      else
        dest=&(gfx->Help_font_t1[char_1++][0][0]);
    
    if (Read_GUI_block(gui, cursor_x, cursor_y, dest, 6, 8, "help font (title)",0))
      return 1;
    cursor_x+=6;
  }
  cursor_y+=8;
  
  Current_help_section=0;
  Help_position=0;

  gfx->Preset_paintbrush_width [ 0]= 1;
  gfx->Preset_paintbrush_height[ 0]= 1;
  gfx->Paintbrush_type         [ 0]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 1]= 2;
  gfx->Preset_paintbrush_height[ 1]= 2;
  gfx->Paintbrush_type         [ 1]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 2]= 3;
  gfx->Preset_paintbrush_height[ 2]= 3;
  gfx->Paintbrush_type         [ 2]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 3]= 4;
  gfx->Preset_paintbrush_height[ 3]= 4;
  gfx->Paintbrush_type         [ 3]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 4]= 5;
  gfx->Preset_paintbrush_height[ 4]= 5;
  gfx->Paintbrush_type         [ 4]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 5]= 7;
  gfx->Preset_paintbrush_height[ 5]= 7;
  gfx->Paintbrush_type         [ 5]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 6]= 8;
  gfx->Preset_paintbrush_height[ 6]= 8;
  gfx->Paintbrush_type         [ 6]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 7]=12;
  gfx->Preset_paintbrush_height[ 7]=12;
  gfx->Paintbrush_type         [ 7]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 8]=16;
  gfx->Preset_paintbrush_height[ 8]=16;
  gfx->Paintbrush_type         [ 8]=PAINTBRUSH_SHAPE_SQUARE;

  gfx->Preset_paintbrush_width [ 9]=16;
  gfx->Preset_paintbrush_height[ 9]=16;
  gfx->Paintbrush_type         [ 9]=PAINTBRUSH_SHAPE_SIEVE_SQUARE;

  gfx->Preset_paintbrush_width [10]=15;
  gfx->Preset_paintbrush_height[10]=15;
  gfx->Paintbrush_type         [10]=PAINTBRUSH_SHAPE_DIAMOND;

  gfx->Preset_paintbrush_width [11]= 5;
  gfx->Preset_paintbrush_height[11]= 5;
  gfx->Paintbrush_type         [11]=PAINTBRUSH_SHAPE_DIAMOND;

  gfx->Preset_paintbrush_width [12]= 3;
  gfx->Preset_paintbrush_height[12]= 3;
  gfx->Paintbrush_type         [12]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [13]= 4;
  gfx->Preset_paintbrush_height[13]= 4;
  gfx->Paintbrush_type         [13]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [14]= 5;
  gfx->Preset_paintbrush_height[14]= 5;
  gfx->Paintbrush_type         [14]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [15]= 6;
  gfx->Preset_paintbrush_height[15]= 6;
  gfx->Paintbrush_type         [15]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [16]= 8;
  gfx->Preset_paintbrush_height[16]= 8;
  gfx->Paintbrush_type         [16]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [17]=10;
  gfx->Preset_paintbrush_height[17]=10;
  gfx->Paintbrush_type         [17]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [18]=12;
  gfx->Preset_paintbrush_height[18]=12;
  gfx->Paintbrush_type         [18]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [19]=14;
  gfx->Preset_paintbrush_height[19]=14;
  gfx->Paintbrush_type         [19]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [20]=16;
  gfx->Preset_paintbrush_height[20]=16;
  gfx->Paintbrush_type         [20]=PAINTBRUSH_SHAPE_ROUND;

  gfx->Preset_paintbrush_width [21]=15;
  gfx->Preset_paintbrush_height[21]=15;
  gfx->Paintbrush_type         [21]=PAINTBRUSH_SHAPE_SIEVE_ROUND;

  gfx->Preset_paintbrush_width [22]=11;
  gfx->Preset_paintbrush_height[22]=11;
  gfx->Paintbrush_type         [22]=PAINTBRUSH_SHAPE_SIEVE_ROUND;

  gfx->Preset_paintbrush_width [23]= 5;
  gfx->Preset_paintbrush_height[23]= 5;
  gfx->Paintbrush_type         [23]=PAINTBRUSH_SHAPE_SIEVE_ROUND;

  gfx->Preset_paintbrush_width [24]= 2;
  gfx->Preset_paintbrush_height[24]= 1;
  gfx->Paintbrush_type         [24]=PAINTBRUSH_SHAPE_HORIZONTAL_BAR;

  gfx->Preset_paintbrush_width [25]= 3;
  gfx->Preset_paintbrush_height[25]= 1;
  gfx->Paintbrush_type         [25]=PAINTBRUSH_SHAPE_HORIZONTAL_BAR;

  gfx->Preset_paintbrush_width [26]= 4;
  gfx->Preset_paintbrush_height[26]= 1;
  gfx->Paintbrush_type         [26]=PAINTBRUSH_SHAPE_HORIZONTAL_BAR;

  gfx->Preset_paintbrush_width [27]= 8;
  gfx->Preset_paintbrush_height[27]= 1;
  gfx->Paintbrush_type         [27]=PAINTBRUSH_SHAPE_HORIZONTAL_BAR;

  gfx->Preset_paintbrush_width [28]= 1;
  gfx->Preset_paintbrush_height[28]= 2;
  gfx->Paintbrush_type         [28]=PAINTBRUSH_SHAPE_VERTICAL_BAR;

  gfx->Preset_paintbrush_width [29]= 1;
  gfx->Preset_paintbrush_height[29]= 3;
  gfx->Paintbrush_type         [29]=PAINTBRUSH_SHAPE_VERTICAL_BAR;

  gfx->Preset_paintbrush_width [30]= 1;
  gfx->Preset_paintbrush_height[30]= 4;
  gfx->Paintbrush_type         [30]=PAINTBRUSH_SHAPE_VERTICAL_BAR;

  gfx->Preset_paintbrush_width [31]= 1;
  gfx->Preset_paintbrush_height[31]= 8;
  gfx->Paintbrush_type         [31]=PAINTBRUSH_SHAPE_VERTICAL_BAR;

  gfx->Preset_paintbrush_width [32]= 3;
  gfx->Preset_paintbrush_height[32]= 3;
  gfx->Paintbrush_type         [32]=PAINTBRUSH_SHAPE_CROSS;

  gfx->Preset_paintbrush_width [33]= 5;
  gfx->Preset_paintbrush_height[33]= 5;
  gfx->Paintbrush_type         [33]=PAINTBRUSH_SHAPE_CROSS;

  gfx->Preset_paintbrush_width [34]= 5;
  gfx->Preset_paintbrush_height[34]= 5;
  gfx->Paintbrush_type         [34]=PAINTBRUSH_SHAPE_PLUS;

  gfx->Preset_paintbrush_width [35]=15;
  gfx->Preset_paintbrush_height[35]=15;
  gfx->Paintbrush_type         [35]=PAINTBRUSH_SHAPE_PLUS;

  gfx->Preset_paintbrush_width [36]= 2;
  gfx->Preset_paintbrush_height[36]= 2;
  gfx->Paintbrush_type         [36]=PAINTBRUSH_SHAPE_SLASH;

  gfx->Preset_paintbrush_width [37]= 4;
  gfx->Preset_paintbrush_height[37]= 4;
  gfx->Paintbrush_type         [37]=PAINTBRUSH_SHAPE_SLASH;

  gfx->Preset_paintbrush_width [38]= 8;
  gfx->Preset_paintbrush_height[38]= 8;
  gfx->Paintbrush_type         [38]=PAINTBRUSH_SHAPE_SLASH;

  gfx->Preset_paintbrush_width [39]= 2;
  gfx->Preset_paintbrush_height[39]= 2;
  gfx->Paintbrush_type         [39]=PAINTBRUSH_SHAPE_ANTISLASH;

  gfx->Preset_paintbrush_width [40]= 4;
  gfx->Preset_paintbrush_height[40]= 4;
  gfx->Paintbrush_type         [40]=PAINTBRUSH_SHAPE_ANTISLASH;

  gfx->Preset_paintbrush_width [41]= 8;
  gfx->Preset_paintbrush_height[41]= 8;
  gfx->Paintbrush_type         [41]=PAINTBRUSH_SHAPE_ANTISLASH;

  gfx->Preset_paintbrush_width [42]= 4;
  gfx->Preset_paintbrush_height[42]= 4;
  gfx->Paintbrush_type         [42]=PAINTBRUSH_SHAPE_RANDOM;

  gfx->Preset_paintbrush_width [43]= 8;
  gfx->Preset_paintbrush_height[43]= 8;
  gfx->Paintbrush_type         [43]=PAINTBRUSH_SHAPE_RANDOM;

  gfx->Preset_paintbrush_width [44]=13;
  gfx->Preset_paintbrush_height[44]=13;
  gfx->Paintbrush_type         [44]=PAINTBRUSH_SHAPE_RANDOM;

  gfx->Preset_paintbrush_width [45]= 3;
  gfx->Preset_paintbrush_height[45]= 3;
  gfx->Paintbrush_type         [45]=PAINTBRUSH_SHAPE_MISC;

  gfx->Preset_paintbrush_width [46]= 3;
  gfx->Preset_paintbrush_height[46]= 3;
  gfx->Paintbrush_type         [46]=PAINTBRUSH_SHAPE_MISC;

  gfx->Preset_paintbrush_width [47]= 7;
  gfx->Preset_paintbrush_height[47]= 7;
  gfx->Paintbrush_type         [47]=PAINTBRUSH_SHAPE_MISC;

  for (index=0;index<NB_PAINTBRUSH_SPRITES;index++)
  {
    gfx->Preset_paintbrush_offset_X[index]=(gfx->Preset_paintbrush_width [index]>>1);
    gfx->Preset_paintbrush_offset_Y[index]=(gfx->Preset_paintbrush_height[index]>>1);
  }
  return 0;
}

T_Gui_skin * Load_graphics(const char * skin_file)
{
  T_Gui_skin * gfx;
  char filename[MAX_PATH_CHARACTERS];
  SDL_Surface * gui;

  gfx = (T_Gui_skin *)malloc(sizeof(T_Gui_skin));
  if (gfx == NULL)
  {
    sprintf(Gui_loading_error_message, "Not enough memory to read skin file\n");
    return NULL;
  }
  
  // Read the "skin" file
  strcpy(filename,Data_directory);
  strcat(filename,"skins" PATH_SEPARATOR);
  strcat(filename,skin_file);
  
  gui=IMG_Load(filename);
  if (!gui)
  {
    sprintf(Gui_loading_error_message, "Unable to load the skin image (missing? not an image file?)\n");
    free(gfx);
    return NULL;
  }
  if (Parse_skin(gui, gfx))
  {
    SDL_FreeSurface(gui);
    free(gfx);
    return NULL;
  }
  SDL_FreeSurface(gui);
  return gfx;
}

// ---- font loading -----

byte Parse_font(SDL_Surface * image, byte * font)
{
  int character;
  byte color;
  int x, y;
  int chars_per_line;
  
  // Check image size
  if (image->w % 8)
  {
    sprintf(Gui_loading_error_message, "Error in font file: Image width is not a multiple of 8.\n");
    return 1;
  }
  if (image->w * image->h < 8*8*256)
  {
    sprintf(Gui_loading_error_message, "Error in font file: Image is too small to be a 256-character 8x8 font.\n");
    return 1;
  }
  chars_per_line = image->w/8;

  for (character=0; character < 256; character++)
  {
    for (y=0; y<8; y++)
    {
      for (x=0;x<8; x++)
      {
        // Pick pixel
        color = Get_SDL_pixel_8(image, (character % chars_per_line)*8+x, (character / chars_per_line)*8+y);
        if (color > 1)
        {
          sprintf(Gui_loading_error_message, "Error in font file: Only colors 0 and 1 can be used for the font.\n");
          return 1;
        }
        // Put it in font. 0 = BG, 1 = FG.
        font[character*64 + y*8 + x]=color;
      }
    }
  }
  return 0;
}

byte * Load_font(const char * font_name)
{
  byte * font;
  char filename[MAX_PATH_CHARACTERS];
  SDL_Surface * image;

  font = (byte *)malloc(8*8*256);
  if (font == NULL)
  {
    sprintf(Gui_loading_error_message, "Not enough memory to read font file\n");
    return NULL;
  }
  
  // Read the file containing the image
  sprintf(filename,"%sskins%s%s", Data_directory, PATH_SEPARATOR, font_name);
  
  image=IMG_Load(filename);
  if (!image)
  {
    sprintf(Gui_loading_error_message, "Unable to load the skin image (missing? not an image file?)\n");
    free(font);
    return NULL;
  }
  if (Parse_font(image, font))
  {
    SDL_FreeSurface(image);
    free(font);
    return NULL;
  }
  SDL_FreeSurface(image);
  return font;
}


// Initialisation des boutons:

  // Action factice:

void Do_nothing(void)
{}

  // Initialiseur d'un bouton:

void Init_button(byte   btn_number,
                        word   x_offset      , word   y_offset,
                        word   width         , word   height,
                        byte   shape,
                        Func_action left_action,
                        Func_action right_action,
                        Func_action unselect_action,
                        byte   family)
{
  Buttons_Pool[btn_number].X_offset        =x_offset;
  Buttons_Pool[btn_number].Y_offset        =y_offset;
  Buttons_Pool[btn_number].Width           =width-1;
  Buttons_Pool[btn_number].Height          =height-1;
  Buttons_Pool[btn_number].Pressed         =0;
  Buttons_Pool[btn_number].Shape           =shape;
  Buttons_Pool[btn_number].Left_action     =left_action;
  Buttons_Pool[btn_number].Right_action    =right_action;
  Buttons_Pool[btn_number].Unselect_action =unselect_action;
  Buttons_Pool[btn_number].Family          =family;
}


  // Initiliseur de tous les boutons:

void Init_buttons(void)
{
  byte button_index;

  for (button_index=0;button_index<NB_BUTTONS;button_index++)
  {
    Buttons_Pool[button_index].Left_shortcut[0]=0;
    Buttons_Pool[button_index].Left_shortcut[1]=0;
    Buttons_Pool[button_index].Right_shortcut[0]=0;
    Buttons_Pool[button_index].Right_shortcut[1]=0;
    Init_button(button_index,
                       0,0,
                       1,1,
                       BUTTON_SHAPE_RECTANGLE,
                       Do_nothing,Do_nothing,
                       Do_nothing,
                       0);
  }

  // Ici viennent les déclarations des boutons que l'on sait gérer

  Init_button(BUTTON_PAINTBRUSHES,
                     0,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Paintbrush_menu,Button_Brush_monochrome,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_ADJUST,
                     0,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Adjust,Button_Transform_menu,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_DRAW,
                     17,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Draw,Button_Draw_switch_mode,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_CURVES,
                     17,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Curves,Button_Curves_switch_mode,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_LINES,
                     34,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Lines,Button_Lines_switch_mode,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_AIRBRUSH,
                     34,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Airbrush,Button_Airbrush_menu,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_FLOODFILL,
                     51,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Fill,Button_Replace,
                     Button_Unselect_fill,
                     FAMILY_TOOL);

  Init_button(BUTTON_POLYGONS,
                     51,18,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_polygon,Button_Polyform,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_POLYFILL,
                     52,19,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Polyfill,Button_Filled_polyform,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_RECTANGLES,
                     68,1,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_Empty_rectangle,Button_Empty_rectangle,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_FILLRECT,
                     69,2,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Filled_rectangle,Button_Filled_rectangle,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_CIRCLES,
                     68,18,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_Empty_circle,Button_Empty_ellipse,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_FILLCIRC,
                     69,19,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Filled_circle,Button_Filled_ellipse,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_GRADRECT,
                     85,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Grad_rectangle,Button_Gradients,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_SPHERES,
                     85,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Grad_circle,Button_Grad_ellipse,
                     Do_nothing,
                     FAMILY_TOOL);

  Init_button(BUTTON_BRUSH,
                     106,1,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_Brush,Button_Restore_brush,
                     Button_Unselect_brush,
                     FAMILY_INTERRUPTION);

  Init_button(BUTTON_POLYBRUSH,
                     107,2,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Lasso,Button_Restore_brush,
                     Button_Unselect_lasso,
                     FAMILY_INTERRUPTION);

  Init_button(BUTTON_BRUSH_EFFECTS,
                     106, 18,
                     16, 16,
                     BUTTON_SHAPE_RECTANGLE,
#ifdef __ENABLE_LUA__
                     Button_Brush_FX, Button_Brush_Factory,
#else
                     Button_Brush_FX, Button_Brush_FX,
#endif
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_EFFECTS,
                     123,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Effects,Button_Effects,
                     Do_nothing,
                     FAMILY_EFFECTS);

  Init_button(BUTTON_TEXT,
                     123,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Text,Button_Text,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_MAGNIFIER,
                     140,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Magnify,Button_Magnify_menu,
                     Button_Unselect_magnifier,
                     FAMILY_INTERRUPTION);

  Init_button(BUTTON_COLORPICKER,
                     140,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Colorpicker,Button_Invert_foreback,
                     Button_Unselect_colorpicker,
                     FAMILY_INTERRUPTION);

  Init_button(BUTTON_RESOL,
                     161,1,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Resolution,Button_Safety_resolution,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_PAGE,
                     161,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Page,Button_Copy_page,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_SAVE,
                     178,1,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_Save,Button_Autosave,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_LOAD,
                     179,2,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Load,Button_Reload,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_SETTINGS,
                     178,18,
                     16,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Settings,Button_Skins,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_CLEAR,
                     195,1,
                     17,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Clear,Button_Clear_with_backcolor,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_HELP,
                     195,18,
                     17,16,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Help,Button_Stats,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_UNDO,
                     213,1,
                     19,12,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Undo,Button_Redo,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_KILL,
                     213,14,
                     19,7,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Kill,Button_Kill,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_QUIT,
                     213,22,
                     19,12,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Quit,Button_Quit,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_PALETTE,
                     237,9,
                     16,8,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Palette,Button_Secondary_palette,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_PAL_LEFT,
                     237,18,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_TOP_LEFT,
                     Button_Pal_left,Button_Pal_left_fast,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_PAL_RIGHT,
                     238,19,
                     15,15,
                     BUTTON_SHAPE_TRIANGLE_BOTTOM_RIGHT,
                     Button_Pal_right,Button_Pal_right_fast,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_CHOOSE_COL,
                     MENU_WIDTH+1,2,
                     1,32, // La largeur est mise à jour à chq chngmnt de mode
                     BUTTON_SHAPE_NO_FRAME,
                     Button_Select_forecolor,Button_Select_backcolor,
                     Do_nothing,
                     FAMILY_INSTANT);

  Init_button(BUTTON_HIDE,
                     0,35,
                     16,9,
                     BUTTON_SHAPE_RECTANGLE,
                     Button_Hide_menu,Button_Hide_menu,
                     Do_nothing,
                     FAMILY_TOOLBAR);
}



// Initialisation des opérations:

  // Initialiseur d'une opération:

void Init_operation(byte operation_number,
                           byte mouse_button,
                           byte stack_index,
                           Func_action action,
                           byte hide_cursor,
                           byte fast_mouse)
{
  Operation[operation_number][mouse_button]
           [stack_index].Action=action;
  Operation[operation_number][mouse_button]
           [stack_index].Hide_cursor=hide_cursor;
  Operation[operation_number][mouse_button]
           [stack_index].Fast_mouse=fast_mouse;
}


  // Initiliseur de toutes les opérations:

void Init_operations(void)
{
  byte number; // Numéro de l'option en cours d'auto-initialisation
  byte Button; // Button souris en cours d'auto-initialisation
  byte stack_index; // Taille de la pile en cours d'auto-initialisation
  #define HIDE_CURSOR 1
  #define FAST_MOUSE 1

  // Auto-initialisation des opérations (vers des actions inoffensives)

  for (number=0;number<NB_OPERATIONS;number++)
    for (Button=0;Button<3;Button++)
      for (stack_index=0;stack_index<OPERATION_STACK_SIZE;stack_index++)
        Init_operation(number,Button,stack_index,Print_coordinates,0,FAST_MOUSE);


  // Ici viennent les déclarations détaillées des opérations
  Init_operation(OPERATION_CONTINUOUS_DRAW,1,0,
                        Freehand_mode1_1_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_CONTINUOUS_DRAW,1,2,
                        Freehand_mode1_1_2,0,0);
  Init_operation(OPERATION_CONTINUOUS_DRAW,0,2,
                        Freehand_mode12_0_2,0,0);
  Init_operation(OPERATION_CONTINUOUS_DRAW,2,0,
                        Freehand_mode1_2_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_CONTINUOUS_DRAW,2,2,
                        Freehand_mode1_2_2,0,0);

  Init_operation(OPERATION_DISCONTINUOUS_DRAW,1,0,
                        Freehand_mode2_1_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_DISCONTINUOUS_DRAW,1,2,
                        Freehand_mode2_1_2,0,0);
  Init_operation(OPERATION_DISCONTINUOUS_DRAW,0,2,
                        Freehand_mode12_0_2,0,0);
  Init_operation(OPERATION_DISCONTINUOUS_DRAW,2,0,
                        Freehand_mode2_2_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_DISCONTINUOUS_DRAW,2,2,
                        Freehand_mode2_2_2,0,0);

  Init_operation(OPERATION_POINT_DRAW,1,0,
                        Freehand_mode3_1_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POINT_DRAW,2,0,
                        Freehand_Mode3_2_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POINT_DRAW,0,1,
                        Freehand_mode3_0_1,0,FAST_MOUSE);

  Init_operation(OPERATION_LINE,1,0,
                        Line_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_LINE,1,5,
                        Line_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_LINE,0,5,
                        Line_0_5,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_LINE,2,0,
                        Line_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_LINE,2,5,
                        Line_12_5,0,FAST_MOUSE);

  Init_operation(OPERATION_K_LIGNE,1,0,
                        K_line_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,1,6,
                        K_line_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,1,7,
                        K_line_12_7,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,2,FAST_MOUSE,
                        K_line_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,2,6,
                        K_line_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,2,7,
                        K_line_12_7,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,0,6,
                        K_line_0_6,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_K_LIGNE,0,7,
                        K_line_12_6,0,FAST_MOUSE);

  Init_operation(OPERATION_EMPTY_RECTANGLE,1,0,
                        Rectangle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_RECTANGLE,2,0,
                        Rectangle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_RECTANGLE,1,5,
                        Rectangle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_RECTANGLE,2,5,
                        Rectangle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_RECTANGLE,0,5,
                        Empty_rectangle_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_FILLED_RECTANGLE,1,0,
                        Rectangle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_RECTANGLE,2,0,
                        Rectangle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_RECTANGLE,1,5,
                        Rectangle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_RECTANGLE,2,5,
                        Rectangle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_RECTANGLE,0,5,
                        Filled_rectangle_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_EMPTY_CIRCLE,1,0,
                        Circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_CIRCLE,2,0,
                        Circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_CIRCLE,1,5,
                        Circle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_CIRCLE,2,5,
                        Circle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_CIRCLE,0,5,
                        Empty_circle_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_FILLED_CIRCLE,1,0,
                        Circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_CIRCLE,2,0,
                        Circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_CIRCLE,1,5,
                        Circle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_CIRCLE,2,5,
                        Circle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_CIRCLE,0,5,
                        Filled_circle_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_EMPTY_ELLIPSE,1,0,
                        Ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_ELLIPSE,2,0,
                        Ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_ELLIPSE,1,5,
                        Ellipse_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_ELLIPSE,2,5,
                        Ellipse_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_EMPTY_ELLIPSE,0,5,
                        Empty_ellipse_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_FILLED_ELLIPSE,1,0,
                        Ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_ELLIPSE,2,0,
                        Ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_ELLIPSE,1,5,
                        Ellipse_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_ELLIPSE,2,5,
                        Ellipse_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_ELLIPSE,0,5,
                        Filled_ellipse_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_FILL,1,0,
                        Fill_1_0,0,FAST_MOUSE);
  Init_operation(OPERATION_FILL,2,0,
                        Fill_2_0,0,FAST_MOUSE);

  Init_operation(OPERATION_REPLACE,1,0,
                        Replace_1_0,0,FAST_MOUSE);
  Init_operation(OPERATION_REPLACE,2,0,
                        Replace_2_0,0,FAST_MOUSE);

  Init_operation(OPERATION_GRAB_BRUSH,1,0,
                        Brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAB_BRUSH,2,0,
                        Brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAB_BRUSH,1,5,
                        Brush_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAB_BRUSH,2,5,
                        Brush_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAB_BRUSH,0,5,
                        Brush_0_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_STRETCH_BRUSH,1,0,
                        Stretch_brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_STRETCH_BRUSH,2,0,
                        Stretch_brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_STRETCH_BRUSH,1,7,
                        Stretch_brush_1_7,0,FAST_MOUSE);
  Init_operation(OPERATION_STRETCH_BRUSH,0,7,
                        Stretch_brush_0_7,0,FAST_MOUSE);
  Init_operation(OPERATION_STRETCH_BRUSH,2,7,
                        Stretch_brush_2_7,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_ROTATE_BRUSH,1,0,
                        Rotate_brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_ROTATE_BRUSH,2,0,
                        Rotate_brush_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_ROTATE_BRUSH,1,5,
                        Rotate_brush_1_5,0,FAST_MOUSE);
  Init_operation(OPERATION_ROTATE_BRUSH,0,5,
                        Rotate_brush_0_5,0,FAST_MOUSE);
  Init_operation(OPERATION_ROTATE_BRUSH,2,5,
                        Rotate_brush_2_5,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_DISTORT_BRUSH,0,0,
                        Distort_brush_0_0,0,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,1,0,
                        Distort_brush_1_0,0,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,1,8,
                        Distort_brush_1_8,0,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,2,8,
                        Distort_brush_2_8,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,2,0,
                        Distort_brush_2_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,1,9,
                        Distort_brush_1_9,0,FAST_MOUSE);
  Init_operation(OPERATION_DISTORT_BRUSH,0,9,
                        Distort_brush_0_9,0,FAST_MOUSE);


  Init_operation(OPERATION_POLYBRUSH,1,0,
                        Filled_polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYBRUSH,2,0,
                        Filled_polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYBRUSH,1,8,
                        Polybrush_12_8,0,0);
  Init_operation(OPERATION_POLYBRUSH,2,8,
                        Polybrush_12_8,0,0);
  Init_operation(OPERATION_POLYBRUSH,0,8,
                        Filled_polyform_0_8,0,FAST_MOUSE);

  Colorpicker_color=-1;
  Init_operation(OPERATION_COLORPICK,1,0,
                        Colorpicker_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_COLORPICK,2,0,
                        Colorpicker_12_0,0,FAST_MOUSE);
  Init_operation(OPERATION_COLORPICK,1,1,
                        Colorpicker_1_1,0,FAST_MOUSE);
  Init_operation(OPERATION_COLORPICK,2,1,
                        Colorpicker_2_1,0,FAST_MOUSE);
  Init_operation(OPERATION_COLORPICK,0,1,
                        Colorpicker_0_1,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_MAGNIFY,1,0,
                        Magnifier_12_0,0,FAST_MOUSE);
  Init_operation(OPERATION_MAGNIFY,2,0,
                        Magnifier_12_0,0,FAST_MOUSE);

  Init_operation(OPERATION_4_POINTS_CURVE,1,0,
                        Curve_34_points_1_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,2,0,
                        Curve_34_points_2_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,1,5,
                        Curve_34_points_1_5,0,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,2,5,
                        Curve_34_points_2_5,0,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,0,5,
                        Curve_4_points_0_5,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,1,9,
                        Curve_4_points_1_9,0,FAST_MOUSE);
  Init_operation(OPERATION_4_POINTS_CURVE,2,9,
                        Curve_4_points_2_9,0,FAST_MOUSE);

  Init_operation(OPERATION_3_POINTS_CURVE,1,0,
                        Curve_34_points_1_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,2,0,
                        Curve_34_points_2_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,1,5,
                        Curve_34_points_1_5,0,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,2,5,
                        Curve_34_points_2_5,0,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,0,5,
                        Curve_3_points_0_5,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,0,11,
                        Curve_3_points_0_11,0,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,1,11,
                        Curve_3_points_12_11,0,FAST_MOUSE);
  Init_operation(OPERATION_3_POINTS_CURVE,2,11,
                        Curve_3_points_12_11,0,FAST_MOUSE);

  Init_operation(OPERATION_AIRBRUSH,1,0,
                        Airbrush_1_0,0,0);
  Init_operation(OPERATION_AIRBRUSH,2,0,
                        Airbrush_2_0,0,0);
  Init_operation(OPERATION_AIRBRUSH,1,2,
                        Airbrush_12_2,0,0);
  Init_operation(OPERATION_AIRBRUSH,2,2,
                        Airbrush_12_2,0,0);
  Init_operation(OPERATION_AIRBRUSH,0,2,
                        Airbrush_0_2,0,0);

  Init_operation(OPERATION_POLYGON,1,0,
                        Polygon_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,2,0,
                        Polygon_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,1,8,
                        K_line_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,2,8,
                        K_line_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,1,9,
                        Polygon_12_9,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,2,9,
                        Polygon_12_9,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,0,8,
                        K_line_0_6,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYGON,0,9,
                        K_line_12_6,0,FAST_MOUSE);

  Init_operation(OPERATION_POLYFILL,1,0,
                        Polyfill_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,2,0,
                        Polyfill_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,1,8,
                        Polyfill_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,2,8,
                        Polyfill_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,1,9,
                        Polyfill_12_9,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,2,9,
                        Polyfill_12_9,0,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,0,8,
                        Polyfill_0_8,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYFILL,0,9,
                        Polyfill_12_8,0,FAST_MOUSE);

  Init_operation(OPERATION_POLYFORM,1,0,
                        Polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYFORM,2,0,
                        Polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_POLYFORM,1,8,
                        Polyform_12_8,0,0);
  Init_operation(OPERATION_POLYFORM,2,8,
                        Polyform_12_8,0,0);
  Init_operation(OPERATION_POLYFORM,0,8,
                        Polyform_0_8,0,FAST_MOUSE);

  Init_operation(OPERATION_FILLED_POLYFORM,1,0,
                        Filled_polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_POLYFORM,2,0,
                        Filled_polyform_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_FILLED_POLYFORM,1,8,
                        Filled_polyform_12_8,0,0);
  Init_operation(OPERATION_FILLED_POLYFORM,2,8,
                        Filled_polyform_12_8,0,0);
  Init_operation(OPERATION_FILLED_POLYFORM,0,8,
                        Filled_polyform_0_8,0,FAST_MOUSE);

  Init_operation(OPERATION_FILLED_CONTOUR,1,0,
                        Filled_polyform_12_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_FILLED_CONTOUR,2,0,
                        Filled_polyform_12_0,HIDE_CURSOR,0);
  Init_operation(OPERATION_FILLED_CONTOUR,1,8,
                        Filled_polyform_12_8,0,0);
  Init_operation(OPERATION_FILLED_CONTOUR,2,8,
                        Filled_polyform_12_8,0,0);
  Init_operation(OPERATION_FILLED_CONTOUR,0,8,
                        Filled_contour_0_8,0,0);

  Init_operation(OPERATION_SCROLL,1,0,
                        Scroll_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_SCROLL,2,0,
                        Scroll_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_SCROLL,1,4,
                        Scroll_12_4,0,FAST_MOUSE);
  Init_operation(OPERATION_SCROLL,2,4,
                        Scroll_12_4,0,FAST_MOUSE);
  Init_operation(OPERATION_SCROLL,0,4,
                        Scroll_0_4,HIDE_CURSOR,FAST_MOUSE);

  Init_operation(OPERATION_GRAD_CIRCLE,1,0,Grad_circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,2,0,Grad_circle_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,1,6,Grad_circle_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,2,6,Grad_circle_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,0,6,Grad_circle_0_6,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,1,8,Grad_circle_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,2,8,Grad_circle_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_CIRCLE,0,8,Grad_circle_or_ellipse_0_8,0,FAST_MOUSE);

  Init_operation(OPERATION_GRAD_ELLIPSE,1,0,Grad_ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,2,0,Grad_ellipse_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,1,6,Grad_ellipse_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,2,6,Grad_ellipse_12_6,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,0,6,Grad_ellipse_0_6,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,1,8,Grad_ellipse_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,2,8,Grad_ellipse_12_8,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_ELLIPSE,0,8,Grad_circle_or_ellipse_0_8,0,FAST_MOUSE);

  Init_operation(OPERATION_GRAD_RECTANGLE,1,0,Grad_rectangle_12_0,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,1,5,Grad_rectangle_12_5,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,0,5,Grad_rectangle_0_5,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,0,7,Grad_rectangle_0_7,0,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,1,7,Grad_rectangle_12_7,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,2,7,Grad_rectangle_12_7,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,1,9,Grad_rectangle_12_9,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_GRAD_RECTANGLE,0,9,Grad_rectangle_0_9,0,FAST_MOUSE);


  Init_operation(OPERATION_CENTERED_LINES,1,0,
                        Centered_lines_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,2,0,
                        Centered_lines_12_0,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,1,3,
                        Centered_lines_12_3,0,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,2,3,
                        Centered_lines_12_3,0,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,0,3,
                        Centered_lines_0_3,HIDE_CURSOR,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,1,7,
                        Centered_lines_12_7,0,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,2,7,
                        Centered_lines_12_7,0,FAST_MOUSE);
  Init_operation(OPERATION_CENTERED_LINES,0,7,
                        Centered_lines_0_7,0,FAST_MOUSE);
}



//-- Définition des modes vidéo: --------------------------------------------

  // Définition d'un mode:

void Set_video_mode(short  width,
                        short  height,
                        byte   mode,
                        word   fullscreen)
{
  byte supported = 0;

  if (Nb_video_modes >= MAX_VIDEO_MODES-1)
  {
    DEBUG("Error! Attempt to create too many videomodes. Maximum is:", MAX_VIDEO_MODES);
    return;
  }
  if (!fullscreen)
    supported = 128; // Prefere, non modifiable
  else if (SDL_VideoModeOK(width, height, 8, SDL_FULLSCREEN))
    supported = 1; // supported
  else
  {
    // Non supporte : on ne le prend pas
    return;
  }

  Video_mode[Nb_video_modes].Width          = width;
  Video_mode[Nb_video_modes].Height          = height;
  Video_mode[Nb_video_modes].Mode             = mode;
  Video_mode[Nb_video_modes].Fullscreen       = fullscreen;
  Video_mode[Nb_video_modes].State                   = supported;
  Nb_video_modes ++;
}

// Utilisé pour trier les modes retournés par SDL
int Compare_video_modes(const void *p1, const void *p2)
{
  const T_Video_mode *mode1 = (const T_Video_mode *)p1;
  const T_Video_mode *mode2 = (const T_Video_mode *)p2;

  // Tris par largeur
  if(mode1->Width - mode2->Width)
    return mode1->Width - mode2->Width;

  // Tri par hauteur
  return mode1->Height - mode2->Height;
}


// Initializes the list of available video modes
void Set_all_video_modes(void)
{
  SDL_Rect** Modes;
  Nb_video_modes=0;
  
  // The first mode will have index number 0.
  // It will be the default mode if an unsupported one
  // is requested in gfx2.ini
  #if defined(__GP2X__)
  // Native GP2X resolution
  Set_video_mode( 320,240,0, 1);
  #else
  // Window mode, with default size of 640x480
  Set_video_mode( 640,480,0, 0);
  #endif

  Set_video_mode( 320,200,0, 1);
  Set_video_mode( 320,224,0, 1);
  #if !defined(__GP2X__)
  // For the GP2X, this one is already declared above.
  Set_video_mode( 320,240,0, 1);
  #endif
  Set_video_mode( 320,256,0, 1);
  Set_video_mode( 320,270,0, 1);
  Set_video_mode( 320,282,0, 1);
  Set_video_mode( 320,300,0, 1);
  Set_video_mode( 320,360,0, 1);
  Set_video_mode( 320,400,0, 1);
  Set_video_mode( 320,448,0, 1);
  Set_video_mode( 320,480,0, 1);
  Set_video_mode( 320,512,0, 1);
  Set_video_mode( 320,540,0, 1);
  Set_video_mode( 320,564,0, 1);
  Set_video_mode( 320,600,0, 1);
  Set_video_mode( 360,200,0, 1);
  Set_video_mode( 360,224,0, 1);
  Set_video_mode( 360,240,0, 1);
  Set_video_mode( 360,256,0, 1);
  Set_video_mode( 360,270,0, 1);
  Set_video_mode( 360,282,0, 1);
  Set_video_mode( 360,300,0, 1);
  Set_video_mode( 360,360,0, 1);
  Set_video_mode( 360,400,0, 1);
  Set_video_mode( 360,448,0, 1);
  Set_video_mode( 360,480,0, 1);
  Set_video_mode( 360,512,0, 1);
  Set_video_mode( 360,540,0, 1);
  Set_video_mode( 360,564,0, 1);
  Set_video_mode( 360,600,0, 1);
  Set_video_mode( 400,200,0, 1);
  Set_video_mode( 400,224,0, 1);
  Set_video_mode( 400,240,0, 1);
  Set_video_mode( 400,256,0, 1);
  Set_video_mode( 400,270,0, 1);
  Set_video_mode( 400,282,0, 1);
  Set_video_mode( 400,300,0, 1);
  Set_video_mode( 400,360,0, 1);
  Set_video_mode( 400,400,0, 1);
  Set_video_mode( 400,448,0, 1);
  Set_video_mode( 400,480,0, 1);
  Set_video_mode( 400,512,0, 1);
  Set_video_mode( 400,540,0, 1);
  Set_video_mode( 400,564,0, 1);
  Set_video_mode( 400,600,0, 1);
  Set_video_mode( 640,224,0, 1);
  Set_video_mode( 640,240,0, 1);
  Set_video_mode( 640,256,0, 1);
  Set_video_mode( 640,270,0, 1);
  Set_video_mode( 640,300,0, 1);
  Set_video_mode( 640,350,0, 1);
  Set_video_mode( 640,400,0, 1);
  Set_video_mode( 640,448,0, 1);
  Set_video_mode( 640,480,0, 1);
  Set_video_mode( 640,512,0, 1);
  Set_video_mode( 640,540,0, 1);
  Set_video_mode( 640,564,0, 1);
  Set_video_mode( 640,600,0, 1);
  Set_video_mode( 800,600,0, 1);
  Set_video_mode(1024,768,0, 1);

  Modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
  if ((Modes != (SDL_Rect**)0) && (Modes!=(SDL_Rect**)-1))
  {
    int index;
    for (index=0; Modes[index]; index++)
    {
      int index2;
#if defined(__GP2X__)
	  // On the GP2X the first mode is not windowed, so include it in the search.
	  index2=0;
#else
	  index2=1;
#endif
      for (/**/; index2 < Nb_video_modes; index2++)
        if (Modes[index]->w == Video_mode[index2].Width &&
            Modes[index]->h == Video_mode[index2].Height)
        {
          // Was already in the hard-coded list: ok, don't add.
          break;
        }
      if (index2 >= Nb_video_modes && Modes[index]->w>=320 && Modes[index]->h>=200)
      {
        // New mode to add to the list
        Set_video_mode(Modes[index]->w,Modes[index]->h,0, 1);
      }
    }
    // Sort the modes : those found by SDL were listed at the end.
    // Note that we voluntarily omit the first entry: the default mode.
    qsort(&Video_mode[1], Nb_video_modes - 1, sizeof(T_Video_mode), Compare_video_modes);
  }
}

//---------------------------------------------------------------------------

int Load_CFG(int reload_all)
{
  FILE*  Handle;
  char filename[MAX_PATH_CHARACTERS];
  long file_size;
  int  index,index2;
  T_Config_header       cfg_header;
  T_Config_chunk        Chunk;
  T_Config_shortcut_info cfg_shortcut_info;
  T_Config_video_mode   cfg_video_mode;
  int key_conversion = 0;

  strcpy(filename,Config_directory);
  strcat(filename,"gfx2.cfg");

  file_size=File_length(filename);

  if ((Handle=fopen(filename,"rb"))==NULL)
    return ERROR_CFG_MISSING;

  if ( (file_size<(long)sizeof(cfg_header))
    || (!Read_bytes(Handle, &cfg_header.Signature, 3))
    || memcmp(cfg_header.Signature,"CFG",3)
    || (!Read_byte(Handle, &cfg_header.Version1))
    || (!Read_byte(Handle, &cfg_header.Version2))
    || (!Read_byte(Handle, &cfg_header.Beta1))
    || (!Read_byte(Handle, &cfg_header.Beta2)) )
      goto Erreur_lecture_config;

  // Version DOS de Robinson et X-Man
  if ( (cfg_header.Version1== 2)
    && (cfg_header.Version2== 0)
    && (cfg_header.Beta1== 96))
  {
    // Les touches (scancodes) sont à convertir)
    key_conversion = 1;
  }
  // Version SDL jusqu'a 98%
  else if ( (cfg_header.Version1== 2)
    && (cfg_header.Version2== 0)
    && (cfg_header.Beta1== 97))
  {
    // Les touches 00FF (pas de touche) sont a comprendre comme 0x0000
    key_conversion = 2;
  }
  // Version SDL
  else if ( (cfg_header.Version1!=VERSION1)
    || (cfg_header.Version2!=VERSION2)
    || (cfg_header.Beta1!=BETA1)
    || (cfg_header.Beta2!=BETA2) )
    goto Erreur_config_ancienne;

  // - Lecture des infos contenues dans le fichier de config -
  while (Read_byte(Handle, &Chunk.Number))
  {
    Read_word_le(Handle, &Chunk.Size);
    switch (Chunk.Number)
    {
      case CHUNK_KEYS: // Touches
        if (reload_all)
        {
          for (index=0; index<(long)(Chunk.Size/sizeof(cfg_shortcut_info)); index++)
          {
            if (!Read_word_le(Handle, &cfg_shortcut_info.Number) ||
                !Read_word_le(Handle, &cfg_shortcut_info.Key) ||
                !Read_word_le(Handle, &cfg_shortcut_info.Key2) )
              goto Erreur_lecture_config;
            else
            {
              if (key_conversion==1)
              {
                cfg_shortcut_info.Key = Key_for_scancode(cfg_shortcut_info.Key);
              }
              else if (key_conversion==2)
              {
                if (cfg_shortcut_info.Key == 0x00FF)
                  cfg_shortcut_info.Key = 0x0000;
                if (cfg_shortcut_info.Key2 == 0x00FF)
                  cfg_shortcut_info.Key2 = 0x0000;
              }
              
              for (index2=0;
                 ((index2<NB_SHORTCUTS) && (ConfigKey[index2].Number!=cfg_shortcut_info.Number));
                 index2++);
              if (index2<NB_SHORTCUTS)
              {
                switch(Ordering[index2]>>8)
                {
                  case 0 :
                    Config_Key[Ordering[index2]&0xFF][0]=cfg_shortcut_info.Key;
                    Config_Key[Ordering[index2]&0xFF][1]=cfg_shortcut_info.Key2;
                    break;
                  case 1 :
                    Buttons_Pool[Ordering[index2]&0xFF].Left_shortcut[0] = cfg_shortcut_info.Key;
                    Buttons_Pool[Ordering[index2]&0xFF].Left_shortcut[1] = cfg_shortcut_info.Key2;
                    break;
                  case 2 :
                    Buttons_Pool[Ordering[index2]&0xFF].Right_shortcut[0] = cfg_shortcut_info.Key;
                    Buttons_Pool[Ordering[index2]&0xFF].Right_shortcut[1] = cfg_shortcut_info.Key2;
                    break;
                }
              }
              else
                goto Erreur_lecture_config;
            }
          }
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_VIDEO_MODES: // Modes vidéo
        for (index=0; index<(long)(Chunk.Size/sizeof(cfg_video_mode)); index++)
        {
          if (!Read_byte(Handle, &cfg_video_mode.State) ||
              !Read_word_le(Handle, &cfg_video_mode.Width) ||
              !Read_word_le(Handle, &cfg_video_mode.Height) )
            goto Erreur_lecture_config;

#if defined(__GP2X__)
		  index2=0;
#else
		  index2=1;
#endif
          for (/**/; index2<Nb_video_modes; index2++)
          {
            if (Video_mode[index2].Width==cfg_video_mode.Width &&
                Video_mode[index2].Height==cfg_video_mode.Height)
            {
              // On ne prend le paramètre utilisateur que si la résolution
              // est effectivement supportée par SDL
              // Seules les deux petits bits sont récupérés, car les anciens fichiers
              // de configuration (DOS 96.5%) utilisaient d'autres bits.
              if (! (Video_mode[index2].State & 128))
                Video_mode[index2].State=cfg_video_mode.State&3;
              break;
            }
          }
        }
        break;
      case CHUNK_SHADE: // Shade
        if (reload_all)
        {
          if (! Read_byte(Handle, &Shade_current) )
            goto Erreur_lecture_config;

          for (index=0; index<8; index++)
          {
            for (index2=0; index2<512; index2++)
            {
              if (! Read_word_le(Handle, &Shade_list[index].List[index2]))
                goto Erreur_lecture_config;
            }
            if (! Read_byte(Handle, &Shade_list[index].Step) ||
              ! Read_byte(Handle, &Shade_list[index].Mode) )
            goto Erreur_lecture_config;
          }
          Shade_list_to_lookup_tables(Shade_list[Shade_current].List,
            Shade_list[Shade_current].Step,
            Shade_list[Shade_current].Mode,
            Shade_table_left,Shade_table_right);
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_MASK: // Masque
        if (reload_all)
        {
          if (!Read_bytes(Handle, Mask_table, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_STENCIL: // Stencil
        if (reload_all)
        {
          if (!Read_bytes(Handle, Stencil, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_GRADIENTS: // Infos sur les dégradés
        if (reload_all)
        {
          if (! Read_byte(Handle, &Current_gradient))
            goto Erreur_lecture_config;
          for(index=0;index<16;index++)
          {
            if (!Read_byte(Handle, &Gradient_array[index].Start) ||
                !Read_byte(Handle, &Gradient_array[index].End) ||
                !Read_dword_le(Handle, &Gradient_array[index].Inverse) ||
                !Read_dword_le(Handle, &Gradient_array[index].Mix) ||
                !Read_dword_le(Handle, &Gradient_array[index].Technique) )
            goto Erreur_lecture_config;
          }
          Load_gradient_data(Current_gradient);
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_SMOOTH: // Matrice du smooth
        if (reload_all)
        {
          for (index=0; index<3; index++)
            for (index2=0; index2<3; index2++)
              if (!Read_byte(Handle, &(Smooth_matrix[index][index2])))
                goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_EXCLUDE_COLORS: // Exclude_color
        if (reload_all)
        {
          if (!Read_bytes(Handle, Exclude_color, 256))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      case CHUNK_QUICK_SHADE: // Quick-shade
        if (reload_all)
        {
          if (!Read_byte(Handle, &Quick_shade_step))
            goto Erreur_lecture_config;
          if (!Read_byte(Handle, &Quick_shade_loop))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
        case CHUNK_GRID: // Grille
        if (reload_all)
        {
          if (!Read_word_le(Handle, &Snap_width))
            goto Erreur_lecture_config;
          if (!Read_word_le(Handle, &Snap_height))
            goto Erreur_lecture_config;
          if (!Read_word_le(Handle, &Snap_offset_X))
            goto Erreur_lecture_config;
          if (!Read_word_le(Handle, &Snap_offset_Y))
            goto Erreur_lecture_config;
        }
        else
        {
          if (fseek(Handle,Chunk.Size,SEEK_CUR)==-1)
            goto Erreur_lecture_config;
        }
        break;
      default: // Chunk inconnu
        goto Erreur_lecture_config;
    }
  }

  if (fclose(Handle))
    return ERROR_CFG_CORRUPTED;

  return 0;

Erreur_lecture_config:
  fclose(Handle);
  return ERROR_CFG_CORRUPTED;
Erreur_config_ancienne:
  fclose(Handle);
  return ERROR_CFG_OLD;
}


int Save_CFG(void)
{
  FILE*  Handle;
  int  index;
  int  index2;
  int modes_to_save;
  char filename[MAX_PATH_CHARACTERS];
  T_Config_header cfg_header;
  T_Config_chunk Chunk;
  T_Config_shortcut_info cfg_shortcut_info={0,0,0};
  T_Config_video_mode   cfg_video_mode={0,0,0};

  strcpy(filename,Config_directory);
  strcat(filename,"gfx2.cfg");

  if ((Handle=fopen(filename,"wb"))==NULL)
    return ERROR_SAVING_CFG;

  // Ecriture du header
  memcpy(cfg_header.Signature,"CFG",3);
  cfg_header.Version1=VERSION1;
  cfg_header.Version2=VERSION2;
  cfg_header.Beta1   =BETA1;
  cfg_header.Beta2   =BETA2;
  if (!Write_bytes(Handle, &cfg_header.Signature,3) ||
      !Write_byte(Handle, cfg_header.Version1) ||
      !Write_byte(Handle, cfg_header.Version2) ||
      !Write_byte(Handle, cfg_header.Beta1) ||
      !Write_byte(Handle, cfg_header.Beta2) )
    goto Erreur_sauvegarde_config;

  // Enregistrement des touches
  Chunk.Number=CHUNK_KEYS;
  Chunk.Size=NB_SHORTCUTS*sizeof(cfg_shortcut_info);

  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  for (index=0; index<NB_SHORTCUTS; index++)
  {
    cfg_shortcut_info.Number = ConfigKey[index].Number;
    switch(Ordering[index]>>8)
    {
      case 0 :
        cfg_shortcut_info.Key =Config_Key[Ordering[index]&0xFF][0]; 
        cfg_shortcut_info.Key2=Config_Key[Ordering[index]&0xFF][1]; 
        break;
      case 1 :
        cfg_shortcut_info.Key =Buttons_Pool[Ordering[index]&0xFF].Left_shortcut[0]; 
        cfg_shortcut_info.Key2=Buttons_Pool[Ordering[index]&0xFF].Left_shortcut[1]; 
        break;
      case 2 : 
        cfg_shortcut_info.Key =Buttons_Pool[Ordering[index]&0xFF].Right_shortcut[0]; 
        cfg_shortcut_info.Key2=Buttons_Pool[Ordering[index]&0xFF].Right_shortcut[1]; 
        break;
    }
    if (!Write_word_le(Handle, cfg_shortcut_info.Number) ||
        !Write_word_le(Handle, cfg_shortcut_info.Key) ||
        !Write_word_le(Handle, cfg_shortcut_info.Key2) )
      goto Erreur_sauvegarde_config;
  }

  // D'abord compter les modes pour lesquels l'utilisateur a mis une préférence
  modes_to_save=0;
#if defined(__GP2X__)
  index = 0;
#else
  index = 1;
#endif
  for (/**/; index<Nb_video_modes; index++)
    if (Video_mode[index].State==0 || Video_mode[index].State==2 || Video_mode[index].State==3)
      modes_to_save++;

  // Sauvegarde de l'état de chaque mode vidéo
  Chunk.Number=CHUNK_VIDEO_MODES;
  Chunk.Size=modes_to_save * sizeof(cfg_video_mode);

  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
#if defined(__GP2X__)
  index = 0;
#else
  index = 1;
#endif
  for (/**/; index<Nb_video_modes; index++)
    if (Video_mode[index].State==0 || Video_mode[index].State==2 || Video_mode[index].State==3)
    {
      cfg_video_mode.State   =Video_mode[index].State;
      cfg_video_mode.Width=Video_mode[index].Width;
      cfg_video_mode.Height=Video_mode[index].Height;

      if (!Write_byte(Handle, cfg_video_mode.State) ||
        !Write_word_le(Handle, cfg_video_mode.Width) ||
        !Write_word_le(Handle, cfg_video_mode.Height) )
        goto Erreur_sauvegarde_config;
    }

  // Ecriture des données du Shade (précédées du shade en cours)
  Chunk.Number=CHUNK_SHADE;
  Chunk.Size=sizeof(Shade_list)+sizeof(Shade_current);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_byte(Handle, Shade_current))
    goto Erreur_sauvegarde_config;
  for (index=0; index<8; index++)
  {
    for (index2=0; index2<512; index2++)
    {
      if (! Write_word_le(Handle, Shade_list[index].List[index2]))
        goto Erreur_sauvegarde_config;
    }
    if (! Write_byte(Handle, Shade_list[index].Step) ||
      ! Write_byte(Handle, Shade_list[index].Mode) )
    goto Erreur_sauvegarde_config;
  }

  // Sauvegarde des informations du Masque
  Chunk.Number=CHUNK_MASK;
  Chunk.Size=sizeof(Mask_table);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_bytes(Handle, Mask_table,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Stencil
  Chunk.Number=CHUNK_STENCIL;
  Chunk.Size=sizeof(Stencil);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_bytes(Handle, Stencil,256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations des dégradés
  Chunk.Number=CHUNK_GRADIENTS;
  Chunk.Size=sizeof(Gradient_array)+1;
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_byte(Handle, Current_gradient))
    goto Erreur_sauvegarde_config;
  for(index=0;index<16;index++)
  {
    if (!Write_byte(Handle,Gradient_array[index].Start) ||
        !Write_byte(Handle,Gradient_array[index].End) ||
        !Write_dword_le(Handle, Gradient_array[index].Inverse) ||
        !Write_dword_le(Handle, Gradient_array[index].Mix) ||
        !Write_dword_le(Handle, Gradient_array[index].Technique) )
        goto Erreur_sauvegarde_config;
  }

  // Sauvegarde de la matrice du Smooth
  Chunk.Number=CHUNK_SMOOTH;
  Chunk.Size=sizeof(Smooth_matrix);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  for (index=0; index<3; index++)
    for (index2=0; index2<3; index2++)
      if (!Write_byte(Handle, Smooth_matrix[index][index2]))
        goto Erreur_sauvegarde_config;

  // Sauvegarde des couleurs à exclure
  Chunk.Number=CHUNK_EXCLUDE_COLORS;
  Chunk.Size=sizeof(Exclude_color);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
 if (!Write_bytes(Handle, Exclude_color, 256))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations du Quick-shade
  Chunk.Number=CHUNK_QUICK_SHADE;
  Chunk.Size=sizeof(Quick_shade_step)+sizeof(Quick_shade_loop);
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_byte(Handle, Quick_shade_step))
    goto Erreur_sauvegarde_config;
  if (!Write_byte(Handle, Quick_shade_loop))
    goto Erreur_sauvegarde_config;

  // Sauvegarde des informations de la grille
  Chunk.Number=CHUNK_GRID;
  Chunk.Size=8;
  if (!Write_byte(Handle, Chunk.Number) ||
      !Write_word_le(Handle, Chunk.Size) )
    goto Erreur_sauvegarde_config;
  if (!Write_word_le(Handle, Snap_width))
    goto Erreur_sauvegarde_config;
  if (!Write_word_le(Handle, Snap_height))
    goto Erreur_sauvegarde_config;
  if (!Write_word_le(Handle, Snap_offset_X))
    goto Erreur_sauvegarde_config;
  if (!Write_word_le(Handle, Snap_offset_Y))
    goto Erreur_sauvegarde_config;


  if (fclose(Handle))
    return ERROR_SAVING_CFG;

  return 0;

Erreur_sauvegarde_config:
  fclose(Handle);
  return ERROR_SAVING_CFG;
}

// (Ré)assigne toutes les valeurs de configuration par défaut
void Set_config_defaults(void)
{
  int index, index2;

  // Raccourcis clavier
  for (index=0; index<NB_SHORTCUTS; index++)
  {
    switch(Ordering[index]>>8)
    {
      case 0 :
        Config_Key[Ordering[index]&0xFF][0]=ConfigKey[index].Key;
        Config_Key[Ordering[index]&0xFF][1]=ConfigKey[index].Key2;
        break;
      case 1 :
        Buttons_Pool[Ordering[index]&0xFF].Left_shortcut[0] = ConfigKey[index].Key;
        Buttons_Pool[Ordering[index]&0xFF].Left_shortcut[1] = ConfigKey[index].Key2;
        break;
      case 2 :
        Buttons_Pool[Ordering[index]&0xFF].Right_shortcut[0] = ConfigKey[index].Key;
        Buttons_Pool[Ordering[index]&0xFF].Right_shortcut[1] = ConfigKey[index].Key2;
        break;
    }
  }
  // Shade
  Shade_current=0;
  for (index=0; index<8; index++)
  {
    Shade_list[index].Step=1;
    Shade_list[index].Mode=0;
    for (index2=0; index2<512; index2++)
      Shade_list[index].List[index2]=256;
  }
  // Shade par défaut pour la palette standard
  for (index=0; index<7; index++)
    for (index2=0; index2<16; index2++)
      Shade_list[0].List[index*17+index2]=index*16+index2+16;

  Shade_list_to_lookup_tables(Shade_list[Shade_current].List,
            Shade_list[Shade_current].Step,
            Shade_list[Shade_current].Mode,
            Shade_table_left,Shade_table_right);

  // Masque
  for (index=0; index<256; index++)
    Mask_table[index]=0;

  // Stencil
  for (index=0; index<256; index++)
    Stencil[index]=1;

  // Dégradés
  Current_gradient=0;
  for(index=0;index<16;index++)
  {
    Gradient_array[index].Start=0;
    Gradient_array[index].End=0;
    Gradient_array[index].Inverse=0;
    Gradient_array[index].Mix=0;
    Gradient_array[index].Technique=0;
  }
  Load_gradient_data(Current_gradient);

  // Smooth
  Smooth_matrix[0][0]=1;
  Smooth_matrix[0][1]=2;
  Smooth_matrix[0][2]=1;
  Smooth_matrix[1][0]=2;
  Smooth_matrix[1][1]=4;
  Smooth_matrix[1][2]=2;
  Smooth_matrix[2][0]=1;
  Smooth_matrix[2][1]=2;
  Smooth_matrix[2][2]=1;

  // Exclude colors
  for (index=0; index<256; index++)
    Exclude_color[index]=0;

  // Quick shade
  Quick_shade_step=1;
  Quick_shade_loop=0;

  // Grille
  Snap_width=Snap_height=8;
  Snap_offset_X=Snap_offset_Y=0;

}

#ifdef GRAFX2_CATCHES_SIGNALS

#if defined(__WIN32__)
  #define SIGHANDLER_T __p_sig_fn_t
#elif defined(__macosx__)
  #define SIGHANDLER_T sig_t
#else
  #define SIGHANDLER_T __sighandler_t
#endif

// Memorize the signal handlers of SDL
SIGHANDLER_T Handler_TERM=SIG_DFL;
SIGHANDLER_T Handler_INT=SIG_DFL;
SIGHANDLER_T Handler_ABRT=SIG_DFL;
SIGHANDLER_T Handler_SEGV=SIG_DFL;
SIGHANDLER_T Handler_FPE=SIG_DFL;

void Sig_handler(int sig)
{
  // Restore default behaviour
  signal(SIGTERM, Handler_TERM);
  signal(SIGINT, Handler_INT);
  signal(SIGABRT, Handler_ABRT);
  signal(SIGSEGV, Handler_SEGV);
  signal(SIGFPE, Handler_FPE);
  
  switch(sig)
  {
    case SIGTERM:
    case SIGINT:
    case SIGABRT:
    case SIGSEGV:
      Image_emergency_backup();
    default:
    break;
   }
}
#endif

void Init_sighandler(void)
{
#ifdef GRAFX2_CATCHES_SIGNALS
  Handler_TERM=signal(SIGTERM,Sig_handler);
  Handler_INT =signal(SIGINT,Sig_handler);
  Handler_ABRT=signal(SIGABRT,Sig_handler);
  Handler_SEGV=signal(SIGSEGV,Sig_handler);
  Handler_FPE =signal(SIGFPE,Sig_handler);
#endif
}

void Init_brush_container(void)
{
  int i;
  
  for (i=0; i<BRUSH_CONTAINER_COLUMNS*BRUSH_CONTAINER_ROWS; i++)
  {
    int x,y;
    
    Brush_container[i].Paintbrush_shape=PAINTBRUSH_SHAPE_MAX;
    Brush_container[i].Width=0;
    Brush_container[i].Height=0;
    memset(Brush_container[i].Palette,sizeof(T_Palette),0);
    Brush_container[i].Transp_color=0;  
    for (y=0; y<BRUSH_CONTAINER_PREVIEW_WIDTH; y++)
      for (x=0; x<BRUSH_CONTAINER_PREVIEW_HEIGHT; x++)
        Brush_container[i].Thumbnail[y][x]=0;
        
    Brush_container[i].Brush = NULL;
  }
}
