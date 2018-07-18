# Z280RC
Software for Bill Shen's Z280RC Single Board Z280 system on a RC2014 board

This repository contains a snapshot of the original software, plus
my versions of modified source files and new utilities for Bill Shen's
Z280RC single board Z280 system - as described on the RetroBrew builders
wiki at

  https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:z280rc

The original software is located in zip files in the "original" subdirectory.

My own modifications to the software are in the "system" subdirectory and
below.

New Utilities are in the "utilities" subdirectory.

As at 18-Jul-2018, only an updated Non-Banked CBIOS and a CP/M utility
to support the on-board clock calendar timekeeping chip (TIME.MAC) have
been added.

The new CBIOS adds support for the DS1302 timekeeper to Non-Banked
CP/M-Plus as well as character I/O changes for DEVICE support and a fix
to the RAM disk size (Z280RC only has 2Mb memory)

I've also uploaded a zip file containing updated versions of CP/M-Plus
binaries that have been patched for Y2K and other updates.

I am working on a CP/M-Plus Banked Memory BIOS for the Z280RC and this
will be added soon.

