create table bossa_app (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(255) not null,
    user_friendly_name  varchar(255) not null,
    long_jobs           tinyint     not null,
    start_url           varchar(255) not null,
    deprecated          tinyint     not null,
    info                text,
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
    more_needed         tinyint     not null,
    npending            smallint    not null,
    nsuccess            smallint    not null,
    nsuccess_needed     smallint    not null,
    primary key(id)
);

create table bossa_job_inst (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    job_id              integer     not null,
    user_id             integer     not null,
    finish_time         integer     not null,
    info                text,
    primary key(id)
);

create table bossa_app_user (
    app_id              integer     not null,
    user_id             integer     not null,
    info                text
);
