<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// Get server status.
//
// default: show as web page
// ?xml=1:  show as XML
// ?counts=1: show only overall job counts, w/o caching
//      (for remote job submission systems)
// Sources of data:
// - daemons on this host: use "ps" to see if each is running
//   (this could be made more efficient using a single "ps",
//   or it could be cached)
// - daemons on other hosts: get from a cached file generated periodically
//   by ops/remote_server_status.php
//   (we can't do this ourselves because apache can't generally ssh)
// - apps and job counts: get from a cached file that we generate ourselves

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/server_version.inc");
require_once("../inc/common_defs.inc");

if (!defined('STATUS_PAGE_TTL')) {
    define('STATUS_PAGE_TTL', 3600);
}

// trim a daemon command for display.
// For now, remove the cmdline args, but show the app if any
//
function command_display($cmd) {
    $x = explode(" -", $cmd);
    $prog = $x[0];
    $x = strpos($cmd, "-app ");
    if ($x) {
        $y = substr($cmd, $x);
        $y = explode(" ", $y);
        $app = $y[1];
        $prog .= " ($app)";
    }
    return $prog;
}

function daemon_html($d) {
    switch ($d->status) {
    case 0:
        $s = tra("Not Running");
        $c = "bg-danger";
        break;
    case 1:
        $s = tra("Running");
        $c = "bg-success";
        break;
    default:
        $s = tra("Disabled");
        $c = "bg-warning";
        break;
    }
    echo "<tr>
        <td>".command_display($d->cmd)."</td>
        <td>$d->host</td>
        <td class=\"$c\"><nobr>$s</nobr></td>
        </tr>
    ";
}

function daemon_xml($d) {
    switch ($d->status) {
    case 0: $s = "not running"; break;
    case 1: $s = "running"; break;
    default: $s = "disabled";
    }
    echo "  <daemon>
        <host>$d->host</host>
        <command>".command_display($d->cmd)."</command>
        <status>$s</status>
    </daemon>
";
}

function item_xml($name, $val) {
    if (!$val) $val = 0;
    echo "   <$name>$val</$name>\n";
}

function item_html($name, $val) {
    $name = tra($name);
    echo "<tr><td>$name</td><td>$val</td></tr>\n";
    //echo "<tr><td align=right>$name</td><td align=right>$val</td></tr>\n";
}

function show_status_html($x) {
    global $server_version, $server_version_str;

    page_head(tra("Project status"));
    $j = $x->jobs;
    $daemons = $x->daemons;
    start_table();
    echo "<tr><td>\n";
    echo "
         <h3>".tra("Server status")."</h3>
    ";
    start_table('table-striped');
    table_header(tra("Program"), tra("Host"), tra("Status"));
    foreach ($daemons->local_daemons as $d) {
        daemon_html($d);
    }
    foreach ($daemons->remote_daemons as $d) {
        daemon_html($d);
    }
    foreach ($daemons->disabled_daemons as $d) {
        daemon_html($d);
    }
    end_table();

    if ($daemons->cached_time) {
        echo "<br>Remote daemon status as of ", time_str($daemons->cached_time);
    }
    if ($daemons->missing_remote_status) {
        echo "<br>Status of remote daemons is missing\n";
    }
    if (function_exists('server_status_project_info')) {
        echo "<br>";
        server_status_project_info();
    }
    echo "</td><td>\n";
    echo "<h3>".tra("Computing status")."</h3>\n";
    echo "<h4>".tra("Work")."</h4>\n";
    start_table('table-striped');
    item_html("Tasks ready to send", $j->results_ready_to_send);
    item_html("Tasks in progress", $j->results_in_progress);
    item_html("Workunits waiting for validation", $j->wus_need_validate);
    item_html("Workunits waiting for assimilation", $j->wus_need_assimilate);
    item_html("Workunits waiting for file deletion", $j->wus_need_file_delete);
    item_html("Tasks waiting for file deletion", $j->results_need_file_delete);
    item_html("Transitioner backlog (hours)", number_format($j->transitioner_backlog, 2));
    end_table();
    echo "<h4>".tra("Users")."</h4>\n";
    start_table('table-striped');
    item_html("With credit", $j->users_with_credit);
    item_html("With recent credit", $j->users_with_recent_credit);
    item_html("Registered in past 24 hours", $j->users_past_24_hours);
    end_table();
    echo "<h4>".tra("Computers")."</h4>\n";
    start_table('table-striped');
    item_html("With credit", $j->hosts_with_credit);
    item_html("With recent credit", $j->hosts_with_recent_credit);
    item_html("Registered in past 24 hours", $j->hosts_past_24_hours);
    item_html("Current GigaFLOPS", round($j->flops, 2));
    end_table();
    echo "</td></tr>\n";
    end_table();
    echo "<h3>".tra("Tasks by application")."</h3>\n";
    start_table('table-striped');
    table_header(
        tra("Application"),
        tra("Unsent"),
        tra("In progress"),
        tra("Runtime of recent tasks in hours: average, min, max"),
        tra("Users in last 24 hours")
    );
    foreach ($j->apps as $app) {
        if ($app->info) {
            $avg = round($app->info->avg, 2);
            $min = round($app->info->min, 2);
            $max = round($app->info->max, 2);
            $x = $max?"$avg ($min - $max)":"---";
            $u = $app->info->users;
        } else {
            $x = '---';
            $u = '---';
        }
        echo "<tr>
            <td>$app->user_friendly_name</td>
            <td>$app->unsent</td>
            <td>$app->in_progress</td>
            <td>$x</td>
            <td>$u</td>
            </tr>
        ";
    }
    end_table();

    // show server software version.
    // If it's a release (minor# is even) link to github branch
    //
    echo "Server software version: $server_version_str";
    if ($server_version[1]%2 == 0) {
        $url = sprintf("%s/%d/%d.%d",
            "https://github.com/BOINC/boinc/tree/server_release",
            $server_version[0],
            $server_version[0],
            $server_version[1]
        );
        echo " <a href=\"$url\">View source on Github</a>.";
    }
    echo "<br>\n";

    if ($j->db_revision) {
        echo tra("Database schema version: "), $j->db_revision;
    }
    echo "<p>Task data as of ".time_str($j->cached_time);
    page_tail();
}

