<?php {

    // show summary of results that have been received or timed out recently

    require_once("util_ops.inc");

    function link_results($n, $query) {
        if ($n == 0) {
            return "0";
        } else {
            return "<a href=db_action.php?table=result&$query&sort_by=received_time&detail=low>$n</a>";
        }
    }

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
    echo "Results that have finished in last $x hours: $x\n";
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

    echo "<table width=100%><tr valign=top>";
    echo "<td><table border=2 cellpadding=4\n";
    echo "<h3>&nbsp;</h3>";
    echo "<tr><th>Server state</th><th># results</th></tr>\n";
    for ($ss=1; $ss<6; $ss++) {
        row2(result_server_state_string($ss), 
             link_results($server_state[$ss], "result_server_state=$ss"));
    }
    echo "</table></td>";
    
    echo "<td><table border=2 cellpadding=4\n";
    echo "<h3>'Over' results:</h3>";
    echo "<tr><th>Outcome</th><th># results</th></tr>\n";
    for ($ro=0; $ro<6; $ro++) {
        c_row2(outcome_color($ro), result_outcome_string($ro),
               link_results($outcome[$ro], "result_outcome=$ro"));
    }
    echo "</table></td>";
    
    echo "<td><table border=2 cellpadding=4\n";
    echo "<h3>'Client error' results:</h3>";
    echo "<tr><th>Client state</th><th># results</th></tr>\n";
    for ($cs=1; $cs<6; $cs++) {
        row2(result_client_state_string($cs), 
             link_results($client_state[$cs], "result_client_state=$cs"));
    }
    print "</td></table>";
    print "</table>";

    page_tail();
} ?>
