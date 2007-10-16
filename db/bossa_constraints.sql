alter table bossa_app
    add unique(name);

alter table bossa_job
    add unique(name)
    add index bj_more_needed(more_needed);
