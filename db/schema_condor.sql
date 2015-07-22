create table batch_file_assoc (
    job_file_id             integer         not null,
    batch_id                integer         not null
) engine=InnoDB;

alter table batch_file_assoc
    add unique(job_file_id, batch_id);

alter table workunit
    add index wu_batch(batch);
