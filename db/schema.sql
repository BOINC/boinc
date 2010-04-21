/*  If you add/change anything, update
    boinc_db.C,h
    and if needed:
    py/Boinc/
        database.py
    html/
        inc/
            host.inc (host)
            db_ops.inc
        ops/
            db_update.php
        user/
            create_account_action.php (user)
            team_create_action.php (team)
    sched/
        db_dump.C (host, user, team)
        db_purge.C (workunit, result)
*/
/* Fields are documented in boinc_db.h */
/* Do not replace this with an automatically generated schema */

/* type is specified as InnoDB for most tables.
   Supposedly this gives better performance.
   The others (post, thread, profile) are myISAM
   because it supports fulltext index
*/

/* fields ending with id (but not _id) are treated specially
   by the Python code (db_base.py)
*/

create table platform (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(254) not null,
    user_friendly_name  varchar(254) not null,
    deprecated          tinyint     not null default 0,
    primary key (id)
) engine=InnoDB;

create table app (
    id                      integer         not null auto_increment,
    create_time             integer         not null,
    name                    varchar(254)    not null,
    min_version             integer         not null default 0,
    deprecated              smallint        not null default 0,
    user_friendly_name      varchar(254)    not null,
    homogeneous_redundancy  smallint        not null default 0,
    weight                  double          not null default 1,
    beta                    smallint        not null default 0,
    target_nresults         smallint        not null default 0,
    min_avg_pfc             double          not null,
    host_scale_check        tinyint         not null,
    max_jobs_in_progress    integer         not null,
    max_gpu_jobs_in_progress integer        not null,
    max_jobs_per_rpc        integer         not null,
    max_jobs_per_day_init   integer         not null,
    primary key (id)
) engine=InnoDB;

create table app_version (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    appid               integer     not null,
    version_num         integer     not null,
    platformid          integer     not null,
    xml_doc             mediumblob,
    min_core_version    integer     not null default 0,
    max_core_version    integer     not null default 0,
    deprecated          tinyint     not null default 0,
    plan_class          varchar(254) not null default '',
    pfc_n               double      not null default 0,
    pfc_avg             double      not null default 0,
    pfc_scale           double      not null default 0,
    expavg_credit       double      not null default 0,
    expavg_time         double      not null default 0,
    primary key (id)
) engine=InnoDB;

create table user (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    email_addr          varchar(254) not null,
    name                varchar(254),
    authenticator       varchar(254),
    country             varchar(254),
    postal_code         varchar(254),
    total_credit        double      not null,
    expavg_credit       double      not null,
    expavg_time         double      not null,
    global_prefs        blob,
    project_prefs       blob,
    teamid              integer     not null,
    venue               varchar(254)    not null,
    url                 varchar(254),
    send_email          smallint    not null,
    show_hosts          smallint    not null,
    posts               smallint    not null,
        -- reused: salt for weak auth
    seti_id             integer     not null,
    seti_nresults       integer     not null,
    seti_last_result_time   integer not null,
    seti_total_cpu      double      not null,
    signature           varchar(254),
        -- deprecated
    has_profile         smallint    not null,
    cross_project_id    varchar(254) not null,
    passwd_hash         varchar(254) not null,
    email_validated     smallint    not null,
    donated             smallint    not null,
    primary key (id)
) engine=InnoDB;

create table team (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    userid              integer     not null,
    name                varchar(254) not null,
    name_lc             varchar(254),
    url                 varchar(254),
    type                integer     not null,
    name_html           varchar(254),
    description         text,
    nusers              integer     not null,   /* temp */
    country             varchar(254),
    total_credit        double      not null,   /* temp */
    expavg_credit       double      not null,   /* temp */
    expavg_time         double      not null,
    seti_id             integer     not null,
    ping_user           integer     not null default 0,
    ping_time           integer unsigned not null default 0,
    joinable            tinyint     not null default 1,
    primary key (id)
) engine=MyISAM;  

create table host (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    userid              integer     not null,
    rpc_seqno           integer     not null,
    rpc_time            integer     not null,
    total_credit        double      not null,
    expavg_credit       double      not null,
    expavg_time         double      not null,

    timezone            integer     not null,
    domain_name         varchar(254),
    serialnum           varchar(254),
    last_ip_addr        varchar(254),
    nsame_ip_addr       integer     not null,

    on_frac             double      not null,
    connected_frac      double      not null,
    active_frac         double      not null,
    cpu_efficiency      double      not null,
    duration_correction_factor double not null,
    p_ncpus             integer     not null,
    p_vendor            varchar(254),
    p_model             varchar(254),
    p_fpops             double      not null,
    p_iops              double      not null,
    p_membw             double      not null,

    os_name             varchar(254),
    os_version          varchar(254),

    m_nbytes            double      not null,
    m_cache             double      not null,
    m_swap              double      not null,

    d_total             double      not null,
    d_free              double      not null,
    d_boinc_used_total  double      not null,
    d_boinc_used_project double     not null,
    d_boinc_max         double      not null,

    n_bwup              double      not null,
    n_bwdown            double      not null,

    credit_per_cpu_sec  double      not null,
    venue               varchar(254) not null,
    nresults_today      integer     not null,
    avg_turnaround      double      not null,
    host_cpid           varchar(254),
    external_ip_addr    varchar(254),
    max_results_day     integer     not null,
    error_rate          double      not null default 0,

    primary key (id)
) engine=InnoDB;

-- see comments in boinc_db.h
create table host_app_version (
    host_id             integer     not null,
    app_version_id      integer     not null,
    pfc_n               double      not null,
    pfc_avg             double      not null,
    et_n                double      not null,
    et_avg              double      not null,
    et_var              double      not null,
    et_q                double      not null,
    max_jobs_per_day    integer     not null,
    n_jobs_today        integer     not null,
    turnaround_n        double      not null,
    turnaround_avg      double      not null,
    turnaround_var      double      not null,
    turnaround_q        double      not null,
    consecutive_valid   integer     not null
) engine = InnoDB;

/*
 * Only information needed by the server or other backend components
 * is broken out into separate fields.
 * Other info, i.e. that needed by the client (files, etc.)
 * is stored in the XML doc
 */
create table workunit (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    appid               integer     not null,
    name                varchar(254) not null,
    xml_doc             blob,
    batch               integer     not null,
    rsc_fpops_est       double      not null,
    rsc_fpops_bound     double      not null,
    rsc_memory_bound    double      not null,
    rsc_disk_bound      double      not null,
    need_validate       smallint    not null,
    canonical_resultid  integer     not null,
    canonical_credit    double      not null,
    transition_time     integer     not null,
    delay_bound         integer     not null,
    error_mask          integer     not null,
    file_delete_state   integer     not null,
    assimilate_state    integer     not null,
    hr_class            integer     not null,
    opaque              double      not null,
    min_quorum          integer     not null,
    target_nresults     integer     not null,
    max_error_results   integer     not null,
    max_total_results   integer     not null,
    max_success_results integer     not null,
    result_template_file varchar(63) not null,
    priority            integer     not null,
    mod_time            timestamp,
    rsc_bandwidth_bound double      not null,
    fileset_id          integer     not null,
    primary key (id)
) engine=InnoDB;

create table result (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    workunitid          integer     not null,
    server_state        integer     not null,
    outcome             integer     not null,
    client_state        integer     not null,
    hostid              integer     not null,
    userid              integer     not null,
    report_deadline     integer     not null,
    sent_time           integer     not null,
    received_time       integer     not null,
    name                varchar(254) not null,
    cpu_time            double      not null,
    xml_doc_in          blob,
    xml_doc_out         blob,
    stderr_out          blob,
    batch               integer     not null,
    file_delete_state   integer     not null,
    validate_state      integer     not null,
    claimed_credit      double      not null,
    granted_credit      double      not null,
    opaque              double      not null,
    random              integer     not null,
    app_version_num     integer     not null,
    appid               integer     not null,
    exit_status         integer     not null,
    teamid              integer     not null,
    priority            integer     not null,
    mod_time            timestamp,
    elapsed_time        double      not null,
    flops_estimate      double      not null,
    app_version_id      integer     not null,
    primary key (id)
) engine=InnoDB;

-- the following are used to implement trickle messages

create table msg_from_host (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    hostid              integer     not null,
    variety             varchar(254) not null,
    handled             smallint    not null,
    xml                 mediumtext,
    primary key (id)
) engine=InnoDB;

create table msg_to_host (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    hostid              integer     not null,
    variety             varchar(254) not null,
    handled             smallint    not null,
    xml                 mediumtext,
    primary key (id)
) engine=InnoDB;

-- An assignment of a WU to a specific host, user, or team, or to all hosts
--
create table assignment (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    target_id           integer     not null,
        -- ID of target entity (see below)
    target_type         integer     not null,
        -- 0=none, 1=host, 2=user, 3=team
    multi               tinyint     not null,
        -- 0=single host, 1=all hosts in set
    workunitid          integer     not null,
    resultid            integer     not null,
        -- if not multi, the result
    primary key (id)
) engine = InnoDB;

-- credit multiplier.  Used by the scheduler and calculate_credit_multiplier
-- script to automatically adjust granted credit.
create table credit_multiplier (
    id                  serial          primary key,
    appid               integer         not null,
    time                integer         not null,
    multiplier          double          not null default 0
) engine=MyISAM;

-- the following not used for anything right now
create table state_counts (
    appid               integer     not null,
    last_update_time    integer     not null,
    result_server_state_2       integer not null,
    result_server_state_4       integer not null,
    result_file_delete_state_1  integer not null,
    result_file_delete_state_2  integer not null,
    result_server_state_5_and_file_delete_state_0       integer not null,
    workunit_need_validate_1    integer not null,
    workunit_assimilate_state_1 integer not null,
    workunit_file_delete_state_1        integer not null,
    workunit_file_delete_state_2        integer not null,
    primary key (appid)
) engine=MyISAM; 


-- EVERYTHING FROM HERE ON IS USED ONLY FROM PHP,
-- SO NOT IN BOINC_DB.H ETC.

-- user profile (description, pictures)
--
create table profile (
    userid              integer     not null,
    language            varchar(254),
    response1           text,
    response2           text,
    has_picture         smallint    not null,
    recommend           integer     not null,
    reject              integer     not null,
    posts               integer     not null,
    uotd_time           integer,
    verification        integer     not null,
        -- UOD screening status: -1 denied, 0 unrated, 1 approved
    primary key (userid)
) engine=MyISAM;

-- message board category
-- help desk is a group of categories that are handled separately
--
create table category (
    id                  integer     not null auto_increment,
    orderID             integer     not null,
        -- order in which to display
    lang                integer     not null,
        -- not used
    name                varchar(254) binary,
    is_helpdesk         smallint    not null,
    primary key (id)
) engine=InnoDB;

-- message board topic
--
create table forum (
    id                  integer     not null auto_increment,
    category            integer     not null,
        -- ID of entity to which this forum is attached.
        -- The type (table) of the entity is determined by parent_type
    orderID             integer     not null,
    title               varchar(254) not null,
    description         varchar(254) not null,
    timestamp           integer     not null,
        -- time of last new or modified thread or post
    threads             integer     not null,
        -- number of non-hidden threads in forum
    posts               integer     not null,
    rate_min_expavg_credit integer not null,
    rate_min_total_credit integer not null,
    post_min_interval   integer not null,
    post_min_expavg_credit integer not null,
    post_min_total_credit integer not null,
    is_dev_blog            tinyint not null default 0,
    parent_type         integer     not null,
        -- entity type to which this forum is attached:
        -- 0 == category (public)
        -- 1 == team
        -- 2 == group
    primary key (id)
) engine=InnoDB;

-- threads in a topic (or questions)
--
create table thread (
    id                  integer     not null auto_increment,
    forum               integer     not null,
    owner               integer     not null,
        -- user ID of creator
    status              integer     not null,
    title               varchar(254) not null,
    timestamp           integer     not null,
        -- time of last new or modified post
    views               integer     not null,
        -- number of times this has been viewed
    replies             integer     not null,
        -- number of non-hidden posts in thread, not counting the initial one
    activity            double      not null,
        -- for questions: number of askers / time since asked
        -- (set periodically by update_forum_activity.php)
    sufferers           integer     not null,
        -- in help desk: # people who indicated they had same problem
    score               double      not null,
    votes               integer     not null,
    create_time         integer     not null,
        -- when this record was created
    hidden              integer     not null,
        -- nonzero if hidden by moderators
    sticky              tinyint not null default 0,
    locked              tinyint not null default 0,
    primary key (id)
) engine=MyISAM;

-- postings in a thread (or answers)
-- Each thread has an initial post
--
create table post (
    id                  integer     not null auto_increment,
    thread              integer     not null,
    user                integer     not null,
    timestamp           integer     not null,
        -- create time
    content             text        not null,
    modified            integer     not null,
        -- when last modified
    parent_post         integer     not null,
        -- post that was replied to, if any
    score               double      not null,
    votes               integer     not null,
    signature           tinyint     not null default 0,
    hidden              integer     not null,
        -- nonzero if hidden by moderators
    primary key (id)
) engine=MyISAM;

-- subscription to a thread
--
create table subscriptions (
    userid              integer     not null,
    threadid            integer     not null,
    notified_time       integer     not null default 0
        -- deprecated
) engine=InnoDB;

