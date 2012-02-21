-- $Id$

-- Should be run from crontab. It will add a row in the "monitoring"
-- table with the results, for later retrieval from the "dailyreport"
-- script.



-- monitoring interval to and from
set @ut=unix_timestamp();
set @uf=@ut-3600*24; -- could be the second to latest monitor_time

-- date when the current naming conventions started
-- for later wus it is possible to reliably extract scientist names
set @begin=unix_timestamp('2009-06-01');


-- Create rows for today's monitoring
insert into monitoring (monitor_time,scientist) 
       select @ut monitor_time, mon_submitterof(name) sci from result 
              where create_time>@begin
	      group by sci;


-- in progress
create temporary table tmp_inprogress 
       select count(*) inpro, mon_submitterof(name) sci from result 
              where server_state=4 
              and   create_time>@begin
	      group by sci;

update monitoring,tmp_inprogress as tmp
       set monitoring.cur_inprogress=tmp.inpro
       where monitoring.monitor_time=@ut
       and   monitoring.scientist=tmp.sci;

drop table tmp_inprogress;


-- unsent
create temporary table tmp_unsent
       select count(*) unsent, mon_submitterof(name) sci from result 
              where server_state=2 
              and   create_time>@begin
	      group by sci;

update monitoring,tmp_unsent as tmp
       set monitoring.cur_unsent=tmp.unsent
       where monitoring.monitor_time=@ut
       and   monitoring.scientist=tmp.sci;

drop table tmp_unsent;


-- day_successful
create temporary table tmp_suc
       select count(*) suc, mon_submitterof(name) sci from result 
       	      where  validate_state=1 
	      and    received_time  between  @uf and @ut 
       	      group by sci;

update monitoring,tmp_suc as tmp
       set monitoring.day_successful=tmp.suc
       where monitoring.monitor_time=@ut
       and   monitoring.scientist=tmp.sci;

drop table tmp_suc;




-- day_unsuccessful
create temporary table tmp_unsuc
       select count(*) unsuc, mon_submitterof(name) sci from result 
       	      where  validate_state=2
	      and    received_time  between  @uf and @ut 
       	      group by sci;

update monitoring,tmp_unsuc as tmp
       set monitoring.day_unsuccessful=tmp.unsuc
       where monitoring.monitor_time=@ut
       and   monitoring.scientist=tmp.sci;

drop table tmp_unsuc;



-- day_credits
create temporary table tmp_cred
       select sum(granted_credit) credits, mon_submitterof(name) sci from result 
       	      where  validate_state=1
       	      and    received_time  between  @uf and @ut 
       	      group by sci;

update monitoring,tmp_cred as tmp
       set monitoring.day_credits=tmp.credits
       where monitoring.monitor_time=@ut
       and   monitoring.scientist=tmp.sci;

drop table tmp_cred;


