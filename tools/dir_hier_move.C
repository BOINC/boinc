static volatile const char *BOINCrcsid="$Id$";
// dir_hier_move src_dir dst_dir fanout
//
// move files from src_dir (flat) into dst_dir (hierarchical)
// with the given fanout

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <errno.h>

#include "filesys.h"
#include "util.h"

int main(int argc, char** argv) {
    char* src_dir, *dst_dir;
    int fanout=0;
    std::string filename;
    char dst_path[256], src_path[256];
    int retval;
    
    if (argc != 4) exit(1);
    src_dir = argv[1];
    dst_dir = argv[2];
    fanout = atoi(argv[3]);
    if (!fanout) exit(1);

    DirScanner scanner(src_dir);
    while (scanner.scan(filename)) {
        retval = dir_hier_path(filename.c_str(), dst_dir, fanout, dst_path, true);
        if (retval) {
            fprintf(stderr, "dir_hier_path: %d\n", retval);
            exit(1);
        }
        sprintf(src_path, "%s/%s", src_dir, filename.c_str());
        retval = rename(src_path, dst_path);
        if (retval) {
            fprintf(stderr, "rename: %d, errno is %d\n", retval, errno);
            exit(1);
        }
    }
}
