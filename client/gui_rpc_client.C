#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "parse.h"
#include "error_numbers.h"
#include "gui_rpc_client.h"

int RPC_CLIENT::init(char* path) {
    int sock, retval;
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        perror("connect");
        exit(1);
    }
    fin = fdopen(dup(sock), "r");
    fout = fdopen(sock, "w");
}

RPC_CLIENT::~RPC_CLIENT() {
    fclose(fin);
    fclose(fout);
}

int RPC_CLIENT::get_projects(vector<PROJECT>& projects) {
    char buf[256];
    int retval;

    fprintf(fout, "<get_projects>\n");
    fflush(fout);
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "<projects>")) continue;
        else if (match_tag(buf, "</projects>")) return 0;
        else if (match_tag(buf, "<project>")) {
            PROJECT project;
            retval = project.parse_state(fin);
            if (!retval) {
                projects.push_back(project);
            }
        } else {
            fprintf(stderr, "unrecognized: %s", buf);
        }
    }
    return ERR_XML_PARSE;
}

int PROJECT::parse_state(FILE* in) {
    char buf[256];
    STRING256 string;

    strcpy(project_name, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    resource_share = 100;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    min_rpc_time = 0;
    min_report_min_rpc_time = 0;
    nrpc_failures = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    scheduler_urls.clear();
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<scheduler_url>", string.text, sizeof(string.text))) {
            scheduler_urls.push_back(string);
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) continue;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
        else if (parse_str(buf, "<team_name>", team_name, sizeof(team_name))) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_int(buf, "<user_create_time>", (int &)user_create_time)) continue;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_int(buf, "<host_create_time>", (int &)host_create_time)) continue;
        else if (parse_double(buf, "<exp_avg_cpu>", exp_avg_cpu)) continue;
        else if (parse_int(buf, "<exp_avg_mod_time>", exp_avg_mod_time)) continue;
        else if (match_tag(buf, "<code_sign_key>")) {
            copy_element_contents(
                in,
                "</code_sign_key>",
                code_sign_key,
                sizeof(code_sign_key)
            );
        }
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_int(buf, "<min_rpc_time>", (int&)min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending/>")) master_url_fetch_pending = true;
        else if (match_tag(buf, "<sched_rpc_pending/>")) sched_rpc_pending = true;
        else printf("PROJECT::parse_state(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

PROJECT::PROJECT() {
    init();
}

void PROJECT::init() {
    strcpy(master_url, "");
    strcpy(authenticator, "");
    project_specific_prefs = "";
    resource_share = 100;
    strcpy(project_name, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    rpc_seqno = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    strcpy(code_sign_key, "");
    nrpc_failures = 0;
    min_rpc_time = 0;
    min_report_min_rpc_time = 0;
    master_fetch_failures = 0;
    resource_debt = 0;
    debt_order = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    tentative = false;
}

PROJECT::~PROJECT() {
}

