use BOINC_DB_NAME;

alter table platform
    add unique(name);

alter table app
    add unique(name);

alter table app_version
    add unique(appid, platformid, version_num);

alter table user
    add unique(email_addr);

alter table team
    add unique(name);

alter table workunit
    add unique(name);

alter table result
    add unique(name);

create index ind_res_st on result(state);
