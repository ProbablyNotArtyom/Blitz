/*
 * $Header: /cvs/sw/new-wave/user/tcpdump/nfsfh.h,v 1.1 2000/08/03 05:59:19 gerg Exp $
 *
 * nfsfh.h - NFS file handle definitions (for portable use)
 *
 * Jeffrey C. Mogul
 * Digital Equipment Corporation
 * Western Research Laboratory
 */

/*
 * Internal representation of dev_t, because different NFS servers
 * that we might be spying upon use different external representations.
 */
typedef struct {
	u_int32_t Minor;	/* upper case to avoid clashing with macro names */
	u_int32_t Major;
} my_devt;

#define	dev_eq(a,b)	((a.Minor == b.Minor) && (a.Major == b.Major))

/*
 * Many file servers now use a large file system ID.  This is
 * our internal representation of that.
 */
typedef	struct {
	my_devt	Fsid_dev;		/* XXX avoid name conflict with AIX */
	u_int32_t fsid_code;
} my_fsid;

#define	fsid_eq(a,b)	((a.fsid_code == b.fsid_code) &&\
			 dev_eq(a.Fsid_dev, b.Fsid_dev))

extern void Parse_fh(caddr_t *, my_fsid *, ino_t *, char **, char **, int);
