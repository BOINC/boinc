static volatile const char *BOINCrcsid="$Id$";
#include <stdio.h>

#include "util.h"
#include "sched_config.h"

int main(int argc, char** argv) {
    SCHED_CONFIG config;
    char path[256];
    int retval;

    retval = config.parse_file("..");
    if (retval) exit(1);

    dir_hier_path(argv[1], "", config.uldl_dir_fanout, path);
    printf("path: %s\n", path);
}
