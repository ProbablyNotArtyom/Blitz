#if 0
/* second.S  -  LILO second stage boot loader */
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

#endif

#define RAID_NEW
#define PIXADDRESS
#define MEMORY_CHECK
#define LOCK_BSS
#define VAR_LOAD


#define LILO_ASM
#include "lilo.h"
get common.s		/* as86 "include" will bypass the CPP */

#if 0
DESCR0 = DESCR+2
#else
DESCR0 = DESCR
#endif

#ifdef DEBUG
#define DEBUG_NEW 1
#else
/* the following gets changed to 0 for release: */
#if VERSION_MINOR<50
#define DEBUG_NEW 0
#else
#define DEBUG_NEW 1
#ifndef MEMORY_CHECK
#define MEMORY_CHECK
#endif
#endif		/* VERSION_MINOR */
#endif		/* DEBUG */

#ifdef LCF_M386
#define BEG_FS
#define SEG_FS seg fs
#define END_FS
#else
#define BEG_FS db 0x1e,0x2e,0x8e,0x1e,0x02,0x00
/*	push ds; \
	seg cs; \
	mov	ds,firstseg */
#define SEG_FS
#define END_FS pop ds
#endif

#define UI_MAGIC 0xff		/* take input from keyboard */

#ifdef MENU
STAGE_MENU	=	STAGE_FLAG_MENU
#else
STAGE_MENU	=	0
#endif

#ifdef BITMAP
STAGE_BITMAP	=	STAGE_FLAG_BMP4
  #if 0
#ifndef LCF_NOSERIAL
#define LCF_NOSERIAL
#endif
  #endif
#else
STAGE_BITMAP	=	0
#endif

#ifdef LCF_NOSERIAL
STAGE_SERIAL	=	0
#else
STAGE_SERIAL	=	STAGE_FLAG_SERIAL
#endif

	.text

	.globl	_main
	.org	0

_main:	jmp	start

#ifndef LCF_M386
firstseg:	dw	0
#endif

	.org	6

! Boot device parameters. They are set by the installer.

sig:	.ascii	"LILO"
stage:	.word	STAGE_SECOND|STAGE_SERIAL|STAGE_MENU|STAGE_BITMAP
version:.word	VERSION

;;;	.org	CODE_START_2

start:	cld			! only CLD in the code; there is no STD
#ifdef LCF_M386
	push	ds
	pop	fs		! address parameters from here
#else
	seg	cs
	mov	firstseg,ds	! save DS here
#endif
#ifndef LCF_NOSERIAL
;;;	seg	ss
	mov	dx,par1_port+SSDIFF	! use a COM port ?
			! watch out, loads par1_ser_param
	dec	dl
	js	nocom		! no -> go on
	xor	ax,ax		! initialize the serial port
	xchg	al,dh

	push	ax
	push	dx

;;;	or	al,#0x06	! stop bits = 2, nbits = 7 or 8
				! this OR is not needed yet (21.7)
	int	0x14		! Communications Port INIT

	push	#0x40
	pop	ds

	pop	bx		! was DX

	shl	bx,#1
	mov	dx,(bx)		! get the port address from the BIOS

	seg	cs		! keep it
	mov	slbase,dx

	pop	bx		! special baud rate test -- was AX

	test	bl,#0x04		! stop bits == 2?
	cli			! do not disturb any code below
	jz	stdbps		! standard BPS

	shr	bx,#5		! index divisor array
	seg	cs
	mov	bl,divisor(bx)

spcbps:				! CLI: do not disturb ...
	push	dx		! save base address
	add	dx,#3		! enable divisor latch access
	in	al,dx
	or	al,#0x80
	out	dx,al
	pop	dx		! set new divisor
	push	dx
	xchg	ax,bx
	out	dx,al
	inc	dx
	mov	al,ah
	out	dx,al
	inc	dx		! disable divisor latch access
	inc	dx
	xchg	ax,bx
	and	al,#0x7f
	out	dx,al
	pop	dx		! restore base address

stdbps: 			! CLI: redundant if fell in from above
	push	dx
	add	dx,#4		! address Modem Control Reg.
#if 0
	in	al,dx
	or	al,#3		! turn on DTR and RTS
#else
	mov	al,#3		! turn on DTR and RTS
#endif
	out	dx,al
	pop	dx
	sti			! done
	
	mov	cx,#32		! drain the queue (if any)
drain:	in	al,dx
	loop	drain
	add	dx,#5		! clear the status register
	in	al,dx

    ! send "\r\nLI" to the serial port

	mov	si,#serLI
	mov	cx,#4
ser1:	seg	cs
	lodsb
	call	serdisp
	loop	ser1
	jmp	comcom		! proceed with the rest of "LILO"

serLI:	.byte	13,10,0x4c,0x49		! cr,lf,"LI"

BAUD_BASE = 115200			! divisor == 1
divisor:
	.byte	BAUD_BASE / 19200	! must be same as bsect.c table
	.byte	BAUD_BASE / 38400
	.byte	BAUD_BASE / 57600
	.byte	BAUD_BASE / 115200
	.byte	BAUD_BASE / 2400
	.byte	BAUD_BASE / 2400
	.byte	BAUD_BASE / 2400
	.byte	BAUD_BASE / 2400

#endif		/* LCF_NOSERIAL */

nocom:
#ifndef LCF_NODRAIN
	mov	cx,#32		! drain type-ahead buffer ?
drkbd:	mov	ah,#1		! is a key pressed ?
	int	0x16
	jz	comcom		! no -> done
	xor	ah,ah		! get the key
	int	0x16
	loop	drkbd
#endif

comcom:
/*#ifdef LCF_BDATA*/
#if 0
	push	cs
	pop	ds

	mov	al,#0x73	! display an 's'
	call	display

	call	io_biosdata

	mov	bx,#bs
	call	say
#endif
	mov	al,#0x4c	! display an 'L'
	call	display
	push	#0		! get pointer to disk parameter table in DS:SI
	pop	ds
	lds	si,[0x78]	! 0x78 = 4*0x1E
#ifndef LCF_XL_SECS
	cmp	byte ptr (si+4),#9 ! okay ?
	ja	dskok		! yes -> do not patch
#endif
	push	cs		! get pointer to new area in ES:DI
	pop	es
	mov	di,#dskprm
	mov	cx,#6		! copy 12 bytes
	rep
	movsw
	seg	es		! patch number of sectors
#ifndef LCF_XL_SECS
	mov	byte ptr (di-8),#18
#else
	mov	byte ptr (di-8),#LCF_XL_SECS
#endif
	push	#0
	pop	ds
	cli			! paranoia
	mov	[0x78],#dskprm
	mov	[0x7a],es
	sti
dskok:
#ifndef	LCF_NOSERIAL
	seg	cs		! clear the break flag
	mov	byte ptr break,#0
#endif
	call	instto		! get timer interrupt
;;;	jmp	restrt		! get going

! Restart here after a boot error

restrt: mov	bx,cs		! adjust segment registers
	mov	ds,bx
	mov	es,bx
#ifdef VAR_LOAD
	sub	bx,#MAX_SETUPSECS*0x20+0x20	! segment for setup code &
						!   bootsect
	mov	cx,#INITSEG
	cmp	bx,cx
	jbe	restrt1
	mov	bx,cx		! BX is the smaller segment #
restrt1:
	mov	[initseg],bx	! set up INITSEG (was 0x9000)
	lea	cx,(bx+0x20)
	mov	[setupseg],cx	! set up SETUPSEG (was 0x9020)
	mov	cx,cs
	sub	cx,bx		! subtract [initseg]
	shl	cx,#4		! get stack size
	mov	ss,bx		! must lock with move to SP below
#else
	mov	cx,bx
	shl	cx,#4
	mov	ax,#STACKSEG
	mov	ss,ax
#endif
	mov	sp,cx		! data on the stack)
#ifndef LCF_M386
	cmp	word [sig],#EX_MAG_L	! check that this is really loaded at the
				! right place ...  "LI"
	jne	crshbrn2
	cmp	word [sig+2],#EX_MAG_H		! "LO"
	jne	crshbrn
	cmp	word [mcmdbeg+6],#0x414d	! "MA"
	jne	crshbrn
	cmp	word [mcmdbeg+8],#0x4547	! "GE"
#else
	cmp	dword [sig],#EX_MAG_HL	! "LILO"
	jne	crshbrn
	cmp	dword [mcmdbeg+6],#0x4547414d	; "MAGE"  from BOOT_IMAGE
#endif
	jne	crshbrn
	cmp	BYTE stage,#STAGE_SECOND
	jne	crshbrn
	cmp	WORD version,#VERSION
crshbrn2: jne	crshbrn
	mov	cmdbeg,#acmdbeg	! probably unattended boot
ldsc:
	mov	bx,#DESCR
#ifdef RAID_NEW
	call	raid_search
#endif
#if 0
	BEG_FS
	SEG_FS
	mov	cx,par1_descr+SSDIFF
	SEG_FS
	mov	dx,par1_descr+2+SSDIFF
        SEG_FS
	mov	al,par1_descr+4+SSDIFF     ;;;
	END_FS
	call	cread
	jc	fdnok1		! error -> retry
	mov	bx,#DESCR+SECTOR_SIZE
	BEG_FS
	SEG_FS
	mov	cx,par1_descr+sa_size+SSDIFF
	SEG_FS
	mov	dx,par1_descr+sa_size+2+SSDIFF
        SEG_FS               
	mov	al,par1_descr+sa_size+4+SSDIFF
	END_FS
	call	cread
	jc	fdnok1		! error -> retry
#else
	mov	si,#par1_descr+SSDIFF	! load the entire descriptor table
descr_more:
	BEG_FS
	SEG_FS
	lodsw	
	xchg	cx,ax
	SEG_FS
	lodsw
	xchg	dx,ax
	SEG_FS
	lodsb
	END_FS
	call	cread
	jc	fdnok1		! error -> retry
	add	bh,#2		! increment address
	cmp	si,#par1_descr+sa_size*MAX_DESCR_SECS_asm+SSDIFF
	jb	descr_more
#endif
	mov	si,#DESCR	! compute a checksum of the descriptor table
	mov	di,#SECTOR_SIZE*MAX_DESCR_SECS-4
#ifdef LCF_M386
	push    dword #CRC_POLY1
#else
	push	#CRC_POLY1>>16
	push	#CRC_POLY1&0xFFFF
#endif
	call	crc32
	add	di,si
#ifdef LCF_M386
	cmp	eax,dword (di)
#else
	cmp	ax,word (di)
	jne	chkerr
	cmp	dx,word (di+2)	
#endif
	jz	nochkerr

! Checksum error
chkerr:
	mov	bx,#msg_chkerr
	jmp	zz		! go wait

! Die

crshbrn:
	mov	bx,#msg_sigerr	! signature not found
zz:	call	say
zzz:	hlt			! wait for interrupt
	jmp	zzz		! sit here forever


nochkerr:
#ifdef DEBUG
	pusha
	mov	bx,#nochker_msg
	call	say
	popa
	jmp	nochkerr1
nochker_msg:
	.ascii	"Descriptor checksum okay\n"
	.byte	0
nochkerr1:
#endif
#if defined(MENU) || defined(BITMAP)
	xor	bx,bx		! defaults are all zero
	mov	[dimage],bx	! set default image to boot
	mov	[abs_cx],bx	! upper left of scroll area
				! means screen is not cleared
#endif
	BEG_FS
	SEG_FS		! load the keyboard translation table
	mov	cx,par1_keytab+SSDIFF		;MSG_OFF+SSDIFF+7
	SEG_FS
	mov	dx,par1_keytab+2+SSDIFF		;MSG_OFF+SSDIFF+9
        SEG_FS                      ;;;
	mov	al,par1_keytab+4+SSDIFF		;MSG_OFF+SSDIFF+11    ;;;
	END_FS
	mov	bx,#KEYTABLE
	call	cread
fdnok1:	jc	fdnok2
#ifdef MENU
	inc	bh			; bx -> KEYTABLE+256
#ifdef LCF_M386
	cmp	dword (bx),#0x554e454d	; "MENU"
	jne	noschema
	mov	edx,(bx+4)
	mov	[mn_attrib],edx
