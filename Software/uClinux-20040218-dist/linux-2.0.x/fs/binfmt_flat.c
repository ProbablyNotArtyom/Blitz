/*
 *  linux/fs/binfmt_flat.c
 *
 *  Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>
 *
 *  This is a relatively simple binary format, intended solely to contain
 *  the bare minimum needed to load and execute simple binaries, with
 *  special attention to executing from ROM, when possible.
 *
 *  Originally based on:
 *
 *  linux/fs/binfmt_aout.c
 *
 *  Copyright (C) 1991, 1992, 1996  Linus Torvalds
 */
 

#include <linux/module.h>

#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/a.out.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/malloc.h>
#include <linux/binfmts.h>
#include <linux/personality.h>
#include <linux/flat.h>
#include <linux/config.h>

#include <asm/system.h>
#include <asm/segment.h>
#include <asm/pgtable.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>

#define MAX(a,b) ((a) <= (b) ? (b) : (a))

#undef DEBUG

#ifdef DEBUG
#define	DBG_FLT(a...)	printk(##a)
#else
#define	DBG_FLT(a...)
#endif

#ifdef __i960__
#define STACK_GROWS_UP
#endif

#ifdef STACK_GROWS_UP
#define SP_PUSH_NBYTES(sp,nbytes)	((char*)sp) += nbytes
#define SP_PUSH_NBYTES_DATA(sp,nbytes,data)	do {	\
	memcpy(sp, data, nbytes);	\
	SP_PUSH_NBYTES(sp,nbytes);	\
} while (0)
#else
#define SP_PUSH_NBYTES(sp,nbytes)	((char*)sp) -= nbytes
#define SP_PUSH_NBYTES_DATA(sp,nbytes,data)	do {	\
	SP_PUSH_NBYTES(sp,nbytes);	\
	memcpy(sp, data, nbytes);	\
} while (0)
#endif

static int load_flat_binary(struct linux_binprm *, struct pt_regs * regs);

extern void dump_thread(struct pt_regs *, struct user *);

static struct linux_binfmt flat_format = {
#ifndef MODULE
	NULL, NULL, load_flat_binary, NULL, NULL
#else
	NULL, &mod_use_count_, load_flat_binary, NULL, NULL
#endif
};


static unsigned long putstring(unsigned long p, char * string)
{
	unsigned long l = strlen(string)+1;

	DBG_FLT("put_string '%s'\n", string);
	SP_PUSH_NBYTES_DATA((char*)p,l,string);
	return p;
}


static unsigned long putstringarray(unsigned long p, int count, char ** array)
{
	DBG_FLT("putstringarray(%d)\n", count);
#ifdef STACK_GROWS_UP
    { 
		int ii;
		for(ii=0; ii < count; ii++) {
			p=putstring(p, array[ii]);
			DBG_FLT("p2=%x\n", (unsigned int)p);
		}
	}
#else
	while(count) {
		p=putstring(p, array[--count]);
		DBG_FLT("p2=%x\n", (unsigned int)p);
	}
#endif
	return p;
}


static unsigned long stringarraylen(int count, char ** array)
{
	int l = 4;
	while(count) {
		l += strlen(array[--count]);
		l++;
		l+=4;
	}
	return l;
}


#ifdef CONFIG_BINFMT_ZFLAT
/*
 * this is fairly harmless unless you use it.  It hasn't had a lot
 * of testing but I have run systems with every binary compressed (davidm)
 *
 * here are the zlib hacks - to replace globals with locals
 */

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;
#define INBUFSIZ 4096
#define WSIZE 0x8000    /* window size--must be a power of two, and */
                        /*  at least 32K for zip's deflate method */
struct s_zloc
{
	struct inode *inode;
	unsigned long start_pos;
	unsigned long bytes2read;
	char *data_pointer, *out_pointer;
	uch *inbuf;
	uch *window;
	unsigned insize;  /* valid bytes in inbuf */
	unsigned inptr;   /* index of next byte to be processed in inbuf */
	unsigned outcnt;  /* bytes in output buffer */
	int exit_code;
	long bytes_out;
	int crd_infp, crd_outfp;
	ulg bb;                         /* bit buffer */
	unsigned bk;                    /* bits in bit buffer */
	ulg crc_32_tab[256];
	ulg crc; /* shift register contents */
	unsigned hufts;
};

