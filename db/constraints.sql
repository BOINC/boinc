use BOINC_DB_NAME;

alter table platform
    add unique(name);

alter table app
    add unique(name);

alter table app_version
    add unique(appid, platformid, version_num);

alter table user
    add unique(email_addr);
alter table user
    add unique(authenticator);
alter table user
    add index ind_tid (teamid);
create index user_avg on user(expavg_credit);

alter table team
    add unique(name);

alter table workunit
    add unique(name);
create index wu_val on workunit(appid, need_validate);
create index wu_retry on workunit(appid, retry_check_time);
create index wu_filedel on workunit(file_delete_state);
create index wu_assim on workunit(appid, assimilate_state);

alter table result
    add unique(name);
create index res_wuid on result(workunitid);
create index ind_res_st on result(server_state);
create index res_filedel on result(file_delete_state);

create index host_avg on host(expavg_credit);
