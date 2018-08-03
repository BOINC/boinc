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
$total_eac = 0;

function show_type($s, $x) {
    global $total_eac;
    $p = number_format(100*$x->c/$total_eac, 4);
    row_array(array($s, $x->n, "$p %"));
}

function cmp_host($x, $y) {
    if ($x->c < $y->c) return 1;
    if ($x->c > $y->c) return -1;
    return 0;
}

// sort an array whose values are structs with n, c
//
function sort_hosts($x) {
    uasort($x, 'cmp_host');
    return $x;
}

function get_stats($clause) {
    global $db, $min_credit;
    $x = $db->enum_general(
        'StdClass',
        "select count(*) as n, sum(expavg_credit) as c from host where expavg_credit>$min_credit and $clause"
    );
    return $x[0];
}

function hosts_win($os_names) {
    $total = new StdClass;
    $total->n = 0;
    $total->c = 0;
    $h = array();
    foreach ($os_names as $n) {
        if (strstr($n, "Windows")) {
            $x = get_stats("os_name='$n'");
            $h[$n] = $x;
            $total->n += $x->n;
            $total->c += $x->c;
        }
    }
    $h = sort_hosts($h);
    $h['Windows total'] = $total;
    return $h;
}

function hosts_darwin() {
    global $db, $min_credit;
    $x = $db->enum_general(
        'StdClass',
        "select distinct(os_version) as v from host where os_name='Darwin' and expavg_credit>$min_credit"
    );
    $vers = array();
    foreach ($x as $y) {
        $vers[] = $y->v;
    }
    $total = new StdClass;
    $total->n = 0;
    $total->c = 0;
    $h = array();
    foreach ($vers as $v) {
        $x = get_stats(
            "os_name='Darwin' and os_version='$v'"
        );
        $h[$v] = $x;
        $total->n += $x->n;
        $total->c += $x->c;
    }
    $h = sort_hosts($h);
    $h['total'] = $total;
    return $h;
}

function hosts_linux() {
    return get_stats("os_name like '%Linux%'");
}

function hosts_other($os_names) {
    $d = array();
    foreach ($os_names as $n) {
        if (strstr($n, 'Windows')) continue;
        if (strstr($n, 'Darwin')) continue;
        if (strstr($n, 'Linux')) continue;
        if (!$n) continue;
        $d[$n] = get_stats("os_name='$n'");
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
    $data->total_eac = $db->sum('host', 'expavg_credit', '');
    $data->windows = hosts_win($os_names);
    $data->darwin = hosts_darwin();
    $data->linux = hosts_linux();
    $data->other = hosts_other($os_names);
    return $data;
}

function hosts_by_os() {
    global $db, $min_credit, $total_eac;
    $data = get_cached_data(86400, "os");
    if ($data) {
        $data = unserialize($data);
    } else {
        $data = get_os_data();
        set_cached_data(86400, serialize($data), "os");
    }
    $total_eac = $data->total_eac;
    page_head("Host breakdown by operating system");
    echo "<p><a href=host_stats.php?boinc_version=1>View breakdown by BOINC version</a><p>\n";
    start_table("table-striped");
    row_heading_array(array("OS", "# of active hosts", "% of total RAC"));
    //echo "total: $total_eac\n";
    foreach ($data->windows as $n=>$x) {
        show_type($n, $x);
    }
    foreach ($data->darwin as $v=>$x) {
        show_type("Darwin $v", $x);
    }
    show_type("Linux total", $data->linux);
    foreach ($data->other as $n=>$x) {
        show_type($n, $x);
    }
    end_table();
    page_tail();
}

function get_boinc_version_data() {
    global $db, $min_credit;
    $db = BoincDb::get();
    $vers = $db->enum_general(
        'StdClass',
        "select distinct substring(serialnum, locate('BOINC', serialnum), 14) as x from host where expavg_credit>$min_credit"
    );
    $boinc_versions = array();
    foreach ($vers as $v) {
        $v = substr($v->x, 6);
        $v = strstr($v, "]", true);
        if ($v) $boinc_versions[] = $v;
    }
    $vers = array();
    foreach ($boinc_versions as $v) {
        $vers[$v] = get_stats("serialnum like '%$v%'");
    }
    $vers = sort_hosts($vers);
    $data = new StdClass;
    $data->total_eac = $db->sum('host', 'expavg_credit', '');
    $data->vers = $vers;
    return $data;
}

function hosts_by_boinc_version() {
    global $db, $min_credit, $total_eac;
    $data = get_cached_data(86400, "boinc_version");
    if ($data) {
        $data = unserialize($data);
    } else {
        $data = get_boinc_version_data();
        set_cached_data(86400, serialize($data), "boinc_version");
    }
    $total_eac = $data->total_eac;
    page_head("Host breakdown by BOINC version");
    echo "<p><a href=host_stats.php>View breakdown by operating system</a><p>\n";
    start_table("table-striped");
    row_heading_array(array("BOINC version", "# of active hosts", "% of total RAC"));
    foreach ($data->vers as $v=>$x) {
        show_type($v, $x);
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
