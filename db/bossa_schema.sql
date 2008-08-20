create table bossa_app (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    short_name          varchar(255) not null,
    description         varchar(255) not null,
    long_jobs           tinyint     not null,
    hidden              tinyint     not null,
    bolt_course_id      integer     not null,
    time_estimate       integer     not null,
    time_limit          integer     not null,
    calibration_frac    double      not null,
    info                text,
        -- app-specific info, JSON
    primary key(id)
) engine = InnoDB;

create table bossa_job (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    app_id              integer     not null,
    batch_id            integer     not null,
    state               integer     not null,
    info                text,
    calibration         tinyint     not null,
    priority_0          double      not null,
        -- add more as needed
        -- for calibration jobs, init to random and decrement on each view
    primary key(id)
) engine=InnoDB;

create table bossa_job_inst (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    app_id              integer     not null,
    job_id              integer     not null,
    user_id             integer     not null,
    batch_id            integer     not null,
    finish_time         integer     not null,
    timeout             integer     not null,
    calibration         tinyint     not null,
    info                text,
    primary key(id)
) engine=InnoDB;

create table bossa_user (
    user_id             integer     not null,
    category            integer     not null,
    flags               integer     not null,
        -- debug, show_all
    info                text,
        -- Project-dependent info about users ability and performance.
    primary key(user_id)
) engine = InnoDB;

create table bossa_batch (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    app_id              integer     not null,
    calibration         tinyint     not null,
    primary key(id)
) engine = InnoDB;
