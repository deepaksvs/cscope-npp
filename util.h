#ifndef __C_UTILS_H__
#define __C_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Get a linr from the file descriptor
	string - Pointer to a string ( Can be NULL )
	size   - Size of the string if not NULL. Else 0
	fd	   - FD from where the data is read
*/
int getline (char **string, int *size, int fd);

int replacechr (char *string, int length, char c, char s);

int strdivide (char	*string, int length, char dlim, char *strs[], int n);

#ifdef __cplusplus
}
#endif

#endif /* __C_UTILS_H__ */