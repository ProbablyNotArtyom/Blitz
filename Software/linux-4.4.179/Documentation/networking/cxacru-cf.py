/* sha1-armv7-neon.S - ARM/NEON accelerated SHA-1 transform function
 *
 * Copyright © 2013-2014 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

.syntax unified
.fpu neon

.text


/* Context structure */

#define state_h0 0
#define state_h1 4
#define state_h2 8
#define state_h3 12
#define state_h4 16


/* Constants */

#define K1  0x5A827999
#define K2  0x6ED9EBA1
#define K3  0x8F1BBCDC
#define K4  0xCA62C1D6
.align 4
.LK_VEC:
.LK1:	.long K1, K1, K1, K1
.LK2:	.long K2, K2, K2, K2
.LK3:	.long K3, K3, K3, K3
.LK4:	.long K4, K4, K4, K4


/* Register macros */

#define RSTATE r0
#define RDATA r1
#define RNBLKS r2
#define ROLDSTACK r3
#define RWK lr

#define _a r4
#define _b r5
#define _c r6
#define _d r7
#define _e r8

#define RT0 r9
#define RT1 r10
#define RT2 r11
#define RT3 r12

#define W0 q0
#define W1 q7
#define W2 q2
#define W3 q3
#define W4 q4
#define W5 q6
#define W6 q5
#define W7 q1

#define tmp0 q8
#define tmp1 q9
#define tmp2 q10
#define tmp3 q11

#define qK1 q12
#define qK2 q13
#define qK3 q14
#define qK4 q15

#ifdef CONFIG_CPU_BIG_ENDIAN
#define ARM_LE(code...)
#else
#define ARM_LE(code...)		code
#endif

/* Round function macros. */

#define WK_offs(i) (((i) & 15) * 4)

#define _R_F1(a,b,c,d,e,i,pre1,pre2,pre3,i16,\
	      W,W_m04,W_m08,W_m12,W_m16,W_m20,W_m24,W_m28) \
	ldr RT3, [sp, WK_