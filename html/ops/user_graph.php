<?php

function draw($xarr, $arr) {
    require_once ("jpgraph/src/jpgraph.php");
    require_once ("jpgraph/src/jpgraph_line.php");
    require_once ("jpgraph/src/jpgraph_bar.php");
    require_once ("jpgraph/src/jpgraph_log.php");

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
    echo $n;
    echo $xarr[1];
    for ($i=0; $i<$n; $i++) {
        echo "<br>$xarr[$i] $yarr[$i]\n";
    }
}

function show_graph() {
    require_once("../inc/db.inc");
    db_init();

    $xaxis = $_GET['xaxis'];
    $yaxis = $_GET['yaxis'];
    $granularity = $_GET['granularity'];

    $fields = 'host.id';
    if ($xaxis == 'active') {
        $query = "select $fields, max(rpc_time) as max_rpc_time from host, user where host.userid=user.id group by userid";
    } else {
        $query = 'select $fields from user';
    }
    $result = mysql_query($query);
    $yarr = array();
    $now = time();
    $maxind = 0;
    while ($user = mysql_fetch_object($result)) {
        $val = $now - $user->max_rpc_time;
        $ind = $val/$granularity;
        $ind = (int)$ind;
        $yarr[$ind]++;
        if ($ind > $maxind) $maxind = $ind;
    }
    $xarr = array();
    for ($i=0; $i<=$maxind; $i++) {
        $xarr[$i] = $i;
        if (is_null($yarr[$i])) $yarr[$i]=0;
    }
    show_text($xarr, $yarr);
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
        Only currently active users?
        <input type=checkbox name=active>

        <p>
        Granularity:
        <input name=granularity value=86400>

        <p>
        Show as text?
        <input type=checkbox name=show_text>

        <p>
        <input type=submit name=submit value=OK>
        </form>
    ";
}

if ($_GET['submit']) {
    show_graph();
} else {
    show_form();
}

?>
