/*  If you add/change anything, update
    boinc_db.C,h
    and if needed:
    py/Boinc/
        database.py
    html/
        inc/
            host.inc (host)
        ops/
            db_ops.inc
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
    deprecated          integer     not null,
    primary key (id)
) type=InnoDB;

create table core_version (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    version_num         integer     not null,
    platformid          integer     not null,
    xml_doc             blob,
    message             varchar(254),
    deprecated          smallint    not null,
    primary key (id)
) type=InnoDB;

create table app (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    name                varchar(254) not null,
    min_version         integer     not null,
    deprecated          smallint    not null,
    user_friendly_name  varchar(254) not null,
    homogeneous_redundancy smallint not null,
    primary key (id)
) type=InnoDB;

create table app_version (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    appid               integer     not null,
    version_num         integer     not null,
    platformid          integer     not null,
    xml_doc             blob,
    min_core_version    integer     not null,
    max_core_version    integer     not null,
    deprecated          integer     not null,
    primary key (id)
) type=InnoDB;

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
    seti_id             integer     ,
    seti_nresults       integer     not null,
    seti_last_result_time   integer not null,
    seti_total_cpu      double      not null,
    signature           varchar(254),
    has_profile         smallint    not null,
    cross_project_id    varchar(254) not null,
    primary key (id)
) type=InnoDB;

create table team (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    userid              integer     not null,
    name                varchar(254) not null,
    name_lc             varchar(254),
    url                 varchar(254),
    type                integer     not null,
    name_html           varchar(254),
    description         blob,
    nusers              integer     not null,   /* temp */
    country             varchar(254),
    total_credit        double      not null,   /* temp */
    expavg_credit       double      not null,   /* temp */
    expavg_time         double      not null,
    seti_id             integer     not null,
    primary key (id)
) type=InnoDB;

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

    primary key (id)
) type=InnoDB;

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
    primary key (id)
) type=InnoDB;

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
    primary key (id)
) type=InnoDB;

create table msg_from_host (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    hostid              integer     not null,
    variety             varchar(254) not null,
    handled             smallint    not null,
    xml                 text,
    primary key (id)
) type=InnoDB;

create table msg_to_host (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    hostid              integer     not null,
    variety             varchar(254) not null,
    handled             smallint    not null,
    xml                 text,
    primary key (id)
) type=InnoDB;

create table workseq (
    id                  integer     not null auto_increment,
    create_time         integer     not null,
    state               integer     not null,
    hostid              integer     not null,
    wuid_last_done      integer     not null,
    wuid_last_sent      integer     not null,
    workseqid_master    integer     not null,
    primary key (id)
) type=InnoDB;

-- EVERYTHING FROM HERE ON IS USED ONLY FROM PHP,
-- SO NOT IN BOINC_DB.H ETC.

-- represents a language (so can have message boards in different languages)
--
create table lang (
    id                  integer     not null auto_increment,
    name                varchar(254) not null,
    charset             varchar(254) not null,
    primary key (id)
) type=InnoDB;


-- user profile (description, pictures)
--
create table profile (
    userid              integer     not null auto_increment,
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
);

-- message board category
-- help desk is a group of categories that are handled separately
--
create table category (
    id                  integer     not null auto_increment,
    orderID             integer     not null,
        -- order in which to display
    lang                integer     not null,
    name                varchar(254) binary,
    is_helpdesk         smallint    not null,
    primary key (id)
) type=InnoDB;

-- message board topic
--
create table forum (
    id                  integer     not null auto_increment,
    category            integer     not null,
    orderID             integer     not null,
    title               varchar(254) not null,
    description         varchar(254) not null,
    timestamp           integer     not null,
        -- time of last new or modified thread or post
    threads             integer     not null,
    posts               integer     not null,
    primary key (id)
) type=InnoDB;

-- threads in a topic (or questions)
--
create table thread (
    id                  integer     not null auto_increment,
    forum               integer     not null,
    owner               integer     not null,
    title               varchar(254) not null,
    timestamp           integer     not null,
        -- time of last new or modified post
    views               integer     not null,
        -- number of times this has been viewed
    replies             integer     not null,
        -- number of postings in thread
    activity            double      not null,
        -- for questions: number of askers / time since asked
        -- (set periodically by update_forum_activity.php)
    sufferers           integer     not null,
        -- in help desk: # people who indicated they had same problem
    create_time         integer     not null,
        -- when this record was created
    hidden              integer     not null,
        -- nonzero if hidden by moderators
    primary key (id)
);

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
    signature           tinyint(1) unsigned not null default 0,
    hidden              integer     not null,
        -- nonzero if hidden by moderators
    primary key (id)
);

-- subscription to a thread
--
create table subscriptions (
    userid              integer     not null,
    threadid            integer     not null
) type=InnoDB;

create table forum_preferences (
    userid              integer     not null default 0,
    signature           varchar(254) not null default '',
    posts               integer     not null default 0,
    avatar              varchar(254) not null default '',
    avatar_type         tinyint(4)  not null default 0,
    hide_avatars        tinyint(1) unsigned not null default 0,
    sorting             varchar(100) not null default '',
    no_signature_by_default tinyint(1) unsigned not null default 0,
    images_as_links     tinyint(1) unsigned not null default 0,
    link_popup          tinyint(1) unsigned not null default 0,
    mark_as_read_timestamp integer not null default 0,
    special_user        integer not null default 0,
    jump_to_unread      tinyint(1) unsigned not null default 0,
    hide_signatures     tinyint(1) unsigned not null default 0,
    rated_posts         varchar(254) not null,
    low_rating_threshold integer not null,
    high_rating_threshold integer not null,
    ignorelist          varchar(254) not null,
    primary key (userid)
) type=MyISAM; 

create table forum_logging (
    userid              integer     not null default 0,
    threadid            integer     not null default 0,
    timestamp           integer     not null default 0,
    primary key (userid,threadid)
) TYPE=MyISAM;

create table tentative_user (
    nonce               varchar(254) not null,
    email_addr          varchar(254) not null,
    confirmed           integer not null,
    primary key(nonce)
);
