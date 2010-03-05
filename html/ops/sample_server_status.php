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

// server_status.php [-f xml_output_filename]
//   (or server_status.php?xml=1)
//
// outputs general information about BOINC server status gathered from
// config.xml or mysql database queries. If you are running all your
// services on one machine, and the database isn't so large, this should
// work right out of the box. Otherwise see configureables below.
//
// Daemons in config.xml are checked to see if they are running by ssh'ing
// into the respective hosts and searching for active pids. Passwordless
// logins must be in effect if there are multiple hosts involved.
//
// The database queries may be very slow. You might consider running these
// queries elsewhere via cronjob, outputing numbers into a readable file,
// and then getting the latest values with a `/bin/tail -1 data_file`.
// See commented example in the code.
//
// You can get an xml version of the stats via the web when the url has the
// optional "?xml=1" tag at the end, i.e 
//   http://yourboincproject.edu/server_status.php?xml=1
//
// If running as a standalone program there is an optional -f flag where
// you can generate xml server status output to the filename you provide
// (this will output both html to stdout and xml to the filename given).
// Some may prefer to do this if takes so long to dredge up the stats for
// the html, you won't have to do it again to generate the xml.
//
// It is highly recommended that you run this program every 10 minutes and
// send its stdout to an .html file, rather than having the values get
// regenerated every time the page is accessed. Or use the available
// web page cache utilities.
//
// You should edit the following variables in config.xml to suit your needs:
//
// <www_host>    hostname of web server (default: same as <host>)
// <sched_host>  hostname of scheduling server (default: same as <host>)
// <uldl_host>   hostname of upload/download server (default: same as <host>)
// <uldl_pid>    pid file of upload/download server httpd.conf
//               (default: /etc/httpd/run/httpd.pid)
// <ssh_exe>     path to ssh (default: /usr/bin/ssh)
// <ps_exe>      path to ps (which supports "w" flag) (default: /bin/ps)

///////////////////////////////////////////////////////////////////////////////

require_once("../inc/util_ops.inc");
require_once("../inc/xml.inc");
require_once("../inc/cache.inc");

$xml = get_int("xml", true);

$cache_args = "";
if ($xml) $cache_args = "xml=1";
$cache_period = 600;
start_cache($cache_period, $cache_args);

$xmlout = "";
if ($argc>0 && $argv[1] == "-f") {
    $xmlout = $argv[2];
    $xmloutfile = fopen($xmlout,"w+");
    if (!$xmloutfile) {
        die( "failed to open file: $xmlout");
    }
}

// daemon status outputs: 1 (running) 0 (not running) or -1 (disabled)
//
function daemon_status($host, $pidname, $progname, $disabled) {
    global $ssh_exe, $ps_exe, $project_host;
    $path = "../../pid_$host/$pidname.pid";
    $running = 0;
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            $pid = trim($pid);
            $command = "$ps_exe ww $pid";
            if ($host != $project_host) {
                $command = "$ssh_exe $host " . $command;
            }
            $foo = exec($command);
            if ($foo) {
                if (strstr($foo, (string)$pid)) $running = 1;
            }
        }
    }
    if ($disabled == 1) $running = -1;
    return $running;
}

function show_status($host, $function, $running) {
    global $xml,$xmlout,$xmloutfile;
    $xmlstring = "    <daemon>\n      <host>$host</host>\n      <command>$function</command>\n";
    $htmlstring = "<tr><td>$function</td><td>$host</td>";
    if ($running == 1) {
        $xmlstring .= "      <status>running</status>\n";
        $htmlstring .= "<td class=\"running\">Running</td>\n";
    } elseif ($running == 0) {
        $xmlstring .= "      <status>not running</status>\n";
        $htmlstring .= "<td class=\"notrunning\">Not Running</td>\n";
    } else {
        $xmlstring .= "      <status>disabled</status>\n";
        $htmlstring .= "<td class=\"disabled\">Disabled</td>\n";
    }
    $xmlstring .= "    </daemon>\n";
    $htmlstring .= "</tr>\n";
    if ($xml) {
        echo $xmlstring; return 0;
    }
    if ($xmlout) {
        fwrite($xmloutfile, $xmlstring);
    }
    echo $htmlstring;
    return 0;
}