function show_status_xml($x) {
    xml_header();
    echo "<server_status>\n<daemon_status>\n";

    $daemons = $x->daemons;
    foreach ($daemons->local_daemons as $d) {
        daemon_xml($d);
    }
    foreach ($daemons->remote_daemons as $d) {
        daemon_xml($d);
    }
    foreach ($daemons->disabled_daemons as $d) {
        daemon_xml($d);
    }
    echo "</daemon_status>\n<database_file_states>\n";
    $j = $x->jobs;
    item_xml("results_ready_to_send", $j->results_ready_to_send);
    item_xml("results_in_progress", $j->results_in_progress);
    item_xml("workunits_waiting_for_validation", $j->wus_need_validate);
    item_xml("workunits_waiting_for_assimilation", $j->wus_need_assimilate);
    item_xml("workunits_waiting_for_deletion", $j->wus_need_file_delete);
    item_xml("results_waiting_for_deletion", $j->results_need_file_delete);
    item_xml("transitioner_backlog_hours", $j->transitioner_backlog);
    item_xml("users_with_recent_credit", $j->users_with_recent_credit);
    item_xml("users_with_credit", $j->users_with_credit);
    item_xml("users_registered_in_past_24_hours", $j->users_past_24_hours);
    item_xml("hosts_with_recent_credit", $j->hosts_with_recent_credit);
    item_xml("hosts_with_credit", $j->hosts_with_credit);
    item_xml("hosts_registered_in_past_24_hours", $j->hosts_past_24_hours);
    item_xml("current_floating_point_speed", $j->flops);
    echo "<tasks_by_app>\n";
    foreach ($j->apps as $app) {
        echo "<app>\n";
        item_xml("id", $app->id);
        item_xml("name", $app->name);
        item_xml("unsent", $app->unsent);
        item_xml("in_progress", $app->in_progress);
        if ($app->info) {
            item_xml("avg_runtime", $app->info->avg);
            item_xml("min_runtime", $app->info->min);
            item_xml("max_runtime", $app->info->max);
            item_xml("users", $app->info->users);
        }
        echo "</app>\n";
    }
    echo "</tasks_by_app>
</database_file_states>
</server_status>
";
}

