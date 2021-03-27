/* vi: set sw=4 ts=4: */
/*
 * This file was originally generated using rpcgen.
 * But now we edit it by hand as needed to make it
 * shut up...
 */

#ifndef _NFSMOUNT_H_RPCGEN
#define _NFSMOUNT_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user or with the express written consent of
 * Sun Microsystems, Inc.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/*
 * Copyright (c) 1985, 1990 by Sun Microsystems, Inc.
 */

/* from @(#)mount.x	1.3 91/03/11 TIRPC 1.0 */
#ifndef _rpcsvc_mount_h
#define _rpcsvc_mount_h
#include <asm/types.h>
#define MOUNTPORT 635
#define MNTPATHLEN 1024
#define MNTNAMLEN 255
#define FHSIZE 32
#define FHSIZE3 64

typedef char fhandle[FHSIZE];

typedef struct {
	u_int fhandle3_len;
	char *fhandle3_val;
} fhandle3;

enum mountstat3 {
	MNT_OK = 0,
	MNT3ERR_PERM = 1,
	MNT3ERR_NOENT = 2,
	MNT3ERR_IO = 5,
	MNT3ERR_ACCES = 13,
	MNT3ERR_NOTDIR = 20,
	MNT3ERR_INVAL = 22,
	MNT3ERR_NAMETOOLONG = 63,
	MNT3ERR_NOTSUPP = 10004,
	MNT3ERR_SERVERFAULT = 10006,
};
typedef enum mountstat3 mountstat3;

struct fhstatus {
	u_int fhs_status;
	union {
		fhandle fhs_fhandle;
	} fhstatus_u;
};
typedef struct fhstatus fhstatus;

struct mountres3_ok {
	fhandle3 fhandle;
	struct {
		u_int auth_flavours_len;
		int *auth_flavours_val;
	} auth_flavours;
};
typedef struct mountres3_ok mountres3_ok;

struct mountres3 {
	mountstat3 fhs_status;
	union {
		mountres3_ok mountinfo;
	} mountres3_u;
};
typedef struct mountres3 mountres3;

typedef char *dirpath;

typedef char *name;

typedef struct mountbody *mountlist;

struct mountbody {
	name ml_hostname;
	dirpath ml_directory;
	mountlist ml_next;
};
typedef struct mountbody mountbody;

typedef struct groupnode *groups;

struct groupnode {
	name gr_name;
	groups gr_next;
};
typedef struct groupnode groupnode;

typedef struct exportnode *exports;

struct exportnode {
	dirpath ex_dir;
	groups ex_groups;
	exports ex_next;
};
typedef struct exportnode exportnode;

struct ppathcnf {
	int pc_link_max;
	short pc_max_canon;
	short pc_max_input;
	short pc_name_max;
	short pc_path_max;
	short pc_pipe_buf;
	u_char pc_vdisable;
	char pc_xxx;
	short pc_mask[2];
};
typedef struct ppathcnf ppathcnf;
#endif /*!_rpcsvc_mount_h*/

#define MOUNTPROG 100005
#define MOUNTVERS 1

#define MOUNTPROC_NULL 0
extern  void * mountproc_null_1(void *, CLIENT *);
extern  void * mountproc_null_1_svc(void *, struct svc_req *);
#define MOUNTPROC_MNT 1
extern  fhstatus * mountproc_mnt_1(dirpath *, CLIENT *);
extern  fhstatus * mountproc_mnt_1_svc(dirpath *, struct svc_req *);
#define MOUNTPROC_DUMP 2
extern  mountlist * mountproc_dump_1(void *, CLIENT *);
extern  mountlist * mountproc_dump_1_svc(void *, struct svc_req *);
#define MOUNTPROC_UMNT 3
extern  void * mountproc_umnt_1(dirpath *, CLIENT *);
extern  void * mountproc_umnt_1_svc(dirpath *, struct svc_req *);
#define MOUNTPROC_UMNTALL 4
extern  void * mountproc_umntall_1(void *, CLIENT *);
extern  void * mountproc_umntall_1_svc(void *, struct svc_req *);
#define MOUNTPROC_EXPORT 5
extern  exports * mountproc_export_1(void *, CLIENT *);
extern  exports * mountproc_export_1_svc(void *, struct svc_req *);
#define MOUNTPROC_EXPORTALL 6
extern  exports * mountproc_exportall_1(void *, CLIENT *);
extern  exports * mountproc_exportall_1_svc(void *, struct svc_req *);
extern int mountprog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#define MOUNTVERS_POSIX 2

