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

///@file c64load.c
/// Load C64 .PRG files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "io.h"
#include "c64load.h"
#include "gfx2mem.h"
#include "gfx2log.h"

#define CPU_6502_STATIC
#define CPU_6502_DEPENDENCIES_H "6502types.h"
#define CPU_6502_USE_LOCAL_HEADER
#include "6502.h"

#define MAX_IO_ACCESS_MSG_COUNT 10

/**
 * Check if it is a machine langage program with a BASIC
 * startup line (eg. 10 SYS2061)
 * @return 0 or the machine language code start address
 */
word C64_isBinaryProgram(FILE * f)
{
  word prg_load_addr;
  word addr, next, linenum;
  word len;
  char * line;
  unsigned long start = 0;
  int i;

  if (f == NULL)
    return 0;
  if (fseek(f, 0, SEEK_SET) < 0)
    return 0;
  if (!Read_word_le(f, &prg_load_addr))
    return 0;
  if (prg_load_addr != 0x0801)
    return 0;
  addr = prg_load_addr;
  while (start == 0)
  {
    if (!Read_word_le(f, &next))
      return 0;
    if (next == 0)
      break;
    if (!Read_word_le(f, &linenum))
      return 0;
    GFX2_Log(GFX2_DEBUG, "$%04x %hu\n", next, linenum);
    len = next - addr - 4;
    line = malloc(len);
    if (!Read_bytes(f, line, len))
    {
      free(line);
      return 0;
    }
    GFX2_LogHexDump(GFX2_DEBUG, "", (byte *)line, 0, len);
    if ((byte)line[0] == 0x9e) // SYS BASIC token
    {
      GFX2_Log(GFX2_DEBUG, "SYS%s\n", line + 1);
      i = 1;
      while (line[i] == ' ')
        i++;
      start = strtoul(line + i, NULL, 10);
    }
    free(line);
  }
  return start;
}

static byte C64_mem_read(void *context, word address)
{
  struct c64state * c64 = (struct c64state *)context;
  if ((c64->ram[1] & 2) && address >= 0xe000)
  {
    GFX2_Log(GFX2_WARNING, "** ROM ** read($%04x)\n", address);
    // $FFE4 GETIN
    // make it return 0 (an Z=1) like if no key is pressed
    if (address == 0xffe4)
    {
      c64->keyjoyread++;
      return 0xA9;  // LDA #$xx
    }
    if (address == 0xffe5)
      return 0x00;  // 2nd byte of LDA #$00
    return 0x60;  // RTS
  }
    
  if ((c64->ram[1] & 4) &&
      (((address >= 0xd000) && (address < 0xd800)) || ((address >= 0xdc00) && (address < 0xe000))))
  {
    if ((address & 0xff00) == 0xd000)
    {
      if (c64->ioaccess[address - 0xd000] < MAX_IO_ACCESS_MSG_COUNT)
      {
        GFX2_Log(GFX2_DEBUG, "** IO ** read($%04x)  $%02x\n",
                 address, c64->ram[address]);
        if (++c64->ioaccess[address - 0xd000] == MAX_IO_ACCESS_MSG_COUNT)
          GFX2_Log(GFX2_DEBUG, " stopping debug log on $%04x\n", address);
      }
    }
    else
    {
      GFX2_Log(GFX2_DEBUG, "** IO ** read($%04x)  $%02x\n",
               address, c64->ram[address]);
    }
    if ((address & 0xfffe) == 0xdc00)
      c64->keyjoyread++;
  }
  return c64->ram[address];
}

