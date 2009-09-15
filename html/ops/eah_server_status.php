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

require_once("../inc/cache.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/db.inc");
require_once("../project/project.inc");

################################################
# local functions

function numerical_query($query) {
    // execute a database query which returns a single numerical result
    $result = mysql_query("$query");
    $x = mysql_fetch_object($result);
    return $x->total;
}

function count_estimate($query) {
    // this use of explain is way off at least for low counts -EAM 28Sep2004
    //$result = mysql_query("explain $query");
    $result = mysql_query("$query");
    $x = mysql_fetch_object($result);
    //    return $x->rows-1;
    return $x->total;
}

function find_oldest() {
   $result=mysql_query("select name,create_time from result where server_state=2 order by create_time limit 1");
   $x = mysql_fetch_object($result);
   return $x->create_time;
}

function daemon_status($host, $pidname) {
    $path = "../../pid_$host/$pidname.pid";
    $running = false;
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            // This needs to be set to work via ssh to other hosts
            //$foo = exec("/usr/bin/ssh $host ps w $pid");
            $foo = exec("ps w $pid");
            if ($foo) {
                if (strstr($foo, $pidname)) {
                    $running = true;
                }
            }
        }
    }
    return $running;
}

function show_status($host, $function, $running) {
    echo "<tr><td>$function</td><td>$host</td>";
    if ($running) {
        echo "<td bgcolor=00ff00>Running</td>\n";
    } else {
        echo "<td bgcolor=ff0000>Not running</td>\n";
    }
}

function show_daemon_status($host, $progname, $pidname) {
    $running = daemon_status($host, $pidname);
    show_status($host, $progname, $running);
}


###############################################
# BEGIN:

start_cache(1800);  
$Nmin = $cached_max_age/60;



$dbrc = db_init(1);    // 1=soft, remember that DB might be down

page_head(PROJECT . " - Server Status");


// Date stamp


echo "<br ALIGN=RIGHT> ".PROJECT. " server status as of ".
    date("g:i A T"). " on ". date("l, j F Y ") .
    " (updated every $Nmin minutes).\n";

$proc_uptime=exec("cat /proc/uptime | cut -d\" \" -f-1");
$days = (int)($proc_uptime/86400);
$hours=(int)($proc_uptime/3600);
$hours=$hours % 24;
$minutes=(int)($proc_uptime/60);
$minutes=$minutes % 60;
echo "<br ALIGN=RIGHT>The ".PROJECT. " main server has been continuously up for ". "$days"." days "."$hours"." hours "."$minutes"." minutes.\n<P>";

// tables side by side
echo "<TABLE><TR><TD align=center> \n";


echo "
    <h2>Server status</h2>
    <table border=2 cellpadding=6>
    <tr><th>Program</th><th>Host</th><th>Status</th></tr>
";



$web_running = !file_exists("../../stop_web");
show_status("einstein", "Web server", $web_running);




show_daemon_status("einstein", "Pulsar work generator (LHO)", "make_pulsar_WU_daemon_h");
show_daemon_status("einstein", "Pulsar work generator (LLO)", "make_pulsar_WU_daemon_l");
show_daemon_status("einstein", "BOINC database feeder", "feeder");
show_daemon_status("einstein", "BOINC transitioner", "transitioner");
$sched_running = !file_exists("../../stop_sched");
show_status("einstein", "BOINC scheduler", $sched_running);
show_daemon_status("einstein", "Einstein validator", "einstein_validator"); 
show_daemon_status("einstein", "Einstein assimilator", "einstein_assimilator");
show_daemon_status("einstein", "BOINC file deleter", "file_deleter");
show_daemon_status("einstein", "BOINC database purger", "db_purge");


echo "\n    </table> 
	</TD><TD>&nbsp;</TD><TD VALIGN=TOP align=center>
	\n";


echo "

    <h2>Users and Computers</h2>
";


