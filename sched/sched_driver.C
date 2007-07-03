// This program generates a stream of scheduler requests;
// it acts as a "driver" for the scheduler when used as:
// sched_driver | cgi --batch
//
// This was written to test the homogeneous redundancy features
// of the feeder and scheduler,
// but it could be used for a variety of other purposes.
//
// Usage: sched_driver --nrequests N
//
// The requests ask for a uniformly-distributed random amount of work.
// Their OS and CPU info is taken from the successive lines of a file
// of the form
// | os_name | p_vendor | p_model |
// You can generate this file using a SQL query
// and trimming off the start and end.

#include <vector>

using std::vector;

struct HOST_DESC{
    char os_name[128];
    char p_vendor[128];
    char p_model[128];
};

vector<HOST_DESC> host_descs;
double min_time = 1;
double max_time = 1;

void read_hosts() {
    char buf[256], buf2[256];
    host_descs.clear();
    FILE* f = fopen("host_descs.txt", "r");
    if (!f) {
        fprintf(stderr, "no input file\n");
        exit(1);
    }
    while (fgets(buf, sizeof(buf), f)) {
        HOST_DESC hd;
        strcpy(buf2, buf);
        char* p1 = strtok(buf2, "\t\n");
        strcpy(hd.os_name, p1);
        char* p2 = strtok(0, "\t\n");
        strcpy(hd.p_vendor, p2);
        char* p3 = strtok(0, "\t\n");
        if (!p3) {
            fprintf(stderr, "bad line: %s\n", buf);
            exit(1);
        }
        strcpy(hd.p_model, p3);
        host_descs.push_back(hd);
    }
}

void make_request(int i) {
    HOST_DESC& hd = host_descs[i%host_descs.size()];
    printf(
        "<scheduler_request>\n"
        "   <authenticator>1234</authenticator>\n"
        "   <hostid>1234</hostid>\n"
        "   <rpc_seqno>1234</rpc_seqno>\n"
        "   <work_req_seconds>1234</work_req_seconds>\n"
        "   <platform_name>1234</platform_name>\n"
        "   <host_info>\n"
        "      <os_name>%s</os_name>\n"
        "      <p_vendor>%s</p_vendor>\n"
        "      <p_model>%s</p_model>\n"
        "      <p_fops>1e9</p_fops>\n"
        "      <m_nbytes>1e9</m_nbytes>\n"
        "      <d_total>1e11</d_total>\n"
        "      <d_free>1e11</d_free>\n"
        "   </host_info>\n"
        "</scheduler_request>\n",
        hd.os_name,
        hd.p_vendor,
        hd.p_model
    );
}

int main(int argc, char** argv) {
    int i, nrequests = 1;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--nrequests")) {
            nrequests = atoi(argv[++i]);
        }
    }
    read_hosts();
    for (i=0; i<nrequests; i++) {
        make_request(i);
    }
}
