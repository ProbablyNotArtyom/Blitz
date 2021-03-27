#if 0
;  first.S  -  LILO first stage boot loader with LBA32 support */
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

#endif


#define JRC_NOCOMPACT

#define LILO_ASM
#include "lilo.h"
get common.s		/* as86 "include" will bypass the CPP */



	.text

	.globl	_main

	.org	0

_main:
zero:
	cli
	call	start
reloc:	.word	theend-zero	! size of the code & params

	.org	6

! Boot device parameters. They are set by the installer.

	.ascii	"LILO"
	.word	STAGE_FIRST
	.word	VERSION

#if 0
	.blkb	BPB_OFFSET_asm

main2:	cli			! NT 4 blows up if this is missing
	jmpi	start,BOOTSEG
#endif

port:	.byte	0		! COM port (0 = unused, 1 = COM1, etc.)
sparam:	.byte	0		! serial port parameters (0 = unused)

raid:	.long	0		! raid sector offset
tstamp:	.long	0		! timestamp

timeout:.word	0		! input timeout
delay:	.word	0		! boot delay

ms_len:	.word	0		! initial greeting message
ms_cx:	.word	0
ms_dx:	.word	0
ms_al:	.byte	0		! (unused)

d1_cx:	.word	0		! first descriptor sector address
d1_dx:	.word	0
d1_al:	.byte	0		! (unused)

d2_addr: .blkb	MAX_DESCR_SECS_asm*sa_size - sa_size

dc_cx:	.word	0		! default command-line sector address
dc_dx:	.word	0
dc_al:	.byte	0		! (unused)

prompt:	.byte	0		! indicates whether to always enter prompt
				! (also used as alignment byte)

kt_cx:	.word	0		! keyboard translation table
kt_dx:	.word	0
kt_al:	.byte	0

d_addr:	.word	0		! second stage sector map of addresses
	.word	0
	.byte	0

	.word	0,0		! terminate the chain

;;;	.org	CODE_START_1
#if 0
!  These locations are referenced as EX_OFF and must be at CODE_START_1
ext_si:	.word	0		! external interface
ext_es:	.word	0		! these locations are referenced in second.S
ext_bx:	.word	0		! do not disturb the ordering
ext_dl:	.byte	0		! second.S will check this magic number
ext_dh:	.byte	0		! not referenced, but must align stack
ext_stack:
#endif
	
start:
	pop	ax		! get reloc source from old stack
	mov	di,cs
	shr	ax,#4		! make into paragraph offset
	add	ax,di		! relocate the segment
	mov	ss,ax
	mov	sp,#SETUP_STACKSIZE	! set the stack for First Stage
	sti			! now it is safe

	push	dx		! set ext_dl (and ext_dh, which is not used)
	push	bx		! WATCH the order of pushes
	push	es		! set ext_es
	push	si		! set ext_si

#define JRC_DS_EQ_SS

	cld			! do not forget to do this !!!
	mov	ds,ax		! address data area
	mov	es,ax		! address data area

	mov	al,#0x0d	! gimme a CR ...
	call	display
	mov	al,#0x0a	! ... an LF ...
	call	display
	mov	al,#0x4c	! ... an 'L' ...
	call	display
	
lagain:
	mov	si,#d_addr	! ready to load the second stage loader
	mov	bx,#map2	! read second stage map to ES:BX
	push	bx		! save for later in SI
	call	pread		! read using pointer in DS:SI
	pop	si		! point at #map2

	int	0x12		! get memory available
	shl	ax,#6		! convert to paragraphs
	sub	ax,#DATAEND>>4+0x20	! allow for PARMLINE
	push	ax
	pop	es
	xor	bx,bx

sload:	call	pread		! read using map at DS:SI
	jnz	sload		! into memory at ES:BX (auto increment)

rdone:	mov	al,#0x49	! display an 'I'
	call	display

! Start the second stage loader     DS=location of Params

	push	es
;;;	push	#0
	push	bx		! display clobbered BH to zero!!!
	retf

pread:	lodsw			! get CX
	xchg	cx,ax
	
        lodsb
	test    al,#LINEAR_FLAG|LBA32_FLAG
        jnz     use_linear

        dec     si
	lodsw
	mov     dx,ax
	or      ax,cx
	jz      done
	lodsb

        mov     ah,#2           ! read command
        int     0x13            ! BIOS read
        jmp     rd_done

use_linear:
	xchg	dx,ax		! was mov dl,al
	lodsw
	test    dl,#LBA32_FLAG
	jnz     is_lba
	xor     ah,ah           ! was LINEAR, zero the hi-nibble (was count)
is_lba:
        xchg    ax,di
	test	dl,#RAID_REL_FLAG
	jz	skip_reloc
	add	cx,raid		! **** RAID *****
	adc	di,raid+2	! **** RAID *****
skip_reloc:

	call	lba_read
rd_done:
	jc	error		! error -> start over again
	add	bh,#2    	! next sector
done:	ret

error:
#ifndef LCF_NO1STDIAG
        mov	al,#32          ! display a space
	call	display
;;;		        ! display error code in AH
	call	bout
#endif
	xor	ax,ax		! reset the FDC
;;;	mov	dl,al
	int	0x13
	pop	ax		! pop return address to  pread
	jmp	lagain		! redo from start

#ifndef LCF_NO1STDIAG
bout:	rol     ax,4            ! bring hi-nibble to position
	call	nout
	rol     ax,4            ! bring lo-nibble to position
nout:	and	al,#0x0F	! display one nibble
#if 0
	add	al,#0x30
	cmp	al,#0x3A	! a..f converts to letter
	jb	nokay
	add	al,#7		! make a letter A..F
nokay:				! fall through
#else
	daa			! shorter conversion routine
	add	al,#0xF0
	adc	al,#0x40	! is now a hex char 0..9A..F
#endif
#endif
display:  push  ax              ! new display does not affect AX
        xor	bh,bh		! display on screen
	mov	ah,#14
	int	0x10
	pop     ax
	ret			! side effect, BH=0
	
#include "read.S"

theend:

!
!   If 'first' loads as the MBR, then there must be space for the partition
!   table.  If 'first' loads as the boot record of some partition, then
!   the space reserved below is not used.  But we must reserve the area
!   as a hedge against the first case.
!
!
	.org	MAX_BOOT_SIZE	!
	.word	0,0,0,0		! space for NT and DRDOS dirty hacks

!!!	.org	0x1be		! spot for the partition table
p_table:
	.blkb	16
	.blkb	16
	.blkb	16
	.blkb	16
	.word	0xAA55		! boot block signature

map2:
