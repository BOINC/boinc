/*
    If you add/change tables:
    - if used by C++ code, update
        db/
            boinc_db.cpp,h
            boinc_db_types.h
        sched/
            db_dump.cpp (host, user, team)
            db_purge.cpp (workunit, result)
    - if used by Python scripts (make_project, update_versions), update
        py/Boinc/database.py
    - if used by PHP code, update as needed
        html/
            inc/
                host.inc (host)
                db_ops.inc
            ops/
                db_update.php
            user/
                create_account_action.php (user)
                team_create_action.php (team)
*/

-- Most fields are documented in boinc_db_types.h

-- All fields should be not null
-- Fields should generally have a default
-- (newer MySQL versions don't have automatic defaults)

-- add new fields to the end of the table
-- (makes it easier to update C++ code)

-- Engine is specified as InnoDB for most tables;
-- supposedly this gives better performance.
-- Some (post, thread, profile) are myISAM because it supports fulltext index

-- Going forward, use double for unix times (no 32-bit problem)

-- Put index definitions in constraints.sql, not here

-- fields ending with id (but not _id) are treated specially
-- by the Python code (db_base.py)

-- Put initial content of any table in content.sql, and not in this file.

create table platform (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    name                    varchar(191)    not null,
    user_friendly_name      varchar(254)    not null,
    deprecated              tinyint         not null default 0,
    primary key (id)
) engine=InnoDB;

create table app (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    name                    varchar(191)    not null,
    min_version             integer         not null default 0,
    deprecated              smallint        not null default 0,
    user_friendly_name      varchar(254)    not null,
    homogeneous_redundancy  smallint        not null default 0,
    weight                  double          not null default 1,
    beta                    smallint        not null default 0,
    target_nresults         smallint        not null default 0,
    min_avg_pfc             double          not null default 1,
    host_scale_check        tinyint         not null default 0,
    homogeneous_app_version tinyint         not null default 0,
    non_cpu_intensive       tinyint         not null default 0,
    locality_scheduling     integer         not null default 0,
    n_size_classes          smallint        not null default 0,
    fraction_done_exact     tinyint         not null default 0,
    primary key (id)
) engine=InnoDB;

create table app_version (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    appid                   integer         not null,
    version_num             integer         not null,
    platformid              integer         not null,
    xml_doc                 mediumblob,
    min_core_version        integer         not null default 0,
    max_core_version        integer         not null default 0,
    deprecated              tinyint         not null default 0,
    plan_class              varchar(128)    not null default '',
    pfc_n                   double          not null default 0,
    pfc_avg                 double          not null default 0,
    pfc_scale               double          not null default 0,
    expavg_credit           double          not null default 0,
    expavg_time             double          not null default 0,
    beta                    tinyint         not null default 0,
    primary key (id)
) engine=InnoDB;

create table user (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    email_addr              varchar(191)    not null,
    name                    varchar(191),
    authenticator           varchar(191),
    country                 varchar(254),
    postal_code             varchar(254),
    total_credit            double          not null,
    expavg_credit           double          not null,
    expavg_time             double          not null,
    global_prefs            blob,
    project_prefs           blob,
    teamid                  integer         not null,
    venue                   varchar(254)    not null,
    url                     varchar(254),
    send_email              smallint        not null,
    show_hosts              smallint        not null,
    posts                   smallint        not null,
        -- reused: salt for weak auth

    seti_id                 integer         not null,
        -- reused as 'run jobs on my hosts' flag from remote job submission
    -- the following 3 not used by BOINC
    seti_nresults           integer         not null,
    seti_last_result_time   integer     not null,
    seti_total_cpu          double          not null,

    signature               varchar(254),
        -- stores invite code, if any, for users created via RPC
    has_profile             smallint        not null,
    cross_project_id        varchar(254)    not null,
    passwd_hash             varchar(254)    not null,
    email_validated         smallint        not null,
    donated                 smallint        not null,
    login_token             char(32)        not null default '',
    login_token_time        double          not null default 0,
    previous_email_addr     varchar(254)    not null default '',
    email_addr_change_time  double          not null default 0,
    primary key (id)
) engine=InnoDB;

create table team (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    userid                  integer         not null,
    name                    varchar(191)    not null,
    name_lc                 varchar(254),
    url                     varchar(254),
    type                    integer         not null,
    name_html               varchar(254),
    description             text,
    nusers                  integer         not null,   /* temp */
    country                 varchar(254),
    total_credit            double          not null default 0.0,   /* temp */
    expavg_credit           double          not null default 0.0,   /* temp */
    expavg_time             double          not null,
    seti_id                 integer         not null default 0,
        -- repurposed to store master ID of BOINC-wide teams
    ping_user               integer         not null default 0,
    ping_time               integer unsigned not null default 0,
    joinable                tinyint         not null default 1,
    mod_time                timestamp default current_timestamp on update current_timestamp,
    primary key (id)
) engine=InnoDB;

create table host (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    userid                  integer         not null,
    rpc_seqno               integer         not null,
    rpc_time                integer         not null,
    total_credit            double          not null,
    expavg_credit           double          not null,
    expavg_time             double          not null,

    timezone                integer         not null,
    domain_name             varchar(254),
    serialnum               varchar(254),
        /* summary of GPUs and VBox (deprecated) */
    last_ip_addr            varchar(254),
    nsame_ip_addr           integer         not null,

    on_frac                 double          not null,
    connected_frac          double          not null,
    active_frac             double          not null,
    cpu_efficiency          double          not null,
    duration_correction_factor double       not null,
    p_ncpus                 integer         not null,
    p_vendor                varchar(254),
    p_model                 varchar(254),
    p_fpops                 double          not null,
    p_iops                  double          not null,
    p_membw                 double          not null,

    os_name                 varchar(254),
    os_version              varchar(254),

    m_nbytes                double          not null,
    m_cache                 double          not null,
    m_swap                  double          not null,

    d_total                 double          not null,
    d_free                  double          not null,
    d_boinc_used_total      double          not null,
    d_boinc_used_project    double          not null,
    d_boinc_max             double          not null,

    n_bwup                  double          not null,
    n_bwdown                double          not null,

    credit_per_cpu_sec      double          not null,
    venue                   varchar(254)    not null,
    nresults_today          integer         not null,
    avg_turnaround          double          not null,
    host_cpid               varchar(254),
    external_ip_addr        varchar(254),
    max_results_day         integer         not null,
    error_rate              double          not null default 0,
    product_name            varchar(254)    not null,
    gpu_active_frac         double          not null,
    p_ngpus                 integer         not null,
    p_gpu_fpops             double          not null,
    misc                    text            not null default '',
        -- JSON description of GPUs, Docker etc.

    primary key (id)
) engine=InnoDB;

-- see comments in boinc_db.h
create table host_app_version (
    host_id                 integer         not null,
    app_version_id          integer         not null,
    pfc_n                   double          not null,
    pfc_avg                 double          not null,
    et_n                    double          not null,
    et_avg                  double          not null,
    et_var                  double          not null,
    et_q                    double          not null,
    max_jobs_per_day        integer         not null,
    n_jobs_today            integer         not null,
    turnaround_n            double          not null,
    turnaround_avg          double          not null,
    turnaround_var          double          not null,
    turnaround_q            double          not null,
    consecutive_valid       integer         not null
) engine = InnoDB;

/*
 * Only information needed by the server or other backend components
 * is broken out into separate fields.
 * Other info, i.e. that needed by the client (files, etc.)
 * is stored in the XML doc
 */
create table workunit (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    appid                   integer         not null,
    name                    varchar(191)    not null,
    xml_doc                 blob,
    batch                   integer         not null,
    rsc_fpops_est           double          not null,
    rsc_fpops_bound         double          not null,
    rsc_memory_bound        double          not null,
    rsc_disk_bound          double          not null,
    need_validate           smallint        not null,
    canonical_resultid      integer         not null,
    canonical_credit        double          not null,
    transition_time         integer         not null,
    delay_bound             integer         not null,
    error_mask              integer         not null,
    file_delete_state       integer         not null,
    assimilate_state        integer         not null,
    hr_class                integer         not null,
    opaque                  double          not null,
    min_quorum              integer         not null,
    target_nresults         integer         not null,
    max_error_results       integer         not null,
    max_total_results       integer         not null,
    max_success_results     integer         not null,
    result_template_file    varchar(63)     not null,
    priority                integer         not null,
    mod_time                timestamp default current_timestamp on update current_timestamp,
    rsc_bandwidth_bound     double          not null,
    fileset_id              integer         not null,
    app_version_id          integer         not null,
    transitioner_flags      tinyint         not null,
    size_class              smallint        not null default -1,
    keywords                varchar(254)    not null,
    app_version_num         integer         not null,
    primary key (id)
) engine=InnoDB;

create table result (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    workunitid              integer         not null,
    server_state            integer         not null,
    outcome                 integer         not null,
    client_state            integer         not null,
    hostid                  integer         not null,
    userid                  integer         not null,
    report_deadline         integer         not null,
    sent_time               integer         not null,
    received_time           integer         not null,
    name                    varchar(191)    not null,
    cpu_time                double          not null,
    xml_doc_in              blob,
    xml_doc_out             blob,
    stderr_out              blob,
    batch                   integer         not null,
    file_delete_state       integer         not null,
    validate_state          integer         not null,
    claimed_credit          double          not null,
    granted_credit          double          not null,
    opaque                  double          not null,
    random                  integer         not null,
    app_version_num         integer         not null,
    appid                   integer         not null,
    exit_status             integer         not null,
    teamid                  integer         not null,
    priority                integer         not null,
    mod_time                timestamp default current_timestamp on update current_timestamp,
    elapsed_time            double          not null,
    flops_estimate          double          not null,
    app_version_id          integer         not null,
    runtime_outlier         tinyint         not null,
    size_class              smallint        not null default -1,
    peak_working_set_size   double          not null,
    peak_swap_size          double          not null,
    peak_disk_usage         double          not null,
    primary key (id)
) engine=InnoDB;

create table batch (
    id                      serial          primary key,
    user_id                 integer         not null,
    create_time             integer         not null,
    logical_start_time      double          not null,
    logical_end_time        double          not null,
    est_completion_time     double          not null,
    njobs                   integer         not null,
    fraction_done           double          not null,
    nerror_jobs             integer         not null,
    state                   integer         not null,
    completion_time         double          not null,
    credit_estimate         double          not null,
    credit_canonical        double          not null,
    credit_total            double          not null,
    name                    varchar(255)    not null,
    app_id                  integer         not null,
    project_state           integer         not null,
    description             varchar(255)    not null,
    expire_time             double          not null
) engine = InnoDB;

-- permissions for job submission
--
create table user_submit (
    user_id                 integer         not null,
    quota                   double          not null,
    logical_start_time      double          not null,
    submit_all              tinyint         not null,
        -- can submit jobs to any app
    manage_all              tinyint         not null,
        -- manager privileges for all apps
        -- grant/revoke permissions (except manage), change quotas
        -- create apps
    max_jobs_in_progress    integer         not null,
    primary key (user_id)
) engine = InnoDB;

-- (user, app) submit permissions
-- The existence of the record implies permission to submit jobs
--
create table user_submit_app (
    user_id                 integer         not null,
    app_id                  integer         not null,
    manage                  tinyint         not null,
        -- can
        --   create/deprecate app versions of this app
        --   grant/revoke permissions (except admin) this app
        --   abort their jobs
    primary key (user_id, app_id)
) engine = InnoDB;

-- Record files (created by remote file mgt) present on server.
--
create table job_file (
    id                      integer         not null auto_increment,
    name                    varchar(191)    not null,
    create_time             double          not null,
    delete_time             double          not null,
    primary key (id)
) engine = InnoDB;

-- the following are used to implement trickle messages

create table msg_from_host (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    hostid                  integer         not null,
    variety                 varchar(254)    not null,
    handled                 smallint        not null,
    xml                     mediumtext,
    primary key (id)
) engine=InnoDB;

create table msg_to_host (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    hostid                  integer         not null,
    variety                 varchar(254)    not null,
    handled                 smallint        not null,
    xml                     mediumtext,
    primary key (id)
) engine=InnoDB;

-- An assignment of a WU to a specific host, user, or team, or to all hosts
--
create table assignment (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    target_id               integer         not null,
        -- ID of target entity (see below)
    target_type             integer         not null,
        -- 0=none, 1=host, 2=user, 3=team
    multi                   tinyint         not null,
        -- 0=normal replication, 1=all hosts in set
    workunitid              integer         not null,
    resultid                integer         not null,
        -- if not multi, the result
        -- deprecated
    primary key (id)
) engine = InnoDB;

-- EVERYTHING FROM HERE ON IS USED ONLY FROM PHP,
-- SO NOT IN BOINC_DB.H ETC.

-- user profile (description, pictures)
--
create table profile (
    userid                  integer         not null,
    language                varchar(254),
    response1               text,
    response2               text,
    has_picture             smallint        not null,
    recommend               integer         not null,
    reject                  integer         not null,
    posts                   integer         not null,
    uotd_time               integer,
    verification            integer         not null,
        -- UOD screening status: -1 denied, 0 unrated, 1 approved
    primary key (userid)
) engine=InnoDB;

-- message board category
-- help desk is a group of categories that are handled separately
--
create table category (
    id                      integer         not null auto_increment,
    orderID                 integer         not null,
        -- order in which to display
    lang                    integer         not null,
        -- not used
    name                    varchar(180)    not null,
    is_helpdesk             smallint        not null,
    primary key (id)
) engine=InnoDB;

-- message board topic
--
create table forum (
    id                      integer         not null auto_increment,
    category                integer         not null,
        -- ID of entity to which this forum is attached.
        -- The type (table) of the entity is determined by parent_type
    orderID                 integer         not null,
    title                   varchar(175)    not null,
    description             varchar(254)    not null,
    timestamp               integer         not null default 0,
        -- time of last new or modified thread or post
    threads                 integer         not null default 0,
        -- number of non-hidden threads in forum
    posts                   integer         not null default 0,
    rate_min_expavg_credit  integer         not null default 0,
    rate_min_total_credit   integer         not null default 0,
    post_min_interval       integer         not null default 0,
    post_min_expavg_credit  integer         not null default 0,
    post_min_total_credit   integer         not null default 0,
    is_dev_blog             tinyint         not null default 0,
    parent_type             integer         not null default 0,
        -- entity type to which this forum is attached:
        -- 0 == category (public)
        -- 1 == team
        -- 2 == group
    primary key (id)
) engine=InnoDB;

-- threads in a topic (or questions)
--
create table thread (
    id                      integer         not null auto_increment,
    forum                   integer         not null,
    owner                   integer         not null,
        -- user ID of creator
    status                  integer         not null,
        -- whether a question has been answered
        -- News forum: if set, don't export as notice
    title                   varchar(254)    not null,
    timestamp               integer         not null,
        -- time of last new or modified post
    views                   integer         not null,
        -- number of times this has been viewed
    replies                 integer         not null,
        -- number of non-hidden posts in thread, not counting the initial one
    activity                double          not null,
        -- for questions: number of askers / time since asked
        -- (set periodically by update_forum_activity.php)
    sufferers               integer         not null,
        -- in help desk: # people who indicated they had same problem
    score                   double          not null,
    votes                   integer         not null,
    create_time             integer         not null,
        -- when this record was created
    hidden                  integer         not null,
        -- nonzero if hidden by moderators
    sticky                  tinyint         not null default 0,
    locked                  tinyint         not null default 0,
    primary key (id)
) engine=InnoDB;

-- postings in a thread (or answers)
-- Each thread has an initial post
--
create table post (
    id                      integer         not null auto_increment,
    thread                  integer         not null,
    user                    integer         not null,
    timestamp               integer         not null,
        -- create time
    content                 text            not null,
    modified                integer         not null,
        -- when last modified
    parent_post             integer         not null,
        -- post that was replied to, if any
    score                   double          not null,
    votes                   integer         not null,
    signature               tinyint         not null default 0,
    hidden                  integer         not null,
        -- nonzero if hidden by moderators
    primary key (id)
) engine=InnoDB;

-- subscription to a thread or forum
--
create table subscriptions (
    userid                  integer         not null,
    threadid                integer         not null,
        -- or negative if forum ID (kludge)
    notified_time           integer         not null default 0
        -- deprecated
) engine=InnoDB;

-- actually: prefs for all community features
--
create table forum_preferences (
    userid                  integer         not null default 0,
    signature               varchar(254)    not null default '',
    posts                   integer         not null default 0,
    last_post               integer         not null,
    avatar                  varchar(254)    not null default '',
    hide_avatars            tinyint         not null default 0,
    forum_sorting           integer         not null,
    thread_sorting          integer         not null,
    no_signature_by_default tinyint         not null default 1,
    images_as_links         tinyint         not null default 0,
    link_popup              tinyint         not null default 0,
    mark_as_read_timestamp  integer         not null default 0,
    special_user            char(12)        not null default '0',
    jump_to_unread          tinyint         not null default 1,
    hide_signatures         tinyint         not null default 0,
    rated_posts             varchar(254)    not null,
    low_rating_threshold    integer         not null default -25,
        -- deprecated
    high_rating_threshold   integer         not null default 5,
        -- deprecated
    minimum_wrap_postcount  integer         DEFAULT 100 NOT NULL,
    display_wrap_postcount  integer         DEFAULT 75 NOT NULL,
    ignorelist              varchar(254)    not null,
    ignore_sticky_posts     tinyint         not null default 0,
    banished_until          integer         not null default 0,
    pm_notification         tinyint         not null default 0,
        -- actually controls all notifications.
        -- 0 = no email
        -- 1 = email per event
        -- 2 = digest email
    highlight_special       tinyint         not null default 1,
    primary key (userid)
) engine=InnoDB;

-- keep track of last time a user read a thread
create table forum_logging (
    userid                  integer         not null default 0,
    threadid                integer         not null default 0,
    timestamp               integer         not null default 0,
    primary key (userid,threadid)
) engine=InnoDB;

create table post_ratings (
    post                    integer         not null,
    user                    integer         not null,
    rating                  tinyint         not null,
    primary key(post, user)
) engine=InnoDB;

create table sent_email (
    userid                  integer         not null,
    time_sent               integer         not null,
    email_type              smallint        not null,
        -- 0 = other
        -- 1 = newsletter
        -- 2 = lapsed reminder
        -- 3 = failed reminder
        -- 4 = forum post hide
        -- 5 = forum ban
        -- 6 = fundraising appeal
    primary key(userid)
) engine=InnoDB;

create table private_messages (
    id                      integer         not null auto_increment,
    userid                  integer         not null,
    senderid                integer         not null,
    date                    integer         not null,
    opened                  tinyint         not null default 0,
    subject                 varchar(255)    not null,
    content                 text            not null,
    primary key(id)
) engine=InnoDB;

create table credited_job (
    userid                  integer         not null,
    workunitid              bigint          not null
) engine=InnoDB;

create table donation_items (
    id                      integer         not null auto_increment,
    item_name               varchar(32)     not null,
    title                   varchar(255)    not null,
    description             varchar(255)    not null,
    required                double          not null default '0',
    PRIMARY KEY(id)
) engine=InnoDB;

create table donation_paypal (
    id                      integer         not null auto_increment,
    order_time              integer         not null,
    userid                  integer         not null,
    email_addr              varchar(255)    not null,
    order_amount            double(6,2)     not null,
    processed               tinyint         not null default '0',
    payment_time            integer         not null,
    item_name               varchar(255)    not null,
    item_number             varchar(255)    not null,
    payment_status          varchar(255)    not null,
    payment_amount          double(6,2)     not null,
    payment_fee             double(5,2)     default null,
    payment_currency        varchar(255)    not null,
    txn_id                  varchar(255)    not null,
    receiver_email          varchar(255)    not null,
    payer_email             varchar(255)    not null,
    payer_name              varchar(255)    not null,
    PRIMARY KEY(id)
) engine=InnoDB;

-- record changes in team membership
create table team_delta (
    userid                  integer         not null,
    teamid                  integer         not null,
    timestamp               integer         not null,
    joining                 tinyint         not null,
    total_credit            double          not null
) engine=InnoDB;

-- tables for moderator banishment votes
create table banishment_vote (
    id                      serial          primary key,
    userid                  integer         not null,
    modid                   integer         not null,
    start_time              integer         not null,
    end_time                integer         not null
) engine=InnoDB;

create table banishment_votes (
    id                      serial          primary key,
    voteid                  integer         not null,
    modid                   integer         not null,
    time                    integer         not null,
    yes                     tinyint         not null
) engine=InnoDB;

create table team_admin (
    teamid                  integer         not null,
    userid                  integer         not null,
    create_time             integer         not null,
    rights                  integer         not null
) engine=InnoDB;

-- A friendship request.
-- The friendship exists if (x,y) and (y,x)
create table friend (
    user_src                integer         not null,
        -- initiator
    user_dest               integer         not null,
        -- target
    message                 varchar(255)    not null,
    create_time             integer         not null,
    reciprocated            tinyint         not null
        -- whether the reciprocal exists
);

-- a notification of something, e.g.
--   a friend request or confirmation
--   a post in a subscribed thread
--   a personal message
-- These records are deleted when the user acts on them
create table notify (
    id                      serial          primary key,
    userid                  integer         not null,
        -- destination of notification
    create_time             integer         not null,
    type                    integer         not null,
        -- see html/inc/forum_db.inc
    opaque                  integer         not null
        -- the ID of the thread, user or PM record
);

create table badge (
    id                      serial          primary key,
    create_time             double          not null,
    type                    tinyint         not null,
        -- 0=user, 1=team
    name                    varchar(255)    not null,
        -- internal use (not visible to users)
    title                   varchar(255)    not null,
        -- user-visible, short
    description             varchar(255)    not null,
        -- user-visible, possibly longer
    image_url               varchar(255)    not null,
        -- location of image
    level                   varchar(255)    not null,
        -- project-defined
    tags                    varchar(255)    not null,
        -- project-defined
    sql_rule                varchar(255)    not null
);

create table badge_user (
    badge_id                integer         not null,
    user_id                 integer         not null,
    create_time             double          not null,
    reassign_time           double          not null
);

create table badge_team (
    badge_id                integer         not null,
    team_id                 integer         not null,
    create_time             double          not null,
    reassign_time           double          not null
);

create table credit_user (
    userid                  integer         not null,
    appid                   integer         not null,
    njobs                   integer         not null,
    total                   double          not null,
    expavg                  double          not null,
    expavg_time             double          not null,
    credit_type             integer         not null,
    primary key (userid, appid, credit_type)
) engine=InnoDB;

create table credit_team (
    teamid                  integer         not null,
    appid                   integer         not null,
    njobs                   integer         not null,
    total                   double          not null,
    expavg                  double          not null,
    expavg_time             double          not null,
    credit_type             integer         not null,
    primary key (teamid, appid, credit_type)
) engine=InnoDB;

create table token (
    token                   varchar(64)     not null,
    userid                  integer         not null,
    type                    char            not null,
    create_time             integer         not null,
    expire_time             integer,
    primary key (token)
) engine=InnoDB;

create table user_deleted (
    userid                  integer         not null,
    public_cross_project_id varchar(254)    not null,
    create_time             double          not null,
    primary key (userid)
) engine=InnoDB;

create table host_deleted (
    hostid                  integer         not null,
    public_cross_project_id varchar(254)    not null,
    create_time             double          not null,
    primary key (hostid)
) engine=InnoDB;

create table consent (
    id                      integer         not null auto_increment,
    userid                  integer         not null,
    consent_type_id         integer         not null,
    consent_time            integer         not null,
    consent_flag            tinyint         not null,
    consent_not_required    tinyint         not null,
    source                  varchar(255)    not null,
    primary key (id)
) engine=InnoDB;

create table consent_type (
    id                      integer         not null auto_increment,
    shortname               varchar(191)    not null,
    description             varchar(255)    not null,
    enabled                 integer         not null,
    project_specific        integer         not null,
    privacypref             integer         not null,
    primary key (id)
) engine=InnoDB;

-- SQL View representing the latest consent state of users for all
-- consent_types. Used in sched/db_dump and Web site preferences to
-- determine if a user has consented to a particular consent type.
create view latest_consent as
SELECT userid,
       consent_type_id,
       consent_flag
  FROM consent
 WHERE NOT EXISTS
       (SELECT *
          FROM consent AS filter
         WHERE consent.userid = filter.userid
           AND consent.consent_type_id = filter.consent_type_id
           AND filter.consent_time > consent.consent_time);