if ($dbrc) {
    echo "The database server is not accessable";
} else {
    $now=time(0);
    $s_day=24*3600;
    $d_ago=$now-$s_day;
    $s_week=7*$s_day;
    $w_ago=$now-$s_week;

    echo "
        <table border=2 cellpadding=6>
        <tr><th>USERS</th><th>Approximate #</th></tr>
    ";
    $n = count_estimate("select count(*) as total from user");
    echo "
        <tr><td>in database</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(*) as total from user where total_credit>0");
    echo "
        <tr><td>with credit</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(*) as total from user where create_time > $d_ago");
    echo "
        <tr><td>registered in past 24 hours</td><td>".number_format($n)."</td></tr>
    ";

    echo "
        <tr><th align=center>HOST COMPUTERS</th><th>Approximate #</th></tr>
    ";

    $n = count_estimate("select count(*) as total from host");
    echo "
        <tr><td>in database</td><td>".number_format($n)."</td></tr>
    ";
    $n = count_estimate("select count(*) as total from host where create_time > $d_ago");
    echo "
        <tr><td>registered in past 24 hours</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(*) as total from host where total_credit>0");
    echo "
        <tr><td>with credit</td><td>".number_format($n)."</td></tr>
    ";
    $n = count_estimate("select count(id) as total from host where rpc_time>$w_ago");
    echo "
        <tr><td>active in past 7 days</td><td>".number_format($n)."</td></tr>
    ";
    $n = count_estimate("select sum(p_fpops) as total from host")/1000000000;
    // echo "
    //    <tr><td>floating point speed</td><td>".number_format($n)." GFLOPS</td></tr>
    //";
    printf("<tr><td>floating point speed<sup>1)</sup></td><td>%.1f TFLOPS</td></tr>", $n/1000);

    $n = count_estimate("select sum(p_fpops) as total from host where rpc_time>$w_ago")/1000000000;
    // echo "
    //    <tr><td>GFLOPS in past 7 days</td><td>".number_format($n)." GFLOPS</td></tr>
    //";
    printf("<tr><td>floating point speed in past 7 days<sup>2)</sup></td><td>%.1f TFLOPS</td></tr>", $n/1000);

    $n = numerical_query("SELECT SUM(cpu_time * p_fpops) / $s_week AS total FROM result,host where outcome = '1' AND (received_time > $w_ago) AND (result.hostid = host.id )")/1000000000;
    printf("<tr><td>floating point speed from results<sup>3)</sup></td><td>%.1f TFLOPS</td></tr>", $n/1000);

    echo "\n    </table>
        </TD><TD>&nbsp;</TD><TD VALIGN=TOP align=center>
            <h2>Work and Results</h2>
        \n";
                                                                                                                               


    echo "
        <table border=2 cellpadding=6>
    ";

    echo "
        <tr><th>WORKUNITS</th><th>Approximate #</th></tr>
    ";

    $n = count_estimate("select count(*) as total from workunit");
    echo "
        <tr><td>in database</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(*) as total from workunit where canonical_resultid!=0");
    echo "
        <tr><td>with canonical result</td><td>".number_format($n)."</td></tr>
    ";

    echo "
        <tr><th>RESULTS</th><th>Approximate #</th></tr>
    ";

    $n = count_estimate("select count(*) as total from result");
    echo "
        <tr><td>in database</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(id) as total from result where server_state=2");
    echo "
        <tr><td>unsent</td><td>".number_format($n)."</td></tr>
    ";
    $n = count_estimate("select count(id) as total from result where server_state=4");
    echo "
        <tr><td>in progress</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(id) as total from result where server_state=5 and file_delete_state=2");
    echo "
        <tr><td>deleted</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(id) as total from result where server_state=5 and outcome=1 and validate_state=1");
    echo "
        <tr><td>valid</td><td>".number_format($n)."</td></tr>
    ";

    $n = numerical_query("SELECT COUNT(id) AS total FROM result WHERE server_state=5 AND outcome=1 AND validate_state=1 AND ( received_time > $w_ago )");
    echo "
        <tr><td>valid last week</td><td>".number_format($n)."</td></tr>
    ";

    $n = count_estimate("select count(id) as total from result where server_state=5 and outcome=1 and validate_state=2");
    echo "
        <tr><td>invalid</td><td>".number_format($n)."</td></tr>
    ";

    $n = time(0)-find_oldest();
    $days = (int)($n/86400);
    $hours=(int)($n/3600);
    $hours=$hours % 24;
    $minutes=(int)($n/60);
    $minutes=$minutes % 60;
    echo "
        <tr><td>Oldest Unsent Result</td><td>".$days." d ".$hours." h ".$minutes." m</td></tr>
    ";



    echo "
        </table>
    ";
}

// Server restrictions

// Display cgi-bin restriction status

if (  file_exists("../../cgi-bin/.htaccess") ) {
    echo "<P><font color=RED>
        <b>The ".PROJECT." scheduler is currently restricted 
	  to uwm.edu and a few other domains.

	</b></font><P>
     ";
} 


echo "</TD></TR>
	</TABLE>
    ";  

    echo "<br> 1) the sum of the benchmarked FLops/s of all hosts in the database";
    echo "<br> 2) the sum of the benchmarked FLops/s of all hosts that have contacted the Einstein@Home scheduler within the past week";
    echo "<br> 3) the sum of the FLops of all valid results from last week divided by the number of seconds in a week";
                                                                                                                                                             


page_tail();

end_cache(600);
?>
