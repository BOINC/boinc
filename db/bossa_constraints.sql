alter table bossa_app
    add unique(name);

alter table bossa_job
    add unique(name),
    add index bj_more_needed(app, more_needed);

alter table bossa_job_inst
    add index bji_job(job_id),
    add index bji_user(user_id);