#else
	cmp	(bx),#0x454d
	jne	noschema
	cmp	(bx+2),#0x554e
	jne	noschema
	mov	dx,(bx+4)
	mov	[mn_attrib],dx
	mov	cx,(bx+6)
	mov	[mn_attrib+2],dx
#endif
noschema:
	add	bx,#9		; point at possible title
	mov	al,(bx-1)	; get length stored by installer
	or	al,al
	jz	notitle		; no title if supplied length is 0
	cbw
	xchg	ax,cx		; supplied length to CX
	call	strlen
	cmp	ax,cx
	jne	notitle
	cmp	ax,#str_title_len
	jae	notitle
	push	di
	mov	di,#str_title	;
titlemov:
	mov	al,(bx)
	inc	bx
	seg	ds
	  stosb
;;;	mov	(di),al
;;;	inc	di
	or	al,al
	jnz	titlemov

	pop	di
notitle:
#endif
	mov	bx,#DFLCMD
	BEG_FS
	SEG_FS
	mov	cx,par1_dflcmd+SSDIFF		;DFCMD_OFF
	SEG_FS
	mov	dx,par1_dflcmd+2+SSDIFF
	SEG_FS                      ;;;
	mov	al,par1_dflcmd+4+SSDIFF   ;;;
	END_FS
	call	cread
fdnok2:	jc	fdnok		! error -> retry
	mov	bx,#DFLCMD
	cmp	word ptr (bx),#DC_MAGIC ! okay ?
	jne	bdcmag		! no -> do not write
#ifndef LCF_READONLY
	mov	word ptr (bx),#DC_MGOFF ! erase the magic number
	call	cmd_write	! write out the command line
#endif
	jmp	dokay		! continue
bdcmag:	mov	byte ptr (bx+2),#0 ! disable the command line
	jmp	dokay		! go on
fdnok:	
#if     0
        xor	ax,ax		! reset FDC
	mov	dl,al
	int	0x13
#endif
	br	ldsc		! retry

! List all known boot images

list:	mov	byte ptr (bx),#0 ! set EOL marker
	call	crlf
#ifdef MENU
	inc	word [suppress]	! suppress console output
#endif
	mov	si,#DESCR0	! list all images
	mov	cx,#IMAGES
	xor	dl,dl		! DL counts the images
lloop:	testb	(si),#0xff	! done ?
	jz	ldone		! yes
	mov	bx,si		! display the name
	call	say
	add	si,#MAX_IMAGE_NAME
	inc	dl		! count the image
	test	dl,#3		! inside line -> go on
	jnz	fill
	call	crlf
	jmp	imgdne		! next image
fill:	push	bx		! fill with spaces
	mov	al,#0x20
	call	display
	pop	bx
	inc	bx
	cmp	bx,si
	jbe	fill
imgdne:	add	si,#id_size-MAX_IMAGE_NAME
	loop	lloop		! next image
ldone:	test	dl,#3		! already at BOL ?
	jz	atbol		! yes -> no CRLF
	call	crlf
atbol:	
#ifdef MENU
	dec	word [suppress]
#endif
	br	iloop		! done

! Ready to process user input

dokay:	mov	bx,#ospc	! display 'O '
	call	say
	mov	ospc,#0		! disable the message
	mov	word ptr vgaovr,#VGA_NOCOVR ! disable VGA override
	mov	word ptr memlim,#0 ! no memory limit
	xor	ax,ax		! get the delay and disable further delays
	BEG_FS
	SEG_FS
	xchg	ax,par1_delay+SSDIFF		;DSC_OFF-8+SSDIFF
	END_FS
	or	old_del,ax	! remember delay
	mov	nodfl,#iloop	! interactive prompt if falling through
	BEG_FS
	SEG_FS		! enter boot prompt ?
	test	byte ptr par1_prompt+SSDIFF,#FLAG_PROMPT	;DSC_OFF+15+SSDIFF,#0
	END_FS
	jnz	extp		! yes -> check for external parameters
	mov	nodfl,#bfirst	! boot first image if falling through
	call	waitsh		! wait for a shifting key
	jc	iloop		! key pressed -> enter interactive mode

! Check for external parameters

extp:	BEG_FS
	SEG_FS		! external parameters ?
	cmp	byte ptr EX_OFF+6,#EX_DL_MAG
	END_FS
	jne	noex		! no -> go on
	BEG_FS
	SEG_FS		! clear flag
	mov	byte ptr EX_OFF+6,#0
	SEG_FS		! load the signature pointer
	les	bx,EX_OFF
	END_FS
#ifndef LCF_M386
	seg	es		! "LI" ?
	cmp	word ptr (bx),#EX_MAG_L
	jne	noex		! no -> go on
	seg	es		! "LO" ?
	cmp	word ptr (bx+2),#EX_MAG_H
#else
	seg	es
	cmp	dword ptr (bx),#EX_MAG_HL	! "LILO"
#endif
	jne	noex		! no -> go on
	BEG_FS
	SEG_FS
	mov	si,EX_OFF+4	! pointer to the command line
	END_FS
	seg	es
	cmp	byte ptr (si),#0 ! empty ?
	je	iloop		! yes -> enter interactive mode
	jmp	niloop		! enter non-interactive mode

! No external parameters after timeout -> boot first image

