// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// read "in", convert to UC, write to "out"
// command line options:
// -run_slow: sleep 1 second after each character, useful for debugging
// -cpu_time N: use about N CPU seconds after copying files
//

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#ifndef _WIN32
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif

#define BOINC_APP_GRAPHICS

#ifdef BOINC_APP_GRAPHICS
#include "graphics_api.h"
#include "graphics_lib.h"
#endif

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

using std::string;

#define CHECKPOINT_FILE "upper_case_state"
#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

bool run_slow;
bool raise_signal;
bool random_exit;
bool random_crash;
double cpu_time=20;

int do_checkpoint(MFILE& mf, int nchars) {
    int retval;
    string resolved_name;

    FILE* f = fopen("temp", "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    fprintf(stderr, "APP: upper_case checkpointing\n");

    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_filename_s(CHECKPOINT_FILE, resolved_name);
    retval = boinc_rename("temp", resolved_name.c_str());
    if (retval) return retval;

    return 0;
}

static void use_some_cpu() {
    double j = 3.14159;
    int i, n = 0;
    for (i=0; i<20000000; i++) {
        n++;
        j *= n+j-3.14159;
        j /= (float)n;
    }
}

void worker() {
    int c, nchars = 0, retval;
    double fsize;
    char resolved_name[512];
    MFILE out;
    FILE* state, *infile;

    boinc_resolve_filename(INPUT_FILENAME, resolved_name, sizeof(resolved_name));
    infile = boinc_fopen(resolved_name, "r");
    if (!infile) {
        fprintf(
            stderr,
            "Couldn't find input file, resolved name %s.\n",
            resolved_name
        );
        exit(-1);
    }

    file_size(resolved_name, fsize);

    boinc_resolve_filename(CHECKPOINT_FILE, resolved_name, sizeof(resolved_name));
    state = boinc_fopen(resolved_name, "r");
    if (state) {
        fscanf(state, "%d", &nchars);
        fclose(state);
        fseek(infile, nchars, SEEK_SET);
        boinc_resolve_filename(OUTPUT_FILENAME, resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "a");
    } else {
        boinc_resolve_filename(OUTPUT_FILENAME, resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "w");
    }
    if (retval) {
        fprintf(stderr, "APP: upper_case output open failed:\n");
        fprintf(stderr, "resolved name %s, retval %d\n", resolved_name, retval);
        perror("open");
        exit(1);
    }
    while (1) {
        c = fgetc(infile);

        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
        nchars++;

        if (run_slow) {
            boinc_sleep(1.);
        }

#ifdef HAVE_SIGNAL_H
        if (raise_signal) {
            raise(SIGHUP);
        }
#endif
        if (random_exit) {
            if (drand() < 0.05) {
                exit(-10);
            }
        }

        if (random_crash) {
            if (drand() < 0.05) {
                boinc_sleep(5.0);
#ifdef _WIN32
                DebugBreak();
#endif
            }
        }

        int flag = boinc_time_to_checkpoint();
        if (flag) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: upper_case checkpoint failed %d\n", retval);
                exit(1);
            }
            boinc_checkpoint_completed();
        }

        boinc_fraction_done(nchars/fsize);
    }
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "APP: upper_case flush failed %d\n", retval);
        exit(1);
    }
    if (cpu_time) {
        double start = dtime();
        while (1) {
            double e = dtime()-start;
            if (e > cpu_time) break;
            boinc_fraction_done(e/cpu_time);
            use_some_cpu();
        }
    }
    boinc_finish(0);
}

int main(int argc, char **argv) {
    int i;
    int retval = 0;
    bool nographics_flag = false;

    boinc_init_diagnostics(
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
        BOINC_DIAG_TRACETOSTDERR |
        BOINC_DIAG_REDIRECTSTDERR
    );

    // Write through to disk
    setbuf(stderr, 0);

#ifdef _WIN32
	// Attempt to load the dlls that are required to display graphics, if
	// any of them fail do not start the application in graphics mode.
	if (FAILED(__HrLoadAllImportsForDll("GDI32.dll"))) {
	   fprintf( stderr, "Failed to load GDI32.DLL...\n" );
	   nographics_flag = true;
	}
	if (FAILED(__HrLoadAllImportsForDll("OPENGL32.dll"))) {
	   fprintf( stderr, "Failed to load OPENGL32.DLL...\n" );
	   nographics_flag = true;
	}
	if (FAILED(__HrLoadAllImportsForDll("GLU32.dll"))) {
	   fprintf( stderr, "Failed to load GLU32.DLL...\n" );
	   nographics_flag = true;
	}
#endif


    // NOTE: if you change output here, remember to change the output that
    // test_uc.py pattern-matches against.

    for (i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-exit")) random_exit = true;
        if (!strcmp(argv[i], "-crash")) random_crash = true;
        if (!strcmp(argv[i], "-run_slow")) run_slow = true;
        if (!strcmp(argv[i], "-cpu_time")) {
            cpu_time = atof(argv[++i]);
        }
    }

#ifdef BOINC_APP_GRAPHICS
#if defined(_WIN32) || defined(__APPLE__)
    if (!nographics_flag) {
        retval = boinc_init_graphics(worker);
    }
#else
    if (!nographics_flag) {
        retval = boinc_init_graphics_lib(worker, argv[0]);
    }
#endif
    if (retval) exit(retval);
#else
    retval = boinc_init();
    if (retval) exit(retval);
    worker();
#endif
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

const char *BOINC_RCSID_33ac47a071 = "$Id$";
