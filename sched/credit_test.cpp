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
#define MAX_CLAIMED_CREDIT  1e3
    // Ignore jobs whose claimed (old) credit is greater than this.
    // Rejects jobs with garbage values.
#define MIN_CLAIMED_CREDIT 50.0
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

#define RSC_TYPE_CPU    -1
#define RSC_TYPE_CUDA   -2
#define RSC_TYPE_ATI    -3

// guess (by looking at stderr_out) which type of app processed this job.
// This is needed for jobs from pre-6.10 clients,
// where the client doesn't report this.
// THIS IS PROJECT-SPECIFIC: YOU'LL NEED TO EDIT THIS
//
int get_rsc_type(RESULT& r) {
    if (strstr(r.stderr_out, "CUDA")) return RSC_TYPE_CUDA;
    if (strstr(r.stderr_out, "ATI")) return RSC_TYPE_ATI;
    return RSC_TYPE_CPU;
}

inline const char* rsc_type_name(int t) {
    switch (t) {
    case RSC_TYPE_CPU: return "CPU";
    case RSC_TYPE_CUDA: return "NVIDIA";
    case RSC_TYPE_ATI: return "ATI";
    }
}

struct HOST_APP_VERSION {
    int host_id;
    int app_version_id;     // -1 means anon platform
    AVERAGE pfc;
    AVERAGE et;

    HOST_APP_VERSION() : pfc(50, .01, 10), et(50, .01, 10) {}
};

vector<APP_VERSION> app_versions;
vector<APP> apps;
vector<HOST_APP_VERSION> host_app_versions;
vector<PLATFORM> platforms;
int windows_platformid;
int linux_platformid;
int mac_platformid;
bool accumulate_stats = false;

void read_db() {
    DB_APP app;
    DB_APP_VERSION av;

    while (!app.enumerate("where deprecated=0")) {
        apps.push_back(app);
    }
    while (!av.enumerate("where deprecated=0 order by id desc")) {
        av.pfc_scale_factor = 1;
        //if (strstr(av.plan_class, "cuda")) {
        //    av.pfc_scale_factor = 0.15;
        //}
        app_versions.push_back(av);
    }
    DB_PLATFORM platform;
    while (!platform.enumerate("")) {
        platforms.push_back(platform);
        if (!strcmp(platform.name, "windows_intelx86")) {
            windows_platformid = platform.id;
        }
        if (!strcmp(platform.name, "i686-pc-linux-gnu")) {
            linux_platformid = platform.id;
        }
        if (!strcmp(platform.name, "i686-apple-darwin")) {
            mac_platformid = platform.id;
        }
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

// find CPU app version matching host's OS
// Note: app_versions is sorted by decreasing ID
// so we'll automatically find the newest one
//
APP_VERSION* lookup_av_old(int appid, HOST& host) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (av.appid != appid) continue;
        if (strlen(av.plan_class)) continue;
        if (av.platformid == windows_platformid && strstr(host.os_name, "Microsoft") != NULL ) return &av;
        if (av.platformid == mac_platformid && strstr(host.os_name, "Darwin") != NULL ) return &av;
        if (av.platformid == linux_platformid && strstr(host.os_name, "Linux") != NULL ) return &av;        
    }
    printf(" missing app version app %d type %s\n", appid, host.os_name);
    return 0;
}

// find the newest Windows app version for the given resource type,
// if there is one.
//
APP_VERSION* lookup_av_anon(int appid, int rsc_type) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (av.appid != appid) continue;
        if (av.platformid != windows_platformid) continue;
        switch(rsc_type) {
        case RSC_TYPE_CPU:
            if (strlen(av.plan_class)) continue;
            break;
        case RSC_TYPE_CUDA:
            if (strcmp(av.plan_class, "cuda")) continue;
            break;
        case RSC_TYPE_ATI:
            if (strcmp(av.plan_class, "ati")) continue;
            break;
        }
        return &av;
    }
    printf(" missing app version app %d type %d\n", appid, rsc_type);
    return 0;
}

APP& lookup_app(int id) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];
        if (app.id == id) return app;
    }
    printf("missing app%d\n", id);
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

// update app version scale factors
//
void update_av_scales() {
    unsigned int i, j;
    printf("----- updating scales --------\n");
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];

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
                return;
            }
        }


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

