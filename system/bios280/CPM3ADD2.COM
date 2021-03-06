��
Some useful CP/M-Plus addresses and register contents
CPM+ADD v.2 15-Dec-85 Jerry Levy

Generic CP/M 3.x 

Control-S stops scrolling, Control-Q resumes.
Press any other key to continue...
$͢� �28���{�#͗�͢�H�* +++�������! ����#�v����###~����! ����* 9 #�v~�!�>#�vQ��* ? #�v~�!�Y#�v���* N #�v~�2�u#�v�����^#V��>"20�'���>$20�'���>&20�'��>(20�'6��>*20�'Z��>:20�'2"���>X20�'���>Z20�'���>[20�'���>\20�',��>]20�'��>b20�'$���10� �       2@� �        (͢&. "B*B}�ʪ}�A2T2�>	2@�7"��*B#"B᯼��U͗S͢ �)�͢*� �v��U͗"͢*�d �)͗�      ����|��}������������Ɛ'�@'��� ��:8� �������:8� ��͊���"�>�29 PY�:���!:����>�28�v~�kz��Z{��]#�*�#"�����Z���v�7> 28͗��29��> ͊�����:9� ʂ*�͗�������_� ����>͊>
͊���	� ���            Misc. addresses
BIOS Base          = cboot jmp = bios jmp table address
COMMON Base        
@MXTPA             = top of TPA = bdos_base
@CTBL        XXXX  = device table, if implemented
@DTBL              = drive tbl (FFFF->none, hashing supported)
                               (FFFE->none, no hashing)
@CBNK              = storage of current_memory_bank
SCB_BASE+0H        = at SCB_BASE+3AH, address of System Control Block

            Selected Register contents
I/O Redirection Flags (16-bit words):
  for CONIN        at SCB_BASE+22H
  for CONOUT       at SCB_BASE+24H
  for AUXIN        at SCB_BASE+26H
  for AUXOUT       at SCB_BASE+28H
  for LSTOUT       at SCB_BASE+2AH

@DATE              at SCB_BASE+58H, 16-bit integer, days since 01-Jan-78
@HOUR              at SCB_BASE+5AH, BCD hours
@MIN               at SCB_BASE+5BH, BCD minutes
@SECOND            at SCB_BASE+5CH, BCD seconds

           CP/M-Plus BIOS addresses
Jump Table Entries and Subroutines
            Jump   Sub-   
             Table  routine
            ------ --------
 0 CBOOT                 
 1 WBOOT                 
 2 CONST                 
 3 CONIN                 
 4 CONOUT                
 5 LIST                  
 6 AUXOUT                
 7 AUXIN                 
 8 HOME                  
 9 SELDSK                
10 SETTRK                
11 SETSEC                
12 SETDMA                
13 READ                  
14 WRITE                 
15 LISTST                
16 SECTRN                
17 CONOST                
18 AUXIST                
19 AUXOST                
20 DEVTBL                
21 DEVINI                
22 DRVTBL                
23 MULTIO                
24 FLUSH                 
25 MOVE                  
26 TIME                  
27 SELMEM                
28 SETBNK                
29 XMOVE                 
30 USERF                 
31 RESERV1               
32 RESERV2               
             $           
             $           
             $           
             $           
             $           
             $           
             $           
$
        DPH's and DPB's for valid drives$  : drive -- Disk Parameter Header
     -xlt- ------72 bit scratch------ mf -dfb- -csv- -alv- -dir- -dta- -hsh- hb$  : drive -- Disk Parameter Block
     -spt- bs bm em -dsm- -drm- a0 a1 -cks- -off- ps pm$ System Control Block$                   K