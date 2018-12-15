
/*
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

/* itob.c */

#define TRUE 1
#define FALSE 0

/* convert an integer to a string in any base (2-36) */
char *itob(n, s, base)
char *s;
	{
	register unsigned int u;
	register char *p, *q;
	register negative, c;

	if (n < 0 && base == -10) {
		negative = TRUE;
		u = -n;
		}
	else {
		negative = FALSE;
		u = n;
		}
	if (base == -10)	/* signals signed conversion */
		base = 10;
	p = q = s;
	do {			/* generate digits in reverse order */
		if ((*p = u % base + '0') > '9')
			*p += ('A' - ('9' + 1));
		++p;
		} while ((u /= base) > 0);
	if (negative)
		*p++ = '-';
	*p = '\0';		/* terminate the string */
	while (q < --p) {	/* reverse the digits */
		c = *q;
		*q++ = *p;
		*p = c;
		}
	return s;
	}

