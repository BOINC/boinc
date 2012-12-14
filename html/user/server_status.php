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

// server_status.php
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

require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/cache.inc");
require_once("../inc/translation.inc");

check_get_args(array("xml"));

$xml = get_int("xml", true);

// daemon status outputs: 1 (running) 0 (not running) or -1 (disabled)
//
function daemon_status($host, $pidname, $progname, $disabled) {
    global $ssh_exe, $ps_exe, $project_host, $project_dir;
    if ($disabled == 1) return -1;
    $path = "$project_dir/pid_$host/$pidname";
    if ($host != $project_host) {
        $command = "$ssh_exe $host $project_dir/bin/pshelper $path";
        $foo = exec($command);
        $running = 1;
        if ($foo) {
            if (strstr($foo, "false")) $running = 0;
        } else $running = 0;
        return $running;
    }
    $running = 0;
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            $pid = trim($pid);
            $command = "$ps_exe ww $pid";
            $foo = exec($command);
            if ($foo) {
                if (strstr($foo, (string)$pid)) $running = 1;
            }
        }
    }
    return $running;
}

function show_status($host, $progname, $running) {
    global $xml;
    $xmlstring = "    <daemon>\n      <host>$host</host>\n      <command>$progname</command>\n";
    $htmlstring = "<tr><td>$progname</td><td>$host</td>";
    if ($running == 1) {
        $xmlstring .= "      <status>running</status>\n";
        $htmlstring .= "<td class=\"running\">".tra("Running")."</td>\n";
    } elseif ($running == 0) {
        $xmlstring .= "      <status>not running</status>\n";
        $htmlstring .= "<td class=\"notrunning\">".tra("Not Running")."</td>\n";
    } else {
        $xmlstring .= "      <status>disabled</status>\n";
        $htmlstring .= "<td class=\"disabled\">".tra("Disabled")."</td>\n";
    }
    $xmlstring .= "    </daemon>\n";
    $htmlstring .= "</tr>\n";
    if ($xml) {
        echo $xmlstring;
    } else {
        echo $htmlstring;
    }
    return 0;
}

function show_daemon_status($host, $pidname, $progname, $disabled) {
    $running = daemon_status($host, $pidname, $progname, $disabled);
    show_status($host, $progname, $running);
}

function show_counts($key, $xmlkey, $value) {
    global $xml;
    $formattedvalue = number_format($value);
    $xmlstring = "    <$xmlkey>$value</$xmlkey>\n";
    if ($xml) {
        echo $xmlstring;
    } else {
        echo "<tr><td>$key</td><td>$formattedvalue</td></tr>";
    }
    return 0;
}

function get_mysql_count($query) {
    $count = unserialize(get_cached_data(3600, "get_mysql_count".$query));
    if ($count == false) {
        $result = mysql_query("select count(*) as count from " . $query);
        $count = mysql_fetch_object($result);
        mysql_free_result($result);
        $count = $count->count;
        set_cached_data(3600, serialize($count), "get_mysql_count".$query);
    }
    return $count;
}

function get_mysql_value($query) {
    $value = unserialize(get_cached_data(3600, "get_mysql_value".$query));
    if ($value == false) {
        $result = mysql_query($query);
        $row = mysql_fetch_object($result);
        mysql_free_result($result);
        $value = $row->value;
        set_cached_data(3600, serialize($value), "get_mysql_value".$query);
    }
    return $value;
}

function get_mysql_assoc($query) {
    $assoc = unserialize(get_cached_data(3600, "get_mysql_assoc".$query));
    if ($assoc == false) {
        $sql = "SELECT * FROM app WHERE deprecated != 1";
        $result = mysql_query($sql);
        while($row = mysql_fetch_assoc($result)) {
            $assoc[] = $row;
        }
        mysql_free_result($result);
        set_cached_data(3600, serialize($assoc), "get_mysql_assoc".$query);
    }
    return $assoc;
}

function get_mysql_user($clause) {
    $count = unserialize(get_cached_data(3600, "get_mysql_user".$clause));
    if ($count == false) {
        $result = mysql_query("select count(userid) as userid from (SELECT distinct userid FROM result where validate_state=1 and received_time > (unix_timestamp()-(3600*24*1)) " . $clause . ") t");
        $count = mysql_fetch_object($result);
        mysql_free_result($result);
        $count = $count->userid;
        set_cached_data(3600, serialize($count), "get_mysql_user".$clause);
    }
    return $count;
}

