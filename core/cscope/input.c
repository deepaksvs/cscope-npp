/*===========================================================================
 Copyright (c) 1998-2000, The Santa Cruz Operation 
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 *Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 *Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 *Neither name of The Santa Cruz Operation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 DAMAGE. 
 =========================================================================*/

/*	cscope - interactive C symbol cross-reference
 *
 *	terminal input functions
 */

#include "global.h"
#include <setjmp.h>	/* jmp_buf */
#include <stdlib.h>
#include <errno.h>

#if defined(HAVE_NCURSES)
#include <ncurses.h>
#elif defined(HAVE_CURSES) 
#include <curses.h>
#endif /* defined(HAVE_NCURSES) */

#if HAVE_SYS_TERMIOS_H
#include <sys/termios.h>
#endif

static char const rcsid[] = "$Id: input.c,v 1.15 2006/08/20 15:00:34 broeker Exp $";
static int prevchar = 0;
static char line[PATHLEN + 1];

// deepak
void
myungetch(int c)
{
	prevchar = c;
	return;
}

/* get a character from the terminal */
int
mygetch(void)
{
    int c = '\n';

    return(c);
}

/* get a line from the terminal in non-canonical mode */
int
mygetline(char p[], char s[], unsigned size, int firstchar, BOOL iscaseless)
{
	int i;

	strcpy(s, p);
	i = strlen(p);

	strcat(s, line);
	i += strlen(line);

    return i;
}

int mysetline (const char *str)
{
	strncpy(line, str, PATHLEN);
	return 1;
}

/* ask user to press the RETURN key after reading the message */

void
askforchar(void)
{
    mygetch();
}

void
askforreturn(void)
{
    fprintf(stderr, "Press the RETURN key to continue: ");
    getchar();
#if defined(WITH_CURSES)
    /* HBB 20060419: message probably messed up the screen --- redraw */
    if (incurses == YES) {
	redrawwin(curscr);
    }
#endif
}
