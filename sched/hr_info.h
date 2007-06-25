#include "hr.h"

struct HR_INFO {
    double *rac_per_class[HR_NTYPES];
        // how much RAC per class
    int *max_slots[HR_NTYPES];
        // max # of work array slots per class
    int *cur_slots[HR_NTYPES];
        // estimate of current # of slots used for per class
    double type_weights[HR_NTYPES];
        // total app weight per type
    int slots_per_type[HR_NTYPES];
        // # of slots per type (fixed at start)
    bool type_being_used[HR_NTYPES];
        // whether any app is actually using this HR type
    int write_file();
    int read_file();
    void scan_db();
    void allot();
    void init();
    void allocate(int);
    bool accept(int, int);
};

#define HR_INFO_FILENAME "../hr_info.txt"
