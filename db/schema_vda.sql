create table vda_file (
    id                      integer not null auto_increment,
    create_time             double not null default 0,
    dir                     varchar(254) not null,
    file_name               varchar(254) not null,
    size                    double not null default 0,
    chunk_size              double not null default 0,
    need_update             tinyint not null default 0,
    initialized             tinyint not null default 0,
    retrieving              tinyint not null default 0,
    primary key(id)
) engine = InnoDB;

alter table vda_file add unique(file_name);

create table vda_chunk_host (
    create_time             double not null default 0,
    vda_file_id             integer not null default 0,
    host_id                 integer not null default 0,
    chunk_name              varchar(254) not null,
    present_on_host         tinyint not null default 0,
    transfer_in_progress    tinyint not null default 0,
    transfer_wait           tinyint not null default 0,
    transfer_request_time   double not null default 0,
    transfer_send_time      double not null default 0 
) engine = InnoDB;

alter table vda_chunk_host
    add index vch_file (vda_file_id),
    add index vch_host (host_id);

alter table host
    add index host_cpu_eff (cpu_efficiency);
