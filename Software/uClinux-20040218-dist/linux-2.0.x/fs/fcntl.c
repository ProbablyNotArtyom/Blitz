/*
 *  linux/fs/fcntl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/string.h>

#include <asm/bitops.h>

extern int sock_fcntl (struct file *, unsigned int cmd, unsigned long arg);

static inline int dupfd(unsigned int fd, unsigned int arg)
{
	struct files_struct * files = current->files;

	if (fd >= NR_OPEN || !files->fd[fd])
		return -EBADF;
	if (arg >= NR_OPEN)
		return -EINVAL;
	arg = find_next_zero_bit(&files->open_fds, NR_OPEN, arg);
	if (arg >= current->rlim[RLIMIT_NOFILE].rlim_cur)
		return -EMFILE;
	FD_SET(arg, &files->open_fds);
	FD_CLR(arg, &files->close_on_exec);
	(files->fd[arg] = files->fd[fd])->f_count++;
	return arg;
}

asmlinkage int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
	if (oldfd >= NR_OPEN || !current->files->fd[oldfd])
		return -EBADF;
	if (newfd == oldfd)
		return newfd;
	if (newfd >= NR_OPEN)
		return -EBADF;	/* following POSIX.1 6.2.1 */

	sys_close(newfd);
	return dupfd(oldfd,newfd);
}

asmlinkage int sys_dup(unsigned int fildes)
{
	return dupfd(fildes,0);
}

asmlinkage long sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{	
	struct file * filp;

	if (fd >= NR_OPEN || !(filp = current->files->fd[fd]))
		return -EBADF;
	switch (cmd) {
		case F_DUPFD:
			return dupfd(fd,arg);
		case F_GETFD:
			return FD_ISSET(fd, &current->files->close_on_exec);
		case F_SETFD:
			if (arg&1)
				FD_SET(fd, &current->files->close_on_exec);
			else
				FD_CLR(fd, &current->files->close_on_exec);
			return 0;
		case F_GETFL:
			return filp->f_flags;
		case F_SETFL:
			/*
			 * In the case of an append-only file, O_APPEND
			 * cannot be cleared
			 */
			if (IS_APPEND(filp->f_inode) && !(arg & O_APPEND))
				return -EPERM;
			if ((arg & FASYNC) && !(filp->f_flags & FASYNC) &&
			    filp->f_op->fasync)
				filp->f_op->fasync(filp->f_inode, filp, 1);
			if (!(arg & FASYNC) && (filp->f_flags & FASYNC) &&
			    filp->f_op->fasync)
				filp->f_op->fasync(filp->f_inode, filp, 0);
			/* required for SunOS emulation */
			if (O_NONBLOCK != O_NDELAY)
			       if (arg & O_NDELAY)
				   arg |= O_NONBLOCK;
			filp->f_flags &= ~(O_APPEND | O_NONBLOCK | FASYNC);
			filp->f_flags |= arg & (O_APPEND | O_NONBLOCK |
						FASYNC);
			return 0;
		case F_GETLK:
			return fcntl_getlk(fd, (struct flock *) arg);
		case F_SETLK:
			return fcntl_setlk(fd, cmd, (struct flock *) arg);
		case F_SETLKW:
			return fcntl_setlk(fd, cmd, (struct flock *) arg);
		case F_GETOWN:
			/*
			 * XXX If f_owner is a process group, the
			 * negative return value will get converted
			 * into an error.  Oops.  If we keep the
			 * current syscall conventions, the only way
			 * to fix this will be in libc.
			 */
			return filp->f_owner.pid;
		case F_SETOWN:
			filp->f_owner.pid = arg;
			filp->f_owner.uid = current->uid;
			filp->f_owner.euid = current->euid;
			if (S_ISSOCK (filp->f_inode->i_mode))
				sock_fcntl (filp, F_SETOWN, arg);
			return 0;
		default:
			/* sockets need a few special fcntls. */
			if (S_ISSOCK (filp->f_inode->i_mode))
			  {
			     return (sock_fcntl (filp, cmd, arg));
			  }
			return -EINVAL;
	}
}

static void send_sigio(int sig, int pid, uid_t uid, uid_t euid)
{
	struct task_struct * p;

	for_each_task(p) {
		int match = p->pid;
		if (pid < 0)
			match = -p->pgrp;
		if (pid != match)
			continue;
		if ((euid != 0) &&
		    (euid ^ p->suid) && (euid ^ p->uid) &&
		    (uid ^ p->suid) && (uid ^ p->uid))
			continue;
		p->signal |= 1 << (sig-1);
		if (p->state == TASK_INTERRUPTIBLE && (p->signal & ~p->blocked))
			wake_up_process(p);
	}
}

void kill_fasync(struct fasync_struct *fa, int sig)
{
	while (fa) {
		struct fown_struct * fown;
		if (fa->magic != FASYNC_MAGIC) {
			printk("kill_fasync: bad magic number in "
			       "fasync_struct!\n");
			return;
		}
		fown = &fa->fa_file->f_owner;
		if (fown->pid)
			send_sigio(sig, fown->pid, fown->uid, fown->euid);
		fa = fa->fa_next;
	}
}
