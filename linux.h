#ifndef _LINUX_H_
#define _LINUX_H_

#ifdef __linux__

struct find_t {
  unsigned char attrib;
  char name[256];
};

int filelength(int);

void itoa(int source,char* dest, int longueur);
/* Integer TO Ascii */

#endif

#endif
