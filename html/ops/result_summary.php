<?php

    // show summary of results that have been received or timed out recently

    require_once("util_ops.inc");

    db_init();
    page_head("Result summary");

    $server_state = array();
    $outcome = array();
    $client_state = array();

    $nsecs = $_GET["nsecs"];

    for ($ss=1; $ss<6; $ss++) {
        $server_state[$ss] = 0;
    }
    for ($ro=0; $ro<6; $ro++) {
        $outcome[$ro] = 0;
    }
    for ($cs=1; $cs<6; $cs++) {
        $client_state[$cs] = 0;
    }

    $x = $nsecs/3600.;
    echo "Results that have finished in last $x hours\n";
    $y = time() - $nsecs;
    $result = mysql_query("select * from result where received_time > $y");
    while ($res = mysql_fetch_object($result)) {
        $server_state[$res->server_state] += 1;
        if ($res->server_state == 5) {
            $outcome[$res->outcome] += 1;
            if ($res->outcome == 3) {
                $client_state[$res->client_state] += 1;
            }
        }
    }
    mysql_free_result($result);

    start_table();
    echo "<tr><th>Server state</th><th># results</th></tr>\n";
    for ($ss=1; $ss<6; $ss++) {
        if ($server_state[$ss] == 0) {
            $x = "0";
        } else {
            $x = "<a href=db_action.php?table=result&received_time=$y&result_server_state=$ss&detail=low>".$server_state[$ss]."</a>";
        }
        row2(result_server_state_string($ss), $x);
    }
    end_table();

    start_table();
    echo "<tr><th>Outcome of 'Over' results</th><th># results</th></tr>\n";
    for ($ro=0; $ro<6; $ro++) {
        if ($outcome[$ro] == 0) {
            $x = "0";
        } else {
            $x = "<a href=db_action.php?table=result&received_time=$y&result_outcome=$ro&detail=low>".$outcome[$ro]."</a>";
        }
        row2(result_outcome_string($ro), $x);
    }
    end_table();

    start_table();
    echo "<tr><th>Client state of 'Client error' results</th><th># results</th></tr>\n";
    for ($cs=1; $cs<6; $cs++) {
        if ($client_state[$cs] == 0) {
            $x = "0";
        } else {
            $x = "<a href=db_action.php?table=result&received_time=$y&result_client_state=$cs&detail=low>".$client_state[$cs]."</a>";
        }
        row2(result_client_state_string($cs), $x);
    }
    end_table();

    page_tail();
?>
