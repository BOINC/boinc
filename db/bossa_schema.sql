create table bossa_app (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    short_name          varchar(255) not null,
    description         varchar(255) not null,
    long_jobs           tinyint     not null,
    display_script      varchar(255) not null,
    backend_script      varchar(255) not null,
    hidden              tinyint     not null,
    min_conf_sum        float       not null,
    min_conf_frac       float       not null,
    max_instances       integer     not null,
    bolt_course_id      integer     not null,
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
    transition_time     integer     not null,
    conf_needed         float       not null,
    canonical_inst_id   integer     not null,
    primary key(id)
) engine=InnoDB;

create table bossa_job_inst (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    job_id              integer     not null,
    user_id             integer     not null,
    finish_time         integer     not null,
    validate_state      integer     not null,
        -- 0 init, 1 valid, 2 invalid
    confidence          float       not null,
    info                text,
    primary key(id)
) engine=InnoDB;

create table bossa_user (
    user_id             integer     not null,
    flags               integer     not null,
    info                text
        -- Project-dependent info about user's ability and performance.
);
