/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: msg-destroy.c,v $
 * Revision 1.5  1997/06/21 01:40:07  drepper
 * More 64bit changes.
 *
 * Revision 1.4  1996/11/15 19:44:43  thomas
 * Tue Nov 12 16:58:41 1996  Thomas Bushnell, n/BSG  <thomas@gnu.ai.mit.edu>
 *
 * 	* mach/mach.h (__mach_msg_destroy, mach_msg_destroy, __mach_msg):
 *  	Provide prototypes.
 *
 * 	* mach/msg-destroy.c (mach_msg_destroy_port,
 * 	mach_msg_destroy_memory): Use prototype	syntax.
 * 	* hurd/hurdmalloc.c (more_memory, malloc_fork_prepare,
 * 	malloc_fork_parent, malloc_fork_child): Likewise.
 *
 * Revision 1.3  1995/01/23 22:16:52  roland
 * (mach_msg_destroy): Define as weak alias for __mach_msg_destroy.
 *
 * Revision 1.2  1993/08/03  06:13:18  roland
 * entered into RCS
 *
 * Revision 2.4  91/05/14  17:53:15  mrt
 * 	Correcting copyright
 *
 * Revision 2.3  91/02/14  14:17:43  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:15  mrt]
 *
 * Revision 2.2  90/08/06  17:24:22  rpd
 * 	Created.
 *
 */

#if 1
#include <mach.h>
#else
/* This is what CMU did, but that fails to declare some used functions.  */
#include <mach/port.h>
#include <mach/message.h>
#include <mach_init.h>
#endif

static void mach_msg_destroy_port(mach_port_t, mach_msg_type_name_t);
static void mach_msg_destroy_memory(vm_offset_t, vm_size_t);

/*
 *	Routine:	mach_msg_destroy
 *	Purpose:
 *		Deallocates all port rights and out-of-line memory
 *		found in a received message.
 */

void
__mach_msg_destroy(msg)
    mach_msg_header_t *msg;
{
    mach_msg_bits_t mbits = msg->msgh_bits;

    /*
     *	The msgh_local_port field doesn't hold a port right.
     *	The receive operation consumes the destination port right.
     */

    mach_msg_destroy_port(msg->msgh_remote_port, MACH_MSGH_BITS_REMOTE(mbits));

    if (mbits & MACH_MSGH_BITS_COMPLEX) {
	vm_offset_t saddr;
	vm_offset_t eaddr;

	saddr = (vm_offset_t) (msg + 1);
	eaddr = (vm_offset_t) msg + msg->msgh_size;

	while (saddr < eaddr) {
	    mach_msg_type_long_t *type;
	    mach_msg_type_name_t name;
	    mach_msg_type_size_t size;
	    mach_msg_type_number_t number;
	    boolean_t is_inline;
	    vm_size_t length;
	    vm_offset_t addr;

	    type = (mach_msg_type_long_t *) saddr;
	    is_inline = type->msgtl_header.msgt_inline;
	    if (type->msgtl_header.msgt_longform) {
		    name = type->msgtl_name;
		    size = type->msgtl_size;
		    number = type->msgtl_number;
		    saddr += sizeof(mach_msg_type_long_t);
	    } else {
		    name = type->msgtl_header.msgt_name;
		    size = type->msgtl_header.msgt_size;
		    number = type->msgtl_header.msgt_number;
		    saddr += sizeof(mach_msg_type_t);
	    }

	    /* calculate length of data in bytes, rounding up */
	    length = (((((number * size) + 7) >> 3) + sizeof (int) - 1)
		      &~ (sizeof (int) - 1));

	    addr = is_inline ? saddr : * (vm_offset_t *) saddr;

	    if (MACH_MSG_TYPE_PORT_ANY(name)) {
		mach_port_t *ports = (mach_port_t *) addr;
		mach_msg_type_number_t i;

		for (i = 0; i < number; i++)
		    mach_msg_destroy_port(*ports++, name);
	    }

	    if (is_inline) {
		/* inline data sizes round up to int boundaries */
		saddr += length;
	    } else {
		mach_msg_destroy_memory(addr, length);
		saddr += sizeof(vm_offset_t);
	    }
	}
    }
}

weak_alias (__mach_msg_destroy, mach_msg_destroy)

static void
mach_msg_destroy_port(port, type)
    mach_port_t port;
    mach_msg_type_name_t type;
{
    if (MACH_PORT_VALID(port)) switch (type) {
      case MACH_MSG_TYPE_PORT_SEND:
      case MACH_MSG_TYPE_PORT_SEND_ONCE:
	(void) __mach_port_deallocate(__mach_task_self(), port);
	break;

      case MACH_MSG_TYPE_PORT_RECEIVE:
	(void) __mach_port_mod_refs(__mach_task_self(), port,
				    MACH_PORT_RIGHT_RECEIVE, -1);
	break;
    }
}

static void
mach_msg_destroy_memory(addr, size)
    vm_offset_t addr;
    vm_size_t size;
{
    if (size > 0)
	(void) __vm_deallocate(__mach_task_self(), addr, size);
}
