#!/bin/sh

# This script should be run under cron. It 
# (1) re-generates the statistics
# (2) produces the daily report
# (3) mails it to the list below

# 0 0 * * * bash /home/ps3grid/remote/monitoring/monitoring_cron.sh

# $Id$

mon_dir=  PATH TO MONITOR DIR
mysql="/usr/bin/mysql  -pPASSWORD DB_NAME"
workflow_results_dir= PATH TO WORKFLOW_RESULTS

mailto="SPACE-SEPARATED LIST OF EMAILS"



# End user serviceable parts
cd $mon_dir

# Compute the statistics
$mysql < monitoring_compute.sql


# Tmp file where to make the report
tmpfile=/tmp/monitoring_report.$$
touch $tmpfile



# ........................................
# Generate begin
echo "== Report begin ==" >> $tmpfile
date >> $tmpfile



# ........................................
# Per-user  summary
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Per-user summary ==" >> $tmpfile
$mysql <<EOF | column -t >>$tmpfile
select scientist,
       cur_inprogress    as sent,
       cur_unsent        as unsent,
       day_successful    as day_suc,
       day_unsuccessful  as day_unsuc,
       day_credits
  from monitoring 
 where monitor_time = 
	(select max(monitor_time) from monitoring);
EOF


# ........................................
# Per-user, per-error breakdown
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Per-user, per-error breakdown ==" >> $tmpfile
/usr/bin/perl monitoring_errors.pl >> $tmpfile




# ........................................
# Cancelled due to too many errors
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Chains failing due to too many errors in the last 24 hours ==" >> $tmpfile
$mysql <<EOF | column -t >>$tmpfile
select mon_wuname(name) as in_group,
       count(*) as how_many 
  from workunit 
 where (error_mask & 2) = 2 
   and mod_time > now() - interval 1 day
 group by in_group;
EOF





# ........................................
# Per-group  summary
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Per-group summary (in progress only) ==" >> $tmpfile
$mysql <<EOF | sed s/NULL/-/g | column -t >>$tmpfile
  SET @ut=unix_timestamp();
  SET @uf=@ut-3600*24;
  select  t_inprogress.n as group_name,
	  t_inprogress.c as sent,
	  t_unsent.c     as unsent,
	  t_success.c    as ok, 
	  t_clienterror.c as fail,
          t_severe.c     as sev,
          round(100*t_severe.c/t_success.c) as 's%'
  from ( 
	 select mon_wuname(name) as n, count(*) as c from result
	  where server_state=4  
	  group by n
  ) as t_inprogress 
  left join (
	 select mon_wuname(name) as n, count(*) as c from result 
	  where server_state=2  
	  group by n
  ) as t_unsent on t_inprogress.n=t_unsent.n 
  left join (
	 select mon_wuname(name) as n, count(*) as c from result 
	  where outcome=1 
	    and received_time  between  @uf and @ut 
	  group by n
  ) as t_success on t_inprogress.n=t_success.n
  left join (
	 select mon_wuname(name) as n, count(*) as c from result 
	  where outcome=3 
	    and received_time  between  @uf and @ut 
	  group by n
  ) as t_clienterror on t_inprogress.n=t_clienterror.n
  left join (
         select mon_wuname(name) as n, count(*) as c from result
          where outcome=3
            and elapsed_time>300
            and received_time  between@uf and @ut
          group by n
  ) as t_severe on t_inprogress.n=t_severe.n
EOF
cat >> $tmpfile <<EOF

Legend (counts over the last 24h) 
   ok: successful completion
 fail: client error 
  sev: client error, ran for > 5 min
   s%: % of severe failures (sev/ok)
EOF



# ........................................
# WU turnaround time
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Turnaround time of today's WUs (create-canonical received within X days) ==" >> $tmpfile
$mysql <<EOF | column -t >>$tmpfile
select if(ceil((r.received_time-w.create_time)/3600/24)<=6,
          ceil((r.received_time-w.create_time)/3600/24),
          '>6') as turnaround,
       count(*)
  from workunit w,result r 
 where canonical_resultid<>0 
   and canonical_resultid=r.id 
   and received_time > unix_timestamp()-3600*24
 group by turnaround
EOF


# ........................................
# Results turnaround time
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Turnaround time of today's results (sent-received within X days) ==" >> $tmpfile
$mysql <<EOF | column -t >>$tmpfile
select if(ceil((received_time-sent_time)/3600/24)<=6,
          ceil((received_time-sent_time)/3600/24),
          '>6')  as turnaround, count(*) 
   from     result 
   where    outcome=1 
   and      received_time > unix_timestamp()-3600*24
   group by turnaround
EOF



# ........................................
# To be duplicated
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Late WUs (2d) to be duplicated ==" >> $tmpfile
$mysql <<EOF | column -t >>$tmpfile
select  mon_wuname(workunit.name) as group_name, 
	count(*) as count, 
	round(avg(workunit.priority)) as w_pri, 
	round(avg(result.priority)) as r_pri,
	round(avg(datediff(now(),from_unixtime(result.sent_time))),1) as sent_age,
	round(avg(datediff(now(),from_unixtime(result.create_time))),1) as cre_age
   from     workunit,result  
   where    workunit.id=result.workunitid
   and      workunit.target_nresults=1
   and      result.sent_time<unix_timestamp()- 2*3600*24
   and      result.server_state=4
   group by group_name
EOF
cat >> $tmpfile <<EOF

Legend (counts over the last 24h) 
   count: no. of results sent before 3 days ago
   w_pri: average workunit priority
   r_pri: average result priority
sent_age: average days elapsed since result sent
 cre_age: average days elapsed since result created
EOF






# ........................................
# Disk occupation
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Disk occupation per group ==" >> $tmpfile
cd $workflow_results_dir
du -sch * >> $tmpfile
cd - > /dev/null


# ........................................
# Free disk space
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Disk space availability ==" >> $tmpfile
df -h >> $tmpfile






# ........................................
# Generate end
echo " " >> $tmpfile
echo " " >> $tmpfile
echo "== Report end ==" >> $tmpfile
date >> $tmpfile



# Mail the result in HTML
(echo "<html><body><pre>";
 cat $tmpfile;
 echo "</pre></body></html>")|
mutt -e 'set content_type="text/html"' \
     -s "GPUGRID monitoring report" $mailto

# Remove tmp
rm $tmpfile
