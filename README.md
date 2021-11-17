# Z280RC
Software for Bill Shen's Z280RC Single Board Z280 system on a RC2014 board

This repository contains a snapshot of the original software, plus
enhancements, updates and new utilities for Bill Shen's
Z280RC single board Z280 system - as described on the RetroBrew builders
wiki at

  https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:z280rc

The original software is located in zip files in the "original" subdirectory.

My modifications to Bill Shen's original non-banked CBIOS3 for CP/M 3 are
in the "system/cpm3-nonbanked" subdirectory - adding DS1302 clock support.  

I've also developed

* A modified CP/M 2 (in system/cpm22) to support file partitions containing
up to 2048 directory entries (four 8Mb drives on CompactFlash with 1.5Mb RAMdisk);

* A CP/M 3 BIOS (in system/bios280) supporting Banked memory (up to eight
banks of memory, four (and optionally eight) 8Mb drives, 1.5Mb RAMdisk,
optional 4-port QuadSer module);

* Ported UZI280 (in system/uzi280*) - a version of the Unix operating system; and

* Some new utility programs in the "utilities" subdirectory - XMODEM,
RESET, TIME, DU, NULU, RELHEX and updated for CP/M 3 versions of DEVICE,
HELP and KERMIT.


## Modification History (in reverse chronological order):


### 17-Nov-2021

Minor tweak to XMODEM to look for its configuration file on the
system drive (in A:XMZ280RC.CFG).  Be sure to set this with the
SYS attribute so it is seen from all user areas under CP/M 3.

```
A>set xmz280rc.cfg [sys ro]
```


### 16-Nov-2021

Ported the Z280 UART console I/O routines to the latest version of
Martin Eberhard's XMODEM version 2.9.  The new version is in the
"utilities" subdirectory as a ZIP file
[utilities/xm29z280.zip](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/xm29z280.zip)
or you can download the individual files
[utilities/XM29Z280.MAC](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XM29Z280.MAC),
[utilities/XM29Z280.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XM29Z280.HEX) and
[utilities/XM29Z280.COM](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XM29Z280.COM)
.  This version uses the same configuration file
[utilities/XMZ280RC.CFG](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XMZ280RC.CFG)
as the previous version (see below)
to obtain its default switch values (/X3 uses the built-in Z280 console UART).


### 11-Oct-2021

A new utility program to reset the system from CP/M back to the
ZZmon command prompt.  (This is the software equivalent of pressing
the reset button).

Assembler source is in
[utilities/RESET.MAC](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/RESET.MAC)
and a pre-built binary is in
[utilities/RESET.COM](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/RESET.COM)


### 08-Oct-2021

The CP/M 3 BIOS checks for the presence of a QuadSer module.  If
none is found then it reports
```
%QUADSER I/O board not detected
```
during the system start-up, and it disables the TTY0 to TTY3 devices.

However, the default character device assignments for both
the AUXIN and AUXOUT devices were not adjusted (causing an error
if you tried to use the DEVICE command).

To fix this, I've changed the defaults for the AUXIN and AUXOUT
to the NULL device.  This means if you have a QuadSer module,
then you will now need to use the DEVICE command (in the PROFILE.SUB
start-up file) to configure the AUXIN and AUXOUT devices under CP/M 3.
For example -

```
C>device aux:=tty2[19200,xon]                                                  
A:DEVICE   COM  (User 0)                                                        
                                                                                
Physical Devices:                                                               
I=Input,O=Output,S=Serial,X=Xon-Xoff                                            
UART   NONE  IOS    TTY0   19200 IOS    TTY1   19200 IOS                        
TTY2   19200 IOSX   TTY3   19200 IOS                                            
                                                                                
Current Assignments:                                                            
CONIN:  = UART                                                                  
CONOUT: = UART                                                                  
AUXIN:  = TTY2                                                                  
AUXOUT: = TTY2                                                                  
LST:    = Null Device                                                           
                                                                                
                                                                                
C>
```

To set them back to NULL, use

```
C>device auxin:=null, auxout:=null                                             
A:DEVICE   COM  (User 0)                                                        
                                                                                
Physical Devices:                                                               
I=Input,O=Output,S=Serial,X=Xon-Xoff                                            
UART   NONE  IOS    TTY0   19200 IOS    TTY1   19200 IOS                        
TTY2   19200 IOSX   TTY3   19200 IOS                                            
                                                                                
Current Assignments:                                                            
CONIN:  = UART                                                                  
CONOUT: = UART                                                                  
AUXIN:  = Null Device                                                           
AUXOUT: = Null Device                                                           
LST:    = Null Device                                                           
                                                                                
                                                                                
C>
```

