use BOINC_DB_NAME

create table project (
    id              integer     not null auto_increment,
    name            varchar(254) not null,
    primary key (id)
);

create table platform (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    name            varchar(254) not null,
    primary key (id)
);

create table app (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    name            varchar(254) not null,
    min_version     integer     not null,
    primary key (id)
);

create table app_version (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    appid           integer     not null,
    version_num     integer     not null,
    platformid      integer     not null,
    xml_doc         blob,
    min_core_version integer    not null,
    max_core_version integer    not null,
    message         varchar(254),
    deprecated      smallint    not null,
    primary key (id)
);

create table user (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    email_addr      varchar(254) not null,
    name            varchar(254),
    web_password    varchar(254),
    authenticator   varchar(254),
    country         varchar(254),
    postal_code     varchar(254),
    total_credit    double      not null,
    expavg_credit   double      not null,
    expavg_time     double      not null,
    global_prefs    blob,
    project_prefs   blob,
    teamid	    integer	not null,
    primary key (id)
);

create table team (
    id		    integer     not null auto_increment,
    userid	    integer	not null,
    name	    varchar(254) not null,
    name_lc	    varchar(254),
    url		    varchar(254),
    type	    integer	not null,
    name_html	    varchar(254),
    description     varchar(254),
    nusers          integer     not null,
    primary key (id)
);


create table host (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    userid          integer     not null,
    rpc_seqno       integer     not null,
    rpc_time        integer     not null,
    total_credit    double       not null,
    expavg_credit   double       not null,
    expavg_time     double       not null,

    timezone        integer     not null,
    domain_name     varchar(254),
    serialnum       varchar(254),
    last_ip_addr    varchar(254),
    nsame_ip_addr   integer     not null,

    on_frac         double       not null,
    connected_frac  double       not null,
    active_frac     double       not null,

    p_ncpus         integer     not null,
    p_vendor        varchar(254),
    p_model         varchar(254),
    p_fpops         double       not null,
    p_iops          double       not null,
    p_membw         double       not null,

    os_name         varchar(254),
    os_version      varchar(254),

    m_nbytes        double       not null,
    m_cache         double       not null,
    m_swap          double       not null,

    d_total         double       not null,
    d_free          double       not null,

    n_bwup          double       not null,
    n_bwdown        double       not null,

    credit_per_cpu_sec double    not null,

    primary key (id)
);

create table workunit (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    appid           integer     not null,
    previous_wuid   integer     not null,
    has_successor   smallint    not null,
    name            varchar(254) not null,
    xml_doc         blob,
    batch           integer     not null,
    rsc_fpops       double      not null,
    rsc_iops        double      not null,
    rsc_memory      double      not null,
    rsc_disk        double      not null,
    need_validate   smallint    not null,
    canonical_resultid integer  not null,
    canonical_credit double     not null,
    primary key (id)
);

create table result (
    id              integer     not null auto_increment,
    create_time     integer     not null,
    workunitid      integer     not null,
    state           integer     not null,
    hostid          integer     not null,
    report_deadline integer     not null,
    sent_time       integer     not null,
    received_time   integer     not null,
    name            varchar(254) not null,
    exit_status     integer     not null,
    cpu_time        double       not null,
    xml_doc_in      blob,
    xml_doc_out     blob,
    stderr_out      blob,
    batch           integer     not null,
    project_state   integer     not null,
    validate_state  integer     not null,
    claimed_credit  double       not null,
    granted_credit  double       not null,
    primary key (id)
);
