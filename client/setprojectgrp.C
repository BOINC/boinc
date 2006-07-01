// setprojectgrp.C
//
// When run as
// setprojectgrp Path
// sets group of file at Path to boinc_project
//
// setprojectgrp runs setuid boinc_master and setgid boinc_project

#include <unistd.h>
#include <grp.h>
#include <stdio.h>
#include <cerrno>

int main(int argc, char** argv) {
    gid_t       project_gid;
    int         retval;
    
    project_gid = getegid();

#if 0           // For debugging
    fprintf(stderr, "setprojectgrp argc=%d, arg[1]= %s, boinc_project gid = %d\n", argc, argv[1], project_gid);
    fflush(stderr);
#endif

    retval = chown(argv[1], (uid_t)-1, project_gid);
    if (retval)
        fprintf(stderr, "chown(%s, -1, %d) failed: errno=%d\n", argv[1], project_gid, errno);

    return retval;
}