noex:	push	cs		! restore ES
	pop	es
	mov	si,#DFLCMD+2	! default command line ?
	cmp	byte ptr (si),#0
	jne	niloop		! yes -> use it
	mov	ax,nodfl	! no idea how to tell as86 to do jmp (addr) :-(
	jmp	ax		! fall through


! Command input processor

iloop:
#if defined(MENU) || defined(BITMAP)
	call	menu_setup
#endif

#ifndef BITMAP
	BEG_FS
	SEG_FS		! message disabled ?
	cmp	word ptr par1_msg_len+SSDIFF,#0		;MSG_OFF+SSDIFF,#0
	END_FS
	je	nomsg		! yes -> skip this
	call	crlf
	BEG_FS
	SEG_FS		! load the message file
	mov	cx,par1_msg+SSDIFF			;MSG_OFF+SSDIFF+2
	SEG_FS
	mov	dx,par1_msg+2+SSDIFF
        SEG_FS
	mov	al,par1_msg+4+SSDIFF
	END_FS
	mov	bx,#MAP
	call	sread
	call	loadfile

	push	#SYSSEG
	pop	ds
	xor	bx,bx		! set the terminating NUL and disable further
				! messages
	BEG_FS
	SEG_FS
	xchg	bx,par1_msg_len+SSDIFF		;MSG_OFF+SSDIFF
	END_FS
	mov	byte ptr (bx),#0
	xor	bx,bx		! display the message
	call	say

	push	cs		! restore segment registers
	pop	ds
	push	cs
	pop	es
#endif

nomsg:	mov	cmdbeg,#acmdbeg	! probably unattended boot
	mov	si,#usrinpm	! interactive mode
niloop:				! ES may point to external params
	mov	bx,#msg_p	! display boot prompt
	call	say
	mov	bx,#cmdline	! move cursor to the end of the line
clend:	mov	al,(bx)
	or	al,al		! at end ?
	jz	cledne		! yes -> go on
	push	bx		! display the character
	call	display
	pop	bx
	inc	bx		! next one
	jne	clend
cledne:	mov	byte ptr prechr,#32 ! character before command line is a space

! Input loop

input:	seg	es		! interactive mode ?
	cmp	byte ptr (si),#UI_MAGIC
	je	kbinp		! yes -> get keyboard input
	seg	es		! get non-interactive input
	mov	al,(si)
	inc	si
	jmp	gotinp		! go on
	
tolist:
#ifdef BITMAP
	call	menu_exit
#endif
	br	list		! ...

kbinp:	mov	cx,#brto	! get a key
	call	getkey
#ifdef BITMAP
	cmp	byte [abs_cx+1],#0
	je	noNull		! skip cursor keys after Tab
#endif
#if defined(MENU) || defined(BITMAP)
	cmp	al,#0xE0	! extended keyboard
	je	toNull
	or	al,al		!
#ifdef LCF_M386
toNull:	je	near null	! cursor control
#else
	jne	gotinp
toNull:	br	null		! null: in crt.S or bitmap.S
#endif
#endif
#ifndef MENU
noNull:	or	al,al		! keyboard NUL input?
	je	input		! yes, skip Keyboard NUL
; stored command line NUL is handled differently
#endif
gotinp:	cmp	al,#9		! TAB ?
	je	tolist		! yes -> list images
	cmp	al,#63		! "?" ?
	je	tolist		! yes -> list images
	or	al,al		! NUL ?
	je	nul		! yes -> go on
	cmp	al,#8		! BS ?
	je	todelch		! yes -> erase one character
	cmp	al,#13		! CR ?
	je	cr		! yes -> go on
	cmp	al,#127		! DEL ?
	je	todelch		! yes -> erase one character
	ja	input		! non-printable -> ignore it
	cmp	al,#21		! ^U ?
	je	todell		! yes -> erase the line
	cmp	al,#24		! ^X ?
	je	todell		! yes -> erase the line
	cmp	al,#32		! ignore non-printable characters except space
	jb	input
	ja	noblnk		! no space -> go on
	cmp	(bx-1),al	! second space in a row ?
	je	input		! yes -> ignore it
noblnk:	cmp	bx,#cmdline+CL_LENGTH ! at end of buffer ?
	je	input		! yes -> ignore
	xor	ah,ah		! cmdline is always NUL terminated ;;;
	mov	(bx),ax		! store in the buffer
	inc	bx		! increment pointer
	push	bx
	call	display		! echo
#if defined(MENU) || defined(BITMAP)
	push	ax
	call	find_image	! we want the side effect of the hilite
	pop	ax
#endif
	pop	bx
	cmp	bx,#cmdline+1	! first character ?
	jne	input		! no -> next input
#ifdef LCF_IGNORECASE
	call	upcase		! convert to upper case
#endif
	mov	cx,#IMAGES	! check if we have a single-key entry
	mov	di,#DESCR0
	mov	ah,al
sklp:	test	word ptr (di+id_flags),#FLAG_SINGLE ! single-key entry ?
	jz	sknext		! no -> try next
	mov	al,(di)		! get first character
#ifdef LCF_IGNORECASE
	call	upcase		! convert to upper case
#endif
	cmp	al,ah		! do we have a match ?
	jne	sknext		! no -> try next
	cmp	byte ptr (di+1),#0 ! at end ?
	je	cr		! yes -> run it
sknext:	add	di,#id_size	! test next entry
	loop	sklp		! next one
	br	input		! done -> get more input

todelch:br	delch		! ...
todell:	br	delline		! ...

! End of input, process the command line

nul:	push	bx		! automatic boot - wait for timeout
	mov	ax,old_del
	call	waitsh
	pop	bx
	jnc	crnul		! no key pressed -> continue
	mov	bx,#msg_int	! interrupted -> display a message
	call	say
	br	iloop		! return to interactive prompt

cr:
#ifdef LCF_HP_TTRC
	push	ax		! HP TTRC boot fail workaround.
	mov	ax, #3		! 2000/10 <yumoto@jpn.hp.com>
	call	setto		! reload timer
short_wait:
	test	byte ptr timeout,#1 ! timed out ?
	jz	short_wait	! No, remain loop..
	pop	ax
#endif
	mov	cmdbeg,#mcmdbeg ! probably manual boot
crnul:
#ifndef LCF_NOSERIAL
	mov	byte ptr break,#0 ! clear the break flag
#endif
	push	cs		! set ES to CS
	pop	es
	xor	al,al		! mark end
	mov	(bx),al
	mov	si,#cmdline	! copy command line to save buffer
	mov	di,#lkcbuf
	mov	byte ptr dolock,#0 ! disable locking
;;;	push	es
;;;	push	ds
;;;	pop	es

cpsav:	lodsb			! copy one byte
	stosb
	or	al,al		! at end ?
	jnz	cpsav		! no -> go on
;;;	pop	es

	cmp	bx,#cmdline	! empty line ?
	je	notrspc		! yes -> boot first image
	cmp	byte ptr (bx-1),#32 ! trailing space ?
	jne	notrspc		! no -> go on
	dec	bx		! remove the space
	mov	byte ptr (bx),al
notrspc:mov	si,#cmdline	! scan the command line for "vga=", "kbd=",
	mov	di,si		! "lock" or "mem="
chkvga:
#ifdef LCF_BDATA
vsktnbd:
#ifndef LCF_M386
	cmp	word ptr (si),#0x6f6e		! "nobd"
	jne	vsktv
	cmp	word ptr (si+2),#0x6462
#else
	cmp	dword ptr (si),#0x64626f6e	! "nobd"
#endif
	jne	vsktv
	cmp	byte (si+4),#32			! terminated with SP or NUL?
	jnbe	vsktv
	inc	byte [no_bdata]
	jmp	vskwd		; skip word
#endif
vsktv:
#ifndef LCF_M386
	cmp 	word ptr (si),#0x6776 ! "vga=" ?
	jne	vsktk		! no -> try "kbd="
	cmp	word ptr (si+2),#0x3d61
#else
	cmp	dword ptr (si),#0x3d616776	! "vga="
#endif
	jne	vsktk
	call	setvga		! set VGA mode
	jc	toiloop		! error -> get next command
	jmp	vskdb		! proceed by discarding last blank
vsktk:
#ifndef LCF_M386
	cmp	word ptr (si),#0x626b ! "kbd=" ?
	jne	vsktl		! no -> try "lock"
	cmp	word ptr (si+2),#0x3d64
#else
	cmp	dword ptr (si),#0x3d64626b	! "kbd="
#endif
	jne	vsktl
	call	putkbd		! pre-load keyboard buffer
	jmp	vskdb		! proceed by discarding last blank
vsktl:
#ifndef LCF_M386
	cmp	word ptr (si),#0x6f6c ! "lock" ?
	jne	vsktm		! no -> skip to next blank
	cmp	word ptr (si+2),#0x6b63
#else
	cmp	dword ptr (si),#0x6b636f6c	! "lock"
#endif
	jne	vsktm
	cmp	byte (si+4),#32		! space?
	jnbe	vsktm
	mov	byte ptr dolock,#1 ! enable locking
vskwd:	add	si,#4		! skip word
vskdb:	dec	di		! discard last blank
	jmp	vsknb		! continue
vsktm:
#ifndef LCF_M386
	cmp	word ptr (si),#0x656d ! "mem=" ?
	jne	vsknb		! no -> skip to next blank
	cmp	word ptr (si+2),#0x3d6d
#else
	cmp	dword ptr (si),#0x3d6d656d	! "mem="
#endif
	jne	vsknb
	call	getmem		! get the user-provided memory limit
vsknb:	lodsb			! copy one byte
	stosb
	cmp	al,#32		! space ?
	je	chkvga		! yes -> look for options again
	or	al,al		! at end ?
	jnz	vsknb		! no -> go on
	call	crlf		! write CR/LF
	cmp	di,#cmdline+1	! empty line ?
emptyl:	je	bfirst		! yes -> boot first image
	jmp	bcmd		! boot the specified image

toiloop:br	iloop		! ...
toinput:br	input		! ...


! Find the boot image and start it

bcmd:
#if !defined(MENU) && !defined(BITMAP)

	mov	cx,#IMAGES	! test all names
	mov	bx,#DESCR0
nextn:	mov	si,bx		! compare the names
	mov	di,#cmdline
nextc:	mov	al,(si)		! get next character
	or	al,al		! NUL ?
	jz	dscend		! yes -> possible match
				! get the character
#ifdef LCF_IGNORECASE
	call	upcase
#endif
	mov	ah,al
	mov	al,(di)
#ifdef LCF_IGNORECASE
	call	upcase
#endif
	cmp	al,ah		! character equal ?
	jne	skipn		! no -> try next one
	inc	si		! test next character
	inc	di
	jmp	nextc
dscend:	cmp	byte ptr (di),#32 ! space or NUL -> boot
	je	boot
	cmp	byte ptr (di),#0
	je	boot
skipn:	add	bx,#id_size	! test next name
	loop	nextn
#else
	call	find_image
	jc	boot		! eureka, it was found
#endif
	mov	bx,#msg_nf	! not found -> display a message
	call	say
	br	iloop		! get more input

! Delete one character

delch:	cmp	bx,#cmdline	! at the beginning ?
	je	toinput		! yes -> do nothing
	dec	bx		! move the pointer
	push	bx		! display[B BS,SPC,BS
	mov	bx,#bs
	call	say
#if defined(MENU) || defined(BITMAP)
	pop	bx
	push	bx
	mov	byte (bx),#0	! NUL terminate the line
	mov	ax,#cmdline
	sub	ax,bx
	jz	delch6
delch4:	call	find_image
	jnc	delch9 
delch6:	mov	bx,[dimage]
	cmp	ax,bx
	je	delch9
	xchg	ax,bx
	call	lowlite
	xchg	ax,bx
	call	hilite
delch9:
#endif
	pop	bx
	jmp	toinput		! go on

! Delete the entire line

delline:
#if !defined(MENU) && !defined(BITMAP)
	cmp	bx,#cmdline	! done ?
	je	toinput		! yes -> go on
	push	bx		! display BS,SPC,BS
	mov	bx,#bs
	call	say
	pop	bx
	dec	bx		! move the pointer
	jmp	delline		! next one
#else
	call	menu_delline
	push	bx		! delch will do a pop
	xor	ax,ax
	jmp	delch6
#endif

! Boot first after timeout

brto:	call	crlf		! display a CRLF
	jmp	brfrst		! boot

! Boot the first image

bfirst:	mov	byte ptr lkcbuf,#0 ! clear default
	cmp	byte ptr cmdline,#0 ! is there a default ?
	jne	bcmd		! yes -> boot that image
brfrst:	
#if defined(MENU) || defined(BITMAP)
	mov	bx,[dimage]
	imul	bx,#id_size
	add	bx,#DESCR0	! boot the selected image
#else
	mov	bx,#DESCR0	! boot the first image
#endif
	mov	si,bx		! copy the name to the command line
	mov	di,#cmdline
bfcpl:	lodsb			! copy one character
	mov	(di),al
	inc	di
	or	al,al		! NUL ?
	jnz	bfcpl		! no -> next one

! Boot the image BX points to (with password check)

boot:	mov	si,#cmdline	! locate start of options
locopt:	lodsb
	or	al,al		! NUL ?
	je	optfnd		! yes -> no options
	cmp	al,#32		! space ?
	jne	locopt		! no -> continue searching
	cmp	(si),#0		! followed by NUL ?
	jne	optfnd		! no -> go on
	mov	byte ptr (si-1),#0 ! discard trailing space
optfnd:	dec	si		! adjust pointer
	mov	options,si	! store pointer for later use
#ifdef BITMAP
	call	menu_exit
#endif
	test	byte ptr (bx+id_flags),#FLAG_PASSWORD  ! use a password
	jz	toboot		! no -> boot
	test	byte ptr (bx+id_flags),#FLAG_RESTR ! restricted ?
	jz	dopw		! no -> get the password
	cmp	byte ptr (si),#0! are there any options ?
	jne	dopw		! yes -> password required
toboot:	br	doboot		! ...
dopw:
#if defined(CRC_PASSWORDS) || defined(SHS_PASSWORDS)
	push	bx		; save the image descriptor
	BEG_FS
	SEG_FS
	mov	word ptr par1_timeout+SSDIFF,#0xffff	; cancel timeout
	END_FS
	mov	bx,#msg_pw	! display a prompt
	call	say

	push	bp		; save BP
	mov	bp,sp		; save SP in BP
	sub	sp,#CL_LENGTH	; allocate space for PW string
	mov	si,sp		; si points at string
	xor	di,di		; di counts characters
pwloop:
#ifdef DEBUG
	pusha
	mov	ax,si
	call	wout
	mov	al,#32
	call	display
	mov	ax,di
	call	wout
	call	crlf
	popa
#endif
	mov	cx,#pwtime	; get timeout exit
	call	getkey
	
	cmp	al,#13		! CR ?
	je	pwcr		! yes -> handle it
	cmp	al,#21		! ^U ?
	je	pwdell		! yes -> erase line
	cmp	al,#24		! ^X
	je	pwdell
	cmp	al,#8		! BS ?
	je	pwdelch		! yes -> erase one character
	cmp	al,#127		! DEL
	je	pwdelch
	ja	pwloop		! ignore other non-printable characters
	cmp	al,#32
	jb	pwloop

	cmp	di,#CL_LENGTH	; check for buffer overflow
	jae	pwloop		; ingnore further input
	seg	ss
	mov	(si),al		; store char in buffer
	inc 	si
	inc	di
	mov	al,#42		; echo '*'
	call	display
	jmp	pwloop		; loop back for more

pwdelch: or	di,di
	jz	pwloop 
	call	pwbs
	dec	si
	dec	di
	jmp	pwloop

pwdell:	inc	di
pwdel:	dec	di
	jz	pwloop
	call	pwbs
	dec	si
	jmp	pwdel

pwbs:	mov	bx,#bs
	call	say
	ret

pwcr:	
	xor	cx,cx		; signal okay
pwtime:				; CX != 0 if enter here
	inc	cx
	call	crlf

	sub	si,di		; point SI at start of buffer
	push	es		; save ES

#if !defined(SHS_PASSWORDS)
	mov	bx,(bp+2)	; restore image descriptor pointer
	push	ss
	pop	es		; ES:SI points at char string

#if MAX_PW_CRC>=1
#ifdef LCF_M386
	push    dword #CRC_POLY1
#else
	push	#CRC_POLY1>>16
	push	#CRC_POLY1&0xFFFF
#endif
	call	crc32
#ifdef DEBUG
	pusha
	push	ax
	mov	ax,di
	call	wout
	mov	al,#32
	call	display
	pop	ax

#ifdef LCF_M386
	push	eax
#else
	push	dx
	push	ax
#endif
	call	dout
	call	crlf
	popa
#endif

#ifdef LCF_M386
	cmp	eax,(bx+id_password_crc)
#else
	cmp	ax,(bx+id_password_crc)
	jne	pwcleanup
	cmp	dx,(bx+id_password_crc+2)
#endif
	jne	pwcleanup
#endif

; insert other checks in here
#if MAX_PW_CRC>=2
#ifdef LCF_M386
	push    dword #CRC_POLY2
#else
	push	#CRC_POLY2>>16
	push	#CRC_POLY2&0xFFFF
#endif
	call	crc32
#ifdef LCF_M386
	cmp	eax,(bx+id_password_crc+4)
#else
	cmp	ax,(bx+id_password_crc+4)
	jne	pwcleanup
	cmp	dx,(bx+id_password_crc+4+2)
#endif
	jne	pwcleanup
#endif

#if MAX_PW_CRC>=3
#ifdef LCF_M386
	push    dword #CRC_POLY3
#else
	push	#CRC_POLY3>>16
	push	#CRC_POLY3&0xFFFF
#endif
	call	crc32
#ifdef LCF_M386
	cmp	eax,(bx+id_password_crc+8)
#else
	cmp	ax,(bx+id_password_crc+8)
	jne	pwcleanup
	cmp	dx,(bx+id_password_crc+8+2)
#endif
	jne	pwcleanup
#endif

#if MAX_PW_CRC>=4
#ifdef LCF_M386
	push    dword #CRC_POLY4
#else
	push	#CRC_POLY4>>16
	push	#CRC_POLY4&0xFFFF
#endif
	call	crc32
#ifdef LCF_M386
	cmp	eax,(bx+id_password_crc+12)
#else
	cmp	ax,(bx+id_password_crc+12)
	jne	pwcleanup
	cmp	dx,(bx+id_password_crc+12+2)
#endif
	jne	pwcleanup
#endif

#if MAX_PW_CRC>=5
#ifdef LCF_M386
	push    dword #CRC_POLY5
#else
	push	#CRC_POLY5>>16
	push	#CRC_POLY5&0xFFFF
#endif
	call	crc32
#ifdef LCF_M386
	cmp	eax,(bx+id_password_crc+16)
#else
	cmp	ax,(bx+id_password_crc+16)
	jne	pwcleanup
	cmp	dx,(bx+id_password_crc+16+2)
#endif
	jne	pwcleanup
#endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	dec	cx			; signal all okay
#else		/* SHS_PASSWORDS */
; DI is the count
; SS:SI is the password string
	push	di
	push	si
	call	_shsInit
	call	_shsUpdate
	call	_shsFinal
	mov	bx,(bp+2)	; restore image descriptor pointer
	lea	di,(bx+id_password_crc)
	mov	si,#shs_digest
	mov	cx,#MAX_PW_CRC*4
; ES==DS
	repe
	    cmpsb

	pop	si		; restore buffer ptr
	pop	di		; clear stack
	push	ss
	pop	es		; ES=SS

	je	pwcleanup	; CX will be 0
	inc	cx		; CX is > 0
#endif		/* !defined(SHS_PASSWORDS) */

pwcleanup:
	push	cx
	mov	cx,#CL_LENGTH
	mov	di,si
	xor	ax,ax
	rep			; wipe out password in memory
	    stosb
	pop	cx
	pop	es		; restore the saved ES

	mov	sp,bp
	pop	bp
	pop	bx
	or	cx,cx		; test CX==0 means all okay
	jz	doboot
; fall into pwfail	

#else
	push	bx		! save the image descriptor
	lea	si,(bx+MAX_IMAGE_NAME+1) ! get a pointer to the password string
	mov	bx,#msg_pw	! display a prompt
	call	say
pwagain:xor	cl,cl		! CL counts characters after a mismatch
pwloop:	push	cx		! get a key
	mov	cx,#pwtime
	call	getkey
	pop	cx
	cmp	al,#13		! CR ?
	je	pwcr		! yes -> handle it
	cmp	al,#21		! ^U ?
	je	pwdell		! yes -> erase line
	cmp	al,#24		! ^X
	je	pwdell
	cmp	al,#8		! BS ?
	je	pwdelch		! yes -> erase one character
	cmp	al,#127		! DEL
	je	pwdelch
	ja	pwloop		! ignore other non-printable characters
	cmp	al,#32
	jb	pwloop
	or	cl,cl		! counting bad input ?
	jnz	pwbad		! yes -> do it
	cmp	al,(si)		! correct input ?
	je	pwgood		! yes -> go on
pwbad:	inc	cl		! count error
	jnz	pwgood		! no overflow -> go on
	dec	cl		! adjust it
	jmp	pwcr		! terminate input
pwgood:	inc	si		! good character -> go on
	jmp	pwloop
pwdell:	pop	si		! reset the pointer
	push	si
	add	si,#MAX_IMAGE_NAME+1
	jmp	pwagain		! get password again
pwdelch:pop	bx		! at the beginning of the line ?
	push	bx
	add	bx,#MAX_IMAGE_NAME+1
	cmp	si,bx
	je	pwloop		! yes -> ignore it
	dec	si		! remove one character
	sub	cl,#1
	jnc	pwloop		! no underflow -> go on
	inc	cl		! adjust it
	jmp	pwloop		! next character
pwtime:	pop	cx		! drop CX ...
	mov	cl,#1		! ... and fail
pwcr:	call	crlf
	pop	bx		! restore the image descriptor
	or	cl,cl		! no errors ?
	jnz	pwfail		! no -> fail
	cmp	byte ptr (si),#0 ! at end ?
	je	doboot		! yes -> continue booting
#endif	/* CRC_PASSWORDS */

pwfail:	mov	bx,#msg_pf	! display an error message
	call	say
	br	iloop		! get next input

! Boot the image BX points to

doboot:	mov	byte ptr prechr,#61 ! switch to equal sign
	push	bx		! save image descr
	mov	bx,#msg_l	! say hi
	call	say
	pop	bx		! display the image name
	push	bx
	call	say
	pop	si

	add	si,#id_rd_size	! point at ramdisk size long
! take care of the RAM disk first
	mov	word ptr (rdmid),#0 ! clear address
#ifdef LCF_M386
	lodsd
	mov	rdszl,eax	! set rdszl+rdszh
	add	eax,#4095	! round up &
	shr	eax,#12		! convert to pages
	xchg	bx,ax		! copy to BX
#else
	lodsw			! number of bytes to load (low word)
	mov	dx,ax
	mov	rdszl,ax
	lodsw			! high word
	mov	rdszh,ax
	xchg	ax,dx		! convert to pages
	add	ax,#4095
	adc	dx,#0
	mov	bx,#4096
	div	bx
	mov	bx,ax
#endif
	lodsw			! address of the first map sector
	xchg	cx,ax
	lodsw
	xchg	dx,ax
        lodsb                   
	or	bx,bx		! no RAM disk ?
	jz	noramd		! yes -> skip it
	push	si		! save SI, ES, and BX (RD size)
	push	es
	push	bx
	mov	bx,#MAP		! load the first map sector
	call	sread
	mov	moff,#0
#ifdef DEBUG
	mov bx,#stepa
	call say
#endif
	mov	ah,#0x88	! okay, compute load address now
	int	0x15		! get amount of memory ...
	cmp	word ptr memlim,#0 ! limit set by user ?
	je	noulim		! no -> continue
	mov	ax,memlim	! use user limit
	sub	ax,#1024	! -1 MB
noulim:	cmp	ax,#0x3c00	! more than 16 MB ?
	jbe	no16mb		! no -> go ahead
	mov	ax,#0x3c00	! limit to 16 MB to be nice to the BIOS
				! (kernel can still use "high" memory)
no16mb:	shr	ax,2		! round to pages
	pop	bx		! subtract RAM disk size
	sub	ax,bx
	jnc	rdokay		! panic if not enough space
	cmp	ax,#128		! we probably need more than 512 kB for the
	ja	rdokay		! kernel to be useful ...
	mov	bx,#msg_rd	! complain
	call	say
	br	restrt	 	! ... and restart if we can

rdokay:	add	ax,#256		! start is at first MB ...
	shl	ax,4		! *16
	mov	(gdt+0x1b),ax	! set up GDT
	mov	byte ptr (gdt+0x1a),#0
	mov	(rdmid),ax	! ... and remember that for patching setup
;;;	xor	ax,ax		! ES=0 is our secret code to load via GDT
;;;	mov	es,ax
	push	#0		;;;
	pop	es		;;;
	mov	bx,#gdt
	call	lfile		! load it
	pop	es		! restore ES and SI
	pop	si
noramd:
;  Now load the kernel sectors
	lodsw			! address of the first map sector
	xchg	cx,ax
	lodsw
	xchg	dx,ax
        lodsb                   ;;;
        push    ax              ;;;
	mov	word ptr (gdt+0x1b),#0 ! set GDT to "load low"
	lodsw			! get the start page
	or	ax,ax		! load low ?
	jz	nohigh		! yup -> do it
	shl	ax,4		! *16
	mov	(gdt+0x1b),ax	! set up GDT
	mov	byte ptr (gdt+0x1a),#0
nohigh:
        pop     ax              ;;;
	push	si		! save SI
#ifdef DEBUG
        push    ax              ;;;
	mov bx,#step0
	call say
        pop     ax              ;;;
#endif
	mov	bx,#MAP		! load the first map sector
	call	sread
	mov	moff,#0		! reset the pointer
#ifdef DEBUG
	mov bx,#step0b
	call say
#endif
	mov	bx,#DFLCMD	! load the default command line
	BEG_FS
	SEG_FS
	mov	cx,par1_dflcmd+SSDIFF
	SEG_FS
	mov	dx,par1_dflcmd+2+SSDIFF
        SEG_FS                      ;;;
	mov	al,par1_dflcmd+4+SSDIFF   ;;;
	END_FS
	call	cread
	push	word ptr (DFLCMD) ! push magic number
	mov	bx,#DFLCMD	! load the fallback sector
	call	load1
	pop	ax		! valid magic number ?
#ifndef LCF_READONLY
	cmp	ax,#DC_MAGIC
	je	dclok		! yes -> can write
	cmp	ax,#DC_MGOFF
	jne	nofbck		! invalid -> must not write
dclok:	mov	bx,#DFLCMD	! fallback data present ?
	cmp	word ptr (bx),#DC_MAGIC
	jne	nofbck		! no -> go on
	call	cmd_write	! write out the command line
nofbck:
#endif
#ifdef DEBUG
	mov bx,#step1
	call say
#endif
	mov	bx,#DFLCMD	! load the options sector
	call	load1
	mov	si,cmdbeg	! copy non-options part of command line
	mov	di,#PARMLINE
cpnocl:	cmp	si,options	! at beginning of options ?
	je	cpnodn		! yes -> go on
	movsb			! copy one byte
	jmp	cpnocl		! next one
cpnodn:	mov	si,#DFLCMD	! constant options ?
	cmp	byte ptr (si),#0
	je	nocopt		! no -> go on
	mov	al,#32		! add a space
	stosb
cpcodsp:lodsb			! fetch next byte
	cmp	al,#32		! space ?
	je	cpcodsp		! yes -> discard it
cpcolp:	or	al,al		! NUL ?
	jz	cpcodn		! yes -> done
	stosb			! store byte
	cmp	al,#32		! a space ?
	je	cpcodsp		! yes -> discard next
	lodsb			! get next byte
	jmp	cpcolp
cpcodn:	seg	es
	cmp	byte ptr (di-1),#32 ! last was space ?
	jne	nocopt		! no -> go on
	dec	di		! discard it
nocopt:	mov	si,options	! append variable options
cpvalp:	lodsb			! copy one byte
	stosb
	or	al,al		! NUL ?
	jnz	cpvalp		! no -> go on
#ifdef DEBUG
	mov bx,#step2
	call say
#endif
#ifdef VAR_LOAD
	mov	es,[initseg]	! load the original boot sector
#else
	push	#INITSEG	! load the original boot sector
	pop	es
#endif
	xor	bx,bx		! load now
	call	load1
	pop	si		! restore SI
	lodsw			! get flags bit map
	xchg	bx,ax		! move to BX
	lodsw			! copy parameters ... VGA mode ... (done)
	cmp	word ptr vgaovr,#VGA_NOCOVR ! VGA mode not overridden on
				! command line ?
	je	vganorm		! no -> go on
	mov	ax,vgaovr	! use that value
	jmp	vgaset
vganorm:test	bx,#FLAG_VGA
	jz	novga
vgaset:	seg	es
	mov	506,ax		! magic offset in the boot sector
novga:	push	bx		! use flags (BX) later
	test	bx,#FLAG_LOCK	! ... lock target ?
	jnz	lockit		! yup -> do it
	cmp	byte ptr dolock,#0 ! did user ask to lock new target ?
	je	nolock		! no -> go on
lockit:
#ifndef LCF_READONLY
	mov	bx,#lkwbuf	! save the command line
#ifdef LOCK_BSS
	mov	word (bx),#DC_MAGIC	;;;
#endif
	push	es
	push	si

        push    ds                      ;;;
        pop     es                      ;;;
	call	cmd_write		; write out the command line

	pop	si
	pop	es
#endif
nolock:
#ifdef DEBUG
	mov bx,#step3
	call say
#endif
#if 0
	mov	cx,#SETUPSECS	! default is to load four sectors
#ifdef LCF_VARSETUP
	seg	es		! variable number of sectors ?
	cmp	byte ptr VSS_NUM,#0
	je	lsetup		! no -> use default
	seg	es		! use that number
	mov	cl,VSS_NUM
#endif
#else
	xor	cx,cx
#ifdef LCF_VARSETUP
	seg	es
	mov	cl,VSS_NUM
	or	cx,cx
	jnz	lsetup
#endif
	mov	cl,#SETUPSECS	! default is to load four sectors
#endif
lsetup:	
#ifdef VAR_LOAD
	mov	es,[setupseg]	! load the setup codes
#else
	push	#SETUPSEG	! load the setup (might meet EOF in the
	pop	es		! process of loading the boot sector of an
				! other operating system)
#endif
#ifdef MEMORY_CHECK
	mov	ax,cx		! number of sectors to AX
	shl	ax,5		! convert to paragraphs (9-4)
	mov	bx,es
	add	bx,ax
	add	bx,#STACK>>4	! allow for stack space in paragraphs
	mov	ax,cs		!
	cmp	bx,ax
	jbe	enough_mem
	mov	bx,#msg_mem	! we are very short on memory
	call	say

enough_mem:

#endif
	xor	bx,bx		! other operating system)
