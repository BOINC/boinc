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

alter table team
    add unique(name);

alter table workunit
    add unique(name);
create index wu_val on workunit(appid, need_validate);
create index wu_retry on workunit(appid, retry_check_time);

alter table result
    add unique(name);
create index res_wuid on result(workunitid);
create index ind_res_st on result(state);
