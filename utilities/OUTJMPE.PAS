program Out_Jump_Check;

(* 300391 Tilmann Reh *)

const signon = ^m^j'OUT/JMP-Tester V1.0  TR 300391'^m^j;

(* Comments translated to English by Google Translate and Tony Nicholson *)

(* This program searches for command sequences in CP/M programs (.COM)
from an output command and a 'program flow change' command (eg CALL/JMP).

According to Errata Sheet to Z280 errors may occur when after a
'external write with wait states' executes a command that returns the
Pipeline of the processor is deleted. For the CPU280, the former is only at
I/O Write (i.e., OUT) possible. Except for CALL and JMP the pipeline will be
also deleted with PCACHE and LDCTL, whereby LDCTL is privileged and
therefore, it does not need to be checked here (only occurs in the BIOS).
Return-Commands are uncritical with an external stack (in general with
the CPU280).

The program OUTJMP examines files for the occurrence of such 'forbidden'
Sequences and displays these as well as their address location. To be examined
only programs with direct I/O control, ie  programming software for EPROMs/
PALs and the like as well as communication programs.  Not all the detected
sequences represent possible problems (for example, data fields may have
been misinterpreted).

If sequences are found, it must first be checked if they are correct
were interpreted. If so, the program should be that way
be patched that between the two commands at least one
'normal' command, preferably an IN command is executed. In the
In most cases, this will only be done by offloading the sequence into
a mini-library subroutine. *)

(* Tables of all conceivable OUT commands (except D3xx) *)

type  bs       = set of byte;
const out_ed   : bs = [$41,$49,$51,$59,$61,$69,$79,$83,
                       $8B,$93,$9B,$A3,$AB,$B3,$BB,$BF];
      out_dd   : bs = [$49,$51,$59,$61,$69];
      out_dd_n : bs = [$41,$79];
      out_fd   : bs = [$61,$69];
      out_fd_n : bs = [$41,$49,$51,$59];

(* Table of all possible CALL/JUMP-commands (without E9) *)

      calljump : bs = [$CD,$C4,$CC,$D4,$DC,$E4,$EC,$F4,$FC,
                       $C3,$C2,$CA,$D2,$DA,$E2,$EA,$F2,$FA];
      cj_dd    : bs = [$CD,$C4,$CC,$D4,$DC,$E4,$EC,$F4,$FC,
                       $C2,$CA,$D2,$DA,$E2,$EA,$F2,$FA];
      jumprel  : bs = [$10,$18,$20,$28,$30,$38];
      jafjar   : bs = [$20,$28];

(* Variable Definitions *)

      start    : integer = $100;  (* Start address *)
      ende     : integer = 1023;  (* Last Byte to be examined *)

(* Program Variables *)

var filnam     : string[20];
    f          : file;
    buf        : array[0..1151] of byte;  (* 9 records *)
    i,j,secs   : integer;

(* Hexadecimal output of Bytes and Words *)

procedure hex_byte(b:byte);
const nyb : array[0..15] of char = '0123456789ABCDEF';
begin
  write(nyb[b shr 4],nyb[b and 15]);
  end;

procedure hex_word(w:integer);
begin hex_byte(hi(w)); hex_byte(lo(w)); end;

(* ALARM: Show address and object code sequence *)

procedure alarm(laenge:integer);
var x : integer;
begin
  if eof(f) and (pred(j+laenge)>ende) then
    if j<=ende then write('started ') else exit;
  write('Sequence found at '); hex_word(i+start);
  write(' :');
  for x:=i to pred(j) do begin write(' '); hex_byte(buf[x]); end;
  write(' -');
  for x:=j to pred(j+laenge) do begin write(' '); hex_byte(buf[x]); end;
  writeln;
  end;

(*--------- MAIN ----------*)

begin
  writeln(signon);
  if paramcount=0 then begin
    writeln('Usage: OUTJMP filename (Default Extension .COM)');
    halt; end;
  filnam:=paramstr(1);
  i:=pos('.',filnam); if i=0 then filnam:=filnam+'.COM';
  assign(f,filnam);
  {$I-} reset(f); {$I+}
  if ioresult<>0 then begin
    writeln('Datei ',filnam,' cannot be opened!');
    halt; end;

  blockread(f,buf,9,secs);
  if secs<8 then ende:=pred(secs shl 7);
  repeat
    for i:=0 to ende do begin
      j:=-1;
      if buf[i]=$D3 then j:=i+2 else
      if (buf[i]=$ED) and (buf[succ(i)] in out_ed) then j:=i+2 else
      if (buf[i]=$DD) and (buf[succ(i)]=$ED) then begin
        if (buf[i+2] in out_dd) then j:=i+3 else
        if (buf[i+2] in out_dd_n) then j:=i+5 end else
      if (buf[i]=$FD) and (buf[succ(i)]=$ED) then begin
        if (buf[i+2] in out_fd) then j:=i+3 else
        if (buf[i+2] in out_fd_n) then j:=i+5 end;
      if j>=0 then begin
        if buf[j] in calljump then alarm(3) else
        if buf[j] in jumprel then alarm(2) else
        if buf[j]=$E9 then alarm(1) else
        if (buf[j] and $DF=$DD) and (buf[succ(j)]=$E9) then alarm(2) else
        if (buf[j]=$FD) and (buf[succ(j)] in calljump) then alarm(4) else
        if (buf[j]=$DD) then begin
          if buf[succ(j)] in cj_dd then alarm(2) else
          if buf[succ(j)] in jafjar then alarm(3) end else
        if (buf[j]=$ED) and (buf[succ(j)]=$65) then alarm(2);
        end;
      end;
    move(buf[1024],buf[0],128);
    blockread(f,buf[128],8,secs);
    if secs<7 then ende:=127+(secs shl 7) else ende:=1023;
    start:=start+1024;
  until ende<128;
  end.