lsloop:	push	cx
	call	loadopt
	pop	cx
	loop	lsloop
#ifdef DEBUG
	mov bx,#step4
	call say
#endif
	pop	bx		! get flags
	test	bx,#FLAG_MODKRN	! "modern" kernel ?
	jz	loadlow		! no -> avoid all patching and such
	seg	es		! set loader version
	mov	byte ptr (16),#LOADER_VERSION
	seg	es		! version >= 1 ?
	cmp	word ptr (6),#NEW_HDR_VERSION
	jbe	noheap		! no -> do not patch heap
	mov	ax,cs
#ifdef VAR_LOAD
	sub	ax,[initseg]	! find no. of paragraphs available
#endif
	shl	ax,4
	add	ax,#SLA_SIZE_DYN
	seg	es
	mov	word ptr (36),ax
	seg	es		! patch flags
	or	byte ptr (17),#LFLAG_USE_HEAP
noheap:	cmp	word ptr (rdmid),#0 ! initial RAM disk ?
	je	nordpt		! no -> no need to patch header for that
	xor	al,al		! store RAM disk start address
	mov	ah,(rdmid)
	seg	es
	mov	(24),ax
	xchg	al,ah
	mov	al,(rdmid+1)
	seg	es
	mov	(26),ax
#ifdef LCF_M386
	mov	eax,rdszl
	seg	es
	mov	(28),eax	! set RAM disk size
