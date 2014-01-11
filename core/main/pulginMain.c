#include <stdio.h>
#include <json.h>
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pluginMain.h>
#include <cscope.h>

/* Memory Debug options */
#ifdef MEMDBG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define NAME "Name"
#define LOCATION "Location"
#define NOFSRCDIRS "NoOfSrcDirs"
#define SRCDIRS "SourceDirs"

extern void myexit (int sig);
extern int mysetline (const char *str);
extern BOOL command(int commandc);

unsigned int directoryExists( const char* absolutePath )
{

    if( _access( absolutePath, 0 ) == 0 ){

        struct stat status;
        stat( absolutePath, &status );

        return (status.st_mode & S_IFDIR) != 0;
    }
    return 0;
}

/* Added functions for supporting features */
int createproject (const char *file, int ndirs,
                   const char *srcdirs[])
{
    json_object     *proj = NULL;
    int             success = 0;
    json_object     *dirs = NULL;
    int             i;

    /* Build only json file */
    proj = json_object_new_object();
    if (proj) {
        json_object_object_add(proj, NAME, json_object_new_string(file));
        json_object_object_add(proj, NOFSRCDIRS, json_object_new_int(ndirs));
        dirs = json_object_new_array();
        for (i = 1; i <= ndirs; i++) {
            json_object_array_add(dirs, json_object_new_string(srcdirs[i - 1]));
        }
        json_object_object_add(proj, SRCDIRS, dirs);
    }

    json_object_to_file(file, proj);
    json_object_put(proj);
    openproject(file);
    return success;
}

static int generateBatchFile (const char *name, const char *prjFolder, const char *sourceDir)
{
	int fd, length;
	char   *path = (char *)malloc(1024 * sizeof(char));
	char	base[101];

	if (path) {
		sprintf(path, "%s\\generate.bat", prjFolder);
		fd = _open(path, O_WRONLY | O_CREAT, _S_IREAD | _S_IWRITE);
		if (-1 == fd) {
			length = errno;
			return fd;
		}

		length = sprintf(path, "cd \"%s\"\n", prjFolder);
		_write(fd, path, length);

		length = sprintf(path, "\"%s\\plugins\\srcnav\\cscope.exe\" -R -b -f %s.conf -s \"%s\"\n",
			_getcwd(base, sizeof(base)), name, sourceDir);
		_write(fd, path, length);

		free (path);
		_close(fd);
	}
	return 0;
}

int buildCscope (const char *prjFolder)
{
	char	absPath[201];

	sprintf(absPath, "%s\\generate.bat", prjFolder);
	
	system(absPath);

    return 1;
}

int openproject (char *file)
{
	json_object		*prj = NULL;
	json_object		*srcDir = NULL;
	char			name[201] = {0};
	int				nSrcDirs = 0;
	int				loop = 0;
	const char		*folder;
	char			*srcPath = NULL;
	const char		*pPrjName = NULL;

	prj = json_object_from_file(file);
	if (!prj) {
		return -1;
	}

	loop = strlen(file);
	while (file[loop] != '\\') {
		loop--;
	}
	if (loop >=0 ) {
		file[loop] = '\0';
	}

	/* Check for project folder */
	if (!directoryExists(file)) {
		return -1;
	}

	pPrjName = json_object_get_string(json_object_object_get(prj, NAME));
	while (*pPrjName) {
		if (*pPrjName == '\\') {
			folder = pPrjName;
		}
		pPrjName++;
	}
	pPrjName = ++folder;
	folder = NULL;

	/* Get the source path */
	nSrcDirs = json_object_get_int(json_object_object_get(prj, NOFSRCDIRS));
	srcDir = json_object_object_get (prj, SRCDIRS);

	for (loop = 0; loop < 1 /*nSrcDirs*/; loop++) {
		folder = json_object_get_string(json_object_array_get_idx(srcDir, loop));
	}

	/* Check for source folder */
	if (!directoryExists(folder)) {
		return -1;
	}


	generateBatchFile(pPrjName, file, folder);
	buildCscope(file);

	sprintf(name, "%s\\%s.conf", file, pPrjName);

	myinit (name);

	json_object_put(prj);

	return 1;
}

int closeproject (const char *prjname, const char *prjfolder)
{
    myexit(0);
	return 1;
}

int deleteproject (const char *prjname, const char *prjfolder)
{
	/* Currently not implemented */
	return -1;
}



int exec_command(commands_e cmd, const char *arg)
{
	mysetline(arg);
	command(cmd);
	return 1;
}

/* Support for history */
#define MAX_HISTORY	(15)
static crossRefs refHistory[MAX_HISTORY];
static unsigned int stktop = 0, stkbase = 0, count = 0;

int add_history (crossRefs	*ref)
{
	/* stktop overflow ??. Dont know the outcome */
	if (count >= MAX_HISTORY) {
		stkbase = (++stkbase % MAX_HISTORY);
	}
	else {
		count++;
	}
	memcpy (&refHistory[stktop], ref, sizeof(*ref));
	stktop = (++stktop % MAX_HISTORY);
	return 1;
}

int pop_history (crossRefs *his)
{
	int	popped = 0;
	if (count && his) {
		count--;
		stktop = (stktop) ? (--stktop % MAX_HISTORY) : 0;
		memcpy(his, &refHistory[stktop], sizeof(*his));
		popped = 1;
	}
	return popped;
}

callh* build_call_tree (const char *function)
{
	callh	*root = NULL;
	callh	*child = NULL;
	int		each, sub;

	/* Create an entry for the function */
	root = (callh *)malloc (sizeof(callh));
	if (root) {
		strcpy(root->function, function);
		root->callers = 0;
		root->caller_list = NULL;
		exec_command(find_calling, function);
		sub = buildCrossRefs();
		/* Get call list */
		if (0 == sub) {
			return NULL;
		}
		root->callers = sub;
		root->caller_list = (callh **) calloc(sub, sizeof(callh *));
		for (each = 0; each < sub && root->caller_list; each++) {
			/* Assign the required fields */
			child = build_call_tree(refInfo[each].function);
			root->caller_list[each] = child;
		}
	}
	return root;
}

int free_call_tree (callh *root)
{
	int each;

	if (root) {
		for (each = 0; each < root->callers; each++) {
			free_call_tree (root->caller_list[each]);
		}
		if (root->caller_list) 
			free (root->caller_list);
		free (root);
	}
	return 1;
}

int display_call_tree (callh *root, FILE *fp)
{
	int				each;
	static  char	spaces[100] = {0};
	static  int     i = 0;
	static  const char s = '\t';

	if (root) {
		fprintf(fp, "%s -> %s\n", spaces, root->function);
		if (root->callers) {
			for (each = 0; each < root->callers; each++)
				memcpy((spaces + i), &s, sizeof(s));
				i++;
				display_call_tree (root->caller_list[each], fp);
		}
	}
	--i;
	spaces[i] = 0;
	return 1;
}
