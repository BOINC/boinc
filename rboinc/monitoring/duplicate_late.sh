#/bin/bash

# $Id$

# Duplicates workunits which are late (sent time < 3 days ago),
# state=INPROGRESS.  Does not duplicate WUs which have been already
# duplicated. Duplicates get target_nresults=2 and priority += 1000.


mysql="/usr/bin/mysql  -pxxxx DBNAME"

echo "$0 running on"
date

echo "Going to duplicate the following workunits"


$mysql <<EOF | column -t
select workunit.name as wu_name,
       result.id as resultid,
       from_unixtime(result.sent_time) as sent_time
from   workunit ,result  
where  workunit.id=result.workunitid    
   and workunit.target_nresults=1                                                                                                                                                               
   and result.sent_time<unix_timestamp()- 2*3600*24                                                                                                                                             
   and result.server_state=4     
order by sent_time
EOF

$mysql <<EOF
update workunit,result
  set workunit.target_nresults=2,
    workunit.transition_time=unix_timestamp(),
    workunit.priority=workunit.priority+1000
 where
    workunit.id=result.workunitid
    and workunit.target_nresults=1
    and result.sent_time<unix_timestamp()- 2*3600*24
    and result.server_state=4
EOF

echo "Executed $0 with return code $?"

