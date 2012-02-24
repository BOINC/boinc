// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// put_file [options]
// --host_id N           ID of host to send to
// --file_name name      name of file

#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string>
#include <time.h>

#include "backend_lib.h"
#include "md5_file.h"
#include "svn_version.h"
#include "filesys.h"

#include "sched_config.h"
#include "sched_util.h"

void usage() {
    fprintf(stderr,
        "put_file [options]: send a file to a host\n\n"
        "Options:\n"
        "  --host_id id                 host DB ID\n"
        "  --file_name name             file name\n"
        "  [--url X]                    file URL (can specify several)\n"
        "  [--md5 X]                    file MD5 (must specify if nonlocal)\n"
        "  [--nbytes X]                 file size (must specify if nonlocal)\n"
        "  [--max_latency X]            max latency, seconds\n"
        "  [ -h | --help ]              Show this help text.\n"
        "  [ -v | --version ]           Show version information.\n"
    );
    exit(1);
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256], url[1024], path[1024];
    int host_id;
    vector<const char*> urls;
    double nbytes = -1;
    char md5[256];
    double max_latency = 7*86400;

    strcpy(file_name, "");
    strcpy(md5, "");
    host_id = 0;

    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) usage();
            host_id = atoi(argv[i]);
        } else if (is_arg(argv[i], "file_name")) {
            if (!argv[++i]) usage();
            strcpy(file_name, argv[i]);
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage();
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
        } else if (is_arg(argv[i], "url")) {
            urls.push_back(argv[++i]);
        } else if (is_arg(argv[i], "md5")) {
            strcpy(md5, argv[++i]);
        } else if (is_arg(argv[i], "nbytes")) {
            nbytes = atof(argv[++i]);
        } else if (is_arg(argv[i], "max_latency")) {
            max_latency = atof(argv[++i]);
        } else {
            usage();
        }
    }

    if (!strlen(file_name)) {
        usage();
    }
    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %s\n", boincerror(retval));
        exit(1);
    }

    if (urls.size() == 0) {
        // The file is local.
        // Make sure it's there, and compute its MD5
        //
        dir_hier_path(file_name, config.download_dir, config.uldl_dir_fanout, path);
        if (!boinc_file_exists(path)) {
            fprintf(stderr, "file not found: %s\n", path);
            exit(1);
        }
        dir_hier_url(file_name, config.download_url, config.uldl_dir_fanout, url);
        urls.push_back(url);
        retval = md5_file(path, md5, nbytes);
        if (retval) {
            fprintf(stderr, "md5_file() failed: %s\n", boincerror(retval));
            exit(1);
        }
    } else {
        if (nbytes == -1 || !strlen(md5)) {
            usage();
        }
    }

    retval = create_put_file_msg(
        host_id, file_name, urls, md5, nbytes, dtime()+max_latency
    );

    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_f3c3c4b892 = "$Id$";
