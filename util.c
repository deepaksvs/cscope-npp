#include <stdio.h>
#include <io.h>
#include <stdlib.h>

int getline (char **s, int *n, int fd)
{
	int		b = 0, p = 0;
	if (!s || !n) {
		return p;
	}

	if (*s == NULL && *n == 0) {
		*s = (char *) calloc (2048, sizeof(char));
		*n = 2048;
	}

	if (!*s || *n == 0) {
		return p;
	}

	while (1) {
		b = _read (fd, (*s + p), 1);
		if (b <= 0) {
			/* has failed to read in the line. may be EOF */
			break;
		}
		p++;
		if ((*(*s + p - 1) == '\n') || (*(*s + p - 1) == '\0') || (p - 1 >= *n)) {
			break;
		}
	}
	return p;
}

int replacechr (char *str, int l, char c, char s)
{
	int o = 0;
	if (str) {
		while (--l >= 0) {
			if (c == (*(str + l))) {
				o++;
				(*(str + l)) = s;
			}
		}
	}
	return o;
}

int strdivide (char	*s, int l, char dlim, char *strs[], int n)
{
	int	c = 0;

	if (!s || !strs) {
		return -1;
	}

	while (*s && (c < n)) {
		strs[c++] = s;
		while ((*s != dlim) && (*s != '\0')) {
			s++;
		}
		if (*s == '\0') break;
		*s = '\0';
		s++;
	}

	return c;
}
