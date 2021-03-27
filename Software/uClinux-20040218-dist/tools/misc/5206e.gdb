#
# GDB Init script for the Coldfire 5206e processor.
#
# The main purpose of this script is to configure the 
# DRAM controller so code can be loaded.
#
#

define addresses

set $mbar  = 0x10000001
set $simr  = $mbar - 1 + 0x003
set $icr1  = $mbar - 1 + 0x014
set $icr2  = $mbar - 1 + 0x015
set $icr3  = $mbar - 1 + 0x016
set $icr4  = $mbar - 1 + 0x017
set $icr5  = $mbar - 1 + 0x018
set $icr6  = $mbar - 1 + 0x019
set $icr7  = $mbar - 1 + 0x01A
set $icr8  = $mbar - 1 + 0x01B
set $icr9  = $mbar - 1 + 0x01C
set $icr10 = $mbar - 1 + 0x01D
set $icr11 = $mbar - 1 + 0x01E
set $icr12 = $mbar - 1 + 0x01F
set $icr13 = $mbar - 1 + 0x020
set $imr   = $mbar - 1 + 0x036
set $ipr   = $mbar - 1 + 0x03A
set $rsr   = $mbar - 1 + 0x040
set $sypcr = $mbar - 1 + 0x041
set $swivr = $mbar - 1 + 0x042
set $swsr  = $mbar - 1 + 0x043
set $dcrr  = $mbar - 1 + 0x046
set $dctr  = $mbar - 1 + 0x04A
set $dcar0 = $mbar - 1 + 0x04C
set $dcmr0 = $mbar - 1 + 0x050
set $dccr0 = $mbar - 1 + 0x057
set $dcar1 = $mbar - 1 + 0x058
set $dcmr1 = $mbar - 1 + 0x05C
set $dccr1 = $mbar - 1 + 0x063
set $csar0 = $mbar - 1 + 0x064
set $csmr0 = $mbar - 1 + 0x068
set $cscr0 = $mbar - 1 + 0x06E
set $csar1 = $mbar - 1 + 0x070
set $csmr1 = $mbar - 1 + 0x074
set $cscr1 = $mbar - 1 + 0x07A
set $csar2 = $mbar - 1 + 0x07C
set $csmr2 = $mbar - 1 + 0x080
set $cscr2 = $mbar - 1 + 0x086
set $csar3 = $mbar - 1 + 0x088
set $csmr3 = $mbar - 1 + 0x08C
set $cscr3 = $mbar - 1 + 0x092
set $csar4 = $mbar - 1 + 0x094
set $csmr4 = $mbar - 1 + 0x098
set $cscr4 = $mbar - 1 + 0x09E
set $csar5 = $mbar - 1 + 0x0A0
set $csmr5 = $mbar - 1 + 0x0A4
set $cscr5 = $mbar - 1 + 0x0AA
set $csar6 = $mbar - 1 + 0x0AC
set $csmr6 = $mbar - 1 + 0x0B0
set $cscr6 = $mbar - 1 + 0x0B6
set $csar7 = $mbar - 1 + 0x0B8
set $csmr7 = $mbar - 1 + 0x0BC
set $cscr7 = $mbar - 1 + 0x0C2
set $dmcr  = $mbar - 1 + 0x0C6
set $par   = $mbar - 1 + 0x0CA
set $tmr1  = $mbar - 1 + 0x100
set $trr1  = $mbar - 1 + 0x104
set $tcr1  = $mbar - 1 + 0x108
set $tcn1  = $mbar - 1 + 0x10C
set $ter1  = $mbar - 1 + 0x111
set $tmr2  = $mbar - 1 + 0x120
set $trr2  = $mbar - 1 + 0x124
set $tcr2  = $mbar - 1 + 0x128
set $tcn2  = $mbar - 1 + 0x12C
set $ter2  = $mbar - 1 + 0x131

end

#
#  Setup CSAR0 for the FLASH ROM.
#

define setup-cs

set *((short*) $csar0) = 0xffe0
set *((long*) $csmr0) = 0x000f0000
set *((short*) $cscr0) = 0x0d83
##set *((short*) $csar1) = 0xffff
##set *((long*) $csmr1) = 0x0000001e
##set *((short*) $cscr1) = 0x0043
set *((short*) $csar2) = 0x3000
set *((long*) $csmr2) = 0x000f0000
set *((short*) $cscr2) = 0x0d03
##set *((short*) $csar3) = 0x4000
##set *((long*) $csmr3) = 0x000f0000
##set *((short*) $cscr3) = 0x0083

end

#
# Setup the DRAM controller.
#

define setup-dram

##set *((short*) $dcrr)  = 24
##set *((short*) $dctr)  = 0x0000
##set *((short*) $dcar0) = 0x0000
##set *((long*)  $dcmr0) = 0x007e0000
##set *((char*)  $dccr0) = 0x07
##set *((short*) $dcar1) = 0x0000
##set *((long*)  $dcmr1) = 0x00000000
##set *((char*)  $dccr1) = 0x00

end


#
#	Added for uClinux-coldfire target...
#
target bdm /dev/bdmcf0

addresses
setup-cs
setup-dram
set print pretty
set print asm-demangle
display/i $pc

