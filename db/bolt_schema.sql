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
    last_view_id    integer         not null,
    mastery         double          not null
);

-- Represents a view of an item;
-- created when we show the item,
-- and finalized when the user clicks on something to leave the page
-- A special view is used to represent the end of a course;
-- its mode is BOLT_MODE_FINISHED.
--
create table bolt_view (
    id              integer         not null auto_increment,
    user_id         integer         not null,
    course_id       integer         not null,
    item_name       varchar(255)    not null,
        -- name of the item
    state           text            not null,
        -- course state
    mode            integer         not null,
        -- distinguishes exercise show/answer
    action          integer         not null,
        -- what the user clicked
    start_time      integer         not null,
    end_time        integer         not null,
    prev_view_id    integer         not null,
        -- for exercise answer views,
        -- this always refers to the original exercise show
    fraction_done   double          not null,
    result_id       integer         not null,
        -- if this was an exercise show, link to result record
    refresh_id      integer         not null,
        -- if unit was flagged for review, link to review record
    primary key (id)
);

-- represents the result of an exercise
--
create table bolt_exercise_result (
    id              integer         not null auto_increment,
    view_id         integer         not null,
        -- the original display of exercise
    score           double          not null,
    response        text            not null,
        -- the query string containing user's responses
    primary key(id)
);

-- represents the result of an exercise set
--
create table bolt_exercise_set_result (
    id              integer         not null auto_increment,
    score           double          not null,
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
