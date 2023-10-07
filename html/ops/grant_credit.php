<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// DEPRECATED - WON'T WORK.
// result.claimed_credit is not used any more

// Award credit to users/hosts/teams for WU which have been
// cancelled or have otherwise failed (error_mask != 0).
// Credit granted is credit claimed, with a hardwired limit of 300 units.
// To enable this script change 1 to 0 in the testquery() function.
// The script can be run multiple times without doing any harm.
// It only grants credit to results which do not (yet) have any
// granted credits.  So it can be run multiple times.


$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/credit.inc");

set_time_limit(0);

db_init();

// set variable to 0 to 'do it for real'

function testquery($argstring) {
    if (0) {
        echo "WOULD DO: $argstring\n";
    }
    else {
        _mysql_query($argstring);
    }
    return;
}

function grant_credits_for_wu($wuid) {
    $max_credit=300;
    $ndone = 0;
    $query_r = _mysql_query("select * from result where granted_credit=0 and claimed_credit>0 and workunitid=$wuid");

    while ($result = _mysql_fetch_object($query_r)) {
        echo "STARTING RESULT $result->id [Credit $result->claimed_credit] ...";
        $ndone++;

        $hostid  = $result->hostid;
        $query_h = _mysql_query("select * from host where id=$hostid");
        $host    = _mysql_fetch_object($query_h);

        $userid  = $result->userid;
        $query_u = _mysql_query("select * from user where id=$userid");
        $user    = _mysql_fetch_object($query_u);

        $credit = $result->claimed_credit;
        if ($credit>$max_credit) {
            $credit=$max_credit;
            echo " WARNING: USER $user->name ($userid) CLAIMED $result->claimed_credit CREDITS (getting $credit)!";
        }
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
            $query_t = _mysql_query("select * from team where id=$teamid");
            $team    = _mysql_fetch_object($query_t);
            $team->total_credit += $credit;
            update_average(time(0), $result->sent_time, $credit, $team->expavg_credit, $team->expavg_time);
            testquery("update team set total_credit=$team->total_credit, expavg_credit=$team->expavg_credit, expavg_time=$team->expavg_time where id=$teamid");
            _mysql_free_result($query_t);
        }
        _mysql_free_result($query_h);
        _mysql_free_result($query_u);
        echo " DONE\n";
    }
    _mysql_free_result($query_r);
    return $ndone;
}

function grant_credits_for_cancelled() {
    $ngranted=0;
    $query_w  = _mysql_query("select * from workunit where error_mask!=0");
    while (($workunit = _mysql_fetch_object($query_w))) {
        // echo "Starting WU $workunit->id\n";
        $ngranted += grant_credits_for_wu($workunit->id);
        // NEED TO SET assimilate_state=READY for WU!!
    }
    _mysql_free_result($query_w);

    echo "\nGranted credits to $ngranted results\n";
}

grant_credits_for_cancelled();

?>