// Compute or estimate peak FLOP count (PFC).
// This is done as follows:
// if new client (reports elapsed time etc.)
//    if anonymous platform
//       user may not have set flops_estimate correctly.
//       So, if it looks like CUDA app (from stderr)
//       use the CUDA average PFC (but don't update the CUDA avg)
//       Otherwise use CPU speed
//    else
//       use ET*flops_est
// else
//    if it looks like CUDA app, use CUDA avg
//    else use CPU
//
bool get_credit(RESULT& r, double& new_claimed_credit) {
    double pfc;
    APP_VERSION* avp = NULL;
    DB_HOST host;
    int rsc_type;

    if (r.elapsed_time && r.flops_estimate && r.app_version_id) {
        // new client
        if (r.app_version_id < 0) {
            // user is using anon platform app.
            // Don't trust the FLOPS estimate.
            // If it's a CPU app, use benchmarks*time.
            // Otherwise use mean PFC for (resource type, app)
            //
            rsc_type = get_rsc_type(r);
            printf("  anonymous platform, rsc type %s\n",
                rsc_type_name(rsc_type)
            );
            if (rsc_type == RSC_TYPE_CPU) {
                lookup_host(host, r.hostid);
                pfc = host.p_fpops * r.elapsed_time;
            } else {
                avp = lookup_av_anon(r.appid, rsc_type);
                if (!avp) {
                    printf(" no version for resource type %s; skipping\n",
                        rsc_type_name(rsc_type)
                    );
                    return false;
                }
                if (avp->pfc.n < 10) {
                    printf(
                        "  app version %d has too few samples %f; skipping\n",
                        avp->id, avp->pfc.n
                    );
                    return false;
                }
                pfc = avp->pfc.get_avg();
                printf("  using mean PFC: %.0fG\n", pfc/1e9);
            }
            printf("  PFC: %.0fG raw credit: %.2f\n",
                pfc/1e9, pfc*COBBLESTONE_SCALE
            );
        } else {
            pfc = r.elapsed_time * r.flops_estimate;
            avp = lookup_av(r.app_version_id);
            printf("  sec: %.0f GFLOPS: %.0f PFC: %.0fG raw credit: %.2f\n",
                r.elapsed_time, r.flops_estimate/1e9, pfc/1e9, pfc*COBBLESTONE_SCALE
            );
            avp->pfc.update(pfc);
        }
    } else {
        // old client
        rsc_type = get_rsc_type(r);
        if (rsc_type != RSC_TYPE_CPU) {
            // ignore GPU jobs since old client doesn't report elapsed time
            
            printf("  old client, GPU app: skipping\n");
            return false;
        }
        printf("  (old client)\n");
        
        lookup_host(host, r.hostid);
        avp = lookup_av_old(r.appid, host);
        if (!avp) return false;
        r.elapsed_time = r.cpu_time;
        r.flops_estimate =  host.p_fpops;
        pfc = r.elapsed_time * r.flops_estimate;
        printf("  sec: %.0f GFLOPS: %.0f PFC: %.0fG raw credit: %.2f\n",
            r.elapsed_time, r.flops_estimate/1e9, pfc/1e9, pfc*COBBLESTONE_SCALE
        );
        avp->pfc.update(pfc);
    }

    APP& app = lookup_app(r.appid);

    double vnpfc = pfc;

    if (avp) {
        vnpfc *= avp->pfc_scale_factor;
        PLATFORM* p = lookup_platform(avp->platformid);
        printf("  version scale (%s %s): %f\n",
            p->name, avp->plan_class, avp->pfc_scale_factor
        );
    }

    // host normalization

    HOST_APP_VERSION& hav = lookup_host_app_version(
        r.hostid, avp?avp->id:-1
    );
    double host_scale = 1;

    // only apply it if have at MIN_HOST_SCALE_SAMPLES
    if (hav.pfc.n >= MIN_HOST_SCALE_SAMPLES) {
        host_scale = avp->pfc.get_avg()/hav.pfc.get_avg();
        // if (host_scale > 1) host_scale = 1;
        printf("  host scale: %f\n", host_scale);
    }
    double claimed_flops = vnpfc * host_scale;
    new_claimed_credit = claimed_flops * COBBLESTONE_SCALE;

    avp->pfc.update(pfc);
    hav.pfc.update(pfc);

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
        "where server_state=%d and outcome=%d and claimed_credit<%f and claimed_credit>%f %s order by id desc limit %d",
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

        double new_claimed_credit;
        if (!get_credit(r, new_claimed_credit)) {
            continue;
        }
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
