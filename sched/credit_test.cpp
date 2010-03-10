// credit_test [--app N]
//
// Simulate the new credit system for the N most recent jobs
// in project's database, and give a comparison of new and old systems.
// Doesn't modify anything.
//
// --app N: restrict to jobs from app with ID N

#include <stdio.h>
#include <string.h>

#include "sched_config.h"
#include "sched_customize.h"
#include "boinc_db.h"

#define NJOBS   100000
    // scan this many jobs
#define MAX_CLAIMED_CREDIT  1e4
    // Ignore jobs whose claimed (old) credit is greater than this.
    // Rejects jobs with garbage values.
#define MIN_CLAIMED_CREDIT 10.0
    // Ignore jobs whose claimed (old) credit is less than this.
    // Small jobs are noisy.
#define COBBLESTONE_SCALE 100/86400e9
    // FLOPS to cobblestones
#define PRINT_AV_PERIOD 100
#define SCALE_AV_PERIOD 20
#define MIN_HOST_SCALE_SAMPLES  10
    // don't use host scaling unless have this many samples for host
#define MIN_SCALE_SAMPLES   100
    // don't update a version's scale unless it has this many samples,
    // and don't accumulate stats until this occurs

struct HOST_APP_VERSION {
    int host_id;
    int app_version_id;     // 0 unknown, -1 anon platform
    int app_id;     // if unknown or anon platform
    AVERAGE pfc;
    AVERAGE et;
};

vector<APP_VERSION> app_versions;
vector<APP> apps;
vector<HOST_APP_VERSION> host_app_versions;
vector<PLATFORM> platforms;
bool accumulate_stats = false;
    // set to true after warm-up period

