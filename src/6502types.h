#ifndef F6502TYPES_H_INCLUDED
#define F6502TYPES_H_INCLUDED

#if defined(__BEOS__) || defined(__TRU64__)
    #include <inttypes.h>
#else
    #include <stdint.h>
#endif

typedef unsigned int zusize;
typedef uint8_t zuint8;
typedef uint16_t zuint16;
typedef uint8_t zboolean;
typedef unsigned int zuint;
typedef int8_t zsint8;

#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* MARK: - Addresses */

#define Z_6502_ADDRESS_NMI_POINTER   0xFFFA
#define Z_6502_ADDRESS_RESET_POINTER 0XFFFC
#define Z_6502_ADDRESS_IRQ_POINTER   0xFFFE
#define Z_6502_ADDRESS_BRK_POINTER   0xFFFE
#define Z_6502_ADDRESS_STACK	     0x0100

/* MARK: - Values after power on */

#define Z_6502_VALUE_AFTER_POWER_ON_PC 0x0000
#define Z_6502_VALUE_AFTER_POWER_ON_S  0xFD
#define Z_6502_VALUE_AFTER_POWER_ON_P  0x36
#define Z_6502_VALUE_AFTER_POWER_ON_A  0x00
#define Z_6502_VALUE_AFTER_POWER_ON_X  0x00
#define Z_6502_VALUE_AFTER_POWER_ON_Y  0x00

/* MARK: - State storage type */
typedef struct {
	zuint16 pc;
	zuint8	s, p, a, x, y;

	struct {zuint8 irq :1;
		zuint8 nmi :1;
	} internal;
} Z6502State;

#define Z_6502_STATE_PC( object) (object)->pc
#define Z_6502_STATE_S(	 object) (object)->s
#define Z_6502_STATE_P(	 object) (object)->p
#define Z_6502_STATE_A(	 object) (object)->a
#define Z_6502_STATE_X(	 object) (object)->x
#define Z_6502_STATE_Y(	 object) (object)->y
#define Z_6502_STATE_NMI(object) (object)->internal.nmi
#define Z_6502_STATE_IRQ(object) (object)->internal.irq

#define Z_6502_STATE_MEMBER_PC	pc
#define Z_6502_STATE_MEMBER_S	s
#define Z_6502_STATE_MEMBER_P	p
#define Z_6502_STATE_MEMBER_A	a
#define Z_6502_STATE_MEMBER_X	x
#define Z_6502_STATE_MEMBER_Y	y
#define Z_6502_STATE_MEMBER_NMI internal.nmi
#define Z_6502_STATE_MEMBER_IRQ internal.irq

#ifdef __cplusplus
#	define Z_C_SYMBOLS_BEGIN extern "C" {
#	define Z_C_SYMBOLS_END	 }
#else
#	define Z_C_SYMBOLS_BEGIN
#	define Z_C_SYMBOLS_END
#endif

#define Z_INLINE

#define Z_EMPTY_(dummy)
#define Z_EMPTY Z_EMPTY_(.)

#endif
