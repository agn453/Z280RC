
/*-
; * All UZI280 source code is  
; * Copyright (c) (1990-95) by Stefan Nitschke and Doug Braun
; *
; * Permission is hereby granted, free of charge, to any person
; * obtaining a copy of this software and associated documentation
; * files (the "Software"), to deal in the Software without
; * restriction, including without limitation the rights to use,
; * copy, modify, merge, publish, distribute, sublicense, and/or
; * sell copies of the Software, and to permit persons to whom
; * the Software is furnished to do so, subject to the following
; * conditions:
; * 
; * The above copyright notice and this permission notice shall be
; * included in all copies or substantial portions of the Software.
; * 
; * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
; * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
; * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
; * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
; * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; * DEALINGS IN THE SOFTWARE.
; */



/* Short version of printf to save space */

static char *x;

kprintf(nargs, n)
char *nargs;
int n;
{
	doprnt(nargs, &n);
}

doprnt(nargs, a)
char *nargs;
int *a;
{
	register char *fmt;
	register c, base;
	char s[7], *itob();

	fmt = nargs;
	while (c = *fmt++) {
		if (c != '%') {
			kputchar(c);
			continue;
		}
		switch (c = *fmt++) {
		case 'c':
			kputchar(*a++);
			continue;
		case 'd':
			base = -10;
			goto prt;
		case 'o':
			base = 8;
			goto prt;
		case 'u':
			base = 10;
			goto prt;
		case 'x':
			base = 16;
		prt:
			puts(itob(*(int *)a, s, base));
			a += sizeof(char *)/ sizeof a; 
			continue;
		case 's':
			x = *(char **)a;
			puts(x);
			a += sizeof(char *)/ sizeof a; 
			continue;
		default:
			kputchar(c);
			continue;
		}
	}
}
