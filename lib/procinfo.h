#include <vector>

struct PROCINFO {
	int id;
	int parentid;
    double swap_size;
    double working_set_size;
    double user_time;
    double kernel_time;
	bool is_boinc_app;
};

extern int procinfo_setup(std::vector<PROCINFO>&);
	// call this first to get data structure
extern void procinfo_app(PROCINFO&, std::vector<PROCINFO>&);
	// call this to get mem usage for a given app
	// (marks process as BOINC)
extern void procinfo_other(PROCINFO&, std::vector<PROCINFO>&);
	// After getting mem usage for all BOINC apps,
	// call this to get mem usage for everything else
