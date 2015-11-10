// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// another BOINC test application;
// converts one or more files to uppercase,
// then uses some CPU time
//
// cmdline: in1 out1 ... inn outn
//

#include <stdio.h>

#include "boinc_api.h"
#include "filesys.h"
#include "util.h"

double cpu_time = 20, comp_result;

// do about .5 seconds of computing
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//
static double do_some_computing(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<50000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

int convert_file(char* in, char* out) {
    char input_path[1024], output_path[1024];
    char buf[256];

    boinc_resolve_filename(in, input_path, sizeof(input_path));
    FILE* infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr,
            "%s Couldn't find input file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), input_path
        );
        return -1;
    }

    boinc_resolve_filename(out, output_path, sizeof(output_path));
    FILE* outfile = boinc_fopen(output_path, "w");
    if (!outfile) {
        fprintf(stderr,
            "%s Couldn't find output file, resolved name %s.\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path
        );
        fclose(infile);
        return -1;
    }

    while (1) {
        char c = fgetc(infile);
        if (c == EOF) break;
        c = toupper(c);
        fputc(c, outfile);
    }

    fclose(infile);
    fclose(outfile);
    return 0;
}

int main(int argc, char **argv) {
    char buf[256];
    int retval = boinc_init();
    if (retval) {
        fprintf(stderr, "%s boinc_init returned %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), retval
        );
        exit(retval);
    }

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--cpu_time")) {
            cpu_time = atof(argv[++i]);
        } else {
            retval = convert_file(argv[i], argv[i+1]);
            if (retval) exit(-1);
            i++;
        }
    }

    // burn up some CPU time if needed
    //
    if (cpu_time) {
        double start = dtime();
        for (int i=0; ; i++) {
            double e = dtime()-start;
            if (e > cpu_time) break;
            comp_result = do_some_computing(i);
        }
    }
    boinc_finish(0);
}

#ifdef _WIN32
int WINAPI WinMain(
    HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode
) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    return main(argc, argv);
}
#endif
