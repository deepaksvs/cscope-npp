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

/*	cscope - interactive C symbol or text cross-reference
 *
 *	command functions
 */

#include "global.h"
#include "build.h"		/* for rebuild() */
#include "alloc.h"

#include <stdlib.h>
#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
//#include <curses.h>
#endif
#include <ctype.h>
#include <io.h>

static char const rcsid[] = "$Id: command.c,v 1.32 2006/08/20 15:00:33 broeker Exp $";


int	selecting;
unsigned int   curdispline = 0;

BOOL	*change;		/* change this line */
BOOL	changing;		/* changing text */

/* HBB FIXME 20060419: these should almost certainly be const */
static	char	appendprompt[] = "Append to file: ";
static	char	pipeprompt[] = "Pipe to shell command: ";
static	char	readprompt[] = "Read from file: ";
static	char	toprompt[] = "To: ";


/* Internal prototypes: */
static	BOOL	changestring(void);
static	void	clearprompt(void);
static	void	mark(unsigned int i);
static	void	scrollbar(MOUSE *p);

/* execute the command */
BOOL
command(int commandc)
{
	field = commandc;

	if (mygetline("", newpat, 0, commandc, caseless) > 0) {
		strcpy(Pattern, newpat);
		if (search() == YES) {
		    curdispline = 0;
		    ++selecting;

		    switch (field) {
		    case DEFINITION:
		    case FILENAME:
			if (totallines > 1) {
			    break;
			}
			topline = 1;
			editref(0);
			break;
		    case CHANGE:
			return(changestring());
		    }

		} else if (field == FILENAME && 
			   _access(newpat, READ) == 0) {
		    /* try to edit the file anyway */
		    edit(newpat, "1");
		}
	}
	return YES;
}
/* clear the prompt line */

static void
clearprompt(void)
{
	//move(PRLINE, 0);
	//clrtoeol();
}

/* read references from a file */

BOOL
readrefs(char *filename)
{
	FILE	*file;
	int	c;

	if ((file = myfopen(filename, "rb")) == NULL) {
		cannotopen(filename);
		return(NO);
	}
	if ((c = getc(file)) == EOF) {	/* if file is empty */
		return(NO);
	}
	totallines = 0;
	disprefs = 0;
	nextline = 1;
	if (writerefsfound() == YES) {
		putc(c, refsfound);
		while ((c = getc(file)) != EOF) {
			putc(c, refsfound);
		}
		fclose(file);
		fclose(refsfound);
		if ( (refsfound = myfopen(temp1, "rb")) == NULL) {
			cannotopen(temp1);
			return(NO);
		}
		countrefs();
	}
	return(YES);
}

/* change one text string to another */

