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

// show various stats of batches:
// err_host
//      list of hosts with the most errors
// err_code
//      list of exit codes with most errors
// flops_graph
//      histogram of average CPU hours

require_once('../inc/util.inc');
require_once('../inc/result.inc');

function err_host($batch) {
    $results = BoincResult::enum_fields(
        'hostid',
        sprintf('batch=%d and outcome=%d',
            $batch->id,
            RESULT_OUTCOME_CLIENT_ERROR
        )
    );
    $x = [];
    foreach ($results as $r) {
        $id = $r->hostid;
        if (array_key_exists($id, $x)) {
            $x[$id] += 1;
        } else {
            $x[$id] = 1;
        }
    }
    if (!$x) error_page('That batch had no error results');
    page_head('Errors by host');
    text_start();
    arsort($x);
    start_table();
    table_header('Host', 'OS', '# errors');
    $n = 0;
    foreach ($x as $id=>$count) {
        $host = BoincHost::lookup_id($id);
        table_row(
            "<a href=show_host_detail.php?hostid=$id>$host->domain_name</a>",
            $host->os_name,
            sprintf(
                '<a href=submit_stats.php?action=host_list_errors&host_id=%d&batch_id=%d>%d</a>',
                $id, $batch->id, $count
            )
        );
        if (++$n == 20) break;
    }
    text_end();
    end_table();
    page_tail();
}

function err_code($batch) {
    $results = BoincResult::enum_fields(
        'exit_status',
        sprintf('batch=%d and outcome=%d',
            $batch->id,
            RESULT_OUTCOME_CLIENT_ERROR
        )
    );
    $x = [];
    foreach ($results as $r) {
        $id = $r->exit_status;
        if (array_key_exists($id, $x)) {
            $x[$id] += 1;
        } else {
            $x[$id] = 1;
        }
    }
    if (!$x) error_page('That batch had no error results');
    page_head('Errors by exit code');
    text_start();
    arsort($x);
    start_table();
    table_header('Code', '# errors');
    $n = 0;
    foreach ($x as $id=>$count) {
        table_row(
            exit_status_string($id),
            sprintf(
                '<a href=submit_stats.php?action=code_list&code=%d&batch_id=%d>%d</a>',
                $id, $batch->id, $count
            )
        );
        if (++$n == 20) break;
    }
    text_end();
    end_table();
    page_tail();
}

// list of error results from a host
//
function host_list_errors($batch_id, $host_id) {
    page_head("Errors for batch $batch_id, host $host_id");
    $results = BoincResult::enum(
        sprintf('batch=%d and hostid=%d and outcome=%d',
            $batch_id, $host_id, RESULT_OUTCOME_CLIENT_ERROR
        )
    );

    start_table();
    table_header('Job instance', 'Exit status');
    foreach ($results as $r) {
        table_row(
            "<a href=result.php?resultid=$r->id>$r->name</a>",
            exit_status_string($r->exit_status)
        );
    }
    end_table();
    page_tail();
}

// list of error results with given code
//
function code_list($batch_id, $code) {
    page_head("Errors for batch $batch_id, exit code $code");
    $results = BoincResult::enum(
        sprintf('batch=%d and exit_status=%d and outcome=%d',
            $batch_id, $code, RESULT_OUTCOME_CLIENT_ERROR
        )
    );

    text_start();
    start_table();
    table_header('Job instance', 'Host');
    foreach ($results as $r) {
        $host = BoincHost::lookup_id($r->hostid);
        table_row(
            "<a href=result.php?resultid=$r->id>$r->name</a>",
            sprintf('<a href=show_host_detail.php?hostid=%d>%d (%s)</a>',
                $host->id,
                $host->id,
                $host->os_name
            )
        );
    }
    end_table();
    text_end();
    page_tail();
}

function graph($data, $id, $xlabel, $ylabel) {
    echo "
    <script type=\"text/javascript\">
      google.charts.load('current', {'packages':['corechart']});
      google.charts.setOnLoadCallback(drawChart);

      function drawChart() {
        var data = google.visualization.arrayToDataTable([
";
    // huh? should work, see https://developers.google.com/chart/interactive/docs/reference#google.visualization.arraytodatatable
    //echo "[{label: '$xlabel', type:'number'},{label:'$ylabel', type='number'}],\n";
    //echo "['$xlabel', '$ylabel'],\n";
    foreach ($data as [$x, $y]) {
        echo "[$x, $y],\n";
    }
    echo "
        ], true);

        var options = {
          title: '',
          legend:  'none',
          hAxis: {title: '$xlabel'},
          vAxis: {title: '$ylabel'}
        };

        var chart = new google.visualization.LineChart(document.getElementById('$id'));

        chart.draw(data, options);
      }
    </script>
    ";
    echo sprintf(
        '<div id="%s" style="width: 900px; height: 500px"></div>', $id
    );
}

