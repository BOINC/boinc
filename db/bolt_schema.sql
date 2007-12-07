create table bolt_user (
    user_id          integer         not null,
    birth_year      integer         not null,
    sex             smallint        not null,
    attrs           text            not null,
    primary key (user_id)
);

create table bolt_course (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    short_name      varchar(255)    not null,
    name            varchar(255)    not null,
    description     text            not null,
    doc_file        varchar(255)    not null,
    primary key (id)
);

create table bolt_enrollment (
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    last_view       integer         not null,
    fraction_done   double          not null,
    mastery         double          not null,
);

create table bolt_view (
    id              integer         not null auto_increment,
    user_id         integer         not null,
    course_id       integer         not null,
    item_name       varchar(255)    not null,
    state           text            not null,
    mode            integer         not null,
    action          integer         not null,
    start_time      integer         not null,
    end_time        integer         not null,
    previous_view   integer         not null,
    result_id       integer         not null,
    refresh_id      integer         not null,
    primary key (id)
);

create table bolt_result (
    id              integer         not null auto_increment,
    view_id         integer         not null,
    score           double          not null,
    response        text            not null,
    primary key(id)
);

create table bolt_refresh (
    id              integer         not null auto_increment,
    view_id         integer         not null,
    due_time        integer         not null,
    name            varchar(255)    not null,
    state           text            not null,
    primary key (id)
);
