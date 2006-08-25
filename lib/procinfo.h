#include <vector>

struct PROCINFO {
    double virtual_size;
    double working_set_size;
    double user_time;
    double kernel_time;
};

extern int get_procinfo(std::vector<PROCINFO>& pi).