#else
	mov	ax,rdszl	! RAM disk size
	seg	es
	mov	(28),ax
	mov	ax,rdszh
	seg	es
	mov	(30),ax
#endif
nordpt:	cmp	word ptr (gdt+0x1b),#0 ! load low ?
	je	loadlow		! yes -> do it
#if 0 /* not necessary ! */
	mov	ax,(gdt+0x1a)	! load high instead: patch setup header ...
	seg	es		! start address
	mov	(20),ax
	mov	ax,(gdt+0x1c)
	xor	ah,ah
	seg	es
	mov	(22),ax
	seg	es		! set "high" flag ...
	mov	byte ptr (17),#LFLAG_HIGH
#endif
	xor	ax,ax		! GDT is already set up ...
	mov	es,ax
	mov	bx,#gdt
	call	lfile		! load the system ...
	jmp	launch		! ... and run it
loadlow:call	loadfile	! load the system
	jmp	launch		! go !

loadfile:
	push	#SYSSEG		! load a file at SYSSEG:0000
	pop	es
	xor	bx,bx
lfile:	call	load
	jmp	lfile

! Load one sector. Issue an error at EOF.

load1:	call	loadit		! load the sector
	mov	bx,#msg_eof	! we only get here at EOF
	call	say
	br	restrt

loadit:	call	load		! load it
	pop	ax		! drop return address of load1
	ret

! Load one sector. Start the system at EOF.

loadopt:call	loadit		! load the sector
	jmp	launch		! go

! Load one sequence of sectors. Leave outer function at EOF.

load:	push	es		! save ES:BX
	push	bx
lfetch:	mov	si,moff		! get map offset
	mov	cx,MAP(si)	! get address
	mov	dx,MAP+2(si)
	mov	al,MAP+4(si)
	or	cx,cx		! at EOF ?
	jnz	noteof		! no -> go on
	or	dx,dx
	jnz	noteof
	pop	bx		! restore ES:BX
	pop	es
	pop	ax		! pop return address
	ret			! return to outer function
noteof:	add	si,#5		! increment pointer
	mov	moff,si
	cmp	si,#508		! page end ?
#ifndef LCF_M386
	bcs	doload		! no -> load it
#else
	jb	near doload
#endif

	mov	moff,#0		! reset pointer
	push	cs		! adjust ES
	pop	es

        mov     bl,hinib        ;;; this might get clobbered
        push    bx              ;;; so save it
	mov	bx,#MAP		! load map page
	call	sread
        pop     ax              ;;; restore the hi-nibble
	mov     hinib,al        ;;; 

	mov	al,#0x2e	! print a dot
	call	display
	jmp	lfetch		! try again

! Start the kernel

launch:
;;;	push	es		! save ES:BX (why ???)
;;;	push	bx
#ifdef MENU
	call	menu_exit	! make the menu area vanish
#endif
	call	crlf		! display a CRLF
	mov	dx,#0x3f2	! stop the floppy motor
	xor	al,al
	outb
	xor	ax,ax		! reset the FDC
	mov	dl,al
	int	0x13
#ifdef	VAR_LOAD
	mov	es,[initseg]	! adjust segment registers
#else
	push	#INITSEG	! adjust segment registers
	pop	es
#endif
	mov	di,#PARMLINE	! set parameter line offset
	mov	ax,cs		! find where we are loaded
#ifdef VAR_LOAD
	sub	ax,[initseg]	! find no. of paragraphs available
#endif
	shl	ax,4		! convert para. to bytes
	add	di,ax
#ifndef LCF_M386
	seg	es
	cmp	word ptr CL_HEADER_ID,#0x6448	! "Hd"
	jne	mbchain
	seg	es
	cmp	word ptr CL_HEADER_ID+2,#0x5372	  ! "rS"
#else
	seg	es
	cmp	dword ptr CL_HEADER_ID,#0x53726448	! "HdrS" (reversed)
#endif
	je	chkver		! go check header version
mbchain:

! it must be the chain loader

#ifdef LCF_BDATA
	inc	byte [no_bdata]	! suppress BIOS data collection
#endif
#if DEBUG_NEW
	mov	bx,#nohdrs
	call	say
	jmp	cl_wait

nohdrs:	.ascii	"\nNo cmdline passed"
	.byte	0
#else
	jmp	start_setup2
#endif

chkver:
#if DEBUG_NEW
	mov	bx,#hdr1
	call	say
	seg	es
	mov	ax,CL_HDRS_VERSION
	call	wout
	mov	bx,#hdr2
	call	say
#endif
	seg	es
	cmp	word ptr CL_HDRS_VERSION,#NEW_VERSION  ! check for
				! new cmdline protocol
	jb	protocol201

! and now the new protocol

	mov	ax,es		! form long address
#ifndef LCF_M386
	xor	dx,dx		! in DX:AX
	mov	cx,#4
chkver11:
	shl	ax,1
	rcl	dx,1
	loop	chkver11
	add	ax,di		! add in offset
	adc	dx,#0
	seg	es
	mov	CL_POINTER,ax	! set the long, absolute pointer
	seg	es
	mov	CL_POINTER+2,dx
#if DEBUG_NEW
	push	dx
	push	ax
#endif
#else
	movzx	edx,ax		! zero extend segment part to EDX
	movzx	edi,di		! zero extend offset
	shl	edx,4		! make segment into address
	add	edx,edi		! form long absolute address
	seg	es
	mov	CL_POINTER,edx	! and pass the address
#if DEBUG_NEW
	push	edx		
#endif
#endif		/* LCF_M386 */
#if DEBUG_NEW
	call	dout
	jmp	cl_wait
#else
	jmp	start_setup
#endif

! the old command line passing protocol

protocol201:
	seg	es
	mov	CL_MAGIC_ADDR,#CL_MAGIC ! set magic number
	seg	es
	mov	word ptr CL_OFFSET,di
#if DEBUG_NEW
	mov	ax,es
	call	wout
	mov	al,#0x3A	! issue colon    
	call	display
	mov	ax,di
	call	wout
cl_wait:
	call	crlf
	call	crlf
	jmp	start_setup

hdr1:	.ascii	"\nHeader 0x"
	.byte	0
hdr2:	.ascii	" cmdline at "
	.byte	0
#endif
start_setup:			! kernel boot comes here
#ifdef LCF_BDATA
	cmp	byte [no_bdata],#0	! skip if flag set
	jne	start_setup2

#if DEBUG_NEW
	call	ss3
	.ascii	"BIOS check\ns"
	.byte	0
ss3:	pop	bx
	call	say
#else
	mov	al,#0x73	! display an 's'
	call	display
#endif

#if DEBUG_NEW
	call	pause
#endif
	call	io_biosdata

	mov	bx,#bs
	call	say
#endif
start_setup2:			! chain loader boot comes here
#if DEBUG_NEW
	call	pause		! last chance to use the timer
#endif
	call	remto		! free timer interrupt
	push	es
	pop	ds
#if 0
	push	es
	pop	fs
	push	es
	pop	gs
#endif
#ifdef VAR_LOAD
	add	sp,#SETUP_STACK_DYN	! increase stack size over this code
	seg	cs
	  push	[setupseg]
	push	#0
	retf			! start the kernel setup code
#else
	mov	ax,cs
	shl	ax,4
	add	ax,#SETUP_STACK_DYN
	mov	sp,ax
	jmpi	0,SETUPSEG	! start the setup
#endif


! Load one sector (called from load)

doload:	pop	bx		! restore ES:BX
	pop	es

! Load a sequence of sectors, possibly moving into "high memory" (> 1 MB)
! afterwards.

xread:	push	ax		! ES == 0 ?
	mov	ax,es
	or	ax,ax
	pop	ax
	jz	rdhigh		! yes -> read into high memory
	jmp	sread
rdhigh:	push	bx		! okay - DS:BX points to GDT in this case
	mov	bx,#SYSSEG	! adjust ES:BX
	mov	es,bx
	xor	bx,bx
	call	sread		! load the sector(s)
        mov     tempal,al
	pop	bx		! get pointer to GDT
	push	ax		! just in case ...
	push	cx
	push	si
	mov	si,bx		! turn ES:SI into pointer to GDT
	push	ds
	pop	es
	xor	cx,cx		! number of words to move
	mov	ch,tempal
#ifdef DEBUG
	push	si
	push	bx
	push	cx
	mov	al,(si+0x14)
	call	bout
	mov	ax,(si+0x12)
	call	wout
	mov	bx,#mov_ar
	call	say
	mov	al,(si+0x1c)
	call	bout
	mov	ax,(si+0x1a)
	call	wout
	mov	bx,#mov_sz
	call	say
	pop	ax
	push	ax
	call	wout
	call	crlf
	pop	cx
	pop	bx
	pop	si
#endif
	push	bx		! do the transfer. (save BX, CX and SI because
	push	cx		! we are paranoid)
	push	si
	mov	ah,#0x87
	int	0x15
	pop	si
	pop	cx
	pop	bx
	jc	badmov		! failed ...
	seg	es		! move pointer
	add	(si+0x1a),cx
	seg	es
	adc	byte ptr (si+0x1c),#0
	seg	es
	add	(si+0x1a),cx
	seg	es
	adc	byte ptr (si+0x1c),#0
	sub	ax,ax		! put ES back to 0
	mov	es,ax
	pop	si
	pop	cx
	pop	ax
	ret			! done

badmov:	push	ax		! save the error code
	mov	bx,#msg_bm	! tell the user ...
	jmp	reset		! (standard procedure)

! Load a sequence of sectors

sread:	push	bx		! save registers
	push	cx
	push	dx
	call	cread
	mov	di,ax		! save AL return count
	jc	rerror		! error -> complain
	pop	dx		! restore registers
	pop	cx
rokay:	pop	bx
        shl     ax,8            ; convert sectors to bytes
        add     ah,ah
	jc	dowrap		! loaded an entire segment -> advance ES
	add	bx,ax		! move BX
	jnc	nowrap		! same segment -> go on
dowrap:	mov	ax,es		! move ES
	add	ax,#0x1000
	mov	es,ax
nowrap:
	mov	ax,di		! restore the block count in AL
aret:	ret			! done

! Read error - try a second time and give up if that fails too

rerror:
#if 0   /* we now have 5 tries down in cread */
	xor	ax,ax		! reset the disk
	mov	dl,al
	int	0x13
	pop	dx		! try again
	pop	cx
	pop	bx
	push	bx
	mov	al,tempal
	call	cread
	jnc	rokay		! okay -> go on
#endif
	push	ax
	mov	bx,#msg_re	! say something
reset:	call	say
	pop	ax		! display the error code
	mov	al,ah
	call	bout
	call	crlf		! a CR/LF
	mov	moff,#0		! restore initial state
	br	restrt

! Convert character in AL to upper case

upcase:	cmp	al,#0x61	! lower case character ? ('a')
	jb	nolower		! no -> go on
	cmp	al,#0x7a	! 'z'
	ja	nolower
	sub	al,#0x20	! convert to upper case
nolower:ret			! done

#if DEBUG_NEW
pause:	push	ax
	mov	ax,#3000/55	! delay 3 seconds
	call	setto
delay1:	test	byte ptr timeout,#-1
	jz	delay1
	pop	ax
	ret
