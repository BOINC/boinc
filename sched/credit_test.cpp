// credit_test
//
// Simulate the new credit system for the N most recent jobs
// in project's database, and give a comparison of new and old systems.
// Doesn't modify anything.
//
// You must first run html/ops/credit_test.php to create a data file
//

#include <stdio.h>
#include <string.h>

#include "sched_config.h"
#include "sched_customize.h"
#include "boinc_db.h"

#define MAX_JOBS 100000
#define COBBLESTONE_SCALE 100/86400e9
#define PRINT_AV_PERIOD 100
#define SCALE_AV_PERIOD 20

#define MIN_HOST_SAMPLES  10
    // don't use host scaling unless have this many samples for host
#define MIN_VERSION_SAMPLES   100
    // don't update a version's scale unless it has this many samples,
    // and don't accumulate stats until this occurs

#define HAV_AVG_THRESH  20
#define HAV_AVG_WEIGHT  .01
#define HAV_AVG_LIMIT   10

#define AV_AVG_THRESH   50000
#define AV_AVG_WEIGHT   .005
#define AV_AVG_LIMIT    10

double min_credit = 0;
vector<APP_VERSION> app_versions;
vector<APP> apps;
vector<HOST_APP_VERSION> host_app_versions;
vector<PLATFORM> platforms;
bool accumulate_stats = false;
    // set to true when we have PFC averages for
    // both a GPU and a CPU version

void read_db() {
    DB_APP app;
    DB_APP_VERSION av;

    while (!app.enumerate("")) {
        apps.push_back(app);
    }
    while (!av.enumerate("where deprecated=0 order by id desc")) {
        av.pfc_scale= 1;
        app_versions.push_back(av);
    }
    DB_PLATFORM platform;
    while (!platform.enumerate("")) {
        platforms.push_back(platform);
    }
}

PLATFORM* lookup_platform(int id) {
    unsigned int i;
    for (i=0; i<platforms.size(); i++) {
        PLATFORM& p = platforms[i];
        if (p.id == id) return &p;
    }
    return NULL;
}

APP_VERSION* lookup_av(int id) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (av.id == id) return &av;
    }
    printf(" missing app version %d\n", id);
    exit(1);
}

APP& lookup_app(int id) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];
        if (app.id == id) return app;
    }
    printf("missing app: %d\n", id);
}

HOST_APP_VERSION& lookup_host_app_version(int hostid, int avid) {
    unsigned int i;
    for (i=0; i<host_app_versions.size(); i++) {
        HOST_APP_VERSION& hav = host_app_versions[i];
        if (hav.host_id != hostid) continue;
        if (hav.app_version_id != avid) continue;
        return hav;
    }
    HOST_APP_VERSION h;
    h.host_id = hostid;
    h.app_version_id = avid;
    host_app_versions.push_back(h);
    return host_app_versions.back();
}

void print_average(AVERAGE& a) {
    printf("n %f avg %f\n", a.n, a.get_avg()
    );
}

void print_avs() {
    unsigned int i;
    printf("----- scales --------\n");
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (!av.pfc.n) continue;
        PLATFORM* p = lookup_platform(av.platformid);
        printf("app %d vers %d (%s %s)\n scale %f ",
            av.appid, av.id, p->name, av.plan_class, av.pfc_scale
        );
        print_average(av.pfc);
        printf("\n");
    }
    printf("-------------\n");
}

void lookup_host(DB_HOST& h, int id) {
    int retval = h.lookup_id(id);
    if (retval) {
        printf("can't find host %d\n", id);
        exit(1);
    }
}

// used in the computation of AV scale factors
//
struct RSC_INFO {
    double pfc_sum;
    double pfc_n;
    int nvers_thresh;   // # app versions w/ lots of samples
    int nvers_total;

    RSC_INFO() {
        pfc_sum = 0;
        pfc_n = 0;
        nvers_thresh = 0;
        nvers_total = 0;
    }
    void update(APP_VERSION& av) {
        nvers_total++;
        if (av.pfc.n > MIN_VERSION_SAMPLES) {
            nvers_thresh++;
            pfc_sum += av.pfc.get_avg() * av.pfc.n;
            pfc_n += av.pfc.n;
        }
    }
    double avg() {
        return pfc_sum/pfc_n;
    }
};

