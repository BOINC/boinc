use BOINC_DB_NAME;

alter table platform
    add unique(name);

alter table app
    add unique(name);

alter table app_version
    add unique(appid, platformid, version_num);

alter table user
    add unique(email_addr),
    add unique(authenticator),
    add index ind_tid (teamid),
    add index user_tot (total_credit),
    add index user_avg (expavg_credit);

alter table team
    add unique(name),
    add index team_avg (expavg_credit),
    add index team_tot (total_credit);

alter table workunit
    add unique(name),
    add index wu_val (appid, need_validate),
    add index wu_retry (appid, retry_check_time),
    add index wu_filedel (file_delete_state),
    add index wu_assim (appid, assimilate_state);

alter table result
    add unique(name),
    add index res_wuid (workunitid),
    add index ind_res_st (server_state),
    add index res_filedel (file_delete_state);

alter table host
    add index host_avg (expavg_credit),
    add index host_tot (total_credit);