static BOOL
changestring(void)
{
    return NO;
#if 0
    char    newfile[PATHLEN + 1];   /* new file name */
    char    oldfile[PATHLEN + 1];   /* old file name */
    char    linenum[NUMLEN + 1];    /* file line number */
    char    msg[MSGLEN + 1];        /* message */
    FILE    *script;                /* shell script file */
    BOOL    anymarked = NO;         /* any line marked */
    MOUSE *p;                       /* mouse data */
    int     c;
    unsigned int i;
    char    *s;

    /* open the temporary file */
    if ((script = myfopen(temp2, "w")) == NULL) {
	cannotopen(temp2);
	return(NO);
    }
    /* create the line change indicators */
    change = mycalloc(totallines, sizeof(BOOL));
    changing = YES;
    mousemenu();

    /* until the quit command is entered */
    for (;;) {
	/* display the current page of lines */
	display();
    same:
	atchange();
		
	/* get a character from the terminal */
	if ((c = mygetch()) == EOF
	    || c == ctrl('D') 
	    || c == ctrl('Z')) {
	    break;	/* change lines */
	}
	/* see if the input character is a command */
	switch (c) {
	case ' ':	/* display next page */
	case '+':
	case ctrl('V'):
#ifdef KEY_NPAGE
	case KEY_NPAGE:
#endif
	case '-':	/* display previous page */
#ifdef KEY_PPAGE
	case KEY_PPAGE:
#endif
	case '!':	/* shell escape */
	case '?':	/* help */
	    command(c);
	    break;

	case ctrl('L'):	/* redraw screen */
#ifdef KEY_CLEAR
	case KEY_CLEAR:
#endif
	    command(c);
	    goto same;

	case ESC:	/* don't change lines */
#if UNIXPC
	    if((p = getmouseaction(DUMMYCHAR)) == NULL) {
		goto nochange;	/* unknown escape sequence */
	    }
	    break;
#endif
	case ctrl('G'):
	    goto nochange;

	case '*':	/* mark/unmark all displayed lines */
	    for (i = 0; topline + i < nextline; ++i) {
		mark(i);
	    }
	    goto same;

	case ctrl('A'):	/* mark/unmark all lines */
	    for (i = 0; i < totallines; ++i) {
		if (change[i] == NO) {
		    change[i] = YES;
		} else {
		    change[i] = NO;
		}
	    }
	    /* show that all have been marked */
	    seekline(totallines);
	    break;

	case ctrl('X'):	/* mouse selection */
	    if ((p = getmouseaction(DUMMYCHAR)) == NULL) {
		goto same;	/* unknown control sequence */
	    }
	    /* if the button number is a scrollbar tag */
	    if (p->button == '0') {
		scrollbar(p);
		break;
	    }
	    /* find the selected line */
	    /* note: the selection is forced into range */
	    for (i = disprefs - 1; i > 0; --i) {
		if (p->y1 >= displine[i]) {
		    break;
		}
	    }
	    mark(i);
	    goto same;

	default:
	    {
		/* if a line was selected */
		char		*cc;

		if ((cc = strchr(dispchars, c)))
		    mark(cc - dispchars);

		goto same;
	    } /* default case */
	} /* switch(change code character) */
    } /* for(ever) */

    /* for each line containing the old text */
    fprintf(script, "ed - <<\\!\n");
    *oldfile = '\0';
    seekline(1);
    for (i = 0; 
	 fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s%*[^\n]", newfile, linenum) == 2;
	 ++i) {
	/* see if the line is to be changed */
	if (change[i] == YES) {
	    anymarked = YES;
		
	    /* if this is a new file */
	    if (strcmp(newfile, oldfile) != 0) {
				
		/* make sure it can be changed */
		if (access(newfile, WRITE) != 0) {
		    sprintf(msg, "Cannot write to file %s", newfile);
		    postmsg(msg);
		    anymarked = NO;
		    break;
		}
		/* if there was an old file */
		if (*oldfile != '\0') {
		    fprintf(script, "w\n");	/* save it */
		}
		/* edit the new file */
		strcpy(oldfile, newfile);
		fprintf(script, "e %s\n", oldfile);
	    }
	    /* output substitute command */
	    fprintf(script, "%ss/", linenum);	/* change */
	    for (s = Pattern; *s != '\0'; ++s) {
		/* old text */
		if (strchr("/\\[.^*", *s) != NULL) {
		    putc('\\', script);
		}
		if (caseless == YES && isalpha((unsigned char)*s)) {
		    putc('[', script);
		    if(islower((unsigned char)*s)) {
			putc(toupper((unsigned char)*s), script);
			putc(*s, script);
		    } else {
			putc(*s, script);
			putc(tolower((unsigned char)*s), script);
		    }
		    putc(']', script);
		} else	
		    putc(*s, script);
	    }
	    putc('/', script);			/* to */
	    for (s = newpat; *s != '\0'; ++s) {	/* new text */
		if (strchr("/\\&", *s) != NULL) {
		    putc('\\', script);
		}
		putc(*s, script);
	    }
	    fprintf(script, "/gp\n");	/* and print */
	}
    }
    fprintf(script, "w\nq\n!\n");	/* write and quit */
    fclose(script);

    /* if any line was marked */
    if (anymarked == YES) {
		
	/* edit the files */
	clearprompt();
	//refresh();
	fprintf(stderr, "Changed lines:\n\r");
	execute("sh", "sh", temp2, NULL);
	askforreturn();
	seekline(1);
    } else {
    nochange:
	clearprompt();
    }
    changing = NO;
    mousemenu();
    free(change);
    return(anymarked);
#endif
}


/* mark/unmark this displayed line to be changed */
static void
mark(unsigned int i)
{
    return;
#if 0
    unsigned int j;
	
    j = i + topline - 1;
    if (j < totallines) {
	move(displine[i], 1);

	if (change[j] == NO) {
	    change[j] = YES;
	    addch('>');
	} else {
	    change[j] = NO;
	    addch(' ');
	}
    }
#endif
}


/* scrollbar actions */
static void
scrollbar(MOUSE *p)
{
    return;
#if 0
    /* reposition list if it makes sense */
    if (totallines == 0) {
	return;
    }
    switch (p->percent) {
		
    case 101: /* scroll down one page */
	if (nextline + mdisprefs > totallines) {
	    nextline = totallines - mdisprefs + 1;
	}
	break;
		
    case 102: /* scroll up one page */
	nextline = topline - mdisprefs;
	if (nextline < 1) {
	    nextline = 1;
	}
	break;

    case 103: /* scroll down one line */
	nextline = topline + 1;
	break;
		
    case 104: /* scroll up one line */
	if (topline > 1) {
	    nextline = topline - 1;
	}
	break;
    default:
	nextline = p->percent * totallines / 100;
    }
    seekline(nextline);
#endif
}
