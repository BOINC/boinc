#include <stdio.h>

#include "gui_rpc_client.h"

main() {
    RPC_CLIENT rpc;
    vector<PROJECT>projects;
    unsigned int i;

    rpc.init("gui_rpc");
    rpc.get_projects(projects);
    for (i=0; i<projects.size(); i++) {
        PROJECT& project = projects[i];
        printf("%s: %s\n", project.master_url, project.project_name);
    }
}
