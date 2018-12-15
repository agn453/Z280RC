#include <stdio.h>

#define FALSE   0
#define TRUE    1

/* convert integer to string in base (2-36) */
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
	if (base == -10)	/* set signed */
		base = 10;
	p = q = s;
	do {			/* generate digits in reverse order */
		if ((*p = u % base + '0') > '9')
			*p += ('A' - ('9' + 1));
		++p;
		} while ((u /= base) > 0);
	if (negative)
		*p++ = '-';
	*p = '\0';		/* terminate string */
	while (q < --p) {	/* reverse the digits */
		c = *q;
		*q++ = *p;
		*p = c;
		}
	return s;
	}
