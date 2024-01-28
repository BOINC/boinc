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

// strip leading AMD, NVIDIA, etc.
// This avoids showing the same model twice
//
function strip_vendor($model) {
    foreach (array("AMD ", "NVIDIA ", "ATI ", "Intel(R) ") as $maker) {
        $n = strlen($maker);
        if (substr($model, 0, $n) == $maker) {
            return substr($model, $n);
        }
    }
    return $model;
}

// take a host.serialnum field (which may encode several GPUs)
// and extract the model name for the given vendor
//
function get_gpu_model($x, $vendor) {
    $descs = explode("]", $x);
    array_pop($descs);
    foreach ($descs as $desc) {
        $desc = trim($desc, "[");
        $d = explode("|", $desc);
        if ($d[0] == "BOINC") continue;
        if ($d[0] != $vendor) continue;
        return strip_vendor(trim($d[1]));
    }
    return null;
}

function add_model($model, $r, $wu, &$models) {
    if (array_key_exists($model, $models)) {
        $models[$model]->count++;
        $models[$model]->time += $r->elapsed_time/$wu->rsc_fpops_est;
    } else {
        $x = new StdClass;
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
    if (count($avs) == 0) {
        $x = new StdClass;
        $x->total = array();
        return $x;
    }

    $av_ids = "";
    foreach($avs as $av) {
        $av_ids .= "$av->id, ";
    }
    if ($vendor == "cuda") {
        $av_ids .= "-3";
    } else if ($vendor == "ati") {
        $av_ids .= "-4";
    } else if ($vendor == "intel_gpu") {
        $av_ids .= "-5";
    } else if ($vendor == "apple_gpu") {
        $av_ids .= "-6";
    } else {
        $av_ids .= "0";
    }

    $t = time() - 30*86400;
    //echo "start enum $vendor $av_ids\n";
    $results = BoincResult::enum(
        "app_version_id in ($av_ids) and create_time > $t and elapsed_time>100 limit 2000"
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
        $v = "";
        if ($vendor == "cuda") {
            $v = "CUDA";
        } else if ($vendor == "intel_gpu") {
            $v = "INTEL";
        } else if ($vendor == "apple_gpu") {
            $v = "Apple";
        } else {
            $v = "CAL";
        }
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
    $x = new StdClass;
    $x->total = $total;
    $x->win = $win;
    $x->linux = $linux;
    $x->mac = $mac;
    return $x;
}

function get_gpu_lists() {
    $x = new StdClass;
    $x->cuda = get_gpu_list("cuda", "nvidia");
    $x->ati = get_gpu_list("ati", "amd");
    $x->intel_gpu = get_gpu_list("intel_gpu");
    $x->apple_gpu = get_gpu_list("apple_gpu");
    $x->time = time();
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
$data = FALSE;
if ($d) {
    $data = unserialize($d);
}

if (!$data) {
    $data = get_gpu_lists();
    set_cached_data(86400, serialize($data));
}

page_head(tra("Top GPU models"));
echo tra("The following lists show the most productive GPU models on different platforms.  Relative speeds, measured by average elapsed time of tasks, are shown in parentheses.");
show_vendor("NVIDIA", $data->cuda);
show_vendor("ATI/AMD", $data->ati);
show_vendor("Intel", $data->intel_gpu);
show_vendor("Apple", $data->apple_gpu);
echo "<p>Generated ".time_str($data->time);
page_tail();

?>
