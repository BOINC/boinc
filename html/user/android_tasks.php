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

// script to show task failure rate broken down by vendor or model

require_once("../inc/boinc_db.inc");
require_once("../inc/cache.inc");
require_once("../inc/util.inc");

function compare($x, $y) {
    $n1 = $x[1] + $x[3];
    $n2 = $y[1] + $y[3];
    if ($n1 == $n2) return 0;
    return ($n1 < $n2)?1:-1;
}

function get_models() {
    $db = BoincDb::get();
    $result = $db->do_query("select result.appid, result.outcome, host.product_name from result join host where host.os_name='Android' and result.hostid=host.id");

    $models = array();

    while ($r = $result->fetch_object()) {
        // standardize case to combine e.g. Samsung and samsung
        //
        $name_uc = strtoupper($r->product_name);
        if (array_key_exists($name_uc, $models)) {
            $m = $models[$name_uc];
            $m[$r->outcome]++;
            $models[$name_uc] = $m;
        } else {
            $m = array(0,0,0,0,0,0,0,0);
            $m[$r->outcome]++;
            $models[$name_uc] = $m;
        }
    }
    return $models;
}

function get_vendors($models) {
    $vendors = array();
    foreach ($models as $model=>$m) {
        $name = explode(" ", $model);
        $name = $name[0];
        if (array_key_exists($name, $vendors)) {
            $v = $vendors[$name];
            for ($i=0; $i<8; $i++) {
                $v[$i] += $m[$i];
            }
            $vendors[$name] = $v;
        } else {
            $vendors[$name] = $m;
        }
    }
    return $vendors;
}

function show_item($name, $c) {
    $s = $c[1];
    $f = $c[3];
    $n = $s + $f;
    if ($n == 0) return;
    $pct = number_format(100*$s/$n, 0)."%";
    table_row($name, $s, $f, $pct);
}

function show_vendor($vendor, $models) {
    page_head("Android task success by $vendor models");
    start_table();
    table_header("Model", "Success", "Failure", "Success rate");
    foreach ($models as $model=>$m) {
        $v = explode(" ", $model);
        $v = $v[0];
        if ($v != $vendor) continue;
        show_item($model, $m);
    }
    end_table();
    page_tail();
}

function show_vendors($vendors) {
    page_head("Android task success by vendor");
    start_table();
    table_header(
        "Vendor<br><p class=\"text-muted\">click for models</p>",
        "Success", "Failure", "Success rate"
    );
    $y = array(0,0,0,0,0,0,0,0);
    foreach ($vendors as $name=>$x) {
        if (!$name) {
            $name = "not reported by client";
        } else {
            $name = "<a href=android_tasks.php?vendor=$name>$name</a>";
        }
        show_item($name, $x);
        $y[1] += $x[1];
        $y[3] += $x[3];
    }
    show_item("total", $y);
    end_table();
    page_tail();
}

$models = get_cached_data(86400);
if ($models) {
    $models = unserialize($models);
} else {
    $models = get_models();
    set_cached_data(86400, serialize($models));
}

$vendor = get_str("vendor", true);
if ($vendor) {
    uasort($models, 'compare');
    show_vendor($vendor, $models);
} else {
    $vendors = get_vendors($models);
    uasort($vendors, 'compare');
    show_vendors($vendors);
}

?>