static int fill_inbuf(struct s_zloc *zloc)
{
	int i;
	if(zloc->exit_code)
		return -1;
	i = (zloc->bytes2read > INBUFSIZ) ? INBUFSIZ : zloc->bytes2read;
	if((i = read_exec(zloc->inode, zloc->start_pos, zloc->data_pointer, i, 0)) >= (unsigned long)-4096)
		return -1;
	zloc->bytes2read -= i;
	zloc->start_pos += i;
	zloc->insize = i;
	zloc->inptr = 1;
	return zloc->inbuf[0];
}

static void flush_window(struct s_zloc *zloc)
{
	ulg c = zloc->crc;
	unsigned n;
	uch *in, ch;
	memcpy(zloc->out_pointer, zloc->window, zloc->outcnt);
	in = zloc->window;
	for(n = 0; n < zloc->outcnt; n++)
	{
		ch = *in++;
		c = zloc->crc_32_tab[((int)c ^ch) & 0xff] ^(c >> 8);
	}
	zloc->crc = c;
	zloc->out_pointer += zloc->outcnt;
	zloc->bytes_out += (ulg)zloc->outcnt;
	zloc->outcnt = 0;
}

#define inbuf (zloc->inbuf)
#define window (zloc->window)
#define insize (zloc->insize)
#define inptr (zloc->inptr)
#define outcnt (zloc->outcnt)
#define exit_code (zloc->exit_code)
#define bytes_out (zloc->bytes_out)
#define crd_infp (zloc->crd_infp)
#define crd_infp (zloc->crd_infp)
#define bb (zloc->bb)
#define bk (zloc->bk)
#define crc (zloc->crc)
#define crc_32_tab (zloc->crc_32_tab)
#define hufts (zloc->hufts)

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf(zloc))
#define memzero(s, n)     memset ((s), 0, (n))

#define OF(args)  args
#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)

#define STATIC static

#define malloc(arg) kmalloc(arg, GFP_KERNEL)
#define free(arg) kfree(arg)

#define error(arg) printk("zflat:" arg "\n")

#include "../lib/inflate2.c"

#undef error

#undef malloc
#undef free

#undef inbuf
#undef window
#undef insize
#undef inptr
#undef outcnt
#undef exit_code
#undef bytes_out
#undef crd_infp
#undef crd_outp
#undef bb
#undef bk
#undef crc
#undef crc_32_tab

#undef get_byte
#undef memzero


static int decompress_exec(struct inode *inode, unsigned long offset, char * buffer, long len, int fd)
{
	struct s_zloc *zloc;
	int res;
	zloc = kmalloc(sizeof(*zloc), GFP_KERNEL);
	if(!zloc)
	{
		return -ENOMEM;
	}
	memset(zloc, 0, sizeof(*zloc));
	zloc->inode = inode;
	zloc->out_pointer = buffer;
	zloc->inbuf = kmalloc(INBUFSIZ, GFP_KERNEL);
	if(!zloc->inbuf)
	{
		kfree(zloc);
		return -ENOMEM;
	}
	zloc->window = kmalloc(WSIZE, GFP_KERNEL);
	if(!zloc->window)
	{
		kfree(zloc->inbuf);
		kfree(zloc);
		return -ENOMEM;
	}
	zloc->data_pointer = zloc->inbuf;
	zloc->bytes2read = len;
	zloc->start_pos = offset;
	zloc->crc = (ulg)0xffffffffL;
	makecrc(zloc);
	res = gunzip(zloc);
	kfree(zloc->window);
	kfree(zloc->inbuf);
	kfree(zloc);
	return res ? -ENOMEM : 0;
}
#endif /* CONFIG_BINFMT_ZFLAT */