// compute 25/50/75 quantiles of values
//
function quantiles($vals) {
    sort($vals);
    $n = count($vals);
    return [$vals[intval($n*.25)], $vals[intval($n*.5)], $vals[intval($n*.75)]];
}

// if $flops is true, show normalized runtime; else turnaround time
//
function batch_graph($batch, $flops) {
    if ($flops) {
        echo "<p>Runtimes of completed jobs, normalized to an average (4.3 GFLOPS) computer.<p>";
        // 'flops' is CPU benchmark times elapsed (i.e. run) time.
        // as of 10/2025, cpu_time is way off for BUDA jobs
        //
        $results = BoincResult::enum_fields(
            'flops_estimate*elapsed_time/(4.3e9*3600) as val',
            sprintf('batch=%d and outcome=%d',
                $batch->id, RESULT_OUTCOME_SUCCESS
            )
        );
    } else {
        page_head("Batch $batch->id turnaround times");
        echo "<p>Turnaround times of completed jobs.<p>";
        $results = BoincResult::enum_fields(
            '(received_time-sent_time)/3600 as val',
            sprintf('batch=%d and outcome=%d',
                $batch->id, RESULT_OUTCOME_SUCCESS
            )
        );
    }

    $x = [];
    $min = 1e99;
    $max = 0;
    foreach ($results as $r) {
        $f = $r->val;
        if ($f > $max) $max = $f;
        if ($f < $min) $min = $f;
        $x[] = $f;
    }
    $n = 100;
    $count = [];
    for ($i=0; $i<$n; $i++) {
        $count[$i] = 0;
    }
    $range = $max - $min;
    foreach ($x as $f) {
        $d = intval($n*($f-$min)/$range);
        if ($d >= $n) $d = $n-1;
        $count[$d] += 1;
    }
    $data = [];
    for ($i=0; $i<$n; $i++) {
        $data[] = [$min+($i*$range)/$n, $count[$i]];
    }
    if ($flops) {
        graph($data, 'runtime', 'Job runtime (hours)', 'job count');
    } else {
        graph($data, 'turnaround', 'Turnaround time (hours)', 'job count');
    }
    [$x25, $x5, $x75] = quantiles($x);
    echo sprintf('quantiles: %s %s %s',
        number_format($x25, 2),
        number_format($x5, 2),
        number_format($x75, 2)
    );
}

function batch_graphs($batch) {
    page_head("Batch $batch->id job times");
    echo "
        <script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>
    ";
    echo '<hr>';
    batch_graph($batch, true);
    echo '<hr>';
    batch_graph($batch, false);
    page_tail();
}

// show hosts that did jobs for this batch
function show_hosts($batch) {
    $results = BoincResult::enum_fields(
        'hostid',
        sprintf('batch=%d and outcome=%d',
            $batch->id,
            RESULT_OUTCOME_SUCCESS
        )
    );
    $x = [];
    foreach ($results as $r) {
        $id = $r->hostid;
        if (array_key_exists($id, $x)) {
            $x[$id] += 1;
        } else {
            $x[$id] = 1;
        }
    }
    arsort($x);
    page_head("Batch $batch->id: completed jobs grouped by host");
    text_start();
    start_table();
    table_header('Host', 'OS', '# jobs');
    foreach ($x as $id => $count) {
        $host = BoincHost::lookup_id($id);
        table_row(
            "<a href=show_host_detail.php?hostid=$id>$id</a>",
            $host->os_name,
            $count
        );
    }
    end_table();
    text_end();
    page_tail();
}

$batch = BoincBatch::lookup_id(get_int('batch_id'));
if (!$batch) error_page('no batch');
switch(get_str('action')) {
case 'err_host':
    err_host($batch);
    break;
case 'err_code':
    err_code($batch);
    break;
case 'host_list_errors':
    host_list_errors($batch->id, get_int('host_id'));
    break;
case 'code_list':
    code_list($batch->id, get_int('code'));
    break;
case 'graphs':
    batch_graphs($batch);
    break;
case 'show_hosts':
    show_hosts($batch);
    break;
default:
    error_page('bad action');
}

?>
