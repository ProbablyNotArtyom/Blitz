This document describes the interfaces /proc/net/tcp and /proc/net/tcp6.
Note that these interfaces are deprecated in favor of tcp_diag.

These /proc interfaces provide information about currently active TCP 
connections, and are implemented by tcp4_seq_show() in net/ipv4/tcp_ipv4.c
and tcp6_seq_show() in net/ipv6/tcp_ipv6.c, respectively.

It will first list all listening TCP sockets, and next list all established
TCP connections. A typical entry of /proc/net/tcp would look like this (sp