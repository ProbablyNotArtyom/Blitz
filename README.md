

# NOTICE ON KICAD VERSIONS 

### the `Rev-L.zip` archive requires KiCad 5.99 or later (nightly release)

### use `Rev-K.zip` if you are on KiCad 5.1.9 or earlier

# Blitz : m68k/68030 Homebrew

<p align="center">
    <img src="https://i.imgur.com/4tw3nfW.jpg"></img>
</p>

<p align="center">
    <a href="https://hackaday.io/project/166534-blitz-32bit-68030-homebrew-with-an-isa-bus">(Link to this project's Hackaday.io page)</a>
    <br>
    <a href="https://blog.notartyoms-box.net/blitz">(Link this project's homepage on my website)</a>
</p>

-------------------------

#### Feature highlights
 * Motorola 68030 @25-50MHz + 68882 FPU
 * 4MB of static RAM + 64MB DRAM
 * 512k of Flash ROM
 * VT8242 based PS/2 Keyboard interface
 * AIC37C65CL based Floppy interface
 * ATA hard disk interface
 * 3x 8-bit ISA expansion slots
 * MicroATX form factor & PSU connector
 * Glue logic handled by a single XC9572XL CPLD

-------------------------

### Software

Blitz runs my 32-bit operating system [G-DOS](https://github.com/ProbablyNotArtyom/G-DOS).
It supports FAT filesystems, and uses a flexible driver & init subsytem that handles hardware interfacing.
G-DOS has my custom shell (G-Shell) and monitor (G-Mon) built-in, and boots directly from the onboard Flash ROM.
A CGA card can be installed in an ISA slot to provide video output, and is required to use Blitz effectively.
Lastly, it includes a bootloader that can load & bootstrap a Linux kernel off a disk.

### What this repository contains

This repository houses what should be all of the stuff needed to build a Blitz.
This includes:
 * The KiCad project files for the latest revision
 * An archive of all older board revisions
 * The VHDL code for the CPLD
 * Various datasheets and resources I used during development

### A quick history of Blitz

Blitz started as a wire-wrap prototype.
It had the 68030 hooked up to 2MB of RAM and ROM with GALs for logic, plus an LED register as its only output.
Once I had proof of code execution by seeing it blink the LEDs, I realized that wire-wrapping two 32-bit busses around was a pain in the ass, and went to design a PCB.
Around this time, I started development of G-DOS, which I would develop concurrently with the hardware.

I then went through 3 PCB stages; The first board was just the CPU+FPU, RAM, and ROM.
The second stage added on the ISA slots and the CPLD-based DRAM interface.
The third stage changed the DRAM design, then added on-board IDE, Floppy, and PS/2 interfaces.
This revision was compatible with the MicroATX form-factor so it could be installed in a PC case, and this ended up being the last revision.
Each of these stages got produced, assembled, debugged, and tested over about 2 years.

### Reflections

Blitz has been a 3 year long journey that took me from doing simple 8-bit brews all the way to designing
complex 32-bit systems.
I've learned an incredible amount about the complexities of 32-bit bus design and system logic in general.
Thousands of hours were put into this thing, going through countless revisions, and a lot of trial and error.

Although I have moved focus to other projects, there are still some parts left incomplete.
Some of the stuff hasn't been implemented fully, like the DRAM interface, and imperfections still exist.
But it has reached a point where i am content with what I've accomplished, so I am releasing all my design files and sources here.
I plan on resuming development of Blitz shortly, now that I have a break from school.

### Are you selling kits/can I make my own Blitz?

I have no plan on selling any sort of kit for Blitz.
It was made as a passion project, and I have no financial interests with it.
There are still parts that don't fully work, like the DRAM, so I recommend that you don't try and assemble one yourself, at least for now.
Since I am continuing to develop Blitz, if you want to build one, try waiting until I release a final revision board and software.
However, if you still decide to make one, I am happy to answer any questions you have.
Shoot me an email (notartyomowo@gmail.com) and I'll do my best to respond.

---------------------------
 
Blitz is licensed under the GNU General Public License v2.
Although I dont expect contributions I'd appreciate if you shared any improvements with me, by either doing a pull request, or notifying me through email. Thankey!


### Authors

* **NotArtyom** - *Everything* - [Website](http://blog.notartyoms-box.net)
