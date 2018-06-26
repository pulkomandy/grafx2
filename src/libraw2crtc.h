/* vim:expandtab:ts=2 sw=2:
*/
/* GFX2CRTC - libraw2crtc.h
 * CloudStrife - 20080921
 * Diffusé sous licence libre CeCILL v2
 * Voire LICENCE
 */

#ifndef LIBRAW2CRTC_H
#define LIBRAW2CRTC_H 1

#include "loadsave.h"

unsigned char * raw2crtc(T_IO_Context* context, unsigned char mode, unsigned char r9, unsigned long *outSize, unsigned char *r1, unsigned char r12, unsigned char r13);

#endif