static void C64_mem_write(void *context, word address, byte value)
{
  struct c64state * c64 = (struct c64state *)context;
  if ((address >= 0xd000 && address < 0xd800) ||
      (address >= 0xdc00 && address < 0xe000))
  {
    if ((address & 0xff00) == 0xd000)
    {
      if (c64->ioaccess[address - 0xd000] < MAX_IO_ACCESS_MSG_COUNT)
      {
        GFX2_Log(GFX2_DEBUG, "** IO ** write($%04x, $%02x)\n", address, value);
        if (++c64->ioaccess[address - 0xd000] == MAX_IO_ACCESS_MSG_COUNT)
          GFX2_Log(GFX2_DEBUG, " stopping debug log on $%04x\n", address);
      }
    }
    else
    {
      GFX2_Log(GFX2_DEBUG, "** IO ** write($%04x, $%02x)\n", address, value);
    }
    switch (address)
    {
      case 0xd011:
        c64->irqrasterline = (c64->irqrasterline & 0x00ff) | (value >> 7);
        break;
      case 0xd012:
        c64->irqrasterline = (c64->irqrasterline & 0xff00) | value;
        break;
      case 0xd018:
        if (c64->ram[0xd011] & 0x10) // Screen is on
          c64->fliscreens[(c64->ram[0xd011] - 50) & 7] = value >> 4;
        break;
      case 0xd019:
        // acknowledge rasterirq
        m6502_irq(c64->cpu, FALSE);
        break;
      case 0xd021:  // background color
        {
          word line = ((word)(c64->ram[0xd011] & 0x80) << 1) | c64->ram[0xd012];
          if (line >= 50 && line < 250)
            c64->backgrounds[line - 50] = value & 0x0f;
        }
        break;
    }
  }
  else if (address >= 0xfffa)
    GFX2_Log(GFX2_DEBUG, "write($%04x, $%02x)\n", address, value);
  else if (address == 0x314 || address == 0x315)
    GFX2_Log(GFX2_DEBUG, "write($%04x, $%02x)\n", address, value);
  c64->ram[address] = value;
}