function local_daemon_running($cmd, $pidname, $host) {
    if (!$pidname) {
        $cmd = trim($cmd);
        $x = explode(" ", $cmd);
        $prog = $x[0];
        $pidname = $prog . '.pid';
    }
    $path = "../../pid_$host/$pidname";
    if (is_file($path)) {
        $pid = file_get_contents($path);
        if ($pid) {
            $pid = trim($pid);
            $out = Array();
            exec("ps -ww $pid", $out);
            foreach ($out as $y) {
                if (strstr($y, (string)$pid)) return 1;
            }
        }
    }
    return 0;
}

// returns a data structure of the form
// local_daemons: array of
//   cmd, status
// remote_daemons: array of
//   cmd, host, status
// disabled_daemons: array of
//   cmd, host
//
function get_daemon_status() {
    $c = simplexml_load_file("../../config.xml");
    if (!$c) {
        die("can't parse config file\n");
    }
    $daemons = $c->daemons;
    $config = $c->config;
    $main_host = trim((string)$config->host);
    $master_url = trim((string)$config->master_url);
    $u = parse_url($master_url);
    if (!array_key_exists("host", $u)) {
        print_r($u);
        die("can't parse URL $master_url");
    }
    $master_host = $u["host"];
    if ($config->www_host) {
        $web_host = trim((string) $config->www_host);
    } else {
        $web_host = $main_host;
    }
    if ($config->sched_host) {
        $sched_host = trim((string) $config->sched_host);
    } else {
        $sched_host = $main_host;
    }
    $have_remote = false;
    $local_daemons = array();
    $disabled_daemons = array();

    // the upload and download servers are sort of daemons too
    //
    $url = trim((string) $config->download_url);
    $u = parse_url($url);
    $h = $u["host"];
    if ($h == $master_host) {
        $y = new StdClass;
        $y->cmd = "Download server";
        $y->host = $h;
        $y->status = 1;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }
    $url = trim((string) $config->upload_url);
    $u = parse_url($url);
    $h = $u["host"];
    if ($h == $master_host) {
        $y = new StdClass;
        $y->cmd = "Upload server";
        $y->host = $h;
        $y->status = !file_exists("../../stop_upload");;
        $local_daemons[] = $y;
    } else {
        $have_remote = true;
    }

    // the scheduler is a CGI program, not a daemon;
    // it doesn't have a PID.
    $y = new StdClass;
    $y->cmd = "Scheduler";
    $y->host = $sched_host;
    $y->status = !file_exists("../../stop_sched");;
    $local_daemons[] = $y;

    foreach ($daemons->daemon as $d) {
        if ((int)$d->disabled != 0) {
            $x = new StdClass;
            $x->cmd = (string)$d->cmd;
            $x->host = (string)$d->host;
            if (!$x->host) $x->host = $main_host;
            $x->status = -1;
            $disabled_daemons[] = $x;
            continue;
        }
        $host = $d->host?(string)$d->host:$main_host;
        if ($host != $web_host) {
            $have_remote = true;
            continue;
        }
        $x = new StdClass;
        $x->cmd = (string)$d->cmd;
        $x->status = local_daemon_running($x->cmd, trim($d->pid_file), $web_host);
        $x->host = $web_host;
        $local_daemons[] = $x;

    }

    $x = new StdClass;
    $x->local_daemons = $local_daemons;
    $x->disabled_daemons = $disabled_daemons;
    $x->missing_remote_status = false;
    $x->cached_time = 0;
    $x->remote_daemons = array();
    if ($have_remote) {
        $f = @file_get_contents("../cache/remote_server_status");
        if ($f) {
            $x->remote_daemons = unserialize($f);
            $x->cached_time = filemtime("../cache/remote_server_status");
        } else {
            $x->missing_remote_status = true;
        }
    }
    return $x;
}

// get info for BUDA apps
// enumerate BUDA batches; get list of app/variants names
// for each app/variant
//      join result/workunit/batch to get runtime stats, users
//      join result/workunit/batch to get unsent, in_progress counts
// return list of pseudo-apps
//
function get_buda_status() {
}

