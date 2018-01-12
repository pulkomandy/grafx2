/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program
 *  Gif Analyzer tool

    Copyright 2018 Thomas Bernard

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
#include <SDL.h>
#include <SDL_image.h>

SDL_Surface * screen = NULL;

int Show_IMG(const char * filename)
{
  SDL_Surface * image;
  SDL_Rect dest;

  image = IMG_Load(filename);
  if(image == NULL) {
    fprintf(stderr, "Cannot load image \"%s\"\n", filename);
    return -1;
  }
  if(screen == NULL) {
    screen = SDL_SetVideoMode(image->w, image->h, image->format->BitsPerPixel, 0);
  }
  dest.x = 0;
  dest.y = 0;
  dest.w = image->w;
  dest.h = image->h;
  if(screen != NULL) {
    SDL_BlitSurface(image, NULL, screen, &dest);
    SDL_UpdateRects(screen, 1, &dest);
  } else {
    fprintf(stderr, "no SDL screen open\n");
  }
  SDL_FreeSurface(image);
  return 0;
}

void Manage_Events(void)
{
  SDL_Event event;

  for(;;) {
    SDL_WaitEvent(&event);
    switch(event.type) {
    case SDL_QUIT:
      return;
    case SDL_KEYDOWN:
      if(event.key.keysym.sym == SDLK_ESCAPE) return; /* ESC to quit */
      break;
    case SDL_MOUSEBUTTONDOWN:
      return; /* clic to quit */
    default:
      break;
    }
  }
}

int main(int argc, char * * argv)
{
  const char * filename;
  if(argc < 2) {
    fprintf(stderr, "Usage:\t%s <image>\n", argv[0]);
    return 1;
  }
  filename = argv[1];

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
	  fprintf(stderr, "*** SDL Init Error ***\n");
	  return 1;
  }
  atexit(SDL_Quit);

  SDL_WM_SetCaption(filename, filename); /* window caption, icon caption */

  if(Show_IMG(filename) < 0)
    return 1;
  Manage_Events();
  return 0;
}
