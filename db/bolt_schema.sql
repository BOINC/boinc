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
    state           text            not null
);

create table bolt_view (
    id              integer         not null auto_increment,
    user_id         integer         not null,
    course_id       integer         not null,
    item_name       varchar(255)    not null,
    start_time      integer         not null,
    end_time        integer         not null,
    primary key (id)
);