function show_daemon_status($host, $pidname, $progname, $disabled) {
    $running = daemon_status($host, $pidname, $progname, $disabled);
    show_status($host, $pidname, $running);
}

function show_counts($key, $xmlkey, $value) {
    global $xml,$xmlout,$xmloutfile;
    $formattedvalue = number_format($value);
    $xmlstring = "    <$xmlkey>$value</$xmlkey>\n";
    if ($xml) {
        echo $xmlstring;
        return 0;
    }
    if ($xmlout) {
        fwrite($xmloutfile,$xmlstring);
    }
    echo "<tr><td>$key</td><td>$formattedvalue</td></tr>";
    return 0;
}

function get_mysql_count ($query) {
    $result = mysql_query("select count(*) as count from " . $query);
    $count = mysql_fetch_object($result);
    mysql_free_result($result);
    return $count->count;
}

$config_xml = get_config();
$config_vars = parse_element($config_xml,"<config>");
$project_host = parse_element($config_vars,"<host>");
$www_host = parse_element($config_vars,"<www_host>");
if ($www_host == "") {
    $www_host = $project_host;
}
$sched_host = parse_element($config_vars,"<sched_host>");
if ($sched_host == "") {
    $sched_host = $project_host;
}
$uldl_pid = parse_element($config_vars,"<uldl_pid>");
if ($uldl_pid == "") {
    $uldl_pid = "/etc/httpd/run/httpd.pid";
}
$uldl_host = parse_element($config_vars,"<uldl_host>");
if ($uldl_host == "") {
    $uldl_host = $project_host;
}
$ssh_exe = parse_element($config_vars,"<ssh_exe>");
if ($ssh_exe == "") {
    $ssh_exe = "/usr/bin/ssh";
} 
$ps_exe = parse_element($config_vars,"<ps_exe>");
if ($ps_exe == "") {
    $ps_exe = "/bin/ps";
}

$version = null;
if (file_exists("../../local.revision")) {
    $version = trim(file_get_contents("../../local.revision"));
}
$now = time();

$xmlstring = "<server_status>
  <update_time>$now</update_time>
";
if ($version) {
    $xmlstring .= "<software_version>$version</software_version>\n";
}
$xmlstring .= "  <daemon_status>\n";
if ($xml) {
    xml_header();
    echo $xmlstring;
} else {
    if ($xmlout) {
        fwrite($xmloutfile,$xmlstring);
    }
    page_head("Server status page");
    if ($version) {
        echo "Server software version: $version<p>\n";
    }
    echo time_str(time()), "
        <table width=100%>
        <tr>
        <td width=40% valign=top>
        <h2>Server status</h2>
        <table border=0 cellpadding=4>
        <tr><th>Program</th><th>Host</th><th>Status</th></tr>
    ";
}
;
// Are the data-driven web sites running? Check for existence of stop_web.
// If it is there, set $web_running to -1 for "disabled",
// otherwise it will be already set to 1 for "enabled."
// Set $www_host to the name of server hosting WWW site.
//
$web_running = !file_exists("../../stop_web");
if ($web_running == 0) $web_running = -1;
show_status($www_host, "data-driven web pages", $web_running);

// Check for httpd.pid file of upload/download server.
//
$uldl_running = file_exists($uldl_pid);
if ($uldl_running == 0) $uldl_running = -1;
show_status($uldl_host, "upload/download server", $uldl_running);

$sched_running = !file_exists("../../stop_sched");
show_status($sched_host, "scheduler", $sched_running);