#endif

#if DEBUG_NEW

!  display a double word, pushed into the stack

dout:	push	bp
	mov	bp,sp
	push	ax

	mov	ax,(bp+6)	! get high order
	call	wout
	mov	ax,(bp+4)	! get low order
	call 	wout

	pop	ax
	leave
	ret	4

! Display a hexadecimal word/byte/nibble

wout:	push	ax
	xchg	al,ah
	call	bout
	pop	ax
; must fall into  bout
#endif
bout:	push	ax		! save byte
	shr	al,#4		! display upper nibble
	call	nout
	pop	ax
nout:	and	al,#0x0F	! lower nible only
	daa			! smaller conversion routine
	add	al,#0xF0
	adc	al,#0x40	! AL is hex char [0..9A..F]
	jmp	display		! display it

! part of the 'say' routine
! actual entry point is below at 'say:'

say_loop:
	cmp	al,#10		! \n ?
	jne	nonl		! no -> go on
	mov	al,#13		! display a CRLF
	call	display
	mov	al,#10
nonl:
	cmp	al,#12		! ^L ?
	jne	nocls		! no -> go on
#ifdef MENU
	call	menu_form_feed	! simulate a FF
#else
#ifdef BITMAP
	cmp	BYTE [abs_cx+1],#0	; graphic screen on?
	jne	tosnext
#endif
	mov	ah,#0xf		! clear the local screen
	int	0x10
	xor	ah,ah
	int	0x10
#endif
tosnext: jmp	snext		! next character
nocls:	call	display		! display, tty-style
snext:
	inc	bx		! next one
! fall into  say   		! process next character

! Display a NUL-terminated string on the console

say:	mov	al,(bx)		! get byte
	or	al,al		! NUL ?
	jnz	say_loop	! not the end
	ret


! Display CR/LF

crlf:	mov	al,#13		! CR
	call	display
	mov	al,#10		! LF
;;;	jmp	display
; fall into  display

! Display one character on the console

display:
	push	bx		! save BX

#ifndef LCF_NOSERIAL
	call	serdisp
#endif
#if defined(MENU) || defined(BITMAP)
	seg	cs
	cmp	word [suppress],#0
	jnz	dispret

#ifdef MENU
	push	ds

	push	cs
	pop	ds

	push	dx

	cmp	byte [abs_cx+1],#0	! is special scrolling in effect?
	je	goshowit	! jump if no special handling
	call	mn_getcursor	! get cursor pos. in DX
	
	cmp	al,#8		! is it BS
	jne	scroll1
	or	dl,dl		! at col. 0?
	jne	goshowit
; must simulate a BS
	mov	dl,[mn_max_row_col]	! move to EOL
	mov	al,#0x0a	! change to LF
	dec	dh		! back up first of two lines
	jmp	scroll_set	! set new cursor pos. & ring BEL

scroll1:
	cmp	al,#0x0a	! test for LF / NL
	jne	scroll2
	cmp	dh,[mn_max_row_col+1]  ! bottom row
	jae	scrollit
scroll2:
	cmp	al,#0x20	! printing char?
	jb	goshowit
	cmp	dx,[mn_max_row_col]	! bottom corner
	jne	goshowit

scrollit:
	pusha
	mov	ax,#0x601	! scroll up 1 line
	mov	bh,[mn_at_mono]
	mov	cx,[abs_cx]
	mov	dx,[mn_max_row_col]
	int	0x10		! do the scroll
	popa
scroll_set:
	dec	dh
	call	mn_setcursor	! set cursor up 1 row
goshowit:
	pop	dx

	pop	ds
#endif
#endif
	xor	bh,bh		! display on screen
	mov	ah,#14
	int	0x10
dispret:
	pop	bx		! restore BX
	ret


#ifndef LCF_NOSERIAL
serdisp:push	dx		! wait for space in the send buffer
	seg	cs
	mov	dx,slbase
	or	dx,dx
	jz	serret
	add	dx,#5
	push	ax
serwait:in	al,dx
	test	al,#0x10	! break -> set break flag
	jz	nobrk
	seg	cs
	mov	byte ptr break,#1
nobrk:	test	al,#0x20	! ready to send ?
	jz	serwait		! no -> wait
	sub	dx,#5		! send the character
	pop	ax
	out	dx,al
serret:	pop	dx		! done
	ret
#endif

! Get a key (CX = timeout exit)

getkey: BEG_FS
	SEG_FS		! set the timeout
	mov	ax,par1_timeout+SSDIFF		;DSC_OFF-10+SSDIFF
	END_FS
	call	setto
gwtkey:	mov	ah,#1		! is a key pressed ?
	int	0x16
	jnz	gotkey		! yes -> get it
#ifndef LCF_NOSERIAL
	mov	dx,slbase	! using a serial port ?
	or	dx,dx
	jz	gnokey		! no -> wait
	add	dx,#5		! character ready ?
	in	al,dx
	test	al,#1
	jz	gnokey		! no -> wait
	sub	dx,#5		! get it
	in	al,dx
	and	al,#0x7f	! strip 8th bit
	jnz	gotch		! ignore NULs
#endif
gnokey:	
#if defined(MENU) || defined(BITMAP)
#ifdef BITMAP
	cmp	byte [abs_cx+1],#0
	je	no_timer_display
#endif
	call	timer_display
no_timer_display:
#endif
	test	byte ptr timeout,#1 ! timed out ?
	jz	gwtkey		! no -> wait
	pop	ax		! discard return address
	jmp	cx		! jump to timeout handler
gotkey:	xor	ah,ah		! read a key
	int	0x16
	push	bx		! keyboard translation (preserve BX)
	mov	bx,#KEYTABLE
	xlatb
	pop	bx
gotch:
#ifdef LCF_ONE_SHOT
	BEG_FS
	SEG_FS		! always enter prompt ?
	test	byte ptr par1_prompt+SSDIFF,#FLAG_PROMPT
;;;	END_FS
	jz	noosht		! yes -> do not disable timeout
;;;	BEG_FS
	SEG_FS		! disable timeout
	mov	word ptr par1_timeout+SSDIFF,#0xffff
;;;	END_FS
noosht:
	END_FS		;;;
#endif
	ret			! done

! Shift wait loop (AX = timeout, returns CY set if interrupred)

waitsh:	call	setto		! set timeout
actlp:	mov	ah,#2		! get shift keys
	int	0x16
	and	al,#0x5f	! anything set ? (except NumLock)
	jnz	shpress		! yes -> return with CY set
#ifndef LCF_NOSERIAL
	mov	dx,slbase	! using a serial port ?
	or	dx,dx
	jz	acnosp		! no -> go on
	cmp	byte ptr break,#0 ! break received ?
	jnz	shpress		! yes -> return with CY set
	add	dx,#5		! check for pending break
	in	al,dx
	test	al,#0x10
	jnz	shpress		! break received -> return with CY set
#endif
acnosp:	test	byte ptr timeout,#1 ! timed out ?
	jz	actlp		! no -> wait
	clc			! clear carry
	ret			! done
shpress:stc			! set carry
	ret			! done

! Timeout handling

instto:	push	ds		! install the timeout handler
	push	#0
	pop	ds

	cli			! no interrupts
#ifdef LCF_M386
	mov	eax,[0x1c*4]	! get the old vector
	seg	cs
	mov	[int1c_l],eax	! save H & L parts
#else
	mov	ax,[0x1c*4]	! get the old vector
	seg	cs
	mov	int1c_l,ax
	mov	ax,[0x1c*4+2]
	seg	cs
	mov	int1c_h,ax
#endif
	mov	[0x1c*4],#tick	! install new vector
	mov	[0x1c*4+2],cs
	sti			! done
	pop	ds
	ret

remto:	push	es		! remove the interrupt handler
	push	#0
	pop	es

#ifdef LCF_M386
	mov	eax,[int1c_l]	! restore the old vector
	seg	es
	mov	[0x1c*4],eax	! **
#else
	cli
	mov	ax,int1c_l	! restore the old vector
	seg	es
	mov	[0x1c*4],ax
	mov	ax,int1c_h
	seg	es
	mov	[0x1c*4+2],ax
	sti			! done
#endif
	pop	es
	ret

! AX = ticks, 0xffff = no timeout

setto:	or	ax,ax		! time out immediately ?
	jz	toimmed		! yes -> do it
	cli			! set timeout value
	mov	cntdown,ax
	mov	byte ptr timeout,#0 ! clear timed-out flag
	sti			! done
	ret
toimmed:mov	byte ptr timeout,#0xff ! set the timed-out flag
	ret			! done

tick:	pushf			! save flags
	seg	cs		! no timeout ?
	cmp	cntdown,#0xffff
	je	notzro		! yes -> go on
	seg	cs		! decrement counter
	dec	cntdown
	jnz	notzro		! not zero -> go on
	seg	cs		! set timeout flag
	mov	byte ptr timeout,#0xff
notzro:
#if 0
	popf			! done
	seg	cs
	jmpi	(int1c_l)	! continue with old interrupt
#else
#ifdef LCF_M386
	seg	cs
	push	dword [int1c_l]
#else
	seg	cs
	push	word [int1c_h]
	seg	cs
	push	word [int1c_l]
#endif
	iret			! continue with old interrupt
#endif

#ifndef LCF_READONLY

! Sector write; used for the stored command line only

cmd_write:	
	BEG_FS
	SEG_FS
	test	byte ptr par1_prompt+SSDIFF,#FLAG_RAID_NOWRITE ; no writes?
	END_FS
	jnz	cwok		; jump if no writing allowed

	BEG_FS
	SEG_FS
	mov	cx,par1_dflcmd+SSDIFF
	SEG_FS
	mov	dx,par1_dflcmd+2+SSDIFF
        SEG_FS                      ;;;
	mov	al,par1_dflcmd+4+SSDIFF   ;;;
#ifdef RAID_NEW
	SEG_FS
	test	byte ptr par1_prompt+SSDIFF,#FLAG_RAID	; is it a RAID write
#endif
	END_FS
#ifdef RAID_NEW
	jnz	cmd_raid_wrt
#endif
        mov     byte ptr (dsk_wrflag),#1        ! flag write operation
        call    cread
        mov     byte ptr (dsk_wrflag),#0        ! flag read operation

	jnc	cwok		! no error - return
cnok:	push	ax		! leave no traces
	push	bx
	mov	al,#87		! "W"
	push	ax		! display clobbers AH
	call	display
	pop	ax
	mov	al,ah		! error code
	call	bout
	pop	bx
	pop	ax
        stc                     ! flag error  JRC
cwok:
#if DEBUG_NEW
	call	pause
#endif
	ret			! done

#ifdef RAID_NEW

;  We know it is RAID, so CHS is not in effect

cmd_raid_wrt:
	mov	di,#raid_tt	; point at RAID translation table
        xor     ah,ah           ;will be zero for LINEAR
        xchg    al,dh           ;AX is possible address 
	test	dl,#LBA32_FLAG	;test for pure LINEAR	*****
	jz	lnwrt		;jump if pure LINEAR	*****
        test    dl,#LBA32_NOCOUNT
        jz      lnwrt
        mov     ah,dh           ;former count is really hi-nibble
lnwrt:			; address is AX:CX, DH=1, DL=device+flags, ES:BX=buffer
        mov     dh,#1           ;set count to 1
	push	ax	
	push	cx
	push	di
	mov     byte ptr (dsk_wrflag),#1        ! flag write operation
#if DEBUG_NEW
	pusha
	push	ax			! save high order
	push	cx			! save low order
	push	dx
	mov	bx,#cmdwr
	call	say
	pop	ax
	call	bout			! device+flags
	mov	bx,#rs_reloc
	call	say
	call	dout
	call	crlf
	popa
#endif
	call	lnread
	mov	byte ptr (dsk_wrflag),#0	! flag read operation
	pop	di
	pop	cx
	jc	cmdwerr
	pop	ax
	mov	dh,(di)			! get next raid device
	or	dh,dh
	jz	cwok
	and	dl,#0xFF-DEV_MASK		! save flags
	or	dl,dh
	add	cx,(di+1)		! apply low order translation
	adc	ax,(di+3)		! apply high order translation
	add	di,#5
	jmp	lnwrt

cmdwerr:
	pop	cx			! was AX
	jmp	cnok			! write dumb error message
#if DEBUG_NEW
cmdwr:	.ascii	"RAID: cmd line write to device "
	.byte	0
#endif

#endif

#endif

! Sector read
!  trashes CX and DI

cread:  
        test    dl,#LINEAR_FLAG|LBA32_FLAG
        jnz     use_linear

        push    ax              ;save the count
	mov	ah,#2		;read command
        call    dsk_do_rw       ; int 0x13 with retries
        pop     cx              ;Carry Set means error on read
        mov     al,cl           ;count in AL, error code in AH
        ret

use_linear:
        mov     ah,hinib        ;will be zero for LINEAR
        xchg    al,dh           ;AX is possible address 
	test	dl,#LBA32_FLAG	;test for LBA32/LINEAR	*****
	jz	lnread		;pure LINEAR		*****
        test    dl,#LBA32_NOCOUNT
        jz      lnread
        mov     ah,dh           ;former count is really hi-nibble
        mov     hinib,ah
        mov     dh,#1           ;set count to 1
lnread:
        xchg    di,ax           ;hi-address to DI
        mov     al,dh           ;count to AL

	test	dl,#RAID_REL_FLAG		; ******
	jz	ln_do_read			; ******

	BEG_FS
	SEG_FS
	add	cx,par1_raid_offset+SSDIFF	; ***** RAID ******
	SEG_FS
	adc	di,par1_raid_offset+2+SSDIFF	; ***** RAID ******
	END_FS

ln_do_read:
        call    lba_read
        mov     al,cl           ;count returned in AL, error code in AH
        ret                     ;Carry Set means error on read

#ifdef RAID_NEW
; raid_search -- search for other raid disks which parallel this one
;
;  enter with DS:BX pointing to buffer to use (DESCR) for searches
;         and with ES=DS=CS
;
;  side effect is to set up the raid translation table for writes
;
;
raid_search:
	push	es
#if DEBUG_NEW
	mov	byte ptr raid_nn,#0	;set number of raid disks to 0
	call	crlf
#endif
	BEG_FS
	SEG_FS
	mov	dl,par1_descr+sa_device+SSDIFF	; get map file drive

; enable matching RAID boot blocks by clearing the DEFEAT flag
	SEG_FS
	and	byte ptr par1_prompt+SSDIFF,#0xFF-FLAG_RAID_DEFEAT

	SEG_FS
	test	byte ptr par1_prompt+SSDIFF,#FLAG_RAID
	END_FS
#if DEBUG_NEW
#ifdef LCF_M386
	jz	near rs_ret
#else
	beq	rs_ret
#endif
#else
	jz	rs_ret
#endif

	and	dx,#0xF0		; save LINEAR/LBA32/LBA32_NOCOUNT
#if 0
	cmp	dl,#0x80		; must be 0x80 for this to be raid
#if DEBUG_NEW
#ifdef LCF_M386
	jne	near rs_ret
#else
	bne	rs_ret
#endif
#else
	jne	rs_ret
#endif
#endif

	mov	bp,#16			; test 16 bios devices
	push	#raid_tt		; set pointer
rs_next:
;;;;	inc	dl			; move to next drive
	push	ds
	pop	es
	mov	cx,#1
#if DEBUG_NEW
	pusha
	push	dx
	mov	bx,#rs_test
	call	say
	pop	ax
	call	bout
	popa
#endif
	push	dx			; protect LBA32/LINEAR flags
	and	dx,#DEV_MASK		; mask flag bits; clear DH, too (0x008F)
	mov	al,cl			; read 1 sector
	call	cread			; read the boot sector (MBR)
					; no RAID relocation will happen
					; because we use CHS addressing
	pop	dx
	jc	rs_done

	pop	di
	call	raid_compare		; watch out for DI - raid ptr in
					; the stack
#ifdef RAID_NEW
	push	bx
	mov	cx,#4
	lea	si,(bx+PART_TABLE_OFFSET)
	add	bh,#SECTOR_SIZE>>8
rs_chk_pri:
#if 0
	cmp	byte ptr (si+4),#0xFD	; check for raid partition
	jne	rs_chk_pri_next
#endif
	cmp	byte ptr (si+4),#0	; check for empty entry
	je	rs_chk_pri_next
	push	cx
	push	di
#if DEBUG_NEW
	push	bx
	mov	bx,#rsckpar
	call	say
	mov	al,#5
	sub	al,cl
	call	nout
	pop	bx
#endif
	mov	al,#1
	mov	cx,(si+8)
	mov	di,(si+8+2)
	call	lba_read
	pop	di
	call	raid_compare
	pop	cx
rs_chk_pri_next:
	add	si,#16			; next partition table entry
	loop	rs_chk_pri
	pop	bx
	push	di
#endif  /* CHECK_PRI_TOO */

	inc	dl			; move to next drive
	dec	bp
	jnz	rs_next			; try next device code

rs_done:
	pop	di
	mov	byte ptr (di),#0		; mark zero on end
#if DEBUG_NEW
	pusha
	call	crlf
	mov	bx,#rs_found
	call	say
	mov	si,#raid_nn
	lodsb
	call	nout
	mov	bx,#rs_found2
	call	say
rs_debug1:
	mov	bx,#rs_dev
	call	say
	lodsb
	call	bout
	mov	bx,#rs_reloc
	call	say
#ifdef LCF_M386
	lodsd
	push	eax
	call	dout
#else
	lodsw
	push	ax
	lodsw
	call	wout
	pop	ax
	call	wout
#endif
	call	crlf
	test	byte ptr (si),#0xFF
	jnz	rs_debug1
	call	pause
	popa
#endif
rs_ret:
	pop	es
	ret

; raid compare
;  
;   enter with  DI = raid pointer 
;   DS:BX  points at the buffer that was read
;     DL is device code
;
raid_compare:
	push	es
	push	si
	push	di

	mov	si,bx			; compare what is read
	xor	di,di			; to our first stage loader
#ifdef LCF_M386
	push	fs
	pop	es
#else
	seg	cs
	mov	es,firstseg
#endif
	mov	cx,#par1_raid_offset-par1_cli	; signature
	repe
		cmpsb
	jne	rs_skip2
	add	si,#8
	add	di,#8			; skip raid_reloc & timestamp
	mov	cx,#par1_size - par1_raid_offset - 8
	repe
		cmpsb
rs_skip2:
#if DEBUG_NEW
	pushf
	pusha
	mov	bx,#rs_nomatch
	jne	rs_nm
	mov	bx,#rs_match
rs_nm:	call	say
	popa
	popf
#endif
	jne	rs_skip
; we found two boot records that are identical
#if DEBUG_NEW
	inc	raid_nn
#endif
#ifdef LCF_M386
	mov	eax,(bx+par1_raid_offset)
#else
	mov	ax,(bx+par1_raid_offset)
	mov	cx,(bx+par1_raid_offset+2)
#endif
#ifdef LCF_M386
	seg	fs
	sub	eax,par1_raid_offset
#else
	seg	es			; ES == FS
	sub	ax,par1_raid_offset
	seg	es
	sbb	cx,par1_raid_offset+2
	push	ax
	or	ax,cx		; do the zero test
	pop	ax
#endif
	jnz	rs_insert

	seg	es		; ES == FS
	cmp	dl,par1_dflcmd+sa_device
#if DEBUG_NEW
	jne	rs_insert
	pusha
	mov	bx,#rs_omitted
	call	say
	popa
	jmp	rs_skip
#else
	je	rs_skip
#endif
rs_insert:
	pop	di
	mov	(di),dl				; save device
#ifdef LCF_M386
	mov	(di+1),eax			; save offset
#else
	mov	(di+1),ax			; save offset lo
	mov	(di+1+2),cx			; save offset hi
#endif
	add	di,#5
	push	di
rs_skip:
	pop	di
	pop	si
	pop	es
	ret

#if DEBUG_NEW
raid_nn:	.byte	0	; must precede raid_tt for printout
#endif
raid_tt:	.blkb	5*16	; format is    drive:      .byte
				;              translate:  .long
		.byte   0
#if DEBUG_NEW
rs_test:	.ascii	"RAID: Testing"
rs_dev:		.ascii	" device "
		.byte	0
rs_nomatch:	.ascii	" no"
rs_match:	.ascii	" match"
		.byte	10,0
rs_omitted:	.ascii	"    omitted"
		.byte	10,0
rs_found:	.ascii	"RAID: found "
		.byte	0
rs_found2:	.ascii	" matching MBR(s)"
		.byte	10,0
rs_reloc:	.ascii	"  reloc "
		.byte	0
rsckpar:	.ascii	"  Checking partition "
		.byte	0
#endif
#endif

#if 1
; crc32 -- calculate CRC-32 checksum
;
;	call:
;		push	dword #POLYNOMIAL
;
;		ES:SI	char string pointer
;		DI	count of characters
;
;		call	crc32
;
;    CRC-32 is returned in EAX  or  DX:AX
;		the arguments are popped from the stack
;
crc32:
		push	bp
		mov	bp,sp

		push	si
		push	di
		push	bx
		push	cx

#ifdef LCF_M386
		mov	eax,#-1		; initialize CRC
#else
		mov	ax,#-1
		cwd
#endif
		inc	di
crc32a:
		dec	di
		jz	crc32d
		mov	cx,#8		; count 8 bits
		seg es
		mov	bl,(si)		; get next character
		inc	si
crc32b:		shl	bx,#1		; get hi bit of char in BH
#ifdef LCF_M386
		shl	eax,#1		; shift hi bit out of CRC
#else
		shl	ax,#1
		rcl	dx,#1
#endif
		adc	bh,#0		; add carry to BH
		shr	bh,#1		; put bit in carry
		jnc	crc32c		; skip the xor
#ifdef LCF_M386
		xor	eax,(bp+4)	; xor in the polynomial
#else
		xor	ax,(bp+4)
		xor	dx,(bp+6)
#endif
crc32c:
		loop	crc32b		; loop back for 8 bits
		jmp	crc32a

crc32d:		
#ifdef LCF_M386
		not	eax		; finialize CRC
#else
		not	ax
		not	dx
#endif

		pop	cx
		pop	bx
		pop	di
		pop	si
		
		leave
		ret	4
#endif

#ifdef SHS_PASSWORDS
#include "shs3.S"
#endif
#include "read.S"
#ifdef LCF_BDATA
#include "bdata.h"
#include "biosdata.S"
#endif
#ifdef MENU
#include "graph.S"
#include "menu.S"
#include "strlen.S"
#include "crt.S"
#endif
#ifdef BITMAP
#include "bitmap.S"
#include "strlen.S"
#include "display4.S"
#endif

! Put tokens into keyboard buffer

putkbd:	add	si,#4		! skip over "kbd="
	push	es
	xor	ax,ax		! set ES to zero
	mov	es,ax
pknext:	lodsb			! get next byte
	or	al,al		! NUL ?
	jz	pkdone		! yes -> done
	cmp	al,#32		! blank ?
	jne	pkrd		! no -> read scan code
pkdone:	dec	si		! return last character
	pop	es		! done
	ret
pkrd:	xor	cx,cx		! clear accumulator
pkrdlp:	cmp	al,#97		! lower case character ?
	jb	pknol		! no -> go on
	sub	al,#32		! make upper case
pknol:	sub	al,#48		! normalize
	cmp	al,#10		! >"9" ?
	jb	pkok		! no -> okay
	cmp	al,#17		! <"A" ?
	jb	pksyn		! yes -> syntax error
	sub	al,#7		! adjust
	cmp	al,#16		! >"F" ?
	jae	pksyn		! yes -> syntax error
pkok:	shl	cx,1		! shift CX
	jc	pksyn		! carry means trouble
	shl	cx,1
	jc	pksyn
	shl	cx,1
	jc	pksyn
	shl	cx,1
	jc	pksyn
	add	cl,al		! put in lowest nibble
	lodsb			! get next byte
	or	al,al		! NUL ?
	jz	pkend		! yes -> at end
	cmp	al,#32		! space ?
	je	pkend		! yes -> at end
	cmp	al,#44		! comma ?
	je	pkmore		! yes -> end of token
	jmp	pkrdlp		! token continues
pksyn:	mov	bx,#msg_pks	! complain
	call	say
