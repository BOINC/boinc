alter table bossa_app
    add unique(name),
    add unique(short_name);

alter table bossa_job
    add index bj_conf_needed(app_id, calibration, priority_0);

alter table bossa_job_inst
    add index bji_job(job_id),
    add index bji_user(user_id);
