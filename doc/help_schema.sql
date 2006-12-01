create table volunteer (
    id              integer      not null auto_increment,
    create_time     integer     not null,
    name            varchar(254) not null,
    password        varchar(254) not null,
    email_addr      varchar(254) not null,
    country         varchar(254) not null,
    skypeid         varchar(254) not null,
    lang1           varchar(254) not null,
    lang2           varchar(254) not null,
    specialties     varchar(254) not null,
    projects        varchar(254) not null,
    availability    varchar(254) not null,
    voice_ok        tinyint not null,
    text_ok         tinyint not null,
    timezone        integer     not null,
    primary key(id)
);

create table rating (
    volunteerid     integer     not null,
    rating          integer     not null,
    timestamp       integer     not null,
    comment         text        not null
);

alter table volunteer
    add unique(email_addr),
    add unique(name);
