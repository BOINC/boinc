// grant_credit result_name
//
// grant credit for the given result, based on its reported CPU time
//
// TODO: redundancy checking

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "db.h"

#define EXP_DECAY_RATE  (1./(3600*24*7))

int main(int argc, char** argv) {
    RESULT result;
    HOST host;
    USER user;
    int retval;
    double host_speed, credit, deltat;
    time_t now;

    retval = db_open(getenv("BOINC_DB_NAME"), getenv("BOINC_DB_PASSWD"));
    if (retval) exit(1);
    strcpy(result.name, argv[1]);
    retval = db_result_lookup_name(result);
    if (retval) {
        printf("can't find result\n");
        exit(1);
    }
    if (result.state != RESULT_STATE_DONE) {
        printf("result is not done\n");
        exit(1);
    }
    retval = db_host(result.hostid, host);
    if (retval) {
        printf("can't find host\n");
        exit(1);
    }
    retval = db_user(host.userid, user);
    if (retval) {
        printf("can't find user\n");
        exit(1);
    }

    host_speed = host.p_fpops + host.p_iops + host.p_membw/10;
    credit = result.cpu_time*host_speed;

    result.granted_credit = credit;
    retval = db_result_update(result);
    if (retval) {
        printf("can't update result\n");
    }

    now = time(0);
    if (user.expavg_time) {
        deltat = now - user.expavg_time;
        user.expavg_credit *= exp(deltat*EXP_DECAY_RATE);
    }
    user.expavg_credit += credit;
    user.expavg_time = now;
    retval = db_user_update(user);
    if (retval) {
        printf("can't update user\n");
    }

    if (host.expavg_time) {
        deltat = now - host.expavg_time;
        host.expavg_credit *= exp(deltat*EXP_DECAY_RATE);
    }
    host.expavg_credit += credit;
    host.expavg_time = now;
    retval = db_host_update(host);
    if (retval) {
        printf("can't update host\n");
    }
        
}