#ifdef STACK_GROWS_UP
/*
 * create_flat_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the "stack", returning the new stack pointer value.
 *
 * This is the version for machines where the stack grows up, like on the the i960.
 *
 * This is the general picture of how things should be set up after everything's done:
 *
 * ^
 * |
 * |envp    -> envp[0]
 * |argv    -> argv[0]
 * |argc
 * |NULL
 * |argv[1] -> argv[1]
 * |argv[0] -> argv[0]
 * |NULL
 * |envp[1] -> env2[0]
 * |envp[0] -> env1[0]
 * |<some alignment space>                -> pp points after the arg2 string when called.
 * |arg2    <string>                      -> arg_end
 * |arg1    <string>
 * |env2    <string>                      -> arg_start, env_end
 * |env1    <string>
 * |filename<string>                      -> env_start points after the filename string.
 * -------------------------------------- 
 *
 */
static unsigned long create_flat_tables_stack_grows_up(unsigned long pp, struct linux_binprm * bprm, unsigned long env_start)
{
	unsigned long *argv,*envp;
	unsigned long * sp = (unsigned long*) pp; 
	char * p = (char*)env_start; /* p is the pointer we use to scan through the strings in the stack */
	int argc = bprm->argc;
	int envc = bprm->envc;

	/* align sp on quad-word boundary */
    /* This is probably i960 specific */
	if ((unsigned long)sp & (15)) {
		sp+=4;
		((unsigned long)sp) &= ~15;
	}

    DBG_FLT("pp: %X, p: %X, sp: %X\n", pp, p, sp);
    DBG_FLT("argc: %d, envc: %d\n", argc, envc);

    envp = sp;              /* envp[0] is right at the bottom of the stack now */
    argv = envp + envc + 1; /* The argv pointer points right after the terminating null of the envp array. */
    sp   = argv + argc + 1; /* argc, argv and envp will be on top of terminating null of the argv array. */
    
    DBG_FLT("argv: %x\n", argv);
    DBG_FLT("envp: %x\n", envp);
    
	put_user(argc,sp++);
    put_user(argv,sp++); /* The argv and envp pointers might not be used for other architecrures, */
    put_user(envp,sp++); /* but since only the i960 is supported, we won't optionnaly compile for clarity. */

    /* Create the envp array on the stack */
    current->mm->env_start = (unsigned long) p;    /* This what the real value should be */

	while (envc-- > 0) {
        DBG_FLT("envp(%p) p:%p, %s\n", envp, p, p);
		put_user(p,envp++);
		while (get_user(p++)) /* nothing */ ;
	}
	put_user(NULL,envp);

    /* Create the argv array on the stack */
	current->mm->env_end = current->mm->arg_start = (unsigned long) p;
	while (argc-- > 0) {
        DBG_FLT("argv(%p) p:%p, %s\n", argv, p, p);
		put_user(p,argv++);
		while (get_user(p++)) /* nothing */ ;
	}
	put_user(NULL,argv++);
    
    current->mm->arg_end =  (unsigned long) p;

    DBG_FLT(" current->mm->arg_end: %X, %p\n", current->mm->arg_end, current->mm->arg_end);
    DBG_FLT(" current->mm->env_start: %X, %p\n", current->mm->env_start, current->mm->env_start);
	return (unsigned long)sp; /* We return the address right after the envp. */
}
#endif

#ifndef STACK_GROWS_UP
/*
 * create_flat_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the "stack", returning the new stack pointer value.
 *
 * This is the version for most machines where the stack grows down.
 */
static unsigned long create_flat_tables_stack_grows_down(unsigned long pp, struct linux_binprm * bprm)
{
	unsigned long *argv,*envp;
	unsigned long * sp;
	char * p = (char*)pp;
	int argc = bprm->argc;
	int envc = bprm->envc;

	sp = (unsigned long *) (pp & 0xfffffffc);	/* align pointers */
	sp -= envc+1;
	envp = sp;
	sp -= argc+1;
	argv = sp;

#if defined(__sparc__)
	/* Sparc requires stacks to start on 8byte boundary */
	sp = (unsigned long *) (((unsigned long) sp) & 0xfffffff8);
	put_user(0,--sp);
#endif
#if defined(__i386__) || defined(__mc68000__) || defined(__arm__) || defined(__sparc__) || defined(__H8300H__)
	put_user(envp,--sp);
	put_user(argv,--sp);
#endif
	put_user(argc,--sp);
	current->mm->arg_start = (unsigned long) p;
	while (argc-- > 0) {
		put_user(p,argv++);
		while (get_user(p++)) /* nothing */ ;
	}
	put_user(NULL,argv);
	current->mm->arg_end = current->mm->env_start = (unsigned long) p;
	while (envc-- > 0) {
		put_user(p,envp++);
		while (get_user(p++)) /* nothing */ ;
	}
	put_user(NULL,envp);
	current->mm->env_end = (unsigned long) p;
	return (unsigned long)sp;
}
#endif