function get_runtime_info($appid) {
    $info = unserialize(get_cached_data(3600, "get_runtime_info".$appid));
    if ($info == false) {
        $result = mysql_query("
        Select ceil(avg(elapsed_time)/3600*100)/100 as avg,
                   ceil(min(elapsed_time)/3600*100)/100 as min,
                   ceil(max(elapsed_time)/3600*100)/100 as max
        from (SELECT elapsed_time FROM `result` WHERE appid = $appid and validate_state =1 and received_time > (unix_timestamp()-(3600*24)) ORDER BY `received_time` DESC limit 100) t");
        $info = mysql_fetch_object($result);
        mysql_free_result($result);
        set_cached_data(3600, serialize($info), "get_runtime_info".$appid);
    }
    return $info;
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
$project_dir = parse_element($config_vars,"<project_dir>");
if ($project_dir == "") {
    $project_dir = "../..";
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
    page_head(tra("Project status"));
    if ($version) {
        echo tra("Server software version: %1", $version) . " / ";
    }
    echo time_str(time()), "
        <table width=100%>
        <tr>
        <td width=40% valign=top>
        <h2>".tra("Server status")."</h2>
        <table border=0 cellpadding=4>
        <tr><th>".tra("Program")."</th><th>".tra("Host")."</th><th>".tra("Status")."</th></tr>
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
show_status($www_host, tra("data-driven web pages"), $web_running);

// Check for httpd.pid file of upload/download server.
//
$uldl_running = file_exists($uldl_pid);
if ($uldl_running == 0) $uldl_running = -1;
show_status($uldl_host, tra("upload/download server"), $uldl_running);

$sched_running = !file_exists("../../stop_sched");
show_status($sched_host, tra("scheduler"), $sched_running);

// parse through config.xml to get all daemons running
//
$cursor = 0;
while ($thisxml = trim(parse_next_element($config_xml,"<daemon>",$cursor))) {
    $host = parse_element($thisxml,"<host>");
    if ($host == "") {
        $host = $project_host;
    }
    $cmd = parse_element($thisxml,"<cmd>");
    list($cmd) = explode(" ", $cmd);
    $log = parse_element($thisxml,"<output>");
    if (!$log) {
        $log = $cmd . ".log";
    }
    list($log) = explode(".log", $log);
    $pid = parse_element($thisxml,"<pid_file>");
    if (!$pid) {
        $pid = $cmd . ".pid";
    }
    $disabled = parse_element($thisxml,"<disabled>");

    // surrogate for command
    list($c) = explode(".", $log);
    show_daemon_status($host, $pid, $c, $disabled);
}

$xmlstring = "  </daemon_status>\n  <database_file_states>\n";
if ($xml) {
    echo $xmlstring;
} else {
    echo "
        <tr><td align=right><b>".tra("Running:")."</b></td>
        <td colspan=2>".tra("Program is operating normally")."</td></tr>
        <tr><td align=right><b>".tra("Not Running:")."</b></td>
        <td colspan=2>".tra("Program failed or the project is down")."</td></tr>
        <tr><td align=right><b>".tra("Disabled:")."</b></td>
        <td colspan=2>".tra("Program is disabled")."</td></tr>
        </table>
        </td>
        <td valign=top>
        <h2>".tra("Computing status")."</h2>
    ";
}

$retval = db_init_aux();
if ($retval) {
    echo tra("The database server is not accessible");
} else {
    if (!$xml) {
        echo "<table border=0 cellpadding=0 cellspacing=0><tr><td>
            <table border=0 cellpadding=4>
            <tr><th>".tra("Work")."</th><th>#</th></tr>
        ";
    }

    // If you are reading these values from a file rather than
    // making live queries to the database, do something like this:
    //
    // $sendfile = "/home/boincadm/server_status_data/count_results_unsent.out";
    // $n = `/bin/tail -1 $sendfile`;
    // show_counts("Tasks ready to send","results_ready_to_send",$n);

    show_counts(
        tra("Tasks ready to send"),
        "results_ready_to_send",
        get_mysql_count("result where server_state = 2")
    );
    show_counts(
        tra("Tasks in progress"),
        "results_in_progress",
        get_mysql_count("result where server_state = 4")
    );
    show_counts(
        tra("Workunits waiting for validation"),
        "workunits_waiting_for_validation",
        get_mysql_count("workunit where need_validate=1")
    );
    show_counts(
        tra("Workunits waiting for assimilation"),
        "workunits_waiting_for_assimilation",
        get_mysql_count("workunit where assimilate_state=1")
    );
    show_counts(
        tra("Workunits waiting for file deletion"),
        "workunits_waiting_for_deletion",
        get_mysql_count("workunit where file_delete_state=1")
    );
    show_counts(
        tra("Tasks waiting for file deletion"),
        "results_waiting_for_deletion",
        get_mysql_count("result where file_delete_state=1")
    );

    $gap = unserialize(get_cached_data(3600, "transitioner_backlog"));
    if ($gap === false) {
        $result = mysql_query("select MIN(transition_time) as min from workunit");
        $min = mysql_fetch_object($result);
        mysql_free_result($result);
        $gap = (time() - $min->min)/3600;
        if (($gap < 0) || ($min->min == 0)) {
            $gap = 0;
        }
        set_cached_data(3600, serialize($gap), "transitioner_backlog");
    }
    show_counts(
        tra("Transitioner backlog (hours)"),
        "transitioner_backlog_hours",
        $gap
    );
    if (!$xml) {
        echo "</table></td><td>";
    echo "<table>";
    echo "<tr><th>".tra("Users")."</th><th>#</th></tr>";
    show_counts(
        tra("with recent credit"),
        "users_with_recent_credit",
        get_mysql_count("user where expavg_credit>1")
    );
    show_counts(
        tra("with credit"),
        "users_with_credit",
        get_mysql_count("user where total_credit>0")
    );
    show_counts(
        tra("registered in past 24 hours"),
        "users_registered_in_past_24_hours",
        get_mysql_count("user where create_time > (unix_timestamp() - (24*3600))")
    );
    echo "<tr><th>".tra("Computers")."</th><th>#</th></tr>";
    show_counts(
        tra("with recent credit"),
        "hosts_with_recent_credit",
        get_mysql_count("host where expavg_credit>1")
    );
    show_counts(
        tra("with credit"),
        "hosts_with_credit",
        get_mysql_count("host where total_credit>0")
    );
    show_counts(
        tra("registered in past 24 hours"),
        "hosts_registered_in_past_24_hours",
        get_mysql_count("host where create_time > (unix_timestamp() - (24*3600))")
    );
    // 200 cobblestones = 1 GigaFLOPS
    show_counts(
        tra("current GigaFLOPs"),
        "current_floating_point_speed",
        get_mysql_value("SELECT sum(expavg_credit)/200 as value FROM user")
    );

    end_table();
    echo "</td></tr></table>";

    start_table();
   echo "<tr><th colspan=5>".tra("Tasks by application")."</th></tr>";
    row_heading_array(
        array(
            tra("application"),
            tra("unsent"),
            tra("in progress"),
            tra("avg runtime of last 100 results in h (min-max)"),
            tra("users in last 24h")
        )
    );
    $apps = get_mysql_assoc("SELECT * FROM app WHERE deprecated != 1");
    foreach($apps as $app) {
        $appid = $app["id"];
        $uf_name = $app["user_friendly_name"];
        echo "<tr><td>$uf_name</td>
            <td>" . number_format(get_mysql_count("result where server_state = 2 and appid = $appid")) . "</td>
            <td>" . number_format(get_mysql_count("result where server_state = 4 and appid = $appid")) . "</td>
            <td>"
        ;
        $info = get_runtime_info($appid);
        echo number_format($info->avg,2) . " (" . number_format($info->min,2) . " - " . number_format($info->max,2) . ")";
        echo "</td>
            <td>" . number_format(get_mysql_user("and appid = $appid")) . "</td>
            </tr>"
        ;
    }
    end_table();

    }
}

$xmlstring = "  </database_file_states>\n</server_status>\n";
if ($xml) {
    echo $xmlstring;
} else {
    echo "
        </td>
        </tr>
        </table>
    ";
    page_tail();
}

?>
