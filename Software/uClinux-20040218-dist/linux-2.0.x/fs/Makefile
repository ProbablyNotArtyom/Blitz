#
# Makefile for the linux filesystem.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

L_TARGET := filesystems.a
L_OBJS    = $(join $(SUB_DIRS),$(SUB_DIRS:%=/%.o))
O_TARGET := fs.o
O_OBJS    = open.o read_write.o inode.o devices.o file_table.o buffer.o \
		super.o  block_dev.o stat.o exec.o pipe.o namei.o fcntl.o \
		ioctl.o readdir.o select.o fifo.o locks.o filesystems.o \
		dcache.o bad_inode.o $(BINFMTS)
OX_OBJS   = $(NLS)

MOD_LIST_NAME := FS_MODULES
ALL_SUB_DIRS = minix ext ext2 fat msdos vfat proc isofs nfs xiafs umsdos \
		hpfs sysv smbfs ncpfs ufs affs autofs romfs jffs

ifeq ($(CONFIG_QUOTA),y)
O_OBJS += dquot.o
else
O_OBJS += noquot.o
endif

ifeq ($(CONFIG_MINIX_FS),y)
SUB_DIRS += minix
else
  ifeq ($(CONFIG_MINIX_FS),m)
  MOD_SUB_DIRS += minix
  endif
endif

ifeq ($(CONFIG_EXT_FS),y)
SUB_DIRS += ext
else
  ifeq ($(CONFIG_EXT_FS),m)
  MOD_SUB_DIRS += ext
  endif
endif

ifeq ($(CONFIG_EXT2_FS),y)
SUB_DIRS += ext2
else
  ifeq ($(CONFIG_EXT2_FS),m)
  MOD_SUB_DIRS += ext2
  endif
endif

ifeq ($(CONFIG_FAT_FS),y)
SUB_DIRS += fat
else
  ifeq ($(CONFIG_FAT_FS),m)
  MOD_SUB_DIRS += fat
  endif
endif

ifeq ($(CONFIG_MSDOS_FS),y)
SUB_DIRS += msdos
else
  ifeq ($(CONFIG_MSDOS_FS),m)
  MOD_SUB_DIRS += msdos
  endif
endif

ifeq ($(CONFIG_VFAT_FS),y)
SUB_DIRS += vfat
else
  ifeq ($(CONFIG_VFAT_FS),m)
  MOD_SUB_DIRS += vfat
  endif
endif

ifdef CONFIG_PROC_FS
SUB_DIRS += proc
endif

ifeq ($(CONFIG_ISO9660_FS),y)
SUB_DIRS += isofs
else
  ifeq ($(CONFIG_ISO9660_FS),m)
  MOD_SUB_DIRS += isofs
  endif
endif

ifeq ($(CONFIG_NFS_FS),y)
SUB_DIRS += nfs
else
  ifeq ($(CONFIG_NFS_FS),m)
  MOD_SUB_DIRS += nfs
  endif
endif

ifeq ($(CONFIG_XIA_FS),y)
SUB_DIRS += xiafs
else
  ifeq ($(CONFIG_XIA_FS),m)
  MOD_SUB_DIRS += xiafs
  endif
endif

ifeq ($(CONFIG_UMSDOS_FS),y)
SUB_DIRS += umsdos
else
  ifeq ($(CONFIG_UMSDOS_FS),m)
  MOD_SUB_DIRS += umsdos
  endif
endif

ifeq ($(CONFIG_SYSV_FS),y)
SUB_DIRS += sysv
else
  ifeq ($(CONFIG_SYSV_FS),m)
  MOD_SUB_DIRS += sysv
  endif
endif

ifeq ($(CONFIG_SMB_FS),y)
SUB_DIRS += smbfs
else
  ifeq ($(CONFIG_SMB_FS),m)
  MOD_SUB_DIRS += smbfs
  endif
endif

ifeq ($(CONFIG_NCP_FS),y)
SUB_DIRS += ncpfs
else
  ifeq ($(CONFIG_NCP_FS),m)
  MOD_SUB_DIRS += ncpfs
  endif
