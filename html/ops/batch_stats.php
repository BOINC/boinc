#!/usr/bin/env php
<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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
// process turnaround time stats files

// compute stats for batch acceleration
//
// - classify hosts as low turnaround time (LTT)
//      host.error_rate = 1
// - classify apps as 'accelerable' if there are sufficient LTT hosts
//      app.n_size_classes = 1
// - compute median TT of batches
//      batch.expire_time
//
// Details: https://github.com/BOINC/boinc/wiki/Batch-acceleration

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/common_defs.inc");

ini_set('display_errors', true);

// global data structures
//
$apps = [];
    // $apps[id] is a list of IDs of hosts that have completed jobs
$hosts = [];
    // $hosts[id] is a struct
    //  ntt_n
    //  ntt_sum

// compute the median TT for the batch;
// compute normalized TT for hosts w/ jobs
// classify apps as accelerable or not
//
function do_batch($batch) {
    echo "processing batch $batch->id\n";
    $results = BoincResult::enum_fields(
        'sent_time, received_time, hostid',
        sprintf('batch=%d and outcome=%d',
            $batch->id, RESULT_OUTCOME_SUCCESS
        )
    );
    $tts = [];
    foreach ($results as $r) {
        $r->tt = $r->received_time - $r->sent_time;
        $tts[] = $r->tt;
    }
    $n = count($tts);
    echo "$n success results\n";
    if ($n < 100) {
        // not enough for a meaningful median
        $batch->update("expire_time=0");
        return;
    }
    sort($tts);
    $median = $tts[intdiv($n, 2)];
    echo "median TT: $median\n";
    foreach ($results as $r) {
        add_ntt($r->hostid, $r->tt/$median);
        add_host($batch->app_id, $r->hostid);
    }

    $results = BoincResult::enum_fields(
        'hostid',
        sprintf('batch=%d and outcome=%d',
            $batch->id, RESULT_OUTCOME_NO_REPLY
        )
    );
    echo sprintf("%d timeout results\n", count($results));
    foreach ($results as $r) {
        add_ntt($r->hostid, 10.);
    }

    $batch->update("expire_time=$median");
}

// increment TT counts in hosts array; add host if needed
//
function add_ntt($hostid, $tt) {
    global $hosts;
    if (!array_key_exists($hostid, $hosts)) {
        $x = new StdClass;
        $x->ntt_n = 0;
        $x->ntt_sum = 0;
        $hosts[$hostid] = $x;
    }
    $h = $hosts[$hostid];
    $h->ntt_n += 1;
    $h->ntt_sum += $tt;
}

function add_host($appid, $hostid) {
    global $apps;
    $x = $apps[$appid];
    if (array_key_exists($hostid, $x)) {
        $x[$hostid] += 1;
    } else {
        $x[$hostid] = 1;
    }
    $apps[$appid] = $x;
}

function update_db() {
    global $hosts, $apps;

    // identify LTT hosts

    echo "Updating host LTT flags\n";
    BoincHost::update_aux("error_rate=0");
    $ltts = [];
    foreach ($hosts as $id=>$x) {
        $avg = $x->ntt_sum / $x->ntt_n;
        if ($avg < 1) {
            $ltts[] = $id;
        }
    }
    if ($ltts) {
        $ltts = implode(',', $ltts);
        BoincHost::update_aux(
            'error_rate=1',
            sprintf('id in (%s)', $ltts)
        );
    }

    // identify accelerable apps

    echo "Updating app accelerable flags\n";
    foreach ($apps as $id=>$hlist) {
        $app = BoincApp::lookup_id($id);
        $accel = false;
        $n = count($hlist);
        if ($n > 100) {
            $nfast = 0;
            foreach ($hlist as $id=>$count) {
                $x = $hosts[$id];
                $avg = $x->ntt_sum / $x->ntt_n;
                if ($avg < 1) {
                    $nfast++;
                }
            }
            echo "app $app->name $n hosts, $nfast are LTT\n";
            if ($nfast > $n*.25) {
                echo "marking $app->name as accelerable\n";
                $accel = true;
            }
        } else {
            echo "app $app->name not enough hosts: $n\n";
        }

        $app->update(
            sprintf('n_size_classes=%d', $accel?1:0)
        );
    }
}

function show_hosts() {
    global $hosts;
    foreach ($hosts as $id=>$h) {
        echo sprintf("host %d: %d jobs, avg NTT %f\n",
            $id, $h->ntt_n, $h->ntt_sum/$h->ntt_n
        );
    }
}

function main() {
    global $apps;
    echo sprintf("starting batch_stats: %s\n", time_str(time()));
    $as = BoincApp::enum('');
    foreach ($as as $a) {
        $apps[$a->id] = [];
    }
    $start = time()-30*86400;
    $batches = BoincBatch::enum(
        sprintf(
            'create_time>%d and state in (%d, %d)',
            $start, BATCH_STATE_IN_PROGRESS, BATCH_STATE_COMPLETE
        )
    );
    foreach($batches as $batch) {
        do_batch($batch);
    }
    update_db();
    echo sprintf("finished batch_stats: %s\n", time_str(time()));
}

main();
?>