void scale_versions(APP& app, double avg) {
    for (unsigned int j=0; j<app_versions.size(); j++) {
        APP_VERSION& av = app_versions[j];
        if (av.appid != app.id) continue;
        if (av.pfc.n < MIN_VERSION_SAMPLES) continue;

        av.pfc_scale= avg/av.pfc.get_avg();
        PLATFORM* p = lookup_platform(av.platformid);
        printf("updating scale factor for (%s %s)\n",
             p->name, av.plan_class
        );
        printf(" n: %f avg PFC: %f new scale: %f\n",
            av.pfc.n, av.pfc.get_avg(), av.pfc_scale
        );
    }
    app.min_avg_pfc = avg;
}

// update app version scale factors,
// and find the min average PFC for each app
//
void update_av_scales() {
    unsigned int i, j;
    printf("----- updating scales --------\n");
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];
        printf("app %d\n", app.id);
        RSC_INFO cpu_info, gpu_info;

        // find the average PFC of CPU and GPU versions

        for (j=0; j<app_versions.size(); j++) {
            APP_VERSION& av = app_versions[j];
            if (av.appid != app.id) continue;
            if (strstr(av.plan_class, "cuda") || strstr(av.plan_class, "ati")) {
                printf("gpu update: %d %s %f\n", av.id, av.plan_class, av.pfc.get_avg());
                gpu_info.update(av);
            } else {
                printf("cpu update: %d %s %f\n", av.id, av.plan_class, av.pfc.get_avg());
                cpu_info.update(av);
            }
        }

        // If there are only CPU or only GPU versions,
        // and 2 are above threshold, normalize to the average
        //
        // If there are both, and at least 1 of each is above threshold,
        // normalize to the min of the averages
        //
        if (cpu_info.nvers_total) {
            if (gpu_info.nvers_total) {
                if (cpu_info.nvers_thresh && gpu_info.nvers_thresh) {
                    printf("CPU avg: %f\n", cpu_info.avg());
                    printf("GPU avg: %f\n", gpu_info.avg());
                    scale_versions(app,
                        cpu_info.avg()<gpu_info.avg()?cpu_info.avg():gpu_info.avg()
                    );
                    accumulate_stats = true;
                }
            } else {
                if (cpu_info.nvers_thresh > 1) {
                    scale_versions(app, cpu_info.avg());
                    accumulate_stats = true;
                }
            }
        } else {
            if (gpu_info.nvers_thresh > 1) {
                scale_versions(app, gpu_info.avg());
                accumulate_stats = true;
            }
        }


    }
    printf("-------------\n");
}

// Compute or estimate normalized peak FLOP count (PFC),
// and update data structures.
// Return true if the PFC was computed in the "normal" way,
// i.e. not anon platform, and reflects version scaling
//
bool get_pfc(RESULT& r, WORKUNIT& wu, double& pfc) {
    APP_VERSION* avp = NULL;
    DB_HOST host;
    int rsc_type;

    APP& app = lookup_app(r.appid);
    HOST_APP_VERSION& hav = lookup_host_app_version(
        r.hostid, r.app_version_id
    );

    if (r.elapsed_time) {
        // new client
        hav.et.update(
            r.elapsed_time/wu.rsc_fpops_est,
            HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
        );
        if (r.app_version_id < 0) {
            // anon platform
            //
            pfc = app.min_avg_pfc;
            if (hav.et.n > MIN_HOST_SAMPLES) {
                pfc *= (r.elapsed_time/wu.rsc_fpops_est)/hav.et.get_avg();
            }
            printf("  skipping: anon platform\n");
            return false;
        } else {
            pfc = (r.elapsed_time * r.flops_estimate);
            avp = lookup_av(r.app_version_id);
            printf("  sec: %.0f GFLOPS: %.0f PFC: %.0fG raw credit: %.2f\n",
                r.elapsed_time, r.flops_estimate/1e9, pfc/1e9, pfc*COBBLESTONE_SCALE
            );
        }
    } else {
        // old client
        //
        hav.et.update(
            r.cpu_time/wu.rsc_fpops_est,
            HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
        );
        pfc = app.min_avg_pfc*wu.rsc_fpops_est;
        if (hav.et.n > MIN_HOST_SAMPLES) {
            double s = r.elapsed_time/hav.et.get_avg();
            pfc *= s;
            printf("  old client: scaling by %f (%f/%f)\n",
                s, r.elapsed_time, hav.et.get_avg()
            );
        } else {
            printf("  old client: not scaling\n");
        }
        return false;
    }

    avp->pfc.update(
        pfc/wu.rsc_fpops_est,
        AV_AVG_THRESH, AV_AVG_WEIGHT, AV_AVG_LIMIT
    );

    // version normalization

    double vnpfc = pfc * avp->pfc_scale;

    PLATFORM* p = lookup_platform(avp->platformid);
    printf("  updated version PFC: %f\n", pfc/wu.rsc_fpops_est);
    printf("  version scale (%s %s): %f\n",
        p->name, avp->plan_class, avp->pfc_scale
    );

    // host normalization

    hav.pfc.update(
        pfc/wu.rsc_fpops_est,
        HAV_AVG_THRESH, HAV_AVG_WEIGHT, HAV_AVG_LIMIT
    );

    double host_scale = 1;

    if (hav.pfc.n > MIN_HOST_SAMPLES && avp->pfc.n > MIN_VERSION_SAMPLES) {
        host_scale = avp->pfc.get_avg()/hav.pfc.get_avg();
        if (host_scale > 1) host_scale = 1;
        printf("  host scale: %f (%f/%f)\n",
            host_scale, avp->pfc.get_avg(), hav.pfc.get_avg()
        );
    }
    pfc = vnpfc * host_scale;

    return avp->pfc.n > MIN_VERSION_SAMPLES;
}

