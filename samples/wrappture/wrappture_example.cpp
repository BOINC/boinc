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

#include "rappture.h"

#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "boinc_api.h"
#include "str_util.h"
#include "filesys.h"
#include "wrappture.h"

int main(int, char**) {
    char buf[256];

    RpLibrary* lib    = NULL;

    const char* data  = NULL;

    double T          = 0.0;
    double Ef         = 0.0;
    double E          = 0.0;
    double dE         = 0.0;
    double kT         = 0.0;
    double Emin       = 0.0;
    double Emax       = 0.0;

    int err           = 0;

    boinc_resolve_filename("driver.xml", buf, sizeof(buf));
    lib = rpLibrary(buf);

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

    // Run core simulator
    sprintf(buf, "%g %g", T, Ef);
    int retval = boinc_run_rappture_app("fermi", buf);
    if (retval) {
        fprintf(stderr, "boinc_run_rappture_app(): %d\n", retval);
        boinc_finish(EXIT_CHILD_FAILED);
    }

    // Read resulting output file
    FILE* file;
    if (!(file = boinc_fopen("fermi_out.dat", "r"))) {
       fprintf(stderr, "Unable to open data file\n");
       exit(-1);
    }
    while (fgets(buf, sizeof(buf), file)) {
       rpPutString(lib, "output.curve(f12).component.xy", buf, RPLIB_APPEND);
    }
    fclose(file);

    // Finish
    rpResult(lib);
    boinc_finish(0);
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line(command_line, argv);
    return main(argc, argv);
}
#endif