endif

ifeq ($(CONFIG_HPFS_FS),y)
SUB_DIRS += hpfs
else
  ifeq ($(CONFIG_HPFS_FS),m)
  MOD_SUB_DIRS += hpfs
  endif
endif

ifeq ($(CONFIG_UFS_FS),y)
SUB_DIRS += ufs
else
  ifeq ($(CONFIG_UFS_FS),m)
  MOD_SUB_DIRS += ufs
  endif
endif

ifeq ($(CONFIG_AFFS_FS),y)
SUB_DIRS += affs
else
  ifeq ($(CONFIG_AFFS_FS),m)
  MOD_SUB_DIRS += affs
  endif
endif

ifeq ($(CONFIG_AUTOFS_FS),y)
SUB_DIRS += autofs
else
  ifeq ($(CONFIG_AUTOFS_FS),m)
  MOD_SUB_DIRS += autofs
  endif
endif

ifeq ($(CONFIG_ROMFS_FS),y)
SUB_DIRS += romfs
else
  ifeq ($(CONFIG_ROMFS_FS),m)
  MOD_SUB_DIRS += romfs
  endif
endif

ifeq ($(CONFIG_JFFS_FS),y)
SUB_DIRS += jffs
else
  ifeq ($(CONFIG_JFFS_FS),m)
  MOD_SUB_DIRS += jffs
  endif
endif

ifeq ($(CONFIG_BINFMT_FLAT),y)
BINFMTS += binfmt_flat.o
else
  ifeq ($(CONFIG_BINFMT_FLAT),m)
  M_OBJS += binfmt_flat.o
  endif
endif

ifeq ($(CONFIG_BINFMT_ELF),y)
BINFMTS += binfmt_elf.o
else
  ifeq ($(CONFIG_BINFMT_ELF),m)
  M_OBJS += binfmt_elf.o
  endif
endif

ifeq ($(CONFIG_BINFMT_AOUT),y)
BINFMTS += binfmt_aout.o
else
  ifeq ($(CONFIG_BINFMT_AOUT),m)
  M_OBJS += binfmt_aout.o
  endif
endif

ifeq ($(CONFIG_BINFMT_JAVA),y)
BINFMTS += binfmt_java.o
else
  ifeq ($(CONFIG_BINFMT_JAVA),m)
  M_OBJS += binfmt_java.o
  endif
endif

ifeq ($(CONFIG_BINFMT_EM86),y)
BINFMTS += binfmt_em86.o
else
  ifeq ($(CONFIG_BINFMT_EM86),m)
  M_OBJS += binfmt_em86.o
  endif
endif

# binfmt_script is always there
BINFMTS += binfmt_script.o

ifeq ($(CONFIG_NLS),y)
NLS += nls.o
else
  ifeq ($(CONFIG_NLS),m)
  MX_OBJS += nls.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_437),y)
NLS += nls_cp437.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_437),m)
  M_OBJS += nls_cp437.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_737),y)
NLS += nls_cp737.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_737),m)
  M_OBJS += nls_cp737.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_775),y)
NLS += nls_cp775.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_775),m)
  M_OBJS += nls_cp775.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_850),y)
NLS += nls_cp850.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_850),m)
  M_OBJS += nls_cp850.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_852),y)
NLS += nls_cp852.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_852),m)
  M_OBJS += nls_cp852.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_855),y)
NLS += nls_cp855.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_855),m)
  M_OBJS += nls_cp855.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_857),y)
NLS += nls_cp857.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_857),m)
  M_OBJS += nls_cp857.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_860),y)
NLS += nls_cp860.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_860),m)
  M_OBJS += nls_cp860.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_861),y)
NLS += nls_cp861.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_861),m)
  M_OBJS += nls_cp861.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_862),y)
NLS += nls_cp862.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_862),m)
  M_OBJS += nls_cp862.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_863),y)
NLS += nls_cp863.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_863),m)
  M_OBJS += nls_cp863.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_864),y)
NLS += nls_cp864.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_864),m)
  M_OBJS += nls_cp864.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_865),y)
