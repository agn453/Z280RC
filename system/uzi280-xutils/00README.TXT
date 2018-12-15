This archive contains the basic files for UZI280 v 1.12, necessary for
a minimal bootable system (the files are somewhat different to older versions).

The MKBOOT.SUB script will install a UZI280 file system on four hard disc with
a default size of 32MB (edit the first line to change the partion size: 
                                 mkfs partion inode_number block_number)
, sets up the entries in /dev and copies the basic files to your 
uzi280 partition. 

Use the COPYALL.SUB script if you already have a UZI280 file system. This 
script performs just a file copy and installs the default passwd and ttytab to
let you login as root (no password).
  
You need the UZI280-CPM cross utilities "ucp" and "mkfs" setup for your system.
 
They are part of the UZI280 kernel archive.
