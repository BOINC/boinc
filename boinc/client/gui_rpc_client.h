#include <stdio.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

struct PROJECT;

struct FILE_INFO {
    string name;
    bool generated_locally;
    bool uploaded;
    bool upload_when_present;
    bool sticky;
    bool pers_xfer_active;
    bool xfer_active;
    int num_retries;
    double bytes_xferred;
    double file_offset;
    double xfer_speed;
    string hostname;
    PROJECT* project;

    int parse(FILE*);
    void print();
};

struct PROJECT {
    string master_url;
    double resource_share;
    string project_name;
    string user_name;
    string team_name;
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    int min_rpc_time;           // earliest time to contact any server

    bool master_url_fetch_pending; // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed

    int parse(FILE*);
    void print();
};

struct APP {
    string name;
    PROJECT* project;

    int parse(FILE*);
    void print();
};

struct APP_VERSION {
    string app_name;
    int version_num;
    APP* app;
    PROJECT* project;

    int parse(FILE*);
    void print();
};

struct WORKUNIT {
    string name;
    string app_name;
    int version_num;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;

    int parse(FILE*);
    void print();
};

struct RESULT {
    string name;
    string wu_name;
    int report_deadline;
    bool ready_to_report;
    bool got_server_ack;
    double final_cpu_time;
    int state;
    int exit_status;
    int signal;
    int active_task_state;
    string stderr_out;
    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    int parse(FILE*);
    void print();
};

struct ACTIVE_TASK {
    string result_name;
    int app_version_num;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    PROJECT* project;
    RESULT* result;

    int parse(FILE*);
    void print();
};

class RPC_CLIENT {
    int sock;
    FILE* fin;
    FILE* fout;
public:
    vector<PROJECT*> projects;
    vector<FILE_INFO*> file_infos;
    vector<APP*> apps;
    vector<APP_VERSION*> app_versions;
    vector<WORKUNIT*> wus;
    vector<RESULT*> results;
    vector<ACTIVE_TASK*> active_tasks;

    APP* lookup_app(string&);
    WORKUNIT* lookup_wu(string&);
    APP_VERSION* lookup_app_version(string&, int);
    RESULT* lookup_result(string&);

    void link();

    ~RPC_CLIENT();
    int init(char*);
    int get_state();
    void print();
};