void read_db() {
    DB_APP app;
    DB_APP_VERSION av;

    while (!app.enumerate("")) {
        apps.push_back(app);
    }
    while (!av.enumerate("where deprecated=0 order by id desc")) {
        av.pfc.init(5000, .005, 10);
        av.pfc_scale_factor = 1;
        //if (strstr(av.plan_class, "cuda")) {
        //    av.pfc_scale_factor = 0.15;
        //}
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
    h.pfc.init(50, .01, 10);
    h.et.init(50, .01, 10);
    host_app_versions.push_back(h);
    return host_app_versions.back();
}

void print_average(AVERAGE& a) {
    printf("n %.0f avg PFC %.0fG avg raw credit %.2f",
        a.n, a.get_avg()/1e9, a.get_avg()*COBBLESTONE_SCALE
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
            av.appid, av.id, p->name, av.plan_class, av.pfc_scale_factor
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

// update app version scale factors,
// and find the min average PFC for each app
//
void update_av_scales() {
    unsigned int i, j;
    printf("----- updating scales --------\n");
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];

        app.min_avg_pfc = 1e9;

        // find the average PFC of CPU and GPU versions

        double cpu_pfc_sum = 0;
        double cpu_pfc_n = 0;
        int cpu_n = 0;
        double gpu_pfc_sum = 0;
        double gpu_pfc_n = 0;
        int gpu_n = 0;
        for (j=0; j<app_versions.size(); j++) {
            APP_VERSION& av = app_versions[j];
            if (av.appid != app.id) continue;
            if (av.pfc.n < MIN_SCALE_SAMPLES) continue;
            if (strstr(av.plan_class, "cuda") || strstr(av.plan_class, "ati")) {
                gpu_pfc_sum += av.pfc.get_avg() * av.pfc.n;
                gpu_pfc_n += av.pfc.n;
                gpu_n++;
            } else {
                cpu_pfc_sum += av.pfc.get_avg() * av.pfc.n;
                cpu_pfc_n += av.pfc.n;
                cpu_n++;
            }
        }
        double cpu_avg, gpu_avg;
        if (cpu_n) {
            cpu_avg = cpu_pfc_sum / cpu_pfc_n;
            printf("CPU avg is %0.fG\n", cpu_avg/1e9);
        }
        if (gpu_n) {
            gpu_avg = gpu_pfc_sum / gpu_pfc_n;
            printf("GPU avg is %0.fG\n", gpu_avg/1e9);
        }
        double min_avg;
        if (cpu_n) {
            if (gpu_n) {
                min_avg = (cpu_avg < gpu_avg)?cpu_avg:gpu_avg;
            } else {
                min_avg = cpu_avg;
            }
        } else {
            if (gpu_n) {
                min_avg = gpu_avg;
            } else {
                continue;
            }
        }

        app.min_avg_pfc = min_avg;

        // update scale factors
        //
        for (j=0; j<app_versions.size(); j++) {
            APP_VERSION& av = app_versions[j];
            if (av.appid != app.id) continue;
            if (av.pfc.n < MIN_SCALE_SAMPLES) continue;

            av.pfc_scale_factor = min_avg/av.pfc.get_avg();
            PLATFORM* p = lookup_platform(av.platformid);
            printf("updating scale factor for (%s %s)\n",
                 p->name, av.plan_class
            );
            printf(" n: %0.f avg PFC: %0.fG new scale: %f\n",
                av.pfc.n, av.pfc.get_avg()/1e9, av.pfc_scale_factor
            );
        }
        accumulate_stats = true;
    }
    printf("-------------\n");
}

// Compute or estimate normalized peak FLOP count (PFC),
// and update data structures.
// Return false if the PFC is an average.
//
bool get_pfc(RESULT& r, WORKUNIT& wu, double& pfc) {
    APP_VERSION* avp = NULL;
    DB_HOST host;
    int rsc_type;

    APP& app = lookup_app(r.appid);
    HOST_APP_VERSION& hav = lookup_host_app_version(
        r.hostid, r.app_version_id
    );

    if (r.elapsed_time && r.flops_estimate && r.app_version_id) {
        // new client
        hav.et.update(r.elapsed_time);
        if (r.app_version_id < 0) {
            // anon platform
            //
            pfc = app.min_avg_pfc;
            if (hav.et.n > MIN_HOST_SCALE_SAMPLES) {
                pfc *= r.elapsed_time/hav.et.get_avg();
            }
            printf("  skipping: anon platform\n");
            return false;
        } else {
            pfc = (r.elapsed_time * r.flops_estimate)/wu.rsc_fpops_est;
            avp = lookup_av(r.app_version_id);
            printf("  sec: %.0f GFLOPS: %.0f PFC: %.0fG raw credit: %.2f\n",
                r.elapsed_time, r.flops_estimate/1e9, pfc/1e9, pfc*COBBLESTONE_SCALE
            );
        }
    } else {
        // old client
        //
        hav.et.update(r.cpu_time);
        pfc = app.min_avg_pfc;
        if (hav.et.n > MIN_HOST_SCALE_SAMPLES) {
            pfc *= r.elapsed_time/hav.et.get_avg();
        }
        printf("  skipping: old client\n");
        return false;
    }

    avp->pfc.update(pfc);

    // version normalization

    double vnpfc = pfc * avp->pfc_scale_factor;

    PLATFORM* p = lookup_platform(avp->platformid);
    printf("  version scale (%s %s): %f\n",
        p->name, avp->plan_class, avp->pfc_scale_factor
    );

    // host normalization

    hav.pfc.update(pfc);

    double host_scale = 1;

    // only apply it if have at MIN_HOST_SCALE_SAMPLES
    //
    if (hav.pfc.n >= MIN_HOST_SCALE_SAMPLES) {
        host_scale = avp->pfc.get_avg()/hav.pfc.get_avg();
        // if (host_scale > 1) host_scale = 1;
        printf("  host scale: %f\n", host_scale);
    }
    pfc = vnpfc * host_scale;

    return true;
}

int main(int argc, char** argv) {
    DB_RESULT r;
    char clause[256], subclause[256];
    int retval;
    int appid=0;
    FILE* f = fopen("credit_test_unsorted", "w");

    if (argc >= 3 && !strcmp(argv[1], "--app")) {
        appid = atoi(argv[2]);
    }

    retval = config.parse_file();
    if (retval) {printf("no config: %d\n", retval); exit(1);}
    //strcpy(config.db_host, "jocelyn");
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {printf("no db\n"); exit(1);}

    read_db();

    strcpy(subclause, "");
    if (appid) {
        sprintf(subclause, "and appid=%d", appid);
    }

    sprintf(clause,
        "where server_state=%d and outcome=%d"
        " and claimed_credit<%f and claimed_credit>%f"
        " %s "
        //" order by id desc "
        " limit %d",
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS,
        MAX_CLAIMED_CREDIT, MIN_CLAIMED_CREDIT,
        subclause, NJOBS
    );

    int n=0, nstats=0;
    double total_old_credit = 0;
    double total_new_credit = 0;
    printf("DB query: select * from result %s\n", clause);
    while (!r.enumerate(clause)) {
        printf("%d) result %d WU %d host %d\n",
            n, r.id, r.workunitid, r.hostid
        );
        DB_WORKUNIT wu;
        retval = wu.lookup_id(r.workunitid);
        if (retval) {
            printf("  No WU!\n");
            continue;
        }

        double pfc;
        if (!get_pfc(r, wu, pfc)) {
            continue;
        }
        double new_claimed_credit = pfc * COBBLESTONE_SCALE;
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
        }

        n++;

        if (n%SCALE_AV_PERIOD ==0) {
            update_av_scales();
        }
        if (n%PRINT_AV_PERIOD ==0) {
            print_avs();
        }
        if (n%1000 == 0) {
            fprintf(stderr, "%d\n", n);
        }
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
