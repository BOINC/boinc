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

// worker - application without BOINC runtime system;
// used for testing wrapper
//
// THIS PROGRAM SHOULDN'T USE ANY BOINC CODE.  That's the whole point.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int, char**) {
    FILE* in = fopen("in", "r");
    FILE* out = fopen("out", "w");
    char buf[256];

    fprintf(stderr, "worker starting\n");
    fflush(stderr);
    if (!in) {
        fprintf(stderr, "no input file\n");
        exit(1);
    }
    if (!out) {
        fprintf(stderr, "no output file\n");
        exit(1);
    }
    fgets(buf, 256, in);
    fputs(buf, out);
    fflush(out);
    fclose(in);
    int start = time(0);
    while (time(0) < start+10) {
        double x=5;
        for (int i=0; i<10000000; i++) {
            x /= 2.1;
            x += 1.5;
        }
    }
    fputs("done!\n", out);
    fclose(out);
    return 0;
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