#ifdef __sparc__
/*
 *	For SPARC architecture we need to handle the funky HI22 and LO10
 *	addressing modes. The relocations occur inside of an instruction,
 *	and so we need to preserve the instruction bits but also perform
 *	the relocation on the address component.
 */

static inline unsigned long get_relocate_addr(unsigned long rel)
{
	return(rel & 0x3fffffff);
}

static inline unsigned long get_addr_from_val(unsigned long val, unsigned long relval)
{
	unsigned long addr = val;
	if (relval & 0x80000000)
		addr &= 0x003fffff;
	else if (relval & 0x40000000)
		addr &= 0x000003ff;
	return(addr);
}

static inline unsigned long put_addr_into_val(unsigned long val, unsigned long addr, unsigned long relval)
{
	if (relval & 0x80000000)
		val = (val & 0xffc00000) | (addr >> 10);
	else if (relval & 0x40000000)
		val = (val & 0xfffffc00) | (addr & 0x3ff);
	else
		val = addr;
	return(val);
}

#else

/*
 *	On most architectures a relocation is a single value update.
 *	No extra information needs to be preserved in the long word
 *	that contains an address to relocate.
 */

static inline unsigned long get_relocate_addr(unsigned long rel)
{
	return(rel);
}

static inline unsigned long get_addr_from_val(unsigned long val, unsigned long relval)
{
	return(val);
}

static inline unsigned long put_addr_into_val(unsigned long val, unsigned long addr, unsigned long relval)
{
	return(addr);
}

#endif  /* ! __sparc__ */


static unsigned long
calc_reloc(unsigned long r, unsigned long text_len)
{
	unsigned long addr;

	if (r > current->mm->start_brk - current->mm->start_data + text_len) {
		printk("BINFMT_FLAT: reloc outside program 0x%x (0 - 0x%x), "
			"killing!\n", (int) r,
			(int)(current->mm->start_brk-current->mm->start_code));
		send_sig(SIGSEGV, current, 0);
		/* return something safe to write to */
		return(current->mm->start_brk);
	}

	if (r < text_len) {
		/* In text segment */
		return r + current->mm->start_code;
	}

	/*
	 * we allow inclusive ranges here so that programs may do things
	 * like reference the end of data (_end) without failing these tests
	 */
	addr =  r - text_len + current->mm->start_data;
	if (addr >= current->mm->start_code &&
			addr <= current->mm->start_code + text_len)
		return(addr);

	if (addr >= current->mm->start_data &&
			addr <= current->mm->start_brk)
		return(addr);

	printk("BINFMT_FLAT: reloc addr outside text/data 0x%x "
			"code(0x%x - 0x%x) data(0x%x - 0x%x) killing\n", (int) addr,
			(int) current->mm->start_code,
			(int) (current->mm->start_code + text_len),
			(int) current->mm->start_data,
			(int) current->mm->start_brk);
	send_sig(SIGSEGV, current, 0);

	return(current->mm->start_brk); /* return something safe to write to */
}