int C64_LoadPrg(struct c64state * c64, FILE * file, word start)
{
  M6502 cpu;
  zusize cycles = 0;
  zusize next_rasterline = 63;
  int i, count = 0;
  byte screen_min = 255;
  int prg_size;

  if (c64->ram == NULL)
  {
    c64->ram = GFX2_malloc(65536);
    if (c64->ram == NULL)
      return 0;
  }
  memset(c64->ram, 0, 65536);
  prg_size = fread(c64->ram + 0x801, 1, 38911, file);
  GFX2_Log(GFX2_DEBUG, "C64_LoadPrg(%d, $%04x)\n", prg_size, start);
  if (prg_size < 0)
    return 0;
  c64->ram[0x00] = 0x2F;
  c64->ram[0x01] = 0x37;
  c64->ram[0x2B] = 0x01;
  c64->ram[0x2C] = 0x08;
  c64->ram[0x2D] = (0x7ff + prg_size) & 0xff;
  c64->ram[0x2E] = (0x7ff + prg_size) >> 8;
  c64->ram[0x2F] = c64->ram[0x2D];
  c64->ram[0x30] = c64->ram[0x2E];
  c64->ram[0x31] = c64->ram[0x2D];
  c64->ram[0x32] = c64->ram[0x2E];
  c64->ram[0xd011] = 0x1B;
  c64->ram[0xd016] = 0xC8;
  c64->ram[0xd018] = 0x15;
  c64->ram[0xd020] = 0xFE;
  c64->ram[0xd021] = 0xF6;
  c64->ram[0xd022] = 0xF1;
  c64->ram[0xd023] = 0xF2;
  c64->ram[0xd024] = 0xF3;
  c64->ram[0xd025] = 0xF4;
  c64->ram[0xd026] = 0xF0;
  c64->ram[0xd027] = 0xF1;
  c64->ram[0xd028] = 0xF2;
  c64->ram[0xd029] = 0xF3;
  c64->ram[0xd02a] = 0xF4;
  c64->ram[0xd02b] = 0xF5;
  c64->ram[0xd02c] = 0xF6;
  c64->ram[0xd02d] = 0xF7;
  c64->ram[0xd02e] = 0xFC;
  c64->ram[0xdd00] = 0x97;
  c64->cpu = &cpu;

  memset(&cpu, 0, sizeof(cpu));
  cpu.context = (void*)c64;
  cpu.read = C64_mem_read;
  cpu.write = C64_mem_write;

  m6502_power(&cpu, TRUE);
  cpu.state.pc = start;
  while (cycles < 10000000 && c64->keyjoyread < 10)
  {
    word lastpc = cpu.state.pc;
    //cycles += m6502_run(&cpu, next_rasterline > cycles ? next_rasterline - cycles : 1);
    cycles += m6502_run(&cpu, 1);
    if (cycles >= next_rasterline)
    {
      word line = ((word)(c64->ram[0xd011] & 0x80) << 1) | c64->ram[0xd012];
      if (++line >= 312)
        line = 0;
      c64->ram[0xd012] = line & 0xff;
      c64->ram[0xd011] = (c64->ram[0xd011] & 0x7f) | ((line >> 1) & 0x80);
      if (c64->ram[0xd01a] & 1)
      {
        if (line == c64->irqrasterline)
        {
          GFX2_Log(GFX2_DEBUG, "pc=$%04x Raster IRQ line %hu\n", cpu.state.pc, line);
          c64->ram[0xd019] |= 0x81;
          m6502_irq(&cpu, TRUE);
        }
      }
      next_rasterline += 63;
    }
    if (lastpc == cpu.state.pc/* && !(c64->ram[0xd01a] & 1)*/)
    {
      GFX2_Log(GFX2_DEBUG, "infinite loop detected\n");
      if ((c64->ram[0xd011] & 0x10) == 0 // Screen off
          && (c64->ram[0x315] != 0))
      {
        // fake system interrupt
        cpu.state.pc = ((word)c64->ram[0x315] << 8) + c64->ram[0x314];
      }
      else
        break;
    }
  }
  GFX2_Log(GFX2_DEBUG, "%u cycles pc=$%04x\n", cycles, cpu.state.pc);
  c64->vicmode = (c64->ram[0xd011] & 0x60) | (c64->ram[0xd016] & 0x10);
  GFX2_Log(GFX2_DEBUG, "$%02x %s%s%s\n",
           c64->vicmode,
           (c64->ram[0xd011] & 0x20) ? "BITMAP" : "TEXT",
           (c64->ram[0xd011] & 0x40) ? " EXTBKG" : "",
           (c64->ram[0xd016] & 0x10) ? " MULTICOLOR" : "");
  GFX2_Log(GFX2_DEBUG, "fliscreens [%x %x %x %x %x %x %x %x]\n",
           c64->fliscreens[0], c64->fliscreens[1], c64->fliscreens[2], c64->fliscreens[3],
           c64->fliscreens[4], c64->fliscreens[5], c64->fliscreens[6], c64->fliscreens[7]);
  for (i = 0; i < 8; i++)
  {
    if (c64->fliscreens[i] != 0)
      count++;
    if (c64->fliscreens[i] < screen_min)
      screen_min = c64->fliscreens[i];
  }
  if (count > 1)
  {
    GFX2_Log(GFX2_INFO, "FLI MODE DETECTED\n");
    c64->vicmode |= C64_VICMODE_FLI;
  }
  c64->bitmap = c64->screen = ((c64->ram[0xdd00] & 3) ^ 3) << 14;
  c64->bitmap += (c64->ram[0xd018] & 0x0f) << 10;
  c64->bitmap &= ((c64->ram[0xd011] & 0x20) ? 0xe000 : 0xf800);
  if (c64->vicmode & C64_VICMODE_FLI)
    c64->screen += screen_min << 10;
  else
    c64->screen += (c64->ram[0xd018] & 0xf0) << 6;
  GFX2_Log(GFX2_DEBUG, "$D018=$%02x bitmap at $%04x, screen at $%04x\n",
           c64->ram[0xd018], c64->bitmap, c64->screen);
  return 1;
}