In addition, I have verified that my CP/M 2, CP/M 3 and UZI280
(and Hector Peraza's RSX280) operating systems all work under the
new Z280RC emulator by Michal Tomek.

The emulator (which runs under Linux/macOS and Windows MinGW)
is available from

https://github.com/mtdev79/z280emu

Note that the UZI280 operating system still needs to be loaded
from CP/M and not yet from the ZZmon monitor -

```
C>bootuzi                                                                       
C:BOOTUZI  COM                                                                  
                                                                                
UZI280 is (c) Copyright (1990-96) by Stefan Nitschke and Doug Braun             
                                                                                
boot: 0                                                                         
UZI280 version 1.12                                                             
login: root                                                                     
[root]/usr/root#
```

I'm working on adding support for the QuadSer module and direct boot capability
from ZZmon to UZI280 and a CP/M RESET utility (to restart back to ZZmon).

They'll be added here soon.



### 07-Oct-2021

Change the CP/M 2 cold-boot default drive to B: (since this is where
the CP/M 2.2 system files are loaded).  Updated only the "BIG" version
of the
[CPM22BIG.MAC](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22BIG.MAC)
monolithic source file - and there's a new Intel
HEX loader file for
[CPM22BIG.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22BIG.HEX)
in the system/cpm22 subdirectory.  No more "REQUIRES CP/M 3" messages
when you type a "PIP" command!  (and a warm-boot returns to the current
selected default drive).


### 21-Sep-2021

Please note:  The CP/M 3 system images I've made available are linked
with Simeon Cran's ZPM3 replacement BDOS routines.

ZPM3 is a Z80 coded CP/M 3 compatible BDOS replacement which includes

* Bug fixes (the "Random Read Bug" and System Control Block sanity checks),

* Enhancements to the BDOS Parse (Function 152) to accept drive/user
area filespecs (like C1:FILE.DAT),

* New functions for file time-stamps manipulation - Get Stamp (Function 54),
Use Stamp (Function 55) can retrieve and change a file's time-stamp, and

* Command-line history and WordStar(C)-style editing for Read Console
Buffer (Function 10).

You'll find more details
[here](https://raw.githubusercontent.com/agn453/Z280RC/master/system/zpm3/ZPM3.TXT)
and I've added the ZPM3 distribution archive (includes sourcecode) in the
[system/zpm3/ZPM3S.ARC](https://raw.githubusercontent.com/agn453/Z280RC/master/system/zpm3/ZPM3S.ARC)
file.

If you wish to revert to the Digital Research CP/M 3 BDOS, you'll need to
select it with the DRI-CPM3.SUB file and rebuild a new CPM3.SYS file (e.g.
using the BIGBIOS.SUB build procedure from the BIOS source area).


### 20-Sep-2021

I updated the CP/M 3 disk image to include the CP/M 2.2 BIOS sources
(on drive B: user area 1), and included Infocom games and terminal
set-up program (on drive D: in user area 0).  Also, updated the IDEHD.MAC
CompactFlash driver module to match the latest version.

Get it from
[z280rc-bigcpm-swab.img.gz](https://raw.githubusercontent.com/agn453/Z280RC/master/system/disk-image/z280rc-bigcpm-swab.img.gz)


### 19-Sep-2021

There's now an updated CP/M 3 banked memory system supporting 2048
directory entries on the A: to D: drives (and if you require additional
drives using the PARTN8 configuration option).

To be able to load CPM3.SYS from the revised drive layout, you'll need
to install the new "BIG" CP/M 3 Loader from
[CPMLBIG.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/system/bios280/CPMLBIG.HEX)
onto the Z280RC's CP/M 3 boot area using ZZmon V2.3 by uploading this
CPMLBIG.HEX Intel HEX file, and then write it to the boot track on the
CompactFlash disk using the C3 command.

Next, you'll need to copy the
[CPM3BIG.SYS](https://raw.githubusercontent.com/agn453/Z280RC/master/system/bios280/CPM3BIG.SYS)
to the A: drive as CPM3.SYS, and install the CP/M 3 distribution files (be
careful not to overwrite the new CPM3.SYS when you do this).

You can use cpmtools 'cpmcp' on a image file that's been transferred to
a Windows, Linux or macOS system - using updated diskdefs from
[here](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/diskdefs-z280rc-swab-BIG),  or by using the CP/M 2.2 system and XMODEM as detailed below

```
TinyZZ Monitor for Z280RC v2.3 18-Sep-2021


>Boot
1 - User Apps
2 - CP/M 2.2
3 - CP/M 3
4 - RSX280
5 - UZI280
Select: 2 Press Return to confirm:

Copyright 1979 (c) by Digital Research
CP/M 2.2 for Z280RC
20210918 w/2048 CF A:-D:

a>b:
b>pip xm.hex=con:[hz]
```

Send the Intel HEX load file for XMODEM from the
[XM27Z280.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XM27Z280.HEX)
file in ASCII mode to PIP.  After a while, the b> prompt will appear, then
load the HEX file to make XM.COM then download the XMODEM default configuartion
file using xmodem protocol (send it using your comms program) from 
[XMZ280RC.CFG](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/XMZ280RC.CFG) -

```
b>load xm

FIRST ADDRESS 0100
LAST  ADDRESS 10FF
BYTES READ    1000
RECORDS WRITTEN 20

b>xm xmz280rc.cfg /r/c/x3/z9/q
```

Now you have a working XMODEM (XM) command, use it to transfer files to the
CP/M drive.  You'll only need to use the /r (receive) or /s (send) options
and maybe the /c (crc/checksum mode) from now on, if you have the
XMZ280RC.CFG file on the current drive.

To save you some time, I have prepared an image of my CompactFlash
CP/M drive partitions (A: thru D:) which is populated with a large range
of software (compilers like Hi-Tech C, FORTRAN-80, BASIC-80, COBOL-80,
RML ALGOL, TurboPascal plus the Z3PLUS Z-System and various editors and
assemblers.  The CP/M 3 BIOS source files are on drive C in user area 1.

Download the gzip'ed image from
[z280rc-bigcpm-swab.img.gz](https://raw.githubusercontent.com/agn453/Z280RC/master/system/disk-image/z280rc-bigcpm-swab.img.gz)
uncompress it with gunzip then write it to your blank 128Mb or 256Mb
CompactFlash card using dd with the byte-swap option.

For example (when /dev/sdX is the CompactFlash
device name under Linux)

```
gunzip z280rc-bigcpm-swab.img.gz
sudo dd if=z280rc-bigcpm-swab.img of=/dev/sdX bs=512 count=65536 conv=swab
```

(substitute for /dev/sdX as appropriate on your system).


### 18-Sep-2021

The changes I made to CP/M 2.2 to support 2048 directory entries (instead
of the original 512 directory entries) seem to be working.

I've updated the system/cpm22/CPM22ALL.MAC file to include a symbol
definition to enable this.  Just change the symbol DE2048 to -1 and
rebuild a new Intel HEX loader file (as I outlined below).

For your convenience, I've included 
[CPM22BIG.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22BIG.HEX)
that you can load into ZZmon V2.3 (see below), then write this
to a *NEW* or *SPARE* CompactFlash card.

WARNING:  THIS CHANGES THE CP/M DISK DIRECTORY AND FILE ALLOCATION
ON EACH OF THE CP/M PARTITIONS - so don't do this to your previous
working CompactFlash card.

In addition to supporting a larger number of files, I've altered the disk
allocation to use the previously unused track 63 on each of the A:, B:,
C: and D: partitions.  The latter three are now 8192 Kilobytes, while
the A: drive has 1 reserved track for the system boot loaders (it is 8064 Kb).

Once you have uploaded CPM22BIG.HEX and issued the C2 command to write
the new CP/M 2.2 image to the boot track, format each of the drives on
your *NEW* CompactFlash drive using the XA, XB, XC, XD (and XE to clear
the RAMdisk). Now you can proceed to load the CP/M 2.2 programs onto the
RAMdisk (drive E:) from the
[CPM22DRI.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22DRI.HEX)
file.

```
TinyZZ Monitor for Z280RC v2.3 18-Sep-2021                                      
                                                                                
                                                                                
>Boot                                                                           
1 - User Apps                                                                   
2 - CP/M 2.2                                                                    
3 - CP/M 3                                                                      
4 - RSX280                                                                      
5 - UZI280                                                                      
Select: 2 Press Return to confirm:                                              

Copyright 1979 (c) by Digital Research                                          
CP/M 2.2 for Z280RC                                                             
20210918 w/2048 CF A:-D:
                                                                                
a>b:                                                                            
b>dir                                                                           
No file                                                                         
b>e:                                                                            
e>dir                                                                           
E: ASM      COM : BIOS     ASM : CBIOS    ASM : DDT      COM                    
E: DEBLOCK  ASM : DISKDEF  LIB : DUMP     COM : DUMP     ASM                    
E: ED       COM : LOAD     COM : MOVCPM   COM : PIP      COM                    
E: STAT     COM : SUBMIT   COM : SYSGEN   COM : XSUB     COM                    
e>pip b:=e:*.*[v]                                                               
                                                                                
COPYING -                                                                       
ASM.COM                                                                         
BIOS.ASM                                                                        
CBIOS.ASM                                                                       
DDT.COM                                                                         
DEBLOCK.ASM                                                                     
DISKDEF.LIB                                                                     
DUMP.COM                                                                        
DUMP.ASM
ED.COM                                                                          
LOAD.COM                                                                        
MOVCPM.COM                                                                      
PIP.COM                                                                         
STAT.COM                                                                        
SUBMIT.COM                                                                      
SYSGEN.COM                                                                      
XSUB.COM                                                                        
                                                                                
e>b:                                                                            
b>stat b:                                                                       
                                                                                
Bytes Remaining On B: 8004k                                                     
                                                                                
b>stat b:dsk:                                                                   
                                                                                
    B: Drive Characteristics                                                    
65536: 128 Byte Record Capacity                                                 
 8192: Kilobyte Drive  Capacity                                                 
 2048: 32  Byte Directory Entries                                               
    0: Checked  Directory Entries                                               
  256: Records/ Extent
   32: Records/ Block                                                           
 1024: Sectors/ Track                                                           
   64: Reserved Tracks                                                          
                                                                                
b>
```

Next up, I'll do the CP/M 3 system updates... stay tuned!


### 17-Sep-2021

In preparation for increasing the number of directory entries on the CP/M
CompactFlash drives A: to D:, I have uploaded my modified
source-code for CP/M 2.2 that can be built using Hector Peraza's ZSM4
Macro Assembler.  This also contains a (minor) fix to the RAMdisk size
to match the Z280RC's memory (Bill Shen's original source was for the TinyZ280
which supported larger SIMM72 DRAM modules, whereas the Z280RC has 2Mb of
static RAM).

To assist with the generation of a CPM22ALL.HEX in Intel
HEX loader format, I've added a RELHEX utility that converts the
assembler's .REL output file into .HEX to the utilities subdirectory. You'll
find the source for RELHEX12.MAC
[here](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/RELHEX12.MAC),
and a compiled CP/M binary)
[here](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/RELHEX12.COM).

To prepare CPM22ALL.HEX from the source in
[CPM22ALL.MAC](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22ALL.MAC)
use commands like -

```
	ZSM4 =CPM22ALL
	RELHEX12 CPM22ALL
```

To install this, you may wish to grab and install the
latest ZZmon2 monitor V2.3 from Hector Peraza's GitHub repository
at https://github.com/hperaza/ZZmon2, clear
memory using the Z command, then upload 
[CPM22ALL.HEX](https://raw.githubusercontent.com/agn453/Z280RC/master/system/cpm22/CPM22ALL.HEX)
to the running monitor. Finally, copy it to the CP/M 2 boot sectors using 
the C2 comand.  For example -

```
TinyZZ Monitor for Z280RC v2.2 17-Sep-2021

>Zero memory
 Press Return to confirm:

>
```
Now send the Intel HEX file in ASCII mode.  You should see a few rows of dots
like

```
>..............................................................................
...............................................................................
...............................................................................
...............................................................................
...............................................................................
.................................0X


>Copy to CF
0 - Boot & ZZmon
1 - User Apps
2 - CP/M 2.2
3 - CP/M 3
Select: 2 Press Return to confirm:


>Boot
1 - User Apps
2 - CP/M 2.2
3 - CP/M 3
4 - RSX280
5 - UZI280
Select: 2 Press Return to confirm:

Copyright 1979 (c) by Digital Research
CP/M 2.2 for Z280RC
20180727 1.5 meg RAMdisk

a>
```

I'll post the new 2048 directory entry modifications in the next few days (after
I test them out).


### 29-Nov-2020

I've included my cpmtools diskdef file entries in the "utilities"
subdirectory.  This allows you to perform file copies and manipulations
to an image file of the Z280RC's CompactFlash CP/M file partitions using
the cpmtools utilities on Windows, macOS and Linux.

Please note that you need to create or write images that have the
bytes swapped (the Z280RC interface has the bytes written in big-endian
order).  The Linux 'dd' command has a 'conv=swab' option that does this.

The definition file can be downloaded from
[here](https://raw.githubusercontent.com/agn453/Z280RC/master/utilities/diskdefs-z280rc-swab).

After adding these entries to the system's diskdefs file you can manipulate
disk images by specifying the '-f z280rc-X' format (where X corresponds
to the CP/M drive letter in lower case).

For example, from macOS or Linux -

```
# Copy CP/M partition from CompactFlash mounted on the device /dev/sda
sudo dd if=/dev/sda of=z280rc-cf.img bs=512 count=131072 conv=swab

# list the CP/M directory of the B: drive partition
cpmls -f z280rc-b -l z280rc-cf.img
[directory of B: partition omitted]

# copy a file to the CP/M E: drive in user area 10
cpmcp -f z280rc-e z280rc-cf.img filename.txt 10:FILE.TXT

# To write it back to CompactFlash use -
#
sudo dd if=z280rc-cf.img of=/dev/sda bs=512 count=131072 conv=swab
```


### 10-Jul-2020

This is just a note about the way I set-up CP/M 3 on my Z280RC.

You might have noticed that I set the default login disk as drive C:.
This is mainly as a convenience - as this is my working directory.  You
can change this using the GENCPM program when you do a system generation
by answering the question with your desired default (some people like
it to be A:) -

```
C>gencpm
A:GENCPM   COM  (User 0)


CP/M 3.0 System Generation

 
Default entries are shown in (parens).
Default base is Hex, precede entry with # for decimal

Use GENCPM.DAT for defaults (Y) ?

Create a new GENCPM.DAT file (N) ? y

Display Load Map at Cold Boot (Y) ?

Number of console columns (#80) ?
Number of lines in console page (#24) ?
Backspace echoes erased character (N) ?
Rubout echoes erased character (N) ?

Initial default drive (C:) ? a:
...
(answer all remaining prompts with Return key)
```

Also, since this is the default drive upon booting, you'll need to
place a copy of SUBMIT.COM on this drive so you can use a PROFILE.SUB
file to tweak the system search path and device settings after boot-up.

If you have the CP/M 3 system files on drive A: then make these
accessible from all user area and drives by ensuring they have the
SYS attribute (and read-only if you're extra cautious).  Do this using -

```
C>a:set a:*.com [sys ro]
C>a:set a:help.hlp [sys ro]
```

(also you may wish to include various libraries and programming header
files) and then

```
C>a:pip c:=a:submit.com[r
```

Finally create a PROFILE.SUB like the following (create this with an editor
if you prefer).  As you see, I set my search path to use the current
drive, the RAMdisk, drive C: and finally drive A:.  I also copy the
XMODEM configuration file to each disk in user 0 so I don't have to
specify the parameters to force it to use the console UART.


```
C>a:pip c:profile.sub=con:
; C:PROFILE.SUB
;
; XMODEM default configuration to M:
a:pip m:=a:xmz280rc.cfg[wrg0]
a:set m:xmz280rc.cfg [sys ro]
; Search path is current drive, RAMdisk, drive C and drive A
a:setdef * m: c: a: [order=(com,sub) display page uk temporary=m]
; Use QuadSer port TTY3 at 115200 bps for Kermit (using the AUX: device)
a:device aux:=tty3[115200]
a:date
^Z
C>
```

(That last line is a Ctrl-Z to end the file).

With the system set-up like this, I no longer need to specify
the disk drive to run a program.  Also, configure programs like
TurboPascal and WordStar with their SETUP programs to load their
overlays etc from drive A: too.


### 07-Jul-2020

Included a copy of CP/M KERMIT V4.11 (configured for CP/M-Plus) so
you can test the QuadSer ports.
[KERMIT.COM](https://github.com/agn453/Z280RC/blob/master/utilities/KERMIT.COM)
is available in the utilities subdirectory (or click the link).

Just set the AUX: port assignment and baud rate prior to running KERMIT
using the DEVICE command (see below).  When you use KERMIT's *connect*
command, what you type will be sent out the QuadSer port, and any
received characters will be displayed on the console.  If you connect
the port to another computer capable of running its version of
kermit in *server* mode, you will be able to transfer files using
KERMIT's *get* and *send* commands.

Further details (and sourcecode) for KERMIT can be downloaded from
http://www.kermitproject.org/archive.html#cpm80


### 01-Jul-2020

Built a modified version of the CP/M-Plus DEVICE utility to support
substituted baudrates.  The new version will accept 28800, 38400,
57600 and 115200 as speed values.  The modified PL/M-80 sourcecode,
[DEVICE.COM](https://github.com/agn453/Z280RC/blob/master/utilities/DEVICE.COM)
as well as updated CP/M Plus
[HELP.HLP](https://github.com/agn453/Z280RC/blob/master/utilities/HELP.HLP)
(and source in HELP.DAT) are in the utilities subdirectory.

Just copy DEVICE.COM and HELP.HLP to your system drive and set them
with the SYS attribute to make them available.

```
A>device tty3[115200
A:DEVICE   COM

Physical Devices:
I=Input,O=Output,S=Serial,X=Xon-Xoff
UART   NONE  IOS    TTY0   19200 IOS    TTY1   19200 IOS
TTY2   19200 IOS    TTY3   115K2 IOS

Current Assignments:
CONIN:  = UART
CONOUT: = UART
AUXIN:  = TTY3
AUXOUT: = TTY3
LST:    = Null Device

A>
```

Also, I added some notes regarding a hardware modification to the QuadSer
4-port module that prevents spurious interrupts from floating inputs
on the serial Rx pins of each port.  You'll find it in the QuadSer-notes
folder with a
[README](https://github.com/agn453/Z280RC/blob/master/QuadSer-notes/README.md)
and photo (courtesy of Hector Peraza) of the mods.

And finally, I included an
[application note](https://github.com/agn453/Z280RC/blob/master/QuadSer-notes/oxan5-Software-Examples-for-the-OX16C95x.pdf)
PDF retrieved from the Oxford Semiconductor pages on the Wayback machine.
These were useful to me when I was writing the drivers for the OX16C954
4-port UART chip.


### 30-Jun-2020

Added CP/M Plus support for Bill Shen's QuadSer Rev1 4-port UART module
in RC2014 form-factor.

This is a UART module using the Oxford Semiconductor Ltd.
OX16C954 rev B chip with 128-byte FIFOs for each receiver and
transmitter.  It has 4 independent UART ports with program selectable baud
rates between 50 and 115200 bps in a variety of framing formats.  Higher
speeds are also possible - but not implemented for CP/M-Plus.

For hardware details see Bill Shen's pages at 

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:quadser:ec4z280rc

The configuration files now contain a "quadser" equate that you set
to "true", along with a "use$device$io" equate to enable this support.

The current implementation uses polled I/O (and I intend to add support
using interrupts soon).

With "quadser" enabled, four additional serial devices named TTY0,
TTY1, TTY2 and TTY3 will be enabled.  You can assign one or more of
these to the console (CON:), auxiliary (AUX:) or printer device (LST:)
using the DEVICE command.  Speed settings can also be adjusted (see below).

For example, to specify TTY3 as the AUX device at 9600 bps use

```
A>device aux:=tty3[9600]
```

or to have the console input and output simultaneously on two devices
(e.g. the Z280's UART and TTY0 at 19200 bps with XON/XOFF flow control) use 

```
A>device con:=uart,tty0[19200,xon]
```

Since CP/M Plus was released, it is now common to use higher speeds than
the 19200 bps maximum that the DEVICE command and internal CP/M data
structures allow.  Therefore I have substituted four of the less
common data rates as follows -

```
    CP/M Old    Now
        75      28800
       134      38400
       150      57600
      1800     115200
```

Until I produce a modified CP/M Plus DEVICE program, you can specify
higher speeds by using the corresponding old value on a DEVICE command.

For example, to use TTY3 at 115200 bps use -

```
A>device tty3[1800]
A:DEVICE   COM

Physical Devices:
I=Input,O=Output,S=Serial,X=Xon-Xoff
UART   NONE  IOS    TTY0   19200 IOS    TTY1   19200 IOS
TTY2   19200 IOS    TTY3   1800  IOS

Current Assignments:
CONIN:  = UART
CONOUT: = UART
AUXIN:  = TTY3
AUXOUT: = TTY3
LST:    = Null Device


A>
```

Pre-built versions of the various banked configurations can be found
in the system/bios280 subdirectory.

You can also download all the CP/M Plus BIOS files in the following
ZIP file -

https://github.com/agn453/Z280RC/blob/master/system/bios280/bios280.zip

### 09-Jan-2020

Fixed some erratic date/time setting under CP/M-Plus.

I've tweaked the DS1302 time-keeping chip routines in the BIOS CLOCK
module (and the TIME.COM utility) to adjust the read/write timing
to use a more accurate 1 microsecond serial timing for the 3-wire
interface.

### 12-Jun-2019

I found the original DU-V88.LBR of Ward Christensen's Disk Utility
(better late than never) and added it to the "utilities" subdirectory

### 21-Dec-2018

Some more tinkering with I/O page selection, interrupt routine and word
address alignment.  Also I found the missing .tar file with HiTech C
header files and the UZI280 system libraries.  I've put this UZISYTAR.LBR
in system/uzi280-usrtar with the other .LBR files that can be extracted
and written to the UZI partition using the XFRSYTAR.SUB submit file.
When you boot UZI280 you'll need to extract this tar file below the root
directory using
```
/bin/sh
cd /
/bin/tar xvf /Tapes/uzi112s.tar
```

The HiTech C compiler now works.
```
/bin/sh
cd /usr/root
cat >hello.c <<EOF
#include <stdio.h>
main()
{
  printf("Hello world from UZI280 on the Z280RC!\n");
}
EOF
cc hello.c
./hello
```

I also added some utilities to the MKBOOT.SUB and COPYALL.SUB submit files
in the system/uzi280-xutils directory.  This allows an UZI system partition
for system recovery to be created (on either /dev/wd1 or /dev/wd2).  When
you run (say) MKBOOT.SUB you'll need to specify a parameter to select the
partition.  Use "2" for /dev/wd1 or "3" for /dev/wd2.  After running this
you can BOOTUZI and answer the boot: prompt with 2 or 3 (instead of 0) to
start the corresponding partition, Login as root and do a file system check
on the other (non-mounted) partitions - e.g. `fsck /dev/wd0` to check the
primary UZI partition.  You can also create mount-points and mount the other
file systems for backup purposes -
```
/bin/sh
mkdir /disk0
mount /dev/wd0 /disk0
cd /disk0; tar cvf /Tapes/backup-wd0.tar .
umount /dev/wd0
```


### 18-Dec-2018

I've implemented support for the DS1302 timekeeper chip in UZI280
to set the time-of-day and fixed an issue with I/O page selection affecting
maskable interrupts in my CompactFlash device support routine (IDECF.AS).
You can fetch MACHDEP.C MACHASM.C and IDECF.AS from system/uzi280-kernel/
and re-compile the kernel using MAKE - or download the UZIKERNL.LBR file
again to get the complete kit of kernel build files.  Fixing the aforementioned
issues has introduced another problem with the UZI280 "ps" command which was
previously working but now gives a "Can't read /dev/kmem: Error 0" which I
am investigating.

Also, a couple more UZI280 binary tar files for the HiTech C compiler (uses
CPM emulation) and "User Binaries" have been added to system/uzi280-usrtar
as well as in the CP/M library files UZICTAR.LBR and UZIUSTAR.LBR. Under
CP/M use NULU to extract the two .tar files and execute the XFRCTAR.SUB
and XFRUSTAR.SUB to transfer them to the /Tapes directory on the UZI
file system partition.  Extract these files under UZI280 using a command
like the following -
```
/bin/sh
cd /
/bin/tar xvf /Tapes/uzi112c.tar
/bin/tar xvf /Tapes/uzi112u.tar
```


### 16-Dec-2018

Added UZI280 binaries to system/uzi280-bintar/UZIBNTAR.LBR.  Use the
XFRBNTAR.SUB submit file to transfer these to the /Tapes directory on
the UZI filesystem partition, then boot up UZI280, login as root (no
password) and extract the tar files using something like the following

```
/bin/sh
cd /bin; for f in /Tapes/bin_0*.tar; do /bin/tar -xvf $f; done
```

CP/M utilities for manipulating library archives (.LBR files) and Disk
Utility V8.8 (suitable for accessing the CP/M-Plus disk drive
partitions and sector view/editing) have also been added to the utilities
folder as NULU.COM and DU.COM


### 15-Dec-2018

I've managed to modify the UZI280 operating system to boot up on
the Z280RC.  There's been a few hiccups along the way (mainly dealing
with a mixture of word-count and byte-count DMA transfers) and the requirement
for using a modified version of the HiTech C V3.09 compiler and Z280
library routines to build the system.  I based my modifications on the
UZI280 source code V1.12 for the Tillman Reh CPU280 system from
http://oldcomputers-ddns.org/public/pub/rechner/zilog/z280/uzi280/index.html

My modified sources are in the system/uzi280-kernel and system/uzi280-xutils
folders.  There are two CP/M library files UZIKERNL.LBR and UZIXUTIL.LBR
that can be transferred and extracted (using NULU.COM under CP/M-Plus) to
different drives/user areas (I use E0: for the kernel files, F0: for the
CP/M Utilities).  A MAKEFILE in each can be used to build the libraries
and UZI system image using the MAKE command.  XUTILS.SUB will build two
CP/M utilities MKFS and UCP for creating a UZI partition and copying files
from CP/M.  The log-file system/kernel/UZIBUILD.LOG is a transcript of the
build process and start-up.

Still to-do is to copy the utilities and CP/M emulator onto the UZI
partition.


### 14-Dec-2018

Squeeze a few more bytes from common memory.  Add a CLOCK$50HZ setting
to select between the 50Hz interrupt heart-beat clock using C/T 0 & C/T 1
cascaded (see below) and a 128Hz clock using only Counter/Timer 0.


### 08-Dec-2018

Implement and test Z280 on-chip device interrupts with a 20ms heart-beat
counter (a single word counter at BIOS+063h) using cascaded Counter/Timer
0 and 1.  These operate in timer mode and an interrupt is generated at
50Hz.  Enable this by setting both INT$ENABLED and USE$TIMER$INTERRUPT
in the configuration file (CONFINT8.LIB has this enabled).  Also removed
the dual banked testing version (CPM3BNK1.SYS) - but you may still
generate it if required using the BNK1BIOS.SUB submit command file.


### 05-Dec-2018

Updated the DS1302 timekeeper chip TIME program (in the utilities directory)
to support a Banked system (this makes adjusting the time-of-day easier
than using the CP/M-Plus DATE command).

Interrupt and Trap handling using Interrupt Mode 3 has been tested for
System Calls and error traps (divison by zero, divide quotient exceptions,
and memory access violations).  Device interrupts are not yet implemented.
Set the INT$ENABLED equate to TRUE to enable this in the configuration
file (CONFINT8.LIB is supplied with DEBUG still enabled). CPM3INT8.SYS is
a bootable system image with this feature active.


### 29-Nov-2018

Squeeze as much as possible out of the Banked system common memory
to maximise the size of the CP/M transient program area - resulting in
61KB free for user programs.  There's a new configuration setting to
remove the Character I/O device support in BIOSKRNL and CHARIO modules.
I've defaulted USE$DEVICE$IO to FALSE to only use the Z280 UART console
(and free up common memory).

Also clean-up the conditional code to build a CPMLDR3.HEX loader. Now the
settings in the CONFLDR.LIB configuration correctly build the aforementioned
loader that you can write to the CompactFlash boot tracks using the inbuilt
ZZMON monitor.  Just upload the Intel Hex format file directly (e.g. using
the File menu Send file... option of Tera Term VT) and do a "C3" command
to write it to the CP/M-Plus boot loader (sectors 1..15 of track 0).


### 26-Nov-2018

Revised the banked memory layout to make the TPA and common memory
contiguous.  The DMAXFR routine now works properly (so you may choose
to set USE$DMA$TO$COPY to TRUE to perform memory-to-memory transfers
using the Z280's on-chip DMA controller instead of the Memory Management
Unit).  Coming up next... Interrupts and System/User mode operation.


### 20-Nov-2018

I examined the DMAXFR code with a view to fixing the defect regarding
transfers crossing the common memory address boundary.  The modifications
increase the size of the MOVE routine considerably (if I stick to the
current physical-logical memory addressing configuration).  I've therefore
decided to not release updated DMA routines and instead will keep the
memory management work-around in place for now.  I may revise the memory
allocation in future though (and make the transient program area and
common memory contiguous).  Do not set the USE$DMA$TO$COPY configuration
item to TRUE in any of the configuration CONF*.LIB files for now.


### 16-Nov-2018

USE$BIG$DRM now causes the extra drives E: thru H: to have 2048 directory
entries (the maximum for the 4096 byte block size).  Updated IDEHD.MAC and
the CPM3*.SYS files.  Also added a check for the maximum number of directory
entries for a given block size in the CPM3M80.LIB macro library (a version
of the Digital Research supplied CPM3.LIB that works with Microsoft's M80
and Hector Peraza's ZSM4).

Also added an initialisation routine to the RAMDISK module to prompt for
and format drive M: if it doesn't have a valid CP/M directory label.


### 15-Nov-2018

I was running out of space/directory entries so I built a version of
Banked CP/M-Plus with eight 8MB CompactFlash drives.  See the CONFBNK8.LIB,
BANKED8.DAT, CPM3BNK8.SYS and BNK8BIOS.SUB files in the system/bios280
directory.

Depending on the actual size of CompactFlash you have, play it safe
and use at least a 128MB one if you use this (64MB is not the same as 64MiB).

Drives E: thru H: are configured with 1024 directory entries each.  A
new configuration option USE$BIG$DRM enables 1024 directory entries
(512 is the default for all other drives).


### 15-Nov-2018

Hector Peraza's latest beta 11 of ZSM4 - a Z80/Z180/Z280 Macro-Assembler
available at
https://www.retrobrewcomputers.org/forum/index.php?t=msg&th=93&goto=3700&#msg_3700
found a typo in IDEHD which I fixed.  Also went through and commented/
removed superfluous lines from the BIOS sources.  No object code changes -
just cosmetic!


### 14-Nov-2018

Added a modified version of Martin Eberhard's XMODEM 2.7 (for file transfer
via the console serial port).  This now includes routines to drive the
built-in Z280 UART directly.  To select the internal routines use the /X3
switch on the XMODEM command line (or make it the default by having the
XMZ280RC.CFG in the default directory). You'll find these files in the
utilities folder or you can download a ZIP containing them directly from 
https://github.com/agn453/Z280RC/blob/master/utilities/xm27z280.zip


### 13-Nov-2018

DMAXFR is not copying memory to memory when the source or destination
physical address crosses one of the logical address bank boundaries.
This is what has been causing interbank moves and disk I/O to fail
when more than two banks have been configured. (Bank 0 and Bank 1 work
because there is no difference between physical and logical addresses
up to the common boundary in Bank 1 - i.e. 01DFFFF).  The CP/M 3 PIP
program was using memory across this boundary - hence why a copy with
verify was failing for large files.

Anyhow, I've modified the MOVE and Disk I/O routines to use the MMU
to shuffle data between Banks (using just logical addresses) and everything
now works.  The Banked configuration (CONFBANK.LIB and BANKED.DAT) now
has Debugging disabled and is using Banks 1..3 with a 60KB TPA (CPM3BANK.SYS).
I've left the Bank0/1 version with the debugger enabled (CONFBNK1.LIB
and BANKED1.DAT resulting in CPM3BNK1.SYS) and the non-banked (CONFNBNK.LIB
and NONBANK.DAT in CPM3NBNK.SYS).

I'll update the DMAXFR routine soon to remedy the problem (described above)
shortly. In the meantime enjoy a Banked version of CP/M-Plus on the Z280RC!

PS; I've included a ZIP file containing all of the system/bios280 files as
https://github.com/agn453/Z280RC/blob/master/system/bios280/bios280.zip
to assist those not wanting to download the whole GitHub repository.


### 12-Nov-2018

Added USE$DMA$TO$COPY configuration equate.  If set to FALSE then extended
memory moves using the BIOS MOVE routine will use the memory management unit
to copy data from one bank to another.  Using this setting I confirmed
that the DMA transfers using the Z280's on-chip controllers were dodgy!
I changed the DMAXFR routine in the BIOSKRNL module to in-line program
the DMA channel without looping.

The Banked memory version using just bank 0 and 1 (CPM3BNK1.SYS) now seems
to be working.


### 11-Nov-2018

Added DEBUG capability to enter the debugger upon CTRL-P being typed
at the console.  Also made a special banked configuration that supports
only two banks (bank 0 and 1) with directory hashing disabled and only
a single directory and data buffer for each drive in bank 0.  The
resulting CPM3BNK1.SYS appears to be working (and I'm now convinced
there's a problem with CP/M 3 PIP corrupting buffers).

Also added CPM3ADD2.MAC which (when assembled and linked) will show
information about the running system's BIOS, SCB and disk drive data
(DPH and DPB).


### 08-Nov-2018

Updated disk modules to use hard-coded disk parameter headers and disk
parameter blocks with internal disk allocation vectors (rather than
letting GENCPM set these up).  This is selectable via a USE$DISK$MACROS
conditional equate in the CONF*.LIB files.  Unfortunately, this had no
effect on the PIP file copying verification error under the banked
system.  Non-banked and Loader configurations work fine!

 
### 06-Nov-2018

My own version of a CP/M Plus BIOS for the Z280RC are in the "system/bios280"
subdirectory.

There are submit files that build Non-banked (working), Banked (still has
problems - see below*) and Loader .COM files (not yet integrated into the
boot loader track on the CompactFlash drive).

The Y2K patched versions of the system page relocatable BDOS modules for
CP/M 3 and the ZCPM3 system page relocatable compatible BDOS modules can
be selected using DRI-CPM3.SUB or USE-ZPM3.SUB prior to building everything
with BUILDALL.SUB.

You'll need Hector Peraza's ZSM4 Z80/Z180/Z280 Macro-Assembler V4.0 beta 9
or later as well as CP/M-Plus supplied LINK and GENCPM utilities too.

ZSM4 is available from the RetroBrew Computers Forum at
https://www.retrobrewcomputers.org/forum/index.php?t=msg&th=93&goto=3700&#msg_3700

The BUILDALL.SUB build procedure produces a CPM3NBNK.SYS and CPM3BANK.SYS
system image that can be copied to the boot drive user area 0 as TEST.SYS
and loaded via a special version of the CP/M Loader - TEST.COM

* Known problem with Banked version.  I've left my debugging enabled, so
there is superfluous output for BIOS kernel routines, and a simple debugger
is invoked prior to starting CP/M's console command processor.  This allows
memory and register examination as well as memory management manipulation.
The system seems to start fine, and you can navigate different drives and
run programs.  If I try to use PIP to copy large files from one of the
CompactFlash drives (A: thru D:) to the RAM disk (drive M:) I get
verification errors and from then on there appears to be a memory corruption.

I suspect some issue with allocation of buffers (which I'll continue to look
at) - however, if you see an obvious error, please raise it as a GitHub
issue.

### 18-Jul-2018

Only an updated Non-Banked CBIOS3 and a CP/M utility to support the on-board
clock calendar timekeeping chip (TIME.MAC) have been added.
The new CBIOS3 adds support for the DS1302 timekeeper to Non-Banked
CP/M-Plus as well as character I/O changes for DEVICE support and a fix
to the RAM disk size (Z280RC only has 2Mb memory)

I've also uploaded a zip file containing updated versions of CP/M-Plus
binaries that have been patched for Y2K and other updates.

I am working on a CP/M-Plus Banked Memory BIOS for the Z280RC and this
will be added soon.

