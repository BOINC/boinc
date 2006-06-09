#ifndef _VALIDATE_UTIL2_
#define _VALIDATE_UTIL2_

extern int init_result(RESULT const&, void*&);
extern int compare_results(RESULT &, void*, RESULT const&, void*, bool&);
extern int cleanup_result(RESULT const&, void*);

#endif
