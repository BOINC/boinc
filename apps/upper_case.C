// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// read "in", convert to UC, write to "out"
// command line options:
// -run_slow: app will sleep 1 second after each character, useful for
//            debugging
// -cpu_time: app will chew up some CPU cycles after each character,
//            used for testing CPU time reporting
// -signal:   app will raise SIGHUP signal (for testing signal handler)
//
// This version does one char/second,
// and writes its state to disk every so often

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef BOINC_APP_GRAPHICS
#ifdef __APPLE_CC__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>

    #include "mac_carbon_gl.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <gl\gl.h>            // Header File For The OpenGL32 Library
#include <gl\glu.h>            // Header File For The GLu32 Library
#include <gl\glaux.h>        // Header File For The Glaux Library
#endif

bool app_render(int xs, int ys, double time_of_day);
void renderBitmapString(float x, float y, void *font, char *string);
int DrawGLScene(GLvoid);
#endif

#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "graphics_api.h"

#define CHECKPOINT_FILE "upper_case_state"

char the_char[10];
double xPos=0, yPos=0;
double xDelta=0.03, yDelta=0.07;

int run_slow=0,cpu_time=0,raise_signal=0;
time_t my_start_time;
APP_INIT_DATA uc_aid;

int do_checkpoint(MFILE& mf, int nchars) {
    int retval;
    char resolved_name[512],res_name2[512];
    FILE *app_time, *client_time;

    if (cpu_time) {
        app_time = fopen("../../app.time", "w"), 
        client_time = fopen("../../client.time", "w");
        boinc_get_init_data(uc_aid);
    }
    boinc_resolve_filename("temp", resolved_name, sizeof(resolved_name));
    FILE* f = fopen(resolved_name, "w");
    if (!f) return 1;
    fprintf(f, "%d", nchars);
    fclose(f);

    fprintf(stderr, "APP: upper_case checkpointing\n");

    // hopefully atomic part starts here
    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_filename(CHECKPOINT_FILE, res_name2, sizeof(res_name2));
    retval = boinc_rename(resolved_name, res_name2);
    if (retval) return retval;
    // hopefully atomic part ends here

    if (cpu_time) {
        // print our own information about cpu time
        fprintf(app_time, "%f\n", difftime(time(0), my_start_time));
        fflush(app_time);
        fclose(app_time);

        // print what the client thinks is our cpu time
        fprintf(client_time, "%f\n", uc_aid.wu_cpu_time + boinc_cpu_time());
        fflush(client_time);
        fclose(client_time);
    }

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
    double j;
    char resolved_name[512];
    MFILE out, time_file;
    FILE* state, *in;
    
    my_start_time = time(0);

    strcpy(the_char, "(none)\0");
    retval = boinc_init();
    if (retval) exit(retval);
    
    retval = boinc_init_opengl();
    if (retval) exit(retval);

    boinc_get_init_data(uc_aid);

    boinc_resolve_filename("in", resolved_name, sizeof(resolved_name));
    fprintf(stderr, "APP: upper_case: starting, argc %d\n", argc);
    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: upper_case: argv[%d] is %s\n", i, argv[i]);
        if (!strcmp(argv[i], "-run_slow")) run_slow = 1;
        if (!strcmp(argv[i], "-cpu_time")) cpu_time = 1;
        if (!strcmp(argv[i], "-signal")) raise_signal = 1;
    }
    in = fopen(resolved_name, "r");
    boinc_resolve_filename(CHECKPOINT_FILE, resolved_name, sizeof(resolved_name));
    if (in == NULL) {
        fprintf( stderr, "Couldn't find input file.\n" );
        exit(-1);
    }
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
        fprintf(stderr, "APP: upper_case output open failed %d\n", retval);
        exit(1);
    }
    time_file.open("../../time.xml", "w");
    while (1) {
        c = fgetc(in);
        if (c == EOF) break;
        sprintf(the_char, "%c -> %c", c, toupper(c));
        c = toupper(c);
        out._putchar(c);
        nchars++;

        if (cpu_time) {
            n = 0;
            j = 3.14159;
            for(i=0; i<200000000; i++) {
                n++;
                j *= n+j-3.14159;
                j /= (float)n;
            }

            if (n==j) n = 0;
        }

        if (run_slow) {
            boinc_sleep(1);
        }
        
        if (raise_signal) {
            raise(SIGHUP);
        }

        if (boinc_time_to_checkpoint()) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: upper_case checkpoint failed %d\n", retval);
                exit(1);
            }
            boinc_checkpoint_completed();
        }
    }
    retval = out.flush();
    if (retval) {
        fprintf(stderr, "APP: upper_case flush failed %d\n", retval);
        exit(1);
    }
    fprintf(stderr, "APP: upper_case ending, wrote %d chars\n", nchars);
    time_file.printf("%f\n", boinc_cpu_time());
    time_file.flush();
    time_file.close();
    
    boinc_finish_opengl();
    boinc_finish(0);
    
    return 0;
}

#ifdef BOINC_APP_GRAPHICS
extern GLuint			main_font;

int DrawGLScene(GLvoid)      // Here's Where We Do All The Drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear Screen And Depth Buffer
    glLoadIdentity();                                    // Reset The Current Modelview Matrix
    glColor3f(1,1,1);

    glRasterPos2f(xPos, yPos);
    glPrint(main_font, the_char);

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

    return TRUE;                                        // Everything Went OK
}

#endif
