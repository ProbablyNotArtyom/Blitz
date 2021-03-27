/*
 *	Local dummy definitions of kernel structs. The inftl and inftl
 *	headers are not app clean...
 */
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

#if defined(__i386__) || defined(__sh__) || defined(__sparc__) || defined(__ppc__)
struct semaphore {
	int	dummy;
};
#endif

typedef struct wait_queue_head {
	int	dummy;
} wait_queue_head_t;

struct list_head {
	int	dummy;
};

#endif
