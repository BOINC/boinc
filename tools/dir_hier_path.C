#include <stdio.h>

#include "util.h"
#include "sched_config.h"

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    char path[256];
    int retval;

    retval = config.parse_file(".");
    if (retval) {
        fprintf(stderr, "Can't find config.xml; run this in project root dir\n");
        exit(1);
    }

    dir_hier_path(argv[1], "", config.uldl_dir_fanout, true, path);
    printf("path: %s%s\n", config.download_dir, path);
}

const char *BOINC_RCSID_c683969ea8 = "$Id$";
