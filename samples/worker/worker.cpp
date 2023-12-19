// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// worker - application without BOINC runtime system;
// used for testing wrappers.
//
// worker [--nsecs N] infile outfile
//
// copy infile to outfile, converting to uppercase
// if infile is 'stdin', use stdin; same for stdout
// --nsecs: use about N sec of CPU time
//
// THIS PROGRAM CAN'T USE ANY BOINC CODE.  That's the whole point.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

// do a billion floating-point ops
// (note: I needed to add an arg to this;
// otherwise the MS C++ compiler optimizes away
// all but the first call to it!)
//
static double do_a_giga_flop(int foo) {
    double x = 3.14159*foo;
    int i;
    for (i=0; i<500000000; i++) {
        x += 5.12313123;
        x *= 0.5398394834;
    }
    return x;
}

void copy_uc(FILE* fin, FILE* fout) {
    int c;
    while (1) {
        c = fgetc(fin);
        if (c == EOF) break;
        fputc(toupper(c), fout);
    }
}

int main(int argc, char** argv) {
    FILE* in=0, *out=0;
    int i, nsecs = 0;
    char c;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--nsecs")) {
            nsecs = atoi(argv[++i]);
        }
        if (!in) {
            if (!strcmp(argv[i], "stdin")) {
                in = stdin;
            } else {
                in = fopen(argv[i], "r");
                if (!in) {
                    fprintf(stderr, "missing input file\n");
                    exit(1);
                }
            }
        } else {
            if (!strcmp(argv[i], "stdout")) {
                out = stdout;
            } else {
                out = fopen(argv[i], "w");
                if (!out) {
                    fprintf(stderr, "missing output file\n");
                    exit(1);
                }
            }
        }
    }
    if (!in || !out) {
        fprintf(stderr, "worker: no files specified\n");
        exit(1);
    }

    fprintf(stderr, "worker: starting\n");

    copy_uc(in, out);

    int start = (int)time(0);

    i=0;
    while (time(0) < start+nsecs) {
        do_a_giga_flop(i++);
    }

    fprintf(stderr, "worker: done\n");
    return 0;
}

#ifdef _WIN32
#include <windows.h>

// take a string containing some space separated words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
// Returns argc
// TODO: use strtok here

#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

int parse_command_line(char* p, char** argv) {
    int state = NOT_IN_TOKEN;
    int argc=0;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (isspace(*p)) {
            } else if (*p == '\'') {
                p++;
                argv[argc++] = p;
                state = IN_SINGLE_QUOTED_TOKEN;
                break;
            } else if (*p == '\"') {
                p++;
                argv[argc++] = p;
                state = IN_DOUBLE_QUOTED_TOKEN;
                break;
            } else {
                argv[argc++] = p;
                state = IN_UNQUOTED_TOKEN;
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        }
        p++;
    }
    argv[argc] = 0;
    return argc;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif
