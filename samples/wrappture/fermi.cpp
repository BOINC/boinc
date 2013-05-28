// fermi.cpp -- core calculation of Rappture example app-fermi

#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <unistd.h>

int main(int argc, char * argv[]) {
    double T, Ef, E, dE, kT, Emin, Emax, f;
    FILE *fo = fopen("fermi_out.dat", "w");

    if (!fo) {
       fprintf(stderr, "Failed to open output file\n");
       exit(-1);
    }

    // Check args
    if (3 != argc) {
        fprintf(stderr, "Usage: %s T Ef\n", argv[0]);
        exit(-1);
    }

    T = atof(argv[1]);   // in K
    Ef = atof(argv[2]);  // in eV
    kT = 8.61734e-5 * T;
    Emin = Ef - 10.0*kT;
    Emax = Ef + 10.0*kT;

    E = Emin;
    dE = 0.005*(Emax-Emin);
    while (E < Emax) {
        f = 1.0/(1.0 + exp((E - Ef)/kT));
        fprintf(fo, "%f %f\n",f, E);
        fprintf(stdout, "=RAPPTURE-PROGRESS=>%d\n", (int)((E-Emin)/(Emax-Emin)*100));
        E = E + dE;
    }
    fclose(fo);
    return 0;
}
