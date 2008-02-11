create table bossa_app (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    description         varchar(255) not null,
    long_jobs           tinyint     not null,
    display_script      varchar(255) not null,
    backend_script      varchar(255) not null,
    hidden              tinyint     not null,
    min_conf_sum        double      not null,
    min_conf_frac       double      not null,
    max_instances       integer     not null,
    info                text,
        -- app-specific info, JSON
    primary key(id)
);

create table bossa_job (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    app_id              integer     not null,
    info                text,
    batch               integer     not null,
    time_estimate       integer     not null,
    time_limit          integer     not null,
    transition_time     double      not null,
    nneeded             integer     not null,
    primary key(id)
) engine=InnoDB;

create table bossa_job_inst (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    job_id              integer     not null,
    user_id             integer     not null,
    finish_time         integer     not null,
    info                text,
    primary key(id)
) engine=InnoDB;

create table bossa_app_user (
    app_id              integer     not null,
    user_id             integer     not null,
    info                text
);
