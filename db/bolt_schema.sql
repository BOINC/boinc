create table bolt_user (
    user_id         integer         not null,
    birth_year      integer         not null,
    sex             tinyint         not null,
    flags           integer         not null,
    attrs           text            not null,
        -- project-defined.  Use serialized PHP object
    primary key (user_id)
);

create table bolt_course (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    short_name      varchar(255)    not null,
    name            varchar(255)    not null,
    description     text            not null,
    hidden          tinyint         not null,
    bossa_app_id    integer         not null,
        -- on completion, go to this Bossa app
    primary key (id)
);

create table bolt_enrollment (
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    last_view_id    integer         not null,
        -- ID of last study phase view
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
    phase           integer         not null,
        -- distinguishes study/review/refresh
    action          integer         not null,
        -- what the user clicked to leave page
    start_time      integer         not null,
    end_time        integer         not null,
    prev_view_id    integer         not null,
        -- for exercise answer views;
        -- refers to the original exercise view
    fraction_done   double          not null,
    result_id       integer         not null,
        -- if this was an exercise show, link to result record
    refresh_id      integer         not null,
        -- if unit was flagged for review, link to review record
		-- ?? remove?
    primary key (id)
);

-- represents the result of a single exercise
--
create table bolt_result (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    view_id         integer         not null,
        -- the display of exercise
    item_name       varchar(255)    not null,
    score           double          not null,
    response        text            not null,
        -- the query string containing user's responses
    primary key(id)
);

-- represents the result of a completed exercise set,
-- where "completed" means the last exercise was scored.
-- In theory this could be reconstructed from the individual exercise results,
-- but this table makes it easier for analytics
--
create table bolt_xset_result (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    start_time      integer         not null,
    end_time        integer         not null,
    name            varchar(255)    not null,
        -- logical name of exercise set unit
    score           double          not null,
		-- weighted average score
    view_id         integer         not null,
        -- the view of the answer page of last exercise in set
    primary key(id)
);

-- represents the completion of a select structure
--
create table bolt_select_finished (
    id              integer         not null auto_increment,
    user_id         integer         not null,
    course_id       integer         not null,
    end_time        integer         not null,
    name            varchar(255)    not null,
        -- logical name of the select unit
    selected_unit   varchar(255)    not null,
        -- name of selected subunit
    view_id         integer         not null,
        -- the view of the last item
    primary key(id)
);

-- represents a refresh/repeat of an exercise set,
-- either pending (not due yet),
-- due but not started, or in progress.
--
create table bolt_refresh (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    name            varchar(255)    not null,
        -- logical name of result set unit
    last_view_id    integer         not null,
        -- if refresh is in progress, the last view
    xset_result_id   integer         not null,
        -- most recent result for this exercise set
    due_time        integer         not null,
        -- when refresh will be due
    count           integer         not null,
        -- index into intervals array
    primary key (id)
);

create table bolt_question (
    id              integer         not null auto_increment,
    create_time     integer         not null,
    user_id         integer         not null,
    course_id       integer         not null,
    name            varchar(255)    not null,
		-- logical name of item where question was asked
    mode            integer         not null,
        -- distinguishes exercise show/answer
	question		text			not null,
	state			integer			not null,
		-- whether question has been handled
    primary key (id)
);
