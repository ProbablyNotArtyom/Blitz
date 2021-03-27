#############################################################################

#
#	Intel IXP425 target gdbinit script.
#
#	Some very bogus support routines for use on the MaCraogir
#	wiggler pod on an Intel XSCALE/IXP425 processor board.
#

#############################################################################

#
#	UART functions
#

define uart-init
set *((long*) 0xc800000c) = 0x83
set *((long*) 0xc8000000) = 0x08
set *((long*) 0xc8000004) = 0x00
set *((long*) 0xc800000c) = 0x03
set *((long*) 0xc8000004) = 0x40
end

define uart-print
set *((long*) 0xc8000000) = $arg0
end


#
#	Memory functions
#

define mem-init
# set for 32MB (3CAS, 3RAS)
set *((long*) 0xcc000000) = 0x18

# disable refresh
set *((long*) 0xcc000004) = 0

# send NOP command
set *((long*) 0xcc000008) = 0x03
shell sleep 1

# set refresh count
set *((long*) 0xcc000004) = 0x384
shell sleep 1

# send PRECHARGE-ALL command
set *((long*) 0xcc000008) = 0x02
shell sleep 1

# send AUTO-REFRESH command
set $num = 8
while ($num > 0)
	set *((long*) 0xcc000008) = 0x04
	shell sleep 1
	set $num = $num - 1
end

# send MODE (CAS3) command
set *((long*) 0xcc000008) = 0x01
shell sleep 1

# send NORMAL-OPERATION command
set *((long*) 0xcc000008) = 0x06
shell sleep 1

end



#
#	Switch memory and expansion regions
#
define mem-switch
set *((unsigned long *) 0x10000000) = 0xe3a01331
set *((unsigned long *) 0x10000004) = 0xe3811020
set *((unsigned long *) 0x10000008) = 0xe5912000
set *((unsigned long *) 0x1000000c) = 0xe3c22102
set *((unsigned long *) 0x10000010) = 0xe5812000
set $pc = 0x10000000
stepi 5
end


define mem-fill
set $num = 0
set $addr = $arg0
set $val = $arg1
while ($num < 0x1000)
	set *((unsigned long *) $addr) = $val
	set $addr = $addr + 4
	set $num = $num + 1
end
end


define mem-check
set $num = 0
set $addr = $arg0
set $val = $arg1
while ($num < 0x1000)
	set $rd = *((unsigned long *) $addr)
	if ($rd != $val)
		print $addr
	end
	set $addr = $addr + 4
	set $num = $num + 1
end
end


#
#	Set BIG endian mode
#
define big
set *((unsigned long *) 0x10000000) = 0xee110f10
set *((unsigned long *) 0x10000004) = 0xe3800080
set *((unsigned long *) 0x10000008) = 0xee010f10
set *((unsigned long *) 0x1000000c) = 0xee120f10
set *((unsigned long *) 0x10000010) = 0xe1a00000
set *((unsigned long *) 0x10000010) = 0xe24ff004
set $pc = 0x10000000
stepi 6
end

define writable
set *((unsigned long *) 0x10000000) = 0xe3a01331
set *((unsigned long *) 0x10000004) = 0xe5912000
set *((unsigned long *) 0x10000008) = 0xe3822002
set *((unsigned long *) 0x1000000c) = 0xe5812000
set $pc = 0x10000000
stepi 4
end


#
#	FLASH writing code
#
define flash-erase1
set *((unsigned long *) 0x10000000) = 0xe3a01205
set *((unsigned long *) 0x10000004) = 0xe3a02020
set *((unsigned long *) 0x10000008) = 0xe3822a02
set *((unsigned long *) 0x1000000c) = 0xe1c120b0
set *((unsigned long *) 0x10000010) = 0xe3a020d0
set *((unsigned long *) 0x10000014) = 0xe3822a0d
set *((unsigned long *) 0x10000018) = 0xe1c120b0
set $pc = 0x10000000
stepi 7
shell sleep 2
set *((unsigned long *) 0x10000000) = 0xe3a020ff
set *((unsigned long *) 0x10000004) = 0xe3822cff
set *((unsigned long *) 0x10000008) = 0xe1c120b0
set $pc = 0x10000000
stepi 3
end

define flash-erase2
set *((unsigned long *) 0x10000000) = 0xe3a01205
set *((unsigned long *) 0x10000004) = 0xe3811802
set *((unsigned long *) 0x10000008) = 0xe3a02020
set *((unsigned long *) 0x1000000c) = 0xe3822a02
set *((unsigned long *) 0x10000010) = 0xe1c120b0
set *((unsigned long *) 0x10000014) = 0xe3a020d0
set *((unsigned long *) 0x10000018) = 0xe3822a0d
set *((unsigned long *) 0x1000001c) = 0xe1c120b0
set $pc = 0x10000000
stepi 8
shell sleep 2
set *((unsigned long *) 0x10000000) = 0xe3a020ff
set *((unsigned long *) 0x10000004) = 0xe3822cff
set *((unsigned long *) 0x10000008) = 0xe1c120b0
set $pc = 0x10000000
stepi 3
end
define flash-id
set *((unsigned long *) 0x10000000) = 0xe3a01205
set *((unsigned long *) 0x10000004) = 0xe3a02090
set *((unsigned long *) 0x10000008) = 0xe3822a09
set *((unsigned long *) 0x1000000c) = 0xe1c120b0
set $pc = 0x10000000
stepi 4
end

define flash-program
set *((unsigned long *) 0x10100000) = 0xe3a01205
set *((unsigned long *) 0x10100004) = 0xe3a02040
set *((unsigned long *) 0x10100008) = 0xe3822901
set *((unsigned long *) 0x1010000c) = 0xe3a03080
set *((unsigned long *) 0x10100010) = 0xe3a040ff
set *((unsigned long *) 0x10100014) = 0xe3844cff
set *((unsigned long *) 0x10100018) = 0xe3a05000
set *((unsigned long *) 0x1010001c) = 0xe3a06701
set *((unsigned long *) 0x10100020) = 0xe1c120b0
set *((unsigned long *) 0x10100024) = 0xe1d570b0
set *((unsigned long *) 0x10100028) = 0xe1c170b0
set *((unsigned long *) 0x1010002c) = 0xe1d170b0
set *((unsigned long *) 0x10100030) = 0xe1170003
set *((unsigned long *) 0x10100034) = 0x0afffffc
set *((unsigned long *) 0x10100038) = 0xe1c140b0
set *((unsigned long *) 0x1010003c) = 0xe2811002
set *((unsigned long *) 0x10100040) = 0xe2855002
set *((unsigned long *) 0x10100044) = 0xe1550006
set *((unsigned long *) 0x10100048) = 0x1afffff4
set *((unsigned long *) 0x1010004c) = 0xe1a00000
set *((unsigned long *) 0x10100050) = 0xe1a00000
set *((unsigned long *) 0x10100054) = 0xeafffffe
set $pc = 0x10100000
end

define program-loader
mem-init
mem-switch
writable
flash-erase1
flash-erase2
load /tmp/redboot-swap.o
flash-program
c
end

#
#	Startup commands...
#
set output-radix 16
set input-radix 16

target remote localhost:8888

#set endian big
#monitor endian big

set print pretty
set print asm-demangle
#display/i $pc