-- actually: prefs for all community features
--
create table forum_preferences (
    userid              integer     not null default 0,
    signature           varchar(254) not null default '',
    posts               integer     not null default 0,
    last_post           integer     not null,
    avatar              varchar(254) not null default '',
    hide_avatars        tinyint     not null default 0,
    forum_sorting       integer     not null,
    thread_sorting      integer     not null,
    no_signature_by_default tinyint not null default 1,
    images_as_links     tinyint     not null default 0,
    link_popup          tinyint     not null default 0,
    mark_as_read_timestamp integer  not null default 0,
    special_user        char(12)    not null default '0',
    jump_to_unread      tinyint     not null default 1,
    hide_signatures     tinyint     not null default 0,
    rated_posts         varchar(254) not null,
    low_rating_threshold integer not null default -25,
        -- deprecated
    high_rating_threshold integer not null default 5,
        -- deprecated
    minimum_wrap_postcount integer  DEFAULT 100 NOT NULL,
    display_wrap_postcount integer  DEFAULT 75 NOT NULL,
    ignorelist          varchar(254) not null,
    ignore_sticky_posts tinyint     not null default 0,
    banished_until      integer     not null default 0,
    pm_notification     tinyint    not null default 0,
        -- actually controls all notifications.
        -- 0 = no email
        -- 1 = email per event
        -- 2 = digest email
    highlight_special   tinyint     not null default 1,
    primary key (userid)
) engine=MyISAM; 

-- keep track of last time a user read a thread
create table forum_logging (
    userid              integer     not null default 0,
    threadid            integer     not null default 0,
    timestamp           integer     not null default 0,
    primary key (userid,threadid)
) engine=MyISAM;

create table post_ratings (
    post                integer     not null,
    user                integer     not null,
    rating              tinyint     not null,
    primary key(post, user)
) engine=MyISAM;

create table sent_email (
    userid              integer     not null,
    time_sent           integer     not null,
    email_type          smallint    not null,
        -- 0 = other
        -- 1 = newsletter
        -- 2 = lapsed reminder
        -- 3 = failed reminder
        -- 4 = forum post hide
        -- 5 = forum ban
        -- 6 = fundraising appeal
    primary key(userid)
) engine=MyISAM;

create table private_messages (
    id                  integer     not null auto_increment,
    userid              integer     not null,
    senderid            integer     not null,
    date                integer     not null,
    opened              tinyint     not null default 0,
    subject             varchar(255)  not null,
    content             text        not null,
    primary key(id),
    key userid (userid)
) engine=MyISAM;

create table credited_job (
    userid              integer     not null,
    workunitid          bigint      not null
) engine=MyISAM;

create table donation_items (
    id                  integer         not null auto_increment,
    item_name           varchar(32)     not null,
    title               varchar(255)    not null,
    description         varchar(255)    not null,
    required            double          not null default '0',
    PRIMARY KEY(id)
) engine=MyISAM;

create table donation_paypal (
    id                  integer         not null auto_increment,
    order_time          integer         not null,
    userid              integer         not null,
    email_addr          varchar(255)    not null,
    order_amount        double(6,2)     not null,
    processed           tinyint         not null default '0',
    payment_time        integer         not null,
    item_name           varchar(255)            not null,
    item_number         varchar(255)            not null,
    payment_status      varchar(255)            not null,
    payment_amount      double(6,2)             not null,
    payment_fee         double(5,2)             default null,
    payment_currency    varchar(255)            not null,
    txn_id              varchar(255)            not null,
    receiver_email      varchar(255)            not null,
    payer_email         varchar(255)            not null,
    payer_name          varchar(255)            not null,
    PRIMARY KEY(id)
) engine=MyISAM;

-- record changes in team membership
create table team_delta (
    userid              integer         not null,
    teamid              integer         not null,
    timestamp           integer         not null,
    joining             tinyint         not null,
    total_credit        double          not null
) engine=MyISAM;

-- tables for moderator banishment votes
create table banishment_vote (
    id                  serial          primary key,
    userid              integer         not null,
    modid               integer         not null,
    start_time          integer         not null,
    end_time            integer         not null
) engine=MyISAM;

create table banishment_votes (
    id                  serial          primary key,
    voteid              integer         not null,
    modid               integer         not null,
    time                integer         not null,
    yes                 tinyint         not null
) engine=MyISAM;

create table team_admin (
    teamid              integer         not null,
    userid              integer         not null,
    create_time         integer         not null,
    rights              integer         not null
) engine=MyISAM;

-- A friendship request.
-- The friendship exists if (x,y) and (y,x)
create table friend (
    user_src            integer         not null,
        -- initiator
    user_dest           integer         not null,
        -- target
    message             varchar(255)    not null,
    create_time         integer         not null,
    reciprocated        tinyint         not null
        -- whether the reciprocal exists
);

-- a notification of something, e.g.
--   a friend request or confirmation
--   a post in a subscribed thread
--   a personal message
-- These records are deleted when the user acts on them
create table notify (
    id                  serial          primary key,
    userid              integer         not null,
        -- destination of notification
    create_time         integer         not null,
    type                integer         not null,
    opaque              integer         not null
        -- some other ID, e.g. that of the thread, user or PM record
);
