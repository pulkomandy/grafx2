/* vim:expandtab:ts=2 sw=2:
*/
/* GFX2CRTC - libraw2crtc.h
 * CloudStrife - 20080921
 * Diffusé sous licence libre CeCILL v2
 * Voire LICENCE
 */

#ifndef LIBRAW2CRTC_H
#define LIBRAW2CRTC_H 1

unsigned char * raw2crtc(unsigned short width, unsigned short height, unsigned char mode, unsigned char r9, unsigned long *outSize, unsigned char *r1, unsigned char r12, unsigned char r13);

#endif
