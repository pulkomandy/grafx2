/* vim:expandtab:ts=2 sw=2:
*/
/* GFX2CRTC - libraw2crtc.c
 * CloudStrife - 20080921
 * Diffusé sous licence libre CeCILL v2
 * Voire LICENCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "const.h"
#include "global.h"
#include "struct.h"
#include "loadsave.h"
#include "gfx2log.h"
#include "gfx2mem.h"

/* 6845 registers :
 * R1 : Horizontal Displayed : number of character displayed per line (4, 8 or 16 pixels depending on mode)
 * R6 : Vertical Displayed : Height of displayed screen in characters
 * R9 : Maximum Raster Address : number of line per character-1
 * R12 : Display Start Address (High)
 * R13 : Display Start Address (Low)
 */
static unsigned short addrCalc(unsigned char vcc, unsigned char rcc, unsigned char hcc, unsigned char cclk, unsigned char r1, unsigned char r12, unsigned char r13)
{
  unsigned short MA;
  unsigned short addr;

  //MA = vcc*r1 + hcc + (0x0C)*256;
  MA = vcc*r1 + hcc + r12*256 + r13;
  addr = cclk | ((MA & 0x03FF) << 1); // MA9-MA0 CCLK
  addr = addr | ((rcc & 0x07) << 11); // RA2-RA0 (row address)
  addr = addr | ((MA & 0x3000) << 2); // MA13-MA12

  return addr;
}

unsigned char mode0interlace(T_IO_Context * context, int x, int y)
{
  // bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
  // p0b0 p1b0 p0b2 p1b2 p0b1 p1b1 p0b3 p1b3
  unsigned char mode0pixel[] = {0, 64, 4, 68, 16, 80, 20, 84, 1, 65, 5, 69, 17, 81, 21, 85};
  return mode0pixel[Get_pixel(context,x*2,y) & 0xf] << 1
       | mode0pixel[Get_pixel(context,x*2+1,y) & 0xf];
}

unsigned char mode1interlace(T_IO_Context * context, int x, int y)
{
  unsigned char mode1pixel[] = {0, 16, 1, 17};
  return mode1pixel[Get_pixel(context,x*4,y) & 3] << 3
       | mode1pixel[Get_pixel(context,x*4+1,y) & 3] << 2
       | mode1pixel[Get_pixel(context,x*4+2,y) & 3] << 1
       | mode1pixel[Get_pixel(context,x*4+3,y) & 3];
}

unsigned char mode2interlace(T_IO_Context * context, int x, int y)
{
  unsigned char out = 0;
  int i;
  for(i = 0; i < 8; i++)
    out += ((Get_pixel(context,x*8+7-i,y)&1) << i);
  return out;
}

unsigned char mode3interlace(T_IO_Context * context, int x, int y)
{
  unsigned char mode3pixel[] = {0, 16, 1, 17};
  return mode3pixel[Get_pixel(context, x, y) & 3] << 3 | mode3pixel[Get_pixel(context, x+1, y) & 3] << 2;
}

///@ingroup cpc
unsigned char *raw2crtc(T_IO_Context *context, unsigned char mode, unsigned char r9, unsigned long *outSize, unsigned char *r1, unsigned char r12, unsigned char r13)
{
  unsigned char *outBuffer;
  unsigned char *tmpBuffer;
  unsigned char *allocationBuffer;
  unsigned short minAddr = 0;
  unsigned char minAddrIsDefined = 0;
  unsigned short maxAddr = 0;

  int i, y, x;
  unsigned char r6;
  unsigned char vcc;  // vertical character count
  unsigned char rcc;  // raster count (line within character)
  unsigned char hcc;  // horizontal character count
  unsigned char cclk;

  int width = context->Width;
  int height = context->Height;

  unsigned char (*ptrMode)(T_IO_Context * context, int x, int y);

  switch(mode)
  {
    case 0:
    {
      *r1 = (width+3)/4;
      ptrMode = mode0interlace;
      break;
    }
    case 1:
    {
      *r1 = (width+7)/8;
      ptrMode = mode1interlace;
      break;
    }
    case 2:
    {
      *r1 = (width+15)/16;
      ptrMode = mode2interlace;
      break;
    }
    case 3:
    {
      *r1 = (width+3)/4;
      ptrMode = mode3interlace;
      break;
    }
    default:
    {
      GFX2_Log(GFX2_ERROR, "raw2crtc() mode %d incorrect\n", (int)mode);
      return NULL;
    }
  }

  tmpBuffer = (unsigned char*)GFX2_malloc(0x10000);
  if (tmpBuffer == NULL)
    return NULL;
  memset(tmpBuffer, 0, 0x10000);

  allocationBuffer = (unsigned char*)GFX2_malloc(0x10000);
  if(allocationBuffer == NULL)
  {
    free(tmpBuffer);
    return NULL;
  }
  memset(allocationBuffer, 0, 0x10000);

  r6 = height/(r9+1);

  GFX2_Log(GFX2_DEBUG, "raw2crtc() r1=%u r6=%u r9=%u\n", *r1, r6, r9);

  for(vcc = 0; vcc < r6; vcc++)
  {
    for(rcc = 0; rcc <= r9; rcc++)
    {
      y = vcc*(r9+1) + rcc;
      for(hcc = 0; hcc < *r1; hcc++)
      {
        for(cclk = 0; cclk < 2; cclk++)
        {
          unsigned short addr;
          x = (hcc << 1 | cclk);
          addr = addrCalc(vcc, rcc, hcc, cclk, *r1, r12, r13);
          tmpBuffer[addr] = (*ptrMode)(context, x, y);
          allocationBuffer[addr] += 1;
        }
      }
    }
  }

  for(i = 0; i < 0x10000; i++)
  {
    if(allocationBuffer[i] > 1)
    {
      fprintf(stderr, "WARNING : Multiple writes to memory address 0x%04X\n",i);
    }
    if(allocationBuffer[i] > 0)
    {
      maxAddr = i;
      if(minAddrIsDefined == 0)
      {
        minAddr = i;
        minAddrIsDefined = 1;
      }
    }
  }

  GFX2_Log(GFX2_DEBUG, "raw2crtc() minaddr=%x maxaddr=%x\n", minAddr, maxAddr);
  *outSize = (maxAddr + 1) - minAddr;

  outBuffer = (unsigned char*)GFX2_malloc((*outSize));
  if (outBuffer != NULL)
    memcpy(outBuffer, tmpBuffer + minAddr, *outSize);

  free(tmpBuffer);
  tmpBuffer = NULL;
  free(allocationBuffer);
  allocationBuffer = NULL;
  
  return outBuffer;
}
