#ifndef __PLUGIN_CORE_H_
#define __PLUGIN_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CROSS_REFS		(100)

typedef enum {
	find_symbol = 0,
	find_def,
	find_calledby,
	find_calling,
	find_string,
	find_string2,
	find_regex,
	find_file,
	find_include,
	find_all_funcs,
	command_none
}commands_e;

typedef struct {
	char	filename[250];
	char	function[50];
	int		lineno;
}crossRefs;


typedef struct _callh {
	char			function[50];
	int				callers;
	struct _callh	**caller_list;
}callh;

extern crossRefs refInfo[MAX_CROSS_REFS];
extern int refs;

int createproject (const char *file, int ndirs,
                const char *srcdirs[]);
int openproject (char *file);
int buildCscope (const char *prjFolder);
int closeproject (const char *prjname, const char *prjfolder);
int deleteproject (const char *prjname, const char *prjfolder);
unsigned int directoryExists( const char* absolutePath );

int exec_command(commands_e cmd, const char *arg);

int buildCrossRefs (void);

int add_history (crossRefs	*ref);
int pop_history (crossRefs *his);

callh* build_call_tree (const char *function);
int free_call_tree (callh *root);
int display_call_tree (callh *root, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /*__PLUGIN_CORE_H_ */