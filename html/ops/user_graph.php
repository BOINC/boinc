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

require_once("../inc/util_ops.inc");

function draw_graph($xarr, $arr) {
    require_once ("jpgraph/jpgraph.php");
    require_once ("jpgraph/jpgraph_line.php");
    require_once ("jpgraph/jpgraph_bar.php");
    require_once ("jpgraph/jpgraph_log.php");


    // Create the graph. These two calls are always required
    $graph = new Graph(350,250,"auto");
    //$graph->SetScale("lin");
    //$graph->SetScale("textlin");
    $graph->SetScale("loglin");

    // Create the linear plot
    $lineplot=new BarPlot($arr, $xarr);
    $lineplot->SetColor("blue");

    // Add the plot to the graph
    $graph->Add($lineplot);

    // Display the graph
    $graph->Stroke();
}

function show_text($xarr, $yarr) {
    $n = sizeof($xarr);
    for ($i=0; $i<$n; $i++) {
        echo "<br>$xarr[$i] $yarr[$i]\n";
    }
}

function show_graph() {
    db_init();

    $xaxis = $_GET['xaxis'];
    $yaxis = $_GET['yaxis'];
    $granularity = $_GET['granularity'];
    $active = $_GET['active'];
    $inactive = $_GET['inactive'];
    $show_text = $_GET['show_text'];

    if (!$active && !$inactive) {
        echo "You must select at least one of (active, inactive)";
        exit();
    }

    $fields = 'host.id, user.create_time';
    if ($xaxis == 'active' || !$active || !$inactive) {
        $query = "select $fields, max(rpc_time) as max_rpc_time from host, user where host.userid=user.id group by userid";
    } else {
        $query = 'select $fields from user';
    }
    $result = _mysql_query($query);
    $yarr = array();
    $now = time();
    $maxind = 0;
    $active_thresh = time() - 30*86400;
    while ($user = _mysql_fetch_object($result)) {
        $val = $now - $user->max_rpc_time;
        if (!$active) {
            if ($user->max_rpc_time > $active_thresh) continue;
        }
        if (!$inactive) {
            if ($user->max_rpc_time < $active_thresh) continue;
        }
        $life = $user->max_rpc_time - $user->create_time;
        $ind = $life/$granularity;
        $ind = (int)$ind;
        $yarr[$ind]++;
        if ($ind > $maxind) $maxind = $ind;
    }
    $xarr = array();
    for ($i=0; $i<=$maxind; $i++) {
        $xarr[$i] = $i;
        if (is_null($yarr[$i])) $yarr[$i]=0;
    }
    if ($show_text) {
        show_text($xarr, $yarr);
    } else {
        draw_graph($xarr, $yarr);
    }
}

function show_form() {
    echo "
        <form action=user_graph.php>
        X axis:
        <select name=xaxis>
        <option value=active>Active time
        </select>

        <p>
        Y axis:
        <select name=yaxis>
        <option value='count'>Count
        <option value='rac'>RAC
        <option value='tc'>Total credit
        </select>

        <p>
        Show active users?
        <input type=checkbox name=active>

        <p>
        Show inactive users?
        <input type=checkbox name=inactive>

        <p>
        Granularity:
        <input name=granularity value=86400>

        <p>
        Show as text?
        <input type=checkbox name=show_text>

        <p>
        <input class=\"btn btn-default\" type=submit name=submit value=OK>
        </form>
    ";
}

if ($_GET['submit']) {
    show_graph();
} else {
    show_form();
}

?>