function get_job_status() {
    $s = unserialize(get_cached_data(STATUS_PAGE_TTL, "job_status"));
    if ($s) {
        return $s;
    }

    $s = new StdClass;
    $apps = BoincApp::enum("deprecated=0");
    foreach ($apps as $app) {
        $info = BoincDB::get()->lookup_fields("result", "stdClass",
            "ceil(avg(elapsed_time)/3600*100)/100 as avg,
                ceil(min(elapsed_time)/3600*100)/100 as min,
                ceil(max(elapsed_time)/3600*100)/100 as max,
                count(distinct userid) as users
            ",
            sprintf('appid=%d
                AND validate_state=%d
                AND received_time > (unix_timestamp()-86400)
                ',
                $app->id,
                VALIDATE_STATE_VALID
            )
        );
        // $info fields will be null if app has no results
        if ($info->avg) {
            $app->info = $info;
        } else {
            $app->info = null;
        }
        $app->unsent = BoincResult::count(
            sprintf('appid=%d and server_state=%d',
                $app->id, RESULT_SERVER_STATE_UNSENT
            )
        );
        $app->in_progress = BoincResult::count(
            sprintf('appid=%d and server_state=%d',
                $app->id, RESULT_SERVER_STATE_IN_PROGRESS
            )
        );
    }
    $s->apps = $apps;
    $s->results_ready_to_send = BoincResult::count(
        sprintf('server_state=%d', RESULT_SERVER_STATE_UNSENT)
    );
    $s->results_in_progress = BoincResult::count(
        sprintf('server_state=%d', RESULT_SERVER_STATE_IN_PROGRESS)
    );
    $s->results_need_file_delete = BoincResult::count(
        sprintf('file_delete_state=%d', FILE_DELETE_READY)
    );
    $s->wus_need_validate = BoincWorkunit::count("need_validate=1");
    $s->wus_need_assimilate = BoincWorkunit::count(
        sprintf('assimilate_state=%d', ASSIMILATE_READY)
    );
    $s->wus_need_file_delete = BoincWorkunit::count(
        sprintf('file_delete_state=%d', FILE_DELETE_READY)
    );
    $x = BoincDB::get()->lookup_fields("workunit", "stdClass", "MIN(transition_time) as min", "TRUE");
    $gap = (time() - $x->min)/3600;
    if (($gap < 0) || ($x->min == 0)) {
        $gap = 0;
    }
    $s->transitioner_backlog = $gap;
    $s->users_with_recent_credit = BoincUser::count("expavg_credit>1");
    $s->users_with_credit = BoincUser::count("total_credit>1");
    $s->users_past_24_hours = BoincUser::count("create_time > (unix_timestamp() - 86400)");
    $s->hosts_with_recent_credit = BoincHost::count("expavg_credit>1");
    $s->hosts_with_credit = BoincHost::count("total_credit>1");
    $s->hosts_past_24_hours = BoincHost::count("create_time > (unix_timestamp() - 86400)");
    $s->flops = BoincUser::sum("expavg_credit")/200;

    $s->db_revision = null;
    if (file_exists("../../db_revision")) {
        $s->db_revision = trim(file_get_contents("../../db_revision"));
    }

    $s->cached_time = time();
    $e = set_cached_data(STATUS_PAGE_TTL, serialize($s), "job_status");
    if ($e) echo "set_cached_data(): $e\n";
    return $s;
}

function show_counts_xml() {
    xml_header();
    echo "<job_counts>\n";
    item_xml('results_ready_to_send', BoincResult::count(
        sprintf('server_state=%d', RESULT_SERVER_STATE_UNSENT)
    ));
    item_xml('results_in_progress', BoincResult::count(
        sprintf('server_state=%d', RESULT_SERVER_STATE_IN_PROGRESS)
    ));
    item_xml('results_need_file_delete', BoincResult::count(
        sprintf('file_delete_state=%d', FILE_DELETE_READY)
    ));
    item_xml('wus_need_validate', BoincWorkunit::count("need_validate=1"));
    item_xml('wus_need_assimilate', BoincWorkunit::count(
        sprintf('assimilate_state=%d', ASSIMILATE_READY)
    ));
    item_xml('wus_need_file_delete', BoincWorkunit::count(
        sprintf('file_delete_state=%d', FILE_DELETE_READY)
    ));
    echo "</job_counts>\n";
}

function main() {
    if (get_int('counts', true)) {
        show_counts_xml();
    } else {
        $x = new StdClass;
        $x->daemons = get_daemon_status();
        $x->jobs = get_job_status();
        if (get_int('xml', true)) {
            show_status_xml($x);
        } else {
            show_status_html($x);
        }
    }
}

main();
?>
