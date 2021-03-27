( /^#define BB_/ && ! /^#define BB_FEATURE/ ) ||
( /^\/\/#define BB_/ && ! /^\/\/#define BB_FEATURE/ ) {
	envname = "CONFIG_USER_BUSYBOX_" substr($2, 4)
	envval = ENVIRON[envname]
	$1 = (envval == "y") ? "#define" : "//#define"
	print $0
}
/^#define BB_FEATURE/ ||
/^\/\/#define BB_FEATURE/ {
	envname = "CONFIG_USER_BUSYBOX_" substr($2, 12)
	envval = ENVIRON[envname]
	$1 = (envval == "y") ? "#define" : "//#define"
	print $0
}
! ( /^#define BB_/ || /^\/\/#define BB_/ ) {
	print
}
