// ----------------------------------------------------------------------
//  EXAMPLE: Fermi-Dirac function in C.
//
//  This simple example shows how to use Rappture within a simulator
//  written in C.
// ======================================================================
//  AUTHOR:  Derrick Kearney, Purdue University
//  Copyright (c) 2004-2008  Purdue Research Foundation
//
//  See the file "license.terms" for information on usage and
//  redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
// ======================================================================

//#include "rappture.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "boinc_api.h"
#include "str_util.h"
#include "wrappture.h"

#define RPLIB_OVERWRITE 0
struct RpLibrary{};
RpLibrary* rpLibrary(char*){return NULL;}
void rpGetString(RpLibrary*, const char*, const char**){}
double rpConvertDbl(const char*, const char*, int*){return 0;}
void rpPutString (RpLibrary*, const char*, const char*, int){}
void rpResult(RpLibrary*){}

int main(int argc, char * argv[]) {

    RpLibrary* lib    = NULL;

    const char* data  = NULL;
    //char line[100];

    double T          = 0.0;
    double Ef         = 0.0;
    double E          = 0.0;
    double dE         = 0.0;
    double kT         = 0.0;
    double Emin       = 0.0;
    double Emax       = 0.0;
    double f          = 0.0;

    int err           = 0;

    // create a rappture library from the file filePath
    lib = rpLibrary(argv[1]);

    if (lib == NULL) {
        // cannot open file or out of memory
        printf("FAILED creating Rappture Library\n");
        return(1);
    }


    rpGetString(lib,"input.number(temperature).current",&data);
    T = rpConvertDbl(data, "K", &err);
    if (err) {
        printf ("Error while retrieving input.number(temperature).current\n");
        return(1);
    }


    rpGetString(lib,"input.number(Ef).current",&data);
    Ef = rpConvertDbl(data, "eV", &err);
    if (err) {
        printf ("Error while retrieving input.number(Ef).current\n");
        return(1);
    }

    kT = 8.61734e-5 * T;
    Emin = Ef - 10*kT;
    Emax = Ef + 10*kT;

    E = Emin;
    dE = 0.005*(Emax-Emin);

    rpPutString (   lib,
                    "output.curve(f12).about.label",
                    "Fermi-Dirac Factor",
                    RPLIB_OVERWRITE );
    rpPutString (   lib,
                    "output.curve(f12).xaxis.label",
                    "Fermi-Dirac Factor",
                    RPLIB_OVERWRITE );
    rpPutString (   lib,
                    "output.curve(f12).yaxis.label",
                    "Energy",
                    RPLIB_OVERWRITE );
    rpPutString (   lib,
                    "output.curve(f12).yaxis.units",
                    "eV",
                    RPLIB_OVERWRITE );

    int retval = boinc_run_rappture_app("foobar.exe", "-kT 4.3");
    if (retval == 0) {
        boinc_finish(0);
        rpResult(lib);
    } else {
        boinc_finish(EXIT_CHILD_FAILED);
    }
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    return main(argc, argv);
}
#endif
