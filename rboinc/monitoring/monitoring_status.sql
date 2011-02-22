-- $Id$

-- mon_status stored procedure. 

-- Monitoring for "gridstatus" operation. Returns currently running
-- WUs for a given user. Can be run on demand from the command line,
-- no cron is involved.

-- Input arguments: scientist name
-- Output: a resultset containing the current job statuses




DELIMITER $$

DROP PROCEDURE IF EXISTS mon_status$$
CREATE PROCEDURE mon_status (in scientistname varchar(100))
BEGIN

-- 
-- monitoring interval to and from
set @ut=unix_timestamp();
set @tfrom=@ut-30*3600*24;	-- 1 week

-- date when the current naming conventions started
-- for later wus it is possible to reliably extract scientist names
-- set @begin=unix_timestamp('2009-06-01');
set @begin=@ut-30*3600*24;


set @who=scientistname;

drop table if  exists tmp_status;
create temporary  table  tmp_status (
	id              int(11)         not null auto_increment,
	grp_sent      	timestamp null default null,
	grp_name        varchar(100),
	cur_unsent       int(11) ,
	cur_inprogress   int(11) ,
	per_successful   int(11) ,
	per_unsuccessful int(11) ,
	per_credits      int(11) ,
	range_priority	varchar(11) ,
	primary key (id),
	unique key grp (grp_name)
) ENGINE=InnoDB;



-- Create rows for today's monitoring
insert into tmp_status (grp_name,grp_sent) 
       select mon_groupof(name) agrp, from_unixtime(min(create_time)) from result 
              where create_time>@begin
	      and mon_submitterof(name) like @who
	      group by agrp;


-- in progress
create temporary table tmp_inprogress 
       select count(*) inpro, mon_groupof(name) grp from result 
              where server_state=4 
              and   create_time>@begin
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_inprogress as tmp
       set tmp_status.cur_inprogress=tmp.inpro
       where tmp_status.grp_name=grp;

drop table tmp_inprogress;




-- unsent
create temporary table tmp_uns 
       select count(*) uns, mon_groupof(name) grp from result 
              where server_state=2
              and   create_time>@begin
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_uns as tmp
       set tmp_status.cur_unsent=tmp.uns
       where tmp_status.grp_name=grp;

drop table tmp_uns;



-- day success
create temporary table tmp_suc
       select count(*) suc, mon_groupof(name) grp from result 
              where validate_state=1
              and   received_time>@tfrom
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_suc as tmp
       set tmp_status.per_successful=tmp.suc
       where tmp_status.grp_name=grp;

drop table tmp_suc;



-- day success
create temporary table tmp_uns
       select count(*) uns, mon_groupof(name) grp from result 
              where validate_state=2
              and   received_time>@tfrom
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_uns as tmp
       set tmp_status.per_unsuccessful=tmp.uns
       where tmp_status.grp_name=grp;

drop table tmp_uns;




-- day credits
create temporary table tmp_cr
       select sum(granted_credit) credits, mon_groupof(name) grp from result 
              where validate_state=1
              and   received_time>@tfrom
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_cr as tmp
       set tmp_status.per_credits=tmp.credits
       where tmp_status.grp_name=grp;

drop table tmp_cr;


-- avg priority
create temporary table tmp_pri
       select concat(min(priority),'-',max(priority)) pr, mon_groupof(name) grp from workunit
              where assimilate_state=0
	      and mon_submitterof(name) like @who
	      group by grp;

update tmp_status,tmp_pri as tmp
       set tmp_status.range_priority=tmp.pr
       where tmp_status.grp_name=grp;

drop table tmp_pri;



select * from tmp_status order by grp_sent;

-- select grp_sent, grp_name, cur_unsent unsent, 
--        cur_inprogress in_prog, per_successful success,
--        per_unsuccessful unsuccess, per_credits credits,
--        range_priority priority
--        from tmp_status order by grp_sent;



END$$

DELIMITER ;
