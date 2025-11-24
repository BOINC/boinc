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

define("MIN_CREDIT", 1);
define("MIN_COUNT", 1);

function compare($a, $b) {
    if ($a->p_fpops < $b->p_fpops) return 1;
    if ($a->p_fpops > $b->p_fpops) return -1;
    return 0;
}

function get_data() {
    $db = BoincDb::get(true);

    // get CPU model status in a special query;
    // enumerating hosts was too slow on SETI@home.
    //
    // Ideally a model's fpops should be the median over hosts of that model.
    // But SQL has no median function.
    // Instead, take the mean of plausible values
    //
    $x = $db->enum_fields('host', 'StdClass',
        'p_model, count(*) as nhosts, avg(p_ncpus) as ncores, avg(p_fpops) as fpops',
        'p_fpops>1e6 and p_fpops<1e11 and p_fpops <> 1e9 and expavg_credit>'.MIN_CREDIT.' group by p_model',
        null
    );
    $m2 = [];
    foreach ($x as $m) {
        if ($m->nhosts < MIN_COUNT) continue;
        $y = new StdClass;
        $y->model = $m->p_model;
        $y->p_fpops = $m->fpops;
        $y->mean_ncores = $m->ncores;
        $y->nhosts = $m->nhosts;
        $m2[] = $y;
    }
    return $m2;
}

function get_cpu_list() {
    $m2 = get_data();
    uasort($m2, 'compare');
    $x = new StdClass;
    $x->cpus = $m2;
    $x->time = time();
    return $x;
    foreach ($m2 as $x) {
        $g = $x->p_fpops/1e9;
        echo "$x->model: $g gflops $x->mean_ncores cores $x->nhosts hosts \n";
    }
}

function show_cpu_list($data) {
    page_head("CPU performance");
    echo "
        This table shows peak CPU speed
        (based on Whetstone benchmarks)
        of computers participating in this project.
        <p>
    ";
    start_table();
    row_heading_array(
        array(
            "CPU model",
            "Number of computers",
            "Avg. cores/computer",
            "GFLOPS/core",
            "GFLOPs/computer"
        )
    );
    $total_nhosts = 0;
    $total_gflops = 0;
    foreach ($data->cpus as $d) {
        row_array(
            array(
                $d->model, $d->nhosts,
                number_format($d->mean_ncores, 2),
                number_format($d->p_fpops/1e9, 2),
                number_format($d->mean_ncores*$d->p_fpops/1e9, 2)
            )
        );
        $total_nhosts += $d->nhosts;
        $total_gflops += $d->nhosts*$d->mean_ncores*$d->p_fpops/1e9;
    }
    row_array(
        array(
            "Total",
            number_format($total_nhosts, 0). " computers",
            "",
            "",
            number_format($total_gflops/1e3, 2)." TeraFLOPS"
        )
    );
    end_table();
    echo "Generated ".time_str($data->time);
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
