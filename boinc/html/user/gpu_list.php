<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// generate a page of the best-performing GPU models.
//
// "best-performing" is defined as minimizing the average of
//
// elapsed_time(J)/rsc_fpops_est(J)
// over completed jobs J currently in the DB

require_once("../inc/util.inc");

function get_gpu_model($x, $vendor) {
    $descs = explode("]", $x);
    array_pop($descs);
    foreach ($descs as $desc) {
        $desc = trim($desc, "[");
        $d = explode("|", $desc);
        if ($d[0] == "BOINC") continue;
        if ($d[0] != $vendor) continue;
        return $d[1];
    }
    return null;
}

function add_model($model, $r, $wu, &$models) {
    if (array_key_exists($model, $models)) {
        $models[$model]->count++;
        $models[$model]->time += $r->elapsed_time;
    } else {
        $x = null;
        $x->count = 1;
        $x->time = $r->elapsed_time/$wu->rsc_fpops_est;
        $models[$model] = $x;
    }
}

// return a data structure containing GPU usage info for a vendor
// $x->total: combined list
// $x->windows
// $x->linux
// $x->mac
//
function get_gpu_list($vendor, $alt_vendor=null) {
    $clause = "plan_class like '%$vendor%'";
    if ($alt_vendor) {
        $clause .= " or plan_class like '%$alt_vendor%'";
    }
    $avs = BoincAppVersion::enum($clause);
    if (count($avs) == 0) return null;

    $av_ids = "";
    foreach($avs as $av) {
        $av_ids .= "$av->id, ";
    }
    $av_ids .= "0";

    $t = time() - 30*86400;
    //echo "start enum $vendor $av_ids\n";
    $results = BoincResult::enum(
        "app_version_id in ($av_ids) and create_time > $t and elapsed_time>100 limit 500"
    );
    //echo "end enum\n";
    $total = array();
    $win = array();
    $linux = array();
    $mac = array();
    foreach ($results as $r) {
        $h = BoincHost::lookup_id($r->hostid);
        if (!$h) continue;
        $wu = BoincWorkunit::lookup_id($r->workunitid);
        if (!$wu) continue;
        $v = $vendor=="cuda"?"CUDA":"CAL";
        $model = get_gpu_model($h->serialnum, $v);
        if (!$model) continue;
        add_model($model, $r, $wu, $total);
        if (strstr($h->os_name, "Windows")) {
            add_model($model, $r, $wu, $win);
        }
        if (strstr($h->os_name, "Linux")) {
            add_model($model, $r, $wu, $linux);
        }
        if (strstr($h->os_name, "Darwin")) {
            add_model($model, $r, $wu, $mac);
        }

    }
    $x = null;
    $x->total = $total;
    $x->win = $win;
    $x->linux = $linux;
    $x->mac = $mac;
    return $x;
}

function get_gpu_lists() {
    $x = null;
    $x->cuda = get_gpu_list("cuda", "nvidia");
    $x->ati = get_gpu_list("ati");
    return $x;
}

function gpucmp($x1, $x2) {
    return $x1->avg_time > $x2->avg_time;
}

function show_list($models, $name) {
    echo "<td><h2>$name</h2>\n";
    if (!count($models)) {
        echo tra("No GPU tasks reported")."</td>\n";
        return;
    }
    $max_count = 0;
    foreach ($models as $model=>$x) {
        if ($x->count > $max_count) $max_count = $x->count;
        $x->avg_time = $x->time/$x->count;
    }
    $min_time = 1e9;
    foreach ($models as $model=>$x) {
        if ($x->count < $max_count/10) continue;
        if ($x->avg_time < $min_time) $min_time = $x->avg_time;
    }
    uasort($models, 'gpucmp');
    echo "<ol>\n";
    foreach ($models as $model=>$x) {
        if ($x->count < $max_count/10) continue;
        $s = number_format($min_time/$x->avg_time, 3);
        echo "<li>($s)  $model\n";
    }
    echo "</ol></td>\n";
}

function show_vendor($vendor, $x) {
    echo "<h2>$vendor</h2>\n";
    if (!count($x->total)) {
        echo tra("No GPU tasks reported");
        return;
    }
    $have_win = count($x->win)>0;
    $have_mac = count($x->mac)>0;
    $have_linux = count($x->linux)>0;
    $n = 0;
    if ($have_win) $n++;
    if ($have_mac) $n++;
    if ($have_linux) $n++;
    $show_total = $n>1;
    start_table();
    echo "<tr>";
    if ($show_total) {
        show_list($x->total, "Total");
    }
    show_list($x->win, "Windows");
    show_list($x->linux, "Linux");
    show_list($x->mac, "Mac");
    echo "</tr></table>\n";
}

$d = get_cached_data(86400);
if ($d) {
    $data = unserialize($d);
} else {
    $data = get_gpu_lists();
    set_cached_data(86400, serialize($data));
}

page_head(tra("Top GPU models"));
echo tra("The following lists show the most productive GPU models on different platforms.  Relative speeds are shown in parentheses.");
show_vendor("NVIDIA", $data->cuda);
show_vendor("ATI/AMD", $data->ati);
page_tail();

?>