NLS += nls_cp865.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_865),m)
  M_OBJS += nls_cp865.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_866),y)
NLS += nls_cp866.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_866),m)
  M_OBJS += nls_cp866.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_869),y)
NLS += nls_cp869.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_869),m)
  M_OBJS += nls_cp869.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_874),y)
NLS += nls_cp874.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_874),m)
  M_OBJS += nls_cp874.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1250),y)
NLS += nls_cp1250.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1250),m)
  M_OBJS += nls_cp1250.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1251),y)
NLS += nls_cp1251.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1251),m)
  M_OBJS += nls_cp1251.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1252),y)
NLS += nls_cp1252.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1252),m)
  M_OBJS += nls_cp1252.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1253),y)
NLS += nls_cp1253.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1253),m)
  M_OBJS += nls_cp1253.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1254),y)
NLS += nls_cp1254.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1254),m)
  M_OBJS += nls_cp1254.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1255),y)
NLS += nls_cp1255.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1255),m)
  M_OBJS += nls_cp1255.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1256),y)
NLS += nls_cp1256.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1256),m)
  M_OBJS += nls_cp1256.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1257),y)
NLS += nls_cp1257.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1257),m)
  M_OBJS += nls_cp1257.o
  endif
endif

ifeq ($(CONFIG_NLS_CODEPAGE_1258),y)
NLS += nls_cp1258.o
else
  ifeq ($(CONFIG_NLS_CODEPAGE_1258),m)
  M_OBJS += nls_cp1258.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_1),y)
NLS += nls_iso8859_1.o
else
  ifeq ($(CONFIG_NLS_ISO8859_1),m)
  M_OBJS += nls_iso8859_1.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_2),y)
NLS += nls_iso8859_2.o
else
  ifeq ($(CONFIG_NLS_ISO8859_2),m)
  M_OBJS += nls_iso8859_2.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_3),y)
NLS += nls_iso8859_3.o
else
  ifeq ($(CONFIG_NLS_ISO8859_3),m)
  M_OBJS += nls_iso8859_3.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_4),y)
NLS += nls_iso8859_4.o
else
  ifeq ($(CONFIG_NLS_ISO8859_4),m)
  M_OBJS += nls_iso8859_4.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_5),y)
NLS += nls_iso8859_5.o
else
  ifeq ($(CONFIG_NLS_ISO8859_5),m)
  M_OBJS += nls_iso8859_5.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_6),y)
NLS += nls_iso8859_6.o
else
  ifeq ($(CONFIG_NLS_ISO8859_6),m)
  M_OBJS += nls_iso8859_6.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_7),y)
NLS += nls_iso8859_7.o
else
  ifeq ($(CONFIG_NLS_ISO8859_7),m)
  M_OBJS += nls_iso8859_7.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_8),y)
NLS += nls_iso8859_8.o
else
  ifeq ($(CONFIG_NLS_ISO8859_8),m)
  M_OBJS += nls_iso8859_8.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_9),y)
NLS += nls_iso8859_9.o
else
  ifeq ($(CONFIG_NLS_ISO8859_9),m)
  M_OBJS += nls_iso8859_9.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_10),y)
NLS += nls_iso8859_10.o
else
  ifeq ($(CONFIG_NLS_ISO8859_10),m)
  M_OBJS += nls_iso8859_10.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_14),y)
NLS += nls_iso8859_14.o
else
  ifeq ($(CONFIG_NLS_ISO8859_14),m)
  M_OBJS += nls_iso8859_14.o
  endif
endif

ifeq ($(CONFIG_NLS_ISO8859_15),y)
NLS += nls_iso8859_15.o
else
  ifeq ($(CONFIG_NLS_ISO8859_15),m)
  M_OBJS += nls_iso8859_15.o
  endif
endif

ifeq ($(CONFIG_NLS_KOI8_R),y)
NLS += nls_koi8_r.o
else
  ifeq ($(CONFIG_NLS_KOI8_R),m)
  M_OBJS += nls_koi8_r.o
  endif
endif

include $(TOPDIR)/Rules.make
