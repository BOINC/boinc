#include <stdio.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "error_numbers.h"
#include "sched_msgs.h"

#include "hr_info.h"

int HR_INFO::write_file() {
    int i, j;

    FILE* f = fopen(HR_INFO_FILENAME, "w");
    if (!f) return ERR_FOPEN;
    for (i=1; i<HR_NTYPES; i++) {
        fprintf(f, "--------- %s ----------\n", hr_names[i]);
        for (j=0; j<hr_nclasses[i]; j++) {
            fprintf(f, "%d %f\n", j, rac_per_class[i][j]);
        }
    }
    fclose(f);
    return 0;
}

int HR_INFO::read_file() {
    char buf[256];
    FILE* f = fopen(HR_INFO_FILENAME, "r");
    if (!f) return ERR_FOPEN;
    int i, j, jj;
    double x;

    for (i=1; i<HR_NTYPES; i++) {
        char* p = fgets(buf, sizeof(buf), f);
        if (!p) {
            fprintf(stderr, "missing delimeter line in HR info");
            exit(1);
        }
        for (j=0; j<hr_nclasses[i]; j++) {
            char* p = fgets(buf, sizeof(buf), f);
            if (!p) {
                fprintf(stderr, "missing data line in HR info");
                exit(1);
            }
            int n = sscanf(buf, "%d %lf", &jj, &x);
            if (n!=2 || j!=jj) {
                fprintf(stderr, "bad line %s in HR info: %d; %d != %d\n", buf, n, j, jj);
                exit(1);
            }
            rac_per_class[i][j] = x;
        }
    }
    fclose(f);
    return 0;
}

void HR_INFO::init() {
    int i;
    for (i=1; i<HR_NTYPES; i++) {
        rac_per_class[i] = (double*) calloc(hr_nclasses[i], sizeof(double));
        max_slots[i] = (int*) calloc(hr_nclasses[i], sizeof(int));
        cur_slots[i] = (int*) calloc(hr_nclasses[i], sizeof(int));
    }
}

void HR_INFO::scan_db() {
    DB_HOST host;
    int retval;
    int i;

    while (1) {
        retval = host.enumerate("where expavg_credit>1");
        if (retval) break;
        //printf("host %d: %s | %s | %s\n", host.id, host.os_name, host.p_vendor, host.p_model);
        for (i=1; i<HR_NTYPES; i++) {
            if (hr_unknown_platform_type(host, i)) {
                //printf("type %d: unknown\n", i);
                continue;
            }
            int hrc = hr_class(host, i);
            //printf("type %d: class %d\n", i, hrc);
            if (!hrc) continue;
            rac_per_class[i][hrc] += host.expavg_credit;
        }
    }
    if (retval != ERR_DB_NOT_FOUND) {
        fprintf(stderr, "host enum: %d", retval);
        exit(1);
    }
}

void HR_INFO::allocate(int total_slots) {
    int i, j;

    double weight_total = 0;
    for (i=1; i<HR_NTYPES; i++) {
        weight_total += type_weights[i];
    }

    // decide how many slots to allocate to each HR type
    //
    for (i=1; i<HR_NTYPES; i++) {
        slots_per_type[i] = (int)(total_slots*type_weights[i]/weight_total);
    }

    // within each type, decide how many slots to allocate to each class
    //
    for (i=1; i<HR_NTYPES; i++) {
        int nuncom = slots_per_type[i]/2;
        int ncom = slots_per_type[i] - nuncom;
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "nu %d nc %d spt %d\n", nuncom, ncom, slots_per_type[i]
        );

        double total_rac = 0;
        for (j=1; j<hr_nclasses[i]; j++) {
            total_rac += rac_per_class[i][j];
        }
        max_slots[i][0] = nuncom;
        for (j=1; j<hr_nclasses[i]; j++) {
            int n = (int)(ncom*rac_per_class[i][j]/total_rac);

            // every HR class has a max of at least one,
            // so that new classes can get "on the board".
            // This means that the sum of the maxes can exceed the # of slots
            //
            if (n == 0) n = 1;
            max_slots[i][j] = n;
            if (n > 1) {
                log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                    "type %d class %d: allocating %d slots\n", i, j, n
                );
            }
        }
        for (j=0; j<hr_nclasses[i]; j++) {
            cur_slots[i][j] = 0;
        }
    }
}

// Decide if job of the given HR type and class should be added to array,
// and if to update counts
//
bool HR_INFO::accept(int hrt, int hrc) {
    if (cur_slots[hrt][hrc] >= max_slots[hrt][hrc]) {
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "skipping job because HR class (%d, %d) full\n", hrt, hrc
        );
        return false;
    }
    cur_slots[hrt][hrc]++;
}

void HR_INFO::show(FILE* f) {
    for (int ht=1; ht<HR_NTYPES; ht++) {
        fprintf(f, "HR type %s: weight %f nslots %d\n",
            hr_names[ht], type_weights[ht], slots_per_type[ht]
        );
        for (int hc=0; hc<hr_nclasses[ht]; hc++) {
            if (hc && rac_per_class[ht][hc] == 0) continue;
            fprintf(f,
                "  class %d: rac %f max_slots %d cur_slots %d\n",
                hc, rac_per_class[ht][hc], max_slots[ht][hc], cur_slots[ht][hc]
            );
        }
    }
}
