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

ini_set("memory_limit", "2048M");

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/cache.inc");

define("MIN_CREDIT", 100);
define("MIN_COUNT", 10);

function compare($a, $b) {
    if ($a->p_fpops < $b->p_fpops) return 1;
    if ($a->p_fpops > $b->p_fpops) return -1;
    return 0;
}

function get_cpu_list() {
    $models = array();
    $hosts = BoincHost::enum("expavg_credit >= ".MIN_CREDIT);
    foreach($hosts as $host) {
        $x = new StdClass;
        $x->p_ncpus = $host->p_ncpus;
        $x->p_fpops = $host->p_fpops;
        if (!array_key_exists($host->p_model, $models)) {
            $models[$host->p_model] = array();
        }
        $models[$host->p_model][] = $x;
    }

    // for each model, find the median FLOPS

    $m2 = array();
    foreach ($models as $model=>$list) {
        $n = sizeof($list);
        if ($n < MIN_COUNT) continue;
        uasort($list, 'compare');
        $m = (int)($n/2);
        $total_cores = 0;
        foreach ($list as $l) {
            $total_cores += $l->p_ncpus;
        }
        $x = $list[$m]->p_fpops;
        $y = new StdClass;
        $y->model = $model;
        $y->p_fpops = $x;
        $y->mean_ncores = $total_cores/$n;
        $y->nhosts = $n;
        $m2[] = $y;
        //echo "$model: $x GFLOPS ($n samples)\n";
    }

    uasort($m2, 'compare');
    return $m2;
    foreach ($m2 as $x) {
        $g = $x->p_fpops/1e9;
        echo "$x->model: $g gflops $x->mean_ncores cores $x->nhosts hosts \n";
    }
}

function show_cpu_list($data) {
    page_head("CPU performance");
    echo "
        This table shows the median peak speed
        (Whetstone benchmark times number of CPUs)
        of the computers participating in this project.
        <p>
    ";
    start_table();
    row_heading_array(array("CPU model", "Number of computers", "Avg. cores/computer", "GFLOPS/core", "GFLOPs/computer"));
    $i = 0;
    $total_nhosts = 0;
    $total_gflops = 0;
    foreach ($data as $d) {
        row_array(
            array(
                $d->model, $d->nhosts,
                number_format($d->mean_ncores, 2),
                number_format($d->p_fpops/1e9, 2),
                number_format($d->mean_ncores*$d->p_fpops/1e9, 2)
            ),
            "row$i"
        );
        $total_nhosts += $d->nhosts;
        $total_gflops += $d->mean_ncores*$d->p_fpops/1e9;
        $i = 1-$i;
    }
    row_array(
        array("Total", $total_nhosts, "", "", number_format($total_gflops, 2)),
        "row$i"
    );
    end_table();
    page_tail();
}

$d = get_cached_data(86400);
if ($d) {
    $data = unserialize($d);
} else {
    $data = get_cpu_list();
    set_cached_data(86400, serialize($data));
}

show_cpu_list($data);

?>
