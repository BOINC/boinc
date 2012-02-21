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

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/email.inc");

// activate/deactivate script
if (0) {
  echo "
This script needs to be activated before it can be run.
Once you understand what the script does you can change the 
if (1) to if (0) at the top of the file to activate it.
Be sure to deactivate the script after using it to make sure
it is not accidentally run. 
";
  exit;
}

db_init();

function send_problem_email($user, $host) {
    global $master_url;
    $body = "";
        
    $host_content = "ID: ".$host->id."
    Created: ".time_str($host->create_time)."
    Venue: ".$host->venue."
    Total credit: ".$host->total_credit."
    Average credit: ".$host->expavg_credit."
    Average update time: ".time_str($host->expavg_time)."
    IP address: $host->last_ip_addr (same the last $host->nsame_ip_addr times)
    Domain name: " . $host->domain_name;
    $x = $host->timezone/3600;
    if ($x >= 0) $x="+$x";
    $host_content.="
    Local Time = UTC $x hours
    Number of CPUs: " . $host->p_ncpus."
    CPU: $host->p_vendor $host->p_model
    FP ops/sec: ".$host->p_fpops."
    Int ops/sec: ".$host->p_iops."
    memory bandwidth: ".$host->p_membw."
    Operating System: $host->os_name $host->os_version";
    $x = $host->m_nbytes/(1024*1024);
    $y = round($x, 2);
    $host_content.="
    Memory: $y MB";
    $x = $host->m_cache/1024;
    $y = round($x, 2);
    $host_content.="
    Cache: $y KB";
    $x = $host->m_swap/(1024*1024);
    $y = round($x, 2);
    $host_content.="
    Swap Space: $y MB";
    $x = $host->d_total/(1024*1024*1024);
    $y = round($x, 2);
    $host_content.="
    Total Disk Space: $y GB";
    $x = $host->d_free/(1024*1024*1024);
    $y = round($x, 2);
    $host_content.="
    Free Disk Space: $y GB
    Avg network bandwidth (upstream): $host->n_bwup bytes/sec
    Avg network bandwidth (downstream): $host->n_bwdown bytes/sec";
    $x = $host->avg_turnaround/86400;
    $host_content.="
    Average turnaround: ".round($x, 2)." days
    Number of RPCs: $host->rpc_seqno
    Last RPC: ".time_str($host->rpc_time)."
    % of time client on: ". 100*$host->on_frac." %
    % of time host connected: " . 100*$host->connected_frac." %
    % of time user active: " . 100*$host->active_frac." %
    # of results today: " . $host->nresults_today;
    
    $subject = PROJECT." notice for $user->name";
    $body = PROJECT." notification:

Dear $user->name
Your machine (host # $host->id) described below appears to have a misconfigured BOINC
installation.  Could you please have a look at it?

Sincerely,
        The ". PROJECT." team
";
$body .= "

This is the content of our database:
" . $host_content."

For further information and assistance with ".PROJECT." go to $master_url";
    echo nl2br($body) . "<br><br>"; 
    return send_email($user, $subject, $body);

}


$hostid = $_GET["hostid"];

if (!$hostid) {
    admin_page_head("Misconfigured Host");
    echo "This script sends an email to the owner of the supplied host which says that something gone wrong with his configuration.<br>";
    echo "<br><form method=\"get\" action=\"problem_host.php\">
    Host ID: 
    <input type=\"text\" size=\"5\" name=\"hostid\">
    <input type=\"submit\" value=\"Send Email\">
    </form>
    ";
} else {
    $res = mysql_query("select * from host where id='$hostid'");
    $host = mysql_fetch_object($res);
    if (!$host) {
    	echo "<h2>No host with that ID</h2>
	 	<center>Please <a href=\"problem_host.php\">try again</a></center>";
    } else {
    	$res = mysql_query("select * from user where id='$host->userid'");
    	$user = mysql_fetch_object($res);
    	echo "<a href=\"problem_host.php\">Do another?</a><br><br>";
    	send_problem_email($user, $host);
    	echo "Email to ".$user->email_addr." has been sent.<br>";
	}
}

admin_page_tail();
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