void old_reloc(unsigned long rl)
{
#ifdef DEBUG
	char *segment[] = { "TEXT", "DATA", "BSS", "*UNKNOWN*" };
#endif
	flat_v2_reloc_t	r;
	unsigned long *ptr;
	
#if !defined(__H8300H__)
	r.value = rl;
#if defined(CONFIG_COLDFIRE) || defined(__i960__)
	ptr = (unsigned long *) (current->mm->start_code + r.reloc.offset);
#else
	ptr = (unsigned long *) (current->mm->start_data + r.reloc.offset);
#endif

#ifdef DEBUG
    printk("Reloc: %X\n", rl);
    printk("Reloc.value: %X, Reloc.type: %x, reloc.offset:%x\n", r.value, r.reloc.type, r.reloc.offset);
	printk("Relocation of variable at DATASEG+%x "
           "(address %p, currently %x) into segment %s\n",
           r.reloc.offset, ptr, (int)*ptr, segment[r.reloc.type]);
#endif
	
	switch (r.reloc.type) {
	case OLD_FLAT_RELOC_TYPE_TEXT:
		*ptr += current->mm->start_code;
		break;
	case OLD_FLAT_RELOC_TYPE_DATA:
		*ptr += current->mm->start_data;
		break;
	case OLD_FLAT_RELOC_TYPE_BSS:
		*ptr += current->mm->end_data;
		break;
	default:
		printk("BINFMT_FLAT: Unknown relocation type=%x\n", r.reloc.type);
		break;
	}

#ifdef DEBUG
	printk("Relocation became %x\n", (int)*ptr);
#endif
#else
	ptr = (unsigned long *)(current->mm->start_code+rl);
	*ptr+=current->mm->start_code;
#endif
}		


/*
 * These are the functions used to load flat style executables and shared
 * libraries.  There is no binary dependent code anywhere else.
 */

