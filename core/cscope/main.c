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
 *	main functions
 */
#include "global.h"

#include "build.h"
#include "vp.h"
#include "version.h"	/* FILEVERSION and FIXVERSION */
#include "scanner.h" 
#include "alloc.h"

#if defined(USE_SORTLIB)
#include "sort.h"
#endif /* defined(USE_SORTLIB) */

#include <stdlib.h>	/* atoi */
#include <sys/types.h>	/* needed by stat.h */
#include <sys/stat.h>	/* stat */
#include <signal.h>
#include <process.h>
#include <direct.h>
#include <pluginMain.h>
#include <io.h>
#include <util.h>

#if defined(HAVE_NCURSES)
#include <ncurses.h>
#elif defined(HAVE_CURSES) 
#include <curses.h>
#endif /* defined(HAVE_NCURSES) */

/* defaults for unset environment variables */
#define	EDITOR	"notepad++"
#define HOME	"."	/* no $HOME --> use root directory */
#define	SHELL	"cmd"
#define LINEFLAG "+%s"	/* default: used by vi and emacs */
#define TMPDIR	"."
#ifndef DFLT_INCDIR
#define DFLT_INCDIR "/usr/include"
#endif

static char const rcsid[] = "$Id: main.c,v 1.45 2008/04/11 11:23:55 nhorman Exp $";

/* note: these digraph character frequencies were calculated from possible 
   printable digraphs in the cross-reference for the C compiler */
char	dichar1[] = " teisaprnl(of)=c";	/* 16 most frequent first chars */
char	dichar2[] = " tnerpla";		/* 8 most frequent second chars 
					   using the above as first chars */
char	dicode1[256];		/* digraph first character code */
char	dicode2[256];		/* digraph second character code */

char	*editor, *shell, *lineflag;	/* environment variables */
char	*home;			/* Home directory */
BOOL	lineflagafterfile;
char	*argv0;			/* command name */
BOOL	compress = YES;		/* compress the characters in the crossref */
BOOL	dbtruncated;		/* database symbols are truncated to 8 chars */
int	dispcomponents = 1;	/* file path components to display */
#if CCS
BOOL	displayversion;		/* display the C Compilation System version */
#endif
BOOL	editallprompt = YES;	/* prompt between editing files */
unsigned int fileargc;		/* file argument count */
char	**fileargv;		/* file argument values */
int	fileversion;		/* cross-reference file version */
BOOL	incurses = NO;		/* in curses */
BOOL	invertedindex;		/* the database has an inverted index */
BOOL	isuptodate;		/* consider the crossref up-to-date */
BOOL	kernelmode;		/* don't use DFLT_INCDIR - bad for kernels */
BOOL	linemode = NO;		/* use line oriented user interface */
BOOL	verbosemode = NO;	/* print extra information on line mode */
BOOL	recurse_dir = NO;	/* recurse dirs when searching for src files */
char	*namefile;		/* file of file names */
BOOL	ogs;			/* display OGS book and subsystem names */
char	*prependpath;		/* prepend path to file names */
FILE	*refsfound;		/* references found file */
char	temp1[PATHLEN + 1];	/* temporary file name */
char	temp2[PATHLEN + 1];	/* temporary file name */
char	tempdirpv[PATHLEN + 1];	/* private temp directory */
long	totalterms;		/* total inverted index terms */
BOOL	trun_syms;		/* truncate symbols to 8 characters */
char	tempstring[TEMPSTRING_LEN + 1]; /* use this as a buffer, instead of 'yytext', 
				 * which had better be left alone */
char	*tmpdir;		/* temporary directory */
/* HBB 20040430: renamed to avoid lots of clashes with function arguments
 * also named 'pattern' */
char	Pattern[PATLEN + 1];	/* symbol or text pattern */
char	newpat[PATLEN + 1];	/* new pattern */
BOOL	caseless;		/* ignore letter case when searching */

crossRefs	refInfo[MAX_CROSS_REFS];

static	BOOL	onesearch;		/* one search only in line mode */
static	char	*reflines;		/* symbol reference lines file */

/* Internal prototypes: */
static	void	initcompress(void);
static	void	longusage(void);
static	void	skiplist(FILE *oldrefs);
static	void	usage(void);

#ifdef HAVE_FIXKEYPAD
void	fixkeypad();
#endif

