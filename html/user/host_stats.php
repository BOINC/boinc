<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

// show statistics of the host population

require_once("../inc/util.inc");
require_once("../inc/cache.inc");

$min_credit = .1;   // only count hosts with this much RAC
$total_rac = 0;

define("CACHE_PERIOD", 7*86400);

function show_type($type, $stats) {
    global $total_rac;
    $pct = $total_rac?number_format(100*$stats->rac/$total_rac, 4):0;
    row_array(array($type, $stats->nhosts, "$pct %"));
}

// a "stats" object consists of #hosts and RAC.  Compare the RAC.
//
function cmp_rac($stat1, $stat2) {
    if ($stat1->rac < $stat2->rac) return 1;
    if ($stat1->rac > $stat2->rac) return -1;
    return 0;
}

// sort an array whose values are stats objects
//
function sort_stats_by_rac($stats) {
    uasort($stats, 'cmp_rac');
    return $stats;
}

function get_stats($clause) {
    global $db, $min_credit;
    $results = $db->enum_general(
        'StdClass',
        "select count(*) as nhosts, sum(expavg_credit) as rac from host where expavg_credit>$min_credit and $clause"
    );
    return $results[0];
}

function hosts_win($os_names) {
    $total = new StdClass;
    $total->nhosts = 0;
    $total->rac = 0;
    $os_stats = array();
    foreach ($os_names as $os_name) {
        if (strstr($os_name, "Windows")) {
            $stats = get_stats("os_name='$os_name'");
            $os_stats[$os_name] = $stats;
            $total->nhosts += $stats->nhosts;
            $total->rac += $stats->rac;
        }
    }
    $os_stats = sort_stats_by_rac($os_stats);
    $os_stats['Windows total'] = $total;
    return $os_stats;
}

function hosts_darwin() {
    global $db, $min_credit;
    $results = $db->enum_general(
        'StdClass',
        "select distinct(os_version) as v from host where os_name='Darwin' and expavg_credit>$min_credit"
    );
    $vers = array();
    foreach ($results as $result) {
        $vers[] = $result->v;
    }
    $total = new StdClass;
    $total->nhosts = 0;
    $total->rac = 0;
    $os_stats = array();
    foreach ($vers as $ver) {
        $stats = get_stats(
            "os_name='Darwin' and os_version='$v'"
        );
        $os_stats[$ver] = $stats;
        $total->nhosts += $stats->nhosts;
        $total->rac += $stats->rac;
    }
    $os_stats = sort_stats_by_rac($os_stats);
    $os_stats['total'] = $total;
    return $os_stats;
}

function hosts_linux() {
    return get_stats("os_name like '%Linux%'");
}

function hosts_other($os_names) {
    $d = array();
    foreach ($os_names as $os_name) {
        if (strstr($os_name, 'Windows')) continue;
        if (strstr($os_name, 'Darwin')) continue;
        if (strstr($os_name, 'Linux')) continue;
        if (!$os_name) continue;
        $d[$os_name] = get_stats("os_name='$os_name'");
    }
    return $d;
}

function get_os_data() {
    global $db, $min_credit;
    $db = BoincDb::get();
    $names = $db->enum_general(
        'StdClass',
        "select distinct(os_name) from host where expavg_credit>$min_credit"
    );
    $os_names = array();
    foreach ($names as $n) {
        $os_names[] = $n->os_name;
    }
    $data = new StdClass;
    $data->total_rac = $db->sum('host', 'expavg_credit', '');
    $data->windows = hosts_win($os_names);
    $data->darwin = hosts_darwin();
    $data->linux = hosts_linux();
    $data->other = hosts_other($os_names);
    return $data;
}

function hosts_by_os() {
    global $db, $min_credit, $total_rac;
    $data = get_cached_data(CACHE_PERIOD, "os");
    if ($data) {
        $data = unserialize($data);
    } else {
        $data = get_os_data();
        set_cached_data(CACHE_PERIOD, serialize($data), "os");
    }
    $total_rac = $data->total_rac;
    page_head("Computer breakdown by operating system");
    echo "<p><a href=host_stats.php?boinc_version=1>View breakdown by BOINC version</a><p>\n";
    start_table("table-striped");
    row_heading_array(array("Operating system", "# of active computers", "% of recent credit"));
    //echo "total: $total_rac\n";
    foreach ($data->windows as $vers=>$stats) {
        show_type($vers, $stats);
    }
    foreach ($data->darwin as $vers=>$state) {
        show_type("Darwin $vers", $state);
    }
    show_type("Linux total", $data->linux);
    foreach ($data->other as $vers=>$stats) {
        show_type($vers, $stats);
    }
    end_table();
    page_tail();
}

function get_boinc_version_data() {
    global $db, $min_credit;
    $db = BoincDb::get();
    $results = $db->enum_general(
        'StdClass',
        "select distinct substring(serialnum, locate('BOINC', serialnum), 14) as boinc_vers from host where expavg_credit>$min_credit"
    );
    $boinc_versions = array();
    foreach ($results as $result) {
        $boinc_vers = substr($result->boinc_vers, 6);
        $boinc_vers = strstr($boinc_vers, "]", true);
        if ($boinc_vers) $boinc_versions[] = $boinc_vers;
    }
    $vers = array();
    foreach ($boinc_versions as $boinc_vers) {
        $vers[$boinc_vers] = get_stats("serialnum like '%$boinc_vers%'");
    }
    $vers = sort_stats_by_rac($vers);
    $data = new StdClass;
    $data->total_rac = $db->sum('host', 'expavg_credit', '');
    $data->vers = $vers;
    return $data;
}

function hosts_by_boinc_version() {
    global $db, $min_credit, $total_rac;
    $data = get_cached_data(CACHE_PERIOD, "boinc_version");
    if ($data) {
        $data = unserialize($data);
    } else {
        $data = get_boinc_version_data();
        set_cached_data(CACHE_PERIOD, serialize($data), "boinc_version");
    }
    $total_rac = $data->total_rac;
    page_head("Computer breakdown by BOINC version");
    echo "<p><a href=host_stats.php>View breakdown by operating system</a><p>\n";
    start_table("table-striped");
    row_heading_array(array("BOINC version", "# of active computers", "% of recent credit"));
    foreach ($data->vers as $vers=>$stats) {
        show_type($vers, $stats);
    }
    end_table();
    page_tail();
}

if (get_str("boinc_version", true)) {
    hosts_by_boinc_version();
} else {
    hosts_by_os();
}

?>
