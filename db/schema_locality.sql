create table file (
    id          integer not null auto_increment,
    name        varchar(254) not null,
    md5sum      varchar(32) not null,
    size        double not null default 0,
    primary key (id)
) engine=InnoDB;

alter table file add unique(name);

create table fileset (
    id          integer not null auto_increment,
    name        varchar(254) not null,
    primary key (id)
) engine=InnoDB;

alter table fileset add unique(name);

create table fileset_file (
    fileset_id  integer not null,
    file_id     integer not null,
    primary key (fileset_id, file_id),
    foreign key (fileset_id) references fileset(id) on delete cascade,
    foreign key (file_id) references file(id) on delete cascade
) engine=InnoDB;

create table sched_trigger (
    id                  integer not null auto_increment,
    fileset_id          integer not null,
    need_work           integer not null default 0,
    work_available      integer not null default 0,
    no_work_available   integer not null default 0,
    working_set_removal integer not null default 0,
    primary key (id)
) engine=InnoDB;

alter table sched_trigger
    add constraint foreign key(fileset_id) references fileset(id),
    add unique(fileset_id),
    add index(need_work),
    add index(work_available),
    add index(no_work_available),
    add index(working_set_removal);

alter table workunit
    add index (fileset_id);
