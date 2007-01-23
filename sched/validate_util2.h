#ifndef _VALIDATE_UTIL2_
#define _VALIDATE_UTIL2_

#include <vector>

extern int init_result(RESULT const&, void*&);
extern int compare_results(RESULT &, void*, RESULT const&, void*, bool&);
extern int cleanup_result(RESULT const&, void*);
extern double compute_granted_credit(std::vector<RESULT>& results);

#endif