#if defined(KEY_RESIZE) && !defined(__DJGPP__)
void 
sigwinch_handler(int sig, siginfo_t *info, void *unused)
{
    (void) sig;
    (void) info;
    (void) unused;
    if(incurses == YES)
        ungetch(KEY_RESIZE);
}
#endif

int
myinit (const char *reffileabs)
{
    struct stat	stat_buf;
    FILE *oldrefs;
    FILE *names;
    int c, oldnum;
    char *s;
    unsigned int i;
    char path[PATHLEN + 1];
    int pid;
    
    yyin = stdin;
    yyout = stdout;

    pid = _getpid();
    /* Consider cross-reference is upto date = -d option */
    isuptodate = YES;
    if (reffileabs) {
        reffile = reffileabs;
    }

    home = mygetenv("HOME", ".");
    tmpdir = mygetenv("TMP", TMPDIR);
    if (_stat (tmpdir, &stat_buf)) {
        /* tmp dir was not correct .. so create here only */
        sprintf(tempdirpv, "");
    }
    else {
        sprintf(tempdirpv, "%s", tmpdir);
    }

	sprintf(temp1, "%s\\cscope.%d.1", tempdirpv, pid);
    sprintf(temp2, "%s\\cscope.%d.2", tempdirpv, pid);

    if ((oldrefs = vpfopen(reffile, "rb")) == NULL) {
	    postfatal("cscope: cannot open file %s\n", reffile);
	    /* NOTREACHED */
	}
	/* get the crossref file version but skip the current directory */
	if (fscanf(oldrefs, "cscope %d %*s", &fileversion) != 1) {
	    postfatal("cscope: cannot read file version from file %s\n", 
		      reffile);
	    /* NOTREACHED */
	}
	if (fileversion >= 8) {

	    /* override these command line options */
	    compress = YES;
	    invertedindex = NO;

	    /* see if there are options in the database */
	    for (;;) {
		getc(oldrefs);	/* skip the blank */
		if ((c = getc(oldrefs)) != '-') {
		    ungetc(c, oldrefs);
		    break;
		}
		switch (c = getc(oldrefs)) {
		case 'c':	/* ASCII characters only */
		    compress = NO;
		    break;
		case 'q':	/* quick search */
		    invertedindex = YES;
		    fscanf(oldrefs, "%ld", &totalterms);
		    break;
		case 'T':	/* truncate symbols to 8 characters */
		    dbtruncated = YES;
		    trun_syms = YES;
		    break;
		}
	    }
	    initcompress();
	    seek_to_trailer(oldrefs);
	}
	/* skip the source and include directory lists */
	skiplist(oldrefs);
	skiplist(oldrefs);

	/* get the number of source files */
	if (fscanf(oldrefs, "%lu", &nsrcfiles) != 1) {
	    postfatal("\
cscope: cannot read source file size from file %s\n", reffile);
	    /* NOTREACHED */
	}
	/* get the source file list */
	srcfiles = (char **) mymalloc(nsrcfiles * sizeof(char *));
	if (fileversion >= 9) {

	    /* allocate the string space */
	    if (fscanf(oldrefs, "%d", &oldnum) != 1) {
		postfatal("\
cscope: cannot read string space size from file %s\n", reffile);
		/* NOTREACHED */
	    }
	    s = (char *)mymalloc(oldnum);
	    getc(oldrefs);	/* skip the newline */
			
	    /* read the strings */
	    if (fread(s, oldnum, 1, oldrefs) != 1) {
		postfatal("\
cscope: cannot read source file names from file %s\n", reffile);
		/* NOTREACHED */
	    }
	    /* change newlines to nulls */
	    for (i = 0; i < nsrcfiles; ++i) {
		srcfiles[i] = s;
		for (++s; *s != '\n'; ++s) {
		    ;
		}
		*s = '\0';
		++s;
	    }
	    /* if there is a file of source file names */
	    if ((namefile != NULL && (names = vpfopen(namefile, "r")) != NULL)
		|| (names = vpfopen(NAMEFILE, "r")) != NULL) {
	
		/* read any -p option from it */
		while (fgets(path, sizeof(path), names) != NULL && *path == '-') {
		    i = path[1];
		    s = path + 2;		/* for "-Ipath" */
		    if (*s == '\0') {	/* if "-I path" */
			fgets(path, sizeof(path), names);
			s = path;
		    }
		    switch (i) {
		    case 'p':	/* file path components to display */
			if (*s < '0' || *s > '9') {
			    posterr("cscope: -p option in file %s: missing or invalid numeric value\n", 								namefile);

			}
			dispcomponents = atoi(s);
		    }
		}
		fclose(names);
	    }
	} else {
	    for (i = 0; i < nsrcfiles; ++i) {
		if (!fgets(path, sizeof(path), oldrefs) ) {
		    postfatal("\
cscope: cannot read source file name from file %s\n", 
			      reffile);
		    /* NOTREACHED */
		}
		srcfiles[i] = my_strdup(path);
	    }
	}
	fclose(oldrefs);
    opendatabase();
    return 0;
}