// parse through config.xml to get all daemons running
//
$cursor = 0;
while ($thisxml = trim(parse_next_element($config_xml,"<daemon>",$cursor))) {
    $host = parse_element($thisxml,"<host>");
    if ($host == "") {
        $host = $project_host;
    }
    $cmd = parse_element($thisxml,"<cmd>");
    list($ncmd) = explode(" ",$cmd);
    $log = parse_element($thisxml,"<output>");
    if (!$log) {
        $log = $ncmd . ".log";
    }
    list($nlog) = explode(".log",$log);
    $pid = parse_element($thisxml,"<pid_file>");
    if (!$pid) {
        $pid = $ncmd . ".pid";
    }
    $disabled = parse_element($thisxml,"<disabled>");
    show_daemon_status($host, $nlog, $ncmd, $disabled);
}

$xmlstring = "  </daemon_status>\n  <database_file_states>\n";
if ($xml) {
    echo $xmlstring;
} else {
    if ($xmlout) {
        fwrite($xmloutfile,$xmlstring);
    }
    echo "
        <tr><td align=right><b>Running:</b></td>
        <td colspan=2>Program is operating normally</td></tr>
        <tr><td align=right><b>Not Running:</b></td>
        <td colspan=2>Program failed or ran out of work<br>
           (or the project is down)</td></tr>
        <tr><td align=right><b>Disabled:</b></td>
        <td colspan=2>Program has been disabled by staff<br>
           (for debugging/maintenance)</td></tr>
        </table>
        </td>
        <td width=40% valign=top>
        <h2>Database/file status</h2>
    ";
}

$retval = db_init_aux();
if ($retval) {
    echo "The database server is not accessible";
} else {
    if (!$xml) {
        echo "
            <table border=0 cellpadding=4>
            <tr><th>State</th><th>#</th></tr>
        ";
    }

    // If you are reading these values from a file rather than
    // making live queries to the database, do something like this:
    //
    // $sendfile = "/home/boincadm/server_status_data/count_results_unsent.out";
    // $n = `/bin/tail -1 $sendfile`;
    // show_counts("Tasks ready to send","results_ready_to_send",$n);

    show_counts(
        "Tasks ready to send",
        "results_ready_to_send",
        get_mysql_count("result where server_state = 2")
    );
    show_counts(
        "Tasks in progress",
        "results_in_progress",
        get_mysql_count("result where server_state = 4")
    );
    show_counts(
        "Workunits waiting for validation",
        "workunits_waiting_for_validation",
        get_mysql_count("workunit where need_validate=1")
    );
    show_counts(
        "Workunits waiting for assimilation",
        "workunits_waiting_for_assimilation",
        get_mysql_count("workunit where assimilate_state=1")
    );
    show_counts(
        "Workunits waiting for file deletion",
        "workunits_waiting_for_deletion",
        get_mysql_count("workunit where file_delete_state=1")
    );
    show_counts(
        "Tasks waiting for file deletion",
        "results_waiting_for_deletion",
        get_mysql_count("result where file_delete_state=1")
    );

    $result = mysql_query("select MIN(transition_time) as min from workunit");
    $min = mysql_fetch_object($result);
    mysql_free_result($result);
    $gap = (time() - $min->min)/3600;
    if (($gap < 0) || ($min->min == 0)) {
        $gap = 0;
    }
    show_counts(
        "Transitioner backlog (hours)",
        "transitioner_backlog_hours",
        $gap
    );
    if (!$xml) {
        echo "</table>";
    }
}

$xmlstring = "  </database_file_states>\n</server_status>\n";
if ($xml) {
    echo $xmlstring;
} else {
    if ($xmlout) {
        fwrite($xmloutfile, $xmlstring);
    }
    echo "
        </td>
        <td>&nbsp;</td>
        </tr>
        </table>
    ";
    page_tail();
}

end_cache($cache_period, $cache_args);
?>
