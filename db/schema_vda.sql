create table vda_file (
    id                      integer not null auto_increment,
    dir                     varchar(254) not null,
    name                    varchar(254) not null,
    size                    double not null default 0,
    chunk_size              double not null default 0,
    created                 double not null default 0,
    need_update             tinyint not null default 0,
    inited                  tinyint not null default 0,
    primary key(id)
) engine = InnoDB;

alter table vda_file add unique(name);

create table vda_chunk_host (
    vda_file_id             integer not null default 0,
    name                    varchar[256] not null,
    host_id                 integer not null default 0,
    present_on_host         tinyint not null default 0,
    transfer_in_progress    tinyint not null default 0,
    transfer_wait           tinyint not null default 0,
    transition_time         double not null default 0
) engine = InnoDB;

alter table vda_chunk_host
     add index vch_file (vda_file_id),
     add index vch_host (host_id),
     add index vch_tt (transition_time);

alter table host
    add index host_rpc_time (rpc_time);
