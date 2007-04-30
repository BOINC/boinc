#include <vector>
#include <algorithm>

using std::vector;

#define INFO0 printf
#define INFO1 printf
#define INFO2 printf

struct IP_RESULT {
    char name[256];
    double report_deadline;
    double cpu_time_remaining;
    //int parse(FILE*);
    IP_RESULT(const char* n, double r, double c) {
        strcpy(name, n);
        report_deadline = r;
        cpu_time_remaining = c;
    }
};

// Compared to IP_RESULT, stores the computation_deadline
// (versus report_deadline),
// and bool indicating whether or not the deadline can be met when in EDF mode

struct TEMP_RESULT {
public:
    char name[256];
    double computation_deadline;
    double cpu_time_remaining;
    bool misses_deadline;
        // whether or not the result would have missed its deadline independent
        // of any newly scheduled result
        // used to determine if late results will complete even later
    double estimated_completion_time;
    TEMP_RESULT (const char* n, double cd, double tr){
        strcpy(name, n);
        computation_deadline = cd;
        cpu_time_remaining = tr;
        misses_deadline = false;
        estimated_completion_time = 0;
    }
};

bool lessthan_deadline(const TEMP_RESULT& p1, const TEMP_RESULT& p2) {
    if (p1.computation_deadline < p2.computation_deadline) return true;
    return false;
}

struct REQUEST_HANDLER_WORK_SEND {
    vector<TEMP_RESULT> create_ipp_results(
        double work_buf_min,
        int ncpus,
        vector<IP_RESULT> ip_results
    );
    void mark_edf_misses (
        int ncpus, vector<TEMP_RESULT>& ipp_results
    );
    vector<TEMP_RESULT> find_sendable_test_results (
        vector<TEMP_RESULT> test_results,
        int ncpus,
        vector<TEMP_RESULT> ipp_results,
        double cpu_pessimism_factor
    );
    bool result_cause_deadline_miss_or_delay (
        double test_computation_deadline,
        double test_cpu_time_remaining,
        int ncpus,
        vector<TEMP_RESULT> ipp_results
    );
};

// For each ip_result, computes computation_deadline from report_deadline,
// and determines if the deadline would be missed by simulating edf
//
vector<TEMP_RESULT> REQUEST_HANDLER_WORK_SEND::create_ipp_results(
    double work_buf_min,
    int ncpus,
    vector<IP_RESULT> ip_results
){
    vector<IP_RESULT>::iterator ip_it;
    vector<TEMP_RESULT> ipp_results;
    for (ip_it = ip_results.begin(); ip_it != ip_results.end(); ip_it++) {
        IP_RESULT ip_r = *ip_it;
        double computation_deadline = ip_r.report_deadline - work_buf_min;
        TEMP_RESULT ipp_r (
            ip_r.name, computation_deadline, ip_r.cpu_time_remaining
        );
        ipp_results.push_back (ipp_r);
    }

    // run edf simulation to determine whether any results miss their deadline
    //
    mark_edf_misses (ncpus, ipp_results);
    return ipp_results;
}

// runs an edf simulation, markings which results will miss
// their deadlines and when
//
void REQUEST_HANDLER_WORK_SEND::mark_edf_misses (
    int ncpus, vector<TEMP_RESULT>& ipp_results
){
    vector<TEMP_RESULT>::iterator ipp_it;
    double booked_to[ncpus];
    int j;

    INFO1("mark_edf_misses\n");

    // keeps track of when each cpu is next free
    //
    for (j=0; j<ncpus; j++) {
        booked_to[j] = 0;
    }

    // sort ipp_results in ascending order by deadline
    //
    sort(ipp_results.begin(), ipp_results.end(), lessthan_deadline);

    // simulate edf execution of all queued results
    //
    for (ipp_it = ipp_results.begin();
	    ipp_it != ipp_results.end();
	    ipp_it++
	) {
        // find the CPU that will be free first
        //
        double lowest_booked_time = booked_to[0];
        int lowest_booked_cpu = 0;
        for (j=1; j<ncpus; j++) {
            if (booked_to[j] < lowest_booked_time) {
                lowest_booked_time = booked_to[j];
                lowest_booked_cpu = j;
            }
        }
      
        booked_to[lowest_booked_cpu] += ipp_it->cpu_time_remaining;
        INFO2("  running %s on cpu %d; finishes at %f\n",
            ipp_it->name, lowest_booked_cpu, booked_to[lowest_booked_cpu]
        );
        if (booked_to[lowest_booked_cpu] > ipp_it->computation_deadline) {
	        ipp_it->misses_deadline = true;
	        ipp_it->estimated_completion_time = booked_to[lowest_booked_cpu];
	        INFO1("  %s misses_deadline; est completion %f\n",
                ipp_it->name, booked_to[lowest_booked_cpu]
            );
        } else {
	        ipp_it->misses_deadline = false;
	        INFO1("  %s makes deadline; est completion %f\n",
                ipp_it->name, booked_to[lowest_booked_cpu]
            );
	        // if result doesn't miss its deadline,
            // then the estimated_completion_time is of no use
        }
    }
}

