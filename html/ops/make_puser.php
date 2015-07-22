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

// ?? what's this script for?

$cli_only = true;
require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

db_init();

function parse_num($xml, $tag) {
    $x = parse_element($xml, $tag);
    if (!$x) return 0;
    return $x;
}

function parse_boolint($xml, $tag) {
    $x = parse_bool($xml, $tag);
    if ($x) return 1;
    return 0;
}

function make_user($user) {
    $prefs = $user->global_prefs;

    $run_on_batteries = parse_boolint($prefs, "run_on_batteries");
    $run_if_user_active = parse_boolint($prefs, "run_if_user_active");
    $start_hour = parse_num($prefs, "<start_hour>");
    $end_hour = parse_num($prefs, "<end_hour>");
    $net_start_hour = parse_num($prefs, "<net_start_hour>");
    $net_end_hour = parse_num($prefs, "<net_end_hour>");
    $leave_apps_in_memory = parse_boolint($prefs, "leave_apps_in_memory");
    $confirm_before_connecting = parse_boolint($prefs, "confirm_before_connecting");
    $hangup_if_dialed = parse_boolint($prefs, "hangup_if_dialed");
    $work_buf_min_days = parse_num($prefs, "<work_buf_min_days>");
    $max_cpus = parse_num($prefs, "<max_cpus>");
    $cpu_scheduling_period_minutes = parse_num($prefs, "<cpu_scheduling_period_minutes>");
    $disk_interval = parse_num($prefs, "<disk_interval>");
    $disk_max_used_gb = parse_num($prefs, "<disk_max_used_gb>");
    $disk_max_used_pct = parse_num($prefs, "<disk_max_used_pct>");
    $disk_min_free_gb = parse_num($prefs, "<disk_min_free_gb>");
    $vm_max_used_pct = parse_num($prefs, "<vm_max_used_pct>");
    $idle_time_to_run = parse_num($prefs, "<idle_time_to_run>");
    $max_bytes_sec_up = parse_num($prefs, "<max_bytes_sec_up>");
    $max_bytes_sec_down = parse_num($prefs, "<max_bytes_sec_down>");
    $query = "insert into puser values
        ($user->id,
        $user->create_time,
        '$user->email_addr',
        '$user->country',
        $user->total_credit,
        '$user->venue',
        $run_on_batteries,
        $run_if_user_active,
        $start_hour,
        $end_hour,
        $net_start_hour,
        $net_end_hour,
        $leave_apps_in_memory,
        $confirm_before_connecting,
        $hangup_if_dialed,
        $work_buf_min_days,
        $max_cpus,
        $cpu_scheduling_period_minutes,
        $disk_interval,
        $disk_max_used_gb,
        $disk_max_used_pct,
        $disk_min_free_gb,
        $vm_max_used_pct,
        $idle_time_to_run,
        $max_bytes_sec_up,
        $max_bytes_sec_down)
    ";
    $retval = _mysql_query($query);
    if (!$retval) {
        echo _mysql_error();
    }
}

set_time_limit(0);

$result = _mysql_query("select * from user where total_credit > 0");
while ($user = _mysql_fetch_object($result)) {
    make_user($user);
}

?>
