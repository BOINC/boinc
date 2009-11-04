#include <stdio.h>
#include <string.h>

#include "sched_config.h"
#include "boinc_db.h"

struct HOST_APP {
    int host_id;
    int app_id;
    AVERAGE vnpfc;
};

struct HOST_APP_VERSION {
    int host_id;
    int app_version_id;
    AVERAGE et;
};

vector<APP_VERSION> app_versions;
vector<APP> apps;
vector<HOST_APP> host_apps;
vector<HOST_APP_VERSION> host_app_versions;

// update app version scale factors
//
void update_av_scales() {
    unsigned int i, j;
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];
        int n=0;

        // find the average PFC of CPU versions

        double sum_avg = 0;
        double sum_n = 0;
        for (j=0; j<app_versions.size(); j++) {
            APP_VERSION& av = app_versions[j];
            if (av.appid != app.id) continue;
            if (av.pfc.n < 10) continue;
            if (strstr(av.plan_class, "cuda")) continue;
            if (strstr(av.plan_class, "ati")) continue;
            sum_avg += av.pfc.recent_mean * av.pfc.n;
            sum_n += av.pfc.n;
            n++;
        }
        if (n < 1) continue;
        double cpu_avg = sum_avg / sum_n;

        // if at least 2 versions have at least 10 samples,
        // update their scale factors
        //
        for (j=0; j<app_versions.size(); j++) {
            APP_VERSION& av = app_versions[j];
            if (av.appid != app.id) continue;
            if (av.pfc.n < 10) continue;
            av.pfc_scale_factor = cpu_avg/av.pfc.recent_mean;
        }
    }
}

void read_db() {
    DB_APP app;
    DB_APP_VERSION av;

    while (!app.enumerate("where deprecated=0")) {
        app.vnpfc.clear();
        apps.push_back(app);
    }
    while (!av.enumerate("where deprecated=0")) {
        av.pfc.clear();
        av.pfc_scale_factor = 1;
        app_versions.push_back(av);
    }
}

APP_VERSION& lookup_av(int id) {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (av.id == id) return av;
    }
    printf("missing app version %d\n", id);
}

APP& lookup_app(int id) {
    unsigned int i;
    for (i=0; i<apps.size(); i++) {
        APP& app = apps[i];
        if (app.id == id) return app;
    }
    printf("missing app%d\n", id);
}

HOST_APP& lookup_host_app(int hostid, int appid) {
    unsigned int i;
    for (i=0; i<host_apps.size(); i++) {
        HOST_APP& ha = host_apps[i];
        if (ha.host_id != hostid) continue;
        if (ha.app_id != appid) continue;
        return ha;
    }
    HOST_APP h;
    h.host_id = hostid;
    h.app_id = appid;
    h.vnpfc.clear();
    host_apps.push_back(h);
    return host_apps.back();
}

void print_average(AVERAGE& a) {
    printf("n %f avg %fG ravg %fG", a.n, a.mean/1e9, a.recent_mean/1e9);
}

void print_avs() {
    unsigned int i;
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION& av = app_versions[i];
        if (!av.pfc.n) continue;
        printf("app %d vers %d (%s) scale %f ",
            av.appid, av.id, av.plan_class, av.pfc_scale_factor
        );
        print_average(av.pfc);
        printf("\n");
    }
}

int main() {
    DB_RESULT r;
    char clause[256];
    int retval;

    retval = config.parse_file();
    if (retval) {printf("no config: %d\n", retval); exit(1);}
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {printf("no db\n"); exit(1);}

    read_db();

    sprintf(clause, "where server_state=%d and outcome=%d order by id desc limit 10000",
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
    );

    int n=0;
    while (!r.enumerate(clause)) {
        if (!r.elapsed_time) {printf("no elapsed_time\n"); continue;}
        if (!r.flops_estimate) {printf("no flops_estimate\n"); continue;}
        if (!r.app_version_id) {printf("no app_version_id\n"); continue;}

        double pfc = r.elapsed_time * r.flops_estimate;
        printf("handling host %d %f s %f Gflops\n",
            r.hostid, r.elapsed_time, r.flops_estimate/1e9
        );
        APP& app = lookup_app(r.appid);


        double vnpfc;
        if (r.app_version_id < 0) {
            // anonymous platform case
            vnpfc = pfc;
            printf(" anonymous platform\n");
        } else {
            // cross-version normalization
            APP_VERSION& av = lookup_av(r.app_version_id);
            vnpfc = pfc * av.pfc_scale_factor;
            printf(" version scale (%s): %f\n", av.plan_class, av.pfc_scale_factor);
            av.pfc.update(pfc);
        }

        // host normalization

        HOST_APP& host_app = lookup_host_app(r.hostid, app.id);
        double host_scale = 1;
        if (host_app.vnpfc.n > 5) {
            host_scale = app.vnpfc.recent_mean/host_app.vnpfc.recent_mean;
            printf(" host scale: %f\n", host_scale);
        }
        double claimed_flops = vnpfc * host_scale;
        double claimed_credit = claimed_flops * 100/86400e9;

        printf(" cc %f claimed %f granted %f\n",
            claimed_credit, r.claimed_credit, r.granted_credit
        );

        app.vnpfc.update(vnpfc);
        host_app.vnpfc.update(vnpfc);

        n++;

        if (n%20 ==0) {
            update_av_scales();
        }
    }
    print_avs();
}
