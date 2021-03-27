/*
 *  linux/fs/binfmt_script.c
 *
 *  Copyright (C) 1996  Martin von Löwis
 *  original #!-checking implemented by tytso.
 *
 */
 
/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 *  FIXME: Modifications are untested!
 *	^^^^^: tested it, found a small bug, seems to work now (davidm@lineo.com)
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/malloc.h>
#include <linux/binfmts.h>

static int do_load_script(struct linux_binprm *bprm,struct pt_regs *regs)
{
	char *cp, *interp, *i_name, *i_arg;
	int retval;
	if ((bprm->buf[0] != '#') || (bprm->buf[1] != '!') || (bprm->sh_bang))
		return -ENOEXEC;
	/*
	 * This section does the #! interpretation.
	 * Sorta complicated, but hopefully it will work.  -TYT
	 */

	bprm->sh_bang++;
	iput(bprm->inode);
	bprm->dont_iput=1;

	bprm->buf[127] = '\0';
	if ((cp = strchr(bprm->buf, '\n')) == NULL)
		cp = bprm->buf+127;
	*cp = '\0';
	while (cp > bprm->buf) {
		cp--;
		if ((*cp == ' ') || (*cp == '\t'))
			*cp = '\0';
		else
			break;
	}
	for (cp = bprm->buf+2; (*cp == ' ') || (*cp == '\t'); cp++);
	if (!cp || *cp == '\0') 
		return -ENOEXEC; /* No interpreter name found */
	interp = i_name = cp;
	i_arg = 0;
	for ( ; *cp && (*cp != ' ') && (*cp != '\t'); cp++) {
		if (*cp == '/')
			i_name = cp+1;
	}
	while ((*cp == ' ') || (*cp == '\t'))
		*cp++ = '\0';
	if (*cp)
		i_arg = cp;
	/*
	 * OK, we've parsed out the interpreter name and
	 * (optional) argument.
	 * Splice in (1) the interpreter's name for argv[0]
	 *           (2) (optional) argument to interpreter
	 *           (3) filename of shell script (replace argv[0])
	 *
	 * This is done in reverse order, because of how the
	 * user environment and arguments are stored.
	 */
#ifndef NO_MM
	remove_arg_zero(bprm);
	bprm->p = copy_strings(1, &bprm->filename, bprm->page, bprm->p, 2);
	bprm->argc++;
	if (i_arg) {
		bprm->p = copy_strings(1, &i_arg, bprm->page, bprm->p, 2);
		bprm->argc++;
	}
	bprm->p = copy_strings(1, &i_name, bprm->page, bprm->p, 2);
	bprm->argc++;
	if ((long)bprm->p < 0)
		return (long)bprm->p;
	/*
	 * OK, now restart the process with the interpreter's inode.
	 * Note that we use open_namei() as the name is now in kernel
	 * space, and we don't need to copy it.
	 */
	retval = open_namei(interp, 0, 0, &bprm->inode, NULL);
	if (retval)
		return retval;
	bprm->dont_iput=0;
	retval=prepare_binprm(bprm);
	if(retval<0)
		return retval;
	return search_binary_handler(bprm,regs);
#else /* !NO_MM */
	{
		char ** oldargv = bprm->argv;
		int oldargc = bprm->argc;
		int i;
		
		bprm->argc = oldargc + (i_arg ? 2 : 1);
		bprm->argv = kmalloc(sizeof(char*) * bprm->argc, GFP_KERNEL);
		
		if (!bprm->argv) {
			bprm->argc = oldargc;
			bprm->argv = oldargv;
			return -E2BIG;
		}
		
		for(i=1;i<=oldargc;i++)
			bprm->argv[bprm->argc-i] = oldargv[oldargc-i];
		
		bprm->argv[0] = i_name;
		i=1;
		if (i_arg)
			bprm->argv[i++] = i_arg;
		bprm->argv[i++] = bprm->filename;
		
		
		/*
		 * OK, now restart the process with the interpreter's inode.
		 * Note that we use open_namei() as the name is now in kernel
		 * space, and we don't need to copy it.
		 */
		retval = open_namei(interp, 0, 0, &bprm->inode, NULL);
		if (!retval) {
			bprm->dont_iput=0;
			retval=prepare_binprm(bprm);
		}
		if (retval >= 0) {
			retval =  search_binary_handler(bprm,regs);
		}
		
		kfree(bprm->argv);
		bprm->argv = oldargv;
		bprm->argc = oldargc;

		return retval;
	}
#endif /* !NO_MM */
}

static int load_script(struct linux_binprm *bprm,struct pt_regs *regs)
{
	int retval;
	MOD_INC_USE_COUNT;
	retval = do_load_script(bprm,regs);
	MOD_DEC_USE_COUNT;
	return retval;
}

struct linux_binfmt script_format = {
#ifndef MODULE
	NULL, 0, load_script, NULL, NULL
#else
	NULL, &mod_use_count_, load_script, NULL, NULL
#endif
};

int init_script_binfmt(void) {
	return register_binfmt(&script_format);
}

#ifdef MODULE
int init_module(void)
{
	return init_script_binfmt();
}

void cleanup_module( void) {
	unregister_binfmt(&script_format);
}
#endif
