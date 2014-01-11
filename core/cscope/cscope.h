#ifndef __CSCOPE_H__
#define __CSCOPE_H__

#ifdef __cplusplus
extern "C" {
#endif

int myinit (const char *reffileabs);
void myexit(int sig);


#ifdef __cplusplus
}
#endif

#endif /* __CSCOPE_H__ */