// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Stage an input file
// Native (maybe more efficient) version of 'stage_file';
// same cmdline args

#include <zlib.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>

#include "filesys.h"
#include "error_numbers.h"
#include "sched_util_basic.h"
#include "md5_file.h"

#include "stage_file_native.h"

static int create_md5_file(
    const char* file_path, const char* md5_file_path, bool verbose
) {
    char md5_file_hash[MD5_LEN], path[MAXPATHLEN];
    FILE* md5_filep;
    int retval;
    double nbytes;

    retval = md5_file(file_path, md5_file_hash, nbytes);
    if (retval) {
        return retval;
    }

    snprintf(path, MAXPATHLEN, "%s.md5", md5_file_path);
    md5_filep  = boinc_fopen(path, "w");
    if (!md5_filep) {
        return ERR_FOPEN;
    }
    fprintf(md5_filep, "%s %d\n", md5_file_hash, (int) nbytes);
    fclose(md5_filep);

    if (verbose) {
        fprintf(stdout, "file's md5 hash and size: %s, %d bytes\n", md5_file_hash, (int) nbytes);
    }
    return 0;
}

int stage_file(
    char* file_path,
    bool gzip,
    bool copy,
    bool verbose
) {
    char dl_hier_path[MAXPATHLEN], gz_path[MAXPATHLEN];
    char* file_name;
    int retval;

    if (!boinc_file_exists(file_path)) {
        return ERR_FILE_MISSING;
    }

    file_name = basename(file_path);

    retval = dir_hier_path(
        file_name, config.download_dir,
        config.uldl_dir_fanout, dl_hier_path, true
    );
    if (retval) {
        return retval;
    }

    if (verbose) {
        fprintf(stdout, "staging %s to %s\n", file_path, dl_hier_path);
    }

    switch (check_download_file(file_path, dl_hier_path)) {
    case 0:
        if (verbose) {
            fprintf(stdout, "file %s has already been staged\n", file_path);
        }
        break;
    case 1:
        retval = create_md5_file(file_path, dl_hier_path, verbose);
        if (retval) {
            fprintf(stdout, "failed to create md5 file: %s\n", boincerror(retval));
            return retval;
        }
        break;
    case 2:
        if (copy) {
            retval = boinc_copy(file_path, dl_hier_path);
        } else {
            retval = boinc_rename(file_path, dl_hier_path);
        }
        if (retval) {
            fprintf(stdout, "failed to copy or move file: %s\n", boincerror(retval));
            return retval;
        }
        retval = create_md5_file(dl_hier_path, dl_hier_path, verbose);
        if (retval) {
            fprintf(stdout, "failed to create md5 file: %s\n", boincerror(retval));
            return retval;
        }
        break;
    case -1:
        fprintf(stderr,
            "There is already a file in your project's download directory with that name,\n"
            "but with different contents. This is not allowed by BOINC, which requires that\n"
            "files be immutable. Please use a different file name.\n"
        );
        return -1;
    case -2:
        fprintf(stderr, "check_download_file: file operation failed - %s\n", strerror(errno));
        return -1;
    default:
        fprintf(stderr, "check_download_file: unknown return code %d\n", retval);
        return -1;
    }

    if (gzip) {
        std::ifstream file(dl_hier_path);
        std::stringstream file_buf;
        file_buf << file.rdbuf();

        snprintf(gz_path, MAXPATHLEN, "%s.gz", dl_hier_path);
        gzFile gz = gzopen(gz_path, "w");
        if (!gz) {
            fprintf(stderr, "failed to open gz: %s\n", strerror(errno));
            return -1;
        }
        int bytes = gzwrite(gz, file_buf.str().c_str(), file_buf.str().size());
        retval = gzclose(gz);
        if (retval != Z_OK) {
            fprintf(stderr, "failed to close gz\n");
            return retval;
        }
        if (!bytes) {
            fprintf(stderr, "failed to write to gz: %s\n", strerror(errno));
            return -1;
        }
        if (verbose) {
            fprintf(stdout, "created .gzip file for %s\n", dl_hier_path);
        }
    }

    return 0;
}

void usage(int exit_code) {
    fprintf(stderr,
        "usage: stage_file [--gzip] [--copy] [--verbose] path\n"
        "    --gzip      make a gzipped version of file for compressed download\n"
        "                (use with <gzip/> in the input template)\n"
        "    --copy      copy the file (default is to move it)\n"
        "    --verbose   verbose output\n"
        "    --help      print this message\n"
        "    path        The file to stage; if directory, stage all files in that dir\n"
    );
    exit(exit_code);
}

void run_stage_file(
    char* file_path,
    bool gzip,
    bool copy,
    bool verbose
) {
    int retval = stage_file(file_path, gzip, copy, verbose);
    if (retval) {
        fprintf(stderr,
            "stage_file failed for file %s: %s\n",
            file_path, boincerror(retval)
        );
        exit(1);
    }
}

int main(int argc, char** argv) {
    int retval;
    bool gzip = false;
    bool copy = false;
    bool verbose = false;
    char path[MAXPATHLEN];
    char file_path[MAXPATHLEN];

    if (!boinc_file_exists("html/inc/dir_hier.inc") || !boinc_file_exists("config.xml")) {
        fprintf(stderr, "This program must be run in the project root directory.\n");
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    if (argc < 2 || argc > 5) {
        fprintf(stderr, "Incorrect number of arguments\n");
        usage(1);
    }

    if (!strcmp(argv[1], "--help")) {
        usage(0);
    }

    for (int i = 1; i < argc - 1; ++i) {
        if (!strcmp(argv[i], "--gzip")) {
            gzip = true;
        } else if (!strcmp(argv[i], "--copy")) {
            copy = true;
        } else if (!strcmp(argv[i], "--verbose")) {
            verbose = true;
        } else {
            fprintf(stderr, "Unknown optional argument: %s\n", argv[i]);
            usage(1);
        }
    }
    snprintf(path, MAXPATHLEN, "%s", argv[argc - 1]);

    if (is_dir(path)) {
        std::string file_name;
        DirScanner dir(path);
        while (dir.scan(file_name)) {
            snprintf(file_path, MAXPATHLEN, "%s/%s", path, file_name.c_str());
            if (!is_file(file_path)) {
                continue;
            }
            run_stage_file(file_path, gzip, copy, verbose);
        }
    } else  {
        run_stage_file(path, gzip, copy, verbose);
    }
}