pkfls:	lodsb			! flush to end of option
	or	al,al
	jz	pkdone
	cmp	al,#32
	je	pkdone
	jmp	pkfls
pkend:	call	pkput		! store token
	jmp	pkdone		! ... and return
pkmore:	call	pkput		! store token
	jmp	pknext		! handle next token
pkput:	seg	es		! get buffer pointer
	mov	bx,[KBEND]
	mov	dx,bx
	add	dx,#2		! increment it
	cmp	dx,#KBHIGH	! (wrap around end)
	jb	pknadj
	mov	dx,#KBLOW
pknadj: seg	es		! buffer full ?
	cmp	dx,[KBBEG]
	je	pkfull		! yes -> error
	seg	es		! store scan code
	mov	(bx+0x400),cx
	seg	es		! store new pointer
	mov	[KBEND],dx
	ret			! done
pkfull:	mov	bx,#msg_pkf	! complain
	call	say
	pop	ax		! discard return address
	jmp	pkfls		! abort

! Set VGA mode

setvga:	add	si,#4		! skip over "vga="
	push	si		! save SI
	mov	bx,#vgatab	! scan VGA table
svgatb:	pop	si		! get pointer to option value
	push	si
	mov	cx,(bx)		! get VGA code
	or	cx,cx		! at end ?
	jz	vganum		! yes -> must be numeric
	add	bx,#2		! compare the strings
vgacmp:	lodsb
	call	upcase		! (case-insensitive)
	mov	ah,(bx)
	inc	bx
	or	ah,ah		! at end ?
	jnz	vgamore		! no -> go on
	or	al,al		! at end of line ?
	jz	vgafnd		! yes -> found it
	cmp	al,#32		! space ?
	je	vgafnd		! yes -> found it
	jmp	svgatb		! try next entry otherwise
vgamore:cmp	al,ah
	je	vgacmp		! equal -> next character
vgaskp:	mov	al,(bx)		! skip to end of reference string
	inc	bx
	or	al,al
	jnz	vgaskp
	jmp	svgatb		! try next entry
vgafnd:	pop	ax		! drop SI
vgaput:	mov	vgaovr,cx	! set VGA mode
	dec	si		! read last character again
	clc			! okay, done
	ret
vganum:	pop	si		! get SI
	xor	cx,cx
	mov	ah,cl
	test	byte ptr (si),#0xff ! no value ?
	jz	vgaerr		! yes -> error
vgadig:	lodsb			! get the next character
	or	al,al		! at end ?
	jz	vgaput		! yes -> done
	cmp	al,#32
	je	vgaput
	cmp	al,#48		! is it a digit ?  (0x30=48="0")
	jb	vgaerr		! no -> error
	cmp	al,#57		! 57=0x39="9"
	ja	vgaerr
	sub	al,#48		! cx = cx*10+al-'0'
#if 0
	mov	bx,cx
	shl	cx,1
	shl	cx,1
	add	cx,bx
	shl	cx,1
#else
	imul	cx,#10
#endif
	add	cx,ax
	jnc	vgadig		! next one
vgaerr:	mov	bx,#msg_v	! display an error message
	call	say
	stc			! return an error
	ret

vgatab:
#ifdef NORMAL_VGA
	.word	ASK_VGA
	.ascii	"ASK"
	.byte	0
	.word	EXTENDED_VGA
	.ascii	"EXTENDED"
	.byte	0
	.word	EXTENDED_VGA
	.ascii	"EXT"
	.byte	0
	.word	NORMAL_VGA
	.ascii	"NORMAL"
	.byte	0
#endif
	.word	0

! Set memory limit

getmem:	cmp	byte ptr (si+4),#0x6e ! 'n' like 'nopentium' ?
	jne	mlreal		! no -> proceed
	ret			! nice try
mlreal:	push	si		! save SI for copying
	add	si,#4		! advance SI to beginning of number
	call	strtoul		! get number in DX:AX
#if 0
	push ax
	push dx
	push ax
	mov ax,dx
	call wout
	pop ax
	call wout
	pop dx
	pop ax
#endif
	mov	bl,(si)		! get next character
	cmp	bl,#0x4b	! 'K' or 'k' ?
	je	mlthis		! yes -> do not change
	cmp	bl,#0x6b
	je	mlthis
	mov	cx,#10		! divide or multiply by 2^10
	cmp	bl,#0x4d	! 'M' or 'm' ?
	je	mlmul		! yes-> multiply
	cmp	bl,#0x6d
	je	mlmul
	cmp	byte ptr (si),#0 ! NUL ?
	je	mldivl		! yes -> divide
	cmp	byte ptr (si),#32
;;;	je	mldivl		! yes -> divide
	jne	s2lbad		! trouble
mldivl:	shr	dx,1		! shr DX:AX,1
	rcr	ax,1
	loop	mldivl		! ten times
	jmp	mlthis		! done
mlmul:	or	dx,dx		! too big already ?
	jnz	mlbig		! yes -> set to 0xffff
mlmull:	shl	ax,1		! shl DX:AX,1
	rcl	dx,1
	loop	mlmull		! ten times
mlthis:	or	dx,dx		! too big ?
	jz	mlnbig		! no -> use AX
mlbig:	mov	ax,#0xffff	! use maximum
mlnbig:	mov	memlim,ax	! set memory limit
	pop	si		! restore SI
	ret			! done


strtoul:          /* string to unsigned long in DX:AX */
	xor     ax,ax
	xor     dx,dx
	mov     cx,#10          ! default radix is decimal
	cmp     byte ptr (si),#0x30        ! == '0'?
	jne     s2lnext
	inc     si
	dec     cx
	dec     cx              ! assume octal : CX = 8
	
	cmp     byte ptr (si),#0x58        ! == 'X'?
	je      s2lhex
	cmp     byte ptr (si),#0x78        ! == 'x'?
	jne     s2lnext
s2lhex: add	cx,cx		! it is hexadecimal
	inc     si
s2lnext:
        xor     bx,bx
        mov     bl,(si)         ! get next character

	sub     bx,#0x30        ! - '0'
	jb      s2ldone
	cmp     bl,cl           ! compare to radix
	jb      s2lmul
	add     bx,#0x30
	and     bl,#0xFF-0x20   ! convert to Upper Case
	sub     bx,#0x41        ! - 'A'
	cmp     bl,#6
	jnb     s2ldone
	add     bl,#10	
s2lmul:
        push    dx              ! save high order
	mul     cx              ! multiply by radix
	add     ax,bx
	adc     dx,#0           ! carry possible only in radix 10
	pop     bx
	push    dx
	xchg    ax,bx
	mul     cx
	or      dx,dx
	jnz     s2lbad
	pop     dx
	add     dx,ax
	jc      s2lbad
	xchg    ax,bx
	inc     si
	jmp     s2lnext

s2lbad:	mov	bx,#msg_s2l	! complain
	call	say
	br	restrt		! start over again
	
s2ldone: ret


msg_s2l:.byte	10
	.ascii	"Invalid number"
	.byte	10,0

! GDT for "high" loading

gdt:	! space for BIOS
	.blkb	0x10
	! source
	.word	0xffff		! no limits
	.word	0		! start: 0x10000
	.byte	1
	.byte	0x93		! permissions
	.word	0		! padding for 80286 mode :-(
	! destination
	.word	0xffff		! no limits
	.word	0		! start - filled in by user
	.byte	0
	.byte	0x93		! permissions
	.word	0		! padding for 80286 mode :-(
	! space for BIOS
	.blkb	0x10

! Some messages

msg_p:	.ascii	"boot: "
	.byte	0

msg_l:	.ascii	"Loading "
	.byte	0

msg_re:	.byte	10
	.ascii	"Error 0x"
	.byte	0

msg_nf:	.ascii	"No such image. [Tab] shows a list."
	.byte	10,0

msg_chkerr:
	.ascii	"O - Descriptor checksum error\n"
	.byte	0
msg_sigerr:
	.ascii	"O - Signature not found\n"
	.byte	0

#ifdef MEMORY_CHECK
msg_mem: .ascii	"EBDA too big"
	.byte	10,0
#endif

msg_int:.byte	10
	.ascii	"*Interrupted*"
	.byte	10,0

msg_eof:.byte	10
	.ascii	"Unexpected EOF"
	.byte	10,0

msg_pw:	.ascii	"Password: "
	.byte	0

msg_pf:	.ascii	"Sorry."
	.byte	10,0

msg_v:	.byte	10
	.ascii	"Valid vga values are ASK, NORMAL, EXTENDED or a "
	.ascii	"decimal number."
	.byte	10,0

msg_pks:.byte	10
	.ascii	"Invalid hexadecimal number. - Ignoring remaining items."
	.byte	10,0

msg_pkf:.byte	10
	.ascii	"Keyboard buffer is full. - Ignoring remaining items."
	.byte	10,0

msg_bm:	.byte	10
	.ascii	"Block move error 0x"
	.byte	0

msg_rd:	.byte	10
	.ascii	"Not enough memory for RAM disk"
	.byte	10,0

ospc:	.ascii	"O"
#ifdef LCF_BEEP
	.byte	7
#endif
#ifdef LCF_VERSION
	.ascii	" "	
	.ascii	S(VERSION_MAJOR)
	.ascii	"."
	.ascii	S(VERSION_MINOR)
	.ascii	VERSION_EDIT
#endif
	.byte	32,0

bs:	.byte	8,32,8,0

#ifdef DEBUG
stepa:	.ascii	" RAM disk,"
	.byte	0
step0:	.ascii	" map page,"
	.byte	0
step0b:	.ascii	" fallback,"
	.byte	0
step1:	.ascii	" options,"
	.byte	0
step1b:	.ascii	" fallback,"
	.byte	0
step2:	.ascii	" boot,"
	.byte	0
step3:	.ascii	" setup,"
	.byte	0
step4:	.ascii	" system "
	.byte	0

sax:	.ascii	"AX="
	.byte	0
sbx:	.ascii	" BX="
	.byte	0
scx:	.ascii	" CX="
	.byte	0
sdx:	.ascii	" DX="
	.byte	0
ses:	.ascii	" ES="
	.byte	0
sdone:	.byte	10
	.byte	0

mov_ar:	.ascii	" -> "
	.byte	0
mov_sz:	.ascii	", words "
	.byte	0
#endif

	.even

#ifdef VAR_LOAD
stackseg:
initseg:	.word	INITSEG
setupseg:	.word	SETUPSEG
#endif

#if defined(MENU) || defined(BITMAP)
suppress:.word	0		! suppress console output (MUST be word)
#endif
hinib:  .byte   0               ;;; hi-nibble of address
tempal:	.byte	0
moff:	.word	0		! map offset

cntdown:.word	0		! count-down
timeout:.byte	0		! timed out

#ifdef LCF_BDATA
no_bdata: .byte	0		! flag no bdata call if !=0
#endif

int1c_l:.word	0		! old timer interrupt
int1c_h:.word	0

old_del:.word	0		! delay before booting

nodfl:	.word	0		! action if no defaults are present

#ifndef LCF_NOSERIAL
slbase:	.word	0		! serial port base (or 0 if unused)
break:	.byte	0		! break received flag
#endif

usrinpm:.byte	UI_MAGIC

cmdbeg:	.word	0
options:.word	0

rdmid:	.word	0		! RAM disk address, "middle" part
rdszl:	.word	0		! RAM disk size
rdszh:	.word	0

vgaovr:	.word	0		! VGA mode overwrite
memlim:	.word	0		! memory limit
dskprm:	.word	0,0,0,0,0,0

#ifndef LOCK_BSS
lkwbuf:	.word	DC_MAGIC
lkcbuf:	.blkb	256
#endif
dolock:	.byte	0

	.even			! control alignment from here down
acmdbeg:.ascii	"auto "
mcmdbeg:.ascii	"BOOT_IMAGE"
prechr:	.byte	32		! space: guard double blank supression
				! equal sign: variable assignment
cmdline:.byte	0
#ifdef BITMAP
_line:		! must be 640 bytes long
#endif
theend:
#ifdef LOCK_BSS
lkwbuf	=	cmdline+CL_LENGTH+2	; this is a word
lkcbuf	=	lkwbuf+2
theend2 =	lkcbuf+CL_LENGTH	; lkcbuf is 256
#endif
the_end1	= theend+511
theends	=	the_end1/512