int main(int argc, char** argv) {
    RESULT r;
    WORKUNIT wu;
    int retval;
    int appid=0;
    FILE* f = fopen("credit_test_unsorted", "w");

    if (argc > 1) {
        min_credit = atof(argv[1]);
    }

    retval = config.parse_file();
    if (retval) {printf("no config: %d\n", retval); exit(1);}
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {printf("no db\n"); exit(1);}

    read_db();

    int n=0, nstats=0;
    double total_old_credit = 0;
    double total_new_credit = 0;
    FILE* in = fopen("credit_test_data", "r");
    printf("min credit: %f\n", min_credit);
    while (!feof(in)) {
        int c = fscanf(in, "%d %d %d %d %lf %d %lf %lf %lf %lf",
            &r.id, &r.workunitid, &r.appid, &r.hostid,
            &r.claimed_credit, &r.app_version_id, &r.elapsed_time,
            &r.flops_estimate, &r.cpu_time, &wu.rsc_fpops_est
        );
        if (c != 10) break;
        printf("%d) result %d WU %d host %d old credit %f\n",
            n, r.id, r.workunitid, r.hostid, r.claimed_credit
        );
        n++;
        if (r.claimed_credit < min_credit) {
            printf("  skipping: small credit\n");
            continue;
        }

        double pfc;
        bool normal = get_pfc(r, wu, pfc);
        double new_claimed_credit = pfc * COBBLESTONE_SCALE;
        if (normal) {
            printf(" new credit %.2f old credit %.2f\n",
                new_claimed_credit, r.claimed_credit
            );
            if (accumulate_stats) {
                total_old_credit += r.claimed_credit;
                total_new_credit += new_claimed_credit;
                nstats++;
                fprintf(f, "%d %d %.2f %.2f\n",
                    r.workunitid, r.id, new_claimed_credit, r.claimed_credit
                );
            } else {
                printf("  not accumulated\n");
            }
        } else {
            printf(" new credit (average): %f\n", new_claimed_credit);
        }

        if (n%SCALE_AV_PERIOD ==0) {
            update_av_scales();
        }
        if (n%PRINT_AV_PERIOD ==0) {
            print_avs();
        }
        if (n%1000 == 0) {
            fprintf(stderr, "%d\n", n);
        }
        if (n >= MAX_JOBS) break;
    }
    fclose(f);
    if (nstats == 0) {
        printf("Insufficient jobs were read from DB\n");
        exit(0);
    }

    print_avs();

    printf("Average credit: old %.2f new %.2f (ratio %.2f)\n",
        total_old_credit/nstats, total_new_credit/nstats,
        total_new_credit/total_old_credit
    );
    //printf("Variance claimed to grant old credit: %f\n", sqrt(variance_old/nstats));
    //printf("Variance claimed to grant old credit: %f\n", sqrt(variance_old/nstats));
}
