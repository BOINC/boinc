<?

// This is a script which awards credit to users/hosts/teams for WU which have been
// cancelled or have otherwise failed (error_mask != 0).  Credit granted is credit
// claimed, with a hardwired limit of 300 units.  To enable this script change 1 to
// 0 in the testquery() function. The script can be run multiple times without
// doing any harm.  It only grants credit to results which do not (yet) have any
// granted credits.  So it can be run multiple times.


require_once("../inc/db.inc");
require_once("../inc/credit.inc");

set_time_limit(0);

db_init();

// set variable to 0 to 'do it for real'

function testquery($argstring) {
    if (0) {
        echo "WOULD DO: $argstring\n";
    }
    else {
        mysql_query($argstring);
    }
    return;
}

function grant_credits_for_wu($wuid) {
    $query_r = mysql_query("select * from result where granted_credit=0 and claimed_credit>0 and workunitid=$wuid");
    while ($result = mysql_fetch_object($query_r)) {
        echo "STARTING RESULT $result->id [Credit $result->claimed_credit]\n";

        $hostid  = $result->hostid;
        $query_h = mysql_query("select * from host where id=$hostid");
        $host    = mysql_fetch_object($query_h);

        $userid  = $result->userid;
        $query_u = mysql_query("select * from user where id=$userid");
        $user    = mysql_fetch_object($query_u);

        $credit = $result->claimed_credit;
        if ($credit>300) $credit=300;
        $user->total_credit += $credit;
        update_average(time(0), $result->sent_time, $credit, $user->expavg_credit, $user->expavg_time);

        $host->total_credit += $credit;
        update_average(time(0), $result->sent_time, $credit, $host->expavg_credit, $host->expavg_time);

        $turnaround = $result->received_time - $result->sent_time;
        if ($host->avg_turnaround > 0)
            $host->avg_turnaround = 0.7*$host->avg_turnaround + 0.3*$turnaround;
        else
            $host->avg_turnaround = $turnaround;

        testquery("update result set granted_credit=$credit where id=$result->id");

        testquery("update user set total_credit=$user->total_credit, expavg_credit=$user->expavg_credit, expavg_time=$user->expavg_time where id=$userid");

        testquery("update host set total_credit=$host->total_credit, expavg_credit=$host->expavg_credit, expavg_time=$host->expavg_time, avg_turnaround=$host->avg_turnaround where id=$hostid");

        $teamid = $user->teamid;
        if ($teamid) {
            $query_t = mysql_query("select * from team where id=$teamid");
            $team    = mysql_fetch_object($query_t);
            $team->total_credit += $credit;
            update_average(time(0), $result->sent_time, $credit, $team->expavg_credit, $team->expavg_time);
            testquery("update team set total_credit=$team->total_credit, expavg_credit=$team->expavg_credit, expavg_time=$team->expavg_time where id=$teamid");
            mysql_free_result($query_t);
        }
        mysql_free_result($query_h);
        mysql_free_result($query_u);
        echo "DONE WITH RESULT $result->id\n\n";
    }
    mysql_free_result($query_r);
    return;
}

function grant_credits_for_cancelled() {

    $query_w  = mysql_query("select * from workunit where error_mask!=0");
    while (($workunit = mysql_fetch_object($query_w))) {
        echo "Starting WU $workunit->id\n";
        grant_credits_for_wu($workunit->id);
        // NEED TO SET assimilate_state=READY for WU!!
    }
    mysql_free_result($query_w);
}

grant_credits_for_cancelled();

?>