void
cannotopen(char *file)
{
    posterr("Cannot open file %s", file);
}

/* FIXME MTE - should use postfatal here */
void
cannotwrite(char *file)
{
#if HAVE_SNPRINTF
    char	msg[MSGLEN + 1];

    snprintf(msg, sizeof(msg), "Removed file %s because write failed", file);
#else
    char *msg = (char *)mymalloc(50 + strlen(file));

    sprintf(msg, "Removed file %s because write failed", file);
#endif

    myperror(msg);	/* display the reason */

#if !HAVE_SNPRINTF
    free(msg);
#endif

    _unlink(file);
    myexit(1);	/* calls exit(2), which closes files */
}


/* set up the digraph character tables for text compression */
static void
initcompress(void)
{
    int	i;
	
    if (compress == YES) {
	for (i = 0; i < 16; ++i) {
	    dicode1[(unsigned char) (dichar1[i])] = i * 8 + 1;
	}
	for (i = 0; i < 8; ++i) {
	    dicode2[(unsigned char) (dichar2[i])] = i + 1;
	}
    }
}

/* skip the list in the cross-reference file */

static void
skiplist(FILE *oldrefs)
{
    int	i;
	
    if (fscanf(oldrefs, "%d", &i) != 1) {
	postfatal("cscope: cannot read list size from file %s\n", reffile);
	/* NOTREACHED */
    }
    while (--i >= 0) {
	if (fscanf(oldrefs, "%*s") != 0) {
	    postfatal("cscope: cannot read list name from file %s\n", reffile);
	    /* NOTREACHED */
	}
    }
}

/* cleanup and exit */
void
myexit(int sig)
{
	/* HBB 20010313; close file before unlinking it. Unix may not care
	 * about that, but DOS absolutely needs it */
	if (refsfound != NULL)
		fclose(refsfound);
	
	/* remove any temporary files */
	if (temp1[0] != '\0') {
#if defined(USE_BTREE)
            file_delete(temp1);
#else
            _unlink(temp1);
#endif /* defined(USE_BTREE) */
            _unlink(temp2);
            //rmdir(tempdirpv);		
	}
#if defined(WITH_CURSES)
	/* restore the terminal to its original mode */
	if (incurses == YES) {
		exitcurses();
	}
#endif /* defined(WITH_CURSES) */
#if defined(HAVE_SIGQUIT)
	/* dump core for debugging on the quit signal */
	if (sig == SIGQUIT) {
		abort();
	}
#endif /* defined(HAVE_SIGQUIT) */
	/* HBB 20000421: be nice: free allocated data */
	freefilelist();
	freeinclist();
	freesrclist();
	freecrossref();
	free_newbuildfiles();

	//exit(sig);
    return;
}

int buildCrossRefs (void)
{
	int		refs = 0;
	char	*line = NULL;
	int		n = 0, len;
	char	*strings[10];

	int		reffd = _open(temp1, O_RDONLY);
	if (-1 == reffd) {
		return reffd;
	}

	while (0 != (len = getline (&line, &n, reffd))) {
		if (line) {
			replacechr(line, len, '/', '\\');
			strdivide(line, len, ' ', strings, 10);
			strcpy(refInfo[refs].filename, strings[0]);
			strcpy(refInfo[refs].function, strings[1]);
			refInfo[refs].lineno = atoi(strings[2]);
			free (line);
			line = NULL;
			n = 0;
			refs++;
		}
	}
	return refs;
}
