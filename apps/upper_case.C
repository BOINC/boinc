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
// -cpu_time: chew up some CPU cycles after each character,
// -signal:   raise SIGHUP signal (for testing signal handler)
// -exit:     exit with status -10 (for testing exit handler)
//

#ifdef _WIN32
#include "boinc_win.h"
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

#ifdef BOINC_APP_GRAPHICS
#include "boinc_gl.h"
#include "graphics_api.h"
#endif

#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

using std::string;

#define CHECKPOINT_FILE "upper_case_state"

#ifdef BOINC_APP_GRAPHICS
char display_buf[10];
double xPos=0, yPos=0;
double xDelta=0.03, yDelta=0.07;
#endif

bool run_slow=false, raise_signal=false, random_exit=false;
bool cpu_time=false;
time_t my_start_time;
APP_INIT_DATA uc_aid;

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

#ifdef _WIN32

extern int main(int argc, char** argv);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

int main(int argc, char **argv) {
    int c, nchars = 0, retval, i, n;
    double j, fsize;
    char resolved_name[512];
    MFILE out;
    FILE* state, *in;
    int flags = 0;

    my_start_time = time(0);


    flags =
        BOINC_DIAG_DUMPCALLSTACKENABLED |
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_REDIRECTSTDERR;
#if 0
    if (standalone)
        flags |= BOINC_DIAG_TRACETOSTDERR;
#endif
    boinc_init_diagnostics(flags);


    // NOTE: if you change output here, remember to change the output that
    // test_uc.py pattern-matches against.

    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: upper_case: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "-run_slow")) run_slow = true;
        if (!strcmp(argv[i], "-cpu_time")) cpu_time = 1;
        if (!strcmp(argv[i], "-signal")) raise_signal = true;
        if (!strcmp(argv[i], "-exit")) random_exit = true;
    }

    retval = boinc_init();
    if (retval) exit(retval);

    // can't write to stderr until after boinc_init()
    //
    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: upper_case: argv[%d] is %s\n", i, argv[i]);
    }

#ifdef BOINC_APP_GRAPHICS
    strcpy(display_buf, "(none)\0");
    retval = boinc_init_opengl();
    if (retval) exit(retval);
#endif

    boinc_get_init_data(uc_aid);
    // fprintf(stderr,
    //     "<app prefs>\n%s\n</app_prefs>\n", uc_aid.app_preferences
    // );

    fprintf(stderr, "APP: upper_case: starting, argc %d\n", argc);

    boinc_resolve_filename("in", resolved_name, sizeof(resolved_name));
    in = fopen(resolved_name, "r");
    if (in == NULL) {
        fprintf(
            stderr,
            "Couldn't find input file, resolved name %s.\n",
            resolved_name
        );
        exit(-1);
    }

    file_size(resolved_name, fsize);

    boinc_resolve_filename(CHECKPOINT_FILE, resolved_name, sizeof(resolved_name));
    state = fopen(resolved_name, "r");
    if (state) {
        fscanf(state, "%d", &nchars);
        printf("nchars %d\n", nchars);
        fseek(in, nchars, SEEK_SET);
        boinc_resolve_filename("out", resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "a");
    } else {
        boinc_resolve_filename("out", resolved_name, sizeof(resolved_name));
        retval = out.open(resolved_name, "w");
    }
    if (retval) {
        fprintf(stderr, "APP: upper_case output open failed:\n");
        fprintf(stderr, "resolved name %s, retval %d\n", resolved_name, retval);
        perror("open");
        exit(1);
    }
    while (1) {
        c = fgetc(in);

        if (c == EOF) break;
#ifdef BOINC_APP_GRAPHICS
        sprintf(display_buf, "%c -> %c", c, toupper(c));
#endif
        c = toupper(c);
        out._putchar(c);
        nchars++;

        if (cpu_time) {
            n = 0;
            j = 3.14159;
            for(i=0; i<20000000; i++) {
                n++;
                j *= n+j-3.14159;
                j /= (float)n;
            }

            if (n==j) n = 0;
        }

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
    if (random_exit) exit(-10);
    fprintf(stderr, "APP: upper_case ending, wrote %d chars\n", nchars);
    double cur_cpu;
    boinc_wu_cpu_time(cur_cpu);

#ifdef BOINC_APP_GRAPHICS
    boinc_finish_opengl();
#endif
    boinc_finish(0);

    return 0;
}

#ifdef BOINC_APP_GRAPHICS
extern GLuint main_font;

void app_init_gl() {}

bool app_render(int xs, int ys, double time_of_day) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear Screen And Depth Buffer
    glLoadIdentity();                                    // Reset The Current Modelview Matrix
    glColor3f(1,1,1);

    glRasterPos2f(xPos, yPos);
    glPrint(main_font, display_buf);

    xPos += xDelta;
    yPos += yDelta;
    if (xPos < -1 || xPos > 1) xDelta *= -1;
    if (yPos < -1 || yPos > 1) yDelta *= -1;

    glRasterPos2f(-0.9, 0.9);
    glPrint(main_font, "User: %s", uc_aid.user_name);

    glRasterPos2f(-0.9, 0.8);
    glPrint(main_font, "Team: %s", uc_aid.team_name);

    glRasterPos2f(-0.9, 0.7);
    glPrint(main_font, "CPU Time: %f", uc_aid.wu_cpu_time);

    return true;                                        // Everything Went OK
}

#endif

const char *BOINC_RCSID_33ac47a071 = "$Id$";
