# Z280RC
Software for Bill Shen's Z280RC Single Board Z280 system on a RC2014 board

This repository contains a snapshot of the original software, plus
my versions of modified source files and new utilities for Bill Shen's
Z280RC single board Z280 system - as described on the RetroBrew builders
wiki at

  https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:z280rc

The original software is located in zip files in the "original" subdirectory.

My own modifications to Bill Shen's original CBIOS3 for CP/M 3 are in the
"system" subdirectory and below.

New Utilities are in the "utilities" subdirectory.

Modification History (in reverse chronological order):
======================================================

15-Nov-2018
-----------

I was running out of space/directory entries so I built a version of
Banked CP/M-Plus with eight 8MB CompactFlash drives.  See the CONFBNK8.LIB,
BANKED8.DAT, CPM3BNK8.SYS and BNK8BIOS.SUB files in the system/bios280
directory.

Depending on the actual size of CompactFlash you have, play it safe
and use at least a 128MB one if you use this (64MB is not the same as 64MiB).

Drives E: thru H: are configured with 1024 directory entries each.  A
new configuration option USE$BIG$DRM enables 1024 directory entries
(512 is the default for all other drives).


15-Nov-2018
-----------

Hector Peraza's latest beta 11 of ZSM4 - a Z80/Z180/Z280 Macro-Assembler
available at
https://www.retrobrewcomputers.org/forum/index.php?t=msg&th=93&goto=3700&#msg_3700
found a typo in IDEHD which I fixed.  Also went through and commented/
removed superfluous lines from the BIOS sources.  No object code changes -
just cosmetic!


14-Nov-2018
-----------

Added a modified version of XMODEM 2.7 (for file transfer via the console
serial port).  This now includes routines to drive the built-in Z280
UART directly.  To select the internal routines use the /X3 switch on
the XMODEM command line (or make it the default by having the XMZ280RC.CFG
in the default directory). You'll find these files in the utilities folder
or you can download a ZIP containing them directly from 
https://github.com/agn453/Z280RC/blob/master/utilities/xm27z280.zip


13-Nov-2018
-----------

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


12-Nov-2018
-----------

Added USE$DMA$TO$COPY configuration equate.  If set to FALSE then extended
memory moves using the BIOS MOVE routine will use the memory management unit
to copy data from one bank to another.  Using this setting I confirmed
that the DMA transfers using the Z280's on-chip controllers were dodgy!
I changed the DMAXFR routine in the BIOSKRNL module to in-line program
the DMA channel without looping.

The Banked memory version using just bank 0 and 1 (CPM3BNK1.SYS) now seems
to be working.


11-Nov-2018
-----------

Added DEBUG capability to enter the debugger upon CTRL-P being typed
at the console.  Also made a special banked configuration that supports
only two banks (bank 0 and 1) with directory hashing disabled and only
a single directory and data buffer for each drive in bank 0.  The
resulting CPM3BNK1.SYS appears to be working (and I'm now convinced
there's a problem with CP/M 3 PIP corrupting buffers).

Also added CPM3ADD2.MAC which (when assembled and linked) will show
information about the running system's BIOS, SCB and disk drive data
(DPH and DPB).


08-Nov-2018
-----------

Updated disk modules to use hard-coded disk parameter headers and disk
parameter blocks with internal disk allocation vectors (rather than
letting GENCPM set these up).  This is selectable via a USE$DISK$MACROS
conditional equate in the CONF*.LIB files.  Unfortunately, this had no
effect on the PIP file copying verification error under the banked
system.  Non-banked and Loader configurations work fine!

 
06-Nov-2018
-----------

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

18-Jul-2018
-----------

Only an updated Non-Banked CBIOS3 and a CP/M utility to support the on-board
clock calendar timekeeping chip (TIME.MAC) have been added.
The new CBIOS3 adds support for the DS1302 timekeeper to Non-Banked
CP/M-Plus as well as character I/O changes for DEVICE support and a fix
to the RAM disk size (Z280RC only has 2Mb memory)

I've also uploaded a zip file containing updated versions of CP/M-Plus
binaries that have been patched for Y2K and other updates.

I am working on a CP/M-Plus Banked Memory BIOS for the Z280RC and this
will be added soon.

