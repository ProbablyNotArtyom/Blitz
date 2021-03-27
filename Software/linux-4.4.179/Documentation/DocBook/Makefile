/proc/sys/net/ipv4/* Variables:

ip_forward - BOOLEAN
	0 - disabled (default)
	not 0 - enabled

	Forward Packets between interfaces.

	This variable is special, its change resets all configuration
	parameters to their default state (RFC1122 for hosts, RFC1812
	for routers)

ip_default_ttl - INTEGER
	Default value of TTL field (Time To Live) for outgoing (but not
	forwarded) IP packets. Should be between 1 and 255 inclusive.
	Default: 64 (as recommended by RFC1700)

ip_no_pmtu_disc - INTEGER
	Disable Path MTU Discovery. If enabled in mode 1 and a
	fragmentation-required ICMP is received, the PMTU to this
	destination will be set to min_pmtu (see below). You will need
	to raise min_pmtu to the smallest interface MTU on your system
	manually if you want to avoid locally generated fragments.

	In mode 2 incoming Path MTU Discovery messages will be
	discarded. Outgoing frames are handled the same as in mode 1,
	implicitly setting IP_PMTUDISC_DONT on every created socket.

	Mode 3 is a hardened pmtu discover mode. The kernel will only
	accept fragmentation-needed errors if the underlying protocol
	can verify them besides a plain socket lookup. Current
	protocols for which pmtu events will be honored are TCP, SCTP
	and DCCP as they verify e.g. the sequence number or the
	association. This mode should not be enabled globally but is
	only intended to secure e.g. name servers in namespaces where
	TCP path mtu must still work but path MTU information of other
	protocols should be discarded. If enabled globally this mode
	could break other protocols.

	Possible values: 0-3
	Default: FALSE

min_pmtu - INTEGER
	default 552 - minimum discovered Path MTU

ip_forward_use_pmtu - BOOLEAN
	By default we don't trust protocol path MTUs while forwarding
	because they could be easily forged and can lead to unwanted
	fragmentation by the router.
	You only need to enable this if you have user-space software
	which tries to discover path mtus by itself and depends on the
	kernel honoring this information. This is normally not the
	case.
	Default: 0 (disabled)
	Possible values:
	0 - disabled
	1 - enabled

fwmark_reflect - BOOLEAN
	Controls the fwmark of kernel-generated IPv4 reply packets that are not
	associated with a socket for example, TCP RSTs or ICMP echo replies).
	If unset, these packets have a fwmark of zero. If set, they have the
	fwmark of the packet they are replying to.
	Default: 0

fib_multipath_use_neigh - BOOLEAN
	Use status of existing neighbor entry when determining nexthop for
	multipath routes. If disabled, neighbor information is not used and
	packets could be directed to a failed nexthop. Only valid for kernels
	built with CONFIG_IP_ROUTE_MULTIPATH enabled.
	Default: 0 (disabled)
	Possible values:
	0 - disabled
	1 - enabled

fib_multipath_hash_policy - INTEGER
	Controls which hash policy to use for multipath routes. Only valid
	for kernels built with CONFIG_IP_ROUTE_MULTIPATH enabled.
	Default: 0 (Layer 3)
	Possible values:
	0 - Layer 3
	1 - Layer 4

ip_forward_update_priority - INTEGER
	Whether to update SKB priority from "TOS" field in IPv4 header after it
	is forwarded. The new SKB priority is mapped from TOS field value
	according to an rt_tos2priority table (see e.g. man tc-prio).
	Default: 1 (Update priority.)
	Possible values:
	0 - Do not update priority.
	1 - Update priority.

route/max_size - INTEGER
	Maximum number of routes allowed in the kernel.  Increase
	this when using large numbers of interfaces and/or routes.
	From linux kernel 3.6 onwards, this is deprecated for ipv4
	as route cache is no longer used.

neigh/default/gc_thresh1 - INTEGER
	Minimum number of entries to keep.  Garbage collector will not
	purge entries if there are fewer than this number.
	Default: 128

neigh/default/gc_thresh2 - INTEGER
	Threshold when garbage collector becomes more aggressive about
	purging entries. Entries older than 5 seconds will be cleared
	when over this number.
	Default: 512

neigh/default/gc_thresh3 - INTEGER
	Maximum number of neighbor entries allowed.  Increase this
	when using large numbers of interfaces and when communicating
	with large numbers of directly-connected peers.
	Default: 1024

neigh/default/unres_qlen_bytes - INTEGER
	The maximum number of bytes which may be used by packets
	queued for each	unresolved address by other network layers.
	(added in linux 3.3)
	Setting negative value is meaningless and will return error.
	Default: SK_WMEM_MAX, (same as net.core.wmem_default).
		Exact value depends on architecture and kernel options,
		but should be enough to allow queuing 256 packets
		of medium size.

neigh/default/unres_qlen - INTEGER
	The maximum number of packets which may be queued for each
	unresolved address by other network layers.
	(deprecated in linux 3.3) : use unres_qlen_bytes instead.
	Prior to linux 3.3, the default value is 3 which may cause
	unexpected packet loss. The current default value is calculated
	according to default value of unres_qlen_bytes and true size of
	packet.
	Default: 101

mtu_expires - INTEGER
	Time, in seconds, that cached PMTU information is kept.

min_adv_mss - INTEGER
	The advertised MSS depends on the first hop route MTU, but will
	never be lower than this setting.

IP Fragmentation:

ipfrag_high_thresh - LONG INTEGER
	Maximum memory used to reassemble IP fragments.

ipfrag_low_thresh - LONG INTEGER
	(Obsolete since linux-4.17)
	Maximum memory used to reassemble IP fragments before the kernel
	begins to remove incomplete fragment queues to free up resources.
	The kernel still accepts new fragments for defragmentation.

ipfrag_time - INTEGER
	Time in seconds to keep an IP fragment in memory.

ipfrag_max_dist - INTEGER
	ipfrag_max_dist is a non-negative integer value which defines the
	maximum "disorder" which is allowed among fragments which share a
	common IP source address. Note that reordering of packets is
	not unusual, but if a large number of fragments arrive from a source
	IP address while a particular fragment queue remains incomplete, it
	probably indicates that one or more fragments belonging to that queue
	have been lost. When ipfrag_max_dist is positive, an additional check
	is done on fragments before they are added to a reassembly queue - if
	ipfrag_max_dist (or more) fragments have arrived from a particular IP
	address between additions to any IP fragment queue using that source
	address, it's presumed that one or more fragments in the queue are
	lost. The existing fragment queue will be dropped, and a new one
	started. An ipfrag_max_dist value of zero disables this check.

	Using a very small value, e.g. 1 or 2, for ipfrag_max_dist can
	result in unnecessarily dropping fragment queues when normal
	reordering of packets occurs, which could lead to poor application
	performance. Using a very large value, e.g. 50000, increases the
	likelihood of incorrectly reassembling IP fragments that originate
	from different IP datagrams, which could result in data corruption.
	Default: 64

INET peer storage:

inet_peer_threshold - INTEGER
	The approximate size of the storage.  Starting from this threshold
	entries will be thrown aggressively.  This threshold also determines
	entries' time-to-live and time intervals between garbage collection
	passes.  More entries, less time-to-live, less GC interval.

inet_peer_minttl - INTEGER
	Minimum time-to-live of entries.  Should be enough to cover fragment
	time-to-live on the reassembling side.  This minimum time-to-live  is
	guaranteed if the pool size is less than inet_peer_threshold.
	Measured in seconds.

inet_peer_maxttl - INTEGER
	Maximum time-to-live of entries.  Unused entries will expire after
	this period of time if there is no memory pressure on the pool (i.e.
	when the number of entries in the pool is very small).
	Measured in seconds.

TCP variables:

somaxconn - INTEGER
	Limit of socket listen() backlog, known in userspace as SOMAXCONN.
	Defaults to 128.  See also tcp_max_syn_backlog for additional tuning
	for TCP sockets.

tcp_abort_on_overflow - BOOLEAN
	If listening service is too slow to accept new connections,
	reset them. Default state is FALSE. It means that if overflow
	occurred due to a burst, connection will recover. Enable this
	option _only_ if you are really sure th