/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program
 *  Gif Analyzer tool

    Copyright 2010 Adrien Destugues

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
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main(int argc, char* argv[])
{
	FILE* theFile;

	if(argc != 2)
	{
		printf("%s file.gif\n",argv[0]);
		exit(-1);
	}

	theFile = fopen(argv[1], "r");

	uint8_t buffer[256];

	fread(buffer, 1, 6, theFile);
  buffer[6] = 0;

	printf("Signature: %6s\n", buffer);

	uint16_t w,h;

	fread(&w,2,1,theFile);
	fread(&h,2,1,theFile);

	printf("Resolution: %dx%d\n",w,h);

  fread(buffer,1,3,theFile);

  int colors = ((buffer[0]&0b01110000)>>4)+1;
  colors = pow(2,colors);

  int bpp = (buffer[0]&0xF) +1;

  printf("Color palette: %#02.2x\n",buffer[0]&0xFF);
  if (buffer[0] & 0b10000000)
    printf("\tGlobal palette\n");
  printf("\tColor count: %d\n", colors); 
  printf("\tBits per pixel: %d\n",bpp);

  printf("Index of background color: %d\n",buffer[1]);
  printf("(reserved byte: %d)\n",buffer[2]);

  printf("Color palette:\n");
  for (int i = 0; i < pow(2,bpp); i++)
  {
    fread(buffer,1,3,theFile);
    printf("\t%d: %u %u %u\t",i,buffer[0], buffer[1], buffer[2]);
    if ((i+1)%4 ==0)puts("");
  }

  int i = 0;
  do {
    fread(buffer,1,1,theFile);
    i++;
  } while (i != ',');

  if (i > 1);
  printf("Skipped %d meaningless bytes before image descriptor\n",i);

  uint16_t x,y;

	fread(&x,2,1,theFile);
	fread(&y,2,1,theFile);
	fread(&w,2,1,theFile);
	fread(&h,2,1,theFile);

  printf("Image descriptor\n");
  printf("\tx=%d y=%d w=%d h=%d\n",x,y,w,h);

	fclose(theFile);
}
