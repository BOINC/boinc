create table volunteer (
    id              integer      not null auto_increment,
    create_time     integer     not null,
    name            char(64) not null,
    password        char(32) not null,
    email_addr      char(100) not null,
    country         char(64) not null,
    skypeid         char(64) not null,
    lang1           char(32) not null,
    lang2           char(32) not null,
    specialties     varchar(255) not null,
    projects        varchar(255) not null,
    availability    char(128) not null,
    voice_ok        tinyint not null,
    text_ok         tinyint not null,
    timezone        integer     not null,
    nratings        integer     not null,
    rating_sum      integer     not null,
    last_online     integer     not null,
    last_check      integer     not null,
    status          integer     not null,
    hide            tinyint not null,
    primary key(id)
);

create table rating (
    volunteerid     integer     not null,
    rating          integer     not null,
    timestamp       integer     not null,
    auth            char(32)    not null,
    comment         text        not null
);

alter table volunteer
    add unique(email_addr),
    add unique(name);

alter table rating
    add index rvt(volunteerid, timestamp),
    add index vauth(volunteerid, auth);