extern  void * mountproc_null_2(void *, CLIENT *);
extern  void * mountproc_null_2_svc(void *, struct svc_req *);
extern  fhstatus * mountproc_mnt_2(dirpath *, CLIENT *);
extern  fhstatus * mountproc_mnt_2_svc(dirpath *, struct svc_req *);
extern  mountlist * mountproc_dump_2(void *, CLIENT *);
extern  mountlist * mountproc_dump_2_svc(void *, struct svc_req *);
extern  void * mountproc_umnt_2(dirpath *, CLIENT *);
extern  void * mountproc_umnt_2_svc(dirpath *, struct svc_req *);
extern  void * mountproc_umntall_2(void *, CLIENT *);
extern  void * mountproc_umntall_2_svc(void *, struct svc_req *);
extern  exports * mountproc_export_2(void *, CLIENT *);
extern  exports * mountproc_export_2_svc(void *, struct svc_req *);
extern  exports * mountproc_exportall_2(void *, CLIENT *);
extern  exports * mountproc_exportall_2_svc(void *, struct svc_req *);
#define MOUNTPROC_PATHCONF 7
extern  ppathcnf * mountproc_pathconf_2(dirpath *, CLIENT *);
extern  ppathcnf * mountproc_pathconf_2_svc(dirpath *, struct svc_req *);
extern int mountprog_2_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#define MOUNT_V3 3

#define MOUNTPROC3_NULL 0
extern  void * mountproc3_null_3(void *, CLIENT *);
extern  void * mountproc3_null_3_svc(void *, struct svc_req *);
#define MOUNTPROC3_MNT 1
extern  mountres3 * mountproc3_mnt_3(dirpath *, CLIENT *);
extern  mountres3 * mountproc3_mnt_3_svc(dirpath *, struct svc_req *);
#define MOUNTPROC3_DUMP 2
extern  mountlist * mountproc3_dump_3(void *, CLIENT *);
extern  mountlist * mountproc3_dump_3_svc(void *, struct svc_req *);
#define MOUNTPROC3_UMNT 3
extern  void * mountproc3_umnt_3(dirpath *, CLIENT *);
extern  void * mountproc3_umnt_3_svc(dirpath *, struct svc_req *);
#define MOUNTPROC3_UMNTALL 4
extern  void * mountproc3_umntall_3(void *, CLIENT *);
extern  void * mountproc3_umntall_3_svc(void *, struct svc_req *);
#define MOUNTPROC3_EXPORT 5
extern  exports * mountproc3_export_3(void *, CLIENT *);
extern  exports * mountproc3_export_3_svc(void *, struct svc_req *);
extern int mountprog_3_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

/* the xdr functions */

static  bool_t xdr_fhandle (XDR *, fhandle);
extern  bool_t xdr_fhandle3 (XDR *, fhandle3*);
extern  bool_t xdr_mountstat3 (XDR *, mountstat3*);
extern  bool_t xdr_fhstatus (XDR *, fhstatus*);
extern  bool_t xdr_mountres3_ok (XDR *, mountres3_ok*);
extern  bool_t xdr_mountres3 (XDR *, mountres3*);
extern  bool_t xdr_dirpath (XDR *, dirpath*);
extern  bool_t xdr_name (XDR *, name*);
extern  bool_t xdr_mountlist (XDR *, mountlist*);
extern  bool_t xdr_mountbody (XDR *, mountbody*);
extern  bool_t xdr_groups (XDR *, groups*);
extern  bool_t xdr_groupnode (XDR *, groupnode*);
extern  bool_t xdr_exports (XDR *, exports*);
extern  bool_t xdr_exportnode (XDR *, exportnode*);
extern  bool_t xdr_ppathcnf (XDR *, ppathcnf*);

#ifdef __cplusplus
}
#endif

#endif /* !_NFSMOUNT_H_RPCGEN */
