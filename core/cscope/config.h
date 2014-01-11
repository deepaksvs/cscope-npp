/*******************************************************************************
 * Copyright (C) 2009 Elad Lahav
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#undef HAVE_NCURSES
#undef HAVE_CURSES

/* Adding custom types for compilation to gothrough */
typedef unsigned short mode_t;

#define USE_SORTLIB

#if defined(HAVE_NCURSES) || defined(HAVE_CURSES)
#define WITH_CURSES
#endif /* defined(HAVE_NCURSES) || defined(HAVE_CURSES) */

#define HAVE_FCNTL_H
#undef HAVE_WAIT_H
#define HAVE_REGEX_H

#undef HAVE_LSTAT
#undef HAVE_STRERROR
#undef HAVE_SIGSETJMP

#undef HAVE_SIGQUIT
#undef HAVE_SIGHUP
#undef HAVE_SIGPIPE
#undef HAVE_SIGSTP

#undef HAVE_F_DUPFD

#undef HAVE_QSORT_R

#endif /* __CONFIG_H__ */