inline int
do_load_flat_binary(struct linux_binprm * bprm, struct pt_regs * regs)
{
	struct flat_hdr * hdr;
	struct file *file;
	unsigned long textpos = 0, datapos = 0, result;
	unsigned long text_len, data_len, bss_len, stack_len, flags;
	unsigned long memp = 0, memkasked = 0; /* for finding the brk area */
	unsigned long extra;
	unsigned long p = bprm->p;
	unsigned long *reloc = 0, *rp;
	int rev, i, relocs = 0;
#ifdef STACK_GROWS_UP
	unsigned long env_start;
#endif

	current->personality = PER_LINUX;
	
	file = current->files->fd[open_inode(bprm->inode, O_RDONLY)];
	
	DBG_FLT("BINFMT_FLAT: Loading file: %x\n", (int)file);
#ifdef DEBUG
	show_free_areas();
#endif

	hdr = (struct flat_hdr*)bprm->buf;
	
	if (flush_old_exec(bprm)) {
		printk("unable to flush\n");
		return -ENOMEM;
	}

	text_len  = ntohl(hdr->data_start);
	data_len  = ntohl(hdr->data_end) - ntohl(hdr->data_start);
	bss_len   = ntohl(hdr->bss_end) - ntohl(hdr->data_end);
	stack_len = ntohl(hdr->stack_size);
	relocs    = ntohl(hdr->reloc_count);
#if !defined(__H8300H__)
	flags     = ntohl(hdr->flags);
#else
	flags     = FLAT_FLAG_RAM;
#endif
	rev       = ntohl(hdr->rev);

	if (strncmp(hdr->magic, "bFLT", 4) ||
			(rev != FLAT_VERSION && rev != OLD_FLAT_VERSION)) {
		printk("BINFMT_FLAT: bad magic/rev (%d, need %d)\n",
				rev, (int) FLAT_VERSION);
		return -ENOEXEC;
	}

	/*
	 * fix up the flags for the older format,  there were all kinds
	 * of endian hacks,  this only works for the simple cases
	 */
	if (rev == OLD_FLAT_VERSION && flags)
		flags = FLAT_FLAG_RAM;

#ifndef CONFIG_BINFMT_ZFLAT
	if (flags & (FLAT_FLAG_GZIP|FLAT_FLAG_GZDATA)) {
		printk("Support for ZFLAT executables is not enabled.\n");
		return -ENOEXEC;
	}
#endif

	/* Make room on stack for arguments & environment */
	stack_len += strlen(bprm->filename) + 1;
	stack_len += stringarraylen(bprm->envc, bprm->envp);
	stack_len += stringarraylen(bprm->argc, bprm->argv);
	
	/*
	 * there are a couple of cases here,  the seperate code/data
	 * case,  and then the fully copied to RAM case which lumps
	 * it all together.
	 */
	if ((flags & (FLAT_FLAG_RAM|FLAT_FLAG_GZIP)) == 0) {
		/*
		 * this should give us a ROM ptr,  but if it doesn't we don't
		 * really care
		 */
		DBG_FLT("BINFMT_FLAT: ROM mapping of file (we hope)\n");

		textpos = do_mmap(file, 0, text_len, PROT_READ | PROT_EXEC, 0, 0);
		if (textpos >= (unsigned long) -4096)
			return(textpos);

		extra = MAX(bss_len + stack_len, relocs * sizeof(unsigned long)),

		datapos = do_mmap(0, 0, data_len + extra,
				PROT_READ|PROT_WRITE|PROT_EXEC, 0, 0);

		if (datapos >= (unsigned long)-4096) {
			printk("Unable to allocate RAM for process data, errno %d\n",
					(int)-datapos);
			do_munmap(textpos, 0);
			return datapos;
		}

		DBG_FLT("BINFMT_FLAT: Allocated data+bss+stack (%d bytes): %x\n",
				data_len + bss_len + stack_len, datapos);

#ifdef CONFIG_BINFMT_ZFLAT
		if (flags & FLAT_FLAG_GZDATA) {
			result = decompress_exec(bprm->inode, ntohl(hdr->data_start),
					(char *)datapos, data_len+(relocs*sizeof(unsigned long)),
					0);
		} else
#endif
			result = read_exec(bprm->inode, ntohl(hdr->data_start),
					(char *)datapos, data_len + extra, 0);
		if (result >= (unsigned long)-4096) {
			printk("Unable to read data+bss, errno %d\n", (int)-result);
			do_munmap(textpos, 0);
			do_munmap(datapos, 0);
			return result;
		}

		reloc = (unsigned long *) (datapos+(ntohl(hdr->reloc_start)-text_len));
		memp = datapos;
		memkasked = data_len + extra;

	} else {

		/*
		 * calculate the extra space we need to map in
		 */

		extra = MAX(bss_len + stack_len, relocs * sizeof(unsigned long)),

		textpos = do_mmap(0, 0, text_len + data_len + extra,
				PROT_READ | PROT_EXEC | PROT_WRITE, 0, 0);
		if (textpos >= (unsigned long)-4096)
			return textpos;

		datapos = textpos + text_len;
		reloc = (unsigned long *) (textpos + ntohl(hdr->reloc_start));
		memp = textpos;
		memkasked = text_len + data_len + extra;

#ifdef CONFIG_BINFMT_ZFLAT
		/*
		 * load it all in and treat it like a RAM load from now on
		 */
		if (flags & FLAT_FLAG_GZIP) {
			result = decompress_exec(bprm->inode, sizeof (struct flat_hdr),
					((char *) textpos) + sizeof (struct flat_hdr),
					text_len + data_len + (relocs * sizeof(unsigned long))
							- ntohl(hdr->entry), 0);
		} else if (flags & FLAT_FLAG_GZDATA) {
			result = read_exec(bprm->inode, 0, (char *)textpos, text_len, 0);
			if (result < (unsigned long)-4096)
				result = decompress_exec(bprm->inode, text_len, (char *)datapos,
						data_len + (relocs * sizeof(unsigned long)), 0);
		} else
#endif
		{
			result = read_exec(bprm->inode, 0,
					(char *)textpos, text_len + data_len + extra, 0);
		}
		if (result >= (unsigned long) -4096) {
			printk("Unable to load code+data+bss, errno %d\n",
					(int)-result);
			do_munmap(textpos, 0);
			return(result);
		}
	}

	DBG_FLT("BINFMT_FLAT: Allocated:\n"
			"code\t0x%x\tOx%x\n"
			"data\t0x%x\tOx%x\n"
			"bss\t-\tOx%x\n"
			"stack\t-\tOx%x\n",
			(int)textpos, (int)text_len, (int)datapos, 
			(int)data_len, (int)bss_len, (int)stack_len);
#ifdef DEBUG
	show_free_areas();
#endif

	current->mm->start_code = textpos + sizeof (struct flat_hdr);
	current->mm->end_code = textpos + text_len;
	current->mm->start_data = datapos;
	current->mm->end_data = datapos + data_len;
#ifdef NO_MM
	/*
	 *	set up the brk stuff (uses any slack left in data/bss/stack allocation
	 *	We put the brk after the bss (between the bss and stack) like other
	 *	platforms.
     *
     *  We leave the brk stuff in the same hole, even if the stack grows up.
	 */
	current->mm->start_brk = datapos + data_len + bss_len;
	current->mm->brk = (current->mm->start_brk + 3) & ~3;
	current->mm->end_brk = memp + (ksize((void *) memp) - 8) - stack_len;
#else  /* !NO_MM */ 
	current->mm->brk = datapos + data_len + bss_len;
#endif

	/*
	 * not sure if we should on do this for XIP ?
	 * the coldfire code never did this and was plenty stable
	 */
	if (bprm->inode->i_sb->s_flags & MS_SYNCHRONOUS) {
		DBG_FLT("Retaining inode\n");
		current->mm->executable = bprm->inode;
		bprm->inode->i_count++;
	} else
		current->mm->executable = 0;

	DBG_FLT("Load %s: TEXT=%x-%x DATA=%x-%x BSS=%x-%x\n",
		bprm->argv[0],
		(int) current->mm->start_code, (int) current->mm->end_code,
		(int) current->mm->start_data, (int) current->mm->end_data,
		(int) current->mm->end_data, (int) current->mm->start_brk);

	text_len -= sizeof(struct flat_hdr); /* the real code len */

	/*
	 * We've got two different sections of relocation entries.
	 * The first is the GOT which resides at the begining of the data segment
	 * and is terminated with a -1.  This one can be relocated in place.
	 * The second is the extra relocation entries tacked after the image's
	 * data segment. These require a little more processing as the entry is
	 * really an offset into the image which contains an offset into the
	 * image.
	 */
	
	if (flags & FLAT_FLAG_GOTPIC) {
		for (rp = (unsigned long *)datapos; *rp != 0xffffffff; rp++)
			*rp = calc_reloc(*rp, text_len);
	}

	/*
	 * Now run through the relocation entries.
	 * We've got to be careful here as C++ produces relocatable zero
	 * entries in the constructor and destructor tables which are then
	 * tested for being not zero (which will always occur unless we're
	 * based from address zero).  This causes an endless loop as __start
	 * is at zero.  The solution used is to not relocate zero addresses.
	 * This has the negative side effect of not allowing a global data
	 * reference to be statically initialised to _stext (I've moved
	 * __start to address 4 so that is okay).
	 */

	if (rev > OLD_FLAT_VERSION) {
		for (i=0; i < relocs; i++) {
			unsigned long val, addr, relval;

			/* Get the address of the pointer to be
			   relocated (of course, the address has to be
			   relocated first).  */
			relval = ntohl(reloc[i]);
			addr = get_relocate_addr(relval);
			rp = (unsigned long *) calc_reloc(addr, text_len);

			/* Get the pointer's value.  */
			val = get_unaligned(rp);

			if (val != 0) {
				addr = get_addr_from_val(val, relval);
				if ((flags & FLAT_FLAG_GOTPIC) == 0)
					addr = ntohl(addr);
				addr = calc_reloc(addr, text_len);
				val = put_addr_into_val(val, addr, relval);

				/* Write back the relocated pointer.  */
				put_unaligned (val, rp);
			}
		}
	} else {
		for (i=0; i < relocs; i++)
			old_reloc(ntohl(reloc[i]));
	}

	/*
	 * we have to clear the BSS/STACK after the relocation otherwise
	 * we trash the relocations that lived in that space in the filesystem
	 */
	memset((void*)(datapos + data_len), 0, bss_len + 
			(current->mm->end_brk - current->mm->start_brk) +
			stack_len);

	current->mm->rss = 0;
	current->suid = current->euid = current->fsuid = bprm->e_uid;
	current->sgid = current->egid = current->fsgid = bprm->e_gid;
 	current->flags &= ~PF_FORKNOEXEC;
        
	if (current->exec_domain && current->exec_domain->use_count)
		(*current->exec_domain->use_count)--;
	if (current->binfmt && current->binfmt->use_count)
		(*current->binfmt->use_count)--;
	current->exec_domain = lookup_exec_domain(current->personality);
	current->binfmt = &flat_format;
	if (current->exec_domain && current->exec_domain->use_count)
		(*current->exec_domain->use_count)++;
	if (current->binfmt && current->binfmt->use_count)
		(*current->binfmt->use_count)++;

#ifndef NO_MM
	/*set_brk(current->mm->start_brk, current->mm->brk);*/
#endif

#ifdef STACK_GROWS_UP
    /* The stack bottom is right at the end of the brk space. */
    p = ((current->mm->end_brk + 3) & ~3);
#else
	p = ((current->mm->end_brk + stack_len + 3) & ~3) - 4;
#endif
    
	DBG_FLT("p=%x\n", (int)p);
	p = putstringarray(p, 1, &bprm->filename);

#ifdef STACK_GROWS_UP
    DBG_FLT("env start,p=%x\n", (int)p);
    env_start = p;
#endif
    
	DBG_FLT("p(filename)=%x\n", (int)p);
	p = putstringarray(p, bprm->envc, bprm->envp);

	DBG_FLT("p(envp)=%x\n", (int)p);
	p = putstringarray(p, bprm->argc, bprm->argv);

	DBG_FLT("p(argv)=%x\n", (int)p);
#ifdef STACK_GROWS_UP
	p = create_flat_tables_stack_grows_up(p, bprm, env_start);
#else
    p = create_flat_tables_stack_grows_down(p, bprm);
#endif
	DBG_FLT("p(create_flat_tables)=%x\n", (int)p);
	DBG_FLT("arg_start = %x\n", (int)current->mm->arg_start);
	DBG_FLT("arg_end = %x\n", (int)current->mm->arg_end);
	DBG_FLT("env_start = %x\n", (int)current->mm->env_start);
	DBG_FLT("env_end = %x\n", (int)current->mm->env_end);
	DBG_FLT("start_brk = %x\n", (int)current->mm->start_brk);
	DBG_FLT("end_brk = %x\n", (int)current->mm->end_brk);
	DBG_FLT("start_code = %x\n", (int)current->mm->start_code);
	DBG_FLT("end_code = %x\n", (int)current->mm->end_code);
	DBG_FLT("start_data = %x\n", (int)current->mm->start_data);
	DBG_FLT("end_data = %x\n", (int)current->mm->end_data);

	current->mm->start_stack = p;

#ifdef DEBUG	
	show_free_areas();
#endif

	flush_cache_mm(current->mm);

	DBG_FLT("start_thread(regs=0x%x, entry=0x%x, start_data=0x%x, "
		"start_stack=0x%x)\n", regs, textpos + ntohl(hdr->entry),
		current->mm->start_data, p);

#ifdef __i960__
    /* It seems that in the 2.0.x kernel, the m68knommu architecture passes the start_data argument,
       but as this is not consistent with what all the other architectures with a mmu are doing, and
       that this argument isn't there anymnore in the 2.4.x kernel, I'll safely leave things as is for
       the i960. */
    start_thread(regs, textpos + ntohl(hdr->entry), p);
#else
    start_thread(regs, textpos + ntohl(hdr->entry), current->mm->start_data, p);
#endif
	
	if (current->flags & PF_PTRACED)
		send_sig(SIGTRAP, current, 0);
	return 0;
}

static int
load_flat_binary(struct linux_binprm * bprm, struct pt_regs * regs)
{
	int retval;

	MOD_INC_USE_COUNT;
	retval = do_load_flat_binary(bprm, regs);
	MOD_DEC_USE_COUNT;
	return retval;
}

int init_flat_binfmt(void) {
	return register_binfmt(&flat_format);
}

#ifdef MODULE
int init_module(void) {
	return init_flat_binfmt();
}

void cleanup_module( void) {
	unregister_binfmt(&flat_format);
}
#endif