// Sort test_results by computation_deadline. 
// For each test result in ascending order of deadline,
// see whether adding it to the work queue would cause deadline misses
// or deadline miss delays.
// If a test result passes these checks, append it to the work queue
// for further result additions.
// Return list of new results that can be sent to the client.  
//
// NOTE: should we sort by increasing deadline or by increasing slack time?
//
vector<TEMP_RESULT> REQUEST_HANDLER_WORK_SEND::find_sendable_test_results (
    vector<TEMP_RESULT> test_results,
    int ncpus, 
    vector<TEMP_RESULT> ipp_results, 
    double cpu_pessimism_factor // = 1 by default
) {
    //test results to send
    vector<TEMP_RESULT> sendable_test_results; 
    vector<TEMP_RESULT>::iterator test_results_it;

    sort(test_results.begin(), test_results.end(), lessthan_deadline);

    // see if each test result can be added to the work queue without
    // causing deadline misses or deadline miss delays
    //
    for (test_results_it = test_results.begin();
        test_results_it != test_results.end();
        test_results_it++
    ) {
        bool failed = result_cause_deadline_miss_or_delay(
            (*test_results_it).computation_deadline*cpu_pessimism_factor,
            (*test_results_it).cpu_time_remaining,
            ncpus,
            ipp_results
        );
        if (!failed){
            // add sendable result to work queue, copying by value, so that we
            // can evaluate what happens if we add more new results to the queue
            //
            ipp_results.push_back(*test_results_it);
            sendable_test_results.push_back (*test_results_it);
        }
    }
    return (sendable_test_results);
}

// Return true if
// 1) the candidate result X would cause another result Y to miss its deadline
//    (which Y would not have otherwise missed)
// 2) X causes another result Y to miss its deadline by more than
//    it otherwise would have, or
// 3) X would miss its deadline
//
bool REQUEST_HANDLER_WORK_SEND::result_cause_deadline_miss_or_delay (
    double test_computation_deadline, 
    double test_cpu_time_remaining,
    int ncpus, 
    vector<TEMP_RESULT> ipp_results
) {
    vector<TEMP_RESULT>::iterator ipp_it;
    double booked_to[ncpus];     // keeps track of when each cpu is free
    int j;

    INFO0 ("result_cause_deadline_miss_or_delay; dl %f cpu %f\n",
        test_computation_deadline, test_cpu_time_remaining
    );

    for (j=0; j<ncpus; j++) {
        booked_to[j] = 0;
    }
    TEMP_RESULT test_ipp(
        "candidate", test_computation_deadline, test_cpu_time_remaining
    );
    ipp_results.push_back(test_ipp);

    // sort ipp_results in ascending order by deadline
    //
    sort(ipp_results.begin(), ipp_results.end(), lessthan_deadline);

    // simulate execution of all queued results and test result
    //
    for (ipp_it = ipp_results.begin(); ipp_it != ipp_results.end(); ipp_it++) {
        // find the CPU that will be free first
        //
        double lowest_booked_time = booked_to[0];
        int lowest_booked_cpu = 0;
        for (j=1; j<ncpus; j++) {
	        if (booked_to[j] < lowest_booked_time) {
                lowest_booked_time = booked_to[j];
                lowest_booked_cpu = j;
            }
        }
        booked_to[lowest_booked_cpu] += ipp_it->cpu_time_remaining;
        INFO2("  running %s on cpu %d; finishes at %f\n",
            ipp_it->name, lowest_booked_cpu, booked_to[lowest_booked_cpu]
        );

        // return true if completion time if > computation_deadline AND
        // result would not have missed deadline to begin with
        //
        if (booked_to[lowest_booked_cpu] > ipp_it->computation_deadline
            && !ipp_it->misses_deadline
        ) {
	        INFO1 ("  %s now misses deadline: %f\n",
                ipp_it->name, booked_to[lowest_booked_cpu]
            );
            return true;
        }
        // check a late result (i.e., one that would have missed its
        // deadline) // would be made even later
        //
        if (ipp_it->misses_deadline 
            && booked_to[lowest_booked_cpu] > ipp_it->estimated_completion_time
        ){
            INFO0 ("  %s: late result to be returned even later\n",
                ipp_it->name
            );
            return true;
        }
    }
    return false;
}

int main() {
    vector<IP_RESULT> ip_results;
    vector<TEMP_RESULT> ipp_results;
    vector<TEMP_RESULT> temp_results;
    vector<TEMP_RESULT> candidates;
    vector<TEMP_RESULT> sendable;
    REQUEST_HANDLER_WORK_SEND ws;

    double work_buf_min = 0;
    double cpu_pessimism_factor = 1;
    int ncpus = 1;

    ip_results.push_back(IP_RESULT("R1", 5, 3));
    ip_results.push_back(IP_RESULT("R2", 5, 3));
    candidates.push_back(TEMP_RESULT("R4_1", 10, 1));

    ipp_results = ws.create_ipp_results(work_buf_min, ncpus, ip_results);
    sendable = ws.find_sendable_test_results(
        candidates, ncpus, ipp_results, cpu_pessimism_factor
    );
    for (unsigned int i=0; i<sendable.size(); i++) {
        TEMP_RESULT tr = sendable[i];
        printf("send: %s\n", tr.name);
    }
}
