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

12-Nov-2018
-----------

Added USE$DMA$TO$COPY configuration equate.  If set to FALSE then extended
memory moves using the BIOS MOVE routine will use the memory management unit
to copy data from one bank to another.  Using this setting I confirmed
that the DMA transfers using the Z280's on-chip controllers were dodgy!
I changed the DMAXFR routine in the BIOSKRNL module to in-line program
the DMA channel without looping.

The Banked memory versionusing just bank 0 and 1 (CPM3BNK1.SYS) now seems
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

